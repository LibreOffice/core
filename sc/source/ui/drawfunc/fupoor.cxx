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

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuPoor::FuPoor(ScTabViewShell* pViewSh, Window* pWin, ScDrawView* pViewP,
               SdrModel* pDoc, SfxRequest& rReq) :
    pView(pViewP),
    pViewShell(pViewSh),
    pWindow(pWin),
    pDrDoc(pDoc),
    aSfxRequest(rReq),
    pDialog(NULL),
    bIsInDragMode(false),
    // remember MouseButton state
    mnCode(0)
{
    aScrollTimer.SetTimeoutHdl( LINK(this, FuPoor, ScrollHdl) );
    aScrollTimer.SetTimeout(SELENG_AUTOREPEAT_INTERVAL);

    aDragTimer.SetTimeoutHdl( LINK(this, FuPoor, DragTimerHdl) );
    aDragTimer.SetTimeout(SELENG_DRAGDROP_TIMEOUT);
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

FuPoor::~FuPoor()
{
    aDragTimer.Stop();
    aScrollTimer.Stop();

    if (pDialog)
        delete pDialog;
}

/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/

void FuPoor::Activate()
{
    if (pDialog)
    {
        pDialog->Show();
    }
}

/*************************************************************************
|*
|* Function deaktivieren
|*
\************************************************************************/

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

    ScViewData* pViewData = pViewShell->GetViewData();
    if ( pViewData->GetDocument()->IsNegativePage( pViewData->GetTabNo() ) )
        dx = -dx;

    ScSplitPos eWhich = pViewData->GetActivePart();
    if ( dx > 0 && pViewData->GetHSplitMode() == SC_SPLIT_FIX && WhichH(eWhich) == SC_SPLIT_LEFT )
    {
        pViewShell->ActivatePart( ( eWhich == SC_SPLIT_TOPLEFT ) ?
                        SC_SPLIT_TOPRIGHT : SC_SPLIT_BOTTOMRIGHT );
        dx = 0;
    }
    if ( dy > 0 && pViewData->GetVSplitMode() == SC_SPLIT_FIX && WhichV(eWhich) == SC_SPLIT_TOP )
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

IMPL_LINK_NOARG_INLINE_START(FuPoor, ScrollHdl)
{
    Point aPosPixel = pWindow->GetPointerPosPixel();

    // use remembered MouseButton state to create correct
    // MouseEvents for this artifical MouseMove.
    MouseMove(MouseEvent(aPosPixel, 1, 0, GetMouseButtonCode()));

    return 0;
}
IMPL_LINK_INLINE_END( FuPoor, ScrollHdl, Timer *, pTimer )

// moved from inline to *.cxx
sal_Bool FuPoor::MouseButtonUp(const MouseEvent& rMEvt)
{
    // remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    return false;
}

// moved from inline to *.cxx
sal_Bool FuPoor::MouseButtonDown(const MouseEvent& rMEvt)
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

sal_Bool FuPoor::KeyInput(const KeyEvent& /* rKEvt */)
{
    sal_Bool bReturn = false;

    return(bReturn);
}

sal_uInt8 FuPoor::Command(const CommandEvent& rCEvt)
{
    if ( COMMAND_STARTDRAG == rCEvt.GetCommand() )
    {
        //!!! sollte Joe eigentlich machen:
        // nur, wenn im Outliner was selektiert ist, darf
        // Command sal_True zurueckliefern:

        OutlinerView* pOutView = pView->GetTextEditOutlinerView();

        if ( pOutView )
            return pOutView->HasSelection() ? pView->Command(rCEvt,pWindow) : SC_CMD_NONE;
        else
            return pView->Command(rCEvt,pWindow);
    }
    else
        return pView->Command(rCEvt,pWindow);
}

/*************************************************************************
|*
|* Cut object to clipboard
|*
\************************************************************************/

void FuPoor::DoCut()
{
    if (pView)
    {
//!     pView->DoCut(pWindow);
    }
}

/*************************************************************************
|*
|* Copy object to clipboard
|*
\************************************************************************/

void FuPoor::DoCopy()
{
    if (pView)
    {
//!     pView->DoCopy(pWindow);
    }
}

/*************************************************************************
|*
|* Paste object from clipboard
|*
\************************************************************************/

void FuPoor::DoPaste()
{
    if (pView)
    {
//!     pView->DoPaste(pWindow);
    }
}

/*************************************************************************
|*
|* Timer-Handler fuer Drag&Drop
|*
\************************************************************************/

IMPL_LINK_NOARG(FuPoor, DragTimerHdl)
{
    //  ExecuteDrag (und das damit verbundene Reschedule) direkt aus dem Timer
    //  aufzurufen, bringt die VCL-Timer-Verwaltung durcheinander, wenn dabei
    //  (z.B. im Drop) wieder ein Timer gestartet wird (z.B. ComeBack-Timer der
    //  DrawView fuer Solid Handles / ModelHasChanged) - der neue Timer laeuft
    //  dann um die Dauer des Drag&Drop zu spaet ab.
    //  Darum Drag&Drop aus eigenem Event:

    Application::PostUserEvent( LINK( this, FuPoor, DragHdl ) );
    return 0;
}

IMPL_LINK_NOARG(FuPoor, DragHdl)
{
    SdrHdl* pHdl = pView->PickHandle(aMDPos);

    if ( pHdl==NULL && pView->IsMarkedHit(aMDPos) )
    {
        pWindow->ReleaseMouse();
        bIsInDragMode = sal_True;

//      pView->BeginDrag(pWindow, aMDPos);
        pViewShell->GetScDrawView()->BeginDrag(pWindow, aMDPos);
    }
    return 0;
}

//  Detektiv-Linie

sal_Bool FuPoor::IsDetectiveHit( const Point& rLogicPos )
{
    SdrPageView* pPV = pView->GetSdrPageView();
    if (!pPV)
        return false;

    sal_Bool bFound = false;
    SdrObjListIter aIter( *pPV->GetObjList(), IM_FLAT );
    SdrObject* pObject = aIter.Next();
    while (pObject && !bFound)
    {
        if (ScDetectiveFunc::IsNonAlienArrow( pObject ))
        {
            sal_uInt16 nHitLog = (sal_uInt16) pWindow->PixelToLogic(
                                Size(pView->GetHitTolerancePixel(),0)).Width();
            if(SdrObjectPrimitiveHit(*pObject, rLogicPos, nHitLog, *pPV, 0, false))
            {
                bFound = sal_True;
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
    return 0L;
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

// #i33136#
bool FuPoor::doConstructOrthogonal() const
{
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
