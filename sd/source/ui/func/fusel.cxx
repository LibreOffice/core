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

#include "fusel.hxx"
#include <basic/sbstar.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdogrp.hxx>
#include <svx/polysc3d.hxx>
#include "drawview.hxx"
#include <svtools/imapobj.hxx>
#include <svl/urihelper.hxx>
#include <unotools/localfilehelper.hxx>
#include <svx/svxids.hrc>
#include <svx/xfillit0.hxx>
#include <sfx2/app.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/aeitem.hxx>
#include <svl/stritem.hxx>
#include <svl/intitem.hxx>
#include <sfx2/dispatch.hxx>
#include <tools/urlobj.hxx>
#include <sfx2/docfile.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/flditem.hxx>

#include <svx/svdotable.hxx>

#include "app.hrc"
#include "strings.hrc"
#include "res_bmp.hrc"
#include "GraphicDocShell.hxx"
#include "app.hxx"
#include "DrawDocShell.hxx"
#include "stlpool.hxx"
#include "anminfo.hxx"
#include "fudraw.hxx"
#include "ViewShell.hxx"
#include "ViewShellBase.hxx"
#include "FrameView.hxx"
#include "View.hxx"
#include "Window.hxx"
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "DrawViewShell.hxx"
#include "ToolBarManager.hxx"
#include "pgjump.hxx"
#include <svx/globl3d.hxx>
#include "Client.hxx"

#include "slideshow.hxx"

#include <svx/svdundo.hxx>
#include <avmedia/mediawindow.hxx>

#include <svx/sdrhittesthelper.hxx>

using namespace ::com::sun::star;

namespace sd {

TYPEINIT1( FuSelection, FuDraw );

/*************************************************************************
|*
|* Constructor
|*
\************************************************************************/

FuSelection::FuSelection (
    ViewShell* pViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument* pDoc,
    SfxRequest& rReq)
    : FuDraw(pViewSh, pWin, pView, pDoc, rReq),
      bTempRotation(sal_False),
      bSelectionChanged(sal_False),
      bHideAndAnimate(sal_False),
      pHdl(NULL),
      bSuppressChangesOfSelection(sal_False),
      bMirrorSide0(sal_False),
      nEditMode(SID_BEZIER_MOVE),
      pWaterCanCandidate(NULL)
{
}

FunctionReference FuSelection::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    FunctionReference xFunc( new FuSelection( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuSelection::DoExecute( SfxRequest& rReq )
{
    FuDraw::DoExecute( rReq );

    // Select object bar
    SelectionHasChanged();
}

/*************************************************************************
|*
|* Destructor
|*
\************************************************************************/

FuSelection::~FuSelection()
{
    mpView->UnmarkAllPoints();
    mpView->ResetCreationActive();

    if ( mpView->GetDragMode() != SDRDRAG_MOVE )
    {
        mpView->SetDragMode(SDRDRAG_MOVE);
    }
}

/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/

sal_Bool FuSelection::MouseButtonDown(const MouseEvent& rMEvt)
{
    // Hack for #?????#
    bHideAndAnimate = sal_False;

    pHdl = NULL;
    sal_Bool bReturn = FuDraw::MouseButtonDown(rMEvt);
    sal_Bool bWaterCan = SD_MOD()->GetWaterCan();
    const bool bReadOnly = mpDocSh->IsReadOnly();
    // When the right mouse button is pressed then only select objects
    // (and deselect others) as a preparation for showing the context
    // menu.
    const bool bSelectionOnly = rMEvt.IsRight();

    bMBDown = sal_True;
    bSelectionChanged = sal_False;

    if ( mpView->IsAction() )
    {
        if ( rMEvt.IsRight() )
            mpView->BckAction();
        return sal_True;
    }

    sal_uInt16 nDrgLog = sal_uInt16 ( mpWindow->PixelToLogic(Size(DRGPIX,0)).Width() );
    sal_uInt16 nHitLog = sal_uInt16 ( mpWindow->PixelToLogic(Size(HITPIX,0)).Width() );

    // The following code is executed for right clicks as well as for left
    // clicks in order to modify the selection for the right button as a
    // preparation for the context menu.  The functions BegMarkObject() and
    // BegDragObject(), however, are not called for right clicks because a)
    // it makes no sense and b) to have IsAction() return sal_False when called
    // from Command() which is a prerequisite for the context menu.
    if ((rMEvt.IsLeft() || rMEvt.IsRight())
        && !mpView->IsAction()
        && (mpView->IsFrameDragSingles() || !mpView->HasMarkablePoints()))
    {
        /******************************************************************
        * NO BEZIER_EDITOR
        ******************************************************************/
        mpWindow->CaptureMouse();
        pHdl = mpView->PickHandle(aMDPos);
        SdrObject* pObj;
        SdrPageView* pPV;

        long nAngle0  = GetAngle(aMDPos - mpView->GetRef1());
        nAngle0 -= 27000;
        nAngle0 = NormAngle360(nAngle0);
        bMirrorSide0 = sal_Bool (nAngle0 < 18000L);

        if (!pHdl && mpView->Is3DRotationCreationActive())
        {
            /******************************************************************
            * If 3D-rotation bodies are about to be created,
            * end creation now.
            ******************************************************************/
            bSuppressChangesOfSelection = sal_True;
            if(mpWindow)
                mpWindow->EnterWait();
            mpView->End3DCreation();
            bSuppressChangesOfSelection = sal_False;
            mpView->ResetCreationActive();
            if(mpWindow)
                mpWindow->LeaveWait();
        }

        sal_Bool bTextEdit = sal_False;
        SdrViewEvent aVEvt;
        SdrHitKind eHit = mpView->PickAnything(rMEvt, SDRMOUSEBUTTONDOWN, aVEvt);

        if ( eHit == SDRHIT_TEXTEDITOBJ && ( mpViewShell->GetFrameView()->IsQuickEdit() || dynamic_cast< sdr::table::SdrTableObj* >( aVEvt.pObj ) != NULL ) )
        {
            bTextEdit = sal_True;
        }

        if(!bTextEdit
            && !mpDocSh->IsReadOnly()
            && ((mpView->IsMarkedHit(aMDPos, nHitLog) && !rMEvt.IsShift() && !rMEvt.IsMod2()) || pHdl != NULL)
            && (rMEvt.GetClicks() != 2)
            )
        {
            if (!pHdl && mpView->Is3DRotationCreationActive())
            {
                // Switch between 3D-rotation body -> selection
                mpView->ResetCreationActive();
            }
            else if (bWaterCan)
            {
                // Remember the selected object for proper handling in
                // MouseButtonUp().
                pWaterCanCandidate = pickObject (aMDPos);
            }
            else
            {
                // Handle oder marked object hit
                bFirstMouseMove = sal_True;
                aDragTimer.Start();
            }

            if ( ! rMEvt.IsRight())
                mpView->BegDragObj(aMDPos, (OutputDevice*) NULL, pHdl, nDrgLog);
            bReturn = sal_True;
        }
        else
        {
            if (!rMEvt.IsMod2() && mpView->PickObj(aMDPos, mpView->getHitTolLog(), pObj, pPV, SDRSEARCH_PICKMACRO))
            {
                mpView->BegMacroObj(aMDPos, nHitLog, pObj, pPV, mpWindow);
                bReturn = sal_True;
            }
            else if ( bTextEdit )
            {
                sal_uInt16 nSdrObjKind = aVEvt.pObj->GetObjIdentifier();

                if (aVEvt.pObj->GetObjInventor() == SdrInventor &&
                    (nSdrObjKind == OBJ_TEXT ||
                     nSdrObjKind == OBJ_TITLETEXT ||
                     nSdrObjKind == OBJ_OUTLINETEXT ||
                     !aVEvt.pObj->IsEmptyPresObj()))
                {
                    // Seamless Editing: branch to text input
                    if (!rMEvt.IsShift())
                        mpView->UnmarkAll();

                    SfxUInt16Item aItem(SID_TEXTEDIT, 1);
                    mpViewShell->GetViewFrame()->GetDispatcher()->
                    Execute(SID_TEXTEDIT, SFX_CALLMODE_SYNCHRON |
                            SFX_CALLMODE_RECORD, &aItem, 0L);
                    return bReturn; // CAUTION, due to the synchronous slot the object is deleted now
                }
            }
            else if ( !rMEvt.IsMod2() && rMEvt.GetClicks() == 1 &&
                      aVEvt.eEvent == SDREVENT_EXECUTEURL )
             {
                mpWindow->ReleaseMouse();
                SfxStringItem aStrItem(SID_FILE_NAME, aVEvt.pURLField->GetURL());
                SfxStringItem aReferer(SID_REFERER, mpDocSh->GetMedium()->GetName());
                SfxBoolItem aBrowseItem( SID_BROWSE, sal_True );
                SfxViewFrame* pFrame = mpViewShell->GetViewFrame();
                mpWindow->ReleaseMouse();

                if (rMEvt.IsMod1())
                {
                    // Open in new frame
                    pFrame->GetDispatcher()->Execute(SID_OPENDOC, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD,
                                &aStrItem, &aBrowseItem, &aReferer, 0L);
                }
                else
                {
                    // Open in current frame
                    SfxFrameItem aFrameItem(SID_DOCFRAME, pFrame);
                    pFrame->GetDispatcher()->Execute(SID_OPENDOC, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD,
                                &aStrItem, &aFrameItem, &aBrowseItem, &aReferer, 0L);
                }

                bReturn = sal_True;
            }
            else if(!rMEvt.IsMod2()
                && mpViewShell->ISA(DrawViewShell)
                )
            {
                if(mpView->PickObj(aMDPos, mpView->getHitTolLog(), pObj, pPV, SDRSEARCH_ALSOONMASTER))
                {
                    // Animate object when not just selecting.
                    if ( ! bSelectionOnly)
                        bReturn = AnimateObj(pObj, aMDPos);

                    if (!bReturn && (pObj->ISA(SdrObjGroup) || pObj->ISA(E3dPolyScene)))
                    {
                        if(rMEvt.GetClicks() == 1)
                        {
                            // Look into the group
                            if (mpView->PickObj(aMDPos, mpView->getHitTolLog(), pObj, pPV, SDRSEARCH_ALSOONMASTER | SDRSEARCH_DEEP))
                                bReturn = AnimateObj(pObj, aMDPos);
                        }
                        else if( !bReadOnly && rMEvt.GetClicks() == 2)
                        {
                            // New: double click on selected Group object
                            // enter group
                            if ( ! bSelectionOnly
                                && pObj
                                && pObj->GetPage() == pPV->GetPage())
                                bReturn = pPV->EnterGroup(pObj);
                        }
                    }
                }

                // #i71727# replaced else here with two possibilities, once the original else (!pObj)
                // and also ignoring the found object when it's on a masterpage
                if(!pObj || (pObj->GetPage() && pObj->GetPage()->IsMasterPage()))
                {
                    if(mpView->IsGroupEntered() && 2 == rMEvt.GetClicks())
                    {
                        // New: double click on empty space/on obj on MasterPage, leave group
                        mpView->LeaveOneGroup();
                        bReturn = sal_True;
                    }
                }
            }

            if (!bReturn)
            {
                if (bWaterCan)
                {
                    if ( ! (rMEvt.IsShift() || rMEvt.IsMod2()))
                    {
                        // Find the object under the current mouse position
                        // and store it for the MouseButtonUp() method to
                        // evaluate.
                        pWaterCanCandidate = pickObject (aMDPos);
                    }
                }
                else
                {
                    bReturn = sal_True;
                    sal_Bool bDeactivateOLE = sal_False;

                    if ( !rMEvt.IsShift() && !rMEvt.IsMod2() )
                    {
                        OSL_ASSERT (mpViewShell->GetViewShell()!=NULL);
                        Client* pIPClient = static_cast<Client*>(
                            mpViewShell->GetViewShell()->GetIPClient());

                        if (pIPClient && pIPClient->IsObjectInPlaceActive())
                        {
                            // OLE-Objekt gets deactivated in subsequent UnmarkAll()
                            bDeactivateOLE = sal_True;
                        }

                        mpView->UnmarkAll();
                    }

                    sal_Bool bMarked = sal_False;

                    if ( !rMEvt.IsMod1() && !bDeactivateOLE)
                    {
                        if ( rMEvt.IsMod2() )
                        {
                            bMarked = mpView->MarkNextObj(aMDPos, nHitLog, rMEvt.IsShift() );
                        }
                        else
                        {
                            sal_Bool bToggle = sal_False;

                            if (rMEvt.IsShift() && mpView->GetMarkedObjectList().GetMarkCount() > 1)
                            {
                                // No Toggle on single selection
                                bToggle = sal_True;
                            }

                            bMarked = mpView->MarkObj(aMDPos, nHitLog, bToggle, sal_False);
                        }
                    }

                    if( !bDeactivateOLE )
                    {
                        if ( !bReadOnly &&
                             bMarked                                                   &&
                             (!rMEvt.IsShift() || mpView->IsMarkedHit(aMDPos, nHitLog)))
                        {
                            /**********************************************************
                             * Move object
                             **********************************************************/
                            aDragTimer.Start();

                            pHdl=mpView->PickHandle(aMDPos);
                            if ( ! rMEvt.IsRight())
                                mpView->BegDragObj(aMDPos, (OutputDevice*) NULL, pHdl, nDrgLog);
                        }
                        else
                        {
                            /**********************************************************
                             * Select object
                             **********************************************************/
                            if ( ! rMEvt.IsRight())
                                mpView->BegMarkObj(aMDPos);
                        }
                    }

                    if( bMarked && bTempRotation && (nSlotId == SID_OBJECT_ROTATE) && !rMEvt.IsShift() && (rMEvt.GetClicks() != 2) )
                    {
                        nSlotId = SID_OBJECT_SELECT;
                        Activate();
                    }
                }
            }
        }
    }
    else if ( !bReadOnly
              && (rMEvt.IsLeft() || rMEvt.IsRight())
              && !mpView->IsAction())
    {
        /**********************************************************************
        * BEZIER-EDITOR
        **********************************************************************/
        mpWindow->CaptureMouse();
        SdrViewEvent aVEvt;
        SdrHitKind eHit = mpView->PickAnything(rMEvt, SDRMOUSEBUTTONDOWN, aVEvt);

        if (eHit == SDRHIT_HANDLE && aVEvt.pHdl->GetKind() == HDL_BWGT)
        {
            /******************************************************************
            * Drag Handle
            ******************************************************************/
            if ( ! rMEvt.IsRight())
                mpView->BegDragObj(aMDPos, (OutputDevice*) NULL, aVEvt.pHdl, nDrgLog);
        }
        else if (eHit == SDRHIT_MARKEDOBJECT && nEditMode == SID_BEZIER_INSERT)
        {
            /******************************************************************
            * Insert glue point
            ******************************************************************/
            mpView->BegInsObjPoint(aMDPos, rMEvt.IsMod1());
        }
        else if (eHit == SDRHIT_MARKEDOBJECT && rMEvt.IsMod1())
        {
            /******************************************************************
            * Select glue point
            ******************************************************************/
            if (!rMEvt.IsShift())
                mpView->UnmarkAllPoints();

            if ( ! rMEvt.IsRight())
                mpView->BegMarkPoints(aMDPos);
        }
        else if (eHit == SDRHIT_MARKEDOBJECT && !rMEvt.IsShift() && !rMEvt.IsMod2())
        {
            /******************************************************************
            * Move object
            ******************************************************************/
            if ( ! rMEvt.IsRight())
                mpView->BegDragObj(aMDPos, (OutputDevice*) NULL, NULL, nDrgLog);
        }
        else if (eHit == SDRHIT_HANDLE)
        {
            /******************************************************************
            * Select glue point
            ******************************************************************/
            if (!mpView->IsPointMarked(*aVEvt.pHdl) || rMEvt.IsShift())
            {
                if (!rMEvt.IsShift())
                {
                    mpView->UnmarkAllPoints();
                    pHdl = mpView->PickHandle(aMDPos);
                }
                else
                {
                    if (mpView->IsPointMarked(*aVEvt.pHdl))
                    {
                        mpView->UnmarkPoint(*aVEvt.pHdl);
                        pHdl = NULL;
                    }
                    else
                    {
                        pHdl = mpView->PickHandle(aMDPos);
                    }
                }

                if (pHdl)
                {
                    mpView->MarkPoint(*pHdl);
                    if ( ! rMEvt.IsRight())
                        mpView->BegDragObj(aMDPos, (OutputDevice*) NULL, pHdl, nDrgLog);
                }
            }
            else
            {
                // Point IS marked and NO shift is pressed. Start
                // dragging of selected point(s)
                pHdl = mpView->PickHandle(aMDPos);
                if(pHdl)
                    if ( ! rMEvt.IsRight())
                        mpView->BegDragObj(aMDPos, (OutputDevice*)NULL, pHdl, nDrgLog);
            }
        }
        else
        {
            /******************************************************************
            * Select or drag object
            ******************************************************************/
            if (!rMEvt.IsShift() && !rMEvt.IsMod2() && eHit == SDRHIT_UNMARKEDOBJECT)
            {
               mpView->UnmarkAllObj();
            }

            sal_Bool bMarked = sal_False;

            if (!rMEvt.IsMod1())
            {
                if (rMEvt.IsMod2())
                {
                    bMarked = mpView->MarkNextObj(aMDPos, nHitLog, rMEvt.IsShift());
                }
                else
                {
                    bMarked = mpView->MarkObj(aMDPos, nHitLog, rMEvt.IsShift(), sal_False);
                }
            }

            if (bMarked &&
                (!rMEvt.IsShift() || eHit == SDRHIT_MARKEDOBJECT))
            {
                // Move object
                if ( ! rMEvt.IsRight())
                    mpView->BegDragObj(aMDPos, (OutputDevice*) NULL, aVEvt.pHdl, nDrgLog);
            }
            else if (mpView->AreObjectsMarked())
            {
                /**************************************************************
                * Select glue point
                **************************************************************/
                if (!rMEvt.IsShift())
                    mpView->UnmarkAllPoints();

                if ( ! rMEvt.IsRight())
                    mpView->BegMarkPoints(aMDPos);
            }
            else
            {
                /**************************************************************
                * Select object
                **************************************************************/
                if ( ! rMEvt.IsRight())
                    mpView->BegMarkObj(aMDPos);
            }

            ForcePointer(&rMEvt);
        }
    }

    if (!bIsInDragMode)
    {
        ForcePointer(&rMEvt);
    }

    return bReturn;
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/

sal_Bool FuSelection::MouseMove(const MouseEvent& rMEvt)
{
    sal_Bool bReturn = FuDraw::MouseMove(rMEvt);

    if (aDragTimer.IsActive())
    {
        if(bFirstMouseMove)
        {
            bFirstMouseMove = sal_False;
        }
        else
        {
            aDragTimer.Stop();
        }
    }

    if (mpView->IsAction())
    {
        Point aPix(rMEvt.GetPosPixel());
        Point aPnt(mpWindow->PixelToLogic(aPix));

        ForceScroll(aPix);

        if (mpView->IsInsObjPoint())
        {
            mpView->MovInsObjPoint(aPnt);
        }
        else
        {
            mpView->MovAction(aPnt);
        }
    }

    ForcePointer(&rMEvt);

    return (bReturn);
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

sal_Bool FuSelection::MouseButtonUp(const MouseEvent& rMEvt)
{
    sal_Bool bReturn = sal_False;
    // When the right mouse button is pressed then only select objects
    // (and deselect others) as a preparation for showing the context
    // menu.
    const bool bSelectionOnly = rMEvt.IsRight();

    if (bHideAndAnimate)
    {
        // Animation is still running -> return immediately
        bHideAndAnimate = sal_False;
        pHdl = NULL;
        mpWindow->ReleaseMouse();
        return(sal_True);
    }

    if (aDragTimer.IsActive() )
    {
        aDragTimer.Stop();
        bIsInDragMode = sal_False;
    }

    if( !mpView )
        return (sal_False);

    Point aPnt( mpWindow->PixelToLogic( rMEvt.GetPosPixel() ) );
    sal_uInt16 nHitLog = sal_uInt16 ( mpWindow->PixelToLogic(Size(HITPIX,0)).Width() );
    sal_uInt16 nDrgLog = sal_uInt16 ( mpWindow->PixelToLogic(Size(DRGPIX,0)).Width() );

    if (mpView->IsFrameDragSingles() || !mpView->HasMarkablePoints())
    {
        /**********************************************************************
        * NO BEZIER_EDITOR
        **********************************************************************/
        if ( mpView->IsDragObj() )
        {
            /******************************************************************
            * Object was moved
            ******************************************************************/
            FrameView* pFrameView = mpViewShell->GetFrameView();
            sal_Bool bDragWithCopy = (rMEvt.IsMod1() && pFrameView->IsDragWithCopy());

            if (bDragWithCopy)
            {
                bDragWithCopy = !mpView->IsPresObjSelected(sal_False, sal_True);
            }

            mpView->SetDragWithCopy(bDragWithCopy);
            mpView->EndDragObj( mpView->IsDragWithCopy() );

            mpView->ForceMarkedToAnotherPage();

            if (!rMEvt.IsShift() && !rMEvt.IsMod1() && !rMEvt.IsMod2() &&
                !bSelectionChanged                   &&
                Abs(aPnt.X() - aMDPos.X()) < nDrgLog &&
                Abs(aPnt.Y() - aMDPos.Y()) < nDrgLog)
            {
                /**************************************************************
                * Toggle between selection and rotation
                **************************************************************/
                SdrObject* pSingleObj = NULL;
                sal_uLong nMarkCount = mpView->GetMarkedObjectList().GetMarkCount();

                if (nMarkCount==1)
                {
                    pSingleObj = mpView->GetMarkedObjectList().GetMark(0)->GetMarkedSdrObj();
                }

                if (nSlotId == SID_OBJECT_SELECT
                    && mpView->IsRotateAllowed()

                    && (rMEvt.GetClicks() != 2)
                    && (mpViewShell->GetFrameView()->IsClickChangeRotation()
                        || (pSingleObj
                            && pSingleObj->GetObjInventor()==E3dInventor))
                    && ! bSelectionOnly)

                {
                    bTempRotation = sal_True;
                    nSlotId = SID_OBJECT_ROTATE;
                    Activate();
                }
                else if (nSlotId == SID_OBJECT_ROTATE)
                {
                    nSlotId = SID_OBJECT_SELECT;
                    Activate();
                }
            }
            else if (nSlotId == SID_CONVERT_TO_3D_LATHE)
            {
                if (!pHdl)
                {
                    bSuppressChangesOfSelection = sal_True;
                    mpView->Start3DCreation();
                    bSuppressChangesOfSelection = sal_False;
                }
                else if (pHdl->GetKind() != HDL_MIRX &&
                         pHdl->GetKind() != HDL_REF1 &&
                         pHdl->GetKind() != HDL_REF2 && mpView->Is3DRotationCreationActive())
                {
                    /*********************************************************
                    * If 3D-rotation bodies are about to be created,
                    * end creation now
                    **********************************************************/
                     long nAngle1  = GetAngle(aPnt - mpView->GetRef1());
                     nAngle1 -= 27000;
                     nAngle1 = NormAngle360(nAngle1);
                     sal_Bool bMirrorSide1 = sal_Bool (nAngle1 < 18000L);

                     if (bMirrorSide0 != bMirrorSide1)
                     {
                         bSuppressChangesOfSelection = sal_True;
                        if(mpWindow)
                            mpWindow->EnterWait();
                         mpView->End3DCreation();
                         bSuppressChangesOfSelection = sal_False;
                         nSlotId = SID_OBJECT_SELECT;
                        if(mpWindow)
                            mpWindow->LeaveWait();
                         Activate();
                    }
                }
            }
        }
        else if (rMEvt.IsMod1()
            && !rMEvt.IsMod2()
            && Abs(aPnt.X() - aMDPos.X()) < nDrgLog
            && Abs(aPnt.Y() - aMDPos.Y()) < nDrgLog)
        {
            // Enter group
            mpView->MarkObj(aPnt, nHitLog, rMEvt.IsShift(), rMEvt.IsMod1());
        }

        if (mpView->IsAction() )
        {
            mpView->EndAction();
        }

        if( SD_MOD()->GetWaterCan() )
        {
            if( rMEvt.IsRight() )
            {
                // In watering-can mode, on press onto right mouse button, a undo is executed
                mpViewShell->GetViewFrame()->GetDispatcher()->Execute( SID_UNDO, SFX_CALLMODE_ASYNCHRON );
            }
            else if (pWaterCanCandidate != NULL)
            {
                // Is the candiate object still under the mouse?
                if (pickObject (aPnt) == pWaterCanCandidate)
                {
                    SdStyleSheetPool* pPool = static_cast<SdStyleSheetPool*>(
                        mpDocSh->GetStyleSheetPool());
                    if (pPool != NULL)
                    {
                        SfxStyleSheet* pStyleSheet = static_cast<SfxStyleSheet*>(
                            pPool->GetActualStyleSheet());
                        if (pStyleSheet != NULL && mpView->IsUndoEnabled() )
                        {
                            // Added UNDOs for the WaterCan mode. This was never done in
                            // the past, thus it was missing all the time.
                            SdrUndoAction* pUndoAttr = mpDoc->GetSdrUndoFactory().CreateUndoAttrObject(*pWaterCanCandidate, sal_True, sal_True);
                            mpView->BegUndo(pUndoAttr->GetComment());
                            mpView->AddUndo(mpDoc->GetSdrUndoFactory().CreateUndoGeoObject(*pWaterCanCandidate));
                            mpView->AddUndo(pUndoAttr);

                            pWaterCanCandidate->SetStyleSheet (pStyleSheet, sal_False);

                            mpView->EndUndo();
                        }
                    }
                }
            }
            // else when there has been no object under the mouse when the
            // button was pressed then nothing happens even when there is
            // one now.
        }

        sal_uInt16 nClicks = rMEvt.GetClicks();

        if (nClicks == 2 && rMEvt.IsLeft() && bMBDown &&
            !rMEvt.IsMod1() && !rMEvt.IsShift() )
        {
            DoubleClick(rMEvt);
        }

        bMBDown = sal_False;

        ForcePointer(&rMEvt);
        pHdl = NULL;
        mpWindow->ReleaseMouse();
        SdrObject* pSingleObj = NULL;
        sal_uLong nMarkCount = mpView->GetMarkedObjectList().GetMarkCount();

        if (nMarkCount==1)
        {
            pSingleObj = mpView->GetMarkedObjectList().GetMark(0)->GetMarkedSdrObj();
        }

        if ( (nSlotId != SID_OBJECT_SELECT && nMarkCount==0)                    ||
             ( mpView->GetDragMode() == SDRDRAG_CROOK &&
              !mpView->IsCrookAllowed( mpView->IsCrookNoContortion() ) ) ||
             ( mpView->GetDragMode() == SDRDRAG_SHEAR &&
              !mpView->IsShearAllowed() && !mpView->IsDistortAllowed() ) ||
             ( nSlotId==SID_CONVERT_TO_3D_LATHE && pSingleObj &&
              (pSingleObj->GetObjInventor() != SdrInventor         ||
               pSingleObj->GetObjIdentifier() == OBJ_MEASURE) ) )
        {
            bReturn = sal_True;
            ForcePointer(&rMEvt);
            pHdl = NULL;
            mpWindow->ReleaseMouse();
            FuDraw::MouseButtonUp(rMEvt);
            mpViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SFX_CALLMODE_SYNCHRON);
            return bReturn; // CAUTION, due to the synchronous slot, the object is deleted now.
        }

        FuDraw::MouseButtonUp(rMEvt);
    }
    else
    {
        /**********************************************************************
        * BEZIER_EDITOR
        **********************************************************************/
        if ( mpView->IsAction() )
        {
            if ( mpView->IsInsObjPoint() )
            {
                mpView->EndInsObjPoint(SDRCREATE_FORCEEND);
            }
            else if ( mpView->IsDragObj() )
            {
                FrameView* pFrameView = mpViewShell->GetFrameView();
                sal_Bool bDragWithCopy = (rMEvt.IsMod1() && pFrameView->IsDragWithCopy());

                if (bDragWithCopy)
                {
                    bDragWithCopy = !mpView->IsPresObjSelected(sal_False, sal_True);
                }

                mpView->SetDragWithCopy(bDragWithCopy);
                mpView->EndDragObj( mpView->IsDragWithCopy() );
            }
            else
            {
                mpView->EndAction();

                sal_uInt16 nDrgLog2 = sal_uInt16 ( mpWindow->PixelToLogic(Size(DRGPIX,0)).Width() );
                Point aPos = mpWindow->PixelToLogic( rMEvt.GetPosPixel() );

                if (Abs(aMDPos.X() - aPos.X()) < nDrgLog2 &&
                    Abs(aMDPos.Y() - aPos.Y()) < nDrgLog2 &&
                    !rMEvt.IsShift() && !rMEvt.IsMod2())
                {
                    SdrViewEvent aVEvt;
                    SdrHitKind eHit = mpView->PickAnything(rMEvt, SDRMOUSEBUTTONDOWN, aVEvt);

                    if (eHit == SDRHIT_NONE)
                    {
                        // Click on the same place - unselect
                        mpView->UnmarkAllObj();
                    }
                }
            }
        }
        else if (!rMEvt.IsShift() && rMEvt.IsMod1() && !rMEvt.IsMod2() &&
                 Abs(aPnt.X() - aMDPos.X()) < nDrgLog &&
                 Abs(aPnt.Y() - aMDPos.Y()) < nDrgLog)
        {
            // Enter group
            mpView->MarkObj(aPnt, nHitLog, sal_False, rMEvt.IsMod1());
        }


        ForcePointer(&rMEvt);
        pHdl = NULL;
        mpWindow->ReleaseMouse();

        FuDraw::MouseButtonUp(rMEvt);
    }

    return (bReturn);
}

/*************************************************************************
|*
|* Process keyboard input
|*
|* If a KeyEvent is being processed, the Return-Wert is sal_True,
|* sal_False otherwise.
|*
\************************************************************************/

sal_Bool FuSelection::KeyInput(const KeyEvent& rKEvt)
{
    sal_Bool bReturn = sal_False;

    switch (rKEvt.GetKeyCode().GetCode())
    {
        case KEY_ESCAPE:
        {
            bReturn = FuSelection::cancel();
        }
        break;
    }

    if (!bReturn)
    {
        bReturn = FuDraw::KeyInput(rKEvt);

        if(mpView->GetMarkedObjectList().GetMarkCount() == 0)
        {
            mpView->ResetCreationActive();

            mpViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
        }
    }

    return(bReturn);

}


/*************************************************************************
|*
|* Activate function
|*
\************************************************************************/

void FuSelection::Activate()
{
    SdrDragMode eMode;
    mpView->ResetCreationActive();
    mpView->SetEditMode(SDREDITMODE_EDIT);

    switch( nSlotId )
    {
        case SID_OBJECT_ROTATE:
        {
            // (mapped) Slot is explicitly set to rotate
            if( mpViewShell->ISA(DrawViewShell) )
            {
                sal_uInt16* pSlotArray =
                    static_cast<DrawViewShell*>(mpViewShell)->GetSlotArray();
                pSlotArray[ 1 ] = SID_OBJECT_ROTATE;
            }

            eMode = SDRDRAG_ROTATE;

            if ( mpView->GetDragMode() != eMode )
                mpView->SetDragMode(eMode);
        }
        break;

        case SID_OBJECT_MIRROR:
        {
            eMode = SDRDRAG_MIRROR;

            if ( mpView->GetDragMode() != eMode )
                mpView->SetDragMode(eMode);
        }
        break;

        case SID_OBJECT_CROP:
        {
            eMode = SDRDRAG_CROP;

            if ( mpView->GetDragMode() != eMode )
                mpView->SetDragMode(eMode);
        }
        break;

        case SID_OBJECT_TRANSPARENCE:
        {
            eMode = SDRDRAG_TRANSPARENCE;

            if ( mpView->GetDragMode() != eMode )
                mpView->SetDragMode(eMode);
        }
        break;

        case SID_OBJECT_GRADIENT:
        {
            eMode = SDRDRAG_GRADIENT;

            if ( mpView->GetDragMode() != eMode )
                mpView->SetDragMode(eMode);
        }
        break;

        case SID_OBJECT_SHEAR:
        {
            eMode = SDRDRAG_SHEAR;

            if ( mpView->GetDragMode() != eMode )
                mpView->SetDragMode(eMode);
        }
        break;

        case SID_OBJECT_CROOK_ROTATE:
        {
            eMode = SDRDRAG_CROOK;

            if ( mpView->GetDragMode() != eMode )
            {
                mpView->SetDragMode(eMode);
                mpView->SetCrookMode(SDRCROOK_ROTATE);
            }
        }
        break;

        case SID_OBJECT_CROOK_SLANT:
        {
            eMode = SDRDRAG_CROOK;

            if ( mpView->GetDragMode() != eMode )
            {
                mpView->SetDragMode(eMode);
                mpView->SetCrookMode(SDRCROOK_SLANT);
            }
        }
        break;

        case SID_OBJECT_CROOK_STRETCH:
        {
            eMode = SDRDRAG_CROOK;

            if ( mpView->GetDragMode() != eMode )
            {
                mpView->SetDragMode(eMode);
                mpView->SetCrookMode(SDRCROOK_STRETCH);
            }
        }
        break;

        case SID_CONVERT_TO_3D_LATHE:
        {
            eMode = SDRDRAG_MIRROR;
            bSuppressChangesOfSelection = sal_True;

            if ( mpView->GetDragMode() != eMode )
                mpView->SetDragMode(eMode);

            if (!mpView->Is3DRotationCreationActive())
                mpView->Start3DCreation();

            bSuppressChangesOfSelection = sal_False;
        }
        break;

        default:
        {
            eMode = SDRDRAG_MOVE;

            if ( mpView->GetDragMode() != eMode )
                mpView->SetDragMode(eMode);
        }
        break;
    }

    if (nSlotId != SID_OBJECT_ROTATE)
    {
        bTempRotation = sal_False;
    }

    FuDraw::Activate();
}



/*************************************************************************
|*
|* Deactivate function
|*
\************************************************************************/

void FuSelection::Deactivate()
{
    FuDraw::Deactivate();
}


/*************************************************************************
|*
|* Selection was changed
|*
\************************************************************************/

void FuSelection::SelectionHasChanged()
{
    bSelectionChanged = sal_True;

    FuDraw::SelectionHasChanged();

    if ((mpView->Is3DRotationCreationActive() && !bSuppressChangesOfSelection))
    {
        // Switch rotation body -> selection
        mpView->ResetCreationActive();
        nSlotId = SID_OBJECT_SELECT;
        Activate();
    }

    // Activate the right tool bar for the current context of the view.
    mpViewShell->GetViewShellBase().GetToolBarManager()->SelectionHasChanged(*mpViewShell, *mpView);
}


/*************************************************************************
|*
|* Set current bezier edit mode
|*
\************************************************************************/

void FuSelection::SetEditMode(sal_uInt16 nMode)
{
    nEditMode = nMode;

    if (nEditMode == SID_BEZIER_INSERT)
    {
        mpView->SetInsObjPointMode(sal_True);
    }
    else
    {
        mpView->SetInsObjPointMode(sal_False);
    }

    ForcePointer();

    SfxBindings& rBindings = mpViewShell->GetViewFrame()->GetBindings();
    rBindings.Invalidate(SID_BEZIER_MOVE);
    rBindings.Invalidate(SID_BEZIER_INSERT);
}

/*************************************************************************
|*
|* Execute animation or interaction
|*
\************************************************************************/

sal_Bool FuSelection::AnimateObj(SdrObject* pObj, const Point& rPos)
{
    sal_Bool bAnimated = sal_False;
    sal_Bool bClosed = pObj->IsClosedObj();
    sal_Bool bFilled = sal_False;

    if (bClosed)
    {
        SfxItemSet aSet(mpDoc->GetPool());

        aSet.Put(pObj->GetMergedItemSet());

        const XFillStyleItem& rFillStyle = (const XFillStyleItem&) aSet.Get(XATTR_FILLSTYLE);
        bFilled = rFillStyle.GetValue() != XFILL_NONE;
    }

    const SetOfByte* pVisiLayer = &mpView->GetSdrPageView()->GetVisibleLayers();
    sal_uInt16 nHitLog = sal_uInt16 ( mpWindow->PixelToLogic(Size(HITPIX,0)).Width() );
    const long  n2HitLog = nHitLog * 2;
    Point aHitPosR(rPos);
    Point aHitPosL(rPos);
    Point aHitPosT(rPos);
    Point aHitPosB(rPos);

    aHitPosR.X() += n2HitLog;
    aHitPosL.X() -= n2HitLog;
    aHitPosT.Y() += n2HitLog;
    aHitPosB.Y() -= n2HitLog;

    if ( !bClosed                                      ||
         !bFilled                                      ||
         (SdrObjectPrimitiveHit(*pObj, aHitPosR, nHitLog, *mpView->GetSdrPageView(), pVisiLayer, false) &&
          SdrObjectPrimitiveHit(*pObj, aHitPosL, nHitLog, *mpView->GetSdrPageView(), pVisiLayer, false) &&
          SdrObjectPrimitiveHit(*pObj, aHitPosT, nHitLog, *mpView->GetSdrPageView(), pVisiLayer, false) &&
          SdrObjectPrimitiveHit(*pObj, aHitPosB, nHitLog, *mpView->GetSdrPageView(), pVisiLayer, false) ) )
    {
        if ( mpDoc->GetIMapInfo( pObj ) )
        {
            const IMapObject* pIMapObj = mpDoc->GetHitIMapObject( pObj, rPos, *mpWindow );

            if ( pIMapObj && pIMapObj->GetURL().Len() )
            {
                // Jump to Document
                mpWindow->ReleaseMouse();
                SfxStringItem aStrItem(SID_FILE_NAME, pIMapObj->GetURL());
                SfxStringItem aReferer(SID_REFERER, mpDocSh->GetMedium()->GetName());
                SfxViewFrame* pFrame = mpViewShell->GetViewFrame();
                SfxFrameItem aFrameItem(SID_DOCFRAME, pFrame);
                SfxBoolItem aBrowseItem( SID_BROWSE, sal_True );
                mpWindow->ReleaseMouse();
                pFrame->GetDispatcher()->
                    Execute(SID_OPENDOC, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD,
                            &aStrItem, &aFrameItem, &aBrowseItem, &aReferer, 0L);

                bAnimated = sal_True;
            }
        }
        else if (!mpDocSh->ISA(GraphicDocShell)        &&
                 mpView->ISA(DrawView)                 &&
                 mpDoc->GetAnimationInfo(pObj))
        {
            /**********************************************************
            * Animation-object hit in the middle -> interaction
            **********************************************************/
            SdAnimationInfo* pInfo = mpDoc->GetAnimationInfo(pObj);
            DrawViewShell* pDrViewSh = static_cast<DrawViewShell*>(mpViewShell);
            mpWindow->ReleaseMouse();

            switch (pInfo->meClickAction)
            {
                case presentation::ClickAction_BOOKMARK:
                {
                     // Jump to Bookmark (Page or Object)
                    SfxStringItem aItem(SID_NAVIGATOR_OBJECT, pInfo->GetBookmark());
                    mpViewShell->GetViewFrame()->GetDispatcher()->
                    Execute(SID_NAVIGATOR_OBJECT, SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD, &aItem, 0L);
                    bAnimated = sal_True;
                }
                break;

                case presentation::ClickAction_DOCUMENT:
                {
                    String sBookmark( pInfo->GetBookmark() );
                    // Jump to document
                    if (sBookmark.Len())
                    {
                        SfxStringItem aReferer(SID_REFERER, mpDocSh->GetMedium()->GetName());
                        SfxStringItem aStrItem(SID_FILE_NAME, sBookmark);
                        SfxViewFrame* pFrame = mpViewShell->GetViewFrame();
                        SfxFrameItem aFrameItem(SID_DOCFRAME, pFrame);
                        SfxBoolItem aBrowseItem( SID_BROWSE, sal_True );
                        pFrame->GetDispatcher()->
                        Execute(SID_OPENDOC, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD,
                                &aStrItem, &aFrameItem, &aBrowseItem, &aReferer, 0L);
                    }

                    bAnimated = sal_True;
                }
                break;

                case presentation::ClickAction_PREVPAGE:
                {
                    // Jump to the previous page
                    SfxUInt16Item aItem(SID_NAVIGATOR_PAGE, PAGE_PREVIOUS);
                    mpViewShell->GetViewFrame()->GetDispatcher()->
                    Execute(SID_NAVIGATOR_PAGE, SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD,
                            &aItem, 0L);
                    bAnimated = sal_True;
                }
                break;

                case presentation::ClickAction_NEXTPAGE:
                {
                    // Jump to the next page
                    SfxUInt16Item aItem(SID_NAVIGATOR_PAGE, PAGE_NEXT);
                    mpViewShell->GetViewFrame()->GetDispatcher()->
                    Execute(SID_NAVIGATOR_PAGE, SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD,
                            &aItem, 0L);
                    bAnimated = sal_True;
                }
                break;

                case presentation::ClickAction_FIRSTPAGE:
                {
                    // Jump to the first page
                    SfxUInt16Item aItem(SID_NAVIGATOR_PAGE, PAGE_FIRST);
                    mpViewShell->GetViewFrame()->GetDispatcher()->
                    Execute(SID_NAVIGATOR_PAGE, SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD,
                            &aItem, 0L);
                    bAnimated = sal_True;
                }
                break;

                case presentation::ClickAction_LASTPAGE:
                {
                    // Jump to the last page
                    SfxUInt16Item aItem(SID_NAVIGATOR_PAGE, PAGE_LAST);
                    mpViewShell->GetViewFrame()->GetDispatcher()->
                    Execute(SID_NAVIGATOR_PAGE, SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD,
                            &aItem, 0L);
                    bAnimated = sal_True;
                }
                break;

                case presentation::ClickAction_SOUND:
                {
                        try
                        {
                            mxPlayer.set( avmedia::MediaWindow::createPlayer( pInfo->GetBookmark()), uno::UNO_QUERY_THROW );
                            mxPlayer->start();
                        }
                        catch( uno::Exception& )
                        {
                        }
                    bAnimated = sal_True;
                }
                break;

                case presentation::ClickAction_VERB:
                {
                    // Assign verb
                    mpView->UnmarkAll();
                    mpView->MarkObj(pObj, mpView->GetSdrPageView(), sal_False, sal_False);
                    pDrViewSh->DoVerb((sal_Int16)pInfo->mnVerb);
                    bAnimated = sal_True;
                }
                break;

                case presentation::ClickAction_PROGRAM:
                {
                   String aBaseURL = GetDocSh()->GetMedium()->GetBaseURL();
                   INetURLObject aURL( ::URIHelper::SmartRel2Abs( INetURLObject(aBaseURL), pInfo->GetBookmark(),
                                                URIHelper::GetMaybeFileHdl(), true, false,
                                                INetURLObject::WAS_ENCODED, INetURLObject::DECODE_UNAMBIGUOUS ) );

                   if( INET_PROT_FILE == aURL.GetProtocol() )
                   {
                        SfxStringItem aUrl( SID_FILE_NAME, aURL.GetMainURL( INetURLObject::NO_DECODE ) );
                        SfxBoolItem aBrowsing( SID_BROWSE, sal_True );

                        SfxViewFrame* pViewFrm = SfxViewFrame::Current();
                        if (pViewFrm)
                            pViewFrm->GetDispatcher()->Execute( SID_OPENDOC,
                                                          SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD,
                                                        &aUrl,
                                                        &aBrowsing,
                                                        0L );
                   }

                    bAnimated = sal_True;
                }
                break;

                case presentation::ClickAction_MACRO:
                {
                    // Execute macro
                    String aMacro = pInfo->GetBookmark();

                    if ( SfxApplication::IsXScriptURL( aMacro ) )
                    {
                        uno::Any aRet;
                        uno::Sequence< sal_Int16 > aOutArgsIndex;
                        uno::Sequence< uno::Any > aOutArgs;
                        uno::Sequence< uno::Any >* pInArgs =
                            new uno::Sequence< uno::Any >(0);

                        ErrCode eErr = mpDocSh->CallXScript( aMacro,
                            *pInArgs, aRet, aOutArgsIndex, aOutArgs);

                        // Check the return value from the script
                        sal_Bool bTmp = sal_False;
                        if ( eErr == ERRCODE_NONE &&
                             aRet.getValueType() == getCppuBooleanType() &&
                             sal_True == ( aRet >>= bTmp ) &&
                             bTmp == sal_True )
                        {
                            bAnimated = sal_True;
                        }
                        else
                        {
                            bAnimated = sal_False;
                        }
                    }
                    else
                    {
                        // aMacro has got following format:
                        // "Macroname.Modulname.Libname.Documentname" or
                        // "Macroname.Modulname.Libname.Applicationsname"
                        String aMacroName =
                            aMacro.GetToken(0, sal_Unicode('.'));
                        String aModulName =
                             aMacro.GetToken(1, sal_Unicode('.'));
                        String aLibName   =
                             aMacro.GetToken(2, sal_Unicode('.'));
                        String aDocName   =
                             aMacro.GetToken(3, sal_Unicode('.'));

                        // In this moment the Call-method only
                        // resolves modulename+macroname
                        String aExecMacro(aModulName);
                        aExecMacro.Append( sal_Unicode('.') );
                        aExecMacro.Append( aMacroName );
                        bAnimated = mpDocSh->GetBasic()->Call(aExecMacro);
                    }
                }
                break;

                default:
                {
                    bAnimated = sal_False;
                }
                break;
            }
        }

        if (!bAnimated                               &&
            mpView->ISA(DrawView)                 &&
            !mpDocSh->ISA(GraphicDocShell)        &&
            SlideShow::IsRunning( mpViewShell->GetViewShellBase() ) &&
            mpDoc->GetAnimationInfo(pObj))
        {
            /**********************************************************
            * Effect-Object hit in the middle -> Play effect
            **********************************************************/
            SdAnimationInfo* pInfo = mpDoc->GetAnimationInfo(pObj);

            switch (pInfo->meClickAction)
            {
                case presentation::ClickAction_VANISH:
                case presentation::ClickAction_INVISIBLE:
                    break;

                default:
                    bAnimated = sal_False;
                break;
            }
        }
    }

    return bAnimated;
}



/** is called when the current function should be aborted. <p>
    This is used when a function gets a KEY_ESCAPE but can also
    be called directly.

    @returns true if a active function was aborted
*/
bool FuSelection::cancel()
{
    if (mpView->Is3DRotationCreationActive())
    {
        mpView->ResetCreationActive();
        mpViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
        return true;
    }
    else
    {
        return false;
    }
}




SdrObject* FuSelection::pickObject (const Point& rTestPoint)
{
    SdrObject* pObject = NULL;
    SdrPageView* pPageView;
    sal_uInt16 nHitLog = sal_uInt16 (mpWindow->PixelToLogic(Size(HITPIX,0)).Width());
    mpView->PickObj (rTestPoint, nHitLog, pObject, pPageView, SDRSEARCH_PICKMARKABLE);
    return pObject;
}
} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
