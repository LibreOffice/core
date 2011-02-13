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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

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
#include <svx/svdpagv.hxx>

#include <sfx2/viewfrm.hxx>

#include "anminfo.hxx"
#include "anmdef.hxx"
#include "imapinfo.hxx"
#include "app.hrc"
#include "glob.hrc"
#include "strings.hrc"
#include "res_bmp.hrc"

#include "app.hxx"
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

/*************************************************************************
|*
|* Base-class for all drawmodul-specific functions
|*
\************************************************************************/

FuDraw::FuDraw(ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView,
               SdDrawDocument* pDoc, SfxRequest& rReq) :
    FuPoor(pViewSh, pWin, pView, pDoc, rReq),
    bMBDown(FALSE),
    bDragHelpLine(FALSE),
    bPermanent(FALSE)
{
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

FuDraw::~FuDraw()
{
    mpView->BrkAction();
}

/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/

BOOL FuDraw::MouseButtonDown(const MouseEvent& rMEvt)
{
    // remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    BOOL bReturn = FALSE;

    bDragHelpLine = FALSE;

    aMDPos = mpWindow->PixelToLogic( rMEvt.GetPosPixel() );

    if ( rMEvt.IsLeft() )
    {
        FrameView* pFrameView = mpViewShell->GetFrameView();

        bool bOrtho = FALSE;

        BOOL bRestricted = TRUE;

        if (mpView->IsDragObj())
        {
            // object is dragged (move, resize,...)
            const SdrHdl* pHdl = mpView->GetDragStat().GetHdl();

            if (!pHdl || (!pHdl->IsCornerHdl() && !pHdl->IsVertexHdl()))
            {
                // Move
                bRestricted = FALSE;
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
            mpView->SetSnapEnabled(TRUE);
        BOOL bCntrl = rMEvt.IsMod1();

        BOOL bGridSnap = pFrameView->IsGridSnap();
        bGridSnap = (bCntrl != bGridSnap);

        if (mpView->IsGridSnap() != bGridSnap)
            mpView->SetGridSnap(bGridSnap);

        BOOL bBordSnap = pFrameView->IsBordSnap();
        bBordSnap = (bCntrl != bBordSnap);

        if (mpView->IsBordSnap() != bBordSnap)
            mpView->SetBordSnap(bBordSnap);

        BOOL bHlplSnap = pFrameView->IsHlplSnap();
        bHlplSnap = (bCntrl != bHlplSnap);

        if (mpView->IsHlplSnap() != bHlplSnap)
            mpView->SetHlplSnap(bHlplSnap);

        BOOL bOFrmSnap = pFrameView->IsOFrmSnap();
        bOFrmSnap = (bCntrl != bOFrmSnap);

        if (mpView->IsOFrmSnap() != bOFrmSnap)
            mpView->SetOFrmSnap(bOFrmSnap);

        BOOL bOPntSnap = pFrameView->IsOPntSnap();
        bOPntSnap = (bCntrl != bOPntSnap);

        if (mpView->IsOPntSnap() != bOPntSnap)
            mpView->SetOPntSnap(bOPntSnap);

        BOOL bOConSnap = pFrameView->IsOConSnap();
        bOConSnap = (bCntrl != bOConSnap);

        if (mpView->IsOConSnap() != bOConSnap)
            mpView->SetOConSnap(bOConSnap);

        BOOL bAngleSnap = rMEvt.IsShift() == !pFrameView->IsAngleSnapEnabled();

        if (mpView->IsAngleSnapEnabled() != bAngleSnap)
            mpView->SetAngleSnapEnabled(bAngleSnap);

        if (mpView->IsOrtho() != bOrtho)
            mpView->SetOrtho(bOrtho);

        BOOL bCenter = rMEvt.IsMod2();

        if ( mpView->IsCreate1stPointAsCenter() != bCenter ||
             mpView->IsResizeAtCenter() != bCenter )
        {
            mpView->SetCreate1stPointAsCenter(bCenter);
            mpView->SetResizeAtCenter(bCenter);
        }

        SdrPageView* pPV = 0;
        USHORT nHitLog = USHORT ( mpWindow->PixelToLogic(Size(HITPIX,0)).Width() );

        // look only for HelpLines when they are visible (!)
        BOOL bHelpLine(FALSE);
        if(mpView->IsHlplVisible())
            bHelpLine = mpView->PickHelpLine(aMDPos, nHitLog, *mpWindow, nHelpLine, pPV);
        BOOL bHitHdl = (mpView->PickHandle(aMDPos) != NULL);

        if ( bHelpLine
            && !mpView->IsCreateObj()
            && ((mpView->GetEditMode() == SDREDITMODE_EDIT && !bHitHdl) || (rMEvt.IsShift() && bCntrl)) )
        {
            mpWindow->CaptureMouse();
            mpView->BegDragHelpLine(nHelpLine, pPV);
            bDragHelpLine = mpView->IsDragHelpLine();
            bReturn = TRUE;
        }
    }
    ForcePointer(&rMEvt);

    return bReturn;
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/

BOOL FuDraw::MouseMove(const MouseEvent& rMEvt)
{
    FrameView* pFrameView = mpViewShell->GetFrameView();
    Point aPos = mpWindow->PixelToLogic( rMEvt.GetPosPixel() );

    bool bOrtho = FALSE;

    BOOL bRestricted = TRUE;

    if (mpView->IsDragObj())
    {
        // object is dragged (move, resize, ...)
        const SdrHdl* pHdl = mpView->GetDragStat().GetHdl();

        if (!pHdl || (!pHdl->IsCornerHdl() && !pHdl->IsVertexHdl()))
        {
            // Move
            bRestricted = FALSE;
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

        BOOL bCntrl = rMEvt.IsMod1();
        mpView->SetDragWithCopy(rMEvt.IsMod1() && pFrameView->IsDragWithCopy());

        BOOL bGridSnap = pFrameView->IsGridSnap();
        bGridSnap = (bCntrl != bGridSnap);

        if (mpView->IsGridSnap() != bGridSnap)
            mpView->SetGridSnap(bGridSnap);

        BOOL bBordSnap = pFrameView->IsBordSnap();
        bBordSnap = (bCntrl != bBordSnap);

        if (mpView->IsBordSnap() != bBordSnap)
            mpView->SetBordSnap(bBordSnap);

        BOOL bHlplSnap = pFrameView->IsHlplSnap();
        bHlplSnap = (bCntrl != bHlplSnap);

        if (mpView->IsHlplSnap() != bHlplSnap)
            mpView->SetHlplSnap(bHlplSnap);

        BOOL bOFrmSnap = pFrameView->IsOFrmSnap();
        bOFrmSnap = (bCntrl != bOFrmSnap);

        if (mpView->IsOFrmSnap() != bOFrmSnap)
            mpView->SetOFrmSnap(bOFrmSnap);

        BOOL bOPntSnap = pFrameView->IsOPntSnap();
        bOPntSnap = (bCntrl != bOPntSnap);

        if (mpView->IsOPntSnap() != bOPntSnap)
            mpView->SetOPntSnap(bOPntSnap);

        BOOL bOConSnap = pFrameView->IsOConSnap();
        bOConSnap = (bCntrl != bOConSnap);

        if (mpView->IsOConSnap() != bOConSnap)
            mpView->SetOConSnap(bOConSnap);

        BOOL bAngleSnap = rMEvt.IsShift() == !pFrameView->IsAngleSnapEnabled();

        if (mpView->IsAngleSnapEnabled() != bAngleSnap)
            mpView->SetAngleSnapEnabled(bAngleSnap);

        if (mpView->IsOrtho() != bOrtho)
            mpView->SetOrtho(bOrtho);

        BOOL bCenter = rMEvt.IsMod2();

        if ( mpView->IsCreate1stPointAsCenter() != bCenter ||
             mpView->IsResizeAtCenter() != bCenter )
        {
            mpView->SetCreate1stPointAsCenter(bCenter);
            mpView->SetResizeAtCenter(bCenter);
        }

        if ( mpView->IsDragHelpLine() )
            mpView->MovDragHelpLine(aPos);
    }

    BOOL bReturn = mpView->MouseMove(rMEvt, mpWindow);

    if (mpView->IsAction())
    {
        // Because the flag set back if necessary in MouseMove
        if (mpView->IsOrtho() != bOrtho)
            mpView->SetOrtho(bOrtho);
    }

    ForcePointer(&rMEvt);

    return bReturn;
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

BOOL FuDraw::MouseButtonUp(const MouseEvent& rMEvt)
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
    mpView->SetSnapEnabled(TRUE);
    mpView->SetCreate1stPointAsCenter(FALSE);
    mpView->SetResizeAtCenter(FALSE);
    mpView->SetDragWithCopy(pFrameView->IsDragWithCopy());
    mpView->SetGridSnap(pFrameView->IsGridSnap());
    mpView->SetBordSnap(pFrameView->IsBordSnap());
    mpView->SetHlplSnap(pFrameView->IsHlplSnap());
    mpView->SetOFrmSnap(pFrameView->IsOFrmSnap());
    mpView->SetOPntSnap(pFrameView->IsOPntSnap());
    mpView->SetOConSnap(pFrameView->IsOConSnap());

    bIsInDragMode = FALSE;
    ForcePointer(&rMEvt);
    FuPoor::MouseButtonUp(rMEvt);

    return FALSE;
}

/*************************************************************************
|*
|* Process keyboard-events
|*
|* When processing a KeyEvent the returnvalue is TRUE, otherwise FALSE.
|*
\************************************************************************/

BOOL FuDraw::KeyInput(const KeyEvent& rKEvt)
{
    BOOL bReturn = FALSE;

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
                if ( mpView && mpView->IsPresObjSelected(FALSE, TRUE, FALSE, TRUE) )
                {
                    InfoBox(mpWindow, String(SdResId(STR_ACTION_NOTPOSSIBLE) ) ).Execute();
                }
                else
                {
                    // Falls IP-Client aktiv, werden die Pointer
                    // auf das OLE- und das alte Graphic-Object
                    // am SdClient zurueckgesetzt, damit bei
                    // ::SelectionHasChanged nach dem Loeschen
                    // nicht mehr versucht wird, ein Grafik-Objekt
                    // zu restaurieren, das gar nicht mehr existiert.
                    // Alle anderen OLE-Objekte sind davon nicht
                    // betroffen
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
            bReturn = TRUE;
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

                bReturn = TRUE;
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
                mpView->MarkNextObj(FALSE);

                if(mpView->AreObjectsMarked())
                    mpView->MakeVisible(mpView->GetAllMarkedRect(), *mpWindow);

                bReturn = TRUE;
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
                mpView->MarkNextObj(TRUE);

                if(mpView->AreObjectsMarked())
                    mpView->MakeVisible(mpView->GetAllMarkedRect(), *mpWindow);

                bReturn = TRUE;
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

/*************************************************************************
|*
|* Fade out the selection-presentation before scrolling
|*
\************************************************************************/

void FuDraw::ScrollStart()
{
}

/*************************************************************************
|*
|* After scrolling show the selection-presentation again
|*
\************************************************************************/

void FuDraw::ScrollEnd()
{
}

/*************************************************************************
|*
|* Aktivate function
|*
\************************************************************************/

void FuDraw::Activate()
{
    FuPoor::Activate();
    ForcePointer();
}

/*************************************************************************
|*
|* Deaktivate function
|*
\************************************************************************/

void FuDraw::Deactivate()
{
    FuPoor::Deactivate();
}


/*************************************************************************
|*
|* Toggle mouse-pointer
|*
\************************************************************************/

void FuDraw::ForcePointer(const MouseEvent* pMEvt)
{
    Point aPnt;
    USHORT nModifier = 0;
    BOOL bLeftDown = FALSE;
    BOOL bDefPointer = TRUE;

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
            /******************************************************************
            * Giesskannenmodus
            ******************************************************************/
            bDefPointer = FALSE;
            mpWindow->SetPointer(Pointer(POINTER_FILL));
        }
    }
    else
    {
        SdrHdl* pHdl = mpView->PickHandle(aPnt);

        if (SD_MOD()->GetWaterCan() && !pHdl)
        {
            /******************************************************************
            * Giesskannenmodus
            ******************************************************************/
            bDefPointer = FALSE;
            mpWindow->SetPointer(Pointer(POINTER_FILL));
        }
        else if (!pHdl &&
                 mpViewShell->GetViewFrame()->HasChildWindow(SvxBmpMaskChildWindow::GetChildWindowId()))
        {
            /******************************************************************
            * Pipettenmodus
            ******************************************************************/
            SvxBmpMask* pMask = (SvxBmpMask*) mpViewShell->GetViewFrame()->GetChildWindow(SvxBmpMaskChildWindow::GetChildWindowId())->GetWindow();

            if (pMask && pMask->IsEyedropping())
            {
                bDefPointer = FALSE;
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
                    bDefPointer = FALSE;     // Otherwise it'll be calles Joes routine and the mousepointer will reconfigurate again
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
                UINT16 nSdrObjKind = aVEvt.pObj->GetObjIdentifier();

                if ( nSdrObjKind != OBJ_TEXT        &&
                     nSdrObjKind != OBJ_TITLETEXT   &&
                     nSdrObjKind != OBJ_OUTLINETEXT &&
                     aVEvt.pObj->IsEmptyPresObj() )
                {
                    pObj = NULL;
                    bDefPointer = FALSE;
                    mpWindow->SetPointer(Pointer(POINTER_ARROW));
                }
            }

            if (pObj && pMEvt && !pMEvt->IsMod2() && this->ISA(FuSelection))
            {
                // Auf Animation oder ImageMap pruefen
                bDefPointer = !SetPointer(pObj, aPnt);

                if (bDefPointer && (pObj->ISA(SdrObjGroup) || pObj->ISA(E3dPolyScene)))
                {
                    // In die Gruppe hineinschauen
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

/*************************************************************************
|*
|* Set cursor for animaton or imagemap
|*
\************************************************************************/

BOOL FuDraw::SetPointer(SdrObject* pObj, const Point& rPos)
{
    BOOL bSet = FALSE;

    BOOL bAnimationInfo = (!mpDocSh->ISA(GraphicDocShell) &&
                          mpDoc->GetAnimationInfo(pObj)) ? TRUE:FALSE;

    BOOL bImageMapInfo = FALSE;

    if (!bAnimationInfo)
        bImageMapInfo = mpDoc->GetIMapInfo(pObj) ? TRUE:FALSE;

    if (bAnimationInfo || bImageMapInfo)
    {
        const SetOfByte* pVisiLayer = &mpView->GetSdrPageView()->GetVisibleLayers();
        USHORT nHitLog(USHORT (mpWindow->PixelToLogic(Size(HITPIX,0)).Width()));
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
                        // Animations-Objekt
                        bSet = TRUE;
                        mpWindow->SetPointer(Pointer(POINTER_REFHAND));
                    }
            }
            else if (bImageMapInfo &&
                     mpDoc->GetHitIMapObject(pObj, rPos, *mpWindow))
            {
                /******************************************************
                * ImageMap
                ******************************************************/
                bSet = TRUE;
                mpWindow->SetPointer(Pointer(POINTER_REFHAND));
            }
        }
    }

    return bSet;
}



/*************************************************************************
|*
|* Response of doubleclick
|*
\************************************************************************/

void FuDraw::DoubleClick(const MouseEvent& rMEvt)
{
    USHORT nHitLog = USHORT ( mpWindow->PixelToLogic(Size(HITPIX,0)).Width() );

    if ( mpView->AreObjectsMarked() )
    {
        const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();

        if (rMarkList.GetMarkCount() == 1)
        {
            SdrMark* pMark = rMarkList.GetMark(0);
            SdrObject* pObj = pMark->GetMarkedSdrObj();

            UINT32 nInv = pObj->GetObjInventor();
            UINT16 nSdrObjKind = pObj->GetObjIdentifier();

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
                mpView->MarkObj(aMDPos, nHitLog, rMEvt.IsShift(), TRUE);
            }
        }
    }
    else
        mpViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
}

/*************************************************************************
|*
|* Help-event
|*
\************************************************************************/

BOOL FuDraw::RequestHelp(const HelpEvent& rHEvt)
{
    BOOL bReturn = FALSE;

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
                // In die Gruppe hineinschauen
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



/*************************************************************************
|*
|* Command-event
|*
\************************************************************************/

BOOL FuDraw::SetHelpText(SdrObject* pObj, const Point& rPosPixel, const SdrViewEvent& rVEvt)
{
    BOOL bSet = FALSE;
    String aHelpText;
    Point aPos(mpWindow->PixelToLogic(mpWindow->ScreenToOutputPixel(rPosPixel)));

    // URL fuer IMapObject unter Pointer ist Hilfetext
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
                aHelpText = String(SdResId(STR_CLICK_ACTION_PREVPAGE));
            }
            break;

            case presentation::ClickAction_NEXTPAGE:
            {
                // jump to the next page
                aHelpText = String(SdResId(STR_CLICK_ACTION_NEXTPAGE));
            }
            break;

            case presentation::ClickAction_FIRSTPAGE:
            {
                // jump to the first page
                aHelpText = String(SdResId(STR_CLICK_ACTION_FIRSTPAGE));
            }
            break;

            case presentation::ClickAction_LASTPAGE:
            {
                // jump to the last page
                aHelpText = String(SdResId(STR_CLICK_ACTION_LASTPAGE));
            }
            break;

            case presentation::ClickAction_BOOKMARK:
            {
                // jump to object/page
                aHelpText = String(SdResId(STR_CLICK_ACTION_BOOKMARK));
                aHelpText.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ": " ) );
                aHelpText.Append( String(INetURLObject::decode( pInfo->GetBookmark(), '%', INetURLObject::DECODE_WITH_CHARSET ) ));
            }
            break;

            case presentation::ClickAction_DOCUMENT:
            {
                // jump to document (object/page)
                aHelpText = String(SdResId(STR_CLICK_ACTION_DOCUMENT));
                aHelpText.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ": " ) );
                aHelpText.Append( String(INetURLObject::decode( pInfo->GetBookmark(), '%', INetURLObject::DECODE_WITH_CHARSET ) ));
            }
            break;

            case presentation::ClickAction_PROGRAM:
            {
                // execute program
                aHelpText = String(SdResId(STR_CLICK_ACTION_PROGRAM));
                aHelpText.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ": " ) );
                aHelpText.Append( String(INetURLObject::decode( pInfo->GetBookmark(), '%', INetURLObject::DECODE_WITH_CHARSET ) ));
            }
            break;

            case presentation::ClickAction_MACRO:
            {
                // execute program
                aHelpText = String(SdResId(STR_CLICK_ACTION_MACRO));
                aHelpText.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ": " ) );

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
                aHelpText = String(SdResId(STR_CLICK_ACTION_SOUND));
            }
            break;

            case presentation::ClickAction_VERB:
            {
                // execute OLE-verb
                aHelpText = String(SdResId(STR_CLICK_ACTION_VERB));
            }
            break;

            case presentation::ClickAction_STOPPRESENTATION:
            {
                // quit presentation
                aHelpText = String(SdResId(STR_CLICK_ACTION_STOPPRESENTATION));
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
        bSet = TRUE;
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
