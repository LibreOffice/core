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
    : GenericDialogController(pParent, "sfx/ui/inputdialog.ui", "InputDialog")
    , m_xEntry(m_xBuilder->weld_entry("entry"))
    , m_xLabel(m_xBuilder->weld_label("label"))
    , m_xLabelB(m_xBuilder->weld_label("labelB"))
    , m_xHelp(m_xBuilder->weld_button("help"))
    , m_xOk(m_xBuilder->weld_button("ok"))
{
    m_xLabel->set_label(rLabelText);
    m_xLabelB->hide();
    m_xEntry->connect_changed(LINK(this, InputDialog, EntryChangedHdl));
}

void InputDialog::HideHelpBtn() { m_xHelp->hide(); }

OUString InputDialog::GetEntryText() const { return m_xEntry->get_text(); }

void InputDialog::SetEntryText(const OUString& rStr)
{
    m_xEntry->set_text(rStr);
    m_xEntry->set_position(-1);
}

void InputDialog::SetLabelB(const OUString& rStr, const Color& rColor)
{
    m_xLabelB->set_label(rStr);
    m_xLabelB->set_font_color(rColor);
}

void InputDialog::SetEntryMessageType(weld::EntryMessageType aType)
{
    m_xEntry->set_message_type(aType);
    if (aType == weld::EntryMessageType::Error)
    {
        m_xEntry->select_region(0, -1);
        m_xEntry->grab_focus();
        m_xOk->set_sensitive(false);
        m_xLabelB->show();
    }
    else
    {
        m_xOk->set_sensitive(true);
        m_xLabelB->hide();
        SetTooltip("");
    }
}

void InputDialog::SetTooltip(const OUString& rStr)
{
    m_xEntry->set_tooltip_text(rStr);
    m_xOk->set_tooltip_text(rStr);
}

IMPL_LINK_NOARG(InputDialog, EntryChangedHdl, weld::Entry&, void)
{
    SetEntryMessageType(weld::EntryMessageType::Normal);
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
