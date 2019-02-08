/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <inputsequencechecker_hi.hxx>


namespace i18npool {

InputSequenceChecker_hi::InputSequenceChecker_hi()
    : InputSequenceCheckerImpl("com.sun.star.i18n.InputSequenceChecker_hi")
{
}

InputSequenceChecker_hi::~InputSequenceChecker_hi()
{
}
/* Non-Defined Class type */
#define ND_        0

/*
 * Devanagari character type definitions
 */
#define UP_  1  // ChandraBindu & Anuswar
#define NP_  2  // Visarg
#define IV_    3  // Independent Vowels
#define CN_    4  // Consonants except _CK & _RC
#define CK_    5  // Consonants that can be followed by Nukta
#define RC_    6  // Ra
#define NM_    7  // Matra
#define RM_    8  // Ra + HAL
#define IM_    9  // Choti I Matra
#define HL_    10 // HAL
#define NK_    11 // Nukta
#define VD_    12 // Vedic
#define HD_    13 // Hindu Numerals

/*
 * Devanagari character type table
 */
static const sal_uInt16 devaCT[128] = {
/*         0,    1,    2,    3,    4,    5,    6,    7,
           8,    9,    A,    B,    C,    D,    E,    F, */
/* 0900 */ ND_, UP_, UP_, NP_, ND_, IV_, IV_, IV_,
           IV_, IV_, IV_, IV_, IV_, IV_, IV_, IV_,
/* 0910 */ IV_, IV_, IV_, IV_, IV_, CK_, CK_, CK_,
           CN_, CN_, CN_, CN_, CK_, CN_, CN_, CN_,
/* 0920 */ CN_, CK_, CK_, CN_, CN_, CN_, CN_, CN_,
           CN_, CN_, CN_, CK_, CN_, CN_, CN_, CN_,
/* 0930 */ RC_, CN_, CN_, CN_, CN_, CN_, CN_, CN_,
           CN_, CN_, ND_, ND_, NK_, VD_, NM_, IM_,
/* 0940 */ RM_, NM_, NM_, NM_, NM_, RM_, RM_, RM_,
           RM_, RM_, RM_, RM_, RM_, HL_, ND_, ND_,
/* 0950 */ ND_, VD_, VD_, VD_, VD_, ND_, ND_, ND_,
           CN_, CN_, CN_, CN_, CN_, CN_, CN_, CN_,
/* 0960 */ IV_, IV_, NM_, NM_, ND_, ND_, HD_, HD_,
           HD_, HD_, HD_, HD_, HD_, HD_, HD_, HD_,
/* 0970 */ ND_, ND_, ND_, ND_, ND_, ND_, ND_, ND_,
           ND_, ND_, ND_, ND_, ND_, ND_, ND_, ND_,
};

/*
 * Devanagari character composition table
 */
static const sal_uInt16 dev_cell_check[14][14] = {
  /*        ND, UP, NP, IV, CN, CK, RC, NM, RM, IM, HL, NK, VD, HD, */
  /* 0  */ { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, /* ND */
  /* 1  */ { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, /* UP */
  /* 2  */ { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, /* NP */
  /* 3  */ { 0,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, /* IV */
  /* 4  */ { 0,  1,  1,  0,  0,  0,  0,  1,  1,  1,  1,  0,  0,  0 }, /* CN */
  /* 5  */ { 0,  1,  1,  0,  0,  0,  0,  1,  1,  1,  1,  1,  0,  0 }, /* CK */
  /* 6  */ { 0,  1,  1,  0,  0,  0,  0,  1,  1,  1,  1,  0,  0,  0 }, /* RC */
  /* 7  */ { 0,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, /* NM */
  /* 8  */ { 0,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, /* RM */
  /* 9  */ { 0,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, /* IM */
  /* 10 */ { 0,  0,  0,  0,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0 }, /* HL */
  /* 11 */ { 0,  1,  1,  0,  0,  0,  0,  1,  1,  1,  1,  0,  0,  0 }, /* NK */
  /* 12 */ { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, /* VD */
  /* 13 */ { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }  /* HD */
};

bool const DEV_Composible[2][2] = {
/* Mode 0 */    {true, true }, // PASSTHROUGH = 0
/* Mode 1 */    {false, true}  // STRICT = 1
};

static constexpr sal_uInt16 getCharType(sal_Unicode x)
{
    return (x >= 0x0900 && x < 0x097f) ? devaCT[x - 0x0900] : ND_;
}

sal_Bool SAL_CALL
InputSequenceChecker_hi::checkInputSequence(const OUString& Text,
                                            sal_Int32       nStartPos,
                                            sal_Unicode     inputChar,
                                            sal_Int16       inputCheckMode)
{
    sal_Unicode currentChar = Text[nStartPos];
    sal_uInt16  ch1 = getCharType(inputChar);
    sal_uInt16  ch2 = getCharType(currentChar);

    return (DEV_Composible[inputCheckMode][dev_cell_check[ch2][ch1]]);
}

sal_Int32 SAL_CALL
InputSequenceChecker_hi::correctInputSequence(OUString& Text,
                                            sal_Int32       nStartPos,
                                            sal_Unicode     inputChar,
                                            sal_Int16       inputCheckMode)
{
    if (checkInputSequence(Text, nStartPos, inputChar, inputCheckMode))
        Text = Text.replaceAt(++nStartPos, 0, OUString(inputChar));
    else
        nStartPos=Text.getLength();
    return nStartPos;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
