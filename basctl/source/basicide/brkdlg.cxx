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

#include <sal/config.h>

#include "breakpoint.hxx"
#include "brkdlg.hxx"
#include <basobj.hxx>

#include <sfx2/dispatch.hxx>
#include <sfx2/sfxsids.hrc>
#include <svl/itemset.hxx>

namespace basctl
{
namespace
{
bool lcl_ParseText(OUString const& rText, size_t& rLineNr)
{
    // aText should look like "# n" where n > 0
    // All spaces are ignored, so there can even be spaces within the
    // number n.  (Maybe it would be better to ignore all whitespace instead
    // of just spaces.)
    OUString aText(rText.replaceAll(" ", ""));
    if (aText.isEmpty())
        return false;
    sal_Unicode cFirst = aText[0];
    if (cFirst != '#' && (cFirst < '0' || cFirst > '9'))
        return false;
    if (cFirst == '#')
        aText = aText.copy(1);
    sal_Int32 n = aText.toInt32();
    if (n <= 0)
        return false;
    rLineNr = static_cast<size_t>(n);
    return true;
}

} // namespace

BreakPointDialog::BreakPointDialog(weld::Window* pParent, BreakPointList& rBrkPntList)
    : GenericDialogController(pParent, u"modules/BasicIDE/ui/managebreakpoints.ui"_ustr,
                              u"ManageBreakpointsDialog"_ustr)
    , m_rOriginalBreakPointList(rBrkPntList)
    , m_aModifiedBreakPointList(rBrkPntList)
    , m_xComboBox(m_xBuilder->weld_entry_tree_view(u"entriesgrid"_ustr, u"entries"_ustr,
                                                   u"entrieslist"_ustr))
    , m_xOKButton(m_xBuilder->weld_button(u"ok"_ustr))
    , m_xNewButton(m_xBuilder->weld_button(u"new"_ustr))
    , m_xDelButton(m_xBuilder->weld_button(u"delete"_ustr))
    , m_xCheckBox(m_xBuilder->weld_check_button(u"active"_ustr))
    , m_xNumericField(m_xBuilder->weld_spin_button(u"pass"_ustr))
{
    m_xComboBox->set_size_request(m_xComboBox->get_approximate_digit_width() * 20, -1);
    m_xComboBox->set_height_request_by_rows(12);

    m_xComboBox->freeze();
    for (size_t i = 0, n = m_aModifiedBreakPointList.size(); i < n; ++i)
    {
        BreakPoint& rBrk = m_aModifiedBreakPointList.at(i);
        OUString aEntryStr("# " + OUString::number(rBrk.nLine));
        m_xComboBox->append_text(aEntryStr);
    }
    m_xComboBox->thaw();

    m_xOKButton->connect_clicked(LINK(this, BreakPointDialog, ButtonHdl));
    m_xNewButton->connect_clicked(LINK(this, BreakPointDialog, ButtonHdl));
    m_xDelButton->connect_clicked(LINK(this, BreakPointDialog, ButtonHdl));

    m_xCheckBox->connect_toggled(LINK(this, BreakPointDialog, CheckBoxHdl));
    m_xComboBox->connect_changed(LINK(this, BreakPointDialog, EditModifyHdl));
    m_xComboBox->connect_row_activated(LINK(this, BreakPointDialog, TreeModifyHdl));
    m_xComboBox->grab_focus();

    m_xNumericField->set_range(0, 0x7FFFFFFF);
    m_xNumericField->set_increments(1, 10);
    m_xNumericField->connect_value_changed(LINK(this, BreakPointDialog, FieldModifyHdl));

    if (m_xComboBox->get_count())
        m_xComboBox->set_active(0);

    if (m_aModifiedBreakPointList.size())
        UpdateFields(m_aModifiedBreakPointList.at(0));

    CheckButtons();
}

BreakPointDialog::~BreakPointDialog() {}

void BreakPointDialog::SetCurrentBreakPoint(BreakPoint const& rBrk)
{
    OUString aStr("# " + OUString::number(rBrk.nLine));
    m_xComboBox->set_entry_text(aStr);
    UpdateFields(rBrk);
}

void BreakPointDialog::CheckButtons()
{
    // "New" button is enabled if the combo box edit contains a valid line
    // number that is not already present in the combo box list; otherwise
    // "OK" and "Delete" buttons are enabled:
    size_t nLine;
    if (lcl_ParseText(m_xComboBox->get_active_text(), nLine)
        && m_aModifiedBreakPointList.FindBreakPoint(nLine) == nullptr)
    {
        m_xNewButton->set_sensitive(true);
        m_xOKButton->set_sensitive(false);
        m_xDelButton->set_sensitive(false);
        m_xDialog->change_default_widget(m_xDelButton.get(), m_xNewButton.get());
    }
    else
    {
        m_xNewButton->set_sensitive(false);
        m_xOKButton->set_sensitive(true);
        m_xDelButton->set_sensitive(true);
        m_xDialog->change_default_widget(m_xNewButton.get(), m_xDelButton.get());
    }
}

IMPL_LINK(BreakPointDialog, CheckBoxHdl, weld::Toggleable&, rButton, void)
{
    BreakPoint* pBrk = GetSelectedBreakPoint();
    if (pBrk)
        pBrk->bEnabled = rButton.get_active();
}

IMPL_LINK(BreakPointDialog, EditModifyHdl, weld::ComboBox&, rBox, void)
{
    CheckButtons();

    int nEntry = rBox.find_text(rBox.get_active_text());
    if (nEntry == -1)
        return;
    BreakPoint& rBrk = m_aModifiedBreakPointList.at(nEntry);
    UpdateFields(rBrk);
}

IMPL_LINK(BreakPointDialog, FieldModifyHdl, weld::SpinButton&, rEdit, void)
{
    BreakPoint* pBrk = GetSelectedBreakPoint();
    if (pBrk)
        pBrk->nStopAfter = rEdit.get_value();
}

IMPL_LINK_NOARG(BreakPointDialog, TreeModifyHdl, weld::TreeView&, bool)
{
    if (m_xDelButton->get_sensitive())
        ButtonHdl(*m_xDelButton);
    return true;
}

IMPL_LINK(BreakPointDialog, ButtonHdl, weld::Button&, rButton, void)
{
    if (&rButton == m_xOKButton.get())
    {
        m_rOriginalBreakPointList.transfer(m_aModifiedBreakPointList);
        m_xDialog->response(RET_OK);
    }
    else if (&rButton == m_xNewButton.get())
    {
        // keep checkbox in mind!
        OUString aText(m_xComboBox->get_active_text());
        size_t nLine;
        bool bValid = lcl_ParseText(aText, nLine);
        if (bValid)
        {
            BreakPoint aBrk(nLine);
            aBrk.bEnabled = m_xCheckBox->get_active();
            aBrk.nStopAfter = static_cast<size_t>(m_xNumericField->get_value());
            m_aModifiedBreakPointList.InsertSorted(aBrk);
            OUString aEntryStr("# " + OUString::number(aBrk.nLine));
            m_xComboBox->append_text(aEntryStr);
            if (SfxDispatcher* pDispatcher = GetDispatcher())
                pDispatcher->Execute(SID_BASICIDE_BRKPNTSCHANGED);
        }
        else
        {
            m_xComboBox->set_active_text(aText);
            m_xComboBox->grab_focus();
        }
        CheckButtons();
    }
    else if (&rButton == m_xDelButton.get())
    {
        int nEntry = m_xComboBox->find_text(m_xComboBox->get_active_text());
        if (nEntry != -1)
        {
            m_aModifiedBreakPointList.remove(nEntry);
            m_xComboBox->remove(nEntry);
            if (nEntry && nEntry >= m_xComboBox->get_count())
                nEntry--;
            m_xComboBox->set_active_text(m_xComboBox->get_text(nEntry));
            if (SfxDispatcher* pDispatcher = GetDispatcher())
                pDispatcher->Execute(SID_BASICIDE_BRKPNTSCHANGED);
            CheckButtons();
        }
    }
}

void BreakPointDialog::UpdateFields(BreakPoint const& rBrk)
{
    m_xCheckBox->set_active(rBrk.bEnabled);
    m_xNumericField->set_value(rBrk.nStopAfter);
}

BreakPoint* BreakPointDialog::GetSelectedBreakPoint()
{
    int nEntry = m_xComboBox->find_text(m_xComboBox->get_active_text());
    if (nEntry == -1)
        return nullptr;
    return &m_aModifiedBreakPointList.at(nEntry);
}

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
