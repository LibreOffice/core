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
#include "precompiled_basic.hxx"
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
#include <basic/sbxform.hxx>
#include <svtools/svtools.hrc>

#include "basrid.hxx"
#include "runtime.hxx"

#include <svl/zforlist.hxx>
#include <comphelper/processfactory.hxx>


void ImpGetIntntlSep( sal_Unicode& rcDecimalSep, sal_Unicode& rcThousandSep )
{
    SvtSysLocale aSysLocale;
    const LocaleDataWrapper& rData = aSysLocale.GetLocaleData();
    rcDecimalSep = rData.getNumDecimalSep().GetBuffer()[0];
    rcThousandSep = rData.getNumThousandSep().GetBuffer()[0];
}

// Scannen eines Strings nach BASIC-Konventionen
// Dies entspricht den ueblichen Konventionen, nur dass der Exponent
// auch ein D sein darf, was den Datentyp auf SbxDOUBLE festlegt.
// Die Routine versucht, den Datentyp so klein wie moeglich zu gestalten.
// Das ganze gibt auch noch einen Konversionsfehler, wenn der Datentyp
// Fixed ist und das ganze nicht hineinpasst!

SbxError ImpScan( const ::rtl::OUString& rWSrc, double& nVal, SbxDataType& rType,
                  sal_uInt16* pLen, sal_Bool bAllowIntntl, sal_Bool bOnlyIntntl )
{
    ::rtl::OString aBStr( ::rtl::OUStringToOString( rWSrc, RTL_TEXTENCODING_ASCII_US ) );

    // Bei International Komma besorgen
    char cIntntlComma, cIntntl1000;
    char cNonIntntlComma = '.';

    sal_Unicode cDecimalSep, cThousandSep = 0;
    if( bAllowIntntl || bOnlyIntntl )
    {
        ImpGetIntntlSep( cDecimalSep, cThousandSep );
        cIntntlComma = (char)cDecimalSep;
        cIntntl1000 = (char)cThousandSep;
    }
    // Sonst einfach auch auf . setzen
    else
    {
        cIntntlComma = cNonIntntlComma;
        cIntntl1000 = cNonIntntlComma;  // Unschaedlich machen
    }
    // Nur International -> IntnlComma uebernehmen
    if( bOnlyIntntl )
    {
        cNonIntntlComma = cIntntlComma;
        cIntntl1000 = (char)cThousandSep;
    }

    const char* pStart = aBStr.getStr();
    const char* p = pStart;
    char buf[ 80 ], *q = buf;
    sal_Bool bRes = sal_True;
    sal_Bool bMinus = sal_False;
    nVal = 0;
    SbxDataType eScanType = SbxSINGLE;
    // Whitespace wech
    while( *p &&( *p == ' ' || *p == '\t' ) ) p++;
    // Zahl? Dann einlesen und konvertieren.
    if( *p == '-' )
        p++, bMinus = sal_True;
    if( isdigit( *p ) ||( (*p == cNonIntntlComma || *p == cIntntlComma ||
            *p == cIntntl1000) && isdigit( *(p+1 ) ) ) )
    {
        short exp = 0;      // >0: Exponentteil
        short comma = 0;    // >0: Nachkomma
        short ndig = 0;     // Anzahl Ziffern
        short ncdig = 0;    // Anzahl Ziffern nach Komma
        ByteString aSearchStr( "0123456789DEde" );
        // Kommas ergaenzen
        aSearchStr += cNonIntntlComma;
        if( cIntntlComma != cNonIntntlComma )
            aSearchStr += cIntntlComma;
        if( bOnlyIntntl )
            aSearchStr += cIntntl1000;
        const char* pSearchStr = aSearchStr.GetBuffer();
        while( strchr( pSearchStr, *p ) && *p )
        {
            // 1000er-Trenner ueberlesen
            if( bOnlyIntntl && *p == cIntntl1000 )
            {
                p++;
                continue;
            }

            // Komma oder Exponent?
            if( *p == cNonIntntlComma || *p == cIntntlComma )
            {
                // Immer '.' einfuegen, damit atof funktioniert
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
                // Vorzeichen hinter Exponent?
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
        // Komma, Exponent mehrfach vorhanden?
        if( comma > 1 || exp > 1 )
            bRes = sal_False;
        // Kann auf Integer gefaltet werden?
        if( !comma && !exp )
        {
            if( nVal >= SbxMININT && nVal <= SbxMAXINT )
                eScanType = SbxINTEGER;
            else if( nVal >= SbxMINLNG && nVal <= SbxMAXLNG )
                eScanType = SbxLONG;
        }

        nVal = atof( buf );
        ndig = ndig - comma;
        // zu viele Zahlen fuer SINGLE?
        if( ndig > 15 || ncdig > 6 )
            eScanType = SbxDOUBLE;

        // Typkennung?
        if( strchr( "%!&#", *p ) && *p ) p++;
    }
    // Hex/Oktalzahl? Einlesen und konvertieren:
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
            default : bRes = sal_False;
        }
        long l = 0;
        int i;
        while( isalnum( *p ) )
        {
            char ch = sal::static_int_cast< char >( toupper( *p ) );
            p++;
            if( strchr( cmp, ch ) ) *q++ = ch;
            else bRes = sal_False;
        }
        *q = 0;
        for( q = buf; *q; q++ )
        {
            i =( *q & 0xFF ) - '0';
            if( i > 9 ) i -= 7;
            l =( l * base ) + i;
            if( !ndig-- )
                bRes = sal_False;
        }
        if( *p == '&' ) p++;
        nVal = (double) l;
        if( l >= SbxMININT && l <= SbxMAXINT )
            eScanType = SbxINTEGER;
    }
    else if ( SbiRuntime::isVBAEnabled() )
    {
        OSL_TRACE("Reporting error converting");
        return SbxERR_CONVERSION;
    }
    if( pLen )
        *pLen = (sal_uInt16) ( p - pStart );
    if( !bRes )
        return SbxERR_CONVERSION;
    if( bMinus )
        nVal = -nVal;
    rType = eScanType;
    return SbxERR_OK;
}

// Schnittstelle fuer CDbl im Basic
SbxError SbxValue::ScanNumIntnl( const String& rSrc, double& nVal, sal_Bool bSingle )
{
    SbxDataType t;
    sal_uInt16 nLen = 0;
    SbxError nRetError = ImpScan( rSrc, nVal, t, &nLen,
        /*bAllowIntntl*/sal_False, /*bOnlyIntntl*/sal_True );
    // Komplett gelesen?
    if( nRetError == SbxERR_OK && nLen != rSrc.Len() )
        nRetError = SbxERR_CONVERSION;

    if( bSingle )
    {
        SbxValues aValues( nVal );
        nVal = (double)ImpGetSingle( &aValues );    // Hier Error bei Overflow
    }
    return nRetError;
}

////////////////////////////////////////////////////////////////////////////

static double roundArray[] = {
    5.0e+0, 0.5e+0, 0.5e-1, 0.5e-2, 0.5e-3, 0.5e-4, 0.5e-5, 0.5e-6, 0.5e-7,
    0.5e-8, 0.5e-9, 0.5e-10,0.5e-11,0.5e-12,0.5e-13,0.5e-14,0.5e-15 };

/***************************************************************************
|*
|*  void myftoa( double, char *, short, short, sal_Bool, sal_Bool )
|*
|*  Beschreibung:       Konversion double --> ASCII
|*  Parameter:          double              die Zahl.
|*                      char *              der Zielpuffer
|*                      short               Anzahl Nachkommastellen
|*                      short               Weite des Exponenten( 0=kein E )
|*                      sal_Bool                sal_True: mit 1000er Punkten
|*                      sal_Bool                sal_True: formatfreie Ausgabe
|*
***************************************************************************/

static void myftoa( double nNum, char * pBuf, short nPrec, short nExpWidth,
                    sal_Bool bPt, sal_Bool bFix, sal_Unicode cForceThousandSep = 0 )
{

    short nExp = 0;                     // Exponent
    short nDig = nPrec + 1;             // Anzahl Digits in Zahl
    short nDec;                         // Anzahl Vorkommastellen
    register int i;

    // Komma besorgen
    sal_Unicode cDecimalSep, cThousandSep;
    ImpGetIntntlSep( cDecimalSep, cThousandSep );
    if( cForceThousandSep )
        cThousandSep = cForceThousandSep;

    // Exponentberechnung:
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

    // Zahl runden:
    if( (nNum += roundArray [( nDig > 16 ) ? 16 : nDig] ) >= 10.0 )
    {
        nNum = 1.0;
        ++nExp;
        if( !nExpWidth ) ++nDig;
    }

    // Bestimmung der Vorkommastellen:
    if( !nExpWidth )
    {
        if( nExp < 0 )
        {
            // #41691: Auch bei bFix eine 0 spendieren
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

    // Zahl ausgeben:
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

    // Exponent ausgeben:
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

// Die Zahl wird unformatiert mit der angegebenen Anzahl NK-Stellen
// aufbereitet. Evtl. wird ein Minus vorangestellt.
// Diese Routine ist public, weil sie auch von den Put-Funktionen
// der Klasse SbxImpSTRING verwendet wird.

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
        sal_False, sal_True, cDecimalSep );
    // Trailing Zeroes weg:
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
    // Merken, ob ueberhaupt was geaendert wurde
    sal_Bool bChanged = sal_False;
    ::rtl::OUString aNewString;

    // Nur Spezial-F�lle behandeln, als Default tun wir nichts
    switch( eTargetType )
    {
        // Bei Fliesskomma International beruecksichtigen
        case SbxSINGLE:
        case SbxDOUBLE:
        case SbxCURRENCY:
        {
            ::rtl::OString aBStr( ::rtl::OUStringToOString( rSrc, RTL_TEXTENCODING_ASCII_US ) );

            // Komma besorgen
            sal_Unicode cDecimalSep, cThousandSep;
            ImpGetIntntlSep( cDecimalSep, cThousandSep );
            aNewString = rSrc;

            // Ersetzen, wenn DecimalSep kein '.' (nur den ersten)
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

        // Bei sal_Bool sal_True und sal_False als String pruefen
        case SbxBOOL:
        {
            if( rSrc.equalsIgnoreAsciiCaseAscii( "true" ) )
            {
                aNewString = ::rtl::OUString::valueOf( (sal_Int32)SbxTRUE );
                bChanged = sal_True;
            }
            else
            if( rSrc.equalsIgnoreAsciiCaseAscii( "false" ) )
            {
                aNewString = ::rtl::OUString::valueOf( (sal_Int32)SbxFALSE );
                bChanged = sal_True;
            }
            break;
        }
        default: break;
    }
    // String bei Aenderung uebernehmen
    if( bChanged )
        rSrc = aNewString;
    return bChanged;
}


// Formatierte Zahlenausgabe
// Der Returnwert ist die Anzahl Zeichen, die aus dem
// Format verwendt wurden.

#ifdef _old_format_code_
// lasse diesen Code vorl"aufig drin, zum 'abgucken'
// der bisherigen Implementation

static sal_uInt16 printfmtnum( double nNum, XubString& rRes, const XubString& rWFmt )
{
    const String& rFmt = rWFmt;
    char    cFill  = ' ';           // Fuellzeichen
    char    cPre   = 0;             // Startzeichen( evtl. "$" )
    short   nExpDig= 0;             // Anzahl Exponentstellen
    short   nPrec  = 0;             // Anzahl Nachkommastellen
    short   nWidth = 0;             // Zahlenweite gesamnt
    short   nLen;                   // Laenge konvertierte Zahl
    sal_Bool    bPoint = sal_False;         // sal_True: mit 1000er Kommas
    sal_Bool    bTrail = sal_False;         // sal_True, wenn folgendes Minus
    sal_Bool    bSign  = sal_False;         // sal_True: immer mit Vorzeichen
    sal_Bool    bNeg   = sal_False;         // sal_True: Zahl ist negativ
    char    cBuf [1024];            // Zahlenpuffer
    char  * p;
    const char* pFmt = rFmt;
    rRes.Erase();
    // $$ und ** abfangen. Einfach wird als Zeichen ausgegeben.
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
    // Vorkomma:
    for( ;; )
    {
        while( *pFmt == '#' ) pFmt++, nWidth++;
        // 1000er Kommas?
        if( *pFmt == ',' )
        {
            nWidth++; pFmt++; bPoint = sal_True;
        } else break;
    }
    // Nachkomma:
    if( *pFmt == '.' )
    {
        while( *++pFmt == '#' ) nPrec++;
        nWidth += nPrec + 1;
    }
    // Exponent:
    while( *pFmt == '^' )
        pFmt++, nExpDig++, nWidth++;
    // Folgendes Minus:
    if( !bSign && *pFmt == '-' )
        pFmt++, bTrail = sal_True;

    // Zahl konvertieren:
    if( nPrec > 15 ) nPrec = 15;
    if( nNum < 0.0 ) nNum = -nNum, bNeg = sal_True;
    p = cBuf;
    if( bSign ) *p++ = bNeg ? '-' : '+';
    myftoa( nNum, p, nPrec, nExpDig, bPoint, sal_False );
    nLen = strlen( cBuf );

    // Ueberlauf?
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

/////////////////////////////////////////////////////////////////////////

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
        pResMgr = ResMgr::CreateResMgr(CREATEVERSIONRESMGR_NAME(sb), aLocale );
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
sal_Bool implDateSerial( sal_Int16 nYear, sal_Int16 nMonth, sal_Int16 nDay, double& rdRet );

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
                     sal_Unicode* p = rRes.AllocBuffer( 2 );
                     *p++ = '0';
                     *p = sal_Unicode( '0' + nMin );
                }
                else
                {
                    rRes = String::CreateFromInt32( nMin );
                }
            }
            else if( aFmtStr.EqualsIgnoreCaseAscii( VBAFORMAT_W ))
            {
                sal_Int32 nWeekDay = implGetWeekDay( nNumber );
                rRes = String::CreateFromInt32( nWeekDay );
            }
            else if( aFmtStr.EqualsIgnoreCaseAscii( VBAFORMAT_Y ))
            {
                sal_Int16 nYear = implGetDateYear( nNumber );
                double dBaseDate;
                implDateSerial( nYear, 1, 1, dBaseDate );
                sal_Int32 nYear32 = 1 + sal_Int32( nNumber - dBaseDate );
                rRes = String::CreateFromInt32( nYear32 );
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
        case SbxNULL:       // #45929 NULL mit durchschummeln
            nComma = 0;     goto cvt;
        case SbxSINGLE:
            nComma = 6;     goto cvt;
        case SbxDOUBLE:
            nComma = 14;

        cvt:
            if( eType != SbxNULL )
                d = GetDouble();

            // #45355 weiterer Einsprungpunkt fuer isnumeric-String
        cvt2:
            if( pFmt )
            {
                // hole die 'statischen' Daten f"ur Sbx
                SbxAppData* pData = GetSbxData_Impl();

                LanguageType eLangType = GetpApp()->GetSettings().GetLanguage();
                if( pData->pBasicFormater )
                {
                    if( pData->eBasicFormaterLangType != eLangType )
                    {
                        delete pData->pBasicFormater;
                        pData->pBasicFormater = NULL;
                    }
                }
                pData->eBasicFormaterLangType = eLangType;

                // falls bisher noch kein BasicFormater-Objekt
                // existiert, so erzeuge dieses
                if( !pData->pBasicFormater )
                {
                    SvtSysLocale aSysLocale;
                    const LocaleDataWrapper& rData = aSysLocale.GetLocaleData();
                    sal_Unicode cComma = rData.getNumDecimalSep().GetBuffer()[0];
                    sal_Unicode c1000  = rData.getNumThousandSep().GetBuffer()[0];
                    String aCurrencyStrg = rData.getCurrSymbol();

                    // Initialisierung des Basic-Formater-Hilfsobjekts:
                    // hole die Resourcen f"ur die vordefinierten Ausgaben
                    // des Format()-Befehls, z.B. f"ur "On/Off".
                    String aOnStrg = String( SbxValueFormatResId(
                        STR_BASICKEY_FORMAT_ON ) );
                    String aOffStrg = String( SbxValueFormatResId(
                        STR_BASICKEY_FORMAT_OFF) );
                    String aYesStrg = String( SbxValueFormatResId(
                        STR_BASICKEY_FORMAT_YES) );
                    String aNoStrg = String( SbxValueFormatResId(
                        STR_BASICKEY_FORMAT_NO) );
                    String aTrueStrg = String( SbxValueFormatResId(
                        STR_BASICKEY_FORMAT_TRUE) );
                    String aFalseStrg = String( SbxValueFormatResId(
                        STR_BASICKEY_FORMAT_FALSE) );
                    String aCurrencyFormatStrg = String( SbxValueFormatResId(
                        STR_BASICKEY_FORMAT_CURRENCY) );
                    // erzeuge das Basic-Formater-Objekt
                    pData->pBasicFormater
                        = new SbxBasicFormater( cComma,c1000,aOnStrg,aOffStrg,
                                    aYesStrg,aNoStrg,aTrueStrg,aFalseStrg,
                                    aCurrencyStrg,aCurrencyFormatStrg );
                }
                // Bem.: Aus Performance-Gr"unden wird nur EIN BasicFormater-
                //    Objekt erzeugt und 'gespeichert', dadurch erspart man
                //    sich das teure Resourcen-Laden (f"ur landesspezifische
                //    vordefinierte Ausgaben, z.B. "On/Off") und die st"andige
                //    String-Erzeugungs Operationen.
                // ABER: dadurch ist dieser Code NICHT multithreading f"ahig !

                // hier gibt es Probleme mit ;;;Null, da diese Methode nur aufgerufen
                // wird, wenn der SbxValue eine Zahl ist !!!
                // dazu koennte: pData->pBasicFormater->BasicFormatNull( *pFmt ); aufgerufen werden !
                if( eType != SbxNULL )
                {
                    rRes = pData->pBasicFormater->BasicFormat( d ,*pFmt );
                }
                else
                {
                    rRes = pData->pBasicFormater->BasicFormatNull( *pFmt );
                }

                // Die alte Implementierung:
                //old: printfmtnum( GetDouble(), rRes, *pFmt );
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
                // #45355 wenn es numerisch ist, muss gewandelt werden
                if( IsNumericRTL() )
                {
                    ScanNumIntnl( GetString(), d, /*bSingle*/sal_False );
                    goto cvt2;
                }
                else
                {
                    // Sonst String-Formatierung
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
