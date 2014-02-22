/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include <inputsequencechecker_hi.hxx>


namespace com {
namespace sun {
namespace star {
namespace i18n {

InputSequenceChecker_hi::InputSequenceChecker_hi()
    : InputSequenceCheckerImpl("com.sun.star.i18n.InputSequenceChecker_hi")
{
}

InputSequenceChecker_hi::~InputSequenceChecker_hi()
{
}
/* Non-Defined Class type */
#define __ND        0

/*
 * Devanagari character type definitions
 */
#define __UP  1  
#define __NP  2  
#define __IV    3  
#define __CN    4  
#define __CK    5  
#define __RC    6  
#define __NM    7  
#define __RM    8  
#define __IM    9  
#define __HL    10 
#define __NK    11 
#define __VD    12 
#define __HD    13 

/*
 * Devanagari character type table
 */
static const sal_uInt16 devaCT[128] = {
/*         0,    1,    2,    3,    4,    5,    6,    7,
           8,    9,    A,    B,    C,    D,    E,    F, */
/* 0900 */ __ND, __UP, __UP, __NP, __ND, __IV, __IV, __IV,
           __IV, __IV, __IV, __IV, __IV, __IV, __IV, __IV,
/* 0910 */ __IV, __IV, __IV, __IV, __IV, __CK, __CK, __CK,
           __CN, __CN, __CN, __CN, __CK, __CN, __CN, __CN,
/* 0920 */ __CN, __CK, __CK, __CN, __CN, __CN, __CN, __CN,
           __CN, __CN, __CN, __CK, __CN, __CN, __CN, __CN,
/* 0930 */ __RC, __CN, __CN, __CN, __CN, __CN, __CN, __CN,
           __CN, __CN, __ND, __ND, __NK, __VD, __NM, __IM,
/* 0940 */ __RM, __NM, __NM, __NM, __NM, __RM, __RM, __RM,
           __RM, __RM, __RM, __RM, __RM, __HL, __ND, __ND,
/* 0950 */ __ND, __VD, __VD, __VD, __VD, __ND, __ND, __ND,
           __CN, __CN, __CN, __CN, __CN, __CN, __CN, __CN,
/* 0960 */ __IV, __IV, __NM, __NM, __ND, __ND, __HD, __HD,
           __HD, __HD, __HD, __HD, __HD, __HD, __HD, __HD,
/* 0970 */ __ND, __ND, __ND, __ND, __ND, __ND, __ND, __ND,
           __ND, __ND, __ND, __ND, __ND, __ND, __ND, __ND,
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

sal_Bool _DEV_Composible[2][2] = {
/* Mode 0 */    {sal_True, sal_True }, 
/* Mode 1 */    {sal_False, sal_True}  
};

#define getCharType(x) \
    ((x >= 0x0900 && x < 0x097f) ? devaCT[x - 0x0900] : __ND)

sal_Bool SAL_CALL
InputSequenceChecker_hi::checkInputSequence(const OUString& Text,
                                            sal_Int32       nStartPos,
                                            sal_Unicode     inputChar,
                                            sal_Int16       inputCheckMode)
  throw(com::sun::star::uno::RuntimeException)
{
    sal_Unicode currentChar = Text[nStartPos];
  sal_uInt16  ch1 = getCharType(inputChar);
    sal_uInt16  ch2 = getCharType(currentChar);

    return (_DEV_Composible[inputCheckMode][dev_cell_check[ch2][ch1]]);
}

sal_Int32 SAL_CALL
InputSequenceChecker_hi::correctInputSequence(OUString& Text,
                                            sal_Int32       nStartPos,
                                            sal_Unicode     inputChar,
                                            sal_Int16       inputCheckMode)
  throw(com::sun::star::uno::RuntimeException)
{
    if (checkInputSequence(Text, nStartPos, inputChar, inputCheckMode))
        Text = Text.replaceAt(++nStartPos, 0, OUString(inputChar));
    else
        nStartPos=Text.getLength();
    return nStartPos;
}
} } } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
