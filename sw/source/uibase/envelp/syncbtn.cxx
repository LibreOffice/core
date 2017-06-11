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

#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>

#include "cmdid.h"
#include "swmodule.hxx"
#include "view.hxx"
#include "edtwin.hxx"
#include "strings.hrc"

#include "syncbtn.hxx"
#include "swtypes.hxx"

SFX_IMPL_FLOATINGWINDOW( SwSyncChildWin, FN_SYNC_LABELS )

SwSyncChildWin::SwSyncChildWin( vcl::Window* _pParent,
                                sal_uInt16 nId,
                                SfxBindings* pBindings,
                                SfxChildWinInfo* pInfo ) :
                                SfxChildWindow( _pParent, nId )
{
    SetWindow(VclPtr<SwSyncBtnDlg>::Create( pBindings, this, _pParent));

    if (!pInfo->aSize.Width() || !pInfo->aSize.Height())
    {
        SwView* pActiveView = ::GetActiveView();
        if(pActiveView)
        {
            const SwEditWin &rEditWin = pActiveView->GetEditWin();
            GetWindow()->SetPosPixel(rEditWin.OutputToScreenPixel(Point(0, 0)));
        }
        else
            GetWindow()->SetPosPixel(_pParent->OutputToScreenPixel(Point(0, 0)));
        pInfo->aPos = GetWindow()->GetPosPixel();
        pInfo->aSize = GetWindow()->GetSizePixel();
    }

    static_cast<SwSyncBtnDlg *>(GetWindow())->Initialize(pInfo);

    GetWindow()->Show();
}

SwSyncBtnDlg::SwSyncBtnDlg( SfxBindings* _pBindings,
                            SfxChildWindow* pChild,
                            vcl::Window *pParent)
    : SfxFloatingWindow(_pBindings, pChild, pParent, "FloatingSync", "modules/swriter/ui/floatingsync.ui")
{
    get(m_pSyncBtn, "sync");
    m_pSyncBtn->SetClickHdl(LINK(this, SwSyncBtnDlg, BtnHdl));
    Show();
}

SwSyncBtnDlg::~SwSyncBtnDlg()
{
    disposeOnce();
}

void SwSyncBtnDlg::dispose()
{
    m_pSyncBtn.clear();
    SfxFloatingWindow::dispose();
}

IMPL_STATIC_LINK_NOARG(SwSyncBtnDlg, BtnHdl, Button*, void)
{
    SfxViewFrame::Current()->GetDispatcher()->Execute(FN_UPDATE_ALL_LINKS, SfxCallMode::ASYNCHRON);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
