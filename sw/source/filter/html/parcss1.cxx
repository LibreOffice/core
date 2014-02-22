/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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

    bWhiteSpace = sal_True; 
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


//


//


//
CSS1Token CSS1Parser::GetNextToken()
{
    CSS1Token nRet = CSS1_NULL;
    aToken = "";

    do {
        
        sal_Bool bPrevWhiteSpace = bWhiteSpace;
        bWhiteSpace = sal_False;

        sal_Bool bNextCh = sal_True;
        switch( cNextCh )
        {
        case '/': 
            {
                cNextCh = GetNextChar();
                if( '*' == cNextCh )
                {
                    
                    cNextCh = GetNextChar();

                    sal_Bool bAsterisk = sal_False;
                    while( !(bAsterisk && '/'==cNextCh) && !IsEOF() )
                    {
                        bAsterisk = ('*'==cNextCh);
                        cNextCh = GetNextChar();
                    }
                }
                else
                {
                    
                    bNextCh = sal_False;
                    nRet = CSS1_SLASH;
                }
            }
            break;

        case '@': 
            {
                cNextCh = GetNextChar();
                if (comphelper::string::isalphaAscii(cNextCh))
                {
                    
                    OUStringBuffer sTmpBuffer( 32L );
                    do {
                        sTmpBuffer.append( cNextCh );
                        cNextCh = GetNextChar();
                    } while( (comphelper::string::isalnumAscii(cNextCh) ||
                             '-' == cNextCh) && !IsEOF() );

                    aToken += sTmpBuffer.makeStringAndClear();

                    
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

        case '!': 
            {
                
                cNextCh = GetNextChar();
                while( ( ' ' == cNextCh ||
                       (cNextCh >= 0x09 && cNextCh <= 0x0d) ) && !IsEOF() )
                {
                    bWhiteSpace = sal_True;
                    cNextCh = GetNextChar();
                }

                if( 'i'==cNextCh || 'I'==cNextCh)
                {
                    
                    OUStringBuffer sTmpBuffer( 32L );
                    do {
                        sTmpBuffer.append( cNextCh );
                        cNextCh = GetNextChar();
                    } while( (comphelper::string::isalnumAscii(cNextCh) ||
                             '-' == cNextCh) && !IsEOF() );

                    aToken += sTmpBuffer.makeStringAndClear();

                    if( ( 'i'==aToken[0] || 'I'==aToken[0] ) &&
                        aToken.equalsIgnoreAsciiCase( "important" ) )
                    {
                        
                        nRet = CSS1_IMPORTANT_SYM;
                    }
                    else
                    {
                        
                        nRet = CSS1_IDENT;
                    }

                    bWhiteSpace = sal_False;
                    bNextCh = sal_False;
                }
                else
                {
                    
                    bNextCh = sal_False;
                }
            }
            break;

        case '\"':
        case '\'': 
            {
                
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
        case '9': 
            {
                
                sal_Size nInPosSave = nInPos;
                sal_Unicode cNextChSave = cNextCh;
                sal_uInt32 nlLineNrSave = nlLineNr;
                sal_uInt32 nlLinePosSave = nlLinePos;
                sal_Bool bEOFSave = bEOF;

                
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
                    
                    aToken += sTmpBuffer.makeStringAndClear();
                    nRet = CSS1_HEXCOLOR;
                    bNextCh = sal_False;

                    break;
                }

                
                nInPos = nInPosSave;
                cNextCh = cNextChSave;
                nlLineNr = nlLineNrSave;
                nlLinePos = nlLinePosSave;
                bEOF = bEOFSave;

                
                sTmpBuffer.setLength( 0L );
                do {
                    sTmpBuffer.append( cNextCh );
                    cNextCh = GetNextChar();
                } while( (('0'<=cNextCh && '9'>=cNextCh) || '.'==cNextCh) &&
                         !IsEOF() );

                aToken += sTmpBuffer.makeStringAndClear();
                nValue = aToken.toDouble();

                
                while( ( ' ' == cNextCh ||
                       (cNextCh >= 0x09 && cNextCh <= 0x0d) ) && !IsEOF() )
                {
                    bWhiteSpace = sal_True;
                    cNextCh = GetNextChar();
                }

                
                switch( cNextCh )
                {
                case '%': 
                    bWhiteSpace = sal_False;
                    nRet = CSS1_PERCENTAGE;
                    break;

                case 'c':
                case 'C': 
                case 'e':
                case 'E': 
                case 'i':
                case 'I': 
                case 'p':
                case 'P': 
                case 'm':
                case 'M': 
                    {
                        
                        sal_Int32 nInPosOld = nInPos;
                        sal_Unicode cNextChOld = cNextCh;
                        sal_uLong nlLineNrOld  = nlLineNr;
                        sal_uLong nlLinePosOld = nlLinePos;
                        sal_Bool bEOFOld = bEOF;

                        
                        OUString aIdent;
                        OUStringBuffer sTmpBuffer2( 64L );
                        do {
                            sTmpBuffer2.append( cNextCh );
                            cNextCh = GetNextChar();
                        } while( (comphelper::string::isalnumAscii(cNextCh) ||
                                 '-' == cNextCh) && !IsEOF() );

                        aIdent += sTmpBuffer2.makeStringAndClear();

                        
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
                            nScale1 = (72.*20.)/2.54; 
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
                            nScale1 = 72.*20.; 
                            break;
                        case 'm':
                        case 'M':
                            pCmp1 = "mm";
                            nScale1 = (72.*20.)/25.4; 
                            break;
                        case 'p':
                        case 'P':
                            pCmp1 = "pt";
                            nScale1 = 20.; 

                            pCmp2 = "pc";
                            nScale2 = 12.*20.; 

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
                            nScale =  1.; 
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
                default: 
                    bNextCh = sal_False;
                    nRet = CSS1_NUMBER;
                    break;
                }
            }
            break;

        case ':': 
            
            nRet = CSS1_COLON;
            break;

        case '.': 
            nRet = bPrevWhiteSpace ? CSS1_DOT_W_WS : CSS1_DOT_WO_WS;
            break;

        case '+': 
            nRet = CSS1_PLUS;
            break;

        case '-': 
            nRet = CSS1_MINUS;
            break;

        case '{': 
            nRet = CSS1_OBRACE;
            break;

        case '}': 
            nRet = CSS1_CBRACE;
            break;

        case ';': 
            nRet = CSS1_SEMICOLON;
            break;

        case ',': 
            nRet = CSS1_COMMA;
            break;

        case '#': 
            cNextCh = GetNextChar();
            if( ('0'<=cNextCh && '9'>=cNextCh) ||
                ('a'<=cNextCh && 'f'>=cNextCh) ||
                ('A'<=cNextCh && 'F'>=cNextCh) )
            {
                
                sal_Int32 nInPosSave = nInPos;
                sal_Unicode cNextChSave = cNextCh;
                sal_uLong nlLineNrSave = nlLineNr;
                sal_uLong nlLinePosSave = nlLinePos;
                sal_Bool bEOFSave = bEOF;

                
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
                    
                    aToken += sTmpBuffer.makeStringAndClear();
                    nRet = CSS1_HEXCOLOR;
                    bNextCh = sal_False;

                    break;
                }

                
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
        case '\n': 
            bWhiteSpace = sal_True;
            break;

        case (sal_Unicode)EOF:
            if( IsEOF() )
            {
                eState = CSS1_PAR_ACCEPTED;
                bNextCh = sal_False;
                break;
            }
            

        default: 
            if (comphelper::string::isalphaAscii(cNextCh))
            {
                

                sal_Bool bHexColor = sal_True;

                
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
            
            break;
        }
        if( bNextCh )
            cNextCh = GetNextChar();

    } while( CSS1_NULL==nRet && IsParserWorking() );

    return nRet;
}



//


//





//


//


//
void CSS1Parser::ParseStyleSheet()
{
    LOOP_CHECK_DECL

    
    sal_Bool bDone = sal_False;
    while( !bDone && IsParserWorking() )
    {
        LOOP_CHECK_CHECK( "Endlos-Schleife in ParseStyleSheet()/import *" )

        switch( nToken )
        {
        case CSS1_IMPORT_SYM:
            
            
            nToken = GetNextToken();
            break;
        case CSS1_IDENT:            
        case CSS1_DOT_W_WS:
        case CSS1_HASH:

        case CSS1_PAGE_SYM:

            
            bDone = sal_True;
            break;
        default:
            
            break;
        }

        if( !bDone )
            nToken = GetNextToken();
    }

    LOOP_CHECK_RESTART

    
    while( IsParserWorking() )
    {
        LOOP_CHECK_CHECK( "Endlos-Schleife in ParseStyleSheet()/rule *" )

        switch( nToken )
        {
        case CSS1_IDENT:        
        case CSS1_DOT_W_WS:
        case CSS1_HASH:

        case CSS1_PAGE_SYM:

            
            ParseRule();
            break;
        default:
            
            nToken = GetNextToken();
            break;
        }
    }
}




//
void CSS1Parser::ParseRule()
{
    
    CSS1Selector *pSelector = ParseSelector();
    if( !pSelector )
        return;

    
    if( SelectorParsed( pSelector, true ) )
        delete pSelector;

    LOOP_CHECK_DECL

    
    while( CSS1_COMMA==nToken && IsParserWorking() )
    {
        LOOP_CHECK_CHECK( "Endlos-Schleife in ParseRule()/selector *" )

        
        nToken = GetNextToken();

        
        pSelector = ParseSelector();
        if( !pSelector )
            return;

        
        if( SelectorParsed( pSelector, false ) )
            delete pSelector;
    }

    
    if( CSS1_OBRACE != nToken )
        return;
    nToken = GetNextToken();

    
    OUString aProperty;
    CSS1Expression *pExpr = ParseDeclaration( aProperty );
    if( !pExpr )
        return;

    
    if( DeclarationParsed( aProperty, pExpr ) )
        delete pExpr;

    LOOP_CHECK_RESTART

    
    while( CSS1_SEMICOLON==nToken && IsParserWorking() )
    {
        LOOP_CHECK_CHECK( "Endlos-Schleife in ParseRule()/declaration *" )

        
        nToken = GetNextToken();

        
        if( CSS1_IDENT == nToken )
        {
            CSS1Expression *pExp = ParseDeclaration( aProperty );
            if( pExp )
            {
                
                if( DeclarationParsed( aProperty, pExp ) )
                    delete pExp;
            }
        }
    }

    
    if( CSS1_CBRACE == nToken )
        nToken = GetNextToken();
}



//




//


//


//


//


//
CSS1Selector *CSS1Parser::ParseSelector()
{
    CSS1Selector *pRoot = 0, *pLast = 0;

    sal_Bool bDone = sal_False;
    CSS1Selector *pNew = 0;

    LOOP_CHECK_DECL

    
    while( !bDone && IsParserWorking() )
    {
        LOOP_CHECK_CHECK( "Endlos-Schleife in ParseSelector()" )

        sal_Bool bNextToken = sal_True;

        switch( nToken )
        {
        case CSS1_IDENT:
            {
                

                
                OUString aElement = aToken;
                CSS1SelectorType eType = CSS1_SELTYPE_ELEMENT;
                nToken = GetNextToken();

                if( CSS1_DOT_WO_WS == nToken )
                {
                    
                    nToken = GetNextToken();

                    
                    if( CSS1_IDENT == nToken )
                    {
                        aElement += "." + aToken;
                        eType = CSS1_SELTYPE_ELEM_CLASS;
                    }
                    else
                    {
                        
                        return pRoot;
                    }
                }
                else
                {
                    
                    bNextToken = sal_False;
                }
                pNew = new CSS1Selector( eType, aElement );
            }
            break;
        case CSS1_DOT_W_WS:
            

            
            nToken = GetNextToken();

            if( CSS1_IDENT==nToken )
            {
                
                pNew = new CSS1Selector( CSS1_SELTYPE_CLASS, aToken );
            }
            else
            {
                
                return pRoot;
            }
            break;
        case CSS1_HASH:
            

            
            nToken = GetNextToken();

            if( CSS1_IDENT==nToken )
            {
                
                pNew = new CSS1Selector( CSS1_SELTYPE_ID, aToken );
            }
            else
            {
                
                return pRoot;
            }
            break;


        case CSS1_PAGE_SYM:
            {
                
                pNew = new CSS1Selector( CSS1_SELTYPE_PAGE, aToken );
            }
            break;


        default:
            
            bDone = sal_True;
            break;
        }

        
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
        
        return pRoot;
    }

    
    if( CSS1_COLON==nToken && IsParserWorking() )
    {
        
        nToken = GetNextToken();
        if( CSS1_IDENT==nToken )
        {
            pLast->SetNext( new CSS1Selector(CSS1_SELTYPE_PSEUDO,aToken) );
            nToken = GetNextToken();
        }
        else
        {
            
            return pRoot;
        }
    }

    return pRoot;
}




//


//




//


//


//


//


CSS1Expression *CSS1Parser::ParseDeclaration( OUString& rProperty )
{
    CSS1Expression *pRoot = 0, *pLast = 0;

    
    if( CSS1_IDENT != nToken )
    {
        
        return pRoot;
    }
    rProperty = aToken;

    nToken = GetNextToken();


    
    if( CSS1_COLON != nToken )
    {
        
        return pRoot;
    }
    nToken = GetNextToken();

    
    
    
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
            nValue = 0; 
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
        
        return pRoot;
    }

    
    if( CSS1_IMPORTANT_SYM==nToken )
    {
        
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

    
    if( DeclarationParsed( aProperty, pExpr ) )
        delete pExpr;

    LOOP_CHECK_DECL

    
    while( CSS1_SEMICOLON==nToken && IsParserWorking() )
    {
        LOOP_CHECK_CHECK( "Endlos-Schleife in ParseStyleOption()" )

        nToken = GetNextToken();
        if( CSS1_IDENT==nToken )
        {
            CSS1Expression *pExp = ParseDeclaration( aProperty );
            if( pExp )
            {
                
                if( DeclarationParsed( aProperty, pExp ) )
                    delete pExp;
            }
        }
    }

    return sal_True;
}

bool CSS1Parser::SelectorParsed( CSS1Selector* /* pSelector */, bool /*bFirst*/ )
{
    
    return true;
}

sal_Bool CSS1Parser::DeclarationParsed( const OUString& /*rProperty*/,
                                    const CSS1Expression * /* pExpr */ )
{
    
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

            if (!aValue.startsWithIgnoreAsciiCase( "rgb" ) || aValue.getLength() < 6 ||
                    aValue[3] != '(' || aValue[aValue.getLength()-1] != ')')
            {
                break;
            }

            OUString aColorStr(aValue.copy(4, aValue.getLength() - 5));

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

            bRet = sal_True;    
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
