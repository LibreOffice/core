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


#include <sot/object.hxx>
#include <editeng/eeitem.hxx>
#include <vcl/waitobj.hxx>

#include <editeng/flditem.hxx>
#include <svx/svdogrp.hxx>
#include <tools/urlobj.hxx>
#include <vcl/help.hxx>
#include <svx/bmpmask.hxx>
#include <svx/svdotext.hxx>
#include <sfx2/app.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/bindings.hxx>
#include <svx/svdpagv.hxx>
#include <svtools/imapobj.hxx>
#include <svx/svxids.hrc>
#include <svx/obj3d.hxx>
#include <svx/polysc3d.hxx>

#include <sfx2/viewfrm.hxx>

#include "anminfo.hxx"
#include "imapinfo.hxx"
#include "app.hrc"
#include "glob.hrc"
#include "strings.hrc"
#include "res_bmp.hrc"

#include "sdmod.hxx"
#include "GraphicDocShell.hxx"
#include "fudraw.hxx"
#include "ViewShell.hxx"
#include "FrameView.hxx"
#include "View.hxx"
#include "Window.hxx"
#include "drawdoc.hxx"
#include "DrawDocShell.hxx"
#include "Client.hxx"
#include "sdresid.hxx"
#include "drawview.hxx"
#include "fusel.hxx"
#include <svl/aeitem.hxx>
#include <vcl/msgbox.hxx>
#include "slideshow.hxx"
#include <svx/sdrhittesthelper.hxx>

using namespace ::com::sun::star;

namespace sd {

TYPEINIT1( FuDraw, FuPoor );

/**
 * Base-class for all drawmodul-specific functions
 */
FuDraw::FuDraw(ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView,
               SdDrawDocument* pDoc, SfxRequest& rReq) :
    FuPoor(pViewSh, pWin, pView, pDoc, rReq),
    bMBDown(sal_False),
    bDragHelpLine(sal_False),
    bPermanent(sal_False)
{
}


FuDraw::~FuDraw()
{
    mpView->BrkAction();
}


sal_Bool FuDraw::MouseButtonDown(const MouseEvent& rMEvt)
{
    // remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    sal_Bool bReturn = sal_False;

    bDragHelpLine = sal_False;

    aMDPos = mpWindow->PixelToLogic( rMEvt.GetPosPixel() );

    if ( rMEvt.IsLeft() )
    {
        FrameView* pFrameView = mpViewShell->GetFrameView();

        bool bOrtho = sal_False;

        sal_Bool bRestricted = sal_True;

        if (mpView->IsDragObj())
        {
            // object is dragged (move, resize,...)
            const SdrHdl* pHdl = mpView->GetDragStat().GetHdl();

            if (!pHdl || (!pHdl->IsCornerHdl() && !pHdl->IsVertexHdl()))
            {
                // Move
                bRestricted = sal_False;
            }
        }

        // #i33136#
        if(bRestricted && doConstructOrthogonal())
        {
            // Restrict movement:
            // rectangle->quadrat, ellipse->circle etc.
            bOrtho = !rMEvt.IsShift();
        }
        else
        {
            bOrtho = rMEvt.IsShift() != pFrameView->IsOrtho();
        }

        if (!mpView->IsSnapEnabled())
            mpView->SetSnapEnabled(sal_True);
        sal_Bool bSnapModPressed = rMEvt.IsMod1();

        sal_Bool bGridSnap = pFrameView->IsGridSnap();
        bGridSnap = (bSnapModPressed != bGridSnap);

        if (mpView->IsGridSnap() != bGridSnap)
            mpView->SetGridSnap(bGridSnap);

        sal_Bool bBordSnap = pFrameView->IsBordSnap();
        bBordSnap = (bSnapModPressed != bBordSnap);

        if (mpView->IsBordSnap() != bBordSnap)
            mpView->SetBordSnap(bBordSnap);

        sal_Bool bHlplSnap = pFrameView->IsHlplSnap();
        bHlplSnap = (bSnapModPressed != bHlplSnap);

        if (mpView->IsHlplSnap() != bHlplSnap)
            mpView->SetHlplSnap(bHlplSnap);

        sal_Bool bOFrmSnap = pFrameView->IsOFrmSnap();
        bOFrmSnap = (bSnapModPressed != bOFrmSnap);

        if (mpView->IsOFrmSnap() != bOFrmSnap)
            mpView->SetOFrmSnap(bOFrmSnap);

        sal_Bool bOPntSnap = pFrameView->IsOPntSnap();
        bOPntSnap = (bSnapModPressed != bOPntSnap);

        if (mpView->IsOPntSnap() != bOPntSnap)
            mpView->SetOPntSnap(bOPntSnap);

        sal_Bool bOConSnap = pFrameView->IsOConSnap();
        bOConSnap = (bSnapModPressed != bOConSnap);

        if (mpView->IsOConSnap() != bOConSnap)
            mpView->SetOConSnap(bOConSnap);

        sal_Bool bAngleSnap = rMEvt.IsShift() == !pFrameView->IsAngleSnapEnabled();

        if (mpView->IsAngleSnapEnabled() != bAngleSnap)
            mpView->SetAngleSnapEnabled(bAngleSnap);

        if (mpView->IsOrtho() != bOrtho)
            mpView->SetOrtho(bOrtho);

        sal_Bool bCenter = rMEvt.IsMod2();

        if ( mpView->IsCreate1stPointAsCenter() != bCenter ||
             mpView->IsResizeAtCenter() != bCenter )
        {
            mpView->SetCreate1stPointAsCenter(bCenter);
            mpView->SetResizeAtCenter(bCenter);
        }

        SdrPageView* pPV = 0;
        sal_uInt16 nHitLog = sal_uInt16 ( mpWindow->PixelToLogic(Size(HITPIX,0)).Width() );

        // look only for HelpLines when they are visible (!)
        sal_Bool bHelpLine(sal_False);
        if(mpView->IsHlplVisible())
            bHelpLine = mpView->PickHelpLine(aMDPos, nHitLog, *mpWindow, nHelpLine, pPV);
        sal_Bool bHitHdl = (mpView->PickHandle(aMDPos) != NULL);

        if ( bHelpLine
            && !mpView->IsCreateObj()
            && ((mpView->GetEditMode() == SDREDITMODE_EDIT && !bHitHdl) || (rMEvt.IsShift() && bSnapModPressed)) )
        {
            mpWindow->CaptureMouse();
            mpView->BegDragHelpLine(nHelpLine, pPV);
            bDragHelpLine = mpView->IsDragHelpLine();
            bReturn = sal_True;
        }
    }
    ForcePointer(&rMEvt);

    return bReturn;
}


sal_Bool FuDraw::MouseMove(const MouseEvent& rMEvt)
{
    FrameView* pFrameView = mpViewShell->GetFrameView();
    Point aPos = mpWindow->PixelToLogic( rMEvt.GetPosPixel() );

    bool bOrtho = sal_False;

    sal_Bool bRestricted = sal_True;

    if (mpView->IsDragObj())
    {
        // object is dragged (move, resize, ...)
        const SdrHdl* pHdl = mpView->GetDragStat().GetHdl();

        if (!pHdl || (!pHdl->IsCornerHdl() && !pHdl->IsVertexHdl()))
        {
            // Move
            bRestricted = sal_False;
        }
    }

    if (mpView->IsAction())
    {
        // #i33136#
        if(bRestricted && doConstructOrthogonal())
        {
            // Restrict movement:
            // rectangle->quadrat, ellipse->circle etc.
            bOrtho = !rMEvt.IsShift();
        }
        else
        {
            bOrtho = rMEvt.IsShift() != pFrameView->IsOrtho();
        }

        sal_Bool bSnapModPressed = rMEvt.IsMod2();
        mpView->SetDragWithCopy(rMEvt.IsMod1() && pFrameView->IsDragWithCopy());

        sal_Bool bGridSnap = pFrameView->IsGridSnap();
        bGridSnap = (bSnapModPressed != bGridSnap);

        if (mpView->IsGridSnap() != bGridSnap)
            mpView->SetGridSnap(bGridSnap);

        sal_Bool bBordSnap = pFrameView->IsBordSnap();
        bBordSnap = (bSnapModPressed != bBordSnap);

        if (mpView->IsBordSnap() != bBordSnap)
            mpView->SetBordSnap(bBordSnap);

        sal_Bool bHlplSnap = pFrameView->IsHlplSnap();
        bHlplSnap = (bSnapModPressed != bHlplSnap);

        if (mpView->IsHlplSnap() != bHlplSnap)
            mpView->SetHlplSnap(bHlplSnap);

        sal_Bool bOFrmSnap = pFrameView->IsOFrmSnap();
        bOFrmSnap = (bSnapModPressed != bOFrmSnap);

        if (mpView->IsOFrmSnap() != bOFrmSnap)
            mpView->SetOFrmSnap(bOFrmSnap);

        sal_Bool bOPntSnap = pFrameView->IsOPntSnap();
        bOPntSnap = (bSnapModPressed != bOPntSnap);

        if (mpView->IsOPntSnap() != bOPntSnap)
            mpView->SetOPntSnap(bOPntSnap);

        sal_Bool bOConSnap = pFrameView->IsOConSnap();
        bOConSnap = (bSnapModPressed != bOConSnap);

        if (mpView->IsOConSnap() != bOConSnap)
            mpView->SetOConSnap(bOConSnap);

        sal_Bool bAngleSnap = rMEvt.IsShift() == !pFrameView->IsAngleSnapEnabled();

        if (mpView->IsAngleSnapEnabled() != bAngleSnap)
            mpView->SetAngleSnapEnabled(bAngleSnap);

        if (mpView->IsOrtho() != bOrtho)
            mpView->SetOrtho(bOrtho);

        sal_Bool bCenter = rMEvt.IsMod2();

        if ( mpView->IsCreate1stPointAsCenter() != bCenter ||
             mpView->IsResizeAtCenter() != bCenter )
        {
            mpView->SetCreate1stPointAsCenter(bCenter);
            mpView->SetResizeAtCenter(bCenter);
        }

        if ( mpView->IsDragHelpLine() )
            mpView->MovDragHelpLine(aPos);
    }

    sal_Bool bReturn = mpView->MouseMove(rMEvt, mpWindow);

    if (mpView->IsAction())
    {
        // Because the flag set back if necessary in MouseMove
        if (mpView->IsOrtho() != bOrtho)
            mpView->SetOrtho(bOrtho);
    }

    ForcePointer(&rMEvt);

    return bReturn;
}


sal_Bool FuDraw::MouseButtonUp(const MouseEvent& rMEvt)
{
    if ( mpView->IsDragHelpLine() )
        mpView->EndDragHelpLine();

    if ( bDragHelpLine )
    {
        Rectangle aOutputArea(Point(0,0), mpWindow->GetOutputSizePixel());

        if ( !aOutputArea.IsInside(rMEvt.GetPosPixel()) )
            mpView->GetSdrPageView()->DeleteHelpLine(nHelpLine);

        mpWindow->ReleaseMouse();
    }

    FrameView* pFrameView = mpViewShell->GetFrameView();
    mpView->SetOrtho( pFrameView->IsOrtho() );
    mpView->SetAngleSnapEnabled( pFrameView->IsAngleSnapEnabled() );
    mpView->SetSnapEnabled(sal_True);
    mpView->SetCreate1stPointAsCenter(sal_False);
    mpView->SetResizeAtCenter(sal_False);
    mpView->SetDragWithCopy(pFrameView->IsDragWithCopy());
    mpView->SetGridSnap(pFrameView->IsGridSnap());
    mpView->SetBordSnap(pFrameView->IsBordSnap());
    mpView->SetHlplSnap(pFrameView->IsHlplSnap());
    mpView->SetOFrmSnap(pFrameView->IsOFrmSnap());
    mpView->SetOPntSnap(pFrameView->IsOPntSnap());
    mpView->SetOConSnap(pFrameView->IsOConSnap());

    bIsInDragMode = sal_False;
    ForcePointer(&rMEvt);
    FuPoor::MouseButtonUp(rMEvt);

    return sal_False;
}

/**
 * Process keyboard input
 * @returns sal_True if a KeyEvent is being processed, sal_False otherwise
 */
sal_Bool FuDraw::KeyInput(const KeyEvent& rKEvt)
{
    sal_Bool bReturn = sal_False;

    switch ( rKEvt.GetKeyCode().GetCode() )
    {
        case KEY_ESCAPE:
        {
            bReturn = FuDraw::cancel();
        }
        break;

        case KEY_DELETE:
        case KEY_BACKSPACE:
        {
            if (!mpDocSh->IsReadOnly())
            {
                if ( mpView && mpView->IsPresObjSelected(sal_False, sal_True, sal_False, sal_True) )
                {
                    InfoBox(mpWindow, SD_RESSTR(STR_ACTION_NOTPOSSIBLE) ).Execute();
                }
                else
                {
                    /* If IP-Client active, we reset the pointer to the OLE- and
                       to the old graphic object of SdClient. With this, we
                       avoid the restoration of an no more existing object in
                       ::SelectionHasChanged after deletion. All other OLE
                       objects are not affected. */
                    OSL_ASSERT (mpViewShell->GetViewShell()!=NULL);
                    Client* pIPClient = static_cast<Client*>(
                        mpViewShell->GetViewShell()->GetIPClient());
                    if (pIPClient && pIPClient->IsObjectInPlaceActive())
                        pIPClient->SetSdrGrafObj(NULL);

                    // wait-mousepointer while deleting object
                    WaitObject aWait( (Window*)mpViewShell->GetActiveWindow() );
                    // delete object
                    mpView->DeleteMarked();
                }
            }
            bReturn = sal_True;
        }
        break;

        case KEY_TAB:
        {
            KeyCode aCode = rKEvt.GetKeyCode();

            if ( !aCode.IsMod1() && !aCode.IsMod2() )
            {
                // Moved next line which was a bugfix itself into
                // the scope which really does the object selection travel
                // and thus is allowed to call SelectionHasChanged().

                // Switch to FuSelect.
                mpViewShell->GetViewFrame()->GetDispatcher()->Execute(
                    SID_OBJECT_SELECT,
                    SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);

                // changeover to the next object
                if(!mpView->MarkNextObj( !aCode.IsShift() ))
                {
                    // No next object: go over open end and
                    // get first from the other side
                    mpView->UnmarkAllObj();
                    mpView->MarkNextObj(!aCode.IsShift());
                }

                if(mpView->AreObjectsMarked())
                    mpView->MakeVisible(mpView->GetAllMarkedRect(), *mpWindow);

                bReturn = sal_True;
            }
        }
        break;

        case KEY_END:
        {
            KeyCode aCode = rKEvt.GetKeyCode();

            if ( aCode.IsMod1() )
            {
                // mark last object
                mpView->UnmarkAllObj();
                mpView->MarkNextObj(sal_False);

                if(mpView->AreObjectsMarked())
                    mpView->MakeVisible(mpView->GetAllMarkedRect(), *mpWindow);

                bReturn = sal_True;
            }
        }
        break;

        case KEY_HOME:
        {
            KeyCode aCode = rKEvt.GetKeyCode();

            if ( aCode.IsMod1() )
            {
                // mark first object
                mpView->UnmarkAllObj();
                mpView->MarkNextObj(sal_True);

                if(mpView->AreObjectsMarked())
                    mpView->MakeVisible(mpView->GetAllMarkedRect(), *mpWindow);

                bReturn = sal_True;
            }
        }
        break;

        default:
        break;
    }

    if (!bReturn)
    {
        bReturn = FuPoor::KeyInput(rKEvt);
    }
    else
    {
        mpWindow->ReleaseMouse();
    }

    return (bReturn);
}


void FuDraw::Activate()
{
    FuPoor::Activate();
    ForcePointer();
}


void FuDraw::Deactivate()
{
    FuPoor::Deactivate();
}


/**
 * Toggle mouse-pointer
 */
void FuDraw::ForcePointer(const MouseEvent* pMEvt)
{
    Point aPnt;
    sal_uInt16 nModifier = 0;
    sal_Bool bLeftDown = sal_False;
    sal_Bool bDefPointer = sal_True;

    if (pMEvt)
    {
        aPnt = mpWindow->PixelToLogic(pMEvt->GetPosPixel());
        nModifier = pMEvt->GetModifier();
        bLeftDown = pMEvt->IsLeft();
    }
    else
    {
        aPnt = mpWindow->PixelToLogic(mpWindow->GetPointerPosPixel());
    }

    if (mpView->IsDragObj())
    {
        if (SD_MOD()->GetWaterCan() && !mpView->PickHandle(aPnt))
        {
            // water can mode
            bDefPointer = sal_False;
            mpWindow->SetPointer(Pointer(POINTER_FILL));
        }
    }
    else
    {
        SdrHdl* pHdl = mpView->PickHandle(aPnt);

        if (SD_MOD()->GetWaterCan() && !pHdl)
        {
            // water can mode
            bDefPointer = sal_False;
            mpWindow->SetPointer(Pointer(POINTER_FILL));
        }
        else if (!pHdl &&
                 mpViewShell->GetViewFrame()->HasChildWindow(SvxBmpMaskChildWindow::GetChildWindowId()))
        {
            // pipette mode
            SvxBmpMask* pMask = (SvxBmpMask*) mpViewShell->GetViewFrame()->GetChildWindow(SvxBmpMaskChildWindow::GetChildWindowId())->GetWindow();

            if (pMask && pMask->IsEyedropping())
            {
                bDefPointer = sal_False;
                mpWindow->SetPointer(Pointer(POINTER_REFHAND));
            }
        }
        else if (!mpView->IsAction())
        {
            SdrObject* pObj = NULL;
            SdrPageView* pPV = NULL;
            SdrViewEvent aVEvt;
            SdrHitKind eHit = SDRHIT_NONE;
            SdrDragMode eDragMode = mpView->GetDragMode();

            if (pMEvt)
            {
                eHit = mpView->PickAnything(*pMEvt, SDRMOUSEMOVE, aVEvt);
            }

            if ((eDragMode == SDRDRAG_ROTATE) && (eHit == SDRHIT_MARKEDOBJECT))
            {
                // The goal of this request is show always the rotation-arrow for 3D-objects at rotation-modus
                // Independent of the settings at Extras->Optionen->Grafik "Objekte immer verschieben"
                // 2D-objects acquit in an other way. Otherwise, the rotation of 3d-objects around any axises
                // wouldn't be possible per default.
                const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();
                SdrObject* pObject = rMarkList.GetMark(0)->GetMarkedSdrObj();
                if ((pObject->ISA(E3dObject)) && (rMarkList.GetMarkCount() == 1))
                {
                    mpWindow->SetPointer(Pointer(POINTER_ROTATE));
                    bDefPointer = sal_False;     // Otherwise it'll be calles Joes routine and the mousepointer will reconfigurate again
                }
            }

            if (eHit == SDRHIT_NONE)
            {
                // found nothing -> look after at the masterpage
                mpView->PickObj(aPnt, mpView->getHitTolLog(), pObj, pPV, SDRSEARCH_ALSOONMASTER);
            }
            else if (eHit == SDRHIT_UNMARKEDOBJECT)
            {
                pObj = aVEvt.pObj;
            }
            else if (eHit == SDRHIT_TEXTEDITOBJ && this->ISA(FuSelection))
            {
                sal_uInt16 nSdrObjKind = aVEvt.pObj->GetObjIdentifier();

                if ( nSdrObjKind != OBJ_TEXT        &&
                     nSdrObjKind != OBJ_TITLETEXT   &&
                     nSdrObjKind != OBJ_OUTLINETEXT &&
                     aVEvt.pObj->IsEmptyPresObj() )
                {
                    pObj = NULL;
                    bDefPointer = sal_False;
                    mpWindow->SetPointer(Pointer(POINTER_ARROW));
                }
            }

            if (pObj && pMEvt && !pMEvt->IsMod2() && this->ISA(FuSelection))
            {
                // test for animation or ImageMap
                bDefPointer = !SetPointer(pObj, aPnt);

                if (bDefPointer && (pObj->ISA(SdrObjGroup) || pObj->ISA(E3dPolyScene)))
                {
                    // take a glance into the group
                    if (mpView->PickObj(aPnt, mpView->getHitTolLog(), pObj, pPV, SDRSEARCH_ALSOONMASTER | SDRSEARCH_DEEP))
                        bDefPointer = !SetPointer(pObj, aPnt);
                }
            }
        }
    }

    if (bDefPointer)
    {
        mpWindow->SetPointer(mpView->GetPreferedPointer(
                            aPnt, mpWindow, nModifier, bLeftDown));
    }
}

/**
 * Set cursor for animaton or imagemap
 */
sal_Bool FuDraw::SetPointer(SdrObject* pObj, const Point& rPos)
{
    sal_Bool bSet = sal_False;

    sal_Bool bAnimationInfo = (!mpDocSh->ISA(GraphicDocShell) &&
                          mpDoc->GetAnimationInfo(pObj)) ? sal_True:sal_False;

    sal_Bool bImageMapInfo = sal_False;

    if (!bAnimationInfo)
        bImageMapInfo = mpDoc->GetIMapInfo(pObj) ? sal_True:sal_False;

    if (bAnimationInfo || bImageMapInfo)
    {
        const SetOfByte* pVisiLayer = &mpView->GetSdrPageView()->GetVisibleLayers();
        sal_uInt16 nHitLog(sal_uInt16 (mpWindow->PixelToLogic(Size(HITPIX,0)).Width()));
        long  n2HitLog(nHitLog * 2);
        Point aHitPosR(rPos);
        Point aHitPosL(rPos);
        Point aHitPosT(rPos);
        Point aHitPosB(rPos);

        aHitPosR.X() += n2HitLog;
        aHitPosL.X() -= n2HitLog;
        aHitPosT.Y() += n2HitLog;
        aHitPosB.Y() -= n2HitLog;

        if ( !pObj->IsClosedObj() ||
            ( SdrObjectPrimitiveHit(*pObj, aHitPosR, nHitLog, *mpView->GetSdrPageView(), pVisiLayer, false) &&
              SdrObjectPrimitiveHit(*pObj, aHitPosL, nHitLog, *mpView->GetSdrPageView(), pVisiLayer, false) &&
              SdrObjectPrimitiveHit(*pObj, aHitPosT, nHitLog, *mpView->GetSdrPageView(), pVisiLayer, false) &&
              SdrObjectPrimitiveHit(*pObj, aHitPosB, nHitLog, *mpView->GetSdrPageView(), pVisiLayer, false)))
        {
            /**********************************************************
            * hit inside the object (without margin) or open object
            ********************************************************/

            if (bAnimationInfo)
            {
                /******************************************************
                * Click-Action
                ******************************************************/
                SdAnimationInfo* pInfo = mpDoc->GetAnimationInfo(pObj);

                if ((mpView->ISA(DrawView) &&
                      (pInfo->meClickAction == presentation::ClickAction_BOOKMARK  ||
                       pInfo->meClickAction == presentation::ClickAction_DOCUMENT  ||
                       pInfo->meClickAction == presentation::ClickAction_PREVPAGE  ||
                       pInfo->meClickAction == presentation::ClickAction_NEXTPAGE  ||
                       pInfo->meClickAction == presentation::ClickAction_FIRSTPAGE ||
                       pInfo->meClickAction == presentation::ClickAction_LASTPAGE  ||
                       pInfo->meClickAction == presentation::ClickAction_VERB      ||
                       pInfo->meClickAction == presentation::ClickAction_PROGRAM   ||
                       pInfo->meClickAction == presentation::ClickAction_MACRO     ||
                       pInfo->meClickAction == presentation::ClickAction_SOUND))
                                                                    ||
                    (mpView->ISA(DrawView) &&
                        SlideShow::IsRunning( mpViewShell->GetViewShellBase() )   &&
                         (pInfo->meClickAction == presentation::ClickAction_VANISH            ||
                          pInfo->meClickAction == presentation::ClickAction_INVISIBLE         ||
                          pInfo->meClickAction == presentation::ClickAction_STOPPRESENTATION ||
                         (pInfo->mbActive &&
                          ( pInfo->meEffect != presentation::AnimationEffect_NONE ||
                            pInfo->meTextEffect != presentation::AnimationEffect_NONE )))))
                    {
                        // Animation object
                        bSet = sal_True;
                        mpWindow->SetPointer(Pointer(POINTER_REFHAND));
                    }
            }
            else if (bImageMapInfo &&
                     mpDoc->GetHitIMapObject(pObj, rPos, *mpWindow))
            {
                /******************************************************
                * ImageMap
                ******************************************************/
                bSet = sal_True;
                mpWindow->SetPointer(Pointer(POINTER_REFHAND));
            }
        }
    }

    return bSet;
}



/**
 * Response of doubleclick
 */
void FuDraw::DoubleClick(const MouseEvent& rMEvt)
{
    sal_uInt16 nHitLog = sal_uInt16 ( mpWindow->PixelToLogic(Size(HITPIX,0)).Width() );

    if ( mpView->AreObjectsMarked() )
    {
        const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();

        if (rMarkList.GetMarkCount() == 1)
        {
            SdrMark* pMark = rMarkList.GetMark(0);
            SdrObject* pObj = pMark->GetMarkedSdrObj();

            sal_uInt32 nInv = pObj->GetObjInventor();
            sal_uInt16 nSdrObjKind = pObj->GetObjIdentifier();

            if (nInv == SdrInventor && nSdrObjKind == OBJ_OLE2)
            {
                DrawDocShell* pDocSh = mpDoc->GetDocSh();

                if ( !pDocSh->IsUIActive() )
                {
                    /**********************************************************
                    * activate OLE-object
                    **********************************************************/
                    mpViewShell->ActivateObject( (SdrOle2Obj*) pObj, 0);
                }
            }
            else if (nInv == SdrInventor &&  nSdrObjKind == OBJ_GRAF && pObj->IsEmptyPresObj() )
            {
                mpViewShell->GetViewFrame()->
                    GetDispatcher()->Execute( SID_INSERT_GRAPHIC,
                                              SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD );
            }
            else if ( ( pObj->ISA(SdrTextObj) || pObj->ISA(SdrObjGroup) ) &&
                      !SD_MOD()->GetWaterCan()                            &&
                      mpViewShell->GetFrameView()->IsDoubleClickTextEdit() &&
                      !mpDocSh->IsReadOnly())
            {
                SfxUInt16Item aItem(SID_TEXTEDIT, 2);
                mpViewShell->GetViewFrame()->GetDispatcher()->
                                 Execute(SID_TEXTEDIT, SFX_CALLMODE_ASYNCHRON |
                                         SFX_CALLMODE_RECORD, &aItem, 0L);
            }
            else if (nInv == SdrInventor &&  nSdrObjKind == OBJ_GRUP)
            {
                // hit group -> select subobject
                mpView->UnMarkAll();
                mpView->MarkObj(aMDPos, nHitLog, rMEvt.IsShift(), sal_True);
            }
        }
    }
    else
        mpViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
}


sal_Bool FuDraw::RequestHelp(const HelpEvent& rHEvt)
{
    sal_Bool bReturn = sal_False;

    if (Help::IsBalloonHelpEnabled() || Help::IsQuickHelpEnabled())
    {
        SdrViewEvent aVEvt;

        MouseEvent aMEvt(mpWindow->GetPointerPosPixel(), 1, 0, MOUSE_LEFT);

        SdrHitKind eHit = mpView->PickAnything(aMEvt, SDRMOUSEBUTTONDOWN, aVEvt);

        SdrObject* pObj = aVEvt.pObj;

        if (eHit != SDRHIT_NONE && pObj != NULL)
        {
            Point aPosPixel = rHEvt.GetMousePosPixel();

            bReturn = SetHelpText(pObj, aPosPixel, aVEvt);

            if (!bReturn && (pObj->ISA(SdrObjGroup) || pObj->ISA(E3dPolyScene)))
            {
                // take a glance into the group
                SdrPageView* pPV = NULL;

                Point aPos(mpWindow->PixelToLogic(mpWindow->ScreenToOutputPixel(aPosPixel)));

                if (mpView->PickObj(aPos, mpView->getHitTolLog(), pObj, pPV, SDRSEARCH_ALSOONMASTER | SDRSEARCH_DEEP))
                    bReturn = SetHelpText(pObj, aPosPixel, aVEvt);
            }
        }
    }

    if (!bReturn)
    {
        bReturn = FuPoor::RequestHelp(rHEvt);
    }

    return(bReturn);
}



sal_Bool FuDraw::SetHelpText(SdrObject* pObj, const Point& rPosPixel, const SdrViewEvent& rVEvt)
{
    sal_Bool bSet = sal_False;
    String aHelpText;
    Point aPos(mpWindow->PixelToLogic(mpWindow->ScreenToOutputPixel(rPosPixel)));

    // URL for IMapObject underneath pointer is help text
    if ( mpDoc->GetIMapInfo(pObj) )
    {
        IMapObject* pIMapObj = mpDoc->GetHitIMapObject(pObj, aPos, *mpWindow );

        if ( pIMapObj )
        {
            // show name
            aHelpText = pIMapObj->GetAltText();

            if (aHelpText.Len() == 0)
            {
                // show url if no name is available
                aHelpText = INetURLObject::decode( pIMapObj->GetURL(), '%', INetURLObject::DECODE_WITH_CHARSET );
            }
        }
    }
    else if (!mpDocSh->ISA(GraphicDocShell) && mpDoc->GetAnimationInfo(pObj))
    {
        SdAnimationInfo* pInfo = mpDoc->GetAnimationInfo(pObj);

        switch (pInfo->meClickAction)
        {
            case presentation::ClickAction_PREVPAGE:
            {
                // jump to the prior page
                aHelpText = SD_RESSTR(STR_CLICK_ACTION_PREVPAGE);
            }
            break;

            case presentation::ClickAction_NEXTPAGE:
            {
                // jump to the next page
                aHelpText = SD_RESSTR(STR_CLICK_ACTION_NEXTPAGE);
            }
            break;

            case presentation::ClickAction_FIRSTPAGE:
            {
                // jump to the first page
                aHelpText = SD_RESSTR(STR_CLICK_ACTION_FIRSTPAGE);
            }
            break;

            case presentation::ClickAction_LASTPAGE:
            {
                // jump to the last page
                aHelpText = SD_RESSTR(STR_CLICK_ACTION_LASTPAGE);
            }
            break;

            case presentation::ClickAction_BOOKMARK:
            {
                // jump to object/page
                aHelpText = SD_RESSTR(STR_CLICK_ACTION_BOOKMARK);
                aHelpText.AppendAscii( ": " );
                aHelpText.Append( String(INetURLObject::decode( pInfo->GetBookmark(), '%', INetURLObject::DECODE_WITH_CHARSET ) ));
            }
            break;

            case presentation::ClickAction_DOCUMENT:
            {
                // jump to document (object/page)
                aHelpText = SD_RESSTR(STR_CLICK_ACTION_DOCUMENT);
                aHelpText.AppendAscii( ": " );
                aHelpText.Append( String(INetURLObject::decode( pInfo->GetBookmark(), '%', INetURLObject::DECODE_WITH_CHARSET ) ));
            }
            break;

            case presentation::ClickAction_PROGRAM:
            {
                // execute program
                aHelpText = SD_RESSTR(STR_CLICK_ACTION_PROGRAM);
                aHelpText.AppendAscii( ": " );
                aHelpText.Append( String(INetURLObject::decode( pInfo->GetBookmark(), '%', INetURLObject::DECODE_WITH_CHARSET ) ));
            }
            break;

            case presentation::ClickAction_MACRO:
            {
                // execute program
                aHelpText = SD_RESSTR(STR_CLICK_ACTION_MACRO);
                aHelpText.AppendAscii( ": " );

                if ( SfxApplication::IsXScriptURL( pInfo->GetBookmark() ) )
                {
                    aHelpText.Append( pInfo->GetBookmark() );
                }
                else
                {
                    String sBookmark( pInfo->GetBookmark() );
                    sal_Unicode cToken = '.';
                    aHelpText.Append( sBookmark.GetToken( 2, cToken ) );
                    aHelpText.Append( cToken );
                    aHelpText.Append( sBookmark.GetToken( 1, cToken ) );
                    aHelpText.Append( cToken );
                    aHelpText.Append( sBookmark.GetToken( 0, cToken ) );
                }
            }
            break;

            case presentation::ClickAction_SOUND:
            {
                // play-back sound
                aHelpText = SD_RESSTR(STR_CLICK_ACTION_SOUND);
            }
            break;

            case presentation::ClickAction_VERB:
            {
                // execute OLE-verb
                aHelpText = SD_RESSTR(STR_CLICK_ACTION_VERB);
            }
            break;

            case presentation::ClickAction_STOPPRESENTATION:
            {
                // quit presentation
                aHelpText = SD_RESSTR(STR_CLICK_ACTION_STOPPRESENTATION);
            }
            break;
            default:
                break;
        }
    }
    else if (rVEvt.pURLField)
    {
        /**************************************************************
        * URL-Field
        **************************************************************/
        aHelpText = INetURLObject::decode( rVEvt.pURLField->GetURL(), '%', INetURLObject::DECODE_WITH_CHARSET );
    }

    if (aHelpText.Len())
    {
        bSet = sal_True;
        Rectangle aLogicPix = mpWindow->LogicToPixel(pObj->GetLogicRect());
        Rectangle aScreenRect(mpWindow->OutputToScreenPixel(aLogicPix.TopLeft()),
                              mpWindow->OutputToScreenPixel(aLogicPix.BottomRight()));

        if (Help::IsBalloonHelpEnabled())
            Help::ShowBalloon( (Window*)mpWindow, rPosPixel, aScreenRect, aHelpText);
        else if (Help::IsQuickHelpEnabled())
            Help::ShowQuickHelp( (Window*)mpWindow, aScreenRect, aHelpText);
    }

    return bSet;
}


/** is called when the currenct function should be aborted. <p>
    This is used when a function gets a KEY_ESCAPE but can also
    be called directly.

    @returns true if a active function was aborted
*/
bool FuDraw::cancel()
{
    bool bReturn = false;

    if ( mpView->IsAction() )
    {
        mpView->BrkAction();
        bReturn = true;
    }
    else if ( mpView->IsTextEdit() )
    {
        mpView->SdrEndTextEdit();
        bReturn = true;

        SfxBindings& rBindings = mpViewShell->GetViewFrame()->GetBindings();
        rBindings.Invalidate( SID_PARASPACE_INCREASE );
        rBindings.Invalidate( SID_PARASPACE_DECREASE );
    }
    else if ( mpView->AreObjectsMarked() )
    {
        const SdrHdlList& rHdlList = mpView->GetHdlList();
        SdrHdl* pHdl = rHdlList.GetFocusHdl();

        if(pHdl)
        {
            ((SdrHdlList&)rHdlList).ResetFocusHdl();
        }
        else
        {
            mpView->UnmarkAll();
        }

        // Switch to FuSelect.
        mpViewShell->GetViewFrame()->GetDispatcher()->Execute(
            SID_OBJECT_SELECT,
            SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);

        bReturn = true;
    }

    return bReturn;
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
