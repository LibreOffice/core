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

#include <editeng/outliner.hxx>
#include <svx/svditer.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svxids.hrc>

#include <fupoor.hxx>
#include <tabvwsh.hxx>
#include <drawview.hxx>
#include <detfunc.hxx>
#include <document.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/svapp.hxx>
#include <svx/sdrhittesthelper.hxx>

FuPoor::FuPoor(ScTabViewShell& rViewSh, vcl::Window* pWin, ScDrawView* pViewP,
               SdrModel* pDoc, const SfxRequest& rReq) :
    pView(pViewP),
    rViewShell(rViewSh),
    pWindow(pWin),
    pDrDoc(pDoc),
    aSfxRequest(rReq),
    aScrollTimer("sc FuPoor aScrollTimer"),
    aDragTimer("sc FuPoor aDragTimer"),
    bIsInDragMode(false),
    // remember MouseButton state
    mnCode(0)
{
    aScrollTimer.SetInvokeHandler( LINK(this, FuPoor, ScrollHdl) );
    aScrollTimer.SetTimeout(SELENG_AUTOREPEAT_INTERVAL);

    aDragTimer.SetInvokeHandler( LINK(this, FuPoor, DragTimerHdl) );
    aDragTimer.SetTimeout(SELENG_DRAGDROP_TIMEOUT);
}

FuPoor::~FuPoor()
{
    aDragTimer.Stop();
    aScrollTimer.Stop();
}

void FuPoor::Activate()
{
}

void FuPoor::Deactivate()
{
    aDragTimer.Stop();
    aScrollTimer.Stop();
}

// Scroll when reached the window border; is called from MouseMove
void FuPoor::ForceScroll(const Point& aPixPos)
{
    aScrollTimer.Stop();

    Size aSize = pWindow->GetSizePixel();
    SCCOL dx = 0;
    SCROW dy = 0;

    if ( aPixPos.X() <= 0              ) dx = -1;
    if ( aPixPos.X() >= aSize.Width()  ) dx =  1;
    if ( aPixPos.Y() <= 0              ) dy = -1;
    if ( aPixPos.Y() >= aSize.Height() ) dy =  1;

    ScViewData& rViewData = rViewShell.GetViewData();
    if ( rViewData.GetDocument().IsNegativePage( rViewData.GetTabNo() ) )
        dx = -dx;

    ScSplitPos eWhich = rViewData.GetActivePart();
    if ( dx > 0 && rViewData.GetHSplitMode() == SC_SPLIT_FIX && WhichH(eWhich) == SC_SPLIT_LEFT )
    {
        rViewShell.ActivatePart( ( eWhich == SC_SPLIT_TOPLEFT ) ?
                        SC_SPLIT_TOPRIGHT : SC_SPLIT_BOTTOMRIGHT );
        dx = 0;
    }
    if ( dy > 0 && rViewData.GetVSplitMode() == SC_SPLIT_FIX && WhichV(eWhich) == SC_SPLIT_TOP )
    {
        rViewShell.ActivatePart( ( eWhich == SC_SPLIT_TOPLEFT ) ?
                        SC_SPLIT_BOTTOMLEFT : SC_SPLIT_BOTTOMRIGHT );
        dy = 0;
    }

    if ( dx != 0 || dy != 0 )
    {
        rViewShell.ScrollLines(2*dx, 4*dy);
        aScrollTimer.Start();
    }
}

// Timer handler for window scrolling
IMPL_LINK_NOARG(FuPoor, ScrollHdl, Timer *, void)
{
    Point aPosPixel = pWindow->GetPointerPosPixel();

    // use remembered MouseButton state to create correct
    // MouseEvents for this artificial MouseMove.
    MouseMove(MouseEvent(aPosPixel, 1, MouseEventModifiers::NONE, GetMouseButtonCode()));
}

bool FuPoor::MouseButtonUp(const MouseEvent& rMEvt)
{
    // remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    return false;
}

bool FuPoor::MouseButtonDown(const MouseEvent& rMEvt)
{
    // remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    return false;
}

// If we handle a KeyEvent, then the return value is sal_True else FALSE.
bool FuPoor::KeyInput(const KeyEvent& /* rKEvt */)
{
    return false;
}

sal_uInt8 FuPoor::Command(const CommandEvent& rCEvt)
{
    if ( CommandEventId::StartDrag == rCEvt.GetCommand() )
    {
        // Only if a selection is in Outliner, then Command is allowed
        // to return sal_True

        OutlinerView* pOutView = pView->GetTextEditOutlinerView();

        if ( pOutView )
            return pOutView->HasSelection() ? (pView->Command(rCEvt,pWindow) ? 1 : 0) : SC_CMD_NONE;
        else
            return pView->Command(rCEvt,pWindow) ? 1 : 0;
    }
    else
        return pView->Command(rCEvt,pWindow) ? 1 : 0;
}

// Timer-Handler for Drag&Drop
IMPL_LINK_NOARG(FuPoor, DragTimerHdl, Timer *, void)
{
    //  Calling ExecuteDrag (and that associated reschedule) directly from
    //  the Timer, will confuse the VCL-Timer-Management, if (e.g during Drop)
    //  a new timer is started (e.g ComeBack-Timer of DrawView for
    //  Solid Handles / ModelHasChanged) - the new timer will end with a delay
    //  of the duration of the Drag&Drop.
    //  Therefore Drag&Drop from own event:

    Application::PostUserEvent( LINK( this, FuPoor, DragHdl ) );
}

IMPL_LINK_NOARG(FuPoor, DragHdl, void*, void)
{
    SdrHdl* pHdl = pView->PickHandle(aMDPos);

    if ( pHdl==nullptr && pView->IsMarkedHit(aMDPos) )
    {
        pWindow->ReleaseMouse();
        bIsInDragMode = true;
        rViewShell.GetScDrawView()->BeginDrag(pWindow, aMDPos);
    }
}

//  Detective-line
bool FuPoor::IsDetectiveHit( const Point& rLogicPos )
{
    SdrPageView* pPV = pView->GetSdrPageView();
    if (!pPV)
        return false;

    bool bFound = false;
    SdrObjListIter aIter( pPV->GetObjList(), SdrIterMode::Flat );
    SdrObject* pObject = aIter.Next();
    while (pObject && !bFound)
    {
        if (ScDetectiveFunc::IsNonAlienArrow( pObject ))
        {
            double fHitLog = pWindow->PixelToLogic(Size(pView->GetHitTolerancePixel(),0)).Width();
            if(SdrObjectPrimitiveHit(*pObject, rLogicPos, {fHitLog, fHitLog}, *pPV, nullptr, false))
            {
                bFound = true;
            }
        }

        pObject = aIter.Next();
    }
    return bFound;
}

void FuPoor::StopDragTimer()
{
    if (aDragTimer.IsActive() )
        aDragTimer.Stop();
}

// Create default drawing objects via keyboard
rtl::Reference<SdrObject> FuPoor::CreateDefaultObject(const sal_uInt16 /* nID */, const tools::Rectangle& /* rRectangle */)
{
    // empty base implementation
    return nullptr;
}

void FuPoor::ImpForceQuadratic(tools::Rectangle& rRect)
{
    if(rRect.GetWidth() > rRect.GetHeight())
    {
        rRect = tools::Rectangle(
            Point(rRect.Left() + ((rRect.GetWidth() - rRect.GetHeight()) / 2), rRect.Top()),
            Size(rRect.GetHeight(), rRect.GetHeight()));
    }
    else
    {
        rRect = tools::Rectangle(
            Point(rRect.Left(), rRect.Top() + ((rRect.GetHeight() - rRect.GetWidth()) / 2)),
            Size(rRect.GetWidth(), rRect.GetWidth()));
    }
}

// #i33136# fdo#88339
bool FuPoor::doConstructOrthogonal() const
{
    // Detect whether we're moving an object or resizing.
    if (pView->IsDragObj())
    {
        const SdrHdl* pHdl = pView->GetDragStat().GetHdl();
        if (!pHdl || (!pHdl->IsCornerHdl() && !pHdl->IsVertexHdl()))
        {
            return false;
        }
    }

    // Detect image/media and resize proportionally, but don't constrain movement by default
    const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
    if (rMarkList.GetMarkCount() != 0)
    {
        if (rMarkList.GetMarkCount() == 1)
        {
            SdrObjKind aObjIdentifier = rMarkList.GetMark(0)->GetMarkedSdrObj()->GetObjIdentifier();
            bool bIsMediaSelected = aObjIdentifier == SdrObjKind::Graphic ||
                                    aObjIdentifier == SdrObjKind::Media ||
                                    aObjIdentifier == SdrObjKind::OLE2;

            SdrHdl* pHdl = pView->PickHandle(aMDPos);
            // Resize proportionally when media is selected and the user drags on a corner
            if (pHdl)
                return bIsMediaSelected && pHdl->IsCornerHdl();
            return bIsMediaSelected;
        }
    }
    else if (aSfxRequest.GetSlot() == SID_DRAW_XPOLYGON
          || aSfxRequest.GetSlot() == SID_DRAW_XPOLYGON_NOFILL
          || aSfxRequest.GetSlot() == SID_DRAW_XLINE)
        return true;

    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
