/*************************************************************************
 *
 *  $RCSfile: scanner.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:12:10 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "sbcomp.hxx"
#pragma hdrstop
#include <ctype.h>
#include <stdio.h>  // sprintf()
#include <string.h>
#if defined (ICC) || defined (WTC) || defined(__powerc) || defined ( MAC ) || defined UNX
#include <stdlib.h>
#else
#include <math.h>   // atof()
#endif
#ifndef _SOLMATH_HXX //autogen wg. SolarMath
#include <tools/solmath.hxx>
#endif
#ifndef _TOOLS_INTN_HXX
#include <tools/intn.hxx>
#endif

#include "segmentc.hxx"
#pragma SW_SEGMENT_CLASS( SBCOMP, SBCOMP_CODE )

SbiScanner::SbiScanner( const String& rBuf, StarBASIC* p ) : aBuf( rBuf )
{
    pBasic   = p;
    pLine    = NULL;
    nVal     = 0;
    eScanType = SbxVARIANT;
    nErrors  =
    nBufPos  =
    nCurCol1 =
    nSavedCol1 =
    nColLock =
    nLine    =
    nCol1    =
    nCol2    =
    nCol     = 0;
    bError   =
    bAbort   =
    bSpaces  =
    bNumber  =
    bSymbol  =
    bUsedForHilite = FALSE;
    bHash    =
    bErrors  = TRUE;
}

SbiScanner::~SbiScanner()
{}

void SbiScanner::LockColumn()
{
    if( !nColLock++ )
        nSavedCol1 = nCol1;
}

void SbiScanner::UnlockColumn()
{
    if( nColLock )
        nColLock--;
}

void SbiScanner::GenError( SbError code )
{
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

// Falls sofort ein Doppelpunkt folgt, wird TRUE zurueckgeliefert.
// Wird von SbiTokenizer::MayBeLabel() verwendet, um einen Label zu erkennen

BOOL SbiScanner::DoesColonFollow()
{
    if( pLine && *pLine == ':' )
    {
        pLine++; nCol++; return TRUE;
    }
    else return FALSE;
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
    static International aEnglischIntn( LANGUAGE_ENGLISH_US, LANGUAGE_ENGLISH_US );

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
        USHORT n = nBufPos;
        USHORT nLen = aBuf.Len();
        if( nBufPos >= nLen )
            return FALSE;
        const sal_Unicode* p = aBuf.GetBuffer();
        p += n;
        while( ( n < nLen ) && ( *p != '\n' ) && ( *p != '\r' ) )
            p++, n++;
        aLine = aBuf.Copy( nBufPos, n - nBufPos );
        if( ( n < nLen ) && *p == '\r' && *( p+1 ) == '\n' )
            n += 2;
        else
            n++;
        nBufPos = n;
        pLine = aLine.GetBuffer();
        nOldLine = ++nLine;
        nCol = nCol1 = nCol2 = nOldCol1 = nOldCol2 = 0;
        nColLock = 0;
    }

    // Leerstellen weg:
    while( *pLine && ( *pLine == ' ' ) || ( *pLine == '\t' ) || ( *pLine == '\f' ) )
        pLine++, nCol++, bSpaces = TRUE;

    nCol1 = nCol;

    // nur Leerzeile?
    if( !*pLine ) goto eoln;

    if( *pLine == '#' ) pLine++, nCol++, bHash = TRUE;

    // Symbol? Dann Zeichen kopieren.
    if( isalpha( *pLine & 0xFF ) || *pLine == '_' )
    {
        // Wenn nach '_' nichts kommt, ist es ein Zeilenabschluss!
        if( *pLine == '_' && !*(pLine+1) )
        {   pLine++;
            goto eoln;  }
        bSymbol = TRUE;
        short n = nCol;
        for ( ; (isalnum( *pLine & 0xFF ) || ( *pLine == '_' ) ); pLine++ )
            nCol++;
        aSym = aLine.Copy( n, nCol - n );
        // Abschliessendes '_' durch Space ersetzen, wenn Zeilenende folgt
        // (sonst falsche Zeilenfortsetzung)
        if( !bUsedForHilite && !*pLine && *(pLine-1) == '_' )
            *((sal_Unicode*)(pLine-1)) = ' ';       // cast wegen const
        // Typkennung?
        // Das Ausrufezeichen bitte nicht testen, wenn
        // danach noch ein Symbol anschliesst
        else if( *pLine != '!' || !isalpha( pLine[ 1 ] & 0xFF ) )
        {
            SbxDataType t = GetSuffixType( *pLine );
            if( t != SbxVARIANT )
            {
                eScanType = t;
                pLine++;
                nCol++;
#ifdef VBSCRIPT_TEST
                nCol2 = nCol;
                GenError( ERRCODE_BASIC_NOT_IN_VBSCRIPT );
#endif
            }
        }
    }

    // Zahl? Dann einlesen und konvertieren.
    else if( isdigit( *pLine & 0xFF )
        || ( *pLine == '.' && isdigit( *(pLine+1) & 0xFF ) ) )
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
        {   aError = '.';
            GenError( SbERR_BAD_CHAR_IN_NUMBER );   }

        // #57844 Lokalisierte Funktion benutzen
        int nErrno;
        nVal = SolarMath::StringToDouble( buf, aEnglischIntn, nErrno );
        // ATL: nVal = atof( buf );

        ndig -= comma;
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
#ifdef VBSCRIPT_TEST
            nCol2 = nCol;
            GenError( ERRCODE_BASIC_NOT_IN_VBSCRIPT );
#endif
        }
    }

    // Hex/Oktalzahl? Einlesen und konvertieren:
    else if( *pLine == '&' )
    {
        pLine++; nCol++;
        sal_Unicode cmp1[] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' };
        sal_Unicode cmp2[] = { '0', '1', '2', '3', '4', '5', '6', '7' };
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
        while( isalnum( *pLine & 0xFF ) )
        {
            sal_Unicode ch = toupper( *pLine & 0xFF );
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
        aSym = aLine.Copy( n, nCol - n - 1 );
        // Doppelte Stringbegrenzer raus
        String s( cSep );
        s += cSep;
        USHORT nIdx;
        do {
            nIdx = aSym.Search( s );
            aSym.Erase( nIdx, 1 );
        } while( nIdx != STRING_NOTFOUND );
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
        aSym = aLine.Copy( nCol, n );
        pLine += n-1; nCol += n;
    }

    nCol2 = nCol-1;

    // Kommentar?
    if( eScanType != SbxSTRING &&
        ( aSym.GetBuffer()[0] == '\'' || aSym.EqualsIgnoreCaseAscii( "REM" ) ) )
    {
        aSym = String::CreateFromAscii( "REM" );
        nCol2 += String( pLine ).Len();
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

