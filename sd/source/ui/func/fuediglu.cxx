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

#include <fuediglu.hxx>
#include <svl/eitem.hxx>
#include <svx/svdglue.hxx>
#include <sfx2/request.hxx>

#include <app.hrc>

#include <Window.hxx>
#include <View.hxx>
#include <ViewShell.hxx>
#include <ViewShellBase.hxx>
#include <ToolBarManager.hxx>

namespace sd {


FuEditGluePoints::FuEditGluePoints (
    ViewShell* pViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument* pDoc,
    SfxRequest& rReq)
    : FuDraw(pViewSh, pWin, pView, pDoc, rReq)
     //Add Shift+UP/DOWN/LEFT/RIGHT key to move the position of insert point,
     //and SHIFT+ENTER key to decide the position and draw the new insert point
     ,bBeginInsertPoint(false),
    oldPoint(0,0)
{
}

rtl::Reference<FuPoor> FuEditGluePoints::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq, bool bPermanent )
{
    FuEditGluePoints* pFunc;
    rtl::Reference<FuPoor> xFunc( pFunc = new FuEditGluePoints( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    pFunc->SetPermanent( bPermanent );
    return xFunc;
}

void FuEditGluePoints::DoExecute( SfxRequest& rReq )
{
    FuDraw::DoExecute( rReq );
    mpView->SetInsGluePointMode(false);
    mpViewShell->GetViewShellBase().GetToolBarManager()->AddToolBar(
        ToolBarManager::ToolBarGroup::Function,
        ToolBarManager::msGluePointsToolBar);
}

FuEditGluePoints::~FuEditGluePoints()
{
    mpView->BrkAction();
    mpView->UnmarkAllGluePoints();
    mpView->SetInsGluePointMode(false);
}

bool FuEditGluePoints::MouseButtonDown(const MouseEvent& rMEvt)
{
    mpView->SetActualWin( mpWindow->GetOutDev() );

    bool bReturn = FuDraw::MouseButtonDown(rMEvt);

    if (mpView->IsAction())
    {
        if (rMEvt.IsRight())
            mpView->BckAction();

        return true;
    }

    if (rMEvt.IsLeft())
    {
        bReturn = true;
        sal_uInt16 nHitLog = sal_uInt16 ( mpWindow->PixelToLogic(Size(HITPIX,0)).Width() );
        sal_uInt16 nDrgLog = sal_uInt16 ( mpWindow->PixelToLogic(Size(mpView->GetDragThresholdPixels(),0)).Width() );
        mpWindow->CaptureMouse();

        SdrViewEvent aVEvt;
        SdrHitKind eHit = mpView->PickAnything(rMEvt, SdrMouseEventKind::BUTTONDOWN, aVEvt);

        if (eHit == SdrHitKind::Handle)
        {
            // drag handle
            SdrHdl* pHdl = aVEvt.mpHdl;

            if (mpView->IsGluePointMarked(aVEvt.mpObj, aVEvt.mnGlueId) && rMEvt.IsShift())
            {
                mpView->UnmarkGluePoint(aVEvt.mpObj, aVEvt.mnGlueId);
                pHdl = nullptr;
            }

            if (pHdl)
            {
                // drag handle
                mpView->BegDragObj(aMDPos, nullptr, aVEvt.mpHdl, nDrgLog);
            }
        }
        else if (eHit == SdrHitKind::MarkedObject && mpView->IsInsGluePointMode())
        {
            // insert gluepoints
            mpView->BegInsGluePoint(aMDPos);
        }
        else if (eHit == SdrHitKind::MarkedObject && rMEvt.IsMod1())
        {
            // select gluepoints
            if (!rMEvt.IsShift())
                mpView->UnmarkAllGluePoints();

            mpView->BegMarkGluePoints(aMDPos);
        }
        else if (eHit == SdrHitKind::MarkedObject && !rMEvt.IsShift() && !rMEvt.IsMod2())
        {
            // move object
            mpView->BegDragObj(aMDPos, nullptr, nullptr, nDrgLog);
        }
        else if (eHit == SdrHitKind::Gluepoint)
        {
            // select gluepoints
            if (!rMEvt.IsShift())
                mpView->UnmarkAllGluePoints();

            mpView->MarkGluePoint(aVEvt.mpObj, aVEvt.mnGlueId, false);
            SdrHdl* pHdl = mpView->GetGluePointHdl(aVEvt.mpObj, aVEvt.mnGlueId);

            if (pHdl)
            {
                mpView->BegDragObj(aMDPos, nullptr, pHdl, nDrgLog);
            }
        }
        else
        {
            // select or drag object
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

            const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();
            if (bMarked &&
                (!rMEvt.IsShift() || eHit == SdrHitKind::MarkedObject))
            {
                // move object
                mpView->BegDragObj(aMDPos, nullptr, aVEvt.mpHdl, nDrgLog);
            }
            else if (rMarkList.GetMarkCount() != 0)
            {
                // select gluepoint
                if (!rMEvt.IsShift())
                    mpView->UnmarkAllGluePoints();

                mpView->BegMarkGluePoints(aMDPos);
            }
            else
            {
                // select object
                mpView->BegMarkObj(aMDPos);
            }
        }

        ForcePointer(&rMEvt);
    }

    return bReturn;
}

bool FuEditGluePoints::MouseMove(const MouseEvent& rMEvt)
{
    mpView->SetActualWin( mpWindow->GetOutDev() );

    FuDraw::MouseMove(rMEvt);

    if (mpView->IsAction())
    {
        Point aPix(rMEvt.GetPosPixel());
        Point aPnt( mpWindow->PixelToLogic(aPix) );
        ForceScroll(aPix);
        mpView->MovAction(aPnt);
    }

    ForcePointer(&rMEvt);

    return true;
}

bool FuEditGluePoints::MouseButtonUp(const MouseEvent& rMEvt)
{
    mpView->SetActualWin( mpWindow->GetOutDev() );

    bool bReturn = false;

    if (mpView->IsAction())
    {
        bReturn = true;
        mpView->EndAction();
    }

    FuDraw::MouseButtonUp(rMEvt);

    sal_uInt16 nDrgLog = sal_uInt16 ( mpWindow->PixelToLogic(Size(mpView->GetDragThresholdPixels(),0)).Width() );
    Point aPos = mpWindow->PixelToLogic( rMEvt.GetPosPixel() );

    if (std::abs(aMDPos.X() - aPos.X()) < nDrgLog &&
        std::abs(aMDPos.Y() - aPos.Y()) < nDrgLog &&
        !rMEvt.IsShift() && !rMEvt.IsMod2())
    {
        SdrViewEvent aVEvt;
        SdrHitKind eHit = mpView->PickAnything(rMEvt, SdrMouseEventKind::BUTTONDOWN, aVEvt);

        if (eHit == SdrHitKind::NONE)
        {
            // click on position: deselect
            mpView->UnmarkAllObj();
        }
    }

    mpWindow->ReleaseMouse();

    return bReturn;
}

/**
 * Process keyboard input
 * @returns sal_True if a KeyEvent is being processed, sal_False otherwise
 */
bool FuEditGluePoints::KeyInput(const KeyEvent& rKEvt)
{
    mpView->SetActualWin( mpWindow->GetOutDev() );

    //Add Shift+UP/DOWN/LEFT/RIGHT key to move the position of insert point,
    //and SHIFT+ENTER key to decide the position and draw the new insert point

    bool bReturn = false;

    switch (rKEvt.GetKeyCode().GetCode())
    {
        case KEY_UP:
        case KEY_DOWN:
        case KEY_LEFT:
        case KEY_RIGHT:
        {
            if(rKEvt.GetKeyCode().IsShift()&& mpView->IsInsGluePointMode() ){
                ::tools::Long nX = 0;
                ::tools::Long nY = 0;
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
                Point aPoint = bBeginInsertPoint? oldPoint:centerPoint;
                Point ePoint = aPoint + Point(nX,nY);
                mpWindow->SetPointerPosPixel(ePoint);
                //simulate mouse move action
                MouseEvent eMevt(ePoint, 1, MouseEventModifiers::DRAGMOVE, MOUSE_LEFT, 0);
                MouseMove(eMevt);
                oldPoint = ePoint;
                bBeginInsertPoint = true;
                bReturn = true;
            }
        }
        break;
        case KEY_RETURN:
            if(rKEvt.GetKeyCode().IsShift() && mpView->IsInsGluePointMode() )
            {
                if(bBeginInsertPoint)
                {
                    mpWindow->SetPointerPosPixel(oldPoint);
                    //simulate mouse button down action
                    MouseEvent aMevt(oldPoint, 1,
                                     MouseEventModifiers::SIMPLEMOVE | MouseEventModifiers::DRAGMOVE,
                                     MOUSE_LEFT, KEY_SHIFT);
                    // MT IA2: Not used?
                    // sal_uInt16 ubuttons = aMevt.GetButtons();
                    // sal_uInt16 uMod      = aMevt.GetModifier();
                    MouseButtonDown(aMevt);
                    mpWindow->CaptureMouse();
                    //simulate mouse button up action
                    MouseEvent rMEvt(oldPoint+Point(0,0), 1,
                                     MouseEventModifiers::SIMPLEMOVE | MouseEventModifiers::ENTERWINDOW,
                                     MOUSE_LEFT, KEY_SHIFT);
                    MouseButtonUp(rMEvt);
                    bReturn= true;
                }
            }
            break;
    }

    if(!bReturn)
        bReturn = FuDraw::KeyInput(rKEvt);

    return bReturn;
}

//Add Shift+UP/DOWN/LEFT/RIGHT key to move the position of insert point, and
//SHIFT+ENTER key to decide the position and draw the new insert point
void FuEditGluePoints::ForcePointer(const MouseEvent* pMEvt)
{
    if(bBeginInsertPoint && pMEvt)
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

bool FuEditGluePoints::Command(const CommandEvent& rCEvt)
{
    mpView->SetActualWin( mpWindow->GetOutDev() );
    return FuPoor::Command( rCEvt );
}

void FuEditGluePoints::Activate()
{
    mpView->SetGluePointEditMode();
    FuDraw::Activate();
}

void FuEditGluePoints::Deactivate()
{
    mpView->SetGluePointEditMode( false );
    FuDraw::Deactivate();
}

void FuEditGluePoints::ReceiveRequest(SfxRequest& rReq)
{
    switch (rReq.GetSlot())
    {
        case SID_GLUE_INSERT_POINT:
        {
            mpView->SetInsGluePointMode(!mpView->IsInsGluePointMode());
        }
        break;

        case SID_GLUE_ESCDIR_LEFT:
        {
            mpView->SetMarkedGluePointsEscDir( SdrEscapeDirection::LEFT,
                    !mpView->IsMarkedGluePointsEscDir( SdrEscapeDirection::LEFT ) );
        }
        break;

        case SID_GLUE_ESCDIR_RIGHT:
        {
            mpView->SetMarkedGluePointsEscDir( SdrEscapeDirection::RIGHT,
                    !mpView->IsMarkedGluePointsEscDir( SdrEscapeDirection::RIGHT ) );
        }
        break;

        case SID_GLUE_ESCDIR_TOP:
        {
            mpView->SetMarkedGluePointsEscDir( SdrEscapeDirection::TOP,
                    !mpView->IsMarkedGluePointsEscDir( SdrEscapeDirection::TOP ) );
        }
        break;

        case SID_GLUE_ESCDIR_BOTTOM:
        {
            mpView->SetMarkedGluePointsEscDir( SdrEscapeDirection::BOTTOM,
                    !mpView->IsMarkedGluePointsEscDir( SdrEscapeDirection::BOTTOM ) );
        }
        break;

        case SID_GLUE_PERCENT:
        {
            const SfxItemSet* pSet = rReq.GetArgs();
            const SfxPoolItem& rItem = pSet->Get(SID_GLUE_PERCENT);
            bool bPercent = static_cast<const SfxBoolItem&>(rItem).GetValue();
            mpView->SetMarkedGluePointsPercent(bPercent);
        }
        break;

        case SID_GLUE_HORZALIGN_CENTER:
        {
            mpView->SetMarkedGluePointsAlign(false, SdrAlign::HORZ_CENTER);
        }
        break;

        case SID_GLUE_HORZALIGN_LEFT:
        {
            mpView->SetMarkedGluePointsAlign(false, SdrAlign::HORZ_LEFT);
        }
        break;

        case SID_GLUE_HORZALIGN_RIGHT:
        {
            mpView->SetMarkedGluePointsAlign(false, SdrAlign::HORZ_RIGHT);
        }
        break;

        case SID_GLUE_VERTALIGN_CENTER:
        {
            mpView->SetMarkedGluePointsAlign(true, SdrAlign::VERT_CENTER);
        }
        break;

        case SID_GLUE_VERTALIGN_TOP:
        {
            mpView->SetMarkedGluePointsAlign(true, SdrAlign::VERT_TOP);
        }
        break;

        case SID_GLUE_VERTALIGN_BOTTOM:
        {
            mpView->SetMarkedGluePointsAlign(true, SdrAlign::VERT_BOTTOM);
        }
        break;
    }

    // at the end, call base class
    FuPoor::ReceiveRequest(rReq);
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
