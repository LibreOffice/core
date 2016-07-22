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
#include <ctype.h>
#include <float.h>
#include <errno.h>
#include <stdlib.h>
#include <comphelper/string.hxx>
#include <sal/log.hxx>
#include <tools/debug.hxx>
#include <osl/diagnose.h>
#include <i18nlangtag/mslangid.hxx>
#include <rtl/math.hxx>
#include <unotools/charclass.hxx>
#include <unotools/calendarwrapper.hxx>
#include <unotools/nativenumberwrapper.hxx>
#include <com/sun/star/i18n/CalendarFieldIndex.hpp>
#include <com/sun/star/i18n/CalendarDisplayIndex.hpp>
#include <com/sun/star/i18n/CalendarDisplayCode.hpp>
#include <com/sun/star/i18n/AmPmValue.hpp>

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

const double D_MAX_U_INT32 = (double) 0xffffffff;      // 4294967295.0

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
            n = (int)cCharWidths[ c - 32 ];
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
        return (long) floor( log10( fAbsVal ) ) + 1;
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

void ImpSvNumberformatInfo::Copy( const ImpSvNumberformatInfo& rNumFor, sal_uInt16 nAnz )
{
    for (sal_uInt16 i = 0; i < nAnz; ++i)
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
    eLang &= 0x03FF;    // 10 bit primary language
    if ( bDate )
    {
        if ( nDBNum == 4 && eLang == (LANGUAGE_KOREAN & 0x03FF) )
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
            switch ( eLang )
            {
            case (LANGUAGE_CHINESE  & 0x03FF):
                nNatNum = 4;
                break;
            case (LANGUAGE_JAPANESE & 0x03FF):
                nNatNum = 1;
                break;
            case (LANGUAGE_KOREAN   & 0x03FF):
                nNatNum = 1;
                break;
            }
            break;
        case 2:
            switch ( eLang )
            {
            case (LANGUAGE_CHINESE  & 0x03FF):
                nNatNum = 5;
                break;
            case (LANGUAGE_JAPANESE & 0x03FF):
                nNatNum = 4;
                break;
            case (LANGUAGE_KOREAN   & 0x03FF):
                nNatNum = 2;
                break;
            }
            break;
        case 3:
            switch ( eLang )
            {
            case (LANGUAGE_CHINESE  & 0x03FF):
                nNatNum = 6;
                break;
            case (LANGUAGE_JAPANESE & 0x03FF):
                nNatNum = 5;
                break;
            case (LANGUAGE_KOREAN   & 0x03FF):
                nNatNum = 3;
                break;
            }
            break;
        case 4:
            switch ( eLang )
            {
            case (LANGUAGE_JAPANESE & 0x03FF):
                nNatNum = 7;
                break;
            case (LANGUAGE_KOREAN   & 0x03FF):
                nNatNum = 9;
                break;
            }
            break;
        }
    }
    return nNatNum;
}

#ifdef THE_FUTURE
/* XXX NOTE: even though the MapNatNumToDBNum method is currently unused please
 * don't remove it in case we'd have to use it for some obscure exports to
 * Excel. */

// static
sal_uInt8 SvNumberNatNum::MapNatNumToDBNum( sal_uInt8 nNatNum, LanguageType eLang, bool bDate )
{
    sal_uInt8 nDBNum = 0;
    eLang = MsLangId::getRealLanguage( eLang );  // resolve SYSTEM etc.
    eLang &= 0x03FF;    // 10 bit primary language
    if ( bDate )
    {
        if ( nNatNum == 9 && eLang == (LANGUAGE_KOREAN & 0x03FF) )
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
            switch ( eLang )
            {
            case (LANGUAGE_JAPANESE & 0x03FF):
                nDBNum = 1;
                break;
            case (LANGUAGE_KOREAN   & 0x03FF):
                nDBNum = 1;
                break;
            }
            break;
        case 2:
            switch ( eLang )
            {
            case (LANGUAGE_KOREAN   & 0x03FF):
                nDBNum = 2;
                break;
            }
            break;
        case 3:
            switch ( eLang )
            {
            case (LANGUAGE_KOREAN   & 0x03FF):
                nDBNum = 3;
                break;
            }
            break;
        case 4:
            switch ( eLang )
            {
            case (LANGUAGE_CHINESE  & 0x03FF):
                nDBNum = 1;
                break;
            case (LANGUAGE_JAPANESE & 0x03FF):
                nDBNum = 2;
                break;
            }
            break;
        case 5:
            switch ( eLang )
            {
            case (LANGUAGE_CHINESE  & 0x03FF):
                nDBNum = 2;
                break;
            case (LANGUAGE_JAPANESE & 0x03FF):
                nDBNum = 3;
                break;
            }
            break;
        case 6:
            switch ( eLang )
            {
            case (LANGUAGE_CHINESE  & 0x03FF):
                nDBNum = 3;
                break;
            }
            break;
        case 7:
            switch ( eLang )
            {
            case (LANGUAGE_JAPANESE & 0x03FF):
                nDBNum = 4;
                break;
            }
            break;
        case 8:
            break;
        case 9:
            switch ( eLang )
            {
            case (LANGUAGE_KOREAN   & 0x03FF):
                nDBNum = 4;
                break;
            }
            break;
        case 10:
            break;
        case 11:
            break;
        }
    }
    return nDBNum;
}
#endif

/**
 * SvNumFor
 */

ImpSvNumFor::ImpSvNumFor()
{
    nAnzStrings = 0;
    aI.nTypeArray = nullptr;
    aI.sStrArray = nullptr;
    aI.eScannedType = css::util::NumberFormat::UNDEFINED;
    aI.bThousand = false;
    aI.nThousand = 0;
    aI.nCntPre = 0;
    aI.nCntPost = 0;
    aI.nCntExp = 0;
    pColor = nullptr;
}

ImpSvNumFor::~ImpSvNumFor()
{
    delete [] aI.sStrArray;
    delete [] aI.nTypeArray;
}

void ImpSvNumFor::Enlarge(sal_uInt16 nAnz)
{
    if ( nAnzStrings != nAnz )
    {
        delete [] aI.nTypeArray;
        delete [] aI.sStrArray;
        nAnzStrings = nAnz;
        if ( nAnz )
        {
            aI.nTypeArray = new short[nAnz];
            aI.sStrArray  = new OUString[nAnz];
        }
        else
        {
            aI.nTypeArray = nullptr;
            aI.sStrArray  = nullptr;
        }
    }
}

void ImpSvNumFor::Copy( const ImpSvNumFor& rNumFor, ImpSvNumberformatScan* pSc )
{
    Enlarge( rNumFor.nAnzStrings );
    aI.Copy( rNumFor.aI, nAnzStrings );
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
    for ( sal_uInt16 j=0; j<nAnzStrings; j++ )
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
    for ( sal_uInt16 j=0; j<nAnzStrings; j++ )
    {
        if ( aI.nTypeArray[j] == NF_SYMBOLTYPE_CURRENCY )
        {
            rSymbol = aI.sStrArray[j];
            if ( j < nAnzStrings-1 && aI.nTypeArray[j+1] == NF_SYMBOLTYPE_CURREXT )
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

SvNumberformat::SvNumberformat( SvNumberformat& rFormat )
    : rScan(rFormat.rScan), bStarFlag( rFormat.bStarFlag )
{
    ImpCopyNumberformat( rFormat );
}

SvNumberformat::SvNumberformat( SvNumberformat& rFormat, ImpSvNumberformatScan& rSc )
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


OUString SvNumberformat::ImpObtainCalendarAndNumerals( OUStringBuffer & rString, sal_Int32 & nPos,
                                                       LanguageType & nLang, const LocaleType & aTmpLocale )
{
    OUString sCalendar;
    /* TODO: this could be enhanced to allow other possible locale dependent
     * calendars and numerals. BUT only if our locale data allows it! For LCID
     * numerals and calendars see
     * http://office.microsoft.com/en-us/excel/HA010346351033.aspx */
    if (MsLangId::getRealLanguage( aTmpLocale.meLanguage) == LANGUAGE_THAI)
    {
        // Numeral shape code "D" = Thai digits.
        if (aTmpLocale.mnNumeralShape == 0xD)
        {
            rString.insert( nPos, "[NatNum1]");
        }
        // Calendar type code "07" = Thai Buddhist calendar, insert this after
        // all prefixes have been consumed as it is actually a format modifier
        // and not a prefix.
        if (aTmpLocale.mnCalendarType == 0x07)
        {
            // Currently calendars are tied to the locale of the entire number
            // format, e.g. [~buddhist] in en_US doesn't work.
            // => Having different locales in sub formats does not work!
            /* TODO: calendars could be tied to a sub format's NatNum info
             * instead, or even better be available for any locale. Needs a
             * different implementation of GetCal() and locale data calendars.
             * */
            // If this is not Thai yet, make it so.
            if (MsLangId::getRealLanguage( maLocale.meLanguage) != LANGUAGE_THAI)
            {
                maLocale = aTmpLocale;
                nLang = maLocale.meLanguage = LANGUAGE_THAI;
            }
            sCalendar="[~buddhist]";
        }
    }
    return sCalendar;
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
    eType = css::util::NumberFormat::DEFINED;

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
            (rScan.GetNumberformatter())->ChangeIntl(rScan.GetTmpLnge());
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
                    eOp1 = (SvNumberformatLimitOps) eSymbolType;
                }
                else if ( nIndex == 1 && bCondition )
                {
                    eOp2 = (SvNumberformatLimitOps) eSymbolType;
                }
                else                                // error
                {
                    bCancel = true;                 // break for
                    nCheckPos = nPosOld;
                }
                if (!bCancel)
                {
                    double fNumber;
                    sal_Int32 nAnzChars = ImpGetNumber(sBuff, nPos, sStr);
                    if (nAnzChars > 0)
                    {
                        short F_Type = css::util::NumberFormat::UNDEFINED;
                        if (!pISc->IsNumberFormat(sStr,F_Type,fNumber) ||
                            ( F_Type != css::util::NumberFormat::NUMBER &&
                              F_Type != css::util::NumberFormat::SCIENTIFIC) )
                        {
                            fNumber = 0.0;
                            nPos = nPos - nAnzChars;
                            sBuff.remove(nPos, nAnzChars);
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
                        sStr = "NatNum";
                        //! eSymbolType is negative
                        sal_uInt8 nNum = (sal_uInt8)(0 - (eSymbolType - BRACKET_SYMBOLTYPE_NATNUM0));
                        sStr += OUString::number( nNum );
                        NumFor[nIndex].SetNatNumNum( nNum, false );
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
                        sStr = "DBNum";
                        //! eSymbolType is negative
                        sal_uInt8 nNum = (sal_uInt8)(1 - (eSymbolType - BRACKET_SYMBOLTYPE_DBNUM1));
                        sStr += OUString((sal_Unicode)('0' + nNum));
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
                            if (nIndex == 0 && (aTmpLocale.meLanguage == 0 ||
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
                            }
                            sStr = "$-" + aTmpLocale.generateCode();
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
                    NumFor[nIndex].Info().eScannedType = css::util::NumberFormat::EMPTY;
                }
                else
                {
                    if (!sInsertCalendar.isEmpty())
                    {
                        sStr = sInsertCalendar + sStr;
                    }
                    sal_Int32 nStrPos = pSc->ScanFormat( sStr);
                    sal_uInt16 nAnz = pSc->GetAnzResStrings();
                    if (nAnz == 0)              // error
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
                            sStr = "[$-" + OUString::number( eLanguage, 16 ).toAsciiUpperCase() + "]" + sStr;
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
                            if (pSc->GetScannedType() == css::util::NumberFormat::TEXT)
                            {
                                // Reset conditions, reverting any set above.
                                if (nIndex == 1)
                                    eOp1 = NUMBERFORMAT_OP_NO;
                                else if (nIndex == 2)
                                    eOp2 = NUMBERFORMAT_OP_NO;
                                nIndex = 3;
                            }
                        }
                        NumFor[nIndex].Enlarge(nAnz);
                        pSc->CopyInfo(&(NumFor[nIndex].Info()), nAnz);
                        // type check
                        if (nIndex == 0)
                        {
                            eType = (short) NumFor[nIndex].Info().eScannedType;
                        }
                        else if (nIndex == 3)
                        {   // #77026# Everything recognized IS text
                            NumFor[nIndex].Info().eScannedType = css::util::NumberFormat::TEXT;
                        }
                        else if ( (short) NumFor[nIndex].Info().eScannedType != eType)
                        {
                            eType = css::util::NumberFormat::DEFINED;
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
            if ( nIndex == 2 && eSymbolType == BRACKET_SYMBOLTYPE_FORMAT &&
                 sBuff[nPos - 1] == ';' )
            {
                // #83510# A 4th subformat explicitly specified to be empty
                // hides any text. Need the type here for HasTextFormat()
                NumFor[3].Info().eScannedType = css::util::NumberFormat::TEXT;
            }
            bCancel = true;
        }
        if ( NumFor[nIndex].GetNatNum().IsSet() )
        {
            NumFor[nIndex].SetNatNumDate( (NumFor[nIndex].Info().eScannedType & css::util::NumberFormat::DATE) != 0 );
        }
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
                sal_uInt16 nAnz = pSc->GetAnzResStrings();
                if ( nAnz )
                {
                    NumFor[0].Enlarge(nAnz);
                    pSc->CopyInfo( &(NumFor[0].Info()), nAnz );
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
                sal_uInt16 nAnz = pSc->GetAnzResStrings();
                if ( nAnz )
                {
                    NumFor[nIndex].Enlarge(nAnz);
                    pSc->CopyInfo( &(NumFor[nIndex].Info()), nAnz );
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
                sal_uInt16 nAnz = pSc->GetAnzResStrings();
                if ( nAnz )
                {
                    NumFor[nIndex].Enlarge(nAnz);
                    pSc->CopyInfo( &(NumFor[nIndex].Info()), nAnz );
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

    sal_uInt16 n16 = static_cast<sal_uInt16>(meLanguage);
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
    : mnNumeralShape(0)
    , mnCalendarType(0)
    , meLanguage(LANGUAGE_DONTKNOW)
{
}

SvNumberformat::LocaleType::LocaleType(sal_uInt32 nRawNum)
    : mnNumeralShape(0)
    , mnCalendarType(0)
    , meLanguage(LANGUAGE_DONTKNOW)
{
    meLanguage = static_cast<LanguageType>(nRawNum & 0x0000FFFF);
    nRawNum = (nRawNum >> 16);
    mnCalendarType = static_cast<sal_uInt8>(nRawNum & 0xFF);
    nRawNum = (nRawNum >> 8);
    mnNumeralShape = static_cast<sal_uInt8>(nRawNum & 0xFF);
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
            return LANGUAGE_DONTKNOW;
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
                                    OUString& sSymbol)
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
                if ( rString[nPos] == '-' )
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
                    sBuffSymbol.append( aBufStr.copy( --nPos, aNatNum.getLength()+1 ));
                    nPos += aNatNum.getLength()+1;
                    //! SymbolType is negative
                    eSymbolType = (short) (BRACKET_SYMBOLTYPE_NATNUM0 - nNatNumNum);
                    eState = SsGetPrefix;
                }
                else if ( lcl_matchKeywordAndGetNumber( aBufStr, nPos-1, aDBNum, nDBNum) &&
                        '1' <= nDBNum && nDBNum <= '9' )
                {
                    sBuffSymbol.stripStart('[');
                    sBuffSymbol.append( aBufStr.copy( --nPos, aDBNum.getLength()+1 ));
                    nPos += aDBNum.getLength()+1;
                    //! SymbolType is negative
                    eSymbolType = sal::static_int_cast< short >( BRACKET_SYMBOLTYPE_DBNUM1 - (nDBNum - '1'));
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
    short nType = eType;
    OUString aFormatString( sFormatstring );
    rConverter.PutandConvertEntry( aFormatString, nCheckPos, nType,
                                   nKey, eConvertFrom, eConvertTo );
    const SvNumberformat* pFormat = rConverter.GetEntry( nKey );
    DBG_ASSERT( pFormat, "SvNumberformat::ConvertLanguage: Conversion ohne Format" );
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
OUString SvNumberformat::StripNewCurrencyDelimiters( const OUString& rStr,
                                                     bool bQuoteSymbol )
{
    OUString aTmp;
    sal_Int32 nStartPos, nPos, nLen;
    nLen = rStr.getLength();
    nStartPos = 0;
    while ( (nPos = rStr.indexOf( "[$", nStartPos )) >= 0 )
    {
        sal_Int32 nEnd;
        if ( (nEnd = GetQuoteEnd( rStr, nPos )) >= 0 )
        {
            aTmp += rStr.copy( nStartPos, ++nEnd - nStartPos );
            nStartPos = nEnd;
        }
        else
        {
            aTmp += rStr.copy( nStartPos, nPos - nStartPos );
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
            if ( !bQuoteSymbol || rStr[ nStartPos ] == '"' )
            {
                aTmp += rStr.copy( nStartPos, nPos - nStartPos );
            }
            else
            {
                aTmp += "\"";
                aTmp += rStr.copy( nStartPos, nPos - nStartPos );
                aTmp += "\"";
            }
            nStartPos = nClose + 1;
        }
    }
    if ( nLen > nStartPos )
    {
        aTmp += rStr.copy( nStartPos, nLen - nStartPos );
    }
    return aTmp;
}

void SvNumberformat::ImpGetOutputStandard(double& fNumber, OUStringBuffer& rOutString)
{
    OUString sTemp;
    ImpGetOutputStandard(fNumber, sTemp);
    rOutString = sTemp;
}

void SvNumberformat::ImpGetOutputStandard(double& fNumber, OUString& rOutString)
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

void SvNumberformat::ImpGetOutputStdToPrecision(double& rNumber, OUString& rOutString, sal_uInt16 nPrecision) const
{
    // Make sure the precision doesn't go over the maximum allowable precision.
    nPrecision = ::std::min(UPPER_PRECISION, nPrecision);

#if 0
{
    // debugger test case for ANSI standard correctness
    OUString aTest;
    // expect 0.00123   OK
    aTest = ::rtl::math::doubleToUString( 0.001234567,
                                          rtl_math_StringFormat_G, 3, '.', true );
    // expect 123       OK
    aTest = ::rtl::math::doubleToUString( 123.4567,
                                          rtl_math_StringFormat_G, 3, '.', true );
    // expect 123.5     OK
    aTest = ::rtl::math::doubleToUString( 123.4567,
                                          rtl_math_StringFormat_G, 4, '.', true );
    // expect 1e+03 (as 999.6 rounded to 3 significant digits results in
    // 1000 with an exponent equal to significant digits)
    // Currently (24-Jan-2003) we do fail in this case and output 1000
    // instead, negligible.
    aTest = ::rtl::math::doubleToUString( 999.6,
                                          rtl_math_StringFormat_G, 3, '.', true );
    // expect what? result is 1.2e+004
    aTest = ::rtl::math::doubleToUString( 12345.6789,
                                          rtl_math_StringFormat_G, -3, '.', true );
}
#endif

    // We decided to strip trailing zeros unconditionally, since binary
    // double-precision rounding error makes it impossible to determine e.g.
    // whether 844.10000000000002273737 is what the user has typed, or the
    // user has typed 844.1 but IEEE 754 represents it that way internally.

    rOutString = ::rtl::math::doubleToUString( rNumber,
                                               rtl_math_StringFormat_F, nPrecision /*2*/,
                                               GetFormatter().GetNumDecimalSep()[0], true );
    if (rOutString[0] == '-' &&
        comphelper::string::getTokenCount(rOutString, '0') == rOutString.getLength())
    {
        rOutString = comphelper::string::stripStart(rOutString, '-'); // not -0
    }
    rOutString = impTransliterate(rOutString, NumFor[0].GetNatNum());
}

void SvNumberformat::ImpGetOutputInputLine(double fNumber, OUString& OutString)
{
    bool bModified = false;
    if ( (eType & css::util::NumberFormat::PERCENT) && (fabs(fNumber) < D_MAX_D_BY_100))
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

    if ( eType & css::util::NumberFormat::PERCENT && bModified)
    {
        OutString += "%";
    }
    return;
}

short SvNumberformat::ImpCheckCondition(double& fNumber,
                                        double& fLimit,
                                        SvNumberformatLimitOps eOp)
{
    switch(eOp)
    {
    case NUMBERFORMAT_OP_NO:
        return -1;
    case NUMBERFORMAT_OP_EQ:
        return (short) (fNumber == fLimit);
    case NUMBERFORMAT_OP_NE:
        return (short) (fNumber != fLimit);
    case NUMBERFORMAT_OP_LT:
        return (short) (fNumber <  fLimit);
    case NUMBERFORMAT_OP_LE:
        return (short) (fNumber <= fLimit);
    case NUMBERFORMAT_OP_GT:
        return (short) (fNumber >  fLimit);
    case NUMBERFORMAT_OP_GE:
        return (short) (fNumber >= fLimit);
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
        rBuf.append((sal_Unicode) 0x1B);
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
        rBuf.insert( nPos, (sal_Unicode) 0x1B);
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
    if (eType & css::util::NumberFormat::TEXT)
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
    if (rInfo.eScannedType == css::util::NumberFormat::TEXT)
    {
        const sal_uInt16 nAnz = NumFor[nIx].GetCount();
        for (sal_uInt16 i = 0; i < nAnz; i++)
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

OUString lcl_GetDenominatorString(const ImpSvNumberformatInfo &rInfo, sal_uInt16 nAnz)
{
    sal_uInt16 i;
    OUStringBuffer aDenominatorString;
    for( i = 0; i < nAnz; i++ )
    {
        if( rInfo.nTypeArray[i] == NF_SYMBOLTYPE_FRAC )
        {
            for( i++; i < nAnz; i++ )
            {
                if( rInfo.nTypeArray[i] == NF_SYMBOLTYPE_FRAC_FDIV || rInfo.nTypeArray[i] == NF_SYMBOLTYPE_DIGIT )
                    aDenominatorString.append( rInfo.sStrArray[i] );
                else
                    i = nAnz;
            }
        }
    }
    return aDenominatorString.makeStringAndClear();
}

OUString lcl_GetNumeratorString(const ImpSvNumberformatInfo &rInfo, sal_uInt16 nAnz)
{
    sal_Int16 i;
    OUStringBuffer aNumeratorString;
    for( i = 0; i < nAnz; i++ )
    {
        if( rInfo.nTypeArray[i] == NF_SYMBOLTYPE_FRAC )
        {
            for( i--; i >= 0 && rInfo.nTypeArray[i] == NF_SYMBOLTYPE_DIGIT ; i-- )
            {
                aNumeratorString.insert( 0, rInfo.sStrArray[i] );
            }
            i = nAnz;
        }
    }
    return aNumeratorString.makeStringAndClear();
}

OUString lcl_GetFractionIntegerString(const ImpSvNumberformatInfo &rInfo, sal_uInt16 nAnz)
{
    sal_Int16 i;
    OUStringBuffer aIntegerString;
    for( i = 0; i < nAnz; i++ )
    {
        if( rInfo.nTypeArray[i] == NF_SYMBOLTYPE_FRACBLANK )
        {
            for( i--; i >= 0 && rInfo.nTypeArray[i] == NF_SYMBOLTYPE_DIGIT ; i-- )
            {
                aIntegerString.insert( 0, rInfo.sStrArray[i] );
            }
            i = nAnz;
        }
    }
    return aIntegerString.makeStringAndClear();
}

}

OUString SvNumberformat::GetDenominatorString( sal_uInt16 nNumFor ) const
{
    const ImpSvNumberformatInfo& rInfo = NumFor[nNumFor].Info();
    sal_uInt16 nAnz = NumFor[nNumFor].GetCount();
    return lcl_GetDenominatorString( rInfo, nAnz );
}

OUString SvNumberformat::GetNumeratorString( sal_uInt16 nNumFor ) const
{
    const ImpSvNumberformatInfo& rInfo = NumFor[nNumFor].Info();
    sal_uInt16 nAnz = NumFor[nNumFor].GetCount();
    return lcl_GetNumeratorString( rInfo, nAnz );
}

bool SvNumberformat::GetOutputString(double fNumber, sal_uInt16 nCharCount, OUString& rOutString) const
{
    using namespace std;

    if (eType != css::util::NumberFormat::NUMBER)
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

bool SvNumberformat::GetOutputString(double fNumber,
                                     OUString& OutString,
                                     Color** ppColor)
{
    bool bRes = false;
    OUStringBuffer sBuff;
    OutString.clear();
    *ppColor = nullptr; // No color change
    if (eType & css::util::NumberFormat::LOGICAL)
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
    if (eType & css::util::NumberFormat::TEXT)
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
        case css::util::NumberFormat::NUMBER: // Standard number format
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
        case css::util::NumberFormat::DATE:
            bRes |= ImpGetDateOutput(fNumber, 0, sBuff);
            bHadStandard = true;
            break;
        case css::util::NumberFormat::TIME:
            bRes |= ImpGetTimeOutput(fNumber, 0, sBuff);
            bHadStandard = true;
            break;
        case css::util::NumberFormat::DATETIME:
            bRes |= ImpGetDateTimeOutput(fNumber, 0, sBuff);
            bHadStandard = true;
            break;
        }
    }
    if ( !bHadStandard )
    {
        sal_uInt16 nIx; // Index of the partial format
        short nCheck = ImpCheckCondition(fNumber, fLimit1, eOp1);
        if (nCheck == -1 || nCheck == 1) // Only 1 String or True
        {
            nIx = 0;
        }
        else
        {
            nCheck = ImpCheckCondition(fNumber, fLimit2, eOp2);
            if (nCheck == -1 || nCheck == 1)
            {
                nIx = 1;
            }
            else
            {
                nIx = 2;
            }
        }
        if (fNumber < 0.0 &&
                ((nIx == 0 && IsFirstSubformatRealNegative()) || // 1st, usually positive subformat
                 (nIx == 1 && IsSecondSubformatRealNegative()))) // 2nd, usually negative subformat
        {
            fNumber = -fNumber; // eliminate sign
        }
        *ppColor = NumFor[nIx].GetColor();
        const ImpSvNumberformatInfo& rInfo = NumFor[nIx].Info();
        const sal_uInt16 nAnz = NumFor[nIx].GetCount();
        if (nAnz == 0 && rInfo.eScannedType == css::util::NumberFormat::EMPTY)
        {
            return false; // Empty => nothing
        }
        else if (nAnz == 0) // Else Standard Format
        {
            ImpGetOutputStandard(fNumber, sBuff);
            OutString = sBuff.makeStringAndClear();
            return false;
        }
        switch (rInfo.eScannedType)
        {
        case css::util::NumberFormat::TEXT:
        case css::util::NumberFormat::DEFINED:
            for (sal_uInt16 i = 0; i < nAnz; i++)
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
        case css::util::NumberFormat::DATE:
            bRes |= ImpGetDateOutput(fNumber, nIx, sBuff);
            break;
        case css::util::NumberFormat::TIME:
            bRes |= ImpGetTimeOutput(fNumber, nIx, sBuff);
                break;
        case css::util::NumberFormat::DATETIME:
            bRes |= ImpGetDateTimeOutput(fNumber, nIx, sBuff);
            break;
        case css::util::NumberFormat::NUMBER:
        case css::util::NumberFormat::PERCENT:
        case css::util::NumberFormat::CURRENCY:
            bRes |= ImpGetNumberOutput(fNumber, nIx, sBuff);
            break;
        case css::util::NumberFormat::FRACTION:
            bRes |= ImpGetFractionOutput(fNumber, nIx, sBuff);
            break;
        case css::util::NumberFormat::SCIENTIFIC:
            bRes |= ImpGetScientificOutput(fNumber, nIx, sBuff);
            break;
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
    const sal_uInt16 nAnz = NumFor[nIx].GetCount();

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

    if ( nExPos >= 0 )
    {
        // split into mantisse and exponent and get rid of "E+" or "E-"
        sal_Int32 nExpStart = nExPos + 1;

        switch ( sStr[ nExpStart ] )
        {
        case '-' :
            nExpSign = -1;
            SAL_FALLTHROUGH;
        case '+' :
            ++nExpStart;
            break;
        }
        ExpStr = sStr.toString().copy( nExpStart );    // part following the "E+"
        sStr.truncate( nExPos );

        if ( rInfo.nCntPre != 1 ) // rescale Exp
        {
            sal_Int32 nExp = ExpStr.toString().toInt32() * nExpSign;
            sal_Int32 nRescale = (rInfo.nCntPre != 0) ? nExp % (sal_Int32)rInfo.nCntPre : -1;
            if( nRescale < 0 && rInfo.nCntPre != 0 )
                nRescale += (sal_Int32)rInfo.nCntPre;
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
            // rescale mantissa
            sStr = ::rtl::math::doubleToUString( fNumber,
                                         rtl_math_StringFormat_E,
                                         nRescale + rInfo.nCntPost, '.' );
            sStr.truncate( sStr.indexOf('E') );
        }

        // cut any decimal delimiter
        sal_Int32 index = 0;

        while((index = sStr.indexOf('.', index)) >= 0)
        {
            sStr.remove(index, 1);
        }
    }

    sal_uInt16 j = nAnz-1;  // Last symbol
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
        bRes |= ImpDecimalFill(sStr, fNumber, j, nIx, false);
    }

    if (bSign)
    {
        sStr.insert(0, '-');
    }
    sStr.append(ExpStr);

    return bRes;
}

bool SvNumberformat::ImpGetFractionOutput(double fNumber,
                                          sal_uInt16 nIx,
                                          OUStringBuffer& sBuff)
{
    bool bRes = false;
    const ImpSvNumberformatInfo& rInfo = NumFor[nIx].Info();
    const sal_uInt16 nAnz = NumFor[nIx].GetCount();
    OUStringBuffer sStr, sFrac, sDiv; // Strings, value for
    sal_uInt64 nFrac=0, nDiv=1;       // Integral part
    bool bSign = false;               // Numerator and denominator
    const OUString sIntegerFormat = lcl_GetFractionIntegerString(rInfo, nAnz);
    const OUString sNumeratorFormat = lcl_GetNumeratorString(rInfo, nAnz);
    const OUString sDenominatorFormat = lcl_GetDenominatorString(rInfo, nAnz);

    if (fNumber < 0)
    {
        if (nIx == 0) // Not in the ones at the end
            bSign = true; // Formats
        fNumber = -fNumber;
    }

    double fNum = floor(fNumber); // Integral part

    fNumber -= fNum; // Fractional part
    if (fNum > D_MAX_U_INT32 || rInfo.nCntExp > 9) // Too large
    {
        sBuff = rScan.GetErrorString();
        return false;
    }
    if (rInfo.nCntExp == 0)
    {
        SAL_WARN( "svl.numbers", "SvNumberformat:: Fraction, nCntExp == 0");
        sBuff.truncate();
        return false;
    }

    if( sal_Int32 nForcedDiv = sDenominatorFormat.toInt32() )
    {   // Forced Denominator
        nDiv = (sal_uInt64) nForcedDiv;
        nFrac = (sal_uInt64)floor ( fNumber * nDiv );
        double fFracNew = (double)nFrac / (double)nDiv;
        double fFracNew1 = (double)(nFrac + 1) / (double)nDiv;
        double fDiff = fNumber - fFracNew;
        if( fDiff > ( fFracNew1 - fNumber ) )
        {
            nFrac++;
        }
        if( nFrac >= nDiv )
        {
            nFrac = nDiv = 0;
            fNum = fNum + 1.0;
        }
    }
    else // Calculated Denominator
    {
        sal_uInt64 nBasis = ((sal_uInt64)floor( pow(10.0,rInfo.nCntExp))) - 1; // 9, 99, 999 ,...
        sal_uInt64 nFracPrev = 1L, nDivPrev = 0, nFracNext, nDivNext, nPartialDenom;
        double fRemainder = fNumber, fTemp;

        // Use continued fraction representation of fNumber
        // See https://en.wikipedia.org/wiki/Continued_fraction#Best_rational_approximations
        while ( fRemainder > 0.0 )
        {
            fTemp = 1.0 / fRemainder;                    // 64bits precision required when fRemainder is very weak
            nPartialDenom = (sal_uInt64) floor(fTemp);   // due to floating point notation with double precision
            fRemainder = fTemp - (double)nPartialDenom;
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
                    double fSign = ((double)nFrac > fNumber * (double)nDiv)?1.0:-1.0;
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

    if (rInfo.nCntPre == 0) // Improper fraction
    {
        double fNum1 = fNum * (double)nDiv + (double)nFrac;

        if (fNum1 > D_MAX_U_INT32)
        {
            sBuff = rScan.GetErrorString();
            return false;
        }
        nFrac = (sal_uInt64) floor(fNum1);
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

    sal_uInt16 j = nAnz-1; // Last symbol -> backwards
    sal_Int32 k;           // Denominator

    bRes |= ImpNumberFill(sDiv, fNumber, k, j, nIx, NF_SYMBOLTYPE_FRAC);

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

bool SvNumberformat::ImpGetTimeOutput(double fNumber,
                                      sal_uInt16 nIx,
                                      OUStringBuffer& sBuff)
{
    using namespace ::com::sun::star::i18n;
    bool bCalendarSet = false;
    double fNumberOrig = fNumber;
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
            sBuff = rScan.GetErrorString();
            return false;
        }
    }
    else
    {
        fNumber -= floor(fNumber); // Else truncate date
    }
    bool bInputLine;
    sal_Int32 nCntPost;
    if ( rScan.GetStandardPrec() == 300 &&
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
    if (bSign && !rInfo.bThousand) // No [] format
    {
        fNumber = 1.0 - fNumber; // "Inverse"
    }
    double fTime = fNumber * 86400.0;
    fTime = ::rtl::math::round( fTime, int(nCntPost) );
    if (bSign && fTime == 0.0)
    {
        bSign = false; // Not -00:00:00
    }
    if( floor( fTime ) > D_MAX_U_INT32 )
    {
        sBuff = rScan.GetErrorString();
        return false;
    }
    sal_uInt32 nSeconds = (sal_uInt32)floor( fTime );

    OUStringBuffer sSecStr( ::rtl::math::doubleToUString( fTime-nSeconds,
                                                          rtl_math_StringFormat_F, int(nCntPost), '.'));
    sSecStr.stripStart('0');
    sSecStr.stripStart('.');
    if ( bInputLine )
    {
        sSecStr.stripEnd('0');
        for(sal_Int32 index = sSecStr.getLength(); index < rInfo.nCntPost; ++index)
        {
            sSecStr.append('0');
        }
        impTransliterate(sSecStr, NumFor[nIx].GetNatNum());
        nCntPost = sSecStr.getLength();
    }
    else
    {
        impTransliterate(sSecStr, NumFor[nIx].GetNatNum());
    }

    sal_Int32 nSecPos = 0; // For figure by figure processing
    sal_uInt32 nHour, nMin, nSec;
    if (!rInfo.bThousand) // No [] format
    {
        nHour = (nSeconds/3600) % 24;
        nMin = (nSeconds%3600) / 60;
        nSec = nSeconds%60;
    }
    else if (rInfo.nThousand == 3) // [ss]
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
    const sal_uInt16 nAnz = NumFor[nIx].GetCount();
    for (sal_uInt16 i = 0; i < nAnz; i++)
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
            for (sal_Int32 j = 0; j < nLen && nSecPos < nCntPost; j++)
            {
                sBuff.append(sSecStr[nSecPos]);
                nSecPos++;
            }
            break;
        case NF_KEY_AMPM:               // AM/PM
            if ( !bCalendarSet )
            {
                double fDiff = DateTime(*(rScan.GetNullDate())) - GetCal().getEpochStart();
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

sal_Int32 SvNumberformat::ImpUseMonthCase( int & io_nState, const ImpSvNumFor& rNumFor, NfKeywordIndex eCodeType ) const
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
    const sal_uInt16 nAnz = rNumFor.GetCount();
    sal_uInt16 i;
    for ( i = 0; i < nAnz; i++ )
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
            return true;
        }
    }
    return false;
}

void SvNumberformat::SwitchToOtherCalendar( OUString& rOrgCalendar,
                                            double& fOrgDateTime ) const
{
    CalendarWrapper& rCal = GetCal();
    if ( rCal.getUniqueID() == GREGORIAN )
    {
        using namespace ::com::sun::star::i18n;
        css::uno::Sequence< OUString > xCals = rCal.getAllCalendars(
                rLoc().getLanguageTag().getLocale() );
        sal_Int32 nCnt = xCals.getLength();
        if ( nCnt > 1 )
        {
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
    const sal_uInt16 nAnz = rNumFor.GetCount();
    for ( sal_uInt16 i = 0; i < nAnz; i++ )
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

bool SvNumberformat::ImpIsIso8601( const ImpSvNumFor& rNumFor )
{
    bool bIsIso = false;
    if ((eType & css::util::NumberFormat::DATE) == css::util::NumberFormat::DATE)
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
        short const * const pType = rNumFor.Info().nTypeArray;
        sal_uInt16 nAnz = rNumFor.GetCount();
        for (sal_uInt16 i=0; i < nAnz && !bIsIso && eState != eNotIso; ++i)
        {
            switch ( pType[i] )
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

bool SvNumberformat::ImpGetDateOutput(double fNumber,
                                      sal_uInt16 nIx,
                                      OUStringBuffer& sBuff)
{
    using namespace ::com::sun::star::i18n;
    bool bRes = false;

    CalendarWrapper& rCal = GetCal();
    double fDiff = DateTime(*(rScan.GetNullDate())) - rCal.getEpochStart();
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
    const sal_uInt16 nAnz = NumFor[nIx].GetCount();
    sal_Int16 nNatNum = NumFor[nIx].GetNatNum().GetNatNum();
    OUString aYear;

    for (sal_uInt16 i = 0; i < nAnz; i++)
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
            sBuff.append(rCal.getDisplayString( CalendarDisplayCode::SHORT_MONTH, nNatNum ));
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
    double fDiff = DateTime(*(rScan.GetNullDate())) - rCal.getEpochStart();
    fNumber += fDiff;

    const ImpSvNumberformatInfo& rInfo = NumFor[nIx].Info();
    bool bInputLine;
    sal_Int32 nCntPost;
    if ( rScan.GetStandardPrec() == 300 &&
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

    sal_uInt32 nSeconds = (sal_uInt32)floor( fTime );
    OUStringBuffer sSecStr( ::rtl::math::doubleToUString( fTime-nSeconds,
                                                  rtl_math_StringFormat_F, int(nCntPost), '.'));
    sSecStr.stripStart('0');
    sSecStr.stripStart('.');
    if ( bInputLine )
    {
        sSecStr.stripEnd('0');
        for(sal_Int32 index = sSecStr.getLength(); index < rInfo.nCntPost; ++index)
        {
            sSecStr.append('0');
        }
        impTransliterate(sSecStr, NumFor[nIx].GetNatNum());
        nCntPost = sSecStr.getLength();
    }
    else
    {
        impTransliterate(sSecStr, NumFor[nIx].GetNatNum());
    }

    sal_Int32 nSecPos = 0; // For figure by figure processing
    sal_uInt32 nHour, nMin, nSec;
    if (!rInfo.bThousand) // [] format
    {
        nHour = (nSeconds/3600) % 24;
        nMin = (nSeconds%3600) / 60;
        nSec = nSeconds%60;
    }
    else if (rInfo.nThousand == 3) // [ss]
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
    const sal_uInt16 nAnz = NumFor[nIx].GetCount();
    sal_Int32 nLen;
    OUString aYear;
    for (sal_uInt16 i = 0; i < nAnz; i++)
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
            for (sal_Int32 j = 0; j < nLen && nSecPos < nCntPost; j++)
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
        if (nIx == 0) // Not in the ones at the back
        {
            bSign = true; // Formats
        }
        else
        {
            bSign = false;
        }
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
    if (rInfo.eScannedType == css::util::NumberFormat::PERCENT)
    {
        if (fNumber < D_MAX_D_BY_100)
        {
            fNumber *= 100.0;
        }
        else
        {
            sStr = rScan.GetErrorString();
            return false;
        }
    }
    sal_uInt16 i, j;
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
                for (long l = 15-nPrecExp; l < (long) rInfo.nCntPost; l++)
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
        sal_Int32 nPoint = sStr.indexOf('.' );
        if ( nPoint >= 0)
        {
            const sal_Unicode* p = sStr.getStr() + nPoint;
            while ( *++p == '0' )
                ;
            if ( !*p )
            {
                bInteger = true;
            }
            sStr.remove( nPoint, 1 ); //  Remove .
        }
        if (bSign && (sStr.isEmpty() ||
                      comphelper::string::getTokenCount(sStr.toString(), '0') == sStr.getLength()+1))   // Only 00000
        {
            bSign = false;              // Not -0.00
        }
    }                                   // End of != FLAG_STANDARD_IN_FORMAT

                                        // Edit backwards:
    j = NumFor[nIx].GetCount()-1;       // Last symbol
                                        // Decimal places:
    bRes |= ImpDecimalFill( sStr, fNumber, j, nIx, bInteger );
    if (bSign)
    {
        sStr.insert(0, '-');
    }
    impTransliterate(sStr, NumFor[nIx].GetNatNum());
    return bRes;
}

bool SvNumberformat::ImpDecimalFill( OUStringBuffer& sStr,  // number string
                                   double& rNumber,       // number
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
            SAL_FALLTHROUGH;
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
                        // erAck: 2008-04-03T16:24+0200
                        // Actually this currently isn't executed
                        // because the format scanner in the context of
                        // "?," doesn't generate a group separator but
                        // a literal ',' character instead that is
                        // inserted unconditionally. Should be changed
                        // on some occasion.
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
                                    short eSymbolType )    // type of stop condition
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
                        sBuff.insert(0, ' ');
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
                                          sal_uInt16& nAnzLeading) const
{
    // as before: take info from nNumFor=0 for whole format (for dialog etc.)

    short nDummyType;
    GetNumForInfo( 0, nDummyType, bThousand, nPrecision, nAnzLeading );

    // "negative in red" is only useful for the whole format

    const Color* pColor = NumFor[1].GetColor();
    if (fLimit1 == 0.0 && fLimit2 == 0.0 && pColor
        && (*pColor == rScan.GetRedColor()))
    {
        IsRed = true;
    }
    else
    {
        IsRed = false;
    }
}

void SvNumberformat::GetNumForInfo( sal_uInt16 nNumFor, short& rScannedType,
                    bool& bThousand, sal_uInt16& nPrecision, sal_uInt16& nAnzLeading ) const
{
    // take info from a specified sub-format (for XML export)

    if ( nNumFor > 3 )
    {
        return; // invalid
    }

    const ImpSvNumberformatInfo& rInfo = NumFor[nNumFor].Info();
    rScannedType = rInfo.eScannedType;
    bThousand = rInfo.bThousand;
    nPrecision = rInfo.nCntPost;
    if (bStandard && rInfo.eScannedType == css::util::NumberFormat::NUMBER)
    {
        // StandardFormat
        nAnzLeading = 1;
    }
    else
    {
        nAnzLeading = 0;
        bool bStop = false;
        sal_uInt16 i = 0;
        const sal_uInt16 nAnz = NumFor[nNumFor].GetCount();
        while (!bStop && i < nAnz)
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
                    nAnzLeading++;
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
    sal_uInt16 nAnz = NumFor[nNumFor].GetCount();
    if ( !nAnz )
    {
        return nullptr;
    }
    if ( nPos == 0xFFFF )
    {
        nPos = nAnz - 1;
        if ( bString )
        {   // Backwards
            short* pType = NumFor[nNumFor].Info().nTypeArray + nPos;
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
    else if ( nPos > nAnz - 1 )
    {
        return nullptr;
    }
    else if ( bString )
    {
        // vorwaerts
        short* pType = NumFor[nNumFor].Info().nTypeArray + nPos;
        while ( nPos < nAnz && (*pType != NF_SYMBOLTYPE_STRING) &&
                (*pType != NF_SYMBOLTYPE_CURRENCY) )
        {
            pType++;
            nPos++;
        }
        if ( nPos >= nAnz || ((*pType != NF_SYMBOLTYPE_STRING) &&
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
    sal_uInt16 nAnz = NumFor[nNumFor].GetCount();
    if ( !nAnz )
    {
        return 0;
    }
    if ( nPos == 0xFFFF )
    {
        nPos = nAnz - 1;
    }
    else if ( nPos > nAnz - 1 )
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
    sal_uInt16 nAnz = NumFor[1].GetCount();
    if (!nAnz)
    {
        return false;
    }
    OUString *tmpStr = NumFor[1].Info().sStrArray;
    return tmpStr[0] == "(" && tmpStr[nAnz-1] == ")";
}

bool SvNumberformat::HasPositiveBracketPlaceholder() const
{
    sal_uInt16 nAnz = NumFor[0].GetCount();
    OUString *tmpStr = NumFor[0].Info().sStrArray;
    return tmpStr[nAnz-1] == "_)";
}

DateFormat SvNumberformat::GetDateOrder() const
{
    if ( (eType & css::util::NumberFormat::DATE) == css::util::NumberFormat::DATE )
    {
        short const * const pType = NumFor[0].Info().nTypeArray;
        sal_uInt16 nAnz = NumFor[0].GetCount();
        for ( sal_uInt16 j=0; j<nAnz; j++ )
        {
            switch ( pType[j] )
            {
            case NF_KEY_D :
            case NF_KEY_DD :
                return DMY;
            case NF_KEY_M :
            case NF_KEY_MM :
            case NF_KEY_MMM :
            case NF_KEY_MMMM :
            case NF_KEY_MMMMM :
                return MDY;
            case NF_KEY_YY :
            case NF_KEY_YYYY :
            case NF_KEY_EC :
            case NF_KEY_EEC :
            case NF_KEY_R :
            case NF_KEY_RR :
                return YMD;
            }
        }
    }
    else
    {
       SAL_WARN( "svl.numbers", "SvNumberformat::GetDateOrder: no date" );
    }
    return rLoc().getDateFormat();
}

sal_uInt32 SvNumberformat::GetExactDateOrder() const
{
    sal_uInt32 nRet = 0;
    if ( (eType & css::util::NumberFormat::DATE) != css::util::NumberFormat::DATE )
    {
        SAL_WARN( "svl.numbers", "SvNumberformat::GetExactDateOrder: no date" );
        return nRet;
    }
    short const * const pType = NumFor[0].Info().nTypeArray;
    sal_uInt16 nAnz = NumFor[0].GetCount();
    int nShift = 0;
    for ( sal_uInt16 j=0; j<nAnz && nShift < 3; j++ )
    {
        switch ( pType[j] )
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
    if ( eOp != NUMBERFORMAT_OP_NO )
    {
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
}

OUString SvNumberformat::GetMappedFormatstring( const NfKeywordTable& rKeywords,
                                                const LocaleDataWrapper& rLocWrp ) const
{
    OUStringBuffer aStr;
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
        if ( n > 0 && NumFor[n].Info().eScannedType != css::util::NumberFormat::UNDEFINED )
        {
            nSem++;
        }
        OUString aPrefix;
        bool bLCIDInserted = false;

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

        const SvNumberNatNum& rNum = NumFor[n].GetNatNum();

        sal_uInt16 nAnz = NumFor[n].GetCount();
        if ( nSem && (nAnz || !aPrefix.isEmpty()) )
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
        if ( nAnz )
        {
            const short* pType = NumFor[n].Info().nTypeArray;
            const OUString* pStr = NumFor[n].Info().sStrArray;
            for ( sal_uInt16 j=0; j<nAnz; j++ )
            {
                if ( 0 <= pType[j] && pType[j] < NF_KEYWORD_ENTRIES_COUNT )
                {
                    aStr.append( rKeywords[pType[j]] );
                    if( NF_KEY_NNNN == pType[j] )
                    {
                        aStr.append( rLocWrp.getLongDateDayOfWeekSep() );
                    }
                }
                else
                {
                    switch ( pType[j] )
                    {
                    case NF_SYMBOLTYPE_DECSEP :
                        aStr.append( rLocWrp.getNumDecimalSep() );
                        break;
                    case NF_SYMBOLTYPE_THSEP :
                        aStr.append( rLocWrp.getNumThousandSep() );
                        break;
                    case NF_SYMBOLTYPE_EXP :
                        // tdf#95677: Excel does not support exponent without sign
                        aStr.append( rKeywords[NF_KEY_E] );
                        aStr.append( "+" );
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
                        if ( pStr[j].getLength() == 1 )
                        {
                            if ( pType[j] == NF_SYMBOLTYPE_STRING )
                                aStr.append( '\\' );
                            aStr.append( pStr[j] );
                        }
                        else
                        {
                            aStr.append( '"' );
                            aStr.append( pStr[j] );
                            aStr.append( '"' );
                        }
                        break;
                    case NF_SYMBOLTYPE_CALDEL :
                        if ( pStr[j+1] == "buddhist" )
                        {
                            if ( rNum.IsSet() && rNum.GetNatNum() == 1 &&
                                 MsLangId::getRealLanguage( rNum.GetLang() ) ==
                                 LANGUAGE_THAI )
                            {
                                aStr.insert( 0, "[$-D07041E]" ); // date in Thai digit, Buddhist era
                            }
                            else
                            {
                                aStr.insert( 0, "[$-107041E]" ); // date in Arabic digit, Buddhist era
                            }
                            j = j+2;
                        }
                        bLCIDInserted = true;
                        break;
                    default:
                        aStr.append( pStr[j] );
                    }
                }
            }
        }
        // The Thai T NatNum modifier during Xcl export.
        if (rNum.IsSet() && rNum.GetNatNum() == 1 &&
            rKeywords[NF_KEY_THAI_T] == "T" &&
            MsLangId::getRealLanguage( rNum.GetLang()) ==
            LANGUAGE_THAI && !bLCIDInserted )
        {

            aStr.insert( 0, "[$-D00041E]" ); // number in Thai digit
        }
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
    return GetFormatter().GetNatNum()->getNativeNumberString( rStr,
                                                              aLocale, rNum.GetNatNum() );
}

void SvNumberformat::impTransliterateImpl(OUStringBuffer& rStr,
                                          const SvNumberNatNum& rNum ) const
{
    css::lang::Locale aLocale( LanguageTag( rNum.GetLang() ).getLocale() );

    OUString sTemp(rStr.makeStringAndClear());
    sTemp = GetFormatter().GetNatNum()->getNativeNumberString( sTemp, aLocale, rNum.GetNatNum() );
    rStr.append(sTemp);
}

void SvNumberformat::GetNatNumXml( css::i18n::NativeNumberXmlAttributes& rAttr,
                                   sal_uInt16 nNumFor ) const
{
    if ( nNumFor <= 3 )
    {
        const SvNumberNatNum& rNum = NumFor[nNumFor].GetNatNum();
        if ( rNum.IsSet() )
        {
            css::lang::Locale aLocale(
                    LanguageTag( rNum.GetLang() ).getLocale() );
            rAttr = GetFormatter().GetNatNum()->convertToXmlAttributes(
                    aLocale, rNum.GetNatNum() );
        }
        else
        {
            rAttr = css::i18n::NativeNumberXmlAttributes();
        }
    }
    else
    {
        rAttr = css::i18n::NativeNumberXmlAttributes();
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
        sal_uInt16 nAnz = NumFor[nNumFor].GetCount();
        return nAnz - ImpGetNumForStringElementCount( nNumFor );
    }
    return 0;
}

sal_uInt16 SvNumberformat::ImpGetNumForStringElementCount( sal_uInt16 nNumFor ) const
{
    sal_uInt16 nCnt = 0;
    sal_uInt16 nAnz = NumFor[nNumFor].GetCount();
    short const * const pType = NumFor[nNumFor].Info().nTypeArray;
    for ( sal_uInt16 j=0; j<nAnz; ++j )
    {
        switch ( pType[j] )
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
