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

#include <stdlib.h>
#include <limits.h>
#include <osl/diagnose.h>
#include <rtl/character.hxx>
#include <rtl/ustrbuf.hxx>
#include <tools/color.hxx>
#include <vcl/svapp.hxx>
#include <svtools/htmltokn.h>
#include <comphelper/string.hxx>
#include "css1kywd.hxx"
#include "parcss1.hxx"

// Loop-Check: Used to avoid infinite loops, is checked after every
// loop, if there is progress of the input position
#define LOOP_CHECK

#ifdef LOOP_CHECK

#define LOOP_CHECK_DECL \
    sal_Int32 nOldInPos = SAL_MAX_INT32;
#define LOOP_CHECK_RESTART \
    nOldInPos = SAL_MAX_INT32;
#define LOOP_CHECK_CHECK( where ) \
    OSL_ENSURE( nOldInPos!=m_nInPos || m_cNextCh==sal_Unicode(EOF), where );    \
    if( nOldInPos==m_nInPos && m_cNextCh!=sal_Unicode(EOF) )                    \
        break;                                                              \
    else                                                                    \
        nOldInPos = m_nInPos;

#else

#define LOOP_CHECK_DECL
#define LOOP_CHECK_RESTART
#define LOOP_CHECK_CHECK( where )

#endif

const sal_Int32 MAX_LEN = 1024;

void CSS1Parser::InitRead( const OUString& rIn )
{
    m_nlLineNr = 0;
    m_nlLinePos = 0;

    m_bWhiteSpace = true; // if nothing was read it's like there was WS
    m_bEOF = false;
    m_eState = CSS1_PAR_WORKING;
    m_nValue = 0.;

    m_aIn = rIn;
    m_nInPos = 0;
    m_cNextCh = GetNextChar();
    m_nToken = GetNextToken();
}

sal_Unicode CSS1Parser::GetNextChar()
{
    if( m_nInPos >= m_aIn.getLength() )
    {
        m_bEOF = true;
        return sal_Unicode(EOF);
    }

    sal_Unicode c = m_aIn[m_nInPos];
    m_nInPos++;

    if( c == '\n' )
    {
        ++m_nlLineNr;
        m_nlLinePos = 1;
    }
    else
        ++m_nlLinePos;

    return c;
}

// This function implements the scanner described in

//       http://www.w3.org/pub/WWW/TR/WD-css1.html
// resp. http://www.w3.org/pub/WWW/TR/WD-css1-960220.html

// for CSS1. It's a direct implementation of the
// described Lex grammar.

CSS1Token CSS1Parser::GetNextToken()
{
    CSS1Token nRet = CSS1_NULL;
    m_aToken.clear();

    do {
        // remember if white space was read
        bool bPrevWhiteSpace = m_bWhiteSpace;
        m_bWhiteSpace = false;

        bool bNextCh = true;
        switch( m_cNextCh )
        {
        case '/': // COMMENT | '/'
            {
                m_cNextCh = GetNextChar();
                if( '*' == m_cNextCh )
                {
                    // COMMENT
                    m_cNextCh = GetNextChar();

                    bool bAsterisk = false;
                    while( !(bAsterisk && '/'==m_cNextCh) && !IsEOF() )
                    {
                        bAsterisk = ('*'==m_cNextCh);
                        m_cNextCh = GetNextChar();
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
                m_cNextCh = GetNextChar();
                if (rtl::isAsciiAlpha(m_cNextCh))
                {
                    // scan the next identifier
                    OUStringBuffer sTmpBuffer(32);
                    do {
                        sTmpBuffer.append( m_cNextCh );
                        m_cNextCh = GetNextChar();
                    } while( (rtl::isAsciiAlphanumeric(m_cNextCh) ||
                             '-' == m_cNextCh) && !IsEOF() );

                    m_aToken += sTmpBuffer;

                    // check if we know it
                    switch( m_aToken[0] )
                    {
                    case 'i':
                    case 'I':
                        if( m_aToken.equalsIgnoreAsciiCase( "import" ) )
                            nRet = CSS1_IMPORT_SYM;
                        break;
                    case 'p':
                    case 'P':
                        if( m_aToken.equalsIgnoreAsciiCase( "page" ) )
                            nRet = CSS1_PAGE_SYM;
                        break;
                    }

                    // error handling: ignore '@indent' and the rest until
                    // semicolon at end of the next block
                    if( CSS1_NULL==nRet )
                    {
                        m_aToken.clear();
                        int nBlockLvl = 0;
                        sal_Unicode cQuoteCh = 0;
                        bool bDone = false, bEscape = false;
                        while( !bDone && !IsEOF() )
                        {
                            bool bOldEscape = bEscape;
                            bEscape = false;
                            switch( m_cNextCh )
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
                                        if( cQuoteCh == m_cNextCh )
                                            cQuoteCh = 0;
                                    }
                                    else
                                    {
                                        cQuoteCh = m_cNextCh;
                                    }
                                }
                                break;
                            case '\\':
                                if( !bOldEscape )
                                    bEscape = true;
                                break;
                            }
                            m_cNextCh = GetNextChar();
                        }
                    }

                    bNextCh = false;
                }
            }
            break;

        case '!': // '!' 'legal' | '!' 'important' | syntax error
            {
                // ignore white space
                m_cNextCh = GetNextChar();
                while( ( ' ' == m_cNextCh ||
                       (m_cNextCh >= 0x09 && m_cNextCh <= 0x0d) ) && !IsEOF() )
                {
                    m_bWhiteSpace = true;
                    m_cNextCh = GetNextChar();
                }

                if( 'i'==m_cNextCh || 'I'==m_cNextCh)
                {
                    // scan next identifier
                    OUStringBuffer sTmpBuffer(32);
                    do {
                        sTmpBuffer.append( m_cNextCh );
                        m_cNextCh = GetNextChar();
                    } while( (rtl::isAsciiAlphanumeric(m_cNextCh) ||
                             '-' == m_cNextCh) && !IsEOF() );

                    m_aToken += sTmpBuffer;

                    if( ( 'i'==m_aToken[0] || 'I'==m_aToken[0] ) &&
                        m_aToken.equalsIgnoreAsciiCase( "important" ) )
                    {
                        // '!' 'important'
                        nRet = CSS1_IMPORTANT_SYM;
                    }
                    else
                    {
                        // error handling: ignore '!', not IDENT
                        nRet = CSS1_IDENT;
                    }

                    m_bWhiteSpace = false;
                    bNextCh = false;
                }
                else
                {
                    // error handling: ignore '!'
                    bNextCh = false;
                }
            }
            break;

        case '\"':
        case '\'': // STRING
            {
                // \... isn't possible yet!!!
                sal_Unicode cQuoteChar = m_cNextCh;
                m_cNextCh = GetNextChar();

                OUStringBuffer sTmpBuffer( MAX_LEN );
                do {
                    sTmpBuffer.append( m_cNextCh );
                    m_cNextCh = GetNextChar();
                } while( cQuoteChar != m_cNextCh && !IsEOF() );

                m_aToken += sTmpBuffer;

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
                // save current position
                std::size_t nInPosSave = m_nInPos;
                sal_Unicode cNextChSave = m_cNextCh;
                sal_uInt32 nlLineNrSave = m_nlLineNr;
                sal_uInt32 nlLinePosSave = m_nlLinePos;
                bool bEOFSave = m_bEOF;

                // first try to parse a hex digit
                OUStringBuffer sTmpBuffer( 16 );
                do {
                    sTmpBuffer.append( m_cNextCh );
                    m_cNextCh = GetNextChar();
                } while( sTmpBuffer.getLength() < 7 &&
                         ( ('0'<=m_cNextCh && '9'>=m_cNextCh) ||
                           ('A'<=m_cNextCh && 'F'>=m_cNextCh) ||
                           ('a'<=m_cNextCh && 'f'>=m_cNextCh) ) &&
                         !IsEOF() );

                if( sTmpBuffer.getLength()==6 )
                {
                    // we found a color in hex
                    m_aToken += sTmpBuffer;
                    nRet = CSS1_HEXCOLOR;
                    bNextCh = false;

                    break;
                }

                // otherwise we try a number
                m_nInPos = nInPosSave;
                m_cNextCh = cNextChSave;
                m_nlLineNr = nlLineNrSave;
                m_nlLinePos = nlLinePosSave;
                m_bEOF = bEOFSave;

                // first parse the number
                sTmpBuffer.setLength( 0 );
                do {
                    sTmpBuffer.append( m_cNextCh );
                    m_cNextCh = GetNextChar();
                } while( (('0'<=m_cNextCh && '9'>=m_cNextCh) || '.'==m_cNextCh) &&
                         !IsEOF() );

                m_aToken += sTmpBuffer;
                m_nValue = m_aToken.toDouble();

                // ignore white space
                while( ( ' ' == m_cNextCh ||
                       (m_cNextCh >= 0x09 && m_cNextCh <= 0x0d) ) && !IsEOF() )
                {
                    m_bWhiteSpace = true;
                    m_cNextCh = GetNextChar();
                }

                // check now, of there is an unit
                switch( m_cNextCh )
                {
                case '%': // PERCENTAGE
                    m_bWhiteSpace = false;
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
                        // save current position
                        sal_Int32 nInPosOld = m_nInPos;
                        sal_Unicode cNextChOld = m_cNextCh;
                        sal_uLong nlLineNrOld  = m_nlLineNr;
                        sal_uLong nlLinePosOld = m_nlLinePos;
                        bool bEOFOld = m_bEOF;

                        // parse the next identifier
                        OUString aIdent;
                        OUStringBuffer sTmpBuffer2(64);
                        do {
                            sTmpBuffer2.append( m_cNextCh );
                            m_cNextCh = GetNextChar();
                        } while( (rtl::isAsciiAlphanumeric(m_cNextCh) ||
                                 '-' == m_cNextCh) && !IsEOF() );

                        aIdent += sTmpBuffer2;

                        // Is it an unit?
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
                        OSL_ENSURE( pCmp1, "Where does the first digit come from?" );
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
                            m_nValue *= nScale;

                        if( nRet == CSS1_NUMBER )
                        {
                            m_nInPos = nInPosOld;
                            m_cNextCh = cNextChOld;
                            m_nlLineNr = nlLineNrOld;
                            m_nlLinePos = nlLinePosOld;
                            m_bEOF = bEOFOld;
                        }
                        else
                        {
                            m_bWhiteSpace = false;
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
            // catch link/visited/active !!!
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
            m_cNextCh = GetNextChar();
            if( ('0'<=m_cNextCh && '9'>=m_cNextCh) ||
                ('a'<=m_cNextCh && 'f'>=m_cNextCh) ||
                ('A'<=m_cNextCh && 'F'>=m_cNextCh) )
            {
                // save current position
                sal_Int32 nInPosSave = m_nInPos;
                sal_Unicode cNextChSave = m_cNextCh;
                sal_uLong nlLineNrSave = m_nlLineNr;
                sal_uLong nlLinePosSave = m_nlLinePos;
                bool bEOFSave = m_bEOF;

                // first try to parse a hex digit
                OUStringBuffer sTmpBuffer(6);
                do {
                    sTmpBuffer.append( m_cNextCh );
                    m_cNextCh = GetNextChar();
                } while( sTmpBuffer.getLength() < 7 &&
                         ( ('0'<=m_cNextCh && '9'>=m_cNextCh) ||
                           ('A'<=m_cNextCh && 'F'>=m_cNextCh) ||
                           ('a'<=m_cNextCh && 'f'>=m_cNextCh) ) &&
                         !IsEOF() );

                if( sTmpBuffer.getLength()==6 || sTmpBuffer.getLength()==3 )
                {
                    // we found a color in hex
                    m_aToken += sTmpBuffer;
                    nRet = CSS1_HEXCOLOR;
                    bNextCh = false;

                    break;
                }

                // otherwise we try a number
                m_nInPos = nInPosSave;
                m_cNextCh = cNextChSave;
                m_nlLineNr = nlLineNrSave;
                m_nlLinePos = nlLinePosSave;
                m_bEOF = bEOFSave;
            }

            nRet = CSS1_HASH;
            bNextCh = false;
            break;

        case ' ':
        case '\t':
        case '\r':
        case '\n': // White-Space
            m_bWhiteSpace = true;
            break;

        case sal_Unicode(EOF):
            if( IsEOF() )
            {
                m_eState = CSS1_PAR_ACCEPTED;
                bNextCh = false;
                break;
            }
            [[fallthrough]];

        default: // IDENT | syntax error
            if (rtl::isAsciiAlpha(m_cNextCh))
            {
                // IDENT

                bool bHexColor = true;

                // parse the next identifier
                OUStringBuffer sTmpBuffer(64);
                do {
                    sTmpBuffer.append( m_cNextCh );
                    if( bHexColor )
                    {
                        bHexColor =  sTmpBuffer.getLength()<7 &&
                                     ( ('0'<=m_cNextCh && '9'>=m_cNextCh) ||
                                       ('A'<=m_cNextCh && 'F'>=m_cNextCh) ||
                                       ('a'<=m_cNextCh && 'f'>=m_cNextCh) );
                    }
                    m_cNextCh = GetNextChar();
                } while( (rtl::isAsciiAlphanumeric(m_cNextCh) ||
                           '-' == m_cNextCh) && !IsEOF() );

                m_aToken += sTmpBuffer;

                if( bHexColor && sTmpBuffer.getLength()==6 )
                {
                    bNextCh = false;
                    nRet = CSS1_HEXCOLOR;

                    break;
                }
                if( '('==m_cNextCh &&
                    ( (('u'==m_aToken[0] || 'U'==m_aToken[0]) &&
                       m_aToken.equalsIgnoreAsciiCase( "url" )) ||
                      (('r'==m_aToken[0] || 'R'==m_aToken[0]) &&
                       m_aToken.equalsIgnoreAsciiCase( "rgb" )) ) )
                {
                    int nNestCnt = 0;
                    OUStringBuffer sTmpBuffer2(64);
                    do {
                        sTmpBuffer2.append( m_cNextCh );
                        switch( m_cNextCh )
                        {
                        case '(':   nNestCnt++; break;
                        case ')':   nNestCnt--; break;
                        }
                        m_cNextCh = GetNextChar();
                    } while( (nNestCnt>1 || ')'!=m_cNextCh) && !IsEOF() );
                    sTmpBuffer2.append( m_cNextCh );
                    m_aToken += sTmpBuffer2;
                    bNextCh = true;
                    nRet = 'u'==m_aToken[0] || 'U'==m_aToken[0]
                                ? CSS1_URL
                                : CSS1_RGB;
                }
                else
                {
                    bNextCh = false;
                    nRet = CSS1_IDENT;
                }
            }
            // error handling: ignore digit
            break;
        }
        if( bNextCh )
            m_cNextCh = GetNextChar();

    } while( CSS1_NULL==nRet && IsParserWorking() );

    return nRet;
}

// These functions implement the parser described in

//       http://www.w3.org/pub/WWW/TR/WD-css1.html
// resp. http://www.w3.org/pub/WWW/TR/WD-css1-960220.html

// for CSS1. It's a direct implementation of the
// described Lex grammar.

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
        LOOP_CHECK_CHECK( "Infinite loop in ParseStyleSheet()/import *" )

        switch( m_nToken )
        {
        case CSS1_IMPORT_SYM:
            // IMPORT_SYM url
            // URL are skipped without checks
            m_nToken = GetNextToken();
            break;
        case CSS1_IDENT:            // Look-Aheads
        case CSS1_DOT_W_WS:
        case CSS1_HASH:
        case CSS1_PAGE_SYM:
            // rule
            bDone = true;
            break;
        default:
            // error handling: ignore
            break;
        }

        if( !bDone )
            m_nToken = GetNextToken();
    }

    LOOP_CHECK_RESTART

    // rule *
    while( IsParserWorking() )
    {
        LOOP_CHECK_CHECK( "Infinite loop in ParseStyleSheet()/rule *" )

        switch( m_nToken )
        {
        case CSS1_IDENT:        // Look-Aheads
        case CSS1_DOT_W_WS:
        case CSS1_HASH:
        case CSS1_PAGE_SYM:
            // rule
            ParseRule();
            break;
        default:
            // error handling: ignore
            m_nToken = GetNextToken();
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
    std::unique_ptr<CSS1Selector> pSelector = ParseSelector();
    if( !pSelector )
        return;

    // process selector
    SelectorParsed( std::move(pSelector), true );

    LOOP_CHECK_DECL

    // [ ',' selector ]*
    while( CSS1_COMMA==m_nToken && IsParserWorking() )
    {
        LOOP_CHECK_CHECK( "Infinite loop in ParseRule()/selector *" )

        // ignore ','
        m_nToken = GetNextToken();

        // selector
        pSelector = ParseSelector();
        if( !pSelector )
            return;

        // process selector
        SelectorParsed( std::move(pSelector), false );
    }

    // '{'
    if( CSS1_OBRACE != m_nToken )
        return;
    m_nToken = GetNextToken();

    // declaration
    OUString aProperty;
    std::unique_ptr<CSS1Expression> pExpr = ParseDeclaration( aProperty );
    if( !pExpr )
        return;

    // process expression
    DeclarationParsed( aProperty, std::move(pExpr) );

    LOOP_CHECK_RESTART

    // [ ';' declaration ]*
    while( CSS1_SEMICOLON==m_nToken && IsParserWorking() )
    {
        LOOP_CHECK_CHECK( "Infinite loop in ParseRule()/declaration *" )

        // ';'
        m_nToken = GetNextToken();

        // declaration
        if( CSS1_IDENT == m_nToken )
        {
            std::unique_ptr<CSS1Expression> pExp = ParseDeclaration( aProperty );
            if( pExp )
            {
                // process expression
                DeclarationParsed( aProperty, std::move(pExp));
            }
        }
    }

    // '}'
    if( CSS1_CBRACE == m_nToken )
        m_nToken = GetNextToken();
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

std::unique_ptr<CSS1Selector> CSS1Parser::ParseSelector()
{
    std::unique_ptr<CSS1Selector> pRoot;
    CSS1Selector *pLast = nullptr;

    bool bDone = false;
    CSS1Selector *pNew = nullptr;

    LOOP_CHECK_DECL

    // simple_selector+
    while( !bDone && IsParserWorking() )
    {
        LOOP_CHECK_CHECK( "Infinite loop in ParseSelector()" )

        bool bNextToken = true;

        switch( m_nToken )
        {
        case CSS1_IDENT:
            {
                // element_name [ DOT_WO_WS class ]?

                // element_name
                OUString aElement = m_aToken;
                CSS1SelectorType eType = CSS1_SELTYPE_ELEMENT;
                m_nToken = GetNextToken();

                if( CSS1_DOT_WO_WS == m_nToken )
                {
                    // DOT_WO_WS
                    m_nToken = GetNextToken();

                    // class
                    if( CSS1_IDENT == m_nToken )
                    {
                        aElement += "." + m_aToken;
                        eType = CSS1_SELTYPE_ELEM_CLASS;
                    }
                    else
                    {
                        // missing class
                        return pRoot;
                    }
                }
                else
                {
                    // that was a look-ahead
                    bNextToken = false;
                }
                pNew = new CSS1Selector( eType, aElement );
            }
            break;
        case CSS1_DOT_W_WS:
            // DOT_W_WS class

            // DOT_W_WS
            m_nToken = GetNextToken();

            if( CSS1_IDENT==m_nToken )
            {
                // class
                pNew = new CSS1Selector( CSS1_SELTYPE_CLASS, m_aToken );
            }
            else
            {
                // missing class
                return pRoot;
            }
            break;
        case CSS1_HASH:
            // '#' id_selector

            // '#'
            m_nToken = GetNextToken();

            if( CSS1_IDENT==m_nToken )
            {
                // id_selector
                pNew = new CSS1Selector( CSS1_SELTYPE_ID, m_aToken );
            }
            else
            {
                // missing id_selector
                return pRoot;
            }
            break;

        case CSS1_PAGE_SYM:
            {
                //  @page
                pNew = new CSS1Selector( CSS1_SELTYPE_PAGE, m_aToken );
            }
            break;

        default:
            // stop because we don't know what's next
            bDone = true;
            break;
        }

        // if created a new selector then save it
        if( pNew )
        {
            OSL_ENSURE( (pRoot!=nullptr) == (pLast!=nullptr),
                    "Root-Selector, but no Last" );
            if( pLast )
                pLast->SetNext( pNew );
            else
                pRoot.reset(pNew);

            pLast = pNew;
            pNew = nullptr;
        }

        if( bNextToken && !bDone )
            m_nToken = GetNextToken();
    }

    if( !pRoot )
    {
        // missing simple_selector
        return pRoot;
    }

    // [ ':' pseudo_element ]?
    if( CSS1_COLON==m_nToken && IsParserWorking() )
    {
        // ':' pseudo element
        m_nToken = GetNextToken();
        if( CSS1_IDENT==m_nToken )
        {
            if (pLast)
                pLast->SetNext( new CSS1Selector(CSS1_SELTYPE_PSEUDO,m_aToken) );
            m_nToken = GetNextToken();
        }
        else
        {
            // missing pseudo_element
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

// the sign is only used for numeric values (except PERCENTAGE)
// and it's applied on nValue!
std::unique_ptr<CSS1Expression> CSS1Parser::ParseDeclaration( OUString& rProperty )
{
    std::unique_ptr<CSS1Expression> pRoot;
    CSS1Expression *pLast = nullptr;

    // property
    if( CSS1_IDENT != m_nToken )
    {
        // missing property
        return pRoot;
    }
    rProperty = m_aToken;

    m_nToken = GetNextToken();

    // ':'
    if( CSS1_COLON != m_nToken )
    {
        // missing ':'
        return pRoot;
    }
    m_nToken = GetNextToken();

    // term [operator term]*
    // here we're pretty lax regarding the syntax, but this shouldn't
    // be a problem
    bool bDone = false;
    sal_Unicode cSign = 0, cOp = 0;
    CSS1Expression *pNew = nullptr;

    LOOP_CHECK_DECL

    while( !bDone && IsParserWorking() )
    {
        LOOP_CHECK_CHECK( "Infinite loop in ParseDeclaration()" )

        switch( m_nToken )
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
                m_nValue = -m_nValue;
            [[fallthrough]];
        case CSS1_STRING:
        case CSS1_PERCENTAGE:
        case CSS1_IDENT:
        case CSS1_URL:
        case CSS1_RGB:
        case CSS1_HEXCOLOR:
            pNew = new CSS1Expression( m_nToken, m_aToken, m_nValue, cOp );
            m_nValue = 0; // otherwise this also is applied to next ident
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

        // if created a new expression save it
        if( pNew )
        {
            OSL_ENSURE( (pRoot!=nullptr) == (pLast!=nullptr),
                    "Root-Selector, but no Last" );
            if( pLast )
                pLast->SetNext( pNew );
            else
                pRoot.reset(pNew);

            pLast = pNew;
            pNew = nullptr;
        }

        if( !bDone )
            m_nToken = GetNextToken();
    }

    if( !pRoot )
    {
        // missing term
        return pRoot;
    }

    // prio?
    if( CSS1_IMPORTANT_SYM==m_nToken )
    {
        // IMPORTANT_SYM
        m_nToken = GetNextToken();
    }

    return pRoot;
}

CSS1Parser::CSS1Parser()
    : m_bWhiteSpace(false)
    , m_bEOF(false)
    , m_cNextCh(0)
    , m_nInPos(0)
    , m_nlLineNr(0)
    , m_nlLinePos(0)
    , m_nValue(0)
    , m_eState(CSS1_PAR_ACCEPTED)
    , m_nToken(CSS1_NULL)
{
}

CSS1Parser::~CSS1Parser()
{
}

void CSS1Parser::ParseStyleSheet( const OUString& rIn )
{
    OUString aTmp( rIn );

    sal_Unicode c;
    while( !aTmp.isEmpty() &&
           ( ' '==(c=aTmp[0]) || '\t'==c || '\r'==c || '\n'==c ) )
        aTmp = aTmp.copy( 1 );

    while( !aTmp.isEmpty() && ( ' '==(c=aTmp[aTmp.getLength()-1])
           || '\t'==c || '\r'==c || '\n'==c ) )
        aTmp = aTmp.copy( 0, aTmp.getLength()-1 );

    // remove SGML comments
    if( aTmp.getLength() >= 4 &&
        aTmp.startsWith( "<!--" ) )
        aTmp = aTmp.copy( 4 );

    if( aTmp.getLength() >=3 &&
        aTmp.endsWith("-->") )
        aTmp = aTmp.copy( 0, aTmp.getLength() - 3 );

    if( aTmp.isEmpty() )
        return;

    InitRead( aTmp );

    ParseStyleSheet();
}

void CSS1Parser::ParseStyleOption( const OUString& rIn )
{
    if( rIn.isEmpty() )
        return;

    InitRead( rIn );

    // fdo#41796: skip over spurious semicolons
    while (CSS1_SEMICOLON == m_nToken)
    {
        m_nToken = GetNextToken();
    }

    OUString aProperty;
    std::unique_ptr<CSS1Expression> pExpr = ParseDeclaration( aProperty );
    if( !pExpr )
        return;

    // process expression
    DeclarationParsed( aProperty, std::move(pExpr) );

    LOOP_CHECK_DECL

    // [ ';' declaration ]*
    while( CSS1_SEMICOLON==m_nToken && IsParserWorking() )
    {
        LOOP_CHECK_CHECK( "Infinite loop in ParseStyleOption()" )

        m_nToken = GetNextToken();
        if( CSS1_IDENT==m_nToken )
        {
            std::unique_ptr<CSS1Expression> pExp = ParseDeclaration( aProperty );
            if( pExp )
            {
                // process expression
                DeclarationParsed( aProperty, std::move(pExp) );
            }
        }
    }
}

void CSS1Parser::SelectorParsed( std::unique_ptr<CSS1Selector> /* pSelector */, bool /*bFirst*/ )
{
}

void CSS1Parser::DeclarationParsed( const OUString& /*rProperty*/,
                                    std::unique_ptr<CSS1Expression> /* pExpr */ )
{
}

CSS1Selector::~CSS1Selector()
{
    delete pNext;
}

CSS1Expression::~CSS1Expression()
{
    delete pNext;
}

void CSS1Expression::GetURL( OUString& rURL  ) const
{
    OSL_ENSURE( CSS1_URL==eType, "CSS1-Expression is not URL" );

    OSL_ENSURE( aValue.startsWithIgnoreAsciiCase( "url" ) &&
                aValue.getLength() > 5 &&
                '(' == aValue[3] &&
                ')' == aValue[aValue.getLength()-1],
                "no valid URL(...)" );

    if( aValue.getLength() > 5 )
    {
        rURL = aValue.copy( 4, aValue.getLength() - 5 );

        // tdf#94088 original stripped only spaces, but there may also be
        // double quotes in CSS style URLs, so be prepared to spaces followed
        // by a single quote followed by spaces
        const sal_Unicode aSpace(' ');
        const sal_Unicode aSingleQuote('\'');

        rURL = comphelper::string::strip(rURL, aSpace);
        rURL = comphelper::string::strip(rURL, aSingleQuote);
        rURL = comphelper::string::strip(rURL, aSpace);
    }
}

bool CSS1Expression::GetColor( Color &rColor ) const
{
    OSL_ENSURE( CSS1_IDENT==eType || CSS1_RGB==eType ||
                CSS1_HEXCOLOR==eType || CSS1_STRING==eType,
                "CSS1-Expression cannot be colour" );

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

            bRet = true;    // something different than a colour isn't possible
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
        [[fallthrough]];
    case CSS1_HEXCOLOR:
        {
            // MS-IE hack: colour can also be a string
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
        rColor.SetRed( static_cast<sal_uInt8>((nColor & 0x00ff0000UL) >> 16) );
        rColor.SetGreen( static_cast<sal_uInt8>((nColor & 0x0000ff00UL) >> 8) );
        rColor.SetBlue( static_cast<sal_uInt8>(nColor & 0x000000ffUL) );
    }

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
