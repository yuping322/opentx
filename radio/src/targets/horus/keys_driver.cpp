/*
 * Copyright (C) OpenTX
 *
 * Based on code named
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "opentx.h"

uint32_t readKeys()
{
  uint32_t result = 0;

  if (~KEYS_GPIO_REG_MENU & KEYS_GPIO_PIN_MENU)
    result |= 1 << KEY_PGUP;
  if (~KEYS_GPIO_REG_EXIT & KEYS_GPIO_PIN_EXIT)
    result |= 1 << KEY_PGDN;
  if (~KEYS_GPIO_REG_ENTER & KEYS_GPIO_PIN_ENTER)
    result |= 1 << KEY_ENTER;
  if (~KEYS_GPIO_REG_UP & KEYS_GPIO_PIN_UP)
    result |= 1 << KEY_MODEL;
  if (~KEYS_GPIO_REG_DOWN & KEYS_GPIO_PIN_DOWN)
    result |= 1 << KEY_EXIT;
  if (~KEYS_GPIO_REG_RIGHT & KEYS_GPIO_PIN_RIGHT)
    result |= 1 << KEY_TELEM;
  if (~KEYS_GPIO_REG_LEFT & KEYS_GPIO_PIN_LEFT)
    result |= 1 << KEY_RADIO;

  // TRACE("readKeys(): %x", result);

  return result;
}

uint32_t readTrims()
{
  uint32_t result = 0;

  if (~TRIMS_GPIO_REG_LHL & TRIMS_GPIO_PIN_LHL)
    result |= 0x01;
  if (~TRIMS_GPIO_REG_LHR & TRIMS_GPIO_PIN_LHR)
    result |= 0x02;
  if (~TRIMS_GPIO_REG_LVD & TRIMS_GPIO_PIN_LVD)
    result |= 0x04;
  if (~TRIMS_GPIO_REG_LVU & TRIMS_GPIO_PIN_LVU)
    result |= 0x08;
  if (~TRIMS_GPIO_REG_RVD & TRIMS_GPIO_PIN_RVD)
    result |= 0x10;
  if (~TRIMS_GPIO_REG_RVU & TRIMS_GPIO_PIN_RVU)
    result |= 0x20;
  if (~TRIMS_GPIO_REG_RHL & TRIMS_GPIO_PIN_RHL)
    result |= 0x40;
  if (~TRIMS_GPIO_REG_RHR & TRIMS_GPIO_PIN_RHR)
    result |= 0x80;
#if !defined(SIMU)
  if (~TRIMS_GPIO_REG_LSD & TRIMS_GPIO_PIN_LSD)
    result |= 0x100;
  if (~TRIMS_GPIO_REG_LSU & TRIMS_GPIO_PIN_LSU)
    result |= 0x200;
  if (~TRIMS_GPIO_REG_RSD & TRIMS_GPIO_PIN_RSD)
    result |= 0x400;
  if (~TRIMS_GPIO_REG_RSU & TRIMS_GPIO_PIN_RSU)
    result |= 0x800;
#endif
  // TRACE("readTrims(): result=0x%04x", result);

  return result;
}

uint16_t trimDown(uint16_t idx)
{
  return readTrims() & (1 << idx);
}

uint8_t keyDown()
{
  return readKeys();
}

int32_t rotencValue;
uint32_t rotencPosition;
uint32_t rotencSpeed = ROTENC_LOWSPEED;

#define ROTENC_DELAY_MIDSPEED          10
#define ROTENC_DELAY_HIGHSPEED         5

void checkRotaryEncoder()
{
  register uint32_t newpos = ROTARY_ENCODER_POSITION();
  if (newpos != rotencPosition) {
    if ((rotencPosition & 0x01) ^ ((newpos & 0x02) >> 1)) {
      --rotencValue;
    }
    else {
      ++rotencValue;
    }
    rotencPosition = newpos;
  }
}

/* TODO common to ARM */
void readKeysAndTrims()
{
  register uint32_t i;

  uint8_t enuk = 0;
  uint32_t in = readKeys();
  for (i = 0; i < TRM_BASE; i++) {
    keys[enuk++].input(in & (1 << i));
  }

  static rotenc_t rePreviousValue;
  rotenc_t reNewValue = (rotencValue / 2);
  int8_t scrollRE = reNewValue - rePreviousValue;
  static uint32_t lastTick = 0;
  if (scrollRE) {
    rePreviousValue = reNewValue;
    if (scrollRE < 0) {
      putEvent(EVT_ROTARY_LEFT);
    }
    else {
      putEvent(EVT_ROTARY_RIGHT);
    }
    uint32_t delay = get_tmr10ms() - lastTick;
    lastTick = get_tmr10ms();
    if (delay < ROTENC_DELAY_HIGHSPEED)
      rotencSpeed = ROTENC_HIGHSPEED;
    else if (delay < ROTENC_DELAY_MIDSPEED)
      rotencSpeed = ROTENC_MIDSPEED;
    else
      rotencSpeed = ROTENC_LOWSPEED;
  }

  in = readTrims();
  for (i = 1; i < 4096; i <<= 1) {
    keys[enuk++].input(in & i);
  }
}

#define ADD_2POS_CASE(x) \
  case SW_S ## x ## 0: \
    xxx = SWITCHES_GPIO_REG_ ## x  & SWITCHES_GPIO_PIN_ ## x ; \
    break; \
  case SW_S ## x ## 2: \
    xxx = ~SWITCHES_GPIO_REG_ ## x  & SWITCHES_GPIO_PIN_ ## x ; \
    break
#define ADD_NEG_2POS_CASE(x) \
  case SW_S ## x ## 2: \
    xxx = SWITCHES_GPIO_REG_ ## x  & SWITCHES_GPIO_PIN_ ## x ; \
    break; \
  case SW_S ## x ## 0: \
    xxx = ~SWITCHES_GPIO_REG_ ## x  & SWITCHES_GPIO_PIN_ ## x ; \
    break
#define ADD_3POS_CASE(x, i) \
  case SW_S ## x ## 0: \
    xxx = (SWITCHES_GPIO_REG_ ## x ## _H & SWITCHES_GPIO_PIN_ ## x ## _H); \
    if (IS_3POS(i)) { \
      xxx = xxx && (~SWITCHES_GPIO_REG_ ## x ## _L & SWITCHES_GPIO_PIN_ ## x ## _L); \
    } \
    break; \
  case SW_S ## x ## 1: \
    xxx = (SWITCHES_GPIO_REG_ ## x ## _H & SWITCHES_GPIO_PIN_ ## x ## _H) && (SWITCHES_GPIO_REG_ ## x ## _L & SWITCHES_GPIO_PIN_ ## x ## _L); \
    break; \
  case SW_S ## x ## 2: \
    xxx = (~SWITCHES_GPIO_REG_ ## x ## _H & SWITCHES_GPIO_PIN_ ## x ## _H); \
    if (IS_3POS(i)) { \
      xxx = xxx && (SWITCHES_GPIO_REG_ ## x ## _L & SWITCHES_GPIO_PIN_ ## x ## _L); \
    } \
    break

#if !defined(BOOT)
bool switchState(EnumKeys enuk)
{
  register uint32_t xxx = 0;

  if (enuk < (int) DIM(keys)) return keys[enuk].state() ? 1 : 0;

  switch ((uint8_t) enuk) {
    ADD_3POS_CASE(A, 0);
    ADD_3POS_CASE(B, 1);
    ADD_3POS_CASE(C, 2);
    ADD_3POS_CASE(D, 3);
    ADD_3POS_CASE(E, 4);
    ADD_NEG_2POS_CASE(F);
    ADD_3POS_CASE(G, 6);
    ADD_2POS_CASE(H);
    default:
      break;
  }

  // TRACE("switch %d => %d", enuk, xxx);
  return xxx;
}
#endif

void keysInit()
{
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;

  GPIO_InitStructure.GPIO_Pin = KEYS_GPIOB_PINS;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = KEYS_GPIOC_PINS;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = KEYS_GPIOD_PINS;
  GPIO_Init(GPIOD, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = KEYS_GPIOE_PINS;
  GPIO_Init(GPIOE, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = KEYS_GPIOG_PINS;
  GPIO_Init(GPIOG, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = KEYS_GPIOH_PINS;
  GPIO_Init(GPIOH, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = KEYS_GPIOI_PINS;
  GPIO_Init(GPIOI, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = KEYS_GPIOJ_PINS;
  GPIO_Init(GPIOJ, &GPIO_InitStructure);

  rotencPosition = ROTARY_ENCODER_POSITION();
}
