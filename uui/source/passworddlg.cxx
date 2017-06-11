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

#include "passworddlg.hxx"
#include "strings.hrc"

#include <tools/simplerm.hxx>
#include <tools/urlobj.hxx>
#include <vcl/layout.hxx>

using namespace ::com::sun::star;

PasswordDialog::PasswordDialog(vcl::Window* _pParent,
    task::PasswordRequestMode nDlgMode, const std::locale& rLocale,
    const OUString& aDocURL, bool bOpenToModify, bool bIsSimplePasswordRequest)
    : ModalDialog(_pParent, "PasswordDialog", "uui/ui/password.ui")
    , nMinLen(1)
    , aPasswdMismatch(Translate::get(STR_PASSWORD_MISMATCH, rLocale))
    , nDialogMode(nDlgMode)
    , rResLocale(rLocale)
{
    get(m_pFTPassword, "newpassFT");
    get(m_pEDPassword, "newpassEntry");
    get(m_pFTConfirmPassword, "confirmpassFT");
    get(m_pEDConfirmPassword, "confirmpassEntry");
    get(m_pOKBtn, "ok");

    if( nDialogMode == task::PasswordRequestMode_PASSWORD_REENTER )
    {
        const char* pOpenToModifyErrStrId = bOpenToModify ? STR_ERROR_PASSWORD_TO_MODIFY_WRONG : STR_ERROR_PASSWORD_TO_OPEN_WRONG;
        const char* pErrStrId = bIsSimplePasswordRequest ? STR_ERROR_SIMPLE_PASSWORD_WRONG : pOpenToModifyErrStrId;
        OUString aErrorMsg(Translate::get(pErrStrId, rResLocale));
        ScopedVclPtrInstance< MessageDialog > aErrorBox(GetParent(), aErrorMsg);
        aErrorBox->Execute();
    }

    // default settings for enter password or reenter passwd...
    OUString aTitle(Translate::get(STR_TITLE_ENTER_PASSWORD, rResLocale));
    m_pFTConfirmPassword->Hide();
    m_pEDConfirmPassword->Hide();
    m_pFTConfirmPassword->Enable( false );
    m_pEDConfirmPassword->Enable( false );

    // settings for create password
    if (nDialogMode == task::PasswordRequestMode_PASSWORD_CREATE)
    {
        aTitle = Translate::get(STR_TITLE_CREATE_PASSWORD, rResLocale);

        m_pFTConfirmPassword->SetText(Translate::get(STR_CONFIRM_SIMPLE_PASSWORD, rResLocale));

        m_pFTConfirmPassword->Show();
        m_pEDConfirmPassword->Show();
        m_pFTConfirmPassword->Enable();
        m_pEDConfirmPassword->Enable();
    }

    SetText( aTitle );

    const char* pStrId = bOpenToModify ? STR_ENTER_PASSWORD_TO_MODIFY : STR_ENTER_PASSWORD_TO_OPEN;
    OUString aMessage(Translate::get(pStrId, rResLocale));
    INetURLObject url(aDocURL);
    aMessage += url.HasError()
        ? aDocURL : url.GetMainURL(INetURLObject::DecodeMechanism::Unambiguous);
    m_pFTPassword->SetText(aMessage);

    if (bIsSimplePasswordRequest)
    {
        DBG_ASSERT( aDocURL.isEmpty(), "A simple password request should not have a document URL! Use document password request instead." );
        m_pFTPassword->SetText(Translate::get(STR_ENTER_SIMPLE_PASSWORD, rResLocale));
    }

    m_pOKBtn->SetClickHdl( LINK( this, PasswordDialog, OKHdl_Impl ) );
}

PasswordDialog::~PasswordDialog()
{
    disposeOnce();
}

void PasswordDialog::dispose()
{
    m_pFTPassword.clear();
    m_pEDPassword.clear();
    m_pFTConfirmPassword.clear();
    m_pEDConfirmPassword.clear();
    m_pOKBtn.clear();
    ModalDialog::dispose();
}

IMPL_LINK_NOARG(PasswordDialog, OKHdl_Impl, Button*, void)
{
    bool bEDPasswdValid = m_pEDPassword->GetText().getLength() >= nMinLen;
    bool bPasswdMismatch = m_pEDConfirmPassword->GetText() != m_pEDPassword->GetText();
    bool bValid = (!m_pEDConfirmPassword->IsVisible() && bEDPasswdValid) ||
            (m_pEDConfirmPassword->IsVisible() && bEDPasswdValid && !bPasswdMismatch);

    if (m_pEDConfirmPassword->IsVisible() && bPasswdMismatch)
    {
        ScopedVclPtrInstance< MessageDialog > aErrorBox(this, aPasswdMismatch);
        aErrorBox->Execute();
    }
    else if (bValid)
        EndDialog( RET_OK );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

