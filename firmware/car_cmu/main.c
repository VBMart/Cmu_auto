#include "stm32f10x.h"
/* #include <stdio.h> */
#include <stdlib.h>
#include <stdint.h>
/* #include <math.h> */
#include <stm32f10x_gpio.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_usart.h>
#include <stm32f10x_exti.h>
#include <misc.h>
#include <printf.h>
#include <stm32f10x_adc.h>

// /dev/rfcomm0

void delay(int ms) {
  int i, j;
  for(i=0; i < ms; i++) {
    for(j=0; j < 2500; j++) {
      asm volatile("nop");
      asm volatile("nop");
      asm volatile("nop");
      asm volatile("nop");
      asm volatile("nop");
      asm volatile("nop");
      asm volatile("nop");
    }
  }
}

int curPos = 0;

void put(void* ptr, char c) {
  USART_SendData(USART2, c);
  while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
}

// эти самые рид и принтчар мож переименовать в bluetooth_read и bluetooth_write
void bluetooth_write(char c) {
  USART_SendData(USART1, c);
  while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
}


// эти самые рид и принтчар мож переименовать в bluetooth_read и bluetooth_write
uint8_t bluetooth_read() {
  while(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET);
  return USART_ReceiveData(USART1);
}

// эти самые рид и принтчар мож переименовать в bluetooth_read и bluetooth_write
uint8_t testBluetooth_read() {
  return (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) != RESET);
}

// not use
void print(char *str) {
  while(*str) {
    USART_SendData(USART1, *(str++));
    while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
  }
}

// not use
void print_a(uint8_t *a, uint32_t size) {
  uint32_t i = 0;
  for( ; i < size; i ++) {
    print_char((char)a[i]);
  }
}

void uart_init() {
  RCC_DeInit();
  RCC_HSEConfig(RCC_HSE_ON);
  RCC_WaitForHSEStartUp();

  RCC_HCLKConfig   (RCC_SYSCLK_Div1);
  RCC_PCLK2Config  (RCC_HCLK_Div1);
  RCC_PCLK1Config  (RCC_HCLK_Div1);
  
  RCC_PLLConfig (RCC_PLLSource_PREDIV1, RCC_PLLMul_2);
  RCC_PLLCmd (ENABLE);

  while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);

  RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
   
  while (RCC_GetSYSCLKSource() != 0x08);

  GPIO_InitTypeDef gpioInitStruct;
  // UART1 - debug
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB |RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO | RCC_APB2Periph_USART1, ENABLE);
  gpioInitStruct.GPIO_Pin = GPIO_Pin_9;
  gpioInitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
  gpioInitStruct.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_Init(GPIOA, &gpioInitStruct);
  
  gpioInitStruct.GPIO_Pin = GPIO_Pin_10;
  gpioInitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  gpioInitStruct.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_Init(GPIOA, &gpioInitStruct);
  
  USART_InitTypeDef usartInit;
  usartInit.USART_BaudRate = 9600;
  usartInit.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  usartInit.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
  usartInit.USART_Parity = USART_Parity_No;
  usartInit.USART_StopBits = USART_StopBits_1;
  usartInit.USART_WordLength = USART_WordLength_8b;
  USART_Init(USART1, &usartInit);
  
  USART_Cmd(USART1, ENABLE);

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

  gpioInitStruct.GPIO_Pin = GPIO_Pin_2;
  gpioInitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
  gpioInitStruct.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_Init(GPIOA, &gpioInitStruct);
  
  gpioInitStruct.GPIO_Pin = GPIO_Pin_3;
  gpioInitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  gpioInitStruct.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_Init(GPIOA, &gpioInitStruct);
  
  USART_Init(USART2, &usartInit);  
  USART_Cmd(USART2, ENABLE);
}

void init() {
  GPIO_InitTypeDef gpioInitStruct;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOA, ENABLE);

  gpioInitStruct.GPIO_Pin = GPIO_Pin_8;
  gpioInitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
  gpioInitStruct.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_Init(GPIOB, &gpioInitStruct);
  GPIO_ResetBits(GPIOB, GPIO_Pin_8);

  gpioInitStruct.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
  gpioInitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
  gpioInitStruct.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_Init(GPIOB, &gpioInitStruct);
  GPIO_ResetBits(GPIOB, GPIO_Pin_8);
  
  SysTick_Config(SystemCoreClock / (255*50*4));
}

#define SOCKET1_ON  GPIO_SetBits(GPIOB,   GPIO_Pin_0);
#define SOCKET1_OFF GPIO_ResetBits(GPIOB, GPIO_Pin_0);
#define SOCKET2_ON  GPIO_SetBits(GPIOB,   GPIO_Pin_1);
#define SOCKET2_OFF GPIO_ResetBits(GPIOB, GPIO_Pin_1);
#define SOCKET3_ON  GPIO_SetBits(GPIOB,   GPIO_Pin_2);
#define SOCKET3_OFF GPIO_ResetBits(GPIOB, GPIO_Pin_2);
#define SOCKET4_ON  GPIO_SetBits(GPIOB,   GPIO_Pin_10);
#define SOCKET4_OFF GPIO_ResetBits(GPIOB, GPIO_Pin_10);
#define SOCKET5_ON  GPIO_SetBits(GPIOB,   GPIO_Pin_11);
#define SOCKET5_OFF GPIO_ResetBits(GPIOB, GPIO_Pin_11);
#define LED1_ON  GPIO_SetBits(GPIOB,   GPIO_Pin_14);
#define LED1_OFF GPIO_ResetBits(GPIOB, GPIO_Pin_14);
#define LED2_ON  GPIO_SetBits(GPIOB,   GPIO_Pin_15);
#define LED2_OFF GPIO_ResetBits(GPIOB, GPIO_Pin_15);

//#define WRITE_LED_DATA(data_val) GPIO_WriteBit(GPIOB, GPIO_Pin_0, data_val);

#define DATA_PIN_ON SOCKET3_ON;
#define DATA_PIN_OFF SOCKET3_OFF;
#define CLOCK_PIN_ON SOCKET4_ON;
#define CLOCK_PIN_OFF SOCKET4_OFF;
#define READY_PIN_ON SOCKET5_ON;
#define READY_PIN_OFF SOCKET5_OFF;

#define LEDS_COUNT 48
#define LEDS_BAM_BUF_LENGTH 6

uint8_t ledsBAMBuf[8][LEDS_BAM_BUF_LENGTH];
uint8_t ledsBrightBuf[LEDS_COUNT];
uint8_t iBAM;
uint8_t iPWM;

inline void writeLedsData(uint8_t ledBuf[], uint8_t length){
  uint8_t i;
  uint8_t j;
  uint8_t tmp;
  for (i = 0; i < length; i++){
    for (j = 0; j < 8; j++){
      tmp = ((ledBuf[i]) & ((uint8_t)(1 << j))) >> j;
      
      if (tmp)
      {
        //printf("1");
        DATA_PIN_ON;
      }else{
        //printf("0");
        DATA_PIN_OFF;
      }
      CLOCK_PIN_ON; // Clock
      CLOCK_PIN_OFF; // Clock       
    }
  }
  //printf("\n\r");
  READY_PIN_ON; // Ready
  READY_PIN_OFF; // Ready   
}


void ledsBrBufToLedsBuf(uint8_t ledsBrBuf[], uint8_t ledsBuf[], uint8_t ledsCount, uint8_t mod){
  uint8_t i;
  uint8_t j;
  uint8_t buf;
  uint8_t k;
  j = 0;
  k = 0;
  buf = 0;
  
  for (i = 0; i < ledsCount; i++){
    //printf("%d, %d, %d, %d, %d", i, j, buf, ledsBrBuf[i], mod);
    
    buf = buf | (((uint8_t)(ledsBrBuf[i] & mod) == mod) << j);
    //printf(", %d, %d, %d, %d", ledsBrBuf[i] & mod, (uint8_t)(ledsBrBuf[i] & mod) == mod, ((uint8_t)(ledsBrBuf[i] & mod) == mod) << j, buf);
    //printf("\n\r");
    j++;
    if (j == 8){
      ledsBuf[k] = buf;
      k++;
      j = 0;
      buf = 0;
    }
  }
}

void ledsBrBufToLedsBAMBuf(uint8_t ledsBrBuf[], uint8_t ledsBuf[][LEDS_BAM_BUF_LENGTH], uint8_t ledsCount){
  uint8_t i;
  for (i = 0; i < 8; i++){
    ledsBrBufToLedsBuf(ledsBrBuf, ledsBuf[i], ledsCount, 1 << i);
  }
}

//-----------------------------------------------------------------------------
// Tis function converts HSV values to RGB values, scaled from 0 to maxBright -
//
// The ranges for the input variables are:
// h: 0-359
// s: 0-255
// v: 0-255
//
// The ranges for the output variables are:
// r: 0-maxBrightness
// g: 0-maxBrightness
// b: 0-maxBrightness
void hsv2rgb(uint16_t h, uint16_t s, uint16_t v, uint8_t *r, uint8_t *g, uint8_t *b, uint8_t maxBright){
  uint16_t h_accent;
  uint16_t bottom;
  uint16_t top;
  uint8_t rising;
  uint8_t falling;

  h_accent = h / 60;
  bottom = ((255 - s) * v) >> 8;
  top = v;
  rising = ((top-bottom)*(h % 60))/60 + bottom;
  falling = ((top-bottom)*(60 - h % 60))/60 + bottom;
  //printf("%d, %d, %d, %d ->", h_accent, rising, falling, bottom);
  switch(h_accent){
    case 0:
      *r = top;
      *g = rising;
      *b = bottom;
      break;
    case 1:
      *r = falling;
      *g = top;
      *b = bottom;
      break;
   case 2:
      *r = bottom;
      *g = top;
      *b = rising;
      break;
    case 3:
      *r = bottom;
      *g = falling;
      *b = top;
      break;
    case 4:
      *r = rising;
      *g = bottom;
      *b = top;
      break;
    case 5:
      *r = top;
      *g = bottom;
      *b = falling;
      break;
  }
  //printf("%d, %d, %d\n\r", r, g, b);
  if (maxBright < 255) {
    *r = *r * maxBright / 255;
    *g = *g * maxBright / 255;
    *b = *b * maxBright / 255;
  }
}
// Tis function converts HSV values to RGB values, scaled from 0 to maxBright -
//-----------------------------------------------------------------------------

void setRainbow(uint8_t ledsBrBuf[], uint8_t ledsCount, uint8_t range, uint16_t iCurrent, uint8_t direction){
  uint16_t i;
  uint16_t hh;
  uint16_t ss;
  uint16_t vv;
  uint16_t rr;
  uint16_t gg;
  uint16_t bb;
  uint16_t ii;
  
  for (i = 0; i < 16; i++){
    if (direction){
      ii = iCurrent;
    }else{
      ii = 360 - iCurrent;
    } 
    hh = ((i*360/(range) + ii)) % 360;
    ss = 255;
    vv = 255;
    hsv2rgb(hh, ss, vv, &rr, &gg, &bb, 255);
    //printf("%d, %d, %d, %d, %d, %d, %d, %d\n\r", i, ii, hh, ss, vv, rr, gg, bb);
    if (direction){
      ii = i*3;
    }else{
      ii = (15-i)*3;
    }  
    ledsBrBuf[ii] = bb;
    ledsBrBuf[ii+1] = rr;
    ledsBrBuf[ii+2] = gg;    
  }
}

void SysTick_Handler(){
  
  iPWM++;
  
  //printf("%d %d %d\n\r", iPWM, iBAM, 1 << iBAM);
  if (iPWM == (1 << iBAM)){
    iBAM++;
    writeLedsData(ledsBAMBuf[iBAM-1], LEDS_BAM_BUF_LENGTH);
  }
  
  if (iPWM == 255){
    iPWM = 0;
    iBAM = 1;
  }
  
}

void init_pins() {
  GPIO_InitTypeDef gpioInitStruct;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

  gpioInitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_14 | GPIO_Pin_15;
  gpioInitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
  gpioInitStruct.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_Init(GPIOB, &gpioInitStruct);
  GPIO_ResetBits(GPIOB, GPIO_Pin_8);
  
  gpioInitStruct.GPIO_Mode = GPIO_Mode_AIN;
  gpioInitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
  GPIO_Init(GPIOA, &gpioInitStruct);
}

void doRainbow(){
  uint16_t ii;
  ii = 360;
  
  while (1){
    setRainbow(ledsBrightBuf, LEDS_COUNT, 10, ii, 1);
    ledsBrBufToLedsBAMBuf(ledsBrightBuf, ledsBAMBuf, LEDS_COUNT);
    ii--;
    if (ii == 0){ii = 360;}
    delay(5);
  }  
}

void doRunningColor(){
  uint8_t led;
  uint8_t iled;
  uint8_t i;
  char dir = 1;
  led = 0;
  dir = 1;
  while (1){
    iled = 0;
    for (i = 0; i < LEDS_COUNT; i++){
      if ( i >= 18){
        if ((i % 3 == 1) || (i % 3 == 1)){
          if (led == iled){
            ledsBrightBuf[i] = 255;
          }else if ((led == iled+1) || (led == iled-1)){
            ledsBrightBuf[i] = 64;
          }else if ((led == iled+2) || (led == iled-2)){
            ledsBrightBuf[i] = 16;
          }else if ((led == iled+3) || (led == iled-3)){
            ledsBrightBuf[i] = 4;
          }else{
            ledsBrightBuf[i] = 0;
          }
          ledsBrightBuf[i] = ledsBrightBuf[i] >> 2;
        }else{
          ledsBrightBuf[i] = 0;
        }
      }else{
        ledsBrightBuf[i] = 0;
      }
      if (i % 3 == 2){
        iled++;
      }
    }  
    ledsBrBufToLedsBAMBuf(ledsBrightBuf, ledsBAMBuf, LEDS_COUNT);
    
    led = led + dir;
    if (led == 18-3){ dir = - 1;}
    if (led == 0+5) {dir = 1;}
    
    delay(160);  
  }
}


int main(void) {
  init_printf(NULL, put);
  init_pins();
  uart_init();
  init();
  iBAM = 0;
  
  LED1_OFF;
  LED2_OFF;
  SOCKET1_OFF;
  SOCKET2_OFF;
  SOCKET3_OFF;
  SOCKET4_OFF;
  SOCKET5_OFF;
  
  READY_PIN_OFF;
  CLOCK_PIN_OFF;
  
  uint8_t i;
  uint8_t j = 0;
  uint8_t k = 0;
  
 
  curPos = 0;
  LED2_OFF;
  
  
  uint8_t ledBuf[LEDS_BAM_BUF_LENGTH];
  uint8_t ledBrBuf[LEDS_COUNT];
  
 
  j = 12;
  for (i = 0; i < LEDS_COUNT; i++){
    ledsBrightBuf[i] = 0;
  }  
  ledsBrBufToLedsBAMBuf(ledsBrightBuf, ledsBAMBuf, LEDS_COUNT);
  
  printf("started\n\r");
  //doRainbow();
  //doRunningColor();
  int cmd;

  
  uint16_t ii;
  ii = 360;
  int delta;
  delta = -1; 
  cmd = 0;
  while (1){
    if (testBluetooth_read()){
//    if (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) != RESET){
      printf("(");        
      cmd = bluetooth_read();
      printf("%d", cmd);
      printf(")");
    }

    if (cmd == 16){
      delta = 1;
    }
    if (cmd == 4){
      delta = -1;
    }  
    if (cmd == 32){
      delta = 0;
    }        
          
    setRainbow(ledsBrightBuf, LEDS_COUNT, 10, ii, 1);
    ledsBrBufToLedsBAMBuf(ledsBrightBuf, ledsBAMBuf, LEDS_COUNT);
    ii = ii + delta;
    if ((ii == 0) && (delta < 0)){ii = 360;}
    if ((ii == 360) && (delta > 0)){ii = 0;}
    delay(5);  
  }
  
  j = 1;
  uint8_t br;
  char dir = 1;
  br = 1; 
  uint16_t led;

  led = 18;
  while (1){
    for (i = 0; i < LEDS_COUNT; i++){
      if ((i == led) || (i == 63 - led)){
        ledsBrightBuf[i] = 255;
      }else{
        ledsBrightBuf[i] = 0;
      }  
    }  
    ledsBrBufToLedsBAMBuf(ledsBrightBuf, ledsBAMBuf, LEDS_COUNT);
    br = br + dir;
    if (br == 0) {
      dir = 3;
      j++;
      if (j >= 16){
        j = 1;
      }
    }
    
    led++;
    if (led >=48){ led = 18;}
    
    if (br == 255) {dir = -3;}
   
    if (curPos == 1){
      LED1_ON;
      LED2_OFF;
    }else{
      LED2_ON;
      LED1_OFF;
    }     
     
    curPos = 1 - curPos;    
    delay(200);  
  }
  
  
  while (1){
    for (i = 0; i < LEDS_COUNT; i++){
      /*if (i % 3 == 1){
        ledsBrightBuf[i] = br;
      }else{
        ledsBrightBuf[i] = 0;
      }*/
      ledsBrightBuf[i] = br * ((j & (1 << (i % 3))) == (1 << (i % 3)));
    }  
    ledsBrBufToLedsBAMBuf(ledsBrightBuf, ledsBAMBuf, LEDS_COUNT);
    br = br + dir;
    if (br == 0) {
      dir = 1;
      j++;
      if (j >= 16){
        j = 1;
      }      
    }
    if (br == 255) {dir = -1;}
    
    if (curPos == 1){
      LED1_ON;
      LED2_OFF;
    }else{
      LED2_ON;
      LED1_OFF;
    }     
     
    curPos = 1 - curPos;    
    delay(2);     
  }
   
  while (1){
    j = j + 1;
    if (j >= 16){
      j = 0;
      br = br * 2;
      if (br == 0) {br = 1;}
    }
   
  
  for (i = 0; i < LEDS_COUNT; i++){
    /*
    if (i % 3 == 0){
      ledsBrightBuf[i] = 255;
    }else{
      ledsBrightBuf[i] = 0;
    }
    */
    ledsBrightBuf[i] = br * ((j & (1 << (i % 3))) == (1 << (i % 3)));
  }  
  ledsBrBufToLedsBAMBuf(ledsBrightBuf, ledsBAMBuf, LEDS_COUNT);
  //for (i = 0; i < 8; i++){
//    writeLedsData(ledsBAMBuf[7], LEDS_BAM_BUF_LENGTH);
//    delay(500);
//  }
    
    
    /*
    for (i = 0; i < LEDS_COUNT; i++){
      ledBrBuf[i] = 255 * ((j & (1 << (i % 3))) == (1 << (i % 3)));
    }    
    ledsBrBufToLedsBuf(ledBrBuf, ledBuf, LEDS_COUNT);
    writeLedsData(ledBuf, LEDS_BUF_LENGTH);
    */
    //writeLedsData(ldBuf[j], LEDS_BAM_BUF_LENGTH);
    
    
    if (curPos == 1){
      LED1_ON;
      LED2_OFF;
    }else{
      LED2_ON;
      LED1_OFF;
    } 
    
     
    curPos = 1 - curPos;    
    delay(1000);    
  }  
  
  
  j = 1;
  while (1){
    j++;
    if (j >= 16){
      j = 1;
    }
    
    for (i = 0; i<LEDS_BAM_BUF_LENGTH; i++){
      ledBuf[i] = (j << 6) + (j << 3) + j;
    }
    printf("%d   ->  %d\n\r", j, ledBuf[0]);
    writeLedsData(ledBuf, LEDS_BAM_BUF_LENGTH);
    
    if (curPos == 1){
      LED1_ON;
      LED2_OFF;
    }else{
      LED2_ON;
      LED1_OFF;
    } 
     
    curPos = 1 - curPos;    
    delay(600);    
  }


  j = 0;
  while (1){
    j++;  
    k++;  
    if (j >= 48){
      j = 0;
    }
    // i == 0 red
    // i == 1 blue
    // i == 2 green
    for (i = 0; i<48; i++){
      if (i == j){
        DATA_PIN_ON; // Data
      }else{
        DATA_PIN_OFF; // Data
      }  
      CLOCK_PIN_ON; // Clock
      CLOCK_PIN_OFF; // Clock
    }
    READY_PIN_ON; // Ready
    READY_PIN_OFF; // Ready      
    printf("%d   ->  %d\n\r", k, j);

    if (curPos == 1){
      LED1_ON;
      LED2_OFF;
    }else{
      LED2_ON;
      LED1_OFF;
    } 
     
    curPos = 1 - curPos;    
    delay(600);
  } 
   
  
  while (1){
    if (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) != RESET){
      printf(bluetooth_read());
    }
    printf("1\n\r");
    delay(500);
  }

  while (1) {
    int cmd = read();
    int socket = read();
    if(cmd == '0') {
      if(socket == '1') {
	SOCKET1_OFF;
	LED2_OFF;
        print_char('1');
//        print('Socket off');
      }
    } else if(cmd == '1') {
      if(socket == '1') {
	SOCKET1_ON;
	LED2_ON;
	print_char('0');
//        print('Socket on');
      }
    }
//    print_char('!');
  }
}
