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

#include <vcl/field.hxx>
#include <swmodule.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <gotodlg.hxx>
#include <edtwin.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>
#include <cmdid.h>

SwGotoPageDlg::SwGotoPageDlg( vcl::Window* pParent, SfxBindings* _pBindings):
        ModalDialog(pParent, "GotoPageDialog", "modules/swriter/ui/gotopagedialog.ui"),
        m_pCreateView(nullptr),
        m_rBindings(*_pBindings)
{
    get(mpMtrPageCtrl, "page");

    GetCreateView();
    UsePage();

    mpMtrPageCtrl->SetGetFocusHdl(LINK(this, SwGotoPageDlg, EditGetFocus));
    mpMtrPageCtrl->SetUpHdl(LINK(this, SwGotoPageDlg, PageEditModifyHdl));
    mpMtrPageCtrl->SetDownHdl(LINK(this, SwGotoPageDlg, PageEditModifyHdl));

    m_aPageChgIdle.SetIdleHdl(LINK(this, SwGotoPageDlg, ChangePageHdl));
    m_aPageChgIdle.SetPriority(SchedulerPriority::LOWEST);
}

SwGotoPageDlg::~SwGotoPageDlg()
{
    disposeOnce();
}

void SwGotoPageDlg::dispose()
{
    mpMtrPageCtrl.clear();
    m_aPageChgIdle.Stop();

    ModalDialog::dispose();
}

SwView*  SwGotoPageDlg::GetCreateView() const
{
    if(!m_pCreateView)
    {
        SwView* pView = SwModule::GetFirstView();
        while(pView)
        {
            if(&pView->GetViewFrame()->GetBindings() == &m_rBindings)
            {
                const_cast<SwGotoPageDlg*>(this)->m_pCreateView = pView;
                break;
            }
            pView = SwModule::GetNextView(pView);
        }
    }

    return m_pCreateView;
}

// If the page can be set here, the maximum is set.

IMPL_LINK_NOARG_TYPED( SwGotoPageDlg, EditGetFocus, Control&, void )
{
    SwView *pView = GetCreateView();
    if (!pView)
        return;
    SwWrtShell &rSh = pView->GetWrtShell();

    const sal_uInt16 nPageCnt = rSh.GetPageCnt();
    mpMtrPageCtrl->SetMax(nPageCnt);
    mpMtrPageCtrl->SetLast(nPageCnt);
}


IMPL_LINK_NOARG_TYPED(SwGotoPageDlg, PageEditModifyHdl, SpinField&, void)
{
    if(m_aPageChgIdle.IsActive())
        m_aPageChgIdle.Stop();
    m_aPageChgIdle.Start();
}

IMPL_LINK_NOARG_TYPED(SwGotoPageDlg, ChangePageHdl, Idle *, void)
{
    if (!IsDisposed())
    {
        EditActionHdl();
        mpMtrPageCtrl->GrabFocus();
    }
}

// Action-Handler Edit:
// Switches to the page if the structure view is not turned on.

void SwGotoPageDlg::EditActionHdl()
{
    SwView *pView = GetCreateView();
    if (pView)
    {
        if(m_aPageChgIdle.IsActive())
            m_aPageChgIdle.Stop();
        pView->GetWrtShell().GotoPage((sal_uInt16)mpMtrPageCtrl->GetValue(), true);
        pView->GetEditWin().GrabFocus();
        m_rBindings.Invalidate(FN_STAT_PAGE);
    }
}

void SwGotoPageDlg::UsePage()
{
    SwView *pView = GetCreateView();
    SwWrtShell *pSh = pView ? &pView->GetWrtShell() : nullptr;
    mpMtrPageCtrl->SetValue(1);
    if (pSh)
    {
        const sal_uInt16 nPageCnt = pSh->GetPageCnt();
        sal_uInt16 nPhyPage, nVirPage;
        pSh->GetPageNum(nPhyPage, nVirPage);

        mpMtrPageCtrl->SetMax(nPageCnt);
        mpMtrPageCtrl->SetLast(nPageCnt);
        mpMtrPageCtrl->SetValue(nPhyPage);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */