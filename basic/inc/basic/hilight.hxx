/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: hilight.hxx,v $
 * $Revision: 1.3 $
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

#ifndef _SB_HILIGHT_HXX
#define _SB_HILIGHT_HXX

#include <tools/string.hxx>
#include <tools/gen.hxx>

#include <svtools/svarray.hxx>

//#include <sbxmod.cxx>
class SimpleTokenizer_Impl;

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
    TT_KEYWORD
};

struct HighlightPortion { UINT16 nBegin; UINT16 nEnd; TokenTypes tokenType; };

SV_DECL_VARARR(HighlightPortions, HighlightPortion, 0, 16)

// Sprachmodus des HighLighters (spaeter eventuell feiner
// differenzieren mit Keyword-Liste, C-Kommentar-Flag)
enum HighlighterLanguage
{
    HIGHLIGHT_BASIC
};

//*** SyntaxHighlighter-Klasse ***
// Konzept: Der Highlighter wird ueber alle Aenderungen im Source
// informiert (notifyChange) und liefert dem Aufrufer jeweils die
// Information zurueck, welcher Zeilen-Bereich des Source-Codes
// aufgrund dieser Aenderung neu gehighlighted werden muss.
// Dazu merkt sich Highlighter intern fuer jede Zeile, ob dort
// C-Kommentare beginnen oder enden.
class SyntaxHighlighter
{
    HighlighterLanguage eLanguage;
    SimpleTokenizer_Impl* m_pSimpleTokenizer;
    char* m_pKeyWords;
    UINT16 m_nKeyWordCount;

//  void initializeKeyWords( HighlighterLanguage eLanguage );

public:
    SyntaxHighlighter( void );
    ~SyntaxHighlighter( void );

    // HighLighter (neu) initialisieren, die Zeilen-Tabelle wird
    // dabei komplett geloescht, d.h. im Abschluss wird von einem
    // leeren Source ausgegangen. In notifyChange() kann dann
    // nur Zeile 0 angegeben werden.
    void initialize( HighlighterLanguage eLanguage_ );

    /**
     * Aenderung im Source anzeigen
     * @param nLine = Zeile, in der die Aenderung erfolgt, dies entspricht
     * der Zeile, in der im Editor der Cursor steht. Index der 1. Zeile ist 0.
     * @param nLineCountDifference = Anzahl der Zeilen, die im Rahmen der
     * Aenderung nach nLine eingefuegt (positiver Wert) oder entfernt
     * (negativer Wert) werden. 0, wenn von der Aenderung nur nLine
     * betroffen ist.
     * @param pChangedLines = Array der Zeilen, die von der Aenderung
     * betroffen sind (das Array enthaelt die geaenderten Zeilen)
     *
     * @return Zeilen-Bereich des SourceCodes, dessen Syntax-Higlighting
     * erneuert werden muss, z.B. weil ein C-Kommentar geoeffnet wurde.
     */
    const Range notifyChange( UINT32 nLine, INT32 nLineCountDifference,
                                const String* pChangedLines, UINT32 nArrayLength);

    /**
     * Higlighting durchfuehren
     * @param nLine = Zeile, fuer die die Highlight-Tokens geliefert
     * werden sollen. Index der 1. Zeile ist 0.
     * @param rLine = Zeile, fuer die die Highlight-Tokens geliefert
     * werden sollen. als String
     * @param Ein VarArray von HighlightPortion, in das die Start- und
     * die End-Indizes und die Typen der Token der Zeile eingetragen werden.
     */
    void getHighlightPortions( UINT32 nLine, const String& rLine,
                                            /*out*/HighlightPortions& pPortions );

};

#endif

