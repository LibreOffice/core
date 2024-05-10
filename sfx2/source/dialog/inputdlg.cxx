/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/inputdlg.hxx>

InputDialog::InputDialog(weld::Widget* pParent, const OUString& rLabelText)
    : GenericDialogController(pParent, u"sfx/ui/inputdialog.ui"_ustr, u"InputDialog"_ustr)
    , m_xEntry(m_xBuilder->weld_entry(u"entry"_ustr))
    , m_xLabel(m_xBuilder->weld_label(u"label"_ustr))
    , m_xHelp(m_xBuilder->weld_button(u"help"_ustr))
    , m_xOk(m_xBuilder->weld_button(u"ok"_ustr))
{
    m_xLabel->set_label(rLabelText);
}

void InputDialog::HideHelpBtn() { m_xHelp->hide(); }

OUString InputDialog::GetEntryText() const { return m_xEntry->get_text(); }

void InputDialog::SetEntryText(const OUString& rStr)
{
    m_xEntry->set_text(rStr);
    m_xEntry->set_position(-1);
}

void InputDialog::SetEntryMessageType(weld::EntryMessageType aType)
{
    m_xEntry->set_message_type(aType);
    if (aType == weld::EntryMessageType::Error)
    {
        m_xEntry->select_region(0, -1);
        m_xEntry->grab_focus();
        m_xOk->set_sensitive(false);
    }
    else
    {
        m_xOk->set_sensitive(true);
        SetTooltip(u""_ustr);
    }
}

void InputDialog::SetTooltip(const OUString& rStr)
{
    m_xEntry->set_tooltip_text(rStr);
    m_xOk->set_tooltip_text(rStr);
}

void InputDialog::setCheckEntry(const std::function<bool(OUString)>& rFunc)
{
    mCheckEntry = rFunc;
    m_xEntry->connect_changed(LINK(this, InputDialog, EntryChangedHdl));
}

IMPL_LINK_NOARG(InputDialog, EntryChangedHdl, weld::Entry&, void)
{
    if (mCheckEntry(m_xEntry->get_text()))
        SetEntryMessageType(weld::EntryMessageType::Normal);
    else
        SetEntryMessageType(weld::EntryMessageType::Error);
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
