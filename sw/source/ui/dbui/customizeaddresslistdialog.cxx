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

#include "customizeaddresslistdialog.hxx"
#include "createaddresslistdialog.hxx"

SwCustomizeAddressListDialog::SwCustomizeAddressListDialog(
        weld::Window* pParent, const SwCSVData& rOldData)
    : SfxDialogController(pParent, u"modules/swriter/ui/customizeaddrlistdialog.ui"_ustr,
                          u"CustomizeAddrListDialog"_ustr)
    , m_xNewData(new SwCSVData(rOldData))
    , m_xFieldsLB(m_xBuilder->weld_tree_view(u"treeview"_ustr))
    , m_xAddPB(m_xBuilder->weld_button(u"add"_ustr))
    , m_xDeletePB(m_xBuilder->weld_button(u"delete"_ustr))
    , m_xRenamePB(m_xBuilder->weld_button(u"rename"_ustr))
    , m_xUpPB(m_xBuilder->weld_button(u"up"_ustr))
    , m_xDownPB(m_xBuilder->weld_button(u"down"_ustr))
{
    m_xFieldsLB->set_size_request(-1, m_xFieldsLB->get_height_rows(14));

    m_xFieldsLB->connect_changed(LINK(this, SwCustomizeAddressListDialog, ListBoxSelectHdl_Impl));
    Link<weld::Button&,void> aAddRenameLk = LINK(this, SwCustomizeAddressListDialog, AddRenameHdl_Impl );
    m_xAddPB->connect_clicked(aAddRenameLk);
    m_xRenamePB->connect_clicked(aAddRenameLk);
    m_xDeletePB->connect_clicked(LINK(this, SwCustomizeAddressListDialog, DeleteHdl_Impl ));
    Link<weld::Button&,void> aUpDownLk = LINK(this, SwCustomizeAddressListDialog, UpDownHdl_Impl);
    m_xUpPB->connect_clicked(aUpDownLk);
    m_xDownPB->connect_clicked(aUpDownLk);

    for (const auto& rHeader : m_xNewData->aDBColumnHeaders)
        m_xFieldsLB->append_text(rHeader);

    m_xFieldsLB->select(0);
    UpdateButtons();
}

SwCustomizeAddressListDialog::~SwCustomizeAddressListDialog()
{
}

IMPL_LINK_NOARG(SwCustomizeAddressListDialog, ListBoxSelectHdl_Impl, weld::TreeView&, void)
{
    UpdateButtons();
}

IMPL_LINK(SwCustomizeAddressListDialog, AddRenameHdl_Impl, weld::Button&, rButton, void)
{
    bool bRename = &rButton == m_xRenamePB.get();
    auto nPos = m_xFieldsLB->get_selected_index();
    if (nPos == -1)
        nPos = 0;

    std::unique_ptr<SwAddRenameEntryDialog> xDlg;
    if (bRename)
        xDlg.reset(new SwRenameEntryDialog(m_xDialog.get(), m_xNewData->aDBColumnHeaders));
    else
        xDlg.reset(new SwAddEntryDialog(m_xDialog.get(), m_xNewData->aDBColumnHeaders));
    if (bRename)
    {
        OUString aTemp = m_xFieldsLB->get_text(nPos);
        xDlg->SetFieldName(aTemp);
    }
    if (xDlg->run() == RET_OK)
    {
        OUString sNew = xDlg->GetFieldName();
        if(bRename)
        {
            m_xNewData->aDBColumnHeaders[nPos] = sNew;
            m_xFieldsLB->remove(nPos);
        }
        else
        {
            if (m_xFieldsLB->get_selected_index() != -1)
                ++nPos; // append the new entry behind the selected
            //add the new column
            m_xNewData->aDBColumnHeaders.insert(m_xNewData->aDBColumnHeaders.begin() + nPos, sNew);
            //add a new entry into all data arrays
            for (auto& rData : m_xNewData->aDBData)
                rData.insert(rData.begin() + nPos, OUString());

        }

        m_xFieldsLB->insert_text(nPos, sNew);
        m_xFieldsLB->select(nPos);
    }
    UpdateButtons();
}

IMPL_LINK_NOARG(SwCustomizeAddressListDialog, DeleteHdl_Impl, weld::Button&, void)
{
    auto nPos = m_xFieldsLB->get_selected_index();
    m_xFieldsLB->remove(nPos);
    m_xFieldsLB->select(nPos > m_xFieldsLB->n_children() - 1 ? nPos - 1 : nPos);

    //remove the column
    m_xNewData->aDBColumnHeaders.erase(m_xNewData->aDBColumnHeaders.begin() + nPos);
    //remove the data
    for (auto& rData : m_xNewData->aDBData)
        rData.erase(rData.begin() + nPos);

    UpdateButtons();
}

IMPL_LINK(SwCustomizeAddressListDialog, UpDownHdl_Impl, weld::Button&, rButton, void)
{
    auto nPos = m_xFieldsLB->get_selected_index();
    auto nOldPos = nPos;
    OUString aTemp = m_xFieldsLB->get_text(nPos);
    m_xFieldsLB->remove(nPos);
    if (&rButton == m_xUpPB.get())
        --nPos;
    else
        ++nPos;
    m_xFieldsLB->insert_text(nPos, aTemp);
    m_xFieldsLB->select(nPos);
    //align m_xNewData
    OUString sHeader = m_xNewData->aDBColumnHeaders[nOldPos];
    m_xNewData->aDBColumnHeaders.erase(m_xNewData->aDBColumnHeaders.begin() + nOldPos);
    m_xNewData->aDBColumnHeaders.insert(m_xNewData->aDBColumnHeaders.begin() + nPos, sHeader);
    for (auto& rData : m_xNewData->aDBData)
    {
        OUString sData = rData[nOldPos];
        rData.erase(rData.begin() + nOldPos);
        rData.insert(rData.begin() + nPos, sData);
    }

    UpdateButtons();
}

void SwCustomizeAddressListDialog::UpdateButtons()
{
    auto nPos = m_xFieldsLB->get_selected_index();
    auto nEntries = m_xFieldsLB->n_children();
    m_xUpPB->set_sensitive(nPos > 0 && nEntries > 0);
    m_xDownPB->set_sensitive(nPos < nEntries -1);
    m_xDeletePB->set_sensitive(nEntries > 0);
    m_xRenamePB->set_sensitive(nEntries > 0);
}

SwAddRenameEntryDialog::SwAddRenameEntryDialog(
        weld::Window* pParent, const OUString& rUIXMLDescription, const OUString& rID,
        const std::vector< OUString >& rCSVHeader)
    : SfxDialogController(pParent, rUIXMLDescription, rID)
    , m_rCSVHeader(rCSVHeader)
    , m_xFieldNameED(m_xBuilder->weld_entry(u"entry"_ustr))
    , m_xOK(m_xBuilder->weld_button(u"ok"_ustr))
{
    m_xFieldNameED->connect_changed(LINK(this, SwAddRenameEntryDialog, ModifyHdl_Impl));
    ModifyHdl_Impl(*m_xFieldNameED);
}

IMPL_LINK(SwAddRenameEntryDialog, ModifyHdl_Impl, weld::Entry&, rEdit, void)
{
    OUString sEntry = rEdit.get_text();
    bool bFound = sEntry.isEmpty();

    if(!bFound)
    {
        bFound = std::any_of(m_rCSVHeader.begin(), m_rCSVHeader.end(),
            [&sEntry](const OUString& rHeader) { return rHeader == sEntry; });
    }
    m_xOK->set_sensitive(!bFound);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
