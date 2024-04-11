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

#include <GraphicViewShellBase.hxx>

#include <GraphicDocShell.hxx>
#include <DrawController.hxx>
#include <app.hrc>
#include <framework/DrawModule.hxx>
#include <framework/FrameworkHelper.hxx>
#include <sfx2/request.hxx>
#include <sfx2/viewfac.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/viewsh.hxx>

namespace sd
{
// We have to expand the SFX_IMPL_VIEWFACTORY macro to call LateInit() after a
// new GraphicViewShellBase object has been constructed.

SfxViewFactory* GraphicViewShellBase::s_pFactory;
SfxViewShell* GraphicViewShellBase::CreateInstance(SfxViewFrame& rFrame, SfxViewShell* pOldView)
{
    GraphicViewShellBase* pBase = new GraphicViewShellBase(rFrame, pOldView);
    pBase->LateInit(framework::FrameworkHelper::msDrawViewURL);
    return pBase;
}
void GraphicViewShellBase::RegisterFactory(SfxInterfaceId nPrio)
{
    s_pFactory = new SfxViewFactory(&CreateInstance, nPrio, "Default");
    InitFactory();
}
void GraphicViewShellBase::InitFactory() { SFX_VIEW_REGISTRATION(GraphicDocShell); }

GraphicViewShellBase::GraphicViewShellBase(SfxViewFrame& _rFrame, SfxViewShell* pOldShell)
    : ViewShellBase(_rFrame, pOldShell)
{
}

GraphicViewShellBase::~GraphicViewShellBase() {}

void GraphicViewShellBase::Execute(SfxRequest& rRequest)
{
    sal_uInt16 nSlotId = rRequest.GetSlot();

    switch (nSlotId)
    {
        case SID_BOTTOM_PANE_IMPRESS:
        case SID_SLIDE_SORTER_MULTI_PANE_GUI:
        case SID_SLIDE_SORTER_MODE:
        case SID_SLIDE_MASTER_MODE:
        case SID_OUTLINE_MODE:
        case SID_NOTES_MODE:
        case SID_NOTES_MASTER_MODE:
        case SID_HANDOUT_MASTER_MODE:
            // Prevent some Impress-only slots from being executed.
            rRequest.Cancel();
            break;

        case SID_SWITCH_SHELL:
        case SID_LEFT_PANE_DRAW:
        case SID_LEFT_PANE_IMPRESS:
        default:
            // The remaining requests are forwarded to our base class.
            ViewShellBase::Execute(rRequest);
            break;
    }
}

void GraphicViewShellBase::InitializeFramework()
{
    rtl::Reference<sd::DrawController> xController(GetDrawController());
    sd::framework::DrawModule::Initialize(xController);
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
