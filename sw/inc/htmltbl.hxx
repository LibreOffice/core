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

#ifndef INCLUDED_SW_INC_HTMLTBL_HXX
#define INCLUDED_SW_INC_HTMLTBL_HXX

#include <vcl/timer.hxx>
#include <editeng/svxenum.hxx>

#include "swtypes.hxx"
#include "node.hxx"

class SwTableBox;
class SwTable;
class SwHTMLTableLayout;
class SwDoc;
class SwFrameFormat;

#define HTMLTABLE_RESIZE_NOW (ULONG_MAX)

class SwHTMLTableLayoutCnts
{
    SwHTMLTableLayoutCnts *pNext;   ///< The next content.

    /// Only one of the following two pointers may be set!
    SwTableBox *pBox;               ///< A Box.
    SwHTMLTableLayout *pTable;      ///< A "table within a table".

    /** During first run there are still no boxes. In this case
       pStartNode is used instead of pBox. */
    const SwStartNode *pStartNode;

    /** The following counters indicate how often a pass has been
        done for this content. Therefore they are compared against
        a reference value. If 255 is reached the continue with 0.
        This avoids reinitialization on every resize. */
    sal_uInt8 nPass1Done;           ///< How many times has Pass 1 been called?
    sal_uInt8 nWidthSet;            ///< How many times has the width been set?

    bool bNoBreakTag;           ///< <NOBR>-Tag over complete content.

public:

    SwHTMLTableLayoutCnts( const SwStartNode* pSttNd, SwHTMLTableLayout* pTab,
                           bool bNoBreakTag, SwHTMLTableLayoutCnts* pNxt );

    ~SwHTMLTableLayoutCnts();

    void SetTableBox( SwTableBox *pBx ) { pBox = pBx; }
    SwTableBox *GetTableBox() const { return pBox; }

    SwHTMLTableLayout *GetTable() const { return pTable; }

    const SwStartNode *GetStartNode() const;

    /// Calculation of next node.
    SwHTMLTableLayoutCnts *GetNext() const { return pNext; }

    void SetWidthSet( sal_uInt8 nRef ) { nWidthSet = nRef; }
    bool IsWidthSet( sal_uInt8 nRef ) const { return nRef==nWidthSet; }

    void SetPass1Done( sal_uInt8 nRef ) { nPass1Done = nRef; }
    bool IsPass1Done( sal_uInt8 nRef ) const { return nRef==nPass1Done; }

    bool HasNoBreakTag() const { return bNoBreakTag; }
};

class SwHTMLTableLayoutCell
{
    SwHTMLTableLayoutCnts *pContents;  ///< Content of cell.

    sal_uInt16 nRowSpan;               ///< ROWSPAN of cell.
    sal_uInt16 nColSpan;               ///< COLSPAN of cell.
    sal_uInt16 nWidthOption;           ///< Given width of cell in Twip or %.

    bool bPrcWidthOption : 1;      ///< nWidth is %-value.
    bool bNoWrapOption : 1;        ///< NOWRAP-option.

public:

    SwHTMLTableLayoutCell( SwHTMLTableLayoutCnts *pCnts,
                         sal_uInt16 nRSpan, sal_uInt16 nCSpan,
                         sal_uInt16 nWidthOpt, bool bPrcWdthOpt,
                         bool bNWrapOpt );

    ~SwHTMLTableLayoutCell();

    /// Set or get content of a cell.
    void SetContents( SwHTMLTableLayoutCnts *pCnts ) { pContents = pCnts; }
    SwHTMLTableLayoutCnts *GetContents() const { return pContents; }

    inline void SetProtected();

    /// Set or get ROWSPAN/COLSPAN of cell.
    void SetRowSpan( sal_uInt16 nRSpan ) { nRowSpan = nRSpan; }
    sal_uInt16 GetRowSpan() const { return nRowSpan; }
    sal_uInt16 GetColSpan() const { return nColSpan; }

    sal_uInt16 GetWidthOption() const { return nWidthOption; }
    bool IsPrcWidthOption() const { return bPrcWidthOption; }

    bool HasNoWrapOption() const { return bNoWrapOption; }
};

class SwHTMLTableLayoutColumn
{

    /// Interim values of AutoLayoutPass1,
    sal_uLong nMinNoAlign, nMaxNoAlign, nAbsMinNoAlign;

    /// Results of AutoLayoutPass1
    sal_uLong nMin, nMax;

    /// Results of Pass 2.
    sal_uInt16 nAbsColWidth;                ///< In Twips.
    sal_uInt16 nRelColWidth;                ///< In Twips or relative to USHRT_MAX.

    sal_uInt16 nWidthOption;                ///< Options of <COL> or <TD>/<TH>.

    bool bRelWidthOption : 1;
    bool bLeftBorder : 1;

public:

    SwHTMLTableLayoutColumn( sal_uInt16 nColWidthOpt, bool bRelColWidthOpt,
                             bool bLBorder );

    ~SwHTMLTableLayoutColumn() {}

    inline void MergeCellWidthOption( sal_uInt16 nWidth, bool bPrc );
    inline void SetWidthOption( sal_uInt16 nWidth, bool bRelWidth, bool bTest );

    sal_uInt16 GetWidthOption() const { return nWidthOption; }
    bool IsRelWidthOption() const { return bRelWidthOption; }

    inline void MergeMinMaxNoAlign( sal_uLong nMin, sal_uLong nMax, sal_uLong nAbsMin );
    sal_uLong GetMinNoAlign() const { return nMinNoAlign; }
    sal_uLong GetMaxNoAlign() const { return nMaxNoAlign; }
    sal_uLong GetAbsMinNoAlign() const { return nAbsMinNoAlign; }
    inline void ClearPass1Info( bool bWidthOpt );

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

    bool HasLeftBorder() const { return bLeftBorder; }
};

class SwHTMLTableLayout
{
    Timer m_aResizeTimer;             ///< Timer for DelayedResize.

    SwHTMLTableLayoutColumn **m_aColumns;
    SwHTMLTableLayoutCell **m_aCells;

    const SwTable *m_pSwTable;            ///< SwTable (Top-Table only).
    SwTableBox *m_pLeftFillerBox;         ///< Left filler-box (table in table only).
    SwTableBox *m_pRightFillerBox;        ///< Right filler-box (table in Table only).

    sal_uLong m_nMin;                     ///< Minimal width of table (Twips).
    sal_uLong m_nMax;                     ///< Maximal width of table (Twips).

    sal_uInt16 m_nRows;                   ///< Row count.
    sal_uInt16 m_nCols;                   ///< Column count.

    sal_uInt16 m_nLeftMargin;             ///< Space to left margin (from paragraph).
    sal_uInt16 m_nRightMargin;            ///< Space to left margin (from paragraph).

    sal_uInt16 m_nInhAbsLeftSpace;        ///< Space inherited from surrounding box
    sal_uInt16 m_nInhAbsRightSpace;       ///< that was added to boxes.

    sal_uInt16 m_nRelLeftFill;            ///< Width of boxes relative to alignment
    sal_uInt16 m_nRelRightFill;           ///< of tables in tables.

    sal_uInt16 m_nRelTabWidth;            ///< Relative width of table.

    sal_uInt16 m_nWidthOption;            ///< Width of table (in Twips oder %).
    sal_uInt16 m_nCellPadding;            ///< Space to contents (in Twips).
    sal_uInt16 m_nCellSpacing;            ///< Cell spacing (in Twips).
    sal_uInt16 m_nBorder;                 /** Line strength of outer border, or rather the
                                        space needed for it as calculated by Netscape. */

    sal_uInt16 m_nLeftBorderWidth;
    sal_uInt16 m_nRightBorderWidth;
    sal_uInt16 m_nInhLeftBorderWidth;
    sal_uInt16 m_nInhRightBorderWidth;
    sal_uInt16 m_nBorderWidth;

    sal_uInt16 m_nDelayedResizeAbsAvail;  ///< Param for delayed Resize.
    sal_uInt16 m_nLastResizeAbsAvail;

    sal_uInt8 m_nPass1Done;               ///< Reference-values for
    sal_uInt8 m_nWidthSet;                ///< the runs through loop.

    SvxAdjust m_eTableAdjust;             ///< Alignment of table.

    bool m_bColsOption : 1;           ///< Table has a COLS-option.
    bool m_bColTags : 1;              ///< Tabelle has COL/COLGRP-tags.
    bool m_bPrcWidthOption : 1;       ///< Width is given in percent.
    bool m_bUseRelWidth : 1;          ///< SwTable gets relative width.

    bool m_bMustResize : 1;           ///< Table width must be defined.
    bool m_bExportable : 1;           ///< Layout may be used for export.
    bool m_bBordersChanged : 1;       ///< Borders have been changed.
    bool m_bMayBeInFlyFrame : 1;      ///< Table could be within frame.

    bool m_bDelayedResizeRecalc : 1;  ///< Param for delayed Resize.
    bool m_bMustNotResize : 1;        ///< Table may not be resized.
    bool m_bMustNotRecalc : 1;        ///< Table may not be adapted to its contents.

    void AddBorderWidth( sal_uLong &rMin, sal_uLong &rMax, sal_uLong& rAbsMin,
                         sal_uInt16 nCol, sal_uInt16 nColSpan,
                         bool bSwBorders=true ) const;
    void SetBoxWidth( SwTableBox *pBox, sal_uInt16 nCol, sal_uInt16 nColSpan ) const;

    const SwStartNode *GetAnyBoxStartNode() const;
    SwFrameFormat *FindFlyFrameFormat() const;
    const SwDoc *GetDoc() const { return GetAnyBoxStartNode()->GetDoc(); }

    void ClearPass1Info() { m_nMin = m_nMax = 0; }

    void _Resize( sal_uInt16 nAbsAvail, bool bRecalc=false );

    DECL_LINK_TYPED( DelayedResize_Impl, Timer*, void );

    static sal_uInt16 GetBrowseWidthByVisArea( const SwDoc& rDoc );
public:

    SwHTMLTableLayout( const SwTable *pSwTable,
                       sal_uInt16 nRows, sal_uInt16 nCols, bool bColsOpt, bool ColTgs,
                       sal_uInt16 nWidth, bool bPrcWidth, sal_uInt16 nBorderOpt,
                       sal_uInt16 nCellPad, sal_uInt16 nCellSp, SvxAdjust eAdjust,
                       sal_uInt16 nLMargin, sal_uInt16 nRMargin, sal_uInt16 nBWidth,
                       sal_uInt16 nLeftBWidth, sal_uInt16 nRightBWidth,
                       sal_uInt16 nInhLeftBWidth, sal_uInt16 nInhRightBWidth );

    ~SwHTMLTableLayout();

    sal_uInt16 GetLeftCellSpace( sal_uInt16 nCol, sal_uInt16 nColSpan,
                             bool bSwBorders=true ) const;
    sal_uInt16 GetRightCellSpace( sal_uInt16 nCol, sal_uInt16 nColSpan,
                              bool bSwBorders=true ) const;
    inline sal_uInt16 GetInhCellSpace( sal_uInt16 nCol, sal_uInt16 nColSpan ) const;

    inline void SetInhBorderWidths( sal_uInt16 nLeft, sal_uInt16 nRight );

    void GetAvail( sal_uInt16 nCol, sal_uInt16 nColSpan, sal_uInt16& rAbsAvail,
                   sal_uInt16& rRelAvail ) const;

    void AutoLayoutPass1();
    void AutoLayoutPass2( sal_uInt16 nAbsAvail, sal_uInt16 nRelAvail,
                          sal_uInt16 nAbsLeftSpace, sal_uInt16 nAbsRightSpace,
                          sal_uInt16 nParentInhSpace );
    void SetWidths( bool bCallPass2=false, sal_uInt16 nAbsAvail=0,
                    sal_uInt16 nRelAvail=0, sal_uInt16 nAbsLeftSpace=0,
                    sal_uInt16 nAbsRightSpace=0,
                    sal_uInt16 nParentInhSpace=0 );

    inline SwHTMLTableLayoutColumn *GetColumn( sal_uInt16 nCol ) const;
    inline void SetColumn( SwHTMLTableLayoutColumn *pCol, sal_uInt16 nCol );

    inline SwHTMLTableLayoutCell *GetCell( sal_uInt16 nRow, sal_uInt16 nCol ) const;
    inline void SetCell( SwHTMLTableLayoutCell *pCell, sal_uInt16 nRow, sal_uInt16 nCol );

    void SetLeftFillerBox( SwTableBox *pBox ) { m_pLeftFillerBox = pBox; }
    void SetRightFillerBox( SwTableBox *pBox ) { m_pRightFillerBox = pBox; }

    sal_uLong GetMin() const { return m_nMin; }
    sal_uLong GetMax() const { return m_nMax; }
    sal_uInt16 GetRelLeftFill() const { return m_nRelLeftFill; }
    sal_uInt16 GetRelRightFill() const { return m_nRelRightFill; }

    inline long GetBrowseWidthMin() const;

    bool HasColsOption() const { return m_bColsOption; }
    bool HasColTags() const { return m_bColTags; }

    bool IsTopTable() const  { return m_pSwTable != nullptr; }

    void SetMustResize( bool bSet ) { m_bMustResize = bSet; }
    void SetMustNotResize( bool bSet ) { m_bMustNotResize = bSet; }
    void SetMustNotRecalc( bool bSet ) { m_bMustNotRecalc = bSet; }

    /** Recalculation of table widths for available width that has been passed.
     - If bRecalc is set, contents of boxes are included into calculation.
     - If bForce is set, table will be recalculated even if this was
       disallowed by SetMustNotResize.
     - If nDelay > 0 the calculation is delayed accordingly. Resizing calls
       occurring during delay-time are ignored, but the delay may be counted
       under certain circumstances.
     - If nDelay == HTMLTABLE_RESIZE_NOW, resize immediately and do not
       consider any resize-calls that might possibly be in order.
     - The return value indicates whether the table has been changed. */
    bool Resize( sal_uInt16 nAbsAvail, bool bRecalc=false, bool bForce=false,
                 sal_uLong nDelay=0 );

    void BordersChanged( sal_uInt16 nAbsAvail, bool bRecalc=false );

    /** Calculate available width. This works only if a layout or a
     SwViewShell exists. Otherwise returns 0.
     This is needed by HTML-filter because it doesn't have access to the layout.) */
    static sal_uInt16 GetBrowseWidth( const SwDoc& rDoc );

    /// Calculates available width by table-frame.
    sal_uInt16 GetBrowseWidthByTabFrm( const SwTabFrm& rTabFrm ) const;

    /** Calculates available width by the table-frame or
     static GetBrowseWidth if no layout exists. */
    sal_uInt16 GetBrowseWidthByTable( const SwDoc& rDoc ) const;

    /// For Export.
    sal_uInt16 GetWidthOption() const { return m_nWidthOption; }
    bool   HasPrcWidthOption() const { return m_bPrcWidthOption; }

    sal_uInt16 GetCellPadding() const { return m_nCellPadding; }
    sal_uInt16 GetCellSpacing() const { return m_nCellSpacing; }
    sal_uInt16 GetBorder() const { return m_nBorder; }

    sal_uInt16 GetRowCount() const { return m_nRows; }
    sal_uInt16 GetColCount() const { return m_nCols; }

    void SetExportable( bool bSet ) { m_bExportable = bSet; }
    bool IsExportable() const { return m_bExportable; }

    bool HaveBordersChanged() const { return m_bBordersChanged; }

    void SetMayBeInFlyFrame( bool bSet ) { m_bMayBeInFlyFrame = bSet; }
    bool MayBeInFlyFrame() const { return m_bMayBeInFlyFrame; }
};

inline void SwHTMLTableLayoutCell::SetProtected()
{
    nRowSpan = 1;
    nColSpan = 1;

    pContents = nullptr;
}

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

inline void SwHTMLTableLayoutColumn::ClearPass1Info( bool bWidthOpt )
{
    nMinNoAlign = nMaxNoAlign = nAbsMinNoAlign = MINLAY;
    nMin = nMax = 0;
    if( bWidthOpt )
    {
        nWidthOption = 0;
        bRelWidthOption = false;
    }
}

inline void SwHTMLTableLayoutColumn::MergeCellWidthOption(
    sal_uInt16 nWidth, bool bRel )
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

inline sal_uInt16 SwHTMLTableLayout::GetInhCellSpace( sal_uInt16 nCol,
                                                  sal_uInt16 nColSpan ) const
{
    sal_uInt16 nSpace = 0;
    if( nCol==0 )
        nSpace = nSpace + m_nInhAbsLeftSpace;
    if( nCol+nColSpan==m_nCols )
        nSpace = nSpace + m_nInhAbsRightSpace;

    return nSpace;
}

inline SwHTMLTableLayoutColumn *SwHTMLTableLayout::GetColumn( sal_uInt16 nCol ) const
{
    return m_aColumns[nCol];
}

inline void SwHTMLTableLayoutColumn::SetWidthOption(
    sal_uInt16 nWidth, bool bRelWidth, bool bTest )
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
    m_aColumns[nCol] = pCol;
}

inline SwHTMLTableLayoutCell *SwHTMLTableLayout::GetCell( sal_uInt16 nRow, sal_uInt16 nCol ) const
{
    return m_aCells[nRow*m_nCols+nCol];
}

inline void SwHTMLTableLayout::SetCell( SwHTMLTableLayoutCell *pCell,
                               sal_uInt16 nRow, sal_uInt16 nCol )
{
    m_aCells[nRow*m_nCols+nCol] = pCell;
}

inline long SwHTMLTableLayout::GetBrowseWidthMin() const
{
    return (long)( (!m_nWidthOption || m_bPrcWidthOption) ? m_nMin : m_nRelTabWidth );
}

void SwHTMLTableLayout::SetInhBorderWidths( sal_uInt16 nLeft, sal_uInt16 nRight )
{
    m_nInhLeftBorderWidth = nLeft;
    m_nInhRightBorderWidth = nRight;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
