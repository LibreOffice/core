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

bool FuDisplayOrder::MouseButtonDown(const MouseEvent& rMEvt)
{
    // #95491# remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    return true;
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/

bool FuDisplayOrder::MouseMove(const MouseEvent& rMEvt)
{
    SdrObject* pPickObj;
    const basegfx::B2DPoint aPnt(mpWindow->GetInverseViewTransformation() * basegfx::B2DPoint(rMEvt.GetPosPixel().X(), rMEvt.GetPosPixel().Y()));

    if ( mpView->PickObj(aPnt, mpView->getHitTolLog(), pPickObj) )
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

    return true;
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

bool FuDisplayOrder::MouseButtonUp(const MouseEvent& rMEvt)
{
    // #95491# remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());
    const basegfx::B2DPoint aPnt(mpWindow->GetInverseViewTransformation() * basegfx::B2DPoint(rMEvt.GetPosPixel().X(), rMEvt.GetPosPixel().Y()));

    if ( mpView->PickObj(aPnt, mpView->getHitTolLog(), mpRefObj) )
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

    return true;
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
