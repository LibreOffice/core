/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: numberchar.h,v $
 * $Revision: 1.7 $
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
#ifndef _L10N_TRANSLITERATION_NUMTOCHAR_H_
#define _L10N_TRANSLITERATION_NUMTOCHAR_H_

namespace com { namespace sun { namespace star { namespace i18n {

static const sal_Int16 NumberChar_HalfWidth     = 0;
static const sal_Int16 NumberChar_FullWidth     = 1;
static const sal_Int16 NumberChar_Lower_zh  = 2;
static const sal_Int16 NumberChar_Upper_zh  = 3;
static const sal_Int16 NumberChar_Upper_zh_TW   = 4;
static const sal_Int16 NumberChar_Modern_ja     = 5;
static const sal_Int16 NumberChar_Traditional_ja= 6;
static const sal_Int16 NumberChar_Lower_ko  = 7;
static const sal_Int16 NumberChar_Upper_ko  = 8;
static const sal_Int16 NumberChar_Hangul_ko     = 9;
static const sal_Int16 NumberChar_Indic_ar  = 10;
static const sal_Int16 NumberChar_EastIndic_ar  = 11;
static const sal_Int16 NumberChar_Indic_hi  = 12;
static const sal_Int16 NumberChar_th        = 13;
static const sal_Int16 NumberChar_Count     = 14;

static const sal_Unicode NumberChar[][10] = {
//  0   1   2   3   4   5   6   7   8   9
    { 0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039 }, // Half Width (Ascii)
    { 0xFF10, 0xFF11, 0xFF12, 0xFF13, 0xFF14, 0xFF15, 0xFF16, 0xFF17, 0xFF18, 0xFF19 }, // Full Width
    { 0x3007, 0x4E00, 0x4E8c, 0x4E09, 0x56DB, 0x4E94, 0x516D, 0x4E03, 0x516B, 0x4E5D }, // Chinese Lower
    { 0x96F6, 0x58F9, 0x8D30, 0x53C1, 0x8086, 0x4F0D, 0x9646, 0x67D2, 0x634C, 0x7396 }, // S. Chinese Upper
    { 0x96F6, 0x58F9, 0x8CB3, 0x53C3, 0x8086, 0x4F0D, 0x9678, 0x67D2, 0x634C, 0x7396 }, // T. Chinese Upper
    { 0x3007, 0x4E00, 0x4E8C, 0x4E09, 0x56DB, 0x4E94, 0x516D, 0x4E03, 0x516B, 0x4E5D }, // Japanese Modern
    { 0x96F6, 0x58F1, 0x5F10, 0x53C2, 0x56DB, 0x4F0D, 0x516D, 0x4E03, 0x516B, 0x4E5D }, // Japanese Trad.
    { 0x3007, 0x4E00, 0x4E8C, 0x4E09, 0x56DB, 0x4E94, 0x516D, 0x4E03, 0x516B, 0x4E5D }, // Korean Lower
    { 0xF9B2, 0x58F9, 0x8CB3, 0x53C3, 0x56DB, 0x4E94, 0x516D, 0x4E03, 0x516B, 0x4E5D }, // Korean Upper
    { 0xC601, 0xC77C, 0xC774, 0xC0BC, 0xC0AC, 0xC624, 0xC721, 0xCE60, 0xD314, 0xAD6C }, // Korean Hangul
    { 0x0660, 0x0661, 0x0662, 0x0663, 0x0664, 0x0665, 0x0666, 0x0667, 0x0668, 0x0669 }, // Arabic Indic
    { 0x06F0, 0x06F1, 0x06F2, 0x06F3, 0x06F4, 0x06F5, 0x06F6, 0x06F7, 0x06F8, 0x06F9 }, // Est. Arabic Indic
    { 0x0966, 0x0967, 0x0968, 0x0969, 0x096A, 0x096B, 0x096C, 0x096D, 0x096E, 0x096F }, // Indic
    { 0x0E50, 0x0E51, 0x0E52, 0x0E53, 0x0E54, 0x0E55, 0x0E56, 0x0E57, 0x0E58, 0x0E59 }  // Thai
};

// not used
#if 0
static sal_Unicode DecimalChar[] = {
    0x002E, // Half Width (Ascii)
    0xFF0E, // Full Width
    0xFF0E, // Chinese Lower
    0x70B9, // S. Chinese Upper
    0x9EDE, // T. Chinese Upper
    0x30FB, // Japanese Modern
    0x30FB, // Japanese Trad.
    0xFF0E, // Korean Lower
    0x9EDE, // Korean Upper
    0xC810, // Korean Hangul
    0x066B, // Arabic Indic
    0x002E, // Est. Arabic Indic
    0x002E, // Indic
    0x002E  // Thai
};
#endif

// not used
#if 0
static sal_Unicode MinusChar[] = {
    0x002D, // Half Width (Ascii)
    0xFF0D, // Full Width
    0xFF0D, // Chinese Lower
    0x8D1F, // S. Chinese Upper
    0x5069, // T. Chinese Upper
    0x2212, // Japanese Modern
    0x2212, // Japanese Trad.
    0xFF0D, // Korean Lower
    0x5069, // Korean Upper
    0xFF0D, // Korean Hangul ???
    0x002D, // Arabic Indic
    0x002D, // Est. Arabic Indic
    0x002D, // Indic
    0x002D, // Thai
};
#endif

#define NUMBER_ZERO     NumberChar[NumberChar_HalfWidth][0] // 0x0030
#define NUMBER_ONE      NumberChar[NumberChar_HalfWidth][1]     // 0x0031
#define NUMBER_NINE     NumberChar[NumberChar_HalfWidth][9]     // 0x0039
#define NUMBER_DECIMAL  DecimalChar[0]
#define NUMBER_MINUS    MinusChar[0]
#define isNumber(n)     ( NUMBER_ZERO <= n && n <= NUMBER_NINE )
#define isDecimal(n)    ( n == NUMBER_DECIMAL )
#define isMinus(n)  ( n == NUMBER_MINUS )

const sal_Int16 Multiplier_Lower_zh     = 0;
const sal_Int16 Multiplier_Upper_zh     = 1;
const sal_Int16 Multiplier_Lower_zh_TW  = 2;
const sal_Int16 Multiplier_Upper_zh_TW  = 3;
const sal_Int16 Multiplier_Hangul_ko    = 4;
const sal_Int16 Multiplier_Modern_ja    = 5;
const sal_Int16 Multiplier_Traditional_ja = 6;
const sal_Int16 Multiplier_Count    = 7;

const sal_Int16 ExponentCount_CJK   = 6;

// not used
#if 0
static sal_Int16 MultiplierExponent_CJK[ExponentCount_CJK] = {
        12,     8,  4,  3,  2,  1
};
static sal_Unicode MultiplierChar_CJK[][ExponentCount_CJK] = {
    0x5146, 0x4EBF, 0x4E07, 0x5343, 0x767E, 0x5341, // S. Chinese Lower
    0x5146, 0x4EBF, 0x4E07, 0x4EDF, 0x4F70, 0x62FE, // S. Chinese Upper
    0x5146, 0x5104, 0x842C, 0x5343, 0x767E, 0x5341, // T. Chinese & Korean Lower
    0x5146, 0x5104, 0x842C, 0x4EDF, 0x4F70, 0x62FE, // T. Chinese & Korean Upper
    0xC870, 0xC5B5, 0xB9CC, 0xCC9C, 0xBC31, 0xC2ED, // Korean Hangul
    0x5146, 0x5104, 0x4E07, 0x5343, 0x767E, 0x5341, // Japanese Modern
    0x5146, 0x5104, 0x842C, 0x9621, 0x767E, 0x62FE, // Japanese Traditional
};
#endif

const sal_Int16 ExponentCount_short_CJK     = 2;

// not used
#if 0
static sal_Int16 MultiplierExponent_short_CJK[ExponentCount_short_CJK] = {
        8,  4,
};

static sal_Unicode MultiplierChar_short_CJK[][ExponentCount_short_CJK] = {
    0x4EBF, 0x4E07, // S. Chinese Lower
    0x4EBF, 0x4E07, // S. Chinese Upper
    0x5104, 0x842C, // T. Chinese & Korean Lower
    0x5104, 0x842C, // T. Chinese & Korean Upper
    0xC5B5, 0xB9CC, // Korean Hangul
    0x5104, 0x4E07, // Japanese Modern
    0x5104, 0x842C, // Japanese Traditional
};
#endif

} } } }

#endif // _L10N_TRANSLITERATION_NUMTOCHAR_H_
