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
#include <IDocumentSettingAccess.hxx>
#include <IDocumentDrawModelAccess.hxx>

using namespace ::com::sun::star;

SwFlyFreeFrm::SwFlyFreeFrm( SwFlyFrameFormat *pFormat, SwFrm* pSib, SwFrm *pAnch ) :
    SwFlyFrm( pFormat, pSib, pAnch ),
    pPage( 0 ),
    // #i34753#
    mbNoMakePos( false ),
    // #i37068#
    mbNoMoveOnCheckClip( false ),
    maUnclippedFrm( )
{
}

void SwFlyFreeFrm::DestroyImpl()
{
    // #i28701# - use new method <GetPageFrm()>
    if( GetPageFrm() )
    {
        if( GetFormat()->GetDoc()->IsInDtor() )
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

    SwFlyFrm::DestroyImpl();
}

SwFlyFreeFrm::~SwFlyFreeFrm()
{
}

// #i28701#
TYPEINIT1(SwFlyFreeFrm,SwFlyFrm);
/** Notifies the background (all ContentFrms that currently are overlapping).
 *
 * Additionally, the window is also directly invalidated (especially where
 * there are no overlapping ContentFrms).
 * This also takes ContentFrms within other Flys into account.
 */
void SwFlyFreeFrm::NotifyBackground( SwPageFrm *pPageFrm,
                                     const SwRect& rRect, PrepareHint eHint )
{
    ::Notify_Background( GetVirtDrawObj(), pPageFrm, rRect, eHint, true );
}

void SwFlyFreeFrm::MakeAll(vcl::RenderContext* /*pRenderContext*/)
{
    if ( !GetFormat()->GetDoc()->getIDocumentDrawModelAccess().IsVisibleLayerId( GetVirtDrawObj()->GetLayer() ) )
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

    Lock(); // The curtain drops

    // takes care of the notification in the dtor
    const SwFlyNotify aNotify( this );

    if ( IsClipped() )
    {
        mbValidSize = m_bHeightClipped = m_bWidthClipped = false;
        // no invalidation of position,
        // if anchored object is anchored inside a Writer fly frame,
        // its position is already locked, and it follows the text flow.
        // #i34753# - add condition:
        // no invalidation of position, if no direct move is requested in <CheckClip(..)>
        if ( !IsNoMoveOnCheckClip() &&
             !( PositionLocked() &&
                GetAnchorFrm()->IsInFly() &&
                GetFrameFormat().GetFollowTextFlow().GetValue() ) )
        {
            mbValidPos = false;
        }
    }

    // #i81146# new loop control
    int nLoopControlRuns = 0;
    const int nLoopControlMax = 10;

    while ( !mbValidPos || !mbValidSize || !mbValidPrtArea || m_bFormatHeightOnly || !m_bValidContentPos )
    {
        SWRECTFN( this )
        const SwFormatFrmSize *pSz;
        {   // Additional scope, so aAccess will be destroyed before the check!

            SwBorderAttrAccess aAccess( SwFrm::GetCache(), this );
            const SwBorderAttrs &rAttrs = *aAccess.Get();
            pSz = &rAttrs.GetAttrSet().GetFrmSize();

            // Only set when the flag is set!
            if ( !mbValidSize )
            {
                mbValidPrtArea = false;
            }

            if ( !mbValidPrtArea )
            {
                MakePrtArea( rAttrs );
                m_bValidContentPos = false;
            }

            if ( !mbValidSize || m_bFormatHeightOnly )
            {
                mbValidSize = false;
                Format( getRootFrm()->GetCurrShell()->GetOut(), &rAttrs );
                m_bFormatHeightOnly = false;
            }
        }

        if ( !mbValidPos )
        {
            const Point aOldPos( (Frm().*fnRect->fnGetPos)() );
            // #i26791# - use new method <MakeObjPos()>
            // #i34753# - no positioning, if requested.
            if ( IsNoMakePos() )
                mbValidPos = true;
            else
                // #i26791# - use new method <MakeObjPos()>
                MakeObjPos();
            if( aOldPos == (Frm().*fnRect->fnGetPos)() )
            {
                if( !mbValidPos && GetAnchorFrm()->IsInSct() &&
                    !GetAnchorFrm()->FindSctFrm()->IsValid() )
                    mbValidPos = true;
            }
            else
                mbValidSize = false;
        }

        if ( !m_bValidContentPos )
        {
            SwBorderAttrAccess aAccess( SwFrm::GetCache(), this );
            const SwBorderAttrs &rAttrs = *aAccess.Get();
            MakeContentPos( rAttrs );
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
    OSL_ENSURE( m_bHeightClipped || ( (Frm().*fnRect->fnGetHeight)() > 0 &&
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

void SwFlyFreeFrm::CheckClip( const SwFormatFrmSize &rSz )
{
    // It's probably time now to take appropriate measures, if the Fly
    // doesn't fit into its surrounding.
    // First, the Fly gives up its position, then it's formatted.
    // Only if it still doesn't fit after giving up its position, the
    // width or height are given up as well. The frame will be squeezed
    // as much as needed.

    const SwVirtFlyDrawObj *pObj = GetVirtDrawObj();
    SwRect aClip, aTmpStretch;
    ::CalcClipRect( pObj, aClip );
    ::CalcClipRect( pObj, aTmpStretch, false );
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
        // #i37068# - no move, if it's requested
        if ( bBot && !IsNoMoveOnCheckClip() &&
             !GetDrawObjs() && !GetAnchorFrm()->IsInTab() )
        {
            SwFrm* pHeader = FindFooterOrHeader();
            // In a header, correction of the position is no good idea.
            // If the fly moves, some paragraphs have to be formatted, this
            // could cause a change of the height of the headerframe,
            // now the flyframe can change its position and so on ...
            if ( !pHeader || !pHeader->IsHeaderFrm() )
            {
                const long nOld = Frm().Top();
                Frm().Pos().Y() = std::max( aClip.Top(), nClipBot - Frm().Height() );
                if ( Frm().Top() != nOld )
                    bAgain = true;
                m_bHeightClipped = true;
            }
        }
        if ( bRig )
        {
            const long nOld = Frm().Left();
            Frm().Pos().X() = std::max( aClip.Left(), nClipRig - Frm().Width() );
            if ( Frm().Left() != nOld )
            {
                const SwFormatHoriOrient &rH = GetFormat()->GetHoriOrient();
                // Left-aligned ones may not be moved to the left when they
                // are avoiding another one.
                if( rH.GetHoriOrient() == text::HoriOrientation::LEFT )
                    Frm().Pos().X() = nOld;
                else
                    bAgain = true;
            }
            m_bWidthClipped = true;
        }
        if ( bAgain )
            mbValidSize = false;
        else
        {
            // If we reach this branch, the Frm protrudes into forbidden
            // areas, and correcting the position is not allowed or not
            // possible or not required.

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
                m_bHeightClipped = true;
            }
            if ( bRig )
            {
                long nDiff = nClipRig;
                nDiff -= aFrmRect.Left();// nDiff represents the available distance
                nDiff = aFrmRect.Width() - nDiff;
                aFrmRect.Width( aFrmRect.Width() - nDiff );
                m_bWidthClipped = true;
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
            if ( Lower() && Lower()->IsNoTextFrm() &&
                 ( static_cast<SwContentFrm*>(Lower())->GetNode()->GetOLENode() ||
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
                    m_bHeightClipped = true;
                }
                // Adjusted the height? change width proportionally
                else if( aFrmRect.Height() != aOldSize.Height() )
                {
                    aFrmRect.Width( aFrmRect.Height() * aOldSize.Width() /
                                    aOldSize.Height() );
                    m_bWidthClipped = true;
                }

                // #i17297# - reactivate change
                // of size attribute for fly frames containing an ole object.

                // Added the aFrmRect.HasArea() hack, because
                // the environment of the ole object does not have to be valid
                // at this moment, or even worse, it does not have to have a
                // reasonable size. In this case we do not want to change to
                // attributes permanentely. Maybe one day somebody dares to remove
                // this code.
                if ( aFrmRect.HasArea() &&
                     static_cast<SwContentFrm*>(Lower())->GetNode()->GetOLENode() &&
                     ( m_bWidthClipped || m_bHeightClipped ) )
                {
                    SwFlyFrameFormat *pFormat = GetFormat();
                    pFormat->LockModify();
                    SwFormatFrmSize aFrmSize( rSz );
                    aFrmSize.SetWidth( aFrmRect.Width() );
                    aFrmSize.SetHeight( aFrmRect.Height() );
                    pFormat->SetFormatAttr( aFrmSize );
                    pFormat->UnlockModify();
                }
            }

            // Now change the Frm; for columns, we put the new values into the attributes,
            // otherwise we'll end up with unwanted side-effects/oscillations
            const long nPrtHeightDiff = Frm().Height() - Prt().Height();
            const long nPrtWidthDiff  = Frm().Width()  - Prt().Width();
            maUnclippedFrm = SwRect( Frm() );
            Frm().Height( aFrmRect.Height() );
            Frm().Width ( std::max( long(MINLAY), aFrmRect.Width() ) );
            if ( Lower() && Lower()->IsColumnFrm() )
            {
                ColLock();  //lock grow/shrink
                const Size aTmpOldSize( Prt().SSize() );
                Prt().Height( Frm().Height() - nPrtHeightDiff );
                Prt().Width ( Frm().Width()  - nPrtWidthDiff );
                ChgLowersProp( aTmpOldSize );
                SwFrm *pLow = Lower();
                do
                {
                    pLow->Calc(getRootFrm()->GetCurrShell()->GetOut());
                    // also calculate the (Column)BodyFrm
                    static_cast<SwLayoutFrm*>(pLow)->Lower()->Calc(getRootFrm()->GetCurrShell()->GetOut());
                    pLow = pLow->GetNext();
                } while ( pLow );
                ::CalcContent( this );
                ColUnlock();
                if ( !mbValidSize && !m_bWidthClipped )
                    m_bFormatHeightOnly = mbValidSize = true;
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

SwFlyLayFrm::SwFlyLayFrm( SwFlyFrameFormat *pFormat, SwFrm* pSib, SwFrm *pAnch ) :
    SwFlyFreeFrm( pFormat, pSib, pAnch )
{
    m_bLayout = true;
}

// #i28701#
TYPEINIT1(SwFlyLayFrm,SwFlyFreeFrm);

void SwFlyLayFrm::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew )
{
    const sal_uInt16 nWhich = pNew ? pNew->Which() : 0;

    const SwFormatAnchor *pAnch = 0;
    if( RES_ATTRSET_CHG == nWhich && SfxItemState::SET ==
        static_cast<const SwAttrSetChg*>(pNew)->GetChgSet()->GetItemState( RES_ANCHOR, false,
            reinterpret_cast<const SfxPoolItem**>(&pAnch) ))
        ; // GetItemState sets the anchor pointer!

    else if( RES_ANCHOR == nWhich )
    {
        // Change of anchor. I'm attaching myself to the new place.
        // It's not allowed to change the anchor type. This is only
        // possible via SwFEShell.
        pAnch = static_cast<const SwFormatAnchor*>(pNew);
    }

    if( pAnch )
    {
        OSL_ENSURE( pAnch->GetAnchorId() ==
                GetFormat()->GetAnchor().GetAnchorId(),
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
            SwPageFrm *pTmpPage = static_cast<SwPageFrm*>(pRoot->Lower());
            for ( sal_uInt16 i = 1; (i <= nPgNum) && pTmpPage; ++i,
                                pTmpPage = static_cast<SwPageFrm*>(pTmpPage->GetNext()) )
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
            SwNodeIndex aIdx( pAnch->GetContentAnchor()->nNode );
            SwContentFrm *pContent = GetFormat()->GetDoc()->GetNodes().GoNext( &aIdx )->
                         GetContentNode()->getLayoutFrm( getRootFrm(), 0, 0, false );
            if( pContent )
            {
                SwFlyFrm *pTmp = pContent->FindFlyFrm();
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

void SwPageFrm::AppendFlyToPage( SwFlyFrm *pNew )
{
    if ( !pNew->GetVirtDrawObj()->IsInserted() )
        getRootFrm()->GetDrawPage()->InsertObject(
                static_cast<SdrObject*>(pNew->GetVirtDrawObj()),
                pNew->GetVirtDrawObj()->GetReferencedObj().GetOrdNumDirect() );

    InvalidateSpelling();
    InvalidateSmartTags();
    InvalidateAutoCompleteWords();
    InvalidateWordCount();

    if ( GetUpper() )
    {
        static_cast<SwRootFrm*>(GetUpper())->SetIdleFlags();
        static_cast<SwRootFrm*>(GetUpper())->InvalidateBrowseWidth();
    }

    SdrObject* pObj = pNew->GetVirtDrawObj();
    OSL_ENSURE( pNew->GetAnchorFrm(), "Fly without Anchor" );
    SwFlyFrm* pFly = const_cast<SwFlyFrm*>(pNew->GetAnchorFrm()->FindFlyFrm());
    if ( pFly && pObj->GetOrdNum() < pFly->GetVirtDrawObj()->GetOrdNum() )
    {
        //#i119945# set pFly's OrdNum to _rNewObj's. So when pFly is removed by Undo, the original OrdNum will not be changed.
        sal_uInt32 nNewNum = pObj->GetOrdNumDirect();
        if ( pObj->GetPage() )
            pObj->GetPage()->SetObjectOrdNum( pFly->GetVirtDrawObj()->GetOrdNumDirect(), nNewNum );
        else
            pFly->GetVirtDrawObj()->SetOrdNum( nNewNum );
    }

    // Don't look further at Flys that sit inside the Content.
    if ( pNew->IsFlyInCntFrm() )
        InvalidateFlyInCnt();
    else
    {
        InvalidateFlyContent();

        if ( !pSortedObjs )
            pSortedObjs = new SwSortedObjs();

        const bool bSucessInserted = pSortedObjs->Insert( *pNew );
        OSL_ENSURE( bSucessInserted, "Fly not inserted in Sorted." );
        (void) bSucessInserted;

        // #i87493#
        OSL_ENSURE( pNew->GetPageFrm() == 0 || pNew->GetPageFrm() == this,
                "<SwPageFrm::AppendFlyToPage(..)> - anchored fly frame seems to be registered at another page frame. Serious defect." );
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
        for ( size_t i = 0; i < rObjs.size(); ++i )
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

void SwPageFrm::RemoveFlyFromPage( SwFlyFrm *pToRemove )
{
    const sal_uInt32 nOrdNum = pToRemove->GetVirtDrawObj()->GetOrdNum();
    getRootFrm()->GetDrawPage()->RemoveObject( nOrdNum );
    pToRemove->GetVirtDrawObj()->ReferencedObj().SetOrdNum( nOrdNum );

    if ( GetUpper() )
    {
        if ( !pToRemove->IsFlyInCntFrm() )
            static_cast<SwRootFrm*>(GetUpper())->SetSuperfluous();
        static_cast<SwRootFrm*>(GetUpper())->InvalidateBrowseWidth();
    }

    // Don't look further at Flys that sit inside the Content.
    if ( pToRemove->IsFlyInCntFrm() )
        return;

    // Don't delete collections just yet. This will happen at the end of the
    // action in the RemoveSuperfluous of the page, kicked off by a method of
    // the same name in the root.
    // The FlyColl might be gone already, because the page's dtor is being
    // executed.
    // Remove it _before_ disposing accessible frames to avoid accesses to
    // the Frm from event handlers.
    if (pSortedObjs)
    {
        pSortedObjs->Remove(*pToRemove);
        if (!pSortedObjs->size())
        {
            delete pSortedObjs;
            pSortedObjs = 0;
        }
    }

    // Notify accessible layout. That's required at this place for
    // frames only where the anchor is moved. Creation of new frames
    // is additionally handled by the SwFrmNotify class.
    if( GetUpper() &&
        static_cast< SwRootFrm * >( GetUpper() )->IsAnyShellAccessible() &&
        static_cast< SwRootFrm * >( GetUpper() )->GetCurrShell() )
    {
        static_cast< SwRootFrm * >( GetUpper() )->GetCurrShell()->Imp()
                                  ->DisposeAccessibleFrm( pToRemove, true );
    }

    // #i28701# - use new method <SetPageFrm(..)>
    pToRemove->SetPageFrm( 0L );
}

void SwPageFrm::MoveFly( SwFlyFrm *pToMove, SwPageFrm *pDest )
{
    // Invalidations
    if ( GetUpper() )
    {
        static_cast<SwRootFrm*>(GetUpper())->SetIdleFlags();
        if ( !pToMove->IsFlyInCntFrm() && pDest->GetPhyPageNum() < GetPhyPageNum() )
            static_cast<SwRootFrm*>(GetUpper())->SetSuperfluous();
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

    // Notify accessible layout. That's required at this place for
    // frames only where the anchor is moved. Creation of new frames
    // is additionally handled by the SwFrmNotify class.
    if( GetUpper() &&
        static_cast< SwRootFrm * >( GetUpper() )->IsAnyShellAccessible() &&
        static_cast< SwRootFrm * >( GetUpper() )->GetCurrShell() )
    {
        static_cast< SwRootFrm * >( GetUpper() )->GetCurrShell()->Imp()
                                  ->DisposeAccessibleFrm( pToMove, true );
    }

    // The FlyColl might be gone already, because the page's dtor is being executed.
    if ( pSortedObjs )
    {
        pSortedObjs->Remove( *pToMove );
        if ( !pSortedObjs->size() )
        {
            DELETEZ( pSortedObjs );
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
    pDest->InvalidateFlyContent();
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
        for ( size_t i = 0; i < rObjs.size(); ++i )
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

void SwPageFrm::AppendDrawObjToPage( SwAnchoredObject& _rNewObj )
{
    if ( !_rNewObj.ISA(SwAnchoredDrawObject) )
    {
        OSL_FAIL( "SwPageFrm::AppendDrawObjToPage(..) - anchored object of unexpected type -> object not appended" );
        return;
    }

    if ( GetUpper() )
    {
        static_cast<SwRootFrm*>(GetUpper())->InvalidateBrowseWidth();
    }

    OSL_ENSURE( _rNewObj.GetAnchorFrm(), "anchored draw object without anchor" );
    SwFlyFrm* pFlyFrm = const_cast<SwFlyFrm*>(_rNewObj.GetAnchorFrm()->FindFlyFrm());
    if ( pFlyFrm &&
         _rNewObj.GetDrawObj()->GetOrdNum() < pFlyFrm->GetVirtDrawObj()->GetOrdNum() )
    {
        //#i119945# set pFly's OrdNum to _rNewObj's. So when pFly is removed by Undo, the original OrdNum will not be changed.
        sal_uInt32 nNewNum = _rNewObj.GetDrawObj()->GetOrdNumDirect();
        if ( _rNewObj.GetDrawObj()->GetPage() )
            _rNewObj.DrawObj()->GetPage()->SetObjectOrdNum( pFlyFrm->GetVirtDrawObj()->GetOrdNumDirect(), nNewNum );
        else
            pFlyFrm->GetVirtDrawObj()->SetOrdNum( nNewNum );
    }

    if ( FLY_AS_CHAR == _rNewObj.GetFrameFormat().GetAnchor().GetAnchorId() )
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
            "<SwPageFrm::AppendDrawObjToPage(..)> - anchored draw object seems to be registered at another page frame. Serious defect." );
    _rNewObj.SetPageFrm( this );

    // invalidate page in order to force a reformat of object layout of the page.
    InvalidateFlyLayout();
}

void SwPageFrm::RemoveDrawObjFromPage( SwAnchoredObject& _rToRemoveObj )
{
    if ( !_rToRemoveObj.ISA(SwAnchoredDrawObject) )
    {
        OSL_FAIL( "SwPageFrm::RemoveDrawObjFromPage(..) - anchored object of unexpected type -> object not removed" );
        return;
    }

    if ( pSortedObjs )
    {
        pSortedObjs->Remove( _rToRemoveObj );
        if ( !pSortedObjs->size() )
        {
            DELETEZ( pSortedObjs );
        }
        if ( GetUpper() )
        {
            if (FLY_AS_CHAR !=
                    _rToRemoveObj.GetFrameFormat().GetAnchor().GetAnchorId())
            {
                static_cast<SwRootFrm*>(GetUpper())->SetSuperfluous();
                InvalidatePage();
            }
            static_cast<SwRootFrm*>(GetUpper())->InvalidateBrowseWidth();
        }
    }
    _rToRemoveObj.SetPageFrm( 0 );
}

// #i50432# - adjust method description and synopsis.
void SwPageFrm::PlaceFly( SwFlyFrm* pFly, SwFlyFrameFormat* pFormat )
{
    // #i50432# - consider the case that page is an empty page:
    // In this case append the fly frame at the next page
    OSL_ENSURE( !IsEmptyPage() || GetNext(),
            "<SwPageFrm::PlaceFly(..)> - empty page with no next page! -> fly frame appended at empty page" );
    if ( IsEmptyPage() && GetNext() )
    {
        static_cast<SwPageFrm*>(GetNext())->PlaceFly( pFly, pFormat );
    }
    else
    {
        // If we received a Fly, we use that one. Otherwise, create a new
        // one using the Format.
        if ( pFly )
            AppendFly( pFly );
        else
        {
            OSL_ENSURE( pFormat, ":-( No Format given for Fly." );
            pFly = new SwFlyLayFrm( pFormat, this, this );
            AppendFly( pFly );
            ::RegistFlys( this, pFly );
        }
    }
}

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
bool CalcClipRect( const SdrObject *pSdrObj, SwRect &rRect, bool bMove )
{
    bool bRet = true;
    if ( pSdrObj->ISA(SwVirtFlyDrawObj) )
    {
        const SwFlyFrm* pFly = static_cast<const SwVirtFlyDrawObj*>(pSdrObj)->GetFlyFrm();
        const bool bFollowTextFlow = pFly->GetFormat()->GetFollowTextFlow().GetValue();
        // #i28701#
        const bool bConsiderWrapOnObjPos =
                                pFly->GetFormat()->getIDocumentSettingAccess().get(DocumentSettingId::CONSIDER_WRAP_ON_OBJECT_POSITION);
        const SwFormatVertOrient &rV = pFly->GetFormat()->GetVertOrient();
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

            // vertical clipping: Top and Bottom, also to PrtArea if necessary
            if( rV.GetVertOrient() != text::VertOrientation::NONE &&
                rV.GetRelationOrient() == text::RelOrientation::PRINT_AREA )
            {
                (rRect.*fnRect->fnSetTop)( (pClip->*fnRect->fnGetPrtTop)() );
                (rRect.*fnRect->fnSetBottom)( (pClip->*fnRect->fnGetPrtBottom)() );
            }
            // horizontal clipping: Top and Bottom, also to PrtArea if necessary
            const SwFormatHoriOrient &rH = pFly->GetFormat()->GetHoriOrient();
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
                const sal_uInt16 nType = bMove ? FRM_ROOT   | FRM_FLY | FRM_HEADER |
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
                        if (pUp)
                        {
                            rRect = pUp->GetUpper()->Frm();
                            (rRect.*fnRect->fnSetTop)( (pUp->*fnRect->fnGetPrtTop)() );
                            (rRect.*fnRect->fnSetBottom)((pUp->*fnRect->fnGetPrtBottom)());
                        }
                    }
                    else
                    {
                        if( ( pUp->GetType() & (FRM_FLY | FRM_FTN ) ) &&
                            !pUp->Frm().IsInside( pFly->Frm().Pos() ) )
                        {
                            if( pUp->IsFlyFrm() )
                            {
                                const SwFlyFrm *pTmpFly = static_cast<const SwFlyFrm*>(pUp);
                                while( pTmpFly->GetNextLink() )
                                {
                                    pTmpFly = pTmpFly->GetNextLink();
                                    if( pTmpFly->Frm().IsInside( pFly->Frm().Pos() ) )
                                        break;
                                }
                                pUp = pTmpFly;
                            }
                            else if( pUp->IsInFootnote() )
                            {
                                const SwFootnoteFrm *pTmp = pUp->FindFootnoteFrm();
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
                    // CellFrms might also sit in unallowed areas. In this case,
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
            const SwFormat *pFormat = static_cast<SwContact*>(GetUserCall(pSdrObj))->GetFormat();
            const SvxULSpaceItem &rUL = pFormat->GetULSpace();
            if( bMove )
            {
                nTop = bVert ? static_cast<const SwFlyInCntFrm*>(pFly)->GetRefPoint().X() :
                               static_cast<const SwFlyInCntFrm*>(pFly)->GetRefPoint().Y();
                nTop = (*fnRect->fnYInc)( nTop, -nHeight );
                long nWidth = (pFly->Frm().*fnRect->fnGetWidth)();
                (rRect.*fnRect->fnSetLeftAndWidth)( bVert ?
                            static_cast<const SwFlyInCntFrm*>(pFly)->GetRefPoint().Y() :
                            static_cast<const SwFlyInCntFrm*>(pFly)->GetRefPoint().X(), nWidth );
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
        const SwDrawContact *pC = static_cast<const SwDrawContact*>(GetUserCall(pSdrObj));
        const SwFrameFormat  *pFormat = pC->GetFormat();
        const SwFormatAnchor &rAnch = pFormat->GetAnchor();
        if ( FLY_AS_CHAR == rAnch.GetAnchorId() )
        {
            const SwFrm* pAnchorFrm = pC->GetAnchorFrm( pSdrObj );
            if( !pAnchorFrm )
            {
                OSL_FAIL( "<::CalcClipRect(..)> - missing anchor frame." );
                const_cast<SwDrawContact*>(pC)->ConnectToLayout();
                pAnchorFrm = pC->GetAnchorFrm();
            }
            const SwFrm* pUp = pAnchorFrm->GetUpper();
            rRect = pUp->Prt();
            rRect += pUp->Frm().Pos();
            SWRECTFN( pAnchorFrm )
            long nHeight = (9*(rRect.*fnRect->fnGetHeight)())/10;
            long nTop;
            const SvxULSpaceItem &rUL = pFormat->GetULSpace();
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
                bRet = false;
            }
        }
    }
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
