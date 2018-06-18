#include <Arduboy2.h>

#define DEBUG

Arduboy2 adb;

const unsigned char TILE_WIDTH = 32;
const unsigned char TILE_HEIGHT = 16;
const unsigned char WORLD_WIDTH = 20;
const unsigned char WORLD_HEIGHT = 16;
const unsigned char PLAYER_WIDTH = 16;
const unsigned char PLAYER_HEIGHT = 16;
const unsigned char Z_STEP = 3;
const unsigned char Z_LEVELS = 5;
const unsigned char DRAW_PADDING = 7;

enum GameStatus {
  Title,
  InGame,
  GameOver,
};
GameStatus gameStatus = InGame;

int xCam;
int yCam;

//lookup table for jumps
const char jumpLUT[] PROGMEM = {1, 2, 3, 3, 4, 4, 5, 5, 6, 6, 6, 5, 5, 4, 4, 3, 3, 2, 1, 0, -2, -4, -6, -9, -12, -15, -64};

////////////////////////////////////////////////////////////////////
/*                          TILE DATA                             */
////////////////////////////////////////////////////////////////////

const unsigned char tile_mask[] PROGMEM = {
  0x80, 0x80, 0xc0, 0xc0, 0xe0, 0xe0, 0xf0, 0xf0,
  0xf8, 0xf8, 0xfc, 0xfc, 0xfe, 0xfe, 0xff, 0xff,
  0xff, 0xff, 0xfe, 0xfe, 0xfc, 0xfc, 0xf8, 0xf8,
  0xf0, 0xf0, 0xe0, 0xe0, 0xc0, 0xc0, 0x80, 0x80,
  0x01, 0x01, 0x03, 0x03, 0x07, 0x07, 0x0f, 0x0f,
  0x1f, 0x1f, 0x3f, 0x3f, 0x7f, 0x7f, 0xff, 0xff,
  0xff, 0xff, 0x7f, 0x7f, 0x3f, 0x3f, 0x1f, 0x1f,
  0x0f, 0x0f, 0x07, 0x07, 0x03, 0x03, 0x01, 0x01
};

const unsigned char face_mask[] PROGMEM = {
  32, 24,
  0xfe, 0xfe, 0xfc, 0xfc, 0xf8, 0xf8, 0xf0, 0xf0,
  0xe0, 0xe0, 0xc0, 0xc0, 0x80, 0x80, 0x00, 0x00,
  0x00, 0x00, 0x80, 0x80, 0xc0, 0xc0, 0xe0, 0xe0,
  0xf0, 0xf0, 0xf8, 0xf8, 0xfc, 0xfc, 0xfe, 0xfe,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0x00, 0x00, 0x01, 0x01, 0x03, 0x03, 0x07, 0x07,
  0x0f, 0x0f, 0x1f, 0x1f, 0x3f, 0x3f, 0x7f, 0x7f,
  0x7f, 0x7f, 0x3f, 0x3f, 0x1f, 0x1f, 0x0f, 0x0f,
  0x07, 0x07, 0x03, 0x03, 0x01, 0x01, 0x00, 0x00,
};

const unsigned char tile_blank[] PROGMEM = {
  TILE_WIDTH, TILE_HEIGHT,
  0x80, 0x80, 0x40, 0x40, 0x20, 0x20, 0x10, 0x10,
  0x08, 0x08, 0x04, 0x04, 0x02, 0x02, 0x01, 0x01,
  0x01, 0x01, 0x02, 0x02, 0x04, 0x04, 0x08, 0x08,
  0x10, 0x10, 0x20, 0x20, 0x40, 0x40, 0x80, 0x80,
  0x01, 0x01, 0x02, 0x02, 0x04, 0x04, 0x08, 0x08,
  0x10, 0x10, 0x20, 0x20, 0x40, 0x40, 0x80, 0x80,
  0x80, 0x80, 0x40, 0x40, 0x20, 0x20, 0x10, 0x10,
  0x08, 0x08, 0x04, 0x04, 0x02, 0x02, 0x01, 0x01
};

const unsigned char tile_grass[] PROGMEM = {
  TILE_WIDTH, TILE_HEIGHT,
  0x80, 0x80, 0x40, 0x40, 0x20, 0xa0, 0x10, 0x10,
  0x08, 0x88, 0x04, 0x14, 0x02, 0x02, 0x01, 0x21,
  0x05, 0x01, 0x42, 0x0a, 0x04, 0x84, 0x08, 0x08,
  0x10, 0x90, 0x20, 0x20, 0x40, 0x40, 0x80, 0x80,
  0x01, 0x01, 0x02, 0x02, 0x04, 0x04, 0x08, 0x08,
  0x10, 0x10, 0x20, 0x24, 0x40, 0x40, 0x80, 0x82,
  0xa0, 0x80, 0x40, 0x40, 0x28, 0x20, 0x10, 0x10,
  0x08, 0x08, 0x04, 0x04, 0x02, 0x02, 0x01, 0x01
};

const unsigned char tile_water[] PROGMEM = {
  TILE_WIDTH, TILE_HEIGHT,

  0x80, 0x80, 0x40, 0x40, 0xa0, 0xa0, 0xd0, 0xd0,
  0xe8, 0xe8, 0xf4, 0xf4, 0xfa, 0xda, 0xdd, 0xed,
  0xed, 0xdd, 0xda, 0xfa, 0xf4, 0xf4, 0xe8, 0xe8,
  0xd0, 0xd0, 0xa0, 0xa0, 0x40, 0x40, 0x80, 0x80,
  0x01, 0x01, 0x02, 0x02, 0x05, 0x05, 0x0b, 0x0a,
  0x16, 0x15, 0x2d, 0x2e, 0x5e, 0x57, 0xb7, 0xbb,
  0xbb, 0xb7, 0x57, 0x5e, 0x2e, 0x2d, 0x15, 0x16,
  0x0a, 0x0b, 0x05, 0x05, 0x02, 0x02, 0x01, 0x01,

  0x80, 0x80, 0x40, 0x40, 0xa0, 0xa0, 0xd0, 0xd0,
  0xe8, 0xe8, 0xf4, 0xf4, 0xfa, 0xda, 0xdd, 0xdd,
  0xdd, 0xdd, 0xda, 0xfa, 0xf4, 0xf4, 0xe8, 0xe8,
  0xd0, 0xd0, 0xa0, 0xa0, 0x40, 0x40, 0x80, 0x80,
  0x01, 0x01, 0x02, 0x02, 0x05, 0x05, 0x0b, 0x0a,
  0x16, 0x16, 0x2e, 0x2e, 0x5e, 0x57, 0xb7, 0xb7,
  0xb7, 0xb7, 0x57, 0x5e, 0x2e, 0x2e, 0x16, 0x16,
  0x0a, 0x0b, 0x05, 0x05, 0x02, 0x02, 0x01, 0x01,

  0x80, 0x80, 0x40, 0x40, 0xa0, 0xa0, 0xd0, 0xd0,
  0xe8, 0x68, 0x74, 0xf4, 0xfa, 0xda, 0xdd, 0xbd,
  0xbd, 0xdd, 0xda, 0xfa, 0xf4, 0x74, 0x68, 0xe8,
  0xd0, 0xd0, 0xa0, 0xa0, 0x40, 0x40, 0x80, 0x80,
  0x01, 0x01, 0x02, 0x02, 0x05, 0x05, 0x0b, 0x0a,
  0x16, 0x17, 0x2f, 0x2e, 0x5e, 0x57, 0xb7, 0xaf,
  0xaf, 0xb7, 0x57, 0x5e, 0x2e, 0x2f, 0x17, 0x16,
  0x0a, 0x0b, 0x05, 0x05, 0x02, 0x02, 0x01, 0x01,
};

const unsigned char *tiles[3] = {
  tile_blank, tile_grass, tile_water
};

////////////////////////////////////////////////////////////////////
/*                        WORLD DATA                              */
////////////////////////////////////////////////////////////////////


enum Tl {
  Blnk,
  Grss,
  Watr,
};

const unsigned char world[WORLD_HEIGHT][WORLD_WIDTH] PROGMEM = {
  /*0*/  {Tl::Grss, Tl::Grss, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Watr, Tl::Watr, Tl::Watr, Tl::Watr, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk  },
  /*1*/  {Tl::Grss, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Watr, Tl::Watr, Tl::Watr, Tl::Watr, Tl::Watr, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Grss, Tl::Grss, Tl::Blnk  },
  /*2*/  {Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Watr, Tl::Watr, Tl::Watr, Tl::Watr, Tl::Watr, Tl::Watr, Tl::Watr, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk  },
  /*3*/  {Tl::Watr, Tl::Watr, Tl::Watr, Tl::Watr, Tl::Blnk, Tl::Watr, Tl::Watr, Tl::Watr, Tl::Watr, Tl::Watr, Tl::Watr, Tl::Watr, Tl::Watr, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk  },
  /*4*/  {Tl::Blnk, Tl::Watr, Tl::Watr, Tl::Watr, Tl::Blnk, Tl::Watr, Tl::Watr, Tl::Watr, Tl::Watr, Tl::Watr, Tl::Watr, Tl::Watr, Tl::Watr, Tl::Watr, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk  },
  /*5*/  {Tl::Grss, Tl::Blnk, Tl::Watr, Tl::Watr, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Watr, Tl::Watr, Tl::Watr, Tl::Watr, Tl::Watr, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk  },
  /*6*/  {Tl::Grss, Tl::Blnk, Tl::Watr, Tl::Blnk, Tl::Grss, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Watr, Tl::Watr, Tl::Watr, Tl::Watr, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Grss  },
  /*7*/  {Tl::Grss, Tl::Blnk, Tl::Blnk, Tl::Watr, Tl::Blnk, Tl::Grss, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Watr, Tl::Watr, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Grss  },
  /*8*/  {Tl::Blnk, Tl::Blnk, Tl::Watr, Tl::Watr, Tl::Blnk, Tl::Grss, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Watr, Tl::Watr, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Grss  },
  /*9*/  {Tl::Grss, Tl::Blnk, Tl::Blnk, Tl::Watr, Tl::Blnk, Tl::Grss, Tl::Grss, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Watr, Tl::Watr, Tl::Watr, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk  },
  /*a*/  {Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Watr, Tl::Watr, Tl::Blnk, Tl::Grss, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Watr, Tl::Watr, Tl::Watr, Tl::Watr, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk  },
  /*b*/  {Tl::Blnk, Tl::Blnk, Tl::Watr, Tl::Watr, Tl::Watr, Tl::Watr, Tl::Blnk, Tl::Watr, Tl::Watr, Tl::Blnk, Tl::Watr, Tl::Watr, Tl::Watr, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk  },
  /*c*/  {Tl::Blnk, Tl::Watr, Tl::Watr, Tl::Watr, Tl::Watr, Tl::Watr, Tl::Watr, Tl::Watr, Tl::Watr, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk  },
  /*d*/  {Tl::Watr, Tl::Watr, Tl::Watr, Tl::Watr, Tl::Watr, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Grss, Tl::Blnk, Tl::Grss, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk  },
  /*e*/  {Tl::Watr, Tl::Watr, Tl::Watr, Tl::Watr, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Grss  },
  /*f*/  {Tl::Watr, Tl::Watr, Tl::Watr, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Grss, Tl::Blnk, Tl::Grss, Tl::Blnk, Tl::Blnk, Tl::Blnk, Tl::Grss, Tl::Blnk, Tl::Grss, Tl::Blnk  },
};

//values in the heightmap should lie between 0 and Z_LEVELS-1
const unsigned char heightmap[WORLD_HEIGHT][WORLD_WIDTH] PROGMEM = {
  /*0*/  {4, 4, 3, 2, 2, 1, 0, 0, 0, 0, 1, 1, 2, 2, 1, 1, 2, 2, 3, 1 },
  /*1*/  {4, 1, 3, 3, 1, 1, 0, 0, 0, 0, 0, 2, 3, 2, 1, 1, 3, 4, 4, 2 },
  /*2*/  {2, 3, 1, 2, 1, 0, 0, 0, 0, 0, 0, 0, 2, 1, 1, 1, 1, 1, 1, 1 },
  /*3*/  {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 1, 1, 1, 1, 2 },
  /*4*/  {2, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 3, 1, 2, 2, 3 },
  /*5*/  {4, 2, 0, 0, 2, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 2, 1, 1, 2, 2 },
  /*6*/  {4, 2, 0, 1, 4, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 2, 4 },
  /*7*/  {4, 1, 1, 0, 3, 4, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 2, 2, 4 },
  /*8*/  {3, 1, 0, 0, 3, 4, 2, 2, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 2, 4 },
  /*9*/  {4, 2, 1, 0, 2, 4, 4, 3, 2, 1, 1, 1, 0, 0, 0, 2, 1, 1, 2, 3 },
  /*a*/  {3, 2, 1, 0, 0, 2, 4, 2, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 3 },
  /*b*/  {3, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 1, 1, 2, 1, 1, 2 },
  /*c*/  {3, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1 },
  /*d*/  {0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 4, 1, 4, 1, 1, 1, 1, 2, 3, 3 },
  /*e*/  {0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 2, 3, 3, 3, 4 },
  /*f*/  {0, 0, 0, 1, 1, 1, 1, 2, 3, 2, 4, 1, 4, 1, 2, 2, 4, 3, 4, 3 },
};

////////////////////////////////////////////////////////////////////
/*                        SPRITE DATA                             */
////////////////////////////////////////////////////////////////////

const unsigned char sprite_NE[] PROGMEM = {
  16, 16,
  0x00, 0x00, 0x00, 0xb8, 0x44, 0xba, 0xba, 0xba,
  0xba, 0xba, 0x3a, 0xc4, 0x38, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x0f, 0x30, 0x67, 0x57, 0x27,
  0x6f, 0x8f, 0xa0, 0x7d, 0x05, 0x02, 0x00, 0x00,

  0x00, 0x00, 0x00, 0xb8, 0x44, 0xba, 0xba, 0xba,
  0xba, 0xba, 0x3a, 0xc4, 0xb8, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x0f, 0x30, 0x47, 0x57, 0x37,
  0x2f, 0x4f, 0xa0, 0x5d, 0x26, 0x01, 0x00, 0x00,
};

const unsigned char sprite_NE_mask[] PROGMEM = {
  0x00, 0x00, 0x00, 0xb8, 0xfc, 0xfe, 0xfe, 0xfe,
  0xfe, 0xfe, 0xfe, 0xfc, 0x38, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x0f, 0x3f, 0x7f, 0x7f, 0x3f,
  0x7f, 0xff, 0xff, 0x7f, 0x07, 0x02, 0x00, 0x00,

  0x00, 0x00, 0x00, 0xb8, 0xfc, 0xfe, 0xfe, 0xfe,
  0xfe, 0xfe, 0xfe, 0xfc, 0xb8, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x0f, 0x3f, 0x7f, 0x7f, 0x3f,
  0x3f, 0x7f, 0xff, 0x7f, 0x27, 0x01, 0x00, 0x00,
};

const unsigned char sprite_SE[] PROGMEM = {
  16, 16,
  0x00, 0x00, 0x00, 0x38, 0xc4, 0x3a, 0xba, 0xaa,
  0xba, 0xaa, 0xba, 0x44, 0xb8, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x0f, 0x30, 0x67, 0x54,
  0x25, 0x6f, 0x8f, 0xa0, 0x7d, 0x05, 0x02, 0x00,

  0x00, 0x00, 0x00, 0x38, 0xc4, 0x3a, 0xba, 0xaa,
  0xba, 0xaa, 0xba, 0x44, 0xb8, 0x80, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x0f, 0x30, 0x47, 0x54,
  0x33, 0x2f, 0x4f, 0xa0, 0x5d, 0x22, 0x01, 0x00,
};

const unsigned char sprite_SE_mask[] PROGMEM = {
  0x00, 0x00, 0x00, 0x38, 0xfc, 0xfe, 0xfe, 0xfe,
  0xfe, 0xfe, 0xfe, 0xfc, 0xb8, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x0f, 0x3f, 0x7f, 0x7f,
  0x3f, 0x7f, 0xff, 0xff, 0x7f, 0x07, 0x02, 0x00,

  0x00, 0x00, 0x00, 0x38, 0xfc, 0xfe, 0xfe, 0xfe,
  0xfe, 0xfe, 0xfe, 0xfc, 0xb8, 0x80, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x0f, 0x3f, 0x7f, 0x7f,
  0x3f, 0x3f, 0x7f, 0xff, 0x7f, 0x23, 0x01, 0x00,
};

const unsigned char sprite_SW[] PROGMEM = {
  16, 16,
  0x00, 0x00, 0x00, 0xb8, 0x44, 0xba, 0xaa, 0xba,
  0xaa, 0xba, 0x3a, 0xc4, 0x38, 0x00, 0x00, 0x00,
  0x00, 0x02, 0x05, 0x7d, 0xa0, 0x8f, 0x6f, 0x25,
  0x54, 0x67, 0x30, 0x0f, 0x00, 0x00, 0x00, 0x00,

  0x00, 0x00, 0x80, 0xb8, 0x44, 0xba, 0xaa, 0xba,
  0xaa, 0xba, 0x3a, 0xc4, 0x38, 0x00, 0x00, 0x00,
  0x00, 0x01, 0x22, 0x5d, 0xa0, 0x4f, 0x2f, 0x33,
  0x54, 0x47, 0x30, 0x0f, 0x00, 0x00, 0x00, 0x00,
};

const unsigned char sprite_SW_mask[] PROGMEM = {
  0x00, 0x00, 0x00, 0xb8, 0xfc, 0xfe, 0xfe, 0xfe,
  0xfe, 0xfe, 0xfe, 0xfc, 0x38, 0x00, 0x00, 0x00,
  0x00, 0x02, 0x07, 0x7f, 0xff, 0xff, 0x7f, 0x3f,
  0x7f, 0x7f, 0x3f, 0x0f, 0x00, 0x00, 0x00, 0x00,

  0x00, 0x00, 0x80, 0xb8, 0xfc, 0xfe, 0xfe, 0xfe,
  0xfe, 0xfe, 0xfe, 0xfc, 0x38, 0x00, 0x00, 0x00,
  0x00, 0x01, 0x23, 0x7f, 0xff, 0x7f, 0x3f, 0x3f,
  0x7f, 0x7f, 0x3f, 0x0f, 0x00, 0x00, 0x00, 0x00,
};

const unsigned char sprite_NW[] PROGMEM = {
  16, 16,
  0x00, 0x00, 0x00, 0x38, 0xc4, 0x3a, 0xba, 0xba,
  0xba, 0xba, 0xba, 0x44, 0xb8, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x02, 0x05, 0x7d, 0xa0, 0x8f, 0x6f,
  0x27, 0x57, 0x67, 0x30, 0x0f, 0x00, 0x00, 0x00,

  0x00, 0x00, 0x00, 0xb8, 0xc4, 0x3a, 0xba, 0xba,
  0xba, 0xba, 0xba, 0x44, 0xb8, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x01, 0x26, 0x5d, 0xa0, 0x4f, 0x2f,
  0x37, 0x57, 0x47, 0x30, 0x0f, 0x00, 0x00, 0x00,
};

const unsigned char sprite_NW_mask[] PROGMEM = {
  0x00, 0x00, 0x00, 0x38, 0xfc, 0xfe, 0xfe, 0xfe,
  0xfe, 0xfe, 0xfe, 0xfc, 0xb8, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x02, 0x07, 0x7f, 0xff, 0xff, 0x7f,
  0x3f, 0x7f, 0x7f, 0x3f, 0x0f, 0x00, 0x00, 0x00,

  0x00, 0x00, 0x00, 0xb8, 0xfc, 0xfe, 0xfe, 0xfe,
  0xfe, 0xfe, 0xfe, 0xfc, 0xb8, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x01, 0x27, 0x7f, 0xff, 0x7f, 0x3f,
  0x3f, 0x7f, 0x7f, 0x3f, 0x0f, 0x00, 0x00, 0x00,
};

////////////////////////////////////////////////////////////////////
/*                 MOVABLE OBJECT CLASS                           */
////////////////////////////////////////////////////////////////////

class MoveableObject {
  public:
    int x;
    int y;
    unsigned char z;
    unsigned char heightAtOrigin = 0;
    unsigned char *sprites[4];
    unsigned char *sprites_masks[4];
    unsigned char animationFrame = 0;
    bool isJump = false;
    unsigned char jumpFrame = 0;

    //convert pixelwise (x,y)-coordinates into isometric tile coordinates (automatic round-off)
    char getXTileCoords() {
      return (this->x + 2 * this->y) / 32;
    }
    char getYTileCoords() {
      return (2 * this->y - this->x) / 32;
    }

    //get current height of tile the MOB is standing on
    char getZCoords() {
      return Z_STEP * pgm_read_byte(&heightmap[this->getYTileCoords()][this->getXTileCoords()]);
    }

    //calculate height difference between current MOB position and some (targetX,targetY)
    char getHeightDiff(int targetX, int targetY) {
      return Z_STEP * (pgm_read_byte(&heightmap[(2 * this->y - this->x) / 32][(this->x + 2 * this->y) / 32]) - pgm_read_byte(&heightmap[(2 * targetY - targetX) / 32][(targetX + 2 * targetY) / 32]));
    }

    //attempt to move one step (+deltaX, +deltaY) ahead, return true if attempt succeeds, otherwise return false
    bool moveIntoNextTile(char deltaX, char deltaY) {
      //return if MOB would move outside the playing field
      if ((((this->x) + deltaX) + 2 * ((this->y) + deltaY) < 0) ||
          (((this->x) + deltaX) + 2 * ((this->y) + deltaY) >= WORLD_WIDTH * TILE_WIDTH) ||
          (2 * ((this->y) + deltaY) - ((this->x) + deltaX) < 0) ||
          (2 * ((this->y) + deltaY) - ((this->x) + deltaX) >= WORLD_HEIGHT * TILE_WIDTH)) {
        return false;
      }
      //calculate z-level difference between current position and next
      char heightDiff = this->getHeightDiff(this->x + deltaX, this->y + deltaY);

      //if there is a ledge and the MOB is not jumping, initiate fall sequence by starting the jump sequence towards the end
      if (heightDiff > 0 && !this->isJump) {
        this->isJump = true;
        this->heightAtOrigin = this->getZCoords();
        this->jumpFrame = 19; // = 0
      }

      //move into the next tile if either the height difference is at most one Z_STEP or is at most zero when jumping
      if ((!this->isJump && heightDiff >= -Z_STEP) || (this->isJump && (this->getZCoords() - heightDiff <= this->z))) {
        this->x += deltaX;
        this->y += deltaY;
        return true;
      }
      return false;
    }

    //cycle through animation frames
    void swapAnimationFrame() {
      if (adb.everyXFrames(3)) {
        if (this->animationFrame == 0) {
          this->animationFrame = 1;
        }
        else this->animationFrame = 0;
      }
    }

    //draw moveable object
    void drawMOB(int spriteID) {
      char zOffset = 0;

      //handle jump
      if (this->isJump) {

        //if this is the first frame of the jump, memorize initial height
        if (this->jumpFrame == 0) {
          this->heightAtOrigin = this->getZCoords();
        }

        //calculate jump height to be drawn
        zOffset = pgm_read_byte(&jumpLUT[this->jumpFrame]) - (this->getZCoords() - this->heightAtOrigin);
        this->jumpFrame++;

        //check if MOB has landed, which ends the jump
        if (zOffset <= 0) {
          this->jumpFrame = 0;
          this->isJump = false;
          zOffset = 0;
        }
      }

      //calculate z-coordinate, taking current jump-height and world height into account
      this->z = pgm_read_byte(&heightmap[this->getYTileCoords()][this->getXTileCoords()]) * Z_STEP + zOffset;

      //actually draw the mob
      Sprites::drawExternalMask(this->x + xCam + WIDTH / 2 - PLAYER_WIDTH / 2,
                                this->y + yCam - PLAYER_HEIGHT - this->z,
                                this->sprites[spriteID],
                                this->sprites_masks[spriteID],
                                this->animationFrame,
                                this->animationFrame);
    }
};

////////////////////////////////////////////////////////////////////
/*                     PLAYER CLASS                               */
////////////////////////////////////////////////////////////////////

class Player: public MoveableObject {
  public:
    enum PlayerSprite {
      NEspr,
      SEspr,
      SWspr,
      NWspr,
    };
    PlayerSprite playerSprite = PlayerSprite::NEspr;

    Player() {
      sprites[0] = sprite_NE;
      sprites[1] = sprite_SE;
      sprites[2] = sprite_SW;
      sprites[3] = sprite_NW;

      sprites_masks[0] = sprite_NE_mask;
      sprites_masks[1] = sprite_SE_mask;
      sprites_masks[2] = sprite_SW_mask;
      sprites_masks[3] = sprite_NW_mask;

    }
};
Player player;

////////////////////////////////////////////////////////////////////
/*                  DRAW WORLD AND ITS CONTENTS                   */
////////////////////////////////////////////////////////////////////


//have the camera move by deltaX, deltaY
void moveCamera(char deltaX, char deltaY) {
  xCam -= deltaX;
  yCam -= deltaY;
}

//draw the world
void drawWorld() {

  static unsigned char frame = 0;
  static unsigned char wtrFrame = 1;
  static bool wtrRising = true;

  //water animation
  if (adb.everyXFrames(30)) {
    if (wtrRising) {
      wtrFrame++;
      if (wtrFrame == 2) {
        wtrRising = false;
      }
    } else {
      wtrFrame--;
      if (wtrFrame == 0) {
        wtrRising = true;
      }
    }
  }

  //center tile of drawing
  char ctrXTile = player.getXTileCoords();
  char ctrYTile = player.getYTileCoords();

  //initialize deltaZ
  static char deltaZleft = 0;
  static char deltaZright = 0;
  static char deltaZbottom = 0;

  //consider only tiles ±5 in every direction from center for drawing
  for (char yTile = max(ctrYTile - 5, 0); yTile < min(ctrYTile + 5, WORLD_HEIGHT); yTile++) {
    for (char xTile = max(ctrXTile - 5, 0); xTile < min(ctrXTile + 5, WORLD_WIDTH); xTile++) {

      //calculate upper left x and y coordinates on the screen for the current tile's drawing process
      char xDraw = ((xTile - yTile + 1) * TILE_WIDTH / 2) + (HEIGHT / 2) + xCam;
      char yDraw = ((xTile + yTile) * TILE_HEIGHT / 2 + yCam) - pgm_read_byte(&heightmap[yTile][xTile]) * Z_STEP;

      //if tile is outside drawing range, continue to next tile
      if ((xDraw > WIDTH) || (xDraw < -TILE_WIDTH) || (yDraw > HEIGHT) || (yDraw < -TILE_HEIGHT - DRAW_PADDING)) {
        continue;
      }

      //if tile is a water tile, get its animation frame
      if ((pgm_read_byte(&world[yTile][xTile]) == Tl::Watr)) {
        frame = wtrFrame;
      } else {
        frame = 0;
      }

      //draw map tile
      Sprites::drawExternalMask(xDraw,
                                yDraw,
                                tiles[pgm_read_byte(&world[yTile][xTile])],
                                tile_mask,
                                frame,
                                0);

      //draw vertical lines in front of tiles with nonzero height where appropriate (taking neighboring tiles into acct)
      if (pgm_read_byte(&heightmap[yTile][xTile]) > 0) {

        //mask out faces, so tiles drawn earlier don't "shine through"
        Sprites::drawErase(xDraw,
                           yDraw + TILE_HEIGHT / 2,
                           face_mask,
                           0);

        //calculate length of vertical lines according to lower left([+1][+0]), lower right([+0][+1]) and lower central ([+1][+1]) neighbors
        //max() ensures that lines only get drawn downward
        //ensure we only use the heightmap-value for the tiles on the bottom-left/bottom-right instead of the difference to the neighbors
        if ((yTile < (WORLD_HEIGHT - 1)) && (xTile < (WORLD_WIDTH - 1))) {
          deltaZleft = max((pgm_read_byte(&heightmap[yTile][xTile]) - pgm_read_byte(&heightmap[yTile + 1][xTile])), 0);
          deltaZright = max((pgm_read_byte(&heightmap[yTile][xTile]) - pgm_read_byte(&heightmap[yTile][xTile + 1])), 0);
          deltaZbottom = max((pgm_read_byte(&heightmap[yTile][xTile]) - pgm_read_byte(&heightmap[yTile + 1][xTile + 1])), 0);
          //handle bottom left and bottom right sides separately
        } else {
          deltaZleft = pgm_read_byte(&heightmap[yTile][xTile]);
          deltaZright = pgm_read_byte(&heightmap[yTile][xTile]);
          deltaZbottom = pgm_read_byte(&heightmap[yTile][xTile]);
        }
        //draw the vertical lines at the appropriate length
        adb.drawFastVLine(xDraw,
                          yDraw + (TILE_HEIGHT / 2),
                          Z_STEP * deltaZleft);

        adb.drawFastVLine(xDraw + TILE_WIDTH - 1,
                          yDraw + (TILE_HEIGHT / 2),
                          Z_STEP * deltaZright);

        adb.drawFastVLine(xDraw + (TILE_WIDTH / 2) - 1,
                          yDraw + TILE_HEIGHT,
                          Z_STEP * deltaZbottom);

        adb.drawFastVLine(xDraw + (TILE_WIDTH / 2),
                          yDraw + TILE_HEIGHT,
                          Z_STEP * deltaZbottom);
      }

      //if player is on this tile, draw player
      //in a future version, the program may cycle through a list of all MOBs to draw on the playing field here
      if (xTile == player.getXTileCoords() && yTile == player.getYTileCoords()) {
        player.drawMOB(player.playerSprite);
      }
    }
  }
  //draw diagonal lines at the very left and right bottom of the world map
  adb.drawLine(((0 - WORLD_HEIGHT + 2) * TILE_WIDTH / 2) + (HEIGHT / 2) + xCam,
               ((0 + WORLD_HEIGHT) * TILE_HEIGHT / 2 + yCam),
               ((WORLD_WIDTH - WORLD_HEIGHT + 2) * TILE_WIDTH / 2) + (HEIGHT / 2) + xCam - 1 ,
               (WORLD_WIDTH + WORLD_HEIGHT) * TILE_HEIGHT / 2 + yCam - 1,
               WHITE);
  adb.drawLine(((WORLD_WIDTH + 2) * TILE_WIDTH / 2) + (HEIGHT / 2) + xCam - 1,
               ((WORLD_WIDTH) * TILE_HEIGHT / 2 + yCam),
               ((WORLD_WIDTH - WORLD_HEIGHT + 2) * TILE_WIDTH / 2) + (HEIGHT / 2) + xCam,
               (WORLD_WIDTH + WORLD_HEIGHT) * TILE_HEIGHT / 2 + yCam - 1,
               WHITE);
}

////////////////////////////////////////////////////////////////////
/*                     PLAYER INPUT                               */
////////////////////////////////////////////////////////////////////

void playerInput() {
  if (adb.pressed(LEFT_BUTTON) && adb.notPressed(RIGHT_BUTTON) && adb.notPressed(UP_BUTTON) && adb.notPressed(DOWN_BUTTON)) {
    if (player.moveIntoNextTile(-2, -1)) {
      moveCamera(-2, -1);
    }
    player.playerSprite = player.PlayerSprite::NWspr;
    player.swapAnimationFrame();
  }
  if (adb.pressed(RIGHT_BUTTON) && adb.notPressed(LEFT_BUTTON) && adb.notPressed(UP_BUTTON) && adb.notPressed(DOWN_BUTTON)) {
    if (player.moveIntoNextTile(+2, +1)) {
      moveCamera(+2, +1);
    }
    player.playerSprite = player.PlayerSprite::SEspr;
    player.swapAnimationFrame();
  }
  if (adb.pressed(UP_BUTTON) && adb.notPressed(RIGHT_BUTTON) && adb.notPressed(LEFT_BUTTON) && adb.notPressed(DOWN_BUTTON)) {
    if (player.moveIntoNextTile(+2, -1)) {
      moveCamera(+2, -1);
    }
    player.playerSprite = player.PlayerSprite::NEspr;
    player.swapAnimationFrame();
  }
  if (adb.pressed(DOWN_BUTTON) && adb.notPressed(RIGHT_BUTTON) && adb.notPressed(UP_BUTTON) && adb.notPressed(LEFT_BUTTON)) {
    if (player.moveIntoNextTile(-2, +1)) {
      moveCamera(-2, +1);
    }
    player.playerSprite = player.PlayerSprite::SWspr;
    player.swapAnimationFrame();
  }
#ifdef DEBUG
  if (adb.pressed(A_BUTTON)) {
    adb.fillRect(0, 0, 30, 39, BLACK);
    adb.setCursor(0, 0);
    adb.print(player.x);
    adb.setCursor(0, 7);
    adb.print(player.y);
    adb.setCursor(0, 15);
    adb.print(static_cast<int>(player.getXTileCoords()));
    adb.setCursor(0, 23);
    adb.print(static_cast<int>(player.getYTileCoords()));
    adb.setCursor(0, 31);
    adb.print(static_cast<int>(player.z));
  }
#endif
#ifdef DEBUG

static int cpuLoad = 0;

  if (adb.pressed(B_BUTTON)) {
    adb.fillRect(WIDTH - 20, 0, 20, 8, BLACK);
    adb.setCursor(WIDTH - 20, 0);
    if (adb.everyXFrames(30))
      cpuLoad = adb.cpuLoad();
    adb.print(cpuLoad);
  }
#endif
  if (adb.justPressed(B_BUTTON)) {
    player.isJump = true;
  }
}

////////////////////////////////////////////////////////////////////
/*                    ENTRY & PROGRAM LOOP                        */
////////////////////////////////////////////////////////////////////

void setup() {
  adb.begin();
  adb.initRandomSeed();
  adb.clear();
  adb.setFrameRate(30);

  //initialize some player-related variables
  player.x = 56;
  player.y = 56;
  player.z = player.getZCoords();
  xCam = 0 - player.x;
  yCam = 40 - player.y;
}

void loop() {
  if (!(adb.nextFrame())) return;
  adb.clear();
  adb.pollButtons();

  switch (gameStatus) {
    case GameStatus::Title:
      break;
    case GameStatus::InGame:
      drawWorld();
      playerInput();
      break;
    case GameStatus::GameOver:
      break;
  }
  adb.display();
}
