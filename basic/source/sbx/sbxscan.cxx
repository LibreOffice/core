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

#include <tools/errcode.hxx>
#include <basic/sbx.hxx>
#include "sbxconv.hxx"

#include "unotools/syslocale.hxx"

#if defined ( UNX )
#include <stdlib.h>
#endif

#include <vcl/svapp.hxx>
#include <math.h>
#include <string.h>
#include <ctype.h>

#include "sbxres.hxx"
#include <basic/sbxbase.hxx>
#include <basic/sbxfac.hxx>
#include <basic/sbxform.hxx>
#include <svtools/svtools.hrc>

#include "basrid.hxx"
#include "runtime.hxx"

#include <rtl/strbuf.hxx>
#include <svl/zforlist.hxx>
#include <comphelper/processfactory.hxx>


void ImpGetIntntlSep( sal_Unicode& rcDecimalSep, sal_Unicode& rcThousandSep )
{
    SvtSysLocale aSysLocale;
    const LocaleDataWrapper& rData = aSysLocale.GetLocaleData();
    rcDecimalSep = rData.getNumDecimalSep()[0];
    rcThousandSep = rData.getNumThousandSep()[0];
}

// scanning a string according to BASIC-conventions
// but exponent may also be a D, so data type is SbxDOUBLED
// conversion error if data type is fixed and it doesn't fit

SbxError ImpScan( const ::rtl::OUString& rWSrc, double& nVal, SbxDataType& rType,
                  sal_uInt16* pLen, sal_Bool bAllowIntntl, sal_Bool bOnlyIntntl )
{
    ::rtl::OString aBStr( ::rtl::OUStringToOString( rWSrc, RTL_TEXTENCODING_ASCII_US ) );

    char cIntntlComma, cIntntl1000;
    char cNonIntntlComma = '.';

    sal_Unicode cDecimalSep, cThousandSep = 0;
    if( bAllowIntntl || bOnlyIntntl )
    {
        ImpGetIntntlSep( cDecimalSep, cThousandSep );
        cIntntlComma = (char)cDecimalSep;
        cIntntl1000 = (char)cThousandSep;
    }

    else
    {
        cIntntlComma = cNonIntntlComma;
        cIntntl1000 = cNonIntntlComma;
    }

    if( bOnlyIntntl )
    {
        cNonIntntlComma = cIntntlComma;
        cIntntl1000 = (char)cThousandSep;
    }

    const char* pStart = aBStr.getStr();
    const char* p = pStart;
    char buf[ 80 ], *q = buf;
    bool bRes = true;
    bool bMinus = false;
    nVal = 0;
    SbxDataType eScanType = SbxSINGLE;
    while( *p &&( *p == ' ' || *p == '\t' ) ) p++;
    if( *p == '-' )
        p++, bMinus = true;
    if( isdigit( *p ) ||( (*p == cNonIntntlComma || *p == cIntntlComma ||
            *p == cIntntl1000) && isdigit( *(p+1 ) ) ) )
    {
        short exp = 0;
        short comma = 0;
        short ndig = 0;
        short ncdig = 0;    // number of digits after decimal point
        rtl::OStringBuffer aSearchStr(RTL_CONSTASCII_STRINGPARAM("0123456789DEde"));
        aSearchStr.append(cNonIntntlComma);
        if( cIntntlComma != cNonIntntlComma )
            aSearchStr.append(cIntntlComma);
        if( bOnlyIntntl )
            aSearchStr.append(cIntntl1000);
        const char* pSearchStr = aSearchStr.getStr();
        while( strchr( pSearchStr, *p ) && *p )
        {
            if( bOnlyIntntl && *p == cIntntl1000 )
            {
                p++;
                continue;
            }

            if( *p == cNonIntntlComma || *p == cIntntlComma )
            {
                // always insert '.' so that atof works
                p++;
                if( ++comma > 1 )
                    continue;
                else
                    *q++ = '.';
            }
            else if( strchr( "DdEe", *p ) )
            {
                if( ++exp > 1 )
                {
                    p++; continue;
                }
                if( toupper( *p ) == 'D' )
                    eScanType = SbxDOUBLE;
                *q++ = 'E'; p++;

                if( *p == '+' )
                    p++;
                else
                if( *p == '-' )
                    *q++ = *p++;
            }
            else
            {
                *q++ = *p++;
                if( comma && !exp ) ncdig++;
            }
            if( !exp ) ndig++;
        }
        *q = 0;

        if( comma > 1 || exp > 1 )
            bRes = false;

        if( !comma && !exp )
        {
            if( nVal >= SbxMININT && nVal <= SbxMAXINT )
                eScanType = SbxINTEGER;
            else if( nVal >= SbxMINLNG && nVal <= SbxMAXLNG )
                eScanType = SbxLONG;
        }

        nVal = atof( buf );
        ndig = ndig - comma;
        // too many numbers for SINGLE?
        if( ndig > 15 || ncdig > 6 )
            eScanType = SbxDOUBLE;

        // type detection?
        if( strchr( "%!&#", *p ) && *p ) p++;
    }
    // hex/octal number? read in and convert:
    else if( *p == '&' )
    {
        p++;
        eScanType = SbxLONG;
        const char *cmp = "0123456789ABCDEF";
        char base = 16;
        char ndig = 8;
        char xch  = *p++;
        switch( toupper( xch ) )
        {
            case 'O': cmp = "01234567"; base = 8; ndig = 11; break;
            case 'H': break;
            default : bRes = false;
        }
        long l = 0;
        int i;
        while( isalnum( *p ) )
        {
            char ch = sal::static_int_cast< char >( toupper( *p ) );
            p++;
            if( strchr( cmp, ch ) ) *q++ = ch;
            else bRes = false;
        }
        *q = 0;
        for( q = buf; *q; q++ )
        {
            i =( *q & 0xFF ) - '0';
            if( i > 9 ) i -= 7;
            l =( l * base ) + i;
            if( !ndig-- )
                bRes = false;
        }
        if( *p == '&' ) p++;
        nVal = (double) l;
        if( l >= SbxMININT && l <= SbxMAXINT )
            eScanType = SbxINTEGER;
    }
#ifndef DISABLE_SCRIPTING
    else if ( SbiRuntime::isVBAEnabled() )
    {
        OSL_TRACE("Reporting error converting");
        return SbxERR_CONVERSION;
    }
#endif
    if( pLen )
        *pLen = (sal_uInt16) ( p - pStart );
    if( !bRes )
        return SbxERR_CONVERSION;
    if( bMinus )
        nVal = -nVal;
    rType = eScanType;
    return SbxERR_OK;
}

// port for CDbl in the Basic
SbxError SbxValue::ScanNumIntnl( const String& rSrc, double& nVal, sal_Bool bSingle )
{
    SbxDataType t;
    sal_uInt16 nLen = 0;
    SbxError nRetError = ImpScan( rSrc, nVal, t, &nLen,
        /*bAllowIntntl*/sal_False, /*bOnlyIntntl*/sal_True );
    // read completely?
    if( nRetError == SbxERR_OK && nLen != rSrc.Len() )
        nRetError = SbxERR_CONVERSION;

    if( bSingle )
    {
        SbxValues aValues( nVal );
        nVal = (double)ImpGetSingle( &aValues );    // here error at overflow
    }
    return nRetError;
}


static double roundArray[] = {
    5.0e+0, 0.5e+0, 0.5e-1, 0.5e-2, 0.5e-3, 0.5e-4, 0.5e-5, 0.5e-6, 0.5e-7,
    0.5e-8, 0.5e-9, 0.5e-10,0.5e-11,0.5e-12,0.5e-13,0.5e-14,0.5e-15 };

/***************************************************************************
|*
|*  void myftoa( double, char *, short, short, bool, bool )
|*
|*  description:        conversion double --> ASCII
|*  parameters:         double              the number
|*                      char *              target buffer
|*                      short               number of positions after decimal point
|*                      short               range of the exponent ( 0=no E )
|*                      bool                true: with 1000-separators
|*                      bool                true: output without formatting
|*
***************************************************************************/

static void myftoa( double nNum, char * pBuf, short nPrec, short nExpWidth,
                    bool bPt, bool bFix, sal_Unicode cForceThousandSep = 0 )
{

    short nExp = 0;
    short nDig = nPrec + 1;
    short nDec;                         // number of positions before decimal point
    register int i;

    sal_Unicode cDecimalSep, cThousandSep;
    ImpGetIntntlSep( cDecimalSep, cThousandSep );
    if( cForceThousandSep )
        cThousandSep = cForceThousandSep;

    // compute exponent
    nExp = 0;
    if( nNum > 0.0 )
    {
        while( nNum <   1.0 ) nNum *= 10.0, nExp--;
        while( nNum >= 10.0 ) nNum /= 10.0, nExp++;
    }
    if( !bFix && !nExpWidth )
        nDig = nDig + nExp;
    else if( bFix && !nPrec )
        nDig = nExp + 1;

    // round number
    if( (nNum += roundArray [( nDig > 16 ) ? 16 : nDig] ) >= 10.0 )
    {
        nNum = 1.0;
        ++nExp;
        if( !nExpWidth ) ++nDig;
    }

    // determine positions before decimal point
    if( !nExpWidth )
    {
        if( nExp < 0 )
        {
            // #41691: also a 0 at bFix
            *pBuf++ = '0';
            if( nPrec ) *pBuf++ = (char)cDecimalSep;
            i = -nExp - 1;
            if( nDig <= 0 ) i = nPrec;
            while( i-- )    *pBuf++ = '0';
            nDec = 0;
        }
        else
            nDec = nExp+1;
    }
    else
        nDec = 1;

    // output number
    if( nDig > 0 )
    {
        register int digit;
        for( i = 0 ; ; ++i )
        {
            if( i < 16 )
            {
                digit = (int) nNum;
                *pBuf++ = sal::static_int_cast< char >(digit + '0');
                nNum =( nNum - digit ) * 10.0;
            } else
                *pBuf++ = '0';
            if( --nDig == 0 ) break;
            if( nDec )
            {
                nDec--;
                if( !nDec )
                    *pBuf++ = (char)cDecimalSep;
                else if( !(nDec % 3 ) && bPt )
                    *pBuf++ = (char)cThousandSep;
            }
        }
    }

    // output exponent
    if( nExpWidth )
    {
        if( nExpWidth < 3 ) nExpWidth = 3;
        nExpWidth -= 2;
        *pBuf++ = 'E';
        *pBuf++ =( nExp < 0 ) ?( (nExp = -nExp ), '-' ) : '+';
        while( nExpWidth > 3 ) *pBuf++ = '0', nExpWidth--;
        if( nExp >= 100 || nExpWidth == 3 )
        {
            *pBuf++ = sal::static_int_cast< char >(nExp/100 + '0');
            nExp %= 100;
        }
        if( nExp/10 || nExpWidth >= 2 )
            *pBuf++ = sal::static_int_cast< char >(nExp/10 + '0');
        *pBuf++ = sal::static_int_cast< char >(nExp%10 + '0');
    }
    *pBuf = 0;
}

// The number is prepared unformattedly with the given number of
// NK-positions. A leading minus is added if applicable.
// This routine is public because it's also used by the Put-functions
// in the class SbxImpSTRING.

#ifdef _MSC_VER
#pragma optimize( "", off )
#pragma warning(disable: 4748) // "... because optimizations are disabled ..."
#endif

void ImpCvtNum( double nNum, short nPrec, ::rtl::OUString& rRes, sal_Bool bCoreString )
{
    char *q;
    char cBuf[ 40 ], *p = cBuf;

    sal_Unicode cDecimalSep, cThousandSep;
    ImpGetIntntlSep( cDecimalSep, cThousandSep );
    if( bCoreString )
        cDecimalSep = '.';

    if( nNum < 0.0 ) {
        nNum = -nNum;
        *p++ = '-';
    }
    double dMaxNumWithoutExp = (nPrec == 6) ? 1E6 : 1E14;
    myftoa( nNum, p, nPrec,( nNum &&( nNum < 1E-1 || nNum >= dMaxNumWithoutExp ) ) ? 4:0,
        false, true, cDecimalSep );
    // remove trailing zeros
    for( p = cBuf; *p &&( *p != 'E' ); p++ ) {}
    q = p; p--;
    while( nPrec && *p == '0' ) nPrec--, p--;
    if( *p == cDecimalSep ) p--;
    while( *q ) *++p = *q++;
    *++p = 0;
    rRes = ::rtl::OUString::createFromAscii( cBuf );
}

#ifdef _MSC_VER
#pragma optimize( "", on )
#endif

sal_Bool ImpConvStringExt( ::rtl::OUString& rSrc, SbxDataType eTargetType )
{
    sal_Bool bChanged = sal_False;
    ::rtl::OUString aNewString;

    // only special cases are handled, nothing on default
    switch( eTargetType )
    {
        // consider international for floating point
        case SbxSINGLE:
        case SbxDOUBLE:
        case SbxCURRENCY:
        {
            ::rtl::OString aBStr( ::rtl::OUStringToOString( rSrc, RTL_TEXTENCODING_ASCII_US ) );

            sal_Unicode cDecimalSep, cThousandSep;
            ImpGetIntntlSep( cDecimalSep, cThousandSep );
            aNewString = rSrc;

            if( cDecimalSep != (sal_Unicode)'.' )
            {
                sal_Int32 nPos = aNewString.indexOf( cDecimalSep );
                if( nPos != -1 )
                {
                    sal_Unicode* pStr = (sal_Unicode*)aNewString.getStr();
                    pStr[nPos] = (sal_Unicode)'.';
                    bChanged = sal_True;
                }
            }
            break;
        }

        // check as string in case of sal_Bool sal_True and sal_False
        case SbxBOOL:
        {
            if( rSrc.equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("true")) )
            {
                aNewString = ::rtl::OUString::valueOf( (sal_Int32)SbxTRUE );
                bChanged = sal_True;
            }
            else
            if( rSrc.equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("false")) )
            {
                aNewString = ::rtl::OUString::valueOf( (sal_Int32)SbxFALSE );
                bChanged = sal_True;
            }
            break;
        }
        default: break;
    }

    if( bChanged )
        rSrc = aNewString;
    return bChanged;
}


// formatted number output
// the return value is the number of characters used
// from the format

#ifdef _old_format_code_
// leave the code provisionally to copy the previous implementation

static sal_uInt16 printfmtnum( double nNum, XubString& rRes, const XubString& rWFmt )
{
    const String& rFmt = rWFmt;
    char    cFill  = ' ';           // filling characters
    char    cPre   = 0;             // start character ( maybe "$" )
    short   nExpDig= 0;             // number of exponent positions
    short   nPrec  = 0;             // number of positions after decimal point
    short   nWidth = 0;             // number range completely
    short   nLen;                   // length of converted number
    bool    bPoint = false;         // true: with 1000 seperators
    sal_Bool    bTrail = sal_False;         // sal_True, if following minus
    sal_Bool    bSign  = sal_False;         // sal_True: always with leading sign
    sal_Bool    bNeg   = sal_False;         // sal_True: number is negative
    char    cBuf [1024];            // number buffer
    char  * p;
    const char* pFmt = rFmt;
    rRes.Erase();
    // catch $$ and **, is simply output as character
    if( *pFmt == '$' )
      if( *++pFmt != '$' ) rRes += '$';
    if( *pFmt == '*' )
      if( *++pFmt != '*' ) rRes += '*';

    switch( *pFmt++ )
    {
        case 0:
            break;
        case '+':
            bSign = sal_True; nWidth++; break;
        case '*':
            nWidth++; cFill = '*';
            if( *pFmt == '$' ) nWidth++, pFmt++, cPre = '$';
            break;
        case '$':
            nWidth++; cPre = '$'; break;
        case '#':
        case '.':
        case ',':
            pFmt--; break;
    }
    // pre point
    for( ;; )
    {
        while( *pFmt == '#' ) pFmt++, nWidth++;
        // 1000 separators?
        if( *pFmt == ',' )
        {
            nWidth++; pFmt++; bPoint = true;
        } else break;
    }
    // after point
    if( *pFmt == '.' )
    {
        while( *++pFmt == '#' ) nPrec++;
        nWidth += nPrec + 1;
    }
    // exponent
    while( *pFmt == '^' )
        pFmt++, nExpDig++, nWidth++;
    // following minus
    if( !bSign && *pFmt == '-' )
        pFmt++, bTrail = sal_True;

    // convert number
    if( nPrec > 15 ) nPrec = 15;
    if( nNum < 0.0 ) nNum = -nNum, bNeg = sal_True;
    p = cBuf;
    if( bSign ) *p++ = bNeg ? '-' : '+';
    myftoa( nNum, p, nPrec, nExpDig, bPoint, false );
    nLen = strlen( cBuf );

    // overflow?
    if( cPre ) nLen++;
    if( nLen > nWidth ) rRes += '%';
    else {
        nWidth -= nLen;
        while( nWidth-- ) rRes += (xub_Unicode)cFill;
        if( cPre ) rRes += (xub_Unicode)cPre;
    }
    rRes += (xub_Unicode*)&(cBuf[0]);
    if( bTrail )
        rRes += bNeg ? '-' : ' ';

    return (sal_uInt16) ( pFmt - (const char*) rFmt );
}

#endif //_old_format_code_

static sal_uInt16 printfmtstr( const XubString& rStr, XubString& rRes, const XubString& rFmt )
{
    const xub_Unicode* pStr = rStr.GetBuffer();
    const xub_Unicode* pFmtStart = rFmt.GetBuffer();
    const xub_Unicode* pFmt = pFmtStart;
    rRes.Erase();
    switch( *pFmt )
    {
        case '!':
                rRes += *pStr++; pFmt++; break;
        case '\\':
            do
            {
                rRes += *pStr ? *pStr++ : static_cast< xub_Unicode >(' ');
                pFmt++;
            } while( *pFmt != '\\' );
            rRes += *pStr ? *pStr++ : static_cast< xub_Unicode >(' ');
            pFmt++; break;
        case '&':
            rRes = rStr;
            pFmt++; break;
        default:
            rRes = rStr;
            break;
    }
    return (sal_uInt16) ( pFmt - pFmtStart );
}


sal_Bool SbxValue::Scan( const XubString& rSrc, sal_uInt16* pLen )
{
    SbxError eRes = SbxERR_OK;
    if( !CanWrite() )
        eRes = SbxERR_PROP_READONLY;
    else
    {
        double n;
        SbxDataType t;
        eRes = ImpScan( rSrc, n, t, pLen );
        if( eRes == SbxERR_OK )
        {
            if( !IsFixed() )
                SetType( t );
            PutDouble( n );
        }
    }
    if( eRes )
    {
        SetError( eRes ); return sal_False;
    }
    else
        return sal_True;
}


ResMgr* implGetResMgr( void )
{
    static ResMgr* pResMgr = NULL;
    if( !pResMgr )
    {
        ::com::sun::star::lang::Locale aLocale = Application::GetSettings().GetUILocale();
        pResMgr = ResMgr::CreateResMgr("sb", aLocale );
    }
    return pResMgr;
}

class SbxValueFormatResId : public ResId
{
public:
    SbxValueFormatResId( sal_uInt16 nId )
        : ResId( nId, *implGetResMgr() )
    {}
};


enum VbaFormatType
{
    VBA_FORMAT_TYPE_OFFSET, // standard number format
    VBA_FORMAT_TYPE_USERDEFINED, // user defined number format
    VBA_FORMAT_TYPE_NULL
};

struct VbaFormatInfo
{
    VbaFormatType meType;
    const char* mpVbaFormat; // Format string in vba
    NfIndexTableOffset meOffset; // SvNumberFormatter format index, if meType = VBA_FORMAT_TYPE_OFFSET
    const char* mpOOoFormat; // if meType = VBA_FORMAT_TYPE_USERDEFINED
};

#define VBA_FORMAT_OFFSET( pcUtf8, eOffset ) \
    { VBA_FORMAT_TYPE_OFFSET, pcUtf8, eOffset, 0 }

#define VBA_FORMAT_USERDEFINED( pcUtf8, pcDefinedUtf8 ) \
    { VBA_FORMAT_TYPE_USERDEFINED, pcUtf8, NF_NUMBER_STANDARD, pcDefinedUtf8 }

static VbaFormatInfo pFormatInfoTable[] =
{
    VBA_FORMAT_OFFSET( "Long Date", NF_DATE_SYSTEM_LONG ),
    VBA_FORMAT_USERDEFINED( "Medium Date", "DD-MMM-YY" ),
    VBA_FORMAT_OFFSET( "Short Date", NF_DATE_SYSTEM_SHORT ),
    VBA_FORMAT_USERDEFINED( "Long Time", "H:MM:SS AM/PM" ),
    VBA_FORMAT_OFFSET( "Medium Time", NF_TIME_HHMMAMPM ),
    VBA_FORMAT_OFFSET( "Short Time", NF_TIME_HHMM ),
    VBA_FORMAT_OFFSET( "ddddd", NF_DATE_SYSTEM_SHORT ),
    VBA_FORMAT_OFFSET( "dddddd", NF_DATE_SYSTEM_LONG ),
    VBA_FORMAT_USERDEFINED( "ttttt", "H:MM:SS AM/PM" ),
    VBA_FORMAT_OFFSET( "ww", NF_DATE_WW ),
    { VBA_FORMAT_TYPE_NULL, 0, NF_INDEX_TABLE_ENTRIES, 0 }
};

VbaFormatInfo* getFormatInfo( const String& rFmt )
{
    VbaFormatInfo* pInfo = NULL;
    sal_Int16 i = 0;
    while( (pInfo = pFormatInfoTable + i )->mpVbaFormat != NULL )
    {
        if( rFmt.EqualsIgnoreCaseAscii( pInfo->mpVbaFormat ) )
            break;
        i++;
    }
    return pInfo;
}

#define VBAFORMAT_GENERALDATE       "General Date"
#define VBAFORMAT_C                 "c"
#define VBAFORMAT_N                 "n"
#define VBAFORMAT_NN                "nn"
#define VBAFORMAT_W                 "w"
#define VBAFORMAT_Y                 "y"
#define VBAFORMAT_LOWERCASE         "<"
#define VBAFORMAT_UPPERCASE         ">"

// From methods1.cxx
sal_Int16 implGetWeekDay( double aDate, bool bFirstDayParam = false, sal_Int16 nFirstDay = 0 );
// from methods.cxx
sal_Int16 implGetMinute( double dDate );
sal_Int16 implGetDateYear( double aDate );
bool implDateSerial( sal_Int16 nYear, sal_Int16 nMonth, sal_Int16 nDay, double& rdRet );

void SbxValue::Format( XubString& rRes, const XubString* pFmt ) const
{
    short nComma = 0;
    double d = 0;

    // pflin, It is better to use SvNumberFormatter to handle the date/time/number format.
    // the SvNumberFormatter output is mostly compatible with
    // VBA output besides the OOo-basic output
    if( pFmt && !SbxBasicFormater::isBasicFormat( *pFmt ) )
    {
        String aStr = GetString();

        if( pFmt->EqualsIgnoreCaseAscii( VBAFORMAT_LOWERCASE ) )
        {
            rRes = aStr.ToLowerAscii();
            return;
        }
        if( pFmt->EqualsIgnoreCaseAscii( VBAFORMAT_UPPERCASE ) )
        {
            rRes = aStr.ToUpperAscii();
            return;
        }

        LanguageType eLangType = GetpApp()->GetSettings().GetLanguage();
        com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >
            xFactory = comphelper::getProcessServiceFactory();
        SvNumberFormatter aFormatter( xFactory, eLangType );

        sal_uInt32 nIndex;
        double nNumber;
        Color* pCol;

        sal_Bool bSuccess = aFormatter.IsNumberFormat( aStr, nIndex, nNumber );

        // number format, use SvNumberFormatter to handle it.
        if( bSuccess )
        {
            xub_StrLen nCheckPos = 0;
            short nType;
            String aFmtStr = *pFmt;
            VbaFormatInfo* pInfo = getFormatInfo( aFmtStr );
            if( pInfo && pInfo->meType != VBA_FORMAT_TYPE_NULL )
               {
                if( pInfo->meType == VBA_FORMAT_TYPE_OFFSET )
                {
                    nIndex = aFormatter.GetFormatIndex( pInfo->meOffset, eLangType );
                }
                else
                   {
                    aFmtStr.AssignAscii( pInfo->mpOOoFormat );
                    aFormatter.PutandConvertEntry( aFmtStr, nCheckPos, nType, nIndex, LANGUAGE_ENGLISH, eLangType );
                }
                aFormatter.GetOutputString( nNumber, nIndex, rRes, &pCol );
            }
            else if( aFmtStr.EqualsIgnoreCaseAscii( VBAFORMAT_GENERALDATE )
                    || aFmtStr.EqualsIgnoreCaseAscii( VBAFORMAT_C ))
            {
                if( nNumber <=-1.0 || nNumber >= 1.0 )
                {
                    // short date
                    nIndex = aFormatter.GetFormatIndex( NF_DATE_SYSTEM_SHORT, eLangType );
                       aFormatter.GetOutputString( nNumber, nIndex, rRes, &pCol );

                    // long time
                    if( floor( nNumber ) != nNumber )
                    {
                        aFmtStr.AssignAscii( "H:MM:SS AM/PM" );
                        aFormatter.PutandConvertEntry( aFmtStr, nCheckPos, nType, nIndex, LANGUAGE_ENGLISH, eLangType );
                        String aTime;
                        aFormatter.GetOutputString( nNumber, nIndex, aTime, &pCol );
                        rRes.AppendAscii(" ");
                        rRes += aTime;
                    }
                }
                else
                {
                    // long time only
                    aFmtStr.AssignAscii( "H:MM:SS AM/PM" );
                    aFormatter.PutandConvertEntry( aFmtStr, nCheckPos, nType, nIndex, LANGUAGE_ENGLISH, eLangType );
                    aFormatter.GetOutputString( nNumber, nIndex, rRes, &pCol );
                }
            }
            else if( aFmtStr.EqualsIgnoreCaseAscii( VBAFORMAT_N )
                    || aFmtStr.EqualsIgnoreCaseAscii( VBAFORMAT_NN ))
            {
                sal_Int32 nMin = implGetMinute( nNumber );
                if( nMin < 10 && aFmtStr.EqualsIgnoreCaseAscii( VBAFORMAT_NN ) )
                {
                    // Minute in two digits
                     sal_Unicode aBuf[2];
                     aBuf[0] = '0';
                     aBuf[1] = '0' + nMin;
                     rRes = rtl::OUString(aBuf, SAL_N_ELEMENTS(aBuf));
                }
                else
                {
                    rRes = rtl::OUString::valueOf(nMin);
                }
            }
            else if( aFmtStr.EqualsIgnoreCaseAscii( VBAFORMAT_W ))
            {
                sal_Int32 nWeekDay = implGetWeekDay( nNumber );
                rRes = rtl::OUString::valueOf(nWeekDay);
            }
            else if( aFmtStr.EqualsIgnoreCaseAscii( VBAFORMAT_Y ))
            {
                sal_Int16 nYear = implGetDateYear( nNumber );
                double dBaseDate;
                implDateSerial( nYear, 1, 1, dBaseDate );
                sal_Int32 nYear32 = 1 + sal_Int32( nNumber - dBaseDate );
                rRes = rtl::OUString::valueOf(nYear32);
            }
            else
            {
                aFormatter.PutandConvertEntry( aFmtStr, nCheckPos, nType, nIndex, LANGUAGE_ENGLISH, eLangType );
                aFormatter.GetOutputString( nNumber, nIndex, rRes, &pCol );
            }

            return;
        }
    }

    SbxDataType eType = GetType();
    switch( eType )
    {
        case SbxCHAR:
        case SbxBYTE:
        case SbxINTEGER:
        case SbxUSHORT:
        case SbxLONG:
        case SbxULONG:
        case SbxINT:
        case SbxUINT:
        case SbxNULL:       // #45929 NULL with a little cheating
            nComma = 0;     goto cvt;
        case SbxSINGLE:
            nComma = 6;     goto cvt;
        case SbxDOUBLE:
            nComma = 14;

        cvt:
            if( eType != SbxNULL )
                d = GetDouble();

            // #45355 another point to jump in for isnumeric-String
        cvt2:
            if( pFmt )
            {
                SbxAppData& rAppData = GetSbxData_Impl();

                LanguageType eLangType = GetpApp()->GetSettings().GetLanguage();
                if( rAppData.pBasicFormater )
                {
                    if( rAppData.eBasicFormaterLangType != eLangType )
                    {
                        delete rAppData.pBasicFormater;
                        rAppData.pBasicFormater = NULL;
                    }
                }
                rAppData.eBasicFormaterLangType = eLangType;


                if( !rAppData.pBasicFormater )
                {
                    SvtSysLocale aSysLocale;
                    const LocaleDataWrapper& rData = aSysLocale.GetLocaleData();
                    sal_Unicode cComma = rData.getNumDecimalSep()[0];
                    sal_Unicode c1000  = rData.getNumThousandSep()[0];
                    String aCurrencyStrg = rData.getCurrSymbol();

                    // initialize the Basic-formater help object:
                    // get resources for predefined output
                    // of the Format()-command, e. g. for "On/Off"
                    rtl::OUString aOnStrg = SbxValueFormatResId(
                        STR_BASICKEY_FORMAT_ON).toString();
                    rtl::OUString aOffStrg = SbxValueFormatResId(
                        STR_BASICKEY_FORMAT_OFF).toString();
                    rtl::OUString aYesStrg = SbxValueFormatResId(
                        STR_BASICKEY_FORMAT_YES).toString();
                    rtl::OUString aNoStrg = SbxValueFormatResId(
                        STR_BASICKEY_FORMAT_NO).toString();
                    rtl::OUString aTrueStrg = SbxValueFormatResId(
                        STR_BASICKEY_FORMAT_TRUE).toString();
                    rtl::OUString aFalseStrg = SbxValueFormatResId(
                        STR_BASICKEY_FORMAT_FALSE).toString();
                    rtl::OUString aCurrencyFormatStrg = SbxValueFormatResId(
                        STR_BASICKEY_FORMAT_CURRENCY).toString();

                    rAppData.pBasicFormater
                        = new SbxBasicFormater( cComma,c1000,aOnStrg,aOffStrg,
                                    aYesStrg,aNoStrg,aTrueStrg,aFalseStrg,
                                    aCurrencyStrg,aCurrencyFormatStrg );
                }
                // Remark: For performance reasons there's only ONE BasicFormater-
                //    object created and 'stored', so that the expensive resource-
                //    loading is saved (for country-specific predefined outputs,
                //    e. g. "On/Off") and the continous string-creation
                //    operations, too.
                // BUT: therefore this code is NOT multithreading capable!

                // here are problems with ;;;Null because this method is only
                // called, if SbxValue is a number!!!
                // in addition rAppData.pBasicFormater->BasicFormatNull( *pFmt ); could be called!
                if( eType != SbxNULL )
                {
                    rRes = rAppData.pBasicFormater->BasicFormat( d ,*pFmt );
                }
                else
                {
                    rRes = rAppData.pBasicFormater->BasicFormatNull( *pFmt );
                }

            }
            else
            {
                ::rtl::OUString aTmpString( rRes );
                ImpCvtNum( GetDouble(), nComma, aTmpString );
                rRes = aTmpString;
            }
            break;
        case SbxSTRING:
            if( pFmt )
            {
                // #45355 converting if numeric
                if( IsNumericRTL() )
                {
                    ScanNumIntnl( GetString(), d, /*bSingle*/sal_False );
                    goto cvt2;
                }
                else
                {
                    printfmtstr( GetString(), rRes, *pFmt );
                }
            }
            else
                rRes = GetString();
            break;
        default:
            rRes = GetString();
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
