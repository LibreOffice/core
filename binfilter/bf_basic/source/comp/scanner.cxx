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

#include "scanner.hxx"
#include "sbintern.hxx"
#include "token.hxx"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#if defined UNX
#include <stdlib.h>
#else
#include <math.h>   // atof()
#endif
#include <rtl/math.hxx>
#include <vcl/svapp.hxx>
#include <unotools/charclass.hxx>

namespace binfilter {

SbiScanner::SbiScanner( const ::rtl::OUString& rBuf, StarBASIC* p ) : aBuf( rBuf )
{
    pBasic   = p;
    pLine    = NULL;
    nVal	 = 0;
    eScanType = SbxVARIANT;
    nErrors  = 0;
    nBufPos  = 0;
    nCurCol1 = 0;
    nSavedCol1 = 0;
    nColLock = 0;
    nLine	 = 0;
    nCol1	 = 0;
    nCol2	 = 0;
    nCol     = 0;
    bError	 =
    bAbort   =
    bSpaces  =
    bNumber  =
    bSymbol  =
    bUsedForHilite =
    bCompatible = 
    bPrevLineExtentsComment = FALSE;
    bHash    =
    bErrors  = TRUE;
}

SbiScanner::~SbiScanner()
{}

/*?*/ // void SbiScanner::LockColumn()
/*?*/ // {
/*?*/ // 	if( !nColLock++ )
/*?*/ // 		nSavedCol1 = nCol1;
/*?*/ // }
/*?*/ // 
/*?*/ // void SbiScanner::UnlockColumn()
/*?*/ // {
/*?*/ // 	if( nColLock )
/*?*/ // 		nColLock--;
/*?*/ // }
/*?*/ // 
void SbiScanner::GenError( SbError code )
{
    if( GetSbData()->bBlockCompilerError )
    {
        bAbort = TRUE;
        return;
    }
    if( !bError && bErrors )
    {
        BOOL bRes = TRUE;
        // Nur einen Fehler pro Statement reporten
        bError = TRUE;
        if( pBasic )
        {
            // Falls EXPECTED oder UNEXPECTED kommen sollte, bezieht es sich
            // immer auf das letzte Token, also die Col1 uebernehmen
            USHORT nc = nColLock ? nSavedCol1 : nCol1;
            switch( code )
            {
                case SbERR_EXPECTED:
                case SbERR_UNEXPECTED:
                case SbERR_SYMBOL_EXPECTED:
                case SbERR_LABEL_EXPECTED:
                    nc = nCol1;
                    if( nc > nCol2 ) nCol2 = nc;
                    break;
            }
            bRes = pBasic->CError( code, aError, nLine, nc, nCol2 );
        }
        bAbort |= !bRes |
             ( code == SbERR_NO_MEMORY || code == SbERR_PROG_TOO_LARGE );
    }
    if( bErrors )
        nErrors++;
}

// Testen auf ein legales Suffix

static SbxDataType GetSuffixType( sal_Unicode c )
{
    static String aSuffixesStr = String::CreateFromAscii( "%&!#@ $" );
    if( c )
    {
        sal_uInt32 n = aSuffixesStr.Search( c );
        if( STRING_NOTFOUND != n && c != ' ' )
            return SbxDataType( (USHORT) n + SbxINTEGER );
    }
    return SbxVARIANT;
}

// Einlesen des naechsten Symbols in die Variablen aSym, nVal und eType
// Returnwert ist FALSE bei EOF oder Fehlern
#define BUF_SIZE 80

BOOL SbiScanner::NextSym()
{
    // Fuer den EOLN-Fall merken
    USHORT nOldLine = nLine;
    USHORT nOldCol1 = nCol1;
    USHORT nOldCol2 = nCol2;
    sal_Unicode buf[ BUF_SIZE ], *p = buf;
    bHash = FALSE;

    eScanType = SbxVARIANT;
    aSym.Erase();
    bSymbol =
    bNumber = bSpaces = FALSE;

    // Zeile einlesen?
    if( !pLine )
    {
        INT32 n = nBufPos;
        INT32 nLen = aBuf.getLength();
        if( nBufPos >= nLen )
            return FALSE;
        const sal_Unicode* p2 = aBuf.getStr();
        p2 += n;
        while( ( n < nLen ) && ( *p2 != '\n' ) && ( *p2 != '\r' ) )
            p2++, n++;
        aLine = aBuf.copy( nBufPos, n - nBufPos );
        if( n < nLen )
        {
            if( *p2 == '\r' && *( p2+1 ) == '\n' )
                n += 2;
            else
                n++;
        }
        nBufPos = n;
        pLine = aLine.getStr();
        nOldLine = ++nLine;
        nCol = nCol1 = nCol2 = nOldCol1 = nOldCol2 = 0;
        nColLock = 0;
    }

    // Leerstellen weg:
    while( *pLine && (( *pLine == ' ' ) || ( *pLine == '\t' ) || ( *pLine == '\f' )) )
        pLine++, nCol++, bSpaces = TRUE;

    nCol1 = nCol;

    // nur Leerzeile?
    if( !*pLine )
        goto eoln;

    if( bPrevLineExtentsComment )
        goto PrevLineCommentLbl;

    if( *pLine == '#' )
    {
        pLine++;
        nCol++;
        bHash = TRUE;
    }

    // Symbol? Dann Zeichen kopieren.
    if( BasicSimpleCharClass::isAlpha( *pLine, bCompatible ) || *pLine == '_' )
    {
        // Wenn nach '_' nichts kommt, ist es ein Zeilenabschluss!
        if(	*pLine == '_' && !*(pLine+1) )
        {	pLine++;
            goto eoln;	}
        bSymbol = TRUE;
        short n = nCol;
        for ( ; (BasicSimpleCharClass::isAlphaNumeric( *pLine, bCompatible ) || ( *pLine == '_' ) ); pLine++ )
            nCol++;
        aSym = aLine.copy( n, nCol - n );
        // Abschliessendes '_' durch Space ersetzen, wenn Zeilenende folgt
        // (sonst falsche Zeilenfortsetzung)
        if(	!bUsedForHilite && !*pLine && *(pLine-1) == '_' )
        {
            aSym.GetBufferAccess();		// #109693 force copy if necessary
            *((sal_Unicode*)(pLine-1)) = ' ';		// cast wegen const
        }
        // Typkennung?
        // Das Ausrufezeichen bitte nicht testen, wenn
        // danach noch ein Symbol anschliesst
        else if( *pLine != '!' || !BasicSimpleCharClass::isAlpha( pLine[ 1 ], bCompatible ) )
        {
            SbxDataType t = GetSuffixType( *pLine );
            if( t != SbxVARIANT )
            {
                eScanType = t;
                pLine++;
                nCol++;
            }
        }
    }

    // Zahl? Dann einlesen und konvertieren.
    else if( BasicSimpleCharClass::isDigit( *pLine & 0xFF )
        || ( *pLine == '.' && BasicSimpleCharClass::isDigit( *(pLine+1) & 0xFF ) ) )
    {
        short exp = 0;
        short comma = 0;
        short ndig = 0;
        short ncdig = 0;
        eScanType = SbxDOUBLE;
        BOOL bBufOverflow = FALSE;
        while( strchr( "0123456789.DEde", *pLine ) && *pLine )
        {
            // AB 4.1.1996: Buffer voll? -> leer weiter scannen
            if( (p-buf) == (BUF_SIZE-1) )
            {
                bBufOverflow = TRUE;
                pLine++, nCol++;
                continue;
            }
            // Komma oder Exponent?
            if( *pLine == '.' )
            {
                if( ++comma > 1 )
                {
                    pLine++; nCol++; continue;
                }
                else *p++ = *pLine++, nCol++;
            }
            else if( strchr( "DdEe", *pLine ) )
            {
                if (++exp > 1)
                {
                    pLine++; nCol++; continue;
                }
//              if( toupper( *pLine ) == 'D' )
//                  eScanType = SbxDOUBLE;
                *p++ = 'E'; pLine++; nCol++;
                // Vorzeichen hinter Exponent?
                if( *pLine == '+' )
                    pLine++, nCol++;
                else
                if( *pLine == '-' )
                    *p++ = *pLine++, nCol++;
            }
            else
            {
                *p++ = *pLine++, nCol++;
                if( comma && !exp ) ncdig++;
            }
            if (!exp) ndig++;
        }
        *p = 0;
        aSym = p; bNumber = TRUE;
        // Komma, Exponent mehrfach vorhanden?
        if( comma > 1 || exp > 1 )
        {	aError = '.';
            GenError( SbERR_BAD_CHAR_IN_NUMBER );	}

        // #57844 Lokalisierte Funktion benutzen
        nVal = rtl_math_uStringToDouble( buf, buf+(p-buf), '.', ',', NULL, NULL );
        // ALT: nVal = atof( buf );

        ndig = ndig - comma;
        if( !comma && !exp )
        {
            if( nVal >= SbxMININT && nVal <= SbxMAXINT )
                eScanType = SbxINTEGER;
            else
            if( nVal >= SbxMINLNG && nVal <= SbxMAXLNG )
                eScanType = SbxLONG;
        }
        if( bBufOverflow )
            GenError( SbERR_MATH_OVERFLOW );
        // zu viele Zahlen fuer SINGLE?
//      if (ndig > 15 || ncdig > 6)
//          eScanType = SbxDOUBLE;
//      else
//      if( nVal > SbxMAXSNG || nVal < SbxMINSNG )
//          eScanType = SbxDOUBLE;

        // Typkennung?
        SbxDataType t = GetSuffixType( *pLine );
        if( t != SbxVARIANT )
        {
            eScanType = t;
            pLine++;
            nCol++;
        }
    }

    // Hex/Oktalzahl? Einlesen und konvertieren:
    else if( *pLine == '&' )
    {
        pLine++; nCol++;
        sal_Unicode cmp1[] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F', 0 };
        sal_Unicode cmp2[] = { '0', '1', '2', '3', '4', '5', '6', '7', 0 };
        sal_Unicode *cmp = cmp1;
        //char *cmp = "0123456789ABCDEF";
        sal_Unicode base = 16;
        sal_Unicode ndig = 8;
        sal_Unicode xch  = *pLine++ & 0xFF; nCol++;
        switch( toupper( xch ) )
        {
            case 'O':
                cmp = cmp2; base = 8; ndig = 11; break;
                //cmp = "01234567"; base = 8; ndig = 11; break;
            case 'H':
                break;
            default :
                // Wird als Operator angesehen
                pLine--; nCol--; nCol1 = nCol-1; aSym = '&'; return SYMBOL;
        }
        bNumber = TRUE;
        long l = 0;
        int i;
        BOOL bBufOverflow = FALSE;
        while( BasicSimpleCharClass::isAlphaNumeric( *pLine & 0xFF, bCompatible ) )
        {
            sal_Unicode ch = sal::static_int_cast< sal_Unicode >(
                toupper( *pLine & 0xFF ) );
            pLine++; nCol++;
            // AB 4.1.1996: Buffer voll, leer weiter scannen
            if( (p-buf) == (BUF_SIZE-1) )
                bBufOverflow = TRUE;
            else if( String( cmp ).Search( ch ) != STRING_NOTFOUND )
            //else if( strchr( cmp, ch ) )
                *p++ = ch;
            else
            {
                aError = ch;
                GenError( SbERR_BAD_CHAR_IN_NUMBER );
            }
        }
        *p = 0;
        for( p = buf; *p; p++ )
        {
            i = (*p & 0xFF) - '0';
            if( i > 9 ) i -= 7;
            l = ( l * base ) + i;
            if( !ndig-- )
            {
                GenError( SbERR_MATH_OVERFLOW ); break;
            }
        }
        if( *pLine == '&' ) pLine++, nCol++;
        nVal = (double) l;
        eScanType = ( l >= SbxMININT && l <= SbxMAXINT ) ? SbxINTEGER : SbxLONG;
        if( bBufOverflow )
            GenError( SbERR_MATH_OVERFLOW );
    }

    // Strings:
    else if( *pLine == '"' || *pLine == '[' )
    {
        sal_Unicode cSep = *pLine;
        if( cSep == '[' )
            bSymbol = TRUE, cSep = ']';
        short n = nCol+1;
        while( *pLine )
        {
            do pLine++, nCol++;
            while( *pLine && ( *pLine != cSep ) );
            if( *pLine == cSep )
            {
                pLine++; nCol++;
                if( *pLine != cSep || cSep == ']' ) break;
            } else aError = cSep, GenError( SbERR_EXPECTED );
        }
        aSym = aLine.copy( n, nCol - n - 1 );
        // Doppelte Stringbegrenzer raus
        String s( cSep );
        s += cSep;
        USHORT nIdx = 0;
        do 
        {
            nIdx = aSym.Search( s, nIdx );
            if( nIdx == STRING_NOTFOUND )
                break;
            aSym.Erase( nIdx, 1 );
            nIdx++;
        } 
        while( true );
        if( cSep != ']' )
            eScanType = ( cSep == '#' ) ? SbxDATE : SbxSTRING;
    }
    // ungueltige Zeichen:
    else if( ( *pLine & 0xFF ) >= 0x7F )
    {
        GenError( SbERR_SYNTAX ); pLine++; nCol++;
    }
    // andere Gruppen:
    else
    {
        short n = 1;
        switch( *pLine++ )
        {
            case '<': if( *pLine == '>' || *pLine == '=' ) n = 2; break;
            case '>': if( *pLine == '=' ) n = 2; break;
            case ':': if( *pLine == '=' ) n = 2; break;
        }
        aSym = aLine.copy( nCol, n );
        pLine += n-1; nCol = nCol + n;
    }

    nCol2 = nCol-1;

PrevLineCommentLbl:
    // Kommentar?
    if( bPrevLineExtentsComment || (eScanType != SbxSTRING &&
        ( aSym.GetBuffer()[0] == '\'' || aSym.EqualsIgnoreCaseAscii( "REM" ) ) ) )
    {
        bPrevLineExtentsComment = FALSE;
        aSym = String::CreateFromAscii( "REM" );
        USHORT nLen = String( pLine ).Len();
        if( bCompatible && pLine[ nLen - 1 ] == '_' && pLine[ nLen - 2 ] == ' ' )
            bPrevLineExtentsComment = TRUE;
        nCol2 = nCol2 + nLen;
        pLine = NULL;
    }
    return TRUE;

    // Sonst Zeilen-Ende: aber bitte auf '_' testen, ob die
    // Zeile nicht weitergeht!
eoln:
    if( nCol && *--pLine == '_' )
    {
        pLine = NULL; return NextSym();
    }
    else
    {
        pLine = NULL;
        nLine = nOldLine;
        nCol1 = nOldCol1;
        nCol2 = nOldCol2;
        aSym = '\n';
        nColLock = 0;
        return TRUE;
    }
}

LetterTable BasicSimpleCharClass::aLetterTable;

LetterTable::LetterTable( void )
{
    for( int i = 0 ; i < 256 ; ++i )
        IsLetterTab[i] = false;

    IsLetterTab[0xC0] = true;	// À , CAPITAL LETTER A WITH GRAVE ACCENT
    IsLetterTab[0xC1] = true;	// Á , CAPITAL LETTER A WITH ACUTE ACCENT
    IsLetterTab[0xC2] = true;	// Â , CAPITAL LETTER A WITH CIRCUMFLEX ACCENT
    IsLetterTab[0xC3] = true;	// Ã , CAPITAL LETTER A WITH TILDE
    IsLetterTab[0xC4] = true;	// Ä , CAPITAL LETTER A WITH DIAERESIS
    IsLetterTab[0xC5] = true;	// Å , CAPITAL LETTER A WITH RING ABOVE
    IsLetterTab[0xC6] = true;	// Æ , CAPITAL LIGATURE AE
    IsLetterTab[0xC7] = true;	// Ç , CAPITAL LETTER C WITH CEDILLA
    IsLetterTab[0xC8] = true;	// È , CAPITAL LETTER E WITH GRAVE ACCENT
    IsLetterTab[0xC9] = true;	// É , CAPITAL LETTER E WITH ACUTE ACCENT
    IsLetterTab[0xCA] = true;	// Ê , CAPITAL LETTER E WITH CIRCUMFLEX ACCENT
    IsLetterTab[0xCB] = true;	// Ë , CAPITAL LETTER E WITH DIAERESIS
    IsLetterTab[0xCC] = true;	// Ì , CAPITAL LETTER I WITH GRAVE ACCENT
    IsLetterTab[0xCD] = true;	// Í , CAPITAL LETTER I WITH ACUTE ACCENT
    IsLetterTab[0xCE] = true;	// Î , CAPITAL LETTER I WITH CIRCUMFLEX ACCENT
    IsLetterTab[0xCF] = true;	// Ï , CAPITAL LETTER I WITH DIAERESIS
    IsLetterTab[0xD0] = true;	// Ð , CAPITAL LETTER ETH
    IsLetterTab[0xD1] = true;	// Ñ , CAPITAL LETTER N WITH TILDE
    IsLetterTab[0xD2] = true;	// Ò , CAPITAL LETTER O WITH GRAVE ACCENT
    IsLetterTab[0xD3] = true;	// Ó , CAPITAL LETTER O WITH ACUTE ACCENT
    IsLetterTab[0xD4] = true;	// Ô , CAPITAL LETTER O WITH CIRCUMFLEX ACCENT
    IsLetterTab[0xD5] = true;	// Õ , CAPITAL LETTER O WITH TILDE
    IsLetterTab[0xD6] = true;	// Ö , CAPITAL LETTER O WITH DIAERESIS
    IsLetterTab[0xD8] = true;	// Ø , CAPITAL LETTER O WITH STROKE
    IsLetterTab[0xD9] = true;	// Ù , CAPITAL LETTER U WITH GRAVE ACCENT
    IsLetterTab[0xDA] = true;	// Ú , CAPITAL LETTER U WITH ACUTE ACCENT
    IsLetterTab[0xDB] = true;	// Û , CAPITAL LETTER U WITH CIRCUMFLEX ACCENT
    IsLetterTab[0xDC] = true;	// Ü , CAPITAL LETTER U WITH DIAERESIS
    IsLetterTab[0xDD] = true;	// Ý , CAPITAL LETTER Y WITH ACUTE ACCENT
    IsLetterTab[0xDE] = true;	// Þ , CAPITAL LETTER THORN
    IsLetterTab[0xDF] = true;	// ß , SMALL LETTER SHARP S
    IsLetterTab[0xE0] = true;	// à , SMALL LETTER A WITH GRAVE ACCENT
    IsLetterTab[0xE1] = true;	// á , SMALL LETTER A WITH ACUTE ACCENT
    IsLetterTab[0xE2] = true;	// â , SMALL LETTER A WITH CIRCUMFLEX ACCENT
    IsLetterTab[0xE3] = true;	// ã , SMALL LETTER A WITH TILDE
    IsLetterTab[0xE4] = true;	// ä , SMALL LETTER A WITH DIAERESIS
    IsLetterTab[0xE5] = true;	// å , SMALL LETTER A WITH RING ABOVE
    IsLetterTab[0xE6] = true;	// æ , SMALL LIGATURE AE
    IsLetterTab[0xE7] = true;	// ç , SMALL LETTER C WITH CEDILLA
    IsLetterTab[0xE8] = true;	// è , SMALL LETTER E WITH GRAVE ACCENT
    IsLetterTab[0xE9] = true;	// é , SMALL LETTER E WITH ACUTE ACCENT
    IsLetterTab[0xEA] = true;	// ê , SMALL LETTER E WITH CIRCUMFLEX ACCENT
    IsLetterTab[0xEB] = true;	// ë , SMALL LETTER E WITH DIAERESIS
    IsLetterTab[0xEC] = true;	// ì , SMALL LETTER I WITH GRAVE ACCENT
    IsLetterTab[0xED] = true;	// í , SMALL LETTER I WITH ACUTE ACCENT
    IsLetterTab[0xEE] = true;	// î , SMALL LETTER I WITH CIRCUMFLEX ACCENT
    IsLetterTab[0xEF] = true;	// ï , SMALL LETTER I WITH DIAERESIS
    IsLetterTab[0xF0] = true;	// ð , SMALL LETTER ETH
    IsLetterTab[0xF1] = true;	// ñ , SMALL LETTER N WITH TILDE
    IsLetterTab[0xF2] = true;	// ò , SMALL LETTER O WITH GRAVE ACCENT
    IsLetterTab[0xF3] = true;	// ó , SMALL LETTER O WITH ACUTE ACCENT
    IsLetterTab[0xF4] = true;	// ô , SMALL LETTER O WITH CIRCUMFLEX ACCENT
    IsLetterTab[0xF5] = true;	// õ , SMALL LETTER O WITH TILDE
    IsLetterTab[0xF6] = true;	// ö , SMALL LETTER O WITH DIAERESIS
    IsLetterTab[0xF8] = true;	// ø , SMALL LETTER O WITH OBLIQUE BAR
    IsLetterTab[0xF9] = true;	// ù , SMALL LETTER U WITH GRAVE ACCENT
    IsLetterTab[0xFA] = true;	// ú , SMALL LETTER U WITH ACUTE ACCENT
    IsLetterTab[0xFB] = true;	// û , SMALL LETTER U WITH CIRCUMFLEX ACCENT
    IsLetterTab[0xFC] = true;	// ü , SMALL LETTER U WITH DIAERESIS
    IsLetterTab[0xFD] = true;	// ý , SMALL LETTER Y WITH ACUTE ACCENT
    IsLetterTab[0xFE] = true;	// þ , SMALL LETTER THORN
    IsLetterTab[0xFF] = true;	// ÿ , SMALL LETTER Y WITH DIAERESIS
}

bool LetterTable::isLetterUnicode( sal_Unicode c )
{
    static CharClass* pCharClass = NULL;
    if( pCharClass == NULL )
        pCharClass = new CharClass( Application::GetSettings().GetLocale() );
    String aStr( c );
    bool bRet = pCharClass->isLetter( aStr, 0 );
    return bRet;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
