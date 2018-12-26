/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "authfallbackdlg.hxx"

#include <iostream>

AuthFallbackDlg::AuthFallbackDlg(weld::Window* pParent, const OUString& instructions,
                                 const OUString& url)
    : GenericDialogController(pParent, "uui/ui/authfallback.ui", "AuthFallbackDlg")
    , m_bGoogleMode( false )
    , m_xTVInstructions(m_xBuilder->weld_label("instructions"))
    , m_xEDUrl(m_xBuilder->weld_entry("url"))
    , m_xEDCode(m_xBuilder->weld_entry("code"))
    , m_xEDGoogleCode(m_xBuilder->weld_entry("google_code"))
    , m_xBTOk(m_xBuilder->weld_button("ok"))
    , m_xBTCancel(m_xBuilder->weld_button("cancel"))
    , m_xGoogleBox(m_xBuilder->weld_widget("GDrive"))
    , m_xOneDriveBox(m_xBuilder->weld_widget("OneDrive"))
{
    m_xBTOk->connect_clicked( LINK( this, AuthFallbackDlg, OKHdl) );
    m_xBTCancel->connect_clicked( LINK( this, AuthFallbackDlg, CancelHdl) );
    m_xBTOk->set_sensitive(true);

    m_xTVInstructions->set_label(instructions);
    if( url.isEmpty() )
    {
        // Google 2FA
        m_bGoogleMode = true;
        m_xGoogleBox->show();
        m_xOneDriveBox->hide();
        m_xEDUrl->hide();
    }
    else
    {
        // OneDrive
        m_bGoogleMode = false;
        m_xGoogleBox->hide();
        m_xOneDriveBox->show();
        m_xEDUrl->set_text( url );
    }
}

AuthFallbackDlg::~AuthFallbackDlg()
{
}

OUString AuthFallbackDlg::GetCode() const
{
    if( m_bGoogleMode )
        return m_xEDGoogleCode->get_text();
    else
        return m_xEDCode->get_text();
}

IMPL_LINK_NOARG(AuthFallbackDlg,  OKHdl, weld::Button&, void)
{
    m_xDialog->response(RET_OK);
}

IMPL_LINK_NOARG(AuthFallbackDlg,  CancelHdl, weld::Button&, void)
{
    m_xDialog->response(RET_CANCEL);
}
