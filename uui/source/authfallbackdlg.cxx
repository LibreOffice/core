/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "authfallbackdlg.hxx"

#include <vcl/msgbox.hxx>
#include <iostream>

using namespace boost;

AuthFallbackDlg::AuthFallbackDlg(Window* pParent, const OUString& instructions,
                                 const OUString& url)
    : ModalDialog(pParent, "AuthFallbackDlg", "uui/ui/authfallback.ui")
    , m_bGoogleMode( false )
{
    get( m_pTVInstructions, "instructions" );
    get( m_pEDUrl, "url" );
    get( m_pEDCode, "code" );
    get( m_pEDGoogleCode, "google_code" );
    get( m_pBTOk, "ok" );
    get( m_pBTCancel, "cancel" );
    get( m_pGoogleBox, "GDrive" );
    get( m_pOneDriveBox, "OneDrive" );

    m_pBTOk->SetClickHdl( LINK( this, AuthFallbackDlg, OKHdl) );
    m_pBTCancel->SetClickHdl( LINK( this, AuthFallbackDlg, CancelHdl) );
    m_pBTOk->Enable();

    m_pTVInstructions->SetText( instructions );
    m_pTVInstructions->SetPaintTransparent(true);
    if( url.isEmpty() )
    {
        // Google 2FA
        m_bGoogleMode = true;
        m_pGoogleBox->Show();
        m_pOneDriveBox->Hide();
        m_pEDUrl->Hide();
    }
    else
    {
        // OneDrive
        m_bGoogleMode = false;
        m_pGoogleBox->Hide();
        m_pOneDriveBox->Show();
        m_pEDUrl->SetText( url );
    }
}

AuthFallbackDlg::~AuthFallbackDlg()
{
    disposeOnce();
}

OUString AuthFallbackDlg::GetCode() const
{
    if( m_bGoogleMode )
        return m_pEDGoogleCode->GetText();
    else
        return m_pEDCode->GetText();
}


void AuthFallbackDlg::dispose()
{
    m_pTVInstructions.clear();
    m_pEDUrl.clear();
    m_pEDCode.clear();
    m_pEDGoogleCode.clear();
    m_pBTOk.clear();
    m_pBTCancel.clear();
    m_pGoogleBox.clear();
    m_pOneDriveBox.clear();
    ModalDialog::dispose();
}

IMPL_LINK_NOARG ( AuthFallbackDlg,  OKHdl, Button *, void)
{
    EndDialog( RET_OK );
}

IMPL_LINK_NOARG ( AuthFallbackDlg,  CancelHdl, Button *, void)
{
    EndDialog();
}
