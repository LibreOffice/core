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

#include "sal/config.h"

#include <cassert>

#include <unicode/uchar.h>
#include <comphelper/syntaxhighlight.hxx>
#include <comphelper/string.hxx>


#define CHAR_START_IDENTIFIER   0x0001
#define CHAR_IN_IDENTIFIER      0x0002
#define CHAR_START_NUMBER       0x0004
#define CHAR_IN_NUMBER          0x0008
#define CHAR_IN_HEX_NUMBER      0x0010
#define CHAR_IN_OCT_NUMBER      0x0020
#define CHAR_START_STRING       0x0040
#define CHAR_OPERATOR           0x0080
#define CHAR_SPACE              0x0100
#define CHAR_EOL                0x0200




static const char* strListBasicKeyWords[] = {
    "access",
    "alias",
    "and",
    "any",
    "append",
    "as",
    "attribute",
    "base",
    "binary",
    "boolean",
    "byref",
    "byte",
    "byval",
    "call",
    "case",
    "cdecl",
    "classmodule",
    "close",
    "compare",
    "compatible",
    "const",
    "currency",
    "date",
    "declare",
    "defbool",
    "defcur",
    "defdate",
    "defdbl",
    "deferr",
    "defint",
    "deflng",
    "defobj",
    "defsng",
    "defstr",
    "defvar",
    "dim",
    "do",
    "double",
    "each",
    "else",
    "elseif",
    "end",
    "end enum",
    "end function",
    "end if",
    "end property",
    "end select",
    "end sub",
    "end type",
    "endif",
    "enum",
    "eqv",
    "erase",
    "error",
    "exit",
    "explicit",
    "for",
    "function",
    "get",
    "global",
    "gosub",
    "goto",
    "if",
    "imp",
    "implements",
    "in",
    "input",
    "integer",
    "is",
    "let",
    "lib",
    "like",
    "line",
    "line input",
    "local",
    "lock",
    "long",
    "loop",
    "lprint",
    "lset",
    "mod",
    "name",
    "new",
    "next",
    "not",
    "object",
    "on",
    "open",
    "option",
    "optional",
    "or",
    "output",
    "paramarray",
    "preserve",
    "print",
    "private",
    "property",
    "public",
    "random",
    "read",
    "redim",
    "rem",
    "resume",
    "return",
    "rset",
    "select",
    "set",
    "shared",
    "single",
    "static",
    "step",
    "stop",
    "string",
    "sub",
    "system",
    "text",
    "then",
    "to",
    "type",
    "typeof",
    "until",
    "variant",
    "vbasupport",
    "wend",
    "while",
    "with",
    "withevents",
    "write",
    "xor"
};


static const char* strListSqlKeyWords[] = {
    "all",
    "and",
    "any",
    "as",
    "asc",
    "avg",
    "between",
    "by",
    "cast",
    "corresponding",
    "count",
    "create",
    "cross",
    "delete",
    "desc",
    "distinct",
    "drop",
    "escape",
    "except",
    "exists",
    "false",
    "from",
    "full",
    "global",
    "group",
    "having",
    "in",
    "inner",
    "insert",
    "intersect",
    "into",
    "is",
    "join",
    "left",
    "like",
    "limit",
    "local",
    "match",
    "max",
    "min",
    "natural",
    "not",
    "null",
    "on",
    "or",
    "order",
    "outer",
    "right",
    "select",
    "set",
    "some",
    "sum",
    "table",
    "temporary",
    "true",
    "union",
    "unique",
    "unknown",
    "update",
    "using",
    "values",
    "where"
};


extern "C" int compare_strings( const void *arg1, const void *arg2 )
{
    return strcmp( (char *)arg1, *(char **)arg2 );
}


namespace
{
    bool isAlpha(sal_Unicode c)
    {
        if (comphelper::string::isalphaAscii(c))
            return true;
        return u_isalpha(c);
    }
}

class SyntaxHighlighter::Tokenizer
{
    
    sal_uInt16 aCharTypeTab[256];

    
    bool testCharFlags( sal_Unicode c, sal_uInt16 nTestFlags );

    
    bool getNextToken( const sal_Unicode*& pos, /*out*/TokenTypes& reType,
        /*out*/const sal_Unicode*& rpStartPos, /*out*/const sal_Unicode*& rpEndPos );

    const char** ppListKeyWords;
    sal_uInt16 nKeyWordCount;

public:
    HighlighterLanguage const aLanguage;

    Tokenizer( HighlighterLanguage aLang );
    ~Tokenizer( void );

    void getHighlightPortions( const OUString& rLine,
                               /*out*/std::vector<HighlightPortion>& portions );
    void setKeyWords( const char** ppKeyWords, sal_uInt16 nCount );
};


bool SyntaxHighlighter::Tokenizer::testCharFlags( sal_Unicode c, sal_uInt16 nTestFlags )
{
    bool bRet = false;
    if( c != 0 && c <= 255 )
    {
        bRet = ( (aCharTypeTab[c] & nTestFlags) != 0 );
    }
    else if( c > 255 )
    {
        bRet = (( CHAR_START_IDENTIFIER | CHAR_IN_IDENTIFIER ) & nTestFlags) != 0
            ? isAlpha(c) : false;
    }
    return bRet;
}

void SyntaxHighlighter::Tokenizer::setKeyWords( const char** ppKeyWords, sal_uInt16 nCount )
{
    ppListKeyWords = ppKeyWords;
    nKeyWordCount = nCount;
}

bool SyntaxHighlighter::Tokenizer::getNextToken( const sal_Unicode*& pos, /*out*/TokenTypes& reType,
    /*out*/const sal_Unicode*& rpStartPos, /*out*/const sal_Unicode*& rpEndPos )
{
    reType = TT_UNKNOWN;

    rpStartPos = pos;

    sal_Unicode c = *pos;
    if( c == 0 )
        return false;

    ++pos;

    
    
    if ( testCharFlags( c, CHAR_SPACE ) )
    {
        while( testCharFlags( *pos, CHAR_SPACE ) )
            ++pos;

        reType = TT_WHITESPACE;
    }

    
    else if ( testCharFlags( c, CHAR_START_IDENTIFIER ) )
    {
        bool bIdentifierChar;
        do
        {
            
            c = *pos;
            bIdentifierChar = testCharFlags( c, CHAR_IN_IDENTIFIER );
            if( bIdentifierChar )
                ++pos;
        }
        while( bIdentifierChar );

        reType = TT_IDENTIFIER;

        
        if (ppListKeyWords != NULL)
        {
            int nCount = pos - rpStartPos;

            
            bool bCanBeKeyword = true;
            for( int i = 0 ; i < nCount ; i++ )
            {
                if( rpStartPos[i] > 255 )
                {
                    bCanBeKeyword = false;
                    break;
                }
            }

            if( bCanBeKeyword )
            {
                OUString aKWString(rpStartPos, nCount);
                OString aByteStr = OUStringToOString(aKWString,
                    RTL_TEXTENCODING_ASCII_US).toAsciiLowerCase();
                if ( bsearch( aByteStr.getStr(), ppListKeyWords, nKeyWordCount, sizeof( char* ),
                                                                        compare_strings ) )
                {
                    reType = TT_KEYWORDS;

                    if( aByteStr == "rem" )
                    {
                        
                        sal_Unicode cPeek = *pos;
                        while( cPeek != 0 && !testCharFlags( cPeek, CHAR_EOL ) )
                        {
                            c = *pos++;
                            cPeek = *pos;
                        }

                        reType = TT_COMMENT;
                    }
                }
            }
        }
    }

    
    
    else if ( testCharFlags( c, CHAR_OPERATOR ) || ( (c == '\'') && (aLanguage==HIGHLIGHT_BASIC)) )
    {
        
        if ( (c==':') || (c=='?'))
        {
            if (c!='?')
            {
                bool bIdentifierChar;
                do
                {
                    
                    c = *pos;
                    bIdentifierChar = isAlpha(c);
                    if( bIdentifierChar )
                        ++pos;
                }
                while( bIdentifierChar );
            }
            reType = TT_PARAMETER;
        }
        else if (c=='-')
        {
            sal_Unicode cPeekNext = *pos;
            if (cPeekNext=='-')
            {
                
                while( cPeekNext != 0 && !testCharFlags( cPeekNext, CHAR_EOL ) )
                {
                    ++pos;
                    cPeekNext = *pos;
                }
                reType = TT_COMMENT;
            }
        }
       else if (c=='/')
       {
           sal_Unicode cPeekNext = *pos;
           if (cPeekNext=='/')
           {
               
               while( cPeekNext != 0 && !testCharFlags( cPeekNext, CHAR_EOL ) )
               {
                   ++pos;
                   cPeekNext = *pos;
               }
               reType = TT_COMMENT;
           }
       }
        else
        {
            
            if ( c == '\'' )
            {
                
                for (;;) {
                    c = *pos;
                    if (c == 0 || testCharFlags(c, CHAR_EOL)) {
                        break;
                    }
                    ++pos;
                }

                reType = TT_COMMENT;
            }

            
            
            if( reType != TT_COMMENT )
            {
                reType = TT_OPERATOR;
            }

        }
    }

    
    else if( c == '.' && ( *pos < '0' || *pos > '9' ) )
    {
        reType = TT_OPERATOR;
    }

    
    else if( testCharFlags( c, CHAR_START_NUMBER ) )
    {
        reType = TT_NUMBER;

        
        int nRadix = 10;

        
        if( c == '&' )
        {
            
            if( *pos == 'o' || *pos == 'O' )
            {
                
                ++pos;
                nRadix = 8;     

                
                while( testCharFlags( *pos, CHAR_IN_OCT_NUMBER ) )
                    c = *pos++;
            }
            
            else if( *pos == 'h' || *pos == 'H' )
            {
                
                ++pos;
                nRadix = 16;     

                
                while( testCharFlags( *pos, CHAR_IN_HEX_NUMBER ) )
                    c = *pos++;
            }
            else
            {
                reType = TT_OPERATOR;
            }
        }

        
        if( reType == TT_NUMBER && nRadix == 10 )
        {
            
            bool bAfterExpChar = false;

            
            while( testCharFlags( *pos, CHAR_IN_NUMBER ) ||
                    (bAfterExpChar && *pos == '+' ) ||
                    (bAfterExpChar && *pos == '-' ) )
                    
            {
                c = *pos++;
                bAfterExpChar = ( c == 'e' || c == 'E' );
            }
        }
    }

    
    else if( testCharFlags( c, CHAR_START_STRING ) )
    {
        
        sal_Unicode cEndString = c;
        if( c == '[' )
            cEndString = ']';

        
        while( *pos != cEndString )
        {
            
            if( *pos == 0 )
            {
                
                reType = TT_ERROR;
                break;
            }
            c = *pos++;
            if( testCharFlags( c, CHAR_EOL ) )
            {
                
                reType = TT_ERROR;
                break;
            }
        }

        if( reType != TT_ERROR )
        {
            ++pos;
            if( cEndString == ']' )
                reType = TT_IDENTIFIER;
            else
                reType = TT_STRING;
        }
    }

    
    else if( testCharFlags( c, CHAR_EOL ) )
    {
        
        sal_Unicode cNext = *pos;
        if( cNext != c && testCharFlags( cNext, CHAR_EOL ) )
            ++pos;

        reType = TT_EOL;
    }

    

    
    rpEndPos = pos;
    return true;
}

SyntaxHighlighter::Tokenizer::Tokenizer( HighlighterLanguage aLang ): aLanguage(aLang)
{
    memset( aCharTypeTab, 0, sizeof( aCharTypeTab ) );

    
    sal_uInt16 i;

    
    sal_uInt16 nHelpMask = (sal_uInt16)( CHAR_START_IDENTIFIER | CHAR_IN_IDENTIFIER );
    for( i = 'a' ; i <= 'z' ; i++ )
        aCharTypeTab[i] |= nHelpMask;
    for( i = 'A' ; i <= 'Z' ; i++ )
        aCharTypeTab[i] |= nHelpMask;
    aCharTypeTab[(int)'_'] |= nHelpMask;
    aCharTypeTab[(int)'$'] |= nHelpMask;

    
    nHelpMask = (sal_uInt16)( CHAR_IN_IDENTIFIER | CHAR_START_NUMBER |
                         CHAR_IN_NUMBER | CHAR_IN_HEX_NUMBER );
    for( i = '0' ; i <= '9' ; i++ )
        aCharTypeTab[i] |= nHelpMask;

    
    aCharTypeTab[(int)'e'] |= CHAR_IN_NUMBER;
    aCharTypeTab[(int)'E'] |= CHAR_IN_NUMBER;
    aCharTypeTab[(int)'.'] |= (sal_uInt16)( CHAR_IN_NUMBER | CHAR_START_NUMBER );
    aCharTypeTab[(int)'&'] |= CHAR_START_NUMBER;

    
    for( i = 'a' ; i <= 'f' ; i++ )
        aCharTypeTab[i] |= CHAR_IN_HEX_NUMBER;
    for( i = 'A' ; i <= 'F' ; i++ )
        aCharTypeTab[i] |= CHAR_IN_HEX_NUMBER;

    
    for( i = '0' ; i <= '7' ; i++ )
        aCharTypeTab[i] |= CHAR_IN_OCT_NUMBER;

    
    aCharTypeTab[(int)'\''] |= CHAR_START_STRING;
    aCharTypeTab[(int)'\"'] |= CHAR_START_STRING;
    aCharTypeTab[(int)'[']  |= CHAR_START_STRING;
    aCharTypeTab[(int)'`']  |= CHAR_START_STRING;

    
    aCharTypeTab[(int)'!'] |= CHAR_OPERATOR;
    aCharTypeTab[(int)'%'] |= CHAR_OPERATOR;
    
    aCharTypeTab[(int)'('] |= CHAR_OPERATOR;
    aCharTypeTab[(int)')'] |= CHAR_OPERATOR;
    aCharTypeTab[(int)'*'] |= CHAR_OPERATOR;
    aCharTypeTab[(int)'+'] |= CHAR_OPERATOR;
    aCharTypeTab[(int)','] |= CHAR_OPERATOR;
    aCharTypeTab[(int)'-'] |= CHAR_OPERATOR;
    aCharTypeTab[(int)'/'] |= CHAR_OPERATOR;
    aCharTypeTab[(int)':'] |= CHAR_OPERATOR;
    aCharTypeTab[(int)'<'] |= CHAR_OPERATOR;
    aCharTypeTab[(int)'='] |= CHAR_OPERATOR;
    aCharTypeTab[(int)'>'] |= CHAR_OPERATOR;
    aCharTypeTab[(int)'?'] |= CHAR_OPERATOR;
    aCharTypeTab[(int)'^'] |= CHAR_OPERATOR;
    aCharTypeTab[(int)'|'] |= CHAR_OPERATOR;
    aCharTypeTab[(int)'~'] |= CHAR_OPERATOR;
    aCharTypeTab[(int)'{'] |= CHAR_OPERATOR;
    aCharTypeTab[(int)'}'] |= CHAR_OPERATOR;
    
    aCharTypeTab[(int)']'] |= CHAR_OPERATOR;
    aCharTypeTab[(int)';'] |= CHAR_OPERATOR;

    
    aCharTypeTab[(int)' ' ] |= CHAR_SPACE;
    aCharTypeTab[(int)'\t'] |= CHAR_SPACE;

    
    aCharTypeTab[(int)'\r'] |= CHAR_EOL;
    aCharTypeTab[(int)'\n'] |= CHAR_EOL;

    ppListKeyWords = NULL;
    nKeyWordCount = 0;
}

SyntaxHighlighter::Tokenizer::~Tokenizer( void )
{
}

void SyntaxHighlighter::Tokenizer::getHighlightPortions( const OUString& rLine,
                                                 /*out*/std::vector<HighlightPortion>& portions  )
{
    
    const sal_Unicode* pos = rLine.getStr();

    
    TokenTypes eType;
    const sal_Unicode* pStartPos;
    const sal_Unicode* pEndPos;

    
    while( getNextToken( pos, eType, pStartPos, pEndPos ) )
    {
        portions.push_back(
            HighlightPortion(
                pStartPos - rLine.getStr(), pEndPos - rLine.getStr(), eType));
    }
}


SyntaxHighlighter::SyntaxHighlighter(HighlighterLanguage language):
    eLanguage(language), m_tokenizer(new SyntaxHighlighter::Tokenizer(language))
{
    switch (eLanguage)
    {
        case HIGHLIGHT_BASIC:
            m_tokenizer->setKeyWords( strListBasicKeyWords,
                                            sizeof( strListBasicKeyWords ) / sizeof( char* ));
            break;
        case HIGHLIGHT_SQL:
            m_tokenizer->setKeyWords( strListSqlKeyWords,
                                            sizeof( strListSqlKeyWords ) / sizeof( char* ));
            break;
        default:
            assert(false); 
    }
}

SyntaxHighlighter::~SyntaxHighlighter() {}

void SyntaxHighlighter::getHighlightPortions( const OUString& rLine,
                                              /*out*/std::vector<HighlightPortion>& portions )
{
    m_tokenizer->getHighlightPortions( rLine, portions );
}

HighlighterLanguage SyntaxHighlighter::GetLanguage()
{
    return m_tokenizer->aLanguage;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
