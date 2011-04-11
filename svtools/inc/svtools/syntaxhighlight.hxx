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
#ifndef _SVTOOLS_SYNTAXHIGHLIGHT_HXX
#define _SVTOOLS_SYNTAXHIGHLIGHT_HXX

#include <list>

#include <osl/mutex.hxx>

#include <vcl/svapp.hxx>

#include <tools/stream.hxx>
#include <tools/shl.hxx>

#include <svl/brdcst.hxx>
#include <svtools/svtdllapi.h>


// for the bsearch
#ifdef WNT
#define CDECL _cdecl
#endif
#if defined(UNX) || defined(OS2)
#define CDECL
#endif
#ifdef UNX
#include <sys/resource.h>
#endif

#include <stdio.h>

#include <tools/string.hxx>
#include <tools/gen.hxx>


// Token-Typen TT_...
enum TokenTypes
{
    TT_UNKNOWN,
    TT_IDENTIFIER,
    TT_WHITESPACE,
    TT_NUMBER,
    TT_STRING,
    TT_EOL,
    TT_COMMENT,
    TT_ERROR,
    TT_OPERATOR,
    TT_KEYWORDS,
    TT_PARAMETER
};

struct HighlightPortion { sal_uInt16 nBegin; sal_uInt16 nEnd; TokenTypes tokenType; };


typedef std::vector<HighlightPortion> HighlightPortions;

/////////////////////////////////////////////////////////////////////////
// Hilfsklasse zur Untersuchung von JavaScript-Modulen, zunaechst zum
// Heraussuchen der Funktionen, spaeter auch zum Syntax-Highlighting verwenden

//  Flags fuer Zeichen-Eigenschaften
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

#define CHAR_EOF                0x00


// Sprachmodus des HighLighters (spaeter eventuell feiner
// differenzieren mit Keyword-Liste, C-Kommentar-Flag)
enum HighlighterLanguage
{
    HIGHLIGHT_BASIC,
    HIGHLIGHT_SQL
};

class SimpleTokenizer_Impl
{
    HighlighterLanguage aLanguage;
    // Zeichen-Info-Tabelle
    sal_uInt16 aCharTypeTab[256];

    const sal_Unicode* mpStringBegin;
    const sal_Unicode* mpActualPos;

    // Zeile und Spalte
    sal_uInt32 nLine;
    sal_uInt32 nCol;

    sal_Unicode peekChar( void )    { return *mpActualPos; }
    sal_Unicode getChar( void )     { nCol++; return *mpActualPos++; }

    // Hilfsfunktion: Zeichen-Flag Testen
    sal_Bool testCharFlags( sal_Unicode c, sal_uInt16 nTestFlags );

    // Neues Token holen, Leerstring == nix mehr da
    sal_Bool getNextToken( /*out*/TokenTypes& reType,
        /*out*/const sal_Unicode*& rpStartPos, /*out*/const sal_Unicode*& rpEndPos );

    String getTokStr( /*out*/const sal_Unicode* pStartPos, /*out*/const sal_Unicode* pEndPos );

#ifdef DBG_UTIL
    // TEST: Token ausgeben
    String getFullTokenStr( /*out*/TokenTypes eType,
        /*out*/const sal_Unicode* pStartPos, /*out*/const sal_Unicode* pEndPos );
#endif

    const char** ppListKeyWords;
    sal_uInt16 nKeyWordCount;

public:
    SimpleTokenizer_Impl( HighlighterLanguage aLang = HIGHLIGHT_BASIC );
    ~SimpleTokenizer_Impl( void );

    sal_uInt16 parseLine( sal_uInt32 nLine, const String* aSource );
    void getHighlightPortions( sal_uInt32 nParseLine, const String& rLine,
                                                    /*out*/HighlightPortions& portions );
    void setKeyWords( const char** ppKeyWords, sal_uInt16 nCount );
};


//*** SyntaxHighlighter-Klasse ***
// Konzept: Der Highlighter wird ueber alle Aenderungen im Source
// informiert (notifyChange) und liefert dem Aufrufer jeweils die
// Information zurueck, welcher Zeilen-Bereich des Source-Codes
// aufgrund dieser Aenderung neu gehighlighted werden muss.
// Dazu merkt sich Highlighter intern fuer jede Zeile, ob dort
// C-Kommentare beginnen oder enden.
class SVT_DLLPUBLIC SyntaxHighlighter
{
    HighlighterLanguage eLanguage;
    SimpleTokenizer_Impl* m_pSimpleTokenizer;
    char* m_pKeyWords;
    sal_uInt16 m_nKeyWordCount;

//  void initializeKeyWords( HighlighterLanguage eLanguage );

public:
    SyntaxHighlighter( void );
    ~SyntaxHighlighter( void );

    // HighLighter (neu) initialisieren, die Zeilen-Tabelle wird
    // dabei komplett geloescht, d.h. im Abschluss wird von einem
    // leeren Source ausgegangen. In notifyChange() kann dann
    // nur Zeile 0 angegeben werden.
    void initialize( HighlighterLanguage eLanguage_ );

    const Range notifyChange( sal_uInt32 nLine, sal_Int32 nLineCountDifference,
                                const String* pChangedLines, sal_uInt32 nArrayLength);

    void getHighlightPortions( sal_uInt32 nLine, const String& rLine,
                                            HighlightPortions& pPortions );

    HighlighterLanguage GetLanguage() { return eLanguage;}
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
