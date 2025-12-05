#include <16F877A.h>
#fuses XT, NOWDT, NOPROTECT, NOBROWNOUT, NOLVP, NOPUT, NOWRT, NODEBUG, NOCPD
#use delay (crystal = 20000000)

// ====== KHAI BÁO LCD 20x4 ======
#define LCD_ENABLE_PIN PIN_B2
#define LCD_RS_PIN     PIN_B4
#define LCD_RW_PIN     PIN_B3
#define LCD_DATA4      PIN_D7
#define LCD_DATA5      PIN_D6
#define LCD_DATA6      PIN_D5
#define LCD_DATA7      PIN_D4
#include <LCD20x4.c>

// ====== KHAI BÁO HX711 ======
#define DT1 PIN_A1
#define SCK PIN_A0

double SCALE = 368.3;

unsigned int32 readCount(void) {
    unsigned int32 data = 0;
    unsigned int8 j;
    output_bit(DT1, 1);
    output_bit(SCK, 0);

    while (input(DT1));  

    for (j = 0; j < 24; j++) {
        output_bit(SCK, 1);
        data = data << 1;
        output_bit(SCK, 0);
        if (input(DT1)) data++;
    }

    output_bit(SCK, 1);  // bit tho 25
    data = data ^ 0x800000;
    output_bit(SCK, 0);

    return data;
}
//doc trung binh 5 lan
int32 readAverage(void) {
    signed int32 sum = 0;
    for (int i = 0; i < 5; i++) {  
        sum += (signed int32)readCount();  
    }
    return sum / 5;
}

// ====== KHAI BÁO KEYPAD 4x4 ======
const unsigned char codekeypad[16] = {
   '1','2','3','/',
   '4','5','6','*',
   '7','8','9','-',
   'C','0','=','+'
};

unsigned int8 key = 16;

void keypad() {
    key = 16;
    output_high(PIN_D0);
    output_high(PIN_D1);
    output_high(PIN_D3);
    output_high(PIN_D2);

    output_low(PIN_D0); delay_us(20);
    if (!input(PIN_C0)) key = 0;
    if (!input(PIN_C1)) key = 4;
    if (!input(PIN_C2)) key = 8;
    if (!input(PIN_C3)) key = 12;
    output_high(PIN_D0);

    output_low(PIN_D1); delay_us(20);
    if (!input(PIN_C0)) key = 1;
    if (!input(PIN_C1)) key = 5;
    if (!input(PIN_C2)) key = 9;
    if (!input(PIN_C3)) key = 13;
    output_high(PIN_D1);

    output_low(PIN_D3); delay_us(20);
    if (!input(PIN_C0)) key = 2;
    if (!input(PIN_C1)) key = 6;
    if (!input(PIN_C2)) key = 10;
    if (!input(PIN_C3)) key = 14;
    output_high(PIN_D3);

    output_low(PIN_D2); delay_us(20);
    if (!input(PIN_C0)) key = 3;
    if (!input(PIN_C1)) key = 7;
    if (!input(PIN_C2)) key = 11;
    if (!input(PIN_C3)) key = 15;
    output_high(PIN_D2);
}

// ====== MAIN ======
void main() {
    set_tris_c(0b00001111);  // RC0-RC3 input 
    set_tris_d(0b00000000);  // RD0-RD3 output 

    lcd_init();
    
    lcd_gotoxy(1, 1);  
    printf(lcd_putc, "  CAN TINH TU DONG  ");
    lcd_gotoxy(1, 2);  
    printf(lcd_putc, "TO VAN HA - 42001010");
    lcd_gotoxy(1, 3);  
    printf(lcd_putc, " Dang khoi dong... ");
    lcd_gotoxy(1, 4);  
    printf(lcd_putc, "Khong de vat len can");

    delay_ms(5000);  
    printf(lcd_putc, "\f");

    unsigned int32 offset = readAverage();
    float gram = 0, last_gram = -1;
    float saved_weight = 0, diff_weight = 0;
    unsigned int8 last_key = 16;

    while(TRUE) {
         
        // ==== Ðoc trong luong ====
        unsigned int32 raw = readAverage();
        gram = ((signed int32)raw - (signed int32)offset) / SCALE;
         
        if (gram < 0) gram = 0;  // kh de hien thi so am
        float kg = gram / 1000.0;  // Chuyen sang kilogram

        float delta = gram - last_gram;
        if (delta > 0.2 || delta < -0.2) {
            lcd_gotoxy(1, 1);
            printf(lcd_putc, "  CAN TINH TU DONG  ");
            lcd_gotoxy(1, 2);
            printf(lcd_putc, "Khoi Luong: %.2f Kg", kg);
            last_gram = gram;
        }

        // ==== Kiem tra phím bam ====
        keypad();

        if (key != 16 ) {
            if (key == 14) {  // '=' 
                saved_weight = kg;
                lcd_gotoxy(1, 3);
                printf(lcd_putc, "Luu KL: %.2f kg   ", saved_weight);
            }
            else if (key == 15) {  // '+'
                diff_weight = kg + saved_weight;
                lcd_gotoxy(1, 4);
                printf(lcd_putc, "Tong KL: %.2f kg  ", diff_weight);
            }
            else if (key == 11) {  // '-'
                diff_weight = kg - saved_weight;
                lcd_gotoxy(1, 4);
                printf(lcd_putc, "Hieu KL: %.2f kg  ", diff_weight);
            }
            else if (key == 7) {  // '*' 
                diff_weight = kg * saved_weight;
                lcd_gotoxy(1, 4);
                printf(lcd_putc, "Tich KL: %.2f kg  ", diff_weight);
            }
            else if (key == 3) {  // '/' 
                if (saved_weight > 0.1) {
                    diff_weight = kg / saved_weight;
                    lcd_gotoxy(1, 4);
                    printf(lcd_putc, "So luong SP: %.2f", diff_weight);
                } 
            }
            else if (key == 12) {  
                saved_weight = 0;
                lcd_gotoxy(1, 3);
                printf(lcd_putc, "                   ");
                lcd_gotoxy(1, 4);
                printf(lcd_putc, "                   ");
            }

            last_key = key;
        }

        // ==== Ðoi tha phím ====
        do {
            keypad();
            delay_ms(5);
        } while (key != 16);
         key != last_key;
        delay_ms(100);  
    }
}
