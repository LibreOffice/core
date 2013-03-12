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


#include "fucushow.hxx"

#include <svx/svxids.hrc>

#include "app.hrc"
#include "sdresid.hxx"
#include "ViewShell.hxx"
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include <vcl/msgbox.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>

#include "sdabstdlg.hxx"

namespace sd {

TYPEINIT1( FuCustomShowDlg, FuPoor );



FuCustomShowDlg::FuCustomShowDlg (
    ViewShell* pViewSh,
    ::sd::Window*    pWin,
    ::sd::View* pView,
    SdDrawDocument* pDoc,
    SfxRequest& rReq)
    : FuPoor( pViewSh, pWin, pView, pDoc, rReq )
{
}

FunctionReference FuCustomShowDlg::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    FunctionReference xFunc( new FuCustomShowDlg( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuCustomShowDlg::DoExecute( SfxRequest& )
{
    SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
    AbstractSdCustomShowDlg* pDlg = pFact ? pFact->CreateSdCustomShowDlg( NULL, *mpDoc ) : 0;
    if( pDlg )
    {
        sal_uInt16 nRet = pDlg->Execute();
        if( pDlg->IsModified() )
        {
            mpDoc->SetChanged( sal_True );
            sd::PresentationSettings& rSettings = mpDoc->getPresentationSettings();
            rSettings.mbCustomShow = pDlg->IsCustomShow();
        }
        delete pDlg;

        if( nRet == RET_YES )
        {
            mpViewShell->SetStartShowWithDialog();

            mpViewShell->GetViewFrame()->GetDispatcher()->Execute( SID_PRESENTATION,
                    SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD );
        }
    }
}

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
