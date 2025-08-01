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

#include <config_wasm_strip.h>

#include <bodyfrm.hxx>
#include <pagefrm.hxx>
#include <rootfrm.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <viewimp.hxx>
#include <fesh.hxx>
#include <swtable.hxx>
#include <deletelistener.hxx>
#include <dflyobj.hxx>
#include <anchoreddrawobject.hxx>
#include <fmtanchr.hxx>
#include <viewopt.hxx>
#include <hints.hxx>
#include <dbg_lay.hxx>
#include <ftnidx.hxx>
#include <svl/itemiter.hxx>
#include <editeng/keepitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/boxitem.hxx>
#include <basegfx/range/b1drange.hxx>
#include <fmtlsplt.hxx>
#include <fmtrowsplt.hxx>
#include <fmtsrnd.hxx>
#include <fmtornt.hxx>
#include <fmtpdsc.hxx>
#include <fmtfsize.hxx>
#include <swtblfmt.hxx>
#include <tabfrm.hxx>
#include <rowfrm.hxx>
#include <cellfrm.hxx>
#include <flyfrms.hxx>
#include <txtfrm.hxx>
#include <ftnfrm.hxx>
#include <notxtfrm.hxx>
#include <htmltbl.hxx>
#include <sectfrm.hxx>
#include <fmtfollowtextflow.hxx>
#include <sortedobjs.hxx>
#include <objectformatter.hxx>
#include <layouter.hxx>
#include <calbck.hxx>
#include <DocumentSettingManager.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <frmatr.hxx>
#include <frmtool.hxx>
#include <ndtxt.hxx>
#include <frameformats.hxx>

using namespace ::com::sun::star;

SwTabFrame::SwTabFrame( SwTable &rTab, SwFrame* pSib )
    : SwLayoutFrame( rTab.GetFrameFormat(), pSib )
    , SwFlowFrame( static_cast<SwFrame&>(*this) )
    , m_pTable( &rTab )
    , m_bComplete(false)
    , m_bCalcLowers(false)
    , m_bLowersFormatted(false)
    , m_bLockBackMove(false)
    , m_bWantBackMove(false)
    , m_bResizeHTMLTable(false)
    , m_bONECalcLowers(false)
    , m_bHasFollowFlowLine(false)
    , m_bIsRebuildLastLine(false)
    , m_bRestrictTableGrowth(false)
    , m_bRemoveFollowFlowLinePending(false)
    , m_bConsiderObjsForMinCellHeight(true)
    , m_bObjsDoesFit(true)
    , m_bInRecalcLowerRow(false)
{
    mbFixSize = false;     //Don't fall for import filter again.
    mnFrameType = SwFrameType::Tab;

    //Create the lines and insert them.
    const SwTableLines &rLines = rTab.GetTabLines();
    SwFrame *pTmpPrev = nullptr;
    bool bHiddenRedlines = getRootFrame()->IsHideRedlines() &&
        !GetFormat()->GetDoc().getIDocumentRedlineAccess().GetRedlineTable().empty();
    SwRedlineTable::size_type nRedlinePos = 0;
    for ( size_t i = 0; i < rLines.size(); ++i )
    {
        // skip lines deleted with track changes
        if ( bHiddenRedlines && rLines[i]->IsDeleted(nRedlinePos) )
            continue;

        SwRowFrame *pNew = new SwRowFrame( *rLines[i], this );
        if( pNew->Lower() )
        {
            pNew->InsertBehind( this, pTmpPrev );
            pTmpPrev = pNew;
        }
        else
            SwFrame::DestroyFrame(pNew);
    }
    SwFrame* pLower = Lower();
    OSL_ENSURE( pLower && pLower->IsRowFrame(), "SwTabFrame::SwTabFrame: No rows." );
}

SwTabFrame::SwTabFrame( SwTabFrame &rTab )
    : SwLayoutFrame( rTab.GetFormat(), &rTab )
    , SwFlowFrame( static_cast<SwFrame&>(*this) )
    , m_pTable( rTab.GetTable() )
    , m_bComplete(false)
    , m_bCalcLowers(false)
    , m_bLowersFormatted(false)
    , m_bLockBackMove(false)
    , m_bWantBackMove(false)
    , m_bResizeHTMLTable(false)
    , m_bONECalcLowers(false)
    , m_bHasFollowFlowLine(false)
    , m_bIsRebuildLastLine(false)
    , m_bRestrictTableGrowth(false)
    , m_bRemoveFollowFlowLinePending(false)
    , m_bConsiderObjsForMinCellHeight(true)
    , m_bObjsDoesFit(true)
    , m_bInRecalcLowerRow(false)
{
    mbFixSize = false;     //Don't fall for import filter again.
    mnFrameType = SwFrameType::Tab;

    SetFollow( rTab.GetFollow() );
    rTab.SetFollow( this );
}

void SwTabFrame::DestroyImpl()
{
    // There is some terrible code in fetab.cxx, that
    // caches pointers to SwTabFrames.
    ::ClearFEShellTabCols(GetFormat()->GetDoc(), this);

    SwLayoutFrame::DestroyImpl();
}

SwTabFrame::~SwTabFrame()
{
}

void SwTabFrame::JoinAndDelFollows()
{
    SwTabFrame *pFoll = GetFollow();
    if ( pFoll->HasFollow() )
        pFoll->JoinAndDelFollows();
    pFoll->Cut();
    SetFollow( pFoll->GetFollow() );
    SwFrame::DestroyFrame(pFoll);
}

void SwTabFrame::RegistFlys()
{
    SwFrame* pLower = Lower();
    OSL_ENSURE( pLower && pLower->IsRowFrame(), "No rows." );

    SwPageFrame *pPage = FindPageFrame();
    if ( pPage )
    {
        SwRowFrame *pRow = static_cast<SwRowFrame*>(pLower);
        do
        {
            pRow->RegistFlys( pPage );
            pRow = static_cast<SwRowFrame*>(pRow->GetNext());
        } while ( pRow );
    }
}

static void InvalidateVertOrientCells(SwRowFrame & rRow)
{
    for (SwFrame * pLower{rRow.Lower()}; pLower != nullptr; pLower = pLower->GetNext())
    {
        SwCellFrame * pCell{static_cast<SwCellFrame *>(pLower)};
        if (pCell->GetLayoutRowSpan() != 1)
        {
            pCell = const_cast<SwCellFrame *>(&pCell->FindStartEndOfRowSpanCell(true));
        }
        if (pCell->GetFormat()->GetVertOrient().GetVertOrient() != text::VertOrientation::NONE)
        {   // tdf#159029 force repositioning of content in cell because it
            pCell->InvalidatePrt(); // was disabled by SetRebuildLastLine()
        }
    }
}

static void SwInvalidateAll( SwFrame *pFrame, tools::Long nBottom );
static void lcl_RecalcRow( SwRowFrame& rRow, tools::Long nBottom );
static bool lcl_ArrangeLowers( SwLayoutFrame *pLay, tools::Long lYStart, bool bInva );
// #i26945# - add parameter <_bOnlyRowsAndCells> to control
// that only row and cell frames are formatted.
static bool lcl_InnerCalcLayout( SwFrame *pFrame,
                                      tools::Long nBottom,
                                      bool _bOnlyRowsAndCells = false );
// OD 2004-02-18 #106629# - correct type of 1st parameter
// #i26945# - add parameter <_bConsiderObjs> in order to
// control, if floating screen objects have to be considered for the minimal
// cell height.
static SwTwips lcl_CalcMinRowHeight( const SwRowFrame *pRow,
                                     const bool _bConsiderObjs );
static sal_uInt16 lcl_GetTopSpace( const SwRowFrame& rRow );
static sal_uInt16 lcl_GetBottomLineDist(const SwRowFrame& rRow);

static SwTwips lcl_CalcTopAndBottomMargin( const SwLayoutFrame&, const SwBorderAttrs& );

static SwTwips lcl_calcHeightOfRowBeforeThisFrame(const SwRowFrame& rRow);

static SwTwips lcl_GetHeightOfRows( const SwFrame* pStart, tools::Long nCount )
{
    if ( !nCount || !pStart)
        return 0;

    // There are moments when some rows have correct size, but wrong position: e.g., when a merged
    // cell is split, the 0-height follow of the previous row is inserted, and arrives here with a
    // correct height, but positioned at [0, 0]; the following rows have correct position (offset
    // to the new page's starting Y coordinate). This disallows to simplify the height calculation
    // to just difference between top row's top and bottom row's bottom.
    SwTwips nRet = 0;
    SwRectFnSet aRectFnSet(pStart);
    while (pStart && nCount > 0)
    {
        nRet += aRectFnSet.GetHeight(pStart->getFrameArea());
        pStart = pStart->GetNext();
        --nCount;
    }

    return nRet;
}

// Local helper function to insert a new follow flow line
static SwRowFrame* lcl_InsertNewFollowFlowLine( SwTabFrame& rTab, const SwFrame& rTmpRow, bool bRowSpanLine )
{
    OSL_ENSURE( rTmpRow.IsRowFrame(), "No row frame to copy for FollowFlowLine" );
    const SwRowFrame& rRow = static_cast<const SwRowFrame&>(rTmpRow);

    rTab.SetFollowFlowLine( true );
    SwRowFrame *pFollowFlowLine = new SwRowFrame(*rRow.GetTabLine(), &rTab, false );
    pFollowFlowLine->SetRowSpanLine( bRowSpanLine );
    SwFrame* pFirstRow = rTab.GetFollow()->GetFirstNonHeadlineRow();
    pFollowFlowLine->InsertBefore( rTab.GetFollow(), pFirstRow );
    return pFollowFlowLine;
}

// #i26945# - local helper function to invalidate all lower
// objects. By parameter <_bMoveObjsOutOfRange> it can be controlled, if
// additionally the objects are moved 'out of range'.
static void lcl_InvalidateLowerObjs( SwLayoutFrame& _rLayoutFrame,
                              const bool _bMoveObjsOutOfRange = false,
                              SwPageFrame* _pPageFrame = nullptr )
{
    // determine page frame, if needed
    if ( !_pPageFrame )
    {
        _pPageFrame = _rLayoutFrame.FindPageFrame();
        OSL_ENSURE( _pPageFrame,
                "<lcl_InvalidateLowerObjs(..)> - missing page frame -> no move of lower objects out of range" );
        if ( !_pPageFrame )
        {
            return;
        }
    }

    // loop on lower frames
    SwFrame* pLowerFrame = _rLayoutFrame.Lower();
    while ( pLowerFrame )
    {
        if ( pLowerFrame->IsLayoutFrame() )
        {
            ::lcl_InvalidateLowerObjs( *static_cast<SwLayoutFrame*>(pLowerFrame),
                                       _bMoveObjsOutOfRange, _pPageFrame );
        }
        if ( pLowerFrame->GetDrawObjs() )
        {
            for (size_t i = 0, nCount = pLowerFrame->GetDrawObjs()->size(); i < nCount; ++i)
            {
                SwAnchoredObject* pAnchoredObj = (*pLowerFrame->GetDrawObjs())[i];

                // invalidate position of anchored object
                pAnchoredObj->SetTmpConsiderWrapInfluence( false );
                pAnchoredObj->SetConsiderForTextWrap( false );
                pAnchoredObj->UnlockPosition();
                pAnchoredObj->InvalidateObjPos();

                SwFlyFrame *pFly = pAnchoredObj->DynCastFlyFrame();

                // move anchored object 'out of range'
                if ( _bMoveObjsOutOfRange )
                {
                    // indicate, that positioning is progress to avoid
                    // modification of the anchored object resp. it's attributes
                    // due to the movement
                    SwObjPositioningInProgress aObjPosInProgress( *pAnchoredObj );
                    pAnchoredObj->SetObjLeft( _pPageFrame->getFrameArea().Right() );
                    // #115759# - reset character rectangle,
                    // top of line and relative position in order to assure,
                    // that anchored object is correctly positioned.
                    pAnchoredObj->ClearCharRectAndTopOfLine();
                    pAnchoredObj->SetCurrRelPos( Point( 0, 0 ) );
                    const SwFrameFormat* pObjFormat = pAnchoredObj->GetFrameFormat();
                    if (pObjFormat->GetAnchor().GetAnchorId() == RndStdIds::FLY_AS_CHAR)
                    {
                        pAnchoredObj->AnchorFrame()
                                ->Prepare( PrepareHint::FlyFrameAttributesChanged,
                                           pObjFormat );
                    }
                    if ( pFly != nullptr )
                    {
                        pFly->GetVirtDrawObj()->SetBoundAndSnapRectsDirty();
                        pFly->GetVirtDrawObj()->SetChanged();
                    }
                }

                // If anchored object is a fly frame, invalidate its lower objects
                if ( pFly != nullptr )
                {
                    ::lcl_InvalidateLowerObjs( *pFly, _bMoveObjsOutOfRange, _pPageFrame );
                }
            }
        }
        pLowerFrame = pLowerFrame->GetNext();
    }
}

// Local helper function to shrink all lowers of pRow to 0 height
static void lcl_ShrinkCellsAndAllContent( SwRowFrame& rRow )
{
    SwCellFrame* pCurrMasterCell = static_cast<SwCellFrame*>(rRow.Lower());
    SwRectFnSet aRectFnSet(pCurrMasterCell);

    bool bAllCellsCollapsed = true;
    while ( pCurrMasterCell )
    {
        // NEW TABLES
        SwCellFrame& rToAdjust = pCurrMasterCell->GetTabBox()->getRowSpan() < 1 ?
                               const_cast<SwCellFrame&>(pCurrMasterCell->FindStartEndOfRowSpanCell( true )) :
                               *pCurrMasterCell;

        // #i26945#
        // all lowers should have the correct position
        lcl_ArrangeLowers( &rToAdjust,
                           aRectFnSet.GetPrtTop(rToAdjust),
                           false );
        // TODO: Optimize number of frames which are set to 0 height
        // we have to start with the last lower frame, otherwise
        // the shrink will not shrink the current cell
        SwFrame* pTmp = rToAdjust.GetLastLower();
        bool bAllLowersCollapsed = true;

        if ( pTmp && pTmp->IsRowFrame() )
        {
            SwRowFrame* pTmpRow = static_cast<SwRowFrame*>(pTmp);
            lcl_ShrinkCellsAndAllContent( *pTmpRow );
        }
        else
        {
            // TODO: Optimize number of frames which are set to 0 height
            while ( pTmp )
            {
                // the frames have to be shrunk
                if ( pTmp->IsTabFrame() )
                {
                    SwRowFrame* pTmpRow = static_cast<SwRowFrame*>(static_cast<SwTabFrame*>(pTmp)->Lower());
                    bool bAllRowsCollapsed = true;

                    while ( pTmpRow )
                    {
                        lcl_ShrinkCellsAndAllContent( *pTmpRow );

                        if (aRectFnSet.GetHeight(pTmpRow->getFrameArea()) > 0)
                            bAllRowsCollapsed = false;

                        pTmpRow = static_cast<SwRowFrame*>(pTmpRow->GetNext());
                    }

                    if (bAllRowsCollapsed)
                    {
                        // All rows of this table have 0 height -> set height of the table itself as well.
                        SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*pTmp);
                        aRectFnSet.SetHeight(aFrm, 0);

                        SwFrameAreaDefinition::FramePrintAreaWriteAccess aPrt(*pTmp);
                        aRectFnSet.SetTop(aPrt, 0);
                        aRectFnSet.SetHeight(aPrt, 0);
                    }
                    else
                        bAllLowersCollapsed = false;
                }
                else
                {
                    pTmp->Shrink(aRectFnSet.GetHeight(pTmp->getFrameArea()));
                    SwFrameAreaDefinition::FramePrintAreaWriteAccess aPrt(*pTmp);
                    aRectFnSet.SetTop(aPrt, 0);
                    aRectFnSet.SetHeight(aPrt, 0);

                    if (aRectFnSet.GetHeight(pTmp->getFrameArea()) > 0)
                    {
                        bAllLowersCollapsed = false;
                    }
                }

                pTmp = pTmp->GetPrev();
            }

            // all lowers should have the correct position
            lcl_ArrangeLowers( &rToAdjust,
                               aRectFnSet.GetPrtTop(rToAdjust),
                               false );
        }

        if (bAllLowersCollapsed)
        {
            // All lower frame of this cell have 0 height -> set height of the cell itself as well.
            SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*pCurrMasterCell);
            aRectFnSet.SetHeight(aFrm, 0);

            SwFrameAreaDefinition::FramePrintAreaWriteAccess aPrt(*pCurrMasterCell);
            aRectFnSet.SetTop(aPrt, 0);
            aRectFnSet.SetHeight(aPrt, 0);
        }
        else
            bAllCellsCollapsed = false;

        pCurrMasterCell = static_cast<SwCellFrame*>(pCurrMasterCell->GetNext());
    }

    if (bAllCellsCollapsed)
    {
        // All cells have 0 height -> set height of row as well.
        SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(rRow);
        aRectFnSet.SetHeight(aFrm, 0);

        SwFrameAreaDefinition::FramePrintAreaWriteAccess aPrt(rRow);
        aRectFnSet.SetTop(aPrt, 0);
        aRectFnSet.SetHeight(aPrt, 0);
    }
}

// Local helper function to move the content from rSourceLine to rDestLine
// The content is inserted behind the last content in the corresponding
// cell in rDestLine.
static void lcl_MoveRowContent( SwRowFrame& rSourceLine, SwRowFrame& rDestLine )
{
    SwCellFrame* pCurrDestCell = static_cast<SwCellFrame*>(rDestLine.Lower());
    SwCellFrame* pCurrSourceCell = static_cast<SwCellFrame*>(rSourceLine.Lower());

    // Move content of follow cells into master cells
    while ( pCurrSourceCell )
    {
        assert(pCurrDestCell);
        SwFrame* pLower = pCurrSourceCell->Lower();
        if ( pLower && pLower->IsRowFrame() )
        {
            SwRowFrame* pTmpSourceRow = static_cast<SwRowFrame*>(pLower);
            while ( pTmpSourceRow )
            {
                // #125926# Attention! It is possible,
                // that pTmpSourceRow->IsFollowFlowRow() but pTmpDestRow
                // cannot be found. In this case, we have to move the complete
                // row.
                SwRowFrame* pTmpDestRow = static_cast<SwRowFrame*>(pCurrDestCell->Lower());

                if ( pTmpSourceRow->IsFollowFlowRow() && pTmpDestRow )
                {
                    // move content from follow flow row to pTmpDestRow:
                    while ( pTmpDestRow->GetNext() )
                        pTmpDestRow = static_cast<SwRowFrame*>(pTmpDestRow->GetNext());

                    assert(pTmpDestRow->GetFollowRow() == pTmpSourceRow);

                    lcl_MoveRowContent( *pTmpSourceRow, *pTmpDestRow );
                    pTmpDestRow->SetFollowRow( pTmpSourceRow->GetFollowRow() );
                    pTmpSourceRow->RemoveFromLayout();
                    SwFrame::DestroyFrame(pTmpSourceRow);
                }
                else
                {
                    // move complete row:
                    pTmpSourceRow->RemoveFromLayout();
                    pTmpSourceRow->InsertBefore( pCurrDestCell, nullptr );
                }

                // RemoveFromLayout invalidates Lower() so it must be refetched
                pTmpSourceRow = static_cast<SwRowFrame*>(pCurrSourceCell->Lower());
            }
        }
        else
        {
            SwFrame *pTmp = ::SaveContent( pCurrSourceCell );
            if ( pTmp )
            {
                // NEW TABLES
                SwCellFrame* pDestCell = pCurrDestCell;
                if ( pDestCell->GetTabBox()->getRowSpan() < 1 )
                    pDestCell = & const_cast<SwCellFrame&>(pDestCell->FindStartEndOfRowSpanCell( true ));

                // Find last content
                SwFrame* pFrame = pDestCell->GetLastLower();
                ::RestoreContent( pTmp, pDestCell, pFrame );
            }
        }
        pCurrDestCell = static_cast<SwCellFrame*>(pCurrDestCell->GetNext());
        pCurrSourceCell = static_cast<SwCellFrame*>(pCurrSourceCell->GetNext());
    }
}

// Local helper function to move all footnotes in rRowFrame from
// the footnote boss of rSource to the footnote boss of rDest.
static void lcl_MoveFootnotes( SwTabFrame& rSource, SwTabFrame& rDest, SwLayoutFrame& rRowFrame )
{
    if ( !rSource.GetFormat()->GetDoc().GetFootnoteIdxs().empty() )
    {
        SwFootnoteBossFrame* pOldBoss = rSource.FindFootnoteBossFrame( true );
        SwFootnoteBossFrame* pNewBoss = rDest.FindFootnoteBossFrame( true );
        rRowFrame.MoveLowerFootnotes( nullptr, pOldBoss, pNewBoss, true );
    }
}

// Local helper function to handle nested table cells before the split process
static void lcl_PreprocessRowsInCells( SwTabFrame& rTab, SwRowFrame& rLastLine,
                                SwRowFrame& rFollowFlowLine, SwTwips nRemain )
{
    SwCellFrame* pCurrLastLineCell = static_cast<SwCellFrame*>(rLastLine.Lower());
    SwCellFrame* pCurrFollowFlowLineCell = static_cast<SwCellFrame*>(rFollowFlowLine.Lower());

    SwRectFnSet aRectFnSet(pCurrLastLineCell);

    // Move content of follow cells into master cells
    while ( pCurrLastLineCell )
    {
        if ( pCurrLastLineCell->Lower() && pCurrLastLineCell->Lower()->IsRowFrame() )
        {
            SwTwips nTmpCut = nRemain;
            SwRowFrame* pTmpLastLineRow = static_cast<SwRowFrame*>(pCurrLastLineCell->Lower());

            // #i26945#
            SwTwips nCurrentHeight =
                    lcl_CalcMinRowHeight( pTmpLastLineRow,
                                          rTab.IsConsiderObjsForMinCellHeight() );
            while ( pTmpLastLineRow->GetNext() && nTmpCut > nCurrentHeight )
            {
                nTmpCut -= nCurrentHeight;
                pTmpLastLineRow = static_cast<SwRowFrame*>(pTmpLastLineRow->GetNext());
                // #i26945#
                nCurrentHeight =
                    lcl_CalcMinRowHeight( pTmpLastLineRow,
                                          rTab.IsConsiderObjsForMinCellHeight() );
            }

            // pTmpLastLineRow does not fit to the line or it is the last line
            // Check if we can move pTmpLastLineRow to the follow table,
            // or if we have to split the line:
            bool bTableLayoutTooComplex = false;
            tools::Long nMinHeight = 0;

            // We have to take into account:
            // 1. The fixed height of the row
            // 2. The borders of the cells inside the row
            // 3. The minimum height of the row
            if ( pTmpLastLineRow->HasFixSize() )
                nMinHeight = aRectFnSet.GetHeight(pTmpLastLineRow->getFrameArea());
            else
            {
                {
                    const SwFormatFrameSize &rSz = pTmpLastLineRow->GetFormat()->GetFrameSize();
                    if ( rSz.GetHeightSizeType() == SwFrameSize::Minimum )
                        nMinHeight = rSz.GetHeight() - lcl_calcHeightOfRowBeforeThisFrame(*pTmpLastLineRow);
                }

                SwFrame* pCell = pTmpLastLineRow->Lower();
                while ( pCell )
                {
                    SwFrame* pCellLower = static_cast<SwCellFrame*>(pCell)->Lower();
                    if ( pCellLower && pCellLower->IsRowFrame() )
                    {
                        bTableLayoutTooComplex = true;
                        break;
                    }

                    SwBorderAttrAccess aAccess( SwFrame::GetCache(), pCell );
                    const SwBorderAttrs &rAttrs = *aAccess.Get();
                    nMinHeight = std::max( nMinHeight, tools::Long(lcl_CalcTopAndBottomMargin( *static_cast<SwLayoutFrame*>(pCell), rAttrs )) );
                    pCell = pCell->GetNext();
                }
            }

            // 1. Case:
            // The line completely fits into the master table.
            // Nevertheless, we build a follow (otherwise painting problems
            // with empty cell).

            // 2. Case:
            // The line has to be split, the minimum height still fits into
            // the master table, and the table structure is not too complex.
            if ( nTmpCut > nCurrentHeight ||
                 ( pTmpLastLineRow->IsRowSplitAllowed() &&
                  !bTableLayoutTooComplex && nMinHeight < nTmpCut ) )
            {
                // The line has to be split:
                SwRowFrame* pNewRow = new SwRowFrame( *pTmpLastLineRow->GetTabLine(), &rTab, false );
                pNewRow->SetFollowFlowRow( true );
                pNewRow->SetFollowRow( pTmpLastLineRow->GetFollowRow() );
                pTmpLastLineRow->SetFollowRow( pNewRow );
                pNewRow->InsertBehind( pCurrFollowFlowLineCell, nullptr );
                pTmpLastLineRow = static_cast<SwRowFrame*>(pTmpLastLineRow->GetNext());
            }

            // The following lines have to be moved:
            while ( pTmpLastLineRow )
            {
                SwRowFrame* pTmp = static_cast<SwRowFrame*>(pTmpLastLineRow->GetNext());
                lcl_MoveFootnotes( rTab, *rTab.GetFollow(), *pTmpLastLineRow );
                pTmpLastLineRow->RemoveFromLayout();
                pTmpLastLineRow->InsertBefore( pCurrFollowFlowLineCell, nullptr );
                pTmpLastLineRow->Shrink( aRectFnSet.GetHeight(pTmpLastLineRow->getFrameArea()) );
                pCurrFollowFlowLineCell->Grow( aRectFnSet.GetHeight(pTmpLastLineRow->getFrameArea()) );
                pTmpLastLineRow = pTmp;
            }
        }

        pCurrLastLineCell = static_cast<SwCellFrame*>(pCurrLastLineCell->GetNext());
        pCurrFollowFlowLineCell = static_cast<SwCellFrame*>(pCurrFollowFlowLineCell->GetNext());
    }
}

// Local helper function to handle nested table cells after the split process
static void lcl_PostprocessRowsInCells( SwTabFrame& rTab, SwRowFrame& rLastLine )
{
    SwCellFrame* pCurrMasterCell = static_cast<SwCellFrame*>(rLastLine.Lower());
    while ( pCurrMasterCell )
    {
        SwFrame* pLower = pCurrMasterCell->Lower();
        if ( pLower && pLower->IsRowFrame() )
        {
            SwRowFrame* pRowFrame = static_cast<SwRowFrame*>(pCurrMasterCell->GetLastLower());

            if ( nullptr != pRowFrame->GetPrev() && !pRowFrame->ContainsContent() )
            {
                OSL_ENSURE( pRowFrame->GetFollowRow(), "Deleting row frame without follow" );

                // The footnotes have to be moved:
                lcl_MoveFootnotes( rTab, *rTab.GetFollow(), *pRowFrame );
                pRowFrame->Cut();
                SwRowFrame* pFollowRow = pRowFrame->GetFollowRow();
                pRowFrame->Paste( pFollowRow->GetUpper(), pFollowRow );
                pRowFrame->SetFollowRow( pFollowRow->GetFollowRow() );
                lcl_MoveRowContent( *pFollowRow, *pRowFrame );
                pFollowRow->Cut();
                SwFrame::DestroyFrame(pFollowRow);
                ::SwInvalidateAll( pCurrMasterCell, LONG_MAX );
            }
        }

        pCurrMasterCell = static_cast<SwCellFrame*>(pCurrMasterCell->GetNext());
    }
}

// Local helper function to re-calculate the split line.
inline void TableSplitRecalcLock( SwFlowFrame *pTab ) { pTab->LockJoin(); }
inline void TableSplitRecalcUnlock( SwFlowFrame *pTab ) { pTab->UnlockJoin(); }

static bool lcl_RecalcSplitLine( SwRowFrame& rLastLine, SwRowFrame& rFollowLine,
                          SwTwips nRemainingSpaceForLastRow, SwTwips nAlreadyFree,
                          bool & rIsFootnoteGrowth)
{
    bool bRet = true;

    vcl::RenderContext* pRenderContext = rLastLine.getRootFrame()->GetCurrShell()->GetOut();
    SwTabFrame& rTab = static_cast<SwTabFrame&>(*rLastLine.GetUpper());
    SwRectFnSet aRectFnSet(rTab.GetUpper());
    SwTwips nCurLastLineHeight = aRectFnSet.GetHeight(rLastLine.getFrameArea());

    SwTwips nFootnoteHeight(0);
    if (SwFootnoteBossFrame const*const pBoss = rTab.FindFootnoteBossFrame())
    {
        if (SwFootnoteContFrame const*const pCont = pBoss->FindFootnoteCont())
        {
            for (SwFootnoteFrame const* pFootnote = static_cast<SwFootnoteFrame const*>(pCont->Lower());
                 pFootnote != nullptr;
                 pFootnote = static_cast<SwFootnoteFrame const*>(pFootnote->GetNext()))
            {
                SwContentFrame const*const pAnchor = pFootnote->GetRef();
                SwTabFrame const* pTab = pAnchor->FindTabFrame();
                if (pTab)
                {
                    while (pTab->GetUpper()->IsInTab())
                    {
                        pTab = pTab->GetUpper()->FindTabFrame();
                    }
                    // TODO currently do this only for top-level tables?
                    // otherwise would need to check rTab's follow and any upper table's follow?
                    if (pTab == &rTab)
                    {
                        nFootnoteHeight += aRectFnSet.GetHeight(pFootnote->getFrameArea());
                    }
                }
            }
        }
    }

    // If there are nested cells in rLastLine, the recalculation of the last
    // line needs some preprocessing.
    lcl_PreprocessRowsInCells( rTab, rLastLine, rFollowLine, nRemainingSpaceForLastRow );

    // Here the recalculation process starts:
    rTab.SetRebuildLastLine( true );
    // #i26945#
    rTab.SetDoesObjsFit( true );

    // #i26945# - invalidate and move floating screen
    // objects 'out of range'
    ::lcl_InvalidateLowerObjs( rLastLine, true );

    // manipulate row and cell sizes

    // #i26945# - Do *not* consider floating screen objects
    // for the minimal cell height.
    rTab.SetConsiderObjsForMinCellHeight( false );
    ::lcl_ShrinkCellsAndAllContent( rLastLine );
    rTab.SetConsiderObjsForMinCellHeight( true );

    // invalidate last line
    ::SwInvalidateAll( &rLastLine, LONG_MAX );

    // Shrink the table to account for the shrunk last row, as well as lower rows
    // that had been moved to follow table in SwTabFrame::Split.
    // It will grow later when last line will recalc its height.
    rTab.Shrink(nAlreadyFree + nCurLastLineHeight - nRemainingSpaceForLastRow + 1);

    // Lock this tab frame and its follow
    bool bUnlockMaster = false;
    SwFlowFrame * pFollow = nullptr;
    SwTabFrame* pMaster = rTab.IsFollow() ? rTab.FindMaster() : nullptr;
    if ( pMaster && !pMaster->IsJoinLocked() )
    {
        bUnlockMaster = true;
        ::TableSplitRecalcLock( pMaster );
    }
    if ( !rTab.GetFollow()->IsJoinLocked() )
    {
        pFollow = rTab.GetFollow();
        ::TableSplitRecalcLock( pFollow );
    }

    bool bInSplit = rLastLine.IsInSplit();
    rLastLine.SetInSplit();

    // Do the recalculation
    lcl_RecalcRow( rLastLine, LONG_MAX );
    // #115759# - force a format of the last line in order to
    // get the correct height.
    rLastLine.InvalidateSize();
    rLastLine.Calc(pRenderContext);

    rLastLine.SetInSplit(bInSplit);

    // Unlock this tab frame and its follow
    if ( pFollow )
        ::TableSplitRecalcUnlock( pFollow );
    if ( bUnlockMaster )
        ::TableSplitRecalcUnlock( pMaster );

    // If there are nested cells in rLastLine, the recalculation of the last
    // line needs some postprocessing.
    lcl_PostprocessRowsInCells( rTab, rLastLine );

    // Do a couple of checks on the current situation.

    // If we are not happy with the current situation we return false.
    // This will start a new try to split the table, this time we do not
    // try to split the table rows.

    // 1. Check if table fits to its upper.
    // #i26945# - include check, if objects fit
    const SwTwips nDistanceToUpperPrtBottom =
        aRectFnSet.BottomDist(rTab.getFrameArea(), aRectFnSet.GetPrtBottom(*rTab.GetUpper()));
    // also check the footnote boss - it *may* be smaller than the upper now!
    const SwTwips nDistanceToFootnoteBodyPrtBottom =
        aRectFnSet.BottomDist(rTab.getFrameArea(), aRectFnSet.GetPrtBottom(*rTab.FindFootnoteBossFrame()->FindBodyCont()));
    // tdf#125685 ignore footnotes that are anchored in follow-table of this
    // table - if split is successful they move to the next page/column anyway
    assert(rTab.GetFollow() == rFollowLine.GetUpper());
    SwTwips nFollowFootnotes(0);
    // actually there should always be a boss frame, except if "this" isn't
    // connected to a page yet; not sure if that can happen
    if (SwFootnoteBossFrame const*const pBoss = rTab.FindFootnoteBossFrame())
    {
        if (SwFootnoteContFrame const*const pCont = pBoss->FindFootnoteCont())
        {
            for (SwFootnoteFrame const* pFootnote = static_cast<SwFootnoteFrame const*>(pCont->Lower());
                 pFootnote != nullptr;
                 pFootnote = static_cast<SwFootnoteFrame const*>(pFootnote->GetNext()))
            {
                SwContentFrame const*const pAnchor = pFootnote->GetRef();
                SwTabFrame const* pTab = pAnchor->FindTabFrame();
                if (pTab)
                {
                    while (pTab->GetUpper()->IsInTab())
                    {
                        pTab = pTab->GetUpper()->FindTabFrame();
                    }
                    // TODO currently do this only for top-level tables?
                    // otherwise would need to check rTab's follow and any upper table's follow?
                    if (pTab == rTab.GetFollow())
                    {
                        nFollowFootnotes += aRectFnSet.GetHeight(pFootnote->getFrameArea());
                    }
                    if (pTab == &rTab)
                    {
                        nFootnoteHeight -= aRectFnSet.GetHeight(pFootnote->getFrameArea());
                    }
                }
            }
            if (nFootnoteHeight < 0)
            {   // tdf#156724 footnotes have grown, try to split again
                rIsFootnoteGrowth = true;
            }
        }
    }
    if (nDistanceToUpperPrtBottom + nFollowFootnotes < 0 || !rTab.DoesObjsFit())
        bRet = false;

    // apparently checking nFootnoteHeight here does *not* guarantee that it fits into the body
    if (bRet && rTab.IsInDocBody()
        && nDistanceToFootnoteBodyPrtBottom + nFollowFootnotes < 0)
    {
        assert(rTab.GetUpper() != rTab.FindFootnoteBossFrame()->FindBodyCont());
        SAL_INFO("sw.layout", "SwTabFrame Split failed because of footnote growth");
        bRet = false; // tdf#160897
    }

    // 2. Check if each cell in the last line has at least one content frame.

    // Note: a FollowFlowRow may contains empty cells!
    if ( bRet )
    {
        if ( !rLastLine.IsInFollowFlowRow() )
        {
            SwCellFrame* pCurrMasterCell = static_cast<SwCellFrame*>(rLastLine.Lower());
            while ( pCurrMasterCell )
            {
                if ( !pCurrMasterCell->ContainsContent() && pCurrMasterCell->GetTabBox()->getRowSpan() >= 1 )
                {
                    bRet = false;
                    break;
                }
                pCurrMasterCell = static_cast<SwCellFrame*>(pCurrMasterCell->GetNext());
            }
        }
    }

    // 3. Check if last line does not contain any content:
    if ( bRet )
    {
        if ( !rLastLine.ContainsContent() )
        {
            bRet = false;
        }
    }

    // 4. Check if follow flow line does not contain content:
    if ( bRet )
    {
        if ( !rFollowLine.IsRowSpanLine() && !rFollowLine.ContainsContent() )
        {
            bRet = false;
        }
    }

    if ( bRet )
    {
        // Everything looks fine. Splitting seems to be successful. We invalidate
        // rFollowLine to force a new formatting.
        ::SwInvalidateAll( &rFollowLine, LONG_MAX );
        InvalidateVertOrientCells(rLastLine);
    }
    else
    {
        // Splitting the table row gave us an unexpected result.
        // Everything has to be prepared for a second try to split
        // the table, this time without splitting the row.
        ::SwInvalidateAll( &rLastLine, LONG_MAX );
    }

    rTab.SetRebuildLastLine( false );
    // #i26945#
    rTab.SetDoesObjsFit( true );

    return bRet;
}

// Sets the correct height for all spanned cells
static void lcl_AdjustRowSpanCells( SwRowFrame* pRow )
{
    SwRectFnSet aRectFnSet(pRow);
    SwCellFrame* pCellFrame = static_cast<SwCellFrame*>(pRow->GetLower());
    while ( pCellFrame )
    {
        const tools::Long nLayoutRowSpan = pCellFrame->GetLayoutRowSpan();
        if ( nLayoutRowSpan > 1 )
        {
            // calculate height of cell:
            const tools::Long nNewCellHeight = lcl_GetHeightOfRows( pRow, nLayoutRowSpan );
            const tools::Long nDiff = nNewCellHeight - aRectFnSet.GetHeight(pCellFrame->getFrameArea());

            if ( nDiff )
            {
                SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*pCellFrame);
                aRectFnSet.AddBottom(aFrm, nDiff);
            }
        }

        pCellFrame = static_cast<SwCellFrame*>(pCellFrame->GetNext());
    }
}

// Returns the maximum layout row span of the row
// Looking for the next row that contains no covered cells:
static tools::Long lcl_GetMaximumLayoutRowSpan( const SwRowFrame& rRow )
{
    tools::Long nRet = 1;

    const SwRowFrame* pCurrentRowFrame = static_cast<const SwRowFrame*>(rRow.GetNext());
    bool bNextRow = false;

    while ( pCurrentRowFrame )
    {
        // if there is any covered cell, we proceed to the next row frame
        const SwCellFrame* pLower = static_cast<const SwCellFrame*>( pCurrentRowFrame->Lower());
        while ( pLower )
        {
            if ( pLower->GetTabBox()->getRowSpan() < 0 )
            {
                ++nRet;
                bNextRow = true;
                break;
            }
            pLower = static_cast<const SwCellFrame*>(pLower->GetNext());
        }
        pCurrentRowFrame = bNextRow ?
                         static_cast<const SwRowFrame*>(pCurrentRowFrame->GetNext() ) :
                         nullptr;
    }

    return nRet;
}

// Function to remove the FollowFlowLine of rTab.
// The content of the FollowFlowLine is moved to the associated line in the
// master table.
bool SwTabFrame::RemoveFollowFlowLine()
{
    // find FollowFlowLine
    SwTabFrame *pFoll = GetFollow();
    SwRowFrame* pFollowFlowLine = pFoll ? pFoll->GetFirstNonHeadlineRow() : nullptr;

    // find last row in master
    SwFrame* pLastLine = GetLastLower();

    OSL_ENSURE( HasFollowFlowLine() &&
            pFollowFlowLine &&
            pLastLine, "There should be a flowline in the follow" );

    // #140081# Make code robust.
    if ( !pFollowFlowLine || !pLastLine )
        return true;
    if (pFollowFlowLine->IsDeleteForbidden())
    {
        SAL_WARN("sw.layout", "Cannot remove in-use Follow Flow Line");
        return false;
    }

    // We have to reset the flag here, because lcl_MoveRowContent
    // calls a GrowFrame(), which has a different behavior if
    // this flag is set.
    SetFollowFlowLine( false );

    // Move content
    lcl_MoveRowContent( *pFollowFlowLine, *static_cast<SwRowFrame*>(pLastLine) );

    // NEW TABLES
    // If a row span follow flow line is removed, we want to move the whole span
    // to the master:
    tools::Long nRowsToMove = lcl_GetMaximumLayoutRowSpan( *pFollowFlowLine );

    if ( nRowsToMove > 1 )
    {
        SwRectFnSet aRectFnSet(this);
        SwFrame* pRow = pFollowFlowLine->GetNext();
        SwFrame* pInsertBehind = GetLastLower();
        SwTwips nGrow = 0;

        while ( pRow && nRowsToMove-- > 1 )
        {
            SwFrame* pNxt = pRow->GetNext();
            nGrow += aRectFnSet.GetHeight(pRow->getFrameArea());

            // The footnotes have to be moved:
            lcl_MoveFootnotes( *GetFollow(), *this, static_cast<SwRowFrame&>(*pRow) );

            pRow->RemoveFromLayout();
            pRow->InsertBehind( this, pInsertBehind );
            pRow->InvalidateAll_();
            pRow->CheckDirChange();
            pInsertBehind = pRow;
            pRow = pNxt;
        }

        SwFrame* pFirstRow = Lower();
        while ( pFirstRow )
        {
            lcl_AdjustRowSpanCells( static_cast<SwRowFrame*>(pFirstRow) );
            pFirstRow = pFirstRow->GetNext();
        }

        Grow( nGrow );
        GetFollow()->Shrink( nGrow );
    }

    bool bJoin = !pFollowFlowLine->GetNext();
    pFollowFlowLine->Cut();
    SwFrame::DestroyFrame(pFollowFlowLine);

    return bJoin;
}

// #i26945# - Floating screen objects are no longer searched.
static bool lcl_FindSectionsInRow( const SwRowFrame& rRow )
{
    bool bRet = false;
    const SwCellFrame* pLower = static_cast<const SwCellFrame*>(rRow.Lower());
    while ( pLower )
    {
        if ( pLower->IsVertical() != rRow.IsVertical() )
            return true;

        const SwFrame* pTmpFrame = pLower->Lower();
        while ( pTmpFrame )
        {
            if ( pTmpFrame->IsRowFrame() )
            {
                bRet = lcl_FindSectionsInRow( *static_cast<const SwRowFrame*>(pTmpFrame) );
            }
            else
            {
                // #i26945# - search only for sections
                if (pTmpFrame->IsSctFrame())
                {
                    bRet = true;

                    if (!rRow.IsInSct())
                    {
                        // This row is not in a section.
                        if (const SwFrame* pSectionLower = pTmpFrame->GetLower())
                        {
                            if (!pSectionLower->IsColumnFrame())
                            {
                                // Section has a single column only, try to
                                // split that.
                                bRet = false;

                                for (const SwFrame* pFrame = pSectionLower; pFrame; pFrame = pFrame->GetNext())
                                {
                                    if (pFrame->IsTabFrame())
                                    {
                                        // Section contains a table, no split in that case.
                                        bRet = true;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            if ( bRet )
                return true;
            pTmpFrame = pTmpFrame->GetNext();
        }

        pLower = static_cast<const SwCellFrame*>(pLower->GetNext());
    }
    return bRet;
}

bool SwTabFrame::Split(const SwTwips nCutPos, bool bTryToSplit,
        bool bTableRowKeep, bool & rIsFootnoteGrowth)
{
    bool bRet = true;

    SwRectFnSet aRectFnSet(this);

    // #i26745# - format row and cell frames of table
    {
        Lower()->InvalidatePos_();
        // #i43913# - correction
        // call method <lcl_InnerCalcLayout> with first lower.
        lcl_InnerCalcLayout( Lower(), LONG_MAX, true );
    }

    //In order to be able to compare the positions of the cells with CutPos,
    //they have to be calculated consecutively starting from the table.
    //They can definitely be invalid because of position changes of the table.
    SwRowFrame *pRow = static_cast<SwRowFrame*>(Lower());
    if( !pRow )
        return bRet;

    const sal_uInt16 nRepeat = GetTable()->GetRowsToRepeat();
    sal_uInt16 nRowCount = 0;           // pRow currently points to the first row

    // Due to the comment above, about possibly invalid positions of cells due to position changes
    // of the table, calculate the rows height subtracting last row's bottom from first row's top,
    // and compare with the available height
    SwTwips nRemainingSpaceForLastRow = aRectFnSet.YDiff(nCutPos, aRectFnSet.GetPrtTop(*this));
    nRemainingSpaceForLastRow -= aRectFnSet.GetBottomMargin(*this);

    if (nRemainingSpaceForLastRow <= 0)
        return false; // upper has no space for this table at all

    auto getRemainingAfter = [aRectFnSet, nAvailable = nRemainingSpaceForLastRow,
                              nFirstRowTop = aRectFnSet.GetTop(pRow->getFrameArea())](SwFrame* p)
    { return nAvailable + (p ? aRectFnSet.BottomDist(p->getFrameArea(), nFirstRowTop) : 0); };

    // Make pRow point to the line that does not fit anymore:
    while (pRow->GetNext())
    {
        SwTwips nNewRemaining = getRemainingAfter(pRow);
        if (nNewRemaining < 0)
            break;
        if( bTryToSplit || !pRow->IsRowSpanLine() ||
            0 != aRectFnSet.GetHeight(pRow->getFrameArea()) )
            ++nRowCount;
        nRemainingSpaceForLastRow = nNewRemaining;
        pRow = static_cast<SwRowFrame*>(pRow->GetNext());
    }

    // bSplitRowAllowed: Row may be split according to its attributes.
    // bTryToSplit:      Row will never be split if bTryToSplit = false.
    //                   This can either be passed as a parameter, indicating
    //                   that we are currently doing the second try to split the
    //                   table, or it will be set to false under certain
    //                   conditions that are not suitable for splitting
    //                   the row.
    bool bSplitRowAllowed = bTryToSplit;
    if (bSplitRowAllowed && !pRow->IsRowSplitAllowed())
    {
        // A row larger than the entire page ought to be allowed to split regardless of setting,
        // otherwise it has hidden content and that makes no sense
        tools::Long nMaxHeight = FindPageFrame()->getFramePrintArea().Height();
        for (auto pBody = FindBodyFrame(); pBody; pBody = pBody->GetUpper()->FindBodyFrame())
        {
            if (pBody->IsPageBodyFrame())
                nMaxHeight = pBody->getFramePrintArea().Height();
        }
        if (pRow->getFrameArea().Height() > nMaxHeight)
            pRow->SetForceRowSplitAllowed( true );
        else
            bSplitRowAllowed = false;
    }
    // #i29438#
    // #i26945# - Floating screen objects no longer forbid
    // a splitting of the table row.
    // Special DoNotSplit case 1:
    // Search for sections inside pRow:
    if (bSplitRowAllowed && lcl_FindSectionsInRow(*pRow))
    {
        bSplitRowAllowed = false;
    }

    SwFlyFrame* pFly = FindFlyFrame();
    if (bSplitRowAllowed && pFly && pFly->IsFlySplitAllowed())
    {
        // The remaining size is less than the minimum row height, then don't even try to split the
        // row, just move it forward.
        const SwFormatFrameSize& rRowSize = pRow->GetFormat()->GetFrameSize();
        if (rRowSize.GetHeightSizeType() == SwFrameSize::Minimum)
        {
            SwTwips nMinHeight = rRowSize.GetHeight();
            if (nMinHeight > nRemainingSpaceForLastRow)
            {
                bSplitRowAllowed = false;

                if (!pRow->GetPrev() && aRectFnSet.GetHeight(pRow->getFrameArea()) > nRemainingSpaceForLastRow)
                {
                    // Split of pRow is not allowed, no previous row, the current row doesn't fit:
                    // that's a failure, we'll have to move forward instead.
                    return false;
                }
            }
        }
    }

    // #i29771#
    // To avoid loops, we do some checks before actually trying to split
    // the row. Maybe we should keep the next row in this table.
    // Note: This is only done if we are at the beginning of our upper
    bool bKeepNextRow = false;
    if ( nRowCount < nRepeat )
    {
        // First case: One of the repeated headline does not fit to the page anymore.
        // tdf#88496 Disable repeated headline (like for #i44910#) to avoid loops and
        // to fix interoperability problems (very long tables only with headline)
        // tdf#150149 except in multi-column sections, where it's possible to enlarge
        // the height of the section frame instead of using this fallback
        OSL_ENSURE( !GetIndPrev(), "Table is supposed to be at beginning" );
        if ( !IsInSct() )
        {
            // This would mean the layout modifies the doc model, so RowsToRepeat drops to 0 while
            // there are existing row frames with RepeatedHeadline == true. Avoid this at least
            // inside split flys, it would lead to a crash in SwTabFrame::MakeAll().
            if (!pFly || !pFly->IsFlySplitAllowed())
            {
                m_pTable->SetRowsToRepeat(0);
            }
            return false;
        }
        else
            bKeepNextRow = true;
    }
    else if ( !GetIndPrev() && nRepeat == nRowCount )
    {
        // Second case: The first non-headline row does not fit to the page.
        // If it is not allowed to be split, or it contains a sub-row that
        // is not allowed to be split, we keep the row in this table:
        if (bSplitRowAllowed)
        {
            // Check if there are (first) rows inside this row,
            // which are not allowed to be split.
            SwCellFrame* pLowerCell = static_cast<SwCellFrame*>(pRow->Lower());
            while ( pLowerCell )
            {
                if ( pLowerCell->Lower() && pLowerCell->Lower()->IsRowFrame() )
                {
                    const SwRowFrame* pLowerRow = static_cast<SwRowFrame*>(pLowerCell->Lower());
                    if ( !pLowerRow->IsRowSplitAllowed() &&
                        aRectFnSet.GetHeight(pLowerRow->getFrameArea()) > nRemainingSpaceForLastRow )
                    {
                        bKeepNextRow = true;
                        break;
                    }
                }
                pLowerCell = static_cast<SwCellFrame*>(pLowerCell->GetNext());
            }
        }
        else
            bKeepNextRow = true;
    }

    // Better keep the next row in this table:
    if ( bKeepNextRow )
    {
        pRow = GetFirstNonHeadlineRow();
        if ( pRow && pRow->IsRowSpanLine() && 0 == aRectFnSet.GetHeight(pRow->getFrameArea()) )
            pRow = static_cast<SwRowFrame*>(pRow->GetNext());
        if ( pRow )
        {
            pRow = static_cast<SwRowFrame*>(pRow->GetNext());
            ++nRowCount;
        }
    }

    // No more row to split or to move to follow table:
    if ( !pRow )
        return bRet;

    // We try to split the row if
    // - the attributes of the row are set accordingly and
    // - we are allowed to do so
    // - it should not be kept with the next row
    bSplitRowAllowed = bSplitRowAllowed && (!bTableRowKeep || !pRow->ShouldRowKeepWithNext());

    // Adjust pRow according to the keep-with-next attribute:
    if ( !bSplitRowAllowed && bTableRowKeep )
    {
        SwRowFrame* pTmpRow = static_cast<SwRowFrame*>(pRow->GetPrev());
        SwRowFrame* pOldRow = pRow;
        while ( pTmpRow && pTmpRow->ShouldRowKeepWithNext() &&
                nRowCount > nRepeat )
        {
            // Special case: pTmpRow wants to keep with pRow, but allows splitting, and some its
            // cells span several rows, including pRow. In this case, the "split" of the spanning
            // cells of the pTmpRow may still happen by moving pRow to the next page, even here
            // with !bSplitRowAllowed.
            if (pTmpRow->IsRowSplitAllowed())
            {
                bool bCellSpanCanSplit = false;
                for (auto pCellFrame = static_cast<const SwCellFrame*>(pTmpRow->GetLower());
                     pCellFrame;
                     pCellFrame = static_cast<const SwCellFrame*>(pCellFrame->GetNext()))
                {
                    if (pCellFrame->GetTabBox()->getRowSpan() > 1) // Master cell
                    {
                        bCellSpanCanSplit = true;
                        break;
                    }
                }
                if (bCellSpanCanSplit)
                    break;
            }
            pRow = pTmpRow;
            --nRowCount;
            pTmpRow = static_cast<SwRowFrame*>(pTmpRow->GetPrev());
        }

        // loop prevention
        if ( nRowCount == nRepeat && !GetIndPrev())
        {
            pRow = pOldRow;
        }
    }

    // If we do not intend to split pRow, we check if we are
    // allowed to move pRow to a follow. Otherwise we return
    // false, indicating an error
    if ( !bSplitRowAllowed )
    {
        SwRowFrame* pFirstNonHeadlineRow = GetFirstNonHeadlineRow();
        if ( pRow == pFirstNonHeadlineRow )
            return false;

        // #i91764#
        // Ignore row span lines
        SwRowFrame* pTmpRow = pFirstNonHeadlineRow;
        while ( pTmpRow && pTmpRow->IsRowSpanLine() )
        {
            pTmpRow = static_cast<SwRowFrame*>(pTmpRow->GetNext());
        }
        if ( !pTmpRow || pRow == pTmpRow )
        {
            return false;
        }
    }

    // Build follow table if not already done:
    bool bNewFollow;
    SwTabFrame *pFoll;
    if ( GetFollow() )
    {
        pFoll = GetFollow();
        bNewFollow = false;
    }
    else
    {
        bNewFollow = true;
        pFoll = new SwTabFrame( *this );

        // We give the follow table an initial width.
        {
            SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*pFoll);
            aRectFnSet.AddWidth(aFrm, aRectFnSet.GetWidth(getFrameArea()));
            aRectFnSet.SetLeft(aFrm, aRectFnSet.GetLeft(getFrameArea()));
        }

        {
            SwFrameAreaDefinition::FramePrintAreaWriteAccess aPrt(*pFoll);
            aRectFnSet.AddWidth(aPrt, aRectFnSet.GetWidth(getFramePrintArea()));
        }

        // Insert the new follow table
        pFoll->InsertBehind( GetUpper(), this );

        // Repeat the headlines.
        auto& rLines = GetTable()->GetTabLines();
        for ( nRowCount = 0; nRowCount < nRepeat; ++nRowCount )
        {
            // Insert new headlines:
            SwRowFrame* pHeadline = new SwRowFrame(*rLines[nRowCount], this);
            {
                sw::FlyCreationSuppressor aSuppressor;
                pHeadline->SetRepeatedHeadline(true);
            }
            pHeadline->InsertBefore( pFoll, nullptr );

            SwPageFrame *pPage = pHeadline->FindPageFrame();
            const sw::SpzFrameFormats* pSpzs = GetFormat()->GetDoc().GetSpzFrameFormats();
            if( !pSpzs->empty() )
            {
                SwNodeOffset nIndex;
                SwContentFrame* pFrame = pHeadline->ContainsContent();
                while( pFrame )
                {
                    // sw_redlinehide: the implementation of AppendObjs
                    // takes care of iterating merged SwTextFrame
                    nIndex = pFrame->IsTextFrame()
                        ? static_cast<SwTextFrame*>(pFrame)->GetTextNodeFirst()->GetIndex()
                        : static_cast<SwNoTextFrame*>(pFrame)->GetNode()->GetIndex();
                    AppendObjs(pSpzs, nIndex, pFrame, pPage, GetFormat()->GetDoc());
                    pFrame = pFrame->GetNextContentFrame();
                    if( !pHeadline->IsAnLower( pFrame ) )
                        break;
                }
            }
        }
    }

    SwRowFrame* pLastRow = nullptr;     // points to the last remaining line in master
    SwRowFrame* pFollowRow = nullptr;   // points to either the follow flow line or the
                                        // first regular line in the follow

    if ( bSplitRowAllowed )
    {
        // If the row that does not fit anymore is allowed
        // to be split, the next row has to be moved to the follow table.
        pLastRow = pRow;
        pRow = static_cast<SwRowFrame*>(pRow->GetNext());

        // new follow flow line for last row of master table
        pFollowRow = lcl_InsertNewFollowFlowLine( *this, *pLastRow, false );
    }
    else
    {
        pFollowRow = pRow;

        // NEW TABLES
        // check if we will break a row span by moving pFollowRow to the follow:
        // In this case we want to reformat the last line.
        const SwCellFrame* pCellFrame = static_cast<const SwCellFrame*>(pFollowRow->GetLower());
        while ( pCellFrame )
        {
            if ( pCellFrame->GetTabBox()->getRowSpan() < 1 )
            {
                pLastRow = static_cast<SwRowFrame*>(pRow->GetPrev());
                break;
            }

            pCellFrame = static_cast<const SwCellFrame*>(pCellFrame->GetNext());
        }

        // new follow flow line for last row of master table
        if ( pLastRow )
            pFollowRow = lcl_InsertNewFollowFlowLine( *this, *pLastRow, true );
    }

    const SwTwips nShrink = lcl_GetHeightOfRows(pRow, std::numeric_limits<tools::Long>::max());

    //Optimization: There is no paste needed for the new Follow and the
    //optimized insert can be used (large numbers of rows luckily only occur in
    //such situations).
    if ( bNewFollow )
    {
        SwFrame* pInsertBehind = pFoll->GetLastLower();

        while ( pRow )
        {
            SwFrame* pNxt = pRow->GetNext();
            // The footnotes do not have to be moved, this is done in the
            // MoveFwd of the follow table!!!
            pRow->RemoveFromLayout();
            pRow->InsertBehind( pFoll, pInsertBehind );
            pRow->InvalidateAll_();
            pInsertBehind = pRow;
            pRow = static_cast<SwRowFrame*>(pNxt);
        }
    }
    else
    {
        SwFrame* pPasteBefore = HasFollowFlowLine() ?
                              pFollowRow->GetNext() :
                              pFoll->GetFirstNonHeadlineRow();

        while ( pRow )
        {
            SwFrame* pNxt = pRow->GetNext();

            // The footnotes have to be moved:
            lcl_MoveFootnotes( *this, *GetFollow(), *pRow );

            pRow->RemoveFromLayout();
            pRow->Paste( pFoll, pPasteBefore );

            pRow->CheckDirChange();
            pRow = static_cast<SwRowFrame*>(pNxt);
        }
    }

    if ( !pLastRow )
        Shrink( nShrink );
    else
    {
        // we rebuild the last line to assure that it will be fully formatted
        // we also don't shrink here, because we will be doing that in lcl_RecalcSplitLine

        // recalculate the split line
        bRet = lcl_RecalcSplitLine(*pLastRow, *pFollowRow, getRemainingAfter(pLastRow->GetPrev()), nShrink, rIsFootnoteGrowth);

        // RecalcSplitLine did not work. In this case we conceal the split error:
        if (!bRet && !bSplitRowAllowed)
        {
            bRet = true;
        }

        // NEW TABLES
        // check if each cell in the row span line has a good height
        if ( bRet && pFollowRow->IsRowSpanLine() )
            lcl_AdjustRowSpanCells( pFollowRow );
    }

    return bRet;
}

namespace
{
    bool CanDeleteFollow(const SwTabFrame *pFoll)
    {
        if (pFoll->IsJoinLocked())
            return false;

        if (pFoll->IsDeleteForbidden())
        {
            SAL_WARN("sw.layout", "Delete Forbidden");
            return false;
        }

        return true;
    }

    auto IsAllHiddenSection(SwSectionFrame const& rSection) -> bool
    {
        if (rSection.IsHiddenNow())
            return true;
        for (SwFrame const* pFrame = rSection.Lower(); pFrame; pFrame = pFrame->GetNext())
        {
            if (pFrame->IsColumnFrame())
            {
                return false; // adds some padding
            }
            else if (pFrame->IsSctFrame())
            {
                assert(false); // these aren't nested?
                if (!IsAllHiddenSection(*static_cast<SwSectionFrame const*>(pFrame)))
                {
                    return false;
                }
            }
            else if (pFrame->IsTabFrame())
            {
                return false; // presumably
            }
            else if (pFrame->IsTextFrame())
            {
                if (!pFrame->IsHiddenNow())
                {
                    return false;
                }
            }
        }
        return true;
    }

    auto IsAllHiddenRow(SwRowFrame const& rRow, SwTabFrame const& rTab) -> bool;

    auto IsAllHiddenCell(SwCellFrame const& rCell, SwRowFrame const& rRow, SwTabFrame const& rTab) -> bool
    {
        for (SwFrame const* pFrame = rCell.Lower(); pFrame; pFrame = pFrame->GetNext())
        {
            if (pFrame->IsRowFrame())
            {
                if (!IsAllHiddenRow(*static_cast<SwRowFrame const*>(pFrame), rTab))
                {
                    return false;
                }
            }
            else if (pFrame->IsSctFrame())
            {
                if (!IsAllHiddenSection(*static_cast<SwSectionFrame const*>(pFrame)))
                {
                    return false;
                }
            }
            else if (pFrame->IsTabFrame())
            {
                return false; // presumably
            }
            else if (pFrame->IsTextFrame())
            {
                if (!pFrame->IsHiddenNow())
                {
                    return false;
                }
            }
        }
        const SwFrame* pLower = rCell.Lower();
        assert(pLower);
        if (rTab.IsCollapsingBorders() && pLower && !pLower->IsRowFrame())
        {
            if (rRow.GetTopMarginForLowers() != 0
                || rRow.GetBottomMarginForLowers() != 0)
            {
                return false;
            }
        }
        else
        {
            SwBorderAttrAccess border(SwFrame::GetCache(), &rCell);
            if (border.Get()->CalcTop() != 0 || border.Get()->CalcBottom() != 0)
            {
                return false;
            }
        }
        return true;
    }

    auto IsAllHiddenRow(SwRowFrame const& rRow, SwTabFrame const& rTab) -> bool
    {
        for (SwFrame const* pCell = rRow.Lower(); pCell; pCell = pCell->GetNext())
        {
            if (!IsAllHiddenCell(*static_cast<SwCellFrame const*>(pCell), rRow, rTab))
            {
                return false;
            }
        }
        return true;
    }

} // namespace

void SwTabFrame::Join()
{
    OSL_ENSURE( !HasFollowFlowLine(), "Joining follow flow line" );

    SwTabFrame *pFoll = GetFollow();

    if (!pFoll || !CanDeleteFollow(pFoll))
        return;

    SwRectFnSet aRectFnSet(this);
    pFoll->Cut();   //Cut out first to avoid unnecessary notifications.

    SwFrame *pRow = pFoll->GetFirstNonHeadlineRow(),
          *pNxt;

    SwFrame* pPrv = GetLastLower();

    SwTwips nHeight = 0;    //Total height of the inserted rows as return value.
    bool isAllHidden(true);

    while ( pRow )
    {
        pNxt = pRow->GetNext();
        nHeight += aRectFnSet.GetHeight(pRow->getFrameArea());
        if (nHeight != 0)
        {
            isAllHidden = false;
        }
        if (isAllHidden)
        {
            isAllHidden = IsAllHiddenRow(*static_cast<SwRowFrame *>(pRow), *this);
        }
        pRow->RemoveFromLayout();
        pRow->InvalidateAll_();
        pRow->InsertBehind( this, pPrv );
        pRow->CheckDirChange();
        pPrv = pRow;
        pRow = pNxt;
    }

    SetFollow( pFoll->GetFollow() );
    SetFollowFlowLine( pFoll->HasFollowFlowLine() );
    SwFrame::DestroyFrame(pFoll);

    Grow( nHeight );

    // In case the row does not have a height, Grow(nHeight) did nothing.
    // If this is not invalidated, subsequent follows may never be joined.
    // Try to guess if the height of the row will be 0.  If the document
    // was just loaded, it will be 0 in any case, but probably it's not a good
    // idea to join *all* follows for a newly loaded document, it would be
    // easier not to split the table in the first place; presumably it is split
    // because that improves performance.
    if (isAllHidden)
    {
        InvalidateSize_();
    }
}

static void SwInvalidatePositions( SwFrame *pFrame, tools::Long nBottom )
{
    // LONG_MAX == nBottom means we have to calculate all
    bool bAll = LONG_MAX == nBottom;
    SwRectFnSet aRectFnSet(pFrame);
    do
    {   pFrame->InvalidatePos_();
        pFrame->InvalidateSize_();
        if( pFrame->IsLayoutFrame() )
        {
            if ( static_cast<SwLayoutFrame*>(pFrame)->Lower() )
            {
                ::SwInvalidatePositions( static_cast<SwLayoutFrame*>(pFrame)->Lower(), nBottom);
                // #i26945#
                ::lcl_InvalidateLowerObjs( *static_cast<SwLayoutFrame*>(pFrame) );
            }
        }
        else
            pFrame->Prepare( PrepareHint::AdjustSizeWithoutFormatting );
        pFrame = pFrame->GetNext();
    } while ( pFrame &&
              ( bAll ||
              aRectFnSet.YDiff( aRectFnSet.GetTop(pFrame->getFrameArea()), nBottom ) < 0 ) );
}

void SwInvalidateAll( SwFrame *pFrame, tools::Long nBottom )
{
    // LONG_MAX == nBottom means we have to calculate all
    bool bAll = LONG_MAX == nBottom;
    SwRectFnSet aRectFnSet(pFrame);
    do
    {
        pFrame->InvalidatePos_();
        pFrame->InvalidateSize_();
        pFrame->InvalidatePrt_();
        if( pFrame->IsLayoutFrame() )
        {
            // NEW TABLES
            SwLayoutFrame* pToInvalidate = static_cast<SwLayoutFrame*>(pFrame);
            if (pFrame->IsCellFrame())
            {
                SwCellFrame* pThisCell = static_cast<SwCellFrame*>(pFrame);
                if ( pThisCell->GetTabBox()->getRowSpan() < 1 )
                {
                    pToInvalidate = & const_cast<SwCellFrame&>(pThisCell->FindStartEndOfRowSpanCell( true ));
                    pToInvalidate->InvalidatePos_();
                    pToInvalidate->InvalidateSize_();
                    pToInvalidate->InvalidatePrt_();
                }
            }
            if ( pToInvalidate->Lower() )
                ::SwInvalidateAll( pToInvalidate->Lower(), nBottom);
        }
        else
            pFrame->Prepare();

        pFrame = pFrame->GetNext();
    } while ( pFrame &&
              ( bAll ||
              aRectFnSet.YDiff( aRectFnSet.GetTop(pFrame->getFrameArea()), nBottom ) < 0 ) );
}

// #i29550#
static void lcl_InvalidateAllLowersPrt( SwLayoutFrame* pLayFrame )
{
    pLayFrame->InvalidatePrt_();
    pLayFrame->InvalidateSize_();
    pLayFrame->SetCompletePaint();

    SwFrame* pFrame = pLayFrame->Lower();

    while ( pFrame )
    {
        if ( pFrame->IsLayoutFrame() )
            lcl_InvalidateAllLowersPrt( static_cast<SwLayoutFrame*>(pFrame) );
        else
        {
            pFrame->InvalidatePrt_();
            pFrame->InvalidateSize_();
            pFrame->SetCompletePaint();
        }

        pFrame = pFrame->GetNext();
    }
}

bool SwContentFrame::CalcLowers(SwLayoutFrame & rLay, SwLayoutFrame const& rDontLeave,
                                 tools::Long nBottom, bool bSkipRowSpanCells )
{
    vcl::RenderContext* pRenderContext = rLay.getRootFrame()->GetCurrShell()->GetOut();
    // LONG_MAX == nBottom means we have to calculate all
    bool bAll = LONG_MAX == nBottom;
    bool bRet = false;
    SwContentFrame *pCnt = rLay.ContainsContent();
    SwRectFnSet aRectFnSet(&rLay);

    // FME 2007-08-30 #i81146# new loop control
    int nLoopControlRuns = 0;
    const int nLoopControlMax = 10;
    const sw::BroadcastingModify* pLoopControlCond = nullptr;

    while (pCnt && rDontLeave.IsAnLower(pCnt))
    {
        // #115759# - check, if a format of content frame is
        // possible. Thus, 'copy' conditions, found at the beginning of
        // <SwContentFrame::MakeAll(..)>, and check these.
        const bool bFormatPossible = !pCnt->IsJoinLocked() &&
                                     ( !pCnt->IsTextFrame() ||
                                       !static_cast<SwTextFrame*>(pCnt)->IsLocked() ) &&
                                     ( pCnt->IsFollow() || !StackHack::IsLocked() );

        // NEW TABLES
        bool bSkipContent = false;
        if ( bSkipRowSpanCells && pCnt->IsInTab() )
        {
            const SwFrame* pCell = pCnt->GetUpper();
            while ( pCell && !pCell->IsCellFrame() )
                pCell = pCell->GetUpper();
            if ( pCell && 1 != static_cast<const SwCellFrame*>( pCell )->GetLayoutRowSpan() )
                bSkipContent = true;
        }

        if ( bFormatPossible && !bSkipContent )
        {
            bRet |= !pCnt->isFrameAreaDefinitionValid();
            // #i26945# - no extra invalidation of floating
            // screen objects needed.
            // Thus, delete call of method <SwFrame::InvalidateObjs( true )>
            pCnt->Calc(pRenderContext);
            // #i46941# - frame has to be valid
            // Note: frame could be invalid after calling its format, if it's locked.
            OSL_ENSURE( !pCnt->IsTextFrame() ||
                    pCnt->isFrameAreaDefinitionValid() ||
                    static_cast<SwTextFrame*>(pCnt)->IsJoinLocked(),
                    "<SwContentFrame::CalcLowers(..)> - text frame invalid and not locked." );
            if ( pCnt->IsTextFrame() && pCnt->isFrameAreaDefinitionValid() )
            {
                // #i23129#, #i36347# - pass correct page frame to
                // the object formatter
                SwFrameDeleteGuard aDeleteGuard(pCnt);
                if ( !SwObjectFormatter::FormatObjsAtFrame( *pCnt,
                                                          *(pCnt->FindPageFrame()) ) )
                {
                    SwTextNode const*const pTextNode(
                        static_cast<SwTextFrame*>(pCnt)->GetTextNodeFirst());
                    if (pTextNode == pLoopControlCond)
                        ++nLoopControlRuns;
                    else
                    {
                        nLoopControlRuns = 0;
                        pLoopControlCond = pTextNode;
                    }

                    if ( nLoopControlRuns < nLoopControlMax )
                    {
                        // restart format with first content
                        pCnt = rLay.ContainsContent();
                        continue;
                    }

                    SAL_WARN("sw.layout", "LoopControl in SwContentFrame::CalcLowers");
                }
            }
            if (!rDontLeave.IsAnLower(pCnt)) // moved backward?
            {
                pCnt = rLay.ContainsContent();
                continue; // avoid formatting new upper on different page
            }
            pCnt->GetUpper()->Calc(pRenderContext);
        }
        if( ! bAll && aRectFnSet.YDiff(aRectFnSet.GetTop(pCnt->getFrameArea()), nBottom) > 0 )
            break;
        pCnt = pCnt->GetNextContentFrame();
    }
    return bRet;
}

bool SwContentFrame::IgnoringSplitFlyAnchor(bool bValue) const
{
    bool bRet = bValue;
    if (IsTextFrame())
    {
        auto pTextFrame = DynCastTextFrame();
        if (pTextFrame->HasSplitFlyDrawObjs())
        {
            // Turn off the value for split fly anchors.
            bRet = false;
        }
    }
    return bRet;
}

// #i26945# - add parameter <_bOnlyRowsAndCells> to control
// that only row and cell frames are formatted.
static bool lcl_InnerCalcLayout( SwFrame *pFrame,
                                      tools::Long nBottom,
                                      bool _bOnlyRowsAndCells )
{
    bool bRet = false;
    if (!pFrame)
    {
        return bRet;
    }

    vcl::RenderContext* pRenderContext = pFrame->getRootFrame()->GetCurrShell() ? pFrame->getRootFrame()->GetCurrShell()->GetOut() : nullptr;
    // LONG_MAX == nBottom means we have to calculate all
    bool bAll = LONG_MAX == nBottom;
    const SwFrame* pOldUp = pFrame->GetUpper();
    SwRectFnSet aRectFnSet(pFrame);
    do
    {
        // #i26945# - parameter <_bOnlyRowsAndCells> controls,
        // if only row and cell frames are formatted.
        if ( pFrame->IsLayoutFrame() &&
             ( !_bOnlyRowsAndCells || pFrame->IsRowFrame() || pFrame->IsCellFrame() ) )
        {
            SwFrameDeleteGuard aDeleteGuard(pFrame);

            // #130744# An invalid locked table frame will
            // not be calculated => It will not become valid =>
            // Loop in lcl_RecalcRow(). Therefore we do not consider them for bRet.
            bRet |= !pFrame->isFrameAreaDefinitionValid() && ( !pFrame->IsTabFrame() || !static_cast<SwTabFrame*>(pFrame)->IsJoinLocked() );
            pFrame->Calc(pRenderContext);
            if( static_cast<SwLayoutFrame*>(pFrame)->Lower() )
                bRet |= lcl_InnerCalcLayout( static_cast<SwLayoutFrame*>(pFrame)->Lower(), nBottom);

            // NEW TABLES
            if (pFrame->IsCellFrame())
            {
                SwCellFrame* pThisCell = static_cast<SwCellFrame*>(pFrame);
                if ( pThisCell->GetTabBox()->getRowSpan() < 1 )
                {
                    SwCellFrame& rToCalc = const_cast<SwCellFrame&>(pThisCell->FindStartEndOfRowSpanCell( true ));
                    bRet |= !rToCalc.isFrameAreaDefinitionValid();
                    rToCalc.Calc(pRenderContext);
                    if ( rToCalc.Lower() )
                        bRet |= lcl_InnerCalcLayout( rToCalc.Lower(), nBottom);
                }
            }
        }
        pFrame = pFrame->GetNext();
    } while( pFrame &&
            ( bAll ||
              aRectFnSet.YDiff(aRectFnSet.GetTop(pFrame->getFrameArea()), nBottom) < 0 )
            && pFrame->GetUpper() == pOldUp );
    return bRet;
}

static void lcl_RecalcRow(SwRowFrame & rRow, tools::Long const nBottom)
{
    // FME 2007-08-30 #i81146# new loop control
    int nLoopControlRuns_1 = 0;
    sal_uInt16 nLoopControlStage_1 = 0;
    const int nLoopControlMax = 10;

    bool bCheck = true;
    do
    {
        // FME 2007-08-30 #i81146# new loop control
        int nLoopControlRuns_2 = 0;
        sal_uInt16 nLoopControlStage_2 = 0;

        while (lcl_InnerCalcLayout(&rRow, nBottom))
        {
            if ( ++nLoopControlRuns_2 > nLoopControlMax )
            {
                SAL_WARN_IF(nLoopControlStage_2 == 0, "sw.layout", "LoopControl_2 in lcl_RecalcRow: Stage 1!");
                SAL_WARN_IF(nLoopControlStage_2 == 1, "sw.layout", "LoopControl_2 in lcl_RecalcRow: Stage 2!!");
                SAL_WARN_IF(nLoopControlStage_2 >= 2, "sw.layout", "LoopControl_2 in lcl_RecalcRow: Stage 3!!!");
                rRow.ValidateThisAndAllLowers( nLoopControlStage_2++ );
                nLoopControlRuns_2 = 0;
                if( nLoopControlStage_2 > 2 )
                    break;
            }

            bCheck = true;
        }

        if( bCheck )
        {
            SwFrameDeleteGuard g(&rRow);

            // #115759# - force another format of the
            // lowers, if at least one of it was invalid.
            bCheck = SwContentFrame::CalcLowers(rRow, *rRow.GetUpper(), nBottom, true);

            // NEW TABLES
            // First we calculate the cells with row span of < 1, afterwards
            // all cells with row span of > 1:
            for ( int i = 0; i < 2; ++i )
            {
                SwCellFrame* pCellFrame = static_cast<SwCellFrame*>(rRow.Lower());
                while ( pCellFrame )
                {
                    const bool bCalc = 0 == i ?
                                       pCellFrame->GetLayoutRowSpan() < 1 :
                                       pCellFrame->GetLayoutRowSpan() > 1;

                    if ( bCalc )
                    {
                        SwCellFrame& rToRecalc = 0 == i ?
                                               const_cast<SwCellFrame&>(pCellFrame->FindStartEndOfRowSpanCell( true )) :
                                               *pCellFrame;
                        bCheck |= SwContentFrame::CalcLowers(rToRecalc, rToRecalc, nBottom, false);
                    }

                    pCellFrame = static_cast<SwCellFrame*>(pCellFrame->GetNext());
                }
            }

            if ( bCheck )
            {
                if ( ++nLoopControlRuns_1 > nLoopControlMax )
                {
                    SAL_WARN_IF(nLoopControlStage_1 == 0, "sw.layout", "LoopControl_1 in lcl_RecalcRow: Stage 1!");
                    SAL_WARN_IF(nLoopControlStage_1 == 1, "sw.layout", "LoopControl_1 in lcl_RecalcRow: Stage 2!!");
                    SAL_WARN_IF(nLoopControlStage_1 >= 2, "sw.layout", "LoopControl_1 in lcl_RecalcRow: Stage 3!!!");
                    rRow.ValidateThisAndAllLowers( nLoopControlStage_1++ );
                    nLoopControlRuns_1 = 0;
                    if( nLoopControlStage_1 > 2 )
                        break;
                }

                continue;
            }
        }
        break;
    } while( true );
}

static void lcl_RecalcTable( SwTabFrame& rTab,
                                  SwLayoutFrame *pFirstRow,
                                  SwLayNotify &rNotify )
{
    SwFrame* pLower = rTab.Lower();
    if ( pLower )
    {
        if ( !pFirstRow )
        {
            pFirstRow = static_cast<SwLayoutFrame*>(pLower);
            rNotify.SetLowersComplete( true );
        }
        ::SwInvalidatePositions( pFirstRow, LONG_MAX );
        lcl_RecalcRow( *static_cast<SwRowFrame*>(pFirstRow), LONG_MAX );
    }
}

// This is a new function to check the first condition whether
// a tab frame may move backward. It replaces the formerly used
// GetIndPrev(), which did not work correctly for #i5947#
static bool lcl_NoPrev( const SwFrame& rFrame )
{
    // #i79774#
    // skip empty sections on investigation of direct previous frame.
    // use information, that at least one empty section is skipped in the following code.
    bool bSkippedDirectPrevEmptySection( false );
    if ( rFrame.GetPrev() )
    {
        const SwFrame* pPrev( rFrame.GetPrev() );
        while ( pPrev &&
                pPrev->IsSctFrame() &&
                !dynamic_cast<const SwSectionFrame&>(*pPrev).GetSection() )
        {
            pPrev = pPrev->GetPrev();
            bSkippedDirectPrevEmptySection = true;
        }
        if ( pPrev )
        {
            return false;
        }
    }

    if ( ( !bSkippedDirectPrevEmptySection && !rFrame.GetIndPrev() ) ||
         ( bSkippedDirectPrevEmptySection &&
           ( !rFrame.IsInSct() || !rFrame.GetIndPrev_() ) ) )
    {
        return true;
    }

    // I do not have a direct prev, but I have an indirect prev.
    // In section frames I have to check if I'm located inside
    // the first column:
    if ( rFrame.IsInSct() )
    {
        const SwFrame* pSct = rFrame.GetUpper();
        if ( pSct && pSct->IsColBodyFrame() &&
             pSct->GetUpper()->GetUpper()->IsSctFrame() )
        {
            const SwFrame* pPrevCol = rFrame.GetUpper()->GetUpper()->GetPrev();
            if ( pPrevCol )
                // I'm not inside the first column and do not have a direct
                // prev. I can try to go backward.
                return true;
        }
    }

    return false;
}

#define KEEPTAB ( !GetFollow() && !IsFollow() )

// - helper method to find next content frame of
// a table frame and format it to assure keep attribute.
// method return true, if a next content frame is formatted.
// Precondition: The given table frame hasn't a follow and isn't a follow.
SwFrame* sw_FormatNextContentForKeep( SwTabFrame* pTabFrame )
{
    vcl::RenderContext* pRenderContext = pTabFrame->getRootFrame()->GetCurrShell()->GetOut();
    // find next content, table or section
    SwFrame* pNxt = pTabFrame->FindNext();

    // skip empty sections
    while ( pNxt && pNxt->IsSctFrame() &&
            !static_cast<SwSectionFrame*>(pNxt)->GetSection() )
    {
        pNxt = pNxt->FindNext();
    }

    // if found next frame is a section, get its first content.
    if ( pNxt && pNxt->IsSctFrame() )
    {
        pNxt = static_cast<SwSectionFrame*>(pNxt)->ContainsAny();
    }

    // format found next frame.
    // if table frame is inside another table, method <SwFrame::MakeAll()> is
    // called to avoid that the superior table frame is formatted.
    if ( pNxt )
    {
        if ( pTabFrame->GetUpper()->IsInTab() )
            pNxt->MakeAll(pNxt->getRootFrame()->GetCurrShell()->GetOut());
        else
            pNxt->Calc(pRenderContext);
    }

    return pNxt;
}

namespace {
    bool AreAllRowsKeepWithNext( const SwRowFrame* pFirstRowFrame, const bool bCheckParents = true  )
    {
        bool bRet = pFirstRowFrame != nullptr &&
                    pFirstRowFrame->ShouldRowKeepWithNext( bCheckParents );

        while ( bRet && pFirstRowFrame->GetNext() != nullptr )
        {
            pFirstRowFrame = dynamic_cast<const SwRowFrame*>(pFirstRowFrame->GetNext());
            bRet = pFirstRowFrame != nullptr &&
                   pFirstRowFrame->ShouldRowKeepWithNext( bCheckParents );
        }

        return bRet;
    }

// Similar to SwObjPosOscillationControl in sw/source/core/layout/anchoreddrawobject.cxx
class PosSizeOscillationControl
{
public:
    bool OscillationDetected(const SwFrameAreaDefinition& rFrameArea);

private:
    // A partial copy of SwFrameAreaDefinition data
    struct FrameData
    {
        SwRect frameArea;
        SwRect framePrintArea;
        bool frameAreaPositionValid;
        bool frameAreaSizeValid;
        bool framePrintAreaValid;

        FrameData(const SwFrameAreaDefinition& src)
            : frameArea(src.getFrameArea())
            , framePrintArea(src.getFramePrintArea())
            , frameAreaPositionValid(src.isFrameAreaPositionValid())
            , frameAreaSizeValid(src.isFrameAreaSizeValid())
            , framePrintAreaValid(src.isFramePrintAreaValid())
        {
        }

        bool operator==(const SwFrameAreaDefinition& src) const
        {
            return frameArea == src.getFrameArea() && framePrintArea == src.getFramePrintArea()
                   && frameAreaPositionValid == src.isFrameAreaPositionValid()
                   && frameAreaSizeValid == src.isFrameAreaSizeValid()
                   && framePrintAreaValid == src.isFramePrintAreaValid();
        }
    };
    std::vector<FrameData> maFrameDatas;
};

bool PosSizeOscillationControl::OscillationDetected(const SwFrameAreaDefinition& rFrameArea)
{
    for (size_t i = 0; i < maFrameDatas.size(); ++i)
    {
        const auto& f = maFrameDatas[i];
        if (f == rFrameArea)
        {
            SAL_WARN("sw.layout",
                     "PosSize oscillation: frame " << i << " repeated; total frames " << maFrameDatas.size());
            return true;
        }
    }

    if (maFrameDatas.size() == 20) // stack is full -> oscillation
    {
        SAL_WARN("sw.layout", "PosSize oscillation: max frames");
        return true;
    }

    maFrameDatas.emplace_back(rFrameArea);
    return false;
}
}

// extern because static can't be friend
void FriendHackInvalidateRowFrame(SwFrameAreaDefinition & rRowFrame)
{
    // hilariously static_cast<SwTabFrame*>(GetLower()) would not require friend declaration, but it's UB...
    rRowFrame.setFrameAreaPositionValid(false);
}

static void InvalidateFramePositions(SwFrame * pFrame)
{
    while (pFrame)
    {
        if (pFrame->IsLayoutFrame())
        {
            InvalidateFramePositions(pFrame->GetLower());
        }
        else if (pFrame->IsTextFrame())
        {
            pFrame->Prepare(PrepareHint::FramePositionChanged);
        }
        pFrame = pFrame->GetNext();
    }
}

static bool IsFirstNonHeadlineRowDeleteForbidden(const SwTabFrame& rFoll)
{
    const SwFrame* pFirstRow = rFoll.GetFirstNonHeadlineRow();
    return pFirstRow && pFirstRow->IsDeleteForbidden();
}

void SwTabFrame::MakeAll(vcl::RenderContext* pRenderContext)
{
    if ( IsJoinLocked() || StackHack::IsLocked() || StackHack::Count() > 50 )
        return;

    if ( HasFollow() )
    {
        SwTabFrame* pFollowFrame = GetFollow();
        OSL_ENSURE( !pFollowFrame->IsJoinLocked() || !pFollowFrame->IsRebuildLastLine(),
                "SwTabFrame::MakeAll for master while follow is in RebuildLastLine()" );
        if ( pFollowFrame->IsJoinLocked() && pFollowFrame->IsRebuildLastLine() )
            return;
    }

    PROTOCOL_ENTER( this, PROT::MakeAll, DbgAction::NONE, nullptr )

    LockJoin(); //I don't want to be destroyed on the way.
    SwLayNotify aNotify( this );    //does the notification in the DTor
    // If pos is invalid, we have to call a SetInvaKeep at aNotify.
    // Otherwise the keep attribute would not work in front of a table.
    const bool bOldValidPos = isFrameAreaPositionValid();

    //If my neighbour is my Follow at the same time, I'll swallow it up.
    // OD 09.04.2003 #108698# - join all follows, which are placed on the
    // same page/column.
    // OD 29.04.2003 #109213# - join follow, only if join for the follow
    // is not locked. Otherwise, join will not be performed and this loop
    // will be endless.
    while ( GetNext() && GetNext() == GetFollow() &&
            CanDeleteFollow(GetFollow())
          )
    {
        if ( HasFollowFlowLine() )
            RemoveFollowFlowLine();
        Join();
    }

    // The bRemoveFollowFlowLinePending is set if the split attribute of the
    // last line is set:
    if ( IsRemoveFollowFlowLinePending() && HasFollowFlowLine() )
    {
        if ( RemoveFollowFlowLine() )
            Join();
        SetRemoveFollowFlowLinePending( false );
    }

    if (m_bResizeHTMLTable) //Optimized interplay with grow/shrink of the content
    {
        m_bResizeHTMLTable = false;
        SwHTMLTableLayout *pLayout = GetTable()->GetHTMLTableLayout();
        if ( pLayout )
            m_bCalcLowers = pLayout->Resize(
                            pLayout->GetBrowseWidthByTabFrame( *this ) );
    }

    // as long as bMakePage is true, a new page can be created (exactly once)
    bool bMakePage = true;
    // bMovedBwd gets set to true when the frame flows backwards
    bool bMovedBwd = false;
    // as long as bMovedFwd is false, the Frame may flow backwards (until
    // it has been moved forward once)
    bool bMovedFwd  = false;
    // gets set to true when the Frame is split
    bool bSplit = false;
    const bool bFootnotesInDoc = !GetFormat()->GetDoc().GetFootnoteIdxs().empty();
    const bool bFly     = IsInFly();

    std::optional<SwBorderAttrAccess> oAccess(std::in_place, SwFrame::GetCache(), this);
    const SwBorderAttrs *pAttrs = oAccess->Get();

    bool const isHiddenNow(IsHiddenNow());
    // All rows should keep together
    bool bDontSplit = isHiddenNow
                || (!IsFollow() && !GetFormat()->GetLayoutSplit().GetValue());

    // The number of repeated headlines
    const sal_uInt16 nRepeat = GetTable()->GetRowsToRepeat();

    // This flag indicates that we are allowed to try to split the
    // table rows.
    bool bTryToSplit = true;

    // Indicates that two individual rows may keep together, based on the keep
    // attribute set at the first paragraph in the first cell.
    bool bTableRowKeep = !bDontSplit && GetFormat()->GetDoc().GetDocumentSettingManager().get(DocumentSettingId::TABLE_ROW_KEEP);
    if (SwFlyFrame* pFly = FindFlyFrame())
    {
        if (pFly->IsFlySplitAllowed())
        {
            // Ignore the above text node -> row inheritance for floating tables.
            bTableRowKeep = false;
        }
        else if (!pFly->GetNextLink())
        {
            // If the fly is not allowed to split and is not chained, then it makes no sense to
            // split the table.
            bDontSplit = true;
        }
    }

    // The Magic Move: Used for the table row keep feature.
    // If only the last row of the table wants to keep (implicitly by setting
    // keep for the first paragraph in the first cell), and this table does
    // not have a next, the last line will be cut. Loop prevention: Only
    // one try.
    // WHAT IS THIS??? It "magically" hides last line (paragraph) in a table,
    // if first is set to keep with next???
    bool bLastRowHasToMoveToFollow = false;
    bool bLastRowMoveNoMoreTries = false;

    const bool bLargeTable = GetTable()->GetTabLines().size() > 64;  //arbitrary value, virtually guaranteed to be larger than one page.
    const bool bEmulateTableKeep = !bLargeTable && bTableRowKeep
        && !pAttrs->GetAttrSet().GetKeep().GetValue()
        && AreAllRowsKeepWithNext(GetFirstNonHeadlineRow(), /*bCheckParents=*/false);
    // The beloved keep attribute
    const bool bKeep{!isHiddenNow && IsKeep(pAttrs->GetAttrSet().GetKeep(), GetBreakItem(), bEmulateTableKeep)};

    // Join follow table, if this table is not allowed to split:
    if ( bDontSplit )
    {
        while ( GetFollow() && !GetFollow()->IsJoinLocked() )
        {
            if ( HasFollowFlowLine() )
                RemoveFollowFlowLine();
            Join();
        }
    }

    // Join follow table, if this does not have enough (repeated) lines:
    if ( nRepeat )
    {
        if( GetFollow() && !GetFollow()->IsJoinLocked() &&
            nullptr == GetFirstNonHeadlineRow() )
        {
            if ( HasFollowFlowLine() )
                RemoveFollowFlowLine();
            Join();
        }
    }

    // Join follow table, if last row of this table should keep:
    if ( bTableRowKeep && GetFollow() && !GetFollow()->IsJoinLocked() )
    {
        const SwRowFrame* pTmpRow = static_cast<const SwRowFrame*>(GetLastLower());
        if ( pTmpRow && pTmpRow->ShouldRowKeepWithNext() )
        {
            // Special case: pTmpRow wants to keep with next, but allows splitting, and some its
            // cells span several rows - i.e., also the next row. In this case, the "split" of the
            // spanning cells of the pTmpRow may still happen by moving next row to the next page,
            // even here with bTableRowKeep.
            bool bCellSpanCanSplit = false;
            if (pTmpRow->IsRowSplitAllowed())
            {
                for (auto pCellFrame = static_cast<const SwCellFrame*>(pTmpRow->GetLower());
                     pCellFrame;
                     pCellFrame = static_cast<const SwCellFrame*>(pCellFrame->GetNext()))
                {
                    if (pCellFrame->GetTabBox()->getRowSpan() > 1) // Master cell
                    {
                        bCellSpanCanSplit = true;
                        break;
                    }
                }
            }

            if (!bCellSpanCanSplit)
            {
                if (HasFollowFlowLine())
                    RemoveFollowFlowLine();
                Join();
            }
        }
    }

    // a new one is moved forwards immediately
    if ( !getFrameArea().Top() && IsFollow() )
    {
        SwFrame *pPre = GetPrev();
        if ( pPre && pPre->IsTabFrame() && static_cast<SwTabFrame*>(pPre)->GetFollow() == this)
        {
            // don't make the effort to move fwd if its known
            // conditions that are known not to work
            if (IsInFootnote() && ForbiddenForFootnoteCntFwd())
                bMakePage = false;
            else if (!MoveFwd(bMakePage, false))
                bMakePage = false;
            bMovedFwd = true;
        }
    }

    int nUnSplitted = 5; // Just another loop control :-(
    int nThrowAwayValidLayoutLimit = 5; // And another one :-(
    PosSizeOscillationControl posSizeOscillationControl; // And yet another one.
    SwRectFnSet aRectFnSet(this);
    while ( !isFrameAreaPositionValid() || !isFrameAreaSizeValid() || !isFramePrintAreaValid() )
    {
        const bool bMoveable = IsMoveable();
        if (bMoveable &&
            !(bMovedFwd && bEmulateTableKeep) )
            if ( CheckMoveFwd( bMakePage, bKeep && KEEPTAB, bEmulateTableKeep ) )
            {
                bMovedFwd = true;
                m_bCalcLowers = true;
                // #i99267#
                // reset <bSplit> after forward move to assure that follows
                // can be joined, if further space is available.
                bSplit = false;
            }

        Point aOldPos( aRectFnSet.GetPos(getFrameArea()) );
        MakePos();

        if (isHiddenNow)
        {   // after MakePos()
            MakeValidZeroHeight();
        }

        if ( aOldPos != aRectFnSet.GetPos(getFrameArea()) )
        {
            if ( aOldPos.Y() != aRectFnSet.GetTop(getFrameArea()) )
            {
                SwHTMLTableLayout *pLayout = GetTable()->GetHTMLTableLayout();
                if( pLayout )
                {
                    oAccess.reset();
                    m_bCalcLowers |= pLayout->Resize(
                        pLayout->GetBrowseWidthByTabFrame( *this ) );
                }

                if (!isHiddenNow)
                {
                    setFramePrintAreaValid(false);
                }
                aNotify.SetLowersComplete( false );
            }
            SwFrame const*const pPre{bKeep ? nullptr : FindPrevIgnoreHidden()};
            if (bKeep || (nullptr != pPre && pPre->GetAttrSet()->GetKeep().GetValue()))
            {
                m_bCalcLowers = true;
            }
            if (GetLower())
            {   // it's possible that the rows already have valid pos - but it is surely wrong if the table's pos changed!
                FriendHackInvalidateRowFrame(*GetLower());
                // invalidate text frames to get rid of their SwFlyPortions
                InvalidateFramePositions(GetLower());
            }
        }

        //We need to know the height of the first row, because the master needs
        //to be invalidated if it shrinks and then absorb the row if possible.
        tools::Long n1StLineHeight = 0;
        if ( IsFollow() )
        {
            SwFrame* pFrame = GetFirstNonHeadlineRow();
            if ( pFrame )
                n1StLineHeight = aRectFnSet.GetHeight(pFrame->getFrameArea());
        }

        if ( !isFrameAreaSizeValid() || !isFramePrintAreaValid() )
        {
            const tools::Long nOldPrtWidth = aRectFnSet.GetWidth(getFramePrintArea());
            const tools::Long nOldFrameWidth = aRectFnSet.GetWidth(getFrameArea());
            const Point aOldPrtPos  = aRectFnSet.GetPos(getFramePrintArea());

            if (!oAccess)
            {
                oAccess.emplace(SwFrame::GetCache(), this);
                pAttrs = oAccess->Get();
            }
            Format( getRootFrame()->GetCurrShell()->GetOut(), pAttrs );

            SwHTMLTableLayout *pLayout = GetTable()->GetHTMLTableLayout();
            if ( pLayout &&
                (aRectFnSet.GetWidth(getFramePrintArea()) != nOldPrtWidth ||
                aRectFnSet.GetWidth(getFrameArea()) != nOldFrameWidth) )
            {
                oAccess.reset();
                m_bCalcLowers |= pLayout->Resize(
                    pLayout->GetBrowseWidthByTabFrame( *this ) );
            }
            if ( aOldPrtPos != aRectFnSet.GetPos(getFramePrintArea()) )
                aNotify.SetLowersComplete( false );
        }

        // If this is the first one in a chain, check if this can flow
        // backwards (if this is movable at all).
        // To prevent oscillations/loops, check that this has not just
        // flowed forwards.
        if ( !bMovedFwd && (bMoveable || bFly) && lcl_NoPrev( *this ) )
        {
            // for Follows notify Master.
            // only move Follow if it has to skip empty pages.
            if ( IsFollow() )
            {
                // Only if the height of the first line got smaller.
                SwFrame *pFrame = GetFirstNonHeadlineRow();
                if( pFrame && n1StLineHeight >aRectFnSet.GetHeight(pFrame->getFrameArea()) )
                {
                    SwTabFrame *pMaster = FindMaster();
                    bool bDummy;
                    if ( ShouldBwdMoved( pMaster->GetUpper(), bDummy ) )
                        pMaster->InvalidatePos();
                }
            }
            SwFootnoteBossFrame *pOldBoss = bFootnotesInDoc ? FindFootnoteBossFrame( true ) : nullptr;
            bool bReformat;
            std::optional<SfxDeleteListener> oDeleteListener;
            if (pOldBoss)
                oDeleteListener.emplace(*pOldBoss);
            SwFrameDeleteGuard g(this);
            if ( MoveBwd( bReformat ) )
            {
                SAL_WARN_IF(oDeleteListener && oDeleteListener->WasDeleted(), "sw.layout", "SwFootnoteBossFrame unexpectedly deleted");

                aRectFnSet.Refresh(this);
                bMovedBwd = true;
                aNotify.SetLowersComplete( false );
                if (bFootnotesInDoc && !oDeleteListener->WasDeleted())
                    MoveLowerFootnotes( nullptr, pOldBoss, nullptr, true );
                if ( bReformat || bKeep )
                {
                    tools::Long nOldTop = aRectFnSet.GetTop(getFrameArea());
                    MakePos();
                    if( nOldTop != aRectFnSet.GetTop(getFrameArea()) )
                    {
                        SwHTMLTableLayout *pHTMLLayout =
                            GetTable()->GetHTMLTableLayout();
                        if( pHTMLLayout )
                        {
                            oAccess.reset();
                            m_bCalcLowers |= pHTMLLayout->Resize(
                                pHTMLLayout->GetBrowseWidthByTabFrame( *this ) );
                        }

                        if (!isHiddenNow)
                        {
                            setFramePrintAreaValid(false);
                        }

                        if (!oAccess)
                        {
                            oAccess.emplace(SwFrame::GetCache(), this);
                            pAttrs = oAccess->Get();
                        }
                        Format( getRootFrame()->GetCurrShell()->GetOut(), pAttrs );
                    }

                    oAccess.reset();

                    lcl_RecalcTable( *this, nullptr, aNotify );

                    m_bLowersFormatted = true;
                    if ( bKeep && KEEPTAB )
                    {

                        // Consider case that table is inside another table,
                        // because it has to be avoided, that superior table
                        // is formatted.
                        // Thus, find next content, table or section
                        // and, if a section is found, get its first
                        // content.
                        if ( nullptr != sw_FormatNextContentForKeep( this ) && !GetNext() )
                        {
                            setFrameAreaPositionValid(false);
                        }
                    }
                }
            }
        }

        //Again an invalid value? - do it again...
        if ( !isFrameAreaPositionValid() || !isFrameAreaSizeValid() || !isFramePrintAreaValid() )
            continue;

        // check, if calculation of table frame is ready.

        // Local variable <nDistanceToUpperPrtBottom>
        //     Introduce local variable and init it with the distance from the
        //     table frame bottom to the bottom of the upper printing area.
        // Note: negative values denotes the situation that table frame doesn't fit in its upper.
        SwTwips nDistanceToUpperPrtBottom =
                aRectFnSet.BottomDist(getFrameArea(), aRectFnSet.GetPrtBottom(*GetUpper()));

        /// In online layout try to grow upper of table frame, if table frame doesn't fit in its upper.
        const SwViewShell *pSh = getRootFrame()->GetCurrShell();
        const bool bBrowseMode = pSh && pSh->GetViewOptions()->getBrowseMode();
        if ( nDistanceToUpperPrtBottom < 0 && bBrowseMode )
        {
            if ( GetUpper()->Grow( -nDistanceToUpperPrtBottom ) )
            {
                // upper is grown --> recalculate <nDistanceToUpperPrtBottom>
                nDistanceToUpperPrtBottom = aRectFnSet.BottomDist(getFrameArea(), aRectFnSet.GetPrtBottom(*GetUpper()));
            }
        }

        if (GetFollow() && GetUpper()->IsFlyFrame())
        {
            auto pUpper = static_cast<SwFlyFrame*>(GetUpper());
            if (pUpper->IsFlySplitAllowed())
            {
                // We have a follow tab frame that may be joined, and we're directly in a split fly.
                // See if the fly could grow.
                SwTwips nTest = GetUpper()->Grow(LONG_MAX, /*bTst=*/true);
                if (nTest >= aRectFnSet.GetHeight(GetFollow()->getFrameArea()))
                {
                    // We have space to join at least one follow tab frame.
                    SwTwips nRequest = 0;
                    for (SwTabFrame* pFollow = GetFollow(); pFollow; pFollow = pFollow->GetFollow())
                    {
                        nRequest += aRectFnSet.GetHeight(pFollow->getFrameArea());
                    }
                    // Try to grow the split fly to join all follows.
                    pUpper->Grow(nRequest);
                    // Determine what is space we actually got from the requested space.
                    nDistanceToUpperPrtBottom = aRectFnSet.BottomDist(getFrameArea(), aRectFnSet.GetPrtBottom(*pUpper));
                }
            }
        }

        // If there is still some space left in the upper, we check if we
        // can join some rows of the follow.
        // Setting bLastRowHasToMoveToFollow to true means we want to force
        // the table to be split! Only skip this if condition once.
        if( nDistanceToUpperPrtBottom >= 0 && !bLastRowHasToMoveToFollow )
        {
            // If there is space left in the upper printing area, join as for trial
            // at least one further row of an existing follow.
            if ( !bSplit && GetFollow() )
            {
                bool bDummy;
                if (!(HasFollowFlowLine() && IsFirstNonHeadlineRowDeleteForbidden(*GetFollow()))
                    && GetFollow()->ShouldBwdMoved(GetUpper(), bDummy))
                {
                    SwFrame *pTmp = GetUpper();
                    SwTwips nDeadLine = aRectFnSet.GetPrtBottom(*pTmp);
                    if ( bBrowseMode )
                        nDeadLine += pTmp->Grow( LONG_MAX, true );
                    bool bFits = aRectFnSet.BottomDist(getFrameArea(), nDeadLine) > 0;
                    if (!bFits && aRectFnSet.GetHeight(GetFollow()->getFrameArea()) == 0)
                        // The follow should move backwards, so allow the case
                        // when the upper has no space, but the follow is
                        // empty.
                        bFits = aRectFnSet.BottomDist(getFrameArea(), nDeadLine) >= 0;

                    if (bFits)
                    {
                        // The follow table's wants to move backwards, see if the first row has a
                        // split fly anchored in it that would have more space than what we have:
                        SwRowFrame* pRow = GetFollow()->GetFirstNonHeadlineRow();
                        if (pRow)
                        {
                            SwPageFrame* pPage = GetFollow()->FindPageFrame();
                            SwSortedObjs* pPageObjs = pPage->GetSortedObjs();
                            if (pPageObjs)
                            {
                                bool bSplitFly = false;
                                for (size_t i = 0; i < pPageObjs->size(); ++i)
                                {
                                    SwAnchoredObject* pAnchoredObj = (*pPage->GetSortedObjs())[i];
                                    auto pFly = pAnchoredObj->DynCastFlyFrame();
                                    if (!pFly || !pFly->IsFlySplitAllowed())
                                    {
                                        continue;
                                    }

                                    SwFrame* pFlyAnchor = pFly->FindAnchorCharFrame();
                                    if (!pFlyAnchor || !pRow->IsAnLower(pFlyAnchor))
                                    {
                                        continue;
                                    }

                                    bSplitFly = true;
                                    break;
                                }
                                SwTwips nFollowFirstRowHeight = aRectFnSet.GetHeight(pRow->getFrameArea());
                                SwTwips nSpace = aRectFnSet.BottomDist(getFrameArea(), nDeadLine);
                                if (bSplitFly && nFollowFirstRowHeight > 0 && nSpace < nFollowFirstRowHeight)
                                {
                                    // The row has at least one split fly and the row would not fit
                                    // to our remaining space, when also taking flys into account,
                                    // so that's not a fit.
                                    bFits = false;
                                }
                            }
                        }
                    }

                    if (bFits)
                    {
                        // First, we remove an existing follow flow line.
                        if ( HasFollowFlowLine() )
                        {
                            SwFrame* pLastLine = GetLastLower();
                            RemoveFollowFlowLine();
                            // invalidate and rebuild last row
                            if ( pLastLine )
                            {
                                ::SwInvalidateAll( pLastLine, LONG_MAX );
                                SetRebuildLastLine( true );
                                lcl_RecalcRow(*static_cast<SwRowFrame*>(pLastLine), LONG_MAX);
                                InvalidateVertOrientCells(*static_cast<SwRowFrame *>(pLastLine));
                                SetRebuildLastLine( false );
                            }

                            SwFrame* pRow = GetFollow()->GetFirstNonHeadlineRow();

                            if ( !pRow || !pRow->GetNext() )
                                // The follow became empty and hence useless
                                Join();

                            continue;
                        }

                        // If there is no follow flow line, we move the first
                        // row in the follow table to the master table.
                        SwRowFrame *pRow = GetFollow()->GetFirstNonHeadlineRow();

                        // The follow became empty and hence useless
                        if ( !pRow )
                        {
                            Join();
                            continue;
                        }

                        const SwTwips nOld = aRectFnSet.GetHeight(getFrameArea());
                        tools::Long nRowsToMove = lcl_GetMaximumLayoutRowSpan( *pRow );
                        SwFrame* pRowToMove = pRow;

                        while ( pRowToMove && nRowsToMove-- > 0 )
                        {
                            const bool bMoveFootnotes = bFootnotesInDoc && !GetFollow()->IsJoinLocked();

                            SwFootnoteBossFrame *pOldBoss = nullptr;
                            if ( bMoveFootnotes )
                                pOldBoss = pRowToMove->FindFootnoteBossFrame( true );

                            SwFrame* pNextRow = pRowToMove->GetNext();

                            if ( !pNextRow )
                            {
                                // The follow became empty and hence useless
                                Join();
                            }
                            else
                            {
                                pRowToMove->Cut();
                                pRowToMove->Paste( this );
                            }

                            // Move the footnotes!
                            if ( bMoveFootnotes )
                                if ( static_cast<SwLayoutFrame*>(pRowToMove)->MoveLowerFootnotes( nullptr, pOldBoss, FindFootnoteBossFrame( true ), true ) )
                                    GetUpper()->Calc(pRenderContext);

                            pRowToMove = pNextRow;
                        }

                        if ( nOld != aRectFnSet.GetHeight(getFrameArea()) )
                            lcl_RecalcTable( *this, static_cast<SwLayoutFrame*>(pRow), aNotify );

                        continue;
                    }
                }
            }
            else if ( KEEPTAB )
            {
                bool bFormat = false;
                if ( bKeep )
                    bFormat = true;
                else if ( bTableRowKeep && !bLastRowMoveNoMoreTries )
                {
                    // We only want to give the last row one chance to move
                    // to the follow table. Set the flag as early as possible:
                    bLastRowMoveNoMoreTries = true;

                    // The last line of the table has to be cut off if:
                    // 1. The table does not want to keep with its next
                    // 2. The compatibility option is set and the table is allowed to split
                    // 3. We did not already cut off the last row
                    // 4. There is not break after attribute set at the table
                    // 5. There is no break before attribute set behind the table
                    // 6. There is no section change behind the table (see IsKeep)
                    // 7. The last table row wants to keep with its next.
                    const SwRowFrame* pLastRow = static_cast<const SwRowFrame*>(GetLastLower());
                    if (pLastRow)
                    {
                        if (!oAccess)
                        {
                            oAccess.emplace(SwFrame::GetCache(), this);
                            pAttrs = oAccess->Get();
                        }
                        if (!isHiddenNow
                            && IsKeep(pAttrs->GetAttrSet().GetKeep(), GetBreakItem(), true)
                            && pLastRow->ShouldRowKeepWithNext())
                        {
                            bFormat = true;
                        }
                    }
                }

                if ( bFormat )
                {
                    oAccess.reset();

                    // Consider case that table is inside another table, because
                    // it has to be avoided, that superior table is formatted.
                    // Thus, find next content, table or section and, if a section
                    // is found, get its first content.
                    const SwFrame* pTmpNxt = sw_FormatNextContentForKeep( this );

                    // The last row wants to keep with the frame behind the table.
                    // Check if the next frame is on a different page and valid.
                    // In this case we do a magic trick:
                    if ( !bKeep && !GetNext() && pTmpNxt && pTmpNxt->isFrameAreaDefinitionValid() )
                    {
                        setFrameAreaPositionValid(false);
                        bLastRowHasToMoveToFollow = true;
                    }
                }
            }

            if ( isFrameAreaDefinitionValid() )
            {
                if (m_bCalcLowers)
                {
                    lcl_RecalcTable( *this, nullptr, aNotify );
                    m_bLowersFormatted = true;
                    m_bCalcLowers = false;
                }
                else if (m_bONECalcLowers)
                {
                    // tdf#147526 is a case of a macro which results in a null Lower() result
                    if (SwRowFrame* pLower = static_cast<SwRowFrame*>(Lower()))
                        lcl_RecalcRow(*pLower, LONG_MAX);
                    m_bONECalcLowers = false;
                }
            }
            continue;
        }

        // I don't fit in the upper Frame anymore, therefore it's the
        // right moment to do some preferably constructive changes.

        // If I'm NOT allowed to leave the upper Frame, I've got a problem.
        // Following Arthur Dent, we do the only thing that you can do with
        // an unsolvable problem: We ignore it with all our power.
        if ( !bMoveable )
        {
            if (m_bCalcLowers && isFrameAreaDefinitionValid())
            {
                lcl_RecalcTable( *this, nullptr, aNotify );
                m_bLowersFormatted = true;
                m_bCalcLowers = false;
            }
            else if (m_bONECalcLowers)
            {
                lcl_RecalcRow(*static_cast<SwRowFrame*>(Lower()), LONG_MAX);
                m_bONECalcLowers = false;
            }

            // It does not make sense to cut off the last line if we are
            // not moveable:
            bLastRowHasToMoveToFollow = false;

            continue;
        }

        if (m_bCalcLowers && isFrameAreaDefinitionValid())
        {
            lcl_RecalcTable( *this, nullptr, aNotify );
            m_bLowersFormatted = true;
            m_bCalcLowers = false;
            if( !isFrameAreaDefinitionValid() )
                continue;
        }

        // First try to split the table. Condition:
        // 1. We have at least one non headline row
        // 2. If this row wants to keep, we need an additional row
        // 3. The table is allowed to split or we do not have a pIndPrev:
        SwFrame* pIndPrev = GetIndPrev();

        SwFlyFrame* pFly = FindFlyFrame();
        if (!pIndPrev && pFly && pFly->IsFlySplitAllowed())
        {
            auto pFlyAtContent = static_cast<SwFlyAtContentFrame*>(pFly);
            SwFrame* pAnchor = pFlyAtContent->FindAnchorCharFrame();
            if (pAnchor)
            {
                // If the anchor of the split has a previous frame, we're allowed to move forward.
                pIndPrev = pAnchor->GetIndPrev();
            }
        }

        const SwRowFrame* pFirstNonHeadlineRow = GetFirstNonHeadlineRow();
        // #i120016# if this row wants to keep, allow split in case that all rows want to keep with next,
        // the table can not move forward as it is the first one and a split is in general allowed.
        const bool bAllowSplitOfRow = bTableRowKeep && !pIndPrev && AreAllRowsKeepWithNext(pFirstNonHeadlineRow);
        // tdf91083 MSCompat: this extends bAllowSplitOfRow (and perhaps should just replace it).
        // If the kept-together items cannot move to a new page, a table split is in general allowed.
        const bool bEmulateTableKeepSplitAllowed =  bEmulateTableKeep && !IsKeepFwdMoveAllowed(/*IgnoreMyOwnKeepValue=*/true);

        if ( pFirstNonHeadlineRow && nUnSplitted > 0 &&
             ( bEmulateTableKeepSplitAllowed || bAllowSplitOfRow ||
               ( ( !bTableRowKeep || pFirstNonHeadlineRow->GetNext() ||
                   !pFirstNonHeadlineRow->ShouldRowKeepWithNext()
                 ) && ( !bDontSplit || !pIndPrev )
           ) ) )
        {
            // #i29438#
            // Special DoNotSplit cases:
            // We better avoid splitting of a row frame if we are inside a columned
            // section which has a height of 0, because this is not growable and thus
            // all kinds of unexpected things could happen.
            if ( IsInSct() && FindSctFrame()->Lower()->IsColumnFrame() &&
                 0 == aRectFnSet.GetHeight(GetUpper()->getFrameArea())
               )
            {
                bTryToSplit = false;
            }

            // 1. Try: bTryToSplit = true  => Try to split the row.
            // 2. Try: bTryToSplit = false => Split the table between the rows.
            if ( pFirstNonHeadlineRow->GetNext() || bTryToSplit )
            {
                SwTwips nDeadLine = aRectFnSet.GetPrtBottom(*GetUpper());
                bool bFlySplit = false;
                if (GetUpper()->IsFlyFrame())
                {
                    // See if this is a split fly that can also grow.
                    auto pUpperFly = static_cast<SwFlyFrame*>(GetUpper());
                    bFlySplit = pUpperFly->IsFlySplitAllowed();

                    if (bFlySplit && bTryToSplit)
                    {
                        // This is a split fly that wants to split the row itself. See if it's also
                        // nested. If so, we'll want to know if the row split has rowspans.
                        SwTextFrame* pAnchorCharFrame = pUpperFly->FindAnchorCharFrame();
                        if (pAnchorCharFrame && pAnchorCharFrame->IsInFly())
                        {
                            // Find the row we'll split.
                            SwTwips nRemaining
                                = aRectFnSet.YDiff(nDeadLine, aRectFnSet.GetTop(getFrameArea()));
                            nRemaining -= aRectFnSet.GetTopMargin(*this);
                            const SwFrame* pRow = Lower();
                            for (; pRow->GetNext(); pRow = pRow->GetNext())
                            {
                                if (nRemaining < aRectFnSet.GetHeight(pRow->getFrameArea()))
                                {
                                    break;
                                }

                                nRemaining -= aRectFnSet.GetHeight(pRow->getFrameArea());
                            }
                            // See if any cells have rowspans.
                            for (const SwFrame* pLower = pRow->GetLower(); pLower;
                                 pLower = pLower->GetNext())
                            {
                                auto pCellFrame = static_cast<const SwCellFrame*>(pLower);
                                if (pCellFrame->GetTabBox()->getRowSpan() != 1)
                                {
                                    // The cell has a rowspan, don't split the row itself in this
                                    // case (but just move it forward, i.e. split between the rows).
                                    bTryToSplit = false;
                                    break;
                                }
                            }
                        }
                    }
                }
                if( IsInSct() || GetUpper()->IsInTab() || bFlySplit )
                {
                    auto const nGrow{GetUpper()->Grow(LONG_MAX, true)};
                    if (nGrow != 0)
                    {
                        if (GetUpper()->IsSctFrame()) // what about table cell?
                        {
                            GetUpper()->InvalidateSize_();
                        }
                        nDeadLine = aRectFnSet.YInc(nDeadLine, nGrow);
                    }
                }

                {
                    SwFrameDeleteGuard g(Lower()); // tdf#134965 prevent RemoveFollowFlowLine()
                    SetInRecalcLowerRow( true );
                    ::lcl_RecalcRow(*static_cast<SwRowFrame*>(Lower()), nDeadLine);
                    SetInRecalcLowerRow( false );
                }
                m_bLowersFormatted = true;
                aNotify.SetLowersComplete( true );

                // One more check if it's really necessary to split the table.
                // 1. The table either has to exceed the deadline or
                // 2. We explicitly want to cut off the last row.
                if( aRectFnSet.BottomDist( getFrameArea(), nDeadLine ) > 0 && !bLastRowHasToMoveToFollow )
                {
                    continue;
                }

                // Set to false again as early as possible.
                bLastRowHasToMoveToFollow = false;

                // #i52781#
                // YaSC - Yet another special case:
                // If our upper is inside a table cell which is not allowed
                // to split, we do not try to split:
                if ( GetUpper()->IsInTab() )
                {
                    const SwFrame* pTmpRow = GetUpper();
                    while ( pTmpRow && !pTmpRow->IsRowFrame() )
                       pTmpRow = pTmpRow->GetUpper();
                    if ( pTmpRow && !static_cast<const SwRowFrame*>(pTmpRow)->IsRowSplitAllowed() )
                        continue;
                }

                sal_uInt16 nMinNumOfLines = nRepeat;

                if ( bTableRowKeep )
                {
                    const SwRowFrame* pTmpRow = GetFirstNonHeadlineRow();
                    // Copying the "NEW TABLES" comment, apparently related to commit dcb682563b24bf487a40a9fe7710b4d500850a52
                    if (pTmpRow && pTmpRow->IsRowSpanLine())
                    {
                        ++nMinNumOfLines;
                        pTmpRow = static_cast<const SwRowFrame*>(pTmpRow->GetNext());
                    }

                    while ( pTmpRow && pTmpRow->ShouldRowKeepWithNext() )
                    {
                        ++nMinNumOfLines;
                        pTmpRow = static_cast<const SwRowFrame*>(pTmpRow->GetNext());
                    }
                }

                if ( !bTryToSplit )
                    ++nMinNumOfLines;

                const SwTwips nBreakLine = aRectFnSet.YInc(
                        aRectFnSet.GetTop(getFrameArea()),
                        aRectFnSet.GetTopMargin(*this) +
                         lcl_GetHeightOfRows( GetLower(), nMinNumOfLines ) );

                bool bHadFollowFlowLineBeforeSplit = false;
                // Some more checks if we want to call the split algorithm or not:
                // The repeating lines / keeping lines still fit into the upper or
                // if we do not have an (in)direct Prev, we split anyway.
                if( aRectFnSet.YDiff(nDeadLine, nBreakLine) >=0
                    || !pIndPrev || bEmulateTableKeepSplitAllowed )
                {
                    aNotify.SetLowersComplete( false );
                    bSplit = true;

                    // An existing follow flow line has to be removed.
                    if ( HasFollowFlowLine() )
                    {
                        if (!nThrowAwayValidLayoutLimit)
                            continue;
                        const bool bInitialLoopEndCondition(isFrameAreaDefinitionValid());
                        bHadFollowFlowLineBeforeSplit = true;
                        RemoveFollowFlowLine();
                        const bool bFinalLoopEndCondition(isFrameAreaDefinitionValid());

                        if (bInitialLoopEndCondition && !bFinalLoopEndCondition)
                        {
                            --nThrowAwayValidLayoutLimit;
                        }
                    }

                    oAccess.reset();
                    bool isFootnoteGrowth(false);
                    bool bEffectiveTableRowKeep;
                    if (bTryToSplit == true)
                    {
                        bEffectiveTableRowKeep = bTableRowKeep && !(bAllowSplitOfRow || bEmulateTableKeepSplitAllowed);
                    }
                    else
                    {
                        // The second attempt; ignore all the flags allowing to split the row
                        bEffectiveTableRowKeep = bTableRowKeep;
                    }
                    const bool bSplitError = !Split(nDeadLine, bTryToSplit,
                        bEffectiveTableRowKeep,
                        isFootnoteGrowth);

                    // tdf#130639 don't start table on a new page after the fallback "switch off repeating header"
                    if (bSplitError && nRepeat > GetTable()->GetRowsToRepeat())
                    {
                        setFrameAreaPositionValid(false);
                        break;
                    }

                    if (!bTryToSplit && !bSplitError)
                    {
                        --nUnSplitted;
                    }

                    // #i29771# Two tries to split the table
                    // If an error occurred during splitting. We start a second
                    // try, this time without splitting of table rows.
                    if ( bSplitError && HasFollowFlowLine() )
                        RemoveFollowFlowLine();

                    // If splitting the table was successful or not,
                    // we do not want to have 'empty' follow tables.
                    if ( GetFollow() && !GetFollow()->GetFirstNonHeadlineRow() )
                    {
                        // For split flys, if we just removed the follow flow line before split,
                        // then avoid the join in the error + rowsplit case, so split can be called
                        // again, this time without a rowsplit.
                        if (!bFlySplit || !bHadFollowFlowLineBeforeSplit || !bSplitError || !bTryToSplit)
                        {
                            Join();
                        }
                    }

                    // We want to restore the situation before the failed
                    // split operation as good as possible. Therefore we
                    // do some more calculations. Note: Restricting this
                    // to nDeadLine may not be enough.
                    // tdf#161508 hack: treat oscillation likewise
                    if ((bSplitError && bTryToSplit) // no restart if we did not try to split: i72847, i79426
                        || (!bSplitError && posSizeOscillationControl.OscillationDetected(*this)))
                    {
                        lcl_RecalcRow(*static_cast<SwRowFrame*>(Lower()), LONG_MAX);
                        setFrameAreaPositionValid(false);
                        // tdf#156724 if the table added footnotes, try to split *again*
                        if (!isFootnoteGrowth)
                        {
                            bTryToSplit = false;
                        }
                        continue;
                    }

                    // If split failed, then next time try without
                    // allowing to split the table rows.
                    bTryToSplit = !bSplitError;

                    //To avoid oscillations the Follow must become valid now
                    if ( GetFollow() )
                    {
                        // #i80924#
                        // After a successful split assure that the first row
                        // is invalid. When graphics are present, this isn't hold.
                        // Note: defect i80924 could also be fixed, if it is
                        // assured, that <SwLayNotify::bLowersComplete> is only
                        // set, if all lower are valid *and* are correct laid out.
                        if ( !bSplitError && GetFollow()->GetLower() )
                        {
                            GetFollow()->GetLower()->InvalidatePos();
                        }
                        SwRectFnSet fnRectX(GetFollow());

                        static sal_uInt8 nStack = 0;
                        if ( !StackHack::IsLocked() && nStack < 4 )
                        {
                            ++nStack;
                            StackHack aHack;
                            oAccess.reset();

                            GetFollow()->MakeAll(pRenderContext);

                            GetFollow()->SetLowersFormatted(false);
                            // #i43913# - lock follow table
                            // to avoid its formatting during the format of
                            // its content.
                            const bool bOldJoinLock =  GetFollow()->IsJoinLocked();
                            GetFollow()->LockJoin();
                            ::lcl_RecalcRow(*static_cast<SwRowFrame*>(GetFollow()->Lower()),
                                             fnRectX.GetBottom(GetFollow()->GetUpper()->getFrameArea()) );
                            // #i43913#
                            // #i63632# Do not unlock the
                            // follow if it wasn't locked before.
                            if ( !bOldJoinLock )
                                GetFollow()->UnlockJoin();

                            if ( !GetFollow()->GetFollow() )
                            {
                                SwFrame* pNxt = static_cast<SwFrame*>(GetFollow())->FindNext();
                                if ( pNxt )
                                {
                                    // #i18103# - no formatting of found next
                                    // frame, if it's a follow section of the
                                    // 'ColLocked' section, the follow table is
                                    // in.
                                    bool bCalcNxt = true;
                                    if ( GetFollow()->IsInSct() && pNxt->IsSctFrame() )
                                    {
                                        SwSectionFrame* pSct = GetFollow()->FindSctFrame();
                                        if ( pSct->IsColLocked() &&
                                             pSct->GetFollow() == pNxt )
                                        {
                                            bCalcNxt = false;
                                        }
                                    }
                                    if (pNxt->IsHiddenNow())
                                    {   // e.g. "testThemeCrash"
                                        bCalcNxt = false;
                                    }
                                    if ( bCalcNxt )
                                    {
                                        // tdf#119109 follow was just formatted,
                                        // don't do it again now
                                        FlowFrameJoinLockGuard g(GetFollow());
                                        pNxt->Calc(pRenderContext);
                                    }
                                }
                            }

                            --nStack;
                        }
                        else if ( GetFollow() == GetNext() )
                            GetFollow()->MoveFwd( true, false );
                    }
                    continue;
                }
            }
        }

        // Set to false again as early as possible.
        bLastRowHasToMoveToFollow = false;

        if( IsInSct() && bMovedFwd && bMakePage && GetUpper()->IsColBodyFrame() &&
            GetUpper()->GetUpper()->GetUpper()->IsSctFrame() &&
            ( GetUpper()->GetUpper()->GetPrev() || GetIndPrev() ) &&
            static_cast<SwSectionFrame*>(GetUpper()->GetUpper()->GetUpper())->MoveAllowed(this) )
        {
            bMovedFwd = false;
        }

        // #i29771# Reset bTryToSplit flag on change of upper
        const SwFrame* pOldUpper = GetUpper();

        //Let's see if we find some place anywhere...
        if (!bMovedFwd)
        {
            bool bMoveAlways = false;
            SwFrame* pUpper = GetUpper();
            if (pUpper && pUpper->IsFlyFrame())
            {
                auto pFlyFrame = static_cast<SwFlyFrame*>(pUpper);
                if (pFlyFrame->IsFlySplitAllowed())
                {
                    // If the anchor of the split has a previous frame, MoveFwd() is allowed to move
                    // forward.
                    bMoveAlways = true;
                }
            }
            // don't make the effort to move fwd if its known
            // conditions that are known not to work
            if (IsInFootnote() && ForbiddenForFootnoteCntFwd())
                bMakePage = false;
            else if (!MoveFwd(bMakePage, false, bMoveAlways))
                bMakePage = false;
        }

        // #i29771# Reset bSplitError flag on change of upper
        if ( GetUpper() != pOldUpper )
        {
            bTryToSplit = true;
            nUnSplitted = 5;
        }

        aRectFnSet.Refresh(this);
        m_bCalcLowers = true;
        bMovedFwd = true;
        aNotify.SetLowersComplete( false );
        if ( IsFollow() )
        {
            // To avoid oscillations, master should not remain invalid
            SwTabFrame *pTab = FindMaster();
            if ( pTab->GetUpper() )
                pTab->GetUpper()->Calc(pRenderContext);
            pTab->Calc(pRenderContext);
            pTab->SetLowersFormatted( false );
        }

        //If my neighbour is my Follow at the same time, I'll swallow it up.
        if ( ( GetNext() && GetNext() == GetFollow() ) || !GetLower() )
        {
            if ( HasFollowFlowLine() )
                RemoveFollowFlowLine();
            if ( GetFollow() )
                Join();
        }
        else if (!GetNext() && !HasFollowFlowLine() && GetFollow()
                 && (getFrameArea().Bottom() + GetFollow()->getFrameArea().Height())
                        < GetUpper()->getFrameArea().Bottom())
        {
            // We're the last lower of the upper, no split row and we have a follow.  That follow
            // fits our upper, still.  Prefer joining that follow in the next iteration, instead of
            // trying to split the current table.
            bSplit = false;
        }

        if ( bMovedBwd && GetUpper() )
        {
            //During flowing back the upper was animated to do a full repaint,
            //we can now skip this after the whole flowing back and forth.
            GetUpper()->ResetCompletePaint();
        }

        if (m_bCalcLowers && isFrameAreaDefinitionValid())
        {
            // #i44910# - format of lower frames unnecessary
            // and can cause layout loops, if table doesn't fit and isn't
            // allowed to split.
            SwTwips nDistToUpperPrtBottom =
                aRectFnSet.BottomDist( getFrameArea(), aRectFnSet.GetPrtBottom(*GetUpper()));

            if (GetUpper()->IsFlyFrame())
            {
                SwFlyFrame* pFlyFrame = GetUpper()->FindFlyFrame();
                if (pFlyFrame->IsFlySplitAllowed())
                {
                    SwTextFrame* pAnchor = pFlyFrame->FindAnchorCharFrame();
                    if (pAnchor && pAnchor->HasFollow())
                    {
                        // The split fly's anchor has a follow frame, we can move there & try to
                        // split again.
                        bTryToSplit = true;
                    }
                }
            }

            if ( nDistToUpperPrtBottom >= 0 || bTryToSplit )
            {
                lcl_RecalcTable( *this, nullptr, aNotify );
                m_bLowersFormatted = true;
                m_bCalcLowers = false;
                if (!isFramePrintAreaValid())
                    m_pTable->SetRowsToRepeat(1);
            }
#if OSL_DEBUG_LEVEL > 0
            else
            {
                OSL_FAIL( "debug assertion: <SwTabFrame::MakeAll()> - format of table lowers suppressed by fix i44910" );
            }
#endif
        }

    } //while ( !isFrameAreaPositionValid() || !isFrameAreaSizeValid() || !isFramePrintAreaValid() )

    //If my direct predecessor is my master now, it can destroy me during the
    //next best opportunity.
    if ( IsFollow() )
    {
        SwFrame *pPre = GetPrev();
        if ( pPre && pPre->IsTabFrame() && static_cast<SwTabFrame*>(pPre)->GetFollow() == this)
            pPre->InvalidatePos();
    }

    m_bCalcLowers = m_bONECalcLowers = false;
    oAccess.reset();
    UnlockJoin();
    if ( bMovedFwd || bMovedBwd || !bOldValidPos )
        aNotify.SetInvaKeep();
}

static bool IsNextOnSamePage(SwPageFrame const& rPage,
        SwTabFrame const& rTabFrame, SwTextFrame const& rAnchorFrame)
{
    for (SwContentFrame const* pContentFrame = rTabFrame.FindNextCnt();
        pContentFrame && pContentFrame->FindPageFrame() == &rPage;
        pContentFrame = pContentFrame->FindNextCnt())
    {
        if (pContentFrame == &rAnchorFrame)
        {
            return true;
        }
    }
    return false;
}

/// Calculate the offsets arising because of FlyFrames
bool SwTabFrame::CalcFlyOffsets( SwTwips& rUpper,
                               tools::Long& rLeftOffset,
                               tools::Long& rRightOffset,
                               SwTwips *const pSpaceBelowBottom) const
{
    if (IsHiddenNow())
    {
        rUpper = 0;
        rLeftOffset = 0;
        rRightOffset = 0;
        if (pSpaceBelowBottom)
            *pSpaceBelowBottom = 0;
        return false;
    }

    bool bInvalidatePrtArea = false;
    const SwPageFrame *pPage = FindPageFrame();
    const SwFlyFrame* pMyFly = FindFlyFrame();

    // --> #108724# Page header/footer content doesn't have to wrap around
    //              floating screen objects

    const IDocumentSettingAccess& rIDSA = GetFormat()->getIDocumentSettingAccess();
    const bool bWrapAllowed = rIDSA.get(DocumentSettingId::USE_FORMER_TEXT_WRAPPING) ||
                                ( !IsInFootnote() && nullptr == FindFooterOrHeader() );

    if (!bWrapAllowed || !pPage->GetSortedObjs())
        return bInvalidatePrtArea;

    SwRectFnSet aRectFnSet(this);
    const bool bConsiderWrapOnObjPos
        = rIDSA.get(DocumentSettingId::CONSIDER_WRAP_ON_OBJECT_POSITION);
    tools::Long nPrtPos = aRectFnSet.GetTop(getFrameArea());
    nPrtPos = aRectFnSet.YInc(nPrtPos, rUpper);
    SwRect aRect(getFrameArea());
    if (pSpaceBelowBottom)
    {
        // set to space below table frame
        aRectFnSet.SetTopAndHeight(aRect, aRectFnSet.GetBottom(aRect), *pSpaceBelowBottom);
    }
    else
    {
        tools::Long nYDiff = aRectFnSet.YDiff(aRectFnSet.GetTop(getFramePrintArea()), rUpper);
        if (nYDiff > 0)
            aRectFnSet.AddBottom(aRect, -nYDiff);
    }

    bool bAddVerticalFlyOffsets = rIDSA.get(DocumentSettingId::ADD_VERTICAL_FLY_OFFSETS);

    for (size_t i = 0; i < pPage->GetSortedObjs()->size(); ++i)
    {
        SwAnchoredObject* pAnchoredObj = (*pPage->GetSortedObjs())[i];
        auto pFly = pAnchoredObj->DynCastFlyFrame();
        if (!pFly)
            continue;

        const SwRect aFlyRect = pFly->GetObjRectWithSpaces();
        // #i26945# - correction of conditions,
        // if Writer fly frame has to be considered:
        // - no need to check, if top of Writer fly frame differs
        //   from FAR_AWAY, because it's also checked, if the Writer
        //   fly frame rectangle overlaps with <aRect>
        // - no check, if bottom of anchor frame is prior the top of
        //   the table, because Writer fly frames can be negative positioned.
        // - correct check, if the Writer fly frame is a lower of the
        //   table, because table lines/rows can split and an at-character
        //   anchored Writer fly frame could be positioned in the follow
        //   flow line.
        // - add condition, that an existing anchor character text frame
        //   has to be on the same page as the table.
        //   E.g., it could happen, that the fly frame is still registered
        //   at the page frame, the table is on, but it's anchor character
        //   text frame has already changed its page.
        const SwTextFrame* pAnchorCharFrame = pFly->FindAnchorCharFrame();
        const SwFormatHoriOrient& rHori= pFly->GetFormat()->GetHoriOrient();
        // TODO: why not just ignore HoriOrient?
        bool isHoriOrientShiftDown =
               rHori.GetHoriOrient() == text::HoriOrientation::NONE
            || rHori.GetHoriOrient() == text::HoriOrientation::LEFT
            || rHori.GetHoriOrient() == text::HoriOrientation::RIGHT;
        // Only consider invalid Writer fly frames if they'll be shifted down.
        bool bIgnoreFlyValidity = bAddVerticalFlyOffsets && isHoriOrientShiftDown;
        bool bConsiderFly =
            // #i46807# - do not consider invalid
            // Writer fly frames.
            (pFly->isFrameAreaDefinitionValid() || bIgnoreFlyValidity)
            // fly anchored at character or at paragraph
            && pFly->IsFlyAtContentFrame()
            // fly overlaps with corresponding table rectangle
            && aFlyRect.Overlaps(aRect)
            // fly isn't lower of table and
            // anchor character frame of fly isn't lower of table
            && (pSpaceBelowBottom // not if in ShouldBwdMoved
                || (!IsAnLower(pFly) && (!pAnchorCharFrame || !IsAnLower(pAnchorCharFrame))))
            // table isn't lower of fly
            && !pFly->IsAnLower(this)
            // fly is lower of fly, the table is in
            // #123274# - correction
            // assure that fly isn't a lower of a fly, the table isn't in.
            // E.g., a table in the body doesn't wrap around a graphic,
            // which is inside a frame.
            && (!pMyFly || pMyFly->IsAnLower(pFly))
            && pMyFly == pFly->GetAnchorFrameContainingAnchPos()->FindFlyFrame()
            // anchor frame not on following page
            && pPage->GetPhyPageNum() >= pFly->GetAnchorFrame()->FindPageFrame()->GetPhyPageNum()
            // anchor character text frame on same page
            && (!pAnchorCharFrame ||
                pAnchorCharFrame->FindPageFrame()->GetPhyPageNum() == pPage->GetPhyPageNum());

        if (!bConsiderFly)
            continue;

        const SwFrame* pFlyHeaderFooterFrame = pFly->GetAnchorFrame()->FindFooterOrHeader();
        const SwFrame* pThisHeaderFooterFrame = FindFooterOrHeader();
        if (pFlyHeaderFooterFrame != pThisHeaderFooterFrame
            // #148493# If bConsiderWrapOnObjPos is set,
            // we want to consider the fly if it is located in the header and
            // the table is located in the body:
            && (!bConsiderWrapOnObjPos || nullptr != pThisHeaderFooterFrame
                || !pFlyHeaderFooterFrame->IsHeaderFrame()))
        {
            continue;
        }

        text::WrapTextMode nSurround = pFly->GetFormat()->GetSurround().GetSurround();
        // If the frame format is a TextBox of a draw shape,
        // then use the surround of the original shape.
        {
            bool bWrapThrough = nSurround == text::WrapTextMode_THROUGH;
            SwTextBoxHelper::getShapeWrapThrough(pFly->GetFormat(), bWrapThrough);
            if (bWrapThrough)
                continue;
        }
        if (nSurround == text::WrapTextMode_THROUGH)
            nSurround = text::WrapTextMode_PARALLEL;

        bool bShiftDown = css::text::WrapTextMode_NONE == nSurround;
        bool bSplitFly = pFly->IsFlySplitAllowed();
        const SwRect aFlyRectWithoutSpaces = pFly->GetObjRect();
        if (!bShiftDown && bAddVerticalFlyOffsets)
        {
            if (nSurround == text::WrapTextMode_PARALLEL && isHoriOrientShiftDown)
            {
                // We know that wrapping was requested and the table frame overlaps with
                // the fly frame. Check if the print area overlaps with the fly frame as
                // well (in case the table does not use all the available width).
                basegfx::B1DRange aTabRange(
                    aRectFnSet.GetLeft(aRect) + aRectFnSet.GetLeft(getFramePrintArea()),
                    aRectFnSet.GetLeft(aRect) + aRectFnSet.GetLeft(getFramePrintArea())
                        + aRectFnSet.GetWidth(getFramePrintArea()));

                // Ignore spacing when determining the left/right edge of the fly, like
                // Word does.
                basegfx::B1DRange aFlyRange(aRectFnSet.GetLeft(aFlyRectWithoutSpaces),
                                            aRectFnSet.GetRight(aFlyRectWithoutSpaces));

                // If it does, shift the table down. Do this only in the compat case,
                // normally an SwFlyPortion is created instead that increases the height
                // of the first table row.
                bShiftDown = aTabRange.overlaps(aFlyRange);

                if (bSplitFly && pFly->GetAnchorFrame()->GetUpper() == GetUpper())
                {
                    // Split fly followed by an inline table. Check if we have enough space to shift
                    // to the right instead.
                    SwTwips nShiftedTabRight = aFlyRectWithoutSpaces.Right() + getFramePrintArea().Width();
                    SwTwips nRightShiftDeadline = pFly->GetAnchorFrame()->GetUpper()->getFrameArea().Right();
                    if (aRectFnSet.XDiff(nRightShiftDeadline, nShiftedTabRight) >= 0)
                    {
                        bShiftDown = false;
                    }
                }
            }
        }

        if (bShiftDown)
        {
            // possible cases:
            //        both in body
            //        both in same fly
            //        any comb. of body, footnote, header/footer
            // to keep it safe, check only in doc body vs page margin for now
            tools::Long nBottom = aRectFnSet.GetBottom(aFlyRect);
            // tdf#138039 don't grow beyond the page body
            // if the fly is anchored below the table; the fly
            // must move with its anchor frame to the next page
            SwRectFnSet fnPage(pPage);
            if (!IsInDocBody() // TODO
                || fnPage.YDiff(fnPage.GetBottom(aFlyRect), fnPage.GetPrtBottom(*pPage)) <= 0
                || !IsNextOnSamePage(
                       *pPage, *this,
                       *static_cast<SwTextFrame*>(pFly->GetAnchorFrameContainingAnchPos())))
            {
                if (aRectFnSet.YDiff(nPrtPos, nBottom) < 0)
                    nPrtPos = nBottom;
                // tdf#116501 subtract flys blocking space from below
                // TODO this may not work ideally for multiple flys
                if (pSpaceBelowBottom && aRectFnSet.YDiff(aRectFnSet.GetBottom(aRect), nBottom) < 0)
                {
                    if (aRectFnSet.YDiff(aRectFnSet.GetTop(aRect), aRectFnSet.GetTop(aFlyRect)) < 0)
                    {
                        aRectFnSet.SetBottom(aRect, aRectFnSet.GetTop(aFlyRect));
                    }
                    else
                    {
                        aRectFnSet.SetHeight(aRect, 0);
                    }
                }
                bInvalidatePrtArea = true;
            }
        }

        bool bFlyHoriOrientLeft = text::HoriOrientation::LEFT == rHori.GetHoriOrient();

        bool bToplevelSplitFly = false;
        if (bSplitFly)
        {
            // Floating table wrapped by table: avoid this in the nested case.
            bToplevelSplitFly = !pFly->GetAnchorFrame()->IsInTab();
        }

        if (bToplevelSplitFly && !bFlyHoriOrientLeft)
        {
            // Only shift to the right if we don't have enough space on the left.
            SwTwips nTabWidth = getFramePrintArea().Width();
            SwTwips nWidthDeadline = aFlyRectWithoutSpaces.Left()
                                     - pFly->GetAnchorFrame()->GetUpper()->getFrameArea().Left();
            if (nTabWidth > nWidthDeadline)
            {
                // If a split fly is oriented "from left", we already checked if it has enough space on
                // the right, so from-left and left means the same here.
                bFlyHoriOrientLeft = rHori.GetHoriOrient() == text::HoriOrientation::NONE;
            }
        }
        if ((css::text::WrapTextMode_RIGHT == nSurround
             || css::text::WrapTextMode_PARALLEL == nSurround)
            && bFlyHoriOrientLeft
            && !bShiftDown)
        {
            const tools::Long nWidth
                = aRectFnSet.XDiff(aRectFnSet.GetRight(aFlyRect),
                                   aRectFnSet.GetLeft(pFly->GetAnchorFrame()->getFrameArea()));
            rLeftOffset = std::max(rLeftOffset, nWidth);
            bInvalidatePrtArea = true;
        }
        if ((css::text::WrapTextMode_LEFT == nSurround
             || css::text::WrapTextMode_PARALLEL == nSurround)
            && text::HoriOrientation::RIGHT == rHori.GetHoriOrient()
            && !bShiftDown)
        {
            const tools::Long nWidth
                = aRectFnSet.XDiff(aRectFnSet.GetRight(pFly->GetAnchorFrame()->getFrameArea()),
                                   aRectFnSet.GetLeft(aFlyRect));
            rRightOffset = std::max(rRightOffset, nWidth);
            bInvalidatePrtArea = true;
        }
    }
    rUpper = aRectFnSet.YDiff( nPrtPos, aRectFnSet.GetTop(getFrameArea()) );
    if (pSpaceBelowBottom)
    {
        *pSpaceBelowBottom = aRectFnSet.GetHeight(aRect);
    }

    return bInvalidatePrtArea;
}

/// "Formats" the frame; Frame and PrtArea.
/// The fixed size is not adjusted here.
void SwTabFrame::Format( vcl::RenderContext* /*pRenderContext*/, const SwBorderAttrs *pAttrs )
{
    OSL_ENSURE( pAttrs, "TabFrame::Format, pAttrs is 0." );

    SwRectFnSet aRectFnSet(this);
    if ( !isFrameAreaSizeValid() )
    {
        tools::Long nDiff = aRectFnSet.GetWidth(GetUpper()->getFramePrintArea()) -
                     aRectFnSet.GetWidth(getFrameArea());
        if( nDiff )
        {
            SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*this);
            aRectFnSet.AddRight( aFrm, nDiff );
        }
    }

    //VarSize is always the height.
    //For the upper/lower margins the same rules apply as for ContentFrames (see
    //MakePrtArea() of those).

    SwTwips nUpper = CalcUpperSpace( pAttrs );

    // We want to dodge the flys. Two possibilities:
    // 1. There are flys with SurroundNone, dodge them completely
    // 2. There are flys which only wrap on the right or the left side and
    //    those are right or left aligned, those set the minimum for the margins
    tools::Long nTmpRight = -1000000,
         nLeftOffset  = 0;
    if (CalcFlyOffsets(nUpper, nLeftOffset, nTmpRight, nullptr))
    {
        setFramePrintAreaValid(false);
    }

    tools::Long nRightOffset = std::max( tools::Long(0), nTmpRight );

    SwTwips nLower = pAttrs->CalcBottomLine();
    // #i29550#
    if ( IsCollapsingBorders() )
        nLower += GetBottomLineSize();

    if ( !isFramePrintAreaValid() )
    {
        setFramePrintAreaValid(true);

        // The width of the PrintArea is given by the FrameFormat, the margins
        // have to be set accordingly.
        // Minimum margins are determined depending on borders and shadows.
        // The margins are set so that the PrintArea is aligned into the
        // Frame according to the adjustment.
        // If the adjustment is 0, the margins are set according to the border
        // attributes.

        const SwTwips nOldHeight = aRectFnSet.GetHeight(getFramePrintArea());
        const SwTwips nMax = aRectFnSet.GetWidth(getFrameArea());

        // OD 14.03.2003 #i9040# - adjust variable names.
        const SwTwips nLeftLine  = pAttrs->CalcLeftLine();
        const SwTwips nRightLine = pAttrs->CalcRightLine();

        // The width possibly is a percentage value. If the table is inside
        // something else, the value refers to the environment. If it's in the
        // body then in the BrowseView the value refers to the screen width.
        const SwFormatFrameSize &rSz = GetFormat()->GetFrameSize();
        // OD 14.03.2003 #i9040# - adjust variable name.
        const SwTwips nWishedTableWidth = CalcRel( rSz );

        bool bCheckBrowseWidth = false;

        // OD 14.03.2003 #i9040# - insert new variables for left/right spacing.
        SwTwips nLeftSpacing  = 0;
        SwTwips nRightSpacing = 0;
        switch ( GetFormat()->GetHoriOrient().GetHoriOrient() )
        {
            case text::HoriOrientation::LEFT:
                {
                    // left indent:
                    nLeftSpacing = nLeftLine + nLeftOffset;
                    // OD 06.03.2003 #i9040# - correct calculation of right indent:
                    // - Consider right indent given by right line attributes.
                    // - Consider negative right indent.
                    // wished right indent determined by wished table width and
                    // left offset given by surround fly frames on the left:
                    const SwTwips nWishRight = nMax - nWishedTableWidth - nLeftOffset;
                    if ( nRightOffset > 0 )
                    {
                        // surrounding fly frames on the right
                        // -> right indent is maximum of given right offset
                        //    and wished right offset.
                        nRightSpacing = nRightLine + std::max( SwTwips(nRightOffset), nWishRight );
                    }
                    else
                    {
                        // no surrounding fly frames on the right
                        // If intrinsic right indent (intrinsic means not considering
                        // determined left indent) is negative,
                        //      then hold this intrinsic indent,
                        //      otherwise non negative wished right indent is hold.
                        nRightSpacing = nRightLine +
                                        ( ( (nWishRight+nLeftOffset) < 0 ) ?
                                            (nWishRight+nLeftOffset) :
                                            std::max( SwTwips(0), nWishRight ) );
                    }
                }
                break;
            case text::HoriOrientation::RIGHT:
                {
                    // right indent:
                    nRightSpacing = nRightLine + nRightOffset;
                    // OD 06.03.2003 #i9040# - correct calculation of left indent:
                    // - Consider left indent given by left line attributes.
                    // - Consider negative left indent.
                    // wished left indent determined by wished table width and
                    // right offset given by surrounding fly frames on the right:
                    const SwTwips nWishLeft = nMax - nWishedTableWidth - nRightOffset;
                    if ( nLeftOffset > 0 )
                    {
                        // surrounding fly frames on the left
                        // -> right indent is maximum of given left offset
                        //    and wished left offset.
                        nLeftSpacing = nLeftLine + std::max( SwTwips(nLeftOffset), nWishLeft );
                    }
                    else
                    {
                        // no surrounding fly frames on the left
                        // If intrinsic left indent (intrinsic = not considering
                        // determined right indent) is negative,
                        //      then hold this intrinsic indent,
                        //      otherwise non negative wished left indent is hold.
                        nLeftSpacing = nLeftLine +
                                       ( ( (nWishLeft+nRightOffset) < 0 ) ?
                                           (nWishLeft+nRightOffset) :
                                           std::max( SwTwips(0), nWishLeft ) );
                    }
                }
                break;
            case text::HoriOrientation::CENTER:
                {
                    // OD 07.03.2003 #i9040# - consider left/right line attribute.
                    const SwTwips nCenterSpacing = ( nMax - nWishedTableWidth ) / 2;
                    nLeftSpacing = nLeftLine +
                                   ( (nLeftOffset > 0) ?
                                     std::max( nCenterSpacing, SwTwips(nLeftOffset) ) :
                                     nCenterSpacing );
                    nRightSpacing = nRightLine +
                                    ( (nRightOffset > 0) ?
                                      std::max( nCenterSpacing, SwTwips(nRightOffset) ) :
                                      nCenterSpacing );
                }
                break;
            case text::HoriOrientation::FULL:
                    //This things grows over the whole width.
                    //Only the free space needed for the border is taken into
                    //account. The attribute values of LRSpace are ignored
                    //intentionally.
                    bCheckBrowseWidth = true;
                    nLeftSpacing  = nLeftLine + nLeftOffset;
                    nRightSpacing = nRightLine + nRightOffset;
                break;
            case text::HoriOrientation::NONE:
                {
                    // The margins are defined by the LRSpace attribute.
                    nLeftSpacing = pAttrs->CalcLeft( this );
                    if( nLeftOffset )
                    {
                        // OD 07.03.2003 #i9040# - surround fly frames only, if
                        // they overlap with the table.
                        // Thus, take maximum of left spacing and left offset.
                        // OD 10.03.2003 #i9040# - consider left line attribute.
                        nLeftSpacing = std::max( nLeftSpacing, SwTwips( nLeftOffset + nLeftLine ) );
                    }
                    // OD 23.01.2003 #106895# - add 1st param to <SwBorderAttrs::CalcRight(..)>
                    nRightSpacing = pAttrs->CalcRight( this );
                    if( nRightOffset )
                    {
                        // OD 07.03.2003 #i9040# - surround fly frames only, if
                        // they overlap with the table.
                        // Thus, take maximum of right spacing and right offset.
                        // OD 10.03.2003 #i9040# - consider right line attribute.
                        nRightSpacing = std::max( nRightSpacing, SwTwips( nRightOffset + nRightLine ) );
                    }
                }
                break;
            case text::HoriOrientation::LEFT_AND_WIDTH:
                {
                    // count left border and width (Word specialty)
                    // OD 10.03.2003 #i9040# - no width alignment in online mode.
                    //bCheckBrowseWidth = true;
                    nLeftSpacing = pAttrs->CalcLeft( this );
                    if( nLeftOffset )
                    {
                        // OD 10.03.2003 #i9040# - surround fly frames only, if
                        // they overlap with the table.
                        // Thus, take maximum of right spacing and right offset.
                        // OD 10.03.2003 #i9040# - consider left line attribute.
                        nLeftSpacing = std::max( nLeftSpacing, SwTwips( pAttrs->CalcLeftLine() + nLeftOffset ) );
                    }
                    // OD 10.03.2003 #i9040# - consider right and left line attribute.
                    const SwTwips nWishRight =
                            nMax - (nLeftSpacing-pAttrs->CalcLeftLine()) - nWishedTableWidth;
                    nRightSpacing = nRightLine +
                                    ( (nRightOffset > 0) ?
                                      std::max( nWishRight, SwTwips(nRightOffset) ) :
                                      nWishRight );
                }
                break;
            default:
                OSL_FAIL( "Invalid orientation for table." );
        }

        // #i26250# - extend bottom printing area, if table
        // is last content inside a table cell.
        if ( GetFormat()->getIDocumentSettingAccess().get(DocumentSettingId::ADD_PARA_SPACING_TO_TABLE_CELLS) &&
             GetUpper()->IsInTab() && !GetIndNext() )
        {
            nLower += pAttrs->GetULSpace().GetLower();
        }
        aRectFnSet.SetYMargins( *this, nUpper, nLower );
        if( (nMax - MINLAY) < (nLeftSpacing + nRightSpacing) )
            aRectFnSet.SetXMargins( *this, 0, 0 );
        else
            aRectFnSet.SetXMargins( *this, nLeftSpacing, nRightSpacing );

        SwViewShell *pSh = getRootFrame()->GetCurrShell();
        if ( bCheckBrowseWidth &&
             pSh && pSh->GetViewOptions()->getBrowseMode() &&
             GetUpper()->IsPageBodyFrame() &&  // only PageBodyFrames and not ColBodyFrames
             pSh->VisArea().Width() )
        {
            //Don't go beyond the edge of the visible area.
            //The page width can be bigger because objects with
            //"over-size" are possible (RootFrame::ImplCalcBrowseWidth())
            tools::Long nWidth = pSh->GetBrowseWidth();
            nWidth -= getFramePrintArea().Left();
            nWidth -= pAttrs->CalcRightLine();

            SwFrameAreaDefinition::FramePrintAreaWriteAccess aPrt(*this);
            aPrt.Width( std::min( nWidth, aPrt.Width() ) );
        }

        if ( nOldHeight != aRectFnSet.GetHeight(getFramePrintArea()) )
        {
            setFrameAreaSizeValid(false);
        }
    }

    if ( isFrameAreaSizeValid() )
        return;

    setFrameAreaSizeValid(true);

    // The size is defined by the content plus the margins.
    SwTwips nRemaining = 0, nDiff;
    SwFrame *pFrame = m_pLower;
    while ( pFrame )
    {
        nRemaining += aRectFnSet.GetHeight(pFrame->getFrameArea());
        pFrame = pFrame->GetNext();
    }
    // And now add the margins
    nRemaining += nUpper + nLower;

    nDiff = aRectFnSet.GetHeight(getFrameArea()) - nRemaining;
    if ( nDiff > 0 )
        Shrink( nDiff );
    else if ( nDiff < 0 )
        Grow( -nDiff );
}

SwTwips SwTabFrame::GrowFrame(SwTwips nDist, SwResizeLimitReason& reason, bool bTst, bool bInfo)
{
    SwRectFnSet aRectFnSet(this);
    SwTwips nHeight = aRectFnSet.GetHeight(getFrameArea());
    if( nHeight > 0 && nDist > ( LONG_MAX - nHeight ) )
        nDist = LONG_MAX - nHeight;

    reason = SwResizeLimitReason::Unspecified;

    if ( bTst && !IsRestrictTableGrowth() )
        return nDist;

    if ( GetUpper() )
    {
        //The upper only grows as far as needed. nReal provides the distance
        //which is already available.
        SwTwips nReal = aRectFnSet.GetHeight(GetUpper()->getFramePrintArea());
        for (SwFrame* pFrame = GetUpper()->Lower(); pFrame && GetFollow() != pFrame;
             pFrame = pFrame->GetNext())
            nReal -= aRectFnSet.GetHeight(pFrame->getFrameArea());

        if ( nReal < nDist )
        {
            tools::Long nTmp = GetUpper()->Grow(nDist - std::max(nReal, SwTwips(0)), reason, bTst, bInfo);

            if ( IsRestrictTableGrowth() )
            {
                nTmp = std::min( tools::Long(nDist), nReal + nTmp );
                nDist = nTmp < 0 ? 0 : nTmp;
            }
        }

        if ( !bTst )
        {
            {
                SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*this);
                aRectFnSet.AddBottom( aFrm, nDist );
            }

#if !ENABLE_WASM_STRIP_ACCESSIBILITY
            SwRootFrame *pRootFrame = getRootFrame();
            if( pRootFrame && pRootFrame->IsAnyShellAccessible() &&
                pRootFrame->GetCurrShell() )
            {
                SwRect aOldFrame( getFrameArea() );
                pRootFrame->GetCurrShell()->Imp()->MoveAccessibleFrame( this, aOldFrame );
            }
#endif
        }
    }

    if ( !bTst && ( nDist || IsRestrictTableGrowth() ) )
    {
        SwPageFrame *pPage = FindPageFrame();
        if ( GetNext() )
        {
            GetNext()->InvalidatePos_();
            if ( GetNext()->IsContentFrame() )
                GetNext()->InvalidatePage( pPage );
        }
        // #i28701# - Due to the new object positioning the
        // frame on the next page/column can flow backward (e.g. it was moved
        // forward due to the positioning of its objects ). Thus, invalivate this
        // next frame, if document compatibility option 'Consider wrapping style
        // influence on object positioning' is ON.
        else if ( GetFormat()->getIDocumentSettingAccess().get(DocumentSettingId::CONSIDER_WRAP_ON_OBJECT_POSITION) )
        {
            InvalidateNextPos();
        }
        InvalidateAll_();
        InvalidatePage( pPage );
        SetComplete();

        std::unique_ptr<SvxBrushItem> aBack = GetFormat()->makeBackgroundBrushItem();
        const SvxGraphicPosition ePos = aBack->GetGraphicPos();
        if ( GPOS_NONE != ePos && GPOS_TILED != ePos )
            SetCompletePaint();
    }

    return nDist;
}

void SwTabFrame::Invalidate(SwTabFrameInvFlags eInvFlags)
{
    if(eInvFlags == SwTabFrameInvFlags::NONE)
        return;
    SwPageFrame* pPage = FindPageFrame();
    InvalidatePage(pPage);
    if(eInvFlags & SwTabFrameInvFlags::InvalidatePrt)
        InvalidatePrt_();
    if(eInvFlags & SwTabFrameInvFlags::InvalidatePos)
        InvalidatePos_();
    SwFrame* pTmp = GetIndNext();
    if(nullptr != pTmp)
    {
        if(eInvFlags & SwTabFrameInvFlags::InvalidateIndNextPrt)
        {
            pTmp->InvalidatePrt_();
            if(pTmp->IsContentFrame())
                pTmp->InvalidatePage(pPage);
        }
        if(eInvFlags & SwTabFrameInvFlags::SetIndNextCompletePaint)
            pTmp->SetCompletePaint();
    }
    if(eInvFlags & SwTabFrameInvFlags::InvalidatePrevPrt && nullptr != (pTmp = GetPrev()))
    {
        pTmp->InvalidatePrt_();
        if(pTmp->IsContentFrame())
            pTmp->InvalidatePage(pPage);
    }
    if(eInvFlags & SwTabFrameInvFlags::InvalidateBrowseWidth)
    {
        if(pPage && pPage->GetUpper() && !IsFollow())
            static_cast<SwRootFrame*>(pPage->GetUpper())->InvalidateBrowseWidth();
    }
    if(eInvFlags & SwTabFrameInvFlags::InvalidateNextPos)
        InvalidateNextPos();
}

void SwTabFrame::SwClientNotify(const SwModify& rMod, const SfxHint& rHint)
{
    if(rHint.GetId() == SfxHintId::SwTableHeadingChange)
    {
        HandleTableHeadlineChange();
    }
    else if(rHint.GetId() == SfxHintId::SwVirtPageNumHint)
    {
        auto& rVirtPageNumHint = const_cast<sw::VirtPageNumHint&>(static_cast<const sw::VirtPageNumHint&>(rHint));
        if(!IsInDocBody() || IsFollow() || rVirtPageNumHint.IsFound())
            return;
        if(const SwPageFrame* pPage = FindPageFrame())
            pPage->UpdateVirtPageNumInfo(rVirtPageNumHint, this);
    }
    else if (rHint.GetId() == SfxHintId::SwAttrSetChange)
    {
        auto pChangeHint = static_cast<const sw::AttrSetChangeHint*>(&rHint);
        SwTabFrameInvFlags eInvFlags = SwTabFrameInvFlags::NONE;
        if(pChangeHint->m_pNew)
        {
            const SwAttrSetChg& rOldSetChg = *pChangeHint->m_pOld;
            const SwAttrSetChg& rNewSetChg = *pChangeHint->m_pNew;
            SfxItemIter aOIter(*rOldSetChg.GetChgSet());
            SfxItemIter aNIter(*rNewSetChg.GetChgSet());
            const SfxPoolItem* pOItem = aOIter.GetCurItem();
            const SfxPoolItem* pNItem = aNIter.GetCurItem();
            SwAttrSetChg aOldSet(rOldSetChg);
            SwAttrSetChg aNewSet(rNewSetChg);
            do
            {
                UpdateAttr_(pOItem, pNItem, eInvFlags, &aOldSet, &aNewSet);
                pNItem = aNIter.NextItem();
                pOItem = aOIter.NextItem();
            } while(pNItem);
            if(aOldSet.Count() || aNewSet.Count())
                SwLayoutFrame::SwClientNotify(rMod, sw::AttrSetChangeHint(&aOldSet, &aNewSet));
        }
        Invalidate(eInvFlags);
    }
    else if (rHint.GetId() == SfxHintId::SwLegacyModify)
    {
        auto pLegacy = static_cast<const sw::LegacyModifyHint*>(&rHint);
        SwTabFrameInvFlags eInvFlags = SwTabFrameInvFlags::NONE;
        UpdateAttr_(pLegacy->m_pOld, pLegacy->m_pNew, eInvFlags);
        Invalidate(eInvFlags);
    }
}

void SwTabFrame::HandleTableHeadlineChange()
{
    if(!IsFollow())
        return;
    // Delete remaining headlines:
    SwRowFrame* pLowerRow = nullptr;
    while(nullptr != (pLowerRow = static_cast<SwRowFrame*>(Lower())) && pLowerRow->IsRepeatedHeadline())
    {
        pLowerRow->Cut();
        SwFrame::DestroyFrame(pLowerRow);
    }

    // insert new headlines
    const sal_uInt16 nNewRepeat = GetTable()->GetRowsToRepeat();
    auto& rLines = GetTable()->GetTabLines();
    for(sal_uInt16 nIdx = 0; nIdx < nNewRepeat; ++nIdx)
    {
        SwRowFrame* pHeadline = new SwRowFrame(*rLines[nIdx], this);
        {
            sw::FlyCreationSuppressor aSuppressor;
            pHeadline->SetRepeatedHeadline(true);
        }
        pHeadline->Paste(this, pLowerRow);
    }
    Invalidate(SwTabFrameInvFlags::InvalidatePrt);
}

void SwTabFrame::UpdateAttr_( const SfxPoolItem *pOld, const SfxPoolItem *pNew,
                            SwTabFrameInvFlags &rInvFlags,
                            SwAttrSetChg *pOldSet, SwAttrSetChg *pNewSet )
{
    bool bClear = true;
    const sal_uInt16 nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;
    switch( nWhich )
    {
        case RES_FRM_SIZE:
        case RES_HORI_ORIENT:
            rInvFlags |= SwTabFrameInvFlags::InvalidatePrt | SwTabFrameInvFlags::InvalidateBrowseWidth;
            break;

        case RES_PAGEDESC:                      //Attribute changes (on/off)
            if ( IsInDocBody() )
            {
                rInvFlags |= SwTabFrameInvFlags::InvalidatePos;
                SwPageFrame *pPage = FindPageFrame();
                if (pPage)
                {
                    if ( !GetPrev() )
                        CheckPageDescs( pPage );
                    if (GetFormat()->GetPageDesc().GetNumOffset())
                        static_cast<SwRootFrame*>(pPage->GetUpper())->SetVirtPageNum( true );
                    GetFormat()->GetDoc().getIDocumentFieldsAccess().UpdatePageFields(pPage->getFrameArea().Top());
                }
            }
            break;

        case RES_BREAK:
            rInvFlags |= SwTabFrameInvFlags::InvalidatePos | SwTabFrameInvFlags::InvalidateNextPos;
            break;

        case RES_LAYOUT_SPLIT:
            if ( !IsFollow() )
                rInvFlags |= SwTabFrameInvFlags::InvalidatePos;
            break;
        case RES_FRAMEDIR :
            SetDerivedR2L( false );
            CheckDirChange();
            break;
        case RES_COLLAPSING_BORDERS :
            rInvFlags |= SwTabFrameInvFlags::InvalidatePrt;
            lcl_InvalidateAllLowersPrt( this );
            break;
        case RES_UL_SPACE:
            rInvFlags |= SwTabFrameInvFlags::InvalidateIndNextPrt | SwTabFrameInvFlags::InvalidatePrevPrt | SwTabFrameInvFlags::SetIndNextCompletePaint;
            [[fallthrough]];

        default:
            bClear = false;
    }
    if ( !bClear )
        return;

    if ( pOldSet || pNewSet )
    {
        if ( pOldSet )
            pOldSet->ClearItem( nWhich );
        if ( pNewSet )
            pNewSet->ClearItem( nWhich );
    }
    else
    {
        SwModify aMod;
        SwLayoutFrame::SwClientNotify(aMod, sw::LegacyModifyHint(pOld, pNew));
    }
}

SwFrame *SwTabFrame::FindLastContentOrTable()
{
    SwFrame *pRet = m_pLower;

    while ( pRet && !pRet->IsContentFrame() )
    {
        SwFrame *pOld = pRet;

        SwFrame *pTmp = pRet;             // To skip empty section frames
        while ( pRet->GetNext() )
        {
            pRet = pRet->GetNext();
            if( !pRet->IsSctFrame() || static_cast<SwSectionFrame*>(pRet)->GetSection() )
                pTmp = pRet;
        }
        pRet = pTmp;

        if ( pRet->GetLower() )
            pRet = pRet->GetLower();
        if ( pRet == pOld )
        {
            // Check all other columns if there is a column based section with
            // an empty last column at the end of the last cell - this is done
            // by SwSectionFrame::FindLastContent
            if( pRet->IsColBodyFrame() )
            {
#if OSL_DEBUG_LEVEL > 0
                SwSectionFrame* pSect = pRet->FindSctFrame();
                OSL_ENSURE( pSect, "Where does this column come from?");
                OSL_ENSURE( IsAnLower( pSect ), "Split cell?" );
#endif
                return pRet->FindSctFrame()->FindLastContent();
            }

            // pRet may be a cell frame without a lower (cell has been split).
            // We have to find the last content the hard way:

            OSL_ENSURE( pRet->IsCellFrame(), "SwTabFrame::FindLastContent failed" );
            SwFrame* pRow = pRet->GetUpper();
            while ( pRow && !pRow->GetUpper()->IsTabFrame() )
                pRow = pRow->GetUpper();
            SwContentFrame* pContentFrame = pRow ? static_cast<SwLayoutFrame*>(pRow)->ContainsContent() : nullptr;
            pRet = nullptr;

            while ( pContentFrame && static_cast<const SwLayoutFrame*>(pRow)->IsAnLower( pContentFrame ) )
            {
                pRet = pContentFrame;
                pContentFrame = pContentFrame->GetNextContentFrame();
            }
        }
    }

    // #112929# There actually is a situation, which results in pRet = 0:
    // Insert frame, insert table via text <-> table. This gives you a frame
    // containing a table without any other content frames. Split the table
    // and undo the splitting. This operation gives us a table frame without
    // a lower.
    if ( pRet )
    {
        while ( pRet->GetNext() )
            pRet = pRet->GetNext();

        if (pRet->IsSctFrame())
            pRet = static_cast<SwSectionFrame*>(pRet)->FindLastContent();
    }

    assert(pRet == nullptr || dynamic_cast<SwContentFrame*>(pRet) || dynamic_cast<SwTabFrame*>(pRet));
    return pRet;
}

SwContentFrame *SwTabFrame::FindLastContent()
{
    SwFrame * pRet(FindLastContentOrTable());

    while (pRet && pRet->IsTabFrame()) // possibly there's only tables here!
    {   // tdf#126138 skip table, don't look inside
        pRet = pRet->GetPrev();
    }

    assert(pRet == nullptr || dynamic_cast<SwContentFrame*>(pRet));
    return static_cast<SwContentFrame*>(pRet);
}

/// Return value defines if the frm needs to be relocated
bool SwTabFrame::ShouldBwdMoved( SwLayoutFrame *pNewUpper, bool &rReformat )
{
    rReformat = false;
    if ( SwFlowFrame::IsMoveBwdJump() || !IsPrevObjMove() )
    {
        //Flowing back Frames is quite time consuming unfortunately.
        //Most often the location where the Frame wants to flow to has the same
        //FixSize as the Frame itself. In such a situation it's easy to check if
        //the Frame will find enough space for its VarSize, if this is not the
        //case, the relocation can be skipped.
        //Checking if the Frame will find enough space is done by the Frame itself,
        //this also takes the possibility of splitting the Frame into account.
        //If the FixSize is different or Flys are involved  (at the old or the
        //new position) the checks are pointless, the Frame then
        //needs to be relocated tentatively (if a bit of space is available).

        //The FixSize of the environments which contain tables is always the
        //width.

        SwPageFrame *pOldPage = FindPageFrame(),
                  *pNewPage = pNewUpper->FindPageFrame();
        bool bMoveAnyway = false;
        SwTwips nSpace = 0;

        SwRectFnSet aRectFnSet(this);
        if ( !SwFlowFrame::IsMoveBwdJump() )
        {

            tools::Long nOldWidth = aRectFnSet.GetWidth(GetUpper()->getFramePrintArea());
            SwRectFnSet fnRectX(pNewUpper);
            tools::Long nNewWidth = fnRectX.GetWidth(pNewUpper->getFramePrintArea());
            if( std::abs( nNewWidth - nOldWidth ) < 2 )
            {
                bMoveAnyway = BwdMoveNecessary( pOldPage, getFrameArea() ) > 1;
                if( !bMoveAnyway )
                {
                    SwRect aRect( pNewUpper->getFramePrintArea() );
                    aRect.Pos() += pNewUpper->getFrameArea().Pos();
                    const SwFrame *pPrevFrame = pNewUpper->Lower();
                    while ( pPrevFrame && pPrevFrame != this )
                    {
                        fnRectX.SetTop( aRect, fnRectX.GetBottom(pPrevFrame->getFrameArea()) );
                        pPrevFrame = pPrevFrame->GetNext();
                    }
                    bMoveAnyway = BwdMoveNecessary( pNewPage, aRect) > 1;

                    // #i54861# Due to changes made in PrepareMake,
                    // the tabfrm may not have a correct position. Therefore
                    // it is possible that pNewUpper->getFramePrintArea().Height == 0. In this
                    // case the above calculation of nSpace might give wrong
                    // results and we really do not want to MoveBackward into a
                    // 0 height frame. If nTmpSpace is already <= 0, we take this
                    // value:
                    const SwTwips nTmpSpace = fnRectX.GetHeight(aRect);
                    if ( fnRectX.GetHeight(pNewUpper->getFramePrintArea()) > 0 || nTmpSpace <= 0 )
                        nSpace = nTmpSpace;

                    const SwViewShell *pSh = getRootFrame()->GetCurrShell();
                    if( pSh && pSh->GetViewOptions()->getBrowseMode() )
                        nSpace += pNewUpper->Grow( LONG_MAX, true );
                    if (0 < nSpace && GetPrecede())
                    {
                        SwTwips nUpperDummy(0);
                        tools::Long nLeftOffsetDummy(0), nRightOffsetDummy(0);
                        // tdf#116501 check for no-wrap fly overlap
                        static_cast<const SwTabFrame*>(GetPrecede())->CalcFlyOffsets(
                            nUpperDummy, nLeftOffsetDummy, nRightOffsetDummy, &nSpace);
                    }
                }
            }
            else if (!m_bLockBackMove)
                bMoveAnyway = true;
            else
            {
                m_bWantBackMove = true;
            }
        }
        else if (!m_bLockBackMove)
            bMoveAnyway = true;
        else
        {
            m_bWantBackMove = true;
        }

        if ( bMoveAnyway )
        {
            rReformat = true;
            return true;
        }

        bool bFits = nSpace > 0;
        if (!bFits && aRectFnSet.GetHeight(getFrameArea()) == 0)
            // This frame fits into pNewUpper in case it has no space, but this
            // frame is empty.
            bFits = nSpace >= 0;
        if (bFits)
        {
            // #i26945# - check, if follow flow line
            // contains frame, which are moved forward due to its object
            // positioning.
            const SwRowFrame* pFirstRow = GetFirstNonHeadlineRow();
            if ( pFirstRow && pFirstRow->IsInFollowFlowRow() &&
                 SwLayouter::DoesRowContainMovedFwdFrame(
                                            pFirstRow->GetFormat()->GetDoc(),
                                            *pFirstRow ) )
            {
                return false;
            }
            SwTwips nTmpHeight = CalcHeightOfFirstContentLine();

            // For some mysterious reason, I changed the good old
            // 'return nHeight <= nSpace' to 'return nTmpHeight < nSpace'.
            // This obviously results in problems with table frames in
            // sections. Remember: Every twip is sacred.
            if (nTmpHeight <= nSpace)
            {
                if (m_bLockBackMove)
                {
                    m_bWantBackMove = true;
                }
                else
                {
                    return true;
                }
            }
        }
    }
    return false;
}

void SwTabFrame::Cut()
{
    OSL_ENSURE( GetUpper(), "Cut without Upper()." );

    SwPageFrame *pPage = FindPageFrame();
    InvalidatePage( pPage );
    SwFrame *pFrame = GetNext();
    if( pFrame )
    {
        // Possibly the old follow calculated a spacing to the predecessor
        // which is obsolete now when it becomes the first frame
        pFrame->InvalidatePrt_();
        pFrame->InvalidatePos_();
        if ( pFrame->IsContentFrame() )
            pFrame->InvalidatePage( pPage );
        if( IsInSct() && !GetPrev() )
        {
            SwSectionFrame* pSct = FindSctFrame();
            if( !pSct->IsFollow() )
            {
                pSct->InvalidatePrt_();
                pSct->InvalidatePage( pPage );
            }
        }
    }
    else
    {
        InvalidateNextPos();
        //Someone has to do the retouch: predecessor or upper
        pFrame = GetPrev();
        if ( nullptr != pFrame )
        {
            pFrame->SetRetouche();
            pFrame->Prepare( PrepareHint::WidowsOrphans );
            pFrame->InvalidatePos_();
            if ( pFrame->IsContentFrame() )
                pFrame->InvalidatePage( pPage );
        }
        //If I am (was) the only FlowFrame in my own upper, it has to do
        //the retouch. Moreover a new empty page might be created.
        else
        {   SwRootFrame *pRoot = static_cast<SwRootFrame*>(pPage->GetUpper());
            pRoot->SetSuperfluous();
            GetUpper()->SetCompletePaint();
            if( IsInSct() )
            {
                SwSectionFrame* pSct = FindSctFrame();
                if( !pSct->IsFollow() )
                {
                    pSct->InvalidatePrt_();
                    pSct->InvalidatePage( pPage );
                }
            }
        }
    }

    //First remove, then shrink the upper.
    SwLayoutFrame *pUp = GetUpper();
    SwRectFnSet aRectFnSet(this);
    RemoveFromLayout();
    if ( pUp )
    {
        OSL_ENSURE( !pUp->IsFootnoteFrame(), "Table in Footnote." );
        SwSectionFrame *pSct = nullptr;
        SwFlyFrame *pFly = nullptr;
        // #126020# - adjust check for empty section
        // #130797# - correct fix #126020#
        if ( !pUp->Lower() && pUp->IsInSct() &&
             !(pSct = pUp->FindSctFrame())->ContainsContent() &&
             !pSct->ContainsAny( true ) )
        {
            if ( pUp->GetUpper() )
            {
                pSct->DelEmpty( false );
                pSct->InvalidateSize_();
            }
        }
        else if (!pUp->Lower() && pUp->IsInFly() &&
                !(pFly = pUp->FindFlyFrame())->ContainsContent() &&
                !pFly->ContainsAny())
        {
            bool bSplitFly = pFly->IsFlySplitAllowed();
            if (!bSplitFly && pFly->IsFlyAtContentFrame())
            {
                // If the fly is not allowed to split, it's still possible that it was allowed to
                // split. That is definitely the case when the fly is a follow.
                auto pFlyAtContent = static_cast<SwFlyAtContentFrame*>(pFly);
                bSplitFly = pFlyAtContent->IsFollow();
            }
            if (pUp == pFly && bSplitFly)
            {
                auto pFlyAtContent = static_cast<SwFlyAtContentFrame*>(pFly);
                pFlyAtContent->DelEmpty();
            }
        }
        // table-in-footnote: delete empty footnote frames (like SwContentFrame::Cut)
        else if (!pUp->Lower() && pUp->IsFootnoteFrame() && !pUp->IsColLocked())
        {
            if (pUp->GetNext() && !pUp->GetPrev())
            {
                if (SwFrame *const pTmp = static_cast<SwLayoutFrame*>(pUp->GetNext())->ContainsAny())
                {
                    pTmp->InvalidatePrt_();
                }
            }
            if (!pUp->IsDeleteForbidden())
            {
                pUp->Cut();
                SwFrame::DestroyFrame(pUp);
            }
        }
        else if( aRectFnSet.GetHeight(getFrameArea()) )
        {
            // OD 26.08.2003 #i18103# - *no* 'ColUnlock' of section -
            // undo changes of fix for #104992#
            pUp->Shrink( getFrameArea().Height() );
        }
    }


    if ( pPage && !IsFollow() && pPage->GetUpper() )
        static_cast<SwRootFrame*>(pPage->GetUpper())->InvalidateBrowseWidth();
}

void SwTabFrame::Paste( SwFrame* pParent, SwFrame* pSibling )
{
    OSL_ENSURE( pParent, "No parent for pasting." );
    OSL_ENSURE( pParent->IsLayoutFrame(), "Parent is ContentFrame." );
    OSL_ENSURE( pParent != this, "I'm the parent myself." );
    OSL_ENSURE( pSibling != this, "I'm my own neighbour." );
    OSL_ENSURE( !GetPrev() && !GetNext() && !GetUpper(),
            "I'm still registered somewhere." );

    //Insert in the tree.
    InsertBefore( static_cast<SwLayoutFrame*>(pParent), pSibling );

    InvalidateAll_();
    SwPageFrame *pPage = FindPageFrame();
    InvalidatePage( pPage );

    if ( GetNext() )
    {
        GetNext()->InvalidatePos_();
        GetNext()->InvalidatePrt_();
        if ( GetNext()->IsContentFrame() )
            GetNext()->InvalidatePage( pPage );
    }

    SwRectFnSet aRectFnSet(this);
    if( aRectFnSet.GetHeight(getFrameArea()) )
        pParent->Grow( aRectFnSet.GetHeight(getFrameArea()) );

    if( aRectFnSet.GetWidth(getFrameArea()) != aRectFnSet.GetWidth(pParent->getFramePrintArea()) )
        Prepare( PrepareHint::FixSizeChanged );
    if ( GetPrev() )
    {
        if ( !IsFollow() )
        {
            GetPrev()->InvalidateSize();
            if ( GetPrev()->IsContentFrame() )
                GetPrev()->InvalidatePage( pPage );
        }
    }
    else if ( GetNext() )
        // Take the spacing into account when dealing with ContentFrames.
        // There are two situations (both always happen at the same time):
        // a) The Content becomes the first in a chain
        // b) The new follower was previously the first in a chain
        GetNext()->InvalidatePrt_();

    if ( !pPage || IsFollow() )
        return;

    if ( pPage->GetUpper() )
        static_cast<SwRootFrame*>(pPage->GetUpper())->InvalidateBrowseWidth();

    if ( !GetPrev() )//At least needed for HTML with a table at the beginning.
    {
        const SwPageDesc *pDesc = GetFormat()->GetPageDesc().GetPageDesc();
        if ( (pDesc && pDesc != pPage->GetPageDesc()) ||
             (!pDesc && pPage->GetPageDesc() != &GetFormat()->GetDoc().GetPageDesc(0)) )
            CheckPageDescs( pPage );
    }
}

bool SwTabFrame::Prepare( const PrepareHint eHint, const void *, bool )
{
    if( PrepareHint::BossChanged == eHint )
        CheckDirChange();
    return false;
}

SwRowFrame::SwRowFrame(const SwTableLine &rLine, SwFrame* pSib, bool bInsertContent)
    : SwLayoutFrame( rLine.GetFrameFormat(), pSib )
    , m_pTabLine( &rLine )
    , m_pFollowRow( nullptr )
    // #i29550#
    , mnTopMarginForLowers( 0 )
    , mnBottomMarginForLowers( 0 )
    , mnBottomLineSize( 0 )
    // --> split table rows
    , m_bIsFollowFlowRow( false )
    // <-- split table rows
    , m_bIsRepeatedHeadline( false )
    , m_bIsRowSpanLine( false )
    , m_bForceRowSplitAllowed( false )
    , m_bIsInSplit( false )
{
    mnFrameType = SwFrameType::Row;

    //Create the boxes and insert them.
    const SwTableBoxes &rBoxes = rLine.GetTabBoxes();
    SwFrame *pTmpPrev = nullptr;

    bool bHiddenRedlines = getRootFrame()->IsHideRedlines() &&
        !GetFormat()->GetDoc().getIDocumentRedlineAccess().GetRedlineTable().empty();
    for ( size_t i = 0; i < rBoxes.size(); ++i )
    {
        // skip cells deleted with track changes
        if ( bHiddenRedlines && RedlineType::Delete == rBoxes[i]->GetRedlineType() )
            continue;

        SwCellFrame *pNew = new SwCellFrame( *rBoxes[i], this, bInsertContent );
        pNew->InsertBehind( this, pTmpPrev );
        pTmpPrev = pNew;
    }
}

void SwRowFrame::DestroyImpl()
{
    sw::BroadcastingModify* pMod = GetFormat();
    if( pMod )
    {
        pMod->Remove(*this);
        if( !pMod->HasWriterListeners() )
            delete pMod;
    }

    SwLayoutFrame::DestroyImpl();
}

SwRowFrame::~SwRowFrame()
{
}

void SwRowFrame::RegistFlys( SwPageFrame *pPage )
{
    ::RegistFlys( pPage ? pPage : FindPageFrame(), this );
}

void SwRowFrame::OnFrameSize(const SfxPoolItem& rSize)
{
    SwTabFrame* pTab = FindTabFrame();
    if(pTab)
    {
        const bool bInFirstNonHeadlineRow = pTab->IsFollow() && this == pTab->GetFirstNonHeadlineRow();
        // #i35063#
        // Invalidation required is pRow is last row
        if(bInFirstNonHeadlineRow)
            pTab = pTab->FindMaster();
        if(bInFirstNonHeadlineRow || !GetNext())
            pTab->InvalidatePos();
    }
    const sw::BroadcastingModify aMod;
    SwLayoutFrame::SwClientNotify(aMod, sw::LegacyModifyHint(nullptr, &rSize));
}

void SwRowFrame::SwClientNotify(const SwModify& rModify, const SfxHint& rHint)
{
    if(rHint.GetId() == SfxHintId::SwTableLineFormatChanged)
    {
        auto pNewFormatHint = static_cast<const sw::TableLineFormatChanged*>(&rHint);
        if(GetTabLine() != &pNewFormatHint->m_rTabLine)
            return;
        RegisterToFormat(const_cast<SwTableLineFormat&>(pNewFormatHint->m_rNewFormat));
        InvalidateSize();
        InvalidatePrt_();
        SetCompletePaint();
        ReinitializeFrameSizeAttrFlags();

        // #i35063#
        // consider 'split row allowed' attribute
        SwTabFrame* pTab = FindTabFrame();
        bool bInFollowFlowRow = false;
        const bool bInFirstNonHeadlineRow = pTab->IsFollow() && this == pTab->GetFirstNonHeadlineRow();
        if(bInFirstNonHeadlineRow ||
             !GetNext() ||
             (bInFollowFlowRow = IsInFollowFlowRow()) ||
             nullptr != IsInSplitTableRow() )
        {
            if(bInFirstNonHeadlineRow || bInFollowFlowRow)
                pTab = pTab->FindMaster();

            pTab->SetRemoveFollowFlowLinePending(true);
            pTab->InvalidatePos();
        }
    }
    else if(rHint.GetId() == SfxHintId::SwMoveTableLine)
    {
        auto pMoveTableLineHint = static_cast<const sw::MoveTableLineHint*>(&rHint);
        if(GetTabLine() != &pMoveTableLineHint->m_rTableLine)
            return;
        const_cast<SwFrameFormat*>(&pMoveTableLineHint->m_rNewFormat)->Add(*this);
        InvalidateAll();
        ReinitializeFrameSizeAttrFlags();
        return;
    }
    else if (rHint.GetId() == SfxHintId::SwAttrSetChange)
    {
        auto pChangeHint = static_cast<const sw::AttrSetChangeHint*>(&rHint);
        if(!pChangeHint->m_pNew)
        {
            // possibly not needed?
            SwLayoutFrame::SwClientNotify(rModify, rHint);
            return;
        }
        const SwAttrSet* pChgSet = pChangeHint->m_pNew->GetChgSet();
        const SfxPoolItem* pItem = nullptr;
        pChgSet->GetItemState(RES_FRM_SIZE, false, &pItem);
        if(!pItem)
            pChgSet->GetItemState(RES_ROW_SPLIT, false, &pItem);
        if(pItem)
            OnFrameSize(*pItem);
        else
            SwLayoutFrame::SwClientNotify(rModify, rHint); // possibly not needed?
        return;
    }
    if (rHint.GetId() != SfxHintId::SwLegacyModify)
        return;
    auto pLegacy = static_cast<const sw::LegacyModifyHint*>(&rHint);
    if(!pLegacy->m_pNew)
    {
        // possibly not needed?
        SwLayoutFrame::SwClientNotify(rModify, rHint);
        return;
    }
    switch(pLegacy->m_pNew->Which())
    {
       case RES_FRM_SIZE:
       case RES_ROW_SPLIT:
            OnFrameSize(*static_cast<const SwFormatFrameSize*>(pLegacy->m_pNew));
            return;
    }
}

void SwRowFrame::MakeAll(vcl::RenderContext* pRenderContext)
{
    if ( !GetNext() )
    {
        setFrameAreaSizeValid(false);
    }

    SwLayoutFrame::MakeAll(pRenderContext);
}

void SwRowFrame::dumpAsXml(xmlTextWriterPtr writer) const
{
    (void)xmlTextWriterStartElement(writer, reinterpret_cast<const xmlChar*>("row"));
    dumpAsXmlAttributes(writer);

    (void)xmlTextWriterStartElement(writer, BAD_CAST("infos"));
    dumpInfosAsXml(writer);
    (void)xmlTextWriterEndElement(writer);
    dumpChildrenAsXml(writer);

    (void)xmlTextWriterEndElement(writer);
}

tools::Long CalcHeightWithFlys( const SwFrame *pFrame )
{
    SwRectFnSet aRectFnSet(pFrame);
    tools::Long nHeight = 0;
    const SwFrame* pTmp = pFrame->IsSctFrame() ?
            static_cast<const SwSectionFrame*>(pFrame)->ContainsContent() : pFrame;
    while( pTmp )
    {
        // #i26945# - consider follow text frames
        const SwSortedObjs* pObjs( nullptr );
        bool bIsFollow( false );
        if ( pTmp->IsTextFrame() && static_cast<const SwTextFrame*>(pTmp)->IsFollow() )
        {
            const SwFrame* pMaster;
            // #i46450# Master does not necessarily have
            // to exist if this function is called from JoinFrame() ->
            // Cut() -> Shrink()
            const SwTextFrame* pTmpFrame = static_cast<const SwTextFrame*>(pTmp);
            if ( pTmpFrame->GetPrev() && pTmpFrame->GetPrev()->IsTextFrame() &&
                 static_cast<const SwTextFrame*>(pTmpFrame->GetPrev())->GetFollow() &&
                 static_cast<const SwTextFrame*>(pTmpFrame->GetPrev())->GetFollow() != pTmp )
                 pMaster = nullptr;
            else
                 pMaster = pTmpFrame->FindMaster();

            if ( pMaster )
            {
                pObjs = static_cast<const SwTextFrame*>(pTmp)->FindMaster()->GetDrawObjs();
                bIsFollow = true;
            }
        }
        else
        {
            pObjs = pTmp->GetDrawObjs();
        }
        if ( pObjs )
        {
            for (SwAnchoredObject* pAnchoredObj : *pObjs)
            {
                // #i26945# - if <pTmp> is follow, the
                // anchor character frame has to be <pTmp>.
                if ( bIsFollow &&
                     pAnchoredObj->FindAnchorCharFrame() != pTmp )
                {
                    continue;
                }
                // #i26945# - consider also drawing objects
                {
                    // OD 30.09.2003 #i18732# - only objects, which follow
                    // the text flow have to be considered.
                    const SwFrameFormat* pFrameFormat = pAnchoredObj->GetFrameFormat();
                    bool bFollowTextFlow = pFrameFormat->GetFollowTextFlow().GetValue();
                    bool bIsFarAway = pAnchoredObj->GetObjRect().Top() != FAR_AWAY;
                    const SwPageFrame* pPageFrm = pTmp->FindPageFrame();
                    bool bIsAnchoredToTmpFrm = false;
                    if ( pPageFrm && pPageFrm->IsPageFrame() && pAnchoredObj->GetPageFrame())
                        bIsAnchoredToTmpFrm = pAnchoredObj->GetPageFrame() == pPageFrm ||
                        (pPageFrm->GetFormatPage().GetPhyPageNum() == pAnchoredObj->GetPageFrame()->GetFormatPage().GetPhyPageNum() + 1);
                    const bool bConsiderObj =
                        (pFrameFormat->GetAnchor().GetAnchorId() != RndStdIds::FLY_AS_CHAR) &&
                        bIsFarAway &&
                        bFollowTextFlow && bIsAnchoredToTmpFrm;
                    bool bWrapThrough = pFrameFormat->GetSurround().GetValue() == text::WrapTextMode_THROUGH;
                    bool bInBackground = !pFrameFormat->GetOpaque().GetValue();
                    // Legacy render requires in-background setting, the new mode does not.
                    bool bConsiderFollowTextFlow = bInBackground
                                                   || !pFrameFormat->getIDocumentSettingAccess().get(
                                                       DocumentSettingId::USE_FORMER_TEXT_WRAPPING);
                    if (pFrame->IsInTab() && bFollowTextFlow && bWrapThrough && bConsiderFollowTextFlow)
                    {
                        // Ignore wrap-through objects when determining the cell height.
                        // Normally FollowTextFlow requires a resize of the cell, but not in case of
                        // wrap-through.
                        continue;
                    }

                    if ( bConsiderObj )
                    {
                        const SwFormatFrameSize &rSz = pFrameFormat->GetFrameSize();
                        if( !rSz.GetHeightPercent() )
                        {
                            const SwTwips nDistOfFlyBottomToAnchorTop =
                                aRectFnSet.GetHeight(pAnchoredObj->GetObjRect()) +
                                    ( aRectFnSet.IsVert() ?
                                      pAnchoredObj->GetCurrRelPos().X() :
                                      pAnchoredObj->GetCurrRelPos().Y() );

                            const SwTwips nFrameDiff =
                                aRectFnSet.YDiff(
                                    aRectFnSet.GetTop(pTmp->getFrameArea()),
                                    aRectFnSet.GetTop(pFrame->getFrameArea()) );

                            nHeight = std::max( nHeight, nDistOfFlyBottomToAnchorTop + nFrameDiff -
                                            aRectFnSet.GetHeight(pFrame->getFrameArea()) );

                            // #i56115# The first height calculation
                            // gives wrong results if pFrame->getFramePrintArea().Y() > 0. We do
                            // a second calculation based on the actual rectangles of
                            // pFrame and pAnchoredObj, and use the maximum of the results.
                            // I do not want to remove the first calculation because
                            // if clipping has been applied, using the GetCurrRelPos
                            // might be the better option to calculate nHeight.
                            const SwTwips nDistOfFlyBottomToAnchorTop2 = aRectFnSet.YDiff(
                                                                            aRectFnSet.GetBottom(pAnchoredObj->GetObjRect()),
                                                                            aRectFnSet.GetBottom(pFrame->getFrameArea()) );

                            nHeight = std::max( nHeight, tools::Long(nDistOfFlyBottomToAnchorTop2 ));
                        }
                    }
                }
            }
        }
        if( !pFrame->IsSctFrame() )
            break;
        pTmp = pTmp->FindNextCnt();
        if( !static_cast<const SwSectionFrame*>(pFrame)->IsAnLower( pTmp ) )
            break;
    }
    return nHeight;
}

static SwTwips lcl_CalcTopAndBottomMargin( const SwLayoutFrame& rCell, const SwBorderAttrs& rAttrs )
{
    const SwTabFrame* pTab = rCell.FindTabFrame();
    SwTwips nTopSpace = 0;
    SwTwips nBottomSpace = 0;

    // #i29550#
    const SwFrame* pLower = rCell.Lower();
    if ( pTab->IsCollapsingBorders() && pLower && !pLower->IsRowFrame() )
    {
        nTopSpace    = static_cast<const SwRowFrame*>(rCell.GetUpper())->GetTopMarginForLowers();
        nBottomSpace = static_cast<const SwRowFrame*>(rCell.GetUpper())->GetBottomMarginForLowers();
    }
    else
    {
        if ( pTab->IsVertical() != rCell.IsVertical() )
        {
            nTopSpace    = rAttrs.CalcLeft( &rCell );
            nBottomSpace = rAttrs.CalcRight( &rCell );
        }
        else
        {
            nTopSpace    = rAttrs.CalcTop();
            nBottomSpace = rAttrs.CalcBottom();
        }
    }

    return nTopSpace + nBottomSpace;
}

// #i26945# - add parameter <_bConsiderObjs> in order to
// control, if floating screen objects have to be considered for the minimal
// cell height.
static SwTwips lcl_CalcMinCellHeight( const SwLayoutFrame *_pCell,
                                      const bool _bConsiderObjs,
                                      const SwBorderAttrs *pAttrs = nullptr )
{
    SwRectFnSet aRectFnSet(_pCell);
    SwTwips nHeight = 0;
    const SwFrame* pLow = _pCell->Lower();
    tools::Long nFlyAdd = 0;
    while ( pLow )
    {
        if ( pLow->IsRowFrame() )
        {
            // #i26945#
            nHeight += ::lcl_CalcMinRowHeight( static_cast<const SwRowFrame*>(pLow),
                                               _bConsiderObjs );
        }
        else
        {
            tools::Long nLowHeight = aRectFnSet.GetHeight(pLow->getFrameArea());
            nHeight += nLowHeight;
            // #i26945#
            if ( _bConsiderObjs )
            {
                nFlyAdd = std::max( tools::Long(0), nFlyAdd - nLowHeight );
                nFlyAdd = std::max( nFlyAdd, ::CalcHeightWithFlys( pLow ) );
            }
        }

        pLow = pLow->GetNext();
    }
    if ( nFlyAdd )
        nHeight += nFlyAdd;
    // The border/margin needs to be considered too, unfortunately it can't be
    // calculated using PrintArea and FrameArea because any or all of those
    // may be invalid.
    if ( _pCell->Lower() )
    {
        if ( pAttrs )
            nHeight += lcl_CalcTopAndBottomMargin( *_pCell, *pAttrs );
        else
        {
            SwBorderAttrAccess aAccess( SwFrame::GetCache(), _pCell );
            const SwBorderAttrs &rAttrs = *aAccess.Get();
            nHeight += lcl_CalcTopAndBottomMargin( *_pCell, rAttrs );
        }
    }
    return nHeight;
}

static SwTwips lcl_GetFixedRowHeight(const SwRowFrame& rRow, SwTwips nHeight)
{
    auto& rIDSA = rRow.GetFormat()->GetDoc().GetDocumentSettingManager();
    if (rIDSA.get(DocumentSettingId::MIN_ROW_HEIGHT_INCL_BORDER)) // MS Word 'exact' oddities
    {
        nHeight += lcl_GetBottomLineDist(rRow);
    }

    return nHeight;
}
// #i26945# - add parameter <_bConsiderObjs> in order to control,
// if floating screen objects have to be considered for the minimal cell height
static SwTwips lcl_CalcMinRowHeight( const SwRowFrame* _pRow,
                                     const bool _bConsiderObjs )
{
    //calc min height including width of horizontal border
    const bool bMinRowHeightInclBorder =
        _pRow->GetFormat()->GetDoc().GetDocumentSettingManager().get(DocumentSettingId::MIN_ROW_HEIGHT_INCL_BORDER);
    SwTwips nHeight = 0;
    if ( !_pRow->IsRowSpanLine() )
    {
        const SwFormatFrameSize &rSz = _pRow->GetFormat()->GetFrameSize();
        if (_pRow->HasFixSize() && !_pRow->IsForceRowSplitAllowed())
        {
            OSL_ENSURE(SwFrameSize::Fixed == rSz.GetHeightSizeType(), "pRow claims to have fixed size");
            return rSz.GetHeight();
        }
        // If this row frame is being split, then row's minimal height shouldn't restrict
        // this frame's minimal height, because the rest will go to follow frame.
        else if ( !_pRow->IsInSplit() && rSz.GetHeightSizeType() == SwFrameSize::Minimum )
        {
            nHeight = rSz.GetHeight();
            if (bMinRowHeightInclBorder) // handle MS Word 'atLeast' oddities
            {
                // add top horizontal border line, along with its padding.
                nHeight += lcl_GetTopSpace(*_pRow);
                // MS Word also adds the bottom border padding (but not the bottom border line)
                nHeight += lcl_GetBottomLineDist(*_pRow);
            }

            bool bSplitFly = false;
            if (_pRow->IsInFly())
            {
                // See if we're in a split fly that is anchored on a page that has enough space to
                // host this row with its minimum row height.
                const SwFlyFrame* pFly = _pRow->FindFlyFrame();
                if (pFly->IsFlySplitAllowed())
                {
                    SwFrame* pAnchor = const_cast<SwFlyFrame*>(pFly)->FindAnchorCharFrame();
                    if (pAnchor)
                    {
                        if (pAnchor->FindPageFrame()->getFramePrintArea().Height() > nHeight)
                        {
                            bSplitFly = true;
                        }
                    }
                }
            }

            if (bSplitFly)
            {
                // Split fly: enforce minimum row height for the master and follows.
            }
            else
            {
                nHeight -= lcl_calcHeightOfRowBeforeThisFrame(*_pRow);
            }
        }
    }

    SwRectFnSet aRectFnSet(_pRow);
    const SwCellFrame* pLow = static_cast<const SwCellFrame*>(_pRow->Lower());
    while ( pLow )
    {
        SwTwips nTmp = 0;
        const tools::Long nRowSpan = pLow->GetLayoutRowSpan();
        // --> NEW TABLES
        // Consider height of
        // 1. current cell if RowSpan == 1
        // 2. current cell if cell is "follow" cell of a cell with RowSpan == -1
        // 3. master cell if RowSpan == -1
        if ( 1 == nRowSpan )
        {
            nTmp = ::lcl_CalcMinCellHeight( pLow, _bConsiderObjs );
        }
        else if ( -1 == nRowSpan )
        {
            // Height of the last cell of a row span is height of master cell
            // minus the height of the other rows which are covered by the master
            // cell:
            const SwCellFrame& rMaster = pLow->FindStartEndOfRowSpanCell( true );
            nTmp = ::lcl_CalcMinCellHeight( &rMaster, _bConsiderObjs );
            const SwFrame* pMasterRow = rMaster.GetUpper();
            while ( pMasterRow && pMasterRow != _pRow )
            {
                nTmp -= aRectFnSet.GetHeight(pMasterRow->getFrameArea());
                pMasterRow = pMasterRow->GetNext();
            }
        }
        // <-- NEW TABLES

        // Do not consider rotated cells:
        if ( pLow->IsVertical() == aRectFnSet.IsVert() && nTmp > nHeight )
            nHeight = nTmp;

        pLow = static_cast<const SwCellFrame*>(pLow->GetNext());
    }

    return nHeight;
}

// #i29550#

// Calculate the maximum of (TopLineSize + TopLineDist) over all lowers:
static sal_uInt16 lcl_GetTopSpace( const SwRowFrame& rRow )
{
    sal_uInt16 nTopSpace = 0;
    for ( const SwCellFrame* pCurrLower = static_cast<const SwCellFrame*>(rRow.Lower()); pCurrLower;
          pCurrLower = static_cast<const SwCellFrame*>(pCurrLower->GetNext()) )
    {
        sal_uInt16 nTmpTopSpace = 0;
        const SwFrame* pLower = pCurrLower->Lower();
        if ( pLower && pLower->IsRowFrame() )
            nTmpTopSpace = lcl_GetTopSpace( *static_cast<const SwRowFrame*>(pLower) );
        else
        {
            const SwAttrSet& rSet = pCurrLower->GetFormat()->GetAttrSet();
            const SvxBoxItem& rBoxItem = rSet.GetBox();
            nTmpTopSpace = rBoxItem.CalcLineSpace( SvxBoxItemLine::TOP, true );
        }
        nTopSpace  = std::max( nTopSpace, nTmpTopSpace );
    }
    return nTopSpace;
}

// Calculate the maximum of TopLineDist over all lowers:
static sal_uInt16 lcl_GetTopLineDist( const SwRowFrame& rRow )
{
    sal_uInt16 nTopLineDist = 0;
    for ( const SwCellFrame* pCurrLower = static_cast<const SwCellFrame*>(rRow.Lower()); pCurrLower;
          pCurrLower = static_cast<const SwCellFrame*>(pCurrLower->GetNext()) )
    {
        sal_uInt16 nTmpTopLineDist = 0;
        const SwFrame* pLower = pCurrLower->Lower();
        if ( pLower && pLower->IsRowFrame() )
            nTmpTopLineDist = lcl_GetTopLineDist( *static_cast<const SwRowFrame*>(pLower) );
        else
        {
            const SwAttrSet& rSet = pCurrLower->GetFormat()->GetAttrSet();
            const SvxBoxItem& rBoxItem = rSet.GetBox();
            nTmpTopLineDist = rBoxItem.GetDistance( SvxBoxItemLine::TOP );
        }
        nTopLineDist = std::max( nTopLineDist, nTmpTopLineDist );
    }
    return nTopLineDist;
}

// Calculate the maximum of BottomLineSize over all lowers:
static sal_uInt16 lcl_GetBottomLineSize( const SwRowFrame& rRow )
{
    sal_uInt16 nBottomLineSize = 0;
    for ( const SwCellFrame* pCurrLower = static_cast<const SwCellFrame*>(rRow.Lower()); pCurrLower;
          pCurrLower = static_cast<const SwCellFrame*>(pCurrLower->GetNext()) )
    {
        sal_uInt16 nTmpBottomLineSize = 0;
        const SwFrame* pLower = pCurrLower->Lower();
        if ( pLower && pLower->IsRowFrame() )
        {
            const SwFrame* pRow = pCurrLower->GetLastLower();
            nTmpBottomLineSize = lcl_GetBottomLineSize( *static_cast<const SwRowFrame*>(pRow) );
        }
        else
        {
            const SwAttrSet& rSet = pCurrLower->GetFormat()->GetAttrSet();
            const SvxBoxItem& rBoxItem = rSet.GetBox();
            nTmpBottomLineSize = rBoxItem.CalcLineSpace( SvxBoxItemLine::BOTTOM, true ) -
                                 rBoxItem.GetDistance( SvxBoxItemLine::BOTTOM );
        }
        nBottomLineSize = std::max( nBottomLineSize, nTmpBottomLineSize );
    }
    return nBottomLineSize;
}

// Calculate the maximum of BottomLineDist over all lowers:
static sal_uInt16 lcl_GetBottomLineDist( const SwRowFrame& rRow )
{
    sal_uInt16 nBottomLineDist = 0;
    for ( const SwCellFrame* pCurrLower = static_cast<const SwCellFrame*>(rRow.Lower()); pCurrLower;
          pCurrLower = static_cast<const SwCellFrame*>(pCurrLower->GetNext()) )
    {
        sal_uInt16 nTmpBottomLineDist = 0;
        const SwFrame* pLower = pCurrLower->Lower();
        if ( pLower && pLower->IsRowFrame() )
        {
            const SwFrame* pRow = pCurrLower->GetLastLower();
            nTmpBottomLineDist = lcl_GetBottomLineDist( *static_cast<const SwRowFrame*>(pRow) );
        }
        else
        {
            const SwAttrSet& rSet = pCurrLower->GetFormat()->GetAttrSet();
            const SvxBoxItem& rBoxItem = rSet.GetBox();
            nTmpBottomLineDist = rBoxItem.GetDistance( SvxBoxItemLine::BOTTOM );
        }
        nBottomLineDist = std::max( nBottomLineDist, nTmpBottomLineDist );
    }
    return nBottomLineDist;
}

// tdf#104425: calculate the height of all row frames,
// for which this frame is a follow.
// When a row has fixed/minimum height, it may span over
// several pages. The minimal height on this page should
// take into account the sum of all the heights of previous
// frames that constitute the table row on previous pages.
// Otherwise, trying to split a too high row frame will
// result in loop trying to create that too high row
// on each following page
static SwTwips lcl_calcHeightOfRowBeforeThisFrame(const SwRowFrame& rRow)
{
    // We don't need to account for previous instances of repeated headlines
    if (rRow.IsRepeatedHeadline())
        return 0;
    SwRectFnSet aRectFnSet(&rRow);
    const SwTableLine* pLine = rRow.GetTabLine();
    const SwTabFrame* pTab = rRow.FindTabFrame();
    if (!pLine || !pTab || !pTab->IsFollow())
        return 0;
    SwTwips nResult = 0;
    SwIterator<SwRowFrame, SwFormat> aIter(*pLine->GetFrameFormat());
    for (const SwRowFrame* pCurRow = aIter.First(); pCurRow; pCurRow = aIter.Next())
    {
        if (pCurRow != &rRow && pCurRow->GetTabLine() == pLine)
        {
            // We've found another row frame that is part of the same table row
            const SwTabFrame* pCurTab = pCurRow->FindTabFrame();
            // A row frame may not belong to a table frame, when it is being cut, e.g., in
            // lcl_PostprocessRowsInCells().
            // Its SwRowFrame::Cut() has been called; it in turn called SwLayoutFrame::Cut(),
            // which nullified row's upper in RemoveFromLayout(), and then called Shrink()
            // for its former upper.
            // Regardless of whether it will be pasted back, or destroyed, currently it's not
            // part of layout, and its height does not count
            if (pCurTab && pCurTab->IsAnFollow(pTab))
            {
                // The found row frame belongs to a table frame that precedes
                // (above) this one in chain. So, include it in the sum
                nResult += aRectFnSet.GetHeight(pCurRow->getFrameArea());
            }
        }
    }
    return nResult;
}

void SwRowFrame::Format( vcl::RenderContext* /*pRenderContext*/, const SwBorderAttrs *pAttrs )
{
    SwRectFnSet aRectFnSet(this);
    OSL_ENSURE( pAttrs, "SwRowFrame::Format without Attrs." );

    const bool bFix = mbFixSize;
    SwTabFrame* pTabFrame = FindTabFrame();

    if ( !isFramePrintAreaValid() )
    {
        // RowFrames don't have borders/margins therefore the PrintArea always
        // matches the FrameArea.
        setFramePrintAreaValid(true);

        {
            SwFrameAreaDefinition::FramePrintAreaWriteAccess aPrt(*this);
            aPrt.Left( 0 );
            aPrt.Top( 0 );
            aPrt.Width ( getFrameArea().Width() );
            aPrt.Height( getFrameArea().Height() );
        }

        // #i29550#
        // Here we calculate the top-printing area for the lower cell frames
        if ( pTabFrame->IsCollapsingBorders() )
        {
            const sal_uInt16 nTopSpace        = lcl_GetTopSpace(       *this );
            const sal_uInt16 nTopLineDist     = lcl_GetTopLineDist(    *this );
            const sal_uInt16 nBottomLineSize  = lcl_GetBottomLineSize( *this );
            const sal_uInt16 nBottomLineDist  = lcl_GetBottomLineDist( *this );

            const SwRowFrame* pPreviousRow = nullptr;

            // #i32456#
            // In order to calculate the top printing area for the lower cell
            // frames, we have to find the 'previous' row frame and compare
            // the bottom values of the 'previous' row with the 'top' values
            // of this row. The best way to find the 'previous' row is to
            // use the table structure:
            const SwTable* pTable = pTabFrame->GetTable();
            const SwTableLine* pPrevTabLine = nullptr;
            const SwRowFrame* pTmpRow = this;

            while ( pTmpRow && !pPrevTabLine )
            {
                size_t nIdx = 0;
                const SwTableLines& rLines = pTmpRow->GetTabLine()->GetUpper() ?
                                             pTmpRow->GetTabLine()->GetUpper()->GetTabLines() :
                                             pTable->GetTabLines();

                while ( rLines[ nIdx ] != pTmpRow->GetTabLine() )
                    ++nIdx;

                if ( nIdx > 0 )
                {
                    // pTmpRow has a 'previous' row in the table structure:
                    pPrevTabLine = rLines[ nIdx - 1 ];
                }
                else
                {
                    // pTmpRow is a first row in the table structure.
                    // We go up in the table structure:
                    pTmpRow = pTmpRow->GetUpper()->GetUpper() &&
                              pTmpRow->GetUpper()->GetUpper()->IsRowFrame() ?
                              static_cast<const SwRowFrame*>( pTmpRow->GetUpper()->GetUpper() ) :
                              nullptr;
                }
            }

            // If we found a 'previous' row, we look for the appropriate row frame:
            if ( pPrevTabLine )
            {
                SwIterator<SwRowFrame,SwFormat> aIter( *pPrevTabLine->GetFrameFormat() );
                for ( SwRowFrame* pRow = aIter.First(); pRow; pRow = aIter.Next() )
                {
                    // #115759# - do *not* take repeated
                    // headlines, because during split of table it can be
                    // invalid and thus can't provide correct border values.
                    if ( pRow->GetTabLine() == pPrevTabLine &&
                         !pRow->IsRepeatedHeadline() )
                    {
                        pPreviousRow = pRow;
                        break;
                    }
                }
            }

            sal_uInt16 nTopPrtMargin = nTopSpace;
            if ( pPreviousRow )
            {
                const sal_uInt16 nTmpPrtMargin = pPreviousRow->GetBottomLineSize() + nTopLineDist;
                if ( nTmpPrtMargin > nTopPrtMargin )
                    nTopPrtMargin = nTmpPrtMargin;
            }

            // table has to be notified if it has to change its lower
            // margin due to changes of nBottomLineSize:
            if ( !GetNext() && nBottomLineSize != GetBottomLineSize() )
                 pTabFrame->InvalidatePrt_();

            // If there are rows nested inside this row, the nested rows
            // may not have been calculated yet. Therefore the
            // ::lcl_CalcMinRowHeight( this ) operation later in this
            // function cannot consider the correct border values. We
            // have to trigger the invalidation of the outer row frame
            // manually:
            // Note: If any further invalidations should be necessary, we
            // should consider moving the invalidation stuff to the
            // appropriate SwNotify object.
            if ( GetUpper()->GetUpper()->IsRowFrame() &&
                 ( nBottomLineDist != GetBottomMarginForLowers() ||
                   nTopPrtMargin   != GetTopMarginForLowers() ) )
                GetUpper()->GetUpper()->InvalidateSize_();

            SetBottomMarginForLowers( nBottomLineDist );    //  3.
            SetBottomLineSize( nBottomLineSize );           //  4.
            SetTopMarginForLowers( nTopPrtMargin );         //  5.

        }
    }

    while ( !isFrameAreaSizeValid() )
    {
        setFrameAreaSizeValid(true);

#if OSL_DEBUG_LEVEL > 0
        if ( HasFixSize() )
        {
            const SwFormatFrameSize &rFrameSize = GetFormat()->GetFrameSize();
            OSL_ENSURE( rFrameSize.GetSize().Height() > 0, "Has it" );
        }
#endif
        const SwTwips nDiff = aRectFnSet.GetHeight(getFrameArea()) -
                              ( HasFixSize() && !IsRowSpanLine()
                                ? lcl_GetFixedRowHeight(*this, pAttrs->GetSize().Height())
                                // #i26945#
                                : ::lcl_CalcMinRowHeight( this,
                                    FindTabFrame()->IsConsiderObjsForMinCellHeight() ) );
        if ( nDiff )
        {
            mbFixSize = false;
            if ( nDiff > 0 )
                Shrink( nDiff, false, true );
            else if ( nDiff < 0 )
                Grow( -nDiff );
            mbFixSize = bFix;
        }
    }

    // last row will fill the space in its upper.
    if ( GetNext() )
        return;

    //The last fills the remaining space in the upper.
    SwTwips nDiff = aRectFnSet.GetHeight(GetUpper()->getFramePrintArea());
    SwFrame *pSibling = GetUpper()->Lower();
    while ( pSibling )
    {   nDiff -= aRectFnSet.GetHeight(pSibling->getFrameArea());
        pSibling = pSibling->GetNext();
    }
    if (nDiff > 0 && pTabFrame->IsCollapsingBorders())
    {
        // In SwTabFrame::Format, this value will be added to the table's bottom margin
        nDiff -= GetBottomLineSize();
    }
    if ( nDiff > 0 )
    {
        mbFixSize = false;
        Grow( nDiff );
        mbFixSize = bFix;
        setFrameAreaSizeValid(true);
    }
}

void SwRowFrame::AdjustCells( const SwTwips nHeight, const bool bHeight )
{
    SwFrame *pFrame = Lower();
    if ( bHeight )
    {
        SwRectFnSet aRectFnSet(this);
#if !ENABLE_WASM_STRIP_ACCESSIBILITY
        SwRect aOldFrame;
#endif

        while ( pFrame )
        {
            SwFrame* pNotify = nullptr;

            SwCellFrame* pCellFrame = static_cast<SwCellFrame*>(pFrame);

            // NEW TABLES
            // Which cells need to be adjusted if the current row changes
            // its height?

            // Current frame is a covered frame:
            // Set new height for covered cell and adjust master cell:
            if ( pCellFrame->GetTabBox()->getRowSpan() < 1 )
            {
                // Set height of current (covered) cell to new line height.
                const tools::Long nDiff = nHeight - aRectFnSet.GetHeight(pCellFrame->getFrameArea());
                if ( nDiff )
                {
                    {
                        SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*pCellFrame);
                        aRectFnSet.AddBottom( aFrm, nDiff );
                    }

                    pCellFrame->InvalidatePrt_();
                }
            }

            SwCellFrame* pToAdjust = nullptr;
            SwFrame* pToAdjustRow = nullptr;

            // If current frame is covered frame, we still want to adjust the
            // height of the cell starting the row span
            if ( pCellFrame->GetLayoutRowSpan() < 1 )
            {
                pToAdjust = const_cast< SwCellFrame*>(&pCellFrame->FindStartEndOfRowSpanCell( true ));
                pToAdjustRow = pToAdjust->GetUpper();
            }
            else
            {
                pToAdjust = pCellFrame;
                pToAdjustRow = this;
            }

            // Set height of master cell to height of all lines spanned by this line.
            tools::Long nRowSpan = pToAdjust->GetLayoutRowSpan();
            SwTwips nSumRowHeight = 0;
            while ( pToAdjustRow )
            {
                // Use new height for the current row:
                nSumRowHeight += pToAdjustRow == this ?
                                 nHeight :
                                 aRectFnSet.GetHeight(pToAdjustRow->getFrameArea());

                if ( nRowSpan-- == 1 )
                    break;

                pToAdjustRow = pToAdjustRow->GetNext();
            }

            if ( pToAdjustRow && pToAdjustRow != this )
                pToAdjustRow->InvalidateSize_();

            const tools::Long nDiff = nSumRowHeight - aRectFnSet.GetHeight(pToAdjust->getFrameArea());
            if ( nDiff )
            {
#if !ENABLE_WASM_STRIP_ACCESSIBILITY
                aOldFrame = pToAdjust->getFrameArea();
#endif
                SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*pToAdjust);
                aRectFnSet.AddBottom( aFrm, nDiff );
                pNotify = pToAdjust;
            }

            if ( pNotify )
            {
#if !ENABLE_WASM_STRIP_ACCESSIBILITY
                SwRootFrame *pRootFrame = getRootFrame();
                if( pRootFrame && pRootFrame->IsAnyShellAccessible() && pRootFrame->GetCurrShell() )
                    pRootFrame->GetCurrShell()->Imp()->MoveAccessibleFrame( pNotify, aOldFrame );
#endif

                pNotify->InvalidatePrt_();
            }

            pFrame = pFrame->GetNext();
        }
    }
    else
    {   while ( pFrame )
        {
            pFrame->InvalidateAll_();
            pFrame = pFrame->GetNext();
        }
    }
    InvalidatePage();
}

void SwRowFrame::Cut()
{
    SwTabFrame *pTab = FindTabFrame();
    if ( pTab && pTab->IsFollow() && this == pTab->GetFirstNonHeadlineRow() )
    {
        pTab->FindMaster()->InvalidatePos();
    }

    SwLayoutFrame::Cut();
}

SwTwips SwRowFrame::GrowFrame(SwTwips nDist, SwResizeLimitReason& reason, bool bTst, bool bInfo)
{
    const auto nOrigDist = nDist;
    SwTwips nReal = 0;

    SwTabFrame* pTab = FindTabFrame();
    SwRectFnSet aRectFnSet(pTab);

    bool bRestrictTableGrowth;
    bool bHasFollowFlowLine = pTab->HasFollowFlowLine();

    if ( GetUpper()->IsTabFrame() )
    {
        const SwRowFrame* pFollowFlowRow = IsInSplitTableRow();
        bRestrictTableGrowth = pFollowFlowRow && !pFollowFlowRow->IsRowSpanLine();
    }
    else
    {
        OSL_ENSURE( GetUpper()->IsCellFrame(), "RowFrame->GetUpper neither table nor cell" );
        bRestrictTableGrowth = GetFollowRow() && bHasFollowFlowLine;
        OSL_ENSURE( !bRestrictTableGrowth || !GetNext(),
                "GetFollowRow for row frame that has a Next" );

        // There may still be some space left in my direct upper:
        const SwTwips nAdditionalSpace =
                aRectFnSet.BottomDist( getFrameArea(), aRectFnSet.GetPrtBottom(*GetUpper()->GetUpper()) );
        if ( bRestrictTableGrowth && nAdditionalSpace > 0 )
        {
            nReal = std::min( nAdditionalSpace, nDist );
            nDist -= nReal;
            if ( !bTst )
            {
                SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*this);
                aRectFnSet.AddBottom( aFrm, nReal );
            }
        }
    }

    if ( bRestrictTableGrowth )
        pTab->SetRestrictTableGrowth( true );
    else
    {
        // Ok, this looks like a hack, indeed, it is a hack.
        // If the current row frame is inside another cell frame,
        // and the current row frame has no follow, it should not
        // be allowed to grow. In fact, setting bRestrictTableGrowth
        // to 'false' does not work, because the surrounding RowFrame
        // would set this to 'true'.
        pTab->SetFollowFlowLine( false );
    }

    nReal += SwLayoutFrame::GrowFrame(nDist, reason, bTst, bInfo);

    pTab->SetRestrictTableGrowth( false );
    pTab->SetFollowFlowLine( bHasFollowFlowLine );

    //Update the height of the cells to the newest value.
    if ( !bTst )
    {
        SwRectFnSet fnRectX(this);
        AdjustCells( fnRectX.GetHeight(getFramePrintArea()) + nReal, true );
        if ( nReal )
            SetCompletePaint();
    }
    if (reason == SwResizeLimitReason::Unspecified && nReal < nOrigDist && IsInSplit())
        reason = SwResizeLimitReason::FlowToFollow;
    return nReal;
}

SwTwips SwRowFrame::ShrinkFrame( SwTwips nDist, bool bTst, bool bInfo )
{
    SwRectFnSet aRectFnSet(this);
    if( HasFixSize() )
    {
        AdjustCells( aRectFnSet.GetHeight(getFramePrintArea()), true );
        return 0;
    }

    // bInfo may be set to true by SwRowFrame::Format; we need to handle this
    // here accordingly
    const bool bShrinkAnyway = bInfo;

    //Only shrink as much as the content of the biggest cell allows.
    SwTwips nRealDist = nDist;
    SwFormat* pMod = GetFormat();
    if (pMod)
    {
        const SwFormatFrameSize &rSz = pMod->GetFrameSize();
        SwTwips nMinHeight = 0;
        if (rSz.GetHeightSizeType() == SwFrameSize::Minimum)
            nMinHeight = std::max(rSz.GetHeight() - lcl_calcHeightOfRowBeforeThisFrame(*this),
                                  tools::Long(0));

        // Only necessary to calculate minimal row height if height
        // of pRow is at least nMinHeight. Otherwise nMinHeight is the
        // minimum height.
        if( nMinHeight < aRectFnSet.GetHeight(getFrameArea()) )
        {
            // #i26945#
            OSL_ENSURE( FindTabFrame(), "<SwRowFrame::ShrinkFrame(..)> - no table frame -> crash." );
            const bool bConsiderObjs( FindTabFrame()->IsConsiderObjsForMinCellHeight() );
            nMinHeight = lcl_CalcMinRowHeight( this, bConsiderObjs );
        }

        if ( (aRectFnSet.GetHeight(getFrameArea()) - nRealDist) < nMinHeight )
            nRealDist = aRectFnSet.GetHeight(getFrameArea()) - nMinHeight;
    }
    if ( nRealDist < 0 )
        nRealDist = 0;

    SwTwips nReal = nRealDist;
    if ( nReal )
    {
        if ( !bTst )
        {
            SwTwips nHeight = aRectFnSet.GetHeight(getFrameArea());
            SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*this);
            aRectFnSet.SetHeight( aFrm, nHeight - nReal );

            if( IsVertical() && !IsVertLR() )
            {
                aFrm.Pos().AdjustX(nReal );
            }
        }

        SwLayoutFrame* pFrame = GetUpper();
        SwTwips nTmp = pFrame ? pFrame->Shrink(nReal, bTst) : 0;
        if ( !bShrinkAnyway && !GetNext() && nTmp != nReal )
        {
            //The last one gets the leftover in the upper and therefore takes
            //care (otherwise: endless loop)
            if ( !bTst )
            {
                nReal -= nTmp;
                SwTwips nHeight = aRectFnSet.GetHeight(getFrameArea());
                SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*this);
                aRectFnSet.SetHeight( aFrm, nHeight + nReal );

                if( IsVertical() && !IsVertLR() )
                {
                    aFrm.Pos().AdjustX( -nReal );
                }
            }
            nReal = nTmp;
        }
    }

    // Invalidate appropriately and update the height to the newest value.
    if ( !bTst )
    {
        if ( nReal )
        {
            if ( GetNext() )
                GetNext()->InvalidatePos_();
            InvalidateAll_();
            SetCompletePaint();

            SwTabFrame *pTab = FindTabFrame();
            if ( !pTab->IsRebuildLastLine()
                 && pTab->IsFollow()
                 && this == pTab->GetFirstNonHeadlineRow()
                 && !pTab->IsInRecalcLowerRow() )
            {
                SwTabFrame* pMasterTab = pTab->FindMaster();
                pMasterTab->InvalidatePos();
            }
        }
        AdjustCells( aRectFnSet.GetHeight(getFramePrintArea()) - nReal, true );
    }
    return nReal;
}

bool SwRowFrame::IsRowSplitAllowed() const
{
    // Fixed size rows are never allowed to split:
    if ( HasFixSize() )
    {
        OSL_ENSURE( SwFrameSize::Fixed == GetFormat()->GetFrameSize().GetHeightSizeType(), "pRow claims to have fixed size" );
        return false;
    }

    // Repeated headlines are never allowed to split:
    const SwTabFrame* pTabFrame = FindTabFrame();
    if ( pTabFrame->GetTable()->GetRowsToRepeat() > 0 &&
         pTabFrame->IsInHeadline( *this ) )
        return false;

    if ( IsForceRowSplitAllowed() )
        return true;

    const SwTableLineFormat* pFrameFormat = GetTabLine()->GetFrameFormat();
    const SwFormatRowSplit& rLP = pFrameFormat->GetRowSplit();
    return rLP.GetValue();
}

bool SwRowFrame::ShouldRowKeepWithNext( const bool bCheckParents ) const
{
    // No KeepWithNext if nested in another table
    if ( GetUpper()->GetUpper()->IsCellFrame() )
        return false;

    const SwCellFrame* pCell = static_cast<const SwCellFrame*>(Lower());
    const SwFrame* pText = pCell ? pCell->Lower() : nullptr;

    return pText && pText->IsTextFrame() && !pText->IsHiddenNow() &&
           static_cast<const SwTextFrame*>(pText)->GetTextNodeForParaProps()->GetSwAttrSet().GetKeep(bCheckParents).GetValue();
}

SwCellFrame::SwCellFrame(const SwTableBox &rBox, SwFrame* pSib, bool bInsertContent)
    : SwLayoutFrame( rBox.GetFrameFormat(), pSib )
    , m_pTabBox( &rBox )
{
    mnFrameType = SwFrameType::Cell;

    if ( !bInsertContent )
        return;

    //If a StartIdx is available, ContentFrames are added in the cell, otherwise
    //Rows have to be present and those are added.
    if ( SwNodeOffset nIndex = rBox.GetSttIdx() )
    {
        ::InsertCnt_( this, rBox.GetFrameFormat()->GetDoc(), ++nIndex );
    }
    else
    {
        const SwTableLines &rLines = rBox.GetTabLines();
        SwFrame *pTmpPrev = nullptr;
        for ( size_t i = 0; i < rLines.size(); ++i )
        {
            SwRowFrame *pNew = new SwRowFrame( *rLines[i], this, bInsertContent );
            pNew->InsertBehind( this, pTmpPrev );
            pTmpPrev = pNew;
        }
    }
}

void SwCellFrame::DestroyImpl()
{
    sw::BroadcastingModify* pMod = GetFormat();
    if( pMod )
    {
        // At this stage the lower frames aren't destroyed already,
        // therefore we have to do a recursive dispose.
#if !ENABLE_WASM_STRIP_ACCESSIBILITY
        SwRootFrame *pRootFrame = getRootFrame();
        if( pRootFrame && pRootFrame->IsAnyShellAccessible() &&
            pRootFrame->GetCurrShell() )
        {
            pRootFrame->GetCurrShell()->Imp()->DisposeAccessibleFrame( this, true );
        }
#endif

        pMod->Remove(*this);
        if( !pMod->HasWriterListeners() )
            delete pMod;
    }

    SwLayoutFrame::DestroyImpl();
}

SwCellFrame::~SwCellFrame()
{
}

static bool lcl_ArrangeLowers( SwLayoutFrame *pLay, tools::Long lYStart, bool bInva )
{
    bool bRet = false;
    SwFrame *pFrame = pLay->Lower();
    SwRectFnSet aRectFnSet(pLay);
    while ( pFrame )
    {
        tools::Long nFrameTop = aRectFnSet.GetTop(pFrame->getFrameArea());
        if( nFrameTop != lYStart )
        {
            bRet = true;
            const tools::Long lDiff = aRectFnSet.YDiff( lYStart, nFrameTop );
            const tools::Long lDiffX = lYStart - nFrameTop;

            {
                SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*pFrame);
                aRectFnSet.SubTop( aFrm, -lDiff );
                aRectFnSet.AddBottom( aFrm, lDiff );
            }

            pFrame->SetCompletePaint();

            if ( !pFrame->GetNext() )
                pFrame->SetRetouche();
            if( bInva )
                pFrame->Prepare( PrepareHint::FramePositionChanged );
            if ( pFrame->IsLayoutFrame() && static_cast<SwLayoutFrame*>(pFrame)->Lower() )
                lcl_ArrangeLowers( static_cast<SwLayoutFrame*>(pFrame),
                    aRectFnSet.GetTop(static_cast<SwLayoutFrame*>(pFrame)->Lower()->getFrameArea())
                    + lDiffX, bInva );
            if ( pFrame->GetDrawObjs() )
            {
                for ( size_t i = 0; i < pFrame->GetDrawObjs()->size(); ++i )
                {
                    SwAnchoredObject* pAnchoredObj = (*pFrame->GetDrawObjs())[i];
                    // #i26945# - check, if anchored object
                    // is lower of layout frame by checking, if the anchor
                    // frame, which contains the anchor position, is a lower
                    // of the layout frame.
                    if ( !pLay->IsAnLower( pAnchoredObj->GetAnchorFrameContainingAnchPos() ) )
                    {
                        continue;
                    }
                    // #i52904# - distinguish between anchored
                    // objects, whose vertical position depends on its anchor
                    // frame and whose vertical position is independent
                    // from its anchor frame.
                    bool bVertPosDepOnAnchor( true );
                    {
                        SwFormatVertOrient aVert( pAnchoredObj->GetFrameFormat()->GetVertOrient() );
                        switch ( aVert.GetRelationOrient() )
                        {
                            case text::RelOrientation::PAGE_FRAME:
                            case text::RelOrientation::PAGE_PRINT_AREA:
                                bVertPosDepOnAnchor = false;
                                break;
                            default: break;
                        }
                    }
                    if ( auto pFly = pAnchoredObj->DynCastFlyFrame() )
                    {

                        // OD 2004-05-18 #i28701# - no direct move of objects,
                        // which are anchored to-paragraph/to-character, if
                        // the wrapping style influence has to be considered
                        // on the object positioning.
                        // #i52904# - no direct move of objects,
                        // whose vertical position doesn't depend on anchor frame.
                        const bool bDirectMove =
                                FAR_AWAY != pFly->getFrameArea().Top() &&
                                bVertPosDepOnAnchor &&
                                !pFly->ConsiderObjWrapInfluenceOnObjPos();
                        if ( bDirectMove )
                        {
                            {
                                SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*pFly);
                                aRectFnSet.SubTop( aFrm, -lDiff );
                                aRectFnSet.AddBottom( aFrm, lDiff );
                            }

                            pFly->GetVirtDrawObj()->SetBoundAndSnapRectsDirty();
                            // --> OD 2004-08-17 - also notify view of <SdrObject>
                            // instance, which represents the Writer fly frame in
                            // the drawing layer
                            pFly->GetVirtDrawObj()->SetChanged();
                            // #i58280#
                            pFly->InvalidateObjRectWithSpaces();
                        }

                        if ( pFly->IsFlyInContentFrame() )
                        {
                            static_cast<SwFlyInContentFrame*>(pFly)->AddRefOfst( lDiff );
                            // #115759# - reset current relative
                            // position to get re-positioned, if not directly moved.
                            if ( !bDirectMove )
                            {
                                pAnchoredObj->SetCurrRelPos( Point( 0, 0 ) );
                            }
                        }
                        else if( pFly->IsAutoPos() )
                        {
                            pFly->AddLastCharY( lDiff );
                            // OD 2004-05-18 #i28701# - follow-up of #i22341#
                            // <mnLastTopOfLine> has also been adjusted.
                            pFly->AddLastTopOfLineY( lDiff );
                        }
                        // #i26945# - re-registration at
                        // page frame of anchor frame, if table frame isn't
                        // a follow table and table frame isn't in its
                        // rebuild of last line.
                        const SwTabFrame* pTabFrame = pLay->FindTabFrame();
                        // - save: check, if table frame is found.
                        if ( pTabFrame &&
                             !( pTabFrame->IsFollow() &&
                                pTabFrame->FindMaster()->IsRebuildLastLine() ) &&
                             pFly->IsFlyFreeFrame() )
                        {
                            SwPageFrame* pPageFrame = pFly->GetPageFrame();
                            SwPageFrame* pPageOfAnchor = pFrame->FindPageFrame();
                            if ( pPageFrame != pPageOfAnchor )
                            {
                                pFly->InvalidatePos();
                                pFly->RegisterAtPage(*pPageOfAnchor);
                            }
                        }
                        // OD 2004-05-11 #i28701# - Because of the introduction
                        // of new positionings and alignments (e.g. aligned at
                        // page area, but anchored at-character), the position
                        // of the Writer fly frame has to be invalidated.
                        pFly->InvalidatePos();

                        // #i26945# - follow-up of #i3317#
                        // No arrangement of lowers, if Writer fly frame isn't
                        // moved
                        if ( bDirectMove &&
                             ::lcl_ArrangeLowers( pFly,
                                                  aRectFnSet.GetPrtTop(*pFly),
                                                  bInva ) )
                        {
                            pFly->SetCompletePaint();
                        }
                    }
                    else if ( dynamic_cast< const SwAnchoredDrawObject *>( pAnchoredObj ) !=  nullptr )
                    {
                        // #i26945#
                        const SwTabFrame* pTabFrame = pLay->FindTabFrame();
                        if ( pTabFrame &&
                             !( pTabFrame->IsFollow() &&
                                pTabFrame->FindMaster()->IsRebuildLastLine() ) &&
                            (pAnchoredObj->GetFrameFormat()->GetAnchor().GetAnchorId()
                                                            != RndStdIds::FLY_AS_CHAR))
                        {
                            SwPageFrame* pPageFrame = pAnchoredObj->GetPageFrame();
                            SwPageFrame* pPageOfAnchor = pFrame->FindPageFrame();
                            if ( pPageFrame != pPageOfAnchor )
                            {
                                pAnchoredObj->InvalidateObjPos();
                                pAnchoredObj->RegisterAtPage(*pPageOfAnchor);
                            }
                        }
                        // #i28701# - adjust last character
                        // rectangle and last top of line.
                        pAnchoredObj->AddLastCharY( lDiff );
                        pAnchoredObj->AddLastTopOfLineY( lDiff );
                        // #i52904# - re-introduce direct move
                        // of drawing objects
                        const bool bDirectMove =
                                static_cast<const SwDrawFrameFormat*>(pAnchoredObj->GetFrameFormat())->IsPosAttrSet() &&
                                bVertPosDepOnAnchor &&
                                !pAnchoredObj->ConsiderObjWrapInfluenceOnObjPos();
                        if ( bDirectMove )
                        {
                            SwObjPositioningInProgress aObjPosInProgress( *pAnchoredObj );
                            if ( aRectFnSet.IsVert() )
                            {
                                pAnchoredObj->DrawObj()->Move( Size( lDiff, 0 ) );
                            }
                            else
                            {
                                pAnchoredObj->DrawObj()->Move( Size( 0, lDiff ) );
                            }
                            // #i58280#
                            pAnchoredObj->InvalidateObjRectWithSpaces();
                        }
                        pAnchoredObj->InvalidateObjPos();
                    }
                    else
                    {
                        OSL_FAIL( "<lcl_ArrangeLowers(..)> - unknown type of anchored object!" );
                    }
                }
            }
        }
        // Columns and cells are ordered horizontal, not vertical
        if( !pFrame->IsColumnFrame() && !pFrame->IsCellFrame() )
            lYStart = aRectFnSet.YInc( lYStart,
                                        aRectFnSet.GetHeight(pFrame->getFrameArea()) );

        // Nowadays, the content inside a cell can flow into the follow table.
        // Thus, the cell may only grow up to the end of the environment.
        // So the content may have grown, but the cell could not grow.
        // Therefore we have to trigger a formatting for the frames, which do
        // not fit into the cell anymore:
        SwTwips nDistanceToUpperPrtBottom =
            aRectFnSet.BottomDist( pFrame->getFrameArea(), aRectFnSet.GetPrtBottom(*pLay) );
        // #i56146# - Revise fix of issue #i26945#
        // do *not* consider content inside fly frames, if it's an undersized paragraph.
        // #i26945# - consider content inside fly frames
        if ( nDistanceToUpperPrtBottom < 0 &&
             ( ( pFrame->IsInFly() &&
                 ( !pFrame->IsTextFrame() ||
                   !static_cast<SwTextFrame*>(pFrame)->IsUndersized() ) ) ||
               pFrame->IsInSplitTableRow() ) )
        {
            pFrame->InvalidatePos();
        }

        pFrame = pFrame->GetNext();
    }
    return bRet;
}

void SwCellFrame::Format( vcl::RenderContext* /*pRenderContext*/, const SwBorderAttrs *pAttrs )
{
    OSL_ENSURE( pAttrs, "CellFrame::Format, pAttrs is 0." );
    const SwTabFrame* pTab = FindTabFrame();
    SwRectFnSet aRectFnSet(pTab);
    SwFrame* pLower = Lower();

    if ( !isFramePrintAreaValid() )
    {
        setFramePrintAreaValid(true);

        //Adjust position.
        if ( pLower )
        {
            SwTwips nTopSpace, nBottomSpace, nLeftSpace, nRightSpace;
            // #i29550#
            if ( pTab->IsCollapsingBorders() && !pLower->IsRowFrame()  )
            {
                const SvxBoxItem& rBoxItem = pAttrs->GetBox();
                nLeftSpace   = rBoxItem.GetDistance( SvxBoxItemLine::LEFT );
                nRightSpace  = rBoxItem.GetDistance( SvxBoxItemLine::RIGHT );
                nTopSpace    =  static_cast<SwRowFrame*>(GetUpper())->GetTopMarginForLowers();
                nBottomSpace =  static_cast<SwRowFrame*>(GetUpper())->GetBottomMarginForLowers();
            }
            else
            {
                // OD 23.01.2003 #106895# - add 1st param to <SwBorderAttrs::CalcRight(..)>
                nLeftSpace   = pAttrs->CalcLeft( this );
                nRightSpace  = pAttrs->CalcRight( this );
                nTopSpace    = pAttrs->CalcTop();
                nBottomSpace = pAttrs->CalcBottom();
            }
            aRectFnSet.SetXMargins( *this, nLeftSpace, nRightSpace );
            aRectFnSet.SetYMargins( *this, nTopSpace, nBottomSpace );
        }
    }
    // #i26945#
    tools::Long nRemaining = GetTabBox()->getRowSpan() >= 1 ?
                      ::lcl_CalcMinCellHeight( this, pTab->IsConsiderObjsForMinCellHeight(), pAttrs ) :
                      0;
    if ( !isFrameAreaSizeValid() )
    {
        setFrameAreaSizeValid(true);

        //The VarSize of the CellFrames is always the width.
        //The width is not variable though, it is defined by the format.
        //This predefined value however does not necessary match the actual
        //width. The width is calculated based on the attribute, the value in
        //the attribute matches the desired value of the TabFrame. Changes which
        //were done there are taken into account here proportionately.
        //If the cell doesn't have a neighbour anymore, it does not take the
        //attribute into account and takes the rest of the upper instead.
        SwTwips nWidth;
        if ( GetNext() )
        {
            const SwTwips nWish = pTab->GetFormat()->GetFrameSize().GetWidth();
            nWidth = pAttrs->GetSize().Width();

            OSL_ENSURE( nWish, "Table without width?" );
            OSL_ENSURE( nWidth <= nWish, "Width of cell larger than table." );
            OSL_ENSURE( nWidth > 0, "Box without width" );

            const tools::Long nPrtWidth = aRectFnSet.GetWidth(pTab->getFramePrintArea());
            if ( nWish != nPrtWidth )
            {
                // Avoid rounding problems, at least for the new table model
                if ( pTab->GetTable()->IsNewModel() )
                {
                    // 1. sum of widths of cells up to this cell (in model)
                    const SwTableLine* pTabLine = GetTabBox()->GetUpper();
                    const SwTableBoxes& rBoxes = pTabLine->GetTabBoxes();
                    const SwTableBox* pTmpBox = nullptr;

                    SwTwips nSumWidth = 0;
                    size_t i = 0;
                    do
                    {
                        pTmpBox = rBoxes[ i++ ];
                        nSumWidth += pTmpBox->GetFrameFormat()->GetFrameSize().GetWidth();
                    }
                    while ( pTmpBox != GetTabBox() );

                    // 2. calculate actual width of cells up to this one
                    double nTmpWidth = nSumWidth;
                    nTmpWidth *= nPrtWidth;
                    nTmpWidth /= nWish;
                    nWidth = static_cast<SwTwips>(nTmpWidth);

                    // 3. calculate frame widths of cells up to this one:
                    const SwFrame* pTmpCell = static_cast<const SwLayoutFrame*>(GetUpper())->Lower();
                    SwTwips nSumFrameWidths = 0;
                    while ( pTmpCell != this )
                    {
                        nSumFrameWidths += aRectFnSet.GetWidth(pTmpCell->getFrameArea());
                        pTmpCell = pTmpCell->GetNext();
                    }

                    nWidth = nWidth - nSumFrameWidths;
                }
                else
                {
                    // #i12092# use double instead of long,
                    // otherwise this could lead to overflows
                    double nTmpWidth = nWidth;
                    nTmpWidth *= nPrtWidth;
                    nTmpWidth /= nWish;
                    nWidth = static_cast<SwTwips>(nTmpWidth);
                }
            }
        }
        else
        {
            OSL_ENSURE( pAttrs->GetSize().Width() > 0, "Box without width" );
            nWidth = aRectFnSet.GetWidth(GetUpper()->getFramePrintArea());
            SwFrame *pPre = GetUpper()->Lower();
            while ( pPre != this )
            {
                nWidth -= aRectFnSet.GetWidth(pPre->getFrameArea());
                pPre = pPre->GetNext();
            }
        }

        const tools::Long nDiff = nWidth - aRectFnSet.GetWidth(getFrameArea());

        {
            SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*this);

            if( IsNeighbourFrame() && IsRightToLeft() )
            {
                aRectFnSet.SubLeft( aFrm, nDiff );
            }
            else
            {
                aRectFnSet.AddRight( aFrm, nDiff );
            }
        }

        {
            SwFrameAreaDefinition::FramePrintAreaWriteAccess aPrt(*this);
            aRectFnSet.AddRight( aPrt, nDiff );
        }

        //Adjust the height, it's defined through the content and the margins.
        const tools::Long nDiffHeight = nRemaining - aRectFnSet.GetHeight(getFrameArea());
        if ( nDiffHeight )
        {
            if ( nDiffHeight > 0 )
            {
                //Validate again if no growth happened. Invalidation is done
                //through AdjustCells of the row.
                if ( !Grow( nDiffHeight ) )
                {
                    setFrameAreaSizeValid(true);
                    setFramePrintAreaValid(true);
                }
            }
            else
            {
                // Only keep invalidated if shrinking was actually done; the
                // attempt can be ignored because all horizontally adjoined
                // cells have to be the same height.
                if ( !Shrink( -nDiffHeight ) )
                {
                    setFrameAreaSizeValid(true);
                    setFramePrintAreaValid(true);
                }
            }
        }
    }
    const SwFormatVertOrient &rOri = pAttrs->GetAttrSet().GetVertOrient();

    if ( !pLower )
        return;

    // From now on, all operations are related to the table cell.
    aRectFnSet.Refresh(this);

    SwPageFrame* pPg = nullptr;
    if ( !FindTabFrame()->IsRebuildLastLine() && text::VertOrientation::NONE != rOri.GetVertOrient() &&
    // #158225# no vertical alignment of covered cells
         !IsCoveredCell() &&
         (pPg = FindPageFrame())!=nullptr )
    {
        if ( !pLower->IsContentFrame() && !pLower->IsSctFrame() && !pLower->IsTabFrame() )
        {
            // OSL_ENSURE(for HTML-import!
            OSL_ENSURE( false, "VAlign to cell without content" );
            return;
        }
        bool bVertDir = true;
        // #i43913# - no vertical alignment, if wrapping
        // style influence is considered on object positioning and
        // an object is anchored inside the cell.
        // No alignment if fly with wrap overlaps the cell.
        if ( pPg->GetSortedObjs() )
        {
            SwRect aRect( getFramePrintArea() ); aRect += getFrameArea().Pos();
            for (SwAnchoredObject* pAnchoredObj : *pPg->GetSortedObjs())
            {
                SwRect aTmp( pAnchoredObj->GetObjRect() );
                if (aTmp.Overlaps(aRect))
                {
                    const SwFrameFormat* pAnchoredObjFrameFormat = pAnchoredObj->GetFrameFormat();
                    const SwFormatSurround &rSur = pAnchoredObjFrameFormat->GetSurround();
                    const IDocumentSettingAccess& rIDSA = GetFormat()->getIDocumentSettingAccess();

                    // Note: I think that technically, bConsiderWrapOnObjPos should not apply here.
                    // However, Word's UI strongly encourages vertical offsets that match
                    // the layout that results from this use of bConsiderWrapOnObjPos. tdf#166710
                    const bool bConsiderWrapOnObjPos
                        = rIDSA.get(DocumentSettingId::CONSIDER_WRAP_ON_OBJECT_POSITION);
                    const bool bForceTopVAlign = bConsiderWrapOnObjPos && rIDSA.get(
                        DocumentSettingId::FORCE_TOP_ALIGNMENT_IN_CELL_WITH_FLOATING_ANCHOR);

                    if (bForceTopVAlign || css::text::WrapTextMode_THROUGH != rSur.GetSurround())
                    {
                        // frames, which the cell is a lower of, aren't relevant
                        if ( auto pFly = pAnchoredObj->DynCastFlyFrame() )
                        {
                            if ( pFly->IsAnLower( this ) )
                                continue;
                        }

                        const SwFrame* pAnch = pAnchoredObj->GetAnchorFrame();
                        // #i43913#
                        // #i52904# - no vertical alignment,
                        // if object, anchored inside cell, has temporarily
                        // consider its wrapping style on object positioning.
                        // #i58806# - no vertical alignment
                        // if object does not follow the text flow.
                        if ( bConsiderWrapOnObjPos ||
                             !IsAnLower( pAnch ) ||
                             pAnchoredObj->IsTmpConsiderWrapInfluence() ||
                             !pAnchoredObjFrameFormat->GetFollowTextFlow().GetValue() )
                        {
                            bVertDir = false;
                            break;
                        }
                    }
                }
            }
        }

        tools::Long nPrtHeight = aRectFnSet.GetHeight(getFramePrintArea());
        if( ( bVertDir && ( nRemaining -= lcl_CalcTopAndBottomMargin( *this, *pAttrs ) ) < nPrtHeight ) ||
            aRectFnSet.GetTop(pLower->getFrameArea()) != aRectFnSet.GetPrtTop(*this) )
        {
            tools::Long nDiff = aRectFnSet.GetHeight(getFramePrintArea()) - nRemaining;
            if ( nDiff >= 0 )
            {
                tools::Long lTopOfst = 0;
                if ( bVertDir )
                {
                    switch ( rOri.GetVertOrient() )
                    {
                        case text::VertOrientation::CENTER:   lTopOfst = nDiff / 2; break;
                        case text::VertOrientation::BOTTOM:   lTopOfst = nDiff;     break;
                        default: break;
                    }
                }
                tools::Long nTmp = aRectFnSet.YInc(
                                    aRectFnSet.GetPrtTop(*this), lTopOfst );
                if ( lcl_ArrangeLowers( this, nTmp, !bVertDir ) )
                    SetCompletePaint();
            }
        }
    }
    else
    {
        //Was an old alignment taken into account?
        if ( pLower->IsContentFrame() )
        {
            const tools::Long lYStart = aRectFnSet.GetPrtTop(*this);
            lcl_ArrangeLowers( this, lYStart, true );
        }
    }

    // Handle rotated portions of lowers: it's possible that we have changed amount of vertical
    // space since the last format, and this affects how many rotated portions we need. So throw
    // away the current portions to build them using the new line width.
    for (SwFrame* pFrame = pLower; pFrame; pFrame = pFrame->GetNext())
    {
        if (!pFrame->IsTextFrame())
        {
            continue;
        }

        auto pTextFrame = static_cast<SwTextFrame*>(pFrame);
        if (!pTextFrame->GetHasRotatedPortions())
        {
            continue;
        }

        pTextFrame->Prepare();
    }
}

void SwCellFrame::SwClientNotify(const SwModify& rMod, const SfxHint& rHint)
{
    if(rHint.GetId() == SfxHintId::SwTableBoxFormatChanged)
    {
        auto pNewFormatHint = static_cast<const sw::TableBoxFormatChanged*>(&rHint);
        if(GetTabBox() != &pNewFormatHint->m_rTableBox)
            return;
        RegisterToFormat(const_cast<SwTableBoxFormat&>(pNewFormatHint->m_rNewFormat));
        InvalidateSize();
        InvalidatePrt_();
        SetCompletePaint();
        SetDerivedVert(false);
        CheckDirChange();

        // #i47489#
        // make sure that the row will be formatted, in order
        // to have the correct Get(Top|Bottom)MarginForLowers values
        // set at the row.
        const SwTabFrame* pTab = FindTabFrame();
        if(pTab && pTab->IsCollapsingBorders())
        {
            SwFrame* pRow = GetUpper();
            pRow->InvalidateSize_();
            pRow->InvalidatePrt_();
        }
    }
    else if(rHint.GetId() == SfxHintId::SwMoveTableBox)
    {
        auto pMoveTableBoxHint = static_cast<const sw::MoveTableBoxHint*>(&rHint);
        if(GetTabBox() != &pMoveTableBoxHint->m_rTableBox)
            return;
        const_cast<SwFrameFormat*>(&pMoveTableBoxHint->m_rNewFormat)->Add(*this);
        InvalidateAll();
        ReinitializeFrameSizeAttrFlags();
        SetDerivedVert(false);
        CheckDirChange();
    }
    else if (rHint.GetId() == SfxHintId::SwFormatChange
            || rHint.GetId() == SfxHintId::SwObjectDying
            || rHint.GetId() == SfxHintId::SwUpdateAttr)
    {
        SwLayoutFrame::SwClientNotify(rMod, rHint);
    }
    else if (rHint.GetId() == SfxHintId::SwLegacyModify || rHint.GetId() == SfxHintId::SwAttrSetChange)
    {
        const SfxPoolItem* pVertOrientItem = nullptr;
#if !ENABLE_WASM_STRIP_ACCESSIBILITY
        const SfxPoolItem* pProtectItem = nullptr;
#endif
        const SfxPoolItem* pFrameDirItem = nullptr;
        const SfxPoolItem* pBoxItem = nullptr;
        if (rHint.GetId() == SfxHintId::SwLegacyModify)
        {
            auto pLegacy = static_cast<const sw::LegacyModifyHint*>(&rHint);
            const auto nWhich = pLegacy->m_pNew ? pLegacy->m_pNew->Which() : 0;
            switch(nWhich)
            {
                case RES_VERT_ORIENT:
                    pVertOrientItem = pLegacy->m_pNew;
                    break;
                case RES_PROTECT:
        #if !ENABLE_WASM_STRIP_ACCESSIBILITY
                    pProtectItem = pLegacy->m_pNew;
        #endif
                    break;
                case RES_FRAMEDIR:
                    pFrameDirItem = pLegacy->m_pNew;
                    break;
                case RES_BOX:
                    pBoxItem = pLegacy->m_pNew;
                    break;
            }
        }
        else // rHint.GetId() == SfxHintId::SwAttrSetChange
        {
            auto pChangeHint = static_cast<const sw::AttrSetChangeHint*>(&rHint);
            auto& rChgSet = *pChangeHint->m_pNew->GetChgSet();
            pVertOrientItem = rChgSet.GetItemIfSet(RES_VERT_ORIENT, false);
#if !ENABLE_WASM_STRIP_ACCESSIBILITY
            pProtectItem = rChgSet.GetItemIfSet(RES_PROTECT, false);
#endif
            pFrameDirItem = rChgSet.GetItemIfSet(RES_FRAMEDIR, false);
            pBoxItem = rChgSet.GetItemIfSet(RES_BOX, false);
        }
        if(pVertOrientItem)
        {
            bool bInva = true;
            const auto eVertOrient = static_cast<const SwFormatVertOrient*>(pVertOrientItem)->GetVertOrient();
            SwFrame* pLower = Lower();
            if(text::VertOrientation::NONE == eVertOrient && pLower && pLower->IsContentFrame())
            {
                SwRectFnSet aRectFnSet(this);
                const tools::Long lYStart = aRectFnSet.GetPrtTop(*this);
                bInva = lcl_ArrangeLowers(this, lYStart, false);
            }
            if (bInva)
            {
                SetCompletePaint();
                InvalidatePrt();
            }
        }
#if !ENABLE_WASM_STRIP_ACCESSIBILITY
        if(pProtectItem)
        {
            SwViewShell* pSh = getRootFrame()->GetCurrShell();
            if(pSh && pSh->GetLayout()->IsAnyShellAccessible())
                pSh->Imp()->InvalidateAccessibleEditableState(true, this);
        }
#endif
        if(pFrameDirItem)
        {
            SetDerivedVert(false);
            CheckDirChange();
        }
        // #i29550#
        if(pBoxItem)
        {
            SwFrame* pTmpUpper = GetUpper();
            while(pTmpUpper->GetUpper() && !pTmpUpper->GetUpper()->IsTabFrame())
                pTmpUpper = pTmpUpper->GetUpper();

            SwTabFrame* pTabFrame = static_cast<SwTabFrame*>(pTmpUpper->GetUpper());
            if(pTabFrame->IsCollapsingBorders())
            {
                // Invalidate lowers of this and next row:
                lcl_InvalidateAllLowersPrt(static_cast<SwRowFrame*>(pTmpUpper));
                pTmpUpper = pTmpUpper->GetNext();
                if(pTmpUpper)
                    lcl_InvalidateAllLowersPrt(static_cast<SwRowFrame*>(pTmpUpper));
                else
                    pTabFrame->InvalidatePrt();
            }
        }
        SwLayoutFrame::SwClientNotify(rMod, rHint);
    }
}

tools::Long SwCellFrame::GetLayoutRowSpan() const
{
    const SwTableBox *pTabBox = GetTabBox();
    tools::Long nRet = pTabBox ? pTabBox->getRowSpan() : 0;
    if ( nRet < 1 )
    {
        const SwFrame* pRow = GetUpper();
        const SwTabFrame* pTab = pRow ? static_cast<const SwTabFrame*>(pRow->GetUpper()) : nullptr;

        if ( pTab && pTab->IsFollow() && pRow == pTab->GetFirstNonHeadlineRow() )
            nRet = -nRet;
    }
    return  nRet;
}

const SwCellFrame* SwCellFrame::GetCoveredCellInRow(const SwRowFrame& rRow) const
{
    if (GetLayoutRowSpan() <= 1)
    {
        // Not merged vertically.
        return nullptr;
    }

    for (const SwFrame* pCell = rRow.GetLower(); pCell; pCell = pCell->GetNext())
    {
        if (!pCell->IsCellFrame())
        {
            continue;
        }

        auto pCellFrame = static_cast<const SwCellFrame*>(pCell);
        if (!pCellFrame->IsCoveredCell())
        {
            continue;
        }

        if (pCellFrame->getFrameArea().Left() != getFrameArea().Left())
        {
            continue;
        }

        if (pCellFrame->getFrameArea().Width() != getFrameArea().Width())
        {
            continue;
        }

        // pCellFrame is covered, there are only covered cell frames between "this" and pCellFrame
        // and the horizontal position/size matches "this".
        return pCellFrame;
    }

    return nullptr;
}

std::vector<const SwCellFrame*> SwCellFrame::GetCoveredCells() const
{
    std::vector<const SwCellFrame*> aRet;
    if (GetLayoutRowSpan() <= 1)
    {
        return aRet;
    }

    if (!GetUpper()->IsRowFrame())
    {
        return aRet;
    }

    auto pFirstRowFrame = static_cast<const SwRowFrame*>(GetUpper());
    if (!pFirstRowFrame->GetNext())
    {
        return aRet;
    }

    if (!pFirstRowFrame->GetNext()->IsRowFrame())
    {
        return aRet;
    }

    for (const SwFrame* pRow = pFirstRowFrame->GetNext(); pRow; pRow = pRow->GetNext())
    {
        if (!pRow->IsRowFrame())
        {
            continue;
        }

        auto pRowFrame = static_cast<const SwRowFrame*>(pRow);
        const SwCellFrame* pCovered = GetCoveredCellInRow(*pRowFrame);
        if (!pCovered)
        {
            continue;
        }

        // Found a cell in a next row that is covered by "this".
        aRet.push_back(pCovered);
    }

    return aRet;
}

void SwCellFrame::dumpAsXmlAttributes(xmlTextWriterPtr pWriter) const
{
    SwFrame::dumpAsXmlAttributes(pWriter);
    if (SwCellFrame* pFollow = GetFollowCell())
        (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("follow"), "%" SAL_PRIuUINT32, pFollow->GetFrameId());

    if (SwCellFrame* pPrevious = GetPreviousCell())
        (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("precede"), "%" SAL_PRIuUINT32, pPrevious->GetFrameId());
}

void SwCellFrame::dumpAsXml(xmlTextWriterPtr writer) const
{
    (void)xmlTextWriterStartElement(writer, reinterpret_cast<const xmlChar*>("cell"));
    dumpAsXmlAttributes(writer);
    (void)xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "rowspan" ), "%ld", GetLayoutRowSpan() );

    (void)xmlTextWriterStartElement(writer, BAD_CAST("infos"));
    dumpInfosAsXml(writer);
    (void)xmlTextWriterEndElement(writer);
    dumpChildrenAsXml(writer);

    (void)xmlTextWriterEndElement(writer);
}

// #i103961#
void SwCellFrame::Cut()
{
    // notification for accessibility
#if !ENABLE_WASM_STRIP_ACCESSIBILITY
    {
        SwRootFrame *pRootFrame = getRootFrame();
        if( pRootFrame && pRootFrame->IsAnyShellAccessible() )
        {
            SwViewShell* pVSh = pRootFrame->GetCurrShell();
            if ( pVSh && pVSh->Imp() )
            {
                pVSh->Imp()->DisposeAccessibleFrame( this );
            }
        }
    }
#endif

    SwLayoutFrame::Cut();
}

// Helper functions for repeated headlines:

bool SwTabFrame::IsInHeadline( const SwFrame& rFrame ) const
{
    OSL_ENSURE( IsAnLower( &rFrame ) && rFrame.IsInTab(),
             "SwTabFrame::IsInHeadline called for frame not lower of table" );

    const SwFrame* pTmp = &rFrame;
    while ( !pTmp->GetUpper()->IsTabFrame() )
        pTmp = pTmp->GetUpper();

    return GetTable()->IsHeadline( *static_cast<const SwRowFrame*>(pTmp)->GetTabLine() );
}

/*
 * If this is a master table, we can may assume, that there are at least
 * nRepeat lines in the table.
 * If this is a follow table, there are intermediate states for the table
 * layout, e.g., during deletion of rows, which makes it necessary to find
 * the first non-headline row by evaluating the headline flag at the row frame.
 */
SwRowFrame* SwTabFrame::GetFirstNonHeadlineRow() const
{
    SwRowFrame* pRet = const_cast<SwRowFrame*>(static_cast<const SwRowFrame*>(Lower()));
    if ( pRet )
    {
        if ( IsFollow() )
        {
            while ( pRet && pRet->IsRepeatedHeadline() )
                pRet = static_cast<SwRowFrame*>(pRet->GetNext());
        }
        else
        {
            sal_uInt16 nRepeat = GetTable()->GetRowsToRepeat();
            while ( pRet && nRepeat > 0 )
            {
                pRet = static_cast<SwRowFrame*>(pRet->GetNext());
                --nRepeat;
            }
        }
    }

    return pRet;
}

bool SwTable::IsHeadline( const SwTableLine& rLine ) const
{
    for ( sal_uInt16 i = 0; i < GetRowsToRepeat(); ++i )
        if ( GetTabLines()[ i ] == &rLine )
            return true;

    return false;
}

bool SwTabFrame::IsLayoutSplitAllowed() const
{
    return GetFormat()->GetLayoutSplit().GetValue();
}

// #i29550#

sal_uInt16 SwTabFrame::GetBottomLineSize() const
{
    OSL_ENSURE( IsCollapsingBorders(),
            "BottomLineSize only required for collapsing borders" );

    OSL_ENSURE( Lower(), "Warning! Trying to prevent a crash" );

    const SwFrame* pTmp = GetLastLower();

    // #124755# Try to make code robust
    if ( !pTmp ) return 0;

    return static_cast<const SwRowFrame*>(pTmp)->GetBottomLineSize();
}

bool SwTabFrame::IsCollapsingBorders() const
{
    return GetFormat()->GetAttrSet().Get( RES_COLLAPSING_BORDERS ).GetValue();
}

void SwTabFrame::dumpAsXml(xmlTextWriterPtr writer) const
{
    (void)xmlTextWriterStartElement(writer, reinterpret_cast<const xmlChar*>("tab"));
    SwFrame::dumpAsXmlAttributes( writer );

    (void)xmlTextWriterWriteAttribute(writer, BAD_CAST("has-follow-flow-line"),
                                      BAD_CAST(OString::boolean(m_bHasFollowFlowLine).getStr()));

    if ( HasFollow() )
        (void)xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "follow" ), "%" SAL_PRIuUINT32, GetFollow()->GetFrameId() );

    if (m_pPrecede != nullptr)
        (void)xmlTextWriterWriteFormatAttribute( writer, BAD_CAST( "precede" ), "%" SAL_PRIuUINT32, static_cast<SwTabFrame*>( m_pPrecede )->GetFrameId() );

    (void)xmlTextWriterStartElement(writer, BAD_CAST("infos"));
    dumpInfosAsXml(writer);
    (void)xmlTextWriterEndElement(writer);
    dumpChildrenAsXml(writer);

    (void)xmlTextWriterEndElement(writer);
}

/// Local helper function to calculate height of first text row
static SwTwips lcl_CalcHeightOfFirstContentLine( const SwRowFrame& rSourceLine )
{
    // Find corresponding split line in master table
    const SwTabFrame* pTab = rSourceLine.FindTabFrame();
    SwRectFnSet aRectFnSet(pTab);
    const SwCellFrame* pCurrSourceCell = static_cast<const SwCellFrame*>(rSourceLine.Lower());

    // 1. Case: rSourceLine is a follow flow line.
    // In this case we have to return the minimum of the heights
    // of the first lines in rSourceLine.

    // 2. Case: rSourceLine is not a follow flow line.
    // In this case we have to return the maximum of the heights
    // of the first lines in rSourceLine.
    bool bIsInFollowFlowLine = rSourceLine.IsInFollowFlowRow();
    SwTwips nHeight = bIsInFollowFlowLine ? LONG_MAX : 0;

    while ( pCurrSourceCell )
    {
        // NEW TABLES
        // Skip cells which are not responsible for the height of
        // the follow flow line:
        if ( bIsInFollowFlowLine && pCurrSourceCell->GetLayoutRowSpan() > 1 )
        {
            pCurrSourceCell = static_cast<const SwCellFrame*>(pCurrSourceCell->GetNext());
            continue;
        }

        const SwFrame *pTmp = pCurrSourceCell->Lower();
        if ( pTmp )
        {
            SwTwips nTmpHeight = std::numeric_limits<SwTwips>::max();
            // #i32456# Consider lower row frames
            if ( pTmp->IsRowFrame() )
            {
                const SwRowFrame* pTmpSourceRow = static_cast<const SwRowFrame*>(pCurrSourceCell->Lower());
                nTmpHeight = lcl_CalcHeightOfFirstContentLine( *pTmpSourceRow );
            }
            else if (pTmp->IsTabFrame() || (pTmp->IsSctFrame() && pTmp->GetLower() && pTmp->GetLower()->IsTabFrame()))
            {
                SwTabFrame const*const pTabFrame(pTmp->IsTabFrame()
                        ? static_cast<SwTabFrame const*>(pTmp)
                        : static_cast<SwTabFrame const*>(pTmp->GetLower()));
                nTmpHeight = pTabFrame->CalcHeightOfFirstContentLine();
            }
            else if (pTmp->IsTextFrame() || (pTmp->IsSctFrame() && pTmp->GetLower() && pTmp->GetLower()->IsTextFrame()))
            {
                // Section frames don't influence the size/position of text
                // frames, so 'text frame' and 'text frame in section frame' is
                // the same case.
                SwTextFrame* pTextFrame = nullptr;
                if (pTmp->IsTextFrame())
                    pTextFrame = const_cast<SwTextFrame*>(static_cast<const SwTextFrame*>(pTmp));
                else
                    pTextFrame = const_cast<SwTextFrame*>(static_cast<const SwTextFrame*>(pTmp->GetLower()));
                pTextFrame->GetFormatted();
                nTmpHeight = pTextFrame->FirstLineHeight();
            }

            if (std::numeric_limits<SwTwips>::max() != nTmpHeight)
            {
                const SwCellFrame* pPrevCell = pCurrSourceCell->GetPreviousCell();
                if ( pPrevCell )
                {
                    // If we are in a split row, there may be some space
                    // left in the cell frame of the master row.
                    // We look for the minimum of all first line heights;
                    SwTwips nReal = aRectFnSet.GetHeight(pPrevCell->getFramePrintArea());
                    const SwFrame* pFrame = pPrevCell->Lower();
                    const SwFrame* pLast = pFrame;
                    while ( pFrame )
                    {
                        nReal -= aRectFnSet.GetHeight(pFrame->getFrameArea());
                        pLast = pFrame;
                        pFrame = pFrame->GetNext();
                    }

                    // #i26831#, #i26520#
                    // The additional lower space of the current last.
                    // #115759# - do *not* consider the
                    // additional lower space for 'master' text frames
                    if ( pLast && pLast->IsFlowFrame() &&
                         ( !pLast->IsTextFrame() ||
                           !static_cast<const SwTextFrame*>(pLast)->GetFollow() ) )
                    {
                        nReal += SwFlowFrame::CastFlowFrame(pLast)->CalcAddLowerSpaceAsLastInTableCell();
                    }
                    // Don't forget the upper space and lower space,
                    // #115759# - do *not* consider the upper
                    // and the lower space for follow text frames.
                    if ( pTmp->IsFlowFrame() &&
                         ( !pTmp->IsTextFrame() ||
                           !static_cast<const SwTextFrame*>(pTmp)->IsFollow() ) )
                    {
                        nTmpHeight += SwFlowFrame::CastFlowFrame(pTmp)->CalcUpperSpace( nullptr, pLast);
                        nTmpHeight += SwFlowFrame::CastFlowFrame(pTmp)->CalcLowerSpace();
                    }
                    // #115759# - consider additional lower
                    // space of <pTmp>, if contains only one line.
                    // In this case it would be the new last text frame, which
                    // would have no follow and thus would add this space.
                    if ( pTmp->IsTextFrame() &&
                         const_cast<SwTextFrame*>(static_cast<const SwTextFrame*>(pTmp))
                            ->GetLineCount(TextFrameIndex(COMPLETE_STRING)) == 1)
                    {
                        nTmpHeight += SwFlowFrame::CastFlowFrame(pTmp)
                                        ->CalcAddLowerSpaceAsLastInTableCell();
                    }
                    if ( nReal > 0 )
                        nTmpHeight -= nReal;
                }
                else
                {
                    // pFirstRow is not a FollowFlowRow. In this case,
                    // we look for the maximum of all first line heights:
                    SwBorderAttrAccess aAccess( SwFrame::GetCache(), pCurrSourceCell );
                    const SwBorderAttrs &rAttrs = *aAccess.Get();
                    nTmpHeight += rAttrs.CalcTop() + rAttrs.CalcBottom();
                    // #i26250#
                    // Don't forget the upper space and lower space,
                    if ( pTmp->IsFlowFrame() )
                    {
                        nTmpHeight += SwFlowFrame::CastFlowFrame(pTmp)->CalcUpperSpace();
                        nTmpHeight += SwFlowFrame::CastFlowFrame(pTmp)->CalcLowerSpace();
                    }
                }
            }

            if ( bIsInFollowFlowLine )
            {
                // minimum
                if ( nTmpHeight < nHeight )
                    nHeight = nTmpHeight;
            }
            else
            {
                // maximum
                if (nTmpHeight > nHeight && std::numeric_limits<SwTwips>::max() != nTmpHeight)
                    nHeight = nTmpHeight;
            }
        }

        pCurrSourceCell = static_cast<const SwCellFrame*>(pCurrSourceCell->GetNext());
    }

    return ( LONG_MAX == nHeight ) ? 0 : nHeight;
}

/// Function to calculate height of first text row
SwTwips SwTabFrame::CalcHeightOfFirstContentLine() const
{
    SwRectFnSet aRectFnSet(this);

    const bool bDontSplit = !IsFollow() && !GetFormat()->GetLayoutSplit().GetValue();

    if ( bDontSplit )
    {
        // Table is not allowed to split: Take the whole height, that's all
        return aRectFnSet.GetHeight(getFrameArea());
    }

    const SwRowFrame* pFirstRow = GetFirstNonHeadlineRow();
    OSL_ENSURE( !IsFollow() || pFirstRow, "FollowTable without Lower" );

    // NEW TABLES
    bool bHasRowSpanLine = pFirstRow && pFirstRow->IsRowSpanLine() && pFirstRow->GetNext();
    if (bHasRowSpanLine)
        pFirstRow = static_cast<const SwRowFrame*>(pFirstRow->GetNext());
    const SwRowFrame* pFirstKeepTogetherRow = pFirstRow;

    // Check how many rows want to keep together
    sal_uInt16 nKeepRows = 0;
    if ( GetFormat()->GetDoc().GetDocumentSettingManager().get(DocumentSettingId::TABLE_ROW_KEEP) )
    {
        while ( pFirstRow && pFirstRow->ShouldRowKeepWithNext() )
        {
            ++nKeepRows;
            pFirstRow = static_cast<const SwRowFrame*>(pFirstRow->GetNext());
        }
    }

    SwTwips nTmpHeight;

    // For master tables, the height of the headlines + the height of the
    // keeping lines (if any) has to be considered. For follow tables, we
    // only consider the height of the keeping rows without the repeated lines:
    if ( !IsFollow() )
    {
        nKeepRows += GetTable()->GetRowsToRepeat();
        if (bHasRowSpanLine)
            ++nKeepRows;
        nTmpHeight = lcl_GetHeightOfRows(GetLower(), nKeepRows);
    }
    else
    {
        nTmpHeight = lcl_GetHeightOfRows(pFirstKeepTogetherRow, nKeepRows);
    }

    // pFirstRow row is the first non-heading row.
    // nTmpHeight is the height of the heading row if we are a follow.
    if ( pFirstRow )
    {
        const bool bSplittable = pFirstRow->IsRowSplitAllowed();
        const SwTwips nFirstLineHeight = aRectFnSet.GetHeight(pFirstRow->getFrameArea());

        if ( !bSplittable )
        {
            // pFirstRow is not splittable, but it is still possible that the line height of pFirstRow
            // actually is determined by a lower cell with rowspan = -1. In this case we should not
            // just return the height of the first line. Basically we need to get the height of the
            // line as it would be on the last page. Since this is quite complicated to calculate,
            // we only calculate the height of the first line.
            SwFormatFrameSize const& rFrameSize(pFirstRow->GetAttrSet()->GetFrameSize());
            if ( pFirstRow->GetPrev() &&
                 static_cast<const SwRowFrame*>(pFirstRow->GetPrev())->IsRowSpanLine()
                && rFrameSize.GetHeightSizeType() != SwFrameSize::Fixed)
            {
                // Calculate maximum height of all cells with rowspan = 1:
                SwTwips nMaxHeight = rFrameSize.GetHeightSizeType() == SwFrameSize::Minimum
                    ? rFrameSize.GetHeight()
                    : 0;
                const SwCellFrame* pLower2 = static_cast<const SwCellFrame*>(pFirstRow->Lower());
                while ( pLower2 )
                {
                    if ( 1 == pLower2->GetTabBox()->getRowSpan() )
                    {
                        const SwTwips nCellHeight = lcl_CalcMinCellHeight( pLower2, true );
                        nMaxHeight = std::max( nCellHeight, nMaxHeight );
                    }
                    pLower2 = static_cast<const SwCellFrame*>(pLower2->GetNext());
                }
                nTmpHeight += nMaxHeight;
            }
            else
            {
                nTmpHeight += nFirstLineHeight;
            }
        }

        // Optimization: lcl_CalcHeightOfFirstContentLine actually can trigger
        // a formatting of the row frame (via the GetFormatted()). We don't
        // want this formatting if the row does not have a height.
        else if ( 0 != nFirstLineHeight )
        {
            const bool bOldJoinLock = IsJoinLocked();
            const_cast<SwTabFrame*>(this)->LockJoin();
            const SwTwips nHeightOfFirstContentLine = lcl_CalcHeightOfFirstContentLine( *pFirstRow );

            // Consider minimum row height:
            const SwFormatFrameSize &rSz = pFirstRow->GetFormat()->GetFrameSize();

            SwTwips nMinRowHeight = 0;
            if (rSz.GetHeightSizeType() == SwFrameSize::Minimum)
            {
                nMinRowHeight = std::max(rSz.GetHeight() - lcl_calcHeightOfRowBeforeThisFrame(*pFirstRow),
                                         tools::Long(0));
            }

            nTmpHeight += std::max( nHeightOfFirstContentLine, nMinRowHeight );

            if ( !bOldJoinLock )
                const_cast<SwTabFrame*>(this)->UnlockJoin();
        }
    }

    return nTmpHeight;
}

// Some more functions for covered/covering cells. This way inclusion of
// SwCellFrame can be avoided

bool SwFrame::IsLeaveUpperAllowed() const
{
    return false;
}

bool SwCellFrame::IsLeaveUpperAllowed() const
{
    return GetLayoutRowSpan() > 1;
}

bool SwFrame::IsCoveredCell() const
{
    return false;
}

bool SwCellFrame::IsCoveredCell() const
{
    return GetLayoutRowSpan() < 1;
}

bool SwFrame::IsInCoveredCell() const
{
    bool bRet = false;

    const SwFrame* pThis = this;
    while ( pThis && !pThis->IsCellFrame() )
        pThis = pThis->GetUpper();

    if ( pThis )
        bRet = pThis->IsCoveredCell();

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
