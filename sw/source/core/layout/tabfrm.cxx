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

#include "pagefrm.hxx"
#include "rootfrm.hxx"
#include <IDocumentFieldsAccess.hxx>
#include "viewimp.hxx"
#include "swtable.hxx"
#include "dflyobj.hxx"
#include <anchoreddrawobject.hxx>
#include <fmtanchr.hxx>
#include "viewopt.hxx"
#include "hints.hxx"
#include "dbg_lay.hxx"
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
#include "tabfrm.hxx"
#include "rowfrm.hxx"
#include "cellfrm.hxx"
#include "flyfrms.hxx"
#include "txtfrm.hxx"
#include "htmltbl.hxx"
#include "sectfrm.hxx"
#include <fmtfollowtextflow.hxx>
#include <sortedobjs.hxx>
#include <objectformatter.hxx>
#include <layouter.hxx>
#include <calbck.hxx>
#include <DocumentSettingManager.hxx>
#include <docary.hxx>
#include <o3tl/make_unique.hxx>

using namespace ::com::sun::star;

SwTabFrm::SwTabFrm( SwTable &rTab, SwFrm* pSib )
    : SwLayoutFrm( rTab.GetFrameFormat(), pSib )
    , SwFlowFrm( static_cast<SwFrm&>(*this) )
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
    mnFrmType = FRM_TAB;

    //Create the lines and insert them.
    const SwTableLines &rLines = rTab.GetTabLines();
    SwFrm *pTmpPrev = 0;
    for ( size_t i = 0; i < rLines.size(); ++i )
    {
        SwRowFrm *pNew = new SwRowFrm( *rLines[i], this );
        if( pNew->Lower() )
        {
            pNew->InsertBehind( this, pTmpPrev );
            pTmpPrev = pNew;
        }
        else
            SwFrm::DestroyFrm(pNew);
    }
    OSL_ENSURE( Lower() && Lower()->IsRowFrm(), "SwTabFrm::SwTabFrm: No rows." );
}

SwTabFrm::SwTabFrm( SwTabFrm &rTab )
    : SwLayoutFrm( rTab.GetFormat(), &rTab )
    , SwFlowFrm( static_cast<SwFrm&>(*this) )
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
    mnFrmType = FRM_TAB;

    SetFollow( rTab.GetFollow() );
    rTab.SetFollow( this );
}

extern const SwTable   *pColumnCacheLastTable;
extern const SwTabFrm  *pColumnCacheLastTabFrm;
extern const SwFrm     *pColumnCacheLastCellFrm;
extern const SwTable   *pRowCacheLastTable;
extern const SwTabFrm  *pRowCacheLastTabFrm;
extern const SwFrm     *pRowCacheLastCellFrm;

//return the SwTabFrm (if any) that this SwTabFrm is a follow flow line for
SwTabFrm* SwTabFrm::GetFollowFlowLineFor()
{
    SwFlowFrm *pPrec = GetPrecede();
    if (pPrec && pPrec->GetFrm().IsTabFrm())
    {
        SwTabFrm *pPrevTabFrm = static_cast<SwTabFrm*>(pPrec);
        assert(this == pPrevTabFrm->GetFollow());
        if (pPrevTabFrm->HasFollowFlowLine() && pPrevTabFrm->GetFollow() == this)
            return pPrevTabFrm;
    }
    return NULL;
}

void SwTabFrm::DestroyImpl()
{
    //rhbz#907933, we are a follow flow line for something and have been
    //deleted, remove ourself as a follow flowline
    SwTabFrm* pFlowFrameFor = GetFollowFlowLineFor();
    if (pFlowFrameFor)
        pFlowFrameFor->RemoveFollowFlowLine();

    // There is some terrible code in fetab.cxx, that
    // makes use of these global pointers. Obviously
    // this code did not consider that a TabFrm can be
    // deleted.
    if ( this == pColumnCacheLastTabFrm )
    {
        pColumnCacheLastTable  = NULL;
        pColumnCacheLastTabFrm = NULL;
        pColumnCacheLastCellFrm= NULL;
        pRowCacheLastTable  = NULL;
        pRowCacheLastTabFrm = NULL;
        pRowCacheLastCellFrm= NULL;
    }

    SwLayoutFrm::DestroyImpl();
}

SwTabFrm::~SwTabFrm()
{
}

void SwTabFrm::JoinAndDelFollows()
{
    SwTabFrm *pFoll = GetFollow();
    if ( pFoll->HasFollow() )
        pFoll->JoinAndDelFollows();
    pFoll->Cut();
    SetFollow( pFoll->GetFollow() );
    SwFrm::DestroyFrm(pFoll);
}

void SwTabFrm::RegistFlys()
{
    OSL_ENSURE( Lower() && Lower()->IsRowFrm(), "No rows." );

    SwPageFrm *pPage = FindPageFrm();
    if ( pPage )
    {
        SwRowFrm *pRow = static_cast<SwRowFrm*>(Lower());
        do
        {
            pRow->RegistFlys( pPage );
            pRow = static_cast<SwRowFrm*>(pRow->GetNext());
        } while ( pRow );
    }
}

void SwInvalidateAll( SwFrm *pFrm, long nBottom );
static void lcl_RecalcRow( SwRowFrm& rRow, long nBottom );
static bool lcl_ArrangeLowers( SwLayoutFrm *pLay, long lYStart, bool bInva );
// #i26945# - add parameter <_bOnlyRowsAndCells> to control
// that only row and cell frames are formatted.
static bool lcl_InnerCalcLayout( SwFrm *pFrm,
                                      long nBottom,
                                      bool _bOnlyRowsAndCells = false );
// OD 2004-02-18 #106629# - correct type of 1st parameter
// #i26945# - add parameter <_bConsiderObjs> in order to
// control, if floating screen objects have to be considered for the minimal
// cell height.
static SwTwips lcl_CalcMinRowHeight( const SwRowFrm *pRow,
                                     const bool _bConsiderObjs );
static SwTwips lcl_CalcTopAndBottomMargin( const SwLayoutFrm&, const SwBorderAttrs& );

static SwTwips lcl_GetHeightOfRows( const SwFrm* pStart, long nCount )
{
    if ( !nCount || !pStart)
        return 0;

    SwTwips nRet = 0;
    SWRECTFN( pStart )
    while ( pStart && nCount > 0 )
    {
        nRet += (pStart->Frm().*fnRect->fnGetHeight)();
        pStart = pStart->GetNext();
        --nCount;
    }

    return nRet;
}

// Local helper function to insert a new follow flow line
static SwRowFrm* lcl_InsertNewFollowFlowLine( SwTabFrm& rTab, const SwFrm& rTmpRow, bool bRowSpanLine )
{
    OSL_ENSURE( rTmpRow.IsRowFrm(), "No row frame to copy for FollowFlowLine" );
    const SwRowFrm& rRow = static_cast<const SwRowFrm&>(rTmpRow);

    rTab.SetFollowFlowLine( true );
    SwRowFrm *pFollowFlowLine = new SwRowFrm(*rRow.GetTabLine(), &rTab, false );
    pFollowFlowLine->SetRowSpanLine( bRowSpanLine );
    SwFrm* pFirstRow = rTab.GetFollow()->GetFirstNonHeadlineRow();
    pFollowFlowLine->InsertBefore( rTab.GetFollow(), pFirstRow );
    return pFollowFlowLine;
}

// #i26945# - local helper function to invalidate all lower
// objects. By parameter <_bMoveObjsOutOfRange> it can be controlled, if
// additionally the objects are moved 'out of range'.
static void lcl_InvalidateLowerObjs( SwLayoutFrm& _rLayoutFrm,
                              const bool _bMoveObjsOutOfRange = false,
                              SwPageFrm* _pPageFrm = 0L )
{
    // determine page frame, if needed
    if ( !_pPageFrm )
    {
        _pPageFrm = _rLayoutFrm.FindPageFrm();
        OSL_ENSURE( _pPageFrm,
                "<lcl_InvalidateLowerObjs(..)> - missing page frame -> no move of lower objects out of range" );
        if ( !_pPageFrm )
        {
            return;
        }
    }

    // loop on lower frames
    SwFrm* pLowerFrm = _rLayoutFrm.Lower();
    while ( pLowerFrm )
    {
        if ( pLowerFrm->IsLayoutFrm() )
        {
            ::lcl_InvalidateLowerObjs( *(static_cast<SwLayoutFrm*>(pLowerFrm)),
                                       _bMoveObjsOutOfRange, _pPageFrm );
        }
        if ( pLowerFrm->GetDrawObjs() )
        {
            for ( size_t i = 0; i < pLowerFrm->GetDrawObjs()->size(); ++i )
            {
                SwAnchoredObject* pAnchoredObj = (*pLowerFrm->GetDrawObjs())[i];

                // invalidate position of anchored object
                pAnchoredObj->SetTmpConsiderWrapInfluence( false );
                pAnchoredObj->SetConsiderForTextWrap( false );
                pAnchoredObj->UnlockPosition();
                pAnchoredObj->InvalidateObjPos();

                // move anchored object 'out of range'
                if ( _bMoveObjsOutOfRange )
                {
                    // indicate, that positioning is progress to avoid
                    // modification of the anchored object resp. its attributes
                    // due to the movement
                    SwObjPositioningInProgress aObjPosInProgress( *pAnchoredObj );
                    pAnchoredObj->SetObjLeft( _pPageFrm->Frm().Right() );
                    // #115759# - reset character rectangle,
                    // top of line and relative position in order to assure,
                    // that anchored object is correctly positioned.
                    pAnchoredObj->ClearCharRectAndTopOfLine();
                    pAnchoredObj->SetCurrRelPos( Point( 0, 0 ) );
                    if ( pAnchoredObj->GetFrameFormat().GetAnchor().GetAnchorId()
                            == FLY_AS_CHAR )
                    {
                        pAnchoredObj->AnchorFrm()
                                ->Prepare( PREP_FLY_ATTR_CHG,
                                           &(pAnchoredObj->GetFrameFormat()) );
                    }
                    if ( dynamic_cast< const SwFlyFrm *>( pAnchoredObj ) !=  nullptr )
                    {
                        SwFlyFrm *pFly = static_cast<SwFlyFrm*>(pAnchoredObj);
                        pFly->GetVirtDrawObj()->SetRectsDirty();
                        pFly->GetVirtDrawObj()->SetChanged();
                    }
                }

                // If anchored object is a fly frame, invalidate its lower objects
                if ( dynamic_cast< const SwFlyFrm *>( pAnchoredObj ) !=  nullptr )
                {
                    SwFlyFrm *pFly = static_cast<SwFlyFrm*>(pAnchoredObj);
                    ::lcl_InvalidateLowerObjs( *pFly, _bMoveObjsOutOfRange, _pPageFrm );
                }
            }
        }
        pLowerFrm = pLowerFrm->GetNext();
    }
}

// Local helper function to shrink all lowers of rRow to 0 height
static void lcl_ShrinkCellsAndAllContent( SwRowFrm& rRow )
{
    SwCellFrm* pCurrMasterCell = static_cast<SwCellFrm*>(rRow.Lower());
    SWRECTFN( pCurrMasterCell )

    while ( pCurrMasterCell )
    {
        // NEW TABLES
        SwCellFrm& rToAdjust = pCurrMasterCell->GetTabBox()->getRowSpan() < 1 ?
                               const_cast<SwCellFrm&>(pCurrMasterCell->FindStartEndOfRowSpanCell( true, true )) :
                               *pCurrMasterCell;

        // #i26945#
        // all lowers should have the correct position
        lcl_ArrangeLowers( &rToAdjust,
                           (rToAdjust.*fnRect->fnGetPrtTop)(),
                           false );
        // TODO: Optimize number of frames which are set to 0 height
        // we have to start with the last lower frame, otherwise
        // the shrink will not shrink the current cell
        SwFrm* pTmp = rToAdjust.GetLastLower();

        if ( pTmp && pTmp->IsRowFrm() )
        {
            SwRowFrm* pTmpRow = static_cast<SwRowFrm*>(pTmp);
            lcl_ShrinkCellsAndAllContent( *pTmpRow );
        }
        else
        {
            // TODO: Optimize number of frames which are set to 0 height
            while ( pTmp )
            {
                // the frames have to be shrunk
                if ( pTmp->IsTabFrm() )
                {
                    SwRowFrm* pTmpRow = static_cast<SwRowFrm*>(static_cast<SwTabFrm*>(pTmp)->Lower());
                    while ( pTmpRow )
                    {
                        lcl_ShrinkCellsAndAllContent( *pTmpRow );
                        pTmpRow = static_cast<SwRowFrm*>(pTmpRow->GetNext());
                    }
                }
                else
                {
                    pTmp->Shrink( (pTmp->Frm().*fnRect->fnGetHeight)() );
                    (pTmp->Prt().*fnRect->fnSetTop)( 0 );
                    (pTmp->Prt().*fnRect->fnSetHeight)( 0 );
                }

                pTmp = pTmp->GetPrev();
            }

            // all lowers should have the correct position
            lcl_ArrangeLowers( &rToAdjust,
                               (rToAdjust.*fnRect->fnGetPrtTop)(),
                               false );
        }

        pCurrMasterCell = static_cast<SwCellFrm*>(pCurrMasterCell->GetNext());
    }
}

// Local helper function to move the content from rSourceLine to rDestLine
// The content is inserted behind the last content in the corresponding
// cell in rDestLine.
static void lcl_MoveRowContent( SwRowFrm& rSourceLine, SwRowFrm& rDestLine )
{
    SwCellFrm* pCurrDestCell = static_cast<SwCellFrm*>(rDestLine.Lower());
    SwCellFrm* pCurrSourceCell = static_cast<SwCellFrm*>(rSourceLine.Lower());

    // Move content of follow cells into master cells
    while ( pCurrSourceCell )
    {
        if ( pCurrSourceCell->Lower() && pCurrSourceCell->Lower()->IsRowFrm() )
        {
            SwRowFrm* pTmpSourceRow = static_cast<SwRowFrm*>(pCurrSourceCell->Lower());
            while ( pTmpSourceRow )
            {
                // #125926# Achtung! It is possible,
                // that pTmpSourceRow->IsFollowFlowRow() but pTmpDestRow
                // cannot be found. In this case, we have to move the complete
                // row.
                SwRowFrm* pTmpDestRow = static_cast<SwRowFrm*>(pCurrDestCell->Lower());

                if ( pTmpSourceRow->IsFollowFlowRow() && pTmpDestRow )
                {
                    // move content from follow flow row to pTmpDestRow:
                    while ( pTmpDestRow->GetNext() )
                        pTmpDestRow = static_cast<SwRowFrm*>(pTmpDestRow->GetNext());

                    OSL_ENSURE( pTmpDestRow->GetFollowRow() == pTmpSourceRow, "Table contains node" );

                    lcl_MoveRowContent( *pTmpSourceRow, *pTmpDestRow );
                    pTmpDestRow->SetFollowRow( pTmpSourceRow->GetFollowRow() );
                    pTmpSourceRow->RemoveFromLayout();
                    SwFrm::DestroyFrm(pTmpSourceRow);
                }
                else
                {
                    // move complete row:
                    pTmpSourceRow->RemoveFromLayout();
                    pTmpSourceRow->InsertBefore( pCurrDestCell, 0 );
                }

                pTmpSourceRow = static_cast<SwRowFrm*>(pCurrSourceCell->Lower());
            }
        }
        else
        {
            SwFrm *pTmp = ::SaveContent( pCurrSourceCell );
            if ( pTmp )
            {
                // NEW TABLES
                SwCellFrm* pDestCell = static_cast<SwCellFrm*>(pCurrDestCell);
                if ( pDestCell->GetTabBox()->getRowSpan() < 1 )
                    pDestCell = & const_cast<SwCellFrm&>(pDestCell->FindStartEndOfRowSpanCell( true, true ));

                // Find last content
                SwFrm* pFrm = pDestCell->GetLastLower();
                ::RestoreContent( pTmp, pDestCell, pFrm, true );
            }
        }
        pCurrDestCell = static_cast<SwCellFrm*>(pCurrDestCell->GetNext());
        pCurrSourceCell = static_cast<SwCellFrm*>(pCurrSourceCell->GetNext());
    }
}

// Local helper function to move all footnotes in rRowFrm from
// the footnote boss of rSource to the footnote boss of rDest.
static void lcl_MoveFootnotes( SwTabFrm& rSource, SwTabFrm& rDest, SwLayoutFrm& rRowFrm )
{
    if ( !rSource.GetFormat()->GetDoc()->GetFootnoteIdxs().empty() )
    {
        SwFootnoteBossFrm* pOldBoss = rSource.FindFootnoteBossFrm( true );
        SwFootnoteBossFrm* pNewBoss = rDest.FindFootnoteBossFrm( true );
        rRowFrm.MoveLowerFootnotes( 0, pOldBoss, pNewBoss, true );
    }
}

// Local helper function to handle nested table cells before the split process
static void lcl_PreprocessRowsInCells( SwTabFrm& rTab, SwRowFrm& rLastLine,
                                SwRowFrm& rFollowFlowLine, SwTwips nRemain )
{
    SwCellFrm* pCurrLastLineCell = static_cast<SwCellFrm*>(rLastLine.Lower());
    SwCellFrm* pCurrFollowFlowLineCell = static_cast<SwCellFrm*>(rFollowFlowLine.Lower());

    SWRECTFN( pCurrLastLineCell )

    // Move content of follow cells into master cells
    while ( pCurrLastLineCell )
    {
        if ( pCurrLastLineCell->Lower() && pCurrLastLineCell->Lower()->IsRowFrm() )
        {
            SwTwips nTmpCut = nRemain;
            SwRowFrm* pTmpLastLineRow = static_cast<SwRowFrm*>(pCurrLastLineCell->Lower());

            // #i26945#
            SwTwips nCurrentHeight =
                    lcl_CalcMinRowHeight( pTmpLastLineRow,
                                          rTab.IsConsiderObjsForMinCellHeight() );
            while ( pTmpLastLineRow->GetNext() && nTmpCut > nCurrentHeight )
            {
                nTmpCut -= nCurrentHeight;
                pTmpLastLineRow = static_cast<SwRowFrm*>(pTmpLastLineRow->GetNext());
                // #i26945#
                nCurrentHeight =
                    lcl_CalcMinRowHeight( pTmpLastLineRow,
                                          rTab.IsConsiderObjsForMinCellHeight() );
            }

            // pTmpLastLineRow does not fit to the line or it is the last line
            // Check if we can move pTmpLastLineRow to the follow table,
            // or if we have to split the line:
            SwFrm* pCell = pTmpLastLineRow->Lower();
            bool bTableLayoutToComplex = false;
            long nMinHeight = 0;

            // We have to take into account:
            // 1. The fixed height of the row
            // 2. The borders of the cells inside the row
            // 3. The minimum height of the row
            if ( pTmpLastLineRow->HasFixSize() )
                nMinHeight = (pTmpLastLineRow->Frm().*fnRect->fnGetHeight)();
            else
            {
                while ( pCell )
                {
                    if ( static_cast<SwCellFrm*>(pCell)->Lower() &&
                         static_cast<SwCellFrm*>(pCell)->Lower()->IsRowFrm() )
                    {
                        bTableLayoutToComplex = true;
                        break;
                    }

                    SwBorderAttrAccess aAccess( SwFrm::GetCache(), pCell );
                    const SwBorderAttrs &rAttrs = *aAccess.Get();
                    nMinHeight = std::max( nMinHeight, lcl_CalcTopAndBottomMargin( *static_cast<SwLayoutFrm*>(pCell), rAttrs ) );
                    pCell = pCell->GetNext();
                }

                const SwFormatFrmSize &rSz = pTmpLastLineRow->GetFormat()->GetFrmSize();
                if ( rSz.GetHeightSizeType() == ATT_MIN_SIZE )
                    nMinHeight = std::max( nMinHeight, rSz.GetHeight() );
            }

            // 1. Case:
            // The line completely fits into the master table.
            // Nevertheless, we build a follow (otherwise painting problems
            // with empty cell).

            // 2. Case:
            // The line has to be split, the minimum height still fits into
            // the master table, and the table structure is not to complex.
            if ( nTmpCut > nCurrentHeight ||
                 ( pTmpLastLineRow->IsRowSplitAllowed() &&
                  !bTableLayoutToComplex && nMinHeight < nTmpCut ) )
            {
                // The line has to be split:
                SwRowFrm* pNewRow = new SwRowFrm( *pTmpLastLineRow->GetTabLine(), &rTab, false );
                pNewRow->SetFollowFlowRow( true );
                pNewRow->SetFollowRow( pTmpLastLineRow->GetFollowRow() );
                pTmpLastLineRow->SetFollowRow( pNewRow );
                pNewRow->InsertBehind( pCurrFollowFlowLineCell, 0 );
                pTmpLastLineRow = static_cast<SwRowFrm*>(pTmpLastLineRow->GetNext());
            }

            // The following lines have to be moved:
            while ( pTmpLastLineRow )
            {
                SwRowFrm* pTmp = static_cast<SwRowFrm*>(pTmpLastLineRow->GetNext());
                lcl_MoveFootnotes( rTab, *rTab.GetFollow(), *pTmpLastLineRow );
                pTmpLastLineRow->RemoveFromLayout();
                pTmpLastLineRow->InsertBefore( pCurrFollowFlowLineCell, 0 );
                pTmpLastLineRow->Shrink( ( pTmpLastLineRow->Frm().*fnRect->fnGetHeight)() );
                pCurrFollowFlowLineCell->Grow( ( pTmpLastLineRow->Frm().*fnRect->fnGetHeight)() );
                pTmpLastLineRow = pTmp;
            }
        }

        pCurrLastLineCell = static_cast<SwCellFrm*>(pCurrLastLineCell->GetNext());
        pCurrFollowFlowLineCell = static_cast<SwCellFrm*>(pCurrFollowFlowLineCell->GetNext());
    }
}

// Local helper function to handle nested table cells after the split process
static void lcl_PostprocessRowsInCells( SwTabFrm& rTab, SwRowFrm& rLastLine )
{
    SwCellFrm* pCurrMasterCell = static_cast<SwCellFrm*>(rLastLine.Lower());
    while ( pCurrMasterCell )
    {
        if ( pCurrMasterCell->Lower() &&
             pCurrMasterCell->Lower()->IsRowFrm() )
        {
            SwRowFrm* pRowFrm = static_cast<SwRowFrm*>(pCurrMasterCell->GetLastLower());

            if ( NULL != pRowFrm->GetPrev() && !pRowFrm->ContainsContent() )
            {
                OSL_ENSURE( pRowFrm->GetFollowRow(), "Deleting row frame without follow" );

                // The footnotes have to be moved:
                lcl_MoveFootnotes( rTab, *rTab.GetFollow(), *pRowFrm );
                pRowFrm->Cut();
                SwRowFrm* pFollowRow = pRowFrm->GetFollowRow();
                pRowFrm->Paste( pFollowRow->GetUpper(), pFollowRow );
                pRowFrm->SetFollowRow( pFollowRow->GetFollowRow() );
                lcl_MoveRowContent( *pFollowRow, *pRowFrm );
                pFollowRow->Cut();
                SwFrm::DestroyFrm(pFollowRow);
                ::SwInvalidateAll( pCurrMasterCell, LONG_MAX );
            }
        }

        pCurrMasterCell = static_cast<SwCellFrm*>(pCurrMasterCell->GetNext());
    }
}

// Local helper function to re-calculate the split line.
inline void TableSplitRecalcLock( SwFlowFrm *pTab ) { pTab->LockJoin(); }
inline void TableSplitRecalcUnlock( SwFlowFrm *pTab ) { pTab->UnlockJoin(); }

static bool lcl_RecalcSplitLine( SwRowFrm& rLastLine, SwRowFrm& rFollowLine,
                          SwTwips nRemainingSpaceForLastRow )
{
    bool bRet = true;

    vcl::RenderContext* pRenderContext = rLastLine.getRootFrm()->GetCurrShell()->GetOut();
    SwTabFrm& rTab = static_cast<SwTabFrm&>(*rLastLine.GetUpper());

    // If there are nested cells in rLastLine, the recalculation of the last
    // line needs some preprocessing.
    lcl_PreprocessRowsInCells( rTab, rLastLine, rFollowLine, nRemainingSpaceForLastRow );

    // Here the recalculation process starts:
    rTab.SetRebuildLastLine( true );
    // #i26945#
    rTab.SetDoesObjsFit( true );
    SWRECTFN( rTab.GetUpper() )

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

    // Lock this tab frame and its follow
    bool bUnlockMaster = false;
    bool bUnlockFollow = false;
    SwTabFrm* pMaster = rTab.IsFollow() ? rTab.FindMaster() : 0;
    if ( pMaster && !pMaster->IsJoinLocked() )
    {
        bUnlockMaster = true;
        ::TableSplitRecalcLock( pMaster );
    }
    if ( !rTab.GetFollow()->IsJoinLocked() )
    {
        bUnlockFollow = true;
        ::TableSplitRecalcLock( rTab.GetFollow() );
    }

    // Do the recalculation
    lcl_RecalcRow( rLastLine, LONG_MAX );
    // #115759# - force a format of the last line in order to
    // get the correct height.
    rLastLine.InvalidateSize();
    rLastLine.Calc(pRenderContext);

    // Unlock this tab frame and its follow
    if ( bUnlockFollow )
        ::TableSplitRecalcUnlock( rTab.GetFollow() );
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
            (rTab.Frm().*fnRect->fnBottomDist)( (rTab.GetUpper()->*fnRect->fnGetPrtBottom)());
    if ( nDistanceToUpperPrtBottom < 0 || !rTab.DoesObjsFit() )
        bRet = false;

    // 2. Check if each cell in the last line has at least one content frame.

    // Note: a FollowFlowRow may contains empty cells!
    if ( bRet )
    {
        if ( !rLastLine.IsInFollowFlowRow() )
        {
            SwCellFrm* pCurrMasterCell = static_cast<SwCellFrm*>(rLastLine.Lower());
            while ( pCurrMasterCell )
            {
                if ( !pCurrMasterCell->ContainsContent() && pCurrMasterCell->GetTabBox()->getRowSpan() >= 1 )
                {
                    bRet = false;
                    break;
                }
                pCurrMasterCell = static_cast<SwCellFrm*>(pCurrMasterCell->GetNext());
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
static void lcl_AdjustRowSpanCells( SwRowFrm* pRow )
{
    SWRECTFN( pRow )
    SwCellFrm* pCellFrm = static_cast<SwCellFrm*>(pRow->GetLower());
    while ( pCellFrm )
    {
        const long nLayoutRowSpan = pCellFrm->GetLayoutRowSpan();
        if ( nLayoutRowSpan > 1 )
        {
            // calculate height of cell:
            const long nNewCellHeight = lcl_GetHeightOfRows( pRow, nLayoutRowSpan );
            const long nDiff = nNewCellHeight - (pCellFrm->Frm().*fnRect->fnGetHeight)();
            if ( nDiff )
                (pCellFrm->Frm().*fnRect->fnAddBottom)( nDiff );
        }

        pCellFrm = static_cast<SwCellFrm*>(pCellFrm->GetNext());
    }
}

// Returns the maximum layout row span of the row
// Looking for the next row that contains no covered cells:
static long lcl_GetMaximumLayoutRowSpan( const SwRowFrm& rRow )
{
    long nRet = 1;

    const SwRowFrm* pCurrentRowFrm = static_cast<const SwRowFrm*>(rRow.GetNext());
    bool bNextRow = false;

    while ( pCurrentRowFrm )
    {
        // if there is any covered cell, we proceed to the next row frame
        const SwCellFrm* pLower = static_cast<const SwCellFrm*>( pCurrentRowFrm->Lower());
        while ( pLower )
        {
            if ( pLower->GetTabBox()->getRowSpan() < 0 )
            {
                ++nRet;
                bNextRow = true;
                break;
            }
            pLower = static_cast<const SwCellFrm*>(pLower->GetNext());
        }
        pCurrentRowFrm = bNextRow ?
                         static_cast<const SwRowFrm*>(pCurrentRowFrm->GetNext() ) :
                         0;
    }

    return nRet;
}

// Function to remove the FollowFlowLine of rTab.
// The content of the FollowFlowLine is moved to the associated line in the
// master table.
bool SwTabFrm::RemoveFollowFlowLine()
{
    // find FollowFlowLine
    SwRowFrm* pFollowFlowLine = static_cast<SwRowFrm*>(GetFollow()->GetFirstNonHeadlineRow());

    // find last row in master
    SwFrm* pLastLine = GetLastLower();

    OSL_ENSURE( HasFollowFlowLine() &&
            pFollowFlowLine &&
            pLastLine, "There should be a flowline in the follow" );

    // We have to reset the flag here, because lcl_MoveRowContent
    // calls a GrowFrm(), which has a different bahavior if
    // this flag is set.
    SetFollowFlowLine( false );

    // #140081# Make code robust.
    if ( !pFollowFlowLine || !pLastLine )
        return true;

    // Move content
    lcl_MoveRowContent( *pFollowFlowLine, *static_cast<SwRowFrm*>(pLastLine) );

    // NEW TABLES
    // If a row span follow flow line is removed, we want to move the whole span
    // to the master:
    long nRowsToMove = lcl_GetMaximumLayoutRowSpan( *pFollowFlowLine );

    if ( nRowsToMove > 1 )
    {
        SWRECTFN( this )
        SwFrm* pRow = pFollowFlowLine->GetNext();
        SwFrm* pInsertBehind = GetLastLower();
        SwTwips nGrow = 0;

        while ( pRow && nRowsToMove-- > 1 )
        {
            SwFrm* pNxt = pRow->GetNext();
            nGrow += (pRow->Frm().*fnRect->fnGetHeight)();

            // The footnotes have to be moved:
            lcl_MoveFootnotes( *GetFollow(), *this, static_cast<SwRowFrm&>(*pRow) );

            pRow->RemoveFromLayout();
            pRow->InsertBehind( this, pInsertBehind );
            pRow->_InvalidateAll();
            pRow->CheckDirChange();
            pInsertBehind = pRow;
            pRow = pNxt;
        }

        SwFrm* pFirstRow = Lower();
        while ( pFirstRow )
        {
            lcl_AdjustRowSpanCells( static_cast<SwRowFrm*>(pFirstRow) );
            pFirstRow = pFirstRow->GetNext();
        }

        Grow( nGrow );
        GetFollow()->Shrink( nGrow );
    }

    bool bJoin = !pFollowFlowLine->GetNext();
    pFollowFlowLine->Cut();
    SwFrm::DestroyFrm(pFollowFlowLine);

    return bJoin;
}

// #i26945# - Floating screen objects are no longer searched.
static bool lcl_FindSectionsInRow( const SwRowFrm& rRow )
{
    bool bRet = false;
    const SwCellFrm* pLower = static_cast<const SwCellFrm*>(rRow.Lower());
    while ( pLower )
    {
        if ( pLower->IsVertical() != rRow.IsVertical() )
            return true;

        const SwFrm* pTmpFrm = pLower->Lower();
        while ( pTmpFrm )
        {
            if ( pTmpFrm->IsRowFrm() )
            {
                bRet = lcl_FindSectionsInRow( *static_cast<const SwRowFrm*>(pTmpFrm) );
            }
            else
            {
                // #i26945# - search only for sections
                bRet = pTmpFrm->IsSctFrm();
            }

            if ( bRet )
                return true;
            pTmpFrm = pTmpFrm->GetNext();
        }

        pLower = static_cast<const SwCellFrm*>(pLower->GetNext());
    }
    return bRet;
}

bool SwTabFrm::Split( const SwTwips nCutPos, bool bTryToSplit, bool bTableRowKeep )
{
    bool bRet = true;

    SWRECTFN( this )

    // #i26745# - format row and cell frames of table
    {
        this->Lower()->_InvalidatePos();
        // #i43913# - correction
        // call method <lcl_InnerCalcLayout> with first lower.
        lcl_InnerCalcLayout( this->Lower(), LONG_MAX, true );
    }

    //In order to be able to compare the positions of the cells whit CutPos,
    //they have to be calculated consecutively starting from the table.
    //They can definitely be invalid because of position changes of the table.
    SwRowFrm *pRow = static_cast<SwRowFrm*>(Lower());
    if( !pRow )
        return bRet;

    const sal_uInt16 nRepeat = GetTable()->GetRowsToRepeat();
    sal_uInt16 nRowCount = 0;           // pRow currently points to the first row

    SwTwips nRemainingSpaceForLastRow =
        (*fnRect->fnYDiff)( nCutPos, (Frm().*fnRect->fnGetTop)() );
    nRemainingSpaceForLastRow -= (this->*fnRect->fnGetTopMargin)();

    // Make pRow point to the line that does not fit anymore:
    while( pRow->GetNext() &&
           nRemainingSpaceForLastRow >= ( (pRow->Frm().*fnRect->fnGetHeight)() +
                                           (IsCollapsingBorders() ?
                                            pRow->GetBottomLineSize() :
                                            0 ) ) )
    {
        if( bTryToSplit || !pRow->IsRowSpanLine() ||
            0 != (pRow->Frm().*fnRect->fnGetHeight)() )
            ++nRowCount;
        nRemainingSpaceForLastRow -= (pRow->Frm().*fnRect->fnGetHeight)();
        pRow = static_cast<SwRowFrm*>(pRow->GetNext());
    }

    // bSplitRowAllowed: Row may be split according to its attributes.
    // bTryToSplit:      Row will never be split if bTryToSplit = false.
    //                   This can either be passed as a parameter, indicating
    //                   that we are currently doing the second try to split the
    //                   table, or it will be set to falseunder certain
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
            SwCellFrm* pLowerCell = static_cast<SwCellFrm*>(pRow->Lower());
            while ( pLowerCell )
            {
                if ( pLowerCell->Lower() && pLowerCell->Lower()->IsRowFrm() )
                {
                    const SwRowFrm* pLowerRow = static_cast<SwRowFrm*>(pLowerCell->Lower());
                    if ( !pLowerRow->IsRowSplitAllowed() &&
                        (pLowerRow->Frm().*fnRect->fnGetHeight)() >
                        nRemainingSpaceForLastRow )
                    {
                        bKeepNextRow = true;
                        break;
                    }
                }
                pLowerCell = static_cast<SwCellFrm*>(pLowerCell->GetNext());
            }
        }
        else
            bKeepNextRow = true;
    }

    // Better keep the next row in this table:
    if ( bKeepNextRow )
    {
        pRow = GetFirstNonHeadlineRow();
        if( pRow && pRow->IsRowSpanLine() && 0 == (pRow->Frm().*fnRect->fnGetHeight)() )
            pRow = static_cast<SwRowFrm*>(pRow->GetNext());
        if ( pRow )
        {
            pRow = static_cast<SwRowFrm*>(pRow->GetNext());
            ++nRowCount;
        }
    }

    // No more row to split or to move to follow table:
    if ( !pRow )
        return bRet;

    // We try to split the row if
    // - the attributes of the row are set accordingly and
    // - we are allowed to do so
    // - the it should not keep with the next row
    bSplitRowAllowed = bSplitRowAllowed && bTryToSplit &&
                       ( !bTableRowKeep ||
                         !pRow->ShouldRowKeepWithNext() );

    // Adjust pRow according to the keep-with-next attribute:
    if ( !bSplitRowAllowed && bTableRowKeep )
    {
        SwRowFrm* pTmpRow = static_cast<SwRowFrm*>(pRow->GetPrev());
        SwRowFrm* pOldRow = pRow;
        while ( pTmpRow && pTmpRow->ShouldRowKeepWithNext() &&
                nRowCount > nRepeat )
        {
            pRow = pTmpRow;
            --nRowCount;
            pTmpRow = static_cast<SwRowFrm*>(pTmpRow->GetPrev());
        }

        // loop prevention
        if ( nRowCount == nRepeat && !GetIndPrev())
        {
            pRow = pOldRow;
        }
    }

    // If we do not indent to split pRow, we check if we are
    // allowed to move pRow to a follow. Otherwise we return
    // false, indicating an error
    if ( !bSplitRowAllowed )
    {
        SwRowFrm* pFirstNonHeadlineRow = GetFirstNonHeadlineRow();
        if ( pRow == pFirstNonHeadlineRow )
            return false;

        // #i91764#
        // Ignore row span lines
        SwRowFrm* pTmpRow = pFirstNonHeadlineRow;
        while ( pTmpRow && pTmpRow->IsRowSpanLine() )
        {
            pTmpRow = static_cast<SwRowFrm*>(pTmpRow->GetNext());
        }
        if ( !pTmpRow || pRow == pTmpRow )
        {
            return false;
        }
    }

    // Build follow table if not already done:
    bool bNewFollow;
    SwTabFrm *pFoll;
    if ( GetFollow() )
    {
        pFoll = GetFollow();
        bNewFollow = false;
    }
    else
    {
        bNewFollow = true;
        pFoll = new SwTabFrm( *this );

        // We give the follow table an initial width.
        (pFoll->Frm().*fnRect->fnAddWidth)( (Frm().*fnRect->fnGetWidth)() );
        (pFoll->Prt().*fnRect->fnAddWidth)( (Prt().*fnRect->fnGetWidth)() );
        (pFoll->Frm().*fnRect->fnSetLeft)( (Frm().*fnRect->fnGetLeft)() );

        // Insert the new follow table
        pFoll->InsertBehind( GetUpper(), this );

        // Repeat the headlines.
        for ( nRowCount = 0; nRowCount < nRepeat; ++nRowCount )
        {
            // Insert new headlines:
            bDontCreateObjects = true;              //frmtool
            SwRowFrm* pHeadline = new SwRowFrm(
                                    *GetTable()->GetTabLines()[ nRowCount ], this );
            pHeadline->SetRepeatedHeadline( true );
            bDontCreateObjects = false;
            pHeadline->InsertBefore( pFoll, 0 );

            SwPageFrm *pPage = pHeadline->FindPageFrm();
            const SwFrameFormats *pTable = GetFormat()->GetDoc()->GetSpzFrameFormats();
            if( !pTable->empty() )
            {
                sal_uLong nIndex;
                SwContentFrm* pFrm = pHeadline->ContainsContent();
                while( pFrm )
                {
                    nIndex = pFrm->GetNode()->GetIndex();
                    AppendObjs( pTable, nIndex, pFrm, pPage, GetFormat()->GetDoc());
                    pFrm = pFrm->GetNextContentFrm();
                    if( !pHeadline->IsAnLower( pFrm ) )
                        break;
                }
            }
        }
    }

    SwRowFrm* pLastRow = 0;     // will point to the last remaining line in master
    SwRowFrm* pFollowRow = 0;   // points to either the follow flow line of the
                                // first regular line in the follow

    if ( bSplitRowAllowed )
    {
        // If the row that does not fit anymore is allowed
        // to be split, the next row has to be moved to the follow table.
        pLastRow = pRow;
        pRow = static_cast<SwRowFrm*>(pRow->GetNext());

        // new follow flow line for last row of master table
        pFollowRow = lcl_InsertNewFollowFlowLine( *this, *pLastRow, false );
    }
    else
    {
        pFollowRow = pRow;

        // NEW TABLES
        // check if we will break a row span by moving pFollowRow to the follow:
        // In this case we want to reformat the last line.
        const SwCellFrm* pCellFrm = static_cast<const SwCellFrm*>(pFollowRow->GetLower());
        while ( pCellFrm )
        {
            if ( pCellFrm->GetTabBox()->getRowSpan() < 1 )
            {
                pLastRow = static_cast<SwRowFrm*>(pRow->GetPrev());
                break;
            }

            pCellFrm = static_cast<const SwCellFrm*>(pCellFrm->GetNext());
        }

        // new follow flow line for last row of master table
        if ( pLastRow )
            pFollowRow = lcl_InsertNewFollowFlowLine( *this, *pLastRow, true );
    }

    SwTwips nRet = 0;

    //Optimization: There is no paste needed for the new Follow and the
    //optimized insert can be used (big amounts of rows luckily only occurs in
    //such situations).
    if ( bNewFollow )
    {
        SwFrm* pInsertBehind = pFoll->GetLastLower();

        while ( pRow )
        {
            SwFrm* pNxt = pRow->GetNext();
            nRet += (pRow->Frm().*fnRect->fnGetHeight)();
            // The footnotes do not have to be moved, this is done in the
            // MoveFwd of the follow table!!!
            pRow->RemoveFromLayout();
            pRow->InsertBehind( pFoll, pInsertBehind );
            pRow->_InvalidateAll();
            pInsertBehind = pRow;
            pRow = static_cast<SwRowFrm*>(pNxt);
        }
    }
    else
    {
        SwFrm* pPasteBefore = HasFollowFlowLine() ?
                              pFollowRow->GetNext() :
                              pFoll->GetFirstNonHeadlineRow();

        while ( pRow )
        {
            SwFrm* pNxt = pRow->GetNext();
            nRet += (pRow->Frm().*fnRect->fnGetHeight)();

            // The footnotes have to be moved:
            lcl_MoveFootnotes( *this, *GetFollow(), *pRow );

            pRow->RemoveFromLayout();
            pRow->Paste( pFoll, pPasteBefore );

            pRow->CheckDirChange();
            pRow = static_cast<SwRowFrm*>(pNxt);
        }
    }

    Shrink( nRet );

    // we rebuild the last line to assure that it will be fully formatted
    if ( pLastRow )
    {
        // recalculate the split line
        bRet = lcl_RecalcSplitLine( *pLastRow, *pFollowRow, nRemainingSpaceForLastRow );

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

bool SwTabFrm::Join()
{
    OSL_ENSURE( !HasFollowFlowLine(), "Joining follow flow line" );

    SwTabFrm *pFoll = GetFollow();

    if ( !pFoll->IsJoinLocked() )
    {
        SWRECTFN( this )
        pFoll->Cut();   //Cut out first to avoid unnecessary notifications.

        SwFrm *pRow = pFoll->GetFirstNonHeadlineRow(),
              *pNxt;

        SwFrm* pPrv = GetLastLower();

        SwTwips nHeight = 0;    //Total height of the inserted rows as return value.

        while ( pRow )
        {
            pNxt = pRow->GetNext();
            nHeight += (pRow->Frm().*fnRect->fnGetHeight)();
            pRow->RemoveFromLayout();
            pRow->_InvalidateAll();
            pRow->InsertBehind( this, pPrv );
            pRow->CheckDirChange();
            pPrv = pRow;
            pRow = pNxt;
        }

        SetFollow( pFoll->GetFollow() );
        SetFollowFlowLine( pFoll->HasFollowFlowLine() );
        SwFrm::DestroyFrm(pFoll);

        Grow( nHeight );
    }

    return true;
}

void SwInvalidatePositions( SwFrm *pFrm, long nBottom )
{
    // LONG_MAX == nBottom means we have to calculate all
    bool bAll = LONG_MAX == nBottom;
    SWRECTFN( pFrm )
    do
    {   pFrm->_InvalidatePos();
        pFrm->_InvalidateSize();
        if( pFrm->IsLayoutFrm() )
        {
            if ( static_cast<SwLayoutFrm*>(pFrm)->Lower() )
            {
                ::SwInvalidatePositions( static_cast<SwLayoutFrm*>(pFrm)->Lower(), nBottom);
                // #i26945#
                ::lcl_InvalidateLowerObjs( *(static_cast<SwLayoutFrm*>(pFrm)) );
            }
        }
        else
            pFrm->Prepare( PREP_ADJUST_FRM );
        pFrm = pFrm->GetNext();
    } while ( pFrm &&
              ( bAll ||
              (*fnRect->fnYDiff)( (pFrm->Frm().*fnRect->fnGetTop)(), nBottom ) < 0 ) );
}

void SwInvalidateAll( SwFrm *pFrm, long nBottom )
{
    // LONG_MAX == nBottom means we have to calculate all
    bool bAll = LONG_MAX == nBottom;
    SWRECTFN( pFrm )
    do
    {
        pFrm->_InvalidatePos();
        pFrm->_InvalidateSize();
        pFrm->_InvalidatePrt();
        if( pFrm->IsLayoutFrm() )
        {
            // NEW TABLES
            SwLayoutFrm* pToInvalidate = static_cast<SwLayoutFrm*>(pFrm);
            SwCellFrm* pThisCell = dynamic_cast<SwCellFrm*>(pFrm);
            if ( pThisCell && pThisCell->GetTabBox()->getRowSpan() < 1 )
            {
                pToInvalidate = & const_cast<SwCellFrm&>(pThisCell->FindStartEndOfRowSpanCell( true, true ));
                pToInvalidate->_InvalidatePos();
                pToInvalidate->_InvalidateSize();
                pToInvalidate->_InvalidatePrt();
            }

            if ( pToInvalidate->Lower() )
                ::SwInvalidateAll( pToInvalidate->Lower(), nBottom);
        }
        else
            pFrm->Prepare();

        pFrm = pFrm->GetNext();
    } while ( pFrm &&
              ( bAll ||
              (*fnRect->fnYDiff)( (pFrm->Frm().*fnRect->fnGetTop)(), nBottom ) < 0 ) );
}

// #i29550#
static void lcl_InvalidateAllLowersPrt( SwLayoutFrm* pLayFrm )
{
    pLayFrm->_InvalidatePrt();
    pLayFrm->_InvalidateSize();
    pLayFrm->SetCompletePaint();

    SwFrm* pFrm = pLayFrm->Lower();

    while ( pFrm )
    {
        if ( pFrm->IsLayoutFrm() )
            lcl_InvalidateAllLowersPrt( static_cast<SwLayoutFrm*>(pFrm) );
        else
        {
            pFrm->_InvalidatePrt();
            pFrm->_InvalidateSize();
            pFrm->SetCompletePaint();
        }

        pFrm = pFrm->GetNext();
    }
}

bool SwContentFrm::CalcLowers( SwLayoutFrm* pLay, const SwLayoutFrm* pDontLeave,
                                 long nBottom, bool bSkipRowSpanCells )
{
    if ( !pLay )
        return true;

    vcl::RenderContext* pRenderContext = pLay->getRootFrm()->GetCurrShell()->GetOut();
    // LONG_MAX == nBottom means we have to calculate all
    bool bAll = LONG_MAX == nBottom;
    bool bRet = false;
    SwContentFrm *pCnt = pLay->ContainsContent();
    SWRECTFN( pLay )

    // FME 2007-08-30 #i81146# new loop control
    int nLoopControlRuns = 0;
    const int nLoopControlMax = 10;
    const SwModify* pLoopControlCond = 0;

    while ( pCnt && pDontLeave->IsAnLower( pCnt ) )
    {
        // #115759# - check, if a format of content frame is
        // possible. Thus, 'copy' conditions, found at the beginning of
        // <SwContentFrm::MakeAll(..)>, and check these.
        const bool bFormatPossible = !pCnt->IsJoinLocked() &&
                                     ( !pCnt->IsTextFrm() ||
                                       !static_cast<SwTextFrm*>(pCnt)->IsLocked() ) &&
                                     ( pCnt->IsFollow() || !StackHack::IsLocked() );

        // NEW TABLES
        bool bSkipContent = false;
        if ( bSkipRowSpanCells && pCnt->IsInTab() )
        {
            const SwFrm* pCell = pCnt->GetUpper();
            while ( pCell && !pCell->IsCellFrm() )
                pCell = pCell->GetUpper();
            if ( pCell && 1 != static_cast<const SwCellFrm*>( pCell )->GetLayoutRowSpan() )
                bSkipContent = true;
        }

        if ( bFormatPossible && !bSkipContent )
        {
            bRet |= !pCnt->IsValid();
            // #i26945# - no extra invalidation of floating
            // screen objects needed.
            // Thus, delete call of method <SwFrm::InvalidateObjs( true )>
            pCnt->Calc(pRenderContext);
            // OD 2004-05-11 #i28701# - usage of new method <::FormatObjsAtFrm(..)>
            // to format the floating screen objects
            // #i46941# - frame has to be valid
            // Note: frame could be invalid after calling its format, if it's locked.
            OSL_ENSURE( !pCnt->IsTextFrm() ||
                    pCnt->IsValid() ||
                    static_cast<SwTextFrm*>(pCnt)->IsJoinLocked(),
                    "<SwContentFrm::CalcLowers(..)> - text frame invalid and not locked." );
            if ( pCnt->IsTextFrm() && pCnt->IsValid() )
            {
                // #i23129#, #i36347# - pass correct page frame to
                // the object formatter
                if ( !SwObjectFormatter::FormatObjsAtFrm( *pCnt,
                                                          *(pCnt->FindPageFrm()) ) )
                {
                    if ( pCnt->GetRegisteredIn() == pLoopControlCond )
                        ++nLoopControlRuns;
                    else
                    {
                        nLoopControlRuns = 0;
                        pLoopControlCond = pCnt->GetRegisteredIn();
                    }

                    if ( nLoopControlRuns < nLoopControlMax )
                    {
                        // restart format with first content
                        pCnt = pLay->ContainsContent();
                        continue;
                    }

#if OSL_DEBUG_LEVEL > 1
                    OSL_FAIL( "LoopControl in SwContentFrm::CalcLowers" );
#endif
                }
            }
            pCnt->GetUpper()->Calc(pRenderContext);
        }
        if( ! bAll && (*fnRect->fnYDiff)((pCnt->Frm().*fnRect->fnGetTop)(), nBottom) > 0 )
            break;
        pCnt = pCnt->GetNextContentFrm();
    }
    return bRet;
}

// #i26945# - add parameter <_bOnlyRowsAndCells> to control
// that only row and cell frames are formatted.
static bool lcl_InnerCalcLayout( SwFrm *pFrm,
                                      long nBottom,
                                      bool _bOnlyRowsAndCells )
{
    vcl::RenderContext* pRenderContext = pFrm->getRootFrm()->GetCurrShell() ? pFrm->getRootFrm()->GetCurrShell()->GetOut() : 0;
    // LONG_MAX == nBottom means we have to calculate all
    bool bAll = LONG_MAX == nBottom;
    bool bRet = false;
    const SwFrm* pOldUp = pFrm->GetUpper();
    SWRECTFN( pFrm )
    do
    {
        // #i26945# - parameter <_bOnlyRowsAndCells> controls,
        // if only row and cell frames are formatted.
        if ( pFrm->IsLayoutFrm() &&
             ( !_bOnlyRowsAndCells || pFrm->IsRowFrm() || pFrm->IsCellFrm() ) )
        {
            // #130744# An invalid locked table frame will
            // not be calculated => It will not become valid =>
            // Loop in lcl_RecalcRow(). Therefore we do not consider them for bRet.
            bRet |= !pFrm->IsValid() && ( !pFrm->IsTabFrm() || !static_cast<SwTabFrm*>(pFrm)->IsJoinLocked() );
            pFrm->Calc(pRenderContext);
            if( static_cast<SwLayoutFrm*>(pFrm)->Lower() )
                bRet |= lcl_InnerCalcLayout( static_cast<SwLayoutFrm*>(pFrm)->Lower(), nBottom);

            // NEW TABLES
            SwCellFrm* pThisCell = dynamic_cast<SwCellFrm*>(pFrm);
            if ( pThisCell && pThisCell->GetTabBox()->getRowSpan() < 1 )
            {
                SwCellFrm& rToCalc = const_cast<SwCellFrm&>(pThisCell->FindStartEndOfRowSpanCell( true, true ));
                bRet |= !rToCalc.IsValid();
                rToCalc.Calc(pRenderContext);
                if ( rToCalc.Lower() )
                    bRet |= lcl_InnerCalcLayout( rToCalc.Lower(), nBottom);
            }
        }
        pFrm = pFrm->GetNext();
    } while( pFrm &&
            ( bAll ||
              (*fnRect->fnYDiff)((pFrm->Frm().*fnRect->fnGetTop)(), nBottom) < 0 )
            && pFrm->GetUpper() == pOldUp );
    return bRet;
}

static void lcl_RecalcRow( SwRowFrm& rRow, long nBottom )
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

        while( lcl_InnerCalcLayout( &rRow, nBottom ) )
        {
            if ( ++nLoopControlRuns_2 > nLoopControlMax )
            {
#if OSL_DEBUG_LEVEL > 1
                OSL_ENSURE( 0 != nLoopControlStage_2, "LoopControl_2 in lcl_RecalcRow: Stage 1!" );
                OSL_ENSURE( 1 != nLoopControlStage_2, "LoopControl_2 in lcl_RecalcRow: Stage 2!!" );
                OSL_ENSURE( 2 >  nLoopControlStage_2, "LoopControl_2 in lcl_RecalcRow: Stage 3!!!" );
#endif
                rRow.ValidateThisAndAllLowers( nLoopControlStage_2++ );
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
            bCheck = SwContentFrm::CalcLowers( &rRow, rRow.GetUpper(), nBottom, true );

            // NEW TABLES
            // First we calculate the cells with row span of < 1, afterwards
            // all cells with row span of > 1:
            for ( int i = 0; i < 2; ++i )
            {
                SwCellFrm* pCellFrm = static_cast<SwCellFrm*>(rRow.Lower());
                while ( pCellFrm )
                {
                    const bool bCalc = 0 == i ?
                                       pCellFrm->GetLayoutRowSpan() < 1 :
                                       pCellFrm->GetLayoutRowSpan() > 1;

                    if ( bCalc )
                    {
                        SwCellFrm& rToRecalc = 0 == i ?
                                               const_cast<SwCellFrm&>(pCellFrm->FindStartEndOfRowSpanCell( true, true )) :
                                               *pCellFrm;
                        bCheck  |= SwContentFrm::CalcLowers( &rToRecalc, &rToRecalc, nBottom, false );
                    }

                    pCellFrm = static_cast<SwCellFrm*>(pCellFrm->GetNext());
                }
            }

            if ( bCheck )
            {
                if ( ++nLoopControlRuns_1 > nLoopControlMax )
                {
#if OSL_DEBUG_LEVEL > 1
                    OSL_ENSURE( 0 != nLoopControlStage_1, "LoopControl_1 in lcl_RecalcRow: Stage 1!" );
                    OSL_ENSURE( 1 != nLoopControlStage_1, "LoopControl_1 in lcl_RecalcRow: Stage 2!!" );
                    OSL_ENSURE( 2 >  nLoopControlStage_1, "LoopControl_1 in lcl_RecalcRow: Stage 3!!!" );
#endif
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

static void lcl_RecalcTable( SwTabFrm& rTab,
                                  SwLayoutFrm *pFirstRow,
                                  SwLayNotify &rNotify )
{
    if ( rTab.Lower() )
    {
        if ( !pFirstRow )
        {
            pFirstRow = static_cast<SwLayoutFrm*>(rTab.Lower());
            rNotify.SetLowersComplete( true );
        }
        ::SwInvalidatePositions( pFirstRow, LONG_MAX );
        lcl_RecalcRow( static_cast<SwRowFrm&>(*pFirstRow), LONG_MAX );
    }
}

// This is a new function to check the first condition whether
// a tab frame may move backward. It replaces the formerly used
// GetIndPrev(), which did not work correctly for #i5947#
static bool lcl_NoPrev( const SwFrm& rFrm )
{
    // #i79774#
    // skip empty sections on investigation of direct previous frame.
    // use information, that at least one empty section is skipped in the following code.
    bool bSkippedDirectPrevEmptySection( false );
    if ( rFrm.GetPrev() )
    {
        const SwFrm* pPrev( rFrm.GetPrev() );
        while ( pPrev &&
                pPrev->IsSctFrm() &&
                !dynamic_cast<const SwSectionFrm&>(*pPrev).GetSection() )
        {
            pPrev = pPrev->GetPrev();
            bSkippedDirectPrevEmptySection = true;
        }
        if ( pPrev )
        {
            return false;
        }
    }

    if ( ( !bSkippedDirectPrevEmptySection && !rFrm.GetIndPrev() ) ||
         ( bSkippedDirectPrevEmptySection &&
           ( !rFrm.IsInSct() || !rFrm._GetIndPrev() ) ) )
    {
        return true;
    }

    // I do not have a direct prev, but I have an indirect prev.
    // In section frames I have to check if I'm located inside
    // the first column:
    if ( rFrm.IsInSct() )
    {
        const SwFrm* pSct = rFrm.GetUpper();
        if ( pSct && pSct->IsColBodyFrm() &&
             pSct->GetUpper()->GetUpper()->IsSctFrm() )
        {
            const SwFrm* pPrevCol = rFrm.GetUpper()->GetUpper()->GetPrev();
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
SwFrm* sw_FormatNextContentForKeep( SwTabFrm* pTabFrm )
{
    vcl::RenderContext* pRenderContext = pTabFrm->getRootFrm()->GetCurrShell()->GetOut();
    // find next content, table or section
    SwFrm* pNxt = pTabFrm->FindNext();

    // skip empty sections
    while ( pNxt && pNxt->IsSctFrm() &&
            !static_cast<SwSectionFrm*>(pNxt)->GetSection() )
    {
        pNxt = pNxt->FindNext();
    }

    // if found next frame is a section, get its first content.
    if ( pNxt && pNxt->IsSctFrm() )
    {
        pNxt = static_cast<SwSectionFrm*>(pNxt)->ContainsAny();
    }

    // format found next frame.
    // if table frame is inside another table, method <SwFrm::MakeAll()> is
    // called to avoid that the superior table frame is formatted.
    if ( pNxt )
    {
        if ( pTabFrm->GetUpper()->IsInTab() )
            pNxt->MakeAll(pNxt->getRootFrm()->GetCurrShell()->GetOut());
        else
            pNxt->Calc(pRenderContext);
    }

    return pNxt;
}

namespace {
    bool AreAllRowsKeepWithNext( const SwRowFrm* pFirstRowFrm )
    {
        bool bRet = pFirstRowFrm != 0 &&
                    pFirstRowFrm->ShouldRowKeepWithNext();

        while ( bRet && pFirstRowFrm->GetNext() != 0 )
        {
            pFirstRowFrm = dynamic_cast<const SwRowFrm*>(pFirstRowFrm->GetNext());
            bRet = pFirstRowFrm != 0 &&
                   pFirstRowFrm->ShouldRowKeepWithNext();
        }

        return bRet;
    }
}
void SwTabFrm::MakeAll(vcl::RenderContext* pRenderContext)
{
    if ( IsJoinLocked() || StackHack::IsLocked() || StackHack::Count() > 50 )
        return;

    if ( HasFollow() )
    {
        SwTabFrm* pFollowFrm = GetFollow();
        OSL_ENSURE( !pFollowFrm->IsJoinLocked() || !pFollowFrm->IsRebuildLastLine(),
                "SwTabFrm::MakeAll for master while follow is in RebuildLastLine()" );
        if ( pFollowFrm->IsJoinLocked() && pFollowFrm->IsRebuildLastLine() )
            return;
    }

    PROTOCOL_ENTER( this, PROT_MAKEALL, 0, 0 )

    LockJoin(); //I don't want to be destroyed on the way.
    SwLayNotify aNotify( this );    //does the notification in the DTor
    // If pos is invalid, we have to call a SetInvaKeep at aNotify.
    // Otherwise the keep attribute would not work in front of a table.
    const bool bOldValidPos = GetValidPosFlag();

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
                            pLayout->GetBrowseWidthByTabFrm( *this ) );
    }

    // as long as bMakePage is true, a new page can be created (exactly once)
    bool bMakePage = true;
    // bMovedBwd gets set to true when the frame flows backwards
    bool bMovedBwd = false;
    // as long as bMovedFwd is false, the Frm may flow backwards (until
    // it has been moved forward once)
    bool bMovedFwd  = false;
    // gets set to true when the Frm is split
    bool bSplit = false;
    const bool bFootnotesInDoc = !GetFormat()->GetDoc()->GetFootnoteIdxs().empty();
    const bool bFly     = IsInFly();

    auto pAccess = o3tl::make_unique<SwBorderAttrAccess>(SwFrm::GetCache(), this);
    const SwBorderAttrs *pAttrs = pAccess->Get();

    // The beloved keep attribute
    const bool bKeep = IsKeep( pAttrs->GetAttrSet() );

    // All rows should keep together
    // OD 2004-05-25 #i21478# - don't split table, if it has to keep with next
    const bool bDontSplit = !IsFollow() &&
                            ( !GetFormat()->GetLayoutSplit().GetValue() || bKeep );

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
            0 == GetFirstNonHeadlineRow() )
        {
            if ( HasFollowFlowLine() )
                RemoveFollowFlowLine();
            Join();
        }
    }

    // Join follow table, if last row of this table should keep:
    if ( bTableRowKeep && GetFollow() && !GetFollow()->IsJoinLocked() )
    {
        const SwRowFrm* pTmpRow = static_cast<const SwRowFrm*>(GetLastLower());
        if ( pTmpRow && pTmpRow->ShouldRowKeepWithNext() )
        {
            if ( HasFollowFlowLine() )
                RemoveFollowFlowLine();
            Join();
        }
    }

    // a new one is moved forwards immediately
    if ( !Frm().Top() && IsFollow() )
    {
        SwFrm *pPre = GetPrev();
        if ( pPre && pPre->IsTabFrm() && static_cast<SwTabFrm*>(pPre)->GetFollow() == this)
        {
            if ( !MoveFwd( bMakePage, false ) )
                bMakePage = false;
            bMovedFwd = true;
        }
    }

    int nUnSplitted = 5; // Just another loop control :-(
    int nThrowAwayValidLayoutLimit = 5; // And another one :-(
    SWRECTFN( this )
    while ( !mbValidPos || !mbValidSize || !mbValidPrtArea )
    {
        const bool bMoveable = IsMoveable();
        if (bMoveable)
            if ( CheckMoveFwd( bMakePage, bKeep && KEEPTAB, bMovedBwd ) )
            {
                bMovedFwd = true;
                m_bCalcLowers = true;
                // #i99267#
                // reset <bSplit> after forward move to assure that follows
                // can be joined, if further space is available.
                bSplit = false;
            }

        Point aOldPos( (Frm().*fnRect->fnGetPos)() );
        MakePos();

        if ( aOldPos != (Frm().*fnRect->fnGetPos)() )
        {
            if ( aOldPos.Y() != (Frm().*fnRect->fnGetTop)() )
            {
                SwHTMLTableLayout *pLayout = GetTable()->GetHTMLTableLayout();
                if( pLayout )
                {
                    pAccess.reset();
                    m_bCalcLowers |= pLayout->Resize(
                        pLayout->GetBrowseWidthByTabFrm( *this ) );
                    pAccess = o3tl::make_unique<SwBorderAttrAccess>(SwFrm::GetCache(), this);
                    pAttrs = pAccess->Get();
                }

                mbValidPrtArea = false;
                aNotify.SetLowersComplete( false );
            }
            SwFrm *pPre;
            if ( bKeep || (0 != (pPre = FindPrev()) &&
                pPre->GetAttrSet()->GetKeep().GetValue()) )
            {
                m_bCalcLowers = true;
                // #i99267#
                // reset <bSplit> after forward move to assure that follows
                // can be joined, if further space is available.
                bSplit = false;
            }
        }

        //We need to know the height of the first row, because the master needs
        //to be activated if it shrinks and then absorb the row if necessary.
        long n1StLineHeight = 0;
        if ( IsFollow() )
        {
            SwFrm* pFrm = GetFirstNonHeadlineRow();
            if ( pFrm )
                n1StLineHeight = (pFrm->Frm().*fnRect->fnGetHeight)();
        }

        if ( !mbValidSize || !mbValidPrtArea )
        {
            const long nOldPrtWidth = (Prt().*fnRect->fnGetWidth)();
            const long nOldFrmWidth = (Frm().*fnRect->fnGetWidth)();
            const Point aOldPrtPos  = (Prt().*fnRect->fnGetPos)();
            Format( getRootFrm()->GetCurrShell()->GetOut(), pAttrs );

            SwHTMLTableLayout *pLayout = GetTable()->GetHTMLTableLayout();
            if ( pLayout &&
                ((Prt().*fnRect->fnGetWidth)() != nOldPrtWidth ||
                (Frm().*fnRect->fnGetWidth)() != nOldFrmWidth) )
            {
                pAccess.reset();
                m_bCalcLowers |= pLayout->Resize(
                    pLayout->GetBrowseWidthByTabFrm( *this ) );
                pAccess = o3tl::make_unique<SwBorderAttrAccess>(SwFrm::GetCache(), this);
                pAttrs = pAccess->Get();
            }
            if ( aOldPrtPos != (Prt().*fnRect->fnGetPos)() )
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
                SwFrm *pFrm = GetFirstNonHeadlineRow();
                if( pFrm && n1StLineHeight >(pFrm->Frm().*fnRect->fnGetHeight )() )
                {
                    SwTabFrm *pMaster = FindMaster();
                    bool bDummy;
                    if ( ShouldBwdMoved( pMaster->GetUpper(), false, bDummy ) )
                        pMaster->InvalidatePos();
                }
            }
            SwFootnoteBossFrm *pOldBoss = bFootnotesInDoc ? FindFootnoteBossFrm( true ) : 0;
            bool bReformat;
            if ( MoveBwd( bReformat ) )
            {
                SWREFRESHFN( this )
                bMovedBwd = true;
                aNotify.SetLowersComplete( false );
                if ( bFootnotesInDoc )
                    MoveLowerFootnotes( 0, pOldBoss, 0, true );
                if ( bReformat || bKeep )
                {
                    long nOldTop = (Frm().*fnRect->fnGetTop)();
                    MakePos();
                    if( nOldTop != (Frm().*fnRect->fnGetTop)() )
                    {
                        SwHTMLTableLayout *pHTMLLayout =
                            GetTable()->GetHTMLTableLayout();
                        if( pHTMLLayout )
                        {
                            pAccess.reset();
                            m_bCalcLowers |= pHTMLLayout->Resize(
                                pHTMLLayout->GetBrowseWidthByTabFrm( *this ) );

                            pAccess = o3tl::make_unique<SwBorderAttrAccess>(SwFrm::GetCache(), this);
                            pAttrs = pAccess->Get();
                        }

                        mbValidPrtArea = false;
                        Format( getRootFrm()->GetCurrShell()->GetOut(), pAttrs );
                    }
                    lcl_RecalcTable( *this, 0, aNotify );
                    m_bLowersFormatted = true;
                    if ( bKeep && KEEPTAB )
                    {

                        // Consider case that table is inside another table,
                        // because it has to be avoided, that superior table
                        // is formatted.
                        // Thus, find next content, table or section
                        // and, if a section is found, get its first
                        // content.
                        if ( 0 != sw_FormatNextContentForKeep( this ) && !GetNext() )
                        {
                            mbValidPos = false;
                        }
                    }
                }
            }
        }

        //Again an invalid value? - do it again...
        if ( !mbValidPos || !mbValidSize || !mbValidPrtArea )
            continue;

        // check, if calculation of table frame is ready.

        // Local variable <nDistanceToUpperPrtBottom>
        //     Introduce local variable and init it with the distance from the
        //     table frame bottom to the bottom of the upper printing area.
        // Note: negative values denotes the situation that table frame doesn't fit in its upper.
        SwTwips nDistanceToUpperPrtBottom =
                (Frm().*fnRect->fnBottomDist)( (GetUpper()->*fnRect->fnGetPrtBottom)());

        /// In online layout try to grow upper of table frame, if table frame doesn't fit in its upper.
        const SwViewShell *pSh = getRootFrm()->GetCurrShell();
        const bool bBrowseMode = pSh && pSh->GetViewOptions()->getBrowseMode();
        if ( nDistanceToUpperPrtBottom < 0 && bBrowseMode )
        {
            if ( GetUpper()->Grow( -nDistanceToUpperPrtBottom ) )
            {
                // upper is grown --> recalculate <nDistanceToUpperPrtBottom>
                nDistanceToUpperPrtBottom = (Frm().*fnRect->fnBottomDist)( (GetUpper()->*fnRect->fnGetPrtBottom)());
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
                    SwFrm *pTmp = GetUpper();
                    SwTwips nDeadLine = (pTmp->*fnRect->fnGetPrtBottom)();
                    if ( bBrowseMode )
                        nDeadLine += pTmp->Grow( LONG_MAX, true );
                    if( (Frm().*fnRect->fnBottomDist)( nDeadLine ) > 0 )
                    {
                        // First, we remove an existing follow flow line.
                        if ( HasFollowFlowLine() )
                        {
                            SwFrm* pLastLine = GetLastLower();
                            RemoveFollowFlowLine();
                            // invalidate and rebuild last row
                            if ( pLastLine )
                            {
                                ::SwInvalidateAll( pLastLine, LONG_MAX );
                                SetRebuildLastLine( true );
                                lcl_RecalcRow( static_cast<SwRowFrm&>(*pLastLine), LONG_MAX );
                                SetRebuildLastLine( false );
                            }

                            SwFrm* pRow = GetFollow()->GetFirstNonHeadlineRow();

                            if ( !pRow || !pRow->GetNext() )
                                //The follow becomes empty and invalid for this reason.
                                Join();

                            continue;
                        }

                        // If there is no follow flow line, we move the first
                        // row in the follow table to the master table.
                        SwRowFrm *pRow = GetFollow()->GetFirstNonHeadlineRow();

                        //The follow becomes empty and invalid for this reason.
                        if ( !pRow )
                        {
                            Join();
                            continue;
                        }

                        const SwTwips nOld = (Frm().*fnRect->fnGetHeight)();
                        long nRowsToMove = lcl_GetMaximumLayoutRowSpan( *pRow );
                        SwFrm* pRowToMove = pRow;

                        while ( pRowToMove && nRowsToMove-- > 0 )
                        {
                            const bool bMoveFootnotes = bFootnotesInDoc && !GetFollow()->IsJoinLocked();

                            SwFootnoteBossFrm *pOldBoss = 0;
                            if ( bMoveFootnotes )
                                pOldBoss = pRowToMove->FindFootnoteBossFrm( true );

                            SwFrm* pNextRow = pRowToMove->GetNext();

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
                                if ( static_cast<SwLayoutFrm*>(pRowToMove)->MoveLowerFootnotes( 0, pOldBoss, FindFootnoteBossFrm( true ), true ) )
                                    GetUpper()->Calc(pRenderContext);

                            pRowToMove = pNextRow;
                        }

                        if ( nOld != (Frm().*fnRect->fnGetHeight)() )
                            lcl_RecalcTable( *this, static_cast<SwLayoutFrm*>(pRow), aNotify );

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
                    const SwRowFrm* pLastRow = static_cast<const SwRowFrm*>(GetLastLower());
                    if ( pLastRow && IsKeep( pAttrs->GetAttrSet(), true ) &&
                         pLastRow->ShouldRowKeepWithNext() )
                        bFormat = true;
                }

                if ( bFormat )
                {
                    pAccess.reset();

                    // Consider case that table is inside another table, because
                    // it has to be avoided, that superior table is formatted.
                    // Thus, find next content, table or section and, if a section
                    // is found, get its first content.
                    const SwFrm* pTmpNxt = sw_FormatNextContentForKeep( this );

                    pAccess = o3tl::make_unique<SwBorderAttrAccess>(SwFrm::GetCache(), this);
                    pAttrs = pAccess->Get();

                    // The last row wants to keep with the frame behind the table.
                    // Check if the next frame is on a different page and valid.
                    // In this case we do a magic trick:
                    if ( !bKeep && !GetNext() && pTmpNxt && pTmpNxt->IsValid() )
                    {
                        mbValidPos = false;
                        bLastRowHasToMoveToFollow = true;
                    }
                }
            }

            if ( IsValid() )
            {
                if (m_bCalcLowers)
                {
                    lcl_RecalcTable( *this, 0, aNotify );
                    m_bLowersFormatted = true;
                    m_bCalcLowers = false;
                }
                else if (m_bONECalcLowers)
                {
                    lcl_RecalcRow( static_cast<SwRowFrm&>(*Lower()), LONG_MAX );
                    m_bONECalcLowers = false;
                }
            }
            continue;
        }

        //I don't fit in the higher-ranked element anymore, therefore it's the
        //right moment to do some preferably constructive changes.

        //If I'm NOT allowed to leave the parent Frm, I've got a problem.
        // Following Arthur Dent, we do the only thing that you can do with
        // an unsolvable problem: We ignore it with all our power.
        if ( !bMoveable )
        {
            if (m_bCalcLowers && IsValid())
            {
                lcl_RecalcTable( *this, 0, aNotify );
                m_bLowersFormatted = true;
                m_bCalcLowers = false;
            }
            else if (m_bONECalcLowers)
            {
                lcl_RecalcRow( static_cast<SwRowFrm&>(*Lower()), LONG_MAX );
                m_bONECalcLowers = false;
            }

            // It does not make sense to cut off the last line if we are
            // not moveable:
            bLastRowHasToMoveToFollow = false;

            continue;
        }

        if (m_bCalcLowers && IsValid())
        {
            lcl_RecalcTable( *this, 0, aNotify );
            m_bLowersFormatted = true;
            m_bCalcLowers = false;
            if( !IsValid() )
                continue;
        }

        // First try to split the table. Condition:
        // 1. We have at least one non headline row
        // 2. If this row wants to keep, we need an additional row
        // 3. The table is allowed to split or we do not have an pIndPrev:
        SwFrm* pIndPrev = GetIndPrev();
        const SwRowFrm* pFirstNonHeadlineRow = GetFirstNonHeadlineRow();
        // #i120016# if this row wants to keep, allow split in case that all rows want to keep with next,
        // the table can not move forward as it is the first one and a split is in general allowed.
        const bool bAllowSplitOfRow = ( bTableRowKeep &&
                                        AreAllRowsKeepWithNext( pFirstNonHeadlineRow ) ) &&
                                      !pIndPrev &&
                                      !bDontSplit;

        if ( pFirstNonHeadlineRow && nUnSplitted > 0 &&
             ( !bTableRowKeep || pFirstNonHeadlineRow->GetNext() || !pFirstNonHeadlineRow->ShouldRowKeepWithNext() || bAllowSplitOfRow ) &&
             ( !bDontSplit || !pIndPrev ) )
        {
            // #i29438#
            // Special DoNotSplit case:
            // We better avoid splitting of a row frame if we are inside a columned
            // section which has a height of 0, because this is not growable and thus
            // all kinds of unexpected things could happen.
            if ( IsInSct() &&
                (FindSctFrm())->Lower()->IsColumnFrm() &&
                0 == (GetUpper()->Frm().*fnRect->fnGetHeight)()  )
            {
                bTryToSplit = false;
            }

            // 1. Try: bTryToSplit = true  => Try to split the row.
            // 2. Try: bTryToSplit = false => Split the table between the rows.
            if ( pFirstNonHeadlineRow->GetNext() || bTryToSplit )
            {
                SwTwips nDeadLine = (GetUpper()->*fnRect->fnGetPrtBottom)();
                if( IsInSct() || GetUpper()->IsInTab() ) // TABLE IN TABLE)
                    nDeadLine = (*fnRect->fnYInc)( nDeadLine,
                                        GetUpper()->Grow( LONG_MAX, true ) );

                {
                    SetInRecalcLowerRow( true );
                    ::lcl_RecalcRow( static_cast<SwRowFrm&>(*Lower()), nDeadLine );
                    SetInRecalcLowerRow( false );
                }
                m_bLowersFormatted = true;
                aNotify.SetLowersComplete( true );

                // One more check if its really necessary to split the table.
                // 1. The table either has to exceed the deadline or
                // 2. We explicitly want to cut off the last row.
                if( (Frm().*fnRect->fnBottomDist)( nDeadLine ) > 0 && !bLastRowHasToMoveToFollow )
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
                    const SwFrm* pTmpRow = GetUpper();
                    while ( pTmpRow && !pTmpRow->IsRowFrm() )
                       pTmpRow = pTmpRow->GetUpper();
                    if ( pTmpRow && !static_cast<const SwRowFrm*>(pTmpRow)->IsRowSplitAllowed() )
                        continue;
                }

                sal_uInt16 nMinNumOfLines = nRepeat;

                if ( bTableRowKeep )
                {
                    const SwRowFrm* pTmpRow = pFirstNonHeadlineRow;
                    while ( pTmpRow && pTmpRow->ShouldRowKeepWithNext() )
                    {
                        ++nMinNumOfLines;
                        pTmpRow = static_cast<const SwRowFrm*>(pTmpRow->GetNext());
                    }
                }

                if ( !bTryToSplit )
                    ++nMinNumOfLines;

                const SwTwips nBreakLine = (*fnRect->fnYInc)(
                        (Frm().*fnRect->fnGetTop)(),
                        (this->*fnRect->fnGetTopMargin)() +
                         lcl_GetHeightOfRows( GetLower(), nMinNumOfLines ) );

                // Some more checks if we want to call the split algorithm or not:
                // The repeating lines / keeping lines still fit into the upper or
                // if we do not have an (in)direct Prev, we split anyway.
                if( (*fnRect->fnYDiff)(nDeadLine, nBreakLine) >=0 || !pIndPrev )
                {
                    aNotify.SetLowersComplete( false );
                    bSplit = true;

                    // An existing follow flow line has to be removed.
                    if ( HasFollowFlowLine() )
                    {
                        if (!nThrowAwayValidLayoutLimit)
                            continue;
                        bool bInitialLoopEndCondition = mbValidPos && mbValidSize && mbValidPrtArea;
                        RemoveFollowFlowLine();
                        bool bFinalLoopEndCondition = mbValidPos && mbValidSize && mbValidPrtArea;
                        if (bInitialLoopEndCondition && !bFinalLoopEndCondition)
                            --nThrowAwayValidLayoutLimit;
                    }

                    const bool bSplitError = !Split( nDeadLine, bTryToSplit, ( bTableRowKeep && !bAllowSplitOfRow ) );
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
                        lcl_RecalcRow( static_cast<SwRowFrm&>(*Lower()), LONG_MAX );
                        mbValidPos = false;
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
                        SWRECTFNX( GetFollow() )

                        static sal_uInt8 nStack = 0;
                        if ( !StackHack::IsLocked() && nStack < 4 )
                        {
                            ++nStack;
                            StackHack aHack;
                            pAccess.reset();

                            GetFollow()->MakeAll(pRenderContext);

                            pAccess = o3tl::make_unique<SwBorderAttrAccess>(SwFrm::GetCache(), this);
                            pAttrs = pAccess->Get();

                            GetFollow()->SetLowersFormatted(false);
                            // #i43913# - lock follow table
                            // to avoid its formatting during the format of
                            // its content.
                            const bool bOldJoinLock =  GetFollow()->IsJoinLocked();
                            GetFollow()->LockJoin();
                            ::lcl_RecalcRow( static_cast<SwRowFrm&>(*GetFollow()->Lower()),
                                             (GetFollow()->GetUpper()->Frm().*fnRectX->fnGetBottom)() );
                            // #i43913#
                            // #i63632# Do not unlock the
                            // follow if it wasn't locked before.
                            if ( !bOldJoinLock )
                                GetFollow()->UnlockJoin();

                            if ( !GetFollow()->GetFollow() )
                            {
                                SwFrm* pNxt = static_cast<SwFrm*>(GetFollow())->FindNext();
                                if ( pNxt )
                                {
                                    // #i18103# - no formatting of found next
                                    // frame, if its a follow section of the
                                    // 'ColLocked' section, the follow table is
                                    // in.
                                    bool bCalcNxt = true;
                                    if ( GetFollow()->IsInSct() && pNxt->IsSctFrm() )
                                    {
                                        SwSectionFrm* pSct = GetFollow()->FindSctFrm();
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

        if( IsInSct() && bMovedFwd && bMakePage && GetUpper()->IsColBodyFrm() &&
            GetUpper()->GetUpper()->GetUpper()->IsSctFrm() &&
            ( GetUpper()->GetUpper()->GetPrev() || GetIndPrev() ) &&
            static_cast<SwSectionFrm*>(GetUpper()->GetUpper()->GetUpper())->MoveAllowed(this) )
        {
            bMovedFwd = false;
        }

        // #i29771# Reset bTryToSplit flag on change of upper
        const SwFrm* pOldUpper = GetUpper();

        //Let's see if we find some place anywhere...
        if ( !bMovedFwd && !MoveFwd( bMakePage, false ) )
            bMakePage = false;

        // #i29771# Reset bSplitError flag on change of upper
        if ( GetUpper() != pOldUpper )
        {
            bTryToSplit = true;
            nUnSplitted = 5;
        }

        SWREFRESHFN( this )
        m_bCalcLowers = true;
        bMovedFwd = true;
        aNotify.SetLowersComplete( false );
        if ( IsFollow() )
        {
            //To avoid oscillations now invalid master should drop behind.
            SwTabFrm *pTab = FindMaster();
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

        if (m_bCalcLowers && IsValid())
        {
            // #i44910# - format of lower frames unnecessary
            // and can cause layout loops, if table doesn't fit and isn't
            // allowed to split.
            SwTwips nDistToUpperPrtBottom =
                (Frm().*fnRect->fnBottomDist)( (GetUpper()->*fnRect->fnGetPrtBottom)());
            if ( nDistToUpperPrtBottom >= 0 || bTryToSplit )
            {
                lcl_RecalcTable( *this, 0, aNotify );
                m_bLowersFormatted = true;
                m_bCalcLowers = false;
            }
#if OSL_DEBUG_LEVEL > 0
            else
            {
                OSL_FAIL( "debug assertion: <SwTabFrm::MakeAll()> - format of table lowers suppressed by fix i44910" );
            }
#endif
        }

    } //while ( !mbValidPos || !mbValidSize || !mbValidPrtArea )

    //If my direct predecessor is my master now, it can destroy me during the
    //next best opportunity.
    if ( IsFollow() )
    {
        SwFrm *pPre = GetPrev();
        if ( pPre && pPre->IsTabFrm() && static_cast<SwTabFrm*>(pPre)->GetFollow() == this)
            pPre->InvalidatePos();
    }

    m_bCalcLowers = m_bONECalcLowers = false;
    pAccess.reset();
    UnlockJoin();
    if ( bMovedFwd || bMovedBwd || !bOldValidPos )
        aNotify.SetInvaKeep();
}

/// Calculate the offsets arising because of FlyFrames
bool SwTabFrm::CalcFlyOffsets( SwTwips& rUpper,
                               long& rLeftOffset,
                               long& rRightOffset ) const
{
    bool bInvalidatePrtArea = false;
    const SwPageFrm *pPage = FindPageFrm();
    const SwFlyFrm* pMyFly = FindFlyFrm();

    // --> #108724# Page header/footer content doesn't have to wrap around
    //              floating screen objects

    const IDocumentSettingAccess& rIDSA = GetFormat()->getIDocumentSettingAccess();
    const bool bWrapAllowed = rIDSA.get(DocumentSettingId::USE_FORMER_TEXT_WRAPPING) ||
                                ( !IsInFootnote() && 0 == FindFooterOrHeader() );

    if ( pPage->GetSortedObjs() && bWrapAllowed )
    {
        SWRECTFN( this )
        const bool bConsiderWrapOnObjPos = rIDSA.get(DocumentSettingId::CONSIDER_WRAP_ON_OBJECT_POSITION);
        long nPrtPos = (Frm().*fnRect->fnGetTop)();
        nPrtPos = (*fnRect->fnYInc)( nPrtPos, rUpper );
        SwRect aRect( Frm() );
        long nYDiff = (*fnRect->fnYDiff)( (Prt().*fnRect->fnGetTop)(), rUpper );
        if( nYDiff > 0 )
            (aRect.*fnRect->fnAddBottom)( -nYDiff );
        for ( size_t i = 0; i < pPage->GetSortedObjs()->size(); ++i )
        {
            SwAnchoredObject* pAnchoredObj = (*pPage->GetSortedObjs())[i];
            if ( dynamic_cast< const SwFlyFrm *>( pAnchoredObj ) !=  nullptr )
            {
                SwFlyFrm *pFly = static_cast<SwFlyFrm*>(pAnchoredObj);
                const SwRect aFlyRect = pFly->GetObjRectWithSpaces();
                // #i26945# - correction of conditions,
                // if Writer fly frame has to be considered:
                // - no need to check, if top of Writer fly frame differs
                //   from FAR_AWAY, because its also check, if the Writer
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
                const SwTextFrm* pAnchorCharFrm = pFly->FindAnchorCharFrm();
                bool bConsiderFly =
                    // #i46807# - do not consider invalid
                    // Writer fly frames.
                    pFly->IsValid() &&
                    // fly anchored at character
                    pFly->IsFlyAtCntFrm() &&
                    // fly overlaps with corresponding table rectangle
                    aFlyRect.IsOver( aRect ) &&
                    // fly isn't lower of table and
                    // anchor character frame of fly isn't lower of table
                    ( !IsAnLower( pFly ) &&
                      ( !pAnchorCharFrm || !IsAnLower( pAnchorCharFrm ) ) ) &&
                    // table isn't lower of fly
                    !pFly->IsAnLower( this ) &&
                    // fly is lower of fly, the table is in
                    // #123274# - correction
                    // assure that fly isn't a lower of a fly, the table isn't in.
                    // E.g., a table in the body doesn't wrap around a graphic,
                    // which is inside a frame.
                    ( ( !pMyFly ||
                        pMyFly->IsAnLower( pFly ) ) &&
                      pMyFly == pFly->GetAnchorFrmContainingAnchPos()->FindFlyFrm() ) &&
                    // anchor frame not on following page
                    pPage->GetPhyPageNum() >=
                      pFly->GetAnchorFrm()->FindPageFrm()->GetPhyPageNum() &&
                    // anchor character text frame on same page
                    ( !pAnchorCharFrm ||
                      pAnchorCharFrm->FindPageFrm()->GetPhyPageNum() ==
                        pPage->GetPhyPageNum() );

                if ( bConsiderFly )
                {
                    const SwFrm* pFlyHeaderFooterFrm = pFly->GetAnchorFrm()->FindFooterOrHeader();
                    const SwFrm* pThisHeaderFooterFrm = FindFooterOrHeader();

                    if ( pFlyHeaderFooterFrm != pThisHeaderFooterFrm &&
                        // #148493# If bConsiderWrapOnObjPos is set,
                        // we want to consider the fly if it is located in the header and
                        // the table is located in the body:
                         ( !bConsiderWrapOnObjPos || 0 != pThisHeaderFooterFrm || !pFlyHeaderFooterFrm->IsHeaderFrm() ) )
                        bConsiderFly = false;
                }

                if ( bConsiderFly )
                {
                    const SwFormatSurround   &rSur = pFly->GetFormat()->GetSurround();
                    const SwFormatHoriOrient &rHori= pFly->GetFormat()->GetHoriOrient();
                    if ( SURROUND_NONE == rSur.GetSurround() )
                    {
                        long nBottom = (aFlyRect.*fnRect->fnGetBottom)();
                        if( (*fnRect->fnYDiff)( nPrtPos, nBottom ) < 0 )
                            nPrtPos = nBottom;
                        bInvalidatePrtArea = true;
                    }
                    if ( (SURROUND_RIGHT    == rSur.GetSurround() ||
                          SURROUND_PARALLEL == rSur.GetSurround())&&
                         text::HoriOrientation::LEFT == rHori.GetHoriOrient() )
                    {
                        const long nWidth = (*fnRect->fnXDiff)(
                            (aFlyRect.*fnRect->fnGetRight)(),
                            (pFly->GetAnchorFrm()->Frm().*fnRect->fnGetLeft)() );
                        rLeftOffset = std::max( rLeftOffset, nWidth );
                        bInvalidatePrtArea = true;
                    }
                    if ( (SURROUND_LEFT     == rSur.GetSurround() ||
                          SURROUND_PARALLEL == rSur.GetSurround())&&
                         text::HoriOrientation::RIGHT == rHori.GetHoriOrient() )
                    {
                        const long nWidth = (*fnRect->fnXDiff)(
                            (pFly->GetAnchorFrm()->Frm().*fnRect->fnGetRight)(),
                            (aFlyRect.*fnRect->fnGetLeft)() );
                        rRightOffset = std::max( rRightOffset, nWidth );
                        bInvalidatePrtArea = true;
                    }
                }
            }
        }
        rUpper = (*fnRect->fnYDiff)( nPrtPos, (Frm().*fnRect->fnGetTop)() );
    }

    return bInvalidatePrtArea;
}

/// "Formats" the frame; Frm and PrtArea.
/// The fixed size is not adjusted here.
void SwTabFrm::Format( vcl::RenderContext* /*pRenderContext*/, const SwBorderAttrs *pAttrs )
{
    OSL_ENSURE( pAttrs, "TabFrm::Format, pAttrs ist 0." );

    SWRECTFN( this )
    if ( !mbValidSize )
    {
        long nDiff = (GetUpper()->Prt().*fnRect->fnGetWidth)() -
                     (Frm().*fnRect->fnGetWidth)();
        if( nDiff )
            (maFrm.*fnRect->fnAddRight)( nDiff );
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
        mbValidPrtArea = false;
    long nRightOffset = std::max( 0L, nTmpRight );

    SwTwips nLower = pAttrs->CalcBottomLine();
    // #i29550#
    if ( IsCollapsingBorders() )
        nLower += GetBottomLineSize();

    if ( !mbValidPrtArea )
    {   mbValidPrtArea = true;

        //The width of the PrtArea is given by the FrameFormat, the borders have to
        //be set accordingly.
        //Minimum borders are determined depending on margins and shadows.
        //The borders are adjusted so that the PrtArea is aligned into the Frm
        //according to the adjustment.
        //If the adjustment is 0, the borders are set according to the border
        //attributes.

        const SwTwips nOldHeight = (Prt().*fnRect->fnGetHeight)();
        const SwTwips nMax = (maFrm.*fnRect->fnGetWidth)();

        // OD 14.03.2003 #i9040# - adjust variable names.
        const SwTwips nLeftLine  = pAttrs->CalcLeftLine();
        const SwTwips nRightLine = pAttrs->CalcRightLine();

        //The width possibly is a percentage value. If the table is inside
        //something else, the value applies to the surrounding. If it's the body
        //the value applies to the screen width in the BrowseView.
        const SwFormatFrmSize &rSz = GetFormat()->GetFrmSize();
        // OD 14.03.2003 #i9040# - adjust variable name.
        const SwTwips nWishedTableWidth = CalcRel( rSz, true );

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
                    //Linker Rand und die Breite zaehlen (Word-Spezialitaet)
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
                OSL_FAIL( "Ungueltige orientation fuer Table." );
        }

        // #i26250# - extend bottom printing area, if table
        // is last content inside a table cell.
        if ( GetFormat()->getIDocumentSettingAccess().get(DocumentSettingId::ADD_PARA_SPACING_TO_TABLE_CELLS) &&
             GetUpper()->IsInTab() && !GetIndNext() )
        {
            nLower += pAttrs->GetULSpace().GetLower();
        }
        (this->*fnRect->fnSetYMargins)( nUpper, nLower );
        if( (nMax - MINLAY) < (nLeftSpacing + nRightSpacing) )
            (this->*fnRect->fnSetXMargins)( 0, 0 );
        else
            (this->*fnRect->fnSetXMargins)( nLeftSpacing, nRightSpacing );

        SwViewShell *pSh = getRootFrm()->GetCurrShell();
        if ( bCheckBrowseWidth &&
             pSh && pSh->GetViewOptions()->getBrowseMode() &&
             GetUpper()->IsPageBodyFrm() &&  // only PageBodyFrms and not ColBodyFrms
             pSh->VisArea().Width() )
        {
            //Don't overlap the edge of the visible area.
            //The page width can be bigger because objects with
            //"over-size" are possible (RootFrm::ImplCalcBrowseWidth())
            long nWidth = pSh->GetBrowseWidth();
            nWidth -= Prt().Left();
            nWidth -= pAttrs->CalcRightLine();
            Prt().Width( std::min( nWidth, Prt().Width() ) );
        }

        if ( nOldHeight != (Prt().*fnRect->fnGetHeight)() )
            mbValidSize = false;
    }

    if ( !mbValidSize )
    {
        mbValidSize = true;

        //The size is defined by the content plus the borders.
        SwTwips nRemaining = 0, nDiff;
        SwFrm *pFrm = m_pLower;
        while ( pFrm )
        {
            nRemaining += (pFrm->Frm().*fnRect->fnGetHeight)();
            pFrm = pFrm->GetNext();
        }
        //And now add the borders
        nRemaining += nUpper + nLower;

        nDiff = (Frm().*fnRect->fnGetHeight)() - nRemaining;
        if ( nDiff > 0 )
            Shrink( nDiff );
        else if ( nDiff < 0 )
            Grow( -nDiff );
    }
}

SwTwips SwTabFrm::GrowFrm( SwTwips nDist, bool bTst, bool bInfo )
{
    SWRECTFN( this )
    SwTwips nHeight =(Frm().*fnRect->fnGetHeight)();
    if( nHeight > 0 && nDist > ( LONG_MAX - nHeight ) )
        nDist = LONG_MAX - nHeight;

    if ( bTst && !IsRestrictTableGrowth() )
        return nDist;

    if ( GetUpper() )
    {
        SwRect aOldFrm( Frm() );

        //The upper only grows as far as needed. nReal provides the distance
        //which is already available.
        SwTwips nReal = (GetUpper()->Prt().*fnRect->fnGetHeight)();
        SwFrm *pFrm = GetUpper()->Lower();
        while ( pFrm && GetFollow() != pFrm )
        {
            nReal -= (pFrm->Frm().*fnRect->fnGetHeight)();
            pFrm = pFrm->GetNext();
        }

        if ( nReal < nDist )
        {
            long nTmp = GetUpper()->Grow( nDist - ( nReal > 0 ? nReal : 0), bTst, bInfo );

            if ( IsRestrictTableGrowth() )
            {
                nTmp = std::min( nDist, nReal + nTmp );
                nDist = nTmp < 0 ? 0 : nTmp;
            }
        }

        if ( !bTst )
        {
            (Frm().*fnRect->fnAddBottom)( nDist );

            SwRootFrm *pRootFrm = getRootFrm();
            if( pRootFrm && pRootFrm->IsAnyShellAccessible() &&
                pRootFrm->GetCurrShell() )
            {
                pRootFrm->GetCurrShell()->Imp()->MoveAccessibleFrm( this, aOldFrm );
            }
        }
    }

    if ( !bTst && ( nDist || IsRestrictTableGrowth() ) )
    {
        SwPageFrm *pPage = FindPageFrm();
        if ( GetNext() )
        {
            GetNext()->_InvalidatePos();
            if ( GetNext()->IsContentFrm() )
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
        _InvalidateAll();
        InvalidatePage( pPage );
        SetComplete();

        SvxBrushItem aBack = GetFormat()->makeBackgroundBrushItem();
        const SvxGraphicPosition ePos = aBack.GetGraphicPos();
        if ( GPOS_NONE != ePos && GPOS_TILED != ePos )
            SetCompletePaint();
    }

    return nDist;
}

void SwTabFrm::Modify( const SfxPoolItem* pOld, const SfxPoolItem * pNew )
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
            _UpdateAttr( aOIter.GetCurItem(),
                         aNIter.GetCurItem(), nInvFlags,
                         &aOldSet, &aNewSet );
            if( aNIter.IsAtEnd() )
                break;
            aNIter.NextItem();
            aOIter.NextItem();
        }
        if ( aOldSet.Count() || aNewSet.Count() )
            SwLayoutFrm::Modify( &aOldSet, &aNewSet );
    }
    else
        _UpdateAttr( pOld, pNew, nInvFlags );

    if ( nInvFlags != 0 )
    {
        SwPageFrm *pPage = FindPageFrm();
        InvalidatePage( pPage );
        if ( nInvFlags & 0x02 )
            _InvalidatePrt();
        if ( nInvFlags & 0x40 )
            _InvalidatePos();
        SwFrm *pTmp;
        if ( 0 != (pTmp = GetIndNext()) )
        {
            if ( nInvFlags & 0x04 )
            {
                pTmp->_InvalidatePrt();
                if ( pTmp->IsContentFrm() )
                    pTmp->InvalidatePage( pPage );
            }
            if ( nInvFlags & 0x10 )
                pTmp->SetCompletePaint();
        }
        if ( nInvFlags & 0x08 && 0 != (pTmp = GetPrev()) )
        {
            pTmp->_InvalidatePrt();
            if ( pTmp->IsContentFrm() )
                pTmp->InvalidatePage( pPage );
        }
        if ( nInvFlags & 0x20  )
        {
            if ( pPage && pPage->GetUpper() && !IsFollow() )
                static_cast<SwRootFrm*>(pPage->GetUpper())->InvalidateBrowseWidth();
        }
        if ( nInvFlags & 0x80 )
            InvalidateNextPos();
    }
}

void SwTabFrm::_UpdateAttr( const SfxPoolItem *pOld, const SfxPoolItem *pNew,
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
                SwRowFrm* pLowerRow = 0;
                while ( 0 != ( pLowerRow = static_cast<SwRowFrm*>(Lower()) ) && pLowerRow->IsRepeatedHeadline() )
                {
                    pLowerRow->Cut();
                    SwFrm::DestroyFrm(pLowerRow);
                }

                // insert new headlines
                const sal_uInt16 nNewRepeat = GetTable()->GetRowsToRepeat();
                for ( sal_uInt16 nIdx = 0; nIdx < nNewRepeat; ++nIdx )
                {
                    bDontCreateObjects = true;          //frmtool
                    SwRowFrm* pHeadline = new SwRowFrm( *GetTable()->GetTabLines()[ nIdx ], this );
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
                SwPageFrm *pPage = FindPageFrm();
                if (pPage)
                {
                    if ( !GetPrev() )
                        CheckPageDescs( pPage );
                    if (GetFormat()->GetPageDesc().GetNumOffset())
                        static_cast<SwRootFrm*>(pPage->GetUpper())->SetVirtPageNum( true );
                    SwDocPosUpdate aMsgHint( pPage->Frm().Top() );
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
            /* no break here */

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
            SwLayoutFrm::Modify( pOld, pNew );
    }
}

bool SwTabFrm::GetInfo( SfxPoolItem &rHint ) const
{
    if ( RES_VIRTPAGENUM_INFO == rHint.Which() && IsInDocBody() && !IsFollow() )
    {
        SwVirtPageNumInfo &rInfo = static_cast<SwVirtPageNumInfo&>(rHint);
        const SwPageFrm *pPage = FindPageFrm();
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

SwContentFrm *SwTabFrm::FindLastContent()
{
    SwFrm *pRet = m_pLower;

    while ( pRet && !pRet->IsContentFrm() )
    {
        SwFrm *pOld = pRet;

        SwFrm *pTmp = pRet;             // To skip empty section frames
        while ( pRet->GetNext() )
        {
            pRet = pRet->GetNext();
            if( !pRet->IsSctFrm() || static_cast<SwSectionFrm*>(pRet)->GetSection() )
                pTmp = pRet;
        }
        pRet = pTmp;

        if ( pRet->GetLower() )
            pRet = pRet->GetLower();
        if ( pRet == pOld )
        {
            // Check all other columns if there is a column based section with
            // an empty last column at the end of the last line - this is done
            // by SwSectionFrm::FindLastContent
            if( pRet->IsColBodyFrm() )
            {
#if OSL_DEBUG_LEVEL > 0
                SwSectionFrm* pSect = pRet->FindSctFrm();
                OSL_ENSURE( pSect, "Where does this column come fron?");
                OSL_ENSURE( IsAnLower( pSect ), "Splited cell?" );
#endif
                return pRet->FindSctFrm()->FindLastContent();
            }

            // pRet may be a cell frame without a lower (cell has been split).
            // We have to find the last content the hard way:

            OSL_ENSURE( pRet->IsCellFrm(), "SwTabFrm::FindLastContent failed" );
            const SwFrm* pRow = pRet->GetUpper();
            while ( pRow && !pRow->GetUpper()->IsTabFrm() )
                pRow = pRow->GetUpper();
            const SwContentFrm* pContentFrm = pRow ? static_cast<const SwLayoutFrm*>(pRow)->ContainsContent() : NULL;
            pRet = 0;

            while ( pContentFrm && static_cast<const SwLayoutFrm*>(pRow)->IsAnLower( pContentFrm ) )
            {
                pRet = const_cast<SwContentFrm*>(pContentFrm);
                pContentFrm = pContentFrm->GetNextContentFrm();
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

        if( pRet->IsSctFrm() )
            pRet = static_cast<SwSectionFrm*>(pRet)->FindLastContent();
    }

    return static_cast<SwContentFrm*>(pRet);
}

/// Return value defines if the frm needs to be relocated
bool SwTabFrm::ShouldBwdMoved( SwLayoutFrm *pNewUpper, bool, bool &rReformat )
{
    rReformat = false;
    if ( (SwFlowFrm::IsMoveBwdJump() || !IsPrevObjMove()) )
    {
        //Floating back Frm's is quite time consuming unfortunately.
        //Most often the location where the Frm wants to float to has the same
        //FixSize as the Frm itself. In such a situation it's easy to check if
        //the Frm will find enough space for its VarSize, if this is not the
        //case, the relocation can be skipped.
        //Checking if the Frm will find enough space is done by the Frm itself,
        //this also takes the possibility of splitting the Frm into account.
        //If the FixSize is different or Flys are involved  (at the old or the
        //new position) the whole checks don't make sense at all, the Frm then
        //needs to be relocated tentatively (if a bit of space is available).

        //The FixSize of the surrounding which contain tables is always the
        //width.

        SwPageFrm *pOldPage = FindPageFrm(),
                  *pNewPage = pNewUpper->FindPageFrm();
        bool bMoveAnyway = false;
        SwTwips nSpace = 0;

        SWRECTFN( this )
        if ( !SwFlowFrm::IsMoveBwdJump() )
        {

            long nOldWidth = (GetUpper()->Prt().*fnRect->fnGetWidth)();
            SWRECTFNX( pNewUpper );
            long nNewWidth = (pNewUpper->Prt().*fnRectX->fnGetWidth)();
            if( std::abs( nNewWidth - nOldWidth ) < 2 )
            {
                if( !( bMoveAnyway = (BwdMoveNecessary( pOldPage, Frm() ) > 1) ) )
                {
                    SwRect aRect( pNewUpper->Prt() );
                    aRect.Pos() += pNewUpper->Frm().Pos();
                    const SwFrm *pPrevFrm = pNewUpper->Lower();
                    while ( pPrevFrm && pPrevFrm != this )
                    {
                        (aRect.*fnRectX->fnSetTop)( (pPrevFrm->Frm().*fnRectX->
                                                    fnGetBottom)() );
                        pPrevFrm = pPrevFrm->GetNext();
                    }
                    bMoveAnyway = BwdMoveNecessary( pNewPage, aRect) > 1;

                    // #i54861# Due to changes made in PrepareMake,
                    // the tabfrm may not have a correct position. Therefore
                    // it is possible that pNewUpper->Prt().Height == 0. In this
                    // case the above calculation of nSpace might give wrong
                    // results and we really do not want to MoveBackwrd into a
                    // 0 height frame. If nTmpSpace is already <= 0, we take this
                    // value:
                    const SwTwips nTmpSpace = (aRect.*fnRectX->fnGetHeight)();
                    if ( (pNewUpper->Prt().*fnRectX->fnGetHeight)() > 0 || nTmpSpace <= 0 )
                        nSpace = nTmpSpace;

                    const SwViewShell *pSh = getRootFrm()->GetCurrShell();
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
        if (!m_bLockBackMove && nSpace > 0)
        {
            // #i26945# - check, if follow flow line
            // contains frame, which are moved forward due to its object
            // positioning.
            SwRowFrm* pFirstRow = GetFirstNonHeadlineRow();
            if ( pFirstRow && pFirstRow->IsInFollowFlowRow() &&
                 SwLayouter::DoesRowContainMovedFwdFrm(
                                            *(pFirstRow->GetFormat()->GetDoc()),
                                            *(pFirstRow) ) )
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

void SwTabFrm::Cut()
{
    OSL_ENSURE( GetUpper(), "Cut ohne Upper()." );

    SwPageFrm *pPage = FindPageFrm();
    InvalidatePage( pPage );
    SwFrm *pFrm = GetNext();
    if( pFrm )
    {
        //The old follower eventually calculated a margin to the predecessor
        //which is obsolete now as it became the first one
        pFrm->_InvalidatePrt();
        pFrm->_InvalidatePos();
        if ( pFrm->IsContentFrm() )
            pFrm->InvalidatePage( pPage );
        if( IsInSct() && !GetPrev() )
        {
            SwSectionFrm* pSct = FindSctFrm();
            if( !pSct->IsFollow() )
            {
                pSct->_InvalidatePrt();
                pSct->InvalidatePage( pPage );
            }
        }
    }
    else
    {
        InvalidateNextPos();
        //Someone has to do the retouch: predecessor or upper
        if ( 0 != (pFrm = GetPrev()) )
        {   pFrm->SetRetouche();
            pFrm->Prepare( PREP_WIDOWS_ORPHANS );
            pFrm->_InvalidatePos();
            if ( pFrm->IsContentFrm() )
                pFrm->InvalidatePage( pPage );
        }
        //If I am (was) the only FlowFrm in my own upper, it has to do
        //the retouch. Moreover it has to do the retouch.
        else
        {   SwRootFrm *pRoot = static_cast<SwRootFrm*>(pPage->GetUpper());
            pRoot->SetSuperfluous();
            GetUpper()->SetCompletePaint();
            if( IsInSct() )
            {
                SwSectionFrm* pSct = FindSctFrm();
                if( !pSct->IsFollow() )
                {
                    pSct->_InvalidatePrt();
                    pSct->InvalidatePage( pPage );
                }
            }
        }
    }

    //First remove, then shrink the upper.
    SwLayoutFrm *pUp = GetUpper();
    SWRECTFN( this )
    RemoveFromLayout();
    if ( pUp )
    {
        OSL_ENSURE( !pUp->IsFootnoteFrm(), "Table in Footnote." );
        SwSectionFrm *pSct = 0;
        // #126020# - adjust check for empty section
        // #130797# - correct fix #126020#
        if ( !pUp->Lower() && pUp->IsInSct() &&
             !(pSct = pUp->FindSctFrm())->ContainsContent() &&
             !pSct->ContainsAny( true ) )
        {
            if ( pUp->GetUpper() )
            {
                pSct->DelEmpty( false );
                pSct->_InvalidateSize();
            }
        }
        else if( (Frm().*fnRect->fnGetHeight)() )
        {
            // OD 26.08.2003 #i18103# - *no* 'ColUnlock' of section -
            // undo changes of fix for #104992#
            pUp->Shrink( Frm().Height() );
        }
    }

    if ( pPage && !IsFollow() && pPage->GetUpper() )
        static_cast<SwRootFrm*>(pPage->GetUpper())->InvalidateBrowseWidth();
}

void SwTabFrm::Paste( SwFrm* pParent, SwFrm* pSibling )
{
    OSL_ENSURE( pParent, "No parent for pasting." );
    OSL_ENSURE( pParent->IsLayoutFrm(), "Parent is ContentFrm." );
    OSL_ENSURE( pParent != this, "I'm the parent myself." );
    OSL_ENSURE( pSibling != this, "I'm my own neighbour." );
    OSL_ENSURE( !GetPrev() && !GetNext() && !GetUpper(),
            "I'm still registred somewhere." );

    //Insert in the tree.
    InsertBefore( static_cast<SwLayoutFrm*>(pParent), pSibling );

    _InvalidateAll();
    SwPageFrm *pPage = FindPageFrm();
    InvalidatePage( pPage );

    if ( GetNext() )
    {
        GetNext()->_InvalidatePos();
        GetNext()->_InvalidatePrt();
        if ( GetNext()->IsContentFrm() )
            GetNext()->InvalidatePage( pPage );
    }

    SWRECTFN( this )
    if( (Frm().*fnRect->fnGetHeight)() )
        pParent->Grow( (Frm().*fnRect->fnGetHeight)() );

    if( (Frm().*fnRect->fnGetWidth)() != (pParent->Prt().*fnRect->fnGetWidth)() )
        Prepare( PREP_FIXSIZE_CHG );
    if ( GetPrev() )
    {
        if ( !IsFollow() )
        {
            GetPrev()->InvalidateSize();
            if ( GetPrev()->IsContentFrm() )
                GetPrev()->InvalidatePage( pPage );
        }
    }
    else if ( GetNext() )
        //Take the marging into account when dealing with ContentFrm's. There are
        //two situations (both always happen at once):
        //a) The Content becomes the first in a chain
        //b) The new follower was the first in a chain before
        GetNext()->_InvalidatePrt();

    if ( pPage && !IsFollow() )
    {
        if ( pPage->GetUpper() )
            static_cast<SwRootFrm*>(pPage->GetUpper())->InvalidateBrowseWidth();

        if ( !GetPrev() )//At least needed for HTML with a table at the beginning.
        {
            const SwPageDesc *pDesc = GetFormat()->GetPageDesc().GetPageDesc();
            if ( (pDesc && pDesc != pPage->GetPageDesc()) ||
                 (!pDesc && pPage->GetPageDesc() != &GetFormat()->GetDoc()->GetPageDesc(0)) )
                CheckPageDescs( pPage );
        }
    }
}

bool SwTabFrm::Prepare( const PrepareHint eHint, const void *, bool )
{
    if( PREP_BOSS_CHGD == eHint )
        CheckDirChange();
    return false;
}

SwRowFrm::SwRowFrm(const SwTableLine &rLine, SwFrm* pSib, bool bInsertContent)
    : SwLayoutFrm( rLine.GetFrameFormat(), pSib )
    , m_pTabLine( &rLine )
    , m_pFollowRow( 0 )
    // #i29550#
    , mnTopMarginForLowers( 0 )
    , mnBottomMarginForLowers( 0 )
    , mnBottomLineSize( 0 )
    // --> split table rows
    , m_bIsFollowFlowRow( false )
    // <-- split table rows
    , m_bIsRepeatedHeadline( false )
    , m_bIsRowSpanLine( false )
{
    mnFrmType = FRM_ROW;

    //Create the boxes and insert them.
    const SwTableBoxes &rBoxes = rLine.GetTabBoxes();
    SwFrm *pTmpPrev = 0;
    for ( size_t i = 0; i < rBoxes.size(); ++i )
    {
        SwCellFrm *pNew = new SwCellFrm( *rBoxes[i], this, bInsertContent );
        pNew->InsertBehind( this, pTmpPrev );
        pTmpPrev = pNew;
    }
}

void SwRowFrm::DestroyImpl()
{
    SwModify* pMod = GetFormat();
    if( pMod )
    {
        pMod->Remove( this );           // remove,
        if( !pMod->HasWriterListeners() )
            delete pMod;                // and delete
    }

    SwLayoutFrm::DestroyImpl();
}

SwRowFrm::~SwRowFrm()
{
}

void SwRowFrm::RegistFlys( SwPageFrm *pPage )
{
    ::RegistFlys( pPage ? pPage : FindPageFrm(), this );
}

void SwRowFrm::Modify( const SfxPoolItem* pOld, const SfxPoolItem * pNew )
{
    bool bAttrSetChg = pNew && RES_ATTRSET_CHG == pNew->Which();
    const SfxPoolItem *pItem = 0;

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
        SwTabFrm *pTab = FindTabFrm();
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

    SwLayoutFrm::Modify( pOld, pNew );
}

void SwRowFrm::MakeAll(vcl::RenderContext* pRenderContext)
{
    if ( !GetNext() )
        mbValidSize = false;
    SwLayoutFrm::MakeAll(pRenderContext);
}

long CalcHeightWithFlys( const SwFrm *pFrm )
{
    SWRECTFN( pFrm )
    long nHeight = 0;
    const SwFrm* pTmp = pFrm->IsSctFrm() ?
            static_cast<const SwSectionFrm*>(pFrm)->ContainsContent() : pFrm;
    while( pTmp )
    {
        // #i26945# - consider follow text frames
        const SwSortedObjs* pObjs( 0L );
        bool bIsFollow( false );
        if ( pTmp->IsTextFrm() && static_cast<const SwTextFrm*>(pTmp)->IsFollow() )
        {
            const SwFrm* pMaster;
            // #i46450# Master does not necessarily have
            // to exist if this function is called from JoinFrm() ->
            // Cut() -> Shrink()
            const SwTextFrm* pTmpFrm = static_cast<const SwTextFrm*>(pTmp);
            if ( pTmpFrm->GetPrev() && pTmpFrm->GetPrev()->IsTextFrm() &&
                 static_cast<const SwTextFrm*>(pTmpFrm->GetPrev())->GetFollow() &&
                 static_cast<const SwTextFrm*>(pTmpFrm->GetPrev())->GetFollow() != pTmp )
                 pMaster = 0;
            else
                 pMaster = pTmpFrm->FindMaster();

            if ( pMaster )
            {
                 pObjs = static_cast<const SwTextFrm*>(pTmp)->FindMaster()->GetDrawObjs();
                bIsFollow = true;
            }
        }
        else
        {
            pObjs = pTmp->GetDrawObjs();
        }
        if ( pObjs )
        {
            for ( size_t i = 0; i < pObjs->size(); ++i )
            {
                const SwAnchoredObject* pAnchoredObj = (*pObjs)[i];
                // #i26945# - if <pTmp> is follow, the
                // anchor character frame has to be <pTmp>.
                if ( bIsFollow &&
                     const_cast<SwAnchoredObject*>(pAnchoredObj)->FindAnchorCharFrm() != pTmp )
                {
                    continue;
                }
                // #i26945# - consider also drawing objects
                {
                    // OD 30.09.2003 #i18732# - only objects, which follow
                    // the text flow have to be considered.
                    const SwFrameFormat& rFrameFormat = pAnchoredObj->GetFrameFormat();
                    const bool bConsiderObj =
                        (rFrameFormat.GetAnchor().GetAnchorId() != FLY_AS_CHAR) &&
                            pAnchoredObj->GetObjRect().Top() != FAR_AWAY &&
                            rFrameFormat.GetFollowTextFlow().GetValue() &&
                            pAnchoredObj->GetPageFrm() == pTmp->FindPageFrm();
                    if ( bConsiderObj )
                    {
                        const SwFormatFrmSize &rSz = rFrameFormat.GetFrmSize();
                        if( !rSz.GetHeightPercent() )
                        {
                            const SwTwips nDistOfFlyBottomToAnchorTop =
                                (pAnchoredObj->GetObjRect().*fnRect->fnGetHeight)() +
                                    ( bVert ?
                                      pAnchoredObj->GetCurrRelPos().X() :
                                      pAnchoredObj->GetCurrRelPos().Y() );

                            const SwTwips nFrmDiff =
                                (*fnRect->fnYDiff)(
                                    (pTmp->Frm().*fnRect->fnGetTop)(),
                                    (pFrm->Frm().*fnRect->fnGetTop)() );

                            nHeight = std::max( nHeight, nDistOfFlyBottomToAnchorTop + nFrmDiff -
                                            (pFrm->Frm().*fnRect->fnGetHeight)() );

                            // #i56115# The first height calculation
                            // gives wrong results if pFrm->Prt().Y() > 0. We do
                            // a second calculation based on the actual rectangles of
                            // pFrm and pAnchoredObj, and use the maximum of the results.
                            // I do not want to remove the first calculation because
                            // if clipping has been applied, using the GetCurrRelPos
                            // might be the better option to calculate nHeight.
                            const SwTwips nDistOfFlyBottomToAnchorTop2 = (*fnRect->fnYDiff)(
                                                                            (pAnchoredObj->GetObjRect().*fnRect->fnGetBottom)(),
                                                                            (pFrm->Frm().*fnRect->fnGetBottom)() );

                            nHeight = std::max( nHeight, nDistOfFlyBottomToAnchorTop2 );
                        }
                    }
                }
            }
        }
        if( !pFrm->IsSctFrm() )
            break;
        pTmp = pTmp->FindNextCnt();
        if( !static_cast<const SwSectionFrm*>(pFrm)->IsAnLower( pTmp ) )
            break;
    }
    return nHeight;
}

static SwTwips lcl_CalcTopAndBottomMargin( const SwLayoutFrm& rCell, const SwBorderAttrs& rAttrs )
{
    const SwTabFrm* pTab = rCell.FindTabFrm();
    SwTwips nTopSpace = 0;
    SwTwips nBottomSpace = 0;

    // #i29550#
    if ( pTab->IsCollapsingBorders() && rCell.Lower() && !rCell.Lower()->IsRowFrm() )
    {
        nTopSpace    = static_cast<const SwRowFrm*>(rCell.GetUpper())->GetTopMarginForLowers();
        nBottomSpace = static_cast<const SwRowFrm*>(rCell.GetUpper())->GetBottomMarginForLowers();
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
static SwTwips lcl_CalcMinCellHeight( const SwLayoutFrm *_pCell,
                                      const bool _bConsiderObjs,
                                      const SwBorderAttrs *pAttrs = 0 )
{
    SWRECTFN( _pCell )
    SwTwips nHeight = 0;
    const SwFrm* pLow = _pCell->Lower();
    if ( pLow )
    {
        long nFlyAdd = 0;
        while ( pLow )
        {
            // OD 2004-02-18 #106629# - change condition and switch then-body
            // and else-body
            if ( pLow->IsRowFrm() )
            {
                // #i26945#
                nHeight += ::lcl_CalcMinRowHeight( static_cast<const SwRowFrm*>(pLow),
                                                   _bConsiderObjs );
            }
            else
            {
                long nLowHeight = (pLow->Frm().*fnRect->fnGetHeight)();
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
    //calculated using PrtArea and Frm because those can be invalid in arbitrary
    //combinations.
    if ( _pCell->Lower() )
    {
        if ( pAttrs )
            nHeight += lcl_CalcTopAndBottomMargin( *_pCell, *pAttrs );
        else
        {
            SwBorderAttrAccess aAccess( SwFrm::GetCache(), _pCell );
            const SwBorderAttrs &rAttrs = *aAccess.Get();
            nHeight += lcl_CalcTopAndBottomMargin( *_pCell, rAttrs );
        }
    }
    return nHeight;
}

// OD 2004-02-18 #106629# - correct type of 1st parameter
// #i26945# - add parameter <_bConsiderObjs> in order to control,
// if floating screen objects have to be considered for the minimal cell height
static SwTwips lcl_CalcMinRowHeight( const SwRowFrm* _pRow,
                                     const bool _bConsiderObjs )
{
    SWRECTFN( _pRow )

    const SwFormatFrmSize &rSz = _pRow->GetFormat()->GetFrmSize();

    if ( _pRow->HasFixSize() && !_pRow->IsRowSpanLine() )
    {
        OSL_ENSURE( ATT_FIX_SIZE == rSz.GetHeightSizeType(), "pRow claims to have fixed size" );
        return rSz.GetHeight();
    }

    SwTwips nHeight = 0;
    const SwCellFrm* pLow = static_cast<const SwCellFrm*>(_pRow->Lower());
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
            const SwCellFrm& rMaster = pLow->FindStartEndOfRowSpanCell( true, true );
            nTmp = ::lcl_CalcMinCellHeight( &rMaster, _bConsiderObjs );
            const SwFrm* pMasterRow = rMaster.GetUpper();
            while ( pMasterRow && pMasterRow != _pRow )
            {
                nTmp -= (pMasterRow->Frm().*fnRect->fnGetHeight)();
                pMasterRow = pMasterRow->GetNext();
            }
        }
        // <-- NEW TABLES

        // Do not consider rotated cells:
        if ( pLow->IsVertical() == bVert && nTmp > nHeight )
            nHeight = nTmp;

        pLow = static_cast<const SwCellFrm*>(pLow->GetNext());
    }
    if ( rSz.GetHeightSizeType() == ATT_MIN_SIZE && !_pRow->IsRowSpanLine() )
        nHeight = std::max( nHeight, rSz.GetHeight() );
    return nHeight;
}

// #i29550#

// Calculate the maximum of (TopLineSize + TopLineDist) over all lowers:
static sal_uInt16 lcl_GetTopSpace( const SwRowFrm& rRow )
{
    sal_uInt16 nTopSpace = 0;
    for ( const SwCellFrm* pCurrLower = static_cast<const SwCellFrm*>(rRow.Lower()); pCurrLower;
          pCurrLower = static_cast<const SwCellFrm*>(pCurrLower->GetNext()) )
    {
        sal_uInt16 nTmpTopSpace = 0;
        if ( pCurrLower->Lower() && pCurrLower->Lower()->IsRowFrm() )
            nTmpTopSpace = lcl_GetTopSpace( *static_cast<const SwRowFrm*>(pCurrLower->Lower()) );
        else
        {
            const SwAttrSet& rSet = const_cast<SwCellFrm*>(pCurrLower)->GetFormat()->GetAttrSet();
            const SvxBoxItem& rBoxItem = rSet.GetBox();
            nTmpTopSpace = rBoxItem.CalcLineSpace( SvxBoxItemLine::TOP, true );
        }
        nTopSpace  = std::max( nTopSpace, nTmpTopSpace );
    }
    return nTopSpace;
}

// Calculate the maximum of TopLineDist over all lowers:
static sal_uInt16 lcl_GetTopLineDist( const SwRowFrm& rRow )
{
    sal_uInt16 nTopLineDist = 0;
    for ( const SwCellFrm* pCurrLower = static_cast<const SwCellFrm*>(rRow.Lower()); pCurrLower;
          pCurrLower = static_cast<const SwCellFrm*>(pCurrLower->GetNext()) )
    {
        sal_uInt16 nTmpTopLineDist = 0;
        if ( pCurrLower->Lower() && pCurrLower->Lower()->IsRowFrm() )
            nTmpTopLineDist = lcl_GetTopLineDist( *static_cast<const SwRowFrm*>(pCurrLower->Lower()) );
        else
        {
            const SwAttrSet& rSet = const_cast<SwCellFrm*>(pCurrLower)->GetFormat()->GetAttrSet();
            const SvxBoxItem& rBoxItem = rSet.GetBox();
            nTmpTopLineDist = rBoxItem.GetDistance( SvxBoxItemLine::TOP );
        }
        nTopLineDist = std::max( nTopLineDist, nTmpTopLineDist );
    }
    return nTopLineDist;
}

// Calculate the maximum of BottomLineSize over all lowers:
static sal_uInt16 lcl_GetBottomLineSize( const SwRowFrm& rRow )
{
    sal_uInt16 nBottomLineSize = 0;
    for ( const SwCellFrm* pCurrLower = static_cast<const SwCellFrm*>(rRow.Lower()); pCurrLower;
          pCurrLower = static_cast<const SwCellFrm*>(pCurrLower->GetNext()) )
    {
        sal_uInt16 nTmpBottomLineSize = 0;
        if ( pCurrLower->Lower() && pCurrLower->Lower()->IsRowFrm() )
        {
            const SwFrm* pRow = pCurrLower->GetLastLower();
            nTmpBottomLineSize = lcl_GetBottomLineSize( *static_cast<const SwRowFrm*>(pRow) );
        }
        else
        {
            const SwAttrSet& rSet = const_cast<SwCellFrm*>(pCurrLower)->GetFormat()->GetAttrSet();
            const SvxBoxItem& rBoxItem = rSet.GetBox();
            nTmpBottomLineSize = rBoxItem.CalcLineSpace( SvxBoxItemLine::BOTTOM, true ) -
                                 rBoxItem.GetDistance( SvxBoxItemLine::BOTTOM );
        }
        nBottomLineSize = std::max( nBottomLineSize, nTmpBottomLineSize );
    }
    return nBottomLineSize;
}

// Calculate the maximum of BottomLineDist over all lowers:
static sal_uInt16 lcl_GetBottomLineDist( const SwRowFrm& rRow )
{
    sal_uInt16 nBottomLineDist = 0;
    for ( const SwCellFrm* pCurrLower = static_cast<const SwCellFrm*>(rRow.Lower()); pCurrLower;
          pCurrLower = static_cast<const SwCellFrm*>(pCurrLower->GetNext()) )
    {
        sal_uInt16 nTmpBottomLineDist = 0;
        if ( pCurrLower->Lower() && pCurrLower->Lower()->IsRowFrm() )
        {
            const SwFrm* pRow = pCurrLower->GetLastLower();
            nTmpBottomLineDist = lcl_GetBottomLineDist( *static_cast<const SwRowFrm*>(pRow) );
        }
        else
        {
            const SwAttrSet& rSet = const_cast<SwCellFrm*>(pCurrLower)->GetFormat()->GetAttrSet();
            const SvxBoxItem& rBoxItem = rSet.GetBox();
            nTmpBottomLineDist = rBoxItem.GetDistance( SvxBoxItemLine::BOTTOM );
        }
        nBottomLineDist = std::max( nBottomLineDist, nTmpBottomLineDist );
    }
    return nBottomLineDist;
}

void SwRowFrm::Format( vcl::RenderContext* /*pRenderContext*/, const SwBorderAttrs *pAttrs )
{
    SWRECTFN( this )
    OSL_ENSURE( pAttrs, "SwRowFrm::Format without Attrs." );

    const bool bFix = mbFixSize;

    if ( !mbValidPrtArea )
    {
        //RowFrms don't have borders and so on therefore the PrtArea always
        //matches the Frm.
        mbValidPrtArea = true;
        maPrt.Left( 0 );
        maPrt.Top( 0 );
        maPrt.Width ( maFrm.Width() );
        maPrt.Height( maFrm.Height() );

        // #i29550#
        // Here we calculate the top-printing area for the lower cell frames
        SwTabFrm* pTabFrm = FindTabFrm();
        if ( pTabFrm->IsCollapsingBorders() )
        {
            const sal_uInt16 nTopSpace        = lcl_GetTopSpace(       *this );
            const sal_uInt16 nTopLineDist     = lcl_GetTopLineDist(    *this );
            const sal_uInt16 nBottomLineSize  = lcl_GetBottomLineSize( *this );
            const sal_uInt16 nBottomLineDist  = lcl_GetBottomLineDist( *this );

            const SwRowFrm* pPreviousRow = 0;

            // #i32456#
            // In order to calculate the top printing area for the lower cell
            // frames, we have to find the 'previous' row frame and compare
            // the bottom values of the 'previous' row with the 'top' values
            // of this row. The best way to find the 'previous' row is to
            // use the table structure:
            const SwTable* pTable = pTabFrm->GetTable();
            const SwTableLine* pPrevTabLine = 0;
            const SwRowFrm* pTmpRow = this;

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
                              pTmpRow->GetUpper()->GetUpper()->IsRowFrm() ?
                              static_cast<const SwRowFrm*>( pTmpRow->GetUpper()->GetUpper() ) :
                              0;
                }
            }

            // If we found a 'previous' row, we look for the appropriate row frame:
            if ( pPrevTabLine )
            {
                SwIterator<SwRowFrm,SwFormat> aIter( *pPrevTabLine->GetFrameFormat() );
                for ( SwRowFrm* pRow = aIter.First(); pRow; pRow = aIter.Next() )
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
                 pTabFrm->_InvalidatePrt();

            // If there are rows nested inside this row, the nested rows
            // may not have been calculated yet. Therefore the
            // ::lcl_CalcMinRowHeight( this ) operation later in this
            // function cannot consider the correct border values. We
            // have to trigger the invalidation of the outer row frame
            // manually:
            // Note: If any further invalidations should be necessary, we
            // should consider moving the invalidation stuff to the
            // appropriate SwNotify object.
            if ( GetUpper()->GetUpper()->IsRowFrm() &&
                 ( nBottomLineDist != GetBottomMarginForLowers() ||
                   nTopPrtMargin   != GetTopMarginForLowers() ) )
                GetUpper()->GetUpper()->_InvalidateSize();

            SetBottomMarginForLowers( nBottomLineDist );    //  3.
            SetBottomLineSize( nBottomLineSize );           //  4.
            SetTopMarginForLowers( nTopPrtMargin );         //  5.

        }
    }

    while ( !mbValidSize )
    {
        mbValidSize = true;

#if OSL_DEBUG_LEVEL > 0
        if ( HasFixSize() )
        {
            const SwFormatFrmSize &rFrmSize = GetFormat()->GetFrmSize();
            OSL_ENSURE( rFrmSize.GetSize().Height() > 0, "Hat ihn" );
        }
#endif
        const SwTwips nDiff = (Frm().*fnRect->fnGetHeight)() -
                              ( HasFixSize() && !IsRowSpanLine()
                                ? pAttrs->GetSize().Height()
                                // #i26945#
                                : ::lcl_CalcMinRowHeight( this,
                                    FindTabFrm()->IsConsiderObjsForMinCellHeight() ) );
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
        SwTwips nDiff = (GetUpper()->Prt().*fnRect->fnGetHeight)();
        SwFrm *pSibling = GetUpper()->Lower();
        do
        {   nDiff -= (pSibling->Frm().*fnRect->fnGetHeight)();
            pSibling = pSibling->GetNext();
        } while ( pSibling );
        if ( nDiff > 0 )
        {
            mbFixSize = false;
            Grow( nDiff );
            mbFixSize = bFix;
            mbValidSize = true;
        }
    }
}

void SwRowFrm::AdjustCells( const SwTwips nHeight, const bool bHeight )
{
    SwFrm *pFrm = Lower();
    if ( bHeight )
    {
        SwRootFrm *pRootFrm = getRootFrm();
        SWRECTFN( this )
        SwRect aOldFrm;

        while ( pFrm )
        {
            SwFrm* pNotify = 0;

            SwCellFrm* pCellFrm = static_cast<SwCellFrm*>(pFrm);

            // NEW TABLES
            // Which cells need to be adjusted if the current row changes
            // its height?

            // Current frame is a covered frame:
            // Set new height for covered cell and adjust master cell:
            if ( pCellFrm->GetTabBox()->getRowSpan() < 1 )
            {
                // Set height of current (covered) cell to new line height.
                const long nDiff = nHeight - (pCellFrm->Frm().*fnRect->fnGetHeight)();
                if ( nDiff )
                {
                    (pCellFrm->Frm().*fnRect->fnAddBottom)( nDiff );
                    pCellFrm->_InvalidatePrt();
                }
            }

            SwCellFrm* pToAdjust = 0;
            SwFrm* pToAdjustRow = 0;

            // If current frame is covered frame, we still want to adjust the
            // height of the cell starting the row span
            if ( pCellFrm->GetLayoutRowSpan() < 1 )
            {
                pToAdjust = const_cast< SwCellFrm*>(&pCellFrm->FindStartEndOfRowSpanCell( true, true ));
                pToAdjustRow = pToAdjust->GetUpper();
            }
            else
            {
                pToAdjust = pCellFrm;
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
                                 (pToAdjustRow->Frm().*fnRect->fnGetHeight)();

                if ( nRowSpan-- == 1 )
                    break;

                pToAdjustRow = pToAdjustRow->GetNext();
            }

            if ( pToAdjustRow && pToAdjustRow != this )
                pToAdjustRow->_InvalidateSize();

            const long nDiff = nSumRowHeight - (pToAdjust->Frm().*fnRect->fnGetHeight)();
            if ( nDiff )
            {
                aOldFrm = pToAdjust->Frm();
                (pToAdjust->Frm().*fnRect->fnAddBottom)( nDiff );
                pNotify = pToAdjust;
            }

            if ( pNotify )
            {
                if( pRootFrm && pRootFrm->IsAnyShellAccessible() && pRootFrm->GetCurrShell() )
                    pRootFrm->GetCurrShell()->Imp()->MoveAccessibleFrm( pNotify, aOldFrm );

                pNotify->_InvalidatePrt();
            }

            pFrm = pFrm->GetNext();
        }
    }
    else
    {   while ( pFrm )
        {
            pFrm->_InvalidateAll();
            pFrm = pFrm->GetNext();
        }
    }
    InvalidatePage();
}

void SwRowFrm::Cut()
{
    SwTabFrm *pTab = FindTabFrm();
    if ( pTab && pTab->IsFollow() && this == pTab->GetFirstNonHeadlineRow() )
    {
        pTab->FindMaster()->InvalidatePos();
    }

    SwLayoutFrm::Cut();
}

SwTwips SwRowFrm::GrowFrm( SwTwips nDist, bool bTst, bool bInfo )
{
    SwTwips nReal = 0;

    SwTabFrm* pTab = FindTabFrm();
    SWRECTFN( pTab )

    bool bRestrictTableGrowth;
    bool bHasFollowFlowLine = pTab->HasFollowFlowLine();

    if ( GetUpper()->IsTabFrm() )
    {
        const SwRowFrm* pFollowFlowRow = IsInSplitTableRow();
        bRestrictTableGrowth = pFollowFlowRow && !pFollowFlowRow->IsRowSpanLine();
    }
    else
    {
        OSL_ENSURE( GetUpper()->IsCellFrm(), "RowFrm->GetUpper neither table nor cell" );
        bRestrictTableGrowth = GetFollowRow() && bHasFollowFlowLine;
        OSL_ENSURE( !bRestrictTableGrowth || !GetNext(),
                "GetFollowRow for row frame that has a Next" );

        // There may still be some space left in my direct upper:
        const SwTwips nAdditionalSpace =
                (Frm().*fnRect->fnBottomDist)( (GetUpper()->GetUpper()->*fnRect->fnGetPrtBottom)() );
        if ( bRestrictTableGrowth && nAdditionalSpace > 0 )
        {
            nReal = std::min( nAdditionalSpace, nDist );
            nDist -= nReal;
            if ( !bTst )
                (Frm().*fnRect->fnAddBottom)( nReal );
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
        // to 'false' does not work, because the surrounding RowFrm
        // would set this to 'true'.
        pTab->SetFollowFlowLine( false );
    }

    nReal += SwLayoutFrm::GrowFrm( nDist, bTst, bInfo);

    pTab->SetRestrictTableGrowth( false );
    pTab->SetFollowFlowLine( bHasFollowFlowLine );

    //Update the height of the cells to the newest value.
    if ( !bTst )
    {
        SWRECTFNX( this )
        AdjustCells( (Prt().*fnRectX->fnGetHeight)() + nReal, true );
        if ( nReal )
            SetCompletePaint();
    }

    return nReal;
}

SwTwips SwRowFrm::ShrinkFrm( SwTwips nDist, bool bTst, bool bInfo )
{
    SWRECTFN( this )
    if( HasFixSize() )
    {
        AdjustCells( (Prt().*fnRect->fnGetHeight)(), true );
        return 0L;
    }

    // bInfo may be set to true by SwRowFrm::Format; we need to hangle this
    // here accordingly
    const bool bShrinkAnyway = bInfo;

    //Only shrink as much as the content of the biggest cell allows.
    SwTwips nRealDist = nDist;
    {
        const SwFormatFrmSize &rSz = GetFormat()->GetFrmSize();
        SwTwips nMinHeight = rSz.GetHeightSizeType() == ATT_MIN_SIZE ?
                             rSz.GetHeight() :
                             0;

        // Only necessary to calculate minimal row height if height
        // of pRow is at least nMinHeight. Otherwise nMinHeight is the
        // minimum height.
        if( nMinHeight < (Frm().*fnRect->fnGetHeight)() )
        {
            // #i26945#
            OSL_ENSURE( FindTabFrm(), "<SwRowFrm::ShrinkFrm(..)> - no table frame -> crash." );
            const bool bConsiderObjs( FindTabFrm()->IsConsiderObjsForMinCellHeight() );
            nMinHeight = lcl_CalcMinRowHeight( this, bConsiderObjs );
        }

        if ( ((Frm().*fnRect->fnGetHeight)() - nRealDist) < nMinHeight )
            nRealDist = (Frm().*fnRect->fnGetHeight)() - nMinHeight;
    }
    if ( nRealDist < 0 )
        nRealDist = 0;

    SwTwips nReal = nRealDist;
    if ( nReal )
    {
        if ( !bTst )
        {
            SwTwips nHeight = (Frm().*fnRect->fnGetHeight)();
            (Frm().*fnRect->fnSetHeight)( nHeight - nReal );

            if( IsVertical() && !IsVertLR() && !bRev )
                Frm().Pos().X() += nReal;
        }

        SwTwips nTmp = GetUpper()->Shrink( nReal, bTst );
        if ( !bShrinkAnyway && !GetNext() && nTmp != nReal )
        {
            //The last one gets the leftover in the upper and therefore takes
            //care (otherwise: endless loop)
            if ( !bTst )
            {
                nReal -= nTmp;
                SwTwips nHeight = (Frm().*fnRect->fnGetHeight)();
                (Frm().*fnRect->fnSetHeight)( nHeight + nReal );

                if( IsVertical() && !IsVertLR() && !bRev )
                    Frm().Pos().X() -= nReal;
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
                GetNext()->_InvalidatePos();
            _InvalidateAll();
            SetCompletePaint();

            SwTabFrm *pTab = FindTabFrm();
            if ( !pTab->IsRebuildLastLine()
                 && pTab->IsFollow()
                 && this == pTab->GetFirstNonHeadlineRow()
                 && !pTab->IsInRecalcLowerRow() )
            {
                SwTabFrm* pMasterTab = pTab->FindMaster();
                pMasterTab->InvalidatePos();
            }
        }
        AdjustCells( (Prt().*fnRect->fnGetHeight)() - nReal, true );
    }
    return nReal;
}

bool SwRowFrm::IsRowSplitAllowed() const
{
    // Fixed size rows are never allowed to split:
    if ( HasFixSize() )
    {
        OSL_ENSURE( ATT_FIX_SIZE == GetFormat()->GetFrmSize().GetHeightSizeType(), "pRow claims to have fixed size" );
        return false;
    }

    // Repeated headlines are never allowed to split:
    const SwTabFrm* pTabFrm = FindTabFrm();
    if ( pTabFrm->GetTable()->GetRowsToRepeat() > 0 &&
         pTabFrm->IsInHeadline( *this ) )
        return false;

    const SwTableLineFormat* pFrameFormat = static_cast<SwTableLineFormat*>(GetTabLine()->GetFrameFormat());
    const SwFormatRowSplit& rLP = pFrameFormat->GetRowSplit();
    return rLP.GetValue();
}

bool SwRowFrm::ShouldRowKeepWithNext() const
{
    // No KeepWithNext if nested in another table
    if ( GetUpper()->GetUpper()->IsCellFrm() )
        return false;

    const SwCellFrm* pCell = static_cast<const SwCellFrm*>(Lower());
    const SwFrm* pText = pCell->Lower();

    return pText && pText->IsTextFrm() &&
           static_cast<const SwTextFrm*>(pText)->GetTextNode()->GetSwAttrSet().GetKeep().GetValue();
}

SwCellFrm::SwCellFrm(const SwTableBox &rBox, SwFrm* pSib, bool bInsertContent)
    : SwLayoutFrm( rBox.GetFrameFormat(), pSib )
    , m_pTabBox( &rBox )
{
    mnFrmType = FRM_CELL;

    if ( !bInsertContent )
        return;

    //If a StartIdx is available, ContentFrms are added in the cell, otherwise
    //Rows have to be present and those are added.
    if ( rBox.GetSttIdx() )
    {
        sal_uLong nIndex = rBox.GetSttIdx();
        ::_InsertCnt( this, rBox.GetFrameFormat()->GetDoc(), ++nIndex );
    }
    else
    {
        const SwTableLines &rLines = rBox.GetTabLines();
        SwFrm *pTmpPrev = 0;
        for ( size_t i = 0; i < rLines.size(); ++i )
        {
            SwRowFrm *pNew = new SwRowFrm( *rLines[i], this, bInsertContent );
            pNew->InsertBehind( this, pTmpPrev );
            pTmpPrev = pNew;
        }
    }
}

void SwCellFrm::DestroyImpl()
{
    SwModify* pMod = GetFormat();
    if( pMod )
    {
        // At this stage the lower frames aren't destroyed already,
        // therefore we have to do a recursive dispose.
        SwRootFrm *pRootFrm = getRootFrm();
        if( pRootFrm && pRootFrm->IsAnyShellAccessible() &&
            pRootFrm->GetCurrShell() )
        {
            pRootFrm->GetCurrShell()->Imp()->DisposeAccessibleFrm( this, true );
        }

        pMod->Remove( this );           // remove,
        if( !pMod->HasWriterListeners() )
            delete pMod;                // and delete
    }

    SwLayoutFrm::DestroyImpl();
}

SwCellFrm::~SwCellFrm()
{
}

static bool lcl_ArrangeLowers( SwLayoutFrm *pLay, long lYStart, bool bInva )
{
    bool bRet = false;
    SwFrm *pFrm = pLay->Lower();
    SWRECTFN( pLay )
    while ( pFrm )
    {
        long nFrmTop = (pFrm->Frm().*fnRect->fnGetTop)();
        if( nFrmTop != lYStart )
        {
            bRet = true;
            const long lDiff = (*fnRect->fnYDiff)( lYStart, nFrmTop );
            const long lDiffX = lYStart - nFrmTop;
            (pFrm->Frm().*fnRect->fnSubTop)( -lDiff );
            (pFrm->Frm().*fnRect->fnAddBottom)( lDiff );
            pFrm->SetCompletePaint();
            if ( !pFrm->GetNext() )
                pFrm->SetRetouche();
            if( bInva )
                pFrm->Prepare( PREP_POS_CHGD );
            if ( pFrm->IsLayoutFrm() && static_cast<SwLayoutFrm*>(pFrm)->Lower() )
                lcl_ArrangeLowers( static_cast<SwLayoutFrm*>(pFrm),
                    (static_cast<SwLayoutFrm*>(pFrm)->Lower()->Frm().*fnRect->fnGetTop)()
                    + lDiffX, bInva );
            if ( pFrm->GetDrawObjs() )
            {
                for ( size_t i = 0; i < pFrm->GetDrawObjs()->size(); ++i )
                {
                    SwAnchoredObject* pAnchoredObj = (*pFrm->GetDrawObjs())[i];
                    // #i26945# - check, if anchored object
                    // is lower of layout frame by checking, if the anchor
                    // frame, which contains the anchor position, is a lower
                    // of the layout frame.
                    if ( !pLay->IsAnLower( pAnchoredObj->GetAnchorFrmContainingAnchPos() ) )
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
                    if ( dynamic_cast< const SwFlyFrm *>( pAnchoredObj ) !=  nullptr )
                    {
                        SwFlyFrm *pFly = static_cast<SwFlyFrm*>(pAnchoredObj);

                        // OD 2004-05-18 #i28701# - no direct move of objects,
                        // which are anchored to-paragraph/to-character, if
                        // the wrapping style influence has to be considered
                        // on the object positioning.
                        // #i52904# - no direct move of objects,
                        // whose vertical position doesn't depend on anchor frame.
                        const bool bDirectMove =
                                FAR_AWAY != pFly->Frm().Top() &&
                                bVertPosDepOnAnchor &&
                                !pFly->ConsiderObjWrapInfluenceOnObjPos();
                        if ( bDirectMove )
                        {
                            (pFly->Frm().*fnRect->fnSubTop)( -lDiff );
                            (pFly->Frm().*fnRect->fnAddBottom)( lDiff );
                            pFly->GetVirtDrawObj()->SetRectsDirty();
                            // --> OD 2004-08-17 - also notify view of <SdrObject>
                            // instance, which represents the Writer fly frame in
                            // the drawing layer
                            pFly->GetVirtDrawObj()->SetChanged();
                            // #i58280#
                            pFly->InvalidateObjRectWithSpaces();
                        }

                        if ( pFly->IsFlyInCntFrm() )
                        {
                            static_cast<SwFlyInCntFrm*>(pFly)->AddRefOfst( lDiff );
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
                        const SwTabFrm* pTabFrm = pLay->FindTabFrm();
                        // - save: check, if table frame is found.
                        if ( pTabFrm &&
                             !( pTabFrm->IsFollow() &&
                                pTabFrm->FindMaster()->IsRebuildLastLine() ) &&
                             pFly->IsFlyFreeFrm() )
                        {
                            SwPageFrm* pPageFrm = pFly->GetPageFrm();
                            SwPageFrm* pPageOfAnchor = pFrm->FindPageFrm();
                            if ( pPageFrm != pPageOfAnchor )
                            {
                                pFly->InvalidatePos();
                                if ( pPageFrm )
                                    pPageFrm->MoveFly( pFly, pPageOfAnchor );
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
                                                  (pFly->*fnRect->fnGetPrtTop)(),
                                                  bInva ) )
                        {
                            pFly->SetCompletePaint();
                        }
                    }
                    else if ( dynamic_cast< const SwAnchoredDrawObject *>( pAnchoredObj ) !=  nullptr )
                    {
                        // #i26945#
                        const SwTabFrm* pTabFrm = pLay->FindTabFrm();
                        if ( pTabFrm &&
                             !( pTabFrm->IsFollow() &&
                                pTabFrm->FindMaster()->IsRebuildLastLine() ) &&
                            (pAnchoredObj->GetFrameFormat().GetAnchor().GetAnchorId()
                                                            != FLY_AS_CHAR))
                        {
                            SwPageFrm* pPageFrm = pAnchoredObj->GetPageFrm();
                            SwPageFrm* pPageOfAnchor = pFrm->FindPageFrm();
                            if ( pPageFrm != pPageOfAnchor )
                            {
                                pAnchoredObj->InvalidateObjPos();
                                if ( pPageFrm )
                                {
                                    pPageFrm->RemoveDrawObjFromPage( *pAnchoredObj );
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
                            if ( bVert )
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
        if( !pFrm->IsColumnFrm() && !pFrm->IsCellFrm() )
            lYStart = (*fnRect->fnYInc)( lYStart,
                                        (pFrm->Frm().*fnRect->fnGetHeight)() );

        // Nowadays, the content inside a cell can flow into the follow table.
        // Thus, the cell may only grow up to the end of the environment.
        // So the content may have grown, but the cell could not grow.
        // Therefore we have to trigger a formatting for the frames, which do
        // not fit into the cell anymore:
        SwTwips nDistanceToUpperPrtBottom =
            (pFrm->Frm().*fnRect->fnBottomDist)( (pLay->*fnRect->fnGetPrtBottom)());
        // #i56146# - Revise fix of issue #i26945#
        // do *not* consider content inside fly frames, if it's an undersized paragraph.
        // #i26945# - consider content inside fly frames
        if ( nDistanceToUpperPrtBottom < 0 &&
             ( ( pFrm->IsInFly() &&
                 ( !pFrm->IsTextFrm() ||
                   !static_cast<SwTextFrm*>(pFrm)->IsUndersized() ) ) ||
               pFrm->IsInSplitTableRow() ) )
        {
            pFrm->InvalidatePos();
        }

        pFrm = pFrm->GetNext();
    }
    return bRet;
}

void SwCellFrm::Format( vcl::RenderContext* /*pRenderContext*/, const SwBorderAttrs *pAttrs )
{
    OSL_ENSURE( pAttrs, "CellFrm::Format, pAttrs ist 0." );
    const SwTabFrm* pTab = FindTabFrm();
    SWRECTFN( pTab )

    if ( !mbValidPrtArea )
    {
        mbValidPrtArea = true;

        //Adjust position.
        if ( Lower() )
        {
            SwTwips nTopSpace, nBottomSpace, nLeftSpace, nRightSpace;
            // #i29550#
            if ( pTab->IsCollapsingBorders() && !Lower()->IsRowFrm()  )
            {
                const SvxBoxItem& rBoxItem = pAttrs->GetBox();
                nLeftSpace   = rBoxItem.GetDistance( SvxBoxItemLine::LEFT );
                nRightSpace  = rBoxItem.GetDistance( SvxBoxItemLine::RIGHT );
                nTopSpace    =  static_cast<SwRowFrm*>(GetUpper())->GetTopMarginForLowers();
                nBottomSpace =  static_cast<SwRowFrm*>(GetUpper())->GetBottomMarginForLowers();
            }
            else
            {
                // OD 23.01.2003 #106895# - add 1st param to <SwBorderAttrs::CalcRight(..)>
                nLeftSpace   = pAttrs->CalcLeft( this );
                nRightSpace  = pAttrs->CalcRight( this );
                nTopSpace    = pAttrs->CalcTop();
                nBottomSpace = pAttrs->CalcBottom();
            }
            (this->*fnRect->fnSetXMargins)( nLeftSpace, nRightSpace );
            (this->*fnRect->fnSetYMargins)( nTopSpace, nBottomSpace );
        }
    }
    // #i26945#
    long nRemaining = GetTabBox()->getRowSpan() >= 1 ?
                      ::lcl_CalcMinCellHeight( this, pTab->IsConsiderObjsForMinCellHeight(), pAttrs ) :
                      0;
    if ( !mbValidSize )
    {
        mbValidSize = true;

        //The VarSize of the CellFrms is always the width.
        //The width is not variable though, it is defined by the format.
        //This predefined value however does not necessary match the actual
        //width. The width is calculated based on the attribute, the value in
        //the attribute matches the desired value of the TabFrm. Changes which
        //were done there are taken into account here proportionately.
        //If the cell doesn't have a neighbour anymore, it does not take the
        //attribute into account and takes the rest of the upper instead.
        SwTwips nWidth;
        if ( GetNext() )
        {
            const SwTwips nWish = pTab->GetFormat()->GetFrmSize().GetWidth();
            nWidth = pAttrs->GetSize().Width();

            OSL_ENSURE( nWish, "Table without width?" );
            OSL_ENSURE( nWidth <= nWish, "Width of cell larger than table." );
            OSL_ENSURE( nWidth > 0, "Box without width" );

            const long nPrtWidth = (pTab->Prt().*fnRect->fnGetWidth)();
            if ( nWish != nPrtWidth )
            {
                // Avoid rounding problems, at least for the new table model
                if ( pTab->GetTable()->IsNewModel() )
                {
                    // 1. sum of widths of cells up to this cell (in model)
                    const SwTableLine* pTabLine = GetTabBox()->GetUpper();
                    const SwTableBoxes& rBoxes = pTabLine->GetTabBoxes();
                    const SwTableBox* pTmpBox = 0;

                    SwTwips nSumWidth = 0;
                    size_t i = 0;
                    do
                    {
                        pTmpBox = rBoxes[ i++ ];
                        nSumWidth += pTmpBox->GetFrameFormat()->GetFrmSize().GetWidth();
                    }
                    while ( pTmpBox != GetTabBox() );

                    // 2. calculate actual width of cells up to this one
                    double nTmpWidth = nSumWidth;
                    nTmpWidth *= nPrtWidth;
                    nTmpWidth /= nWish;
                    nWidth = (SwTwips)nTmpWidth;

                    // 3. calculate frame widths of cells up to this one:
                    const SwFrm* pTmpCell = static_cast<const SwLayoutFrm*>(GetUpper())->Lower();
                    SwTwips nSumFrameWidths = 0;
                    while ( pTmpCell != this )
                    {
                        nSumFrameWidths += (pTmpCell->Frm().*fnRect->fnGetWidth)();
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
                    nWidth = (SwTwips)nTmpWidth;
                }
            }
        }
        else
        {
            OSL_ENSURE( pAttrs->GetSize().Width() > 0, "Box without width" );
            nWidth = (GetUpper()->Prt().*fnRect->fnGetWidth)();
            SwFrm *pPre = GetUpper()->Lower();
            while ( pPre != this )
            {
                nWidth -= (pPre->Frm().*fnRect->fnGetWidth)();
                pPre = pPre->GetNext();
            }
        }
        const long nDiff = nWidth - (Frm().*fnRect->fnGetWidth)();
        if( IsNeighbourFrm() && IsRightToLeft() )
            (Frm().*fnRect->fnSubLeft)( nDiff );
        else
            (Frm().*fnRect->fnAddRight)( nDiff );
        (Prt().*fnRect->fnAddRight)( nDiff );

        //Adjust the height, it's defined through the content and the border.
        const long nDiffHeight = nRemaining - (Frm().*fnRect->fnGetHeight)();
        if ( nDiffHeight )
        {
            if ( nDiffHeight > 0 )
            {
                //Validate again if no growth happened. Invalidation is done
                //through AdjustCells of the row.
                if ( !Grow( nDiffHeight ) )
                    mbValidSize = mbValidPrtArea = true;
            }
            else
            {
                //Only keep invalidated if shrinking was done; this can be
                //dismissed because all adjoined cells have to be the same size.
                if ( !Shrink( -nDiffHeight ) )
                    mbValidSize = mbValidPrtArea = true;
            }
        }
    }
    const SwFormatVertOrient &rOri = pAttrs->GetAttrSet().GetVertOrient();

    if ( !Lower() )
        return;

    // From now on, all operations are related to the table cell.
    SWREFRESHFN( this )

    SwPageFrm* pPg = 0;
    if ( !FindTabFrm()->IsRebuildLastLine() && text::VertOrientation::NONE != rOri.GetVertOrient() &&
    // #158225# no vertical alignment of covered cells
         !IsCoveredCell() &&
         (pPg = FindPageFrm())!=NULL )
    {
        if ( !Lower()->IsContentFrm() && !Lower()->IsSctFrm() && !Lower()->IsTabFrm() )
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
            SwRect aRect( Prt() ); aRect += Frm().Pos();
            for ( size_t i = 0; i < pPg->GetSortedObjs()->size(); ++i )
            {
                const SwAnchoredObject* pAnchoredObj = (*pPg->GetSortedObjs())[i];
                SwRect aTmp( pAnchoredObj->GetObjRect() );
                if ( aTmp.IsOver( aRect ) )
                {
                    const SwFrameFormat& rAnchoredObjFrameFormat = pAnchoredObj->GetFrameFormat();
                    const SwFormatSurround &rSur = rAnchoredObjFrameFormat.GetSurround();

                    if ( SURROUND_THROUGHT != rSur.GetSurround() )
                    {
                        // frames, which the cell is a lower of, aren't relevant
                        if ( dynamic_cast< const SwFlyFrm *>( pAnchoredObj ) !=  nullptr )
                        {
                            const SwFlyFrm *pFly =
                                    static_cast<const SwFlyFrm*>(pAnchoredObj);
                            if ( pFly->IsAnLower( this ) )
                                continue;
                        }

                        const SwFrm* pAnch = pAnchoredObj->GetAnchorFrm();
                        // #i43913#
                        // #i52904# - no vertical alignment,
                        // if object, anchored inside cell, has temporarly
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

        long nPrtHeight = (Prt().*fnRect->fnGetHeight)();
        if( ( bVertDir && ( nRemaining -= lcl_CalcTopAndBottomMargin( *this, *pAttrs ) ) < nPrtHeight ) ||
            (Lower()->Frm().*fnRect->fnGetTop)() != (this->*fnRect->fnGetPrtTop)() )
        {
            long nDiff = (Prt().*fnRect->fnGetHeight)() - nRemaining;
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
                long nTmp = (*fnRect->fnYInc)(
                                    (this->*fnRect->fnGetPrtTop)(), lTopOfst );
                if ( lcl_ArrangeLowers( this, nTmp, !bVertDir ) )
                    SetCompletePaint();
            }
        }
    }
    else
    {
        //Was an old alignment taken into account?
        if ( Lower()->IsContentFrm() )
        {
            const long lYStart = (this->*fnRect->fnGetPrtTop)();
            lcl_ArrangeLowers( this, lYStart, true );
        }
    }
}

void SwCellFrm::Modify( const SfxPoolItem* pOld, const SfxPoolItem * pNew )
{
    bool bAttrSetChg = pNew && RES_ATTRSET_CHG == pNew->Which();
    const SfxPoolItem *pItem = 0;

    if( bAttrSetChg )
        static_cast<const SwAttrSetChg*>(pNew)->GetChgSet()->GetItemState( RES_VERT_ORIENT, false, &pItem);
    else if (pNew && RES_VERT_ORIENT == pNew->Which())
        pItem = pNew;

    if ( pItem )
    {
        bool bInva = true;
        if ( text::VertOrientation::NONE == static_cast<const SwFormatVertOrient*>(pItem)->GetVertOrient() &&
             // OD 04.11.2003 #112910#
             Lower() && Lower()->IsContentFrm() )
        {
            SWRECTFN( this )
            const long lYStart = (this->*fnRect->fnGetPrtTop)();
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
        SwViewShell *pSh = getRootFrm()->GetCurrShell();
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
        SwFrm* pTmpUpper = GetUpper();
        while ( pTmpUpper->GetUpper() && !pTmpUpper->GetUpper()->IsTabFrm() )
            pTmpUpper = pTmpUpper->GetUpper();

        SwTabFrm* pTabFrm = static_cast<SwTabFrm*>(pTmpUpper->GetUpper());
        if ( pTabFrm->IsCollapsingBorders() )
        {
            // Invalidate lowers of this and next row:
            lcl_InvalidateAllLowersPrt( static_cast<SwRowFrm*>(pTmpUpper) );
            pTmpUpper = pTmpUpper->GetNext();
            if ( pTmpUpper )
                lcl_InvalidateAllLowersPrt( static_cast<SwRowFrm*>(pTmpUpper) );
            else
                pTabFrm->InvalidatePrt();
        }
    }

    SwLayoutFrm::Modify( pOld, pNew );
}

long SwCellFrm::GetLayoutRowSpan() const
{
    long nRet = GetTabBox()->getRowSpan();
    if ( nRet < 1 )
    {
        const SwFrm* pRow = GetUpper();
        const SwTabFrm* pTab = pRow ? static_cast<const SwTabFrm*>(pRow->GetUpper()) : NULL;

        if ( pTab && pTab->IsFollow() && pRow == pTab->GetFirstNonHeadlineRow() )
            nRet = -nRet;
    }
    return  nRet;
}

// #i103961#
void SwCellFrm::Cut()
{
    // notification for accessibility
    {
        SwRootFrm *pRootFrm = getRootFrm();
        if( pRootFrm && pRootFrm->IsAnyShellAccessible() )
        {
            SwViewShell* pVSh = pRootFrm->GetCurrShell();
            if ( pVSh && pVSh->Imp() )
            {
                pVSh->Imp()->DisposeAccessibleFrm( this );
            }
        }
    }

    SwLayoutFrm::Cut();
}

// Helper functions for repeated headlines:

bool SwTabFrm::IsInHeadline( const SwFrm& rFrm ) const
{
    OSL_ENSURE( IsAnLower( &rFrm ) && rFrm.IsInTab(),
             "SwTabFrm::IsInHeadline called for frame not lower of table" );

    const SwFrm* pTmp = &rFrm;
    while ( !pTmp->GetUpper()->IsTabFrm() )
        pTmp = pTmp->GetUpper();

    return GetTable()->IsHeadline( *static_cast<const SwRowFrm*>(pTmp)->GetTabLine() );
}

/*
 * If this is a master table, we can may assume, that there are at least
 * nRepeat lines in the table.
 * If this is a follow table, there are intermediate states for the table
 * layout, e.g., during deletion of rows, which makes it necessary to find
 * the first non-headline row by evaluating the headline flag at the row frame.
 */
SwRowFrm* SwTabFrm::GetFirstNonHeadlineRow() const
{
    SwRowFrm* pRet = const_cast<SwRowFrm*>(static_cast<const SwRowFrm*>(Lower()));
    if ( pRet )
    {
        if ( IsFollow() )
        {
            while ( pRet && pRet->IsRepeatedHeadline() )
                pRet = static_cast<SwRowFrm*>(pRet->GetNext());
        }
        else
        {
            sal_uInt16 nRepeat = GetTable()->GetRowsToRepeat();
            while ( pRet && nRepeat > 0 )
            {
                pRet = static_cast<SwRowFrm*>(pRet->GetNext());
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

bool SwTabFrm::IsLayoutSplitAllowed() const
{
    return GetFormat()->GetLayoutSplit().GetValue();
}

// #i29550#

sal_uInt16 SwTabFrm::GetBottomLineSize() const
{
    OSL_ENSURE( IsCollapsingBorders(),
            "BottomLineSize only required for collapsing borders" );

    OSL_ENSURE( Lower(), "Warning! Trying to prevent a crash" );

    const SwFrm* pTmp = GetLastLower();

    // #124755# Try to make code robust
    if ( !pTmp ) return 0;

    return static_cast<const SwRowFrm*>(pTmp)->GetBottomLineSize();
}

bool SwTabFrm::IsCollapsingBorders() const
{
    return static_cast<const SfxBoolItem&>(GetFormat()->GetAttrSet().Get( RES_COLLAPSING_BORDERS )).GetValue();
}

/// Local helper function to calculate height of first text row
static SwTwips lcl_CalcHeightOfFirstContentLine( const SwRowFrm& rSourceLine )
{
    // Find corresponding split line in master table
    const SwTabFrm* pTab = rSourceLine.FindTabFrm();
    SWRECTFN( pTab )
    const SwCellFrm* pCurrSourceCell = static_cast<const SwCellFrm*>(rSourceLine.Lower());

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
            pCurrSourceCell = static_cast<const SwCellFrm*>(pCurrSourceCell->GetNext());
            continue;
        }

        const SwFrm *pTmp = pCurrSourceCell->Lower();
        if ( pTmp )
        {
            SwTwips nTmpHeight = USHRT_MAX;
            // #i32456# Consider lower row frames
            if ( pTmp->IsRowFrm() )
            {
                const SwRowFrm* pTmpSourceRow = static_cast<const SwRowFrm*>(pCurrSourceCell->Lower());
                nTmpHeight = lcl_CalcHeightOfFirstContentLine( *pTmpSourceRow );
            }
            if ( pTmp->IsTabFrm() )
            {
                nTmpHeight = static_cast<const SwTabFrm*>(pTmp)->CalcHeightOfFirstContentLine();
            }
            else if ( pTmp->IsTextFrm() )
            {
                SwTextFrm* pTextFrm = const_cast<SwTextFrm*>(static_cast<const SwTextFrm*>(pTmp));
                pTextFrm->GetFormatted();
                nTmpHeight = pTextFrm->FirstLineHeight();
            }

            if ( USHRT_MAX != nTmpHeight )
            {
                const SwCellFrm* pPrevCell = pCurrSourceCell->GetPreviousCell();
                if ( pPrevCell )
                {
                    // If we are in a split row, there may be some space
                    // left in the cell frame of the master row.
                    // We look for the minimum of all first line heights;
                    SwTwips nReal = (pPrevCell->Prt().*fnRect->fnGetHeight)();
                    const SwFrm* pFrm = pPrevCell->Lower();
                    const SwFrm* pLast = pFrm;
                    while ( pFrm )
                    {
                        nReal -= (pFrm->Frm().*fnRect->fnGetHeight)();
                        pLast = pFrm;
                        pFrm = pFrm->GetNext();
                    }

                    // #i26831#, #i26520#
                    // The additional lower space of the current last.
                    // #115759# - do *not* consider the
                    // additional lower space for 'master' text frames
                    if ( pLast && pLast->IsFlowFrm() &&
                         ( !pLast->IsTextFrm() ||
                           !static_cast<const SwTextFrm*>(pLast)->GetFollow() ) )
                    {
                        nReal += SwFlowFrm::CastFlowFrm(pLast)->CalcAddLowerSpaceAsLastInTableCell();
                    }
                    // Don't forget the upper space and lower space,
                    // #115759# - do *not* consider the upper
                    // and the lower space for follow text frames.
                    if ( pTmp->IsFlowFrm() &&
                         ( !pTmp->IsTextFrm() ||
                           !static_cast<const SwTextFrm*>(pTmp)->IsFollow() ) )
                    {
                        nTmpHeight += SwFlowFrm::CastFlowFrm(pTmp)->CalcUpperSpace( NULL, pLast);
                        nTmpHeight += SwFlowFrm::CastFlowFrm(pTmp)->CalcLowerSpace();
                    }
                    // #115759# - consider additional lower
                    // space of <pTmp>, if contains only one line.
                    // In this case it would be the new last text frame, which
                    // would have no follow and thus would add this space.
                    if ( pTmp->IsTextFrm() &&
                         const_cast<SwTextFrm*>(static_cast<const SwTextFrm*>(pTmp))
                                            ->GetLineCount( COMPLETE_STRING ) == 1 )
                    {
                        nTmpHeight += SwFlowFrm::CastFlowFrm(pTmp)
                                        ->CalcAddLowerSpaceAsLastInTableCell();
                    }
                    if ( nReal > 0 )
                        nTmpHeight -= nReal;
                }
                else
                {
                    // pFirstRow is not a FollowFlowRow. In this case,
                    // we look for the maximum of all first line heights:
                    SwBorderAttrAccess aAccess( SwFrm::GetCache(), pCurrSourceCell );
                    const SwBorderAttrs &rAttrs = *aAccess.Get();
                    nTmpHeight += rAttrs.CalcTop() + rAttrs.CalcBottom();
                    // #i26250#
                    // Don't forget the upper space and lower space,
                    if ( pTmp->IsFlowFrm() )
                    {
                        nTmpHeight += SwFlowFrm::CastFlowFrm(pTmp)->CalcUpperSpace();
                        nTmpHeight += SwFlowFrm::CastFlowFrm(pTmp)->CalcLowerSpace();
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

        pCurrSourceCell = static_cast<const SwCellFrm*>(pCurrSourceCell->GetNext());
    }

    return ( LONG_MAX == nHeight ) ? 0 : nHeight;
}

/// Function to calculate height of first text row
SwTwips SwTabFrm::CalcHeightOfFirstContentLine() const
{
    SWRECTFN( this )

    const bool bDontSplit = !IsFollow() && !GetFormat()->GetLayoutSplit().GetValue();

    if ( bDontSplit )
    {
        // Table is not allowed to split: Take the whole height, that's all
        return (Frm().*fnRect->fnGetHeight)();
    }

    SwTwips nTmpHeight = 0;

    SwRowFrm* pFirstRow = GetFirstNonHeadlineRow();
    OSL_ENSURE( !IsFollow() || pFirstRow, "FollowTable without Lower" );

    // NEW TABLES
    if ( pFirstRow && pFirstRow->IsRowSpanLine() && pFirstRow->GetNext() )
        pFirstRow = static_cast<SwRowFrm*>(pFirstRow->GetNext());

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
            pFirstRow = static_cast<SwRowFrm*>(pFirstRow->GetNext());
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
        const SwTwips nFirstLineHeight = (pFirstRow->Frm().*fnRect->fnGetHeight)();

        if ( !bSplittable )
        {
            // pFirstRow is not splittable, but it is still possible that the line height of pFirstRow
            // actually is determined by a lower cell with rowspan = -1. In this case we should not
            // just return the height of the first line. Basically we need to get the height of the
            // line as it would be on the last page. Since this is quite complicated to calculate,
            // we only calculate the height of the first line.
            if ( pFirstRow->GetPrev() &&
                 static_cast<SwRowFrm*>(pFirstRow->GetPrev())->IsRowSpanLine() )
            {
                // Calculate maximum height of all cells with rowspan = 1:
                SwTwips nMaxHeight = 0;
                const SwCellFrm* pLower2 = static_cast<const SwCellFrm*>(pFirstRow->Lower());
                while ( pLower2 )
                {
                    if ( 1 == pLower2->GetTabBox()->getRowSpan() )
                    {
                        const SwTwips nCellHeight = lcl_CalcMinCellHeight( pLower2, true );
                        nMaxHeight = std::max( nCellHeight, nMaxHeight );
                    }
                    pLower2 = static_cast<const SwCellFrm*>(pLower2->GetNext());
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
            const_cast<SwTabFrm*>(this)->LockJoin();
            const SwTwips nHeightOfFirstContentLine = lcl_CalcHeightOfFirstContentLine( *pFirstRow );

            // Consider minimum row height:
            const SwFormatFrmSize &rSz = static_cast<const SwRowFrm*>(pFirstRow)->GetFormat()->GetFrmSize();
            const SwTwips nMinRowHeight = rSz.GetHeightSizeType() == ATT_MIN_SIZE ?
                                          rSz.GetHeight() : 0;

            nTmpHeight += std::max( nHeightOfFirstContentLine, nMinRowHeight );

            if ( !bOldJoinLock )
                const_cast<SwTabFrm*>(this)->UnlockJoin();
        }
    }

    return nTmpHeight;
}

// Some more functions for covered/covering cells. This way inclusion of
// SwCellFrm can be avoided

bool SwFrm::IsLeaveUpperAllowed() const
{
    const SwCellFrm* pThisCell = dynamic_cast<const SwCellFrm*>(this);
    return pThisCell && pThisCell->GetLayoutRowSpan() > 1;
}

bool SwFrm::IsCoveredCell() const
{
    const SwCellFrm* pThisCell = dynamic_cast<const SwCellFrm*>(this);
    return pThisCell && pThisCell->GetLayoutRowSpan() < 1;
}

bool SwFrm::IsInCoveredCell() const
{
    bool bRet = false;

    const SwFrm* pThis = this;
    while ( pThis && !pThis->IsCellFrm() )
        pThis = pThis->GetUpper();

    if ( pThis )
        bRet = pThis->IsCoveredCell();

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
