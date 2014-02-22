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
#include "dflyobj.hxx"
#include "dcontact.hxx"
#include "ftnfrm.hxx"
#include "frmtool.hxx"
#include "hints.hxx"
#include "sectfrm.hxx"

#include <svx/svdpage.hxx>
#include <editeng/ulspitem.hxx>
#include <fmtornt.hxx>
#include <fmtfsize.hxx>
#include "ndole.hxx"
#include "tabfrm.hxx"
#include "flyfrms.hxx"

#include <fmtfollowtextflow.hxx>
#include <environmentofanchoredobject.hxx>

#include <sortedobjs.hxx>
#include <viewimp.hxx>

using namespace ::com::sun::star;

SwFlyFreeFrm::SwFlyFreeFrm( SwFlyFrmFmt *pFmt, SwFrm* pSib, SwFrm *pAnch ) :
    SwFlyFrm( pFmt, pSib, pAnch ),
    pPage( 0 ),
    
    mbNoMakePos( false ),
    
    mbNoMoveOnCheckClip( false ),
    maUnclippedFrm( )
{
}

SwFlyFreeFrm::~SwFlyFreeFrm()
{
    
    if( GetPageFrm() )
    {
        if( GetFmt()->GetDoc()->IsInDtor() )
        {
            
            
            const bool bRemoveFromPage =
                    GetPageFrm()->GetSortedObjs() &&
                    ( IsFlyAtCntFrm() ||
                      ( GetAnchorFrm() && GetAnchorFrm()->IsFlyFrm() ) );
            if ( bRemoveFromPage )
            {
                GetPageFrm()->GetSortedObjs()->Remove( *this );
            }
        }
        else
        {
            SwRect aTmp( GetObjRectWithSpaces() );
            SwFlyFreeFrm::NotifyBackground( GetPageFrm(), aTmp, PREP_FLY_LEAVE );
        }
    }
}


TYPEINIT1(SwFlyFreeFrm,SwFlyFrm);
/** Notifies the background (all CntntFrms that currently are overlapping).
 *
 * Additionally, the window is also directly invalidated (especially where
 * there are no overlapping CntntFrms).
 * This also takes CntntFrms within other Flys into account.
 */
void SwFlyFreeFrm::NotifyBackground( SwPageFrm *pPageFrm,
                                     const SwRect& rRect, PrepareHint eHint )
{
    ::Notify_Background( GetVirtDrawObj(), pPageFrm, rRect, eHint, sal_True );
}

void SwFlyFreeFrm::MakeAll()
{
    if ( !GetFmt()->GetDoc()->IsVisibleLayerId( GetVirtDrawObj()->GetLayer() ) )
    {
        return;
    }

    if ( !GetAnchorFrm() || IsLocked() || IsColLocked() )
        return;
    
    if( !GetPageFrm() && GetAnchorFrm() && GetAnchorFrm()->IsInFly() )
    {
        SwFlyFrm* pFly = AnchorFrm()->FindFlyFrm();
        SwPageFrm *pPageFrm = pFly ? pFly->FindPageFrm() : NULL;
        if( pPageFrm )
            pPageFrm->AppendFlyToPage( this );
    }
    if( !GetPageFrm() )
        return;

    Lock(); 

    
    const SwFlyNotify aNotify( this );

    if ( IsClipped() )
    {
        mbValidSize = bHeightClipped = bWidthClipped = sal_False;
        
        
        
        
        
        if ( !IsNoMoveOnCheckClip() &&
             !( PositionLocked() &&
                GetAnchorFrm()->IsInFly() &&
                GetFrmFmt().GetFollowTextFlow().GetValue() ) )
        {
            mbValidPos = sal_False;
        }
    }

    
    sal_uInt16 nLoopControlRuns = 0;
    const sal_uInt16 nLoopControlMax = 10;

    while ( !mbValidPos || !mbValidSize || !mbValidPrtArea || bFormatHeightOnly )
    {
        SWRECTFN( this )
        const SwFmtFrmSize *pSz;
        {   

            SwBorderAttrAccess aAccess( SwFrm::GetCache(), this );
            const SwBorderAttrs &rAttrs = *aAccess.Get();
            pSz = &rAttrs.GetAttrSet().GetFrmSize();

            
            if ( !mbValidSize )
            {
                mbValidPrtArea = sal_False;
            }

            if ( !mbValidPrtArea )
                MakePrtArea( rAttrs );

            if ( !mbValidSize || bFormatHeightOnly )
            {
                mbValidSize = sal_False;
                Format( &rAttrs );
                bFormatHeightOnly = sal_False;
            }

            if ( !mbValidPos )
            {
                const Point aOldPos( (Frm().*fnRect->fnGetPos)() );
                
                
                if ( IsNoMakePos() )
                    mbValidPos = sal_True;
                else
                    
                    MakeObjPos();
                if( aOldPos == (Frm().*fnRect->fnGetPos)() )
                {
                    if( !mbValidPos && GetAnchorFrm()->IsInSct() &&
                        !GetAnchorFrm()->FindSctFrm()->IsValid() )
                        mbValidPos = sal_True;
                }
                else
                    mbValidSize = sal_False;
            }
        }

        if ( mbValidPos && mbValidSize )
        {
            ++nLoopControlRuns;

            OSL_ENSURE( nLoopControlRuns < nLoopControlMax, "LoopControl in SwFlyFreeFrm::MakeAll" );

            if ( nLoopControlRuns < nLoopControlMax )
                CheckClip( *pSz );
        }
        else
            nLoopControlRuns = 0;
    }
    Unlock();

#if OSL_DEBUG_LEVEL > 0
    SWRECTFN( this )
    OSL_ENSURE( bHeightClipped || ( (Frm().*fnRect->fnGetHeight)() > 0 &&
            (Prt().*fnRect->fnGetHeight)() > 0),
            "SwFlyFreeFrm::Format(), flipping Fly." );

#endif
}

/** determines, if direct environment of fly frame has 'auto' size

    #i17297#
    start with anchor frame and search via <GetUpper()> for a header, footer,
    row or fly frame stopping at page frame.
    return <true>, if such a frame is found and it has 'auto' size.
    otherwise <false> is returned.

    @return boolean indicating, that direct environment has 'auto' size
*/
bool SwFlyFreeFrm::HasEnvironmentAutoSize() const
{
    bool bRetVal = false;

    const SwFrm* pToBeCheckedFrm = GetAnchorFrm();
    while ( pToBeCheckedFrm &&
            !pToBeCheckedFrm->IsPageFrm() )
    {
        if ( pToBeCheckedFrm->IsHeaderFrm() ||
             pToBeCheckedFrm->IsFooterFrm() ||
             pToBeCheckedFrm->IsRowFrm() ||
             pToBeCheckedFrm->IsFlyFrm() )
        {
            bRetVal = ATT_FIX_SIZE !=
                      pToBeCheckedFrm->GetAttrSet()->GetFrmSize().GetHeightSizeType();
            break;
        }
        else
        {
            pToBeCheckedFrm = pToBeCheckedFrm->GetUpper();
        }
    }

    return bRetVal;
}

void SwFlyFreeFrm::CheckClip( const SwFmtFrmSize &rSz )
{
    
    
    
    
    
    

    const SwVirtFlyDrawObj *pObj = GetVirtDrawObj();
    SwRect aClip, aTmpStretch;
    ::CalcClipRect( pObj, aClip, sal_True );
    ::CalcClipRect( pObj, aTmpStretch, sal_False );
    aClip._Intersection( aTmpStretch );

    const long nBot = Frm().Top() + Frm().Height();
    const long nRig = Frm().Left() + Frm().Width();
    const long nClipBot = aClip.Top() + aClip.Height();
    const long nClipRig = aClip.Left() + aClip.Width();

    const bool bBot = nBot > nClipBot;
    const bool bRig = nRig > nClipRig;
    if ( bBot || bRig )
    {
        bool bAgain = false;
        
        if ( bBot && !IsNoMoveOnCheckClip() &&
             !GetDrawObjs() && !GetAnchorFrm()->IsInTab() )
        {
            SwFrm* pHeader = FindFooterOrHeader();
            
            
            
            
            if ( !pHeader || !pHeader->IsHeaderFrm() )
            {
                const long nOld = Frm().Top();
                Frm().Pos().Y() = std::max( aClip.Top(), nClipBot - Frm().Height() );
                if ( Frm().Top() != nOld )
                    bAgain = true;
                bHeightClipped = sal_True;
            }
        }
        if ( bRig )
        {
            const long nOld = Frm().Left();
            Frm().Pos().X() = std::max( aClip.Left(), nClipRig - Frm().Width() );
            if ( Frm().Left() != nOld )
            {
                const SwFmtHoriOrient &rH = GetFmt()->GetHoriOrient();
                
                
                if( rH.GetHoriOrient() == text::HoriOrientation::LEFT )
                    Frm().Pos().X() = nOld;
                else
                    bAgain = true;
            }
            bWidthClipped = sal_True;
        }
        if ( bAgain )
            mbValidSize = sal_False;
        else
        {
            
            
            

            
            
            Size aOldSize( Frm().SSize() );

            
            SwRect aFrmRect( Frm() );

            if ( bBot )
            {
                long nDiff = nClipBot;
                nDiff -= aFrmRect.Top(); 
                nDiff = aFrmRect.Height() - nDiff;
                aFrmRect.Height( aFrmRect.Height() - nDiff );
                bHeightClipped = sal_True;
            }
            if ( bRig )
            {
                long nDiff = nClipRig;
                nDiff -= aFrmRect.Left();
                nDiff = aFrmRect.Width() - nDiff;
                aFrmRect.Width( aFrmRect.Width() - nDiff );
                bWidthClipped = sal_True;
            }

            
            
            
            
            
            
            
            
            
            
            if ( Lower() && Lower()->IsNoTxtFrm() &&
                 ( static_cast<SwCntntFrm*>(Lower())->GetNode()->GetOLENode() ||
                   !HasEnvironmentAutoSize() ) )
            {
                
                
                if ( aFrmRect.Width() != aOldSize.Width() &&
                     aFrmRect.Height()!= aOldSize.Height() )
                {
                    if ( (aOldSize.Width() - aFrmRect.Width()) >
                         (aOldSize.Height()- aFrmRect.Height()) )
                        aFrmRect.Height( aOldSize.Height() );
                    else
                        aFrmRect.Width( aOldSize.Width() );
                }

                
                if( aFrmRect.Width() != aOldSize.Width() )
                {
                    aFrmRect.Height( aFrmRect.Width() * aOldSize.Height() /
                                     aOldSize.Width() );
                    bHeightClipped = sal_True;
                }
                
                else if( aFrmRect.Height() != aOldSize.Height() )
                {
                    aFrmRect.Width( aFrmRect.Height() * aOldSize.Width() /
                                    aOldSize.Height() );
                    bWidthClipped = sal_True;
                }

                
                

                
                
                
                
                
                
                if ( aFrmRect.HasArea() &&
                     static_cast<SwCntntFrm*>(Lower())->GetNode()->GetOLENode() &&
                     ( bWidthClipped || bHeightClipped ) )
                {
                    SwFlyFrmFmt *pFmt = (SwFlyFrmFmt*)GetFmt();
                    pFmt->LockModify();
                    SwFmtFrmSize aFrmSize( rSz );
                    aFrmSize.SetWidth( aFrmRect.Width() );
                    aFrmSize.SetHeight( aFrmRect.Height() );
                    pFmt->SetFmtAttr( aFrmSize );
                    pFmt->UnlockModify();
                }
            }

            
            
            const long nPrtHeightDiff = Frm().Height() - Prt().Height();
            const long nPrtWidthDiff  = Frm().Width()  - Prt().Width();
            maUnclippedFrm = SwRect( Frm() );
            Frm().Height( aFrmRect.Height() );
            Frm().Width ( std::max( long(MINLAY), aFrmRect.Width() ) );
            if ( Lower() && Lower()->IsColumnFrm() )
            {
                ColLock();  
                const Size aTmpOldSize( Prt().SSize() );
                Prt().Height( Frm().Height() - nPrtHeightDiff );
                Prt().Width ( Frm().Width()  - nPrtWidthDiff );
                ChgLowersProp( aTmpOldSize );
                SwFrm *pLow = Lower();
                do
                {   pLow->Calc();
                    
                    ((SwLayoutFrm*)pLow)->Lower()->Calc();
                    pLow = pLow->GetNext();
                } while ( pLow );
                ::CalcCntnt( this );
                ColUnlock();
                if ( !mbValidSize && !bWidthClipped )
                    bFormatHeightOnly = mbValidSize = sal_True;
            }
            else
            {
                Prt().Height( Frm().Height() - nPrtHeightDiff );
                Prt().Width ( Frm().Width()  - nPrtWidthDiff );
            }
        }
    }

    
    OSL_ENSURE( Frm().Height() >= 0,
            "<SwFlyFreeFrm::CheckClip(..)> - fly frame has negative height now." );
}

/** method to determine, if a <MakeAll()> on the Writer fly frame is possible
    #i43771#
*/
bool SwFlyFreeFrm::IsFormatPossible() const
{
    return SwFlyFrm::IsFormatPossible() &&
           ( GetPageFrm() ||
             ( GetAnchorFrm() && GetAnchorFrm()->IsInFly() ) );
}

SwFlyLayFrm::SwFlyLayFrm( SwFlyFrmFmt *pFmt, SwFrm* pSib, SwFrm *pAnch ) :
    SwFlyFreeFrm( pFmt, pSib, pAnch )
{
    bLayout = sal_True;
}


TYPEINIT1(SwFlyLayFrm,SwFlyFreeFrm);

void SwFlyLayFrm::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew )
{
    sal_uInt16 nWhich = pNew ? pNew->Which() : 0;

    SwFmtAnchor *pAnch = 0;
    if( RES_ATTRSET_CHG == nWhich && SFX_ITEM_SET ==
        ((SwAttrSetChg*)pNew)->GetChgSet()->GetItemState( RES_ANCHOR, false,
            (const SfxPoolItem**)&pAnch ))
        ; 

    else if( RES_ANCHOR == nWhich )
    {
        
        
        
        pAnch = (SwFmtAnchor*)pNew;
    }

    if( pAnch )
    {
        OSL_ENSURE( pAnch->GetAnchorId() ==
                GetFmt()->GetAnchor().GetAnchorId(),
                "8-) Invalid change of anchor type." );

        
        SwRect aOld( GetObjRectWithSpaces() );
        
        SwPageFrm *pOldPage = GetPageFrm();
        AnchorFrm()->RemoveFly( this );

        if ( FLY_AT_PAGE == pAnch->GetAnchorId() )
        {
            sal_uInt16 nPgNum = pAnch->GetPageNum();
            SwRootFrm *pRoot = getRootFrm();
            SwPageFrm *pTmpPage = (SwPageFrm*)pRoot->Lower();
            for ( sal_uInt16 i = 1; (i <= nPgNum) && pTmpPage; ++i,
                                pTmpPage = (SwPageFrm*)pTmpPage->GetNext() )
            {
                if ( i == nPgNum )
                {
                    
                    pTmpPage->PlaceFly( this, 0 );
                }
            }
            if( !pTmpPage )
            {
                pRoot->SetAssertFlyPages();
                pRoot->AssertFlyPages();
            }
        }
        else
        {
            SwNodeIndex aIdx( pAnch->GetCntntAnchor()->nNode );
            SwCntntFrm *pCntnt = GetFmt()->GetDoc()->GetNodes().GoNext( &aIdx )->
                         GetCntntNode()->getLayoutFrm( getRootFrm(), 0, 0, sal_False );
            if( pCntnt )
            {
                SwFlyFrm *pTmp = pCntnt->FindFlyFrm();
                if( pTmp )
                    pTmp->AppendFly( this );
            }
        }
        
        if ( pOldPage && pOldPage != GetPageFrm() )
            NotifyBackground( pOldPage, aOld, PREP_FLY_LEAVE );
        SetCompletePaint();
        InvalidateAll();
        SetNotifyBack();
    }
    else
        SwFlyFrm::Modify( pOld, pNew );
}

void SwPageFrm::AppendFlyToPage( SwFlyFrm *pNew )
{
    if ( !pNew->GetVirtDrawObj()->IsInserted() )
        getRootFrm()->GetDrawPage()->InsertObject(
                (SdrObject*)pNew->GetVirtDrawObj(),
                pNew->GetVirtDrawObj()->GetReferencedObj().GetOrdNumDirect() );

    InvalidateSpelling();
    InvalidateSmartTags();  
    InvalidateAutoCompleteWords();
    InvalidateWordCount();

    if ( GetUpper() )
    {
        ((SwRootFrm*)GetUpper())->SetIdleFlags();
        ((SwRootFrm*)GetUpper())->InvalidateBrowseWidth();
    }

    SdrObject* pObj = pNew->GetVirtDrawObj();
    OSL_ENSURE( pNew->GetAnchorFrm(), "Fly without Anchor" );
    SwFlyFrm* pFly = (SwFlyFrm*)pNew->GetAnchorFrm()->FindFlyFrm();
    if ( pFly && pObj->GetOrdNum() < pFly->GetVirtDrawObj()->GetOrdNum() )
    {
        
        sal_uInt32 nNewNum = pObj->GetOrdNumDirect();
        if ( pObj->GetPage() )
            pObj->GetPage()->SetObjectOrdNum( pFly->GetVirtDrawObj()->GetOrdNumDirect(), nNewNum );
        else
            pFly->GetVirtDrawObj()->SetOrdNum( nNewNum );
    }

    
    if ( pNew->IsFlyInCntFrm() )
        InvalidateFlyInCnt();
    else
    {
        InvalidateFlyCntnt();

        if ( !pSortedObjs )
            pSortedObjs = new SwSortedObjs();

        const bool bSucessInserted = pSortedObjs->Insert( *pNew );
        OSL_ENSURE( bSucessInserted, "Fly not inserted in Sorted." );
        (void) bSucessInserted;

        
        OSL_ENSURE( pNew->GetPageFrm() == 0 || pNew->GetPageFrm() == this,
                "<SwPageFrm::AppendFlyToPage(..)> - anchored fly frame seems to be registered at another page frame. Serious defect -> please inform OD." );
        
        pNew->SetPageFrm( this );
        pNew->InvalidatePage( this );
        
        pNew->UnlockPosition();

        
        
        
        if( GetUpper() &&
            static_cast< SwRootFrm * >( GetUpper() )->IsAnyShellAccessible() &&
             static_cast< SwRootFrm * >( GetUpper() )->GetCurrShell() )
        {
            static_cast< SwRootFrm * >( GetUpper() )->GetCurrShell()->Imp()
                                      ->AddAccessibleFrm( pNew );
        }
    }

    
    if ( pNew->GetDrawObjs() )
    {
        SwSortedObjs &rObjs = *pNew->GetDrawObjs();
        for ( sal_uInt16 i = 0; i < rObjs.Count(); ++i )
        {
            SwAnchoredObject* pTmpObj = rObjs[i];
            if ( pTmpObj->ISA(SwFlyFrm) )
            {
                SwFlyFrm* pTmpFly = static_cast<SwFlyFrm*>(pTmpObj);
                
                if ( pTmpFly->IsFlyFreeFrm() && !pTmpFly->GetPageFrm() )
                    AppendFlyToPage( pTmpFly );
            }
            else if ( pTmpObj->ISA(SwAnchoredDrawObject) )
            {
                
                if ( pTmpObj->GetPageFrm() != this )
                {
                    if ( pTmpObj->GetPageFrm() != 0 )
                    {
                        pTmpObj->GetPageFrm()->RemoveDrawObjFromPage( *pTmpObj );
                    }
                    AppendDrawObjToPage( *pTmpObj );
                }
            }
        }
    }
}

void SwPageFrm::RemoveFlyFromPage( SwFlyFrm *pToRemove )
{
    const sal_uInt32 nOrdNum = pToRemove->GetVirtDrawObj()->GetOrdNum();
    getRootFrm()->GetDrawPage()->RemoveObject( nOrdNum );
    pToRemove->GetVirtDrawObj()->ReferencedObj().SetOrdNum( nOrdNum );

    if ( GetUpper() )
    {
        if ( !pToRemove->IsFlyInCntFrm() )
            ((SwRootFrm*)GetUpper())->SetSuperfluous();
        ((SwRootFrm*)GetUpper())->InvalidateBrowseWidth();
    }

    
    if ( pToRemove->IsFlyInCntFrm() )
        return;

    
    
    
    
    
    
    
    if (pSortedObjs)
    {
        pSortedObjs->Remove(*pToRemove);
        if (!pSortedObjs->Count())
        {
            delete pSortedObjs;
            pSortedObjs = 0;
        }
    }

    
    
    
    if( GetUpper() &&
        static_cast< SwRootFrm * >( GetUpper() )->IsAnyShellAccessible() &&
        static_cast< SwRootFrm * >( GetUpper() )->GetCurrShell() )
    {
        static_cast< SwRootFrm * >( GetUpper() )->GetCurrShell()->Imp()
                                  ->DisposeAccessibleFrm( pToRemove, sal_True );
    }

    
    pToRemove->SetPageFrm( 0L );
}

void SwPageFrm::MoveFly( SwFlyFrm *pToMove, SwPageFrm *pDest )
{
    
    if ( GetUpper() )
    {
        ((SwRootFrm*)GetUpper())->SetIdleFlags();
        if ( !pToMove->IsFlyInCntFrm() && pDest->GetPhyPageNum() < GetPhyPageNum() )
            ((SwRootFrm*)GetUpper())->SetSuperfluous();
    }

    pDest->InvalidateSpelling();
    pDest->InvalidateSmartTags();   
    pDest->InvalidateAutoCompleteWords();
    pDest->InvalidateWordCount();

    if ( pToMove->IsFlyInCntFrm() )
    {
        pDest->InvalidateFlyInCnt();
        return;
    }

    
    
    
    if( GetUpper() &&
        static_cast< SwRootFrm * >( GetUpper() )->IsAnyShellAccessible() &&
        static_cast< SwRootFrm * >( GetUpper() )->GetCurrShell() )
    {
        static_cast< SwRootFrm * >( GetUpper() )->GetCurrShell()->Imp()
                                  ->DisposeAccessibleFrm( pToMove, sal_True );
    }

    
    if ( pSortedObjs )
    {
        pSortedObjs->Remove( *pToMove );
        if ( !pSortedObjs->Count() )
        {   DELETEZ( pSortedObjs );
        }
    }

    
    if ( !pDest->GetSortedObjs() )
        pDest->pSortedObjs = new SwSortedObjs();

    const bool bSucessInserted = pDest->GetSortedObjs()->Insert( *pToMove );
    OSL_ENSURE( bSucessInserted, "Fly not inserted in Sorted." );
    (void) bSucessInserted;

    
    pToMove->SetPageFrm( pDest );
    pToMove->InvalidatePage( pDest );
    pToMove->SetNotifyBack();
    pDest->InvalidateFlyCntnt();
    
    pToMove->UnlockPosition();

    
    
    
    if( GetUpper() &&
        static_cast< SwRootFrm * >( GetUpper() )->IsAnyShellAccessible() &&
        static_cast< SwRootFrm * >( GetUpper() )->GetCurrShell() )
    {
        static_cast< SwRootFrm * >( GetUpper() )->GetCurrShell()->Imp()
                                  ->AddAccessibleFrm( pToMove );
    }

    
    if ( pToMove->GetDrawObjs() )
    {
        SwSortedObjs &rObjs = *pToMove->GetDrawObjs();
        for ( sal_uInt32 i = 0; i < rObjs.Count(); ++i )
        {
            SwAnchoredObject* pObj = rObjs[i];
            if ( pObj->ISA(SwFlyFrm) )
            {
                SwFlyFrm* pFly = static_cast<SwFlyFrm*>(pObj);
                if ( pFly->IsFlyFreeFrm() )
                {
                    
                    SwPageFrm* pPageFrm = pFly->GetPageFrm();
                    if ( pPageFrm )
                        pPageFrm->MoveFly( pFly, pDest );
                    else
                        pDest->AppendFlyToPage( pFly );
                }
            }
            else if ( pObj->ISA(SwAnchoredDrawObject) )
            {
                RemoveDrawObjFromPage( *pObj );
                pDest->AppendDrawObjToPage( *pObj );
            }
        }
    }
}

void SwPageFrm::AppendDrawObjToPage( SwAnchoredObject& _rNewObj )
{
    if ( !_rNewObj.ISA(SwAnchoredDrawObject) )
    {
        OSL_FAIL( "SwPageFrm::AppendDrawObjToPage(..) - anchored object of unexcepted type -> object not appended" );
        return;
    }

    if ( GetUpper() )
    {
        ((SwRootFrm*)GetUpper())->InvalidateBrowseWidth();
    }

    OSL_ENSURE( _rNewObj.GetAnchorFrm(), "anchored draw object without anchor" );
    SwFlyFrm* pFlyFrm = (SwFlyFrm*)_rNewObj.GetAnchorFrm()->FindFlyFrm();
    if ( pFlyFrm &&
         _rNewObj.GetDrawObj()->GetOrdNum() < pFlyFrm->GetVirtDrawObj()->GetOrdNum() )
    {
        
        sal_uInt32 nNewNum = _rNewObj.GetDrawObj()->GetOrdNumDirect();
        if ( _rNewObj.GetDrawObj()->GetPage() )
            _rNewObj.DrawObj()->GetPage()->SetObjectOrdNum( pFlyFrm->GetVirtDrawObj()->GetOrdNumDirect(), nNewNum );
        else
            pFlyFrm->GetVirtDrawObj()->SetOrdNum( nNewNum );
    }

    if ( FLY_AS_CHAR == _rNewObj.GetFrmFmt().GetAnchor().GetAnchorId() )
    {
        return;
    }

    if ( !pSortedObjs )
    {
        pSortedObjs = new SwSortedObjs();
    }
    if ( !pSortedObjs->Insert( _rNewObj ) )
    {
        OSL_ENSURE( pSortedObjs->Contains( _rNewObj ),
                "Drawing object not appended into list <pSortedObjs>." );
    }
    
    OSL_ENSURE( _rNewObj.GetPageFrm() == 0 || _rNewObj.GetPageFrm() == this,
            "<SwPageFrm::AppendDrawObjToPage(..)> - anchored draw object seems to be registered at another page frame. Serious defect -> please inform OD." );
    _rNewObj.SetPageFrm( this );

    
    InvalidateFlyLayout();
}

void SwPageFrm::RemoveDrawObjFromPage( SwAnchoredObject& _rToRemoveObj )
{
    if ( !_rToRemoveObj.ISA(SwAnchoredDrawObject) )
    {
        OSL_FAIL( "SwPageFrm::RemoveDrawObjFromPage(..) - anchored object of unexcepted type -> object not removed" );
        return;
    }

    if ( pSortedObjs )
    {
        pSortedObjs->Remove( _rToRemoveObj );
        if ( !pSortedObjs->Count() )
        {
            DELETEZ( pSortedObjs );
        }
        if ( GetUpper() )
        {
            if (FLY_AS_CHAR !=
                    _rToRemoveObj.GetFrmFmt().GetAnchor().GetAnchorId())
            {
                ((SwRootFrm*)GetUpper())->SetSuperfluous();
                InvalidatePage();
            }
            ((SwRootFrm*)GetUpper())->InvalidateBrowseWidth();
        }
    }
    _rToRemoveObj.SetPageFrm( 0 );
}


void SwPageFrm::PlaceFly( SwFlyFrm* pFly, SwFlyFrmFmt* pFmt )
{
    
    
    OSL_ENSURE( !IsEmptyPage() || GetNext(),
            "<SwPageFrm::PlaceFly(..)> - empty page with no next page! -> fly frame appended at empty page" );
    if ( IsEmptyPage() && GetNext() )
    {
        static_cast<SwPageFrm*>(GetNext())->PlaceFly( pFly, pFmt );
    }
    else
    {
        
        
        if ( pFly )
            AppendFly( pFly );
        else
        {   OSL_ENSURE( pFmt, ":-( No Format given for Fly." );
            pFly = new SwFlyLayFrm( (SwFlyFrmFmt*)pFmt, this, this );
            AppendFly( pFly );
            ::RegistFlys( this, pFly );
        }
    }
}











sal_Bool CalcClipRect( const SdrObject *pSdrObj, SwRect &rRect, sal_Bool bMove )
{
    sal_Bool bRet = sal_True;
    if ( pSdrObj->ISA(SwVirtFlyDrawObj) )
    {
        const SwFlyFrm* pFly = ((const SwVirtFlyDrawObj*)pSdrObj)->GetFlyFrm();
        const bool bFollowTextFlow = pFly->GetFmt()->GetFollowTextFlow().GetValue();
        
        const bool bConsiderWrapOnObjPos =
                                pFly->GetFmt()->getIDocumentSettingAccess()->get(IDocumentSettingAccess::CONSIDER_WRAP_ON_OBJECT_POSITION);
        const SwFmtVertOrient &rV = pFly->GetFmt()->GetVertOrient();
        if( pFly->IsFlyLayFrm() )
        {
            const SwFrm* pClip;
            
            
            if ( !bFollowTextFlow || bConsiderWrapOnObjPos )
            {
                pClip = pFly->GetAnchorFrm()->FindPageFrm();
            }
            else
            {
                pClip = pFly->GetAnchorFrm();
            }

            rRect = pClip->Frm();
            SWRECTFN( pClip )

            
            if( rV.GetVertOrient() != text::VertOrientation::NONE &&
                rV.GetRelationOrient() == text::RelOrientation::PRINT_AREA )
            {
                (rRect.*fnRect->fnSetTop)( (pClip->*fnRect->fnGetPrtTop)() );
                (rRect.*fnRect->fnSetBottom)( (pClip->*fnRect->fnGetPrtBottom)() );
            }
            
            const SwFmtHoriOrient &rH = pFly->GetFmt()->GetHoriOrient();
            if( rH.GetHoriOrient() != text::HoriOrientation::NONE &&
                rH.GetRelationOrient() == text::RelOrientation::PRINT_AREA )
            {
                (rRect.*fnRect->fnSetLeft)( (pClip->*fnRect->fnGetPrtLeft)() );
                (rRect.*fnRect->fnSetRight)((pClip->*fnRect->fnGetPrtRight)());
            }
        }
        else if( pFly->IsFlyAtCntFrm() )
        {
            
            
            const SwFrm* pVertPosOrientFrm = pFly->GetVertPosOrientFrm();
            if ( !pVertPosOrientFrm )
            {
                OSL_FAIL( "::CalcClipRect(..) - frame, vertical position is oriented at, is missing .");
                pVertPosOrientFrm = pFly->GetAnchorFrm();
            }

            if ( !bFollowTextFlow || bConsiderWrapOnObjPos )
            {
                const SwLayoutFrm* pClipFrm = pVertPosOrientFrm->FindPageFrm();
                if (!pClipFrm)
                {
                    OSL_FAIL("!pClipFrm: "
                            "if you can reproduce this please file a bug");
                    return false;
                }
                rRect = bMove ? pClipFrm->GetUpper()->Frm()
                              : pClipFrm->Frm();
                
                
                
                if ( pFly->GetAnchorFrm()->IsInTab() )
                {
                    const SwTabFrm* pTabFrm = const_cast<SwFlyFrm*>(pFly)
                                ->GetAnchorFrmContainingAnchPos()->FindTabFrm();
                    SwRect aTmp( pTabFrm->Prt() );
                    aTmp += pTabFrm->Frm().Pos();
                    rRect.Union( aTmp );
                    
                    const SwFrm* pCellFrm = const_cast<SwFlyFrm*>(pFly)
                                ->GetAnchorFrmContainingAnchPos()->GetUpper();
                    while ( pCellFrm && !pCellFrm->IsCellFrm() )
                    {
                        pCellFrm = pCellFrm->GetUpper();
                    }
                    if ( pCellFrm )
                    {
                        aTmp = pCellFrm->Prt();
                        aTmp += pCellFrm->Frm().Pos();
                        rRect.Union( aTmp );
                    }
                }
            }
            else if ( rV.GetRelationOrient() == text::RelOrientation::PAGE_FRAME ||
                      rV.GetRelationOrient() == text::RelOrientation::PAGE_PRINT_AREA )
            {
                
                objectpositioning::SwEnvironmentOfAnchoredObject
                                                aEnvOfObj( bFollowTextFlow );
                const SwLayoutFrm& rVertClipFrm =
                    aEnvOfObj.GetVertEnvironmentLayoutFrm( *pVertPosOrientFrm );
                if ( rV.GetRelationOrient() == text::RelOrientation::PAGE_FRAME )
                {
                    rRect = rVertClipFrm.Frm();
                }
                else if ( rV.GetRelationOrient() == text::RelOrientation::PAGE_PRINT_AREA )
                {
                    if ( rVertClipFrm.IsPageFrm() )
                    {
                        rRect = static_cast<const SwPageFrm&>(rVertClipFrm).PrtWithoutHeaderAndFooter();
                    }
                    else
                    {
                        rRect = rVertClipFrm.Frm();
                    }
                }
                const SwLayoutFrm* pHoriClipFrm =
                        pFly->GetAnchorFrm()->FindPageFrm()->GetUpper();
                SWRECTFN( pFly->GetAnchorFrm() )
                (rRect.*fnRect->fnSetLeft)( (pHoriClipFrm->Frm().*fnRect->fnGetLeft)() );
                (rRect.*fnRect->fnSetRight)((pHoriClipFrm->Frm().*fnRect->fnGetRight)());
            }
            else
            {
                
                const SwFrm *pClip =
                        const_cast<SwFlyFrm*>(pFly)->GetAnchorFrmContainingAnchPos();
                SWRECTFN( pClip )
                const SwLayoutFrm *pUp = pClip->GetUpper();
                const SwFrm *pCell = pUp->IsCellFrm() ? pUp : 0;
                sal_uInt16 nType = bMove ? FRM_ROOT   | FRM_FLY | FRM_HEADER |
                                       FRM_FOOTER | FRM_FTN
                                     : FRM_BODY   | FRM_FLY | FRM_HEADER |
                                       FRM_FOOTER | FRM_CELL| FRM_FTN;

                while ( !(pUp->GetType() & nType) || pUp->IsColBodyFrm() )
                {
                    pUp = pUp->GetUpper();
                    if ( !pCell && pUp->IsCellFrm() )
                        pCell = pUp;
                }
                if ( bMove )
                {
                    if ( pUp->IsRootFrm() )
                    {
                        rRect  = pUp->Prt();
                        rRect += pUp->Frm().Pos();
                        pUp = 0;
                    }
                }
                if ( pUp )
                {
                    if ( pUp->GetType() & FRM_BODY )
                    {
                        const SwPageFrm *pPg;
                        if ( pUp->GetUpper() != (pPg = pFly->FindPageFrm()) )
                            pUp = pPg->FindBodyCont();
                        rRect = pUp->GetUpper()->Frm();
                        (rRect.*fnRect->fnSetTop)( (pUp->*fnRect->fnGetPrtTop)() );
                        (rRect.*fnRect->fnSetBottom)((pUp->*fnRect->fnGetPrtBottom)());
                    }
                    else
                    {
                        if( ( pUp->GetType() & (FRM_FLY | FRM_FTN ) ) &&
                            !pUp->Frm().IsInside( pFly->Frm().Pos() ) )
                        {
                            if( pUp->IsFlyFrm() )
                            {
                                SwFlyFrm *pTmpFly = (SwFlyFrm*)pUp;
                                while( pTmpFly->GetNextLink() )
                                {
                                    pTmpFly = pTmpFly->GetNextLink();
                                    if( pTmpFly->Frm().IsInside( pFly->Frm().Pos() ) )
                                        break;
                                }
                                pUp = pTmpFly;
                            }
                            else if( pUp->IsInFtn() )
                            {
                                const SwFtnFrm *pTmp = pUp->FindFtnFrm();
                                while( pTmp->GetFollow() )
                                {
                                    pTmp = pTmp->GetFollow();
                                    if( pTmp->Frm().IsInside( pFly->Frm().Pos() ) )
                                        break;
                                }
                                pUp = pTmp;
                            }
                        }
                        rRect = pUp->Prt();
                        rRect.Pos() += pUp->Frm().Pos();
                        if ( pUp->GetType() & (FRM_HEADER | FRM_FOOTER) )
                        {
                            rRect.Left ( pUp->GetUpper()->Frm().Left() );
                            rRect.Width( pUp->GetUpper()->Frm().Width());
                        }
                        else if ( pUp->IsCellFrm() )                
                        {
                            const SwFrm *pTab = pUp->FindTabFrm();
                            (rRect.*fnRect->fnSetBottom)(
                                        (pTab->GetUpper()->*fnRect->fnGetPrtBottom)() );
                            
                            rRect.Left ( pUp->Frm().Left() );
                            rRect.Width( pUp->Frm().Width() );
                        }
                    }
                }
                if ( pCell )
                {
                    
                    
                    SwRect aTmp( pCell->Prt() );
                    aTmp += pCell->Frm().Pos();
                    rRect.Union( aTmp );
                }
            }
        }
        else
        {
            const SwFrm *pUp = pFly->GetAnchorFrm()->GetUpper();
            SWRECTFN( pFly->GetAnchorFrm() )
            while( pUp->IsColumnFrm() || pUp->IsSctFrm() || pUp->IsColBodyFrm())
                pUp = pUp->GetUpper();
            rRect = pUp->Frm();
            if( !pUp->IsBodyFrm() )
            {
                rRect += pUp->Prt().Pos();
                rRect.SSize( pUp->Prt().SSize() );
                if ( pUp->IsCellFrm() )
                {
                    const SwFrm *pTab = pUp->FindTabFrm();
                    (rRect.*fnRect->fnSetBottom)(
                                    (pTab->GetUpper()->*fnRect->fnGetPrtBottom)() );
                }
            }
            else if ( pUp->GetUpper()->IsPageFrm() )
            {
                
                
                (rRect.*fnRect->fnSetRight)( (pUp->GetUpper()->Frm().*fnRect->fnGetRight)() );
            }
            long nHeight = (9*(rRect.*fnRect->fnGetHeight)())/10;
            long nTop;
            const SwFmt *pFmt = ((SwContact*)GetUserCall(pSdrObj))->GetFmt();
            const SvxULSpaceItem &rUL = pFmt->GetULSpace();
            if( bMove )
            {
                nTop = bVert ? ((SwFlyInCntFrm*)pFly)->GetRefPoint().X() :
                               ((SwFlyInCntFrm*)pFly)->GetRefPoint().Y();
                nTop = (*fnRect->fnYInc)( nTop, -nHeight );
                long nWidth = (pFly->Frm().*fnRect->fnGetWidth)();
                (rRect.*fnRect->fnSetLeftAndWidth)( bVert ?
                            ((SwFlyInCntFrm*)pFly)->GetRefPoint().Y() :
                            ((SwFlyInCntFrm*)pFly)->GetRefPoint().X(), nWidth );
                nHeight = 2*nHeight - rUL.GetLower() - rUL.GetUpper();
            }
            else
            {
                nTop = (*fnRect->fnYInc)( (pFly->Frm().*fnRect->fnGetBottom)(),
                                           rUL.GetLower() - nHeight );
                nHeight = 2*nHeight - (pFly->Frm().*fnRect->fnGetHeight)()
                          - rUL.GetLower() - rUL.GetUpper();
            }
            (rRect.*fnRect->fnSetTopAndHeight)( nTop, nHeight );
        }
    }
    else
    {
        const SwDrawContact *pC = (const SwDrawContact*)GetUserCall(pSdrObj);
        const SwFrmFmt  *pFmt = (const SwFrmFmt*)pC->GetFmt();
        const SwFmtAnchor &rAnch = pFmt->GetAnchor();
        if ( FLY_AS_CHAR == rAnch.GetAnchorId() )
        {
            const SwFrm* pAnchorFrm = pC->GetAnchorFrm( pSdrObj );
            if( !pAnchorFrm )
            {
                OSL_FAIL( "<::CalcClipRect(..)> - missing anchor frame." );
                ((SwDrawContact*)pC)->ConnectToLayout();
                pAnchorFrm = pC->GetAnchorFrm();
            }
            const SwFrm* pUp = pAnchorFrm->GetUpper();
            rRect = pUp->Prt();
            rRect += pUp->Frm().Pos();
            SWRECTFN( pAnchorFrm )
            long nHeight = (9*(rRect.*fnRect->fnGetHeight)())/10;
            long nTop;
            const SvxULSpaceItem &rUL = pFmt->GetULSpace();
            SwRect aSnapRect( pSdrObj->GetSnapRect() );
            long nTmpH = 0;
            if( bMove )
            {
                nTop = (*fnRect->fnYInc)( bVert ? pSdrObj->GetAnchorPos().X() :
                                       pSdrObj->GetAnchorPos().Y(), -nHeight );
                long nWidth = (aSnapRect.*fnRect->fnGetWidth)();
                (rRect.*fnRect->fnSetLeftAndWidth)( bVert ?
                            pSdrObj->GetAnchorPos().Y() :
                            pSdrObj->GetAnchorPos().X(), nWidth );
            }
            else
            {
                
                
                nTmpH = bVert ? pSdrObj->GetCurrentBoundRect().GetWidth() :
                                pSdrObj->GetCurrentBoundRect().GetHeight();
                nTop = (*fnRect->fnYInc)( (aSnapRect.*fnRect->fnGetTop)(),
                                          rUL.GetLower() + nTmpH - nHeight );
            }
            nHeight = 2*nHeight - nTmpH - rUL.GetLower() - rUL.GetUpper();
            (rRect.*fnRect->fnSetTopAndHeight)( nTop, nHeight );
        }
        else
        {
            
            
            
            const SwFrm* pAnchorFrm = pC->GetAnchorFrm( pSdrObj );
            if ( pAnchorFrm && pAnchorFrm->FindFooterOrHeader() )
            {
                
                const SwFrm* pClipFrm = pAnchorFrm->FindPageFrm();
                rRect = pClipFrm->Frm();
            }
            else
            {
                bRet = sal_False;
            }
        }
    }
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
