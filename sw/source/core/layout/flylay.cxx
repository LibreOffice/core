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

#include "doc.hxx"
#include "pagefrm.hxx"
#include "rootfrm.hxx"
#include "cntfrm.hxx"
#include "dview.hxx"
#include "dflyobj.hxx"
#include "dcontact.hxx"
#include "flyfrm.hxx"
#include "ftnfrm.hxx"
#include "frmtool.hxx"
#include "frmfmt.hxx"
#include "hints.hxx"
#include "pam.hxx"
#include "sectfrm.hxx"


#include <svx/svdpage.hxx>
#include <editeng/ulspitem.hxx>
#include <fmtanchr.hxx>
#include <fmtornt.hxx>
#include <fmtfsize.hxx>
#include "ndole.hxx"
#include "tabfrm.hxx"
#include "flyfrms.hxx"
// #i18732#
#include <fmtfollowtextflow.hxx>
#include <environmentofanchoredobject.hxx>
// #i28701#
#include <sortedobjs.hxx>
#include <viewsh.hxx>
#include <viewimp.hxx>


using namespace ::com::sun::star;


/*************************************************************************
|*
|*  SwFlyFreeFrm::SwFlyFreeFrm(), ~SwFlyFreeFrm()
|*
|*************************************************************************/

SwFlyFreeFrm::SwFlyFreeFrm( SwFlyFrmFmt *pFmt, SwFrm* pSib, SwFrm *pAnch ) :
    SwFlyFrm( pFmt, pSib, pAnch ),
    pPage( 0 ),
    // #i34753#
    mbNoMakePos( false ),
    // #i37068#
    mbNoMoveOnCheckClip( false ),
    maUnclippedFrm( )
{
}

SwFlyFreeFrm::~SwFlyFreeFrm()
{
    // and goodbye.
    // #i28701# - use new method <GetPageFrm()>
    if( GetPageFrm() )
    {
        if( GetFmt()->GetDoc()->IsInDtor() )
        {
            // #i29879# - remove also to-frame anchored Writer
            // fly frame from page.
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

// #i28701#
TYPEINIT1(SwFlyFreeFrm,SwFlyFrm);
/*************************************************************************
|*
|*  SwFlyFreeFrm::NotifyBackground()
|*
|*  Description      notifies the background (all CntntFrms that currently
|*       are overlapping. Additionally, the window is also directly
|*       invalidated (especially where there are no overlapping CntntFrms)
|*       This also takes CntntFrms within other Flys into account.
|*
|*************************************************************************/

void SwFlyFreeFrm::NotifyBackground( SwPageFrm *pPageFrm,
                                     const SwRect& rRect, PrepareHint eHint )
{
    ::Notify_Background( GetVirtDrawObj(), pPageFrm, rRect, eHint, sal_True );
}

/*************************************************************************
|*
|*  SwFlyFreeFrm::MakeAll()
|*
|*************************************************************************/

void SwFlyFreeFrm::MakeAll()
{
    if ( !GetFmt()->GetDoc()->IsVisibleLayerId( GetVirtDrawObj()->GetLayer() ) )
    {
        return;
    }

    if ( !GetAnchorFrm() || IsLocked() || IsColLocked() )
        return;
    // #i28701# - use new method <GetPageFrm()>
    if( !GetPageFrm() && GetAnchorFrm() && GetAnchorFrm()->IsInFly() )
    {
        SwFlyFrm* pFly = AnchorFrm()->FindFlyFrm();
        SwPageFrm *pPageFrm = pFly ? pFly->FindPageFrm() : NULL;
        if( pPageFrm )
            pPageFrm->AppendFlyToPage( this );
    }
    if( !GetPageFrm() )
        return;

    Lock(); // The courtain drops

    // takes care of the notification in the dtor
    const SwFlyNotify aNotify( this );

    if ( IsClipped() )
    {
        bValidSize = bHeightClipped = bWidthClipped = sal_False;
        // no invalidation of position,
        // if anchored object is anchored inside a Writer fly frame,
        // its position is already locked, and it follows the text flow.
        // #i34753# - add condition:
        // no invalidation of position, if no direct move is requested in <CheckClip(..)>
        if ( !IsNoMoveOnCheckClip() &&
             !( PositionLocked() &&
                GetAnchorFrm()->IsInFly() &&
                GetFrmFmt().GetFollowTextFlow().GetValue() ) )
        {
            bValidPos = sal_False;
        }
    }

    // #i81146# new loop control
    sal_uInt16 nLoopControlRuns = 0;
    const sal_uInt16 nLoopControlMax = 10;

    while ( !bValidPos || !bValidSize || !bValidPrtArea || bFormatHeightOnly )
    {
        SWRECTFN( this )
        const SwFmtFrmSize *pSz;
        {   // Additional scope, so aAccess will be destroyed before the check!

            SwBorderAttrAccess aAccess( SwFrm::GetCache(), this );
            const SwBorderAttrs &rAttrs = *aAccess.Get();
            pSz = &rAttrs.GetAttrSet().GetFrmSize();

            // Only set when the flag is set!
            if ( !bValidSize )
            {
                bValidPrtArea = sal_False;
            }

            if ( !bValidPrtArea )
                MakePrtArea( rAttrs );

            if ( !bValidSize || bFormatHeightOnly )
            {
                bValidSize = sal_False;
                Format( &rAttrs );
                bFormatHeightOnly = sal_False;
            }

            if ( !bValidPos )
            {
                const Point aOldPos( (Frm().*fnRect->fnGetPos)() );
                // #i26791# - use new method <MakeObjPos()>
                // #i34753# - no positioning, if requested.
                if ( IsNoMakePos() )
                    bValidPos = sal_True;
                else
                    // #i26791# - use new method <MakeObjPos()>
                    MakeObjPos();
                if( aOldPos == (Frm().*fnRect->fnGetPos)() )
                {
                    if( !bValidPos && GetAnchorFrm()->IsInSct() &&
                        !GetAnchorFrm()->FindSctFrm()->IsValid() )
                        bValidPos = sal_True;
                }
                else
                    bValidSize = sal_False;
            }
        }

        if ( bValidPos && bValidSize )
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

/*************************************************************************
|*
|*  SwFlyFreeFrm::CheckClip()
|*
|*************************************************************************/

void SwFlyFreeFrm::CheckClip( const SwFmtFrmSize &rSz )
{
    // It's probably time now to take appropriate measures, if the Fly
    // doesn't fit into its surrounding.
    // First, the Fly gives up its position, then it's formatted.
    // Only if it still doesn't fit after giving up its position, the
    // width or height are given up as well. The frame will be squeezed
    // as much as needed.

    const SwVirtFlyDrawObj *pObj = GetVirtDrawObj();
    SwRect aClip, aTmpStretch;
    ::CalcClipRect( pObj, aClip, sal_True );
    ::CalcClipRect( pObj, aTmpStretch, sal_False );
    aClip._Intersection( aTmpStretch );

    const long nBot = Frm().Top() + Frm().Height();
    const long nRig = Frm().Left() + Frm().Width();
    const long nClipBot = aClip.Top() + aClip.Height();
    const long nClipRig = aClip.Left() + aClip.Width();

    const sal_Bool bBot = nBot > nClipBot;
    const sal_Bool bRig = nRig > nClipRig;
    if ( bBot || bRig )
    {
        sal_Bool bAgain = sal_False;
        // #i37068# - no move, if it's requested
        if ( bBot && !IsNoMoveOnCheckClip() &&
             !GetDrawObjs() && !GetAnchorFrm()->IsInTab() )
        {
            SwFrm* pHeader = FindFooterOrHeader();
            // In a header, correction of the position is no good idea.
            // If the fly moves, some paragraphs has to be formatted, this
            // could cause a change of the height of the headerframe,
            // now the flyframe can change its position and so on ...
            if ( !pHeader || !pHeader->IsHeaderFrm() )
            {
                const long nOld = Frm().Top();
                Frm().Pos().Y() = Max( aClip.Top(), nClipBot - Frm().Height() );
                if ( Frm().Top() != nOld )
                    bAgain = sal_True;
                bHeightClipped = sal_True;
            }
        }
        if ( bRig )
        {
            const long nOld = Frm().Left();
            Frm().Pos().X() = Max( aClip.Left(), nClipRig - Frm().Width() );
            if ( Frm().Left() != nOld )
            {
                const SwFmtHoriOrient &rH = GetFmt()->GetHoriOrient();
                // Left-aligned ones may not be moved to the left when they
                // are avoiding another one.
                if( rH.GetHoriOrient() == text::HoriOrientation::LEFT )
                    Frm().Pos().X() = nOld;
                else
                    bAgain = sal_True;
            }
            bWidthClipped = sal_True;
        }
        if ( bAgain )
            bValidSize = sal_False;
        else
        {
            // If we reach this branch, the Frm protrudes into forbidden
            // sections, and correcting the position is neither allowed
            // nor possible nor required.

            // For Flys with OLE objects as lower, we make sure that
            // we always resize proportionally
            Size aOldSize( Frm().SSize() );

            // First, setup the FrmRect, then transfer it to the Frm.
            SwRect aFrmRect( Frm() );

            if ( bBot )
            {
                long nDiff = nClipBot;
                nDiff -= aFrmRect.Top(); // nDiff represents the available distance
                nDiff = aFrmRect.Height() - nDiff;
                aFrmRect.Height( aFrmRect.Height() - nDiff );
                bHeightClipped = sal_True;
            }
            if ( bRig )
            {
                long nDiff = nClipRig;
                nDiff -= aFrmRect.Left();// nDiff represents the available distance
                nDiff = aFrmRect.Width() - nDiff;
                aFrmRect.Width( aFrmRect.Width() - nDiff );
                bWidthClipped = sal_True;
            }

            // #i17297# - no proportional
            // scaling of graphics in environments, which determines its size
            // by its content ('auto' size). Otherwise layout loops can occur and
            // layout sizes of the environment can be incorrect.
            // Such environment are:
            // (1) header and footer frames with 'auto' size
            // (2) table row frames with 'auto' size
            // (3) fly frames with 'auto' size
            // Note: section frames seems to be not critical - didn't found
            //       any critical layout situation so far.
            if ( Lower() && Lower()->IsNoTxtFrm() &&
                 ( static_cast<SwCntntFrm*>(Lower())->GetNode()->GetOLENode() ||
                   !HasEnvironmentAutoSize() ) )
            {
                // If width and height got adjusted, then the bigger
                // change is relevant.
                if ( aFrmRect.Width() != aOldSize.Width() &&
                     aFrmRect.Height()!= aOldSize.Height() )
                {
                    if ( (aOldSize.Width() - aFrmRect.Width()) >
                         (aOldSize.Height()- aFrmRect.Height()) )
                        aFrmRect.Height( aOldSize.Height() );
                    else
                        aFrmRect.Width( aOldSize.Width() );
                }

                // Adjusted the width? change height proportionally
                if( aFrmRect.Width() != aOldSize.Width() )
                {
                    aFrmRect.Height( aFrmRect.Width() * aOldSize.Height() /
                                     aOldSize.Width() );
                    bHeightClipped = sal_True;
                }
                // Adjusted the height? change width proportionally
                else if( aFrmRect.Height() != aOldSize.Height() )
                {
                    aFrmRect.Width( aFrmRect.Height() * aOldSize.Width() /
                                    aOldSize.Height() );
                    bWidthClipped = sal_True;
                }

                // #i17297# - reactivate change
                // of size attribute for fly frames containing an ole object.

                // Added the aFrmRect.HasArea() hack, because
                // the environment of the ole object does not have to be valid
                // at this moment, or even worse, it does not have to have a
                // resonable size. In this case we do not want to change to
                // attributes permanentely. Maybe one day somebody dares to remove
                // this code.
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

            // Now change the Frm; for columns, we put the new values into the attributes,
            // otherwise we'll end up with unwanted side-effects
            const long nPrtHeightDiff = Frm().Height() - Prt().Height();
            const long nPrtWidthDiff  = Frm().Width()  - Prt().Width();
            maUnclippedFrm = SwRect( Frm() );
            Frm().Height( aFrmRect.Height() );
            Frm().Width ( Max( long(MINLAY), aFrmRect.Width() ) );
            if ( Lower() && Lower()->IsColumnFrm() )
            {
                ColLock();  //lock grow/shrink
                const Size aTmpOldSize( Prt().SSize() );
                Prt().Height( Frm().Height() - nPrtHeightDiff );
                Prt().Width ( Frm().Width()  - nPrtWidthDiff );
                ChgLowersProp( aTmpOldSize );
                SwFrm *pLow = Lower();
                do
                {   pLow->Calc();
                    // also calculate the (Column)BodyFrm
                    ((SwLayoutFrm*)pLow)->Lower()->Calc();
                    pLow = pLow->GetNext();
                } while ( pLow );
                ::CalcCntnt( this );
                ColUnlock();
                if ( !bValidSize && !bWidthClipped )
                    bFormatHeightOnly = bValidSize = sal_True;
            }
            else
            {
                Prt().Height( Frm().Height() - nPrtHeightDiff );
                Prt().Width ( Frm().Width()  - nPrtWidthDiff );
            }
        }
    }

    // #i26945#
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

/*************************************************************************
|*
|*  SwFlyLayFrm::SwFlyLayFrm()
|*
|*************************************************************************/

SwFlyLayFrm::SwFlyLayFrm( SwFlyFrmFmt *pFmt, SwFrm* pSib, SwFrm *pAnch ) :
    SwFlyFreeFrm( pFmt, pSib, pAnch )
{
    bLayout = sal_True;
}

// #i28701#
TYPEINIT1(SwFlyLayFrm,SwFlyFreeFrm);
/*************************************************************************
|*
|*  SwFlyLayFrm::Modify()
|*
|*************************************************************************/

void SwFlyLayFrm::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew )
{
    sal_uInt16 nWhich = pNew ? pNew->Which() : 0;

    SwFmtAnchor *pAnch = 0;
    if( RES_ATTRSET_CHG == nWhich && SFX_ITEM_SET ==
        ((SwAttrSetChg*)pNew)->GetChgSet()->GetItemState( RES_ANCHOR, sal_False,
            (const SfxPoolItem**)&pAnch ))
        ; // GetItemState sets the anchor pointer!

    else if( RES_ANCHOR == nWhich )
    {
        // Change of anchor. I'm attaching myself to the new place.
        // It's not allowed to change the anchor type. This is only
        // possible via SwFEShell.
        pAnch = (SwFmtAnchor*)pNew;
    }

    if( pAnch )
    {
        OSL_ENSURE( pAnch->GetAnchorId() ==
                GetFmt()->GetAnchor().GetAnchorId(),
                "8-) Invalid change of anchor type." );

        // Unregister, get hold of the page, attach to the corresponding LayoutFrm.
        SwRect aOld( GetObjRectWithSpaces() );
        // #i28701# - use new method <GetPageFrm()>
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
                    // #i50432# - adjust synopsis of <PlaceFly(..)>
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
        // #i28701# - use new method <GetPageFrm()>
        if ( pOldPage && pOldPage != GetPageFrm() )
            NotifyBackground( pOldPage, aOld, PREP_FLY_LEAVE );
        SetCompletePaint();
        InvalidateAll();
        SetNotifyBack();
    }
    else
        SwFlyFrm::Modify( pOld, pNew );
}

/*************************************************************************
|*
|*  SwPageFrm::AppendFly()
|*
|*************************************************************************/

void SwPageFrm::AppendFlyToPage( SwFlyFrm *pNew )
{
    if ( !pNew->GetVirtDrawObj()->IsInserted() )
        getRootFrm()->GetDrawPage()->InsertObject(
                (SdrObject*)pNew->GetVirtDrawObj(),
                pNew->GetVirtDrawObj()->GetReferencedObj().GetOrdNumDirect() );

    InvalidateSpelling();
    InvalidateSmartTags();  // SMARTTAGS
    InvalidateAutoCompleteWords();
    InvalidateWordCount();

    if ( GetUpper() )
    {
        ((SwRootFrm*)GetUpper())->SetIdleFlags();
        ((SwRootFrm*)GetUpper())->InvalidateBrowseWidth();
    }

    SdrObject* pObj = pNew->GetVirtDrawObj();
    OSL_ENSURE( pNew->GetAnchorFrm(), "Fly without Anchor" );
    const SwFlyFrm* pFly = pNew->GetAnchorFrm()->FindFlyFrm();
    if ( pFly && pObj->GetOrdNum() < pFly->GetVirtDrawObj()->GetOrdNum() )
    {
        sal_uInt32 nNewNum = pFly->GetVirtDrawObj()->GetOrdNumDirect();
        if ( pObj->GetPage() )
            pObj->GetPage()->SetObjectOrdNum( pObj->GetOrdNumDirect(), nNewNum);
        else
            pObj->SetOrdNum( nNewNum );
    }

    // Don't look further at Flys that sit inside the Cntnt.
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

        // #i87493#
        OSL_ENSURE( pNew->GetPageFrm() == 0 || pNew->GetPageFrm() == this,
                "<SwPageFrm::AppendFlyToPage(..)> - anchored fly frame seems to be registered at another page frame. Serious defect -> please inform OD." );
        // #i28701# - use new method <SetPageFrm(..)>
        pNew->SetPageFrm( this );
        pNew->InvalidatePage( this );
        // #i28701#
        pNew->UnlockPosition();

        // Notify accessible layout. That's required at this place for
        // frames only where the anchor is moved. Creation of new frames
        // is additionally handled by the SwFrmNotify class.
        if( GetUpper() &&
            static_cast< SwRootFrm * >( GetUpper() )->IsAnyShellAccessible() &&
             static_cast< SwRootFrm * >( GetUpper() )->GetCurrShell() )
        {
            static_cast< SwRootFrm * >( GetUpper() )->GetCurrShell()->Imp()
                                      ->AddAccessibleFrm( pNew );
        }
    }

    // #i28701# - correction: consider also drawing objects
    if ( pNew->GetDrawObjs() )
    {
        SwSortedObjs &rObjs = *pNew->GetDrawObjs();
        for ( sal_uInt16 i = 0; i < rObjs.Count(); ++i )
        {
            SwAnchoredObject* pTmpObj = rObjs[i];
            if ( pTmpObj->ISA(SwFlyFrm) )
            {
                SwFlyFrm* pTmpFly = static_cast<SwFlyFrm*>(pTmpObj);
                // #i28701# - use new method <GetPageFrm()>
                if ( pTmpFly->IsFlyFreeFrm() && !pTmpFly->GetPageFrm() )
                    AppendFlyToPage( pTmpFly );
            }
            else if ( pTmpObj->ISA(SwAnchoredDrawObject) )
            {
                // #i87493#
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

/*************************************************************************
|*
|*  SwPageFrm::RemoveFly()
|*
|*************************************************************************/

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

    // Don't look further at Flys that sit inside the Cntnt.
    if ( pToRemove->IsFlyInCntFrm() )
        return;

    // Notify accessible layout. That's required at this place for
    // frames only where the anchor is moved. Creation of new frames
    // is additionally handled by the SwFrmNotify class.
    if( GetUpper() &&
        static_cast< SwRootFrm * >( GetUpper() )->IsAnyShellAccessible() &&
        static_cast< SwRootFrm * >( GetUpper() )->GetCurrShell() )
    {
        static_cast< SwRootFrm * >( GetUpper() )->GetCurrShell()->Imp()
                                  ->DisposeAccessibleFrm( pToRemove, sal_True );
    }

    // Don't delete collections just yet. This will happen at the end of the
    // action in the RemoveSuperfluous of the page, kicked off by a method of
    // the same name in the root.
    // The FlyColl might be gone already, because the page's dtor is being executed.
    if ( pSortedObjs )
    {
        pSortedObjs->Remove( *pToRemove );
        if ( !pSortedObjs->Count() )
        {   DELETEZ( pSortedObjs );
        }
    }
    // #i28701# - use new method <SetPageFrm(..)>
    pToRemove->SetPageFrm( 0L );
}

/*************************************************************************
|*
|*  SwPageFrm::MoveFly
|*
|*************************************************************************/

void SwPageFrm::MoveFly( SwFlyFrm *pToMove, SwPageFrm *pDest )
{
    // Invalidations
    if ( GetUpper() )
    {
        ((SwRootFrm*)GetUpper())->SetIdleFlags();
        if ( !pToMove->IsFlyInCntFrm() && pDest->GetPhyPageNum() < GetPhyPageNum() )
            ((SwRootFrm*)GetUpper())->SetSuperfluous();
    }

    pDest->InvalidateSpelling();
    pDest->InvalidateSmartTags();   // SMARTTAGS
    pDest->InvalidateAutoCompleteWords();
    pDest->InvalidateWordCount();

    if ( pToMove->IsFlyInCntFrm() )
    {
        pDest->InvalidateFlyInCnt();
        return;
    }

    // Notify accessible layout. That's required at this place for
    // frames only where the anchor is moved. Creation of new frames
    // is additionally handled by the SwFrmNotify class.
    if( GetUpper() &&
        static_cast< SwRootFrm * >( GetUpper() )->IsAnyShellAccessible() &&
        static_cast< SwRootFrm * >( GetUpper() )->GetCurrShell() )
    {
        static_cast< SwRootFrm * >( GetUpper() )->GetCurrShell()->Imp()
                                  ->DisposeAccessibleFrm( pToMove, sal_True );
    }

    // The FlyColl might be gone already, because the page's dtor is being executed.
    if ( pSortedObjs )
    {
        pSortedObjs->Remove( *pToMove );
        if ( !pSortedObjs->Count() )
        {   DELETEZ( pSortedObjs );
        }
    }

    // Register
    if ( !pDest->GetSortedObjs() )
        pDest->pSortedObjs = new SwSortedObjs();

    const bool bSucessInserted = pDest->GetSortedObjs()->Insert( *pToMove );
    OSL_ENSURE( bSucessInserted, "Fly not inserted in Sorted." );
    (void) bSucessInserted;

    // #i28701# - use new method <SetPageFrm(..)>
    pToMove->SetPageFrm( pDest );
    pToMove->InvalidatePage( pDest );
    pToMove->SetNotifyBack();
    pDest->InvalidateFlyCntnt();
    // #i28701#
    pToMove->UnlockPosition();

    // Notify accessible layout. That's required at this place for
    // frames only where the anchor is moved. Creation of new frames
    // is additionally handled by the SwFrmNotify class.
    if( GetUpper() &&
        static_cast< SwRootFrm * >( GetUpper() )->IsAnyShellAccessible() &&
        static_cast< SwRootFrm * >( GetUpper() )->GetCurrShell() )
    {
        static_cast< SwRootFrm * >( GetUpper() )->GetCurrShell()->Imp()
                                  ->AddAccessibleFrm( pToMove );
    }

    // #i28701# - correction: move lowers of Writer fly frame
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
                    // #i28701# - use new method <GetPageFrm()>
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

/*************************************************************************
|*
|*  SwPageFrm::AppendDrawObjToPage(), RemoveDrawObjFromPage()
|*
|*  #i28701# - new methods
|*
|*************************************************************************/
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
    const SwFlyFrm* pFlyFrm = _rNewObj.GetAnchorFrm()->FindFlyFrm();
    if ( pFlyFrm &&
         _rNewObj.GetDrawObj()->GetOrdNum() < pFlyFrm->GetVirtDrawObj()->GetOrdNum() )
    {
        sal_uInt32 nNewNum = pFlyFrm->GetVirtDrawObj()->GetOrdNumDirect();
        if ( _rNewObj.GetDrawObj()->GetPage() )
            _rNewObj.DrawObj()->GetPage()->SetObjectOrdNum(
                            _rNewObj.GetDrawObj()->GetOrdNumDirect(), nNewNum);
        else
            _rNewObj.DrawObj()->SetOrdNum( nNewNum );
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
    // #i87493#
    OSL_ENSURE( _rNewObj.GetPageFrm() == 0 || _rNewObj.GetPageFrm() == this,
            "<SwPageFrm::AppendDrawObjToPage(..)> - anchored draw object seems to be registered at another page frame. Serious defect -> please inform OD." );
    _rNewObj.SetPageFrm( this );

    // invalidate page in order to force a reformat of object layout of the page.
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

/*************************************************************************
|*
|*  SwPageFrm::PlaceFly
|*
|*************************************************************************/

// #i50432# - adjust method description and synopsis.
void SwPageFrm::PlaceFly( SwFlyFrm* pFly, SwFlyFrmFmt* pFmt )
{
    // #i50432# - consider the case that page is an empty page:
    // In this case append the fly frame at the next page
    OSL_ENSURE( !IsEmptyPage() || GetNext(),
            "<SwPageFrm::PlaceFly(..)> - empty page with no next page! -> fly frame appended at empty page" );
    if ( IsEmptyPage() && GetNext() )
    {
        static_cast<SwPageFrm*>(GetNext())->PlaceFly( pFly, pFmt );
    }
    else
    {
        // If we received a Fly, we use that one. Otherwise, create a new
        // one using the Format.
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

/*************************************************************************
|*
|*  ::CalcClipRect
|*
|*************************************************************************/
// #i18732# - adjustments for following text flow or not
// AND alignment at 'page areas' for to paragraph/to character anchored objects
// #i22305# - adjustment for following text flow for to frame anchored objects
// #i29778# - Because calculating the floating screen object's position
// (Writer fly frame or drawing object) doesn't perform a calculation on its
// upper frames and its anchor frame, a calculation of the upper frames in this
// method is no longer sensible.
// #i28701# - if document compatibility option 'Consider wrapping style influence
// on object positioning' is ON, the clip area corresponds to the one as the
// object doesn't follow the text flow.
sal_Bool CalcClipRect( const SdrObject *pSdrObj, SwRect &rRect, sal_Bool bMove )
{
    sal_Bool bRet = sal_True;
    if ( pSdrObj->ISA(SwVirtFlyDrawObj) )
    {
        const SwFlyFrm* pFly = ((const SwVirtFlyDrawObj*)pSdrObj)->GetFlyFrm();
        const bool bFollowTextFlow = pFly->GetFmt()->GetFollowTextFlow().GetValue();
        // #i28701#
        const bool bConsiderWrapOnObjPos =
                                pFly->GetFmt()->getIDocumentSettingAccess()->get(IDocumentSettingAccess::CONSIDER_WRAP_ON_OBJECT_POSITION);
        const SwFmtVertOrient &rV = pFly->GetFmt()->GetVertOrient();
        if( pFly->IsFlyLayFrm() )
        {
            const SwFrm* pClip;
            // #i22305#
            // #i28701#
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

            // vertical clipping: Top and Bottom, also to PrtArea if neccessary
            if( rV.GetVertOrient() != text::VertOrientation::NONE &&
                rV.GetRelationOrient() == text::RelOrientation::PRINT_AREA )
            {
                (rRect.*fnRect->fnSetTop)( (pClip->*fnRect->fnGetPrtTop)() );
                (rRect.*fnRect->fnSetBottom)( (pClip->*fnRect->fnGetPrtBottom)() );
            }
            // horizontal clipping: Top and Bottom, also to PrtArea if necessary
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
            // #i18732# - consider following text flow or not
            // AND alignment at 'page areas'
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
                // #i26945# - consider that a table, during
                // its format, can exceed its upper printing area bottom.
                // Thus, enlarge the clip rectangle, if such a case occurred
                if ( pFly->GetAnchorFrm()->IsInTab() )
                {
                    const SwTabFrm* pTabFrm = const_cast<SwFlyFrm*>(pFly)
                                ->GetAnchorFrmContainingAnchPos()->FindTabFrm();
                    SwRect aTmp( pTabFrm->Prt() );
                    aTmp += pTabFrm->Frm().Pos();
                    rRect.Union( aTmp );
                    // #i43913# - consider also the cell frame
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
                // new class <SwEnvironmentOfAnchoredObject>
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
                // #i26945#
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
                        else if ( pUp->IsCellFrm() )                //MA_FLY_HEIGHT
                        {
                            const SwFrm *pTab = pUp->FindTabFrm();
                            (rRect.*fnRect->fnSetBottom)(
                                        (pTab->GetUpper()->*fnRect->fnGetPrtBottom)() );
                            // expand to left and right cell border
                            rRect.Left ( pUp->Frm().Left() );
                            rRect.Width( pUp->Frm().Width() );
                        }
                    }
                }
                if ( pCell )
                {
                    // CellFrms might also sit in unallowed sections. In this case,
                    // the Fly is allowed to do so as well
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
                // Objects anchored as character may exceed right margin
                // of body frame:
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
                // #i26791# - value of <nTmpH> is needed to
                // calculate value of <nTop>.
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
            // restrict clip rectangle for drawing
            // objects in header/footer to the page frame.
            // #i26791#
            const SwFrm* pAnchorFrm = pC->GetAnchorFrm( pSdrObj );
            if ( pAnchorFrm && pAnchorFrm->FindFooterOrHeader() )
            {
                // clip frame is the page frame the header/footer is on.
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
