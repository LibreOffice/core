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


#include <svtools/syntaxhighlight.hxx>

#include <unotools/charclass.hxx>
#include <comphelper/string.hxx>

// ##########################################################################
// ATTENTION: all these words needs to be in small caps
// ##########################################################################
static const char* strListBasicKeyWords[] = {
    "access",
    "alias",
    "and",
    "any",
    "append",
    "attrubute",
    "as",
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
    "vbasupport",
    "variant",
    "wend",
    "while",
    "with",
    "withevent",
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

    class LetterTable
    {
        bool        IsLetterTab[256];

    public:
        LetterTable( void );

        inline bool isLetter( sal_Unicode c )
        {
            bool bRet = (c < 256) ? IsLetterTab[c] : isLetterUnicode( c );
            return bRet;
        }
        bool isLetterUnicode( sal_Unicode c );
    };

    static bool isAlpha(sal_Unicode c)
    {
        if (comphelper::string::isalphaAscii(c))
            return true;
        static LetterTable aLetterTable;
        return aLetterTable.isLetter(c);
    }
}

LetterTable::LetterTable( void )
{
    for( int i = 0 ; i < 256 ; ++i )
        IsLetterTab[i] = false;

    IsLetterTab[0xC0] = true;   // ?, CAPITAL LETTER A WITH GRAVE ACCENT
    IsLetterTab[0xC1] = true;   // ?, CAPITAL LETTER A WITH ACUTE ACCENT
    IsLetterTab[0xC2] = true;   // ?, CAPITAL LETTER A WITH CIRCUMFLEX ACCENT
    IsLetterTab[0xC3] = true;   // ?, CAPITAL LETTER A WITH TILDE
    IsLetterTab[0xC4] = true;   // ?, CAPITAL LETTER A WITH DIAERESIS
    IsLetterTab[0xC5] = true;   // ?, CAPITAL LETTER A WITH RING ABOVE
    IsLetterTab[0xC6] = true;   // ?, CAPITAL LIGATURE AE
    IsLetterTab[0xC7] = true;   // ?, CAPITAL LETTER C WITH CEDILLA
    IsLetterTab[0xC8] = true;   // ?, CAPITAL LETTER E WITH GRAVE ACCENT
    IsLetterTab[0xC9] = true;   // ?, CAPITAL LETTER E WITH ACUTE ACCENT
    IsLetterTab[0xCA] = true;   // ?, CAPITAL LETTER E WITH CIRCUMFLEX ACCENT
    IsLetterTab[0xCB] = true;   // ?, CAPITAL LETTER E WITH DIAERESIS
    IsLetterTab[0xCC] = true;   // ?, CAPITAL LETTER I WITH GRAVE ACCENT
    IsLetterTab[0xCD] = true;   // ?, CAPITAL LETTER I WITH ACUTE ACCENT
    IsLetterTab[0xCE] = true;   // ?, CAPITAL LETTER I WITH CIRCUMFLEX ACCENT
    IsLetterTab[0xCF] = true;   // ?, CAPITAL LETTER I WITH DIAERESIS
    IsLetterTab[0xD0] = true;   // ?, CAPITAL LETTER ETH
    IsLetterTab[0xD1] = true;   // ?, CAPITAL LETTER N WITH TILDE
    IsLetterTab[0xD2] = true;   // ?, CAPITAL LETTER O WITH GRAVE ACCENT
    IsLetterTab[0xD3] = true;   // ?, CAPITAL LETTER O WITH ACUTE ACCENT
    IsLetterTab[0xD4] = true;   // ?, CAPITAL LETTER O WITH CIRCUMFLEX ACCENT
    IsLetterTab[0xD5] = true;   // ?, CAPITAL LETTER O WITH TILDE
    IsLetterTab[0xD6] = true;   // ?, CAPITAL LETTER O WITH DIAERESIS
    IsLetterTab[0xD8] = true;   // ?, CAPITAL LETTER O WITH STROKE
    IsLetterTab[0xD9] = true;   // ?, CAPITAL LETTER U WITH GRAVE ACCENT
    IsLetterTab[0xDA] = true;   // ?, CAPITAL LETTER U WITH ACUTE ACCENT
    IsLetterTab[0xDB] = true;   // ?, CAPITAL LETTER U WITH CIRCUMFLEX ACCENT
    IsLetterTab[0xDC] = true;   // ?, CAPITAL LETTER U WITH DIAERESIS
    IsLetterTab[0xDD] = true;   // ?, CAPITAL LETTER Y WITH ACUTE ACCENT
    IsLetterTab[0xDE] = true;   // ?, CAPITAL LETTER THORN
    IsLetterTab[0xDF] = true;   // ?, SMALL LETTER SHARP S
    IsLetterTab[0xE0] = true;   // ?, SMALL LETTER A WITH GRAVE ACCENT
    IsLetterTab[0xE1] = true;   // ?, SMALL LETTER A WITH ACUTE ACCENT
    IsLetterTab[0xE2] = true;   // ?, SMALL LETTER A WITH CIRCUMFLEX ACCENT
    IsLetterTab[0xE3] = true;   // ?, SMALL LETTER A WITH TILDE
    IsLetterTab[0xE4] = true;   // ?, SMALL LETTER A WITH DIAERESIS
    IsLetterTab[0xE5] = true;   // ?, SMALL LETTER A WITH RING ABOVE
    IsLetterTab[0xE6] = true;   // ?, SMALL LIGATURE AE
    IsLetterTab[0xE7] = true;   // ?, SMALL LETTER C WITH CEDILLA
    IsLetterTab[0xE8] = true;   // ?, SMALL LETTER E WITH GRAVE ACCENT
    IsLetterTab[0xE9] = true;   // ?, SMALL LETTER E WITH ACUTE ACCENT
    IsLetterTab[0xEA] = true;   // ?, SMALL LETTER E WITH CIRCUMFLEX ACCENT
    IsLetterTab[0xEB] = true;   // ?, SMALL LETTER E WITH DIAERESIS
    IsLetterTab[0xEC] = true;   // ?, SMALL LETTER I WITH GRAVE ACCENT
    IsLetterTab[0xED] = true;   // ?, SMALL LETTER I WITH ACUTE ACCENT
    IsLetterTab[0xEE] = true;   // ?, SMALL LETTER I WITH CIRCUMFLEX ACCENT
    IsLetterTab[0xEF] = true;   // ?, SMALL LETTER I WITH DIAERESIS
    IsLetterTab[0xF0] = true;   // ?, SMALL LETTER ETH
    IsLetterTab[0xF1] = true;   // ?, SMALL LETTER N WITH TILDE
    IsLetterTab[0xF2] = true;   // ?, SMALL LETTER O WITH GRAVE ACCENT
    IsLetterTab[0xF3] = true;   // ?, SMALL LETTER O WITH ACUTE ACCENT
    IsLetterTab[0xF4] = true;   // ?, SMALL LETTER O WITH CIRCUMFLEX ACCENT
    IsLetterTab[0xF5] = true;   // ?, SMALL LETTER O WITH TILDE
    IsLetterTab[0xF6] = true;   // ?, SMALL LETTER O WITH DIAERESIS
    IsLetterTab[0xF8] = true;   // ?, SMALL LETTER O WITH OBLIQUE BAR
    IsLetterTab[0xF9] = true;   // ?, SMALL LETTER U WITH GRAVE ACCENT
    IsLetterTab[0xFA] = true;   // ?, SMALL LETTER U WITH ACUTE ACCENT
    IsLetterTab[0xFB] = true;   // ?, SMALL LETTER U WITH CIRCUMFLEX ACCENT
    IsLetterTab[0xFC] = true;   // ?, SMALL LETTER U WITH DIAERESIS
    IsLetterTab[0xFD] = true;   // ?, SMALL LETTER Y WITH ACUTE ACCENT
    IsLetterTab[0xFE] = true;   // ?, SMALL LETTER THORN
    IsLetterTab[0xFF] = true;   // � , SMALL LETTER Y WITH DIAERESIS
}

bool LetterTable::isLetterUnicode( sal_Unicode c )
{
    static CharClass* pCharClass = NULL;
    if( pCharClass == NULL )
        pCharClass = new CharClass( Application::GetSettings().GetLocale() );
    rtl::OUString aStr( c );
    bool bRet = pCharClass->isLetter( aStr, 0 );
    return bRet;
}

// Hilfsfunktion: Zeichen-Flag Testen
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

// Neues Token holen
sal_Bool SimpleTokenizer_Impl::getNextToken( /*out*/TokenTypes& reType,
    /*out*/const sal_Unicode*& rpStartPos, /*out*/const sal_Unicode*& rpEndPos )
{
    reType = TT_UNKNOWN;

    // Position merken
    rpStartPos = mpActualPos;

    // Zeichen untersuchen
    sal_Unicode c = peekChar();
    if( c == CHAR_EOF )
        return sal_False;

    // Zeichen lesen
    getChar();

    //*** Alle Moeglichkeiten durchgehen ***
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

        // Schluesselwort-Tabelle
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
                rtl::OUString aKWString(rpStartPos, nCount);
                rtl::OString aByteStr = rtl::OUStringToOString(aKWString,
                    RTL_TEXTENCODING_ASCII_US).toAsciiLowerCase();
                if ( bsearch( aByteStr.getStr(), ppListKeyWords, nKeyWordCount, sizeof( char* ),
                                                                        compare_strings ) )
                {
                    reType = TT_KEYWORDS;

                    if (aByteStr.equalsL(RTL_CONSTASCII_STRINGPARAM("rem")))
                    {
                        // Alle Zeichen bis Zeilen-Ende oder EOF entfernen
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
                    // Naechstes Zeichen holen
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
                // Alle Zeichen bis Zeilen-Ende oder EOF entfernen
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
               // Alle Zeichen bis Zeilen-Ende oder EOF entfernen
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
            // Kommentar ?
            if ( c == '\'' )
            {
                c = getChar();  // '/' entfernen

                // Alle Zeichen bis Zeilen-Ende oder EOF entfernen
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

    // Zahl?
    else if( testCharFlags( c, CHAR_START_NUMBER ) == sal_True )
    {
        reType = TT_NUMBER;

        // Zahlensystem, 10 = normal, wird bei Oct/Hex geaendert
        int nRadix = 10;

        // Ist es eine Hex- oder Oct-Zahl?
        if( c == '&' )
        {
            // Octal?
            if( peekChar() == 'o' || peekChar() == 'O' )
            {
                // o entfernen
                getChar();
                nRadix = 8;     // Octal-Basis

                // Alle Ziffern einlesen
                while( testCharFlags( peekChar(), CHAR_IN_OCT_NUMBER ) )
                    c = getChar();
            }
            // Hex?
            else if( peekChar() == 'h' || peekChar() == 'H' )
            {
                // x entfernen
                getChar();
                nRadix = 16;     // Hex-Basis

                // Alle Ziffern einlesen und puffern
                while( testCharFlags( peekChar(), CHAR_IN_HEX_NUMBER ) )
                    c = getChar();
            }
            else
            {
                reType = TT_OPERATOR;
            }
        }

        // Wenn nicht Oct oder Hex als double ansehen
        if( reType == TT_NUMBER && nRadix == 10 )
        {
            // Flag, ob das letzte Zeichen ein Exponent war
            sal_Bool bAfterExpChar = sal_False;

            // Alle Ziffern einlesen
            while( testCharFlags( peekChar(), CHAR_IN_NUMBER ) ||
                    (bAfterExpChar && peekChar() == '+' ) ||
                    (bAfterExpChar && peekChar() == '-' ) )
                    // Nach Exponent auch +/- OK
            {
                c = getChar();                  // Zeichen lesen
                bAfterExpChar = ( c == 'e' || c == 'E' );
            }
        }

        // reType = TT_NUMBER;
    }

    // String?
    else if( testCharFlags( c, CHAR_START_STRING ) == sal_True )
    {
        // Merken, welches Zeichen den String eroeffnet hat
        sal_Unicode cEndString = c;
        if( c == '[' )
            cEndString = ']';

        // Alle Ziffern einlesen und puffern
        while( peekChar() != cEndString )
        {
            // #58846 EOF vor getChar() abfangen, damit EOF micht verloren geht
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

        //  Zeichen lesen
        if( reType != TT_ERROR )
        {
            getChar();
            if( cEndString == ']' )
                reType = TT_IDENTIFIER;
            else
                reType = TT_STRING;
        }
    }

    // Zeilenende?
    else if( testCharFlags( c, CHAR_EOL ) == sal_True )
    {
        // Falls ein weiteres anderes EOL-Char folgt, weg damit
        sal_Unicode cNext = peekChar();
        if( cNext != c && testCharFlags( cNext, CHAR_EOL ) == sal_True )
            getChar();

        // Positions-Daten auf Zeilen-Beginn setzen
        nCol = 0;
        nLine++;

        reType = TT_EOL;
    }

    // Alles andere bleibt TT_UNKNOWN


    // End-Position eintragen
    rpEndPos = mpActualPos;
    return sal_True;
}

SimpleTokenizer_Impl::SimpleTokenizer_Impl( HighlighterLanguage aLang ): aLanguage(aLang)
{
    memset( aCharTypeTab, 0, sizeof( aCharTypeTab ) );

    // Zeichen-Tabelle fuellen
    sal_uInt16 i;

    // Zulaessige Zeichen fuer Identifier
    sal_uInt16 nHelpMask = (sal_uInt16)( CHAR_START_IDENTIFIER | CHAR_IN_IDENTIFIER );
    for( i = 'a' ; i <= 'z' ; i++ )
        aCharTypeTab[i] |= nHelpMask;
    for( i = 'A' ; i <= 'Z' ; i++ )
        aCharTypeTab[i] |= nHelpMask;
    // '_' extra eintragen
    aCharTypeTab[(int)'_'] |= nHelpMask;
    // AB 23.6.97: '$' ist auch erlaubt
    aCharTypeTab[(int)'$'] |= nHelpMask;

    // Ziffern (Identifier und Number ist moeglich)
    nHelpMask = (sal_uInt16)( CHAR_IN_IDENTIFIER | CHAR_START_NUMBER |
                         CHAR_IN_NUMBER | CHAR_IN_HEX_NUMBER );
    for( i = '0' ; i <= '9' ; i++ )
        aCharTypeTab[i] |= nHelpMask;

    // e und E sowie . von Hand ergaenzen
    aCharTypeTab[(int)'e'] |= CHAR_IN_NUMBER;
    aCharTypeTab[(int)'E'] |= CHAR_IN_NUMBER;
    aCharTypeTab[(int)'.'] |= (sal_uInt16)( CHAR_IN_NUMBER | CHAR_START_NUMBER );
    aCharTypeTab[(int)'&'] |= CHAR_START_NUMBER;

    // Hex-Ziffern
    for( i = 'a' ; i <= 'f' ; i++ )
        aCharTypeTab[i] |= CHAR_IN_HEX_NUMBER;
    for( i = 'A' ; i <= 'F' ; i++ )
        aCharTypeTab[i] |= CHAR_IN_HEX_NUMBER;

    // Oct-Ziffern
    for( i = '0' ; i <= '7' ; i++ )
        aCharTypeTab[i] |= CHAR_IN_OCT_NUMBER;

    // String-Beginn/End-Zeichen
    aCharTypeTab[(int)'\''] |= CHAR_START_STRING;
    aCharTypeTab[(int)'\"'] |= CHAR_START_STRING;
    aCharTypeTab[(int)'[']  |= CHAR_START_STRING;
    aCharTypeTab[(int)'`']  |= CHAR_START_STRING;

    // Operator-Zeichen
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

    // Zeilen-Ende-Zeichen
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

// Heraussuchen der jeweils naechsten Funktion aus einem JavaScript-Modul
sal_uInt16 SimpleTokenizer_Impl::parseLine( sal_uInt32 nParseLine, const String* aSource )
{
    // Position auf den Anfang des Source-Strings setzen
    mpStringBegin = mpActualPos = aSource->GetBuffer();

    // Zeile und Spalte initialisieren
    nLine = nParseLine;
    nCol = 0L;

    // Variablen fuer die Out-Parameter
    TokenTypes eType;
    const sal_Unicode* pStartPos;
    const sal_Unicode* pEndPos;

    // Schleife ueber alle Tokens
    sal_uInt16 nTokenCount = 0;
    while( getNextToken( eType, pStartPos, pEndPos ) )
        nTokenCount++;

    return nTokenCount;
}

void SimpleTokenizer_Impl::getHighlightPortions( sal_uInt32 nParseLine, const String& rLine,
                                                    /*out*/HighlightPortions& portions  )
{
    // Position auf den Anfang des Source-Strings setzen
    mpStringBegin = mpActualPos = rLine.GetBuffer();

    // Zeile und Spalte initialisieren
    nLine = nParseLine;
    nCol = 0L;

    // Variablen fuer die Out-Parameter
    TokenTypes eType;
    const sal_Unicode* pStartPos;
    const sal_Unicode* pEndPos;

    // Schleife ueber alle Tokens
    while( getNextToken( eType, pStartPos, pEndPos ) )
    {
        HighlightPortion portion;

        portion.nBegin = (sal_uInt16)(pStartPos - mpStringBegin);
        portion.nEnd = (sal_uInt16)(pEndPos - mpStringBegin);
        portion.tokenType = eType;

        portions.push_back(portion);
    }
}


//////////////////////////////////////////////////////////////////////////
// Implementierung des SyntaxHighlighter

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

const Range SyntaxHighlighter::notifyChange( sal_uInt32 nLine, sal_Int32 nLineCountDifference,
                                const String* pChangedLines, sal_uInt32 nArrayLength)
{
    (void)nLineCountDifference;

    for( sal_uInt32 i=0 ; i < nArrayLength ; i++ )
        m_pSimpleTokenizer->parseLine(nLine+i, &pChangedLines[i]);

    return Range( nLine, nLine + nArrayLength-1 );
}

void SyntaxHighlighter::getHighlightPortions( sal_uInt32 nLine, const String& rLine,
                                            /*out*/HighlightPortions& portions )
{
    m_pSimpleTokenizer->getHighlightPortions( nLine, rLine, portions );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
