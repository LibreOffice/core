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

#include <comphelper/string.hxx>

#include <svl/style.hxx>

#include <sfx2/newstyle.hxx>
#include <sfx2/strings.hrc>
#include <sfx2/sfxresid.hxx>
#include <vcl/layout.hxx>
#include <vcl/weld.hxx>

// Private methods ------------------------------------------------------

IMPL_LINK_NOARG(SfxNewStyleDlg, OKClickHdl, weld::Button&, void)
{
    const OUString aName(m_xEntry->get_text());
    SfxStyleSheetBase* pStyle = m_rPool.Find(aName, m_rPool.GetSearchFamily());
    if ( pStyle )
    {
        if ( !pStyle->IsUserDefined() )
        {
            std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                                     VclMessageType::Info, VclButtonsType::Ok,
                                                                     SfxResId(STR_POOL_STYLE_NAME)));
            xBox->run();
            return;
        }

        if (RET_YES == m_xQueryOverwriteBox->run())
            m_xDialog->response(RET_OK);
    }
    else
        m_xDialog->response(RET_OK);
}

IMPL_LINK(SfxNewStyleDlg, OKHdl, weld::TreeView&, rView, void)
{
    m_xEntry->set_text(rView.get_selected_text());
    OKClickHdl(*m_xOKBtn);
}

IMPL_LINK(SfxNewStyleDlg, ClickHdl, weld::TreeView&, rView, void)
{
    m_xEntry->set_text(rView.get_selected_text());
}

IMPL_LINK(SfxNewStyleDlg, ModifyHdl, weld::Entry&, rBox, void)
{
    OUString sText(rBox.get_text());
    m_xOKBtn->set_sensitive(!sText.replaceAll(" ", "").isEmpty());
    int nExists = m_xColBox->find_text(sText);
    if (nExists != -1)
    {
        m_xColBox->select(nExists);
        return;
    }

    m_xColBox->select(-1);
    if (sText.isEmpty())
        return;

    int nCount = m_xColBox->n_children();
    for (int i = 0; i < nCount; ++i)
    {
        if (m_xColBox->get_text(i).startsWith(sText))
        {
            m_xColBox->select(i);
            break;
        }
    }
}

SfxNewStyleDlg::SfxNewStyleDlg(weld::Window* pParent, SfxStyleSheetBasePool& rInPool)
    : GenericDialogController(pParent, "sfx/ui/newstyle.ui", "CreateStyleDialog")
    , m_rPool(rInPool)
    , m_xEntry(m_xBuilder->weld_entry("entry"))
    , m_xColBox(m_xBuilder->weld_tree_view("styles"))
    , m_xOKBtn(m_xBuilder->weld_button("ok"))
    , m_xQueryOverwriteBox(Application::CreateMessageDialog(m_xDialog.get(), VclMessageType::Question, VclButtonsType::YesNo,
                                                                           SfxResId(STR_QUERY_OVERWRITE)))
{
    m_xColBox->set_size_request(m_xColBox->get_approximate_digit_width() * 20,
                                m_xColBox->get_height_rows(8));

    m_xOKBtn->connect_clicked(LINK(this, SfxNewStyleDlg, OKClickHdl));
    m_xEntry->connect_changed(LINK(this, SfxNewStyleDlg, ModifyHdl));
    m_xColBox->connect_row_activated(LINK(this, SfxNewStyleDlg, OKHdl));
    m_xColBox->connect_changed(LINK(this, SfxNewStyleDlg, ClickHdl));

    SfxStyleSheetBase *pStyle = m_rPool.First();
    while (pStyle)
    {
        m_xColBox->append_text(pStyle->GetName());
        pStyle = m_rPool.Next();
    }
}

SfxNewStyleDlg::~SfxNewStyleDlg()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
