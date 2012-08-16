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


#include "ImpressViewShellBase.hxx"

#include "DrawDocShell.hxx"
#include "sdresid.hxx"
#include "strings.hrc"
#include "app.hrc"
#include "framework/FrameworkHelper.hxx"
#include "framework/ImpressModule.hxx"
#include "MasterPageObserver.hxx"
#include <sfx2/request.hxx>

namespace sd {

TYPEINIT1(ImpressViewShellBase, ViewShellBase);

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




ImpressViewShellBase::~ImpressViewShellBase (void)
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




void ImpressViewShellBase::InitializeFramework (void)
{
    ::com::sun::star::uno::Reference<com::sun::star::frame::XController>
        xController (GetController());
    sd::framework::ImpressModule::Initialize(xController);
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
