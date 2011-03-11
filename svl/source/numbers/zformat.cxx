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
#include "precompiled_svl.hxx"
#include <stdio.h>
#include <ctype.h>
#include <float.h>
#include <errno.h>
#include <stdlib.h>
#include <tools/debug.hxx>
#include <osl/diagnose.h>
#include <i18npool/mslangid.hxx>
#include <rtl/math.hxx>
#include <rtl/instance.hxx>
#include <unotools/charclass.hxx>
#include <unotools/calendarwrapper.hxx>
#include <unotools/nativenumberwrapper.hxx>
#include <com/sun/star/i18n/CalendarFieldIndex.hpp>
#include <com/sun/star/i18n/CalendarDisplayIndex.hpp>
#include <com/sun/star/i18n/CalendarDisplayCode.hpp>
#include <com/sun/star/i18n/AmPmValue.hpp>

#define _ZFORMAT_CXX
#include <svl/zformat.hxx>
#include <zforscan.hxx>

#include "zforfind.hxx"
#include <svl/zforlist.hxx>
#include "numhead.hxx"
#include <unotools/digitgroupingiterator.hxx>
#include <svl/nfsymbol.hxx>

#include <cmath>

using namespace svt;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;

namespace {
struct Gregorian
    : public rtl::StaticWithInit<const ::rtl::OUString, Gregorian> {
    const ::rtl::OUString operator () () {
        return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("gregorian"));
    }
};

const sal_uInt16 UPPER_PRECISION = 300; // entirely arbitrary...
const double EXP_LOWER_BOUND = 1.0E-4; // prefer scientific notation below this value.

}

const double _D_MAX_U_LONG_ = (double) 0xffffffff;      // 4294967295.0
const double _D_MAX_LONG_   = (double) 0x7fffffff;      // 2147483647.0
const sal_uInt16 _MAX_FRACTION_PREC = 3;
const double D_EPS = 1.0E-2;

const double _D_MAX_D_BY_100  = 1.7E306;
const double _D_MIN_M_BY_1000 = 2.3E-305;

static sal_uInt8 cCharWidths[ 128-32 ] = {
    1,1,1,2,2,3,2,1,1,1,1,2,1,1,1,1,
    2,2,2,2,2,2,2,2,2,2,1,1,2,2,2,2,
    3,2,2,2,2,2,2,3,2,1,2,2,2,3,3,3,
    2,3,2,2,2,2,2,3,2,2,2,1,1,1,2,2,
    1,2,2,2,2,2,1,2,2,1,1,2,1,3,2,2,
    2,2,1,2,1,2,2,2,2,2,2,1,1,1,2,1
};

// static
xub_StrLen SvNumberformat::InsertBlanks( String& r, xub_StrLen nPos, sal_Unicode c )
{
    if( c >= 32 )
    {
        sal_uInt16 n = 2;   // Default fuer Zeichen > 128 (HACK!)
        if( c <= 127 )
            n = cCharWidths[ c - 32 ];
        while( n-- )
            r.Insert( ' ', nPos++ );
    }
    return nPos;
}

static long GetPrecExp( double fAbsVal )
{
    DBG_ASSERT( fAbsVal > 0.0, "GetPrecExp: fAbsVal <= 0.0" );
    if ( fAbsVal < 1e-7 || fAbsVal > 1e7 )
    {   // die Schere, ob's schneller ist oder nicht, liegt zwischen 1e6 und 1e7
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

const sal_uInt16 nNewCurrencyVersionId = 0x434E;    // "NC"
const sal_Unicode cNewCurrencyMagic = 0x01;     // Magic for format code in comment
const sal_uInt16 nNewStandardFlagVersionId = 0x4653;    // "SF"

/***********************Funktion SvNumberformatInfo******************************/

void ImpSvNumberformatInfo::Copy( const ImpSvNumberformatInfo& rNumFor, sal_uInt16 nAnz )
{
    for (sal_uInt16 i = 0; i < nAnz; i++)
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

void ImpSvNumberformatInfo::Save(SvStream& rStream, sal_uInt16 nAnz) const
{
    for (sal_uInt16 i = 0; i < nAnz; i++)
    {
        rStream.WriteByteString( sStrArray[i], rStream.GetStreamCharSet() );
        short nType = nTypeArray[i];
        switch ( nType )
        {   // der Krampf fuer Versionen vor SV_NUMBERFORMATTER_VERSION_NEW_CURR
            case NF_SYMBOLTYPE_CURRENCY :
                rStream << short( NF_SYMBOLTYPE_STRING );
            break;
            case NF_SYMBOLTYPE_CURRDEL :
            case NF_SYMBOLTYPE_CURREXT :
                rStream << short(0);        // werden ignoriert (hoffentlich..)
            break;
            default:
                if ( nType > NF_KEY_LASTKEYWORD_SO5 )
                    rStream << short( NF_SYMBOLTYPE_STRING );  // all new keywords are string
                else
                    rStream << nType;
        }

    }
    rStream << eScannedType << bThousand << nThousand
            << nCntPre << nCntPost << nCntExp;
}

void ImpSvNumberformatInfo::Load(SvStream& rStream, sal_uInt16 nAnz)
{
    for (sal_uInt16 i = 0; i < nAnz; i++)
    {
        SvNumberformat::LoadString( rStream, sStrArray[i] );
        rStream >> nTypeArray[i];
    }
    rStream >> eScannedType >> bThousand >> nThousand
            >> nCntPre >> nCntPost >> nCntExp;
}

//============================================================================

// static
sal_uInt8 SvNumberNatNum::MapDBNumToNatNum( sal_uInt8 nDBNum, LanguageType eLang, sal_Bool bDate )
{
    sal_uInt8 nNatNum = 0;
    eLang = MsLangId::getRealLanguage( eLang );  // resolve SYSTEM etc.
    eLang &= 0x03FF;    // 10 bit primary language
    if ( bDate )
    {
        if ( nDBNum == 4 && eLang == LANGUAGE_KOREAN )
            nNatNum = 9;
        else if ( nDBNum <= 3 )
            nNatNum = nDBNum;   // known to be good for: zh,ja,ko / 1,2,3
    }
    else
    {
        switch ( nDBNum )
        {
            case 1:
                switch ( eLang )
                {
                    case (LANGUAGE_CHINESE  & 0x03FF) : nNatNum = 4; break;
                    case (LANGUAGE_JAPANESE & 0x03FF) : nNatNum = 1; break;
                    case (LANGUAGE_KOREAN   & 0x03FF) : nNatNum = 1; break;
                }
                break;
            case 2:
                switch ( eLang )
                {
                    case (LANGUAGE_CHINESE  & 0x03FF) : nNatNum = 5; break;
                    case (LANGUAGE_JAPANESE & 0x03FF) : nNatNum = 4; break;
                    case (LANGUAGE_KOREAN   & 0x03FF) : nNatNum = 2; break;
                }
                break;
            case 3:
                switch ( eLang )
                {
                    case (LANGUAGE_CHINESE  & 0x03FF) : nNatNum = 6; break;
                    case (LANGUAGE_JAPANESE & 0x03FF) : nNatNum = 5; break;
                    case (LANGUAGE_KOREAN   & 0x03FF) : nNatNum = 3; break;
                }
                break;
            case 4:
                switch ( eLang )
                {
                    case (LANGUAGE_JAPANESE & 0x03FF) : nNatNum = 7; break;
                    case (LANGUAGE_KOREAN   & 0x03FF) : nNatNum = 9; break;
                }
                break;
        }
    }
    return nNatNum;
}

// static
sal_uInt8 SvNumberNatNum::MapNatNumToDBNum( sal_uInt8 nNatNum, LanguageType eLang, sal_Bool bDate )
{
    sal_uInt8 nDBNum = 0;
    eLang = MsLangId::getRealLanguage( eLang );  // resolve SYSTEM etc.
    eLang &= 0x03FF;    // 10 bit primary language
    if ( bDate )
    {
        if ( nNatNum == 9 && eLang == LANGUAGE_KOREAN )
            nDBNum = 4;
        else if ( nNatNum <= 3 )
            nDBNum = nNatNum;   // known to be good for: zh,ja,ko / 1,2,3
    }
    else
    {
        switch ( nNatNum )
        {
            case 1:
                switch ( eLang )
                {
                    case (LANGUAGE_JAPANESE & 0x03FF) : nDBNum = 1; break;
                    case (LANGUAGE_KOREAN   & 0x03FF) : nDBNum = 1; break;
                }
                break;
            case 2:
                switch ( eLang )
                {
                    case (LANGUAGE_KOREAN   & 0x03FF) : nDBNum = 2; break;
                }
                break;
            case 3:
                switch ( eLang )
                {
                    case (LANGUAGE_KOREAN   & 0x03FF) : nDBNum = 3; break;
                }
                break;
            case 4:
                switch ( eLang )
                {
                    case (LANGUAGE_CHINESE  & 0x03FF) : nDBNum = 1; break;
                    case (LANGUAGE_JAPANESE & 0x03FF) : nDBNum = 2; break;
                }
                break;
            case 5:
                switch ( eLang )
                {
                    case (LANGUAGE_CHINESE  & 0x03FF) : nDBNum = 2; break;
                    case (LANGUAGE_JAPANESE & 0x03FF) : nDBNum = 3; break;
                }
                break;
            case 6:
                switch ( eLang )
                {
                    case (LANGUAGE_CHINESE  & 0x03FF) : nDBNum = 3; break;
                }
                break;
            case 7:
                switch ( eLang )
                {
                    case (LANGUAGE_JAPANESE & 0x03FF) : nDBNum = 4; break;
                }
                break;
            case 8:
                break;
            case 9:
                switch ( eLang )
                {
                    case (LANGUAGE_KOREAN   & 0x03FF) : nDBNum = 4; break;
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

/***********************Funktionen SvNumFor******************************/

ImpSvNumFor::ImpSvNumFor()
{
    nAnzStrings = 0;
    aI.nTypeArray = NULL;
    aI.sStrArray = NULL;
    aI.eScannedType = NUMBERFORMAT_UNDEFINED;
    aI.bThousand = sal_False;
    aI.nThousand = 0;
    aI.nCntPre = 0;
    aI.nCntPost = 0;
    aI.nCntExp = 0;
    pColor = NULL;
}

ImpSvNumFor::~ImpSvNumFor()
{
    for (sal_uInt16 i = 0; i < nAnzStrings; i++)
        aI.sStrArray[i].Erase();
    delete [] aI.sStrArray;
    delete [] aI.nTypeArray;
}

void ImpSvNumFor::Enlarge(sal_uInt16 nAnz)
{
    if ( nAnzStrings != nAnz )
    {
        if ( aI.nTypeArray )
            delete [] aI.nTypeArray;
        if ( aI.sStrArray )
            delete [] aI.sStrArray;
        nAnzStrings = nAnz;
        if ( nAnz )
        {
            aI.nTypeArray = new short[nAnz];
            aI.sStrArray  = new String[nAnz];
        }
        else
        {
            aI.nTypeArray = NULL;
            aI.sStrArray  = NULL;
        }
    }
}

void ImpSvNumFor::Copy( const ImpSvNumFor& rNumFor, ImpSvNumberformatScan* pSc )
{
    Enlarge( rNumFor.nAnzStrings );
    aI.Copy( rNumFor.aI, nAnzStrings );
    sColorName = rNumFor.sColorName;
    if ( pSc )
        pColor = pSc->GetColor( sColorName );   // #121103# don't copy pointer between documents
    else
        pColor = rNumFor.pColor;
    aNatNum = rNumFor.aNatNum;
}

void ImpSvNumFor::Save(SvStream& rStream) const
{
    rStream << nAnzStrings;
    aI.Save(rStream, nAnzStrings);
    rStream.WriteByteString( sColorName, rStream.GetStreamCharSet() );
}

void ImpSvNumFor::Load(SvStream& rStream, ImpSvNumberformatScan& rSc,
        String& rLoadedColorName )
{
    sal_uInt16 nAnz;
    rStream >> nAnz;        //! noch nicht direkt nAnzStrings wg. Enlarge
    Enlarge( nAnz );
    aI.Load( rStream, nAnz );
    rStream.ReadByteString( sColorName, rStream.GetStreamCharSet() );
    rLoadedColorName = sColorName;
    pColor = rSc.GetColor(sColorName);
}

sal_Bool ImpSvNumFor::HasNewCurrency() const
{
    for ( sal_uInt16 j=0; j<nAnzStrings; j++ )
    {
        if ( aI.nTypeArray[j] == NF_SYMBOLTYPE_CURRENCY )
            return sal_True;
    }
    return sal_False;
}

sal_Bool ImpSvNumFor::GetNewCurrencySymbol( String& rSymbol,
            String& rExtension ) const
{
    for ( sal_uInt16 j=0; j<nAnzStrings; j++ )
    {
        if ( aI.nTypeArray[j] == NF_SYMBOLTYPE_CURRENCY )
        {
            rSymbol = aI.sStrArray[j];
            if ( j < nAnzStrings-1 && aI.nTypeArray[j+1] == NF_SYMBOLTYPE_CURREXT )
                rExtension = aI.sStrArray[j+1];
            else
                rExtension.Erase();
            return sal_True;
        }
    }
    //! kein Erase an rSymbol, rExtension
    return sal_False;
}

void ImpSvNumFor::SaveNewCurrencyMap( SvStream& rStream ) const
{
    sal_uInt16 j;
    sal_uInt16 nCnt = 0;
    for ( j=0; j<nAnzStrings; j++ )
    {
        switch ( aI.nTypeArray[j] )
        {
            case NF_SYMBOLTYPE_CURRENCY :
            case NF_SYMBOLTYPE_CURRDEL :
            case NF_SYMBOLTYPE_CURREXT :
                nCnt++;
            break;
        }
    }
    rStream << nCnt;
    for ( j=0; j<nAnzStrings; j++ )
    {
        switch ( aI.nTypeArray[j] )
        {
            case NF_SYMBOLTYPE_CURRENCY :
            case NF_SYMBOLTYPE_CURRDEL :
            case NF_SYMBOLTYPE_CURREXT :
                rStream << j << aI.nTypeArray[j];
            break;
        }
    }
}

void ImpSvNumFor::LoadNewCurrencyMap( SvStream& rStream )
{
    sal_uInt16 nCnt;
    rStream >> nCnt;
    for ( sal_uInt16 j=0; j<nCnt; j++ )
    {
        sal_uInt16 nPos;
        short nType;
        rStream >> nPos >> nType;
        if ( nPos < nAnzStrings )
            aI.nTypeArray[nPos] = nType;
    }
}

/***********************Funktionen SvNumberformat************************/

enum BracketFormatSymbolType
{
    BRACKET_SYMBOLTYPE_FORMAT   = -1,   // subformat string
    BRACKET_SYMBOLTYPE_COLOR    = -2,   // color
    BRACKET_SYMBOLTYPE_ERROR    = -3,   // error
    BRACKET_SYMBOLTYPE_DBNUM1   = -4,   // DoubleByteNumber, represent numbers
    BRACKET_SYMBOLTYPE_DBNUM2   = -5,   // using CJK characters, Excel compatible.
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
    BRACKET_SYMBOLTYPE_NATNUM2  = -16,  //  numbers using CJK, CTL, ...
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

SvNumberformat::SvNumberformat( ImpSvNumberformatScan& rSc, LanguageType eLge )
        :
        rScan(rSc),
        nNewStandardDefined(0),
        bStarFlag( sal_False )
{
    maLocale.meLanguage = eLge;
}

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
    nNewStandardDefined = rFormat.nNewStandardDefined;

    // #121103# when copying between documents, get color pointers from own scanner
    ImpSvNumberformatScan* pColorSc = ( &rScan != &rFormat.rScan ) ? &rScan : NULL;

    for (sal_uInt16 i = 0; i < 4; i++)
        NumFor[i].Copy(rFormat.NumFor[i], pColorSc);
}

SvNumberformat::SvNumberformat( SvNumberformat& rFormat )
    : rScan(rFormat.rScan), bStarFlag( rFormat.bStarFlag )
{
    ImpCopyNumberformat( rFormat );
}

SvNumberformat::SvNumberformat( SvNumberformat& rFormat, ImpSvNumberformatScan& rSc )
    : rScan(rSc), bStarFlag( rFormat.bStarFlag )
{
    ImpCopyNumberformat( rFormat );
}

sal_Bool lcl_SvNumberformat_IsBracketedPrefix( short nSymbolType )
{
    if ( nSymbolType > 0  )
        return sal_True;        // conditions
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
            return sal_True;
    }
    return sal_False;
}

SvNumberformat::SvNumberformat(String& rString,
                               ImpSvNumberformatScan* pSc,
                               ImpSvNumberInputScan* pISc,
                               xub_StrLen& nCheckPos,
                               LanguageType& eLan,
                               sal_Bool bStan)
        :
        rScan(*pSc),
        nNewStandardDefined(0),
        bStarFlag( sal_False )
{
    // If the group (AKA thousand) separator is a Non-Breaking Space (French)
    // replace all occurrences by a simple space.
    // The tokens will be changed to the LocaleData separator again later on.
    const sal_Unicode cNBSp = 0xA0;
    const String& rThSep = GetFormatter().GetNumThousandSep();
    if ( rThSep.GetChar(0) == cNBSp && rThSep.Len() == 1 )
    {
        xub_StrLen nIndex = 0;
        do
            nIndex = rString.SearchAndReplace( cNBSp, ' ', nIndex );
        while ( nIndex != STRING_NOTFOUND );
    }

    if (rScan.GetConvertMode())
    {
        maLocale.meLanguage = rScan.GetNewLnge();
        eLan = maLocale.meLanguage;      // Wechsel auch zurueckgeben
    }
    else
        maLocale.meLanguage = eLan;
    bStandard = bStan;
    bIsUsed = sal_False;
    fLimit1 = 0.0;
    fLimit2 = 0.0;
    eOp1 = NUMBERFORMAT_OP_NO;
    eOp2 = NUMBERFORMAT_OP_NO;
    eType = NUMBERFORMAT_DEFINED;

    sal_Bool bCancel = sal_False;
    sal_Bool bCondition = sal_False;
    short eSymbolType;
    xub_StrLen nPos = 0;
    xub_StrLen nPosOld;
    nCheckPos = 0;
    String aComment;

    // Split into 4 sub formats
    sal_uInt16 nIndex;
    for ( nIndex = 0; nIndex < 4 && !bCancel; nIndex++ )
    {
        // Original language/country may have to be reestablished
        if (rScan.GetConvertMode())
            (rScan.GetNumberformatter())->ChangeIntl(rScan.GetTmpLnge());

        String sStr;
        nPosOld = nPos;                         // Start position of substring
        // first get bracketed prefixes; e.g. conditions, color
        do
        {
            eSymbolType = ImpNextSymbol(rString, nPos, sStr);
            if (eSymbolType > 0)                    // condition
            {
                if ( nIndex == 0 && !bCondition )
                {
                    bCondition = sal_True;
                    eOp1 = (SvNumberformatLimitOps) eSymbolType;
                }
                else if ( nIndex == 1 && bCondition )
                    eOp2 = (SvNumberformatLimitOps) eSymbolType;
                else                                // error
                {
                    bCancel = sal_True;                 // break for
                    nCheckPos = nPosOld;
                }
                if (!bCancel)
                {
                    double fNumber;
                    xub_StrLen nAnzChars = ImpGetNumber(rString, nPos, sStr);
                    if (nAnzChars > 0)
                    {
                        short F_Type = NUMBERFORMAT_UNDEFINED;
                        if (!pISc->IsNumberFormat(sStr,F_Type,fNumber) ||
                            ( F_Type != NUMBERFORMAT_NUMBER &&
                            F_Type != NUMBERFORMAT_SCIENTIFIC) )
                        {
                            fNumber = 0.0;
                            nPos = nPos - nAnzChars;
                            rString.Erase(nPos, nAnzChars);
                            rString.Insert('0',nPos);
                            nPos++;
                        }
                    }
                    else
                    {
                        fNumber = 0.0;
                        rString.Insert('0',nPos++);
                    }
                    if (nIndex == 0)
                        fLimit1 = fNumber;
                    else
                        fLimit2 = fNumber;
                    if ( rString.GetChar(nPos) == ']' )
                        nPos++;
                    else
                    {
                        bCancel = sal_True;             // break for
                        nCheckPos = nPos;
                    }
                }
                nPosOld = nPos;                     // position before string
            }
            else if ( lcl_SvNumberformat_IsBracketedPrefix( eSymbolType ) )
            {
                switch ( eSymbolType )
                {
                    case BRACKET_SYMBOLTYPE_COLOR :
                    {
                        if ( NumFor[nIndex].GetColor() != NULL )
                        {                           // error, more than one color
                            bCancel = sal_True;         // break for
                            nCheckPos = nPosOld;
                        }
                        else
                        {
                            Color* pColor = pSc->GetColor( sStr);
                            NumFor[nIndex].SetColor( pColor, sStr);
                            if (pColor == NULL)
                            {                       // error
                                bCancel = sal_True;     // break for
                                nCheckPos = nPosOld;
                            }
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
                    {
                        if ( NumFor[nIndex].GetNatNum().IsSet() )
                        {
                            bCancel = sal_True;         // break for
                            nCheckPos = nPosOld;
                        }
                        else
                        {
                            sStr.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "NatNum" ) );
                            //! eSymbolType is negative
                            sal_uInt8 nNum = sal::static_int_cast< sal_uInt8 >(0 - (eSymbolType - BRACKET_SYMBOLTYPE_NATNUM0));
                            sStr += String::CreateFromInt32( nNum );
                            NumFor[nIndex].SetNatNumNum( nNum, sal_False );
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
                    {
                        if ( NumFor[nIndex].GetNatNum().IsSet() )
                        {
                            bCancel = sal_True;         // break for
                            nCheckPos = nPosOld;
                        }
                        else
                        {
                            sStr.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "DBNum" ) );
                            //! eSymbolType is negative
                            sal_uInt8 nNum = sal::static_int_cast< sal_uInt8 >(1 - (eSymbolType - BRACKET_SYMBOLTYPE_DBNUM1));
                            sStr += static_cast< sal_Unicode >('0' + nNum);
                            NumFor[nIndex].SetNatNumNum( nNum, sal_True );
                        }
                    }
                    break;
                    case BRACKET_SYMBOLTYPE_LOCALE :
                    {
                        if ( NumFor[nIndex].GetNatNum().GetLang() != LANGUAGE_DONTKNOW )
                        {
                            bCancel = sal_True;         // break for
                            nCheckPos = nPosOld;
                        }
                        else
                        {
                            xub_StrLen nTmp = 2;
                            maLocale = ImpGetLocaleType( sStr, nTmp );
                            if (maLocale.meLanguage == LANGUAGE_DONTKNOW)
                            {
                                bCancel = sal_True;         // break for
                                nCheckPos = nPosOld;
                            }
                            else
                            {
                                sStr.AssignAscii( RTL_CONSTASCII_STRINGPARAM("$-") );
                                sStr = sStr + maLocale.generateCode();
                                NumFor[nIndex].SetNatNumLang(maLocale.meLanguage);
                            }
                        }
                    }
                    break;
                }
                if ( !bCancel )
                {
                    rString.Erase(nPosOld,nPos-nPosOld);
                    rString.Insert(sStr,nPosOld);
                    nPos = nPosOld + sStr.Len();
                    rString.Insert(']', nPos);
                    rString.Insert('[', nPosOld);
                    nPos += 2;
                    nPosOld = nPos;     // position before string
                }
            }
        } while ( !bCancel && lcl_SvNumberformat_IsBracketedPrefix( eSymbolType ) );

        // The remaining format code string
        if ( !bCancel )
        {
            if (eSymbolType == BRACKET_SYMBOLTYPE_FORMAT)
            {
                if (nIndex == 1 && eOp1 == NUMBERFORMAT_OP_NO)
                    eOp1 = NUMBERFORMAT_OP_GT;  // undefined condition, default: > 0
                else if (nIndex == 2 && eOp2 == NUMBERFORMAT_OP_NO)
                    eOp2 = NUMBERFORMAT_OP_LT;  // undefined condition, default: < 0
                if (sStr.Len() == 0)
                {   // empty sub format
                }
                else
                {
                    xub_StrLen nStrPos = pSc->ScanFormat( sStr, aComment );
                    sal_uInt16 nAnz = pSc->GetAnzResStrings();
                    if (nAnz == 0)              // error
                        nStrPos = 1;
                    if (nStrPos == 0)               // ok
                    {
                        // e.g. Thai T speciality
                        if (pSc->GetNatNumModifier() && !NumFor[nIndex].GetNatNum().IsSet())
                        {
                            String aNat( RTL_CONSTASCII_USTRINGPARAM( "[NatNum"));
                            aNat += String::CreateFromInt32( pSc->GetNatNumModifier());
                            aNat += ']';
                            sStr.Insert( aNat, 0);
                            NumFor[nIndex].SetNatNumNum( pSc->GetNatNumModifier(), sal_False );
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
                                ((eLanguage =
                                  MsLangId::getRealLanguage( eLan))
                                 == LANGUAGE_THAI) &&
                                NumFor[nIndex].GetNatNum().GetLang() ==
                                LANGUAGE_DONTKNOW)
                        {
                            String aLID( RTL_CONSTASCII_USTRINGPARAM( "[$-"));
                            aLID += String::CreateFromInt32( sal_Int32(
                                        eLanguage), 16 ).ToUpperAscii();
                            aLID += ']';
                            sStr.Insert( aLID, 0);
                            NumFor[nIndex].SetNatNumLang( eLanguage);
                        }
                        rString.Erase(nPosOld,nPos-nPosOld);
                        rString.Insert(sStr,nPosOld);
                        nPos = nPosOld + sStr.Len();
                        if (nPos < rString.Len())
                        {
                            rString.Insert(';',nPos);
                            nPos++;
                        }
                        NumFor[nIndex].Enlarge(nAnz);
                        pSc->CopyInfo(&(NumFor[nIndex].Info()), nAnz);
                        // type check
                        if (nIndex == 0)
                            eType = (short) NumFor[nIndex].Info().eScannedType;
                        else if (nIndex == 3)
                        {   // #77026# Everything recognized IS text
                            NumFor[nIndex].Info().eScannedType = NUMBERFORMAT_TEXT;
                        }
                        else if ( (short) NumFor[nIndex].Info().eScannedType !=
                            eType)
                            eType = NUMBERFORMAT_DEFINED;
                    }
                    else
                    {
                        nCheckPos = nPosOld + nStrPos;  // error in string
                        bCancel = sal_True;                 // break for
                    }
                }
            }
            else if (eSymbolType == BRACKET_SYMBOLTYPE_ERROR)   // error
            {
                nCheckPos = nPosOld;
                bCancel = sal_True;
            }
            else if ( lcl_SvNumberformat_IsBracketedPrefix( eSymbolType ) )
            {
                nCheckPos = nPosOld+1;                  // error, prefix in string
                bCancel = sal_True;                         // break for
            }
        }
        if ( bCancel && !nCheckPos )
            nCheckPos = 1;      // nCheckPos is used as an error condition
        if ( !bCancel )
        {
            if ( NumFor[nIndex].GetNatNum().IsSet() &&
                    NumFor[nIndex].GetNatNum().GetLang() == LANGUAGE_DONTKNOW )
                 NumFor[nIndex].SetNatNumLang( eLan );
        }
        if (rString.Len() == nPos)
        {
            if ( nIndex == 2 && eSymbolType == BRACKET_SYMBOLTYPE_FORMAT &&
                    rString.GetChar(nPos-1) == ';' )
            {   // #83510# A 4th subformat explicitly specified to be empty
                // hides any text. Need the type here for HasTextFormat()
                NumFor[3].Info().eScannedType = NUMBERFORMAT_TEXT;
            }
            bCancel = sal_True;
        }
        if ( NumFor[nIndex].GetNatNum().IsSet() )
            NumFor[nIndex].SetNatNumDate(
                (NumFor[nIndex].Info().eScannedType & NUMBERFORMAT_DATE) != 0 );
    }

    if ( bCondition && !nCheckPos )
    {
        if ( nIndex == 1 && NumFor[0].GetCount() == 0 &&
                rString.GetChar(rString.Len()-1) != ';' )
        {   // No format code => GENERAL   but not if specified empty
            String aAdd( pSc->GetStandardName() );
            String aTmp;
            if ( !pSc->ScanFormat( aAdd, aTmp ) )
            {
                sal_uInt16 nAnz = pSc->GetAnzResStrings();
                if ( nAnz )
                {
                    NumFor[0].Enlarge(nAnz);
                    pSc->CopyInfo( &(NumFor[0].Info()), nAnz );
                    rString += aAdd;
                }
            }
        }
        else if ( nIndex == 1 && NumFor[nIndex].GetCount() == 0 &&
                rString.GetChar(rString.Len()-1) != ';' &&
                (NumFor[0].GetCount() > 1 || (NumFor[0].GetCount() == 1 &&
                NumFor[0].Info().nTypeArray[0] != NF_KEY_GENERAL)) )
        {   // No trailing second subformat => GENERAL   but not if specified empty
            // and not if first subformat is GENERAL
            String aAdd( pSc->GetStandardName() );
            String aTmp;
            if ( !pSc->ScanFormat( aAdd, aTmp ) )
            {
                sal_uInt16 nAnz = pSc->GetAnzResStrings();
                if ( nAnz )
                {
                    NumFor[nIndex].Enlarge(nAnz);
                    pSc->CopyInfo( &(NumFor[nIndex].Info()), nAnz );
                    rString += ';';
                    rString += aAdd;
                }
            }
        }
        else if ( nIndex == 2 && NumFor[nIndex].GetCount() == 0 &&
                rString.GetChar(rString.Len()-1) != ';' &&
                eOp2 != NUMBERFORMAT_OP_NO )
        {   // No trailing third subformat => GENERAL   but not if specified empty
            String aAdd( pSc->GetStandardName() );
            String aTmp;
            if ( !pSc->ScanFormat( aAdd, aTmp ) )
            {
                sal_uInt16 nAnz = pSc->GetAnzResStrings();
                if ( nAnz )
                {
                    NumFor[nIndex].Enlarge(nAnz);
                    pSc->CopyInfo( &(NumFor[nIndex].Info()), nAnz );
                    rString += ';';
                    rString += aAdd;
                }
            }
        }
    }
    sFormatstring = rString;
    if ( aComment.Len() )
    {
        SetComment( aComment );     // setzt sComment und sFormatstring
        rString = sFormatstring;    // geaenderten sFormatstring uebernehmen
    }
    if (NumFor[2].GetCount() == 0 &&                 // kein 3. Teilstring
        eOp1 == NUMBERFORMAT_OP_GT && eOp2 == NUMBERFORMAT_OP_NO &&
        fLimit1 == 0.0 && fLimit2 == 0.0)
        eOp1 = NUMBERFORMAT_OP_GE;                  // 0 zum ersten Format dazu

}

SvNumberformat::~SvNumberformat()
{
}

//---------------------------------------------------------------------------
// Next_Symbol
//---------------------------------------------------------------------------
// Zerlegt die Eingabe in Symbole fuer die weitere
// Verarbeitung (Turing-Maschine).
//---------------------------------------------------------------------------
// Ausgangs Zustand = SsStart
//---------------+-------------------+-----------------------+---------------
// Alter Zustand | gelesenes Zeichen | Aktion                | Neuer Zustand
//---------------+-------------------+-----------------------+---------------
// SsStart       | ;                 | Pos--                 | SsGetString
//               | [                 | Symbol += Zeichen     | SsGetBracketed
//               | ]                 | Fehler                | SsStop
//               | BLANK             |                       |
//               | Sonst             | Symbol += Zeichen     | SsGetString
//---------------+-------------------+-----------------------+---------------
// SsGetString   | ;                 |                       | SsStop
//               | Sonst             | Symbol+=Zeichen       |
//---------------+-------------------+-----------------------+---------------
// SsGetBracketed| <, > =            | del [                 |
//               |                   | Symbol += Zeichen     | SsGetCon
//               | BLANK             |                       |
//               | h, H, m, M, s, S  | Symbol += Zeichen     | SsGetTime
//               | sonst             | del [                 |
//               |                   | Symbol += Zeichen     | SsGetPrefix
//---------------+-------------------+-----------------------+---------------
// SsGetTime     | ]                 | Symbol += Zeichen     | SsGetString
//               | h, H, m, M, s, S  | Symbol += Zeichen, *  | SsGetString
//               | sonst             | del [; Symbol+=Zeichen| SsGetPrefix
//---------------+-------------------+-----------------------+---------------
// SsGetPrefix   | ]                 |                       | SsStop
//               | sonst             | Symbol += Zeichen     |
//---------------+-------------------+-----------------------+---------------
// SsGetCon      | >, =              | Symbol+=Zeichen       |
//               | ]                 |                       | SsStop
//               | sonst             | Fehler                | SsStop
//---------------+-------------------+-----------------------+---------------
// * : Sonderbedingung

enum ScanState
{
    SsStop,
    SsStart,
    SsGetCon,           // condition
    SsGetString,        // format string
    SsGetPrefix,        // color or NatNumN
    SsGetTime,          // [HH] for time
    SsGetBracketed      // any [...] not decided yet
};

// read a string until ']' and delete spaces in input
// static
xub_StrLen SvNumberformat::ImpGetNumber(String& rString,
                                 xub_StrLen& nPos,
                                 String& sSymbol)
{
    xub_StrLen nStartPos = nPos;
    sal_Unicode cToken;
    xub_StrLen nLen = rString.Len();
    sSymbol.Erase();
    while ( nPos < nLen && ((cToken = rString.GetChar(nPos)) != ']') )
    {
        if (cToken == ' ')
        {                                               // delete spaces
            rString.Erase(nPos,1);
            nLen--;
        }
        else
        {
            nPos++;
            sSymbol += cToken;
        }
    }
    return nPos - nStartPos;
}

namespace {

sal_Unicode toUniChar(sal_uInt8 n)
{
    sal_Char c;
    if (n < 10)
        c = '0' + n;
    else
        c = 'A' + n - 10;
    return sal_Unicode(c);
}

}

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
            aBuf.append(toUniChar(n));
            nVal = nVal << 4;
        }
    }

    if (mnNumeralShape || mnCalendarType)
    {
        sal_uInt8 nVal = mnCalendarType;
        for (sal_uInt8 i = 0; i < 2; ++i)
        {
            sal_uInt8 n = (nVal & 0xF0) >> 4;
            aBuf.append(toUniChar(n));
            nVal = nVal << 4;
        }
    }
#endif

    sal_uInt16 n16 = static_cast<sal_uInt16>(meLanguage);
    for (sal_uInt8 i = 0; i < 4; ++i)
    {
        sal_uInt8 n = static_cast<sal_uInt8>((n16 & 0xF000) >> 12);
        aBuf.append(toUniChar(n));
        n16 = n16 << 4;
    }

    return aBuf.makeStringAndClear();
}

SvNumberformat::LocaleType::LocaleType() :
    mnNumeralShape(0),
    mnCalendarType(0),
    meLanguage(LANGUAGE_DONTKNOW)
{
}

SvNumberformat::LocaleType::LocaleType(sal_uInt32 nRawNum) :
    mnNumeralShape(0),
    mnCalendarType(0),
    meLanguage(LANGUAGE_DONTKNOW)
{
    meLanguage = static_cast<LanguageType>(nRawNum & 0x0000FFFF);
    nRawNum = (nRawNum >> 16);
    mnCalendarType = static_cast<sal_uInt8>(nRawNum & 0xFF);
    nRawNum = (nRawNum >> 8);
    mnNumeralShape = static_cast<sal_uInt8>(nRawNum & 0xFF);
}

// static
SvNumberformat::LocaleType SvNumberformat::ImpGetLocaleType(
    const String& rString, xub_StrLen& nPos )
{
    sal_uInt32 nNum = 0;
    sal_Unicode cToken = 0;
    xub_StrLen nLen = rString.Len();
    while ( nPos < nLen && ((cToken = rString.GetChar(nPos)) != ']') )
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
            return LANGUAGE_DONTKNOW;
        ++nPos;
    }

    return (nNum && (cToken == ']' || nPos == nLen)) ? LocaleType(nNum) : LocaleType();
}

short SvNumberformat::ImpNextSymbol(String& rString,
                                 xub_StrLen& nPos,
                                 String& sSymbol)
{
    short eSymbolType = BRACKET_SYMBOLTYPE_FORMAT;
    sal_Unicode cToken;
    sal_Unicode cLetter = ' ';                               // Zwischenergebnis
    xub_StrLen nLen = rString.Len();
    ScanState eState = SsStart;
    sSymbol.Erase();
    const String* pKeywords = rScan.GetKeywords();
    while (nPos < nLen && eState != SsStop)
    {
        cToken = rString.GetChar(nPos);
        nPos++;
        switch (eState)
        {
            case SsStart:
            {
                if (cToken == '[')
                {
                    eState = SsGetBracketed;
                    sSymbol += cToken;
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
                else if (cToken == ' ')             // Skip Blanks
                {
                    rString.Erase(nPos-1,1);
                    nPos--;
                    nLen--;
                }
                else
                {
                    sSymbol += cToken;
                    eState = SsGetString;
                    eSymbolType = BRACKET_SYMBOLTYPE_FORMAT;
                }
            }
            break;
            case SsGetBracketed:
            {
                switch (cToken)
                {
                    case '<':
                    case '>':
                    case '=':
                    {
                        sSymbol.EraseAllChars('[');
                        sSymbol += cToken;
                        cLetter = cToken;
                        eState = SsGetCon;
                        switch (cToken)
                        {
                            case '<': eSymbolType = NUMBERFORMAT_OP_LT; break;
                            case '>': eSymbolType = NUMBERFORMAT_OP_GT; break;
                            case '=': eSymbolType = NUMBERFORMAT_OP_EQ; break;
                            default: break;
                        }
                    }
                    break;
                    case ' ':
                    {
                        rString.Erase(nPos-1,1);
                        nPos--;
                        nLen--;
                    }
                    break;
                    case '$' :
                    {
                        if ( rString.GetChar(nPos) == '-' )
                        {   // [$-xxx] locale
                            sSymbol.EraseAllChars('[');
                            eSymbolType = BRACKET_SYMBOLTYPE_LOCALE;
                            eState = SsGetPrefix;
                        }
                        else
                        {   // currency as of SV_NUMBERFORMATTER_VERSION_NEW_CURR
                            eSymbolType = BRACKET_SYMBOLTYPE_FORMAT;
                            eState = SsGetString;
                        }
                        sSymbol += cToken;
                    }
                    break;
                    case '~' :
                    {   // calendarID as of SV_NUMBERFORMATTER_VERSION_CALENDAR
                        eSymbolType = BRACKET_SYMBOLTYPE_FORMAT;
                        sSymbol += cToken;
                        eState = SsGetString;
                    }
                    break;
                    default:
                    {
                        static const String aNatNum( RTL_CONSTASCII_USTRINGPARAM( "NATNUM" ) );
                        static const String aDBNum( RTL_CONSTASCII_USTRINGPARAM( "DBNUM" ) );
                        String aUpperNatNum( rChrCls().toUpper( rString, nPos-1, aNatNum.Len() ) );
                        String aUpperDBNum( rChrCls().toUpper( rString, nPos-1, aDBNum.Len() ) );
                        sal_Unicode cUpper = aUpperNatNum.GetChar(0);
                        sal_Int32 nNatNumNum = rString.Copy( nPos-1+aNatNum.Len() ).ToInt32();
                        sal_Unicode cDBNum = rString.GetChar( nPos-1+aDBNum.Len() );
                        if ( aUpperNatNum == aNatNum && 0 <= nNatNumNum && nNatNumNum <= 19 )
                        {
                            sSymbol.EraseAllChars('[');
                            sSymbol += rString.Copy( --nPos, aNatNum.Len()+1 );
                            nPos += aNatNum.Len()+1;
                            //! SymbolType is negative
                            eSymbolType = (short) (BRACKET_SYMBOLTYPE_NATNUM0 - nNatNumNum);
                            eState = SsGetPrefix;
                        }
                        else if ( aUpperDBNum == aDBNum && '1' <= cDBNum && cDBNum <= '9' )
                        {
                            sSymbol.EraseAllChars('[');
                            sSymbol += rString.Copy( --nPos, aDBNum.Len()+1 );
                            nPos += aDBNum.Len()+1;
                            //! SymbolType is negative
                            eSymbolType = sal::static_int_cast< short >(
                                BRACKET_SYMBOLTYPE_DBNUM1 - (cDBNum - '1'));
                            eState = SsGetPrefix;
                        }
                        else if (cUpper == pKeywords[NF_KEY_H].GetChar(0)   ||  // H
                            cUpper == pKeywords[NF_KEY_MI].GetChar(0)   ||  // M
                            cUpper == pKeywords[NF_KEY_S].GetChar(0)    )   // S
                        {
                            sSymbol += cToken;
                            eState = SsGetTime;
                            cLetter = cToken;
                        }
                        else
                        {
                            sSymbol.EraseAllChars('[');
                            sSymbol += cToken;
                            eSymbolType = BRACKET_SYMBOLTYPE_COLOR;
                            eState = SsGetPrefix;
                        }
                    }
                    break;
                }
            }
            break;
            case SsGetString:
            {
                if (cToken == ';')
                    eState = SsStop;
                else
                    sSymbol += cToken;
            }
            break;
            case SsGetTime:
            {
                if (cToken == ']')
                {
                    sSymbol += cToken;
                    eState = SsGetString;
                    eSymbolType = BRACKET_SYMBOLTYPE_FORMAT;
                }
                else
                {
                    sal_Unicode cUpper = rChrCls().toUpper( rString, nPos-1, 1 ).GetChar(0);
                    if (cUpper == pKeywords[NF_KEY_H].GetChar(0)    ||  // H
                        cUpper == pKeywords[NF_KEY_MI].GetChar(0)   ||  // M
                        cUpper == pKeywords[NF_KEY_S].GetChar(0)    )   // S
                    {
                        if (cLetter == cToken)
                        {
                            sSymbol += cToken;
                            cLetter = ' ';
                        }
                        else
                        {
                            sSymbol.EraseAllChars('[');
                            sSymbol += cToken;
                            eState = SsGetPrefix;
                        }
                    }
                    else
                    {
                        sSymbol.EraseAllChars('[');
                        sSymbol += cToken;
                        eSymbolType = BRACKET_SYMBOLTYPE_COLOR;
                        eState = SsGetPrefix;
                    }
                }
            }
            break;
            case SsGetCon:
            {
                switch (cToken)
                {
                    case '<':
                    {
                        eState = SsStop;
                        eSymbolType = BRACKET_SYMBOLTYPE_ERROR;
                    }
                    break;
                    case '>':
                    {
                        if (cLetter == '<')
                        {
                            sSymbol += cToken;
                            cLetter = ' ';
                            eState = SsStop;
                            eSymbolType = NUMBERFORMAT_OP_NE;
                        }
                        else
                        {
                            eState = SsStop;
                            eSymbolType = BRACKET_SYMBOLTYPE_ERROR;
                        }
                    }
                    break;
                    case '=':
                    {
                        if (cLetter == '<')
                        {
                            sSymbol += cToken;
                            cLetter = ' ';
                            eSymbolType = NUMBERFORMAT_OP_LE;
                        }
                        else if (cLetter == '>')
                        {
                            sSymbol += cToken;
                            cLetter = ' ';
                            eSymbolType = NUMBERFORMAT_OP_GE;
                        }
                        else
                        {
                            eState = SsStop;
                            eSymbolType = BRACKET_SYMBOLTYPE_ERROR;
                        }
                    }
                    break;
                    case ' ':
                    {
                        rString.Erase(nPos-1,1);
                        nPos--;
                        nLen--;
                    }
                    break;
                    default:
                    {
                        eState = SsStop;
                        nPos--;
                    }
                    break;
                }
            }
            break;
            case SsGetPrefix:
            {
                if (cToken == ']')
                    eState = SsStop;
                else
                    sSymbol += cToken;
            }
            break;
            default:
            break;
        }                                   // of switch
    }                                       // of while

    return eSymbolType;
}

NfHackConversion SvNumberformat::Load( SvStream& rStream,
        ImpSvNumMultipleReadHeader& rHdr, SvNumberFormatter* pHackConverter,
        ImpSvNumberInputScan& rISc )
{
    rHdr.StartEntry();
    sal_uInt16 nOp1, nOp2;
    SvNumberformat::LoadString( rStream, sFormatstring );
    rStream >> eType >> fLimit1 >> fLimit2
            >> nOp1 >> nOp2 >> bStandard >> bIsUsed;
    NfHackConversion eHackConversion = NF_CONVERT_NONE;
    sal_Bool bOldConvert = sal_False;
    LanguageType eOldTmpLang = 0;
    LanguageType eOldNewLang = 0;
    if ( pHackConverter )
    {   // werden nur hierbei gebraucht
        bOldConvert = rScan.GetConvertMode();
        eOldTmpLang = rScan.GetTmpLnge();
        eOldNewLang = rScan.GetNewLnge();
    }
    String aLoadedColorName;
    for (sal_uInt16 i = 0; i < 4; i++)
    {
        NumFor[i].Load( rStream, rScan, aLoadedColorName );
        if ( pHackConverter && eHackConversion == NF_CONVERT_NONE )
        {
            //! HACK! ER 29.07.97 13:52
            // leider wurde nicht gespeichert, was SYSTEM on Save wirklich war :-/
            // aber immerhin wird manchmal fuer einen Entry FARBE oder COLOR gespeichert..
            // System-German FARBE nach System-xxx COLOR umsetzen und vice versa,
            //! geht davon aus, dass onSave nur GERMAN und ENGLISH KeyWords in
            //! ImpSvNumberformatScan existierten
            if ( aLoadedColorName.Len() && !NumFor[i].GetColor()
                    && aLoadedColorName != rScan.GetColorString() )
            {
                if ( rScan.GetColorString().EqualsAscii( "FARBE" ) )
                {   // English -> German
                    eHackConversion = NF_CONVERT_ENGLISH_GERMAN;
                    rScan.GetNumberformatter()->ChangeIntl( LANGUAGE_ENGLISH_US );
                    rScan.SetConvertMode( LANGUAGE_ENGLISH_US, LANGUAGE_GERMAN );
                }
                else
                {   // German -> English
                    eHackConversion = NF_CONVERT_GERMAN_ENGLISH;
                    rScan.GetNumberformatter()->ChangeIntl( LANGUAGE_GERMAN );
                    rScan.SetConvertMode( LANGUAGE_GERMAN, LANGUAGE_ENGLISH_US );
                }
                String aColorName = NumFor[i].GetColorName();
                const Color* pColor = rScan.GetColor( aColorName );
                if ( !pColor && aLoadedColorName == aColorName )
                    eHackConversion = NF_CONVERT_NONE;
                rScan.GetNumberformatter()->ChangeIntl( LANGUAGE_SYSTEM );
                rScan.SetConvertMode( eOldTmpLang, eOldNewLang );
                rScan.SetConvertMode( bOldConvert );
            }
        }
    }
    eOp1 = (SvNumberformatLimitOps) nOp1;
    eOp2 = (SvNumberformatLimitOps) nOp2;
    String aComment;        // wird nach dem NewCurrency-Geraffel richtig gesetzt
    if ( rHdr.BytesLeft() )
    {   // ab SV_NUMBERFORMATTER_VERSION_NEWSTANDARD
        SvNumberformat::LoadString( rStream, aComment );
        rStream >> nNewStandardDefined;
    }

    xub_StrLen nNewCurrencyEnd = STRING_NOTFOUND;
    sal_Bool bNewCurrencyComment = ( aComment.GetChar(0) == cNewCurrencyMagic &&
        (nNewCurrencyEnd = aComment.Search( cNewCurrencyMagic, 1 )) != STRING_NOTFOUND );
    sal_Bool bNewCurrencyLoaded = sal_False;
    sal_Bool bNewCurrency = sal_False;

    sal_Bool bGoOn = sal_True;
    while ( rHdr.BytesLeft() && bGoOn )
    {   // as of SV_NUMBERFORMATTER_VERSION_NEW_CURR
        sal_uInt16 nId;
        rStream >> nId;
        switch ( nId )
        {
            case nNewCurrencyVersionId :
            {
                bNewCurrencyLoaded = sal_True;
                rStream >> bNewCurrency;
                if ( bNewCurrency )
                {
                    for ( sal_uInt16 j=0; j<4; j++ )
                    {
                        NumFor[j].LoadNewCurrencyMap( rStream );
                    }
                }
            }
            break;
            case nNewStandardFlagVersionId :
                rStream >> bStandard;   // the real standard flag
            break;
            default:
                DBG_ERRORFILE( "SvNumberformat::Load: unknown header bytes left nId" );
                bGoOn = sal_False;  // stop reading unknown stream left over of newer versions
                // Would be nice to have multiple read/write headers instead
                // but old versions wouldn't know it, TLOT.
        }
    }
    rHdr.EndEntry();

    if ( bNewCurrencyLoaded )
    {
        if ( bNewCurrency && bNewCurrencyComment )
        {   // original Formatstring und Kommentar wiederherstellen
            sFormatstring = aComment.Copy( 1, nNewCurrencyEnd-1 );
            aComment.Erase( 0, nNewCurrencyEnd+1 );
        }
    }
    else if ( bNewCurrencyComment )
    {   // neu, aber mit Version vor SV_NUMBERFORMATTER_VERSION_NEW_CURR gespeichert
        // original Formatstring und Kommentar wiederherstellen
        sFormatstring = aComment.Copy( 1, nNewCurrencyEnd-1 );
        aComment.Erase( 0, nNewCurrencyEnd+1 );
        // Zustaende merken
        short nDefined = ( eType & NUMBERFORMAT_DEFINED );
        sal_uInt16 nNewStandard = nNewStandardDefined;
        // neu parsen etc.
        String aStr( sFormatstring );
        xub_StrLen nCheckPos = 0;
        SvNumberformat* pFormat = new SvNumberformat( aStr, &rScan, &rISc,
            nCheckPos, maLocale.meLanguage, bStandard );
        DBG_ASSERT( !nCheckPos, "SvNumberformat::Load: NewCurrencyRescan nCheckPos" );
        ImpCopyNumberformat( *pFormat );
        delete pFormat;
        // Zustaende wiederherstellen
        eType |= nDefined;
        if ( nNewStandard )
            SetNewStandardDefined( nNewStandard );
    }
    SetComment( aComment );

    if ( eHackConversion != NF_CONVERT_NONE )
    {   //! und weiter mit dem HACK!
        switch ( eHackConversion )
        {
            case NF_CONVERT_ENGLISH_GERMAN :
                ConvertLanguage( *pHackConverter,
                    LANGUAGE_ENGLISH_US, LANGUAGE_GERMAN, sal_True );
            break;
            case NF_CONVERT_GERMAN_ENGLISH :
                ConvertLanguage( *pHackConverter,
                    LANGUAGE_GERMAN, LANGUAGE_ENGLISH_US, sal_True );
            break;
            default:
                DBG_ERRORFILE( "SvNumberformat::Load: eHackConversion unknown" );
        }
    }
    return eHackConversion;
}

void SvNumberformat::ConvertLanguage( SvNumberFormatter& rConverter,
        LanguageType eConvertFrom, LanguageType eConvertTo, sal_Bool bSystem )
{
    xub_StrLen nCheckPos;
    sal_uInt32 nKey;
    short nType = eType;
    String aFormatString( sFormatstring );
    if ( bSystem )
        rConverter.PutandConvertEntrySystem( aFormatString, nCheckPos, nType,
            nKey, eConvertFrom, eConvertTo );
    else
        rConverter.PutandConvertEntry( aFormatString, nCheckPos, nType,
            nKey, eConvertFrom, eConvertTo );
    const SvNumberformat* pFormat = rConverter.GetEntry( nKey );
    DBG_ASSERT( pFormat, "SvNumberformat::ConvertLanguage: Conversion ohne Format" );
    if ( pFormat )
    {
        ImpCopyNumberformat( *pFormat );
        // aus Formatter/Scanner uebernommene Werte zuruecksetzen
        if ( bSystem )
            maLocale.meLanguage = LANGUAGE_SYSTEM;
        // pColor zeigt noch auf Tabelle in temporaerem Formatter/Scanner
        for ( sal_uInt16 i = 0; i < 4; i++ )
        {
            String aColorName = NumFor[i].GetColorName();
            Color* pColor = rScan.GetColor( aColorName );
            NumFor[i].SetColor( pColor, aColorName );
        }
    }
}

// static
void SvNumberformat::LoadString( SvStream& rStream, String& rStr )
{
    CharSet eStream = rStream.GetStreamCharSet();
    ByteString aStr;
    rStream.ReadByteString( aStr );
    sal_Char cStream = NfCurrencyEntry::GetEuroSymbol( eStream );
    if ( aStr.Search( cStream ) == STRING_NOTFOUND )
    {   // simple conversion to unicode
        rStr = UniString( aStr, eStream );
    }
    else
    {
        sal_Unicode cTarget = NfCurrencyEntry::GetEuroSymbol();
        register const sal_Char* p = aStr.GetBuffer();
        register const sal_Char* const pEnd = p + aStr.Len();
        register sal_Unicode* pUni = rStr.AllocBuffer( aStr.Len() );
        while ( p < pEnd )
        {
            if ( *p == cStream )
                *pUni = cTarget;
            else
                *pUni = ByteString::ConvertToUnicode( *p, eStream );
            p++;
            pUni++;
        }
        *pUni = 0;
    }
}

void SvNumberformat::Save( SvStream& rStream, ImpSvNumMultipleWriteHeader& rHdr ) const
{
    String aFormatstring( sFormatstring );
    String aComment( sComment );
#if NF_COMMENT_IN_FORMATSTRING
    // der Kommentar im Formatstring wird nicht gespeichert, um in alten Versionen
    // nicht ins schleudern zu kommen und spaeter getrennte Verarbeitung
    // (z.B. im Dialog) zu ermoeglichen
    SetComment( "", aFormatstring, aComment );
#endif

    sal_Bool bNewCurrency = HasNewCurrency();
    if ( bNewCurrency )
    {   // SV_NUMBERFORMATTER_VERSION_NEW_CURR im Kommentar speichern
        aComment.Insert( cNewCurrencyMagic, 0 );
        aComment.Insert( cNewCurrencyMagic, 0 );
        aComment.Insert( aFormatstring, 1 );
        Build50Formatstring( aFormatstring );       // alten Formatstring generieren
    }

    // old SO5 versions do behave strange (no output) if standard flag is set
    // on formats not prepared for it (not having the following exact types)
    sal_Bool bOldStandard = bStandard;
    if ( bOldStandard )
    {
        switch ( eType )
        {
            case NUMBERFORMAT_NUMBER :
            case NUMBERFORMAT_DATE :
            case NUMBERFORMAT_TIME :
            case NUMBERFORMAT_DATETIME :
            case NUMBERFORMAT_PERCENT :
            case NUMBERFORMAT_SCIENTIFIC :
                // ok to save
            break;
            default:
                bOldStandard = sal_False;
        }
    }

    rHdr.StartEntry();
    rStream.WriteByteString( aFormatstring, rStream.GetStreamCharSet() );
    rStream << eType << fLimit1 << fLimit2 << (sal_uInt16) eOp1 << (sal_uInt16) eOp2
            << bOldStandard << bIsUsed;
    for (sal_uInt16 i = 0; i < 4; i++)
        NumFor[i].Save(rStream);
    // ab SV_NUMBERFORMATTER_VERSION_NEWSTANDARD
    rStream.WriteByteString( aComment, rStream.GetStreamCharSet() );
    rStream << nNewStandardDefined;
    // ab SV_NUMBERFORMATTER_VERSION_NEW_CURR
    rStream << nNewCurrencyVersionId;
    rStream << bNewCurrency;
    if ( bNewCurrency )
    {
        for ( sal_uInt16 j=0; j<4; j++ )
        {
            NumFor[j].SaveNewCurrencyMap( rStream );
        }
    }

    // the real standard flag to load with versions >638 if different
    if ( bStandard != bOldStandard )
    {
        rStream << nNewStandardFlagVersionId;
        rStream << bStandard;
    }

    rHdr.EndEntry();
}

sal_Bool SvNumberformat::HasNewCurrency() const
{
    for ( sal_uInt16 j=0; j<4; j++ )
    {
        if ( NumFor[j].HasNewCurrency() )
            return sal_True;
    }
    return sal_False;
}

sal_Bool SvNumberformat::GetNewCurrencySymbol( String& rSymbol,
            String& rExtension ) const
{
    for ( sal_uInt16 j=0; j<4; j++ )
    {
        if ( NumFor[j].GetNewCurrencySymbol( rSymbol, rExtension ) )
            return sal_True;
    }
    rSymbol.Erase();
    rExtension.Erase();
    return sal_False;
}

// static
String SvNumberformat::StripNewCurrencyDelimiters( const String& rStr,
            sal_Bool bQuoteSymbol )
{
    String aTmp;
    xub_StrLen nStartPos, nPos, nLen;
    nLen = rStr.Len();
    nStartPos = 0;
    while ( (nPos = rStr.SearchAscii( "[$", nStartPos )) != STRING_NOTFOUND )
    {
        xub_StrLen nEnd;
        if ( (nEnd = GetQuoteEnd( rStr, nPos )) < nLen )
        {
            aTmp += rStr.Copy( nStartPos, ++nEnd - nStartPos );
            nStartPos = nEnd;
        }
        else
        {
            aTmp += rStr.Copy( nStartPos, nPos - nStartPos );
            nStartPos = nPos + 2;
            xub_StrLen nDash;
            nEnd = nStartPos - 1;
            do
            {
                nDash = rStr.Search( '-', ++nEnd );
            } while ( (nEnd = GetQuoteEnd( rStr, nDash )) < nLen );
            xub_StrLen nClose;
            nEnd = nStartPos - 1;
            do
            {
                nClose = rStr.Search( ']', ++nEnd );
            } while ( (nEnd = GetQuoteEnd( rStr, nClose )) < nLen );
            nPos = ( nDash < nClose ? nDash : nClose );
            if ( !bQuoteSymbol || rStr.GetChar( nStartPos ) == '"' )
                aTmp += rStr.Copy( nStartPos, nPos - nStartPos );
            else
            {
                aTmp += '"';
                aTmp += rStr.Copy( nStartPos, nPos - nStartPos );
                aTmp += '"';
            }
            nStartPos = nClose + 1;
        }
    }
    if ( nLen > nStartPos )
        aTmp += rStr.Copy( nStartPos, nLen - nStartPos );
    return aTmp;
}

void SvNumberformat::Build50Formatstring( String& rStr ) const
{
    rStr = StripNewCurrencyDelimiters( sFormatstring, sal_True );
}

void SvNumberformat::ImpGetOutputStandard(double& fNumber, String& OutString)
{
    sal_uInt16 nStandardPrec = rScan.GetStandardPrec();

    if ( fabs(fNumber) > 1.0E15 )       // #58531# war E16
    {
        nStandardPrec = ::std::min(nStandardPrec, static_cast<sal_uInt16>(14)); // limits to 14 decimals
        OutString = ::rtl::math::doubleToUString( fNumber,
                rtl_math_StringFormat_E, nStandardPrec /*2*/,
                GetFormatter().GetNumDecimalSep().GetChar(0));
    }
    else
        ImpGetOutputStdToPrecision(fNumber, OutString, nStandardPrec);
}

void SvNumberformat::ImpGetOutputStdToPrecision(double& rNumber, String& rOutString, sal_uInt16 nPrecision) const
{
    // Make sure the precision doesn't go over the maximum allowable precision.
    nPrecision = ::std::min(UPPER_PRECISION, nPrecision);

#if 0
{
    // debugger test case for ANSI standard correctness
    ::rtl::OUString aTest;
    // expect 0.00123   OK
    aTest = ::rtl::math::doubleToUString( 0.001234567,
            rtl_math_StringFormat_G, 3, '.', sal_True );
    // expect 123       OK
    aTest = ::rtl::math::doubleToUString( 123.4567,
            rtl_math_StringFormat_G, 3, '.', sal_True );
    // expect 123.5     OK
    aTest = ::rtl::math::doubleToUString( 123.4567,
            rtl_math_StringFormat_G, 4, '.', sal_True );
    // expect 1e+03 (as 999.6 rounded to 3 significant digits results in
    // 1000 with an exponent equal to significant digits)
    // Currently (24-Jan-2003) we do fail in this case and output 1000
    // instead, negligible.
    aTest = ::rtl::math::doubleToUString( 999.6,
            rtl_math_StringFormat_G, 3, '.', sal_True );
    // expect what? result is 1.2e+004
    aTest = ::rtl::math::doubleToUString( 12345.6789,
            rtl_math_StringFormat_G, -3, '.', sal_True );
}
#endif

    // We decided to strip trailing zeros unconditionally, since binary
    // double-precision rounding error makes it impossible to determine e.g.
    // whether 844.10000000000002273737 is what the user has typed, or the
    // user has typed 844.1 but IEEE 754 represents it that way internally.

    rOutString = ::rtl::math::doubleToUString( rNumber,
            rtl_math_StringFormat_F, nPrecision /*2*/,
            GetFormatter().GetNumDecimalSep().GetChar(0), true );
    if (rOutString.GetChar(0) == '-' &&
        rOutString.GetTokenCount('0') == rOutString.Len())
        rOutString.EraseLeadingChars('-');            // nicht -0

    ImpTransliterate( rOutString, NumFor[0].GetNatNum() );
}

void SvNumberformat::ImpGetOutputInputLine(double fNumber, String& OutString)
{
    sal_Bool bModified = sal_False;
    if ( (eType & NUMBERFORMAT_PERCENT) && (fabs(fNumber) < _D_MAX_D_BY_100))
    {
        if (fNumber == 0.0)
        {
            OutString.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "0%" ) );
            return;
        }
        fNumber *= 100;
        bModified = sal_True;
    }

    if (fNumber == 0.0)
    {
        OutString = '0';
        return;
    }

    OutString = ::rtl::math::doubleToUString( fNumber,
            rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max,
            GetFormatter().GetNumDecimalSep().GetChar(0), sal_True );

    if ( eType & NUMBERFORMAT_PERCENT && bModified)
        OutString += '%';
    return;
}

short SvNumberformat::ImpCheckCondition(double& fNumber,
                                     double& fLimit,
                                     SvNumberformatLimitOps eOp)
{
    switch(eOp)
    {
        case NUMBERFORMAT_OP_NO: return -1;
        case NUMBERFORMAT_OP_EQ: return (short) (fNumber == fLimit);
        case NUMBERFORMAT_OP_NE: return (short) (fNumber != fLimit);
        case NUMBERFORMAT_OP_LT: return (short) (fNumber <  fLimit);
        case NUMBERFORMAT_OP_LE: return (short) (fNumber <= fLimit);
        case NUMBERFORMAT_OP_GT: return (short) (fNumber >  fLimit);
        case NUMBERFORMAT_OP_GE: return (short) (fNumber >= fLimit);
        default: return -1;
    }
}

sal_Bool SvNumberformat::GetOutputString(String& sString,
                                     String& OutString,
                                     Color** ppColor)
{
    OutString.Erase();
    sal_uInt16 nIx;
    if (eType & NUMBERFORMAT_TEXT)
        nIx = 0;
    else if (NumFor[3].GetCount() > 0)
        nIx = 3;
    else
    {
        *ppColor = NULL;        // no change of color
        return sal_False;
    }
    *ppColor = NumFor[nIx].GetColor();
    const ImpSvNumberformatInfo& rInfo = NumFor[nIx].Info();
    if (rInfo.eScannedType == NUMBERFORMAT_TEXT)
    {
        sal_Bool bRes = sal_False;
        const sal_uInt16 nAnz = NumFor[nIx].GetCount();
        for (sal_uInt16 i = 0; i < nAnz; i++)
        {
            switch (rInfo.nTypeArray[i])
            {
                case NF_SYMBOLTYPE_STAR:
                    if( bStarFlag )
                    {
                        OutString += (sal_Unicode) 0x1B;
                        OutString += rInfo.sStrArray[i].GetChar(1);
                        bRes = sal_True;
                    }
                break;
                case NF_SYMBOLTYPE_BLANK:
                    InsertBlanks( OutString, OutString.Len(),
                        rInfo.sStrArray[i].GetChar(1) );
                break;
                case NF_KEY_GENERAL :   // #77026# "General" is the same as "@"
                case NF_SYMBOLTYPE_DEL :
                    OutString += sString;
                break;
                default:
                    OutString += rInfo.sStrArray[i];
            }
        }
        return bRes;
    }
    return sal_False;
}
/*
void SvNumberformat::GetNextFareyNumber(sal_uLong nPrec, sal_uLong x0, sal_uLong x1,
                                        sal_uLong y0, sal_uLong y1,
                                        sal_uLong& x2,sal_uLong& y2)
{
    x2 = ((y0+nPrec)/y1)*x1 - x0;
    y2 = ((y0+nPrec)/y1)*y1 - y0;
}
*/
sal_uLong SvNumberformat::ImpGGT(sal_uLong x, sal_uLong y)
{
    if (y == 0)
        return x;
    else
    {
        sal_uLong z = x%y;
        while (z)
        {
            x = y;
            y = z;
            z = x%y;
        }
        return y;
    }
}

sal_uLong SvNumberformat::ImpGGTRound(sal_uLong x, sal_uLong y)
{
    if (y == 0)
        return x;
    else
    {
        sal_uLong z = x%y;
        while ((double)z/(double)y > D_EPS)
        {
            x = y;
            y = z;
            z = x%y;
        }
        return y;
    }
}

namespace {

void lcl_GetOutputStringScientific(
    double fNumber, sal_uInt16 nCharCount, const SvNumberFormatter& rFormatter, String& rOutString)
{
    bool bSign = ::rtl::math::isSignBitSet(fNumber);

    // 1.000E+015 (one digit and the decimal point, and the five chars for the exponential part, totalling 7).
    sal_uInt16 nPrec = nCharCount > 7 ? nCharCount - 7 : 0;
    if (nPrec && bSign)
        // Make room for the negative sign.
        --nPrec;

    nPrec = ::std::min(nPrec, static_cast<sal_uInt16>(14)); // limit to 14 decimals.

    rOutString = ::rtl::math::doubleToUString(
        fNumber, rtl_math_StringFormat_E, nPrec, rFormatter.GetNumDecimalSep().GetChar(0));
}

sal_Int32 lcl_GetForcedDenominator(ImpSvNumberformatInfo &rInfo, USHORT nAnz)
{
    USHORT i;
    rtl::OUString aDiv;
    for( i = 0; i < nAnz; i++ )
    {
        if( rInfo.nTypeArray[i] == NF_SYMBOLTYPE_FRAC_FDIV )
            aDiv += rInfo.sStrArray[i];
    }
    return aDiv.toInt32();
}

// TODO: More optimizations?
void lcl_ForcedDenominator(ULONG &nFrac, ULONG &nDiv, ULONG nForcedDiv)
{
    double fFrac = (double)nFrac / (double)nDiv;
    double fMultiplier = (double)nForcedDiv / (double)nDiv;
    nFrac = (ULONG)( (double)nFrac * fMultiplier );

    double fFracNew = (double)nFrac / (double)nForcedDiv;
    double fFracNew1 = (double)(nFrac + 1) / (double)nForcedDiv;
    double fDiff = fFrac - fFracNew;
    if( fDiff > ( fFracNew1 - fFrac ) )
        nFrac++;
    nDiv = nForcedDiv;
}

}

bool SvNumberformat::GetOutputString(double fNumber, sal_uInt16 nCharCount, String& rOutString) const
{
    using namespace std;

    if (eType != NUMBERFORMAT_NUMBER)
        return false;

    double fTestNum = fNumber;
    bool bSign = ::rtl::math::isSignBitSet(fTestNum);
    if (bSign)
        fTestNum = -fTestNum;

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
        // Subtract the negative sign.
        --nPrec;
    if (nPrec)
        // Subtract the decimal point.
        --nPrec;

    ImpGetOutputStdToPrecision(fNumber, rOutString, nPrec);
    if (rOutString.Len() > nCharCount)
        // String still wider than desired.  Switch to scientific notation.
        lcl_GetOutputStringScientific(fNumber, nCharCount, GetFormatter(), rOutString);

    return true;
}

sal_Bool SvNumberformat::GetOutputString(double fNumber,
                                     String& OutString,
                                     Color** ppColor)
{
    sal_Bool bRes = sal_False;
    OutString.Erase();                          // alles loeschen
    *ppColor = NULL;                            // keine Farbaenderung
    if (eType & NUMBERFORMAT_LOGICAL)
    {
        if (fNumber)
            OutString = rScan.GetTrueString();
        else
            OutString = rScan.GetFalseString();
        return sal_False;
    }
    if (eType & NUMBERFORMAT_TEXT)
    {
        ImpGetOutputStandard(fNumber, OutString);
        return sal_False;
    }
    sal_Bool bHadStandard = sal_False;
    if (bStandard)                              // einzelne Standardformate
    {
        if (rScan.GetStandardPrec() == SvNumberFormatter::INPUTSTRING_PRECISION)     // alle Zahlformate InputLine
        {
            ImpGetOutputInputLine(fNumber, OutString);
            return false;
        }
        switch (eType)
        {
            case NUMBERFORMAT_NUMBER:                   // Standardzahlformat
            {
                if (rScan.GetStandardPrec() == SvNumberFormatter::UNLIMITED_PRECISION)
                {
                    bool bSign = ::rtl::math::isSignBitSet(fNumber);
                    if (bSign)
                        fNumber = -fNumber;
                    ImpGetOutputStdToPrecision(fNumber, OutString, 10); // Use 10 decimals for general 'unlimited' format.
                    if (fNumber < EXP_LOWER_BOUND)
                    {
                        xub_StrLen nLen = OutString.Len();
                        if (!nLen)
                            return false;

                        // #i112250# With the 10-decimal limit, small numbers are formatted as "0".
                        // Switch to scientific in that case, too:
                        if (nLen > 11 || (OutString.EqualsAscii("0") && fNumber != 0.0))
                        {
                            sal_uInt16 nStandardPrec = rScan.GetStandardPrec();
                            nStandardPrec = ::std::min(nStandardPrec, static_cast<sal_uInt16>(14)); // limits to 14 decimals
                            OutString = ::rtl::math::doubleToUString( fNumber,
                                    rtl_math_StringFormat_E, nStandardPrec /*2*/,
                                    GetFormatter().GetNumDecimalSep().GetChar(0), true);
                        }
                    }
                    if (bSign)
                        OutString.Insert('-', 0);
                    return false;
                }
                ImpGetOutputStandard(fNumber, OutString);
                bHadStandard = sal_True;
            }
            break;
            case NUMBERFORMAT_DATE:
                bRes |= ImpGetDateOutput(fNumber, 0, OutString);
                bHadStandard = sal_True;
            break;
            case NUMBERFORMAT_TIME:
                bRes |= ImpGetTimeOutput(fNumber, 0, OutString);
                bHadStandard = sal_True;
            break;
            case NUMBERFORMAT_DATETIME:
                bRes |= ImpGetDateTimeOutput(fNumber, 0, OutString);
                bHadStandard = sal_True;
            break;
        }
    }
    if ( !bHadStandard )
    {
        sal_uInt16 nIx;                             // Index des Teilformats
        short nCheck = ImpCheckCondition(fNumber, fLimit1, eOp1);
        if (nCheck == -1 || nCheck == 1)            // nur 1 String oder True
            nIx = 0;
        else
        {
            nCheck = ImpCheckCondition(fNumber, fLimit2, eOp2);
            if (nCheck == -1 || nCheck == 1)
                nIx = 1;
            else
                nIx = 2;
        }
        if (nIx == 1 && fNumber < 0.0 &&        // negatives Format
                IsNegativeRealNegative() )      // ohne Vorzeichen
            fNumber = -fNumber;                 // Vorzeichen eliminieren
        *ppColor = NumFor[nIx].GetColor();
        const ImpSvNumberformatInfo& rInfo = NumFor[nIx].Info();
        const sal_uInt16 nAnz = NumFor[nIx].GetCount();
        if (nAnz == 0 && rInfo.eScannedType == NUMBERFORMAT_UNDEFINED)
            return sal_False;                       // leer => nichts
        else if (nAnz == 0)                     // sonst Standard-Format
        {
            ImpGetOutputStandard(fNumber, OutString);
            return sal_False;
        }
        switch (rInfo.eScannedType)
        {
            case NUMBERFORMAT_TEXT:
            case NUMBERFORMAT_DEFINED:
            {
                for (sal_uInt16 i = 0; i < nAnz; i++)
                {
                    switch (rInfo.nTypeArray[i])
                    {
                        case NF_SYMBOLTYPE_STAR:
                            if( bStarFlag )
                            {
                                OutString += (sal_Unicode) 0x1B;
                                OutString += rInfo.sStrArray[i].GetChar(1);
                                bRes = sal_True;
                            }
                            break;
                        case NF_SYMBOLTYPE_BLANK:
                            InsertBlanks( OutString, OutString.Len(),
                                rInfo.sStrArray[i].GetChar(1) );
                            break;
                        case NF_SYMBOLTYPE_STRING:
                        case NF_SYMBOLTYPE_CURRENCY:
                            OutString += rInfo.sStrArray[i];
                            break;
                        case NF_SYMBOLTYPE_THSEP:
                            if (rInfo.nThousand == 0)
                                OutString += rInfo.sStrArray[i];
                        break;
                        default:
                        break;
                    }
                }
            }
            break;
            case NUMBERFORMAT_DATE:
                bRes |= ImpGetDateOutput(fNumber, nIx, OutString);
            break;
            case NUMBERFORMAT_TIME:
                bRes |= ImpGetTimeOutput(fNumber, nIx, OutString);
            break;
            case NUMBERFORMAT_DATETIME:
                bRes |= ImpGetDateTimeOutput(fNumber, nIx, OutString);
            break;
            case NUMBERFORMAT_NUMBER:
            case NUMBERFORMAT_PERCENT:
            case NUMBERFORMAT_CURRENCY:
                bRes |= ImpGetNumberOutput(fNumber, nIx, OutString);
            break;
            case NUMBERFORMAT_FRACTION:
            {
                String sStr, sFrac, sDiv;               // Strings, Wert fuer
                sal_uLong nFrac, nDiv;                      // Vorkommaanteil
                                                        // Zaehler und Nenner
                sal_Bool bSign = sal_False;
                if (fNumber < 0)
                {
                    if (nIx == 0)                       // nicht in hinteren
                        bSign = sal_True;                   // Formaten
                    fNumber = -fNumber;
                }
                double fNum = floor(fNumber);           // Vorkommateil
                fNumber -= fNum;                        // Nachkommateil
                if (fNum > _D_MAX_U_LONG_ || rInfo.nCntExp > 9)
                                                        // zu gross
                {
                    OutString = rScan.GetErrorString();
                    return sal_False;
                }
                if (rInfo.nCntExp == 0)
                {
                    OSL_FAIL("SvNumberformat:: Bruch, nCntExp == 0");
                    return sal_False;
                }
                sal_uLong nBasis = ((sal_uLong)floor(           // 9, 99, 999 ,...
                                    pow(10.0,rInfo.nCntExp))) - 1;
                sal_uLong x0, y0, x1, y1;

                if (rInfo.nCntExp <= _MAX_FRACTION_PREC)
                {
                    sal_Bool bUpperHalf;
                    if (fNumber > 0.5)
                    {
                        bUpperHalf = sal_True;
                        fNumber -= (fNumber - 0.5) * 2.0;
                    }
                    else
                        bUpperHalf = sal_False;
                                                    // Einstieg in Farey-Serie
                                                    // finden:
                    x0 = (sal_uLong) floor(fNumber*nBasis); // z.B. 2/9 <= x < 3/9
                    if (x0 == 0)                        //      => x0 = 2
                    {
                        y0 = 1;
                        x1 = 1;
                        y1 = nBasis;
                    }
                    else if (x0 == (nBasis-1)/2)    // (b-1)/2, 1/2
                    {                               // geht (nBasis ungerade)
                        y0 = nBasis;
                        x1 = 1;
                        y1 = 2;
                    }
                    else if (x0 == 1)
                    {
                        y0 = nBasis;                    //  1/n; 1/(n-1)
                        x1 = 1;
                        y1 = nBasis - 1;
                    }
                    else
                    {
                        y0 = nBasis;                    // z.B. 2/9   2/8
                        x1 = x0;
                        y1 = nBasis - 1;
                        double fUg = (double) x0 / (double) y0;
                        double fOg = (double) x1 / (double) y1;
                        sal_uLong nGgt = ImpGGT(y0, x0);       // x0/y0 kuerzen
                        x0 /= nGgt;
                        y0 /= nGgt;                     // Einschachteln:
                        sal_uLong x2 = 0;
                        sal_uLong y2 = 0;
                        sal_Bool bStop = sal_False;
                        while (!bStop)
                        {
#ifdef GCC
                            // #i21648# GCC over-optimizes something resulting
                            // in wrong fTest values throughout the loops.
                            volatile
#endif
                                double fTest = (double)x1/(double)y1;
                            while (!bStop)
                            {
                                while (fTest > fOg)
                                {
                                    x1--;
                                    fTest = (double)x1/(double)y1;
                                }
                                while (fTest < fUg && y1 > 1)
                                {
                                    y1--;
                                    fTest = (double)x1/(double)y1;
                                }
                                if (fTest <= fOg)
                                {
                                    fOg = fTest;
                                    bStop = sal_True;
                                }
                                else if (y1 == 1)
                                    bStop = sal_True;
                            }                               // of while
                            nGgt = ImpGGT(y1, x1);             // x1/y1 kuerzen
                            x2 = x1 / nGgt;
                            y2 = y1 / nGgt;
                            if (x2*y0 - x0*y2 == 1 || y1 <= 1)  // Test, ob x2/y2
                                bStop = sal_True;               // naechste Farey-Zahl
                            else
                            {
                                y1--;
                                bStop = sal_False;
                            }
                        }                                   // of while
                        x1 = x2;
                        y1 = y2;
                    }                                       // of else
                    double fup, flow;
                    flow = (double)x0/(double)y0;
                    fup  = (double)x1/(double)y1;
                    while (fNumber > fup)
                    {
                        sal_uLong x2 = ((y0+nBasis)/y1)*x1 - x0; // naechste Farey-Zahl
                        sal_uLong y2 = ((y0+nBasis)/y1)*y1 - y0;
//                      GetNextFareyNumber(nBasis, x0, x1, y0, y1, x2, y2);
                        x0 = x1;
                        y0 = y1;
                        x1 = x2;
                        y1 = y2;
                        flow = fup;
                        fup  = (double)x1/(double)y1;
                    }
                    if (fNumber - flow < fup - fNumber)
                    {
                        nFrac = x0;
                        nDiv  = y0;
                    }
                    else
                    {
                        nFrac = x1;
                        nDiv  = y1;
                    }
                    if (bUpperHalf)                     // Original restaur.
                    {
                        if (nFrac == 0 && nDiv == 1)    // 1/1
                            fNum += 1.0;
                        else
                            nFrac = nDiv - nFrac;
                    }
                }
                else                                    // grosse Nenner
                {                                       // 0,1234->123/1000
                    sal_uLong nGgt;
/*
                    nDiv = nBasis+1;
                    nFrac = ((sal_uLong)floor(0.5 + fNumber *
                                    pow(10.0,rInfo.nCntExp)));
*/
                    nDiv = 10000000;
                    nFrac = ((sal_uLong)floor(0.5 + fNumber * 10000000.0));
                    nGgt = ImpGGT(nDiv, nFrac);
                    if (nGgt > 1)
                    {
                        nDiv  /= nGgt;
                        nFrac /= nGgt;
                    }
                    if (nDiv > nBasis)
                    {
                        nGgt = ImpGGTRound(nDiv, nFrac);
                        if (nGgt > 1)
                        {
                            nDiv  /= nGgt;
                            nFrac /= nGgt;
                        }
                    }
                    if (nDiv > nBasis)
                    {
                        nDiv = nBasis;
                        nFrac = ((sal_uLong)floor(0.5 + fNumber *
                                    pow(10.0,rInfo.nCntExp)));
                        nGgt = ImpGGTRound(nDiv, nFrac);
                        if (nGgt > 1)
                        {
                            nDiv  /= nGgt;
                            nFrac /= nGgt;
                        }
                    }
                }

                if( sal_Int32 nForcedDiv = lcl_GetForcedDenominator(NumFor[nIx].Info(), nAnz) )
                {
                    lcl_ForcedDenominator(nFrac, nDiv, nForcedDiv);
                    if( nFrac >= nDiv )
                    {
                        nFrac = nDiv = 0;
                        fNum = fNum + 1.0;
                    }
                }

                if (rInfo.nCntPre == 0)    // unechter Bruch
                {
                    double fNum1 = fNum * (double)nDiv + (double)nFrac;
                    if (fNum1 > _D_MAX_U_LONG_)
                    {
                        OutString = rScan.GetErrorString();
                        return sal_False;
                    }
                    nFrac = (sal_uLong) floor(fNum1);
                    sStr.Erase();
                }
                else if (fNum == 0.0 && nFrac != 0)
                    sStr.Erase();
                else
                {
                    char aBuf[100];
                    sprintf( aBuf, "%.f", fNum );   // simple rounded integer (#100211# - checked)
                    sStr.AssignAscii( aBuf );
                    ImpTransliterate( sStr, NumFor[nIx].GetNatNum() );
                }
                if (rInfo.nCntPre > 0 && nFrac == 0)
                {
                    sFrac.Erase();
                    sDiv.Erase();
                }
                else
                {
                    sFrac = ImpIntToString( nIx, nFrac );
                    sDiv = ImpIntToString( nIx, nDiv );
                }

                sal_uInt16 j = nAnz-1;                  // letztes Symbol->rueckw.
                xub_StrLen k;                       // Nenner:
                bRes |= ImpNumberFill(sDiv, fNumber, k, j, nIx, NF_SYMBOLTYPE_FRAC);
                sal_Bool bCont = sal_True;
                if (rInfo.nTypeArray[j] == NF_SYMBOLTYPE_FRAC)
                {
                    if (rInfo.nCntPre > 0 && nFrac == 0)
                        sDiv.Insert(' ',0);
                    else
                        sDiv.Insert( rInfo.sStrArray[j].GetChar(0), 0 );
                    if ( j )
                        j--;
                    else
                        bCont = sal_False;
                }
                                                    // weiter Zaehler:
                if ( !bCont )
                    sFrac.Erase();
                else
                {
                    bRes |= ImpNumberFill(sFrac, fNumber, k, j, nIx, NF_SYMBOLTYPE_FRACBLANK);
                    if (rInfo.nTypeArray[j] == NF_SYMBOLTYPE_FRACBLANK)
                    {
                        sFrac.Insert(rInfo.sStrArray[j],0);
                        if ( j )
                            j--;
                        else
                            bCont = sal_False;
                    }
                }
                                                    // weiter Hauptzahl
                if ( !bCont )
                    sStr.Erase();
                else
                {
                    k = sStr.Len();                 // hinter letzter Ziffer
                    bRes |= ImpNumberFillWithThousands(sStr, fNumber, k, j, nIx,
                                            rInfo.nCntPre);
                }
                if (bSign && !(nFrac == 0 && fNum == 0.0))
                    OutString.Insert('-',0);        // nicht -0
                OutString += sStr;
                OutString += sFrac;
                OutString += sDiv;
            }
            break;
            case NUMBERFORMAT_SCIENTIFIC:
            {
                sal_Bool bSign = sal_False;
                if (fNumber < 0)
                {
                    if (nIx == 0)                       // nicht in hinteren
                        bSign = sal_True;                   // Formaten
                    fNumber = -fNumber;
                }
                String sStr( ::rtl::math::doubleToUString( fNumber,
                            rtl_math_StringFormat_E,
                            rInfo.nCntPre + rInfo.nCntPost - 1, '.' ));

                String ExpStr;
                short nExpSign = 1;
                xub_StrLen nExPos = sStr.Search('E');
                if ( nExPos != STRING_NOTFOUND )
                {
                    // split into mantisse and exponent and get rid of "E+" or "E-"
                    xub_StrLen nExpStart = nExPos + 1;
                    switch ( sStr.GetChar( nExpStart ) )
                    {
                        case '-' :
                            nExpSign = -1;
                            // fallthru
                        case '+' :
                            ++nExpStart;
                        break;
                    }
                    ExpStr = sStr.Copy( nExpStart );    // part following the "E+"
                    sStr.Erase( nExPos );
                    sStr.EraseAllChars('.');        // cut any decimal delimiter
                    if ( rInfo.nCntPre != 1 )       // rescale Exp
                    {
                        sal_Int32 nExp = ExpStr.ToInt32() * nExpSign;
                        nExp -= sal_Int32(rInfo.nCntPre)-1;
                        if ( nExp < 0 )
                        {
                            nExpSign = -1;
                            nExp = -nExp;
                        }
                        else
                            nExpSign = 1;
                        ExpStr = String::CreateFromInt32( nExp );
                    }
                }
                sal_uInt16 j = nAnz-1;                  // last symbol
                xub_StrLen k;                       // position in ExpStr
                bRes |= ImpNumberFill(ExpStr, fNumber, k, j, nIx, NF_SYMBOLTYPE_EXP);

                xub_StrLen nZeros = 0;              // erase leading zeros
                while (nZeros < k && ExpStr.GetChar(nZeros) == '0')
                    ++nZeros;
                if (nZeros)
                    ExpStr.Erase( 0, nZeros);

                sal_Bool bCont = sal_True;
                if (rInfo.nTypeArray[j] == NF_SYMBOLTYPE_EXP)
                {
                    const String& rStr = rInfo.sStrArray[j];
                    if (nExpSign == -1)
                        ExpStr.Insert('-',0);
                    else if (rStr.Len() > 1 && rStr.GetChar(1) == '+')
                        ExpStr.Insert('+',0);
                    ExpStr.Insert(rStr.GetChar(0),0);
                    if ( j )
                        j--;
                    else
                        bCont = sal_False;
                }
                                                    // weiter Hauptzahl:
                if ( !bCont )
                    sStr.Erase();
                else
                {
                    k = sStr.Len();                 // hinter letzter Ziffer
                    bRes |= ImpNumberFillWithThousands(sStr,fNumber, k,j,nIx,
                                            rInfo.nCntPre +
                                            rInfo.nCntPost);
                }
                if (bSign)
                    sStr.Insert('-',0);
                OutString = sStr;
                OutString += ExpStr;
            }
            break;
        }
    }
    return bRes;
}

sal_Bool SvNumberformat::ImpGetTimeOutput(double fNumber,
                                   sal_uInt16 nIx,
                                   String& OutString)
{
    using namespace ::com::sun::star::i18n;
    sal_Bool bCalendarSet = sal_False;
    double fNumberOrig = fNumber;
    sal_Bool bRes = sal_False;
    sal_Bool bSign = sal_False;
    if (fNumber < 0.0)
    {
        fNumber = -fNumber;
        if (nIx == 0)
            bSign = sal_True;
    }
    const ImpSvNumberformatInfo& rInfo = NumFor[nIx].Info();
    if (rInfo.bThousand)       // []-Format
    {
        if (fNumber > 1.0E10)               // zu gross
        {
            OutString = rScan.GetErrorString();
            return sal_False;
        }
    }
    else
        fNumber -= floor(fNumber);          // sonst Datum abtrennen
    sal_Bool bInputLine;
    xub_StrLen nCntPost;
    if ( rScan.GetStandardPrec() == 300 &&
            0 < rInfo.nCntPost && rInfo.nCntPost < 7 )
    {   // round at 7 decimals (+5 of 86400 == 12 significant digits)
        bInputLine = sal_True;
        nCntPost = 7;
    }
    else
    {
        bInputLine = sal_False;
        nCntPost = xub_StrLen(rInfo.nCntPost);
    }
    if (bSign && !rInfo.bThousand)     // kein []-Format
        fNumber = 1.0 - fNumber;        // "Kehrwert"
    double fTime = fNumber * 86400.0;
    fTime = ::rtl::math::round( fTime, int(nCntPost) );
    if (bSign && fTime == 0.0)
        bSign = sal_False;                      // nicht -00:00:00

    if( floor( fTime ) > _D_MAX_U_LONG_ )
    {
        OutString = rScan.GetErrorString();
        return sal_False;
    }
    sal_uLong nSeconds = (sal_uLong)floor( fTime );

    String sSecStr( ::rtl::math::doubleToUString( fTime-nSeconds,
                rtl_math_StringFormat_F, int(nCntPost), '.'));
    sSecStr.EraseLeadingChars('0');
    sSecStr.EraseLeadingChars('.');
    if ( bInputLine )
    {
        sSecStr.EraseTrailingChars('0');
        if ( sSecStr.Len() < xub_StrLen(rInfo.nCntPost) )
            sSecStr.Expand( xub_StrLen(rInfo.nCntPost), '0' );
        ImpTransliterate( sSecStr, NumFor[nIx].GetNatNum() );
        nCntPost = sSecStr.Len();
    }
    else
        ImpTransliterate( sSecStr, NumFor[nIx].GetNatNum() );

    xub_StrLen nSecPos = 0;                 // Zum Ziffernweisen
                                            // abarbeiten
    sal_uLong nHour, nMin, nSec;
    if (!rInfo.bThousand)      // kein [] Format
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
    else {
        // TODO  What should these be set to?
        nHour = 0;
        nMin  = 0;
        nSec  = 0;
    }

    sal_Unicode cAmPm = ' ';                   // a oder p
    if (rInfo.nCntExp)     // AM/PM
    {
        if (nHour == 0)
        {
            nHour = 12;
            cAmPm = 'a';
        }
        else if (nHour < 12)
            cAmPm = 'a';
        else
        {
            cAmPm = 'p';
            if (nHour > 12)
                nHour -= 12;
        }
    }
    const sal_uInt16 nAnz = NumFor[nIx].GetCount();
    for (sal_uInt16 i = 0; i < nAnz; i++)
    {
        switch (rInfo.nTypeArray[i])
        {
            case NF_SYMBOLTYPE_STAR:
                if( bStarFlag )
                {
                    OutString += (sal_Unicode) 0x1B;
                    OutString += rInfo.sStrArray[i].GetChar(1);
                    bRes = sal_True;
                }
                break;
            case NF_SYMBOLTYPE_BLANK:
                InsertBlanks( OutString, OutString.Len(),
                    rInfo.sStrArray[i].GetChar(1) );
                break;
            case NF_SYMBOLTYPE_STRING:
            case NF_SYMBOLTYPE_CURRENCY:
            case NF_SYMBOLTYPE_DATESEP:
            case NF_SYMBOLTYPE_TIMESEP:
            case NF_SYMBOLTYPE_TIME100SECSEP:
                OutString += rInfo.sStrArray[i];
                break;
            case NF_SYMBOLTYPE_DIGIT:
            {
                xub_StrLen nLen = ( bInputLine && i > 0 &&
                    (rInfo.nTypeArray[i-1] == NF_SYMBOLTYPE_STRING ||
                     rInfo.nTypeArray[i-1] == NF_SYMBOLTYPE_TIME100SECSEP) ?
                    nCntPost : rInfo.sStrArray[i].Len() );
                for (xub_StrLen j = 0; j < nLen && nSecPos < nCntPost; j++)
                {
                    OutString += sSecStr.GetChar(nSecPos);
                    nSecPos++;
                }
            }
            break;
            case NF_KEY_AMPM:               // AM/PM
            {
                if ( !bCalendarSet )
                {
                    double fDiff = DateTime(*(rScan.GetNullDate())) - GetCal().getEpochStart();
                    fDiff += fNumberOrig;
                    GetCal().setLocalDateTime( fDiff );
                    bCalendarSet = sal_True;
                }
                if (cAmPm == 'a')
                    OutString += GetCal().getDisplayName(
                        CalendarDisplayIndex::AM_PM, AmPmValue::AM, 0 );
                else
                    OutString += GetCal().getDisplayName(
                        CalendarDisplayIndex::AM_PM, AmPmValue::PM, 0 );
            }
            break;
            case NF_KEY_AP:                 // A/P
            {
                if (cAmPm == 'a')
                    OutString += 'a';
                else
                    OutString += 'p';
            }
            break;
            case NF_KEY_MI:                 // M
                OutString += ImpIntToString( nIx, nMin );
            break;
            case NF_KEY_MMI:                // MM
                OutString += ImpIntToString( nIx, nMin, 2 );
            break;
            case NF_KEY_H:                  // H
                OutString += ImpIntToString( nIx, nHour );
            break;
            case NF_KEY_HH:                 // HH
                OutString += ImpIntToString( nIx, nHour, 2 );
            break;
            case NF_KEY_S:                  // S
                OutString += ImpIntToString( nIx, nSec );
            break;
            case NF_KEY_SS:                 // SS
                OutString += ImpIntToString( nIx, nSec, 2 );
            break;
            default:
            break;
        }
    }
    if (bSign && rInfo.bThousand)
        OutString.Insert('-',0);
    return bRes;
}

sal_Bool SvNumberformat::ImpIsOtherCalendar( const ImpSvNumFor& rNumFor ) const
{
    if ( GetCal().getUniqueID() != Gregorian::get() )
        return sal_False;
    const ImpSvNumberformatInfo& rInfo = rNumFor.Info();
    const sal_uInt16 nAnz = rNumFor.GetCount();
    sal_uInt16 i;
    for ( i = 0; i < nAnz; i++ )
    {
        switch ( rInfo.nTypeArray[i] )
        {
            case NF_SYMBOLTYPE_CALENDAR :
                return sal_False;
            case NF_KEY_EC :
            case NF_KEY_EEC :
            case NF_KEY_R :
            case NF_KEY_RR :
            case NF_KEY_AAA :
            case NF_KEY_AAAA :
                return sal_True;
        }
    }
    return sal_False;
}

void SvNumberformat::SwitchToOtherCalendar( String& rOrgCalendar,
        double& fOrgDateTime ) const
{
    CalendarWrapper& rCal = GetCal();
    const rtl::OUString &rGregorian = Gregorian::get();
    if ( rCal.getUniqueID() == rGregorian )
    {
        using namespace ::com::sun::star::i18n;
        ::com::sun::star::uno::Sequence< ::rtl::OUString > xCals
            = rCal.getAllCalendars( rLoc().getLocale() );
        sal_Int32 nCnt = xCals.getLength();
        if ( nCnt > 1 )
        {
            for ( sal_Int32 j=0; j < nCnt; j++ )
            {
                if ( xCals[j] != rGregorian )
                {
                    if ( !rOrgCalendar.Len() )
                    {
                        rOrgCalendar = rCal.getUniqueID();
                        fOrgDateTime = rCal.getDateTime();
                    }
                    rCal.loadCalendar( xCals[j], rLoc().getLocale() );
                    rCal.setDateTime( fOrgDateTime );
                    break;  // for
                }
            }
        }
    }
}

void SvNumberformat::SwitchToGregorianCalendar( const String& rOrgCalendar,
        double fOrgDateTime ) const
{
    CalendarWrapper& rCal = GetCal();
    const rtl::OUString &rGregorian = Gregorian::get();
    if ( rOrgCalendar.Len() && rCal.getUniqueID() != rGregorian )
    {
        rCal.loadCalendar( rGregorian, rLoc().getLocale() );
        rCal.setDateTime( fOrgDateTime );
    }
}

sal_Bool SvNumberformat::ImpFallBackToGregorianCalendar( String& rOrgCalendar, double& fOrgDateTime )
{
    using namespace ::com::sun::star::i18n;
    CalendarWrapper& rCal = GetCal();
    const rtl::OUString &rGregorian = Gregorian::get();
    if ( rCal.getUniqueID() != rGregorian )
    {
        sal_Int16 nVal = rCal.getValue( CalendarFieldIndex::ERA );
        if ( nVal == 0 && rCal.getLoadedCalendar().Eras[0].ID.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM( "Dummy" ) ) )
        {
            if ( !rOrgCalendar.Len() )
            {
                rOrgCalendar = rCal.getUniqueID();
                fOrgDateTime = rCal.getDateTime();
            }
            else if ( rOrgCalendar == String(rGregorian) )
                rOrgCalendar.Erase();
            rCal.loadCalendar( rGregorian, rLoc().getLocale() );
            rCal.setDateTime( fOrgDateTime );
            return sal_True;
        }
    }
    return sal_False;
}

sal_Bool SvNumberformat::ImpSwitchToSpecifiedCalendar( String& rOrgCalendar,
        double& fOrgDateTime, const ImpSvNumFor& rNumFor ) const
{
    const ImpSvNumberformatInfo& rInfo = rNumFor.Info();
    const sal_uInt16 nAnz = rNumFor.GetCount();
    for ( sal_uInt16 i = 0; i < nAnz; i++ )
    {
        if ( rInfo.nTypeArray[i] == NF_SYMBOLTYPE_CALENDAR )
        {
            CalendarWrapper& rCal = GetCal();
            if ( !rOrgCalendar.Len() )
            {
                rOrgCalendar = rCal.getUniqueID();
                fOrgDateTime = rCal.getDateTime();
            }
            rCal.loadCalendar( rInfo.sStrArray[i], rLoc().getLocale() );
            rCal.setDateTime( fOrgDateTime );
            return sal_True;
        }
    }
    return sal_False;
}

// static
void SvNumberformat::ImpAppendEraG( String& OutString,
        const CalendarWrapper& rCal, sal_Int16 nNatNum )
{
    using namespace ::com::sun::star::i18n;
    if ( rCal.getUniqueID().equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "gengou" ) ) )
    {
        sal_Unicode cEra;
        sal_Int16 nVal = rCal.getValue( CalendarFieldIndex::ERA );
        switch ( nVal )
        {
            case 1 :    cEra = 'M'; break;
            case 2 :    cEra = 'T'; break;
            case 3 :    cEra = 'S'; break;
            case 4 :    cEra = 'H'; break;
            default:
                cEra = '?';
        }
        OutString += cEra;
    }
    else
        OutString += rCal.getDisplayString( CalendarDisplayCode::SHORT_ERA, nNatNum );
}

sal_Bool SvNumberformat::ImpGetDateOutput(double fNumber,
                                   sal_uInt16 nIx,
                                   String& OutString)
{
    using namespace ::com::sun::star::i18n;
    sal_Bool bRes = sal_False;
    CalendarWrapper& rCal = GetCal();
    double fDiff = DateTime(*(rScan.GetNullDate())) - rCal.getEpochStart();
    fNumber += fDiff;
    rCal.setLocalDateTime( fNumber );
    String aOrgCalendar;        // empty => not changed yet
    double fOrgDateTime;
    sal_Bool bOtherCalendar = ImpIsOtherCalendar( NumFor[nIx] );
    if ( bOtherCalendar )
        SwitchToOtherCalendar( aOrgCalendar, fOrgDateTime );
    if ( ImpFallBackToGregorianCalendar( aOrgCalendar, fOrgDateTime ) )
        bOtherCalendar = sal_False;
    const ImpSvNumberformatInfo& rInfo = NumFor[nIx].Info();
    const sal_uInt16 nAnz = NumFor[nIx].GetCount();
    sal_Int16 nNatNum = NumFor[nIx].GetNatNum().GetNatNum();
    for (sal_uInt16 i = 0; i < nAnz; i++)
    {
        switch (rInfo.nTypeArray[i])
        {
            case NF_SYMBOLTYPE_CALENDAR :
                if ( !aOrgCalendar.Len() )
                {
                    aOrgCalendar = rCal.getUniqueID();
                    fOrgDateTime = rCal.getDateTime();
                }
                rCal.loadCalendar( rInfo.sStrArray[i], rLoc().getLocale() );
                rCal.setDateTime( fOrgDateTime );
                ImpFallBackToGregorianCalendar( aOrgCalendar, fOrgDateTime );
            break;
            case NF_SYMBOLTYPE_STAR:
                if( bStarFlag )
                {
                    OutString += (sal_Unicode) 0x1B;
                    OutString += rInfo.sStrArray[i].GetChar(1);
                    bRes = sal_True;
                }
            break;
            case NF_SYMBOLTYPE_BLANK:
                InsertBlanks( OutString, OutString.Len(),
                    rInfo.sStrArray[i].GetChar(1) );
            break;
            case NF_SYMBOLTYPE_STRING:
            case NF_SYMBOLTYPE_CURRENCY:
            case NF_SYMBOLTYPE_DATESEP:
            case NF_SYMBOLTYPE_TIMESEP:
            case NF_SYMBOLTYPE_TIME100SECSEP:
                OutString += rInfo.sStrArray[i];
            break;
            case NF_KEY_M:                  // M
                OutString += rCal.getDisplayString(
                        CalendarDisplayCode::SHORT_MONTH, nNatNum );
            break;
            case NF_KEY_MM:                 // MM
                OutString += rCal.getDisplayString(
                        CalendarDisplayCode::LONG_MONTH, nNatNum );
            break;
            case NF_KEY_MMM:                // MMM
                OutString += rCal.getDisplayString(
                        CalendarDisplayCode::SHORT_MONTH_NAME, nNatNum );
            break;
            case NF_KEY_MMMM:               // MMMM
                OutString += rCal.getDisplayString(
                        CalendarDisplayCode::LONG_MONTH_NAME, nNatNum );
            break;
            case NF_KEY_MMMMM:              // MMMMM
                OutString += rCal.getDisplayString(
                        CalendarDisplayCode::LONG_MONTH_NAME, nNatNum ).GetChar(0);
            break;
            case NF_KEY_Q:                  // Q
                OutString += rCal.getDisplayString(
                        CalendarDisplayCode::SHORT_QUARTER, nNatNum );
            break;
            case NF_KEY_QQ:                 // QQ
                OutString += rCal.getDisplayString(
                        CalendarDisplayCode::LONG_QUARTER, nNatNum );
            break;
            case NF_KEY_D:                  // D
                OutString += rCal.getDisplayString(
                        CalendarDisplayCode::SHORT_DAY, nNatNum );
            break;
            case NF_KEY_DD:                 // DD
                OutString += rCal.getDisplayString(
                        CalendarDisplayCode::LONG_DAY, nNatNum );
            break;
            case NF_KEY_DDD:                // DDD
            {
                if ( bOtherCalendar )
                    SwitchToGregorianCalendar( aOrgCalendar, fOrgDateTime );
                OutString += rCal.getDisplayString(
                        CalendarDisplayCode::SHORT_DAY_NAME, nNatNum );
                if ( bOtherCalendar )
                    SwitchToOtherCalendar( aOrgCalendar, fOrgDateTime );
            }
            break;
            case NF_KEY_DDDD:               // DDDD
            {
                if ( bOtherCalendar )
                    SwitchToGregorianCalendar( aOrgCalendar, fOrgDateTime );
                OutString += rCal.getDisplayString(
                        CalendarDisplayCode::LONG_DAY_NAME, nNatNum );
                if ( bOtherCalendar )
                    SwitchToOtherCalendar( aOrgCalendar, fOrgDateTime );
            }
            break;
            case NF_KEY_YY:                 // YY
            {
                if ( bOtherCalendar )
                    SwitchToGregorianCalendar( aOrgCalendar, fOrgDateTime );
                OutString += rCal.getDisplayString(
                        CalendarDisplayCode::SHORT_YEAR, nNatNum );
                if ( bOtherCalendar )
                    SwitchToOtherCalendar( aOrgCalendar, fOrgDateTime );
            }
            break;
            case NF_KEY_YYYY:               // YYYY
            {
                if ( bOtherCalendar )
                    SwitchToGregorianCalendar( aOrgCalendar, fOrgDateTime );
                OutString += rCal.getDisplayString(
                        CalendarDisplayCode::LONG_YEAR, nNatNum );
                if ( bOtherCalendar )
                    SwitchToOtherCalendar( aOrgCalendar, fOrgDateTime );
            }
            break;
            case NF_KEY_EC:                 // E
                OutString += rCal.getDisplayString(
                        CalendarDisplayCode::SHORT_YEAR, nNatNum );
            break;
            case NF_KEY_EEC:                // EE
            case NF_KEY_R:                  // R
                OutString += rCal.getDisplayString(
                        CalendarDisplayCode::LONG_YEAR, nNatNum );
            break;
            case NF_KEY_NN:                 // NN
            case NF_KEY_AAA:                // AAA
                OutString += rCal.getDisplayString(
                        CalendarDisplayCode::SHORT_DAY_NAME, nNatNum );
            break;
            case NF_KEY_NNN:                // NNN
            case NF_KEY_AAAA:               // AAAA
                OutString += rCal.getDisplayString(
                        CalendarDisplayCode::LONG_DAY_NAME, nNatNum );
            break;
            case NF_KEY_NNNN:               // NNNN
            {
                OutString += rCal.getDisplayString(
                        CalendarDisplayCode::LONG_DAY_NAME, nNatNum );
                OutString += rLoc().getLongDateDayOfWeekSep();
            }
            break;
            case NF_KEY_WW :                // WW
            {
                sal_Int16 nVal = rCal.getValue( CalendarFieldIndex::WEEK_OF_YEAR );
                OutString += ImpIntToString( nIx, nVal );
            }
            break;
            case NF_KEY_G:                  // G
                ImpAppendEraG( OutString, rCal, nNatNum );
            break;
            case NF_KEY_GG:                 // GG
                OutString += rCal.getDisplayString(
                        CalendarDisplayCode::SHORT_ERA, nNatNum );
            break;
            case NF_KEY_GGG:                // GGG
                OutString += rCal.getDisplayString(
                        CalendarDisplayCode::LONG_ERA, nNatNum );
            break;
            case NF_KEY_RR:                 // RR => GGGEE
                OutString += rCal.getDisplayString(
                        CalendarDisplayCode::LONG_YEAR_AND_ERA, nNatNum );
            break;
        }
    }
    if ( aOrgCalendar.Len() )
        rCal.loadCalendar( aOrgCalendar, rLoc().getLocale() );  // restore calendar
    return bRes;
}

sal_Bool SvNumberformat::ImpGetDateTimeOutput(double fNumber,
                                       sal_uInt16 nIx,
                                       String& OutString)
{
    using namespace ::com::sun::star::i18n;
    sal_Bool bRes = sal_False;

    CalendarWrapper& rCal = GetCal();
    double fDiff = DateTime(*(rScan.GetNullDate())) - rCal.getEpochStart();
    fNumber += fDiff;

    const ImpSvNumberformatInfo& rInfo = NumFor[nIx].Info();
    sal_Bool bInputLine;
    xub_StrLen nCntPost;
    if ( rScan.GetStandardPrec() == 300 &&
            0 < rInfo.nCntPost && rInfo.nCntPost < 7 )
    {   // round at 7 decimals (+5 of 86400 == 12 significant digits)
        bInputLine = sal_True;
        nCntPost = 7;
    }
    else
    {
        bInputLine = sal_False;
        nCntPost = xub_StrLen(rInfo.nCntPost);
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

    String aOrgCalendar;        // empty => not changed yet
    double fOrgDateTime;
    sal_Bool bOtherCalendar = ImpIsOtherCalendar( NumFor[nIx] );
    if ( bOtherCalendar )
        SwitchToOtherCalendar( aOrgCalendar, fOrgDateTime );
    if ( ImpFallBackToGregorianCalendar( aOrgCalendar, fOrgDateTime ) )
        bOtherCalendar = sal_False;
    sal_Int16 nNatNum = NumFor[nIx].GetNatNum().GetNatNum();

    sal_uLong nSeconds = (sal_uLong)floor( fTime );
    String sSecStr( ::rtl::math::doubleToUString( fTime-nSeconds,
                rtl_math_StringFormat_F, int(nCntPost), '.'));
    sSecStr.EraseLeadingChars('0');
    sSecStr.EraseLeadingChars('.');
    if ( bInputLine )
    {
        sSecStr.EraseTrailingChars('0');
        if ( sSecStr.Len() < xub_StrLen(rInfo.nCntPost) )
            sSecStr.Expand( xub_StrLen(rInfo.nCntPost), '0' );
        ImpTransliterate( sSecStr, NumFor[nIx].GetNatNum() );
        nCntPost = sSecStr.Len();
    }
    else
        ImpTransliterate( sSecStr, NumFor[nIx].GetNatNum() );

    xub_StrLen nSecPos = 0;                     // Zum Ziffernweisen
                                            // abarbeiten
    sal_uLong nHour, nMin, nSec;
    if (!rInfo.bThousand)      // [] Format
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
    else {
        nHour = 0;  // TODO What should these values be?
        nMin  = 0;
        nSec  = 0;
    }
    sal_Unicode cAmPm = ' ';                   // a oder p
    if (rInfo.nCntExp)     // AM/PM
    {
        if (nHour == 0)
        {
            nHour = 12;
            cAmPm = 'a';
        }
        else if (nHour < 12)
            cAmPm = 'a';
        else
        {
            cAmPm = 'p';
            if (nHour > 12)
                nHour -= 12;
        }
    }
    const sal_uInt16 nAnz = NumFor[nIx].GetCount();
    for (sal_uInt16 i = 0; i < nAnz; i++)
    {
        switch (rInfo.nTypeArray[i])
        {
            case NF_SYMBOLTYPE_CALENDAR :
                if ( !aOrgCalendar.Len() )
                {
                    aOrgCalendar = rCal.getUniqueID();
                    fOrgDateTime = rCal.getDateTime();
                }
                rCal.loadCalendar( rInfo.sStrArray[i], rLoc().getLocale() );
                rCal.setDateTime( fOrgDateTime );
                ImpFallBackToGregorianCalendar( aOrgCalendar, fOrgDateTime );
                break;
            case NF_SYMBOLTYPE_STAR:
                if( bStarFlag )
                {
                    OutString += (sal_Unicode) 0x1B;
                    OutString += rInfo.sStrArray[i].GetChar(1);
                    bRes = sal_True;
                }
                break;
            case NF_SYMBOLTYPE_BLANK:
                InsertBlanks( OutString, OutString.Len(),
                    rInfo.sStrArray[i].GetChar(1) );
                break;
            case NF_SYMBOLTYPE_STRING:
            case NF_SYMBOLTYPE_CURRENCY:
            case NF_SYMBOLTYPE_DATESEP:
            case NF_SYMBOLTYPE_TIMESEP:
            case NF_SYMBOLTYPE_TIME100SECSEP:
                OutString += rInfo.sStrArray[i];
                break;
            case NF_SYMBOLTYPE_DIGIT:
            {
                xub_StrLen nLen = ( bInputLine && i > 0 &&
                    (rInfo.nTypeArray[i-1] == NF_SYMBOLTYPE_STRING ||
                     rInfo.nTypeArray[i-1] == NF_SYMBOLTYPE_TIME100SECSEP) ?
                    nCntPost : rInfo.sStrArray[i].Len() );
                for (xub_StrLen j = 0; j < nLen && nSecPos < nCntPost; j++)
                {
                    OutString += sSecStr.GetChar(nSecPos);
                    nSecPos++;
                }
            }
            break;
            case NF_KEY_AMPM:               // AM/PM
            {
                if (cAmPm == 'a')
                    OutString += rCal.getDisplayName( CalendarDisplayIndex::AM_PM,
                        AmPmValue::AM, 0 );
                else
                    OutString += rCal.getDisplayName( CalendarDisplayIndex::AM_PM,
                        AmPmValue::PM, 0 );
            }
            break;
            case NF_KEY_AP:                 // A/P
            {
                if (cAmPm == 'a')
                    OutString += 'a';
                else
                    OutString += 'p';
            }
            break;
            case NF_KEY_MI:                 // M
                OutString += ImpIntToString( nIx, nMin );
            break;
            case NF_KEY_MMI:                // MM
                OutString += ImpIntToString( nIx, nMin, 2 );
            break;
            case NF_KEY_H:                  // H
                OutString += ImpIntToString( nIx, nHour );
            break;
            case NF_KEY_HH:                 // HH
                OutString += ImpIntToString( nIx, nHour, 2 );
            break;
            case NF_KEY_S:                  // S
                OutString += ImpIntToString( nIx, nSec );
            break;
            case NF_KEY_SS:                 // SS
                OutString += ImpIntToString( nIx, nSec, 2 );
            break;
            case NF_KEY_M:                  // M
                OutString += rCal.getDisplayString(
                        CalendarDisplayCode::SHORT_MONTH, nNatNum );
            break;
            case NF_KEY_MM:                 // MM
                OutString += rCal.getDisplayString(
                        CalendarDisplayCode::LONG_MONTH, nNatNum );
            break;
            case NF_KEY_MMM:                // MMM
                OutString += rCal.getDisplayString(
                        CalendarDisplayCode::SHORT_MONTH_NAME, nNatNum );
            break;
            case NF_KEY_MMMM:               // MMMM
                OutString += rCal.getDisplayString(
                        CalendarDisplayCode::LONG_MONTH_NAME, nNatNum );
            break;
            case NF_KEY_MMMMM:              // MMMMM
                OutString += rCal.getDisplayString(
                        CalendarDisplayCode::LONG_MONTH_NAME, nNatNum ).GetChar(0);
            break;
            case NF_KEY_Q:                  // Q
                OutString += rCal.getDisplayString(
                        CalendarDisplayCode::SHORT_QUARTER, nNatNum );
            break;
            case NF_KEY_QQ:                 // QQ
                OutString += rCal.getDisplayString(
                        CalendarDisplayCode::LONG_QUARTER, nNatNum );
            break;
            case NF_KEY_D:                  // D
                OutString += rCal.getDisplayString(
                        CalendarDisplayCode::SHORT_DAY, nNatNum );
            break;
            case NF_KEY_DD:                 // DD
                OutString += rCal.getDisplayString(
                        CalendarDisplayCode::LONG_DAY, nNatNum );
            break;
            case NF_KEY_DDD:                // DDD
            {
                if ( bOtherCalendar )
                    SwitchToGregorianCalendar( aOrgCalendar, fOrgDateTime );
                OutString += rCal.getDisplayString(
                        CalendarDisplayCode::SHORT_DAY_NAME, nNatNum );
                if ( bOtherCalendar )
                    SwitchToOtherCalendar( aOrgCalendar, fOrgDateTime );
            }
            break;
            case NF_KEY_DDDD:               // DDDD
            {
                if ( bOtherCalendar )
                    SwitchToGregorianCalendar( aOrgCalendar, fOrgDateTime );
                OutString += rCal.getDisplayString(
                        CalendarDisplayCode::LONG_DAY_NAME, nNatNum );
                if ( bOtherCalendar )
                    SwitchToOtherCalendar( aOrgCalendar, fOrgDateTime );
            }
            break;
            case NF_KEY_YY:                 // YY
            {
                if ( bOtherCalendar )
                    SwitchToGregorianCalendar( aOrgCalendar, fOrgDateTime );
                OutString += rCal.getDisplayString(
                        CalendarDisplayCode::SHORT_YEAR, nNatNum );
                if ( bOtherCalendar )
                    SwitchToOtherCalendar( aOrgCalendar, fOrgDateTime );
            }
            break;
            case NF_KEY_YYYY:               // YYYY
            {
                if ( bOtherCalendar )
                    SwitchToGregorianCalendar( aOrgCalendar, fOrgDateTime );
                OutString += rCal.getDisplayString(
                        CalendarDisplayCode::LONG_YEAR, nNatNum );
                if ( bOtherCalendar )
                    SwitchToOtherCalendar( aOrgCalendar, fOrgDateTime );
            }
            break;
            case NF_KEY_EC:                 // E
                OutString += rCal.getDisplayString(
                        CalendarDisplayCode::SHORT_YEAR, nNatNum );
            break;
            case NF_KEY_EEC:                // EE
            case NF_KEY_R:                  // R
                OutString += rCal.getDisplayString(
                        CalendarDisplayCode::LONG_YEAR, nNatNum );
            break;
            case NF_KEY_NN:                 // NN
            case NF_KEY_AAA:                // AAA
                OutString += rCal.getDisplayString(
                        CalendarDisplayCode::SHORT_DAY_NAME, nNatNum );
            break;
            case NF_KEY_NNN:                // NNN
            case NF_KEY_AAAA:               // AAAA
                OutString += rCal.getDisplayString(
                        CalendarDisplayCode::LONG_DAY_NAME, nNatNum );
            break;
            case NF_KEY_NNNN:               // NNNN
            {
                OutString += rCal.getDisplayString(
                        CalendarDisplayCode::LONG_DAY_NAME, nNatNum );
                OutString += rLoc().getLongDateDayOfWeekSep();
            }
            break;
            case NF_KEY_WW :                // WW
            {
                sal_Int16 nVal = rCal.getValue( CalendarFieldIndex::WEEK_OF_YEAR );
                OutString += ImpIntToString( nIx, nVal );
            }
            break;
            case NF_KEY_G:                  // G
                ImpAppendEraG( OutString, rCal, nNatNum );
            break;
            case NF_KEY_GG:                 // GG
                OutString += rCal.getDisplayString(
                        CalendarDisplayCode::SHORT_ERA, nNatNum );
            break;
            case NF_KEY_GGG:                // GGG
                OutString += rCal.getDisplayString(
                        CalendarDisplayCode::LONG_ERA, nNatNum );
            break;
            case NF_KEY_RR:                 // RR => GGGEE
                OutString += rCal.getDisplayString(
                        CalendarDisplayCode::LONG_YEAR_AND_ERA, nNatNum );
            break;
        }
    }
    if ( aOrgCalendar.Len() )
        rCal.loadCalendar( aOrgCalendar, rLoc().getLocale() );  // restore calendar
    return bRes;
}

sal_Bool SvNumberformat::ImpGetNumberOutput(double fNumber,
                                     sal_uInt16 nIx,
                                     String& OutString)
{
    sal_Bool bRes = sal_False;
    sal_Bool bSign;
    if (fNumber < 0.0)
    {
        if (nIx == 0)                       // nicht in hinteren
            bSign = sal_True;                   // Formaten
        else
            bSign = sal_False;
        fNumber = -fNumber;
    }
    else
    {
        bSign = sal_False;
        if ( ::rtl::math::isSignBitSet( fNumber ) )
            fNumber = -fNumber;     // yes, -0.0 is possible, eliminate '-'
    }
    const ImpSvNumberformatInfo& rInfo = NumFor[nIx].Info();
    if (rInfo.eScannedType == NUMBERFORMAT_PERCENT)
    {
        if (fNumber < _D_MAX_D_BY_100)
            fNumber *= 100.0;
        else
        {
            OutString = rScan.GetErrorString();
            return sal_False;
        }
    }
    sal_uInt16 i, j;
    xub_StrLen k;
    String sStr;
    long nPrecExp;
    sal_Bool bInteger = sal_False;
    if ( rInfo.nThousand != FLAG_STANDARD_IN_FORMAT )
    {   // special formatting only if no GENERAL keyword in format code
        const sal_uInt16 nThousand = rInfo.nThousand;
        for (i = 0; i < nThousand; i++)
        {
           if (fNumber > _D_MIN_M_BY_1000)
               fNumber /= 1000.0;
           else
               fNumber = 0.0;
        }
        if (fNumber > 0.0)
            nPrecExp = GetPrecExp( fNumber );
        else
            nPrecExp = 0;
        if (rInfo.nCntPost)    // NachkommaStellen
        {
            if (rInfo.nCntPost + nPrecExp > 15 && nPrecExp < 15)
            {
                sStr = ::rtl::math::doubleToUString( fNumber,
                        rtl_math_StringFormat_F, 15-nPrecExp, '.');
                for (long l = 15-nPrecExp; l < (long) rInfo.nCntPost; l++)
                    sStr += '0';
            }
            else
                sStr = ::rtl::math::doubleToUString( fNumber,
                        rtl_math_StringFormat_F, rInfo.nCntPost, '.' );
            sStr.EraseLeadingChars('0');        // fuehrende Nullen weg
        }
        else if (fNumber == 0.0)            // Null
        {
            // nothing to be done here, keep empty string sStr,
            // ImpNumberFillWithThousands does the rest
        }
        else                                // Integer
        {
            sStr = ::rtl::math::doubleToUString( fNumber,
                    rtl_math_StringFormat_F, 0, '.');
            sStr.EraseLeadingChars('0');        // fuehrende Nullen weg
        }
        xub_StrLen nPoint = sStr.Search( '.' );
        if ( nPoint != STRING_NOTFOUND )
        {
            register const sal_Unicode* p = sStr.GetBuffer() + nPoint;
            while ( *++p == '0' )
                ;
            if ( !*p )
                bInteger = sal_True;
            sStr.Erase( nPoint, 1 );            //  . herausnehmen
        }
        if (bSign &&
            (sStr.Len() == 0 || sStr.GetTokenCount('0') == sStr.Len()+1))   // nur 00000
            bSign = sal_False;              // nicht -0.00
    }                                   // End of != FLAG_STANDARD_IN_FORMAT

                                        // von hinten nach vorn
                                        // editieren:
    k = sStr.Len();                     // hinter letzter Ziffer
    j = NumFor[nIx].GetCount()-1;        // letztes Symbol
                                        // Nachkommastellen:
    if (rInfo.nCntPost > 0)
    {
        sal_Bool bTrailing = sal_True;          // ob Endnullen?
        sal_Bool bFilled = sal_False;           // ob aufgefuellt wurde ?
        short nType;
        while (j > 0 &&                 // rueckwaerts
           (nType = rInfo.nTypeArray[j]) != NF_SYMBOLTYPE_DECSEP)
        {
            switch ( nType )
            {
                case NF_SYMBOLTYPE_STAR:
                    if( bStarFlag )
                    {
                        sStr.Insert( (sal_Unicode) 0x1B, k /*++*/ );
                        sStr.Insert(rInfo.sStrArray[j].GetChar(1),k);
                        bRes = sal_True;
                    }
                    break;
                case NF_SYMBOLTYPE_BLANK:
                    /*k = */ InsertBlanks( sStr,k,rInfo.sStrArray[j].GetChar(1) );
                    break;
                case NF_SYMBOLTYPE_STRING:
                case NF_SYMBOLTYPE_CURRENCY:
                case NF_SYMBOLTYPE_PERCENT:
                    sStr.Insert(rInfo.sStrArray[j],k);
                    break;
                case NF_SYMBOLTYPE_THSEP:
                    if (rInfo.nThousand == 0)
                        sStr.Insert(rInfo.sStrArray[j],k);
                break;
                case NF_SYMBOLTYPE_DIGIT:
                {
                    const String& rStr = rInfo.sStrArray[j];
                    const sal_Unicode* p1 = rStr.GetBuffer();
                    register const sal_Unicode* p = p1 + rStr.Len();
                    while ( p1 < p-- )
                    {
                        const sal_Unicode c = *p;
                        k--;
                        if ( sStr.GetChar(k) != '0' )
                            bTrailing = sal_False;
                        if (bTrailing)
                        {
                            if ( c == '0' )
                                bFilled = sal_True;
                            else if ( c == '-' )
                            {
                                if ( bInteger )
                                    sStr.SetChar( k, '-' );
                                bFilled = sal_True;
                            }
                            else if ( c == '?' )
                            {
                                sStr.SetChar( k, ' ' );
                                bFilled = sal_True;
                            }
                            else if ( !bFilled )    // #
                                sStr.Erase(k,1);
                        }
                    }                           // of for
                }                               // of case digi
                break;
                case NF_KEY_CCC:                // CCC-Waehrung
                    sStr.Insert(rScan.GetCurAbbrev(), k);
                break;
                case NF_KEY_GENERAL:            // Standard im String
                {
                    String sNum;
                    ImpGetOutputStandard(fNumber, sNum);
                    sNum.EraseLeadingChars('-');
                    sStr.Insert(sNum, k);
                }
                break;
                default:
                break;
            }                                   // of switch
            j--;
        }                                       // of while
    }                                           // of Nachkomma

    bRes |= ImpNumberFillWithThousands(sStr, fNumber, k, j, nIx, // ggfs Auffuellen mit .
                            rInfo.nCntPre);
    if ( rInfo.nCntPost > 0 )
    {
        const String& rDecSep = GetFormatter().GetNumDecimalSep();
        xub_StrLen nLen = rDecSep.Len();
        if ( sStr.Len() > nLen && sStr.Equals( rDecSep, sStr.Len() - nLen, nLen ) )
            sStr.Erase( sStr.Len() - nLen );        // no decimals => strip DecSep
    }
    if (bSign)
        sStr.Insert('-',0);
    ImpTransliterate( sStr, NumFor[nIx].GetNatNum() );
    OutString = sStr;
    return bRes;
}

sal_Bool SvNumberformat::ImpNumberFillWithThousands(
                                String& sStr,       // number string
                                double& rNumber,    // number
                                xub_StrLen k,       // position within string
                                sal_uInt16 j,           // symbol index within format code
                                sal_uInt16 nIx,         // subformat index
                                sal_uInt16 nDigCnt)     // count of integer digits in format
{
    sal_Bool bRes = sal_False;
    xub_StrLen nLeadingStringChars = 0; // inserted StringChars before number
    xub_StrLen nDigitCount = 0;         // count of integer digits from the right
    sal_Bool bStop = sal_False;
    const ImpSvNumberformatInfo& rInfo = NumFor[nIx].Info();
    // no normal thousands separators if number divided by thousands
    sal_Bool bDoThousands = (rInfo.nThousand == 0);
    utl::DigitGroupingIterator aGrouping(
            GetFormatter().GetLocaleData()->getDigitGrouping());
    while (!bStop)                                      // backwards
    {
        if (j == 0)
            bStop = sal_True;
        switch (rInfo.nTypeArray[j])
        {
            case NF_SYMBOLTYPE_DECSEP:
                aGrouping.reset();
                // fall thru
            case NF_SYMBOLTYPE_STRING:
            case NF_SYMBOLTYPE_CURRENCY:
            case NF_SYMBOLTYPE_PERCENT:
                sStr.Insert(rInfo.sStrArray[j],k);
                if ( k == 0 )
                    nLeadingStringChars =
                        nLeadingStringChars + rInfo.sStrArray[j].Len();
            break;
            case NF_SYMBOLTYPE_STAR:
                if( bStarFlag )
                {
                    sStr.Insert( (sal_Unicode) 0x1B, k/*++*/ );
                    sStr.Insert(rInfo.sStrArray[j].GetChar(1),k);
                    bRes = sal_True;
                }
                break;
            case NF_SYMBOLTYPE_BLANK:
                /*k = */ InsertBlanks( sStr,k,rInfo.sStrArray[j].GetChar(1) );
                break;
            case NF_SYMBOLTYPE_THSEP:
            {
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
                    bDoThousands = ((j == 0) ||
                            (rInfo.nTypeArray[j-1] != NF_SYMBOLTYPE_DIGIT &&
                             rInfo.nTypeArray[j-1] != NF_SYMBOLTYPE_THSEP) ||
                            (rInfo.nTypeArray[j+1] == NF_SYMBOLTYPE_DIGIT));
                if ( bDoThousands )
                {
                    if (k > 0)
                        sStr.Insert(rInfo.sStrArray[j],k);
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
                            const String& rStr = rInfo.sStrArray[j-1];
                            xub_StrLen nLen = rStr.Len();
                            if (nLen)
                                cLeader = rStr.GetChar(nLen-1);
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
                                sStr.Insert(' ',k);
                                break;
                            default:
                                sStr.Insert(rInfo.sStrArray[j],k);
                        }
                    }
                    aGrouping.advance();
                }
            }
            break;
            case NF_SYMBOLTYPE_DIGIT:
            {
                const String& rStr = rInfo.sStrArray[j];
                const sal_Unicode* p1 = rStr.GetBuffer();
                register const sal_Unicode* p = p1 + rStr.Len();
                while ( p1 < p-- )
                {
                    nDigitCount++;
                    if (k > 0)
                        k--;
                    else
                    {
                        switch (*p)
                        {
                            case '0':
                                sStr.Insert('0',0);
                                break;
                            case '?':
                                sStr.Insert(' ',0);
                                break;
                        }
                    }
                    if (nDigitCount == nDigCnt && k > 0)
                    {   // more digits than specified
                        ImpDigitFill(sStr, 0, k, nIx, nDigitCount, aGrouping);
                    }
                }
            }
            break;
            case NF_KEY_CCC:                        // CCC currency
                sStr.Insert(rScan.GetCurAbbrev(), k);
            break;
            case NF_KEY_GENERAL:                    // "General" in string
            {
                String sNum;
                ImpGetOutputStandard(rNumber, sNum);
                sNum.EraseLeadingChars('-');
                sStr.Insert(sNum, k);
            }
            break;

            default:
            break;
        } // switch
        j--;        // next format code string
    } // while
    k = k + nLeadingStringChars;    // MSC converts += to int and then warns, so ...
    if (k > nLeadingStringChars)
        ImpDigitFill(sStr, nLeadingStringChars, k, nIx, nDigitCount, aGrouping);
    return bRes;
}

void SvNumberformat::ImpDigitFill(
        String& sStr,                   // number string
        xub_StrLen nStart,              // start of digits
        xub_StrLen& k,                  // position within string
        sal_uInt16 nIx,                     // subformat index
        xub_StrLen & nDigitCount,       // count of integer digits from the right so far
        utl::DigitGroupingIterator & rGrouping )    // current grouping
{
    if (NumFor[nIx].Info().bThousand)               // only if grouping
    {                                               // fill in separators
        const String& rThousandSep = GetFormatter().GetNumThousandSep();
        while (k > nStart)
        {
            if (nDigitCount == rGrouping.getPos())
            {
                sStr.Insert( rThousandSep, k );
                rGrouping.advance();
            }
            nDigitCount++;
            k--;
        }
    }
    else                                            // simply skip
        k = nStart;
}

sal_Bool SvNumberformat::ImpNumberFill( String& sStr,       // number string
                                double& rNumber,        // number for "General" format
                                xub_StrLen& k,          // position within string
                                sal_uInt16& j,              // symbol index within format code
                                sal_uInt16 nIx,             // subformat index
                                short eSymbolType )     // type of stop condition
{
    sal_Bool bRes = sal_False;
    k = sStr.Len();                         // behind last digit
    const ImpSvNumberformatInfo& rInfo = NumFor[nIx].Info();
    // no normal thousands separators if number divided by thousands
    sal_Bool bDoThousands = (rInfo.nThousand == 0);
    short nType;
    while (j > 0 && (nType = rInfo.nTypeArray[j]) != eSymbolType )
    {                                       // rueckwaerts:
        switch ( nType )
        {
            case NF_SYMBOLTYPE_STAR:
                if( bStarFlag )
                {
                    sStr.Insert( sal_Unicode(0x1B), k++ );
                    sStr.Insert(rInfo.sStrArray[j].GetChar(1),k);
                    bRes = sal_True;
                }
                break;
            case NF_SYMBOLTYPE_BLANK:
                k = InsertBlanks( sStr,k,rInfo.sStrArray[j].GetChar(1) );
                break;
            case NF_SYMBOLTYPE_THSEP:
            {
                // Same as in ImpNumberFillWithThousands() above, do not insert
                // if divided and regex [0#,],[^0#] and no other digit symbol
                // follows (which was already detected during scan of format
                // code, otherwise there would be no division), else do insert.
                if ( !bDoThousands && j < NumFor[nIx].GetCount()-1 )
                    bDoThousands = ((j == 0) ||
                            (rInfo.nTypeArray[j-1] != NF_SYMBOLTYPE_DIGIT &&
                             rInfo.nTypeArray[j-1] != NF_SYMBOLTYPE_THSEP) ||
                            (rInfo.nTypeArray[j+1] == NF_SYMBOLTYPE_DIGIT));
                if ( bDoThousands && k > 0 )
                {
                    sStr.Insert(rInfo.sStrArray[j],k);
                }
            }
            break;
            case NF_SYMBOLTYPE_DIGIT:
            {
                const String& rStr = rInfo.sStrArray[j];
                const sal_Unicode* p1 = rStr.GetBuffer();
                register const sal_Unicode* p = p1 + rStr.Len();
                while ( p1 < p-- )
                {
                    if (k > 0)
                        k--;
                    else
                    {
                        switch (*p)
                        {
                            case '0':
                                sStr.Insert('0',0);
                                break;
                            case '?':
                                sStr.Insert(' ',0);
                                break;
                        }
                    }
                }
            }
            break;
            case NF_KEY_CCC:                // CCC-Waehrung
                sStr.Insert(rScan.GetCurAbbrev(), k);
            break;
            case NF_KEY_GENERAL:            // Standard im String
            {
                String sNum;
                ImpGetOutputStandard(rNumber, sNum);
                sNum.EraseLeadingChars('-');    // Vorzeichen weg!!
                sStr.Insert(sNum, k);
            }
            break;
            case NF_SYMBOLTYPE_FRAC_FDIV:       // Do Nothing
                break;

            default:
                sStr.Insert(rInfo.sStrArray[j],k);
            break;
        }                                       // of switch
        j--;                                    // naechster String
    }                                           // of while
    return bRes;
}

void SvNumberformat::GetFormatSpecialInfo(sal_Bool& bThousand,
                                          sal_Bool& IsRed,
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
        IsRed = sal_True;
    else
        IsRed = sal_False;
}

void SvNumberformat::GetNumForInfo( sal_uInt16 nNumFor, short& rScannedType,
                    sal_Bool& bThousand, sal_uInt16& nPrecision, sal_uInt16& nAnzLeading ) const
{
    // take info from a specified sub-format (for XML export)

    if ( nNumFor > 3 )
        return;             // invalid

    const ImpSvNumberformatInfo& rInfo = NumFor[nNumFor].Info();
    rScannedType = rInfo.eScannedType;
    bThousand = rInfo.bThousand;
    nPrecision = rInfo.nCntPost;
    if (bStandard && rInfo.eScannedType == NUMBERFORMAT_NUMBER)
                                                        // StandardFormat
        nAnzLeading = 1;
    else
    {
        nAnzLeading = 0;
        sal_Bool bStop = sal_False;
        sal_uInt16 i = 0;
        const sal_uInt16 nAnz = NumFor[nNumFor].GetCount();
        while (!bStop && i < nAnz)
        {
            short nType = rInfo.nTypeArray[i];
            if ( nType == NF_SYMBOLTYPE_DIGIT)
            {
                register const sal_Unicode* p = rInfo.sStrArray[i].GetBuffer();
                while ( *p == '#' )
                    p++;
                while ( *p++ == '0' )
                    nAnzLeading++;
            }
            else if (nType == NF_SYMBOLTYPE_DECSEP || nType == NF_SYMBOLTYPE_EXP)
                bStop = sal_True;
            i++;
        }
    }
}

const String* SvNumberformat::GetNumForString( sal_uInt16 nNumFor, sal_uInt16 nPos,
            sal_Bool bString /* = sal_False */ ) const
{
    if ( nNumFor > 3 )
        return NULL;
    sal_uInt16 nAnz = NumFor[nNumFor].GetCount();
    if ( !nAnz )
        return NULL;
    if ( nPos == 0xFFFF )
    {
        nPos = nAnz - 1;
        if ( bString )
        {   // rueckwaerts
            short* pType = NumFor[nNumFor].Info().nTypeArray + nPos;
            while ( nPos > 0 && (*pType != NF_SYMBOLTYPE_STRING) &&
                    (*pType != NF_SYMBOLTYPE_CURRENCY) )
            {
                pType--;
                nPos--;
            }
            if ( (*pType != NF_SYMBOLTYPE_STRING) && (*pType != NF_SYMBOLTYPE_CURRENCY) )
                return NULL;
        }
    }
    else if ( nPos > nAnz - 1 )
        return NULL;
    else if ( bString )
    {   // vorwaerts
        short* pType = NumFor[nNumFor].Info().nTypeArray + nPos;
        while ( nPos < nAnz && (*pType != NF_SYMBOLTYPE_STRING) &&
                (*pType != NF_SYMBOLTYPE_CURRENCY) )
        {
            pType++;
            nPos++;
        }
        if ( nPos >= nAnz || ((*pType != NF_SYMBOLTYPE_STRING) &&
                    (*pType != NF_SYMBOLTYPE_CURRENCY)) )
            return NULL;
    }
    return &NumFor[nNumFor].Info().sStrArray[nPos];
}

short SvNumberformat::GetNumForType( sal_uInt16 nNumFor, sal_uInt16 nPos,
            sal_Bool bString /* = sal_False */ ) const
{
    if ( nNumFor > 3 )
        return 0;
    sal_uInt16 nAnz = NumFor[nNumFor].GetCount();
    if ( !nAnz )
        return 0;
    if ( nPos == 0xFFFF )
    {
        nPos = nAnz - 1;
        if ( bString )
        {   // rueckwaerts
            short* pType = NumFor[nNumFor].Info().nTypeArray + nPos;
            while ( nPos > 0 && (*pType != NF_SYMBOLTYPE_STRING) &&
                    (*pType != NF_SYMBOLTYPE_CURRENCY) )
            {
                pType--;
                nPos--;
            }
            if ( (*pType != NF_SYMBOLTYPE_STRING) && (*pType != NF_SYMBOLTYPE_CURRENCY) )
                return 0;
        }
    }
    else if ( nPos > nAnz - 1 )
        return 0;
    else if ( bString )
    {   // vorwaerts
        short* pType = NumFor[nNumFor].Info().nTypeArray + nPos;
        while ( nPos < nAnz && (*pType != NF_SYMBOLTYPE_STRING) &&
                (*pType != NF_SYMBOLTYPE_CURRENCY) )
        {
            pType++;
            nPos++;
        }
        if ( (*pType != NF_SYMBOLTYPE_STRING) && (*pType != NF_SYMBOLTYPE_CURRENCY) )
            return 0;
    }
    return NumFor[nNumFor].Info().nTypeArray[nPos];
}

sal_Bool SvNumberformat::IsNegativeWithoutSign() const
{
    if ( IsNegativeRealNegative() )
    {
        const String* pStr = GetNumForString( 1, 0, sal_True );
        if ( pStr )
            return !HasStringNegativeSign( *pStr );
    }
    return sal_False;
}

BOOL SvNumberformat::IsNegativeInBracket() const
{
    USHORT nAnz = NumFor[1].GetCount();
    if (!nAnz)
        return FALSE;

    String *tmpStr = NumFor[1].Info().sStrArray;
    return (tmpStr[0] == '(' && tmpStr[nAnz-1] == ')' );
}

BOOL SvNumberformat::HasPositiveBracketPlaceholder() const
{
    USHORT nAnz = NumFor[0].GetCount();
    String *tmpStr = NumFor[0].Info().sStrArray;
    return (tmpStr[nAnz-1].EqualsAscii( "_)" ));
}

DateFormat SvNumberformat::GetDateOrder() const
{
    if ( (eType & NUMBERFORMAT_DATE) == NUMBERFORMAT_DATE )
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
       OSL_FAIL( "SvNumberformat::GetDateOrder: no date" );
    }
    return rLoc().getDateFormat();
}

sal_uInt32 SvNumberformat::GetExactDateOrder() const
{
    sal_uInt32 nRet = 0;
    if ( (eType & NUMBERFORMAT_DATE) != NUMBERFORMAT_DATE )
    {
        OSL_FAIL( "SvNumberformat::GetExactDateOrder: no date" );
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
        return NULL;

    return NumFor[nNumFor].GetColor();
}

void lcl_SvNumberformat_AddLimitStringImpl( String& rStr,
            SvNumberformatLimitOps eOp, double fLimit, const String& rDecSep )
{
    if ( eOp != NUMBERFORMAT_OP_NO )
    {
        switch ( eOp )
        {
            case NUMBERFORMAT_OP_EQ :
                rStr.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "[=" ) );
            break;
            case NUMBERFORMAT_OP_NE :
                rStr.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "[<>" ) );
            break;
            case NUMBERFORMAT_OP_LT :
                rStr.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "[<" ) );
            break;
            case NUMBERFORMAT_OP_LE :
                rStr.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "[<=" ) );
            break;
            case NUMBERFORMAT_OP_GT :
                rStr.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "[>" ) );
            break;
            case NUMBERFORMAT_OP_GE :
                rStr.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "[>=" ) );
            break;
            default:
                OSL_FAIL( "unsupported number format" );
                break;
        }
        rStr += String( ::rtl::math::doubleToUString( fLimit,
                rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max,
                rDecSep.GetChar(0), sal_True));
        rStr += ']';
    }
}

String SvNumberformat::GetMappedFormatstring(
        const NfKeywordTable& rKeywords, const LocaleDataWrapper& rLocWrp,
        sal_Bool bDontQuote ) const
{
    String aStr;
    sal_Bool bDefault[4];
    // 1 subformat matches all if no condition specified,
    bDefault[0] = ( NumFor[1].GetCount() == 0 && eOp1 == NUMBERFORMAT_OP_NO );
    // with 2 subformats [>=0];[<0] is implied if no condition specified
    bDefault[1] = ( !bDefault[0] && NumFor[2].GetCount() == 0 &&
        eOp1 == NUMBERFORMAT_OP_GE && fLimit1 == 0.0 &&
        eOp2 == NUMBERFORMAT_OP_NO && fLimit2 == 0.0 );
    // with 3 or more subformats [>0];[<0];[=0] is implied if no condition specified,
    // note that subformats may be empty (;;;) and NumFor[2].GetnAnz()>0 is not checked.
    bDefault[2] = ( !bDefault[0] && !bDefault[1] &&
        eOp1 == NUMBERFORMAT_OP_GT && fLimit1 == 0.0 &&
        eOp2 == NUMBERFORMAT_OP_LT && fLimit2 == 0.0 );
    sal_Bool bDefaults = bDefault[0] || bDefault[1] || bDefault[2];
    // from now on bDefault[] values are used to append empty subformats at the end
    bDefault[3] = sal_False;
    if ( !bDefaults )
    {   // conditions specified
        if ( eOp1 != NUMBERFORMAT_OP_NO && eOp2 == NUMBERFORMAT_OP_NO )
            bDefault[0] = bDefault[1] = sal_True;                               // [];x
        else if ( eOp1 != NUMBERFORMAT_OP_NO && eOp2 != NUMBERFORMAT_OP_NO &&
                NumFor[2].GetCount() == 0 )
            bDefault[0] = bDefault[1] = bDefault[2] = bDefault[3] = sal_True;   // [];[];;
        // nothing to do if conditions specified for every subformat
    }
    else if ( bDefault[0] )
        bDefault[0] = sal_False;    // a single unconditional subformat is never delimited
    else
    {
        if ( bDefault[2] && NumFor[2].GetCount() == 0 && NumFor[1].GetCount() > 0 )
            bDefault[3] = sal_True;     // special cases x;x;; and ;x;;
        for ( int i=0; i<3 && !bDefault[i]; ++i )
            bDefault[i] = sal_True;
    }
    int nSem = 0;       // needed ';' delimiters
    int nSub = 0;       // subformats delimited so far
    for ( int n=0; n<4; n++ )
    {
        if ( n > 0 )
            nSem++;

        String aPrefix;

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

        const String& rColorName = NumFor[n].GetColorName();
        if ( rColorName.Len() )
        {
            const String* pKey = rScan.GetKeywords() + NF_KEY_FIRSTCOLOR;
            for ( int j=NF_KEY_FIRSTCOLOR; j<=NF_KEY_LASTCOLOR; j++, pKey++ )
            {
                if ( *pKey == rColorName )
                {
                    aPrefix += '[';
                    aPrefix += rKeywords[j];
                    aPrefix += ']';
                    break;  // for
                }
            }
        }

        const SvNumberNatNum& rNum = NumFor[n].GetNatNum();
        // The Thai T NatNum modifier during Xcl export.
        if (rNum.IsSet() && rNum.GetNatNum() == 1 &&
                rKeywords[NF_KEY_THAI_T].EqualsAscii( "T") &&
                MsLangId::getRealLanguage( rNum.GetLang()) ==
                LANGUAGE_THAI)
        {
            aPrefix += 't';     // must be lowercase, otherwise taken as literal
        }

        sal_uInt16 nAnz = NumFor[n].GetCount();
        if ( nSem && (nAnz || aPrefix.Len()) )
        {
            for ( ; nSem; --nSem )
                aStr += ';';
            for ( ; nSub <= n; ++nSub )
                bDefault[nSub] = sal_False;
        }

        if ( aPrefix.Len() )
            aStr += aPrefix;

        if ( nAnz )
        {
            const short* pType = NumFor[n].Info().nTypeArray;
            const String* pStr = NumFor[n].Info().sStrArray;
            for ( sal_uInt16 j=0; j<nAnz; j++ )
            {
                if ( 0 <= pType[j] && pType[j] < NF_KEYWORD_ENTRIES_COUNT )
                {
                    aStr += rKeywords[pType[j]];
                    if( NF_KEY_NNNN == pType[j] )
                        aStr += rLocWrp.getLongDateDayOfWeekSep();
                }
                else
                {
                    switch ( pType[j] )
                    {
                        case NF_SYMBOLTYPE_DECSEP :
                            aStr += rLocWrp.getNumDecimalSep();
                        break;
                        case NF_SYMBOLTYPE_THSEP :
                            aStr += rLocWrp.getNumThousandSep();
                        break;
                        case NF_SYMBOLTYPE_DATESEP :
                            aStr += rLocWrp.getDateSep();
                        break;
                        case NF_SYMBOLTYPE_TIMESEP :
                            aStr += rLocWrp.getTimeSep();
                        break;
                        case NF_SYMBOLTYPE_TIME100SECSEP :
                            aStr += rLocWrp.getTime100SecSep();
                        break;
                        case NF_SYMBOLTYPE_STRING :
                            if( bDontQuote )
                                aStr += pStr[j];
                            else if ( pStr[j].Len() == 1 )
                            {
                                aStr += '\\';
                                aStr += pStr[j];
                            }
                            else
                            {
                                aStr += '"';
                                aStr += pStr[j];
                                aStr += '"';
                            }
                            break;
                        default:
                            aStr += pStr[j];
                    }

                }
            }
        }
    }
    for ( ; nSub<4 && bDefault[nSub]; ++nSub )
    {   // append empty subformats
        aStr += ';';
    }
    return aStr;
}

String SvNumberformat::ImpGetNatNumString( const SvNumberNatNum& rNum,
        sal_Int32 nVal, sal_uInt16 nMinDigits ) const
{
    String aStr;
    if ( nMinDigits )
    {
        if ( nMinDigits == 2 )
        {   // speed up the most common case
            if ( 0 <= nVal && nVal < 10 )
            {
                sal_Unicode* p = aStr.AllocBuffer( 2 );
                *p++ = '0';
                *p = sal_Unicode( '0' + nVal );
            }
            else
                aStr = String::CreateFromInt32( nVal );
        }
        else
        {
            String aValStr( String::CreateFromInt32( nVal ) );
            if ( aValStr.Len() >= nMinDigits )
                aStr = aValStr;
            else
            {
                aStr.Fill( nMinDigits - aValStr.Len(), '0' );
                aStr += aValStr;
            }
        }
    }
    else
        aStr = String::CreateFromInt32( nVal );
    ImpTransliterate( aStr, rNum );
    return aStr;
}

void SvNumberformat::ImpTransliterateImpl( String& rStr,
        const SvNumberNatNum& rNum ) const
{
    com::sun::star::lang::Locale aLocale(
            MsLangId::convertLanguageToLocale( rNum.GetLang() ) );
    rStr = GetFormatter().GetNatNum()->getNativeNumberString( rStr,
            aLocale, rNum.GetNatNum() );
}

void SvNumberformat::GetNatNumXml(
        com::sun::star::i18n::NativeNumberXmlAttributes& rAttr,
        sal_uInt16 nNumFor ) const
{
    if ( nNumFor <= 3 )
    {
        const SvNumberNatNum& rNum = NumFor[nNumFor].GetNatNum();
        if ( rNum.IsSet() )
        {
            com::sun::star::lang::Locale aLocale(
                    MsLangId::convertLanguageToLocale( rNum.GetLang() ) );
            rAttr = GetFormatter().GetNatNum()->convertToXmlAttributes(
                    aLocale, rNum.GetNatNum() );
        }
        else
            rAttr = com::sun::star::i18n::NativeNumberXmlAttributes();
    }
    else
        rAttr = com::sun::star::i18n::NativeNumberXmlAttributes();
}

// static
sal_Bool SvNumberformat::HasStringNegativeSign( const String& rStr )
{
    // fuer Sign muss '-' am Anfang oder am Ende des TeilStrings sein (Blanks ignored)
    xub_StrLen nLen = rStr.Len();
    if ( !nLen )
        return sal_False;
    const sal_Unicode* const pBeg = rStr.GetBuffer();
    const sal_Unicode* const pEnd = pBeg + nLen;
    register const sal_Unicode* p = pBeg;
    do
    {   // Anfang
        if ( *p == '-' )
            return sal_True;
    } while ( *p == ' ' && ++p < pEnd );
    p = pEnd - 1;
    do
    {   // Ende
        if ( *p == '-' )
            return sal_True;
    } while ( *p == ' ' && pBeg < --p );
    return sal_False;
}

// static
void SvNumberformat::SetComment( const String& rStr, String& rFormat,
        String& rComment )
{
    if ( rComment.Len() )
    {   // alten Kommentar aus Formatstring loeschen
        //! nicht per EraseComment, der Kommentar muss matchen
        String aTmp( '{' );
        aTmp += ' ';
        aTmp += rComment;
        aTmp += ' ';
        aTmp += '}';
        xub_StrLen nCom = 0;
        do
        {
            nCom = rFormat.Search( aTmp, nCom );
        } while ( (nCom != STRING_NOTFOUND) && (nCom + aTmp.Len() != rFormat.Len()) );
        if ( nCom != STRING_NOTFOUND )
            rFormat.Erase( nCom );
    }
    if ( rStr.Len() )
    {   // neuen Kommentar setzen
        rFormat += '{';
        rFormat += ' ';
        rFormat += rStr;
        rFormat += ' ';
        rFormat += '}';
        rComment = rStr;
    }
}

// static
void SvNumberformat::EraseCommentBraces( String& rStr )
{
    xub_StrLen nLen = rStr.Len();
    if ( nLen && rStr.GetChar(0) == '{' )
    {
        rStr.Erase( 0, 1 );
        --nLen;
    }
    if ( nLen && rStr.GetChar(0) == ' ' )
    {
        rStr.Erase( 0, 1 );
        --nLen;
    }
    if ( nLen && rStr.GetChar( nLen-1 ) == '}' )
        rStr.Erase( --nLen, 1 );
    if ( nLen && rStr.GetChar( nLen-1 ) == ' ' )
        rStr.Erase( --nLen, 1 );
}

// static
void SvNumberformat::EraseComment( String& rStr )
{
    register const sal_Unicode* p = rStr.GetBuffer();
    sal_Bool bInString = sal_False;
    sal_Bool bEscaped = sal_False;
    sal_Bool bFound = sal_False;
    xub_StrLen nPos = 0;
    while ( !bFound && *p )
    {
        switch ( *p )
        {
            case '\\' :
                bEscaped = !bEscaped;
            break;
            case '\"' :
                if ( !bEscaped )
                    bInString = !bInString;
            break;
            case '{' :
                if ( !bEscaped && !bInString )
                {
                    bFound = sal_True;
                    nPos = sal::static_int_cast< xub_StrLen >(
                        p - rStr.GetBuffer());
                }
            break;
        }
        if ( bEscaped && *p != '\\' )
            bEscaped = sal_False;
        ++p;
    }
    if ( bFound )
        rStr.Erase( nPos );
}

// static
sal_Bool SvNumberformat::IsInQuote( const String& rStr, xub_StrLen nPos,
            sal_Unicode cQuote, sal_Unicode cEscIn, sal_Unicode cEscOut )
{
    xub_StrLen nLen = rStr.Len();
    if ( nPos >= nLen )
        return sal_False;
    register const sal_Unicode* p0 = rStr.GetBuffer();
    register const sal_Unicode* p = p0;
    register const sal_Unicode* p1 = p0 + nPos;
    sal_Bool bQuoted = sal_False;
    while ( p <= p1 )
    {
        if ( *p == cQuote )
        {
            if ( p == p0 )
                bQuoted = sal_True;
            else if ( bQuoted )
            {
                if ( *(p-1) != cEscIn )
                    bQuoted = sal_False;
            }
            else
            {
                if ( *(p-1) != cEscOut )
                    bQuoted = sal_True;
            }
        }
        p++;
    }
    return bQuoted;
}

// static
xub_StrLen SvNumberformat::GetQuoteEnd( const String& rStr, xub_StrLen nPos,
            sal_Unicode cQuote, sal_Unicode cEscIn, sal_Unicode cEscOut )
{
    xub_StrLen nLen = rStr.Len();
    if ( nPos >= nLen )
        return STRING_NOTFOUND;
    if ( !IsInQuote( rStr, nPos, cQuote, cEscIn, cEscOut ) )
    {
        if ( rStr.GetChar( nPos ) == cQuote )
            return nPos;        // schliessendes cQuote
        return STRING_NOTFOUND;
    }
    register const sal_Unicode* p0 = rStr.GetBuffer();
    register const sal_Unicode* p = p0 + nPos;
    register const sal_Unicode* p1 = p0 + nLen;
    while ( p < p1 )
    {
        if ( *p == cQuote && p > p0 && *(p-1) != cEscIn )
            return sal::static_int_cast< xub_StrLen >(p - p0);
        p++;
    }
    return nLen;        // String Ende
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
