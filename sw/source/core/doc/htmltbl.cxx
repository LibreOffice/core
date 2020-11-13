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
#include <sal/log.hxx>

#include <algorithm>
#include <memory>

#include <fmtornt.hxx>
#include <fmtfsize.hxx>
#include <frmfmt.hxx>
#include <ndtxt.hxx>
#include <doc.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <swtable.hxx>
#include <rootfrm.hxx>
#include <flyfrm.hxx>
#include <poolfmt.hxx>
#include <viewsh.hxx>
#include <tabfrm.hxx>
#include <viewopt.hxx>
#include <htmltbl.hxx>
#include <calbck.hxx>
#include <o3tl/numeric.hxx>
#include <osl/diagnose.h>
#ifdef DBG_UTIL
#include <tblrwcl.hxx>
#endif

using namespace ::com::sun::star;

constexpr SwTwips COLFUZZY(20);
constexpr SwTwips MAX_TABWIDTH(USHRT_MAX - 2001);

namespace {

class SwHTMLTableLayoutConstraints
{
    sal_uInt16 m_nRow; // start row
    sal_uInt16 m_nCol; // start column
    sal_uInt16 m_nColSpan; // the column's COLSPAN

    std::unique_ptr<SwHTMLTableLayoutConstraints> m_pNext; // the next constraint

    SwTwips m_nMinNoAlign, m_nMaxNoAlign; // provisional result of AL-Pass 1

public:
    SwHTMLTableLayoutConstraints( SwTwips nMin, SwTwips nMax, sal_uInt16 nRow,
                                sal_uInt16 nCol, sal_uInt16 nColSp );

    SwTwips GetMinNoAlign() const { return m_nMinNoAlign; }
    SwTwips GetMaxNoAlign() const { return m_nMaxNoAlign; }

    SwHTMLTableLayoutConstraints *InsertNext( SwHTMLTableLayoutConstraints *pNxt );
    SwHTMLTableLayoutConstraints* GetNext() const { return m_pNext.get(); }

    sal_uInt16 GetColSpan() const { return m_nColSpan; }
    sal_uInt16 GetColumn() const { return m_nCol; }
};

}

SwHTMLTableLayoutCnts::SwHTMLTableLayoutCnts(const SwStartNode *pSttNd,
                                             std::shared_ptr<SwHTMLTableLayout> const& rTab,
                                             bool bNoBrTag,
                                             std::shared_ptr<SwHTMLTableLayoutCnts> const& rNxt ) :
    m_xNext( rNxt ), m_pBox( nullptr ), m_xTable( rTab ), m_pStartNode( pSttNd ),
    m_nPass1Done( 0 ), m_nWidthSet( 0 ), m_bNoBreakTag( bNoBrTag )
{}

const SwStartNode *SwHTMLTableLayoutCnts::GetStartNode() const
{
    return m_pBox ? m_pBox->GetSttNd() : m_pStartNode;
}

SwHTMLTableLayoutCell::SwHTMLTableLayoutCell(std::shared_ptr<SwHTMLTableLayoutCnts> const& rCnts,
                                          sal_uInt16 nRSpan, sal_uInt16 nCSpan,
                                          sal_uInt16 nWidth, bool bPercentWidth,
                                          bool bNWrapOpt ) :
    m_xContents(rCnts),
    m_nRowSpan( nRSpan ), m_nColSpan( nCSpan ),
    m_nWidthOption( nWidth ), m_bPercentWidthOption( bPercentWidth ),
    m_bNoWrapOption( bNWrapOpt )
{}

SwHTMLTableLayoutColumn::SwHTMLTableLayoutColumn( sal_uInt16 nWidth,
                                                  bool bRelWidth,
                                                  bool bLBorder ) :
    m_nMinNoAlign(MINLAY), m_nMaxNoAlign(MINLAY), m_nAbsMinNoAlign(MINLAY),
    m_nMin(0), m_nMax(0),
    m_nAbsColWidth(0), m_nRelColWidth(0),
    m_nWidthOption( nWidth ), m_bRelWidthOption( bRelWidth ),
    m_bLeftBorder( bLBorder )
{}

SwHTMLTableLayoutConstraints::SwHTMLTableLayoutConstraints(SwTwips nMin, SwTwips nMax,
                                                           sal_uInt16 nRw, sal_uInt16 nColumn,
                                                           sal_uInt16 nColSp)
    : m_nRow(nRw)
    , m_nCol(nColumn)
    , m_nColSpan(nColSp)
    , m_nMinNoAlign(nMin)
    , m_nMaxNoAlign(nMax)
{}

SwHTMLTableLayoutConstraints *SwHTMLTableLayoutConstraints::InsertNext(
    SwHTMLTableLayoutConstraints *pNxt )
{
    SwHTMLTableLayoutConstraints *pPrev = nullptr;
    SwHTMLTableLayoutConstraints *pConstr = this;
    while( pConstr )
    {
        if (pConstr->m_nRow > pNxt->m_nRow || pConstr->GetColumn() > pNxt->GetColumn())
            break;
        pPrev = pConstr;
        pConstr = pConstr->GetNext();
    }

    if( pPrev )
    {
        pNxt->m_pNext = std::move(pPrev->m_pNext);
        pPrev->m_pNext.reset(pNxt);
        pConstr = this;
    }
    else
    {
        pNxt->m_pNext.reset(this);
        pConstr = pNxt;
    }

    return pConstr;
}

SwHTMLTableLayout::SwHTMLTableLayout( const SwTable * pTable,
                                      sal_uInt16 nRws, sal_uInt16 nCls,
                                      bool bColsOpt, bool bColTgs,
                                      sal_uInt16 nWdth, bool bPercentWdth,
                                      sal_uInt16 nBorderOpt, sal_uInt16 nCellPad,
                                      sal_uInt16 nCellSp, SvxAdjust eAdjust,
                                      sal_uInt16 nLMargin, sal_uInt16 nRMargin,
                                      sal_uInt16 nBWidth, sal_uInt16 nLeftBWidth,
                                      sal_uInt16 nRightBWidth )
    : m_aColumns( nCls )
    , m_aCells( static_cast<size_t>(nRws)*nCls )
    , m_pSwTable( pTable )
    , m_nMin( 0 )
    , m_nMax( 0 )
    , m_nRows( nRws )
    , m_nCols( nCls )
    , m_nLeftMargin( nLMargin )
    , m_nRightMargin( nRMargin )
    , m_nInhAbsLeftSpace( 0 )
    , m_nInhAbsRightSpace( 0 )
    , m_nRelLeftFill( 0 )
    , m_nRelRightFill( 0 )
    , m_nRelTabWidth( 0 )
    , m_nWidthOption( nWdth )
    , m_nCellPadding( nCellPad )
    , m_nCellSpacing( nCellSp )
    , m_nBorder( nBorderOpt )
    , m_nLeftBorderWidth( nLeftBWidth )
    , m_nRightBorderWidth( nRightBWidth )
    , m_nInhLeftBorderWidth( 0 )
    , m_nInhRightBorderWidth( 0 )
    , m_nBorderWidth( nBWidth )
    , m_nDelayedResizeAbsAvail( 0 )
    , m_nLastResizeAbsAvail( 0 )
    , m_nPass1Done( 0 )
    , m_nWidthSet( 0 )
    , m_eTableAdjust( eAdjust )
    , m_bColsOption( bColsOpt )
    , m_bColTags( bColTgs )
    , m_bPercentWidthOption( bPercentWdth )
    , m_bUseRelWidth( false )
    , m_bMustResize( true )
    , m_bExportable( true )
    , m_bBordersChanged( false )
    , m_bMayBeInFlyFrame( false )
    , m_bDelayedResizeRecalc( false)
    , m_bMustNotResize( false )
    , m_bMustNotRecalc( false )
{
    m_aResizeTimer.SetInvokeHandler( LINK( this, SwHTMLTableLayout,
                                             DelayedResize_Impl ) );
}

SwHTMLTableLayout::~SwHTMLTableLayout()
{
}

/// The border widths are calculated like in Netscape:
/// Outer border: BORDER + CELLSPACING + CELLPADDING
/// Inner border: CELLSPACING + CELLPADDING
/// However, we respect the border widths in SW if bSwBorders is set,
/// so that we don't wrap wrongly.
/// We also need to respect the distance to the content. Even if
/// only the opposite side has a border.
SwTwips SwHTMLTableLayout::GetLeftCellSpace( sal_uInt16 nCol, sal_uInt16 nColSpan,
                                            bool bSwBorders ) const
{
    SwTwips nSpace = m_nCellSpacing + m_nCellPadding;

    if( nCol == 0 )
    {
        nSpace = nSpace + m_nBorder;

        if( bSwBorders && nSpace < m_nLeftBorderWidth )
            nSpace = m_nLeftBorderWidth;
    }
    else if( bSwBorders )
    {
        if( GetColumn(nCol)->HasLeftBorder() )
        {
            if( nSpace < m_nBorderWidth )
                nSpace = m_nBorderWidth;
        }
        else if( nCol+nColSpan == m_nCols && m_nRightBorderWidth &&
                 nSpace < MIN_BORDER_DIST )
        {
            OSL_ENSURE( !m_nCellPadding, "GetLeftCellSpace: CELLPADDING!=0" );
            // If the opposite side has a border we need to respect at
            // least the minimum distance to the content.
            // Additionally, we could also use nCellPadding for this.
            nSpace = MIN_BORDER_DIST;
        }
    }

    return nSpace;
}

SwTwips SwHTMLTableLayout::GetRightCellSpace( sal_uInt16 nCol, sal_uInt16 nColSpan,
                                             bool bSwBorders ) const
{
    SwTwips nSpace = m_nCellPadding;

    if( nCol+nColSpan == m_nCols )
    {
        nSpace += m_nBorder + m_nCellSpacing;
        if( bSwBorders && nSpace < m_nRightBorderWidth )
            nSpace = m_nRightBorderWidth;
    }
    else if( bSwBorders && GetColumn(nCol)->HasLeftBorder() &&
             nSpace < MIN_BORDER_DIST )
    {
        OSL_ENSURE( !m_nCellPadding, "GetRightCellSpace: CELLPADDING!=0" );
        // If the opposite side has a border we need to respect at
        // least the minimum distance to the content.
        // Additionally, we could also use nCellPadding for this.
        nSpace = MIN_BORDER_DIST;
    }

    return nSpace;
}

void SwHTMLTableLayout::AddBorderWidth( SwTwips &rMin, SwTwips &rMax,
                                        SwTwips &rAbsMin,
                                        sal_uInt16 nCol, sal_uInt16 nColSpan,
                                        bool bSwBorders ) const
{
    SwTwips nAdd = GetLeftCellSpace( nCol, nColSpan, bSwBorders ) +
                 GetRightCellSpace( nCol, nColSpan, bSwBorders );

    rMin += nAdd;
    rMax += nAdd;
    rAbsMin += nAdd;
}

void SwHTMLTableLayout::SetBoxWidth( SwTableBox *pBox, sal_uInt16 nCol,
                             sal_uInt16 nColSpan ) const
{
    SwFrameFormat *pFrameFormat = pBox->GetFrameFormat();

    // calculate the box's width
    SwTwips nFrameWidth(0);
    while( nColSpan-- )
        nFrameWidth += GetColumn( nCol++ )->GetRelColWidth();

    // and reset
    pFrameFormat->SetFormatAttr( SwFormatFrameSize( SwFrameSize::Variable, nFrameWidth, SwTwips(0) ));
}

void SwHTMLTableLayout::GetAvail( sal_uInt16 nCol, sal_uInt16 nColSpan,
                                  SwTwips& rAbsAvail, SwTwips& rRelAvail ) const
{
    rAbsAvail = SwTwips(0);
    rRelAvail = SwTwips(0);
    for( sal_uInt16 i=nCol; i<nCol+nColSpan;i++ )
    {
        const SwHTMLTableLayoutColumn *pColumn = GetColumn(i);
        rAbsAvail = rAbsAvail + pColumn->GetAbsColWidth();
        rRelAvail = rRelAvail + pColumn->GetRelColWidth();
    }
}

SwTwips SwHTMLTableLayout::GetBrowseWidthByVisArea( const SwDoc& rDoc )
{
    SwViewShell const *pVSh = rDoc.getIDocumentLayoutAccess().GetCurrentViewShell();
    if( pVSh )
    {
        return pVSh->GetBrowseWidth();
    }

    return SwTwips(0);
}

SwTwips SwHTMLTableLayout::GetBrowseWidth( const SwDoc& rDoc )
{
    // If we have a layout, we can get the width from there.
    const SwRootFrame *pRootFrame = rDoc.getIDocumentLayoutAccess().GetCurrentLayout();
    if( pRootFrame )
    {
        const SwFrame *pPageFrame = pRootFrame->GetLower();
        if( pPageFrame )
            return pPageFrame->getFramePrintArea().Width();
    }

    // #i91658#
    // Assertion removed which state that no browse width is available.
    // Investigation reveals that all calls can handle the case that no browse
    // width is provided.
    return GetBrowseWidthByVisArea( rDoc );
}

SwTwips SwHTMLTableLayout::GetBrowseWidthByTabFrame(
    const SwTabFrame& rTabFrame ) const
{
    SwTwips nWidth(0);

    const SwFrame *pUpper = rTabFrame.GetUpper();
    if( MayBeInFlyFrame() && pUpper->IsFlyFrame() &&
        static_cast<const SwFlyFrame *>(pUpper)->GetAnchorFrame() )
    {
        // If the table is located within a self-created frame, the anchor's
        // width is relevant not the frame's width.
        // For paragraph-bound frames we don't respect paragraph indents.
        const SwFrame *pAnchor = static_cast<const SwFlyFrame *>(pUpper)->GetAnchorFrame();
        if( pAnchor->IsTextFrame() )
            nWidth = pAnchor->getFrameArea().Width();
        else
            nWidth = pAnchor->getFramePrintArea().Width();
    }
    else
    {
        nWidth = pUpper->getFramePrintArea().Width();
    }

    SwTwips nUpperDummy(0);
    SwTwips nRightOffset(0),
            nLeftOffset(0);
    rTabFrame.CalcFlyOffsets(nUpperDummy, nLeftOffset, nRightOffset, nullptr);
    nWidth -= (nLeftOffset + nRightOffset);

    return nWidth;
}

SwTwips SwHTMLTableLayout::GetBrowseWidthByTable( const SwDoc& rDoc ) const
{
    SwTwips nBrowseWidth(0);
    SwTabFrame* pFrame = SwIterator<SwTabFrame,SwFormat>( *m_pSwTable->GetFrameFormat() ).First();
    if( pFrame )
    {
        nBrowseWidth = GetBrowseWidthByTabFrame( *pFrame );
    }
    else
    {
        nBrowseWidth = SwHTMLTableLayout::GetBrowseWidth( rDoc );
    }

    return nBrowseWidth;
}

const SwStartNode *SwHTMLTableLayout::GetAnyBoxStartNode() const
{
    const SwStartNode *pBoxSttNd;

    const SwTableBox* pBox = m_pSwTable->GetTabLines()[0]->GetTabBoxes()[0];
    while( nullptr == (pBoxSttNd = pBox->GetSttNd()) )
    {
        OSL_ENSURE( !pBox->GetTabLines().empty(),
                "Box without start node and lines" );
        OSL_ENSURE( !pBox->GetTabLines().front()->GetTabBoxes().empty(),
                "Line without boxes" );
        pBox = pBox->GetTabLines().front()->GetTabBoxes().front();
    }

    return pBoxSttNd;
}

SwFrameFormat *SwHTMLTableLayout::FindFlyFrameFormat() const
{
    const SwTableNode *pTableNd = GetAnyBoxStartNode()->FindTableNode();
    OSL_ENSURE( pTableNd, "No Table-Node?" );
    return pTableNd->GetFlyFormat();
}

static void lcl_GetMinMaxSize( SwTwips& rMinNoAlignCnts, SwTwips& rMaxNoAlignCnts,
                        SwTwips& rAbsMinNoAlignCnts,
                        SwTextNode const *pTextNd, sal_uLong nIdx, bool bNoBreak )
{
    pTextNd->GetMinMaxSize( nIdx, rMinNoAlignCnts, rMaxNoAlignCnts,
                           rAbsMinNoAlignCnts );
    OSL_ENSURE( rAbsMinNoAlignCnts <= rMinNoAlignCnts,
            "GetMinMaxSize: absmin > min" );
    OSL_ENSURE( rMinNoAlignCnts <= rMaxNoAlignCnts,
            "GetMinMaxSize: max > min" );

    // The maximal width for a <PRE> paragraph is the minimal width
    const SwFormatColl *pColl = &pTextNd->GetAnyFormatColl();
    while( pColl && !pColl->IsDefault() &&
            (USER_FMT & pColl->GetPoolFormatId()) )
    {
        pColl = static_cast<const SwFormatColl *>(pColl->DerivedFrom());
    }

    // <NOBR> in the whole cell apply to text but not to tables.
    // Netscape only considers this for graphics.
    if( (pColl && RES_POOLCOLL_HTML_PRE==pColl->GetPoolFormatId()) || bNoBreak )
    {
        rMinNoAlignCnts = rMaxNoAlignCnts;
        rAbsMinNoAlignCnts = rMaxNoAlignCnts;
    }
}

void SwHTMLTableLayout::AutoLayoutPass1()
{
    m_nPass1Done++;

    m_nMin = m_nMax = SwTwips(0); // clear pass1 info

    bool bFixRelWidths = false;
    sal_uInt16 i;

    std::unique_ptr<SwHTMLTableLayoutConstraints> xConstraints;

    for( i=0; i<m_nCols; i++ )
    {
        SwHTMLTableLayoutColumn *pColumn = GetColumn( i );
        pColumn->ClearPass1Info( !HasColTags() );
        sal_uInt16 nMinColSpan = USHRT_MAX; // Column count to which the calculated width refers to
        sal_uInt16 nColSkip = USHRT_MAX;    // How many columns need to be skipped

        for( sal_uInt16 j=0; j<m_nRows; j++ )
        {
            SwHTMLTableLayoutCell *pCell = GetCell(j,i);
            SwHTMLTableLayoutCnts *pCnts = pCell->GetContents().get();

            // We need to examine all rows in order to
            // get the column that should be calculated next.
            sal_uInt16 nColSpan = pCell->GetColSpan();
            if( nColSpan < nColSkip )
                nColSkip = nColSpan;

            if( !pCnts || !pCnts->IsPass1Done(m_nPass1Done) )
            {
                // The cell is empty or it's content was not edited
                if( nColSpan < nMinColSpan )
                    nMinColSpan = nColSpan;

                SwTwips nMinNoAlignCell(0);
                SwTwips nMaxNoAlignCell(0);
                SwTwips nAbsMinNoAlignCell(0);
                SwTwips nMaxTableCell(0);
                SwTwips nAbsMinTableCell(0);

                while( pCnts )
                {
                    const SwStartNode *pSttNd = pCnts->GetStartNode();
                    if( pSttNd )
                    {
                        const SwDoc& rDoc = pSttNd->GetDoc();
                        sal_uLong nIdx = pSttNd->GetIndex();
                        while (!rDoc.GetNodes()[nIdx]->IsEndNode())
                        {
                            SwTextNode *pTextNd = (rDoc.GetNodes()[nIdx])->GetTextNode();
                            if( pTextNd )
                            {
                                SwTwips nMinNoAlignCnts(0);
                                SwTwips nMaxNoAlignCnts(0);
                                SwTwips nAbsMinNoAlignCnts(0);

                                lcl_GetMinMaxSize( nMinNoAlignCnts,
                                                   nMaxNoAlignCnts,
                                                   nAbsMinNoAlignCnts,
                                                   pTextNd, nIdx,
                                                   pCnts->HasNoBreakTag() );

                                if( nMinNoAlignCnts > nMinNoAlignCell )
                                    nMinNoAlignCell = nMinNoAlignCnts;
                                if( nMaxNoAlignCnts > nMaxNoAlignCell )
                                    nMaxNoAlignCell = nMaxNoAlignCnts;
                                if( nAbsMinNoAlignCnts > nAbsMinNoAlignCell )
                                    nAbsMinNoAlignCell = nAbsMinNoAlignCnts;
                            }
                            else
                            {
                                SwTableNode *pTabNd = (rDoc.GetNodes()[nIdx])->GetTableNode();
                                if( pTabNd )
                                {
                                    SwHTMLTableLayout *pChild = pTabNd->GetTable().GetHTMLTableLayout();
                                    if( pChild )
                                    {
                                        pChild->AutoLayoutPass1();
                                        SwTwips nMaxTableCnts = pChild->m_nMax;
                                        SwTwips nAbsMinTableCnts = pChild->m_nMin;

                                        // A fixed table width is taken over as minimum and
                                        // maximum at the same time
                                        if( !pChild->m_bPercentWidthOption && pChild->m_nWidthOption )
                                        {
                                            SwTwips nTabWidth = pChild->m_nWidthOption;
                                            if( nTabWidth >= nAbsMinTableCnts  )
                                            {
                                                nMaxTableCnts = nTabWidth;
                                                nAbsMinTableCnts = nTabWidth;
                                            }
                                            else
                                            {
                                                nMaxTableCnts = nAbsMinTableCnts;
                                            }
                                        }

                                        if( nMaxTableCnts > nMaxTableCell )
                                            nMaxTableCell = nMaxTableCnts;
                                        if( nAbsMinTableCnts > nAbsMinTableCell )
                                            nAbsMinTableCell = nAbsMinTableCnts;
                                    }
                                    nIdx = pTabNd->EndOfSectionNode()->GetIndex();
                                }
                            }
                            nIdx++;
                        }
                    }
                    else if (SwHTMLTableLayout *pChild = pCnts->GetTable())
                    {
                        OSL_ENSURE( false, "Sub tables in HTML import?" );
                        pChild->AutoLayoutPass1();
                        SwTwips nMaxTableCnts = pChild->m_nMax;
                        SwTwips nAbsMinTableCnts = pChild->m_nMin;

                        // A fixed table width is taken over as minimum and
                        // maximum at the same time
                        if( !pChild->m_bPercentWidthOption && pChild->m_nWidthOption )
                        {
                            SwTwips nTabWidth = pChild->m_nWidthOption;
                            if( nTabWidth >= nAbsMinTableCnts  )
                            {
                                nMaxTableCnts = nTabWidth;
                                nAbsMinTableCnts = nTabWidth;
                            }
                            else
                            {
                                nMaxTableCnts = nAbsMinTableCnts;
                            }
                        }

                        if( nMaxTableCnts > nMaxTableCell )
                            nMaxTableCell = nMaxTableCnts;
                        if( nAbsMinTableCnts > nAbsMinTableCell )
                            nAbsMinTableCell = nAbsMinTableCnts;
                    }
                    pCnts->SetPass1Done( m_nPass1Done );
                    pCnts = pCnts->GetNext().get();
                }

// This code previously came after AddBorderWidth
                // If a table's width is wider in a cell than what we've calculated
                // for the other content we need to use the table's width.
                if( nMaxTableCell > nMaxNoAlignCell )
                    nMaxNoAlignCell = nMaxTableCell;
                if( nAbsMinTableCell > nAbsMinNoAlignCell )
                {
                    nAbsMinNoAlignCell = nAbsMinTableCell;
                    if( nMinNoAlignCell < nAbsMinNoAlignCell )
                        nMinNoAlignCell = nAbsMinNoAlignCell;
                    if( nMaxNoAlignCell < nMinNoAlignCell )
                        nMaxNoAlignCell = nMinNoAlignCell;
                }
// This code previously came after AddBorderWidth

                bool bRelWidth = pCell->IsPercentWidthOption();
                SwTwips nWidth = pCell->GetWidthOption();

                // A NOWRAP option applies to text and tables, but is
                // not applied for fixed cell width.
                // Instead, the stated cell width behaves like a minimal
                // width.
                if( pCell->HasNoWrapOption() )
                {
                    if( nWidth==SwTwips(0) || bRelWidth )
                    {
                        nMinNoAlignCell = nMaxNoAlignCell;
                        nAbsMinNoAlignCell = nMaxNoAlignCell;
                    }
                    else
                    {
                        if( nWidth>nMinNoAlignCell )
                            nMinNoAlignCell = nWidth;
                        if( nWidth>nAbsMinNoAlignCell )
                            nAbsMinNoAlignCell = nWidth;
                    }
                }

                // Respect minimum width for content
                if( nMinNoAlignCell < MINLAY )
                    nMinNoAlignCell = MINLAY;
                if( nMaxNoAlignCell < MINLAY )
                    nMaxNoAlignCell = MINLAY;
                if( nAbsMinNoAlignCell < MINLAY )
                    nAbsMinNoAlignCell = MINLAY;

                // Respect the border and distance to the content
                AddBorderWidth( nMinNoAlignCell, nMaxNoAlignCell,
                                nAbsMinNoAlignCell, i, nColSpan );

                if( 1==nColSpan )
                {
                    // take over the values directly
                    pColumn->MergeMinMaxNoAlign( nMinNoAlignCell,
                                                 nMaxNoAlignCell,
                                                 nAbsMinNoAlignCell );

                    // the widest WIDTH wins
                    if( !HasColTags() )
                        pColumn->MergeCellWidthOption( nWidth, bRelWidth );
                }
                else
                {
                    // Process the data line by line from left to right at the end

                    // When which values is taken over will be explained further down.
                    if( !HasColTags() && nWidth && !bRelWidth )
                    {
                        SwTwips nAbsWidth = nWidth, nDummy = SwTwips(0), nDummy2 = SwTwips(0);
                        AddBorderWidth( nAbsWidth, nDummy, nDummy2,
                                        i, nColSpan, false );

                        if( nAbsWidth >= nMinNoAlignCell )
                        {
                            nMaxNoAlignCell = nAbsWidth;
                            if( HasColsOption() )
                                nMinNoAlignCell = nAbsWidth;
                        }
                        else if( nAbsWidth >= nAbsMinNoAlignCell )
                        {
                            nMaxNoAlignCell = nAbsWidth;
                            nMinNoAlignCell = nAbsWidth;
                        }
                        else
                        {
                            nMaxNoAlignCell = nAbsMinNoAlignCell;
                            nMinNoAlignCell = nAbsMinNoAlignCell;
                        }
                    }
                    else if( HasColsOption() || HasColTags() )
                        nMinNoAlignCell = nAbsMinNoAlignCell;

                    SwHTMLTableLayoutConstraints *pConstr =
                        new SwHTMLTableLayoutConstraints( nMinNoAlignCell,
                            nMaxNoAlignCell, j, i, nColSpan );
                    if (xConstraints)
                    {
                        SwHTMLTableLayoutConstraints* pConstraints = xConstraints->InsertNext(pConstr);
                        xConstraints.release();
                        xConstraints.reset(pConstraints);
                    }
                    else
                        xConstraints.reset(pConstr);
                }
            }
        }

        OSL_ENSURE( nMinColSpan>0 && nColSkip>0 && nColSkip <= nMinColSpan,
                "Layout pass 1: Columns are being forgotten!" );
        OSL_ENSURE( nMinColSpan!=USHRT_MAX,
                "Layout pass 1: unnecessary pass through the loop or a bug" );

        if( 1==nMinColSpan )
        {
            // There are cells with COLSPAN 1 and therefore also useful
            // values in pColumn

            // Take over values according to the following table (Netscape 4.0 pv 3):

            // WIDTH:           no COLS         COLS

            // none             min = min       min = absmin
            //                  max = max       max = max

            // >= min           min = min       min = width
            //                  max = width     max = width

            // >= absmin        min = width(*)  min = width
            //                  max = width     max = width

            // < absmin         min = absmin    min = absmin
            //                  max = absmin    max = absmin

            // (*) Netscape uses the minimum width without a break before
            //     the last graphic here. We don't have that (yet?),
            //     so we leave it set to width.

            if( pColumn->GetWidthOption() && !pColumn->IsRelWidthOption() )
            {
                // Take over absolute widths as minimal and maximal widths.
                SwTwips nAbsWidth = pColumn->GetWidthOption();
                SwTwips nDummy = SwTwips(0), nDummy2 = SwTwips(0);
                AddBorderWidth( nAbsWidth, nDummy, nDummy2, i, 1, false );

                if( nAbsWidth >= pColumn->GetMinNoAlign() )
                {
                    pColumn->SetMinMax( HasColsOption() ? nAbsWidth
                                                   : pColumn->GetMinNoAlign(),
                                        nAbsWidth );
                }
                else if( nAbsWidth >= pColumn->GetAbsMinNoAlign() )
                {
                    pColumn->SetMinMax( nAbsWidth, nAbsWidth );
                }
                else
                {
                    pColumn->SetMinMax( pColumn->GetAbsMinNoAlign(),
                                        pColumn->GetAbsMinNoAlign() );
                }
            }
            else
            {
                pColumn->SetMinMax( HasColsOption() ? pColumn->GetAbsMinNoAlign()
                                               : pColumn->GetMinNoAlign(),
                                    pColumn->GetMaxNoAlign() );
            }
        }
        else if( USHRT_MAX!=nMinColSpan )
        {
            // Can be anything != 0, because it is altered by the constraints.
            pColumn->SetMinMax( MINLAY, MINLAY );

            // the next columns need not to be processed
            i += (nColSkip-1);
        }

        m_nMin += pColumn->GetMin();
        m_nMax += pColumn->GetMax();
        if (pColumn->IsRelWidthOption()) bFixRelWidths = true;
    }

    // Now process the constraints
    SwHTMLTableLayoutConstraints *pConstr = xConstraints.get();
    while( pConstr )
    {
        // At first we need to process the width in the same way
        // as the column widths
        sal_uInt16 nCol = pConstr->GetColumn();
        sal_uInt16 nColSpan = pConstr->GetColSpan();
        SwTwips nConstrMin = pConstr->GetMinNoAlign();
        SwTwips nConstrMax = pConstr->GetMaxNoAlign();

        // We get the hitherto width of the spanned columns
        SwTwips nColsMin(0);
        SwTwips nColsMax(0);
        for( sal_uInt16 j=nCol; j<nCol+nColSpan; j++ )
        {
            SwHTMLTableLayoutColumn *pColumn = GetColumn( j );
            nColsMin += pColumn->GetMin();
            nColsMax += pColumn->GetMax();
        }

        if( nColsMin<nConstrMin )
        {
            // Proportionately distribute the minimum value to the columns
            SwTwips nMinD = nConstrMin-nColsMin;

            if( nConstrMin > nColsMax )
            {
                // Proportional according to the minimum widths
                sal_uInt16 nEndCol = nCol+nColSpan;
                SwTwips nDiff = nMinD;
                for( sal_uInt16 ic=nCol; ic<nEndCol; ic++ )
                {
                    SwHTMLTableLayoutColumn *pColumn = GetColumn( ic );

                    SwTwips nColMin = pColumn->GetMin();
                    SwTwips nColMax = pColumn->GetMax();

                    m_nMin -= nColMin;
                    SwTwips nAdd;
                    if (ic < nEndCol-1)
                    {
                        if (nColsMin == SwTwips(0))
                            throw o3tl::divide_by_zero();
                        nAdd = (nColMin * nMinD) / nColsMin;
                    }
                    else
                    {
                        nAdd = nDiff;
                    }
                    nColMin += nAdd;
                    m_nMin += nColMin;
                    OSL_ENSURE( nDiff >= nAdd, "Ooops: nDiff is not correct anymore" );
                    nDiff -= nAdd;

                    if( nColMax < nColMin )
                    {
                        m_nMax -= nColMax;
                        nColsMax -= nColMax;
                        nColMax = nColMin;
                        m_nMax += nColMax;
                        nColsMax += nColMax;
                    }

                    pColumn->SetMinMax( nColMin, nColMax );
                }
            }
            else
            {
                // Proportional according to the difference of max and min
                for( sal_uInt16 ic=nCol; ic<nCol+nColSpan; ic++ )
                {
                    SwHTMLTableLayoutColumn *pColumn = GetColumn( ic );

                    SwTwips nDiff = pColumn->GetMax()-pColumn->GetMin();
                    if( nMinD < nDiff )
                        nDiff = nMinD;

                    pColumn->AddToMin( nDiff );

                    OSL_ENSURE( pColumn->GetMax() >= pColumn->GetMin(),
                            "Why is the Column suddenly too narrow?" );

                    m_nMin += nDiff;
                    nMinD -= nDiff;
                }
            }
        }

        if( !HasColTags() && nColsMax<nConstrMax )
        {
            SwTwips nMaxD = nConstrMax-nColsMax;

            for( sal_uInt16 ic=nCol; ic<nCol+nColSpan; ic++ )
            {
                SwHTMLTableLayoutColumn *pColumn = GetColumn( ic );

                m_nMax -= pColumn->GetMax();

                pColumn->AddToMax( (pColumn->GetMax() * nMaxD) / nColsMax );

                m_nMax += pColumn->GetMax();
            }
        }

        pConstr = pConstr->GetNext();
    }

    if( !bFixRelWidths )
        return;

    if( HasColTags() )
    {
        // To adapt the relative widths, in a first step we multiply the
        // minimum width of all affected cells with the relative width
        // of the column.
        // Thus, the width ratio among the columns is correct.

        // Furthermore, a factor is calculated that says by how much the
        // cell has gotten wider than the minimum width.

        // In the second step the calculated widths are divided by this
        // factor.  Thereby a cell's width is preserved and serves as a
        // basis for the other cells.
        // We only change the maximum widths here!

        SwTwips nAbsMin(0);  // absolute minimum width of all widths with relative width
        SwTwips nRel(0);     // sum of all relative widths of all columns
        for( i=0; i<m_nCols; i++ )
        {
            SwHTMLTableLayoutColumn *pColumn = GetColumn( i );
            if( pColumn->IsRelWidthOption() && pColumn->GetWidthOption() )
            {
                nAbsMin += pColumn->GetMin();
                nRel += pColumn->GetWidthOption();
            }
        }

        SwTwips nQuot(LONG_MAX);
        for( i=0; i<m_nCols; i++ )
        {
            SwHTMLTableLayoutColumn *pColumn = GetColumn( i );
            if( pColumn->IsRelWidthOption() )
            {
                m_nMax -= pColumn->GetMax();
                if( pColumn->GetWidthOption() && pColumn->GetMin() )
                {
                    pColumn->SetMax( nAbsMin * pColumn->GetWidthOption() );
                    SwTwips nColQuot = pColumn->GetMax() / pColumn->GetMin();
                    if( nColQuot<nQuot )
                        nQuot = nColQuot;
                }
            }
        }
        OSL_ENSURE( SwTwips(0)==nRel || nQuot!=SwTwips(LONG_MAX),
                "Where did the relative columns go?" );
        for( i=0; i<m_nCols; i++ )
        {
            SwHTMLTableLayoutColumn *pColumn = GetColumn( i );
            if( pColumn->IsRelWidthOption() )
            {
                if( pColumn->GetWidthOption() )
                    pColumn->SetMax( pColumn->GetMax() / nQuot );
                else
                    pColumn->SetMax( pColumn->GetMin() );
                OSL_ENSURE( pColumn->GetMax() >= pColumn->GetMin(),
                        "Maximum column width is lower than the minimum column width" );
                m_nMax += pColumn->GetMax();
            }
        }
    }
    else
    {
        SwTwips nRel(0);        // sum of the relative widths of all columns
        sal_uInt16 nRelCols = 0;    // count of the columns with a relative setting
        SwTwips nRelMax(0);      // fraction of the maximum of this column
        for( i=0; i<m_nCols; i++ )
        {
            OSL_ENSURE( nRel<=SwTwips(100), "relative width of all columns > 100%" );
            SwHTMLTableLayoutColumn *pColumn = GetColumn( i );
            if( pColumn->IsRelWidthOption() && pColumn->GetWidthOption() )
            {
                // Make sure that the relative widths don't go above 100%
                SwTwips nColWidth = pColumn->GetWidthOption();
                if( nRel+nColWidth > SwTwips(100) )
                {
                    nColWidth = SwTwips(100) - nRel;
                    pColumn->SetWidthOption( nColWidth );
                }
                nRelMax += pColumn->GetMax();
                nRel = nRel + nColWidth;
                nRelCols++;
            }
            else if( !pColumn->GetMin() )
            {
                // The column is empty (so it was solely created by
                // COLSPAN) and therefore must not be assigned a % width.
                nRelCols++;
            }
        }

        // If there are percentages left we distribute them to the columns
        // that don't have a width setting. Like in Netscape we distribute
        // the remaining percentages according to the ratio of the maximum
        // width of the affected columns.
        // For the maximum widths we also take the fixed-width columns
        // into account.  Is that correct?
        SwTwips nFixMax(0);
        if( nRel < SwTwips(100) && nRelCols < m_nCols )
        {
            nFixMax = m_nMax - nRelMax;
            SAL_WARN_IF(!nFixMax, "sw.core", "bad fixed width max");
        }
        if (nFixMax)
        {
            SwTwips nRelLeft = SwTwips(100) - nRel;
            for( i=0; i<m_nCols; i++ )
            {
                SwHTMLTableLayoutColumn *pColumn = GetColumn( i );
                if( !pColumn->IsRelWidthOption() &&
                    !pColumn->GetWidthOption() &&
                    pColumn->GetMin() )
                {
                    // the next column gets the rest
                    SwTwips nColWidth = (pColumn->GetMax() * nRelLeft) / nFixMax;
                    pColumn->SetWidthOption( nColWidth );
                }
            }
        }

        // adjust the maximum widths now accordingly
        SwTwips nQuotMax(LONG_MAX);
        SwTwips nOldMax = m_nMax;
        m_nMax = SwTwips(0);
        for( i=0; i<m_nCols; i++ )
        {
            // Columns with a % setting are adapted accordingly.
            // Columns, that
            // - do not have a % setting and are located within a tables
            // with COLS and WIDTH, or
            // - their width is 0%
            // get set to the minimum width.
            SwHTMLTableLayoutColumn *pColumn = GetColumn( i );
            if( pColumn->IsRelWidthOption() && pColumn->GetWidthOption() )
            {
                SwTwips nNewMax;
                SwTwips nColQuotMax;
                if( !m_nWidthOption )
                {
                    nNewMax = nOldMax * pColumn->GetWidthOption();
                    nColQuotMax = nNewMax / pColumn->GetMax();
                }
                else
                {
                    nNewMax = m_nMin * pColumn->GetWidthOption();
                    nColQuotMax = nNewMax / pColumn->GetMin();
                }
                pColumn->SetMax( nNewMax );
                if( nColQuotMax < nQuotMax )
                    nQuotMax = nColQuotMax;
            }
            else if( HasColsOption() || m_nWidthOption ||
                     (pColumn->IsRelWidthOption() &&
                      !pColumn->GetWidthOption()) )
                pColumn->SetMax( pColumn->GetMin() );
        }
        // and divide by the quotient
        SAL_WARN_IF(!nQuotMax, "sw.core", "Where did the relative columns go?");
        for (i = 0; i < m_nCols; ++i)
        {
            SwHTMLTableLayoutColumn *pColumn = GetColumn( i );
            if (pColumn->IsRelWidthOption() && pColumn->GetWidthOption() && nQuotMax)
            {
                pColumn->SetMax( pColumn->GetMax() / nQuotMax );
                OSL_ENSURE( pColumn->GetMax() >= pColumn->GetMin(),
                        "Minimum width is one column bigger than maximum" );
                if( pColumn->GetMax() < pColumn->GetMin() )
                    pColumn->SetMax( pColumn->GetMin() );
            }
            m_nMax += pColumn->GetMax();
        }
    }
}

//TODO: provide documentation
/**

    @param nAbsAvail available space in TWIPS.
    @param nRelAvail available space related to USHRT_MAX or 0
    @param nAbsSpace fraction of nAbsAvail, which is reserved by the surrounding
                     cell for the border and the distance to the paragraph.
*/
void SwHTMLTableLayout::AutoLayoutPass2( SwTwips nAbsAvail, SwTwips nRelAvail,
                                         SwTwips nAbsLeftSpace,
                                         SwTwips nAbsRightSpace,
                                         SwTwips nParentInhAbsSpace )
{
    // For a start we do a lot of plausibility tests

    // An absolute width always has to be passed
    OSL_ENSURE( nAbsAvail, "AutoLayout pass 2: No absolute width given" );

    // A relative width must only be passed for tables within tables (?)
    OSL_ENSURE( IsTopTable() == (nRelAvail==SwTwips(0)),
            "AutoLayout pass 2: Relative width at table in table or the other way around" );

    // The table's minimum width must not be bigger than its maximum width
    OSL_ENSURE( m_nMin<=m_nMax, "AutoLayout pass 2: nMin > nMax" );

    // Remember the available width for which the table was calculated.
    // This is a good place as we pass by here for the initial calculation
    // of the table in the parser and for each Resize_ call.
    m_nLastResizeAbsAvail = nAbsAvail;

    // Step 1: The available space is readjusted for the left/right border,
    // possibly existing filler cells and distances.

    // Distance to the content and border
    SwTwips nAbsLeftFill(0), nAbsRightFill(0);
    if( !IsTopTable() &&
        GetMin() + nAbsLeftSpace + nAbsRightSpace <= nAbsAvail )
    {
        nAbsLeftFill = nAbsLeftSpace;
        nAbsRightFill = nAbsRightSpace;
    }

    // Left and right distance
    if( m_nLeftMargin || m_nRightMargin )
    {
        if( IsTopTable() )
        {
            // For the top table we always respect the borders, because we
            // never go below the table's minimum width.
            nAbsAvail -= (m_nLeftMargin + m_nRightMargin);
        }
        else if( GetMin() + m_nLeftMargin + m_nRightMargin <= nAbsAvail )
        {
            // Else, we only respect the borders if there's space available
            // for them (nMin has already been calculated!)
            nAbsLeftFill = nAbsLeftFill + m_nLeftMargin;
            nAbsRightFill = nAbsRightFill + m_nRightMargin;
        }
    }

    // Read just the available space
    m_nRelLeftFill = SwTwips(0);
    m_nRelRightFill = SwTwips(0);
    if( !IsTopTable() && (nAbsLeftFill>SwTwips(0) || nAbsRightFill) )
    {
        SwTwips nAbsLeftFillL = nAbsLeftFill, nAbsRightFillL = nAbsRightFill;

        m_nRelLeftFill = (nAbsLeftFillL * nRelAvail) / nAbsAvail;
        m_nRelRightFill = (nAbsRightFillL * nRelAvail) / nAbsAvail;

        nAbsAvail -= (nAbsLeftFill + nAbsRightFill);
        if( nRelAvail )
            nRelAvail -= (m_nRelLeftFill + m_nRelRightFill);
    }

    // Step 2: Calculate the absolute table width.
    SwTwips nAbsTabWidth(0);
    m_bUseRelWidth = false;
    if( m_nWidthOption )
    {
        if( m_bPercentWidthOption )
        {
            OSL_ENSURE( m_nWidthOption<=SwTwips(100), "Percentage value too high" );
            if( m_nWidthOption > SwTwips(100) )
                m_nWidthOption = SwTwips(100);

            // The absolute width is equal to the given percentage of
            // the available width.
            // Top tables only get a relative width if the available space
            // is *strictly larger* than the minimum width.

            // CAUTION: We need the "strictly larger" because changing from a
            // relative width to an absolute width by resizing would lead
            // to an infinite loop.

            // Because we do not call resize for tables in frames if the
            // frame has a non-relative width, we cannot play such games.

            // Let's play such games now anyway. We had a graphic in a 1% wide
            // table and it didn't fit in of course.
            nAbsTabWidth = (nAbsAvail * m_nWidthOption) / SwTwips(100);
            if( IsTopTable() &&
                ( /*MayBeInFlyFrame() ||*/ nAbsTabWidth > m_nMin ) )
            {
                nRelAvail = SwTwips(USHRT_MAX);
                m_bUseRelWidth = true;
            }
        }
        else
        {
            nAbsTabWidth = m_nWidthOption;
            if( nAbsTabWidth > MAX_TABWIDTH )
                nAbsTabWidth = MAX_TABWIDTH;

            // Tables within tables must never get wider than the available
            // space.
            if( !IsTopTable() && nAbsTabWidth > nAbsAvail )
                nAbsTabWidth = nAbsAvail;
        }
    }

    OSL_ENSURE( IsTopTable() || nAbsTabWidth<=nAbsAvail,
            "AutoLayout pass 2: nAbsTabWidth > nAbsAvail for table in table" );
    OSL_ENSURE( !nRelAvail || nAbsTabWidth<=nAbsAvail,
            "AutoLayout pass 2: nAbsTabWidth > nAbsAvail for relative width" );

    // Catch for the two asserts above (we never know!)
    if( (!IsTopTable() || nRelAvail>SwTwips(0)) && nAbsTabWidth>nAbsAvail )
        nAbsTabWidth = nAbsAvail;

    // Step 3: Identify the column width and, if applicable, the absolute
    // and relative table widths.
    if( (!IsTopTable() && m_nMin > nAbsAvail) ||
        m_nMin > MAX_TABWIDTH )
    {
        // If
        // - an inner table's minimum is larger than the available space, or
        // - a top table's minimum is larger than USHORT_MAX the table
        // has to be adapted to the available space or USHORT_MAX.
        // We preserve the widths' ratio amongst themselves, however.

        nAbsTabWidth = IsTopTable() ? MAX_TABWIDTH : nAbsAvail;
        m_nRelTabWidth = (nRelAvail ? nRelAvail : nAbsTabWidth );

        // First of all, we check whether we can fit the layout constrains,
        // which are: Every cell's width excluding the borders must be at least
        // MINLAY:

        SwTwips nRealMin(0);
        for( sal_uInt16 i=0; i<m_nCols; i++ )
        {
            SwTwips nRealColMin = MINLAY, nDummy1 = SwTwips(0), nDummy2 = SwTwips(0);
            AddBorderWidth( nRealColMin, nDummy1, nDummy2, i, 1 );
            nRealMin += nRealColMin;
        }
        if( (nRealMin >= nAbsTabWidth) || (nRealMin >= m_nMin) )
        {
            // "Rien ne va plus": we cannot get the minimum column widths
            // the layout wants to have.

            SwTwips nAbs(0);
            SwTwips nRel(0);
            SwHTMLTableLayoutColumn *pColumn;
            for( sal_uInt16 i=0; i<m_nCols-1; i++ )
            {
                pColumn = GetColumn( i );
                SwTwips nColMin = pColumn->GetMin();
                if( nColMin <= SwTwips(USHRT_MAX) )
                {
                    pColumn->SetAbsColWidth( (nColMin * nAbsTabWidth) / m_nMin );
                    pColumn->SetRelColWidth( (nColMin * SwTwips(m_nRelTabWidth)) / m_nMin );
                }
                else
                {
                    double nFraction = double(tools::Long(nColMin)) / tools::Long(m_nMin);
                    pColumn->SetAbsColWidth( SwTwips(tools::Long((nFraction * tools::Long(nAbsTabWidth)))) );
                    pColumn->SetRelColWidth( SwTwips(tools::Long((nFraction * tools::Long(m_nRelTabWidth)))) );
                }

                nAbs = nAbs + pColumn->GetAbsColWidth();
                nRel = nRel + pColumn->GetRelColWidth();
            }
            pColumn = GetColumn( m_nCols-1 );
            pColumn->SetAbsColWidth( nAbsTabWidth - nAbs );
            pColumn->SetRelColWidth( SwTwips(m_nRelTabWidth) - nRel );
        }
        else
        {
            SwTwips nDistAbs = nAbsTabWidth - nRealMin;
            SwTwips nDistRel = m_nRelTabWidth - nRealMin;
            SwTwips nDistMin = m_nMin - nRealMin;
            SwTwips nAbs = SwTwips(0), nRel = SwTwips(0);
            SwHTMLTableLayoutColumn *pColumn;
            for( sal_uInt16 i=0; i<m_nCols-1; i++ )
            {
                pColumn = GetColumn( i );
                SwTwips nColMin = pColumn->GetMin();
                SwTwips nRealColMin = MINLAY, nDummy1 = SwTwips(0), nDummy2 = SwTwips(0);
                AddBorderWidth( nRealColMin, nDummy1, nDummy2, i, 1 );

                if( nColMin <= SwTwips(USHRT_MAX) )
                {
                    pColumn->SetAbsColWidth(
                        (((nColMin-nRealColMin) * nDistAbs) / nDistMin) + nRealColMin );
                    pColumn->SetRelColWidth(
                        (((nColMin-nRealColMin) * nDistRel) / nDistMin) + nRealColMin );
                }
                else
                {
                    double nColMinD = double(tools::Long((nColMin - nRealColMin))) / tools::Long(nDistMin);
                    pColumn->SetAbsColWidth(
                        SwTwips(tools::Long((nColMinD * tools::Long(nDistAbs)) + tools::Long(nRealColMin))) );
                    pColumn->SetRelColWidth(
                        SwTwips(tools::Long((nColMinD * tools::Long(nDistRel)) + tools::Long(nRealColMin))) );
                }

                nAbs = nAbs + pColumn->GetAbsColWidth();
                nRel = nRel + pColumn->GetRelColWidth();
            }
            pColumn = GetColumn( m_nCols-1 );
            pColumn->SetAbsColWidth( nAbsTabWidth - nAbs );
            pColumn->SetRelColWidth( m_nRelTabWidth - nRel );
        }
    }
    else if( m_nMax <= (nAbsTabWidth ? nAbsTabWidth : nAbsAvail) )
    {
        // If
        // - the table has a fixed width and the table's maximum is
        //   smaller, or
        //- the maximum is smaller than the available space,
        // we can take over the maximum as it is. Respectively
        // the table can only be adapted to the fixed width by
        // respecting the maximum.

        // No fixed width, use the maximum.
        if( !nAbsTabWidth )
            nAbsTabWidth = m_nMax;

        // A top table may also get wider then the available space.
        if( nAbsTabWidth > nAbsAvail )
        {
            OSL_ENSURE( IsTopTable(),
                    "Table in table should get wider than the surrounding cell." );
            nAbsAvail = nAbsTabWidth;
        }

        // Only use the relative widths' fraction, that is used for the
        // absolute width.
        SwTwips nAbsTabWidthL = nAbsTabWidth;
        if (nRelAvail)
        {
            if (nAbsAvail == SwTwips(0))
                throw o3tl::divide_by_zero();
            m_nRelTabWidth = (nAbsTabWidthL * nRelAvail) / nAbsAvail;
        }
        else
            m_nRelTabWidth = nAbsTabWidth;

        // Are there columns width a percentage setting and some without one?
        SwTwips nFixMax = m_nMax;
        for( sal_uInt16 i=0; i<m_nCols; i++ )
        {
            const SwHTMLTableLayoutColumn *pColumn = GetColumn( i );
            if( pColumn->IsRelWidthOption() && pColumn->GetWidthOption() > SwTwips(0) )
                nFixMax -= pColumn->GetMax();
        }

        if( nFixMax > SwTwips(0) && nFixMax < m_nMax )
        {
            // Yes, distribute the to-be-distributed space only to the
            // columns with a percentage setting.

            // In this case (and in this case only) there are columns
            // that exactly keep their maximum width, that is they neither
            // get smaller nor wider. When calculating the absolute width
            // from the relative width we can get rounding errors.
            // To correct this, we first make the fixed widths compensate for
            // this error. We then fix the relative widths the same way.

            SwTwips nAbs = SwTwips(0), nRel = SwTwips(0);
            sal_uInt16 nFixedCols = 0;
            sal_uInt16 i;

            for( i = 0; i < m_nCols; i++ )
            {
                SwHTMLTableLayoutColumn *pColumn = GetColumn( i );
                if( !pColumn->IsRelWidthOption() || !pColumn->GetWidthOption() )
                {
                    // The column keeps its width.
                    nFixedCols++;
                    SwTwips nColMax = pColumn->GetMax();
                    pColumn->SetAbsColWidth( nColMax );

                    SwTwips nRelColWidth =
                        (nColMax * m_nRelTabWidth) / nAbsTabWidth;
                    SwTwips nChkWidth =
                        (nRelColWidth * nAbsTabWidth) / m_nRelTabWidth;
                    if( nChkWidth < nColMax )
                        nRelColWidth++;
                    else if( nChkWidth > nColMax )
                        nRelColWidth--;
                    pColumn->SetRelColWidth( nRelColWidth );

                    nAbs = nAbs + nColMax;
                    nRel = nRel + nRelColWidth;
                }
            }

            // The to-be-distributed percentage of the maximum, the
            // relative and absolute widths. Here, nFixMax corresponds
            // to nAbs, so that we could've called it nAbs.
            // The code is, however, more readable like that.
            OSL_ENSURE( nFixMax == nAbs, "Two loops, two sums?" );
            SwTwips nDistMax = m_nMax - nFixMax;
            SwTwips nDistAbsTabWidth = nAbsTabWidth - nAbs;
            SwTwips nDistRelTabWidth = m_nRelTabWidth - nRel;

            for( i=0; i<m_nCols; i++ )
            {
                SwHTMLTableLayoutColumn *pColumn = GetColumn( i );
                if( pColumn->IsRelWidthOption() && pColumn->GetWidthOption() > SwTwips(0) )
                {
                    // The column gets proportionately wider.
                    nFixedCols++;
                    if( nFixedCols == m_nCols )
                    {
                        pColumn->SetAbsColWidth( nAbsTabWidth-nAbs );
                        pColumn->SetRelColWidth( m_nRelTabWidth-nRel );
                    }
                    else
                    {
                        SwTwips nColMax = pColumn->GetMax();
                        pColumn->SetAbsColWidth(
                            (nColMax * nDistAbsTabWidth) / nDistMax );
                        pColumn->SetRelColWidth(
                            (nColMax * nDistRelTabWidth) / nDistMax );
                    }
                    nAbs = nAbs + pColumn->GetAbsColWidth();
                    nRel = nRel + pColumn->GetRelColWidth();
                }
            }
            OSL_ENSURE( m_nCols==nFixedCols, "Missed a column!" );
        }
        else if (m_nCols > 0)
        {
            if (m_nMax == SwTwips(0))
                throw o3tl::divide_by_zero();
            // No. So distribute the space regularly among all columns.
            for (sal_uInt16 i=0; i < m_nCols; ++i)
            {
                SwTwips nColMax = GetColumn( i )->GetMax();
                GetColumn( i )->SetAbsColWidth(
                    (nColMax * nAbsTabWidth) / m_nMax );
                GetColumn( i )->SetRelColWidth(
                    (nColMax * m_nRelTabWidth) / m_nMax );
            }
        }
    }
    else
    {
        // Proportionately distribute the space that extends over the minimum
        // width among the columns.
        if( !nAbsTabWidth )
            nAbsTabWidth = nAbsAvail;
        if( nAbsTabWidth < m_nMin )
            nAbsTabWidth = m_nMin;

        if( nAbsTabWidth > nAbsAvail )
        {
            OSL_ENSURE( IsTopTable(),
                    "A nested table should become wider than the available space." );
            nAbsAvail = nAbsTabWidth;
        }

        SwTwips nAbsTabWidthL = nAbsTabWidth;
        if (nRelAvail)
        {
            if (nAbsAvail == SwTwips(0))
                throw o3tl::divide_by_zero();
            m_nRelTabWidth = (nAbsTabWidthL * nRelAvail) / nAbsAvail;
        }
        else
            m_nRelTabWidth = nAbsTabWidth;
        double nW = tools::Long(nAbsTabWidth - m_nMin);
        double nD = tools::Long(m_nMax==m_nMin ? SwTwips(1) : m_nMax-m_nMin);
        SwTwips nAbs = SwTwips(0), nRel = SwTwips(0);
        for( sal_uInt16 i=0; i<m_nCols-1; i++ )
        {
            double nd = tools::Long(GetColumn( i )->GetMax() - GetColumn( i )->GetMin());
            SwTwips nAbsColWidth = GetColumn( i )->GetMin() + SwTwips(static_cast<tools::Long>((nd*nW)/nD));
            SwTwips nRelColWidth = nRelAvail
                                    ? (nAbsColWidth * m_nRelTabWidth) / nAbsTabWidth
                                    : nAbsColWidth;

            GetColumn( i )->SetAbsColWidth( nAbsColWidth );
            GetColumn( i )->SetRelColWidth( nRelColWidth );
            nAbs = nAbs + nAbsColWidth;
            nRel = nRel + nRelColWidth;
        }
        GetColumn( m_nCols-1 )->SetAbsColWidth( nAbsTabWidth - nAbs );
        GetColumn( m_nCols-1 )->SetRelColWidth( m_nRelTabWidth - nRel );

    }

    // Step 4: For nested tables we can have balancing cells on the
    // left or right. Here we calculate their width.
    m_nInhAbsLeftSpace = SwTwips(0);
    m_nInhAbsRightSpace = SwTwips(0);
    if( IsTopTable() ||
        !(m_nRelLeftFill>SwTwips(0) || m_nRelRightFill>SwTwips(0) || nAbsTabWidth<nAbsAvail) )
        return;

    // Calculate the width of additional cells we use for
    // aligning inner tables.
    SwTwips nAbsDist = nAbsAvail-nAbsTabWidth;
    SwTwips nRelDist = nRelAvail-m_nRelTabWidth;
    SwTwips nParentInhAbsLeftSpace = SwTwips(0), nParentInhAbsRightSpace = SwTwips(0);

    // Calculate the size and position of the additional cells.
    switch( m_eTableAdjust )
    {
    case SvxAdjust::Right:
        nAbsLeftFill = nAbsLeftFill + nAbsDist;
        m_nRelLeftFill = m_nRelLeftFill + nRelDist;
        nParentInhAbsLeftSpace = nParentInhAbsSpace;
        break;
    case SvxAdjust::Center:
        {
            SwTwips nAbsLeftDist = nAbsDist / SwTwips(2);
            nAbsLeftFill = nAbsLeftFill + nAbsLeftDist;
            nAbsRightFill += nAbsDist - nAbsLeftDist;
            SwTwips nRelLeftDist = nRelDist / SwTwips(2);
            m_nRelLeftFill = m_nRelLeftFill + nRelLeftDist;
            m_nRelRightFill += nRelDist - nRelLeftDist;
            nParentInhAbsLeftSpace = nParentInhAbsSpace / SwTwips(2);
            nParentInhAbsRightSpace = nParentInhAbsSpace -
                                      nParentInhAbsLeftSpace;
        }
        break;
    case SvxAdjust::Left:
    default:
        nAbsRightFill = nAbsRightFill + nAbsDist;
        m_nRelRightFill = m_nRelRightFill + nRelDist;
        nParentInhAbsRightSpace = nParentInhAbsSpace;
        break;
    }

    // Filler widths are added to the outer columns, if there are no boxes
    // for them after the first pass (nWidth>0) or their width would become
    // too small or if there are COL tags and the filler width corresponds
    // to the border width.
    // In the last case we probably exported the table ourselves.
    if( m_nRelLeftFill &&
        ( m_nWidthSet>0 || nAbsLeftFill<MINLAY+m_nInhLeftBorderWidth ||
          (HasColTags() && nAbsLeftFill < nAbsLeftSpace+nParentInhAbsLeftSpace+SwTwips(20)) ) )
    {
        SwHTMLTableLayoutColumn *pColumn = GetColumn( 0 );
        pColumn->SetAbsColWidth( pColumn->GetAbsColWidth()+nAbsLeftFill );
        pColumn->SetRelColWidth( pColumn->GetRelColWidth()+m_nRelLeftFill );
        m_nRelLeftFill = SwTwips(0);
        m_nInhAbsLeftSpace = nAbsLeftSpace + nParentInhAbsLeftSpace;
    }
    if( m_nRelRightFill &&
        ( m_nWidthSet>0 || nAbsRightFill<MINLAY+m_nInhRightBorderWidth ||
          (HasColTags() && nAbsRightFill < nAbsRightSpace+nParentInhAbsRightSpace+SwTwips(20)) ) )
    {
        SwHTMLTableLayoutColumn *pColumn = GetColumn( m_nCols-1 );
        pColumn->SetAbsColWidth( pColumn->GetAbsColWidth()+nAbsRightFill );
        pColumn->SetRelColWidth( pColumn->GetRelColWidth()+m_nRelRightFill );
        m_nRelRightFill = SwTwips(0);
        m_nInhAbsRightSpace = nAbsRightSpace + nParentInhAbsRightSpace;
    }
}

static void lcl_ResizeLine( const SwTableLine* pLine, SwTwips *pWidth );

static void lcl_ResizeBox( const SwTableBox* pBox, SwTwips* pWidth )
{
    if( !pBox->GetSttNd() )
    {
        SwTwips nWidth(0);
        for( const SwTableLine *pLine : pBox->GetTabLines() )
            lcl_ResizeLine( pLine, &nWidth );
        pBox->GetFrameFormat()->SetFormatAttr( SwFormatFrameSize( SwFrameSize::Variable, nWidth, SwTwips(0) ));
        *pWidth = *pWidth + nWidth;
    }
    else
    {
        *pWidth = *pWidth + pBox->GetFrameFormat()->GetFrameSize().GetSize().Width();
    }
}

static void lcl_ResizeLine( const SwTableLine* pLine, SwTwips *pWidth )
{
    SwTwips nOldWidth = *pWidth;
    *pWidth = SwTwips(0);
    for( const SwTableBox* pBox : pLine->GetTabBoxes() )
        lcl_ResizeBox(pBox, pWidth );

    SAL_WARN_IF( nOldWidth && std::abs(tools::Long(*pWidth-nOldWidth)) >= tools::Long(COLFUZZY), "sw.core",
                 "A box's rows have all a different length" );
}

void SwHTMLTableLayout::SetWidths( bool bCallPass2, SwTwips nAbsAvail,
                                   SwTwips nRelAvail, SwTwips nAbsLeftSpace,
                                   SwTwips nAbsRightSpace,
                                   SwTwips nParentInhAbsSpace )
{
    // SetWidth must have been passed through once more for every cell in the
    // end.
    m_nWidthSet++;

    // Step 0: If necessary, we call the layout algorithm of Pass2.
    if( bCallPass2 )
        AutoLayoutPass2( nAbsAvail, nRelAvail, nAbsLeftSpace, nAbsRightSpace,
                         nParentInhAbsSpace );

    // Step 1: Set the new width in all content boxes.
    // Because the boxes don't know anything about the HTML table structure,
    // we iterate over the HTML table structure.
    // For tables in tables in tables we call SetWidth recursively.
    for( sal_uInt16 i=0; i<m_nRows; i++ )
    {
        for( sal_uInt16 j=0; j<m_nCols; j++ )
        {
            SwHTMLTableLayoutCell *pCell = GetCell( i, j );

            SwHTMLTableLayoutCnts* pContents = pCell->GetContents().get();
            while( pContents && !pContents->IsWidthSet(m_nWidthSet) )
            {
                SwTableBox *pBox = pContents->GetTableBox();
                if( pBox )
                {
                    SetBoxWidth( pBox, j, pCell->GetColSpan() );
                }
                else if (SwHTMLTableLayout *pTable = pContents->GetTable())
                {
                    SwTwips nAbs = SwTwips(0), nRel = SwTwips(0), nLSpace = SwTwips(0), nRSpace = SwTwips(0),
                           nInhSpace = SwTwips(0);
                    if( bCallPass2 )
                    {
                        sal_uInt16 nColSpan = pCell->GetColSpan();
                        GetAvail( j, nColSpan, nAbs, nRel );
                        nLSpace = GetLeftCellSpace( j, nColSpan );
                        nRSpace = GetRightCellSpace( j, nColSpan );
                        nInhSpace = GetInhCellSpace( j, nColSpan );
                    }
                    pTable->SetWidths( bCallPass2, nAbs, nRel,
                                                    nLSpace, nRSpace,
                                                    nInhSpace );
                }

                pContents->SetWidthSet( m_nWidthSet );
                pContents = pContents->GetNext().get();
            }
        }
    }

    // Step 2: If we have a top table, we adapt the formats of the
    // non-content-boxes. Because they are not known in the HTML table
    // due to garbage collection there, we need the iterate over the
    // whole table.
    // We also adapt the table frame format. For nested tables we set the
    // filler cell's width instead.
    if( !IsTopTable() )
        return;

    SwTwips nCalcTabWidth(0);
    for( const SwTableLine *pLine : m_pSwTable->GetTabLines() )
        lcl_ResizeLine( pLine, &nCalcTabWidth );
    SAL_WARN_IF( std::abs( tools::Long(m_nRelTabWidth-nCalcTabWidth) ) >= tools::Long(COLFUZZY), "sw.core",
                 "Table width is not equal to the row width" );

    // Lock the table format when altering it, or else the box formats
    // are altered again.
    // Also, we need to preserve a percent setting if it exists.
    SwFrameFormat *pFrameFormat = m_pSwTable->GetFrameFormat();
    const_cast<SwTable *>(m_pSwTable)->LockModify();
    SwFormatFrameSize aFrameSize( pFrameFormat->GetFrameSize() );
    aFrameSize.SetWidth( m_nRelTabWidth );
    bool bRel = m_bUseRelWidth &&
                text::HoriOrientation::FULL!=pFrameFormat->GetHoriOrient().GetHoriOrient();
    aFrameSize.SetWidthPercent( static_cast<sal_uInt8>(bRel ? tools::Long(m_nWidthOption) : 0) );
    pFrameFormat->SetFormatAttr( aFrameSize );
    const_cast<SwTable *>(m_pSwTable)->UnlockModify();

    // If the table is located in a frame, we also need to adapt the
    // frame's width.
    if( MayBeInFlyFrame() )
    {
        SwFrameFormat *pFlyFrameFormat = FindFlyFrameFormat();
        if( pFlyFrameFormat )
        {
            SwFormatFrameSize aFlyFrameSize( SwFrameSize::Variable, m_nRelTabWidth, MINLAY );

            if( m_bUseRelWidth )
            {
                // For percentage settings we set the width to the minimum.
                aFlyFrameSize.SetWidth(  m_nMin > SwTwips(USHRT_MAX) ? SwTwips(USHRT_MAX)
                                                        : m_nMin );
                aFlyFrameSize.SetWidthPercent( static_cast<sal_uInt8>(tools::Long(m_nWidthOption)) );
            }
            pFlyFrameFormat->SetFormatAttr( aFlyFrameSize );
        }
    }

#ifdef DBG_UTIL
    {
        // check if the tables have correct widths
        SwTwips nSize = m_pSwTable->GetFrameFormat()->GetFrameSize().GetWidth();
        const SwTableLines& rLines = m_pSwTable->GetTabLines();
        for (size_t n = 0; n < rLines.size(); ++n)
        {
            CheckBoxWidth( *rLines[ n ], nSize );
        }
    }
#endif
}

void SwHTMLTableLayout::Resize_( SwTwips nAbsAvail, bool bRecalc )
{
    // If bRecalc is set, the table's content changed.
    // We need to execute pass 1 again.
    if( bRecalc )
        AutoLayoutPass1();

    SwRootFrame *pRoot = GetDoc().getIDocumentLayoutAccess().GetCurrentViewShell()->GetLayout();
    if ( pRoot && pRoot->IsCallbackActionEnabled() )
        pRoot->StartAllAction();

    // Else we can set the widths, in which we have to run Pass 2 in each case.
    SetWidths( true, nAbsAvail );

    if ( pRoot && pRoot->IsCallbackActionEnabled() )
        pRoot->EndAllAction( true );    //True per VirDev (browsing is calmer)
}

IMPL_LINK_NOARG( SwHTMLTableLayout, DelayedResize_Impl, Timer*, void )
{
    m_aResizeTimer.Stop();
    Resize_( m_nDelayedResizeAbsAvail, m_bDelayedResizeRecalc );
}

bool SwHTMLTableLayout::Resize( SwTwips nAbsAvail, bool bRecalc,
                                bool bForce, sal_uLong nDelay )
{
    if( SwTwips(0) == nAbsAvail )
        return false;
    OSL_ENSURE( IsTopTable(), "Resize must only be called for top tables!" );

    // May the table be resized at all? Or is it forced?
    if( m_bMustNotResize && !bForce )
        return false;

    // May the table be recalculated? Or is it forced?
    if( m_bMustNotRecalc && !bForce )
        bRecalc = false;

    const SwDoc& rDoc = GetDoc();

    // If there is a layout, the root frame's size instead of the
    // VisArea's size was potentially passed.
    // If we're not in a frame we need to calculate the table for the VisArea,
    // because switching from relative to absolute wouldn't work.
    if( rDoc.getIDocumentLayoutAccess().GetCurrentViewShell() && rDoc.getIDocumentLayoutAccess().GetCurrentViewShell()->GetViewOptions()->getBrowseMode() )
    {
        const SwTwips nVisAreaWidth = GetBrowseWidthByVisArea( rDoc );
        if( nVisAreaWidth < nAbsAvail && !FindFlyFrameFormat() )
            nAbsAvail = nVisAreaWidth;
    }

    if( nDelay==0 && m_aResizeTimer.IsActive() )
    {
        m_nDelayedResizeAbsAvail = nAbsAvail;
        return false;
    }

    // Optimisation:
    // If the minimum or maximum should not be recalculated and
    // - the table's width never needs to be recalculated, or
    // - the table was already calculated for the passed width, or
    // - the available space is less or equal to the minimum width
    //   and the table already has the minimum width, or
    // - the available space is larger than the maximum width and
    //   the table already has the maximum width
    // nothing will happen to the table.
    if( !bRecalc && ( !m_bMustResize ||
                      (m_nLastResizeAbsAvail==nAbsAvail) ||
                      (nAbsAvail<=m_nMin && m_nRelTabWidth==m_nMin) ||
                      (!m_bPercentWidthOption && nAbsAvail>=m_nMax && m_nRelTabWidth==m_nMax) ) )
        return false;

    if( nDelay==HTMLTABLE_RESIZE_NOW )
    {
        if( m_aResizeTimer.IsActive() )
            m_aResizeTimer.Stop();
        Resize_( nAbsAvail, bRecalc );
    }
    else if( nDelay > 0 )
    {
        m_nDelayedResizeAbsAvail = nAbsAvail;
        m_bDelayedResizeRecalc = bRecalc;
        m_aResizeTimer.SetTimeout( nDelay );
        m_aResizeTimer.Start();
    }
    else
    {
        Resize_( nAbsAvail, bRecalc );
    }

    return true;
}

void SwHTMLTableLayout::BordersChanged( SwTwips nAbsAvail )
{
    m_bBordersChanged = true;

    Resize( nAbsAvail, true/*bRecalc*/ );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
