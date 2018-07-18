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

#include <pagefrm.hxx>
#include <rootfrm.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <viewimp.hxx>
#include <fesh.hxx>
#include <swtable.hxx>
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
#include <notxtfrm.hxx>
#include <htmltbl.hxx>
#include <sectfrm.hxx>
#include <fmtfollowtextflow.hxx>
#include <sortedobjs.hxx>
#include <objectformatter.hxx>
#include <layouter.hxx>
#include <calbck.hxx>
#include <DocumentSettingManager.hxx>
#include <docary.hxx>
#include <o3tl/make_unique.hxx>

using namespace ::com::sun::star;

SwTabFrame::SwTabFrame( SwTable &rTab, SwFrame* pSib )
    : SwLayoutFrame( rTab.GetFrameFormat(), pSib )
    , SwFlowFrame( static_cast<SwFrame&>(*this) )
    , m_pTable( &rTab )
    , m_bComplete(false)
    , m_bCalcLowers(false)
    , m_bLowersFormatted(false)
    , m_bLockBackMove(false)
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
    for ( size_t i = 0; i < rLines.size(); ++i )
    {
        SwRowFrame *pNew = new SwRowFrame( *rLines[i], this );
        if( pNew->Lower() )
        {
            pNew->InsertBehind( this, pTmpPrev );
            pTmpPrev = pNew;
        }
        else
            SwFrame::DestroyFrame(pNew);
    }
    OSL_ENSURE( Lower() && Lower()->IsRowFrame(), "SwTabFrame::SwTabFrame: No rows." );
}

SwTabFrame::SwTabFrame( SwTabFrame &rTab )
    : SwLayoutFrame( rTab.GetFormat(), &rTab )
    , SwFlowFrame( static_cast<SwFrame&>(*this) )
    , m_pTable( rTab.GetTable() )
    , m_bComplete(false)
    , m_bCalcLowers(false)
    , m_bLowersFormatted(false)
    , m_bLockBackMove(false)
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
    ::ClearFEShellTabCols(*GetFormat()->GetDoc(), this);

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
    OSL_ENSURE( Lower() && Lower()->IsRowFrame(), "No rows." );

    SwPageFrame *pPage = FindPageFrame();
    if ( pPage )
    {
        SwRowFrame *pRow = static_cast<SwRowFrame*>(Lower());
        do
        {
            pRow->RegistFlys( pPage );
            pRow = static_cast<SwRowFrame*>(pRow->GetNext());
        } while ( pRow );
    }
}

void SwInvalidateAll( SwFrame *pFrame, long nBottom );
static bool lcl_RecalcRow( SwRowFrame* pRow, long nBottom );
static bool lcl_ArrangeLowers( SwLayoutFrame *pLay, long lYStart, bool bInva );
// #i26945# - add parameter <_bOnlyRowsAndCells> to control
// that only row and cell frames are formatted.
static bool lcl_InnerCalcLayout( SwFrame *pFrame,
                                      long nBottom,
                                      bool _bOnlyRowsAndCells = false );
// OD 2004-02-18 #106629# - correct type of 1st parameter
// #i26945# - add parameter <_bConsiderObjs> in order to
// control, if floating screen objects have to be considered for the minimal
// cell height.
static SwTwips lcl_CalcMinRowHeight( const SwRowFrame *pRow,
                                     const bool _bConsiderObjs );
static SwTwips lcl_CalcTopAndBottomMargin( const SwLayoutFrame&, const SwBorderAttrs& );

static SwTwips lcl_calcHeightOfRowBeforeThisFrame(const SwRowFrame& rRow);

static SwTwips lcl_GetHeightOfRows( const SwFrame* pStart, long nCount )
{
    if ( !nCount || !pStart)
        return 0;

    SwTwips nRet = 0;
    SwRectFnSet aRectFnSet(pStart);
    while ( pStart && nCount > 0 )
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
            for (SwAnchoredObject* pAnchoredObj : *pLowerFrame->GetDrawObjs())
            {
                // invalidate position of anchored object
                pAnchoredObj->SetTmpConsiderWrapInfluence( false );
                pAnchoredObj->SetConsiderForTextWrap( false );
                pAnchoredObj->UnlockPosition();
                pAnchoredObj->InvalidateObjPos();

                SwFlyFrame *pFly = dynamic_cast<SwFlyFrame*>(pAnchoredObj);

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
                    if ( pAnchoredObj->GetFrameFormat().GetAnchor().GetAnchorId()
                            == RndStdIds::FLY_AS_CHAR )
                    {
                        pAnchoredObj->AnchorFrame()
                                ->Prepare( PREP_FLY_ATTR_CHG,
                                           &(pAnchoredObj->GetFrameFormat()) );
                    }
                    if ( pFly != nullptr )
                    {
                        pFly->GetVirtDrawObj()->SetRectsDirty();
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
        if ( pCurrSourceCell->Lower() && pCurrSourceCell->Lower()->IsRowFrame() )
        {
            SwRowFrame* pTmpSourceRow = static_cast<SwRowFrame*>(pCurrSourceCell->Lower());
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

                    OSL_ENSURE( pTmpDestRow->GetFollowRow() == pTmpSourceRow, "Table contains node" );

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
    if ( !rSource.GetFormat()->GetDoc()->GetFootnoteIdxs().empty() )
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
            long nMinHeight = 0;

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
                    if ( rSz.GetHeightSizeType() == ATT_MIN_SIZE )
                        nMinHeight = rSz.GetHeight() - lcl_calcHeightOfRowBeforeThisFrame(*pTmpLastLineRow);
                }

                SwFrame* pCell = pTmpLastLineRow->Lower();
                while ( pCell )
                {
                    if ( static_cast<SwCellFrame*>(pCell)->Lower() &&
                         static_cast<SwCellFrame*>(pCell)->Lower()->IsRowFrame() )
                    {
                        bTableLayoutTooComplex = true;
                        break;
                    }

                    SwBorderAttrAccess aAccess( SwFrame::GetCache(), pCell );
                    const SwBorderAttrs &rAttrs = *aAccess.Get();
                    nMinHeight = std::max( nMinHeight, lcl_CalcTopAndBottomMargin( *static_cast<SwLayoutFrame*>(pCell), rAttrs ) );
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
        if ( pCurrMasterCell->Lower() &&
             pCurrMasterCell->Lower()->IsRowFrame() )
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
                          SwTwips nRemainingSpaceForLastRow, SwTwips nAlreadyFree )
{
    bool bRet = true;

    vcl::RenderContext* pRenderContext = rLastLine.getRootFrame()->GetCurrShell()->GetOut();
    SwTabFrame& rTab = static_cast<SwTabFrame&>(*rLastLine.GetUpper());
    SwRectFnSet aRectFnSet(rTab.GetUpper());
    SwTwips nCurLastLineHeight = aRectFnSet.GetHeight(rLastLine.getFrameArea());

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
    lcl_RecalcRow( &rLastLine, LONG_MAX );
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
    if ( nDistanceToUpperPrtBottom < 0 || !rTab.DoesObjsFit() )
        bRet = false;

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
        const long nLayoutRowSpan = pCellFrame->GetLayoutRowSpan();
        if ( nLayoutRowSpan > 1 )
        {
            // calculate height of cell:
            const long nNewCellHeight = lcl_GetHeightOfRows( pRow, nLayoutRowSpan );
            const long nDiff = nNewCellHeight - aRectFnSet.GetHeight(pCellFrame->getFrameArea());

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
static long lcl_GetMaximumLayoutRowSpan( const SwRowFrame& rRow )
{
    long nRet = 1;

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

    // We have to reset the flag here, because lcl_MoveRowContent
    // calls a GrowFrame(), which has a different behavior if
    // this flag is set.
    SetFollowFlowLine( false );

    // #140081# Make code robust.
    if ( !pFollowFlowLine || !pLastLine )
        return true;

    // Move content
    lcl_MoveRowContent( *pFollowFlowLine, *static_cast<SwRowFrame*>(pLastLine) );

    // NEW TABLES
    // If a row span follow flow line is removed, we want to move the whole span
    // to the master:
    long nRowsToMove = lcl_GetMaximumLayoutRowSpan( *pFollowFlowLine );

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

bool SwTabFrame::Split( const SwTwips nCutPos, bool bTryToSplit, bool bTableRowKeep )
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

    SwTwips nRemainingSpaceForLastRow =
        aRectFnSet.YDiff(nCutPos, aRectFnSet.GetTop(getFrameArea()));
    nRemainingSpaceForLastRow -= aRectFnSet.GetTopMargin(*this);

    // Make pRow point to the line that does not fit anymore:
    while( pRow->GetNext() &&
           nRemainingSpaceForLastRow >= ( aRectFnSet.GetHeight(pRow->getFrameArea()) +
                                           (IsCollapsingBorders() ?
                                            pRow->GetBottomLineSize() :
                                            0 ) ) )
    {
        if( bTryToSplit || !pRow->IsRowSpanLine() ||
            0 != aRectFnSet.GetHeight(pRow->getFrameArea()) )
            ++nRowCount;
        nRemainingSpaceForLastRow -= aRectFnSet.GetHeight(pRow->getFrameArea());
        pRow = static_cast<SwRowFrame*>(pRow->GetNext());
    }

    // bSplitRowAllowed: Row may be split according to its attributes.
    // bTryToSplit:      Row will never be split if bTryToSplit = false.
    //                   This can either be passed as a parameter, indicating
    //                   that we are currently doing the second try to split the
    //                   table, or it will be set to false under certain
    //                   conditions that are not suitable for splitting
    //                   the row.
    bool bSplitRowAllowed = pRow->IsRowSplitAllowed();

    // #i29438#
    // #i26945# - Floating screen objects no longer forbid
    // a splitting of the table row.
    // Special DoNotSplit case 1:
    // Search for sections inside pRow:
    if ( lcl_FindSectionsInRow( *pRow ) )
    {
        bTryToSplit = false;
    }

    // #i29771#
    // To avoid loops, we do some checks before actually trying to split
    // the row. Maybe we should keep the next row in this table.
    // Note: This is only done if we are at the beginning of our upper
    bool bKeepNextRow = false;
    if ( nRowCount < nRepeat )
    {
        // First case: One of the repeated headline does not fit to the page anymore.
        // At least one more non-heading row has to stay in this table in
        // order to avoid loops:
        OSL_ENSURE( !GetIndPrev(), "Table is supposed to be at beginning" );
        bKeepNextRow = true;
    }
    else if ( !GetIndPrev() && nRepeat == nRowCount )
    {
        // Second case: The first non-headline row does not fit to the page.
        // If it is not allowed to be split, or it contains a sub-row that
        // is not allowed to be split, we keep the row in this table:
        if ( bTryToSplit && bSplitRowAllowed )
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
    bSplitRowAllowed = bSplitRowAllowed && bTryToSplit &&
                       ( !bTableRowKeep ||
                         !pRow->ShouldRowKeepWithNext() );

    // Adjust pRow according to the keep-with-next attribute:
    if ( !bSplitRowAllowed && bTableRowKeep )
    {
        SwRowFrame* pTmpRow = static_cast<SwRowFrame*>(pRow->GetPrev());
        SwRowFrame* pOldRow = pRow;
        while ( pTmpRow && pTmpRow->ShouldRowKeepWithNext() &&
                nRowCount > nRepeat )
        {
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
        for ( nRowCount = 0; nRowCount < nRepeat; ++nRowCount )
        {
            // Insert new headlines:
            bDontCreateObjects = true;              //frmtool
            SwRowFrame* pHeadline = new SwRowFrame(
                                    *GetTable()->GetTabLines()[ nRowCount ], this );
            pHeadline->SetRepeatedHeadline( true );
            bDontCreateObjects = false;
            pHeadline->InsertBefore( pFoll, nullptr );

            SwPageFrame *pPage = pHeadline->FindPageFrame();
            const SwFrameFormats *pTable = GetFormat()->GetDoc()->GetSpzFrameFormats();
            if( !pTable->empty() )
            {
                sal_uLong nIndex;
                SwContentFrame* pFrame = pHeadline->ContainsContent();
                while( pFrame )
                {
                    // sw_redlinehide: the implementation of AppendObjs
                    // takes care of iterating merged SwTextFrame
                    nIndex = pFrame->IsTextFrame()
                        ? static_cast<SwTextFrame*>(pFrame)->GetTextNodeFirst()->GetIndex()
                        : static_cast<SwNoTextFrame*>(pFrame)->GetNode()->GetIndex();
                    AppendObjs( pTable, nIndex, pFrame, pPage, GetFormat()->GetDoc());
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

    SwTwips nShrink = 0;

    //Optimization: There is no paste needed for the new Follow and the
    //optimized insert can be used (big amounts of rows luckily only occurs in
    //such situations).
    if ( bNewFollow )
    {
        SwFrame* pInsertBehind = pFoll->GetLastLower();

        while ( pRow )
        {
            SwFrame* pNxt = pRow->GetNext();
            nShrink += aRectFnSet.GetHeight(pRow->getFrameArea());
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
            nShrink += aRectFnSet.GetHeight(pRow->getFrameArea());

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
        bRet = lcl_RecalcSplitLine( *pLastRow, *pFollowRow, nRemainingSpaceForLastRow, nShrink );

        // NEW TABLES
        // check if each cell in the row span line has a good height
        if ( bRet && pFollowRow->IsRowSpanLine() )
            lcl_AdjustRowSpanCells( pFollowRow );

        // We The RowSplitLine stuff did not work. In this case we conceal the split error:
        if ( !bRet && !bSplitRowAllowed )
        {
            bRet = true;
        }
    }

    return bRet;
}

bool SwTabFrame::Join()
{
    OSL_ENSURE( !HasFollowFlowLine(), "Joining follow flow line" );

    SwTabFrame *pFoll = GetFollow();

    if (pFoll && !pFoll->IsJoinLocked())
    {
        SwRectFnSet aRectFnSet(this);
        pFoll->Cut();   //Cut out first to avoid unnecessary notifications.

        SwFrame *pRow = pFoll->GetFirstNonHeadlineRow(),
              *pNxt;

        SwFrame* pPrv = GetLastLower();

        SwTwips nHeight = 0;    //Total height of the inserted rows as return value.

        while ( pRow )
        {
            pNxt = pRow->GetNext();
            nHeight += aRectFnSet.GetHeight(pRow->getFrameArea());
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
    }

    return true;
}

void SwInvalidatePositions( SwFrame *pFrame, long nBottom )
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
            pFrame->Prepare( PREP_ADJUST_FRM );
        pFrame = pFrame->GetNext();
    } while ( pFrame &&
              ( bAll ||
              aRectFnSet.YDiff( aRectFnSet.GetTop(pFrame->getFrameArea()), nBottom ) < 0 ) );
}

void SwInvalidateAll( SwFrame *pFrame, long nBottom )
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
            SwCellFrame* pThisCell = dynamic_cast<SwCellFrame*>(pFrame);
            if ( pThisCell && pThisCell->GetTabBox()->getRowSpan() < 1 )
            {
                pToInvalidate = & const_cast<SwCellFrame&>(pThisCell->FindStartEndOfRowSpanCell( true ));
                pToInvalidate->InvalidatePos_();
                pToInvalidate->InvalidateSize_();
                pToInvalidate->InvalidatePrt_();
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

bool SwContentFrame::CalcLowers( SwLayoutFrame* pLay, const SwLayoutFrame* pDontLeave,
                                 long nBottom, bool bSkipRowSpanCells )
{
    if ( !pLay )
        return true;

    vcl::RenderContext* pRenderContext = pLay->getRootFrame()->GetCurrShell()->GetOut();
    // LONG_MAX == nBottom means we have to calculate all
    bool bAll = LONG_MAX == nBottom;
    bool bRet = false;
    SwContentFrame *pCnt = pLay->ContainsContent();
    SwRectFnSet aRectFnSet(pLay);

    // FME 2007-08-30 #i81146# new loop control
    int nLoopControlRuns = 0;
    const int nLoopControlMax = 10;
    const SwModify* pLoopControlCond = nullptr;

    while ( pCnt && pDontLeave->IsAnLower( pCnt ) )
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
            // OD 2004-05-11 #i28701# - usage of new method <::FormatObjsAtFrame(..)>
            // to format the floating screen objects
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
                        pCnt = pLay->ContainsContent();
                        continue;
                    }

#if OSL_DEBUG_LEVEL > 1
                    OSL_FAIL( "LoopControl in SwContentFrame::CalcLowers" );
#endif
                }
            }
            pCnt->GetUpper()->Calc(pRenderContext);
        }
        if( ! bAll && aRectFnSet.YDiff(aRectFnSet.GetTop(pCnt->getFrameArea()), nBottom) > 0 )
            break;
        pCnt = pCnt->GetNextContentFrame();
    }
    return bRet;
}

// #i26945# - add parameter <_bOnlyRowsAndCells> to control
// that only row and cell frames are formatted.
static bool lcl_InnerCalcLayout( SwFrame *pFrame,
                                      long nBottom,
                                      bool _bOnlyRowsAndCells )
{
    vcl::RenderContext* pRenderContext = pFrame->getRootFrame()->GetCurrShell() ? pFrame->getRootFrame()->GetCurrShell()->GetOut() : nullptr;
    // LONG_MAX == nBottom means we have to calculate all
    bool bAll = LONG_MAX == nBottom;
    bool bRet = false;
    const SwFrame* pOldUp = pFrame->GetUpper();
    SwRectFnSet aRectFnSet(pFrame);
    do
    {
        // #i26945# - parameter <_bOnlyRowsAndCells> controls,
        // if only row and cell frames are formatted.
        if ( pFrame->IsLayoutFrame() &&
             ( !_bOnlyRowsAndCells || pFrame->IsRowFrame() || pFrame->IsCellFrame() ) )
        {
            // #130744# An invalid locked table frame will
            // not be calculated => It will not become valid =>
            // Loop in lcl_RecalcRow(). Therefore we do not consider them for bRet.
            bRet |= !pFrame->isFrameAreaDefinitionValid() && ( !pFrame->IsTabFrame() || !static_cast<SwTabFrame*>(pFrame)->IsJoinLocked() );
            pFrame->Calc(pRenderContext);
            if( static_cast<SwLayoutFrame*>(pFrame)->Lower() )
                bRet |= lcl_InnerCalcLayout( static_cast<SwLayoutFrame*>(pFrame)->Lower(), nBottom);

            // NEW TABLES
            SwCellFrame* pThisCell = dynamic_cast<SwCellFrame*>(pFrame);
            if ( pThisCell && pThisCell->GetTabBox()->getRowSpan() < 1 )
            {
                SwCellFrame& rToCalc = const_cast<SwCellFrame&>(pThisCell->FindStartEndOfRowSpanCell( true ));
                bRet |= !rToCalc.isFrameAreaDefinitionValid();
                rToCalc.Calc(pRenderContext);
                if ( rToCalc.Lower() )
                    bRet |= lcl_InnerCalcLayout( rToCalc.Lower(), nBottom);
            }
        }
        pFrame = pFrame->GetNext();
    } while( pFrame &&
            ( bAll ||
              aRectFnSet.YDiff(aRectFnSet.GetTop(pFrame->getFrameArea()), nBottom) < 0 )
            && pFrame->GetUpper() == pOldUp );
    return bRet;
}

// returns false if pRow is invalid
static bool lcl_RecalcRow( SwRowFrame* pRow, long nBottom )
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

        while( lcl_InnerCalcLayout( pRow, nBottom ) )
        {
            if ( ++nLoopControlRuns_2 > nLoopControlMax )
            {
                SAL_WARN_IF(nLoopControlStage_2 == 0, "sw.layout", "LoopControl_2 in lcl_RecalcRow: Stage 1!");
                SAL_WARN_IF(nLoopControlStage_2 == 1, "sw.layout", "LoopControl_2 in lcl_RecalcRow: Stage 2!!");
                SAL_WARN_IF(nLoopControlStage_2 >= 2, "sw.layout", "LoopControl_2 in lcl_RecalcRow: Stage 3!!!");
                pRow->ValidateThisAndAllLowers( nLoopControlStage_2++ );
                nLoopControlRuns_2 = 0;
                if( nLoopControlStage_2 > 2 )
                    break;
            }

            bCheck = true;
        }

        if( bCheck )
        {
            // #115759# - force another format of the
            // lowers, if at least one of it was invalid.

            // tdf#114306 writer may crash because pRow points to a deleted SwRowFrame
            SwRowFrame* pOriginalRow = pRow;
            OSL_ENSURE(pOriginalRow->GetUpper() && pOriginalRow->GetUpper()->IsTabFrame(), "No table");
            SwTabFrame* pOriginalTab = static_cast<SwTabFrame*>(pRow->GetUpper());

            bCheck = SwContentFrame::CalcLowers( pRow, pRow->GetUpper(), nBottom, true );

            bool bRowStillExists = false;
            SwFrame* pTestRow = pOriginalTab->Lower();

            while (pTestRow)
            {
                if (pTestRow == pRow)
                {
                    bRowStillExists = true;
                    break;
                }
                pTestRow = pTestRow->GetNext();
            }

            if (!bRowStillExists)
            {
                SAL_WARN("sw.layout", "no row anymore at " << pRow);
                return false;
            }

            // NEW TABLES
            // First we calculate the cells with row span of < 1, afterwards
            // all cells with row span of > 1:
            for ( int i = 0; i < 2; ++i )
            {
                SwCellFrame* pCellFrame = static_cast<SwCellFrame*>(pRow->Lower());
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
                        bCheck  |= SwContentFrame::CalcLowers( &rToRecalc, &rToRecalc, nBottom, false );
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
                    pRow->ValidateThisAndAllLowers( nLoopControlStage_1++ );
                    nLoopControlRuns_1 = 0;
                    if( nLoopControlStage_1 > 2 )
                        break;
                }

                continue;
            }
        }
        break;
    } while (true);
    return true;
}

static void lcl_RecalcTable( SwTabFrame& rTab,
                                  SwLayoutFrame *pFirstRow,
                                  SwLayNotify &rNotify )
{
    if ( rTab.Lower() )
    {
        if ( !pFirstRow )
        {
            pFirstRow = static_cast<SwLayoutFrame*>(rTab.Lower());
            rNotify.SetLowersComplete( true );
        }
        ::SwInvalidatePositions( pFirstRow, LONG_MAX );
        lcl_RecalcRow( static_cast<SwRowFrame*>(pFirstRow), LONG_MAX );
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
            !GetFollow()->IsJoinLocked()
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
    const bool bFootnotesInDoc = !GetFormat()->GetDoc()->GetFootnoteIdxs().empty();
    const bool bFly     = IsInFly();

    auto pAccess = o3tl::make_unique<SwBorderAttrAccess>(SwFrame::GetCache(), this);
    const SwBorderAttrs *pAttrs = pAccess->Get();

    const bool bLargeTable = GetTable()->GetTabLines().size() > 64;  //arbitrary value, virtually guaranteed to be larger than one page.
    const bool bEmulateTableKeep = !bLargeTable && AreAllRowsKeepWithNext( GetFirstNonHeadlineRow(), /*bCheckParents=*/false );
    // The beloved keep attribute
    const bool bKeep = IsKeep(pAttrs->GetAttrSet().GetKeep(), GetBreakItem(), bEmulateTableKeep);

    // All rows should keep together
    const bool bDontSplit = !IsFollow() &&
                            ( !GetFormat()->GetLayoutSplit().GetValue() );

    // The number of repeated headlines
    const sal_uInt16 nRepeat = GetTable()->GetRowsToRepeat();

    // This flag indicates that we are allowed to try to split the
    // table rows.
    bool bTryToSplit = true;

    // Indicates that two individual rows may keep together, based on the keep
    // attribute set at the first paragraph in the first cell.
    const bool bTableRowKeep = !bDontSplit && GetFormat()->GetDoc()->GetDocumentSettingManager().get(DocumentSettingId::TABLE_ROW_KEEP);

    // The Magic Move: Used for the table row keep feature.
    // If only the last row of the table wants to keep (implicitly by setting
    // keep for the first paragraph in the first cell), and this table does
    // not have a next, the last line will be cut. Loop prevention: Only
    // one try.
    // WHAT IS THIS??? It "magically" hides last line (paragraph) in a table,
    // if first is set to keep with next???
    bool bLastRowHasToMoveToFollow = false;
    bool bLastRowMoveNoMoreTries = false;

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
            if ( HasFollowFlowLine() )
                RemoveFollowFlowLine();
            Join();
        }
    }

    // a new one is moved forwards immediately
    if ( !getFrameArea().Top() && IsFollow() )
    {
        SwFrame *pPre = GetPrev();
        if ( pPre && pPre->IsTabFrame() && static_cast<SwTabFrame*>(pPre)->GetFollow() == this)
        {
            if ( !MoveFwd( bMakePage, false ) )
                bMakePage = false;
            bMovedFwd = true;
        }
    }

    int nUnSplitted = 5; // Just another loop control :-(
    int nThrowAwayValidLayoutLimit = 5; // And another one :-(
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

        if ( aOldPos != aRectFnSet.GetPos(getFrameArea()) )
        {
            if ( aOldPos.Y() != aRectFnSet.GetTop(getFrameArea()) )
            {
                SwHTMLTableLayout *pLayout = GetTable()->GetHTMLTableLayout();
                if( pLayout )
                {
                    pAccess.reset();
                    m_bCalcLowers |= pLayout->Resize(
                        pLayout->GetBrowseWidthByTabFrame( *this ) );
                    pAccess = o3tl::make_unique<SwBorderAttrAccess>(SwFrame::GetCache(), this);
                    pAttrs = pAccess->Get();
                }

                setFramePrintAreaValid(false);
                aNotify.SetLowersComplete( false );
            }
            SwFrame *pPre;
            if ( bKeep || (nullptr != (pPre = FindPrev()) &&
                pPre->GetAttrSet()->GetKeep().GetValue()) )
            {
                m_bCalcLowers = true;
            }
        }

        //We need to know the height of the first row, because the master needs
        //to be activated if it shrinks and then absorb the row if necessary.
        long n1StLineHeight = 0;
        if ( IsFollow() )
        {
            SwFrame* pFrame = GetFirstNonHeadlineRow();
            if ( pFrame )
                n1StLineHeight = aRectFnSet.GetHeight(pFrame->getFrameArea());
        }

        if ( !isFrameAreaSizeValid() || !isFramePrintAreaValid() )
        {
            const long nOldPrtWidth = aRectFnSet.GetWidth(getFramePrintArea());
            const long nOldFrameWidth = aRectFnSet.GetWidth(getFrameArea());
            const Point aOldPrtPos  = aRectFnSet.GetPos(getFramePrintArea());
            Format( getRootFrame()->GetCurrShell()->GetOut(), pAttrs );

            SwHTMLTableLayout *pLayout = GetTable()->GetHTMLTableLayout();
            if ( pLayout &&
                (aRectFnSet.GetWidth(getFramePrintArea()) != nOldPrtWidth ||
                aRectFnSet.GetWidth(getFrameArea()) != nOldFrameWidth) )
            {
                pAccess.reset();
                m_bCalcLowers |= pLayout->Resize(
                    pLayout->GetBrowseWidthByTabFrame( *this ) );
                pAccess = o3tl::make_unique<SwBorderAttrAccess>(SwFrame::GetCache(), this);
                pAttrs = pAccess->Get();
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
                    if ( ShouldBwdMoved( pMaster->GetUpper(), false, bDummy ) )
                        pMaster->InvalidatePos();
                }
            }
            SwFootnoteBossFrame *pOldBoss = bFootnotesInDoc ? FindFootnoteBossFrame( true ) : nullptr;
            bool bReformat;
            if ( MoveBwd( bReformat ) )
            {
                aRectFnSet.Refresh(this);
                bMovedBwd = true;
                aNotify.SetLowersComplete( false );
                if ( bFootnotesInDoc )
                    MoveLowerFootnotes( nullptr, pOldBoss, nullptr, true );
                if ( bReformat || bKeep )
                {
                    long nOldTop = aRectFnSet.GetTop(getFrameArea());
                    MakePos();
                    if( nOldTop != aRectFnSet.GetTop(getFrameArea()) )
                    {
                        SwHTMLTableLayout *pHTMLLayout =
                            GetTable()->GetHTMLTableLayout();
                        if( pHTMLLayout )
                        {
                            pAccess.reset();
                            m_bCalcLowers |= pHTMLLayout->Resize(
                                pHTMLLayout->GetBrowseWidthByTabFrame( *this ) );

                            pAccess = o3tl::make_unique<SwBorderAttrAccess>(SwFrame::GetCache(), this);
                            pAttrs = pAccess->Get();
                        }

                        setFramePrintAreaValid(false);
                        Format( getRootFrame()->GetCurrShell()->GetOut(), pAttrs );
                    }
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
                if ( GetFollow()->ShouldBwdMoved( GetUpper(), false, bDummy ) )
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
                                lcl_RecalcRow( static_cast<SwRowFrame*>(pLastLine), LONG_MAX );
                                SetRebuildLastLine( false );
                            }

                            SwFrame* pRow = GetFollow()->GetFirstNonHeadlineRow();

                            if ( !pRow || !pRow->GetNext() )
                                //The follow becomes empty and invalid for this reason.
                                Join();

                            continue;
                        }

                        // If there is no follow flow line, we move the first
                        // row in the follow table to the master table.
                        SwRowFrame *pRow = GetFollow()->GetFirstNonHeadlineRow();

                        //The follow becomes empty and invalid for this reason.
                        if ( !pRow )
                        {
                            Join();
                            continue;
                        }

                        const SwTwips nOld = aRectFnSet.GetHeight(getFrameArea());
                        long nRowsToMove = lcl_GetMaximumLayoutRowSpan( *pRow );
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
                                //The follow becomes empty and invalid for this reason.
                                Join();
                            }
                            else
                            {
                                pRowToMove->Cut();
                                pRowToMove->Paste( this );
                            }

                            //Displace the footnotes!
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
                    if (pLastRow
                        && IsKeep(pAttrs->GetAttrSet().GetKeep(), GetBreakItem(), true)
                        && pLastRow->ShouldRowKeepWithNext())
                    {
                        bFormat = true;
                    }
                }

                if ( bFormat )
                {
                    pAccess.reset();

                    // Consider case that table is inside another table, because
                    // it has to be avoided, that superior table is formatted.
                    // Thus, find next content, table or section and, if a section
                    // is found, get its first content.
                    const SwFrame* pTmpNxt = sw_FormatNextContentForKeep( this );

                    pAccess = o3tl::make_unique<SwBorderAttrAccess>(SwFrame::GetCache(), this);
                    pAttrs = pAccess->Get();

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
                    lcl_RecalcRow( static_cast<SwRowFrame*>(Lower()), LONG_MAX );
                    m_bONECalcLowers = false;
                }
            }
            continue;
        }

        //I don't fit in the higher-ranked element anymore, therefore it's the
        //right moment to do some preferably constructive changes.

        //If I'm NOT allowed to leave the parent Frame, I've got a problem.
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
                lcl_RecalcRow( static_cast<SwRowFrame*>(Lower()), LONG_MAX );
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
        // 3. The table is allowed to split or we do not have an pIndPrev:
        SwFrame* pIndPrev = GetIndPrev();
        const SwRowFrame* pFirstNonHeadlineRow = GetFirstNonHeadlineRow();
        // #i120016# if this row wants to keep, allow split in case that all rows want to keep with next,
        // the table can not move forward as it is the first one and a split is in general allowed.
        const bool bAllowSplitOfRow = ( bTableRowKeep &&
                                        AreAllRowsKeepWithNext( pFirstNonHeadlineRow ) ) &&
                                      !pIndPrev &&
                                      !bDontSplit;
        const bool bEmulateTableKeepFwdMoveAllowed = IsKeepFwdMoveAllowed(bEmulateTableKeep);

        if ( pFirstNonHeadlineRow && nUnSplitted > 0 &&
             ( !bEmulateTableKeepFwdMoveAllowed ||
               ( ( !bTableRowKeep || pFirstNonHeadlineRow->GetNext() ||
                   !pFirstNonHeadlineRow->ShouldRowKeepWithNext() || bAllowSplitOfRow
                 ) && ( !bDontSplit || !pIndPrev )
           ) ) )
        {
            // #i29438#
            // Special DoNotSplit cases:
            // We better avoid splitting if the table keeps with next paragraph and can move fwd still.
            // We better avoid splitting of a row frame if we are inside a columned
            // section which has a height of 0, because this is not growable and thus
            // all kinds of unexpected things could happen.
            if ( !bEmulateTableKeepFwdMoveAllowed ||
                 ( IsInSct() && FindSctFrame()->Lower()->IsColumnFrame() &&
                   0 == aRectFnSet.GetHeight(GetUpper()->getFrameArea())
               ) )
            {
                bTryToSplit = false;
            }

            // 1. Try: bTryToSplit = true  => Try to split the row.
            // 2. Try: bTryToSplit = false => Split the table between the rows.
            if ((pFirstNonHeadlineRow && pFirstNonHeadlineRow->GetNext()) || bTryToSplit )
            {
                SwTwips nDeadLine = aRectFnSet.GetPrtBottom(*GetUpper());
                if( IsInSct() || GetUpper()->IsInTab() ) // TABLE IN TABLE)
                    nDeadLine = aRectFnSet.YInc( nDeadLine,
                                        GetUpper()->Grow( LONG_MAX, true ) );

                {
                    SetInRecalcLowerRow( true );
                    SwRowFrame* pRow = static_cast<SwRowFrame*>(Lower());
                    if (!lcl_RecalcRow(pRow, nDeadLine))
                    {
                        pFirstNonHeadlineRow = GetFirstNonHeadlineRow();
                    }
                    SetInRecalcLowerRow( false );
                }
                m_bLowersFormatted = true;
                aNotify.SetLowersComplete( true );

                // One more check if its really necessary to split the table.
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
                    const SwRowFrame* pTmpRow = pFirstNonHeadlineRow;
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

                // Some more checks if we want to call the split algorithm or not:
                // The repeating lines / keeping lines still fit into the upper or
                // if we do not have an (in)direct Prev, we split anyway.
                if( aRectFnSet.YDiff(nDeadLine, nBreakLine) >=0
                    || !pIndPrev || !bEmulateTableKeepFwdMoveAllowed )
                {
                    aNotify.SetLowersComplete( false );
                    bSplit = true;

                    // An existing follow flow line has to be removed.
                    if ( HasFollowFlowLine() )
                    {
                        if (!nThrowAwayValidLayoutLimit)
                            continue;
                        const bool bInitialLoopEndCondition(isFrameAreaDefinitionValid());
                        RemoveFollowFlowLine();
                        const bool bFinalLoopEndCondition(isFrameAreaDefinitionValid());

                        if (bInitialLoopEndCondition && !bFinalLoopEndCondition)
                        {
                            --nThrowAwayValidLayoutLimit;
                        }
                    }

                    const bool bSplitError = !Split( nDeadLine, bTryToSplit, ( bTableRowKeep && !(bAllowSplitOfRow || !bEmulateTableKeepFwdMoveAllowed) ) );
                    if( !bTryToSplit && !bSplitError && nUnSplitted > 0 )
                    {
                        --nUnSplitted;
                    }

                    // #i29771# Two tries to split the table
                    // If an error occurred during splitting. We start a second
                    // try, this time without splitting of table rows.
                    if ( bSplitError )
                    {
                        if ( HasFollowFlowLine() )
                            RemoveFollowFlowLine();
                    }

                    // If splitting the table was successful or not,
                    // we do not want to have 'empty' follow tables.
                    if ( GetFollow() && !GetFollow()->GetFirstNonHeadlineRow() )
                        Join();

                    // We want to restore the situation before the failed
                    // split operation as good as possible. Therefore we
                    // do some more calculations. Note: Restricting this
                    // to nDeadLine may not be enough.
                    if ( bSplitError && bTryToSplit ) // no restart if we did not try to split: i72847, i79426
                    {
                        lcl_RecalcRow( static_cast<SwRowFrame*>(Lower()), LONG_MAX );
                        setFrameAreaPositionValid(false);
                        bTryToSplit = false;
                        continue;
                    }

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
                            pAccess.reset();

                            GetFollow()->MakeAll(pRenderContext);

                            pAccess = o3tl::make_unique<SwBorderAttrAccess>(SwFrame::GetCache(), this);
                            pAttrs = pAccess->Get();

                            GetFollow()->SetLowersFormatted(false);
                            // #i43913# - lock follow table
                            // to avoid its formatting during the format of
                            // its content.
                            const bool bOldJoinLock =  GetFollow()->IsJoinLocked();
                            GetFollow()->LockJoin();
                            ::lcl_RecalcRow( static_cast<SwRowFrame*>(GetFollow()->Lower()),
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
                                    if ( bCalcNxt )
                                    {
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
        if ( !bMovedFwd && !MoveFwd( bMakePage, false ) )
            bMakePage = false;

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
            //To avoid oscillations now invalid master should drop behind.
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

        if ( bMovedBwd && GetUpper() )
        {
            //During floating back the upper was animated to do a full repaint,
            //we can now skip this after the whole back and forth floating.
            GetUpper()->ResetCompletePaint();
        }

        if (m_bCalcLowers && isFrameAreaDefinitionValid())
        {
            // #i44910# - format of lower frames unnecessary
            // and can cause layout loops, if table doesn't fit and isn't
            // allowed to split.
            SwTwips nDistToUpperPrtBottom =
                aRectFnSet.BottomDist( getFrameArea(), aRectFnSet.GetPrtBottom(*GetUpper()));
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
    pAccess.reset();
    UnlockJoin();
    if ( bMovedFwd || bMovedBwd || !bOldValidPos )
        aNotify.SetInvaKeep();
}

/// Calculate the offsets arising because of FlyFrames
bool SwTabFrame::CalcFlyOffsets( SwTwips& rUpper,
                               long& rLeftOffset,
                               long& rRightOffset ) const
{
    bool bInvalidatePrtArea = false;
    const SwPageFrame *pPage = FindPageFrame();
    const SwFlyFrame* pMyFly = FindFlyFrame();

    // --> #108724# Page header/footer content doesn't have to wrap around
    //              floating screen objects

    const IDocumentSettingAccess& rIDSA = GetFormat()->getIDocumentSettingAccess();
    const bool bWrapAllowed = rIDSA.get(DocumentSettingId::USE_FORMER_TEXT_WRAPPING) ||
                                ( !IsInFootnote() && nullptr == FindFooterOrHeader() );

    if ( pPage->GetSortedObjs() && bWrapAllowed )
    {
        SwRectFnSet aRectFnSet(this);
        const bool bConsiderWrapOnObjPos = rIDSA.get(DocumentSettingId::CONSIDER_WRAP_ON_OBJECT_POSITION);
        long nPrtPos = aRectFnSet.GetTop(getFrameArea());
        nPrtPos = aRectFnSet.YInc( nPrtPos, rUpper );
        SwRect aRect( getFrameArea() );
        long nYDiff = aRectFnSet.YDiff( aRectFnSet.GetTop(getFramePrintArea()), rUpper );
        if( nYDiff > 0 )
            aRectFnSet.AddBottom( aRect, -nYDiff );
        for ( size_t i = 0; i < pPage->GetSortedObjs()->size(); ++i )
        {
            SwAnchoredObject* pAnchoredObj = (*pPage->GetSortedObjs())[i];
            if ( dynamic_cast< const SwFlyFrame *>( pAnchoredObj ) !=  nullptr )
            {
                SwFlyFrame *pFly = static_cast<SwFlyFrame*>(pAnchoredObj);
                const SwRect aFlyRect = pFly->GetObjRectWithSpaces();
                // #i26945# - correction of conditions,
                // if Writer fly frame has to be considered:
                // - no need to check, if top of Writer fly frame differs
                //   from FAR_AWAY, because it's also checked, if the Writer
                //   fly frame rectangle overlaps with <aRect>
                // - no check, if bottom of anchor frame is prior the top of
                //   the table, because Writer fly frames can be negative positioned.
                // - correct check, if the Writer fly frame is an lower of the
                //   table, because table lines/rows can split and a at-character
                //   anchored Writer fly frame could be positioned in the follow
                //   flow line.
                // - add condition, that an existing anchor character text frame
                //   has to be on the same page as the table.
                //   E.g., it could happen, that the fly frame is still registered
                //   at the page frame, the table is on, but it's anchor character
                //   text frame has already changed its page.
                const SwTextFrame* pAnchorCharFrame = pFly->FindAnchorCharFrame();
                bool bConsiderFly =
                    // #i46807# - do not consider invalid
                    // Writer fly frames.
                    pFly->isFrameAreaDefinitionValid() &&
                    // fly anchored at character
                    pFly->IsFlyAtContentFrame() &&
                    // fly overlaps with corresponding table rectangle
                    aFlyRect.IsOver( aRect ) &&
                    // fly isn't lower of table and
                    // anchor character frame of fly isn't lower of table
                    ( !IsAnLower( pFly ) &&
                      ( !pAnchorCharFrame || !IsAnLower( pAnchorCharFrame ) ) ) &&
                    // table isn't lower of fly
                    !pFly->IsAnLower( this ) &&
                    // fly is lower of fly, the table is in
                    // #123274# - correction
                    // assure that fly isn't a lower of a fly, the table isn't in.
                    // E.g., a table in the body doesn't wrap around a graphic,
                    // which is inside a frame.
                    ( ( !pMyFly ||
                        pMyFly->IsAnLower( pFly ) ) &&
                      pMyFly == pFly->GetAnchorFrameContainingAnchPos()->FindFlyFrame() ) &&
                    // anchor frame not on following page
                    pPage->GetPhyPageNum() >=
                      pFly->GetAnchorFrame()->FindPageFrame()->GetPhyPageNum() &&
                    // anchor character text frame on same page
                    ( !pAnchorCharFrame ||
                      pAnchorCharFrame->FindPageFrame()->GetPhyPageNum() ==
                        pPage->GetPhyPageNum() );

                if ( bConsiderFly )
                {
                    const SwFrame* pFlyHeaderFooterFrame = pFly->GetAnchorFrame()->FindFooterOrHeader();
                    const SwFrame* pThisHeaderFooterFrame = FindFooterOrHeader();

                    if ( pFlyHeaderFooterFrame != pThisHeaderFooterFrame &&
                        // #148493# If bConsiderWrapOnObjPos is set,
                        // we want to consider the fly if it is located in the header and
                        // the table is located in the body:
                         ( !bConsiderWrapOnObjPos || nullptr != pThisHeaderFooterFrame || !pFlyHeaderFooterFrame->IsHeaderFrame() ) )
                        bConsiderFly = false;
                }

                if ( bConsiderFly )
                {
                    const SwFormatSurround   &rSur = pFly->GetFormat()->GetSurround();
                    const SwFormatHoriOrient &rHori= pFly->GetFormat()->GetHoriOrient();
                    if ( css::text::WrapTextMode_NONE == rSur.GetSurround() )
                    {
                        long nBottom = aRectFnSet.GetBottom(aFlyRect);
                        if( aRectFnSet.YDiff( nPrtPos, nBottom ) < 0 )
                            nPrtPos = nBottom;
                        bInvalidatePrtArea = true;
                    }
                    if ( (css::text::WrapTextMode_RIGHT    == rSur.GetSurround() ||
                          css::text::WrapTextMode_PARALLEL == rSur.GetSurround())&&
                         text::HoriOrientation::LEFT == rHori.GetHoriOrient() )
                    {
                        const long nWidth = aRectFnSet.XDiff(
                            aRectFnSet.GetRight(aFlyRect),
                            aRectFnSet.GetLeft(pFly->GetAnchorFrame()->getFrameArea()) );
                        rLeftOffset = std::max( rLeftOffset, nWidth );
                        bInvalidatePrtArea = true;
                    }
                    if ( (css::text::WrapTextMode_LEFT     == rSur.GetSurround() ||
                          css::text::WrapTextMode_PARALLEL == rSur.GetSurround())&&
                         text::HoriOrientation::RIGHT == rHori.GetHoriOrient() )
                    {
                        const long nWidth = aRectFnSet.XDiff(
                            aRectFnSet.GetRight(pFly->GetAnchorFrame()->getFrameArea()),
                            aRectFnSet.GetLeft(aFlyRect) );
                        rRightOffset = std::max( rRightOffset, nWidth );
                        bInvalidatePrtArea = true;
                    }
                }
            }
        }
        rUpper = aRectFnSet.YDiff( nPrtPos, aRectFnSet.GetTop(getFrameArea()) );
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
        long nDiff = aRectFnSet.GetWidth(GetUpper()->getFramePrintArea()) -
                     aRectFnSet.GetWidth(getFrameArea());
        if( nDiff )
        {
            SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*this);
            aRectFnSet.AddRight( aFrm, nDiff );
        }
    }

    //VarSize is always the height.
    //For the upper/lower border the same rules apply as for cntfrms (see
    //MakePrtArea() of those).

    SwTwips nUpper = CalcUpperSpace( pAttrs );

    //We want to dodge the border. Two possibilities:
    //1. There are borders with SurroundNone, dodge them completely
    //2. There are borders which only float on the right or the left side and
    //   are right or left aligned, those set the minimum for the borders.
    long nTmpRight = -1000000,
         nLeftOffset  = 0;
    if( CalcFlyOffsets( nUpper, nLeftOffset, nTmpRight ) )
    {
        setFramePrintAreaValid(false);
    }

    long nRightOffset = std::max( 0L, nTmpRight );

    SwTwips nLower = pAttrs->CalcBottomLine();
    // #i29550#
    if ( IsCollapsingBorders() )
        nLower += GetBottomLineSize();

    if ( !isFramePrintAreaValid() )
    {
        setFramePrintAreaValid(true);

        //The width of the PrtArea is given by the FrameFormat, the borders have to
        //be set accordingly.
        //Minimum borders are determined depending on margins and shadows.
        //The borders are adjusted so that the PrtArea is aligned into the Frame
        //according to the adjustment.
        //If the adjustment is 0, the borders are set according to the border
        //attributes.

        const SwTwips nOldHeight = aRectFnSet.GetHeight(getFramePrintArea());
        const SwTwips nMax = aRectFnSet.GetWidth(getFrameArea());

        // OD 14.03.2003 #i9040# - adjust variable names.
        const SwTwips nLeftLine  = pAttrs->CalcLeftLine();
        const SwTwips nRightLine = pAttrs->CalcRightLine();

        //The width possibly is a percentage value. If the table is inside
        //something else, the value applies to the surrounding. If it's the body
        //the value applies to the screen width in the BrowseView.
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
                        // -> right indent is maximun of given right offset
                        //    and wished right offset.
                        nRightSpacing = nRightLine + std::max( nRightOffset, nWishRight );
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
                                            std::max( 0L, nWishRight ) );
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
                        // -> right indent is maximun of given left offset
                        //    and wished left offset.
                        nLeftSpacing = nLeftLine + std::max( nLeftOffset, nWishLeft );
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
                                           std::max( 0L, nWishLeft ) );
                    }
                }
                break;
            case text::HoriOrientation::CENTER:
                {
                    // OD 07.03.2003 #i9040# - consider left/right line attribute.
                    // OD 10.03.2003 #i9040# -
                    const SwTwips nCenterSpacing = ( nMax - nWishedTableWidth ) / 2;
                    nLeftSpacing = nLeftLine +
                                   ( (nLeftOffset > 0) ?
                                     std::max( nCenterSpacing, nLeftOffset ) :
                                     nCenterSpacing );
                    nRightSpacing = nRightLine +
                                    ( (nRightOffset > 0) ?
                                      std::max( nCenterSpacing, nRightOffset ) :
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
                    //The border are defined by the border attribute.
                    nLeftSpacing = pAttrs->CalcLeft( this );
                    if( nLeftOffset )
                    {
                        // OD 07.03.2003 #i9040# - surround fly frames only, if
                        // they overlap with the table.
                        // Thus, take maximun of left spacing and left offset.
                        // OD 10.03.2003 #i9040# - consider left line attribute.
                        nLeftSpacing = std::max( nLeftSpacing, ( nLeftOffset + nLeftLine ) );
                    }
                    // OD 23.01.2003 #106895# - add 1st param to <SwBorderAttrs::CalcRight(..)>
                    nRightSpacing = pAttrs->CalcRight( this );
                    if( nRightOffset )
                    {
                        // OD 07.03.2003 #i9040# - surround fly frames only, if
                        // they overlap with the table.
                        // Thus, take maximun of right spacing and right offset.
                        // OD 10.03.2003 #i9040# - consider right line attribute.
                        nRightSpacing = std::max( nRightSpacing, ( nRightOffset + nRightLine ) );
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
                        // Thus, take maximun of right spacing and right offset.
                        // OD 10.03.2003 #i9040# - consider left line attribute.
                        nLeftSpacing = std::max( nLeftSpacing, ( pAttrs->CalcLeftLine() + nLeftOffset ) );
                    }
                    // OD 10.03.2003 #i9040# - consider right and left line attribute.
                    const SwTwips nWishRight =
                            nMax - (nLeftSpacing-pAttrs->CalcLeftLine()) - nWishedTableWidth;
                    nRightSpacing = nRightLine +
                                    ( (nRightOffset > 0) ?
                                      std::max( nWishRight, nRightOffset ) :
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
            //Don't overlap the edge of the visible area.
            //The page width can be bigger because objects with
            //"over-size" are possible (RootFrame::ImplCalcBrowseWidth())
            long nWidth = pSh->GetBrowseWidth();
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

    if ( !isFrameAreaSizeValid() )
    {
        setFrameAreaSizeValid(true);

        //The size is defined by the content plus the borders.
        SwTwips nRemaining = 0, nDiff;
        SwFrame *pFrame = m_pLower;
        while ( pFrame )
        {
            nRemaining += aRectFnSet.GetHeight(pFrame->getFrameArea());
            pFrame = pFrame->GetNext();
        }
        //And now add the borders
        nRemaining += nUpper + nLower;

        nDiff = aRectFnSet.GetHeight(getFrameArea()) - nRemaining;
        if ( nDiff > 0 )
            Shrink( nDiff );
        else if ( nDiff < 0 )
            Grow( -nDiff );
    }
}

SwTwips SwTabFrame::GrowFrame( SwTwips nDist, bool bTst, bool bInfo )
{
    SwRectFnSet aRectFnSet(this);
    SwTwips nHeight = aRectFnSet.GetHeight(getFrameArea());
    if( nHeight > 0 && nDist > ( LONG_MAX - nHeight ) )
        nDist = LONG_MAX - nHeight;

    if ( bTst && !IsRestrictTableGrowth() )
        return nDist;

    if ( GetUpper() )
    {
        SwRect aOldFrame( getFrameArea() );

        //The upper only grows as far as needed. nReal provides the distance
        //which is already available.
        SwTwips nReal = aRectFnSet.GetHeight(GetUpper()->getFramePrintArea());
        SwFrame *pFrame = GetUpper()->Lower();
        while ( pFrame && GetFollow() != pFrame )
        {
            nReal -= aRectFnSet.GetHeight(pFrame->getFrameArea());
            pFrame = pFrame->GetNext();
        }

        if ( nReal < nDist )
        {
            long nTmp = GetUpper()->Grow( nDist - std::max<long>(nReal, 0), bTst, bInfo );

            if ( IsRestrictTableGrowth() )
            {
                nTmp = std::min( nDist, nReal + nTmp );
                nDist = nTmp < 0 ? 0 : nTmp;
            }
        }

        if ( !bTst )
        {
            {
                SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*this);
                aRectFnSet.AddBottom( aFrm, nDist );
            }

            SwRootFrame *pRootFrame = getRootFrame();
            if( pRootFrame && pRootFrame->IsAnyShellAccessible() &&
                pRootFrame->GetCurrShell() )
            {
                pRootFrame->GetCurrShell()->Imp()->MoveAccessibleFrame( this, aOldFrame );
            }
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

        SvxBrushItem aBack = GetFormat()->makeBackgroundBrushItem();
        const SvxGraphicPosition ePos = aBack.GetGraphicPos();
        if ( GPOS_NONE != ePos && GPOS_TILED != ePos )
            SetCompletePaint();
    }

    return nDist;
}

void SwTabFrame::Modify( const SfxPoolItem* pOld, const SfxPoolItem * pNew )
{
    sal_uInt8 nInvFlags = 0;
    bool bAttrSetChg = pNew && RES_ATTRSET_CHG == pNew->Which();

    if( bAttrSetChg )
    {
        SfxItemIter aNIter( *static_cast<const SwAttrSetChg*>(pNew)->GetChgSet() );
        SfxItemIter aOIter( *static_cast<const SwAttrSetChg*>(pOld)->GetChgSet() );
        SwAttrSetChg aOldSet( *static_cast<const SwAttrSetChg*>(pOld) );
        SwAttrSetChg aNewSet( *static_cast<const SwAttrSetChg*>(pNew) );
        while( true )
        {
            UpdateAttr_( aOIter.GetCurItem(),
                         aNIter.GetCurItem(), nInvFlags,
                         &aOldSet, &aNewSet );
            if( aNIter.IsAtEnd() )
                break;
            aNIter.NextItem();
            aOIter.NextItem();
        }
        if ( aOldSet.Count() || aNewSet.Count() )
            SwLayoutFrame::Modify( &aOldSet, &aNewSet );
    }
    else
        UpdateAttr_( pOld, pNew, nInvFlags );

    if ( nInvFlags != 0 )
    {
        SwPageFrame *pPage = FindPageFrame();
        InvalidatePage( pPage );
        if ( nInvFlags & 0x02 )
            InvalidatePrt_();
        if ( nInvFlags & 0x40 )
            InvalidatePos_();
        SwFrame *pTmp;
        if ( nullptr != (pTmp = GetIndNext()) )
        {
            if ( nInvFlags & 0x04 )
            {
                pTmp->InvalidatePrt_();
                if ( pTmp->IsContentFrame() )
                    pTmp->InvalidatePage( pPage );
            }
            if ( nInvFlags & 0x10 )
                pTmp->SetCompletePaint();
        }
        if ( nInvFlags & 0x08 && nullptr != (pTmp = GetPrev()) )
        {
            pTmp->InvalidatePrt_();
            if ( pTmp->IsContentFrame() )
                pTmp->InvalidatePage( pPage );
        }
        if ( nInvFlags & 0x20  )
        {
            if ( pPage && pPage->GetUpper() && !IsFollow() )
                static_cast<SwRootFrame*>(pPage->GetUpper())->InvalidateBrowseWidth();
        }
        if ( nInvFlags & 0x80 )
            InvalidateNextPos();
    }
}

void SwTabFrame::UpdateAttr_( const SfxPoolItem *pOld, const SfxPoolItem *pNew,
                            sal_uInt8 &rInvFlags,
                            SwAttrSetChg *pOldSet, SwAttrSetChg *pNewSet )
{
    bool bClear = true;
    const sal_uInt16 nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;
    switch( nWhich )
    {
        case RES_TBLHEADLINECHG:
            if ( IsFollow() )
            {
                // Delete remaining headlines:
                SwRowFrame* pLowerRow = nullptr;
                while ( nullptr != ( pLowerRow = static_cast<SwRowFrame*>(Lower()) ) && pLowerRow->IsRepeatedHeadline() )
                {
                    pLowerRow->Cut();
                    SwFrame::DestroyFrame(pLowerRow);
                }

                // insert new headlines
                const sal_uInt16 nNewRepeat = GetTable()->GetRowsToRepeat();
                for ( sal_uInt16 nIdx = 0; nIdx < nNewRepeat; ++nIdx )
                {
                    bDontCreateObjects = true;          //frmtool
                    SwRowFrame* pHeadline = new SwRowFrame( *GetTable()->GetTabLines()[ nIdx ], this );
                    pHeadline->SetRepeatedHeadline( true );
                    bDontCreateObjects = false;
                    pHeadline->Paste( this, pLowerRow );
                }
            }
            rInvFlags |= 0x02;
            break;

        case RES_FRM_SIZE:
        case RES_HORI_ORIENT:
            rInvFlags |= 0x22;
            break;

        case RES_PAGEDESC:                      //Attribute changes (on/off)
            if ( IsInDocBody() )
            {
                rInvFlags |= 0x40;
                SwPageFrame *pPage = FindPageFrame();
                if (pPage)
                {
                    if ( !GetPrev() )
                        CheckPageDescs( pPage );
                    if (GetFormat()->GetPageDesc().GetNumOffset())
                        static_cast<SwRootFrame*>(pPage->GetUpper())->SetVirtPageNum( true );
                    SwDocPosUpdate aMsgHint( pPage->getFrameArea().Top() );
                    GetFormat()->GetDoc()->getIDocumentFieldsAccess().UpdatePageFields( &aMsgHint );
                }
            }
            break;

        case RES_BREAK:
            rInvFlags |= 0xC0;
            break;

        case RES_LAYOUT_SPLIT:
            if ( !IsFollow() )
                rInvFlags |= 0x40;
            break;
        case RES_FRAMEDIR :
            SetDerivedR2L( false );
            CheckDirChange();
            break;
        case RES_COLLAPSING_BORDERS :
            rInvFlags |= 0x02;
            lcl_InvalidateAllLowersPrt( this );
            break;
        case RES_UL_SPACE:
            rInvFlags |= 0x1C;
            SAL_FALLTHROUGH;

        default:
            bClear = false;
    }
    if ( bClear )
    {
        if ( pOldSet || pNewSet )
        {
            if ( pOldSet )
                pOldSet->ClearItem( nWhich );
            if ( pNewSet )
                pNewSet->ClearItem( nWhich );
        }
        else
            SwLayoutFrame::Modify( pOld, pNew );
    }
}

bool SwTabFrame::GetInfo( SfxPoolItem &rHint ) const
{
    if ( RES_VIRTPAGENUM_INFO == rHint.Which() && IsInDocBody() && !IsFollow() )
    {
        SwVirtPageNumInfo &rInfo = static_cast<SwVirtPageNumInfo&>(rHint);
        const SwPageFrame *pPage = FindPageFrame();
        if ( pPage  )
        {
            if ( pPage == rInfo.GetOrigPage() && !GetPrev() )
            {
                //Here it should be (can temporary be different, should we be
                //                    concerned about this?)
                rInfo.SetInfo( pPage, this );
                return false;
            }
            if ( pPage->GetPhyPageNum() < rInfo.GetOrigPage()->GetPhyPageNum() &&
                 (!rInfo.GetPage() || pPage->GetPhyPageNum() > rInfo.GetPage()->GetPhyPageNum()))
            {
                //This could be the one.
                rInfo.SetInfo( pPage, this );
            }
        }
    }
    return true;
}

SwContentFrame *SwTabFrame::FindLastContent()
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
            // an empty last column at the end of the last line - this is done
            // by SwSectionFrame::FindLastContent
            if( pRet->IsColBodyFrame() )
            {
#if OSL_DEBUG_LEVEL > 0
                SwSectionFrame* pSect = pRet->FindSctFrame();
                OSL_ENSURE( pSect, "Where does this column come from?");
                OSL_ENSURE( IsAnLower( pSect ), "Splitted cell?" );
#endif
                return pRet->FindSctFrame()->FindLastContent();
            }

            // pRet may be a cell frame without a lower (cell has been split).
            // We have to find the last content the hard way:

            OSL_ENSURE( pRet->IsCellFrame(), "SwTabFrame::FindLastContent failed" );
            const SwFrame* pRow = pRet->GetUpper();
            while ( pRow && !pRow->GetUpper()->IsTabFrame() )
                pRow = pRow->GetUpper();
            const SwContentFrame* pContentFrame = pRow ? static_cast<const SwLayoutFrame*>(pRow)->ContainsContent() : nullptr;
            pRet = nullptr;

            while ( pContentFrame && static_cast<const SwLayoutFrame*>(pRow)->IsAnLower( pContentFrame ) )
            {
                pRet = const_cast<SwContentFrame*>(pContentFrame);
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

        if( pRet->IsSctFrame() )
            pRet = static_cast<SwSectionFrame*>(pRet)->FindLastContent();
    }

    return static_cast<SwContentFrame*>(pRet);
}

/// Return value defines if the frm needs to be relocated
bool SwTabFrame::ShouldBwdMoved( SwLayoutFrame *pNewUpper, bool, bool &rReformat )
{
    rReformat = false;
    if ( SwFlowFrame::IsMoveBwdJump() || !IsPrevObjMove() )
    {
        //Floating back Frame's is quite time consuming unfortunately.
        //Most often the location where the Frame wants to float to has the same
        //FixSize as the Frame itself. In such a situation it's easy to check if
        //the Frame will find enough space for its VarSize, if this is not the
        //case, the relocation can be skipped.
        //Checking if the Frame will find enough space is done by the Frame itself,
        //this also takes the possibility of splitting the Frame into account.
        //If the FixSize is different or Flys are involved  (at the old or the
        //new position) the whole checks don't make sense at all, the Frame then
        //needs to be relocated tentatively (if a bit of space is available).

        //The FixSize of the surrounding which contain tables is always the
        //width.

        SwPageFrame *pOldPage = FindPageFrame(),
                  *pNewPage = pNewUpper->FindPageFrame();
        bool bMoveAnyway = false;
        SwTwips nSpace = 0;

        SwRectFnSet aRectFnSet(this);
        if ( !SwFlowFrame::IsMoveBwdJump() )
        {

            long nOldWidth = aRectFnSet.GetWidth(GetUpper()->getFramePrintArea());
            SwRectFnSet fnRectX(pNewUpper);
            long nNewWidth = fnRectX.GetWidth(pNewUpper->getFramePrintArea());
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
                }
            }
            else if (!m_bLockBackMove)
                bMoveAnyway = true;
        }
        else if (!m_bLockBackMove)
            bMoveAnyway = true;

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
        if (!m_bLockBackMove && bFits)
        {
            // #i26945# - check, if follow flow line
            // contains frame, which are moved forward due to its object
            // positioning.
            SwRowFrame* pFirstRow = GetFirstNonHeadlineRow();
            if ( pFirstRow && pFirstRow->IsInFollowFlowRow() &&
                 SwLayouter::DoesRowContainMovedFwdFrame(
                                            *(pFirstRow->GetFormat()->GetDoc()),
                                            *pFirstRow ) )
            {
                return false;
            }
            SwTwips nTmpHeight = CalcHeightOfFirstContentLine();

            // For some mysterious reason, I changed the good old
            // 'return nHeight <= nSpace' to 'return nTmpHeight < nSpace'.
            // This obviously results in problems with table frames in
            // sections. Remember: Every twip is sacred.
            return nTmpHeight <= nSpace;
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
        //The old follower eventually calculated a margin to the predecessor
        //which is obsolete now as it became the first one
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
        if ( nullptr != (pFrame = GetPrev()) )
        {   pFrame->SetRetouche();
            pFrame->Prepare( PREP_WIDOWS_ORPHANS );
            pFrame->InvalidatePos_();
            if ( pFrame->IsContentFrame() )
                pFrame->InvalidatePage( pPage );
        }
        //If I am (was) the only FlowFrame in my own upper, it has to do
        //the retouch. Moreover it has to do the retouch.
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
        Prepare( PREP_FIXSIZE_CHG );
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
        //Take the margin into account when dealing with ContentFrame's. There are
        //two situations (both always happen at once):
        //a) The Content becomes the first in a chain
        //b) The new follower was the first in a chain before
        GetNext()->InvalidatePrt_();

    if ( pPage && !IsFollow() )
    {
        if ( pPage->GetUpper() )
            static_cast<SwRootFrame*>(pPage->GetUpper())->InvalidateBrowseWidth();

        if ( !GetPrev() )//At least needed for HTML with a table at the beginning.
        {
            const SwPageDesc *pDesc = GetFormat()->GetPageDesc().GetPageDesc();
            if ( (pDesc && pDesc != pPage->GetPageDesc()) ||
                 (!pDesc && pPage->GetPageDesc() != &GetFormat()->GetDoc()->GetPageDesc(0)) )
                CheckPageDescs( pPage );
        }
    }
}

bool SwTabFrame::Prepare( const PrepareHint eHint, const void *, bool )
{
    if( PREP_BOSS_CHGD == eHint )
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
    , m_bIsInSplit( false )
{
    mnFrameType = SwFrameType::Row;

    //Create the boxes and insert them.
    const SwTableBoxes &rBoxes = rLine.GetTabBoxes();
    SwFrame *pTmpPrev = nullptr;
    for ( size_t i = 0; i < rBoxes.size(); ++i )
    {
        SwCellFrame *pNew = new SwCellFrame( *rBoxes[i], this, bInsertContent );
        pNew->InsertBehind( this, pTmpPrev );
        pTmpPrev = pNew;
    }
}

void SwRowFrame::DestroyImpl()
{
    SwModify* pMod = GetFormat();
    if( pMod )
    {
        pMod->Remove( this );           // remove,
        if( !pMod->HasWriterListeners() )
            delete pMod;                // and delete
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

void SwRowFrame::Modify( const SfxPoolItem* pOld, const SfxPoolItem * pNew )
{
    bool bAttrSetChg = pNew && RES_ATTRSET_CHG == pNew->Which();
    const SfxPoolItem *pItem = nullptr;

    if( bAttrSetChg )
    {
        const SwAttrSet* pChgSet = static_cast<const SwAttrSetChg*>(pNew)->GetChgSet();
        pChgSet->GetItemState( RES_FRM_SIZE, false, &pItem);
        if ( !pItem )
            pChgSet->GetItemState( RES_ROW_SPLIT, false, &pItem);
    }
    else if (pNew && (RES_FRM_SIZE == pNew->Which() || RES_ROW_SPLIT == pNew->Which()))
        pItem = pNew;

    if ( pItem )
    {
        SwTabFrame *pTab = FindTabFrame();
        if ( pTab )
        {
            const bool bInFirstNonHeadlineRow = pTab->IsFollow() &&
                                                this == pTab->GetFirstNonHeadlineRow();
            // #i35063#
            // Invalidation required is pRow is last row
            if ( bInFirstNonHeadlineRow || !GetNext() )
            {
                if ( bInFirstNonHeadlineRow )
                    pTab = pTab->FindMaster();
                pTab->InvalidatePos();
            }
        }
    }

    SwLayoutFrame::Modify( pOld, pNew );
}

void SwRowFrame::MakeAll(vcl::RenderContext* pRenderContext)
{
    if ( !GetNext() )
    {
        setFrameAreaSizeValid(false);
    }

    SwLayoutFrame::MakeAll(pRenderContext);
}

long CalcHeightWithFlys( const SwFrame *pFrame )
{
    SwRectFnSet aRectFnSet(pFrame);
    long nHeight = 0;
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
                    const SwFrameFormat& rFrameFormat = pAnchoredObj->GetFrameFormat();
                    const bool bConsiderObj =
                        (rFrameFormat.GetAnchor().GetAnchorId() != RndStdIds::FLY_AS_CHAR) &&
                            pAnchoredObj->GetObjRect().Top() != FAR_AWAY &&
                            rFrameFormat.GetFollowTextFlow().GetValue() &&
                            pAnchoredObj->GetPageFrame() == pTmp->FindPageFrame();
                    if ( bConsiderObj )
                    {
                        const SwFormatFrameSize &rSz = rFrameFormat.GetFrameSize();
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

                            nHeight = std::max( nHeight, nDistOfFlyBottomToAnchorTop2 );
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
    if ( pTab->IsCollapsingBorders() && rCell.Lower() && !rCell.Lower()->IsRowFrame() )
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
    if ( pLow )
    {
        long nFlyAdd = 0;
        while ( pLow )
        {
            // OD 2004-02-18 #106629# - change condition and switch then-body
            // and else-body
            if ( pLow->IsRowFrame() )
            {
                // #i26945#
                nHeight += ::lcl_CalcMinRowHeight( static_cast<const SwRowFrame*>(pLow),
                                                   _bConsiderObjs );
            }
            else
            {
                long nLowHeight = aRectFnSet.GetHeight(pLow->getFrameArea());
                nHeight += nLowHeight;
                // #i26945#
                if ( _bConsiderObjs )
                {
                    nFlyAdd = std::max( 0L, nFlyAdd - nLowHeight );
                    nFlyAdd = std::max( nFlyAdd, ::CalcHeightWithFlys( pLow ) );
                }
            }

            pLow = pLow->GetNext();
        }
        if ( nFlyAdd )
            nHeight += nFlyAdd;
    }
    //The border needs to be considered too, unfortunately it can't be
    //calculated using PrtArea and Frame because those can be invalid in arbitrary
    //combinations.
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

// OD 2004-02-18 #106629# - correct type of 1st parameter
// #i26945# - add parameter <_bConsiderObjs> in order to control,
// if floating screen objects have to be considered for the minimal cell height
static SwTwips lcl_CalcMinRowHeight( const SwRowFrame* _pRow,
                                     const bool _bConsiderObjs )
{
    SwTwips nHeight = 0;
    if ( !_pRow->IsRowSpanLine() )
    {
        const SwFormatFrameSize &rSz = _pRow->GetFormat()->GetFrameSize();
        if ( _pRow->HasFixSize() )
        {
            OSL_ENSURE(ATT_FIX_SIZE == rSz.GetHeightSizeType(), "pRow claims to have fixed size");
            return rSz.GetHeight();
        }
        // If this row frame is being split, then row's minimal height shouldn't restrict
        // this frame's minimal height, because the rest will go to follow frame.
        else if ( !_pRow->IsInSplit() && rSz.GetHeightSizeType() == ATT_MIN_SIZE )
        {
            nHeight = rSz.GetHeight() - lcl_calcHeightOfRowBeforeThisFrame(*_pRow);
        }
    }

    SwRectFnSet aRectFnSet(_pRow);
    const SwCellFrame* pLow = static_cast<const SwCellFrame*>(_pRow->Lower());
    while ( pLow )
    {
        SwTwips nTmp = 0;
        const long nRowSpan = pLow->GetLayoutRowSpan();
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
        if ( pCurrLower->Lower() && pCurrLower->Lower()->IsRowFrame() )
            nTmpTopSpace = lcl_GetTopSpace( *static_cast<const SwRowFrame*>(pCurrLower->Lower()) );
        else
        {
            const SwAttrSet& rSet = const_cast<SwCellFrame*>(pCurrLower)->GetFormat()->GetAttrSet();
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
        if ( pCurrLower->Lower() && pCurrLower->Lower()->IsRowFrame() )
            nTmpTopLineDist = lcl_GetTopLineDist( *static_cast<const SwRowFrame*>(pCurrLower->Lower()) );
        else
        {
            const SwAttrSet& rSet = const_cast<SwCellFrame*>(pCurrLower)->GetFormat()->GetAttrSet();
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
        if ( pCurrLower->Lower() && pCurrLower->Lower()->IsRowFrame() )
        {
            const SwFrame* pRow = pCurrLower->GetLastLower();
            nTmpBottomLineSize = lcl_GetBottomLineSize( *static_cast<const SwRowFrame*>(pRow) );
        }
        else
        {
            const SwAttrSet& rSet = const_cast<SwCellFrame*>(pCurrLower)->GetFormat()->GetAttrSet();
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
        if ( pCurrLower->Lower() && pCurrLower->Lower()->IsRowFrame() )
        {
            const SwFrame* pRow = pCurrLower->GetLastLower();
            nTmpBottomLineDist = lcl_GetBottomLineDist( *static_cast<const SwRowFrame*>(pRow) );
        }
        else
        {
            const SwAttrSet& rSet = const_cast<SwCellFrame*>(pCurrLower)->GetFormat()->GetAttrSet();
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

    if ( !isFramePrintAreaValid() )
    {
        //RowFrames don't have borders and so on therefore the PrtArea always
        //matches the Frame.
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
        SwTabFrame* pTabFrame = FindTabFrame();
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
                                ? pAttrs->GetSize().Height()
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
    if ( !GetNext() )
    {
        //The last fills the remaining space in the upper.
        SwTwips nDiff = aRectFnSet.GetHeight(GetUpper()->getFramePrintArea());
        SwFrame *pSibling = GetUpper()->Lower();
        do
        {   nDiff -= aRectFnSet.GetHeight(pSibling->getFrameArea());
            pSibling = pSibling->GetNext();
        } while ( pSibling );
        if ( nDiff > 0 )
        {
            mbFixSize = false;
            Grow( nDiff );
            mbFixSize = bFix;
            setFrameAreaSizeValid(true);
        }
    }
}

void SwRowFrame::AdjustCells( const SwTwips nHeight, const bool bHeight )
{
    SwFrame *pFrame = Lower();
    if ( bHeight )
    {
        SwRootFrame *pRootFrame = getRootFrame();
        SwRectFnSet aRectFnSet(this);
        SwRect aOldFrame;

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
                const long nDiff = nHeight - aRectFnSet.GetHeight(pCellFrame->getFrameArea());
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
            long nRowSpan = pToAdjust->GetLayoutRowSpan();
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

            const long nDiff = nSumRowHeight - aRectFnSet.GetHeight(pToAdjust->getFrameArea());
            if ( nDiff )
            {
                aOldFrame = pToAdjust->getFrameArea();
                SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*pToAdjust);
                aRectFnSet.AddBottom( aFrm, nDiff );
                pNotify = pToAdjust;
            }

            if ( pNotify )
            {
                if( pRootFrame && pRootFrame->IsAnyShellAccessible() && pRootFrame->GetCurrShell() )
                    pRootFrame->GetCurrShell()->Imp()->MoveAccessibleFrame( pNotify, aOldFrame );

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

SwTwips SwRowFrame::GrowFrame( SwTwips nDist, bool bTst, bool bInfo )
{
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

    nReal += SwLayoutFrame::GrowFrame( nDist, bTst, bInfo);

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
        if (rSz.GetHeightSizeType() == ATT_MIN_SIZE)
            nMinHeight = std::max(rSz.GetHeight() - lcl_calcHeightOfRowBeforeThisFrame(*this),
                                  0L);

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

    //Invalidate if possible and update the height to the newest value.
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
        OSL_ENSURE( ATT_FIX_SIZE == GetFormat()->GetFrameSize().GetHeightSizeType(), "pRow claims to have fixed size" );
        return false;
    }

    // Repeated headlines are never allowed to split:
    const SwTabFrame* pTabFrame = FindTabFrame();
    if ( pTabFrame->GetTable()->GetRowsToRepeat() > 0 &&
         pTabFrame->IsInHeadline( *this ) )
        return false;

    const SwTableLineFormat* pFrameFormat = static_cast<SwTableLineFormat*>(GetTabLine()->GetFrameFormat());
    const SwFormatRowSplit& rLP = pFrameFormat->GetRowSplit();
    return rLP.GetValue();
}

bool SwRowFrame::ShouldRowKeepWithNext( const bool bCheckParents ) const
{
    // No KeepWithNext if nested in another table
    if ( GetUpper()->GetUpper()->IsCellFrame() )
        return false;

    const SwCellFrame* pCell = static_cast<const SwCellFrame*>(Lower());
    const SwFrame* pText = pCell->Lower();

    return pText && pText->IsTextFrame() &&
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
    if ( rBox.GetSttIdx() )
    {
        sal_uLong nIndex = rBox.GetSttIdx();
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
    SwModify* pMod = GetFormat();
    if( pMod )
    {
        // At this stage the lower frames aren't destroyed already,
        // therefore we have to do a recursive dispose.
        SwRootFrame *pRootFrame = getRootFrame();
        if( pRootFrame && pRootFrame->IsAnyShellAccessible() &&
            pRootFrame->GetCurrShell() )
        {
            pRootFrame->GetCurrShell()->Imp()->DisposeAccessibleFrame( this, true );
        }

        pMod->Remove( this );           // remove,
        if( !pMod->HasWriterListeners() )
            delete pMod;                // and delete
    }

    SwLayoutFrame::DestroyImpl();
}

SwCellFrame::~SwCellFrame()
{
}

static bool lcl_ArrangeLowers( SwLayoutFrame *pLay, long lYStart, bool bInva )
{
    bool bRet = false;
    SwFrame *pFrame = pLay->Lower();
    SwRectFnSet aRectFnSet(pLay);
    while ( pFrame )
    {
        long nFrameTop = aRectFnSet.GetTop(pFrame->getFrameArea());
        if( nFrameTop != lYStart )
        {
            bRet = true;
            const long lDiff = aRectFnSet.YDiff( lYStart, nFrameTop );
            const long lDiffX = lYStart - nFrameTop;

            {
                SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*pFrame);
                aRectFnSet.SubTop( aFrm, -lDiff );
                aRectFnSet.AddBottom( aFrm, lDiff );
            }

            pFrame->SetCompletePaint();

            if ( !pFrame->GetNext() )
                pFrame->SetRetouche();
            if( bInva )
                pFrame->Prepare( PREP_POS_CHGD );
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
                        SwFormatVertOrient aVert( pAnchoredObj->GetFrameFormat().GetVertOrient() );
                        switch ( aVert.GetRelationOrient() )
                        {
                            case text::RelOrientation::PAGE_FRAME:
                            case text::RelOrientation::PAGE_PRINT_AREA:
                                bVertPosDepOnAnchor = false;
                                break;
                            default: break;
                        }
                    }
                    if ( dynamic_cast< const SwFlyFrame *>( pAnchoredObj ) !=  nullptr )
                    {
                        SwFlyFrame *pFly = static_cast<SwFlyFrame*>(pAnchoredObj);

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

                            pFly->GetVirtDrawObj()->SetRectsDirty();
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
                                if ( pPageFrame )
                                    pPageFrame->MoveFly( pFly, pPageOfAnchor );
                                else
                                    pPageOfAnchor->AppendFlyToPage( pFly );
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
                            (pAnchoredObj->GetFrameFormat().GetAnchor().GetAnchorId()
                                                            != RndStdIds::FLY_AS_CHAR))
                        {
                            SwPageFrame* pPageFrame = pAnchoredObj->GetPageFrame();
                            SwPageFrame* pPageOfAnchor = pFrame->FindPageFrame();
                            if ( pPageFrame != pPageOfAnchor )
                            {
                                pAnchoredObj->InvalidateObjPos();
                                if ( pPageFrame )
                                {
                                    pPageFrame->RemoveDrawObjFromPage( *pAnchoredObj );
                                }
                                pPageOfAnchor->AppendDrawObjToPage( *pAnchoredObj );
                            }
                        }
                        // #i28701# - adjust last character
                        // rectangle and last top of line.
                        pAnchoredObj->AddLastCharY( lDiff );
                        pAnchoredObj->AddLastTopOfLineY( lDiff );
                        // #i52904# - re-introduce direct move
                        // of drawing objects
                        const bool bDirectMove =
                                static_cast<const SwDrawFrameFormat&>(pAnchoredObj->GetFrameFormat()).IsPosAttrSet() &&
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

    if ( !isFramePrintAreaValid() )
    {
        setFramePrintAreaValid(true);

        //Adjust position.
        if ( Lower() )
        {
            SwTwips nTopSpace, nBottomSpace, nLeftSpace, nRightSpace;
            // #i29550#
            if ( pTab->IsCollapsingBorders() && !Lower()->IsRowFrame()  )
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
    long nRemaining = GetTabBox()->getRowSpan() >= 1 ?
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

            const long nPrtWidth = aRectFnSet.GetWidth(pTab->getFramePrintArea());
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

        const long nDiff = nWidth - aRectFnSet.GetWidth(getFrameArea());

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

        //Adjust the height, it's defined through the content and the border.
        const long nDiffHeight = nRemaining - aRectFnSet.GetHeight(getFrameArea());
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
                //Only keep invalidated if shrinking was done; this can be
                //dismissed because all adjoined cells have to be the same size.
                if ( !Shrink( -nDiffHeight ) )
                {
                    setFrameAreaSizeValid(true);
                    setFramePrintAreaValid(true);
                }
            }
        }
    }
    const SwFormatVertOrient &rOri = pAttrs->GetAttrSet().GetVertOrient();

    if ( !Lower() )
        return;

    // From now on, all operations are related to the table cell.
    aRectFnSet.Refresh(this);

    SwPageFrame* pPg = nullptr;
    if ( !FindTabFrame()->IsRebuildLastLine() && text::VertOrientation::NONE != rOri.GetVertOrient() &&
    // #158225# no vertical alignment of covered cells
         !IsCoveredCell() &&
         (pPg = FindPageFrame())!=nullptr )
    {
        if ( !Lower()->IsContentFrame() && !Lower()->IsSctFrame() && !Lower()->IsTabFrame() )
        {
            // OSL_ENSURE(for HTML-import!
            OSL_ENSURE( false, "VAlign to cell without content" );
            return;
        }
        bool bVertDir = true;
        // #i43913# - no vertical alignment, if wrapping
        // style influence is considered on object positioning and
        // an object is anchored inside the cell.
        const bool bConsiderWrapOnObjPos( GetFormat()->getIDocumentSettingAccess().get(DocumentSettingId::CONSIDER_WRAP_ON_OBJECT_POSITION) );
        //No alignment if border with flow overlaps the cell.
        if ( pPg->GetSortedObjs() )
        {
            SwRect aRect( getFramePrintArea() ); aRect += getFrameArea().Pos();
            for (SwAnchoredObject* pAnchoredObj : *pPg->GetSortedObjs())
            {
                SwRect aTmp( pAnchoredObj->GetObjRect() );
                const SwFrame* pAnch = pAnchoredObj->GetAnchorFrame();
                if ( (bConsiderWrapOnObjPos && IsAnLower( pAnch )) || (!bConsiderWrapOnObjPos && aTmp.IsOver( aRect )) )
                {
                    const SwFrameFormat& rAnchoredObjFrameFormat = pAnchoredObj->GetFrameFormat();
                    const SwFormatSurround &rSur = rAnchoredObjFrameFormat.GetSurround();

                    if ( bConsiderWrapOnObjPos || css::text::WrapTextMode_THROUGH != rSur.GetSurround() )
                    {
                        // frames, which the cell is a lower of, aren't relevant
                        if ( auto pFly = dynamic_cast< const SwFlyFrame *>( pAnchoredObj ) )
                        {
                            if ( pFly->IsAnLower( this ) )
                                continue;
                        }

                        // #i43913#
                        // #i52904# - no vertical alignment,
                        // if object, anchored inside cell, has temporarily
                        // consider its wrapping style on object positioning.
                        // #i58806# - no vertical alignment
                        // if object does not follow the text flow.
                        if ( bConsiderWrapOnObjPos ||
                             !IsAnLower( pAnch ) ||
                             pAnchoredObj->IsTmpConsiderWrapInfluence() ||
                             !rAnchoredObjFrameFormat.GetFollowTextFlow().GetValue() )
                        {
                            bVertDir = false;
                            break;
                        }
                    }
                }
            }
        }

        long nPrtHeight = aRectFnSet.GetHeight(getFramePrintArea());
        if( ( bVertDir && ( nRemaining -= lcl_CalcTopAndBottomMargin( *this, *pAttrs ) ) < nPrtHeight ) ||
            aRectFnSet.GetTop(Lower()->getFrameArea()) != aRectFnSet.GetPrtTop(*this) )
        {
            long nDiff = aRectFnSet.GetHeight(getFramePrintArea()) - nRemaining;
            if ( nDiff >= 0 )
            {
                long lTopOfst = 0;
                if ( bVertDir )
                {
                    switch ( rOri.GetVertOrient() )
                    {
                        case text::VertOrientation::CENTER:   lTopOfst = nDiff / 2; break;
                        case text::VertOrientation::BOTTOM:   lTopOfst = nDiff;     break;
                        default: break;
                    };
                }
                long nTmp = aRectFnSet.YInc(
                                    aRectFnSet.GetPrtTop(*this), lTopOfst );
                if ( lcl_ArrangeLowers( this, nTmp, !bVertDir ) )
                    SetCompletePaint();
            }
        }
    }
    else
    {
        //Was an old alignment taken into account?
        if ( Lower()->IsContentFrame() )
        {
            const long lYStart = aRectFnSet.GetPrtTop(*this);
            lcl_ArrangeLowers( this, lYStart, true );
        }
    }
}

void SwCellFrame::Modify( const SfxPoolItem* pOld, const SfxPoolItem * pNew )
{
    bool bAttrSetChg = pNew && RES_ATTRSET_CHG == pNew->Which();
    const SfxPoolItem *pItem = nullptr;

    if( bAttrSetChg )
        static_cast<const SwAttrSetChg*>(pNew)->GetChgSet()->GetItemState( RES_VERT_ORIENT, false, &pItem);
    else if (pNew && RES_VERT_ORIENT == pNew->Which())
        pItem = pNew;

    if ( pItem )
    {
        bool bInva = true;
        if ( text::VertOrientation::NONE == static_cast<const SwFormatVertOrient*>(pItem)->GetVertOrient() &&
             // OD 04.11.2003 #112910#
             Lower() && Lower()->IsContentFrame() )
        {
            SwRectFnSet aRectFnSet(this);
            const long lYStart = aRectFnSet.GetPrtTop(*this);
            bInva = lcl_ArrangeLowers( this, lYStart, false );
        }
        if ( bInva )
        {
            SetCompletePaint();
            InvalidatePrt();
        }
    }

    if ( ( bAttrSetChg && pNew &&
           SfxItemState::SET == static_cast<const SwAttrSetChg*>(pNew)->GetChgSet()->GetItemState( RES_PROTECT, false ) ) ||
         ( pNew && RES_PROTECT == pNew->Which()) )
    {
        SwViewShell *pSh = getRootFrame()->GetCurrShell();
        if( pSh && pSh->GetLayout()->IsAnyShellAccessible() )
            pSh->Imp()->InvalidateAccessibleEditableState( true, this );
    }

    if ( bAttrSetChg && pNew &&
         SfxItemState::SET == static_cast<const SwAttrSetChg*>(pNew)->GetChgSet()->GetItemState( RES_FRAMEDIR, false, &pItem ) )
    {
        SetDerivedVert( false );
        CheckDirChange();
    }

    // #i29550#
    if ( bAttrSetChg && pNew &&
         SfxItemState::SET == static_cast<const SwAttrSetChg*>(pNew)->GetChgSet()->GetItemState( RES_BOX, false, &pItem ) )
    {
        SwFrame* pTmpUpper = GetUpper();
        while ( pTmpUpper->GetUpper() && !pTmpUpper->GetUpper()->IsTabFrame() )
            pTmpUpper = pTmpUpper->GetUpper();

        SwTabFrame* pTabFrame = static_cast<SwTabFrame*>(pTmpUpper->GetUpper());
        if ( pTabFrame->IsCollapsingBorders() )
        {
            // Invalidate lowers of this and next row:
            lcl_InvalidateAllLowersPrt( static_cast<SwRowFrame*>(pTmpUpper) );
            pTmpUpper = pTmpUpper->GetNext();
            if ( pTmpUpper )
                lcl_InvalidateAllLowersPrt( static_cast<SwRowFrame*>(pTmpUpper) );
            else
                pTabFrame->InvalidatePrt();
        }
    }

    SwLayoutFrame::Modify( pOld, pNew );
}

long SwCellFrame::GetLayoutRowSpan() const
{
    long nRet = GetTabBox()->getRowSpan();
    if ( nRet < 1 )
    {
        const SwFrame* pRow = GetUpper();
        const SwTabFrame* pTab = pRow ? static_cast<const SwTabFrame*>(pRow->GetUpper()) : nullptr;

        if ( pTab && pTab->IsFollow() && pRow == pTab->GetFirstNonHeadlineRow() )
            nRet = -nRet;
    }
    return  nRet;
}

void SwCellFrame::dumpAsXmlAttributes(xmlTextWriterPtr pWriter) const
{
    SwFrame::dumpAsXmlAttributes(pWriter);
    if (SwCellFrame* pFollow = GetFollowCell())
        xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("follow"), "%" SAL_PRIuUINT32, pFollow->GetFrameId());

    if (SwCellFrame* pPrevious = GetPreviousCell())
        xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("precede"), "%" SAL_PRIuUINT32, pPrevious->GetFrameId());
}

// #i103961#
void SwCellFrame::Cut()
{
    // notification for accessibility
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
            SwTwips nTmpHeight = USHRT_MAX;
            // #i32456# Consider lower row frames
            if ( pTmp->IsRowFrame() )
            {
                const SwRowFrame* pTmpSourceRow = static_cast<const SwRowFrame*>(pCurrSourceCell->Lower());
                nTmpHeight = lcl_CalcHeightOfFirstContentLine( *pTmpSourceRow );
            }
            else if ( pTmp->IsTabFrame() )
            {
                nTmpHeight = static_cast<const SwTabFrame*>(pTmp)->CalcHeightOfFirstContentLine();
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

            if ( USHRT_MAX != nTmpHeight )
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
                if ( nTmpHeight > nHeight && USHRT_MAX != nTmpHeight )
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

    SwTwips nTmpHeight = 0;

    const SwRowFrame* pFirstRow = GetFirstNonHeadlineRow();
    OSL_ENSURE( !IsFollow() || pFirstRow, "FollowTable without Lower" );

    // NEW TABLES
    if ( pFirstRow && pFirstRow->IsRowSpanLine() && pFirstRow->GetNext() )
        pFirstRow = static_cast<const SwRowFrame*>(pFirstRow->GetNext());

    // Calculate the height of the headlines:
    const sal_uInt16 nRepeat = GetTable()->GetRowsToRepeat();
    SwTwips nRepeatHeight = nRepeat ? lcl_GetHeightOfRows( GetLower(), nRepeat ) : 0;

    // Calculate the height of the keeping lines
    // (headlines + following keeping lines):
    SwTwips nKeepHeight = nRepeatHeight;
    if ( GetFormat()->GetDoc()->GetDocumentSettingManager().get(DocumentSettingId::TABLE_ROW_KEEP) )
    {
        sal_uInt16 nKeepRows = nRepeat;

        // Check how many rows want to keep together
        while ( pFirstRow && pFirstRow->ShouldRowKeepWithNext() )
        {
            ++nKeepRows;
            pFirstRow = static_cast<const SwRowFrame*>(pFirstRow->GetNext());
        }

        if ( nKeepRows > nRepeat )
            nKeepHeight = lcl_GetHeightOfRows( GetLower(), nKeepRows );
    }

    // For master tables, the height of the headlines + the height of the
    // keeping lines (if any) has to be considered. For follow tables, we
    // only consider the height of the keeping rows without the repeated lines:
    if ( !IsFollow() )
    {
        nTmpHeight = nKeepHeight;
    }
    else
    {
        nTmpHeight = nKeepHeight - nRepeatHeight;
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
            if ( pFirstRow->GetPrev() &&
                 static_cast<const SwRowFrame*>(pFirstRow->GetPrev())->IsRowSpanLine() )
            {
                // Calculate maximum height of all cells with rowspan = 1:
                SwTwips nMaxHeight = 0;
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
            if (rSz.GetHeightSizeType() == ATT_MIN_SIZE)
            {
                nMinRowHeight = std::max(rSz.GetHeight() - lcl_calcHeightOfRowBeforeThisFrame(*pFirstRow),
                                         0L);
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
    const SwCellFrame* pThisCell = dynamic_cast<const SwCellFrame*>(this);
    return pThisCell && pThisCell->GetLayoutRowSpan() > 1;
}

bool SwFrame::IsCoveredCell() const
{
    const SwCellFrame* pThisCell = dynamic_cast<const SwCellFrame*>(this);
    return pThisCell && pThisCell->GetLayoutRowSpan() < 1;
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
