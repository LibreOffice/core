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

#include <comphelper/string.hxx>
#include <vcl/msgbox.hxx>
#include "logindlg.hxx"

#include "ids.hrc"
#include <tools/resid.hxx>
#include <osl/file.hxx>

#ifdef UNX
#include <limits.h>
#define _MAX_PATH PATH_MAX
#endif

#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/ui/dialogs/FolderPicker.hpp>
#include <comphelper/processfactory.hxx>

using namespace com::sun::star;

void LoginDialog::HideControls_Impl( sal_uInt16 nFlags )
{
    if ( ( nFlags & LF_NO_PATH ) == LF_NO_PATH )
    {
        m_pPathFT->Hide();
        m_pPathED->Hide();
        m_pPathBtn->Hide();
    }
    else if ( ( nFlags & LF_PATH_READONLY ) == LF_PATH_READONLY )
    {
        m_pPathED->Enable( sal_False );
        m_pPathBtn->Enable( sal_False );
    }

    if ( ( nFlags & LF_NO_USERNAME ) == LF_NO_USERNAME )
    {
        m_pNameFT->Hide();
        m_pNameED->Hide();
    }
    else if ( ( nFlags & LF_USERNAME_READONLY ) == LF_USERNAME_READONLY )
    {
        m_pNameED->Enable( sal_False );
    }

    if ( ( nFlags & LF_NO_PASSWORD ) == LF_NO_PASSWORD )
    {
        m_pPasswordFT->Hide();
        m_pPasswordED->Hide();
    }

    if ( ( nFlags & LF_NO_SAVEPASSWORD ) == LF_NO_SAVEPASSWORD )
        m_pSavePasswdBtn->Hide();

    if ( ( nFlags & LF_NO_ERRORTEXT ) == LF_NO_ERRORTEXT )
    {
        m_pErrorInfo->Hide();
        m_pErrorFT->Hide();
    }

    if ( ( nFlags & LF_NO_ACCOUNT ) == LF_NO_ACCOUNT )
    {
        m_pAccountFT->Hide();
        m_pAccountED->Hide();
    }

    if ( ( nFlags & LF_NO_USESYSCREDS ) == LF_NO_USESYSCREDS )
    {
        m_pUseSysCredsCB->Hide();
    }
}

void LoginDialog::EnableUseSysCredsControls_Impl( sal_Bool bUseSysCredsEnabled )
{
    m_pErrorInfo->Enable( !bUseSysCredsEnabled );
    m_pErrorFT->Enable( !bUseSysCredsEnabled );
    m_pRequestInfo->Enable( !bUseSysCredsEnabled );
    m_pPathFT->Enable( !bUseSysCredsEnabled );
    m_pPathED->Enable( !bUseSysCredsEnabled );
    m_pPathBtn->Enable( !bUseSysCredsEnabled );
    m_pNameFT->Enable( !bUseSysCredsEnabled );
    m_pNameED->Enable( !bUseSysCredsEnabled );
    m_pPasswordFT->Enable( !bUseSysCredsEnabled );
    m_pPasswordED->Enable( !bUseSysCredsEnabled );
    m_pAccountFT->Enable( !bUseSysCredsEnabled );
    m_pAccountED->Enable( !bUseSysCredsEnabled );
}

IMPL_LINK_NOARG(LoginDialog, OKHdl_Impl)
{
    // trim the strings
    m_pNameED->SetText(comphelper::string::strip(m_pNameED->GetText(), ' '));
    m_pPasswordED->SetText(comphelper::string::strip(m_pPasswordED->GetText(), ' '));
    EndDialog( RET_OK );
    return 1;
}

IMPL_LINK_NOARG(LoginDialog, PathHdl_Impl)
{
    try
    {
        uno::Reference<ui::dialogs::XFolderPicker2> xFolderPicker = ui::dialogs::FolderPicker::create(comphelper::getProcessComponentContext());

        OUString aPath( m_pPathED->GetText() );
        osl::FileBase::getFileURLFromSystemPath( aPath, aPath );
        xFolderPicker->setDisplayDirectory( aPath );

        if (xFolderPicker->execute() == ui::dialogs::ExecutableDialogResults::OK)
        {
            osl::FileBase::getSystemPathFromFileURL( xFolderPicker->getDirectory(), aPath );
            m_pPathED->SetText( aPath );
        }
    }
    catch (uno::Exception & e)
    {
        SAL_WARN("uui", "LoginDialog::PathHdl_Impl: caught UNO exception: " << e.Message);
    }

    return 1;
}

IMPL_LINK_NOARG(LoginDialog, UseSysCredsHdl_Impl)
{
    EnableUseSysCredsControls_Impl( m_pUseSysCredsCB->IsChecked() );
    return 1;
}

LoginDialog::LoginDialog(Window* pParent, sal_uInt16 nFlags,
    const OUString& rServer, const OUString& rRealm)
    : ModalDialog(pParent, "LoginDialog", "uui/ui/logindialog.ui")
{
    get(m_pErrorFT, "errorft");
    get(m_pErrorInfo, "errorinfo");
    get(m_pRequestInfo, "requestinfo");
    get(m_pPathFT, "pathft");
    get(m_pPathED, "pathed");
    get(m_pPathBtn, "pathbtn");
    get(m_pNameFT, "nameft");
    get(m_pNameED, "nameed");
    get(m_pPasswordFT, "passwordft");
    get(m_pPasswordED, "passworded");
    get(m_pAccountFT, "accountft");
    get(m_pAccountED, "accounted");
    get(m_pSavePasswdBtn, "remember");
    get(m_pUseSysCredsCB, "syscreds");
    get(m_pOKBtn, "ok");

    OUString aRequest;
    if ((nFlags & LF_NO_ACCOUNT) != 0 && !rRealm.isEmpty())
    {
        aRequest = get<FixedText>("loginrealm")->GetText();
        aRequest = aRequest.replaceAll("%2", rRealm);
    }
    else
        aRequest = m_pRequestInfo->GetText();

    if ( !( ( nFlags & LF_NO_USESYSCREDS ) == LF_NO_USESYSCREDS ) )
      EnableUseSysCredsControls_Impl( m_pUseSysCredsCB->IsChecked() );

    aRequest = aRequest.replaceAll("%1", rServer);
    m_pRequestInfo->SetText(aRequest);

    m_pPathED->SetMaxTextLen( _MAX_PATH );
    m_pNameED->SetMaxTextLen( _MAX_PATH );

    m_pOKBtn->SetClickHdl( LINK( this, LoginDialog, OKHdl_Impl ) );
    m_pPathBtn->SetClickHdl( LINK( this, LoginDialog, PathHdl_Impl ) );
    m_pUseSysCredsCB->SetClickHdl( LINK( this, LoginDialog, UseSysCredsHdl_Impl ) );

    HideControls_Impl( nFlags );
}

void LoginDialog::SetUseSystemCredentials( sal_Bool bUse )
{
    if ( m_pUseSysCredsCB->IsVisible() )
    {
        m_pUseSysCredsCB->Check( bUse );
        EnableUseSysCredsControls_Impl( bUse );
    }
}

void LoginDialog::ClearPassword()
{
    m_pPasswordED->SetText( OUString() );

    if ( m_pNameED->GetText().isEmpty() )
        m_pNameED->GrabFocus();
    else
        m_pPasswordED->GrabFocus();
}

void LoginDialog::ClearAccount()
{
    m_pAccountED->SetText( OUString() );
    m_pAccountED->GrabFocus();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
