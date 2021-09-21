/*

  The purpose of this program is create a simple oscilloscope using an Arduino UNO and an LCD 128 x 64 display

  Code will create a PWM signal for display on the graph
  The display is a Balance world Inc 0.96" Inch Yellow and Blue I2c IIC Serial 128x64 Oled LCD Oled LED Module for Arduino


  Revisions
  rev     date        author      description
  1       12-24-2015  kasprzak    initial creation

  Pin connections
  Arduino   device
  Ground    LCD ground
  +5VDC     Vcc
  A0    input for data read (5 VDC limit)
  A1    input voltage to adjust PWM (optional used here for demo purposes)
  A2
  A3
  A4    SDA (if no SDA pin)
  A5    SCL (if not SCL pin)
  1
  2
  3
  4
  5
  6
  7
  8
  9     PWM output (optional used here for demo purposes, connect to A0)
  10
  11
  12
  13
  SDA   SDA
  SLC   SLC

  Graphics Libraries
  https://github.com/adafruit/Adafruit-GFX-Library
  https://github.com/adafruit/Adafruit_SSD1306

  display
  http://www.amazon.com/Balance-world-Yellow-Arduino-Display/dp/B00ZI01RO0/ref=sr_1_1?ie=UTF8&qid=1448860339&sr=8-1&keywords=balance+world+.96


*/

#include <Adafruit_SSD1306.h>

#define READ_PIN      A0
#define PWM_PIN       11
#define ADJ_PIN       A1
#define OLED_RESET    4

Adafruit_SSD1306 display(OLED_RESET);

byte data[128];
int x;
int scnt, pos, ecnt;
double y, pwmVolts;
unsigned long etime, stime, ttime, xtime;

void setup() {
  Serial.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  display.clearDisplay();
  display.display();

  //---------------------------------------------- Set PWM frequency for D5 & D6 -------------------------------

  //TCCR0B = TCCR0B & B11111000 | B00000001;    // set timer 0 divisor to     1 for PWM frequency of 62500.00 Hz
  //TCCR0B = TCCR0B & B11111000 | B00000010;    // set timer 0 divisor to     8 for PWM frequency of  7812.50 Hz
  //TCCR0B = TCCR0B & B11111000 | B00000011;    // set timer 0 divisor to    64 for PWM frequency of   976.56 Hz (The DEFAULT)
  //TCCR0B = TCCR0B & B11111000 | B00000100;    // set timer 0 divisor to   256 for PWM frequency of   244.14 Hz
  //TCCR0B = TCCR0B & B11111000 | B00000101;    // set timer 0 divisor to  1024 for PWM frequency of    61.04 Hz


  //---------------------------------------------- Set PWM frequency for D9 & D10 ------------------------------

  //TCCR1B = TCCR1B & B11111000 | B00000001;    // set timer 1 divisor to     1 for PWM frequency of 31372.55 Hz
  //TCCR1B = TCCR1B & B11111000 | B00000010;    // set timer 1 divisor to     8 for PWM frequency of  3921.16 Hz
  //TCCR1B = TCCR1B & B11111000 | B00000011;    // set timer 1 divisor to    64 for PWM frequency of   490.20 Hz (The DEFAULT)
  //TCCR1B = TCCR1B & B11111000 | B00000100;    // set timer 1 divisor to   256 for PWM frequency of   122.55 Hz
  //TCCR1B = TCCR1B & B11111000 | B00000101;    // set timer 1 divisor to  1024 for PWM frequency of    30.64 Hz

  //---------------------------------------------- Set PWM frequency for D3 & D11 ------------------------------

  //TCCR2B = TCCR2B & B11111000 | B00000001;    // set timer 2 divisor to     1 for PWM frequency of 31372.55 Hz
  //TCCR2B = TCCR2B & B11111000 | B00000010;    // set timer 2 divisor to     8 for PWM frequency of  3921.16 Hz
  //TCCR2B = TCCR2B & B11111000 | B00000011;    // set timer 2 divisor to    32 for PWM frequency of   980.39 Hz
  TCCR2B = TCCR2B & B11111000 | B00000100;    // set timer 2 divisor to    64 for PWM frequency of   490.20 Hz (The DEFAULT)
  //TCCR2B = TCCR2B & B11111000 | B00000101;    // set timer 2 divisor to   128 for PWM frequency of   245.10 Hz
  //TCCR2B = TCCR2B & B11111000 | B00000110;    // set timer 2 divisor to   256 for PWM frequency of   122.55 Hz
  //TCCR2B = TCCR2B & B11111000 | B00000111;    // set timer 2 divisor to  1024 for PWM frequency of    30.64 Hz

  pinMode(READ_PIN, INPUT);
  pinMode(ADJ_PIN, INPUT);
  pinMode(PWM_PIN, OUTPUT);

}

void loop() {

  pwmVolts = analogRead(ADJ_PIN) / 4 ;
  analogWrite(PWM_PIN, pwmVolts);


  // find the first rise time
  scnt = 0;
  ecnt = 0;
  etime = 0;
  stime = 0;
  while (scnt++ < 400) {
    pos = analogRead(READ_PIN);
    if (pos == 0) {
      while (ecnt++ < 400) {
        pos = analogRead(READ_PIN);
        if (pos != 0) {
          stime = micros();
          break;
        }
      }
      break;
    }
  }

  // compute the time to find the next rise time and deduct--an attempt to make the Hx more accurate
  xtime = micros() - stime;
  // find the second rise time
  scnt = 0;
  ecnt = 0;
  while (scnt++ < 400) {
    pos = analogRead(READ_PIN);
    if (pos == 0) {
      while (ecnt++ < 400) {
        pos = analogRead(READ_PIN);
        if (pos != 0) {
          etime = micros();
          break;
        }
      }
      break;
    }
  }

  // compute times between rise times
  // note i use negative to force an error value to 0
  // many ways to do this...
  if (etime == 0 | stime == 0) {
    ttime = -1000000;
  }
  else {
    ttime = etime - stime - xtime;
  }

  // since we are at rise get the data
  // you may think to just draw the data as we get it but it's way too slow

  for (x = 10; x <= 127; x++) {
    data[x] = (analogRead(READ_PIN) / 21.7659);
  }



  display.clearDisplay();

  for (x = 10; x <= 127; x++) {
    display.drawLine(x , 63 - (data[x - 1]), x , 63 - (data[x]), WHITE);
  }
  // draw the axis, lables and tick marks
  for (int y = 0; y < 6; y++) {
    display.drawFastHLine(7, y * (48) / 5 + 16, 3, WHITE);
  }
  // don't forget to set the back color arguement otherwise numbers may draw on the previous number
  display.setTextColor(WHITE, BLACK);
  display.setTextSize(1);
  display.setCursor(0, 57);
  display.println("0");
  display.setCursor(0, 16);
  display.println("5");

  display.fillRect(0, 0,  127 , 14, WHITE);
  display.setTextColor(BLACK, WHITE);
  display.drawFastHLine(10, 63,  128 - 10, WHITE);
  display.drawFastVLine(10, 16,  63, WHITE);
  display.setTextSize(1);
  display.setCursor(2, 3);
  display.println("cheap-O-scope");


  // compute and write the frequency
  display.setCursor(85, 3);
  display.println(String(1000000 / ttime) + " Hz");
  

  // now that the display buffer is built, display it
  display.display();


}
