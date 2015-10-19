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

#include "sc.hrc"
#include "retypepassdlg.hxx"
#include "scresid.hxx"
#include "document.hxx"
#include "tabprotection.hxx"

#include <vcl/msgbox.hxx>

ScRetypePassDlg::ScRetypePassDlg(vcl::Window* pParent) :
    ModalDialog(pParent, "RetypePass", "modules/scalc/ui/retypepassdialog.ui"),

    maTextNotProtected(ScResId(STR_NOT_PROTECTED)),
    maTextNotPassProtected(ScResId(STR_NOT_PASS_PROTECTED)),
    maTextHashBad(ScResId(STR_HASH_BAD)),
    maTextHashGood(ScResId(STR_HASH_GOOD)),
    maTextHashRegen(ScResId(STR_HASH_REGENERATED)),

    mpDocItem(static_cast<ScDocProtection*>(NULL)),
    mnCurScrollPos(0),
    meDesiredHash(PASSHASH_SHA1)
{
    get(mpBtnOk ,"ok");
    get(mpTextDocStatus, "docStatusLabel");
    get(mpBtnRetypeDoc, "retypeDocButton");
    vcl::Window *pScrolledWindow = get<vcl::Window>("scrolledwindow");
    Size aSize(LogicToPixel(Size(190, 90), MAP_APPFONT));
    pScrolledWindow->set_width_request(aSize.Width());
    pScrolledWindow->set_height_request(aSize.Height());
    get(mpSheetsBox, "sheetsBox");

    Init();
}

ScRetypePassDlg::~ScRetypePassDlg()
{
    disposeOnce();
}

void ScRetypePassDlg::dispose()
{
    DeleteSheets();
    mpBtnOk.clear();
    mpTextDocStatus.clear();
    mpBtnRetypeDoc.clear();
    mpSheetsBox.clear();
    ModalDialog::dispose();
}

void ScRetypePassDlg::DeleteSheets()
{
    for(auto it = maSheets.begin(); it != maSheets.end(); ++it)
    {
        VclPtr<vcl::Window> pWindow = (*it);
        vcl::Window *pChild = pWindow->GetWindow(GetWindowType::FirstChild);
        while (pChild)
        {
            VclPtr<vcl::Window> pOldChild = pChild;
            pChild = pChild->GetWindow(GetWindowType::Next);
            pOldChild.disposeAndClear();
        }
        pWindow.disposeAndClear();
    }
}

short ScRetypePassDlg::Execute()
{
    PopulateDialog();
    CheckHashStatus();
    return ModalDialog::Execute();
}

void ScRetypePassDlg::SetDataFromDocument(const ScDocument& rDoc)
{
    DeleteSheets();
    const ScDocProtection* pDocProtect = rDoc.GetDocProtection();
    if (pDocProtect && pDocProtect->isProtected())
        mpDocItem.reset(new ScDocProtection(*pDocProtect));

    SCTAB nTabCount = rDoc.GetTableCount();
    maTableItems.reserve(nTabCount);
    maSheets.reserve(nTabCount);
    for (SCTAB i = 0; i < nTabCount; ++i)
    {
        TableItem aTabItem;
        rDoc.GetName(i, aTabItem.maName);

        const ScTableProtection* pTabProtect = rDoc.GetTabProtection(i);
        if (pTabProtect && pTabProtect->isProtected())
            aTabItem.mpProtect.reset(new ScTableProtection(*pTabProtect));

        maTableItems.push_back(aTabItem);
        VclPtr<VclHBox> pSheet = VclPtr<VclHBox>::Create(mpSheetsBox, false, 12);
        pSheet->Show();

        VclPtr<FixedText> pFtSheetName = VclPtr<FixedText>::Create(pSheet);
        pFtSheetName->Show();
        pFtSheetName->SetStyle(WB_VCENTER);
        FixedText* pFtSheetStatus = VclPtr<FixedText>::Create(pSheet);
        pFtSheetStatus->Show();
        pFtSheetStatus->SetStyle(WB_VCENTER);

        VclPtr<PushButton> pBtnSheet = VclPtr<PushButton>::Create(static_cast<vcl::Window*>(pSheet));
        pBtnSheet->SetText(ScResId(STR_RETYPE));
        pBtnSheet->SetClickHdl(LINK(this, ScRetypePassDlg, RetypeBtnHdl));
        pBtnSheet->Disable();
        pBtnSheet->Show();

        maSheets.push_back(pSheet);
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
    Link<Button*,void> aLink = LINK( this, ScRetypePassDlg, OKHdl );
    mpBtnOk->SetClickHdl(aLink);

    aLink = LINK( this, ScRetypePassDlg, RetypeBtnHdl );
    mpBtnRetypeDoc->SetClickHdl(aLink);

    mpTextDocStatus->SetText(maTextNotProtected);
    mpBtnRetypeDoc->Disable();
}

void ScRetypePassDlg::PopulateDialog()
{
    // Document protection first.
    SetDocData();

    // Sheet protection next.
    for (size_t i = 0; i < maTableItems.size(); ++i)
        SetTableData(i, static_cast< SCTAB >( i ));
}

void ScRetypePassDlg::SetDocData()
{
    bool bBtnEnabled = false;
    if (mpDocItem.get() && mpDocItem->isProtected())
    {
        if (mpDocItem->isPasswordEmpty())
            mpTextDocStatus->SetText(maTextNotPassProtected);
        else if (mpDocItem->hasPasswordHash(meDesiredHash))
            mpTextDocStatus->SetText(maTextHashGood);
        else
        {
            // incompatible hash
            mpTextDocStatus->SetText(maTextHashBad);
            bBtnEnabled = true;
        }
    }
    mpBtnRetypeDoc->Enable(bBtnEnabled);
}

void ScRetypePassDlg::SetTableData(size_t nRowPos, SCTAB nTab)
{
    if(nRowPos < maSheets.size())
    {
        FixedText* pName = static_cast<FixedText*>(maSheets[nRowPos]->GetChild(0));
        FixedText* pStatus = static_cast<FixedText*>(maSheets[nRowPos]->GetChild(1));
        PushButton* pBtn = static_cast<PushButton*>(maSheets[nRowPos]->GetChild(2));

        bool bBtnEnabled = false;
        pName->SetText(maTableItems[nTab].maName);
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

        pBtn->Enable(bBtnEnabled);
    }
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

        mpBtnOk->Enable();
        return;
    }
    while (false);

    mpBtnOk->Disable();
}

IMPL_LINK_NOARG_TYPED(ScRetypePassDlg, OKHdl, Button*, void)
{
    EndDialog(RET_OK);
}

IMPL_LINK_TYPED( ScRetypePassDlg, RetypeBtnHdl, Button*, pBtn, void )
{
    ScPassHashProtectable* pProtected = NULL;
    if (pBtn == mpBtnRetypeDoc)
    {
        // document protection.
        pProtected = mpDocItem.get();
    }
    else
    {
        // sheet protection.
        size_t aPos = 0;
        while(aPos < maSheets.size() && pBtn != maSheets[aPos]->GetChild(2))
            ++aPos;

        pProtected = aPos < maSheets.size() ? maTableItems[aPos].mpProtect.get() : 0;
    }

    if (!pProtected)
        // What the ... !?
        return;

    ScopedVclPtrInstance< ScRetypePassInputDlg > aDlg(this, pProtected);
    if (aDlg->Execute() == RET_OK)
    {
        // OK is pressed.  Update the protected item.
        if (aDlg->IsRemovePassword())
        {
            // Remove password from this item.
            pProtected->setPassword(OUString());
        }
        else
        {
            // Set a new password.
            OUString aNewPass = aDlg->GetNewPassword();
            pProtected->setPassword(aNewPass);
        }

        SetDocData();
        CheckHashStatus();
    }
}

ScRetypePassInputDlg::ScRetypePassInputDlg(vcl::Window* pParent, ScPassHashProtectable* pProtected)
    : ModalDialog(pParent, "RetypePasswordDialog",
        "modules/scalc/ui/retypepassworddialog.ui")
    , mpProtected(pProtected)
{
    get(m_pBtnOk, "ok");
    get(m_pBtnRetypePassword, "retypepassword");
    get(m_pBtnRemovePassword, "removepassword");
    get(m_pPasswordGrid, "passwordgrid");
    get(m_pPassword1Edit, "newpassEntry");
    get(m_pPassword2Edit, "confirmpassEntry");
    get(m_pBtnMatchOldPass, "mustmatch");

    Init();
}

ScRetypePassInputDlg::~ScRetypePassInputDlg()
{
    disposeOnce();
}

void ScRetypePassInputDlg::dispose()
{
    m_pBtnOk.clear();
    m_pBtnRetypePassword.clear();
    m_pPasswordGrid.clear();
    m_pPassword1Edit.clear();
    m_pPassword2Edit.clear();
    m_pBtnMatchOldPass.clear();
    m_pBtnRemovePassword.clear();
    ModalDialog::dispose();
}

short ScRetypePassInputDlg::Execute()
{
    return ModalDialog::Execute();
}

bool ScRetypePassInputDlg::IsRemovePassword() const
{
    return m_pBtnRemovePassword->IsChecked();
}

OUString ScRetypePassInputDlg::GetNewPassword() const
{
    return m_pPassword1Edit->GetText();
}

void ScRetypePassInputDlg::Init()
{
    Link<Button*,void> aLink = LINK( this, ScRetypePassInputDlg, OKHdl );
    m_pBtnOk->SetClickHdl(aLink);
    aLink = LINK( this, ScRetypePassInputDlg, RadioBtnHdl );
    m_pBtnRetypePassword->SetClickHdl(aLink);
    m_pBtnRemovePassword->SetClickHdl(aLink);
    aLink = LINK( this, ScRetypePassInputDlg, CheckBoxHdl );
    m_pBtnMatchOldPass->SetClickHdl(aLink);
    Link<Edit&,void> aLink2 = LINK( this, ScRetypePassInputDlg, PasswordModifyHdl );
    m_pPassword1Edit->SetModifyHdl(aLink2);
    m_pPassword2Edit->SetModifyHdl(aLink2);

    m_pBtnOk->Disable();
    m_pBtnRetypePassword->Check();
    m_pBtnMatchOldPass->Check();
    m_pPassword1Edit->GrabFocus();
}

void ScRetypePassInputDlg::CheckPasswordInput()
{
    OUString aPass1 = m_pPassword1Edit->GetText();
    OUString aPass2 = m_pPassword2Edit->GetText();

    if (aPass1.isEmpty() || aPass2.isEmpty())
    {
        // Empty password is not allowed.
        m_pBtnOk->Disable();
        return;
    }

    if (aPass1 != aPass2)
    {
        // The two passwords differ.
        m_pBtnOk->Disable();
        return;
    }

    if (!m_pBtnMatchOldPass->IsChecked())
    {
        m_pBtnOk->Enable();
        return;
    }

    if (!mpProtected)
    {
        // This should never happen!
        m_pBtnOk->Disable();
        return;
    }

    bool bPassGood = mpProtected->verifyPassword(aPass1);
    m_pBtnOk->Enable(bPassGood);
}

IMPL_LINK_NOARG_TYPED(ScRetypePassInputDlg, OKHdl, Button*, void)
{
    EndDialog(RET_OK);
}

IMPL_LINK_TYPED( ScRetypePassInputDlg, RadioBtnHdl, Button*, pBtn, void )
{
    if (pBtn == m_pBtnRetypePassword)
    {
        m_pBtnRemovePassword->Check(false);
        m_pPasswordGrid->Enable();
        CheckPasswordInput();
    }
    else if (pBtn == m_pBtnRemovePassword)
    {
        m_pBtnRetypePassword->Check(false);
        m_pPasswordGrid->Disable();
        m_pBtnOk->Enable();
    }
}

IMPL_LINK_NOARG_TYPED(ScRetypePassInputDlg, CheckBoxHdl, Button*, void)
{
    CheckPasswordInput();
}

IMPL_LINK_NOARG_TYPED(ScRetypePassInputDlg, PasswordModifyHdl, Edit&, void)
{
    CheckPasswordInput();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
