#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include "fonts.h"
/*
 * --------SSD1306--------
 */
#define SSD1306
#define INTERFACE_I2C

#ifdef INTERFACE_4WIRE_SPI

#define SSD1306_GPIO_PIN_DC     125U

#define SSD1306_PIN_SET()   do {\
                                    GPIO_setPadConfig(SSD1306_GPIO_PIN_DC,  GPIO_PIN_TYPE_STD);\
                                    GPIO_setDirectionMode(SSD1306_GPIO_PIN_DC,  GPIO_DIR_MODE_OUT);\
                            } while (0);

#define SSD1306_DC_SET()    GPIO_writePin(SSD1306_GPIO_PIN_DC,  1);
#define SSD1306_DC_CLR()    GPIO_writePin(SSD1306_GPIO_PIN_DC,  0);

#define SSD1306_WRITE_BYTE(__DATA)  do {\
                                            SPI_writeDataBlockingNonFIFO(SPIA_BASE, __DATA << 8);\
                                    } while (0);

#elif defined(INTERFACE_3WIRE_SPI)

#define SSD1306_WRITE_BYTE(__DATA)  do {\
                                            SPI_writeDataBlockingNonFIFO(SPIA_BASE, __DATA << 7);\
                                    } while (0);

#elif defined(INTERFACE_I2C)

int fd;

#define SSD1306_ADDRESS 0x3c

#define SSD1306_WRITE_BYTE(__DATA)  write(fd, &__DATA, 2)

#endif

#define SSD1306_CMD    0
#define SSD1306_DATA   1

#define SSD1306_WIDTH    128
#define SSD1306_HEIGHT   64

#define SET_COL_START_ADDR() 	ssd1306_write_byte(0x00, SSD1306_CMD); ssd1306_write_byte(0x10, SSD1306_CMD)

static unsigned char s_chDispalyBuffer[128][8];

/**
  * @brief  Writes an byte to the display data ram or the command register
  *         
  * @param  chData: Data to be writen to the display data ram or the command register
  * @param chCmd:  
  *                           0: Writes to the command register
  *                           1: Writes to the display data ram
  * @retval None
**/
static void ssd1306_write_byte(unsigned char chData, unsigned char chCmd)
{
#ifdef INTERFACE_4WIRE_SPI

	if (chCmd == SSD1306_CMD) {
		SSD1306_DC_CLR();
	} else {
		SSD1306_DC_SET();
	}	

	SSD1306_WRITE_BYTE(chData);

#elif defined(INTERFACE_3WIRE_SPI)

	uint16_t temp = 0;

    if (chCmd == SSD1306_CMD) {
        temp = 0x0000;
    } else {
        temp = 0x0100;
    }

    temp += chData;

    SSD1306_WRITE_BYTE(temp);

#elif defined(INTERFACE_I2C)
	
	unsigned char temp[2] = {0};

	if (chCmd) {
		temp[0] = 0x40;
	} else {
	    temp[0] = 0x00;
	}

	temp[1] = chData;
	
	SSD1306_WRITE_BYTE(temp);
	
#endif
}   	  

/**
  * @brief  OLED turns on 
  *         
  * @param  None
  *         
  * @retval None
**/ 
void ssd1306_display_on(void)
{
	ssd1306_write_byte(0x8D, SSD1306_CMD);  
	ssd1306_write_byte(0x14, SSD1306_CMD);  
	ssd1306_write_byte(0xAF, SSD1306_CMD);  
}
   
/**
  * @brief  OLED turns off
  *         
  * @param  None
  *         
  * @retval  None
**/
void ssd1306_display_off(void)
{
	ssd1306_write_byte(0x8D, SSD1306_CMD);  
	ssd1306_write_byte(0x10, SSD1306_CMD); 
	ssd1306_write_byte(0xAE, SSD1306_CMD);  
}

/**
  * @brief  Refreshs the graphic ram
  *         
  * @param  None
  *         
  * @retval  None
**/

void ssd1306_refresh_gram(void)
{
	unsigned char i, j;
	
	for (i = 0; i < 8; i ++) {  
		ssd1306_write_byte(0xB0 + i, SSD1306_CMD);    
		SET_COL_START_ADDR();
		for (j = 0; j < 128; j ++) {
			ssd1306_write_byte(s_chDispalyBuffer[j][i], SSD1306_DATA);
		}
	}   
}

/**
  * @brief  Draws a piont on the screen
  *         
  * @param  chXpos: Specifies the X position
  * @param  chYpos: Specifies the Y position
  * @param  chPoint: 0: the point turns off    1: the piont turns on 
  *         
  * @retval None
**/

void ssd1306_draw_point(unsigned char chXpos, unsigned char chYpos, unsigned char chPoint)
{
    unsigned char chPos, chBx, chTemp = 0;

    if (chXpos > 127 || chYpos > 63) {
        return;
    }
    chPos = 7 - chYpos / 8; //
    chBx = chYpos % 8;
    chTemp = 1 << (7 - chBx);

    if (chPoint) {
        s_chDispalyBuffer[chXpos][chPos] |= chTemp;
    } else {
        s_chDispalyBuffer[chXpos][chPos] &=~chTemp;
    }
}

/**
  * @brief   Clears the screen
  *
  * @param  None
  *
  * @retval  None
**/

void ssd1306_clear_screen(unsigned char chFill)
{
	unsigned char i, j;

	for (i = 0; i < 8; i++) {
		for (j = 0; j < 128; j++) {
		    s_chDispalyBuffer[j][i] = chFill;
		}
	}
}

/**
  * @brief  Fills a rectangle
  *         
  * @param  chXpos1: Specifies the X position 1 (X top left position)
  * @param  chYpos1: Specifies the Y position 1 (Y top left position)
  * @param  chXpos2: Specifies the X position 2 (X bottom right position)
  * @param  chYpos3: Specifies the Y position 2 (Y bottom right position)
  *         
  * @retval 
**/

void ssd1306_fill_screen(unsigned char chXpos1, unsigned char chYpos1, unsigned char chXpos2, unsigned char chYpos2, unsigned char chDot)
{  
	unsigned char chXpos, chYpos;
	
	for (chXpos = chXpos1; chXpos <= chXpos2; chXpos ++) {
		for (chYpos = chYpos1; chYpos <= chYpos2; chYpos ++) {
			ssd1306_draw_point(chXpos, chYpos, chDot);
		}
	}
}

/**
  * @brief Displays one character at the specified position    
  *         
  * @param  chXpos: Specifies the X position
  * @param  chYpos: Specifies the Y position
  * @param  chSize: 
  * @param  chMode
  * @retval 
**/
void ssd1306_display_char(unsigned char chXpos, unsigned char chYpos, unsigned char chChr, unsigned char chFontIndex, unsigned char chMode)
{      	
	unsigned char i, j;
	unsigned char chTemp, chYpos0 = chYpos;
			   
    for (i = 0; i < fonts_width[chFontIndex] * ((fonts_height[chFontIndex] + 7) / 8); i ++) {
        if (chMode) {
            chTemp = 0x00;
        } else {
            chTemp = 0xff;
        }

        switch (chFontIndex) {
            case FONT_1206:
                if (chMode) {
                    chTemp = c_chFont1206[chChr-' '][i];
                } else {
                    chTemp = ~c_chFont1206[chChr-' '][i];
                }
                break;
            case FONT_1608:
                if (chMode) {
                    chTemp = c_chFont1608[chChr-' '][i];
                } else {
                    chTemp = ~c_chFont1608[chChr-' '][i];
                }
                break;
            case FONT_1616:
                if (chChr - '0' <= 10) {
                    if (chMode) {
                        chTemp = c_chFont1616[chChr-'0'][i];
                    } else {
                        chTemp = ~c_chFont1616[chChr-'0'][i];
                    }
                }
                break;
            case FONT_3216:
                if (chChr - '0' <= 10) {
                    if (chMode) {
                        chTemp = c_chFont3216[chChr-'0'][i];
                    } else {
                        chTemp = ~c_chFont3216[chChr-'0'][i];
                    }
                }
                break;
            default:
                break;
        }
        
        for (j = 0; j < 8; j ++) {
		    if (chTemp & 0x80) {
			    ssd1306_draw_point(chXpos, chYpos, 1);
		    } else {
			    ssd1306_draw_point(chXpos, chYpos, 0);
		    }
		    chTemp <<= 1;
		    chYpos ++;
            
		    if ((chYpos - chYpos0) == fonts_height[chFontIndex]) {
			    chYpos = chYpos0;
			    chXpos ++;
			    break;
		    }
	    }

    } 
}
static unsigned long _pow(unsigned char m, unsigned char n)
{
	unsigned long result = 1;
	while(n --) result *= m;    
	return result;
}	

void ssd1306_display_num(unsigned char chXpos, unsigned char chYpos, unsigned long chNum, unsigned char chLen, unsigned char chFontIndex, unsigned char chMode)
{         	
	unsigned char i;
	unsigned char chTemp, chShow = 0;
	
	for(i = 0; i < chLen; i ++) {
		chTemp = (chNum / _pow(10, chLen - i - 1)) % 10;
		if(chShow == 0 && i < (chLen - 1)) {
			if(chTemp == 0) {
				ssd1306_display_char(chXpos + fonts_width[chFontIndex] * i, chYpos, ' ', chFontIndex, 1);
				continue;
			} else {
				chShow = 1;
			}	 
		}
	 	ssd1306_display_char(chXpos + fonts_width[chFontIndex] * i, chYpos, chTemp + '0', chFontIndex, chMode);
	}
} 

/**
  * @brief  Displays a string on the screen
  *         
  * @param  chXpos: Specifies the X position
  * @param  chYpos: Specifies the Y position
  * @param  pchString: Pointer to a string to display on the screen 
  *         
  * @retval  None
**/
void ssd1306_display_string(unsigned char chXpos, unsigned char chYpos, const char *pchString, unsigned char chFontIndex, unsigned char chMode)
{
    while (*pchString != '\0') {       
        if (chXpos > (SSD1306_WIDTH - fonts_width[chFontIndex])) {
			chXpos = 0;
			chYpos += fonts_height[chFontIndex];
			if (chYpos > (SSD1306_HEIGHT - fonts_height[chFontIndex])) {
				chYpos = chXpos = 0;
				ssd1306_clear_screen(0x00);
			}
		}
		
        ssd1306_display_char(chXpos, chYpos, *pchString, chFontIndex, chMode);
        chXpos += fonts_width[chFontIndex];
        pchString++;
    }
}

void ssd1306_draw_bitmap(unsigned char chXpos, unsigned char chYpos, const unsigned char *pchBmp, unsigned char chWidth, unsigned char chHeight)
{
	unsigned int i, j, byteWidth = (chWidth + 7) / 8;
	
    for(j = 0; j < chHeight; j ++){
        for(i = 0; i < chWidth; i ++ ) {
            if(*(pchBmp + j * byteWidth + i / 8) & (128 >> (i & 7))) {
                ssd1306_draw_point(chXpos + i, chYpos + j, 1);
            }
        }
    }
}

/**
  * @brief  SSd1306 initialization
  *         
  * @param  None
  *         
  * @retval None
**/
void ssd1306_init(void)
{
#ifdef INTERFACE_4WIRE_SPI
    SSD1306_PIN_SET();
#elif defined(INTERFACE_I2C)
    fd = open("/dev/i2c-1", O_RDWR);
    ioctl(fd, I2C_SLAVE, SSD1306_ADDRESS);
#endif

	ssd1306_write_byte(0xAE, SSD1306_CMD);//--turn off oled panel
	ssd1306_write_byte(0x00, SSD1306_CMD);//---set low column address
	ssd1306_write_byte(0x10, SSD1306_CMD);//---set high column address
	ssd1306_write_byte(0x40, SSD1306_CMD);//--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
	ssd1306_write_byte(0x81, SSD1306_CMD);//--set contrast control register
	ssd1306_write_byte(0xCF, SSD1306_CMD);// Set SEG Output Current Brightness
	ssd1306_write_byte(0xA1, SSD1306_CMD);//--Set SEG/Column Mapping     
	ssd1306_write_byte(0xC0, SSD1306_CMD);//Set COM/Row Scan Direction   
	ssd1306_write_byte(0xA6, SSD1306_CMD);//--set normal display
	ssd1306_write_byte(0xA8, SSD1306_CMD);//--set multiplex ratio(1 to 64)
	ssd1306_write_byte(0x3f, SSD1306_CMD);//--1/64 duty
	ssd1306_write_byte(0xD3, SSD1306_CMD);//-set display offset	Shift Mapping RAM Counter (0x00~0x3F)
	ssd1306_write_byte(0x00, SSD1306_CMD);//-not offset
	ssd1306_write_byte(0xd5, SSD1306_CMD);//--set display clock divide ratio/oscillator frequency
	ssd1306_write_byte(0x80, SSD1306_CMD);//--set divide ratio, Set Clock as 100 Frames/Sec
	ssd1306_write_byte(0xD9, SSD1306_CMD);//--set pre-charge period
	ssd1306_write_byte(0xF1, SSD1306_CMD);//Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
	ssd1306_write_byte(0xDA, SSD1306_CMD);//--set com pins hardware configuration
	ssd1306_write_byte(0x12, SSD1306_CMD);
	ssd1306_write_byte(0xDB, SSD1306_CMD);//--set vcomh
	ssd1306_write_byte(0x40, SSD1306_CMD);//Set VCOM Deselect Level
	ssd1306_write_byte(0x20, SSD1306_CMD);//-Set Page Addressing Mode (0x00/0x01/0x02)
	ssd1306_write_byte(0x02, SSD1306_CMD);//
	ssd1306_write_byte(0x8D, SSD1306_CMD);//--set Charge Pump enable/disable
	ssd1306_write_byte(0x14, SSD1306_CMD);//--set(0x10) disable
	ssd1306_write_byte(0xA4, SSD1306_CMD);// Disable Entire Display On (0xa4/0xa5)
	ssd1306_write_byte(0xA6, SSD1306_CMD);// Disable Inverse Display On (0xa6/a7) 
	ssd1306_write_byte(0xAF, SSD1306_CMD);//--turn on oled panel
	
	ssd1306_clear_screen(0x00);
}
