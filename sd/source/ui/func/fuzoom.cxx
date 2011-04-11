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


#include "fuzoom.hxx"

#include <svx/svxids.hrc>
#include <sfx2/bindings.hxx>
#include <sfx2/viewfrm.hxx>
#include "app.hrc"
#include <svx/svdpagv.hxx>

#include "FrameView.hxx"
#include "ViewShell.hxx"
#include "View.hxx"
#include "Window.hxx"
#include "drawdoc.hxx"
#include "zoomlist.hxx"

namespace sd {

sal_uInt16 SidArrayZoom[] = {
                    SID_ATTR_ZOOM,
                    SID_ZOOM_OUT,
                    SID_ZOOM_IN,
                    0 };

TYPEINIT1( FuZoom, FuPoor );

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuZoom::FuZoom(
    ViewShell* pViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument* pDoc,
    SfxRequest& rReq)
    : FuPoor(pViewSh, pWin, pView, pDoc, rReq),
      bVisible(sal_False),
      bStartDrag(sal_False)
{
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

FuZoom::~FuZoom()
{
    if (bVisible)
    {
        // Hide ZoomRect
        mpViewShell->DrawMarkRect(aZoomRect);

        bVisible = sal_False;
        bStartDrag = sal_False;
    }
}

FunctionReference FuZoom::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    FunctionReference xFunc( new FuZoom( pViewSh, pWin, pView, pDoc, rReq ) );
    return xFunc;
}

/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/

sal_Bool FuZoom::MouseButtonDown(const MouseEvent& rMEvt)
{
    // remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    mpWindow->CaptureMouse();
    bStartDrag = sal_True;

    aBeginPosPix = rMEvt.GetPosPixel();
    aBeginPos = mpWindow->PixelToLogic(aBeginPosPix);

    return sal_True;
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/

sal_Bool FuZoom::MouseMove(const MouseEvent& rMEvt)
{
    if (bStartDrag)
    {
        if (bVisible)
        {
            mpViewShell->DrawMarkRect(aZoomRect);
        }

        Point aPosPix = rMEvt.GetPosPixel();
        ForceScroll(aPosPix);

        aEndPos = mpWindow->PixelToLogic(aPosPix);
        aBeginPos = mpWindow->PixelToLogic(aBeginPosPix);

        if (nSlotId == SID_ZOOM_PANNING)
        {
            // Panning

            Point aScroll = aBeginPos - aEndPos;

            // #i2237#
            // removed old stuff here which still forced zoom to be
            // %BRUSH_SIZE which is outdated now

            if (aScroll.X() != 0 || aScroll.Y() != 0)
            {
                Size aWorkSize = mpView->GetWorkArea().GetSize();
                Size aPageSize = mpView->GetSdrPageView()->GetPage()->GetSize();
                aScroll.X() /= aWorkSize.Width()  / aPageSize.Width();
                aScroll.Y() /= aWorkSize.Height() / aPageSize.Height();
                mpViewShell->Scroll(aScroll.X(), aScroll.Y());
                aBeginPosPix = aPosPix;
            }
        }
        else
        {
            Rectangle aRect(aBeginPos, aEndPos);
            aZoomRect = aRect;
            aZoomRect.Justify();
            mpViewShell->DrawMarkRect(aZoomRect);
        }

        bVisible = sal_True;
    }

    return bStartDrag;
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

sal_Bool FuZoom::MouseButtonUp(const MouseEvent& rMEvt)
{
    // remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    if (bVisible)
    {
        // Hide ZoomRect
        mpViewShell->DrawMarkRect(aZoomRect);
        bVisible = sal_False;
    }

    Point aPosPix = rMEvt.GetPosPixel();

    if(SID_ZOOM_PANNING != nSlotId)
    {
        // Zoom
        Size aZoomSizePixel = mpWindow->LogicToPixel(aZoomRect).GetSize();
        sal_uLong nTol = DRGPIX + DRGPIX;

        if ( aZoomSizePixel.Width() < (long) nTol && aZoomSizePixel.Height() < (long) nTol )
        {
            // Klick auf der Stelle: Zoomfaktor verdoppeln
            Point aPos = mpWindow->PixelToLogic(aPosPix);
            Size aSize = mpWindow->PixelToLogic(mpWindow->GetOutputSizePixel());
            aSize.Width() /= 2;
            aSize.Height() /= 2;
            aPos.X() -= aSize.Width() / 2;
            aPos.Y() -= aSize.Height() / 2;
            aZoomRect.SetPos(aPos);
            aZoomRect.SetSize(aSize);
        }

        mpViewShell->SetZoomRect(aZoomRect);
    }

    Rectangle aVisAreaWin = mpWindow->PixelToLogic(Rectangle(Point(0,0),
                                           mpWindow->GetOutputSizePixel()));
    mpViewShell->GetZoomList()->InsertZoomRect(aVisAreaWin);

    bStartDrag = sal_False;
    mpWindow->ReleaseMouse();
    mpViewShell->Cancel();

    return sal_True;
}

/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/

void FuZoom::Activate()
{
    aPtr = mpWindow->GetPointer();

    if (nSlotId == SID_ZOOM_PANNING)
    {
        mpWindow->SetPointer(Pointer(POINTER_HAND));
    }
    else
    {
        mpWindow->SetPointer(Pointer(POINTER_MAGNIFY));
    }
}

/*************************************************************************
|*
|* Function deaktivieren
|*
\************************************************************************/

void FuZoom::Deactivate()
{
    mpWindow->SetPointer( aPtr );
    mpViewShell->GetViewFrame()->GetBindings().Invalidate( SidArrayZoom );
}
} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
