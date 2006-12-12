/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fudspord.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 17:17:20 $
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



#include "fudspord.hxx"

#include <svx/svxids.hrc>
#ifndef _VCL_POINTR_HXX //autogen
#include <vcl/pointr.hxx>
#endif

#include "app.hrc"
#ifndef SD_FU_POOR_HXX
#include "fupoor.hxx"
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

BOOL FuDisplayOrder::MouseButtonDown(const MouseEvent& rMEvt)
{
    // #95491# remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    return TRUE;
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/

BOOL FuDisplayOrder::MouseMove(const MouseEvent& rMEvt)
{
    SdrObject* pPickObj;
    SdrPageView* pPV;
    Point aPnt( mpWindow->PixelToLogic( rMEvt.GetPosPixel() ) );

    if ( mpView->PickObj(aPnt, pPickObj, pPV) )
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

    return TRUE;
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

BOOL FuDisplayOrder::MouseButtonUp(const MouseEvent& rMEvt)
{
    // #95491# remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    SdrPageView* pPV = NULL;
    Point aPnt( mpWindow->PixelToLogic( rMEvt.GetPosPixel() ) );

    if ( mpView->PickObj(aPnt, mpRefObj, pPV) )
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

    return TRUE;
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
