#ifndef FONTS_H
#define FONTS_H

enum fonts_index{
    FONT_1206 = 0,
    FONT_1608 = 1,
    FONT_1616 = 2,
    FONT_3216 = 3
};

extern unsigned char fonts_height[];
extern unsigned char fonts_width[];

extern const unsigned char c_chFont1206[95][12];
extern const unsigned char c_chFont1608[95][16];
extern const unsigned char c_chFont1616[11][32];
extern const unsigned char c_chFont3216[11][64];
extern const unsigned char c_chBmp4016[96];
extern const unsigned char c_chSingal816[16];
extern const unsigned char c_chMsg816[16];
extern const unsigned char c_chBluetooth88[8];
extern const unsigned char c_chBat816[16];
extern const unsigned char c_chGPRS88[8];
extern const unsigned char c_chAlarm88[8];

#endif
