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


#include "GraphicViewShellBase.hxx"

#include "GraphicDocShell.hxx"
#include "sdresid.hxx"
#include "strings.hrc"
#include "app.hrc"
#include "framework/DrawModule.hxx"
#include "framework/FrameworkHelper.hxx"
#include <sfx2/request.hxx>

namespace sd {

TYPEINIT1(GraphicViewShellBase, ViewShellBase);

// We have to expand the SFX_IMPL_VIEWFACTORY macro to call LateInit() after a
// new GraphicViewShellBase object has been constructed.

SfxViewFactory* GraphicViewShellBase::pFactory;
SfxViewShell* GraphicViewShellBase::CreateInstance (
    SfxViewFrame *pFrame, SfxViewShell *pOldView)
{
    GraphicViewShellBase* pBase = new GraphicViewShellBase(pFrame, pOldView);
    pBase->LateInit(framework::FrameworkHelper::msDrawViewURL);
    return pBase;
}
void GraphicViewShellBase::RegisterFactory( sal_uInt16 nPrio )
{
    pFactory = new SfxViewFactory(&CreateInstance,nPrio,"Default");
    InitFactory();
}
void GraphicViewShellBase::InitFactory()
{
    SFX_VIEW_REGISTRATION(GraphicDocShell);
}








GraphicViewShellBase::GraphicViewShellBase (
    SfxViewFrame* _pFrame,
    SfxViewShell* pOldShell)
    : ViewShellBase (_pFrame, pOldShell)
{
}




GraphicViewShellBase::~GraphicViewShellBase (void)
{
}




void GraphicViewShellBase::Execute (SfxRequest& rRequest)
{
    sal_uInt16 nSlotId = rRequest.GetSlot();

    switch (nSlotId)
    {
        case SID_NOTES_WINDOW:
        case SID_SLIDE_SORTER_MULTI_PANE_GUI:
        case SID_DIAMODE:
        case SID_OUTLINEMODE:
        case SID_NOTESMODE:
        case SID_HANDOUTMODE:
            // Prevent some Impress-only slots from being executed.
            rRequest.Cancel();
            break;

        case SID_TASKPANE:
        case SID_SWITCH_SHELL:
        case SID_LEFT_PANE_DRAW:
        case SID_LEFT_PANE_IMPRESS:
        default:
            // The remaining requests are forwarded to our base class.
            ViewShellBase::Execute (rRequest);
            break;
    }

}




void GraphicViewShellBase::InitializeFramework (void)
{
    com::sun::star::uno::Reference<com::sun::star::frame::XController>
        xController (GetController());
    sd::framework::DrawModule::Initialize(xController);
}


} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
