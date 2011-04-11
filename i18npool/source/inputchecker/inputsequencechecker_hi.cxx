/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_i18npool.hxx"

#include <inputsequencechecker_hi.hxx>

using ::rtl::OUString;

namespace com {
namespace sun {
namespace star {
namespace i18n {

InputSequenceChecker_hi::InputSequenceChecker_hi()
{
    serviceName = "com.sun.star.i18n.InputSequenceChecker_hi";
}

InputSequenceChecker_hi::~InputSequenceChecker_hi()
{
}
/* Non-Defined Class type */
#define __ND        0

/*
 * Devanagari character type definitions
 */
#define __UP  1  // ChandraBindu & Anuswar
#define __NP  2  // Visarg
#define __IV    3  // Independant Vowels
#define __CN    4  // Consonants except _CK & _RC
#define __CK    5  // Consonants that can be followed by Nukta
#define __RC    6  // Ra
#define __NM    7  // Matra
#define __RM    8  // Ra + HAL
#define __IM    9  // Choti I Matra
#define __HL    10 // HAL
#define __NK    11 // Nukta
#define __VD    12 // Vedic
#define __HD    13 // Hindu Numerals

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
/* Mode 0 */    {sal_True, sal_True }, // PASSTHROUGH = 0
/* Mode 1 */    {sal_False, sal_True}  // STRICT = 1
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
