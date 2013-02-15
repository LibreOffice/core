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


#include <unicode/uchar.h>
#include <comphelper/syntaxhighlight.hxx>
#include <comphelper/string.hxx>

// ##########################################################################
// ATTENTION: all these words need to be in lower case
// ##########################################################################
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


extern "C" int CDECL compare_strings( const void *arg1, const void *arg2 )
{
    return strcmp( (char *)arg1, *(char **)arg2 );
}


namespace
{
    static bool isAlpha(sal_Unicode c)
    {
        if (comphelper::string::isalphaAscii(c))
            return true;
        return u_isalpha(c);
    }
}

// Helper function: test character flag
sal_Bool SimpleTokenizer_Impl::testCharFlags( sal_Unicode c, sal_uInt16 nTestFlags )
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

void SimpleTokenizer_Impl::setKeyWords( const char** ppKeyWords, sal_uInt16 nCount )
{
    ppListKeyWords = ppKeyWords;
    nKeyWordCount = nCount;
}

sal_Bool SimpleTokenizer_Impl::getNextToken( /*out*/TokenTypes& reType,
    /*out*/const sal_Unicode*& rpStartPos, /*out*/const sal_Unicode*& rpEndPos )
{
    reType = TT_UNKNOWN;

    rpStartPos = mpActualPos;

    sal_Unicode c = peekChar();
    if( c == CHAR_EOF )
        return sal_False;

    getChar();

    //*** Go through all possibilities ***
    // Space?
    if ( (testCharFlags( c, CHAR_SPACE ) == sal_True) )
    {
        while( testCharFlags( peekChar(), CHAR_SPACE ) == sal_True )
            getChar();

        reType = TT_WHITESPACE;
    }

    // Identifier?
    else if ( (testCharFlags( c, CHAR_START_IDENTIFIER ) == sal_True) )
    {
        sal_Bool bIdentifierChar;
        do
        {
            // Naechstes Zeichen holen
            c = peekChar();
            bIdentifierChar = testCharFlags( c, CHAR_IN_IDENTIFIER );
            if( bIdentifierChar )
                getChar();
        }
        while( bIdentifierChar );

        reType = TT_IDENTIFIER;

        // Keyword table
        if (ppListKeyWords != NULL)
        {
            int nCount = mpActualPos - rpStartPos;

            // No keyword if string contains char > 255
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

                    if (aByteStr.equalsL(RTL_CONSTASCII_STRINGPARAM("rem")))
                    {
                        // Remove all characters until end of line or EOF
                        sal_Unicode cPeek = peekChar();
                        while( cPeek != CHAR_EOF && testCharFlags( cPeek, CHAR_EOL ) == sal_False )
                        {
                            c = getChar();
                            cPeek = peekChar();
                        }

                        reType = TT_COMMENT;
                    }
                }
            }
        }
    }

    // Operator?
    // only for BASIC '\'' should be a comment, otherwise it is a normal string and handled there
    else if ( ( testCharFlags( c, CHAR_OPERATOR ) == sal_True ) || ( (c == '\'') && (aLanguage==HIGHLIGHT_BASIC)) )
    {
        // parameters for SQL view
        if ( (c==':') || (c=='?'))
        {
            if (c!='?')
            {
                sal_Bool bIdentifierChar;
                do
                {
                    // Get next character
                    c = peekChar();
                    bIdentifierChar = isAlpha(c);
                    if( bIdentifierChar )
                        getChar();
                }
                while( bIdentifierChar );
            }
            reType = TT_PARAMETER;
        }
        else if (c=='-')
        {
            sal_Unicode cPeekNext = peekChar();
            if (cPeekNext=='-')
            {
                // Remove all characters until end of line or EOF
                while( cPeekNext != CHAR_EOF && testCharFlags( cPeekNext, CHAR_EOL ) == sal_False )
                {
                    getChar();
                    cPeekNext = peekChar();
                }
                reType = TT_COMMENT;
            }
        }
       else if (c=='/')
       {
           sal_Unicode cPeekNext = peekChar();
           if (cPeekNext=='/')
           {
               // Remove all characters until end of line or EOF
               while( cPeekNext != CHAR_EOF && testCharFlags( cPeekNext, CHAR_EOL ) == sal_False )
               {
                   getChar();
                   cPeekNext = peekChar();
               }
               reType = TT_COMMENT;
           }
       }
        else
        {
            // Comment?
            if ( c == '\'' )
            {
                c = getChar();

                // Remove all characters until end of line or EOF
                sal_Unicode cPeek = c;
                while( cPeek != CHAR_EOF && testCharFlags( cPeek, CHAR_EOL ) == sal_False )
                {
                    getChar();
                    cPeek = peekChar();
                }

                reType = TT_COMMENT;
            }

            // Echter Operator, kann hier einfach behandelt werden,
            // da nicht der wirkliche Operator, wie z.B. += interessiert,
            // sondern nur die Tatsache, dass es sich um einen handelt.
            if( reType != TT_COMMENT )
            {
                reType = TT_OPERATOR;
            }

        }
    }

    // Objekt-Trenner? Muss vor Number abgehandelt werden
    else if( c == '.' && ( peekChar() < '0' || peekChar() > '9' ) )
    {
        reType = TT_OPERATOR;
    }

    // Number?
    else if( testCharFlags( c, CHAR_START_NUMBER ) == sal_True )
    {
        reType = TT_NUMBER;

        // Number system, 10 = normal, it is changed for Oct/Hex
        int nRadix = 10;

        // Is it an Oct or a Hex number?
        if( c == '&' )
        {
            // Octal?
            if( peekChar() == 'o' || peekChar() == 'O' )
            {
                // remove o
                getChar();
                nRadix = 8;     // Octal base

                // Read all numbers
                while( testCharFlags( peekChar(), CHAR_IN_OCT_NUMBER ) )
                    c = getChar();
            }
            // Hexadecimal?
            else if( peekChar() == 'h' || peekChar() == 'H' )
            {
                // remove x
                getChar();
                nRadix = 16;     // Hexadecimal base

                // Read all numbers
                while( testCharFlags( peekChar(), CHAR_IN_HEX_NUMBER ) )
                    c = getChar();
            }
            else
            {
                reType = TT_OPERATOR;
            }
        }

        // When it is not Oct or Hex, then it is double
        if( reType == TT_NUMBER && nRadix == 10 )
        {
            // Flag if the last character is an exponent
            sal_Bool bAfterExpChar = sal_False;

            // Read all numbers
            while( testCharFlags( peekChar(), CHAR_IN_NUMBER ) ||
                    (bAfterExpChar && peekChar() == '+' ) ||
                    (bAfterExpChar && peekChar() == '-' ) )
                    // After exponent +/- are OK, too
            {
                c = getChar();
                bAfterExpChar = ( c == 'e' || c == 'E' );
            }
        }
    }

    // String?
    else if( testCharFlags( c, CHAR_START_STRING ) == sal_True )
    {
        // Remember which character has opened the string
        sal_Unicode cEndString = c;
        if( c == '[' )
            cEndString = ']';

        // Read all characters
        while( peekChar() != cEndString )
        {
            // Detect EOF before getChar(), so we do not loose EOF
            if( peekChar() == CHAR_EOF )
            {
                // ERROR: unterminated string literal
                reType = TT_ERROR;
                break;
            }
            c = getChar();
            if( testCharFlags( c, CHAR_EOL ) == sal_True )
            {
                // ERROR: unterminated string literal
                reType = TT_ERROR;
                break;
            }
        }

        if( reType != TT_ERROR )
        {
            getChar();
            if( cEndString == ']' )
                reType = TT_IDENTIFIER;
            else
                reType = TT_STRING;
        }
    }

    // End of line?
    else if( testCharFlags( c, CHAR_EOL ) == sal_True )
    {
        // If another EOL character comes, read it
        sal_Unicode cNext = peekChar();
        if( cNext != c && testCharFlags( cNext, CHAR_EOL ) == sal_True )
            getChar();

        // Set position data at the line start
        nCol = 0;
        nLine++;

        reType = TT_EOL;
    }

    // All other will remain TT_UNKNOWN

    // Save end position
    rpEndPos = mpActualPos;
    return sal_True;
}

SimpleTokenizer_Impl::SimpleTokenizer_Impl( HighlighterLanguage aLang ): aLanguage(aLang)
{
    memset( aCharTypeTab, 0, sizeof( aCharTypeTab ) );

    // Fill character table
    sal_uInt16 i;

    // Allowed characters for identifiers
    sal_uInt16 nHelpMask = (sal_uInt16)( CHAR_START_IDENTIFIER | CHAR_IN_IDENTIFIER );
    for( i = 'a' ; i <= 'z' ; i++ )
        aCharTypeTab[i] |= nHelpMask;
    for( i = 'A' ; i <= 'Z' ; i++ )
        aCharTypeTab[i] |= nHelpMask;
    aCharTypeTab[(int)'_'] |= nHelpMask;
    aCharTypeTab[(int)'$'] |= nHelpMask;

    // Digit (can be identifier and number)
    nHelpMask = (sal_uInt16)( CHAR_IN_IDENTIFIER | CHAR_START_NUMBER |
                         CHAR_IN_NUMBER | CHAR_IN_HEX_NUMBER );
    for( i = '0' ; i <= '9' ; i++ )
        aCharTypeTab[i] |= nHelpMask;

    // Add e, E, . and & here manually
    aCharTypeTab[(int)'e'] |= CHAR_IN_NUMBER;
    aCharTypeTab[(int)'E'] |= CHAR_IN_NUMBER;
    aCharTypeTab[(int)'.'] |= (sal_uInt16)( CHAR_IN_NUMBER | CHAR_START_NUMBER );
    aCharTypeTab[(int)'&'] |= CHAR_START_NUMBER;

    // Hexadecimal digit
    for( i = 'a' ; i <= 'f' ; i++ )
        aCharTypeTab[i] |= CHAR_IN_HEX_NUMBER;
    for( i = 'A' ; i <= 'F' ; i++ )
        aCharTypeTab[i] |= CHAR_IN_HEX_NUMBER;

    // Octal digit
    for( i = '0' ; i <= '7' ; i++ )
        aCharTypeTab[i] |= CHAR_IN_OCT_NUMBER;

    // String literal start/end characters
    aCharTypeTab[(int)'\''] |= CHAR_START_STRING;
    aCharTypeTab[(int)'\"'] |= CHAR_START_STRING;
    aCharTypeTab[(int)'[']  |= CHAR_START_STRING;
    aCharTypeTab[(int)'`']  |= CHAR_START_STRING;

    // Operator characters
    aCharTypeTab[(int)'!'] |= CHAR_OPERATOR;
    aCharTypeTab[(int)'%'] |= CHAR_OPERATOR;
    // aCharTypeTab[(int)'&'] |= CHAR_OPERATOR;     Removed because of #i14140
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
    // aCharTypeTab[(int)'['] |= CHAR_OPERATOR;     Removed because of #i17826
    aCharTypeTab[(int)']'] |= CHAR_OPERATOR;
    aCharTypeTab[(int)';'] |= CHAR_OPERATOR;

    // Space
    aCharTypeTab[(int)' ' ] |= CHAR_SPACE;
    aCharTypeTab[(int)'\t'] |= CHAR_SPACE;

    // End of line characters
    aCharTypeTab[(int)'\r'] |= CHAR_EOL;
    aCharTypeTab[(int)'\n'] |= CHAR_EOL;

    ppListKeyWords = NULL;
}

SimpleTokenizer_Impl::~SimpleTokenizer_Impl( void )
{
}

SimpleTokenizer_Impl* getSimpleTokenizer( void )
{
    static SimpleTokenizer_Impl* pSimpleTokenizer = NULL;
    if( !pSimpleTokenizer )
        pSimpleTokenizer = new SimpleTokenizer_Impl();
    return pSimpleTokenizer;
}

sal_uInt16 SimpleTokenizer_Impl::parseLine( sal_uInt32 nParseLine, const OUString* aSource )
{
    // Set the position to the beginning of the source string
    mpStringBegin = mpActualPos = aSource->getStr();

    // Initialize row and column
    nLine = nParseLine;
    nCol = 0L;

    // Variables for the out parameter
    TokenTypes eType;
    const sal_Unicode* pStartPos;
    const sal_Unicode* pEndPos;

    // Loop over all the tokens
    sal_uInt16 nTokenCount = 0;
    while( getNextToken( eType, pStartPos, pEndPos ) )
        nTokenCount++;

    return nTokenCount;
}

void SimpleTokenizer_Impl::getHighlightPortions( sal_uInt32 nParseLine, const OUString& rLine,
                                                    /*out*/HighlightPortions& portions  )
{
    // Set the position to the beginning of the source string
    mpStringBegin = mpActualPos = rLine.getStr();

    // Initialize row and column
    nLine = nParseLine;
    nCol = 0L;

    // Variables for the out parameter
    TokenTypes eType;
    const sal_Unicode* pStartPos;
    const sal_Unicode* pEndPos;

    // Loop over all the tokens
    while( getNextToken( eType, pStartPos, pEndPos ) )
    {
        HighlightPortion portion;

        portion.nBegin = (sal_uInt16)(pStartPos - mpStringBegin);
        portion.nEnd = (sal_uInt16)(pEndPos - mpStringBegin);
        portion.tokenType = eType;

        portions.push_back(portion);
    }
}


SyntaxHighlighter::SyntaxHighlighter()
{
    m_pSimpleTokenizer = 0;
    m_pKeyWords = NULL;
    m_nKeyWordCount = 0;
}

SyntaxHighlighter::~SyntaxHighlighter()
{
    delete m_pSimpleTokenizer;
    delete m_pKeyWords;
}

void SyntaxHighlighter::initialize( HighlighterLanguage eLanguage_ )
{
    eLanguage = eLanguage_;
    delete m_pSimpleTokenizer;
    m_pSimpleTokenizer = new SimpleTokenizer_Impl(eLanguage);

    switch (eLanguage)
    {
        case HIGHLIGHT_BASIC:
            m_pSimpleTokenizer->setKeyWords( strListBasicKeyWords,
                                            sizeof( strListBasicKeyWords ) / sizeof( char* ));
            break;
        case HIGHLIGHT_SQL:
            m_pSimpleTokenizer->setKeyWords( strListSqlKeyWords,
                                            sizeof( strListSqlKeyWords ) / sizeof( char* ));
            break;
        default:
            m_pSimpleTokenizer->setKeyWords( NULL, 0 );
    }
}

void SyntaxHighlighter::notifyChange( sal_uInt32 nLine, sal_Int32 nLineCountDifference,
                                const OUString* pChangedLines, sal_uInt32 nArrayLength)
{
    (void)nLineCountDifference;

    for( sal_uInt32 i=0 ; i < nArrayLength ; i++ )
        m_pSimpleTokenizer->parseLine(nLine+i, &pChangedLines[i]);
}

void SyntaxHighlighter::getHighlightPortions( sal_uInt32 nLine, const OUString& rLine,
                                            /*out*/HighlightPortions& portions )
{
    m_pSimpleTokenizer->getHighlightPortions( nLine, rLine, portions );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
