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

#ifndef _HTMLTBL_HXX
#define _HTMLTBL_HXX


#include <vcl/timer.hxx>
#include <editeng/svxenum.hxx>

#include "swtypes.hxx"
#include "node.hxx"     // For SwStartNode


class SwTableBox;
class SwTable;
class SwHTMLTableLayout;
class SwDoc;
class SwFrmFmt;

#define HTMLTABLE_RESIZE_NOW (ULONG_MAX)

class SwHTMLTableLayoutCnts
{
    SwHTMLTableLayoutCnts *pNext;   // The next content.

    // Only one of the following two pointers may be set!
    SwTableBox *pBox;           // A Box.
    SwHTMLTableLayout *pTable;  // A "table within a table".

    // During first run there are still no boxes. In this case
    // pStartNode is used instead of pBox.
    const SwStartNode *pStartNode;

    // The following counters indicate how often a pass has been
    // done for this content. Therefore they are compared against
    // a reference value. If 255 is reached the continue with 0.
    // This avoids reinitialization on every resize.
    BYTE nPass1Done;            // How many times has Pass 1 been called?
    BYTE nWidthSet;             // How many times has the width been set?

    BOOL bNoBreakTag;       // <NOBR>-Tag over complete content.

public:

    SwHTMLTableLayoutCnts( const SwStartNode* pSttNd, SwHTMLTableLayout* pTab,
                           BOOL bNoBreakTag, SwHTMLTableLayoutCnts* pNxt );

    ~SwHTMLTableLayoutCnts();

    void SetTableBox( SwTableBox *pBx ) { pBox = pBx; }
    SwTableBox *GetTableBox() const { return pBox; }

    SwHTMLTableLayout *GetTable() const { return pTable; }

    const SwStartNode *GetStartNode() const;

    // Calculation of next node.
    SwHTMLTableLayoutCnts *GetNext() const { return pNext; }

    void SetWidthSet( BYTE nRef ) { nWidthSet = nRef; }
    BOOL IsWidthSet( BYTE nRef ) const { return nRef==nWidthSet; }

    void SetPass1Done( BYTE nRef ) { nPass1Done = nRef; }
    BOOL IsPass1Done( BYTE nRef ) const { return nRef==nPass1Done; }

    BOOL HasNoBreakTag() const { return bNoBreakTag; }
};

class SwHTMLTableLayoutCell
{
    SwHTMLTableLayoutCnts *pContents;       // Content of cell.

    USHORT nRowSpan;    // ROWSPAN of cell.
    USHORT nColSpan;    // COLSPAN of cell.
    USHORT nWidthOption;// Given width of cell in Twip or %.

    BOOL bPrcWidthOption : 1;// nWidth is %-value.
    BOOL bNoWrapOption : 1; // NOWRAP-option.

public:

    SwHTMLTableLayoutCell( SwHTMLTableLayoutCnts *pCnts,
                         USHORT nRSpan, USHORT nCSpan,
                         USHORT nWidthOpt, BOOL bPrcWdthOpt,
                         BOOL nNWrapOpt );

    ~SwHTMLTableLayoutCell();

    // Set or get content of a cell.
    void SetContents( SwHTMLTableLayoutCnts *pCnts ) { pContents = pCnts; }
    SwHTMLTableLayoutCnts *GetContents() const { return pContents; }

    inline void SetProtected();

    // Set or get ROWSPAN/COLSPAN of cell.
    void SetRowSpan( USHORT nRSpan ) { nRowSpan = nRSpan; }
    USHORT GetRowSpan() const { return nRowSpan; }
    USHORT GetColSpan() const { return nColSpan; }

    USHORT GetWidthOption() const { return nWidthOption; }
    BOOL IsPrcWidthOption() const { return bPrcWidthOption; }

    BOOL HasNoWrapOption() const { return bNoWrapOption; }
};

class SwHTMLTableLayoutColumn
{

    // Interim values of AutoLayoutPass1,
    ULONG nMinNoAlign, nMaxNoAlign, nAbsMinNoAlign;

    // Results of AutoLayoutPass1
    ULONG nMin, nMax;

    // Results of Pass 2.
    USHORT nAbsColWidth;                // In Twips.
    USHORT nRelColWidth;                // In Twips or relative to USHRT_MAX.

    USHORT nWidthOption;                // Options of <COL> or <TD>/<TH>.

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

class SwHTMLTableLayout
{
    Timer aResizeTimer;             // Timer for DelayedResize.

    SwHTMLTableLayoutColumn **aColumns;
    SwHTMLTableLayoutCell **aCells;

    const SwTable *pSwTable;        // SwTable (Top-Table only).
    SwTableBox *pLeftFillerBox;     // Left filler-box (table in table only).
    SwTableBox *pRightFillerBox;    // Right filler-box (table in Table only).

    ULONG nMin;                     // Minimal width of table (Twips).
    ULONG nMax;                     // Maximal width of table (Twips).

    USHORT nRows;                   // Row count.
    USHORT nCols;                   // Column count.

    USHORT nLeftMargin;             // Space to left margin (from paragraph).
    USHORT nRightMargin;            // Space to left margin (from paragraph).

    USHORT nInhAbsLeftSpace;        // Space inherited from surrounding box
    USHORT nInhAbsRightSpace;       // that was added to boxes.

    USHORT nRelLeftFill;            // Width of boxes relative to alignment
    USHORT nRelRightFill;           // of tables in tables.

    USHORT nRelTabWidth;            // Relative width of table.

    USHORT nWidthOption;            // Width of table (in Twips oder %).
    USHORT nCellPadding;            // Space to contents (in Twips).
    USHORT nCellSpacing;            // Cell spacing (in Twips).
    USHORT nBorder;                 // Line strength of outer border, or rather the
                                    // space needed for it as calculated by Netscape.

    USHORT nLeftBorderWidth;
    USHORT nRightBorderWidth;
    USHORT nInhLeftBorderWidth;
    USHORT nInhRightBorderWidth;
    USHORT nBorderWidth;

    USHORT nDelayedResizeAbsAvail;  // Param for delayed Resize.
    USHORT nLastResizeAbsAvail;

    BYTE nPass1Done;                // Reference-values for
    BYTE nWidthSet;                 // the runs through loop.

    SvxAdjust eTableAdjust;         // Alignment of table.

    BOOL bColsOption : 1;           // Table has a COLS-option.
    BOOL bColTags : 1;              // Tabelle has COL/COLGRP-tags.
    BOOL bPrcWidthOption : 1;       // Width is given in percent.
    BOOL bUseRelWidth : 1;          // SwTable gets relative width.

    BOOL bMustResize : 1;           // Table width must be defined.
    BOOL bExportable : 1;           // Layout may be used for export.
    BOOL bBordersChanged : 1;       // Borders have been changed.
    BOOL bMayBeInFlyFrame : 1;      // Table could be within frame.

    BOOL bDelayedResizeRecalc : 1;  // Param for delayed Resize.
    BOOL bMustNotResize : 1;        // Table may not be resized.
    BOOL bMustNotRecalc : 1;        // Table may not be adapted to its contents.

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

    static USHORT GetBrowseWidthByVisArea( const SwDoc& rDoc );
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

    // Recalculation of table widths for available width that has been passed.
    // - If bRecalc is set, contents of boxes are included into calculation.
    // - If bForce is set, table will be recalculated even if this was
    //   disallowed by SetMustNotResize.
    // - If nDelay > 0 the calculation is delayed accordingly. Resizing calls
    //   occuring during delay-time are ignored, but the delay may be counted
    //   under certain circumstances.
    // - If nDelay == HTMLTABLE_RESIZE_NOW, resize immediately and do not
    //   consider any resize-calls that might possibly be in order.
    // - The return value indicates whether the table has been changed.
    BOOL Resize( USHORT nAbsAvail, BOOL bRecalc=FALSE, BOOL bForce=FALSE,
                 ULONG nDelay=0 );

    void BordersChanged( USHORT nAbsAvail, BOOL bRecalc=FALSE );

    // Calculate available width. This works only if a layout or a
    // ViewShell exists. Otherwise returns 0.
    // This is needed by HTML-filter because it doesn't have access to the layout.)
    static USHORT GetBrowseWidth( const SwDoc& rDoc );

    // Calculates available width by table-frame.
    USHORT GetBrowseWidthByTabFrm( const SwTabFrm& rTabFrm ) const;

    // Calculates available width by the table-frame or
    // static GetBrowseWidth if no layout exists.
    USHORT GetBrowseWidthByTable( const SwDoc& rDoc ) const;

    // For Export.
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

inline void SwHTMLTableLayoutCell::SetProtected()
{
    nRowSpan = 1;
    nColSpan = 1;

    pContents = 0;
}

inline void SwHTMLTableLayoutColumn::MergeMinMaxNoAlign( ULONG nCMin,
    ULONG nCMax,    ULONG nAbsMin )
{
    if( nCMin > nMinNoAlign )
        nMinNoAlign = nCMin;
    if( nCMax > nMaxNoAlign )
        nMaxNoAlign = nCMax;
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

inline USHORT SwHTMLTableLayout::GetInhCellSpace( USHORT nCol,
                                                  USHORT nColSpan ) const
{
    USHORT nSpace = 0;
    if( nCol==0 )
        nSpace = nSpace + sal::static_int_cast< USHORT >(nInhAbsLeftSpace);
    if( nCol+nColSpan==nCols )
        nSpace = nSpace + sal::static_int_cast< USHORT >(nInhAbsRightSpace);

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
