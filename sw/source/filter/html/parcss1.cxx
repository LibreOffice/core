/*************************************************************************
 *
 *  $RCSfile: parcss1.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:56 $
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

#ifdef PRECOMPILED
#include "filt_pch.hxx"
#endif

#pragma hdrstop

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif
#ifndef _INTN_HXX //autogen
#include <tools/intn.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_COLOR_HXX //autogen
#include <vcl/color.hxx>
#endif
#ifndef _HTMLTOKN_H
#include <svtools/htmltokn.h>
#endif



#include "css1kywd.hxx"
#include "parcss1.hxx"


// Loop-Check: Um Endlos-Schleifen zu vermeiden, wird in jeder
// Schalife geprueft, ob ein Fortschritt in der Eingabe-Position
// stattgefunden hat
#define LOOP_CHECK

#ifdef LOOP_CHECK

#define LOOP_CHECK_DECL \
    xub_StrLen nOldInPos = STRING_MAXLEN;
#define LOOP_CHECK_RESTART \
    nOldInPos = STRING_MAXLEN;
#define LOOP_CHECK_CHECK( where ) \
    DBG_ASSERT( nOldInPos!=nInPos || cNextCh==(sal_Unicode)EOF, where );    \
    if( nOldInPos==nInPos && cNextCh!=(sal_Unicode)EOF )                    \
        break;                                                              \
    else                                                                    \
        nOldInPos = nInPos;

#else

#define LOOP_CHECK_DECL
#define LOOP_CHECK_RESTART
#define LOOP_CHECK_CHECK( where )

#endif



const sal_Int32 MAX_LEN = 1024;

/*  */

void CSS1Parser::InitRead( const String& rIn )
{
    nlLineNr = 0;
    nlLinePos = 0;

    bWhiteSpace = TRUE; // Wenn noch nichts gelesen wurde ist das wie WS
    bEOF = FALSE;
    eState = CSS1_PAR_WORKING;
    nValue = 0.;

    aIn = rIn;
    nInPos = 0;
    cNextCh = GetNextChar();
    nToken = GetNextToken();
}

sal_Unicode CSS1Parser::GetNextChar()
{
    if( nInPos >= aIn.Len() )
    {
        bEOF = TRUE;
        return (sal_Unicode)EOF;
    }

    sal_Unicode c = aIn.GetChar( nInPos );
    nInPos++;

    if( c == '\n' )
    {
        IncLineNr();
        SetLinePos( 1L );
    }
    else
        IncLinePos();

    return c;
}

/*  */

// Diese Funktion realisiert den in
//
//      http://www.w3.orh/pub/WWW/TR/WD-css1.html
// bzw. http://www.w3.orh/pub/WWW/TR/WD-css1-960220.html
//
// beschriebenen Scanner fuer CSS1. Es handelt sich um eine direkte
// Umsetzung der dort beschriebenen Lex-Grammatik
//
CSS1Token CSS1Parser::GetNextToken()
{
    CSS1Token nRet = CSS1_NULL;
    aToken.Erase();

    do {
        // Merken, ob davor White-Space gelesen wurde
        BOOL bPrevWhiteSpace = bWhiteSpace;
        bWhiteSpace = FALSE;

        BOOL bNextCh = TRUE;
        switch( cNextCh )
        {
        case '/': // COMMENT | '/'
            {
                cNextCh = GetNextChar();
                if( '*' == cNextCh )
                {
                    // COMMENT
                    cNextCh = GetNextChar();

                    BOOL bAsterix = FALSE;
                    while( !(bAsterix && '/'==cNextCh) && !IsEOF() )
                    {
                        bAsterix = ('*'==cNextCh);
                        cNextCh = GetNextChar();
                    }
                }
                else
                {
                    // '/'
                    bNextCh = FALSE;
                    nRet = CSS1_SLASH;
                }
            }
            break;

        case '@': // '@import' | '@XXX'
            {
                cNextCh = GetNextChar();
                if( ('A' <= cNextCh && cNextCh <= 'Z') ||
                    ('a' <= cNextCh && cNextCh <= 'z') )
                {
                    // den naechsten Identifer scannen
                    ::rtl::OUStringBuffer sTmpBuffer( 32L );
                    do {
                        sTmpBuffer.append( cNextCh );
                        cNextCh = GetNextChar();
                    } while( ('A' <= cNextCh && cNextCh <= 'Z') ||
                             ('a' <= cNextCh && cNextCh <= 'z') ||
                             ('0' <= cNextCh && cNextCh <= '9') ||
                             '-'==cNextCh && !IsEOF() );

                    aToken += String(sTmpBuffer.makeStringAndClear());

                    // und schauen, ob wir ihn kennen
                    switch( aToken.GetChar(0) )
                    {
                    case 'i':
                    case 'I':
                        if( aToken.EqualsIgnoreCaseAscii(sCSS1_import) )
                            nRet = CSS1_IMPORT_SYM;
                        break;
// /Feature: PrintExt
                    case 'p':
                    case 'P':
                        if( aToken.EqualsIgnoreCaseAscii(sCSS1_page) )
                            nRet = CSS1_PAGE_SYM;
                        break;
// /Feature: PrintExt
                    }

                    // Fehlerbehandlung: '@ident' und alles bis
                    // zu einem Semikolon der dem Ende des folgenden
                    // Blocks ignorieren
                    if( CSS1_NULL==nRet )
                    {
                        aToken.Erase();
                        USHORT nBlockLvl = 0;
                        sal_Unicode cQuoteCh = 0;
                        BOOL bDone = FALSE, bEscape = FALSE;
                        while( !bDone && !IsEOF() )
                        {
                            BOOL bOldEscape = bEscape;
                            bEscape = FALSE;
                            switch( cNextCh )
                            {
                            case '{':
                                if( !cQuoteCh && !bOldEscape )
                                    nBlockLvl++;;
                                break;
                            case ';':
                                if( !cQuoteCh && !bOldEscape )
                                    bDone = nBlockLvl==0;
                                break;
                            case '}':
                                if( !cQuoteCh && !bOldEscape )
                                    bDone = --nBlockLvl==0;
                                break;
                            case '\"':
                            case '\'':
                                if( !bOldEscape )
                                {
                                    if( cQuoteCh )
                                    {
                                        if( cQuoteCh == cNextCh )
                                            cQuoteCh = 0;
                                    }
                                    else
                                    {
                                        cQuoteCh = cNextCh;
                                    }
                                }
                                break;
                            case '\\':
                                if( !bOldEscape )
                                    bEscape = TRUE;
                                break;
                            }
                            cNextCh = GetNextChar();
                        }
                    }

                    bNextCh = FALSE;
                }
            }
            break;

        case '!': // '!' 'legal' | '!' 'important' | syntax error
            {
                // White Space ueberlesen
                cNextCh = GetNextChar();
                while( ( ' ' == cNextCh ||
                       (cNextCh >= 0x09 && cNextCh <= 0x0d) ) && !IsEOF() )
                {
                    bWhiteSpace = TRUE;
                    cNextCh = GetNextChar();
                }

                if( 'i'==cNextCh || 'I'==cNextCh)
                {
                    // den naechsten Identifer scannen
                    ::rtl::OUStringBuffer sTmpBuffer( 32L );
                    do {
                        sTmpBuffer.append( cNextCh );
                        cNextCh = GetNextChar();
                    } while( ('A' <= cNextCh && cNextCh <= 'Z') ||
                             ('a' <= cNextCh && cNextCh <= 'z') ||
                             ('0' <= cNextCh && cNextCh <= '9') ||
                             '-' == cNextCh && !IsEOF() );

                    aToken += String(sTmpBuffer.makeStringAndClear());

                    if( ('i'==aToken.GetChar(0) || 'I'==aToken.GetChar(0)) &&
                        aToken.EqualsIgnoreCaseAscii(sCSS1_important) )
                    {
                        // '!' 'important'
                        nRet = CSS1_IMPORTANT_SYM;
                    }
                    else
                    {
                        // Fehlerbehandlung: '!' ignorieren, IDENT nicht
                        nRet = CSS1_IDENT;
                    }

                    bWhiteSpace = FALSE;
                    bNextCh = FALSE;
                }
                else
                {
                    // Fehlerbehandlung: '!' ignorieren
                    bNextCh = FALSE;
                }
            }
            break;

        case '\"':
        case '\'': // STRING
            {
                // \... geht noch nicht!!!
                sal_Unicode cQuoteChar = cNextCh;
                cNextCh = GetNextChar();

                ::rtl::OUStringBuffer sTmpBuffer( MAX_LEN );
                do {
                    sTmpBuffer.append( cNextCh );
                    cNextCh = GetNextChar();
                } while( cQuoteChar != cNextCh && !IsEOF() );

                aToken += String(sTmpBuffer.makeStringAndClear());

                nRet = CSS1_STRING;
            }
            break;

        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9': // NUMBER | PERCENTAGE | LENGTH
            {
                // die aktuelle Position retten
                xub_StrLen nInPosSave = nInPos;
                sal_Unicode cNextChSave = cNextCh;
                sal_uInt32 nlLineNrSave = nlLineNr;
                sal_uInt32 nlLinePosSave = nlLinePos;
                BOOL bEOFSave = bEOF;

                // erstmal versuchen eine Hex-Zahl zu scannen
                ::rtl::OUStringBuffer sTmpBuffer( 16 );
                do {
                    sTmpBuffer.append( cNextCh );
                    cNextCh = GetNextChar();
                } while( sTmpBuffer.getLength() < 7 &&
                         ( ('0'<=cNextCh && '9'>=cNextCh) ||
                           ('A'<=cNextCh && 'F'>=cNextCh) ||
                           ('a'<=cNextCh && 'f'>=cNextCh) ) &&
                         !IsEOF() );

                if( sTmpBuffer.getLength()==6 )
                {
                    // wir haben eine hexadezimale Farbe gefunden
                    aToken += String(sTmpBuffer.makeStringAndClear());
                    nRet = CSS1_HEXCOLOR;
                    bNextCh = FALSE;

                    break;
                }

                // sonst versuchen wir es mit einer Zahl
                nInPos = nInPosSave;
                cNextCh = cNextChSave;
                nlLineNr = nlLineNrSave;
                nlLinePos = nlLinePosSave;
                bEOF = bEOFSave;

                // erstmal die Zahl scannen
                sTmpBuffer.setLength( 0L );
                do {
                    sTmpBuffer.append( cNextCh );
                    cNextCh = GetNextChar();
                } while( (('0'<=cNextCh && '9'>=cNextCh) || '.'==cNextCh) &&
                         !IsEOF() );

                aToken += String(sTmpBuffer.makeStringAndClear());
                nValue = aToken.ToDouble();

                // White Space ueberlesen
                while( ( ' ' == cNextCh ||
                       (cNextCh >= 0x09 && cNextCh <= 0x0d) ) && !IsEOF() )
                {
                    bWhiteSpace = TRUE;
                    cNextCh = GetNextChar();
                }

                // und nun Schauen, ob es eine Einheit gibt
                switch( cNextCh )
                {
                case '%': // PERCENTAGE
                    bWhiteSpace = FALSE;
                    nRet = CSS1_PERCENTAGE;
                    break;

                case 'c':
                case 'C': // LENGTH cm | LENGTH IDENT
                case 'e':
                case 'E': // LENGTH (em | ex) | LENGTH IDENT
                case 'i':
                case 'I': // LENGTH inch | LENGTH IDENT
                case 'p':
                case 'P': // LENGTH (pt | px | pc) | LENGTH IDENT
                case 'm':
                case 'M': // LENGTH mm | LENGTH IDENT
                    {
                        // die aktuelle Position retten
                        xub_StrLen nInPosSave = nInPos;
                        sal_Unicode cNextChSave = cNextCh;
                        ULONG nlLineNrSave = nlLineNr;
                        ULONG nlLinePosSave = nlLinePos;
                        BOOL bEOFSave = bEOF;

                        // den naechsten Identifer scannen
                        String aIdent;
                        ::rtl::OUStringBuffer sTmpBuffer( 64L );
                        do {
                            sTmpBuffer.append( cNextCh );
                            cNextCh = GetNextChar();
                        } while( ( ('A' <= cNextCh && cNextCh <= 'Z') ||
                                    ('a' <= cNextCh && cNextCh <= 'z') ||
                                    ('0' <= cNextCh && cNextCh <= '9') ||
                                 '-'==cNextCh) && !IsEOF() );

                        aIdent += String(sTmpBuffer.makeStringAndClear());

                        // Ist es eine Einheit?
                        const sal_Char *pCmp1 = 0, *pCmp2 = 0, *pCmp3 = 0;
                        double nScale1 = 1., nScale2 = 1., nScale3 = 1.;
                        CSS1Token nToken1 = CSS1_LENGTH,
                                  nToken2 = CSS1_LENGTH,
                                  nToken3 = CSS1_LENGTH;
                        switch( aIdent.GetChar(0) )
                        {
                        case 'c':
                        case 'C':
                            pCmp1 = sCSS1_UNIT_cm;
                            nScale1 = (72.*20.)/2.54; // twip
                            break;
                        case 'e':
                        case 'E':
                            pCmp1 = sCSS1_UNIT_em;
                            nToken1 = CSS1_EMS;

                            pCmp2 = sCSS1_UNIT_ex;
                            nToken2 = CSS1_EMX;
                            break;
                        case 'i':
                        case 'I':
                            pCmp1 = sCSS1_UNIT_inch;
                            nScale1 = 72.*20.; // twip
                            break;
                        case 'm':
                        case 'M':
                            pCmp1 = sCSS1_UNIT_mm;
                            nScale1 = (72.*20.)/25.4; // twip
                            break;
                        case 'p':
                        case 'P':
                            pCmp1 = sCSS1_UNIT_pt;
                            nScale1 = 20.; // twip

                            pCmp2 = sCSS1_UNIT_pc;
                            nScale2 = 12.*20.; // twip

                            pCmp3 = sCSS1_UNIT_px;
                            nToken3 = CSS1_PIXLENGTH;
                            break;
                        }

                        double nScale;
                        DBG_ASSERT( pCmp1, "Wo kommt das erste Zeichen her?" );
                        if( aIdent.EqualsIgnoreCaseAscii(pCmp1) )
                        {
                            nScale = nScale1;
                            nRet = nToken1;
                        }
                        else if( pCmp2 &&
                                 aIdent.EqualsIgnoreCaseAscii(pCmp2) )
                        {
                            nScale = nScale2;
                            nRet = nToken2;
                        }
                        else if( pCmp3 &&
                                 aIdent.EqualsIgnoreCaseAscii(pCmp3) )
                        {
                            nScale = nScale3;
                            nRet = nToken3;
                        }
                        else
                        {
                            nRet = CSS1_NUMBER;
                        }

                        if( CSS1_LENGTH==nRet && nScale!=1. )
                            nValue *= nScale;

                        if( nRet == CSS1_NUMBER )
                        {
                            nInPos = nInPosSave;
                            cNextCh = cNextChSave;
                            nlLineNr = nlLineNrSave;
                            nlLinePos = nlLinePosSave;
                            bEOF = bEOFSave;
                        }
                        else
                        {
                            bWhiteSpace = FALSE;
                        }
                        bNextCh = FALSE;
                    }
                    break;
                default: // NUMBER IDENT
                    bNextCh = FALSE;
                    nRet = CSS1_NUMBER;
                    break;
                }
            }
            break;

        case ':': // ':'
            // link/visited/active abfangen !!!
            nRet = CSS1_COLON;
            break;

        case '.': // DOT_W_WS | DOT_WO_WS
            nRet = bPrevWhiteSpace ? CSS1_DOT_W_WS : CSS1_DOT_WO_WS;
            break;

        // case '/': siehe oben

        case '+': // '+'
            nRet = CSS1_PLUS;
            break;

        case '-': // '-'
            nRet = CSS1_MINUS;
            break;

        case '{': // '{'
            nRet = CSS1_OBRACE;
            break;

        case '}': // '}'
            nRet = CSS1_CBRACE;
            break;

        case ';': // ';'
            nRet = CSS1_SEMICOLON;
            break;

        case ',': // ','
            nRet = CSS1_COMMA;
            break;

        case '#': // '#'
            cNextCh = GetNextChar();
            if( ('0'<=cNextCh && '9'>=cNextCh) ||
                ('a'<=cNextCh && 'f'>=cNextCh) ||
                ('A'<=cNextCh && 'F'>=cNextCh) )
            {
                // die aktuelle Position retten
                xub_StrLen nInPosSave = nInPos;
                sal_Unicode cNextChSave = cNextCh;
                ULONG nlLineNrSave = nlLineNr;
                ULONG nlLinePosSave = nlLinePos;
                BOOL bEOFSave = bEOF;

                // erstmal versuchen eine Hex-Zahl zu scannen
                ::rtl::OUStringBuffer sTmpBuffer( 6L );
                do {
                    sTmpBuffer.append( cNextCh );
                    cNextCh = GetNextChar();
                } while( sTmpBuffer.getLength() < 7 &&
                         ( ('0'<=cNextCh && '9'>=cNextCh) ||
                           ('A'<=cNextCh && 'F'>=cNextCh) ||
                           ('a'<=cNextCh && 'f'>=cNextCh) ) &&
                         !IsEOF() );

                if( sTmpBuffer.getLength()==6 || sTmpBuffer.getLength()==3 )
                {
                    // wir haben eine hexadezimale Farbe gefunden
                    aToken += String(sTmpBuffer.makeStringAndClear());
                    nRet = CSS1_HEXCOLOR;
                    bNextCh = FALSE;

                    break;
                }

                // sonst versuchen wir es mit einer Zahl
                nInPos = nInPosSave;
                cNextCh = cNextChSave;
                nlLineNr = nlLineNrSave;
                nlLinePos = nlLinePosSave;
                bEOF = bEOFSave;
            }

            nRet = CSS1_HASH;
            bNextCh = FALSE;
            break;

        case ' ':
        case '\t':
        case '\r':
        case '\n': // White-Space
            bWhiteSpace = TRUE;
            break;

        case (sal_Unicode)EOF:
            if( IsEOF() )
            {
                eState = CSS1_PAR_ACCEPTED;
                bNextCh = FALSE;
                break;
            }
            // kein break;

        default: // IDENT | syntax error
            // TODO IsAlpha
            if( ('A' <= cNextCh && cNextCh <= 'Z') ||
                ('a' <= cNextCh && cNextCh <= 'z') )
            {
                // IDENT

                BOOL bHexColor = TRUE;

                // den naechsten Identifer scannen
                ::rtl::OUStringBuffer sTmpBuffer( 64L );
                do {
                    sTmpBuffer.append( cNextCh );
                    if( bHexColor )
                    {
                        bHexColor =  sTmpBuffer.getLength()<7 &&
                                     ( ('0'<=cNextCh && '9'>=cNextCh) ||
                                       ('A'<=cNextCh && 'F'>=cNextCh) ||
                                       ('a'<=cNextCh && 'f'>=cNextCh) );
                    }
                    cNextCh = GetNextChar();
                    // TODO: AlphaNumeric
                } while( ( ('0'<=cNextCh && '9'>=cNextCh) ||
                           ('A'<=cNextCh && 'Z'>=cNextCh) ||
                           ('a'<=cNextCh && 'z'>=cNextCh) ||
                           '-'==cNextCh ) &&
                         !IsEOF() );

                aToken += String(sTmpBuffer.makeStringAndClear());

                if( bHexColor && sTmpBuffer.getLength()==6 )
                {
                    bNextCh = FALSE;
                    nRet = CSS1_HEXCOLOR;

                    break;
                }
                if( '('==cNextCh &&
                    ( (('u'==aToken.GetChar(0) || 'U'==aToken.GetChar(0)) &&
                       aToken.EqualsIgnoreCaseAscii(sCSS1_url)) ||
                      (('r'==aToken.GetChar(0) || 'R'==aToken.GetChar(0)) &&
                       aToken.EqualsIgnoreCaseAscii(sCSS1_rgb)) ) )
                {
                    USHORT nNestCnt = 0;
                    ::rtl::OUStringBuffer sTmpBuffer( 64L );
                    do {
                        sTmpBuffer.append( cNextCh );
                        switch( cNextCh )
                        {
                        case '(':   nNestCnt++; break;
                        case ')':   nNestCnt--; break;
                        }
                        cNextCh = GetNextChar();
                    } while( (nNestCnt>1 || ')'!=cNextCh) && !IsEOF() );
                    sTmpBuffer.append( cNextCh );
                    aToken += String(sTmpBuffer.makeStringAndClear());
                    bNextCh = TRUE;
                    nRet = 'u'==aToken.GetChar(0) || 'U'==aToken.GetChar(0)
                                ? CSS1_URL
                                : CSS1_RGB;
                }
                else
                {
                    bNextCh = FALSE;
                    nRet = CSS1_IDENT;
                }
            }
            // Fehlerbehandlung: Zeichen ignorieren
            break;
        }
        if( bNextCh )
            cNextCh = GetNextChar();

    } while( CSS1_NULL==nRet && IsParserWorking() );

    return nRet;
}


/*  */


// Dies folegenden Funktionen realisieren den in
//
//      http://www.w3.orh/pub/WWW/TR/WD-css1.html
// bzw. http://www.w3.orh/pub/WWW/TR/WD-css1-960220.html
//
// beschriebenen Parser fuer CSS1. Es handelt sich um eine direkte
// Umsetzung der dort beschriebenen Grammatik

// stylesheet
//  : import* rule*
//
// import
//  : IMPORT_SYM url
//
// url
//  : STRING
//
void CSS1Parser::ParseStyleSheet()
{
    LOOP_CHECK_DECL

    // import*
    BOOL bDone = FALSE;
    while( !bDone && IsParserWorking() )
    {
        LOOP_CHECK_CHECK( "Endlos-Schleife in ParseStyleSheet()/import *" )

        switch( nToken )
        {
        case CSS1_IMPORT_SYM:
            // IMPORT_SYM url
            // url ueberspringen wir ungeprueft
            nToken = GetNextToken();
            break;
        case CSS1_IDENT:            // Look-Aheads
        case CSS1_DOT_W_WS:
        case CSS1_HASH:
// /Feature: PrintExt
        case CSS1_PAGE_SYM:
// /Feature: PrintExt
            // rule
            bDone = TRUE;
            break;
        default:
            // Fehlerbehandlung: ueberlesen
            break;
        }

        if( !bDone )
            nToken = GetNextToken();
    }

    LOOP_CHECK_RESTART

    // rule *
    while( IsParserWorking() )
    {
        LOOP_CHECK_CHECK( "Endlos-Schleife in ParseStyleSheet()/rule *" )

        switch( nToken )
        {
        case CSS1_IDENT:        // Look-Aheads
        case CSS1_DOT_W_WS:
        case CSS1_HASH:
// /Feature: PrintExt
        case CSS1_PAGE_SYM:
// /Feature: PrintExt
            // rule
            ParseRule();
            break;
        default:
            // Fehlerbehandlung: ueberlesen
            nToken = GetNextToken();
            break;
        }
    }
}

// rule
//  : selector [ ',' selector ]*
//    '{' declaration [ ';' declaration ]* '}'
//
void CSS1Parser::ParseRule()
{
    // selector
    CSS1Selector *pSelector = ParseSelector();
    if( !pSelector )
        return;

    // Selektor verarbeiten
    if( SelectorParsed( pSelector, TRUE ) )
        delete pSelector;

    LOOP_CHECK_DECL

    // [ ',' selector ]*
    while( CSS1_COMMA==nToken && IsParserWorking() )
    {
        LOOP_CHECK_CHECK( "Endlos-Schleife in ParseRule()/selector *" )

        // ',' ueberelesen
        nToken = GetNextToken();

        // selector
        pSelector = ParseSelector();
        if( !pSelector )
            return;

        // Selektor verarbeiten
        if( SelectorParsed( pSelector, FALSE ) )
            delete pSelector;
    }

    // '{'
    if( CSS1_OBRACE != nToken )
        return;
    nToken = GetNextToken();

    // declaration
    String aProperty;
    CSS1Expression *pExpr = ParseDeclaration( aProperty );
    if( !pExpr )
        return;

    // expression verarbeiten
    if( DeclarationParsed( aProperty, pExpr ) )
        delete pExpr;

    LOOP_CHECK_RESTART

    // [ ';' declaration ]*
    while( CSS1_SEMICOLON==nToken && IsParserWorking() )
    {
        LOOP_CHECK_CHECK( "Endlos-Schleife in ParseRule()/declaration *" )

        // ';'
        nToken = GetNextToken();

        // declaration
        if( CSS1_IDENT == nToken )
        {
            CSS1Expression *pExpr = ParseDeclaration( aProperty );
            if( pExpr )
            {
                // expression verarbeiten
                if( DeclarationParsed( aProperty, pExpr ) )
                    delete pExpr;
            }
        }
    }

    // '}'
    if( CSS1_CBRACE == nToken )
        nToken = GetNextToken();
}

// selector
//  : simple_selector+ [ ':' pseudo_element ]?
//
// simple_selector
//  : element_name [ DOT_WO_WS class ]?
//  | DOT_W_WS class
//  | id_selector
//
// element_name
//  : IDENT
//
// class
//  : IDENT
//
// id_selector
//  : '#' IDENT
//
// pseude_element
//  : IDENT
//
CSS1Selector *CSS1Parser::ParseSelector()
{
    CSS1Selector *pRoot = 0, *pLast = 0;

    BOOL bDone = FALSE;
    CSS1Selector *pNew = 0;

    LOOP_CHECK_DECL

    // simple_selector+
    while( !bDone && IsParserWorking() )
    {
        LOOP_CHECK_CHECK( "Endlos-Schleife in ParseSelector()" )

        BOOL bNextToken = TRUE;

        switch( nToken )
        {
        case CSS1_IDENT:
            {
                // element_name [ DOT_WO_WS class ]?

                // element_name
                String aElement = aToken;
                CSS1SelectorType eType = CSS1_SELTYPE_ELEMENT;
                nToken = GetNextToken();

                if( CSS1_DOT_WO_WS == nToken )
                {
                    // DOT_WO_WS
                    nToken = GetNextToken();

                    // class
                    if( CSS1_IDENT == nToken )
                    {
                        (aElement += '.') += aToken;
                        eType = CSS1_SELTYPE_ELEM_CLASS;
                    }
                    else
                    {
                        // class fehlt
                        return pRoot;
                    }
                }
                else
                {
                    // das war jetzt ein Look-Ahead
                    bNextToken = FALSE;
                }
                pNew = new CSS1Selector( eType, aElement );
            }
            break;
        case CSS1_DOT_W_WS:
            // DOT_W_WS class

            // DOT_W_WS
            nToken = GetNextToken();

            if( CSS1_IDENT==nToken )
            {
                // class
                pNew = new CSS1Selector( CSS1_SELTYPE_CLASS, aToken );
            }
            else
            {
                // class fehlt
                return pRoot;
            }
            break;
        case CSS1_HASH:
            // '#' id_selector

            // '#'
            nToken = GetNextToken();

            if( CSS1_IDENT==nToken )
            {
                // id_selector
                pNew = new CSS1Selector( CSS1_SELTYPE_ID, aToken );
            }
            else
            {
                // id_selector fehlt
                return pRoot;
            }
            break;

// /Feature: PrintExt
        case CSS1_PAGE_SYM:
            {
                //  @page
                pNew = new CSS1Selector( CSS1_SELTYPE_PAGE, aToken );
            }
            break;
// /Feature: PrintExt

        default:
            // wir wissen nicht was kommt, also aufhoehren
            bDone = TRUE;
            break;
        }

        // falls ein Selektor angelegt wurd, ihn speichern
        if( pNew )
        {
            DBG_ASSERT( (pRoot!=0) == (pLast!=0),
                    "Root-Selektor, aber kein Last" );
            if( pLast )
                pLast->SetNext( pNew );
            else
                pRoot = pNew;

            pLast = pNew;
            pNew = 0;
        }

        if( bNextToken && !bDone )
            nToken = GetNextToken();
    }

    if( !pRoot )
    {
        // simple_selector fehlt
        return pRoot;
    }

    // [ ':' pseudo_element ]?
    if( CSS1_COLON==nToken && IsParserWorking() )
    {
        // ':' pseudo element
        nToken = GetNextToken();
        if( CSS1_IDENT==nToken )
        {
            pLast->SetNext( new CSS1Selector(CSS1_SELTYPE_PSEUDO,aToken) );
            nToken = GetNextToken();
        }
        else
        {
            // pseudo_element fehlt
            return pRoot;
        }
    }

    return pRoot;
}

// declaration
//  : property ':' expr prio?
//  | /* empty */
//
// expression
//  : term [ operator term ]*
//
// term
//  : unary_operator?
//     [ NUMBER | STRING | PERCENTAGE | LENGTH | EMS | EXS | IDENT |
//       HEXCOLOR | URL | RGB ]
//
// operator
//  : '/' | ',' | /* empty */
//
// unary_operator
//  : '-' | '+'
//
// property
//  : ident
//
// das Vorzeichen wird nur fuer numerische Werte (ausser PERCENTAGE)
// beruecksichtigt und wird auf nValue angewendet!
CSS1Expression *CSS1Parser::ParseDeclaration( String& rProperty )
{
    CSS1Expression *pRoot = 0, *pLast = 0;

    // property
    if( CSS1_IDENT != nToken )
    {
        // property fehlt
        return pRoot;
    }
    rProperty = aToken;

    nToken = GetNextToken();


    // ':'
    if( CSS1_COLON != nToken )
    {
        // ':' fehlt
        return pRoot;
    }
    nToken = GetNextToken();

    // term [operator term]*
    // hier sind wir sehr lax, was die Syntax angeht, sollte aber kein
    // Problem sein
    BOOL bDone = FALSE;
    sal_Unicode cSign = 0, cOp = 0;
    CSS1Expression *pNew = 0;

    LOOP_CHECK_DECL

    while( !bDone && IsParserWorking() )
    {
        LOOP_CHECK_CHECK( "Endlos-Schleife in ParseDeclaration()" )

        switch( nToken )
        {
        case CSS1_MINUS:
            cSign = '-';
            break;

        case CSS1_PLUS:
            cSign = '+';
            break;

        case CSS1_NUMBER:
        case CSS1_LENGTH:
        case CSS1_PIXLENGTH:
        case CSS1_EMS:
        case CSS1_EMX:
            if( '-'==cSign )
                nValue = -nValue;
        case CSS1_STRING:
        case CSS1_PERCENTAGE:
        case CSS1_IDENT:
        case CSS1_URL:
        case CSS1_RGB:
        case CSS1_HEXCOLOR:
            pNew = new CSS1Expression( nToken, aToken, nValue, cOp );
            nValue = 0; // sonst landet das auch im naechsten Ident
            cSign = 0;
            cOp = 0;
            break;

        case CSS1_SLASH:
            cOp = '/';
            cSign = 0;
            break;

        case CSS1_COMMA:
            cOp = ',';
            cSign = 0;
            break;

        default:
            bDone = TRUE;
            break;
        }

        // falls ein Expression angelegt wurde, diesen speichern
        if( pNew )
        {
            DBG_ASSERT( (pRoot!=0) == (pLast!=0),
                    "Root-Selektor, aber kein Last" );
            if( pLast )
                pLast->SetNext( pNew );
            else
                pRoot = pNew;

            pLast = pNew;
            pNew = 0;
        }

        if( !bDone )
            nToken = GetNextToken();
    }

    if( !pRoot )
    {
        // term fehlt
        return pRoot;
    }

    // prio?
    if( CSS1_IMPORTANT_SYM==nToken )
    {
        // IMPORTANT_SYM
        nToken = GetNextToken();
    }

    return pRoot;
}

/*  */

CSS1Parser::CSS1Parser()
{
}

CSS1Parser::~CSS1Parser()
{
}

/*  */

BOOL CSS1Parser::ParseStyleSheet( const String& rIn )
{
    String aTmp( rIn );

    sal_Unicode c;
    while( aTmp.Len() &&
           ( ' '==(c=aTmp.GetChar(0)) || '\t'==c || '\r'==c || '\n'==c ) )
        aTmp.Erase( 0, 1 );

    while( aTmp.Len() && ( ' '==(c=aTmp.GetChar( aTmp.Len()-1))
           || '\t'==c || '\r'==c || '\n'==c ) )
        aTmp.Erase( aTmp.Len()-1 );

    // SGML-Kommentare entfernen
    if( aTmp.Len() >= 4 &&
        aTmp.CompareToAscii("<!--",4) == COMPARE_EQUAL )
        aTmp.Erase( 0, 4 );

    if( aTmp.Len() >=3 &&
        aTmp.Copy(aTmp.Len()-3).CompareToAscii("-->") == COMPARE_EQUAL )
        aTmp.Erase( aTmp.Len()-3 );

    if( !aTmp.Len() )
        return TRUE;

    InitRead( aTmp );

    ParseStyleSheet();

    return TRUE;
}


BOOL CSS1Parser::ParseStyleOption( const String& rIn )
{
    if( !rIn.Len() )
        return TRUE;

    InitRead( rIn );

    String aProperty;
    CSS1Expression *pExpr = ParseDeclaration( aProperty );
    if( !pExpr )
    {
        return FALSE;
    }

    // expression verarbeiten
    if( DeclarationParsed( aProperty, pExpr ) )
        delete pExpr;

    LOOP_CHECK_DECL

    // [ ';' declaration ]*
    while( CSS1_SEMICOLON==nToken && IsParserWorking() )
    {
        LOOP_CHECK_CHECK( "Endlos-Schleife in ParseStyleOption()" )

        nToken = GetNextToken();
        if( CSS1_IDENT==nToken )
        {
            CSS1Expression *pExpr = ParseDeclaration( aProperty );
            if( pExpr )
            {
                // expression verarbeiten
                if( DeclarationParsed( aProperty, pExpr ) )
                    delete pExpr;
            }
        }
    }

    return TRUE;
}

BOOL CSS1Parser::SelectorParsed( const CSS1Selector *pSelector, BOOL bFirst )
{
    // Selektor loeschen
    return TRUE;
}

BOOL CSS1Parser::DeclarationParsed( const String& rProperty,
                                    const CSS1Expression *pExpr )
{
    // Deklaration loeschen
    return TRUE;
}


/*  */

CSS1Selector::~CSS1Selector()
{
    delete pNext;
}

/*  */

CSS1Expression::~CSS1Expression()
{
    delete pNext;
}

BOOL CSS1Expression::GetURL( String& rURL  ) const
{
    DBG_ASSERT( CSS1_URL==eType, "CSS1-Ausruck ist keine Farbe URL" );

    DBG_ASSERT( aValue.CompareIgnoreCaseToAscii( sCSS1_url, 3 ) ==
                                        COMPARE_EQUAL &&
                aValue.Len() > 5 &&
                '(' == aValue.GetChar(3) &&
                ')' == aValue.GetChar(aValue.Len()-1),
                "keine gueltiges URL(...)" );

    BOOL bRet = FALSE;

    if( aValue.Len() > 5 )
    {
        rURL = aValue.Copy( 4, aValue.Len()-5 );
        rURL.EraseTrailingChars();
        rURL.EraseLeadingChars();
        bRet = TRUE;
    }

    return bRet;
}

BOOL CSS1Expression::GetColor( Color &rColor ) const
{
    DBG_ASSERT( CSS1_IDENT==eType || CSS1_RGB==eType ||
                CSS1_HEXCOLOR==eType || CSS1_STRING==eType,
                "CSS1-Ausruck kann keine Farbe sein" );

    BOOL bRet = FALSE;
    ULONG nColor = ULONG_MAX;

    switch( eType )
    {
    case CSS1_RGB:
        {
            BYTE aColors[3] = { 0, 0, 0 };

            DBG_ASSERT( aValue.CompareIgnoreCaseToAscii( sCSS1_rgb, 3 )
                                            == COMPARE_EQUAL &&
                        aValue.Len() > 5 &&
                        '(' == aValue.GetChar( 3 ) &&
                        ')' == aValue.GetChar( aValue.Len()-1),
                        "keine gueltiges RGB(...)" );

            String aColorStr( aValue.Copy( 4, aValue.Len()-1 ) );

            xub_StrLen nPos = 0;
            USHORT nColor = 0;

            while( nColor<3 && nPos<aColorStr.Len() )
            {
                register sal_Unicode c;
                while( nPos < aColorStr.Len() &&
                        ((c=aColorStr.GetChar(nPos)) == ' ' || c == '\t' ||
                        c == '\n' || c== '\r' ) )
                    nPos++;

                xub_StrLen nEnd = aColorStr.Search( ',', nPos );
                String aNumber;
                if( STRING_NOTFOUND==nEnd )
                {
                    aNumber = aColorStr.Copy(nPos);
                    nPos = aColorStr.Len();
                }
                else
                {
                    aNumber = aColorStr.Copy( nPos, nEnd-nPos );
                    nPos = nEnd+1;
                }

                USHORT nNumber = (USHORT)aNumber.ToInt32();
                if( aNumber.Search('%') != STRING_NOTFOUND )
                {
                    if( nNumber > 100 )
                        nNumber = 100;
                    nNumber *= 255;
                    nNumber /= 100;
                }
                else if( nNumber > 255 )
                    nNumber = 255;

                aColors[nColor] = (BYTE)nNumber;
                nColor ++;
            }

            rColor.SetRed( aColors[0] );
            rColor.SetGreen( aColors[1] );
            rColor.SetBlue( aColors[2] );

            bRet = TRUE;    // etwas anderes als eine Farbe kann es nicht sein
        }
        break;

    case CSS1_IDENT:
    case CSS1_STRING:
        {
            String aTmp( aValue );
            aTmp.ToUpperAscii();
            nColor = GetHTMLColor( aTmp );
            bRet = nColor != ULONG_MAX;
        }
        if( bRet || CSS1_STRING != eType || !aValue.Len() ||
            aValue.GetChar( 0 )!='#' )
            break;

    case CSS1_HEXCOLOR:
        {
            // HACK fuer MS-IE: DIe Farbe kann auch in einem String stehen
            xub_StrLen nOffset = CSS1_STRING==eType ? 1 : 0;
            BOOL bDouble = aValue.Len()-nOffset == 3;
            xub_StrLen i = nOffset, nEnd = (bDouble ? 3 : 6) + nOffset;

            nColor = 0;
            for( ; i<nEnd; i++ )
            {
                register sal_Unicode c = (i<aValue.Len() ? aValue.GetChar(i)
                                                         : '0' );
                if( c >= '0' && c <= '9' )
                    c -= 48;
                else if( c >= 'A' && c <= 'F' )
                    c -= 55;
                else if( c >= 'a' && c <= 'f' )
                    c -= 87;
                else
                    c = 16;

                nColor *= 16;
                if( c<16 )
                    nColor += c;
                if( bDouble )
                {
                    nColor *= 16;
                    if( c<16 )
                        nColor += c;
                }
            }
            // bRet = i==6;
            bRet = TRUE;
        }
        break;
    }


    if( bRet && nColor!=ULONG_MAX )
    {
        rColor.SetRed( (BYTE)((nColor & 0x00ff0000UL) >> 16) );
        rColor.SetGreen( (BYTE)((nColor & 0x0000ff00UL) >> 8) );
        rColor.SetBlue( (BYTE)(nColor & 0x000000ffUL) );
    }

    return bRet;
}

/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/filter/html/parcss1.cxx,v 1.1.1.1 2000-09-18 17:14:56 hr Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.35  2000/09/18 16:04:47  willem.vandorp
      OpenOffice header added.

      Revision 1.34  2000/06/16 13:01:33  os
      Unicode error corrected

      Revision 1.33  2000/04/10 12:20:57  mib
      unicode

      Revision 1.32  2000/02/11 14:37:36  hr
      #70473# changes for unicode ( patched by automated patchtool )

      Revision 1.31  1999/04/19 10:46:32  MIB
      #64963#: EOF beim Zurueckseeken loeschen


      Rev 1.30   19 Apr 1999 12:46:32   MIB
   #64963#: EOF beim Zurueckseeken loeschen

      Rev 1.29   20 Feb 1998 19:02:20   MA
   header

      Rev 1.28   26 Nov 1997 19:09:48   MA
   includes

      Rev 1.27   03 Nov 1997 14:08:50   MA
   precomp entfernt

      Rev 1.26   10 Oct 1997 10:14:36   MIB
   CSS1-Printing-Ext: @page,page-break-before/after (noch kein left/right)

      Rev 1.25   19 Sep 1997 13:57:54   MIB
   unbekannte @-Bloecke werden jetzt ueberlesen

      Rev 1.24   29 Aug 1997 13:32:22   JP
   VCL Color Anpassung

      Rev 1.23   15 Jul 1997 16:52:24   MIB
   fix: Auch 3-stellige Farben erlauben (fuer www.w3.org/Style

      Rev 1.22   02 Jul 1997 17:13:14   MIB
   fix: r??() nicht wie rgb() behandeln

      Rev 1.21   13 Jun 1997 17:08:12   MIB
   Ein par MS-IE Syntax-Goodies nachgebaut

      Rev 1.20   13 Jun 1997 11:58:32   MIB
   fix: .foo auch am Anfang erkennen

      Rev 1.19   07 May 1997 15:46:38   MIB
   Destruktoren nicht inline, sonst GPF unter Win16 (Warum auch immer?)

      Rev 1.18   20 Dec 1996 14:00:46   MIB
   Warnings

      Rev 1.17   28 Oct 1996 19:12:14   MIB
   Umaute in Identifiern-Erlauben, EOF-Behandlung

      Rev 1.16   23 Oct 1996 19:49:06   JP
   String Umstellung: [] -> GetChar()

      Rev 1.15   21 Oct 1996 08:57:42   MIB
   bug fix: H1.foo wurde falsch geparsed

      Rev 1.14   13 Oct 1996 12:31:26   MIB
   Argument von unbekannten @ident ueberlesen

      Rev 1.13   20 Aug 1996 16:55:40   MIB
   bug-fix: Hexzahlen muessen 6 Zeichen lang sein

      Rev 1.12   19 Aug 1996 18:11:22   MIB
   bugfix: PIXLENGTH durchlassen

      Rev 1.11   14 Aug 1996 16:22:24   MIB
   gcc und msvc: statt *aStr nicht aStr[0] sondern aStr[(USHORT)0]

      Rev 1.10   13 Aug 1996 21:08:32   sdo
   GCC

      Rev 1.9   12 Aug 1996 10:32:16   MIB
   Anpassungen an draft vom 26.7.96, Background-Property, Scannen von Farben verb.

      Rev 1.8   07 Aug 1996 13:21:46   MIB
   Import von color und text-decoration

      Rev 1.7   17 Jul 1996 10:22:50   MIB
   Loop-Check verbessert

      Rev 1.6   27 Jun 1996 16:44:16   MIB
   fix #29184#: Endlosschleife bei SGML-Kommentaren und andere Verbesserungen

      Rev 1.5   26 Jun 1996 15:24:24   MIB
   Anpassung an 324, Includes aufgeraeumt

      Rev 1.4   20 Jun 1996 12:33:26   MIB
   bug fix: Einheiten sind jetzt case-insensitiv

      Rev 1.3   17 Jun 1996 15:59:50   MIB
   Length nun in Twips, bug fixes und Erweiterungen

      Rev 1.2   16 Jun 1996 14:32:58   MIB
   Loops sicherer gemacht, Strings ausgelagert

      Rev 1.1   14 Jun 1996 20:12:56   MIB
   die erste Endlos-Schleife gefixed

      Rev 1.0   14 Jun 1996 17:42:34   MIB
   Initial revision.

*************************************************************************/

