/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fuzoom.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 14:31:59 $
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
      bStartDrag(FALSE),
      bLineDraft(FALSE),
      bFillDraft(FALSE),
      bTextDraft(FALSE),
      bGrafDraft(FALSE)
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
        pViewShell->DrawMarkRect(aZoomRect);

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

    pWindow->CaptureMouse();
    bStartDrag = TRUE;

    aBeginPosPix = rMEvt.GetPosPixel();
    aBeginPos = pWindow->PixelToLogic(aBeginPosPix);

    if (nSlotId == SID_ZOOM_PANNING)
    {
        // Ersatzdarstellung merken
        FrameView* pFrameView = pViewShell->GetFrameView();
        bLineDraft = pFrameView->IsLineDraft();
        bFillDraft = pFrameView->IsFillDraft();
        bTextDraft = pFrameView->IsTextDraft();
        bGrafDraft = pFrameView->IsGrafDraft();

        // Ersatzdarstellungen einschalten
        pView->SetLineDraft(TRUE);
        pView->SetFillDraft(TRUE);
        pView->SetTextDraft(TRUE);
        pView->SetGrafDraft(TRUE);
    }

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
            pViewShell->DrawMarkRect(aZoomRect);
        }

        Point aPosPix = rMEvt.GetPosPixel();
        ForceScroll(aPosPix);

        aEndPos = pWindow->PixelToLogic(aPosPix);
        aBeginPos = pWindow->PixelToLogic(aBeginPosPix);

        if (nSlotId == SID_ZOOM_PANNING)
        {
            // Panning

            Point aScroll = aBeginPos - aEndPos;

            // #i2237#
            // removed old stuff here which still forced zoom to be
            // %BRUSH_SIZE which is outdated now

            if (aScroll.X() != 0 || aScroll.Y() != 0)
            {
                Size aWorkSize = pView->GetWorkArea().GetSize();
                Size aPageSize = pView->GetSdrPageView()->GetPage()->GetSize();
                aScroll.X() /= aWorkSize.Width()  / aPageSize.Width();
                aScroll.Y() /= aWorkSize.Height() / aPageSize.Height();
                pViewShell->Scroll(aScroll.X(), aScroll.Y());
                aBeginPosPix = aPosPix;
            }
        }
        else
        {
            Rectangle aRect(aBeginPos, aEndPos);
            aZoomRect = aRect;
            aZoomRect.Justify();
            pViewShell->DrawMarkRect(aZoomRect);
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
        pViewShell->DrawMarkRect(aZoomRect);
        bVisible = FALSE;
    }

    Point aPosPix = rMEvt.GetPosPixel();

    if (nSlotId == SID_ZOOM_PANNING)
    {
        // Panning
        // Ersatzdarstellung restaurieren
        pView->SetLineDraft(bLineDraft);
        pView->SetFillDraft(bFillDraft);
        pView->SetTextDraft(bTextDraft);
        pView->SetGrafDraft(bGrafDraft);
    }
    else
    {
        // Zoom
        Size aZoomSizePixel = pWindow->LogicToPixel(aZoomRect).GetSize();
        ULONG nTol = DRGPIX + DRGPIX;

        if ( aZoomSizePixel.Width() < (long) nTol && aZoomSizePixel.Height() < (long) nTol )
        {
            // Klick auf der Stelle: Zoomfaktor verdoppeln
            Point aPos = pWindow->PixelToLogic(aPosPix);
            Size aSize = pWindow->PixelToLogic(pWindow->GetOutputSizePixel());
            aSize.Width() /= 2;
            aSize.Height() /= 2;
            aPos.X() -= aSize.Width() / 2;
            aPos.Y() -= aSize.Height() / 2;
            aZoomRect.SetPos(aPos);
            aZoomRect.SetSize(aSize);
        }

        pViewShell->SetZoomRect(aZoomRect);
    }

    Rectangle aVisAreaWin = pWindow->PixelToLogic(Rectangle(Point(0,0),
                                           pWindow->GetOutputSizePixel()));
    pViewShell->GetZoomList()->InsertZoomRect(aVisAreaWin);

    bStartDrag = FALSE;
    pWindow->ReleaseMouse();
    pViewShell->Cancel();

    return TRUE;
}

/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/

void FuZoom::Activate()
{
    aPtr = pWindow->GetPointer();

    if (nSlotId == SID_ZOOM_PANNING)
    {
        pWindow->SetPointer(Pointer(POINTER_HAND));
    }
    else
    {
        pWindow->SetPointer(Pointer(POINTER_MAGNIFY));
    }
}

/*************************************************************************
|*
|* Function deaktivieren
|*
\************************************************************************/

void FuZoom::Deactivate()
{
    pWindow->SetPointer( aPtr );
    pViewShell->GetViewFrame()->GetBindings().Invalidate( SidArrayZoom );
}
} // end of namespace sd
