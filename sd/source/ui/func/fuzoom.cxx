/*************************************************************************
 *
 *  $RCSfile: fuzoom.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:36 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#pragma hdrstop

#include <svx/svxids.hrc>
#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif

#include "app.hrc"

#ifndef _SVDPAGV_HXX //autogen
#include <svx/svdpagv.hxx>
#endif

#include "fuzoom.hxx"
#include "frmview.hxx"
#include "viewshel.hxx"
#include "sdview.hxx"
#include "sdwindow.hxx"
#include "drawdoc.hxx"
#include "zoomlist.hxx"

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

FuZoom::FuZoom(SdViewShell*     pViewSh,
               SdWindow*        pWin,
               SdView*          pView,
               SdDrawDocument*  pDoc,
               SfxRequest&      rReq) :
    FuPoor(pViewSh, pWin, pView, pDoc, rReq),
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
#pragma SEG_FUNCDEF(fuzoom_02)

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

/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/
#pragma SEG_FUNCDEF(fuzoom_03)

BOOL FuZoom::MouseButtonDown(const MouseEvent& rMEvt)
{
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
#pragma SEG_FUNCDEF(fuzoom_04)

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

            // Nur um Vielfache der BRUSH_SIZE scrollen
            if (Abs(aBeginPosPix.X() - aPosPix.X()) < BRUSH_SIZE)
            {
                aScroll.X() = 0;
                aPosPix.X() = aBeginPosPix.X();
            }

            if (Abs(aBeginPosPix.Y() - aPosPix.Y()) < BRUSH_SIZE)
            {
                aScroll.Y() = 0;
                aPosPix.Y() = aBeginPosPix.Y();
            }

            if (aScroll.X() != 0 || aScroll.Y() != 0)
            {
                Size aWorkSize = pView->GetWorkArea().GetSize();
                Size aPageSize = pView->GetPageViewPvNum(0)->GetPage()->GetSize();
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
#pragma SEG_FUNCDEF(fuzoom_05)

BOOL FuZoom::MouseButtonUp(const MouseEvent& rMEvt)
{
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

        if ( aZoomSizePixel.Width() < nTol && aZoomSizePixel.Height() < nTol )
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
#pragma SEG_FUNCDEF(fuzoom_06)

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
#pragma SEG_FUNCDEF(fuzoom_07)

void FuZoom::Deactivate()
{
    pWindow->SetPointer( aPtr );
    // Updaten der StatusBar
    SFX_BINDINGS().Invalidate( SidArrayZoom );
}


