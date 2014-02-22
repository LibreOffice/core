/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "fudspord.hxx"

#include <svx/svxids.hrc>
#include <vcl/pointr.hxx>

#include "app.hrc"
#include "fupoor.hxx"
#include "ViewShell.hxx"
#include "View.hxx"
#include "Window.hxx"
#include "drawdoc.hxx"

namespace sd {

TYPEINIT1( FuDisplayOrder, FuPoor );


FuDisplayOrder::FuDisplayOrder( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq)
: FuPoor(pViewSh, pWin, pView, pDoc, rReq)
, mpRefObj(NULL)
, mpOverlay(0L)
{
}


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

rtl::Reference<FuPoor> FuDisplayOrder::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    rtl::Reference<FuPoor> xFunc( new FuDisplayOrder( pViewSh, pWin, pView, pDoc, rReq ) );
    return xFunc;
}


sal_Bool FuDisplayOrder::MouseButtonDown(const MouseEvent& rMEvt)
{
    
    SetMouseButtonCode(rMEvt.GetButtons());

    return sal_True;
}


sal_Bool FuDisplayOrder::MouseMove(const MouseEvent& rMEvt)
{
    SdrObject* pPickObj;
    SdrPageView* pPV;
    Point aPnt( mpWindow->PixelToLogic( rMEvt.GetPosPixel() ) );

    if ( mpView->PickObj(aPnt, mpView->getHitTolLog(), pPickObj, pPV) )
    {
        if (mpRefObj != pPickObj)
        {
            
            implClearOverlay();

            
            mpOverlay = new SdrDropMarkerOverlay(*mpView, *pPickObj);

            
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


sal_Bool FuDisplayOrder::MouseButtonUp(const MouseEvent& rMEvt)
{
    
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


void FuDisplayOrder::Activate()
{
    maPtr = mpWindow->GetPointer();
    mpWindow->SetPointer( Pointer( POINTER_REFHAND ) );
}


void FuDisplayOrder::Deactivate()
{
    mpWindow->SetPointer( maPtr );
}


} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
