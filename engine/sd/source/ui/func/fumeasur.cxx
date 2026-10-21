/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <fumeasur.hxx>
#include <sfx2/request.hxx>
#include <View.hxx>
#include <drawdoc.hxx>
#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>

#include <memory>

namespace sd {


FuMeasureDlg::FuMeasureDlg (
    ViewShell& rViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument& rDoc,
    SfxRequest& rReq)
    : FuPoor(rViewSh, pWin, pView, rDoc, rReq)
{
}

rtl::Reference<FuPoor> FuMeasureDlg::Create( ViewShell& rViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument& rDoc, SfxRequest& rReq )
{
    rtl::Reference<FuPoor> xFunc( new FuMeasureDlg( rViewSh, pWin, pView, rDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuMeasureDlg::DoExecute( SfxRequest& rReq )
{
    if (const SfxItemSet* pArgs = rReq.GetArgs())
    {
        mpView->SetAttributes( *pArgs );
        return;
    }

    // The dialog reads the attributes from the set while it runs, so the set lives as long as the
    // dialog does.
    auto xNewAttr = std::make_shared<SfxItemSet>(mrDoc.GetPool());
    mpView->GetAttributes( *xNewAttr );

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    VclPtr<SfxAbstractDialog> pDlg(
        pFact->CreateSfxDialog(rReq.GetFrameWeld(), *xNewAttr, mpView, RID_SVXPAGE_MEASURE));

    auto xRequest = std::make_shared<SfxRequest>(rReq);
    rtl::Reference<FuMeasureDlg> xThis(this); // the function is needed until the dialog ends
    pDlg->StartExecuteAsync(
        [pDlg, xNewAttr, xRequest = std::move(xRequest),
         xThis = std::move(xThis)](sal_Int32 nResult)
        {
            if (nResult == RET_OK)
            {
                xRequest->Done(*pDlg->GetOutputItemSet());
                xThis->mpView->SetAttributes(*xRequest->GetArgs());
            }
            pDlg->disposeOnce();
        });
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
