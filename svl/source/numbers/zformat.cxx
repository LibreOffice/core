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

#include <stdio.h>
#include <float.h>
#include <errno.h>
#include <stdlib.h>
#include <string_view>

#include <comphelper/string.hxx>
#include <sal/log.hxx>
#include <tools/debug.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <rtl/math.hxx>
#include <unotools/charclass.hxx>
#include <unotools/calendarwrapper.hxx>
#include <unotools/nativenumberwrapper.hxx>
#include <com/sun/star/i18n/CalendarFieldIndex.hpp>
#include <com/sun/star/i18n/CalendarDisplayIndex.hpp>
#include <com/sun/star/i18n/CalendarDisplayCode.hpp>
#include <com/sun/star/i18n/AmPmValue.hpp>
#include <com/sun/star/i18n/NativeNumberMode.hpp>
#include <com/sun/star/i18n/NativeNumberXmlAttributes2.hpp>

#include <svl/zformat.hxx>
#include "zforscan.hxx"

#include "zforfind.hxx"
#include <svl/zforlist.hxx>
#include <unotools/digitgroupingiterator.hxx>
#include <svl/nfsymbol.hxx>

#include <cmath>

using namespace svt;

namespace {

char const GREGORIAN[] = "gregorian";

const sal_uInt16 UPPER_PRECISION = 300; // entirely arbitrary...
const double EXP_LOWER_BOUND = 1.0E-4; // prefer scientific notation below this value.
const double EXP_ABS_UPPER_BOUND = 1.0E15;  // use exponential notation above that absolute value.
                                            // Back in time was E16 that lead
                                            // to display rounding errors, see
                                            // also sal/rtl/math.cxx
                                            // doubleToString()

} // namespace

const double D_MAX_U_INT32 = double(0xffffffff);      // 4294967295.0

const double D_MAX_D_BY_100  = 1.7E306;
const double D_MIN_M_BY_1000 = 2.3E-305;

static const sal_uInt8 cCharWidths[ 128-32 ] = {
    1,1,1,2,2,3,2,1,1,1,1,2,1,1,1,1,
    2,2,2,2,2,2,2,2,2,2,1,1,2,2,2,2,
    3,2,2,2,2,2,2,3,2,1,2,2,2,3,3,3,
    2,3,2,2,2,2,2,3,2,2,2,1,1,1,2,2,
    1,2,2,2,2,2,1,2,2,1,1,2,1,3,2,2,
    2,2,1,2,1,2,2,2,2,2,2,1,1,1,2,1
};

// static
sal_Int32 SvNumberformat::InsertBlanks( OUStringBuffer& r, sal_Int32 nPos, sal_Unicode c )
{
    if( c >= 32 )
    {
        int n = 2;   // Default for chars > 128 (HACK!)
        if( c <= 127 )
        {
            n = static_cast<int>(cCharWidths[ c - 32 ]);
        }
        while( n-- )
        {
            r.insert( nPos++, ' ');
        }
    }
    return nPos;
}

static long GetPrecExp( double fAbsVal )
{
    DBG_ASSERT( fAbsVal > 0.0, "GetPrecExp: fAbsVal <= 0.0" );
    if ( fAbsVal < 1e-7 || fAbsVal > 1e7 )
    {
        // Shear: whether it's faster or not, falls in between 1e6 and 1e7
        return static_cast<long>(floor( log10( fAbsVal ) )) + 1;
    }
    else
    {
        long nPrecExp = 1;
        while( fAbsVal < 1 )
        {
            fAbsVal *= 10;
            nPrecExp--;
        }
        while( fAbsVal >= 10 )
        {
            fAbsVal /= 10;
            nPrecExp++;
        }
        return nPrecExp;
    }
}

/**
 * SvNumberformatInfo
 * */

void ImpSvNumberformatInfo::Copy( const ImpSvNumberformatInfo& rNumFor, sal_uInt16 nCnt )
{
    for (sal_uInt16 i = 0; i < nCnt; ++i)
    {
        sStrArray[i]  = rNumFor.sStrArray[i];
        nTypeArray[i] = rNumFor.nTypeArray[i];
    }
    eScannedType = rNumFor.eScannedType;
    bThousand    = rNumFor.bThousand;
    nThousand    = rNumFor.nThousand;
    nCntPre      = rNumFor.nCntPre;
    nCntPost     = rNumFor.nCntPost;
    nCntExp      = rNumFor.nCntExp;
}

// static
sal_uInt8 SvNumberNatNum::MapDBNumToNatNum( sal_uInt8 nDBNum, LanguageType eLang, bool bDate )
{
    sal_uInt8 nNatNum = 0;
    eLang = MsLangId::getRealLanguage( eLang );  // resolve SYSTEM etc.
    eLang = primary(eLang);    // 10 bit primary language
    if ( bDate )
    {
        if ( nDBNum == 4 && eLang == primary(LANGUAGE_KOREAN) )
        {
            nNatNum = 9;
        }
        else if ( nDBNum <= 3 )
        {
            nNatNum = nDBNum;   // known to be good for: zh,ja,ko / 1,2,3
        }
    }
    else
    {
        switch ( nDBNum )
        {
        case 1:
            if ( eLang == primary(LANGUAGE_CHINESE) )
                nNatNum = 4;
            else if ( eLang == primary(LANGUAGE_JAPANESE) )
                nNatNum = 1;
            else if ( eLang == primary(LANGUAGE_KOREAN) )
                nNatNum = 1;
            break;
        case 2:
            if ( eLang == primary(LANGUAGE_CHINESE))
                nNatNum = 5;
            else if ( eLang == primary(LANGUAGE_JAPANESE) )
                nNatNum = 4;
            else if ( eLang == primary(LANGUAGE_KOREAN) )
                nNatNum = 2;
            break;
        case 3:
            if ( eLang == primary(LANGUAGE_CHINESE) )
                nNatNum = 6;
            else if ( eLang == primary(LANGUAGE_JAPANESE) )
                nNatNum = 5;
            else if ( eLang == primary(LANGUAGE_KOREAN) )
                nNatNum = 3;
            break;
        case 4:
            if ( eLang == primary(LANGUAGE_JAPANESE) )
                nNatNum = 7;
            else if ( eLang == primary(LANGUAGE_KOREAN) )
                nNatNum = 9;
            break;
        }
    }
    return nNatNum;
}

// static
sal_uInt8 SvNumberNatNum::MapNatNumToDBNum( sal_uInt8 nNatNum, LanguageType eLang, bool bDate )
{
    sal_uInt8 nDBNum = 0;
    eLang = MsLangId::getRealLanguage( eLang );  // resolve SYSTEM etc.
    eLang = primary(eLang);    // 10 bit primary language
    if ( bDate )
    {
        if ( nNatNum == 9 && eLang == primary(LANGUAGE_KOREAN) )
        {
            nDBNum = 4;
        }
        else if ( nNatNum <= 3 )
        {
            nDBNum = nNatNum;   // known to be good for: zh,ja,ko / 1,2,3
        }
    }
    else
    {
        switch ( nNatNum )
        {
        case 1:
            if ( eLang == primary(LANGUAGE_JAPANESE) )
                nDBNum = 1;
            else if ( eLang == primary(LANGUAGE_KOREAN) )
                nDBNum = 1;
            break;
        case 2:
            if ( eLang == primary(LANGUAGE_KOREAN) )
                nDBNum = 2;
            break;
        case 3:
            if ( eLang == primary(LANGUAGE_KOREAN) )
                nDBNum = 3;
            break;
        case 4:
            if ( eLang == primary(LANGUAGE_CHINESE) )
                nDBNum = 1;
            else if ( eLang == primary(LANGUAGE_JAPANESE) )
                nDBNum = 2;
            break;
        case 5:
            if ( eLang == primary(LANGUAGE_CHINESE) )
                nDBNum = 2;
            else if ( eLang == primary(LANGUAGE_JAPANESE) )
                nDBNum = 3;
            break;
        case 6:
            if ( eLang == primary(LANGUAGE_CHINESE) )
                nDBNum = 3;
            break;
        case 7:
            if ( eLang == primary(LANGUAGE_JAPANESE) )
                nDBNum = 4;
            break;
        case 8:
            break;
        case 9:
            if ( eLang == primary(LANGUAGE_KOREAN) )
                nDBNum = 4;
            break;
        case 10:
            break;
        case 11:
            break;
        }
    }
    return nDBNum;
}

/**
 * SvNumFor
 */

ImpSvNumFor::ImpSvNumFor()
{
    nStringsCnt = 0;
    aI.eScannedType = SvNumFormatType::UNDEFINED;
    aI.bThousand = false;
    aI.nThousand = 0;
    aI.nCntPre = 0;
    aI.nCntPost = 0;
    aI.nCntExp = 0;
    pColor = nullptr;
}

ImpSvNumFor::~ImpSvNumFor()
{
}

void ImpSvNumFor::Enlarge(sal_uInt16 nCnt)
{
    if ( nStringsCnt != nCnt )
    {
        nStringsCnt = nCnt;
        aI.nTypeArray.resize(nCnt);
        aI.sStrArray.resize(nCnt);
    }
}

void ImpSvNumFor::Copy( const ImpSvNumFor& rNumFor, ImpSvNumberformatScan* pSc )
{
    Enlarge( rNumFor.nStringsCnt );
    aI.Copy( rNumFor.aI, nStringsCnt );
    sColorName = rNumFor.sColorName;
    if ( pSc )
    {
        pColor = pSc->GetColor( sColorName );   // #121103# don't copy pointer between documents
    }
    else
    {
        pColor = rNumFor.pColor;
    }
    aNatNum = rNumFor.aNatNum;
}

bool ImpSvNumFor::HasNewCurrency() const
{
    for ( sal_uInt16 j=0; j<nStringsCnt; j++ )
    {
        if ( aI.nTypeArray[j] == NF_SYMBOLTYPE_CURRENCY )
        {
            return true;
        }
    }
    return false;
}

bool ImpSvNumFor::GetNewCurrencySymbol( OUString& rSymbol,
                                        OUString& rExtension ) const
{
    for ( sal_uInt16 j=0; j<nStringsCnt; j++ )
    {
        if ( aI.nTypeArray[j] == NF_SYMBOLTYPE_CURRENCY )
        {
            rSymbol = aI.sStrArray[j];
            if ( j < nStringsCnt-1 && aI.nTypeArray[j+1] == NF_SYMBOLTYPE_CURREXT )
            {
                rExtension = aI.sStrArray[j+1];
            }
            else
            {
                rExtension.clear();
            }
            return true;
        }
    }
    //! No Erase at rSymbol, rExtension
    return false;
}

/**
 * SvNumberformat
 */

enum BracketFormatSymbolType
{
    BRACKET_SYMBOLTYPE_FORMAT   = -1,   // subformat string
    BRACKET_SYMBOLTYPE_COLOR    = -2,   // color
    BRACKET_SYMBOLTYPE_ERROR    = -3,   // error
    BRACKET_SYMBOLTYPE_DBNUM1   = -4,   // DoubleByteNumber, represent numbers
    BRACKET_SYMBOLTYPE_DBNUM2   = -5,   // using CJK characters, Excel compatible
    BRACKET_SYMBOLTYPE_DBNUM3   = -6,
    BRACKET_SYMBOLTYPE_DBNUM4   = -7,
    BRACKET_SYMBOLTYPE_DBNUM5   = -8,
    BRACKET_SYMBOLTYPE_DBNUM6   = -9,
    BRACKET_SYMBOLTYPE_DBNUM7   = -10,
    BRACKET_SYMBOLTYPE_DBNUM8   = -11,
    BRACKET_SYMBOLTYPE_DBNUM9   = -12,
    BRACKET_SYMBOLTYPE_LOCALE   = -13,
    BRACKET_SYMBOLTYPE_NATNUM0  = -14,  // Our NativeNumber support, ASCII
    BRACKET_SYMBOLTYPE_NATNUM1  = -15,  // Our NativeNumber support, represent
    BRACKET_SYMBOLTYPE_NATNUM2  = -16,  // numbers using CJK, CTL, ...
    BRACKET_SYMBOLTYPE_NATNUM3  = -17,
    BRACKET_SYMBOLTYPE_NATNUM4  = -18,
    BRACKET_SYMBOLTYPE_NATNUM5  = -19,
    BRACKET_SYMBOLTYPE_NATNUM6  = -20,
    BRACKET_SYMBOLTYPE_NATNUM7  = -21,
    BRACKET_SYMBOLTYPE_NATNUM8  = -22,
    BRACKET_SYMBOLTYPE_NATNUM9  = -23,
    BRACKET_SYMBOLTYPE_NATNUM10 = -24,
    BRACKET_SYMBOLTYPE_NATNUM11 = -25,
    BRACKET_SYMBOLTYPE_NATNUM12 = -26,
    BRACKET_SYMBOLTYPE_NATNUM13 = -27,
    BRACKET_SYMBOLTYPE_NATNUM14 = -28,
    BRACKET_SYMBOLTYPE_NATNUM15 = -29,
    BRACKET_SYMBOLTYPE_NATNUM16 = -30,
    BRACKET_SYMBOLTYPE_NATNUM17 = -31,
    BRACKET_SYMBOLTYPE_NATNUM18 = -32,
    BRACKET_SYMBOLTYPE_NATNUM19 = -33
};

void SvNumberformat::ImpCopyNumberformat( const SvNumberformat& rFormat )
{
    sFormatstring = rFormat.sFormatstring;
    eType         = rFormat.eType;
    maLocale      = rFormat.maLocale;
    fLimit1       = rFormat.fLimit1;
    fLimit2       = rFormat.fLimit2;
    eOp1          = rFormat.eOp1;
    eOp2          = rFormat.eOp2;
    bStandard     = rFormat.bStandard;
    bIsUsed       = rFormat.bIsUsed;
    sComment      = rFormat.sComment;
    bAdditionalBuiltin = rFormat.bAdditionalBuiltin;

    // #121103# when copying between documents, get color pointers from own scanner
    ImpSvNumberformatScan* pColorSc = ( &rScan != &rFormat.rScan ) ? &rScan : nullptr;

    for (sal_uInt16 i = 0; i < 4; i++)
    {
        NumFor[i].Copy(rFormat.NumFor[i], pColorSc);
    }
}

SvNumberformat::SvNumberformat( SvNumberformat const & rFormat )
    : rScan(rFormat.rScan), bStarFlag( rFormat.bStarFlag )
{
    ImpCopyNumberformat( rFormat );
}

SvNumberformat::SvNumberformat( SvNumberformat const & rFormat, ImpSvNumberformatScan& rSc )
    : rScan(rSc)
    , bStarFlag( rFormat.bStarFlag )
{
    ImpCopyNumberformat( rFormat );
}

static bool lcl_SvNumberformat_IsBracketedPrefix( short nSymbolType )
{
    if ( nSymbolType > 0  )
    {
        return true; // conditions
    }
    switch ( nSymbolType )
    {
    case BRACKET_SYMBOLTYPE_COLOR :
    case BRACKET_SYMBOLTYPE_DBNUM1 :
    case BRACKET_SYMBOLTYPE_DBNUM2 :
    case BRACKET_SYMBOLTYPE_DBNUM3 :
    case BRACKET_SYMBOLTYPE_DBNUM4 :
    case BRACKET_SYMBOLTYPE_DBNUM5 :
    case BRACKET_SYMBOLTYPE_DBNUM6 :
    case BRACKET_SYMBOLTYPE_DBNUM7 :
    case BRACKET_SYMBOLTYPE_DBNUM8 :
    case BRACKET_SYMBOLTYPE_DBNUM9 :
    case BRACKET_SYMBOLTYPE_LOCALE :
    case BRACKET_SYMBOLTYPE_NATNUM0 :
    case BRACKET_SYMBOLTYPE_NATNUM1 :
    case BRACKET_SYMBOLTYPE_NATNUM2 :
    case BRACKET_SYMBOLTYPE_NATNUM3 :
    case BRACKET_SYMBOLTYPE_NATNUM4 :
    case BRACKET_SYMBOLTYPE_NATNUM5 :
    case BRACKET_SYMBOLTYPE_NATNUM6 :
    case BRACKET_SYMBOLTYPE_NATNUM7 :
    case BRACKET_SYMBOLTYPE_NATNUM8 :
    case BRACKET_SYMBOLTYPE_NATNUM9 :
    case BRACKET_SYMBOLTYPE_NATNUM10 :
    case BRACKET_SYMBOLTYPE_NATNUM11 :
    case BRACKET_SYMBOLTYPE_NATNUM12 :
    case BRACKET_SYMBOLTYPE_NATNUM13 :
    case BRACKET_SYMBOLTYPE_NATNUM14 :
    case BRACKET_SYMBOLTYPE_NATNUM15 :
    case BRACKET_SYMBOLTYPE_NATNUM16 :
    case BRACKET_SYMBOLTYPE_NATNUM17 :
    case BRACKET_SYMBOLTYPE_NATNUM18 :
    case BRACKET_SYMBOLTYPE_NATNUM19 :
        return true;
    }
    return false;
}

/** Import extended LCID from Excel
 */
OUString SvNumberformat::ImpObtainCalendarAndNumerals( OUStringBuffer& rString, sal_Int32 nPos,
                                                       LanguageType& nLang, const LocaleType& aTmpLocale )
{
    OUString sCalendar;
    sal_uInt16 nNatNum = 0;
    LanguageType nLocaleLang = MsLangId::getRealLanguage( maLocale.meLanguage );
    LanguageType nTmpLocaleLang = MsLangId::getRealLanguage( aTmpLocale.meLanguage );
    /* NOTE: enhancement to allow other possible locale dependent
     * calendars and numerals. BUT only if our locale data allows it! For LCID
     * numerals and calendars see
     * http://office.microsoft.com/en-us/excel/HA010346351033.aspx
     * Calendar is inserted after
     * all prefixes have been consumed as it is actually a format modifier
     * and not a prefix.
     * Currently calendars are tied to the locale of the entire number
     * format, e.g. [~buddhist] in en_US doesn't work.
     * => Having different locales in sub formats does not work!
     * */
    /* TODO: calendars could be tied to a sub format's NatNum info
     * instead, or even better be available for any locale. Needs a
     * different implementation of GetCal() and locale data calendars.
     * */
    switch ( aTmpLocale.mnCalendarType & 0x7F )
    {
        case 0x03 : // Gengou calendar
            sCalendar = "[~gengou]";
            // Only Japanese language support Gengou calendar
            if ( nLocaleLang != LANGUAGE_JAPANESE )
            {
                nLang = maLocale.meLanguage = LANGUAGE_JAPANESE;
            }
            break;
        case 0x05 : // unknown calendar
            break;
        case 0x06 : // Hijri calendar
        case 0x17 : // same?
            sCalendar = "[~hijri]";
            // Only Arabic or Farsi languages support Hijri calendar
            if ( ( primary( nLocaleLang ) != LANGUAGE_ARABIC_PRIMARY_ONLY )
                  && nLocaleLang != LANGUAGE_FARSI )
            {
                if ( ( primary( nTmpLocaleLang ) == LANGUAGE_ARABIC_PRIMARY_ONLY )
                      || nTmpLocaleLang == LANGUAGE_FARSI )
                {
                    nLang = maLocale.meLanguage = aTmpLocale.meLanguage;
                }
                else
                {
                    nLang = maLocale.meLanguage = LANGUAGE_ARABIC_SAUDI_ARABIA;
                }
            }
            break;
        case 0x07 : // Buddhist calendar
            sCalendar="[~buddhist]";
            // Only Thai or Lao languages support Buddhist calendar
            if ( nLocaleLang != LANGUAGE_THAI && nLocaleLang != LANGUAGE_LAO )
            {
                if ( nTmpLocaleLang == LANGUAGE_THAI || nTmpLocaleLang == LANGUAGE_LAO )
                {
                    nLang = maLocale.meLanguage = aTmpLocale.meLanguage;
                }
                else
                {
                    nLang = maLocale.meLanguage = LANGUAGE_THAI;
                }
            }
            break;
        case 0x08 : // Hebrew calendar
            sCalendar = "[~jewish]";
            // Many languages (but not all) support Jewish calendar
            // Unable to find any logic => keep same language
            break;
        case 0x0E : // unknown calendar
        case 0x0F : // unknown calendar
        case 0x10 : // Indian calendar (unsupported)
        case 0x11 : // unknown calendar
        case 0x12 : // unknown calendar
        case 0x13 : // unknown calendar
        default : // other calendars (see tdf#36038) are not handle by LibO
            break;
    }
    /** Reference language for each numeral ID */
    static const LanguageType aNumeralIDtoLanguage []=
    {
        LANGUAGE_DONTKNOW,              // 0x00
        LANGUAGE_ENGLISH_US,            // 0x01
        LANGUAGE_ARABIC_SAUDI_ARABIA,   // 0x02 + all Arabic
        LANGUAGE_FARSI,                 // 0x03
        LANGUAGE_HINDI,                 // 0x04 + Devanagari
        LANGUAGE_BENGALI,               // 0x05
        LANGUAGE_PUNJABI,               // 0x06
        LANGUAGE_GUJARATI,              // 0x07
        LANGUAGE_ODIA,                  // 0x08
        LANGUAGE_TAMIL,                 // 0x09
        LANGUAGE_TELUGU,                // 0x0A
        LANGUAGE_KANNADA,               // 0x0B
        LANGUAGE_MALAYALAM,             // 0x0C
        LANGUAGE_THAI,                  // 0x0D
        LANGUAGE_LAO,                   // 0x0E
        LANGUAGE_TIBETAN,               // 0x0F
        LANGUAGE_BURMESE,               // 0x10
        LANGUAGE_TIGRIGNA_ETHIOPIA,     // 0x11
        LANGUAGE_KHMER,                 // 0x12
        LANGUAGE_MONGOLIAN_MONGOLIAN_MONGOLIA, // 0x13
        LANGUAGE_DONTKNOW,              // 0x14
        LANGUAGE_DONTKNOW,              // 0x15
        LANGUAGE_DONTKNOW,              // 0x16
        LANGUAGE_DONTKNOW,              // 0x17
        LANGUAGE_DONTKNOW,              // 0x18
        LANGUAGE_DONTKNOW,              // 0x19
        LANGUAGE_DONTKNOW,              // 0x1A
        LANGUAGE_JAPANESE,              // 0x1B
        LANGUAGE_JAPANESE,              // 0x1C
        LANGUAGE_JAPANESE,              // 0x1D
        LANGUAGE_CHINESE_SIMPLIFIED,    // 0x1E
        LANGUAGE_CHINESE_SIMPLIFIED,    // 0x1F
        LANGUAGE_CHINESE_SIMPLIFIED,    // 0x20
        LANGUAGE_CHINESE_TRADITIONAL,   // 0x21
        LANGUAGE_CHINESE_TRADITIONAL,   // 0x22
        LANGUAGE_CHINESE_TRADITIONAL,   // 0x23
        LANGUAGE_KOREAN,                // 0x24
        LANGUAGE_KOREAN,                // 0x25
        LANGUAGE_KOREAN,                // 0x26
        LANGUAGE_KOREAN                 // 0x27
    };

    sal_uInt16 nNumeralID = aTmpLocale.mnNumeralShape & 0x7F;
    LanguageType nReferenceLanguage = nNumeralID <= 0x27 ? aNumeralIDtoLanguage[nNumeralID] : LANGUAGE_DONTKNOW;

    switch ( nNumeralID )
    {
        // Regular cases: all languages with same primary mask use same numerals
        case 0x03 : // Perso-Arabic (Farsi) numerals
        case 0x05 : // Bengali numerals
        case 0x06 : // Punjabi numerals
        case 0x07 : // Gujarati numerals
        case 0x08 : // Odia (Orya) numerals
        case 0x09 : // Tamil numerals
        case 0x0A : // Telugu numerals
        case 0x0B : // Kannada numerals
        case 0x0C : // Malayalam numerals
        case 0x0D : // Thai numerals
        case 0x0E : // Lao numerals
        case 0x0F : // Tibetan numerals
        case 0x10 : // Burmese (Myanmar) numerals
        case 0x11 : // Tigrigna (Ethiopia) numerals
        case 0x12 : // Khmer numerals
            if ( primary( nLocaleLang ) != primary( nReferenceLanguage ) )
            {
                if ( primary( nTmpLocaleLang ) == primary( nReferenceLanguage ) )
                {
                    nLang = maLocale.meLanguage = aTmpLocale.meLanguage;
                }
                else
                {
                    nLang = maLocale.meLanguage = nReferenceLanguage;
                }
            }
            break;
        // Special cases
        case 0x04 : // Devanagari (Hindi) numerals
            // same numerals (Devanagari) for languages with different primary masks
            if ( nLocaleLang != LANGUAGE_HINDI    && nLocaleLang != LANGUAGE_MARATHI
            && primary( nLocaleLang ) != primary( LANGUAGE_NEPALI ) )
            {
                if ( nTmpLocaleLang == LANGUAGE_HINDI || nTmpLocaleLang == LANGUAGE_MARATHI
                || primary( nTmpLocaleLang ) == primary( LANGUAGE_NEPALI ) )
                {
                    nLang = maLocale.meLanguage = aTmpLocale.meLanguage;
                }
                else
                {
                    nLang = maLocale.meLanguage = LANGUAGE_HINDI;
                }
            }
            break;
        case 0x13 : // Mongolian numerals
            // not all Mongolian languages use Mongolian numerals
            if ( nLocaleLang != LANGUAGE_MONGOLIAN_MONGOLIAN_MONGOLIA
              && nLocaleLang != LANGUAGE_MONGOLIAN_MONGOLIAN_CHINA
              && nLocaleLang != LANGUAGE_MONGOLIAN_MONGOLIAN_LSO )
            {
                if ( nTmpLocaleLang == LANGUAGE_MONGOLIAN_MONGOLIAN_MONGOLIA
                  || nTmpLocaleLang == LANGUAGE_MONGOLIAN_MONGOLIAN_CHINA
                  || nTmpLocaleLang == LANGUAGE_MONGOLIAN_MONGOLIAN_LSO )
                {
                    nLang = maLocale.meLanguage = aTmpLocale.meLanguage;
                }
                else
                {
                    nLang = maLocale.meLanguage = LANGUAGE_MONGOLIAN_MONGOLIAN_MONGOLIA;
                }
            }
            break;
        case 0x02 : // Eastern-Arabic numerals
            // all arabic primary mask + LANGUAGE_PUNJABI_ARABIC_LSO
            if ( primary( nLocaleLang ) != LANGUAGE_ARABIC_PRIMARY_ONLY
                && nLocaleLang != LANGUAGE_PUNJABI_ARABIC_LSO )
            {
                if ( primary( nTmpLocaleLang ) == LANGUAGE_ARABIC_PRIMARY_ONLY
                    || nTmpLocaleLang != LANGUAGE_PUNJABI_ARABIC_LSO )
                {
                    nLang = maLocale.meLanguage = aTmpLocale.meLanguage;
                }
                else
                {
                    nLang = maLocale.meLanguage = nReferenceLanguage;
                }
            }
            break;
        // CJK numerals
        case 0x1B : // simple Asian numerals, Japanese
        case 0x1C : // financial Asian numerals, Japanese
        case 0x1D : // Arabic fullwidth numerals, Japanese
        case 0x24 : // simple Asian numerals, Korean
        case 0x25 : // financial Asian numerals, Korean
        case 0x26 : // Arabic fullwidth numerals, Korean
        case 0x27 : // Korean Hangul numerals
            // Japanese and Korean are regular
            if ( primary( nLocaleLang ) != primary( nReferenceLanguage ) )
            {
                if ( primary( nTmpLocaleLang ) == primary( nReferenceLanguage ) )
                {
                    nLang = maLocale.meLanguage = aTmpLocale.meLanguage;
                }
                else
                {
                    nLang = maLocale.meLanguage = nReferenceLanguage;
                }
            }
            [[fallthrough]];
        case 0x1E : // simple Asian numerals, Chinese-PRC
        case 0x1F : // financial Asian numerals, Chinese-PRC
        case 0x20 : // Arabic fullwidth numerals, Chinese-PRC
        case 0x21 : // simple Asian numerals, Chinese-Taiwan
        case 0x22 : // financial Asian numerals, Chinese-Taiwan
        case 0x23 : // Arabic fullwidth numerals, Chinese-Taiwan
            nNatNum = nNumeralID == 0x27 ? 9 : ( ( nNumeralID - 0x1B ) % 3 ) + 1;
            // [NatNum1] simple numerals
            // [natNum2] financial numerals
            // [NatNum3] Arabic fullwidth numerals
            // Chinese simplified and Chinese traditional have same primary mask
            // Chinese-PRC
            if ( nReferenceLanguage == LANGUAGE_CHINESE_SIMPLIFIED
              && nLocaleLang != LANGUAGE_CHINESE_SIMPLIFIED
              && nLocaleLang != LANGUAGE_CHINESE_SINGAPORE
              && nLocaleLang != LANGUAGE_CHINESE_LSO )
            {
                if ( nTmpLocaleLang == LANGUAGE_CHINESE_SIMPLIFIED
                  || nTmpLocaleLang == LANGUAGE_CHINESE_SINGAPORE
                  || nTmpLocaleLang == LANGUAGE_CHINESE_LSO )
                {
                    nLang = maLocale.meLanguage = aTmpLocale.meLanguage;
                }
                else
                {
                    nLang = maLocale.meLanguage = LANGUAGE_CHINESE_SIMPLIFIED;
                }
            }
            // Chinese-Taiwan
            else if ( nReferenceLanguage == LANGUAGE_CHINESE_TRADITIONAL
                   && nLocaleLang != LANGUAGE_CHINESE_TRADITIONAL
                   && nLocaleLang != LANGUAGE_CHINESE_HONGKONG
                   && nLocaleLang != LANGUAGE_CHINESE_MACAU )
            {
                if ( nTmpLocaleLang == LANGUAGE_CHINESE_TRADITIONAL
                  || nTmpLocaleLang == LANGUAGE_CHINESE_HONGKONG
                  || nTmpLocaleLang == LANGUAGE_CHINESE_MACAU )
                {
                    nLang = maLocale.meLanguage = aTmpLocale.meLanguage;
                }
                else
                {
                    nLang = maLocale.meLanguage = LANGUAGE_CHINESE_TRADITIONAL;
                }
            }
            break;
    }
    if ( nNumeralID >= 0x02 && nNumeralID <= 0x13 )
        nNatNum = 1;
    if ( nNatNum )
        rString.insert( nPos, "[NatNum"+OUString::number(nNatNum)+"]");
    return sCalendar;
}

namespace
{
bool NatNumTakesParameters(sal_Int16 nNum)
{
    return (nNum == css::i18n::NativeNumberMode::NATNUM12);
}
}

SvNumberformat::SvNumberformat(OUString& rString,
                               ImpSvNumberformatScan* pSc,
                               ImpSvNumberInputScan* pISc,
                               sal_Int32& nCheckPos,
                               LanguageType& eLan)
        : rScan(*pSc)
        , bAdditionalBuiltin( false )
        , bStarFlag( false )
{
    rScan.ReplaceBooleanEquivalent( rString);

    OUStringBuffer sBuff(rString);

    // If the group (AKA thousand) separator is a No-Break Space (French)
    // replace all occurrences by a simple space.
    // The same for Narrow No-Break Space just in case some locale uses it.
    // The tokens will be changed to the LocaleData separator again later on.
    const OUString& rThSep = GetFormatter().GetNumThousandSep();
    if ( rThSep.getLength() == 1)
    {
        const sal_Unicode cNBSp = 0xA0;
        const sal_Unicode cNNBSp = 0x202F;
        if (rThSep[0] == cNBSp )
            sBuff.replace( cNBSp, ' ');
        else if (rThSep[0] == cNNBSp )
            sBuff.replace( cNNBSp, ' ');
    }

    if (rScan.GetConvertMode())
    {
        maLocale.meLanguage = rScan.GetNewLnge();
        eLan = maLocale.meLanguage; // Make sure to return switch
    }
    else
    {
        maLocale.meLanguage = eLan;
    }
    bStandard = false;
    bIsUsed = false;
    fLimit1 = 0.0;
    fLimit2 = 0.0;
    eOp1 = NUMBERFORMAT_OP_NO;
    eOp2 = NUMBERFORMAT_OP_NO;
    eType = SvNumFormatType::DEFINED;

    bool bCancel = false;
    bool bCondition = false;
    short eSymbolType;
    sal_Int32 nPos = 0;
    sal_Int32 nPosOld;
    nCheckPos = 0;

    // Split into 4 sub formats
    sal_uInt16 nIndex;
    for ( nIndex = 0; nIndex < 4 && !bCancel; nIndex++ )
    {
        // Original language/country may have to be reestablished
        if (rScan.GetConvertMode())
        {
            rScan.GetNumberformatter()->ChangeIntl(rScan.GetTmpLnge());
        }
        OUString sInsertCalendar; // a calendar resulting from parsing LCID
        OUString sStr;
        nPosOld = nPos; // Start position of substring
        // first get bracketed prefixes; e.g. conditions, color
        do
        {
            eSymbolType = ImpNextSymbol(sBuff, nPos, sStr);
            if (eSymbolType > 0) // condition
            {
                if ( nIndex == 0 && !bCondition )
                {
                    bCondition = true;
                    eOp1 = static_cast<SvNumberformatLimitOps>(eSymbolType);
                }
                else if ( nIndex == 1 && bCondition )
                {
                    eOp2 = static_cast<SvNumberformatLimitOps>(eSymbolType);
                }
                else                                // error
                {
                    bCancel = true;                 // break for
                    nCheckPos = nPosOld;
                }
                if (!bCancel)
                {
                    double fNumber;
                    sal_Int32 nCntChars = ImpGetNumber(sBuff, nPos, sStr);
                    if (nCntChars > 0)
                    {
                        SvNumFormatType F_Type = SvNumFormatType::UNDEFINED;
                        if (!pISc->IsNumberFormat(sStr, F_Type, fNumber, nullptr) ||
                            ( F_Type != SvNumFormatType::NUMBER &&
                              F_Type != SvNumFormatType::SCIENTIFIC) )
                        {
                            fNumber = 0.0;
                            nPos = nPos - nCntChars;
                            sBuff.remove(nPos, nCntChars);
                            sBuff.insert(nPos, '0');
                            nPos++;
                        }
                    }
                    else
                    {
                        fNumber = 0.0;
                        sBuff.insert(nPos++, '0');
                    }
                    if (nIndex == 0)
                    {
                        fLimit1 = fNumber;
                    }
                    else
                    {
                        fLimit2 = fNumber;
                    }
                    if ( nPos < sBuff.getLength() && sBuff[nPos] == ']' )
                    {
                        nPos++;
                    }
                    else
                    {
                        bCancel = true;             // break for
                        nCheckPos = nPos;
                    }
                }
                nPosOld = nPos;                     // position before string
            }
            else if ( lcl_SvNumberformat_IsBracketedPrefix( eSymbolType ) )
            {
                OUString sSymbol( sStr);
                switch ( eSymbolType )
                {
                case BRACKET_SYMBOLTYPE_COLOR :
                    if ( NumFor[nIndex].GetColor() != nullptr )
                    {                           // error, more than one color
                        bCancel = true;         // break for
                        nCheckPos = nPosOld;
                    }
                    else
                    {
                        Color* pColor = pSc->GetColor( sStr);
                        NumFor[nIndex].SetColor( pColor, sStr);
                        if (pColor == nullptr)
                        {                       // error
                            bCancel = true;     // break for
                            nCheckPos = nPosOld;
                        }
                    }
                    break;
                case BRACKET_SYMBOLTYPE_NATNUM0 :
                case BRACKET_SYMBOLTYPE_NATNUM1 :
                case BRACKET_SYMBOLTYPE_NATNUM2 :
                case BRACKET_SYMBOLTYPE_NATNUM3 :
                case BRACKET_SYMBOLTYPE_NATNUM4 :
                case BRACKET_SYMBOLTYPE_NATNUM5 :
                case BRACKET_SYMBOLTYPE_NATNUM6 :
                case BRACKET_SYMBOLTYPE_NATNUM7 :
                case BRACKET_SYMBOLTYPE_NATNUM8 :
                case BRACKET_SYMBOLTYPE_NATNUM9 :
                case BRACKET_SYMBOLTYPE_NATNUM10 :
                case BRACKET_SYMBOLTYPE_NATNUM11 :
                case BRACKET_SYMBOLTYPE_NATNUM12 :
                case BRACKET_SYMBOLTYPE_NATNUM13 :
                case BRACKET_SYMBOLTYPE_NATNUM14 :
                case BRACKET_SYMBOLTYPE_NATNUM15 :
                case BRACKET_SYMBOLTYPE_NATNUM16 :
                case BRACKET_SYMBOLTYPE_NATNUM17 :
                case BRACKET_SYMBOLTYPE_NATNUM18 :
                case BRACKET_SYMBOLTYPE_NATNUM19 :
                    if ( NumFor[nIndex].GetNatNum().IsSet() )
                    {
                        bCancel = true;         // break for
                        nCheckPos = nPosOld;
                    }
                    else
                    {
                        OUString sParams;
                        sal_Int32 nSpacePos = sStr.indexOf(' ');
                        if (nSpacePos >= 0)
                        {
                            sParams = sStr.copy(nSpacePos+1).trim();
                        }
                        //! eSymbolType is negative
                        sal_uInt8 nNum = static_cast<sal_uInt8>(0 - (eSymbolType - BRACKET_SYMBOLTYPE_NATNUM0));
                        if (!sParams.isEmpty() && !NatNumTakesParameters(nNum))
                        {
                            bCancel = true; // break for
                            nCheckPos = nPosOld;
                            break;
                        }
                        sStr = "NatNum" + OUString::number(nNum);
                        NumFor[nIndex].SetNatNumNum( nNum, false );
                        // NatNum12 supports arguments
                        if (nNum == 12)
                        {
                            if (sParams.isEmpty())
                                sParams = "cardinal"; // default NatNum12 format is "cardinal"
                            NumFor[nIndex].SetNatNumParams(sParams);
                            sStr += " " + sParams;
                        }
                    }
                    break;
                case BRACKET_SYMBOLTYPE_DBNUM1 :
                case BRACKET_SYMBOLTYPE_DBNUM2 :
                case BRACKET_SYMBOLTYPE_DBNUM3 :
                case BRACKET_SYMBOLTYPE_DBNUM4 :
                case BRACKET_SYMBOLTYPE_DBNUM5 :
                case BRACKET_SYMBOLTYPE_DBNUM6 :
                case BRACKET_SYMBOLTYPE_DBNUM7 :
                case BRACKET_SYMBOLTYPE_DBNUM8 :
                case BRACKET_SYMBOLTYPE_DBNUM9 :
                    if ( NumFor[nIndex].GetNatNum().IsSet() )
                    {
                        bCancel = true;         // break for
                        nCheckPos = nPosOld;
                    }
                    else
                    {
                        //! eSymbolType is negative
                        sal_uInt8 nNum = static_cast<sal_uInt8>(1 - (eSymbolType - BRACKET_SYMBOLTYPE_DBNUM1));
                        sStr = "DBNum" + OUStringLiteral1('0' + nNum);
                        NumFor[nIndex].SetNatNumNum( nNum, true );
                    }
                    break;
                case BRACKET_SYMBOLTYPE_LOCALE :
                    if ( NumFor[nIndex].GetNatNum().GetLang() != LANGUAGE_DONTKNOW ||
                         sBuff[nPos-1] != ']' )
                        // Check also for ']' to avoid pulling in
                        // locale data for the preview string for not
                        // yet completed LCIDs in the dialog.
                    {
                        bCancel = true;         // break for
                        nCheckPos = nPosOld;
                    }
                    else
                    {
                        sal_Int32 nTmp = 2;
                        LocaleType aTmpLocale( ImpGetLocaleType( sStr, nTmp));
                        if (aTmpLocale.meLanguage == LANGUAGE_DONTKNOW)
                        {
                            bCancel = true;         // break for
                            nCheckPos = nPosOld;
                        }
                        else
                        {
                            // Only the first sub format's locale will be
                            // used as the format's overall locale.
                            // Sorts this also under the corresponding
                            // locale for the dialog.
                            // If we don't support the locale this would
                            // result in an unknown (empty) language
                            // listbox entry and the user would never see
                            // this format.
                            if (nIndex == 0 && (aTmpLocale.meLanguage == LANGUAGE_SYSTEM ||
                                                SvNumberFormatter::IsLocaleInstalled( aTmpLocale.meLanguage)))
                            {
                                maLocale = aTmpLocale;
                                eLan = aTmpLocale.meLanguage;   // return to caller
                                /* TODO: fiddle with scanner to make this
                                 * known? A change in the locale may affect
                                 * separators and keywords. On the other
                                 * hand they may have been entered as used
                                 * in the originating locale, there's no
                                 * way to predict other than analyzing the
                                 * format code, we assume here the current
                                 * context is used, which is most likely
                                 * the case.
                                 * */

                                // Strip a plain locale identifier if locale
                                // data is available to avoid duplicated
                                // formats with and without LCID for the same
                                // locale. Besides it looks ugly and confusing
                                // and is unnecessary as the format will be
                                // listed for the resulting locale.
                                if (aTmpLocale.isPlainLocale())
                                    sStr.clear();
                                else
                                    sStr = "$-" + aTmpLocale.generateCode();
                            }
                            else
                            {
                                if (nIndex == 0)
                                    // Locale data not available, remember.
                                    maLocale.meLanguageWithoutLocaleData = aTmpLocale.meLanguage;

                                sStr = "$-" + aTmpLocale.generateCode();
                            }
                            NumFor[nIndex].SetNatNumLang( MsLangId::getRealLanguage( aTmpLocale.meLanguage));

                            // "$-NNCCLLLL" Numerals and Calendar
                            if (sSymbol.getLength() > 6)
                            {
                                sInsertCalendar = ImpObtainCalendarAndNumerals( sBuff, nPos, eLan, aTmpLocale);
                            }
                            /* NOTE: there can be only one calendar
                             * inserted so the last one wins, though
                             * our own calendar modifiers support
                             * multiple calendars within one sub format
                             * code if at different positions. */
                        }
                    }
                    break;
                }
                if ( !bCancel )
                {
                    if (sStr == sSymbol)
                    {
                        nPosOld = nPos;
                    }
                    else
                    {
                        sBuff.remove(nPosOld, nPos - nPosOld);
                        if (!sStr.isEmpty())
                        {
                            sBuff.insert(nPosOld, sStr);
                            nPos = nPosOld + sStr.getLength();
                            sBuff.insert(nPos, "]");
                            sBuff.insert(nPosOld, "[");
                            nPos += 2;
                            nPosOld = nPos;     // position before string
                        }
                        else
                        {
                            nPos = nPosOld;     // prefix removed for whatever reason
                        }
                    }
                }
            }
        }
        while ( !bCancel && lcl_SvNumberformat_IsBracketedPrefix( eSymbolType ) );

        // The remaining format code string
        if ( !bCancel )
        {
            if (eSymbolType == BRACKET_SYMBOLTYPE_FORMAT)
            {
                if (nIndex == 1 && eOp1 == NUMBERFORMAT_OP_NO)
                {
                    eOp1 = NUMBERFORMAT_OP_GT;  // undefined condition, default: > 0
                }
                else if (nIndex == 2 && eOp2 == NUMBERFORMAT_OP_NO)
                {
                    eOp2 = NUMBERFORMAT_OP_LT;  // undefined condition, default: < 0
                }
                if (sStr.isEmpty())
                {
                    // Empty sub format.
                    NumFor[nIndex].Info().eScannedType = SvNumFormatType::EMPTY;
                }
                else
                {
                    if (!sInsertCalendar.isEmpty())
                    {
                        sStr = sInsertCalendar + sStr;
                    }
                    sal_Int32 nStrPos = pSc->ScanFormat( sStr);
                    sal_uInt16 nCnt = pSc->GetResultStringsCnt();
                    if (nCnt == 0)              // error
                    {
                        nStrPos = 1;
                    }
                    if (nStrPos == 0)               // ok
                    {
                        // e.g. Thai T speciality
                        if (pSc->GetNatNumModifier() && !NumFor[nIndex].GetNatNum().IsSet())
                        {
                            sStr = "[NatNum"  + OUString::number( pSc->GetNatNumModifier()) + "]" + sStr;
                            NumFor[nIndex].SetNatNumNum( pSc->GetNatNumModifier(), false );
                        }
                        // #i53826# #i42727# For the Thai T speciality we need
                        // to freeze the locale and immunize it against
                        // conversions during exports, just in case we want to
                        // save to Xcl. This disables the feature of being able
                        // to convert a NatNum to another locale. You can't
                        // have both.
                        // FIXME: implement a specialized export conversion
                        // that works on tokens (have to tokenize all first)
                        // and doesn't use the format string and
                        // PutandConvertEntry() to LANGUAGE_ENGLISH_US in
                        // sc/source/filter/excel/xestyle.cxx
                        // XclExpNumFmtBuffer::WriteFormatRecord().
                        LanguageType eLanguage;
                        if (NumFor[nIndex].GetNatNum().GetNatNum() == 1 &&
                            ((eLanguage = MsLangId::getRealLanguage( eLan)) == LANGUAGE_THAI) &&
                            NumFor[nIndex].GetNatNum().GetLang() == LANGUAGE_DONTKNOW)
                        {
                            sStr = "[$-" + OUString::number( sal_uInt16(eLanguage), 16 ).toAsciiUpperCase() + "]" + sStr;
                            NumFor[nIndex].SetNatNumLang( eLanguage);
                        }
                        sBuff.remove(nPosOld, nPos - nPosOld);
                        sBuff.insert(nPosOld, sStr);
                        nPos = nPosOld + sStr.getLength();
                        if (nPos < sBuff.getLength())
                        {
                            sBuff.insert(nPos, ";");
                            nPos++;
                        }
                        else if (nIndex > 0)
                        {
                            // The last subformat. If it is a trailing text
                            // format the omitted subformats act like they were
                            // not specified and "inherited" the first format,
                            // e.g.  0;@  behaves like  0;-0;0;@
                            if (pSc->GetScannedType() == SvNumFormatType::TEXT)
                            {
                                // Reset conditions, reverting any set above.
                                if (nIndex == 1)
                                    eOp1 = NUMBERFORMAT_OP_NO;
                                else if (nIndex == 2)
                                    eOp2 = NUMBERFORMAT_OP_NO;
                                nIndex = 3;
                            }
                        }
                        NumFor[nIndex].Enlarge(nCnt);
                        pSc->CopyInfo(&(NumFor[nIndex].Info()), nCnt);
                        // type check
                        if (nIndex == 0)
                        {
                            eType = NumFor[nIndex].Info().eScannedType;
                        }
                        else if (nIndex == 3)
                        {   // #77026# Everything recognized IS text
                            NumFor[nIndex].Info().eScannedType = SvNumFormatType::TEXT;
                        }
                        else if ( NumFor[nIndex].Info().eScannedType != eType)
                        {
                            eType = SvNumFormatType::DEFINED;
                        }
                    }
                    else
                    {
                        nCheckPos = nPosOld + nStrPos;  // error in string
                        bCancel = true;                 // break for
                    }
                }
            }
            else if (eSymbolType == BRACKET_SYMBOLTYPE_ERROR)   // error
            {
                nCheckPos = nPosOld;
                bCancel = true;
            }
            else if ( lcl_SvNumberformat_IsBracketedPrefix( eSymbolType ) )
            {
                nCheckPos = nPosOld + 1;                // error, prefix in string
                bCancel = true;                         // break for
            }
        }
        if ( bCancel && !nCheckPos )
        {
            nCheckPos = 1;      // nCheckPos is used as an error condition
        }
        if ( !bCancel )
        {
            if ( NumFor[nIndex].GetNatNum().IsSet() &&
                 NumFor[nIndex].GetNatNum().GetLang() == LANGUAGE_DONTKNOW )
            {
                 NumFor[nIndex].SetNatNumLang( eLan );
            }
        }
        if (sBuff.getLength() == nPos)
        {
            if (nIndex < 3 && rString[rString.getLength()-1] == ';')
            {
                // A trailing ';' is significant and specifies the following
                // subformat to be empty. We don't enter the scanning loop
                // above again though.
                // Note that the operators apply to the current last scanned
                // subformat.
                if (nIndex == 0 && eOp1 == NUMBERFORMAT_OP_NO)
                {
                    eOp1 = NUMBERFORMAT_OP_GT;  // undefined condition, default: > 0
                }
                else if (nIndex == 1 && eOp2 == NUMBERFORMAT_OP_NO)
                {
                    eOp2 = NUMBERFORMAT_OP_LT;  // undefined condition, default: < 0
                }
                NumFor[nIndex+1].Info().eScannedType = SvNumFormatType::EMPTY;
                if (sBuff[nPos-1] != ';')
                    sBuff.insert( nPos++, ';');
            }
            if (nIndex == 2 && eSymbolType == BRACKET_SYMBOLTYPE_FORMAT && sBuff[nPos-1] == ';')
            {
                // #83510# A 4th subformat explicitly specified to be empty
                // hides any text. Need the type here for HasTextFormat()
                NumFor[3].Info().eScannedType = SvNumFormatType::TEXT;
            }
            bCancel = true;
        }
        if ( NumFor[nIndex].GetNatNum().IsSet() )
        {
            NumFor[nIndex].SetNatNumDate( bool(NumFor[nIndex].Info().eScannedType & SvNumFormatType::DATE) );
        }
    }

    if (!nCheckPos && IsSubstituted())
    {
        // For to be substituted formats the scanned type must match the
        // substitute type.
        if (IsSystemTimeFormat())
        {
            if ((eType & ~SvNumFormatType::DEFINED) != SvNumFormatType::TIME)
                nCheckPos = std::max<sal_Int32>( sBuff.indexOf(']') + 1, 1);
        }
        else if (IsSystemLongDateFormat())
        {
            if ((eType & ~SvNumFormatType::DEFINED) != SvNumFormatType::DATE)
                nCheckPos = std::max<sal_Int32>( sBuff.indexOf(']') + 1, 1);
        }
        else
            assert(!"unhandled substitute");
    }

    if ( bCondition && !nCheckPos )
    {
        if ( nIndex == 1 && NumFor[0].GetCount() == 0 &&
             sBuff[sBuff.getLength() - 1] != ';' )
        {
            // No format code => GENERAL but not if specified empty
            OUString aAdd( pSc->GetStandardName() );
            if ( !pSc->ScanFormat( aAdd ) )
            {
                sal_uInt16 nCnt = pSc->GetResultStringsCnt();
                if ( nCnt )
                {
                    NumFor[0].Enlarge(nCnt);
                    pSc->CopyInfo( &(NumFor[0].Info()), nCnt );
                    sBuff.append(aAdd);
                }
            }
        }
        else if ( nIndex == 1 && NumFor[nIndex].GetCount() == 0 &&
                  sBuff[sBuff.getLength() - 1] != ';' &&
                  (NumFor[0].GetCount() > 1 ||
                   (NumFor[0].GetCount() == 1 &&
                    NumFor[0].Info().nTypeArray[0] != NF_KEY_GENERAL)) )
        {
            // No trailing second subformat => GENERAL but not if specified empty
            // and not if first subformat is GENERAL
            OUString aAdd( pSc->GetStandardName() );
            if ( !pSc->ScanFormat( aAdd ) )
            {
                sal_uInt16 nCnt = pSc->GetResultStringsCnt();
                if ( nCnt )
                {
                    NumFor[nIndex].Enlarge(nCnt);
                    pSc->CopyInfo( &(NumFor[nIndex].Info()), nCnt );
                    sBuff.append(";");
                    sBuff.append(aAdd);
                }
            }
        }
        else if ( nIndex == 2 && NumFor[nIndex].GetCount() == 0 &&
                  sBuff[sBuff.getLength() - 1] != ';' &&
                  eOp2 != NUMBERFORMAT_OP_NO )
        {
            // No trailing third subformat => GENERAL but not if specified empty
            OUString aAdd( pSc->GetStandardName() );
            if ( !pSc->ScanFormat( aAdd ) )
            {
                sal_uInt16 nCnt = pSc->GetResultStringsCnt();
                if ( nCnt )
                {
                    NumFor[nIndex].Enlarge(nCnt);
                    pSc->CopyInfo( &(NumFor[nIndex].Info()), nCnt );
                    sBuff.append(";");
                    sBuff.append(aAdd);
                }
            }
        }
    }
    rString = sBuff.makeStringAndClear();
    sFormatstring = rString;

    if (NumFor[2].GetCount() == 0 && // No third partial string
        eOp1 == NUMBERFORMAT_OP_GT && eOp2 == NUMBERFORMAT_OP_NO &&
        fLimit1 == 0.0 && fLimit2 == 0.0)
    {
        eOp1 = NUMBERFORMAT_OP_GE; // Add 0 to the first format
    }

}

SvNumberformat::~SvNumberformat()
{
}

/**
 * Next_Symbol
 *
 * Splits up the symbols for further processing (by the Turing machine)
 *
 * Start state = SsStart, * = Special state
 * ---------------+-------------------+----------------------------+---------------
 *  Old State     | Symbol read       | Event                      | New state
 * ---------------+-------------------+----------------------------+---------------
 *  SsStart       | "                 | Symbol += Character        | SsGetQuoted
 *                | ;                 | Pos--                      | SsGetString
 *                | [                 | Symbol += Character        | SsGetBracketed
 *                | ]                 | Error                      | SsStop
 *                | BLANK             |                            |
 *                | Else              | Symbol += Character        | SsGetString
 * ---------------+-------------------+----------------------------+---------------
 *  SsGetString   | "                 | Symbol += Character        | SsGetQuoted
 *                | ;                 |                            | SsStop
 *                | Else              | Symbol += Character        |
 * ---------------+-------------------+----------------------------+---------------
 *  SsGetQuoted   | "                 | Symbol += Character        | SsGetString
 *                | Else              | Symbol += Character        |
 * ---------------+-------------------+----------------------------+---------------
 * SsGetBracketed | <, > =            | del [                      |
 *                |                   | Symbol += Character        | SsGetCon
 *                | BLANK             |                            |
 *                | h, H, m, M, s, S  | Symbol += Character        | SsGetTime
 *                | Else              | del [                      |
 *                |                   | Symbol += Character        | SsGetPrefix
 * ---------------+-------------------+----------------------------+---------------
 *  SsGetTime     | ]                 | Symbol += Character        | SsGetString
 *                | h, H, m, M, s, S  | Symbol += Character, *     | SsGetString
 *                | Else              | del [; Symbol += Character | SsGetPrefix
 * ---------------+-------------------+----------------------------+---------------
 *  SsGetPrefix   | ]                 |                            | SsStop
 *                | Else              | Symbol += Character        |
 * ---------------+-------------------+----------------------------+---------------
 *  SsGetCon      | >, =              | Symbol += Character        |
 *                | ]                 |                            | SsStop
 *                | Else              | Error                      | SsStop
 * ---------------+-------------------+----------------------------+---------------
 */

enum ScanState
{
    SsStop,
    SsStart,
    SsGetCon,           // condition
    SsGetString,        // format string
    SsGetPrefix,        // color or NatNumN
    SsGetTime,          // [HH] for time
    SsGetBracketed,     // any [...] not decided yet
    SsGetQuoted         // quoted text
};

// read a string until ']' and delete spaces in input
// static
sal_Int32 SvNumberformat::ImpGetNumber(OUStringBuffer& rString,
                                       sal_Int32& nPos,
                                       OUString& sSymbol)
{
    sal_Int32 nStartPos = nPos;
    sal_Unicode cToken;
    sal_Int32 nLen = rString.getLength();
    OUStringBuffer sBuffSymbol;
    while ( nPos < nLen && ((cToken = rString[nPos]) != ']') )
    {
        if (cToken == ' ')
        {                                               // delete spaces
            rString.remove(nPos,1);
            nLen--;
        }
        else
        {
            nPos++;
            sBuffSymbol.append(cToken);
        }
    }
    sSymbol = sBuffSymbol.makeStringAndClear();
    return nPos - nStartPos;
}

namespace {

sal_Unicode toUniChar(sal_uInt8 n)
{
    sal_Char c;
    if (n < 10)
    {
        c = '0' + n;
    }
    else
    {
        c = 'A' + n - 10;
    }
    return sal_Unicode(c);
}

bool IsCombiningSymbol( OUStringBuffer& rStringBuffer, sal_Int32 nPos )
{
    bool bRet = false;
    while (nPos >= 0)
    {
        switch (rStringBuffer[nPos])
        {
            case '*':
            case '\\':
            case '_':
                bRet = !bRet;
                --nPos;
                break;
            default:
                return bRet;
        }
    }
    return bRet;
}

} // namespace

OUString SvNumberformat::LocaleType::generateCode() const
{
    OUStringBuffer aBuf;
#if 0
    // TODO: We may re-enable this later. Don't remove it! --Kohei
    if (mnNumeralShape)
    {
        sal_uInt8 nVal = mnNumeralShape;
        for (sal_uInt8 i = 0; i < 2; ++i)
        {
            sal_uInt8 n = (nVal & 0xF0) >> 4;
            if (n || aBuf.getLength())
            {
                aBuf.append(toUniChar(n));
            }
            nVal = nVal << 4;
        }
    }

    if (mnNumeralShape || mnCalendarType)
    {
        sal_uInt8 nVal = mnCalendarType;
        for (sal_uInt8 i = 0; i < 2; ++i)
        {
            sal_uInt8 n = (nVal & 0xF0) >> 4;
            if (n || aBuf.getLength())
            {
                aBuf.append(toUniChar(n));
            }
            nVal = nVal << 4;
        }
    }
#endif

    sal_uInt16 n16 = static_cast<sal_uInt16>(
            (meLanguageWithoutLocaleData == LANGUAGE_DONTKNOW) ? meLanguage :
            meLanguageWithoutLocaleData);
    if (meLanguage == LANGUAGE_SYSTEM)
    {
        switch (meSubstitute)
        {
            case Substitute::NONE:
                ;   // nothing
                break;
            case Substitute::TIME:
                n16 = static_cast<sal_uInt16>(LANGUAGE_NF_SYSTEM_TIME);
                break;
            case Substitute::LONGDATE:
                n16 = static_cast<sal_uInt16>(LANGUAGE_NF_SYSTEM_DATE);
                break;
        }
    }
    for (sal_uInt8 i = 0; i < 4; ++i)
    {
        sal_uInt8 n = static_cast<sal_uInt8>((n16 & 0xF000) >> 12);
        // Omit leading zeros for consistency.
        if (n || !aBuf.isEmpty() || i == 3)
        {
            aBuf.append(toUniChar(n));
        }
        n16 = n16 << 4;
    }

    return aBuf.makeStringAndClear();
}

SvNumberformat::LocaleType::LocaleType()
    : meLanguage(LANGUAGE_DONTKNOW)
    , meLanguageWithoutLocaleData(LANGUAGE_DONTKNOW)
    , meSubstitute(Substitute::NONE)
    , mnNumeralShape(0)
    , mnCalendarType(0)
{
}

SvNumberformat::LocaleType::LocaleType(sal_uInt32 nRawNum)
    : meLanguage(LANGUAGE_DONTKNOW)
    , meLanguageWithoutLocaleData(LANGUAGE_DONTKNOW)
    , meSubstitute(Substitute::NONE)
    , mnNumeralShape(0)
    , mnCalendarType(0)
{
    meLanguage = static_cast<LanguageType>(nRawNum & 0x0000FFFF);
    if (meLanguage == LANGUAGE_NF_SYSTEM_TIME)
    {
        meSubstitute = Substitute::TIME;
        meLanguage = LANGUAGE_SYSTEM;
    }
    else if (meLanguage == LANGUAGE_NF_SYSTEM_DATE)
    {
        meSubstitute = Substitute::LONGDATE;
        meLanguage = LANGUAGE_SYSTEM;
    }
    nRawNum = (nRawNum >> 16);
    mnCalendarType = static_cast<sal_uInt8>(nRawNum & 0xFF);
    nRawNum = (nRawNum >> 8);
    mnNumeralShape = static_cast<sal_uInt8>(nRawNum & 0xFF);
}

bool SvNumberformat::LocaleType::isPlainLocale() const
{
    return meSubstitute == Substitute::NONE && !mnCalendarType && !mnNumeralShape;
}

// static
SvNumberformat::LocaleType SvNumberformat::ImpGetLocaleType(const OUString& rString, sal_Int32& nPos )
{
    sal_uInt32 nNum = 0;
    sal_Unicode cToken = 0;
    sal_Int32 nStart = nPos;
    sal_Int32 nLen = rString.getLength();
    while ( nPos < nLen && (nPos - nStart < 8) && ((cToken = rString[nPos]) != ']') )
    {
        if ( '0' <= cToken && cToken <= '9' )
        {
            nNum *= 16;
            nNum += cToken - '0';
        }
        else if ( 'a' <= cToken && cToken <= 'f' )
        {
            nNum *= 16;
            nNum += cToken - 'a' + 10;
        }
        else if ( 'A' <= cToken && cToken <= 'F' )
        {
            nNum *= 16;
            nNum += cToken - 'A' + 10;
        }
        else
        {
            return LocaleType(); // LANGUAGE_DONTKNOW;
        }
        ++nPos;
    }

    return (cToken == ']' || nPos == nLen) ? LocaleType(nNum) : LocaleType();
}

static bool lcl_matchKeywordAndGetNumber( const OUString & rString, const sal_Int32 nPos,
        const OUString & rKeyword, sal_Int32 & nNumber )
{
    if (0 <= nPos && nPos + rKeyword.getLength() < rString.getLength() && rString.matchIgnoreAsciiCase( rKeyword, nPos))
    {
        nNumber = rString.copy( nPos + rKeyword.getLength()).toInt32();
        return true;
    }
    else
    {
        nNumber = 0;
        return false;
    }
}

short SvNumberformat::ImpNextSymbol(OUStringBuffer& rString,
                                    sal_Int32& nPos,
                                    OUString& sSymbol) const
{
    short eSymbolType = BRACKET_SYMBOLTYPE_FORMAT;
    sal_Unicode cToken;
    sal_Unicode cLetter = ' '; // Preliminary result
    sal_Int32 nLen = rString.getLength();
    ScanState eState = SsStart;
    OUStringBuffer sBuffSymbol;

    const NfKeywordTable & rKeywords = rScan.GetKeywords();
    while (nPos < nLen && eState != SsStop)
    {
        cToken = rString[nPos];
        nPos++;
        switch (eState)
        {
        case SsStart:
            if (cToken == '\"')
            {
                eState = SsGetQuoted;
                sBuffSymbol.append(cToken);
            }
            else if (cToken == '[')
            {
                eState = SsGetBracketed;
                sBuffSymbol.append(cToken);
            }
            else if (cToken == ';')
            {
                eState = SsGetString;
                nPos--;
                eSymbolType = BRACKET_SYMBOLTYPE_FORMAT;
            }
            else if (cToken == ']')
            {
                eState = SsStop;
                eSymbolType = BRACKET_SYMBOLTYPE_ERROR;
            }
            else if (cToken == ' ') // Skip Blanks
            {
                nPos--;
                rString.remove(nPos, 1);
                nLen--;
            }
            else
            {
                sBuffSymbol.append(cToken);
                eState = SsGetString;
                eSymbolType = BRACKET_SYMBOLTYPE_FORMAT;
            }
            break;
        case SsGetBracketed:
            switch (cToken)
            {
            case '<':
            case '>':
            case '=':
                sBuffSymbol.stripStart('[');
                sBuffSymbol.append(cToken);
                cLetter = cToken;
                eState = SsGetCon;
                switch (cToken)
                {
                case '<':
                    eSymbolType = NUMBERFORMAT_OP_LT;
                    break;
                case '>':
                    eSymbolType = NUMBERFORMAT_OP_GT;
                    break;
                case '=':
                    eSymbolType = NUMBERFORMAT_OP_EQ;
                    break;
                }
                break;
            case ' ':
                nPos--;
                rString.remove(nPos, 1);
                nLen--;
                break;
            case '$' :
                if ( nPos < nLen && rString[nPos] == '-' )
                {
                    // [$-xxx] locale
                    sBuffSymbol.stripStart('[');
                    eSymbolType = BRACKET_SYMBOLTYPE_LOCALE;
                    eState = SsGetPrefix;
                }
                else
                {   // currency
                    eSymbolType = BRACKET_SYMBOLTYPE_FORMAT;
                    eState = SsGetString;
                }
                sBuffSymbol.append(cToken);
                break;
            case '~' :
                // calendarID
                eSymbolType = BRACKET_SYMBOLTYPE_FORMAT;
                sBuffSymbol.append(cToken);
                eState = SsGetString;
                break;
            default:
            {
                const OUString aNatNum("NATNUM");
                const OUString aDBNum("DBNUM");
                const OUString aBufStr( rString.toString());
                sal_Int32 nNatNumNum;
                sal_Int32 nDBNum;
                if ( lcl_matchKeywordAndGetNumber( aBufStr, nPos-1, aNatNum, nNatNumNum) &&
                        0 <= nNatNumNum && nNatNumNum <= 19 )
                {
                    sBuffSymbol.stripStart('[');
                    sBuffSymbol.append( std::u16string_view(aBufStr).substr(--nPos, aNatNum.getLength()+1) );
                    nPos += aNatNum.getLength()+1;
                    //! SymbolType is negative
                    eSymbolType = static_cast<short>(BRACKET_SYMBOLTYPE_NATNUM0 - nNatNumNum);
                    eState = SsGetPrefix;
                }
                else if ( lcl_matchKeywordAndGetNumber( aBufStr, nPos-1, aDBNum, nDBNum) &&
                        1 <= nDBNum && nDBNum <= 9 )
                {
                    sBuffSymbol.stripStart('[');
                    sBuffSymbol.append( std::u16string_view(aBufStr).substr(--nPos, aDBNum.getLength()+1) );
                    nPos += aDBNum.getLength()+1;
                    //! SymbolType is negative
                    eSymbolType = sal::static_int_cast< short >( BRACKET_SYMBOLTYPE_DBNUM1 - (nDBNum - 1) );
                    eState = SsGetPrefix;
                }
                else
                {
                    sal_Unicode cUpper = rChrCls().uppercase( aBufStr, nPos-1, 1)[0];
                    if (    cUpper == rKeywords[NF_KEY_H][0] ||     // H
                            cUpper == rKeywords[NF_KEY_MI][0] ||    // M
                            cUpper == rKeywords[NF_KEY_S][0] )      // S
                    {
                        sBuffSymbol.append(cToken);
                        eState = SsGetTime;
                        cLetter = cToken;
                    }
                    else
                    {
                        sBuffSymbol.stripStart('[');
                        sBuffSymbol.append(cToken);
                        eSymbolType = BRACKET_SYMBOLTYPE_COLOR;
                        eState = SsGetPrefix;
                    }
                }
            }
            }
            break;
        case SsGetString:
            if (cToken == '\"')
            {
                eState = SsGetQuoted;
                sBuffSymbol.append(cToken);
            }
            else if (cToken == ';' && (nPos < 2 || !IsCombiningSymbol( rString, nPos-2)))
            {
                eState = SsStop;
            }
            else
            {
                sBuffSymbol.append(cToken);
            }
            break;
        case SsGetQuoted:
            if (cToken == '\"')
            {
                eState = SsGetString;
                sBuffSymbol.append(cToken);
            }
            else
            {
                sBuffSymbol.append(cToken);
            }
            break;
        case SsGetTime:
            if (cToken == ']')
            {
                sBuffSymbol.append(cToken);
                eState = SsGetString;
                eSymbolType = BRACKET_SYMBOLTYPE_FORMAT;
            }
            else
            {
                sal_Unicode cUpper = rChrCls().uppercase(rString.toString(), nPos-1, 1)[0];
                if (cUpper == rKeywords[NF_KEY_H][0] ||   // H
                    cUpper == rKeywords[NF_KEY_MI][0] ||  // M
                    cUpper == rKeywords[NF_KEY_S][0] )    // S
                {
                    if (cLetter == cToken)
                    {
                        sBuffSymbol.append(cToken);
                        cLetter = ' ';
                    }
                    else
                    {
                        sBuffSymbol.stripStart('[');
                        sBuffSymbol.append(cToken);
                        eState = SsGetPrefix;
                    }
                }
                else
                {
                    sBuffSymbol.stripStart('[');
                    sBuffSymbol.append(cToken);
                    eSymbolType = BRACKET_SYMBOLTYPE_COLOR;
                    eState = SsGetPrefix;
                }
            }
            break;
        case SsGetCon:
            switch (cToken)
            {
            case '<':
                eState = SsStop;
                eSymbolType = BRACKET_SYMBOLTYPE_ERROR;
                break;
            case '>':
                if (cLetter == '<')
                {
                    sBuffSymbol.append(cToken);
                    cLetter = ' ';
                    eState = SsStop;
                    eSymbolType = NUMBERFORMAT_OP_NE;
                }
                else
                {
                    eState = SsStop;
                    eSymbolType = BRACKET_SYMBOLTYPE_ERROR;
                }
                break;
            case '=':
                if (cLetter == '<')
                {
                    sBuffSymbol.append(cToken);
                    cLetter = ' ';
                    eSymbolType = NUMBERFORMAT_OP_LE;
                }
                else if (cLetter == '>')
                {
                    sBuffSymbol.append(cToken);
                    cLetter = ' ';
                    eSymbolType = NUMBERFORMAT_OP_GE;
                }
                else
                {
                    eState = SsStop;
                    eSymbolType = BRACKET_SYMBOLTYPE_ERROR;
                }
                break;
            case ' ':
                nPos--;
                rString.remove(nPos,1);
                nLen--;
                break;
            default:
                eState = SsStop;
                nPos--;
                break;
            }
            break;
        case SsGetPrefix:
            if (cToken == ']')
            {
                eState = SsStop;
            }
            else
            {
                sBuffSymbol.append(cToken);
            }
            break;
        default:
            break;
        } // of switch
    } // of while
    sSymbol = sBuffSymbol.makeStringAndClear();
    return eSymbolType;
}

void SvNumberformat::ConvertLanguage( SvNumberFormatter& rConverter,
                                      LanguageType eConvertFrom,
                                      LanguageType eConvertTo )
{
    sal_Int32 nCheckPos;
    sal_uInt32 nKey;
    SvNumFormatType nType = eType;
    OUString aFormatString( sFormatstring );
    rConverter.PutandConvertEntry( aFormatString, nCheckPos, nType,
                                   nKey, eConvertFrom, eConvertTo, false);
    const SvNumberformat* pFormat = rConverter.GetEntry( nKey );
    DBG_ASSERT( pFormat, "SvNumberformat::ConvertLanguage: Conversion without format" );
    if ( pFormat )
    {
        ImpCopyNumberformat( *pFormat );
        // Reset values taken over from Formatter/Scanner
        // pColor still points to table in temporary Formatter/Scanner
        for (ImpSvNumFor & rFormatter : NumFor)
        {
            OUString aColorName = rFormatter.GetColorName();
            Color* pColor = rScan.GetColor( aColorName );
            rFormatter.SetColor( pColor, aColorName );
        }
    }
}

bool SvNumberformat::HasNewCurrency() const
{
    for (const auto & j : NumFor)
    {
        if ( j.HasNewCurrency() )
        {
            return true;
        }
    }
    return false;
}

bool SvNumberformat::GetNewCurrencySymbol( OUString& rSymbol,
                                           OUString& rExtension ) const
{
    for (const auto & j : NumFor)
    {
        if ( j.GetNewCurrencySymbol( rSymbol, rExtension ) )
        {
            return true;
        }
    }
    rSymbol.clear();
    rExtension.clear();
    return false;
}

// static
OUString SvNumberformat::StripNewCurrencyDelimiters( const OUString& rStr )
{
    OUStringBuffer aTmp;
    sal_Int32 nStartPos, nPos, nLen;
    nLen = rStr.getLength();
    nStartPos = 0;
    while ( (nPos = rStr.indexOf( "[$", nStartPos )) >= 0 )
    {
        sal_Int32 nEnd;
        if ( (nEnd = GetQuoteEnd( rStr, nPos )) >= 0 )
        {
            aTmp.append(rStr.copy( nStartPos, ++nEnd - nStartPos ));
            nStartPos = nEnd;
        }
        else
        {
            aTmp.append(std::u16string_view(rStr).substr(nStartPos, nPos - nStartPos) );
            nStartPos = nPos + 2;
            sal_Int32 nDash;
            nEnd = nStartPos - 1;
            do
            {
                nDash = rStr.indexOf( '-', ++nEnd );
            }
            while ( (nEnd = GetQuoteEnd( rStr, nDash )) >= 0 );
            sal_Int32 nClose;
            nEnd = nStartPos - 1;
            do
            {
                nClose = rStr.indexOf( ']', ++nEnd );
            }
            while ( (nEnd = GetQuoteEnd( rStr, nClose )) >= 0 );

            if(nClose < 0)
            {
                /* there should always be a closing ]
                 * but the old String class would have hidden
                 * that. so be conservative too
                 */
                nClose = nLen;
            }

            nPos = nClose;
            if(nDash >= 0 && nDash < nClose)
            {
                nPos = nDash;
            }
            aTmp.append(std::u16string_view(rStr).substr(nStartPos, nPos - nStartPos) );
            nStartPos = nClose + 1;
        }
    }
    if ( nLen > nStartPos )
    {
        aTmp.append(std::u16string_view(rStr).substr(nStartPos, nLen - nStartPos) );
    }
    return aTmp.makeStringAndClear();
}

void SvNumberformat::ImpGetOutputStandard(double& fNumber, OUStringBuffer& rOutString) const
{
    OUString sTemp;
    ImpGetOutputStandard(fNumber, sTemp);
    rOutString = sTemp;
}

void SvNumberformat::ImpGetOutputStandard(double& fNumber, OUString& rOutString) const
{
    sal_uInt16 nStandardPrec = rScan.GetStandardPrec();

    if ( fabs(fNumber) > EXP_ABS_UPPER_BOUND )
    {
        nStandardPrec = ::std::min(nStandardPrec, static_cast<sal_uInt16>(14)); // limits to 14 decimals
        rOutString = ::rtl::math::doubleToUString( fNumber,
                                                  rtl_math_StringFormat_E2, nStandardPrec /*2*/,
                                                  GetFormatter().GetNumDecimalSep()[0]);
    }
    else
    {
        ImpGetOutputStdToPrecision(fNumber, rOutString, nStandardPrec);
    }
}

namespace
{

template<typename T>
bool checkForAll0s(const T& rString, sal_Int32 nIdx=0)
{
    if (nIdx>=rString.getLength())
        return false;

    do
    {
        if (rString[nIdx]!='0')
            return false;
    }
    while (++nIdx<rString.getLength());

    return true;
}

}

void SvNumberformat::ImpGetOutputStdToPrecision(double& rNumber, OUString& rOutString, sal_uInt16 nPrecision) const
{
    // Make sure the precision doesn't go over the maximum allowable precision.
    nPrecision = ::std::min(UPPER_PRECISION, nPrecision);

    // We decided to strip trailing zeros unconditionally, since binary
    // double-precision rounding error makes it impossible to determine e.g.
    // whether 844.10000000000002273737 is what the user has typed, or the
    // user has typed 844.1 but IEEE 754 represents it that way internally.

    rOutString = ::rtl::math::doubleToUString( rNumber,
                                               rtl_math_StringFormat_F, nPrecision /*2*/,
                                               GetFormatter().GetNumDecimalSep()[0], true );
    if (rOutString[0] == '-' && checkForAll0s(rOutString, 1))
    {
        rOutString = comphelper::string::stripStart(rOutString, '-'); // not -0
    }
    rOutString = impTransliterate(rOutString, NumFor[0].GetNatNum());
}

void SvNumberformat::ImpGetOutputInputLine(double fNumber, OUString& OutString) const
{
    bool bModified = false;
    if ( (eType & SvNumFormatType::PERCENT) && (fabs(fNumber) < D_MAX_D_BY_100))
    {
        if (fNumber == 0.0)
        {
            OutString = "0%";
            return;
        }
        fNumber *= 100;
        bModified = true;
    }

    if (fNumber == 0.0)
    {
        OutString = "0";
        return;
    }

    OutString = ::rtl::math::doubleToUString( fNumber,
                                              rtl_math_StringFormat_Automatic,
                                              rtl_math_DecimalPlaces_Max,
                                              GetFormatter().GetNumDecimalSep()[0], true );

    if ( eType & SvNumFormatType::PERCENT && bModified)
    {
        OutString += "%";
    }
}

short SvNumberformat::ImpCheckCondition(double fNumber,
                                        double fLimit,
                                        SvNumberformatLimitOps eOp)
{
    switch(eOp)
    {
    case NUMBERFORMAT_OP_NO:
        return -1;
    case NUMBERFORMAT_OP_EQ:
        return static_cast<short>(fNumber == fLimit);
    case NUMBERFORMAT_OP_NE:
        return static_cast<short>(fNumber != fLimit);
    case NUMBERFORMAT_OP_LT:
        return static_cast<short>(fNumber <  fLimit);
    case NUMBERFORMAT_OP_LE:
        return static_cast<short>(fNumber <= fLimit);
    case NUMBERFORMAT_OP_GT:
        return static_cast<short>(fNumber >  fLimit);
    case NUMBERFORMAT_OP_GE:
        return static_cast<short>(fNumber >= fLimit);
    default:
        return -1;
    }
}

static bool lcl_appendStarFillChar( OUStringBuffer& rBuf, const OUString& rStr )
{
    // Right during user input the star symbol is the very
    // last character before the user enters another one.
    if (rStr.getLength() > 1)
    {
        rBuf.append(u'\x001B');
        rBuf.append(rStr[1]);
        return true;
    }
    return false;
}

static bool lcl_insertStarFillChar( OUStringBuffer& rBuf, sal_Int32 nPos, const OUString& rStr )
{
    if (rStr.getLength() > 1)
    {
        rBuf.insert( nPos, rStr[1]);
        rBuf.insert( nPos, u'\x001B');
        return true;
    }
    return false;
}

void SvNumberformat::GetOutputString(const OUString& sString,
                                     OUString& OutString,
                                     Color** ppColor)
{
    OUStringBuffer sOutBuff;
    sal_uInt16 nIx;
    if (eType & SvNumFormatType::TEXT)
    {
        nIx = 0;
    }
    else if (NumFor[3].GetCount() > 0)
    {
        nIx = 3;
    }
    else
    {
        *ppColor = nullptr; // no change of color
        return;
    }
    *ppColor = NumFor[nIx].GetColor();
    const ImpSvNumberformatInfo& rInfo = NumFor[nIx].Info();
    if (rInfo.eScannedType == SvNumFormatType::TEXT)
    {
        const sal_uInt16 nCnt = NumFor[nIx].GetCount();
        for (sal_uInt16 i = 0; i < nCnt; i++)
        {
            switch (rInfo.nTypeArray[i])
            {
            case NF_SYMBOLTYPE_STAR:
                if( bStarFlag )
                {
                    lcl_appendStarFillChar( sOutBuff, rInfo.sStrArray[i]);
                }
                break;
            case NF_SYMBOLTYPE_BLANK:
                if (rInfo.sStrArray[i].getLength() >= 2)
                    InsertBlanks( sOutBuff, sOutBuff.getLength(), rInfo.sStrArray[i][1] );
                break;
            case NF_KEY_GENERAL :   // #77026# "General" is the same as "@"
            case NF_SYMBOLTYPE_DEL :
                sOutBuff.append(sString);
                break;
            default:
                sOutBuff.append(rInfo.sStrArray[i]);
            }
        }
    }
    OutString = sOutBuff.makeStringAndClear();
}

namespace {

void lcl_GetOutputStringScientific(double fNumber, sal_uInt16 nCharCount,
                                   const SvNumberFormatter& rFormatter, OUString& rOutString)
{
    bool bSign = ::rtl::math::isSignBitSet(fNumber);

    // 1.000E+015 (one digit and the decimal point, and the two chars +
    // nExpDigit for the exponential part, totalling 6 or 7).
    double fExp = log10( fabs(fNumber) );
    if( fExp < 0.0 )
      fExp = 1.0 - fExp;
    sal_uInt16 nCharFormat = 6 + (fExp >= 100.0 ? 1 : 0);
    sal_uInt16 nPrec = nCharCount > nCharFormat ? nCharCount - nCharFormat : 0;
    if (nPrec && bSign)
    {
        // Make room for the negative sign.
        --nPrec;
    }
    nPrec = ::std::min(nPrec, static_cast<sal_uInt16>(14)); // limit to 14 decimals.

    rOutString = ::rtl::math::doubleToUString(fNumber, rtl_math_StringFormat_E2,
                                              nPrec, rFormatter.GetNumDecimalSep()[0], true );
}

OUString lcl_GetDenominatorString(const ImpSvNumberformatInfo &rInfo, sal_uInt16 nCnt)
{
    sal_Int32 i;
    OUStringBuffer aDenominatorString;
    for( i = 0; i < nCnt; i++ )
    {
        if( rInfo.nTypeArray[i] == NF_SYMBOLTYPE_FRAC )
        {
            while ( ( ++i < nCnt ) && rInfo.nTypeArray[i] != NF_SYMBOLTYPE_FRAC_FDIV
                                   && rInfo.nTypeArray[i] != NF_SYMBOLTYPE_DIGIT );
            for( ; i < nCnt; i++ )
            {
                if( rInfo.nTypeArray[i] == NF_SYMBOLTYPE_FRAC_FDIV || rInfo.nTypeArray[i] == NF_SYMBOLTYPE_DIGIT )
                    aDenominatorString.append( rInfo.sStrArray[i] );
                else
                    i = nCnt;
            }
        }
    }
    return aDenominatorString.makeStringAndClear();
}

OUString lcl_GetNumeratorString(const ImpSvNumberformatInfo &rInfo, sal_uInt16 nCnt)
{
    sal_Int32 i;
    OUStringBuffer aNumeratorString;
    for( i = 0; i < nCnt; i++ )
    {
        if( rInfo.nTypeArray[i] == NF_SYMBOLTYPE_FRAC )
        {
            for( i--; i >= 0 && rInfo.nTypeArray[i] == NF_SYMBOLTYPE_DIGIT ; i-- )
            {
                aNumeratorString.insert( 0, rInfo.sStrArray[i] );
            }
            i = nCnt;
        }
    }
    return aNumeratorString.makeStringAndClear();
}

OUString lcl_GetFractionIntegerString(const ImpSvNumberformatInfo &rInfo, sal_uInt16 nCnt)
{
    sal_Int32 i;
    OUStringBuffer aIntegerString;
    for( i = 0; i < nCnt; i++ )
    {
        if( rInfo.nTypeArray[i] == NF_SYMBOLTYPE_FRACBLANK )
        {
            for( i--; i >= 0 && ( rInfo.nTypeArray[i] == NF_SYMBOLTYPE_DIGIT
                               || rInfo.nTypeArray[i] == NF_SYMBOLTYPE_THSEP ); i-- )
            {
                aIntegerString.insert( 0, rInfo.sStrArray[i] );
            }
            i = nCnt;
        }
    }
    return aIntegerString.makeStringAndClear();
}

OUString lcl_GetIntegerFractionDelimiterString(const ImpSvNumberformatInfo &rInfo, sal_uInt16 nCnt)
{
    sal_uInt16 i;
    for( i = 0; i < nCnt; i++ )
    {
        if( rInfo.nTypeArray[i] == NF_SYMBOLTYPE_FRACBLANK )
        {
            return rInfo.sStrArray[i];
        }
    }
    return OUString();
}

}

OUString SvNumberformat::GetDenominatorString( sal_uInt16 nNumFor ) const
{
    const ImpSvNumberformatInfo& rInfo = NumFor[nNumFor].Info();
    sal_uInt16 nCnt = NumFor[nNumFor].GetCount();
    return lcl_GetDenominatorString( rInfo, nCnt );
}

OUString SvNumberformat::GetNumeratorString( sal_uInt16 nNumFor ) const
{
    const ImpSvNumberformatInfo& rInfo = NumFor[nNumFor].Info();
    sal_uInt16 nCnt = NumFor[nNumFor].GetCount();
    return lcl_GetNumeratorString( rInfo, nCnt );
}

OUString SvNumberformat::GetIntegerFractionDelimiterString( sal_uInt16 nNumFor ) const
{
    const ImpSvNumberformatInfo& rInfo = NumFor[nNumFor].Info();
    sal_uInt16 nCnt = NumFor[nNumFor].GetCount();
    return lcl_GetIntegerFractionDelimiterString( rInfo, nCnt );
}

bool SvNumberformat::GetOutputString(double fNumber, sal_uInt16 nCharCount, OUString& rOutString) const
{
    using namespace std;

    if (eType != SvNumFormatType::NUMBER)
    {
        return false;
    }
    double fTestNum = fNumber;
    bool bSign = ::rtl::math::isSignBitSet(fTestNum);
    if (bSign)
    {
        fTestNum = -fTestNum;
    }
    if (fTestNum < EXP_LOWER_BOUND)
    {
        lcl_GetOutputStringScientific(fNumber, nCharCount, GetFormatter(), rOutString);
        return true;
    }

    double fExp = log10(fTestNum);
    // Values < 1.0 always have one digit before the decimal point.
    sal_uInt16 nDigitPre = fExp >= 0.0 ? static_cast<sal_uInt16>(ceil(fExp)) : 1;

    if (nDigitPre > 15)
    {
        lcl_GetOutputStringScientific(fNumber, nCharCount, GetFormatter(), rOutString);
        return true;
    }

    sal_uInt16 nPrec = nCharCount >= nDigitPre ? nCharCount - nDigitPre : 0;
    if (nPrec && bSign)
    {
        // Subtract the negative sign.
        --nPrec;
    }
    if (nPrec)
    {
        // Subtract the decimal point.
        --nPrec;
    }
    ImpGetOutputStdToPrecision(fNumber, rOutString, nPrec);
    if (rOutString.getLength() > nCharCount)
    {
        // String still wider than desired.  Switch to scientific notation.
        lcl_GetOutputStringScientific(fNumber, nCharCount, GetFormatter(), rOutString);
    }
    return true;
}

sal_uInt16 SvNumberformat::GetSubformatIndex (double fNumber ) const
{
    sal_uInt16 nIx; // Index of the partial format
    double fLimit_1 = fLimit1;
    short nCheck = ImpCheckCondition(fNumber, fLimit_1, eOp1);
    if (nCheck == -1 || nCheck == 1) // Only 1 String or True
    {
        nIx = 0;
    }
    else
    {
        double fLimit_2 = fLimit2;
        nCheck = ImpCheckCondition(fNumber, fLimit_2, eOp2);
        if (nCheck == -1 || nCheck == 1)
        {
            nIx = 1;
        }
        else
        {
            nIx = 2;
        }
    }
    return nIx;
}

bool SvNumberformat::GetOutputString(double fNumber,
                                     OUString& OutString,
                                     Color** ppColor)
{
    bool bRes = false;
    OUStringBuffer sBuff;
    OutString.clear();
    *ppColor = nullptr; // No color change
    if (eType & SvNumFormatType::LOGICAL)
    {
        if (fNumber)
        {
            OutString = rScan.GetTrueString();
        }
        else
        {
            OutString = rScan.GetFalseString();
        }
        return false;
    }
    if (eType & SvNumFormatType::TEXT)
    {
        ImpGetOutputStandard(fNumber, sBuff);
        OutString = sBuff.makeStringAndClear();
        return false;
    }
    bool bHadStandard = false;
    if (bStandard) // Individual standard formats
    {
        if (rScan.GetStandardPrec() == SvNumberFormatter::INPUTSTRING_PRECISION) // All number format InputLine
        {
            ImpGetOutputInputLine(fNumber, OutString);
            return false;
        }
        switch (eType)
        {
        case SvNumFormatType::NUMBER: // Standard number format
            if (rScan.GetStandardPrec() == SvNumberFormatter::UNLIMITED_PRECISION)
            {
                if (::rtl::math::isSignBitSet(fNumber))
                {
                    if (!(fNumber < 0.0))
                        fNumber = -fNumber;     // do not display -0.0
                }
                if (fNumber == 0.0)
                {
                    OutString = "0";
                }
                else if (fNumber < EXP_LOWER_BOUND && fNumber > -EXP_LOWER_BOUND)
                {
                    OutString = ::rtl::math::doubleToUString( fNumber,
                                rtl_math_StringFormat_E2,
                                15,
                                GetFormatter().GetNumDecimalSep()[0], true);
                }
                else if (fNumber < 1.0 && fNumber > -1.0)
                {
                    OutString = ::rtl::math::doubleToUString( fNumber,
                                rtl_math_StringFormat_Automatic,
                                15,
                                GetFormatter().GetNumDecimalSep()[0], true);
                }
                else
                {
                    OutString = ::rtl::math::doubleToUString( fNumber,
                                rtl_math_StringFormat_Automatic,
                                rtl_math_DecimalPlaces_Max,
                                GetFormatter().GetNumDecimalSep()[0], true);
                }
                return false;
            }
            ImpGetOutputStandard(fNumber, sBuff);
            bHadStandard = true;
            break;
        case SvNumFormatType::DATE:
            bRes |= ImpGetDateOutput(fNumber, 0, sBuff);
            bHadStandard = true;
            break;
        case SvNumFormatType::TIME:
            bRes |= ImpGetTimeOutput(fNumber, 0, sBuff);
            bHadStandard = true;
            break;
        case SvNumFormatType::DATETIME:
            bRes |= ImpGetDateTimeOutput(fNumber, 0, sBuff);
            bHadStandard = true;
            break;
        default: break;
        }
    }
    if ( !bHadStandard )
    {
        sal_uInt16 nIx = GetSubformatIndex ( fNumber ); // Index of the partial format
        if (fNumber < 0.0 &&
                ((nIx == 0 && IsFirstSubformatRealNegative()) || // 1st, usually positive subformat
                 (nIx == 1 && IsSecondSubformatRealNegative()))) // 2nd, usually negative subformat
        {
            fNumber = -fNumber; // eliminate sign
        }
        *ppColor = NumFor[nIx].GetColor();
        const ImpSvNumberformatInfo& rInfo = NumFor[nIx].Info();
        const sal_uInt16 nCnt = NumFor[nIx].GetCount();
        if (nCnt == 0 && rInfo.eScannedType == SvNumFormatType::EMPTY)
        {
            return false; // Empty => nothing
        }
        else if (nCnt == 0) // Else Standard Format
        {
            ImpGetOutputStandard(fNumber, sBuff);
            OutString = sBuff.makeStringAndClear();
            return false;
        }
        switch (rInfo.eScannedType)
        {
        case SvNumFormatType::TEXT:
        case SvNumFormatType::DEFINED:
            for (sal_uInt16 i = 0; i < nCnt; i++)
            {
                switch (rInfo.nTypeArray[i])
                {
                case NF_SYMBOLTYPE_STAR:
                    if( bStarFlag )
                    {
                        bRes = lcl_appendStarFillChar( sBuff, rInfo.sStrArray[i]);
                    }
                    break;
                case NF_SYMBOLTYPE_BLANK:
                    if (rInfo.sStrArray[i].getLength() >= 2)
                        InsertBlanks(sBuff, sBuff.getLength(), rInfo.sStrArray[i][1] );
                    break;
                case NF_SYMBOLTYPE_STRING:
                case NF_SYMBOLTYPE_CURRENCY:
                    sBuff.append(rInfo.sStrArray[i]);
                    break;
                case NF_SYMBOLTYPE_THSEP:
                    if (rInfo.nThousand == 0)
                    {
                        sBuff.append(rInfo.sStrArray[i]);
                    }
                    break;
                default:
                    break;
                }
            }
            break;
        case SvNumFormatType::DATE:
            bRes |= ImpGetDateOutput(fNumber, nIx, sBuff);
            break;
        case SvNumFormatType::TIME:
            bRes |= ImpGetTimeOutput(fNumber, nIx, sBuff);
                break;
        case SvNumFormatType::DATETIME:
            bRes |= ImpGetDateTimeOutput(fNumber, nIx, sBuff);
            break;
        case SvNumFormatType::NUMBER:
        case SvNumFormatType::PERCENT:
        case SvNumFormatType::CURRENCY:
            bRes |= ImpGetNumberOutput(fNumber, nIx, sBuff);
            break;
        case SvNumFormatType::FRACTION:
            bRes |= ImpGetFractionOutput(fNumber, nIx, sBuff);
            break;
        case SvNumFormatType::SCIENTIFIC:
            bRes |= ImpGetScientificOutput(fNumber, nIx, sBuff);
            break;
        default: break;
        }
    }
    OutString = sBuff.makeStringAndClear();
    return bRes;
}

bool SvNumberformat::ImpGetScientificOutput(double fNumber,
                                            sal_uInt16 nIx,
                                            OUStringBuffer& sStr)
{
    bool bRes = false;
    bool bSign = false;

    const ImpSvNumberformatInfo& rInfo = NumFor[nIx].Info();
    const sal_uInt16 nCnt = NumFor[nIx].GetCount();

    if (fNumber < 0)
    {
        if (nIx == 0) // Not in the ones at the end
        {
            bSign = true; // Formats
        }
        fNumber = -fNumber;
    }

    sStr = ::rtl::math::doubleToUString( fNumber,
                                         rtl_math_StringFormat_E,
                                         rInfo.nCntPre + rInfo.nCntPost - 1, '.' );
    OUStringBuffer ExpStr;
    short nExpSign = 1;
    sal_Int32 nExPos = sStr.indexOf('E');
    sal_Int32 nDecPos = -1;

    if ( nExPos >= 0 )
    {
        // split into mantisse and exponent and get rid of "E+" or "E-"
        sal_Int32 nExpStart = nExPos + 1;

        switch ( sStr[ nExpStart ] )
        {
        case '-' :
            nExpSign = -1;
            [[fallthrough]];
        case '+' :
            ++nExpStart;
            break;
        }
        ExpStr = sStr.toString().copy( nExpStart );    // part following the "E+"
        sStr.truncate( nExPos );

        if ( rInfo.nCntPre != 1 ) // rescale Exp
        {
            sal_Int32 nExp = ExpStr.toString().toInt32() * nExpSign;
            sal_Int32 nRescale = (rInfo.nCntPre != 0) ? nExp % static_cast<sal_Int32>(rInfo.nCntPre) : -1;
            if( nRescale < 0 && rInfo.nCntPre != 0 )
                nRescale += static_cast<sal_Int32>(rInfo.nCntPre);
            nExp -= nRescale;
            if ( nExp < 0 )
            {
                nExpSign = -1;
                nExp = -nExp;
            }
            else
            {
                nExpSign = 1;
            }
            ExpStr = OUString::number( nExp );
            const sal_Unicode cFirstDigit = sStr[0];
            // rescale mantissa
            sStr = ::rtl::math::doubleToUString( fNumber,
                                         rtl_math_StringFormat_E,
                                         nRescale + rInfo.nCntPost, '.' );

            // sStr now may contain a rounded-up value shifted into the next
            // magnitude, for example 1.000E+02 (4 digits) for fNumber 99.995
            // (9.9995E+02 rounded to 3 decimals) but we want the final result
            // to be 100.00E+00 (5 digits), so for the following fill routines
            // below to work correctly append a zero decimal.
            /* TODO: this is awkward, could an engineering notation mode be
             * introduced to rtl_math_doubleToUString()? */
            sStr.truncate( sStr.indexOf('E') );
            if (sStr[0] == '1' && cFirstDigit != '1')
                sStr.append('0');
        }

        // cut any decimal delimiter
        sal_Int32 index = 0;

        while((index = sStr.indexOf('.', index)) >= 0)
        {
            if (nDecPos < 0)
                nDecPos = index;
            sStr.remove(index, 1);
        }
    }

    sal_uInt16 j = nCnt-1;  // Last symbol
    sal_Int32 k;  // Position in ExpStr
    sal_Int32 nZeros = 0; // Erase leading zeros

    bRes |= ImpNumberFill(ExpStr, fNumber, k, j, nIx, NF_SYMBOLTYPE_EXP);

    while (nZeros < k && ExpStr[nZeros] == '0')
    {
        ++nZeros;
    }
    if (nZeros)
    {
        ExpStr.remove( 0, nZeros);
    }

    bool bCont = true;

    if (rInfo.nTypeArray[j] == NF_SYMBOLTYPE_EXP)
    {
        const OUString& rStr = rInfo.sStrArray[j];
        if (nExpSign == -1)
        {
            ExpStr.insert(0, '-');
        }
        else if (rStr.getLength() > 1 && rStr[1] == '+')
        {
            ExpStr.insert(0, '+');
        }
        ExpStr.insert(0, rStr[0]);
        if ( j )
        {
            j--;
        }
        else
        {
            bCont = false;
        }
    }
    // Continue main number:
    if ( !bCont )
    {
        sStr.truncate();
    }
    else
    {
        bRes |= ImpDecimalFill(sStr, fNumber, nDecPos, j, nIx, false);
    }

    if (bSign)
    {
        sStr.insert(0, '-');
    }
    sStr.append(ExpStr);

    return bRes;
}

double SvNumberformat::GetRoundFractionValue ( double fNumber ) const
{
    sal_uInt16 nIx = GetSubformatIndex ( fNumber );
    double fIntPart = 0.0;           // integer part of fraction
    sal_uInt64 nFrac = 0, nDiv = 1;  // numerator and denominator
    double fSign = (fNumber < 0.0) ? -1.0 : 1.0;
    // fNumber is modified in ImpGetFractionElements to absolute fractional part
    ImpGetFractionElements ( fNumber, nIx, fIntPart, nFrac, nDiv );
    if ( nDiv > 0 )
        return fSign * ( fIntPart + static_cast<double>(nFrac) / static_cast<double>(nDiv) );
    else
        return fSign * fIntPart;
}

void SvNumberformat::ImpGetFractionElements ( double& fNumber, sal_uInt16 nIx,
                                              double& fIntPart, sal_uInt64& nFrac, sal_uInt64& nDiv ) const
{
    if ( fNumber < 0.0 )
        fNumber = -fNumber;
    fIntPart = floor(fNumber); // Integral part
    fNumber -= fIntPart;         // Fractional part
    const ImpSvNumberformatInfo& rInfo = NumFor[nIx].Info();
    nDiv = lcl_GetDenominatorString( rInfo, NumFor[nIx].GetCount() ).toInt32();
    if( nDiv > 0 )
    {   // Forced Denominator
        nFrac = static_cast<sal_uInt64>(floor ( fNumber * nDiv ));
        double fFracNew = static_cast<double>(nFrac) / static_cast<double>(nDiv);
        double fFracNew1 = static_cast<double>(nFrac + 1) / static_cast<double>(nDiv);
        double fDiff = fNumber - fFracNew;
        if( fDiff > ( fFracNew1 - fNumber ) )
        {
            nFrac++;
        }
    }
    else // Calculated Denominator
    {
        nDiv = 1;
        sal_uInt64 nBasis = static_cast<sal_uInt64>(floor( pow(10.0,rInfo.nCntExp))) - 1; // 9, 99, 999 ,...
        sal_uInt64 nFracPrev = 1, nDivPrev = 0, nFracNext, nDivNext, nPartialDenom;
        double fRemainder = fNumber;

        // Use continued fraction representation of fNumber
        // See https://en.wikipedia.org/wiki/Continued_fraction#Best_rational_approximations
        while ( fRemainder > 0.0 )
        {
            double fTemp = 1.0 / fRemainder;             // 64bits precision required when fRemainder is very weak
            nPartialDenom = static_cast<sal_uInt64>(floor(fTemp));   // due to floating point notation with double precision
            fRemainder = fTemp - static_cast<double>(nPartialDenom);
            nDivNext = nPartialDenom * nDiv + nDivPrev;
            if ( nDivNext <= nBasis )  // continue loop
            {
                nFracNext = nPartialDenom * nFrac + nFracPrev;
                nFracPrev = nFrac;
                nFrac = nFracNext;
                nDivPrev = nDiv;
                nDiv = nDivNext;
            }
            else // calculate collateral fraction and exit
            {
                sal_uInt64 nCollat = (nBasis - nDivPrev) / nDiv;
                if ( 2 * nCollat >= nPartialDenom )
                {
                    sal_uInt64 nFracTest = nCollat * nFrac + nFracPrev;
                    sal_uInt64 nDivTest  = nCollat * nDiv  + nDivPrev;
                    double fSign = (static_cast<double>(nFrac) > fNumber * static_cast<double>(nDiv))?1.0:-1.0;
                    if ( fSign * ( double(nFrac * nDivTest + nDiv * nFracTest) - 2.0 * double(nDiv * nDivTest) * fNumber ) > 0.0 )
                    {
                        nFrac = nFracTest;
                        nDiv  = nDivTest;
                    }
                }
                fRemainder = 0.0; // exit while loop
            }
        }
    }
    if (nFrac >= nDiv)
    {
        ++fIntPart;
        nFrac = nDiv = 0;
    }
}

bool SvNumberformat::ImpGetFractionOutput(double fNumber,
                                          sal_uInt16 nIx,
                                          OUStringBuffer& sBuff)
{
    bool bRes = false;
    const ImpSvNumberformatInfo& rInfo = NumFor[nIx].Info();
    const sal_uInt16 nCnt = NumFor[nIx].GetCount();
    OUStringBuffer sStr, sFrac, sDiv; // Strings, value for Integral part Numerator and denominator
    bool bSign = ( (fNumber < 0) && (nIx == 0) ); // sign Not in the ones at the end
    const OUString sIntegerFormat = lcl_GetFractionIntegerString(rInfo, nCnt);
    const OUString sNumeratorFormat = lcl_GetNumeratorString(rInfo, nCnt);
    const OUString sDenominatorFormat = lcl_GetDenominatorString(rInfo, nCnt);

    sal_uInt64 nFrac = 0, nDiv = 1;
    double fNum = floor(fNumber); // Integral part

    if (fNum > D_MAX_U_INT32 || rInfo.nCntExp > 9) // Too large
    {
        sBuff = ImpSvNumberformatScan::GetErrorString();
        return false;
    }
    if (rInfo.nCntExp == 0)
    {
        SAL_WARN( "svl.numbers", "SvNumberformat:: Fraction, nCntExp == 0");
        sBuff.truncate();
        return false;
    }

    ImpGetFractionElements( fNumber, nIx, fNum, nFrac, nDiv);

    if (rInfo.nCntPre == 0) // Improper fraction
    {
        double fNum1 = fNum * static_cast<double>(nDiv) + static_cast<double>(nFrac);

        if (fNum1 > D_MAX_U_INT32)
        {
            sBuff = ImpSvNumberformatScan::GetErrorString();
            return false;
        }
        nFrac = static_cast<sal_uInt64>(floor(fNum1));
    }
    else if (fNum == 0.0 && nFrac != 0)
    {
    }
    else
    {
        char aBuf[100];
        sprintf( aBuf, "%.f", fNum ); // simple rounded integer (#100211# - checked)
        sStr.appendAscii( aBuf );
        impTransliterate(sStr, NumFor[nIx].GetNatNum());
    }
    bool bHideFraction = (rInfo.nCntPre > 0 && nFrac == 0
                        && (sNumeratorFormat.indexOf('0') < 0)
                        && (sDenominatorFormat.indexOf('0') < 0
                        || sDenominatorFormat.toInt32() > 0) );
    if ( bHideFraction )
    {
        sDiv.truncate();
    }
    else  // if there are some '0' in format, force display of fraction
    {
        sFrac = ImpIntToString( nIx, nFrac );
        sDiv = ImpIntToString( nIx, nDiv );
    }

    sal_uInt16 j = nCnt-1; // Last symbol -> backwards
    sal_Int32 k;           // Denominator

    bRes |= ImpNumberFill(sDiv, fNumber, k, j, nIx, NF_SYMBOLTYPE_FRAC, true);

    bool bCont = true;
    if (rInfo.nTypeArray[j] == NF_SYMBOLTYPE_FRAC)
    {
        if ( bHideFraction )
        {   // do not insert blank for fraction if there is no '?'
            if ( sNumeratorFormat.indexOf('?') >= 0
              || sDenominatorFormat.indexOf('?') >= 0 )
                sDiv.insert(0, ' ');
        }
        else
        {
            sDiv.insert(0, rInfo.sStrArray[j][0]);
        }
        if ( j )
        {
            j--;
        }
        else
        {
            bCont = false;
        }
    }
    // Further numerators:
    if ( !bCont )
    {
        sFrac.truncate();
    }
    else
    {
        bRes |= ImpNumberFill(sFrac, fNumber, k, j, nIx, NF_SYMBOLTYPE_FRACBLANK);
        bCont = false;  // there is no integer part?
        if (rInfo.nTypeArray[j] == NF_SYMBOLTYPE_FRACBLANK)
        {
            if ( j )
            {
                if ( bHideFraction )
                {   // '?' in any format force display of blank as delimiter
                    if ( sIntegerFormat.indexOf('?') >= 0
                      || sNumeratorFormat.indexOf('?') >= 0
                      || sDenominatorFormat.indexOf('?') >= 0 )
                    {
                        for (sal_Int32 i = 0; i < rInfo.sStrArray[j].getLength(); i++)
                            sFrac.insert(0, ' ');
                    }
                }
                else
                {
                    if ( fNum != 0.0 || sIntegerFormat.indexOf('0') >= 0 )
                        sFrac.insert(0, rInfo.sStrArray[j]); // insert Blank string only if there are both integer and fraction
                    else
                    {
                        if ( sIntegerFormat.indexOf('?') >= 0
                          || sNumeratorFormat.indexOf('?') >= 0 )
                        {
                            for (sal_Int32 i = 0; i < rInfo.sStrArray[j].getLength(); i++)
                                sFrac.insert(0, ' ');
                        }
                    }
                }
                j--;
                bCont = true;  // Yes, there is an integer
            }
            else
                sFrac.insert(0, rInfo.sStrArray[j]);
        }
    }
    // Continue integer part
    if ( !bCont )
    {
        sStr.truncate();
    }
    else
    {
        k = sStr.getLength(); // After last figure
        bRes |= ImpNumberFillWithThousands(sStr, fNumber, k, j, nIx,
                                           rInfo.nCntPre);
    }
    if (bSign && !(nFrac == 0 && fNum == 0.0))
    {
        sBuff.insert(0, '-'); // Not -0
    }
    sBuff.append(sStr);
    sBuff.append(sFrac);
    sBuff.append(sDiv);
    return bRes;
}

sal_uInt16 SvNumberformat::ImpGetFractionOfSecondString( OUStringBuffer& rBuf, double fFractionOfSecond,
        int nFractionDecimals, bool bAddOneRoundingDecimal, sal_uInt16 nIx, sal_uInt16 nMinimumInputLineDecimals )
{
    if (!nFractionDecimals)
        return 0;

    // nFractionDecimals+1 to not round up what Time::GetClock() carefully
    // truncated.
    rBuf.append( rtl::math::doubleToUString( fFractionOfSecond, rtl_math_StringFormat_F,
                (bAddOneRoundingDecimal ? nFractionDecimals + 1 : nFractionDecimals), '.'));
    rBuf.stripStart('0');
    rBuf.stripStart('.');
    if (bAddOneRoundingDecimal && rBuf.getLength() > nFractionDecimals)
        rBuf.truncate( nFractionDecimals); // the digit appended because of nFractionDecimals+1
    if (nMinimumInputLineDecimals)
    {
        rBuf.stripEnd('0');
        for (sal_Int32 index = rBuf.getLength(); index < nMinimumInputLineDecimals; ++index)
        {
            rBuf.append('0');
        }
        impTransliterate(rBuf, NumFor[nIx].GetNatNum());
        nFractionDecimals = rBuf.getLength();
    }
    else
    {
        impTransliterate(rBuf, NumFor[nIx].GetNatNum());
    }
    return static_cast<sal_uInt16>(nFractionDecimals);
}

bool SvNumberformat::ImpGetTimeOutput(double fNumber,
                                      sal_uInt16 nIx,
                                      OUStringBuffer& sBuff)
{
    using namespace ::com::sun::star::i18n;
    bool bCalendarSet = false;
    const double fNumberOrig = fNumber;
    bool bRes = false;
    bool bSign = false;
    if (fNumber < 0.0)
    {
        fNumber = -fNumber;
        if (nIx == 0)
        {
            bSign = true;
        }
    }
    const ImpSvNumberformatInfo& rInfo = NumFor[nIx].Info();
    if (rInfo.bThousand) // [] format
    {
        if (fNumber > 1.0E10) // Too large
        {
            sBuff = ImpSvNumberformatScan::GetErrorString();
            return false;
        }
    }
    bool bInputLine;
    sal_Int32 nCntPost;
    if ( rScan.GetStandardPrec() == SvNumberFormatter::INPUTSTRING_PRECISION &&
         0 < rInfo.nCntPost && rInfo.nCntPost < 7 )
    {   // round at 7 decimals (+5 of 86400 == 12 significant digits)
        bInputLine = true;
        nCntPost = 7;
    }
    else
    {
        bInputLine = false;
        nCntPost = rInfo.nCntPost;
    }

    OUStringBuffer sSecStr;
    sal_Int32 nSecPos = 0; // For figure by figure processing
    sal_uInt32 nHour, nMin, nSec;
    if (!rInfo.bThousand) // No [] format
    {
        sal_uInt16 nCHour, nCMinute, nCSecond;
        double fFractionOfSecond;
        tools::Time::GetClock( fNumberOrig, nCHour, nCMinute, nCSecond, fFractionOfSecond, nCntPost);
        nHour = nCHour;
        nMin = nCMinute;
        nSec = nCSecond;
        nCntPost = ImpGetFractionOfSecondString( sSecStr, fFractionOfSecond, nCntPost, true, nIx,
                (bInputLine ? rInfo.nCntPost : 0));
    }
    else
    {
        const double fTime = rtl::math::round( fNumber * 86400.0, int(nCntPost));
        if (bSign && fTime == 0.0)
        {
            bSign = false; // Not -00:00:00
        }
        if (fTime > D_MAX_U_INT32)
        {
            sBuff = ImpSvNumberformatScan::GetErrorString();
            return false;
        }
        sal_uInt32 nSeconds = static_cast<sal_uInt32>(fTime);

        nCntPost = ImpGetFractionOfSecondString( sSecStr, fTime - nSeconds, nCntPost, false, nIx,
                (bInputLine ? rInfo.nCntPost : 0));

        if (rInfo.nThousand == 3) // [ss]
        {
            nHour = 0;
            nMin = 0;
            nSec = nSeconds;
        }
        else if (rInfo.nThousand == 2) // [mm]:ss
        {
            nHour = 0;
            nMin = nSeconds / 60;
            nSec = nSeconds % 60;
        }
        else if (rInfo.nThousand == 1) // [hh]:mm:ss
        {
            nHour = nSeconds / 3600;
            nMin = (nSeconds%3600) / 60;
            nSec = nSeconds%60;
        }
        else
        {
            // TODO  What should these be set to?
            nHour = 0;
            nMin  = 0;
            nSec  = 0;
        }
    }

    sal_Unicode cAmPm = ' '; // a or p
    if (rInfo.nCntExp) // AM/PM
    {
        if (nHour == 0)
        {
            nHour = 12;
            cAmPm = 'a';
        }
        else if (nHour < 12)
        {
            cAmPm = 'a';
        }
        else
        {
            cAmPm = 'p';
            if (nHour > 12)
            {
                nHour -= 12;
            }
        }
    }
    const sal_uInt16 nCnt = NumFor[nIx].GetCount();
    for (sal_uInt16 i = 0; i < nCnt; i++)
    {
        sal_Int32 nLen;
        switch (rInfo.nTypeArray[i])
        {
        case NF_SYMBOLTYPE_STAR:
            if( bStarFlag )
            {
                bRes = lcl_appendStarFillChar( sBuff, rInfo.sStrArray[i]);
            }
            break;
        case NF_SYMBOLTYPE_BLANK:
            if (rInfo.sStrArray[i].getLength() >= 2)
                InsertBlanks(sBuff, sBuff.getLength(), rInfo.sStrArray[i][1] );
            break;
        case NF_SYMBOLTYPE_STRING:
        case NF_SYMBOLTYPE_CURRENCY:
        case NF_SYMBOLTYPE_DATESEP:
        case NF_SYMBOLTYPE_TIMESEP:
        case NF_SYMBOLTYPE_TIME100SECSEP:
            sBuff.append(rInfo.sStrArray[i]);
            break;
        case NF_SYMBOLTYPE_DIGIT:
            nLen = ( bInputLine && i > 0 &&
                     (rInfo.nTypeArray[i-1] == NF_SYMBOLTYPE_STRING ||
                      rInfo.nTypeArray[i-1] == NF_SYMBOLTYPE_TIME100SECSEP) ?
                     nCntPost : rInfo.sStrArray[i].getLength() );
            for (sal_Int32 j = 0; j < nLen && nSecPos < nCntPost && nSecPos < sSecStr.getLength(); ++j)
            {
                sBuff.append(sSecStr[nSecPos]);
                nSecPos++;
            }
            break;
        case NF_KEY_AMPM:               // AM/PM
            if ( !bCalendarSet )
            {
                double fDiff = DateTime(rScan.GetNullDate()) - GetCal().getEpochStart();
                fDiff += fNumberOrig;
                GetCal().setLocalDateTime( fDiff );
                bCalendarSet = true;
            }
            if (cAmPm == 'a')
            {
                sBuff.append(GetCal().getDisplayName(
                                 CalendarDisplayIndex::AM_PM, AmPmValue::AM, 0 ));
            }
            else
            {
                sBuff.append(GetCal().getDisplayName(
                                 CalendarDisplayIndex::AM_PM, AmPmValue::PM, 0 ));
            }
            break;
        case NF_KEY_AP:                 // A/P
            if (cAmPm == 'a')
            {
                sBuff.append('a');
            }
            else
            {
                sBuff.append('p');
            }
            break;
        case NF_KEY_MI:                 // M
            sBuff.append(ImpIntToString( nIx, nMin ));
            break;
        case NF_KEY_MMI:                // MM
            sBuff.append(ImpIntToString( nIx, nMin, 2 ));
            break;
        case NF_KEY_H:                  // H
            sBuff.append(ImpIntToString( nIx, nHour ));
            break;
        case NF_KEY_HH:                 // HH
            sBuff.append(ImpIntToString( nIx, nHour, 2 ));
            break;
        case NF_KEY_S:                  // S
            sBuff.append(ImpIntToString( nIx, nSec ));
            break;
        case NF_KEY_SS:                 // SS
            sBuff.append(ImpIntToString( nIx, nSec, 2 ));
            break;
        default:
            break;
        }
    }
    if (bSign && rInfo.bThousand)
    {
        sBuff.insert(0, '-');
    }
    return bRes;
}


/** If a day of month occurs within the format, the month name is in possessive
    genitive case if the day follows the month, and partitive case if the day
    precedes the month. If there is no day of month the nominative case (noun)
    is returned. Also if the month is immediately preceded or followed by a
    literal string other than space the nominative name is used, this prevents
    duplicated casing for MMMM\t\a and such in documents imported from (e.g.
    Finnish) Excel or older LibO/OOo releases.
 */

// IDEA: instead of eCodeType pass the index to nTypeArray and restrict
// inspection of month name around that one, that would enable different month
// cases in one format. Though probably the most rare use case ever..

sal_Int32 SvNumberformat::ImpUseMonthCase( int & io_nState, const ImpSvNumFor& rNumFor, NfKeywordIndex eCodeType )
{
    using namespace ::com::sun::star::i18n;
    if (!io_nState)
    {
        bool bMonthSeen = false;
        bool bDaySeen = false;
        const ImpSvNumberformatInfo& rInfo = rNumFor.Info();
        const sal_uInt16 nCount = rNumFor.GetCount();
        for (sal_uInt16 i = 0; i < nCount && io_nState == 0; ++i)
        {
            sal_Int32 nLen;
            switch (rInfo.nTypeArray[i])
            {
            case NF_KEY_D :
            case NF_KEY_DD :
                if (bMonthSeen)
                {
                    io_nState = 2;
                }
                else
                {
                    bDaySeen = true;
                }
                break;
            case NF_KEY_MMM:
            case NF_KEY_MMMM:
            case NF_KEY_MMMMM:
                if ((i < nCount-1 &&
                     rInfo.nTypeArray[i+1] == NF_SYMBOLTYPE_STRING &&
                     rInfo.sStrArray[i+1][0] != ' ') ||
                    (i > 0 && rInfo.nTypeArray[i-1] == NF_SYMBOLTYPE_STRING &&
                     ((nLen = rInfo.sStrArray[i-1].getLength()) > 0) &&
                     rInfo.sStrArray[i-1][nLen-1] != ' '))
                {
                    io_nState = 1;
                }
                else if (bDaySeen)
                {
                    io_nState = 3;
                }
                else
                {
                    bMonthSeen = true;
                }
                break;
            }
        }
        if (io_nState == 0)
        {
            io_nState = 1; // No day of month
        }
    }
    switch (io_nState)
    {
    case 1:
        // No day of month or forced nominative
        switch (eCodeType)
        {
        case NF_KEY_MMM:
            return CalendarDisplayCode::SHORT_MONTH_NAME;
        case NF_KEY_MMMM:
            return CalendarDisplayCode::LONG_MONTH_NAME;
        case NF_KEY_MMMMM:
            return CalendarDisplayCode::NARROW_MONTH_NAME;
        default:
            ;   // nothing
        }
        break;
    case 2:
        // Day of month follows month (the month's 17th)
        switch (eCodeType)
        {
        case NF_KEY_MMM:
            return CalendarDisplayCode::SHORT_GENITIVE_MONTH_NAME;
        case NF_KEY_MMMM:
            return CalendarDisplayCode::LONG_GENITIVE_MONTH_NAME;
        case NF_KEY_MMMMM:
            return CalendarDisplayCode::NARROW_GENITIVE_MONTH_NAME;
        default:
            ;   // Nothing
        }
        break;
    case 3:
        // Day of month precedes month (17 of month)
        switch (eCodeType)
        {
        case NF_KEY_MMM:
            return CalendarDisplayCode::SHORT_PARTITIVE_MONTH_NAME;
        case NF_KEY_MMMM:
            return CalendarDisplayCode::LONG_PARTITIVE_MONTH_NAME;
        case NF_KEY_MMMMM:
            return CalendarDisplayCode::NARROW_PARTITIVE_MONTH_NAME;
        default:
            ;   // nothing
        }
        break;
    }
    SAL_WARN( "svl.numbers", "ImpUseMonthCase: unhandled keyword index eCodeType");
    return CalendarDisplayCode::LONG_MONTH_NAME;
}


bool SvNumberformat::ImpIsOtherCalendar( const ImpSvNumFor& rNumFor ) const
{
    if ( GetCal().getUniqueID() != GREGORIAN )
    {
        return false;
    }
    const ImpSvNumberformatInfo& rInfo = rNumFor.Info();
    const sal_uInt16 nCnt = rNumFor.GetCount();
    sal_uInt16 i;
    for ( i = 0; i < nCnt; i++ )
    {
        switch ( rInfo.nTypeArray[i] )
        {
        case NF_SYMBOLTYPE_CALENDAR :
            return false;
        case NF_KEY_EC :
        case NF_KEY_EEC :
        case NF_KEY_R :
        case NF_KEY_RR :
        case NF_KEY_AAA :
        case NF_KEY_AAAA :
        case NF_KEY_G :
        case NF_KEY_GG :
        case NF_KEY_GGG :
            return true;
        }
    }
    return false;
}

void SvNumberformat::SwitchToOtherCalendar( OUString& rOrgCalendar,
                                            double& fOrgDateTime ) const
{
    CalendarWrapper& rCal = GetCal();
    if ( rCal.getUniqueID() != GREGORIAN )
        return;

    using namespace ::com::sun::star::i18n;
    css::uno::Sequence< OUString > xCals = rCal.getAllCalendars(
            rLoc().getLanguageTag().getLocale() );
    sal_Int32 nCnt = xCals.getLength();
    if ( nCnt <= 1 )
        return;

    for ( sal_Int32 j=0; j < nCnt; j++ )
    {
        if ( xCals[j] != GREGORIAN )
        {
            if ( !rOrgCalendar.getLength() )
            {
                rOrgCalendar = rCal.getUniqueID();
                fOrgDateTime = rCal.getDateTime();
            }
            rCal.loadCalendar( xCals[j], rLoc().getLanguageTag().getLocale() );
            rCal.setDateTime( fOrgDateTime );
            break;  // for
        }
    }
}

void SvNumberformat::SwitchToGregorianCalendar( const OUString& rOrgCalendar,
                                                double fOrgDateTime ) const
{
    CalendarWrapper& rCal = GetCal();
    if ( rOrgCalendar.getLength() && rCal.getUniqueID() != GREGORIAN )
    {
        rCal.loadCalendar( GREGORIAN, rLoc().getLanguageTag().getLocale() );
        rCal.setDateTime( fOrgDateTime );
    }
}

bool SvNumberformat::ImpFallBackToGregorianCalendar( OUString& rOrgCalendar, double& fOrgDateTime )
{
    using namespace ::com::sun::star::i18n;
    CalendarWrapper& rCal = GetCal();
    if ( rCal.getUniqueID() != GREGORIAN )
    {
        sal_Int16 nVal = rCal.getValue( CalendarFieldIndex::ERA );
        if ( nVal == 0 && rCal.getLoadedCalendar().Eras[0].ID == "Dummy" )
        {
            if ( !rOrgCalendar.getLength() )
            {
                rOrgCalendar = rCal.getUniqueID();
                fOrgDateTime = rCal.getDateTime();
            }
            else if ( rOrgCalendar == GREGORIAN )
            {
                rOrgCalendar.clear();
            }
            rCal.loadCalendar( GREGORIAN, rLoc().getLanguageTag().getLocale() );
            rCal.setDateTime( fOrgDateTime );
            return true;
        }
    }
    return false;
}


#ifdef THE_FUTURE
/* XXX NOTE: even if the ImpSwitchToSpecifiedCalendar method is currently
 * unused please don't remove it, it would be needed by
 * SwitchToSpecifiedCalendar(), see comment in
 * ImpSvNumberInputScan::GetDateRef() */

bool SvNumberformat::ImpSwitchToSpecifiedCalendar( OUString& rOrgCalendar,
                                                   double& fOrgDateTime,
                                                   const ImpSvNumFor& rNumFor ) const
{
    const ImpSvNumberformatInfo& rInfo = rNumFor.Info();
    const sal_uInt16 nCnt = rNumFor.GetCount();
    for ( sal_uInt16 i = 0; i < nCnt; i++ )
    {
        if ( rInfo.nTypeArray[i] == NF_SYMBOLTYPE_CALENDAR )
        {
            CalendarWrapper& rCal = GetCal();
            if ( !rOrgCalendar.getLength() )
            {
                rOrgCalendar = rCal.getUniqueID();
                fOrgDateTime = rCal.getDateTime();
            }
            rCal.loadCalendar( rInfo.sStrArray[i], rLoc().getLocale() );
            rCal.setDateTime( fOrgDateTime );
            return true;
        }
    }
    return false;
}
#endif

// static
void SvNumberformat::ImpAppendEraG( OUStringBuffer& OutString,
                                    const CalendarWrapper& rCal,
                                    sal_Int16 nNatNum )
{
    using namespace ::com::sun::star::i18n;
    if ( rCal.getUniqueID() == "gengou" )
    {
        sal_Unicode cEra;
        sal_Int16 nVal = rCal.getValue( CalendarFieldIndex::ERA );
        switch ( nVal )
        {
        case 1:
            cEra = 'M';
            break;
        case 2:
            cEra = 'T';
            break;
        case 3:
            cEra = 'S';
            break;
        case 4:
            cEra = 'H';
            break;
        case 5:
            cEra = 'N';     /* TODO: the real one letter era name is not known yet (2018-07-26) */
            break;
        default:
            cEra = '?';
            break;
        }
        OutString.append(cEra);
    }
    else
    {
        OutString.append(rCal.getDisplayString( CalendarDisplayCode::SHORT_ERA, nNatNum ));
    }
}

bool SvNumberformat::ImpIsIso8601( const ImpSvNumFor& rNumFor ) const
{
    bool bIsIso = false;
    if (eType & SvNumFormatType::DATE)
    {
        enum State
        {
            eNone,
            eAtYear,
            eAtSep1,
            eAtMonth,
            eAtSep2,
            eNotIso
        };
        State eState = eNone;
        auto & rTypeArray = rNumFor.Info().nTypeArray;
        sal_uInt16 nCnt = rNumFor.GetCount();
        for (sal_uInt16 i=0; i < nCnt && !bIsIso && eState != eNotIso; ++i)
        {
            switch ( rTypeArray[i] )
            {
            case NF_KEY_YY:     // two digits not strictly ISO 8601
            case NF_KEY_YYYY:
                if (eState != eNone)
                {
                    eState = eNotIso;
                }
                else
                {
                    eState = eAtYear;
                }
                break;
            case NF_KEY_M:      // single digit not strictly ISO 8601
            case NF_KEY_MM:
                if (eState != eAtSep1)
                {
                    eState = eNotIso;
                }
                else
                {
                    eState = eAtMonth;
                }
                break;
            case NF_KEY_D:      // single digit not strictly ISO 8601
            case NF_KEY_DD:
                if (eState != eAtSep2)
                {
                    eState = eNotIso;
                }
                else
                {
                    bIsIso = true;
                }
                break;
            case NF_SYMBOLTYPE_STRING:
            case NF_SYMBOLTYPE_DATESEP:
                if (rNumFor.Info().sStrArray[i] == "-")
                {
                    if (eState == eAtYear)
                    {
                        eState = eAtSep1;
                    }
                    else if (eState == eAtMonth)
                    {
                        eState = eAtSep2;
                    }
                    else
                    {
                        eState = eNotIso;
                    }
                }
                else
                {
                    eState = eNotIso;
                }
                break;
            default:
                eState = eNotIso;
            }
        }
    }
    else
    {
       SAL_WARN( "svl.numbers", "SvNumberformat::ImpIsIso8601: no date" );
    }
    return bIsIso;
}

static bool lcl_hasEra( const ImpSvNumFor& rNumFor )
{
    const ImpSvNumberformatInfo& rInfo = rNumFor.Info();
    const sal_uInt16 nCnt = rNumFor.GetCount();
    for ( sal_uInt16 i = 0; i < nCnt; i++ )
    {
        switch ( rInfo.nTypeArray[i] )
        {
            case NF_KEY_RR :
            case NF_KEY_G :
            case NF_KEY_GG :
            case NF_KEY_GGG :
                return true;
        }
    }
    return false;
}

static bool lcl_isSignedYear( const CalendarWrapper& rCal, const ImpSvNumFor& rNumFor )
{
    return rCal.getValue( css::i18n::CalendarFieldIndex::ERA ) == 0 &&
        rCal.getUniqueID() == GREGORIAN && !lcl_hasEra( rNumFor );
}

bool SvNumberformat::ImpGetDateOutput(double fNumber,
                                      sal_uInt16 nIx,
                                      OUStringBuffer& sBuff)
{
    using namespace ::com::sun::star::i18n;
    bool bRes = false;

    CalendarWrapper& rCal = GetCal();
    double fDiff = DateTime(rScan.GetNullDate()) - rCal.getEpochStart();
    fNumber += fDiff;
    rCal.setLocalDateTime( fNumber );
    int nUseMonthCase = 0; // Not decided yet
    OUString aOrgCalendar; // empty => not changed yet

    double fOrgDateTime(0.0);
    bool bOtherCalendar = ImpIsOtherCalendar( NumFor[nIx] );
    if ( bOtherCalendar )
    {
        SwitchToOtherCalendar( aOrgCalendar, fOrgDateTime );
    }
    if ( ImpFallBackToGregorianCalendar( aOrgCalendar, fOrgDateTime ) )
    {
        bOtherCalendar = false;
    }
    const ImpSvNumberformatInfo& rInfo = NumFor[nIx].Info();
    const sal_uInt16 nCnt = NumFor[nIx].GetCount();
    sal_Int16 nNatNum = NumFor[nIx].GetNatNum().GetNatNum();
    OUString aStr;

    // NatNum12: if the date format contains more than a date
    // field, it needs to specify in NatNum12 argument
    // which date element needs special formatting:
    //
    // '[NatNum12 ordinal-number]D'              -> "1st"
    // '[NatNum12 D=ordinal-number]D" of "MMMM'  -> "1st of April"
    // '[NatNum12 D=ordinal]D" of "MMMM'         -> "first of April"
    // '[NatNum12 YYYY=year,D=ordinal]D" of "MMMM", "YYYY' -> "first of April, nineteen ninety"
    //
    // Note: set only for YYYY, MMMM, M, DDDD, D and NNN/AAAA in date formats.
    // XXX It's possible to extend this for other keywords and date + time
    // combinations, as required.

    bool bUseSpellout = NatNumTakesParameters(nNatNum) &&
            (nCnt == 1 || NumFor[nIx].GetNatNum().GetParams().indexOf('=') > -1);

    for (sal_uInt16 i = 0; i < nCnt; i++)
    {
        switch (rInfo.nTypeArray[i])
        {
        case NF_SYMBOLTYPE_CALENDAR :
            if ( !aOrgCalendar.getLength() )
            {
                aOrgCalendar = rCal.getUniqueID();
                fOrgDateTime = rCal.getDateTime();
            }
            rCal.loadCalendar( rInfo.sStrArray[i], rLoc().getLanguageTag().getLocale() );
            rCal.setDateTime( fOrgDateTime );
            ImpFallBackToGregorianCalendar( aOrgCalendar, fOrgDateTime );
            break;
        case NF_SYMBOLTYPE_STAR:
            if( bStarFlag )
            {
                bRes = lcl_appendStarFillChar( sBuff, rInfo.sStrArray[i]);
            }
            break;
        case NF_SYMBOLTYPE_BLANK:
            if (rInfo.sStrArray[i].getLength() >= 2)
                InsertBlanks( sBuff, sBuff.getLength(), rInfo.sStrArray[i][1] );
            break;
        case NF_SYMBOLTYPE_STRING:
        case NF_SYMBOLTYPE_CURRENCY:
        case NF_SYMBOLTYPE_DATESEP:
        case NF_SYMBOLTYPE_TIMESEP:
        case NF_SYMBOLTYPE_TIME100SECSEP:
            sBuff.append(rInfo.sStrArray[i]);
            break;
        case NF_KEY_M:                  // M
            aStr = rCal.getDisplayString( CalendarDisplayCode::SHORT_MONTH, nNatNum );
            // NatNum12: support variants of preposition, suffixation or article
            // for example, Catalan "de març", but "d'abril" etc.
            if ( bUseSpellout )
            {
                aStr = impTransliterate(aStr, NumFor[nIx].GetNatNum(), rInfo.nTypeArray[i]);
            }
            sBuff.append(aStr);
            break;
        case NF_KEY_MM:                 // MM
            sBuff.append(rCal.getDisplayString( CalendarDisplayCode::LONG_MONTH, nNatNum ));
            break;
        case NF_KEY_MMM:                // MMM
            sBuff.append(rCal.getDisplayString( ImpUseMonthCase( nUseMonthCase, NumFor[nIx],
                                                                 static_cast<NfKeywordIndex>(rInfo.nTypeArray[i])),
                                                nNatNum));
            break;
        case NF_KEY_MMMM:               // MMMM
            // NatNum12: support variants of preposition, suffixation or article
            // Note: result of the "spell out" conversion can depend from the optional
            // PartitiveMonths or GenitiveMonths defined in the locale data,
            // see description of ImpUseMonthCase(), and locale data in
            // i18npool/source/localedata/data/ and libnumbertext
            aStr = rCal.getDisplayString( ImpUseMonthCase( nUseMonthCase, NumFor[nIx],
                                                                 static_cast<NfKeywordIndex>(rInfo.nTypeArray[i])),
                                                nNatNum);
            if ( bUseSpellout )
            {
                aStr = impTransliterate(aStr, NumFor[nIx].GetNatNum(), rInfo.nTypeArray[i]);
            }
            sBuff.append(aStr);
            break;
        case NF_KEY_MMMMM:              // MMMMM
            sBuff.append(rCal.getDisplayString( ImpUseMonthCase( nUseMonthCase, NumFor[nIx],
                                                                 static_cast<NfKeywordIndex>(rInfo.nTypeArray[i])),
                                                nNatNum));
            break;
        case NF_KEY_Q:                  // Q
            sBuff.append(rCal.getDisplayString( CalendarDisplayCode::SHORT_QUARTER, nNatNum ));
            break;
        case NF_KEY_QQ:                 // QQ
            sBuff.append(rCal.getDisplayString( CalendarDisplayCode::LONG_QUARTER, nNatNum ));
            break;
        case NF_KEY_D:                  // D
            aStr = rCal.getDisplayString( CalendarDisplayCode::SHORT_DAY, nNatNum );
            // NatNum12: support variants of preposition, suffixation or article
            if ( bUseSpellout )
            {
                aStr = impTransliterate(aStr, NumFor[nIx].GetNatNum(), rInfo.nTypeArray[i]);
            }
            sBuff.append(aStr);
            break;
        case NF_KEY_DD:                 // DD
            sBuff.append(rCal.getDisplayString( CalendarDisplayCode::LONG_DAY, nNatNum ));
            break;
        case NF_KEY_DDD:                // DDD
            if ( bOtherCalendar )
            {
                SwitchToGregorianCalendar( aOrgCalendar, fOrgDateTime );
            }
            sBuff.append(rCal.getDisplayString( CalendarDisplayCode::SHORT_DAY_NAME, nNatNum ));
            if ( bOtherCalendar )
            {
                SwitchToOtherCalendar( aOrgCalendar, fOrgDateTime );
            }
            break;
        case NF_KEY_DDDD:               // DDDD
            if ( bOtherCalendar )
            {
                SwitchToGregorianCalendar( aOrgCalendar, fOrgDateTime );
            }
            aStr = rCal.getDisplayString( CalendarDisplayCode::LONG_DAY_NAME, nNatNum );
            // NatNum12: support variants of preposition, suffixation or article
            if ( bUseSpellout )
            {
                aStr = impTransliterate(aStr, NumFor[nIx].GetNatNum(), rInfo.nTypeArray[i]);
            }
            sBuff.append(aStr);
            if ( bOtherCalendar )
            {
                SwitchToOtherCalendar( aOrgCalendar, fOrgDateTime );
            }
            break;
        case NF_KEY_YY:                 // YY
            if ( bOtherCalendar )
            {
                SwitchToGregorianCalendar( aOrgCalendar, fOrgDateTime );
            }
            // Prepend a minus sign if Gregorian BCE and era is not displayed.
            if (lcl_isSignedYear( rCal, NumFor[nIx] ))
            {
                sBuff.append('-');
            }
            sBuff.append(rCal.getDisplayString( CalendarDisplayCode::SHORT_YEAR, nNatNum ));
            if ( bOtherCalendar )
            {
                SwitchToOtherCalendar( aOrgCalendar, fOrgDateTime );
            }
            break;
        case NF_KEY_YYYY:               // YYYY
            if ( bOtherCalendar )
            {
                SwitchToGregorianCalendar( aOrgCalendar, fOrgDateTime );
            }
            // Prepend a minus sign if Gregorian BCE and era is not displayed.
            if (lcl_isSignedYear( rCal, NumFor[nIx] ))
            {
                sBuff.append('-');
            }
            aStr = rCal.getDisplayString( CalendarDisplayCode::LONG_YEAR, nNatNum );
            if (aStr.getLength() < 4)
            {
                using namespace comphelper::string;
                // Ensure that year consists of at least 4 digits, so it
                // can be distinguished from 2 digits display and edited
                // without suddenly being hit by the 2-digit year magic.
                OUStringBuffer aBuf;
                padToLength(aBuf, 4 - aStr.getLength(), '0');
                impTransliterate(aBuf, NumFor[nIx].GetNatNum());
                aBuf.append(aStr);
                aStr = aBuf.makeStringAndClear();
            }
            // NatNum12: support variants of preposition, suffixation or article
            if ( bUseSpellout )
            {
                aStr = impTransliterate(aStr, NumFor[nIx].GetNatNum(), rInfo.nTypeArray[i]);
            }
            sBuff.append(aStr);
            if ( bOtherCalendar )
            {
                SwitchToOtherCalendar( aOrgCalendar, fOrgDateTime );
            }
            break;
        case NF_KEY_EC:                 // E
            sBuff.append(rCal.getDisplayString( CalendarDisplayCode::SHORT_YEAR, nNatNum ));
            break;
        case NF_KEY_EEC:                // EE
        case NF_KEY_R:                  // R
            sBuff.append(rCal.getDisplayString( CalendarDisplayCode::LONG_YEAR, nNatNum ));
            break;
        case NF_KEY_NN:                 // NN
        case NF_KEY_AAA:                // AAA
            sBuff.append(rCal.getDisplayString( CalendarDisplayCode::SHORT_DAY_NAME, nNatNum ));
            break;
        case NF_KEY_NNN:                // NNN
        case NF_KEY_AAAA:               // AAAA
            aStr = rCal.getDisplayString( CalendarDisplayCode::LONG_DAY_NAME, nNatNum );
            // NatNum12: support variants of preposition, suffixation or article
            if ( bUseSpellout )
            {
                aStr = impTransliterate(aStr, NumFor[nIx].GetNatNum(), rInfo.nTypeArray[i]);
            }
            sBuff.append(aStr);
            break;
        case NF_KEY_NNNN:               // NNNN
            sBuff.append(rCal.getDisplayString( CalendarDisplayCode::LONG_DAY_NAME, nNatNum ));
            sBuff.append(rLoc().getLongDateDayOfWeekSep());
            break;
        case NF_KEY_WW :                // WW
            sBuff.append(ImpIntToString( nIx,
                                         rCal.getValue( CalendarFieldIndex::WEEK_OF_YEAR )));
            break;
        case NF_KEY_G:                  // G
            ImpAppendEraG(sBuff, rCal, nNatNum );
            break;
        case NF_KEY_GG:                 // GG
            sBuff.append(rCal.getDisplayString( CalendarDisplayCode::SHORT_ERA, nNatNum ));
            break;
        case NF_KEY_GGG:                // GGG
            sBuff.append(rCal.getDisplayString( CalendarDisplayCode::LONG_ERA, nNatNum ));
            break;
        case NF_KEY_RR:                 // RR => GGGEE
            sBuff.append(rCal.getDisplayString( CalendarDisplayCode::LONG_YEAR_AND_ERA, nNatNum ));
            break;
        }
    }
    if ( aOrgCalendar.getLength() )
    {
        rCal.loadCalendar( aOrgCalendar, rLoc().getLanguageTag().getLocale() );  // restore calendar
    }
    return bRes;
}

bool SvNumberformat::ImpGetDateTimeOutput(double fNumber,
                                          sal_uInt16 nIx,
                                          OUStringBuffer& sBuff)
{
    using namespace ::com::sun::star::i18n;
    bool bRes = false;

    CalendarWrapper& rCal = GetCal();
    double fDiff = DateTime(rScan.GetNullDate()) - rCal.getEpochStart();
    fNumber += fDiff;

    const ImpSvNumberformatInfo& rInfo = NumFor[nIx].Info();
    bool bInputLine;
    sal_Int32 nCntPost;
    if ( rScan.GetStandardPrec() == SvNumberFormatter::INPUTSTRING_PRECISION &&
         0 < rInfo.nCntPost && rInfo.nCntPost < 7 )
    {
        // round at 7 decimals (+5 of 86400 == 12 significant digits)
        bInputLine = true;
        nCntPost = 7;
    }
    else
    {
        bInputLine = false;
        nCntPost = rInfo.nCntPost;
    }
    double fTime = (fNumber - floor( fNumber )) * 86400.0;
    fTime = ::rtl::math::round( fTime, int(nCntPost) );
    if (fTime >= 86400.0)
    {
        // result of fNumber==x.999999999... rounded up, use correct date/time
        fTime -= 86400.0;
        fNumber = floor( fNumber + 0.5) + fTime;
    }
    rCal.setLocalDateTime( fNumber );

    int nUseMonthCase = 0; // Not decided yet
    OUString aOrgCalendar; // empty => not changed yet
    double fOrgDateTime(0.0);
    bool bOtherCalendar = ImpIsOtherCalendar( NumFor[nIx] );
    if ( bOtherCalendar )
    {
        SwitchToOtherCalendar( aOrgCalendar, fOrgDateTime );
    }
    if ( ImpFallBackToGregorianCalendar( aOrgCalendar, fOrgDateTime ) )
    {
        bOtherCalendar = false;
    }
    sal_Int16 nNatNum = NumFor[nIx].GetNatNum().GetNatNum();

    OUStringBuffer sSecStr;
    sal_Int32 nSecPos = 0; // For figure by figure processing
    sal_uInt32 nHour, nMin, nSec;
    if (!rInfo.bThousand) // No [] format
    {
        sal_uInt16 nCHour, nCMinute, nCSecond;
        double fFractionOfSecond;
        tools::Time::GetClock( fNumber, nCHour, nCMinute, nCSecond, fFractionOfSecond, nCntPost);
        nHour = nCHour;
        nMin = nCMinute;
        nSec = nCSecond;
        nCntPost = ImpGetFractionOfSecondString( sSecStr, fFractionOfSecond, nCntPost, true, nIx,
                (bInputLine ? rInfo.nCntPost : 0));
    }
    else
    {
        sal_uInt32 nSeconds = static_cast<sal_uInt32>(floor( fTime ));

        nCntPost = ImpGetFractionOfSecondString( sSecStr, fTime - nSeconds, nCntPost, false, nIx,
                (bInputLine ? rInfo.nCntPost : 0));

        if (rInfo.nThousand == 3) // [ss]
        {
            nHour = 0;
            nMin = 0;
            nSec = nSeconds;
        }
        else if (rInfo.nThousand == 2) // [mm]:ss
        {
            nHour = 0;
            nMin = nSeconds / 60;
            nSec = nSeconds % 60;
        }
        else if (rInfo.nThousand == 1) // [hh]:mm:ss
        {
            nHour = nSeconds / 3600;
            nMin = (nSeconds%3600) / 60;
            nSec = nSeconds%60;
        }
        else
        {
            nHour = 0;  // TODO What should these values be?
            nMin  = 0;
            nSec  = 0;
        }
    }
    sal_Unicode cAmPm = ' '; // a or p
    if (rInfo.nCntExp) // AM/PM
    {
        if (nHour == 0)
        {
            nHour = 12;
            cAmPm = 'a';
        }
        else if (nHour < 12)
        {
            cAmPm = 'a';
        }
        else
        {
            cAmPm = 'p';
            if (nHour > 12)
            {
                nHour -= 12;
            }
        }
    }
    const sal_uInt16 nCnt = NumFor[nIx].GetCount();
    sal_Int32 nLen;
    OUString aYear;
    for (sal_uInt16 i = 0; i < nCnt; i++)
    {
        switch (rInfo.nTypeArray[i])
        {
        case NF_SYMBOLTYPE_CALENDAR :
            if ( !aOrgCalendar.getLength() )
            {
                aOrgCalendar = rCal.getUniqueID();
                fOrgDateTime = rCal.getDateTime();
            }
            rCal.loadCalendar( rInfo.sStrArray[i], rLoc().getLanguageTag().getLocale() );
            rCal.setDateTime( fOrgDateTime );
            ImpFallBackToGregorianCalendar( aOrgCalendar, fOrgDateTime );
            break;
        case NF_SYMBOLTYPE_STAR:
            if( bStarFlag )
            {
                bRes = lcl_appendStarFillChar( sBuff, rInfo.sStrArray[i]);
            }
            break;
        case NF_SYMBOLTYPE_BLANK:
            if (rInfo.sStrArray[i].getLength() >= 2)
                InsertBlanks( sBuff, sBuff.getLength(), rInfo.sStrArray[i][1] );
            break;
        case NF_SYMBOLTYPE_STRING:
        case NF_SYMBOLTYPE_CURRENCY:
        case NF_SYMBOLTYPE_DATESEP:
        case NF_SYMBOLTYPE_TIMESEP:
        case NF_SYMBOLTYPE_TIME100SECSEP:
            sBuff.append(rInfo.sStrArray[i]);
            break;
        case NF_SYMBOLTYPE_DIGIT:
            nLen = ( bInputLine && i > 0 &&
                     (rInfo.nTypeArray[i-1] == NF_SYMBOLTYPE_STRING ||
                      rInfo.nTypeArray[i-1] == NF_SYMBOLTYPE_TIME100SECSEP) ?
                     nCntPost : rInfo.sStrArray[i].getLength() );
            for (sal_Int32 j = 0; j < nLen && nSecPos < nCntPost && nSecPos < sSecStr.getLength(); ++j)
            {
                sBuff.append(sSecStr[ nSecPos ]);
                nSecPos++;
            }
            break;
        case NF_KEY_AMPM:               // AM/PM
            if (cAmPm == 'a')
            {
                sBuff.append(rCal.getDisplayName( CalendarDisplayIndex::AM_PM,
                                                  AmPmValue::AM, 0 ));
            }
            else
            {
                sBuff.append(rCal.getDisplayName( CalendarDisplayIndex::AM_PM,
                                                  AmPmValue::PM, 0 ));
            }
            break;
        case NF_KEY_AP:                 // A/P
            if (cAmPm == 'a')
            {
                sBuff.append('a');
            }
            else
            {
                sBuff.append('p');
            }
            break;
        case NF_KEY_MI:                 // M
            sBuff.append(ImpIntToString( nIx, nMin ));
            break;
        case NF_KEY_MMI:                // MM
            sBuff.append(ImpIntToString( nIx, nMin, 2 ));
            break;
        case NF_KEY_H:                  // H
            sBuff.append(ImpIntToString( nIx, nHour ));
            break;
        case NF_KEY_HH:                 // HH
            sBuff.append(ImpIntToString( nIx, nHour, 2 ));
            break;
        case NF_KEY_S:                  // S
            sBuff.append(ImpIntToString( nIx, nSec ));
            break;
        case NF_KEY_SS:                 // SS
            sBuff.append(ImpIntToString( nIx, nSec, 2 ));
            break;
        case NF_KEY_M:                  // M
            sBuff.append(rCal.getDisplayString(
                             CalendarDisplayCode::SHORT_MONTH, nNatNum ));
            break;
        case NF_KEY_MM:                 // MM
            sBuff.append(rCal.getDisplayString(
                             CalendarDisplayCode::LONG_MONTH, nNatNum ));
            break;
        case NF_KEY_MMM:                // MMM
            sBuff.append(rCal.getDisplayString( ImpUseMonthCase( nUseMonthCase, NumFor[nIx],
                                                                 static_cast<NfKeywordIndex>(rInfo.nTypeArray[i])),
                                                nNatNum));
            break;
        case NF_KEY_MMMM:               // MMMM
            sBuff.append(rCal.getDisplayString( ImpUseMonthCase( nUseMonthCase, NumFor[nIx],
                                                                 static_cast<NfKeywordIndex>(rInfo.nTypeArray[i])),
                                                nNatNum));
            break;
        case NF_KEY_MMMMM:              // MMMMM
            sBuff.append(rCal.getDisplayString( ImpUseMonthCase( nUseMonthCase, NumFor[nIx],
                                                                 static_cast<NfKeywordIndex>(rInfo.nTypeArray[i])),
                                                nNatNum));
            break;
        case NF_KEY_Q:                  // Q
            sBuff.append(rCal.getDisplayString( CalendarDisplayCode::SHORT_QUARTER, nNatNum ));
            break;
        case NF_KEY_QQ:                 // QQ
            sBuff.append(rCal.getDisplayString( CalendarDisplayCode::LONG_QUARTER, nNatNum ));
            break;
        case NF_KEY_D:                  // D
            sBuff.append(rCal.getDisplayString( CalendarDisplayCode::SHORT_DAY, nNatNum ));
            break;
        case NF_KEY_DD:                 // DD
            sBuff.append(rCal.getDisplayString( CalendarDisplayCode::LONG_DAY, nNatNum ));
            break;
        case NF_KEY_DDD:                // DDD
            if ( bOtherCalendar )
            {
                SwitchToGregorianCalendar( aOrgCalendar, fOrgDateTime );
            }
            sBuff.append(rCal.getDisplayString( CalendarDisplayCode::SHORT_DAY_NAME, nNatNum ));
            if ( bOtherCalendar )
            {
                SwitchToOtherCalendar( aOrgCalendar, fOrgDateTime );
            }
            break;
        case NF_KEY_DDDD:               // DDDD
            if ( bOtherCalendar )
            {
                SwitchToGregorianCalendar( aOrgCalendar, fOrgDateTime );
            }
            sBuff.append(rCal.getDisplayString( CalendarDisplayCode::LONG_DAY_NAME, nNatNum ));
            if ( bOtherCalendar )
            {
                SwitchToOtherCalendar( aOrgCalendar, fOrgDateTime );
            }
            break;
        case NF_KEY_YY:                 // YY
            if ( bOtherCalendar )
            {
                SwitchToGregorianCalendar( aOrgCalendar, fOrgDateTime );
            }
            // Prepend a minus sign if Gregorian BCE and era is not displayed.
            if (lcl_isSignedYear( rCal, NumFor[nIx] ))
            {
                sBuff.append('-');
            }
            sBuff.append(rCal.getDisplayString( CalendarDisplayCode::SHORT_YEAR, nNatNum ));
            if ( bOtherCalendar )
            {
                SwitchToOtherCalendar( aOrgCalendar, fOrgDateTime );
            }
            break;
        case NF_KEY_YYYY:               // YYYY
            if ( bOtherCalendar )
            {
                SwitchToGregorianCalendar( aOrgCalendar, fOrgDateTime );
            }
            // Prepend a minus sign if Gregorian BCE and era is not displayed.
            if (lcl_isSignedYear( rCal, NumFor[nIx] ))
            {
                sBuff.append('-');
            }
            aYear = rCal.getDisplayString( CalendarDisplayCode::LONG_YEAR, nNatNum );
            if (aYear.getLength() < 4)
            {
                using namespace comphelper::string;
                // Ensure that year consists of at least 4 digits, so it
                // can be distinguished from 2 digits display and edited
                // without suddenly being hit by the 2-digit year magic.
                OUStringBuffer aBuf;
                padToLength(aBuf, 4 - aYear.getLength(), '0');
                impTransliterate(aBuf, NumFor[nIx].GetNatNum());
                aBuf.append(aYear);
                sBuff.append(aBuf);
            }
            else
            {
                sBuff.append(aYear);
            }
            if ( bOtherCalendar )
            {
                SwitchToOtherCalendar( aOrgCalendar, fOrgDateTime );
            }
            break;
        case NF_KEY_EC:                 // E
            sBuff.append(rCal.getDisplayString( CalendarDisplayCode::SHORT_YEAR, nNatNum ));
            break;
        case NF_KEY_EEC:                // EE
        case NF_KEY_R:                  // R
            sBuff.append(rCal.getDisplayString( CalendarDisplayCode::LONG_YEAR, nNatNum ));
            break;
        case NF_KEY_NN:                 // NN
        case NF_KEY_AAA:                // AAA
            sBuff.append(rCal.getDisplayString( CalendarDisplayCode::SHORT_DAY_NAME, nNatNum ));
            break;
        case NF_KEY_NNN:                // NNN
        case NF_KEY_AAAA:               // AAAA
            sBuff.append(rCal.getDisplayString( CalendarDisplayCode::LONG_DAY_NAME, nNatNum ));
            break;
        case NF_KEY_NNNN:               // NNNN
            sBuff.append(rCal.getDisplayString( CalendarDisplayCode::LONG_DAY_NAME, nNatNum ));
            sBuff.append(rLoc().getLongDateDayOfWeekSep());
            break;
        case NF_KEY_WW :                // WW
            sBuff.append(ImpIntToString( nIx, rCal.getValue( CalendarFieldIndex::WEEK_OF_YEAR )));
            break;
        case NF_KEY_G:                  // G
            ImpAppendEraG( sBuff, rCal, nNatNum );
            break;
        case NF_KEY_GG:                 // GG
            sBuff.append(rCal.getDisplayString( CalendarDisplayCode::SHORT_ERA, nNatNum ));
            break;
        case NF_KEY_GGG:                // GGG
            sBuff.append(rCal.getDisplayString( CalendarDisplayCode::LONG_ERA, nNatNum ));
            break;
        case NF_KEY_RR:                 // RR => GGGEE
            sBuff.append(rCal.getDisplayString( CalendarDisplayCode::LONG_YEAR_AND_ERA, nNatNum ));
            break;
        }
    }
    if ( aOrgCalendar.getLength() )
    {
        rCal.loadCalendar( aOrgCalendar, rLoc().getLanguageTag().getLocale() );  // restore calendar
    }
    return bRes;
}

bool SvNumberformat::ImpGetNumberOutput(double fNumber,
                                        sal_uInt16 nIx,
                                        OUStringBuffer& sStr)
{
    bool bRes = false;
    bool bSign;
    if (fNumber < 0.0)
    {
        bSign = (nIx == 0); // Not in the ones at the back;
        fNumber = -fNumber;
    }
    else
    {
        bSign = false;
        if ( ::rtl::math::isSignBitSet( fNumber ) )
        {
            fNumber = -fNumber; // yes, -0.0 is possible, eliminate '-'
        }
    }
    const ImpSvNumberformatInfo& rInfo = NumFor[nIx].Info();
    if (rInfo.eScannedType == SvNumFormatType::PERCENT)
    {
        if (fNumber < D_MAX_D_BY_100)
        {
            fNumber *= 100.0;
        }
        else
        {
            sStr = ImpSvNumberformatScan::GetErrorString();
            return false;
        }
    }
    sal_uInt16 i, j;
    sal_Int32 nDecPos = -1;
    bool bInteger = false;
    if ( rInfo.nThousand != FLAG_STANDARD_IN_FORMAT )
    {
        // Special formatting only if no GENERAL keyword in format code
        const sal_uInt16 nThousand = rInfo.nThousand;
        long nPrecExp;
        for (i = 0; i < nThousand; i++)
        {
           if (fNumber > D_MIN_M_BY_1000)
           {
               fNumber /= 1000.0;
           }
           else
           {
               fNumber = 0.0;
           }
        }
        if (fNumber > 0.0)
        {
            nPrecExp = GetPrecExp( fNumber );
        }
        else
        {
            nPrecExp = 0;
        }
        if (rInfo.nCntPost) // Decimal places
        {
            if ((rInfo.nCntPost + nPrecExp) > 15 && nPrecExp < 15)
            {
                sStr = ::rtl::math::doubleToUString( fNumber, rtl_math_StringFormat_F, 15-nPrecExp, '.');
                for (long l = 15-nPrecExp; l < static_cast<long>(rInfo.nCntPost); l++)
                {
                    sStr.append('0');
                }
            }
            else
            {
                sStr = ::rtl::math::doubleToUString( fNumber, rtl_math_StringFormat_F, rInfo.nCntPost, '.' );
            }
            sStr.stripStart('0'); // Strip leading zeros
        }
        else if (fNumber == 0.0) // Null
        {
            // Nothing to be done here, keep empty string sStr,
            // ImpNumberFillWithThousands does the rest
        }
        else // Integer
        {
            sStr = ::rtl::math::doubleToUString( fNumber, rtl_math_StringFormat_F, 0, '.');
            sStr.stripStart('0'); // Strip leading zeros
        }
        nDecPos = sStr.indexOf('.' );
        if ( nDecPos >= 0)
        {
            const sal_Unicode* p = sStr.getStr() + nDecPos;
            while ( *++p == '0' )
                ;
            if ( !*p )
            {
                bInteger = true;
            }
            sStr.remove( nDecPos, 1 ); //  Remove .
        }
        if (bSign && (sStr.isEmpty() || checkForAll0s(sStr)))   // Only 00000
        {
            bSign = false;              // Not -0.00
        }
    }                                   // End of != FLAG_STANDARD_IN_FORMAT

                                        // Edit backwards:
    j = NumFor[nIx].GetCount()-1;       // Last symbol
                                        // Decimal places:
    bRes |= ImpDecimalFill( sStr, fNumber, nDecPos, j, nIx, bInteger );
    if (bSign)
    {
        sStr.insert(0, '-');
    }
    impTransliterate(sStr, NumFor[nIx].GetNatNum());
    return bRes;
}

bool SvNumberformat::ImpDecimalFill( OUStringBuffer& sStr,  // number string
                                   double& rNumber,       // number
                                   sal_Int32 nDecPos,     // decimals start
                                   sal_uInt16 j,          // symbol index within format code
                                   sal_uInt16 nIx,        // subformat index
                                   bool bInteger)         // is integer
{
    bool bRes = false;
    bool bFilled = false;               // Was filled?
    const ImpSvNumberformatInfo& rInfo = NumFor[nIx].Info();
    sal_Int32 k = sStr.getLength();     // After last figure
                                        // Decimal places:
    if (rInfo.nCntPost > 0)
    {
        bool bTrailing = true;          // Trailing zeros?
        short nType;
        while (j > 0 &&                 // Backwards
               (nType = rInfo.nTypeArray[j]) != NF_SYMBOLTYPE_DECSEP)
        {
            switch ( nType )
            {
            case NF_SYMBOLTYPE_STAR:
                if( bStarFlag )
                {
                    bRes = lcl_insertStarFillChar( sStr, k, rInfo.sStrArray[j]);
                }
                break;
            case NF_SYMBOLTYPE_BLANK:
                if (rInfo.sStrArray[j].getLength() >= 2)
                    /*k = */ InsertBlanks(sStr, k, rInfo.sStrArray[j][1] );
                break;
            case NF_SYMBOLTYPE_STRING:
            case NF_SYMBOLTYPE_CURRENCY:
            case NF_SYMBOLTYPE_PERCENT:
                sStr.insert(k, rInfo.sStrArray[j]);
                break;
            case NF_SYMBOLTYPE_THSEP:
                if (rInfo.nThousand == 0)
                {
                    sStr.insert(k, rInfo.sStrArray[j]);
                }
                break;
            case NF_SYMBOLTYPE_DIGIT:
            {
                const OUString& rStr = rInfo.sStrArray[j];
                const sal_Unicode* p1 = rStr.getStr();
                const sal_Unicode* p = p1 + rStr.getLength();
                // In case the number of decimals passed are less than the
                // "digits" given, append trailing '0' characters, which here
                // means insert them because literal strings may have been
                // appended already. If they weren't to be '0' characters
                // they'll be changed below, as if decimals with trailing zeros
                // were passed.
                if (nDecPos >= 0 && nDecPos <= k)
                {
                    sal_Int32 nAppend = rStr.getLength() - (k - nDecPos);
                    while (nAppend-- > 0)
                    {
                        sStr.insert( k++, '0');
                    }
                }
                while (k && p1 < p--)
                {
                    const sal_Unicode c = *p;
                    k--;
                    if ( sStr[k] != '0' )
                    {
                        bTrailing = false;
                        bFilled = true;
                    }
                    if (bTrailing)
                    {
                        if ( c == '0' )
                        {
                            bFilled = true;
                        }
                        else if ( c == '-' )
                        {
                            if ( bInteger )
                            {
                                sStr[ k ] = '-';
                            }
                            bFilled = true;
                        }
                        else if ( c == '?' )
                        {
                            sStr[ k ] = ' ';
                            bFilled = true;
                        }
                        else if ( !bFilled ) // #
                        {
                            sStr.remove(k,1);
                        }
                    }
                } // of for
                break;
            } // of case digi
            case NF_KEY_CCC: // CCC currency
                sStr.insert(k, rScan.GetCurAbbrev());
                break;
            case NF_KEY_GENERAL: // Standard in the String
            {
                OUStringBuffer sNum;
                ImpGetOutputStandard(rNumber, sNum);
                sNum.stripStart('-');
                sStr.insert(k, sNum.makeStringAndClear());
                break;
            }
            default:
                break;
            } // of switch
            j--;
        } // of while
    } // of decimal places

    bRes |= ImpNumberFillWithThousands(sStr, rNumber, k, j, nIx, // Fill with . if needed
                                       rInfo.nCntPre, bFilled );

    return bRes;
}

bool SvNumberformat::ImpNumberFillWithThousands( OUStringBuffer& sBuff,  // number string
                                                 double& rNumber,       // number
                                                 sal_Int32 k,           // position within string
                                                 sal_uInt16 j,          // symbol index within format code
                                                 sal_uInt16 nIx,        // subformat index
                                                 sal_Int32 nDigCnt,     // count of integer digits in format
                                                 bool bAddDecSep)       // add decimal separator if necessary
{
    bool bRes = false;
    sal_Int32 nLeadingStringChars = 0; // inserted StringChars before number
    sal_Int32 nDigitCount = 0;         // count of integer digits from the right
    bool bStop = false;
    const ImpSvNumberformatInfo& rInfo = NumFor[nIx].Info();
    // no normal thousands separators if number divided by thousands
    bool bDoThousands = (rInfo.nThousand == 0);
    utl::DigitGroupingIterator aGrouping( GetFormatter().GetLocaleData()->getDigitGrouping());

    while (!bStop) // backwards
    {
        if (j == 0)
        {
            bStop = true;
        }
        switch (rInfo.nTypeArray[j])
        {
        case NF_SYMBOLTYPE_DECSEP:
            aGrouping.reset();
            [[fallthrough]];
        case NF_SYMBOLTYPE_STRING:
        case NF_SYMBOLTYPE_CURRENCY:
        case NF_SYMBOLTYPE_PERCENT:
            if ( rInfo.nTypeArray[j] != NF_SYMBOLTYPE_DECSEP || bAddDecSep )
                sBuff.insert(k, rInfo.sStrArray[j]);
            if ( k == 0 )
            {
                nLeadingStringChars = nLeadingStringChars + rInfo.sStrArray[j].getLength();
            }
            break;
        case NF_SYMBOLTYPE_STAR:
            if( bStarFlag )
            {
                bRes = lcl_insertStarFillChar( sBuff, k, rInfo.sStrArray[j]);
            }
            break;
        case NF_SYMBOLTYPE_BLANK:
            if (rInfo.sStrArray[j].getLength() >= 2)
                /*k = */ InsertBlanks(sBuff, k, rInfo.sStrArray[j][1] );
            break;
        case NF_SYMBOLTYPE_THSEP:
            // #i7284# #102685# Insert separator also if number is divided
            // by thousands and the separator is specified somewhere in
            // between and not only at the end.
            // #i12596# But do not insert if it's a parenthesized negative
            // format like (#,)
            // In fact, do not insert if divided and regex [0#,],[^0#] and
            // no other digit symbol follows (which was already detected
            // during scan of format code, otherwise there would be no
            // division), else do insert. Same in ImpNumberFill() below.
            if ( !bDoThousands && j < NumFor[nIx].GetCount()-1 )
            {
                bDoThousands = ((j == 0) ||
                                (rInfo.nTypeArray[j-1] != NF_SYMBOLTYPE_DIGIT &&
                                 rInfo.nTypeArray[j-1] != NF_SYMBOLTYPE_THSEP) ||
                                (rInfo.nTypeArray[j+1] == NF_SYMBOLTYPE_DIGIT));
            }
            if ( bDoThousands )
            {
                if (k > 0)
                {
                    sBuff.insert(k, rInfo.sStrArray[j]);
                }
                else if (nDigitCount < nDigCnt)
                {
                    // Leading '#' displays nothing (e.g. no leading
                    // separator for numbers <1000 with #,##0 format).
                    // Leading '?' displays blank.
                    // Everything else, including nothing, displays the
                    // separator.
                    sal_Unicode cLeader = 0;
                    if (j > 0 && rInfo.nTypeArray[j-1] == NF_SYMBOLTYPE_DIGIT)
                    {
                        const OUString& rStr = rInfo.sStrArray[j-1];
                        sal_Int32 nLen = rStr.getLength();
                        if (nLen)
                        {
                            cLeader = rStr[ nLen - 1 ];
                        }
                    }
                    switch (cLeader)
                    {
                    case '#':
                        ;   // nothing
                        break;
                    case '?':
                        // replace thousand separator with blank
                        sBuff.insert(k, ' ');
                        break;
                    default:
                        sBuff.insert(k, rInfo.sStrArray[j]);
                    }
                }
                aGrouping.advance();
            }
            break;
        case NF_SYMBOLTYPE_DIGIT:
        {
            const OUString& rStr = rInfo.sStrArray[j];
            const sal_Unicode* p1 = rStr.getStr();
            const sal_Unicode* p = p1 + rStr.getLength();
            while ( p1 < p-- )
            {
                nDigitCount++;
                if (k > 0)
                {
                    k--;
                }
                else
                {
                    switch (*p)
                    {
                    case '0':
                        sBuff.insert(0, '0');
                        break;
                    case '?':
                        sBuff.insert(0, ' ');
                        break;
                    }
                }
                if (nDigitCount == nDigCnt && k > 0)
                {
                    // more digits than specified
                    ImpDigitFill(sBuff, 0, k, nIx, nDigitCount, aGrouping);
                }
            }
            break;
        }
        case NF_KEY_CCC: // CCC currency
            sBuff.insert(k, rScan.GetCurAbbrev());
            break;
        case NF_KEY_GENERAL: // "General" in string
        {
            OUStringBuffer sNum;
            ImpGetOutputStandard(rNumber, sNum);
            sNum.stripStart('-');
            sBuff.insert(k, sNum.makeStringAndClear());
            break;
        }
        default:
            break;
        } // switch
        j--; // next format code string
    } // while

    k = k + nLeadingStringChars;    // MSC converts += to int and then warns, so ...
    if (k > nLeadingStringChars)
    {
        ImpDigitFill(sBuff, nLeadingStringChars, k, nIx, nDigitCount, aGrouping);
    }
    return bRes;
}

void SvNumberformat::ImpDigitFill(OUStringBuffer& sStr,     // number string
                                  sal_Int32 nStart,         // start of digits
                                  sal_Int32 & k,            // position within string
                                  sal_uInt16 nIx,           // subformat index
                                  sal_Int32 & nDigitCount,  // count of integer digits from the right so far
                                  utl::DigitGroupingIterator & rGrouping )  // current grouping
{
    if (NumFor[nIx].Info().bThousand) // Only if grouping fill in separators
    {
        const OUString& rThousandSep = GetFormatter().GetNumThousandSep();
        while (k > nStart)
        {
            if (nDigitCount == rGrouping.getPos())
            {
                sStr.insert( k, rThousandSep );
                rGrouping.advance();
            }
            nDigitCount++;
            k--;
        }
    }
    else // simply skip
    {
        k = nStart;
    }
}

bool SvNumberformat::ImpNumberFill( OUStringBuffer& sBuff, // number string
                                    double& rNumber,       // number for "General" format
                                    sal_Int32& k,          // position within string
                                    sal_uInt16& j,         // symbol index within format code
                                    sal_uInt16 nIx,        // subformat index
                                    short eSymbolType,     // type of stop condition
                                    bool bInsertRightBlank)// insert blank on right for denominator (default = false)
{
    bool bRes = false;
    bool bStop = false;
    const ImpSvNumberformatInfo& rInfo = NumFor[nIx].Info();
    // no normal thousands separators if number divided by thousands
    bool bDoThousands = (rInfo.nThousand == 0);
    bool bFoundNumber = false;
    short nType;

    k = sBuff.getLength(); // behind last digit

    while (!bStop && (nType = rInfo.nTypeArray[j]) != eSymbolType ) // Backwards
    {
        switch ( nType )
        {
        case NF_SYMBOLTYPE_STAR:
            if( bStarFlag )
            {
                if ( bFoundNumber && eSymbolType != NF_SYMBOLTYPE_EXP )
                    k = 0; // tdf#100842 jump to beginning of number before inserting something else
                bRes = lcl_insertStarFillChar( sBuff, k, rInfo.sStrArray[j]);
            }
            break;
        case NF_SYMBOLTYPE_BLANK:
            if (rInfo.sStrArray[j].getLength() >= 2)
            {
                if ( bFoundNumber && eSymbolType != NF_SYMBOLTYPE_EXP )
                    k = 0; // tdf#100842 jump to beginning of number before inserting something else
                k = InsertBlanks(sBuff, k, rInfo.sStrArray[j][1] );
            }
            break;
        case NF_SYMBOLTYPE_THSEP:
            // Same as in ImpNumberFillWithThousands() above, do not insert
            // if divided and regex [0#,],[^0#] and no other digit symbol
            // follows (which was already detected during scan of format
            // code, otherwise there would be no division), else do insert.
            if ( !bDoThousands && j < NumFor[nIx].GetCount()-1 )
            {
                bDoThousands = ((j == 0) ||
                                (rInfo.nTypeArray[j-1] != NF_SYMBOLTYPE_DIGIT &&
                                 rInfo.nTypeArray[j-1] != NF_SYMBOLTYPE_THSEP) ||
                                (rInfo.nTypeArray[j+1] == NF_SYMBOLTYPE_DIGIT));
            }
            if ( bDoThousands && k > 0 )
            {
                sBuff.insert(k, rInfo.sStrArray[j]);
            }
            break;
        case NF_SYMBOLTYPE_DIGIT:
        {
            bFoundNumber = true;
            sal_uInt16 nPosInsertBlank = bInsertRightBlank ? k : 0; // left alignment of denominator
            const OUString& rStr = rInfo.sStrArray[j];
            const sal_Unicode* p1 = rStr.getStr();
            const sal_Unicode* p = p1 + rStr.getLength();
            while ( p1 < p-- )
            {
                if (k > 0)
                {
                    k--;
                }
                else
                {
                    switch (*p)
                    {
                    case '0':
                        sBuff.insert(0, '0');
                        break;
                    case '?':
                        sBuff.insert(nPosInsertBlank, ' ');
                        break;
                    }
                }
            }
        }
        break;
        case NF_KEY_CCC:                // CCC currency
            sBuff.insert(k, rScan.GetCurAbbrev());
            break;
        case NF_KEY_GENERAL: // Standard in the String
        {
            OUStringBuffer sNum;
            bFoundNumber = true;
            ImpGetOutputStandard(rNumber, sNum);
            sNum.stripStart('-');
            sBuff.insert(k, sNum.makeStringAndClear());
        }
        break;
        case NF_SYMBOLTYPE_FRAC_FDIV: // Do Nothing
            if (k > 0)
            {
                k--;
            }
            break;

        default:
            if ( bFoundNumber && eSymbolType != NF_SYMBOLTYPE_EXP )
                k = 0; // tdf#100842 jump to beginning of number before inserting something else
            sBuff.insert(k, rInfo.sStrArray[j]);
            break;
        } // of switch
        if ( j )
            j--; // Next String
        else
            bStop = true;
    } // of while
    return bRes;
}

void SvNumberformat::GetFormatSpecialInfo(bool& bThousand,
                                          bool& IsRed,
                                          sal_uInt16& nPrecision,
                                          sal_uInt16& nLeadingCnt) const
{
    // as before: take info from nNumFor=0 for whole format (for dialog etc.)

    SvNumFormatType nDummyType;
    GetNumForInfo( 0, nDummyType, bThousand, nPrecision, nLeadingCnt );

    // "negative in red" is only useful for the whole format

    const Color* pColor = NumFor[1].GetColor();
    IsRed = fLimit1 == 0.0 && fLimit2 == 0.0 && pColor
        && (*pColor == ImpSvNumberformatScan::GetRedColor());
}

void SvNumberformat::GetNumForInfo( sal_uInt16 nNumFor, SvNumFormatType& rScannedType,
                    bool& bThousand, sal_uInt16& nPrecision, sal_uInt16& nLeadingCnt ) const
{
    // take info from a specified sub-format (for XML export)

    if ( nNumFor > 3 )
    {
        return; // invalid
    }

    const ImpSvNumberformatInfo& rInfo = NumFor[nNumFor].Info();
    rScannedType = rInfo.eScannedType;
    bThousand = rInfo.bThousand;
    nPrecision = (rInfo.eScannedType == SvNumFormatType::FRACTION)
                    ? rInfo.nCntExp  // number of denominator digits for fraction
                    : rInfo.nCntPost;
    sal_Int32 nPosHash = 1;
    if ( rInfo.eScannedType == SvNumFormatType::FRACTION &&
            ( (nPosHash += GetDenominatorString(nNumFor).indexOf('#')) > 0 ) )
        nPrecision -= nPosHash;
    if (bStandard && rInfo.eScannedType == SvNumFormatType::NUMBER)
    {
        // StandardFormat
        nLeadingCnt = 1;
    }
    else
    {
        nLeadingCnt = 0;
        bool bStop = false;
        sal_uInt16 i = 0;
        const sal_uInt16 nCnt = NumFor[nNumFor].GetCount();
        while (!bStop && i < nCnt)
        {
            short nType = rInfo.nTypeArray[i];
            if ( nType == NF_SYMBOLTYPE_DIGIT)
            {
                const sal_Unicode* p = rInfo.sStrArray[i].getStr();
                while ( *p == '#' )
                {
                    p++;
                }
                while ( *p++ == '0' )
                {
                    nLeadingCnt++;
                }
            }
            else if (nType == NF_SYMBOLTYPE_DECSEP
                  || nType == NF_SYMBOLTYPE_EXP
                  || nType == NF_SYMBOLTYPE_FRACBLANK)  // Fraction: stop after integer part,
            {                                           // do not count '0' of fraction
                bStop = true;
            }
            i++;
        }
    }
}

const OUString* SvNumberformat::GetNumForString( sal_uInt16 nNumFor, sal_uInt16 nPos,
            bool bString /* = false */ ) const
{
    if ( nNumFor > 3 )
    {
        return nullptr;
    }
    sal_uInt16 nCnt = NumFor[nNumFor].GetCount();
    if ( !nCnt )
    {
        return nullptr;
    }
    if ( nPos == 0xFFFF )
    {
        nPos = nCnt - 1;
        if ( bString )
        {   // Backwards
            short const * pType = NumFor[nNumFor].Info().nTypeArray.data() + nPos;
            while ( nPos > 0 && (*pType != NF_SYMBOLTYPE_STRING) &&
                    (*pType != NF_SYMBOLTYPE_CURRENCY) )
            {
                pType--;
                nPos--;
            }
            if ( (*pType != NF_SYMBOLTYPE_STRING) && (*pType != NF_SYMBOLTYPE_CURRENCY) )
            {
                return nullptr;
            }
        }
    }
    else if ( nPos > nCnt - 1 )
    {
        return nullptr;
    }
    else if ( bString )
    {
        // forward
        short const * pType = NumFor[nNumFor].Info().nTypeArray.data() + nPos;
        while ( nPos < nCnt && (*pType != NF_SYMBOLTYPE_STRING) &&
                (*pType != NF_SYMBOLTYPE_CURRENCY) )
        {
            pType++;
            nPos++;
        }
        if ( nPos >= nCnt || ((*pType != NF_SYMBOLTYPE_STRING) &&
                              (*pType != NF_SYMBOLTYPE_CURRENCY)) )
        {
            return nullptr;
        }
    }
    return &NumFor[nNumFor].Info().sStrArray[nPos];
}

short SvNumberformat::GetNumForType( sal_uInt16 nNumFor, sal_uInt16 nPos ) const
{
    if ( nNumFor > 3 )
    {
        return 0;
    }
    sal_uInt16 nCnt = NumFor[nNumFor].GetCount();
    if ( !nCnt )
    {
        return 0;
    }
    if ( nPos == 0xFFFF )
    {
        nPos = nCnt - 1;
    }
    else if ( nPos > nCnt - 1 )
    {
        return 0;
    }
    return NumFor[nNumFor].Info().nTypeArray[nPos];
}

bool SvNumberformat::IsNegativeWithoutSign() const
{
    if ( IsSecondSubformatRealNegative() )
    {
        const OUString* pStr = GetNumForString( 1, 0, true );
        if ( pStr )
        {
            return !HasStringNegativeSign( *pStr );
        }
    }
    return false;
}

bool SvNumberformat::IsNegativeInBracket() const
{
    sal_uInt16 nCnt = NumFor[1].GetCount();
    if (!nCnt)
    {
        return false;
    }
    auto& tmp = NumFor[1].Info().sStrArray;
    return tmp[0] == "(" && tmp[nCnt-1] == ")";
}

bool SvNumberformat::HasPositiveBracketPlaceholder() const
{
    sal_uInt16 nCnt = NumFor[0].GetCount();
    return NumFor[0].Info().sStrArray[nCnt-1] == "_)";
}

DateOrder SvNumberformat::GetDateOrder() const
{
    if ( eType & SvNumFormatType::DATE )
    {
        auto& rTypeArray = NumFor[0].Info().nTypeArray;
        sal_uInt16 nCnt = NumFor[0].GetCount();
        for ( sal_uInt16 j=0; j<nCnt; j++ )
        {
            switch ( rTypeArray[j] )
            {
            case NF_KEY_D :
            case NF_KEY_DD :
                return DateOrder::DMY;
            case NF_KEY_M :
            case NF_KEY_MM :
            case NF_KEY_MMM :
            case NF_KEY_MMMM :
            case NF_KEY_MMMMM :
                return DateOrder::MDY;
            case NF_KEY_YY :
            case NF_KEY_YYYY :
            case NF_KEY_EC :
            case NF_KEY_EEC :
            case NF_KEY_R :
            case NF_KEY_RR :
                return DateOrder::YMD;
            }
        }
    }
    else
    {
       SAL_WARN( "svl.numbers", "SvNumberformat::GetDateOrder: no date" );
    }
    return rLoc().getDateOrder();
}

sal_uInt32 SvNumberformat::GetExactDateOrder() const
{
    sal_uInt32 nRet = 0;
    if ( !(eType & SvNumFormatType::DATE) )
    {
        SAL_WARN( "svl.numbers", "SvNumberformat::GetExactDateOrder: no date" );
        return nRet;
    }
    auto& rTypeArray = NumFor[0].Info().nTypeArray;
    sal_uInt16 nCnt = NumFor[0].GetCount();
    int nShift = 0;
    for ( sal_uInt16 j=0; j<nCnt && nShift < 3; j++ )
    {
        switch ( rTypeArray[j] )
        {
        case NF_KEY_D :
        case NF_KEY_DD :
            nRet = (nRet << 8) | 'D';
            ++nShift;
            break;
        case NF_KEY_M :
        case NF_KEY_MM :
        case NF_KEY_MMM :
        case NF_KEY_MMMM :
        case NF_KEY_MMMMM :
            nRet = (nRet << 8) | 'M';
            ++nShift;
            break;
        case NF_KEY_YY :
        case NF_KEY_YYYY :
        case NF_KEY_EC :
        case NF_KEY_EEC :
        case NF_KEY_R :
        case NF_KEY_RR :
            nRet = (nRet << 8) | 'Y';
            ++nShift;
            break;
        }
    }
    return nRet;
}

void SvNumberformat::GetConditions( SvNumberformatLimitOps& rOper1, double& rVal1,
                                    SvNumberformatLimitOps& rOper2, double& rVal2 ) const
{
    rOper1 = eOp1;
    rOper2 = eOp2;
    rVal1  = fLimit1;
    rVal2  = fLimit2;
}

Color* SvNumberformat::GetColor( sal_uInt16 nNumFor ) const
{
    if ( nNumFor > 3 )
    {
        return nullptr;
    }
    return NumFor[nNumFor].GetColor();
}

static void lcl_SvNumberformat_AddLimitStringImpl( OUString& rStr,
                                                   SvNumberformatLimitOps eOp,
                                                   double fLimit, const OUString& rDecSep )
{
    if ( eOp == NUMBERFORMAT_OP_NO )
        return;

    switch ( eOp )
    {
    case NUMBERFORMAT_OP_EQ :
        rStr = "[=";
        break;
    case NUMBERFORMAT_OP_NE :
        rStr = "[<>";
        break;
    case NUMBERFORMAT_OP_LT :
        rStr = "[<";
        break;
    case NUMBERFORMAT_OP_LE :
        rStr = "[<=";
        break;
    case NUMBERFORMAT_OP_GT :
        rStr = "[>";
        break;
    case NUMBERFORMAT_OP_GE :
        rStr = "[>=";
        break;
    default:
        SAL_WARN( "svl.numbers", "unsupported number format" );
        break;
    }
    rStr +=  ::rtl::math::doubleToUString( fLimit,
                                           rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max,
                                           rDecSep[0], true);
    rStr += "]";
}

static void lcl_insertLCID( OUStringBuffer& rFormatStr, sal_uInt32 nLCID, sal_Int32 nPosInsertLCID, bool bDBNumInserted )
{
    if ( nLCID == 0 )
        return;
    if (nPosInsertLCID == rFormatStr.getLength() && !bDBNumInserted)
        // No format code, no locale.
        return;

    OUStringBuffer aLCIDString = OUString::number( nLCID , 16 ).toAsciiUpperCase();
    // Search for only last DBNum which is the last element before insertion position
    if ( bDBNumInserted && nPosInsertLCID >= 8
        && aLCIDString.getLength() > 4
        && rFormatStr.indexOf( "[DBNum", nPosInsertLCID-8) == nPosInsertLCID-8 )
    {   // remove DBNumX code if long LCID
        nPosInsertLCID -= 8;
        rFormatStr.remove( nPosInsertLCID, 8 );
    }
    aLCIDString.insert( 0, "[$-" );
    aLCIDString.append( "]" );
    rFormatStr.insert( nPosInsertLCID, aLCIDString.toString() );
}

/** Increment nAlphabetID for CJK numerals
 * +1 for financial numerals [NatNum2]
 * +2 for Arabic fullwidth numerals [NatNum3]
 * */
static void lcl_incrementAlphabetWithNatNum ( sal_uInt32& nAlphabetID, sal_uInt32 nNatNum )
{
    if ( nNatNum == 2) // financial
        nAlphabetID += 1;
    else if ( nNatNum == 3)
        nAlphabetID += 2;
    nAlphabetID = nAlphabetID << 24;
}

OUString SvNumberformat::GetMappedFormatstring( const NfKeywordTable& rKeywords,
                                                const LocaleDataWrapper& rLocWrp,
                                                LanguageType nOriginalLang /* =LANGUAGE_DONTKNOW */,
                                                bool bSystemLanguage /* =false */ ) const
{
    OUStringBuffer aStr;
    if (maLocale.meSubstitute != LocaleType::Substitute::NONE)
    {
        // XXX: theoretically this could clash with the first subformat's
        // lcl_insertLCID() below, in practice as long as it is used for system
        // time and date modifiers it shouldn't (i.e. there is no calendar or
        // numeral specified as well).
        aStr.append("[$-").append( maLocale.generateCode()).append(']');
    }
    bool bDefault[4];
    // 1 subformat matches all if no condition specified,
    bDefault[0] = ( NumFor[1].GetCount() == 0 && eOp1 == NUMBERFORMAT_OP_NO );
    // with 2 subformats [>=0];[<0] is implied if no condition specified
    bDefault[1] = ( !bDefault[0] && NumFor[2].GetCount() == 0 &&
                    eOp1 == NUMBERFORMAT_OP_GE && fLimit1 == 0.0 &&
                    eOp2 == NUMBERFORMAT_OP_NO && fLimit2 == 0.0 );
    // with 3 or more subformats [>0];[<0];[=0] is implied if no condition specified,
    // note that subformats may be empty (;;;) and NumFor[2].GetCount()>0 is not checked.
    bDefault[2] = ( !bDefault[0] && !bDefault[1] &&
                    eOp1 == NUMBERFORMAT_OP_GT && fLimit1 == 0.0 &&
                    eOp2 == NUMBERFORMAT_OP_LT && fLimit2 == 0.0 );
    bool bDefaults = bDefault[0] || bDefault[1] || bDefault[2];
    // from now on bDefault[] values are used to append empty subformats at the end
    bDefault[3] = false;
    if ( !bDefaults )
    {
        // conditions specified
        if ( eOp1 != NUMBERFORMAT_OP_NO && eOp2 == NUMBERFORMAT_OP_NO )
        {
            bDefault[0] = bDefault[1] = true;                               // [];x
        }
        else if ( eOp1 != NUMBERFORMAT_OP_NO && eOp2 != NUMBERFORMAT_OP_NO &&
                  NumFor[2].GetCount() == 0 )
        {
            bDefault[0] = bDefault[1] = bDefault[2] = bDefault[3] = true;   // [];[];;
        }
        // nothing to do if conditions specified for every subformat
    }
    else if ( bDefault[0] )
    {
        bDefault[0] = false; // a single unconditional subformat is never delimited
    }
    else
    {
        if ( bDefault[2] && NumFor[2].GetCount() == 0 && NumFor[1].GetCount() > 0 )
        {
            bDefault[3] = true; // special cases x;x;; and ;x;;
        }
        for ( int i=0; i<3 && !bDefault[i]; ++i )
        {
            bDefault[i] = true;
        }
    }
    int nSem = 0; // needed ';' delimiters
    int nSub = 0; // subformats delimited so far
    for ( int n=0; n<4; n++ )
    {
        if ( n > 0 && NumFor[n].Info().eScannedType != SvNumFormatType::UNDEFINED )
        {
            nSem++;
        }
        OUString aPrefix;

        if ( !bDefaults )
        {
            switch ( n )
            {
            case 0 :
                lcl_SvNumberformat_AddLimitStringImpl( aPrefix, eOp1,
                                                       fLimit1, rLocWrp.getNumDecimalSep() );
                break;
            case 1 :
                lcl_SvNumberformat_AddLimitStringImpl( aPrefix, eOp2,
                                                       fLimit2, rLocWrp.getNumDecimalSep() );
                break;
            }
        }

        const OUString& rColorName = NumFor[n].GetColorName();
        if ( !rColorName.isEmpty() )
        {
            const NfKeywordTable & rKey = rScan.GetKeywords();
            for ( int j = NF_KEY_FIRSTCOLOR; j <= NF_KEY_LASTCOLOR; j++ )
            {
                if ( rKey[j] == rColorName )
                {
                    aPrefix += "[";
                    aPrefix += rKeywords[j];
                    aPrefix += "]";
                    break;  // for
                }
            }
        }

        SvNumberNatNum aNatNum = NumFor[n].GetNatNum();
        bool bDBNumInserted = false;
        if (aNatNum.IsComplete() && (aNatNum.GetDBNum() > 0 || nOriginalLang != LANGUAGE_DONTKNOW))
        {   // GetFormatStringForExcel() may have changed language to en_US
            if (aNatNum.GetLang() == LANGUAGE_ENGLISH_US && nOriginalLang != LANGUAGE_DONTKNOW)
                aNatNum.SetLang( nOriginalLang );
            if ( aNatNum.GetDBNum() > 0 )
            {
                aPrefix += "[DBNum";
                aPrefix += OUString::number( aNatNum.GetDBNum() );
                aPrefix += "]";
                bDBNumInserted = true;
            }
        }

        sal_uInt16 nCnt = NumFor[n].GetCount();
        if ( nSem && (nCnt || !aPrefix.isEmpty()) )
        {
            for ( ; nSem; --nSem )
            {
                aStr.append( ';' );
            }
            for ( ; nSub <= n; ++nSub )
            {
                bDefault[nSub] = false;
            }
        }

        if ( !aPrefix.isEmpty() )
        {
            aStr.append( aPrefix );
        }
        sal_Int32 nPosInsertLCID = aStr.getLength();
        sal_uInt32 nCalendarID = 0x0000000; // Excel ID of calendar used in sub-format see tdf#36038
        if ( nCnt )
        {
            auto& rTypeArray = NumFor[n].Info().nTypeArray;
            auto& rStrArray = NumFor[n].Info().sStrArray;
            for ( sal_uInt16 j=0; j<nCnt; j++ )
            {
                if ( 0 <= rTypeArray[j] && rTypeArray[j] < NF_KEYWORD_ENTRIES_COUNT )
                {
                    aStr.append( rKeywords[rTypeArray[j]] );
                    if( NF_KEY_NNNN == rTypeArray[j] )
                    {
                        aStr.append( rLocWrp.getLongDateDayOfWeekSep() );
                    }
                }
                else
                {
                    switch ( rTypeArray[j] )
                    {
                    case NF_SYMBOLTYPE_DECSEP :
                        aStr.append( rLocWrp.getNumDecimalSep() );
                        break;
                    case NF_SYMBOLTYPE_THSEP :
                        aStr.append( rLocWrp.getNumThousandSep() );
                        break;
                    case NF_SYMBOLTYPE_EXP :
                        aStr.append( rKeywords[NF_KEY_E] );
                        if ( rStrArray[j].getLength() > 1 && rStrArray[j][1] == '+' )
                            aStr.append( "+" );
                        else
                        // tdf#102370: Excel code for exponent without sign
                            aStr.append( "-" );
                        break;
                    case NF_SYMBOLTYPE_DATESEP :
                        aStr.append( rLocWrp.getDateSep() );
                        break;
                    case NF_SYMBOLTYPE_TIMESEP :
                        aStr.append( rLocWrp.getTimeSep() );
                        break;
                    case NF_SYMBOLTYPE_TIME100SECSEP :
                        aStr.append( rLocWrp.getTime100SecSep() );
                        break;
                    case NF_SYMBOLTYPE_FRACBLANK :
                    case NF_SYMBOLTYPE_STRING :
                        if ( rStrArray[j].getLength() == 1 )
                        {
                            if ( rTypeArray[j] == NF_SYMBOLTYPE_STRING )
                                aStr.append( '\\' );
                            aStr.append( rStrArray[j] );
                        }
                        else
                        {
                            aStr.append( '"' );
                            aStr.append( rStrArray[j] );
                            aStr.append( '"' );
                        }
                        break;
                    case NF_SYMBOLTYPE_CALDEL :
                        if ( rStrArray[j+1] == "gengou" )
                        {
                            nCalendarID = 0x0030000;
                        }
                        else if ( rStrArray[j+1] == "hijri" )
                        {
                            nCalendarID = 0x0060000;
                        }
                        else if ( rStrArray[j+1] == "buddhist" )
                        {
                            nCalendarID = 0x0070000;
                        }
                        else if ( rStrArray[j+1] == "jewish" )
                        {
                            nCalendarID = 0x0080000;
                        }
                        // other calendars (see tdf#36038) not corresponding between LibO and XL
                        if ( nCalendarID > 0 )
                            j = j+2;
                        break;
                    default:
                        aStr.append( rStrArray[j] );
                    }
                }
            }
        }
        sal_uInt32 nAlphabetID = 0x0000000; // Excel ID of alphabet used for numerals see tdf#36038
        LanguageType nLanguageID = LANGUAGE_SYSTEM;
        if ( aNatNum.IsComplete() )
        {
            nLanguageID = MsLangId::getRealLanguage( aNatNum.GetLang());
            if ( aNatNum.GetNatNum() == 0 )
            {
                nAlphabetID = 0x01000000;  // Arabic-european numerals
            }
            else if ( nCalendarID > 0 || aNatNum.GetDBNum() == 0 || aNatNum.GetDBNum() == aNatNum.GetNatNum() )
            {   // if no DBNum code then use long LCID
                // if DBNum value != NatNum value, use DBNum and not extended LCID
                // if calendar, then DBNum will be removed
                LanguageType pri = primary(nLanguageID);
                if ( pri == LANGUAGE_ARABIC_PRIMARY_ONLY )
                        nAlphabetID = 0x02000000;  // Arabic-indic numerals
                else if ( pri == primary(LANGUAGE_FARSI) )
                        nAlphabetID = 0x03000000;  // Farsi numerals
                else if ( pri.anyOf(
                    primary(LANGUAGE_HINDI),
                    primary(LANGUAGE_MARATHI),
                    primary(LANGUAGE_NEPALI) ))
                        nAlphabetID = 0x04000000;  // Devanagari numerals
                else if ( pri == primary(LANGUAGE_BENGALI) )
                        nAlphabetID = 0x05000000;  // Bengali numerals
                else if ( pri == primary(LANGUAGE_PUNJABI) )
                {
                    if ( nLanguageID == LANGUAGE_PUNJABI_ARABIC_LSO )
                        nAlphabetID =  0x02000000;  // Arabic-indic numerals
                    else
                        nAlphabetID = 0x06000000;  // Punjabi numerals
                }
                else if ( pri == primary(LANGUAGE_GUJARATI) )
                        nAlphabetID = 0x07000000;  // Gujarati numerals
                else if ( pri == primary(LANGUAGE_ODIA))
                        nAlphabetID = 0x08000000;  // Odia (Oriya) numerals
                else if ( pri == primary(LANGUAGE_TAMIL))
                        nAlphabetID = 0x09000000;  // Tamil numerals
                else if ( pri == primary(LANGUAGE_TELUGU))
                        nAlphabetID = 0x0A000000;  // Telugu numerals
                else if ( pri == primary(LANGUAGE_KANNADA))
                        nAlphabetID = 0x0B000000;  // Kannada numerals
                else if ( pri == primary(LANGUAGE_MALAYALAM))
                        nAlphabetID = 0x0C000000;  // Malayalam numerals
                else if ( pri == primary(LANGUAGE_THAI))
                {
                    // The Thai T NatNum modifier during Xcl export.
                    if ( rKeywords[NF_KEY_THAI_T] == "T" )
                        nAlphabetID = 0x0D000000;  // Thai numerals
                }
                else if ( pri == primary(LANGUAGE_LAO))
                        nAlphabetID = 0x0E000000;  // Lao numerals
                else if ( pri == primary(LANGUAGE_TIBETAN))
                        nAlphabetID = 0x0F000000;  // Tibetan numerals
                else if ( pri == primary(LANGUAGE_BURMESE))
                        nAlphabetID = 0x10000000;  // Burmese numerals
                else if ( pri == primary(LANGUAGE_TIGRIGNA_ETHIOPIA))
                        nAlphabetID = 0x11000000;  // Tigrigna numerals
                else if ( pri == primary(LANGUAGE_KHMER))
                        nAlphabetID = 0x12000000;  // Khmer numerals
                else if ( pri == primary(LANGUAGE_MONGOLIAN_MONGOLIAN_MONGOLIA))
                {
                    if ( nLanguageID != LANGUAGE_MONGOLIAN_CYRILLIC_MONGOLIA
                      && nLanguageID != LANGUAGE_MONGOLIAN_CYRILLIC_LSO )
                        nAlphabetID = 0x13000000;  // Mongolian numerals
                }
                    // CJK numerals
                else if ( pri == primary(LANGUAGE_JAPANESE))
                {
                    nAlphabetID = 0x1B;
                    lcl_incrementAlphabetWithNatNum ( nAlphabetID, aNatNum.GetNatNum() );
                }
                else if ( pri == primary(LANGUAGE_CHINESE))
                {
                    if ( nLanguageID == LANGUAGE_CHINESE_TRADITIONAL
                      || nLanguageID == LANGUAGE_CHINESE_HONGKONG
                      || nLanguageID == LANGUAGE_CHINESE_MACAU )
                    {
                        nAlphabetID = 0x21;
                        lcl_incrementAlphabetWithNatNum ( nAlphabetID, aNatNum.GetNatNum() );
                    }
                    else // LANGUAGE_CHINESE_SIMPLIFIED
                    {
                        nAlphabetID = 0x1E;
                        lcl_incrementAlphabetWithNatNum ( nAlphabetID, aNatNum.GetNatNum() );
                    }
                }
                else if ( pri == primary(LANGUAGE_KOREAN))
                {
                    if ( aNatNum.GetNatNum() == 9 ) // Hangul
                    {
                        nAlphabetID = 0x27000000;
                    }
                    else
                    {
                        nAlphabetID = 0x24;
                        lcl_incrementAlphabetWithNatNum ( nAlphabetID, aNatNum.GetNatNum() );
                    }
                }
            }
            // Add LCID to DBNum
            if ( aNatNum.GetDBNum() > 0 && nLanguageID == LANGUAGE_SYSTEM )
                nLanguageID = MsLangId::getRealLanguage( aNatNum.GetLang());
        }
        else if (!bSystemLanguage && nOriginalLang != LANGUAGE_DONTKNOW)
        {
            // Explicit locale, write only to the first subformat.
            if (n == 0)
                nLanguageID = MsLangId::getRealLanguage( nOriginalLang);
        }
        else if (bSystemLanguage && maLocale.meLanguageWithoutLocaleData != LANGUAGE_DONTKNOW)
        {
            // Explicit locale but no locale data thus assigned to system
            // locale, preserve for roundtrip, write only to the first
            // subformat.
            if (n == 0)
                nLanguageID = maLocale.meLanguageWithoutLocaleData;
        }
        if ( nCalendarID > 0 )
        {   // Add alphabet and language to calendar
            if ( nAlphabetID == 0 )
                nAlphabetID = 0x01000000;
            if ( nLanguageID == LANGUAGE_SYSTEM && nOriginalLang != LANGUAGE_DONTKNOW )
                nLanguageID = nOriginalLang;
        }
        lcl_insertLCID( aStr, nAlphabetID + nCalendarID + static_cast<sal_uInt16>(nLanguageID), nPosInsertLCID,
                bDBNumInserted);
    }
    for ( ; nSub<4 && bDefault[nSub]; ++nSub )
    {   // append empty subformats
        aStr.append( ';' );
    }
    return aStr.makeStringAndClear();
}

OUString SvNumberformat::ImpGetNatNumString( const SvNumberNatNum& rNum,
                                           sal_Int32 nVal, sal_uInt16 nMinDigits ) const
{
    OUString aStr;
    if ( nMinDigits )
    {
        if ( nMinDigits == 2 )
        {
            // speed up the most common case
            if ( 0 <= nVal && nVal < 10 )
            {
                sal_Unicode aBuf[2];
                aBuf[0] = '0';
                aBuf[1] = '0' + nVal;
                aStr = OUString(aBuf, SAL_N_ELEMENTS(aBuf));
            }
            else
            {
                aStr = OUString::number( nVal );
            }
        }
        else
        {
            OUString aValStr( OUString::number( nVal ) );
            if ( aValStr.getLength() >= nMinDigits )
            {
                aStr = aValStr;
            }
            else
            {
                OUStringBuffer aBuf;
                for(sal_Int32 index = 0; index < nMinDigits - aValStr.getLength(); ++index)
                {
                    aBuf.append('0');
                }
                aBuf.append(aValStr);
                aStr = aBuf.makeStringAndClear();
            }
        }
    }
    else
    {
        aStr = OUString::number( nVal );
    }
    return impTransliterate(aStr, rNum);
}

OUString SvNumberformat::impTransliterateImpl(const OUString& rStr,
                                              const SvNumberNatNum& rNum ) const
{
    css::lang::Locale aLocale( LanguageTag( rNum.GetLang() ).getLocale() );
    return GetFormatter().GetNatNum()->getNativeNumberStringParams(rStr, aLocale, rNum.GetNatNum(),
                                                                   rNum.GetParams());
}

void SvNumberformat::impTransliterateImpl(OUStringBuffer& rStr,
                                          const SvNumberNatNum& rNum ) const
{
    css::lang::Locale aLocale( LanguageTag( rNum.GetLang() ).getLocale() );

    OUString sTemp(rStr.makeStringAndClear());
    sTemp = GetFormatter().GetNatNum()->getNativeNumberStringParams(
        sTemp, aLocale, rNum.GetNatNum(), rNum.GetParams());
    rStr.append(sTemp);
}

OUString SvNumberformat::impTransliterateImpl(const OUString& rStr,
                                              const SvNumberNatNum& rNum,
                                              const sal_uInt16 nDateKey) const
{
    // no KEYWORD=argument list in NatNum12
    if (rNum.GetParams().indexOf('=') == -1)
        return impTransliterateImpl( rStr, rNum);

    const NfKeywordTable & rKeywords = rScan.GetKeywords();

    // Format: KEYWORD=numbertext_prefix, ..., for example:
    // [NatNum12 YYYY=title ordinal,MMMM=article, D=ordinal-number]
    sal_Int32 nField = -1;
    do
    {
        nField = rNum.GetParams().indexOf(rKeywords[nDateKey] + "=", ++nField);
    }
    while (nField != -1 && nField != 0 &&
            !(rNum.GetParams()[nField - 1] == ',' ||
              rNum.GetParams()[nField - 1] == ' '));

    // no format specified for actual keyword
    if (nField == -1)
        return rStr;

    sal_Int32 nKeywordLen = rKeywords[nDateKey].getLength() + 1;
    sal_Int32 nFieldEnd = rNum.GetParams().indexOf(',', nField);

    if (nFieldEnd == -1)
        nFieldEnd = rNum.GetParams().getLength();

    css::lang::Locale aLocale( LanguageTag( rNum.GetLang() ).getLocale() );

    return GetFormatter().GetNatNum()->getNativeNumberStringParams(
        rStr, aLocale, rNum.GetNatNum(),
        rNum.GetParams().copy(nField + nKeywordLen, nFieldEnd - nField - nKeywordLen));
}

void SvNumberformat::GetNatNumXml( css::i18n::NativeNumberXmlAttributes2& rAttr,
                                   sal_uInt16 nNumFor ) const
{
    if ( nNumFor <= 3 )
    {
        const SvNumberNatNum& rNum = NumFor[nNumFor].GetNatNum();
        if ( rNum.IsSet() )
        {
            css::lang::Locale aLocale(
                    LanguageTag( rNum.GetLang() ).getLocale() );

            /* TODO: a new XNativeNumberSupplier2::convertToXmlAttributes()
             * should rather return NativeNumberXmlAttributes2 and places
             * adapted, and whether to fill Spellout or something different
             * should be internal there. */
            css::i18n::NativeNumberXmlAttributes aTmp(
                    GetFormatter().GetNatNum()->convertToXmlAttributes(
                        aLocale, rNum.GetNatNum()));
            rAttr.Locale = aTmp.Locale;
            rAttr.Format = aTmp.Format;
            rAttr.Style = aTmp.Style;
            if ( NatNumTakesParameters(rNum.GetNatNum()) )
            {
                // NatNum12 spell out numbers, dates and money amounts
                rAttr.Spellout = rNum.GetParams();
                // Mutually exclusive.
                rAttr.Format.clear();
                rAttr.Style.clear();
            }
            else
            {
                rAttr.Spellout.clear();
            }
        }
        else
        {
            rAttr = css::i18n::NativeNumberXmlAttributes2();
        }
    }
    else
    {
        rAttr = css::i18n::NativeNumberXmlAttributes2();
    }
}

// static
bool SvNumberformat::HasStringNegativeSign( const OUString& rStr )
{
    // For Sign '-' needs to be at the start or at the end of the string (blanks ignored)
    sal_Int32 nLen = rStr.getLength();
    if ( !nLen )
    {
        return false;
    }
    const sal_Unicode* const pBeg = rStr.getStr();
    const sal_Unicode* const pEnd = pBeg + nLen;
    const sal_Unicode* p = pBeg;
    do
    {   // Start
        if ( *p == '-' )
        {
            return true;
        }
    }
    while ( *p == ' ' && ++p < pEnd );

    p = pEnd - 1;

    do
    {   // End
        if ( *p == '-' )
        {
            return true;
        }
    }
    while ( *p == ' ' && pBeg < --p );
    return false;
}

// static
bool SvNumberformat::IsInQuote( const OUString& rStr, sal_Int32 nPos,
                                sal_Unicode cQuote, sal_Unicode cEscIn, sal_Unicode cEscOut )
{
    sal_Int32 nLen = rStr.getLength();
    if ( nPos >= nLen )
    {
        return false;
    }
    const sal_Unicode* p0 = rStr.getStr();
    const sal_Unicode* p = p0;
    const sal_Unicode* p1 = p0 + nPos;
    bool bQuoted = false;
    while ( p <= p1 )
    {
        if ( *p == cQuote )
        {
            if ( p == p0 )
            {
                bQuoted = true;
            }
            else if ( bQuoted )
            {
                if ( *(p-1) != cEscIn )
                {
                    bQuoted = false;
                }
            }
            else
            {
                if ( *(p-1) != cEscOut )
                {
                    bQuoted = true;
                }
            }
        }
        p++;
    }
    return bQuoted;
}

// static
sal_Int32 SvNumberformat::GetQuoteEnd( const OUString& rStr, sal_Int32 nPos,
                                       sal_Unicode cQuote, sal_Unicode cEscIn )
{
    if ( nPos < 0 )
    {
        return -1;
    }
    sal_Int32 nLen = rStr.getLength();
    if ( nPos >= nLen )
    {
        return -1;
    }
    if ( !IsInQuote( rStr, nPos, cQuote, cEscIn ) )
    {
        if ( rStr[ nPos ] == cQuote )
        {
            return nPos; // Closing cQuote
        }
        return -1;
    }
    const sal_Unicode* p0 = rStr.getStr();
    const sal_Unicode* p = p0 + nPos;
    const sal_Unicode* p1 = p0 + nLen;
    while ( p < p1 )
    {
        if ( *p == cQuote && p > p0 && *(p-1) != cEscIn )
        {
            return sal::static_int_cast< sal_Int32 >(p - p0);
        }
        p++;
    }
    return nLen; // End of String
}

sal_uInt16 SvNumberformat::GetNumForNumberElementCount( sal_uInt16 nNumFor ) const
{
    if ( nNumFor < 4 )
    {
        sal_uInt16 nCnt = NumFor[nNumFor].GetCount();
        return nCnt - ImpGetNumForStringElementCount( nNumFor );
    }
    return 0;
}

sal_uInt16 SvNumberformat::ImpGetNumForStringElementCount( sal_uInt16 nNumFor ) const
{
    sal_uInt16 nCnt = 0;
    sal_uInt16 nNumForCnt = NumFor[nNumFor].GetCount();
    auto& rTypeArray = NumFor[nNumFor].Info().nTypeArray;
    for ( sal_uInt16 j=0; j<nNumForCnt; ++j )
    {
        switch ( rTypeArray[j] )
        {
        case NF_SYMBOLTYPE_STRING:
        case NF_SYMBOLTYPE_CURRENCY:
        case NF_SYMBOLTYPE_DATESEP:
        case NF_SYMBOLTYPE_TIMESEP:
        case NF_SYMBOLTYPE_TIME100SECSEP:
        case NF_SYMBOLTYPE_PERCENT:
            ++nCnt;
            break;
        }
    }
    return nCnt;
}

const CharClass& SvNumberformat::rChrCls() const
{
    return rScan.GetChrCls();
}

const LocaleDataWrapper& SvNumberformat::rLoc() const
{
    return rScan.GetLoc();
}

CalendarWrapper& SvNumberformat::GetCal() const
{
    return rScan.GetCal();
}

const SvNumberFormatter& SvNumberformat::GetFormatter() const
{
    return *rScan.GetNumberformatter();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
