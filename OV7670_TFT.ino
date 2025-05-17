#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>

#include "I2C.h"
#include "FifoCamera.h"

const int VSYNC = 21; //vertical sync
const int SIOD = 37; //SDA
const int SIOC = 36; //SCL

const int RRST = 48;  //read reset
const int WRST = 46;  //write reset
const int RCK = 47;    //read clock
const int WR = 0;     //write flag
//OE -> GND     (output enable always on since we control the read clock)
//PWDN not nonnected  
//HREF not connected
//STR not connected
//RST -> 3.3V 

const int D0 = 8;
const int D1 = 18;
const int D2 = 17;
const int D3 = 16;
const int D4 = 7;
const int D5 = 5;
const int D6 = 6;
const int D7 = 4;

#define TFT_CS    10
#define TFT_RST   15   // Nếu không có, đặt -1
#define TFT_DC    14
#define TFT_MOSI  12  // SPI MOSI
#define TFT_SCLK  13  // SPI Clock
//DIN <- MOSI 23
//CLK <- SCK 18

I2C<SIOD, SIOC> i2c;
FifoCamera<I2C<SIOD, SIOC>, RRST, WRST, RCK, WR, D0, D1, D2, D3, D4, D5, D6, D7> camera(i2c);

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);

#define QQVGA
//#define QQQVGA

#ifdef QQVGA
const int XRES = 160;
const int YRES = 128;
#endif
#ifdef QQQVGA
const int XRES = 80;
const int YRES = 60;
#endif

const int BYTES_PER_PIXEL = 2;
const int frameSize = XRES * YRES * BYTES_PER_PIXEL;
unsigned char frame[frameSize];

void setup() 
{
  Serial.begin(115200);
  Serial.println("Initialization...");
  i2c.init();
  camera.init();
  
  // Khởi tạo SPI (nếu cần)
  SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);
  
  #ifdef QQVGA
    camera.QQVGARGB565();
  #endif
  #ifdef QQQVGA
    camera.QQQVGARGB565();
  #endif
  
  //camera.QQVGAYUV();
  //camera.RGBRaw();
  //camera.testImage();
  
  pinMode(VSYNC, INPUT);
  Serial.println("start");
  tft.initR(INITR_BLACKTAB);
  tft.fillScreen(0);
}

void displayRGB565()
{
  tft.setAddrWindow(0, 0, YRES - 1, XRES - 1);
  int i = 0;
  for(int x = 0; x < XRES; x++)
    for(int y = 0; y < YRES; y++)
    {
      i = (y * XRES + x) << 1;
      tft.pushColor(frame[i] | (frame[i + 1] << 8));
      //tft.pushColor(((frame[i] | (frame[i + 1] << 8)) >> 1) & 0b111101111101111); //dimming to test for tft error
    }  
}

void testTFT() //a small tft test output showing errors on my tft with bright colors
{
  tft.setAddrWindow(0, 0, 31, 63);
  int i = 0;
  for(int y = 0; y < 64; y++)
    for(int x = 0; x < 32; x++)
      tft.pushColor(x | y << 5);
}

void displayY8()
{
  tft.setAddrWindow(0, 0, YRES - 1, XRES - 1);
  int i = 0;
  for(int x = 0; x < XRES; x++)
    for(int y = 0; y < YRES; y++)
    {
      i = y * XRES + x;
      unsigned char c = frame[i];
      unsigned short r = c >> 3;
      unsigned short g = c >> 2;
      unsigned short b = c >> 3;
      tft.pushColor(r << 11 | g << 5 | b);
    }  
}

void frameToSerial()
{
  int i = 0;
  Serial.println("var frame=[");
  for(int y = 0; y < YRES; y+=1)
  {
    i = y * XRES;
    for(int x = 0; x < XRES; x+=1)
    {
//Serial.print(frame[i + x], HEX);
      Serial.print(frame[i + x]);
      Serial.print(',');
    }
    Serial.println();
  }
  Serial.println("];");  
}

void loop() 
{
  while(!digitalRead(VSYNC));
  while(digitalRead(VSYNC));
  camera.prepareCapture();
  camera.startCapture();
  while(!digitalRead(VSYNC));
  camera.stopCapture();

  //color
  camera.readFrame(frame, XRES, YRES, BYTES_PER_PIXEL);
  displayRGB565();
  delay(200);
  //testTFT();
  
  //b/w 
  //camera.readFrameOnlySecondByte(frame, XRES, YRES);
  //displayY8();
}