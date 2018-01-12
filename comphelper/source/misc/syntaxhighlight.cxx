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

#include <sal/config.h>

#include <cassert>

#include <rtl/character.hxx>
#include <unicode/uchar.h>
#include <comphelper/syntaxhighlight.hxx>
#include <o3tl/typed_flags_set.hxx>

// Flags for character properties
enum class CharFlags {
    StartIdentifier   = 0x0001,
    InIdentifier      = 0x0002,
    StartNumber       = 0x0004,
    InNumber          = 0x0008,
    InHexNumber       = 0x0010,
    InOctNumber       = 0x0020,
    StartString       = 0x0040,
    Operator          = 0x0080,
    Space             = 0x0100,
    EOL               = 0x0200
};
namespace o3tl {
    template<> struct typed_flags<CharFlags> : is_typed_flags<CharFlags, 0x03ff> {};
}

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
    "doevents",
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
    return strcmp( static_cast<char const *>(arg1), *static_cast<char * const *>(arg2) );
}


namespace
{
    bool isAlpha(sal_Unicode c)
    {
        if (rtl::isAsciiAlpha(c))
            return true;
        return u_isalpha(c);
    }
}

class SyntaxHighlighter::Tokenizer
{
    // Character information tables
    CharFlags aCharTypeTab[256];

    // Auxiliary function: testing of the character flags
    bool testCharFlags(sal_Unicode c, CharFlags nTestFlags) const;

    // Get new token, EmptyString == nothing more over there
    bool getNextToken(const sal_Unicode*& pos, /*out*/TokenType& reType,
        /*out*/const sal_Unicode*& rpStartPos, /*out*/const sal_Unicode*& rpEndPos) const;

    const char** ppListKeyWords;
    sal_uInt16 nKeyWordCount;

public:
    HighlighterLanguage const aLanguage;

    explicit Tokenizer( HighlighterLanguage aLang );

    void getHighlightPortions(const OUString& rLine,
                               /*out*/std::vector<HighlightPortion>& portions) const;
    void setKeyWords( const char** ppKeyWords, sal_uInt16 nCount );
};

// Helper function: test character flag
bool SyntaxHighlighter::Tokenizer::testCharFlags(sal_Unicode c, CharFlags nTestFlags) const
{
    bool bRet = false;
    if( c != 0 && c <= 255 )
    {
        bRet = bool(aCharTypeTab[c] & nTestFlags);
    }
    else if( c > 255 )
    {
        bRet = (( CharFlags::StartIdentifier | CharFlags::InIdentifier ) & nTestFlags)
            && isAlpha(c);
    }
    return bRet;
}

void SyntaxHighlighter::Tokenizer::setKeyWords( const char** ppKeyWords, sal_uInt16 nCount )
{
    ppListKeyWords = ppKeyWords;
    nKeyWordCount = nCount;
}

bool SyntaxHighlighter::Tokenizer::getNextToken(const sal_Unicode*& pos, /*out*/TokenType& reType,
    /*out*/const sal_Unicode*& rpStartPos, /*out*/const sal_Unicode*& rpEndPos) const
{
    reType = TokenType::Unknown;

    rpStartPos = pos;

    sal_Unicode c = *pos;
    if( c == 0 )
        return false;

    ++pos;

    //*** Go through all possibilities ***
    // Space?
    if ( testCharFlags( c, CharFlags::Space ) )
    {
        while( testCharFlags( *pos, CharFlags::Space ) )
            ++pos;

        reType = TokenType::Whitespace;
    }

    // Identifier?
    else if ( testCharFlags( c, CharFlags::StartIdentifier ) )
    {
        bool bIdentifierChar;
        do
        {
            // Fetch next character
            c = *pos;
            bIdentifierChar = testCharFlags( c, CharFlags::InIdentifier );
            if( bIdentifierChar )
                ++pos;
        }
        while( bIdentifierChar );

        reType = TokenType::Identifier;

        // Keyword table
        if (ppListKeyWords != nullptr)
        {
            int nCount = pos - rpStartPos;

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
                    reType = TokenType::Keywords;

                    if( aByteStr == "rem" )
                    {
                        // Remove all characters until end of line or EOF
                        sal_Unicode cPeek = *pos;
                        while( cPeek != 0 && !testCharFlags( cPeek, CharFlags::EOL ) )
                        {
                            cPeek = *++pos;
                        }

                        reType = TokenType::Comment;
                    }
                }
            }
        }
    }

    // Operator?
    // only for BASIC '\'' should be a comment, otherwise it is a normal string and handled there
    else if ( testCharFlags( c, CharFlags::Operator ) || ( (c == '\'') && (aLanguage==HighlighterLanguage::Basic)) )
    {
        // parameters for SQL view
        if (((c==':') || (c=='?')) && (aLanguage == HighlighterLanguage::SQL))
        {
            if (c!='?')
            {
                bool bIdentifierChar;
                do
                {
                    // Get next character
                    c = *pos;
                    bIdentifierChar = isAlpha(c);
                    if( bIdentifierChar )
                        ++pos;
                }
                while( bIdentifierChar );
            }
            reType = TokenType::Parameter;
        }
        else if ((c=='-') && (aLanguage == HighlighterLanguage::SQL))
        {
            sal_Unicode cPeekNext = *pos;
            if (cPeekNext=='-')
            {
                // Remove all characters until end of line or EOF
                while( cPeekNext != 0 && !testCharFlags( cPeekNext, CharFlags::EOL ) )
                {
                    ++pos;
                    cPeekNext = *pos;
                }
                reType = TokenType::Comment;
            }
        }
        else if ((c=='/') && (aLanguage == HighlighterLanguage::SQL))
        {
            sal_Unicode cPeekNext = *pos;
            if (cPeekNext=='/')
            {
                // Remove all characters until end of line or EOF
                while( cPeekNext != 0 && !testCharFlags( cPeekNext, CharFlags::EOL ) )
                {
                    ++pos;
                    cPeekNext = *pos;
                }
                reType = TokenType::Comment;
            }
        }
        else
        {
            // Apostrophe is Basic comment
            if (( c == '\'') && (aLanguage == HighlighterLanguage::Basic))
            {
                // Skip all characters until end of input or end of line:
                for (;;) {
                    c = *pos;
                    if (c == 0 || testCharFlags(c, CharFlags::EOL)) {
                        break;
                    }
                    ++pos;
                }

                reType = TokenType::Comment;
            }

            // The real operator; can be easily used since not the actual
            // operator (e.g. +=) is concerned, but the fact that it is one
            if( reType != TokenType::Comment )
            {
                reType = TokenType::Operator;
            }

        }
    }

    // Object separator? Must be handled before Number
    else if( c == '.' && ( *pos < '0' || *pos > '9' ) )
    {
        reType = TokenType::Operator;
    }

    // Number?
    else if( testCharFlags( c, CharFlags::StartNumber ) )
    {
        reType = TokenType::Number;

        // Number system, 10 = normal, it is changed for Oct/Hex
        int nRadix = 10;

        // Is it an Oct or a Hex number?
        if( c == '&' )
        {
            // Octal?
            if( *pos == 'o' || *pos == 'O' )
            {
                // remove o
                ++pos;
                nRadix = 8;     // Octal base

                // Read all numbers
                while( testCharFlags( *pos, CharFlags::InOctNumber ) )
                    ++pos;
            }
            // Hexadecimal?
            else if( *pos == 'h' || *pos == 'H' )
            {
                // remove x
                ++pos;
                nRadix = 16;     // Hexadecimal base

                // Read all numbers
                while( testCharFlags( *pos, CharFlags::InHexNumber ) )
                    ++pos;
            }
            else
            {
                reType = TokenType::Operator;
            }
        }

        // When it is not Oct or Hex, then it is double
        if( reType == TokenType::Number && nRadix == 10 )
        {
            // Flag if the last character is an exponent
            bool bAfterExpChar = false;

            // Read all numbers
            while( testCharFlags( *pos, CharFlags::InNumber ) ||
                    (bAfterExpChar && *pos == '+' ) ||
                    (bAfterExpChar && *pos == '-' ) )
                    // After exponent +/- are OK, too
            {
                c = *pos++;
                bAfterExpChar = ( c == 'e' || c == 'E' );
            }
        }
    }

    // String?
    else if( testCharFlags( c, CharFlags::StartString ) )
    {
        // Remember which character has opened the string
        sal_Unicode cEndString = c;
        if( c == '[' )
            cEndString = ']';

        // Read all characters
        while( *pos != cEndString )
        {
            // Detect EOF before reading next char, so we do not lose EOF
            if( *pos == 0 )
            {
                // ERROR: unterminated string literal
                reType = TokenType::Error;
                break;
            }
            c = *pos++;
            if( testCharFlags( c, CharFlags::EOL ) )
            {
                // ERROR: unterminated string literal
                reType = TokenType::Error;
                break;
            }
        }

        if( reType != TokenType::Error )
        {
            ++pos;
            if( cEndString == ']' )
                reType = TokenType::Identifier;
            else
                reType = TokenType::String;
        }
    }

    // End of line?
    else if( testCharFlags( c, CharFlags::EOL ) )
    {
        // If another EOL character comes, read it
        sal_Unicode cNext = *pos;
        if( cNext != c && testCharFlags( cNext, CharFlags::EOL ) )
            ++pos;

        reType = TokenType::EOL;
    }

    // All other will remain TokenType::Unknown

    // Save end position
    rpEndPos = pos;
    return true;
}

SyntaxHighlighter::Tokenizer::Tokenizer( HighlighterLanguage aLang ): aLanguage(aLang)
{
    memset( aCharTypeTab, 0, sizeof( aCharTypeTab ) );

    // Fill character table
    sal_uInt16 i;

    // Allowed characters for identifiers
    CharFlags nHelpMask = CharFlags::StartIdentifier | CharFlags::InIdentifier;
    for( i = 'a' ; i <= 'z' ; i++ )
        aCharTypeTab[i] |= nHelpMask;
    for( i = 'A' ; i <= 'Z' ; i++ )
        aCharTypeTab[i] |= nHelpMask;
    aCharTypeTab[int('_')] |= nHelpMask;
    aCharTypeTab[int('$')] |= nHelpMask;

    // Digit (can be identifier and number)
    nHelpMask = CharFlags::InIdentifier | CharFlags::StartNumber |
                         CharFlags::InNumber | CharFlags::InHexNumber;
    for( i = '0' ; i <= '9' ; i++ )
        aCharTypeTab[i] |= nHelpMask;

    // Add e, E, . and & here manually
    aCharTypeTab[int('e')] |= CharFlags::InNumber;
    aCharTypeTab[int('E')] |= CharFlags::InNumber;
    aCharTypeTab[int('.')] |= CharFlags::InNumber | CharFlags::StartNumber;
    aCharTypeTab[int('&')] |= CharFlags::StartNumber;

    // Hexadecimal digit
    for( i = 'a' ; i <= 'f' ; i++ )
        aCharTypeTab[i] |= CharFlags::InHexNumber;
    for( i = 'A' ; i <= 'F' ; i++ )
        aCharTypeTab[i] |= CharFlags::InHexNumber;

    // Octal digit
    for( i = '0' ; i <= '7' ; i++ )
        aCharTypeTab[i] |= CharFlags::InOctNumber;

    // String literal start/end characters
    aCharTypeTab[int('\'')] |= CharFlags::StartString;
    aCharTypeTab[int('\"')] |= CharFlags::StartString;
    aCharTypeTab[int('[')]  |= CharFlags::StartString;
    aCharTypeTab[int('`')]  |= CharFlags::StartString;

    // Operator characters
    aCharTypeTab[int('!')] |= CharFlags::Operator;
    aCharTypeTab[int('%')] |= CharFlags::Operator;
    // aCharTypeTab[(int)'&'] |= CharFlags::Operator;     Removed because of #i14140
    aCharTypeTab[int('(')] |= CharFlags::Operator;
    aCharTypeTab[int(')')] |= CharFlags::Operator;
    aCharTypeTab[int('*')] |= CharFlags::Operator;
    aCharTypeTab[int('+')] |= CharFlags::Operator;
    aCharTypeTab[int(',')] |= CharFlags::Operator;
    aCharTypeTab[int('-')] |= CharFlags::Operator;
    aCharTypeTab[int('/')] |= CharFlags::Operator;
    aCharTypeTab[int(':')] |= CharFlags::Operator;
    aCharTypeTab[int('<')] |= CharFlags::Operator;
    aCharTypeTab[int('=')] |= CharFlags::Operator;
    aCharTypeTab[int('>')] |= CharFlags::Operator;
    aCharTypeTab[int('?')] |= CharFlags::Operator;
    aCharTypeTab[int('^')] |= CharFlags::Operator;
    aCharTypeTab[int('|')] |= CharFlags::Operator;
    aCharTypeTab[int('~')] |= CharFlags::Operator;
    aCharTypeTab[int('{')] |= CharFlags::Operator;
    aCharTypeTab[int('}')] |= CharFlags::Operator;
    // aCharTypeTab[(int)'['] |= CharFlags::Operator;     Removed because of #i17826
    aCharTypeTab[int(']')] |= CharFlags::Operator;
    aCharTypeTab[int(';')] |= CharFlags::Operator;

    // Space
    aCharTypeTab[int(' ') ] |= CharFlags::Space;
    aCharTypeTab[int('\t')] |= CharFlags::Space;

    // End of line characters
    aCharTypeTab[int('\r')] |= CharFlags::EOL;
    aCharTypeTab[int('\n')] |= CharFlags::EOL;

    ppListKeyWords = nullptr;
    nKeyWordCount = 0;
}

void SyntaxHighlighter::Tokenizer::getHighlightPortions(const OUString& rLine,
                                                 /*out*/std::vector<HighlightPortion>& portions) const
{
    // Set the position to the beginning of the source string
    const sal_Unicode* pos = rLine.getStr();

    // Variables for the out parameter
    TokenType eType;
    const sal_Unicode* pStartPos;
    const sal_Unicode* pEndPos;

    // Loop over all the tokens
    while( getNextToken( pos, eType, pStartPos, pEndPos ) )
    {
        portions.emplace_back(
                pStartPos - rLine.getStr(), pEndPos - rLine.getStr(), eType);
    }
}


SyntaxHighlighter::SyntaxHighlighter(HighlighterLanguage language):
    eLanguage(language), m_tokenizer(new SyntaxHighlighter::Tokenizer(language))
{
    switch (eLanguage)
    {
        case HighlighterLanguage::Basic:
            m_tokenizer->setKeyWords( strListBasicKeyWords,
                                      SAL_N_ELEMENTS( strListBasicKeyWords ));
            break;
        case HighlighterLanguage::SQL:
            m_tokenizer->setKeyWords( strListSqlKeyWords,
                                      SAL_N_ELEMENTS( strListSqlKeyWords ));
            break;
        default:
            assert(false); // this cannot happen
    }
}

SyntaxHighlighter::~SyntaxHighlighter() {}

void SyntaxHighlighter::getHighlightPortions(const OUString& rLine,
                                              /*out*/std::vector<HighlightPortion>& portions) const
{
    m_tokenizer->getHighlightPortions( rLine, portions );
}

HighlighterLanguage SyntaxHighlighter::GetLanguage() const
{
    return m_tokenizer->aLanguage;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
