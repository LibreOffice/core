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

#include <svl/style.hxx>

#include <sfx2/app.hxx>
#include <sfx2/module.hxx>
#include <sfx2/newstyle.hxx>
#include <sfx2/strings.hrc>
#include <sfx2/sfxresid.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>

// Private methods ------------------------------------------------------

IMPL_LINK_NOARG(SfxNewStyleDlg, OKClickHdl, weld::Button&, void)
{
    const OUString aName(m_xColBox->get_active_text());
    SfxStyleSheetBase* pStyle = m_rPool.Find(aName, m_eSearchFamily);
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

IMPL_LINK_NOARG(SfxNewStyleDlg, OKHdl, weld::TreeView&, bool)
{
    OKClickHdl(*m_xOKBtn);
    return true;
}

IMPL_LINK(SfxNewStyleDlg, ModifyHdl, weld::ComboBox&, rBox, void)
{
    m_xOKBtn->set_sensitive(!rBox.get_active_text().replaceAll(" ", "").isEmpty());
}

SfxNewStyleDlg::SfxNewStyleDlg(weld::Widget* pParent, SfxStyleSheetBasePool& rInPool, SfxStyleFamily eFam)
    : GenericDialogController(pParent, "sfx/ui/newstyle.ui", "CreateStyleDialog")
    , m_rPool(rInPool)
    , m_eSearchFamily(eFam)
    , m_xColBox(m_xBuilder->weld_entry_tree_view("stylegrid", "stylename", "styles"))
    , m_xLabel(m_xBuilder->weld_label("categorylabel"))
    , m_xOKBtn(m_xBuilder->weld_button("ok"))
    , m_xQueryOverwriteBox(Application::CreateMessageDialog(m_xDialog.get(), VclMessageType::Question, VclButtonsType::YesNo,
                                                                           SfxResId(STR_QUERY_OVERWRITE)))
{
    m_xColBox->set_entry_width_chars(20);
    m_xColBox->set_height_request_by_rows(8);

    auto xFamilies = SfxApplication::GetModule_Impl()->CreateStyleFamilies();
    for (size_t i = 0, nCount = xFamilies->size(); i < nCount; ++i)
    {
        if (eFam == (*xFamilies)[i].GetFamily())
        {
            m_xLabel->set_label((*xFamilies)[i].GetText());
            m_xLabel->show();
            break;
        }
    }

    m_xOKBtn->connect_clicked(LINK(this, SfxNewStyleDlg, OKClickHdl));
    m_xColBox->connect_changed(LINK(this, SfxNewStyleDlg, ModifyHdl));
    m_xColBox->connect_row_activated(LINK(this, SfxNewStyleDlg, OKHdl));

    auto xIter = m_rPool.CreateIterator(eFam, SfxStyleSearchBits::UserDefined);
    SfxStyleSheetBase *pStyle = xIter->First();
    while (pStyle)
    {
        m_xColBox->append_text(pStyle->GetName());
        pStyle = xIter->Next();
    }
}

SfxNewStyleDlg::~SfxNewStyleDlg()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
