#include "interval_tree.h"
#include "bit_vector.h"

/* Bitmap indicating ISO-8859-x graphic characters starting from 0xff..0xa0 (lsb) */
bit_vector_t iso_8859_graphic[16] = {
    {.size = 96, .data = (uint64_t[]){0xffffffffffffffff,0x00000000ffffffff}}, // iso-8859-1
    {.size = 96, .data = (uint64_t[]){0xffffffffffffffff,0x00000000ffffffff}}, // iso-8859-2
    {.size = 96, .data = (uint64_t[]){0xfffefff7bfffbfdf,0x00000000fffefff7}}, // iso-8859-3
    {.size = 96, .data = (uint64_t[]){0xffffffffffffffff,0x00000000ffffffff}}, // iso-8859-4
    {.size = 96, .data = (uint64_t[]){0xffffffffffffffff,0x00000000ffffffff}}, // iso-8859-5
    {.size = 96, .data = (uint64_t[]){0x07fffffe88003011,0x000000000007ffff}}, // iso-8859-6
    {.size = 96, .data = (uint64_t[]){0xfffbffffffffbfff,0x000000007fffffff}}, // iso-8859-7
    {.size = 96, .data = (uint64_t[]){0x800000007ffffffd,0x0000000067ffffff}}, // iso-8859-8
    {.size = 96, .data = (uint64_t[]){0xffffffffffffffff,0x00000000ffffffff}}, // iso-8859-9
    {.size = 96, .data = (uint64_t[]){0xffffffffffffffff,0x00000000ffffffff}}, // iso-8859-10
    {.size = 96, .data = (uint64_t[]){0x87ffffffffffffff,0x000000000fffffff}}, // iso-8859-11
    {.size = 96, .data = (uint64_t[]){0x0000000000000000,0x0000000000000000}}, // iso-8859-12
    {.size = 96, .data = (uint64_t[]){0xffffffffffffffff,0x00000000ffffffff}}, // iso-8859-13
    {.size = 96, .data = (uint64_t[]){0xffffffffffffffff,0x00000000ffffffff}}, // iso-8859-14
    {.size = 96, .data = (uint64_t[]){0xffffffffffffffff,0x00000000ffffffff}}, // iso-8859-15
    {.size = 96, .data = (uint64_t[]){0xffffffffffffffff,0x00000000ffffffff}}, // iso-8859-16
};

/* List of intervals of indicating graphic Unicode codepoints */
interval_tree_t ucs_codepoints_graphic = {
  .length = 667,
  .min = (unsigned int[667]){0x20,0xa0,0xae,0x37a,0x384,0x38c,0x38e,0x3a3,0x531,0x559,0x58d,0x591,0x5d0,0x5ef,0x606,0x61e,0x6de,0x710,0x74d,0x7c0,0x7fd,0x830,0x840,0x85e,0x860,0x8a0,0x8b6,0x8d3,0x8e3,0x985,0x98f,0x993,0x9aa,0x9b2,0x9b6,0x9bc,0x9c7,0x9cb,0x9d7,0x9dc,0x9df,0x9e6,0xa01,0xa05,0xa0f,0xa13,0xa2a,0xa32,0xa35,0xa38,0xa3c,0xa3e,0xa47,0xa4b,0xa51,0xa59,0xa5e,0xa66,0xa81,0xa85,0xa8f,0xa93,0xaaa,0xab2,0xab5,0xabc,0xac7,0xacb,0xad0,0xae0,0xae6,0xaf9,0xb01,0xb05,0xb0f,0xb13,0xb2a,0xb32,0xb35,0xb3c,0xb47,0xb4b,0xb56,0xb5c,0xb5f,0xb66,0xb82,0xb85,0xb8e,0xb92,0xb99,0xb9c,0xb9e,0xba3,0xba8,0xbae,0xbbe,0xbc6,0xbca,0xbd0,0xbd7,0xbe6,0xc00,0xc0e,0xc12,0xc2a,0xc3d,0xc46,0xc4a,0xc55,0xc58,0xc60,0xc66,0xc77,0xc8e,0xc92,0xcaa,0xcb5,0xcbc,0xcc6,0xcca,0xcd5,0xcde,0xce0,0xce6,0xcf1,0xd00,0xd05,0xd0e,0xd12,0xd46,0xd4a,0xd54,0xd66,0xd82,0xd85,0xd9a,0xdb3,0xdbd,0xdc0,0xdca,0xdcf,0xdd6,0xdd8,0xde6,0xdf2,0xe01,0xe3f,0xe81,0xe84,0xe86,0xe8c,0xea5,0xea7,0xec0,0xec6,0xec8,0xed0,0xedc,0xf00,0xf49,0xf71,0xf99,0xfbe,0xfce,0x1000,0x10c7,0x10cd,0x10d0,0x124a,0x1250,0x1258,0x125a,0x1260,0x128a,0x1290,0x12b2,0x12b8,0x12c0,0x12c2,0x12c8,0x12d8,0x1312,0x1318,0x135d,0x1380,0x13a0,0x13f8,0x1400,0x16a0,0x1700,0x170e,0x1720,0x1740,0x1760,0x176e,0x1772,0x1780,0x17e0,0x17f0,0x1800,0x1810,0x1820,0x1880,0x18b0,0x1900,0x1920,0x1930,0x1940,0x1944,0x1970,0x1980,0x19b0,0x19d0,0x19de,0x1a1e,0x1a60,0x1a7f,0x1a90,0x1aa0,0x1ab0,0x1b00,0x1b50,0x1b80,0x1bfc,0x1c3b,0x1c4d,0x1c90,0x1cbd,0x1cd0,0x1d00,0x1dfb,0x1f18,0x1f20,0x1f48,0x1f50,0x1f59,0x1f5b,0x1f5d,0x1f5f,0x1f80,0x1fb6,0x1fc6,0x1fd6,0x1fdd,0x1ff2,0x1ff6,0x2000,0x2010,0x202f,0x2070,0x2074,0x2090,0x20a0,0x20d0,0x2100,0x2190,0x2440,0x2460,0x2b76,0x2b98,0x2c30,0x2c60,0x2cf9,0x2d27,0x2d2d,0x2d30,0x2d6f,0x2d7f,0x2da0,0x2da8,0x2db0,0x2db8,0x2dc0,0x2dc8,0x2dd0,0x2dd8,0x2de0,0x2e80,0x2e9b,0x2f00,0x2ff0,0x3000,0x3041,0x3099,0x3105,0x3131,0x3190,0x31c0,0x31f0,0x3220,0x4dc0,0xa000,0xa490,0xa4d0,0xa640,0xa700,0xa7c2,0xa7f7,0xa830,0xa840,0xa880,0xa8ce,0xa8e0,0xa95f,0xa980,0xa9cf,0xa9de,0xaa00,0xaa40,0xaa50,0xaa5c,0xaadb,0xab01,0xab09,0xab11,0xab20,0xab28,0xab30,0xab70,0xabf0,0xac00,0xd7b0,0xd7cb,0xf900,0xfa70,0xfb00,0xfb13,0xfb1d,0xfb38,0xfb3e,0xfb40,0xfb43,0xfb46,0xfbd3,0xfd50,0xfd92,0xfdf0,0xfe00,0xfe20,0xfe54,0xfe68,0xfe70,0xfe76,0xff01,0xffc2,0xffca,0xffd2,0xffda,0xffe0,0xffe8,0xfffc,0x10000,0x1000d,0x10028,0x1003c,0x1003f,0x10050,0x10080,0x10100,0x10107,0x10137,0x10190,0x101a0,0x101d0,0x10280,0x102a0,0x102e0,0x10300,0x1032d,0x10350,0x10380,0x1039f,0x103c8,0x10400,0x104a0,0x104b0,0x104d8,0x10500,0x10530,0x1056f,0x10600,0x10740,0x10760,0x10800,0x10808,0x1080a,0x10837,0x1083c,0x1083f,0x10857,0x108a7,0x108e0,0x108f4,0x108fb,0x1091f,0x1093f,0x10980,0x109bc,0x109d2,0x10a05,0x10a0c,0x10a15,0x10a19,0x10a38,0x10a3f,0x10a50,0x10a60,0x10ac0,0x10aeb,0x10b00,0x10b39,0x10b58,0x10b78,0x10b99,0x10ba9,0x10c00,0x10c80,0x10cc0,0x10cfa,0x10d30,0x10e60,0x10f00,0x10f30,0x10fe0,0x11000,0x11052,0x1107f,0x110be,0x110d0,0x110f0,0x11100,0x11136,0x11150,0x11180,0x111d0,0x111e1,0x11200,0x11213,0x11280,0x11288,0x1128a,0x1128f,0x1129f,0x112b0,0x112f0,0x11300,0x11305,0x1130f,0x11313,0x1132a,0x11332,0x11335,0x1133b,0x11347,0x1134b,0x11350,0x11357,0x1135d,0x11366,0x11370,0x11400,0x1145b,0x1145d,0x11480,0x114d0,0x11580,0x115b8,0x11600,0x11650,0x11660,0x11680,0x116c0,0x11700,0x1171d,0x11730,0x11800,0x118a0,0x118ff,0x119a0,0x119aa,0x119da,0x11a00,0x11a50,0x11ac0,0x11c00,0x11c0a,0x11c38,0x11c50,0x11c70,0x11c92,0x11ca9,0x11d00,0x11d08,0x11d0b,0x11d3a,0x11d3c,0x11d3f,0x11d50,0x11d60,0x11d67,0x11d6a,0x11d90,0x11d93,0x11da0,0x11ee0,0x11fc0,0x11fff,0x12400,0x12470,0x12480,0x13000,0x14400,0x16800,0x16a40,0x16a60,0x16a6e,0x16ad0,0x16af0,0x16b00,0x16b50,0x16b5b,0x16b63,0x16b7d,0x16e40,0x16f00,0x16f4f,0x16f8f,0x16fe0,0x17000,0x18800,0x1b000,0x1b150,0x1b164,0x1b170,0x1bc00,0x1bc70,0x1bc80,0x1bc90,0x1bc9c,0x1d000,0x1d100,0x1d129,0x1d17b,0x1d200,0x1d2e0,0x1d300,0x1d360,0x1d400,0x1d456,0x1d49e,0x1d4a2,0x1d4a5,0x1d4a9,0x1d4ae,0x1d4bb,0x1d4bd,0x1d4c5,0x1d507,0x1d50d,0x1d516,0x1d51e,0x1d53b,0x1d540,0x1d546,0x1d54a,0x1d552,0x1d6a8,0x1d7ce,0x1da9b,0x1daa1,0x1e000,0x1e008,0x1e01b,0x1e023,0x1e026,0x1e100,0x1e130,0x1e140,0x1e14e,0x1e2c0,0x1e2ff,0x1e800,0x1e8c7,0x1e900,0x1e950,0x1e95e,0x1ec71,0x1ed01,0x1ee00,0x1ee05,0x1ee21,0x1ee24,0x1ee27,0x1ee29,0x1ee34,0x1ee39,0x1ee3b,0x1ee42,0x1ee47,0x1ee49,0x1ee4b,0x1ee4d,0x1ee51,0x1ee54,0x1ee57,0x1ee59,0x1ee5b,0x1ee5d,0x1ee5f,0x1ee61,0x1ee64,0x1ee67,0x1ee6c,0x1ee74,0x1ee79,0x1ee7e,0x1ee80,0x1ee8b,0x1eea1,0x1eea5,0x1eeab,0x1eef0,0x1f000,0x1f030,0x1f0a0,0x1f0b1,0x1f0c1,0x1f0d1,0x1f100,0x1f110,0x1f170,0x1f1e6,0x1f210,0x1f240,0x1f250,0x1f260,0x1f300,0x1f6e0,0x1f6f0,0x1f700,0x1f780,0x1f7e0,0x1f800,0x1f810,0x1f850,0x1f860,0x1f890,0x1f900,0x1f90d,0x1f973,0x1f97a,0x1f9a5,0x1f9ae,0x1f9cd,0x1fa60,0x1fa70,0x1fa78,0x1fa80,0x1fa90,0x20000,0x2a700,0x2b740,0x2b820,0x2ceb0,0x2f800,0xe0100},
  .max = (unsigned int[667]){0x7e,0xac,0x377,0x37f,0x38a,0x38c,0x3a1,0x52f,0x556,0x58a,0x58f,0x5c7,0x5ea,0x5f4,0x61b,0x6dc,0x70d,0x74a,0x7b1,0x7fa,0x82d,0x83e,0x85b,0x85e,0x86a,0x8b4,0x8bd,0x8e1,0x983,0x98c,0x990,0x9a8,0x9b0,0x9b2,0x9b9,0x9c4,0x9c8,0x9ce,0x9d7,0x9dd,0x9e3,0x9fe,0xa03,0xa0a,0xa10,0xa28,0xa30,0xa33,0xa36,0xa39,0xa3c,0xa42,0xa48,0xa4d,0xa51,0xa5c,0xa5e,0xa76,0xa83,0xa8d,0xa91,0xaa8,0xab0,0xab3,0xab9,0xac5,0xac9,0xacd,0xad0,0xae3,0xaf1,0xaff,0xb03,0xb0c,0xb10,0xb28,0xb30,0xb33,0xb39,0xb44,0xb48,0xb4d,0xb57,0xb5d,0xb63,0xb77,0xb83,0xb8a,0xb90,0xb95,0xb9a,0xb9c,0xb9f,0xba4,0xbaa,0xbb9,0xbc2,0xbc8,0xbcd,0xbd0,0xbd7,0xbfa,0xc0c,0xc10,0xc28,0xc39,0xc44,0xc48,0xc4d,0xc56,0xc5a,0xc63,0xc6f,0xc8c,0xc90,0xca8,0xcb3,0xcb9,0xcc4,0xcc8,0xccd,0xcd6,0xcde,0xce3,0xcef,0xcf2,0xd03,0xd0c,0xd10,0xd44,0xd48,0xd4f,0xd63,0xd7f,0xd83,0xd96,0xdb1,0xdbb,0xdbd,0xdc6,0xdca,0xdd4,0xdd6,0xddf,0xdef,0xdf4,0xe3a,0xe5b,0xe82,0xe84,0xe8a,0xea3,0xea5,0xebd,0xec4,0xec6,0xecd,0xed9,0xedf,0xf47,0xf6c,0xf97,0xfbc,0xfcc,0xfda,0x10c5,0x10c7,0x10cd,0x1248,0x124d,0x1256,0x1258,0x125d,0x1288,0x128d,0x12b0,0x12b5,0x12be,0x12c0,0x12c5,0x12d6,0x1310,0x1315,0x135a,0x137c,0x1399,0x13f5,0x13fd,0x169c,0x16f8,0x170c,0x1714,0x1736,0x1753,0x176c,0x1770,0x1773,0x17dd,0x17e9,0x17f9,0x180d,0x1819,0x1878,0x18aa,0x18f5,0x191e,0x192b,0x193b,0x1940,0x196d,0x1974,0x19ab,0x19c9,0x19da,0x1a1b,0x1a5e,0x1a7c,0x1a89,0x1a99,0x1aad,0x1abe,0x1b4b,0x1b7c,0x1bf3,0x1c37,0x1c49,0x1c88,0x1cba,0x1cc7,0x1cfa,0x1df9,0x1f15,0x1f1d,0x1f45,0x1f4d,0x1f57,0x1f59,0x1f5b,0x1f5d,0x1f7d,0x1fb4,0x1fc4,0x1fd3,0x1fdb,0x1fef,0x1ff4,0x1ffe,0x200a,0x2029,0x205f,0x2071,0x208e,0x209c,0x20bf,0x20f0,0x218b,0x2426,0x244a,0x2b73,0x2b95,0x2c2e,0x2c5e,0x2cf3,0x2d25,0x2d27,0x2d2d,0x2d67,0x2d70,0x2d96,0x2da6,0x2dae,0x2db6,0x2dbe,0x2dc6,0x2dce,0x2dd6,0x2dde,0x2e4f,0x2e99,0x2ef3,0x2fd5,0x2ffb,0x303f,0x3096,0x30ff,0x312f,0x318e,0x31ba,0x31e3,0x321e,0x4db5,0x9fef,0xa48c,0xa4c6,0xa62b,0xa6f7,0xa7bf,0xa7c6,0xa82b,0xa839,0xa877,0xa8c5,0xa8d9,0xa953,0xa97c,0xa9cd,0xa9d9,0xa9fe,0xaa36,0xaa4d,0xaa59,0xaac2,0xaaf6,0xab06,0xab0e,0xab16,0xab26,0xab2e,0xab67,0xabed,0xabf9,0xd7a3,0xd7c6,0xd7fb,0xfa6d,0xfad9,0xfb06,0xfb17,0xfb36,0xfb3c,0xfb3e,0xfb41,0xfb44,0xfbc1,0xfd3f,0xfd8f,0xfdc7,0xfdfd,0xfe19,0xfe52,0xfe66,0xfe6b,0xfe74,0xfefc,0xffbe,0xffc7,0xffcf,0xffd7,0xffdc,0xffe6,0xffee,0xfffd,0x1000b,0x10026,0x1003a,0x1003d,0x1004d,0x1005d,0x100fa,0x10102,0x10133,0x1018e,0x1019b,0x101a0,0x101fd,0x1029c,0x102d0,0x102fb,0x10323,0x1034a,0x1037a,0x1039d,0x103c3,0x103d5,0x1049d,0x104a9,0x104d3,0x104fb,0x10527,0x10563,0x1056f,0x10736,0x10755,0x10767,0x10805,0x10808,0x10835,0x10838,0x1083c,0x10855,0x1089e,0x108af,0x108f2,0x108f5,0x1091b,0x10939,0x1093f,0x109b7,0x109cf,0x10a03,0x10a06,0x10a13,0x10a17,0x10a35,0x10a3a,0x10a48,0x10a58,0x10a9f,0x10ae6,0x10af6,0x10b35,0x10b55,0x10b72,0x10b91,0x10b9c,0x10baf,0x10c48,0x10cb2,0x10cf2,0x10d27,0x10d39,0x10e7e,0x10f27,0x10f59,0x10ff6,0x1104d,0x1106f,0x110bc,0x110c1,0x110e8,0x110f9,0x11134,0x11146,0x11176,0x111cd,0x111df,0x111f4,0x11211,0x1123e,0x11286,0x11288,0x1128d,0x1129d,0x112a9,0x112ea,0x112f9,0x11303,0x1130c,0x11310,0x11328,0x11330,0x11333,0x11339,0x11344,0x11348,0x1134d,0x11350,0x11357,0x11363,0x1136c,0x11374,0x11459,0x1145b,0x1145f,0x114c7,0x114d9,0x115b5,0x115dd,0x11644,0x11659,0x1166c,0x116b8,0x116c9,0x1171a,0x1172b,0x1173f,0x1183b,0x118f2,0x118ff,0x119a7,0x119d7,0x119e4,0x11a47,0x11aa2,0x11af8,0x11c08,0x11c36,0x11c45,0x11c6c,0x11c8f,0x11ca7,0x11cb6,0x11d06,0x11d09,0x11d36,0x11d3a,0x11d3d,0x11d47,0x11d59,0x11d65,0x11d68,0x11d8e,0x11d91,0x11d98,0x11da9,0x11ef8,0x11ff1,0x12399,0x1246e,0x12474,0x12543,0x1342e,0x14646,0x16a38,0x16a5e,0x16a69,0x16a6f,0x16aed,0x16af5,0x16b45,0x16b59,0x16b61,0x16b77,0x16b8f,0x16e9a,0x16f4a,0x16f87,0x16f9f,0x16fe3,0x187f7,0x18af2,0x1b11e,0x1b152,0x1b167,0x1b2fb,0x1bc6a,0x1bc7c,0x1bc88,0x1bc99,0x1bc9f,0x1d0f5,0x1d126,0x1d172,0x1d1e8,0x1d245,0x1d2f3,0x1d356,0x1d378,0x1d454,0x1d49c,0x1d49f,0x1d4a2,0x1d4a6,0x1d4ac,0x1d4b9,0x1d4bb,0x1d4c3,0x1d505,0x1d50a,0x1d514,0x1d51c,0x1d539,0x1d53e,0x1d544,0x1d546,0x1d550,0x1d6a5,0x1d7cb,0x1da8b,0x1da9f,0x1daaf,0x1e006,0x1e018,0x1e021,0x1e024,0x1e02a,0x1e12c,0x1e13d,0x1e149,0x1e14f,0x1e2f9,0x1e2ff,0x1e8c4,0x1e8d6,0x1e94b,0x1e959,0x1e95f,0x1ecb4,0x1ed3d,0x1ee03,0x1ee1f,0x1ee22,0x1ee24,0x1ee27,0x1ee32,0x1ee37,0x1ee39,0x1ee3b,0x1ee42,0x1ee47,0x1ee49,0x1ee4b,0x1ee4f,0x1ee52,0x1ee54,0x1ee57,0x1ee59,0x1ee5b,0x1ee5d,0x1ee5f,0x1ee62,0x1ee64,0x1ee6a,0x1ee72,0x1ee77,0x1ee7c,0x1ee7e,0x1ee89,0x1ee9b,0x1eea3,0x1eea9,0x1eebb,0x1eef1,0x1f02b,0x1f093,0x1f0ae,0x1f0bf,0x1f0cf,0x1f0f5,0x1f10c,0x1f16c,0x1f1ac,0x1f202,0x1f23b,0x1f248,0x1f251,0x1f265,0x1f6d5,0x1f6ec,0x1f6fa,0x1f773,0x1f7d8,0x1f7eb,0x1f80b,0x1f847,0x1f859,0x1f887,0x1f8ad,0x1f90b,0x1f971,0x1f976,0x1f9a2,0x1f9aa,0x1f9ca,0x1fa53,0x1fa6d,0x1fa73,0x1fa7a,0x1fa82,0x1fa95,0x2a6d6,0x2b734,0x2b81d,0x2cea1,0x2ebe0,0x2fa1d,0xe01ef}
};

/* List of intervals of indicating whitespace Unicode codepoints */
interval_tree_t ucs_codepoints_whitespace = {
  .length = 10,
  .min = (unsigned int[10]){0x09,0x20,0x85,0xa0,0x1680,0x2000,0x2028,0x202f,0x205f,0x3000},
  .max = (unsigned int[10]){0x0d,0x20,0x85,0xa0,0x1680,0x200a,0x2029,0x202f,0x205f,0x3000}
};
