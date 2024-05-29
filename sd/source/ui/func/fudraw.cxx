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

#include <vcl/svapp.hxx>
#include <vcl/ptrstyle.hxx>
#include <editeng/flditem.hxx>
#include <svx/svdogrp.hxx>
#include <tools/urlobj.hxx>
#include <vcl/help.hxx>
#include <svx/bmpmask.hxx>
#include <svx/svdotext.hxx>
#include <svx/ImageMapInfo.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/sfxhelp.hxx>
#include <svx/svdpagv.hxx>
#include <vcl/imapobj.hxx>
#include <svx/svxids.hrc>
#include <svx/obj3d.hxx>
#include <svx/scene3d.hxx>
#include <sfx2/viewfrm.hxx>

#include <strings.hrc>


#include <sdmod.hxx>
#include <fudraw.hxx>
#include <ViewShell.hxx>
#include <FrameView.hxx>
#include <View.hxx>
#include <Window.hxx>
#include <drawdoc.hxx>
#include <DrawDocShell.hxx>
#include <sdresid.hxx>
#include <fusel.hxx>
#include <vcl/weld.hxx>
#include <svx/sdrhittesthelper.hxx>

using namespace ::com::sun::star;

namespace sd {


/**
 * Base-class for all drawmodul-specific functions
 */
FuDraw::FuDraw(ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView,
               SdDrawDocument* pDoc, SfxRequest& rReq)
    : FuPoor(pViewSh, pWin, pView, pDoc, rReq)
    , aNewPointer(PointerStyle::Arrow)
    , aOldPointer(PointerStyle::Arrow)
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
            // rectangle->square, ellipse->circle, etc.
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
            bHelpLine = mpView->PickHelpLine(aMDPos, nHitLog, *mpWindow->GetOutDev(), nHelpLine, pPV);
        bool bHitHdl = (mpView->PickHandle(aMDPos) != nullptr);

        if ( bHelpLine
            && !mpView->IsCreateObj()
            && ((mpView->GetEditMode() == SdrViewEditMode::Edit && !bHitHdl) || (rMEvt.IsShift() && bSnapModPressed)) )
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
            // rectangle->square, ellipse->circle, images, etc.
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

    bool bReturn = mpView->MouseMove(rMEvt, mpWindow->GetOutDev());

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
        ::tools::Rectangle aOutputArea(Point(0,0), mpWindow->GetOutputSizePixel());

        if (mpView && !aOutputArea.Contains(rMEvt.GetPosPixel()))
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
                    std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(mpWindow->GetFrameWeld(),
                                                                  VclMessageType::Info, VclButtonsType::Ok,
                                                                  SdResId(STR_ACTION_NOTPOSSIBLE)));
                    xInfoBox->run();
                }
                else
                {
                    // wait-mousepointer while deleting object
                    weld::WaitObject aWait(mpViewShell->GetFrameWeld());
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
                    if ( mpView->HasMultipleMarkableObjects() && mpView->GetMarkedObjectList().GetMarkCount() != 0 )
                    {
                        // No next object: go over open end and get first from
                        // the other side
                        mpView->UnmarkAllObj();
                        mpView->MarkNextObj(!aCode.IsShift());
                    }
                }

                if(mpView->GetMarkedObjectList().GetMarkCount() != 0)
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

                if(mpView->GetMarkedObjectList().GetMarkCount() != 0)
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

                if(mpView->GetMarkedObjectList().GetMarkCount() != 0)
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
            mpWindow->SetPointer(PointerStyle::Fill);
        }
    }
    else
    {
        SdrHdl* pHdl = mpView->PickHandle(aPnt);

        if (SD_MOD()->GetWaterCan() && !pHdl)
        {
            // water can mode
            bDefPointer = false;
            mpWindow->SetPointer(PointerStyle::Fill);
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
                mpWindow->SetPointer(PointerStyle::RefHand);
            }
        }
        else if (!mpView->IsAction())
        {
            SdrObject* pObj = nullptr;
            SdrPageView* pPV = nullptr;
            SdrViewEvent aVEvt;
            SdrHitKind eHit = SdrHitKind::NONE;
            SdrDragMode eDragMode = mpView->GetDragMode();

            if (pMEvt)
            {
                eHit = mpView->PickAnything(*pMEvt, SdrMouseEventKind::MOVE, aVEvt);
            }

            if ((eDragMode == SdrDragMode::Rotate) && (eHit == SdrHitKind::MarkedObject))
            {
                // The goal of this request is show always the rotation arrow for 3D-objects at rotation mode
                // Independent of the settings at Tools->Options->Draw "Objects always moveable"
                // 2D-objects acquit in another way. Otherwise, the rotation of 3d-objects around any axes
                // wouldn't be possible per default.
                const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();
                SdrObject* pObject = rMarkList.GetMark(0)->GetMarkedSdrObj();
                if (DynCastE3dObject(pObject) && (rMarkList.GetMarkCount() == 1))
                {
                    mpWindow->SetPointer(PointerStyle::Rotate);
                    bDefPointer = false;     // Otherwise it'll be called Joe's routine and the mousepointer will reconfigurate again
                }
            }

            if (eHit == SdrHitKind::NONE)
            {
                // found nothing -> look after at the masterpage
                pObj = mpView->PickObj(aPnt, mpView->getHitTolLog(), pPV, SdrSearchOptions::ALSOONMASTER);
            }
            else if (eHit == SdrHitKind::UnmarkedObject)
            {
                pObj = aVEvt.mpObj;
            }
            else if (eHit == SdrHitKind::TextEditObj && dynamic_cast< const FuSelection *>( this ) !=  nullptr)
            {
                SdrObjKind nSdrObjKind = aVEvt.mpObj->GetObjIdentifier();

                if ( nSdrObjKind != SdrObjKind::Text        &&
                     nSdrObjKind != SdrObjKind::TitleText   &&
                     nSdrObjKind != SdrObjKind::OutlineText &&
                     aVEvt.mpObj->IsEmptyPresObj() )
                {
                    pObj = nullptr;
                    bDefPointer = false;
                    mpWindow->SetPointer(PointerStyle::Arrow);
                }
            }

            if (pObj && pMEvt && !pMEvt->IsMod2()
                && dynamic_cast<const FuSelection*>(this) != nullptr)
            {
                // test for ImageMap
                bDefPointer = !SetPointer(pObj, aPnt);

                if (bDefPointer
                    && (dynamic_cast<const SdrObjGroup*>(pObj) != nullptr
                        || DynCastE3dScene(pObj)))
                {
                    // take a glance into the group
                    pObj = mpView->PickObj(aPnt, mpView->getHitTolLog(), pPV,
                                           SdrSearchOptions::ALSOONMASTER | SdrSearchOptions::DEEP);
                    if (pObj)
                        bDefPointer = !SetPointer(pObj, aPnt);
                }
            }
        }
    }

    if (bDefPointer)
    {
        mpWindow->SetPointer(mpView->GetPreferredPointer(
                            aPnt, mpWindow->GetOutDev(), nModifier, bLeftDown));
    }
}

/**
 * Set cursor to pointer when in clickable area of an ImageMap
 *
 * @return True when pointer was set
 */
bool FuDraw::SetPointer(const SdrObject* pObj, const Point& rPos)
{
    bool bImageMapInfo = SvxIMapInfo::GetIMapInfo(pObj) != nullptr;

    if (!bImageMapInfo)
        return false;

    const SdrLayerIDSet* pVisiLayer = &mpView->GetSdrPageView()->GetVisibleLayers();
    double fHitLog(mpWindow->PixelToLogic(Size(HITPIX, 0)).Width());
    ::tools::Long n2HitLog(fHitLog * 2);
    Point aHitPosR(rPos);
    Point aHitPosL(rPos);
    Point aHitPosT(rPos);
    Point aHitPosB(rPos);

    aHitPosR.AdjustX(n2HitLog);
    aHitPosL.AdjustX(-n2HitLog);
    aHitPosT.AdjustY(n2HitLog);
    aHitPosB.AdjustY(-n2HitLog);

    if (!pObj->IsClosedObj()
        || (SdrObjectPrimitiveHit(*pObj, aHitPosR, {fHitLog, fHitLog}, *mpView->GetSdrPageView(), pVisiLayer,
                                  false)
            && SdrObjectPrimitiveHit(*pObj, aHitPosL, {fHitLog, fHitLog}, *mpView->GetSdrPageView(),
                                     pVisiLayer, false)
            && SdrObjectPrimitiveHit(*pObj, aHitPosT, {fHitLog, fHitLog}, *mpView->GetSdrPageView(),
                                     pVisiLayer, false)
            && SdrObjectPrimitiveHit(*pObj, aHitPosB, {fHitLog, fHitLog}, *mpView->GetSdrPageView(),
                                     pVisiLayer, false)))
    {
        // hit inside the object (without margin) or open object
        if (SvxIMapInfo::GetHitIMapObject(pObj, rPos))
        {
            mpWindow->SetPointer(PointerStyle::RefHand);
            return true;
        }
    }

    return false;
}

/**
 * Response of doubleclick
 */
void FuDraw::DoubleClick(const MouseEvent& rMEvt)
{
    sal_uInt16 nHitLog = sal_uInt16 ( mpWindow->PixelToLogic(Size(HITPIX,0)).Width() );

    if ( mpView->GetMarkedObjectList().GetMarkCount() != 0 )
    {
        const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();

        if (rMarkList.GetMarkCount() == 1)
        {
            SdrMark* pMark = rMarkList.GetMark(0);
            SdrObject* pObj = pMark->GetMarkedSdrObj();

            SdrInventor nInv = pObj->GetObjInventor();
            SdrObjKind  nSdrObjKind = pObj->GetObjIdentifier();

            if (nInv == SdrInventor::Default && nSdrObjKind == SdrObjKind::OLE2)
            {
                // activate OLE-object
                SfxInt16Item aItem(SID_OBJECT, 0);
                mpViewShell->GetViewFrame()->
                    GetDispatcher()->ExecuteList(SID_OBJECT,
                                                 SfxCallMode::ASYNCHRON | SfxCallMode::RECORD,
                                                 { &aItem });
            }
            else if (nInv == SdrInventor::Default &&  nSdrObjKind == SdrObjKind::Graphic && pObj->IsEmptyPresObj() )
            {
                mpViewShell->GetViewFrame()->
                    GetDispatcher()->Execute( SID_INSERT_GRAPHIC,
                                              SfxCallMode::ASYNCHRON | SfxCallMode::RECORD );
            }
            else if ( ( DynCastSdrTextObj( pObj ) != nullptr || dynamic_cast< const SdrObjGroup *>( pObj ) !=  nullptr ) &&
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
            else if (nInv == SdrInventor::Default &&  nSdrObjKind == SdrObjKind::Group)
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

        SdrObject* pObj = aVEvt.mpObj;

        if (eHit != SdrHitKind::NONE && pObj != nullptr)
        {
            Point aPosPixel = rHEvt.GetMousePosPixel();

            bReturn = SetHelpText(pObj, aPosPixel, aVEvt);

            if (!bReturn && (dynamic_cast< const SdrObjGroup *>( pObj ) != nullptr || DynCastE3dScene(pObj)))
            {
                // take a glance into the group
                SdrPageView* pPV = nullptr;

                Point aPos(mpWindow->PixelToLogic(mpWindow->ScreenToOutputPixel(aPosPixel)));

                pObj = mpView->PickObj(aPos, mpView->getHitTolLog(), pPV, SdrSearchOptions::ALSOONMASTER | SdrSearchOptions::DEEP);
                if (pObj)
                    bReturn = SetHelpText(pObj, aPosPixel, aVEvt);
            }
        }
    }

    if (!bReturn)
    {
        bReturn = FuPoor::RequestHelp(rHEvt);
    }

    if (!bReturn)
       bReturn = mpView->RequestHelp(rHEvt);

    return bReturn;
}

bool FuDraw::SetHelpText(const SdrObject* pObj, const Point& rPosPixel, const SdrViewEvent& rVEvt)
{
    OUString aHelpText;
    Point aPos(mpWindow->PixelToLogic(mpWindow->ScreenToOutputPixel(rPosPixel)));
    IMapObject* pIMapObj = SvxIMapInfo::GetHitIMapObject(pObj, aPos);

    if (!rVEvt.mpURLField && !pIMapObj)
        return false;

    OUString aURL;
    if (rVEvt.mpURLField)
        aURL = INetURLObject::decode(rVEvt.mpURLField->GetURL(),
                                     INetURLObject::DecodeMechanism::WithCharset);
    else if (pIMapObj)
    {
        aURL = pIMapObj->GetAltText() +
            " (" +
            INetURLObject::decode(pIMapObj->GetURL(),
                                        INetURLObject::DecodeMechanism::WithCharset) +
            ")";
    }
    else
        return false;

    aHelpText = SfxHelp::GetURLHelpText(aURL);

    if (aHelpText.isEmpty())
        return false;

    ::tools::Rectangle aLogicPix = mpWindow->LogicToPixel(pObj->GetLogicRect());
    ::tools::Rectangle aScreenRect(mpWindow->OutputToScreenPixel(aLogicPix.TopLeft()),
                            mpWindow->OutputToScreenPixel(aLogicPix.BottomRight()));

    if (Help::IsBalloonHelpEnabled())
        Help::ShowBalloon( static_cast<vcl::Window*>(mpWindow), rPosPixel, aScreenRect, aHelpText);
    else if (Help::IsQuickHelpEnabled())
        Help::ShowQuickHelp( static_cast<vcl::Window*>(mpWindow), aScreenRect, aHelpText);

    return true;
}

/** is called when the current function should be aborted. <p>
    This is used when a function gets a KEY_ESCAPE but can also
    be called directly.

    @returns true if an active function was aborted
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
        rBindings.Invalidate( SID_DEC_INDENT );
        rBindings.Invalidate( SID_INC_INDENT );
        rBindings.Invalidate( SID_PARASPACE_INCREASE );
        rBindings.Invalidate( SID_PARASPACE_DECREASE );
    }
    else if ( mpView->GetMarkedObjectList().GetMarkCount() != 0 )
    {
        const SdrHdlList& rHdlList = mpView->GetHdlList();
        SdrHdl* pHdl = rHdlList.GetFocusHdl();

        if(pHdl)
        {
            const_cast<SdrHdlList&>(rHdlList).ResetFocusHdl();
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
