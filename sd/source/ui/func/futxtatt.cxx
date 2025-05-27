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

#include <futxtatt.hxx>
#include <sfx2/request.hxx>

#include <svx/svxdlg.hxx>
#include <View.hxx>
#include <drawdoc.hxx>

namespace sd {


FuTextAttrDlg::FuTextAttrDlg (
    ViewShell& rViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument& rDoc,
    SfxRequest& rReq)
    : FuPoor(rViewSh, pWin, pView, rDoc, rReq)
{
}

rtl::Reference<FuPoor> FuTextAttrDlg::Create( ViewShell& rViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument& rDoc, SfxRequest& rReq )
{
    rtl::Reference<FuPoor> xFunc( new FuTextAttrDlg( rViewSh, pWin, pView, rDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuTextAttrDlg::DoExecute( SfxRequest& rReq )
{
    SfxItemSet aNewAttr( mrDoc.GetPool() );
    mpView->GetAttributes( aNewAttr );

    const SfxItemSet* pArgs = rReq.GetArgs();

    if( pArgs )
    {
        mpView->SetAttributes( *pArgs );
        return;
    }

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    VclPtr<SfxAbstractTabDialog> pDlg(pFact->CreateTextTabDialog(rReq.GetFrameWeld(), &aNewAttr, mpView));

    auto xRequest = std::make_shared<SfxRequest>(rReq);
    rReq.Ignore(); // the 'old' request is not relevant any more
    auto pView = mpView; // copy vars we need, FuTextAttrDlg object will be gone by the time the dialog completes
    pDlg->StartExecuteAsync(
        [pDlg, xRequest=std::move(xRequest), pView] (sal_Int32 nResult)->void
        {
            if (nResult == RET_OK)
            {
                xRequest->Done( *pDlg->GetOutputItemSet() );
                pView->SetAttributes( *xRequest->GetArgs() );
            }
            pDlg->disposeOnce();
        }
    );
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
