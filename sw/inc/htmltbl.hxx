/*************************************************************************
 *
 *  $RCSfile: htmltbl.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:26 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _HTMLTBL_HXX
#define _HTMLTBL_HXX


#ifndef _TIMER_HXX //autogen
#include <vcl/timer.hxx>
#endif

#ifndef _SVX_SVXENUM_HXX //autogen
#include <svx/svxenum.hxx>
#endif

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
    BYTE nPass1Done;            // Wieoft wurde Pass 1 aufgerufen?
    BYTE nWidthSet;             // Wieoft wurde die Breite gesetzt?

    BOOL bNoBreakTag;       // <NOBR>-Tag ueber gesamten Inhalt

public:

    SwHTMLTableLayoutCnts( const SwStartNode* pSttNd, SwHTMLTableLayout* pTab,
                           BOOL bNoBreakTag, SwHTMLTableLayoutCnts* pNxt );

    ~SwHTMLTableLayoutCnts();

    void SetTableBox( SwTableBox *pBx ) { pBox = pBx; }
    SwTableBox *GetTableBox() const { return pBox; }

    SwHTMLTableLayout *GetTable() const { return pTable; }

    const SwStartNode *GetStartNode() const;

    // Ermitteln des naechsten Knotens
    SwHTMLTableLayoutCnts *GetNext() const { return pNext; }

    void SetWidthSet( BYTE nRef ) { nWidthSet = nRef; }
    BOOL IsWidthSet( BYTE nRef ) const { return nRef==nWidthSet; }

    void SetPass1Done( BYTE nRef ) { nPass1Done = nRef; }
    BOOL IsPass1Done( BYTE nRef ) const { return nRef==nPass1Done; }

    BOOL HasNoBreakTag() const { return bNoBreakTag; }
};

/*  */

class SwHTMLTableLayoutCell
{
    SwHTMLTableLayoutCnts *pContents;       // der Inhalt der Zelle

    USHORT nRowSpan;    // ROWSPAN der Zelle
    USHORT nColSpan;    // COLSPAN der Zelle
    USHORT nWidthOption;// angegebene Breite der Zelle in Twip oder %

    BOOL bPrcWidthOption : 1;// nWidth ist %-Angabe
    BOOL bNoWrapOption : 1; // NOWRAP-Option

public:

    SwHTMLTableLayoutCell( SwHTMLTableLayoutCnts *pCnts,
                         USHORT nRSpan, USHORT nCSpan,
                         USHORT nWidthOpt, BOOL bPrcWdthOpt,
                         BOOL nNWrapOpt );

    ~SwHTMLTableLayoutCell();

    // Setzen/Ermitteln des Inhalts einer Zelle
    void SetContents( SwHTMLTableLayoutCnts *pCnts ) { pContents = pCnts; }
    SwHTMLTableLayoutCnts *GetContents() const { return pContents; }

    inline void SetProtected();

    // ROWSPAN/COLSPAN der Zelle Setzen/Ermitteln
    void SetRowSpan( USHORT nRSpan ) { nRowSpan = nRSpan; }
    USHORT GetRowSpan() const { return nRowSpan; }
    USHORT GetColSpan() const { return nColSpan; }

    USHORT GetWidthOption() const { return nWidthOption; }
    BOOL IsPrcWidthOption() const { return bPrcWidthOption; }

    BOOL HasNoWrapOption() const { return bNoWrapOption; }
};

/*  */

class SwHTMLTableLayoutColumn
{
    // Zwischenwerte von AutoLayoutPass1
    ULONG nMinNoAlign, nMaxNoAlign, nAbsMinNoAlign;

    // Ergebnisse von AutoLayoutPass1
    ULONG nMin, nMax;

    // Ergibnisse von Pass 2
    USHORT nAbsColWidth;                // in Twips
    USHORT nRelColWidth;                // in Twips bzw. relativ zu USHRT_MAX

    USHORT nWidthOption;                // Optionen von <COL> oder <TD>/<TH>

    BOOL bRelWidthOption : 1;
    BOOL bLeftBorder : 1;

public:

    SwHTMLTableLayoutColumn( USHORT nColWidthOpt, BOOL bRelColWidthOpt,
                             BOOL bLBorder );

    ~SwHTMLTableLayoutColumn() {}

    inline void MergeCellWidthOption( USHORT nWidth, BOOL bPrc );
    inline void SetWidthOption( USHORT nWidth, BOOL bRelWidth, BOOL bTest );

    USHORT GetWidthOption() const { return nWidthOption; }
    BOOL IsRelWidthOption() const { return bRelWidthOption; }

    inline void MergeMinMaxNoAlign( ULONG nMin, ULONG nMax, ULONG nAbsMin );
    ULONG GetMinNoAlign() const { return nMinNoAlign; }
    ULONG GetMaxNoAlign() const { return nMaxNoAlign; }
    ULONG GetAbsMinNoAlign() const { return nAbsMinNoAlign; }
    inline void ClearPass1Info( BOOL bWidthOpt );

    inline void SetMinMax( ULONG nMin, ULONG nMax );
    void SetMax( ULONG nVal ) { nMax = nVal; }
    void AddToMin( ULONG nVal ) { nMin += nVal; }
    void AddToMax( ULONG nVal ) { nMax += nVal; }
    ULONG GetMin() const { return nMin; }
    ULONG GetMax() const { return nMax; }

    void SetAbsColWidth( USHORT nWidth ) { nAbsColWidth = nWidth; }
    USHORT GetAbsColWidth() const { return nAbsColWidth; }

    void SetRelColWidth( USHORT nWidth ) { nRelColWidth = nWidth; }
    USHORT GetRelColWidth() const { return nRelColWidth; }

    BOOL HasLeftBorder() const { return bLeftBorder; }
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

    ULONG nMin;                     // minimale Breite der Tabelle (Twips)
    ULONG nMax;                     // maximale Breite der Tabelle (Twips)

    USHORT nRows;                   // Anzahl Zeilen
    USHORT nCols;                   // Anzahl Spalten

    USHORT nLeftMargin;             // Abstand zum linken Rand (aus Absatz)
    USHORT nRightMargin;            // Abstand zum rechten Rand (aus Absatz)

    USHORT nInhAbsLeftSpace;        // von umgebender Zelle geerbter Abstand,
    USHORT nInhAbsRightSpace;       // der Zellen zugeschlagen wurde

    USHORT nRelLeftFill;            // relative Breiten der Zellen zur
    USHORT nRelRightFill;           // Ausrichtung von Tabellen in Tabellen

    USHORT nRelTabWidth;            // Die relative Breite der Tabelle

    USHORT nWidthOption;            // die Breite der Tabelle (in Twip oder %)
    USHORT nCellPadding;            // Abstand zum Inhalt (in Twip)
    USHORT nCellSpacing;            // Absatnd zwischen Zellen (in Twip)
    USHORT nBorder;                 // Dicke der ausseren Umrandung bzw.
                                    // Platz, den Netscape hierfuer einrechnet.

    USHORT nLeftBorderWidth;
    USHORT nRightBorderWidth;
    USHORT nInhLeftBorderWidth;
    USHORT nInhRightBorderWidth;
    USHORT nBorderWidth;

    USHORT nDelayedResizeAbsAvail;  // Param fuer's verzoegerte Resize
    USHORT nLastResizeAbsAvail;

    BYTE nPass1Done;                // Vorgabe-Werte fuer die einzelen
    BYTE nWidthSet;                 // Schleifen-Durchlauefe

    SvxAdjust eTableAdjust;         // Die Ausrichtung der Tabelle

    BOOL bColsOption : 1;           // Tabelle besitzt eine COLS-Option
    BOOL bColTags : 1;              // Tabelle besitzt COL/COLGRP-Tags
    BOOL bPrcWidthOption : 1;       // Breite ist eine %-Angabe
    BOOL bUseRelWidth : 1;          // SwTable bekommt relative Breite

    BOOL bMustResize : 1;           // Tabelle muss in der Breite ang. werden
    BOOL bExportable : 1;           // Layout kann zum Export genutzt werden
    BOOL bBordersChanged : 1;       // Umrandung wurde geaendert
    BOOL bMayBeInFlyFrame : 1;      // Die Tabelle koennte im Rahmen sein

    BOOL bDelayedResizeRecalc : 1;  // Param fuer's verzoegerte Resize
    BOOL bMustNotResize : 1;        // Die Tabelle darf nicht reseized werden
    BOOL bMustNotRecalc : 1;        // Tabelle darf nicht an Inhalt angepasst
                                    // werden

//  USHORT GetLeftBorderWidth( USHORT nCol ) const;
//  USHORT GetRightBorderWidth( USHORT nCol, USHORT nColSpan ) const;

    void AddBorderWidth( ULONG &rMin, ULONG &rMax, ULONG& rAbsMin,
                         USHORT nCol, USHORT nColSpan,
                         BOOL bSwBorders=TRUE ) const;
    void SetBoxWidth( SwTableBox *pBox, USHORT nCol, USHORT nColSpan ) const;

    const SwStartNode *GetAnyBoxStartNode() const;
    SwFrmFmt *FindFlyFrmFmt() const;
    const SwDoc *GetDoc() const { return GetAnyBoxStartNode()->GetDoc(); }

    void ClearPass1Info() { nMin = nMax = 0; }

    void _Resize( USHORT nAbsAvail, BOOL bRecalc=FALSE );

    DECL_STATIC_LINK( SwHTMLTableLayout, DelayedResize_Impl, void* );

public:

    SwHTMLTableLayout( const SwTable *pSwTbl,
                       USHORT nRows, USHORT nCols, BOOL bColsOpt, BOOL ColTgs,
                       USHORT nWidth, BOOL bPrcWidth, USHORT nBorderOpt,
                       USHORT nCellPad, USHORT nCellSp, SvxAdjust eAdjust,
                       USHORT nLMargin, USHORT nRMargin, USHORT nBWidth,
                       USHORT nLeftBWidth, USHORT nRightBWidth,
                       USHORT nInhLeftBWidth, USHORT nInhRightBWidth );

    ~SwHTMLTableLayout();

    USHORT GetLeftCellSpace( USHORT nCol, USHORT nColSpan,
                             BOOL bSwBorders=TRUE ) const;
    USHORT GetRightCellSpace( USHORT nCol, USHORT nColSpan,
                              BOOL bSwBorders=TRUE ) const;
    inline USHORT GetInhCellSpace( USHORT nCol, USHORT nColSpan ) const;

    inline void SetInhBorderWidths( USHORT nLeft, USHORT nRight );


    void GetAvail( USHORT nCol, USHORT nColSpan, USHORT& rAbsAvail,
                   USHORT& rRelAvail ) const;

    void AutoLayoutPass1();
    void AutoLayoutPass2( USHORT nAbsAvail, USHORT nRelAvail,
                          USHORT nAbsLeftSpace, USHORT nAbsRightSpace,
                          USHORT nParentInhSpace );
    void SetWidths( BOOL bCallPass2=FALSE, USHORT nAbsAvail=0,
                    USHORT nRelAvail=0, USHORT nAbsLeftSpace=0,
                    USHORT nAbsRightSpace=0,
                    USHORT nParentInhSpace=0 );

    inline SwHTMLTableLayoutColumn *GetColumn( USHORT nCol ) const;
    inline void SetColumn( SwHTMLTableLayoutColumn *pCol, USHORT nCol );

    inline SwHTMLTableLayoutCell *GetCell( USHORT nRow, USHORT nCol ) const;
    inline void SetCell( SwHTMLTableLayoutCell *pCell, USHORT nRow, USHORT nCol );

    void SetLeftFillerBox( SwTableBox *pBox ) { pLeftFillerBox = pBox; }
    void SetRightFillerBox( SwTableBox *pBox ) { pRightFillerBox = pBox; }

    ULONG GetMin() const { return nMin; }
    ULONG GetMax() const { return nMax; }
    USHORT GetRelLeftFill() const { return nRelLeftFill; }
    USHORT GetRelRightFill() const { return nRelRightFill; }

    inline long GetBrowseWidthMin() const;

    BOOL HasColsOption() const { return bColsOption; }
    BOOL HasColTags() const { return bColTags; }

    BOOL IsTopTable() const  { return pSwTable != 0; }

    void SetMustResize( BOOL bSet ) { bMustResize = bSet; }
    void SetMustNotResize( BOOL bSet ) { bMustNotResize = bSet; }
    void SetMustNotRecalc( BOOL bSet ) { bMustNotRecalc = bSet; }

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
    BOOL Resize( USHORT nAbsAvail, BOOL bRecalc=FALSE, BOOL bForce=FALSE,
                 ULONG nDelay=0 );

    void BordersChanged( USHORT nAbsAvail, BOOL bRecalc=FALSE );

    // Ermitteln der verfuegbaren Breite. Das geht nur, wenn ein Layout
    // oder eine ViewShell vorhanden ist. Sonst wird 0 zurueckgegeben.
    // (Wird vom HTML-Filter benoetigt, da der nicht an das Layout kommt.)
    static USHORT GetBrowseWidth( const SwDoc& rDoc );
    static USHORT GetBrowseWidthByVisArea( const SwDoc& rDoc );

    // Ermitteln der verfuegbaren Breite uber den Tabellen-Frame
    USHORT GetBrowseWidthByTabFrm( const SwTabFrm& rTabFrm ) const;

    // Ermitteln der verfuegbaren Breite uber den Tabellen-Frame oder
    // das statische GetBrowseWidth, wenn kein Layout existiert.
    USHORT GetBrowseWidthByTable( const SwDoc& rDoc ) const;

    // Fuer Export
    USHORT GetWidthOption() const { return nWidthOption; }
    BOOL   HasPrcWidthOption() const { return bPrcWidthOption; }

    USHORT GetCellPadding() const { return nCellPadding; }
    USHORT GetCellSpacing() const { return nCellSpacing; }
    USHORT GetBorder() const { return nBorder; }

    USHORT GetRowCount() const { return nRows; }
    USHORT GetColCount() const { return nCols; }

    void SetExportable( BOOL bSet ) { bExportable = bSet; }
    BOOL IsExportable() const { return bExportable; }

    BOOL HaveBordersChanged() const { return bBordersChanged; }

    void SetMayBeInFlyFrame( BOOL bSet ) { bMayBeInFlyFrame = bSet; }
    BOOL MayBeInFlyFrame() const { return bMayBeInFlyFrame; }
};

/*  */

inline void SwHTMLTableLayoutCell::SetProtected()
{
    nRowSpan = 1;
    nColSpan = 1;

    pContents = 0;
}

/*  */

inline void SwHTMLTableLayoutColumn::MergeMinMaxNoAlign( ULONG nMin,
    ULONG nMax, ULONG nAbsMin )
{
    if( nMin > nMinNoAlign )
        nMinNoAlign = nMin;
    if( nMax > nMaxNoAlign )
        nMaxNoAlign = nMax;
    if( nAbsMin > nAbsMinNoAlign )
        nAbsMinNoAlign = nAbsMin;
}

inline void SwHTMLTableLayoutColumn::ClearPass1Info( BOOL bWidthOpt )
{
    nMinNoAlign = nMaxNoAlign = nAbsMinNoAlign = MINLAY;
    nMin = nMax = 0;
    if( bWidthOpt )
    {
        nWidthOption = 0;
        bRelWidthOption = FALSE;
    }
}

inline void SwHTMLTableLayoutColumn::MergeCellWidthOption(
    USHORT nWidth, BOOL bRel )
{
    if( !nWidthOption ||
        (bRel==bRelWidthOption && nWidthOption < nWidth) )
    {
        nWidthOption = nWidth;
        bRelWidthOption = bRel;
    }
}

inline void SwHTMLTableLayoutColumn::SetMinMax( ULONG nMn, ULONG nMx )
{
    nMin = nMn;
    nMax = nMx;
}

/*  */

inline USHORT SwHTMLTableLayout::GetInhCellSpace( USHORT nCol,
                                                  USHORT nColSpan ) const
{
    USHORT nSpace = 0;
    if( nCol==0 )
        nSpace += nInhAbsLeftSpace;
    if( nCol+nColSpan==nCols )
        nSpace += nInhAbsRightSpace;

    return nSpace;
}

inline SwHTMLTableLayoutColumn *SwHTMLTableLayout::GetColumn( USHORT nCol ) const
{
    return aColumns[nCol];
}

inline void SwHTMLTableLayoutColumn::SetWidthOption(
    USHORT nWidth, BOOL bRelWidth, BOOL bTest )
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

inline void SwHTMLTableLayout::SetColumn( SwHTMLTableLayoutColumn *pCol, USHORT nCol )
{
    aColumns[nCol] = pCol;
}

inline SwHTMLTableLayoutCell *SwHTMLTableLayout::GetCell( USHORT nRow, USHORT nCol ) const
{
    return aCells[nRow*nCols+nCol];
}

inline void SwHTMLTableLayout::SetCell( SwHTMLTableLayoutCell *pCell,
                               USHORT nRow, USHORT nCol )
{
    aCells[nRow*nCols+nCol] = pCell;
}

inline long SwHTMLTableLayout::GetBrowseWidthMin() const
{
    return (long)( (!nWidthOption || bPrcWidthOption) ? nMin : nRelTabWidth );
}

void SwHTMLTableLayout::SetInhBorderWidths( USHORT nLeft, USHORT nRight )
{
    nInhLeftBorderWidth = nLeft;
    nInhRightBorderWidth = nRight;
}


#endif
