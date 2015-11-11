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

#include "ImpressViewShellBase.hxx"

#include "DrawDocShell.hxx"
#include "sdresid.hxx"
#include "strings.hrc"
#include "app.hrc"
#include "framework/FrameworkHelper.hxx"
#include "framework/ImpressModule.hxx"
#include "MasterPageObserver.hxx"
#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/viewsh.hxx>

namespace sd {


// We have to expand the SFX_IMPL_VIEWFACTORY macro to call LateInit() after a
// new ImpressViewShellBase object has been constructed.

SfxViewFactory* ImpressViewShellBase::pFactory;
SfxViewShell* ImpressViewShellBase::CreateInstance (
    SfxViewFrame *pFrame, SfxViewShell *pOldView)
{
    ImpressViewShellBase* pBase = new ImpressViewShellBase(pFrame, pOldView);
    pBase->LateInit("");
    return pBase;
}
void ImpressViewShellBase::RegisterFactory( sal_uInt16 nPrio )
{
    pFactory = new SfxViewFactory(&CreateInstance,nPrio,"Default");
    InitFactory();
}
void ImpressViewShellBase::InitFactory()
{
    SFX_VIEW_REGISTRATION(DrawDocShell);
}

ImpressViewShellBase::ImpressViewShellBase (
    SfxViewFrame* _pFrame,
    SfxViewShell* pOldShell)
    : ViewShellBase (_pFrame, pOldShell)
{
    MasterPageObserver::Instance().RegisterDocument (*GetDocShell()->GetDoc());
}

ImpressViewShellBase::~ImpressViewShellBase()
{
    MasterPageObserver::Instance().UnregisterDocument (*GetDocShell()->GetDoc());
}

void ImpressViewShellBase::Execute (SfxRequest& rRequest)
{
    sal_uInt16 nSlotId = rRequest.GetSlot();

    switch (nSlotId)
    {
        case SID_LEFT_PANE_DRAW:
            // Prevent a Draw-only slots from being executed.
            rRequest.Cancel();
            break;

        default:
            // The remaining requests are forwarded to our base class.
            ViewShellBase::Execute(rRequest);
            break;
    }
}

void ImpressViewShellBase::InitializeFramework()
{
    css::uno::Reference<css::frame::XController>
        xController (GetController());
    sd::framework::ImpressModule::Initialize(xController);
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
