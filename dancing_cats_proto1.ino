#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

// Stepper stuff
#include <AccelStepper.h>
#define HALFSTEP 8

// Motor pin definitions
#define motorPin1  2     // IN1 on the ULN2003 driver 1
#define motorPin2  3     // IN2 on the ULN2003 driver 1
#define motorPin3  4     // IN3 on the ULN2003 driver 1
#define motorPin4  5     // IN4 on the ULN2003 driver 1

#define motorPin5  7     // IN1 on the ULN2003 driver 2
#define motorPin6  8     // IN2 on the ULN2003 driver 2
#define motorPin7  9     // IN3 on the ULN2003 driver 2
#define motorPin8  10    // IN4 on the ULN2003 driver 2

// Initialize with pin sequence IN1-IN3-IN2-IN4 for using the AccelStepper with 28BYJ-48
AccelStepper stepper1(HALFSTEP, motorPin1, motorPin3, motorPin2, motorPin4);
AccelStepper stepper2(HALFSTEP, motorPin5, motorPin7, motorPin6, motorPin8);

#define PIN 11
#define TIME_ON 30000

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(35, PIN, NEO_GRB + NEO_KHZ800);

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

void delayPoll ( int ms )
{
  uint32_t currTime = millis ( ) ;
  uint32_t targetTime = currTime + ms ;
  int interval = ms / 5 ;

  while ( currTime < targetTime ) 
  {
    stepper1.run();
    stepper2.run( ) ;
    delay ( interval ) ;
    currTime = millis ( ) ;
  }
}

void startSteppers ( )
{
  // Stepper settings
  stepper1.setMaxSpeed(1000.0);
  stepper1.setAcceleration(200.0);
  stepper1.setSpeed(900);
  stepper1.moveTo(900);

  stepper2.setMaxSpeed(1000.0);
  stepper2.setAcceleration(200.0);
  stepper2.setSpeed(900);
  stepper2.moveTo(600);
}

void stopSteppers ( )
{
  digitalWrite( motorPin1, LOW ) ;
  digitalWrite( motorPin2, LOW ) ;
  digitalWrite( motorPin3, LOW ) ;
  digitalWrite( motorPin4, LOW ) ;

  digitalWrite( motorPin5, LOW ) ;
  digitalWrite( motorPin6, LOW ) ;
  digitalWrite( motorPin7, LOW ) ;
  digitalWrite( motorPin8, LOW ) ;
  
}

void fadeOut ( ) 
{
  
}
static byte state = 0 ;  // Initially stopped, waiting for trigger
static uint32_t timer = 0 ;

void setup() {

  // Setup for control via serial port
  Serial.begin( 9600 ) ;
  
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  // Initialize steppers
  startSteppers ( ) ;

  state = 0 ;
  timer = millis ( ) ;

  Serial.println ( "Starting" ) ;
}

void loop() {
  static uint32_t targetTime = timer + TIME_ON ;
  if( state )
  {
    // Some example procedures showing how to display to the pixels:
    //colorWipe(strip.Color(255, 0, 0), 50); // Red
    //colorWipe(strip.Color(0, 255, 0), 50); // Green
    //colorWipe(strip.Color(0, 0, 255), 50); // Blue
    //colorWipe(strip.Color(0, 0, 0, 255), 50); // White RGBW
    // Send a theater pixel chase in...
    theaterChase(strip.Color(127, 0, 0), 50); // Red
    stepper1.run();
    stepper2.run( ) ;
    theaterChase(strip.Color(127, 127, 127), 50); // White
    stepper1.run();
    stepper2.run( ) ;
    theaterChase(strip.Color(0, 0, 127), 50); // Blue
    stepper1.run();
    stepper2.run( ) ;
  
    //rainbow(20);
    //rainbowCycle(20);
    //theaterChaseRainbow(50);
  
    //Change direction when the stepper reaches the target position
    if (stepper1.distanceToGo() == 0) {
      stepper1.moveTo(-stepper1.currentPosition());
    }
  
    if (stepper2.distanceToGo() == 0) {
      stepper2.moveTo(-stepper2.currentPosition());
    }
  
    stepper1.run();
    stepper2.run( ) ;

    timer = millis ( ) ;
    if( timer > targetTime )
    {
      stopSteppers ( ) ;
      colorWipe(strip.Color(128, 0, 0), 50);
      delay( 1000 ) ;
      colorWipe( strip.Color( 128, 128, 128 ), 50 ) ;
      delay( 1000 ) ;
      colorWipe( strip.Color( 0, 0, 128 ), 50 ) ;
      delay( 1000 ) ;
      colorWipe(strip.Color(0, 0, 0 ), 50 ) ;
      strip.show ( ) ;
      state = 0 ;
    }
  }
  else
  {
    // Wait for a character, or a sensor input to retrigger
    if( Serial.available ( ) )
    { 
      char c = Serial.read ( ) ;
      state = 1 ;
      timer = millis ( ) ;
      targetTime = timer + TIME_ON ;
      startSteppers ( ) ;
    }
  }
  
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    stepper1.run ( ) ;
    stepper2.run ( ) ;
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, c);    //turn every third pixel on
        stepper1.run ( ) ;
        stepper2.run ( ) ;
      }
      strip.show();
      stepper1.run ( ) ;
      stepper2.run ( ) ;
      delayPoll(wait);
      stepper1.run ( ) ;
      stepper2.run ( ) ;

      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
        stepper1.run ( ) ;
        stepper2.run ( ) ;
      }
      stepper1.run ( ) ;
      stepper2.run ( ) ;
    }
  }
}

/*
void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}
*/

//Theatre-style crawling lights with rainbow effect
/*
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}
*/


