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

#include <fuzoom.hxx>

#include <svx/svxids.hrc>
#include <sfx2/bindings.hxx>
#include <sfx2/viewfrm.hxx>
#include <app.hrc>
#include <svx/svdpagv.hxx>
#include <vcl/ptrstyle.hxx>

#include <FrameView.hxx>
#include <ViewShell.hxx>
#include <View.hxx>
#include <Window.hxx>
#include <drawdoc.hxx>
#include <zoomlist.hxx>

namespace sd {

const sal_uInt16 SidArrayZoom[] = {
                    SID_ATTR_ZOOM,
                    SID_ZOOM_OUT,
                    SID_ZOOM_IN,
                    0 };


FuZoom::FuZoom(
    ViewShell* pViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument* pDoc,
    SfxRequest& rReq)
    : FuPoor(pViewSh, pWin, pView, pDoc, rReq),
      bVisible(false),
      bStartDrag(false),
      aPtr(PointerStyle::Arrow)
{
}

FuZoom::~FuZoom()
{
    if (bVisible)
    {
        // Hide ZoomRect
        mpViewShell->DrawMarkRect(aZoomRect);

        bVisible = false;
        bStartDrag = false;
    }
}

rtl::Reference<FuPoor> FuZoom::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    rtl::Reference<FuPoor> xFunc( new FuZoom( pViewSh, pWin, pView, pDoc, rReq ) );
    return xFunc;
}

bool FuZoom::MouseButtonDown(const MouseEvent& rMEvt)
{
    // remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    mpWindow->CaptureMouse();
    bStartDrag = true;

    aBeginPosPix = rMEvt.GetPosPixel();
    aBeginPos = mpWindow->PixelToLogic(aBeginPosPix);
    aZoomRect.SetSize( Size( 0, 0 ) );
    aZoomRect.SetPos( aBeginPos );

    return true;
}

bool FuZoom::MouseMove(const MouseEvent& rMEvt)
{
    if (rMEvt.IsShift())
        mpWindow->SetPointer(PointerStyle::Hand);
    else if (nSlotId != SID_ZOOM_PANNING)
        mpWindow->SetPointer(PointerStyle::Magnify);

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

        if (nSlotId == SID_ZOOM_PANNING || (rMEvt.IsShift() && !bVisible) )
        {
            // Panning

            Point aScroll = aBeginPos - aEndPos;

            if (aScroll.X() != 0 || aScroll.Y() != 0)
            {
                Size aWorkSize = mpView->GetWorkArea().GetSize();
                Size aPageSize = mpView->GetSdrPageView()->GetPage()->GetSize();
                aScroll.setX( aScroll.X() / ( aWorkSize.Width()  / aPageSize.Width()) );
                aScroll.setY( aScroll.Y() / ( aWorkSize.Height() / aPageSize.Height()) );
                mpViewShell->Scroll(aScroll.X(), aScroll.Y());
                aBeginPosPix = aPosPix;
            }
        }
        else
        {
            ::tools::Rectangle aRect(aBeginPos, aEndPos);
            aZoomRect = aRect;
            aZoomRect.Justify();
            mpViewShell->DrawMarkRect(aZoomRect);
            bVisible = true;
        }
    }

    return bStartDrag;
}

bool FuZoom::MouseButtonUp(const MouseEvent& rMEvt)
{
    // remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    if (bVisible)
    {
        // Hide ZoomRect
        mpViewShell->DrawMarkRect(aZoomRect);
        bVisible = false;
    }

    Point aPosPix = rMEvt.GetPosPixel();

    if(SID_ZOOM_PANNING != nSlotId && !rMEvt.IsShift())
    {
        // Zoom
        Size aZoomSizePixel = mpWindow->LogicToPixel(aZoomRect).GetSize();
        sal_uLong nTol = DRGPIX + DRGPIX;

        if ( ( aZoomSizePixel.Width() < static_cast<long>(nTol) && aZoomSizePixel.Height() < static_cast<long>(nTol) ) || rMEvt.IsMod1() )
        {
            // click at place: double zoom factor
            Point aPos = mpWindow->PixelToLogic(aPosPix);
            Size aSize = mpWindow->PixelToLogic(mpWindow->GetOutputSizePixel());
            if ( rMEvt.IsMod1() )
            {
                aSize.setWidth( aSize.Width() * 2 );
                aSize.setHeight( aSize.Height() * 2 );
            }
            else
            {
                aSize.setWidth( aSize.Width() / 2 );
                aSize.setHeight( aSize.Height() / 2 );
            }
            aPos.AdjustX( -(aSize.Width() / 2) );
            aPos.AdjustY( -(aSize.Height() / 2) );
            aZoomRect.SetPos(aPos);
            aZoomRect.SetSize(aSize);
        }

        mpViewShell->SetZoomRect(aZoomRect);
        mpViewShell->GetViewFrame()->GetBindings().Invalidate( SidArrayZoom );
    }

    ::tools::Rectangle aVisAreaWin = mpWindow->PixelToLogic(::tools::Rectangle(Point(0,0),
                                           mpWindow->GetOutputSizePixel()));
    mpViewShell->GetZoomList()->InsertZoomRect(aVisAreaWin);

    bStartDrag = false;
    mpWindow->ReleaseMouse();

    return true;
}

void FuZoom::Activate()
{
    aPtr = mpWindow->GetPointer();

    if (nSlotId == SID_ZOOM_PANNING)
    {
        mpWindow->SetPointer(PointerStyle::Hand);
    }
    else
    {
        mpWindow->SetPointer(PointerStyle::Magnify);
    }
}

void FuZoom::Deactivate()
{
    mpWindow->SetPointer( aPtr );
    mpViewShell->GetViewFrame()->GetBindings().Invalidate( SidArrayZoom );
}
} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
