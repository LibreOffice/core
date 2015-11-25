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

SwFlyFreeFrame::SwFlyFreeFrame( SwFlyFrameFormat *pFormat, SwFrame* pSib, SwFrame *pAnch ) :
    SwFlyFrame( pFormat, pSib, pAnch ),
    // #i34753#
    mbNoMakePos( false ),
    // #i37068#
    mbNoMoveOnCheckClip( false ),
    maUnclippedFrame( )
{
}

void SwFlyFreeFrame::DestroyImpl()
{
    // #i28701# - use new method <GetPageFrame()>
    if( GetPageFrame() )
    {
        if( GetFormat()->GetDoc()->IsInDtor() )
        {
            // #i29879# - remove also to-frame anchored Writer
            // fly frame from page.
            const bool bRemoveFromPage =
                    GetPageFrame()->GetSortedObjs() &&
                    ( IsFlyAtContentFrame() ||
                      ( GetAnchorFrame() && GetAnchorFrame()->IsFlyFrame() ) );
            if ( bRemoveFromPage )
            {
                GetPageFrame()->GetSortedObjs()->Remove( *this );
            }
        }
        else
        {
            SwRect aTmp( GetObjRectWithSpaces() );
            SwFlyFreeFrame::NotifyBackground( GetPageFrame(), aTmp, PREP_FLY_LEAVE );
        }
    }

    SwFlyFrame::DestroyImpl();
}

SwFlyFreeFrame::~SwFlyFreeFrame()
{
}

// #i28701#
/** Notifies the background (all ContentFrames that currently are overlapping).
 *
 * Additionally, the window is also directly invalidated (especially where
 * there are no overlapping ContentFrames).
 * This also takes ContentFrames within other Flys into account.
 */
void SwFlyFreeFrame::NotifyBackground( SwPageFrame *pPageFrame,
                                     const SwRect& rRect, PrepareHint eHint )
{
    ::Notify_Background( GetVirtDrawObj(), pPageFrame, rRect, eHint, true );
}

void SwFlyFreeFrame::MakeAll(vcl::RenderContext* /*pRenderContext*/)
{
    if ( !GetFormat()->GetDoc()->getIDocumentDrawModelAccess().IsVisibleLayerId( GetVirtDrawObj()->GetLayer() ) )
    {
        return;
    }

    if ( !GetAnchorFrame() || IsLocked() || IsColLocked() )
        return;
    // #i28701# - use new method <GetPageFrame()>
    if( !GetPageFrame() && GetAnchorFrame() && GetAnchorFrame()->IsInFly() )
    {
        SwFlyFrame* pFly = AnchorFrame()->FindFlyFrame();
        SwPageFrame *pPageFrame = pFly ? pFly->FindPageFrame() : nullptr;
        if( pPageFrame )
            pPageFrame->AppendFlyToPage( this );
    }
    if( !GetPageFrame() )
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
                GetAnchorFrame()->IsInFly() &&
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
        const SwFormatFrameSize *pSz;
        {   // Additional scope, so aAccess will be destroyed before the check!

            SwBorderAttrAccess aAccess( SwFrame::GetCache(), this );
            const SwBorderAttrs &rAttrs = *aAccess.Get();
            pSz = &rAttrs.GetAttrSet().GetFrameSize();

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
                Format( getRootFrame()->GetCurrShell()->GetOut(), &rAttrs );
                m_bFormatHeightOnly = false;
            }
        }

        if ( !mbValidPos )
        {
            const Point aOldPos( (Frame().*fnRect->fnGetPos)() );
            // #i26791# - use new method <MakeObjPos()>
            // #i34753# - no positioning, if requested.
            if ( IsNoMakePos() )
                mbValidPos = true;
            else
                // #i26791# - use new method <MakeObjPos()>
                MakeObjPos();
            if( aOldPos == (Frame().*fnRect->fnGetPos)() )
            {
                if( !mbValidPos && GetAnchorFrame()->IsInSct() &&
                    !GetAnchorFrame()->FindSctFrame()->IsValid() )
                    mbValidPos = true;
            }
            else
                mbValidSize = false;
        }

        if ( !m_bValidContentPos )
        {
            SwBorderAttrAccess aAccess( SwFrame::GetCache(), this );
            const SwBorderAttrs &rAttrs = *aAccess.Get();
            MakeContentPos( rAttrs );
        }

        if ( mbValidPos && mbValidSize )
        {
            ++nLoopControlRuns;

            OSL_ENSURE( nLoopControlRuns < nLoopControlMax, "LoopControl in SwFlyFreeFrame::MakeAll" );

            if ( nLoopControlRuns < nLoopControlMax )
                CheckClip( *pSz );
        }
        else
            nLoopControlRuns = 0;
    }
    Unlock();

#if OSL_DEBUG_LEVEL > 0
    SWRECTFN( this )
    OSL_ENSURE( m_bHeightClipped || ( (Frame().*fnRect->fnGetHeight)() > 0 &&
            (Prt().*fnRect->fnGetHeight)() > 0),
            "SwFlyFreeFrame::Format(), flipping Fly." );

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
bool SwFlyFreeFrame::HasEnvironmentAutoSize() const
{
    bool bRetVal = false;

    const SwFrame* pToBeCheckedFrame = GetAnchorFrame();
    while ( pToBeCheckedFrame &&
            !pToBeCheckedFrame->IsPageFrame() )
    {
        if ( pToBeCheckedFrame->IsHeaderFrame() ||
             pToBeCheckedFrame->IsFooterFrame() ||
             pToBeCheckedFrame->IsRowFrame() ||
             pToBeCheckedFrame->IsFlyFrame() )
        {
            bRetVal = ATT_FIX_SIZE !=
                      pToBeCheckedFrame->GetAttrSet()->GetFrameSize().GetHeightSizeType();
            break;
        }
        else
        {
            pToBeCheckedFrame = pToBeCheckedFrame->GetUpper();
        }
    }

    return bRetVal;
}

void SwFlyFreeFrame::CheckClip( const SwFormatFrameSize &rSz )
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

    const long nBot = Frame().Top() + Frame().Height();
    const long nRig = Frame().Left() + Frame().Width();
    const long nClipBot = aClip.Top() + aClip.Height();
    const long nClipRig = aClip.Left() + aClip.Width();

    const bool bBot = nBot > nClipBot;
    const bool bRig = nRig > nClipRig;
    if ( bBot || bRig )
    {
        bool bAgain = false;
        // #i37068# - no move, if it's requested
        if ( bBot && !IsNoMoveOnCheckClip() &&
             !GetDrawObjs() && !GetAnchorFrame()->IsInTab() )
        {
            SwFrame* pHeader = FindFooterOrHeader();
            // In a header, correction of the position is no good idea.
            // If the fly moves, some paragraphs have to be formatted, this
            // could cause a change of the height of the headerframe,
            // now the flyframe can change its position and so on ...
            if ( !pHeader || !pHeader->IsHeaderFrame() )
            {
                const long nOld = Frame().Top();
                Frame().Pos().Y() = std::max( aClip.Top(), nClipBot - Frame().Height() );
                if ( Frame().Top() != nOld )
                    bAgain = true;
                m_bHeightClipped = true;
            }
        }
        if ( bRig )
        {
            const long nOld = Frame().Left();
            Frame().Pos().X() = std::max( aClip.Left(), nClipRig - Frame().Width() );
            if ( Frame().Left() != nOld )
            {
                const SwFormatHoriOrient &rH = GetFormat()->GetHoriOrient();
                // Left-aligned ones may not be moved to the left when they
                // are avoiding another one.
                if( rH.GetHoriOrient() == text::HoriOrientation::LEFT )
                    Frame().Pos().X() = nOld;
                else
                    bAgain = true;
            }
            m_bWidthClipped = true;
        }
        if ( bAgain )
            mbValidSize = false;
        else
        {
            // If we reach this branch, the Frame protrudes into forbidden
            // areas, and correcting the position is not allowed or not
            // possible or not required.

            // For Flys with OLE objects as lower, we make sure that
            // we always resize proportionally
            Size aOldSize( Frame().SSize() );

            // First, setup the FrameRect, then transfer it to the Frame.
            SwRect aFrameRect( Frame() );

            if ( bBot )
            {
                long nDiff = nClipBot;
                nDiff -= aFrameRect.Top(); // nDiff represents the available distance
                nDiff = aFrameRect.Height() - nDiff;
                aFrameRect.Height( aFrameRect.Height() - nDiff );
                m_bHeightClipped = true;
            }
            if ( bRig )
            {
                long nDiff = nClipRig;
                nDiff -= aFrameRect.Left();// nDiff represents the available distance
                nDiff = aFrameRect.Width() - nDiff;
                aFrameRect.Width( aFrameRect.Width() - nDiff );
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
            if ( Lower() && Lower()->IsNoTextFrame() &&
                 ( static_cast<SwContentFrame*>(Lower())->GetNode()->GetOLENode() ||
                   !HasEnvironmentAutoSize() ) )
            {
                // If width and height got adjusted, then the bigger
                // change is relevant.
                if ( aFrameRect.Width() != aOldSize.Width() &&
                     aFrameRect.Height()!= aOldSize.Height() )
                {
                    if ( (aOldSize.Width() - aFrameRect.Width()) >
                         (aOldSize.Height()- aFrameRect.Height()) )
                        aFrameRect.Height( aOldSize.Height() );
                    else
                        aFrameRect.Width( aOldSize.Width() );
                }

                // Adjusted the width? change height proportionally
                if( aFrameRect.Width() != aOldSize.Width() )
                {
                    aFrameRect.Height( aFrameRect.Width() * aOldSize.Height() /
                                     aOldSize.Width() );
                    m_bHeightClipped = true;
                }
                // Adjusted the height? change width proportionally
                else if( aFrameRect.Height() != aOldSize.Height() )
                {
                    aFrameRect.Width( aFrameRect.Height() * aOldSize.Width() /
                                    aOldSize.Height() );
                    m_bWidthClipped = true;
                }

                // #i17297# - reactivate change
                // of size attribute for fly frames containing an ole object.

                // Added the aFrameRect.HasArea() hack, because
                // the environment of the ole object does not have to be valid
                // at this moment, or even worse, it does not have to have a
                // reasonable size. In this case we do not want to change to
                // attributes permanentely. Maybe one day somebody dares to remove
                // this code.
                if ( aFrameRect.HasArea() &&
                     static_cast<SwContentFrame*>(Lower())->GetNode()->GetOLENode() &&
                     ( m_bWidthClipped || m_bHeightClipped ) )
                {
                    SwFlyFrameFormat *pFormat = GetFormat();
                    pFormat->LockModify();
                    SwFormatFrameSize aFrameSize( rSz );
                    aFrameSize.SetWidth( aFrameRect.Width() );
                    aFrameSize.SetHeight( aFrameRect.Height() );
                    pFormat->SetFormatAttr( aFrameSize );
                    pFormat->UnlockModify();
                }
            }

            // Now change the Frame; for columns, we put the new values into the attributes,
            // otherwise we'll end up with unwanted side-effects/oscillations
            const long nPrtHeightDiff = Frame().Height() - Prt().Height();
            const long nPrtWidthDiff  = Frame().Width()  - Prt().Width();
            maUnclippedFrame = SwRect( Frame() );
            Frame().Height( aFrameRect.Height() );
            Frame().Width ( std::max( long(MINLAY), aFrameRect.Width() ) );
            if ( Lower() && Lower()->IsColumnFrame() )
            {
                ColLock();  //lock grow/shrink
                const Size aTmpOldSize( Prt().SSize() );
                Prt().Height( Frame().Height() - nPrtHeightDiff );
                Prt().Width ( Frame().Width()  - nPrtWidthDiff );
                ChgLowersProp( aTmpOldSize );
                SwFrame *pLow = Lower();
                do
                {
                    pLow->Calc(getRootFrame()->GetCurrShell()->GetOut());
                    // also calculate the (Column)BodyFrame
                    static_cast<SwLayoutFrame*>(pLow)->Lower()->Calc(getRootFrame()->GetCurrShell()->GetOut());
                    pLow = pLow->GetNext();
                } while ( pLow );
                ::CalcContent( this );
                ColUnlock();
                if ( !mbValidSize && !m_bWidthClipped )
                    m_bFormatHeightOnly = mbValidSize = true;
            }
            else
            {
                Prt().Height( Frame().Height() - nPrtHeightDiff );
                Prt().Width ( Frame().Width()  - nPrtWidthDiff );
            }
        }
    }

    // #i26945#
    OSL_ENSURE( Frame().Height() >= 0,
            "<SwFlyFreeFrame::CheckClip(..)> - fly frame has negative height now." );
}

/** method to determine, if a <MakeAll()> on the Writer fly frame is possible
    #i43771#
*/
bool SwFlyFreeFrame::IsFormatPossible() const
{
    return SwFlyFrame::IsFormatPossible() &&
           ( GetPageFrame() ||
             ( GetAnchorFrame() && GetAnchorFrame()->IsInFly() ) );
}

SwFlyLayFrame::SwFlyLayFrame( SwFlyFrameFormat *pFormat, SwFrame* pSib, SwFrame *pAnch ) :
    SwFlyFreeFrame( pFormat, pSib, pAnch )
{
    m_bLayout = true;
}

// #i28701#

void SwFlyLayFrame::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew )
{
    const sal_uInt16 nWhich = pNew ? pNew->Which() : 0;

    const SwFormatAnchor *pAnch = nullptr;
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

        // Unregister, get hold of the page, attach to the corresponding LayoutFrame.
        SwRect aOld( GetObjRectWithSpaces() );
        // #i28701# - use new method <GetPageFrame()>
        SwPageFrame *pOldPage = GetPageFrame();
        AnchorFrame()->RemoveFly( this );

        if ( FLY_AT_PAGE == pAnch->GetAnchorId() )
        {
            sal_uInt16 nPgNum = pAnch->GetPageNum();
            SwRootFrame *pRoot = getRootFrame();
            SwPageFrame *pTmpPage = static_cast<SwPageFrame*>(pRoot->Lower());
            for ( sal_uInt16 i = 1; (i <= nPgNum) && pTmpPage; ++i,
                                pTmpPage = static_cast<SwPageFrame*>(pTmpPage->GetNext()) )
            {
                if ( i == nPgNum )
                {
                    // #i50432# - adjust synopsis of <PlaceFly(..)>
                    pTmpPage->PlaceFly( this, nullptr );
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
            SwContentFrame *pContent = GetFormat()->GetDoc()->GetNodes().GoNext( &aIdx )->
                         GetContentNode()->getLayoutFrame( getRootFrame(), nullptr, nullptr, false );
            if( pContent )
            {
                SwFlyFrame *pTmp = pContent->FindFlyFrame();
                if( pTmp )
                    pTmp->AppendFly( this );
            }
        }
        // #i28701# - use new method <GetPageFrame()>
        if ( pOldPage && pOldPage != GetPageFrame() )
            NotifyBackground( pOldPage, aOld, PREP_FLY_LEAVE );
        SetCompletePaint();
        InvalidateAll();
        SetNotifyBack();
    }
    else
        SwFlyFrame::Modify( pOld, pNew );
}

void SwPageFrame::AppendFlyToPage( SwFlyFrame *pNew )
{
    if ( !pNew->GetVirtDrawObj()->IsInserted() )
        getRootFrame()->GetDrawPage()->InsertObject(
                static_cast<SdrObject*>(pNew->GetVirtDrawObj()),
                pNew->GetVirtDrawObj()->GetReferencedObj().GetOrdNumDirect() );

    InvalidateSpelling();
    InvalidateSmartTags();
    InvalidateAutoCompleteWords();
    InvalidateWordCount();

    if ( GetUpper() )
    {
        static_cast<SwRootFrame*>(GetUpper())->SetIdleFlags();
        static_cast<SwRootFrame*>(GetUpper())->InvalidateBrowseWidth();
    }

    SdrObject* pObj = pNew->GetVirtDrawObj();
    OSL_ENSURE( pNew->GetAnchorFrame(), "Fly without Anchor" );
    SwFlyFrame* pFly = const_cast<SwFlyFrame*>(pNew->GetAnchorFrame()->FindFlyFrame());
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
    if ( pNew->IsFlyInContentFrame() )
        InvalidateFlyInCnt();
    else
    {
        InvalidateFlyContent();

        if ( !m_pSortedObjs )
            m_pSortedObjs = new SwSortedObjs();

        const bool bSucessInserted = m_pSortedObjs->Insert( *pNew );
        OSL_ENSURE( bSucessInserted, "Fly not inserted in Sorted." );
        (void) bSucessInserted;

        // #i87493#
        OSL_ENSURE( pNew->GetPageFrame() == nullptr || pNew->GetPageFrame() == this,
                "<SwPageFrame::AppendFlyToPage(..)> - anchored fly frame seems to be registered at another page frame. Serious defect." );
        // #i28701# - use new method <SetPageFrame(..)>
        pNew->SetPageFrame( this );
        pNew->InvalidatePage( this );
        // #i28701#
        pNew->UnlockPosition();

        // Notify accessible layout. That's required at this place for
        // frames only where the anchor is moved. Creation of new frames
        // is additionally handled by the SwFrameNotify class.
        if( GetUpper() &&
            static_cast< SwRootFrame * >( GetUpper() )->IsAnyShellAccessible() &&
             static_cast< SwRootFrame * >( GetUpper() )->GetCurrShell() )
        {
            static_cast< SwRootFrame * >( GetUpper() )->GetCurrShell()->Imp()
                                      ->AddAccessibleFrame( pNew );
        }
    }

    // #i28701# - correction: consider also drawing objects
    if ( pNew->GetDrawObjs() )
    {
        SwSortedObjs &rObjs = *pNew->GetDrawObjs();
        for ( size_t i = 0; i < rObjs.size(); ++i )
        {
            SwAnchoredObject* pTmpObj = rObjs[i];
            if ( dynamic_cast<const SwFlyFrame*>( pTmpObj) !=  nullptr )
            {
                SwFlyFrame* pTmpFly = static_cast<SwFlyFrame*>(pTmpObj);
                // #i28701# - use new method <GetPageFrame()>
                if ( pTmpFly->IsFlyFreeFrame() && !pTmpFly->GetPageFrame() )
                    AppendFlyToPage( pTmpFly );
            }
            else if ( dynamic_cast<const SwAnchoredDrawObject*>( pTmpObj) !=  nullptr )
            {
                // #i87493#
                if ( pTmpObj->GetPageFrame() != this )
                {
                    if ( pTmpObj->GetPageFrame() != nullptr )
                    {
                        pTmpObj->GetPageFrame()->RemoveDrawObjFromPage( *pTmpObj );
                    }
                    AppendDrawObjToPage( *pTmpObj );
                }
            }
        }
    }
}

void SwPageFrame::RemoveFlyFromPage( SwFlyFrame *pToRemove )
{
    const sal_uInt32 nOrdNum = pToRemove->GetVirtDrawObj()->GetOrdNum();
    getRootFrame()->GetDrawPage()->RemoveObject( nOrdNum );
    pToRemove->GetVirtDrawObj()->ReferencedObj().SetOrdNum( nOrdNum );

    if ( GetUpper() )
    {
        if ( !pToRemove->IsFlyInContentFrame() )
            static_cast<SwRootFrame*>(GetUpper())->SetSuperfluous();
        static_cast<SwRootFrame*>(GetUpper())->InvalidateBrowseWidth();
    }

    // Don't look further at Flys that sit inside the Content.
    if ( pToRemove->IsFlyInContentFrame() )
        return;

    // Don't delete collections just yet. This will happen at the end of the
    // action in the RemoveSuperfluous of the page, kicked off by a method of
    // the same name in the root.
    // The FlyColl might be gone already, because the page's dtor is being
    // executed.
    // Remove it _before_ disposing accessible frames to avoid accesses to
    // the Frame from event handlers.
    if (m_pSortedObjs)
    {
        m_pSortedObjs->Remove(*pToRemove);
        if (!m_pSortedObjs->size())
        {
            delete m_pSortedObjs;
            m_pSortedObjs = nullptr;
        }
    }

    // Notify accessible layout. That's required at this place for
    // frames only where the anchor is moved. Creation of new frames
    // is additionally handled by the SwFrameNotify class.
    if( GetUpper() &&
        static_cast< SwRootFrame * >( GetUpper() )->IsAnyShellAccessible() &&
        static_cast< SwRootFrame * >( GetUpper() )->GetCurrShell() )
    {
        static_cast< SwRootFrame * >( GetUpper() )->GetCurrShell()->Imp()
                                  ->DisposeAccessibleFrame( pToRemove, true );
    }

    // #i28701# - use new method <SetPageFrame(..)>
    pToRemove->SetPageFrame( nullptr );
}

void SwPageFrame::MoveFly( SwFlyFrame *pToMove, SwPageFrame *pDest )
{
    // Invalidations
    if ( GetUpper() )
    {
        static_cast<SwRootFrame*>(GetUpper())->SetIdleFlags();
        if ( !pToMove->IsFlyInContentFrame() && pDest->GetPhyPageNum() < GetPhyPageNum() )
            static_cast<SwRootFrame*>(GetUpper())->SetSuperfluous();
    }

    pDest->InvalidateSpelling();
    pDest->InvalidateSmartTags();
    pDest->InvalidateAutoCompleteWords();
    pDest->InvalidateWordCount();

    if ( pToMove->IsFlyInContentFrame() )
    {
        pDest->InvalidateFlyInCnt();
        return;
    }

    // Notify accessible layout. That's required at this place for
    // frames only where the anchor is moved. Creation of new frames
    // is additionally handled by the SwFrameNotify class.
    if( GetUpper() &&
        static_cast< SwRootFrame * >( GetUpper() )->IsAnyShellAccessible() &&
        static_cast< SwRootFrame * >( GetUpper() )->GetCurrShell() )
    {
        static_cast< SwRootFrame * >( GetUpper() )->GetCurrShell()->Imp()
                                  ->DisposeAccessibleFrame( pToMove, true );
    }

    // The FlyColl might be gone already, because the page's dtor is being executed.
    if ( m_pSortedObjs )
    {
        m_pSortedObjs->Remove( *pToMove );
        if ( !m_pSortedObjs->size() )
        {
            DELETEZ( m_pSortedObjs );
        }
    }

    // Register
    if ( !pDest->GetSortedObjs() )
        pDest->m_pSortedObjs = new SwSortedObjs();

    const bool bSucessInserted = pDest->GetSortedObjs()->Insert( *pToMove );
    OSL_ENSURE( bSucessInserted, "Fly not inserted in Sorted." );
    (void) bSucessInserted;

    // #i28701# - use new method <SetPageFrame(..)>
    pToMove->SetPageFrame( pDest );
    pToMove->InvalidatePage( pDest );
    pToMove->SetNotifyBack();
    pDest->InvalidateFlyContent();
    // #i28701#
    pToMove->UnlockPosition();

    // Notify accessible layout. That's required at this place for
    // frames only where the anchor is moved. Creation of new frames
    // is additionally handled by the SwFrameNotify class.
    if( GetUpper() &&
        static_cast< SwRootFrame * >( GetUpper() )->IsAnyShellAccessible() &&
        static_cast< SwRootFrame * >( GetUpper() )->GetCurrShell() )
    {
        static_cast< SwRootFrame * >( GetUpper() )->GetCurrShell()->Imp()
                                  ->AddAccessibleFrame( pToMove );
    }

    // #i28701# - correction: move lowers of Writer fly frame
    if ( pToMove->GetDrawObjs() )
    {
        SwSortedObjs &rObjs = *pToMove->GetDrawObjs();
        for ( size_t i = 0; i < rObjs.size(); ++i )
        {
            SwAnchoredObject* pObj = rObjs[i];
            if ( dynamic_cast<const SwFlyFrame*>( pObj) !=  nullptr )
            {
                SwFlyFrame* pFly = static_cast<SwFlyFrame*>(pObj);
                if ( pFly->IsFlyFreeFrame() )
                {
                    // #i28701# - use new method <GetPageFrame()>
                    SwPageFrame* pPageFrame = pFly->GetPageFrame();
                    if ( pPageFrame )
                        pPageFrame->MoveFly( pFly, pDest );
                    else
                        pDest->AppendFlyToPage( pFly );
                }
            }
            else if ( dynamic_cast<const SwAnchoredDrawObject*>( pObj) !=  nullptr )
            {
                RemoveDrawObjFromPage( *pObj );
                pDest->AppendDrawObjToPage( *pObj );
            }
        }
    }
}

void SwPageFrame::AppendDrawObjToPage( SwAnchoredObject& _rNewObj )
{
    if ( dynamic_cast<const SwAnchoredDrawObject*>( &_rNewObj) ==  nullptr )
    {
        OSL_FAIL( "SwPageFrame::AppendDrawObjToPage(..) - anchored object of unexpected type -> object not appended" );
        return;
    }

    if ( GetUpper() )
    {
        static_cast<SwRootFrame*>(GetUpper())->InvalidateBrowseWidth();
    }

    OSL_ENSURE( _rNewObj.GetAnchorFrame(), "anchored draw object without anchor" );
    SwFlyFrame* pFlyFrame = const_cast<SwFlyFrame*>(_rNewObj.GetAnchorFrame()->FindFlyFrame());
    if ( pFlyFrame &&
         _rNewObj.GetDrawObj()->GetOrdNum() < pFlyFrame->GetVirtDrawObj()->GetOrdNum() )
    {
        //#i119945# set pFly's OrdNum to _rNewObj's. So when pFly is removed by Undo, the original OrdNum will not be changed.
        sal_uInt32 nNewNum = _rNewObj.GetDrawObj()->GetOrdNumDirect();
        if ( _rNewObj.GetDrawObj()->GetPage() )
            _rNewObj.DrawObj()->GetPage()->SetObjectOrdNum( pFlyFrame->GetVirtDrawObj()->GetOrdNumDirect(), nNewNum );
        else
            pFlyFrame->GetVirtDrawObj()->SetOrdNum( nNewNum );
    }

    if ( FLY_AS_CHAR == _rNewObj.GetFrameFormat().GetAnchor().GetAnchorId() )
    {
        return;
    }

    if ( !m_pSortedObjs )
    {
        m_pSortedObjs = new SwSortedObjs();
    }
    if ( !m_pSortedObjs->Insert( _rNewObj ) )
    {
        OSL_ENSURE( m_pSortedObjs->Contains( _rNewObj ),
                "Drawing object not appended into list <pSortedObjs>." );
    }
    // #i87493#
    OSL_ENSURE( _rNewObj.GetPageFrame() == nullptr || _rNewObj.GetPageFrame() == this,
            "<SwPageFrame::AppendDrawObjToPage(..)> - anchored draw object seems to be registered at another page frame. Serious defect." );
    _rNewObj.SetPageFrame( this );

    // invalidate page in order to force a reformat of object layout of the page.
    InvalidateFlyLayout();
}

void SwPageFrame::RemoveDrawObjFromPage( SwAnchoredObject& _rToRemoveObj )
{
    if ( dynamic_cast<const SwAnchoredDrawObject*>( &_rToRemoveObj) ==  nullptr )
    {
        OSL_FAIL( "SwPageFrame::RemoveDrawObjFromPage(..) - anchored object of unexpected type -> object not removed" );
        return;
    }

    if ( m_pSortedObjs )
    {
        m_pSortedObjs->Remove( _rToRemoveObj );
        if ( !m_pSortedObjs->size() )
        {
            DELETEZ( m_pSortedObjs );
        }
        if ( GetUpper() )
        {
            if (FLY_AS_CHAR !=
                    _rToRemoveObj.GetFrameFormat().GetAnchor().GetAnchorId())
            {
                static_cast<SwRootFrame*>(GetUpper())->SetSuperfluous();
                InvalidatePage();
            }
            static_cast<SwRootFrame*>(GetUpper())->InvalidateBrowseWidth();
        }
    }
    _rToRemoveObj.SetPageFrame( nullptr );
}

// #i50432# - adjust method description and synopsis.
void SwPageFrame::PlaceFly( SwFlyFrame* pFly, SwFlyFrameFormat* pFormat )
{
    // #i50432# - consider the case that page is an empty page:
    // In this case append the fly frame at the next page
    OSL_ENSURE( !IsEmptyPage() || GetNext(),
            "<SwPageFrame::PlaceFly(..)> - empty page with no next page! -> fly frame appended at empty page" );
    if ( IsEmptyPage() && GetNext() )
    {
        static_cast<SwPageFrame*>(GetNext())->PlaceFly( pFly, pFormat );
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
            pFly = new SwFlyLayFrame( pFormat, this, this );
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
    if ( dynamic_cast<const SwVirtFlyDrawObj*>( pSdrObj) !=  nullptr )
    {
        const SwFlyFrame* pFly = static_cast<const SwVirtFlyDrawObj*>(pSdrObj)->GetFlyFrame();
        const bool bFollowTextFlow = pFly->GetFormat()->GetFollowTextFlow().GetValue();
        // #i28701#
        const bool bConsiderWrapOnObjPos =
                                pFly->GetFormat()->getIDocumentSettingAccess().get(DocumentSettingId::CONSIDER_WRAP_ON_OBJECT_POSITION);
        const SwFormatVertOrient &rV = pFly->GetFormat()->GetVertOrient();
        if( pFly->IsFlyLayFrame() )
        {
            const SwFrame* pClip;
            // #i22305#
            // #i28701#
            if ( !bFollowTextFlow || bConsiderWrapOnObjPos )
            {
                pClip = pFly->GetAnchorFrame()->FindPageFrame();
            }
            else
            {
                pClip = pFly->GetAnchorFrame();
            }

            rRect = pClip->Frame();
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
        else if( pFly->IsFlyAtContentFrame() )
        {
            // #i18732# - consider following text flow or not
            // AND alignment at 'page areas'
            const SwFrame* pVertPosOrientFrame = pFly->GetVertPosOrientFrame();
            if ( !pVertPosOrientFrame )
            {
                OSL_FAIL( "::CalcClipRect(..) - frame, vertical position is oriented at, is missing .");
                pVertPosOrientFrame = pFly->GetAnchorFrame();
            }

            if ( !bFollowTextFlow || bConsiderWrapOnObjPos )
            {
                const SwLayoutFrame* pClipFrame = pVertPosOrientFrame->FindPageFrame();
                if (!pClipFrame)
                {
                    OSL_FAIL("!pClipFrame: "
                            "if you can reproduce this please file a bug");
                    return false;
                }
                rRect = bMove ? pClipFrame->GetUpper()->Frame()
                              : pClipFrame->Frame();
                // #i26945# - consider that a table, during
                // its format, can exceed its upper printing area bottom.
                // Thus, enlarge the clip rectangle, if such a case occurred
                if ( pFly->GetAnchorFrame()->IsInTab() )
                {
                    const SwTabFrame* pTabFrame = const_cast<SwFlyFrame*>(pFly)
                                ->GetAnchorFrameContainingAnchPos()->FindTabFrame();
                    SwRect aTmp( pTabFrame->Prt() );
                    aTmp += pTabFrame->Frame().Pos();
                    rRect.Union( aTmp );
                    // #i43913# - consider also the cell frame
                    const SwFrame* pCellFrame = const_cast<SwFlyFrame*>(pFly)
                                ->GetAnchorFrameContainingAnchPos()->GetUpper();
                    while ( pCellFrame && !pCellFrame->IsCellFrame() )
                    {
                        pCellFrame = pCellFrame->GetUpper();
                    }
                    if ( pCellFrame )
                    {
                        aTmp = pCellFrame->Prt();
                        aTmp += pCellFrame->Frame().Pos();
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
                const SwLayoutFrame& rVertClipFrame =
                    aEnvOfObj.GetVertEnvironmentLayoutFrame( *pVertPosOrientFrame );
                if ( rV.GetRelationOrient() == text::RelOrientation::PAGE_FRAME )
                {
                    rRect = rVertClipFrame.Frame();
                }
                else if ( rV.GetRelationOrient() == text::RelOrientation::PAGE_PRINT_AREA )
                {
                    if ( rVertClipFrame.IsPageFrame() )
                    {
                        rRect = static_cast<const SwPageFrame&>(rVertClipFrame).PrtWithoutHeaderAndFooter();
                    }
                    else
                    {
                        rRect = rVertClipFrame.Frame();
                    }
                }
                const SwLayoutFrame* pHoriClipFrame =
                        pFly->GetAnchorFrame()->FindPageFrame()->GetUpper();
                SWRECTFN( pFly->GetAnchorFrame() )
                (rRect.*fnRect->fnSetLeft)( (pHoriClipFrame->Frame().*fnRect->fnGetLeft)() );
                (rRect.*fnRect->fnSetRight)((pHoriClipFrame->Frame().*fnRect->fnGetRight)());
            }
            else
            {
                // #i26945#
                const SwFrame *pClip =
                        const_cast<SwFlyFrame*>(pFly)->GetAnchorFrameContainingAnchPos();
                SWRECTFN( pClip )
                const SwLayoutFrame *pUp = pClip->GetUpper();
                const SwFrame *pCell = pUp->IsCellFrame() ? pUp : nullptr;
                const sal_uInt16 nType = bMove ? FRM_ROOT   | FRM_FLY | FRM_HEADER |
                                       FRM_FOOTER | FRM_FTN
                                     : FRM_BODY   | FRM_FLY | FRM_HEADER |
                                       FRM_FOOTER | FRM_CELL| FRM_FTN;

                while ( !(pUp->GetType() & nType) || pUp->IsColBodyFrame() )
                {
                    pUp = pUp->GetUpper();
                    if ( !pCell && pUp->IsCellFrame() )
                        pCell = pUp;
                }
                if ( bMove )
                {
                    if ( pUp->IsRootFrame() )
                    {
                        rRect  = pUp->Prt();
                        rRect += pUp->Frame().Pos();
                        pUp = nullptr;
                    }
                }
                if ( pUp )
                {
                    if ( pUp->GetType() & FRM_BODY )
                    {
                        const SwPageFrame *pPg;
                        if ( pUp->GetUpper() != (pPg = pFly->FindPageFrame()) )
                            pUp = pPg->FindBodyCont();
                        if (pUp)
                        {
                            rRect = pUp->GetUpper()->Frame();
                            (rRect.*fnRect->fnSetTop)( (pUp->*fnRect->fnGetPrtTop)() );
                            (rRect.*fnRect->fnSetBottom)((pUp->*fnRect->fnGetPrtBottom)());
                        }
                    }
                    else
                    {
                        if( ( pUp->GetType() & (FRM_FLY | FRM_FTN ) ) &&
                            !pUp->Frame().IsInside( pFly->Frame().Pos() ) )
                        {
                            if( pUp->IsFlyFrame() )
                            {
                                const SwFlyFrame *pTmpFly = static_cast<const SwFlyFrame*>(pUp);
                                while( pTmpFly->GetNextLink() )
                                {
                                    pTmpFly = pTmpFly->GetNextLink();
                                    if( pTmpFly->Frame().IsInside( pFly->Frame().Pos() ) )
                                        break;
                                }
                                pUp = pTmpFly;
                            }
                            else if( pUp->IsInFootnote() )
                            {
                                const SwFootnoteFrame *pTmp = pUp->FindFootnoteFrame();
                                while( pTmp->GetFollow() )
                                {
                                    pTmp = pTmp->GetFollow();
                                    if( pTmp->Frame().IsInside( pFly->Frame().Pos() ) )
                                        break;
                                }
                                pUp = pTmp;
                            }
                        }
                        rRect = pUp->Prt();
                        rRect.Pos() += pUp->Frame().Pos();
                        if ( pUp->GetType() & (FRM_HEADER | FRM_FOOTER) )
                        {
                            rRect.Left ( pUp->GetUpper()->Frame().Left() );
                            rRect.Width( pUp->GetUpper()->Frame().Width());
                        }
                        else if ( pUp->IsCellFrame() )                //MA_FLY_HEIGHT
                        {
                            const SwFrame *pTab = pUp->FindTabFrame();
                            (rRect.*fnRect->fnSetBottom)(
                                        (pTab->GetUpper()->*fnRect->fnGetPrtBottom)() );
                            // expand to left and right cell border
                            rRect.Left ( pUp->Frame().Left() );
                            rRect.Width( pUp->Frame().Width() );
                        }
                    }
                }
                if ( pCell )
                {
                    // CellFrames might also sit in unallowed areas. In this case,
                    // the Fly is allowed to do so as well
                    SwRect aTmp( pCell->Prt() );
                    aTmp += pCell->Frame().Pos();
                    rRect.Union( aTmp );
                }
            }
        }
        else
        {
            const SwFrame *pUp = pFly->GetAnchorFrame()->GetUpper();
            SWRECTFN( pFly->GetAnchorFrame() )
            while( pUp->IsColumnFrame() || pUp->IsSctFrame() || pUp->IsColBodyFrame())
                pUp = pUp->GetUpper();
            rRect = pUp->Frame();
            if( !pUp->IsBodyFrame() )
            {
                rRect += pUp->Prt().Pos();
                rRect.SSize( pUp->Prt().SSize() );
                if ( pUp->IsCellFrame() )
                {
                    const SwFrame *pTab = pUp->FindTabFrame();
                    (rRect.*fnRect->fnSetBottom)(
                                    (pTab->GetUpper()->*fnRect->fnGetPrtBottom)() );
                }
            }
            else if ( pUp->GetUpper()->IsPageFrame() )
            {
                // Objects anchored as character may exceed right margin
                // of body frame:
                (rRect.*fnRect->fnSetRight)( (pUp->GetUpper()->Frame().*fnRect->fnGetRight)() );
            }
            long nHeight = (9*(rRect.*fnRect->fnGetHeight)())/10;
            long nTop;
            const SwFormat *pFormat = static_cast<SwContact*>(GetUserCall(pSdrObj))->GetFormat();
            const SvxULSpaceItem &rUL = pFormat->GetULSpace();
            if( bMove )
            {
                nTop = bVert ? static_cast<const SwFlyInContentFrame*>(pFly)->GetRefPoint().X() :
                               static_cast<const SwFlyInContentFrame*>(pFly)->GetRefPoint().Y();
                nTop = (*fnRect->fnYInc)( nTop, -nHeight );
                long nWidth = (pFly->Frame().*fnRect->fnGetWidth)();
                (rRect.*fnRect->fnSetLeftAndWidth)( bVert ?
                            static_cast<const SwFlyInContentFrame*>(pFly)->GetRefPoint().Y() :
                            static_cast<const SwFlyInContentFrame*>(pFly)->GetRefPoint().X(), nWidth );
                nHeight = 2*nHeight - rUL.GetLower() - rUL.GetUpper();
            }
            else
            {
                nTop = (*fnRect->fnYInc)( (pFly->Frame().*fnRect->fnGetBottom)(),
                                           rUL.GetLower() - nHeight );
                nHeight = 2*nHeight - (pFly->Frame().*fnRect->fnGetHeight)()
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
            const SwFrame* pAnchorFrame = pC->GetAnchorFrame( pSdrObj );
            if( !pAnchorFrame )
            {
                OSL_FAIL( "<::CalcClipRect(..)> - missing anchor frame." );
                const_cast<SwDrawContact*>(pC)->ConnectToLayout();
                pAnchorFrame = pC->GetAnchorFrame();
            }
            const SwFrame* pUp = pAnchorFrame->GetUpper();
            rRect = pUp->Prt();
            rRect += pUp->Frame().Pos();
            SWRECTFN( pAnchorFrame )
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
            const SwFrame* pAnchorFrame = pC->GetAnchorFrame( pSdrObj );
            if ( pAnchorFrame && pAnchorFrame->FindFooterOrHeader() )
            {
                // clip frame is the page frame the header/footer is on.
                const SwFrame* pClipFrame = pAnchorFrame->FindPageFrame();
                rRect = pClipFrame->Frame();
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
