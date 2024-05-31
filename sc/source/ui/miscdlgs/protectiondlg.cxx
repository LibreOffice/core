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

#include <protectiondlg.hxx>
#include <tabprotection.hxx>
#include <svl/PasswordHelper.hxx>

#include <vector>

namespace {

// The order must match that of the list box.
const std::vector<ScTableProtection::Option> aOptions = {
    ScTableProtection::SELECT_LOCKED_CELLS,
    ScTableProtection::SELECT_UNLOCKED_CELLS,
    ScTableProtection::INSERT_COLUMNS,
    ScTableProtection::INSERT_ROWS,
    ScTableProtection::DELETE_COLUMNS,
    ScTableProtection::DELETE_ROWS,
};

}

ScTableProtectionDlg::ScTableProtectionDlg(weld::Window* pParent)
    : weld::GenericDialogController(pParent, u"modules/scalc/ui/protectsheetdlg.ui"_ustr, u"ProtectSheetDialog"_ustr)
    , m_xBtnProtect(m_xBuilder->weld_check_button(u"protect"_ustr))
    , m_xPasswords(m_xBuilder->weld_container(u"passwords"_ustr))
    , m_xOptions(m_xBuilder->weld_container(u"options"_ustr))
    , m_xPassword1Edit(m_xBuilder->weld_entry(u"password1"_ustr))
    , m_xPassword2Edit(m_xBuilder->weld_entry(u"password2"_ustr))
    , m_xPasswordStrengthBar(m_xBuilder->weld_level_bar(u"passwordbar"_ustr))
    , m_xOptionsListBox(m_xBuilder->weld_tree_view(u"checklist"_ustr))
    , m_xBtnOk(m_xBuilder->weld_button(u"ok"_ustr))
    , m_xProtected(m_xBuilder->weld_label(u"protected"_ustr))
    , m_xUnprotected(m_xBuilder->weld_label(u"unprotected"_ustr))
    , m_xInsertColumns(m_xBuilder->weld_label(u"insert-columns"_ustr))
    , m_xInsertRows(m_xBuilder->weld_label(u"insert-rows"_ustr))
    , m_xDeleteColumns(m_xBuilder->weld_label(u"delete-columns"_ustr))
    , m_xDeleteRows(m_xBuilder->weld_label(u"delete-rows"_ustr))
{
    m_aSelectLockedCells = m_xProtected->get_label();
    m_aSelectUnlockedCells = m_xUnprotected->get_label();
    m_aInsertColumns = m_xInsertColumns->get_label();
    m_aInsertRows = m_xInsertRows->get_label();
    m_aDeleteColumns = m_xDeleteColumns->get_label();
    m_aDeleteRows = m_xDeleteRows->get_label();

    m_xOptionsListBox->enable_toggle_buttons(weld::ColumnToggleType::Check);

    Init();
}

ScTableProtectionDlg::~ScTableProtectionDlg()
{
}

void ScTableProtectionDlg::SetDialogData(const ScTableProtection& rData)
{
    for (size_t i = 0; i < aOptions.size(); ++i)
        m_xOptionsListBox->set_toggle(i, rData.isOptionEnabled(aOptions[i]) ? TRISTATE_TRUE : TRISTATE_FALSE);
}

void ScTableProtectionDlg::WriteData(ScTableProtection& rData) const
{
    rData.setProtected(m_xBtnProtect->get_active());

    // We assume that the two password texts match.
    rData.setPassword(m_xPassword1Edit->get_text());

    for (size_t i = 0; i < aOptions.size(); ++i)
        rData.setOption(aOptions[i], m_xOptionsListBox->get_toggle(i) == TRISTATE_TRUE);
}

void ScTableProtectionDlg::InsertEntry(const OUString& rTxt)
{
    m_xOptionsListBox->append();
    const int nRow = m_xOptionsListBox->n_children() - 1;
    m_xOptionsListBox->set_toggle(nRow, TRISTATE_FALSE);
    m_xOptionsListBox->set_text(nRow, rTxt, 0);
}

void ScTableProtectionDlg::Init()
{
    m_xBtnProtect->connect_toggled(LINK(this, ScTableProtectionDlg, CheckBoxHdl));

    m_xBtnOk->connect_clicked(LINK(this, ScTableProtectionDlg, OKHdl));

    Link<weld::Entry&,void> aLink = LINK(this, ScTableProtectionDlg, PasswordModifyHdl);
    m_xPassword1Edit->connect_changed(aLink);
    m_xPassword2Edit->connect_changed(aLink);

    m_xOptionsListBox->freeze();
    m_xOptionsListBox->clear();

    InsertEntry(m_aSelectLockedCells);
    InsertEntry(m_aSelectUnlockedCells);
    InsertEntry(m_aInsertColumns);
    InsertEntry(m_aInsertRows);
    InsertEntry(m_aDeleteColumns);
    InsertEntry(m_aDeleteRows);

    m_xOptionsListBox->set_toggle(0, TRISTATE_TRUE);
    m_xOptionsListBox->set_toggle(1, TRISTATE_TRUE);

    m_xOptionsListBox->thaw();

    // Set the default state of the dialog.
    m_xBtnProtect->set_active(true);
    m_xPassword1Edit->grab_focus();
}

void ScTableProtectionDlg::EnableOptionalWidgets(bool bEnable)
{
    m_xPasswords->set_sensitive(bEnable);
    m_xOptions->set_sensitive(bEnable);
}

IMPL_LINK(ScTableProtectionDlg, CheckBoxHdl, weld::Toggleable&, rBtn, void)
{
    if (&rBtn == m_xBtnProtect.get())
    {
        bool bChecked = m_xBtnProtect->get_active();
        EnableOptionalWidgets(bChecked);
        m_xBtnOk->set_sensitive(bChecked);
    }
}

IMPL_LINK_NOARG(ScTableProtectionDlg, OKHdl, weld::Button&, void)
{
    m_xDialog->response(RET_OK);
}

IMPL_LINK(ScTableProtectionDlg, PasswordModifyHdl, weld::Entry&, rEntry, void)
{
    OUString aPass1 = m_xPassword1Edit->get_text();
    if (&rEntry == m_xPassword1Edit.get())
    {
        m_xPasswordStrengthBar->set_percentage(
            SvPasswordHelper::GetPasswordStrengthPercentage(aPass1));
    }
    OUString aPass2 = m_xPassword2Edit->get_text();
    m_xBtnOk->set_sensitive(aPass1 == aPass2);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
