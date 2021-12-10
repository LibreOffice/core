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

#include <svx/svdpage.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/lspcitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <sal/log.hxx>
#include <o3tl/deleter.hxx>
#include <osl/diagnose.h>

#include <drawdoc.hxx>
#include <fmtornt.hxx>
#include <fmthdft.hxx>
#include <fmtfsize.hxx>
#include <fmtsrnd.hxx>
#include <docary.hxx>
#include <lineinfo.hxx>
#include <swmodule.hxx>
#include <pagefrm.hxx>
#include <colfrm.hxx>
#include <fesh.hxx>
#include <viewimp.hxx>
#include <viewopt.hxx>
#include <dflyobj.hxx>
#include <dcontact.hxx>
#include <frmatr.hxx>
#include <frmtool.hxx>
#include <tabfrm.hxx>
#include <rowfrm.hxx>
#include <ftnfrm.hxx>
#include <txtfrm.hxx>
#include <notxtfrm.hxx>
#include <flyfrms.hxx>
#include <layact.hxx>
#include <pagedesc.hxx>
#include <section.hxx>
#include <sectfrm.hxx>
#include <node2lay.hxx>
#include <ndole.hxx>
#include <hints.hxx>
#include "layhelp.hxx"
#include <laycache.hxx>
#include <rootfrm.hxx>
#include <paratr.hxx>
#include <redline.hxx>
#include <sortedobjs.hxx>
#include <objectformatter.hxx>
#include <calbck.hxx>
#include <ndtxt.hxx>
#include <undobj.hxx>
#include <DocumentSettingManager.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <IDocumentTimerAccess.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentState.hxx>
#include <frameformats.hxx>
#include <boost/circular_buffer.hpp>
#include <svx/sdr/attribute/sdrallfillattributeshelper.hxx>

using namespace ::com::sun::star;

namespace {
    // FIXME: would likely better be a member of SwRootFrame instead of a global flag
    bool isFlyCreationSuppressed = false;
}
namespace sw {
    FlyCreationSuppressor::FlyCreationSuppressor(bool wasAlreadySuppressedAllowed)
        : m_wasAlreadySuppressed(isFlyCreationSuppressed)
    {
        (void)wasAlreadySuppressedAllowed;
        assert(wasAlreadySuppressedAllowed || !isFlyCreationSuppressed);
        isFlyCreationSuppressed = true;
    }
    FlyCreationSuppressor::~FlyCreationSuppressor()
    {
        isFlyCreationSuppressed = m_wasAlreadySuppressed;
    }
}

bool bObjsDirect = true;
bool bSetCompletePaintOnInvalidate = false;

sal_uInt8 StackHack::s_nCnt = 0;
bool StackHack::s_bLocked = false;

SwFrameNotify::SwFrameNotify( SwFrame *pF ) :
    mpFrame( pF ),
    maFrame( pF->getFrameArea() ),
    maPrt( pF->getFramePrintArea() ),
    mbInvaKeep( false ),
    mbValidSize( pF->isFrameAreaSizeValid() )
{
    if ( pF->IsTextFrame() )
    {
        mnFlyAnchorOfst = static_cast<SwTextFrame*>(pF)->GetBaseOffsetForFly( true );
        mnFlyAnchorOfstNoWrap = static_cast<SwTextFrame*>(pF)->GetBaseOffsetForFly( false );
    }
    else
    {
        mnFlyAnchorOfst = 0;
        mnFlyAnchorOfstNoWrap = 0;
    }

    mbHadFollow = pF->IsContentFrame() && static_cast<SwContentFrame*>(pF)->GetFollow();
}

SwFrameNotify::~SwFrameNotify()
{
    suppress_fun_call_w_exception(ImplDestroy());
}

void SwFrameNotify::ImplDestroy()
{
    SwRectFnSet aRectFnSet(mpFrame);
    const bool bAbsP = aRectFnSet.PosDiff(maFrame, mpFrame->getFrameArea());
    const bool bChgWidth =
            aRectFnSet.GetWidth(maFrame) != aRectFnSet.GetWidth(mpFrame->getFrameArea());
    const bool bChgHeight =
            aRectFnSet.GetHeight(maFrame)!=aRectFnSet.GetHeight(mpFrame->getFrameArea());
    const bool bChgFlyBasePos = mpFrame->IsTextFrame() &&
       ( ( mnFlyAnchorOfst != static_cast<SwTextFrame*>(mpFrame)->GetBaseOffsetForFly( true ) ) ||
         ( mnFlyAnchorOfstNoWrap != static_cast<SwTextFrame*>(mpFrame)->GetBaseOffsetForFly( false ) ) );

    if ( mpFrame->IsFlowFrame() && !mpFrame->IsInFootnote() )
    {
        SwFlowFrame *pFlow = SwFlowFrame::CastFlowFrame( mpFrame );

        if ( !pFlow->IsFollow() )
        {
            if ( !mpFrame->GetIndPrev() )
            {
                if ( mbInvaKeep )
                {
                    SwFrame *pPre = mpFrame->FindPrev();
                    if ( pPre && pPre->IsFlowFrame() )
                    {
                        // 1. pPre wants to keep with me:
                        bool bInvalidPrePos = SwFlowFrame::CastFlowFrame(pPre)->IsKeep(pPre->GetAttrSet()->GetKeep(), pPre->GetBreakItem())
                            && pPre->GetIndPrev();

                        // 2. pPre is a table and the last row wants to keep with me:
                        if ( !bInvalidPrePos && pPre->IsTabFrame() )
                        {
                            SwTabFrame* pPreTab = static_cast<SwTabFrame*>(pPre);
                            if ( pPreTab->GetFormat()->GetDoc()->GetDocumentSettingManager().get(DocumentSettingId::TABLE_ROW_KEEP) )
                            {
                                SwRowFrame* pLastRow = static_cast<SwRowFrame*>(pPreTab->GetLastLower());
                                if ( pLastRow && pLastRow->ShouldRowKeepWithNext() )
                                    bInvalidPrePos = true;
                            }
                        }

                        if ( bInvalidPrePos )
                            pPre->InvalidatePos();
                    }
                }
            }
            else if ( !pFlow->HasFollow() )
            {
                tools::Long nOldHeight = aRectFnSet.GetHeight(maFrame);
                tools::Long nNewHeight = aRectFnSet.GetHeight(mpFrame->getFrameArea());
                if( (nOldHeight > nNewHeight) || (!nOldHeight && nNewHeight) )
                    pFlow->CheckKeep();
            }
        }
    }

    if ( bAbsP )
    {
        mpFrame->SetCompletePaint();

        SwFrame* pNxt = mpFrame->GetIndNext();
        // #121888# - skip empty section frames
        while ( pNxt &&
                pNxt->IsSctFrame() && !static_cast<SwSectionFrame*>(pNxt)->GetSection() )
        {
            pNxt = pNxt->GetIndNext();
        }

        if ( pNxt )
            pNxt->InvalidatePos();
        else
        {
            // #104100# - correct condition for setting retouche
            // flag for vertical layout.
            if( mpFrame->IsRetoucheFrame() &&
                aRectFnSet.TopDist( maFrame, aRectFnSet.GetTop(mpFrame->getFrameArea()) ) > 0 )
            {
                mpFrame->SetRetouche();
            }

            // A fresh follow frame does not have to be invalidated, because
            // it is already formatted:
            if ( mbHadFollow || !mpFrame->IsContentFrame() || !static_cast<SwContentFrame*>(mpFrame)->GetFollow() )
            {
                if ( !mpFrame->IsTabFrame() || !static_cast<SwTabFrame*>(mpFrame)->GetFollow() )
                    mpFrame->InvalidateNextPos();
            }
        }
    }

    //For each resize of the background graphics is a repaint necessary.
    const bool bPrtWidth =
            aRectFnSet.GetWidth(maPrt) != aRectFnSet.GetWidth(mpFrame->getFramePrintArea());
    const bool bPrtHeight =
            aRectFnSet.GetHeight(maPrt)!=aRectFnSet.GetHeight(mpFrame->getFramePrintArea());
    if ( bPrtWidth || bPrtHeight )
    {
        bool bUseNewFillProperties(false);
        if (mpFrame->supportsFullDrawingLayerFillAttributeSet())
        {
            drawinglayer::attribute::SdrAllFillAttributesHelperPtr aFillAttributes(mpFrame->getSdrAllFillAttributesHelper());
            if(aFillAttributes && aFillAttributes->isUsed())
            {
                bUseNewFillProperties = true;
                // use SetCompletePaint if needed
                if(aFillAttributes->needCompleteRepaint())
                {
                    mpFrame->SetCompletePaint();
                }
            }
        }
        if (!bUseNewFillProperties)
        {
            const SvxGraphicPosition ePos = mpFrame->GetAttrSet()->GetBackground().GetGraphicPos();
            if(GPOS_NONE != ePos && GPOS_TILED != ePos)
                mpFrame->SetCompletePaint();
        }
    }
    else
    {
        // #97597# - consider case that *only* margins between
        // frame and printing area has changed. Then, frame has to be repainted,
        // in order to force paint of the margin areas.
        if ( !bAbsP && (bChgWidth || bChgHeight) )
        {
            mpFrame->SetCompletePaint();
        }
    }

    const bool bPrtP = aRectFnSet.PosDiff( maPrt, mpFrame->getFramePrintArea() );
    if ( bAbsP || bPrtP || bChgWidth || bChgHeight ||
         bPrtWidth || bPrtHeight || bChgFlyBasePos )
    {
        if( mpFrame->IsAccessibleFrame() )
        {
            SwRootFrame *pRootFrame = mpFrame->getRootFrame();
            if( pRootFrame && pRootFrame->IsAnyShellAccessible() &&
                pRootFrame->GetCurrShell() )
            {
                pRootFrame->GetCurrShell()->Imp()->MoveAccessibleFrame( mpFrame, maFrame );
            }
        }

        // Notification of anchored objects
        if ( mpFrame->GetDrawObjs() )
        {
            const SwSortedObjs &rObjs = *mpFrame->GetDrawObjs();
            SwPageFrame* pPageFrame = nullptr;
            for (SwAnchoredObject* pObj : rObjs)
            {
                // OD 2004-03-31 #i26791# - no general distinction between
                // Writer fly frames and drawing objects
                bool bNotify = false;
                bool bNotifySize = false;
                SwContact* pContact = ::GetUserCall( pObj->GetDrawObj() );
                const bool bAnchoredAsChar = pContact->ObjAnchoredAsChar();
                if ( !bAnchoredAsChar )
                {
                    // Notify object, which aren't anchored as-character:

                    // always notify objects, if frame position has changed
                    // or if the object is to-page|to-fly anchored.
                    if ( bAbsP ||
                         pContact->ObjAnchoredAtPage() ||
                         pContact->ObjAnchoredAtFly() )
                    {
                        bNotify = true;

                        // assure that to-fly anchored Writer fly frames are
                        // registered at the correct page frame, if frame
                        // position has changed.
                        if ( bAbsP && pContact->ObjAnchoredAtFly() &&
                             pObj->DynCastFlyFrame() !=  nullptr )
                        {
                            // determine to-fly anchored Writer fly frame
                            SwFlyFrame* pFlyFrame = static_cast<SwFlyFrame*>(pObj);
                            // determine page frame of to-fly anchored
                            // Writer fly frame
                            SwPageFrame* pFlyPageFrame = pFlyFrame->FindPageFrame();
                            // determine page frame, if needed.
                            if ( !pPageFrame )
                            {
                                pPageFrame = mpFrame->FindPageFrame();
                            }
                            if ( pPageFrame != pFlyPageFrame )
                            {
                                OSL_ENSURE( pFlyPageFrame, "~SwFrameNotify: Fly from Nowhere" );
                                if( pFlyPageFrame )
                                    pFlyPageFrame->MoveFly( pFlyFrame, pPageFrame );
                                else
                                    pPageFrame->AppendFlyToPage( pFlyFrame );
                            }
                        }
                    }
                    // otherwise the objects are notified in dependence to
                    // its positioning and alignment
                    else
                    {
                        const SwFormatVertOrient& rVert =
                                        pContact->GetFormat()->GetVertOrient();
                        if ( ( rVert.GetVertOrient() == text::VertOrientation::CENTER ||
                               rVert.GetVertOrient() == text::VertOrientation::BOTTOM ||
                               rVert.GetRelationOrient() == text::RelOrientation::PRINT_AREA ) &&
                             ( bChgHeight || bPrtHeight ) )
                        {
                            bNotify = true;
                        }
                        if ( !bNotify )
                        {
                            const SwFormatHoriOrient& rHori =
                                        pContact->GetFormat()->GetHoriOrient();
                            if ( ( rHori.GetHoriOrient() != text::HoriOrientation::NONE ||
                                   rHori.GetRelationOrient()== text::RelOrientation::PRINT_AREA ||
                                   rHori.GetRelationOrient()== text::RelOrientation::FRAME ) &&
                                 ( bChgWidth || bPrtWidth || bChgFlyBasePos ) )
                            {
                                bNotify = true;
                            }
                        }
                    }
                }
                else if ( bPrtWidth )
                {
                    // Notify as-character anchored objects, if printing area
                    // width has changed.
                    bNotify = true;
                    bNotifySize = true;
                }

                // perform notification via the corresponding invalidations
                if ( bNotify )
                {
                    if ( auto pFlyFrame = pObj->DynCastFlyFrame() )
                    {
                        if ( bNotifySize )
                            pFlyFrame->InvalidateSize_();
                        // #115759# - no invalidation of
                        // position for as-character anchored objects.
                        if ( !bAnchoredAsChar )
                        {
                            pFlyFrame->InvalidatePos_();
                        }
                        pFlyFrame->Invalidate_();
                    }
                    else if ( dynamic_cast<const SwAnchoredDrawObject*>( pObj) !=  nullptr )
                    {
                        // #115759# - no invalidation of
                        // position for as-character anchored objects.
                        if ( !bAnchoredAsChar )
                        {
                            pObj->InvalidateObjPos();
                        }
                    }
                    else
                    {
                        OSL_FAIL( "<SwContentNotify::~SwContentNotify()> - unknown anchored object type." );
                    }
                }
            }
        }
    }
    else if( mpFrame->IsTextFrame() && mbValidSize != mpFrame->isFrameAreaSizeValid() )
    {
        SwRootFrame *pRootFrame = mpFrame->getRootFrame();
        if( pRootFrame && pRootFrame->IsAnyShellAccessible() &&
            pRootFrame->GetCurrShell() )
        {
            pRootFrame->GetCurrShell()->Imp()->InvalidateAccessibleFrameContent( mpFrame );
        }
    }

    // #i9046# Automatic frame width
    SwFlyFrame* pFly = nullptr;
    // #i35879# Do not trust the inf flags. pFrame does not
    // necessarily have to have an upper!
    if ( mpFrame->IsFlyFrame() || nullptr == ( pFly = mpFrame->ImplFindFlyFrame() ))
        return;

    // #i61999#
    // no invalidation of columned Writer fly frames, because automatic
    // width doesn't make sense for such Writer fly frames.
    if ( !pFly->Lower() || pFly->Lower()->IsColumnFrame() )
        return;

    const SwFormatFrameSize &rFrameSz = pFly->GetFormat()->GetFrameSize();

    // This could be optimized. Basically the fly frame only has to
    // be invalidated, if the first line of pFrame (if pFrame is a content
    // frame, for other frame types it's the print area) has changed its
    // size and pFrame was responsible for the current width of pFly. On
    // the other hand, this is only rarely used and re-calculation of
    // the fly frame does not cause too much trouble. So we keep it this
    // way:
    if ( SwFrameSize::Fixed != rFrameSz.GetWidthSizeType() )
    {
        // #i50668#, #i50998# - invalidation of position
        // of as-character anchored fly frames not needed and can cause
        // layout loops
        if ( dynamic_cast<const SwFlyInContentFrame*>( pFly) ==  nullptr )
        {
            pFly->InvalidatePos();
        }
        pFly->InvalidateSize();
    }
}

SwLayNotify::SwLayNotify( SwLayoutFrame *pLayFrame ) :
    SwFrameNotify( pLayFrame ),
    m_bLowersComplete( false )
{
}

// OD 2004-05-11 #i28701# - local method to invalidate the position of all
// frames inclusive its floating screen objects, which are lowers of the given
// layout frame
static void lcl_InvalidatePosOfLowers( SwLayoutFrame& _rLayoutFrame )
{
    if( _rLayoutFrame.IsFlyFrame() && _rLayoutFrame.GetDrawObjs() )
    {
        _rLayoutFrame.InvalidateObjs( false );
    }

    SwFrame* pLowerFrame = _rLayoutFrame.Lower();
    while ( pLowerFrame )
    {
        pLowerFrame->InvalidatePos();
        if ( pLowerFrame->IsTextFrame() )
        {
            static_cast<SwTextFrame*>(pLowerFrame)->Prepare( PrepareHint::FramePositionChanged );
        }
        else if ( pLowerFrame->IsTabFrame() )
        {
            pLowerFrame->InvalidatePrt();
        }

        pLowerFrame->InvalidateObjs( false );

        pLowerFrame = pLowerFrame->GetNext();
    }
}

SwLayNotify::~SwLayNotify()
{
    SwLayoutFrame *pLay = static_cast<SwLayoutFrame*>(mpFrame);
    SwRectFnSet aRectFnSet(pLay);
    bool bNotify = false;
    if ( pLay->getFramePrintArea().SSize() != maPrt.SSize() )
    {
        if ( !IsLowersComplete() )
        {
            bool bInvaPercent;

            if ( pLay->IsRowFrame() )
            {
                bInvaPercent = true;
                tools::Long nNew = aRectFnSet.GetHeight(pLay->getFramePrintArea());
                if( nNew != aRectFnSet.GetHeight(maPrt) )
                     static_cast<SwRowFrame*>(pLay)->AdjustCells( nNew, true);
                if( aRectFnSet.GetWidth(pLay->getFramePrintArea())
                    != aRectFnSet.GetWidth(maPrt) )
                     static_cast<SwRowFrame*>(pLay)->AdjustCells( 0, false );
            }
            else
            {
                //Proportional adoption of the internal.
                //1. If the formatted is no Fly
                //2. If he contains no columns
                //3. If the Fly has a fixed height and the columns
                //   are next to be.
                //   Hoehe danebenliegen.
                //4. Never at SectionFrames.
                bool bLow;
                if( pLay->IsFlyFrame() )
                {
                    if ( pLay->Lower() )
                    {
                        bLow = !pLay->Lower()->IsColumnFrame() ||
                            aRectFnSet.GetHeight(pLay->Lower()->getFrameArea())
                             != aRectFnSet.GetHeight(pLay->getFramePrintArea());
                    }
                    else
                        bLow = false;
                }
                else if( pLay->IsSctFrame() )
                {
                    if ( pLay->Lower() )
                    {
                        if( pLay->Lower()->IsColumnFrame() && pLay->Lower()->GetNext() )
                            bLow = pLay->Lower()->getFrameArea().Height() != pLay->getFramePrintArea().Height();
                        else
                            bLow = pLay->getFramePrintArea().Width() != maPrt.Width();
                    }
                    else
                        bLow = false;
                }
                else if( pLay->IsFooterFrame() && !pLay->HasFixSize() )
                    bLow = pLay->getFramePrintArea().Width() != maPrt.Width();
                else
                    bLow = true;
                bInvaPercent = bLow;
                if ( bLow )
                {
                    pLay->ChgLowersProp( maPrt.SSize() );
                }
                // If the PrtArea has been extended, it might be possible that the chain of parts
                // can take another frame. As a result, the "possible right one" needs to be
                // invalidated. This only pays off if this or its Uppers are moveable sections.
                // A PrtArea has been extended if width or height are larger than before.
                if ( (pLay->getFramePrintArea().Height() > maPrt.Height() ||
                      pLay->getFramePrintArea().Width()  > maPrt.Width()) &&
                     (pLay->IsMoveable() || pLay->IsFlyFrame()) )
                {
                    SwFrame *pTmpFrame = pLay->Lower();
                    if ( pTmpFrame && pTmpFrame->IsFlowFrame() )
                    {
                        while ( pTmpFrame->GetNext() )
                            pTmpFrame = pTmpFrame->GetNext();
                        pTmpFrame->InvalidateNextPos();
                    }
                }
            }
            bNotify = true;
            //EXPENSIVE!! But how we do it more elegant?
            if( bInvaPercent )
                pLay->InvaPercentLowers( pLay->getFramePrintArea().Height() - maPrt.Height() );
        }
        if ( pLay->IsTabFrame() )
            //So that _only_ the shadow is drawn while resizing.
            static_cast<SwTabFrame*>(pLay)->SetComplete();
        else
        {
            const SwViewShell *pSh = pLay->getRootFrame()->GetCurrShell();
            if( !( pSh && pSh->GetViewOptions()->getBrowseMode() ) ||
                  !(pLay->GetType() & (SwFrameType::Body | SwFrameType::Page)) )
            //Thereby the subordinates are retouched clean.
            //Example problem: Take the Flys with the handles and downsize.
            //Not for body and page, otherwise it flickers when loading HTML.
                pLay->SetCompletePaint();
        }
    }
    //Notify Lower if the position has changed.
    const bool bPrtPos = aRectFnSet.PosDiff( maPrt, pLay->getFramePrintArea() );
    const bool bPos = bPrtPos || aRectFnSet.PosDiff( maFrame, pLay->getFrameArea() );
    const bool bSize = pLay->getFrameArea().SSize() != maFrame.SSize();

    if ( bPos && pLay->Lower() && !IsLowersComplete() )
    {
        pLay->Lower()->InvalidatePos();
        SwFootnoteFrame* pFtnFrame = pLay->Lower()->IsFootnoteFrame() ?
            static_cast<SwFootnoteFrame*>(pLay->Lower()) : nullptr;
        SwFrame* pFtnLower = pFtnFrame ? pFtnFrame->Lower() : nullptr;
        if (pFtnLower)
            pFtnLower->InvalidatePos();
    }

    if ( bPrtPos )
        pLay->SetCompletePaint();

    //Inform the Follower if the SSize has changed.
    if ( bSize )
    {
        if( pLay->GetNext() )
        {
            if ( pLay->GetNext()->IsLayoutFrame() )
                pLay->GetNext()->InvalidatePos_();
            else
                pLay->GetNext()->InvalidatePos();
        }
        else if( pLay->IsSctFrame() )
            pLay->InvalidateNextPos();
    }
    if ( !IsLowersComplete() &&
         !(pLay->GetType()&(SwFrameType::Fly|SwFrameType::Section) &&
            pLay->Lower() && pLay->Lower()->IsColumnFrame()) &&
         (bPos || bNotify) &&
         !(pLay->GetType() & (SwFrameType::Row|SwFrameType::Tab|SwFrameType::FtnCont|SwFrameType::Page|SwFrameType::Root)))
    {
        // #i44016# - force unlock of position of lower objects.
        // #i43913# - no unlock of position of objects,
        // if <pLay> is a cell frame, and its table frame resp. its parent table
        // frame is locked.
        // #i47458# - force unlock of position of lower objects,
        // only if position of layout frame has changed.
        bool bUnlockPosOfObjs( bPos );
        if ( bUnlockPosOfObjs && pLay->IsCellFrame() )
        {
            SwTabFrame* pTabFrame( pLay->FindTabFrame() );
            if ( pTabFrame &&
                 ( pTabFrame->IsJoinLocked() ||
                   ( pTabFrame->IsFollow() &&
                     pTabFrame->FindMaster()->IsJoinLocked() ) ) )
            {
                bUnlockPosOfObjs = false;
            }
        }
        // #i49383# - check for footnote frame, if unlock
        // of position of lower objects is allowed.
        else if ( bUnlockPosOfObjs && pLay->IsFootnoteFrame() )
        {
            bUnlockPosOfObjs = static_cast<SwFootnoteFrame*>(pLay)->IsUnlockPosOfLowerObjs();
        }
        // #i51303# - no unlock of object positions for sections
        else if ( bUnlockPosOfObjs && pLay->IsSctFrame() )
        {
            bUnlockPosOfObjs = false;
        }
        pLay->NotifyLowerObjs( bUnlockPosOfObjs );
    }
    if ( bPos && pLay->IsFootnoteFrame() && pLay->Lower() )
    {
        // OD 2004-05-11 #i28701#
        ::lcl_InvalidatePosOfLowers( *pLay );
    }
    if( ( bPos || bSize ) && pLay->IsFlyFrame() && static_cast<SwFlyFrame*>(pLay)->GetAnchorFrame()
          && static_cast<SwFlyFrame*>(pLay)->GetAnchorFrame()->IsFlyFrame() )
        static_cast<SwFlyFrame*>(pLay)->AnchorFrame()->InvalidateSize();
}

SwFlyNotify::SwFlyNotify( SwFlyFrame *pFlyFrame ) :
    SwLayNotify( pFlyFrame ),
    // #115759# - keep correct page frame - the page frame
    // the Writer fly frame is currently registered at.
    pOldPage( pFlyFrame->GetPageFrame() ),
    aFrameAndSpace( pFlyFrame->GetObjRectWithSpaces() )
{
}

SwFlyNotify::~SwFlyNotify()
{
    SwFlyFrame *pFly = static_cast<SwFlyFrame*>(mpFrame);
    if ( pFly->IsNotifyBack() )
    {
        SwViewShell *pSh = pFly->getRootFrame()->GetCurrShell();
        SwViewShellImp *pImp = pSh ? pSh->Imp() : nullptr;
        if ( !pImp || !pImp->IsAction() || !pImp->GetLayAction().IsAgain() )
        {
            //If in the LayAction the IsAgain is set it can be
            //that the old page is destroyed in the meantime!
            ::Notify( pFly, pOldPage, aFrameAndSpace, &maPrt );
            // #i35640# - additional notify anchor text frame,
            // if Writer fly frame has changed its page
            if ( pFly->GetAnchorFrame()->IsTextFrame() &&
                 pFly->GetPageFrame() != pOldPage )
            {
                pFly->AnchorFrame()->Prepare( PrepareHint::FlyFrameLeave );
            }
        }
        pFly->ResetNotifyBack();
    }

    //Have the size or the position changed,
    //so should the view know this.
    SwRectFnSet aRectFnSet(pFly);
    const bool bPosChgd = aRectFnSet.PosDiff( maFrame, pFly->getFrameArea() );
    const bool bFrameChgd = pFly->getFrameArea().SSize() != maFrame.SSize();
    const bool bPrtChgd = maPrt != pFly->getFramePrintArea();
    if ( bPosChgd || bFrameChgd || bPrtChgd )
    {
        pFly->NotifyDrawObj();
    }
    if ( bPosChgd && maFrame.Pos().X() != FAR_AWAY )
    {
        // OD 2004-05-10 #i28701# - no direct move of lower Writer fly frames.
        // reason: New positioning and alignment (e.g. to-paragraph anchored,
        // but aligned at page) are introduced.
        // <SwLayNotify::~SwLayNotify()> takes care of invalidation of lower
        // floating screen objects by calling method <SwLayoutFrame::NotifyLowerObjs()>.

        if ( pFly->IsFlyAtContentFrame() )
        {
            SwFrame *pNxt = pFly->AnchorFrame()->FindNext();
            while (pNxt)
            {
                pNxt->InvalidatePos();
                if (!pNxt->IsSctFrame())
                {
                    break;
                }
                // invalidating pos of a section frame doesn't have much
                // effect, so try again with its lower
                pNxt = static_cast<SwSectionFrame*>(pNxt)->Lower();
            }
        }

        // #i26945# - notify anchor.
        // Needed for negative positioned Writer fly frames
        if ( pFly->GetAnchorFrame()->IsTextFrame() )
        {
            pFly->AnchorFrame()->Prepare( PrepareHint::FlyFrameLeave );
        }
    }

    // OD 2004-05-13 #i28701#
    // #i45180# - no adjustment of layout process flags and
    // further notifications/invalidations, if format is called by grow/shrink
    if ( !pFly->ConsiderObjWrapInfluenceOnObjPos() )
        return;
    if (pFly->IsFlyFreeFrame())
    {
        if (static_cast<SwFlyFreeFrame*>(pFly)->IsNoMoveOnCheckClip())
            return;
    }

    // #i54138# - suppress restart of the layout process
    // on changed frame height.
    // Note: It doesn't seem to be necessary and can cause layout loops.
    if ( bPosChgd )
    {
        // indicate a restart of the layout process
        pFly->SetRestartLayoutProcess( true );
    }
    else
    {
        // lock position
        pFly->LockPosition();
    }

    if ( pFly->ConsiderForTextWrap() )
        return;

    // indicate that object has to be considered for text wrap
    pFly->SetConsiderForTextWrap( true );
    // invalidate 'background' in order to allow its 'background'
    // to wrap around it.
    pFly->NotifyBackground( pFly->GetPageFrame(),
                            pFly->GetObjRectWithSpaces(),
                            PrepareHint::FlyFrameArrive );
    // invalidate position of anchor frame in order to force
    // a re-format of the anchor frame, which also causes a
    // re-format of the invalid previous frames of the anchor frame.
    pFly->AnchorFrame()->InvalidatePos();
}

SwContentNotify::SwContentNotify( SwContentFrame *pContentFrame ) :
    SwFrameNotify( pContentFrame ),
    // OD 08.01.2004 #i11859#
    mbChkHeightOfLastLine( false ),
    mnHeightOfLastLine( 0 ),
    // OD 2004-02-26 #i25029#
    mbInvalidatePrevPrtArea( false ),
    mbBordersJoinedWithPrev( false )
{
    // OD 08.01.2004 #i11859#
    if ( !pContentFrame->IsTextFrame() )
        return;

    SwTextFrame* pTextFrame = static_cast<SwTextFrame*>(pContentFrame);
    if (!pTextFrame->GetDoc().getIDocumentSettingAccess().get(DocumentSettingId::OLD_LINE_SPACING))
    {
        const SvxLineSpacingItem &rSpace = pTextFrame->GetAttrSet()->GetLineSpacing();
        if ( rSpace.GetInterLineSpaceRule() == SvxInterLineSpaceRule::Prop )
        {
            mbChkHeightOfLastLine = true;
            mnHeightOfLastLine = pTextFrame->GetHeightOfLastLine();
        }
    }
}

void SwContentNotify::ImplDestroy()
{
    SwContentFrame *pCnt = static_cast<SwContentFrame*>(mpFrame);
    if ( bSetCompletePaintOnInvalidate )
        pCnt->SetCompletePaint();

    SwRectFnSet aRectFnSet(pCnt);
    if ( pCnt->IsInTab() && ( aRectFnSet.PosDiff( pCnt->getFrameArea(), maFrame ) ||
                             pCnt->getFrameArea().SSize() != maFrame.SSize()))
    {
        SwLayoutFrame* pCell = pCnt->GetUpper();
        while( !pCell->IsCellFrame() && pCell->GetUpper() )
            pCell = pCell->GetUpper();
        OSL_ENSURE( pCell->IsCellFrame(), "Where's my cell?" );
        if ( text::VertOrientation::NONE != pCell->GetFormat()->GetVertOrient().GetVertOrient() )
            pCell->InvalidatePrt(); //for the vertical align.
    }

    // OD 2004-02-26 #i25029#
    if ( mbInvalidatePrevPrtArea && mbBordersJoinedWithPrev &&
         pCnt->IsTextFrame() &&
         !pCnt->IsFollow() && !pCnt->GetIndPrev() )
    {
        // determine previous frame
        SwFrame* pPrevFrame = pCnt->FindPrev();
        // skip empty section frames and hidden text frames
        {
            while ( pPrevFrame &&
                    ( ( pPrevFrame->IsSctFrame() &&
                        !static_cast<SwSectionFrame*>(pPrevFrame)->GetSection() ) ||
                      ( pPrevFrame->IsTextFrame() &&
                        static_cast<SwTextFrame*>(pPrevFrame)->IsHiddenNow() ) ) )
            {
                pPrevFrame = pPrevFrame->FindPrev();
            }
        }

        // Invalidate printing area of found previous frame
        if ( pPrevFrame )
        {
            if ( pPrevFrame->IsSctFrame() )
            {
                if ( pCnt->IsInSct() )
                {
                    // Note: found previous frame is a section frame and
                    //       <pCnt> is also inside a section.
                    //       Thus due to <mbBordersJoinedWithPrev>,
                    //       <pCnt> had joined its borders/shadow with the
                    //       last content of the found section.
                    // Invalidate printing area of last content in found section.
                    SwFrame* pLstContentOfSctFrame =
                            static_cast<SwSectionFrame*>(pPrevFrame)->FindLastContent();
                    if ( pLstContentOfSctFrame )
                    {
                        pLstContentOfSctFrame->InvalidatePrt();
                    }
                }
            }
            else
            {
                pPrevFrame->InvalidatePrt();
            }
        }
    }

    const bool bFirst = aRectFnSet.GetWidth(maFrame) == 0;

    if ( pCnt->IsNoTextFrame() )
    {
        //Active PlugIn's or OLE-Objects should know something of the change
        //thereby they move their window appropriate.
        SwViewShell *pSh  = pCnt->getRootFrame()->GetCurrShell();
        if ( pSh )
        {
            SwOLENode *const pNd(static_cast<SwNoTextFrame*>(pCnt)->GetNode()->GetOLENode());
            if (nullptr != pNd &&
                 (pNd->GetOLEObj().IsOleRef() ||
                  pNd->IsOLESizeInvalid()) )
            {
                const bool bNoTextFramePrtAreaChanged =
                        ( maPrt.SSize().Width() != 0 &&
                          maPrt.SSize().Height() != 0 ) &&
                        maPrt.SSize() != pCnt->getFramePrintArea().SSize();
                OSL_ENSURE( pCnt->IsInFly(), "OLE not in FlyFrame" );
                SwFlyFrame *pFly = pCnt->FindFlyFrame();
                svt::EmbeddedObjectRef& xObj = pNd->GetOLEObj().GetObject();
                SwFEShell *pFESh = nullptr;
                for(SwViewShell& rCurrentShell : pSh->GetRingContainer())
                {   if ( dynamic_cast<const SwCursorShell*>( &rCurrentShell) !=  nullptr )
                    {
                        pFESh = static_cast<SwFEShell*>(&rCurrentShell);
                        // #108369#: Here used to be the condition if (!bFirst).
                        // I think this should mean "do not call CalcAndSetScale"
                        // if the frame is formatted for the first time.
                        // Unfortunately this is not valid anymore since the
                        // SwNoTextFrame already gets a width during CalcLowerPreps.
                        // Nevertheless, the indention of !bFirst seemed to be
                        // to assure that the OLE objects have already been notified
                        // if necessary before calling CalcAndSetScale.
                        // So I replaced !bFirst by !IsOLESizeInvalid. There is
                        // one additional problem specific to the word import:
                        // The layout is calculated _before_ calling PrtOLENotify,
                        // and the OLE objects are not invalidated during import.
                        // Therefore I added the condition !IsUpdateExpField,
                        // have a look at the occurrence of CalcLayout in
                        // uiview/view.cxx.
                        if ( !pNd->IsOLESizeInvalid() &&
                             !pSh->GetDoc()->getIDocumentState().IsUpdateExpField() )
                            pFESh->CalcAndSetScale( xObj,
                                                    &pFly->getFramePrintArea(), &pFly->getFrameArea(),
                                                    bNoTextFramePrtAreaChanged );
                    }
                }

                if ( pFESh && pNd->IsOLESizeInvalid() )
                {
                    pNd->SetOLESizeInvalid( false );
                    pFESh->CalcAndSetScale( xObj ); // create client
                }
            }
            // ditto animated graphics
            if ( getFrameArea().HasArea() && static_cast<SwNoTextFrame*>(pCnt)->HasAnimation() )
            {
                static_cast<SwNoTextFrame*>(pCnt)->StopAnimation();
                pSh->InvalidateWindows( getFrameArea() );
            }
        }
    }

    if ( bFirst )
    {
        pCnt->SetRetouche();    //fix(13870)

        SwDoc& rDoc = pCnt->IsTextFrame()
            ? static_cast<SwTextFrame*>(pCnt)->GetDoc()
            : static_cast<SwNoTextFrame*>(pCnt)->GetNode()->GetDoc();
        if ( !rDoc.GetSpzFrameFormats()->empty() &&
             rDoc.DoesContainAtPageObjWithContentAnchor() && !rDoc.getIDocumentState().IsNewDoc() )
        {
            // If certain import filters for foreign file format import
            // AT_PAGE anchored objects, the corresponding page number is
            // typically not known. In this case the content position is
            // stored at which the anchored object is found in the
            // imported document.
            // When this content is formatted it is the time at which
            // the page is known. Thus, this data can be corrected now.

            const SwPageFrame *pPage = nullptr;
            SwFrameFormats *pTable = rDoc.GetSpzFrameFormats();

            for ( size_t i = 0; i < pTable->size(); ++i )
            {
                SwFrameFormat *pFormat = (*pTable)[i];
                const SwFormatAnchor &rAnch = pFormat->GetAnchor();
                if ( RndStdIds::FLY_AT_PAGE != rAnch.GetAnchorId() ||
                     rAnch.GetContentAnchor() == nullptr )
                {
                    continue;
                }

                if (FrameContainsNode(*pCnt, rAnch.GetContentAnchor()->nNode.GetIndex()))
                {
                    OSL_FAIL( "<SwContentNotify::~SwContentNotify()> - to page anchored object with content position." );
                    if ( !pPage )
                    {
                        pPage = pCnt->FindPageFrame();
                    }
                    SwFormatAnchor aAnch( rAnch );
                    aAnch.SetAnchor( nullptr );
                    aAnch.SetPageNum( pPage->GetPhyPageNum() );
                    pFormat->SetFormatAttr( aAnch );
                    if ( RES_DRAWFRMFMT != pFormat->Which() )
                    {
                        pFormat->MakeFrames();
                    }
                }
            }
        }
    }

    // OD 12.01.2004 #i11859# - invalidate printing area of following frame,
    //  if height of last line has changed.
    if ( pCnt->IsTextFrame() && mbChkHeightOfLastLine )
    {
        if ( mnHeightOfLastLine != static_cast<SwTextFrame*>(pCnt)->GetHeightOfLastLine() )
        {
            pCnt->InvalidateNextPrtArea();
        }
    }

    // #i44049#
    if ( pCnt->IsTextFrame() && aRectFnSet.PosDiff( maFrame, pCnt->getFrameArea() ) )
    {
        pCnt->InvalidateObjs();
    }

    // #i43255# - move code to invalidate at-character
    // anchored objects due to a change of its anchor character from
    // method <SwTextFrame::Format(..)>.
    if ( !pCnt->IsTextFrame() )
        return;

    SwTextFrame* pMasterFrame = pCnt->IsFollow()
                           ? static_cast<SwTextFrame*>(pCnt)->FindMaster()
                           : static_cast<SwTextFrame*>(pCnt);
    if ( pMasterFrame && !pMasterFrame->IsFlyLock() &&
         pMasterFrame->GetDrawObjs() )
    {
        SwSortedObjs* pObjs = pMasterFrame->GetDrawObjs();
        for (SwAnchoredObject* pAnchoredObj : *pObjs)
        {
            if ( pAnchoredObj->GetFrameFormat().GetAnchor().GetAnchorId()
                    == RndStdIds::FLY_AT_CHAR )
            {
                pAnchoredObj->CheckCharRectAndTopOfLine( !pMasterFrame->IsEmpty() );
            }
        }
    }
}

SwContentNotify::~SwContentNotify()
{
    suppress_fun_call_w_exception(ImplDestroy());
}

// note this *cannot* be static because it's a friend
void AppendObj(SwFrame *const pFrame, SwPageFrame *const pPage, SwFrameFormat *const pFormat, const SwFormatAnchor & rAnch)
{
            const bool bFlyAtFly = rAnch.GetAnchorId() == RndStdIds::FLY_AT_FLY; // LAYER_IMPL
            //Is a frame or a SdrObject described?
            const bool bSdrObj = RES_DRAWFRMFMT == pFormat->Which();
            // OD 23.06.2003 #108784# - append also drawing objects anchored
            // as character.
            const bool bDrawObjInContent = bSdrObj &&
                                         (rAnch.GetAnchorId() == RndStdIds::FLY_AS_CHAR);

            if( !(bFlyAtFly ||
                (rAnch.GetAnchorId() == RndStdIds::FLY_AT_PARA) ||
                (rAnch.GetAnchorId() == RndStdIds::FLY_AT_CHAR) ||
                bDrawObjInContent) )
                return;

            SdrObject* pSdrObj = nullptr;
            if ( bSdrObj && nullptr == (pSdrObj = pFormat->FindSdrObject()) )
            {
                OSL_ENSURE( !bSdrObj, "DrawObject not found." );
                pFormat->GetDoc()->DelFrameFormat( pFormat );
                return;
            }
            if ( pSdrObj )
            {
                if ( !pSdrObj->getSdrPageFromSdrObject() )
                {
                    pFormat->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0)->
                            InsertObject(pSdrObj, pSdrObj->GetOrdNumDirect());
                }

                SwDrawContact* pNew =
                    static_cast<SwDrawContact*>(GetUserCall( pSdrObj ));
                if ( !pNew->GetAnchorFrame() )
                {
                    pFrame->AppendDrawObj( *(pNew->GetAnchoredObj( nullptr )) );
                }
                // OD 19.06.2003 #108784# - add 'virtual' drawing object,
                // if necessary. But control objects have to be excluded.
                else if ( !::CheckControlLayer( pSdrObj ) &&
                          pNew->GetAnchorFrame() != pFrame &&
                          !pNew->GetDrawObjectByAnchorFrame( *pFrame ) )
                {
                    SwDrawVirtObj* pDrawVirtObj = pNew->AddVirtObj(*pFrame);
                    pFrame->AppendDrawObj( *(pNew->GetAnchoredObj( pDrawVirtObj )) );

                    pDrawVirtObj->ActionChanged();
                }
            }
            else
            {
                SwFlyFrame *pFly;
                if( bFlyAtFly )
                    pFly = new SwFlyLayFrame( static_cast<SwFlyFrameFormat*>(pFormat), pFrame, pFrame );
                else
                    pFly = new SwFlyAtContentFrame( static_cast<SwFlyFrameFormat*>(pFormat), pFrame, pFrame );
                pFly->Lock();
                pFrame->AppendFly( pFly );
                pFly->Unlock();
                if ( pPage )
                    ::RegistFlys( pPage, pFly );
            }
}

static bool IsShown(SwNodeOffset const nIndex,
    const SwFormatAnchor & rAnch,
    std::vector<sw::Extent>::const_iterator const*const pIter,
    std::vector<sw::Extent>::const_iterator const*const pEnd,
    SwTextNode const*const pFirstNode, SwTextNode const*const pLastNode)
{
    assert(!pIter || *pIter == *pEnd || (*pIter)->pNode->GetIndex() == nIndex);
    SwPosition const& rAnchor(*rAnch.GetContentAnchor());
    if (rAnchor.nNode.GetIndex() != nIndex)
    {
        return false;
    }
    if (rAnch.GetAnchorId() == RndStdIds::FLY_AT_PARA)
    {
        return pIter == nullptr // not merged
            || pIter != pEnd    // at least one char visible in node
            || !IsSelectFrameAnchoredAtPara(rAnchor,
                    SwPosition(const_cast<SwTextNode&>(*pFirstNode), 0),
                    SwPosition(const_cast<SwTextNode&>(*pLastNode), pLastNode->Len()));
    }
    if (pIter)
    {
        // note: frames are not sorted by anchor position.
        assert(pEnd);
        assert(pFirstNode);
        assert(pLastNode);
        assert(rAnch.GetAnchorId() != RndStdIds::FLY_AT_FLY);
        for (auto iter = *pIter; iter != *pEnd; ++iter)
        {
            assert(iter->nStart != iter->nEnd); // TODO possible?
            assert(iter->pNode->GetIndex() == nIndex);
            if (rAnchor.nContent.GetIndex() < iter->nStart)
            {
                return false;
            }
            if (rAnch.GetAnchorId() == RndStdIds::FLY_AT_CHAR)
            {
                // if there is an extent then obviously the node was not
                // deleted fully...
                // show if start <= pos <= end
                // *or* if first-node/0  *and* not StartOfSection
                // *or* if last-node/Len *and* not EndOfSection

                // first determine the extent to compare to, then
                // construct start/end positions for the deletion *before* the
                // extent and compare once.
                // the interesting corner cases are on the edge of the extent!
                // no need to check for > the last extent because those
                // are never visible.
                if (rAnchor.nContent.GetIndex() <= iter->nEnd)
                {
                    if (iter->nStart == 0)
                    {
                        return true;
                    }
                    else
                    {
                        SwPosition const start(
                            const_cast<SwTextNode&>(
                                iter == *pIter
                                    ? *pFirstNode // simplification
                                    : *iter->pNode),
                            iter == *pIter // first extent?
                                ? iter->pNode == pFirstNode
                                    ? 0 // at start of 1st node
                                    : pFirstNode->Len() // previous node; simplification but should get right result
                                : (iter-1)->nEnd); // previous extent
                        SwPosition const end(*iter->pNode, iter->nStart);
                        return !IsDestroyFrameAnchoredAtChar(rAnchor, start, end);
                    }
                }
                else if (iter == *pEnd - 1) // special case: after last extent
                {
                    if (iter->nEnd == iter->pNode->Len())
                    {
                        return true; // special case: end of node
                    }
                    else
                    {
                        SwPosition const start(*iter->pNode, iter->nEnd);
                        SwPosition const end(
                            const_cast<SwTextNode&>(*pLastNode), // simplification
                            iter->pNode == pLastNode
                                ? iter->pNode->Len()
                                : 0);
                        return !IsDestroyFrameAnchoredAtChar(rAnchor, start, end);
                    }
                }
            }
            else
            {
                assert(rAnch.GetAnchorId() == RndStdIds::FLY_AS_CHAR);
                // for AS_CHAR obviously must be <
                if (rAnchor.nContent.GetIndex() < iter->nEnd)
                {
                    return true;
                }
            }
        }
        return false;
    }
    else
    {
        return true;
    }
}

void RemoveHiddenObjsOfNode(SwTextNode const& rNode,
    std::vector<sw::Extent>::const_iterator const*const pIter,
    std::vector<sw::Extent>::const_iterator const*const pEnd,
    SwTextNode const*const pFirstNode, SwTextNode const*const pLastNode)
{
    std::vector<SwFrameFormat*> const & rFlys(rNode.GetAnchoredFlys());
    for (SwFrameFormat * pFrameFormat : rFlys)
    {
        SwFormatAnchor const& rAnchor = pFrameFormat->GetAnchor();
        if (rAnchor.GetAnchorId() == RndStdIds::FLY_AT_CHAR
            || (rAnchor.GetAnchorId() == RndStdIds::FLY_AS_CHAR
                && RES_DRAWFRMFMT == pFrameFormat->Which()))
        {
            assert(rAnchor.GetContentAnchor()->nNode.GetIndex() == rNode.GetIndex());
            if (!IsShown(rNode.GetIndex(), rAnchor, pIter, pEnd, pFirstNode, pLastNode))
            {
                pFrameFormat->DelFrames();
            }
        }
    }
}

void AppendObjsOfNode(SwFrameFormats const*const pTable, SwNodeOffset const nIndex,
    SwFrame *const pFrame, SwPageFrame *const pPage, SwDoc *const pDoc,
    std::vector<sw::Extent>::const_iterator const*const pIter,
    std::vector<sw::Extent>::const_iterator const*const pEnd,
    SwTextNode const*const pFirstNode, SwTextNode const*const pLastNode)
{
#if OSL_DEBUG_LEVEL > 0
    std::vector<SwFrameFormat*> checkFormats;
    for ( size_t i = 0; i < pTable->size(); ++i )
    {
        SwFrameFormat *pFormat = (*pTable)[i];
        const SwFormatAnchor &rAnch = pFormat->GetAnchor();
        if ( rAnch.GetContentAnchor() &&
            IsShown(nIndex, rAnch, pIter, pEnd, pFirstNode, pLastNode))
        {
            checkFormats.push_back( pFormat );
        }
    }
#else
    (void)pTable;
#endif

    SwNode const& rNode(*pDoc->GetNodes()[nIndex]);
    std::vector<SwFrameFormat*> const & rFlys(rNode.GetAnchoredFlys());
    for (size_t it = 0; it != rFlys.size(); )
    {
        SwFrameFormat *const pFormat = rFlys[it];
        const SwFormatAnchor &rAnch = pFormat->GetAnchor();
        if ( rAnch.GetContentAnchor() &&
            IsShown(nIndex, rAnch, pIter, pEnd, pFirstNode, pLastNode))
        {
#if OSL_DEBUG_LEVEL > 0
            std::vector<SwFrameFormat*>::iterator checkPos = std::find( checkFormats.begin(), checkFormats.end(), pFormat );
            assert( checkPos != checkFormats.end());
            checkFormats.erase( checkPos );
#endif
            AppendObj(pFrame, pPage, pFormat, rAnch);
        }
        ++it;
    }

#if OSL_DEBUG_LEVEL > 0
    assert( checkFormats.empty());
#endif
}


void AppendObjs(const SwFrameFormats *const pTable, SwNodeOffset const nIndex,
        SwFrame *const pFrame, SwPageFrame *const pPage, SwDoc *const pDoc)
{
    if (pFrame->IsTextFrame())
    {
        SwTextFrame const*const pTextFrame(static_cast<SwTextFrame const*>(pFrame));
        if (sw::MergedPara const*const pMerged = pTextFrame->GetMergedPara())
        {
            std::vector<sw::Extent>::const_iterator iterFirst(pMerged->extents.begin());
            std::vector<sw::Extent>::const_iterator iter(iterFirst);
            SwTextNode const* pNode(pMerged->pFirstNode);
            for ( ; ; ++iter)
            {
                if (iter == pMerged->extents.end()
                    || iter->pNode != pNode)
                {
                    AppendObjsOfNode(pTable, pNode->GetIndex(), pFrame, pPage, pDoc,
                        &iterFirst, &iter, pMerged->pFirstNode, pMerged->pLastNode);
                    SwNodeOffset const until = iter == pMerged->extents.end()
                        ? pMerged->pLastNode->GetIndex() + 1
                        : iter->pNode->GetIndex();
                    for (SwNodeOffset i = pNode->GetIndex() + 1; i < until; ++i)
                    {
                        // let's show at-para flys on nodes that contain start/end of
                        // redline too, even if there's no text there
                        SwNode const*const pTmp(pNode->GetNodes()[i]);
                        if (pTmp->GetRedlineMergeFlag() == SwNode::Merge::NonFirst)
                        {
                            AppendObjsOfNode(pTable, pTmp->GetIndex(), pFrame, pPage, pDoc, &iter, &iter, pMerged->pFirstNode, pMerged->pLastNode);
                        }
                    }
                    if (iter == pMerged->extents.end())
                    {
                        break;
                    }
                    pNode = iter->pNode;
                    iterFirst = iter;
                }
            }
        }
        else
        {
            return AppendObjsOfNode(pTable, nIndex, pFrame, pPage, pDoc, nullptr, nullptr, nullptr, nullptr);
        }
    }
    else
    {
        return AppendObjsOfNode(pTable, nIndex, pFrame, pPage, pDoc, nullptr, nullptr, nullptr, nullptr);
    }
}

bool IsAnchoredObjShown(SwTextFrame const& rFrame, SwFormatAnchor const& rAnchor)
{
    assert(rAnchor.GetAnchorId() == RndStdIds::FLY_AT_PARA ||
           rAnchor.GetAnchorId() == RndStdIds::FLY_AT_CHAR ||
           rAnchor.GetAnchorId() == RndStdIds::FLY_AS_CHAR);
    bool ret(true);
    if (auto const pMergedPara = rFrame.GetMergedPara())
    {
        ret = false;
        auto const pAnchor(rAnchor.GetContentAnchor());
        auto iterFirst(pMergedPara->extents.cbegin());
        if (iterFirst == pMergedPara->extents.end()
            && (rAnchor.GetAnchorId() == RndStdIds::FLY_AT_PARA
                || rAnchor.GetAnchorId() == RndStdIds::FLY_AT_CHAR))
        {
            ret = (&pAnchor->nNode.GetNode() == pMergedPara->pFirstNode
                    && (rAnchor.GetAnchorId() == RndStdIds::FLY_AT_PARA
                        || pAnchor->nContent == 0))
                || (&pAnchor->nNode.GetNode() == pMergedPara->pLastNode
                    && (rAnchor.GetAnchorId() == RndStdIds::FLY_AT_PARA
                        || pAnchor->nContent == pMergedPara->pLastNode->Len()));
        }
        auto iter(iterFirst);
        SwTextNode const* pNode(pMergedPara->pFirstNode);
        for ( ; ; ++iter)
        {
            if (iter == pMergedPara->extents.end()
                || iter->pNode != pNode)
            {
                assert(pNode->GetRedlineMergeFlag() != SwNode::Merge::Hidden);
                if (pNode == &pAnchor->nNode.GetNode())
                {
                    ret = IsShown(pNode->GetIndex(), rAnchor, &iterFirst, &iter,
                            pMergedPara->pFirstNode, pMergedPara->pLastNode);
                    break;
                }
                if (iter == pMergedPara->extents.end())
                {
                    break;
                }
                pNode = iter->pNode;
                if (pAnchor->nNode.GetIndex() < pNode->GetIndex())
                {
                    break;
                }
                iterFirst = iter;
            }
        }
    }
    return ret;
}

void AppendAllObjs(const SwFrameFormats* pTable, const SwFrame* pSib)
{
    //Connecting of all Objects, which are described in the SpzTable with the
    //layout.

    boost::circular_buffer<SwFrameFormat*> vFormatsToConnect(pTable->size());
    for(const auto& pFormat : *pTable)
    {
        const auto& rAnch = pFormat->GetAnchor();
        // Formats can still remain, because we neither use character bound
        // frames nor objects which are anchored to character bounds.
        if ((rAnch.GetAnchorId() != RndStdIds::FLY_AT_PAGE) && (rAnch.GetAnchorId() != RndStdIds::FLY_AS_CHAR))
        {
            auto pContentAnchor = rAnch.GetContentAnchor();
            // formats in header/footer have no dependencies
            if(pContentAnchor && pFormat->GetDoc()->IsInHeaderFooter(pContentAnchor->nNode))
                pFormat->MakeFrames();
            else
                vFormatsToConnect.push_back(pFormat);
        }
    }
    const SwRootFrame* pRoot = pSib ? pSib->getRootFrame() : nullptr;
    const SwFrameFormat* pFirstRequeued(nullptr);
    while(!vFormatsToConnect.empty())
    {
        auto& pFormat = vFormatsToConnect.front();
        bool isConnected(false);
        pFormat->CallSwClientNotify(sw::GetObjectConnectedHint(isConnected, pRoot));
        if(!isConnected)
        {
            pFormat->MakeFrames();
            pFormat->CallSwClientNotify(sw::GetObjectConnectedHint(isConnected, pRoot));
        }
        // do this *before* push_back! the circular_buffer can be "full"!
        vFormatsToConnect.pop_front();
        if (!isConnected)
        {
            if(pFirstRequeued == pFormat)
                // If nothing happens anymore we can stop.
                break;
            if(!pFirstRequeued)
                pFirstRequeued = pFormat;
            assert(!vFormatsToConnect.full());
            vFormatsToConnect.push_back(pFormat);
        }
        else
        {
            pFirstRequeued = nullptr;
        }
    }
}

namespace sw {

void RecreateStartTextFrames(SwTextNode & rNode)
{
    std::vector<SwTextFrame*> frames;
    SwIterator<SwTextFrame, SwTextNode, sw::IteratorMode::UnwrapMulti> aIter(rNode);
    for (SwTextFrame* pFrame = aIter.First(); pFrame; pFrame = aIter.Next())
    {
        if (pFrame->getRootFrame()->HasMergedParas())
        {
            frames.push_back(pFrame);
        }
    }
    auto eMode(sw::FrameMode::Existing);
    for (SwTextFrame * pFrame : frames)
    {
        // SplitNode could have moved the original frame to the start node
        // & created a new one on end, or could have created new frame on
        // start node... grab start node's frame and recreate MergedPara.
        SwTextNode & rFirstNode(pFrame->GetMergedPara()
            ? *pFrame->GetMergedPara()->pFirstNode
            : rNode);
        assert(rFirstNode.GetIndex() <= rNode.GetIndex());
        // clear old one first to avoid DelFrames confusing updates & asserts...
        pFrame->SetMergedPara(nullptr);
        pFrame->SetMergedPara(sw::CheckParaRedlineMerge(
                    *pFrame, rFirstNode, eMode));
        eMode = sw::FrameMode::New; // Existing is not idempotent!
        // note: this may or may not delete frames on the end node
    }
}

} // namespace sw

/** local method to set 'working' position for newly inserted frames

    OD 12.08.2003 #i17969#
*/
static void lcl_SetPos( SwFrame&             _rNewFrame,
                 const SwLayoutFrame& _rLayFrame )
{
    SwRectFnSet aRectFnSet(&_rLayFrame);
    SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(_rNewFrame);
    aRectFnSet.SetPos( aFrm, aRectFnSet.GetPos(_rLayFrame.getFrameArea()) );

    // move position by one SwTwip in text flow direction in order to get
    // notifications for a new calculated position after its formatting.
    if ( aRectFnSet.IsVert() )
    {
        aFrm.Pos().AdjustX( -1 );
    }
    else
    {
        aFrm.Pos().AdjustY(1 );
    }
}

void InsertCnt_( SwLayoutFrame *pLay, SwDoc *pDoc,
                             SwNodeOffset nIndex, bool bPages, SwNodeOffset nEndIndex,
                             SwFrame *pPrv, sw::FrameMode const eMode )
{
    pDoc->getIDocumentTimerAccess().BlockIdling();
    SwRootFrame* pLayout = pLay->getRootFrame();
    const bool bOldCallbackActionEnabled = pLayout && pLayout->IsCallbackActionEnabled();
    if( bOldCallbackActionEnabled )
        pLayout->SetCallbackActionEnabled( false );

    //In the generation of the Layout bPages=true will be handed over.
    //Then will be new pages generated all x paragraphs already times in advance.
    //On breaks and/or pagedescriptorchanges the corresponding will be generated
    //immediately.
    //The advantage is, that on one hand already a nearly realistic number of
    //pages are created, but above all there are no almost endless long chain
    //of paragraphs, which must be moved expensively until it reaches a tolerable
    //reduced level.
    //We'd like to think that 20 Paragraphs fit on one page.
    //So that it does not become in extreme situations so violent we calculate depending
    //on the node something to it.
    //If in the DocStatistic a usable given pagenumber
    //(Will be cared for while writing), so it will be presumed that this will be
    //number of pages.
    const bool bStartPercent = bPages && !nEndIndex;

    SwPageFrame *pPage = pLay->FindPageFrame();
    const SwFrameFormats *pTable = pDoc->GetSpzFrameFormats();
    SwFrame       *pFrame = nullptr;
    std::unique_ptr<SwActualSection> pActualSection;
    std::unique_ptr<SwLayHelper> pPageMaker;

    //If the layout will be created (bPages == true) we do head on the progress
    //Flys and DrawObjects are not connected immediately, this
    //happens only at the end of the function.
    if ( bPages )
    {
        // Attention: the SwLayHelper class uses references to the content-,
        // page-, layout-frame etc. and may change them!
        pPageMaker.reset(new SwLayHelper( pDoc, pFrame, pPrv, pPage, pLay,
                pActualSection, nIndex, SwNodeOffset(0) == nEndIndex ));
        if( bStartPercent )
        {
            const sal_uLong nPageCount = pPageMaker->CalcPageCount();
            if( nPageCount )
                bObjsDirect = false;
        }
    }
    else
        pPageMaker = nullptr;

    if( pLay->IsInSct() &&
        ( pLay->IsSctFrame() || pLay->GetUpper() ) ) // Hereby will newbies
            // be intercepted, of which flags could not determined yet,
            // for e.g. while inserting a table
    {
        SwSectionFrame* pSct = pLay->FindSctFrame();
        // If content will be inserted in a footnote, which in a column area,
        // the column area it is not allowed to be broken up.
        // Only if in the inner of the footnote lies an area, is this a candidate
        // for pActualSection.
        // The same applies for areas in tables, if inside the table will be
        // something inserted, it's only allowed to break up areas, which
        // lies in the inside also.
        if( ( !pLay->IsInFootnote() || pSct->IsInFootnote() ) &&
            ( !pLay->IsInTab() || pSct->IsInTab() ) )
        {
            pActualSection.reset(new SwActualSection(nullptr, pSct, pSct->GetSection()->GetFormat()->GetSectionNode()));
            // tdf#132236 for SwUndoDelete: find outer sections whose start
            // nodes aren't contained in the range but whose end nodes are,
            // because section frames may need to be created for them
            SwActualSection * pUpperSection(pActualSection.get());
            while (pUpperSection->GetSectionNode()->EndOfSectionIndex() < nEndIndex)
            {
                SwStartNode *const pStart(pUpperSection->GetSectionNode()->StartOfSectionNode());
                if (!pStart->IsSectionNode())
                {
                    break;
                }
                // note: these don't have a section frame, check it in EndNode case!
                auto const pTmp(new SwActualSection(nullptr, nullptr, static_cast<SwSectionNode*>(pStart)));
                pUpperSection->SetUpper(pTmp);
                pUpperSection = pTmp;
            }
            OSL_ENSURE( !pLay->Lower() || !pLay->Lower()->IsColumnFrame(),
                "InsertCnt_: Wrong Call" );
        }
    }

    //If a section is "open", the pActualSection points to an SwActualSection.
    //If the page breaks, for "open" sections a follow will created.
    //For nested sections (which have, however, not a nested layout),
    //the SwActualSection class has a member, which points to an upper(section).
    //When the "inner" section finishes, the upper will used instead.

    // Do not consider the end node. The caller (Section/MakeFrames()) has to
    // ensure that the end of this range is positioned before EndIndex!
    for ( ; nEndIndex == SwNodeOffset(0) || nIndex < nEndIndex; ++nIndex)
    {
        SwNode *pNd = pDoc->GetNodes()[nIndex];
        if ( pNd->IsContentNode() )
        {
            SwContentNode* pNode = static_cast<SwContentNode*>(pNd);
            if (pLayout->HasMergedParas() && !pNd->IsCreateFrameWhenHidingRedlines())
            {
                if (pNd->IsTextNode()
                    && pNd->GetRedlineMergeFlag() == SwNode::Merge::NonFirst)
                {   // must have a frame already
                    assert(static_cast<SwTextFrame*>(pNode->getLayoutFrame(pLayout))->GetMergedPara());
                }
                continue; // skip it
            }
            pFrame = pNode->IsTextNode()
                        ? sw::MakeTextFrame(*pNode->GetTextNode(), pLay, eMode)
                        : pNode->MakeFrame(pLay);
            if( pPageMaker )
                pPageMaker->CheckInsert( nIndex );

            pFrame->InsertBehind( pLay, pPrv );
            // #i27138#
            // notify accessibility paragraphs objects about changed
            // CONTENT_FLOWS_FROM/_TO relation.
            // Relation CONTENT_FLOWS_FROM for next paragraph will change
            // and relation CONTENT_FLOWS_TO for previous paragraph will change.
            if ( pFrame->IsTextFrame() )
            {
                SwViewShell* pViewShell( pFrame->getRootFrame()->GetCurrShell() );
                // no notification, if <SwViewShell> is in construction
                if ( pViewShell && !pViewShell->IsInConstructor() &&
                     pViewShell->GetLayout() &&
                     pViewShell->GetLayout()->IsAnyShellAccessible() &&
                     pFrame->FindPageFrame() != nullptr)
                {
                    auto pNext = pFrame->FindNextCnt( true );
                    auto pPrev = pFrame->FindPrevCnt();
                    pViewShell->InvalidateAccessibleParaFlowRelation(
                        pNext ? pNext->DynCastTextFrame() : nullptr,
                        pPrev ? pPrev->DynCastTextFrame() : nullptr );
                    // #i68958#
                    // The information flags of the text frame are validated
                    // in methods <FindNextCnt(..)> and <FindPrevCnt(..)>.
                    // The information flags have to be invalidated, because
                    // it is possible, that the one of its upper frames
                    // isn't inserted into the layout.
                    pFrame->InvalidateInfFlags();
                }
            }
            // OD 12.08.2003 #i17969# - consider horizontal/vertical layout
            // for setting position at newly inserted frame
            lcl_SetPos( *pFrame, *pLay );
            pPrv = pFrame;

            if ( !pTable->empty() && bObjsDirect && !isFlyCreationSuppressed )
                AppendObjs( pTable, nIndex, pFrame, pPage, pDoc );
        }
        else if ( pNd->IsTableNode() )
        {   //Should we have encountered a table?
            SwTableNode *pTableNode = static_cast<SwTableNode*>(pNd);
            if (pLayout->IsHideRedlines())
            {
                // in the problematic case, there can be only 1 redline...
                SwPosition const tmp(*pNd);
                SwRangeRedline const*const pRedline(
                    pDoc->getIDocumentRedlineAccess().GetRedline(tmp, nullptr));
                // pathology: redline that starts on a TableNode; cannot
                // be created in UI but by import filters...
                if (pRedline
                    && pRedline->GetType() == RedlineType::Delete
                    && &pRedline->Start()->nNode.GetNode() == pNd)
                {
                    SAL_WARN("sw.pageframe", "skipping table frame creation on bizarre redline");
                    while (true)
                    {
                        pTableNode->GetNodes()[nIndex]->SetRedlineMergeFlag(SwNode::Merge::Hidden);
                        if (nIndex == pTableNode->EndOfSectionIndex())
                        {
                            break;
                        }
                        ++nIndex;
                    }
                    continue;
                }
            }
            if (pLayout->HasMergedParas() && !pNd->IsCreateFrameWhenHidingRedlines())
            {
                assert(pNd->GetRedlineMergeFlag() == SwNode::Merge::Hidden);
                nIndex = pTableNode->EndOfSectionIndex();
                continue; // skip it
            }

            pFrame = pTableNode->MakeFrame( pLay );

            // skip tables deleted with track changes
            if ( !static_cast<SwTabFrame*>(pFrame)->Lower() )
            {
                nIndex = pTableNode->EndOfSectionIndex();
                continue; // skip it
            }

            // #108116# loading may produce table structures that GCLines
            // needs to clean up. To keep table formulas correct, change
            // all table formulas to internal (BOXPTR) representation.
            SwTableFormulaUpdate aMsgHint( &pTableNode->GetTable() );
            aMsgHint.m_eFlags = TBL_BOXPTR;
            pDoc->getIDocumentFieldsAccess().UpdateTableFields( &aMsgHint );
            pTableNode->GetTable().GCLines();

            if( pPageMaker )
                pPageMaker->CheckInsert( nIndex );

            pFrame->InsertBehind( pLay, pPrv );
            if (pPage) // would null in SwCellFrame ctor
            {   // tdf#134931 call ResetTurbo(); not sure if Paste() would be
                pFrame->InvalidatePage(pPage); // better than InsertBehind()?
            }
            // #i27138#
            // notify accessibility paragraphs objects about changed
            // CONTENT_FLOWS_FROM/_TO relation.
            // Relation CONTENT_FLOWS_FROM for next paragraph will change
            // and relation CONTENT_FLOWS_TO for previous paragraph will change.
            {
                SwViewShell* pViewShell( pFrame->getRootFrame()->GetCurrShell() );
                // no notification, if <SwViewShell> is in construction
                if ( pViewShell && !pViewShell->IsInConstructor() &&
                     pViewShell->GetLayout() &&
                     pViewShell->GetLayout()->IsAnyShellAccessible() &&
                     pFrame->FindPageFrame() != nullptr)
                {
                    auto pNext = pFrame->FindNextCnt( true );
                    auto pPrev = pFrame->FindPrevCnt();
                    pViewShell->InvalidateAccessibleParaFlowRelation(
                            pNext ? pNext->DynCastTextFrame() : nullptr,
                            pPrev ? pPrev->DynCastTextFrame() : nullptr );
                }
            }
            if ( bObjsDirect && !pTable->empty() )
                static_cast<SwTabFrame*>(pFrame)->RegistFlys();
            // OD 12.08.2003 #i17969# - consider horizontal/vertical layout
            // for setting position at newly inserted frame
            lcl_SetPos( *pFrame, *pLay );

            pPrv = pFrame;
            //Set the index to the endnode of the table section.
            nIndex = pTableNode->EndOfSectionIndex();

            SwTabFrame* pTmpFrame = static_cast<SwTabFrame*>(pFrame);
            while ( pTmpFrame )
            {
                pTmpFrame->CheckDirChange();
                pTmpFrame = pTmpFrame->IsFollow() ? pTmpFrame->FindMaster() : nullptr;
            }

        }
        else if ( pNd->IsSectionNode() )
        {
            if (pLayout->HasMergedParas() && !pNd->IsCreateFrameWhenHidingRedlines())
            {
                assert(pNd->GetRedlineMergeFlag() == SwNode::Merge::Hidden);
                continue; // skip it
            }
            SwSectionNode *pNode = static_cast<SwSectionNode*>(pNd);
            if( pNode->GetSection().CalcHiddenFlag() )
                // is hidden, skip the area
                nIndex = pNode->EndOfSectionIndex();
            else
            {
                pFrame = pNode->MakeFrame( pLay );
                pActualSection.reset( new SwActualSection( pActualSection.release(),
                                                static_cast<SwSectionFrame*>(pFrame), pNode ) );
                if ( pActualSection->GetUpper() )
                {
                    //Insert behind the Upper, the "Follow" of the Upper will be
                    //generated at the EndNode.
                    SwSectionFrame *pTmp = pActualSection->GetUpper()->GetSectionFrame();
                    pFrame->InsertBehind( pTmp->GetUpper(), pTmp );
                    // OD 25.03.2003 #108339# - direct initialization of section
                    // after insertion in the layout
                    static_cast<SwSectionFrame*>(pFrame)->Init();
                }
                else
                {
                    pFrame->InsertBehind( pLay, pPrv );
                    // OD 25.03.2003 #108339# - direct initialization of section
                    // after insertion in the layout
                    static_cast<SwSectionFrame*>(pFrame)->Init();

                    // #i33963#
                    // Do not trust the IsInFootnote flag. If we are currently
                    // building up a table, the upper of pPrv may be a cell
                    // frame, but the cell frame does not have an upper yet.
                    if( pPrv && nullptr != pPrv->ImplFindFootnoteFrame() )
                    {
                        if( pPrv->IsSctFrame() )
                            pPrv = static_cast<SwSectionFrame*>(pPrv)->ContainsContent();
                        if( pPrv && pPrv->IsTextFrame() )
                            static_cast<SwTextFrame*>(pPrv)->Prepare( PrepareHint::QuoVadis, nullptr, false );
                    }
                }

                if (nIndex + 1 == nEndIndex
                        // tdf#136452 may also be needed at end of section
                    || pNode->EndOfSectionIndex() - 1 == nEndIndex)
                {   // tdf#131684 tdf#132236 fix upper of frame moved in
                    // SwUndoDelete; can't be done there unfortunately
                    // because empty section frames are deleted here
                    SwFrame *const pNext(
                        // if there's a parent section, it has been split
                        // into 2 SwSectionFrame already :(
                        (   pFrame->GetNext()
                         && pFrame->GetNext()->IsSctFrame()
                         && pActualSection->GetUpper()
                         && pActualSection->GetUpper()->GetSectionNode() ==
                             static_cast<SwSectionFrame const*>(pFrame->GetNext())->GetSection()->GetFormat()->GetSectionNode())
                        ? static_cast<SwSectionFrame *>(pFrame->GetNext())->ContainsContent()
                        : pFrame->GetNext());
                    if (pNext
                        && pNext->IsTextFrame()
                        && static_cast<SwTextFrame*>(pNext)->GetTextNodeFirst() == pDoc->GetNodes()[nEndIndex]
                        && (pNext->GetUpper() == pFrame->GetUpper()
                            || pFrame->GetNext()->IsSctFrame())) // checked above
                    {
                        pNext->Cut();
                        pNext->InvalidateInfFlags(); // mbInfSct changed
                        // could have columns
                        SwSectionFrame *const pSection(static_cast<SwSectionFrame*>(pFrame));
                        assert(!pSection->Lower() || pSection->Lower()->IsLayoutFrame());
                        SwLayoutFrame *const pParent(pSection->Lower() ? pSection->GetNextLayoutLeaf() : pSection);
                        assert(!pParent->Lower());
                        // paste invalidates, section could have indent...
                        pNext->Paste(pParent, nullptr);
                    }
                }
                // #i27138#
                // notify accessibility paragraphs objects about changed
                // CONTENT_FLOWS_FROM/_TO relation.
                // Relation CONTENT_FLOWS_FROM for next paragraph will change
                // and relation CONTENT_FLOWS_TO for previous paragraph will change.
                {
                    SwViewShell* pViewShell( pFrame->getRootFrame()->GetCurrShell() );
                    // no notification, if <SwViewShell> is in construction
                    if ( pViewShell && !pViewShell->IsInConstructor() &&
                         pViewShell->GetLayout() &&
                         pViewShell->GetLayout()->IsAnyShellAccessible() &&
                         pFrame->FindPageFrame() != nullptr)
                    {
                        auto pNext = pFrame->FindNextCnt( true );
                        auto pPrev = pFrame->FindPrevCnt();
                        pViewShell->InvalidateAccessibleParaFlowRelation(
                            pNext ? pNext->DynCastTextFrame() : nullptr,
                            pPrev ? pPrev->DynCastTextFrame() : nullptr );
                    }
                }
                pFrame->CheckDirChange();

                // OD 12.08.2003 #i17969# - consider horizontal/vertical layout
                // for setting position at newly inserted frame
                lcl_SetPos( *pFrame, *pLay );

                // OD 20.11.2002 #105405# - no page, no invalidate.
                if ( pPage )
                {
                    // OD 18.09.2002 #100522#
                    // invalidate page in order to force format and paint of
                    // inserted section frame
                    pFrame->InvalidatePage( pPage );

                    // FME 10.11.2003 #112243#
                    // Invalidate fly content flag:
                    if ( pFrame->IsInFly() )
                        pPage->InvalidateFlyContent();

                    // OD 14.11.2002 #104684# - invalidate page content in order to
                    // force format and paint of section content.
                    pPage->InvalidateContent();
                }

                pLay = static_cast<SwLayoutFrame*>(pFrame);
                if ( pLay->Lower() && pLay->Lower()->IsLayoutFrame() )
                    pLay = pLay->GetNextLayoutLeaf();
                pPrv = nullptr;
            }
        }
        else if ( pNd->IsEndNode() && pNd->StartOfSectionNode()->IsSectionNode() )
        {
            if (pLayout->HasMergedParas() && !pNd->IsCreateFrameWhenHidingRedlines())
            {
                assert(pNd->GetRedlineMergeFlag() == SwNode::Merge::Hidden);
                continue; // skip it
            }
            if (pLayout->HasMergedParas() && !pNd->StartOfSectionNode()->IsCreateFrameWhenHidingRedlines())
            {   // tdf#135014 section break in fieldmark (start inside, end outside)
                assert(pNd->StartOfSectionNode()->GetRedlineMergeFlag() == SwNode::Merge::Hidden);
                continue; // skip it
            }
            assert(pActualSection && "Section end without section start?");
            assert(pActualSection->GetSectionNode() == pNd->StartOfSectionNode());

            //Close the section, where appropriate activate the surrounding
            //section again.
            pActualSection.reset(pActualSection->GetUpper());
            pLay = pLay->FindSctFrame();
            if ( pActualSection )
            {
                //Could be, that the last SectionFrame remains empty.
                //Then now is the time to remove them.
                if ( !pLay->ContainsContent() )
                {
                    SwFrame *pTmpFrame = pLay;
                    pLay = pTmpFrame->GetUpper();
                    pPrv = pTmpFrame->GetPrev();
                    pTmpFrame->RemoveFromLayout();
                    SwFrame::DestroyFrame(pTmpFrame);
                }
                else
                {
                    pPrv = pLay;
                    pLay = pLay->GetUpper();
                }

                // new section frame
                pFrame = pActualSection->GetSectionNode()->MakeFrame( pLay );
                pFrame->InsertBehind( pLay, pPrv );
                static_cast<SwSectionFrame*>(pFrame)->Init();

                // OD 12.08.2003 #i17969# - consider horizontal/vertical layout
                // for setting position at newly inserted frame
                lcl_SetPos( *pFrame, *pLay );

                SwSectionFrame* pOuterSectionFrame = pActualSection->GetSectionFrame();

                // a follow has to be appended to the new section frame
                SwSectionFrame* pFollow = pOuterSectionFrame ? pOuterSectionFrame->GetFollow() : nullptr;
                if ( pFollow )
                {
                    pOuterSectionFrame->SetFollow( nullptr );
                    pOuterSectionFrame->InvalidateSize();
                    static_cast<SwSectionFrame*>(pFrame)->SetFollow( pFollow );
                }

                // We don't want to leave empty parts back.
                if (pOuterSectionFrame &&
                    ! pOuterSectionFrame->IsColLocked() &&
                    ! pOuterSectionFrame->ContainsContent() )
                {
                    pOuterSectionFrame->DelEmpty( true );
                    SwFrame::DestroyFrame(pOuterSectionFrame);
                }
                pActualSection->SetSectionFrame( static_cast<SwSectionFrame*>(pFrame) );

                pLay = static_cast<SwLayoutFrame*>(pFrame);
                if ( pLay->Lower() && pLay->Lower()->IsLayoutFrame() )
                    pLay = pLay->GetNextLayoutLeaf();
                pPrv = nullptr;
            }
            else
            {
                //Nothing more with sections, it goes on right behind
                //the SectionFrame.
                pPrv = pLay;
                pLay = pLay->GetUpper();
            }
        }
        else if( pNd->IsStartNode() &&
                 SwFlyStartNode == static_cast<SwStartNode*>(pNd)->GetStartNodeType() )
        {
            if (pLayout->HasMergedParas() && !pNd->IsCreateFrameWhenHidingRedlines())
            {
                assert(pNd->GetRedlineMergeFlag() == SwNode::Merge::Hidden);
                assert(false); // actually a fly-section can't be deleted?
                continue; // skip it
            }
            if ( !pTable->empty() && bObjsDirect && !isFlyCreationSuppressed )
            {
                SwFlyFrame* pFly = pLay->FindFlyFrame();
                if( pFly )
                    AppendObjs( pTable, nIndex, pFly, pPage, pDoc );
            }
        }
        else
        {
            assert(!pLayout->HasMergedParas()
                || pNd->GetRedlineMergeFlag() != SwNode::Merge::Hidden);
            // Neither Content nor table nor section, so we are done.
            break;
        }
    }

    if ( pActualSection )
    {
        // Might happen that an empty (Follow-)Section is left over.
        if ( !(pLay = pActualSection->GetSectionFrame())->ContainsContent() )
        {
            pLay->RemoveFromLayout();
            SwFrame::DestroyFrame(pLay);
        }
        pActualSection.reset();
    }

    if ( bPages ) // let the Flys connect to each other
    {
        if ( !isFlyCreationSuppressed )
            AppendAllObjs( pTable, pLayout );
        bObjsDirect = true;
    }

    if( pPageMaker )
    {
        pPageMaker->CheckFlyCache( pPage );
        pPageMaker.reset();
        if( pDoc->GetLayoutCache() )
        {
#ifdef DBG_UTIL
            pDoc->GetLayoutCache()->CompareLayout( *pDoc );
#endif
            pDoc->GetLayoutCache()->ClearImpl();
        }
    }

    pDoc->getIDocumentTimerAccess().UnblockIdling();
    if( bOldCallbackActionEnabled )
        pLayout->SetCallbackActionEnabled( bOldCallbackActionEnabled );
}

void MakeFrames( SwDoc *pDoc, const SwNodeIndex &rSttIdx,
               const SwNodeIndex &rEndIdx )
{
    bObjsDirect = false;

    SwNodeIndex aTmp( rSttIdx );
    SwNodeOffset nEndIdx = rEndIdx.GetIndex();
    SwNode* pNd = pDoc->GetNodes().FindPrvNxtFrameNode( aTmp,
                                            pDoc->GetNodes()[ nEndIdx-1 ]);
    if ( pNd )
    {
        bool bApres = aTmp < rSttIdx;
        SwNode2Layout aNode2Layout( *pNd, rSttIdx.GetIndex() );
        SwFrame* pFrame;
        sw::FrameMode eMode = sw::FrameMode::Existing;
        while( nullptr != (pFrame = aNode2Layout.NextFrame()) )
        {
            SwLayoutFrame *pUpper = pFrame->GetUpper();
            SwFootnoteFrame* pFootnoteFrame = pUpper->FindFootnoteFrame();
            bool bOldLock, bOldFootnote;
            if( pFootnoteFrame )
            {
                bOldFootnote = pFootnoteFrame->IsColLocked();
                pFootnoteFrame->ColLock();
            }
            else
                bOldFootnote = true;
            SwSectionFrame* pSct = pUpper->FindSctFrame();
            // Inside of footnotes only those areas are interesting that are inside of them. But
            // not the ones (e.g. column areas) in which are the footnote containers positioned.
            // #109767# Table frame is in section, insert section in cell frame.
            if( pSct && ((pFootnoteFrame && !pSct->IsInFootnote()) || pUpper->IsCellFrame()) )
                pSct = nullptr;
            if( pSct )
            {   // to prevent pTmp->MoveFwd from destroying the SectionFrame
                bOldLock = pSct->IsColLocked();
                pSct->ColLock();
            }
            else
                bOldLock = true;

            // If pFrame cannot be moved, it is not possible to move it to the next page. This applies
            // also for frames (in the first column of a frame pFrame is moveable) and column
            // sections of tables (also here pFrame is moveable).
            bool bMoveNext = nEndIdx - rSttIdx.GetIndex() > SwNodeOffset(120);
            bool bAllowMove = !pFrame->IsInFly() && pFrame->IsMoveable() &&
                 (!pFrame->IsInTab() || pFrame->IsTabFrame() );
            if ( bMoveNext && bAllowMove )
            {
                SwFrame *pMove = pFrame;
                SwFrame *pPrev = pFrame->GetPrev();
                SwFlowFrame *pTmp = SwFlowFrame::CastFlowFrame( pMove );
                assert(pTmp);

                if ( bApres )
                {
                    // The rest of this page should be empty. Thus, the following one has to move to
                    // the next page (it might also be located in the following column).
                    assert(!pTmp->HasFollow() && "prev. node's frame is not last");
                    pPrev = pFrame;
                    // If the surrounding SectionFrame has a "next" one,
                    // so this one needs to be moved as well.
                    pMove = pFrame->GetIndNext();
                    SwColumnFrame* pCol = static_cast<SwColumnFrame*>(pFrame->FindColFrame());
                    if( pCol )
                        pCol = static_cast<SwColumnFrame*>(pCol->GetNext());
                    do
                    {
                        if( pCol && !pMove )
                        {   // No successor so far, look into the next column
                            pMove = pCol->ContainsAny();
                            if( pCol->GetNext() )
                                pCol = static_cast<SwColumnFrame*>(pCol->GetNext());
                            else if( pCol->IsInSct() )
                            {   // If there is no following column but we are in a column frame,
                                // there might be (page) columns outside of it.
                                pCol = static_cast<SwColumnFrame*>(pCol->FindSctFrame()->FindColFrame());
                                if( pCol )
                                    pCol = static_cast<SwColumnFrame*>(pCol->GetNext());
                            }
                            else
                                pCol = nullptr;
                        }
                        // skip invalid SectionFrames
                        while( pMove && pMove->IsSctFrame() &&
                               !static_cast<SwSectionFrame*>(pMove)->GetSection() )
                            pMove = pMove->GetNext();
                    } while( !pMove && pCol );

                    if( pMove )
                    {
                        if ( pMove->IsContentFrame() )
                            pTmp = static_cast<SwContentFrame*>(pMove);
                        else if ( pMove->IsTabFrame() )
                            pTmp = static_cast<SwTabFrame*>(pMove);
                        else if ( pMove->IsSctFrame() )
                        {
                            pMove = static_cast<SwSectionFrame*>(pMove)->ContainsAny();
                            if( pMove )
                                pTmp = SwFlowFrame::CastFlowFrame( pMove );
                            else
                                pTmp = nullptr;
                        }
                    }
                    else
                        pTmp = nullptr;
                }
                else
                {
                    assert(!pTmp->IsFollow() && "next node's frame is not master");
                    // move the _content_ of a section frame
                    if( pMove->IsSctFrame() )
                    {
                        while( pMove && pMove->IsSctFrame() &&
                               !static_cast<SwSectionFrame*>(pMove)->GetSection() )
                            pMove = pMove->GetNext();
                        if( pMove && pMove->IsSctFrame() )
                            pMove = static_cast<SwSectionFrame*>(pMove)->ContainsAny();
                        if( pMove )
                            pTmp = SwFlowFrame::CastFlowFrame( pMove );
                        else
                            pTmp = nullptr;
                    }
                }

                if( pTmp )
                {
                    SwFrame* pOldUp = pTmp->GetFrame().GetUpper();
                    // MoveFwd==true means that we are still on the same page.
                    // But since we want to move if possible!
                    bool bTmpOldLock = pTmp->IsJoinLocked();
                    pTmp->LockJoin();
                    while( pTmp->MoveFwd( true, false, true ) )
                    {
                        if( pOldUp == pTmp->GetFrame().GetUpper() )
                            break;
                        pOldUp = pTmp->GetFrame().GetUpper();
                    }
                    if( !bTmpOldLock )
                        pTmp->UnlockJoin();
                }
                ::InsertCnt_( pUpper, pDoc, rSttIdx.GetIndex(),
                              pFrame->IsInDocBody(), nEndIdx, pPrev, eMode );
            }
            else
            {
                bool bSplit;
                SwFrame* pPrv = bApres ? pFrame : pFrame->GetPrev();
                // If the section frame is inserted into another one, it must be split.
                if( pSct && rSttIdx.GetNode().IsSectionNode() )
                {
                    bSplit = pSct->SplitSect( pFrame, bApres );
                    if( !bSplit && !bApres )
                    {
                        pUpper = pSct->GetUpper();
                        pPrv = pSct->GetPrev();
                    }
                }
                else
                    bSplit = false;

                ::InsertCnt_( pUpper, pDoc, rSttIdx.GetIndex(), false,
                              nEndIdx, pPrv, eMode );
                // OD 23.06.2003 #108784# - correction: append objects doesn't
                // depend on value of <bAllowMove>
                if( !isFlyCreationSuppressed )
                {
                    const SwFrameFormats *pTable = pDoc->GetSpzFrameFormats();
                    if( !pTable->empty() )
                        AppendAllObjs( pTable, pUpper );
                }

                // If nothing was added (e.g. a hidden section), the split must be reversed.
                if( bSplit && pSct && pSct->GetNext()
                    && pSct->GetNext()->IsSctFrame() )
                    pSct->MergeNext( static_cast<SwSectionFrame*>(pSct->GetNext()) );
                if( pFrame->IsInFly() )
                    pFrame->FindFlyFrame()->Invalidate_();
                if( pFrame->IsInTab() )
                    pFrame->InvalidateSize();
            }

            SwPageFrame *pPage = pUpper->FindPageFrame();
            SwFrame::CheckPageDescs( pPage, false );
            if( !bOldFootnote )
                pFootnoteFrame->ColUnlock();
            if( !bOldLock )
            {
                pSct->ColUnlock();
                // pSct might be empty (e.g. when inserting linked section containing further
                // sections) and can be destroyed in such cases.
                if( !pSct->ContainsContent() )
                {
                    pSct->DelEmpty( true );
                    pUpper->getRootFrame()->RemoveFromList( pSct );
                    SwFrame::DestroyFrame(pSct);
                }
            }
            eMode = sw::FrameMode::New; // use Existing only once!
        }
    }

    bObjsDirect = true;
}

SwBorderAttrs::SwBorderAttrs(const sw::BorderCacheOwner* pOwner, const SwFrame* pConstructor)
    : SwCacheObj(pOwner)
    , m_rAttrSet(pConstructor->IsContentFrame()
                    ? pConstructor->IsTextFrame()
                        ? static_cast<const SwTextFrame*>(pConstructor)->GetTextNodeForParaProps()->GetSwAttrSet()
                        : static_cast<const SwNoTextFrame*>(pConstructor)->GetNode()->GetSwAttrSet()
                    : static_cast<const SwLayoutFrame*>(pConstructor)->GetFormat()->GetAttrSet())
    , m_rUL(m_rAttrSet.GetULSpace())
    // #i96772#
    // LRSpaceItem is copied due to the possibility that it is adjusted - see below
    , m_rLR(m_rAttrSet.GetLRSpace().Clone())
    , m_rBox(m_rAttrSet.GetBox())
    , m_rShadow(m_rAttrSet.GetShadow())
    , m_aFrameSize(m_rAttrSet.GetFrameSize().GetSize())
    , m_bIsLine(false)
    , m_bJoinedWithPrev(false)
    , m_bJoinedWithNext(false)
    , m_nTopLine(0)
    , m_nBottomLine(0)
    , m_nLeftLine(0)
    , m_nRightLine(0)
    , m_nTop(0)
    , m_nBottom(0)
    , m_nGetTopLine(0)
    , m_nGetBottomLine(0)
    , m_nLineSpacing(0)
{
    // #i96772#
    const SwTextFrame* pTextFrame = pConstructor->DynCastTextFrame();
    if ( pTextFrame )
    {
        pTextFrame->GetTextNodeForParaProps()->ClearLRSpaceItemDueToListLevelIndents( m_rLR );
    }
    else if ( pConstructor->IsNoTextFrame() )
    {
        m_rLR = std::make_shared<SvxLRSpaceItem>(RES_LR_SPACE);
    }

    // Caution: The USHORTs for the cached values are not initialized by intention!

    // everything needs to be calculated at least once:
    m_bTopLine = m_bBottomLine = m_bLeftLine = m_bRightLine =
    m_bTop     = m_bBottom     = m_bLine   = true;

    // except this one: calculate line spacing before cell border only for text frames
    m_bLineSpacing = bool(pTextFrame);

    m_bCacheGetLine = m_bCachedGetTopLine = m_bCachedGetBottomLine = false;
    // OD 21.05.2003 #108789# - init cache status for values <m_bJoinedWithPrev>
    // and <m_bJoinedWithNext>, which aren't initialized by default.
    m_bCachedJoinedWithPrev = false;
    m_bCachedJoinedWithNext = false;
}

SwBorderAttrs::~SwBorderAttrs()
{
    const_cast<sw::BorderCacheOwner*>(static_cast<sw::BorderCacheOwner const *>(m_pOwner))->m_bInCache = false;
}

/* All calc methods calculate a safety distance in addition to the values given by the attributes.
 * This safety distance is only added when working with borders and/or shadows to prevent that
 * e.g. borders are painted over.
 */

void SwBorderAttrs::CalcTop_()
{
    m_nTop = CalcTopLine() + m_rUL.GetUpper();

    if (m_rLR)
    {
        bool bGutterAtTop = m_rAttrSet.GetDoc()->getIDocumentSettingAccess().get(
            DocumentSettingId::GUTTER_AT_TOP);
        if (bGutterAtTop)
        {
            // Decrease the print area: the top space is the sum of top and gutter margins.
            m_nTop += m_rLR->GetGutterMargin();
        }
    }

    m_bTop = false;
}

void SwBorderAttrs::CalcBottom_()
{
    m_nBottom = CalcBottomLine() + m_rUL.GetLower();
    m_bBottom = false;
}

tools::Long SwBorderAttrs::CalcRight( const SwFrame* pCaller ) const
{
    tools::Long nRight=0;

    if (!pCaller->IsTextFrame() || !static_cast<const SwTextFrame*>(pCaller)->GetDoc().GetDocumentSettingManager().get(DocumentSettingId::INVERT_BORDER_SPACING)) {
    // OD 23.01.2003 #106895# - for cell frame in R2L text direction the left
    // and right border are painted on the right respectively left.
    if ( pCaller->IsCellFrame() && pCaller->IsRightToLeft() )
        nRight = CalcLeftLine();
    else
        nRight = CalcRightLine();

    }
    // for paragraphs, "left" is "before text" and "right" is "after text"
    if ( pCaller->IsTextFrame() && pCaller->IsRightToLeft() )
        nRight += m_rLR->GetLeft();
    else
        nRight += m_rLR->GetRight();

    // correction: retrieve left margin for numbering in R2L-layout
    if ( pCaller->IsTextFrame() && pCaller->IsRightToLeft() )
    {
        nRight += static_cast<const SwTextFrame*>(pCaller)->GetTextNodeForParaProps()->GetLeftMarginWithNum();
    }

    if (pCaller->IsPageFrame() && m_rLR)
    {
        const auto pPageFrame = static_cast<const SwPageFrame*>(pCaller);
        bool bGutterAtTop = pPageFrame->GetFormat()->getIDocumentSettingAccess().get(
            DocumentSettingId::GUTTER_AT_TOP);
        if (!bGutterAtTop)
        {
            bool bRtlGutter = pPageFrame->GetAttrSet()->GetItem<SfxBoolItem>(RES_RTL_GUTTER)->GetValue();
            tools::Long nGutterMargin = bRtlGutter ? m_rLR->GetGutterMargin() : m_rLR->GetRightGutterMargin();
            // Decrease the print area: the right space is the sum of right and right gutter
            // margins.
            nRight += nGutterMargin;
        }
    }

    return nRight;
}

/// Tries to detect if this paragraph has a floating table attached.
static bool lcl_hasTabFrame(const SwTextFrame* pTextFrame)
{
    if (pTextFrame->GetDrawObjs())
    {
        const SwSortedObjs* pSortedObjs = pTextFrame->GetDrawObjs();
        if (pSortedObjs->size() > 0)
        {
            SwAnchoredObject* pObject = (*pSortedObjs)[0];
            if (auto pFly = pObject->DynCastFlyFrame())
            {
                if (pFly->Lower() && pFly->Lower()->IsTabFrame())
                    return true;
            }
        }
    }
    return false;
}

tools::Long SwBorderAttrs::CalcLeft( const SwFrame *pCaller ) const
{
    tools::Long nLeft=0;

    if (!pCaller->IsTextFrame() || !static_cast<const SwTextFrame*>(pCaller)->GetDoc().GetDocumentSettingManager().get(DocumentSettingId::INVERT_BORDER_SPACING))
    {
        // OD 23.01.2003 #106895# - for cell frame in R2L text direction the left
        // and right border are painted on the right respectively left.
        if ( pCaller->IsCellFrame() && pCaller->IsRightToLeft() )
            nLeft = CalcRightLine();
        else
            nLeft = CalcLeftLine();
    }

    // for paragraphs, "left" is "before text" and "right" is "after text"
    if ( pCaller->IsTextFrame() && pCaller->IsRightToLeft() )
        nLeft += m_rLR->GetRight();
    else
    {
        bool bIgnoreMargin = false;
        if (pCaller->IsTextFrame())
        {
            const SwTextFrame* pTextFrame = static_cast<const SwTextFrame*>(pCaller);
            if (pTextFrame->GetDoc().GetDocumentSettingManager().get(DocumentSettingId::FLOATTABLE_NOMARGINS))
            {
                // If this is explicitly requested, ignore the margins next to the floating table.
                if (lcl_hasTabFrame(pTextFrame))
                    bIgnoreMargin = true;
                // TODO here we only handle the first two paragraphs, would be nice to generalize this.
                else if (pTextFrame->FindPrev() && pTextFrame->FindPrev()->IsTextFrame() && lcl_hasTabFrame(static_cast<const SwTextFrame*>(pTextFrame->FindPrev())))
                    bIgnoreMargin = true;
            }
        }
        if (!bIgnoreMargin)
            nLeft += m_rLR->GetLeft();
    }

    // correction: do not retrieve left margin for numbering in R2L-layout
    if ( pCaller->IsTextFrame() && !pCaller->IsRightToLeft() )
    {
        nLeft += static_cast<const SwTextFrame*>(pCaller)->GetTextNodeForParaProps()->GetLeftMarginWithNum();
    }

    if (pCaller->IsPageFrame() && m_rLR)
    {
        const auto pPageFrame = static_cast<const SwPageFrame*>(pCaller);
        bool bGutterAtTop = pPageFrame->GetFormat()->getIDocumentSettingAccess().get(
            DocumentSettingId::GUTTER_AT_TOP);
        if (!bGutterAtTop)
        {
            bool bRtlGutter = pPageFrame->GetAttrSet()->GetItem<SfxBoolItem>(RES_RTL_GUTTER)->GetValue();
            tools::Long nGutterMargin = bRtlGutter ? m_rLR->GetRightGutterMargin() : m_rLR->GetGutterMargin();
            // Decrease the print area: the left space is the sum of left and gutter margins.
            nLeft += nGutterMargin;
        }
    }

    return nLeft;
}

/* Calculated values for borders and shadows.
 * It might be that a distance is wanted even without lines. This will be
 * considered here and not by the attribute (e.g. bBorderDist for cells).
 */

void SwBorderAttrs::CalcTopLine_()
{
    m_nTopLine = m_rBox.CalcLineSpace( SvxBoxItemLine::TOP, /*bEvenIfNoLine*/true );
    m_nTopLine = m_nTopLine + m_rShadow.CalcShadowSpace(SvxShadowItemSide::TOP);
    m_bTopLine = false;
}

void SwBorderAttrs::CalcBottomLine_()
{
    m_nBottomLine = m_rBox.CalcLineSpace( SvxBoxItemLine::BOTTOM, true );
    m_nBottomLine = m_nBottomLine + m_rShadow.CalcShadowSpace(SvxShadowItemSide::BOTTOM);
    m_bBottomLine = false;
}

void SwBorderAttrs::CalcLeftLine_()
{
    m_nLeftLine = m_rBox.CalcLineSpace( SvxBoxItemLine::LEFT, true);
    m_nLeftLine = m_nLeftLine + m_rShadow.CalcShadowSpace(SvxShadowItemSide::LEFT);
    m_bLeftLine = false;
}

void SwBorderAttrs::CalcRightLine_()
{
    m_nRightLine = m_rBox.CalcLineSpace( SvxBoxItemLine::RIGHT, true );
    m_nRightLine = m_nRightLine + m_rShadow.CalcShadowSpace(SvxShadowItemSide::RIGHT);
    m_bRightLine = false;
}

void SwBorderAttrs::IsLine_()
{
    m_bIsLine = m_rBox.GetTop() || m_rBox.GetBottom() ||
              m_rBox.GetLeft()|| m_rBox.GetRight();
    m_bLine = false;
}

/* The borders of neighboring paragraphs are condensed by following algorithm:
 *
 * 1. No top border if the predecessor has the same top border and (3) applies.
 *    In addition, the paragraph needs to have a border at least one side (left/right/bottom).
 * 2. No bottom border if the successor has the same bottom border and (3) applies.
 *    In addition, the paragraph needs to have a border at least one side (left/right/top).
 * 3. The borders on the left and right side are identical between the current and the
 *    pre-/succeeding paragraph.
 */

static bool CmpLines( const editeng::SvxBorderLine *pL1, const editeng::SvxBorderLine *pL2 )
{
    return ( ((pL1 && pL2) && (*pL1 == *pL2)) || (!pL1 && !pL2) );
}

// OD 21.05.2003 #108789# - change name of 1st parameter - "rAttrs" -> "rCmpAttrs"
// OD 21.05.2003 #108789# - compare <CalcRight()> and <rCmpAttrs.CalcRight()>
//          instead of only the right LR-spacing, because R2L-layout has to be
//          considered.
bool SwBorderAttrs::CmpLeftRight( const SwBorderAttrs &rCmpAttrs,
                                  const SwFrame *pCaller,
                                  const SwFrame *pCmp ) const
{
    return ( CmpLines( rCmpAttrs.GetBox().GetLeft(), GetBox().GetLeft()  ) &&
             CmpLines( rCmpAttrs.GetBox().GetRight(),GetBox().GetRight() ) &&
             CalcLeft( pCaller ) == rCmpAttrs.CalcLeft( pCmp ) &&
             // OD 21.05.2003 #108789# - compare <CalcRight> with <rCmpAttrs.CalcRight>.
             CalcRight( pCaller ) == rCmpAttrs.CalcRight( pCmp ) );
}

bool SwBorderAttrs::JoinWithCmp( const SwFrame& _rCallerFrame,
                                  const SwFrame& _rCmpFrame ) const
{
    bool bReturnVal = false;

    SwBorderAttrAccess aCmpAccess( SwFrame::GetCache(), &_rCmpFrame );
    const SwBorderAttrs &rCmpAttrs = *aCmpAccess.Get();
    if ( m_rShadow == rCmpAttrs.GetShadow() &&
         CmpLines( m_rBox.GetTop(), rCmpAttrs.GetBox().GetTop() ) &&
         CmpLines( m_rBox.GetBottom(), rCmpAttrs.GetBox().GetBottom() ) &&
         CmpLeftRight( rCmpAttrs, &_rCallerFrame, &_rCmpFrame )
       )
    {
        bReturnVal = true;
    }

    return bReturnVal;
}

// OD 21.05.2003 #108789# - method to determine, if borders are joined with
// previous frame. Calculated value saved in cached value <m_bJoinedWithPrev>
// OD 2004-02-26 #i25029# - add 2nd parameter <_pPrevFrame>
void SwBorderAttrs::CalcJoinedWithPrev( const SwFrame& _rFrame,
                                         const SwFrame* _pPrevFrame )
{
    // set default
    m_bJoinedWithPrev = false;

    if ( _rFrame.IsTextFrame() )
    {
        // text frame can potentially join with previous text frame, if
        // corresponding attribute set is set at previous text frame.
        // OD 2004-02-26 #i25029# - If parameter <_pPrevFrame> is set, take this
        // one as previous frame.
        const SwFrame* pPrevFrame = _pPrevFrame ? _pPrevFrame : _rFrame.GetPrev();
        // OD 2004-02-13 #i25029# - skip hidden text frames.
        while ( pPrevFrame && pPrevFrame->IsTextFrame() &&
                static_cast<const SwTextFrame*>(pPrevFrame)->IsHiddenNow() )
        {
            pPrevFrame = pPrevFrame->GetPrev();
        }
        if ( pPrevFrame && pPrevFrame->IsTextFrame() &&
             pPrevFrame->GetAttrSet()->GetParaConnectBorder().GetValue()
           )
        {
            m_bJoinedWithPrev = JoinWithCmp( _rFrame, *pPrevFrame );
        }
    }

    // valid cache status, if demanded
    // OD 2004-02-26 #i25029# - Do not validate cache, if parameter <_pPrevFrame>
    // is set.
    m_bCachedJoinedWithPrev = m_bCacheGetLine && !_pPrevFrame;
}

// OD 21.05.2003 #108789# - method to determine, if borders are joined with
// next frame. Calculated value saved in cached value <m_bJoinedWithNext>
void SwBorderAttrs::CalcJoinedWithNext( const SwFrame& _rFrame )
{
    // set default
    m_bJoinedWithNext = false;

    if ( _rFrame.IsTextFrame() )
    {
        // text frame can potentially join with next text frame, if
        // corresponding attribute set is set at current text frame.
        // OD 2004-02-13 #i25029# - get next frame, but skip hidden text frames.
        const SwFrame* pNextFrame = _rFrame.GetNext();
        while ( pNextFrame && pNextFrame->IsTextFrame() &&
                static_cast<const SwTextFrame*>(pNextFrame)->IsHiddenNow() )
        {
            pNextFrame = pNextFrame->GetNext();
        }
        if ( pNextFrame && pNextFrame->IsTextFrame() &&
             _rFrame.GetAttrSet()->GetParaConnectBorder().GetValue()
           )
        {
            m_bJoinedWithNext = JoinWithCmp( _rFrame, *pNextFrame );
        }
    }

    // valid cache status, if demanded
    m_bCachedJoinedWithNext = m_bCacheGetLine;
}

// OD 21.05.2003 #108789# - accessor for cached values <m_bJoinedWithPrev>
// OD 2004-02-26 #i25029# - add 2nd parameter <_pPrevFrame>, which is passed to
// method <_CalcJoindWithPrev(..)>.
bool SwBorderAttrs::JoinedWithPrev( const SwFrame& _rFrame,
                                    const SwFrame* _pPrevFrame ) const
{
    if ( !m_bCachedJoinedWithPrev || _pPrevFrame )
    {
        // OD 2004-02-26 #i25029# - pass <_pPrevFrame> as 2nd parameter
        const_cast<SwBorderAttrs*>(this)->CalcJoinedWithPrev( _rFrame, _pPrevFrame );
    }

    return m_bJoinedWithPrev;
}

bool SwBorderAttrs::JoinedWithNext( const SwFrame& _rFrame ) const
{
    if ( !m_bCachedJoinedWithNext )
    {
        const_cast<SwBorderAttrs*>(this)->CalcJoinedWithNext( _rFrame );
    }

    return m_bJoinedWithNext;
}

// OD 2004-02-26 #i25029# - added 2nd parameter <_pPrevFrame>, which is passed to
// method <JoinedWithPrev>
void SwBorderAttrs::GetTopLine_( const SwFrame& _rFrame,
                                 const SwFrame* _pPrevFrame )
{
    sal_uInt16 nRet = CalcTopLine();

    // OD 21.05.2003 #108789# - use new method <JoinWithPrev()>
    // OD 2004-02-26 #i25029# - add 2nd parameter
    if ( JoinedWithPrev( _rFrame, _pPrevFrame ) )
    {
        nRet = 0;
    }

    m_bCachedGetTopLine = m_bCacheGetLine;

    m_nGetTopLine = nRet;
}

void SwBorderAttrs::GetBottomLine_( const SwFrame& _rFrame )
{
    sal_uInt16 nRet = CalcBottomLine();

    // OD 21.05.2003 #108789# - use new method <JoinWithPrev()>
    if ( JoinedWithNext( _rFrame ) )
    {
        nRet = 0;
    }

    m_bCachedGetBottomLine = m_bCacheGetLine;

    m_nGetBottomLine = nRet;
}

void SwBorderAttrs::CalcLineSpacing_()
{
    // tdf#125300 compatibility option AddParaLineSpacingToTableCells needs also line spacing
    const SvxLineSpacingItem &rSpace = m_rAttrSet.GetLineSpacing();
    if ( rSpace.GetInterLineSpaceRule() == SvxInterLineSpaceRule::Prop && rSpace.GetPropLineSpace() > 100 )
    {
        sal_Int32 nFontSize = m_rAttrSet.Get(RES_CHRATR_FONTSIZE).GetHeight();
        m_nLineSpacing = nFontSize * (rSpace.GetPropLineSpace() - 100) * 1.15 / 100;
    }
    m_bLineSpacing = false;
}

static sw::BorderCacheOwner const* GetBorderCacheOwner(SwFrame const& rFrame)
{
    return rFrame.IsContentFrame()
        ? static_cast<sw::BorderCacheOwner const*>(rFrame.IsTextFrame()
        // sw_redlinehide: presumably this caches the border attrs at the model level and can be shared across different layouts so we want the ParaProps node here
            ? static_cast<const SwTextFrame&>(rFrame).GetTextNodeForParaProps()
            : static_cast<const SwNoTextFrame&>(rFrame).GetNode())
        : static_cast<sw::BorderCacheOwner const*>(static_cast<const SwLayoutFrame&>(rFrame).GetFormat());
}

SwBorderAttrAccess::SwBorderAttrAccess( SwCache &rCach, const SwFrame *pFrame ) :
    SwCacheAccess( rCach,
        static_cast<void const *>(GetBorderCacheOwner(*pFrame)),
        GetBorderCacheOwner(*pFrame)->IsInCache()),
    m_pConstructor( pFrame )
{
}

SwCacheObj *SwBorderAttrAccess::NewObj()
{
    const_cast<sw::BorderCacheOwner *>(static_cast<sw::BorderCacheOwner const *>(m_pOwner))->m_bInCache = true;
    return new SwBorderAttrs( static_cast<sw::BorderCacheOwner const *>(m_pOwner), m_pConstructor );
}

SwBorderAttrs *SwBorderAttrAccess::Get()
{
    return static_cast<SwBorderAttrs*>(SwCacheAccess::Get());
}

SwOrderIter::SwOrderIter( const SwPageFrame *pPg ) :
    m_pPage( pPg ),
    m_pCurrent( nullptr )
{
}

void SwOrderIter::Top()
{
    m_pCurrent = nullptr;
    if ( !m_pPage->GetSortedObjs() )
        return;

    const SwSortedObjs *pObjs = m_pPage->GetSortedObjs();
    if ( !pObjs->size() )
        return;

    sal_uInt32 nTopOrd = 0;
    (*pObjs)[0]->GetDrawObj()->GetOrdNum();  // force updating
    for (SwAnchoredObject* i : *pObjs)
    {
        const SdrObject* pObj = i->GetDrawObj();
        if ( dynamic_cast<const SwVirtFlyDrawObj*>( pObj) ==  nullptr )
            continue;
        sal_uInt32 nTmp = pObj->GetOrdNumDirect();
        if ( nTmp >= nTopOrd )
        {
            nTopOrd = nTmp;
            m_pCurrent = pObj;
        }
    }
}

const SdrObject *SwOrderIter::Bottom()
{
    m_pCurrent = nullptr;
    if ( m_pPage->GetSortedObjs() )
    {
        sal_uInt32 nBotOrd = USHRT_MAX;
        const SwSortedObjs *pObjs = m_pPage->GetSortedObjs();
        if ( pObjs->size() )
        {
            (*pObjs)[0]->GetDrawObj()->GetOrdNum();  // force updating
            for (SwAnchoredObject* i : *pObjs)
            {
                const SdrObject* pObj = i->GetDrawObj();
                if ( dynamic_cast<const SwVirtFlyDrawObj*>( pObj) ==  nullptr )
                    continue;
                sal_uInt32 nTmp = pObj->GetOrdNumDirect();
                if ( nTmp < nBotOrd )
                {
                    nBotOrd = nTmp;
                    m_pCurrent = pObj;
                }
            }
        }
    }
    return m_pCurrent;
}

const SdrObject *SwOrderIter::Next()
{
    const sal_uInt32 nCurOrd = m_pCurrent ? m_pCurrent->GetOrdNumDirect() : 0;
    m_pCurrent = nullptr;
    if ( m_pPage->GetSortedObjs() )
    {
        sal_uInt32 nOrd = USHRT_MAX;
        const SwSortedObjs *pObjs = m_pPage->GetSortedObjs();
        if ( pObjs->size() )
        {
            (*pObjs)[0]->GetDrawObj()->GetOrdNum();  // force updating
            for (SwAnchoredObject* i : *pObjs)
            {
                const SdrObject* pObj = i->GetDrawObj();
                if ( dynamic_cast<const SwVirtFlyDrawObj*>( pObj) ==  nullptr )
                    continue;
                sal_uInt32 nTmp = pObj->GetOrdNumDirect();
                if ( nTmp > nCurOrd && nTmp < nOrd )
                {
                    nOrd = nTmp;
                    m_pCurrent = pObj;
                }
            }
        }
    }
    return m_pCurrent;
}

void SwOrderIter::Prev()
{
    const sal_uInt32 nCurOrd = m_pCurrent ? m_pCurrent->GetOrdNumDirect() : 0;
    m_pCurrent = nullptr;
    if ( !m_pPage->GetSortedObjs() )
        return;

    const SwSortedObjs *pObjs = m_pPage->GetSortedObjs();
    if ( !pObjs->size() )
        return;

    sal_uInt32 nOrd = 0;
    (*pObjs)[0]->GetDrawObj()->GetOrdNum();  // force updating
    for (SwAnchoredObject* i : *pObjs)
    {
        const SdrObject* pObj = i->GetDrawObj();
        if ( dynamic_cast<const SwVirtFlyDrawObj*>( pObj) ==  nullptr )
            continue;
        sal_uInt32 nTmp = pObj->GetOrdNumDirect();
        if ( nTmp < nCurOrd && nTmp >= nOrd )
        {
            nOrd = nTmp;
            m_pCurrent = pObj;
        }
    }
}

/// Keep and restore the substructure of a layout frame for an action.
// New algorithm:
//   Do not look at each neighbor one by one to set all pointers correctly.
//   It is sufficient to detach a part of a chain and check if another chain needs to be added
//   when attaching it again. Only the pointers necessary for the chain connection need to be
//   adjusted. The correction happens in RestoreContent(). In between all access is restricted.
//   During this action, the Flys are detached from the page.

// #115759# - 'remove' also drawing object from page and
// at-fly anchored objects from page
static void lcl_RemoveObjsFromPage( SwFrame* _pFrame )
{
    OSL_ENSURE( _pFrame->GetDrawObjs(), "no DrawObjs in lcl_RemoveObjsFromPage." );
    SwSortedObjs &rObjs = *_pFrame->GetDrawObjs();
    for (SwAnchoredObject* pObj : rObjs)
    {
        // #115759# - reset member, at which the anchored
        // object orients its vertical position
        pObj->ClearVertPosOrientFrame();
        // #i43913#
        pObj->ResetLayoutProcessBools();
        // #115759# - remove also lower objects of as-character
        // anchored Writer fly frames from page
        if ( auto pFlyFrame = pObj->DynCastFlyFrame() )
        {
            // #115759# - remove also direct lowers of Writer
            // fly frame from page
            if ( pFlyFrame->GetDrawObjs() )
            {
                ::lcl_RemoveObjsFromPage( pFlyFrame );
            }

            SwContentFrame* pCnt = pFlyFrame->ContainsContent();
            while ( pCnt )
            {
                if ( pCnt->GetDrawObjs() )
                    ::lcl_RemoveObjsFromPage( pCnt );
                pCnt = pCnt->GetNextContentFrame();
            }
            if ( pFlyFrame->IsFlyFreeFrame() )
            {
                // #i28701# - use new method <GetPageFrame()>
                if (SwPageFrame *pPg = pFlyFrame->GetPageFrame())
                    pPg->RemoveFlyFromPage(pFlyFrame);
            }
        }
        // #115759# - remove also drawing objects from page
        else if ( auto pDrawObj = dynamic_cast<SwAnchoredDrawObject*>( pObj) )
        {
            if (pObj->GetFrameFormat().GetAnchor().GetAnchorId() != RndStdIds::FLY_AS_CHAR)
            {
                if (SwPageFrame *pPg = pObj->GetPageFrame())
                    pPg->RemoveDrawObjFromPage( *pDrawObj );
            }
        }
    }
}

SwFrame *SaveContent( SwLayoutFrame *pLay, SwFrame *pStart )
{
    if( pLay->IsSctFrame() && pLay->Lower() && pLay->Lower()->IsColumnFrame() )
        sw_RemoveFootnotes( static_cast<SwColumnFrame*>(pLay->Lower()), true, true );

    SwFrame *pSav = pLay->ContainsAny();
    if ( nullptr == pSav )
        return nullptr;

    if( pSav->IsInFootnote() && !pLay->IsInFootnote() )
    {
        do
            pSav = pSav->FindNext();
        while( pSav && pSav->IsInFootnote() );
        if( !pSav || !pLay->IsAnLower( pSav ) )
            return nullptr;
    }

    // Tables should be saved as a whole, exception:
    // The contents of a section or a cell inside a table should be saved
    if ( pSav->IsInTab() && !( ( pLay->IsSctFrame() || pLay->IsCellFrame() ) && pLay->IsInTab() ) )
        while ( !pSav->IsTabFrame() )
            pSav = pSav->GetUpper();

    if( pSav->IsInSct() )
    { // search the upmost section inside of pLay
        SwFrame* pSect = pLay->FindSctFrame();
        SwFrame *pTmp = pSav;
        do
        {
            pSav = pTmp;
            pTmp = (pSav && pSav->GetUpper()) ? pSav->GetUpper()->FindSctFrame() : nullptr;
        } while ( pTmp != pSect );
    }

    SwFrame *pFloat = pSav;
    if( !pStart )
        pStart = pSav;
    bool bGo = pStart == pSav;
    do
    {
        if( bGo )
            pFloat->GetUpper()->m_pLower = nullptr; // detach the chain part

        // search the end of the chain part, remove Flys on the way
        do
        {
            if( bGo )
            {
                if ( pFloat->IsContentFrame() )
                {
                    if ( pFloat->GetDrawObjs() )
                        ::lcl_RemoveObjsFromPage( static_cast<SwContentFrame*>(pFloat) );
                }
                else if ( pFloat->IsTabFrame() || pFloat->IsSctFrame() )
                {
                    SwContentFrame *pCnt = static_cast<SwLayoutFrame*>(pFloat)->ContainsContent();
                    if( pCnt )
                    {
                        do
                        {   if ( pCnt->GetDrawObjs() )
                                ::lcl_RemoveObjsFromPage( pCnt );
                            pCnt = pCnt->GetNextContentFrame();
                        } while ( pCnt && static_cast<SwLayoutFrame*>(pFloat)->IsAnLower( pCnt ) );
                    }
                }
                else {
                    OSL_ENSURE( !pFloat, "new FloatFrame?" );
                }
            }
            if ( pFloat->GetNext()  )
            {
                if( bGo )
                    pFloat->mpUpper = nullptr;
                pFloat = pFloat->GetNext();
                if( !bGo && pFloat == pStart )
                {
                    bGo = true;
                    pFloat->mpPrev->mpNext = nullptr;
                    pFloat->mpPrev = nullptr;
                }
            }
            else
                break;

        } while ( pFloat );

        // search next chain part and connect both chains
        SwFrame *pTmp = pFloat->FindNext();
        if( bGo )
            pFloat->mpUpper = nullptr;

        if( !pLay->IsInFootnote() )
            while( pTmp && pTmp->IsInFootnote() )
                pTmp = pTmp->FindNext();

        if ( !pLay->IsAnLower( pTmp ) )
            pTmp = nullptr;

        if ( pTmp && bGo )
        {
            pFloat->mpNext = pTmp; // connect both chains
            pFloat->mpNext->mpPrev = pFloat;
        }
        pFloat = pTmp;
        bGo = bGo || ( pStart == pFloat );
    }  while ( pFloat );

    return bGo ? pStart : nullptr;
}

// #115759# - add also drawing objects to page and at-fly
// anchored objects to page
static void lcl_AddObjsToPage( SwFrame* _pFrame, SwPageFrame* _pPage )
{
    OSL_ENSURE( _pFrame->GetDrawObjs(), "no DrawObjs in lcl_AddObjsToPage." );
    SwSortedObjs &rObjs = *_pFrame->GetDrawObjs();
    for (SwAnchoredObject* pObj : rObjs)
    {
        // #115759# - unlock position of anchored object
        // in order to get the object's position calculated.
        pObj->UnlockPosition();
        // #115759# - add also lower objects of as-character
        // anchored Writer fly frames from page
        if ( auto pFlyFrame = pObj->DynCastFlyFrame() )
        {
            if (pFlyFrame->IsFlyFreeFrame())
            {
                _pPage->AppendFlyToPage( pFlyFrame );
            }
            pFlyFrame->InvalidatePos_();
            pFlyFrame->InvalidateSize_();
            pFlyFrame->InvalidatePage( _pPage );

            // #115759# - add also at-fly anchored objects
            // to page
            if ( pFlyFrame->GetDrawObjs() )
            {
                ::lcl_AddObjsToPage( pFlyFrame, _pPage );
            }

            SwContentFrame *pCnt = pFlyFrame->ContainsContent();
            while ( pCnt )
            {
                if ( pCnt->GetDrawObjs() )
                    ::lcl_AddObjsToPage( pCnt, _pPage );
                pCnt = pCnt->GetNextContentFrame();
            }
        }
        // #115759# - remove also drawing objects from page
        else if ( dynamic_cast<const SwAnchoredDrawObject*>( pObj) !=  nullptr )
        {
            if (pObj->GetFrameFormat().GetAnchor().GetAnchorId() != RndStdIds::FLY_AS_CHAR)
            {
                pObj->InvalidateObjPos();
                _pPage->AppendDrawObjToPage(
                                *static_cast<SwAnchoredDrawObject*>(pObj) );
            }
        }
    }
}

void RestoreContent( SwFrame *pSav, SwLayoutFrame *pParent, SwFrame *pSibling )
{
    OSL_ENSURE( pSav && pParent, "no Save or Parent provided for RestoreContent." );
    SwRectFnSet aRectFnSet(pParent);

    // If there are already FlowFrames below the new parent, so add the chain (starting with pSav)
    // after the last one. The parts are inserted and invalidated if needed.
    // On the way, the Flys of the ContentFrames are registered at the page.

    SwPageFrame *pPage = pParent->FindPageFrame();

    if ( pPage )
        pPage->InvalidatePage( pPage );

    // determine predecessor and establish connection or initialize
    pSav->mpPrev = pSibling;
    SwFrame* pNxt;
    if ( pSibling )
    {
        pNxt = pSibling->mpNext;
        pSibling->mpNext = pSav;
        pSibling->InvalidatePrt_();
        pSibling->InvalidatePage( pPage );
        SwFlowFrame *pFlowFrame = dynamic_cast<SwFlowFrame*>(pSibling);
        if (pFlowFrame && pFlowFrame->GetFollow())
            pSibling->Prepare( PrepareHint::Clear, nullptr, false );
    }
    else
    {   pNxt = pParent->m_pLower;
        pParent->m_pLower = pSav;
        pSav->mpUpper = pParent; // set here already, so that it is explicit when invalidating

        if ( pSav->IsContentFrame() )
            static_cast<SwContentFrame*>(pSav)->InvalidatePage( pPage );
        else
        {   // pSav might be an empty SectFrame
            SwContentFrame* pCnt = pParent->ContainsContent();
            if( pCnt )
                pCnt->InvalidatePage( pPage );
        }
    }

    // the parent needs to grow appropriately
    SwTwips nGrowVal = 0;
    SwFrame* pLast;
    do
    {   pSav->mpUpper = pParent;
        nGrowVal += aRectFnSet.GetHeight(pSav->getFrameArea());
        pSav->InvalidateAll_();

        // register Flys, if TextFrames than also invalidate appropriately
        if ( pSav->IsContentFrame() )
        {
            if ( pSav->IsTextFrame() &&
                 static_cast<SwTextFrame*>(pSav)->GetCacheIdx() != USHRT_MAX )
                static_cast<SwTextFrame*>(pSav)->Init();  // I am its friend

            if ( pPage && pSav->GetDrawObjs() )
                ::lcl_AddObjsToPage( static_cast<SwContentFrame*>(pSav), pPage );
        }
        else
        {   SwContentFrame *pBlub = static_cast<SwLayoutFrame*>(pSav)->ContainsContent();
            if( pBlub )
            {
                do
                {   if ( pPage && pBlub->GetDrawObjs() )
                        ::lcl_AddObjsToPage( pBlub, pPage );
                    if( pBlub->IsTextFrame() && static_cast<SwTextFrame*>(pBlub)->HasFootnote() &&
                         static_cast<SwTextFrame*>(pBlub)->GetCacheIdx() != USHRT_MAX )
                        static_cast<SwTextFrame*>(pBlub)->Init(); // I am its friend
                    pBlub = pBlub->GetNextContentFrame();
                } while ( pBlub && static_cast<SwLayoutFrame*>(pSav)->IsAnLower( pBlub ));
            }
        }
        pLast = pSav;
        pSav = pSav->GetNext();

    } while ( pSav );

    if( pNxt )
    {
        pLast->mpNext = pNxt;
        pNxt->mpPrev = pLast;
    }

    pParent->Grow( nGrowVal );
}

namespace sw {

bool IsRightPageByNumber(SwRootFrame const& rLayout, sal_uInt16 const nPageNum)
{
    assert(rLayout.GetLower());
    // unfortunately can only get SwPageDesc, not SwFormatPageDesc here...
    auto const nFirstVirtPageNum(rLayout.GetLower()->GetVirtPageNum());
    bool const isFirstPageOfLayoutOdd(nFirstVirtPageNum % 2 == 1);
    return ((nPageNum % 2) == 1) == isFirstPageOfLayoutOdd;
}

} // namespace sw

SwPageFrame * InsertNewPage( SwPageDesc &rDesc, SwFrame *pUpper,
        bool const isRightPage, bool const bFirst, bool bInsertEmpty,
        bool const bFootnote,
        SwFrame *pSibling,
        bool const bVeryFirstPage )
{
    assert(pUpper);
    assert(pUpper->IsRootFrame());
    assert(!pSibling || static_cast<SwLayoutFrame const*>(pUpper)->Lower() != pSibling); // currently no insert before 1st page
    SwPageFrame *pRet;
    SwDoc *pDoc = static_cast<SwLayoutFrame*>(pUpper)->GetFormat()->GetDoc();
    if (bFirst)
    {
        if (rDesc.IsFirstShared())
        {
            // We need to fallback to left or right page format, decide it now.
            // FIXME: is this still needed?
            if (isRightPage)
            {
                rDesc.GetFirstMaster().SetFormatAttr( rDesc.GetMaster().GetHeader() );
                rDesc.GetFirstMaster().SetFormatAttr( rDesc.GetMaster().GetFooter() );
                // fdo#60250 copy margins for mirrored pages
                rDesc.GetFirstMaster().SetFormatAttr( rDesc.GetMaster().GetLRSpace() );
            }
            else
            {
                rDesc.GetFirstLeft().SetFormatAttr( rDesc.GetLeft().GetHeader() );
                rDesc.GetFirstLeft().SetFormatAttr( rDesc.GetLeft().GetFooter() );
                rDesc.GetFirstLeft().SetFormatAttr( rDesc.GetLeft().GetLRSpace() );
            }
        }
    }
    SwFrameFormat *pFormat(isRightPage ? rDesc.GetRightFormat(bFirst) : rDesc.GetLeftFormat(bFirst));
    // If there is no FrameFormat for this page, add an empty page
    if ( !pFormat )
    {
        pFormat = isRightPage ? rDesc.GetLeftFormat(bVeryFirstPage) : rDesc.GetRightFormat(bVeryFirstPage);
        OSL_ENSURE( pFormat, "Descriptor without any format?!" );
        bInsertEmpty = !bInsertEmpty;
    }
    if( bInsertEmpty )
    {
        SwPageDesc *pTmpDesc = pSibling && pSibling->GetPrev() ?
                static_cast<SwPageFrame*>(pSibling->GetPrev())->GetPageDesc() : &rDesc;
        pRet = new SwPageFrame( pDoc->GetEmptyPageFormat(), pUpper, pTmpDesc );
        SAL_INFO( "sw.pageframe", "InsertNewPage - insert empty p: " << pRet << " d: " << pTmpDesc );
        pRet->Paste( pUpper, pSibling );
        pRet->PreparePage( bFootnote );
    }
    pRet = new SwPageFrame( pFormat, pUpper, &rDesc );
    SAL_INFO( "sw.pageframe", "InsertNewPage p: " << pRet << " d: " << &rDesc << " f: " << pFormat );
    pRet->Paste( pUpper, pSibling );
    pRet->PreparePage( bFootnote );
    if ( pRet->GetNext() )
        SwRootFrame::AssertPageFlys( pRet );
    return pRet;
}

/* The following two methods search the layout structure recursively and
 * register all Flys at the page that have a Frame in this structure as an anchor.
 */

static void lcl_Regist( SwPageFrame *pPage, const SwFrame *pAnch )
{
    SwSortedObjs *pObjs = const_cast<SwSortedObjs*>(pAnch->GetDrawObjs());
    for (SwAnchoredObject* pObj : *pObjs)
    {
        if (SwFlyFrame* pFly = pObj->DynCastFlyFrame())
        {
            // register (not if already known)
            // #i28701# - use new method <GetPageFrame()>
            SwPageFrame *pPg = pFly->IsFlyFreeFrame()
                             ? pFly->GetPageFrame() : pFly->FindPageFrame();
            if ( pPg != pPage )
            {
                if ( pPg )
                    pPg->RemoveFlyFromPage( pFly );
                pPage->AppendFlyToPage( pFly );
            }
            ::RegistFlys( pPage, pFly );
        }
        else
        {
            // #i87493#
            if ( pPage != pObj->GetPageFrame() )
            {
                // #i28701#
                if (SwPageFrame *pPg = pObj->GetPageFrame())
                    pPg->RemoveDrawObjFromPage( *pObj );
                pPage->AppendDrawObjToPage( *pObj );
            }
        }

        const SwFlyFrame* pFly = pAnch->FindFlyFrame();
        if ( pFly &&
             pObj->GetDrawObj()->GetOrdNum() < pFly->GetVirtDrawObj()->GetOrdNum() &&
             pObj->GetDrawObj()->getSdrPageFromSdrObject() )
        {
            //#i119945# set pFly's OrdNum to pObj's. So when pFly is removed by Undo, the original OrdNum will not be changed.
            pObj->DrawObj()->getSdrPageFromSdrObject()->SetObjectOrdNum( pFly->GetVirtDrawObj()->GetOrdNumDirect(),
                                                         pObj->GetDrawObj()->GetOrdNumDirect() );
        }
    }
}

void RegistFlys( SwPageFrame *pPage, const SwLayoutFrame *pLay )
{
    if ( pLay->GetDrawObjs() )
        ::lcl_Regist( pPage, pLay );
    const SwFrame *pFrame = pLay->Lower();
    while ( pFrame )
    {
        if ( pFrame->IsLayoutFrame() )
            ::RegistFlys( pPage, static_cast<const SwLayoutFrame*>(pFrame) );
        else if ( pFrame->GetDrawObjs() )
            ::lcl_Regist( pPage, pFrame );
        pFrame = pFrame->GetNext();
    }
}

/// Notify the background based on the difference between old and new rectangle
void Notify( SwFlyFrame *pFly, SwPageFrame *pOld, const SwRect &rOld,
             const SwRect* pOldPrt )
{
    const SwRect aFrame( pFly->GetObjRectWithSpaces() );
    if ( rOld.Pos() != aFrame.Pos() )
    {   // changed position, invalidate old and new area
        if ( rOld.HasArea() &&
             rOld.Left()+pFly->GetFormat()->GetLRSpace().GetLeft() < FAR_AWAY )
        {
            pFly->NotifyBackground( pOld, rOld, PrepareHint::FlyFrameLeave );
        }
        pFly->NotifyBackground( pFly->FindPageFrame(), aFrame, PrepareHint::FlyFrameArrive );
    }
    else if ( rOld.SSize() != aFrame.SSize() )
    {   // changed size, invalidate the area that was left or is now overlapped
        // For simplicity, we purposely invalidate a Twip even if not needed.

        SwViewShell *pSh = pFly->getRootFrame()->GetCurrShell();
        if( pSh && rOld.HasArea() )
            pSh->InvalidateWindows( rOld );

        // #i51941# - consider case that fly frame isn't
        // registered at the old page <pOld>
        SwPageFrame* pPageFrame = pFly->FindPageFrame();
        if ( pOld != pPageFrame )
        {
            pFly->NotifyBackground( pPageFrame, aFrame, PrepareHint::FlyFrameArrive );
        }

        if ( rOld.Left() != aFrame.Left() )
        {
            SwRect aTmp( rOld );
            aTmp.Union( aFrame );
            aTmp.Left(  std::min(aFrame.Left(), rOld.Left()) );
            aTmp.Right( std::max(aFrame.Left(), rOld.Left()) );
            pFly->NotifyBackground( pOld, aTmp, PrepareHint::FlyFrameSizeChanged );
        }
        SwTwips nOld = rOld.Right();
        SwTwips nNew = aFrame.Right();
        if ( nOld != nNew )
        {
            SwRect aTmp( rOld );
            aTmp.Union( aFrame );
            aTmp.Left(  std::min(nNew, nOld) );
            aTmp.Right( std::max(nNew, nOld) );
            pFly->NotifyBackground( pOld, aTmp, PrepareHint::FlyFrameSizeChanged );
        }
        if ( rOld.Top() != aFrame.Top() )
        {
            SwRect aTmp( rOld );
            aTmp.Union( aFrame );
            aTmp.Top(    std::min(aFrame.Top(), rOld.Top()) );
            aTmp.Bottom( std::max(aFrame.Top(), rOld.Top()) );
            pFly->NotifyBackground( pOld, aTmp, PrepareHint::FlyFrameSizeChanged );
        }
        nOld = rOld.Bottom();
        nNew = aFrame.Bottom();
        if ( nOld != nNew )
        {
            SwRect aTmp( rOld );
            aTmp.Union( aFrame );
            aTmp.Top(    std::min(nNew, nOld) );
            aTmp.Bottom( std::max(nNew, nOld) );
            pFly->NotifyBackground( pOld, aTmp, PrepareHint::FlyFrameSizeChanged );
        }
    }
    else if(pOldPrt && *pOldPrt != pFly->getFramePrintArea())
    {
        bool bNotifyBackground(pFly->GetFormat()->GetSurround().IsContour());

        if(!bNotifyBackground &&
            pFly->IsFlyFreeFrame() &&
            static_cast< const SwFlyFreeFrame* >(pFly)->supportsAutoContour())
        {
            // RotateFlyFrame3: Also notify for FlyFrames which allow AutoContour
            bNotifyBackground = true;
        }

        if(bNotifyBackground)
        {
            // #i24097#
            pFly->NotifyBackground( pFly->FindPageFrame(), aFrame, PrepareHint::FlyFrameArrive );
        }
    }
}

static void lcl_CheckFlowBack( SwFrame* pFrame, const SwRect &rRect )
{
    SwTwips nBottom = rRect.Bottom();
    while( pFrame )
    {
        if( pFrame->IsLayoutFrame() )
        {
            if( rRect.Overlaps( pFrame->getFrameArea() ) )
                lcl_CheckFlowBack( static_cast<SwLayoutFrame*>(pFrame)->Lower(), rRect );
        }
        else if( !pFrame->GetNext() && nBottom > pFrame->getFrameArea().Bottom() )
        {
            if( pFrame->IsContentFrame() && static_cast<SwContentFrame*>(pFrame)->HasFollow() )
                pFrame->InvalidateSize();
            else
                pFrame->InvalidateNextPos();
        }
        pFrame = pFrame->GetNext();
    }
}

static void lcl_NotifyContent( const SdrObject *pThis, SwContentFrame *pCnt,
    const SwRect &rRect, const PrepareHint eHint )
{
    if ( !pCnt->IsTextFrame() )
        return;

    SwRect aCntPrt( pCnt->getFramePrintArea() );
    aCntPrt.Pos() += pCnt->getFrameArea().Pos();
    if ( eHint == PrepareHint::FlyFrameAttributesChanged )
    {
        // #i35640# - use given rectangle <rRect> instead
        // of current bound rectangle
        if ( aCntPrt.Overlaps( rRect ) )
            pCnt->Prepare( PrepareHint::FlyFrameAttributesChanged );
    }
    // #i23129# - only invalidate, if the text frame
    // printing area overlaps with the given rectangle.
    else if ( aCntPrt.Overlaps( rRect ) )
        pCnt->Prepare( eHint, static_cast<void*>(&aCntPrt.Intersection_( rRect )) );
    if ( !pCnt->GetDrawObjs() )
        return;

    const SwSortedObjs &rObjs = *pCnt->GetDrawObjs();
    for (SwAnchoredObject* pObj : rObjs)
    {
        if ( auto pFly = pObj->DynCastFlyFrame() )
        {
            if ( pFly->IsFlyInContentFrame() )
            {
                SwContentFrame *pContent = pFly->ContainsContent();
                while ( pContent )
                {
                    ::lcl_NotifyContent( pThis, pContent, rRect, eHint );
                    pContent = pContent->GetNextContentFrame();
                }
            }
        }
    }
}

void Notify_Background( const SdrObject* pObj,
                        SwPageFrame* pPage,
                        const SwRect& rRect,
                        const PrepareHint eHint,
                        const bool bInva )
{
    // If the frame was positioned correctly for the first time, do not inform the old area
    if ( eHint == PrepareHint::FlyFrameLeave && rRect.Top() == FAR_AWAY )
         return;

    SwLayoutFrame* pArea;
    SwFlyFrame *pFlyFrame = nullptr;
    SwFrame* pAnchor;
    if( auto pVirtFlyDrawObj = dynamic_cast<const SwVirtFlyDrawObj*>( pObj) )
    {
        pFlyFrame = const_cast<SwVirtFlyDrawObj*>(pVirtFlyDrawObj)->GetFlyFrame();
        pAnchor = pFlyFrame->AnchorFrame();
    }
    else
    {
        pFlyFrame = nullptr;
        pAnchor = const_cast<SwFrame*>(
                    GetUserCall(pObj)->GetAnchoredObj( pObj )->GetAnchorFrame() );
    }
    if( PrepareHint::FlyFrameLeave != eHint && pAnchor->IsInFly() )
        pArea = pAnchor->FindFlyFrame();
    else
        pArea = pPage;
    SwContentFrame *pCnt = nullptr;
    if ( pArea )
    {
        if( PrepareHint::FlyFrameArrive != eHint )
            lcl_CheckFlowBack( pArea, rRect );

        // Only the Flys following this anchor are reacting. Thus, those do not
        // need to be processed.
        // An exception is LEAVE, since the Fly might come "from above".
        // If the anchor is positioned on the previous page, the whole page
        // needs to be processed (47722).
        // OD 2004-05-13 #i28701# - If the wrapping style has to be considered
        // on the object positioning, the complete area has to be processed,
        // because content frames before the anchor frame also have to consider
        // the object for the text wrapping.
        // #i3317# - The complete area has always been
        // processed.
        {
            pCnt = pArea->ContainsContent();
        }
    }
    SwFrame *pLastTab = nullptr;

    bool isValidTableBeforeAnchor(false);
    while ( pCnt && pArea && pArea->IsAnLower( pCnt ) )
    {
        ::lcl_NotifyContent( pObj, pCnt, rRect, eHint );
        if ( pCnt->IsInTab() )
        {
            SwTabFrame *pTab = pCnt->FindTabFrame();
            if ( pTab != pLastTab )
            {
                pLastTab = pTab;
                isValidTableBeforeAnchor = false;
                if (PrepareHint::FlyFrameArrive == eHint
                    && pFlyFrame // TODO: do it for draw objects too?
                    && pTab->IsFollow() // table starts on previous page?
                    // "through" means they will actually overlap anyway
                    && css::text::WrapTextMode_THROUGH != pFlyFrame->GetFormat()->GetSurround().GetSurround()
                    // if it's anchored in footer it can't move to other page
                    && !pAnchor->FindFooterOrHeader())
                {
                    SwFrame * pTmp(pAnchor->GetPrev());
                    while (pTmp)
                    {
                        if (pTmp == pTab)
                        {
                            // tdf#99460 the table shouldn't be moved by the fly
                            isValidTableBeforeAnchor = true;
                            break;
                        }
                        pTmp = pTmp->GetPrev();
                    }
                }
                // #i40606# - use <GetLastBoundRect()>
                // instead of <GetCurrentBoundRect()>, because a recalculation
                // of the bounding rectangle isn't intended here.
                if (!isValidTableBeforeAnchor
                    && (pTab->getFrameArea().Overlaps(SwRect(pObj->GetLastBoundRect())) ||
                        pTab->getFrameArea().Overlaps(rRect)))
                {
                    if ( !pFlyFrame || !pFlyFrame->IsLowerOf( pTab ) )
                        pTab->InvalidatePrt();
                }
            }
            SwLayoutFrame* pCell = pCnt->GetUpper();
            // #i40606# - use <GetLastBoundRect()>
            // instead of <GetCurrentBoundRect()>, because a recalculation
            // of the bounding rectangle isn't intended here.
            if (!isValidTableBeforeAnchor && pCell->IsCellFrame() &&
                 ( pCell->getFrameArea().Overlaps( SwRect(pObj->GetLastBoundRect()) ) ||
                   pCell->getFrameArea().Overlaps( rRect ) ) )
            {
                const SwFormatVertOrient &rOri = pCell->GetFormat()->GetVertOrient();
                if ( text::VertOrientation::NONE != rOri.GetVertOrient() )
                    pCell->InvalidatePrt();
            }
        }
        pCnt = pCnt->GetNextContentFrame();
    }
    // #128702# - make code robust
    if ( pPage && pPage->GetSortedObjs() )
    {
        pObj->GetOrdNum();
        const SwSortedObjs &rObjs = *pPage->GetSortedObjs();
        for (SwAnchoredObject* pAnchoredObj : rObjs)
        {
            if ( pAnchoredObj->DynCastFlyFrame() !=  nullptr )
            {
                if( pAnchoredObj->GetDrawObj() == pObj )
                    continue;
                SwFlyFrame *pFly = static_cast<SwFlyFrame*>(pAnchoredObj);
                if ( pFly->getFrameArea().Top() == FAR_AWAY )
                    continue;

                if ( !pFlyFrame ||
                        (!pFly->IsLowerOf( pFlyFrame ) &&
                        pFly->GetVirtDrawObj()->GetOrdNumDirect() < pObj->GetOrdNumDirect()))
                {
                    pCnt = pFly->ContainsContent();
                    while ( pCnt )
                    {
                        ::lcl_NotifyContent( pObj, pCnt, rRect, eHint );
                        pCnt = pCnt->GetNextContentFrame();
                    }
                }
                if( pFly->IsFlyLayFrame() )
                {
                    if( pFly->Lower() && pFly->Lower()->IsColumnFrame() &&
                        pFly->getFrameArea().Bottom() >= rRect.Top() &&
                        pFly->getFrameArea().Top() <= rRect.Bottom() &&
                        pFly->getFrameArea().Right() >= rRect.Left() &&
                        pFly->getFrameArea().Left() <= rRect.Right() )
                    {
                        pFly->InvalidateSize();
                    }
                }
                // Flys above myself might sidestep if they have an automatic
                // alignment. This happens independently of my attributes since
                // this might have been changed as well.
                else if ( pFly->IsFlyAtContentFrame() &&
                        pObj->GetOrdNumDirect() <
                        pFly->GetVirtDrawObj()->GetOrdNumDirect() &&
                        pFlyFrame && !pFly->IsLowerOf( pFlyFrame ) )
                {
                    const SwFormatHoriOrient &rH = pFly->GetFormat()->GetHoriOrient();
                    if ( text::HoriOrientation::NONE != rH.GetHoriOrient()  &&
                            text::HoriOrientation::CENTER != rH.GetHoriOrient()  &&
                            ( !pFly->IsAutoPos() || text::RelOrientation::CHAR != rH.GetRelationOrient() ) &&
                            (pFly->getFrameArea().Bottom() >= rRect.Top() &&
                            pFly->getFrameArea().Top() <= rRect.Bottom()) )
                        pFly->InvalidatePos();
                }
            }
        }
    }
    if ( pFlyFrame && pAnchor->GetUpper() && pAnchor->IsInTab() )//MA_FLY_HEIGHT
        pAnchor->GetUpper()->InvalidateSize();

    // #i82258# - make code robust
    SwViewShell* pSh = nullptr;
    if ( bInva && pPage &&
        nullptr != (pSh = pPage->getRootFrame()->GetCurrShell()) )
    {
        pSh->InvalidateWindows( rRect );
    }
}

/// Provides the Upper of an anchor in paragraph-bound objects. If the latter
/// is a chained border or a footnote, the "virtual" Upper might be returned.
const SwFrame* GetVirtualUpper( const SwFrame* pFrame, const Point& rPos )
{
    if( pFrame->IsTextFrame() )
    {
        pFrame = pFrame->GetUpper();
        if( !pFrame->getFrameArea().Contains( rPos ) )
        {
            if( pFrame->IsFootnoteFrame() )
            {
                const SwFootnoteFrame* pTmp = static_cast<const SwFootnoteFrame*>(pFrame)->GetFollow();
                while( pTmp )
                {
                    if( pTmp->getFrameArea().Contains( rPos ) )
                        return pTmp;
                    pTmp = pTmp->GetFollow();
                }
            }
            else
            {
                SwFlyFrame* pTmp = const_cast<SwFlyFrame*>(pFrame->FindFlyFrame());
                while( pTmp )
                {
                    if( pTmp->getFrameArea().Contains( rPos ) )
                        return pTmp;
                    pTmp = pTmp->GetNextLink();
                }
            }
        }
    }
    return pFrame;
}

bool Is_Lower_Of(const SwFrame *pCurrFrame, const SdrObject* pObj)
{
    Point aPos;
    const SwFrame* pFrame;
    if (const SwVirtFlyDrawObj *pFlyDrawObj = dynamic_cast<const SwVirtFlyDrawObj*>(pObj))
    {
        const SwFlyFrame* pFly = pFlyDrawObj->GetFlyFrame();
        pFrame = pFly->GetAnchorFrame();
        aPos = pFly->getFrameArea().Pos();
    }
    else
    {
        pFrame = static_cast<SwDrawContact*>(GetUserCall(pObj))->GetAnchorFrame(pObj);
        aPos = pObj->GetCurrentBoundRect().TopLeft();
    }
    OSL_ENSURE( pFrame, "8-( Fly is lost in Space." );
    pFrame = GetVirtualUpper( pFrame, aPos );
    do
    {   if ( pFrame == pCurrFrame )
            return true;
        if( pFrame->IsFlyFrame() )
        {
            aPos = pFrame->getFrameArea().Pos();
            pFrame = GetVirtualUpper( static_cast<const SwFlyFrame*>(pFrame)->GetAnchorFrame(), aPos );
        }
        else
            pFrame = pFrame->GetUpper();
    } while ( pFrame );
    return false;
}

/// provides the area of a frame in that no Fly from another area can overlap
const SwFrame *FindContext( const SwFrame *pFrame, SwFrameType nAdditionalContextType )
{
    const SwFrameType nTyp = SwFrameType::Root | SwFrameType::Header   | SwFrameType::Footer | SwFrameType::FtnCont  |
                        SwFrameType::Ftn  | SwFrameType::Fly      |
                        SwFrameType::Tab  | SwFrameType::Row      | SwFrameType::Cell |
                        nAdditionalContextType;
    do
    {   if ( pFrame->GetType() & nTyp )
            break;
        pFrame = pFrame->GetUpper();
    } while( pFrame );
    return pFrame;
}

bool IsFrameInSameContext( const SwFrame *pInnerFrame, const SwFrame *pFrame )
{
    const SwFrame *pContext = FindContext( pInnerFrame, SwFrameType::None );

    const SwFrameType nTyp = SwFrameType::Root | SwFrameType::Header | SwFrameType::Footer | SwFrameType::FtnCont |
                        SwFrameType::Ftn  | SwFrameType::Fly      |
                        SwFrameType::Tab  | SwFrameType::Row      | SwFrameType::Cell;
    do
    {   if ( pFrame->GetType() & nTyp )
        {
            if( pFrame == pContext )
                return true;
            if( pFrame->IsCellFrame() )
                return false;
        }
        if( pFrame->IsFlyFrame() )
        {
            Point aPos( pFrame->getFrameArea().Pos() );
            pFrame = GetVirtualUpper( static_cast<const SwFlyFrame*>(pFrame)->GetAnchorFrame(), aPos );
        }
        else
            pFrame = pFrame->GetUpper();
    } while( pFrame );

    return false;
}

static SwTwips lcl_CalcCellRstHeight( SwLayoutFrame *pCell )
{
    SwFrame *pLow = pCell->Lower();
    if ( pLow && (pLow->IsContentFrame() || pLow->IsSctFrame()) )
    {
        tools::Long nHeight = 0, nFlyAdd = 0;
        do
        {
            tools::Long nLow = pLow->getFrameArea().Height();
            if( pLow->IsTextFrame() && static_cast<SwTextFrame*>(pLow)->IsUndersized() )
                nLow += static_cast<SwTextFrame*>(pLow)->GetParHeight()-pLow->getFramePrintArea().Height();
            else if( pLow->IsSctFrame() && static_cast<SwSectionFrame*>(pLow)->IsUndersized() )
                nLow += static_cast<SwSectionFrame*>(pLow)->Undersize();
            nFlyAdd = std::max( tools::Long(0), nFlyAdd - nLow );
            nFlyAdd = std::max( nFlyAdd, ::CalcHeightWithFlys( pLow ) );
            nHeight += nLow;
            pLow = pLow->GetNext();
        } while ( pLow );
        if ( nFlyAdd )
            nHeight += nFlyAdd;

        // The border cannot be calculated based on PrtArea and Frame, since both can be invalid.
        SwBorderAttrAccess aAccess( SwFrame::GetCache(), pCell );
        const SwBorderAttrs &rAttrs = *aAccess.Get();
        nHeight += rAttrs.CalcTop() + rAttrs.CalcBottom();

        return pCell->getFrameArea().Height() - nHeight;
    }
    else
    {
        tools::Long nRstHeight = 0;
        while (pLow && pLow->IsLayoutFrame())
        {
            nRstHeight += ::CalcRowRstHeight(static_cast<SwLayoutFrame*>(pLow));
            pLow = pLow->GetNext();
        }
        return nRstHeight;
    }
}

SwTwips CalcRowRstHeight( SwLayoutFrame *pRow )
{
    SwFrame *pLow = pRow->Lower();
    if (!(pLow && pLow->IsLayoutFrame()))
    {
        return 0;
    }
    SwTwips nRstHeight = LONG_MAX;
    while (pLow && pLow->IsLayoutFrame())
    {
        nRstHeight = std::min(nRstHeight, ::lcl_CalcCellRstHeight(static_cast<SwLayoutFrame*>(pLow)));
        pLow = pLow->GetNext();
    }
    return nRstHeight;
}

const SwFrame* FindPage( const SwRect &rRect, const SwFrame *pPage )
{
    if ( !rRect.Overlaps( pPage->getFrameArea() ) )
    {
        const SwRootFrame* pRootFrame = static_cast<const SwRootFrame*>(pPage->GetUpper());
        const SwFrame* pTmpPage = pRootFrame ? pRootFrame->GetPageAtPos( rRect.TopLeft(), &rRect.SSize(), true ) : nullptr;
        if ( pTmpPage )
            pPage = pTmpPage;
    }

    return pPage;
}

namespace {

class SwFrameHolder : private SfxListener
{
    SwFrame* m_pFrame;
    bool m_bSet;
    virtual void Notify(  SfxBroadcaster& rBC, const SfxHint& rHint ) override;
public:
    SwFrameHolder()
        : m_pFrame(nullptr)
        , m_bSet(false)
    {
    }
    void SetFrame( SwFrame* pHold );
    SwFrame* GetFrame() { return m_pFrame; }
    void Reset();
    bool IsSet() const { return m_bSet; }
};

}

void SwFrameHolder::SetFrame( SwFrame* pHold )
{
    m_bSet = true;
    if (m_pFrame != pHold)
    {
        if (m_pFrame)
            EndListening(*m_pFrame);
        StartListening(*pHold);
        m_pFrame = pHold;
    }
}

void SwFrameHolder::Reset()
{
    if (m_pFrame)
        EndListening(*m_pFrame);
    m_bSet = false;
    m_pFrame = nullptr;
}

void SwFrameHolder::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if (rHint.GetId() == SfxHintId::Dying && &rBC == m_pFrame)
    {
        m_pFrame = nullptr;
    }
}

SwFrame* GetFrameOfModify(SwRootFrame const*const pLayout, sw::BroadcastingModify const& rMod,
        SwFrameType const nFrameType, SwPosition const*const pPos,
        std::pair<Point, bool> const*const pViewPosAndCalcFrame)
{
    SwFrame *pMinFrame = nullptr, *pTmpFrame;
    SwFrameHolder aHolder;
    SwRect aCalcRect;
    bool bClientIterChanged = false;

    SwIterator<SwFrame, sw::BroadcastingModify, sw::IteratorMode::UnwrapMulti> aIter(rMod);
    do {
        pMinFrame = nullptr;
        aHolder.Reset();
        sal_uInt64 nMinDist = 0;
        bClientIterChanged = false;

        for( pTmpFrame = aIter.First(); pTmpFrame; pTmpFrame = aIter.Next() )
        {
            if( pTmpFrame->GetType() & nFrameType &&
                ( !pLayout || pLayout == pTmpFrame->getRootFrame() ) &&
                (!pTmpFrame->IsFlowFrame() ||
                 !SwFlowFrame::CastFlowFrame( pTmpFrame )->IsFollow() ))
            {
                if (pViewPosAndCalcFrame)
                {
                    // watch for Frame being deleted
                    if ( pMinFrame )
                        aHolder.SetFrame( pMinFrame );
                    else
                        aHolder.Reset();

                    if (pViewPosAndCalcFrame->second)
                    {
                        // tdf#108118 prevent recursion
                        DisableCallbackAction a(*pTmpFrame->getRootFrame());
                        // - format parent Writer
                        // fly frame, if it isn't been formatted yet.
                        // Note: The Writer fly frame could be the frame itself.
                        SwFlyFrame* pFlyFrame( pTmpFrame->FindFlyFrame() );
                        if ( pFlyFrame &&
                             pFlyFrame->getFrameArea().Pos().X() == FAR_AWAY &&
                             pFlyFrame->getFrameArea().Pos().Y() == FAR_AWAY )
                        {
                            SwObjectFormatter::FormatObj( *pFlyFrame );
                        }
                        pTmpFrame->Calc(pLayout ? pLayout->GetCurrShell()->GetOut() : nullptr);
                    }

                    // aIter.IsChanged checks if the current pTmpFrame has been deleted while
                    // it is the current iterator
                    // FrameHolder watches for deletion of the current pMinFrame
                    if( aIter.IsChanged() || ( aHolder.IsSet() && !aHolder.GetFrame() ) )
                    {
                        // restart iteration
                        bClientIterChanged = true;
                        break;
                    }

                    // for Flys go via the parent if the Fly is not yet "formatted"
                    if (!pViewPosAndCalcFrame->second &&
                        pTmpFrame->GetType() & SwFrameType::Fly &&
                        static_cast<SwFlyFrame*>(pTmpFrame)->GetAnchorFrame() &&
                        FAR_AWAY == pTmpFrame->getFrameArea().Pos().getX() &&
                        FAR_AWAY == pTmpFrame->getFrameArea().Pos().getY() )
                        aCalcRect = static_cast<SwFlyFrame*>(pTmpFrame)->GetAnchorFrame()->getFrameArea();
                    else
                        aCalcRect = pTmpFrame->getFrameArea();

                    if (aCalcRect.Contains(pViewPosAndCalcFrame->first))
                    {
                        pMinFrame = pTmpFrame;
                        break;
                    }

                    // Point not in rectangle. Compare distances:
                    const Point aCalcRectCenter = aCalcRect.Center();
                    const Point aDiff = aCalcRectCenter - pViewPosAndCalcFrame->first;
                    const sal_uInt64 nCurrentDist = sal_Int64(aDiff.getX()) * sal_Int64(aDiff.getX()) + sal_Int64(aDiff.getY()) * sal_Int64(aDiff.getY()); // opt: no sqrt
                    if ( !pMinFrame || nCurrentDist < nMinDist )
                    {
                        pMinFrame = pTmpFrame;
                        nMinDist = nCurrentDist;
                    }
                }
                else
                {
                    // if no pViewPosAndCalcFrame is provided, take the first one
                    pMinFrame = pTmpFrame;
                    break;
                }
            }
        }
    } while( bClientIterChanged );

    if( pPos && pMinFrame && pMinFrame->IsTextFrame() )
        return static_cast<SwTextFrame*>(pMinFrame)->GetFrameAtPos( *pPos );

    return pMinFrame;
}

bool IsExtraData( const SwDoc *pDoc )
{
    const SwLineNumberInfo &rInf = pDoc->GetLineNumberInfo();
    return rInf.IsPaintLineNumbers() ||
           rInf.IsCountInFlys() ||
           (static_cast<sal_Int16>(SW_MOD()->GetRedlineMarkPos()) != text::HoriOrientation::NONE &&
            !pDoc->getIDocumentRedlineAccess().GetRedlineTable().empty()) ||
            (pDoc->GetEditShell() && pDoc->GetEditShell()->GetViewOptions() &&
             pDoc->GetEditShell()->GetViewOptions()->IsShowOutlineContentVisibilityButton());
}

// OD 22.09.2003 #110978#
SwRect SwPageFrame::PrtWithoutHeaderAndFooter() const
{
    SwRect aPrtWithoutHeaderFooter( getFramePrintArea() );
    aPrtWithoutHeaderFooter.Pos() += getFrameArea().Pos();

    const SwFrame* pLowerFrame = Lower();
    while ( pLowerFrame )
    {
        // Note: independent on text direction page header and page footer are
        //       always at top respectively at bottom of the page frame.
        if ( pLowerFrame->IsHeaderFrame() )
        {
            aPrtWithoutHeaderFooter.AddTop( pLowerFrame->getFrameArea().Height() );
        }
        if ( pLowerFrame->IsFooterFrame() )
        {
            aPrtWithoutHeaderFooter.AddBottom( - pLowerFrame->getFrameArea().Height() );
        }

        pLowerFrame = pLowerFrame->GetNext();
    }

    return aPrtWithoutHeaderFooter;
}

/** method to determine the spacing values of a frame

    OD 2004-03-10 #i28701#
    OD 2009-08-28 #i102458#
    Add output parameter <obIsLineSpacingProportional>
*/
void GetSpacingValuesOfFrame( const SwFrame& rFrame,
                            SwTwips& onLowerSpacing,
                            SwTwips& onLineSpacing,
                            bool& obIsLineSpacingProportional,
                            bool bIdenticalStyles )
{
    if ( !rFrame.IsFlowFrame() )
    {
        onLowerSpacing = 0;
        onLineSpacing = 0;
    }
    else
    {
        const SvxULSpaceItem& rULSpace = rFrame.GetAttrSet()->GetULSpace();
        // check contextual spacing if the style of actual and next paragraphs are identical
        if (bIdenticalStyles)
            onLowerSpacing = (rULSpace.GetContext() ? 0 : rULSpace.GetLower());
        else
            onLowerSpacing = rULSpace.GetLower();

        onLineSpacing = 0;
        obIsLineSpacingProportional = false;
        if ( rFrame.IsTextFrame() )
        {
            onLineSpacing = static_cast<const SwTextFrame&>(rFrame).GetLineSpace();
            obIsLineSpacingProportional =
                onLineSpacing != 0 &&
                static_cast<const SwTextFrame&>(rFrame).GetLineSpace( true ) == 0;
        }

        OSL_ENSURE( onLowerSpacing >= 0 && onLineSpacing >= 0,
                "<GetSpacingValuesOfFrame(..)> - spacing values aren't positive!" );
    }
}

/// get the content of the table cell, skipping content from nested tables
const SwContentFrame* GetCellContent( const SwLayoutFrame& rCell )
{
    const SwContentFrame* pContent = rCell.ContainsContent();
    const SwTabFrame* pTab = rCell.FindTabFrame();

    while ( pContent && rCell.IsAnLower( pContent ) )
    {
        const SwTabFrame* pTmpTab = pContent->FindTabFrame();
        if ( pTmpTab != pTab )
        {
            SwFrame const*const pTmp = pTmpTab->FindLastContentOrTable();
            if (pTmp)
            {
                pContent = pTmp->FindNextCnt();
            }
            else
            {
                pContent = nullptr;
            }
        }
        else
            break;
    }
    return pContent;
}

SwDeletionChecker::SwDeletionChecker(const SwFrame* pFrame)
    : mpFrame( pFrame )
    , mpRegIn( pFrame
        ? pFrame->IsTextFrame()
            // sw_redlinehide: GetDep() may be a member of SwTextFrame!
            ? static_cast<SwTextFrame const*>(pFrame)->GetTextNodeFirst()
            : const_cast<SwFrame*>(pFrame)->GetDep()
        : nullptr )
{
}

/// Can be used to check if a frame has been deleted
bool SwDeletionChecker::HasBeenDeleted() const
{
    if ( !mpFrame || !mpRegIn )
        return false;

    SwIterator<SwFrame, sw::BroadcastingModify, sw::IteratorMode::UnwrapMulti> aIter(*mpRegIn);
    SwFrame* pLast = aIter.First();
    while ( pLast )
    {
        if ( pLast == mpFrame )
            return false;
        pLast = aIter.Next();
    }

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
