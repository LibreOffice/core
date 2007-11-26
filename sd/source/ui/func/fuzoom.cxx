/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fuzoom.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-26 14:35:33 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"


#include "fuzoom.hxx"

#include <svx/svxids.hrc>
#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif
#include "app.hrc"

#ifndef _SVDPAGV_HXX //autogen
#include <svx/svdpagv.hxx>
#endif

#ifndef SD_FRAMW_VIEW_HXX
#include "FrameView.hxx"
#endif
#ifndef SD_VIEW_SHELL_HXX
#include "ViewShell.hxx"
#endif
#ifndef SD_VIEW_HXX
#include "View.hxx"
#endif
#ifndef SD_WINDOW_SHELL_HXX
#include "Window.hxx"
#endif
#include "drawdoc.hxx"
#include "zoomlist.hxx"

namespace sd {

USHORT SidArrayZoom[] = {
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
      bVisible(FALSE),
      bStartDrag(FALSE)
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

        bVisible = FALSE;
        bStartDrag = FALSE;
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

BOOL FuZoom::MouseButtonDown(const MouseEvent& rMEvt)
{
    // #95491# remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    mpWindow->CaptureMouse();
    bStartDrag = TRUE;

    aBeginPosPix = rMEvt.GetPosPixel();
    aBeginPos = mpWindow->PixelToLogic(aBeginPosPix);

    return TRUE;
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/

BOOL FuZoom::MouseMove(const MouseEvent& rMEvt)
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

        bVisible = TRUE;
    }

    return bStartDrag;
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

BOOL FuZoom::MouseButtonUp(const MouseEvent& rMEvt)
{
    // #95491# remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    if (bVisible)
    {
        // Hide ZoomRect
        mpViewShell->DrawMarkRect(aZoomRect);
        bVisible = FALSE;
    }

    Point aPosPix = rMEvt.GetPosPixel();

    if(SID_ZOOM_PANNING != nSlotId)
    {
        // Zoom
        Size aZoomSizePixel = mpWindow->LogicToPixel(aZoomRect).GetSize();
        ULONG nTol = DRGPIX + DRGPIX;

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

    bStartDrag = FALSE;
    mpWindow->ReleaseMouse();
    mpViewShell->Cancel();

    return TRUE;
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
