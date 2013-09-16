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
#include "date.hxx"
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

inline bool ImpIsDigit( sal_Unicode c )
{
    return '0' <= c && c <= '9';
}

/** NOTE: slightly differs from strchr() in that it does not consider the
    terminating NULL character to be part of the string and returns bool
    instead of pointer, if character is 0 returns false.
 */
bool ImpStrChr( const sal_Unicode* p, sal_Unicode c )
{
    if (!c)
        return false;
    while (*p)
    {
        if (*p++ == c)
            return true;
    }
    return false;
}

bool ImpIsAlNum( sal_Unicode c )
{
    return (c < 128) ? isalnum( static_cast<char>(c) ) : false;
}

// scanning a string according to BASIC-conventions
// but exponent may also be a D, so data type is SbxDOUBLE
// conversion error if data type is fixed and it doesn't fit

SbxError ImpScan( const OUString& rWSrc, double& nVal, SbxDataType& rType,
                  sal_uInt16* pLen, bool bAllowIntntl, bool bOnlyIntntl )
{
    sal_Unicode cIntntlDecSep, cIntntlGrpSep;
    sal_Unicode cNonIntntlDecSep = '.';
    if( bAllowIntntl || bOnlyIntntl )
    {
        ImpGetIntntlSep( cIntntlDecSep, cIntntlGrpSep );
        if( bOnlyIntntl )
            cNonIntntlDecSep = cIntntlDecSep;
    }
    else
    {
        cIntntlDecSep = cNonIntntlDecSep;
        cIntntlGrpSep = 0;  // no group separator accepted in non-i18n
    }

    const sal_Unicode* const pStart = rWSrc.getStr();
    const sal_Unicode* p = pStart;
    OUStringBuffer aBuf( rWSrc.getLength());
    bool bRes = true;
    bool bMinus = false;
    nVal = 0;
    SbxDataType eScanType = SbxSINGLE;
    while( *p == ' ' || *p == '\t' )
        p++;
    if( *p == '-' )
    {
        p++;
        bMinus = true;
    }
    if( ImpIsDigit( *p ) || ((*p == cNonIntntlDecSep || *p == cIntntlDecSep ||
                    (cIntntlDecSep && *p == cIntntlGrpSep)) && ImpIsDigit( *(p+1) )))
    {
        short exp = 0;
        short decsep = 0;
        short ndig = 0;
        short ncdig = 0;    // number of digits after decimal point
        OUStringBuffer aSearchStr("0123456789DEde");
        aSearchStr.append(cNonIntntlDecSep);
        if( cIntntlDecSep != cNonIntntlDecSep )
            aSearchStr.append(cIntntlDecSep);
        if( bOnlyIntntl )
            aSearchStr.append(cIntntlGrpSep);
        const sal_Unicode* const pSearchStr = aSearchStr.getStr();
        const sal_Unicode pDdEe[] = { 'D', 'd', 'E', 'e', 0 };
        while( ImpStrChr( pSearchStr, *p ) )
        {
            aBuf.append( *p );
            if( bOnlyIntntl && *p == cIntntlGrpSep )
            {
                p++;
                continue;
            }
            if( *p == cNonIntntlDecSep || *p == cIntntlDecSep )
            {
                // Use the separator that is passed to stringToDouble()
                aBuf[ p - pStart ] = cIntntlDecSep;
                p++;
                if( ++decsep > 1 )
                    continue;
            }
            else if( ImpStrChr( pDdEe, *p ) )
            {
                if( ++exp > 1 )
                {
                    p++;
                    continue;
                }
                if( *p == 'D' || *p == 'd' )
                    eScanType = SbxDOUBLE;
                aBuf[ p - pStart ] = 'E';
                p++;
            }
            else
            {
                p++;
                if( decsep && !exp )
                    ncdig++;
            }
            if( !exp )
                ndig++;
        }

        if( decsep > 1 || exp > 1 )
            bRes = false;

        OUString aBufStr( aBuf.makeStringAndClear());
        rtl_math_ConversionStatus eStatus = rtl_math_ConversionStatus_Ok;
        sal_Int32 nParseEnd = 0;
        nVal = rtl::math::stringToDouble( aBufStr, cIntntlDecSep, cIntntlGrpSep, &eStatus, &nParseEnd );
        if( eStatus != rtl_math_ConversionStatus_Ok || nParseEnd != aBufStr.getLength() )
            bRes = false;

        if( !decsep && !exp )
        {
            if( nVal >= SbxMININT && nVal <= SbxMAXINT )
                eScanType = SbxINTEGER;
            else if( nVal >= SbxMINLNG && nVal <= SbxMAXLNG )
                eScanType = SbxLONG;
        }

        ndig = ndig - decsep;
        // too many numbers for SINGLE?
        if( ndig > 15 || ncdig > 6 )
            eScanType = SbxDOUBLE;

        // type detection?
        const sal_Unicode pTypes[] = { '%', '!', '&', '#', 0 };
        if( ImpStrChr( pTypes, *p ) )
            p++;
    }
    // hex/octal number? read in and convert:
    else if( *p == '&' )
    {
        p++;
        eScanType = SbxLONG;
        OUString aCmp( "0123456789ABCDEFabcdef" );
        char base = 16;
        char ndig = 8;
        switch( *p++ )
        {
            case 'O':
            case 'o':
                aCmp = "01234567";
                base = 8;
                ndig = 11;
                break;
            case 'H':
            case 'h':
                break;
            default :
                bRes = false;
        }
        const sal_Unicode* const pCmp = aCmp.getStr();
        while( ImpIsAlNum( *p ) )    /* XXX: really munge all alnum also when error? */
        {
            sal_Unicode ch = *p;
            if( ImpStrChr( pCmp, ch ) )
            {
                if (ch > 0x60)
                    ch -= 0x20;     // convert ASCII lower to upper case
                aBuf.append( ch );
            }
            else
                bRes = false;
            p++;
        }
        OUString aBufStr( aBuf.makeStringAndClear());
        long l = 0;
        for( const sal_Unicode* q = aBufStr.getStr(); bRes && *q; q++ )
        {
            int i = *q - '0';
            if( i > 9 )
                i -= 7;     // 'A'-'0' = 17 => 10, ...
            l = ( l * base ) + i;
            if( !ndig-- )
                bRes = false;
        }
        if( *p == '&' )
            p++;
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
SbxError SbxValue::ScanNumIntnl( const OUString& rSrc, double& nVal, bool bSingle )
{
    SbxDataType t;
    sal_uInt16 nLen = 0;
    SbxError nRetError = ImpScan( rSrc, nVal, t, &nLen,
        /*bAllowIntntl*/false, /*bOnlyIntntl*/true );
    // read completely?
    if( nRetError == SbxERR_OK && nLen != rSrc.getLength() )
    {
        nRetError = SbxERR_CONVERSION;
    }
    if( bSingle )
    {
        SbxValues aValues( nVal );
        nVal = (double)ImpGetSingle( &aValues );    // here error at overflow
    }
    return nRetError;
}


static const double roundArray[] = {
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
    int i;

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
        int digit;
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

void ImpCvtNum( double nNum, short nPrec, OUString& rRes, bool bCoreString )
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
    rRes = OUString::createFromAscii( cBuf );
}

bool ImpConvStringExt( OUString& rSrc, SbxDataType eTargetType )
{
    bool bChanged = false;
    OUString aNewString;

    // only special cases are handled, nothing on default
    switch( eTargetType )
    {
        // consider international for floating point
        case SbxSINGLE:
        case SbxDOUBLE:
        case SbxCURRENCY:
        {
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
                    bChanged = true;
                }
            }
            break;
        }

        // check as string in case of sal_Bool sal_True and sal_False
        case SbxBOOL:
        {
            if( rSrc.equalsIgnoreAsciiCase("true") )
            {
                aNewString = OUString::number( SbxTRUE );
                bChanged = true;
            }
            else if( rSrc.equalsIgnoreAsciiCase("false") )
            {
                aNewString = OUString::number( SbxFALSE );
                bChanged = true;
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

static sal_uInt16 printfmtnum( double nNum, OUString& rRes, const OUString& rWFmt )
{
    const String& rFmt = rWFmt;
    char    cFill  = ' ';           // filling characters
    char    cPre   = 0;             // start character ( maybe "$" )
    short   nExpDig= 0;             // number of exponent positions
    short   nPrec  = 0;             // number of positions after decimal point
    short   nWidth = 0;             // number range completely
    short   nLen;                   // length of converted number
    bool    bPoint = false;         // true: with 1000 separators
    bool    bTrail = false;         // true, if following minus
    bool    bSign  = false;         // true: always with leading sign
    bool    bNeg   = false;         // true: number is negative
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
            bSign = true; nWidth++; break;
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
        pFmt++, bTrail = true;

    // convert number
    if( nPrec > 15 ) nPrec = 15;
    if( nNum < 0.0 ) nNum = -nNum, bNeg = true;
    p = cBuf;
    if( bSign ) *p++ = bNeg ? '-' : '+';
    myftoa( nNum, p, nPrec, nExpDig, bPoint, false );
    nLen = strlen( cBuf );

    // overflow?
    if( cPre ) nLen++;
    if( nLen > nWidth ) rRes += '%';
    else {
        nWidth -= nLen;
        while( nWidth-- ) rRes += (sal_Unicode)cFill;
        if( cPre ) rRes += (sal_Unicode)cPre;
    }
    rRes += (sal_Unicode*)&(cBuf[0]);
    if( bTrail )
        rRes += bNeg ? '-' : ' ';

    return (sal_uInt16) ( pFmt - (const char*) rFmt );
}

#endif //_old_format_code_

static sal_uInt16 printfmtstr( const OUString& rStr, OUString& rRes, const OUString& rFmt )
{
    OUStringBuffer aTemp;
    const sal_Unicode* pStr = rStr.getStr();
    const sal_Unicode* pFmtStart = rFmt.getStr();
    const sal_Unicode* pFmt = pFmtStart;

    switch( *pFmt )
    {
    case '!':
        aTemp.append(*pStr++);
        pFmt++;
        break;
    case '\\':
        do
        {
            aTemp.append( *pStr ? *pStr++ : static_cast< sal_Unicode >(' '));
            pFmt++;
        }
        while( *pFmt != '\\' );
        aTemp.append(*pStr ? *pStr++ : static_cast< sal_Unicode >(' '));
        pFmt++; break;
    case '&':
        aTemp = rStr;
        pFmt++; break;
    default:
        aTemp = rStr;
        break;
    }
    rRes = aTemp.makeStringAndClear();
    return (sal_uInt16) ( pFmt - pFmtStart );
}


sal_Bool SbxValue::Scan( const OUString& rSrc, sal_uInt16* pLen )
{
    SbxError eRes = SbxERR_OK;
    if( !CanWrite() )
    {
        eRes = SbxERR_PROP_READONLY;
    }
    else
    {
        double n;
        SbxDataType t;
        eRes = ImpScan( rSrc, n, t, pLen );
        if( eRes == SbxERR_OK )
        {
            if( !IsFixed() )
            {
                SetType( t );
            }
            PutDouble( n );
        }
    }
    if( eRes )
    {
        SetError( eRes ); return sal_False;
    }
    else
    {
        return sal_True;
    }
}


ResMgr* implGetResMgr( void )
{
    static ResMgr* pResMgr = NULL;
    if( !pResMgr )
    {
        pResMgr = ResMgr::CreateResMgr("sb", Application::GetSettings().GetUILanguageTag() );
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
    OUString mpVbaFormat; // Format string in vba
    NfIndexTableOffset meOffset; // SvNumberFormatter format index, if meType = VBA_FORMAT_TYPE_OFFSET
    const char* mpOOoFormat; // if meType = VBA_FORMAT_TYPE_USERDEFINED
};

#define VBA_FORMAT_OFFSET( pcUtf8, eOffset ) \
    { VBA_FORMAT_TYPE_OFFSET, OUString(pcUtf8), eOffset, 0 }

#define VBA_FORMAT_USERDEFINED( pcUtf8, pcDefinedUtf8 ) \
    { VBA_FORMAT_TYPE_USERDEFINED, OUString(pcUtf8), NF_NUMBER_STANDARD, pcDefinedUtf8 }

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
    { VBA_FORMAT_TYPE_NULL, OUString(""), NF_INDEX_TABLE_ENTRIES, 0 }
};

VbaFormatInfo* getFormatInfo( const OUString& rFmt )
{
    VbaFormatInfo* pInfo = NULL;
    sal_Int16 i = 0;
    while( (pInfo = pFormatInfoTable + i )->meType != VBA_FORMAT_TYPE_NULL )
    {
        if( rFmt.equalsIgnoreAsciiCase( pInfo->mpVbaFormat ) )
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

void SbxValue::Format( OUString& rRes, const OUString* pFmt ) const
{
    short nComma = 0;
    double d = 0;

    // pflin, It is better to use SvNumberFormatter to handle the date/time/number format.
    // the SvNumberFormatter output is mostly compatible with
    // VBA output besides the OOo-basic output
    if( pFmt && !SbxBasicFormater::isBasicFormat( *pFmt ) )
    {
        OUString aStr = GetOUString();

        SvtSysLocale aSysLocale;
        const CharClass& rCharClass = aSysLocale.GetCharClass();

        if( pFmt->equalsIgnoreAsciiCase( VBAFORMAT_LOWERCASE ) )
        {
            rRes = rCharClass.lowercase( aStr );
            return;
        }
        if( pFmt->equalsIgnoreAsciiCase( VBAFORMAT_UPPERCASE ) )
        {
            rRes = rCharClass.uppercase( aStr );
            return;
        }

        LanguageType eLangType = GetpApp()->GetSettings().GetLanguageTag().getLanguageType();
        SvNumberFormatter aFormatter( comphelper::getProcessComponentContext(), eLangType );

        sal_uInt32 nIndex = 0;
        double nNumber;
        Color* pCol;

        sal_Bool bSuccess = aFormatter.IsNumberFormat( aStr, nIndex, nNumber );

        // number format, use SvNumberFormatter to handle it.
        if( bSuccess )
        {
            sal_Int32 nCheckPos = 0;
            short nType;
            OUString aFmtStr = *pFmt;
            VbaFormatInfo* pInfo = getFormatInfo( aFmtStr );
            if( pInfo && pInfo->meType != VBA_FORMAT_TYPE_NULL )
            {
                if( pInfo->meType == VBA_FORMAT_TYPE_OFFSET )
                {
                    nIndex = aFormatter.GetFormatIndex( pInfo->meOffset, eLangType );
                }
                else
                {
                    aFmtStr = OUString::createFromAscii(pInfo->mpOOoFormat);
                    aFormatter.PutandConvertEntry( aFmtStr, nCheckPos, nType, nIndex, LANGUAGE_ENGLISH, eLangType );
                }
                aFormatter.GetOutputString( nNumber, nIndex, rRes, &pCol );
            }
            else if( aFmtStr.equalsIgnoreAsciiCase( VBAFORMAT_GENERALDATE )
                    || aFmtStr.equalsIgnoreAsciiCase( VBAFORMAT_C ))
            {
                if( nNumber <=-1.0 || nNumber >= 1.0 )
                {
                    // short date
                    nIndex = aFormatter.GetFormatIndex( NF_DATE_SYSTEM_SHORT, eLangType );
                    aFormatter.GetOutputString( nNumber, nIndex, rRes, &pCol );

                    // long time
                    if( floor( nNumber ) != nNumber )
                    {
                        aFmtStr = "H:MM:SS AM/PM";
                        aFormatter.PutandConvertEntry( aFmtStr, nCheckPos, nType, nIndex, LANGUAGE_ENGLISH, eLangType );
                        OUString aTime;
                        aFormatter.GetOutputString( nNumber, nIndex, aTime, &pCol );
                        rRes += " ";
                        rRes += aTime;
                    }
                }
                else
                {
                    // long time only
                    aFmtStr = "H:MM:SS AM/PM";
                    aFormatter.PutandConvertEntry( aFmtStr, nCheckPos, nType, nIndex, LANGUAGE_ENGLISH, eLangType );
                    aFormatter.GetOutputString( nNumber, nIndex, rRes, &pCol );
                }
            }
            else if( aFmtStr.equalsIgnoreAsciiCase( VBAFORMAT_N ) ||
                     aFmtStr.equalsIgnoreAsciiCase( VBAFORMAT_NN ))
            {
                sal_Int32 nMin = implGetMinute( nNumber );
                if( nMin < 10 && aFmtStr.equalsIgnoreAsciiCase( VBAFORMAT_NN ))
                {
                    // Minute in two digits
                     sal_Unicode aBuf[2];
                     aBuf[0] = '0';
                     aBuf[1] = '0' + nMin;
                     rRes = OUString(aBuf, SAL_N_ELEMENTS(aBuf));
                }
                else
                {
                    rRes = OUString::number(nMin);
                }
            }
            else if( aFmtStr.equalsIgnoreAsciiCase( VBAFORMAT_W ))
            {
                sal_Int32 nWeekDay = implGetWeekDay( nNumber );
                rRes = OUString::number(nWeekDay);
            }
            else if( aFmtStr.equalsIgnoreAsciiCase( VBAFORMAT_Y ))
            {
                sal_Int16 nYear = implGetDateYear( nNumber );
                double dBaseDate;
                implDateSerial( nYear, 1, 1, dBaseDate );
                sal_Int32 nYear32 = 1 + sal_Int32( nNumber - dBaseDate );
                rRes = OUString::number(nYear32);
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
        {
            d = GetDouble();
        }
        // #45355 another point to jump in for isnumeric-String
    cvt2:
        if( pFmt )
        {
            SbxAppData& rAppData = GetSbxData_Impl();

            LanguageType eLangType = GetpApp()->GetSettings().GetLanguageTag().getLanguageType();
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
                OUString aCurrencyStrg = rData.getCurrSymbol();

                // initialize the Basic-formater help object:
                // get resources for predefined output
                // of the Format()-command, e. g. for "On/Off"
                OUString aOnStrg = SbxValueFormatResId(STR_BASICKEY_FORMAT_ON).toString();
                OUString aOffStrg = SbxValueFormatResId(STR_BASICKEY_FORMAT_OFF).toString();
                OUString aYesStrg = SbxValueFormatResId(STR_BASICKEY_FORMAT_YES).toString();
                OUString aNoStrg = SbxValueFormatResId(STR_BASICKEY_FORMAT_NO).toString();
                OUString aTrueStrg = SbxValueFormatResId(STR_BASICKEY_FORMAT_TRUE).toString();
                OUString aFalseStrg = SbxValueFormatResId(STR_BASICKEY_FORMAT_FALSE).toString();
                OUString aCurrencyFormatStrg = SbxValueFormatResId(STR_BASICKEY_FORMAT_CURRENCY).toString();

                rAppData.pBasicFormater = new SbxBasicFormater( cComma,c1000,aOnStrg,aOffStrg,
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
            OUString aTmpString( rRes );
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
                ScanNumIntnl( GetOUString(), d, /*bSingle*/false );
                goto cvt2;
            }
            else
            {
                printfmtstr( GetOUString(), rRes, *pFmt );
            }
        }
        else
        {
            rRes = GetOUString();
        }
        break;
    default:
        rRes = GetOUString();
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
