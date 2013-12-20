/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "inputdlg.hxx"

#include <sfx2/sfxresid.hxx>
#include <vcl/button.hxx>
#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>

InputDialog::InputDialog(const OUString &rLabelText, Window *pParent)
    : ModalDialog(pParent, "InputDialog", "sfx/ui/inputdialog.ui")
{
    get(m_pEntry, "entry");
    get(m_pLabel, "label");
    get(m_pOK, "ok");
    get(m_pCancel, "cancel");
    m_pLabel->SetText(rLabelText);
    m_pOK->SetClickHdl(LINK(this,InputDialog,ClickHdl));
    m_pCancel->SetClickHdl(LINK(this,InputDialog,ClickHdl));
}

OUString InputDialog::getEntryText() const
{
    return m_pEntry->GetText();
}

IMPL_LINK(InputDialog,ClickHdl,PushButton*, pButton)
{
    EndDialog(pButton == m_pOK ? true : false);
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */


