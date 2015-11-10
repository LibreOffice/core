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

#include "fupoor.hxx"
#include "tabvwsh.hxx"
#include "drawview.hxx"
#include "detfunc.hxx"
#include "document.hxx"
#include <vcl/svapp.hxx>
#include <svx/sdrhittesthelper.hxx>

FuPoor::FuPoor(ScTabViewShell* pViewSh, vcl::Window* pWin, ScDrawView* pViewP,
               SdrModel* pDoc, SfxRequest& rReq) :
    pView(pViewP),
    pViewShell(pViewSh),
    pWindow(pWin),
    pDrDoc(pDoc),
    aSfxRequest(rReq),
    pDialog(nullptr),
    bIsInDragMode(false),
    // remember MouseButton state
    mnCode(0)
{
    aScrollTimer.SetTimeoutHdl( LINK(this, FuPoor, ScrollHdl) );
    aScrollTimer.SetTimeout(SELENG_AUTOREPEAT_INTERVAL);

    aDragTimer.SetTimeoutHdl( LINK(this, FuPoor, DragTimerHdl) );
    aDragTimer.SetTimeout(SELENG_DRAGDROP_TIMEOUT);
}

FuPoor::~FuPoor()
{
    aDragTimer.Stop();
    aScrollTimer.Stop();
    pDialog.disposeAndClear();
}

void FuPoor::Activate()
{
    if (pDialog)
    {
        pDialog->Show();
    }
}

void FuPoor::Deactivate()
{
    aDragTimer.Stop();
    aScrollTimer.Stop();

    if (pDialog)
    {
        pDialog->Hide();
    }
}

/*************************************************************************
|*
|* Scrollen bei Erreichen des Fensterrandes; wird von
|* MouseMove aufgerufen
|*
\************************************************************************/

void FuPoor::ForceScroll(const Point& aPixPos)
{
    aScrollTimer.Stop();

    Size aSize = pWindow->GetSizePixel();
    SCsCOL dx = 0;
    SCsROW dy = 0;

    if ( aPixPos.X() <= 0              ) dx = -1;
    if ( aPixPos.X() >= aSize.Width()  ) dx =  1;
    if ( aPixPos.Y() <= 0              ) dy = -1;
    if ( aPixPos.Y() >= aSize.Height() ) dy =  1;

    ScViewData& rViewData = pViewShell->GetViewData();
    if ( rViewData.GetDocument()->IsNegativePage( rViewData.GetTabNo() ) )
        dx = -dx;

    ScSplitPos eWhich = rViewData.GetActivePart();
    if ( dx > 0 && rViewData.GetHSplitMode() == SC_SPLIT_FIX && WhichH(eWhich) == SC_SPLIT_LEFT )
    {
        pViewShell->ActivatePart( ( eWhich == SC_SPLIT_TOPLEFT ) ?
                        SC_SPLIT_TOPRIGHT : SC_SPLIT_BOTTOMRIGHT );
        dx = 0;
    }
    if ( dy > 0 && rViewData.GetVSplitMode() == SC_SPLIT_FIX && WhichV(eWhich) == SC_SPLIT_TOP )
    {
        pViewShell->ActivatePart( ( eWhich == SC_SPLIT_TOPLEFT ) ?
                        SC_SPLIT_BOTTOMLEFT : SC_SPLIT_BOTTOMRIGHT );
        dy = 0;
    }

    if ( dx != 0 || dy != 0 )
    {
        pViewShell->ScrollLines(2*dx, 4*dy);
        aScrollTimer.Start();
    }
}

/*************************************************************************
|*
|* Timer-Handler fuer Fensterscrolling
|*
\************************************************************************/

IMPL_LINK_NOARG_TYPED(FuPoor, ScrollHdl, Timer *, void)
{
    Point aPosPixel = pWindow->GetPointerPosPixel();

    // use remembered MouseButton state to create correct
    // MouseEvents for this artificial MouseMove.
    MouseMove(MouseEvent(aPosPixel, 1, MouseEventModifiers::NONE, GetMouseButtonCode()));
}

// moved from inline to *.cxx
bool FuPoor::MouseButtonUp(const MouseEvent& rMEvt)
{
    // remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    return false;
}

// moved from inline to *.cxx
bool FuPoor::MouseButtonDown(const MouseEvent& rMEvt)
{
    // remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    return false;
}

/*************************************************************************
|*
|* String in Applikations-Statuszeile ausgeben
|*
\************************************************************************/

//  WriteStatus gibt's nicht mehr

/*************************************************************************
|*
|* Tastaturereignisse bearbeiten
|*
|* Wird ein KeyEvent bearbeitet, so ist der Return-Wert sal_True, andernfalls
|* FALSE.
|*
\************************************************************************/

bool FuPoor::KeyInput(const KeyEvent& /* rKEvt */)
{
    return false;
}

sal_uInt8 FuPoor::Command(const CommandEvent& rCEvt)
{
    if ( CommandEventId::StartDrag == rCEvt.GetCommand() )
    {
        //!!! sollte Joe eigentlich machen:
        // nur, wenn im Outliner was selektiert ist, darf
        // Command sal_True zurueckliefern:

        OutlinerView* pOutView = pView->GetTextEditOutlinerView();

        if ( pOutView )
            return pOutView->HasSelection() ? (pView->Command(rCEvt,pWindow) ? 1 : 0) : SC_CMD_NONE;
        else
            return pView->Command(rCEvt,pWindow) ? 1 : 0;
    }
    else
        return pView->Command(rCEvt,pWindow) ? 1 : 0;
}

/*************************************************************************
|*
|* Timer-Handler fuer Drag&Drop
|*
\************************************************************************/
IMPL_LINK_NOARG_TYPED(FuPoor, DragTimerHdl, Timer *, void)
{
    //  ExecuteDrag (und das damit verbundene Reschedule) direkt aus dem Timer
    //  aufzurufen, bringt die VCL-Timer-Verwaltung durcheinander, wenn dabei
    //  (z.B. im Drop) wieder ein Timer gestartet wird (z.B. ComeBack-Timer der
    //  DrawView fuer Solid Handles / ModelHasChanged) - der neue Timer laeuft
    //  dann um die Dauer des Drag&Drop zu spaet ab.
    //  Darum Drag&Drop aus eigenem Event:

    Application::PostUserEvent( LINK( this, FuPoor, DragHdl ) );
}

IMPL_LINK_NOARG_TYPED(FuPoor, DragHdl, void*, void)
{
    SdrHdl* pHdl = pView->PickHandle(aMDPos);

    if ( pHdl==nullptr && pView->IsMarkedHit(aMDPos) )
    {
        pWindow->ReleaseMouse();
        bIsInDragMode = true;
        pViewShell->GetScDrawView()->BeginDrag(pWindow, aMDPos);
    }
}

//  Detektiv-Linie

bool FuPoor::IsDetectiveHit( const Point& rLogicPos )
{
    SdrPageView* pPV = pView->GetSdrPageView();
    if (!pPV)
        return false;

    bool bFound = false;
    SdrObjListIter aIter( *pPV->GetObjList(), IM_FLAT );
    SdrObject* pObject = aIter.Next();
    while (pObject && !bFound)
    {
        if (ScDetectiveFunc::IsNonAlienArrow( pObject ))
        {
            sal_uInt16 nHitLog = (sal_uInt16) pWindow->PixelToLogic(
                                Size(pView->GetHitTolerancePixel(),0)).Width();
            if(SdrObjectPrimitiveHit(*pObject, rLogicPos, nHitLog, *pPV, nullptr, false))
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

/*************************************************************************
|*
|* Create default drawing objects via keyboard
|*
\************************************************************************/

SdrObject* FuPoor::CreateDefaultObject(const sal_uInt16 /* nID */, const Rectangle& /* rRectangle */)
{
    // empty base implementation
    return nullptr;
}

void FuPoor::ImpForceQuadratic(Rectangle& rRect)
{
    if(rRect.GetWidth() > rRect.GetHeight())
    {
        rRect = Rectangle(
            Point(rRect.Left() + ((rRect.GetWidth() - rRect.GetHeight()) / 2), rRect.Top()),
            Size(rRect.GetHeight(), rRect.GetHeight()));
    }
    else
    {
        rRect = Rectangle(
            Point(rRect.Left(), rRect.Top() + ((rRect.GetHeight() - rRect.GetWidth()) / 2)),
            Size(rRect.GetWidth(), rRect.GetWidth()));
    }
}

// #i33136# fdo#88339
bool FuPoor::doConstructOrthogonal() const
{
    // Detect whether we're moving an object or resizing.
    bool bIsMoveMode = false;
    if (pView->IsDragObj())
    {
        const SdrHdl* pHdl = pView->GetDragStat().GetHdl();
        if (!pHdl || (!pHdl->IsCornerHdl() && !pHdl->IsVertexHdl()))
        {
            bIsMoveMode = true;
        }
    }

    // Detect image and resize proportionally, but don't constrain movement by default
    if (!bIsMoveMode && pView->AreObjectsMarked())
    {
        const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
        if (rMarkList.GetMarkCount() == 1)
        {
            if (rMarkList.GetMark(0)->GetMarkedSdrObj()->GetObjIdentifier() == OBJ_GRAF)
            {
                return true;
            }
        }
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
