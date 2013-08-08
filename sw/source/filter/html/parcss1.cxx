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


#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <rtl/ustrbuf.hxx>
#include <vcl/svapp.hxx>
#include <svtools/htmltokn.h>
#include <comphelper/string.hxx>
#include "css1kywd.hxx"
#include "parcss1.hxx"


// Loop-Check: Um Endlos-Schleifen zu vermeiden, wird in jeder
// Schalife geprueft, ob ein Fortschritt in der Eingabe-Position
// stattgefunden hat
#define LOOP_CHECK

#ifdef LOOP_CHECK

#define LOOP_CHECK_DECL \
    sal_Int32 nOldInPos = SAL_MAX_INT32;
#define LOOP_CHECK_RESTART \
    nOldInPos = SAL_MAX_INT32;
#define LOOP_CHECK_CHECK( where ) \
    OSL_ENSURE( nOldInPos!=nInPos || cNextCh==(sal_Unicode)EOF, where );    \
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


void CSS1Parser::InitRead( const OUString& rIn )
{
    nlLineNr = 0;
    nlLinePos = 0;

    bWhiteSpace = sal_True; // Wenn noch nichts gelesen wurde ist das wie WS
    bEOF = sal_False;
    eState = CSS1_PAR_WORKING;
    nValue = 0.;

    aIn = rIn;
    nInPos = 0;
    cNextCh = GetNextChar();
    nToken = GetNextToken();
}

sal_Unicode CSS1Parser::GetNextChar()
{
    if( nInPos >= aIn.getLength() )
    {
        bEOF = sal_True;
        return (sal_Unicode)EOF;
    }

    sal_Unicode c = aIn[nInPos];
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
    aToken = "";

    do {
        // Merken, ob davor White-Space gelesen wurde
        sal_Bool bPrevWhiteSpace = bWhiteSpace;
        bWhiteSpace = sal_False;

        sal_Bool bNextCh = sal_True;
        switch( cNextCh )
        {
        case '/': // COMMENT | '/'
            {
                cNextCh = GetNextChar();
                if( '*' == cNextCh )
                {
                    // COMMENT
                    cNextCh = GetNextChar();

                    sal_Bool bAsterix = sal_False;
                    while( !(bAsterix && '/'==cNextCh) && !IsEOF() )
                    {
                        bAsterix = ('*'==cNextCh);
                        cNextCh = GetNextChar();
                    }
                }
                else
                {
                    // '/'
                    bNextCh = sal_False;
                    nRet = CSS1_SLASH;
                }
            }
            break;

        case '@': // '@import' | '@XXX'
            {
                cNextCh = GetNextChar();
                if (comphelper::string::isalphaAscii(cNextCh))
                {
                    // den naechsten Identifer scannen
                    OUStringBuffer sTmpBuffer( 32L );
                    do {
                        sTmpBuffer.append( cNextCh );
                        cNextCh = GetNextChar();
                    } while( (comphelper::string::isalnumAscii(cNextCh) ||
                             '-' == cNextCh) && !IsEOF() );

                    aToken += sTmpBuffer.makeStringAndClear();

                    // und schauen, ob wir ihn kennen
                    switch( aToken[0] )
                    {
                    case 'i':
                    case 'I':
                        if( aToken.equalsIgnoreAsciiCase( "import" ) )
                            nRet = CSS1_IMPORT_SYM;
                        break;
// /Feature: PrintExt
                    case 'p':
                    case 'P':
                        if( aToken.equalsIgnoreAsciiCase( "page" ) )
                            nRet = CSS1_PAGE_SYM;
                        break;
// /Feature: PrintExt
                    }

                    // Fehlerbehandlung: '@ident' und alles bis
                    // zu einem Semikolon der dem Ende des folgenden
                    // Blocks ignorieren
                    if( CSS1_NULL==nRet )
                    {
                        aToken = "";
                        sal_uInt16 nBlockLvl = 0;
                        sal_Unicode cQuoteCh = 0;
                        sal_Bool bDone = sal_False, bEscape = sal_False;
                        while( !bDone && !IsEOF() )
                        {
                            sal_Bool bOldEscape = bEscape;
                            bEscape = sal_False;
                            switch( cNextCh )
                            {
                            case '{':
                                if( !cQuoteCh && !bOldEscape )
                                    nBlockLvl++;
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
                                    bEscape = sal_True;
                                break;
                            }
                            cNextCh = GetNextChar();
                        }
                    }

                    bNextCh = sal_False;
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
                    bWhiteSpace = sal_True;
                    cNextCh = GetNextChar();
                }

                if( 'i'==cNextCh || 'I'==cNextCh)
                {
                    // den naechsten Identifer scannen
                    OUStringBuffer sTmpBuffer( 32L );
                    do {
                        sTmpBuffer.append( cNextCh );
                        cNextCh = GetNextChar();
                    } while( (comphelper::string::isalnumAscii(cNextCh) ||
                             '-' == cNextCh) && !IsEOF() );

                    aToken += sTmpBuffer.makeStringAndClear();

                    if( ( 'i'==aToken[0] || 'I'==aToken[0] ) &&
                        aToken.equalsIgnoreAsciiCaseAscii( "important" ) )
                    {
                        // '!' 'important'
                        nRet = CSS1_IMPORTANT_SYM;
                    }
                    else
                    {
                        // Fehlerbehandlung: '!' ignorieren, IDENT nicht
                        nRet = CSS1_IDENT;
                    }

                    bWhiteSpace = sal_False;
                    bNextCh = sal_False;
                }
                else
                {
                    // Fehlerbehandlung: '!' ignorieren
                    bNextCh = sal_False;
                }
            }
            break;

        case '\"':
        case '\'': // STRING
            {
                // \... geht noch nicht!!!
                sal_Unicode cQuoteChar = cNextCh;
                cNextCh = GetNextChar();

                OUStringBuffer sTmpBuffer( MAX_LEN );
                do {
                    sTmpBuffer.append( cNextCh );
                    cNextCh = GetNextChar();
                } while( cQuoteChar != cNextCh && !IsEOF() );

                aToken += sTmpBuffer.toString();

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
                sal_Size nInPosSave = nInPos;
                sal_Unicode cNextChSave = cNextCh;
                sal_uInt32 nlLineNrSave = nlLineNr;
                sal_uInt32 nlLinePosSave = nlLinePos;
                sal_Bool bEOFSave = bEOF;

                // erstmal versuchen eine Hex-Zahl zu scannen
                OUStringBuffer sTmpBuffer( 16 );
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
                    aToken += sTmpBuffer.makeStringAndClear();
                    nRet = CSS1_HEXCOLOR;
                    bNextCh = sal_False;

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

                aToken += sTmpBuffer.makeStringAndClear();
                nValue = aToken.toDouble();

                // White Space ueberlesen
                while( ( ' ' == cNextCh ||
                       (cNextCh >= 0x09 && cNextCh <= 0x0d) ) && !IsEOF() )
                {
                    bWhiteSpace = sal_True;
                    cNextCh = GetNextChar();
                }

                // und nun Schauen, ob es eine Einheit gibt
                switch( cNextCh )
                {
                case '%': // PERCENTAGE
                    bWhiteSpace = sal_False;
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
                        sal_Int32 nInPosOld = nInPos;
                        sal_Unicode cNextChOld = cNextCh;
                        sal_uLong nlLineNrOld  = nlLineNr;
                        sal_uLong nlLinePosOld = nlLinePos;
                        sal_Bool bEOFOld = bEOF;

                        // den naechsten Identifer scannen
                        OUString aIdent;
                        OUStringBuffer sTmpBuffer2( 64L );
                        do {
                            sTmpBuffer2.append( cNextCh );
                            cNextCh = GetNextChar();
                        } while( (comphelper::string::isalnumAscii(cNextCh) ||
                                 '-' == cNextCh) && !IsEOF() );

                        aIdent += sTmpBuffer2.makeStringAndClear();

                        // Ist es eine Einheit?
                        const sal_Char *pCmp1 = 0, *pCmp2 = 0, *pCmp3 = 0;
                        double nScale1 = 1., nScale2 = 1.;
                        CSS1Token nToken1 = CSS1_LENGTH,
                                  nToken2 = CSS1_LENGTH,
                                  nToken3 = CSS1_LENGTH;
                        switch( aIdent[0] )
                        {
                        case 'c':
                        case 'C':
                            pCmp1 = "cm";
                            nScale1 = (72.*20.)/2.54; // twip
                            break;
                        case 'e':
                        case 'E':
                            pCmp1 = "em";
                            nToken1 = CSS1_EMS;

                            pCmp2 = "ex";
                            nToken2 = CSS1_EMX;
                            break;
                        case 'i':
                        case 'I':
                            pCmp1 = "in";
                            nScale1 = 72.*20.; // twip
                            break;
                        case 'm':
                        case 'M':
                            pCmp1 = "mm";
                            nScale1 = (72.*20.)/25.4; // twip
                            break;
                        case 'p':
                        case 'P':
                            pCmp1 = "pt";
                            nScale1 = 20.; // twip

                            pCmp2 = "pc";
                            nScale2 = 12.*20.; // twip

                            pCmp3 = "px";
                            nToken3 = CSS1_PIXLENGTH;
                            break;
                        }

                        double nScale = 0.0;
                        OSL_ENSURE( pCmp1, "Wo kommt das erste Zeichen her?" );
                        if( aIdent.equalsIgnoreAsciiCaseAscii( pCmp1 ) )
                        {
                            nScale = nScale1;
                            nRet = nToken1;
                        }
                        else if( pCmp2 &&
                                 aIdent.equalsIgnoreAsciiCaseAscii( pCmp2 ) )
                        {
                            nScale = nScale2;
                            nRet = nToken2;
                        }
                        else if( pCmp3 &&
                                 aIdent.equalsIgnoreAsciiCaseAscii( pCmp3 ) )
                        {
                            nScale =  1.; // nScale3
                            nRet = nToken3;
                        }
                        else
                        {
                            nRet = CSS1_NUMBER;
                        }

                        if( CSS1_LENGTH==nRet && nScale!=1.0 )
                            nValue *= nScale;

                        if( nRet == CSS1_NUMBER )
                        {
                            nInPos = nInPosOld;
                            cNextCh = cNextChOld;
                            nlLineNr = nlLineNrOld;
                            nlLinePos = nlLinePosOld;
                            bEOF = bEOFOld;
                        }
                        else
                        {
                            bWhiteSpace = sal_False;
                        }
                        bNextCh = sal_False;
                    }
                    break;
                default: // NUMBER IDENT
                    bNextCh = sal_False;
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
                sal_Int32 nInPosSave = nInPos;
                sal_Unicode cNextChSave = cNextCh;
                sal_uLong nlLineNrSave = nlLineNr;
                sal_uLong nlLinePosSave = nlLinePos;
                sal_Bool bEOFSave = bEOF;

                // erstmal versuchen eine Hex-Zahl zu scannen
                OUStringBuffer sTmpBuffer( 6L );
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
                    aToken += sTmpBuffer.makeStringAndClear();
                    nRet = CSS1_HEXCOLOR;
                    bNextCh = sal_False;

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
            bNextCh = sal_False;
            break;

        case ' ':
        case '\t':
        case '\r':
        case '\n': // White-Space
            bWhiteSpace = sal_True;
            break;

        case (sal_Unicode)EOF:
            if( IsEOF() )
            {
                eState = CSS1_PAR_ACCEPTED;
                bNextCh = sal_False;
                break;
            }
            // kein break;

        default: // IDENT | syntax error
            if (comphelper::string::isalphaAscii(cNextCh))
            {
                // IDENT

                sal_Bool bHexColor = sal_True;

                // den naechsten Identifer scannen
                OUStringBuffer sTmpBuffer( 64L );
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
                } while( (comphelper::string::isalnumAscii(cNextCh) ||
                           '-' == cNextCh) && !IsEOF() );

                aToken += sTmpBuffer.makeStringAndClear();

                if( bHexColor && sTmpBuffer.getLength()==6 )
                {
                    bNextCh = sal_False;
                    nRet = CSS1_HEXCOLOR;

                    break;
                }
                if( '('==cNextCh &&
                    ( (('u'==aToken[0] || 'U'==aToken[0]) &&
                       aToken.equalsIgnoreAsciiCase( "url" )) ||
                      (('r'==aToken[0] || 'R'==aToken[0]) &&
                       aToken.equalsIgnoreAsciiCase( "rgb" )) ) )
                {
                    sal_uInt16 nNestCnt = 0;
                    OUStringBuffer sTmpBuffer2( 64L );
                    do {
                        sTmpBuffer2.append( cNextCh );
                        switch( cNextCh )
                        {
                        case '(':   nNestCnt++; break;
                        case ')':   nNestCnt--; break;
                        }
                        cNextCh = GetNextChar();
                    } while( (nNestCnt>1 || ')'!=cNextCh) && !IsEOF() );
                    sTmpBuffer2.append( cNextCh );
                    aToken += sTmpBuffer2.makeStringAndClear();
                    bNextCh = sal_True;
                    nRet = 'u'==aToken[0] || 'U'==aToken[0]
                                ? CSS1_URL
                                : CSS1_RGB;
                }
                else
                {
                    bNextCh = sal_False;
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
    sal_Bool bDone = sal_False;
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
            bDone = sal_True;
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
    if( SelectorParsed( pSelector, sal_True ) )
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
        if( SelectorParsed( pSelector, sal_False ) )
            delete pSelector;
    }

    // '{'
    if( CSS1_OBRACE != nToken )
        return;
    nToken = GetNextToken();

    // declaration
    OUString aProperty;
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
            CSS1Expression *pExp = ParseDeclaration( aProperty );
            if( pExp )
            {
                // expression verarbeiten
                if( DeclarationParsed( aProperty, pExp ) )
                    delete pExp;
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

    sal_Bool bDone = sal_False;
    CSS1Selector *pNew = 0;

    LOOP_CHECK_DECL

    // simple_selector+
    while( !bDone && IsParserWorking() )
    {
        LOOP_CHECK_CHECK( "Endlos-Schleife in ParseSelector()" )

        sal_Bool bNextToken = sal_True;

        switch( nToken )
        {
        case CSS1_IDENT:
            {
                // element_name [ DOT_WO_WS class ]?

                // element_name
                OUString aElement = aToken;
                CSS1SelectorType eType = CSS1_SELTYPE_ELEMENT;
                nToken = GetNextToken();

                if( CSS1_DOT_WO_WS == nToken )
                {
                    // DOT_WO_WS
                    nToken = GetNextToken();

                    // class
                    if( CSS1_IDENT == nToken )
                    {
                        aElement += "." + aToken;
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
                    bNextToken = sal_False;
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
            bDone = sal_True;
            break;
        }

        // falls ein Selektor angelegt wurd, ihn speichern
        if( pNew )
        {
            OSL_ENSURE( (pRoot!=0) == (pLast!=0),
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
CSS1Expression *CSS1Parser::ParseDeclaration( OUString& rProperty )
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
    sal_Bool bDone = sal_False;
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
            bDone = sal_True;
            break;
        }

        // falls ein Expression angelegt wurde, diesen speichern
        if( pNew )
        {
            OSL_ENSURE( (pRoot!=0) == (pLast!=0),
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


CSS1Parser::CSS1Parser()
    : nValue(0)
    , eState(CSS1_PAR_ACCEPTED)
    , nToken(CSS1_NULL)
{
}

CSS1Parser::~CSS1Parser()
{
}


sal_Bool CSS1Parser::ParseStyleSheet( const OUString& rIn )
{
    OUString aTmp( rIn );

    sal_Unicode c;
    while( !aTmp.isEmpty() &&
           ( ' '==(c=aTmp[0]) || '\t'==c || '\r'==c || '\n'==c ) )
        aTmp = aTmp.copy( 1, aTmp.getLength() - 1 );

    while( !aTmp.isEmpty() && ( ' '==(c=aTmp[aTmp.getLength()-1])
           || '\t'==c || '\r'==c || '\n'==c ) )
        aTmp = aTmp.copy( 0, aTmp.getLength()-1 );

    // SGML-Kommentare entfernen
    if( aTmp.getLength() >= 4 &&
        aTmp.startsWith( "<!--" ) )
        aTmp = aTmp.copy( 4, aTmp.getLength() - 4 );

    if( aTmp.getLength() >=3 &&
        aTmp.endsWith("-->") )
        aTmp = aTmp.copy( 0, aTmp.getLength() - 3 );

    if( aTmp.isEmpty() )
        return sal_True;

    InitRead( aTmp );

    ParseStyleSheet();

    return sal_True;
}


sal_Bool CSS1Parser::ParseStyleOption( const OUString& rIn )
{
    if( rIn.isEmpty() )
        return sal_True;

    InitRead( rIn );

    // fdo#41796: skip over spurious semicolons
    while (CSS1_SEMICOLON == nToken)
    {
        nToken = GetNextToken();
    }

    OUString aProperty;
    CSS1Expression *pExpr = ParseDeclaration( aProperty );
    if( !pExpr )
    {
        return sal_False;
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
            CSS1Expression *pExp = ParseDeclaration( aProperty );
            if( pExp )
            {
                // expression verarbeiten
                if( DeclarationParsed( aProperty, pExp ) )
                    delete pExp;
            }
        }
    }

    return sal_True;
}

bool CSS1Parser::SelectorParsed( CSS1Selector* /* pSelector */, bool /*bFirst*/ )
{
    // Selektor loeschen
    return true;
}

sal_Bool CSS1Parser::DeclarationParsed( const OUString& /*rProperty*/,
                                    const CSS1Expression * /* pExpr */ )
{
    // Deklaration loeschen
    return sal_True;
}


CSS1Selector::~CSS1Selector()
{
    delete pNext;
}

CSS1Expression::~CSS1Expression()
{
    delete pNext;
}

sal_Bool CSS1Expression::GetURL( OUString& rURL  ) const
{
    OSL_ENSURE( CSS1_URL==eType, "CSS1-Ausruck ist keine Farbe URL" );

    OSL_ENSURE( aValue.startsWithIgnoreAsciiCase( "url" ) &&
                aValue.getLength() > 5 &&
                '(' == aValue[3] &&
                ')' == aValue[aValue.getLength()-1],
                "keine gueltiges URL(...)" );

    sal_Bool bRet = sal_False;

    if( aValue.getLength() > 5 )
    {
        rURL = aValue.copy( 4, aValue.getLength() - 5 );
        rURL = comphelper::string::strip(rURL, ' ');
        bRet = sal_True;
    }

    return bRet;
}

sal_Bool CSS1Expression::GetColor( Color &rColor ) const
{
    OSL_ENSURE( CSS1_IDENT==eType || CSS1_RGB==eType ||
                CSS1_HEXCOLOR==eType || CSS1_STRING==eType,
                "CSS1-Ausruck kann keine Farbe sein" );

    sal_Bool bRet = sal_False;
    sal_uInt32 nColor = SAL_MAX_UINT32;

    switch( eType )
    {
    case CSS1_RGB:
        {
            sal_uInt8 aColors[3] = { 0, 0, 0 };

            OSL_ENSURE( aValue.startsWithIgnoreAsciiCase( "rgb" ) &&
                        aValue.getLength() > 5 &&
                        '(' == aValue[3] &&
                        ')' == aValue[aValue.getLength()-1],
                        "keine gueltiges RGB(...)" );

            OUString aColorStr( aValue.copy( 4, aValue.getLength()-1 ) );

            sal_Int32 nPos = 0;
            sal_uInt16 nCol = 0;

            while( nCol < 3 && nPos < aColorStr.getLength() )
            {
                sal_Unicode c;
                while( nPos < aColorStr.getLength() &&
                        ((c=aColorStr[nPos]) == ' ' || c == '\t' ||
                        c == '\n' || c== '\r' ) )
                    nPos++;

                sal_Int32 nEnd = aColorStr.indexOf( ',', nPos );
                OUString aNumber;
                if( nEnd == -1 )
                {
                    aNumber = aColorStr.copy(nPos);
                    nPos = aColorStr.getLength();
                }
                else
                {
                    aNumber = aColorStr.copy( nPos, nEnd-nPos );
                    nPos = nEnd+1;
                }

                sal_uInt16 nNumber = (sal_uInt16)aNumber.toInt32();
                if( aNumber.indexOf('%') >= 0 )
                {
                    if( nNumber > 100 )
                        nNumber = 100;
                    nNumber *= 255;
                    nNumber /= 100;
                }
                else if( nNumber > 255 )
                    nNumber = 255;

                aColors[nCol] = (sal_uInt8)nNumber;
                nCol ++;
            }

            rColor.SetRed( aColors[0] );
            rColor.SetGreen( aColors[1] );
            rColor.SetBlue( aColors[2] );

            bRet = sal_True;    // etwas anderes als eine Farbe kann es nicht sein
        }
        break;

    case CSS1_IDENT:
    case CSS1_STRING:
        {
            OUString aTmp( aValue.toAsciiUpperCase() );
            nColor = GetHTMLColor( aTmp );
            bRet = nColor != SAL_MAX_UINT32;
        }
        if( bRet || CSS1_STRING != eType || aValue.isEmpty() ||
            aValue[0] != '#' )
            break;

    case CSS1_HEXCOLOR:
        {
            // HACK fuer MS-IE: DIe Farbe kann auch in einem String stehen
            sal_Int32 nOffset = CSS1_STRING==eType ? 1 : 0;
            sal_Bool bDouble = aValue.getLength()-nOffset == 3;
            sal_Int32 i = nOffset, nEnd = (bDouble ? 3 : 6) + nOffset;

            nColor = 0;
            for( ; i<nEnd; i++ )
            {
                sal_Unicode c = (i<aValue.getLength() ? aValue[i]
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
            bRet = sal_True;
        }
        break;
    default:
        ;
    }


    if( bRet && nColor!=SAL_MAX_UINT32 )
    {
        rColor.SetRed( (sal_uInt8)((nColor & 0x00ff0000UL) >> 16) );
        rColor.SetGreen( (sal_uInt8)((nColor & 0x0000ff00UL) >> 8) );
        rColor.SetBlue( (sal_uInt8)(nColor & 0x000000ffUL) );
    }

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
