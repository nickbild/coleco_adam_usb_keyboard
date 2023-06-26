/*
 * Nick Bild
 * June 2023
 * Use a Coleco Adam keyboard as a USB keyboard on a modern computer with the help of a Teensy 4.1.
 *
 * Thanks to Mike McMahon for the code (https://gist.github.com/MMcM/36fed5189a22cd381af2c6e26cd922a6)
 * describing ADAMNet communications with a Teensy 3.2, which that portion of this work is heavily based on.
 *
 * Wiring for Teensy 4.1:
 * Teensy 1(TX1) -- RJ 1
 * Teensy 2   -- RJ 2 (optional)
 * Teensy GND -- RJ 3,4,5
 * Teensy Vin -- RJ 6
 */

#include <Keyboard.h>

// ADAMNet packet type definitions.
const uint8_t MN_RESET   = 0x01;  // command.control (reset)
const uint8_t MN_STATUS  = 0x11;  // command.control (status)
const uint8_t MN_ACK     = 0x21;  // command.control (ack)
const uint8_t MN_CLR     = 0x31;  // command.control (clr)
const uint8_t MN_RECEIVE = 0x41;  // command.control (receive)
const uint8_t MN_CANCEL  = 0x51;  // command.control (cancel)
const uint8_t MN_SEND    = 0x61;  // command.data (send)
const uint8_t MN_NACK    = 0x71;  // command.control (nack)
const uint8_t MN_READY   = 0xD1;  // command.control (ready)
const uint8_t NM_STATUS  = 0x81;  // response.control (status)
const uint8_t NM_ACK     = 0x91;  // response.control (ack)
const uint8_t NM_CANCEL  = 0xA1;  // response.control (cancel)
const uint8_t NM_SEND    = 0xB1;  // response.data (send)
const uint8_t NM_NACK    = 0xC1;  // response.control (nack)

const uint32_t RECEIVE_INTERVAL = 10;
static uint32_t last_receive_time = 0;
uint8_t char_mappings[170];


void setup() {
    setup_char_mappings();

    // Hardware serial for interaction with Coleco keyboard.
    Serial1.begin(62500, SERIAL_8N1_RXINV_TXINV | SERIAL_HALF_DUPLEX);
    Serial1.setTimeout(100);

    // For Teensy to act as a USB keyboard.
    Keyboard.begin();

    // Hard reset signal.
    pinMode(2, OUTPUT);
    digitalWrite(2, HIGH);

    delay(100);
    
    // Soft reset.
    send_command(MN_RESET);
}

void loop() {
    if (millis() - last_receive_time > RECEIVE_INTERVAL) {
        uint8_t buf[8];
        send_command(MN_RECEIVE);
        
        if (receive_response(buf, 1)) {
            if (buf[0] == NM_ACK) {
                send_command(MN_CLR); // Clear to Send
                
                if (receive_response(buf, 5) && buf[0] == NM_SEND) {
                    if (buf[1] == 0 && buf[2] == 1 && buf[3] == buf[4]) {
                        // Checksum OK, we have received good data.  Send the keyboard an acknowledgement.
                        delay(5);
                        send_command(MN_ACK);

                        // buf[3] contains the ASCII code received from the keyboard.
                        if (buf[3] == 39) {
                          Keyboard.write('\'');
                        } else {
                          keyboardOutput(buf[3]);
                        }

                        last_receive_time = millis();
                    } else {
                        // Transmission error.  Try again.
                        send_command(MN_NACK);
                    }
                }
            } else if (buf[0] == NM_NACK) {
                // No input available, wait a bit.
                last_receive_time = millis();
            }
        }
    }
}

void send_command(uint8_t command) {
    Serial1.write(command);
    Serial1.flush();       // Wait for transmit to complete.
}

bool receive_response(uint8_t *buffer, uint8_t expected_length) {
    uint8_t len = Serial1.readBytes(buffer, expected_length);
    return len == expected_length;
}

void keyboardOutput(uint8_t pressed) {
  // Determine if any modifier keys were pressed.
  int ctrl = 0;
  int shift = 0;

  // Shift
  if ((pressed > 59 && pressed < 91) || (pressed > 32 && pressed < 42) 
  || (pressed == 58 || pressed == 64) || (pressed > 93 && pressed < 96)
  || (pressed > 122 && pressed < 127)) {
    shift = 1;
  }
  if (pressed == 61) {
    shift = 0;
  }

  // Control
  if (pressed > 0 and pressed < 27) {
    ctrl = 1;
  }
  if (pressed == 8 || pressed == 9 || pressed == 13) {
    ctrl = 0;
  }

  if (shift) {
    Keyboard.set_modifier(MODIFIERKEY_SHIFT);          
    Keyboard.send_now();
    delay(10);
  }

  if (ctrl) {
    Keyboard.set_modifier(MODIFIERKEY_CTRL);          
    Keyboard.send_now();
    delay(10);
  }

  // Simulate the USB key press.
  Keyboard.set_key1(char_mappings[pressed]);
  Keyboard.send_now();
  delay(5);

  // Clean up.
  Keyboard.set_modifier(0);
  Keyboard.set_key1(0);
  Keyboard.send_now();
}

void setup_char_mappings() {
  // Map Adam key codes to Teensy key codes.

  // Ctrl needs to be pressed.
  char_mappings[1] = KEY_A;
  char_mappings[2] = KEY_B;
  char_mappings[3] = KEY_C;
  char_mappings[4] = KEY_D;
  char_mappings[5] = KEY_E;
  char_mappings[6] = KEY_F;
  char_mappings[7] = KEY_G;
  char_mappings[10] = KEY_J;
  char_mappings[11] = KEY_K;
  char_mappings[12] = KEY_L;
  char_mappings[14] = KEY_N;
  char_mappings[15] = KEY_O;
  char_mappings[16] = KEY_P;
  char_mappings[17] = KEY_Q;
  char_mappings[18] = KEY_R;
  char_mappings[19] = KEY_S;
  char_mappings[20] = KEY_T;
  char_mappings[21] = KEY_U;
  char_mappings[22] = KEY_V;
  char_mappings[23] = KEY_W;
  char_mappings[24] = KEY_X;
  char_mappings[25] = KEY_Y;
  char_mappings[26] = KEY_Z;

  char_mappings[8] = KEY_BACKSPACE;
  char_mappings[9] = KEY_TAB;
  char_mappings[13] = KEY_ENTER;
  char_mappings[32] = KEY_SPACE;
  char_mappings[34] = KEY_QUOTE;
  char_mappings[42] = KEYPAD_ASTERIX;
  char_mappings[43] = KEYPAD_PLUS;
  char_mappings[44] = KEY_COMMA;
  char_mappings[45] = KEY_MINUS;
  char_mappings[46] = KEY_PERIOD;
  char_mappings[47] = KEY_SLASH;

  // These need shifted.
  char_mappings[33] = KEY_1;
  char_mappings[35] = KEY_3;
  char_mappings[36] = KEY_4;
  char_mappings[37] = KEY_5;
  char_mappings[38] = KEY_7;
  char_mappings[40] = KEY_9;
  char_mappings[41] = KEY_0;

  char_mappings[48] = KEY_0;
  char_mappings[49] = KEY_1;
  char_mappings[50] = KEY_2;
  char_mappings[51] = KEY_3;
  char_mappings[52] = KEY_4;
  char_mappings[53] = KEY_5;
  char_mappings[54] = KEY_6;
  char_mappings[55] = KEY_7;
  char_mappings[56] = KEY_8;
  char_mappings[57] = KEY_9;  
  char_mappings[59] = KEY_SEMICOLON;
  char_mappings[61] = KEY_EQUAL;

  // These need shifted.
  char_mappings[58] = KEY_SEMICOLON;
  char_mappings[60] = KEY_COMMA;
  char_mappings[62] = KEY_PERIOD;
  char_mappings[63] = KEY_SLASH;
  char_mappings[64] = KEY_2;

  char_mappings[65] = KEY_A;
  char_mappings[66] = KEY_B;
  char_mappings[67] = KEY_C;
  char_mappings[68] = KEY_D;
  char_mappings[69] = KEY_E;
  char_mappings[70] = KEY_F;
  char_mappings[71] = KEY_G;
  char_mappings[72] = KEY_H;
  char_mappings[73] = KEY_I;
  char_mappings[74] = KEY_J;
  char_mappings[75] = KEY_K;
  char_mappings[76] = KEY_L;
  char_mappings[77] = KEY_M;
  char_mappings[78] = KEY_N;
  char_mappings[79] = KEY_O;
  char_mappings[80] = KEY_P;
  char_mappings[81] = KEY_Q;
  char_mappings[82] = KEY_R;
  char_mappings[83] = KEY_S;
  char_mappings[84] = KEY_T;
  char_mappings[85] = KEY_U;
  char_mappings[86] = KEY_V;
  char_mappings[87] = KEY_W;
  char_mappings[88] = KEY_X;
  char_mappings[89] = KEY_Y;
  char_mappings[90] = KEY_Z;
  char_mappings[91] = KEY_LEFT_BRACE;
  char_mappings[92] = KEY_BACKSLASH;
  char_mappings[93] = KEY_RIGHT_BRACE;

  // These need shifted.
  char_mappings[94] = KEY_6; 
  char_mappings[95] = KEY_MINUS;
  
  char_mappings[96] = KEY_TILDE;

  char_mappings[97] = KEY_A;
  char_mappings[98] = KEY_B;
  char_mappings[99] = KEY_C;
  char_mappings[100] = KEY_D;
  char_mappings[101] = KEY_E;
  char_mappings[102] = KEY_F;
  char_mappings[103] = KEY_G;
  char_mappings[104] = KEY_H;
  char_mappings[105] = KEY_I;
  char_mappings[106] = KEY_J;
  char_mappings[107] = KEY_K;
  char_mappings[108] = KEY_L;
  char_mappings[109] = KEY_M;
  char_mappings[110] = KEY_N;
  char_mappings[111] = KEY_O;
  char_mappings[112] = KEY_P;
  char_mappings[113] = KEY_Q;
  char_mappings[114] = KEY_R;
  char_mappings[115] = KEY_S;
  char_mappings[116] = KEY_T;
  char_mappings[117] = KEY_U;
  char_mappings[118] = KEY_V;
  char_mappings[119] = KEY_W;
  char_mappings[120] = KEY_X;
  char_mappings[121] = KEY_Y;
  char_mappings[122] = KEY_Z;

  // These need shifted.
  char_mappings[123] = KEY_LEFT_BRACE;
  char_mappings[124] = KEY_BACKSLASH;
  char_mappings[125] = KEY_RIGHT_BRACE;

  char_mappings[126] = KEY_TILDE;
  char_mappings[127] = KEY_DELETE;

  char_mappings[160] = KEY_UP;
  char_mappings[161] = KEY_RIGHT;
  char_mappings[162] = KEY_DOWN;
  char_mappings[163] = KEY_LEFT;
}
