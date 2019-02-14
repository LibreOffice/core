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

#include <fudspord.hxx>

#include <svx/svxids.hrc>
#include <vcl/ptrstyle.hxx>

#include <app.hrc>
#include <fupoor.hxx>
#include <ViewShell.hxx>
#include <View.hxx>
#include <Window.hxx>
#include <drawdoc.hxx>

namespace sd {


FuDisplayOrder::FuDisplayOrder( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq)
: FuPoor(pViewSh, pWin, pView, pDoc, rReq)
, maPtr(PointerStyle::Arrow)
, mpRefObj(nullptr)
{
}

FuDisplayOrder::~FuDisplayOrder()
{
}

void FuDisplayOrder::implClearOverlay()
{
    mpOverlay.reset();
}

rtl::Reference<FuPoor> FuDisplayOrder::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    rtl::Reference<FuPoor> xFunc( new FuDisplayOrder( pViewSh, pWin, pView, pDoc, rReq ) );
    return xFunc;
}

bool FuDisplayOrder::MouseButtonDown(const MouseEvent& rMEvt)
{
    // remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    return true;
}

bool FuDisplayOrder::MouseMove(const MouseEvent& rMEvt)
{
    SdrPageView* pPV;
    Point aPnt( mpWindow->PixelToLogic( rMEvt.GetPosPixel() ) );

    SdrObject* pPickObj = mpView->PickObj(aPnt, mpView->getHitTolLog(), pPV);
    if (pPickObj)
    {
        if (mpRefObj != pPickObj)
        {
            // delete current overlay
            implClearOverlay();

            // create new one
            mpOverlay.reset( new SdrDropMarkerOverlay(*mpView, *pPickObj) );

            // remember referenced object
            mpRefObj = pPickObj;
        }
    }
    else
    {
        mpRefObj = nullptr;
        implClearOverlay();
    }

    return true;
}

bool FuDisplayOrder::MouseButtonUp(const MouseEvent& rMEvt)
{
    // remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    SdrPageView* pPV = nullptr;
    Point aPnt( mpWindow->PixelToLogic( rMEvt.GetPosPixel() ) );

    mpRefObj = mpView->PickObj(aPnt, mpView->getHitTolLog(), pPV);
    if (mpRefObj)
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

void FuDisplayOrder::Activate()
{
    maPtr = mpWindow->GetPointer();
    mpWindow->SetPointer( PointerStyle::RefHand );
}

void FuDisplayOrder::Deactivate()
{
    mpWindow->SetPointer( maPtr );
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
