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
            // click at place: double zoom factor
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

void FuZoom::Deactivate()
{
    mpWindow->SetPointer( aPtr );
    mpViewShell->GetViewFrame()->GetBindings().Invalidate( SidArrayZoom );
}
} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
