#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "rrr.h"
#include "bit_vector.h"

const uint8_t MAX_BLOCK_NBITS = 64;

/* This implementation is based on "Fast, Small, Simple Rank/Select on Bitmaps",
 * and implements "A Structure for Compressed Bitmaps"
 *
 *  https://users.dcc.uchile.cl/~gnavarro/ps/sea12.1.pdf
 *
 * Bit string s of length n is divided to blocks of size u bits. There are ⌈n/u⌉
 * blocks. Each block is assigned a class, r, which is the number of 1-bits in
 * the block. Each class contains (u choose r) elements. We can encode each block
 * as a pair (r, o) where r is its class and o identifies the element in class r.
 *
 * Representing a class requires ⌈lb(u+1)⌉ bits, and representing an element of
 * class r requires ⌈lb(u choose r)⌉ bits. Because this width varies based on
 * r, the amount of compression RRR encoding can achieve varies depending on
 * the entropy, H₀, of the original bit string. Entropy can be calculated by
 * n₀lb(n/n₀) + n₁lb(n/n₁); it is minimized when the bit string is either all
 * 1s or all 0s (most dense or most sparse), and maximized when the bit string
 * is half 0s and half 1s.
 *
 * The table below shows how many bits are needed to encode one input * bit,
 * based on the entropy of the input (p₁ is the probability of 1s bits, so
 * maximum entropy is p₁=0.50). RRR compression increases with lower entropy
 * and/or with larger block sizes. When entropy is high enough, an RRR-encoded
 * string will require more space than the original string.
 *
 *      u   p₁=0.05  p₁=0.10  p₁=0.20  p₁=0.50
 *     ----------------------------------------
 *      1      1.00     1.00     1.00     1.00
 *      2      1.05     1.09     1.16     1.25
 *      3      0.76     0.85     0.99     1.17
 *      4      0.85     0.93     1.08     1.28
 *      5      0.74     0.86     1.05     1.29
 *      6      0.64     0.76     0.94     1.17
 *      7      0.57     0.70     0.91     1.18
 *      8      0.64     0.77     0.97     1.23
 *     15      0.45     0.60     0.83     1.10
 *     16      0.50     0.65     0.89     1.15
 *     31      0.38     0.55     0.80     1.06
 *     32      0.41     0.58     0.82     1.09
 *     63      0.34     0.52     0.77     1.04
 *
 * One important thing to note is compression increases from u=2^k up to
 * 2^(k+1)-1, where k is an intereger; but then it drops at 2^(k+1). This is
 * because a block size of 2^k requires k+1 bits to encode each block's class,
 * but that extra bit is mostly wasted; on the other hand, block size 2^k-1
 * requires k bits to encode class and the full range from 0..2^k-1 is used.
 */

/* Number of 1-bits */
inline static uint64_t popcount(uint64_t x) {
#if defined ___POPCNT___ || defined __SSE3__
    return __builtin_popcountll(x);
#else
    register uint64_t v = x - ((x & 0xaaaaaaaaaaaaaaaa) >> 1);
    v = (v & 0x3333333333333333) + ((v >> 2) & 0x3333333333333333);
    v = (v + (v >> 4)) & 0x0f0f0f0f0f0f0f0f;
    return v * 0x0101010101010101 >> 56;
#endif
}

/* Count leading zeros */
inline static uint64_t clz(uint64_t x) {
#if defined __GNUC__ && __has_builtin(__builtin_clzll)
    return __builtin_clzll(x);
#else
    /* TODO */
#endif
}

/* Count trailing zeros */
inline static uint64_t ctz(uint64_t x) {
#if defined __GNUC__ && __has_builtin(__builtin_ctzll)
    return __builtin_ctzll(x);
#else
    /* TODO */
    return (x == 0) ? 64 : 63 - clz((x ^ -x));
#endif
}

/* Minimum number of bits needed to represent x */
inline static uint64_t nbits(uint64_t x) {
    return (x < 2) ? 0 : 64 - clz(x - 1);
}

/* (b choose r) = binomial[b][r] */
static uint64_t **binomial = NULL;

static inline uint64_t offset_nbits(uint64_t block_nbits, uint64_t class) {
    return nbits(binomial[block_nbits][class]);
}

static void
rrr_precompute_binomials(void) {
    if (binomial == NULL) {
        binomial = ALLOC_N(uint64_t*, MAX_BLOCK_NBITS + 1);

        /* (n choose k) = binomial[n][k] */
        for (uint8_t n = 0; n <= MAX_BLOCK_NBITS; n ++) {
            binomial[n] = ALLOC_N(uint64_t, n+1);
            binomial[n][0] = 1;
            binomial[n][n] = 1;

            for (uint8_t k = 1; k < n; k ++)
                binomial[n][k] = binomial[n - 1][k - 1] + binomial[n - 1][k];
        }
    }
}

static inline uint64_t
rrr_encode_block(uint64_t block_nbits, uint64_t class, uint64_t value) {
    assert(block_nbits > 0);
    assert(binomial != NULL);
    assert(class <= block_nbits);
    assert(class == popcount(value));

    if (class == 0 || class == block_nbits)
        return 0;

    /* When block_nbits is 5, here are all elements of class 2 next to their
     * offset:
     *
     *   0:  00011 \ There are (5-1 choose 2) values with first bit 0
     *   1:  00101 |
     *   2:  00110 |
     *   3:  01001 |
     *   4:  01010 |
     *   5:  01100 /
     *   6:  10001 \ There are (5-1 choose 1) values with first bit 1
     *   7:  10010 |
     *   8:  10100 |
     *   9:  11000 /
     *
     * We can determine the offset of a value from this set by first inspecting
     * its 5th bit. If it's 0, we know it's one of the first 6 values. If it's
     * 1, we know offset >= 6 because 6 values precede it. We next look at the
     * 4th bit and so on, until we've accounted for all the 1s in the given
     * value.
     */

    uint64_t offset = 0; /* Minimum offset so far */
    uint64_t n;         /* Which bit we're inspecting */

    /* Immediately skip leading zeros to the most significant 1-bit */
    n = (value == 0) ? 0 : 63 - clz(value);

    for (; class > 0 && n >= class; n --) {
        if (value & (1ULL << n)) {
            offset += binomial[n][class];
            class  --;
        }
    };

    return offset;
}

static inline uint64_t
rrr_decode_block(uint64_t block_nbits, uint64_t class, uint64_t offset) {
    assert(block_nbits > 0);
    assert(binomial != NULL);
    assert(class <= block_nbits);
    assert(offset < binomial[block_nbits][class]);

    /* When block_nbits is 5, here are the elements of class 2 with their offset:
     *
     *   0:  00011 \ There are (5-1 choose 2) values with first bit 0
     *   1:  00101 |
     *   2:  00110 |
     *   3:  01001 |
     *   4:  01010 |
     *   5:  01100 /
     *   6:  10001 \ There are (5-1 choose 1) values with first bit 1
     *   7:  10010 |
     *   8:  10100 |
     *   9:  11000 /
     *
     * We can determine the value at an offset by first comparing the offset to
     * binomial(5-1, 2) = 6. If it's less, then the first bit must be zero, else
     * it is 1. The next bit is determined by comparing either binomial(4, 2)
     * or binomial(4, 1) depending on how many 1s bits have been accounted for.
     * This continues until two 1s bits have been generated.
     */

    uint64_t value = 0;
    uint64_t n = block_nbits - 1; /* Which bit we're generating */

    for (; class <= n && n > 0; n --) {
        uint64_t before = binomial[n][class];

        if (before <= offset) {
            value  |= (1ULL << n);
            offset -= before;
            class  --;
        }
    }

    if (class > 0)
        value |= (1ULL << class) - 1;

    return value;
}

void rrr_free(rrr_t* rrr) {
    FREE(rrr->classes);
    FREE(rrr->offsets);
    FREE(rrr->marked_ranks);
    FREE(rrr->marked_offsets);
    FREE(rrr);
}

void
rrr_print(const rrr_t* rrr) {
    if (rrr == NULL) {
        printf("NULL");
        return;
    }

    printf("<rrr size=%u rank=%u t=%u s=%u\n", rrr->size, rrr->rank, rrr->block_nbits, rrr->marker_nbits);
    printf("  classes="); bit_vector_print(rrr->classes); printf("\n");
    printf("  offsets="); bit_vector_print(rrr->offsets); printf("\n");
    printf("  marked_ranks="); bit_vector_print(rrr->marked_ranks); printf("\n");
    printf("  marked_offsets="); bit_vector_print(rrr->marked_offsets); printf(">\n");
}

rrr_t*
rrr_alloc(bit_vector_t* bits, uint8_t block_nbits, uint8_t marker_nbits) {
    assert(bits->size > 0);
    assert(block_nbits <= MAX_BLOCK_NBITS);
    assert(block_nbits <= marker_nbits);

    /* One-time initialization of global variables */
    rrr_precompute_binomials();

    rrr_t *rrr  = ALLOC(rrr_t);
    rrr->size         = bits->size;
    rrr->rank         = 0;
    rrr->nblocks      = (bits->size + block_nbits - 1) / block_nbits;
    rrr->nmarkers     = (bits->size + marker_nbits - 1) / marker_nbits;
    rrr->block_nbits  = block_nbits;
    rrr->marker_nbits = marker_nbits;

    uint64_t orig_record_nbits, offset_at, class_at, marker_at, offset_nbits_max, marker_need;

    /* The most bits needed to store any offset. While offsets are stored
     * in variable number of bits, this is used to estimate how much space
     * to allocate for the whole vector of offsets. We will give back any
     * unused bits. */
    offset_nbits_max = offset_nbits(block_nbits, block_nbits / 2);

    /* These two vectors are enough to represent the original bit vector. The
     * additional vectors allocated below are the o(n) atop nH₀, and are used
     * for making rank and select operations fast. */
    rrr->classes = bit_vector_alloc_record(nbits(block_nbits + 1), rrr->nblocks);
    rrr->offsets = bit_vector_alloc(rrr->nblocks * offset_nbits_max);

    class_at  = 0;
    offset_at = 0;

    rrr->marked_ranks   = bit_vector_alloc_record(nbits(bits->size + 1), rrr->nmarkers);
    rrr->marked_offsets = bit_vector_alloc_record(nbits(rrr->offsets->size), rrr->nmarkers);

    marker_at   = 0;
    marker_need = marker_nbits;

    /* Read and encode input one block at a time */
    orig_record_nbits  = bits->record_nbits;
    bits->record_nbits = block_nbits;
    for (uint64_t k = 0; k < rrr->nblocks; k ++) {
        uint64_t block, class, offset;
        block  = bit_vector_read_record(bits, k);
        class  = popcount(block);
        offset = rrr_encode_block(block_nbits, class, block);

        class_at  = bit_vector_write_record(rrr->classes, class_at, class);
        offset_at = bit_vector_write(rrr->offsets, offset_at, offset_nbits(block_nbits, class), offset);

        /* Write marker if we have enough data. We know there is no more
         * than one marker because block_nbits <= sblock_nbits */
        int64_t marker_extra = block_nbits - marker_need;

        if (marker_extra >= 0) {
            /* We might need only first few bits from `block` */
            uint64_t want, prefix;
            want   = block_nbits - marker_extra;
            prefix = (want >= 64) ? block : block & ((1ULL << want) - 1);

            bit_vector_write_record(rrr->marked_offsets, marker_at, offset_at);
            bit_vector_write_record(rrr->marked_ranks, marker_at, rrr->rank + popcount(prefix));
            marker_at ++;

            /* Next marker counts the bits not used in last marker */
            marker_need = marker_nbits - marker_extra;
        } else {
            marker_need -= block_nbits;
        }

        rrr->rank += class;
    }

    bits->record_nbits = orig_record_nbits;

    /* Truncate unused space */
    bit_vector_resize(rrr->offsets, offset_at);

    return rrr;
}

uint8_t /* access(B, i) = B[i] */
rrr_access(const rrr_t* rrr, uint64_t i) {
    assert(rrr != NULL);
    assert(i < rrr->size);

    uint64_t marker_at, class_at, offset_at, class, width, offset, block;

    /* Find nearest marker so we can skip forward in rrr->offsets */
    marker_at = i / rrr->marker_nbits;

    if (marker_at <= 0) {
        class_at  = 0;
        offset_at = 0;
    } else {
        class_at  = (marker_at * rrr->marker_nbits) / rrr->block_nbits;
        offset_at = bit_vector_read_record(rrr->marked_offsets, marker_at - 1);
    }

    /* Move forward one block at a time */
    for ( i -= class_at * rrr->block_nbits
        ; i >= rrr->block_nbits
        ; i -= rrr->block_nbits ) {
        class = bit_vector_read_record(rrr->classes, class_at);
        width = offset_nbits(rrr->block_nbits, class);
        offset_at += width;
        class_at  ++;
    }

    class  = bit_vector_read_record(rrr->classes, class_at);
    width  = offset_nbits(rrr->block_nbits, class);
    offset = bit_vector_read(rrr->offsets, offset_at, width);
    block  = rrr_decode_block(rrr->block_nbits, class, offset);

    return (block & (1 << i)) >> i;
}

uint32_t /* rank0(B, i) = |{j ∈ [0, i) : B[j] = 0}| */
rrr_rank0(const rrr_t* rrr, uint64_t i) {
    return i - rrr_rank1(rrr, i);
}

uint32_t /* rank0(B, i) = |{j ∈ [0, i) : B[j] = 1}| */
rrr_rank1(const rrr_t* rrr, uint64_t i) {
    assert(rrr != NULL);

    if (i >= rrr->size)
        return rrr->rank;

    uint64_t marker_at, class_at, offset_at, rank, class, width, offset, block, mask;

    /* Find nearest sample so we can skip forward in rrr->offsets */
    marker_at = i / rrr->marker_nbits;

    if (marker_at <= 0) {
        class_at  = 0;
        offset_at = 0;
        rank      = 0;
    } else {
        class_at  = (marker_at * rrr->marker_nbits) / rrr->block_nbits;
        offset_at = bit_vector_read_record(rrr->marked_offsets, marker_at - 1);
        rank      = bit_vector_read_record(rrr->marked_ranks, marker_at - 1);
    }

    /* Move forward one block at a time */
    for ( i -= class_at * rrr->block_nbits
        ; i >= rrr->block_nbits
        ; i -= rrr->block_nbits) {
        class = bit_vector_read_record(rrr->classes, class_at);
        width = offset_nbits(rrr->block_nbits, class);

        rank      += class;
        offset_at += width;
        class_at  ++;
    }

    class  = bit_vector_read_record(rrr->classes, class_at);
    width  = offset_nbits(rrr->block_nbits, class);
    offset = bit_vector_read(rrr->offsets, offset_at, width);
    block  = rrr_decode_block(rrr->block_nbits, class, offset);
    mask   = (1ULL << i) - 1;

    return (uint32_t) (rank + popcount(block & mask));
}

uint32_t /* select0(B, i) = max{j ∈ [0, n) | rank0(j) = i} */
rrr_select0(const rrr_t* rrr, uint64_t i) {
    /* TODO */
    return 0ULL;
}

static uint64_t
rrr_find_marker(const rrr_t* rrr, uint64_t j) {
    assert(rrr != NULL);
    assert(j <= rrr->rank);

    uint64_t lo, hi, _k, k, rank;

    /* Check if `j` occurs before the start of the first marker */
    if (rrr->size <= rrr->marker_nbits
            || j < bit_vector_read_record(rrr->marked_ranks, 0))
        return 0;

    for ( lo = 0, hi = rrr->nmarkers - 1, _k = -1
        ; k = lo + (hi - lo) / 2, lo <= hi ; ) {
        rank = bit_vector_read_record(rrr->marked_ranks, k);

        if (rank < j)
            lo = (_k = k) + 1;
        else if (j < rank)
            hi = k - 1;
        else
            break;
    }

    if (j <= rank)
        k = _k;

    return k + 1;
}

uint32_t /* select0(B, i) = max{j ∈ [0, n) | rank1(j) = i} */
rrr_select1(const rrr_t* rrr, uint64_t j) {
    if (j > rrr->rank)
        return 0;

    uint64_t marker_at, class_at, offset_at, class, width, rank, block, offset;
    int64_t i;

    /* Start at marker before rank = j */
    marker_at = rrr_find_marker(rrr, j);
    class_at  = (marker_at * rrr->marker_nbits) / rrr->block_nbits;

    if (marker_at <= 0) {
        offset_at = 0;
        rank      = 0;
    } else {
        offset_at = bit_vector_read_record(rrr->marked_offsets, marker_at - 1);
        rank      = bit_vector_read_record(rrr->marked_ranks, marker_at - 1);
    }

    /* Scan past blocks one at a time */
    for (; class_at < rrr->nblocks; class_at ++) {
        class = bit_vector_read_record(rrr->classes, class_at);
        width = offset_nbits(rrr->block_nbits, class);

        if (rank + class >= j)
            break;

        rank      += class;
        offset_at += width;
    }

    /* The j-th bit occurs within this block */
    offset = bit_vector_read(rrr->offsets, offset_at, width);
    block  = rrr_decode_block(rrr->block_nbits, class, offset);

    assert(j - rank <= popcount(block));

    /* Need to locate the (j - rank)-th 1-bit of block */
    for (i = -1; rank < j; rank ++) {
        i      = ctz(block);
        block &= ~(1ULL << i);
    }

    return 1 + i + class_at * rrr->block_nbits;
}
