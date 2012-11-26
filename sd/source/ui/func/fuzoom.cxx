/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"


#include "fuzoom.hxx"

#include <svx/svxids.hrc>
#include <sfx2/bindings.hxx>
#include <sfx2/viewfrm.hxx>
#include "app.hrc"
#include <svx/svdpagv.hxx>

#ifndef SD_FRAMW_VIEW_HXX
#include "FrameView.hxx"
#endif
#include "ViewShell.hxx"
#include "View.hxx"
#ifndef SD_WINDOW_SHELL_HXX
#include "Window.hxx"
#endif
#include "drawdoc.hxx"
#include "zoomlist.hxx"

namespace sd {

sal_uInt16 SidArrayZoom[] = {
                    SID_ATTR_ZOOM,
                    SID_ZOOM_OUT,
                    SID_ZOOM_IN,
                    0 };

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
    maBeginPosPixel(),
    maBeginPos(),
    maEndPos(),
    maZoomRange(),
    maPtr(),
    mbVisible(false),
    mbStartDrag(false)
{
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

FuZoom::~FuZoom()
{
    if (mbVisible)
    {
        // Hide ZoomRect
        mpViewShell->DrawMarkRange(maZoomRange);

        mbVisible = false;
        mbStartDrag = false;
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

bool FuZoom::MouseButtonDown(const MouseEvent& rMEvt)
{
    // #95491# remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    mpWindow->CaptureMouse();
    mbStartDrag = true;

    maBeginPosPixel = basegfx::B2DPoint(rMEvt.GetPosPixel().X(), rMEvt.GetPosPixel().Y());
    maBeginPos = mpWindow->GetInverseViewTransformation() * maBeginPosPixel;

    return true;
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/

bool FuZoom::MouseMove(const MouseEvent& rMEvt)
{
    if (mbStartDrag)
    {
        if (mbVisible)
        {
            mpViewShell->DrawMarkRange(maZoomRange);
        }

        const basegfx::B2DPoint aPosPix(rMEvt.GetPosPixel().X(), rMEvt.GetPosPixel().Y());

        ForceScroll(aPosPix);
        maEndPos = mpWindow->GetInverseViewTransformation() * aPosPix;
        maBeginPos = mpWindow->GetInverseViewTransformation() * maBeginPosPixel;

        if (nSlotId == SID_ZOOM_PANNING)
        {
            // Panning
            basegfx::B2DVector aScroll(maBeginPos - maEndPos);

            if(!aScroll.equalZero() && mpView->GetSdrPageView())
            {
                const basegfx::B2DVector aWorkRange(mpView->GetWorkArea().getRange());
                const SdrPage& rPage = mpView->GetSdrPageView()->getSdrPageFromSdrPageView();
                const basegfx::B2DVector aPageRange(rPage.GetPageScale());
                const basegfx::B2DVector aFactor(
                    aPageRange.getX() / (basegfx::fTools::equalZero(aWorkRange.getX()) ? 1.0 : aWorkRange.getX()),
                    aPageRange.getY() / (basegfx::fTools::equalZero(aWorkRange.getY()) ? 1.0 : aWorkRange.getY()));

                aScroll *= aFactor;
                mpViewShell->Scroll(basegfx::fround(aScroll.getX()), basegfx::fround(aScroll.getY()));
                maBeginPosPixel = aPosPix;
            }
        }
        else
        {
            maZoomRange = basegfx::B2DRange(maBeginPos, maEndPos);
            mpViewShell->DrawMarkRange(maZoomRange);
        }

        mbVisible = true;
    }

    return mbStartDrag;
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

bool FuZoom::MouseButtonUp(const MouseEvent& rMEvt)
{
    // #95491# remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    if (mbVisible)
    {
        // Hide ZoomRect
        mpViewShell->DrawMarkRange(maZoomRange);
        mbVisible = false;
    }

    const basegfx::B2DPoint aPosPix(rMEvt.GetPosPixel().X(), rMEvt.GetPosPixel().Y());

    if(SID_ZOOM_PANNING != nSlotId)
    {
        // Zoom
        const basegfx::B2DVector aZoomSizePixel(mpWindow->GetInverseViewTransformation() * maZoomRange.getRange());
        const double fTol(DRGPIX + DRGPIX);

        if ( aZoomSizePixel.getX() < fTol && aZoomSizePixel.getY() < fTol )
        {
            // Klick auf der Stelle: Zoomfaktor verdoppeln
            const basegfx::B2DPoint aPos(mpWindow->GetInverseViewTransformation() * aPosPix);
            const basegfx::B2DVector aScale(mpWindow->GetLogicVector() * 0.25);

            maZoomRange = basegfx::B2DRange(aPos - aScale, aScale * 2.0);
        }

        mpViewShell->SetZoomRange(maZoomRange);
    }

    mpViewShell->GetZoomList()->InsertZoomRange(mpWindow->GetLogicRange());

    mbStartDrag = false;
    mpWindow->ReleaseMouse();
    mpViewShell->Cancel();

    return true;
}

/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/

void FuZoom::Activate()
{
    maPtr = mpWindow->GetPointer();

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
    mpWindow->SetPointer( maPtr );
    mpViewShell->GetViewFrame()->GetBindings().Invalidate( SidArrayZoom );
}
} // end of namespace sd
