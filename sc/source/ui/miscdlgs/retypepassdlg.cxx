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

#include <vcl/svapp.hxx>
#include <strings.hrc>
#include <retypepassdlg.hxx>
#include <scresid.hxx>
#include <document.hxx>
#include <tabprotection.hxx>

ScRetypePassDlg::ScRetypePassDlg(weld::Window* pParent)
    : GenericDialogController(pParent, "modules/scalc/ui/retypepassdialog.ui", "RetypePass")
    , maTextNotProtected(ScResId(STR_NOT_PROTECTED))
    , maTextNotPassProtected(ScResId(STR_NOT_PASS_PROTECTED))
    , maTextHashBad(ScResId(STR_HASH_BAD))
    , maTextHashGood(ScResId(STR_HASH_GOOD))
    , mpDocItem(static_cast<ScDocProtection*>(nullptr))
    , meDesiredHash(PASSHASH_SHA1)
    , mxBtnOk(m_xBuilder->weld_button("ok"))
    , mxTextDocStatus(m_xBuilder->weld_label("docStatusLabel"))
    , mxBtnRetypeDoc(m_xBuilder->weld_button("retypeDocButton"))
    , mxScrolledWindow(m_xBuilder->weld_scrolled_window("scrolledwindow"))
    , mxSheetsBox(m_xBuilder->weld_container("sheetsBox"))
{
    mxScrolledWindow->set_size_request(mxScrolledWindow->get_approximate_digit_width() * 46,
                                       mxScrolledWindow->get_text_height() * 10);
    Init();
}

ScRetypePassDlg::~ScRetypePassDlg()
{
}

void ScRetypePassDlg::DeleteSheets()
{
    maSheets.clear();
}

short ScRetypePassDlg::run()
{
    PopulateDialog();
    CheckHashStatus();
    return GenericDialogController::run();
}

PassFragment::PassFragment(weld::Widget* pParent)
    : m_xBuilder(Application::CreateBuilder(pParent, "modules/scalc/ui/passfragment.ui"))
    , m_xSheetsBox(m_xBuilder->weld_container("PassEntry"))
    , m_xName(m_xBuilder->weld_label("name"))
    , m_xStatus(m_xBuilder->weld_label("status"))
    , m_xButton(m_xBuilder->weld_button("button"))
{
    m_xButton->set_label(ScResId(STR_RETYPE));
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
        maSheets.emplace_back(new PassFragment(mxSheetsBox.get()));
        maSheets.back()->m_xButton->connect_clicked(LINK(this, ScRetypePassDlg, RetypeBtnHdl));
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
    Link<weld::Button&,void> aLink = LINK( this, ScRetypePassDlg, OKHdl );
    mxBtnOk->connect_clicked(aLink);

    aLink = LINK( this, ScRetypePassDlg, RetypeBtnHdl );
    mxBtnRetypeDoc->connect_clicked(aLink);

    mxTextDocStatus->set_label(maTextNotProtected);
    mxBtnRetypeDoc->set_sensitive(false);
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
            mxTextDocStatus->set_label(maTextNotPassProtected);
        else if (mpDocItem->hasPasswordHash(meDesiredHash))
            mxTextDocStatus->set_label(maTextHashGood);
        else
        {
            // incompatible hash
            mxTextDocStatus->set_label(maTextHashBad);
            bBtnEnabled = true;
        }
    }
    mxBtnRetypeDoc->set_sensitive(bBtnEnabled);
}

void ScRetypePassDlg::SetTableData(size_t nRowPos, SCTAB nTab)
{
    if (nRowPos < maSheets.size())
    {
        weld::Label& rName = *maSheets[nRowPos]->m_xName;
        weld::Label& rStatus = *maSheets[nRowPos]->m_xStatus;
        weld::Button& rBtn = *maSheets[nRowPos]->m_xButton;

        bool bBtnEnabled = false;
        rName.set_label(maTableItems[nTab].maName);
        const ScTableProtection* pTabProtect = maTableItems[nTab].mpProtect.get();
        if (pTabProtect && pTabProtect->isProtected())
        {
            if (pTabProtect->isPasswordEmpty())
                rStatus.set_label(maTextNotPassProtected);
            else if (pTabProtect->hasPasswordHash(meDesiredHash))
                rStatus.set_label(maTextHashGood);
            else
            {
                // incompatible hash
                rStatus.set_label(maTextHashBad);
                bBtnEnabled = true;
            }
        }
        else
            rStatus.set_label(maTextNotProtected);

        rBtn.set_sensitive(bBtnEnabled);
    }
}

static bool lcl_IsInGoodStatus(const ScPassHashProtectable* pProtected, ScPasswordHash eDesiredHash)
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

        mxBtnOk->set_sensitive(true);
        return;
    }
    while (false);

    mxBtnOk->set_sensitive(false);
}

IMPL_LINK_NOARG(ScRetypePassDlg, OKHdl, weld::Button&, void)
{
    m_xDialog->response(RET_OK);
}

IMPL_LINK(ScRetypePassDlg, RetypeBtnHdl, weld::Button&, rBtn, void)
{
    ScPassHashProtectable* pProtected = nullptr;
    if (&rBtn == mxBtnRetypeDoc.get())
    {
        // document protection.
        pProtected = mpDocItem.get();
    }
    else
    {
        // sheet protection.
        size_t aPos = 0;
        while (aPos < maSheets.size() && &rBtn != maSheets[aPos]->m_xButton.get())
            ++aPos;

        pProtected = aPos < maSheets.size() ? maTableItems[aPos].mpProtect.get() : nullptr;
    }

    if (!pProtected)
        // What the ... !?
        return;

    ScRetypePassInputDlg aDlg(m_xDialog.get(), pProtected);
    if (aDlg.run() == RET_OK)
    {
        // OK is pressed.  Update the protected item.
        if (aDlg.IsRemovePassword())
        {
            // Remove password from this item.
            pProtected->setPassword(OUString());
        }
        else
        {
            // Set a new password.
            OUString aNewPass = aDlg.GetNewPassword();
            pProtected->setPassword(aNewPass);
        }

        SetDocData();
        CheckHashStatus();
    }
}

ScRetypePassInputDlg::ScRetypePassInputDlg(weld::Window* pParent, ScPassHashProtectable* pProtected)
    : GenericDialogController(pParent, "modules/scalc/ui/retypepassworddialog.ui", "RetypePasswordDialog")
    , m_pProtected(pProtected)
    , m_xBtnOk(m_xBuilder->weld_button("ok"))
    , m_xBtnRetypePassword(m_xBuilder->weld_radio_button("retypepassword"))
    , m_xPasswordGrid(m_xBuilder->weld_widget("passwordgrid"))
    , m_xPassword1Edit(m_xBuilder->weld_entry("newpassEntry"))
    , m_xPassword2Edit(m_xBuilder->weld_entry("confirmpassEntry"))
    , m_xBtnMatchOldPass(m_xBuilder->weld_check_button("mustmatch"))
    , m_xBtnRemovePassword(m_xBuilder->weld_radio_button("removepassword"))
{
    Init();
}

ScRetypePassInputDlg::~ScRetypePassInputDlg()
{
}

bool ScRetypePassInputDlg::IsRemovePassword() const
{
    return m_xBtnRemovePassword->get_active();
}

OUString ScRetypePassInputDlg::GetNewPassword() const
{
    return m_xPassword1Edit->get_text();
}

void ScRetypePassInputDlg::Init()
{
    m_xBtnOk->connect_clicked(LINK(this, ScRetypePassInputDlg, OKHdl));
    m_xBtnRetypePassword->connect_toggled(LINK(this, ScRetypePassInputDlg, RadioBtnHdl));
    m_xBtnRemovePassword->connect_toggled(LINK(this, ScRetypePassInputDlg, RadioBtnHdl));
    m_xBtnMatchOldPass->connect_toggled(LINK(this, ScRetypePassInputDlg, CheckBoxHdl));
    Link<weld::Entry&,void> aLink2 = LINK( this, ScRetypePassInputDlg, PasswordModifyHdl );
    m_xPassword1Edit->connect_changed(aLink2);
    m_xPassword2Edit->connect_changed(aLink2);

    m_xBtnOk->set_sensitive(false);
    m_xBtnRetypePassword->set_active(true);
    m_xBtnMatchOldPass->set_active(true);
    m_xPassword1Edit->grab_focus();
}

void ScRetypePassInputDlg::CheckPasswordInput()
{
    OUString aPass1 = m_xPassword1Edit->get_text();
    OUString aPass2 = m_xPassword2Edit->get_text();

    if (aPass1.isEmpty() || aPass2.isEmpty())
    {
        // Empty password is not allowed.
        m_xBtnOk->set_sensitive(false);
        return;
    }

    if (aPass1 != aPass2)
    {
        // The two passwords differ.
        m_xBtnOk->set_sensitive(false);
        return;
    }

    if (!m_xBtnMatchOldPass->get_active())
    {
        m_xBtnOk->set_sensitive(true);
        return;
    }

    if (!m_pProtected)
    {
        // This should never happen!
        m_xBtnOk->set_sensitive(false);
        return;
    }

    bool bPassGood = m_pProtected->verifyPassword(aPass1);
    m_xBtnOk->set_sensitive(bPassGood);
}

IMPL_LINK_NOARG(ScRetypePassInputDlg, OKHdl, weld::Button&, void)
{
    m_xDialog->response(RET_OK);
}

IMPL_LINK_NOARG(ScRetypePassInputDlg, RadioBtnHdl, weld::ToggleButton&, void)
{
    if (m_xBtnRetypePassword->get_active())
    {
        m_xPasswordGrid->set_sensitive(true);
        CheckPasswordInput();
    }
    else
    {
        m_xPasswordGrid->set_sensitive(false);
        m_xBtnOk->set_sensitive(false);
    }
}

IMPL_LINK_NOARG(ScRetypePassInputDlg, CheckBoxHdl, weld::ToggleButton&, void)
{
    CheckPasswordInput();
}

IMPL_LINK_NOARG(ScRetypePassInputDlg, PasswordModifyHdl, weld::Entry&, void)
{
    CheckPasswordInput();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
