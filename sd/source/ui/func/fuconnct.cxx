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

#include "fuconnct.hxx"
#include <sfx2/request.hxx>
#include <vcl/msgbox.hxx>
#include "View.hxx"
#include "ViewShell.hxx"
#include "drawdoc.hxx"
#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>
#include <memory>

namespace sd {


FuConnectionDlg::FuConnectionDlg (
    ViewShell* pViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument* pDoc,
    SfxRequest& rReq)
    : FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{
}

rtl::Reference<FuPoor> FuConnectionDlg::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    rtl::Reference<FuPoor> xFunc( new FuConnectionDlg( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuConnectionDlg::DoExecute( SfxRequest& rReq )
{
    SfxItemSet aNewAttr( mpDoc->GetPool() );
    mpView->GetAttributes( aNewAttr );

    const SfxItemSet* pArgs = rReq.GetArgs();

    if( !pArgs )
    {
        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        std::unique_ptr<SfxAbstractDialog> pDlg( pFact ? pFact->CreateSfxDialog( nullptr, aNewAttr, mpView, RID_SVXPAGE_CONNECTION) : nullptr);

        if( pDlg.get() && (pDlg->Execute() == RET_OK) )
        {
            rReq.Done( *pDlg->GetOutputItemSet() );
            pArgs = rReq.GetArgs();
        }
    }
    if( pArgs )
        mpView->SetAttributes( *pArgs );
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
