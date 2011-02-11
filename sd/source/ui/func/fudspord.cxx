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



#include "fudspord.hxx"

#include <svx/svxids.hrc>
#include <vcl/pointr.hxx>

#include "app.hrc"
#include "fupoor.hxx"
#include "ViewShell.hxx"
#include "View.hxx"
#ifndef SD_WINDOW_SHELL_HXX
#include "Window.hxx"
#endif
#include "drawdoc.hxx"

namespace sd {

TYPEINIT1( FuDisplayOrder, FuPoor );

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuDisplayOrder::FuDisplayOrder( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq)
: FuPoor(pViewSh, pWin, pView, pDoc, rReq)
, mpRefObj(NULL)
, mpOverlay(0L)
{
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

FuDisplayOrder::~FuDisplayOrder()
{
    implClearOverlay();
}

void FuDisplayOrder::implClearOverlay()
{
    if(mpOverlay)
    {
        delete mpOverlay;
        mpOverlay = 0L;
    }
}

FunctionReference FuDisplayOrder::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    FunctionReference xFunc( new FuDisplayOrder( pViewSh, pWin, pView, pDoc, rReq ) );
    return xFunc;
}

/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/

sal_Bool FuDisplayOrder::MouseButtonDown(const MouseEvent& rMEvt)
{
    // #95491# remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    return sal_True;
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/

sal_Bool FuDisplayOrder::MouseMove(const MouseEvent& rMEvt)
{
    SdrObject* pPickObj;
    SdrPageView* pPV;
    Point aPnt( mpWindow->PixelToLogic( rMEvt.GetPosPixel() ) );

    if ( mpView->PickObj(aPnt, mpView->getHitTolLog(), pPickObj, pPV) )
    {
        if (mpRefObj != pPickObj)
        {
            // delete current overlay
            implClearOverlay();

            // create new one
            mpOverlay = new SdrDropMarkerOverlay(*mpView, *pPickObj);

            // remember referenced object
            mpRefObj = pPickObj;
        }
    }
    else
    {
        mpRefObj = NULL;
        implClearOverlay();
    }

    return sal_True;
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

sal_Bool FuDisplayOrder::MouseButtonUp(const MouseEvent& rMEvt)
{
    // #95491# remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    SdrPageView* pPV = NULL;
    Point aPnt( mpWindow->PixelToLogic( rMEvt.GetPosPixel() ) );

    if ( mpView->PickObj(aPnt, mpView->getHitTolLog(), mpRefObj, pPV) )
    {
        if (nSlotId == SID_BEFORE_OBJ)
        {
            mpView->PutMarkedInFrontOfObj(mpRefObj);
        }
        else
        {
            mpView->PutMarkedBehindObj(mpRefObj);
        }
    }

    mpViewShell->Cancel();

    return sal_True;
}

/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/

void FuDisplayOrder::Activate()
{
    maPtr = mpWindow->GetPointer();
    mpWindow->SetPointer( Pointer( POINTER_REFHAND ) );
}

/*************************************************************************
|*
|* Function deaktivieren
|*
\************************************************************************/

void FuDisplayOrder::Deactivate()
{
    mpWindow->SetPointer( maPtr );
}


} // end of namespace sd
