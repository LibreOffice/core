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
#include <rtl/ustring.hxx>
#include <svl/aeitem.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/request.hxx>
#include <svl/intitem.hxx>

#include <svx/fmglob.hxx>

#include <svx/dialogs.hrc>
#include <vcl/ptrstyle.hxx>

#include <app.hrc>
#include <strings.hrc>
#include <ViewShell.hxx>
#include <View.hxx>
#include <Window.hxx>
#include <ViewShellBase.hxx>
#include <ToolBarManager.hxx>
#include <drawdoc.hxx>
#include <unokywds.hxx>


namespace sd {


FuConstructUnoControl::FuConstructUnoControl (
    ViewShell*  pViewSh,
    ::sd::Window*       pWin,
    ::sd::View*         pView,
    SdDrawDocument* pDoc,
    SfxRequest&     rReq)
    : FuConstruct(pViewSh, pWin, pView, pDoc, rReq)
    , nInventor(SdrInventor::Unknown)
    , nIdentifier(0)
{
}

rtl::Reference<FuPoor> FuConstructUnoControl::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq, bool bPermanent )
{
    FuConstructUnoControl* pFunc;
    rtl::Reference<FuPoor> xFunc( pFunc = new FuConstructUnoControl( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    pFunc->SetPermanent(bPermanent);
    return xFunc;
}

void FuConstructUnoControl::DoExecute( SfxRequest& rReq )
{
    FuConstruct::DoExecute( rReq );

    const SfxUInt32Item* pInventorItem = rReq.GetArg<SfxUInt32Item>(SID_FM_CONTROL_INVENTOR);
    const SfxUInt16Item* pIdentifierItem = rReq.GetArg<SfxUInt16Item>(SID_FM_CONTROL_IDENTIFIER);
    if( pInventorItem )
        nInventor = static_cast<SdrInventor>(pInventorItem->GetValue());
    if( pIdentifierItem )
        nIdentifier = pIdentifierItem->GetValue();

    mpViewShell->GetViewShellBase().GetToolBarManager()->SetToolBar(
        ToolBarManager::ToolBarGroup::Function,
        ToolBarManager::msDrawingObjectToolBar);
}

bool FuConstructUnoControl::MouseButtonDown(const MouseEvent& rMEvt)
{
    bool bReturn = FuConstruct::MouseButtonDown(rMEvt);

    if ( rMEvt.IsLeft() && !mpView->IsAction() )
    {
        Point aPnt( mpWindow->PixelToLogic( rMEvt.GetPosPixel() ) );
        mpWindow->CaptureMouse();
        sal_uInt16 nDrgLog = sal_uInt16 ( mpWindow->PixelToLogic(Size(DRGPIX,0)).Width() );
        mpView->BegCreateObj(aPnt, nullptr, nDrgLog);
        bReturn = true;
    }
    return bReturn;
}

bool FuConstructUnoControl::MouseButtonUp(const MouseEvent& rMEvt)
{
    bool bReturn = false;

    if ( mpView->IsCreateObj() && rMEvt.IsLeft() )
    {
        mpView->EndCreateObj(SdrCreateCmd::ForceEnd);
        bReturn = true;
    }

    bReturn = (FuConstruct::MouseButtonUp(rMEvt) || bReturn);

    if (!bPermanent)
        mpViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SfxCallMode::ASYNCHRON);

    return bReturn;
}

void FuConstructUnoControl::Activate()
{
    mpView->SetCurrentObj( nIdentifier, nInventor );

    aNewPointer = PointerStyle::DrawRect;
    aOldPointer = mpWindow->GetPointer();
    mpWindow->SetPointer( aNewPointer );

    aOldLayer = mpView->GetActiveLayer();
    mpView->SetActiveLayer(sUNO_LayerName_controls);

    FuConstruct::Activate();
}

void FuConstructUnoControl::Deactivate()
{
    FuConstruct::Deactivate();
    mpView->SetActiveLayer( aOldLayer );
    mpWindow->SetPointer( aOldPointer );
}

SdrObjectUniquePtr FuConstructUnoControl::CreateDefaultObject(const sal_uInt16, const ::tools::Rectangle& rRectangle)
{
    // case SID_FM_CREATE_CONTROL:

    SdrObjectUniquePtr pObj(SdrObjFactory::MakeNewObject(
        mpView->getSdrModelFromSdrView(),
        mpView->GetCurrentObjInventor(),
        mpView->GetCurrentObjIdentifier()));

    if(pObj)
    {
        pObj->SetLogicRect(rRectangle);
    }

    return pObj;
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
