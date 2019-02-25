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

#include <fuconuno.hxx>
#include <tabvwsh.hxx>
#include <drawview.hxx>

FuConstUnoControl::FuConstUnoControl(ScTabViewShell& rViewSh, vcl::Window* pWin, ScDrawView* pViewP,
                                     SdrModel* pDoc, const SfxRequest& rReq)
    : FuConstruct(rViewSh, pWin, pViewP, pDoc, rReq)
    , nInventor(SdrInventor::Unknown)
    , nIdentifier(0)
{
    const SfxUInt32Item* pInventorItem = rReq.GetArg<SfxUInt32Item>(SID_FM_CONTROL_INVENTOR);
    const SfxUInt16Item* pIdentifierItem = rReq.GetArg<SfxUInt16Item>(SID_FM_CONTROL_IDENTIFIER);
    if( pInventorItem )
        nInventor = static_cast<SdrInventor>(pInventorItem->GetValue());
    if( pIdentifierItem )
        nIdentifier = pIdentifierItem->GetValue();
}

FuConstUnoControl::~FuConstUnoControl()
{
}

bool FuConstUnoControl::MouseButtonDown(const MouseEvent& rMEvt)
{
    // remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    bool bReturn = FuConstruct::MouseButtonDown(rMEvt);

    if ( rMEvt.IsLeft() && !pView->IsAction() )
    {
        Point aPnt( pWindow->PixelToLogic( rMEvt.GetPosPixel() ) );
        pWindow->CaptureMouse();
        pView->BegCreateObj(aPnt);
        bReturn = true;
    }
    return bReturn;
}

bool FuConstUnoControl::MouseButtonUp(const MouseEvent& rMEvt)
{
    // remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    bool bReturn = false;

    if ( pView->IsCreateObj() && rMEvt.IsLeft() )
    {
        pView->EndCreateObj(SdrCreateCmd::ForceEnd);
        bReturn = true;
    }
    return (FuConstruct::MouseButtonUp(rMEvt) || bReturn);
}

void FuConstUnoControl::Activate()
{
    pView->SetCurrentObj( nIdentifier, nInventor );

    aNewPointer = PointerStyle::DrawRect;
    aOldPointer = pWindow->GetPointer();
    rViewShell.SetActivePointer( aNewPointer );

    SdrLayer* pLayer = pView->GetModel()->GetLayerAdmin().GetLayerPerID(SC_LAYER_CONTROLS);
    if (pLayer)
        pView->SetActiveLayer( pLayer->GetName() );

    FuConstruct::Activate();
}

void FuConstUnoControl::Deactivate()
{
    FuConstruct::Deactivate();

    SdrLayer* pLayer = pView->GetModel()->GetLayerAdmin().GetLayerPerID(SC_LAYER_FRONT);
    if (pLayer)
        pView->SetActiveLayer( pLayer->GetName() );

    rViewShell.SetActivePointer( aOldPointer );
}

// Create default drawing objects via keyboard
SdrObjectUniquePtr FuConstUnoControl::CreateDefaultObject(const sal_uInt16 /* nID */, const tools::Rectangle& rRectangle)
{
    // case SID_FM_CREATE_CONTROL:

    SdrObjectUniquePtr pObj(SdrObjFactory::MakeNewObject(
        *pDrDoc,
        pView->GetCurrentObjInventor(),
        pView->GetCurrentObjIdentifier()));

    if(pObj)
    {
        pObj->SetLogicRect(rRectangle);
    }

    return pObj;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
