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
    , m_xHelp(m_xBuilder->weld_button("help"))
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
