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
#include <utility>
#include "logindlg.hxx"

#ifdef UNX
#include <limits.h>
#define _MAX_PATH PATH_MAX
#endif

LoginDialog::~LoginDialog()
{
}

void LoginDialog::SetPassword( const OUString& rNew )
{
    m_xPasswordED->set_text( rNew );
    SetRequest();
}

void LoginDialog::HideControls_Impl( LoginFlags nFlags )
{
    if ( nFlags & LoginFlags::UsernameReadonly )
    {
        m_xNameED->set_sensitive( false );
    }

    if ( nFlags & LoginFlags::NoSavePassword )
        m_xSavePasswdBtn->hide();

    if ( nFlags & LoginFlags::NoErrorText )
    {
        m_xErrorInfo->hide();
        m_xErrorFT->hide();
    }

    if ( nFlags & LoginFlags::NoAccount )
    {
        m_xAccountFT->hide();
        m_xAccountED->hide();
    }

    if ( nFlags & LoginFlags::NoUseSysCreds )
    {
        m_xUseSysCredsCB->hide();
    }
}

void LoginDialog::EnableUseSysCredsControls_Impl( bool bUseSysCredsset_sensitived )
{
    m_xErrorInfo->set_sensitive( !bUseSysCredsset_sensitived );
    m_xErrorFT->set_sensitive( !bUseSysCredsset_sensitived );
    m_xRequestInfo->set_sensitive( !bUseSysCredsset_sensitived );
    m_xNameFT->set_sensitive( !bUseSysCredsset_sensitived );
    m_xNameED->set_sensitive( !bUseSysCredsset_sensitived );
    m_xPasswordFT->set_sensitive( !bUseSysCredsset_sensitived );
    m_xPasswordED->set_sensitive( !bUseSysCredsset_sensitived );
    m_xAccountFT->set_sensitive( !bUseSysCredsset_sensitived );
    m_xAccountED->set_sensitive( !bUseSysCredsset_sensitived );
}

void LoginDialog::SetRequest()
{
    bool oldPwd = !m_xPasswordED->get_text().isEmpty();
    OUString aRequest;
    if (m_xAccountFT->get_visible() && !m_realm.isEmpty())
    {
        std::unique_ptr<weld::Label> xText(m_xBuilder->weld_label(oldPwd ? u"wrongloginrealm"_ustr : u"loginrealm"_ustr));
        aRequest = xText->get_label();
        aRequest = aRequest.replaceAll("%2", m_realm);
    }
    else
    {
        std::unique_ptr<weld::Label> xText(m_xBuilder->weld_label(oldPwd ? u"wrongrequestinfo"_ustr : u"requestinfo"_ustr));
        aRequest = xText->get_label();
    }
    aRequest = aRequest.replaceAll("%1", m_server);
    m_xRequestInfo->set_label(aRequest);
}

IMPL_LINK_NOARG(LoginDialog, OKHdl_Impl, weld::Button&, void)
{
    // trim the strings
    m_xNameED->set_text(comphelper::string::strip(m_xNameED->get_text(), ' '));
    m_xPasswordED->set_text(comphelper::string::strip(m_xPasswordED->get_text(), ' '));
    m_xDialog->response(RET_OK);
}

IMPL_LINK_NOARG(LoginDialog, UseSysCredsHdl_Impl, weld::Toggleable&, void)
{
    EnableUseSysCredsControls_Impl( m_xUseSysCredsCB->get_active() );
}

LoginDialog::LoginDialog(weld::Window* pParent, LoginFlags nFlags,
    OUString aServer, OUString aRealm)
    : GenericDialogController(pParent, u"uui/ui/logindialog.ui"_ustr, u"LoginDialog"_ustr)
    , m_xErrorFT(m_xBuilder->weld_label(u"errorft"_ustr))
    , m_xErrorInfo(m_xBuilder->weld_label(u"errorinfo"_ustr))
    , m_xRequestInfo(m_xBuilder->weld_label(u"requestinfo"_ustr))
    , m_xNameFT(m_xBuilder->weld_label(u"nameft"_ustr))
    , m_xNameED(m_xBuilder->weld_entry(u"nameed"_ustr))
    , m_xPasswordFT(m_xBuilder->weld_label(u"passwordft"_ustr))
    , m_xPasswordED(m_xBuilder->weld_entry(u"passworded"_ustr))
    , m_xAccountFT(m_xBuilder->weld_label(u"accountft"_ustr))
    , m_xAccountED(m_xBuilder->weld_entry(u"accounted"_ustr))
    , m_xSavePasswdBtn(m_xBuilder->weld_check_button(u"remember"_ustr))
    , m_xUseSysCredsCB(m_xBuilder->weld_check_button(u"syscreds"_ustr))
    , m_xOKBtn(m_xBuilder->weld_button(u"ok"_ustr))
    , m_server(std::move(aServer)), m_realm(std::move(aRealm))
{
    if ( !( nFlags & LoginFlags::NoUseSysCreds ) )
      EnableUseSysCredsControls_Impl( m_xUseSysCredsCB->get_active() );

    SetRequest();

    m_xNameED->set_max_length( _MAX_PATH );

    m_xOKBtn->connect_clicked( LINK( this, LoginDialog, OKHdl_Impl ) );
    m_xUseSysCredsCB->connect_toggled( LINK( this, LoginDialog, UseSysCredsHdl_Impl ) );

    HideControls_Impl( nFlags );
}

void LoginDialog::SetUseSystemCredentials( bool bUse )
{
    if ( m_xUseSysCredsCB->get_visible() )
    {
        m_xUseSysCredsCB->set_active( bUse );
        EnableUseSysCredsControls_Impl( bUse );
    }
}

void LoginDialog::ClearPassword()
{
    m_xPasswordED->set_text( OUString() );

    if ( m_xNameED->get_text().isEmpty() )
        m_xNameED->grab_focus();
    else
        m_xPasswordED->grab_focus();
}

void LoginDialog::ClearAccount()
{
    m_xAccountED->set_text( OUString() );
    m_xAccountED->grab_focus();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
