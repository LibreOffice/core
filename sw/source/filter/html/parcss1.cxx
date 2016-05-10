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

    bWhiteSpace = true; // Wenn noch nichts gelesen wurde ist das wie WS
    bEOF = false;
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
        bEOF = true;
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

//      http://www.w3.orh/pub/WWW/TR/WD-css1.html
// bzw. http://www.w3.orh/pub/WWW/TR/WD-css1-960220.html

// beschriebenen Scanner fuer CSS1. Es handelt sich um eine direkte
// Umsetzung der dort beschriebenen Lex-Grammatik

CSS1Token CSS1Parser::GetNextToken()
{
    CSS1Token nRet = CSS1_NULL;
    aToken.clear();

    do {
        // Merken, ob davor White-Space gelesen wurde
        bool bPrevWhiteSpace = bWhiteSpace;
        bWhiteSpace = false;

        bool bNextCh = true;
        switch( cNextCh )
        {
        case '/': // COMMENT | '/'
            {
                cNextCh = GetNextChar();
                if( '*' == cNextCh )
                {
                    // COMMENT
                    cNextCh = GetNextChar();

                    bool bAsterisk = false;
                    while( !(bAsterisk && '/'==cNextCh) && !IsEOF() )
                    {
                        bAsterisk = ('*'==cNextCh);
                        cNextCh = GetNextChar();
                    }
                }
                else
                {
                    // '/'
                    bNextCh = false;
                    nRet = CSS1_SLASH;
                }
            }
            break;

        case '@': // '@import' | '@XXX'
            {
                cNextCh = GetNextChar();
                if (rtl::isAsciiAlpha(cNextCh))
                {
                    // den naechsten Identifer scannen
                    OUStringBuffer sTmpBuffer(32);
                    do {
                        sTmpBuffer.append( cNextCh );
                        cNextCh = GetNextChar();
                    } while( (rtl::isAsciiAlphanumeric(cNextCh) ||
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
                    case 'p':
                    case 'P':
                        if( aToken.equalsIgnoreAsciiCase( "page" ) )
                            nRet = CSS1_PAGE_SYM;
                        break;
                    }

                    // Fehlerbehandlung: '@ident' und alles bis
                    // zu einem Semikolon der dem Ende des folgenden
                    // Blocks ignorieren
                    if( CSS1_NULL==nRet )
                    {
                        aToken.clear();
                        int nBlockLvl = 0;
                        sal_Unicode cQuoteCh = 0;
                        bool bDone = false, bEscape = false;
                        while( !bDone && !IsEOF() )
                        {
                            bool bOldEscape = bEscape;
                            bEscape = false;
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
                                    bEscape = true;
                                break;
                            }
                            cNextCh = GetNextChar();
                        }
                    }

                    bNextCh = false;
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
                    bWhiteSpace = true;
                    cNextCh = GetNextChar();
                }

                if( 'i'==cNextCh || 'I'==cNextCh)
                {
                    // den naechsten Identifer scannen
                    OUStringBuffer sTmpBuffer(32);
                    do {
                        sTmpBuffer.append( cNextCh );
                        cNextCh = GetNextChar();
                    } while( (rtl::isAsciiAlphanumeric(cNextCh) ||
                             '-' == cNextCh) && !IsEOF() );

                    aToken += sTmpBuffer.makeStringAndClear();

                    if( ( 'i'==aToken[0] || 'I'==aToken[0] ) &&
                        aToken.equalsIgnoreAsciiCase( "important" ) )
                    {
                        // '!' 'important'
                        nRet = CSS1_IMPORTANT_SYM;
                    }
                    else
                    {
                        // Fehlerbehandlung: '!' ignorieren, IDENT nicht
                        nRet = CSS1_IDENT;
                    }

                    bWhiteSpace = false;
                    bNextCh = false;
                }
                else
                {
                    // Fehlerbehandlung: '!' ignorieren
                    bNextCh = false;
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
                bool bEOFSave = bEOF;

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
                    bNextCh = false;

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
                    bWhiteSpace = true;
                    cNextCh = GetNextChar();
                }

                // und nun Schauen, ob es eine Einheit gibt
                switch( cNextCh )
                {
                case '%': // PERCENTAGE
                    bWhiteSpace = false;
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
                        bool bEOFOld = bEOF;

                        // den naechsten Identifer scannen
                        OUString aIdent;
                        OUStringBuffer sTmpBuffer2(64);
                        do {
                            sTmpBuffer2.append( cNextCh );
                            cNextCh = GetNextChar();
                        } while( (rtl::isAsciiAlphanumeric(cNextCh) ||
                                 '-' == cNextCh) && !IsEOF() );

                        aIdent += sTmpBuffer2.makeStringAndClear();

                        // Ist es eine Einheit?
                        const sal_Char *pCmp1 = nullptr, *pCmp2 = nullptr, *pCmp3 = nullptr;
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
                            bWhiteSpace = false;
                        }
                        bNextCh = false;
                    }
                    break;
                default: // NUMBER IDENT
                    bNextCh = false;
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
                bool bEOFSave = bEOF;

                // erstmal versuchen eine Hex-Zahl zu scannen
                OUStringBuffer sTmpBuffer(6);
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
                    bNextCh = false;

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
            bNextCh = false;
            break;

        case ' ':
        case '\t':
        case '\r':
        case '\n': // White-Space
            bWhiteSpace = true;
            break;

        case (sal_Unicode)EOF:
            if( IsEOF() )
            {
                eState = CSS1_PAR_ACCEPTED;
                bNextCh = false;
                break;
            }
            SAL_FALLTHROUGH;

        default: // IDENT | syntax error
            if (rtl::isAsciiAlpha(cNextCh))
            {
                // IDENT

                bool bHexColor = true;

                // den naechsten Identifer scannen
                OUStringBuffer sTmpBuffer(64);
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
                } while( (rtl::isAsciiAlphanumeric(cNextCh) ||
                           '-' == cNextCh) && !IsEOF() );

                aToken += sTmpBuffer.makeStringAndClear();

                if( bHexColor && sTmpBuffer.getLength()==6 )
                {
                    bNextCh = false;
                    nRet = CSS1_HEXCOLOR;

                    break;
                }
                if( '('==cNextCh &&
                    ( (('u'==aToken[0] || 'U'==aToken[0]) &&
                       aToken.equalsIgnoreAsciiCase( "url" )) ||
                      (('r'==aToken[0] || 'R'==aToken[0]) &&
                       aToken.equalsIgnoreAsciiCase( "rgb" )) ) )
                {
                    int nNestCnt = 0;
                    OUStringBuffer sTmpBuffer2(64);
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
                    bNextCh = true;
                    nRet = 'u'==aToken[0] || 'U'==aToken[0]
                                ? CSS1_URL
                                : CSS1_RGB;
                }
                else
                {
                    bNextCh = false;
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

//      http://www.w3.orh/pub/WWW/TR/WD-css1.html
// bzw. http://www.w3.orh/pub/WWW/TR/WD-css1-960220.html

// beschriebenen Parser fuer CSS1. Es handelt sich um eine direkte
// Umsetzung der dort beschriebenen Grammatik

// stylesheet
//  : import* rule*

// import
//  : IMPORT_SYM url

// url
//  : STRING

void CSS1Parser::ParseStyleSheet()
{
    LOOP_CHECK_DECL

    // import*
    bool bDone = false;
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
        case CSS1_PAGE_SYM:
            // rule
            bDone = true;
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
        case CSS1_PAGE_SYM:
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

void CSS1Parser::ParseRule()
{
    // selector
    CSS1Selector *pSelector = ParseSelector();
    if( !pSelector )
        return;

    // Selektor verarbeiten
    if( SelectorParsed( pSelector, true ) )
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
        if( SelectorParsed( pSelector, false ) )
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

// simple_selector
//  : element_name [ DOT_WO_WS class ]?
//  | DOT_W_WS class
//  | id_selector

// element_name
//  : IDENT

// class
//  : IDENT

// id_selector
//  : '#' IDENT

// pseude_element
//  : IDENT

CSS1Selector *CSS1Parser::ParseSelector()
{
    CSS1Selector *pRoot = nullptr, *pLast = nullptr;

    bool bDone = false;
    CSS1Selector *pNew = nullptr;

    LOOP_CHECK_DECL

    // simple_selector+
    while( !bDone && IsParserWorking() )
    {
        LOOP_CHECK_CHECK( "Endlos-Schleife in ParseSelector()" )

        bool bNextToken = true;

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
                    bNextToken = false;
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

        case CSS1_PAGE_SYM:
            {
                //  @page
                pNew = new CSS1Selector( CSS1_SELTYPE_PAGE, aToken );
            }
            break;

        default:
            // wir wissen nicht was kommt, also aufhoehren
            bDone = true;
            break;
        }

        // falls ein Selektor angelegt wurd, ihn speichern
        if( pNew )
        {
            OSL_ENSURE( (pRoot!=nullptr) == (pLast!=nullptr),
                    "Root-Selektor, aber kein Last" );
            if( pLast )
                pLast->SetNext( pNew );
            else
                pRoot = pNew;

            pLast = pNew;
            pNew = nullptr;
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

// expression
//  : term [ operator term ]*

// term
//  : unary_operator?
//     [ NUMBER | STRING | PERCENTAGE | LENGTH | EMS | EXS | IDENT |
//       HEXCOLOR | URL | RGB ]

// operator
//  : '/' | ',' | /* empty */

// unary_operator
//  : '-' | '+'

// property
//  : ident

// das Vorzeichen wird nur fuer numerische Werte (ausser PERCENTAGE)
// beruecksichtigt und wird auf nValue angewendet!
CSS1Expression *CSS1Parser::ParseDeclaration( OUString& rProperty )
{
    CSS1Expression *pRoot = nullptr, *pLast = nullptr;

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
    bool bDone = false;
    sal_Unicode cSign = 0, cOp = 0;
    CSS1Expression *pNew = nullptr;

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
            SAL_FALLTHROUGH;
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
            bDone = true;
            break;
        }

        // falls ein Expression angelegt wurde, diesen speichern
        if( pNew )
        {
            OSL_ENSURE( (pRoot!=nullptr) == (pLast!=nullptr),
                    "Root-Selektor, aber kein Last" );
            if( pLast )
                pLast->SetNext( pNew );
            else
                pRoot = pNew;

            pLast = pNew;
            pNew = nullptr;
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
    : bWhiteSpace(false)
    , bEOF(false)
    , cNextCh(0)
    , nInPos(0)
    , nlLineNr(0)
    , nlLinePos(0)
    , nValue(0)
    , eState(CSS1_PAR_ACCEPTED)
    , nToken(CSS1_NULL)
{
}

CSS1Parser::~CSS1Parser()
{
}

bool CSS1Parser::ParseStyleSheet( const OUString& rIn )
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
        return true;

    InitRead( aTmp );

    ParseStyleSheet();

    return true;
}

bool CSS1Parser::ParseStyleOption( const OUString& rIn )
{
    if( rIn.isEmpty() )
        return true;

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
        return false;
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

    return true;
}

bool CSS1Parser::SelectorParsed( CSS1Selector* /* pSelector */, bool /*bFirst*/ )
{
    // Selektor loeschen
    return true;
}

bool CSS1Parser::DeclarationParsed( const OUString& /*rProperty*/,
                                    const CSS1Expression * /* pExpr */ )
{
    // Deklaration loeschen
    return true;
}

CSS1Selector::~CSS1Selector()
{
    delete pNext;
}

CSS1Expression::~CSS1Expression()
{
    delete pNext;
}

bool CSS1Expression::GetURL( OUString& rURL  ) const
{
    OSL_ENSURE( CSS1_URL==eType, "CSS1-Ausruck ist keine Farbe URL" );

    OSL_ENSURE( aValue.startsWithIgnoreAsciiCase( "url" ) &&
                aValue.getLength() > 5 &&
                '(' == aValue[3] &&
                ')' == aValue[aValue.getLength()-1],
                "keine gueltiges URL(...)" );

    bool bRet = false;

    if( aValue.getLength() > 5 )
    {
        rURL = aValue.copy( 4, aValue.getLength() - 5 );
        rURL = comphelper::string::strip(rURL, ' ');
        bRet = true;
    }

    return bRet;
}

bool CSS1Expression::GetColor( Color &rColor ) const
{
    OSL_ENSURE( CSS1_IDENT==eType || CSS1_RGB==eType ||
                CSS1_HEXCOLOR==eType || CSS1_STRING==eType,
                "CSS1-Ausruck kann keine Farbe sein" );

    bool bRet = false;
    sal_uInt32 nColor = SAL_MAX_UINT32;

    switch( eType )
    {
    case CSS1_RGB:
        {
            sal_uInt8 aColors[3] = { 0, 0, 0 };

            if (!aValue.startsWithIgnoreAsciiCase( "rgb" ) || aValue.getLength() < 6 ||
                    aValue[3] != '(' || aValue[aValue.getLength()-1] != ')')
            {
                break;
            }

            sal_Int32 nPos = 4; // start after "rgb("
            for ( int nCol = 0; nCol < 3 && nPos > 0; ++nCol )
            {
                const OUString aNumber = aValue.getToken(0, ',', nPos);

                sal_Int32 nNumber = aNumber.toInt32();
                if( nNumber<0 )
                {
                    nNumber = 0;
                }
                else if( aNumber.indexOf('%') >= 0 )
                {
                    if( nNumber > 100 )
                        nNumber = 100;
                    nNumber *= 255;
                    nNumber /= 100;
                }
                else if( nNumber > 255 )
                    nNumber = 255;

                aColors[nCol] = static_cast<sal_uInt8>(nNumber);
            }

            rColor.SetRed( aColors[0] );
            rColor.SetGreen( aColors[1] );
            rColor.SetBlue( aColors[2] );

            bRet = true;    // etwas anderes als eine Farbe kann es nicht sein
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
        SAL_FALLTHROUGH;
    case CSS1_HEXCOLOR:
        {
            // HACK fuer MS-IE: DIe Farbe kann auch in einem String stehen
            sal_Int32 nOffset = CSS1_STRING==eType ? 1 : 0;
            bool bDouble = aValue.getLength()-nOffset == 3;
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
            bRet = true;
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
