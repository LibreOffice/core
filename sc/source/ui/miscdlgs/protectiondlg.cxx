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


#include "protectiondlg.hxx"
#include "scresid.hxx"
#include "tabprotection.hxx"

#include <sal/macros.h>
#include <vcl/msgbox.hxx>


// The order must match that of the list box.
static const ScTableProtection::Option aOptions[] = {
    ScTableProtection::SELECT_LOCKED_CELLS,
    ScTableProtection::SELECT_UNLOCKED_CELLS,
};
static const sal_uInt16 nOptionCount = sizeof(aOptions) / sizeof (aOptions[0]);


ScTableProtectionDlg::ScTableProtectionDlg(Window* pParent)
    : ModalDialog( pParent, "ProtectSheetDialog", "modules/scalc/ui/protectsheetdlg.ui" )
{
    get(m_pPasswords, "passwords");
    get(m_pOptions, "options");
    get(m_pBtnProtect, "protect");
    get(m_pOptionsListBox, "checklist");
    get(m_pPassword1Edit, "password1");
    get(m_pPassword2Edit, "password2");
    get(m_pBtnOk, "ok");

    m_aSelectLockedCells = get<FixedText>("protected")->GetText();
    m_aSelectUnlockedCells = get<FixedText>("unprotected")->GetText();

    Init();
}

ScTableProtectionDlg::~ScTableProtectionDlg()
{
}

short ScTableProtectionDlg::Execute()
{
    return ModalDialog::Execute();
}

void ScTableProtectionDlg::SetDialogData(const ScTableProtection& rData)
{
    for (sal_uInt16 i = 0; i < nOptionCount; ++i)
        m_pOptionsListBox->CheckEntryPos(i, rData.isOptionEnabled(aOptions[i]));
}

void ScTableProtectionDlg::WriteData(ScTableProtection& rData) const
{
    rData.setProtected(m_pBtnProtect->IsChecked());

    // We assume that the two password texts match.
    rData.setPassword(m_pPassword1Edit->GetText());

    for (sal_uInt16 i = 0; i < nOptionCount; ++i)
        rData.setOption(aOptions[i], m_pOptionsListBox->IsChecked(i));
}

void ScTableProtectionDlg::Init()
{
    Link aLink = LINK( this, ScTableProtectionDlg, CheckBoxHdl );
    m_pBtnProtect->SetClickHdl(aLink);

    aLink = LINK( this, ScTableProtectionDlg, OKHdl );
    m_pBtnOk->SetClickHdl(aLink);

    aLink = LINK( this, ScTableProtectionDlg, PasswordModifyHdl );
    m_pPassword1Edit->SetModifyHdl(aLink);
    m_pPassword2Edit->SetModifyHdl(aLink);

    m_pOptionsListBox->SetUpdateMode(false);
    m_pOptionsListBox->Clear();

    m_pOptionsListBox->InsertEntry(m_aSelectLockedCells);
    m_pOptionsListBox->InsertEntry(m_aSelectUnlockedCells);

    m_pOptionsListBox->CheckEntryPos(0, true);
    m_pOptionsListBox->CheckEntryPos(1, true);

    m_pOptionsListBox->SetUpdateMode(true);

    // Set the default state of the dialog.
    m_pBtnProtect->Check(true);
    m_pPassword1Edit->GrabFocus();
}

void ScTableProtectionDlg::EnableOptionalWidgets(bool bEnable)
{
    m_pPasswords->Enable(bEnable);
    m_pOptions->Enable(bEnable);
    m_pOptionsListBox->Invalidate();
}

IMPL_LINK( ScTableProtectionDlg, CheckBoxHdl, CheckBox*, pBtn )
{
    if (pBtn == m_pBtnProtect)
    {
        bool bChecked = m_pBtnProtect->IsChecked();
        EnableOptionalWidgets(bChecked);
        m_pBtnOk->Enable(bChecked);
    }

    return 0;
}

IMPL_LINK_NOARG(ScTableProtectionDlg, OKHdl)
{
    EndDialog(RET_OK);
    return 0;
}

IMPL_LINK_NOARG(ScTableProtectionDlg, PasswordModifyHdl)
{
    OUString aPass1 = m_pPassword1Edit->GetText();
    OUString aPass2 = m_pPassword2Edit->GetText();
    m_pBtnOk->Enable(aPass1 == aPass2);
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
