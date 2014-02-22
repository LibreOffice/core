/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

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
#include <editeng/brushitem.hxx>
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
#include "txtfrm.hxx"
#include "htmltbl.hxx"
#include "sectfrm.hxx"
#include <fmtfollowtextflow.hxx>
#include <sortedobjs.hxx>
#include <objectformatter.hxx>
#include <layouter.hxx>
#include <switerator.hxx>

extern void AppendObjs( const SwFrmFmts *pTbl, sal_uLong nIndex,
                        SwFrm *pFrm, SwPageFrm *pPage );

using namespace ::com::sun::star;

SwTabFrm::SwTabFrm( SwTable &rTab, SwFrm* pSib ):
    SwLayoutFrm( rTab.GetFrmFmt(), pSib ),
    SwFlowFrm( (SwFrm&)*this ),
    pTable( &rTab )
{
    bComplete = bCalcLowers = bONECalcLowers = bLowersFormatted = bLockBackMove =
    bResizeHTMLTable = bHasFollowFlowLine = bIsRebuildLastLine =
    bRestrictTableGrowth = bRemoveFollowFlowLinePending = sal_False;
    
    bConsiderObjsForMinCellHeight = sal_True;
    bObjsDoesFit = sal_True;
    mbInRecalcLowerRow = false;
    mbFixSize = sal_False;     
    mnType = FRMC_TAB;

    
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
    
    bConsiderObjsForMinCellHeight = sal_True;
    bObjsDoesFit = sal_True;
    mbInRecalcLowerRow = false;
    mbFixSize = sal_False;     
    mnType = FRMC_TAB;

    SetFollow( rTab.GetFollow() );
    rTab.SetFollow( this );
}

extern const SwTable   *pColumnCacheLastTable;
extern const SwTabFrm  *pColumnCacheLastTabFrm;
extern const SwFrm     *pColumnCacheLastCellFrm;
extern const SwTable   *pRowCacheLastTable;
extern const SwTabFrm  *pRowCacheLastTabFrm;
extern const SwFrm     *pRowCacheLastCellFrm;


SwTabFrm* SwTabFrm::GetFollowFlowLineFor()
{
    SwFlowFrm *pPrec = GetPrecede();
    if (pPrec && pPrec->GetFrm()->IsTabFrm())
    {
        SwTabFrm *pPrevTabFrm = (SwTabFrm*)pPrec;
        assert(this == pPrevTabFrm->GetFollow());
        if (pPrevTabFrm->HasFollowFlowLine() && pPrevTabFrm->GetFollow() == this)
            return pPrevTabFrm;
    }
    return NULL;
}

SwTabFrm::~SwTabFrm()
{
    
    
    SwTabFrm* pFlowFrameFor = GetFollowFlowLineFor();
    if (pFlowFrameFor)
        pFlowFrameFor->RemoveFollowFlowLine();

    
    
    
    
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

void SwTabFrm::JoinAndDelFollows()
{
    SwTabFrm *pFoll = GetFollow();
    if ( pFoll->HasFollow() )
        pFoll->JoinAndDelFollows();
    pFoll->Cut();
    SetFollow( pFoll->GetFollow() );
    delete pFoll;
}

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
static bool lcl_ArrangeLowers( SwLayoutFrm *pLay, long lYStart, bool bInva );


static bool lcl_InnerCalcLayout( SwFrm *pFrm,
                                      long nBottom,
                                      bool _bOnlyRowsAndCells = false );




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




static void lcl_InvalidateLowerObjs( SwLayoutFrm& _rLayoutFrm,
                              const bool _bMoveObjsOutOfRange = false,
                              SwPageFrm* _pPageFrm = 0L )
{
    
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

                
                pAnchoredObj->SetTmpConsiderWrapInfluence( false );
                pAnchoredObj->SetConsiderForTextWrap( false );
                pAnchoredObj->UnlockPosition();
                pAnchoredObj->InvalidateObjPos();

                
                if ( _bMoveObjsOutOfRange )
                {
                    
                    
                    
                    SwObjPositioningInProgress aObjPosInProgress( *pAnchoredObj );
                    pAnchoredObj->SetObjLeft( _pPageFrm->Frm().Right() );
                    
                    
                    
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


static void lcl_ShrinkCellsAndAllContent( SwRowFrm& rRow )
{
    SwCellFrm* pCurrMasterCell = static_cast<SwCellFrm*>(rRow.Lower());
    SWRECTFN( pCurrMasterCell )

    while ( pCurrMasterCell )
    {
        
        SwCellFrm& rToAdjust = pCurrMasterCell->GetTabBox()->getRowSpan() < 1 ?
                               const_cast<SwCellFrm&>(pCurrMasterCell->FindStartEndOfRowSpanCell( true, true )) :
                               *pCurrMasterCell;

        
        
        lcl_ArrangeLowers( &rToAdjust,
                           (rToAdjust.*fnRect->fnGetPrtTop)(),
                           false );
        
        
        
        SwFrm* pTmp = rToAdjust.GetLastLower();

        if ( pTmp && pTmp->IsRowFrm() )
        {
            SwRowFrm* pTmpRow = (SwRowFrm*)pTmp;
            lcl_ShrinkCellsAndAllContent( *pTmpRow );
        }
        else
        {
            
            while ( pTmp )
            {
                
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

            
            lcl_ArrangeLowers( &rToAdjust,
                               (rToAdjust.*fnRect->fnGetPrtTop)(),
                               false );
        }

        pCurrMasterCell = static_cast<SwCellFrm*>(pCurrMasterCell->GetNext());
    }
}




static void lcl_MoveRowContent( SwRowFrm& rSourceLine, SwRowFrm& rDestLine )
{
    SwCellFrm* pCurrDestCell = (SwCellFrm*)rDestLine.Lower();
    SwCellFrm* pCurrSourceCell = (SwCellFrm*)rSourceLine.Lower();

    
    while ( pCurrSourceCell )
    {
        if ( pCurrSourceCell->Lower() && pCurrSourceCell->Lower()->IsRowFrm() )
        {
            SwRowFrm* pTmpSourceRow = (SwRowFrm*)pCurrSourceCell->Lower();
            while ( pTmpSourceRow )
            {
                
                
                
                
                SwRowFrm* pTmpDestRow = (SwRowFrm*)pCurrDestCell->Lower();

                if ( pTmpSourceRow->IsFollowFlowRow() && pTmpDestRow )
                {
                    
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
                
                SwCellFrm* pDestCell = static_cast<SwCellFrm*>(pCurrDestCell);
                if ( pDestCell->GetTabBox()->getRowSpan() < 1 )
                    pDestCell = & const_cast<SwCellFrm&>(pDestCell->FindStartEndOfRowSpanCell( true, true ));

                
                SwFrm* pFrm = pDestCell->GetLastLower();
                ::RestoreCntnt( pTmp, pDestCell, pFrm, true );
            }
        }
        pCurrDestCell = (SwCellFrm*)pCurrDestCell->GetNext();
        pCurrSourceCell = (SwCellFrm*)pCurrSourceCell->GetNext();
    }
}



static void lcl_MoveFootnotes( SwTabFrm& rSource, SwTabFrm& rDest, SwLayoutFrm& rRowFrm )
{
    if ( !rSource.GetFmt()->GetDoc()->GetFtnIdxs().empty() )
    {
        SwFtnBossFrm* pOldBoss = rSource.FindFtnBossFrm( sal_True );
        SwFtnBossFrm* pNewBoss = rDest.FindFtnBossFrm( sal_True );
        rRowFrm.MoveLowerFtns( 0, pOldBoss, pNewBoss, sal_True );
    }
}


static void lcl_PreprocessRowsInCells( SwTabFrm& rTab, SwRowFrm& rLastLine,
                                SwRowFrm& rFollowFlowLine, SwTwips nRemain )
{
    SwCellFrm* pCurrLastLineCell = (SwCellFrm*)rLastLine.Lower();
    SwCellFrm* pCurrFollowFlowLineCell = (SwCellFrm*)rFollowFlowLine.Lower();

    SWRECTFN( pCurrLastLineCell )

    
    while ( pCurrLastLineCell )
    {
        if ( pCurrLastLineCell->Lower() && pCurrLastLineCell->Lower()->IsRowFrm() )
        {
            SwTwips nTmpCut = nRemain;
            SwRowFrm* pTmpLastLineRow = (SwRowFrm*)pCurrLastLineCell->Lower();

            
            SwTwips nCurrentHeight =
                    lcl_CalcMinRowHeight( pTmpLastLineRow,
                                          rTab.IsConsiderObjsForMinCellHeight() );
            while ( pTmpLastLineRow && pTmpLastLineRow->GetNext() && nTmpCut > nCurrentHeight )
            {
                nTmpCut -= nCurrentHeight;
                pTmpLastLineRow = (SwRowFrm*)pTmpLastLineRow->GetNext();
                
                nCurrentHeight =
                    lcl_CalcMinRowHeight( pTmpLastLineRow,
                                          rTab.IsConsiderObjsForMinCellHeight() );
            }

            
            if ( pTmpLastLineRow )
            {
                
                
                SwFrm* pCell = pTmpLastLineRow->Lower();
                bool bTableLayoutToComplex = false;
                long nMinHeight = 0;

                
                
                
                
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
                        nMinHeight = std::max( nMinHeight, lcl_CalcTopAndBottomMargin( *(SwLayoutFrm*)pCell, rAttrs ) );
                        pCell = pCell->GetNext();
                    }

                    const SwFmtFrmSize &rSz = pTmpLastLineRow->GetFmt()->GetFrmSize();
                    if ( rSz.GetHeightSizeType() == ATT_MIN_SIZE )
                        nMinHeight = std::max( nMinHeight, rSz.GetHeight() );
                }

                
                
                
                
                //
                
                
                
                if ( nTmpCut > nCurrentHeight ||
                     ( pTmpLastLineRow->IsRowSplitAllowed() &&
                      !bTableLayoutToComplex && nMinHeight < nTmpCut ) )
                {
                    
                    SwRowFrm* pNewRow = new SwRowFrm( *pTmpLastLineRow->GetTabLine(), &rTab, false );
                    pNewRow->SetFollowFlowRow( true );
                    pNewRow->SetFollowRow( pTmpLastLineRow->GetFollowRow() );
                    pTmpLastLineRow->SetFollowRow( pNewRow );
                    pNewRow->InsertBehind( pCurrFollowFlowLineCell, 0 );
                    pTmpLastLineRow = (SwRowFrm*)pTmpLastLineRow->GetNext();
                }

                
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


inline void TableSplitRecalcLock( SwFlowFrm *pTab ) { pTab->LockJoin(); }
inline void TableSplitRecalcUnlock( SwFlowFrm *pTab ) { pTab->UnlockJoin(); }

static bool lcl_RecalcSplitLine( SwRowFrm& rLastLine, SwRowFrm& rFollowLine,
                          SwTwips nRemainingSpaceForLastRow )
{
    bool bRet = true;

    SwTabFrm& rTab = (SwTabFrm&)*rLastLine.GetUpper();

    
    
    lcl_PreprocessRowsInCells( rTab, rLastLine, rFollowLine, nRemainingSpaceForLastRow );

    
    rTab.SetRebuildLastLine( sal_True );
    
    rTab.SetDoesObjsFit( sal_True );
    SWRECTFN( rTab.GetUpper() )

    
    
    ::lcl_InvalidateLowerObjs( rLastLine, true );

    

    
    
    rTab.SetConsiderObjsForMinCellHeight( sal_False );
    ::lcl_ShrinkCellsAndAllContent( rLastLine );
    rTab.SetConsiderObjsForMinCellHeight( sal_True );

    
    ::SwInvalidateAll( &rLastLine, LONG_MAX );

    
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

    
    lcl_RecalcRow( rLastLine, LONG_MAX );
    
    
    rLastLine.InvalidateSize();
    rLastLine.Calc();

    
    if ( bUnlockFollow )
        ::TableSplitRecalcUnlock( rTab.GetFollow() );
    if ( bUnlockMaster )
        ::TableSplitRecalcUnlock( pMaster );

    
    
    lcl_PostprocessRowsInCells( rTab, rLastLine );

    
    //
    
    
    

    
    
    const SwTwips nDistanceToUpperPrtBottom =
            (rTab.Frm().*fnRect->fnBottomDist)( (rTab.GetUpper()->*fnRect->fnGetPrtBottom)());
    if ( nDistanceToUpperPrtBottom < 0 || !rTab.DoesObjsFit() )
        bRet = false;

    
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

    
    if ( bRet )
    {
        if ( !rLastLine.ContainsCntnt() )
        {
            bRet = false;
        }
    }

    
    if ( bRet )
    {
        if ( !rFollowLine.IsRowSpanLine() && !rFollowLine.ContainsCntnt() )
        {
            bRet = false;
        }
    }

    if ( bRet )
    {
        
        
        ::SwInvalidateAll( &rFollowLine, LONG_MAX );
    }
    else
    {
        
        
        
        ::SwInvalidateAll( &rLastLine, LONG_MAX );
    }

    rTab.SetRebuildLastLine( sal_False );
    
    rTab.SetDoesObjsFit( sal_True );

    return bRet;
}


static void lcl_AdjustRowSpanCells( SwRowFrm* pRow )
{
    SWRECTFN( pRow )
    SwCellFrm* pCellFrm = static_cast<SwCellFrm*>(pRow->GetLower());
    while ( pCellFrm )
    {
        const long nLayoutRowSpan = pCellFrm->GetLayoutRowSpan();
        if ( nLayoutRowSpan > 1 )
        {
            
            const long nNewCellHeight = lcl_GetHeightOfRows( pRow, nLayoutRowSpan );
            const long nDiff = nNewCellHeight - (pCellFrm->Frm().*fnRect->fnGetHeight)();
            if ( nDiff )
                (pCellFrm->Frm().*fnRect->fnAddBottom)( nDiff );
        }

        pCellFrm = static_cast<SwCellFrm*>(pCellFrm->GetNext());
    }
}



static long lcl_GetMaximumLayoutRowSpan( const SwRowFrm& rRow )
{
    long nRet = 1;

    const SwRowFrm* pCurrentRowFrm = static_cast<const SwRowFrm*>(rRow.GetNext());
    bool bNextRow = false;

    while ( pCurrentRowFrm )
    {
        
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




bool SwTabFrm::RemoveFollowFlowLine()
{
    
    SwRowFrm* pFollowFlowLine = static_cast<SwRowFrm*>(GetFollow()->GetFirstNonHeadlineRow());

    
    SwFrm* pLastLine = GetLastLower();

    OSL_ENSURE( HasFollowFlowLine() &&
            pFollowFlowLine &&
            pLastLine, "There should be a flowline in the follow" );

    
    
    
    SetFollowFlowLine( sal_False );

    
    if ( !pFollowFlowLine || !pLastLine )
        return true;

    
    lcl_MoveRowContent( *pFollowFlowLine, *(SwRowFrm*)pLastLine );

    
    
    
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

bool SwTabFrm::Split( const SwTwips nCutPos, bool bTryToSplit, bool bTableRowKeep )
{
    bool bRet = true;

    SWRECTFN( this )

    
    {
        this->Lower()->_InvalidatePos();
        
        
        lcl_InnerCalcLayout( this->Lower(), LONG_MAX, true );
    }

    
    
    
    SwRowFrm *pRow = static_cast<SwRowFrm*>(Lower());
    if( !pRow )
        return bRet;

    const sal_uInt16 nRepeat = GetTable()->GetRowsToRepeat();
    sal_uInt16 nRowCount = 0;           

    SwTwips nRemainingSpaceForLastRow =
        (*fnRect->fnYDiff)( nCutPos, (Frm().*fnRect->fnGetTop)() );
    nRemainingSpaceForLastRow -= (this->*fnRect->fnGetTopMargin)();

    
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

    
    
    
    
    
    
    
    bool bSplitRowAllowed = pRow->IsRowSplitAllowed();

    
    
    
    
    
    if ( lcl_FindSectionsInRow( *pRow ) )
    {
        bTryToSplit = false;
    }

    
    
    
    
    bool bKeepNextRow = false;
    if ( nRowCount < nRepeat )
    {
        
        
        
        OSL_ENSURE( !GetIndPrev(), "Table is supposed to be at beginning" );
        bKeepNextRow = true;
    }
    else if ( !GetIndPrev() && nRepeat == nRowCount )
    {
        
        
        
        if ( bTryToSplit && bSplitRowAllowed )
        {
            
            
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

    
    if ( !pRow )
        return bRet;

    
    
    
    
    bSplitRowAllowed = bSplitRowAllowed && bTryToSplit &&
                       ( !bTableRowKeep ||
                         !pRow->ShouldRowKeepWithNext() );

    
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

        
        if ( nRowCount == nRepeat && !GetIndPrev())
        {
            pRow = pOldRow;
        }
    }

    
    
    
    if ( !bSplitRowAllowed )
    {
        SwRowFrm* pFirstNonHeadlineRow = GetFirstNonHeadlineRow();
        if ( pRow == pFirstNonHeadlineRow )
            return false;

        
        
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

        
        (pFoll->Frm().*fnRect->fnAddWidth)( (Frm().*fnRect->fnGetWidth)() );
        (pFoll->Prt().*fnRect->fnAddWidth)( (Prt().*fnRect->fnGetWidth)() );
        (pFoll->Frm().*fnRect->fnSetLeft)( (Frm().*fnRect->fnGetLeft)() );

        
        pFoll->InsertBehind( GetUpper(), this );

        
        for ( nRowCount = 0; nRowCount < nRepeat; ++nRowCount )
        {
            
            bDontCreateObjects = true;              
            SwRowFrm* pHeadline = new SwRowFrm(
                                    *GetTable()->GetTabLines()[ nRowCount ], this );
            pHeadline->SetRepeatedHeadline( true );
            bDontCreateObjects = false;
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

    SwRowFrm* pLastRow = 0;     
    SwRowFrm* pFollowRow = 0;   
                                

    if ( bSplitRowAllowed )
    {
        
        
        pLastRow = pRow;
        pRow = static_cast<SwRowFrm*>(pRow->GetNext());

        
        pFollowRow = lcl_InsertNewFollowFlowLine( *this, *pLastRow, false );
    }
    else
    {
        pFollowRow = pRow;

        
        
        
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

        
        if ( pLastRow )
            pFollowRow = lcl_InsertNewFollowFlowLine( *this, *pLastRow, true );
    }

    SwTwips nRet = 0;

    
    
    
    if ( bNewFollow )
    {
        SwFrm* pNxt = 0;
        SwFrm* pInsertBehind = pFoll->GetLastLower();

        while ( pRow )
        {
            pNxt = pRow->GetNext();
            nRet += (pRow->Frm().*fnRect->fnGetHeight)();
            
            
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

            
            lcl_MoveFootnotes( *this, *GetFollow(), *pRow );

            pRow->Remove();
            pRow->Paste( pFoll, pPasteBefore );

            pRow->CheckDirChange();
            pRow = static_cast<SwRowFrm*>(pNxt);
        }
    }

    Shrink( nRet );

    
    if ( pLastRow )
    {
        
        bRet = lcl_RecalcSplitLine( *pLastRow, *pFollowRow, nRemainingSpaceForLastRow );

        
        
        if ( bRet && pFollowRow->IsRowSpanLine() )
            lcl_AdjustRowSpanCells( pFollowRow );

        
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
        pFoll->Cut();   

        SwFrm *pRow = pFoll->GetFirstNonHeadlineRow(),
              *pNxt;

        SwFrm* pPrv = GetLastLower();

        SwTwips nHeight = 0;    

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

void SwInvalidatePositions( SwFrm *pFrm, long nBottom )
{
    
    bool bAll = LONG_MAX == nBottom;
    SWRECTFN( pFrm )
    do
    {   pFrm->_InvalidatePos();
        pFrm->_InvalidateSize();
        if( pFrm->IsLayoutFrm() )
        {
            if ( ((SwLayoutFrm*)pFrm)->Lower() )
            {
                ::SwInvalidatePositions( ((SwLayoutFrm*)pFrm)->Lower(), nBottom);
                
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
    
    bool bAll = LONG_MAX == nBottom;
    SWRECTFN( pFrm )
    do
    {
        pFrm->_InvalidatePos();
        pFrm->_InvalidateSize();
        pFrm->_InvalidatePrt();
        if( pFrm->IsLayoutFrm() )
        {
            
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

bool SwCntntFrm::CalcLowers( SwLayoutFrm* pLay, const SwLayoutFrm* pDontLeave,
                                 long nBottom, bool bSkipRowSpanCells )
{
    if ( !pLay )
        return true;

    
    bool bAll = LONG_MAX == nBottom;
    bool bRet = false;
    SwCntntFrm *pCnt = pLay->ContainsCntnt();
    SWRECTFN( pLay )

    
    sal_uInt16 nLoopControlRuns = 0;
    const sal_uInt16 nLoopControlMax = 10;
    const SwModify* pLoopControlCond = 0;

    while ( pCnt && pDontLeave->IsAnLower( pCnt ) )
    {
        
        
        
        const bool bFormatPossible = !pCnt->IsJoinLocked() &&
                                     ( !pCnt->IsTxtFrm() ||
                                       !static_cast<SwTxtFrm*>(pCnt)->IsLocked() ) &&
                                     ( pCnt->IsFollow() || !StackHack::IsLocked() );

        
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
            
            
            
            pCnt->Calc();
            
            
            
            
            OSL_ENSURE( !pCnt->IsTxtFrm() ||
                    pCnt->IsValid() ||
                    static_cast<SwTxtFrm*>(pCnt)->IsJoinLocked(),
                    "<SwCntntFrm::CalcLowers(..)> - text frame invalid and not locked." );
            if ( pCnt->IsTxtFrm() && pCnt->IsValid() )
            {
                
                
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



static bool lcl_InnerCalcLayout( SwFrm *pFrm,
                                      long nBottom,
                                      bool _bOnlyRowsAndCells )
{
    
    bool bAll = LONG_MAX == nBottom;
    bool bRet = false;
    const SwFrm* pOldUp = pFrm->GetUpper();
    SWRECTFN( pFrm )
    do
    {
        
        
        if ( pFrm->IsLayoutFrm() &&
             ( !_bOnlyRowsAndCells || pFrm->IsRowFrm() || pFrm->IsCellFrm() ) )
        {
            
            
            
            bRet |= !pFrm->IsValid() && ( !pFrm->IsTabFrm() || !static_cast<SwTabFrm*>(pFrm)->IsJoinLocked() );
            pFrm->Calc();
            if( static_cast<SwLayoutFrm*>(pFrm)->Lower() )
                bRet |= lcl_InnerCalcLayout( static_cast<SwLayoutFrm*>(pFrm)->Lower(), nBottom);

            
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
    
    sal_uInt16 nLoopControlRuns_1 = 0;
    sal_uInt16 nLoopControlStage_1 = 0;
    const sal_uInt16 nLoopControlMax = 10;

    bool bCheck = true;
    do
    {
        
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
            
            
            bCheck = SwCntntFrm::CalcLowers( &rRow, rRow.GetUpper(), nBottom, true );

            
            
            
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




static bool lcl_NoPrev( const SwFrm& rFrm )
{
    
    
    
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

    
    
    
    if ( rFrm.IsInSct() )
    {
        const SwFrm* pSct = rFrm.GetUpper();
        if ( pSct && pSct->IsColBodyFrm() &&
             pSct->GetUpper()->GetUpper()->IsSctFrm() )
        {
            const SwFrm* pPrevCol = rFrm.GetUpper()->GetUpper()->GetPrev();
            if ( pPrevCol )
                
                
                return true;
        }
    }

    return false;
}

#define KEEPTAB ( !GetFollow() && !IsFollow() )





SwFrm* sw_FormatNextCntntForKeep( SwTabFrm* pTabFrm )
{
    
    SwFrm* pNxt = pTabFrm->FindNext();

    
    while ( pNxt && pNxt->IsSctFrm() &&
            !static_cast<SwSectionFrm*>(pNxt)->GetSection() )
    {
        pNxt = pNxt->FindNext();
    }

    
    if ( pNxt && pNxt->IsSctFrm() )
    {
        pNxt = static_cast<SwSectionFrm*>(pNxt)->ContainsAny();
    }

    
    
    
    if ( pNxt )
    {
        if ( pTabFrm->GetUpper()->IsInTab() )
            pNxt->MakeAll();
        else
            pNxt->Calc();
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

    LockJoin(); 
    SwLayNotify aNotify( this );    
    
    
    const sal_Bool bOldValidPos = GetValidPosFlag();

    
    
    
    
    
    
    while ( GetNext() && GetNext() == GetFollow() &&
            !GetFollow()->IsJoinLocked()
          )
    {
        if ( HasFollowFlowLine() )
            RemoveFollowFlowLine();
        Join();
    }

    
    
    if ( IsRemoveFollowFlowLinePending() && HasFollowFlowLine() )
    {
        if ( RemoveFollowFlowLine() )
            Join();
        SetRemoveFollowFlowLinePending( sal_False );
    }

    if ( bResizeHTMLTable ) 
    {
        bResizeHTMLTable = sal_False;
        SwHTMLTableLayout *pLayout = GetTable()->GetHTMLTableLayout();
        if ( pLayout )
            bCalcLowers = pLayout->Resize(
                            pLayout->GetBrowseWidthByTabFrm( *this ), sal_False );
    }

    
    bool bMakePage = true;
    
    bool bMovedBwd = false;
    
    
    bool bMovedFwd  = false;
    
    bool bSplit = false;
    const bool bFtnsInDoc = !GetFmt()->GetDoc()->GetFtnIdxs().empty();
    bool bMoveable;
    const sal_Bool bFly     = IsInFly();

    SwBorderAttrAccess  *pAccess= new SwBorderAttrAccess( SwFrm::GetCache(), this );
    const SwBorderAttrs *pAttrs = pAccess->Get();

    
    const bool bKeep = IsKeep( pAttrs->GetAttrSet() );

    
    
    const bool bDontSplit = !IsFollow() &&
                            ( !GetFmt()->GetLayoutSplit().GetValue() || bKeep );

    
    const sal_uInt16 nRepeat = GetTable()->GetRowsToRepeat();

    
    
    bool bTryToSplit = true;

    
    
    
    const bool bTableRowKeep = !bDontSplit && GetFmt()->GetDoc()->get(IDocumentSettingAccess::TABLE_ROW_KEEP);

    
    
    
    
    
    bool bLastRowHasToMoveToFollow = false;
    bool bLastRowMoveNoMoreTries = false;

    
    if ( bDontSplit )
    {
        while ( GetFollow() && !GetFollow()->IsJoinLocked() )
        {
            if ( HasFollowFlowLine() )
                RemoveFollowFlowLine();
            Join();
        }
    }

    
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

    int nUnSplitted = 5; 
    SWRECTFN( this )
    while ( !mbValidPos || !mbValidSize || !mbValidPrtArea )
    {
        if ( (bMoveable = IsMoveable()) )
            if ( CheckMoveFwd( bMakePage, bKeep && KEEPTAB, bMovedBwd ) )
            {
                bMovedFwd = true;
                bCalcLowers = sal_True;
                
                
                
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

                mbValidPrtArea = sal_False;
                aNotify.SetLowersComplete( sal_False );
            }
            SwFrm *pPre;
            if ( bKeep || (0 != (pPre = FindPrev()) &&
                pPre->GetAttrSet()->GetKeep().GetValue()) )
            {
                bCalcLowers = sal_True;
                
                
                
                bSplit = false;
            }
        }

        
        
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

        
        
        
        
        if ( !bMovedFwd && (bMoveable || bFly) && lcl_NoPrev( *this ) )
        {
            
            
            if ( IsFollow() )
            {
                
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

                            pAccess= new SwBorderAttrAccess( SwFrm::GetCache(), this );
                            pAttrs = pAccess->Get();
                        }

                        mbValidPrtArea = sal_False;
                        Format( pAttrs );
                    }
                    lcl_RecalcTable( *this, 0, aNotify );
                    bLowersFormatted = sal_True;
                    if ( bKeep && KEEPTAB )
                    {
                        //
                        
                        
                        
                        
                        
                        
                        if ( 0 != sw_FormatNextCntntForKeep( this ) && !GetNext() )
                        {
                            mbValidPos = sal_False;
                        }
                    }
                }
            }
        }

        
        if ( !mbValidPos || !mbValidSize || !mbValidPrtArea )
            continue;

        

        
        
        
        
        SwTwips nDistanceToUpperPrtBottom =
                (Frm().*fnRect->fnBottomDist)( (GetUpper()->*fnRect->fnGetPrtBottom)());

        
        const SwViewShell *pSh = getRootFrm()->GetCurrShell();
        const bool bBrowseMode = pSh && pSh->GetViewOptions()->getBrowseMode();
        if ( nDistanceToUpperPrtBottom < 0 && bBrowseMode )
        {
            if ( GetUpper()->Grow( -nDistanceToUpperPrtBottom ) )
            {
                
                nDistanceToUpperPrtBottom = (Frm().*fnRect->fnBottomDist)( (GetUpper()->*fnRect->fnGetPrtBottom)());
            }
        }

        
        
        
        
        if( nDistanceToUpperPrtBottom >= 0 && !bLastRowHasToMoveToFollow )
        {
            
            
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
                        
                        if ( HasFollowFlowLine() )
                        {
                            SwFrm* pLastLine = const_cast<SwFrm*>(GetLastLower());
                            RemoveFollowFlowLine();
                            
                            if ( pLastLine )
                            {
                                ::SwInvalidateAll( pLastLine, LONG_MAX );
                                SetRebuildLastLine( sal_True );
                                lcl_RecalcRow( static_cast<SwRowFrm&>(*pLastLine), LONG_MAX );
                                SetRebuildLastLine( sal_False );
                            }

                            SwFrm* pRow = GetFollow()->GetFirstNonHeadlineRow();

                            if ( !pRow || !pRow->GetNext() )
                                
                                Join();

                            continue;
                        }

                        
                        
                        SwRowFrm *pRow = GetFollow()->GetFirstNonHeadlineRow();

                        
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
                            const bool bMoveFtns = bFtnsInDoc && !GetFollow()->IsJoinLocked();

                            SwFtnBossFrm *pOldBoss = 0;
                            if ( bMoveFtns )
                                pOldBoss = pRowToMove->FindFtnBossFrm( sal_True );

                            SwFrm* pNextRow = pRowToMove->GetNext();

                            if ( !pNextRow )
                            {
                                
                                Join();
                            }
                            else
                            {
                                pRowToMove->Cut();
                                pRowToMove->Paste( this );
                            }

                            
                            if ( bMoveFtns )
                                if ( ((SwLayoutFrm*)pRowToMove)->MoveLowerFtns( 0, pOldBoss, FindFtnBossFrm( sal_True ), sal_True ) )
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
                    
                    
                    bLastRowMoveNoMoreTries = true;

                    
                    
                    
                    
                    
                    
                    
                    
                    const SwRowFrm* pLastRow = static_cast<const SwRowFrm*>(GetLastLower());
                    if ( pLastRow && IsKeep( pAttrs->GetAttrSet(), true ) &&
                         pLastRow->ShouldRowKeepWithNext() )
                        bFormat = true;
                }

                if ( bFormat )
                {
                    delete pAccess;

                    //
                    
                    
                    
                    
                    const SwFrm* pTmpNxt = sw_FormatNextCntntForKeep( this );

                    pAccess= new SwBorderAttrAccess( SwFrm::GetCache(), this );
                    pAttrs = pAccess->Get();

                    
                    
                    
                    if ( !bKeep && !GetNext() && pTmpNxt && pTmpNxt->IsValid() )
                    {
                        mbValidPos = sal_False;
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

        
        
        
        
        SwFrm* pIndPrev = GetIndPrev();
        const SwRowFrm* pFirstNonHeadlineRow = GetFirstNonHeadlineRow();
        
        
        const bool bAllowSplitOfRow = ( bTableRowKeep &&
                                        AreAllRowsKeepWithNext( pFirstNonHeadlineRow ) ) &&
                                      !pIndPrev &&
                                      !bDontSplit;

        if ( pFirstNonHeadlineRow && nUnSplitted > 0 &&
             ( !bTableRowKeep || pFirstNonHeadlineRow->GetNext() || !pFirstNonHeadlineRow->ShouldRowKeepWithNext() || bAllowSplitOfRow ) &&
             ( !bDontSplit || !pIndPrev ) )
        {
            
            
            
            
            
            if ( IsInSct() &&
                (FindSctFrm())->Lower()->IsColumnFrm() &&
                0 == (GetUpper()->Frm().*fnRect->fnGetHeight)()  )
            {
                bTryToSplit = false;
            }

            
            
            if ( pFirstNonHeadlineRow->GetNext() || bTryToSplit )
            {
                SwTwips nDeadLine = (GetUpper()->*fnRect->fnGetPrtBottom)();
                if( IsInSct() || GetUpper()->IsInTab() ) 
                    nDeadLine = (*fnRect->fnYInc)( nDeadLine,
                                        GetUpper()->Grow( LONG_MAX, sal_True ) );

                {
                    SetInRecalcLowerRow( true );
                    ::lcl_RecalcRow( static_cast<SwRowFrm&>(*Lower()), nDeadLine );
                    SetInRecalcLowerRow( false );
                }
                bLowersFormatted = sal_True;
                aNotify.SetLowersComplete( sal_True );

                
                
                
                if( (Frm().*fnRect->fnBottomDist)( nDeadLine ) > 0 && !bLastRowHasToMoveToFollow )
                {
                    continue;
                }

                
                bLastRowHasToMoveToFollow = false;

                
                
                
                
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

                
                
                
                if( (*fnRect->fnYDiff)(nDeadLine, nBreakLine) >=0 || !pIndPrev )
                {
                    aNotify.SetLowersComplete( sal_False );
                    bSplit = true;

                    
                    if ( HasFollowFlowLine() )
                    {
                        RemoveFollowFlowLine();
                    }

                    const bool bSplitError = !Split( nDeadLine, bTryToSplit, ( bTableRowKeep && !bAllowSplitOfRow ) );
                    if( !bTryToSplit && !bSplitError && nUnSplitted > 0 )
                    {
                        --nUnSplitted;
                    }

                    
                    
                    
                    if ( bSplitError )
                    {
                        if ( HasFollowFlowLine() )
                            RemoveFollowFlowLine();
                    }

                    
                    
                    
                    if ( GetFollow() && !GetFollow()->GetFirstNonHeadlineRow() )
                        Join();

                    
                    
                    
                    
                    if ( bSplitError && bTryToSplit ) 
                    {
                        lcl_RecalcRow( static_cast<SwRowFrm&>(*Lower()), LONG_MAX );
                        mbValidPos = sal_False;
                        bTryToSplit = false;
                        continue;
                    }

                    bTryToSplit = !bSplitError;

                    
                    if ( GetFollow() )
                    {
                        
                        
                        
                        
                        
                        
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
                            
                            
                            
                            const bool bOldJoinLock =  GetFollow()->IsJoinLocked();
                            GetFollow()->LockJoin();
                            ::lcl_RecalcRow( static_cast<SwRowFrm&>(*GetFollow()->Lower()),
                                             (GetFollow()->GetUpper()->Frm().*fnRectX->fnGetBottom)() );
                            
                            
                            
                            if ( !bOldJoinLock )
                                GetFollow()->UnlockJoin();

                            if ( !GetFollow()->GetFollow() )
                            {
                                SwFrm* pNxt = ((SwFrm*)GetFollow())->FindNext();
                                if ( pNxt )
                                {
                                    
                                    
                                    
                                    
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

        
        bLastRowHasToMoveToFollow = false;

        if( IsInSct() && bMovedFwd && bMakePage && GetUpper()->IsColBodyFrm() &&
            GetUpper()->GetUpper()->GetUpper()->IsSctFrm() &&
            ( GetUpper()->GetUpper()->GetPrev() || GetIndPrev() ) &&
            ((SwSectionFrm*)GetUpper()->GetUpper()->GetUpper())->MoveAllowed(this) )
        {
            bMovedFwd = false;
        }

        
        const SwFrm* pOldUpper = GetUpper();

        
        if ( !bMovedFwd && !MoveFwd( bMakePage, sal_False ) )
            bMakePage = false;

        
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
            
            SwTabFrm *pTab = FindMaster();
            if ( pTab->GetUpper() )
                pTab->GetUpper()->Calc();
            pTab->Calc();
            pTab->SetLowersFormatted( sal_False );
        }

        
        if ( ( GetNext() && GetNext() == GetFollow() ) || !GetLower() )
        {
            if ( HasFollowFlowLine() )
                RemoveFollowFlowLine();
            if ( GetFollow() )
                Join();
        }

        if ( bMovedBwd && GetUpper() )
        {
            
            
            GetUpper()->ResetCompletePaint();
        }

        if ( bCalcLowers && IsValid() )
        {
            
            
            
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

    } 

    
    
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


sal_Bool SwTabFrm::CalcFlyOffsets( SwTwips& rUpper,
                               long& rLeftOffset,
                               long& rRightOffset ) const
{
    sal_Bool bInvalidatePrtArea = sal_False;
    const SwPageFrm *pPage = FindPageFrm();
    const SwFlyFrm* pMyFly = FindFlyFrm();

    
    

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
                
                
                
                
                
                
                
                
                
                
                
                
                
                
                
                
                const SwTxtFrm* pAnchorCharFrm = pFly->FindAnchorCharFrm();
                bool bConsiderFly =
                    
                    
                    pFly->IsValid() &&
                    
                    pFly->IsFlyAtCntFrm() &&
                    
                    aFlyRect.IsOver( aRect ) &&
                    
                    
                    ( !IsAnLower( pFly ) &&
                      ( !pAnchorCharFrm || !IsAnLower( pAnchorCharFrm ) ) ) &&
                    
                    !pFly->IsAnLower( this ) &&
                    
                    
                    
                    
                    
                    ( ( !pMyFly ||
                        pMyFly->IsAnLower( pFly ) ) &&
                      pMyFly == pFly->GetAnchorFrmContainingAnchPos()->FindFlyFrm() ) &&
                    
                    pPage->GetPhyPageNum() >=
                      pFly->GetAnchorFrm()->FindPageFrm()->GetPhyPageNum() &&
                    
                    ( !pAnchorCharFrm ||
                      pAnchorCharFrm->FindPageFrm()->GetPhyPageNum() ==
                        pPage->GetPhyPageNum() );

                if ( bConsiderFly )
                {
                    const SwFrm* pFlyHeaderFooterFrm = pFly->GetAnchorFrm()->FindFooterOrHeader();
                    const SwFrm* pThisHeaderFooterFrm = FindFooterOrHeader();

                    if ( pFlyHeaderFooterFrm != pThisHeaderFooterFrm &&
                        
                        
                        
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
                        rLeftOffset = std::max( rLeftOffset, nWidth );
                        bInvalidatePrtArea = sal_True;
                    }
                    if ( (SURROUND_LEFT     == rSur.GetSurround() ||
                          SURROUND_PARALLEL == rSur.GetSurround())&&
                         text::HoriOrientation::RIGHT == rHori.GetHoriOrient() )
                    {
                        const long nWidth = (*fnRect->fnXDiff)(
                            (pFly->GetAnchorFrm()->Frm().*fnRect->fnGetRight)(),
                            (aFlyRect.*fnRect->fnGetLeft)() );
                        rRightOffset = std::max( rRightOffset, nWidth );
                        bInvalidatePrtArea = sal_True;
                    }
                }
            }
        }
        rUpper = (*fnRect->fnYDiff)( nPrtPos, (Frm().*fnRect->fnGetTop)() );
    }

    return bInvalidatePrtArea;
}



void SwTabFrm::Format( const SwBorderAttrs *pAttrs )
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

    
    
    

    SwTwips nUpper = CalcUpperSpace( pAttrs );

    
    
    
    
    long nTmpRight = -1000000,
         nLeftOffset  = 0;
    if( CalcFlyOffsets( nUpper, nLeftOffset, nTmpRight ) )
        mbValidPrtArea = sal_False;
    long nRightOffset = std::max( 0L, nTmpRight );

    SwTwips nLower = pAttrs->CalcBottomLine();
    
    if ( IsCollapsingBorders() )
        nLower += GetBottomLineSize();

    if ( !mbValidPrtArea )
    {   mbValidPrtArea = sal_True;

        
        
        
        
        
        
        

        const SwTwips nOldHeight = (Prt().*fnRect->fnGetHeight)();
        const SwTwips nMax = (maFrm.*fnRect->fnGetWidth)();

        
        const SwTwips nLeftLine  = pAttrs->CalcLeftLine();
        const SwTwips nRightLine = pAttrs->CalcRightLine();

        
        
        
        const SwFmtFrmSize &rSz = GetFmt()->GetFrmSize();
        
        const SwTwips nWishedTableWidth = CalcRel( rSz, sal_True );

        bool bCheckBrowseWidth = false;

        
        SwTwips nLeftSpacing  = 0;
        SwTwips nRightSpacing = 0;
        switch ( GetFmt()->GetHoriOrient().GetHoriOrient() )
        {
            case text::HoriOrientation::LEFT:
                {
                    
                    nLeftSpacing = nLeftLine + nLeftOffset;
                    
                    
                    
                    
                    
                    const SwTwips nWishRight = nMax - nWishedTableWidth - nLeftOffset;
                    if ( nRightOffset > 0 )
                    {
                        
                        
                        
                        nRightSpacing = nRightLine + std::max( nRightOffset, nWishRight );
                    }
                    else
                    {
                        
                        
                        
                        
                        
                        nRightSpacing = nRightLine +
                                        ( ( (nWishRight+nLeftOffset) < 0 ) ?
                                            (nWishRight+nLeftOffset) :
                                            std::max( 0L, nWishRight ) );
                    }
                }
                break;
            case text::HoriOrientation::RIGHT:
                {
                    
                    nRightSpacing = nRightLine + nRightOffset;
                    
                    
                    
                    
                    
                    const SwTwips nWishLeft = nMax - nWishedTableWidth - nRightOffset;
                    if ( nLeftOffset > 0 )
                    {
                        
                        
                        
                        nLeftSpacing = nLeftLine + std::max( nLeftOffset, nWishLeft );
                    }
                    else
                    {
                        
                        
                        
                        
                        
                        nLeftSpacing = nLeftLine +
                                       ( ( (nWishLeft+nRightOffset) < 0 ) ?
                                           (nWishLeft+nRightOffset) :
                                           std::max( 0L, nWishLeft ) );
                    }
                }
                break;
            case text::HoriOrientation::CENTER:
                {
                    
                    
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
                    
                    
                    
                    
                    bCheckBrowseWidth = true;
                    nLeftSpacing  = nLeftLine + nLeftOffset;
                    nRightSpacing = nRightLine + nRightOffset;
                break;
            case text::HoriOrientation::NONE:
                {
                    
                    nLeftSpacing = pAttrs->CalcLeft( this );
                    if( nLeftOffset )
                    {
                        
                        
                        
                        
                        nLeftSpacing = std::max( nLeftSpacing, ( nLeftOffset + nLeftLine ) );
                    }
                    
                    nRightSpacing = pAttrs->CalcRight( this );
                    if( nRightOffset )
                    {
                        
                        
                        
                        
                        nRightSpacing = std::max( nRightSpacing, ( nRightOffset + nRightLine ) );
                    }
                }
                break;
            case text::HoriOrientation::LEFT_AND_WIDTH:
                {
                    
                    
                    
                    nLeftSpacing = pAttrs->CalcLeft( this );
                    if( nLeftOffset )
                    {
                        
                        
                        
                        
                        nLeftSpacing = std::max( nLeftSpacing, ( pAttrs->CalcLeftLine() + nLeftOffset ) );
                    }
                    
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

        SwViewShell *pSh = getRootFrm()->GetCurrShell();
        if ( bCheckBrowseWidth &&
             pSh && pSh->GetViewOptions()->getBrowseMode() &&
             GetUpper()->IsPageBodyFrm() &&  
             pSh->VisArea().Width() )
        {
            
            
            
            long nWidth = pSh->GetBrowseWidth();
            nWidth -= Prt().Left();
            nWidth -= pAttrs->CalcRightLine();
            Prt().Width( std::min( nWidth, Prt().Width() ) );
        }

        if ( nOldHeight != (Prt().*fnRect->fnGetHeight)() )
            mbValidSize = sal_False;
    }

    if ( !mbValidSize )
    {
        mbValidSize = sal_True;

        
        SwTwips nRemaining = 0, nDiff;
        SwFrm *pFrm = pLower;
        while ( pFrm )
        {
            nRemaining += (pFrm->Frm().*fnRect->fnGetHeight)();
            pFrm = pFrm->GetNext();
        }
        
        nRemaining += nUpper + nLower;

        nDiff = (Frm().*fnRect->fnGetHeight)() - nRemaining;
        if ( nDiff > 0 )
            Shrink( nDiff );
        else if ( nDiff < 0 )
            Grow( -nDiff );
    }
}

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
            if ( GetNext()->IsCntntFrm() )
                GetNext()->InvalidatePage( pPage );
        }
        
        
        
        
        
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

void SwTabFrm::Modify( const SfxPoolItem* pOld, const SfxPoolItem * pNew )
{
    sal_uInt8 nInvFlags = 0;
    bool bAttrSetChg = pNew && RES_ATTRSET_CHG == pNew->Which();

    if( bAttrSetChg )
    {
        SfxItemIter aNIter( *((SwAttrSetChg*)pNew)->GetChgSet() );
        SfxItemIter aOIter( *((SwAttrSetChg*)pOld)->GetChgSet() );
        SwAttrSetChg aOldSet( *(SwAttrSetChg*)pOld );
        SwAttrSetChg aNewSet( *(SwAttrSetChg*)pNew );
        while( true )
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
    bool bClear = true;
    const sal_uInt16 nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;
    switch( nWhich )
    {
        case RES_TBLHEADLINECHG:
            if ( IsFollow() )
            {
                
                SwRowFrm* pLowerRow = 0;
                while ( 0 != ( pLowerRow = (SwRowFrm*)Lower() ) && pLowerRow->IsRepeatedHeadline() )
                {
                    pLowerRow->Cut();
                    delete pLowerRow;
                }

                
                const sal_uInt16 nNewRepeat = GetTable()->GetRowsToRepeat();
                for ( sal_uInt16 nIdx = 0; nIdx < nNewRepeat; ++nIdx )
                {
                    bDontCreateObjects = true;          
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

        case RES_PAGEDESC:                      
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

bool SwTabFrm::GetInfo( SfxPoolItem &rHnt ) const
{
    if ( RES_VIRTPAGENUM_INFO == rHnt.Which() && IsInDocBody() && !IsFollow() )
    {
        SwVirtPageNumInfo &rInfo = (SwVirtPageNumInfo&)rHnt;
        const SwPageFrm *pPage = FindPageFrm();
        if ( pPage  )
        {
            if ( pPage == rInfo.GetOrigPage() && !GetPrev() )
            {
                
                
                rInfo.SetInfo( pPage, this );
                return false;
            }
            if ( pPage->GetPhyPageNum() < rInfo.GetOrigPage()->GetPhyPageNum() &&
                 (!rInfo.GetPage() || pPage->GetPhyPageNum() > rInfo.GetPage()->GetPhyPageNum()))
            {
                
                rInfo.SetInfo( pPage, this );
            }
        }
    }
    return true;
}

SwCntntFrm *SwTabFrm::FindLastCntnt()
{
    SwFrm *pRet = pLower;

    while ( pRet && !pRet->IsCntntFrm() )
    {
        SwFrm *pOld = pRet;

        SwFrm *pTmp = pRet;             
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

    
    
    
    
    
    if ( pRet )
    {
        while ( pRet->GetNext() )
            pRet = pRet->GetNext();

        if( pRet->IsSctFrm() )
            pRet = ((SwSectionFrm*)pRet)->FindLastCntnt();
    }

    return (SwCntntFrm*)pRet;
}


sal_Bool SwTabFrm::ShouldBwdMoved( SwLayoutFrm *pNewUpper, sal_Bool, sal_Bool &rReformat )
{
    rReformat = sal_False;
    if ( (SwFlowFrm::IsMoveBwdJump() || !IsPrevObjMove()) )
    {
        
        
        
        
        
        
        
        
        
        

        
        

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

                    
                    
                    
                    
                    
                    
                    
                    const SwTwips nTmpSpace = (aRect.*fnRectX->fnGetHeight)();
                    if ( (pNewUpper->Prt().*fnRectX->fnGetHeight)() > 0 || nTmpSpace <= 0 )
                        nSpace = nTmpSpace;

                    const SwViewShell *pSh = getRootFrm()->GetCurrShell();
                    if( pSh && pSh->GetViewOptions()->getBrowseMode() )
                        nSpace += pNewUpper->Grow( LONG_MAX, sal_True );
                }
            }
            else if( !bLockBackMove )
                bMoveAnyway = true;
        }
        else if( !bLockBackMove )
            bMoveAnyway = true;

        if ( bMoveAnyway )
            return rReformat = sal_True;
        else if ( !bLockBackMove && nSpace > 0 )
        {
            
            
            
            SwRowFrm* pFirstRow = GetFirstNonHeadlineRow();
            if ( pFirstRow && pFirstRow->IsInFollowFlowRow() &&
                 SwLayouter::DoesRowContainMovedFwdFrm(
                                            *(pFirstRow->GetFmt()->GetDoc()),
                                            *(pFirstRow) ) )
            {
                return sal_False;
            }
            SwTwips nTmpHeight = CalcHeightOfFirstContentLine();

            
            
            
            
            
            return nTmpHeight <= nSpace;
        }
    }
    return sal_False;
}

void SwTabFrm::Cut()
{
    OSL_ENSURE( GetUpper(), "Cut ohne Upper()." );

    SwPageFrm *pPage = FindPageFrm();
    InvalidatePage( pPage );
    SwFrm *pFrm = GetNext();
    if( pFrm )
    {
        
        
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
        
        if ( 0 != (pFrm = GetPrev()) )
        {   pFrm->SetRetouche();
            pFrm->Prepare( PREP_WIDOWS_ORPHANS );
            pFrm->_InvalidatePos();
            if ( pFrm->IsCntntFrm() )
                pFrm->InvalidatePage( pPage );
        }
        
        
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

    
    SwLayoutFrm *pUp = GetUpper();
    SWRECTFN( this )
    Remove();
    if ( pUp )
    {
        OSL_ENSURE( !pUp->IsFtnFrm(), "Table in Footnote." );
        SwSectionFrm *pSct = 0;
        
        
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
            
            
            pUp->Shrink( Frm().Height() );
        }
    }

    if ( pPage && !IsFollow() && pPage->GetUpper() )
        ((SwRootFrm*)pPage->GetUpper())->InvalidateBrowseWidth();
}

void SwTabFrm::Paste( SwFrm* pParent, SwFrm* pSibling )
{
    OSL_ENSURE( pParent, "No parent for pasting." );
    OSL_ENSURE( pParent->IsLayoutFrm(), "Parent is CntntFrm." );
    OSL_ENSURE( pParent != this, "I'm the parent myself." );
    OSL_ENSURE( pSibling != this, "I'm my own neighbour." );
    OSL_ENSURE( !GetPrev() && !GetNext() && !GetUpper(),
            "I'm still registred somewhere." );

    
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
        
        
        
        
        GetNext()->_InvalidatePrt();

    if ( pPage && !IsFollow() )
    {
        if ( pPage->GetUpper() )
            ((SwRootFrm*)pPage->GetUpper())->InvalidateBrowseWidth();

        if ( !GetPrev() )
        {
            const SwPageDesc *pDesc = GetFmt()->GetPageDesc().GetPageDesc();
            if ( (pDesc && pDesc != pPage->GetPageDesc()) ||
                 (!pDesc && pPage->GetPageDesc() != &GetFmt()->GetDoc()->GetPageDesc(0)) )
                CheckPageDescs( pPage, sal_True );
        }
    }
}

void SwTabFrm::Prepare( const PrepareHint eHint, const void *, sal_Bool )
{
    if( PREP_BOSS_CHGD == eHint )
        CheckDirChange();
}

SwRowFrm::SwRowFrm( const SwTableLine &rLine, SwFrm* pSib, bool bInsertContent ):
    SwLayoutFrm( rLine.GetFrmFmt(), pSib ),
    pTabLine( &rLine ),
    pFollowRow( 0 ),
    
    mnTopMarginForLowers( 0 ),
    mnBottomMarginForLowers( 0 ),
    mnBottomLineSize( 0 ),
    
    bIsFollowFlowRow( false ),
    
    bIsRepeatedHeadline( false ),
    mbIsRowSpanLine( false )
{
    mnType = FRMC_ROW;

    
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
        pMod->Remove( this );           
        if( !pMod->GetDepends() )
            delete pMod;                
    }
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
        const SwAttrSet* pChgSet = ((SwAttrSetChg*)pNew)->GetChgSet();
        pChgSet->GetItemState( RES_FRM_SIZE, false, &pItem);
        if ( !pItem )
            pChgSet->GetItemState( RES_ROW_SPLIT, false, &pItem);
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

void SwRowFrm::MakeAll()
{
    if ( !GetNext() )
        mbValidSize = sal_False;
    SwLayoutFrm::MakeAll();
}

long CalcHeightWidthFlys( const SwFrm *pFrm )
{
    SWRECTFN( pFrm )
    long nHeight = 0;
    const SwFrm* pTmp = pFrm->IsSctFrm() ?
            ((SwSectionFrm*)pFrm)->ContainsCntnt() : pFrm;
    while( pTmp )
    {
        
        const SwSortedObjs* pObjs( 0L );
        bool bIsFollow( false );
        if ( pTmp->IsTxtFrm() && static_cast<const SwTxtFrm*>(pTmp)->IsFollow() )
        {
            const SwFrm* pMaster;
            
            
            
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
                
                
                if ( bIsFollow &&
                     const_cast<SwAnchoredObject*>(pAnchoredObj)->FindAnchorCharFrm() != pTmp )
                {
                    continue;
                }
                
                {
                    
                    
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

                            nHeight = std::max( nHeight, nDistOfFlyBottomToAnchorTop + nFrmDiff -
                                            (pFrm->Frm().*fnRect->fnGetHeight)() );

                            
                            
                            
                            
                            
                            
                            
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

    
    if ( pTab->IsCollapsingBorders() && rCell.Lower() && !rCell.Lower()->IsRowFrm() )
    {
        nTopSpace    = ((SwRowFrm*)rCell.GetUpper())->GetTopMarginForLowers();
        nBottomSpace = ((SwRowFrm*)rCell.GetUpper())->GetBottomMarginForLowers();
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
            
            
            if ( pLow->IsRowFrm() )
            {
                
                nHeight += ::lcl_CalcMinRowHeight( static_cast<const SwRowFrm*>(pLow),
                                                   _bConsiderObjs );
            }
            else
            {
                long nLowHeight = (pLow->Frm().*fnRect->fnGetHeight)();
                nHeight += nLowHeight;
                
                if ( _bConsiderObjs )
                {
                    nFlyAdd = std::max( 0L, nFlyAdd - nLowHeight );
                    nFlyAdd = std::max( nFlyAdd, ::CalcHeightWidthFlys( pLow ) );
                }
            }

            pLow = pLow->GetNext();
        }
        if ( nFlyAdd )
            nHeight += nFlyAdd;
    }
    
    
    
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
        
        
        
        
        
        if ( 1 == nRowSpan )
        {
            nTmp = ::lcl_CalcMinCellHeight( pLow, _bConsiderObjs );
        }
        else if ( -1 == nRowSpan )
        {
            
            
            
            const SwCellFrm& rMaster = pLow->FindStartEndOfRowSpanCell( true, true );
            nTmp = ::lcl_CalcMinCellHeight( &rMaster, _bConsiderObjs );
            const SwFrm* pMasterRow = rMaster.GetUpper();
            while ( pMasterRow && pMasterRow != _pRow )
            {
                nTmp -= (pMasterRow->Frm().*fnRect->fnGetHeight)();
                pMasterRow = pMasterRow->GetNext();
            }
        }
        

        
        if ( pLow->IsVertical() == bVert && nTmp > nHeight )
            nHeight = nTmp;

        pLow = static_cast<const SwCellFrm*>(pLow->GetNext());
    }
    if ( rSz.GetHeightSizeType() == ATT_MIN_SIZE && !_pRow->IsRowSpanLine() )
        nHeight = std::max( nHeight, rSz.GetHeight() );
    return nHeight;
}




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
            nTmpTopSpace = rBoxItem.CalcLineSpace( BOX_LINE_TOP, true );
        }
        nTopSpace  = std::max( nTopSpace, nTmpTopSpace );
    }
    return nTopSpace;
}


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
        nTopLineDist = std::max( nTopLineDist, nTmpTopLineDist );
    }
    return nTopLineDist;
}


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
            nTmpBottomLineSize = rBoxItem.CalcLineSpace( BOX_LINE_BOTTOM, true ) -
                                 rBoxItem.GetDistance( BOX_LINE_BOTTOM );
        }
        nBottomLineSize = std::max( nBottomLineSize, nTmpBottomLineSize );
    }
    return nBottomLineSize;
}


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
        nBottomLineDist = std::max( nBottomLineDist, nTmpBottomLineDist );
    }
    return nBottomLineDist;
}

void SwRowFrm::Format( const SwBorderAttrs *pAttrs )
{
    SWRECTFN( this )
    OSL_ENSURE( pAttrs, "SwRowFrm::Format without Attrs." );

    const sal_Bool bFix = mbFixSize;

    if ( !mbValidPrtArea )
    {
        
        
        mbValidPrtArea = sal_True;
        maPrt.Left( 0 );
        maPrt.Top( 0 );
        maPrt.Width ( maFrm.Width() );
        maPrt.Height( maFrm.Height() );

        
        
        SwTabFrm* pTabFrm = FindTabFrm();
        if ( pTabFrm->IsCollapsingBorders() )
        {
            const sal_uInt16 nTopSpace        = lcl_GetTopSpace(       *this );
            const sal_uInt16 nTopLineDist     = lcl_GetTopLineDist(    *this );
            const sal_uInt16 nBottomLineSize  = lcl_GetBottomLineSize( *this );
            const sal_uInt16 nBottomLineDist  = lcl_GetBottomLineDist( *this );

            const SwRowFrm* pPreviousRow = 0;

            
            
            
            
            
            
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
                    
                    pPrevTabLine = rLines[ nIdx - 1 ];
                }
                else
                {
                    
                    
                    pTmpRow = pTmpRow->GetUpper()->GetUpper() &&
                              pTmpRow->GetUpper()->GetUpper()->IsRowFrm() ?
                              static_cast<const SwRowFrm*>( pTmpRow->GetUpper()->GetUpper() ) :
                              0;
                }
            }

            
            if ( pPrevTabLine )
            {
                SwIterator<SwRowFrm,SwFmt> aIter( *pPrevTabLine->GetFrmFmt() );
                for ( SwRowFrm* pRow = aIter.First(); pRow; pRow = aIter.Next() )
                {
                    
                    
                    
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

            
            
            if ( !GetNext() && nBottomLineSize != GetBottomLineSize() )
                 pTabFrm->_InvalidatePrt();

            
            
            
            
            
            
            
            
            
            if ( GetUpper()->GetUpper()->IsRowFrm() &&
                 ( nBottomLineDist != GetBottomMarginForLowers() ||
                   nTopPrtMargin   != GetTopMarginForLowers() ) )
                GetUpper()->GetUpper()->_InvalidateSize();

            SetBottomMarginForLowers( nBottomLineDist );    
            SetBottomLineSize( nBottomLineSize );           
            SetTopMarginForLowers( nTopPrtMargin );         

        }
    }

    while ( !mbValidSize )
    {
        mbValidSize = sal_True;

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
                                
                                : ::lcl_CalcMinRowHeight( this,
                                    FindTabFrm()->IsConsiderObjsForMinCellHeight() ) );
        if ( nDiff )
        {
            mbFixSize = sal_False;
            if ( nDiff > 0 )
                Shrink( nDiff, sal_False, sal_True );
            else if ( nDiff < 0 )
                Grow( -nDiff );
            mbFixSize = bFix;
        }
    }

    
    if ( !GetNext() )
    {
        
        SwTwips nDiff = (GetUpper()->Prt().*fnRect->fnGetHeight)();
        SwFrm *pSibling = GetUpper()->Lower();
        do
        {   nDiff -= (pSibling->Frm().*fnRect->fnGetHeight)();
            pSibling = pSibling->GetNext();
        } while ( pSibling );
        if ( nDiff > 0 )
        {
            mbFixSize = sal_False;
            Grow( nDiff );
            mbFixSize = bFix;
            mbValidSize = sal_True;
        }
    }
}

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

            
            
            

            
            
            if ( pCellFrm->GetTabBox()->getRowSpan() < 1 )
            {
                
                const long nDiff = nHeight - (pCellFrm->Frm().*fnRect->fnGetHeight)();
                if ( nDiff )
                {
                    (pCellFrm->Frm().*fnRect->fnAddBottom)( nDiff );
                    pCellFrm->_InvalidatePrt();
                }
            }

            SwCellFrm* pToAdjust = 0;
            SwFrm* pToAdjustRow = 0;

            
            
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

            
            long nRowSpan = pToAdjust->GetLayoutRowSpan();
            SwTwips nSumRowHeight = 0;
            while ( pToAdjustRow )
            {
                
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

    
    
    {
        SwRootFrm *pRootFrm = getRootFrm();
        if( pRootFrm && pRootFrm->IsAnyShellAccessible() )
        {
            SwViewShell* pVSh = pRootFrm->GetCurrShell();
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
        pTab->SetRestrictTableGrowth( sal_True );
    else
    {
        
        
        
        
        
        
        pTab->SetFollowFlowLine( sal_False );
    }

    nReal += SwLayoutFrm::GrowFrm( nDist, bTst, bInfo);

    pTab->SetRestrictTableGrowth( sal_False );
    pTab->SetFollowFlowLine( bHasFollowFlowLine );

    
    if ( !bTst )
    {
        SWRECTFNX( this )
        AdjustCells( (Prt().*fnRectX->fnGetHeight)() + nReal, sal_True );
        if ( nReal )
            SetCompletePaint();
    }

    return nReal;
}

SwTwips SwRowFrm::ShrinkFrm( SwTwips nDist, sal_Bool bTst, sal_Bool bInfo )
{
    SWRECTFN( this )
    if( HasFixSize() )
    {
        AdjustCells( (Prt().*fnRect->fnGetHeight)(), sal_True );
        return 0L;
    }

    
    
    const sal_Bool bShrinkAnyway = bInfo;

    
    SwTwips nRealDist = nDist;
    {
        const SwFmtFrmSize &rSz = GetFmt()->GetFrmSize();
        SwTwips nMinHeight = rSz.GetHeightSizeType() == ATT_MIN_SIZE ?
                             rSz.GetHeight() :
                             0;

        
        
        
        if( nMinHeight < (Frm().*fnRect->fnGetHeight)() )
        {
            
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
                SwTabFrm* pMasterTab = const_cast< SwTabFrm* >( pTab->FindMaster() );
                pMasterTab->InvalidatePos();
            }
        }
        AdjustCells( (Prt().*fnRect->fnGetHeight)() - nReal, sal_True );
    }
    return nReal;
}

bool SwRowFrm::IsRowSplitAllowed() const
{
    
    if ( HasFixSize() )
    {
        OSL_ENSURE( ATT_FIX_SIZE == GetFmt()->GetFrmSize().GetHeightSizeType(), "pRow claims to have fixed size" );
        return false;
    }

    
    const SwTabFrm* pTabFrm = FindTabFrm();
    if ( pTabFrm->GetTable()->GetRowsToRepeat() > 0 &&
         pTabFrm->IsInHeadline( *this ) )
        return false;

    const SwTableLineFmt* pFrmFmt = (SwTableLineFmt*)GetTabLine()->GetFrmFmt();
    const SwFmtRowSplit& rLP = pFrmFmt->GetRowSplit();
    return rLP.GetValue();
}

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

SwCellFrm::SwCellFrm( const SwTableBox &rBox, SwFrm* pSib, bool bInsertContent ) :
    SwLayoutFrm( rBox.GetFrmFmt(), pSib ),
    pTabBox( &rBox )
{
    mnType = FRMC_CELL;

    if ( !bInsertContent )
        return;

    
    
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
        
        
        SwRootFrm *pRootFrm = getRootFrm();
        if( pRootFrm && pRootFrm->IsAnyShellAccessible() &&
            pRootFrm->GetCurrShell() )
        {
            pRootFrm->GetCurrShell()->Imp()->DisposeAccessibleFrm( this, sal_True );
        }

        pMod->Remove( this );           
        if( !pMod->GetDepends() )
            delete pMod;                
    }
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
            if ( pFrm->IsLayoutFrm() && ((SwLayoutFrm*)pFrm)->Lower() )
                lcl_ArrangeLowers( (SwLayoutFrm*)pFrm,
                    (((SwLayoutFrm*)pFrm)->Lower()->Frm().*fnRect->fnGetTop)()
                    + lDiffX, bInva );
            if ( pFrm->GetDrawObjs() )
            {
                for ( sal_uInt16 i = 0; i < pFrm->GetDrawObjs()->Count(); ++i )
                {
                    SwAnchoredObject* pAnchoredObj = (*pFrm->GetDrawObjs())[i];
                    
                    
                    
                    
                    if ( !pLay->IsAnLower( pAnchoredObj->GetAnchorFrmContainingAnchPos() ) )
                    {
                        continue;
                    }
                    
                    
                    
                    
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

                        
                        
                        
                        
                        
                        
                        const bool bDirectMove =
                                FAR_AWAY != pFly->Frm().Top() &&
                                bVertPosDepOnAnchor &&
                                !pFly->ConsiderObjWrapInfluenceOnObjPos();
                        if ( bDirectMove )
                        {
                            (pFly->Frm().*fnRect->fnSubTop)( -lDiff );
                            (pFly->Frm().*fnRect->fnAddBottom)( lDiff );
                            pFly->GetVirtDrawObj()->SetRectsDirty();
                            
                            
                            
                            pFly->GetVirtDrawObj()->SetChanged();
                            
                            pFly->InvalidateObjRectWithSpaces();
                        }

                        if ( pFly->IsFlyInCntFrm() )
                        {
                            static_cast<SwFlyInCntFrm*>(pFly)->AddRefOfst( lDiff );
                            
                            
                            if ( !bDirectMove )
                            {
                                pAnchoredObj->SetCurrRelPos( Point( 0, 0 ) );
                            }
                        }
                        else if( pFly->IsAutoPos() )
                        {
                            pFly->AddLastCharY( lDiff );
                            
                            
                            pFly->AddLastTopOfLineY( lDiff );
                        }
                        
                        
                        
                        
                        const SwTabFrm* pTabFrm = pLay->FindTabFrm();
                        
                        
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
                        
                        
                        
                        
                        pFly->InvalidatePos();

                        
                        
                        
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
                        
                        const SwTabFrm* pTabFrm = pLay->FindTabFrm();
                        if ( pTabFrm &&
                             !( pTabFrm->IsFollow() &&
                                pTabFrm->FindMaster()->IsRebuildLastLine() ) &&
                            (pAnchoredObj->GetFrmFmt().GetAnchor().GetAnchorId()
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
                        
                        
                        pAnchoredObj->AddLastCharY( lDiff );
                        pAnchoredObj->AddLastTopOfLineY( lDiff );
                        
                        
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
        
        if( !pFrm->IsColumnFrm() && !pFrm->IsCellFrm() )
            lYStart = (*fnRect->fnYInc)( lYStart,
                                        (pFrm->Frm().*fnRect->fnGetHeight)() );

        
        
        
        
        
        SwTwips nDistanceToUpperPrtBottom =
            (pFrm->Frm().*fnRect->fnBottomDist)( (pLay->*fnRect->fnGetPrtBottom)());
        
        
        
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

    if ( !mbValidPrtArea )
    {
        mbValidPrtArea = sal_True;

        
        if ( Lower() )
        {
            SwTwips nTopSpace, nBottomSpace, nLeftSpace, nRightSpace;
            
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
                
                nLeftSpace   = pAttrs->CalcLeft( this );
                nRightSpace  = pAttrs->CalcRight( this );
                nTopSpace    = pAttrs->CalcTop();
                nBottomSpace = pAttrs->CalcBottom();
            }
            (this->*fnRect->fnSetXMargins)( nLeftSpace, nRightSpace );
            (this->*fnRect->fnSetYMargins)( nTopSpace, nBottomSpace );
        }
    }
    
    long nRemaining = GetTabBox()->getRowSpan() >= 1 ?
                      ::lcl_CalcMinCellHeight( this, pTab->IsConsiderObjsForMinCellHeight(), pAttrs ) :
                      0;
    if ( !mbValidSize )
    {
        mbValidSize = sal_True;

        
        
        
        
        
        
        
        
        SwTwips nWidth;
        if ( GetNext() )
        {
            const SwTwips nWish = pTab->GetFmt()->GetFrmSize().GetWidth();
            nWidth = pAttrs->GetSize().Width();

            OSL_ENSURE( nWish, "Table without width?" );
            OSL_ENSURE( nWidth <= nWish, "Width of cell larger than table." );
            OSL_ENSURE( nWidth > 0, "Box without width" );

            const long nPrtWidth = (pTab->Prt().*fnRect->fnGetWidth)();
            if ( nWish != nPrtWidth )
            {
                
                if ( pTab->GetTable()->IsNewModel() )
                {
                    
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

                    
                    double nTmpWidth = nSumWidth;
                    nTmpWidth *= nPrtWidth;
                    nTmpWidth /= nWish;
                    nWidth = (SwTwips)nTmpWidth;

                    
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

        
        const long nDiffHeight = nRemaining - (Frm().*fnRect->fnGetHeight)();
        if ( nDiffHeight )
        {
            if ( nDiffHeight > 0 )
            {
                
                
                if ( !Grow( nDiffHeight ) )
                    mbValidSize = mbValidPrtArea = sal_True;
            }
            else
            {
                
                
                if ( !Shrink( -nDiffHeight ) )
                    mbValidSize = mbValidPrtArea = sal_True;
            }
        }
    }
    const SwFmtVertOrient &rOri = pAttrs->GetAttrSet().GetVertOrient();

    if ( !Lower() )
        return;

    
    SWREFRESHFN( this )

    SwPageFrm* pPg = 0;
    if ( !FindTabFrm()->IsRebuildLastLine() && text::VertOrientation::NONE != rOri.GetVertOrient() &&
    
         !IsCoveredCell() &&
    
         !(pPg = FindPageFrm())->HasGrid() )
    {
        if ( !Lower()->IsCntntFrm() && !Lower()->IsSctFrm() && !Lower()->IsTabFrm() )
        {
            
            OSL_ENSURE( !this, "VAlign to cell without content" );
            return;
        }
        bool bVertDir = true;
        
        
        
        const bool bConsiderWrapOnObjPos( GetFmt()->getIDocumentSettingAccess()->get(IDocumentSettingAccess::CONSIDER_WRAP_ON_OBJECT_POSITION) );
        
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
                        
                        if ( pAnchoredObj->ISA(SwFlyFrm) )
                        {
                            const SwFlyFrm *pFly =
                                    static_cast<const SwFlyFrm*>(pAnchoredObj);
                            if ( pFly->IsAnLower( this ) )
                                continue;
                        }

                        const SwFrm* pAnch = pAnchoredObj->GetAnchorFrm();
                        
                        
                        
                        
                        
                        
                        if ( bConsiderWrapOnObjPos ||
                             !IsAnLower( pAnch ) ||
                             pAnchoredObj->IsTmpConsiderWrapInfluence() ||
                             !rAnchoredObjFrmFmt.GetFollowTextFlow().GetValue() )
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
        
        if ( Lower()->IsCntntFrm() )
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
        ((SwAttrSetChg*)pNew)->GetChgSet()->GetItemState( RES_VERT_ORIENT, false, &pItem);
    else if ( RES_VERT_ORIENT == pNew->Which() )
        pItem = pNew;

    if ( pItem )
    {
        bool bInva = true;
        if ( text::VertOrientation::NONE == ((SwFmtVertOrient*)pItem)->GetVertOrient() &&
             
             Lower() && Lower()->IsCntntFrm() )
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

    if ( ( bAttrSetChg &&
           SFX_ITEM_SET == ((SwAttrSetChg*)pNew)->GetChgSet()->GetItemState( RES_PROTECT, false ) ) ||
         RES_PROTECT == pNew->Which() )
    {
        SwViewShell *pSh = getRootFrm()->GetCurrShell();
        if( pSh && pSh->GetLayout()->IsAnyShellAccessible() )
            pSh->Imp()->InvalidateAccessibleEditableState( sal_True, this );
    }

    if ( bAttrSetChg &&
         SFX_ITEM_SET == ((SwAttrSetChg*)pNew)->GetChgSet()->GetItemState( RES_FRAMEDIR, false, &pItem ) )
    {
        SetDerivedVert( sal_False );
        CheckDirChange();
    }

    
    if ( bAttrSetChg &&
         SFX_ITEM_SET == ((SwAttrSetChg*)pNew)->GetChgSet()->GetItemState( RES_BOX, false, &pItem ) )
    {
        SwFrm* pTmpUpper = GetUpper();
        while ( pTmpUpper->GetUpper() && !pTmpUpper->GetUpper()->IsTabFrm() )
            pTmpUpper = pTmpUpper->GetUpper();

        SwTabFrm* pTabFrm = (SwTabFrm*)pTmpUpper->GetUpper();
        if ( pTabFrm->IsCollapsingBorders() )
        {
            
            lcl_InvalidateAllLowersPrt( (SwRowFrm*)pTmpUpper );
            pTmpUpper = pTmpUpper->GetNext();
            if ( pTmpUpper )
                lcl_InvalidateAllLowersPrt( (SwRowFrm*)pTmpUpper );
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
        const SwTabFrm* pTab = static_cast<const SwTabFrm*>(pRow->GetUpper());

        if ( pTab && pTab->IsFollow() && pRow == pTab->GetFirstNonHeadlineRow() )
            nRet = -nRet;
    }
    return  nRet;
}


void SwCellFrm::Cut()
{
    
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



sal_uInt16 SwTabFrm::GetBottomLineSize() const
{
    OSL_ENSURE( IsCollapsingBorders(),
            "BottomLineSize only required for collapsing borders" );

    OSL_ENSURE( Lower(), "Warning! Trying to prevent a crash, please inform FME" );

    const SwFrm* pTmp = GetLastLower();

    
    if ( !pTmp ) return 0;

    return static_cast<const SwRowFrm*>(pTmp)->GetBottomLineSize();
}

bool SwTabFrm::IsCollapsingBorders() const
{
    return ((SfxBoolItem&)GetFmt()->GetAttrSet().Get( RES_COLLAPSING_BORDERS )).GetValue();
}


static SwTwips lcl_CalcHeightOfFirstContentLine( const SwRowFrm& rSourceLine )
{
    
    const SwTabFrm* pTab = rSourceLine.FindTabFrm();
    SWRECTFN( pTab )
    const SwCellFrm* pCurrSourceCell = (SwCellFrm*)rSourceLine.Lower();

    
    
    
    //
    
    
    
    bool bIsInFollowFlowLine = rSourceLine.IsInFollowFlowRow();
    SwTwips nHeight = bIsInFollowFlowLine ? LONG_MAX : 0;

    while ( pCurrSourceCell )
    {
        
        
        
        if ( bIsInFollowFlowLine && pCurrSourceCell->GetLayoutRowSpan() > 1 )
        {
            pCurrSourceCell = (SwCellFrm*)pCurrSourceCell->GetNext();
            continue;
        }

        const SwFrm *pTmp = pCurrSourceCell->Lower();
        if ( pTmp )
        {
            SwTwips nTmpHeight = USHRT_MAX;
            
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
                    
                    
                    
                    SwTwips nReal = (pPrevCell->Prt().*fnRect->fnGetHeight)();
                    const SwFrm* pFrm = pPrevCell->Lower();
                    const SwFrm* pLast = pFrm;
                    while ( pFrm )
                    {
                        nReal -= (pFrm->Frm().*fnRect->fnGetHeight)();
                        pLast = pFrm;
                        pFrm = pFrm->GetNext();
                    }

                    
                    
                    
                    
                    if ( pLast && pLast->IsFlowFrm() &&
                         ( !pLast->IsTxtFrm() ||
                           !static_cast<const SwTxtFrm*>(pLast)->GetFollow() ) )
                    {
                        nReal += SwFlowFrm::CastFlowFrm(pLast)->CalcAddLowerSpaceAsLastInTableCell();
                    }
                    
                    
                    
                    if ( pTmp->IsFlowFrm() &&
                         ( !pTmp->IsTxtFrm() ||
                           !static_cast<const SwTxtFrm*>(pTmp)->IsFollow() ) )
                    {
                        nTmpHeight += SwFlowFrm::CastFlowFrm(pTmp)->CalcUpperSpace( NULL, pLast);
                        nTmpHeight += SwFlowFrm::CastFlowFrm(pTmp)->CalcLowerSpace();
                    }
                    
                    
                    
                    
                    if ( pTmp->IsTxtFrm() &&
                         const_cast<SwTxtFrm*>(static_cast<const SwTxtFrm*>(pTmp))
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
                    
                    
                    SwBorderAttrAccess aAccess( SwFrm::GetCache(), pCurrSourceCell );
                    const SwBorderAttrs &rAttrs = *aAccess.Get();
                    nTmpHeight += rAttrs.CalcTop() + rAttrs.CalcBottom();
                    
                    
                    if ( pTmp->IsFlowFrm() )
                    {
                        nTmpHeight += SwFlowFrm::CastFlowFrm(pTmp)->CalcUpperSpace();
                        nTmpHeight += SwFlowFrm::CastFlowFrm(pTmp)->CalcLowerSpace();
                    }
                }
            }

            if ( bIsInFollowFlowLine )
            {
                
                if ( nTmpHeight < nHeight )
                    nHeight = nTmpHeight;
            }
            else
            {
                
                if ( nTmpHeight > nHeight && USHRT_MAX != nTmpHeight )
                    nHeight = nTmpHeight;
            }
        }

        pCurrSourceCell = (SwCellFrm*)pCurrSourceCell->GetNext();
    }

    return ( LONG_MAX == nHeight ) ? 0 : nHeight;
}


SwTwips SwTabFrm::CalcHeightOfFirstContentLine() const
{
    SWRECTFN( this )

    const bool bDontSplit = !IsFollow() && !GetFmt()->GetLayoutSplit().GetValue();

    if ( bDontSplit )
    {
        
        return (Frm().*fnRect->fnGetHeight)();
    }

    SwTwips nTmpHeight = 0;

    SwRowFrm* pFirstRow = GetFirstNonHeadlineRow();
    OSL_ENSURE( !IsFollow() || pFirstRow, "FollowTable without Lower" );

    
    if ( pFirstRow && pFirstRow->IsRowSpanLine() && pFirstRow->GetNext() )
        pFirstRow = static_cast<SwRowFrm*>(pFirstRow->GetNext());

    
    const sal_uInt16 nRepeat = GetTable()->GetRowsToRepeat();
    SwTwips nRepeatHeight = nRepeat ? lcl_GetHeightOfRows( GetLower(), nRepeat ) : 0;

    
    
    SwTwips nKeepHeight = nRepeatHeight;
    if ( GetFmt()->GetDoc()->get(IDocumentSettingAccess::TABLE_ROW_KEEP) )
    {
        sal_uInt16 nKeepRows = nRepeat;

        
        while ( pFirstRow && pFirstRow->ShouldRowKeepWithNext() )
        {
            ++nKeepRows;
            pFirstRow = static_cast<SwRowFrm*>(pFirstRow->GetNext());
        }

        if ( nKeepRows > nRepeat )
            nKeepHeight = lcl_GetHeightOfRows( GetLower(), nKeepRows );
    }

    
    
    
    if ( !IsFollow() )
    {
        nTmpHeight = nKeepHeight;
    }
    else
    {
        nTmpHeight = nKeepHeight - nRepeatHeight;
    }

    
    
    if ( pFirstRow )
    {
        const bool bSplittable = pFirstRow->IsRowSplitAllowed();
        const SwTwips nFirstLineHeight = (pFirstRow->Frm().*fnRect->fnGetHeight)();

        if ( !bSplittable )
        {
            
            
            
            
            
            if ( pFirstRow->GetPrev() &&
                 static_cast<SwRowFrm*>(pFirstRow->GetPrev())->IsRowSpanLine() )
            {
                
                SwTwips nMaxHeight = 0;
                const SwCellFrm* pLower2 = static_cast<const SwCellFrm*>(pFirstRow->Lower());
                while ( pLower2 )
                {
                    if ( 1 == pLower2->GetTabBox()->getRowSpan() )
                    {
                        const SwTwips nCellHeight = lcl_CalcMinCellHeight( pLower2, sal_True );
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

        
        
        
        
        else if ( 0 != nFirstLineHeight )
        {
            const bool bOldJoinLock = IsJoinLocked();
            ((SwTabFrm*)this)->LockJoin();
            const SwTwips nHeightOfFirstContentLine = lcl_CalcHeightOfFirstContentLine( *(SwRowFrm*)pFirstRow );

            
            const SwFmtFrmSize &rSz = static_cast<const SwRowFrm*>(pFirstRow)->GetFmt()->GetFrmSize();
            const SwTwips nMinRowHeight = rSz.GetHeightSizeType() == ATT_MIN_SIZE ?
                                          rSz.GetHeight() : 0;

            nTmpHeight += std::max( nHeightOfFirstContentLine, nMinRowHeight );

            if ( !bOldJoinLock )
                ((SwTabFrm*)this)->UnlockJoin();
        }
    }

    return nTmpHeight;
}




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
