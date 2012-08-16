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
        case SID_SHOW_TOOL_PANEL:
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
