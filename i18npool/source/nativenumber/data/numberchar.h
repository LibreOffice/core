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
static const sal_Int16 NumberChar_hi        = 12;
static const sal_Int16 NumberChar_th        = 13;
static const sal_Int16 NumberChar_or        = 14;
static const sal_Int16 NumberChar_mr        = 15;
static const sal_Int16 NumberChar_bn        = 16;
static const sal_Int16 NumberChar_pa        = 17;
static const sal_Int16 NumberChar_gu        = 18;
static const sal_Int16 NumberChar_ta        = 19;
static const sal_Int16 NumberChar_te        = 20;
static const sal_Int16 NumberChar_kn        = 21;
static const sal_Int16 NumberChar_ml        = 22;
static const sal_Int16 NumberChar_lo        = 23;
static const sal_Int16 NumberChar_bo        = 24;
static const sal_Int16 NumberChar_my        = 25;
static const sal_Int16 NumberChar_km        = 26;
static const sal_Int16 NumberChar_mn        = 27;
static const sal_Int16 NumberChar_he        = 28;
static const sal_Int16 NumberChar_ne        = 29;
static const sal_Int16 NumberChar_dz        = 30;
static const sal_Int16 NumberChar_Count     = 31;

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
    { 0x0966, 0x0967, 0x0968, 0x0969, 0x096A, 0x096B, 0x096C, 0x096D, 0x096E, 0x096F }, // Indic (Devanagari)
    { 0x0E50, 0x0E51, 0x0E52, 0x0E53, 0x0E54, 0x0E55, 0x0E56, 0x0E57, 0x0E58, 0x0E59 }, // Thai
    { 0x0866, 0x0867, 0x0868, 0x0869, 0x086A, 0x086B, 0x086C, 0x086D, 0x086E, 0x086F }, // Oriya
    { 0x0966, 0x0967, 0x0968, 0x0969, 0x096A, 0x096B, 0x096C, 0x096D, 0x096E, 0x096F }, // Marathi
    { 0x09E6, 0x09E7, 0x09E8, 0x09E9, 0x09EA, 0x09EB, 0x09EC, 0x09ED, 0x09EE, 0x09EF }, // Bengali
    { 0x0A66, 0x0A67, 0x0A68, 0x0A69, 0x0A6A, 0x0A6B, 0x0A6C, 0x0A6D, 0x0A6E, 0x0A6F }, // Punjabi (Gurmukhi)
    { 0x0AE6, 0x0AE7, 0x0AE8, 0x0AE9, 0x0AEA, 0x0AEB, 0x0AEC, 0x0AED, 0x0AEE, 0x0AEF }, // Gujarali
    { 0x0030, 0x0BE7, 0x0BE8, 0x0BE9, 0x0BEA, 0x0BEB, 0x0BEC, 0x0BED, 0x0BEE, 0x0BEF }, // Tamil
    { 0x0C66, 0x0C67, 0x0C68, 0x0C69, 0x0C6A, 0x0C6B, 0x0C6C, 0x0C6D, 0x0C6E, 0x0C6F }, // Telugu
    { 0x0CE6, 0x0CE7, 0x0CE8, 0x0CE9, 0x0CEA, 0x0CEB, 0x0CEC, 0x0CED, 0x0CEE, 0x0CEF }, // Kannada
    { 0x0DE6, 0x0DE7, 0x0DE8, 0x0DE9, 0x0DEA, 0x0DEB, 0x0DEC, 0x0DED, 0x0DEE, 0x0DEF }, // Malayalam
    { 0x0ED0, 0x0ED1, 0x0ED2, 0x0ED3, 0x0ED4, 0x0ED5, 0x0ED6, 0x0ED7, 0x0ED8, 0x0ED9 }, // Lao
    { 0x0F20, 0x0F21, 0x0F22, 0x0F23, 0x0F24, 0x0F25, 0x0F26, 0x0F27, 0x0F28, 0x0F29 }, // Tibetan
    { 0x1040, 0x1041, 0x1042, 0x1043, 0x1044, 0x1045, 0x1046, 0x1047, 0x1048, 0x1049 }, // Myammar
    { 0x17E0, 0x17E1, 0x17E2, 0x17E3, 0x17E4, 0x17E5, 0x17E6, 0x17E7, 0x17E8, 0x17E9 }, // Cambodian (Khmer)
    { 0x1810, 0x1811, 0x1812, 0x1813, 0x1814, 0x1815, 0x1816, 0x1817, 0x1818, 0x1819 }, // Mongolian
    { 0x0020, 0x05D0, 0x05D1, 0x05D2, 0x05D3, 0x05D4, 0x05D5, 0x05D6, 0x05D7, 0x05D8 }, // Hebrew
    { 0x0966, 0x0967, 0x0968, 0x0969, 0x096A, 0x096B, 0x096C, 0x096D, 0x096E, 0x096F }, // Nepali
    { 0x0F20, 0x0F21, 0x0F22, 0x0F23, 0x0F24, 0x0F25, 0x0F26, 0x0F27, 0x0F28, 0x0F29 }, // Dzongkha
};

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
    0x066B, // Est. Arabic Indic
    0x0000, // Indic (Devanagari)
    0x0000, // Thai
    0x0000, // Oriya
    0x0000, // Marathi
    0x0000, // Bengali
    0x0000, // Punjabi (Gurmukhi)
    0x0000, // Gujarali
    0x0000, // Tamil
    0x0000, // Telugu
    0x0000, // Kannada
    0x0000, // Malayalam
    0x0000, // Lao
    0x0000, // Tibetan
    0x0000, // Myammar
    0x0000, // Cambodian (Khmer)
    0x0000, // Mongolian
    0x0000, // Hebrew
    0x0000, // Nepali
    0x0000, // Dzongkha
};

static const sal_Unicode MinusChar[] = {
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
    0x0000, // Arabic Indic
    0x2212, // Est. Arabic Indic
    0x0000, // Indic
    0x0000, // Thai
    0x0000, // Oriya
    0x0000, // Marathi
    0x0000, // Bengali
    0x0000, // Punjabi
    0x0000, // Gujarali
    0x0000, // Tamil
    0x0000, // Telugu
    0x0000, // Kannada
    0x0000, // Malayalam
    0x0000, // Lao
    0x0000, // Tibetan
    0x0000, // Myammar
    0x0000, // Cambodian (Khmer)
    0x0000, // Mongolian
    0x0000, // Hebrew
    0x0000, // Nepali
    0x0000, // Dzongkha
};

static sal_uInt16 SeparatorChar[] = {
    0x002C, // Half Width (Ascii)
    0xFF0C, // Full Width
    0x3001, // Chinese Lower
    0x3001, // S. Chinese Upper
    0x3001, // T. Chinese Upper
    0x3001, // Japanese Modern
    0x3001, // Japanese Trad.
    0x3001, // Korean Lower
    0x3001, // Korean Upper
    0x3001, // Korean Hangul
    0x0000, // Arabic Indic
    0x066C, // Est. Arabic Indic
    0x0000, // Indic
    0x0000, // Thai
    0x0000, // Oriya
    0x0000, // Marathi
    0x0000, // Bengali
    0x0000, // Punjabi
    0x0000, // Gujarali
    0x0000, // Tamil
    0x0000, // Telugu
    0x0000, // Kannada
    0x0000, // Malayalam
    0x0000, // Lao
    0x0000, // Tibetan
    0x0000, // Myammar
    0x0000, // Cambodian (Khmer)
    0x0000, // Mongolian
    0x0000, // Hebrew
    0x0000, // Nepali
    0x0000, // Dzongkha
};

#define NUMBER_ZERO     NumberChar[NumberChar_HalfWidth][0] // 0x0030
#define NUMBER_ONE      NumberChar[NumberChar_HalfWidth][1]     // 0x0031
#define NUMBER_NINE     NumberChar[NumberChar_HalfWidth][9]     // 0x0039
#define isNumber(n)     ( NUMBER_ZERO <= n && n <= NUMBER_NINE )
#define isDecimal(n)    ( n == DecimalChar[NumberChar_HalfWidth] )
#define isMinus(n)      ( n == MinusChar[NumberChar_HalfWidth] )
#define isSeparator(n)  ( n == SeparatorChar[NumberChar_HalfWidth] )

const sal_Int16 Multiplier_Lower_zh     = 0;
const sal_Int16 Multiplier_Upper_zh     = 1;
const sal_Int16 Multiplier_Lower_zh_TW  = 2;
const sal_Int16 Multiplier_Upper_zh_TW  = 3;
const sal_Int16 Multiplier_Lower_ko     = 4;
const sal_Int16 Multiplier_Hangul_ko    = 5;
const sal_Int16 Multiplier_Modern_ja    = 6;
const sal_Int16 Multiplier_Traditional_ja = 7;
const sal_Int16 Multiplier_Count    = 8;

const sal_Int16 ExponentCount_6_CJK     = 6;

static const sal_Int16 MultiplierExponent_6_CJK[ExponentCount_6_CJK] = {
        12,     8,  4,  3,  2,  1
};
static const sal_Unicode MultiplierChar_6_CJK[][ExponentCount_6_CJK] = {
    {0x5146, 0x4EBF, 0x4E07, 0x5343, 0x767E, 0x5341},   // S. Chinese Lower
    {0x5146, 0x4EBF, 0x4E07, 0x4EDF, 0x4F70, 0x62FE},   // S. Chinese Upper
    {0x5146, 0x5104, 0x842C, 0x5343, 0x767E, 0x5341},   // T. Chinese Lower
    {0x5146, 0x5104, 0x842C, 0x4EDF, 0x4F70, 0x62FE},   // T. Chinese & Korean Upper
    {0x5146, 0x5104, 0x4E07, 0x5343, 0x767E, 0x5341},   // Korean Lower
    {0xC870, 0xC5B5, 0xB9CC, 0xCC9C, 0xBC31, 0xC2ED},   // Korean Hangul
    {0x5146, 0x5104, 0x4E07, 0x5343, 0x767E, 0x5341},   // Japanese Modern
    {0x5146, 0x5104, 0x842C, 0x9621, 0x767E, 0x62FE}    // Japanese Traditional
};

const sal_Int16 ExponentCount_2_CJK     = 2;

static const sal_Int16 MultiplierExponent_2_CJK[ExponentCount_2_CJK] = {
        8,  4,
};

static const sal_Unicode MultiplierChar_2_CJK[][ExponentCount_2_CJK] = {
    {0x4EBF, 0x4E07}, // S. Chinese Lower
    {0x4EBF, 0x4E07}, // S. Chinese Upper
    {0x5104, 0x842C}, // T. Chinese Lower
    {0x5104, 0x842C}, // T. Chinese & Korean Upper
    {0x5104, 0x4E07}, // Korean Lower
    {0xC5B5, 0xB9CC}, // Korean Hangul
    {0x5104, 0x4E07}, // Japanese Modern
    {0x5104, 0x842C}  // Japanese Traditional
};

const sal_Int16 ExponentCount_7_CJK     = 7;

static const sal_Int16 MultiplierExponent_7_CJK[ExponentCount_7_CJK] = {
        16, 12,     8,  4,  3,  2,  1
};
static const sal_Unicode MultiplierChar_7_CJK[][ExponentCount_7_CJK] = {
    {0x4EAC, 0x5146, 0x4EBF, 0x4E07, 0x5343, 0x767E, 0x5341},   // S. Chinese Lower
    {0x4EAC, 0x5146, 0x4EBF, 0x4E07, 0x4EDF, 0x4F70, 0x62FE},   // S. Chinese Upper
    {0x4EAC, 0x5146, 0x5104, 0x842C, 0x5343, 0x767E, 0x5341},   // T. Chinese Lower
    {0x4EAC, 0x5146, 0x5104, 0x842C, 0x4EDF, 0x4F70, 0x62FE},   // T. Chinese & Korean Upper
    {0x4EAC, 0x5146, 0x5104, 0x4E07, 0x5343, 0x767E, 0x5341},   // Korean Lower
    {0x4EAC, 0xC870, 0xC5B5, 0xB9CC, 0xCC9C, 0xBC31, 0xC2ED},   // Korean Hangul
    {0x4EAC, 0x5146, 0x5104, 0x4E07, 0x5343, 0x767E, 0x5341},   // Japanese Modern
    {0x4EAC, 0x5146, 0x5104, 0x842C, 0x9621, 0x767E, 0x62FE}    // Japanese Traditional
};

typedef struct {
    sal_Int16 ExponentCount;
    sal_Int16* MultiplierExponent;
    sal_Unicode *MultiplierChar;
} Multiplier;

} } } }

#endif // _L10N_TRANSLITERATION_NUMTOCHAR_H_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
