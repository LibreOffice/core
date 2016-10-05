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

#include <swmodule.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <gotodlg.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <cmdid.h>

using namespace com::sun::star;

SwGotoPageDlg::SwGotoPageDlg( vcl::Window* pParent, SfxBindings* _pBindings):
        ModalDialog(pParent, "GotoPageDialog", "modules/swriter/ui/gotopagedialog.ui"),
        m_pCreateView(nullptr),
        m_rBindings(_pBindings),
        mnMaxPageCnt(1)
{
    get(mpMtrPageCtrl, "page");
    get(mpPageNumberLbl, "page_count");

    sal_uInt16 nTotalPage = GetPageInfo();

    if(nTotalPage)
    {
        OUString sStr = mpPageNumberLbl->GetText();
        mpPageNumberLbl->SetText(sStr.replaceFirst("$1", OUString::number(nTotalPage)));
        mnMaxPageCnt = nTotalPage;
    }
    mpMtrPageCtrl->SetModifyHdl(LINK(this, SwGotoPageDlg, PageModifiedHdl));
    mpMtrPageCtrl->SetCursorAtLast();
    Selection aSel(0, EDIT_NOLIMIT);
    mpMtrPageCtrl->SetSelection(aSel);
}

SwGotoPageDlg::~SwGotoPageDlg()
{
    disposeOnce();
}

void SwGotoPageDlg::dispose()
{
    mpMtrPageCtrl.clear();
    mpPageNumberLbl.clear();

    ModalDialog::dispose();
}

IMPL_LINK_NOARG(SwGotoPageDlg, PageModifiedHdl, Edit&, void)
{
    if(!(mpMtrPageCtrl->GetText()).isEmpty() )
    {
        int page_value = (mpMtrPageCtrl->GetText()).toInt32();

        if(page_value <= 0.0)
            mpMtrPageCtrl->SetText(OUString::number(1));
        else if(page_value > mnMaxPageCnt)
            mpMtrPageCtrl->SetText(OUString::number(mnMaxPageCnt));

        mpMtrPageCtrl->SetCursorAtLast();
    }
}

SwView*  SwGotoPageDlg::GetCreateView() const
{
    if(!m_pCreateView)
    {
        SwView* pView = SwModule::GetFirstView();
        while(pView)
        {
            if(&pView->GetViewFrame()->GetBindings() == m_rBindings)
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

sal_uInt16 SwGotoPageDlg::GetPageInfo()
{
    SwView *pView = GetCreateView();
    SwWrtShell *pSh = pView ? &pView->GetWrtShell() : nullptr;
    mpMtrPageCtrl->SetText(OUString::number(1));
    if (pSh)
    {
        const sal_uInt16 nPageCnt = pSh->GetPageCnt();
        sal_uInt16 nPhyPage, nVirPage;
        pSh->GetPageNum(nPhyPage, nVirPage);
        mpMtrPageCtrl->SetText(OUString::number(nPhyPage));
        return nPageCnt;
    }
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */