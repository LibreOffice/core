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

#include <sfx2/bindings.hxx>
#include <sfx2/viewfrm.hxx>

#include <svx/svxdlg.hxx>

#include <drawdoc.hxx>
#include <ViewShell.hxx>
#include <app.hrc>

#include <svtools/strings.hrc>
#include <svtools/svtresid.hxx>
#include <officecfg/Office/Common.hxx>

class SfxRequest;

namespace sd {


FuLink::FuLink (
    ViewShell& rViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument* pDoc,
    SfxRequest& rReq )
    : FuPoor(rViewSh, pWin, pView, pDoc, rReq)
{
}

rtl::Reference<FuPoor> FuLink::Create( ViewShell& rViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    rtl::Reference<FuPoor> xFunc( new FuLink( rViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuLink::DoExecute( SfxRequest& )
{
    if (officecfg::Office::Common::Security::Scripting::DisableActiveContent::get())
    {
        std::unique_ptr<weld::MessageDialog> xError(
            Application::CreateMessageDialog(nullptr, VclMessageType::Warning, VclButtonsType::Ok,
                                             SvtResId(STR_WARNING_EXTERNAL_LINK_EDIT_DISABLED)));
        xError->run();
        return;
    }

    sfx2::LinkManager* pLinkManager = mpDoc->GetLinkManager();

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    VclPtr<SfxAbstractLinksDialog> pDlg(pFact->CreateLinksDialog(mrViewShell.GetFrameWeld(), pLinkManager));
    pDlg->StartExecuteAsync(
        [pDlg, pViewShell = &mrViewShell] (sal_Int32 /*nResult*/)->void
        {
            pViewShell->GetViewFrame()->GetBindings().Invalidate( SID_MANAGE_LINKS );
            pDlg->disposeOnce();
        }
    );
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
