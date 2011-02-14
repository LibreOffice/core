/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#include "protectiondlg.hxx"
#include "protectiondlg.hrc"
#include "scresid.hxx"
#include "tabprotection.hxx"

#include <vcl/msgbox.hxx>


// The order must match that of the list box.
static const ScTableProtection::Option aOptions[] = {
    ScTableProtection::SELECT_LOCKED_CELLS,
    ScTableProtection::SELECT_UNLOCKED_CELLS,
};
static const sal_uInt16 nOptionCount = sizeof(aOptions)/sizeof(aOptions[0]);


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

    maSelectLockedCells(ScResId(ST_SELECT_LOCKED_CELLS)),
    maSelectUnlockedCells(ScResId(ST_SELECT_UNLOCKED_CELLS))
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

IMPL_LINK( ScTableProtectionDlg, OKHdl, OKButton*, EMPTYARG )
{
    EndDialog(RET_OK);
    return 0;
}

IMPL_LINK( ScTableProtectionDlg, PasswordModifyHdl, Edit*, EMPTYARG )
{
    String aPass1 = maPassword1Edit.GetText();
    String aPass2 = maPassword2Edit.GetText();
    maBtnOk.Enable(aPass1.Equals(aPass2));
    return 0;
}
