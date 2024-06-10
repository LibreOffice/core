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
#include <vcl/windowstate.hxx>

#include <cmdid.h>
#include <swmodule.hxx>
#include <view.hxx>
#include <edtwin.hxx>

#include <syncbtn.hxx>

SFX_IMPL_MODELESSDIALOGCONTOLLER(SwSyncChildWin, FN_SYNC_LABELS)

SwSyncChildWin::SwSyncChildWin(vcl::Window* _pParent,
                               sal_uInt16 nId,
                               SfxBindings* pBindings,
                               SfxChildWinInfo* pInfo)
    : SfxChildWindow(_pParent, nId)
{
    SetController(std::make_shared<SwSyncBtnDlg>(pBindings, this, _pParent->GetFrameWeld()));
    SwSyncBtnDlg* pBtnDlg = static_cast<SwSyncBtnDlg*>(GetController().get());

    if (!pInfo->aSize.Width() || !pInfo->aSize.Height())
    {
        weld::Dialog* pDlg = pBtnDlg->getDialog();
        Point aPos;

        if (SwView* pActiveView = GetActiveView())
        {
            const SwEditWin &rEditWin = pActiveView->GetEditWin();
            aPos = rEditWin.OutputToScreenPixel(Point(0, 0));
        }
        else
            aPos = _pParent->OutputToScreenPixel(Point(0, 0));

        vcl::WindowData aState;
        aState.setMask(vcl::WindowDataMask::Pos);
        aState.setPos(aPos);
        pDlg->set_window_state(aState.toStr());

        pInfo->aPos = pDlg->get_position();
        pInfo->aSize = pDlg->get_size();
    }

    pBtnDlg->Initialize(pInfo);
}

SwSyncBtnDlg::SwSyncBtnDlg(SfxBindings* pBindings,
                           SfxChildWindow* pChild,
                           weld::Window *pParent)
    : SfxModelessDialogController(pBindings, pChild, pParent, u"modules/swriter/ui/floatingsync.ui"_ustr, u"FloatingSync"_ustr)
    , m_xSyncBtn(m_xBuilder->weld_button(u"sync"_ustr))
{
    m_xSyncBtn->connect_clicked(LINK(this, SwSyncBtnDlg, BtnHdl));
}

SwSyncBtnDlg::~SwSyncBtnDlg()
{
}

IMPL_STATIC_LINK_NOARG(SwSyncBtnDlg, BtnHdl, weld::Button&, void)
{
    if (SfxViewFrame* pViewFrm = SfxViewFrame::Current())
        pViewFrm->GetDispatcher()->Execute(FN_UPDATE_ALL_LINKS, SfxCallMode::ASYNCHRON);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
