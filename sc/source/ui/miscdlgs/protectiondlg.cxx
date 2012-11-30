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
#include "protectiondlg.hrc"
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


ScTableProtectionDlg::ScTableProtectionDlg(Window* pParent) :
    ModalDialog(pParent, ScResId(RID_SCDLG_TABPROTECTION)),

    maBtnProtect    (this, ScResId(BTN_PROTECT)),
    maPassword1Text (this, ScResId(FT_PASSWORD1)),
    maPassword1Edit (this, ScResId(ED_PASSWORD1)),
    maPassword2Text (this, ScResId(FT_PASSWORD2)),
    maPassword2Edit (this, ScResId(ED_PASSWORD2)),
    maOptionsLine   (this, ScResId(FL_OPTIONS)),
    maOptionsText   (this, ScResId(FT_OPTIONS)),
    maOptionsListBox(this, ScResId(CLB_OPTIONS)),

    maBtnOk     (this, ScResId(BTN_OK)),
    maBtnCancel (this, ScResId(BTN_CANCEL)),
    maBtnHelp   (this, ScResId(BTN_HELP)),

    maSelectLockedCells(ScResId(ST_SELECT_PROTECTED_CELLS)),
    maSelectUnlockedCells(ScResId(ST_SELECT_UNPROTECTED_CELLS))
{
    Init();
    FreeResource();
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
        maOptionsListBox.CheckEntryPos(i, rData.isOptionEnabled(aOptions[i]));
}

void ScTableProtectionDlg::WriteData(ScTableProtection& rData) const
{
    rData.setProtected(maBtnProtect.IsChecked());

    // We assume that the two password texts match.
    rData.setPassword(maPassword1Edit.GetText());

    for (sal_uInt16 i = 0; i < nOptionCount; ++i)
        rData.setOption(aOptions[i], maOptionsListBox.IsChecked(i));
}

void ScTableProtectionDlg::Init()
{
    Link aLink = LINK( this, ScTableProtectionDlg, CheckBoxHdl );
    maBtnProtect.SetClickHdl(aLink);

    aLink = LINK( this, ScTableProtectionDlg, OKHdl );
    maBtnOk.SetClickHdl(aLink);

    aLink = LINK( this, ScTableProtectionDlg, PasswordModifyHdl );
    maPassword1Edit.SetModifyHdl(aLink);
    maPassword2Edit.SetModifyHdl(aLink);

    maOptionsListBox.SetUpdateMode(false);
    maOptionsListBox.Clear();

    maOptionsListBox.InsertEntry(maSelectLockedCells);
    maOptionsListBox.InsertEntry(maSelectUnlockedCells);

    maOptionsListBox.CheckEntryPos(0, true);
    maOptionsListBox.CheckEntryPos(1, true);

    maOptionsListBox.SetUpdateMode(true);

    // Set the default state of the dialog.
    maBtnProtect.Check(true);
    maPassword1Edit.GrabFocus();
}

void ScTableProtectionDlg::EnableOptionalWidgets(bool bEnable)
{
    maPassword1Text.Enable(bEnable);
    maPassword1Edit.Enable(bEnable);
    maPassword2Text.Enable(bEnable);
    maPassword2Edit.Enable(bEnable);
    maOptionsLine.Enable(bEnable);
    maOptionsText.Enable(bEnable);

    maOptionsListBox.Enable(bEnable);
    maOptionsListBox.Invalidate();
}

IMPL_LINK( ScTableProtectionDlg, CheckBoxHdl, CheckBox*, pBtn )
{
    if (pBtn == &maBtnProtect)
    {
        bool bChecked = maBtnProtect.IsChecked();
        EnableOptionalWidgets(bChecked);
        maBtnOk.Enable(bChecked);
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
    String aPass1 = maPassword1Edit.GetText();
    String aPass2 = maPassword2Edit.GetText();
    maBtnOk.Enable(aPass1.Equals(aPass2));
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
