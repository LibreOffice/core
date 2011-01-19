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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"

#include <svtools/syntaxhighlight.hxx>

#include <unotools/charclass.hxx>
#include <tools/debug.hxx>


// ##########################################################################
// ATTENTION: all these words needs to be in small caps
// ##########################################################################
static const char* strListBasicKeyWords[] = {
    "access",
    "alias",
    "and",
    "any",
    "append",
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
    "wend",
    "while",
    "with",
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

class BasicSimpleCharClass
{
    static LetterTable aLetterTable;

public:
    static BOOL isAlpha( sal_Unicode c, bool bCompatible )
    {
        BOOL bRet = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
                    || (bCompatible && aLetterTable.isLetter( c ));
        return bRet;
    }

    static BOOL isDigit( sal_Unicode c )
    {
        BOOL bRet = (c >= '0' && c <= '9');
        return bRet;
    }

    static BOOL isAlphaNumeric( sal_Unicode c, bool bCompatible )
    {
        BOOL bRet = isDigit( c ) || isAlpha( c, bCompatible );
        return bRet;
    }
};

LetterTable BasicSimpleCharClass::aLetterTable;

LetterTable::LetterTable( void )
{
    for( int i = 0 ; i < 256 ; ++i )
        IsLetterTab[i] = false;

    IsLetterTab[0xC0] = true;   // À , CAPITAL LETTER A WITH GRAVE ACCENT
    IsLetterTab[0xC1] = true;   // Á , CAPITAL LETTER A WITH ACUTE ACCENT
    IsLetterTab[0xC2] = true;   // Â , CAPITAL LETTER A WITH CIRCUMFLEX ACCENT
    IsLetterTab[0xC3] = true;   // Ã , CAPITAL LETTER A WITH TILDE
    IsLetterTab[0xC4] = true;   // Ä , CAPITAL LETTER A WITH DIAERESIS
    IsLetterTab[0xC5] = true;   // Å , CAPITAL LETTER A WITH RING ABOVE
    IsLetterTab[0xC6] = true;   // Æ , CAPITAL LIGATURE AE
    IsLetterTab[0xC7] = true;   // Ç , CAPITAL LETTER C WITH CEDILLA
    IsLetterTab[0xC8] = true;   // È , CAPITAL LETTER E WITH GRAVE ACCENT
    IsLetterTab[0xC9] = true;   // É , CAPITAL LETTER E WITH ACUTE ACCENT
    IsLetterTab[0xCA] = true;   // Ê , CAPITAL LETTER E WITH CIRCUMFLEX ACCENT
    IsLetterTab[0xCB] = true;   // Ë , CAPITAL LETTER E WITH DIAERESIS
    IsLetterTab[0xCC] = true;   // Ì , CAPITAL LETTER I WITH GRAVE ACCENT
    IsLetterTab[0xCD] = true;   // Í , CAPITAL LETTER I WITH ACUTE ACCENT
    IsLetterTab[0xCE] = true;   // Î , CAPITAL LETTER I WITH CIRCUMFLEX ACCENT
    IsLetterTab[0xCF] = true;   // Ï , CAPITAL LETTER I WITH DIAERESIS
    IsLetterTab[0xD0] = true;   // Ð , CAPITAL LETTER ETH
    IsLetterTab[0xD1] = true;   // Ñ , CAPITAL LETTER N WITH TILDE
    IsLetterTab[0xD2] = true;   // Ò , CAPITAL LETTER O WITH GRAVE ACCENT
    IsLetterTab[0xD3] = true;   // Ó , CAPITAL LETTER O WITH ACUTE ACCENT
    IsLetterTab[0xD4] = true;   // Ô , CAPITAL LETTER O WITH CIRCUMFLEX ACCENT
    IsLetterTab[0xD5] = true;   // Õ , CAPITAL LETTER O WITH TILDE
    IsLetterTab[0xD6] = true;   // Ö , CAPITAL LETTER O WITH DIAERESIS
    IsLetterTab[0xD8] = true;   // Ø , CAPITAL LETTER O WITH STROKE
    IsLetterTab[0xD9] = true;   // Ù , CAPITAL LETTER U WITH GRAVE ACCENT
    IsLetterTab[0xDA] = true;   // Ú , CAPITAL LETTER U WITH ACUTE ACCENT
    IsLetterTab[0xDB] = true;   // Û , CAPITAL LETTER U WITH CIRCUMFLEX ACCENT
    IsLetterTab[0xDC] = true;   // Ü , CAPITAL LETTER U WITH DIAERESIS
    IsLetterTab[0xDD] = true;   // Ý , CAPITAL LETTER Y WITH ACUTE ACCENT
    IsLetterTab[0xDE] = true;   // Þ , CAPITAL LETTER THORN
    IsLetterTab[0xDF] = true;   // ß , SMALL LETTER SHARP S
    IsLetterTab[0xE0] = true;   // à , SMALL LETTER A WITH GRAVE ACCENT
    IsLetterTab[0xE1] = true;   // á , SMALL LETTER A WITH ACUTE ACCENT
    IsLetterTab[0xE2] = true;   // â , SMALL LETTER A WITH CIRCUMFLEX ACCENT
    IsLetterTab[0xE3] = true;   // ã , SMALL LETTER A WITH TILDE
    IsLetterTab[0xE4] = true;   // ä , SMALL LETTER A WITH DIAERESIS
    IsLetterTab[0xE5] = true;   // å , SMALL LETTER A WITH RING ABOVE
    IsLetterTab[0xE6] = true;   // æ , SMALL LIGATURE AE
    IsLetterTab[0xE7] = true;   // ç , SMALL LETTER C WITH CEDILLA
    IsLetterTab[0xE8] = true;   // è , SMALL LETTER E WITH GRAVE ACCENT
    IsLetterTab[0xE9] = true;   // é , SMALL LETTER E WITH ACUTE ACCENT
    IsLetterTab[0xEA] = true;   // ê , SMALL LETTER E WITH CIRCUMFLEX ACCENT
    IsLetterTab[0xEB] = true;   // ë , SMALL LETTER E WITH DIAERESIS
    IsLetterTab[0xEC] = true;   // ì , SMALL LETTER I WITH GRAVE ACCENT
    IsLetterTab[0xED] = true;   // í , SMALL LETTER I WITH ACUTE ACCENT
    IsLetterTab[0xEE] = true;   // î , SMALL LETTER I WITH CIRCUMFLEX ACCENT
    IsLetterTab[0xEF] = true;   // ï , SMALL LETTER I WITH DIAERESIS
    IsLetterTab[0xF0] = true;   // ð , SMALL LETTER ETH
    IsLetterTab[0xF1] = true;   // ñ , SMALL LETTER N WITH TILDE
    IsLetterTab[0xF2] = true;   // ò , SMALL LETTER O WITH GRAVE ACCENT
    IsLetterTab[0xF3] = true;   // ó , SMALL LETTER O WITH ACUTE ACCENT
    IsLetterTab[0xF4] = true;   // ô , SMALL LETTER O WITH CIRCUMFLEX ACCENT
    IsLetterTab[0xF5] = true;   // õ , SMALL LETTER O WITH TILDE
    IsLetterTab[0xF6] = true;   // ö , SMALL LETTER O WITH DIAERESIS
    IsLetterTab[0xF8] = true;   // ø , SMALL LETTER O WITH OBLIQUE BAR
    IsLetterTab[0xF9] = true;   // ù , SMALL LETTER U WITH GRAVE ACCENT
    IsLetterTab[0xFA] = true;   // ú , SMALL LETTER U WITH ACUTE ACCENT
    IsLetterTab[0xFB] = true;   // û , SMALL LETTER U WITH CIRCUMFLEX ACCENT
    IsLetterTab[0xFC] = true;   // ü , SMALL LETTER U WITH DIAERESIS
    IsLetterTab[0xFD] = true;   // ý , SMALL LETTER Y WITH ACUTE ACCENT
    IsLetterTab[0xFE] = true;   // þ , SMALL LETTER THORN
    IsLetterTab[0xFF] = true;   // ÿ , SMALL LETTER Y WITH DIAERESIS
}

bool LetterTable::isLetterUnicode( sal_Unicode c )
{
    static CharClass* pCharClass = NULL;
    if( pCharClass == NULL )
        pCharClass = new CharClass( Application::GetSettings().GetLocale() );
    String aStr( c );
    bool bRet = pCharClass->isLetter( aStr, 0 );
    return bRet;
}

// Hilfsfunktion: Zeichen-Flag Testen
BOOL SimpleTokenizer_Impl::testCharFlags( sal_Unicode c, USHORT nTestFlags )
{
    bool bRet = false;
    if( c != 0 && c <= 255 )
    {
        bRet = ( (aCharTypeTab[c] & nTestFlags) != 0 );
    }
    else if( c > 255 )
    {
        bRet = (( CHAR_START_IDENTIFIER | CHAR_IN_IDENTIFIER ) & nTestFlags) != 0
            ? BasicSimpleCharClass::isAlpha( c, true ) : false;
    }
    return bRet;
}

void SimpleTokenizer_Impl::setKeyWords( const char** ppKeyWords, UINT16 nCount )
{
    ppListKeyWords = ppKeyWords;
    nKeyWordCount = nCount;
}

// Neues Token holen
BOOL SimpleTokenizer_Impl::getNextToken( /*out*/TokenTypes& reType,
    /*out*/const sal_Unicode*& rpStartPos, /*out*/const sal_Unicode*& rpEndPos )
{
    reType = TT_UNKNOWN;

    // Position merken
    rpStartPos = mpActualPos;

    // Zeichen untersuchen
    sal_Unicode c = peekChar();
    if( c == CHAR_EOF )
        return FALSE;

    // Zeichen lesen
    getChar();

    //*** Alle Moeglichkeiten durchgehen ***
    // Space?
    if ( (testCharFlags( c, CHAR_SPACE ) == TRUE) )
    {
        while( testCharFlags( peekChar(), CHAR_SPACE ) == TRUE )
            getChar();

        reType = TT_WHITESPACE;
    }

    // Identifier?
    else if ( (testCharFlags( c, CHAR_START_IDENTIFIER ) == TRUE) )
    {
        BOOL bIdentifierChar;
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
                String aKWString(rpStartPos, sal::static_int_cast< xub_StrLen >(nCount) );
                ByteString aByteStr( aKWString, RTL_TEXTENCODING_ASCII_US );
                aByteStr.ToLowerAscii();
                if ( bsearch( aByteStr.GetBuffer(), ppListKeyWords, nKeyWordCount, sizeof( char* ),
                                                                        compare_strings ) )
                {
                    reType = TT_KEYWORDS;

                    if ( aByteStr.Equals( "rem" ) )
                    {
                        // Alle Zeichen bis Zeilen-Ende oder EOF entfernen
                        sal_Unicode cPeek = peekChar();
                        while( cPeek != CHAR_EOF && testCharFlags( cPeek, CHAR_EOL ) == FALSE )
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
    else if ( ( testCharFlags( c, CHAR_OPERATOR ) == TRUE ) || ( (c == '\'') && (aLanguage==HIGHLIGHT_BASIC)) )
    {
        // paramters for SQL view
        if ( (c==':') || (c=='?'))
        {
            if (c!='?')
            {
                BOOL bIdentifierChar;
                do
                {
                    // Naechstes Zeichen holen
                    c = peekChar();
                    bIdentifierChar =  BasicSimpleCharClass::isAlpha( c, true );
                    if( bIdentifierChar )
                        getChar();
                }
                while( bIdentifierChar );
            }
            reType = TT_PARAMETER;
        }
        else if ((c=='-'))
        {
            sal_Unicode cPeekNext = peekChar();
            if (cPeekNext=='-')
            {
                // Alle Zeichen bis Zeilen-Ende oder EOF entfernen
                while( cPeekNext != CHAR_EOF && testCharFlags( cPeekNext, CHAR_EOL ) == FALSE )
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
               while( cPeekNext != CHAR_EOF && testCharFlags( cPeekNext, CHAR_EOL ) == FALSE )
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
                while( cPeek != CHAR_EOF && testCharFlags( cPeek, CHAR_EOL ) == FALSE )
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
    else if( testCharFlags( c, CHAR_START_NUMBER ) == TRUE )
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
            BOOL bAfterExpChar = FALSE;

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
    else if( testCharFlags( c, CHAR_START_STRING ) == TRUE )
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
            if( testCharFlags( c, CHAR_EOL ) == TRUE )
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
    else if( testCharFlags( c, CHAR_EOL ) == TRUE )
    {
        // Falls ein weiteres anderes EOL-Char folgt, weg damit
        sal_Unicode cNext = peekChar();
        if( cNext != c && testCharFlags( cNext, CHAR_EOL ) == TRUE )
            getChar();

        // Positions-Daten auf Zeilen-Beginn setzen
        nCol = 0;
        nLine++;

        reType = TT_EOL;
    }

    // Alles andere bleibt TT_UNKNOWN


    // End-Position eintragen
    rpEndPos = mpActualPos;
    return TRUE;
}

String SimpleTokenizer_Impl::getTokStr
    ( /*out*/const sal_Unicode* pStartPos, /*out*/const sal_Unicode* pEndPos )
{
    return String( pStartPos, (USHORT)( pEndPos - pStartPos ) );
}

#ifdef DBG_UTIL
// TEST: Token ausgeben
String SimpleTokenizer_Impl::getFullTokenStr( /*out*/TokenTypes eType,
    /*out*/const sal_Unicode* pStartPos, /*out*/const sal_Unicode* pEndPos )
{
    String aOut;
    switch( eType )
    {
        case TT_UNKNOWN:    aOut = String( RTL_CONSTASCII_USTRINGPARAM("TT_UNKNOWN:") ); break;
        case TT_IDENTIFIER: aOut = String( RTL_CONSTASCII_USTRINGPARAM("TT_IDENTIFIER:") ); break;
        case TT_WHITESPACE: aOut = String( RTL_CONSTASCII_USTRINGPARAM("TT_WHITESPACE:") ); break;
        case TT_NUMBER:     aOut = String( RTL_CONSTASCII_USTRINGPARAM("TT_NUMBER:") ); break;
        case TT_STRING:     aOut = String( RTL_CONSTASCII_USTRINGPARAM("TT_STRING:") ); break;
        case TT_EOL:        aOut = String( RTL_CONSTASCII_USTRINGPARAM("TT_EOL:") ); break;
        case TT_COMMENT:    aOut = String( RTL_CONSTASCII_USTRINGPARAM("TT_COMMENT:") ); break;
        case TT_ERROR:      aOut = String( RTL_CONSTASCII_USTRINGPARAM("TT_ERROR:") ); break;
        case TT_OPERATOR:   aOut = String( RTL_CONSTASCII_USTRINGPARAM("TT_OPERATOR:") ); break;
        case TT_KEYWORDS:   aOut = String( RTL_CONSTASCII_USTRINGPARAM("TT_KEYWORD:") ); break;
        case TT_PARAMETER:  aOut = String( RTL_CONSTASCII_USTRINGPARAM("TT_PARAMETER:") ); break;
    }
    if( eType != TT_EOL )
    {
        aOut += String( pStartPos, (USHORT)( pEndPos - pStartPos ) );
    }
    aOut += String( RTL_CONSTASCII_USTRINGPARAM("\n") );
    return aOut;
}
#endif

SimpleTokenizer_Impl::SimpleTokenizer_Impl( HighlighterLanguage aLang ): aLanguage(aLang)
{
    memset( aCharTypeTab, 0, sizeof( aCharTypeTab ) );

    // Zeichen-Tabelle fuellen
    USHORT i;

    // Zulaessige Zeichen fuer Identifier
    USHORT nHelpMask = (USHORT)( CHAR_START_IDENTIFIER | CHAR_IN_IDENTIFIER );
    for( i = 'a' ; i <= 'z' ; i++ )
        aCharTypeTab[i] |= nHelpMask;
    for( i = 'A' ; i <= 'Z' ; i++ )
        aCharTypeTab[i] |= nHelpMask;
    // '_' extra eintragen
    aCharTypeTab[(int)'_'] |= nHelpMask;
    // AB 23.6.97: '$' ist auch erlaubt
    aCharTypeTab[(int)'$'] |= nHelpMask;

    // Ziffern (Identifier und Number ist moeglich)
    nHelpMask = (USHORT)( CHAR_IN_IDENTIFIER | CHAR_START_NUMBER |
                         CHAR_IN_NUMBER | CHAR_IN_HEX_NUMBER );
    for( i = '0' ; i <= '9' ; i++ )
        aCharTypeTab[i] |= nHelpMask;

    // e und E sowie . von Hand ergaenzen
    aCharTypeTab[(int)'e'] |= CHAR_IN_NUMBER;
    aCharTypeTab[(int)'E'] |= CHAR_IN_NUMBER;
    aCharTypeTab[(int)'.'] |= (USHORT)( CHAR_IN_NUMBER | CHAR_START_NUMBER );
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
UINT16 SimpleTokenizer_Impl::parseLine( UINT32 nParseLine, const String* aSource )
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
    UINT16 nTokenCount = 0;
    while( getNextToken( eType, pStartPos, pEndPos ) )
        nTokenCount++;

    return nTokenCount;
}

void SimpleTokenizer_Impl::getHighlightPortions( UINT32 nParseLine, const String& rLine,
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

        portion.nBegin = (UINT16)(pStartPos - mpStringBegin);
        portion.nEnd = (UINT16)(pEndPos - mpStringBegin);
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

const Range SyntaxHighlighter::notifyChange( UINT32 nLine, INT32 nLineCountDifference,
                                const String* pChangedLines, UINT32 nArrayLength)
{
    (void)nLineCountDifference;

    for( UINT32 i=0 ; i < nArrayLength ; i++ )
        m_pSimpleTokenizer->parseLine(nLine+i, &pChangedLines[i]);

    return Range( nLine, nLine + nArrayLength-1 );
}

void SyntaxHighlighter::getHighlightPortions( UINT32 nLine, const String& rLine,
                                            /*out*/HighlightPortions& portions )
{
    m_pSimpleTokenizer->getHighlightPortions( nLine, rLine, portions );
}
