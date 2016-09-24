/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/inputdlg.hxx>

#include <sfx2/sfxresid.hxx>
#include <vcl/button.hxx>
#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>

InputDialog::InputDialog(const OUString &rLabelText, vcl::Window *pParent)
    : ModalDialog(pParent, "InputDialog", "sfx/ui/inputdialog.ui")
{
    get(m_pEntry, "entry");
    get(m_pLabel, "label");
    get(m_pOK, "ok");
    get(m_pCancel, "cancel");
    get(m_pHelp, "help");
    m_pLabel->SetText(rLabelText);
    m_pOK->SetClickHdl(LINK(this,InputDialog,ClickHdl));
    m_pCancel->SetClickHdl(LINK(this,InputDialog,ClickHdl));
}

InputDialog::~InputDialog()
{
    disposeOnce();
}

void InputDialog::dispose()
{
    m_pEntry.clear();
    m_pLabel.clear();
    m_pOK.clear();
    m_pCancel.clear();
    m_pHelp.clear();
    ModalDialog::dispose();
}

void InputDialog::HideHelpBtn()
{
    m_pHelp->Hide();
}

OUString InputDialog::GetEntryText() const
{
    return m_pEntry->GetText();
}

void InputDialog::SetEntryText( OUString const & sStr)
{
    m_pEntry->SetText(sStr);
    m_pEntry->SetCursorAtLast();
}

IMPL_LINK_TYPED(InputDialog,ClickHdl, Button*, pButton, void)
{
    EndDialog(pButton == m_pOK ? RET_OK : RET_CANCEL);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */


