/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "precompiled_sc.hxx"

#include "fuconuno.hxx"
#include "tabvwsh.hxx"
#include "sc.hrc"
#include "drawview.hxx"

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuConstUnoControl::FuConstUnoControl(ScTabViewShell* pViewSh, Window* pWin, ScDrawView* pViewP,
                   SdrModel* pDoc, SfxRequest& rReq)
    : FuConstruct(pViewSh, pWin, pViewP, pDoc, rReq)
{
    SFX_REQUEST_ARG( rReq, pInventorItem, SfxUInt32Item, SID_FM_CONTROL_INVENTOR, FALSE );
    SFX_REQUEST_ARG( rReq, pIdentifierItem, SfxUInt16Item, SID_FM_CONTROL_IDENTIFIER, FALSE );
    if( pInventorItem )
        nInventor = pInventorItem->GetValue();
    if( pIdentifierItem )
        nIdentifier = pIdentifierItem->GetValue();
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

FuConstUnoControl::~FuConstUnoControl()
{
}

/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/

BOOL FuConstUnoControl::MouseButtonDown(const MouseEvent& rMEvt)
{
    // remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    BOOL bReturn = FuConstruct::MouseButtonDown(rMEvt);

    if ( rMEvt.IsLeft() && !pView->IsAction() )
    {
        Point aPnt( pWindow->PixelToLogic( rMEvt.GetPosPixel() ) );
        pWindow->CaptureMouse();
        pView->BegCreateObj(aPnt);
        bReturn = TRUE;
    }
    return bReturn;
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/

BOOL FuConstUnoControl::MouseMove(const MouseEvent& rMEvt)
{
    return FuConstruct::MouseMove(rMEvt);
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

BOOL FuConstUnoControl::MouseButtonUp(const MouseEvent& rMEvt)
{
    // remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    BOOL bReturn = FALSE;

    if ( pView->IsCreateObj() && rMEvt.IsLeft() )
    {
        Point aPnt( pWindow->PixelToLogic( rMEvt.GetPosPixel() ) );
        pView->EndCreateObj(SDRCREATE_FORCEEND);
        bReturn = TRUE;
    }
    return (FuConstruct::MouseButtonUp(rMEvt) || bReturn);
}

/*************************************************************************
|*
|* Tastaturereignisse bearbeiten
|*
|* Wird ein KeyEvent bearbeitet, so ist der Return-Wert TRUE, andernfalls
|* FALSE.
|*
\************************************************************************/

BOOL FuConstUnoControl::KeyInput(const KeyEvent& rKEvt)
{
    BOOL bReturn = FuConstruct::KeyInput(rKEvt);
    return(bReturn);
}

/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/

void FuConstUnoControl::Activate()
{
    pView->SetCurrentObj( nIdentifier, nInventor );

    aNewPointer = Pointer( POINTER_DRAW_RECT );
    aOldPointer = pWindow->GetPointer();
    pViewShell->SetActivePointer( aNewPointer );

    SdrLayer* pLayer = pView->GetModel()->GetLayerAdmin().GetLayerPerID(SC_LAYER_CONTROLS);
    if (pLayer)
        pView->SetActiveLayer( pLayer->GetName() );

    FuConstruct::Activate();
}

/*************************************************************************
|*
|* Function deaktivieren
|*
\************************************************************************/

void FuConstUnoControl::Deactivate()
{
    FuConstruct::Deactivate();

    SdrLayer* pLayer = pView->GetModel()->GetLayerAdmin().GetLayerPerID(SC_LAYER_FRONT);
    if (pLayer)
        pView->SetActiveLayer( pLayer->GetName() );

    pViewShell->SetActivePointer( aOldPointer );
}

// Create default drawing objects via keyboard
SdrObject* FuConstUnoControl::CreateDefaultObject(const sal_uInt16 /* nID */, const Rectangle& rRectangle)
{
    // case SID_FM_CREATE_CONTROL:

    SdrObject* pObj = SdrObjFactory::MakeNewObject(
        pView->GetCurrentObjInventor(), pView->GetCurrentObjIdentifier(),
        0L, pDrDoc);

    if(pObj)
    {
        pObj->SetLogicRect(rRectangle);
    }

    return pObj;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
