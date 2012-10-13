/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


#include "retypepassdlg.hxx"
#include "retypepassdlg.hrc"
#include "scresid.hxx"
#include "document.hxx"
#include "tabprotection.hxx"

#include <stdio.h>

#include <vcl/msgbox.hxx>

ScRetypePassDlg::ScRetypePassDlg(Window* pParent) :
    ModalDialog(pParent, ScResId(RID_SCDLG_RETYPEPASS)),

    maBtnOk     (this, ScResId(BTN_OK)),
    maBtnCancel (this, ScResId(BTN_CANCEL)),
    maBtnHelp   (this, ScResId(BTN_HELP)),

    maTextDescription(this, ScResId(FT_DESC)),
    maLineDocument(this, ScResId(FL_DOCUMENT)),
    maTextDocStatus(this, ScResId(FT_DOCSTATUS)),
    maBtnRetypeDoc(this, ScResId(BTN_RETYPE_DOC)),

    maLineSheet(this, ScResId(FL_SHEET)),
    maTextSheetName1(this, ScResId(FT_SHEETNAME1)),
    maTextSheetStatus1(this, ScResId(FT_SHEETSTATUS1)),
    maBtnRetypeSheet1(this, ScResId(BTN_RETYPE_SHEET1)),

    maTextSheetName2(this, ScResId(FT_SHEETNAME2)),
    maTextSheetStatus2(this, ScResId(FT_SHEETSTATUS2)),
    maBtnRetypeSheet2(this, ScResId(BTN_RETYPE_SHEET2)),

    maTextSheetName3(this, ScResId(FT_SHEETNAME3)),
    maTextSheetStatus3(this, ScResId(FT_SHEETSTATUS3)),
    maBtnRetypeSheet3(this, ScResId(BTN_RETYPE_SHEET3)),

    maTextSheetName4(this, ScResId(FT_SHEETNAME4)),
    maTextSheetStatus4(this, ScResId(FT_SHEETSTATUS4)),
    maBtnRetypeSheet4(this, ScResId(BTN_RETYPE_SHEET4)),

    maScrollBar (this, ScResId(SB_SCROLL)),

    maTextNotProtected(ScResId(STR_NOT_PROTECTED)),
    maTextNotPassProtected(ScResId(STR_NOT_PASS_PROTECTED)),
    maTextHashBad(ScResId(STR_HASH_BAD)),
    maTextHashGood(ScResId(STR_HASH_GOOD)),
    maTextHashRegen(ScResId(STR_HASH_REGENERATED)),

    mpDocItem(static_cast<ScDocProtection*>(NULL)),
    mnCurScrollPos(0),
    meDesiredHash(PASSHASH_SHA1)
{
    Init();
}

ScRetypePassDlg::~ScRetypePassDlg()
{
}

short ScRetypePassDlg::Execute()
{
    PopulateDialog();
    CheckHashStatus();
    return ModalDialog::Execute();
}

void ScRetypePassDlg::SetDataFromDocument(const ScDocument& rDoc)
{
    const ScDocProtection* pDocProtect = rDoc.GetDocProtection();
    if (pDocProtect && pDocProtect->isProtected())
        mpDocItem.reset(new ScDocProtection(*pDocProtect));

    SCTAB nTabCount = rDoc.GetTableCount();
    maTableItems.reserve(nTabCount);
    for (SCTAB i = 0; i < nTabCount; ++i)
    {
        TableItem aTabItem;
        rDoc.GetName(i, aTabItem.maName);

        const ScTableProtection* pTabProtect = rDoc.GetTabProtection(i);
        if (pTabProtect && pTabProtect->isProtected())
            aTabItem.mpProtect.reset(new ScTableProtection(*pTabProtect));

        maTableItems.push_back(aTabItem);
    }
}

void ScRetypePassDlg::SetDesiredHash(ScPasswordHash eHash)
{
    meDesiredHash = eHash;
}

void ScRetypePassDlg::WriteNewDataToDocument(ScDocument& rDoc) const
{
    if (mpDocItem.get())
        rDoc.SetDocProtection(mpDocItem.get());

    size_t nTabCount = static_cast<size_t>(rDoc.GetTableCount());
    size_t n = maTableItems.size();
    for (size_t i = 0; i < n; ++i)
    {
        if (i >= nTabCount)
            break;

        ScTableProtection* pTabProtect = maTableItems[i].mpProtect.get();
        if (pTabProtect)
            rDoc.SetTabProtection(static_cast<SCTAB>(i), pTabProtect);
    }
}

void ScRetypePassDlg::Init()
{
    Link aLink = LINK( this, ScRetypePassDlg, OKHdl );
    maBtnOk.SetClickHdl(aLink);

    aLink = LINK( this, ScRetypePassDlg, RetypeBtnHdl );
    maBtnRetypeDoc.SetClickHdl(aLink);
    maBtnRetypeSheet1.SetClickHdl(aLink);
    maBtnRetypeSheet2.SetClickHdl(aLink);
    maBtnRetypeSheet3.SetClickHdl(aLink);
    maBtnRetypeSheet4.SetClickHdl(aLink);

    maTextDocStatus.SetText(maTextNotProtected);
    maTextSheetStatus1.SetText(maTextNotProtected);
    maTextSheetStatus2.SetText(maTextNotProtected);
    maTextSheetStatus3.SetText(maTextNotProtected);
    maTextSheetStatus4.SetText(maTextNotProtected);
    maBtnRetypeDoc.Disable();

    // Make all sheet rows invisible.

    maTextSheetName1.Show(false);
    maTextSheetStatus1.Show(false);
    maBtnRetypeSheet1.Show(false);
    maBtnRetypeSheet1.Disable();

    maTextSheetName2.Show(false);
    maTextSheetStatus2.Show(false);
    maBtnRetypeSheet2.Show(false);
    maBtnRetypeSheet2.Disable();

    maTextSheetName3.Show(false);
    maTextSheetStatus3.Show(false);
    maBtnRetypeSheet3.Show(false);
    maBtnRetypeSheet3.Disable();

    maTextSheetName4.Show(false);
    maTextSheetStatus4.Show(false);
    maBtnRetypeSheet4.Show(false);
    maBtnRetypeSheet4.Disable();

    maScrollBar.Show(false);

    maScrollBar.SetEndScrollHdl( LINK( this, ScRetypePassDlg, ScrollHdl ) );
    maScrollBar.SetScrollHdl( LINK( this, ScRetypePassDlg, ScrollHdl ) );

    maScrollBar.SetPageSize(4);
    maScrollBar.SetVisibleSize(4);
    maScrollBar.SetLineSize(1);
}

void ScRetypePassDlg::PopulateDialog()
{
    // Document protection first.
    SetDocData();

    // Sheet protection next.  We're only interested in the first 4 sheets
    // (or less).
    size_t n = maTableItems.size();
    for (size_t i = 0; i < n && i < 4; ++i)
        SetTableData(i, static_cast< SCTAB >( i ));

    if (n > 4)
    {
        maScrollBar.Show(true);
        maScrollBar.SetRange(Range(0, n));
    }
}

void ScRetypePassDlg::SetDocData()
{
    bool bBtnEnabled = false;
    if (mpDocItem.get() && mpDocItem->isProtected())
    {
        if (mpDocItem->isPasswordEmpty())
            maTextDocStatus.SetText(maTextNotPassProtected);
        else if (mpDocItem->hasPasswordHash(meDesiredHash))
            maTextDocStatus.SetText(maTextHashGood);
        else
        {
            // incompatible hash
            maTextDocStatus.SetText(maTextHashBad);
            bBtnEnabled = true;
        }
    }
    maBtnRetypeDoc.Enable(bBtnEnabled);
}

void ScRetypePassDlg::SetTableData(size_t nRowPos, SCTAB nTab)
{
    if (nRowPos >= 4)
        return;

    FixedText* pName = NULL;
    FixedText* pStatus = NULL;
    PushButton* pBtn = NULL;
    switch (nRowPos)
    {
        case 0:
            pName = &maTextSheetName1;
            pStatus = &maTextSheetStatus1;
            pBtn = &maBtnRetypeSheet1;
        break;
        case 1:
            pName = &maTextSheetName2;
            pStatus = &maTextSheetStatus2;
            pBtn = &maBtnRetypeSheet2;
        break;
        case 2:
            pName = &maTextSheetName3;
            pStatus = &maTextSheetStatus3;
            pBtn = &maBtnRetypeSheet3;
        break;
        case 3:
            pName = &maTextSheetName4;
            pStatus = &maTextSheetStatus4;
            pBtn = &maBtnRetypeSheet4;
        break;
        default:
            return;
    }

    bool bBtnEnabled = false;
    pName->SetText(maTableItems[nTab].maName);
    pName->Show(true);
    const ScTableProtection* pTabProtect = maTableItems[nTab].mpProtect.get();
    if (pTabProtect && pTabProtect->isProtected())
    {
        if (pTabProtect->isPasswordEmpty())
            pStatus->SetText(maTextNotPassProtected);
        else if (pTabProtect->hasPasswordHash(meDesiredHash))
            pStatus->SetText(maTextHashGood);
        else
        {
            // incompatible hash
            pStatus->SetText(maTextHashBad);
            bBtnEnabled = true;
        }
    }
    else
        pStatus->SetText(maTextNotProtected);

    pStatus->Show(true);
    pBtn->Show(true);
    pBtn->Enable(bBtnEnabled);
}

void ScRetypePassDlg::ResetTableRows()
{
    long nScrollPos = maScrollBar.GetThumbPos();
    mnCurScrollPos = nScrollPos < 0 ? 0 : nScrollPos;
    size_t nRowCount = maTableItems.size() - nScrollPos;
    for (size_t i = 0; i < nRowCount; ++i)
        SetTableData(i, static_cast< SCTAB >( i + nScrollPos ));
}

static bool lcl_IsInGoodStatus(ScPassHashProtectable* pProtected, ScPasswordHash eDesiredHash)
{
    if (!pProtected || !pProtected->isProtected())
        // Not protected.
        return true;

    if (pProtected->isPasswordEmpty())
        return true;

    if (pProtected->hasPasswordHash(eDesiredHash))
        return true;

    return false;
}

void ScRetypePassDlg::CheckHashStatus()
{
    do
    {
        if (!lcl_IsInGoodStatus(mpDocItem.get(), meDesiredHash))
            break;

        bool bStatusGood = true;
        size_t nTabCount = maTableItems.size();
        for (size_t i = 0; i < nTabCount && bStatusGood; ++i)
        {
            if (!lcl_IsInGoodStatus(maTableItems[i].mpProtect.get(), meDesiredHash))
                bStatusGood = false;
        }
        if (!bStatusGood)
            break;

        maBtnOk.Enable();
        return;
    }
    while (false);

    maBtnOk.Disable();
}

IMPL_LINK_NOARG(ScRetypePassDlg, OKHdl)
{
    EndDialog(RET_OK);
    return 0;
}

IMPL_LINK( ScRetypePassDlg, RetypeBtnHdl, PushButton*, pBtn )
{
    ScPassHashProtectable* pProtected = NULL;
    if (pBtn == &maBtnRetypeDoc)
    {
        // document protection.
        pProtected = mpDocItem.get();
    }
    else
    {
        // sheet protection.
        size_t nTabPos = mnCurScrollPos;
        if (pBtn == &maBtnRetypeSheet2)
            nTabPos += 1;
        else if (pBtn == &maBtnRetypeSheet3)
            nTabPos += 2;
        else if (pBtn == &maBtnRetypeSheet4)
            nTabPos += 3;
        else if (pBtn != &maBtnRetypeSheet1)
            // This should never happen !
            return 0;

        if (nTabPos >= maTableItems.size())
            // Likewise, this should never happen !
            return 0;

        pProtected = maTableItems[nTabPos].mpProtect.get();
    }

    if (!pProtected)
        // What the ... !?
        return 0;

    ScRetypePassInputDlg aDlg(this, pProtected);
    if (aDlg.Execute() == RET_OK)
    {
        // OK is pressed.  Update the protected item.
        if (aDlg.IsRemovePassword())
        {
            // Remove password from this item.
            pProtected->setPassword(String());
        }
        else
        {
            // Set a new password.
            String aNewPass = aDlg.GetNewPassword();
            pProtected->setPassword(aNewPass);
        }

        SetDocData();
        ResetTableRows();
        CheckHashStatus();
    }
    return 0;
}

IMPL_LINK_NOARG(ScRetypePassDlg, ScrollHdl)
{
    ResetTableRows();
    return 0;
}

// ============================================================================

ScRetypePassInputDlg::ScRetypePassInputDlg(Window* pParent, ScPassHashProtectable* pProtected) :
    ModalDialog(pParent, ScResId(RID_SCDLG_RETYPEPASS_INPUT)),

    maBtnOk     (this, ScResId(BTN_OK)),
    maBtnCancel (this, ScResId(BTN_CANCEL)),
    maBtnHelp   (this, ScResId(BTN_HELP)),

    maBtnRetypePassword(this, ScResId(BTN_RETYPE_PASSWORD)),

    maPassword1Text (this, ScResId(FT_PASSWORD1)),
    maPassword1Edit (this, ScResId(ED_PASSWORD1)),
    maPassword2Text (this, ScResId(FT_PASSWORD2)),
    maPassword2Edit (this, ScResId(ED_PASSWORD2)),
    maBtnMatchOldPass(this, ScResId(BTN_MATCH_OLD_PASSWORD)),

    maBtnRemovePassword(this, ScResId(BTN_REMOVE_PASSWORD)),

    mpProtected(pProtected)
{
    Init();
}

ScRetypePassInputDlg::~ScRetypePassInputDlg()
{
}

short ScRetypePassInputDlg::Execute()
{
    return ModalDialog::Execute();
}

bool ScRetypePassInputDlg::IsRemovePassword() const
{
    return maBtnRemovePassword.IsChecked();
}

String ScRetypePassInputDlg::GetNewPassword() const
{
    return maPassword1Edit.GetText();
}

void ScRetypePassInputDlg::Init()
{
    Link aLink = LINK( this, ScRetypePassInputDlg, OKHdl );
    maBtnOk.SetClickHdl(aLink);
    aLink = LINK( this, ScRetypePassInputDlg, RadioBtnHdl );
    maBtnRetypePassword.SetClickHdl(aLink);
    maBtnRemovePassword.SetClickHdl(aLink);
    aLink = LINK( this, ScRetypePassInputDlg, CheckBoxHdl );
    maBtnMatchOldPass.SetClickHdl(aLink);
    aLink = LINK( this, ScRetypePassInputDlg, PasswordModifyHdl );
    maPassword1Edit.SetModifyHdl(aLink);
    maPassword2Edit.SetModifyHdl(aLink);

    maBtnOk.Disable();
    maBtnRetypePassword.Check(true);
    maBtnMatchOldPass.Check(true);
    maPassword1Edit.GrabFocus();
}

void ScRetypePassInputDlg::CheckPasswordInput()
{
    String aPass1 = maPassword1Edit.GetText();
    String aPass2 = maPassword2Edit.GetText();

    if (!aPass1.Len() || !aPass2.Len())
    {
        // Empty password is not allowed.
        maBtnOk.Disable();
        return;
    }

    if (!aPass1.Equals(aPass2))
    {
        // The two passwords differ.
        maBtnOk.Disable();
        return;
    }

    if (!maBtnMatchOldPass.IsChecked())
    {
        maBtnOk.Enable();
        return;
    }

    if (!mpProtected)
    {
        // This should never happen!
        maBtnOk.Disable();
        return;
    }

    bool bPassGood = mpProtected->verifyPassword(aPass1);
    maBtnOk.Enable(bPassGood);
}

IMPL_LINK_NOARG(ScRetypePassInputDlg, OKHdl)
{
    EndDialog(RET_OK);
    return 0;
}

IMPL_LINK( ScRetypePassInputDlg, RadioBtnHdl, RadioButton*, pBtn )
{
    if (pBtn == &maBtnRetypePassword)
    {
        maBtnRemovePassword.Check(false);
        maPassword1Text.Enable();
        maPassword1Edit.Enable();
        maPassword2Text.Enable();
        maPassword2Edit.Enable();
        maBtnMatchOldPass.Enable();
        CheckPasswordInput();
    }
    else if (pBtn == &maBtnRemovePassword)
    {
        maBtnRetypePassword.Check(false);
        maPassword1Text.Disable();
        maPassword1Edit.Disable();
        maPassword2Text.Disable();
        maPassword2Edit.Disable();
        maBtnMatchOldPass.Disable();
        maBtnOk.Enable();
    }

    return 0;
}

IMPL_LINK_NOARG(ScRetypePassInputDlg, CheckBoxHdl)
{
    CheckPasswordInput();
    return 0;
}

IMPL_LINK_NOARG(ScRetypePassInputDlg, PasswordModifyHdl)
{
    CheckPasswordInput();
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
