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

#include "hintids.hxx"
#include <svx/sdrobjectfilter.hxx>
#include <svx/svddrgmt.hxx>
#include <svx/svditer.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdouno.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdocirc.hxx>
#include <svx/svdopath.hxx>
#include <svx/sxciaitm.hxx>
#include <svx/xfillit.hxx>
#include <svx/svdocapt.hxx>
#include <sfx2/app.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/opaqitem.hxx>
#include <editeng/protitem.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdpagv.hxx>
#include <IDocumentSettingAccess.hxx>
#include <DocumentSettingManager.hxx>
#include <IDocumentState.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <cmdid.h>
#include <drawdoc.hxx>
#include <textboxhelper.hxx>
#include <poolfmt.hrc>
#include <frmfmt.hxx>
#include <frmatr.hxx>
#include <fmtfsize.hxx>
#include <fmtanchr.hxx>
#include <fmtornt.hxx>
#include <fmtsrnd.hxx>
#include <fmtcntnt.hxx>
#include <fmtflcnt.hxx>
#include <fmtcnct.hxx>
#include <docary.hxx>
#include <tblsel.hxx>
#include <swtable.hxx>
#include <flyfrms.hxx>
#include "fesh.hxx"
#include "rootfrm.hxx"
#include "pagefrm.hxx"
#include "sectfrm.hxx"
#include "doc.hxx"
#include <IDocumentUndoRedo.hxx>
#include "dview.hxx"
#include "dflyobj.hxx"
#include "dcontact.hxx"
#include "viewimp.hxx"
#include "flyfrm.hxx"
#include "pam.hxx"
#include "ndole.hxx"
#include "ndgrf.hxx"
#include "ndtxt.hxx"
#include "viewopt.hxx"
#include "swundo.hxx"
#include "notxtfrm.hxx"
#include "txtfrm.hxx"
#include "txatbase.hxx"
#include "mdiexp.hxx"
#include <sortedobjs.hxx>
#include <HandleAnchorNodeChg.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <calbck.hxx>

#include <com/sun/star/embed/EmbedMisc.hpp>
#include <com/sun/star/embed/Aspects.hpp>

#define SCROLLVAL 75

using namespace com::sun::star;

SwFlyFrame *GetFlyFromMarked( const SdrMarkList *pLst, SwViewShell *pSh )
{
    if ( !pLst )
        pLst = pSh->HasDrawView() ? &pSh->Imp()->GetDrawView()->GetMarkedObjectList():nullptr;

    if ( pLst && pLst->GetMarkCount() == 1 )
    {
        SdrObject *pO = pLst->GetMark( 0 )->GetMarkedSdrObj();
        if ( pO && dynamic_cast<const SwVirtFlyDrawObj*>( pO) !=  nullptr )
            return static_cast<SwVirtFlyDrawObj*>(pO)->GetFlyFrame();
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
extern bool g_bNoInterrupt;       // in swmodule.cxx
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
    SET_CURR_SHELL( this );
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
                    Point aPt( pOldSelFly->Frame().Pos() );
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
                    GetWin()->Invalidate( pOldSelFly->Frame().SVRect() );
                }

                // Cancel crop mode
                if ( SDRDRAG_CROP == GetDragMode() )
                    SetDragMode( SDRDRAG_MOVE );

                bUnmark = true;
            }
        }
        if ( bUnmark )
            pDView->UnmarkAll();
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

    if (!(nFlag & SW_ALLOW_TEXTBOX))
    {
        // If the fly frame is a textbox of a shape, then select the shape instead.
        std::map<SwFrameFormat*, SwFrameFormat*> aTextBoxShapes = SwTextBoxHelper::findShapes(mpDoc);
        for (size_t i = 0; i < rMrkList.GetMarkCount(); ++i)
        {
            SdrObject* pObject = rMrkList.GetMark(i)->GetMarkedSdrObj();
            SwContact* pDrawContact = static_cast<SwContact*>(GetUserCall(pObject));
            SwFrameFormat* pFormat = pDrawContact->GetFormat();
            if (aTextBoxShapes.find(pFormat) != aTextBoxShapes.end())
            {
                SdrObject* pShape = aTextBoxShapes[pFormat]->FindSdrObject();
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

    // update der Statuszeile
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

#define LESS_X( aPt1, aPt2, bOld ) ( aPt1.getX() < aPt2.getX() || \
        ( aPt1.getX() == aPt2.getX() && ( aPt1.getY() < aPt2.getY() || \
        ( aPt1.getY() == aPt2.getY() && bOld ) ) ) )
#define LESS_Y( aPt1, aPt2, bOld ) ( aPt1.getY() < aPt2.getY() || \
        ( aPt1.getY() == aPt2.getY() && ( aPt1.getX() < aPt2.getX() || \
        ( aPt1.getX() == aPt2.getX() && bOld ) ) ) )

bool SwFEShell::MoveAnchor( SwMove nDir )
{
    const SdrMarkList* pMrkList;
    if( !Imp()->GetDrawView() ||
        nullptr == (pMrkList = &Imp()->GetDrawView()->GetMarkedObjectList()) ||
        1 != pMrkList->GetMarkCount())
        return false;
    SwFrame* pOld;
    SwFlyFrame* pFly = nullptr;
    SdrObject *pObj = pMrkList->GetMark( 0 )->GetMarkedSdrObj();
    if( dynamic_cast<const SwVirtFlyDrawObj*>( pObj) !=  nullptr )
    {
        pFly = static_cast<SwVirtFlyDrawObj*>(pObj)->GetFlyFrame();
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
        if ( FLY_AS_CHAR == nAnchorId )
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
            case FLY_AT_PAGE:
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
            case FLY_AT_CHAR:
            {
                OSL_ENSURE( pOld->IsContentFrame(), "Wrong anchor, page expected." );
                if( SwMove::LEFT == nDir || SwMove::RIGHT == nDir )
                {
                    SwPosition pos = *aAnch.GetContentAnchor();
                    SwTextNode* pTextNd = static_cast<SwTextFrame*>(pOld)->GetTextNode();
                    const sal_Int32 nAct = pos.nContent.GetIndex();
                    if( SwMove::LEFT == nDir )
                    {
                        bRet = true;
                        if( nAct )
                        {
                            pos.nContent.Assign( pTextNd, nAct-1 );
                        }
                        else
                            nDir = SwMove::UP;
                    }
                    else
                    {
                        const sal_Int32 nMax =
                            static_cast<SwTextFrame*>(pOld)->GetTextNode()->GetText().getLength();
                        if( nAct < nMax )
                        {
                            bRet = true;
                            pos.nContent.Assign( pTextNd, nAct+1 );
                        }
                        else
                            nDir = SwMove::DOWN;
                    }
                    if( pos != *aAnch.GetContentAnchor())
                        aAnch.SetAnchor( &pos );
                }
                SAL_FALLTHROUGH;
            }
            case FLY_AT_PARA:
            {
                OSL_ENSURE( pOld->IsContentFrame(), "Wrong anchor, page expected." );
                if( SwMove::UP == nDir )
                    pNew = pOld->FindPrev();
                else if( SwMove::DOWN == nDir )
                    pNew = pOld->FindNext();
                if( pNew && pNew != pOld && pNew->IsContentFrame() )
                {
                    SwPosition pos = *aAnch.GetContentAnchor();
                    SwTextNode* pTextNd = static_cast<SwTextFrame*>(pNew)->GetTextNode();
                    pos.nNode = *pTextNd;
                    sal_Int32 nTmp = 0;
                    if( bRet )
                    {
                        nTmp = static_cast<SwTextFrame*>(pNew)->GetTextNode()->GetText().getLength();
                        if( nTmp )
                            --nTmp;
                    }
                    pos.nContent.Assign( pTextNd, nTmp );
                    aAnch.SetAnchor( &pos );
                    bRet = true;
                }
                else if( SwMove::UP == nDir || SwMove::DOWN == nDir )
                    bRet = false;
                break;
            }
            case FLY_AT_FLY:
            {
                OSL_ENSURE( pOld->IsFlyFrame(), "Wrong anchor, fly frame expected.");
                SwPageFrame* pPage = pOld->FindPageFrame();
                OSL_ENSURE( pPage, "Where's my page?" );
                SwFlyFrame* pNewFly = nullptr;
                if( pPage->GetSortedObjs() )
                {
                    bool bOld = false;
                    Point aCenter( pOld->Frame().Left() + pOld->Frame().Width()/2,
                                   pOld->Frame().Top() + pOld->Frame().Height()/2 );
                    Point aBest;
                    for( size_t i = 0; i<pPage->GetSortedObjs()->size(); ++i )
                    {
                        SwAnchoredObject* pAnchObj = (*pPage->GetSortedObjs())[i];
                        if( dynamic_cast<const SwFlyFrame*>( pAnchObj) !=  nullptr )
                        {
                            SwFlyFrame* pTmp = static_cast<SwFlyFrame*>(pAnchObj);
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
                                Point aNew( pTmp->Frame().Left() +
                                            pTmp->Frame().Width()/2,
                                            pTmp->Frame().Top() +
                                            pTmp->Frame().Height()/2 );
                                bool bAccept = false;
                                switch( nDir ) {
                                    case SwMove::RIGHT:
                                    {
                                        bAccept = LESS_X( aCenter, aNew, bOld )
                                             && ( !pNewFly ||
                                             LESS_X( aNew, aBest, false ) );
                                        break;
                                    }
                                    case SwMove::LEFT:
                                    {
                                        bAccept = LESS_X( aNew, aCenter, !bOld )
                                             && ( !pNewFly ||
                                             LESS_X( aBest, aNew, true ) );
                                        break;
                                    }
                                    case SwMove::UP:
                                    {
                                        bAccept = LESS_Y( aNew, aCenter, !bOld )
                                             && ( !pNewFly ||
                                             LESS_Y( aBest, aNew, true ) );
                                        break;
                                    }
                                    case SwMove::DOWN:
                                    {
                                        bAccept = LESS_Y( aCenter, aNew, bOld )
                                             && ( !pNewFly ||
                                             LESS_Y( aNew, aBest, false ) );
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
                SwHandleAnchorNodeChg* pHandleAnchorNodeChg( nullptr );
                SwFlyFrameFormat* pFlyFrameFormat( dynamic_cast<SwFlyFrameFormat*>(&rFormat) );
                if ( pFlyFrameFormat )
                {
                    pHandleAnchorNodeChg =
                        new SwHandleAnchorNodeChg( *pFlyFrameFormat, aAnch );
                }
                rFormat.GetDoc()->SetAttr( aAnch, rFormat );
                delete pHandleAnchorNodeChg;
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

const SdrMarkList* SwFEShell::_GetMarkList() const
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
    const SdrMarkList* pMarkList = _GetMarkList();
    if( pMarkList == nullptr  ||  pMarkList->GetMarkCount() == 0 )
        eType = FrameTypeFlags::NONE;
    else
    {
        // obtain marked item as fly frame; if no fly frame, it must
        // be a draw object
        const SwFlyFrame* pFly = ::GetFlyFromMarked(pMarkList, const_cast<SwViewShell*>(static_cast<SwViewShell const *>(this)));
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
    const SdrMarkList* pMarkList = _GetMarkList();
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

void SwFEShell::SetDragMode( sal_uInt16 eDragMode )
{
    if ( Imp()->HasDrawView() )
        Imp()->GetDrawView()->SetDragMode( (SdrDragMode)eDragMode );
}

SdrDragMode SwFEShell::GetDragMode() const
{
    SdrDragMode nRet = (SdrDragMode)0;
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
    pView->SetEditMode( SDREDITMODE_EDIT );
    SetDragMode( SDRDRAG_CROP );
}

long SwFEShell::BeginDrag( const Point* pPt, bool bIsShift)
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
        return 1;
    }
    return 0;
}

long SwFEShell::Drag( const Point *pPt, bool )
{
    OSL_ENSURE( Imp()->HasDrawView(), "Drag without DrawView?" );
    if ( Imp()->GetDrawView()->IsDragObj() )
    {
        ScrollTo( *pPt );
        Imp()->GetDrawView()->MovDragObj( *pPt );
        Imp()->GetDrawView()->ShowDragAnchor();
        ::FrameNotify( this );
        return 1;
    }
    return 0;
}

long SwFEShell::EndDrag( const Point *, bool )
{
    OSL_ENSURE( Imp()->HasDrawView(), "EndDrag without DrawView?" );
    SdrView *pView = Imp()->GetDrawView();
    if ( pView->IsDragObj() )
    {
        for(SwViewShell& rSh : GetRingContainer())
            rSh.StartAction();

        StartUndo( UNDO_START );

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
        ChgAnchor( 0, true );

        EndUndo( UNDO_END );

        for(SwViewShell& rSh : GetRingContainer())
        {
            rSh.EndAction();
            if( dynamic_cast<const SwCursorShell *>(&rSh) != nullptr )
                static_cast<SwCursorShell*>(&rSh)->CallChgLnk();
        }

        GetDoc()->getIDocumentState().SetModified();
        ::FrameNotify( this );

        return 1;
    }
    return 0;
}

void SwFEShell::BreakDrag()
{
    OSL_ENSURE( Imp()->HasDrawView(), "BreakDrag without DrawView?" );
    if ( Imp()->GetDrawView()->IsDragObj() )
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
                SwContentNode *pCNode = pCFrame->GetNode();
                // --> assure, that the cursor is consistent.
                KillPams();
                ClearMark();
                SwPaM       *pCursor  = GetCursor();

                pCursor->GetPoint()->nNode = *pCNode;
                pCursor->GetPoint()->nContent.Assign( pCNode, 0 );

                SwRect& rChrRect = (SwRect&)GetCharRect();
                rChrRect = pFly->Prt();
                rChrRect.Pos() += pFly->Frame().Pos();
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
        if ( dynamic_cast<const SwVirtFlyDrawObj*>( pO) !=  nullptr )
        {
            SwFlyFrame *pFly = static_cast<SwVirtFlyDrawObj*>(pO)->GetFlyFrame();

            const SwFormatHoriOrient &rHori = pFly->GetFormat()->GetHoriOrient();
            aHori = rHori.GetHoriOrient();
            if( text::HoriOrientation::NONE != aHori && text::HoriOrientation::CENTER != aHori &&
                pFly->IsFlyAtContentFrame() )
            {
                bCheckNeighbours = true;
                pFly->InvalidatePos();
                pFly->Frame().Pos().Y() += 1;
            }

            pPage = pFly->FindPageFrame();
            aRect = pFly->Frame();
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
            if ( dynamic_cast<const SwFlyFrame*>( pAnchoredObj) ==  nullptr )
                continue;

            SwFlyFrame* pAct = static_cast<SwFlyFrame*>(pAnchoredObj);
            SwRect aTmpCalcPnt( pAct->Prt() );
            aTmpCalcPnt += pAct->Frame().Pos();
            if ( aRect.IsOver( aTmpCalcPnt ) )
            {
                SwContentFrame *pCnt = pAct->ContainsContent();
                while ( pCnt )
                {
                    aTmpCalcPnt = pCnt->Prt();
                    aTmpCalcPnt += pCnt->Frame().Pos();
                    if ( aRect.IsOver( aTmpCalcPnt ) )
                        static_cast<SwFrame*>(pCnt)->Prepare( PREP_FLY_ATTR_CHG );
                    pCnt = pCnt->GetNextContentFrame();
                }
            }
            if ( bCheckNeighbours && pAct->IsFlyAtContentFrame() )
            {
                const SwFormatHoriOrient &rH = pAct->GetFormat()->GetHoriOrient();
                if ( rH.GetHoriOrient() == aHori &&
                     pAct->Frame().Top()    <= aRect.Bottom() &&
                     pAct->Frame().Bottom() >= aRect.Top() )
                {
                    pAct->InvalidatePos();
                    pAct->Frame().Pos().Y() += 1;
                }
            }
        }
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
    GetDoc()->getIDocumentState().SetModified();
    EndAllAction();
}

// Object above/below the document? 2 Controls, 1 Heaven, 0 Hell,
// -1 Ambiguous
short SwFEShell::GetLayerId() const
{
    short nRet = SHRT_MAX;
    if ( Imp()->HasDrawView() )
    {
        const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkedObjectList();
        for ( size_t i = 0; i < rMrkList.GetMarkCount(); ++i )
        {
            const SdrObject *pObj = rMrkList.GetMark( i )->GetMarkedSdrObj();
            if( !pObj )
                continue;
            if ( nRet == SHRT_MAX )
                nRet = pObj->GetLayer();
            else if ( nRet != pObj->GetLayer() )
            {
                nRet = -1;
                break;
            }
        }
    }
    if ( nRet == SHRT_MAX )
        nRet = -1;
    return nRet;
}

// Object above/below the document
// Note: only visible objects can be marked. Thus, objects with invisible
//       layer IDs have not to be considered.
//       If <SwFEShell> exists, layout exists!!
void SwFEShell::ChangeOpaque( SdrLayerID nLayerId )
{
    if ( Imp()->HasDrawView() )
    {
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
                if ( dynamic_cast<const SwVirtFlyDrawObj*>( pObj) !=  nullptr )
                {
                    SwFormat *pFormat = static_cast<SwVirtFlyDrawObj*>(pObj)->GetFlyFrame()->GetFormat();
                    SvxOpaqueItem aOpa( pFormat->GetOpaque() );
                    aOpa.SetValue(  nLayerId == rIDDMA.GetHellId() );
                    pFormat->SetFormatAttr( aOpa );
                }
            }
        }
        GetDoc()->getIDocumentState().SetModified();
    }
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
                                        const_cast<SwViewShell*>(static_cast<SwViewShell const *>(this)) );
}

bool SwFEShell::IsObjSelected( const SdrObject& rObj ) const
{
    if ( IsFrameSelected() || !Imp()->HasDrawView() )
        return false;
    else
        return Imp()->GetDrawView()
                    ->IsObjMarked( const_cast< SdrObject * >( &rObj ) );
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
            if (pMarkObj && pMarkObj->GetUpGroup() == pObj->GetUpGroup())
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
            "EndTextEdit an no Object" );

    StartAllAction();
    SdrView *pView = Imp()->GetDrawView();
    SdrObject *pObj = pView->GetTextEditObject();
    SdrObjUserCall* pUserCall;
    if( nullptr != ( pUserCall = GetUserCall(pObj) ) )
    {
        SdrObject *pTmp = static_cast<SwContact*>(pUserCall)->GetMaster();
        if( !pTmp )
            pTmp = pObj;
        pUserCall->Changed( *pTmp, SDRUSERCALL_RESIZE, pTmp->GetLastBoundRect() );
    }
    if ( !pObj->GetUpGroup() )
    {
        if ( SDRENDTEXTEDIT_SHOULDBEDELETED == pView->SdrEndTextEdit(true) )
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
    EndAllAction();
}

int SwFEShell::IsInsideSelectedObj( const Point &rPt )
{
    if( Imp()->HasDrawView() )
    {
        SwDrawView *pDView = Imp()->GetDrawView();

        if( pDView->GetMarkedObjectList().GetMarkCount() &&
            pDView->IsMarkedObjHit( rPt ) )
        {
            return SDRHIT_OBJECT;
        }
    }
    return SDRHIT_NONE;
}

bool SwFEShell::IsObjSelectable( const Point& rPt )
{
    SET_CURR_SHELL(this);
    SwDrawView *pDView = Imp()->GetDrawView();
    bool bRet = false;
    if( pDView )
    {
        SdrObject* pObj;
        SdrPageView* pPV;
        const auto nOld = pDView->GetHitTolerancePixel();
        pDView->SetHitTolerancePixel( pDView->GetMarkHdlSizePixel()/2 );

        bRet = pDView->PickObj( rPt, pDView->getHitTolLog(), pObj, pPV, SdrSearchOptions::PICKMARKABLE );
        pDView->SetHitTolerancePixel( nOld );
    }
    return bRet;
}

SdrObject* SwFEShell::GetObjAt( const Point& rPt )
{
    SdrObject* pRet = nullptr;
    SET_CURR_SHELL(this);
    SwDrawView *pDView = Imp()->GetDrawView();
    if( pDView )
    {
        SdrPageView* pPV;
        const auto nOld = pDView->GetHitTolerancePixel();
        pDView->SetHitTolerancePixel( pDView->GetMarkHdlSizePixel()/2 );

        pDView->PickObj( rPt, pDView->getHitTolLog(), pRet, pPV, SdrSearchOptions::PICKMARKABLE );
        pDView->SetHitTolerancePixel( nOld );
    }
    return pRet;
}

// Test if there is a object at that position and if it should be selected.
bool SwFEShell::ShouldObjectBeSelected(const Point& rPt)
{
    SET_CURR_SHELL(this);
    SwDrawView *pDrawView = Imp()->GetDrawView();
    bool bRet(false);

    if(pDrawView)
    {
        SdrObject* pObj;
        SdrPageView* pPV;
        const auto nOld(pDrawView->GetHitTolerancePixel());

        pDrawView->SetHitTolerancePixel(pDrawView->GetMarkHdlSizePixel()/2);
        bRet = pDrawView->PickObj(rPt, pDrawView->getHitTolLog(), pObj, pPV, SdrSearchOptions::PICKMARKABLE);
        pDrawView->SetHitTolerancePixel(nOld);

        if ( bRet && pObj )
        {
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
                    if ( rSurround.GetSurround() == SURROUND_THROUGHT )
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
                        if ( pContentFrame->UnionFrame().IsInside( rPt ) )
                        {
                            const SwTextFrame* pTextFrame =
                                    dynamic_cast<const SwTextFrame*>(pContentFrame);
                            if ( pTextFrame )
                            {
                                SwPosition aPos( *(pTextFrame->GetTextNode()) );
                                Point aTmpPt( rPt );
                                if (pTextFrame->GetKeyCursorOfst(&aPos, aTmpPt))
                                {
                                    SwRect aCursorCharRect;
                                    if (pTextFrame->GetCharRect(aCursorCharRect,
                                                aPos))
                                    {
                                        if ( aCursorCharRect.IsOver( SwRect( pObj->GetLastBoundRect() ) ) )
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
                if ( ( IsHeaderFooterEdit() && !bInHdrFtr ) ||
                     ( !IsHeaderFooterEdit() && bInHdrFtr ) )
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

                    if (dynamic_cast<const SwVirtFlyDrawObj*>( pCandidate) !=  nullptr &&
                       static_cast<SwVirtFlyDrawObj*>(pCandidate)->GetCurrentBoundRect().IsInside(rPt) )
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
    else if( dynamic_cast<const SdrObjGroup*>( pObj) !=  nullptr )
    {
        bRet = true;
        const SdrObjList *pLst = static_cast<const SdrObjGroup*>(pObj)->GetSubList();
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
            return m_pPV && m_pPV->GetView().IsObjMarkable( const_cast< SdrObject* >( &i_rObject ), m_pPV );
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

    const long nTmp = bNext ? LONG_MAX : 0;
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
            if( dynamic_cast<const SwVirtFlyDrawObj*>( pStartObj) !=  nullptr )
                aPos = static_cast<const SwVirtFlyDrawObj*>(pStartObj)->GetFlyFrame()->Frame().Pos();
            else
                aPos = pStartObj->GetSnapRect().TopLeft();

            // If an object inside a group is selected, we want to
            // iterate over the group members.
            if ( ! pStartObj->GetUserCall() )
                pList = pStartObj->GetObjList();
        }
        else
        {
            // If no object is selected, we check if we just entered a group.
            // In this case we want to iterate over the group members.
            aPos = GetCharRect().Center();
            const SdrObject* pStartObj = pPV ? pPV->GetAktGroup() : nullptr;
            if ( pStartObj && dynamic_cast<const SdrObjGroup*>( pStartObj) !=  nullptr )
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

        SdrObjListIter aObjIter( *pList, bFlat ? IM_FLAT : IM_DEEPNOGROUPS );
        while ( aObjIter.IsMore() )
        {
            SdrObject* pObj = aObjIter.Next();
            bool bFlyFrame = dynamic_cast<const SwVirtFlyDrawObj*>( pObj) !=  nullptr;
            if( ( bNoFly && bFlyFrame ) ||
                ( bNoDraw && !bFlyFrame ) ||
                ( eType == GotoObjFlags::DrawSimple && lcl_IsControlGroup( pObj ) ) ||
                ( eType == GotoObjFlags::DrawControl && !lcl_IsControlGroup( pObj ) ) ||
                ( pFilter && !pFilter->includeObject( *pObj ) ) )
                continue;
            if( bFlyFrame )
            {
                SwVirtFlyDrawObj *pO = static_cast<SwVirtFlyDrawObj*>(pObj);
                SwFlyFrame *pFly = pO->GetFlyFrame();
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
                                (pFly->Lower()->IsLayoutFrame() ||
                                !static_cast<SwContentFrame*>(pFly->Lower())->GetNode()->GetGrfNode()))
                                continue;
                        break;
                        case GotoObjFlags::FlyOLE:
                            if ( pFly->Lower() &&
                                (pFly->Lower()->IsLayoutFrame() ||
                                !static_cast<SwContentFrame*>(pFly->Lower())->GetNode()->GetOLENode()))
                                continue;
                        break;
                        default: break;
                    }
                }
                aCurPos = pFly->Frame().Pos();
            }
            else
                aCurPos = pObj->GetCurrentBoundRect().TopLeft();

            // Special case if another object is on same Y.
            if( aCurPos != aPos &&          // only when it is not me
                aCurPos.getY() == aPos.getY() &&  // Y positions equal
                (bNext? (aCurPos.getX() > aPos.getX()) :  // lies next to me
                        (aCurPos.getX() < aPos.getX())) ) // " reverse
            {
                aBestPos = Point( nTmp, nTmp );
                SdrObjListIter aTmpIter( *pList, bFlat ? IM_FLAT : IM_DEEPNOGROUPS );
                while ( aTmpIter.IsMore() )
                {
                    SdrObject* pTmpObj = aTmpIter.Next();
                    bFlyFrame = dynamic_cast<const SwVirtFlyDrawObj*>( pTmpObj) !=  nullptr;
                    if( ( bNoFly && bFlyFrame ) || ( bNoDraw && !bFlyFrame ) )
                        continue;
                    if( bFlyFrame )
                    {
                        SwVirtFlyDrawObj *pO = static_cast<SwVirtFlyDrawObj*>(pTmpObj);
                        aCurPos = pO->GetFlyFrame()->Frame().Pos();
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
        if( (bNext? (aBestPos.getX() == LONG_MAX) : (aBestPos.getX() == 0)) )
            pBest = pTop;
    }

    return pBest;
}

bool SwFEShell::GotoObj( bool bNext, GotoObjFlags eType )
{
    const SdrObject* pBest = GetBestObject( bNext, eType );

    if ( !pBest )
        return false;

    bool bFlyFrame = dynamic_cast<const SwVirtFlyDrawObj*>( pBest) !=  nullptr;
    if( bFlyFrame )
    {
        const SwVirtFlyDrawObj *pO = static_cast<const SwVirtFlyDrawObj*>(pBest);
        const SwRect& rFrame = pO->GetFlyFrame()->Frame();
        SelectObj( rFrame.Pos(), 0, const_cast<SdrObject*>(pBest) );
        if( !ActionPend() )
            MakeVisible( rFrame );
    }
    else
    {
        SelectObj( Point(), 0, const_cast<SdrObject*>(pBest) );
        if( !ActionPend() )
            MakeVisible( pBest->GetCurrentBoundRect() );
    }
    CallChgLnk();
    return true;
}

bool SwFEShell::BeginCreate( sal_uInt16 /*SdrObjKind ?*/  eSdrObjectKind, const Point &rPos )
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

bool SwFEShell::BeginCreate( sal_uInt16 /*SdrObjKind ?*/  eSdrObjectKind, sal_uInt32 eObjInventor,
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

bool SwFEShell::EndCreate( sal_uInt16 eSdrCreateCmd )
{
    // To assure undo-object from the DrawEngine is not stored,
    // (we create our own undo-object!), temporarily switch-off Undo
    OSL_ENSURE( Imp()->HasDrawView(), "EndCreate without DrawView?" );
    if( !Imp()->GetDrawView()->IsGroupEntered() )
    {
        GetDoc()->GetIDocumentUndoRedo().DoDrawUndo(false);
    }
    bool bCreate = Imp()->GetDrawView()->EndCreateObj(
                                    SdrCreateCmd( eSdrCreateCmd ) );
    GetDoc()->GetIDocumentUndoRedo().DoDrawUndo(true);

    if ( !bCreate )
    {
        ::FrameNotify( this, FLY_DRAG_END );
        return false;
    }

    if ( (SdrCreateCmd)eSdrCreateCmd == SDRCREATE_NEXTPOINT )
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

    if( rSdrObj.GetUpGroup() )
    {
        Point aTmpPos( rSdrObj.GetSnapRect().TopLeft() );
        Point aNewAnchor( rSdrObj.GetUpGroup()->GetAnchorPos() );
        // OD 2004-04-05 #i26791# - direct object positioning for group members
        rSdrObj.NbcSetRelativePos( aTmpPos - aNewAnchor );
        rSdrObj.NbcSetAnchorPos( aNewAnchor );
        ::FrameNotify( this );
        return true;
    }

    LockPaint();
    StartAllAction();

    Imp()->GetDrawView()->UnmarkAll();

    const Rectangle &rBound = rSdrObj.GetSnapRect();
    Point aPt( rBound.TopRight() );

    // alien identifier should end up on defaults
    // duplications possible!!
    sal_uInt16 nIdent = SdrInventor == rSdrObj.GetObjInventor()
                        ? rSdrObj.GetObjIdentifier()
                        : 0xFFFF;

    // default for controls character bound, otherwise paragraph bound.
    SwFormatAnchor aAnch;
    const SwFrame *pAnch = nullptr;
    bool bCharBound = false;
    if( dynamic_cast<const SdrUnoObj*>( &rSdrObj) !=  nullptr )
    {
        SwPosition aPos( GetDoc()->GetNodes() );
        SwCursorMoveState aState( MV_SETONLYTEXT );
        Point aPoint( aPt.getX(), aPt.getY() + rBound.GetHeight()/2 );
        GetLayout()->GetCursorOfst( &aPos, aPoint, &aState );

        // characterbinding not allowed in readonly-content
        if( !aPos.nNode.GetNode().IsProtect() )
        {
            pAnch = aPos.nNode.GetNode().GetContentNode()->getLayoutFrame( GetLayout(), &aPoint, &aPos );
            SwRect aTmp;
            pAnch->GetCharRect( aTmp, aPos );

            // The crsr should not be too far away
            bCharBound = true;
            Rectangle aRect( aTmp.SVRect() );
            aRect.Left()  -= MM50*2;
            aRect.Top()   -= MM50*2;
            aRect.Right() += MM50*2;
            aRect.Bottom()+= MM50*2;

            if( !aRect.IsOver( rBound ) && !::GetHtmlMode( GetDoc()->GetDocShell() ))
                bCharBound = false;

            // anchor in header/footer also not allowed.
            if( bCharBound )
                bCharBound = !GetDoc()->IsInHeaderFooter( aPos.nNode );

            if( bCharBound )
            {
                aAnch.SetType( FLY_AS_CHAR );
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
        SwCursorMoveState aState( MV_SETONLYTEXT );
        Point aPoint( aPt );
        SwPosition aPos( GetDoc()->GetNodes() );
        GetLayout()->GetCursorOfst( &aPos, aPoint, &aState );

        // do not set in ReadnOnly-content
        if (aPos.nNode.GetNode().IsProtect())
        {
            // then only page bound. Or should we
            // search the next not-readonly position?
            bAtPage = true;
        }

        SwContentNode* pCNode = aPos.nNode.GetNode().GetContentNode();
        pAnch = pCNode ? pCNode->getLayoutFrame( GetLayout(), &aPoint, nullptr, false ) : nullptr;
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
                    if( pTmp->Frame().IsInside( aBound ) )
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
            aPos.nNode = *static_cast<const SwContentFrame*>(pAnch)->GetNode();

            // do not set in ReadnOnly-content
            if( aPos.nNode.GetNode().IsProtect() )
                // then only page bound. Or should we
                // search the next not-readonly position?
                bAtPage = true;
            else
            {
                aAnch.SetType( FLY_AT_PARA );
                aAnch.SetAnchor( &aPos );
            }
        }

        if( bAtPage )
        {
            pPage = pAnch ? pAnch->FindPageFrame() : GetLayout()->GetPageAtPos(aPoint);

            aAnch.SetType( FLY_AT_PAGE );
            aAnch.SetPageNum( pPage->GetPhyPageNum() );
            pAnch = pPage;      // page becomes an anchor
        }
    }

    SfxItemSet aSet( GetDoc()->GetAttrPool(), RES_FRM_SIZE, RES_FRM_SIZE,
                                            RES_SURROUND, RES_ANCHOR, 0 );
    aSet.Put( aAnch );

    // OD 2004-03-30 #i26791# - determine relative object position
    SwTwips nXOffset;
    SwTwips nYOffset = rBound.Top() - pAnch->Frame().Top();
    {
        if( pAnch->IsVertical() )
        {
            nXOffset = nYOffset;
            nYOffset = pAnch->Frame().Left()+pAnch->Frame().Width()-rBound.Right();
        }
        else if( pAnch->IsRightToLeft() )
            nXOffset = pAnch->Frame().Left()+pAnch->Frame().Width()-rBound.Right();
        else
            nXOffset = rBound.Left() - pAnch->Frame().Left();
        if( pAnch->IsTextFrame() && static_cast<const SwTextFrame*>(pAnch)->IsFollow() )
        {
            const SwTextFrame* pTmp = static_cast<const SwTextFrame*>(pAnch);
            do {
                pTmp = pTmp->FindMaster();
                OSL_ENSURE( pTmp, "Where's my Master?" );
                // OD 2004-03-30 #i26791# - correction: add frame area height
                // of master frames.
                nYOffset += pTmp->IsVertical() ?
                            pTmp->Frame().Width() : pTmp->Frame().Height();
            } while ( pTmp->IsFollow() );
        }
    }

    if( OBJ_NONE == nIdent )
    {
        // For OBJ_NONE a fly is inserted.
        const long nWidth = rBound.Right()  - rBound.Left();
        const long nHeight= rBound.Bottom() - rBound.Top();
        aSet.Put( SwFormatFrameSize( ATT_MIN_SIZE, std::max( nWidth,  long(MINFLY) ),
                                              std::max( nHeight, long(MINFLY) )));

        SwFormatHoriOrient aHori( nXOffset, text::HoriOrientation::NONE, text::RelOrientation::FRAME );
        SwFormatVertOrient aVert( nYOffset, text::VertOrientation::NONE, text::RelOrientation::FRAME );
        aSet.Put( SwFormatSurround( SURROUND_PARALLEL ) );
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
            pPg = pTmpSdrModel->AllocPage( false );
            pTmpSdrModel->InsertPage( pPg );
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
            SfxItemSet aHtmlSet( GetDoc()->GetAttrPool(), RES_VERT_ORIENT, RES_HORI_ORIENT );
            // horizontal orientation:
            const bool bLeftFrame = aFlyRect.Left() <
                                      pAnch->Frame().Left() + pAnch->Prt().Left(),
                           bLeftPrt = aFlyRect.Left() + aFlyRect.Width() <
                                      pAnch->Frame().Left() + pAnch->Prt().Width()/2;
            if( bLeftFrame || bLeftPrt )
            {
                aHori.SetHoriOrient( text::HoriOrientation::LEFT );
                aHori.SetRelationOrient( bLeftFrame ? text::RelOrientation::FRAME : text::RelOrientation::PRINT_AREA );
            }
            else
            {
                const bool bRightFrame = aFlyRect.Left() >
                                           pAnch->Frame().Left() + pAnch->Prt().Width();
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
        Point aRelNullPt;
        if( OBJ_CAPTION == nIdent )
            aRelNullPt = static_cast<SdrCaptionObj&>(rSdrObj).GetTailPos();
        else
            aRelNullPt = rBound.TopLeft();

        aSet.Put( aAnch );
        aSet.Put( SwFormatSurround( SURROUND_THROUGHT ) );
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
                            pTmp->Prt().Width() : pTmp->Prt().Height();
            } while ( pTmp->IsFollow() );
        }
        SwFormatVertOrient aVert( nYOffset, text::VertOrientation::NONE, text::RelOrientation::FRAME );
        aSet.Put( aVert );
        SwDrawFrameFormat* pFormat = static_cast<SwDrawFrameFormat*>(getIDocumentLayoutAccess().MakeLayoutFormat( RND_DRAW_OBJECT, &aSet ));
        // #i36010# - set layout direction of the position
        pFormat->SetPositionLayoutDir(
            text::PositionLayoutDir::PositionInLayoutDirOfAnchor );
        // #i44344#, #i44681# - positioning attributes already set
        pFormat->PosAttrSet();

        SwDrawContact *pContact = new SwDrawContact( pFormat, &rSdrObj );
        // #i35635#
        pContact->MoveObjToVisibleLayer( &rSdrObj );
        if( bCharBound )
        {
            OSL_ENSURE( aAnch.GetAnchorId() == FLY_AS_CHAR, "wrong AnchorType" );
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

        if (rSdrObj.GetName().isEmpty())
            rSdrObj.SetName(GetUniqueShapeName());

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
            return pDView->BegMarkObj( rPos );
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
            SdrMarkList &rMrkList = (SdrMarkList&)pDView->GetMarkedObjectList();
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

void SwFEShell::BreakMark()
{
    OSL_ENSURE( Imp()->HasDrawView(), "BreakMark without DrawView?" );
    Imp()->GetDrawView()->BrkMarkObj();
}

short SwFEShell::GetAnchorId() const
{
    short nRet = SHRT_MAX;
    if ( Imp()->HasDrawView() )
    {
        const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkedObjectList();
        for ( size_t i = 0; i < rMrkList.GetMarkCount(); ++i )
        {
            SdrObject *pObj = rMrkList.GetMark( i )->GetMarkedSdrObj();
            if ( dynamic_cast<const SwVirtFlyDrawObj*>( pObj) !=  nullptr )
            {
                nRet = -1;
                break;
            }
            SwDrawContact *pContact = static_cast<SwDrawContact*>(GetUserCall(pObj));
            short nId = static_cast<short>(pContact->GetFormat()->GetAnchor().GetAnchorId());
            if ( nRet == SHRT_MAX )
                nRet = nId;
            else if ( nRet != nId )
            {
                nRet = -1;
                break;
            }
        }
    }
    if ( nRet == SHRT_MAX )
        nRet = -1;
    return nRet;
}

void SwFEShell::ChgAnchor( int eAnchorId, bool bSameOnly, bool bPosCorr )
{
    OSL_ENSURE( Imp()->HasDrawView(), "ChgAnchor without DrawView?" );
    const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkedObjectList();
    if( rMrkList.GetMarkCount() &&
        !rMrkList.GetMark( 0 )->GetMarkedSdrObj()->GetUpGroup() )
    {
        StartAllAction();

        if( GetDoc()->ChgAnchor( rMrkList, (RndStdIds)eAnchorId, bSameOnly, bPosCorr ))
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
    Rectangle aRect;
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

    Rectangle aRect;
    if ( Imp()->GetDrawView()->IsAction() )
        Imp()->GetDrawView()->TakeActionRect( aRect );
    else
        aRect = Imp()->GetDrawView()->GetAllMarkedRect();

    Point aRet( aRect.TopLeft() );

    if ( IsFrameSelected() )
    {
        SwFlyFrame *pFly = GetSelectedFlyFrame();
        aRet -= pFly->GetAnchorFrame()->Frame().Pos();
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
                 FLY_AS_CHAR != static_cast<SwDrawContact*>(GetUserCall(pObj))->
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
        else if (FLY_AS_CHAR == pFrameFormat->GetAnchor().GetAnchorId())
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
                bIsGroupAllowed = pObj->GetUpGroup() == pUpGroup;
            else
                pUpGroup = pObj->GetUpGroup();

            if ( bIsGroupAllowed )
                bIsGroupAllowed = HasSuitableGroupingAnchor(pObj);

            // check, if all selected objects are in the
            // same header/footer or not in header/footer.
            if ( bIsGroupAllowed )
            {
                const SwFrame* pAnchorFrame = nullptr;
                if ( dynamic_cast<const SwVirtFlyDrawObj*>( pObj) !=  nullptr )
                {
                    const SwFlyFrame* pFlyFrame =
                            static_cast<const SwVirtFlyDrawObj*>(pObj)->GetFlyFrame();
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
        StartUndo( UNDO_START );

        GetDoc()->GroupSelection( *Imp()->GetDrawView() );

        EndUndo( UNDO_END );
        EndAllAction();
    }
}

// The individual objects get a copy of the anchor and the contactobject of the group
void SwFEShell::UnGroupSelection()
{
    if ( IsGroupSelected() )
    {
        StartAllAction();
        StartUndo( UNDO_START );

        GetDoc()->UnGroupSelection( *Imp()->GetDrawView() );

        EndUndo( UNDO_END );
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
    static sal_uInt8 const aChkArr[ 4 ] = {
             /* FLYCNTTYPE_ALL */   0,
             /* FLYCNTTYPE_FRM */   ND_TEXTNODE,
             /* FLYCNTTYPE_GRF */   ND_GRFNODE,
             /* FLYCNTTYPE_OLE */   ND_OLENODE
            };

    const SwFlyFrameFormat* pFlyFormat = mpDoc->FindFlyByName( rName, aChkArr[ eType]);
    if( pFlyFormat )
    {
        SET_CURR_SHELL( this );

        SwFlyFrame* pFrame = SwIterator<SwFlyFrame,SwFormat>( *pFlyFormat ).First();
        if( pFrame )
        {
            if( bSelFrame )
            {
                SelectObj( pFrame->Frame().Pos(), 0, pFrame->GetVirtDrawObj() );
                if( !ActionPend() )
                    MakeVisible( pFrame->Frame() );
            }
            else
            {
                SwContentFrame *pCFrame = pFrame->ContainsContent();
                if ( pCFrame )
                {
                    SwContentNode *pCNode = pCFrame->GetNode();
                    ClearMark();
                    SwPaM* pCursor = GetCursor();

                    pCursor->GetPoint()->nNode = *pCNode;
                    pCursor->GetPoint()->nContent.Assign( pCNode, 0 );

                    SwRect& rChrRect = (SwRect&)GetCharRect();
                    rChrRect = pFrame->Prt();
                    rChrRect.Pos() += pFrame->Frame().Pos();
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
        MakeVisible( Imp()->GetDrawView()->GetAllMarkedRect() );
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
            if( !bParent )
            {
                nChk |= ( pObj->IsMoveProtect() ? FlyProtectFlags::Pos : FlyProtectFlags::NONE ) |
                        ( pObj->IsResizeProtect()? FlyProtectFlags::Size : FlyProtectFlags::NONE );

                if( dynamic_cast<const SwVirtFlyDrawObj*>( pObj) !=  nullptr )
                {
                    SwFlyFrame *pFly = static_cast<SwVirtFlyDrawObj*>(pObj)->GetFlyFrame();
                    if ( (FlyProtectFlags::Content & eType) && pFly->GetFormat()->GetProtect().IsContentProtected() )
                        nChk |= FlyProtectFlags::Content;

                    if ( pFly->Lower() && pFly->Lower()->IsNoTextFrame() )
                    {
                        SwOLENode *pNd = static_cast<SwContentFrame*>(pFly->Lower())->GetNode()->GetOLENode();
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
                                    && FLY_AS_CHAR == pFly->GetFormat()->GetAnchor().GetAnchorId()
                                    && mpDoc->GetDocumentSettingManager().get( DocumentSettingId::MATH_BASELINE_ALIGNMENT );
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
            if( dynamic_cast<const SwVirtFlyDrawObj*>( pObj) !=  nullptr )
                pAnch = static_cast<SwVirtFlyDrawObj*>( pObj )->GetFlyFrame()->GetAnchorFrame();
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

bool SwFEShell::SetObjAttr( const SfxItemSet& rSet )
{
    SET_CURR_SHELL( this );

    if ( !rSet.Count() )
    { OSL_ENSURE( false, "SetObjAttr, empty set." );
        return false;
    }

    StartAllAction();
    StartUndo( UNDO_INSATTR );

    const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkedObjectList();
    for ( size_t i = 0; i < rMrkList.GetMarkCount(); ++i )
    {
        SdrObject *pObj = rMrkList.GetMark( i )->GetMarkedSdrObj();
        SwDrawContact *pContact = static_cast<SwDrawContact*>(GetUserCall(pObj));
        GetDoc()->SetAttr( rSet, *pContact->GetFormat() );
    }

    EndUndo( UNDO_INSATTR );
    EndAllActionAndCall();
    GetDoc()->getIDocumentState().SetModified();
    return true;
}

bool SwFEShell::IsAlignPossible() const
{
    const size_t nCnt = IsObjSelected();
    if ( 0 < nCnt )
    {
        bool bRet = true;
        if ( nCnt == 1 )
        {
            SdrObject *pO = Imp()->GetDrawView()->GetMarkedObjectList().GetMark(0)->GetMarkedSdrObj();
            SwDrawContact *pC = static_cast<SwDrawContact*>(GetUserCall(pO));
            OSL_ENSURE( pC, "No SwDrawContact!");
            //only as character bound drawings can be aligned
            bRet = pC && pC->GetFormat()->GetAnchor().GetAnchorId() == FLY_AS_CHAR;
        }
        if ( bRet )
            return Imp()->GetDrawView()->IsAlignPossible();
    }
    return false;
}

// temporary fix till  SS of JOE is available
void SwFEShell::CheckUnboundObjects()
{
    SET_CURR_SHELL( this );

    const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkedObjectList();
    for ( size_t i = 0; i < rMrkList.GetMarkCount(); ++i )
    {
        SdrObject *pObj = rMrkList.GetMark( i )->GetMarkedSdrObj();
        if ( !GetUserCall(pObj) )
        {
            const Rectangle &rBound = pObj->GetSnapRect();
            const Point aPt( rBound.TopLeft() );
            const SwFrame *pPage = GetLayout()->Lower();
            const SwFrame *pLast = pPage;
            while ( pPage && !pPage->Frame().IsInside( aPt ) )
            {
                if ( aPt.Y() > pPage->Frame().Bottom() )
                    pLast = pPage;
                pPage = pPage->GetNext();
            }
            if ( !pPage )
                pPage = pLast;
            OSL_ENSURE( pPage, "Page not found." );

            // Alien identifier should roll into the default,
            // Duplications are possible!!
            sal_uInt16 nIdent =
                    Imp()->GetDrawView()->GetCurrentObjInventor() == SdrInventor ?
                            Imp()->GetDrawView()->GetCurrentObjIdentifier() : 0xFFFF;

            SwFormatAnchor aAnch;
            {
            const SwFrame *pAnch = ::FindAnchor( pPage, aPt, true );
            SwPosition aPos( *static_cast<const SwContentFrame*>(pAnch)->GetNode() );
            aAnch.SetType( FLY_AT_PARA );
            aAnch.SetAnchor( &aPos );
            const_cast<SwRect&>(GetCharRect()).Pos() = aPt;
            }

            // First the action here, to assure GetCharRect delivers current values.
            StartAllAction();

            SfxItemSet aSet( GetAttrPool(), RES_FRM_SIZE, RES_FRM_SIZE,
                                            RES_SURROUND, RES_ANCHOR, 0 );
            aSet.Put( aAnch );

            Point aRelNullPt;

            if( OBJ_CAPTION == nIdent )
                aRelNullPt = static_cast<SdrCaptionObj*>(pObj)->GetTailPos();
            else
                aRelNullPt = rBound.TopLeft();

            aSet.Put( aAnch );
            aSet.Put( SwFormatSurround( SURROUND_THROUGHT ) );
            SwFrameFormat* pFormat = getIDocumentLayoutAccess().MakeLayoutFormat( RND_DRAW_OBJECT, &aSet );

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
        SdrObject* pObj;
        SdrPageView* pPView;
        SwDrawView *pDView = const_cast<SwDrawView*>(Imp()->GetDrawView());
        const auto nOld = pDView->GetHitTolerancePixel();
        pDView->SetHitTolerancePixel( 0 );
        if( pDView->PickObj( rPt, pDView->getHitTolLog(), pObj, pPView, SdrSearchOptions::PICKMARKABLE ) &&
            dynamic_cast<const SwVirtFlyDrawObj*>( pObj) !=  nullptr )
        {
            SwFlyFrame *pFly = static_cast<SwVirtFlyDrawObj*>(pObj)->GetFlyFrame();
            rRect = pFly->Frame();

            // Target and source should not be equal and the list
            // should not be cyclic
            SwFrameFormat *pFormat = pFly->GetFormat();
            nRet = GetDoc()->Chainable(rSource, *pFormat);
        }
        pDView->SetHitTolerancePixel( nOld );
    }
    return nRet;
}

SwChainRet SwFEShell::Chain( SwFrameFormat &rSource, const SwFrameFormat &rDest )
{
    return GetDoc()->Chain(rSource, rDest);
}

SwChainRet SwFEShell::Chain( SwFrameFormat &rSource, const Point &rPt )
{
    SwRect aDummy;
    SwChainRet nErr = Chainable( aDummy, rSource, rPt );
    if ( nErr == SwChainRet::OK )
    {
        StartAllAction();
        SdrObject* pObj;
        SdrPageView* pPView;
        SwDrawView *pDView = Imp()->GetDrawView();
        const auto nOld = pDView->GetHitTolerancePixel();
        pDView->SetHitTolerancePixel( 0 );
        pDView->PickObj( rPt, pDView->getHitTolLog(), pObj, pPView, SdrSearchOptions::PICKMARKABLE );
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

            Point aStart( pPre->Frame().Right(), pPre->Frame().Bottom());
            Point aEnd(pFly->Frame().Pos());

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

            Point aStart( pFly->Frame().Right(), pFly->Frame().Bottom());
            Point aEnd(pNxt->Frame().Pos());

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

long SwFEShell::GetSectionWidth( SwFormat const & rFormat ) const
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
                return pSect->Frame().Width();
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
            return pSct->Frame().Width();
        }
    }
    return 0;
}

 void SwFEShell::CreateDefaultShape( sal_uInt16 /*SdrObjKind ?*/ eSdrObjectKind, const Rectangle& rRect,
                sal_uInt16 nSlotId)
{
    SdrView* pDrawView = GetDrawView();
    SdrModel* pDrawModel = pDrawView->GetModel();
    SdrObject* pObj = SdrObjFactory::MakeNewObject(
        SdrInventor, eSdrObjectKind,
        nullptr, pDrawModel);

    if(pObj)
    {
        Rectangle aRect(rRect);
        if(OBJ_CARC == eSdrObjectKind || OBJ_CCUT == eSdrObjectKind)
        {
            // force quadratic
            if(aRect.GetWidth() > aRect.GetHeight())
            {
                aRect = Rectangle(
                    Point(aRect.Left() + ((aRect.GetWidth() - aRect.GetHeight()) / 2), aRect.Top()),
                    Size(aRect.GetHeight(), aRect.GetHeight()));
            }
            else
            {
                aRect = Rectangle(
                    Point(aRect.Left(), aRect.Top() + ((aRect.GetHeight() - aRect.GetWidth()) / 2)),
                    Size(aRect.GetWidth(), aRect.GetWidth()));
            }
        }
        pObj->SetLogicRect(aRect);

        if(dynamic_cast<const SdrCircObj*>( pObj) !=  nullptr)
        {
            SfxItemSet aAttr(pDrawModel->GetItemPool());
            aAttr.Put(makeSdrCircStartAngleItem(9000));
            aAttr.Put(makeSdrCircEndAngleItem(0));
            pObj->SetMergedItemSet(aAttr);
        }
        else if(dynamic_cast<const SdrPathObj*>( pObj) !=  nullptr)
        {
            basegfx::B2DPolyPolygon aPoly;

            switch(eSdrObjectKind)
            {
                case OBJ_PATHLINE:
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
                }
                break;
            }

            static_cast<SdrPathObj*>(pObj)->SetPathPoly(aPoly);
        }
        else if(dynamic_cast<const SdrCaptionObj*>( pObj) !=  nullptr)
        {
            bool bVerticalText = ( SID_DRAW_TEXT_VERTICAL == nSlotId ||
                                            SID_DRAW_CAPTION_VERTICAL == nSlotId );
            static_cast<SdrTextObj*>(pObj)->SetVerticalWriting(bVerticalText);
            if(bVerticalText)
            {
                SfxItemSet aSet(pObj->GetMergedItemSet());
                aSet.Put(SdrTextVertAdjustItem(SDRTEXTVERTADJUST_CENTER));
                aSet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_RIGHT));
                pObj->SetMergedItemSet(aSet);
            }

            static_cast<SdrCaptionObj*>(pObj)->SetLogicRect(aRect);
            static_cast<SdrCaptionObj*>(pObj)->SetTailPos(
                aRect.TopLeft() - Point(aRect.GetWidth() / 2, aRect.GetHeight() / 2));
        }
        else if(dynamic_cast<const SdrTextObj*>( pObj) !=  nullptr)
        {
            SdrTextObj* pText = static_cast<SdrTextObj*>(pObj);
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
                SfxItemSet aSet(pDrawModel->GetItemPool(), SDRATTR_MISC_FIRST, SDRATTR_MISC_LAST);
                aSet.Put( makeSdrTextAutoGrowWidthItem( false ) );
                aSet.Put( makeSdrTextAutoGrowHeightItem( false ) );
                aSet.Put( SdrTextAniKindItem( SDRTEXTANI_SLIDE ) );
                aSet.Put( SdrTextAniDirectionItem( SDRTEXTANI_LEFT ) );
                aSet.Put( SdrTextAniCountItem( 1 ) );
                aSet.Put( SdrTextAniAmountItem( (sal_Int16)GetWin()->PixelToLogic(Size(2,1)).Width()) );
                pObj->SetMergedItemSetAndBroadcast(aSet);
            }
        }
        SdrPageView* pPageView = pDrawView->GetSdrPageView();
        pDrawView->InsertObjectAtView(pObj, *pPageView);
    }
    ImpEndCreate();
}

/** SwFEShell::GetShapeBackgrd
    method determines background color of the page the selected drawing
    object is on and returns this color.
    If no color is found, because no drawing object is selected or ...,
    color COL_BLACK (default color on constructing object of class Color)
    is returned.

    @returns an object of class Color
*/
const Color SwFEShell::GetShapeBackgrd() const
{
    Color aRetColor;

    // check, if a draw view exists
    OSL_ENSURE( Imp()->GetDrawView(), "wrong usage of SwFEShell::GetShapeBackgrd - no draw view!");
    if( Imp()->GetDrawView() )
    {
        // determine list of selected objects
        const SdrMarkList* pMrkList = &Imp()->GetDrawView()->GetMarkedObjectList();
        // check, if exactly one object is selected.
        OSL_ENSURE( pMrkList->GetMarkCount() == 1, "wrong usage of SwFEShell::GetShapeBackgrd - no selected object!");
        if ( pMrkList->GetMarkCount() == 1)
        {
            // get selected object
            const SdrObject *pSdrObj = pMrkList->GetMark( 0 )->GetMarkedSdrObj();
            // check, if selected object is a shape (drawing object)
            OSL_ENSURE( dynamic_cast<const SwVirtFlyDrawObj*>( pSdrObj) ==  nullptr, "wrong usage of SwFEShell::GetShapeBackgrd - selected object is not a drawing object!");
            if ( dynamic_cast<const SwVirtFlyDrawObj*>( pSdrObj) ==  nullptr )
            {
                // determine page frame of the frame the shape is anchored.
                const SwFrame* pAnchorFrame =
                        static_cast<SwDrawContact*>(GetUserCall(pSdrObj))->GetAnchorFrame( pSdrObj );
                OSL_ENSURE( pAnchorFrame, "inconsistent modell - no anchor at shape!");
                if ( pAnchorFrame )
                {
                    const SwPageFrame* pPageFrame = pAnchorFrame->FindPageFrame();
                    OSL_ENSURE( pPageFrame, "inconsistent modell - no page!");
                    if ( pPageFrame )
                    {
                        aRetColor = pPageFrame->GetDrawBackgrdColor();
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
    OSL_ENSURE( Imp()->GetDrawView(), "wrong usage of SwFEShell::GetShapeBackgrd - no draw view!");
    if( Imp()->GetDrawView() )
    {
        // determine list of selected objects
        const SdrMarkList* pMrkList = &Imp()->GetDrawView()->GetMarkedObjectList();
        // check, if exactly one object is selected.
        OSL_ENSURE( pMrkList->GetMarkCount() == 1, "wrong usage of SwFEShell::GetShapeBackgrd - no selected object!");
        if ( pMrkList->GetMarkCount() == 1)
        {
            // get selected object
            const SdrObject *pSdrObj = pMrkList->GetMark( 0 )->GetMarkedSdrObj();
            // check, if selected object is a shape (drawing object)
            OSL_ENSURE( dynamic_cast<const SwVirtFlyDrawObj*>( pSdrObj) ==  nullptr, "wrong usage of SwFEShell::GetShapeBackgrd - selected object is not a drawing object!");
            if ( dynamic_cast<const SwVirtFlyDrawObj*>( pSdrObj) ==  nullptr )
            {
                // determine page frame of the frame the shape is anchored.
                const SwFrame* pAnchorFrame =
                        static_cast<SwDrawContact*>(GetUserCall(pSdrObj))->GetAnchorFrame( pSdrObj );
                OSL_ENSURE( pAnchorFrame, "inconsistent modell - no anchor at shape!");
                if ( pAnchorFrame )
                {
                    const SwPageFrame* pPageFrame = pAnchorFrame->FindPageFrame();
                    OSL_ENSURE( pPageFrame, "inconsistent modell - no page!");
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
    while ( pPage && !pPage->Frame().IsInside( rDocPos ) )
    {
        pPage = pPage->GetNext();
    }
    if(pPage)
    {
        aRet = rDocPos - pPage->Frame().TopLeft();
    }
    return aRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
