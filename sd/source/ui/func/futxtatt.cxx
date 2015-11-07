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

#include "futxtatt.hxx"
#include <vcl/msgbox.hxx>
#include <sfx2/request.hxx>

#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>
#include "View.hxx"
#include "ViewShell.hxx"
#include "drawdoc.hxx"
#include <memory>

namespace sd {


FuTextAttrDlg::FuTextAttrDlg (
    ViewShell* pViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument* pDoc,
    SfxRequest& rReq)
    : FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{
}

rtl::Reference<FuPoor> FuTextAttrDlg::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    rtl::Reference<FuPoor> xFunc( new FuTextAttrDlg( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuTextAttrDlg::DoExecute( SfxRequest& rReq )
{
    SfxItemSet aNewAttr( mpDoc->GetPool() );
    mpView->GetAttributes( aNewAttr );

    const SfxItemSet* pArgs = rReq.GetArgs();

    if( !pArgs )
    {
        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        std::unique_ptr<SfxAbstractTabDialog> pDlg(pFact->CreateTextTabDialog( NULL, &aNewAttr, mpView ));

        sal_uInt16 nResult = pDlg->Execute();

        switch( nResult )
        {
            case RET_OK:
            {
                rReq.Done( *( pDlg->GetOutputItemSet() ) );

                pArgs = rReq.GetArgs();
            }
            break;

            default:
            return; // Cancel
        }
    }
    mpView->SetAttributes( *pArgs );
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
