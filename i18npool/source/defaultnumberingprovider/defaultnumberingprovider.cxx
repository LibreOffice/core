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

#include <defaultnumberingprovider.hxx>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/style/NumberingType.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <osl/diagnose.h>
#include <rtl/ref.hxx>
#include <localedata.hxx>
#include <nativenumbersupplier.hxx>
#include <stdio.h>
#include <string.h>
#include <comphelper/propertysequence.hxx>
#include <cppuhelper/supportsservice.hxx>

// Cyrillic upper case
#define C_CYR_A "\xD0\x90"
#define C_CYR_B "\xD0\x91"
// Cyrillic lower case
#define S_CYR_A "\xD0\xB0"
#define S_CYR_B "\xD0\xB1"

//Greek upper case
#define C_GR_A "\xCE\x91"
#define C_GR_B "\xCE\x92"
//Greek lower case
#define S_GR_A "\xCE\xB1"
#define S_GR_B "\xCE\xB2"

//Hebrew
#define S_HE_ALEPH "\xD7\x90"
#define S_HE_YOD "\xD7\x99"
#define S_HE_QOF "\xD7\xA7"

//Arabic-Indic
#define S_AR_ONE "\xd9\xa1"
#define S_AR_TWO "\xd9\xa2"
#define S_AR_THREE "\xd9\xa3"

// East Arabic-Indic
#define S_FA_ONE "\xDB\xB1"
#define S_FA_TWO "\xDB\xB2"
#define S_FA_THREE "\xDB\xB3"

// Indic Devanagari
#define S_HI_ONE "\xDB\xB1"
#define S_HI_TWO "\xDB\xB2"
#define S_HI_THREE "\xDB\xB3"

#include <math.h>
#include <sal/macros.h>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/i18n/XTransliteration.hpp>
#include <com/sun/star/i18n/TransliterationType.hpp>
#include <com/sun/star/i18n/XLocaleData.hpp>

#include <bullet.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace ::com::sun::star::i18n;
using namespace com::sun::star::lang;

namespace i18npool {

static const sal_Unicode table_Alphabet_ar[] = {
    0x0623, 0x0628, 0x062A, 0x062B, 0x062C, 0x062D, 0x062E,
    0x062F, 0x0630, 0x0631, 0x0632, 0x0633, 0x0634, 0x0635,
    0x0636, 0x0637, 0x0638, 0x0639, 0x063A, 0x0641, 0x0642,
    0x0643, 0x0644, 0x0645, 0x0646, 0x0647, 0x0648, 0x0649
};

static const sal_Unicode table_Alphabet_ar_abjad[] = {
    0x0627, 0x0628, 0x062c, 0x062f, 0x0647, 0x0648, 0x0632, 0x062d,
    0x0637, 0x064a, 0x0643, 0x0644, 0x0645, 0x0646, 0x0633, 0x0639,
    0x0641, 0x0635, 0x0642, 0x0631, 0x0634, 0x062a, 0x062b, 0x062e,
    0x0630, 0x0636, 0x0638, 0x063a
};

static const sal_Unicode table_Alphabet_th[] = {
    0x0E01, 0x0E02, 0x0E04, 0x0E07,
    0x0E08, 0x0E09, 0x0E0A, 0x0E0B, 0x0E0C, 0x0E0D, 0x0E0E, 0x0E0F,
    0x0E10, 0x0E11, 0x0E12, 0x0E13, 0x0E14, 0x0E15, 0x0E16, 0x0E17,
    0x0E18, 0x0E19, 0x0E1A, 0x0E1B, 0x0E1C, 0x0E1D, 0x0E1E, 0x0E1F,
    0x0E20, 0x0E21, 0x0E22, 0x0E23, 0x0E24, 0x0E25, 0x0E26, 0x0E27,
    0x0E28, 0x0E29, 0x0E2A, 0x0E2B, 0x0E2C, 0x0E2D, 0x0E2E
};

static const sal_Unicode table_Alphabet_he[] = {
    0x05D0, 0x05D1, 0x05D2, 0x05D3, 0x05D4, 0x05D5, 0x05D6, 0x05D7,
    0x05D8, 0x05D9, 0x05DB, 0x05DC, 0x05DE, 0x05E0, 0x05E1, 0x05E2,
    0x05E4, 0x05E6, 0x05E7, 0x05E8, 0x05E9, 0x05EA
};

static const sal_Unicode table_Alphabet_ne[] = {
    0x0915, 0x0916, 0x0917, 0x0918, 0x0919, 0x091A, 0x091B, 0x091C,
    0x091D, 0x091E, 0x091F, 0x0920, 0x0921, 0x0922, 0x0923, 0x0924,
    0x0925, 0x0926, 0x0927, 0x0928, 0x092A, 0x092B, 0x092C, 0x092D,
    0x092E, 0x092F, 0x0930, 0x0932, 0x0935, 0x0936, 0x0937, 0x0938,
    0x0939
};

static const sal_Unicode table_Alphabet_km[] = {
    0x1780, 0x1781, 0x1782, 0x1783, 0x1784, 0x1785, 0x1786, 0x1787,
    0x1788, 0x1789, 0x178A, 0x178B, 0x178C, 0x178D, 0x178E, 0x178F,
    0x1790, 0x1791, 0x1792, 0x1793, 0x1794, 0x1795, 0x1796, 0x1797,
    0x1798, 0x1799, 0x179A, 0x179B, 0x179C, 0x179F,
    0x17A0, 0x17A1, 0x17A2
};

static const sal_Unicode table_Alphabet_lo[] = {
    0x0E81, 0x0E82, 0x0E84, 0x0E87, 0x0E88, 0x0E8A, 0x0E8D, 0x0E94,
    0x0E95, 0x0E96, 0x0E97, 0x0E99, 0x0E9A, 0x0E9B, 0x0E9C,
    0x0E9D, 0x0E9E, 0x0E9F, 0x0EA1, 0x0EA2, 0x0EA3, 0x0EA5, 0x0EA7,
    0x0EAA, 0x0EAB, 0x0EAD, 0x0EAE, 0x0EAF, 0x0EAE, 0x0EDC, 0x0EDD
};

static const sal_Unicode table_Alphabet_dz[] = {
    0x0F40, 0x0F41, 0x0F42, 0x0F44, 0x0F45, 0x0F46, 0x0F47, 0x0F49,
    0x0F4F, 0x0F50, 0x0F51, 0x0F53, 0x0F54, 0x0F55, 0x0F56, 0x0F58,
    0x0F59, 0x0F5A, 0x0F5B, 0x0F5D, 0x0F5E, 0x0F5F, 0x0F60, 0x0F61,
    0x0F62, 0x0F63, 0x0F64, 0x0F66, 0x0F67, 0x0F68
};

static const sal_Unicode table_Alphabet_my[] = {
    0x1000, 0x1001, 0x1002, 0x1003, 0x1004, 0x1005, 0x1006, 0x1007,
    0x1008,/*0x1009,*/0x100A, 0x100B, 0x100C, 0x100D, 0x100E, 0x100F,
    0x1010, 0x1011, 0x1012, 0x1013, 0x1014, 0x1015, 0x1016, 0x1017,
    0x1018, 0x1019, 0x101A, 0x101B, 0x101C, 0x101D, 0x101E, 0x101F,
    0x1020, 0x1021
};

// Bulgarian Cyrillic upper case letters
static const sal_Unicode table_CyrillicUpperLetter_bg[] = {
    0x0410, 0x0411, 0x0412, 0x0413, 0x0414, 0x0415, 0x0416, 0x0417, 0x0418,
    0x041A, 0x041B, 0x041C, 0x041D, 0x041E, 0x041F, 0x0420, 0x0421, 0x0422,
    0x0423, 0x0424, 0x0425, 0x0426, 0x0427, 0x0428, 0x0429, 0x042A, 0x042E,
    0x042F
};

// Bulgarian cyrillic lower case letters
static const sal_Unicode table_CyrillicLowerLetter_bg[] = {
    0x0430, 0x0431, 0x0432, 0x0433, 0x0434, 0x0435, 0x0436, 0x0437, 0x0438,
    0x043A, 0x043B, 0x043C, 0x043D, 0x043E, 0x043F, 0x0440, 0x0441, 0x0442,
    0x0443, 0x0444, 0x0445, 0x0446, 0x0447, 0x0448, 0x0449, 0x044A, 0x044E,
    0x044F
};

// Russian Cyrillic upper letters
static const sal_Unicode table_CyrillicUpperLetter_ru[] = {
    0x0410, 0x0411, 0x0412, 0x0413, 0x0414, 0x0415, 0x0416, 0x0417,
    0x0418, 0x041A, 0x041B, 0x041C, 0x041D, 0x041E, 0x041F, 0x0420,
    0x0421, 0x0422, 0x0423, 0x0424, 0x0425, 0x0426, 0x0427, 0x0428,
    0x0429, 0x042B, 0x042D, 0x042E, 0x042F
};

// Russian cyrillic lower letters
static const sal_Unicode table_CyrillicLowerLetter_ru[] = {
    0x0430, 0x0431, 0x0432, 0x0433, 0x0434, 0x0435, 0x0436, 0x0437,
    0x0438, 0x043A, 0x043B, 0x043C, 0x043D, 0x043E, 0x043F, 0x0440,
    0x0441, 0x0442, 0x0443, 0x0444, 0x0445, 0x0446, 0x0447, 0x0448,
    0x0449, 0x044B, 0x044D, 0x044E, 0x044F
};

// Serbian Cyrillic upper letters
static const sal_Unicode table_CyrillicUpperLetter_sr[] = {
    0x0410, 0x0411, 0x0412, 0x0413, 0x0414, 0x0402, 0x0415, 0x0416,
    0x0417, 0x0418, 0x0408, 0x041A, 0x041B, 0x0409, 0x041C, 0x041D,
    0x040A, 0x041E, 0x041F, 0x0420, 0x0421, 0x0422, 0x040B, 0x0423,
    0x0424, 0x0425, 0x0426, 0x0427, 0x040F, 0x0428
};

// Serbian cyrillic lower letters
static const sal_Unicode table_CyrillicLowerLetter_sr[] = {
    0x0430, 0x0431, 0x0432, 0x0433, 0x0434, 0x0452, 0x0435, 0x0436,
    0x0437, 0x0438, 0x0458, 0x043A, 0x043B, 0x0459, 0x043C, 0x043D,
    0x045A, 0x043E, 0x043F, 0x0440, 0x0441, 0x0442, 0x045B, 0x0443,
    0x0444, 0x0445, 0x0446, 0x0447, 0x045F, 0x0448
};

static const sal_Unicode table_GreekUpperLetter[] = {
    0x0391, 0x0392, 0x0393, 0x0394, 0x0395, 0x03DB, 0x0396, 0x0397, 0x0398,
    0x0399, 0x039A, 0x039B, 0x039C, 0x039D, 0x039E, 0x039F, 0x03A0, 0x03DF,
    0x03A1, 0x03A3, 0x03A4, 0x03A5, 0x03A6, 0x03A7, 0x03A8, 0x03A9, 0x03E0
};

static const sal_Unicode table_GreekLowerLetter[] = {
    0x03B1, 0x03B2, 0x03B3, 0x03B4, 0x03B5, 0x03DB, 0x03B6, 0x03B7, 0x03B8,
    0x03B9, 0x03BA, 0x03BB, 0x03BC, 0x03BD, 0x03BE, 0x03BF, 0x03C0, 0x03DF,
    0x03C1, 0x03C3, 0x03C4, 0x03C5, 0x03C6, 0x03C7, 0x03C8, 0x03C9, 0x03E1
};

static const sal_Unicode table_Alphabet_fa[] = {
    0x0622, 0x0628, 0x067E, 0x062A, 0x062B, 0x062C, 0x0686, 0x062D,
    0x062E, 0x062F, 0x0630, 0x0631, 0x0632, 0x0698, 0x0633, 0x0634,
    0x0635, 0x0636, 0x0637, 0x0638, 0x0639, 0x0640, 0x0641, 0x0642,
    0x06A9, 0x06AF, 0x0644, 0x0645, 0x0646, 0x0648, 0x0647, 0x06CC
};

static const sal_Unicode upperLetter[] = {
    0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
    0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50, 0x51, 0x52,
    0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A
};

static const sal_Unicode lowerLetter[] = {
    0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
    0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72,
    0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A
};

// Tables used for numbering in persian words
static const sal_Unicode table_PersianWord_decade1[][7]={
    {0},                                                 // 0
    {0x06cc, 0x06a9, 0},                                 // 1
    {0x062f, 0x0648, 0},                                 // 2
    {0x0633, 0x0647, 0},                                 // 3
    {0x0686, 0x0647, 0x0627, 0x0631, 0},                 // 4
    {0x067e, 0x0646, 0x062c, 0},                         // 5
    {0x0634, 0x0634, 0},                                 // 6
    {0x0647, 0x0641, 0x062a, 0},                         // 7
    {0x0647, 0x0634, 0x062a, 0},                         // 8
    {0x0646, 0x0647, 0},                                 // 9
    {0x062f, 0x0647, 0},                                 // 10
    {0x06cc, 0x0627, 0x0632, 0x062f, 0x0647, 0},         // 11
    {0x062f, 0x0648, 0x0627, 0x0632, 0x062f, 0x0647, 0}, // 12
    {0x0633, 0x06cc, 0x0632, 0x062f, 0x0647, 0},         // 13
    {0x0686, 0x0647, 0x0627, 0x0631, 0x062f, 0x0647, 0}, // 14
    {0x067e, 0x0627, 0x0646, 0x0632, 0x062f, 0x0647, 0}, // 15
    {0x0634, 0x0627, 0x0646, 0x0632, 0x062f, 0x0647, 0}, // 16
    {0x0647, 0x0641, 0x062f, 0x0647, 0},                 // 17
    {0x0647, 0x062c, 0x062f, 0x0647, 0},                 // 18
    {0x0646, 0x0648, 0x0632, 0x062f, 0x0647, 0}          // 19
};

static const sal_Unicode table_PersianWord_decade2[][6]={
    {0x0628, 0x06cc, 0x0633, 0x062a, 0},                 // 20
    {0x0633, 0x06cc, 0},                                 // 30
    {0x0686, 0x0647, 0x0644, 0},                         // 40
    {0x067e, 0x0646, 0x062c, 0x0627, 0x0647, 0},         // 50
    {0x0634, 0x0635, 0x062a, 0},                         // 60
    {0x0647, 0x0641, 0x062a, 0x0627, 0x062f, 0},         // 70
    {0x0647, 0x0634, 0x062a, 0x0627, 0x062f, 0},         // 80
    {0x0646, 0x0648, 0x062f, 0}                          // 90
};

static const sal_Unicode table_PersianWord_decade3[][7]={
    {0x0635, 0x062f, 0},                                 // 100
    {0x062f, 0x0648, 0x06cc, 0x0633, 0x062a, 0},         // 200
    {0x0633, 0x06cc, 0x0635, 0x062f, 0},                 // 300
    {0x0686, 0x0647, 0x0627, 0x0631, 0x0635, 0x062f, 0}, // 400
    {0x067e, 0x0627, 0x0646, 0x0635, 0x062f, 0},         // 500
    {0x0634, 0x0635, 0x062f, 0},                         // 600
    {0x0647, 0x0641, 0x062a, 0x0635, 0x062f, 0},         // 700
    {0x0647, 0x0634, 0x062a, 0x0635, 0x062f, 0},         // 800
    {0x0646, 0x0647, 0x0635, 0x062f, 0}                  // 900
};

static const sal_Unicode table_PersianWord_decadeX[][8]={
    {0x0647, 0x0632, 0x0627, 0x0631, 0},                        // 1000
    {0x0645, 0x06cc, 0x0644, 0x06cc, 0x0648, 0x0646, 0},        // 1000000
    {0x0645, 0x06cc, 0x0644, 0x06cc, 0x0627, 0x0631, 0x062f, 0} // 1000000000
};


DefaultNumberingProvider::DefaultNumberingProvider( const Reference < XComponentContext >& rxContext ) : m_xContext(rxContext)
{

}

DefaultNumberingProvider::~DefaultNumberingProvider()
{
}

Sequence< Reference<container::XIndexAccess> >
DefaultNumberingProvider::getDefaultOutlineNumberings(const Locale& rLocale )
{
     return LocaleDataImpl::get()->getOutlineNumberingLevels( rLocale );
}

Sequence< Sequence<beans::PropertyValue> >
DefaultNumberingProvider::getDefaultContinuousNumberingLevels( const Locale& rLocale )
{
     return LocaleDataImpl::get()->getContinuousNumberingLevels( rLocale );
}

static OUString toRoman( sal_Int32 n )
{

//              i, ii, iii, iv, v, vi, vii, vii, viii, ix
//                                                      (Dummy),1000,500,100,50,10,5,1
    static const sal_Char coRomanArr[] = "MDCLXVI--";       // +2 Dummy entries !!
    const sal_Char* cRomanStr = coRomanArr;
    sal_uInt16 nMask = 1000;
    sal_uInt32 nOver1000 = n / nMask;
    n -= ( nOver1000 * nMask );

    OUStringBuffer sTmp;
    while(nOver1000--)
        sTmp.append(*coRomanArr);

    while( nMask )
    {
        sal_uInt8 nNumber = sal_uInt8( n / nMask );
        sal_uInt8 nDiff = 1;
        n %= nMask;

        if( 5 < nNumber )
        {
            if( nNumber < 9 )
                sTmp.append(*(cRomanStr-1));
            ++nDiff;
            nNumber -= 5;
        }
        switch( nNumber )
        {
            case 3: sTmp.append(*cRomanStr);           [[fallthrough]];
            case 2: sTmp.append(*cRomanStr);           [[fallthrough]];
            case 1: sTmp.append(*cRomanStr);           break;
            case 4: sTmp.append(*cRomanStr).append(*(cRomanStr-nDiff)); break;
            case 5: sTmp.append(*(cRomanStr-nDiff));   break;
        }

        nMask /= 10;                    // to the next decade
        cRomanStr += 2;
    }
    return sTmp.makeStringAndClear();
}

// not used:

static
void lcl_formatChars( const sal_Unicode table[], int tableSize, int n, OUString& s )
{
     // string representation of n is appended to s.
     // if A=='A' then 0=>A, 1=>B, ..., 25=>Z, 26=>AA, 27=>AB, ...
     // if A=='a' then 0=>a, 1=>b, ..., 25=>z, 26=>aa, 27=>ab, ...

     if( n>=tableSize ) lcl_formatChars( table, tableSize, (n-tableSize)/tableSize, s );

     s += OUStringLiteral1( table[ n % tableSize ] );
}

static
void lcl_formatChars1( const sal_Unicode table[], int tableSize, int n, OUString& s )
{
     // string representation of n is appended to s.
     // if A=='A' then 0=>A, 1=>B, ..., 25=>Z, 26=>AA, 27=>BB, ...
     // if A=='a' then 0=>a, 1=>b, ..., 25=>z, 26=>aa, 27=>bb, ...

     int repeat_count = n / tableSize + 1;

     for( int i=0; i<repeat_count; i++ )
         s += OUStringLiteral1( table[ n%tableSize ] );
}

static
void lcl_formatChars2( const sal_Unicode table_capital[], const sal_Unicode table_small[], int tableSize, int n, OUString& s )
{
     // string representation of n is appended to s.
     // if A=='A' then 0=>A, 1=>B, ..., 25=>Z, 26=>Aa, 27=>Ab, ...

     if( n>=tableSize )
     {
          lcl_formatChars2( table_capital, table_small, tableSize, (n-tableSize)/tableSize, s );
          s += OUStringLiteral1( table_small[ n % tableSize ] );
     } else
          s += OUStringLiteral1( table_capital[ n % tableSize ] );
}

static
void lcl_formatChars3( const sal_Unicode table_capital[], const sal_Unicode table_small[], int tableSize, int n, OUString& s )
{
     // string representation of n is appended to s.
     // if A=='A' then 0=>A, 1=>B, ..., 25=>Z, 26=>Aa, 27=>Bb, ...

     int repeat_count = n / tableSize + 1;
     s += OUStringLiteral1( table_capital[ n%tableSize ] );

     for( int i=1; i<repeat_count; i++ )
         s += OUStringLiteral1( table_small[ n%tableSize ] );
}


/** Returns number's representation in persian words up to 999999999999
    respectively limited by sal_Int32 >=0.
    The caller assures that nNumber is not negative.

    @throws IllegalArgumentException
    @throws RuntimeException
 */
static
void lcl_formatPersianWord( sal_Int32 nNumber, OUString& rsResult )
{
    OUStringBuffer aTemp(64);
    static const sal_Unicode asPersianWord_conjunction_data[] = {0x20,0x0648,0x20,0};
    OUString asPersianWord_conjunction( asPersianWord_conjunction_data );
    unsigned char nSection = 0;

    while (int nPart = nNumber % 1000)
    {
        if (nSection)
        {
            if (nSection > SAL_N_ELEMENTS( table_PersianWord_decadeX))
                throw IllegalArgumentException();   // does not happen with sal_Int32
            aTemp.insert( 0, asPersianWord_conjunction).insert( 0, table_PersianWord_decadeX[nSection-1]);
        }

        unsigned int nDigit;
        if ((nDigit = nPart % 100) < 20)
        {
            if (!aTemp.isEmpty())
                aTemp.insert( 0, u' ');
            aTemp.insert( 0, table_PersianWord_decade1[nDigit]);
        }
        else
        {
            if ((nDigit = nPart % 10) != 0)
            {
                if (!aTemp.isEmpty())
                    aTemp.insert( 0, asPersianWord_conjunction);
                aTemp.insert( 0, table_PersianWord_decade1[nDigit]);
            }
            if ((nDigit = (nPart / 10) % 10) != 0)
            {
                if (!aTemp.isEmpty())
                    aTemp.insert( 0, asPersianWord_conjunction);
                aTemp.insert( 0, table_PersianWord_decade2[nDigit-2]);
            }
        }

        if ((nDigit = nPart / 100) != 0)
        {
            if (!aTemp.isEmpty())
                aTemp.insert( 0, asPersianWord_conjunction);
            aTemp.insert( 0, table_PersianWord_decade3[nDigit-1]);
        }

        nNumber /= 1000;
        nSection++;
    }
    rsResult += aTemp;
}


// Greek Letter Numbering

// KERAIA separates numerals from other text
#define STIGMA        u'\x03DB'
#define LEFT_KERAIA   u'\x0375'
#define MYRIAD_SYM    u'\x039C'
#define DOT_SYM       u'.'
#define SIGMA_OFFSET  19
#define TAU_OFFSET    20
#define MYRIAD        10000

/*
* Return the 1-999999 number's representation in the Greek numbering system.
* Adding a "left keraia" to represent numbers in the range 10000 ... 999999 is
* not orthodox, so it's better to use the myriad notation and call this method
* only for numbers up to 9999.
*/
static
OUString gr_smallNum(const sal_Unicode table[], int n)
{
    if (n > 9999)
        throw IllegalArgumentException();

    int i = 0;
    OUStringBuffer sb;
    for (int v = n; v > 0; v /= 10, i++) {
        int digit = v % 10;
        if (digit == 0)
            continue;

        sal_Unicode sign = table[(digit - 1) + 9 * (i % 3)];
        if (sign == STIGMA) {
            sb.insert(0, table[TAU_OFFSET]);
            sb.insert(0, table[SIGMA_OFFSET]);
        } else {
            sb.insert(0, sign);
        }

        if (i > 2)
            sb.insert(0, LEFT_KERAIA);
    }

    return sb.makeStringAndClear();
}

static
void lcl_formatCharsGR(const sal_Unicode table[], int n, OUString& s )
{
    OUStringBuffer sb;
    int myriadPower = 2;

    for (int divisor = MYRIAD * MYRIAD; divisor > 1; divisor /= MYRIAD, myriadPower--) {
        if (n > divisor - 1) {
            /*
             * Follow the Diophantus representation of:
             *   A myriad sign, M(10000) as many times as the power
             *   followed by the multiplier for the myriad
             *   followed by a dot
             *   followed by the rest
             *   This is enough for 32-bit integers
             */
            for (int i = 0; i < myriadPower; i++)
                sb.append(MYRIAD_SYM);

            sb.append(gr_smallNum(table, n/divisor));
            n %= divisor;

            if (n > 0)
                sb.append(DOT_SYM);
        }
    }
    sb.append(gr_smallNum(table,n));

    s += sb;
}

static
bool should_ignore( const OUString& s )
{
        // return true if blank or null
        return s == " " || (!s.isEmpty() && s[0]==0);
}

static
Any getPropertyByName( const Sequence<beans::PropertyValue>& aProperties,
                                                const char* name, bool bRequired )
{
        for( int i=0; i<aProperties.getLength(); i++ )
                if( aProperties[i].Name.equalsAscii(name) )
                        return aProperties[i].Value;
        if(bRequired)
            throw IllegalArgumentException();
        return Any();
}

//XNumberingFormatter
OUString
DefaultNumberingProvider::makeNumberingString( const Sequence<beans::PropertyValue>& aProperties,
                                               const Locale& aLocale )
{
     // the Sequence of PropertyValues is expected to have at least 4 elements:
     // elt Name              Type             purpose


     // 0.  "Prefix"          OUString
     // 1.  "NumberingType"   sal_Int16        type of formatting from style::NumberingType (roman, arabic, etc)
     // 2.  "Suffix"          OUString
     // ... ...               ...
     // n.  "Value"           sal_Int32        the number to be formatted
     // example:
     // given the Sequence { '(', NumberingType::ROMAN_UPPER, ')', ..., 7 }
     // makeNumberingString() returns the string "(VII)".

     // Q: why is the type of numType sal_Int16 instead of style::NumberingType?
     // A: an Any can't hold a style::NumberingType for some reason.
        //      add.: style::NumberingType holds constants of type sal_Int16, it's not an enum type

     sal_Int16 natNum = 0;
     sal_Int16 tableSize = 0;
     const sal_Unicode *table = nullptr;     // initialize to avoid compiler warning
     bool bRecycleSymbol = false;
     OUString sNatNumParams;
     Locale locale;

     OUString  prefix;
     sal_Int16        numType = -1; // type of formatting from style::NumberingType (roman, arabic, etc)
     OUString  suffix;
     sal_Int32        number = -12345; // the number that needs to be formatted.

//     int nProperties = aProperties.getLength();
//     int last        = nProperties-1;

     try {
        getPropertyByName(aProperties, "Prefix", false)      >>=prefix;
     } catch (Exception&) {
        //prefix _must_ be empty here!
     }
     try {
        getPropertyByName(aProperties, "Suffix", false)      >>=suffix;
     } catch (Exception&) {
        //suffix _must_ be empty here!
     }
     try {
        getPropertyByName(aProperties, "NumberingType", true)   >>=numType;
     } catch (Exception& ) {
        numType = -1;
     }
     try {
        getPropertyByName(aProperties, "Value", true)       >>=number;
     } catch (Exception& ) {
        number = -1;
     }

     if( number <= 0 )
          throw IllegalArgumentException();

     // start empty
     OUString result;

     // append prefix
     if( !should_ignore(prefix) ) result += prefix;

     // append formatted number
     using namespace style::NumberingType;
     switch( numType )
     {
          case CHARS_UPPER_LETTER:
               lcl_formatChars( upperLetter, 26, number-1, result ); // 1=>A, 2=>B, ..., 26=>Z, 27=>AA, 28=>AB, ...
               break;
          case CHARS_LOWER_LETTER:
               lcl_formatChars( lowerLetter, 26, number-1, result );
               break;
          case TEXT_NUMBER: // ordinal indicators (1st, 2nd, 3rd, ...)
               natNum = NativeNumberMode::NATNUM12;
               sNatNumParams = "capitalize ordinal-number";
               locale = aLocale;
               break;
          case TEXT_CARDINAL: // cardinal number names (One, Two, Three, ...)
               natNum = NativeNumberMode::NATNUM12;
               sNatNumParams = "capitalize";
               locale = aLocale;
               break;
          case TEXT_ORDINAL: // ordinal number names (First, Second, Third, ...)
               natNum = NativeNumberMode::NATNUM12;
               sNatNumParams = "capitalize ordinal";
               locale = aLocale;
               break;
          case ROMAN_UPPER:
               result += toRoman( number );
               break;
          case ROMAN_LOWER:
               result += toRoman( number ).toAsciiLowerCase();
               break;
          case ARABIC:
               result += OUString::number( number );
               break;
          case NUMBER_NONE:
               return OUString(); // ignore prefix and suffix
          case CHAR_SPECIAL:
               // apparently, we're supposed to return an empty string in this case...
               return OUString(); // ignore prefix and suffix
          case PAGE_DESCRIPTOR:
          case BITMAP:
               OSL_ASSERT(false);
               throw IllegalArgumentException();
          case CHARS_UPPER_LETTER_N:
               lcl_formatChars1( upperLetter, 26, number-1, result ); // 1=>A, 2=>B, ..., 26=>Z, 27=>AA, 28=>BB, ...
               break;
          case CHARS_LOWER_LETTER_N:
               lcl_formatChars1( lowerLetter, 26,  number-1, result ); // 1=>A, 2=>B, ..., 26=>Z, 27=>AA, 28=>BB, ...
               break;
          case TRANSLITERATION:
               try {
                    const OUString &tmp = OUString::number( number );
                    OUString transliteration;
                    getPropertyByName(aProperties, "Transliteration", true) >>= transliteration;
                    if ( !translit )
                        translit = new TransliterationImpl(m_xContext);
                    translit->loadModuleByImplName(transliteration, aLocale);
                    result += translit->transliterateString2String(tmp, 0, tmp.getLength());
               } catch (Exception& ) {
                    // When transliteration property is missing, return default number (bug #101141#)
                    result += OUString::number( number );
                    // OSL_ASSERT(0);
                    // throw IllegalArgumentException();
               }
               break;
          case NATIVE_NUMBERING:
                natNum = NativeNumberMode::NATNUM1;
                locale = aLocale;
                break;
          case FULLWIDTH_ARABIC:
                natNum = NativeNumberMode::NATNUM3;
                locale = aLocale;
                break;
          case NUMBER_LOWER_ZH:
                natNum = NativeNumberMode::NATNUM7;
                locale.Language = "zh";
                break;
          case NUMBER_UPPER_ZH:
                natNum = NativeNumberMode::NATNUM8;
                locale.Language = "zh";
                break;
          case NUMBER_UPPER_ZH_TW:
                natNum = NativeNumberMode::NATNUM8;
                locale.Language = "zh";
                locale.Country = "TW";
                break;
          case NUMBER_TRADITIONAL_JA:
                natNum = NativeNumberMode::NATNUM8;
                locale.Language = "ja";
                break;
          case NUMBER_UPPER_KO:
                natNum = NativeNumberMode::NATNUM8;
                locale.Language = "ko";
                break;
          case NUMBER_HANGUL_KO:
                natNum = NativeNumberMode::NATNUM11;
                locale.Language = "ko";
                break;

          case CIRCLE_NUMBER:
              table = table_CircledNumber;
              tableSize = SAL_N_ELEMENTS(table_CircledNumber);
              break;
          case TIAN_GAN_ZH:
              table = table_TianGan_zh;
              tableSize = SAL_N_ELEMENTS(table_TianGan_zh);
              break;
          case DI_ZI_ZH:
              table = table_DiZi_zh;
              tableSize = SAL_N_ELEMENTS(table_DiZi_zh);
              break;
          case AIU_FULLWIDTH_JA:
              table = table_AIUFullWidth_ja_JP;
              tableSize = SAL_N_ELEMENTS(table_AIUFullWidth_ja_JP);
              bRecycleSymbol = true;
              break;
          case AIU_HALFWIDTH_JA:
              table = table_AIUHalfWidth_ja_JP;
              tableSize = SAL_N_ELEMENTS(table_AIUHalfWidth_ja_JP);
              bRecycleSymbol = true;
              break;
          case IROHA_FULLWIDTH_JA:
              table = table_IROHAFullWidth_ja_JP;
              tableSize = SAL_N_ELEMENTS(table_IROHAFullWidth_ja_JP);
              bRecycleSymbol = true;
              break;
          case IROHA_HALFWIDTH_JA:
              table = table_IROHAHalfWidth_ja_JP;
              tableSize = SAL_N_ELEMENTS(table_IROHAHalfWidth_ja_JP);
              bRecycleSymbol = true;
              break;
          case HANGUL_JAMO_KO:
              table = table_HangulJamo_ko;
              tableSize = SAL_N_ELEMENTS(table_HangulJamo_ko);
              bRecycleSymbol = true;
              break;
          case HANGUL_SYLLABLE_KO:
              table = table_HangulSyllable_ko;
              tableSize = SAL_N_ELEMENTS(table_HangulSyllable_ko);
              bRecycleSymbol = true;
              break;
          case HANGUL_CIRCLED_JAMO_KO:
              table = table_HangulCircledJamo_ko;
              tableSize = SAL_N_ELEMENTS(table_HangulCircledJamo_ko);
              bRecycleSymbol = true;
              break;
          case HANGUL_CIRCLED_SYLLABLE_KO:
              table = table_HangulCircledSyllable_ko;
              tableSize = SAL_N_ELEMENTS(table_HangulCircledSyllable_ko);
              bRecycleSymbol = true;
              break;
          case CHARS_ARABIC:
              lcl_formatChars(table_Alphabet_ar, SAL_N_ELEMENTS(table_Alphabet_ar), number - 1, result);
              break;
          case CHARS_ARABIC_ABJAD:
              lcl_formatChars(table_Alphabet_ar_abjad, SAL_N_ELEMENTS(table_Alphabet_ar_abjad), number - 1, result);
              break;
          case NUMBER_ARABIC_INDIC:
              natNum = NativeNumberMode::NATNUM1;
              locale.Language = "ar";
              break;
          case NUMBER_EAST_ARABIC_INDIC:
              natNum = NativeNumberMode::NATNUM1;
              locale.Language = "fa";
              break;
          case NUMBER_INDIC_DEVANAGARI:
              natNum = NativeNumberMode::NATNUM1;
              locale.Language = "hi";
              break;
          case CHARS_THAI:
              lcl_formatChars(table_Alphabet_th, SAL_N_ELEMENTS(table_Alphabet_th), number - 1, result);
              break;
          case CHARS_HEBREW:
              lcl_formatChars(table_Alphabet_he, SAL_N_ELEMENTS(table_Alphabet_he), number - 1, result);
              break;
          case NUMBER_HEBREW:
              natNum = NativeNumberMode::NATNUM1;
              locale.Language = "he";
              break;
          case CHARS_NEPALI:
              lcl_formatChars(table_Alphabet_ne, SAL_N_ELEMENTS(table_Alphabet_ne), number - 1, result);
              break;
          case CHARS_KHMER:
              lcl_formatChars(table_Alphabet_km, SAL_N_ELEMENTS(table_Alphabet_km), number - 1, result);
              break;
          case CHARS_LAO:
              lcl_formatChars(table_Alphabet_lo, SAL_N_ELEMENTS(table_Alphabet_lo), number - 1, result);
              break;
          case CHARS_MYANMAR:
              lcl_formatChars(table_Alphabet_my, SAL_N_ELEMENTS(table_Alphabet_my), number - 1, result);
              break;
         case CHARS_TIBETAN:
              lcl_formatChars(table_Alphabet_dz, SAL_N_ELEMENTS(table_Alphabet_dz), number - 1, result);
              break;
         case CHARS_CYRILLIC_UPPER_LETTER_BG:
              lcl_formatChars2( table_CyrillicUpperLetter_bg,
                      table_CyrillicLowerLetter_bg,
                      SAL_N_ELEMENTS(table_CyrillicLowerLetter_bg), number-1,
                      result); // 1=>a, 2=>b, ..., 28=>z, 29=>Aa, 30=>Ab, ...
              break;
         case CHARS_CYRILLIC_LOWER_LETTER_BG:
              lcl_formatChars( table_CyrillicLowerLetter_bg,
                      SAL_N_ELEMENTS(table_CyrillicLowerLetter_bg), number-1,
                      result); // 1=>a, 2=>b, ..., 28=>z, 29=>aa, 30=>ab, ...
              break;
         case CHARS_CYRILLIC_UPPER_LETTER_N_BG:
              lcl_formatChars3( table_CyrillicUpperLetter_bg,
                      table_CyrillicLowerLetter_bg,
                      SAL_N_ELEMENTS(table_CyrillicLowerLetter_bg), number-1,
                      result); // 1=>a, 2=>b, ..., 28=>z, 29=>Aa, 30=>Bb, ...
              break;
         case CHARS_CYRILLIC_LOWER_LETTER_N_BG:
              lcl_formatChars1( table_CyrillicLowerLetter_bg,
                      SAL_N_ELEMENTS(table_CyrillicLowerLetter_bg), number-1,
                      result); // 1=>a, 2=>b, ..., 28=>z, 29=>aa, 30=>bb, ...
              break;
         case CHARS_CYRILLIC_UPPER_LETTER_RU:
              lcl_formatChars2( table_CyrillicUpperLetter_ru,
                      table_CyrillicLowerLetter_ru,
                      SAL_N_ELEMENTS(table_CyrillicLowerLetter_ru), number-1,
                      result); // 1=>a, 2=>b, ..., 27=>z, 28=>Aa, 29=>Ab, ...
              break;
         case CHARS_CYRILLIC_LOWER_LETTER_RU:
              lcl_formatChars( table_CyrillicLowerLetter_ru,
                      SAL_N_ELEMENTS(table_CyrillicLowerLetter_ru), number-1,
                      result); // 1=>a, 2=>b, ..., 27=>z, 28=>aa, 29=>ab, ...
              break;
         case CHARS_CYRILLIC_UPPER_LETTER_N_RU:
              lcl_formatChars3( table_CyrillicUpperLetter_ru,
                      table_CyrillicLowerLetter_ru,
                      SAL_N_ELEMENTS(table_CyrillicLowerLetter_ru), number-1,
                      result); // 1=>a, 2=>b, ..., 27=>z, 28=>Aa, 29=>Bb, ...
              break;
         case CHARS_CYRILLIC_LOWER_LETTER_N_RU:
              lcl_formatChars1( table_CyrillicLowerLetter_ru,
                      SAL_N_ELEMENTS(table_CyrillicLowerLetter_ru), number-1,
                      result); // 1=>a, 2=>b, ..., 27=>z, 28=>aa, 29=>bb, ...
              break;
         case CHARS_CYRILLIC_UPPER_LETTER_SR:
              lcl_formatChars2( table_CyrillicUpperLetter_sr,
                      table_CyrillicLowerLetter_sr,
                      SAL_N_ELEMENTS(table_CyrillicLowerLetter_sr), number-1,
                      result); // 1=>a, 2=>b, ..., 27=>z, 28=>Aa, 29=>Ab, ...
              break;
         case CHARS_CYRILLIC_LOWER_LETTER_SR:
              lcl_formatChars( table_CyrillicLowerLetter_sr,
                      SAL_N_ELEMENTS(table_CyrillicLowerLetter_sr), number-1,
                      result); // 1=>a, 2=>b, ..., 27=>z, 28=>aa, 29=>ab, ...
              break;
         case CHARS_CYRILLIC_UPPER_LETTER_N_SR:
              lcl_formatChars3( table_CyrillicUpperLetter_sr,
                      table_CyrillicLowerLetter_sr,
                      SAL_N_ELEMENTS(table_CyrillicLowerLetter_sr), number-1,
                      result); // 1=>a, 2=>b, ..., 27=>z, 28=>Aa, 29=>Bb, ...
              break;
         case CHARS_CYRILLIC_LOWER_LETTER_N_SR:
              lcl_formatChars1( table_CyrillicLowerLetter_sr,
                      SAL_N_ELEMENTS(table_CyrillicLowerLetter_sr), number-1,
                      result); // 1=>a, 2=>b, ..., 27=>z, 28=>aa, 29=>bb, ...
              break;

          case CHARS_GREEK_LOWER_LETTER:
              lcl_formatCharsGR( table_GreekLowerLetter, number, result);
              break;

          case CHARS_GREEK_UPPER_LETTER:
              lcl_formatCharsGR( table_GreekUpperLetter, number, result);
              break;

          case CHARS_PERSIAN:
              lcl_formatChars(table_Alphabet_fa, SAL_N_ELEMENTS(table_Alphabet_fa), number - 1, result);
              break;

          case CHARS_PERSIAN_WORD:
              lcl_formatPersianWord(number, result);
              break;

          default:
               OSL_ASSERT(false);
               throw IllegalArgumentException();
     }

     if (natNum) {
            rtl::Reference<NativeNumberSupplierService> xNatNum(new NativeNumberSupplierService);
            result += xNatNum->getNativeNumberStringParams(OUString::number(number), locale,
                                                                 natNum, sNatNumParams);
     } else if (tableSize) {
            if ( number > tableSize && !bRecycleSymbol)
                result += OUString::number( number);
            else
                result += OUString(&table[--number % tableSize], 1);
     }

     // append suffix
     if( !should_ignore(suffix) ) result += suffix;

     return result;
}

#define LANG_ALL        (1 << 0)
#define LANG_CJK        (1 << 1)
#define LANG_CTL        (1 << 2)

struct Supported_NumberingType
{
        sal_Int16               nType;
        const sal_Char* cSymbol;
        sal_Int16               langOption;
};
static const Supported_NumberingType aSupportedTypes[] =
{
        {style::NumberingType::CHARS_UPPER_LETTER,      "A", LANG_ALL},
        {style::NumberingType::CHARS_LOWER_LETTER,      "a", LANG_ALL},
        {style::NumberingType::ROMAN_UPPER,                     "I", LANG_ALL},
        {style::NumberingType::ROMAN_LOWER,                     "i", LANG_ALL},
        {style::NumberingType::ARABIC,                          "1", LANG_ALL},
        {style::NumberingType::NUMBER_NONE,                     "''", LANG_ALL},
        {style::NumberingType::CHAR_SPECIAL,                    "Bullet", LANG_ALL},
        {style::NumberingType::PAGE_DESCRIPTOR,                 "Page", LANG_ALL},
        {style::NumberingType::BITMAP,                          "Bitmap", LANG_ALL},
        {style::NumberingType::TEXT_NUMBER,             "1st", LANG_ALL},
        {style::NumberingType::TEXT_CARDINAL,           "One", LANG_ALL},
        {style::NumberingType::TEXT_ORDINAL,            "First", LANG_ALL},
        {style::NumberingType::CHARS_UPPER_LETTER_N,    "AAA", LANG_ALL},
        {style::NumberingType::CHARS_LOWER_LETTER_N,    "aaa", LANG_ALL},
        {style::NumberingType::NATIVE_NUMBERING,        "Native Numbering", LANG_CJK|LANG_CTL},
        {style::NumberingType::FULLWIDTH_ARABIC,        nullptr, LANG_CJK},
        {style::NumberingType::CIRCLE_NUMBER,           nullptr, LANG_CJK},
        {style::NumberingType::NUMBER_LOWER_ZH,         nullptr, LANG_CJK},
        {style::NumberingType::NUMBER_UPPER_ZH,         nullptr, LANG_CJK},
        {style::NumberingType::NUMBER_UPPER_ZH_TW,      nullptr, LANG_CJK},
        {style::NumberingType::TIAN_GAN_ZH,             nullptr, LANG_CJK},
        {style::NumberingType::DI_ZI_ZH,                nullptr, LANG_CJK},
        {style::NumberingType::NUMBER_TRADITIONAL_JA,   nullptr, LANG_CJK},
        {style::NumberingType::AIU_FULLWIDTH_JA,        nullptr, LANG_CJK},
        {style::NumberingType::AIU_HALFWIDTH_JA,        nullptr, LANG_CJK},
        {style::NumberingType::IROHA_FULLWIDTH_JA,      nullptr, LANG_CJK},
        {style::NumberingType::IROHA_HALFWIDTH_JA,      nullptr, LANG_CJK},
        {style::NumberingType::NUMBER_UPPER_KO,         nullptr, LANG_CJK},
        {style::NumberingType::NUMBER_HANGUL_KO,        nullptr, LANG_CJK},
        {style::NumberingType::HANGUL_JAMO_KO,          nullptr, LANG_CJK},
        {style::NumberingType::HANGUL_SYLLABLE_KO,      nullptr, LANG_CJK},
        {style::NumberingType::HANGUL_CIRCLED_JAMO_KO,  nullptr, LANG_CJK},
        {style::NumberingType::HANGUL_CIRCLED_SYLLABLE_KO,      nullptr, LANG_CJK},
        {style::NumberingType::CHARS_ARABIC,    nullptr, LANG_CTL},
        {style::NumberingType::CHARS_ARABIC_ABJAD,   nullptr, LANG_CTL},
        {style::NumberingType::NUMBER_ARABIC_INDIC,    S_AR_ONE ", " S_AR_TWO ", " S_AR_THREE ", ...", LANG_CTL},
        {style::NumberingType::NUMBER_EAST_ARABIC_INDIC,    S_FA_ONE ", " S_FA_TWO ", " S_FA_THREE ", ...", LANG_CTL},
        {style::NumberingType::NUMBER_INDIC_DEVANAGARI,    S_HI_ONE ", " S_HI_TWO ", " S_HI_THREE ", ...", LANG_CTL},
        {style::NumberingType::CHARS_THAI,      nullptr, LANG_CTL},
        {style::NumberingType::CHARS_HEBREW,    nullptr, LANG_CTL},
        {style::NumberingType::NUMBER_HEBREW,    S_HE_ALEPH ", " S_HE_YOD ", " S_HE_QOF ", ...", LANG_CTL},
        {style::NumberingType::CHARS_NEPALI,    nullptr, LANG_CTL},
        {style::NumberingType::CHARS_KHMER,     nullptr, LANG_CTL},
        {style::NumberingType::CHARS_LAO,       nullptr, LANG_CTL},
        {style::NumberingType::CHARS_MYANMAR,   nullptr, LANG_CTL},
        {style::NumberingType::CHARS_TIBETAN,   nullptr, LANG_CTL},
        {style::NumberingType::CHARS_CYRILLIC_UPPER_LETTER_BG,   C_CYR_A ", " C_CYR_B ", .., " C_CYR_A S_CYR_A ", " C_CYR_A S_CYR_B ", ... (bg)", LANG_ALL},
        {style::NumberingType::CHARS_CYRILLIC_LOWER_LETTER_BG,   S_CYR_A ", " S_CYR_B ", .., " S_CYR_A S_CYR_A ", " S_CYR_A S_CYR_B ", ... (bg)", LANG_ALL},
        {style::NumberingType::CHARS_CYRILLIC_UPPER_LETTER_N_BG, C_CYR_A ", " C_CYR_B ", .., " C_CYR_A S_CYR_A ", " C_CYR_B S_CYR_B ", ... (bg)", LANG_ALL},
        {style::NumberingType::CHARS_CYRILLIC_LOWER_LETTER_N_BG, S_CYR_A ", " S_CYR_B ", .., " S_CYR_A S_CYR_A ", " S_CYR_B S_CYR_B ", ... (bg)", LANG_ALL},
        {style::NumberingType::CHARS_CYRILLIC_UPPER_LETTER_RU,   C_CYR_A ", " C_CYR_B ", .., " C_CYR_A S_CYR_A ", " C_CYR_A S_CYR_B ", ... (ru)", LANG_ALL},
        {style::NumberingType::CHARS_CYRILLIC_LOWER_LETTER_RU,   S_CYR_A ", " S_CYR_B ", .., " S_CYR_A S_CYR_A ", " S_CYR_A S_CYR_B ", ... (ru)", LANG_ALL},
        {style::NumberingType::CHARS_CYRILLIC_UPPER_LETTER_N_RU, C_CYR_A ", " C_CYR_B ", .., " C_CYR_A S_CYR_A ", " C_CYR_B S_CYR_B ", ... (ru)", LANG_ALL},
        {style::NumberingType::CHARS_CYRILLIC_LOWER_LETTER_N_RU, S_CYR_A ", " S_CYR_B ", .., " S_CYR_A S_CYR_A ", " S_CYR_B S_CYR_B ", ... (ru)", LANG_ALL},
        {style::NumberingType::CHARS_CYRILLIC_UPPER_LETTER_SR,   C_CYR_A ", " C_CYR_B ", .., " C_CYR_A S_CYR_A ", " C_CYR_A S_CYR_B ", ... (sr)", LANG_ALL},
        {style::NumberingType::CHARS_CYRILLIC_LOWER_LETTER_SR,   S_CYR_A ", " S_CYR_B ", .., " S_CYR_A S_CYR_A ", " S_CYR_A S_CYR_B ", ... (sr)", LANG_ALL},
        {style::NumberingType::CHARS_CYRILLIC_UPPER_LETTER_N_SR, C_CYR_A ", " C_CYR_B ", .., " C_CYR_A S_CYR_A ", " C_CYR_B S_CYR_B ", ... (sr)", LANG_ALL},
        {style::NumberingType::CHARS_CYRILLIC_LOWER_LETTER_N_SR, S_CYR_A ", " S_CYR_B ", .., " S_CYR_A S_CYR_A ", " S_CYR_B S_CYR_B ", ... (sr)", LANG_ALL},
        {style::NumberingType::CHARS_PERSIAN,   nullptr, LANG_CTL},
        {style::NumberingType::CHARS_PERSIAN_WORD,   nullptr, LANG_CTL},
        {style::NumberingType::CHARS_GREEK_UPPER_LETTER,   C_GR_A ", " C_GR_B ", ... (gr)", LANG_ALL},
        {style::NumberingType::CHARS_GREEK_LOWER_LETTER,   S_GR_A ", " S_GR_B ", ... (gr)", LANG_ALL},
};
static const sal_Int32 nSupported_NumberingTypes = SAL_N_ELEMENTS(aSupportedTypes);

OUString DefaultNumberingProvider::makeNumberingIdentifier(sal_Int16 index)
{
    if (index < 0 || index >= nSupported_NumberingTypes)
        throw RuntimeException();

    if (aSupportedTypes[index].cSymbol)
        return OUString(aSupportedTypes[index].cSymbol, strlen(aSupportedTypes[index].cSymbol), RTL_TEXTENCODING_UTF8);
    else {
        OUStringBuffer result;
        Locale aLocale("en", OUString(), OUString());
        Sequence<beans::PropertyValue> aProperties(2);
        aProperties[0].Name = "NumberingType";
        aProperties[0].Value <<= aSupportedTypes[index].nType;
        aProperties[1].Name = "Value";
        for (sal_Int32 j = 1; j <= 3; j++) {
            aProperties[1].Value <<= j;
            result.append( makeNumberingString( aProperties, aLocale ) );
            result.append(", ");
        }
        result.append("...");
        return result.makeStringAndClear();
    }
}

bool
DefaultNumberingProvider::isScriptFlagEnabled(const OUString& aName)
{
    if (! xHierarchicalNameAccess.is()) {
        Reference< XMultiServiceFactory > xConfigProvider =
            configuration::theDefaultProvider::get(m_xContext);

        if (! xConfigProvider.is())
            throw RuntimeException();

        uno::Sequence<uno::Any> aArgs(comphelper::InitAnyPropertySequence(
        {
            {"nodepath", uno::Any(OUString("/org.openoffice.Office.Common/I18N"))}
        }));

        Reference<XInterface> xInterface = xConfigProvider->createInstanceWithArguments(
            "com.sun.star.configuration.ConfigurationAccess", aArgs);

        xHierarchicalNameAccess.set(xInterface, UNO_QUERY_THROW);
    }

    Any aEnabled = xHierarchicalNameAccess->getByHierarchicalName(aName);

    bool enabled = false;

    aEnabled >>= enabled;

    return enabled;
}

Sequence< sal_Int16 > DefaultNumberingProvider::getSupportedNumberingTypes(  )
{
    Sequence< sal_Int16 > aRet(nSupported_NumberingTypes );
    sal_Int16* pArray = aRet.getArray();

    bool cjkEnabled = isScriptFlagEnabled("CJK/CJKFont");
    bool ctlEnabled = isScriptFlagEnabled("CTL/CTLFont");

    for(sal_Int16 i = 0; i < nSupported_NumberingTypes; i++) {
        if ( (aSupportedTypes[i].langOption & LANG_ALL) ||
                ((aSupportedTypes[i].langOption & LANG_CJK) && cjkEnabled) ||
                ((aSupportedTypes[i].langOption & LANG_CTL) && ctlEnabled) )
            pArray[i] = aSupportedTypes[i].nType;
    }
    return aRet;
}

sal_Int16 DefaultNumberingProvider::getNumberingType( const OUString& rNumberingIdentifier )
{
    for(sal_Int16 i = 0; i < nSupported_NumberingTypes; i++)
        if(rNumberingIdentifier == makeNumberingIdentifier(i))
            return aSupportedTypes[i].nType;
    throw RuntimeException();
}

sal_Bool DefaultNumberingProvider::hasNumberingType( const OUString& rNumberingIdentifier )
{
    for(sal_Int16 i = 0; i < nSupported_NumberingTypes; i++)
        if(rNumberingIdentifier == makeNumberingIdentifier(i))
            return true;
    return false;
}

OUString DefaultNumberingProvider::getNumberingIdentifier( sal_Int16 nNumberingType )
{
    for(sal_Int16 i = 0; i < nSupported_NumberingTypes; i++)
        if(nNumberingType == aSupportedTypes[i].nType)
            return makeNumberingIdentifier(i);
    return OUString();
}

OUString DefaultNumberingProvider::getImplementationName()
{
    return OUString("com.sun.star.text.DefaultNumberingProvider");
}

sal_Bool DefaultNumberingProvider::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > DefaultNumberingProvider::getSupportedServiceNames()
{
    Sequence< OUString > aRet { "com.sun.star.text.DefaultNumberingProvider" };
    return aRet;
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_text_DefaultNumberingProvider_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new i18npool::DefaultNumberingProvider(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
