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

#include <hintids.hxx>
#include <svx/strings.hrc>
#include <svx/sdrobjectfilter.hxx>
#include <svx/svddrgmt.hxx>
#include <svx/svditer.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdouno.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdocirc.hxx>
#include <svx/svdopath.hxx>
#include <svx/sxciaitm.hxx>
#include <svx/svdocapt.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/xlnstwit.hxx>
#include <svx/xlnedwit.hxx>
#include <svx/xlnedit.hxx>
#include <svx/xlnstit.hxx>
#include <svx/svdomeas.hxx>
#include <svx/sdtagitm.hxx>
#include <svx/sdtacitm.hxx>
#include <svx/sdtaaitm.hxx>
#include <editeng/opaqitem.hxx>
#include <editeng/protitem.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdpagv.hxx>
#include <svx/dialmgr.hxx>
#include <tools/globname.hxx>
#include <sot/exchange.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <IDocumentSettingAccess.hxx>
#include <DocumentSettingManager.hxx>
#include <IDocumentState.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <drawdoc.hxx>
#include <textboxhelper.hxx>
#include <frmfmt.hxx>
#include <frmatr.hxx>
#include <frmtool.hxx>
#include <fmtfsize.hxx>
#include <fmtanchr.hxx>
#include <fmtornt.hxx>
#include <fmtsrnd.hxx>
#include <fmtcntnt.hxx>
#include <fmtflcnt.hxx>
#include <fmtcnct.hxx>
#include <swmodule.hxx>
#include <fesh.hxx>
#include <rootfrm.hxx>
#include <pagefrm.hxx>
#include <sectfrm.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <dview.hxx>
#include <dflyobj.hxx>
#include <dcontact.hxx>
#include <viewimp.hxx>
#include <flyfrm.hxx>
#include <pam.hxx>
#include <ndole.hxx>
#include <ndgrf.hxx>
#include <ndtxt.hxx>
#include <viewopt.hxx>
#include <swundo.hxx>
#include <notxtfrm.hxx>
#include <txtfrm.hxx>
#include <mdiexp.hxx>
#include <sortedobjs.hxx>
#include <HandleAnchorNodeChg.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <comphelper/lok.hxx>
#include <sfx2/lokhelper.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <calbck.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <svx/svxids.hrc>
#include <osl/diagnose.h>

#include <com/sun/star/embed/EmbedMisc.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>

#include <svx/srchdlg.hxx>

#define SCROLLVAL 75

using namespace com::sun::star;

/**
 * set line starts and ends for the object to be created
 */

namespace {

::basegfx::B2DPolyPolygon getPolygon(TranslateId pResId, const SdrModel& rModel)
{
    ::basegfx::B2DPolyPolygon aRetval;
    XLineEndListRef pLineEndList(rModel.GetLineEndList());

    if( pLineEndList.is() )
    {
        OUString aArrowName( SvxResId(pResId) );
        tools::Long nCount = pLineEndList->Count();
        tools::Long nIndex;
        for( nIndex = 0; nIndex < nCount; nIndex++ )
        {
            const XLineEndEntry* pEntry = pLineEndList->GetLineEnd(nIndex);
            if( pEntry->GetName() == aArrowName )
            {
                aRetval = pEntry->GetLineEnd();
                break;
            }
        }
    }

    return aRetval;
}

}

SwFlyFrame *GetFlyFromMarked( const SdrMarkList *pLst, SwViewShell *pSh )
{
    if ( !pLst )
        pLst = pSh->HasDrawView() ? &pSh->Imp()->GetDrawView()->GetMarkedObjectList():nullptr;

    if ( pLst && pLst->GetMarkCount() == 1 )
    {
        SdrObject *pO = pLst->GetMark( 0 )->GetMarkedSdrObj();
        if (SwVirtFlyDrawObj* pVirtO = dynamic_cast<SwVirtFlyDrawObj*>(pO))
            return pVirtO->GetFlyFrame();
    }
    return nullptr;
}

static void lcl_GrabCursor( SwFEShell* pSh, SwFlyFrame* pOldSelFly)
{
    const SwFrameFormat *pFlyFormat = pSh->SelFlyGrabCursor();
    if( pFlyFormat && !pSh->ActionPend() &&
                        (!pOldSelFly || pOldSelFly->GetFormat() != pFlyFormat) )
    {
        // now call set macro if applicable
        pSh->GetFlyMacroLnk().Call( static_cast<const SwFlyFrameFormat*>(pFlyFormat) );
        // if a dialog was started inside a macro, then
        // MouseButtonUp arrives at macro and not to us. Therefore
        // flag is always set here and will never be switched to
        // respective Shell !!!!!!!

        g_bNoInterrupt = false;
    }
    else if( !pFlyFormat || RES_DRAWFRMFMT == pFlyFormat->Which() )
    {
        // --> assure consistent cursor
        pSh->KillPams();
        pSh->ClearMark();
        pSh->SetCursor( pSh->Imp()->GetDrawView()->GetAllMarkedRect().TopLeft(), true);
    }
}

bool SwFEShell::SelectObj( const Point& rPt, sal_uInt8 nFlag, SdrObject *pObj )
{
    SwDrawView *pDView = Imp()->GetDrawView();
    if(!pDView)
        return false;
    CurrShell aCurr( this );
    StartAction();    // action is necessary to assure only one AttrChgdNotify
                      // (e.g. due to Unmark->MarkListHasChgd) arrives

    const SdrMarkList &rMrkList = pDView->GetMarkedObjectList();
    const bool bHadSelection = rMrkList.GetMarkCount();
    const bool bAddSelect = 0 != (SW_ADD_SELECT & nFlag);
    const bool bEnterGroup = 0 != (SW_ENTER_GROUP & nFlag);
    SwFlyFrame* pOldSelFly = nullptr;
    const Point aOldPos( pDView->GetAllMarkedRect().TopLeft() );

    if( bHadSelection )
    {
        // call Unmark when !bAddSelect or if fly was selected
        bool bUnmark = !bAddSelect;

        if ( rMrkList.GetMarkCount() == 1 )
        {
            // if fly was selected, deselect it first
            pOldSelFly = ::GetFlyFromMarked( &rMrkList, this );
            if ( pOldSelFly )
            {
                const sal_uInt16 nType = GetCntType();
                if( nType != CNT_TXT || (SW_LEAVE_FRAME & nFlag) ||
                    ( pOldSelFly->GetFormat()->GetProtect().IsContentProtected()
                     && !IsReadOnlyAvailable() ))
                {
                    // If a fly is deselected, which contains graphic, OLE or
                    // otherwise, the cursor should be removed from it.
                    // Similar if a fly with protected content is deselected.
                    // For simplicity we put the cursor next to the upper-left
                    // corner.
                    Point aPt( pOldSelFly->getFrameArea().Pos() );
                    aPt.setX(aPt.getX() - 1);
                    bool bUnLockView = !IsViewLocked();
                    LockView( true );
                    SetCursor( aPt, true );
                    if( bUnLockView )
                        LockView( false );
                }
                if ( nType & CNT_GRF &&
                     static_cast<SwNoTextFrame*>(pOldSelFly->Lower())->HasAnimation() )
                {
                    GetWin()->Invalidate( pOldSelFly->getFrameArea().SVRect() );
                }

                // Cancel crop mode
                if ( SdrDragMode::Crop == GetDragMode() )
                    SetDragMode( SdrDragMode::Move );

                bUnmark = true;
            }
        }
        if ( bUnmark )
        {
            pDView->UnmarkAll();
            if (pOldSelFly)
                pOldSelFly->SelectionHasChanged(this);
        }
    }
    else
    {
        KillPams();
        ClearMark();
    }

    if ( pObj )
    {
        OSL_ENSURE( !bEnterGroup, "SW_ENTER_GROUP is not supported" );
        pDView->MarkObj( pObj, Imp()->GetPageView() );
    }
    else
    {
        // tolerance limit of Drawing-SS
        const auto nHdlSizePixel = Imp()->GetDrawView()->GetMarkHdlSizePixel();
        const short nMinMove = static_cast<short>(GetOut()->PixelToLogic(Size(nHdlSizePixel/2, 0)).Width());
        pDView->MarkObj( rPt, nMinMove, bAddSelect, bEnterGroup );
    }

    const bool bRet = 0 != rMrkList.GetMarkCount();

    if ( rMrkList.GetMarkCount() > 1 )
    {
        // It sucks if Drawing objects were selected and now
        // additionally a fly is selected.
        for ( size_t i = 0; i < rMrkList.GetMarkCount(); ++i )
        {
            SdrObject *pTmpObj = rMrkList.GetMark( i )->GetMarkedSdrObj();
            bool bForget = dynamic_cast<const SwVirtFlyDrawObj*>( pTmpObj) !=  nullptr;
            if( bForget )
            {
                pDView->UnmarkAll();
                pDView->MarkObj( pTmpObj, Imp()->GetPageView(), bAddSelect, bEnterGroup );
                break;
            }
        }
    }

    if ( rMrkList.GetMarkCount() == 1 )
    {
        SwFlyFrame *pSelFly = ::GetFlyFromMarked( &rMrkList, this );
        if (pSelFly)
            pSelFly->SelectionHasChanged(this);
    }

    if (!(nFlag & SW_ALLOW_TEXTBOX))
    {
        // If the fly frame is a textbox of a shape, then select the shape instead.
        for (size_t i = 0; i < rMrkList.GetMarkCount(); ++i)
        {
            SdrObject* pObject = rMrkList.GetMark(i)->GetMarkedSdrObj();
            SwContact* pContact = GetUserCall(pObject);
            if (!pContact)
            {
                continue;
            }

            SwFrameFormat* pFormat = pContact->GetFormat();
            if (SwFrameFormat* pShapeFormat = SwTextBoxHelper::getOtherTextBoxFormat(pFormat, RES_FLYFRMFMT))
            {
                SdrObject* pShape = pShapeFormat->FindSdrObject();
                pDView->UnmarkAll();
                pDView->MarkObj(pShape, Imp()->GetPageView(), bAddSelect, bEnterGroup);
                break;
            }
        }
    }

    if ( bRet )
    {
        ::lcl_GrabCursor(this, pOldSelFly);
        if ( GetCntType() & CNT_GRF )
        {
            const SwFlyFrame *pTmp = GetFlyFromMarked( &rMrkList, this );
            OSL_ENSURE( pTmp, "Graphic without Fly" );
            if ( static_cast<const SwNoTextFrame*>(pTmp->Lower())->HasAnimation() )
                static_cast<const SwNoTextFrame*>(pTmp->Lower())->StopAnimation( GetOut() );
        }
    }
    else if ( !pOldSelFly && bHadSelection )
        SetCursor( aOldPos, true);

    if( bRet || !bHadSelection )
        CallChgLnk();

    // update status line
    ::FrameNotify( this, bRet ? FLY_DRAG_START : FLY_DRAG_END );

    EndAction();
    return bRet;
}

/*
 *  Description: MoveAnchor( nDir ) looked for an another Anchor for
 *  the selected drawing object (or fly frame) in the given direction.
 *  An object "as character" doesn't moves anyway.
 *  A page bounded object could move to the previous/next page with up/down,
 *  an object bounded "at paragraph" moves to the previous/next paragraph, too.
 *  An object bounded "at character" moves to the previous/next paragraph
 *  with up/down and to the previous/next character with left/right.
 *  If the anchor for at paragraph/character bounded objects has vertical or
 *  right_to_left text direction, the directions for up/down/left/right will
 *  interpreted accordingly.
 *  An object bounded "at fly" takes the center of the actual anchor and looks
 *  for the nearest fly frame in the given direction.
 */

static bool LessX( Point const & aPt1, Point const & aPt2, bool bOld )
{
    return aPt1.getX() < aPt2.getX()
            || ( aPt1.getX() == aPt2.getX()
                && ( aPt1.getY() < aPt2.getY()
                    || ( aPt1.getY() == aPt2.getY() && bOld ) ) );
}
static bool LessY( Point const & aPt1, Point const & aPt2, bool bOld )
{
    return aPt1.getY() < aPt2.getY()
            || ( aPt1.getY() == aPt2.getY()
                && ( aPt1.getX() < aPt2.getX()
                    || ( aPt1.getX() == aPt2.getX() && bOld ) ) );
}

bool SwFEShell::MoveAnchor( SwMove nDir )
{
    if (!Imp()->GetDrawView())
        return false;
    const SdrMarkList& pMrkList = Imp()->GetDrawView()->GetMarkedObjectList();
    if (1 != pMrkList.GetMarkCount())
        return false;
    SwFrame* pOld;
    SwFlyFrame* pFly = nullptr;
    SdrObject *pObj = pMrkList.GetMark( 0 )->GetMarkedSdrObj();
    if (SwVirtFlyDrawObj* pVirtO = dynamic_cast<SwVirtFlyDrawObj*>(pObj))
    {
        pFly = pVirtO->GetFlyFrame();
        pOld = pFly->AnchorFrame();
    }
    else
        pOld = static_cast<SwDrawContact*>(GetUserCall(pObj))->GetAnchorFrame( pObj );
    bool bRet = false;
    if( pOld )
    {
        SwFrame* pNew = pOld;
        // #i28701#
        SwAnchoredObject* pAnchoredObj = ::GetUserCall( pObj )->GetAnchoredObj( pObj );
        SwFrameFormat& rFormat = pAnchoredObj->GetFrameFormat();
        SwFormatAnchor aAnch( rFormat.GetAnchor() );
        RndStdIds nAnchorId = aAnch.GetAnchorId();
        if ( RndStdIds::FLY_AS_CHAR == nAnchorId )
            return false;
        if( pOld->IsVertical() )
        {
            if( pOld->IsTextFrame() )
            {
                switch( nDir ) {
                    case SwMove::UP: nDir = SwMove::LEFT; break;
                    case SwMove::DOWN: nDir = SwMove::RIGHT; break;
                    case SwMove::LEFT: nDir = SwMove::DOWN; break;
                    case SwMove::RIGHT: nDir = SwMove::UP; break;
                }
                if( pOld->IsRightToLeft() )
                {
                    if( nDir == SwMove::LEFT )
                        nDir = SwMove::RIGHT;
                    else if( nDir == SwMove::RIGHT )
                        nDir = SwMove::LEFT;
                }
            }
        }
        switch ( nAnchorId ) {
            case RndStdIds::FLY_AT_PAGE:
            {
                OSL_ENSURE( pOld->IsPageFrame(), "Wrong anchor, page expected." );
                if( SwMove::UP == nDir )
                    pNew = pOld->GetPrev();
                else if( SwMove::DOWN == nDir )
                    pNew = pOld->GetNext();
                if( pNew && pNew != pOld )
                {
                    aAnch.SetPageNum( static_cast<SwPageFrame*>(pNew)->GetPhyPageNum() );
                    bRet = true;
                }
                break;
            }
            case RndStdIds::FLY_AT_CHAR:
            {
                OSL_ENSURE(pOld->IsTextFrame(), "Wrong anchor, text frame expected.");
                if( SwMove::LEFT == nDir || SwMove::RIGHT == nDir )
                {
                    SwPosition pos = *aAnch.GetContentAnchor();
                    SwTextFrame *const pOldFrame(static_cast<SwTextFrame*>(pOld));
                    TextFrameIndex const nAct(pOldFrame->MapModelToViewPos(pos));
                    if( SwMove::LEFT == nDir )
                    {
                        bRet = true;
                        if( nAct )
                        {
                            pos = pOldFrame->MapViewToModelPos(nAct - TextFrameIndex(1));
                        }
                        else
                            nDir = SwMove::UP;
                    }
                    else
                    {
                        TextFrameIndex const nMax(pOldFrame->GetText().getLength());
                        if( nAct < nMax )
                        {
                            bRet = true;
                            pos = pOldFrame->MapViewToModelPos(nAct + TextFrameIndex(1));
                        }
                        else
                            nDir = SwMove::DOWN;
                    }
                    if( pos != *aAnch.GetContentAnchor())
                        aAnch.SetAnchor( &pos );
                }
                [[fallthrough]];
            }
            case RndStdIds::FLY_AT_PARA:
            {
                OSL_ENSURE(pOld->IsTextFrame(), "Wrong anchor, text frame expected.");
                if( SwMove::UP == nDir )
                    pNew = pOld->FindPrev();
                else if( SwMove::DOWN == nDir )
                    pNew = pOld->FindNext();
                if( pNew && pNew != pOld && pNew->IsContentFrame() )
                {
                    SwTextFrame *const pNewFrame(static_cast<SwTextFrame*>(pNew));
                    SwPosition const pos = pNewFrame->MapViewToModelPos(
                        TextFrameIndex(
                            (bRet && pNewFrame->GetText().getLength() != 0)
                                ? pNewFrame->GetText().getLength() - 1
                                : 0));
                    aAnch.SetAnchor( &pos );
                    bRet = true;
                }
                else if( SwMove::UP == nDir || SwMove::DOWN == nDir )
                    bRet = false;
                break;
            }
            case RndStdIds::FLY_AT_FLY:
            {
                OSL_ENSURE( pOld->IsFlyFrame(), "Wrong anchor, fly frame expected.");
                SwPageFrame* pPage = pOld->FindPageFrame();
                OSL_ENSURE( pPage, "Where's my page?" );
                SwFlyFrame* pNewFly = nullptr;
                if( pPage->GetSortedObjs() )
                {
                    bool bOld = false;
                    Point aCenter( pOld->getFrameArea().Left() + pOld->getFrameArea().Width()/2,
                                   pOld->getFrameArea().Top() + pOld->getFrameArea().Height()/2 );
                    Point aBest;
                    for(SwAnchoredObject* pAnchObj : *pPage->GetSortedObjs())
                    {
                        if( auto pTmp = pAnchObj->DynCastFlyFrame() )
                        {
                            if( pTmp == pOld )
                                bOld = true;
                            else
                            {
                                const SwFlyFrame* pCheck = pFly ? pTmp : nullptr;
                                while( pCheck )
                                {
                                    if( pCheck == pFly )
                                        break;
                                    const SwFrame *pNxt = pCheck->GetAnchorFrame();
                                    pCheck = pNxt ? pNxt->FindFlyFrame() : nullptr;
                                }
                                if( pCheck || pTmp->IsProtected() )
                                    continue;
                                Point aNew( pTmp->getFrameArea().Left() +
                                            pTmp->getFrameArea().Width()/2,
                                            pTmp->getFrameArea().Top() +
                                            pTmp->getFrameArea().Height()/2 );
                                bool bAccept = false;
                                switch( nDir ) {
                                    case SwMove::RIGHT:
                                    {
                                        bAccept = LessX( aCenter, aNew, bOld )
                                             && ( !pNewFly ||
                                             LessX( aNew, aBest, false ) );
                                        break;
                                    }
                                    case SwMove::LEFT:
                                    {
                                        bAccept = LessX( aNew, aCenter, !bOld )
                                             && ( !pNewFly ||
                                             LessX( aBest, aNew, true ) );
                                        break;
                                    }
                                    case SwMove::UP:
                                    {
                                        bAccept = LessY( aNew, aCenter, !bOld )
                                             && ( !pNewFly ||
                                             LessY( aBest, aNew, true ) );
                                        break;
                                    }
                                    case SwMove::DOWN:
                                    {
                                        bAccept = LessY( aCenter, aNew, bOld )
                                             && ( !pNewFly ||
                                             LessY( aNew, aBest, false ) );
                                        break;
                                    }
                                }
                                if( bAccept )
                                {
                                    pNewFly = pTmp;
                                    aBest = aNew;
                                }
                            }
                        }
                    }
                }

                if( pNewFly )
                {
                    SwPosition aPos( *pNewFly->GetFormat()->
                                        GetContent().GetContentIdx());
                    aAnch.SetAnchor( &aPos );
                    bRet = true;
                }
                break;
            }
            default: break;
        }
        if( bRet )
        {
            StartAllAction();
            // --> handle change of anchor node:
            // if count of the anchor frame also change, the fly frames have to be
            // re-created. Thus, delete all fly frames except the <this> before the
            // anchor attribute is change and re-create them afterwards.
            {
                std::unique_ptr<SwHandleAnchorNodeChg> pHandleAnchorNodeChg;
                SwFlyFrameFormat* pFlyFrameFormat( dynamic_cast<SwFlyFrameFormat*>(&rFormat) );
                if ( pFlyFrameFormat )
                {
                    pHandleAnchorNodeChg.reset(
                        new SwHandleAnchorNodeChg( *pFlyFrameFormat, aAnch ));
                }
                rFormat.GetDoc()->SetAttr( aAnch, rFormat );
            }
            // #i28701# - no call of method
            // <CheckCharRectAndTopOfLine()> for to-character anchored
            // Writer fly frame needed. This method call can cause a
            // format of the anchor frame, which is no longer intended.
                    // Instead clear the anchor character rectangle and
                    // the top of line values for all to-character anchored objects.
            pAnchoredObj->ClearCharRectAndTopOfLine();
            EndAllAction();
        }
    }
    return bRet;
}

const SdrMarkList* SwFEShell::GetMarkList_() const
{
    const SdrMarkList* pMarkList = nullptr;
    if( Imp()->GetDrawView() != nullptr )
        pMarkList = &Imp()->GetDrawView()->GetMarkedObjectList();
    return pMarkList;
}

FrameTypeFlags SwFEShell::GetSelFrameType() const
{
    FrameTypeFlags eType;

    // get marked frame list, and check if anything is selected
    const SdrMarkList* pMarkList = GetMarkList_();
    if( pMarkList == nullptr  ||  pMarkList->GetMarkCount() == 0 )
        eType = FrameTypeFlags::NONE;
    else
    {
        // obtain marked item as fly frame; if no fly frame, it must
        // be a draw object
        const SwFlyFrame* pFly = ::GetFlyFromMarked(pMarkList, const_cast<SwFEShell*>(this));
        if ( pFly != nullptr )
        {
            if( pFly->IsFlyLayFrame() )
                eType = FrameTypeFlags::FLY_FREE;
            else if( pFly->IsFlyAtContentFrame() )
                eType = FrameTypeFlags::FLY_ATCNT;
            else
            {
                OSL_ENSURE( pFly->IsFlyInContentFrame(), "New frametype?" );
                eType = FrameTypeFlags::FLY_INCNT;
            }
        }
        else
            eType = FrameTypeFlags::DRAWOBJ;
    }

    return eType;
}

// does the draw selection contain a control?
bool SwFEShell::IsSelContainsControl() const
{
    bool bRet = false;

    // basically, copy the mechanism from GetSelFrameType(), but call
    // CheckControl... if you get a drawing object
    const SdrMarkList* pMarkList = GetMarkList_();
    if( pMarkList != nullptr  &&  pMarkList->GetMarkCount() == 1 )
    {
        // if we have one marked object, get the SdrObject and check
        // whether it contains a control
        const SdrObject* pSdrObject = pMarkList->GetMark( 0 )->GetMarkedSdrObj();
        bRet = pSdrObject && ::CheckControlLayer( pSdrObject );
    }
    return bRet;
}

void SwFEShell::ScrollTo( const Point &rPt )
{
    const SwRect aRect( rPt, rPt );
    if ( IsScrollMDI( this, aRect ) &&
         (!Imp()->GetDrawView()->GetMarkedObjectList().GetMarkCount() ||
          Imp()->IsDragPossible( rPt )) )
    {
        ScrollMDI( this, aRect, SCROLLVAL, SCROLLVAL );
    }
}

void SwFEShell::SetDragMode( SdrDragMode eDragMode )
{
    if ( Imp()->HasDrawView() )
        Imp()->GetDrawView()->SetDragMode( eDragMode );
}

SdrDragMode SwFEShell::GetDragMode() const
{
    SdrDragMode nRet = SdrDragMode(0);
    if ( Imp()->HasDrawView() )
    {
        nRet = Imp()->GetDrawView()->GetDragMode();
    }
    return nRet;
}

void SwFEShell::StartCropImage()
{
    if ( !Imp()->HasDrawView() )
    {
        return;
    }
    SdrView *pView = Imp()->GetDrawView();
    if (!pView) return;

    const SdrMarkList &rMarkList = pView->GetMarkedObjectList();
    if( 0 == rMarkList.GetMarkCount() ) {
        // No object selected
        return;
    }

    // If more than a single SwVirtFlyDrawObj is selected, select only the first SwVirtFlyDrawObj
    if ( rMarkList.GetMarkCount() > 1 )
    {
        for ( size_t i = 0; i < rMarkList.GetMarkCount(); ++i )
        {
            SdrObject *pTmpObj = rMarkList.GetMark( i )->GetMarkedSdrObj();
            bool bForget = dynamic_cast<const SwVirtFlyDrawObj*>( pTmpObj) !=  nullptr;
            if( bForget )
            {
                pView->UnmarkAll();
                pView->MarkObj( pTmpObj, Imp()->GetPageView() );
                break;
            }
        }
    }

    // Activate CROP mode
    pView->SetEditMode( SdrViewEditMode::Edit );
    SetDragMode( SdrDragMode::Crop );
}

void SwFEShell::BeginDrag( const Point* pPt, bool bIsShift)
{
    SdrView *pView = Imp()->GetDrawView();
    if ( pView && pView->AreObjectsMarked() )
    {
        m_pChainFrom.reset();
        m_pChainTo.reset();
        SdrHdl* pHdl = pView->PickHandle( *pPt );
        if (pView->BegDragObj( *pPt, nullptr, pHdl ))
            pView->GetDragMethod()->SetShiftPressed( bIsShift );
        ::FrameNotify( this );
    }
}

void SwFEShell::Drag( const Point *pPt, bool )
{
    OSL_ENSURE( Imp()->HasDrawView(), "Drag without DrawView?" );
    if ( HasDrawViewDrag() )
    {
        ScrollTo( *pPt );
        Imp()->GetDrawView()->MovDragObj( *pPt );
        Imp()->GetDrawView()->ShowDragAnchor();
        ::FrameNotify( this );
    }
}

void SwFEShell::EndDrag()
{
    OSL_ENSURE( Imp()->HasDrawView(), "EndDrag without DrawView?" );
    SdrView *pView = Imp()->GetDrawView();
    if ( !pView->IsDragObj() )
        return;

    for(SwViewShell& rSh : GetRingContainer())
        rSh.StartAction();

    StartUndo( SwUndoId::START );

    // #50778# Bug during dragging: In StartAction a HideShowXor is called.
    // In EndDragObj() this is reversed, for no reason and even wrong.
    // To restore consistency we should bring up the Xor again.

    // Reanimation from the hack #50778 to fix bug #97057
    // May be not the best solution, but the one with lowest risc at the moment.
    // pView->ShowShownXor( GetOut() );

    pView->EndDragObj();

    // DrawUndo on to flyframes are not stored
    //             The flys change the flag.
    GetDoc()->GetIDocumentUndoRedo().DoDrawUndo(true);
    ChgAnchor( RndStdIds::FLY_AT_PARA, true );

    EndUndo( SwUndoId::END );

    for(SwViewShell& rSh : GetRingContainer())
    {
        rSh.EndAction();
        if( auto pCursorShell = dynamic_cast<SwCursorShell *>(&rSh) )
            pCursorShell->CallChgLnk();
    }

    GetDoc()->getIDocumentState().SetModified();
    ::FrameNotify( this );
}

void SwFEShell::BreakDrag()
{
    OSL_ENSURE( Imp()->HasDrawView(), "BreakDrag without DrawView?" );
    if( HasDrawViewDrag() )
        Imp()->GetDrawView()->BrkDragObj();
    SetChainMarker();
}

// If a fly is selected, pulls the crsr in the first ContentFrame
const SwFrameFormat* SwFEShell::SelFlyGrabCursor()
{
    if ( Imp()->HasDrawView() )
    {
        const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkedObjectList();
        SwFlyFrame *pFly = ::GetFlyFromMarked( &rMrkList, this );

        if( pFly )
        {
            SwContentFrame *pCFrame = pFly->ContainsContent();
            if ( pCFrame )
            {
                // --> assure, that the cursor is consistent.
                KillPams();
                ClearMark();
                SwPaM       *pCursor  = GetCursor();

                if (pCFrame->IsTextFrame())
                {
                    *pCursor->GetPoint() = static_cast<SwTextFrame *>(pCFrame)
                        ->MapViewToModelPos(TextFrameIndex(0));
                }
                else
                {
                    assert(pCFrame->IsNoTextFrame());
                    SwContentNode *const pCNode = static_cast<SwNoTextFrame *>(pCFrame)->GetNode();
                    pCursor->GetPoint()->nNode = *pCNode;
                    pCursor->GetPoint()->nContent.Assign( pCNode, 0 );
                }

                SwRect& rChrRect = const_cast<SwRect&>(GetCharRect());
                rChrRect = pFly->getFramePrintArea();
                rChrRect.Pos() += pFly->getFrameArea().Pos();
                GetCursorDocPos() = rChrRect.Pos();
            }
            return pFly->GetFormat();
        }
    }
    return nullptr;
}

// Selection to above/below (Z-Order)
static void lcl_NotifyNeighbours( const SdrMarkList *pLst )
{
    // Rules for evasion have changed.
    // 1. The environment of the fly and everything inside should be notified
    // 2. The content of the frame itself has to be notified
    // 3. Frames displaced by the frame have to be notified
    // 4. Also Drawing objects can displace frames
    for( size_t j = 0; j < pLst->GetMarkCount(); ++j )
    {
        SwPageFrame *pPage;
        bool bCheckNeighbours = false;
        sal_Int16 aHori = text::HoriOrientation::NONE;
        SwRect aRect;
        SdrObject *pO = pLst->GetMark( j )->GetMarkedSdrObj();
        if (SwVirtFlyDrawObj* pVirtO = dynamic_cast<SwVirtFlyDrawObj*>(pO))
        {
            SwFlyFrame *pFly = pVirtO->GetFlyFrame();

            const SwFormatHoriOrient &rHori = pFly->GetFormat()->GetHoriOrient();
            aHori = rHori.GetHoriOrient();
            if( text::HoriOrientation::NONE != aHori && text::HoriOrientation::CENTER != aHori &&
                pFly->IsFlyAtContentFrame() )
            {
                bCheckNeighbours = true;
                pFly->InvalidatePos();
                SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*pFly);
                aFrm.Pos().AdjustY(1 );
            }

            pPage = pFly->FindPageFrame();
            aRect = pFly->getFrameArea();
        }
        else
        {
            SwFrame* pAnch = static_cast<SwDrawContact*>( GetUserCall(pO) )->GetAnchorFrame( pO );
            if( !pAnch )
                continue;
            pPage = pAnch->FindPageFrame();
            // #i68520# - naming changed
            aRect = GetBoundRectOfAnchoredObj( pO );
        }

        const size_t nCount = pPage->GetSortedObjs() ? pPage->GetSortedObjs()->size() : 0;
        for ( size_t i = 0; i < nCount; ++i )
        {
            SwAnchoredObject* pAnchoredObj = (*pPage->GetSortedObjs())[i];
            SwFlyFrame* pAct = pAnchoredObj->DynCastFlyFrame();
            if ( !pAct )
                continue;

            SwRect aTmpCalcPnt( pAct->getFramePrintArea() );
            aTmpCalcPnt += pAct->getFrameArea().Pos();
            if ( aRect.Overlaps( aTmpCalcPnt ) )
            {
                SwContentFrame *pCnt = pAct->ContainsContent();
                while ( pCnt )
                {
                    aTmpCalcPnt = pCnt->getFramePrintArea();
                    aTmpCalcPnt += pCnt->getFrameArea().Pos();
                    if ( aRect.Overlaps( aTmpCalcPnt ) )
                        static_cast<SwFrame*>(pCnt)->Prepare( PrepareHint::FlyFrameAttributesChanged );
                    pCnt = pCnt->GetNextContentFrame();
                }
            }
            if ( bCheckNeighbours && pAct->IsFlyAtContentFrame() )
            {
                const SwFormatHoriOrient &rH = pAct->GetFormat()->GetHoriOrient();
                if ( rH.GetHoriOrient() == aHori &&
                     pAct->getFrameArea().Top()    <= aRect.Bottom() &&
                     pAct->getFrameArea().Bottom() >= aRect.Top() )
                {
                    pAct->InvalidatePos();
                    SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*pAct);
                    aFrm.Pos().AdjustY(1 );
                }
            }
        }
    }
}

void SwFEShell::SetLineEnds(SfxItemSet& rAttr, SdrObject const & rObj, sal_uInt16 nSlotId)
{
    SdrModel& rModel(rObj.getSdrModelFromSdrObject());

    if ( !(nSlotId == SID_LINE_ARROW_START      ||
          nSlotId == SID_LINE_ARROW_END        ||
          nSlotId == SID_LINE_ARROWS           ||
          nSlotId == SID_LINE_ARROW_CIRCLE     ||
          nSlotId == SID_LINE_CIRCLE_ARROW     ||
          nSlotId == SID_LINE_ARROW_SQUARE     ||
          nSlotId == SID_LINE_SQUARE_ARROW     ||
          nSlotId == SID_DRAW_MEASURELINE) )
        return;

    // set attributes of line start and ends

    // arrowhead
    ::basegfx::B2DPolyPolygon aArrow( getPolygon( RID_SVXSTR_ARROW, rModel ) );
    if( !aArrow.count() )
    {
        ::basegfx::B2DPolygon aNewArrow;
        aNewArrow.append(::basegfx::B2DPoint(10.0, 0.0));
        aNewArrow.append(::basegfx::B2DPoint(0.0, 30.0));
        aNewArrow.append(::basegfx::B2DPoint(20.0, 30.0));
        aNewArrow.setClosed(true);
        aArrow.append(aNewArrow);
    }

    // Circles
    ::basegfx::B2DPolyPolygon aCircle( getPolygon( RID_SVXSTR_CIRCLE, rModel ) );
    if( !aCircle.count() )
    {
        ::basegfx::B2DPolygon aNewCircle = ::basegfx::utils::createPolygonFromEllipse(::basegfx::B2DPoint(0.0, 0.0), 250.0, 250.0);
        aNewCircle.setClosed(true);
        aCircle.append(aNewCircle);
    }

    // Square
    ::basegfx::B2DPolyPolygon aSquare( getPolygon( RID_SVXSTR_SQUARE, rModel ) );
    if( !aSquare.count() )
    {
        ::basegfx::B2DPolygon aNewSquare;
        aNewSquare.append(::basegfx::B2DPoint(0.0, 0.0));
        aNewSquare.append(::basegfx::B2DPoint(10.0, 0.0));
        aNewSquare.append(::basegfx::B2DPoint(10.0, 10.0));
        aNewSquare.append(::basegfx::B2DPoint(0.0, 10.0));
        aNewSquare.setClosed(true);
        aSquare.append(aNewSquare);
    }

    SfxItemSet aSet( rModel.GetItemPool() );
    tools::Long nWidth = 100; // (1/100th mm)

    // determine line width and calculate with it the line end width
    if( aSet.GetItemState( XATTR_LINEWIDTH ) != SfxItemState::DONTCARE )
    {
        tools::Long nValue = aSet.Get( XATTR_LINEWIDTH ).GetValue();
        if( nValue > 0 )
            nWidth = nValue * 3;
    }

    switch (nSlotId)
    {
        case SID_LINE_ARROWS:
        case SID_DRAW_MEASURELINE:
        {
            // connector with arrow ends
            rAttr.Put(XLineStartItem(SvxResId(RID_SVXSTR_ARROW), aArrow));
            rAttr.Put(XLineStartWidthItem(nWidth));
            rAttr.Put(XLineEndItem(SvxResId(RID_SVXSTR_ARROW), aArrow));
            rAttr.Put(XLineEndWidthItem(nWidth));
        }
        break;

        case SID_LINE_ARROW_START:
        case SID_LINE_ARROW_CIRCLE:
        case SID_LINE_ARROW_SQUARE:
        {
            // connector with arrow start
            rAttr.Put(XLineStartItem(SvxResId(RID_SVXSTR_ARROW), aArrow));
            rAttr.Put(XLineStartWidthItem(nWidth));
        }
        break;

        case SID_LINE_ARROW_END:
        case SID_LINE_CIRCLE_ARROW:
        case SID_LINE_SQUARE_ARROW:
        {
            // connector with arrow end
            rAttr.Put(XLineEndItem(SvxResId(RID_SVXSTR_ARROW), aArrow));
            rAttr.Put(XLineEndWidthItem(nWidth));
        }
        break;
    }

    // and again, for the still missing ends
    switch (nSlotId)
    {
        case SID_LINE_ARROW_CIRCLE:
        {
            // circle end
            rAttr.Put(XLineEndItem(SvxResId(RID_SVXSTR_CIRCLE), aCircle));
            rAttr.Put(XLineEndWidthItem(nWidth));
        }
        break;

        case SID_LINE_CIRCLE_ARROW:
        {
            // circle start
            rAttr.Put(XLineStartItem(SvxResId(RID_SVXSTR_CIRCLE), aCircle));
            rAttr.Put(XLineStartWidthItem(nWidth));
        }
        break;

        case SID_LINE_ARROW_SQUARE:
        {
            // square end
            rAttr.Put(XLineEndItem(SvxResId(RID_SVXSTR_SQUARE), aSquare));
            rAttr.Put(XLineEndWidthItem(nWidth));
        }
        break;

        case SID_LINE_SQUARE_ARROW:
        {
            // square start
            rAttr.Put(XLineStartItem(SvxResId(RID_SVXSTR_SQUARE), aSquare));
            rAttr.Put(XLineStartWidthItem(nWidth));
        }
        break;
    }

}

void SwFEShell::SelectionToTop( bool bTop )
{
    OSL_ENSURE( Imp()->HasDrawView(), "SelectionToTop without DrawView?" );
    const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkedObjectList();
    OSL_ENSURE( rMrkList.GetMarkCount(), "No object selected." );

    SwFlyFrame *pFly = ::GetFlyFromMarked( &rMrkList, this );
    if ( pFly && pFly->IsFlyInContentFrame() )
        return;

    StartAllAction();
    if ( bTop )
        Imp()->GetDrawView()->PutMarkedToTop();
    else
        Imp()->GetDrawView()->MovMarkedToTop();
    ::lcl_NotifyNeighbours( &rMrkList );

    // Does the selection contain a textbox?
    for (size_t i = 0; i < rMrkList.GetMarkCount(); i++)
        if (auto pObj = rMrkList.GetMark(i)->GetMarkedSdrObj())
            // Get the textbox-shape
            if (auto pFormat = FindFrameFormat(pObj))
            {
                // If it has not textframe skip...
                if (!SwTextBoxHelper::isTextBoxShapeHasValidTextFrame(pFormat))
                    continue;
                // If it has a textframe so it is a textbox, get its page
                if (auto pDrwModel
                    = pFormat->GetDoc()->getIDocumentDrawModelAccess().GetDrawModel())
                    // Not really understood why everything is on page 0...
                    // but it is easier to handle sdrobjects, that's true
                    if (auto pPage = pDrwModel->GetPage(0))
                    {
                        // nShift: it means how many layers the pObj have to be shifted up,
                        // in order not to interfere with other shapes and textboxes.
                        // Situations:
                        // - The next shape has textframe: This shape have to shifted with
                        //   two layers.
                        // - The next shape has not got textframe: This shape have to be
                        //   shifted only one layer up.
                        // - The next shape is null:
                        //      - This shape is already at heaven: Only the textframe have
                        //        to be adjusted.
                        sal_uInt32 nShift = 0;
                        // Get the one level higher object (note: can be nullptr!)
                        const auto pNextObj = pPage->SetObjectOrdNum(pObj->GetOrdNum() + 1, pObj->GetOrdNum() + 1);
                        // If there is a higher object (not null)...
                        if (pNextObj)
                        {
                            // One level shift is necessary
                            nShift++;
                            // If this object is a textbox, two level increasing needed
                            // (one for the shape and one for the frame)
                            if (auto pNextFormat = FindFrameFormat(pNextObj))
                                if (SwTextBoxHelper::isTextBox(pNextFormat, RES_DRAWFRMFMT)
                                    || SwTextBoxHelper::isTextBox(pNextFormat, RES_FLYFRMFMT))
                                    nShift++;
                        }
                        // Set the new z-order.
                        pPage->SetObjectOrdNum(pObj->GetOrdNum(), pObj->GetOrdNum() + nShift);
                    }
                // The shape is on the right level, correct the layer of the frame
                SwTextBoxHelper::DoTextBoxZOrderCorrection(pFormat, pObj);
            }

    GetDoc()->getIDocumentState().SetModified();
    EndAllAction();
}

void SwFEShell::SelectionToBottom( bool bBottom )
{
    OSL_ENSURE( Imp()->HasDrawView(), "SelectionToBottom without DrawView?" );
    const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkedObjectList();
    OSL_ENSURE( rMrkList.GetMarkCount(), "No object selected." );

    SwFlyFrame *pFly = ::GetFlyFromMarked( &rMrkList, this );
    if ( pFly && pFly->IsFlyInContentFrame() )
        return;

    StartAllAction();
    if ( bBottom )
        Imp()->GetDrawView()->PutMarkedToBtm();
    else
        Imp()->GetDrawView()->MovMarkedToBtm();
    ::lcl_NotifyNeighbours( &rMrkList );

    // If the selection has textbox
    for(size_t i = 0; i < rMrkList.GetMarkCount(); i++)
        if (auto pObj = rMrkList.GetMark(i)->GetMarkedSdrObj())
            // Get the shape of the textbox
            if (auto pFormat = FindFrameFormat(pObj))
            {
                // If the shape has not textframes skip.
                if (!SwTextBoxHelper::isTextBoxShapeHasValidTextFrame(pFormat))
                    continue;
                // If has, move the shape to correct level with...
                if (auto pDrwModel
                    = pFormat->GetDoc()->getIDocumentDrawModelAccess().GetDrawModel())
                    if (auto pPage = pDrwModel->GetPage(0))
                    {
                        const auto pNextObj = pPage->SetObjectOrdNum(pObj->GetOrdNum() - 1, pObj->GetOrdNum() - 1);
                        // If there is a lower object (not null)...
                        if (pNextObj)
                        {
                            // If the lower has no textframe, just do nothing, else move by one lower
                            if (auto pNextFormat = FindFrameFormat(pNextObj))
                                if (SwTextBoxHelper::isTextBox(pNextFormat, RES_DRAWFRMFMT)
                                    || SwTextBoxHelper::isTextBox(pNextFormat, RES_FLYFRMFMT))
                                    pPage->SetObjectOrdNum(pObj->GetOrdNum(), pObj->GetOrdNum() - 1);
                        }
                    }
                // And set correct layer for the selected textbox.
                SwTextBoxHelper::DoTextBoxZOrderCorrection(pFormat, pObj);
            }

    GetDoc()->getIDocumentState().SetModified();
    EndAllAction();
}

// Object above/below the document? 2 Controls, 1 Heaven, 0 Hell,
// SDRLAYER_NOTFOUND Ambiguous
SdrLayerID SwFEShell::GetLayerId() const
{
    if ( !Imp()->HasDrawView() )
        return SDRLAYER_NOTFOUND;

    SdrLayerID nRet = SDRLAYER_NOTFOUND;
    const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkedObjectList();
    for ( size_t i = 0; i < rMrkList.GetMarkCount(); ++i )
    {
        const SdrObject *pObj = rMrkList.GetMark( i )->GetMarkedSdrObj();
        if( !pObj )
            continue;
        if ( nRet == SDRLAYER_NOTFOUND )
            nRet = pObj->GetLayer();
        else if ( nRet != pObj->GetLayer() )
        {
            return SDRLAYER_NOTFOUND;
        }
    }
    return nRet;
}

// Object above/below the document
// Note: only visible objects can be marked. Thus, objects with invisible
//       layer IDs have not to be considered.
//       If <SwFEShell> exists, layout exists!!
void SwFEShell::ChangeOpaque( SdrLayerID nLayerId )
{
    if ( !Imp()->HasDrawView() )
        return;

    const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkedObjectList();
    const IDocumentDrawModelAccess& rIDDMA = getIDocumentDrawModelAccess();
    // correct type of <nControls>
    for ( size_t i = 0; i < rMrkList.GetMarkCount(); ++i )
    {
        SdrObject* pObj = rMrkList.GetMark( i )->GetMarkedSdrObj();
        if( !pObj )
            continue;
        // or group objects containing controls.
        // --> #i113730#
        // consider that a member of a drawing group has been selected.
        const SwContact* pContact = ::GetUserCall( pObj );
        OSL_ENSURE( pContact && pContact->GetMaster(), "<SwFEShell::ChangeOpaque(..)> - missing contact or missing master object at contact!" );
        const bool bControlObj = ( pContact && pContact->GetMaster() )
                                 ? ::CheckControlLayer( pContact->GetMaster() )
                                 : ::CheckControlLayer( pObj );
        if ( !bControlObj && pObj->GetLayer() != nLayerId )
        {
            pObj->SetLayer( nLayerId );
            InvalidateWindows( SwRect( pObj->GetCurrentBoundRect() ) );
            if (SwVirtFlyDrawObj* pVirtO = dynamic_cast<SwVirtFlyDrawObj*>(pObj))
            {
                SwFormat *pFormat = pVirtO->GetFlyFrame()->GetFormat();
                SvxOpaqueItem aOpa( pFormat->GetOpaque() );
                aOpa.SetValue(  nLayerId == rIDDMA.GetHellId() );
                pFormat->SetFormatAttr( aOpa );
                // If pObj has textframe, put its textframe to the right level
                if (auto pTextBx = FindFrameFormat(pObj))
                    SwTextBoxHelper::DoTextBoxZOrderCorrection(pTextBx, pObj);
            }
        }
    }
    GetDoc()->getIDocumentState().SetModified();
}

void SwFEShell::SelectionToHeaven()
{
    ChangeOpaque( getIDocumentDrawModelAccess().GetHeavenId() );
}

void SwFEShell::SelectionToHell()
{
    ChangeOpaque( getIDocumentDrawModelAccess().GetHellId() );
}

size_t SwFEShell::IsObjSelected() const
{
    if ( IsFrameSelected() || !Imp()->HasDrawView() )
        return 0;

    return Imp()->GetDrawView()->GetMarkedObjectList().GetMarkCount();
}

bool SwFEShell::IsFrameSelected() const
{
    if ( !Imp()->HasDrawView() )
        return false;
    else
        return nullptr != ::GetFlyFromMarked( &Imp()->GetDrawView()->GetMarkedObjectList(),
                                        const_cast<SwFEShell*>(this) );
}

bool SwFEShell::IsObjSelected( const SdrObject& rObj ) const
{
    if ( IsFrameSelected() || !Imp()->HasDrawView() )
        return false;
    else
        return Imp()->GetDrawView()->IsObjMarked( &rObj );
}

bool SwFEShell::IsRotationOfSwGrfNodePossible() const
{
    // RotGrfFlyFrame: check if RotationMode is possible
    const SdrView *pSdrView = Imp()->GetDrawView();

    if(pSdrView)
    {
        const SdrMarkList& rList(pSdrView->GetMarkedObjectList());

        if(1 == rList.GetMarkCount())
        {
            const SwVirtFlyDrawObj* pVirtFlyDraw(dynamic_cast< const SwVirtFlyDrawObj* >(rList.GetMark(0)->GetMarkedSdrObj()));

            if(nullptr != pVirtFlyDraw)
            {
                return pVirtFlyDraw->ContainsSwGrfNode();
            }
        }
    }

    return false;
}

bool SwFEShell::IsObjSameLevelWithMarked(const SdrObject* pObj) const
{
    if (pObj)
    {
        const SdrMarkList& aMarkList = Imp()->GetDrawView()->GetMarkedObjectList();
        if (aMarkList.GetMarkCount() == 0)
        {
            return true;
        }
        SdrMark* pM=aMarkList.GetMark(0);
        if (pM)
        {
            SdrObject* pMarkObj = pM->GetMarkedSdrObj();
            if (pMarkObj && pMarkObj->getParentSdrObjectFromSdrObject() == pObj->getParentSdrObjectFromSdrObject())
                return true;
        }
    }
    return false;
}

void SwFEShell::EndTextEdit()
{
    // Terminate the TextEditMode. If required (default if the object
    // does not contain any more text and does not carry attributes) the object
    // is deleted. All other objects marked are preserved.

    OSL_ENSURE( Imp()->HasDrawView() && Imp()->GetDrawView()->IsTextEdit(),
            "EndTextEdit a no Object" );

    StartAllAction();
    SdrView *pView = Imp()->GetDrawView();
    SdrObject *pObj = pView->GetTextEditObject();
    SdrObjUserCall* pUserCall = GetUserCall(pObj);
    if( nullptr != pUserCall )
    {
        SdrObject *pTmp = static_cast<SwContact*>(pUserCall)->GetMaster();
        if( !pTmp )
            pTmp = pObj;
        pUserCall->Changed( *pTmp, SdrUserCallType::Resize, pTmp->GetLastBoundRect() );
    }
    if ( !pObj->getParentSdrObjectFromSdrObject() )
    {
        if ( SdrEndTextEditKind::ShouldBeDeleted == pView->SdrEndTextEdit(true) )
        {
            if ( pView->GetMarkedObjectList().GetMarkCount() > 1 )
            {
                SdrMarkList aSave( pView->GetMarkedObjectList() );
                aSave.DeleteMark( aSave.FindObject( pObj ) );
                if ( aSave.GetMarkCount() )
                {
                    pView->UnmarkAll();
                    pView->MarkObj( pObj, Imp()->GetPageView() );
                }
                DelSelectedObj();
                for ( size_t i = 0; i < aSave.GetMarkCount(); ++i )
                    pView->MarkObj( aSave.GetMark( i )->GetMarkedSdrObj(), Imp()->GetPageView() );
            }
            else
                DelSelectedObj();
        }
    }
    else
        pView->SdrEndTextEdit();

    if (comphelper::LibreOfficeKit::isActive())
        SfxLokHelper::notifyOtherViews(GetSfxViewShell(), LOK_CALLBACK_VIEW_LOCK, "rectangle", "EMPTY");

    EndAllAction();
}

bool SwFEShell::IsInsideSelectedObj( const Point &rPt )
{
    if( Imp()->HasDrawView() )
    {
        SwDrawView *pDView = Imp()->GetDrawView();

        if( pDView->GetMarkedObjectList().GetMarkCount() &&
            pDView->IsMarkedObjHit( rPt ) )
        {
            return true;
        }
    }
    return false;
}

bool SwFEShell::IsObjSelectable( const Point& rPt )
{
    CurrShell aCurr(this);
    SwDrawView *pDView = Imp()->GetDrawView();
    bool bRet = false;
    if( pDView )
    {
        SdrPageView* pPV;
        const auto nOld = pDView->GetHitTolerancePixel();
        pDView->SetHitTolerancePixel( pDView->GetMarkHdlSizePixel()/2 );

        bRet = pDView->PickObj(rPt, pDView->getHitTolLog(), pPV, SdrSearchOptions::PICKMARKABLE) != nullptr;
        pDView->SetHitTolerancePixel( nOld );
    }
    return bRet;
}

SdrObject* SwFEShell::GetObjAt( const Point& rPt )
{
    SdrObject* pRet = nullptr;
    CurrShell aCurr(this);
    SwDrawView *pDView = Imp()->GetDrawView();
    if( pDView )
    {
        SdrPageView* pPV;
        const auto nOld = pDView->GetHitTolerancePixel();
        pDView->SetHitTolerancePixel( pDView->GetMarkHdlSizePixel()/2 );

        pRet = pDView->PickObj(rPt, pDView->getHitTolLog(), pPV, SdrSearchOptions::PICKMARKABLE);
        pDView->SetHitTolerancePixel( nOld );
    }
    return pRet;
}

// Test if there is an object at that position and if it should be selected.
bool SwFEShell::ShouldObjectBeSelected(const Point& rPt)
{
    CurrShell aCurr(this);
    SwDrawView *pDrawView = Imp()->GetDrawView();
    bool bRet(false);

    if(pDrawView)
    {
        SdrPageView* pPV;
        const auto nOld(pDrawView->GetHitTolerancePixel());

        pDrawView->SetHitTolerancePixel(pDrawView->GetMarkHdlSizePixel()/2);
        SdrObject* pObj = pDrawView->PickObj(rPt, pDrawView->getHitTolLog(), pPV, SdrSearchOptions::PICKMARKABLE);
        pDrawView->SetHitTolerancePixel(nOld);

        if (pObj)
        {
            bRet = true;
            const IDocumentDrawModelAccess& rIDDMA = getIDocumentDrawModelAccess();
            // #i89920#
            // Do not select object in background which is overlapping this text
            // at the given position.
            bool bObjInBackground( false );
            {
                if ( pObj->GetLayer() == rIDDMA.GetHellId() )
                {
                    const SwAnchoredObject* pAnchoredObj = ::GetUserCall( pObj )->GetAnchoredObj( pObj );
                    const SwFrameFormat& rFormat = pAnchoredObj->GetFrameFormat();
                    const SwFormatSurround& rSurround = rFormat.GetSurround();
                    if ( rSurround.GetSurround() == css::text::WrapTextMode_THROUGH )
                    {
                        bObjInBackground = true;
                    }
                }
            }
            if ( bObjInBackground )
            {
                const SwPageFrame* pPageFrame = GetLayout()->GetPageAtPos( rPt );
                if( pPageFrame )
                {
                    const SwContentFrame* pContentFrame( pPageFrame->ContainsContent() );
                    while ( pContentFrame )
                    {
                        if ( pContentFrame->UnionFrame().Contains( rPt ) )
                        {
                            const SwTextFrame* pTextFrame = pContentFrame->DynCastTextFrame();
                            if ( pTextFrame )
                            {
                                SwPosition aPos(GetDoc()->GetNodes());
                                Point aTmpPt( rPt );
                                if (pTextFrame->GetKeyCursorOfst(&aPos, aTmpPt))
                                {
                                    SwRect aCursorCharRect;
                                    if (pTextFrame->GetCharRect(aCursorCharRect,
                                                aPos))
                                    {
                                        if ( aCursorCharRect.Overlaps( SwRect( pObj->GetLastBoundRect() ) ) )
                                        {
                                            bRet = false;
                                        }
                                    }
                                }
                            }
                            else
                            {
                                bRet = false;
                            }
                            break;
                        }

                        pContentFrame = pContentFrame->GetNextContentFrame();
                    }
                }
            }

            // Don't select header / footer objects in body edition and vice-versa
            SwContact* pContact = static_cast<SwContact*>(pObj->GetUserCall());
            if (pContact && !pContact->ObjAnchoredAtPage() )
            {
                const SwPosition& rPos = pContact->GetContentAnchor();
                bool bInHdrFtr = GetDoc()->IsInHeaderFooter( rPos.nNode );
                if (IsHeaderFooterEdit() != bInHdrFtr)
                {
                    bRet = false;
                }
            }

            if ( bRet )
            {
                const SdrPage* pPage = rIDDMA.GetDrawModel()->GetPage(0);
                for(size_t a = pObj->GetOrdNum()+1; bRet && a < pPage->GetObjCount(); ++a)
                {
                    SdrObject *pCandidate = pPage->GetObj(a);

                    SwVirtFlyDrawObj* pDrawObj = dynamic_cast<SwVirtFlyDrawObj*>(pCandidate);
                    if (pDrawObj && pDrawObj->GetCurrentBoundRect().Contains(rPt))
                    {
                        bRet = false;
                    }
                }
            }
        }
    }

    return bRet;
}

/*
 * If an object was selected, we assume its upper-left corner
 * otherwise the middle of the current CharRects.
 * Does the object include a control or groups,
 * which comprise only controls
 */
static bool lcl_IsControlGroup( const SdrObject *pObj )
{
    bool bRet = false;
    if(dynamic_cast<const SdrUnoObj*>( pObj) !=  nullptr)
        bRet = true;
    else if( auto pObjGroup = dynamic_cast<const SdrObjGroup*>( pObj) )
    {
        bRet = true;
        const SdrObjList *pLst = pObjGroup->GetSubList();
        for ( size_t i = 0; i < pLst->GetObjCount(); ++i )
            if( !::lcl_IsControlGroup( pLst->GetObj( i ) ) )
                return false;
    }
    return bRet;
}

namespace
{
    class MarkableObjectsOnly : public svx::ISdrObjectFilter
    {
    public:
        explicit MarkableObjectsOnly( SdrPageView* i_pPV )
            :m_pPV( i_pPV )
        {
        }

        virtual bool    includeObject( const SdrObject& i_rObject ) const override
        {
            return m_pPV && m_pPV->GetView().IsObjMarkable( &i_rObject, m_pPV );
        }

    private:
        SdrPageView*    m_pPV;
    };
}

const SdrObject* SwFEShell::GetBestObject( bool bNext, GotoObjFlags eType, bool bFlat, const svx::ISdrObjectFilter* pFilter )
{
    if( !Imp()->HasDrawView() )
        return nullptr;

    const SdrObject *pBest  = nullptr,
                    *pTop   = nullptr;

    const tools::Long nTmp = bNext ? LONG_MAX : 0;
    Point aBestPos( nTmp, nTmp );
    Point aTopPos(  nTmp, nTmp );
    Point aCurPos;
    Point aPos;
    bool bNoDraw((GotoObjFlags::DrawAny & eType) == GotoObjFlags::NONE);
    bool bNoFly((GotoObjFlags::FlyAny & eType) == GotoObjFlags::NONE);

    if( !bNoFly && bNoDraw )
    {
        SwFlyFrame *pFly = GetCurrFrame( false )->FindFlyFrame();
        if( pFly )
            pBest = pFly->GetVirtDrawObj();
    }
    const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkedObjectList();
    SdrPageView* pPV = Imp()->GetDrawView()->GetSdrPageView();

    MarkableObjectsOnly aDefaultFilter( pPV );
    if ( !pFilter )
        pFilter = &aDefaultFilter;

    if( !pBest || rMrkList.GetMarkCount() == 1 )
    {
        // Determine starting point
        SdrObjList* pList = nullptr;
        if ( rMrkList.GetMarkCount() )
        {
            const SdrObject* pStartObj = rMrkList.GetMark(0)->GetMarkedSdrObj();
            if( auto pVirtFlyDrawObj = dynamic_cast<const SwVirtFlyDrawObj*>( pStartObj) )
                aPos = pVirtFlyDrawObj->GetFlyFrame()->getFrameArea().Pos();
            else
                aPos = pStartObj->GetSnapRect().TopLeft();

            // If an object inside a group is selected, we want to
            // iterate over the group members.
            if ( ! pStartObj->GetUserCall() )
                pList = pStartObj->getParentSdrObjListFromSdrObject();
        }
        else
        {
            // If no object is selected, we check if we just entered a group.
            // In this case we want to iterate over the group members.
            aPos = GetCharRect().Center();
            const SdrObject* pStartObj = pPV ? pPV->GetCurrentGroup() : nullptr;
            if ( dynamic_cast<const SdrObjGroup*>( pStartObj) )
                pList = pStartObj->GetSubList();
        }

        if ( ! pList )
        {
            // Here we are if
            // A  No object has been selected and no group has been entered or
            // B  An object has been selected and it is not inside a group
            pList = getIDocumentDrawModelAccess().GetDrawModel()->GetPage( 0 );
        }

        OSL_ENSURE( pList, "No object list to iterate" );

        SdrObjListIter aObjIter( pList, bFlat ? SdrIterMode::Flat : SdrIterMode::DeepNoGroups );
        while ( aObjIter.IsMore() )
        {
            SdrObject* pObj = aObjIter.Next();
            SwVirtFlyDrawObj *pVirtO = dynamic_cast<SwVirtFlyDrawObj*>(pObj);
            if( ( bNoFly && pVirtO ) ||
                ( bNoDraw && !pVirtO ) ||
                // Ignore TextBoxes of draw shapes here, so that
                // SwFEShell::SelectObj() won't jump back on this list, meaning
                // we never jump to the next draw shape.
                (pVirtO && pVirtO->IsTextBox()) ||
                ( eType == GotoObjFlags::DrawSimple && lcl_IsControlGroup( pObj ) ) ||
                ( eType == GotoObjFlags::DrawControl && !lcl_IsControlGroup( pObj ) ) ||
                !pFilter->includeObject( *pObj ) )
                continue;
            if (pVirtO)
            {
                SwFlyFrame *pFly = pVirtO->GetFlyFrame();
                if( GotoObjFlags::FlyAny != ( GotoObjFlags::FlyAny & eType ) )
                {
                    switch ( eType )
                    {
                        case GotoObjFlags::FlyFrame:
                            if ( pFly->Lower() && pFly->Lower()->IsNoTextFrame() )
                                continue;
                        break;
                        case GotoObjFlags::FlyGrf:
                            if ( pFly->Lower() &&
                                (!pFly->Lower()->IsNoTextFrame() ||
                                 !static_cast<SwNoTextFrame*>(pFly->Lower())->GetNode()->GetGrfNode()))
                                continue;
                        break;
                        case GotoObjFlags::FlyOLE:
                            if ( pFly->Lower() &&
                                (!pFly->Lower()->IsNoTextFrame() ||
                                 !static_cast<SwNoTextFrame*>(pFly->Lower())->GetNode()->GetOLENode()))
                                continue;
                        break;
                        default: break;
                    }
                }
                aCurPos = pFly->getFrameArea().Pos();
            }
            else
                aCurPos = pObj->GetSnapRect().TopLeft();

            // Special case if another object is on same Y.
            if( aCurPos != aPos &&          // only when it is not me
                aCurPos.getY() == aPos.getY() &&  // Y positions equal
                (bNext? (aCurPos.getX() > aPos.getX()) :  // lies next to me
                        (aCurPos.getX() < aPos.getX())) ) // " reverse
            {
                aBestPos = Point( nTmp, nTmp );
                SdrObjListIter aTmpIter( pList, bFlat ? SdrIterMode::Flat : SdrIterMode::DeepNoGroups );
                while ( aTmpIter.IsMore() )
                {
                    SdrObject* pTmpObj = aTmpIter.Next();
                    pVirtO = dynamic_cast<SwVirtFlyDrawObj*>(pTmpObj);
                    if( ( bNoFly && pVirtO ) || ( bNoDraw && !pVirtO ) )
                        continue;
                    if (pVirtO)
                    {
                        aCurPos = pVirtO->GetFlyFrame()->getFrameArea().Pos();
                    }
                    else
                        aCurPos = pTmpObj->GetCurrentBoundRect().TopLeft();

                    if( aCurPos != aPos && aCurPos.Y() == aPos.Y() &&
                        (bNext? (aCurPos.getX() > aPos.getX()) :  // lies next to me
                                (aCurPos.getX() < aPos.getX())) &&    // " reverse
                        (bNext? (aCurPos.getX() < aBestPos.getX()) :  // better as best
                                (aCurPos.getX() > aBestPos.getX())) ) // " reverse
                    {
                        aBestPos = aCurPos;
                        pBest = pTmpObj;
                    }
                }
                break;
            }

            if( (
                (bNext? (aPos.getY() < aCurPos.getY()) :          // only below me
                        (aPos.getY() > aCurPos.getY())) &&        // " reverse
                (bNext? (aBestPos.getY() > aCurPos.getY()) :      // closer below
                        (aBestPos.getY() < aCurPos.getY()))
                    ) ||    // " reverse
                        (aBestPos.getY() == aCurPos.getY() &&
                (bNext? (aBestPos.getX() > aCurPos.getX()) :      // further left
                        (aBestPos.getX() < aCurPos.getX()))))     // " reverse

            {
                aBestPos = aCurPos;
                pBest = pObj;
            }

            if( (bNext? (aTopPos.getY() > aCurPos.getY()) :       // higher as best
                        (aTopPos.getY() < aCurPos.getY())) ||     // " reverse
                        (aTopPos.getY() == aCurPos.getY() &&
                (bNext? (aTopPos.getX() > aCurPos.getX()) :       // further left
                        (aTopPos.getX() < aCurPos.getX()))))      // " reverse
            {
                aTopPos = aCurPos;
                pTop = pObj;
            }
        }
        // unfortunately nothing found
        if( bNext ? (aBestPos.getX() == LONG_MAX) : (aBestPos.getX() == 0) )
        {
            pBest = pTop;
            SvxSearchDialogWrapper::SetSearchLabel( bNext ? SearchLabel::EndWrapped : SearchLabel::StartWrapped );
        }
    }

    return pBest;
}

bool SwFEShell::GotoObj( bool bNext, GotoObjFlags eType )
{
    SvxSearchDialogWrapper::SetSearchLabel( SearchLabel::Empty );

    const SdrObject* pBest = GetBestObject( bNext, eType );

    if ( !pBest )
    {
        SvxSearchDialogWrapper::SetSearchLabel( SearchLabel::NavElementNotFound );
        return false;
    }

    const SwVirtFlyDrawObj *pVirtO = dynamic_cast<const SwVirtFlyDrawObj*>(pBest);
    if (pVirtO)
    {
        const SwRect& rFrame = pVirtO->GetFlyFrame()->getFrameArea();
        SelectObj( rFrame.Pos(), 0, const_cast<SdrObject*>(pBest) );
        if( !ActionPend() )
            MakeVisible( rFrame );
    }
    else
    {
        SelectObj( Point(), 0, const_cast<SdrObject*>(pBest) );
        if( !ActionPend() )
            MakeVisible( SwRect(pBest->GetCurrentBoundRect()) );
    }
    CallChgLnk();
    return true;
}

bool SwFEShell::BeginCreate( SdrObjKind eSdrObjectKind, const Point &rPos )
{
    bool bRet = false;

    if ( !Imp()->HasDrawView() )
        Imp()->MakeDrawView();

    if ( GetPageNumber( rPos ) )
    {
        Imp()->GetDrawView()->SetCurrentObj( eSdrObjectKind );
        if ( eSdrObjectKind == OBJ_CAPTION )
            bRet = Imp()->GetDrawView()->BegCreateCaptionObj(
                        rPos, Size( lMinBorder - MINFLY, lMinBorder - MINFLY ),
                        GetOut() );
        else
            bRet = Imp()->GetDrawView()->BegCreateObj( rPos, GetOut() );
    }
    if ( bRet )
    {
        ::FrameNotify( this, FLY_DRAG_START );
    }
    return bRet;
}

bool SwFEShell::BeginCreate( SdrObjKind eSdrObjectKind, SdrInventor eObjInventor,
                             const Point &rPos )
{
    bool bRet = false;

    if ( !Imp()->HasDrawView() )
        Imp()->MakeDrawView();

    if ( GetPageNumber( rPos ) )
    {
        Imp()->GetDrawView()->SetCurrentObj( eSdrObjectKind, eObjInventor );
        bRet = Imp()->GetDrawView()->BegCreateObj( rPos, GetOut() );
    }
    if ( bRet )
        ::FrameNotify( this, FLY_DRAG_START );
    return bRet;
}

void SwFEShell::MoveCreate( const Point &rPos )
{
    OSL_ENSURE( Imp()->HasDrawView(), "MoveCreate without DrawView?" );
    if ( GetPageNumber( rPos ) )
    {
        ScrollTo( rPos );
        Imp()->GetDrawView()->MovCreateObj( rPos );
        ::FrameNotify( this );
    }
}

bool SwFEShell::EndCreate( SdrCreateCmd eSdrCreateCmd )
{
    // To assure undo-object from the DrawEngine is not stored,
    // (we create our own undo-object!), temporarily switch-off Undo
    OSL_ENSURE( Imp()->HasDrawView(), "EndCreate without DrawView?" );
    if( !Imp()->GetDrawView()->IsGroupEntered() )
    {
        GetDoc()->GetIDocumentUndoRedo().DoDrawUndo(false);
    }
    bool bCreate = Imp()->GetDrawView()->EndCreateObj( eSdrCreateCmd );
    GetDoc()->GetIDocumentUndoRedo().DoDrawUndo(true);

    if ( !bCreate )
    {
        ::FrameNotify( this, FLY_DRAG_END );
        return false;
    }

    if ( eSdrCreateCmd == SdrCreateCmd::NextPoint )
    {
        ::FrameNotify( this );
        return true;
    }
    return ImpEndCreate();
}

bool SwFEShell::ImpEndCreate()
{
    OSL_ENSURE( Imp()->GetDrawView()->GetMarkedObjectList().GetMarkCount() == 1,
            "New object not selected." );

    SdrObject& rSdrObj = *Imp()->GetDrawView()->GetMarkedObjectList().GetMark(0)->GetMarkedSdrObj();

    if( rSdrObj.GetSnapRect().IsEmpty() )
    {
        // preferably we forget the object, only gives problems
        Imp()->GetDrawView()->DeleteMarked();
        Imp()->GetDrawView()->UnmarkAll();
        ::FrameNotify( this, FLY_DRAG_END );
        return false;
    }

    if( rSdrObj.getParentSdrObjectFromSdrObject() )
    {
        Point aTmpPos( rSdrObj.GetSnapRect().TopLeft() );
        Point aNewAnchor( rSdrObj.getParentSdrObjectFromSdrObject()->GetAnchorPos() );
        // OD 2004-04-05 #i26791# - direct object positioning for group members
        rSdrObj.NbcSetRelativePos( aTmpPos - aNewAnchor );
        rSdrObj.NbcSetAnchorPos( aNewAnchor );
        ::FrameNotify( this );
        return true;
    }

    LockPaint();
    StartAllAction();

    Imp()->GetDrawView()->UnmarkAll();

    const tools::Rectangle &rBound = rSdrObj.GetSnapRect();
    Point aPt( rBound.TopRight() );

    // alien identifier should end up on defaults
    // duplications possible!!
    sal_uInt16 nIdent = SdrInventor::Default == rSdrObj.GetObjInventor()
                        ? rSdrObj.GetObjIdentifier()
                        : 0xFFFF;

    // default for controls character bound, otherwise paragraph bound.
    SwFormatAnchor aAnch;
    const SwFrame *pAnch = nullptr;
    bool bCharBound = false;
    if( dynamic_cast<const SdrUnoObj*>( &rSdrObj) !=  nullptr )
    {
        SwPosition aPos( GetDoc()->GetNodes() );
        SwCursorMoveState aState( CursorMoveState::SetOnlyText );
        Point aPoint( aPt.getX(), aPt.getY() + rBound.GetHeight()/2 );
        GetLayout()->GetModelPositionForViewPoint( &aPos, aPoint, &aState );

        // characterbinding not allowed in readonly-content
        if( !aPos.nNode.GetNode().IsProtect() )
        {
            std::pair<Point, bool> const tmp(aPoint, true);
            pAnch = aPos.nNode.GetNode().GetContentNode()->getLayoutFrame(GetLayout(), &aPos, &tmp);
            SwRect aTmp;
            pAnch->GetCharRect( aTmp, aPos );

            // The crsr should not be too far away
            bCharBound = true;
            constexpr tools::Long constTwips_1cm = o3tl::toTwips(1, o3tl::Length::cm);
            tools::Rectangle aRect( aTmp.SVRect() );
            // Extend by 1 cm in each direction
            aRect.AdjustLeft(-constTwips_1cm);
            aRect.AdjustTop(-constTwips_1cm);
            aRect.AdjustRight(constTwips_1cm);
            aRect.AdjustBottom(constTwips_1cm);

            if( !aRect.Overlaps( rBound ) && !::GetHtmlMode( GetDoc()->GetDocShell() ))
                bCharBound = false;

            // anchor in header/footer also not allowed.
            if( bCharBound )
                bCharBound = !GetDoc()->IsInHeaderFooter( aPos.nNode );

            if( bCharBound )
            {
                aAnch.SetType( RndStdIds::FLY_AS_CHAR );
                aAnch.SetAnchor( &aPos );
            }
        }
    }

    if( !bCharBound )
    {
        // allow native drawing objects in header/footer.
        // Thus, set <bBodyOnly> to <false> for these objects using value
        // of <nIdent> - value <0xFFFF> indicates control objects, which aren't
        // allowed in header/footer.
        //bool bBodyOnly = OBJ_NONE != nIdent;
        bool bBodyOnly = 0xFFFF == nIdent;
        bool bAtPage = false;
        const SwFrame* pPage = nullptr;
        SwCursorMoveState aState( CursorMoveState::SetOnlyText );
        Point aPoint( aPt );
        SwPosition aPos( GetDoc()->GetNodes() );
        GetLayout()->GetModelPositionForViewPoint( &aPos, aPoint, &aState );

        // do not set in ReadnOnly-content
        if (aPos.nNode.GetNode().IsProtect())
        {
            // then only page bound. Or should we
            // search the next not-readonly position?
            bAtPage = true;
        }

        SwContentNode* pCNode = aPos.nNode.GetNode().GetContentNode();
        std::pair<Point, bool> const tmp(aPoint, false);
        pAnch = pCNode ? pCNode->getLayoutFrame(GetLayout(), nullptr, &tmp) : nullptr;
        if (!pAnch)
        {
            // Hidden content. Anchor to the page instead
            bAtPage = true;
        }

        if( !bAtPage )
        {
            const SwFlyFrame *pTmp = pAnch->FindFlyFrame();
            if( pTmp )
            {
                const SwFrame* pTmpFrame = pAnch;
                SwRect aBound( rBound );
                while( pTmp )
                {
                    if( pTmp->getFrameArea().Contains( aBound ) )
                    {
                        if( !bBodyOnly || !pTmp->FindFooterOrHeader() )
                            pPage = pTmpFrame;
                        break;
                    }
                    pTmp = pTmp->GetAnchorFrame()
                                ? pTmp->GetAnchorFrame()->FindFlyFrame()
                                : nullptr;
                    pTmpFrame = pTmp;
                }
            }

            if( !pPage )
                pPage = pAnch->FindPageFrame();

            // Always via FindAnchor, to assure the frame will be bound
            // to the previous. With GetCrsOfst we can also reach the next. THIS IS WRONG.
            pAnch = ::FindAnchor( pPage, aPt, bBodyOnly );
            if (pAnch->IsTextFrame())
            {
                std::pair<SwTextNode const*, sal_Int32> const pos(
                    static_cast<SwTextFrame const*>(pAnch)->MapViewToModel(TextFrameIndex(0)));
                aPos.nNode = *pos.first;
            }
            else
            {
                aPos.nNode = *static_cast<const SwNoTextFrame*>(pAnch)->GetNode();
            }

            // do not set in ReadnOnly-content
            if( aPos.nNode.GetNode().IsProtect() )
                // then only page bound. Or should we
                // search the next not-readonly position?
                bAtPage = true;
            else
            {
                aAnch.SetType( RndStdIds::FLY_AT_PARA );
                aAnch.SetAnchor( &aPos );
            }
        }

        if( bAtPage )
        {
            pPage = pAnch ? pAnch->FindPageFrame() : GetLayout()->GetPageAtPos(aPoint);

            aAnch.SetType( RndStdIds::FLY_AT_PAGE );
            aAnch.SetPageNum( pPage->GetPhyPageNum() );
            pAnch = pPage;      // page becomes an anchor
        }
    }

    SfxItemSetFixed<RES_FRM_SIZE, RES_FRM_SIZE,
                    RES_SURROUND, RES_ANCHOR>  aSet( GetDoc()->GetAttrPool() );
    aSet.Put( aAnch );

    // OD 2004-03-30 #i26791# - determine relative object position
    SwTwips nXOffset;
    SwTwips nYOffset = rBound.Top() - pAnch->getFrameArea().Top();
    {
        if( pAnch->IsVertical() )
        {
            nXOffset = nYOffset;
            nYOffset = pAnch->getFrameArea().Left()+pAnch->getFrameArea().Width()-rBound.Right();
        }
        else if( pAnch->IsRightToLeft() )
            nXOffset = pAnch->getFrameArea().Left()+pAnch->getFrameArea().Width()-rBound.Right();
        else
            nXOffset = rBound.Left() - pAnch->getFrameArea().Left();
        if (pAnch->IsTextFrame())
        {
            const SwTextFrame* pTmp = static_cast<const SwTextFrame*>(pAnch);
            if (pTmp->IsFollow())
            {
                do {
                    pTmp = pTmp->FindMaster();
                    OSL_ENSURE(pTmp, "Where's my Master?");
                    // OD 2004-03-30 #i26791# - correction: add frame area height
                    // of master frames.
                    nYOffset += pTmp->IsVertical() ?
                                pTmp->getFrameArea().Width() : pTmp->getFrameArea().Height();
                } while (pTmp->IsFollow());
            }

            nYOffset -= pTmp->GetBaseVertOffsetForFly(false);
        }
    }

    if( OBJ_NONE == nIdent )
    {
        // For OBJ_NONE a fly is inserted.
        const tools::Long nWidth = rBound.Right()  - rBound.Left();
        const tools::Long nHeight= rBound.Bottom() - rBound.Top();
        aSet.Put( SwFormatFrameSize( SwFrameSize::Minimum, std::max( nWidth,  tools::Long(MINFLY) ),
                                              std::max( nHeight, tools::Long(MINFLY) )));

        SwFormatHoriOrient aHori( nXOffset, text::HoriOrientation::NONE, text::RelOrientation::FRAME );
        SwFormatVertOrient aVert( nYOffset, text::VertOrientation::NONE, text::RelOrientation::FRAME );
        aSet.Put( SwFormatSurround( css::text::WrapTextMode_PARALLEL ) );
        aSet.Put( aHori );
        aSet.Put( aVert );

        // Quickly store the square
        const SwRect aFlyRect( rBound );

        // Throw away generated object, now the fly can nicely
        // via the available SS be generated.
        GetDoc()->GetIDocumentUndoRedo().DoDrawUndo(false); // see above
        // #i52858# - method name changed
        SdrPage *pPg = getIDocumentDrawModelAccess().GetOrCreateDrawModel()->GetPage( 0 );
        if( !pPg )
        {
            SdrModel* pTmpSdrModel = getIDocumentDrawModelAccess().GetDrawModel();
            auto pNewPage = pTmpSdrModel->AllocPage( false );
            pTmpSdrModel->InsertPage( pNewPage.get() );
            pPg = pNewPage.get();
        }
        pPg->RecalcObjOrdNums();
        SdrObject* pRemovedObject = pPg->RemoveObject( rSdrObj.GetOrdNumDirect() );
        SdrObject::Free( pRemovedObject );
        GetDoc()->GetIDocumentUndoRedo().DoDrawUndo(true);

        SwFlyFrame* pFlyFrame;
        if( NewFlyFrame( aSet, true ) &&
            ::GetHtmlMode( GetDoc()->GetDocShell() ) &&
            nullptr != ( pFlyFrame = GetSelectedFlyFrame() ))
        {
            SfxItemSetFixed<RES_VERT_ORIENT, RES_HORI_ORIENT> aHtmlSet( GetDoc()->GetAttrPool() );
            // horizontal orientation:
            const bool bLeftFrame = aFlyRect.Left() <
                                      pAnch->getFrameArea().Left() + pAnch->getFramePrintArea().Left(),
                           bLeftPrt = aFlyRect.Left() + aFlyRect.Width() <
                                      pAnch->getFrameArea().Left() + pAnch->getFramePrintArea().Width()/2;
            if( bLeftFrame || bLeftPrt )
            {
                aHori.SetHoriOrient( text::HoriOrientation::LEFT );
                aHori.SetRelationOrient( bLeftFrame ? text::RelOrientation::FRAME : text::RelOrientation::PRINT_AREA );
            }
            else
            {
                const bool bRightFrame = aFlyRect.Left() >
                                           pAnch->getFrameArea().Left() + pAnch->getFramePrintArea().Width();
                aHori.SetHoriOrient( text::HoriOrientation::RIGHT );
                aHori.SetRelationOrient( bRightFrame ? text::RelOrientation::FRAME : text::RelOrientation::PRINT_AREA );
            }
            aHtmlSet.Put( aHori );
            aVert.SetVertOrient( text::VertOrientation::TOP );
            aVert.SetRelationOrient( text::RelOrientation::PRINT_AREA );
            aHtmlSet.Put( aVert );

            GetDoc()->SetAttr( aHtmlSet, *pFlyFrame->GetFormat() );
        }
    }
    else
    {
        if (rSdrObj.GetName().isEmpty())
        {
            bool bRestore = GetDoc()->GetIDocumentUndoRedo().DoesDrawUndo();
            GetDoc()->GetIDocumentUndoRedo().DoDrawUndo(false);
            rSdrObj.MakeNameUnique();
            GetDoc()->GetIDocumentUndoRedo().DoDrawUndo(bRestore);
        }

        aSet.Put( aAnch );
        aSet.Put( SwFormatSurround( css::text::WrapTextMode_THROUGH ) );
        // OD 2004-03-30 #i26791# - set horizontal position
        SwFormatHoriOrient aHori( nXOffset, text::HoriOrientation::NONE, text::RelOrientation::FRAME );
        aSet.Put( aHori );
        // OD 2004-03-30 #i26791# - set vertical position
        if( pAnch->IsTextFrame() && static_cast<const SwTextFrame*>(pAnch)->IsFollow() )
        {
            const SwTextFrame* pTmp = static_cast<const SwTextFrame*>(pAnch);
            do {
                pTmp = pTmp->FindMaster();
                assert(pTmp && "Where's my Master?");
                nYOffset += pTmp->IsVertical() ?
                            pTmp->getFramePrintArea().Width() : pTmp->getFramePrintArea().Height();
            } while ( pTmp->IsFollow() );
        }
        SwFormatVertOrient aVert( nYOffset, text::VertOrientation::NONE, text::RelOrientation::FRAME );
        aSet.Put( aVert );
        SwDrawFrameFormat* pFormat = static_cast<SwDrawFrameFormat*>(getIDocumentLayoutAccess().MakeLayoutFormat( RndStdIds::DRAW_OBJECT, &aSet ));
        // #i36010# - set layout direction of the position
        pFormat->SetPositionLayoutDir(
            text::PositionLayoutDir::PositionInLayoutDirOfAnchor );
        // #i44344#, #i44681# - positioning attributes already set
        pFormat->PosAttrSet();
        pFormat->SetName(rSdrObj.GetName());

        SwDrawContact *pContact = new SwDrawContact( pFormat, &rSdrObj );
        // #i35635#
        pContact->MoveObjToVisibleLayer( &rSdrObj );
        if( bCharBound )
        {
            OSL_ENSURE( aAnch.GetAnchorId() == RndStdIds::FLY_AS_CHAR, "wrong AnchorType" );
            SwTextNode *pNd = aAnch.GetContentAnchor()->nNode.GetNode().GetTextNode();
            SwFormatFlyCnt aFormat( pFormat );
            pNd->InsertItem(aFormat,
                            aAnch.GetContentAnchor()->nContent.GetIndex(), 0 );
            SwFormatVertOrient aVertical( pFormat->GetVertOrient() );
            aVertical.SetVertOrient( text::VertOrientation::LINE_CENTER );
            pFormat->SetFormatAttr( aVertical );
        }
        if( pAnch->IsTextFrame() && static_cast<const SwTextFrame*>(pAnch)->IsFollow() )
        {
            const SwTextFrame* pTmp = static_cast<const SwTextFrame*>(pAnch);
            do {
                pTmp = pTmp->FindMaster();
                OSL_ENSURE( pTmp, "Where's my Master?" );
            } while( pTmp->IsFollow() );
            pAnch = pTmp;
        }

        pContact->ConnectToLayout();

        // mark object at frame the object is inserted at.
        {
            SdrObject* pMarkObj = pContact->GetDrawObjectByAnchorFrame( *pAnch );
            if ( pMarkObj )
            {
                Imp()->GetDrawView()->MarkObj( pMarkObj, Imp()->GetPageView() );
            }
            else
            {
                Imp()->GetDrawView()->MarkObj( &rSdrObj, Imp()->GetPageView() );
            }
        }
    }

    GetDoc()->getIDocumentState().SetModified();

    KillPams();
    EndAllActionAndCall();
    UnlockPaint();
    return true;
}

void SwFEShell::BreakCreate()
{
    OSL_ENSURE( Imp()->HasDrawView(), "BreakCreate without DrawView?" );
    Imp()->GetDrawView()->BrkCreateObj();
    ::FrameNotify( this, FLY_DRAG_END );
}

bool SwFEShell::IsDrawCreate() const
{
    return Imp()->HasDrawView() && Imp()->GetDrawView()->IsCreateObj();
}

bool SwFEShell::BeginMark( const Point &rPos )
{
    if ( !Imp()->HasDrawView() )
        Imp()->MakeDrawView();

    if ( GetPageNumber( rPos ) )
    {
        SwDrawView* pDView = Imp()->GetDrawView();

        if (pDView->HasMarkablePoints())
            return pDView->BegMarkPoints( rPos );
        else
        {
            pDView->BegMarkObj( rPos );
            return true;
        }
    }
    else
        return false;
}

void SwFEShell::MoveMark( const Point &rPos )
{
    OSL_ENSURE( Imp()->HasDrawView(), "MoveMark without DrawView?" );

    if ( GetPageNumber( rPos ) )
    {
        ScrollTo( rPos );
        SwDrawView* pDView = Imp()->GetDrawView();

        if (pDView->IsInsObjPoint())
            pDView->MovInsObjPoint( rPos );
        else if (pDView->IsMarkPoints())
            pDView->MovMarkPoints( rPos );
        else
            pDView->MovAction( rPos );
    }
}

bool SwFEShell::EndMark()
{
    bool bRet = false;
    OSL_ENSURE( Imp()->HasDrawView(), "EndMark without DrawView?" );

    if (Imp()->GetDrawView()->IsMarkObj())
    {
        bRet = Imp()->GetDrawView()->EndMarkObj();

        if ( bRet )
        {
            bool bShowHdl = false;
            SwDrawView* pDView = Imp()->GetDrawView();
            // frames are not selected this way, except when
            // it is only one frame
            SdrMarkList &rMrkList = const_cast<SdrMarkList&>(pDView->GetMarkedObjectList());
            SwFlyFrame* pOldSelFly = ::GetFlyFromMarked( &rMrkList, this );

            if ( rMrkList.GetMarkCount() > 1 )
                for ( size_t i = 0; i < rMrkList.GetMarkCount(); ++i )
                {
                    SdrObject *pObj = rMrkList.GetMark( i )->GetMarkedSdrObj();
                    if( dynamic_cast<const SwVirtFlyDrawObj*>( pObj) !=  nullptr )
                    {
                        if ( !bShowHdl )
                        {
                            bShowHdl = true;
                        }
                        rMrkList.DeleteMark( i );
                        --i;    // no exceptions
                    }
                }

            if( bShowHdl )
            {
                pDView->MarkListHasChanged();
                pDView->AdjustMarkHdl();
            }

            if ( rMrkList.GetMarkCount() )
                ::lcl_GrabCursor(this, pOldSelFly);
            else
                bRet = false;
        }
        if ( bRet )
            ::FrameNotify( this, FLY_DRAG_START );
    }
    else
    {
        if (Imp()->GetDrawView()->IsMarkPoints())
            bRet = Imp()->GetDrawView()->EndMarkPoints();
    }

    SetChainMarker();
    return bRet;
}

RndStdIds SwFEShell::GetAnchorId() const
{
    RndStdIds nRet = RndStdIds(SHRT_MAX);
    if ( Imp()->HasDrawView() )
    {
        const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkedObjectList();
        for ( size_t i = 0; i < rMrkList.GetMarkCount(); ++i )
        {
            SdrObject *pObj = rMrkList.GetMark( i )->GetMarkedSdrObj();
            if ( dynamic_cast<const SwVirtFlyDrawObj*>( pObj) != nullptr )
            {
                nRet = RndStdIds::UNKNOWN;
                break;
            }
            SwDrawContact *pContact = static_cast<SwDrawContact*>(GetUserCall(pObj));
            RndStdIds nId = pContact->GetFormat()->GetAnchor().GetAnchorId();
            if ( nRet == RndStdIds(SHRT_MAX) )
                nRet = nId;
            else if ( nRet != nId )
            {
                nRet = RndStdIds::UNKNOWN;
                break;
            }
        }
    }
    if ( nRet == RndStdIds(SHRT_MAX) )
        nRet = RndStdIds::UNKNOWN;
    return nRet;
}

void SwFEShell::ChgAnchor( RndStdIds eAnchorId, bool bSameOnly, bool bPosCorr )
{
    OSL_ENSURE( Imp()->HasDrawView(), "ChgAnchor without DrawView?" );
    const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkedObjectList();
    if( rMrkList.GetMarkCount() &&
        !rMrkList.GetMark( 0 )->GetMarkedSdrObj()->getParentSdrObjectFromSdrObject() )
    {
        StartAllAction();

        if( GetDoc()->ChgAnchor( rMrkList, eAnchorId, bSameOnly, bPosCorr ))
            Imp()->GetDrawView()->UnmarkAll();

        EndAllAction();

        ::FrameNotify( this );
    }
}

void SwFEShell::DelSelectedObj()
{
    OSL_ENSURE( Imp()->HasDrawView(), "DelSelectedObj(), no DrawView available" );
    if ( Imp()->HasDrawView() )
    {
        StartAllAction();
        Imp()->GetDrawView()->DeleteMarked();
        EndAllAction();
        ::FrameNotify( this, FLY_DRAG_END );
    }
}

// For the statusline to request the current conditions
Size SwFEShell::GetObjSize() const
{
    tools::Rectangle aRect;
    if ( Imp()->HasDrawView() )
    {
        if ( Imp()->GetDrawView()->IsAction() )
            Imp()->GetDrawView()->TakeActionRect( aRect );
        else
            aRect = Imp()->GetDrawView()->GetAllMarkedRect();
    }
    return aRect.GetSize();
}

Point SwFEShell::GetAnchorObjDiff() const
{
    const SdrView *pView = Imp()->GetDrawView();
    OSL_ENSURE( pView, "GetAnchorObjDiff without DrawView?" );

    tools::Rectangle aRect;
    if ( Imp()->GetDrawView()->IsAction() )
        Imp()->GetDrawView()->TakeActionRect( aRect );
    else
        aRect = Imp()->GetDrawView()->GetAllMarkedRect();

    Point aRet( aRect.TopLeft() );

    if ( IsFrameSelected() )
    {
        SwFlyFrame *pFly = GetSelectedFlyFrame();
        aRet -= pFly->GetAnchorFrame()->getFrameArea().Pos();
    }
    else
    {
        const SdrObject *pObj = pView->GetMarkedObjectList().GetMarkCount() == 1 ?
                                pView->GetMarkedObjectList().GetMark(0)->GetMarkedSdrObj() : nullptr;
        if ( pObj )
            aRet -= pObj->GetAnchorPos();
    }

    return aRet;
}

Point SwFEShell::GetObjAbsPos() const
{
    OSL_ENSURE( Imp()->GetDrawView(), "GetObjAbsPos() without DrawView?" );
    return Imp()->GetDrawView()->GetDragStat().GetActionRect().TopLeft();
}

bool SwFEShell::IsGroupSelected()
{
    if ( IsObjSelected() )
    {
        const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkedObjectList();
        for ( size_t i = 0; i < rMrkList.GetMarkCount(); ++i )
        {
            SdrObject *pObj = rMrkList.GetMark( i )->GetMarkedSdrObj();
            // consider 'virtual' drawing objects.
            // Thus, use corresponding method instead of checking type.
            if ( pObj->IsGroupObject() &&
                 // --> #i38505# No ungroup allowed for 3d objects
                 !pObj->Is3DObj() &&
                 RndStdIds::FLY_AS_CHAR != static_cast<SwDrawContact*>(GetUserCall(pObj))->
                                      GetFormat()->GetAnchor().GetAnchorId() )
            {
                return true;
            }
        }
    }
    return false;
}

namespace
{
    bool HasSuitableGroupingAnchor(const SdrObject* pObj)
    {
        bool bSuitable = true;
        SwFrameFormat* pFrameFormat(::FindFrameFormat(const_cast<SdrObject*>(pObj)));
        if (!pFrameFormat)
        {
            OSL_FAIL( "<HasSuitableGroupingAnchor> - missing frame format" );
            bSuitable = false;
        }
        else if (RndStdIds::FLY_AS_CHAR == pFrameFormat->GetAnchor().GetAnchorId())
        {
            bSuitable = false;
        }
        return bSuitable;
    }
}

// Change return type.
// Adjustments for drawing objects in header/footer:
//      allow group, only if all selected objects are in the same header/footer
//      or not in header/footer.
bool SwFEShell::IsGroupAllowed() const
{
    bool bIsGroupAllowed = false;
    if ( IsObjSelected() > 1 )
    {
        bIsGroupAllowed = true;
        const SdrObject* pUpGroup = nullptr;
        const SwFrame* pHeaderFooterFrame = nullptr;
        const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkedObjectList();
        for ( size_t i = 0; bIsGroupAllowed && i < rMrkList.GetMarkCount(); ++i )
        {
            const SdrObject* pObj = rMrkList.GetMark( i )->GetMarkedSdrObj();
            if ( i )
                bIsGroupAllowed = pObj->getParentSdrObjectFromSdrObject() == pUpGroup;
            else
                pUpGroup = pObj->getParentSdrObjectFromSdrObject();

            if ( bIsGroupAllowed )
                bIsGroupAllowed = HasSuitableGroupingAnchor(pObj);

            // check, if all selected objects are in the
            // same header/footer or not in header/footer.
            if ( bIsGroupAllowed )
            {
                const SwFrame* pAnchorFrame = nullptr;
                if ( auto pVirtFlyDrawObj = dynamic_cast<const SwVirtFlyDrawObj*>( pObj) )
                {
                    const SwFlyFrame* pFlyFrame = pVirtFlyDrawObj->GetFlyFrame();
                    if ( pFlyFrame )
                    {
                        pAnchorFrame = pFlyFrame->GetAnchorFrame();
                    }
                }
                else
                {
                    SwDrawContact* pDrawContact = static_cast<SwDrawContact*>(GetUserCall( pObj ));
                    if ( pDrawContact )
                    {
                        pAnchorFrame = pDrawContact->GetAnchorFrame( pObj );
                    }
                }
                if ( pAnchorFrame )
                {
                    if ( i )
                    {
                        bIsGroupAllowed =
                            ( pAnchorFrame->FindFooterOrHeader() == pHeaderFooterFrame );
                    }
                    else
                    {
                        pHeaderFooterFrame = pAnchorFrame->FindFooterOrHeader();
                    }
                }
            }
        }
    }

    return bIsGroupAllowed;
}

bool SwFEShell::IsUnGroupAllowed() const
{
    bool bIsUnGroupAllowed = false;

    const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkedObjectList();
    for (size_t i = 0; i < rMrkList.GetMarkCount(); ++i)
    {
        const SdrObject* pObj = rMrkList.GetMark(i)->GetMarkedSdrObj();
        bIsUnGroupAllowed = HasSuitableGroupingAnchor(pObj);
        if (!bIsUnGroupAllowed)
            break;
    }

    return bIsUnGroupAllowed;
}

// The group gets the anchor and the contactobject of the first in the selection
void SwFEShell::GroupSelection()
{
    if ( IsGroupAllowed() )
    {
        StartAllAction();
        StartUndo( SwUndoId::START );

        GetDoc()->GroupSelection( *Imp()->GetDrawView() );

        EndUndo( SwUndoId::END );
        EndAllAction();
    }
}

// The individual objects get a copy of the anchor and the contactobject of the group
void SwFEShell::UnGroupSelection()
{
    if ( IsGroupSelected() )
    {
        StartAllAction();
        StartUndo( SwUndoId::START );

        GetDoc()->UnGroupSelection( *Imp()->GetDrawView() );

        EndUndo( SwUndoId::END );
        EndAllAction();
    }
}

void SwFEShell::MirrorSelection( bool bHorizontal )
{
    SdrView *pView = Imp()->GetDrawView();
    if ( IsObjSelected() && pView->IsMirrorAllowed() )
    {
        if ( bHorizontal )
            pView->MirrorAllMarkedHorizontal();
        else
            pView->MirrorAllMarkedVertical();
    }
}

// jump to named frame (Graphic/OLE)

bool SwFEShell::GotoFly( const OUString& rName, FlyCntType eType, bool bSelFrame )
{
    bool bRet = false;
    static SwNodeType const aChkArr[ 4 ] = {
             /* FLYCNTTYPE_ALL */   SwNodeType::NONE,
             /* FLYCNTTYPE_FRM */   SwNodeType::Text,
             /* FLYCNTTYPE_GRF */   SwNodeType::Grf,
             /* FLYCNTTYPE_OLE */   SwNodeType::Ole
            };

    const SwFlyFrameFormat* pFlyFormat = mxDoc->FindFlyByName( rName, aChkArr[ eType]);
    if( pFlyFormat )
    {
        CurrShell aCurr( this );

        SwFlyFrame* pFrame = SwIterator<SwFlyFrame,SwFormat>( *pFlyFormat ).First();
        if( pFrame )
        {
            if( bSelFrame )
            {
                // first make visible, to get a11y events in proper order
                if (!ActionPend())
                    MakeVisible( pFrame->getFrameArea() );
                SelectObj( pFrame->getFrameArea().Pos(), 0, pFrame->GetVirtDrawObj() );
            }
            else
            {
                SwContentFrame *pCFrame = pFrame->ContainsContent();
                if ( pCFrame )
                {
                    ClearMark();
                    SwPaM* pCursor = GetCursor();

                    if (pCFrame->IsTextFrame())
                    {
                        *pCursor->GetPoint() = static_cast<SwTextFrame *>(pCFrame)
                            ->MapViewToModelPos(TextFrameIndex(0));
                    }
                    else
                    {
                        assert(pCFrame->IsNoTextFrame());
                        SwContentNode *const pCNode = static_cast<SwNoTextFrame *>(pCFrame)->GetNode();

                        pCursor->GetPoint()->nNode = *pCNode;
                        pCursor->GetPoint()->nContent.Assign( pCNode, 0 );
                    }

                    SwRect& rChrRect = const_cast<SwRect&>(GetCharRect());
                    rChrRect = pFrame->getFramePrintArea();
                    rChrRect.Pos() += pFrame->getFrameArea().Pos();
                    GetCursorDocPos() = rChrRect.Pos();
                }
            }
            bRet = true;
        }
    }
    return bRet;
}

size_t SwFEShell::GetFlyCount( FlyCntType eType, bool bIgnoreTextBoxes ) const
{
    return GetDoc()->GetFlyCount(eType, bIgnoreTextBoxes);
}

const SwFrameFormat*  SwFEShell::GetFlyNum(size_t nIdx, FlyCntType eType, bool bIgnoreTextBoxes ) const
{
    return GetDoc()->GetFlyNum(nIdx, eType, bIgnoreTextBoxes);
}

std::vector<SwFrameFormat const*> SwFEShell::GetFlyFrameFormats(
        FlyCntType const eType, bool const bIgnoreTextBoxes)
{
    return GetDoc()->GetFlyFrameFormats(eType, bIgnoreTextBoxes);
}

// show the current selected object
void SwFEShell::MakeSelVisible()
{
    if ( Imp()->HasDrawView() &&
         Imp()->GetDrawView()->GetMarkedObjectList().GetMarkCount() )
    {
        GetCurrFrame(); // just to trigger formatting in case the selected object is not formatted.
        MakeVisible( SwRect(Imp()->GetDrawView()->GetAllMarkedRect()) );
    }
    else
        SwCursorShell::MakeSelVisible();
}

// how is the selected object protected?
FlyProtectFlags SwFEShell::IsSelObjProtected( FlyProtectFlags eType ) const
{
    FlyProtectFlags nChk = FlyProtectFlags::NONE;
    const bool bParent(eType & FlyProtectFlags::Parent);
    if( Imp()->HasDrawView() )
    {
        const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkedObjectList();
        for( size_t i = rMrkList.GetMarkCount(); i; )
        {
            SdrObject *pObj = rMrkList.GetMark( --i )->GetMarkedSdrObj();
            if (!pObj)
            {
                continue;
            }

            if( !bParent )
            {
                nChk |= ( pObj->IsMoveProtect() ? FlyProtectFlags::Pos : FlyProtectFlags::NONE ) |
                        ( pObj->IsResizeProtect()? FlyProtectFlags::Size : FlyProtectFlags::NONE );

                if (SwVirtFlyDrawObj* pVirtO = dynamic_cast<SwVirtFlyDrawObj*>(pObj))
                {
                    SwFlyFrame *pFly = pVirtO->GetFlyFrame();
                    if ( (FlyProtectFlags::Content & eType) && pFly->GetFormat()->GetProtect().IsContentProtected() )
                        nChk |= FlyProtectFlags::Content;

                    if ( pFly->Lower() && pFly->Lower()->IsNoTextFrame() )
                    {
                        SwOLENode *pNd = static_cast<SwNoTextFrame*>(pFly->Lower())->GetNode()->GetOLENode();
                        uno::Reference < embed::XEmbeddedObject > xObj( pNd ? pNd->GetOLEObj().GetOleRef() : nullptr );
                        if ( xObj.is() )
                        {
                            // TODO/LATER: use correct aspect
                            const bool bNeverResize = (embed::EmbedMisc::EMBED_NEVERRESIZE & xObj->getStatus( embed::Aspects::MSOLE_CONTENT ));
                            if ( ( (FlyProtectFlags::Content & eType) || (FlyProtectFlags::Size & eType) ) && bNeverResize )
                            {
                                nChk |= FlyProtectFlags::Size;
                                nChk |= FlyProtectFlags::Fixed;
                            }

                            // set FlyProtectFlags::Pos if it is a Math object anchored 'as char' and baseline alignment is activated
                            const bool bProtectMathPos = SotExchange::IsMath( xObj->getClassID() )
                                    && RndStdIds::FLY_AS_CHAR == pFly->GetFormat()->GetAnchor().GetAnchorId()
                                    && mxDoc->GetDocumentSettingManager().get( DocumentSettingId::MATH_BASELINE_ALIGNMENT );
                            if ((FlyProtectFlags::Pos & eType) && bProtectMathPos)
                                nChk |= FlyProtectFlags::Pos;
                        }
                    }
                }
                nChk &= eType;
                if( nChk == eType )
                    return eType;
            }
            const SwFrame* pAnch;
            if (SwVirtFlyDrawObj* pVirtO = dynamic_cast<SwVirtFlyDrawObj*>(pObj))
                pAnch = pVirtO->GetFlyFrame()->GetAnchorFrame();
            else
            {
                SwDrawContact* pTmp = static_cast<SwDrawContact*>(GetUserCall(pObj));
                pAnch = pTmp ? pTmp->GetAnchorFrame( pObj ) : nullptr;
            }
            if( pAnch && pAnch->IsProtected() )
                return eType;
        }
    }
    return nChk;
}

bool SwFEShell::GetObjAttr( SfxItemSet &rSet ) const
{
    if ( !IsObjSelected() )
        return false;

    const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkedObjectList();
    for ( size_t i = 0; i < rMrkList.GetMarkCount(); ++i )
    {
        SdrObject *pObj = rMrkList.GetMark( i )->GetMarkedSdrObj();
        SwDrawContact *pContact = static_cast<SwDrawContact*>(GetUserCall(pObj));
        // --> make code robust
        OSL_ENSURE( pContact, "<SwFEShell::GetObjAttr(..)> - missing <pContact>." );
        if ( pContact )
        {
            if ( i )
                rSet.MergeValues( pContact->GetFormat()->GetAttrSet() );
            else
                rSet.Put( pContact->GetFormat()->GetAttrSet() );
        }
    }
    return true;
}

void SwFEShell::SetObjAttr( const SfxItemSet& rSet )
{
    CurrShell aCurr( this );

    if ( !rSet.Count() )
    {
        OSL_ENSURE( false, "SetObjAttr, empty set." );
        return;
    }

    StartAllAction();
    StartUndo( SwUndoId::INSATTR );

    const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkedObjectList();
    for ( size_t i = 0; i < rMrkList.GetMarkCount(); ++i )
    {
        SdrObject *pObj = rMrkList.GetMark( i )->GetMarkedSdrObj();
        SwDrawContact *pContact = static_cast<SwDrawContact*>(GetUserCall(pObj));
        GetDoc()->SetAttr( rSet, *pContact->GetFormat() );
    }

    EndUndo( SwUndoId::INSATTR );
    EndAllActionAndCall();
    GetDoc()->getIDocumentState().SetModified();
}

bool SwFEShell::IsAlignPossible() const
{
    return Imp()->GetDrawView()->IsAlignPossible();
}

void SwFEShell::CheckUnboundObjects()
{
    CurrShell aCurr( this );

    const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkedObjectList();
    for ( size_t i = 0; i < rMrkList.GetMarkCount(); ++i )
    {
        SdrObject *pObj = rMrkList.GetMark( i )->GetMarkedSdrObj();
        if ( !GetUserCall(pObj) )
        {
            const tools::Rectangle &rBound = pObj->GetSnapRect();
            const Point aPt( rBound.TopLeft() );
            const SwFrame *pPage = GetLayout()->Lower();
            const SwFrame *pLast = pPage;
            while ( pPage && !pPage->getFrameArea().Contains( aPt ) )
            {
                if ( aPt.Y() > pPage->getFrameArea().Bottom() )
                    pLast = pPage;
                pPage = pPage->GetNext();
            }
            if ( !pPage )
                pPage = pLast;
            OSL_ENSURE( pPage, "Page not found." );

            SwFormatAnchor aAnch;
            {
            const SwContentFrame *const pAnch = ::FindAnchor(pPage, aPt, true);
            SwPosition aPos( pAnch->IsTextFrame()
                ? *static_cast<SwTextFrame const*>(pAnch)->GetTextNodeForParaProps()
                : *static_cast<SwNoTextFrame const*>(pAnch)->GetNode() );
            aAnch.SetType( RndStdIds::FLY_AT_PARA );
            aAnch.SetAnchor( &aPos );
            const_cast<SwRect&>(GetCharRect()).Pos() = aPt;
            }

            // First the action here, to assure GetCharRect delivers current values.
            StartAllAction();

            SfxItemSetFixed<RES_FRM_SIZE, RES_FRM_SIZE,
                            RES_SURROUND, RES_ANCHOR>  aSet( GetAttrPool() );
            aSet.Put( aAnch );
            aSet.Put( SwFormatSurround( css::text::WrapTextMode_THROUGH ) );
            SwFrameFormat* pFormat = getIDocumentLayoutAccess().MakeLayoutFormat( RndStdIds::DRAW_OBJECT, &aSet );

            SwDrawContact *pContact = new SwDrawContact(
                                            static_cast<SwDrawFrameFormat*>(pFormat), pObj );

            // #i35635#
            pContact->MoveObjToVisibleLayer( pObj );
            pContact->ConnectToLayout();

            EndAllAction();
        }
    }
}

void SwFEShell::SetCalcFieldValueHdl(Outliner* pOutliner)
{
    GetDoc()->SetCalcFieldValueHdl(pOutliner);
}

SwChainRet SwFEShell::Chainable( SwRect &rRect, const SwFrameFormat &rSource,
                            const Point &rPt ) const
{
    rRect.Clear();

    // The source is not allowed to have a follow.
    const SwFormatChain &rChain = rSource.GetChain();
    if ( rChain.GetNext() )
        return SwChainRet::SOURCE_CHAINED;

    SwChainRet nRet = SwChainRet::NOT_FOUND;
    if( Imp()->HasDrawView() )
    {
        SdrPageView* pPView;
        SwDrawView *pDView = const_cast<SwDrawView*>(Imp()->GetDrawView());
        const auto nOld = pDView->GetHitTolerancePixel();
        pDView->SetHitTolerancePixel( 0 );
        SdrObject* pObj = pDView->PickObj(rPt, pDView->getHitTolLog(), pPView, SdrSearchOptions::PICKMARKABLE);
        SwVirtFlyDrawObj* pDrawObj = dynamic_cast<SwVirtFlyDrawObj*>(pObj);
        if (pDrawObj)
        {
            SwFlyFrame *pFly = pDrawObj->GetFlyFrame();
            rRect = pFly->getFrameArea();

            // Target and source should not be equal and the list
            // should not be cyclic
            SwFrameFormat *pFormat = pFly->GetFormat();
            nRet = GetDoc()->Chainable(rSource, *pFormat);
        }
        pDView->SetHitTolerancePixel( nOld );
    }
    return nRet;
}

void SwFEShell::Chain( SwFrameFormat &rSource, const SwFrameFormat &rDest )
{
    GetDoc()->Chain(rSource, rDest);
}

SwChainRet SwFEShell::Chain( SwFrameFormat &rSource, const Point &rPt )
{
    SwRect aDummy;
    SwChainRet nErr = Chainable( aDummy, rSource, rPt );
    if ( nErr == SwChainRet::OK )
    {
        StartAllAction();
        SdrPageView* pPView;
        SwDrawView *pDView = Imp()->GetDrawView();
        const auto nOld = pDView->GetHitTolerancePixel();
        pDView->SetHitTolerancePixel( 0 );
        SdrObject* pObj = pDView->PickObj(rPt, pDView->getHitTolLog(), pPView, SdrSearchOptions::PICKMARKABLE);
        pDView->SetHitTolerancePixel( nOld );
        SwFlyFrame *pFly = static_cast<SwVirtFlyDrawObj*>(pObj)->GetFlyFrame();

        SwFlyFrameFormat *pFormat = pFly->GetFormat();
        GetDoc()->Chain(rSource, *pFormat);
        EndAllAction();
        SetChainMarker();
    }
    return nErr;
}

void SwFEShell::Unchain( SwFrameFormat &rFormat )
{
    StartAllAction();
    GetDoc()->Unchain(rFormat);
    EndAllAction();
}

void SwFEShell::HideChainMarker()
{
    m_pChainFrom.reset();
    m_pChainTo.reset();
}

void SwFEShell::SetChainMarker()
{
    bool bDelFrom = true,
         bDelTo   = true;
    if ( IsFrameSelected() )
    {
        SwFlyFrame *pFly = GetSelectedFlyFrame();

        if ( pFly->GetPrevLink() )
        {
            bDelFrom = false;
            const SwFrame *pPre = pFly->GetPrevLink();

            Point aStart( pPre->getFrameArea().Right(), pPre->getFrameArea().Bottom());
            Point aEnd(pFly->getFrameArea().Pos());

            if (!m_pChainFrom)
            {
                m_pChainFrom.reset(
                    new SdrDropMarkerOverlay( *GetDrawView(), aStart, aEnd ));
            }
        }
        if ( pFly->GetNextLink() )
        {
            bDelTo = false;
            const SwFlyFrame *pNxt = pFly->GetNextLink();

            Point aStart( pFly->getFrameArea().Right(), pFly->getFrameArea().Bottom());
            Point aEnd(pNxt->getFrameArea().Pos());

            if (!m_pChainTo)
            {
                m_pChainTo.reset(
                    new SdrDropMarkerOverlay( *GetDrawView(), aStart, aEnd ));
            }
        }
    }

    if ( bDelFrom )
    {
        m_pChainFrom.reset();
    }

    if ( bDelTo )
    {
        m_pChainTo.reset();
    }
}

tools::Long SwFEShell::GetSectionWidth( SwFormat const & rFormat ) const
{
    SwFrame *pFrame = GetCurrFrame();
    // Is the cursor at this moment in a SectionFrame?
    if( pFrame && pFrame->IsInSct() )
    {
        SwSectionFrame* pSect = pFrame->FindSctFrame();
        do
        {
            // Is it the right one?
            if( pSect->KnowsFormat( rFormat ) )
                return pSect->getFrameArea().Width();
            // for nested areas
            pSect = pSect->GetUpper()->FindSctFrame();
        }
        while( pSect );
    }
    SwIterator<SwSectionFrame,SwFormat> aIter( rFormat );
    for ( SwSectionFrame* pSct = aIter.First(); pSct; pSct = aIter.Next() )
    {
        if( !pSct->IsFollow() )
        {
            return pSct->getFrameArea().Width();
        }
    }
    return 0;
}

void SwFEShell::CreateDefaultShape( SdrObjKind eSdrObjectKind, const tools::Rectangle& rRect,
                sal_uInt16 nSlotId)
{
    SdrView* pDrawView = GetDrawView();
    SdrModel* pDrawModel = pDrawView->GetModel();
    SdrObject* pObj = SdrObjFactory::MakeNewObject(
        *pDrawModel,
        SdrInventor::Default,
        eSdrObjectKind);

    if(pObj)
    {
        tools::Rectangle aRect(rRect);
        if(OBJ_CARC == eSdrObjectKind || OBJ_CCUT == eSdrObjectKind)
        {
            // force quadratic
            if(aRect.GetWidth() > aRect.GetHeight())
            {
                aRect = tools::Rectangle(
                    Point(aRect.Left() + ((aRect.GetWidth() - aRect.GetHeight()) / 2), aRect.Top()),
                    Size(aRect.GetHeight(), aRect.GetHeight()));
            }
            else
            {
                aRect = tools::Rectangle(
                    Point(aRect.Left(), aRect.Top() + ((aRect.GetHeight() - aRect.GetWidth()) / 2)),
                    Size(aRect.GetWidth(), aRect.GetWidth()));
            }
        }
        pObj->SetLogicRect(aRect);

        Point aStart = aRect.TopLeft();
        Point aEnd = aRect.BottomRight();

        if(dynamic_cast<const SdrCircObj*>( pObj) !=  nullptr)
        {
            SfxItemSet aAttr(pDrawModel->GetItemPool());
            aAttr.Put(makeSdrCircStartAngleItem(9000_deg100));
            aAttr.Put(makeSdrCircEndAngleItem(0_deg100));
            pObj->SetMergedItemSet(aAttr);
        }
        else if(auto pPathObj = dynamic_cast<SdrPathObj*>( pObj))
        {
            basegfx::B2DPolyPolygon aPoly;

            switch(eSdrObjectKind)
            {
                case OBJ_PATHLINE:
                case OBJ_PATHFILL:
                {
                    basegfx::B2DPolygon aInnerPoly;

                    aInnerPoly.append(basegfx::B2DPoint(aRect.Left(), aRect.Bottom()));

                    const basegfx::B2DPoint aCenterBottom(aRect.Center().getX(), aRect.Bottom());
                    aInnerPoly.appendBezierSegment(
                        aCenterBottom,
                        aCenterBottom,
                        basegfx::B2DPoint(aRect.Center().getX(), aRect.Center().getY()));

                    const basegfx::B2DPoint aCenterTop(aRect.Center().getX(), aRect.Top());
                    aInnerPoly.appendBezierSegment(
                        aCenterTop,
                        aCenterTop,
                        basegfx::B2DPoint(aRect.Right(), aRect.Top()));

                    aInnerPoly.setClosed(true);
                    aPoly.append(aInnerPoly);
                }
                break;
                case OBJ_FREELINE:
                case OBJ_FREEFILL:
                {
                    basegfx::B2DPolygon aInnerPoly;

                    aInnerPoly.append(basegfx::B2DPoint(aRect.Left(), aRect.Bottom()));

                    aInnerPoly.appendBezierSegment(
                        basegfx::B2DPoint(aRect.Left(), aRect.Top()),
                        basegfx::B2DPoint(aRect.Center().getX(), aRect.Top()),
                        basegfx::B2DPoint(aRect.Center().getX(), aRect.Center().getY()));

                    aInnerPoly.appendBezierSegment(
                        basegfx::B2DPoint(aRect.Center().getX(), aRect.Bottom()),
                        basegfx::B2DPoint(aRect.Right(), aRect.Bottom()),
                        basegfx::B2DPoint(aRect.Right(), aRect.Top()));

                    aInnerPoly.append(basegfx::B2DPoint(aRect.Right(), aRect.Bottom()));
                    aInnerPoly.setClosed(true);
                    aPoly.append(aInnerPoly);
                }
                break;
                case OBJ_POLY:
                case OBJ_PLIN:
                {
                    basegfx::B2DPolygon aInnerPoly;
                    sal_Int32 nWdt(aRect.GetWidth());
                    sal_Int32 nHgt(aRect.GetHeight());

                    aInnerPoly.append(basegfx::B2DPoint(aRect.Left(), aRect.Bottom()));
                    aInnerPoly.append(basegfx::B2DPoint(aRect.Left() + (nWdt * 30) / 100, aRect.Top() + (nHgt * 70) / 100));
                    aInnerPoly.append(basegfx::B2DPoint(aRect.Left(), aRect.Top() + (nHgt * 15) / 100));
                    aInnerPoly.append(basegfx::B2DPoint(aRect.Left() + (nWdt * 65) / 100, aRect.Top()));
                    aInnerPoly.append(basegfx::B2DPoint(aRect.Left() + nWdt, aRect.Top() + (nHgt * 30) / 100));
                    aInnerPoly.append(basegfx::B2DPoint(aRect.Left() + (nWdt * 80) / 100, aRect.Top() + (nHgt * 50) / 100));
                    aInnerPoly.append(basegfx::B2DPoint(aRect.Left() + (nWdt * 80) / 100, aRect.Top() + (nHgt * 75) / 100));
                    aInnerPoly.append(basegfx::B2DPoint(aRect.Bottom(), aRect.Right()));

                    if(OBJ_PLIN == eSdrObjectKind)
                    {
                        aInnerPoly.append(basegfx::B2DPoint(aRect.Center().getX(), aRect.Bottom()));
                    }
                    else
                    {
                        aInnerPoly.setClosed(true);
                    }

                    aPoly.append(aInnerPoly);
                }
                break;
                case OBJ_LINE :
                {
                    sal_Int32 nYMiddle((aRect.Top() + aRect.Bottom()) / 2);
                    basegfx::B2DPolygon aTempPoly;
                    aTempPoly.append(basegfx::B2DPoint(aRect.TopLeft().getX(), nYMiddle));
                    aTempPoly.append(basegfx::B2DPoint(aRect.BottomRight().getX(), nYMiddle));
                    aPoly.append(aTempPoly);

                    SfxItemSet aAttr(pObj->getSdrModelFromSdrObject().GetItemPool());
                    SetLineEnds(aAttr, *pObj, nSlotId);
                    pObj->SetMergedItemSet(aAttr);
                }
                break;
                default:
                break;
            }

            pPathObj->SetPathPoly(aPoly);
        }
        else if(auto pMeasureObj = dynamic_cast<SdrMeasureObj*>( pObj))
        {
            sal_Int32 nYMiddle((aRect.Top() + aRect.Bottom()) / 2);
            pMeasureObj->SetPoint(Point(aStart.X(), nYMiddle), 0);
            pMeasureObj->SetPoint(Point(aEnd.X(), nYMiddle), 1);

            SfxItemSet aAttr(pObj->getSdrModelFromSdrObject().GetItemPool());
            SetLineEnds(aAttr, *pObj, nSlotId);
            pObj->SetMergedItemSet(aAttr);
        }
        else if(auto pCaptionObj = dynamic_cast<SdrCaptionObj*>( pObj))
        {
            bool bVerticalText = ( SID_DRAW_TEXT_VERTICAL == nSlotId ||
                                            SID_DRAW_CAPTION_VERTICAL == nSlotId );
            pCaptionObj->SetVerticalWriting(bVerticalText);
            if(bVerticalText)
            {
                SfxItemSet aSet(pObj->GetMergedItemSet());
                aSet.Put(SdrTextVertAdjustItem(SDRTEXTVERTADJUST_CENTER));
                aSet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_RIGHT));
                pObj->SetMergedItemSet(aSet);
            }

            pCaptionObj->SetLogicRect(aRect);
            pCaptionObj->SetTailPos(
                aRect.TopLeft() - Point(aRect.GetWidth() / 2, aRect.GetHeight() / 2));
        }
        else if(auto pText = dynamic_cast<SdrTextObj*>( pObj))
        {
            pText->SetLogicRect(aRect);

            bool bVertical = (SID_DRAW_TEXT_VERTICAL == nSlotId);
            bool bMarquee = (SID_DRAW_TEXT_MARQUEE == nSlotId);

            pText->SetVerticalWriting(bVertical);

            if(bVertical)
            {
                SfxItemSet aSet(pDrawModel->GetItemPool());
                aSet.Put(makeSdrTextAutoGrowWidthItem(true));
                aSet.Put(makeSdrTextAutoGrowHeightItem(false));
                aSet.Put(SdrTextVertAdjustItem(SDRTEXTVERTADJUST_TOP));
                aSet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_RIGHT));
                pText->SetMergedItemSet(aSet);
            }

            if(bMarquee)
            {
                SfxItemSetFixed<SDRATTR_MISC_FIRST, SDRATTR_MISC_LAST> aSet(pDrawModel->GetItemPool());
                aSet.Put( makeSdrTextAutoGrowWidthItem( false ) );
                aSet.Put( makeSdrTextAutoGrowHeightItem( false ) );
                aSet.Put( SdrTextAniKindItem( SdrTextAniKind::Slide ) );
                aSet.Put( SdrTextAniDirectionItem( SdrTextAniDirection::Left ) );
                aSet.Put( SdrTextAniCountItem( 1 ) );
                aSet.Put( SdrTextAniAmountItem( static_cast<sal_Int16>(GetWin()->PixelToLogic(Size(2,1)).Width())) );
                pObj->SetMergedItemSetAndBroadcast(aSet);
            }
        }
        SdrPageView* pPageView = pDrawView->GetSdrPageView();
        SdrCreateView::SetupObjLayer(pPageView, pDrawView->GetActiveLayer(), pObj);
        // switch undo off or this combined with ImpEndCreate will cause two undos
        // see comment made in SwFEShell::EndCreate (we create our own undo-object!)
        const bool bUndo(GetDoc()->GetIDocumentUndoRedo().DoesUndo());
        GetDoc()->GetIDocumentUndoRedo().DoUndo(false);
        pDrawView->InsertObjectAtView(pObj, *pPageView);
        GetDoc()->GetIDocumentUndoRedo().DoUndo(bUndo);
    }
    ImpEndCreate();
}

/** SwFEShell::GetShapeBackground
    method determines background color of the page the selected drawing
    object is on and returns this color.
    If no color is found, because no drawing object is selected or ...,
    color COL_BLACK (default color on constructing object of class Color)
    is returned.

    @returns an object of class Color
*/
Color SwFEShell::GetShapeBackground() const
{
    Color aRetColor;

    // check, if a draw view exists
    OSL_ENSURE( Imp()->GetDrawView(), "wrong usage of SwFEShell::GetShapeBackground - no draw view!");
    if( Imp()->GetDrawView() )
    {
        // determine list of selected objects
        const SdrMarkList* pMrkList = &Imp()->GetDrawView()->GetMarkedObjectList();
        // check, if exactly one object is selected.
        OSL_ENSURE( pMrkList->GetMarkCount() == 1, "wrong usage of SwFEShell::GetShapeBackground - no selected object!");
        if ( pMrkList->GetMarkCount() == 1)
        {
            // get selected object
            const SdrObject *pSdrObj = pMrkList->GetMark( 0 )->GetMarkedSdrObj();
            // check, if selected object is a shape (drawing object)
            OSL_ENSURE( dynamic_cast<const SwVirtFlyDrawObj*>( pSdrObj) ==  nullptr, "wrong usage of SwFEShell::GetShapeBackground - selected object is not a drawing object!");
            if ( dynamic_cast<const SwVirtFlyDrawObj*>( pSdrObj) ==  nullptr )
            {
                // determine page frame of the frame the shape is anchored.
                const SwFrame* pAnchorFrame =
                        static_cast<SwDrawContact*>(GetUserCall(pSdrObj))->GetAnchorFrame( pSdrObj );
                OSL_ENSURE( pAnchorFrame, "inconsistent model - no anchor at shape!");
                if ( pAnchorFrame )
                {
                    const SwPageFrame* pPageFrame = pAnchorFrame->FindPageFrame();
                    OSL_ENSURE( pPageFrame, "inconsistent model - no page!");
                    if ( pPageFrame )
                    {
                        aRetColor = pPageFrame->GetDrawBackgroundColor();
                    }
                }
            }
        }
    }

    return aRetColor;
}

/** Is default horizontal text direction for selected drawing object right-to-left
    Because drawing objects only painted for each page only, the default
    horizontal text direction of a drawing object is given by the corresponding
    page property.

    @returns boolean, indicating, if the horizontal text direction of the
    page, the selected drawing object is on, is right-to-left.
*/
bool SwFEShell::IsShapeDefaultHoriTextDirR2L() const
{
    bool bRet = false;

    // check, if a draw view exists
    OSL_ENSURE( Imp()->GetDrawView(), "wrong usage of SwFEShell::GetShapeBackground - no draw view!");
    if( Imp()->GetDrawView() )
    {
        // determine list of selected objects
        const SdrMarkList* pMrkList = &Imp()->GetDrawView()->GetMarkedObjectList();
        // check, if exactly one object is selected.
        OSL_ENSURE( pMrkList->GetMarkCount() == 1, "wrong usage of SwFEShell::GetShapeBackground - no selected object!");
        if ( pMrkList->GetMarkCount() == 1)
        {
            // get selected object
            const SdrObject *pSdrObj = pMrkList->GetMark( 0 )->GetMarkedSdrObj();
            // check, if selected object is a shape (drawing object)
            OSL_ENSURE( dynamic_cast<const SwVirtFlyDrawObj*>( pSdrObj) ==  nullptr, "wrong usage of SwFEShell::GetShapeBackground - selected object is not a drawing object!");
            if ( dynamic_cast<const SwVirtFlyDrawObj*>( pSdrObj) ==  nullptr )
            {
                // determine page frame of the frame the shape is anchored.
                const SwFrame* pAnchorFrame =
                        static_cast<SwDrawContact*>(GetUserCall(pSdrObj))->GetAnchorFrame( pSdrObj );
                OSL_ENSURE( pAnchorFrame, "inconsistent model - no anchor at shape!");
                if ( pAnchorFrame )
                {
                    const SwPageFrame* pPageFrame = pAnchorFrame->FindPageFrame();
                    OSL_ENSURE( pPageFrame, "inconsistent model - no page!");
                    if ( pPageFrame )
                    {
                        bRet = pPageFrame->IsRightToLeft();
                    }
                }
            }
        }
    }

    return bRet;
}

Point SwFEShell::GetRelativePagePosition(const Point& rDocPos)
{
    Point aRet(-1, -1);
    const SwFrame *pPage = GetLayout()->Lower();
    while ( pPage && !pPage->getFrameArea().Contains( rDocPos ) )
    {
        pPage = pPage->GetNext();
    }
    if(pPage)
    {
        aRet = rDocPos - pPage->getFrameArea().TopLeft();
    }
    return aRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
