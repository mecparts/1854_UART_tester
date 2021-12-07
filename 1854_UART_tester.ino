// 1854_UART_tester: a quick and dirty test rig for a CDP1854 UART.
//
// D13  (PB5) is connected to RSEL
// D12  (PB4) is connected to /CLEAR (passively pulled down by h/w)
// D11  (PB3) is connected to RxCLK & TxCLK
// D10  (PB2) is connected to TPB (passively pulled down by h/w)
// D9   (PB1) is connected to RD/WR
// D8   (PB0) is connected to /CS2 (passively pulled up by h/w)
// D0-D7 (PD) is connected to D0-D7
//
// Set your terminal up for 9600 baud, 8 data bits, 1 stop bit, no parity.
//
#define RSEL_PIN       13
#define nCLEAR_PIN     12
#define CLOCK_PIN      11
#define TPB_PIN        10
#define RDnWR_PIN       9
#define nCS2_PIN        8

void writeUart(byte data, bool ctl) {
   digitalWrite(RSEL_PIN, ctl ? HIGH : LOW );
   digitalWrite(RDnWR_PIN, LOW);
   digitalWrite(nCS2_PIN, LOW); 
   DDRD = 0xFF;                           // PORT D to output
   PORTD = data;   

   asm("\tnop\n\tnop\n");                 // 125 ns delay
   digitalWrite(TPB_PIN, HIGH);
   asm("\tnop\n\tnop\n\tnop\n");          // 187.5 ns delay
   digitalWrite(TPB_PIN, LOW);
   
   digitalWrite(nCS2_PIN, HIGH);
   asm("\tnop\n\tnop\n");                 // 125 ns delay
   DDRD = 0x00;                           // PORT D to input
}

void writeUartData(byte data) {
   writeUart(data, false);
}

void writeUartCtl(byte data) {
   writeUart(data, true);
}

byte readUart(bool status) {
   byte data;
   
   digitalWrite(RSEL_PIN, status ? HIGH : LOW );
   digitalWrite(RDnWR_PIN, HIGH);
   digitalWrite(nCS2_PIN, LOW); 

   asm("\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n");   // 312.5 ns delay
   digitalWrite(TPB_PIN, HIGH);
   asm("\tnop\n\tnop\n\tnop\n");          // 187.5 ns delay
   data = PIND;   
   digitalWrite(TPB_PIN, LOW);

   digitalWrite(nCS2_PIN, HIGH);
   
   return data;
}

byte readUartData(void) {
   return readUart(false);
}

byte readUartCtl(void) {
   return readUart(true);
}

void initBus(void) {
   DDRD = 0x00;                        // PORT D to input

   digitalWrite(TPB_PIN, LOW);
   pinMode(TPB_PIN, OUTPUT);

   digitalWrite(nCS2_PIN, HIGH);
   pinMode(nCS2_PIN, OUTPUT);

   digitalWrite(RSEL_PIN, HIGH);
   pinMode(RSEL_PIN, OUTPUT);

   digitalWrite(RDnWR_PIN, HIGH);
   pinMode(RDnWR_PIN, OUTPUT);

   digitalWrite(nCLEAR_PIN, HIGH);     // circuit passively pulls
   pinMode(nCLEAR_PIN, OUTPUT);        // /CLEAR low on startup
}

// Sets a ~153.6KHz clock on OC2A (PORTB3)
void initBaudClock() {
   pinMode(CLOCK_PIN, OUTPUT);
   TCCR2A = 0;
   TCCR2B = 0;
   //UART BAUD CLOCK on pin 11 (153.6 khz which is baud 9600x16) thats why we need divisor 16
   //Timer2 register setup
   //Bits 7:6 mean toggle pin 11 on a compare match
   //Bits 1:0 and bit 3 in TCCR2B select Fast PWM mode, with OCRA for the TOP value
   TCCR2A = 0b01000011;
   //Bits 2:0 are the prescaler setting, this is a 1 prescaler
   TCCR2B = 0b00001001;
   //This is the value that, when the timer reaches it, will toggle the
   //output pin and restart. A value of 51 in this case gives a 153.85kHz output
   OCR2A = 51;
}

void init1854() {
   writeUartCtl(0b00011001);              // 8 data, 1 stop, no parity, no interrupts
   writeUartCtl(0b10000000);              // set TR bit
}

void writeUartChar( char c ) {
   while( !(readUartCtl() & 0b10000000) );
   writeUartData(c);
}

void writeUartString( const char *s ) {
   while( *s ) {
      writeUartChar( *s );
      ++s;
   }
}

void writeUartPgmString( const char *s ) {
   unsigned l = strlen_P(s);
   for( unsigned i = 0; i < l; ++i ) {
      char c = pgm_read_byte_near(s + i);
      writeUartChar(c);
   }
}

bool uartCharAvailable(void) {
   return (readUartCtl() & 0b00000001) != 0;
}

void setup() {
   initBus();
   initBaudClock();
   init1854();
   writeUartString("\r\nCDP1854 UART tester\r\n\nHello, world! Start typing.\r\n");
   writeUartString("If you're using an ANSI terminal, press the INSert key for a canned blurb.\r\n\n");
}

const char ipsum_lorem[] PROGMEM = {
   "Insert key pressed. Blurb follows:\r\n\n"
   "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor\r\n"
   "incididunt ut labore et dolore magna aliqua. Vitae tortor condimentum lacinia\r\n"
   "quis vel eros donec ac odio. Elit scelerisque mauris pellentesque pulvinar\r\n"
   "pellentesque. Eget gravida cum sociis natoque penatibus et. Fermentum iaculis\r\n"
   "eu non diam. Enim nunc faucibus a pellentesque sit amet porttitor eget. Lectus\r\n"
   "vestibulum mattis ullamcorper velit sed ullamcorper morbi tincidunt. Enim\r\n"
   "blandit volutpat maecenas volutpat. Et molestie ac feugiat sed lectus\r\n"
   "vestibulum mattis ullamcorper velit. Nisi vitae suscipit tellus mauris a. Ut\r\n"
   "placerat orci nulla pellentesque dignissim enim sit amet. Eget nunc lobortis\r\n"
   "mattis aliquam faucibus purus in. Turpis massa tincidunt dui ut. Sit amet\r\n"
   "volutpat consequat mauris nunc congue nisi vitae suscipit. Venenatis urna\r\n"
   "cursus eget nunc. Scelerisque eu ultrices vitae auctor eu. Et molestie ac\r\n"
   "feugiat sed lectus.\r\n\n"
};

int insKeyState = 0;

#define ESC '\x1b'

void loop() {
   char c;
   
   if( uartCharAvailable() ) {
      c = readUartData();
      switch( insKeyState ) {             // detect if INSert key is
         case 0:                          // pressed and send blurb
            if ( c == ESC ) {             // if it is
               ++insKeyState;
            } else {
               writeUartChar(c);
               if( c == '\r' ) {
                  writeUartChar('\n');
               }
            }
            break;
         case 1:
            if( c == '[' ) {
               ++insKeyState;
            } else {
               writeUartChar(ESC);
               writeUartChar(c);
               insKeyState = 0;
            }
            break;
         case 2:
            if( c == '2' ) {
               ++insKeyState;
            } else {
               writeUartString("\x1b[");
               writeUartChar(c);
               insKeyState = 0;
            }
            break;
         case 3:
            if( c == '~' ) {
               writeUartPgmString(ipsum_lorem);
            } else {
               writeUartString("\x1b[2");
               writeUartChar(c);
            }
            insKeyState = 0;
            break;
      }
   }
}

