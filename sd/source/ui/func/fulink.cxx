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

#include <fulink.hxx>

#include <sfx2/linkmgr.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/viewfrm.hxx>

#include <svx/svxdlg.hxx>

#include <Window.hxx>
#include <drawdoc.hxx>
#include <ViewShell.hxx>
#include <app.hrc>
#include <memory>

class SfxRequest;

namespace sd {


FuLink::FuLink (
    ViewShell* pViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument* pDoc,
    SfxRequest& rReq )
    : FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{
}

rtl::Reference<FuPoor> FuLink::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    rtl::Reference<FuPoor> xFunc( new FuLink( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuLink::DoExecute( SfxRequest& )
{
    sfx2::LinkManager* pLinkManager = mpDoc->GetLinkManager();

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    ScopedVclPtr<SfxAbstractLinksDialog> pDlg(pFact->CreateLinksDialog(mpViewShell->GetFrameWeld(), pLinkManager));
    pDlg->Execute();
    mpViewShell->GetViewFrame()->GetBindings().Invalidate( SID_MANAGE_LINKS );
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
