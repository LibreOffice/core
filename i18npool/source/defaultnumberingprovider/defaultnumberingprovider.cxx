/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: defaultnumberingprovider.cxx,v $
 * $Revision: 1.30 $
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
#include <defaultnumberingprovider.hxx>
#include <com/sun/star/style/NumberingType.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <localedata.hxx>
#include <nativenumbersupplier.hxx>
#include <stdio.h>
#include <string.h>

#if OSL_DEBUG_LEVEL == 0 && !defined(NDEBUG)
#define NDEBUG
#endif
#include <assert.h>

// Cyrillic upper case
#define C_CYR_A "\xD0\x90"
#define C_CYR_B "\xD0\x91"
// Cyrillic lower case
#define S_CYR_A "\xD0\xB0"
#define S_CYR_B "\xD0\xB1"

#include <math.h>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/i18n/XTransliteration.hpp>
#include <com/sun/star/i18n/TransliterationType.hpp>
#include <com/sun/star/i18n/TransliterationModulesNew.hpp>
#include <com/sun/star/i18n/XLocaleData.hpp>

#include <bullet.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace ::rtl;

namespace com { namespace sun { namespace star { namespace i18n {

static sal_Unicode table_Alphabet_ar[] = {
    0x0623, 0x0628, 0x062A, 0x062B, 0x062C, 0x062D, 0x062E,
    0x062F, 0x0630, 0x0631, 0x0632, 0x0633, 0x0634, 0x0635,
    0x0636, 0x0637, 0x0638, 0x0639, 0x063A, 0x0641, 0x0642,
    0x0643, 0x0644, 0x0645, 0x0646, 0x0647, 0x0648, 0x0649
};

static sal_Unicode table_Alphabet_th[] = {
    0x0E01, 0x0E02, 0x0E04, 0x0E07,
    0x0E08, 0x0E09, 0x0E0A, 0x0E0B, 0x0E0C, 0x0E0D, 0x0E0E, 0x0E0F,
    0x0E10, 0x0E11, 0x0E12, 0x0E13, 0x0E14, 0x0E15, 0x0E16, 0x0E17,
    0x0E18, 0x0E19, 0x0E1A, 0x0E1B, 0x0E1C, 0x0E1D, 0x0E1E, 0x0E1F,
    0x0E20, 0x0E21, 0x0E22, 0x0E23, 0x0E24, 0x0E25, 0x0E26, 0x0E27,
    0x0E28, 0x0E29, 0x0E2A, 0x0E2B, 0x0E2C, 0x0E2D, 0x0E2E
};

static sal_Unicode table_Alphabet_he[] = {
    0x05D0, 0x05D1, 0x05D2, 0x05D3, 0x05D4, 0x05D5, 0x05D6, 0x05D7,
    0x05D8, 0x05D9, 0x05DB, 0x05DC, 0x05DE, 0x05E0, 0x05E1, 0x05E2,
    0x05E4, 0x05E6, 0x05E7, 0x05E8, 0x05E9, 0x05EA
};

static sal_Unicode table_Alphabet_ne[] = {
    0x0915, 0x0916, 0x0917, 0x0918, 0x0919, 0x091A, 0x091B, 0x091C,
    0x091D, 0x091E, 0x091F, 0x0920, 0x0921, 0x0922, 0x0923, 0x0924,
    0x0925, 0x0926, 0x0927, 0x0928, 0x092A, 0x092B, 0x092C, 0x092D,
    0x092E, 0x092F, 0x0930, 0x0932, 0x0935, 0x0936, 0x0937, 0x0938,
    0x0939
};

static sal_Unicode table_Alphabet_km[] = {
    0x1780, 0x1781, 0x1782, 0x1783, 0x1784, 0x1785, 0x1786, 0x1787,
    0x1788, 0x1789, 0x178A, 0x178B, 0x178C, 0x178D, 0x178E, 0x178F,
    0x1790, 0x1791, 0x1792, 0x1793, 0x1794, 0x1795, 0x1796, 0x1797,
    0x1798, 0x1799, 0x179A, 0x179B, 0x179C, 0x179F,
    0x17A0, 0x17A1, 0x17A2
};

static sal_Unicode table_Alphabet_lo[] = {
    0x0E81, 0x0E82, 0x0E84, 0x0E87, 0x0E88, 0x0E8A, 0x0E8D, 0x0E94,
    0x0E95, 0x0E96, 0x0E97, 0x0E99, 0x0E9A, 0x0E9B, 0x0E9C,
    0x0E9D, 0x0E9E, 0x0E9F, 0x0EA1, 0x0EA2, 0x0EA3, 0x0EA5, 0x0EA7,
    0x0EAA, 0x0EAB, 0x0EAD, 0x0EAE, 0x0EAF, 0x0EAE, 0x0EDC, 0x0EDD
};

static sal_Unicode table_Alphabet_dz[] = {
    0x0F40, 0x0F41, 0x0F42, 0x0F44, 0x0F45, 0x0F46, 0x0F47, 0x0F49,
    0x0F4F, 0x0F50, 0x0F51, 0x0F53, 0x0F54, 0x0F55, 0x0F56, 0x0F58,
    0x0F59, 0x0F5A, 0x0F5B, 0x0F5D, 0x0F5E, 0x0F5F, 0x0F60, 0x0F61,
    0x0F62, 0x0F63, 0x0F64, 0x0F66, 0x0F67, 0x0F68
};


// Bulgarian Cyrillic upper case letters
static sal_Unicode table_CyrillicUpperLetter_bg[] = {
    0x0410, 0x0411, 0x0412, 0x0413, 0x0414, 0x0415, 0x0416, 0x0417, 0x0418,
    0x041A, 0x041B, 0x041C, 0x041D, 0x041E, 0x041F, 0x0420, 0x0421, 0x0422,
    0x0423, 0x0424, 0x0425, 0x0426, 0x0427, 0x0428, 0x0429, 0x042A, 0x042E,
    0x042F
};

// Bulgarian cyrillic lower case letters
static sal_Unicode table_CyrillicLowerLetter_bg[] = {
    0x0430, 0x0431, 0x0432, 0x0433, 0x0434, 0x0435, 0x0436, 0x0437, 0x0438,
    0x043A, 0x043B, 0x043C, 0x043D, 0x043E, 0x043F, 0x0440, 0x0441, 0x0442,
    0x0443, 0x0444, 0x0445, 0x0446, 0x0447, 0x0448, 0x0449, 0x044A, 0x044E,
    0x044F
};

// Russian Cyrillic upper letters
static sal_Unicode table_CyrillicUpperLetter_ru[] = {
    0x0410, 0x0411, 0x0412, 0x0413, 0x0414, 0x0415, 0x0416, 0x0417,
    0x0418, 0x041A, 0x041B, 0x041C, 0x041D, 0x041E, 0x041F, 0x0420,
    0x0421, 0x0422, 0x0423, 0x0424, 0x0425, 0x0426, 0x0427, 0x0428,
    0x0429, 0x042B, 0x042D, 0x042E, 0x042F
};

// Russian cyrillic lower letters
static sal_Unicode table_CyrillicLowerLetter_ru[] = {
    0x0430, 0x0431, 0x0432, 0x0433, 0x0434, 0x0435, 0x0436, 0x0437,
    0x0438, 0x043A, 0x043B, 0x043C, 0x043D, 0x043E, 0x043F, 0x0440,
    0x0441, 0x0442, 0x0443, 0x0444, 0x0445, 0x0446, 0x0447, 0x0448,
    0x0449, 0x044B, 0x044D, 0x044E, 0x044F
};

static sal_Unicode table_Alphabet_fa[] = {
    0x0622, 0x0628, 0x067E, 0x062A, 0x062B, 0x062C, 0x0686, 0x062D,
    0x062E, 0x062F, 0x0630, 0x0631, 0x0632, 0x0698, 0x0633, 0x0634,
    0x0635, 0x0636, 0x0637, 0x0638, 0x0639, 0x0640, 0x0641, 0x0642,
    0x06A9, 0x06AF, 0x0644, 0x0645, 0x0646, 0x0648, 0x0647, 0x06CC
};

static sal_Unicode upperLetter[] = {
    0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
    0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50, 0x51, 0x52,
    0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A
};

static sal_Unicode lowerLetter[] = {
    0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
    0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72,
    0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A
};

DefaultNumberingProvider::DefaultNumberingProvider( const Reference < XMultiServiceFactory >& xMSF ) : xSMgr(xMSF)
{
        translit = new TransliterationImpl(xMSF);
}

DefaultNumberingProvider::~DefaultNumberingProvider()
{
        delete translit;
}

Sequence< Reference<container::XIndexAccess> >
DefaultNumberingProvider::getDefaultOutlineNumberings(const Locale& rLocale ) throw(RuntimeException)
{
     return LocaleData().getOutlineNumberingLevels( rLocale );
}

Sequence< Sequence<beans::PropertyValue> >
DefaultNumberingProvider::getDefaultContinuousNumberingLevels( const Locale& rLocale ) throw(RuntimeException)
{
     return LocaleData().getContinuousNumberingLevels( rLocale );
}

OUString toRoman( sal_Int32 n )
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
        sTmp.append(sal_Unicode(*coRomanArr));

        while( nMask )
        {
                sal_uInt8 nZahl = sal_uInt8( n / nMask );
                sal_uInt8 nDiff = 1;
                n %= nMask;

                if( 5 < nZahl )
                {
                        if( nZahl < 9 )
                                sTmp.append(sal_Unicode(*(cRomanStr-1)));
                        ++nDiff;
                        nZahl -= 5;
                }
                switch( nZahl )
                {
                case 3: sTmp.append(sal_Unicode(*cRomanStr));           //no break!
                case 2: sTmp.append(sal_Unicode(*cRomanStr));           //no break!
                case 1: sTmp.append(sal_Unicode(*cRomanStr));           break;
                case 4: sTmp.append(sal_Unicode(*cRomanStr)).append(sal_Unicode(*(cRomanStr-nDiff))); break;
                case 5: sTmp.append(sal_Unicode(*(cRomanStr-nDiff)));   break;
                }

                nMask /= 10;                    // to the next decade
                cRomanStr += 2;
        }
        return sTmp.makeStringAndClear();
}

// not used:
#if 0

static
const char* expected_name( int i, int last )
{
     if(0);
     else if( i==0    ) return "Prefix";
     else if( i==1    ) return "NumberingType";
     else if( i==2    ) return "Suffix";
     else if( i==last ) return "Value";
     else { assert(0); return ""; }
}
static
const char* expected_type( int i, int last )
{
     if(0);
     else if( i==0    ) return "OUString";
     else if( i==1    ) return "sal_Int16";
     else if( i==2    ) return "OUString";
     else if( i==last ) return "sal_Int32";
     else { assert(0); return ""; }
}
static
void failedToConvert( int i, int last )
{
     throw IllegalArgumentException();
}

#endif

static
void lcl_formatChars( sal_Unicode table[], int tableSize, int n, OUString& s )
{
     // string representation of n is appended to s.
     // if A=='A' then 0=>A, 1=>B, ..., 25=>Z, 26=>AA, 27=>AB, ...
     // if A=='a' then 0=>a, 1=>b, ..., 25=>z, 26=>aa, 27=>ab, ...

     if( n>=tableSize ) lcl_formatChars( table, tableSize, (n-tableSize)/tableSize, s );

     s += OUString::valueOf( table[ n % tableSize ] );
}

static
void lcl_formatChars1( sal_Unicode table[], int tableSize, int n, OUString& s )
{
     // string representation of n is appended to s.
     // if A=='A' then 0=>A, 1=>B, ..., 25=>Z, 26=>AA, 27=>BB, ...
     // if A=='a' then 0=>a, 1=>b, ..., 25=>z, 26=>aa, 27=>bb, ...

     int repeat_count = n / tableSize + 1;

     for( int i=0; i<repeat_count; i++ )
         s += OUString::valueOf( table[ n%tableSize ] );
}

static
void lcl_formatChars2( sal_Unicode table_capital[], sal_Unicode table_small[], int tableSize, int n, OUString& s )
{
     // string representation of n is appended to s.
     // if A=='A' then 0=>A, 1=>B, ..., 25=>Z, 26=>Aa, 27=>Ab, ...

     if( n>=tableSize )
     {
          lcl_formatChars2( table_capital, table_small, tableSize, (n-tableSize)/tableSize, s );
          s += OUString::valueOf( table_small[ n % tableSize ] );
     } else
          s += OUString::valueOf( table_capital[ n % tableSize ] );
}

static
void lcl_formatChars3( sal_Unicode table_capital[], sal_Unicode table_small[], int tableSize, int n, OUString& s )
{
     // string representation of n is appended to s.
     // if A=='A' then 0=>A, 1=>B, ..., 25=>Z, 26=>Aa, 27=>Bb, ...

     int repeat_count = n / tableSize + 1;
     s += OUString::valueOf( table_capital[ n%tableSize ] );

     for( int i=1; i<repeat_count; i++ )
         s += OUString::valueOf( table_small[ n%tableSize ] );
}

static
int should_ignore( OUString s )
{
        // return true if blank or null
        return s.compareToAscii(" ")==0 || (s.getLength()>0 && s[0]==0);
}

static
Any getPropertyByName( const Sequence<beans::PropertyValue>& aProperties,
                                                const char* name, sal_Bool bRequired )
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
     throw( IllegalArgumentException, RuntimeException )
{
     // the Sequence of PropertyValues is expected to have at least 4 elements:
     // elt Name              Type             purpose
     // -----------------------------------------------------------------
     //
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
     sal_Unicode *table = NULL;     // initialize to avoid compiler warning
     sal_Bool recycleSymbol = sal_False;
     Locale locale;

     OUString  prefix;
     sal_Int16        numType = -1; // type of formatting from style::NumberingType (roman, arabic, etc)
     OUString  suffix;
     sal_Int32        number = -12345; // the number that needs to be formatted.

//     int nProperties = aProperties.getLength();
//     int last        = nProperties-1;

     try {
        getPropertyByName(aProperties, "Prefix", sal_False)      >>=prefix;
     } catch (Exception&) {
        //prefix _must_ be empty here!
     }
     try {
        getPropertyByName(aProperties, "Suffix", sal_False)      >>=suffix;
     } catch (Exception&) {
        //suffix _must_ be empty here!
     }
     try {
        getPropertyByName(aProperties, "NumberingType", sal_True)   >>=numType;
     } catch (Exception& ) {
        numType = -1;
     }
     try {
        getPropertyByName(aProperties, "Value", sal_True)       >>=number;
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
          case ROMAN_UPPER:
               result += toRoman( number );
               break;
          case ROMAN_LOWER:
               result += toRoman( number ).toAsciiLowerCase();
               break;
          case ARABIC:
               result += OUString::valueOf( number );
               break;
          case NUMBER_NONE:
               return OUString::createFromAscii(""); // ignore prefix and suffix
          case CHAR_SPECIAL:
               // apparently, we're supposed to return an empty string in this case...
               return OUString::createFromAscii(""); // ignore prefix and suffix
          case PAGE_DESCRIPTOR:
          case BITMAP:
               assert(0);
               throw IllegalArgumentException();
          case CHARS_UPPER_LETTER_N:
               lcl_formatChars1( upperLetter, 26, number-1, result ); // 1=>A, 2=>B, ..., 26=>Z, 27=>AA, 28=>BB, ...
               break;
          case CHARS_LOWER_LETTER_N:
               lcl_formatChars1( lowerLetter, 26,  number-1, result ); // 1=>A, 2=>B, ..., 26=>Z, 27=>AA, 28=>BB, ...
               break;
          case TRANSLITERATION:
               try {
                    const OUString &tmp = OUString::valueOf( number );
                    OUString transliteration;
                    getPropertyByName(aProperties, "Transliteration", sal_True) >>= transliteration;
                    translit->loadModuleByImplName(transliteration, aLocale);
                    result += translit->transliterateString2String(tmp, 0, tmp.getLength());
               } catch (Exception& ) {
                    // When translteration property is missing, return default number (bug #101141#)
                    result += OUString::valueOf( number );
                    // assert(0);
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
                locale.Language = OUString::createFromAscii("zh");
                break;
          case NUMBER_UPPER_ZH_TW:
                locale.Country = OUString::createFromAscii("TW");
          case NUMBER_UPPER_ZH:
                natNum = NativeNumberMode::NATNUM8;
                locale.Language = OUString::createFromAscii("zh");
                break;
          case NUMBER_TRADITIONAL_JA:
                natNum = NativeNumberMode::NATNUM8;
                locale.Language = OUString::createFromAscii("ja");
                break;
          case NUMBER_UPPER_KO:
                natNum = NativeNumberMode::NATNUM8;
                locale.Language = OUString::createFromAscii("ko");
                break;
          case NUMBER_HANGUL_KO:
                natNum = NativeNumberMode::NATNUM11;
                locale.Language = OUString::createFromAscii("ko");
                break;

          case CIRCLE_NUMBER:
              table = table_CircledNumber;
              tableSize = sizeof(table_CircledNumber) / sizeof(sal_Unicode);
              break;
          case TIAN_GAN_ZH:
              table = table_TianGan_zh;
              tableSize = sizeof(table_TianGan_zh) / sizeof(sal_Unicode);
              break;
          case DI_ZI_ZH:
              table = table_DiZi_zh;
              tableSize = sizeof(table_DiZi_zh) / sizeof(sal_Unicode);
              break;
          case AIU_FULLWIDTH_JA:
              table = table_AIUFullWidth_ja_JP;
              tableSize = sizeof(table_AIUFullWidth_ja_JP) / sizeof(sal_Unicode);
              recycleSymbol = sal_True;
              break;
          case AIU_HALFWIDTH_JA:
              table = table_AIUHalfWidth_ja_JP;
              tableSize = sizeof(table_AIUHalfWidth_ja_JP) / sizeof(sal_Unicode);
              recycleSymbol = sal_True;
              break;
          case IROHA_FULLWIDTH_JA:
              table = table_IROHAFullWidth_ja_JP;
              tableSize = sizeof(table_IROHAFullWidth_ja_JP) / sizeof(sal_Unicode);
              recycleSymbol = sal_True;
              break;
          case IROHA_HALFWIDTH_JA:
              table = table_IROHAHalfWidth_ja_JP;
              tableSize = sizeof(table_IROHAHalfWidth_ja_JP) / sizeof(sal_Unicode);
              recycleSymbol = sal_True;
              break;
          case HANGUL_JAMO_KO:
              table = table_HangulJamo_ko;
              tableSize = sizeof(table_HangulJamo_ko) / sizeof(sal_Unicode);
              recycleSymbol = sal_True;
              break;
          case HANGUL_SYLLABLE_KO:
              table = table_HangulSyllable_ko;
              tableSize = sizeof(table_HangulSyllable_ko) / sizeof(sal_Unicode);
              recycleSymbol = sal_True;
              break;
          case HANGUL_CIRCLED_JAMO_KO:
              table = table_HangulCircledJamo_ko;
              tableSize = sizeof(table_HangulCircledJamo_ko) / sizeof(sal_Unicode);
              recycleSymbol = sal_True;
              break;
          case HANGUL_CIRCLED_SYLLABLE_KO:
              table = table_HangulCircledSyllable_ko;
              tableSize = sizeof(table_HangulCircledSyllable_ko) / sizeof(sal_Unicode);
              recycleSymbol = sal_True;
              break;
          case CHARS_ARABIC:
              lcl_formatChars(table_Alphabet_ar, sizeof(table_Alphabet_ar) / sizeof(sal_Unicode), number - 1, result);
              break;
          case CHARS_THAI:
              lcl_formatChars(table_Alphabet_th, sizeof(table_Alphabet_th) / sizeof(sal_Unicode), number - 1, result);
              break;
          case CHARS_HEBREW:
              lcl_formatChars(table_Alphabet_he, sizeof(table_Alphabet_he) / sizeof(sal_Unicode), number - 1, result);
              break;
          case CHARS_NEPALI:
              lcl_formatChars(table_Alphabet_ne, sizeof(table_Alphabet_ne) / sizeof(sal_Unicode), number - 1, result);
              break;
          case CHARS_KHMER:
              lcl_formatChars(table_Alphabet_km, sizeof(table_Alphabet_km) / sizeof(sal_Unicode), number - 1, result);
              break;
          case CHARS_LAO:
              lcl_formatChars(table_Alphabet_lo, sizeof(table_Alphabet_lo) / sizeof(sal_Unicode), number - 1, result);
              break;
         case CHARS_TIBETAN:
              lcl_formatChars(table_Alphabet_dz, sizeof(table_Alphabet_dz) / sizeof(sal_Unicode), number - 1, result);
              break;
         case CHARS_CYRILLIC_UPPER_LETTER_BG:
              lcl_formatChars2( table_CyrillicUpperLetter_bg,
                      table_CyrillicLowerLetter_bg,
                      sizeof(table_CyrillicLowerLetter_bg) /
                      sizeof(table_CyrillicLowerLetter_bg[0]), number-1,
                      result); // 1=>a, 2=>b, ..., 28=>z, 29=>Aa, 30=>Ab, ...
              break;
         case CHARS_CYRILLIC_LOWER_LETTER_BG:
              lcl_formatChars( table_CyrillicLowerLetter_bg,
                      sizeof(table_CyrillicLowerLetter_bg) /
                      sizeof(table_CyrillicLowerLetter_bg[0]), number-1,
                      result); // 1=>a, 2=>b, ..., 28=>z, 29=>aa, 30=>ab, ...
              break;
         case CHARS_CYRILLIC_UPPER_LETTER_N_BG:
              lcl_formatChars3( table_CyrillicUpperLetter_bg,
                      table_CyrillicLowerLetter_bg,
                      sizeof(table_CyrillicLowerLetter_bg) /
                      sizeof(table_CyrillicLowerLetter_bg[0]), number-1,
                      result); // 1=>a, 2=>b, ..., 28=>z, 29=>Aa, 30=>Bb, ...
              break;
         case CHARS_CYRILLIC_LOWER_LETTER_N_BG:
              lcl_formatChars1( table_CyrillicLowerLetter_bg,
                      sizeof(table_CyrillicLowerLetter_bg) /
                      sizeof(table_CyrillicLowerLetter_bg[0]), number-1,
                      result); // 1=>a, 2=>b, ..., 28=>z, 29=>aa, 30=>bb, ...
              break;
         case CHARS_CYRILLIC_UPPER_LETTER_RU:
              lcl_formatChars2( table_CyrillicUpperLetter_ru,
                      table_CyrillicLowerLetter_ru,
                      sizeof(table_CyrillicLowerLetter_ru) /
                      sizeof(table_CyrillicLowerLetter_ru[0]), number-1,
                      result); // 1=>a, 2=>b, ..., 27=>z, 28=>Aa, 29=>Ab, ...
              break;
         case CHARS_CYRILLIC_LOWER_LETTER_RU:
              lcl_formatChars( table_CyrillicLowerLetter_ru,
                      sizeof(table_CyrillicLowerLetter_ru) /
                      sizeof(table_CyrillicLowerLetter_ru[0]), number-1,
                      result); // 1=>a, 2=>b, ..., 27=>z, 28=>aa, 29=>ab, ...
              break;
         case CHARS_CYRILLIC_UPPER_LETTER_N_RU:
              lcl_formatChars3( table_CyrillicUpperLetter_ru,
                      table_CyrillicLowerLetter_ru,
                      sizeof(table_CyrillicLowerLetter_ru) /
                      sizeof(table_CyrillicLowerLetter_ru[0]), number-1,
                      result); // 1=>a, 2=>b, ..., 27=>z, 28=>Aa, 29=>Bb, ...
              break;
         case CHARS_CYRILLIC_LOWER_LETTER_N_RU:
              lcl_formatChars1( table_CyrillicLowerLetter_ru,
                      sizeof(table_CyrillicLowerLetter_ru) /
                      sizeof(table_CyrillicLowerLetter_ru[0]), number-1,
                      result); // 1=>a, 2=>b, ..., 27=>z, 28=>aa, 29=>bb, ...
              break;
          case CHARS_PERSIAN:
              lcl_formatChars(table_Alphabet_fa, sizeof(table_Alphabet_fa) / sizeof(sal_Unicode), number - 1, result);
              break;

          default:
               assert(0);
               throw IllegalArgumentException();
      }

        if (natNum) {
            NativeNumberSupplier sNatNum;
            result += sNatNum.getNativeNumberString(OUString::valueOf( number ), locale, natNum);
        } else if (tableSize) {
            if ( number > tableSize && !recycleSymbol)
                result += OUString::valueOf( number);
            else
                result += OUString(&table[--number % tableSize], 1);
        }

        // append suffix
        if( !should_ignore(suffix) ) result += suffix;

        return result;
}
/* -----------------------------21.02.01 15:57--------------------------------

 ---------------------------------------------------------------------------*/

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
        {style::NumberingType::CHARS_UPPER_LETTER_N,    "AAA", LANG_ALL},
        {style::NumberingType::CHARS_LOWER_LETTER_N,    "aaa", LANG_ALL},
        {style::NumberingType::NATIVE_NUMBERING,        "Native Numbering", LANG_CJK|LANG_CTL},
        {style::NumberingType::FULLWIDTH_ARABIC,        NULL, LANG_CJK},
        {style::NumberingType::CIRCLE_NUMBER,           NULL, LANG_CJK},
        {style::NumberingType::NUMBER_LOWER_ZH,         NULL, LANG_CJK},
        {style::NumberingType::NUMBER_UPPER_ZH,         NULL, LANG_CJK},
        {style::NumberingType::NUMBER_UPPER_ZH_TW,      NULL, LANG_CJK},
        {style::NumberingType::TIAN_GAN_ZH,             NULL, LANG_CJK},
        {style::NumberingType::DI_ZI_ZH,                NULL, LANG_CJK},
        {style::NumberingType::NUMBER_TRADITIONAL_JA,   NULL, LANG_CJK},
        {style::NumberingType::AIU_FULLWIDTH_JA,        NULL, LANG_CJK},
        {style::NumberingType::AIU_HALFWIDTH_JA,        NULL, LANG_CJK},
        {style::NumberingType::IROHA_FULLWIDTH_JA,      NULL, LANG_CJK},
        {style::NumberingType::IROHA_HALFWIDTH_JA,      NULL, LANG_CJK},
        {style::NumberingType::NUMBER_UPPER_KO,         NULL, LANG_CJK},
        {style::NumberingType::NUMBER_HANGUL_KO,        NULL, LANG_CJK},
        {style::NumberingType::HANGUL_JAMO_KO,          NULL, LANG_CJK},
        {style::NumberingType::HANGUL_SYLLABLE_KO,      NULL, LANG_CJK},
        {style::NumberingType::HANGUL_CIRCLED_JAMO_KO,  NULL, LANG_CJK},
        {style::NumberingType::HANGUL_CIRCLED_SYLLABLE_KO,      NULL, LANG_CJK},
        {style::NumberingType::CHARS_ARABIC,    NULL, LANG_CTL},
        {style::NumberingType::CHARS_THAI,      NULL, LANG_CTL},
        {style::NumberingType::CHARS_HEBREW,    NULL, LANG_CTL},
        {style::NumberingType::CHARS_NEPALI,    NULL, LANG_CTL},
        {style::NumberingType::CHARS_KHMER,     NULL, LANG_CTL},
        {style::NumberingType::CHARS_LAO,       NULL, LANG_CTL},
        {style::NumberingType::CHARS_TIBETAN,   NULL, LANG_CTL},
        {style::NumberingType::CHARS_CYRILLIC_UPPER_LETTER_BG,   C_CYR_A ", " C_CYR_B ", .., " C_CYR_A S_CYR_A ", " C_CYR_A S_CYR_B ", ... (bg)", LANG_ALL},
        {style::NumberingType::CHARS_CYRILLIC_LOWER_LETTER_BG,   S_CYR_A ", " S_CYR_B ", .., " S_CYR_A S_CYR_A ", " S_CYR_A S_CYR_B ", ... (bg)", LANG_ALL},
        {style::NumberingType::CHARS_CYRILLIC_UPPER_LETTER_N_BG, C_CYR_A ", " C_CYR_B ", .., " C_CYR_A S_CYR_A ", " C_CYR_B S_CYR_B ", ... (bg)", LANG_ALL},
        {style::NumberingType::CHARS_CYRILLIC_LOWER_LETTER_N_BG, S_CYR_A ", " S_CYR_B ", .., " S_CYR_A S_CYR_A ", " S_CYR_B S_CYR_B ", ... (bg)", LANG_ALL},
        {style::NumberingType::CHARS_CYRILLIC_UPPER_LETTER_RU,   C_CYR_A ", " C_CYR_B ", .., " C_CYR_A S_CYR_A ", " C_CYR_A S_CYR_B ", ... (ru)", LANG_ALL},
        {style::NumberingType::CHARS_CYRILLIC_LOWER_LETTER_RU,   S_CYR_A ", " S_CYR_B ", .., " S_CYR_A S_CYR_A ", " S_CYR_A S_CYR_B ", ... (ru)", LANG_ALL},
        {style::NumberingType::CHARS_CYRILLIC_UPPER_LETTER_N_RU, C_CYR_A ", " C_CYR_B ", .., " C_CYR_A S_CYR_A ", " C_CYR_B S_CYR_B ", ... (ru)", LANG_ALL},
        {style::NumberingType::CHARS_CYRILLIC_LOWER_LETTER_N_RU, S_CYR_A ", " S_CYR_B ", .., " S_CYR_A S_CYR_A ", " S_CYR_B S_CYR_B ", ... (ru)", LANG_ALL},
        {style::NumberingType::CHARS_PERSIAN,   NULL, LANG_CTL},
};
static const sal_Int32 nSupported_NumberingTypes = sizeof(aSupportedTypes) / sizeof(Supported_NumberingType);
/* -----------------------------21.02.01 15:57--------------------------------

 ---------------------------------------------------------------------------*/

OUString DefaultNumberingProvider::makeNumberingIdentifier(sal_Int16 index)
                                throw(RuntimeException)
{
        if (aSupportedTypes[index].cSymbol)
            return OUString(aSupportedTypes[index].cSymbol, strlen(aSupportedTypes[index].cSymbol), RTL_TEXTENCODING_UTF8);
//            return OUString::createFromAscii(aSupportedTypes[index].cSymbol);
        else {
            OUString result;
            Locale aLocale(OUString::createFromAscii("en"), OUString(), OUString());
            Sequence<beans::PropertyValue> aProperties(2);
            aProperties[0].Name = OUString::createFromAscii("NumberingType");
            aProperties[0].Value <<= aSupportedTypes[index].nType;
            aProperties[1].Name = OUString::createFromAscii("Value");
            for (sal_Int32 j = 1; j <= 3; j++) {
                aProperties[1].Value <<= j;
                result += makeNumberingString( aProperties, aLocale );
                result += OUString::createFromAscii(", ");
            }
            result += OUString::createFromAscii("...");
            return result;
        }
}

sal_Bool SAL_CALL
DefaultNumberingProvider::isScriptFlagEnabled(const OUString& aName) throw(RuntimeException)
{
    if (! xHierarchicalNameAccess.is()) {
        Reference< XInterface > xInterface;

        xInterface = xSMgr->createInstance(OUString::createFromAscii("com.sun.star.configuration.ConfigurationProvider"));
        Reference< XMultiServiceFactory > xConfigProvider =
                Reference< XMultiServiceFactory >(xInterface, UNO_QUERY );

        if (! xConfigProvider.is())
            throw RuntimeException();

        Sequence< Any > aArgs(1);
        beans::PropertyValue aPath;
        aPath.Name = OUString::createFromAscii("nodepath");
        aPath.Value <<= OUString::createFromAscii("/org.openoffice.Office.Common/I18N"),
        aArgs[0] <<= aPath;

        xInterface = xConfigProvider->createInstanceWithArguments(
            OUString::createFromAscii("com.sun.star.configuration.ConfigurationAccess"), aArgs);

        xHierarchicalNameAccess.set(xInterface, UNO_QUERY);

        if (! xHierarchicalNameAccess.is())
            throw RuntimeException();
    }

    Any aEnabled = xHierarchicalNameAccess->getByHierarchicalName(aName);

    sal_Bool enabled = sal_False;

    aEnabled >>= enabled;

    return enabled;
}

Sequence< sal_Int16 > DefaultNumberingProvider::getSupportedNumberingTypes(  )
                                throw(RuntimeException)
{
        Sequence< sal_Int16 > aRet(nSupported_NumberingTypes );
        sal_Int16* pArray = aRet.getArray();

        sal_Bool cjkEnabled = isScriptFlagEnabled(OUString::createFromAscii("CJK/CJKFont"));
        sal_Bool ctlEnabled = isScriptFlagEnabled(OUString::createFromAscii("CTL/CTLFont"));

        for(sal_Int16 i = 0; i < nSupported_NumberingTypes; i++) {
            if ( (aSupportedTypes[i].langOption & LANG_ALL) ||
                    (aSupportedTypes[i].langOption & LANG_CJK) && cjkEnabled ||
                    (aSupportedTypes[i].langOption & LANG_CTL) && ctlEnabled)
                pArray[i] = aSupportedTypes[i].nType;
        }
        return aRet;
}
/* -----------------------------21.02.01 15:57--------------------------------

 ---------------------------------------------------------------------------*/
sal_Int16 DefaultNumberingProvider::getNumberingType( const OUString& rNumberingIdentifier )
                                throw(RuntimeException)
{
        for(sal_Int16 i = 0; i < nSupported_NumberingTypes; i++)
                if(rNumberingIdentifier.equals(makeNumberingIdentifier(i)))
                        return aSupportedTypes[i].nType;
        throw RuntimeException();
}
/* -----------------------------21.02.01 15:57--------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool DefaultNumberingProvider::hasNumberingType( const OUString& rNumberingIdentifier )
                                throw(RuntimeException)
{
        for(sal_Int16 i = 0; i < nSupported_NumberingTypes; i++)
                if(rNumberingIdentifier.equals(makeNumberingIdentifier(i)))
                        return sal_True;
        return sal_False;
}
/* -----------------------------21.02.01 15:57--------------------------------

 ---------------------------------------------------------------------------*/
OUString DefaultNumberingProvider::getNumberingIdentifier( sal_Int16 nNumberingType )
                                throw(RuntimeException)
{
        for(sal_Int16 i = 0; i < nSupported_NumberingTypes; i++)
            if(nNumberingType == aSupportedTypes[i].nType)
                return makeNumberingIdentifier(i);
        return OUString();
}
/* -----------------------------05.07.01 13:34--------------------------------

 ---------------------------------------------------------------------------*/
const sal_Char cDefaultNumberingProvider[] = "com.sun.star.text.DefaultNumberingProvider";
OUString DefaultNumberingProvider::getImplementationName(void)
                throw( RuntimeException )
{
    return OUString::createFromAscii(cDefaultNumberingProvider);
}
/* -----------------------------05.07.01 13:34--------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool DefaultNumberingProvider::supportsService(const rtl::OUString& rServiceName)
                throw( RuntimeException )
{
    return rServiceName.equalsAscii(cDefaultNumberingProvider);
}
/* -----------------------------05.07.01 13:34--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< OUString > DefaultNumberingProvider::getSupportedServiceNames(void)
                throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    aRet[0] = OUString::createFromAscii(cDefaultNumberingProvider);
    return aRet;
}

} } } }
