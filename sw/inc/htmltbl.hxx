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

#ifndef _HTMLTBL_HXX
#define _HTMLTBL_HXX


#include <vcl/timer.hxx>
#include <editeng/svxenum.hxx>

#include "swtypes.hxx"
#include "node.hxx"     // Fuer SwStartNode


class SwTableBox;
class SwTable;
class SwHTMLTableLayout;
class SwDoc;
class SwFrmFmt;

#define HTMLTABLE_RESIZE_NOW (ULONG_MAX)

class SwHTMLTableLayoutCnts
{
    SwHTMLTableLayoutCnts *pNext;   // der naechste Inhalt

    // von den beiden naechsten Pointern darf nur einer gesetzt sein!
    SwTableBox *pBox;           // ein Box
    SwHTMLTableLayout *pTable;  // eine "Tabelle in der Tabelle"

    // Beim ersten Durchlauf gibt es noch keine Boxen. Es wird dann
    // pStartNode anstelle von pBox verwendet.
    const SwStartNode *pStartNode;

    // Die folgenden Zahler geben an, wie oft ein Pass bereits fuer diesen
    // Inhalt durchgefuehrt wurde. Dazu werden sie mit einer Soll-Vorgabe
    // verglichen. Wird 255 erreicht laufen sie bei 0 weiter. So wird
    // eine Reinitialisierung bei jedem Resize vermieden.
    sal_uInt8 nPass1Done;           // Wieoft wurde Pass 1 aufgerufen?
    sal_uInt8 nWidthSet;                // Wieoft wurde die Breite gesetzt?

    sal_Bool bNoBreakTag;       // <NOBR>-Tag ueber gesamten Inhalt

public:

    SwHTMLTableLayoutCnts( const SwStartNode* pSttNd, SwHTMLTableLayout* pTab,
                           sal_Bool bNoBreakTag, SwHTMLTableLayoutCnts* pNxt );

    ~SwHTMLTableLayoutCnts();

    void SetTableBox( SwTableBox *pBx ) { pBox = pBx; }
    SwTableBox *GetTableBox() const { return pBox; }

    SwHTMLTableLayout *GetTable() const { return pTable; }

    const SwStartNode *GetStartNode() const;

    // Ermitteln des naechsten Knotens
    SwHTMLTableLayoutCnts *GetNext() const { return pNext; }

    void SetWidthSet( sal_uInt8 nRef ) { nWidthSet = nRef; }
    sal_Bool IsWidthSet( sal_uInt8 nRef ) const { return nRef==nWidthSet; }

    void SetPass1Done( sal_uInt8 nRef ) { nPass1Done = nRef; }
    sal_Bool IsPass1Done( sal_uInt8 nRef ) const { return nRef==nPass1Done; }

    sal_Bool HasNoBreakTag() const { return bNoBreakTag; }
};

/*  */

class SwHTMLTableLayoutCell
{
    SwHTMLTableLayoutCnts *pContents;       // der Inhalt der Zelle

    sal_uInt16 nRowSpan;    // ROWSPAN der Zelle
    sal_uInt16 nColSpan;    // COLSPAN der Zelle
    sal_uInt16 nWidthOption;// angegebene Breite der Zelle in Twip oder %

    sal_Bool bPrcWidthOption : 1;// nWidth ist %-Angabe
    sal_Bool bNoWrapOption : 1; // NOWRAP-Option

public:

    SwHTMLTableLayoutCell( SwHTMLTableLayoutCnts *pCnts,
                         sal_uInt16 nRSpan, sal_uInt16 nCSpan,
                         sal_uInt16 nWidthOpt, sal_Bool bPrcWdthOpt,
                         sal_Bool nNWrapOpt );

    ~SwHTMLTableLayoutCell();

    // Setzen/Ermitteln des Inhalts einer Zelle
    void SetContents( SwHTMLTableLayoutCnts *pCnts ) { pContents = pCnts; }
    SwHTMLTableLayoutCnts *GetContents() const { return pContents; }

    inline void SetProtected();

    // ROWSPAN/COLSPAN der Zelle Setzen/Ermitteln
    void SetRowSpan( sal_uInt16 nRSpan ) { nRowSpan = nRSpan; }
    sal_uInt16 GetRowSpan() const { return nRowSpan; }
    sal_uInt16 GetColSpan() const { return nColSpan; }

    sal_uInt16 GetWidthOption() const { return nWidthOption; }
    sal_Bool IsPrcWidthOption() const { return bPrcWidthOption; }

    sal_Bool HasNoWrapOption() const { return bNoWrapOption; }
};

/*  */

class SwHTMLTableLayoutColumn
{
    // Zwischenwerte von AutoLayoutPass1
    sal_uLong nMinNoAlign, nMaxNoAlign, nAbsMinNoAlign;

    // Ergebnisse von AutoLayoutPass1
    sal_uLong nMin, nMax;

    // Ergibnisse von Pass 2
    sal_uInt16 nAbsColWidth;                // in Twips
    sal_uInt16 nRelColWidth;                // in Twips bzw. relativ zu USHRT_MAX

    sal_uInt16 nWidthOption;                // Optionen von <COL> oder <TD>/<TH>

    sal_Bool bRelWidthOption : 1;
    sal_Bool bLeftBorder : 1;

public:

    SwHTMLTableLayoutColumn( sal_uInt16 nColWidthOpt, sal_Bool bRelColWidthOpt,
                             sal_Bool bLBorder );

    ~SwHTMLTableLayoutColumn() {}

    inline void MergeCellWidthOption( sal_uInt16 nWidth, sal_Bool bPrc );
    inline void SetWidthOption( sal_uInt16 nWidth, sal_Bool bRelWidth, sal_Bool bTest );

    sal_uInt16 GetWidthOption() const { return nWidthOption; }
    sal_Bool IsRelWidthOption() const { return bRelWidthOption; }

    inline void MergeMinMaxNoAlign( sal_uLong nMin, sal_uLong nMax, sal_uLong nAbsMin );
    sal_uLong GetMinNoAlign() const { return nMinNoAlign; }
    sal_uLong GetMaxNoAlign() const { return nMaxNoAlign; }
    sal_uLong GetAbsMinNoAlign() const { return nAbsMinNoAlign; }
    inline void ClearPass1Info( sal_Bool bWidthOpt );

    inline void SetMinMax( sal_uLong nMin, sal_uLong nMax );
    void SetMax( sal_uLong nVal ) { nMax = nVal; }
    void AddToMin( sal_uLong nVal ) { nMin += nVal; }
    void AddToMax( sal_uLong nVal ) { nMax += nVal; }
    sal_uLong GetMin() const { return nMin; }
    sal_uLong GetMax() const { return nMax; }

    void SetAbsColWidth( sal_uInt16 nWidth ) { nAbsColWidth = nWidth; }
    sal_uInt16 GetAbsColWidth() const { return nAbsColWidth; }

    void SetRelColWidth( sal_uInt16 nWidth ) { nRelColWidth = nWidth; }
    sal_uInt16 GetRelColWidth() const { return nRelColWidth; }

    sal_Bool HasLeftBorder() const { return bLeftBorder; }
};

/*  */

class SwHTMLTableLayout
{
    Timer aResizeTimer;             // Timer fuer DelayedResize

    SwHTMLTableLayoutColumn **aColumns;
    SwHTMLTableLayoutCell **aCells;

    const SwTable *pSwTable;        // die SwTable (nur Top-Table)
    SwTableBox *pLeftFillerBox;     // linke Filler-Zelle (nur Tab in Tab)
    SwTableBox *pRightFillerBox;    // rechte Filler-Zelle (nur Tab-in Tab)

    sal_uLong nMin;                     // minimale Breite der Tabelle (Twips)
    sal_uLong nMax;                     // maximale Breite der Tabelle (Twips)

    sal_uInt16 nRows;                   // Anzahl Zeilen
    sal_uInt16 nCols;                   // Anzahl Spalten

    sal_uInt16 nLeftMargin;             // Abstand zum linken Rand (aus Absatz)
    sal_uInt16 nRightMargin;            // Abstand zum rechten Rand (aus Absatz)

    sal_uInt16 nInhAbsLeftSpace;        // von umgebender Zelle geerbter Abstand,
    sal_uInt16 nInhAbsRightSpace;       // der Zellen zugeschlagen wurde

    sal_uInt16 nRelLeftFill;            // relative Breiten der Zellen zur
    sal_uInt16 nRelRightFill;           // Ausrichtung von Tabellen in Tabellen

    sal_uInt16 nRelTabWidth;            // Die relative Breite der Tabelle

    sal_uInt16 nWidthOption;            // die Breite der Tabelle (in Twip oder %)
    sal_uInt16 nCellPadding;            // Abstand zum Inhalt (in Twip)
    sal_uInt16 nCellSpacing;            // Absatnd zwischen Zellen (in Twip)
    sal_uInt16 nBorder;                 // Dicke der ausseren Umrandung bzw.
                                    // Platz, den Netscape hierfuer einrechnet.

    sal_uInt16 nLeftBorderWidth;
    sal_uInt16 nRightBorderWidth;
    sal_uInt16 nInhLeftBorderWidth;
    sal_uInt16 nInhRightBorderWidth;
    sal_uInt16 nBorderWidth;

    sal_uInt16 nDelayedResizeAbsAvail;  // Param fuer's verzoegerte Resize
    sal_uInt16 nLastResizeAbsAvail;

    sal_uInt8 nPass1Done;               // Vorgabe-Werte fuer die einzelen
    sal_uInt8 nWidthSet;                    // Schleifen-Durchlauefe

    SvxAdjust eTableAdjust;         // Die Ausrichtung der Tabelle

    sal_Bool bColsOption : 1;           // Tabelle besitzt eine COLS-Option
    sal_Bool bColTags : 1;              // Tabelle besitzt COL/COLGRP-Tags
    sal_Bool bPrcWidthOption : 1;       // Breite ist eine %-Angabe
    sal_Bool bUseRelWidth : 1;          // SwTable bekommt relative Breite

    sal_Bool bMustResize : 1;           // Tabelle muss in der Breite ang. werden
    sal_Bool bExportable : 1;           // Layout kann zum Export genutzt werden
    sal_Bool bBordersChanged : 1;       // Umrandung wurde geaendert
    sal_Bool bMayBeInFlyFrame : 1;      // Die Tabelle koennte im Rahmen sein

    sal_Bool bDelayedResizeRecalc : 1;  // Param fuer's verzoegerte Resize
    sal_Bool bMustNotResize : 1;        // Die Tabelle darf nicht reseized werden
    sal_Bool bMustNotRecalc : 1;        // Tabelle darf nicht an Inhalt angepasst
                                    // werden

//  sal_uInt16 GetLeftBorderWidth( sal_uInt16 nCol ) const;
//  sal_uInt16 GetRightBorderWidth( sal_uInt16 nCol, sal_uInt16 nColSpan ) const;

    void AddBorderWidth( sal_uLong &rMin, sal_uLong &rMax, sal_uLong& rAbsMin,
                         sal_uInt16 nCol, sal_uInt16 nColSpan,
                         sal_Bool bSwBorders=sal_True ) const;
    void SetBoxWidth( SwTableBox *pBox, sal_uInt16 nCol, sal_uInt16 nColSpan ) const;

    const SwStartNode *GetAnyBoxStartNode() const;
    SwFrmFmt *FindFlyFrmFmt() const;
    const SwDoc *GetDoc() const { return GetAnyBoxStartNode()->GetDoc(); }

    void ClearPass1Info() { nMin = nMax = 0; }

    void _Resize( sal_uInt16 nAbsAvail, sal_Bool bRecalc=sal_False );

    DECL_STATIC_LINK( SwHTMLTableLayout, DelayedResize_Impl, void* );

    static sal_uInt16 GetBrowseWidthByVisArea( const SwDoc& rDoc );
public:

    SwHTMLTableLayout( const SwTable *pSwTbl,
                       sal_uInt16 nRows, sal_uInt16 nCols, sal_Bool bColsOpt, sal_Bool ColTgs,
                       sal_uInt16 nWidth, sal_Bool bPrcWidth, sal_uInt16 nBorderOpt,
                       sal_uInt16 nCellPad, sal_uInt16 nCellSp, SvxAdjust eAdjust,
                       sal_uInt16 nLMargin, sal_uInt16 nRMargin, sal_uInt16 nBWidth,
                       sal_uInt16 nLeftBWidth, sal_uInt16 nRightBWidth,
                       sal_uInt16 nInhLeftBWidth, sal_uInt16 nInhRightBWidth );

    ~SwHTMLTableLayout();

    sal_uInt16 GetLeftCellSpace( sal_uInt16 nCol, sal_uInt16 nColSpan,
                             sal_Bool bSwBorders=sal_True ) const;
    sal_uInt16 GetRightCellSpace( sal_uInt16 nCol, sal_uInt16 nColSpan,
                              sal_Bool bSwBorders=sal_True ) const;
    inline sal_uInt16 GetInhCellSpace( sal_uInt16 nCol, sal_uInt16 nColSpan ) const;

    inline void SetInhBorderWidths( sal_uInt16 nLeft, sal_uInt16 nRight );


    void GetAvail( sal_uInt16 nCol, sal_uInt16 nColSpan, sal_uInt16& rAbsAvail,
                   sal_uInt16& rRelAvail ) const;

    void AutoLayoutPass1();
    void AutoLayoutPass2( sal_uInt16 nAbsAvail, sal_uInt16 nRelAvail,
                          sal_uInt16 nAbsLeftSpace, sal_uInt16 nAbsRightSpace,
                          sal_uInt16 nParentInhSpace );
    void SetWidths( sal_Bool bCallPass2=sal_False, sal_uInt16 nAbsAvail=0,
                    sal_uInt16 nRelAvail=0, sal_uInt16 nAbsLeftSpace=0,
                    sal_uInt16 nAbsRightSpace=0,
                    sal_uInt16 nParentInhSpace=0 );

    inline SwHTMLTableLayoutColumn *GetColumn( sal_uInt16 nCol ) const;
    inline void SetColumn( SwHTMLTableLayoutColumn *pCol, sal_uInt16 nCol );

    inline SwHTMLTableLayoutCell *GetCell( sal_uInt16 nRow, sal_uInt16 nCol ) const;
    inline void SetCell( SwHTMLTableLayoutCell *pCell, sal_uInt16 nRow, sal_uInt16 nCol );

    void SetLeftFillerBox( SwTableBox *pBox ) { pLeftFillerBox = pBox; }
    void SetRightFillerBox( SwTableBox *pBox ) { pRightFillerBox = pBox; }

    sal_uLong GetMin() const { return nMin; }
    sal_uLong GetMax() const { return nMax; }
    sal_uInt16 GetRelLeftFill() const { return nRelLeftFill; }
    sal_uInt16 GetRelRightFill() const { return nRelRightFill; }

    inline long GetBrowseWidthMin() const;

    sal_Bool HasColsOption() const { return bColsOption; }
    sal_Bool HasColTags() const { return bColTags; }

    sal_Bool IsTopTable() const  { return pSwTable != 0; }

    void SetMustResize( sal_Bool bSet ) { bMustResize = bSet; }
    void SetMustNotResize( sal_Bool bSet ) { bMustNotResize = bSet; }
    void SetMustNotRecalc( sal_Bool bSet ) { bMustNotRecalc = bSet; }

    // Neueberechnung der Tabellenbreiten fuer die uebergebene verfuegbare
    // Breite.
    // - Wenn bRecalc gesetzt ist, werden auch der Inhalt der Boxen
    //   zur Berechnung herangezogen.
    //   neu berechnet.
    // - Wenn bForce gesetzt ist, wird die Tabelle auch neu berechnet, wenn
    //   dies mit SetMustNotResize unterdrueckt werden soll.
    // - Wenn nDelay>0 wird die Berechnung entsprechend verzoegert.
    //   Innerhalb der Verzeoegerung auftretende Resize-Aufrufe werden
    //   ignoriert, die Verzeogerung wird aber ggf. uebernommen.
    // - Wenn nDelay==HTMLTABLE_RESIZE_NOW ist, wird sofort Resized und
    //   eventuell noch asstehende Resize-Aufrufe werden nicht mehr
    //   ausgefuehrt.
    // - Der Rueckgabewert gibt an, ob sich die Tabelle geaendert hat.
    sal_Bool Resize( sal_uInt16 nAbsAvail, sal_Bool bRecalc=sal_False, sal_Bool bForce=sal_False,
                 sal_uLong nDelay=0 );

    void BordersChanged( sal_uInt16 nAbsAvail, sal_Bool bRecalc=sal_False );

    // Ermitteln der verfuegbaren Breite. Das geht nur, wenn ein Layout
    // oder eine ViewShell vorhanden ist. Sonst wird 0 zurueckgegeben.
    // (Wird vom HTML-Filter benoetigt, da der nicht an das Layout kommt.)
    static sal_uInt16 GetBrowseWidth( const SwDoc& rDoc );

    // Ermitteln der verfuegbaren Breite uber den Tabellen-Frame
    sal_uInt16 GetBrowseWidthByTabFrm( const SwTabFrm& rTabFrm ) const;

    // Ermitteln der verfuegbaren Breite uber den Tabellen-Frame oder
    // das statische GetBrowseWidth, wenn kein Layout existiert.
    sal_uInt16 GetBrowseWidthByTable( const SwDoc& rDoc ) const;

    // Fuer Export
    sal_uInt16 GetWidthOption() const { return nWidthOption; }
    sal_Bool   HasPrcWidthOption() const { return bPrcWidthOption; }

    sal_uInt16 GetCellPadding() const { return nCellPadding; }
    sal_uInt16 GetCellSpacing() const { return nCellSpacing; }
    sal_uInt16 GetBorder() const { return nBorder; }

    sal_uInt16 GetRowCount() const { return nRows; }
    sal_uInt16 GetColCount() const { return nCols; }

    void SetExportable( sal_Bool bSet ) { bExportable = bSet; }
    sal_Bool IsExportable() const { return bExportable; }

    sal_Bool HaveBordersChanged() const { return bBordersChanged; }

    void SetMayBeInFlyFrame( sal_Bool bSet ) { bMayBeInFlyFrame = bSet; }
    sal_Bool MayBeInFlyFrame() const { return bMayBeInFlyFrame; }
};

/*  */

inline void SwHTMLTableLayoutCell::SetProtected()
{
    nRowSpan = 1;
    nColSpan = 1;

    pContents = 0;
}

/*  */

inline void SwHTMLTableLayoutColumn::MergeMinMaxNoAlign( sal_uLong nCMin,
    sal_uLong nCMax,    sal_uLong nAbsMin )
{
    if( nCMin > nMinNoAlign )
        nMinNoAlign = nCMin;
    if( nCMax > nMaxNoAlign )
        nMaxNoAlign = nCMax;
    if( nAbsMin > nAbsMinNoAlign )
        nAbsMinNoAlign = nAbsMin;
}

inline void SwHTMLTableLayoutColumn::ClearPass1Info( sal_Bool bWidthOpt )
{
    nMinNoAlign = nMaxNoAlign = nAbsMinNoAlign = MINLAY;
    nMin = nMax = 0;
    if( bWidthOpt )
    {
        nWidthOption = 0;
        bRelWidthOption = sal_False;
    }
}

inline void SwHTMLTableLayoutColumn::MergeCellWidthOption(
    sal_uInt16 nWidth, sal_Bool bRel )
{
    if( !nWidthOption ||
        (bRel==bRelWidthOption && nWidthOption < nWidth) )
    {
        nWidthOption = nWidth;
        bRelWidthOption = bRel;
    }
}

inline void SwHTMLTableLayoutColumn::SetMinMax( sal_uLong nMn, sal_uLong nMx )
{
    nMin = nMn;
    nMax = nMx;
}

/*  */

inline sal_uInt16 SwHTMLTableLayout::GetInhCellSpace( sal_uInt16 nCol,
                                                  sal_uInt16 nColSpan ) const
{
    sal_uInt16 nSpace = 0;
    if( nCol==0 )
        nSpace = nSpace + sal::static_int_cast< sal_uInt16 >(nInhAbsLeftSpace);
    if( nCol+nColSpan==nCols )
        nSpace = nSpace + sal::static_int_cast< sal_uInt16 >(nInhAbsRightSpace);

    return nSpace;
}

inline SwHTMLTableLayoutColumn *SwHTMLTableLayout::GetColumn( sal_uInt16 nCol ) const
{
    return aColumns[nCol];
}

inline void SwHTMLTableLayoutColumn::SetWidthOption(
    sal_uInt16 nWidth, sal_Bool bRelWidth, sal_Bool bTest )
{
    if( bTest && bRelWidthOption==bRelWidth )
    {
        if( nWidth > nWidthOption )
            nWidthOption = nWidth;
    }
    else
        nWidthOption = nWidth;
    bRelWidthOption = bRelWidth;
}

inline void SwHTMLTableLayout::SetColumn( SwHTMLTableLayoutColumn *pCol, sal_uInt16 nCol )
{
    aColumns[nCol] = pCol;
}

inline SwHTMLTableLayoutCell *SwHTMLTableLayout::GetCell( sal_uInt16 nRow, sal_uInt16 nCol ) const
{
    return aCells[nRow*nCols+nCol];
}

inline void SwHTMLTableLayout::SetCell( SwHTMLTableLayoutCell *pCell,
                               sal_uInt16 nRow, sal_uInt16 nCol )
{
    aCells[nRow*nCols+nCol] = pCell;
}

inline long SwHTMLTableLayout::GetBrowseWidthMin() const
{
    return (long)( (!nWidthOption || bPrcWidthOption) ? nMin : nRelTabWidth );
}

void SwHTMLTableLayout::SetInhBorderWidths( sal_uInt16 nLeft, sal_uInt16 nRight )
{
    nInhLeftBorderWidth = nLeft;
    nInhRightBorderWidth = nRight;
}


#endif
