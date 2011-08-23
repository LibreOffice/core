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

#include <tools/errcode.hxx>
#include "sbx.hxx"
#include "sbxconv.hxx"

#include "bf_svtools/syslocale.hxx"

#if defined ( UNX )
#include <stdlib.h>
#endif

#include <vcl/svapp.hxx>
#include <math.h>
#include <string.h>
#include <ctype.h>

#include "sbxres.hxx"
#include "sbxbase.hxx"
#include <bf_svtools/svtools.hrc>

namespace binfilter {

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

SbxError ImpScan( const XubString& rWSrc, double& nVal, SbxDataType& rType,
                  USHORT* pLen, BOOL bAllowIntntl, BOOL bOnlyIntntl )
{
    ByteString aBStr( rWSrc, RTL_TEXTENCODING_ASCII_US );

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
        cIntntl1000 = cNonIntntlComma;	// Unschaedlich machen
    }
    // Nur International -> IntnlComma uebernehmen
    if( bOnlyIntntl )
    {
        cNonIntntlComma = cIntntlComma;
        cIntntl1000 = (char)cThousandSep;
    }

    const char* pStart = aBStr.GetBuffer();
    const char* p = pStart;
    char buf[ 80 ], *q = buf;
    BOOL bRes = TRUE;
    BOOL bMinus = FALSE;
    nVal = 0;
    SbxDataType eScanType = SbxSINGLE;
    // Whitespace wech
    while( *p &&( *p == ' ' || *p == '\t' ) ) p++;
    // Zahl? Dann einlesen und konvertieren.
    if( *p == '-' )
        p++, bMinus = TRUE;
    if( isdigit( *p ) ||( (*p == cNonIntntlComma || *p == cIntntlComma ||
            *p == cIntntl1000) && isdigit( *(p+1 ) ) ) )
    {
        short exp = 0;		// >0: Exponentteil
        short comma = 0;	// >0: Nachkomma
        short ndig = 0;		// Anzahl Ziffern
        short ncdig = 0;	// Anzahl Ziffern nach Komma
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
            bRes = FALSE;
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
            default : bRes = FALSE;
        }
        long l = 0;
        int i;
        while( isalnum( *p ) )
        {
            char ch = sal::static_int_cast< char >( toupper( *p ) );
            p++;
            if( strchr( cmp, ch ) ) *q++ = ch;
            else bRes = FALSE;
        }
        *q = 0;
        for( q = buf; *q; q++ )
        {
            i =( *q & 0xFF ) - '0';
            if( i > 9 ) i -= 7;
            l =( l * base ) + i;
            if( !ndig-- )
                bRes = FALSE;
        }
        if( *p == '&' ) p++;
        nVal = (double) l;
        if( l >= SbxMININT && l <= SbxMAXINT )
            eScanType = SbxINTEGER;
    }
    if( pLen )
        *pLen = (USHORT) ( p - pStart );
    if( !bRes )
        return SbxERR_CONVERSION;
    if( bMinus )
        nVal = -nVal;
    rType = eScanType;
    return SbxERR_OK;
}

////////////////////////////////////////////////////////////////////////////

static double roundArray[] = {
    5.0e+0, 0.5e+0,	0.5e-1,	0.5e-2,	0.5e-3,	0.5e-4,	0.5e-5,	0.5e-6,	0.5e-7,
    0.5e-8,	0.5e-9,	0.5e-10,0.5e-11,0.5e-12,0.5e-13,0.5e-14,0.5e-15 };

/***************************************************************************
|*
|*	void myftoa( double, char *, short, short, BOOL, BOOL )
|*
|*	Beschreibung:		Konversion double --> ASCII
|*	Parameter:			double				die Zahl.
|*						char *				der Zielpuffer
|*						short				Anzahl Nachkommastellen
|*						short				Weite des Exponenten( 0=kein E )
|*						BOOL				TRUE: mit 1000er Punkten
|*						BOOL				TRUE: formatfreie Ausgabe
|*
***************************************************************************/

static void myftoa( double nNum, char * pBuf, short nPrec, short nExpWidth,
                    BOOL bPt, BOOL bFix, sal_Unicode cForceThousandSep = 0 )
{

    short nExp = 0;						// Exponent
    short nDig = nPrec + 1;				// Anzahl Digits in Zahl
    short nDec;							// Anzahl Vorkommastellen
    register int i, digit;

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
            while( i-- )	*pBuf++ = '0';
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

void ImpCvtNum( double nNum, short nPrec, XubString& rRes, BOOL bCoreString )
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
    myftoa( nNum, p, nPrec,( nNum &&( nNum < 1E-1 || nNum > dMaxNumWithoutExp ) ) ? 4:0,
        FALSE, TRUE, cDecimalSep );
    // Trailing Zeroes weg:
    for( p = cBuf; *p &&( *p != 'E' ); p++ ) {}
    q = p; p--;
    while( nPrec && *p == '0' ) nPrec--, p--;
    if( *p == cDecimalSep ) p--;
    while( *q ) *++p = *q++;
    *++p = 0;
    rRes = String::CreateFromAscii( cBuf );
}

#ifdef _MSC_VER
#pragma optimize( "", on )
#endif

// Formatierte Zahlenausgabe
// Der Returnwert ist die Anzahl Zeichen, die aus dem
// Format verwendt wurden.

#ifdef _old_format_code_
// lasse diesen Code vorl"aufig drin, zum 'abgucken'
// der bisherigen Implementation

static USHORT printfmtnum( double nNum, XubString& rRes, const XubString& rWFmt )
{
    const String& rFmt = rWFmt;
    char	cFill  = ' ';			// Fuellzeichen
    char	cPre   = 0;				// Startzeichen( evtl. "$" )
    short	nExpDig= 0;				// Anzahl Exponentstellen
    short	nPrec  = 0;				// Anzahl Nachkommastellen
    short	nWidth = 0;				// Zahlenweite gesamnt
    short	nLen;					// Laenge konvertierte Zahl
    BOOL	bPoint = FALSE;			// TRUE: mit 1000er Kommas
    BOOL	bTrail = FALSE;			// TRUE, wenn folgendes Minus
    BOOL	bSign  = FALSE;			// TRUE: immer mit Vorzeichen
    BOOL	bNeg   = FALSE;			// TRUE: Zahl ist negativ
    char	cBuf [1024];			// Zahlenpuffer
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
            bSign = TRUE; nWidth++; break;
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
            nWidth++; pFmt++; bPoint = TRUE;
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
        pFmt++, bTrail = TRUE;

    // Zahl konvertieren:
    if( nPrec > 15 ) nPrec = 15;
    if( nNum < 0.0 ) nNum = -nNum, bNeg = TRUE;
    p = cBuf;
    if( bSign ) *p++ = bNeg ? '-' : '+';
    myftoa( nNum, p, nPrec, nExpDig, bPoint, FALSE );
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

    return (USHORT) ( pFmt - (const char*) rFmt );
}

#endif //_old_format_code_

/////////////////////////////////////////////////////////////////////////

BOOL SbxValue::Scan( const XubString& rSrc, USHORT* pLen )
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
        SetError( eRes ); return FALSE;
    }
    else
        return TRUE;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
