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

#include "pagefrm.hxx"
#include "rootfrm.hxx"
#include "cntfrm.hxx"
#include "viewsh.hxx"
#include "doc.hxx"
#include "docsh.hxx"
#include "viewimp.hxx"
#include "swtable.hxx"
#include "dflyobj.hxx"
#include "flyfrm.hxx"
#include "frmtool.hxx"
#include "frmfmt.hxx"
#include "dcontact.hxx"
#include <anchoreddrawobject.hxx>
#include <fmtanchr.hxx>
#include "viewopt.hxx"
#include "hints.hxx"
#include "dbg_lay.hxx"
#include <ftnidx.hxx>
#include <svl/itemiter.hxx>
#include <docary.hxx>
#include <editeng/keepitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/brshitem.hxx>
#include <editeng/boxitem.hxx>
#include <vcl/outdev.hxx>
#include <fmtlsplt.hxx>
#include <fmtrowsplt.hxx>
#include <fmtsrnd.hxx>
#include <fmtornt.hxx>
#include <fmtpdsc.hxx>
#include <fmtfsize.hxx>
#include <swtblfmt.hxx>
#include <ndtxt.hxx>
#include "tabfrm.hxx"
#include "rowfrm.hxx"
#include "cellfrm.hxx"
#include "flyfrms.hxx"
#include "txtfrm.hxx"       //HasFtn()
#include "htmltbl.hxx"
#include "sectfrm.hxx"  //SwSectionFrm
#include <fmtfollowtextflow.hxx>
#include <sortedobjs.hxx>
#include <objectformatter.hxx>
#include <layouter.hxx>
#include <switerator.hxx>

extern void AppendObjs( const SwFrmFmts *pTbl, sal_uLong nIndex,
                        SwFrm *pFrm, SwPageFrm *pPage );

using namespace ::com::sun::star;


/*************************************************************************
|*
|*  SwTabFrm::SwTabFrm(), ~SwTabFrm()
|*
|*************************************************************************/
SwTabFrm::SwTabFrm( SwTable &rTab, SwFrm* pSib ):
    SwLayoutFrm( rTab.GetFrmFmt(), pSib ),
    SwFlowFrm( (SwFrm&)*this ),
    pTable( &rTab )
{
    bComplete = bCalcLowers = bONECalcLowers = bLowersFormatted = bLockBackMove =
    bResizeHTMLTable = bHasFollowFlowLine = bIsRebuildLastLine =
    bRestrictTableGrowth = bRemoveFollowFlowLinePending = sal_False;
    // #i26945#
    bConsiderObjsForMinCellHeight = sal_True;
    bObjsDoesFit = sal_True;
    bFixSize = sal_False;     //Don't fall for import filter again.
    nType = FRMC_TAB;

    //Create the lines and insert them.
    const SwTableLines &rLines = rTab.GetTabLines();
    SwFrm *pTmpPrev = 0;
    for ( sal_uInt16 i = 0; i < rLines.size(); ++i )
    {
        SwRowFrm *pNew = new SwRowFrm( *rLines[i], this );
        if( pNew->Lower() )
        {
            pNew->InsertBehind( this, pTmpPrev );
            pTmpPrev = pNew;
        }
        else
            delete pNew;
    }
    OSL_ENSURE( Lower() && Lower()->IsRowFrm(), "SwTabFrm::SwTabFrm: No rows." );
}

SwTabFrm::SwTabFrm( SwTabFrm &rTab ) :
    SwLayoutFrm( rTab.GetFmt(), &rTab ),
    SwFlowFrm( (SwFrm&)*this ),
    pTable( rTab.GetTable() )
{
    bComplete = bONECalcLowers = bCalcLowers = bLowersFormatted = bLockBackMove =
    bResizeHTMLTable = bHasFollowFlowLine = bIsRebuildLastLine =
    bRestrictTableGrowth = bRemoveFollowFlowLinePending = sal_False;
    // #i26945#
    bConsiderObjsForMinCellHeight = sal_True;
    bObjsDoesFit = sal_True;
    bFixSize = sal_False;     //Don't fall for import filter again.
    nType = FRMC_TAB;

    SetFollow( rTab.GetFollow() );
    rTab.SetFollow( this );
}

extern const SwTable   *pColumnCacheLastTable;
extern const SwTabFrm  *pColumnCacheLastTabFrm;
extern const SwFrm     *pColumnCacheLastCellFrm;
extern const SwTable   *pRowCacheLastTable;
extern const SwTabFrm  *pRowCacheLastTabFrm;
extern const SwFrm     *pRowCacheLastCellFrm;

SwTabFrm::~SwTabFrm()
{
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
}

/*************************************************************************
|*
|*  SwTabFrm::JoinAndDelFollows()
|*
|*************************************************************************/
void SwTabFrm::JoinAndDelFollows()
{
    SwTabFrm *pFoll = GetFollow();
    if ( pFoll->HasFollow() )
        pFoll->JoinAndDelFollows();
    pFoll->Cut();
    SetFollow( pFoll->GetFollow() );
    delete pFoll;
}

/*************************************************************************
|*
|*  SwTabFrm::RegistFlys()
|*
|*************************************************************************/
void SwTabFrm::RegistFlys()
{
    OSL_ENSURE( Lower() && Lower()->IsRowFrm(), "No rows." );

    SwPageFrm *pPage = FindPageFrm();
    if ( pPage )
    {
        SwRowFrm *pRow = (SwRowFrm*)Lower();
        do
        {
            pRow->RegistFlys( pPage );
            pRow = (SwRowFrm*)pRow->GetNext();
        } while ( pRow );
    }
}

/*************************************************************************
|*  Some prototypes
|*************************************************************************/
void SwInvalidateAll( SwFrm *pFrm, long nBottom );
static void lcl_RecalcRow( SwRowFrm& rRow, long nBottom );
static sal_Bool lcl_ArrangeLowers( SwLayoutFrm *pLay, long lYStart, sal_Bool bInva );
// #i26945# - add parameter <_bOnlyRowsAndCells> to control
// that only row and cell frames are formatted.
static sal_Bool lcl_InnerCalcLayout( SwFrm *pFrm,
                                      long nBottom,
                                      bool _bOnlyRowsAndCells = false );
// OD 2004-02-18 #106629# - correct type of 1st parameter
// #i26945# - add parameter <_bConsiderObjs> in order to
// control, if floating screen objects have to be considered for the minimal
// cell height.
static SwTwips lcl_CalcMinRowHeight( const SwRowFrm *pRow,
                                          const sal_Bool _bConsiderObjs );
static SwTwips lcl_CalcTopAndBottomMargin( const SwLayoutFrm&, const SwBorderAttrs& );

/*************************************************************************
|*  START: local helper functions for repeated headlines
|*************************************************************************/

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

/*************************************************************************
|*  END: local helper functions for repeated headlines
|*************************************************************************/

/*************************************************************************
|*  START: local helper functions for splitting row frames
|*************************************************************************/

//
// Local helper function to insert a new follow flow line
//
static SwRowFrm* lcl_InsertNewFollowFlowLine( SwTabFrm& rTab, const SwFrm& rTmpRow, bool bRowSpanLine )
{
    OSL_ENSURE( rTmpRow.IsRowFrm(), "No row frame to copy for FollowFlowLine" );
    const SwRowFrm& rRow = (SwRowFrm&)rTmpRow;

    rTab.SetFollowFlowLine( sal_True );
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
            for ( sal_uInt16 i = 0; i < pLowerFrm->GetDrawObjs()->Count(); ++i )
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
                    if ( pAnchoredObj->GetFrmFmt().GetAnchor().GetAnchorId()
                            == FLY_AS_CHAR )
                    {
                        pAnchoredObj->AnchorFrm()
                                ->Prepare( PREP_FLY_ATTR_CHG,
                                           &(pAnchoredObj->GetFrmFmt()) );
                    }
                    if ( pAnchoredObj->ISA(SwFlyFrm) )
                    {
                        SwFlyFrm *pFly = static_cast<SwFlyFrm*>(pAnchoredObj);
                        pFly->GetVirtDrawObj()->SetRectsDirty();
                        pFly->GetVirtDrawObj()->SetChanged();
                    }
                }

                // If anchored object is a fly frame, invalidate its lower objects
                if ( pAnchoredObj->ISA(SwFlyFrm) )
                {
                    SwFlyFrm *pFly = static_cast<SwFlyFrm*>(pAnchoredObj);
                    ::lcl_InvalidateLowerObjs( *pFly, _bMoveObjsOutOfRange, _pPageFrm );
                }
            }
        }
        pLowerFrm = pLowerFrm->GetNext();
    }
}
//
// Local helper function to shrink all lowers of rRow to 0 height
//
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
                           sal_False );
        // TODO: Optimize number of frames which are set to 0 height
        // we have to start with the last lower frame, otherwise
        // the shrink will not shrink the current cell
        SwFrm* pTmp = rToAdjust.GetLastLower();

        if ( pTmp && pTmp->IsRowFrm() )
        {
            SwRowFrm* pTmpRow = (SwRowFrm*)pTmp;
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
                    SwRowFrm* pTmpRow = (SwRowFrm*)((SwTabFrm*)pTmp)->Lower();
                    while ( pTmpRow )
                    {
                        lcl_ShrinkCellsAndAllContent( *pTmpRow );
                        pTmpRow = (SwRowFrm*)pTmpRow->GetNext();
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
                               sal_False );
        }

        pCurrMasterCell = static_cast<SwCellFrm*>(pCurrMasterCell->GetNext());
    }
}

//
// Local helper function to move the content from rSourceLine to rDestLine
// The content is inserted behind the last content in the corresponding
// cell in rDestLine.
//
static void lcl_MoveRowContent( SwRowFrm& rSourceLine, SwRowFrm& rDestLine )
{
    SwCellFrm* pCurrDestCell = (SwCellFrm*)rDestLine.Lower();
    SwCellFrm* pCurrSourceCell = (SwCellFrm*)rSourceLine.Lower();

    // Move content of follow cells into master cells
    while ( pCurrSourceCell )
    {
        if ( pCurrSourceCell->Lower() && pCurrSourceCell->Lower()->IsRowFrm() )
        {
            SwRowFrm* pTmpSourceRow = (SwRowFrm*)pCurrSourceCell->Lower();
            while ( pTmpSourceRow )
            {
                // #125926# Achtung! It is possible,
                // that pTmpSourceRow->IsFollowFlowRow() but pTmpDestRow
                // cannot be found. In this case, we have to move the complete
                // row.
                SwRowFrm* pTmpDestRow = (SwRowFrm*)pCurrDestCell->Lower();

                if ( pTmpSourceRow->IsFollowFlowRow() && pTmpDestRow )
                {
                    // move content from follow flow row to pTmpDestRow:
                    while ( pTmpDestRow->GetNext() )
                        pTmpDestRow = (SwRowFrm*)pTmpDestRow->GetNext();

                    OSL_ENSURE( pTmpDestRow->GetFollowRow() == pTmpSourceRow, "Table contains node" );

                    lcl_MoveRowContent( *pTmpSourceRow, *pTmpDestRow );
                    pTmpDestRow->SetFollowRow( pTmpSourceRow->GetFollowRow() );
                    pTmpSourceRow->Remove();
                    delete pTmpSourceRow;
                }
                else
                {
                    // move complete row:
                    pTmpSourceRow->Remove();
                    pTmpSourceRow->InsertBefore( pCurrDestCell, 0 );
                }

                pTmpSourceRow = (SwRowFrm*)pCurrSourceCell->Lower();
            }
        }
        else
        {
            SwFrm *pTmp = ::SaveCntnt( (SwCellFrm*)pCurrSourceCell );
            if ( pTmp )
            {
                // NEW TABLES
                SwCellFrm* pDestCell = static_cast<SwCellFrm*>(pCurrDestCell);
                if ( pDestCell->GetTabBox()->getRowSpan() < 1 )
                    pDestCell = & const_cast<SwCellFrm&>(pDestCell->FindStartEndOfRowSpanCell( true, true ));

                // Find last content
                SwFrm* pFrm = pDestCell->GetLastLower();
                ::RestoreCntnt( pTmp, pDestCell, pFrm, true );
            }
        }
        pCurrDestCell = (SwCellFrm*)pCurrDestCell->GetNext();
        pCurrSourceCell = (SwCellFrm*)pCurrSourceCell->GetNext();
    }
}

//
// Local helper function to move all footnotes in rRowFrm from
// the footnote boss of rSource to the footnote boss of rDest.
//
static void lcl_MoveFootnotes( SwTabFrm& rSource, SwTabFrm& rDest, SwLayoutFrm& rRowFrm )
{
    if ( !rSource.GetFmt()->GetDoc()->GetFtnIdxs().empty() )
    {
        SwFtnBossFrm* pOldBoss = rSource.FindFtnBossFrm( sal_True );
        SwFtnBossFrm* pNewBoss = rDest.FindFtnBossFrm( sal_True );
        rRowFrm.MoveLowerFtns( 0, pOldBoss, pNewBoss, sal_True );
    }
}

//
// Local helper function to handle nested table cells before the split process
//
static void lcl_PreprocessRowsInCells( SwTabFrm& rTab, SwRowFrm& rLastLine,
                                SwRowFrm& rFollowFlowLine, SwTwips nRemain )
{
    SwCellFrm* pCurrLastLineCell = (SwCellFrm*)rLastLine.Lower();
    SwCellFrm* pCurrFollowFlowLineCell = (SwCellFrm*)rFollowFlowLine.Lower();

    SWRECTFN( pCurrLastLineCell )

    //
    // Move content of follow cells into master cells
    //
    while ( pCurrLastLineCell )
    {
        if ( pCurrLastLineCell->Lower() && pCurrLastLineCell->Lower()->IsRowFrm() )
        {
            SwTwips nTmpCut = nRemain;
            SwRowFrm* pTmpLastLineRow = (SwRowFrm*)pCurrLastLineCell->Lower();

            // #i26945#
            SwTwips nCurrentHeight =
                    lcl_CalcMinRowHeight( pTmpLastLineRow,
                                          rTab.IsConsiderObjsForMinCellHeight() );
            while ( pTmpLastLineRow && pTmpLastLineRow->GetNext() && nTmpCut > nCurrentHeight )
            {
                nTmpCut -= nCurrentHeight;
                pTmpLastLineRow = (SwRowFrm*)pTmpLastLineRow->GetNext();
                // #i26945#
                nCurrentHeight =
                    lcl_CalcMinRowHeight( pTmpLastLineRow,
                                          rTab.IsConsiderObjsForMinCellHeight() );
            }

            //
            // pTmpLastLineRow does not fit to the line or it is the last line
            //
            if ( pTmpLastLineRow )
            {
                //
                // Check if we can move pTmpLastLineRow to the follow table,
                // or if we have to split the line:
                //
                SwFrm* pCell = pTmpLastLineRow->Lower();
                bool bTableLayoutToComplex = false;
                long nMinHeight = 0;

                //
                // We have to take into account:
                // 1. The fixed height of the row
                // 2. The borders of the cells inside the row
                // 3. The minimum height of the row
                //
                if ( pTmpLastLineRow->HasFixSize() )
                    nMinHeight = (pTmpLastLineRow->Frm().*fnRect->fnGetHeight)();
                else
                {
                    while ( pCell )
                    {
                        if ( ((SwCellFrm*)pCell)->Lower() &&
                             ((SwCellFrm*)pCell)->Lower()->IsRowFrm() )
                        {
                            bTableLayoutToComplex = true;
                            break;
                        }

                        SwBorderAttrAccess aAccess( SwFrm::GetCache(), pCell );
                        const SwBorderAttrs &rAttrs = *aAccess.Get();
                        nMinHeight = Max( nMinHeight, lcl_CalcTopAndBottomMargin( *(SwLayoutFrm*)pCell, rAttrs ) );
                        pCell = pCell->GetNext();
                    }

                    const SwFmtFrmSize &rSz = pTmpLastLineRow->GetFmt()->GetFrmSize();
                    if ( rSz.GetHeightSizeType() == ATT_MIN_SIZE )
                        nMinHeight = Max( nMinHeight, rSz.GetHeight() );
                }

                //
                // 1. Case:
                // The line completely fits into the master table.
                // Nevertheless, we build a follow (otherwise painting problems
                // with empty cell).
                //
                // 2. Case:
                // The line has to be split, the minimum height still fits into
                // the master table, and the table structure is not to complex.
                //
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
                    pTmpLastLineRow = (SwRowFrm*)pTmpLastLineRow->GetNext();
                }

                //
                // The following lines have to be moved:
                //
                while ( pTmpLastLineRow )
                {
                    SwRowFrm* pTmp = (SwRowFrm*)pTmpLastLineRow->GetNext();
                    lcl_MoveFootnotes( rTab, *rTab.GetFollow(), *pTmpLastLineRow );
                    pTmpLastLineRow->Remove();
                    pTmpLastLineRow->InsertBefore( pCurrFollowFlowLineCell, 0 );
                    pTmpLastLineRow->Shrink( ( pTmpLastLineRow->Frm().*fnRect->fnGetHeight)() );
                    pCurrFollowFlowLineCell->Grow( ( pTmpLastLineRow->Frm().*fnRect->fnGetHeight)() );
                    pTmpLastLineRow = pTmp;
                }
            }
        }

        pCurrLastLineCell = (SwCellFrm*)pCurrLastLineCell->GetNext();
        pCurrFollowFlowLineCell = (SwCellFrm*)pCurrFollowFlowLineCell->GetNext();
    }
}

//
// Local helper function to handle nested table cells after the split process
//
static void lcl_PostprocessRowsInCells( SwTabFrm& rTab, SwRowFrm& rLastLine )
{
    SwCellFrm* pCurrMasterCell = (SwCellFrm*)rLastLine.Lower();
    while ( pCurrMasterCell )
    {
        if ( pCurrMasterCell->Lower() &&
             pCurrMasterCell->Lower()->IsRowFrm() )
        {
            SwRowFrm* pRowFrm = static_cast<SwRowFrm*>(pCurrMasterCell->GetLastLower());

            if ( NULL != pRowFrm->GetPrev() && !pRowFrm->ContainsCntnt() )
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
                delete pFollowRow;
                ::SwInvalidateAll( pCurrMasterCell, LONG_MAX );
            }
        }

        pCurrMasterCell = (SwCellFrm*)pCurrMasterCell->GetNext();
    }
}

//
// Local helper function to re-calculate the split line.
//
inline void TableSplitRecalcLock( SwFlowFrm *pTab ) { pTab->LockJoin(); }
inline void TableSplitRecalcUnlock( SwFlowFrm *pTab ) { pTab->UnlockJoin(); }

static bool lcl_RecalcSplitLine( SwRowFrm& rLastLine, SwRowFrm& rFollowLine,
                          SwTwips nRemainingSpaceForLastRow )
{
    bool bRet = true;

    SwTabFrm& rTab = (SwTabFrm&)*rLastLine.GetUpper();

    //
    // If there are nested cells in rLastLine, the recalculation of the last
    // line needs some preprocessing.
    //
    lcl_PreprocessRowsInCells( rTab, rLastLine, rFollowLine, nRemainingSpaceForLastRow );

    //
    // Here the recalculation process starts:
    //
    rTab.SetRebuildLastLine( sal_True );
    // #i26945#
    rTab.SetDoesObjsFit( sal_True );
    SWRECTFN( rTab.GetUpper() )

    // #i26945# - invalidate and move floating screen
    // objects 'out of range'
    ::lcl_InvalidateLowerObjs( rLastLine, true );
    //
    // manipulate row and cell sizes
    //
    // #i26945# - Do *not* consider floating screen objects
    // for the minimal cell height.
    rTab.SetConsiderObjsForMinCellHeight( sal_False );
    ::lcl_ShrinkCellsAndAllContent( rLastLine );
    rTab.SetConsiderObjsForMinCellHeight( sal_True );

    //
    // invalidate last line
    //
    ::SwInvalidateAll( &rLastLine, LONG_MAX );

    //
    // Lock this tab frame and its follow
    //
    bool bUnlockMaster = false;
    bool bUnlockFollow = false;
    SwTabFrm* pMaster = rTab.IsFollow() ? (SwTabFrm*)rTab.FindMaster() : 0;
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

    //
    // Do the recalculation
    //
    lcl_RecalcRow( rLastLine, LONG_MAX );
    // #115759# - force a format of the last line in order to
    // get the correct height.
    rLastLine.InvalidateSize();
    rLastLine.Calc();

    //
    // Unlock this tab frame and its follow
    //
    if ( bUnlockFollow )
        ::TableSplitRecalcUnlock( rTab.GetFollow() );
    if ( bUnlockMaster )
        ::TableSplitRecalcUnlock( pMaster );

    //
    // If there are nested cells in rLastLine, the recalculation of the last
    // line needs some postprocessing.
    //
    lcl_PostprocessRowsInCells( rTab, rLastLine );

    //
    // Do a couple of checks on the current situation.
    //
    // If we are not happy with the current situation we return false.
    // This will start a new try to split the table, this time we do not
    // try to split the table rows.
    //

    //
    // 1. Check if table fits to its upper.
    // #i26945# - include check, if objects fit
    //
    const SwTwips nDistanceToUpperPrtBottom =
            (rTab.Frm().*fnRect->fnBottomDist)( (rTab.GetUpper()->*fnRect->fnGetPrtBottom)());
    if ( nDistanceToUpperPrtBottom < 0 || !rTab.DoesObjsFit() )
        bRet = false;

    //
    // 2. Check if each cell in the last line has at least one content frame.
    //
    // Note: a FollowFlowRow may contains empty cells!
    //
    if ( bRet )
    {
        if ( !rLastLine.IsInFollowFlowRow() )
        {
            SwCellFrm* pCurrMasterCell = (SwCellFrm*)rLastLine.Lower();
            while ( pCurrMasterCell )
            {
                if ( !pCurrMasterCell->ContainsCntnt() && pCurrMasterCell->GetTabBox()->getRowSpan() >= 1 )
                {
                    bRet = false;
                    break;
                }
                pCurrMasterCell = (SwCellFrm*)pCurrMasterCell->GetNext();
            }
        }
    }

    //
    // 3. Check if last line does not contain any content:
    //
    if ( bRet )
    {
        if ( !rLastLine.ContainsCntnt() )
        {
            bRet = false;
        }
    }


    //
    // 4. Check if follow flow line does not contain content:
    //
    if ( bRet )
    {
        if ( !rFollowLine.IsRowSpanLine() && !rFollowLine.ContainsCntnt() )
        {
            bRet = false;
        }
    }

    if ( bRet )
    {
        //
        // Everything looks fine. Splitting seems to be successful. We invalidate
        // rFollowLine to force a new formatting.
        //
        ::SwInvalidateAll( &rFollowLine, LONG_MAX );
    }
    else
    {
        //
        // Splitting the table row gave us an unexpected result.
        // Everything has to be prepared for a second try to split
        // the table, this time without splitting the row.
        //
        ::SwInvalidateAll( &rLastLine, LONG_MAX );
    }

    rTab.SetRebuildLastLine( sal_False );
    // #i26945#
    rTab.SetDoesObjsFit( sal_True );

    return bRet;
}

//
// Sets the correct height for all spanned cells
//
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

//
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

/*************************************************************************
|*  END: local helper functions for splitting row frames
|*************************************************************************/

//
// Function to remove the FollowFlowLine of rTab.
// The content of the FollowFlowLine is moved to the associated line in the
// master table.
//
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
    SetFollowFlowLine( sal_False );

    // #140081# Make code robust.
    if ( !pFollowFlowLine || !pLastLine )
        return true;

    // Move content
    lcl_MoveRowContent( *pFollowFlowLine, *(SwRowFrm*)pLastLine );

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
            lcl_MoveFootnotes( *GetFollow(), *this, (SwRowFrm&)*pRow );

            pRow->Remove();
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
    delete pFollowFlowLine;

    return bJoin;
}

// #i26945# - Floating screen objects are no longer searched.
static bool lcl_FindSectionsInRow( const SwRowFrm& rRow )
{
    bool bRet = false;
    SwCellFrm* pLower = (SwCellFrm*)rRow.Lower();
    while ( pLower )
    {
        if ( pLower->IsVertical() != rRow.IsVertical() )
            return true;

        SwFrm* pTmpFrm = pLower->Lower();
        while ( pTmpFrm )
        {
            if ( pTmpFrm->IsRowFrm() )
            {
                bRet = lcl_FindSectionsInRow( *(SwRowFrm*)pTmpFrm );
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

        pLower = (SwCellFrm*)pLower->GetNext();
    }
    return bRet;
}

/*************************************************************************
|*
|*  SwTabFrm::Split(), Join()
|*
|*************************************************************************/
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

    //
    // Make pRow point to the line that does not fit anymore:
    //
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

    //
    // bSplitRowAllowed: Row may be split according to its attributes.
    // bTryToSplit:      Row will never be split if bTryToSplit = false.
    //                   This can either be passed as a parameter, indicating
    //                   that we are currently doing the second try to split the
    //                   table, or it will be set to falseunder certain
    //                   conditions that are not suitable for splitting
    //                   the row.
    //
    bool bSplitRowAllowed = pRow->IsRowSplitAllowed();

    // #i29438#
    // #i26945# - Floating screen objects no longer forbid
    // a splitting of the table row.
    // Special DoNotSplit case 1:
    // Search for sections inside pRow:
    //
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
        //
        // First case: One of the repeated headline does not fit to the page anymore.
        // At least one more non-heading row has to stay in this table in
        // order to avoid loops:
        //
        OSL_ENSURE( !GetIndPrev(), "Table is supposed to be at beginning" );
        bKeepNextRow = true;
    }
    else if ( !GetIndPrev() && nRepeat == nRowCount )
    {
        //
        // Second case: The first non-headline row does not fit to the page.
        // If it is not allowed to be split, or it contains a sub-row that
        // is not allowed to be split, we keep the row in this table:
        //
        if ( bTryToSplit && bSplitRowAllowed )
        {
            // Check if there are (first) rows inside this row,
            // which are not allowed to be split.
            SwCellFrm* pLowerCell = pRow ? (SwCellFrm*)pRow->Lower() : 0;
            while ( pLowerCell )
            {
                if ( pLowerCell->Lower() && pLowerCell->Lower()->IsRowFrm() )
                {
                    const SwRowFrm* pLowerRow = (SwRowFrm*)pLowerCell->Lower();
                    if ( !pLowerRow->IsRowSplitAllowed() &&
                        (pLowerRow->Frm().*fnRect->fnGetHeight)() >
                        nRemainingSpaceForLastRow )
                    {
                        bKeepNextRow = true;
                        break;
                    }
                }
                pLowerCell = (SwCellFrm*)pLowerCell->GetNext();
            }
        }
        else
            bKeepNextRow = true;
    }

    //
    // Better keep the next row in this table:
    //
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

    //
    // No more row to split or to move to follow table:
    //
    if ( !pRow )
        return bRet;

    //
    // We try to split the row if
    // - the attributes of the row are set accordingly and
    // - we are allowed to do so
    // - the it should not keep with the next row
    //
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

    //
    // If we do not indent to split pRow, we check if we are
    // allowed to move pRow to a follow. Otherwise we return
    // false, indicating an error
    //
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

    //
    // Build follow table if not already done:
    //
    sal_Bool bNewFollow;
    SwTabFrm *pFoll;
    if ( GetFollow() )
    {
        pFoll = GetFollow();
        bNewFollow = sal_False;
    }
    else
    {
        bNewFollow = sal_True;
        pFoll = new SwTabFrm( *this );

        //
        // We give the follow table an initial width.
        //
        (pFoll->Frm().*fnRect->fnAddWidth)( (Frm().*fnRect->fnGetWidth)() );
        (pFoll->Prt().*fnRect->fnAddWidth)( (Prt().*fnRect->fnGetWidth)() );
        (pFoll->Frm().*fnRect->fnSetLeft)( (Frm().*fnRect->fnGetLeft)() );

        //
        // Insert the new follow table
        //
        pFoll->InsertBehind( GetUpper(), this );

        //
        // Repeat the headlines.
        //
        for ( nRowCount = 0; nRowCount < nRepeat; ++nRowCount )
        {
            // Insert new headlines:
            bDontCreateObjects = sal_True;              //frmtool
            SwRowFrm* pHeadline = new SwRowFrm(
                                    *GetTable()->GetTabLines()[ nRowCount ], this );
            pHeadline->SetRepeatedHeadline( true );
            bDontCreateObjects = sal_False;
            pHeadline->InsertBefore( pFoll, 0 );

            SwPageFrm *pPage = pHeadline->FindPageFrm();
            const SwFrmFmts *pTbl = GetFmt()->GetDoc()->GetSpzFrmFmts();
            if( !pTbl->empty() )
            {
                sal_uLong nIndex;
                SwCntntFrm* pFrm = pHeadline->ContainsCntnt();
                while( pFrm )
                {
                    nIndex = pFrm->GetNode()->GetIndex();
                    AppendObjs( pTbl, nIndex, pFrm, pPage );
                    pFrm = pFrm->GetNextCntntFrm();
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
        SwFrm* pNxt = 0;
        SwFrm* pInsertBehind = pFoll->GetLastLower();

        while ( pRow )
        {
            pNxt = pRow->GetNext();
            nRet += (pRow->Frm().*fnRect->fnGetHeight)();
            // The footnotes do not have to be moved, this is done in the
            // MoveFwd of the follow table!!!
            pRow->Remove();
            pRow->InsertBehind( pFoll, pInsertBehind );
            pRow->_InvalidateAll();
            pInsertBehind = pRow;
            pRow = static_cast<SwRowFrm*>(pNxt);
        }
    }
    else
    {
        SwFrm* pNxt = 0;
        SwFrm* pPasteBefore = HasFollowFlowLine() ?
                              pFollowRow->GetNext() :
                              pFoll->GetFirstNonHeadlineRow();

        while ( pRow )
        {
            pNxt = pRow->GetNext();
            nRet += (pRow->Frm().*fnRect->fnGetHeight)();

            // The footnotes have to be moved:
            lcl_MoveFootnotes( *this, *GetFollow(), *pRow );

            pRow->Remove();
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
            pRow->Remove();
            pRow->_InvalidateAll();
            pRow->InsertBehind( this, pPrv );
            pRow->CheckDirChange();
            pPrv = pRow;
            pRow = pNxt;
        }

        SetFollow( pFoll->GetFollow() );
        SetFollowFlowLine( pFoll->HasFollowFlowLine() );
        delete pFoll;

        Grow( nHeight );
    }

    return true;
}

/*************************************************************************
|*
|*  SwTabFrm::MakeAll()
|*
|*************************************************************************/
void SwInvalidatePositions( SwFrm *pFrm, long nBottom )
{
    // LONG_MAX == nBottom means we have to calculate all
    sal_Bool bAll = LONG_MAX == nBottom;
    SWRECTFN( pFrm )
    do
    {   pFrm->_InvalidatePos();
        pFrm->_InvalidateSize();
        if( pFrm->IsLayoutFrm() )
        {
            if ( ((SwLayoutFrm*)pFrm)->Lower() )
            {
                ::SwInvalidatePositions( ((SwLayoutFrm*)pFrm)->Lower(), nBottom);
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
    sal_Bool bAll = LONG_MAX == nBottom;
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
            pFrm->Prepare( PREP_CLEAR );

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
            lcl_InvalidateAllLowersPrt( (SwLayoutFrm*)pFrm );
        else
        {
            pFrm->_InvalidatePrt();
            pFrm->_InvalidateSize();
            pFrm->SetCompletePaint();
        }

        pFrm = pFrm->GetNext();
    }
}
// <-- collapsing

bool SwCntntFrm::CalcLowers( SwLayoutFrm* pLay, const SwLayoutFrm* pDontLeave,
                                 long nBottom, bool bSkipRowSpanCells )
{
    if ( !pLay )
        return sal_True;

    // LONG_MAX == nBottom means we have to calculate all
    bool bAll = LONG_MAX == nBottom;
    bool bRet = sal_False;
    SwCntntFrm *pCnt = pLay->ContainsCntnt();
    SWRECTFN( pLay )

    // FME 2007-08-30 #i81146# new loop control
    sal_uInt16 nLoopControlRuns = 0;
    const sal_uInt16 nLoopControlMax = 10;
    const SwModify* pLoopControlCond = 0;

    while ( pCnt && pDontLeave->IsAnLower( pCnt ) )
    {
        // #115759# - check, if a format of content frame is
        // possible. Thus, 'copy' conditions, found at the beginning of
        // <SwCntntFrm::MakeAll(..)>, and check these.
        const bool bFormatPossible = !pCnt->IsJoinLocked() &&
                                     ( !pCnt->IsTxtFrm() ||
                                       !static_cast<SwTxtFrm*>(pCnt)->IsLocked() ) &&
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
            pCnt->Calc();
            // OD 2004-05-11 #i28701# - usage of new method <::FormatObjsAtFrm(..)>
            // to format the floating screen objects
            // #i46941# - frame has to be valid
            // Note: frame could be invalid after calling its format, if it's locked.
            OSL_ENSURE( !pCnt->IsTxtFrm() ||
                    pCnt->IsValid() ||
                    static_cast<SwTxtFrm*>(pCnt)->IsJoinLocked(),
                    "<SwCntntFrm::CalcLowers(..)> - text frame invalid and not locked." );
            if ( pCnt->IsTxtFrm() && pCnt->IsValid() )
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
                        pCnt = pLay->ContainsCntnt();
                        continue;
                    }

#if OSL_DEBUG_LEVEL > 1
                    OSL_FAIL( "LoopControl in SwCntntFrm::CalcLowers" );
#endif
                }
            }
            pCnt->GetUpper()->Calc();
        }
        if( ! bAll && (*fnRect->fnYDiff)((pCnt->Frm().*fnRect->fnGetTop)(), nBottom) > 0 )
            break;
        pCnt = pCnt->GetNextCntntFrm();
    }
    return bRet;
}

// #i26945# - add parameter <_bOnlyRowsAndCells> to control
// that only row and cell frames are formatted.
static sal_Bool lcl_InnerCalcLayout( SwFrm *pFrm,
                                      long nBottom,
                                      bool _bOnlyRowsAndCells )
{
    // LONG_MAX == nBottom means we have to calculate all
    sal_Bool bAll = LONG_MAX == nBottom;
    sal_Bool bRet = sal_False;
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
            pFrm->Calc();
            if( static_cast<SwLayoutFrm*>(pFrm)->Lower() )
                bRet |= lcl_InnerCalcLayout( static_cast<SwLayoutFrm*>(pFrm)->Lower(), nBottom);

            // NEW TABLES
            SwCellFrm* pThisCell = dynamic_cast<SwCellFrm*>(pFrm);
            if ( pThisCell && pThisCell->GetTabBox()->getRowSpan() < 1 )
            {
                SwCellFrm& rToCalc = const_cast<SwCellFrm&>(pThisCell->FindStartEndOfRowSpanCell( true, true ));
                bRet |= !rToCalc.IsValid();
                rToCalc.Calc();
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
    // #i26945# - For correct appliance of the 'straightforward
    // object positioning process, it's needed to notify that the page frame,
    // on which the given layout frame is in, is in its layout process.
    SwPageFrm* pPageFrm = rRow.FindPageFrm();
    if ( pPageFrm && !pPageFrm->IsLayoutInProgress() )
        pPageFrm->SetLayoutInProgress( true );
    else
        pPageFrm = 0L;

    // FME 2007-08-30 #i81146# new loop control
    sal_uInt16 nLoopControlRuns_1 = 0;
    sal_uInt16 nLoopControlStage_1 = 0;
    const sal_uInt16 nLoopControlMax = 10;

    bool bCheck = true;
    do
    {
        // FME 2007-08-30 #i81146# new loop control
        sal_uInt16 nLoopControlRuns_2 = 0;
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
            bCheck = SwCntntFrm::CalcLowers( &rRow, rRow.GetUpper(), nBottom, true );

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
                        bCheck  |= SwCntntFrm::CalcLowers( &rToRecalc, &rToRecalc, nBottom, false );
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

    // #i26945#
    if ( pPageFrm )
        pPageFrm->SetLayoutInProgress( false );
}

static void lcl_RecalcTable( SwTabFrm& rTab,
                                  SwLayoutFrm *pFirstRow,
                                  SwLayNotify &rNotify )
{
    if ( rTab.Lower() )
    {
        if ( !pFirstRow )
        {
            pFirstRow = (SwLayoutFrm*)rTab.Lower();
            rNotify.SetLowersComplete( sal_True );
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
                !dynamic_cast<const SwSectionFrm*>(pPrev)->GetSection() )
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
            (pSct = pSct->GetUpper()->GetUpper())->IsSctFrm() )
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
SwFrm* sw_FormatNextCntntForKeep( SwTabFrm* pTabFrm )
{
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
            pNxt->MakeAll();
        else
            pNxt->Calc();
    }

    return pNxt;
}

void SwTabFrm::MakeAll()
{
    if ( IsJoinLocked() || StackHack::IsLocked() || StackHack::Count() > 50 )
        return;

    if ( HasFollow() )
    {
        SwTabFrm* pFollowFrm = (SwTabFrm*)GetFollow();
        OSL_ENSURE( !pFollowFrm->IsJoinLocked() || !pFollowFrm->IsRebuildLastLine(),
                "SwTabFrm::MakeAll for master while follow is in RebuildLastLine()" );
        if ( pFollowFrm->IsJoinLocked() && pFollowFrm->IsRebuildLastLine() )
            return;
    }

    PROTOCOL_ENTER( this, PROT_MAKEALL, 0, 0 )

    LockJoin(); //I don't want to be destroyed on the way.
    SwLayNotify aNotify( this );    //does the notification in the DTor
    // If pos is invalid, we have to call a SetInvaKeep at aNotify.
    // Otherwise the keep atribute would not work in front of a table.
    const sal_Bool bOldValidPos = GetValidPosFlag();

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
        SetRemoveFollowFlowLinePending( sal_False );
    }

    if ( bResizeHTMLTable ) //Optimized interplay with grow/shrink of the content
    {
        bResizeHTMLTable = sal_False;
        SwHTMLTableLayout *pLayout = GetTable()->GetHTMLTableLayout();
        if ( pLayout )
            bCalcLowers = pLayout->Resize(
                            pLayout->GetBrowseWidthByTabFrm( *this ), sal_False );
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
    const bool bFtnsInDoc = !GetFmt()->GetDoc()->GetFtnIdxs().empty();
    sal_Bool bMoveable;
    const sal_Bool bFly     = IsInFly();

    SwBorderAttrAccess  *pAccess= new SwBorderAttrAccess( SwFrm::GetCache(), this );
    const SwBorderAttrs *pAttrs = pAccess->Get();

    // The beloved keep attribute
    const bool bKeep = IsKeep( pAttrs->GetAttrSet() );

    // All rows should keep together
    // OD 2004-05-25 #i21478# - don't split table, if it has to keep with next
    const bool bDontSplit = !IsFollow() &&
                            ( !GetFmt()->GetLayoutSplit().GetValue() || bKeep );

    // The number of repeated headlines
    const sal_uInt16 nRepeat = GetTable()->GetRowsToRepeat();

    // This flag indicates that we are allowed to try to split the
    // table rows.
    bool bTryToSplit = true;

    // #131283#
    // Indicates that two individual rows may keep together, based on the keep
    // attribute set at the first paragraph in the first cell.
    const bool bTableRowKeep = !bDontSplit && GetFmt()->GetDoc()->get(IDocumentSettingAccess::TABLE_ROW_KEEP);

    // The Magic Move: Used for the table row keep feature.
    // If only the last row of the table wants to keep (implicitely by setting
    // keep for the first paragraph in the first cell), and this table does
    // not have a next, the last line will be cut. Loop prevention: Only
    // one try.
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
        if ( pPre && pPre->IsTabFrm() && ((SwTabFrm*)pPre)->GetFollow() == this)
        {
            if ( !MoveFwd( bMakePage, sal_False ) )
                bMakePage = false;
            bMovedFwd = true;
        }
    }

    int nUnSplitted = 5; // Just another loop control :-(
    SWRECTFN( this )
    while ( !bValidPos || !bValidSize || !bValidPrtArea )
    {
        if ( sal_True == (bMoveable = IsMoveable()) )
            if ( CheckMoveFwd( bMakePage, bKeep && KEEPTAB, bMovedBwd ) )
            {
                bMovedFwd = true;
                bCalcLowers = sal_True;
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
                    delete pAccess;
                    bCalcLowers |= pLayout->Resize(
                        pLayout->GetBrowseWidthByTabFrm( *this ), sal_False );
                    pAccess = new SwBorderAttrAccess( SwFrm::GetCache(), this );
                    pAttrs = pAccess->Get();
                }

                bValidPrtArea = sal_False;
                aNotify.SetLowersComplete( sal_False );
            }
            SwFrm *pPre;
            if ( bKeep || (0 != (pPre = FindPrev()) &&
                           pPre->GetAttrSet()->GetKeep().GetValue()) )
            {
                bCalcLowers = sal_True;
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

        if ( !bValidSize || !bValidPrtArea )
        {
            const long nOldPrtWidth = (Prt().*fnRect->fnGetWidth)();
            const long nOldFrmWidth = (Frm().*fnRect->fnGetWidth)();
            const Point aOldPrtPos  = (Prt().*fnRect->fnGetPos)();
            Format( pAttrs );

            SwHTMLTableLayout *pLayout = GetTable()->GetHTMLTableLayout();
            if ( pLayout &&
                 ((Prt().*fnRect->fnGetWidth)() != nOldPrtWidth ||
                  (Frm().*fnRect->fnGetWidth)() != nOldFrmWidth) )
            {
                delete pAccess;
                bCalcLowers |= pLayout->Resize(
                        pLayout->GetBrowseWidthByTabFrm( *this ), sal_False );
                pAccess= new SwBorderAttrAccess( SwFrm::GetCache(), this );
                pAttrs = pAccess->Get();
            }
            if ( aOldPrtPos != (Prt().*fnRect->fnGetPos)() )
                aNotify.SetLowersComplete( sal_False );
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
                    SwTabFrm *pMaster = (SwTabFrm*)FindMaster();
                    sal_Bool bDummy;
                    if ( ShouldBwdMoved( pMaster->GetUpper(), sal_False, bDummy ) )
                        pMaster->InvalidatePos();
                }
            }
            SwFtnBossFrm *pOldBoss = bFtnsInDoc ? FindFtnBossFrm( sal_True ) : 0;
            sal_Bool bReformat;
            if ( MoveBwd( bReformat ) )
            {
                SWREFRESHFN( this )
                bMovedBwd = true;
                aNotify.SetLowersComplete( sal_False );
                if ( bFtnsInDoc )
                    MoveLowerFtns( 0, pOldBoss, 0, sal_True );
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
                            delete pAccess;
                            bCalcLowers |= pHTMLLayout->Resize(
                                pHTMLLayout->GetBrowseWidthByTabFrm( *this ),
                                sal_False );

                            pAccess= new SwBorderAttrAccess(
                                        SwFrm::GetCache(), this );
                            pAttrs = pAccess->Get();
                        }

                        bValidPrtArea = sal_False;
                        Format( pAttrs );
                    }
                    lcl_RecalcTable( *this, 0, aNotify );
                    bLowersFormatted = sal_True;
                    if ( bKeep && KEEPTAB )
                    {
                        //
                        // Consider case that table is inside another table,
                        // because it has to be avoided, that superior table
                        // is formatted.
                        // Thus, find next content, table or section
                        // and, if a section is found, get its first
                        // content.
                        if ( 0 != sw_FormatNextCntntForKeep( this ) && !GetNext() )
                        {
                            bValidPos = sal_False;
                        }
                    }
                }
            }
        }

        //Again an invalid value? - do it again...
        if ( !bValidPos || !bValidSize || !bValidPrtArea )
            continue;

        // check, if calculation of table frame is ready.

        /// OD 23.10.2002 #103517# - Local variable <nDistanceToUpperPrtBottom>
        ///     Introduce local variable and init it with the distance from the
        ///     table frame bottom to the bottom of the upper printing area.
        /// Note: negative values denotes the situation that table frame doesn't
        ///     fit in its upper.

        SwTwips nDistanceToUpperPrtBottom =
                (Frm().*fnRect->fnBottomDist)( (GetUpper()->*fnRect->fnGetPrtBottom)());

        /// OD 23.10.2002 #103517# - In online layout try to grow upper of table
        /// frame, if table frame doesn't fit in its upper.
        const ViewShell *pSh = getRootFrm()->GetCurrShell();
        const bool bBrowseMode = pSh && pSh->GetViewOptions()->getBrowseMode();
        if ( nDistanceToUpperPrtBottom < 0 && bBrowseMode )
        {
            if ( GetUpper()->Grow( -nDistanceToUpperPrtBottom ) )
            {
                // upper is grown --> recalculate <nDistanceToUpperPrtBottom>
                nDistanceToUpperPrtBottom =
                    (Frm().*fnRect->fnBottomDist)( (GetUpper()->*fnRect->fnGetPrtBottom)());
            }
        }

        // If there is still some space left in the upper, we check if we
        // can join some rows of the follow.
        // Setting bLastRowHasToMoveToFollow to true means we want to force
        // the table to be split! Only skip this if condition once.
        if( nDistanceToUpperPrtBottom >= 0 && !bLastRowHasToMoveToFollow )
        {
            // OD 23.10.2002 - translate german commentary
            // If there is space left in the upper printing area, join as for trial
            // at least one further row of an existing follow.
            if ( !bSplit && GetFollow() )
            {
                sal_Bool bDummy;
                if ( GetFollow()->ShouldBwdMoved( GetUpper(), sal_False, bDummy ) )
                {
                    SwFrm *pTmp = GetUpper();
                    SwTwips nDeadLine = (pTmp->*fnRect->fnGetPrtBottom)();
                    if ( bBrowseMode )
                        nDeadLine += pTmp->Grow( LONG_MAX, sal_True );
                    if( (Frm().*fnRect->fnBottomDist)( nDeadLine ) > 0 )
                    {
                        //
                        // First, we remove an existing follow flow line.
                        //
                        if ( HasFollowFlowLine() )
                        {
                            SwFrm* pLastLine = const_cast<SwFrm*>(GetLastLower());
                            RemoveFollowFlowLine();
                            // invalidate and rebuild last row
                            if ( pLastLine )
                            {
                                ::SwInvalidateAll( pLastLine, LONG_MAX );
                                SetRebuildLastLine( sal_True );
                                lcl_RecalcRow( static_cast<SwRowFrm&>(*pLastLine), LONG_MAX );
                                SetRebuildLastLine( sal_False );
                            }

                            SwFrm* pRow = GetFollow()->GetFirstNonHeadlineRow();

                            if ( !pRow || !pRow->GetNext() )
                                //The follow becomes empty and invalid for this reason.
                                Join();

                            continue;
                        }

                        //
                        // If there is no follow flow line, we move the first
                        // row in the follow table to the master table.
                        //
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
                            const sal_Bool bMoveFtns = bFtnsInDoc && !GetFollow()->IsJoinLocked();

                            SwFtnBossFrm *pOldBoss = 0;
                            if ( bMoveFtns )
                                pOldBoss = pRowToMove->FindFtnBossFrm( sal_True );

                            SwFrm* pNextRow = pRowToMove->GetNext();

                            if ( !pNextRow )
                                //The follow becomes empty and invalid for this reason.
                                Join();
                            else
                            {
                                pRowToMove->Cut();
                                pRowToMove->Paste( this );
                            }

                            //Displace the footnotes!
                            if ( bMoveFtns )
                                if ( ((SwLayoutFrm*)pRowToMove)->MoveLowerFtns(
                                     0, pOldBoss, FindFtnBossFrm( sal_True ), sal_True ) )
                                    GetUpper()->Calc();

                            pRowToMove = pNextRow;
                        }

                        if ( nOld != (Frm().*fnRect->fnGetHeight)() )
                            lcl_RecalcTable( *this, (SwLayoutFrm*)pRow, aNotify );

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
                    delete pAccess;

                    //
                    // Consider case that table is inside another table, because
                    // it has to be avoided, that superior table is formatted.
                    // Thus, find next content, table or section and, if a section
                    // is found, get its first content.
                    const SwFrm* pTmpNxt = sw_FormatNextCntntForKeep( this );

                    pAccess= new SwBorderAttrAccess( SwFrm::GetCache(), this );
                    pAttrs = pAccess->Get();

                    // The last row wants to keep with the frame behind the table.
                    // Check if the next frame is on a different page and valid.
                    // In this case we do a magic trick:
                    if ( !bKeep && !GetNext() && pTmpNxt && pTmpNxt->IsValid() )
                    {
                        bValidPos = sal_False;
                        bLastRowHasToMoveToFollow = true;
                    }
                }
            }

            if ( IsValid() )
            {
                if ( bCalcLowers )
                {
                    lcl_RecalcTable( *this, 0, aNotify );
                    bLowersFormatted = sal_True;
                    bCalcLowers = sal_False;
                }
                else if ( bONECalcLowers )
                {
                    lcl_RecalcRow( static_cast<SwRowFrm&>(*Lower()), LONG_MAX );
                    bONECalcLowers = sal_False;
                }
            }
            continue;
        }

        //I don't fit in the higher-ranked element anymore, therefore it's the
        //right moment to do some preferably constructive changes.

        //If I'm NOT allowed to leave the parent Frm, I've got a problem.
        // Following Artur Dent, we do the only thing that you can do with
        // an unsolvable problem: We ignore it with all our power.
        if ( !bMoveable )
        {
            if ( bCalcLowers && IsValid() )
            {
                lcl_RecalcTable( *this, 0, aNotify );
                bLowersFormatted = sal_True;
                bCalcLowers = sal_False;
            }
            else if ( bONECalcLowers )
            {
                lcl_RecalcRow( static_cast<SwRowFrm&>(*Lower()), LONG_MAX );
                bONECalcLowers = sal_False;
            }

            // It does not make sense to cut off the last line if we are
            // not moveable:
            bLastRowHasToMoveToFollow = false;

            continue;
        }

        if ( bCalcLowers && IsValid() )
        {
            lcl_RecalcTable( *this, 0, aNotify );
            bLowersFormatted = sal_True;
            bCalcLowers = sal_False;
            if( !IsValid() )
                continue;
        }

        //
        // First try to split the table. Condition:
        // 1. We have at least one non headline row
        // 2. If this row wants to keep, we need an additional row
        // 3. The table is allowed to split or we do not have an pIndPrev:
        //
        SwFrm* pIndPrev = GetIndPrev();
        const SwRowFrm* pFirstNonHeadlineRow = GetFirstNonHeadlineRow();

        if ( pFirstNonHeadlineRow && nUnSplitted > 0 &&
             ( !bTableRowKeep || pFirstNonHeadlineRow->GetNext() || !pFirstNonHeadlineRow->ShouldRowKeepWithNext() ) &&
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
                                        GetUpper()->Grow( LONG_MAX, sal_True ) );

                ::lcl_RecalcRow( static_cast<SwRowFrm&>(*Lower()), nDeadLine );
                bLowersFormatted = sal_True;
                aNotify.SetLowersComplete( sal_True );

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
                // if we do not have an (in)direkt Prev, we split anyway.
                if( (*fnRect->fnYDiff)(nDeadLine, nBreakLine) >=0 || !pIndPrev )
                {
                    aNotify.SetLowersComplete( sal_False );
                    bSplit = true;

                    //
                    // An existing follow flow line has to be removed.
                    //
                    if ( HasFollowFlowLine() )
                        RemoveFollowFlowLine();

                    const bool bSplitError = !Split( nDeadLine, bTryToSplit, bTableRowKeep );
                    if( !bTryToSplit && !bSplitError && nUnSplitted > 0 )
                        --nUnSplitted;

                    // #i29771# Two tries to split the table
                    // If an error occurred during splitting. We start a second
                    // try, this time without splitting of table rows.
                    if ( bSplitError )
                    {
                        if ( HasFollowFlowLine() )
                            RemoveFollowFlowLine();
                    }

                    // #119477#
                    // If splitting the table was successfull or not,
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
                        bValidPos = sal_False;
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
                            delete pAccess;

                            GetFollow()->MakeAll();

                            pAccess= new SwBorderAttrAccess( SwFrm::GetCache(), this );
                            pAttrs = pAccess->Get();

                            ((SwTabFrm*)GetFollow())->SetLowersFormatted(sal_False);
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
                                SwFrm* pNxt = ((SwFrm*)GetFollow())->FindNext();
                                if ( pNxt )
                                {
                                    // OD 26.08.2003 #i18103# - no formatting
                                    // of found next frame, if its a follow
                                    // section of the 'ColLocked' section,
                                    // the follow table is in.
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
                                        pNxt->Calc();
                                    }
                                }
                            }
                            --nStack;
                        }
                        else if ( GetFollow() == GetNext() )
                            ((SwTabFrm*)GetFollow())->MoveFwd( sal_True, sal_False );
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
            ((SwSectionFrm*)GetUpper()->GetUpper()->GetUpper())->MoveAllowed(this) )
        {
            bMovedFwd = false;
        }

        // #i29771# Reset bTryToSplit flag on change of upper
        const SwFrm* pOldUpper = GetUpper();

        //Let's see if we find some place anywhere...
        if ( !bMovedFwd && !MoveFwd( bMakePage, sal_False ) )
            bMakePage = false;

        // #i29771# Reset bSplitError flag on change of upper
        if ( GetUpper() != pOldUpper )
        {
            bTryToSplit = true;
            nUnSplitted = 5;
        }

        SWREFRESHFN( this )
        bCalcLowers = sal_True;
        bMovedFwd = true;
        aNotify.SetLowersComplete( sal_False );
        if ( IsFollow() )
        {
            //To avoid oscillations now invalid master should drop behind.
            SwTabFrm *pTab = FindMaster();
            if ( pTab->GetUpper() )
                pTab->GetUpper()->Calc();
            pTab->Calc();
            pTab->SetLowersFormatted( sal_False );
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
            //During floating back the upper was animated to do a full repaint,
            //we can now skip this after the whole back and forth floating.
            GetUpper()->ResetCompletePaint();

        if ( bCalcLowers && IsValid() )
        {
            // #i44910# - format of lower frames unnecessary
            // and can cause layout loops, if table doesn't fit and isn't
            // allowed to split.
            SwTwips nDistToUpperPrtBottom =
                (Frm().*fnRect->fnBottomDist)( (GetUpper()->*fnRect->fnGetPrtBottom)());
            if ( nDistToUpperPrtBottom >= 0 || bTryToSplit )
            {
                lcl_RecalcTable( *this, 0, aNotify );
                bLowersFormatted = sal_True;
                bCalcLowers = sal_False;
            }
#if OSL_DEBUG_LEVEL > 0
            else
            {
                OSL_FAIL( "debug assertion: <SwTabFrm::MakeAll()> - format of table lowers suppressed by fix i44910" );
            }
#endif
        }

    } //while ( !bValidPos || !bValidSize || !bValidPrtArea )

    //If my direct predecessor is my master now, it can destroy me during the
    //next best opportunity.
    if ( IsFollow() )
    {
        SwFrm *pPre = GetPrev();
        if ( pPre && pPre->IsTabFrm() && ((SwTabFrm*)pPre)->GetFollow() == this)
            pPre->InvalidatePos();
    }

    bCalcLowers = bONECalcLowers = sal_False;
    delete pAccess;
    UnlockJoin();
    if ( bMovedFwd || bMovedBwd || !bOldValidPos )
        aNotify.SetInvaKeep();
}

/*************************************************************************
|*
|*  SwTabFrm::CalcFlyOffsets()
|*
|*  Description:       Calculate the offsets arising because of FlyFrames
|*
|*************************************************************************/
sal_Bool SwTabFrm::CalcFlyOffsets( SwTwips& rUpper,
                               long& rLeftOffset,
                               long& rRightOffset ) const
{
    sal_Bool bInvalidatePrtArea = sal_False;
    const SwPageFrm *pPage = FindPageFrm();
    const SwFlyFrm* pMyFly = FindFlyFrm();

    // --> #108724# Page header/footer content doesn't have to wrap around
    //              floating screen objects

    const IDocumentSettingAccess* pIDSA = GetFmt()->getIDocumentSettingAccess();
    const bool bWrapAllowed = pIDSA->get(IDocumentSettingAccess::USE_FORMER_TEXT_WRAPPING) ||
                                ( !IsInFtn() && 0 == FindFooterOrHeader() );

    if ( pPage->GetSortedObjs() && bWrapAllowed )
    {
        SWRECTFN( this )
        const bool bConsiderWrapOnObjPos = pIDSA->get(IDocumentSettingAccess::CONSIDER_WRAP_ON_OBJECT_POSITION);
        long nPrtPos = (Frm().*fnRect->fnGetTop)();
        nPrtPos = (*fnRect->fnYInc)( nPrtPos, rUpper );
        SwRect aRect( Frm() );
        long nYDiff = (*fnRect->fnYDiff)( (Prt().*fnRect->fnGetTop)(), rUpper );
        if( nYDiff > 0 )
            (aRect.*fnRect->fnAddBottom)( -nYDiff );
        for ( sal_uInt16 i = 0; i < pPage->GetSortedObjs()->Count(); ++i )
        {
            SwAnchoredObject* pAnchoredObj = (*pPage->GetSortedObjs())[i];
            if ( pAnchoredObj->ISA(SwFlyFrm) )
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
                const SwTxtFrm* pAnchorCharFrm = pFly->FindAnchorCharFrm();
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
                    const SwFmtSurround   &rSur = pFly->GetFmt()->GetSurround();
                    const SwFmtHoriOrient &rHori= pFly->GetFmt()->GetHoriOrient();
                    if ( SURROUND_NONE == rSur.GetSurround() )
                    {
                        long nBottom = (aFlyRect.*fnRect->fnGetBottom)();
                        if( (*fnRect->fnYDiff)( nPrtPos, nBottom ) < 0 )
                            nPrtPos = nBottom;
                        bInvalidatePrtArea = sal_True;
                    }
                    if ( (SURROUND_RIGHT    == rSur.GetSurround() ||
                          SURROUND_PARALLEL == rSur.GetSurround())&&
                         text::HoriOrientation::LEFT == rHori.GetHoriOrient() )
                    {
                        const long nWidth = (*fnRect->fnXDiff)(
                            (aFlyRect.*fnRect->fnGetRight)(),
                            (pFly->GetAnchorFrm()->Frm().*fnRect->fnGetLeft)() );
                        rLeftOffset = Max( rLeftOffset, nWidth );
                        bInvalidatePrtArea = sal_True;
                    }
                    if ( (SURROUND_LEFT     == rSur.GetSurround() ||
                          SURROUND_PARALLEL == rSur.GetSurround())&&
                         text::HoriOrientation::RIGHT == rHori.GetHoriOrient() )
                    {
                        const long nWidth = (*fnRect->fnXDiff)(
                            (pFly->GetAnchorFrm()->Frm().*fnRect->fnGetRight)(),
                            (aFlyRect.*fnRect->fnGetLeft)() );
                        rRightOffset = Max( rRightOffset, nWidth );
                        bInvalidatePrtArea = sal_True;
                    }
                }
            }
        }
        rUpper = (*fnRect->fnYDiff)( nPrtPos, (Frm().*fnRect->fnGetTop)() );
    }

    return bInvalidatePrtArea;
}

/*************************************************************************
|*
|*  SwTabFrm::Format()
|*
|*  Description:        "Formats" the frame; Frm and PrtArea
|*                      The fixed size is not adjusted here.
|*
|*************************************************************************/
void SwTabFrm::Format( const SwBorderAttrs *pAttrs )
{
    OSL_ENSURE( pAttrs, "TabFrm::Format, pAttrs ist 0." );

    SWRECTFN( this )
    if ( !bValidSize )
    {
        long nDiff = (GetUpper()->Prt().*fnRect->fnGetWidth)() -
                     (Frm().*fnRect->fnGetWidth)();
        if( nDiff )
            (aFrm.*fnRect->fnAddRight)( nDiff );
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
        bValidPrtArea = sal_False;
    long nRightOffset = Max( 0L, nTmpRight );

    SwTwips nLower = pAttrs->CalcBottomLine();
    // #i29550#
    if ( IsCollapsingBorders() )
        nLower += GetBottomLineSize();
    // <-- collapsing

    if ( !bValidPrtArea )
    {   bValidPrtArea = sal_True;

        //The width of the PrtArea is given by the FrmFmt, the borders have to
        //be set accordingly.
        //Minimum borders are determined depending on margins and shadows.
        //The borders are adjusted so that the PrtArea is aligned into the Frm
        //according to the adjustment.
        //If the adjustment is 0, the borders are set according to the border
        //attributes.

        const SwTwips nOldHeight = (Prt().*fnRect->fnGetHeight)();
        const SwTwips nMax = (aFrm.*fnRect->fnGetWidth)();

        // OD 14.03.2003 #i9040# - adjust variable names.
        const SwTwips nLeftLine  = pAttrs->CalcLeftLine();
        const SwTwips nRightLine = pAttrs->CalcRightLine();

        //The width possibly is a percentage value. If the table is inside
        //something else, the value applies to the surrounding. If it's the body
        //the value applies to the screen width in the BrowseView.
        const SwFmtFrmSize &rSz = GetFmt()->GetFrmSize();
        // OD 14.03.2003 #i9040# - adjust variable name.
        const SwTwips nWishedTableWidth = CalcRel( rSz, sal_True );

        sal_Bool bCheckBrowseWidth = sal_False;

        // OD 14.03.2003 #i9040# - insert new variables for left/right spacing.
        SwTwips nLeftSpacing  = 0;
        SwTwips nRightSpacing = 0;
        switch ( GetFmt()->GetHoriOrient().GetHoriOrient() )
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
                        nRightSpacing = nRightLine + Max( nRightOffset, nWishRight );
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
                                            Max( 0L, nWishRight ) );
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
                    // right offset given by surrounding fyl frames on the right:
                    const SwTwips nWishLeft = nMax - nWishedTableWidth - nRightOffset;
                    if ( nLeftOffset > 0 )
                    {
                        // surrounding fly frames on the left
                        // -> right indent is maximun of given left offset
                        //    and wished left offset.
                        nLeftSpacing = nLeftLine + Max( nLeftOffset, nWishLeft );
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
                                           Max( 0L, nWishLeft ) );
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
                                     Max( nCenterSpacing, nLeftOffset ) :
                                     nCenterSpacing );
                    nRightSpacing = nRightLine +
                                    ( (nRightOffset > 0) ?
                                      Max( nCenterSpacing, nRightOffset ) :
                                      nCenterSpacing );
                }
                break;
            case text::HoriOrientation::FULL:
                    //This things grows over the whole width.
                    //Only the free space needed for the border is taken into
                    //account. The attribute values of LRSpace are ignored
                    //intentionally.
                    bCheckBrowseWidth = sal_True;
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
                        nLeftSpacing = Max( nLeftSpacing, ( nLeftOffset + nLeftLine ) );
                    }
                    // OD 23.01.2003 #106895# - add 1st param to <SwBorderAttrs::CalcRight(..)>
                    nRightSpacing = pAttrs->CalcRight( this );
                    if( nRightOffset )
                    {
                        // OD 07.03.2003 #i9040# - surround fly frames only, if
                        // they overlap with the table.
                        // Thus, take maximun of right spacing and right offset.
                        // OD 10.03.2003 #i9040# - consider right line attribute.
                        nRightSpacing = Max( nRightSpacing, ( nRightOffset + nRightLine ) );
                    }
                }
                break;
            case text::HoriOrientation::LEFT_AND_WIDTH:
                {
                    //Linker Rand und die Breite zaehlen (Word-Spezialitaet)
                    // OD 10.03.2003 #i9040# - no width alignment in online mode.
                    //bCheckBrowseWidth = sal_True;
                    nLeftSpacing = pAttrs->CalcLeft( this );
                    if( nLeftOffset )
                    {
                        // OD 10.03.2003 #i9040# - surround fly frames only, if
                        // they overlap with the table.
                        // Thus, take maximun of right spacing and right offset.
                        // OD 10.03.2003 #i9040# - consider left line attribute.
                        nLeftSpacing = Max( nLeftSpacing, ( pAttrs->CalcLeftLine() + nLeftOffset ) );
                    }
                    // OD 10.03.2003 #i9040# - consider right and left line attribute.
                    const SwTwips nWishRight =
                            nMax - (nLeftSpacing-pAttrs->CalcLeftLine()) - nWishedTableWidth;
                    nRightSpacing = nRightLine +
                                    ( (nRightOffset > 0) ?
                                      Max( nWishRight, nRightOffset ) :
                                      nWishRight );
                }
                break;
            default:
                OSL_FAIL( "Ungueltige orientation fuer Table." );
        }

        // #i26250# - extend bottom printing area, if table
        // is last content inside a table cell.
        if ( GetFmt()->getIDocumentSettingAccess()->get(IDocumentSettingAccess::ADD_PARA_SPACING_TO_TABLE_CELLS) &&
             GetUpper()->IsInTab() && !GetIndNext() )
        {
            nLower += pAttrs->GetULSpace().GetLower();
        }
        (this->*fnRect->fnSetYMargins)( nUpper, nLower );
        if( (nMax - MINLAY) < (nLeftSpacing + nRightSpacing) )
            (this->*fnRect->fnSetXMargins)( 0, 0 );
        else
            (this->*fnRect->fnSetXMargins)( nLeftSpacing, nRightSpacing );

        ViewShell *pSh = getRootFrm()->GetCurrShell();
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
            Prt().Width( Min( nWidth, Prt().Width() ) );
        }

        if ( nOldHeight != (Prt().*fnRect->fnGetHeight)() )
            bValidSize = sal_False;
    }

    if ( !bValidSize )
    {
        bValidSize = sal_True;

        //The size is defined by the content plus the borders.
        SwTwips nRemaining = 0, nDiff;
        SwFrm *pFrm = pLower;
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
/*************************************************************************
|*
|*  SwTabFrm::GrowFrm()
|*
|*************************************************************************/
SwTwips SwTabFrm::GrowFrm( SwTwips nDist, sal_Bool bTst, sal_Bool bInfo )
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

        //Der Upper wird nur soweit wie notwendig gegrowed. In nReal wird erstmal
        //die bereits zur Verfuegung stehende Strecke bereitgestellt.

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
                nTmp = Min( nDist, nReal + nTmp );
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
            if ( GetNext()->IsCntntFrm() )
                GetNext()->InvalidatePage( pPage );
        }
        // #i28701# - Due to the new object positioning the
        // frame on the next page/column can flow backward (e.g. it was moved
        // forward due to the positioning of its objects ). Thus, invalivate this
        // next frame, if document compatibility option 'Consider wrapping style
        // influence on object positioning' is ON.
        else if ( GetFmt()->getIDocumentSettingAccess()->get(IDocumentSettingAccess::CONSIDER_WRAP_ON_OBJECT_POSITION) )
        {
            InvalidateNextPos();
        }
        _InvalidateAll();
        InvalidatePage( pPage );
        SetComplete();

        const SvxGraphicPosition ePos = GetFmt()->GetBackground().GetGraphicPos();
        if ( GPOS_NONE != ePos && GPOS_TILED != ePos )
            SetCompletePaint();
    }

    return nDist;
}
/*************************************************************************
|*
|*    SwTabFrm::Modify()
|*
|*************************************************************************/
void SwTabFrm::Modify( const SfxPoolItem* pOld, const SfxPoolItem * pNew )
{
    sal_uInt8 nInvFlags = 0;
    sal_Bool bAttrSetChg = pNew && RES_ATTRSET_CHG == pNew->Which();

    if( bAttrSetChg )
    {
        SfxItemIter aNIter( *((SwAttrSetChg*)pNew)->GetChgSet() );
        SfxItemIter aOIter( *((SwAttrSetChg*)pOld)->GetChgSet() );
        SwAttrSetChg aOldSet( *(SwAttrSetChg*)pOld );
        SwAttrSetChg aNewSet( *(SwAttrSetChg*)pNew );
        while( sal_True )
        {
            _UpdateAttr( (SfxPoolItem*)aOIter.GetCurItem(),
                         (SfxPoolItem*)aNIter.GetCurItem(), nInvFlags,
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
                if ( pTmp->IsCntntFrm() )
                    pTmp->InvalidatePage( pPage );
            }
            if ( nInvFlags & 0x10 )
                pTmp->SetCompletePaint();
        }
        if ( nInvFlags & 0x08 && 0 != (pTmp = GetPrev()) )
        {
            pTmp->_InvalidatePrt();
            if ( pTmp->IsCntntFrm() )
                pTmp->InvalidatePage( pPage );
        }
        if ( nInvFlags & 0x20  )
        {
            if ( pPage && pPage->GetUpper() && !IsFollow() )
                ((SwRootFrm*)pPage->GetUpper())->InvalidateBrowseWidth();
        }
        if ( nInvFlags & 0x80 )
            InvalidateNextPos();
    }
}

void SwTabFrm::_UpdateAttr( const SfxPoolItem *pOld, const SfxPoolItem *pNew,
                            sal_uInt8 &rInvFlags,
                            SwAttrSetChg *pOldSet, SwAttrSetChg *pNewSet )
{
    sal_Bool bClear = sal_True;
    const sal_uInt16 nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;
    switch( nWhich )
    {
        case RES_TBLHEADLINECHG:
            if ( IsFollow() )
            {
                // Delete remaining headlines:
                SwRowFrm* pLowerRow = 0;
                while ( 0 != ( pLowerRow = (SwRowFrm*)Lower() ) && pLowerRow->IsRepeatedHeadline() )
                {
                    pLowerRow->Cut();
                    delete pLowerRow;
                }

                // insert new headlines
                const sal_uInt16 nNewRepeat = GetTable()->GetRowsToRepeat();
                for ( sal_uInt16 nIdx = 0; nIdx < nNewRepeat; ++nIdx )
                {
                    bDontCreateObjects = sal_True;          //frmtool
                    SwRowFrm* pHeadline = new SwRowFrm( *GetTable()->GetTabLines()[ nIdx ], this );
                    pHeadline->SetRepeatedHeadline( true );
                    bDontCreateObjects = sal_False;
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
                if ( !GetPrev() )
                    CheckPageDescs( pPage );
                if ( pPage && GetFmt()->GetPageDesc().GetNumOffset() )
                    ((SwRootFrm*)pPage->GetUpper())->SetVirtPageNum( sal_True );
                SwDocPosUpdate aMsgHnt( pPage->Frm().Top() );
                GetFmt()->GetDoc()->UpdatePageFlds( &aMsgHnt );
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
            SetDerivedR2L( sal_False );
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
            bClear = sal_False;
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

/*************************************************************************
|*
|*    SwTabFrm::GetInfo()
|*
|*************************************************************************/
sal_Bool SwTabFrm::GetInfo( SfxPoolItem &rHnt ) const
{
    if ( RES_VIRTPAGENUM_INFO == rHnt.Which() && IsInDocBody() && !IsFollow() )
    {
        SwVirtPageNumInfo &rInfo = (SwVirtPageNumInfo&)rHnt;
        const SwPageFrm *pPage = FindPageFrm();
        if ( pPage  )
        {
            if ( pPage == rInfo.GetOrigPage() && !GetPrev() )
            {
                //Here it should be (can temporary be different, should we be
                //                    concerned about this?)
                rInfo.SetInfo( pPage, this );
                return sal_False;
            }
            if ( pPage->GetPhyPageNum() < rInfo.GetOrigPage()->GetPhyPageNum() &&
                 (!rInfo.GetPage() || pPage->GetPhyPageNum() > rInfo.GetPage()->GetPhyPageNum()))
            {
                //This could be the one.
                rInfo.SetInfo( pPage, this );
            }
        }
    }
    return sal_True;
}

/*************************************************************************
|*
|*    SwTabFrm::FindLastCntnt()
|*
|*************************************************************************/
SwCntntFrm *SwTabFrm::FindLastCntnt()
{
    SwFrm *pRet = pLower;

    while ( pRet && !pRet->IsCntntFrm() )
    {
        SwFrm *pOld = pRet;

        SwFrm *pTmp = pRet;             // To skip empty section frames
        while ( pRet->GetNext() )
        {
            pRet = pRet->GetNext();
            if( !pRet->IsSctFrm() || ((SwSectionFrm*)pRet)->GetSection() )
                pTmp = pRet;
        }
        pRet = pTmp;

        if ( pRet->GetLower() )
            pRet = pRet->GetLower();
        if ( pRet == pOld )
        {
            // Check all other columns if there is a column based section with
            // an empty last column at the end of the last line - this is done
            // by SwSectionFrm::FindLastCntnt
            if( pRet->IsColBodyFrm() )
            {
#if OSL_DEBUG_LEVEL > 0
                SwSectionFrm* pSect = pRet->FindSctFrm();
                OSL_ENSURE( pSect, "Where does this column come fron?");
                OSL_ENSURE( IsAnLower( pSect ), "Splited cell?" );
#endif
                return pRet->FindSctFrm()->FindLastCntnt();
            }

            //
            // pRet may be a cell frame without a lower (cell has been split).
            // We have to find the last content the hard way:
            //
            OSL_ENSURE( pRet->IsCellFrm(), "SwTabFrm::FindLastCntnt failed" );
            const SwFrm* pRow = pRet->GetUpper();
            while ( pRow && !pRow->GetUpper()->IsTabFrm() )
                pRow = pRow->GetUpper();
            SwCntntFrm* pCntntFrm = ((SwLayoutFrm*)pRow)->ContainsCntnt();
            pRet = 0;

            while ( pCntntFrm && ((SwLayoutFrm*)pRow)->IsAnLower( pCntntFrm ) )
            {
                pRet = pCntntFrm;
                pCntntFrm = pCntntFrm->GetNextCntntFrm();
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
            pRet = ((SwSectionFrm*)pRet)->FindLastCntnt();
    }

    return (SwCntntFrm*)pRet;
}

/*************************************************************************
|*
|*  SwTabFrm::ShouldBwdMoved()
|*
|*  Description        Return value defines if the frm needs to be relocated
|*
|*************************************************************************/
sal_Bool SwTabFrm::ShouldBwdMoved( SwLayoutFrm *pNewUpper, sal_Bool, sal_Bool &rReformat )
{
    rReformat = sal_False;
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
        sal_Bool bMoveAnyway = sal_False;
        SwTwips nSpace = 0;

        SWRECTFN( this )
        if ( !SwFlowFrm::IsMoveBwdJump() )
        {

            long nOldWidth = (GetUpper()->Prt().*fnRect->fnGetWidth)();
            SWRECTFNX( pNewUpper );
            long nNewWidth = (pNewUpper->Prt().*fnRectX->fnGetWidth)();
            if( Abs( nNewWidth - nOldWidth ) < 2 )
            {
                if( sal_False ==
                    ( bMoveAnyway = (BwdMoveNecessary( pOldPage, Frm() ) > 1) ) )
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

                    const ViewShell *pSh = getRootFrm()->GetCurrShell();
                    if( pSh && pSh->GetViewOptions()->getBrowseMode() )
                        nSpace += pNewUpper->Grow( LONG_MAX, sal_True );
                }
            }
            else if( !bLockBackMove )
                bMoveAnyway = sal_True;
        }
        else if( !bLockBackMove )
            bMoveAnyway = sal_True;

        if ( bMoveAnyway )
            return rReformat = sal_True;
        else if ( !bLockBackMove && nSpace > 0 )
        {
            // #i26945# - check, if follow flow line
            // contains frame, which are moved forward due to its object
            // positioning.
            SwRowFrm* pFirstRow = GetFirstNonHeadlineRow();
            if ( pFirstRow && pFirstRow->IsInFollowFlowRow() &&
                 SwLayouter::DoesRowContainMovedFwdFrm(
                                            *(pFirstRow->GetFmt()->GetDoc()),
                                            *(pFirstRow) ) )
            {
                return sal_False;
            }
            SwTwips nTmpHeight = CalcHeightOfFirstContentLine();

            // #118840#
            // For some mysterious reason, I changed the good old
            // 'return nHeight <= nSpace' to 'return nTmpHeight < nSpace'.
            // This obviously results in problems with table frames in
            // sections. Remember: Every twip is sacred.
            return nTmpHeight <= nSpace;
        }
    }
    return sal_False;
}

/*************************************************************************
|*
|*  SwTabFrm::Cut()
|*
|*************************************************************************/
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
        if ( pFrm->IsCntntFrm() )
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
            if ( pFrm->IsCntntFrm() )
                pFrm->InvalidatePage( pPage );
        }
        //If I am (was) the only FlowFrm in my own upper, it has to do
        //the retouch. Moreover it has to do the retouch.
        else
        {   SwRootFrm *pRoot = (SwRootFrm*)pPage->GetUpper();
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
    Remove();
    if ( pUp )
    {
        OSL_ENSURE( !pUp->IsFtnFrm(), "Table in Footnote." );
        SwSectionFrm *pSct = 0;
        // #126020# - adjust check for empty section
        // #130797# - correct fix #126020#
        if ( !pUp->Lower() && pUp->IsInSct() &&
             !(pSct = pUp->FindSctFrm())->ContainsCntnt() &&
             !pSct->ContainsAny( true ) )
        {
            if ( pUp->GetUpper() )
            {
                pSct->DelEmpty( sal_False );
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
        ((SwRootFrm*)pPage->GetUpper())->InvalidateBrowseWidth();
}

/*************************************************************************
|*
|*  SwTabFrm::Paste()
|*
|*************************************************************************/
void SwTabFrm::Paste( SwFrm* pParent, SwFrm* pSibling )
{
    OSL_ENSURE( pParent, "No parent for pasting." );
    OSL_ENSURE( pParent->IsLayoutFrm(), "Parent is CntntFrm." );
    OSL_ENSURE( pParent != this, "I'm the parent myself." );
    OSL_ENSURE( pSibling != this, "I'm my own neighbour." );
    OSL_ENSURE( !GetPrev() && !GetNext() && !GetUpper(),
            "I'm still registred somewhere." );

    //Insert in the tree.
    InsertBefore( (SwLayoutFrm*)pParent, pSibling );

    _InvalidateAll();
    SwPageFrm *pPage = FindPageFrm();
    InvalidatePage( pPage );

    if ( GetNext() )
    {
        GetNext()->_InvalidatePos();
        GetNext()->_InvalidatePrt();
        if ( GetNext()->IsCntntFrm() )
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
            if ( GetPrev()->IsCntntFrm() )
                GetPrev()->InvalidatePage( pPage );
        }
    }
    else if ( GetNext() )
        //Take the marging into account when dealing with CntntFrm's. There are
        //two situations (both always happen at once):
        //a) The Cntnt becomes the first in a chain
        //b) The new follower was the first in a chain before
        GetNext()->_InvalidatePrt();

    if ( pPage && !IsFollow() )
    {
        if ( pPage->GetUpper() )
            ((SwRootFrm*)pPage->GetUpper())->InvalidateBrowseWidth();

        if ( !GetPrev() )//At least needed for HTML with a table at the beginning.
        {
            const SwPageDesc *pDesc = GetFmt()->GetPageDesc().GetPageDesc();
            if ( (pDesc && pDesc != pPage->GetPageDesc()) ||
                 (!pDesc && pPage->GetPageDesc() != &GetFmt()->GetDoc()->GetPageDesc(0)) )
                CheckPageDescs( pPage, sal_True );
        }
    }
}

/*************************************************************************
|*
|*  SwTabFrm::Prepare()
|*
|*************************************************************************/
void SwTabFrm::Prepare( const PrepareHint eHint, const void *, sal_Bool )
{
    if( PREP_BOSS_CHGD == eHint )
        CheckDirChange();
}

/*************************************************************************
|*
|*  SwRowFrm::SwRowFrm(), ~SwRowFrm()
|*
|*************************************************************************/
SwRowFrm::SwRowFrm( const SwTableLine &rLine, SwFrm* pSib, bool bInsertContent ):
    SwLayoutFrm( rLine.GetFrmFmt(), pSib ),
    pTabLine( &rLine ),
    pFollowRow( 0 ),
    // #i29550#
    mnTopMarginForLowers( 0 ),
    mnBottomMarginForLowers( 0 ),
    mnBottomLineSize( 0 ),
    // <-- collapsing
    // --> split table rows
    bIsFollowFlowRow( false ),
    // <-- split table rows
    bIsRepeatedHeadline( false ),
    mbIsRowSpanLine( false )
{
    nType = FRMC_ROW;

    //Create the boxes and insert them.
    const SwTableBoxes &rBoxes = rLine.GetTabBoxes();
    SwFrm *pTmpPrev = 0;
    for ( sal_uInt16 i = 0; i < rBoxes.size(); ++i )
    {
        SwCellFrm *pNew = new SwCellFrm( *rBoxes[i], this, bInsertContent );
        pNew->InsertBehind( this, pTmpPrev );
        pTmpPrev = pNew;
    }
}

SwRowFrm::~SwRowFrm()
{
    SwModify* pMod = GetFmt();
    if( pMod )
    {
        pMod->Remove( this );           // remove,
        if( !pMod->GetDepends() )
            delete pMod;                // and delete
    }
}

/*************************************************************************
|*
|*  SwRowFrm::RegistFlys()
|*
|*************************************************************************/
void SwRowFrm::RegistFlys( SwPageFrm *pPage )
{
    ::RegistFlys( pPage ? pPage : FindPageFrm(), this );
}

/*************************************************************************
|*
|*    SwRowFrm::Modify()
|*
|*************************************************************************/
void SwRowFrm::Modify( const SfxPoolItem* pOld, const SfxPoolItem * pNew )
{
    sal_Bool bAttrSetChg = pNew && RES_ATTRSET_CHG == pNew->Which();
    const SfxPoolItem *pItem = 0;

    if( bAttrSetChg )
    {
        const SwAttrSet* pChgSet = ((SwAttrSetChg*)pNew)->GetChgSet();
        pChgSet->GetItemState( RES_FRM_SIZE, sal_False, &pItem);
        if ( !pItem )
            pChgSet->GetItemState( RES_ROW_SPLIT, sal_False, &pItem);
    }
    else if ( RES_FRM_SIZE == pNew->Which() || RES_ROW_SPLIT == pNew->Which() )
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



/*************************************************************************
|*
|*  SwRowFrm::MakeAll()
|*
|*************************************************************************/
void SwRowFrm::MakeAll()
{
    if ( !GetNext() )
        bValidSize = sal_False;
    SwLayoutFrm::MakeAll();
}

/*************************************************************************
|*
|*  SwRowFrm::Format()
|*
|*************************************************************************/
long CalcHeightWidthFlys( const SwFrm *pFrm )
{
    SWRECTFN( pFrm )
    long nHeight = 0;
    const SwFrm* pTmp = pFrm->IsSctFrm() ?
            ((SwSectionFrm*)pFrm)->ContainsCntnt() : pFrm;
    while( pTmp )
    {
        // #i26945# - consider follow text frames
        const SwSortedObjs* pObjs( 0L );
        bool bIsFollow( false );
        if ( pTmp->IsTxtFrm() && static_cast<const SwTxtFrm*>(pTmp)->IsFollow() )
        {
            const SwFrm* pMaster;
            // #i46450# Master does not necessarily have
            // to exist if this function is called from JoinFrm() ->
            // Cut() -> Shrink()
            const SwTxtFrm* pTmpFrm = static_cast<const SwTxtFrm*>(pTmp);
            if ( pTmpFrm->GetPrev() && pTmpFrm->GetPrev()->IsTxtFrm() &&
                 static_cast<const SwTxtFrm*>(pTmpFrm->GetPrev())->GetFollow() &&
                 static_cast<const SwTxtFrm*>(pTmpFrm->GetPrev())->GetFollow() != pTmp )
                 pMaster = 0;
            else
                 pMaster = pTmpFrm->FindMaster();

            if ( pMaster )
            {
                 pObjs = static_cast<const SwTxtFrm*>(pTmp)->FindMaster()->GetDrawObjs();
                bIsFollow = true;
            }
        }
        else
        {
            pObjs = pTmp->GetDrawObjs();
        }
        if ( pObjs )
        {
            for ( sal_uInt16 i = 0; i < pObjs->Count(); ++i )
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
                    const SwFrmFmt& rFrmFmt = pAnchoredObj->GetFrmFmt();
                    const bool bConsiderObj =
                        (rFrmFmt.GetAnchor().GetAnchorId() != FLY_AS_CHAR) &&
                            pAnchoredObj->GetObjRect().Top() != FAR_AWAY &&
                            rFrmFmt.GetFollowTextFlow().GetValue() &&
                            pAnchoredObj->GetPageFrm() == pTmp->FindPageFrm();
                    if ( bConsiderObj )
                    {
                        const SwFmtFrmSize &rSz = rFrmFmt.GetFrmSize();
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

                            nHeight = Max( nHeight, nDistOfFlyBottomToAnchorTop + nFrmDiff -
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

                            nHeight = Max( nHeight, nDistOfFlyBottomToAnchorTop2 );
                        }
                    }
                }
            }
        }
        if( !pFrm->IsSctFrm() )
            break;
        pTmp = pTmp->FindNextCnt();
        if( !((SwSectionFrm*)pFrm)->IsAnLower( pTmp ) )
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
        nTopSpace    = ((SwRowFrm*)rCell.GetUpper())->GetTopMarginForLowers();
        nBottomSpace = ((SwRowFrm*)rCell.GetUpper())->GetBottomMarginForLowers();
    }
    // <-- collapsing
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
                                           const sal_Bool _bConsiderObjs,
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
                    nFlyAdd = Max( 0L, nFlyAdd - nLowHeight );
                    nFlyAdd = Max( nFlyAdd, ::CalcHeightWidthFlys( pLow ) );
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
                                          const sal_Bool _bConsiderObjs )
{
    SWRECTFN( _pRow )

    const SwFmtFrmSize &rSz = _pRow->GetFmt()->GetFrmSize();

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
        if ( ( 0 != pLow->IsVertical() ) == ( 0 != bVert ) && nTmp > nHeight )
            nHeight = nTmp;

        pLow = static_cast<const SwCellFrm*>(pLow->GetNext());
    }
    if ( rSz.GetHeightSizeType() == ATT_MIN_SIZE && !_pRow->IsRowSpanLine() )
        nHeight = Max( nHeight, rSz.GetHeight() );
    return nHeight;
}

// #i29550#

// Calculate the maximum of (TopLineSize + TopLineDist) over all lowers:
static sal_uInt16 lcl_GetTopSpace( const SwRowFrm& rRow )
{
    sal_uInt16 nTopSpace = 0;
    for ( SwCellFrm* pCurrLower = (SwCellFrm*)rRow.Lower(); pCurrLower;
          pCurrLower = (SwCellFrm*)pCurrLower->GetNext() )
    {
        sal_uInt16 nTmpTopSpace = 0;
        if ( pCurrLower->Lower() && pCurrLower->Lower()->IsRowFrm() )
            nTmpTopSpace = lcl_GetTopSpace( *(SwRowFrm*)pCurrLower->Lower() );
        else
        {
            const SwAttrSet& rSet = ((SwCellFrm*)pCurrLower)->GetFmt()->GetAttrSet();
            const SvxBoxItem& rBoxItem = rSet.GetBox();
            nTmpTopSpace = rBoxItem.CalcLineSpace( BOX_LINE_TOP, sal_True );
        }
        nTopSpace  = Max( nTopSpace, nTmpTopSpace );
    }
    return nTopSpace;
}

// Calculate the maximum of TopLineDist over all lowers:
static sal_uInt16 lcl_GetTopLineDist( const SwRowFrm& rRow )
{
    sal_uInt16 nTopLineDist = 0;
    for ( SwCellFrm* pCurrLower = (SwCellFrm*)rRow.Lower(); pCurrLower;
          pCurrLower = (SwCellFrm*)pCurrLower->GetNext() )
    {
        sal_uInt16 nTmpTopLineDist = 0;
        if ( pCurrLower->Lower() && pCurrLower->Lower()->IsRowFrm() )
            nTmpTopLineDist = lcl_GetTopLineDist( *(SwRowFrm*)pCurrLower->Lower() );
        else
        {
            const SwAttrSet& rSet = ((SwCellFrm*)pCurrLower)->GetFmt()->GetAttrSet();
            const SvxBoxItem& rBoxItem = rSet.GetBox();
            nTmpTopLineDist = rBoxItem.GetDistance( BOX_LINE_TOP );
        }
        nTopLineDist = Max( nTopLineDist, nTmpTopLineDist );
    }
    return nTopLineDist;
}

// Calculate the maximum of BottomLineSize over all lowers:
static sal_uInt16 lcl_GetBottomLineSize( const SwRowFrm& rRow )
{
    sal_uInt16 nBottomLineSize = 0;
    for ( SwCellFrm* pCurrLower = (SwCellFrm*)rRow.Lower(); pCurrLower;
          pCurrLower = (SwCellFrm*)pCurrLower->GetNext() )
    {
        sal_uInt16 nTmpBottomLineSize = 0;
        if ( pCurrLower->Lower() && pCurrLower->Lower()->IsRowFrm() )
        {
            const SwFrm* pRow = pCurrLower->GetLastLower();
            nTmpBottomLineSize = lcl_GetBottomLineSize( *(SwRowFrm*)pRow );
        }
        else
        {
            const SwAttrSet& rSet = ((SwCellFrm*)pCurrLower)->GetFmt()->GetAttrSet();
            const SvxBoxItem& rBoxItem = rSet.GetBox();
            nTmpBottomLineSize = rBoxItem.CalcLineSpace( BOX_LINE_BOTTOM, sal_True ) -
                                 rBoxItem.GetDistance( BOX_LINE_BOTTOM );
        }
        nBottomLineSize = Max( nBottomLineSize, nTmpBottomLineSize );
    }
    return nBottomLineSize;
}

// Calculate the maximum of BottomLineDist over all lowers:
static sal_uInt16 lcl_GetBottomLineDist( const SwRowFrm& rRow )
{
    sal_uInt16 nBottomLineDist = 0;
    for ( SwCellFrm* pCurrLower = (SwCellFrm*)rRow.Lower(); pCurrLower;
          pCurrLower = (SwCellFrm*)pCurrLower->GetNext() )
    {
        sal_uInt16 nTmpBottomLineDist = 0;
        if ( pCurrLower->Lower() && pCurrLower->Lower()->IsRowFrm() )
        {
            const SwFrm* pRow = pCurrLower->GetLastLower();
            nTmpBottomLineDist = lcl_GetBottomLineDist( *(SwRowFrm*)pRow );
        }
        else
        {
            const SwAttrSet& rSet = ((SwCellFrm*)pCurrLower)->GetFmt()->GetAttrSet();
            const SvxBoxItem& rBoxItem = rSet.GetBox();
            nTmpBottomLineDist = rBoxItem.GetDistance( BOX_LINE_BOTTOM );
        }
        nBottomLineDist = Max( nBottomLineDist, nTmpBottomLineDist );
    }
    return nBottomLineDist;
}

// <-- collapsing

void SwRowFrm::Format( const SwBorderAttrs *pAttrs )
{
    SWRECTFN( this )
    OSL_ENSURE( pAttrs, "SwRowFrm::Format without Attrs." );

    const sal_Bool bFix = bFixSize;

    if ( !bValidPrtArea )
    {
        //RowFrms don't have borders and so on therefore the PrtArea always
        //matches the Frm.
        bValidPrtArea = sal_True;
        aPrt.Left( 0 );
        aPrt.Top( 0 );
        aPrt.Width ( aFrm.Width() );
        aPrt.Height( aFrm.Height() );

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
                sal_uInt16 nIdx = 0;
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
                    // pTmpRow is a first row in the table structue.
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
                SwIterator<SwRowFrm,SwFmt> aIter( *pPrevTabLine->GetFrmFmt() );
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
// <-- collapsing
    }

    while ( !bValidSize )
    {
        bValidSize = sal_True;

#if OSL_DEBUG_LEVEL > 0
        if ( HasFixSize() )
        {
            const SwFmtFrmSize &rFrmSize = GetFmt()->GetFrmSize();
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
            bFixSize = sal_False;
            if ( nDiff > 0 )
                Shrink( nDiff, sal_False, sal_True );
            else if ( nDiff < 0 )
                Grow( -nDiff );
            bFixSize = bFix;
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
            bFixSize = sal_False;
            Grow( nDiff );
            bFixSize = bFix;
            bValidSize = sal_True;
        }
    }
}

/*************************************************************************
|*
|*  SwRowFrm::AdjustCells()
|*
|*************************************************************************/
void SwRowFrm::AdjustCells( const SwTwips nHeight, const sal_Bool bHeight )
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

/*************************************************************************
|*
|*  SwRowFrm::Cut()
|*
|*************************************************************************/
void SwRowFrm::Cut()
{
    SwTabFrm *pTab = FindTabFrm();
    if ( pTab && pTab->IsFollow() && this == pTab->GetFirstNonHeadlineRow() )
    {
        pTab->FindMaster()->InvalidatePos();
    }

    // #i103961#
    // notification for accessibility
    {
        SwRootFrm *pRootFrm = getRootFrm();
        if( pRootFrm && pRootFrm->IsAnyShellAccessible() )
        {
            ViewShell* pVSh = pRootFrm->GetCurrShell();
            if ( pVSh && pVSh->Imp() )
            {
                SwFrm* pCellFrm( GetLower() );
                while ( pCellFrm )
                {
                    OSL_ENSURE( pCellFrm->IsCellFrm(),
                            "<SwRowFrm::Cut()> - unexpected type of SwRowFrm lower." );
                    pVSh->Imp()->DisposeAccessibleFrm( pCellFrm );

                    pCellFrm = pCellFrm->GetNext();
                }
            }
        }
    }

    SwLayoutFrm::Cut();
}

/*************************************************************************
|*
|*  SwRowFrm::GrowFrm()
|*
|*************************************************************************/


SwTwips SwRowFrm::GrowFrm( SwTwips nDist, sal_Bool bTst, sal_Bool bInfo )
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

        //
        // There may still be some space left in my direct upper:
        //
        const SwTwips nAdditionalSpace =
                (Frm().*fnRect->fnBottomDist)( (GetUpper()->GetUpper()->*fnRect->fnGetPrtBottom)() );
        if ( bRestrictTableGrowth && nAdditionalSpace > 0 )
        {
            nReal = Min( nAdditionalSpace, nDist );
            nDist -= nReal;
            if ( !bTst )
                (Frm().*fnRect->fnAddBottom)( nReal );
        }
    }

    if ( bRestrictTableGrowth )
        pTab->SetRestrictTableGrowth( sal_True );
    else
    {
        // Ok, this looks like a hack, indeed, it is a hack.
        // If the current row frame is inside another cell frame,
        // and the current row frame has no follow, it should not
        // be allowed to grow. In fact, setting bRestrictTableGrowth
        // to 'false' does not work, because the surrounding RowFrm
        // would set this to 'true'.
        pTab->SetFollowFlowLine( sal_False );
    }

    nReal += SwLayoutFrm::GrowFrm( nDist, bTst, bInfo);

    pTab->SetRestrictTableGrowth( sal_False );
    pTab->SetFollowFlowLine( bHasFollowFlowLine );

    //Update the height of the cells to the newest value.
    if ( !bTst )
    {
        SWRECTFNX( this )
        AdjustCells( (Prt().*fnRectX->fnGetHeight)() + nReal, sal_True );
        if ( nReal )
            SetCompletePaint();
    }

    return nReal;
}

/*************************************************************************
|*
|*  SwRowFrm::ShrinkFrm()
|*
|*************************************************************************/
SwTwips SwRowFrm::ShrinkFrm( SwTwips nDist, sal_Bool bTst, sal_Bool bInfo )
{
    SWRECTFN( this )
    if( HasFixSize() )
    {
        AdjustCells( (Prt().*fnRect->fnGetHeight)(), sal_True );
        return 0L;
    }

    //bInfo wird ggf. vom SwRowFrm::Format auf sal_True gesetzt, hier muss dann
    //entsprechend reagiert werden
    const sal_Bool bShrinkAnyway = bInfo;

    //Only shrink as much as the content of the biggest cell allows.
    SwTwips nRealDist = nDist;
    {
        const SwFmtFrmSize &rSz = GetFmt()->GetFrmSize();
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
            //Badaa: 2008-04-18 * Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
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
                //Badaa: 2008-04-18 * Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
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
            if ( !pTab->IsRebuildLastLine() && pTab->IsFollow() &&
                 this == pTab->GetFirstNonHeadlineRow() )
            {
                SwTabFrm* pMasterTab = const_cast< SwTabFrm* >( pTab->FindMaster() );
                pMasterTab->InvalidatePos();
            }
        }
        AdjustCells( (Prt().*fnRect->fnGetHeight)() - nReal, sal_True );
    }
    return nReal;
}

/*************************************************************************
|*
|*  SwRowFrm::IsRowSplitAllowed()
|*
|*************************************************************************/
bool SwRowFrm::IsRowSplitAllowed() const
{
    // Fixed size rows are never allowed to split:
    if ( HasFixSize() )
    {
        OSL_ENSURE( ATT_FIX_SIZE == GetFmt()->GetFrmSize().GetHeightSizeType(), "pRow claims to have fixed size" );
        return false;
    }

    // Repeated headlines are never allowed to split:
    const SwTabFrm* pTabFrm = FindTabFrm();
    if ( pTabFrm->GetTable()->GetRowsToRepeat() > 0 &&
         pTabFrm->IsInHeadline( *this ) )
        return false;

    const SwTableLineFmt* pFrmFmt = (SwTableLineFmt*)GetTabLine()->GetFrmFmt();
    const SwFmtRowSplit& rLP = pFrmFmt->GetRowSplit();
    return 0 != rLP.GetValue();
}

/*************************************************************************
|*
|*  SwRowFrm::ShouldRowKeepWithNext()
|*
|*************************************************************************/
bool SwRowFrm::ShouldRowKeepWithNext() const
{
    bool bRet = false;

    const SwCellFrm* pCell = static_cast<const SwCellFrm*>(Lower());
    const SwFrm* pTxt = pCell->Lower();

    if ( pTxt && pTxt->IsTxtFrm() )
    {
        bRet = static_cast<const SwTxtFrm*>(pTxt)->GetTxtNode()->GetSwAttrSet().GetKeep().GetValue();
    }
    return bRet;
}

/*************************************************************************
|*
|*  SwCellFrm::SwCellFrm(), ~SwCellFrm()
|*
|*************************************************************************/
SwCellFrm::SwCellFrm( const SwTableBox &rBox, SwFrm* pSib, bool bInsertContent ) :
    SwLayoutFrm( rBox.GetFrmFmt(), pSib ),
    pTabBox( &rBox )
{
    nType = FRMC_CELL;

    if ( !bInsertContent )
        return;

    //If a StartIdx is available, CntntFrms are added in the cell, otherwise
    //Rows have to be present and those are added.
    if ( rBox.GetSttIdx() )
    {
        sal_uLong nIndex = rBox.GetSttIdx();
        ::_InsertCnt( this, rBox.GetFrmFmt()->GetDoc(), ++nIndex );
    }
    else
    {
        const SwTableLines &rLines = rBox.GetTabLines();
        SwFrm *pTmpPrev = 0;
        for ( sal_uInt16 i = 0; i < rLines.size(); ++i )
        {
            SwRowFrm *pNew = new SwRowFrm( *rLines[i], this, bInsertContent );
            pNew->InsertBehind( this, pTmpPrev );
            pTmpPrev = pNew;
        }
    }
}

SwCellFrm::~SwCellFrm()
{
    SwModify* pMod = GetFmt();
    if( pMod )
    {
        // At this stage the lower frames aren't destroyed already,
        // therfor we have to do a recursive dispose.
        SwRootFrm *pRootFrm = getRootFrm();
        if( pRootFrm && pRootFrm->IsAnyShellAccessible() &&
            pRootFrm->GetCurrShell() )
        {
            pRootFrm->GetCurrShell()->Imp()->DisposeAccessibleFrm( this, sal_True );
        }

        pMod->Remove( this );           // remove,
        if( !pMod->GetDepends() )
            delete pMod;                // and delete
    }
}

/*************************************************************************
|*
|*  SwCellFrm::Format()
|*
|*************************************************************************/
static sal_Bool lcl_ArrangeLowers( SwLayoutFrm *pLay, long lYStart, sal_Bool bInva )
{
    sal_Bool bRet = sal_False;
    SwFrm *pFrm = pLay->Lower();
    SWRECTFN( pLay )
    while ( pFrm )
    {
        long nFrmTop = (pFrm->Frm().*fnRect->fnGetTop)();
        if( nFrmTop != lYStart )
        {
            bRet = sal_True;
            const long lDiff = (*fnRect->fnYDiff)( lYStart, nFrmTop );
            const long lDiffX = lYStart - nFrmTop;
            (pFrm->Frm().*fnRect->fnSubTop)( -lDiff );
            (pFrm->Frm().*fnRect->fnAddBottom)( lDiff );
            pFrm->SetCompletePaint();
            if ( !pFrm->GetNext() )
                pFrm->SetRetouche();
            if( bInva )
                pFrm->Prepare( PREP_POS_CHGD );
            if ( pFrm->IsLayoutFrm() && ((SwLayoutFrm*)pFrm)->Lower() )
                lcl_ArrangeLowers( (SwLayoutFrm*)pFrm,
                    (((SwLayoutFrm*)pFrm)->Lower()->Frm().*fnRect->fnGetTop)()
                    + lDiffX, bInva );
            if ( pFrm->GetDrawObjs() )
            {
                for ( sal_uInt16 i = 0; i < pFrm->GetDrawObjs()->Count(); ++i )
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
                        SwFmtVertOrient aVert( pAnchoredObj->GetFrmFmt().GetVertOrient() );
                        switch ( aVert.GetRelationOrient() )
                        {
                            case text::RelOrientation::PAGE_FRAME:
                            case text::RelOrientation::PAGE_PRINT_AREA:
                                bVertPosDepOnAnchor = false;
                                break;
                            default: break;
                        }
                    }
                    if ( pAnchoredObj->ISA(SwFlyFrm) )
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
                        // #115759#
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
                    else if ( pAnchoredObj->ISA(SwAnchoredDrawObject) )
                    {
                        // #i26945#
                        const SwTabFrm* pTabFrm = pLay->FindTabFrm();
                        if ( pTabFrm &&
                             !( pTabFrm->IsFollow() &&
                                pTabFrm->FindMaster()->IsRebuildLastLine() ) &&
                             !pAnchoredObj->GetFrmFmt().GetAnchor().GetAnchorId()
                                                            == FLY_AS_CHAR )
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
                                static_cast<const SwDrawFrmFmt&>(pAnchoredObj->GetFrmFmt()).IsPosAttrSet() &&
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
                 ( !pFrm->IsTxtFrm() ||
                   !static_cast<SwTxtFrm*>(pFrm)->IsUndersized() ) ) ||
               pFrm->IsInSplitTableRow() ) )
        {
            pFrm->InvalidatePos();
        }

        pFrm = pFrm->GetNext();
    }
    return bRet;
}

void SwCellFrm::Format( const SwBorderAttrs *pAttrs )
{
    OSL_ENSURE( pAttrs, "CellFrm::Format, pAttrs ist 0." );
    const SwTabFrm* pTab = FindTabFrm();
    SWRECTFN( pTab )

    if ( !bValidPrtArea )
    {
        bValidPrtArea = sal_True;

        //Adjust position.
        if ( Lower() )
        {
            SwTwips nTopSpace, nBottomSpace, nLeftSpace, nRightSpace;
            // #i29550#
            if ( pTab->IsCollapsingBorders() && !Lower()->IsRowFrm()  )
            {
                const SvxBoxItem& rBoxItem = pAttrs->GetBox();
                nLeftSpace   = rBoxItem.GetDistance( BOX_LINE_LEFT );
                nRightSpace  = rBoxItem.GetDistance( BOX_LINE_RIGHT );
                nTopSpace    =  ((SwRowFrm*)GetUpper())->GetTopMarginForLowers();
                nBottomSpace =  ((SwRowFrm*)GetUpper())->GetBottomMarginForLowers();
            }
            else
            {
            // <-- collapsing
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
    if ( !bValidSize )
    {
        bValidSize = sal_True;

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
            const SwTwips nWish = pTab->GetFmt()->GetFrmSize().GetWidth();
            nWidth = pAttrs->GetSize().Width();

            OSL_ENSURE( nWish, "Tabele without width?" );
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
                    sal_uInt16 i = 0;
                    do
                    {
                        pTmpBox = rBoxes[ i++ ];
                        nSumWidth += pTmpBox->GetFrmFmt()->GetFrmSize().GetWidth();
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
                    bValidSize = bValidPrtArea = sal_True;
            }
            else
            {
                //Only keep invalidated if shrinking was done; this can be
                //dismissed because all adjoined cells have to be the same size.
                if ( !Shrink( -nDiffHeight ) )
                    bValidSize = bValidPrtArea = sal_True;
            }
        }
    }
    const SwFmtVertOrient &rOri = pAttrs->GetAttrSet().GetVertOrient();

    if ( !Lower() )
        return;

    // From now on, all operations are related to the table cell.
    SWREFRESHFN( this )

    SwPageFrm* pPg = 0;
    if ( !FindTabFrm()->IsRebuildLastLine() && text::VertOrientation::NONE != rOri.GetVertOrient() &&
    // #158225# no vertical alignment of covered cells
         !IsCoveredCell() &&
    // #116532# Do not consider vertical alignment in grid mode
         !(pPg = FindPageFrm())->HasGrid() )
    {
        if ( !Lower()->IsCntntFrm() && !Lower()->IsSctFrm() && !Lower()->IsTabFrm() )
        {
            // OSL_ENSURE(for HTML-import!
            OSL_ENSURE( !this, "VAlign to cell without content" );
            return;
        }
        sal_Bool bVertDir = sal_True;
        // #i43913# - no vertical alignment, if wrapping
        // style influence is considered on object positioning and
        // an object is anchored inside the cell.
        const bool bConsiderWrapOnObjPos( GetFmt()->getIDocumentSettingAccess()->get(IDocumentSettingAccess::CONSIDER_WRAP_ON_OBJECT_POSITION) );
        //No alignment if border with flow overlaps the cell.
        if ( pPg->GetSortedObjs() )
        {
            SwRect aRect( Prt() ); aRect += Frm().Pos();
            for ( sal_uInt16 i = 0; i < pPg->GetSortedObjs()->Count(); ++i )
            {
                const SwAnchoredObject* pAnchoredObj = (*pPg->GetSortedObjs())[i];
                SwRect aTmp( pAnchoredObj->GetObjRect() );
                if ( aTmp.IsOver( aRect ) )
                {
                    const SwFrmFmt& rAnchoredObjFrmFmt = pAnchoredObj->GetFrmFmt();
                    const SwFmtSurround &rSur = rAnchoredObjFrmFmt.GetSurround();

                    if ( SURROUND_THROUGHT != rSur.GetSurround() )
                    {
                        // frames, which the cell is a lower of, aren't relevant
                        if ( pAnchoredObj->ISA(SwFlyFrm) )
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
                             !rAnchoredObjFrmFmt.GetFollowTextFlow().GetValue() )
                        {
                            bVertDir = sal_False;
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
        if ( Lower()->IsCntntFrm() )
        {
            const long lYStart = (this->*fnRect->fnGetPrtTop)();
            lcl_ArrangeLowers( this, lYStart, sal_True );
        }
    }
}

/*************************************************************************
|*
|*    SwCellFrm::Modify()
|*
|*************************************************************************/

void SwCellFrm::Modify( const SfxPoolItem* pOld, const SfxPoolItem * pNew )
{
    sal_Bool bAttrSetChg = pNew && RES_ATTRSET_CHG == pNew->Which();
    const SfxPoolItem *pItem = 0;

    if( bAttrSetChg )
        ((SwAttrSetChg*)pNew)->GetChgSet()->GetItemState( RES_VERT_ORIENT, sal_False, &pItem);
    else if ( RES_VERT_ORIENT == pNew->Which() )
        pItem = pNew;

    if ( pItem )
    {
        sal_Bool bInva = sal_True;
        if ( text::VertOrientation::NONE == ((SwFmtVertOrient*)pItem)->GetVertOrient() &&
             // OD 04.11.2003 #112910#
             Lower() && Lower()->IsCntntFrm() )
        {
            SWRECTFN( this )
            const long lYStart = (this->*fnRect->fnGetPrtTop)();
            bInva = lcl_ArrangeLowers( this, lYStart, sal_False );
        }
        if ( bInva )
        {
            SetCompletePaint();
            InvalidatePrt();
        }
    }

    if ( ( bAttrSetChg &&
           SFX_ITEM_SET == ((SwAttrSetChg*)pNew)->GetChgSet()->GetItemState( RES_PROTECT, sal_False ) ) ||
         RES_PROTECT == pNew->Which() )
    {
        ViewShell *pSh = getRootFrm()->GetCurrShell();
        if( pSh && pSh->GetLayout()->IsAnyShellAccessible() )
            pSh->Imp()->InvalidateAccessibleEditableState( sal_True, this );
    }

    if ( bAttrSetChg &&
         SFX_ITEM_SET == ((SwAttrSetChg*)pNew)->GetChgSet()->GetItemState( RES_FRAMEDIR, sal_False, &pItem ) )
    {
        SetDerivedVert( sal_False );
        CheckDirChange();
    }

    // #i29550#
    if ( bAttrSetChg &&
         SFX_ITEM_SET == ((SwAttrSetChg*)pNew)->GetChgSet()->GetItemState( RES_BOX, sal_False, &pItem ) )
    {
        SwFrm* pTmpUpper = GetUpper();
        while ( pTmpUpper->GetUpper() && !pTmpUpper->GetUpper()->IsTabFrm() )
            pTmpUpper = pTmpUpper->GetUpper();

        SwTabFrm* pTabFrm = (SwTabFrm*)pTmpUpper->GetUpper();
        if ( pTabFrm->IsCollapsingBorders() )
        {
            // Invalidate lowers of this and next row:
            lcl_InvalidateAllLowersPrt( (SwRowFrm*)pTmpUpper );
            pTmpUpper = pTmpUpper->GetNext();
            if ( pTmpUpper )
                lcl_InvalidateAllLowersPrt( (SwRowFrm*)pTmpUpper );
            else
                pTabFrm->InvalidatePrt();
        }
    }
    // <-- collapsing

    SwLayoutFrm::Modify( pOld, pNew );
}

/*************************************************************************
|*  SwCellFrm::GetLayoutRowSpan() const
|*************************************************************************/

long SwCellFrm::GetLayoutRowSpan() const
{
    long nRet = GetTabBox()->getRowSpan();
    if ( nRet < 1 )
    {
        const SwFrm* pRow = GetUpper();
        const SwTabFrm* pTab = static_cast<const SwTabFrm*>(pRow->GetUpper());

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
            ViewShell* pVSh = pRootFrm->GetCurrShell();
            if ( pVSh && pVSh->Imp() )
            {
                pVSh->Imp()->DisposeAccessibleFrm( this );
            }
        }
    }

    SwLayoutFrm::Cut();
}

//
// Helper functions for repeated headlines:
//

/*
 * SwTabFrm::IsInHeadline( const SwFrm& rFrm )
 */
bool SwTabFrm::IsInHeadline( const SwFrm& rFrm ) const
{
    OSL_ENSURE( IsAnLower( &rFrm ) && rFrm.IsInTab(),
             "SwTabFrm::IsInHeadline called for frame not lower of table" );

    const SwFrm* pTmp = &rFrm;
    while ( !pTmp->GetUpper()->IsTabFrm() )
        pTmp = pTmp->GetUpper();

    return GetTable()->IsHeadline( *((SwRowFrm*)pTmp)->GetTabLine() );
}

/*
 * SwTabFrm::GetFirstNonHeadlineRow()
 *
 * If this is a master table, we can may assume, that there are at least
 * nRepeat lines in the table.
 * If this is a follow table, there are intermediate states for the table
 * layout, e.g., during deletion of rows, which makes it necessary to find
 * the first non-headline row by evaluating the headline flag at the row frame.
 */
SwRowFrm* SwTabFrm::GetFirstNonHeadlineRow() const
{
    SwRowFrm* pRet = (SwRowFrm*)Lower();
    if ( pRet )
    {
        if ( IsFollow() )
        {
            while ( pRet && pRet->IsRepeatedHeadline() )
                pRet = (SwRowFrm*)pRet->GetNext();
        }
        else
        {
            sal_uInt16 nRepeat = GetTable()->GetRowsToRepeat();
            while ( pRet && nRepeat > 0 )
            {
                pRet = (SwRowFrm*)pRet->GetNext();
                --nRepeat;
            }
        }
    }

    return (SwRowFrm*)pRet;
}

/*
 * SwTable::IsHeadline()
 */
bool SwTable::IsHeadline( const SwTableLine& rLine ) const
{
    for ( sal_uInt16 i = 0; i < GetRowsToRepeat(); ++i )
        if ( GetTabLines()[ i ] == &rLine )
            return true;

    return false;
}

bool SwTabFrm::IsLayoutSplitAllowed() const
{
    return GetFmt()->GetLayoutSplit().GetValue();
}

// #i29550#

sal_uInt16 SwTabFrm::GetBottomLineSize() const
{
    OSL_ENSURE( IsCollapsingBorders(),
            "BottomLineSize only required for collapsing borders" );

    OSL_ENSURE( Lower(), "Warning! Trying to prevent a crash, please inform FME" );

    const SwFrm* pTmp = GetLastLower();

    // #124755# Try to make code robust
    if ( !pTmp ) return 0;

    return static_cast<const SwRowFrm*>(pTmp)->GetBottomLineSize();
}

bool SwTabFrm::IsCollapsingBorders() const
{
    return ((SfxBoolItem&)GetFmt()->GetAttrSet().Get( RES_COLLAPSING_BORDERS )).GetValue();
}

// <-- collapsing


//
// Local helper function to calculate height of first text row
//
static SwTwips lcl_CalcHeightOfFirstContentLine( const SwRowFrm& rSourceLine )
{
    // Find corresponding split line in master table
    const SwTabFrm* pTab = rSourceLine.FindTabFrm();
    SWRECTFN( pTab )
    const SwCellFrm* pCurrSourceCell = (SwCellFrm*)rSourceLine.Lower();

    //
    // 1. Case: rSourceLine is a follow flow line.
    // In this case we have to return the minimum of the heights
    // of the first lines in rSourceLine.
    //
    // 2. Case: rSourceLine is not a follow flow line.
    // In this case we have to return the maximum of the heights
    // of the first lines in rSourceLine.
    //
    bool bIsInFollowFlowLine = rSourceLine.IsInFollowFlowRow();
    SwTwips nHeight = bIsInFollowFlowLine ? LONG_MAX : 0;

    while ( pCurrSourceCell )
    {
        // NEW TABLES
        // Skip cells which are not responsible for the height of
        // the follow flow line:
        if ( bIsInFollowFlowLine && pCurrSourceCell->GetLayoutRowSpan() > 1 )
        {
            pCurrSourceCell = (SwCellFrm*)pCurrSourceCell->GetNext();
            continue;
        }

        const SwFrm *pTmp = pCurrSourceCell->Lower();
        if ( pTmp )
        {
            SwTwips nTmpHeight = USHRT_MAX;
            // #i32456# Consider lower row frames
            if ( pTmp->IsRowFrm() )
            {
                const SwRowFrm* pTmpSourceRow = (SwRowFrm*)pCurrSourceCell->Lower();
                nTmpHeight = lcl_CalcHeightOfFirstContentLine( *pTmpSourceRow );
            }
            if ( pTmp->IsTabFrm() )
            {
                nTmpHeight = ((SwTabFrm*)pTmp)->CalcHeightOfFirstContentLine();
            }
            else if ( pTmp->IsTxtFrm() )
            {
                SwTxtFrm* pTxtFrm = (SwTxtFrm*)pTmp;
                pTxtFrm->GetFormatted();
                nTmpHeight = pTxtFrm->FirstLineHeight();
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
                         ( !pLast->IsTxtFrm() ||
                           !static_cast<const SwTxtFrm*>(pLast)->GetFollow() ) )
                    {
                        nReal += SwFlowFrm::CastFlowFrm(pLast)->CalcAddLowerSpaceAsLastInTableCell();
                    }
                    // Don't forget the upper space and lower space,
                    // #115759# - do *not* consider the upper
                    // and the lower space for follow text frames.
                    if ( pTmp->IsFlowFrm() &&
                         ( !pTmp->IsTxtFrm() ||
                           !static_cast<const SwTxtFrm*>(pTmp)->IsFollow() ) )
                    {
                        nTmpHeight += SwFlowFrm::CastFlowFrm(pTmp)->CalcUpperSpace( NULL, pLast);
                        nTmpHeight += SwFlowFrm::CastFlowFrm(pTmp)->CalcLowerSpace();
                    }
                    // #115759# - consider additional lower
                    // space of <pTmp>, if contains only one line.
                    // In this case it would be the new last text frame, which
                    // would have no follow and thus would add this space.
                    if ( pTmp->IsTxtFrm() &&
                         const_cast<SwTxtFrm*>(static_cast<const SwTxtFrm*>(pTmp))
                                            ->GetLineCount( STRING_LEN ) == 1 )
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

        pCurrSourceCell = (SwCellFrm*)pCurrSourceCell->GetNext();
    }

    return ( LONG_MAX == nHeight ) ? 0 : nHeight;
}

//
// Function to calculate height of first text row
//
SwTwips SwTabFrm::CalcHeightOfFirstContentLine() const
{
    SWRECTFN( this )

    const bool bDontSplit = !IsFollow() && !GetFmt()->GetLayoutSplit().GetValue();

    if ( bDontSplit )
    {
        // Table is not allowed to split: Take the whole height, that's all
        return (Frm().*fnRect->fnGetHeight)();
    }

    SwRowFrm* pFirstRow = 0;
    SwTwips nTmpHeight = 0;

    pFirstRow = GetFirstNonHeadlineRow();
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
    if ( GetFmt()->GetDoc()->get(IDocumentSettingAccess::TABLE_ROW_KEEP) )
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

    // For master tables, the height of the headlines + the heigth of the
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
            // we olny calculate the height of the first line.
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
                        const SwTwips nCellHeight = lcl_CalcMinCellHeight( pLower2, sal_True );
                        nMaxHeight = Max( nCellHeight, nMaxHeight );
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

        // #118411#
        // Optimization: lcl_CalcHeightOfFirstContentLine actually can trigger
        // a formatting of the row frame (via the GetFormatted()). We don't
        // want this formatting if the row does not have a height.
        else if ( 0 != nFirstLineHeight )
        {
            const bool bOldJoinLock = IsJoinLocked();
            ((SwTabFrm*)this)->LockJoin();
            const SwTwips nHeightOfFirstContentLine = lcl_CalcHeightOfFirstContentLine( *(SwRowFrm*)pFirstRow );

            // Consider minimum row height:
            const SwFmtFrmSize &rSz = static_cast<const SwRowFrm*>(pFirstRow)->GetFmt()->GetFrmSize();
            const SwTwips nMinRowHeight = rSz.GetHeightSizeType() == ATT_MIN_SIZE ?
                                          rSz.GetHeight() : 0;

            nTmpHeight += Max( nHeightOfFirstContentLine, nMinRowHeight );

            if ( !bOldJoinLock )
                ((SwTabFrm*)this)->UnlockJoin();
        }
    }

    return nTmpHeight;
}

//
// Some more functions for covered/covering cells. This way inclusion of
// SwCellFrm can be avoided
//

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
