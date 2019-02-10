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

#include <config_features.h>

#include <com/sun/star/media/XPlayer.hpp>
#include <fusel.hxx>
#include <basic/sbstar.hxx>
#include <svx/svddrgmt.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdogrp.hxx>
#include <svx/scene3d.hxx>
#include <drawview.hxx>
#include <vcl/imapobj.hxx>
#include <svl/urihelper.hxx>
#include <unotools/localfilehelper.hxx>
#include <unotools/securityoptions.hxx>
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

#include <app.hrc>

#include <GraphicDocShell.hxx>
#include <sdmod.hxx>
#include <DrawDocShell.hxx>
#include <stlpool.hxx>
#include <anminfo.hxx>
#include <fudraw.hxx>
#include <ViewShell.hxx>
#include <ViewShellBase.hxx>
#include <FrameView.hxx>
#include <View.hxx>
#include <Window.hxx>
#include <drawdoc.hxx>
#include <sdpage.hxx>
#include <DrawViewShell.hxx>
#include <ToolBarManager.hxx>
#include <pgjump.hxx>
#include <svx/globl3d.hxx>
#include <Client.hxx>

#include <slideshow.hxx>

#include <svx/svdundo.hxx>
#include <avmedia/mediawindow.hxx>

#include <svx/sdrhittesthelper.hxx>

#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <comphelper/lok.hxx>

using namespace ::com::sun::star;

namespace sd {

FuSelection::FuSelection (
    ViewShell* pViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument* pDoc,
    SfxRequest& rReq)
    : FuDraw(pViewSh, pWin, pView, pDoc, rReq),
      bTempRotation(false),
      bSelectionChanged(false),
      pHdl(nullptr),
      bSuppressChangesOfSelection(false),
      bMirrorSide0(false),
      nEditMode(SID_BEZIER_MOVE),
      pWaterCanCandidate(nullptr)
     //Add Shift+UP/DOWN/LEFT/RIGHT key to move the position of insert point,
     //and SHIFT+ENTER key to decide the position and draw the new insert point
    ,bBeginInsertPoint(false),
      oldPoint(0,0)
  ,bMovedToCenterPoint(false)
{
}

rtl::Reference<FuPoor> FuSelection::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    rtl::Reference<FuPoor> xFunc( new FuSelection( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuSelection::DoExecute( SfxRequest& rReq )
{
    FuDraw::DoExecute( rReq );

    // Select object bar
    SelectionHasChanged();
}

FuSelection::~FuSelection()
{
    mpView->UnmarkAllPoints();
    mpView->ResetCreationActive();

    if ( mpView->GetDragMode() != SdrDragMode::Move )
    {
        mpView->SetDragMode(SdrDragMode::Move);
    }
}

bool FuSelection::MouseButtonDown(const MouseEvent& rMEvt)
{
    pHdl = nullptr;
    bool bReturn = FuDraw::MouseButtonDown(rMEvt);
    bool bWaterCan = SD_MOD()->GetWaterCan();
    const bool bReadOnly = mpDocSh->IsReadOnly();
    // When the right mouse button is pressed then only select objects
    // (and deselect others) as a preparation for showing the context
    // menu.
    const bool bSelectionOnly = rMEvt.IsRight();

    bMBDown = true;
    bSelectionChanged = false;

    if ( mpView->IsAction() )
    {
        if ( rMEvt.IsRight() )
            mpView->BckAction();
        return true;
    }

    sal_uInt16 nDrgLog = sal_uInt16 ( mpWindow->PixelToLogic(Size(DRGPIX,0)).Width() );
    sal_uInt16 nHitLog = sal_uInt16 ( mpWindow->PixelToLogic(Size(HITPIX,0)).Width() );

    if (comphelper::LibreOfficeKit::isActive())
    {
        // When tiled rendering, we always work in logic units, use the non-pixel constants.
        nDrgLog = DRGLOG;
        nHitLog = HITLOG;
    }

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

        long nAngle0  = GetAngle(aMDPos - mpView->GetRef1());
        nAngle0 -= 27000;
        nAngle0 = NormAngle36000(nAngle0);
        bMirrorSide0 = nAngle0 < 18000;

        if (!pHdl && mpView->Is3DRotationCreationActive())
        {
            /******************************************************************
            * If 3D-rotation bodies are about to be created,
            * end creation now.
            ******************************************************************/
            bSuppressChangesOfSelection = true;
            mpWindow->EnterWait();
            mpView->End3DCreation();
            bSuppressChangesOfSelection = false;
            mpView->ResetCreationActive();
            mpWindow->LeaveWait();
        }

        bool bTextEdit = false;
        SdrViewEvent aVEvt;
        SdrHitKind eHit = mpView->PickAnything(rMEvt, SdrMouseEventKind::BUTTONDOWN, aVEvt);

        if ( eHit == SdrHitKind::TextEditObj && ( mpViewShell->GetFrameView()->IsQuickEdit() || dynamic_cast< sdr::table::SdrTableObj* >( aVEvt.pObj ) != nullptr ) )
        {
            bTextEdit = true;
        }

        if(!bTextEdit
            && !mpDocSh->IsReadOnly()
            && ((mpView->IsMarkedHit(aMDPos, nHitLog) && !rMEvt.IsShift() && !rMEvt.IsMod2()) || pHdl != nullptr)
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
                // hit handle or marked object
                bFirstMouseMove = true;
                aDragTimer.Start();
            }

            if ( ! rMEvt.IsRight())
                if (mpView->BegDragObj(aMDPos, nullptr, pHdl, nDrgLog))
                    mpView->GetDragMethod()->SetShiftPressed( rMEvt.IsShift() );
            bReturn = true;
        }
        else
        {
            SdrPageView* pPV = nullptr;
            SdrObject* pObj = !rMEvt.IsMod2() ? mpView->PickObj(aMDPos, mpView->getHitTolLog(), pPV, SdrSearchOptions::PICKMACRO) : nullptr;
            if (pObj)
            {
                mpView->BegMacroObj(aMDPos, nHitLog, pObj, pPV, mpWindow);
                bReturn = true;
            }
            else if ( bTextEdit )
            {
                sal_uInt16 nSdrObjKind = aVEvt.pObj->GetObjIdentifier();

                if (aVEvt.pObj->GetObjInventor() == SdrInventor::Default &&
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
                    ExecuteList(SID_TEXTEDIT,
                            SfxCallMode::SYNCHRON | SfxCallMode::RECORD,
                            { &aItem });
                    return bReturn; // CAUTION, due to the synchronous slot the object is deleted now
                }
            }
            else if ( !rMEvt.IsMod2() && rMEvt.GetClicks() == 1 &&
                      aVEvt.eEvent == SdrEventKind::ExecuteUrl )
             {
                mpWindow->ReleaseMouse();

                SvtSecurityOptions aSecOpt;
                if (!rMEvt.IsMod1() && aSecOpt.IsOptionSet(SvtSecurityOptions::EOption::CtrlClickHyperlink))
                    return true;
                if (rMEvt.IsMod1() && !aSecOpt.IsOptionSet(SvtSecurityOptions::EOption::CtrlClickHyperlink))
                    return true;

                SfxStringItem aStrItem(SID_FILE_NAME, aVEvt.pURLField->GetURL());
                SfxStringItem aReferer(SID_REFERER, mpDocSh->GetMedium()->GetName());
                SfxBoolItem aBrowseItem( SID_BROWSE, true );
                SfxViewFrame* pFrame = mpViewShell->GetViewFrame();
                mpWindow->ReleaseMouse();

                // If tiled rendering, let client handles URL execution and early returns.
                if (comphelper::LibreOfficeKit::isActive())
                {
                    SfxViewShell& rSfxViewShell = mpViewShell->GetViewShellBase();
                    rSfxViewShell.libreOfficeKitViewCallback(LOK_CALLBACK_HYPERLINK_CLICKED, aVEvt.pURLField->GetURL().toUtf8().getStr());
                    return true;
                }

                if (rMEvt.IsMod1())
                {
                    // Open in new frame
                    pFrame->GetDispatcher()->ExecuteList(SID_OPENDOC,
                        SfxCallMode::ASYNCHRON | SfxCallMode::RECORD,
                        { &aStrItem, &aBrowseItem, &aReferer });
                }
                else
                {
                    // Open in current frame
                    SfxFrameItem aFrameItem(SID_DOCFRAME, pFrame);
                    pFrame->GetDispatcher()->ExecuteList(SID_OPENDOC,
                        SfxCallMode::ASYNCHRON | SfxCallMode::RECORD,
                        { &aStrItem, &aFrameItem, &aBrowseItem, &aReferer });
                }

                bReturn = true;
            }
            else if(!rMEvt.IsMod2()
                && dynamic_cast< const DrawViewShell *>( mpViewShell ) !=  nullptr
                )
            {
                pObj = mpView->PickObj(aMDPos, mpView->getHitTolLog(), pPV, SdrSearchOptions::ALSOONMASTER);
                if (pObj)
                {
                    // Animate object when not just selecting.
                    if ( ! bSelectionOnly)
                        bReturn = AnimateObj(pObj, aMDPos);

                    if( !bReturn && (dynamic_cast< const SdrObjGroup *>( pObj ) != nullptr || dynamic_cast< const E3dScene* >(pObj) != nullptr))
                    {
                        if(rMEvt.GetClicks() == 1)
                        {
                            // Look into the group
                            pObj = mpView->PickObj(aMDPos, mpView->getHitTolLog(), pPV, SdrSearchOptions::ALSOONMASTER | SdrSearchOptions::DEEP);
                            if (pObj)
                                bReturn = AnimateObj(pObj, aMDPos);
                        }
                        else if( !bReadOnly && rMEvt.GetClicks() == 2)
                        {
                            // New: double click on selected Group object
                            // enter group
                            if ( ! bSelectionOnly
                                && pObj->getSdrPageFromSdrObject() == pPV->GetPage())
                                bReturn = pPV->EnterGroup(pObj);
                        }
                    }
                }

                // #i71727# replaced else here with two possibilities, once the original else (!pObj)
                // and also ignoring the found object when it's on a masterpage
                if(!pObj || (pObj->getSdrPageFromSdrObject() && pObj->getSdrPageFromSdrObject()->IsMasterPage()))
                {
                    if(mpView->IsGroupEntered() && 2 == rMEvt.GetClicks())
                    {
                        // New: double click on empty space/on obj on MasterPage, leave group
                        mpView->LeaveOneGroup();
                        bReturn = true;
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
                    bReturn = true;
                    bool bDeactivateOLE = false;

                    if ( !rMEvt.IsShift() && !rMEvt.IsMod2() )
                    {
                        OSL_ASSERT (mpViewShell->GetViewShell()!=nullptr);
                        Client* pIPClient = static_cast<Client*>(
                            mpViewShell->GetViewShell()->GetIPClient());

                        if (pIPClient && pIPClient->IsObjectInPlaceActive())
                        {
                            // OLE-object gets deactivated in subsequent UnmarkAll()
                            bDeactivateOLE = true;
                        }

                        mpView->UnmarkAll();
                    }

                    bool bMarked = false;

                    if ( !rMEvt.IsMod1() && !bDeactivateOLE)
                    {
                        if ( rMEvt.IsMod2() )
                        {
                            bMarked = mpView->MarkNextObj(aMDPos, nHitLog, rMEvt.IsShift() );
                        }
                        else
                        {
                            bool bToggle = false;

                            if (rMEvt.IsShift() && mpView->GetMarkedObjectList().GetMarkCount() > 1)
                            {
                                // No Toggle on single selection
                                bToggle = true;
                            }

                            bMarked = mpView->MarkObj(aMDPos, nHitLog, bToggle);
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
                                mpView->BegDragObj(aMDPos, nullptr, pHdl, nDrgLog);
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
        SdrHitKind eHit = mpView->PickAnything(rMEvt, SdrMouseEventKind::BUTTONDOWN, aVEvt);

        if (eHit == SdrHitKind::Handle && aVEvt.pHdl->GetKind() == SdrHdlKind::BezierWeight)
        {
            /******************************************************************
            * Drag Handle
            ******************************************************************/
            if ( ! rMEvt.IsRight())
                mpView->BegDragObj(aMDPos, nullptr, aVEvt.pHdl, nDrgLog);
        }
        else if (eHit == SdrHitKind::MarkedObject && nEditMode == SID_BEZIER_INSERT)
        {
            /******************************************************************
            * Insert glue point
            ******************************************************************/
            mpView->BegInsObjPoint(aMDPos, rMEvt.IsMod1());
        }
        else if (eHit == SdrHitKind::MarkedObject && rMEvt.IsMod1())
        {
            /******************************************************************
            * Select glue point
            ******************************************************************/
            if (!rMEvt.IsShift())
                mpView->UnmarkAllPoints();

            if ( ! rMEvt.IsRight())
                mpView->BegMarkPoints(aMDPos);
        }
        else if (eHit == SdrHitKind::MarkedObject && !rMEvt.IsShift() && !rMEvt.IsMod2())
        {
            /******************************************************************
            * Move object
            ******************************************************************/
            if ( ! rMEvt.IsRight())
                mpView->BegDragObj(aMDPos, nullptr, nullptr, nDrgLog);
        }
        else if (eHit == SdrHitKind::Handle)
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
                        pHdl = nullptr;
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
                        mpView->BegDragObj(aMDPos, nullptr, pHdl, nDrgLog);

                }
            }
            else
            {
                // Point IS marked and NO shift is pressed. Start
                // dragging of selected point(s)
                pHdl = mpView->PickHandle(aMDPos);
                if(pHdl && ! rMEvt.IsRight())
                    mpView->BegDragObj(aMDPos, nullptr, pHdl, nDrgLog);
            }
        }
        else
        {
            /******************************************************************
            * Select or drag object
            ******************************************************************/
            if (!rMEvt.IsShift() && !rMEvt.IsMod2() && eHit == SdrHitKind::UnmarkedObject)
            {
               mpView->UnmarkAllObj();
            }

            bool bMarked = false;

            if (!rMEvt.IsMod1())
            {
                if (rMEvt.IsMod2())
                {
                    bMarked = mpView->MarkNextObj(aMDPos, nHitLog, rMEvt.IsShift());
                }
                else
                {
                    bMarked = mpView->MarkObj(aMDPos, nHitLog, rMEvt.IsShift());
                }
            }

            if (bMarked &&
                (!rMEvt.IsShift() || eHit == SdrHitKind::MarkedObject))
            {
                // Move object
                if ( ! rMEvt.IsRight())
                    mpView->BegDragObj(aMDPos, nullptr, aVEvt.pHdl, nDrgLog);
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

bool FuSelection::MouseMove(const MouseEvent& rMEvt)
{
    bool bReturn = FuDraw::MouseMove(rMEvt);

    if (aDragTimer.IsActive())
    {
        if(bFirstMouseMove)
        {
            bFirstMouseMove = false;
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

    return bReturn;
}

bool FuSelection::MouseButtonUp(const MouseEvent& rMEvt)
{
    bool bReturn = false;
    // When the right mouse button is pressed then only select objects
    // (and deselect others) as a preparation for showing the context
    // menu.
    const bool bSelectionOnly = rMEvt.IsRight();

    if (aDragTimer.IsActive() )
    {
        aDragTimer.Stop();
        bIsInDragMode = false;
    }

    if( !mpView )
        return false;

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
            bool bDragWithCopy = (rMEvt.IsMod1() && pFrameView->IsDragWithCopy());

            if (bDragWithCopy)
            {
                bDragWithCopy = !mpView->IsPresObjSelected(false);
            }

            mpView->SetDragWithCopy(bDragWithCopy);
            mpView->EndDragObj( mpView->IsDragWithCopy() );

            mpView->ForceMarkedToAnotherPage();

            if (!rMEvt.IsShift() && !rMEvt.IsMod1() && !rMEvt.IsMod2() &&
                !bSelectionChanged                   &&
                std::abs(aPnt.X() - aMDPos.X()) < nDrgLog &&
                std::abs(aPnt.Y() - aMDPos.Y()) < nDrgLog)
            {
                /*************************************************************
                * If a user wants to click on an object in front of a marked
                * one, he releases the mouse button immediately
                **************************************************************/
                SdrPageView* pPV;
                SdrObject* pObj = mpView->PickObj(aMDPos, mpView->getHitTolLog(), pPV, SdrSearchOptions::ALSOONMASTER | SdrSearchOptions::BEFOREMARK);
                if (pObj && pPV->IsObjMarkable(pObj))
                {
                    mpView->UnmarkAllObj();
                    mpView->MarkObj(pObj,pPV);
                    return true;
                }
                /**************************************************************
                * Toggle between selection and rotation
                **************************************************************/
                SdrObject* pSingleObj = nullptr;

                if (mpView->GetMarkedObjectList().GetMarkCount()==1)
                {
                    pSingleObj = mpView->GetMarkedObjectList().GetMark(0)->GetMarkedSdrObj();
                }

                if (nSlotId == SID_OBJECT_SELECT
                    && mpView->IsRotateAllowed()

                    && (rMEvt.GetClicks() != 2)
                    && (mpViewShell->GetFrameView()->IsClickChangeRotation()
                        || (pSingleObj
                            && pSingleObj->GetObjInventor()==SdrInventor::E3d))
                    && ! bSelectionOnly)

                {
                    bTempRotation = true;
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
                    bSuppressChangesOfSelection = true;
                    mpView->Start3DCreation();
                    bSuppressChangesOfSelection = false;
                }
                else if (pHdl->GetKind() != SdrHdlKind::MirrorAxis &&
                         pHdl->GetKind() != SdrHdlKind::Ref1 &&
                         pHdl->GetKind() != SdrHdlKind::Ref2 && mpView->Is3DRotationCreationActive())
                {
                    /*********************************************************
                    * If 3D-rotation bodies are about to be created,
                    * end creation now
                    **********************************************************/
                     long nAngle1  = GetAngle(aPnt - mpView->GetRef1());
                     nAngle1 -= 27000;
                     nAngle1 = NormAngle36000(nAngle1);
                     bool bMirrorSide1 = nAngle1 < 18000;

                     if (bMirrorSide0 != bMirrorSide1)
                     {
                         bSuppressChangesOfSelection = true;
                         mpWindow->EnterWait();
                         mpView->End3DCreation();
                         bSuppressChangesOfSelection = false;
                         nSlotId = SID_OBJECT_SELECT;
                         mpWindow->LeaveWait();
                         Activate();
                    }
                }
            }
        }
        else if (rMEvt.IsMod1()
            && !rMEvt.IsMod2()
            && std::abs(aPnt.X() - aMDPos.X()) < nDrgLog
            && std::abs(aPnt.Y() - aMDPos.Y()) < nDrgLog)
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
                mpViewShell->GetViewFrame()->GetDispatcher()->Execute( SID_UNDO, SfxCallMode::ASYNCHRON );
            }
            else if (pWaterCanCandidate != nullptr)
            {
                // Is the candidate object still under the mouse?
                if (pickObject (aPnt) == pWaterCanCandidate)
                {
                    SdStyleSheetPool* pPool = static_cast<SdStyleSheetPool*>(
                        mpDocSh->GetStyleSheetPool());
                    if (pPool != nullptr)
                    {
                        SfxStyleSheet* pStyleSheet = static_cast<SfxStyleSheet*>(
                            pPool->GetActualStyleSheet());
                        if (pStyleSheet != nullptr && mpView->IsUndoEnabled() )
                        {
                            // Added UNDOs for the WaterCan mode. This was never done in
                            // the past, thus it was missing all the time.
                            std::unique_ptr<SdrUndoAction> pUndoAttr = mpDoc->GetSdrUndoFactory().CreateUndoAttrObject(*pWaterCanCandidate, true, true);
                            mpView->BegUndo(pUndoAttr->GetComment());
                            mpView->AddUndo(mpDoc->GetSdrUndoFactory().CreateUndoGeoObject(*pWaterCanCandidate));
                            mpView->AddUndo(std::move(pUndoAttr));

                            pWaterCanCandidate->SetStyleSheet (pStyleSheet, false);

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

        bMBDown = false;

        ForcePointer(&rMEvt);
        pHdl = nullptr;
        mpWindow->ReleaseMouse();
        SdrObject* pSingleObj = nullptr;
        const size_t nMarkCount = mpView->GetMarkedObjectList().GetMarkCount();

        if (nMarkCount==1)
        {
            pSingleObj = mpView->GetMarkedObjectList().GetMark(0)->GetMarkedSdrObj();
        }

        if ( (nSlotId != SID_OBJECT_SELECT && nMarkCount==0)                    ||
             ( mpView->GetDragMode() == SdrDragMode::Crook &&
              !mpView->IsCrookAllowed( mpView->IsCrookNoContortion() ) ) ||
             ( mpView->GetDragMode() == SdrDragMode::Shear &&
              !mpView->IsShearAllowed() && !mpView->IsDistortAllowed() ) ||
             ( nSlotId==SID_CONVERT_TO_3D_LATHE && pSingleObj &&
              (pSingleObj->GetObjInventor() != SdrInventor::Default         ||
               pSingleObj->GetObjIdentifier() == OBJ_MEASURE) ) )
        {
            bReturn = true;
            ForcePointer(&rMEvt);
            pHdl = nullptr;
            mpWindow->ReleaseMouse();
            FuDraw::MouseButtonUp(rMEvt);
            mpViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SfxCallMode::SYNCHRON);
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
                mpView->EndInsObjPoint(SdrCreateCmd::ForceEnd);
            }
            else if ( mpView->IsDragObj() )
            {
                FrameView* pFrameView = mpViewShell->GetFrameView();
                bool bDragWithCopy = (rMEvt.IsMod1() && pFrameView->IsDragWithCopy());

                if (bDragWithCopy)
                {
                    bDragWithCopy = !mpView->IsPresObjSelected(false);
                }

                mpView->SetDragWithCopy(bDragWithCopy);
                mpView->EndDragObj( mpView->IsDragWithCopy() );
            }
            else
            {
                mpView->EndAction();

                sal_uInt16 nDrgLog2 = sal_uInt16 ( mpWindow->PixelToLogic(Size(DRGPIX,0)).Width() );
                Point aPos = mpWindow->PixelToLogic( rMEvt.GetPosPixel() );

                if (std::abs(aMDPos.X() - aPos.X()) < nDrgLog2 &&
                    std::abs(aMDPos.Y() - aPos.Y()) < nDrgLog2 &&
                    !rMEvt.IsShift() && !rMEvt.IsMod2())
                {
                    SdrViewEvent aVEvt;
                    SdrHitKind eHit = mpView->PickAnything(rMEvt, SdrMouseEventKind::BUTTONDOWN, aVEvt);

                    if (eHit == SdrHitKind::NONE)
                    {
                        // Click on the same place - unselect
                        mpView->UnmarkAllObj();
                    }
                }
            }
        }
        else if (!rMEvt.IsShift() && rMEvt.IsMod1() && !rMEvt.IsMod2() &&
                 std::abs(aPnt.X() - aMDPos.X()) < nDrgLog &&
                 std::abs(aPnt.Y() - aMDPos.Y()) < nDrgLog)
        {
            // Enter group
            mpView->MarkObj(aPnt, nHitLog, false, rMEvt.IsMod1());
        }

        ForcePointer(&rMEvt);
        pHdl = nullptr;
        mpWindow->ReleaseMouse();

        FuDraw::MouseButtonUp(rMEvt);
    }

    return bReturn;
}

/**
 * Process keyboard input
 * @returns sal_True if a KeyEvent is being processed, sal_False otherwise
 */
bool FuSelection::KeyInput(const KeyEvent& rKEvt)
{
    bool bReturn = false;

    switch (rKEvt.GetKeyCode().GetCode())
    {
        case KEY_ESCAPE:
        {
            bReturn = FuSelection::cancel();
        }
        break;
        //add keyboard operation for insert points in drawing curve
        case KEY_UP:
        case KEY_DOWN:
        case KEY_LEFT:
        case KEY_RIGHT:
        {
            if(rKEvt.GetKeyCode().IsShift()&&(nEditMode == SID_BEZIER_INSERT)){
                long nX = 0;
                long nY = 0;
                sal_uInt16  nCode = rKEvt.GetKeyCode().GetCode();
                if (nCode == KEY_UP)
                {
                    // scroll up
                    nX = 0;
                    nY =-1;
                }
                else if (nCode == KEY_DOWN)
                {
                    // scroll down
                    nX = 0;
                    nY = 1;
                }
                else if (nCode == KEY_LEFT)
                {
                    // scroll left
                    nX =-1;
                    nY = 0;
                }
                else if (nCode == KEY_RIGHT)
                {
                    // scroll right
                    nX = 1;
                    nY = 0;
                }

                Point centerPoint;
                ::tools::Rectangle rect = mpView->GetMarkedObjRect();
                centerPoint = mpWindow->LogicToPixel(rect.Center());
                Point aPoint = bMovedToCenterPoint? oldPoint:centerPoint;
                Point ePoint = aPoint + Point(nX,nY);
                mpWindow->SetPointerPosPixel(ePoint);
                //simulate mouse move action
                MouseEvent eMevt(ePoint, 1, MouseEventModifiers::DRAGMOVE, MOUSE_LEFT, 0);
                MouseMove(eMevt);
                oldPoint = ePoint;
                bMovedToCenterPoint = true;
                bReturn = true;
            }
        }
        break;
        case KEY_RETURN:
            if(rKEvt.GetKeyCode().IsShift()&&(nEditMode == SID_BEZIER_INSERT))
            {
                if(!bBeginInsertPoint)
                {
                    //simulate mouse button down action
                    MouseEvent aMevt(oldPoint, 1,
                                     MouseEventModifiers::SIMPLEMOVE | MouseEventModifiers::DRAGMOVE,
                                     MOUSE_LEFT, KEY_SHIFT);
                    MouseButtonDown(aMevt);
                    mpWindow->CaptureMouse();
                    bBeginInsertPoint = true;
                }
                else
                {
                    //simulate mouse button up action
                    MouseEvent rMEvt(oldPoint, 1,
                                     MouseEventModifiers::SIMPLEMOVE | MouseEventModifiers::ENTERWINDOW,
                                     MOUSE_LEFT, KEY_SHIFT);
                    MouseButtonUp(rMEvt);
                    bBeginInsertPoint = false;
                }
                bReturn= true;
            }
            break;
    }
    if (!bReturn)
    {
        bReturn = FuDraw::KeyInput(rKEvt);

        if(mpView->GetMarkedObjectList().GetMarkCount() == 0)
        {
            mpView->ResetCreationActive();

            mpViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SfxCallMode::ASYNCHRON | SfxCallMode::RECORD);
        }
    }

    return bReturn;

}

void FuSelection::Activate()
{
    SdrDragMode eMode;
    mpView->ResetCreationActive();
    mpView->SetEditMode(SdrViewEditMode::Edit);

    switch( nSlotId )
    {
        case SID_OBJECT_ROTATE:
        {
            eMode = SdrDragMode::Rotate;

            if ( mpView->GetDragMode() != eMode )
                mpView->SetDragMode(eMode);
        }
        break;

        case SID_OBJECT_MIRROR:
        {
            eMode = SdrDragMode::Mirror;

            if ( mpView->GetDragMode() != eMode )
                mpView->SetDragMode(eMode);
        }
        break;

        case SID_OBJECT_CROP:
        {
            eMode = SdrDragMode::Crop;

            if ( mpView->GetDragMode() != eMode )
                mpView->SetDragMode(eMode);
        }
        break;

        case SID_OBJECT_TRANSPARENCE:
        {
            eMode = SdrDragMode::Transparence;

            if ( mpView->GetDragMode() != eMode )
                mpView->SetDragMode(eMode);
        }
        break;

        case SID_OBJECT_GRADIENT:
        {
            eMode = SdrDragMode::Gradient;

            if ( mpView->GetDragMode() != eMode )
                mpView->SetDragMode(eMode);
        }
        break;

        case SID_OBJECT_SHEAR:
        {
            eMode = SdrDragMode::Shear;

            if ( mpView->GetDragMode() != eMode )
                mpView->SetDragMode(eMode);
        }
        break;

        case SID_OBJECT_CROOK_ROTATE:
        {
            eMode = SdrDragMode::Crook;

            if ( mpView->GetDragMode() != eMode )
            {
                mpView->SetDragMode(eMode);
                mpView->SetCrookMode(SdrCrookMode::Rotate);
            }
        }
        break;

        case SID_OBJECT_CROOK_SLANT:
        {
            eMode = SdrDragMode::Crook;

            if ( mpView->GetDragMode() != eMode )
            {
                mpView->SetDragMode(eMode);
                mpView->SetCrookMode(SdrCrookMode::Slant);
            }
        }
        break;

        case SID_OBJECT_CROOK_STRETCH:
        {
            eMode = SdrDragMode::Crook;

            if ( mpView->GetDragMode() != eMode )
            {
                mpView->SetDragMode(eMode);
                mpView->SetCrookMode(SdrCrookMode::Stretch);
            }
        }
        break;

        case SID_CONVERT_TO_3D_LATHE:
        {
            eMode = SdrDragMode::Mirror;
            bSuppressChangesOfSelection = true;

            if ( mpView->GetDragMode() != eMode )
                mpView->SetDragMode(eMode);

            if (!mpView->Is3DRotationCreationActive())
                mpView->Start3DCreation();

            bSuppressChangesOfSelection = false;
        }
        break;

        default:
        {
            eMode = SdrDragMode::Move;

            if ( mpView->GetDragMode() != eMode )
                mpView->SetDragMode(eMode);
        }
        break;
    }

    if (nSlotId != SID_OBJECT_ROTATE)
    {
        bTempRotation = false;
    }

    FuDraw::Activate();
}

void FuSelection::SelectionHasChanged()
{
    bSelectionChanged = true;

    FuDraw::SelectionHasChanged();

    if (mpView->Is3DRotationCreationActive() && !bSuppressChangesOfSelection)
    {
        // Switch rotation body -> selection
        mpView->ResetCreationActive();
        nSlotId = SID_OBJECT_SELECT;
        Activate();
    }

    // Activate the right tool bar for the current context of the view.
    mpViewShell->GetViewShellBase().GetToolBarManager()->SelectionHasChanged(*mpViewShell, *mpView);
}

/**
 * Set current bezier edit mode
 */
void FuSelection::SetEditMode(sal_uInt16 nMode)
{
    nEditMode = nMode;

    if (nEditMode == SID_BEZIER_INSERT)
    {
        mpView->SetInsObjPointMode(true);
    }
    else
    {
        mpView->SetInsObjPointMode(false);
    }

    ForcePointer();

    SfxBindings& rBindings = mpViewShell->GetViewFrame()->GetBindings();
    rBindings.Invalidate(SID_BEZIER_MOVE);
    rBindings.Invalidate(SID_BEZIER_INSERT);
}

/**
 * Execute animation or interaction
 */
bool FuSelection::AnimateObj(SdrObject* pObj, const Point& rPos)
{
    bool bAnimated = false;
    bool bClosed = pObj->IsClosedObj();
    bool bFilled = false;

    if (bClosed)
    {
        SfxItemSet aSet(mpDoc->GetPool());

        aSet.Put(pObj->GetMergedItemSet());

        const XFillStyleItem& rFillStyle = aSet.Get(XATTR_FILLSTYLE);
        bFilled = rFillStyle.GetValue() != drawing::FillStyle_NONE;
    }

    const SdrLayerIDSet* pVisiLayer = &mpView->GetSdrPageView()->GetVisibleLayers();
    sal_uInt16 nHitLog = sal_uInt16 ( mpWindow->PixelToLogic(Size(HITPIX,0)).Width() );
    const long  n2HitLog = nHitLog * 2;
    Point aHitPosR(rPos);
    Point aHitPosL(rPos);
    Point aHitPosT(rPos);
    Point aHitPosB(rPos);

    aHitPosR.AdjustX(n2HitLog );
    aHitPosL.AdjustX( -n2HitLog );
    aHitPosT.AdjustY( n2HitLog );
    aHitPosB.AdjustY( -n2HitLog );

    if ( !bClosed                                      ||
         !bFilled                                      ||
         (SdrObjectPrimitiveHit(*pObj, aHitPosR, nHitLog, *mpView->GetSdrPageView(), pVisiLayer, false) &&
          SdrObjectPrimitiveHit(*pObj, aHitPosL, nHitLog, *mpView->GetSdrPageView(), pVisiLayer, false) &&
          SdrObjectPrimitiveHit(*pObj, aHitPosT, nHitLog, *mpView->GetSdrPageView(), pVisiLayer, false) &&
          SdrObjectPrimitiveHit(*pObj, aHitPosB, nHitLog, *mpView->GetSdrPageView(), pVisiLayer, false) ) )
    {
        if ( SdDrawDocument::GetIMapInfo( pObj ) )
        {
            const IMapObject* pIMapObj = SdDrawDocument::GetHitIMapObject( pObj, rPos );

            if ( pIMapObj && !pIMapObj->GetURL().isEmpty() )
            {
                // Jump to Document
                mpWindow->ReleaseMouse();
                SfxStringItem aStrItem(SID_FILE_NAME, pIMapObj->GetURL());
                SfxStringItem aReferer(SID_REFERER, mpDocSh->GetMedium()->GetName());
                SfxViewFrame* pFrame = mpViewShell->GetViewFrame();
                SfxFrameItem aFrameItem(SID_DOCFRAME, pFrame);
                SfxBoolItem aBrowseItem( SID_BROWSE, true );
                mpWindow->ReleaseMouse();
                pFrame->GetDispatcher()->ExecuteList(SID_OPENDOC,
                        SfxCallMode::ASYNCHRON | SfxCallMode::RECORD,
                        { &aStrItem, &aFrameItem, &aBrowseItem, &aReferer });

                bAnimated = true;
            }
        }
        else if( dynamic_cast< const GraphicDocShell *>( mpDocSh ) ==  nullptr        &&
                 dynamic_cast< const DrawView *>( mpView ) !=  nullptr                 &&
                 SdDrawDocument::GetAnimationInfo(pObj))
        {
            /**********************************************************
            * Animation-object hit in the middle -> interaction
            **********************************************************/
            SdAnimationInfo* pInfo = SdDrawDocument::GetAnimationInfo(pObj);
            DrawViewShell* pDrViewSh = static_cast<DrawViewShell*>(mpViewShell);
            mpWindow->ReleaseMouse();

            switch (pInfo->meClickAction)
            {
                case presentation::ClickAction_BOOKMARK:
                {
                     // Jump to Bookmark (Page or Object)
                    SfxStringItem aItem(SID_NAVIGATOR_OBJECT, pInfo->GetBookmark());
                    mpViewShell->GetViewFrame()->GetDispatcher()->ExecuteList(
                        SID_NAVIGATOR_OBJECT,
                        SfxCallMode::SLOT | SfxCallMode::RECORD, { &aItem });
                    bAnimated = true;
                }
                break;

                case presentation::ClickAction_DOCUMENT:
                {
                    OUString sBookmark( pInfo->GetBookmark() );
                    // Jump to document
                    if (!sBookmark.isEmpty())
                    {
                        SfxStringItem aReferer(SID_REFERER, mpDocSh->GetMedium()->GetName());
                        SfxStringItem aStrItem(SID_FILE_NAME, sBookmark);
                        SfxViewFrame* pFrame = mpViewShell->GetViewFrame();
                        SfxFrameItem aFrameItem(SID_DOCFRAME, pFrame);
                        SfxBoolItem aBrowseItem( SID_BROWSE, true );
                        pFrame->GetDispatcher()->
                        ExecuteList(SID_OPENDOC,
                            SfxCallMode::ASYNCHRON | SfxCallMode::RECORD,
                            { &aStrItem, &aFrameItem, &aBrowseItem, &aReferer });
                    }

                    bAnimated = true;
                }
                break;

                case presentation::ClickAction_PREVPAGE:
                {
                    // Jump to the previous page
                    SfxUInt16Item aItem(SID_NAVIGATOR_PAGE, PAGE_PREVIOUS);
                    mpViewShell->GetViewFrame()->GetDispatcher()->
                    ExecuteList(SID_NAVIGATOR_PAGE,
                            SfxCallMode::SLOT | SfxCallMode::RECORD,
                            { &aItem });
                    bAnimated = true;
                }
                break;

                case presentation::ClickAction_NEXTPAGE:
                {
                    // Jump to the next page
                    SfxUInt16Item aItem(SID_NAVIGATOR_PAGE, PAGE_NEXT);
                    mpViewShell->GetViewFrame()->GetDispatcher()->
                    ExecuteList(SID_NAVIGATOR_PAGE,
                            SfxCallMode::SLOT | SfxCallMode::RECORD,
                            { &aItem });
                    bAnimated = true;
                }
                break;

                case presentation::ClickAction_FIRSTPAGE:
                {
                    // Jump to the first page
                    SfxUInt16Item aItem(SID_NAVIGATOR_PAGE, PAGE_FIRST);
                    mpViewShell->GetViewFrame()->GetDispatcher()->
                    ExecuteList(SID_NAVIGATOR_PAGE,
                            SfxCallMode::SLOT | SfxCallMode::RECORD,
                            { &aItem });
                    bAnimated = true;
                }
                break;

                case presentation::ClickAction_LASTPAGE:
                {
                    // Jump to the last page
                    SfxUInt16Item aItem(SID_NAVIGATOR_PAGE, PAGE_LAST);
                    mpViewShell->GetViewFrame()->GetDispatcher()->
                    ExecuteList(SID_NAVIGATOR_PAGE,
                            SfxCallMode::SLOT | SfxCallMode::RECORD,
                            { &aItem });
                    bAnimated = true;
                }
                break;

                case presentation::ClickAction_SOUND:
                {
#if HAVE_FEATURE_AVMEDIA
                        try
                        {
                            mxPlayer.set( avmedia::MediaWindow::createPlayer( pInfo->GetBookmark(), ""/*TODO?*/), uno::UNO_QUERY_THROW );
                            mxPlayer->start();
                        }
                        catch( uno::Exception& )
                        {
                        }
#endif
                        bAnimated = true;
                }
                break;

                case presentation::ClickAction_VERB:
                {
                    // Assign verb
                    mpView->UnmarkAll();
                    mpView->MarkObj(pObj, mpView->GetSdrPageView());
                    pDrViewSh->DoVerb(static_cast<sal_Int16>(pInfo->mnVerb));
                    bAnimated = true;
                }
                break;

                case presentation::ClickAction_PROGRAM:
                {
                   OUString aBaseURL = GetDocSh()->GetMedium()->GetBaseURL();
                   INetURLObject aURL( ::URIHelper::SmartRel2Abs( INetURLObject(aBaseURL), pInfo->GetBookmark(),
                                                URIHelper::GetMaybeFileHdl(), true, false,
                                                INetURLObject::EncodeMechanism::WasEncoded, INetURLObject::DecodeMechanism::Unambiguous ) );

                   if( INetProtocol::File == aURL.GetProtocol() )
                   {
                        SfxStringItem aUrl( SID_FILE_NAME, aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );
                        SfxBoolItem aBrowsing( SID_BROWSE, true );

                        SfxViewFrame* pViewFrm = SfxViewFrame::Current();
                        if (pViewFrm)
                            pViewFrm->GetDispatcher()->ExecuteList(SID_OPENDOC,
                                  SfxCallMode::ASYNCHRON | SfxCallMode::RECORD,
                                  { &aUrl, &aBrowsing });
                   }

                   bAnimated = true;
                }
                break;

                case presentation::ClickAction_MACRO:
                {
                    // Execute macro
                    OUString aMacro = pInfo->GetBookmark();

                    if ( SfxApplication::IsXScriptURL( aMacro ) )
                    {
                        uno::Any aRet;
                        uno::Sequence< sal_Int16 > aOutArgsIndex;
                        uno::Sequence<uno::Any> aParams;
                        uno::Sequence< uno::Any > aOutArgs;

                        ErrCode eErr = mpDocSh->CallXScript( aMacro,
                            aParams, aRet, aOutArgsIndex, aOutArgs);

                        // Check the return value from the script
                        bool bTmp = false;
                        bAnimated = eErr == ERRCODE_NONE &&
                             aRet.getValueType() == cppu::UnoType<bool>::get() &&
                             ( aRet >>= bTmp ) &&
                             bTmp;
                    }
                    else
                    {
                        // aMacro has got following format:
                        // "Macroname.Modulname.Libname.Documentname" or
                        // "Macroname.Modulname.Libname.Applicationname"
                        sal_Int32 nIdx{ 0 };
                        const OUString aMacroName = aMacro.getToken(0, '.', nIdx);
                        const OUString aModulName = aMacro.getToken(0, '.', nIdx);

                        // In this moment the Call-method only
                        // resolves modulename+macroname
                        OUString aExecMacro(aModulName + "." + aMacroName);
                        bAnimated = mpDocSh->GetBasic()->Call(aExecMacro);
                    }
                }
                break;

                default:
                {
                    bAnimated = false;
                }
                break;
            }
        }

        if (!bAnimated                               &&
            dynamic_cast< const DrawView *>( mpView ) !=  nullptr                 &&
            dynamic_cast< const GraphicDocShell *>( mpDocSh ) ==  nullptr        &&
            SlideShow::IsRunning( mpViewShell->GetViewShellBase() ) &&
            SdDrawDocument::GetAnimationInfo(pObj))
        {
            /**********************************************************
            * Effect-Object hit in the middle -> Play effect
            **********************************************************/
            SdAnimationInfo* pInfo = SdDrawDocument::GetAnimationInfo(pObj);

            switch (pInfo->meClickAction)
            {
                case presentation::ClickAction_VANISH:
                case presentation::ClickAction_INVISIBLE:
                    break;

                default:
                    bAnimated = false;
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
        mpViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SfxCallMode::ASYNCHRON | SfxCallMode::RECORD);
        return true;
    }
    else
    {
        return false;
    }
}

SdrObject* FuSelection::pickObject (const Point& rTestPoint)
{
    SdrPageView* pPageView;
    sal_uInt16 nHitLog = sal_uInt16 (mpWindow->PixelToLogic(Size(HITPIX,0)).Width());
    return mpView->PickObj(rTestPoint, nHitLog, pPageView, SdrSearchOptions::PICKMARKABLE);
}

void FuSelection::ForcePointer(const MouseEvent* pMEvt)
{
    if(bMovedToCenterPoint && !bBeginInsertPoint && pMEvt)
    {
        MouseEvent aMEvt(pMEvt->GetPosPixel(), pMEvt->GetClicks(),
            pMEvt->GetMode(), pMEvt->GetButtons(), pMEvt->GetModifier() & ~KEY_SHIFT);
        FuDraw::ForcePointer(&aMEvt);
    }
    else
    {
        FuDraw::ForcePointer(pMEvt);
    }
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
