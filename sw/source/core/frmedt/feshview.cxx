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


#include <com/sun/star/embed/EmbedMisc.hpp>

#include "hintids.hxx"

#include <svx/sdrobjectfilter.hxx>
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
#include <cmdid.h>
#include <poolfmt.hrc>      // for InitFldTypes
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
#include "viewopt.hxx"                  // for GetHTMLMode
#include "swundo.hxx"
#include "notxtfrm.hxx"
#include "txtfrm.hxx"
#include "txatbase.hxx"
#include "mdiexp.hxx"                   // for update of the statusline during dragging
#include <sortedobjs.hxx>
#include <HandleAnchorNodeChg.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <switerator.hxx>

#define SCROLLVAL 75

using namespace com::sun::star;

// tolerance limit of Drawing-SS
#define MINMOVE ((sal_uInt16)GetOut()->PixelToLogic(Size(Imp()->GetDrawView()->GetMarkHdlSizePixel()/2,0)).Width())

SwFlyFrm *GetFlyFromMarked( const SdrMarkList *pLst, ViewShell *pSh )
{
    if ( !pLst )
        pLst = pSh->HasDrawView() ? &pSh->Imp()->GetDrawView()->GetMarkedObjectList():0;

    if ( pLst && pLst->GetMarkCount() == 1 )
    {
        SdrObject *pO = pLst->GetMark( 0 )->GetMarkedSdrObj();
        if ( pO && pO->ISA(SwVirtFlyDrawObj) )
            return ((SwVirtFlyDrawObj*)pO)->GetFlyFrm();
    }
    return 0;
}

static void lcl_GrabCursor( SwFEShell* pSh, SwFlyFrm* pOldSelFly)
{
    const SwFrmFmt *pFlyFmt = pSh->SelFlyGrabCrsr();
    if( pFlyFmt && !pSh->ActionPend() &&
                        (!pOldSelFly || pOldSelFly->GetFmt() != pFlyFmt) )
    {
        // now call set macro if applicable
        pSh->GetFlyMacroLnk().Call( (void*)pFlyFmt );
extern sal_Bool bNoInterrupt;       // in swapp.cxx
        // if a dialog was started inside a macro, then
        // MouseButtonUp arrives at macro and not to us. Therefore
        // flag is always set here and will never be switched to
        // respective Shell !!!!!!!

        bNoInterrupt = sal_False;
    }
    else if( !pFlyFmt || RES_DRAWFRMFMT == pFlyFmt->Which() )
    {
        // --> assure consistent cursor
        pSh->KillPams();
        pSh->ClearMark();
        pSh->SetCrsr( pSh->Imp()->GetDrawView()->GetAllMarkedRect().TopLeft(), sal_True);
    }
}

/*************************************************************************
|*
|*  SwFEShell::SelectObj()
*************************************************************************/

sal_Bool SwFEShell::SelectObj( const Point& rPt, sal_uInt8 nFlag, SdrObject *pObj )
{
    SwDrawView *pDView = Imp()->GetDrawView();
    if(!pDView)
        return sal_False;
    SET_CURR_SHELL( this );
    StartAction();    // action is necessary to assure only one AttrChgdNotify
                      // (e.g. due to Unmark->MarkListHasChgd) arrives

    const SdrMarkList &rMrkList = pDView->GetMarkedObjectList();
    const sal_Bool bHadSelection = rMrkList.GetMarkCount() ? sal_True : sal_False;
    const sal_Bool bAddSelect = 0 != (SW_ADD_SELECT & nFlag);
    const sal_Bool bEnterGroup = 0 != (SW_ENTER_GROUP & nFlag);
    SwFlyFrm* pOldSelFly = 0;
    const Point aOldPos( pDView->GetAllMarkedRect().TopLeft() );

    if( bHadSelection )
    {
        // call Unmark when !bAddSelect or if fly was selected
        sal_Bool bUnmark = !bAddSelect;

        if ( rMrkList.GetMarkCount() == 1 )
        {
            // if fly was selected, deselect it first
            pOldSelFly = ::GetFlyFromMarked( &rMrkList, this );
            if ( pOldSelFly )
            {
                const sal_uInt16 nType = GetCntType();
                if( nType != CNT_TXT || (SW_LEAVE_FRAME & nFlag) ||
                    ( pOldSelFly->GetFmt()->GetProtect().IsCntntProtected()
                     && !IsReadOnlyAvailable() ))
                {
                    // If a fly is deselected, which contains graphic, OLE or
                    // otherwise, the cursor should be removed from it.
                    // Similar if a fly with protected content is deselected.
                    // For simplicity we put the cursor next to the upper-left
                    // corner.
                    Point aPt( pOldSelFly->Frm().Pos() );
                    aPt.X() -= 1;
                    sal_Bool bUnLockView = !IsViewLocked();
                    LockView( sal_True );
                    SetCrsr( aPt, sal_True );
                    if( bUnLockView )
                        LockView( sal_False );
                }
                if ( nType & CNT_GRF &&
                     ((SwNoTxtFrm*)pOldSelFly->Lower())->HasAnimation() )
                {
                    GetWin()->Invalidate( pOldSelFly->Frm().SVRect() );
                }
                bUnmark = sal_True;
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
        pDView->MarkObj( rPt, MINMOVE, bAddSelect, bEnterGroup );
    }

    const sal_Bool bRet = 0 != rMrkList.GetMarkCount();

    if ( rMrkList.GetMarkCount() > 1 )
    {
        // It sucks if Drawing objects were selected and now
        // additionally a fly is selected.
        for ( sal_uInt16 i = 0; i < rMrkList.GetMarkCount(); ++i )
        {
            SdrObject *pTmpObj = rMrkList.GetMark( i )->GetMarkedSdrObj();
            sal_Bool bForget = pTmpObj->ISA(SwVirtFlyDrawObj);
            if( bForget )
            {
                pDView->UnmarkAll();
                pDView->MarkObj( pTmpObj, Imp()->GetPageView(), bAddSelect, bEnterGroup );
                break;
            }
        }
    }

    if ( bRet )
    {
        ::lcl_GrabCursor(this, pOldSelFly);
        if ( GetCntType() & CNT_GRF )
        {
            const SwFlyFrm *pTmp = GetFlyFromMarked( &rMrkList, this );
            OSL_ENSURE( pTmp, "Graphic without Fly" );
            if ( ((SwNoTxtFrm*)pTmp->Lower())->HasAnimation() )
                ((SwNoTxtFrm*)pTmp->Lower())->StopAnimation( GetOut() );
        }
    }
    else if ( !pOldSelFly && bHadSelection )
        SetCrsr( aOldPos, sal_True);

    if( bRet || !bHadSelection )
        CallChgLnk();

    // update der Statuszeile
    ::FrameNotify( this, bRet ? FLY_DRAG_START : FLY_DRAG_END );

    EndAction();
    return bRet;
}

/*************************************************************************
|*
|*  sal_Bool SwFEShell::MoveAnchor( sal_uInt16 nDir )
|*
|*  Description: MoveAnchor( nDir ) looked for an another Anchor for
|*  the selected drawing object (or fly frame) in the given direction.
|*  An object "as character" doesn't moves anyway.
|*  A page bounded object could move to the previous/next page with up/down,
|*  an object bounded "at paragraph" moves to the previous/next paragraph, too.
|*  An object bounded "at character" moves to the previous/next paragraph
|*  with up/down and to the previous/next character with left/right.
|*  If the anchor for at paragraph/character bounded objects has vertical or
|*  right_to_left text direction, the directions for up/down/left/right will
|*  interpreted accordingly.
|*  An object bounded "at fly" takes the center of the actual anchor and looks
|*  for the nearest fly frame in the given direction.
|*
*************************************************************************/

#define LESS_X( aPt1, aPt2, bOld ) ( aPt1.X() < aPt2.X() || \
        ( aPt1.X() == aPt2.X() && ( aPt1.Y() < aPt2.Y() || \
        ( aPt1.Y() == aPt2.Y() && bOld ) ) ) )
#define LESS_Y( aPt1, aPt2, bOld ) ( aPt1.Y() < aPt2.Y() || \
        ( aPt1.Y() == aPt2.Y() && ( aPt1.X() < aPt2.X() || \
        ( aPt1.X() == aPt2.X() && bOld ) ) ) )

sal_Bool SwFEShell::MoveAnchor( sal_uInt16 nDir )
{
    const SdrMarkList* pMrkList;
    if( !Imp()->GetDrawView() ||
        0 == (pMrkList = &Imp()->GetDrawView()->GetMarkedObjectList()) ||
        1 != pMrkList->GetMarkCount())
        return sal_False;
    SwFrm* pOld;
    SwFlyFrm* pFly = NULL;
    SdrObject *pObj = pMrkList->GetMark( 0 )->GetMarkedSdrObj();
    if( pObj->ISA(SwVirtFlyDrawObj) )
    {
        pFly = ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm();
        pOld = pFly->AnchorFrm();
    }
    else
        pOld = ((SwDrawContact*)GetUserCall(pObj))->GetAnchorFrm( pObj );
    sal_Bool bRet = sal_False;
    if( pOld )
    {
        SwFrm* pNew = pOld;
        // #i28701#
        SwAnchoredObject* pAnchoredObj = ::GetUserCall( pObj )->GetAnchoredObj( pObj );
        SwFrmFmt& rFmt = pAnchoredObj->GetFrmFmt();
        SwFmtAnchor aAnch( rFmt.GetAnchor() );
        RndStdIds nAnchorId = aAnch.GetAnchorId();
        if ( FLY_AS_CHAR == nAnchorId )
            return sal_False;
        if( pOld->IsVertical() )
        {
            if( pOld->IsTxtFrm() )
            {
                switch( nDir ) {
                    case SW_MOVE_UP: nDir = SW_MOVE_LEFT; break;
                    case SW_MOVE_DOWN: nDir = SW_MOVE_RIGHT; break;
                    case SW_MOVE_LEFT: nDir = SW_MOVE_DOWN; break;
                    case SW_MOVE_RIGHT: nDir = SW_MOVE_UP; break;
                }
                if( pOld->IsRightToLeft() )
                {
                    if( nDir == SW_MOVE_LEFT )
                        nDir = SW_MOVE_RIGHT;
                    else if( nDir == SW_MOVE_RIGHT )
                        nDir = SW_MOVE_LEFT;
                }
            }
        }
        switch ( nAnchorId ) {
            case FLY_AT_PAGE:
            {
                OSL_ENSURE( pOld->IsPageFrm(), "Wrong anchor, page expected." );
                if( SW_MOVE_UP == nDir )
                    pNew = pOld->GetPrev();
                else if( SW_MOVE_DOWN == nDir )
                    pNew = pOld->GetNext();
                if( pNew && pNew != pOld )
                {
                    aAnch.SetPageNum( ((SwPageFrm*)pNew)->GetPhyPageNum() );
                    bRet = sal_True;
                }
                break;
            }
            case FLY_AT_CHAR:
            {
                OSL_ENSURE( pOld->IsCntntFrm(), "Wrong anchor, page expected." );
                if( SW_MOVE_LEFT == nDir || SW_MOVE_RIGHT == nDir )
                {
                    SwPosition *pPos = (SwPosition*)aAnch.GetCntntAnchor();
                    SwTxtNode* pTxtNd = ((SwTxtFrm*)pOld)->GetTxtNode();
                    xub_StrLen nAct = pPos->nContent.GetIndex();
                    if( SW_MOVE_LEFT == nDir )
                    {
                        bRet = sal_True;
                        if( nAct )
                        {
                            --nAct;
                            pPos->nContent.Assign( pTxtNd, nAct );
                        }
                        else
                            nDir = SW_MOVE_UP;
                    }
                    else
                    {
                        xub_StrLen nMax =
                            ((SwTxtFrm*)pOld)->GetTxtNode()->GetTxt().Len();
                        if( nAct < nMax )
                        {
                            ++nAct;
                            bRet = sal_True;
                            pPos->nContent.Assign( pTxtNd, nAct );
                        }
                        else
                            nDir = SW_MOVE_DOWN;
                    }
                }
            } // no break!
            case FLY_AT_PARA:
            {
                OSL_ENSURE( pOld->IsCntntFrm(), "Wrong anchor, page expected." );
                if( SW_MOVE_UP == nDir )
                    pNew = pOld->FindPrev();
                else if( SW_MOVE_DOWN == nDir )
                    pNew = pOld->FindNext();
                if( pNew && pNew != pOld && pNew->IsCntntFrm() )
                {
                    SwPosition *pPos = (SwPosition*)aAnch.GetCntntAnchor();
                    SwTxtNode* pTxtNd = ((SwTxtFrm*)pNew)->GetTxtNode();
                    pPos->nNode = *pTxtNd;
                    xub_StrLen nTmp = 0;
                    if( bRet )
                    {
                        nTmp = ((SwTxtFrm*)pNew)->GetTxtNode()->GetTxt().Len();
                        if( nTmp )
                            --nTmp;
                    }
                    pPos->nContent.Assign( pTxtNd, nTmp );
                    bRet = sal_True;
                }
                else if( SW_MOVE_UP == nDir || SW_MOVE_DOWN == nDir )
                    bRet = sal_False;
                break;
            }
            case FLY_AT_FLY:
            {
                OSL_ENSURE( pOld->IsFlyFrm(), "Wrong anchor, fly frame expected.");
                SwPageFrm* pPage = pOld->FindPageFrm();
                OSL_ENSURE( pPage, "Where's my page?" );
                SwFlyFrm* pNewFly = NULL;
                if( pPage->GetSortedObjs() )
                {
                    int i;
                    sal_Bool bOld = sal_False;
                    Point aCenter( pOld->Frm().Left() + pOld->Frm().Width()/2,
                                   pOld->Frm().Top() + pOld->Frm().Height()/2 );
                    Point aBest;
                    for( i = 0; (sal_uInt16)i<pPage->GetSortedObjs()->Count(); ++i )
                    {
                        SwAnchoredObject* pAnchObj =
                                                (*pPage->GetSortedObjs())[i];
                        if( pAnchObj->ISA(SwFlyFrm) )
                        {
                            SwFlyFrm* pTmp = static_cast<SwFlyFrm*>(pAnchObj);
                            if( pTmp == pOld )
                                bOld = sal_True;
                            else
                            {
                                const SwFlyFrm* pCheck = pFly ? pTmp : 0;
                                while( pCheck )
                                {
                                    if( pCheck == pFly )
                                        break;
                                    const SwFrm *pNxt = pCheck->GetAnchorFrm();
                                    pCheck = pNxt ? pNxt->FindFlyFrm() : NULL;
                                }
                                if( pCheck || pTmp->IsProtected() )
                                    continue;
                                Point aNew( pTmp->Frm().Left() +
                                            pTmp->Frm().Width()/2,
                                            pTmp->Frm().Top() +
                                            pTmp->Frm().Height()/2 );
                                sal_Bool bAccept = sal_False;
                                switch( nDir ) {
                                    case SW_MOVE_RIGHT:
                                    {
                                        bAccept = LESS_X( aCenter, aNew, bOld )
                                             && ( !pNewFly ||
                                             LESS_X( aNew, aBest, sal_False ) );
                                        break;
                                    }
                                    case SW_MOVE_LEFT:
                                    {
                                        bAccept = LESS_X( aNew, aCenter, !bOld )
                                             && ( !pNewFly ||
                                             LESS_X( aBest, aNew, sal_True ) );
                                        break;
                                    }
                                    case SW_MOVE_UP:
                                    {
                                        bAccept = LESS_Y( aNew, aCenter, !bOld )
                                             && ( !pNewFly ||
                                             LESS_Y( aBest, aNew, sal_True ) );
                                        break;
                                    }
                                    case SW_MOVE_DOWN:
                                    {
                                        bAccept = LESS_Y( aCenter, aNew, bOld )
                                             && ( !pNewFly ||
                                             LESS_Y( aNew, aBest, sal_False ) );
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
                    SwPosition aPos( *pNewFly->GetFmt()->
                                        GetCntnt().GetCntntIdx());
                    aAnch.SetAnchor( &aPos );
                    bRet = sal_True;
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
                SwHandleAnchorNodeChg* pHandleAnchorNodeChg( 0L );
                SwFlyFrmFmt* pFlyFrmFmt( dynamic_cast<SwFlyFrmFmt*>(&rFmt) );
                if ( pFlyFrmFmt )
                {
                    pHandleAnchorNodeChg =
                        new SwHandleAnchorNodeChg( *pFlyFrmFmt, aAnch );
                }
                rFmt.GetDoc()->SetAttr( aAnch, rFmt );
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

/*************************************************************************
|*
|*  SwFEShell::GetSelFrmType()
|*
*************************************************************************/

const SdrMarkList* SwFEShell::_GetMarkList() const
{
    const SdrMarkList* pMarkList = NULL;
    if( Imp()->GetDrawView() != NULL )
        pMarkList = &Imp()->GetDrawView()->GetMarkedObjectList();
    return pMarkList;
}

sal_uInt16 SwFEShell::GetSelFrmType() const
{
    sal_uInt16 eType;

    // get marked frame list, and check if anything is selected
    const SdrMarkList* pMarkList = _GetMarkList();
    if( pMarkList == NULL  ||  pMarkList->GetMarkCount() == 0 )
        eType = FRMTYPE_NONE;
    else
    {
        // obtain marked item as fly frame; if no fly frame, it must
        // be a draw object
        const SwFlyFrm* pFly = ::GetFlyFromMarked(pMarkList, (ViewShell*)this);
        if ( pFly != NULL )
        {
            if( pFly->IsFlyLayFrm() )
                eType = FRMTYPE_FLY_FREE;
            else if( pFly->IsFlyAtCntFrm() )
                eType = FRMTYPE_FLY_ATCNT;
            else
            {
                OSL_ENSURE( pFly->IsFlyInCntFrm(), "New frametype?" );
                eType = FRMTYPE_FLY_INCNT;
            }
        }
        else
            eType = FRMTYPE_DRAWOBJ;
    }

    return eType;
}

// does the draw selection contain a control?
bool SwFEShell::IsSelContainsControl() const
{
    bool bRet = false;

    // basically, copy the mechanism from GetSelFrmType(), but call
    // CheckControl... if you get a drawing object
    const SdrMarkList* pMarkList = _GetMarkList();
    if( pMarkList != NULL  &&  pMarkList->GetMarkCount() == 1 )
    {
        // if we have one marked object, get the SdrObject and check
        // whether it contains a control
        const SdrObject* pSdrObject = pMarkList->GetMark( 0 )->GetMarkedSdrObj();
        bRet = pSdrObject && ::CheckControlLayer( pSdrObject );
    }
    return bRet;
}

/*************************************************************************
|*
|*  SwFEShell::Scroll()
|*
*************************************************************************/

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

/*************************************************************************
|*
|*  SwFEShell::SetDragMode()
|*
*************************************************************************/

void SwFEShell::SetDragMode( sal_uInt16 eDragMode )
{
    if ( Imp()->HasDrawView() )
        Imp()->GetDrawView()->SetDragMode( (SdrDragMode)eDragMode );
}

/*************************************************************************
|*
|*  SwFEShell::BeginDrag()
|*
*************************************************************************/

long SwFEShell::BeginDrag( const Point* pPt, sal_Bool )
{
    SdrView *pView = Imp()->GetDrawView();
    if ( pView && pView->AreObjectsMarked() )
    {
        delete pChainFrom; delete pChainTo; pChainFrom = pChainTo = 0;
        SdrHdl* pHdl = pView->PickHandle( *pPt );
        pView->BegDragObj( *pPt, 0, pHdl );
        ::FrameNotify( this, FLY_DRAG );
        return 1;
    }
    return 0;
}
/*************************************************************************
|*
|*  SwFEShell::Drag()
|*
*************************************************************************/

long SwFEShell::Drag( const Point *pPt, sal_Bool )
{
    OSL_ENSURE( Imp()->HasDrawView(), "Drag without DrawView?" );
    if ( Imp()->GetDrawView()->IsDragObj() )
    {
        ScrollTo( *pPt );
        Imp()->GetDrawView()->MovDragObj( *pPt );
        Imp()->GetDrawView()->ShowDragAnchor();
        ::FrameNotify( this, FLY_DRAG );
        return 1;
    }
    return 0;
}

/*************************************************************************
|*
|*  SwFEShell::EndDrag()
|*
*************************************************************************/

long SwFEShell::EndDrag( const Point *, sal_Bool )
{
    OSL_ENSURE( Imp()->HasDrawView(), "EndDrag without DrawView?" );
    SdrView *pView = Imp()->GetDrawView();
    if ( pView->IsDragObj() )
    {
        // Setup Start-/EndActions only to the ViewShell
        ViewShell *pSh = this;
        do {
            pSh->StartAction();
        } while ( this != (pSh = (ViewShell*)pSh->GetNext()) );

        StartUndo( UNDO_START );

        // #50778# Bug during dragging: In StartAction a HideShowXor is called.
        // In EndDragObj() this is reversed, for no reason and even wrong.
        // To restore consistancy we should bring up the Xor again.

        // Reanimation from the hack #50778 to fix bug #97057
        // May be not the best solution, but the one with lowest risc at the moment.
        // pView->ShowShownXor( GetOut() );

        pView->EndDragObj();

        // DrawUndo on to flyframes are not stored
        //             The flys change the flag.
        GetDoc()->GetIDocumentUndoRedo().DoDrawUndo(true);
        ChgAnchor( 0, sal_True );

        EndUndo( UNDO_END );

        do {
            pSh->EndAction();
            if( pSh->IsA( TYPE( SwCrsrShell ) ) )
                ((SwCrsrShell*)pSh)->CallChgLnk();
        } while ( this != (pSh = (ViewShell*)pSh->GetNext()) );

        GetDoc()->SetModified();
        ::FrameNotify( this, FLY_DRAG );
        return 1;
    }
    return 0;
}

/*************************************************************************
|*
|*  SwFEShell::BreakDrag()
|*
*************************************************************************/

void SwFEShell::BreakDrag()
{
    OSL_ENSURE( Imp()->HasDrawView(), "BreakDrag without DrawView?" );
    if ( Imp()->GetDrawView()->IsDragObj() )
        Imp()->GetDrawView()->BrkDragObj();
    SetChainMarker();
}

/*************************************************************************
|*
|*  SwFEShell::SelFlyGrabCrsr()
|*
|*  Description         If a fly is selected, pulls the crsr in
|*                      the first CntntFrm
*************************************************************************/

const SwFrmFmt* SwFEShell::SelFlyGrabCrsr()
{
    if ( Imp()->HasDrawView() )
    {
        const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkedObjectList();
        SwFlyFrm *pFly = ::GetFlyFromMarked( &rMrkList, this );

        if( pFly )
        {
            SwCntntFrm *pCFrm = pFly->ContainsCntnt();
            if ( pCFrm )
            {
                SwCntntNode *pCNode = pCFrm->GetNode();
                // --> assure, that the cursor is consistent.
                KillPams();
                ClearMark();
                SwPaM       *pCrsr  = GetCrsr();

                pCrsr->GetPoint()->nNode = *pCNode;
                pCrsr->GetPoint()->nContent.Assign( pCNode, 0 );

                SwRect& rChrRect = (SwRect&)GetCharRect();
                rChrRect = pFly->Prt();
                rChrRect.Pos() += pFly->Frm().Pos();
                GetCrsrDocPos() = rChrRect.Pos();
            }
            return pFly->GetFmt();
        }
    }
    return 0;
}


/*************************************************************************
|*
|*  SwFEShell::SelectionToTop(), SelectionToBottom()
|*
|*  Description        Selection to above/below (Z-Order)
|*
*************************************************************************/

static void lcl_NotifyNeighbours( const SdrMarkList *pLst )
{
    // Rules for evasion have changed.
    // 1. The environment of the fly and everything inside should be notified
    // 2. The content of the frame itself has to be notified
    // 3. Frames displaced by the frame have to be notified
    // 4. Also Drawing objects can displace frames
    for( sal_uInt16 j = 0; j < pLst->GetMarkCount(); ++j )
    {
        SwPageFrm *pPage;
        sal_Bool bCheckNeighbours = sal_False;
        sal_Int16 aHori = text::HoriOrientation::NONE;
        SwRect aRect;
        SdrObject *pO = pLst->GetMark( 0 )->GetMarkedSdrObj();
        if ( pO->ISA(SwVirtFlyDrawObj) )
        {
            SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pO)->GetFlyFrm();

            const SwFmtHoriOrient &rHori = pFly->GetFmt()->GetHoriOrient();
            aHori = rHori.GetHoriOrient();
            if( text::HoriOrientation::NONE != aHori && text::HoriOrientation::CENTER != aHori &&
                pFly->IsFlyAtCntFrm() )
            {
                bCheckNeighbours = sal_True;
                pFly->InvalidatePos();
                pFly->Frm().Pos().Y() += 1;
            }

            pPage = pFly->FindPageFrm();
            aRect = pFly->Frm();
        }
        else
        {
            SwFrm* pAnch = ( (SwDrawContact*)GetUserCall(pO) )->GetAnchorFrm( pO );
            if( !pAnch )
                continue;
            pPage = pAnch->FindPageFrm();
            // #i68520# - naming changed
            aRect = GetBoundRectOfAnchoredObj( pO );
        }

        sal_uInt32 nCount = pPage->GetSortedObjs() ? pPage->GetSortedObjs()->Count() : 0;
        for ( sal_uInt32 i = 0; i < nCount; ++i )
        {
            SwAnchoredObject* pAnchoredObj = (*pPage->GetSortedObjs())[i];
            if ( !pAnchoredObj->ISA(SwFlyFrm) )
                continue;

            SwFlyFrm* pAct = static_cast<SwFlyFrm*>(pAnchoredObj);
            SwRect aTmpCalcPnt( pAct->Prt() );
            aTmpCalcPnt += pAct->Frm().Pos();
            if ( aRect.IsOver( aTmpCalcPnt ) )
            {
                SwCntntFrm *pCnt = pAct->ContainsCntnt();
                while ( pCnt )
                {
                    aTmpCalcPnt = pCnt->Prt();
                    aTmpCalcPnt += pCnt->Frm().Pos();
                    if ( aRect.IsOver( aTmpCalcPnt ) )
                        ((SwFrm*)pCnt)->Prepare( PREP_FLY_ATTR_CHG );
                    pCnt = pCnt->GetNextCntntFrm();
                }
            }
            if ( bCheckNeighbours && pAct->IsFlyAtCntFrm() )
            {
                const SwFmtHoriOrient &rH = pAct->GetFmt()->GetHoriOrient();
                if ( rH.GetHoriOrient() == aHori &&
                     pAct->Frm().Top()    <= aRect.Bottom() &&
                     pAct->Frm().Bottom() >= aRect.Top() )
                {
                    pAct->InvalidatePos();
                    pAct->Frm().Pos().Y() += 1;
                }
            }
        }
    }
}

void SwFEShell::SelectionToTop( sal_Bool bTop )
{
    OSL_ENSURE( Imp()->HasDrawView(), "SelectionToTop without DrawView?" );
    const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkedObjectList();
    OSL_ENSURE( rMrkList.GetMarkCount(), "No object selected." );

    SwFlyFrm *pFly = ::GetFlyFromMarked( &rMrkList, this );
    if ( pFly && pFly->IsFlyInCntFrm() )
        return;

    StartAllAction();
    if ( bTop )
        Imp()->GetDrawView()->PutMarkedToTop();
    else
        Imp()->GetDrawView()->MovMarkedToTop();
    ::lcl_NotifyNeighbours( &rMrkList );
    GetDoc()->SetModified();
    EndAllAction();
}

void SwFEShell::SelectionToBottom( sal_Bool bBottom )
{
    OSL_ENSURE( Imp()->HasDrawView(), "SelectionToBottom without DrawView?" );
    const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkedObjectList();
    OSL_ENSURE( rMrkList.GetMarkCount(), "No object selected." );

    SwFlyFrm *pFly = ::GetFlyFromMarked( &rMrkList, this );
    if ( pFly && pFly->IsFlyInCntFrm() )
        return;

    StartAllAction();
    if ( bBottom )
        Imp()->GetDrawView()->PutMarkedToBtm();
    else
        Imp()->GetDrawView()->MovMarkedToBtm();
    ::lcl_NotifyNeighbours( &rMrkList );
    GetDoc()->SetModified();
    EndAllAction();
}

/*************************************************************************
|*
|*  SwFEShell::GetLayerId()
|*
|*  Description         Object above/below the document?
|*                      2 Controls, 1 Heaven, 0 Hell, -1 Ambiguous
*************************************************************************/

short SwFEShell::GetLayerId() const
{
    short nRet = SHRT_MAX;
    if ( Imp()->HasDrawView() )
    {
        const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkedObjectList();
        for ( sal_uInt16 i = 0; i < rMrkList.GetMarkCount(); ++i )
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

/*************************************************************************
|*
|*  SwFEShell::SelectionToHeaven(), SelectionToHell()
|*
|*  Description         Object above/below the document
|*
*************************************************************************/
// Note: only visible objects can be marked. Thus, objects with invisible
//       layer IDs have not to be considered.
//       If <SwFEShell> exists, layout exists!!
void SwFEShell::ChangeOpaque( SdrLayerID nLayerId )
{
    if ( Imp()->HasDrawView() )
    {
        const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkedObjectList();
        const IDocumentDrawModelAccess* pIDDMA = getIDocumentDrawModelAccess();
        // correct type of <nControls>
        for ( sal_uInt16 i = 0; i < rMrkList.GetMarkCount(); ++i )
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
                if ( pObj->ISA(SwVirtFlyDrawObj) )
                {
                    SwFmt *pFmt = ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm()->GetFmt();
                    SvxOpaqueItem aOpa( pFmt->GetOpaque() );
                    aOpa.SetValue(  nLayerId == pIDDMA->GetHellId() );
                    pFmt->SetFmtAttr( aOpa );
                }
            }
        }
        GetDoc()->SetModified();
    }
}

void SwFEShell::SelectionToHeaven()
{
    ChangeOpaque( getIDocumentDrawModelAccess()->GetHeavenId() );
}

void SwFEShell::SelectionToHell()
{
    ChangeOpaque( getIDocumentDrawModelAccess()->GetHellId() );
}

/*************************************************************************
|*
|*  SwFEShell::IsObjSelected(), IsFrmSelected()
|*
*************************************************************************/

sal_uInt16 SwFEShell::IsObjSelected() const
{
    if ( IsFrmSelected() || !Imp()->HasDrawView() )
        return 0;
    else
        return sal_uInt16( Imp()->GetDrawView()->GetMarkedObjectList().GetMarkCount() );
}

sal_Bool SwFEShell::IsFrmSelected() const
{
    if ( !Imp()->HasDrawView() )
        return sal_False;
    else
        return 0 != ::GetFlyFromMarked( &Imp()->GetDrawView()->GetMarkedObjectList(),
                                        (ViewShell*)this );
}

sal_Bool SwFEShell::IsObjSelected( const SdrObject& rObj ) const
{
    if ( IsFrmSelected() || !Imp()->HasDrawView() )
        return sal_False;
    else
        return Imp()->GetDrawView()
                    ->IsObjMarked( const_cast< SdrObject * >( &rObj ) );
}

/*************************************************************************
|*
|*  SwFEShell::EndTextEdit()
|*
*************************************************************************/

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
    if( 0 != ( pUserCall = GetUserCall(pObj) ) )
    {
        SdrObject *pTmp = ((SwContact*)pUserCall)->GetMaster();
        if( !pTmp )
            pTmp = pObj;
        pUserCall->Changed( *pTmp, SDRUSERCALL_RESIZE, pTmp->GetLastBoundRect() );
    }
    if ( !pObj->GetUpGroup() )
    {
        if ( SDRENDTEXTEDIT_SHOULDBEDELETED == pView->SdrEndTextEdit(sal_True) )
        {
            if ( pView->GetMarkedObjectList().GetMarkCount() > 1 )
            {
                {
                    SdrMarkList aSave( pView->GetMarkedObjectList() );
                    aSave.DeleteMark( aSave.FindObject( pObj ) );
                    if ( aSave.GetMarkCount() )
                    {
                        pView->UnmarkAll();
                        pView->MarkObj( pObj, Imp()->GetPageView() );
                    }
                    DelSelectedObj();
                    if ( aSave.GetMarkCount() )
                    {
                        for ( sal_uInt16 i = 0; i < aSave.GetMarkCount(); ++i )
                            pView->MarkObj( aSave.GetMark( i )->GetMarkedSdrObj(),
                                            Imp()->GetPageView() );
                    }
                }
            }
            else
                DelSelectedObj();
        }
    }
    else
        pView->SdrEndTextEdit();
    EndAllAction();
}

/*************************************************************************
|*
|*  SwFEShell::IsInsideSelectedObj()
|*
*************************************************************************/

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

/*************************************************************************
|*
|*  SwFEShell::IsObjSelectable()
|*
*************************************************************************/

bool SwFEShell::IsObjSelectable( const Point& rPt )
{
    SET_CURR_SHELL(this);
    SwDrawView *pDView = Imp()->GetDrawView();
    bool bRet = false;
    if( pDView )
    {
        SdrObject* pObj;
        SdrPageView* pPV;
        sal_uInt16 nOld = pDView->GetHitTolerancePixel();
        pDView->SetHitTolerancePixel( pDView->GetMarkHdlSizePixel()/2 );

        bRet = 0 != pDView->PickObj( rPt, pDView->getHitTolLog(), pObj, pPV, SDRSEARCH_PICKMARKABLE );
        pDView->SetHitTolerancePixel( nOld );
    }
    return bRet;
}

SdrObject* SwFEShell::GetObjAt( const Point& rPt )
{
    SdrObject* pRet = 0;
    SET_CURR_SHELL(this);
    SwDrawView *pDView = Imp()->GetDrawView();
    if( pDView )
    {
        SdrPageView* pPV;
        sal_uInt16 nOld = pDView->GetHitTolerancePixel();
        pDView->SetHitTolerancePixel( pDView->GetMarkHdlSizePixel()/2 );

        pDView->PickObj( rPt, pDView->getHitTolLog(), pRet, pPV, SDRSEARCH_PICKMARKABLE );
        pDView->SetHitTolerancePixel( nOld );
    }
    return pRet;
}

// Test if there is a object at that position and if it should be selected.
sal_Bool SwFEShell::ShouldObjectBeSelected(const Point& rPt)
{
    SET_CURR_SHELL(this);
    SwDrawView *pDrawView = Imp()->GetDrawView();
    sal_Bool bRet(sal_False);

    if(pDrawView)
    {
        SdrObject* pObj;
        SdrPageView* pPV;
        sal_uInt16 nOld(pDrawView->GetHitTolerancePixel());

        pDrawView->SetHitTolerancePixel(pDrawView->GetMarkHdlSizePixel()/2);
        bRet = pDrawView->PickObj(rPt, pDrawView->getHitTolLog(), pObj, pPV, SDRSEARCH_PICKMARKABLE);
        pDrawView->SetHitTolerancePixel(nOld);

        if ( bRet && pObj )
        {
            const IDocumentDrawModelAccess* pIDDMA = getIDocumentDrawModelAccess();
            // #i89920#
            // Do not select object in background which is overlapping this text
            // at the given position.
            bool bObjInBackground( false );
            {
                if ( pObj->GetLayer() == pIDDMA->GetHellId() )
                {
                    const SwAnchoredObject* pAnchoredObj = ::GetUserCall( pObj )->GetAnchoredObj( pObj );
                    const SwFrmFmt& rFmt = pAnchoredObj->GetFrmFmt();
                    const SwFmtSurround& rSurround = rFmt.GetSurround();
                    if ( rSurround.GetSurround() == SURROUND_THROUGHT )
                    {
                        bObjInBackground = true;
                    }
                }
            }
            if ( bObjInBackground )
            {
                const SwPageFrm* pPageFrm = GetLayout()->GetPageAtPos( rPt );
                if( pPageFrm )
                {
                    const SwCntntFrm* pCntntFrm( pPageFrm->ContainsCntnt() );
                    while ( pCntntFrm )
                    {
                        if ( pCntntFrm->UnionFrm().IsInside( rPt ) )
                        {
                            const SwTxtFrm* pTxtFrm =
                                    dynamic_cast<const SwTxtFrm*>(pCntntFrm);
                            if ( pTxtFrm )
                            {
                                SwPosition aPos( *(pTxtFrm->GetTxtNode()) );
                                Point aTmpPt( rPt );
                                if (pTxtFrm->GetKeyCrsrOfst(&aPos, aTmpPt))
                                {
                                    SwRect aCursorCharRect;
                                    if (pTxtFrm->GetCharRect(aCursorCharRect,
                                                aPos))
                                    {
                                        if ( aCursorCharRect.IsOver( SwRect( pObj->GetLastBoundRect() ) ) )
                                        {
                                            bRet = sal_False;
                                        }
                                    }
                                }
                            }
                            else
                            {
                                bRet = sal_False;
                            }
                            break;
                        }

                        pCntntFrm = pCntntFrm->GetNextCntntFrm();
                    }
                }
            }

            // Don't select header / footer objects in body edition and vice-versa
            SwContact* pContact = static_cast<SwContact*>(pObj->GetUserCall());
            if ( !pContact->ObjAnchoredAtPage() )
            {
                const SwPosition& rPos = pContact->GetCntntAnchor();
                bool bInHdrFtr = GetDoc()->IsInHeaderFooter( rPos.nNode );
                if ( ( IsHeaderFooterEdit() && !bInHdrFtr ) ||
                     ( !IsHeaderFooterEdit() && bInHdrFtr ) )
                {
                    bRet = sal_False;
                }
            }

            if ( bRet )
            {
                const SdrPage* pPage = pIDDMA->GetDrawModel()->GetPage(0);
                for(sal_uInt32 a(pObj->GetOrdNum() + 1); bRet && a < pPage->GetObjCount(); a++)
                {
                    SdrObject *pCandidate = pPage->GetObj(a);

                    if (pCandidate->ISA(SwVirtFlyDrawObj) &&
                       ( (SwVirtFlyDrawObj*)pCandidate)->GetCurrentBoundRect().IsInside(rPt) )
                    {
                        bRet = sal_False;
                    }
                }
            }
        }
    }

    return bRet;
}

/*************************************************************************
|*
|*  SwFEShell::GotoObj()
|*
|*  Description        If an object was selected, we assume its upper-left corner
|*                     otherwise the middle of the current CharRects.
|*
*************************************************************************/
/* --------------------------------------------------
 * Does the object include a control or groups,
 * which comprise only controls
 * --------------------------------------------------*/
static sal_Bool lcl_IsControlGroup( const SdrObject *pObj )
{
    sal_Bool bRet = sal_False;
    if(pObj->ISA(SdrUnoObj))
        bRet = sal_True;
    else if( pObj->ISA( SdrObjGroup ) )
    {
        bRet = sal_True;
        const SdrObjList *pLst = ((SdrObjGroup*)pObj)->GetSubList();
        for ( sal_uInt16 i = 0; i < pLst->GetObjCount(); ++i )
            if( !::lcl_IsControlGroup( pLst->GetObj( i ) ) )
                return sal_False;
    }
    return bRet;
}

namespace
{
    class MarkableObjectsOnly : public ::svx::ISdrObjectFilter
    {
    public:
        MarkableObjectsOnly( SdrPageView* i_pPV )
            :m_pPV( i_pPV )
        {
        }

        virtual bool    includeObject( const SdrObject& i_rObject ) const
        {
            return m_pPV && m_pPV->GetView().IsObjMarkable( const_cast< SdrObject* >( &i_rObject ), m_pPV );
        }

    private:
        SdrPageView*    m_pPV;
    };
}

const SdrObject* SwFEShell::GetBestObject( sal_Bool bNext, sal_uInt16 /*GOTOOBJ_...*/ eType, sal_Bool bFlat, const ::svx::ISdrObjectFilter* pFilter )
{
    if( !Imp()->HasDrawView() )
        return NULL;

    const SdrObject *pBest  = 0,
                    *pTop   = 0;

    const long nTmp = bNext ? LONG_MAX : 0;
    Point aBestPos( nTmp, nTmp );
    Point aTopPos(  nTmp, nTmp );
    Point aCurPos;
    Point aPos;
    sal_Bool bNoDraw = 0 == (GOTOOBJ_DRAW_ANY & eType);
    sal_Bool bNoFly = 0 == (GOTOOBJ_FLY_ANY & eType);

    if( !bNoFly && bNoDraw )
    {
        SwFlyFrm *pFly = GetCurrFrm( sal_False )->FindFlyFrm();
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
        SdrObjList* pList = NULL;
        if ( rMrkList.GetMarkCount() )
        {
            const SdrObject* pStartObj = rMrkList.GetMark(0)->GetMarkedSdrObj();
            if( pStartObj->ISA(SwVirtFlyDrawObj) )
                aPos = ((SwVirtFlyDrawObj*)pStartObj)->GetFlyFrm()->Frm().Pos();
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
            const SdrObject* pStartObj = pPV ? pPV->GetAktGroup() : 0;
            if ( pStartObj && pStartObj->ISA( SdrObjGroup ) )
                pList = pStartObj->GetSubList();
        }

        if ( ! pList )
        {
            // Here we are if
            // A  No object has been selected and no group has been entered or
            // B  An object has been selected and it is not inside a group
            pList = getIDocumentDrawModelAccess()->GetDrawModel()->GetPage( 0 );
        }


        OSL_ENSURE( pList, "No object list to iterate" );

        SdrObjListIter aObjIter( *pList, bFlat ? IM_FLAT : IM_DEEPNOGROUPS );
        while ( aObjIter.IsMore() )
        {
            SdrObject* pObj = aObjIter.Next();
            sal_Bool bFlyFrm = pObj->ISA(SwVirtFlyDrawObj);
            if( ( bNoFly && bFlyFrm ) ||
                ( bNoDraw && !bFlyFrm ) ||
                ( eType == GOTOOBJ_DRAW_SIMPLE && lcl_IsControlGroup( pObj ) ) ||
                ( eType == GOTOOBJ_DRAW_CONTROL && !lcl_IsControlGroup( pObj ) ) ||
                ( pFilter && !pFilter->includeObject( *pObj ) ) )
                continue;
            if( bFlyFrm )
            {
                SwVirtFlyDrawObj *pO = (SwVirtFlyDrawObj*)pObj;
                SwFlyFrm *pFly = pO->GetFlyFrm();
                if( GOTOOBJ_FLY_ANY != ( GOTOOBJ_FLY_ANY & eType ) )
                {
                    switch ( eType )
                    {
                        case GOTOOBJ_FLY_FRM:
                            if ( pFly->Lower() && pFly->Lower()->IsNoTxtFrm() )
                                continue;
                        break;
                        case GOTOOBJ_FLY_GRF:
                            if ( pFly->Lower() &&
                                (pFly->Lower()->IsLayoutFrm() ||
                                !((SwCntntFrm*)pFly->Lower())->GetNode()->GetGrfNode()))
                                continue;
                        break;
                        case GOTOOBJ_FLY_OLE:
                            if ( pFly->Lower() &&
                                (pFly->Lower()->IsLayoutFrm() ||
                                !((SwCntntFrm*)pFly->Lower())->GetNode()->GetOLENode()))
                                continue;
                        break;
                    }
                }
                aCurPos = pFly->Frm().Pos();
            }
            else
                aCurPos = pObj->GetCurrentBoundRect().TopLeft();

            // Special case if another object is on same Y.
            if( aCurPos != aPos &&          // only when it is not me
                aCurPos.Y() == aPos.Y() &&  // Y positions equal
                (bNext? (aCurPos.X() > aPos.X()) :  // lies next to me
                        (aCurPos.X() < aPos.X())) ) // " reverse
            {
                aBestPos = Point( nTmp, nTmp );
                SdrObjListIter aTmpIter( *pList, bFlat ? IM_FLAT : IM_DEEPNOGROUPS );
                while ( aTmpIter.IsMore() )
                {
                    SdrObject* pTmpObj = aTmpIter.Next();
                    bFlyFrm = pTmpObj->ISA(SwVirtFlyDrawObj);
                    if( ( bNoFly && bFlyFrm ) || ( bNoDraw && !bFlyFrm ) )
                        continue;
                    if( bFlyFrm )
                    {
                        SwVirtFlyDrawObj *pO = (SwVirtFlyDrawObj*)pTmpObj;
                        aCurPos = pO->GetFlyFrm()->Frm().Pos();
                    }
                    else
                        aCurPos = pTmpObj->GetCurrentBoundRect().TopLeft();

                    if( aCurPos != aPos && aCurPos.Y() == aPos.Y() &&
                        (bNext? (aCurPos.X() > aPos.X()) :  // lies next to me
                                (aCurPos.X() < aPos.X())) &&    // " reverse
                        (bNext? (aCurPos.X() < aBestPos.X()) :  // better as best
                                (aCurPos.X() > aBestPos.X())) ) // " reverse
                    {
                        aBestPos = aCurPos;
                        pBest = pTmpObj;
                    }
                }
                break;
            }

            if( (
                (bNext? (aPos.Y() < aCurPos.Y()) :          // only below me
                        (aPos.Y() > aCurPos.Y())) &&        // " reverse
                (bNext? (aBestPos.Y() > aCurPos.Y()) :      // closer below
                        (aBestPos.Y() < aCurPos.Y()))
                    ) ||    // " reverse
                        (aBestPos.Y() == aCurPos.Y() &&
                (bNext? (aBestPos.X() > aCurPos.X()) :      // further left
                        (aBestPos.X() < aCurPos.X()))))     // " reverse

            {
                aBestPos = aCurPos;
                pBest = pObj;
            }

            if( (bNext? (aTopPos.Y() > aCurPos.Y()) :       // higher as best
                        (aTopPos.Y() < aCurPos.Y())) ||     // " reverse
                        (aTopPos.Y() == aCurPos.Y() &&
                (bNext? (aTopPos.X() > aCurPos.X()) :       // further left
                        (aTopPos.X() < aCurPos.X()))))      // " reverse
            {
                aTopPos = aCurPos;
                pTop = pObj;
            }
        }
        // unfortunately nothing found
        if( (bNext? (aBestPos.X() == LONG_MAX) : (aBestPos.X() == 0)) )
            pBest = pTop;
    }

    return pBest;
}

sal_Bool SwFEShell::GotoObj( sal_Bool bNext, sal_uInt16 /*GOTOOBJ_...*/ eType )
{
    const SdrObject* pBest = GetBestObject( bNext, eType );

    if ( !pBest )
        return sal_False;

    sal_Bool bFlyFrm = pBest->ISA(SwVirtFlyDrawObj);
    if( bFlyFrm )
    {
        SwVirtFlyDrawObj *pO = (SwVirtFlyDrawObj*)pBest;
        const SwRect& rFrm = pO->GetFlyFrm()->Frm();
        SelectObj( rFrm.Pos(), 0, (SdrObject*)pBest );
        if( !ActionPend() )
            MakeVisible( rFrm );
    }
    else
    {
        SelectObj( Point(), 0, (SdrObject*)pBest );
        if( !ActionPend() )
            MakeVisible( pBest->GetCurrentBoundRect() );
    }
    CallChgLnk();
    return sal_True;
}

/*************************************************************************
|*
|*  SwFEShell::BeginCreate()
|*
*************************************************************************/

sal_Bool SwFEShell::BeginCreate( sal_uInt16 /*SdrObjKind ?*/  eSdrObjectKind, const Point &rPos )
{
    sal_Bool bRet = sal_False;

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

sal_Bool SwFEShell::BeginCreate( sal_uInt16 /*SdrObjKind ?*/  eSdrObjectKind, sal_uInt32 eObjInventor,
                             const Point &rPos )
{
    sal_Bool bRet = sal_False;

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

/*************************************************************************
|*
|*  SwFEShell::MoveCreate()
|*
*************************************************************************/

void SwFEShell::MoveCreate( const Point &rPos )
{
    OSL_ENSURE( Imp()->HasDrawView(), "MoveCreate without DrawView?" );
    if ( GetPageNumber( rPos ) )
    {
        ScrollTo( rPos );
        Imp()->GetDrawView()->MovCreateObj( rPos );
        ::FrameNotify( this, FLY_DRAG );
    }
}

/*************************************************************************
|*
|*  SwFEShell::EndCreate(), ImpEndCreate()
|*
*************************************************************************/

sal_Bool SwFEShell::EndCreate( sal_uInt16 eSdrCreateCmd )
{
    // To assure undo-object from the DrawEngine is not stored,
    // (we create our own undo-object!), temporarily switch-off Undo
    OSL_ENSURE( Imp()->HasDrawView(), "EndCreate without DrawView?" );
    if( !Imp()->GetDrawView()->IsGroupEntered() )
    {
        GetDoc()->GetIDocumentUndoRedo().DoDrawUndo(false);
    }
    sal_Bool bCreate = Imp()->GetDrawView()->EndCreateObj(
                                    SdrCreateCmd( eSdrCreateCmd ) );
    GetDoc()->GetIDocumentUndoRedo().DoDrawUndo(true);

    if ( !bCreate )
    {
        ::FrameNotify( this, FLY_DRAG_END );
        return sal_False;
    }

    if ( (SdrCreateCmd)eSdrCreateCmd == SDRCREATE_NEXTPOINT )
    {
        ::FrameNotify( this, FLY_DRAG );
        return sal_True;
    }
    return ImpEndCreate();
}


sal_Bool SwFEShell::ImpEndCreate()
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
        return sal_False;
    }

    if( rSdrObj.GetUpGroup() )
    {
        Point aTmpPos( rSdrObj.GetSnapRect().TopLeft() );
        Point aNewAnchor( rSdrObj.GetUpGroup()->GetAnchorPos() );
        // OD 2004-04-05 #i26791# - direct object positioning for group members
        rSdrObj.NbcSetRelativePos( aTmpPos - aNewAnchor );
        rSdrObj.NbcSetAnchorPos( aNewAnchor );
        ::FrameNotify( this, FLY_DRAG );
        return sal_True;
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
    SwFmtAnchor aAnch;
    const SwFrm *pAnch = 0;
    bool bCharBound = false;
    if( rSdrObj.ISA( SdrUnoObj ) )
    {
        SwPosition aPos( GetDoc()->GetNodes() );
        SwCrsrMoveState aState( MV_SETONLYTEXT );
        Point aPoint( aPt.X(), aPt.Y() + rBound.GetHeight()/2 );
        GetLayout()->GetCrsrOfst( &aPos, aPoint, &aState ); //swmod 080317

        // characterbinding not allowed in readonly-content
        if( !aPos.nNode.GetNode().IsProtect() )
        {
            pAnch = aPos.nNode.GetNode().GetCntntNode()->getLayoutFrm( GetLayout(), &aPoint, &aPos );
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
        const SwFrm* pPage = 0;
        SwCrsrMoveState aState( MV_SETONLYTEXT );
        Point aPoint( aPt );
        SwPosition aPos( GetDoc()->GetNodes() );
        GetLayout()->GetCrsrOfst( &aPos, aPoint, &aState );

        // do not set in ReadnOnly-content
        if( aPos.nNode.GetNode().IsProtect() )
            // then only page bound. Or should we
            // search the next not-readonly position?
            bAtPage = true;

        pAnch = aPos.nNode.GetNode().GetCntntNode()->getLayoutFrm( GetLayout(), &aPoint, 0, sal_False );

        if( !bAtPage )
        {
            const SwFlyFrm *pTmp = pAnch->FindFlyFrm();
            if( pTmp )
            {
                const SwFrm* pTmpFrm = pAnch;
                SwRect aBound( rBound );
                while( pTmp )
                {
                    if( pTmp->Frm().IsInside( aBound ) )
                    {
                        if( !bBodyOnly || !pTmp->FindFooterOrHeader() )
                            pPage = pTmpFrm;
                        break;
                    }
                    pTmp = pTmp->GetAnchorFrm()
                                ? pTmp->GetAnchorFrm()->FindFlyFrm()
                                : 0;
                    pTmpFrm = pTmp;
                }
            }

            if( !pPage )
                pPage = pAnch->FindPageFrm();

            // Always via FindAnchor, to assure the frame will be bound
            // to the previous. With GetCrsOfst we can also reach the next. THIS IS WRONG.
            pAnch = ::FindAnchor( pPage, aPt, bBodyOnly );
            aPos.nNode = *((SwCntntFrm*)pAnch)->GetNode();

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
            pPage = pAnch->FindPageFrm();

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
    SwTwips nYOffset = rBound.Top() - pAnch->Frm().Top();
    {
        if( pAnch->IsVertical() )
        {
            nXOffset = nYOffset;
            nYOffset = pAnch->Frm().Left()+pAnch->Frm().Width()-rBound.Right();
        }
        else if( pAnch->IsRightToLeft() )
            nXOffset = pAnch->Frm().Left()+pAnch->Frm().Width()-rBound.Right();
        else
            nXOffset = rBound.Left() - pAnch->Frm().Left();
        if( pAnch->IsTxtFrm() && ((SwTxtFrm*)pAnch)->IsFollow() )
        {
            SwTxtFrm* pTmp = (SwTxtFrm*)pAnch;
            do {
                pTmp = pTmp->FindMaster();
                OSL_ENSURE( pTmp, "Where's my Master?" );
                // OD 2004-03-30 #i26791# - correction: add frame area height
                // of master frames.
                nYOffset += pTmp->IsVertical() ?
                            pTmp->Frm().Width() : pTmp->Frm().Height();
            } while ( pTmp->IsFollow() );
        }
    }

    if( OBJ_NONE == nIdent )
    {
        // For OBJ_NONE a fly is inserted.
        const long nWidth = rBound.Right()  - rBound.Left();
        const long nHeight= rBound.Bottom() - rBound.Top();
        aSet.Put( SwFmtFrmSize( ATT_MIN_SIZE, Max( nWidth,  long(MINFLY) ),
                                              Max( nHeight, long(MINFLY) )));

        SwFmtHoriOrient aHori( nXOffset, text::HoriOrientation::NONE, text::RelOrientation::FRAME );
        SwFmtVertOrient aVert( nYOffset, text::VertOrientation::NONE, text::RelOrientation::FRAME );
        aSet.Put( SwFmtSurround( SURROUND_PARALLEL ) );
        aSet.Put( aHori );
        aSet.Put( aVert );

        // Quickly store the square
        const SwRect aFlyRect( rBound );

        // Throw away generated object, now the fly can nicely
        // via the available SS be generated.
        GetDoc()->GetIDocumentUndoRedo().DoDrawUndo(false); // see above
        // #i52858# - method name changed
        SdrPage *pPg = getIDocumentDrawModelAccess()->GetOrCreateDrawModel()->GetPage( 0 );
        if( !pPg )
        {
            SdrModel* pTmpSdrModel = getIDocumentDrawModelAccess()->GetDrawModel();
            pPg = pTmpSdrModel->AllocPage( sal_False );
            pTmpSdrModel->InsertPage( pPg );
        }
        pPg->RecalcObjOrdNums();
        SdrObject* pRemovedObject = pPg->RemoveObject( rSdrObj.GetOrdNumDirect() );
        SdrObject::Free( pRemovedObject );
        GetDoc()->GetIDocumentUndoRedo().DoDrawUndo(true);

        SwFlyFrm* pFlyFrm;
        if( NewFlyFrm( aSet, sal_True ) &&
            ::GetHtmlMode( GetDoc()->GetDocShell() ) &&
            0 != ( pFlyFrm = FindFlyFrm() ))
        {
            SfxItemSet aHtmlSet( GetDoc()->GetAttrPool(), RES_VERT_ORIENT, RES_HORI_ORIENT );
            // horizontal orientation:
            const sal_Bool bLeftFrm = aFlyRect.Left() <
                                      pAnch->Frm().Left() + pAnch->Prt().Left(),
                           bLeftPrt = aFlyRect.Left() + aFlyRect.Width() <
                                      pAnch->Frm().Left() + pAnch->Prt().Width()/2;
            if( bLeftFrm || bLeftPrt )
            {
                aHori.SetHoriOrient( text::HoriOrientation::LEFT );
                aHori.SetRelationOrient( bLeftFrm ? text::RelOrientation::FRAME : text::RelOrientation::PRINT_AREA );
            }
            else
            {
                const sal_Bool bRightFrm = aFlyRect.Left() >
                                           pAnch->Frm().Left() + pAnch->Prt().Width();
                aHori.SetHoriOrient( text::HoriOrientation::RIGHT );
                aHori.SetRelationOrient( bRightFrm ? text::RelOrientation::FRAME : text::RelOrientation::PRINT_AREA );
            }
            aHtmlSet.Put( aHori );
            aVert.SetVertOrient( text::VertOrientation::TOP );
            aVert.SetRelationOrient( text::RelOrientation::PRINT_AREA );
            aHtmlSet.Put( aVert );

            GetDoc()->SetAttr( aHtmlSet, *pFlyFrm->GetFmt() );
        }
    }
    else
    {
        Point aRelNullPt;
        if( OBJ_CAPTION == nIdent )
            aRelNullPt = ((SdrCaptionObj&)rSdrObj).GetTailPos();
        else
            aRelNullPt = rBound.TopLeft();

        aSet.Put( aAnch );
        aSet.Put( SwFmtSurround( SURROUND_THROUGHT ) );
        // OD 2004-03-30 #i26791# - set horizontal position
        SwFmtHoriOrient aHori( nXOffset, text::HoriOrientation::NONE, text::RelOrientation::FRAME );
        aSet.Put( aHori );
        // OD 2004-03-30 #i26791# - set vertical position
        if( pAnch->IsTxtFrm() && ((SwTxtFrm*)pAnch)->IsFollow() )
        {
            SwTxtFrm* pTmp = (SwTxtFrm*)pAnch;
            do {
                pTmp = pTmp->FindMaster();
                OSL_ENSURE( pTmp, "Where's my Master?" );
                nYOffset += pTmp->IsVertical() ?
                            pTmp->Prt().Width() : pTmp->Prt().Height();
            } while ( pTmp->IsFollow() );
        }
        SwFmtVertOrient aVert( nYOffset, text::VertOrientation::NONE, text::RelOrientation::FRAME );
        aSet.Put( aVert );
        SwDrawFrmFmt* pFmt = (SwDrawFrmFmt*)getIDocumentLayoutAccess()->MakeLayoutFmt( RND_DRAW_OBJECT, &aSet );
        // #i36010# - set layout direction of the position
        pFmt->SetPositionLayoutDir(
            text::PositionLayoutDir::PositionInLayoutDirOfAnchor );
        // #i44344#, #i44681# - positioning attributes already set
        pFmt->PosAttrSet();

        SwDrawContact *pContact = new SwDrawContact( pFmt, &rSdrObj );
        // #i35635#
        pContact->MoveObjToVisibleLayer( &rSdrObj );
        if( bCharBound )
        {
            OSL_ENSURE( aAnch.GetAnchorId() == FLY_AS_CHAR, "wrong AnchorType" );
            SwTxtNode *pNd = aAnch.GetCntntAnchor()->nNode.GetNode().GetTxtNode();
            SwFmtFlyCnt aFmt( pFmt );
            pNd->InsertItem(aFmt,
                            aAnch.GetCntntAnchor()->nContent.GetIndex(), 0 );
            SwFmtVertOrient aVertical( pFmt->GetVertOrient() );
            aVertical.SetVertOrient( text::VertOrientation::LINE_CENTER );
            pFmt->SetFmtAttr( aVertical );
        }
        if( pAnch->IsTxtFrm() && ((SwTxtFrm*)pAnch)->IsFollow() )
        {
            SwTxtFrm* pTmp = (SwTxtFrm*)pAnch;
            do {
                pTmp = pTmp->FindMaster();
                OSL_ENSURE( pTmp, "Where's my Master?" );
            } while( pTmp->IsFollow() );
            pAnch = pTmp;
        }

        pContact->ConnectToLayout();

        // mark object at frame the object is inserted at.
        {
            SdrObject* pMarkObj = pContact->GetDrawObjectByAnchorFrm( *pAnch );
            if ( pMarkObj )
            {
                Imp()->GetDrawView()->MarkObj( pMarkObj, Imp()->GetPageView(),
                                                sal_False, sal_False );
            }
            else
            {
                Imp()->GetDrawView()->MarkObj( &rSdrObj, Imp()->GetPageView(),
                                                sal_False, sal_False );
            }
        }
    }

    GetDoc()->SetModified();

    KillPams();
    EndAllActionAndCall();
    UnlockPaint();
    return sal_True;
}


/*************************************************************************
|*
|*  SwFEShell::BreakCreate()
|*
*************************************************************************/

void SwFEShell::BreakCreate()
{
    OSL_ENSURE( Imp()->HasDrawView(), "BreakCreate without DrawView?" );
    Imp()->GetDrawView()->BrkCreateObj();
    ::FrameNotify( this, FLY_DRAG_END );
}

/*************************************************************************
|*
|*  SwFEShell::IsDrawCreate()
|*
*************************************************************************/

sal_Bool SwFEShell::IsDrawCreate() const
{
    return Imp()->HasDrawView() ? Imp()->GetDrawView()->IsCreateObj() : sal_False;
}

/*************************************************************************
|*
|*  SwFEShell::BeginMark()
|*
*************************************************************************/

sal_Bool SwFEShell::BeginMark( const Point &rPos )
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
        return sal_False;
}

/*************************************************************************
|*
|*  SwFEShell::MoveMark()
|*
*************************************************************************/

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

/*************************************************************************
|*
|*  SwFEShell::EndMark()
|*
*************************************************************************/

sal_Bool SwFEShell::EndMark()
{
    sal_Bool bRet = sal_False;
    OSL_ENSURE( Imp()->HasDrawView(), "EndMark without DrawView?" );

    if (Imp()->GetDrawView()->IsMarkObj())
    {
        bRet = Imp()->GetDrawView()->EndMarkObj();

        if ( bRet )
        {
            sal_Bool bShowHdl = sal_False;
            SwDrawView* pDView = Imp()->GetDrawView();
            // frames are not selected this way, except when
            // it is only one frame
            SdrMarkList &rMrkList = (SdrMarkList&)pDView->GetMarkedObjectList();
            SwFlyFrm* pOldSelFly = ::GetFlyFromMarked( &rMrkList, this );

            if ( rMrkList.GetMarkCount() > 1 )
                for ( sal_uInt16 i = 0; i < rMrkList.GetMarkCount(); ++i )
                {
                    SdrObject *pObj = rMrkList.GetMark( i )->GetMarkedSdrObj();
                    if( pObj->ISA(SwVirtFlyDrawObj) )
                    {
                        if ( !bShowHdl )
                        {
                            bShowHdl = sal_True;
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
                bRet = sal_False;
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

/*************************************************************************
|*
|*  SwFEShell::BreakSelect()
|*
*************************************************************************/

void SwFEShell::BreakMark()
{
    OSL_ENSURE( Imp()->HasDrawView(), "BreakMark without DrawView?" );
    Imp()->GetDrawView()->BrkMarkObj();
}

/*************************************************************************
|*
|*  SwFEShell::GetAnchorId()
|*
*************************************************************************/

short SwFEShell::GetAnchorId() const
{
    short nRet = SHRT_MAX;
    if ( Imp()->HasDrawView() )
    {
        const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkedObjectList();
        for ( sal_uInt16 i = 0; i < rMrkList.GetMarkCount(); ++i )
        {
            SdrObject *pObj = rMrkList.GetMark( i )->GetMarkedSdrObj();
            if ( pObj->ISA(SwVirtFlyDrawObj) )
            {
                nRet = -1;
                break;
            }
            SwDrawContact *pContact = (SwDrawContact*)GetUserCall(pObj);
            short nId = static_cast<short>(pContact->GetFmt()->GetAnchor().GetAnchorId());
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

/*************************************************************************
|*
|*  SwFEShell::ChgAnchor()
|*
*************************************************************************/

void SwFEShell::ChgAnchor( int eAnchorId, sal_Bool bSameOnly, sal_Bool bPosCorr )
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

        ::FrameNotify( this, FLY_DRAG );
    }
}

/*************************************************************************
|*
|*  SwFEShell::DelSelectedObj()
|*
*************************************************************************/

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

/*************************************************************************
|*
|*  SwFEShell::GetObjSize(), GetAnchorObjDiff()
|*
|*  Description        For the statusline to request the current
|*                     conditions
|*
*************************************************************************/

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

    if ( IsFrmSelected() )
    {
        SwFlyFrm *pFly = FindFlyFrm();
        aRet -= pFly->GetAnchorFrm()->Frm().Pos();
    }
    else
    {
        const SdrObject *pObj = pView->GetMarkedObjectList().GetMarkCount() == 1 ?
                                pView->GetMarkedObjectList().GetMark(0)->GetMarkedSdrObj() : 0;
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



/*************************************************************************
|*
|*  SwFEShell::IsGroupSelected()
|*
*************************************************************************/

sal_Bool SwFEShell::IsGroupSelected()
{
    if ( IsObjSelected() )
    {
        const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkedObjectList();
        for ( sal_uInt16 i = 0; i < rMrkList.GetMarkCount(); ++i )
        {
            SdrObject *pObj = rMrkList.GetMark( i )->GetMarkedSdrObj();
            // consider 'virtual' drawing objects.
            // Thus, use corresponding method instead of checking type.
            if ( pObj->IsGroupObject() &&
                 // --> #i38505# No ungroup allowed for 3d objects
                 !pObj->Is3DObj() &&
                 FLY_AS_CHAR != ((SwDrawContact*)GetUserCall(pObj))->
                                      GetFmt()->GetAnchor().GetAnchorId() )
            {
                return sal_True;
            }
        }
    }
    return sal_False;
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
        const SdrObject* pUpGroup = 0L;
        const SwFrm* pHeaderFooterFrm = 0L;
        const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkedObjectList();
        for ( sal_uInt16 i = 0; bIsGroupAllowed && i < rMrkList.GetMarkCount(); ++i )
        {
            const SdrObject* pObj = rMrkList.GetMark( i )->GetMarkedSdrObj();
            if ( i )
                bIsGroupAllowed = pObj->GetUpGroup() == pUpGroup;
            else
                pUpGroup = pObj->GetUpGroup();

            if ( bIsGroupAllowed )
            {
                SwFrmFmt* pFrmFmt( ::FindFrmFmt( const_cast<SdrObject*>(pObj) ) );
                if ( !pFrmFmt )
                {
                    OSL_FAIL( "<SwFEShell::IsGroupAllowed()> - missing frame format" );
                    bIsGroupAllowed = false;
                }
                else if ( FLY_AS_CHAR == pFrmFmt->GetAnchor().GetAnchorId() )
                {
                    bIsGroupAllowed = false;
                }
            }

            // check, if all selected objects are in the
            // same header/footer or not in header/footer.
            if ( bIsGroupAllowed )
            {
                const SwFrm* pAnchorFrm = 0L;
                if ( pObj->ISA(SwVirtFlyDrawObj) )
                {
                    const SwFlyFrm* pFlyFrm =
                            static_cast<const SwVirtFlyDrawObj*>(pObj)->GetFlyFrm();
                    if ( pFlyFrm )
                    {
                        pAnchorFrm = pFlyFrm->GetAnchorFrm();
                    }
                }
                else
                {
                    SwDrawContact* pDrawContact = static_cast<SwDrawContact*>(GetUserCall( pObj ));
                    if ( pDrawContact )
                    {
                        pAnchorFrm = pDrawContact->GetAnchorFrm( pObj );
                    }
                }
                if ( pAnchorFrm )
                {
                    if ( i )
                    {
                        bIsGroupAllowed =
                            ( pAnchorFrm->FindFooterOrHeader() == pHeaderFooterFrm );
                    }
                    else
                    {
                        pHeaderFooterFrm = pAnchorFrm->FindFooterOrHeader();
                    }
                }
            }

        }
    }

    return bIsGroupAllowed;
}

/*************************************************************************
|*
|*  SwFEShell::GroupSelection()
|*
|*  Description        The group gets the anchor and the contactobject
|*                     of the first in the selection
|*
*************************************************************************/

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

/*************************************************************************
|*
|*  SwFEShell::UnGroupSelection()
|*
|*  Description         The individual objects get a copy of the anchor and
|*                      the contactobject of the group
|*
*************************************************************************/

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

/*************************************************************************
|*
|*  SwFEShell::MirrorSelection()
|*
*************************************************************************/

void SwFEShell::MirrorSelection( sal_Bool bHorizontal )
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

sal_Bool SwFEShell::GotoFly( const String& rName, FlyCntType eType, sal_Bool bSelFrm )
{
    sal_Bool bRet = sal_False;
    static sal_uInt8 const aChkArr[ 4 ] = {
             /* FLYCNTTYPE_ALL */   0,
             /* FLYCNTTYPE_FRM */   ND_TEXTNODE,
             /* FLYCNTTYPE_GRF */   ND_GRFNODE,
             /* FLYCNTTYPE_OLE */   ND_OLENODE
            };

    const SwFlyFrmFmt* pFlyFmt = pDoc->FindFlyByName( rName, aChkArr[ eType]);
    if( pFlyFmt )
    {
        SET_CURR_SHELL( this );

        SwFlyFrm* pFrm = SwIterator<SwFlyFrm,SwFmt>::FirstElement( *pFlyFmt );
        if( pFrm )
        {
            if( bSelFrm )
            {
                SelectObj( pFrm->Frm().Pos(), 0, pFrm->GetVirtDrawObj() );
                if( !ActionPend() )
                    MakeVisible( pFrm->Frm() );
            }
            else
            {
                SwCntntFrm *pCFrm = pFrm->ContainsCntnt();
                if ( pCFrm )
                {
                    SwCntntNode *pCNode = pCFrm->GetNode();
                    ClearMark();
                    SwPaM* pCrsr = GetCrsr();

                    pCrsr->GetPoint()->nNode = *pCNode;
                    pCrsr->GetPoint()->nContent.Assign( pCNode, 0 );

                    SwRect& rChrRect = (SwRect&)GetCharRect();
                    rChrRect = pFrm->Prt();
                    rChrRect.Pos() += pFrm->Frm().Pos();
                    GetCrsrDocPos() = rChrRect.Pos();
                }
            }
            bRet = sal_True;
        }
    }
    return bRet;
}

sal_uInt16 SwFEShell::GetFlyCount( FlyCntType eType ) const
{
    return GetDoc()->GetFlyCount(eType);
}


const SwFrmFmt*  SwFEShell::GetFlyNum(sal_uInt16 nIdx, FlyCntType eType ) const
{
    return GetDoc()->GetFlyNum(nIdx, eType );
}

// show the current selected object
void SwFEShell::MakeSelVisible()
{
    if( Imp()->HasDrawView() &&
        Imp()->GetDrawView()->GetMarkedObjectList().GetMarkCount() )
    {
        MakeVisible( Imp()->GetDrawView()->GetAllMarkedRect() );
    }
    else
        SwCrsrShell::MakeSelVisible();
}


// how is the selected object protected?
sal_uInt8 SwFEShell::IsSelObjProtected( sal_uInt16 eType ) const
{
    int nChk = 0;
    const bool bParent = (eType & FLYPROTECT_PARENT);
    if( Imp()->HasDrawView() )
    {
        const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkedObjectList();
        for( sal_uLong i = rMrkList.GetMarkCount(); i; )
        {
            SdrObject *pObj = rMrkList.GetMark( --i )->GetMarkedSdrObj();
            if( !bParent )
            {
                nChk |= ( pObj->IsMoveProtect() ? FLYPROTECT_POS : 0 ) |
                        ( pObj->IsResizeProtect()? FLYPROTECT_SIZE : 0 );

                if( pObj->ISA(SwVirtFlyDrawObj) )
                {
                    SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm();
                    if ( (FLYPROTECT_CONTENT & eType) && pFly->GetFmt()->GetProtect().IsCntntProtected() )
                        nChk |= FLYPROTECT_CONTENT;

                    if ( pFly->Lower() && pFly->Lower()->IsNoTxtFrm() )
                    {
                        SwOLENode *pNd = ((SwCntntFrm*)pFly->Lower())->GetNode()->GetOLENode();
                        uno::Reference < embed::XEmbeddedObject > xObj( pNd ? pNd->GetOLEObj().GetOleRef() : 0 );
                        if ( xObj.is() )
                        {
                            // TODO/LATER: use correct aspect
                            const bool bNeverResize = (embed::EmbedMisc::EMBED_NEVERRESIZE & xObj->getStatus( embed::Aspects::MSOLE_CONTENT ));
                            if ( (FLYPROTECT_CONTENT & eType) && bNeverResize )
                            {
                                nChk |= FLYPROTECT_SIZE;
                                nChk |= FLYPROTECT_FIXED;
                            }

                            // set FLYPROTECT_POS if it is a Math object anchored 'as char' and baseline alignment is activated
                            const bool bProtectMathPos = SotExchange::IsMath( xObj->getClassID() )
                                    && FLY_AS_CHAR == pFly->GetFmt()->GetAnchor().GetAnchorId()
                                    && pDoc->get( IDocumentSettingAccess::MATH_BASELINE_ALIGNMENT );
                            if ((FLYPROTECT_POS & eType) && bProtectMathPos)
                                nChk |= FLYPROTECT_POS;
                        }
                    }
                }
                nChk &= eType;
                if( nChk == eType )
                    return static_cast<sal_uInt8>(eType);
            }
            const SwFrm* pAnch;
            if( pObj->ISA(SwVirtFlyDrawObj) )
                pAnch = ( (SwVirtFlyDrawObj*)pObj )->GetFlyFrm()->GetAnchorFrm();
            else
            {
                SwDrawContact* pTmp = (SwDrawContact*)GetUserCall(pObj);
                pAnch = pTmp ? pTmp->GetAnchorFrm( pObj ) : NULL;
            }
            if( pAnch && pAnch->IsProtected() )
                return static_cast<sal_uInt8>(eType);
        }
    }
    return static_cast<sal_uInt8>(nChk);
}

sal_Bool SwFEShell::GetObjAttr( SfxItemSet &rSet ) const
{
    if ( !IsObjSelected() )
        return sal_False;

    const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkedObjectList();
    for ( sal_uInt16 i = 0; i < rMrkList.GetMarkCount(); ++i )
    {
        SdrObject *pObj = rMrkList.GetMark( i )->GetMarkedSdrObj();
        SwDrawContact *pContact = (SwDrawContact*)GetUserCall(pObj);
        // --> make code robust
        OSL_ENSURE( pContact, "<SwFEShell::GetObjAttr(..)> - missing <pContact> - please inform OD." );
        if ( pContact )
        {
            if ( i )
                rSet.MergeValues( pContact->GetFmt()->GetAttrSet() );
            else
                rSet.Put( pContact->GetFmt()->GetAttrSet() );
        }
    }
    return sal_True;
}

sal_Bool SwFEShell::SetObjAttr( const SfxItemSet& rSet )
{
    SET_CURR_SHELL( this );

    if ( !rSet.Count() )
    { OSL_ENSURE( !this, "SetObjAttr, empty set." );
        return sal_False;
    }

    StartAllAction();
    StartUndo( UNDO_INSATTR );

    const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkedObjectList();
    for ( sal_uInt16 i = 0; i < rMrkList.GetMarkCount(); ++i )
    {
        SdrObject *pObj = rMrkList.GetMark( i )->GetMarkedSdrObj();
        SwDrawContact *pContact = (SwDrawContact*)GetUserCall(pObj);
        GetDoc()->SetAttr( rSet, *pContact->GetFmt() );
    }

    EndUndo( UNDO_INSATTR );
    EndAllActionAndCall();
    GetDoc()->SetModified();
    return sal_True;
}

sal_Bool SwFEShell::IsAlignPossible() const
{
    sal_uInt16 nCnt;
    if ( 0 < (nCnt = IsObjSelected()) )
    {
        sal_Bool bRet = sal_True;
        if ( nCnt == 1 )
        {
            SdrObject *pO = Imp()->GetDrawView()->GetMarkedObjectList().GetMark(0)->GetMarkedSdrObj();
            SwDrawContact *pC = (SwDrawContact*)GetUserCall(pO);
            OSL_ENSURE( pC, "No SwDrawContact!");
            //only as character bound drawings can be aligned
            bRet = pC ? (pC->GetFmt()->GetAnchor().GetAnchorId() == FLY_AS_CHAR) : sal_False;
        }
        if ( bRet )
            return Imp()->GetDrawView()->IsAlignPossible();
    }
    return sal_False;
}


// temporary fix till  SS of JOE is availale
void SwFEShell::CheckUnboundObjects()
{
    SET_CURR_SHELL( this );

    const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkedObjectList();
    for ( sal_uInt16 i = 0; i < rMrkList.GetMarkCount(); ++i )
    {
        SdrObject *pObj = rMrkList.GetMark( i )->GetMarkedSdrObj();
        if ( !GetUserCall(pObj) )
        {
            const Rectangle &rBound = pObj->GetSnapRect();
            const Point aPt( rBound.TopLeft() );
            const SwFrm *pPage = GetLayout()->Lower();
            const SwFrm *pLast = pPage;
            while ( pPage && !pPage->Frm().IsInside( aPt ) )
            {
                if ( aPt.Y() > pPage->Frm().Bottom() )
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

            SwFmtAnchor aAnch;
            const SwFrm *pAnch = 0;
            {
            pAnch = ::FindAnchor( pPage, aPt, sal_True );
            SwPosition aPos( *((SwCntntFrm*)pAnch)->GetNode() );
            aAnch.SetType( FLY_AT_PARA );
            aAnch.SetAnchor( &aPos );
            ((SwRect&)GetCharRect()).Pos() = aPt;
            }

            // First the action here, to assure GetCharRect delivers current values.
            StartAllAction();

            SfxItemSet aSet( GetAttrPool(), RES_FRM_SIZE, RES_FRM_SIZE,
                                            RES_SURROUND, RES_ANCHOR, 0 );
            aSet.Put( aAnch );

            Point aRelNullPt;

            if( OBJ_CAPTION == nIdent )
                aRelNullPt = ((SdrCaptionObj*)pObj)->GetTailPos();
            else
                aRelNullPt = rBound.TopLeft();

            aSet.Put( aAnch );
            aSet.Put( SwFmtSurround( SURROUND_THROUGHT ) );
            SwFrmFmt* pFmt = getIDocumentLayoutAccess()->MakeLayoutFmt( RND_DRAW_OBJECT, &aSet );

            SwDrawContact *pContact = new SwDrawContact(
                                            (SwDrawFrmFmt*)pFmt, pObj );

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



int SwFEShell::Chainable( SwRect &rRect, const SwFrmFmt &rSource,
                            const Point &rPt ) const
{
    rRect.Clear();

    // The source is not allowed to have a follow.
    const SwFmtChain &rChain = rSource.GetChain();
    if ( rChain.GetNext() )
        return SW_CHAIN_SOURCE_CHAINED;

    if( Imp()->HasDrawView() )
    {
        SdrObject* pObj;
        SdrPageView* pPView;
        SwDrawView *pDView = (SwDrawView*)Imp()->GetDrawView();
        const sal_uInt16 nOld = pDView->GetHitTolerancePixel();
        pDView->SetHitTolerancePixel( 0 );
        if( pDView->PickObj( rPt, pDView->getHitTolLog(), pObj, pPView, SDRSEARCH_PICKMARKABLE ) &&
            pObj->ISA(SwVirtFlyDrawObj) )
        {
            SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm();
            rRect = pFly->Frm();

            // Target and source should not be equal and the list
            // should not be cyclic
            SwFrmFmt *pFmt = pFly->GetFmt();
            return GetDoc()->Chainable(rSource, *pFmt);
        }
        pDView->SetHitTolerancePixel( nOld );
    }
    return SW_CHAIN_NOT_FOUND;
}

int SwFEShell::Chain( SwFrmFmt &rSource, const SwFrmFmt &rDest )
{
    return GetDoc()->Chain(rSource, rDest);
}

int SwFEShell::Chain( SwFrmFmt &rSource, const Point &rPt )
{
    SwRect aDummy;
    int nErr = Chainable( aDummy, rSource, rPt );
    if ( !nErr )
    {
        StartAllAction();
        SdrObject* pObj;
        SdrPageView* pPView;
        SwDrawView *pDView = (SwDrawView*)Imp()->GetDrawView();
        const sal_uInt16 nOld = pDView->GetHitTolerancePixel();
        pDView->SetHitTolerancePixel( 0 );
        pDView->PickObj( rPt, pDView->getHitTolLog(), pObj, pPView, SDRSEARCH_PICKMARKABLE );
        pDView->SetHitTolerancePixel( nOld );
        SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm();

        SwFlyFrmFmt *pFmt = (SwFlyFrmFmt*)pFly->GetFmt();
        GetDoc()->Chain(rSource, *pFmt);
        EndAllAction();
        SetChainMarker();
    }
    return nErr;
}

void SwFEShell::Unchain( SwFrmFmt &rFmt )
{
    StartAllAction();
    GetDoc()->Unchain(rFmt);
    EndAllAction();
}


void SwFEShell::HideChainMarker()
{
    if ( pChainFrom )
    {
        delete pChainFrom;
        pChainFrom = 0L;
    }
    if ( pChainTo )
    {
        delete pChainTo;
        pChainTo = 0L;
    }
}

void SwFEShell::SetChainMarker()
{
    sal_Bool bDelFrom = sal_True,
             bDelTo   = sal_True;
    if ( IsFrmSelected() )
    {
        SwFlyFrm *pFly = FindFlyFrm();

        if ( pFly->GetPrevLink() )
        {
            bDelFrom = sal_False;
            const SwFrm *pPre = pFly->GetPrevLink();

            Point aStart( pPre->Frm().Right(), pPre->Frm().Bottom());
            Point aEnd(pFly->Frm().Pos());

            if ( !pChainFrom )
            {
                pChainFrom = new SdrDropMarkerOverlay( *GetDrawView(), aStart, aEnd );
            }
        }
        if ( pFly->GetNextLink() )
        {
            bDelTo = sal_False;
            const SwFlyFrm *pNxt = pFly->GetNextLink();

            Point aStart( pFly->Frm().Right(), pFly->Frm().Bottom());
            Point aEnd(pNxt->Frm().Pos());

            if ( !pChainTo )
            {
                pChainTo = new SdrDropMarkerOverlay( *GetDrawView(), aStart, aEnd );
            }
        }
    }

    if ( bDelFrom )
    {
        delete pChainFrom, pChainFrom = 0;
    }

    if ( bDelTo )
    {
        delete pChainTo,   pChainTo = 0;
    }
}

long SwFEShell::GetSectionWidth( SwFmt& rFmt ) const
{
    SwFrm *pFrm = GetCurrFrm();
    // Is the cursor at this moment in a SectionFrm?
    if( pFrm && pFrm->IsInSct() )
    {
        SwSectionFrm* pSect = pFrm->FindSctFrm();
        do
        {
            // Is it the right one?
            if( pSect->KnowsFormat( rFmt ) )
                return pSect->Frm().Width();
            // for nested areas
            pSect = pSect->GetUpper()->FindSctFrm();
        }
        while( pSect );
    }
    SwIterator<SwSectionFrm,SwFmt> aIter( rFmt );
    for ( SwSectionFrm* pSct = aIter.First(); pSct; pSct = aIter.Next() )
    {
        if( !pSct->IsFollow() )
        {
            return pSct->Frm().Width();
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
        0L, pDrawModel);

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

        if(pObj->ISA(SdrCircObj))
        {
            SfxItemSet aAttr(pDrawModel->GetItemPool());
            aAttr.Put(SdrCircStartAngleItem(9000));
            aAttr.Put(SdrCircEndAngleItem(0));
            pObj->SetMergedItemSet(aAttr);
        }
        else if(pObj->ISA(SdrPathObj))
        {
            basegfx::B2DPolyPolygon aPoly;

            switch(eSdrObjectKind)
            {
                case OBJ_PATHLINE:
                {
                    basegfx::B2DPolygon aInnerPoly;

                    aInnerPoly.append(basegfx::B2DPoint(aRect.Left(), aRect.Bottom()));

                    const basegfx::B2DPoint aCenterBottom(aRect.Center().X(), aRect.Bottom());
                    aInnerPoly.appendBezierSegment(
                        aCenterBottom,
                        aCenterBottom,
                        basegfx::B2DPoint(aRect.Center().X(), aRect.Center().Y()));

                    const basegfx::B2DPoint aCenterTop(aRect.Center().X(), aRect.Top());
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
                        basegfx::B2DPoint(aRect.Center().X(), aRect.Top()),
                        basegfx::B2DPoint(aRect.Center().X(), aRect.Center().Y()));

                    aInnerPoly.appendBezierSegment(
                        basegfx::B2DPoint(aRect.Center().X(), aRect.Bottom()),
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
                        aInnerPoly.append(basegfx::B2DPoint(aRect.Center().X(), aRect.Bottom()));
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
                    aTempPoly.append(basegfx::B2DPoint(aRect.TopLeft().X(), nYMiddle));
                    aTempPoly.append(basegfx::B2DPoint(aRect.BottomRight().X(), nYMiddle));
                    aPoly.append(aTempPoly);
                }
                break;
            }

            ((SdrPathObj*)pObj)->SetPathPoly(aPoly);
        }
        else if(pObj->ISA(SdrCaptionObj))
        {
            sal_Bool bVerticalText = ( SID_DRAW_TEXT_VERTICAL == nSlotId ||
                                            SID_DRAW_CAPTION_VERTICAL == nSlotId );
            ((SdrTextObj*)pObj)->SetVerticalWriting(bVerticalText);
            if(bVerticalText)
            {
                SfxItemSet aSet(pObj->GetMergedItemSet());
                aSet.Put(SdrTextVertAdjustItem(SDRTEXTVERTADJUST_CENTER));
                aSet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_RIGHT));
                pObj->SetMergedItemSet(aSet);
            }

            ((SdrCaptionObj*)pObj)->SetLogicRect(aRect);
            ((SdrCaptionObj*)pObj)->SetTailPos(
                aRect.TopLeft() - Point(aRect.GetWidth() / 2, aRect.GetHeight() / 2));
        }
        else if(pObj->ISA(SdrTextObj))
        {
            SdrTextObj* pText = (SdrTextObj*)pObj;
            pText->SetLogicRect(aRect);

            sal_Bool bVertical = (SID_DRAW_TEXT_VERTICAL == nSlotId);
            sal_Bool bMarquee = (SID_DRAW_TEXT_MARQUEE == nSlotId);

            pText->SetVerticalWriting(bVertical);

            if(bVertical)
            {
                SfxItemSet aSet(pDrawModel->GetItemPool());
                aSet.Put(SdrTextAutoGrowWidthItem(sal_True));
                aSet.Put(SdrTextAutoGrowHeightItem(sal_False));
                aSet.Put(SdrTextVertAdjustItem(SDRTEXTVERTADJUST_TOP));
                aSet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_RIGHT));
                pText->SetMergedItemSet(aSet);
            }

            if(bMarquee)
            {
                SfxItemSet aSet(pDrawModel->GetItemPool(), SDRATTR_MISC_FIRST, SDRATTR_MISC_LAST);
                aSet.Put( SdrTextAutoGrowWidthItem( sal_False ) );
                aSet.Put( SdrTextAutoGrowHeightItem( sal_False ) );
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

    @author OD

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
            OSL_ENSURE( !pSdrObj->ISA(SwVirtFlyDrawObj), "wrong usage of SwFEShell::GetShapeBackgrd - selected object is not a drawing object!");
            if ( !pSdrObj->ISA(SwVirtFlyDrawObj) )
            {
                // determine page frame of the frame the shape is anchored.
                const SwFrm* pAnchorFrm =
                        static_cast<SwDrawContact*>(GetUserCall(pSdrObj))->GetAnchorFrm( pSdrObj );
                OSL_ENSURE( pAnchorFrm, "inconsistent modell - no anchor at shape!");
                if ( pAnchorFrm )
                {
                    const SwPageFrm* pPageFrm = pAnchorFrm->FindPageFrm();
                    OSL_ENSURE( pPageFrm, "inconsistent modell - no page!");
                    if ( pPageFrm )
                    {
                        aRetColor = pPageFrm->GetDrawBackgrdColor();
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

    @author OD

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
            OSL_ENSURE( !pSdrObj->ISA(SwVirtFlyDrawObj), "wrong usage of SwFEShell::GetShapeBackgrd - selected object is not a drawing object!");
            if ( !pSdrObj->ISA(SwVirtFlyDrawObj) )
            {
                // determine page frame of the frame the shape is anchored.
                const SwFrm* pAnchorFrm =
                        static_cast<SwDrawContact*>(GetUserCall(pSdrObj))->GetAnchorFrm( pSdrObj );
                OSL_ENSURE( pAnchorFrm, "inconsistent modell - no anchor at shape!");
                if ( pAnchorFrm )
                {
                    const SwPageFrm* pPageFrm = pAnchorFrm->FindPageFrm();
                    OSL_ENSURE( pPageFrm, "inconsistent modell - no page!");
                    if ( pPageFrm )
                    {
                        bRet = pPageFrm->IsRightToLeft() ? true : false;
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
    const SwFrm *pPage = GetLayout()->Lower();
    while ( pPage && !pPage->Frm().IsInside( rDocPos ) )
    {
        pPage = pPage->GetNext();
    }
    if(pPage)
    {
        aRet = rDocPos - pPage->Frm().TopLeft();
    }
    return aRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
