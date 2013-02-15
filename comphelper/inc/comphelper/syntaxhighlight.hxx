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
#ifndef _COMPHELPER_SYNTAXHIGHLIGHT_HXX
#define _COMPHELPER_SYNTAXHIGHLIGHT_HXX

#include <vector>
#include <rtl/ustring.hxx>

#include <comphelper/comphelperdllapi.h>


#if defined CDECL
#undef CDECL
#endif

// for the bsearch
#ifdef WNT
#define CDECL _cdecl
#endif
#if defined(UNX)
#define CDECL
#endif
#ifdef UNX
#include <sys/resource.h>
#endif


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

    const char** ppListKeyWords;
    sal_uInt16 nKeyWordCount;

public:
    SimpleTokenizer_Impl( HighlighterLanguage aLang = HIGHLIGHT_BASIC );
    ~SimpleTokenizer_Impl( void );

    sal_uInt16 parseLine( sal_uInt32 nLine, const OUString* aSource );
    void getHighlightPortions( sal_uInt32 nParseLine, const OUString& rLine,
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
class COMPHELPER_DLLPUBLIC SyntaxHighlighter
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

    void notifyChange( sal_uInt32 nLine, sal_Int32 nLineCountDifference,
                                const OUString* pChangedLines, sal_uInt32 nArrayLength);

    void getHighlightPortions( sal_uInt32 nLine, const OUString& rLine,
                                            HighlightPortions& pPortions );

    HighlighterLanguage GetLanguage() { return eLanguage;}
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
