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

#include <sal/config.h>

#include <editeng/eeitem.hxx>
#include <vcl/waitobj.hxx>
#include <editeng/flditem.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdoole2.hxx>
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


/**
 * Base-class for all drawmodul-specific functions
 */
FuDraw::FuDraw(ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView,
               SdDrawDocument* pDoc, SfxRequest& rReq)
    : FuPoor(pViewSh, pWin, pView, pDoc, rReq)
    , bMBDown(false)
    , bDragHelpLine(false)
    , nHelpLine(0)
    , bPermanent(false)
{
}

FuDraw::~FuDraw()
{
    mpView->BrkAction();
}


/**
 * Code shared by MouseButtonDown and MouseMove
 */
void FuDraw::DoModifiers(const MouseEvent& rMEvt, bool bSnapModPressed)
{
    FrameView* pFrameView = mpViewShell->GetFrameView();
    bool bGridSnap = pFrameView->IsGridSnap();
    bGridSnap = (bSnapModPressed != bGridSnap);

    if (mpView->IsGridSnap() != bGridSnap)
        mpView->SetGridSnap(bGridSnap);

    bool bBordSnap = pFrameView->IsBordSnap();
    bBordSnap = (bSnapModPressed != bBordSnap);

    if (mpView->IsBordSnap() != bBordSnap)
        mpView->SetBordSnap(bBordSnap);

    bool bHlplSnap = pFrameView->IsHlplSnap();
    bHlplSnap = (bSnapModPressed != bHlplSnap);

    if (mpView->IsHlplSnap() != bHlplSnap)
        mpView->SetHlplSnap(bHlplSnap);

    bool bOFrmSnap = pFrameView->IsOFrmSnap();
    bOFrmSnap = (bSnapModPressed != bOFrmSnap);

    if (mpView->IsOFrmSnap() != bOFrmSnap)
        mpView->SetOFrmSnap(bOFrmSnap);

    bool bOPntSnap = pFrameView->IsOPntSnap();
    bOPntSnap = (bSnapModPressed != bOPntSnap);

    if (mpView->IsOPntSnap() != bOPntSnap)
        mpView->SetOPntSnap(bOPntSnap);

    bool bOConSnap = pFrameView->IsOConSnap();
    bOConSnap = (bSnapModPressed != bOConSnap);

    if (mpView->IsOConSnap() != bOConSnap)
        mpView->SetOConSnap(bOConSnap);

    bool bAngleSnap = rMEvt.IsShift() == !pFrameView->IsAngleSnapEnabled();

    if (mpView->IsAngleSnapEnabled() != bAngleSnap)
        mpView->SetAngleSnapEnabled(bAngleSnap);

    bool bCenter = rMEvt.IsMod2();

    if ( mpView->IsCreate1stPointAsCenter() != bCenter ||
         mpView->IsResizeAtCenter() != bCenter )
    {
        mpView->SetCreate1stPointAsCenter(bCenter);
        mpView->SetResizeAtCenter(bCenter);
    }
}


bool FuDraw::MouseButtonDown(const MouseEvent& rMEvt)
{
    // remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    bool bReturn = false;
    bDragHelpLine = false;
    aMDPos = mpWindow->PixelToLogic( rMEvt.GetPosPixel() );

    if ( rMEvt.IsLeft() )
    {
        FrameView* pFrameView = mpViewShell->GetFrameView();

        bool bOrtho = false;

        bool bRestricted = true;

        if (mpView->IsDragObj())
        {
            // object is dragged (move, resize,...)
            const SdrHdl* pHdl = mpView->GetDragStat().GetHdl();

            if (!pHdl || (!pHdl->IsCornerHdl() && !pHdl->IsVertexHdl()))
            {
                // Move
                bRestricted = false;
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
            mpView->SetSnapEnabled(true);

        bool bSnapModPressed = rMEvt.IsMod1();
        if (mpView->IsOrtho() != bOrtho)
            mpView->SetOrtho(bOrtho);

        DoModifiers(rMEvt, bSnapModPressed);

        SdrPageView* pPV = nullptr;
        sal_uInt16 nHitLog = sal_uInt16 ( mpWindow->PixelToLogic(Size(HITPIX,0)).Width() );

        // look only for HelpLines when they are visible (!)
        bool bHelpLine(false);
        if(mpView->IsHlplVisible())
            bHelpLine = mpView->PickHelpLine(aMDPos, nHitLog, *mpWindow, nHelpLine, pPV);
        bool bHitHdl = (mpView->PickHandle(aMDPos) != nullptr);

        if ( bHelpLine
            && !mpView->IsCreateObj()
            && ((mpView->GetEditMode() == SDREDITMODE_EDIT && !bHitHdl) || (rMEvt.IsShift() && bSnapModPressed)) )
        {
            mpWindow->CaptureMouse();
            mpView->BegDragHelpLine(nHelpLine, pPV);
            bDragHelpLine = mpView->IsDragHelpLine();
            bReturn = true;
        }
    }
    ForcePointer(&rMEvt);

    return bReturn;
}

bool FuDraw::MouseMove(const MouseEvent& rMEvt)
{
    FrameView* pFrameView = mpViewShell->GetFrameView();
    Point aPos = mpWindow->PixelToLogic( rMEvt.GetPosPixel() );

    bool bOrtho = false;
    bool bRestricted = true;

    if (mpView->IsDragObj())
    {
        // object is dragged (move, resize, ...)
        const SdrHdl* pHdl = mpView->GetDragStat().GetHdl();

        if (!pHdl || (!pHdl->IsCornerHdl() && !pHdl->IsVertexHdl()))
        {
            // Move
            bRestricted = false;
        }
    }

    if (mpView->IsAction())
    {
        // #i33136# and fdo#88339
        if(bRestricted && doConstructOrthogonal())
        {
            // Scale proportionally by default:
            // rectangle->quadrat, ellipse->circle, Images etc.
            bOrtho = !rMEvt.IsShift();
        }
        else
        {
            bOrtho = rMEvt.IsShift() != pFrameView->IsOrtho();
        }

        bool bSnapModPressed = rMEvt.IsMod2();
        mpView->SetDragWithCopy(rMEvt.IsMod1() && pFrameView->IsDragWithCopy());

        if (mpView->IsOrtho() != bOrtho)
            mpView->SetOrtho(bOrtho);
        DoModifiers(rMEvt, bSnapModPressed);


        if ( mpView->IsDragHelpLine() )
            mpView->MovDragHelpLine(aPos);
    }

    bool bReturn = mpView->MouseMove(rMEvt, mpWindow);

    if (mpView->IsAction())
    {
        // Because the flag set back if necessary in MouseMove
        if (mpView->IsOrtho() != bOrtho)
            mpView->SetOrtho(bOrtho);
    }

    ForcePointer(&rMEvt);

    return bReturn;
}

bool FuDraw::MouseButtonUp(const MouseEvent& rMEvt)
{
    if (mpView && mpView->IsDragHelpLine())
        mpView->EndDragHelpLine();

    if ( bDragHelpLine )
    {
        Rectangle aOutputArea(Point(0,0), mpWindow->GetOutputSizePixel());

        if (mpView && !aOutputArea.IsInside(rMEvt.GetPosPixel()))
            mpView->GetSdrPageView()->DeleteHelpLine(nHelpLine);

        mpWindow->ReleaseMouse();
    }

    if (mpView)
    {
        FrameView* pFrameView = mpViewShell->GetFrameView();
        mpView->SetOrtho( pFrameView->IsOrtho() );
        mpView->SetAngleSnapEnabled( pFrameView->IsAngleSnapEnabled() );
        mpView->SetSnapEnabled(true);
        mpView->SetCreate1stPointAsCenter(false);
        mpView->SetResizeAtCenter(false);
        mpView->SetDragWithCopy(pFrameView->IsDragWithCopy());
        mpView->SetGridSnap(pFrameView->IsGridSnap());
        mpView->SetBordSnap(pFrameView->IsBordSnap());
        mpView->SetHlplSnap(pFrameView->IsHlplSnap());
        mpView->SetOFrmSnap(pFrameView->IsOFrmSnap());
        mpView->SetOPntSnap(pFrameView->IsOPntSnap());
        mpView->SetOConSnap(pFrameView->IsOConSnap());
    }

    bIsInDragMode = false;
    ForcePointer(&rMEvt);
    FuPoor::MouseButtonUp(rMEvt);

    return false;
}

/**
 * Process keyboard input
 * @returns sal_True if a KeyEvent is being processed, sal_False otherwise
 */
bool FuDraw::KeyInput(const KeyEvent& rKEvt)
{
    bool bReturn = false;

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
                if (mpView->IsPresObjSelected(false, true, false, true))
                {
                    ScopedVclPtrInstance<InfoBox>(mpWindow, SD_RESSTR(STR_ACTION_NOTPOSSIBLE))->Execute();
                }
                else
                {
                    /* If IP-Client active, we reset the pointer to the OLE- and
                       to the old graphic object of SdClient. With this, we
                       avoid the restoration of an no more existing object in
                       ::SelectionHasChanged after deletion. All other OLE
                       objects are not affected. */
                    OSL_ASSERT (mpViewShell->GetViewShell()!=nullptr);
                    Client* pIPClient = static_cast<Client*>(
                        mpViewShell->GetViewShell()->GetIPClient());
                    if (pIPClient && pIPClient->IsObjectInPlaceActive())
                        pIPClient->SetSdrGrafObj(nullptr);

                    // wait-mousepointer while deleting object
                    WaitObject aWait( static_cast<vcl::Window*>(mpViewShell->GetActiveWindow()) );
                    // delete object
                    mpView->DeleteMarked();
                }
            }
            bReturn = true;
        }
        break;

        case KEY_TAB:
        {
            vcl::KeyCode aCode = rKEvt.GetKeyCode();

            if ( !aCode.IsMod1() && !aCode.IsMod2() )
            {
                // Moved next line which was a bugfix itself into
                // the scope which really does the object selection travel
                // and thus is allowed to call SelectionHasChanged().

                // Switch to FuSelect.
                mpViewShell->GetViewFrame()->GetDispatcher()->Execute(
                    SID_OBJECT_SELECT,
                    SfxCallMode::ASYNCHRON | SfxCallMode::RECORD);

                // changeover to the next object
                if(!mpView->MarkNextObj( !aCode.IsShift() ))
                {
                    //If there is only one object, don't do the UnmarkAllObj() & MarkNextObj().
                    if ( mpView->GetMarkableObjCount() > 1 && mpView->AreObjectsMarked() )
                    {
                        // No next object: go over open end and get first from
                        // the other side
                        mpView->UnmarkAllObj();
                        mpView->MarkNextObj(!aCode.IsShift());
                    }
                }

                if(mpView->AreObjectsMarked())
                    mpView->MakeVisible(mpView->GetAllMarkedRect(), *mpWindow);

                bReturn = true;
            }
        }
        break;

        case KEY_END:
        {
            vcl::KeyCode aCode = rKEvt.GetKeyCode();

            if ( aCode.IsMod1() )
            {
                // mark last object
                mpView->UnmarkAllObj();
                mpView->MarkNextObj();

                if(mpView->AreObjectsMarked())
                    mpView->MakeVisible(mpView->GetAllMarkedRect(), *mpWindow);

                bReturn = true;
            }
        }
        break;

        case KEY_HOME:
        {
            vcl::KeyCode aCode = rKEvt.GetKeyCode();

            if ( aCode.IsMod1() )
            {
                // mark first object
                mpView->UnmarkAllObj();
                mpView->MarkNextObj(true);

                if(mpView->AreObjectsMarked())
                    mpView->MakeVisible(mpView->GetAllMarkedRect(), *mpWindow);

                bReturn = true;
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

    return bReturn;
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
    bool bLeftDown = false;
    bool bDefPointer = true;

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
            bDefPointer = false;
            mpWindow->SetPointer(Pointer(PointerStyle::Fill));
        }
    }
    else
    {
        SdrHdl* pHdl = mpView->PickHandle(aPnt);

        if (SD_MOD()->GetWaterCan() && !pHdl)
        {
            // water can mode
            bDefPointer = false;
            mpWindow->SetPointer(Pointer(PointerStyle::Fill));
        }
        else if (!pHdl &&
                 mpViewShell->GetViewFrame()->HasChildWindow(SvxBmpMaskChildWindow::GetChildWindowId()))
        {
            // pipette mode
            SfxChildWindow* pWnd = mpViewShell->GetViewFrame()->GetChildWindow(SvxBmpMaskChildWindow::GetChildWindowId());
            SvxBmpMask* pMask = pWnd ? static_cast<SvxBmpMask*>(pWnd->GetWindow()) : nullptr;
            if (pMask && pMask->IsEyedropping())
            {
                bDefPointer = false;
                mpWindow->SetPointer(Pointer(PointerStyle::RefHand));
            }
        }
        else if (!mpView->IsAction())
        {
            SdrObject* pObj = nullptr;
            SdrPageView* pPV = nullptr;
            SdrViewEvent aVEvt;
            SdrHitKind eHit = SDRHIT_NONE;
            SdrDragMode eDragMode = mpView->GetDragMode();

            if (pMEvt)
            {
                eHit = mpView->PickAnything(*pMEvt, SdrMouseEventKind::MOVE, aVEvt);
            }

            if ((eDragMode == SDRDRAG_ROTATE) && (eHit == SDRHIT_MARKEDOBJECT))
            {
                // The goal of this request is show always the rotation-arrow for 3D-objects at rotation-modus
                // Independent of the settings at Extras->Optionen->Grafik "Objekte immer verschieben"
                // 2D-objects acquit in an other way. Otherwise, the rotation of 3d-objects around any axes
                // wouldn't be possible per default.
                const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();
                SdrObject* pObject = rMarkList.GetMark(0)->GetMarkedSdrObj();
                if ((dynamic_cast<const E3dObject* >(pObject) !=  nullptr) && (rMarkList.GetMarkCount() == 1))
                {
                    mpWindow->SetPointer(Pointer(PointerStyle::Rotate));
                    bDefPointer = false;     // Otherwise it'll be calles Joes routine and the mousepointer will reconfigurate again
                }
            }

            if (eHit == SDRHIT_NONE)
            {
                // found nothing -> look after at the masterpage
                mpView->PickObj(aPnt, mpView->getHitTolLog(), pObj, pPV, SdrSearchOptions::ALSOONMASTER);
            }
            else if (eHit == SDRHIT_UNMARKEDOBJECT)
            {
                pObj = aVEvt.pObj;
            }
            else if (eHit == SDRHIT_TEXTEDITOBJ && dynamic_cast< const FuSelection *>( this ) !=  nullptr)
            {
                sal_uInt16 nSdrObjKind = aVEvt.pObj->GetObjIdentifier();

                if ( nSdrObjKind != OBJ_TEXT        &&
                     nSdrObjKind != OBJ_TITLETEXT   &&
                     nSdrObjKind != OBJ_OUTLINETEXT &&
                     aVEvt.pObj->IsEmptyPresObj() )
                {
                    pObj = nullptr;
                    bDefPointer = false;
                    mpWindow->SetPointer(Pointer(PointerStyle::Arrow));
                }
            }

            if (pObj && pMEvt && !pMEvt->IsMod2() && dynamic_cast< const   FuSelection *>( this ) !=  nullptr)
            {
                // test for animation or ImageMap
                bDefPointer = !SetPointer(pObj, aPnt);

                if (bDefPointer && (dynamic_cast< const SdrObjGroup *>( pObj ) != nullptr || dynamic_cast< const E3dPolyScene* >(pObj) !=  nullptr))
                {
                    // take a glance into the group
                    if (mpView->PickObj(aPnt, mpView->getHitTolLog(), pObj, pPV, SdrSearchOptions::ALSOONMASTER | SdrSearchOptions::DEEP))
                        bDefPointer = !SetPointer(pObj, aPnt);
                }
            }
        }
    }

    if (bDefPointer)
    {
        mpWindow->SetPointer(mpView->GetPreferredPointer(
                            aPnt, mpWindow, nModifier, bLeftDown));
    }
}

/**
 * Set cursor for animation or imagemap
 */
bool FuDraw::SetPointer(SdrObject* pObj, const Point& rPos)
{
    bool bSet = false;

    bool bAnimationInfo = dynamic_cast< const GraphicDocShell *>( mpDocSh ) ==  nullptr &&
                          mpDoc->GetAnimationInfo(pObj);

    bool bImageMapInfo = false;

    if (!bAnimationInfo)
        bImageMapInfo = mpDoc->GetIMapInfo(pObj) != nullptr;

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

                if(( dynamic_cast< const DrawView *>( mpView ) !=  nullptr &&
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
                    ( dynamic_cast< const DrawView *>( mpView ) !=  nullptr &&
                        SlideShow::IsRunning( mpViewShell->GetViewShellBase() )   &&
                         (pInfo->meClickAction == presentation::ClickAction_VANISH            ||
                          pInfo->meClickAction == presentation::ClickAction_INVISIBLE         ||
                          pInfo->meClickAction == presentation::ClickAction_STOPPRESENTATION ||
                         (pInfo->mbActive &&
                          ( pInfo->meEffect != presentation::AnimationEffect_NONE ||
                            pInfo->meTextEffect != presentation::AnimationEffect_NONE )))))
                    {
                        // Animation object
                        bSet = true;
                        mpWindow->SetPointer(Pointer(PointerStyle::RefHand));
                    }
            }
            else if (bImageMapInfo &&
                     mpDoc->GetHitIMapObject(pObj, rPos, *mpWindow))
            {
                /******************************************************
                * ImageMap
                ******************************************************/
                bSet = true;
                mpWindow->SetPointer(Pointer(PointerStyle::RefHand));
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
                    mpViewShell->ActivateObject( static_cast<SdrOle2Obj*>(pObj), 0);
                }
            }
            else if (nInv == SdrInventor &&  nSdrObjKind == OBJ_GRAF && pObj->IsEmptyPresObj() )
            {
                mpViewShell->GetViewFrame()->
                    GetDispatcher()->Execute( SID_INSERT_GRAPHIC,
                                              SfxCallMode::ASYNCHRON | SfxCallMode::RECORD );
            }
            else if ( ( dynamic_cast< const SdrTextObj *>( pObj ) != nullptr || dynamic_cast< const SdrObjGroup *>( pObj ) !=  nullptr ) &&
                      !SD_MOD()->GetWaterCan()                            &&
                      mpViewShell->GetFrameView()->IsDoubleClickTextEdit() &&
                      !mpDocSh->IsReadOnly())
            {
                SfxUInt16Item aItem(SID_TEXTEDIT, 2);
                mpViewShell->GetViewFrame()->GetDispatcher()->ExecuteList(
                        SID_TEXTEDIT,
                        SfxCallMode::ASYNCHRON | SfxCallMode::RECORD,
                        { &aItem });
            }
            else if (nInv == SdrInventor &&  nSdrObjKind == OBJ_GRUP)
            {
                // hit group -> select subobject
                mpView->UnMarkAll();
                mpView->MarkObj(aMDPos, nHitLog, rMEvt.IsShift(), true);
            }
        }
    }
    else
        mpViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SfxCallMode::ASYNCHRON | SfxCallMode::RECORD);
}

bool FuDraw::RequestHelp(const HelpEvent& rHEvt)
{
    bool bReturn = false;

    if (Help::IsBalloonHelpEnabled() || Help::IsQuickHelpEnabled())
    {
        SdrViewEvent aVEvt;

        MouseEvent aMEvt(mpWindow->GetPointerPosPixel(), 1, MouseEventModifiers::NONE, MOUSE_LEFT);

        SdrHitKind eHit = mpView->PickAnything(aMEvt, SdrMouseEventKind::BUTTONDOWN, aVEvt);

        SdrObject* pObj = aVEvt.pObj;

        if (eHit != SDRHIT_NONE && pObj != nullptr)
        {
            Point aPosPixel = rHEvt.GetMousePosPixel();

            bReturn = SetHelpText(pObj, aPosPixel, aVEvt);

            if (!bReturn && (dynamic_cast< const SdrObjGroup *>( pObj ) != nullptr || dynamic_cast< const E3dPolyScene* >(pObj) !=  nullptr))
            {
                // take a glance into the group
                SdrPageView* pPV = nullptr;

                Point aPos(mpWindow->PixelToLogic(mpWindow->ScreenToOutputPixel(aPosPixel)));

                if (mpView->PickObj(aPos, mpView->getHitTolLog(), pObj, pPV, SdrSearchOptions::ALSOONMASTER | SdrSearchOptions::DEEP))
                    bReturn = SetHelpText(pObj, aPosPixel, aVEvt);
            }
        }
    }

    if (!bReturn)
    {
        bReturn = FuPoor::RequestHelp(rHEvt);
    }

    return bReturn;
}

bool FuDraw::SetHelpText(SdrObject* pObj, const Point& rPosPixel, const SdrViewEvent& rVEvt)
{
    bool bSet = false;
    OUString aHelpText;
    Point aPos(mpWindow->PixelToLogic(mpWindow->ScreenToOutputPixel(rPosPixel)));

    // URL for IMapObject underneath pointer is help text
    if ( mpDoc->GetIMapInfo(pObj) )
    {
        IMapObject* pIMapObj = mpDoc->GetHitIMapObject(pObj, aPos, *mpWindow );

        if ( pIMapObj )
        {
            // show name
            aHelpText = pIMapObj->GetAltText();

            if (aHelpText.isEmpty())
            {
                // show url if no name is available
                aHelpText = INetURLObject::decode( pIMapObj->GetURL(), INetURLObject::DECODE_WITH_CHARSET );
            }
        }
    }
    else if (dynamic_cast< GraphicDocShell *>( mpDocSh ) ==  nullptr && mpDoc->GetAnimationInfo(pObj))
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
                aHelpText += ": ";
                aHelpText += INetURLObject::decode( pInfo->GetBookmark(), INetURLObject::DECODE_WITH_CHARSET );
            }
            break;

            case presentation::ClickAction_DOCUMENT:
            {
                // jump to document (object/page)
                aHelpText = SD_RESSTR(STR_CLICK_ACTION_DOCUMENT);
                aHelpText += ": ";
                aHelpText += INetURLObject::decode( pInfo->GetBookmark(), INetURLObject::DECODE_WITH_CHARSET );
            }
            break;

            case presentation::ClickAction_PROGRAM:
            {
                // execute program
                aHelpText = SD_RESSTR(STR_CLICK_ACTION_PROGRAM);
                aHelpText += ": ";
                aHelpText += INetURLObject::decode( pInfo->GetBookmark(), INetURLObject::DECODE_WITH_CHARSET );
            }
            break;

            case presentation::ClickAction_MACRO:
            {
                // execute program
                aHelpText = SD_RESSTR(STR_CLICK_ACTION_MACRO);
                aHelpText += ": ";

                if ( SfxApplication::IsXScriptURL( pInfo->GetBookmark() ) )
                {
                    aHelpText += pInfo->GetBookmark();
                }
                else
                {
                    OUString sBookmark( pInfo->GetBookmark() );
                    aHelpText += sBookmark.getToken( 2, '.' );
                    aHelpText += ".";
                    aHelpText += sBookmark.getToken( 1, '.' );
                    aHelpText += ".";
                    aHelpText += sBookmark.getToken( 0, '.' );
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
        aHelpText = INetURLObject::decode( rVEvt.pURLField->GetURL(), INetURLObject::DECODE_WITH_CHARSET );
    }

    if (!aHelpText.isEmpty())
    {
        bSet = true;
        Rectangle aLogicPix = mpWindow->LogicToPixel(pObj->GetLogicRect());
        Rectangle aScreenRect(mpWindow->OutputToScreenPixel(aLogicPix.TopLeft()),
                              mpWindow->OutputToScreenPixel(aLogicPix.BottomRight()));

        if (Help::IsBalloonHelpEnabled())
            Help::ShowBalloon( static_cast<vcl::Window*>(mpWindow), rPosPixel, aScreenRect, aHelpText);
        else if (Help::IsQuickHelpEnabled())
            Help::ShowQuickHelp( static_cast<vcl::Window*>(mpWindow), aScreenRect, aHelpText);
    }

    return bSet;
}

/** is called when the current function should be aborted. <p>
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
            SfxCallMode::ASYNCHRON | SfxCallMode::RECORD);

        bReturn = true;
    }

    return bReturn;
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
