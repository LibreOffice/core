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

#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/lspcitem.hxx>

#include <drawdoc.hxx>
#include <fmtornt.hxx>
#include <fmthdft.hxx>
#include <fmtfsize.hxx>
#include <fmtsrnd.hxx>
#include <docary.hxx>
#include <lineinfo.hxx>
#include <swmodule.hxx>
#include "pagefrm.hxx"
#include "colfrm.hxx"
#include "fesh.hxx"
#include "viewimp.hxx"
#include "viewopt.hxx"
#include "dflyobj.hxx"
#include "dcontact.hxx"
#include "frmtool.hxx"
#include "tabfrm.hxx"
#include "rowfrm.hxx"
#include "ftnfrm.hxx"
#include "txtfrm.hxx"
#include "notxtfrm.hxx"
#include "flyfrms.hxx"
#include "layact.hxx"
#include "pagedesc.hxx"
#include "section.hxx"
#include "sectfrm.hxx"
#include "node2lay.hxx"
#include "ndole.hxx"
#include "hints.hxx"
#include <layhelp.hxx>
#include <laycache.hxx>
#include <rootfrm.hxx>
#include <paratr.hxx>
#include <sortedobjs.hxx>
#include <objectformatter.hxx>
#include <calbck.hxx>
#include <DocumentSettingManager.hxx>
#include <IDocumentTimerAccess.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentState.hxx>

//UUUU
#include <svx/sdr/attribute/sdrallfillattributeshelper.hxx>

using namespace ::com::sun::star;

bool bObjsDirect = true;
bool bDontCreateObjects = false;
bool bSetCompletePaintOnInvalidate = false;

sal_uInt8 StackHack::nCnt = 0;
bool StackHack::bLocked = false;

SwFrameNotify::SwFrameNotify( SwFrame *pF ) :
    mpFrame( pF ),
    maFrame( pF->Frame() ),
    maPrt( pF->Prt() ),
    mbInvaKeep( false ),
    mbValidSize( pF->GetValidSizeFlag() ),
    mbFrameDeleted( false )     // #i49383#
{
    if ( pF->IsTextFrame() )
    {
        mnFlyAnchorOfst = static_cast<SwTextFrame*>(pF)->GetBaseOfstForFly( true );
        mnFlyAnchorOfstNoWrap = static_cast<SwTextFrame*>(pF)->GetBaseOfstForFly( false );
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
    // #i49383#
    if ( mbFrameDeleted )
    {
        return;
    }

    SWRECTFN( mpFrame )
    const bool bAbsP = POS_DIFF( maFrame, mpFrame->Frame() );
    const bool bChgWidth =
            (maFrame.*fnRect->fnGetWidth)() != (mpFrame->Frame().*fnRect->fnGetWidth)();
    const bool bChgHeight =
            (maFrame.*fnRect->fnGetHeight)()!=(mpFrame->Frame().*fnRect->fnGetHeight)();
    const bool bChgFlyBasePos = mpFrame->IsTextFrame() &&
       ( ( mnFlyAnchorOfst != static_cast<SwTextFrame*>(mpFrame)->GetBaseOfstForFly( true ) ) ||
         ( mnFlyAnchorOfstNoWrap != static_cast<SwTextFrame*>(mpFrame)->GetBaseOfstForFly( false ) ) );

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
                        bool bInvalidPrePos = SwFlowFrame::CastFlowFrame( pPre )->IsKeep( *pPre->GetAttrSet() ) && pPre->GetIndPrev();

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
                long nOldHeight = (maFrame.*fnRect->fnGetHeight)();
                long nNewHeight = (mpFrame->Frame().*fnRect->fnGetHeight)();
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
                (maFrame.*fnRect->fnTopDist)( (mpFrame->Frame().*fnRect->fnGetTop)() ) > 0 )
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
            (maPrt.*fnRect->fnGetWidth)() != (mpFrame->Prt().*fnRect->fnGetWidth)();
    const bool bPrtHeight =
            (maPrt.*fnRect->fnGetHeight)()!=(mpFrame->Prt().*fnRect->fnGetHeight)();
    if ( bPrtWidth || bPrtHeight )
    {
        //UUUU
        bool bUseNewFillProperties(false);
        if (mpFrame->supportsFullDrawingLayerFillAttributeSet())
        {
            drawinglayer::attribute::SdrAllFillAttributesHelperPtr aFillAttributes(mpFrame->getSdrAllFillAttributesHelper());
            if(aFillAttributes.get() && aFillAttributes->isUsed())
            {
                bUseNewFillProperties = true;
                //UUUU use SetCompletePaint if needed
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

    const bool bPrtP = POS_DIFF( maPrt, mpFrame->Prt() );
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
            for ( size_t i = 0; i < rObjs.size(); ++i )
            {
                // OD 2004-03-31 #i26791# - no general distinction between
                // Writer fly frames and drawing objects
                bool bNotify = false;
                bool bNotifySize = false;
                SwAnchoredObject* pObj = rObjs[i];
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
                             dynamic_cast<const SwFlyFrame*>( pObj) !=  nullptr )
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
                    if ( dynamic_cast<const SwFlyFrame*>( pObj) !=  nullptr )
                    {
                        SwFlyFrame* pFlyFrame = static_cast<SwFlyFrame*>(pObj);
                        if ( bNotifySize )
                            pFlyFrame->_InvalidateSize();
                        // #115759# - no invalidation of
                        // position for as-character anchored objects.
                        if ( !bAnchoredAsChar )
                        {
                            pFlyFrame->_InvalidatePos();
                        }
                        pFlyFrame->_Invalidate();
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
    else if( mpFrame->IsTextFrame() && mbValidSize != mpFrame->GetValidSizeFlag() )
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
    if ( !mpFrame->IsFlyFrame() && nullptr != ( pFly = mpFrame->ImplFindFlyFrame() ) )
    {
        // #i61999#
        // no invalidation of columned Writer fly frames, because automatic
        // width doesn't make sense for such Writer fly frames.
        if ( pFly->Lower() && !pFly->Lower()->IsColumnFrame() )
        {
            const SwFormatFrameSize &rFrameSz = pFly->GetFormat()->GetFrameSize();

            // This could be optimized. Basically the fly frame only has to
            // be invalidated, if the first line of pFrame (if pFrame is a content
            // frame, for other frame types its the print area) has changed its
            // size and pFrame was responsible for the current width of pFly. On
            // the other hand, this is only rarely used and re-calculation of
            // the fly frame does not cause too much trouble. So we keep it this
            // way:
            if ( ATT_FIX_SIZE != rFrameSz.GetWidthSizeType() )
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
    }
}

SwLayNotify::SwLayNotify( SwLayoutFrame *pLayFrame ) :
    SwFrameNotify( pLayFrame ),
    bLowersComplete( false )
{
}

// OD 2004-05-11 #i28701# - local method to invalidate the position of all
// frames inclusive its floating screen objects, which are lowers of the given
// layout frame
static void lcl_InvalidatePosOfLowers( SwLayoutFrame& _rLayoutFrame )
{
    if( _rLayoutFrame.IsFlyFrame() && _rLayoutFrame.GetDrawObjs() )
    {
        _rLayoutFrame.InvalidateObjs( true, false );
    }

    SwFrame* pLowerFrame = _rLayoutFrame.Lower();
    while ( pLowerFrame )
    {
        pLowerFrame->InvalidatePos();
        if ( pLowerFrame->IsTextFrame() )
        {
            static_cast<SwTextFrame*>(pLowerFrame)->Prepare( PREP_POS_CHGD );
        }
        else if ( pLowerFrame->IsTabFrame() )
        {
            pLowerFrame->InvalidatePrt();
        }

        pLowerFrame->InvalidateObjs( true, false );

        pLowerFrame = pLowerFrame->GetNext();
    }
}

SwLayNotify::~SwLayNotify()
{
    // #i49383#
    if ( mbFrameDeleted )
    {
        return;
    }

    SwLayoutFrame *pLay = GetLay();
    SWRECTFN( pLay )
    bool bNotify = false;
    if ( pLay->Prt().SSize() != maPrt.SSize() )
    {
        if ( !IsLowersComplete() )
        {
            bool bInvaPercent;

            if ( pLay->IsRowFrame() )
            {
                bInvaPercent = true;
                long nNew = (pLay->Prt().*fnRect->fnGetHeight)();
                if( nNew != (maPrt.*fnRect->fnGetHeight)() )
                     static_cast<SwRowFrame*>(pLay)->AdjustCells( nNew, true);
                if( (pLay->Prt().*fnRect->fnGetWidth)()
                    != (maPrt.*fnRect->fnGetWidth)() )
                     static_cast<SwRowFrame*>(pLay)->AdjustCells( 0, false );
            }
            else
            {
                //Proportional adoption of the internal.
                //1. If the formatted is no Fly
                //2. If he contains no columns
                //3. If the Fly has a fixed hight and the columns
                //   are next to be.
                //   Hoehe danebenliegen.
                //4. Never at SectionFrames.
                bool bLow;
                if( pLay->IsFlyFrame() )
                {
                    if ( pLay->Lower() )
                    {
                        bLow = !pLay->Lower()->IsColumnFrame() ||
                            (pLay->Lower()->Frame().*fnRect->fnGetHeight)()
                             != (pLay->Prt().*fnRect->fnGetHeight)();
                    }
                    else
                        bLow = false;
                }
                else if( pLay->IsSctFrame() )
                {
                    if ( pLay->Lower() )
                    {
                        if( pLay->Lower()->IsColumnFrame() && pLay->Lower()->GetNext() )
                            bLow = pLay->Lower()->Frame().Height() != pLay->Prt().Height();
                        else
                            bLow = pLay->Prt().Width() != maPrt.Width();
                    }
                    else
                        bLow = false;
                }
                else if( pLay->IsFooterFrame() && !pLay->HasFixSize() )
                    bLow = pLay->Prt().Width() != maPrt.Width();
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
                if ( (pLay->Prt().Height() > maPrt.Height() ||
                      pLay->Prt().Width()  > maPrt.Width()) &&
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
                pLay->InvaPercentLowers( pLay->Prt().Height() - maPrt.Height() );
        }
        if ( pLay->IsTabFrame() )
            //So that _only_ the shadow is drawn while resizing.
            static_cast<SwTabFrame*>(pLay)->SetComplete();
        else
        {
            const SwViewShell *pSh = pLay->getRootFrame()->GetCurrShell();
            if( !( pSh && pSh->GetViewOptions()->getBrowseMode() ) ||
                  !(pLay->GetType() & (FRM_BODY | FRM_PAGE)) )
            //Thereby the subordinates are retouched clean.
            //Example problem: Take the Flys with the handles and downsize.
            //Not for body and page, otherwise it flickers when loading HTML.
            pLay->SetCompletePaint();
        }
    }
    //Notify Lower if the position has changed.
    const bool bPrtPos = POS_DIFF( maPrt, pLay->Prt() );
    const bool bPos = bPrtPos || POS_DIFF( maFrame, pLay->Frame() );
    const bool bSize = pLay->Frame().SSize() != maFrame.SSize();

    if ( bPos && pLay->Lower() && !IsLowersComplete() )
        pLay->Lower()->InvalidatePos();

    if ( bPrtPos )
        pLay->SetCompletePaint();

    //Inform the Follower if the SSize has changed.
    if ( bSize )
    {
        if( pLay->GetNext() )
        {
            if ( pLay->GetNext()->IsLayoutFrame() )
                pLay->GetNext()->_InvalidatePos();
            else
                pLay->GetNext()->InvalidatePos();
        }
        else if( pLay->IsSctFrame() )
            pLay->InvalidateNextPos();
    }
    if ( !IsLowersComplete() &&
         !(pLay->GetType()&(FRM_FLY|FRM_SECTION) &&
            pLay->Lower() && pLay->Lower()->IsColumnFrame()) &&
         (bPos || bNotify) &&
         !(pLay->GetType() & (FRM_ROW|FRM_TAB|FRM_FTNCONT|FRM_PAGE|FRM_ROOT)))
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
    // #i49383#
    if ( mbFrameDeleted )
    {
        return;
    }

    SwFlyFrame *pFly = GetFly();
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
                pFly->AnchorFrame()->Prepare( PREP_FLY_LEAVE );
            }
        }
        pFly->ResetNotifyBack();
    }

    //Have the size or the position changed,
    //so should the view know this.
    SWRECTFN( pFly )
    const bool bPosChgd = POS_DIFF( maFrame, pFly->Frame() );
    const bool bFrameChgd = pFly->Frame().SSize() != maFrame.SSize();
    const bool bPrtChgd = maPrt != pFly->Prt();
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
            if ( pNxt )
            {
                pNxt->InvalidatePos();
            }
        }

        // #i26945# - notify anchor.
        // Needed for negative positioned Writer fly frames
        if ( pFly->GetAnchorFrame()->IsTextFrame() )
        {
            pFly->AnchorFrame()->Prepare( PREP_FLY_LEAVE );
        }
    }

    // OD 2004-05-13 #i28701#
    // #i45180# - no adjustment of layout process flags and
    // further notifications/invalidations, if format is called by grow/shrink
    if ( pFly->ConsiderObjWrapInfluenceOnObjPos() &&
         ( dynamic_cast<const SwFlyFreeFrame*>( pFly) ==  nullptr ||
           !static_cast<SwFlyFreeFrame*>(pFly)->IsNoMoveOnCheckClip() ) )
    {
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

            if ( !pFly->ConsiderForTextWrap() )
            {
                // indicate that object has to be considered for text wrap
                pFly->SetConsiderForTextWrap( true );
                // invalidate 'background' in order to allow its 'background'
                // to wrap around it.
                pFly->NotifyBackground( pFly->GetPageFrame(),
                                        pFly->GetObjRectWithSpaces(),
                                        PREP_FLY_ARRIVE );
                // invalidate position of anchor frame in order to force
                // a re-format of the anchor frame, which also causes a
                // re-format of the invalid previous frames of the anchor frame.
                pFly->AnchorFrame()->InvalidatePos();
            }
        }
    }
}

SwContentFrame *SwContentNotify::GetCnt()
{
    return static_cast<SwContentFrame*>(mpFrame);
}

SwContentNotify::SwContentNotify( SwContentFrame *pContentFrame ) :
    SwFrameNotify( pContentFrame ),
    // OD 08.01.2004 #i11859#
    mbChkHeightOfLastLine( false ),
    mnHeightOfLastLine( 0L ),
    // OD 2004-02-26 #i25029#
    mbInvalidatePrevPrtArea( false ),
    mbBordersJoinedWithPrev( false )
{
    // OD 08.01.2004 #i11859#
    if ( pContentFrame->IsTextFrame() )
    {
        SwTextFrame* pTextFrame = static_cast<SwTextFrame*>(pContentFrame);
        if ( !pTextFrame->GetTextNode()->getIDocumentSettingAccess()->get(DocumentSettingId::OLD_LINE_SPACING) )
        {
            const SwAttrSet* pSet = pTextFrame->GetAttrSet();
            const SvxLineSpacingItem &rSpace = pSet->GetLineSpacing();
            if ( rSpace.GetInterLineSpaceRule() == SVX_INTER_LINE_SPACE_PROP )
            {
                mbChkHeightOfLastLine = true;
                mnHeightOfLastLine = pTextFrame->GetHeightOfLastLine();
            }
        }
    }
}

SwContentNotify::~SwContentNotify()
{
    // #i49383#
    if ( mbFrameDeleted )
    {
        return;
    }

    SwContentFrame *pCnt = GetCnt();
    if ( bSetCompletePaintOnInvalidate )
        pCnt->SetCompletePaint();

    SWRECTFN( pCnt )
    if ( pCnt->IsInTab() && ( POS_DIFF( pCnt->Frame(), maFrame ) ||
                             pCnt->Frame().SSize() != maFrame.SSize()))
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

    const bool bFirst = (maFrame.*fnRect->fnGetWidth)() == 0;

    if ( pCnt->IsNoTextFrame() )
    {
        //Active PlugIn's or OLE-Objects should know something of the change
        //thereby they move their window appropriate.
        SwViewShell *pSh  = pCnt->getRootFrame()->GetCurrShell();
        if ( pSh )
        {
            SwOLENode *pNd;
            if ( nullptr != (pNd = pCnt->GetNode()->GetOLENode()) &&
                 (pNd->GetOLEObj().IsOleRef() ||
                  pNd->IsOLESizeInvalid()) )
            {
                const bool bNoTextFramePrtAreaChanged =
                        ( maPrt.SSize().Width() != 0 &&
                          maPrt.SSize().Height() != 0 ) &&
                        maPrt.SSize() != pCnt->Prt().SSize();
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
                                                    &pFly->Prt(), &pFly->Frame(),
                                                    bNoTextFramePrtAreaChanged );
                    }
                }

                if ( pFESh && pNd->IsOLESizeInvalid() )
                {
                    pNd->SetOLESizeInvalid( false );
                    pFESh->CalcAndSetScale( xObj ); // create client
                }
            }
            //dito animated graphics
            if ( Frame().HasArea() && static_cast<SwNoTextFrame*>(pCnt)->HasAnimation() )
            {
                static_cast<SwNoTextFrame*>(pCnt)->StopAnimation();
                pSh->InvalidateWindows( Frame() );
            }
        }
    }

    if ( bFirst )
    {
        pCnt->SetRetouche();    //fix(13870)

        SwDoc *pDoc = pCnt->GetNode()->GetDoc();
        if ( !pDoc->GetSpzFrameFormats()->empty() &&
             pDoc->DoesContainAtPageObjWithContentAnchor() && !pDoc->getIDocumentState().IsNewDoc() )
        {
            // If certain import filters for foreign file format import
            // AT_PAGE anchored objects, the corresponding page number is
            // typically not known. In this case the content position is
            // stored at which the anchored object is found in the
            // imported document.
            // When this content is formatted it is the time at which
            // the page is known. Thus, this data can be corrected now.

            const SwPageFrame *pPage = nullptr;
            SwNodeIndex *pIdx  = nullptr;
            SwFrameFormats *pTable = pDoc->GetSpzFrameFormats();

            for ( size_t i = 0; i < pTable->size(); ++i )
            {
                SwFrameFormat *pFormat = (*pTable)[i];
                const SwFormatAnchor &rAnch = pFormat->GetAnchor();
                if ( FLY_AT_PAGE != rAnch.GetAnchorId() ||
                     rAnch.GetContentAnchor() == nullptr )
                {
                    continue;
                }

                if ( !pIdx )
                {
                    pIdx = new SwNodeIndex( *pCnt->GetNode() );
                }
                if ( rAnch.GetContentAnchor()->nNode == *pIdx )
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
            delete pIdx;
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
    if ( pCnt->IsTextFrame() && POS_DIFF( maFrame, pCnt->Frame() ) )
    {
        pCnt->InvalidateObjs( true );
    }

    // #i43255# - move code to invalidate at-character
    // anchored objects due to a change of its anchor character from
    // method <SwTextFrame::Format(..)>.
    if ( pCnt->IsTextFrame() )
    {
        SwTextFrame* pMasterFrame = pCnt->IsFollow()
                               ? static_cast<SwTextFrame*>(pCnt)->FindMaster()
                               : static_cast<SwTextFrame*>(pCnt);
        if ( pMasterFrame && !pMasterFrame->IsFlyLock() &&
             pMasterFrame->GetDrawObjs() )
        {
            SwSortedObjs* pObjs = pMasterFrame->GetDrawObjs();
            for ( size_t i = 0; i < pObjs->size(); ++i )
            {
                SwAnchoredObject* pAnchoredObj = (*pObjs)[i];
                if ( pAnchoredObj->GetFrameFormat().GetAnchor().GetAnchorId()
                        == FLY_AT_CHAR )
                {
                    pAnchoredObj->CheckCharRectAndTopOfLine( !pMasterFrame->IsEmpty() );
                }
            }
        }
    }
}

void AppendObjs( const SwFrameFormats *pTable, sal_uLong nIndex,
                        SwFrame *pFrame, SwPageFrame *pPage, SwDoc* doc )
{
    (void) pTable;
#if OSL_DEBUG_LEVEL > 0
    std::list<SwFrameFormat*> checkFormats;
    for ( size_t i = 0; i < pTable->size(); ++i )
    {
        SwFrameFormat *pFormat = (*pTable)[i];
        const SwFormatAnchor &rAnch = pFormat->GetAnchor();
        if ( rAnch.GetContentAnchor() &&
             (rAnch.GetContentAnchor()->nNode.GetIndex() == nIndex) )
        {
            checkFormats.push_back( pFormat );
        }
    }
#else
    (void)pTable;
#endif
    SwNode const& rNode(*doc->GetNodes()[nIndex]);
    std::vector<SwFrameFormat*> const*const pFlys(rNode.GetAnchoredFlys());
    for (size_t it = 0; pFlys && it != pFlys->size(); )
    {
        SwFrameFormat *const pFormat = (*pFlys)[it];
        const SwFormatAnchor &rAnch = pFormat->GetAnchor();
        if ( rAnch.GetContentAnchor() &&
             (rAnch.GetContentAnchor()->nNode.GetIndex() == nIndex) )
        {
#if OSL_DEBUG_LEVEL > 0
            std::list<SwFrameFormat*>::iterator checkPos = std::find( checkFormats.begin(), checkFormats.end(), pFormat );
            assert( checkPos != checkFormats.end());
            checkFormats.erase( checkPos );
#endif
            const bool bFlyAtFly = rAnch.GetAnchorId() == FLY_AT_FLY; // LAYER_IMPL
            //Is a frame or a SdrObject described?
            const bool bSdrObj = RES_DRAWFRMFMT == pFormat->Which();
            // OD 23.06.2003 #108784# - append also drawing objects anchored
            // as character.
            const bool bDrawObjInContent = bSdrObj &&
                                         (rAnch.GetAnchorId() == FLY_AS_CHAR);

            if( bFlyAtFly ||
                (rAnch.GetAnchorId() == FLY_AT_PARA) ||
                (rAnch.GetAnchorId() == FLY_AT_CHAR) ||
                bDrawObjInContent )
            {
                SdrObject* pSdrObj = nullptr;
                if ( bSdrObj && nullptr == (pSdrObj = pFormat->FindSdrObject()) )
                {
                    OSL_ENSURE( !bSdrObj, "DrawObject not found." );
                    ++it;
                    pFormat->GetDoc()->DelFrameFormat( pFormat );
                    continue;
                }
                if ( pSdrObj )
                {
                    if ( !pSdrObj->GetPage() )
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
                        SwDrawVirtObj* pDrawVirtObj = pNew->AddVirtObj();
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
        }
        ++it;
    }
#if OSL_DEBUG_LEVEL > 0
    assert( checkFormats.empty());
#endif
}

static bool lcl_ObjConnected( const SwFrameFormat *pFormat, const SwFrame* pSib )
{
    if ( RES_FLYFRMFMT == pFormat->Which() )
    {
        SwIterator<SwFlyFrame,SwFormat> aIter( *pFormat );
        const SwRootFrame* pRoot = pSib ? pSib->getRootFrame() : nullptr;
        const SwFlyFrame* pTmpFrame;
        for( pTmpFrame = aIter.First(); pTmpFrame; pTmpFrame = aIter.Next() )
        {
            if(! pRoot || pRoot == pTmpFrame->getRootFrame() )
                return true;
        }
    }
    else
    {
        SwDrawContact *pContact = SwIterator<SwDrawContact,SwFormat>(*pFormat).First();
        if ( pContact )
            return pContact->GetAnchorFrame() != nullptr;
    }
    return false;
}

/** helper method to determine, if a <SwFrameFormat>, which has an object connected,
    is located in header or footer.

    OD 23.06.2003 #108784#
*/
static bool lcl_InHeaderOrFooter( const SwFrameFormat& _rFormat )
{
    bool bRetVal = false;

    const SwFormatAnchor& rAnch = _rFormat.GetAnchor();

    if (rAnch.GetAnchorId() != FLY_AT_PAGE)
    {
        bRetVal = _rFormat.GetDoc()->IsInHeaderFooter( rAnch.GetContentAnchor()->nNode );
    }

    return bRetVal;
}

void AppendAllObjs( const SwFrameFormats *pTable, const SwFrame* pSib )
{
    //Connecting of all Objects, which are described in the SpzTable with the
    //layout.
    //If nothing happens anymore we can stop. Then formats can still remain,
    //because we neither use character bound frames nor objects which
    //are anchored to character bounds.

    // Optimization: This code used to make a copy of pTable and erase() handled items, but using
    // vector::erase() is a bad idea for performance (especially with large mailmerge documents
    // it results in extensive repeated copying). Use another vector for marking whether the item
    // has been handled and operate on the original data without altering them.
    std::vector< bool > handled( pTable->size(), false );
    size_t handledCount = 0;

    while ( handledCount < pTable->size())
    {
        bool changed = false;
        for ( int i = 0; i < int(pTable->size()); ++i )
        {
            if( handled[ i ] )
                continue;
            SwFrameFormat *pFormat = (*pTable)[ i ];
            const SwFormatAnchor &rAnch = pFormat->GetAnchor();
            bool bRemove = false;
            if ((rAnch.GetAnchorId() == FLY_AT_PAGE) ||
                (rAnch.GetAnchorId() == FLY_AS_CHAR))
            {
                //Page bounded are already anchored, character bounded
                //I don't want here.
                bRemove = true;
            }
            else
            {
                bRemove = ::lcl_ObjConnected( pFormat, pSib );
                if  ( !bRemove || ::lcl_InHeaderOrFooter( *pFormat ) )
                {
                    // OD 23.06.2003 #108784# - correction: for objects in header
                    // or footer create frames, in spite of the fact that an connected
                    // objects already exists.
                    //Call for Flys and DrawObjs only a MakeFrames if nor
                    //no dependent exists, otherwise, or if the MakeDrms creates no
                    //dependents, remove.
                    pFormat->MakeFrames();
                    bRemove = ::lcl_ObjConnected( pFormat, pSib );
                }
            }
            if ( bRemove )
            {
                handled[ i ] = true;
                ++handledCount;
                changed = true;
            }
        }
        if( !changed )
            break;
    }
}

/** local method to set 'working' position for newly inserted frames

    OD 12.08.2003 #i17969#
*/
static void lcl_SetPos( SwFrame&             _rNewFrame,
                 const SwLayoutFrame& _rLayFrame )
{
    SWRECTFN( (&_rLayFrame) )
    (_rNewFrame.Frame().*fnRect->fnSetPos)( (_rLayFrame.Frame().*fnRect->fnGetPos)() );
    // move position by one SwTwip in text flow direction in order to get
    // notifications for a new calculated position after its formatting.
    if ( bVert )
        _rNewFrame.Frame().Pos().X() -= 1;
    else
        _rNewFrame.Frame().Pos().Y() += 1;
}

void _InsertCnt( SwLayoutFrame *pLay, SwDoc *pDoc,
                             sal_uLong nIndex, bool bPages, sal_uLong nEndIndex,
                             SwFrame *pPrv )
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
    //of paragraphs, which must be moved expensively until it reaches a tolarable
    //reduced level.
    //We'd like to think that 20 Paragraphs fit on one page.
    //So that it does not become in extreme situations so violent we calculate depending
    //on the node something to it.
    //If in the DocStatistik a usable given pagenumber
    //(Will be cared for while writing), so it will be presumed that this will be
    //number of pages.
    const bool bStartPercent = bPages && !nEndIndex;

    SwPageFrame *pPage = pLay->FindPageFrame();
    const SwFrameFormats *pTable = pDoc->GetSpzFrameFormats();
    SwFrame       *pFrame = nullptr;
    SwActualSection *pActualSection = nullptr;
    SwLayHelper *pPageMaker;

    //If the layout will be created (bPages == true) we do head on the progress
    //Flys and DrawObjekte are not connected immediately, this
    //happens only at the end of the function.
    if ( bPages )
    {
        // Attention: the SwLayHelper class uses references to the content-,
        // page-, layout-frame etc. and may change them!
        bool   bBreakAfter   = false;
        pPageMaker = new SwLayHelper( pDoc, pFrame, pPrv, pPage, pLay,
                pActualSection, bBreakAfter, nIndex, 0 == nEndIndex );
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
        // If content will be inserted in a footnote, which in an column area,
        // the column area it is not allowed to be broken up.
        // Only if in the inner of the footnote lies an area, is this a candidate
        // for pActualSection.
        // The same applies for areas in tables, if inside the table will be
        // something inserted, it's only allowed to break up areas, which
        // lies in the inside also.
        if( ( !pLay->IsInFootnote() || pSct->IsInFootnote() ) &&
            ( !pLay->IsInTab() || pSct->IsInTab() ) )
        {
            pActualSection = new SwActualSection( nullptr, pSct, nullptr );
            OSL_ENSURE( !pLay->Lower() || !pLay->Lower()->IsColumnFrame(),
                "_InsertCnt: Wrong Call" );
        }
    }

    //If a section is "open", the pActualSection points to an SwActualSection.
    //If the page breaks, for "open" sections a follow will created.
    //For nested sections (which have, however, not a nested layout),
    //the SwActualSection class has a member, which points to an upper(section).
    //When the "inner" section finishes, the upper will used instead.

    while( true )
    {
        SwNode *pNd = pDoc->GetNodes()[nIndex];
        if ( pNd->IsContentNode() )
        {
            SwContentNode* pNode = static_cast<SwContentNode*>(pNd);
            pFrame = pNode->MakeFrame(pLay);
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
                     pViewShell->GetLayout()->IsAnyShellAccessible() )
                {
                    pViewShell->InvalidateAccessibleParaFlowRelation(
                        dynamic_cast<SwTextFrame*>(pFrame->FindNextCnt( true )),
                        dynamic_cast<SwTextFrame*>(pFrame->FindPrevCnt( true )) );
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

            if ( !pTable->empty() && bObjsDirect && !bDontCreateObjects )
                AppendObjs( pTable, nIndex, pFrame, pPage, pDoc );
        }
        else if ( pNd->IsTableNode() )
        {   //Should we have encountered a table?
            SwTableNode *pTableNode = static_cast<SwTableNode*>(pNd);

            // #108116# loading may produce table structures that GCLines
            // needs to clean up. To keep table formulas correct, change
            // all table formulas to internal (BOXPTR) representation.
            SwTableFormulaUpdate aMsgHint( &pTableNode->GetTable() );
            aMsgHint.m_eFlags = TBL_BOXPTR;
            pDoc->getIDocumentFieldsAccess().UpdateTableFields( &aMsgHint );
            pTableNode->GetTable().GCLines();

            pFrame = pTableNode->MakeFrame( pLay );

            if( pPageMaker )
                pPageMaker->CheckInsert( nIndex );

            pFrame->InsertBehind( pLay, pPrv );
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
                     pViewShell->GetLayout()->IsAnyShellAccessible() )
                {
                    pViewShell->InvalidateAccessibleParaFlowRelation(
                            dynamic_cast<SwTextFrame*>(pFrame->FindNextCnt( true )),
                            dynamic_cast<SwTextFrame*>(pFrame->FindPrevCnt( true )) );
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
            SwSectionNode *pNode = static_cast<SwSectionNode*>(pNd);
            if( pNode->GetSection().CalcHiddenFlag() )
                // is hidden, skip the area
                nIndex = pNode->EndOfSectionIndex();
            else
            {
                pFrame = pNode->MakeFrame( pLay );
                pActualSection = new SwActualSection( pActualSection,
                                                static_cast<SwSectionFrame*>(pFrame), pNode );
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
                            static_cast<SwTextFrame*>(pPrv)->Prepare( PREP_QUOVADIS, nullptr, false );
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
                         pViewShell->GetLayout()->IsAnyShellAccessible() )
                    {
                        pViewShell->InvalidateAccessibleParaFlowRelation(
                            dynamic_cast<SwTextFrame*>(pFrame->FindNextCnt( true )),
                            dynamic_cast<SwTextFrame*>(pFrame->FindPrevCnt( true )) );
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
            OSL_ENSURE( pActualSection, "Sectionende ohne Anfang?" );
            OSL_ENSURE( pActualSection->GetSectionNode() == pNd->StartOfSectionNode(),
                            "Sectionende mit falschen Start Node?" );

            //Close the section, where appropriate activate the surrounding
            //section again.
            SwActualSection *pTmp = pActualSection ? pActualSection->GetUpper() : nullptr;
            delete pActualSection;
            pLay = pLay->FindSctFrame();
            if ( nullptr != (pActualSection = pTmp) )
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
                SwSectionFrame* pFollow = pOuterSectionFrame->GetFollow();
                if ( pFollow )
                {
                    pOuterSectionFrame->SetFollow( nullptr );
                    pOuterSectionFrame->InvalidateSize();
                    static_cast<SwSectionFrame*>(pFrame)->SetFollow( pFollow );
                }

                // We don't want to leave empty parts back.
                if( ! pOuterSectionFrame->IsColLocked() &&
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
            if ( !pTable->empty() && bObjsDirect && !bDontCreateObjects )
            {
                SwFlyFrame* pFly = pLay->FindFlyFrame();
                if( pFly )
                    AppendObjs( pTable, nIndex, pFly, pPage, pDoc );
            }
        }
        else
            // Neither Content nor table nor section,
            // so we have to be ready.
            break;

        ++nIndex;
        // Do not consider the end node. The caller (section/MakeFrames()) has to ensure that the end
        // of this area is positioned before EndIndex!
        if ( nEndIndex && nIndex >= nEndIndex )
            break;
    }

    if ( pActualSection )
    {
        // Might happen that an empty (Follow-)Section is left over.
        if ( !(pLay = pActualSection->GetSectionFrame())->ContainsContent() )
        {
            pLay->RemoveFromLayout();
            SwFrame::DestroyFrame(pLay);
        }
        delete pActualSection;
    }

    if ( bPages ) // let the Flys connect to each other
    {
        if ( !bDontCreateObjects )
            AppendAllObjs( pTable, pLayout );
        bObjsDirect = true;
    }

    if( pPageMaker )
    {
        pPageMaker->CheckFlyCache( pPage );
        delete pPageMaker;
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
    sal_uLong nEndIdx = rEndIdx.GetIndex();
    SwNode* pNd = pDoc->GetNodes().FindPrvNxtFrameNode( aTmp,
                                            pDoc->GetNodes()[ nEndIdx-1 ]);
    if ( pNd )
    {
        bool bApres = aTmp < rSttIdx;
        SwNode2Layout aNode2Layout( *pNd, rSttIdx.GetIndex() );
        SwFrame* pFrame;
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
            bool bMoveNext = nEndIdx - rSttIdx.GetIndex() > 120;
            bool bAllowMove = !pFrame->IsInFly() && pFrame->IsMoveable() &&
                 (!pFrame->IsInTab() || pFrame->IsTabFrame() );
            if ( bMoveNext && bAllowMove )
            {
                SwFrame *pMove = pFrame;
                SwFrame *pPrev = pFrame->GetPrev();
                SwFlowFrame *pTmp = SwFlowFrame::CastFlowFrame( pMove );
                OSL_ENSURE( pTmp, "Missing FlowFrame" );

                if ( bApres )
                {
                    // The rest of this page should be empty. Thus, the following one has to move to
                    // the next page (it might also be located in the following column).
                    OSL_ENSURE( !pTmp->HasFollow(), "Follows forbidden" );
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
                    OSL_ENSURE( !pTmp->IsFollow(), "Follows really forbidden" );
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
                ::_InsertCnt( pUpper, pDoc, rSttIdx.GetIndex(),
                              pFrame->IsInDocBody(), nEndIdx, pPrev );
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

                ::_InsertCnt( pUpper, pDoc, rSttIdx.GetIndex(), false,
                              nEndIdx, pPrv );
                // OD 23.06.2003 #108784# - correction: append objects doesn't
                // depend on value of <bAllowMove>
                if( !bDontCreateObjects )
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
                    pFrame->FindFlyFrame()->_Invalidate();
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
        }
    }

    bObjsDirect = true;
}

SwBorderAttrs::SwBorderAttrs(const SwModify *pMod, const SwFrame *pConstructor)
    : SwCacheObj(pMod)
    , m_rAttrSet(pConstructor->IsContentFrame()
                    ? static_cast<const SwContentFrame*>(pConstructor)->GetNode()->GetSwAttrSet()
                    : static_cast<const SwLayoutFrame*>(pConstructor)->GetFormat()->GetAttrSet())
    , m_rUL(m_rAttrSet.GetULSpace())
    // #i96772#
    // LRSpaceItem is copied due to the possibility that it is adjusted - see below
    , m_rLR(m_rAttrSet.GetLRSpace())
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
{
    // #i96772#
    const SwTextFrame* pTextFrame = dynamic_cast<const SwTextFrame*>(pConstructor);
    if ( pTextFrame )
    {
        pTextFrame->GetTextNode()->ClearLRSpaceItemDueToListLevelIndents( m_rLR );
    }
    else if ( pConstructor->IsNoTextFrame() )
    {
        m_rLR = SvxLRSpaceItem ( RES_LR_SPACE );
    }

    // Caution: The USHORTs for the cached values are not initialized by intention!

    // everything needs to be calculated at least once:
    m_bTopLine = m_bBottomLine = m_bLeftLine = m_bRightLine =
    m_bTop     = m_bBottom     = m_bLine   = true;

    m_bCacheGetLine = m_bCachedGetTopLine = m_bCachedGetBottomLine = false;
    // OD 21.05.2003 #108789# - init cache status for values <m_bJoinedWithPrev>
    // and <m_bJoinedWithNext>, which aren't initialized by default.
    m_bCachedJoinedWithPrev = false;
    m_bCachedJoinedWithNext = false;

    m_bBorderDist = 0 != (pConstructor->GetType() & (FRM_CELL));
}

SwBorderAttrs::~SwBorderAttrs()
{
    const_cast<SwModify *>(static_cast<SwModify const *>(m_pOwner))->SetInCache( false );
}

/* All calc methods calculate a safety distance in addition to the values given by the attributes.
 * This safety distance is only added when working with borders and/or shadows to prevent that
 * e.g. borders are painted over.
 */

void SwBorderAttrs::_CalcTop()
{
    m_nTop = CalcTopLine() + m_rUL.GetUpper();
    m_bTop = false;
}

void SwBorderAttrs::_CalcBottom()
{
    m_nBottom = CalcBottomLine() + m_rUL.GetLower();
    m_bBottom = false;
}

long SwBorderAttrs::CalcRight( const SwFrame* pCaller ) const
{
    long nRight=0;

    if (!pCaller->IsTextFrame() || !static_cast<const SwTextFrame*>(pCaller)->GetTextNode()->GetDoc()->GetDocumentSettingManager().get(DocumentSettingId::INVERT_BORDER_SPACING)) {
    // OD 23.01.2003 #106895# - for cell frame in R2L text direction the left
    // and right border are painted on the right respectively left.
    if ( pCaller->IsCellFrame() && pCaller->IsRightToLeft() )
        nRight = CalcLeftLine();
    else
        nRight = CalcRightLine();

    }
    // for paragraphs, "left" is "before text" and "right" is "after text"
    if ( pCaller->IsTextFrame() && pCaller->IsRightToLeft() )
        nRight += m_rLR.GetLeft();
    else
        nRight += m_rLR.GetRight();

    // correction: retrieve left margin for numbering in R2L-layout
    if ( pCaller->IsTextFrame() && pCaller->IsRightToLeft() )
    {
        nRight += static_cast<const SwTextFrame*>(pCaller)->GetTextNode()->GetLeftMarginWithNum();
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
            if (dynamic_cast<const SwFlyFrame*>(pObject) !=  nullptr)
            {
                SwFlyFrame* pFly = static_cast<SwFlyFrame*>(pObject);
                if (pFly->Lower() && pFly->Lower()->IsTabFrame())
                    return true;
            }
        }
    }
    return false;
}

long SwBorderAttrs::CalcLeft( const SwFrame *pCaller ) const
{
    long nLeft=0;

    if (!pCaller->IsTextFrame() || !static_cast<const SwTextFrame*>(pCaller)->GetTextNode()->GetDoc()->GetDocumentSettingManager().get(DocumentSettingId::INVERT_BORDER_SPACING)) {
    // OD 23.01.2003 #106895# - for cell frame in R2L text direction the left
    // and right border are painted on the right respectively left.
    if ( pCaller->IsCellFrame() && pCaller->IsRightToLeft() )
        nLeft = CalcRightLine();
    else
        nLeft = CalcLeftLine();
    }

    // for paragraphs, "left" is "before text" and "right" is "after text"
    if ( pCaller->IsTextFrame() && pCaller->IsRightToLeft() )
        nLeft += m_rLR.GetRight();
    else
    {
        bool bIgnoreMargin = false;
        if (pCaller->IsTextFrame())
        {
            const SwTextFrame* pTextFrame = static_cast<const SwTextFrame*>(pCaller);
            if (pTextFrame->GetTextNode()->GetDoc()->GetDocumentSettingManager().get(DocumentSettingId::FLOATTABLE_NOMARGINS))
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
            nLeft += m_rLR.GetLeft();
    }

    // correction: do not retrieve left margin for numbering in R2L-layout
    if ( pCaller->IsTextFrame() && !pCaller->IsRightToLeft() )
    {
        nLeft += static_cast<const SwTextFrame*>(pCaller)->GetTextNode()->GetLeftMarginWithNum();
    }

    return nLeft;
}

/* Calculated values for borders and shadows.
 * It might be that a distance is wanted even without lines. This will be
 * considered here and not by the attribute (e.g. bBorderDist for cells).
 */

void SwBorderAttrs::_CalcTopLine()
{
    m_nTopLine = (m_bBorderDist && !m_rBox.GetTop())
                            ? m_rBox.GetDistance  (SvxBoxItemLine::TOP)
                            : m_rBox.CalcLineSpace(SvxBoxItemLine::TOP);
    m_nTopLine = m_nTopLine + m_rShadow.CalcShadowSpace(SvxShadowItemSide::TOP);
    m_bTopLine = false;
}

void SwBorderAttrs::_CalcBottomLine()
{
    m_nBottomLine = (m_bBorderDist && !m_rBox.GetBottom())
                            ? m_rBox.GetDistance  (SvxBoxItemLine::BOTTOM)
                            : m_rBox.CalcLineSpace(SvxBoxItemLine::BOTTOM);
    m_nBottomLine = m_nBottomLine + m_rShadow.CalcShadowSpace(SvxShadowItemSide::BOTTOM);
    m_bBottomLine = false;
}

void SwBorderAttrs::_CalcLeftLine()
{
    m_nLeftLine = (m_bBorderDist && !m_rBox.GetLeft())
                            ? m_rBox.GetDistance  (SvxBoxItemLine::LEFT)
                            : m_rBox.CalcLineSpace(SvxBoxItemLine::LEFT);
    m_nLeftLine = m_nLeftLine + m_rShadow.CalcShadowSpace(SvxShadowItemSide::LEFT);
    m_bLeftLine = false;
}

void SwBorderAttrs::_CalcRightLine()
{
    m_nRightLine = (m_bBorderDist && !m_rBox.GetRight())
                            ? m_rBox.GetDistance  (SvxBoxItemLine::RIGHT)
                            : m_rBox.CalcLineSpace(SvxBoxItemLine::RIGHT);
    m_nRightLine = m_nRightLine + m_rShadow.CalcShadowSpace(SvxShadowItemSide::RIGHT);
    m_bRightLine = false;
}

void SwBorderAttrs::_IsLine()
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

inline bool CmpLines( const editeng::SvxBorderLine *pL1, const editeng::SvxBorderLine *pL2 )
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

bool SwBorderAttrs::_JoinWithCmp( const SwFrame& _rCallerFrame,
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
void SwBorderAttrs::_CalcJoinedWithPrev( const SwFrame& _rFrame,
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
            m_bJoinedWithPrev = _JoinWithCmp( _rFrame, *(pPrevFrame) );
        }
    }

    // valid cache status, if demanded
    // OD 2004-02-26 #i25029# - Do not validate cache, if parameter <_pPrevFrame>
    // is set.
    m_bCachedJoinedWithPrev = m_bCacheGetLine && !_pPrevFrame;
}

// OD 21.05.2003 #108789# - method to determine, if borders are joined with
// next frame. Calculated value saved in cached value <m_bJoinedWithNext>
void SwBorderAttrs::_CalcJoinedWithNext( const SwFrame& _rFrame )
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
            m_bJoinedWithNext = _JoinWithCmp( _rFrame, *(pNextFrame) );
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
        const_cast<SwBorderAttrs*>(this)->_CalcJoinedWithPrev( _rFrame, _pPrevFrame );
    }

    return m_bJoinedWithPrev;
}

bool SwBorderAttrs::JoinedWithNext( const SwFrame& _rFrame ) const
{
    if ( !m_bCachedJoinedWithNext )
    {
        const_cast<SwBorderAttrs*>(this)->_CalcJoinedWithNext( _rFrame );
    }

    return m_bJoinedWithNext;
}

// OD 2004-02-26 #i25029# - added 2nd parameter <_pPrevFrame>, which is passed to
// method <JoinedWithPrev>
void SwBorderAttrs::_GetTopLine( const SwFrame& _rFrame,
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

void SwBorderAttrs::_GetBottomLine( const SwFrame& _rFrame )
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

SwBorderAttrAccess::SwBorderAttrAccess( SwCache &rCach, const SwFrame *pFrame ) :
    SwCacheAccess( rCach,
                   (pFrame->IsContentFrame() ?
                      const_cast<void*>(static_cast<void const *>(static_cast<const SwContentFrame*>(pFrame)->GetNode())) :
                      const_cast<void*>(static_cast<void const *>(static_cast<const SwLayoutFrame*>(pFrame)->GetFormat()))),
                   (pFrame->IsContentFrame() ?
                      static_cast<SwModify const *>(static_cast<const SwContentFrame*>(pFrame)->GetNode())->IsInCache() :
                      static_cast<SwModify const *>(static_cast<const SwLayoutFrame*>(pFrame)->GetFormat())->IsInCache()) ),
    pConstructor( pFrame )
{
}

SwCacheObj *SwBorderAttrAccess::NewObj()
{
    const_cast<SwModify *>(static_cast<SwModify const *>(pOwner))->SetInCache( true );
    return new SwBorderAttrs( static_cast<SwModify const *>(pOwner), pConstructor );
}

SwBorderAttrs *SwBorderAttrAccess::Get()
{
    return static_cast<SwBorderAttrs*>(SwCacheAccess::Get());
}

SwOrderIter::SwOrderIter( const SwPageFrame *pPg, bool bFlys ) :
    m_pPage( pPg ),
    m_pCurrent( nullptr ),
    m_bFlysOnly( bFlys )
{
}

const SdrObject *SwOrderIter::Top()
{
    m_pCurrent = nullptr;
    if ( m_pPage->GetSortedObjs() )
    {
        const SwSortedObjs *pObjs = m_pPage->GetSortedObjs();
        if ( pObjs->size() )
        {
            sal_uInt32 nTopOrd = 0;
            (*pObjs)[0]->GetDrawObj()->GetOrdNum();  // force updating
            for ( size_t i = 0; i < pObjs->size(); ++i )
            {
                const SdrObject* pObj = (*pObjs)[i]->GetDrawObj();
                if ( m_bFlysOnly && dynamic_cast<const SwVirtFlyDrawObj*>( pObj) ==  nullptr )
                    continue;
                sal_uInt32 nTmp = pObj->GetOrdNumDirect();
                if ( nTmp >= nTopOrd )
                {
                    nTopOrd = nTmp;
                    m_pCurrent = pObj;
                }
            }
        }
    }
    return m_pCurrent;
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
            for ( size_t i = 0; i < pObjs->size(); ++i )
            {
                const SdrObject* pObj = (*pObjs)[i]->GetDrawObj();
                if ( m_bFlysOnly && dynamic_cast<const SwVirtFlyDrawObj*>( pObj) ==  nullptr )
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
            for ( size_t i = 0; i < pObjs->size(); ++i )
            {
                const SdrObject* pObj = (*pObjs)[i]->GetDrawObj();
                if ( m_bFlysOnly && dynamic_cast<const SwVirtFlyDrawObj*>( pObj) ==  nullptr )
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

const SdrObject *SwOrderIter::Prev()
{
    const sal_uInt32 nCurOrd = m_pCurrent ? m_pCurrent->GetOrdNumDirect() : 0;
    m_pCurrent = nullptr;
    if ( m_pPage->GetSortedObjs() )
    {
        const SwSortedObjs *pObjs = m_pPage->GetSortedObjs();
        if ( pObjs->size() )
        {
            sal_uInt32 nOrd = 0;
            (*pObjs)[0]->GetDrawObj()->GetOrdNum();  // force updating
            for ( size_t i = 0; i < pObjs->size(); ++i )
            {
                const SdrObject* pObj = (*pObjs)[i]->GetDrawObj();
                if ( m_bFlysOnly && dynamic_cast<const SwVirtFlyDrawObj*>( pObj) ==  nullptr )
                    continue;
                sal_uInt32 nTmp = pObj->GetOrdNumDirect();
                if ( nTmp < nCurOrd && nTmp >= nOrd )
                {
                    nOrd = nTmp;
                    m_pCurrent = pObj;
                }
            }
        }
    }
    return m_pCurrent;
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
    for ( size_t i = 0; i < rObjs.size(); ++i )
    {
        SwAnchoredObject* pObj = rObjs[i];
        // #115759# - reset member, at which the anchored
        // object orients its vertical position
        pObj->ClearVertPosOrientFrame();
        // #i43913#
        pObj->ResetLayoutProcessBools();
        // #115759# - remove also lower objects of as-character
        // anchored Writer fly frames from page
        if ( dynamic_cast<const SwFlyFrame*>( pObj) !=  nullptr )
        {
            SwFlyFrame* pFlyFrame = static_cast<SwFlyFrame*>(pObj);

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
                pFlyFrame->GetPageFrame()->RemoveFlyFromPage( pFlyFrame );
            }
        }
        // #115759# - remove also drawing objects from page
        else if ( dynamic_cast<const SwAnchoredDrawObject*>( pObj) !=  nullptr )
        {
            if (pObj->GetFrameFormat().GetAnchor().GetAnchorId() != FLY_AS_CHAR)
            {
                pObj->GetPageFrame()->RemoveDrawObjFromPage(
                                *(static_cast<SwAnchoredDrawObject*>(pObj)) );
            }
        }
    }
}

SwFrame *SaveContent( SwLayoutFrame *pLay, SwFrame *pStart )
{
    if( pLay->IsSctFrame() && pLay->Lower() && pLay->Lower()->IsColumnFrame() )
        sw_RemoveFootnotes( static_cast<SwColumnFrame*>(pLay->Lower()), true, true );

    SwFrame *pSav;
    if ( nullptr == (pSav = pLay->ContainsAny()) )
        return nullptr;

    if( pSav->IsInFootnote() && !pLay->IsInFootnote() )
    {
        do
            pSav = pSav->FindNext();
        while( pSav && pSav->IsInFootnote() );
        if( !pSav || !pLay->IsAnLower( pSav ) )
            return nullptr;
    }

    // Tables should be saved as a whole, expection:
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
    for ( size_t i = 0; i < rObjs.size(); ++i )
    {
        SwAnchoredObject* pObj = rObjs[i];

        // #115759# - unlock position of anchored object
        // in order to get the object's position calculated.
        pObj->UnlockPosition();
        // #115759# - add also lower objects of as-character
        // anchored Writer fly frames from page
        if ( dynamic_cast<const SwFlyFrame*>( pObj) !=  nullptr )
        {
            SwFlyFrame* pFlyFrame = static_cast<SwFlyFrame*>(pObj);
            if ( dynamic_cast<const SwFlyFreeFrame*>( pObj) !=  nullptr )
            {
                _pPage->AppendFlyToPage( pFlyFrame );
            }
            pFlyFrame->_InvalidatePos();
            pFlyFrame->_InvalidateSize();
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
            if (pObj->GetFrameFormat().GetAnchor().GetAnchorId() != FLY_AS_CHAR)
            {
                pObj->InvalidateObjPos();
                _pPage->AppendDrawObjToPage(
                                *(static_cast<SwAnchoredDrawObject*>(pObj)) );
            }
        }
    }
}

void RestoreContent( SwFrame *pSav, SwLayoutFrame *pParent, SwFrame *pSibling, bool bGrow )
{
    OSL_ENSURE( pSav && pParent, "no Save or Parent provided for RestoreContent." );
    SWRECTFN( pParent )

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
        pSibling->_InvalidatePrt();
        pSibling->InvalidatePage( pPage );
        SwFlowFrame *pFlowFrame = dynamic_cast<SwFlowFrame*>(pSibling);
        if (pFlowFrame && pFlowFrame->GetFollow())
            pSibling->Prepare( PREP_CLEAR, nullptr, false );
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
        nGrowVal += (pSav->Frame().*fnRect->fnGetHeight)();
        pSav->_InvalidateAll();

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

    if ( bGrow )
        pParent->Grow( nGrowVal );
}

SwPageFrame * InsertNewPage( SwPageDesc &rDesc, SwFrame *pUpper,
                          bool bOdd, bool bFirst, bool bInsertEmpty, bool bFootnote,
                          SwFrame *pSibling )
{
    SwPageFrame *pRet;
    SwDoc *pDoc = static_cast<SwLayoutFrame*>(pUpper)->GetFormat()->GetDoc();
    if (bFirst)
    {
        if (rDesc.IsFirstShared())
        {
            // We need to fallback to left or right page format, decide it now.
            // FIXME: is this still needed?
            if (bOdd)
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
    SwFrameFormat *pFormat(bOdd ? rDesc.GetRightFormat(bFirst) : rDesc.GetLeftFormat(bFirst));
    // If there is no FrameFormat for this page, add an empty page
    if ( !pFormat )
    {
        pFormat = bOdd ? rDesc.GetLeftFormat() : rDesc.GetRightFormat();
        OSL_ENSURE( pFormat, "Descriptor without any format?!" );
        bInsertEmpty = !bInsertEmpty;
    }
    if( bInsertEmpty )
    {
        SwPageDesc *pTmpDesc = pSibling && pSibling->GetPrev() ?
                static_cast<SwPageFrame*>(pSibling->GetPrev())->GetPageDesc() : &rDesc;
        pRet = new SwPageFrame( pDoc->GetEmptyPageFormat(), pUpper, pTmpDesc );
        pRet->Paste( pUpper, pSibling );
        pRet->PreparePage( bFootnote );
    }
    pRet = new SwPageFrame( pFormat, pUpper, &rDesc );
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
    for ( size_t i = 0; i < pObjs->size(); ++i )
    {
        SwAnchoredObject* pObj = (*pObjs)[i];
        if ( dynamic_cast<const SwFlyFrame*>( pObj) !=  nullptr )
        {
            SwFlyFrame *pFly = static_cast<SwFlyFrame*>(pObj);
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
                if ( pObj->GetPageFrame() )
                    pObj->GetPageFrame()->RemoveDrawObjFromPage( *pObj );
                pPage->AppendDrawObjToPage( *pObj );
            }
        }

        const SwFlyFrame* pFly = pAnch->FindFlyFrame();
        if ( pFly &&
             pObj->GetDrawObj()->GetOrdNum() < pFly->GetVirtDrawObj()->GetOrdNum() &&
             pObj->GetDrawObj()->GetPage() )
        {
            //#i119945# set pFly's OrdNum to pObj's. So when pFly is removed by Undo, the original OrdNum will not be changed.
            pObj->DrawObj()->GetPage()->SetObjectOrdNum( pFly->GetVirtDrawObj()->GetOrdNumDirect(),
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
            pFly->NotifyBackground( pOld, rOld, PREP_FLY_LEAVE );
        }
        pFly->NotifyBackground( pFly->FindPageFrame(), aFrame, PREP_FLY_ARRIVE );
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
            pFly->NotifyBackground( pPageFrame, aFrame, PREP_FLY_ARRIVE );
        }

        if ( rOld.Left() != aFrame.Left() )
        {
            SwRect aTmp( rOld );
            aTmp.Union( aFrame );
            aTmp.Left(  std::min(aFrame.Left(), rOld.Left()) );
            aTmp.Right( std::max(aFrame.Left(), rOld.Left()) );
            pFly->NotifyBackground( pOld, aTmp, PREP_FLY_CHGD );
        }
        SwTwips nOld = rOld.Right();
        SwTwips nNew = aFrame.Right();
        if ( nOld != nNew )
        {
            SwRect aTmp( rOld );
            aTmp.Union( aFrame );
            aTmp.Left(  std::min(nNew, nOld) );
            aTmp.Right( std::max(nNew, nOld) );
            pFly->NotifyBackground( pOld, aTmp, PREP_FLY_CHGD );
        }
        if ( rOld.Top() != aFrame.Top() )
        {
            SwRect aTmp( rOld );
            aTmp.Union( aFrame );
            aTmp.Top(    std::min(aFrame.Top(), rOld.Top()) );
            aTmp.Bottom( std::max(aFrame.Top(), rOld.Top()) );
            pFly->NotifyBackground( pOld, aTmp, PREP_FLY_CHGD );
        }
        nOld = rOld.Bottom();
        nNew = aFrame.Bottom();
        if ( nOld != nNew )
        {
            SwRect aTmp( rOld );
            aTmp.Union( aFrame );
            aTmp.Top(    std::min(nNew, nOld) );
            aTmp.Bottom( std::max(nNew, nOld) );
            pFly->NotifyBackground( pOld, aTmp, PREP_FLY_CHGD );
        }
    }
    else if ( pOldPrt && *pOldPrt != pFly->Prt() &&
              pFly->GetFormat()->GetSurround().IsContour() )
    {
        // #i24097#
        pFly->NotifyBackground( pFly->FindPageFrame(), aFrame, PREP_FLY_ARRIVE );
    }
}

static void lcl_CheckFlowBack( SwFrame* pFrame, const SwRect &rRect )
{
    SwTwips nBottom = rRect.Bottom();
    while( pFrame )
    {
        if( pFrame->IsLayoutFrame() )
        {
            if( rRect.IsOver( pFrame->Frame() ) )
                lcl_CheckFlowBack( static_cast<SwLayoutFrame*>(pFrame)->Lower(), rRect );
        }
        else if( !pFrame->GetNext() && nBottom > pFrame->Frame().Bottom() )
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
    if ( pCnt->IsTextFrame() )
    {
        SwRect aCntPrt( pCnt->Prt() );
        aCntPrt.Pos() += pCnt->Frame().Pos();
        if ( eHint == PREP_FLY_ATTR_CHG )
        {
            // #i35640# - use given rectangle <rRect> instead
            // of current bound rectangle
            if ( aCntPrt.IsOver( rRect ) )
                pCnt->Prepare( PREP_FLY_ATTR_CHG );
        }
        // #i23129# - only invalidate, if the text frame
        // printing area overlaps with the given rectangle.
        else if ( aCntPrt.IsOver( rRect ) )
            pCnt->Prepare( eHint, static_cast<void*>(&aCntPrt._Intersection( rRect )) );
        if ( pCnt->GetDrawObjs() )
        {
            const SwSortedObjs &rObjs = *pCnt->GetDrawObjs();
            for ( size_t i = 0; i < rObjs.size(); ++i )
            {
                SwAnchoredObject* pObj = rObjs[i];
                if ( dynamic_cast<const SwFlyFrame*>( pObj) !=  nullptr )
                {
                    SwFlyFrame *pFly = static_cast<SwFlyFrame*>(pObj);
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
    }
}

void Notify_Background( const SdrObject* pObj,
                        SwPageFrame* pPage,
                        const SwRect& rRect,
                        const PrepareHint eHint,
                        const bool bInva )
{
    // If the frame was positioned correctly for the first time, do not inform the old area
    if ( eHint == PREP_FLY_LEAVE && rRect.Top() == FAR_AWAY )
         return;

    SwLayoutFrame* pArea;
    SwFlyFrame *pFlyFrame = nullptr;
    SwFrame* pAnchor;
    if( dynamic_cast<const SwVirtFlyDrawObj*>( pObj) !=  nullptr )
    {
        pFlyFrame = const_cast<SwVirtFlyDrawObj*>(static_cast<const SwVirtFlyDrawObj*>(pObj))->GetFlyFrame();
        pAnchor = pFlyFrame->AnchorFrame();
    }
    else
    {
        pFlyFrame = nullptr;
        pAnchor = const_cast<SwFrame*>(
                    GetUserCall(pObj)->GetAnchoredObj( pObj )->GetAnchorFrame() );
    }
    if( PREP_FLY_LEAVE != eHint && pAnchor->IsInFly() )
        pArea = pAnchor->FindFlyFrame();
    else
        pArea = pPage;
    SwContentFrame *pCnt = nullptr;
    if ( pArea )
    {
        if( PREP_FLY_ARRIVE != eHint )
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

    while ( pCnt && pArea && pArea->IsAnLower( pCnt ) )
    {
        ::lcl_NotifyContent( pObj, pCnt, rRect, eHint );
        if ( pCnt->IsInTab() )
        {
            SwLayoutFrame* pCell = pCnt->GetUpper();
            // #i40606# - use <GetLastBoundRect()>
            // instead of <GetCurrentBoundRect()>, because a recalculation
            // of the bounding rectangle isn't intended here.
            if ( pCell->IsCellFrame() &&
                 ( pCell->Frame().IsOver( pObj->GetLastBoundRect() ) ||
                   pCell->Frame().IsOver( rRect ) ) )
            {
                const SwFormatVertOrient &rOri = pCell->GetFormat()->GetVertOrient();
                if ( text::VertOrientation::NONE != rOri.GetVertOrient() )
                    pCell->InvalidatePrt();
            }
            SwTabFrame *pTab = pCnt->FindTabFrame();
            if ( pTab != pLastTab )
            {
                pLastTab = pTab;
                // #i40606# - use <GetLastBoundRect()>
                // instead of <GetCurrentBoundRect()>, because a recalculation
                // of the bounding rectangle isn't intended here.
                if ( pTab->Frame().IsOver( pObj->GetLastBoundRect() ) ||
                     pTab->Frame().IsOver( rRect ) )
                {
                    if ( !pFlyFrame || !pFlyFrame->IsLowerOf( pTab ) )
                        pTab->InvalidatePrt();
                }
            }
        }
        pCnt = pCnt->GetNextContentFrame();
    }
    // #128702# - make code robust
    if ( pPage && pPage->GetSortedObjs() )
    {
        pObj->GetOrdNum();
        const SwSortedObjs &rObjs = *pPage->GetSortedObjs();
        for ( size_t i = 0; i < rObjs.size(); ++i )
        {
            SwAnchoredObject* pAnchoredObj = rObjs[i];
            if ( dynamic_cast<const SwFlyFrame*>( pAnchoredObj) !=  nullptr )
            {
                if( pAnchoredObj->GetDrawObj() == pObj )
                    continue;
                SwFlyFrame *pFly = static_cast<SwFlyFrame*>(pAnchoredObj);
                if ( pFly->Frame().Top() == FAR_AWAY )
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
                        pFly->Frame().Bottom() >= rRect.Top() &&
                        pFly->Frame().Top() <= rRect.Bottom() &&
                        pFly->Frame().Right() >= rRect.Left() &&
                        pFly->Frame().Left() <= rRect.Right() )
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
                            (pFly->Frame().Bottom() >= rRect.Top() &&
                            pFly->Frame().Top() <= rRect.Bottom()) )
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
/// is a chained border or a footnote, the "virtual" Upper might be returne.
const SwFrame* GetVirtualUpper( const SwFrame* pFrame, const Point& rPos )
{
    if( pFrame->IsTextFrame() )
    {
        pFrame = pFrame->GetUpper();
        if( !pFrame->Frame().IsInside( rPos ) )
        {
            if( pFrame->IsFootnoteFrame() )
            {
                const SwFootnoteFrame* pTmp = static_cast<const SwFootnoteFrame*>(pFrame)->GetFollow();
                while( pTmp )
                {
                    if( pTmp->Frame().IsInside( rPos ) )
                        return pTmp;
                    pTmp = pTmp->GetFollow();
                }
            }
            else
            {
                SwFlyFrame* pTmp = const_cast<SwFlyFrame*>(pFrame->FindFlyFrame());
                while( pTmp )
                {
                    if( pTmp->Frame().IsInside( rPos ) )
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
        aPos = pFly->Frame().Pos();
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
            aPos = pFrame->Frame().Pos();
            pFrame = GetVirtualUpper( static_cast<const SwFlyFrame*>(pFrame)->GetAnchorFrame(), aPos );
        }
        else
            pFrame = pFrame->GetUpper();
    } while ( pFrame );
    return false;
}

/// provides the area of a frame in that no Fly from another area can overlap
const SwFrame *FindKontext( const SwFrame *pFrame, sal_uInt16 nAdditionalContextType )
{
    const sal_uInt16 nTyp = FRM_ROOT | FRM_HEADER   | FRM_FOOTER | FRM_FTNCONT  |
                        FRM_FTN  | FRM_FLY      |
                        FRM_TAB  | FRM_ROW      | FRM_CELL |
                        nAdditionalContextType;
    do
    {   if ( pFrame->GetType() & nTyp )
            break;
        pFrame = pFrame->GetUpper();
    } while( pFrame );
    return pFrame;
}

bool IsFrameInSameKontext( const SwFrame *pInnerFrame, const SwFrame *pFrame )
{
    const SwFrame *pKontext = FindKontext( pInnerFrame, 0 );

    const sal_uInt16 nTyp = FRM_ROOT | FRM_HEADER   | FRM_FOOTER | FRM_FTNCONT  |
                        FRM_FTN  | FRM_FLY      |
                        FRM_TAB  | FRM_ROW      | FRM_CELL;
    do
    {   if ( pFrame->GetType() & nTyp )
        {
            if( pFrame == pKontext )
                return true;
            if( pFrame->IsCellFrame() )
                return false;
        }
        if( pFrame->IsFlyFrame() )
        {
            Point aPos( pFrame->Frame().Pos() );
            pFrame = GetVirtualUpper( static_cast<const SwFlyFrame*>(pFrame)->GetAnchorFrame(), aPos );
        }
        else
            pFrame = pFrame->GetUpper();
    } while( pFrame );

    return false;
}

static SwTwips lcl_CalcCellRstHeight( SwLayoutFrame *pCell )
{
    if ( pCell->Lower()->IsContentFrame() || pCell->Lower()->IsSctFrame() )
    {
        SwFrame *pLow = pCell->Lower();
        long nHeight = 0, nFlyAdd = 0;
        do
        {
            long nLow = pLow->Frame().Height();
            if( pLow->IsTextFrame() && static_cast<SwTextFrame*>(pLow)->IsUndersized() )
                nLow += static_cast<SwTextFrame*>(pLow)->GetParHeight()-pLow->Prt().Height();
            else if( pLow->IsSctFrame() && static_cast<SwSectionFrame*>(pLow)->IsUndersized() )
                nLow += static_cast<SwSectionFrame*>(pLow)->Undersize();
            nFlyAdd = std::max( 0L, nFlyAdd - nLow );
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

        return pCell->Frame().Height() - nHeight;
    }
    else
    {
        long nRstHeight = 0;
        SwFrame *pLow = pCell->Lower();
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
    SwTwips nRstHeight = LONG_MAX;
    SwFrame *pLow = pRow->Lower();
    while (pLow && pLow->IsLayoutFrame())
    {
        nRstHeight = std::min(nRstHeight, ::lcl_CalcCellRstHeight(static_cast<SwLayoutFrame*>(pLow)));
        pLow = pLow->GetNext();
    }
    return nRstHeight;
}

const SwFrame* FindPage( const SwRect &rRect, const SwFrame *pPage )
{
    if ( !rRect.IsOver( pPage->Frame() ) )
    {
        const SwRootFrame* pRootFrame = static_cast<const SwRootFrame*>(pPage->GetUpper());
        const SwFrame* pTmpPage = pRootFrame ? pRootFrame->GetPageAtPos( rRect.TopLeft(), &rRect.SSize(), true ) : nullptr;
        if ( pTmpPage )
            pPage = pTmpPage;
    }

    return pPage;
}

class SwFrameHolder : private SfxListener
{
    SwFrame* pFrame;
    bool bSet;
    virtual void Notify(  SfxBroadcaster& rBC, const SfxHint& rHint ) override;
public:
    SwFrameHolder() : pFrame(nullptr), bSet(false) {}
    void SetFrame( SwFrame* pHold );
    SwFrame* GetFrame() { return pFrame; }
    void Reset();
    bool IsSet() { return bSet; }
};

void SwFrameHolder::SetFrame( SwFrame* pHold )
{
    bSet = true;
    pFrame = pHold;
    StartListening(*pHold);
}

void SwFrameHolder::Reset()
{
    if (pFrame)
        EndListening(*pFrame);
    bSet = false;
    pFrame = nullptr;
}

void SwFrameHolder::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    const SfxSimpleHint* pSimpleHint = dynamic_cast<const SfxSimpleHint*>(&rHint);
    if ( pSimpleHint && pSimpleHint->GetId() == SFX_HINT_DYING && &rBC == pFrame )
    {
        pFrame = nullptr;
    }
}

SwFrame* GetFrameOfModify( const SwRootFrame* pLayout, SwModify const& rMod, sal_uInt16 const nFrameType,
        const Point* pPoint, const SwPosition *pPos, const bool bCalcFrame )
{
    SwFrame *pMinFrame = nullptr, *pTmpFrame;
    SwFrameHolder aHolder;
    SwRect aCalcRect;
    bool bClientIterChanged = false;

    SwIterator<SwFrame,SwModify> aIter( rMod );
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
                if( pPoint )
                {
                    // watch for Frame being deleted
                    if ( pMinFrame )
                        aHolder.SetFrame( pMinFrame );
                    else
                        aHolder.Reset();

                    if( bCalcFrame )
                    {
                        // - format parent Writer
                        // fly frame, if it isn't been formatted yet.
                        // Note: The Writer fly frame could be the frame itself.
                        SwFlyFrame* pFlyFrame( pTmpFrame->FindFlyFrame() );
                        if ( pFlyFrame &&
                             pFlyFrame->Frame().Pos().X() == FAR_AWAY &&
                             pFlyFrame->Frame().Pos().Y() == FAR_AWAY )
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
                    if( !bCalcFrame && pTmpFrame->GetType() & FRM_FLY &&
                        static_cast<SwFlyFrame*>(pTmpFrame)->GetAnchorFrame() &&
                        FAR_AWAY == pTmpFrame->Frame().Pos().getX() &&
                        FAR_AWAY == pTmpFrame->Frame().Pos().getY() )
                        aCalcRect = static_cast<SwFlyFrame*>(pTmpFrame)->GetAnchorFrame()->Frame();
                    else
                        aCalcRect = pTmpFrame->Frame();

                    if ( aCalcRect.IsInside( *pPoint ) )
                    {
                        pMinFrame = pTmpFrame;
                        break;
                    }

                    // Point not in rectangle. Compare distances:
                    const Point aCalcRectCenter = aCalcRect.Center();
                    const Point aDiff = aCalcRectCenter - *pPoint;
                    const sal_uInt64 nCurrentDist = sal_Int64(aDiff.getX()) * sal_Int64(aDiff.getX()) + sal_Int64(aDiff.getY()) * sal_Int64(aDiff.getY()); // opt: no sqrt
                    if ( !pMinFrame || nCurrentDist < nMinDist )
                    {
                        pMinFrame = pTmpFrame;
                        nMinDist = nCurrentDist;
                    }
                }
                else
                {
                    // if no pPoint is provided, take the first one
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
           ((sal_Int16)SW_MOD()->GetRedlineMarkPos() != text::HoriOrientation::NONE &&
            !pDoc->getIDocumentRedlineAccess().GetRedlineTable().empty());
}

// OD 22.09.2003 #110978#
const SwRect SwPageFrame::PrtWithoutHeaderAndFooter() const
{
    SwRect aPrtWithoutHeaderFooter( Prt() );
    aPrtWithoutHeaderFooter.Pos() += Frame().Pos();

    const SwFrame* pLowerFrame = Lower();
    while ( pLowerFrame )
    {
        // Note: independent on text direction page header and page footer are
        //       always at top respectively at bottom of the page frame.
        if ( pLowerFrame->IsHeaderFrame() )
        {
            aPrtWithoutHeaderFooter.Top( aPrtWithoutHeaderFooter.Top() +
                                         pLowerFrame->Frame().Height() );
        }
        if ( pLowerFrame->IsFooterFrame() )
        {
            aPrtWithoutHeaderFooter.Bottom( aPrtWithoutHeaderFooter.Bottom() -
                                            pLowerFrame->Frame().Height() );
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
                            bool& obIsLineSpacingProportional )
{
    if ( !rFrame.IsFlowFrame() )
    {
        onLowerSpacing = 0;
        onLineSpacing = 0;
    }
    else
    {
        const SvxULSpaceItem& rULSpace = rFrame.GetAttrSet()->GetULSpace();
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
            pContent = pTmpTab->FindLastContent();
            if ( pContent )

                pContent = pContent->FindNextCnt();

        }
        else
            break;
    }
    return pContent;
}

/// Can be used to check if a frame has been deleted
bool SwDeletionChecker::HasBeenDeleted()
{
    if ( !mpFrame || !mpRegIn )
        return false;

    SwIterator<SwFrame,SwModify> aIter(*mpRegIn);
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
