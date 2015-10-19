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
{
    get( m_pTVInstructions, "instructions" );
    get( m_pEDUrl, "url" );
    get( m_pEDCode, "code" );
    get( m_pBTOk, "ok" );
    get( m_pBTCancel, "cancel" );

    m_pBTOk->SetClickHdl( LINK( this, AuthFallbackDlg, OKHdl) );
    m_pBTCancel->SetClickHdl( LINK( this, AuthFallbackDlg, CancelHdl) );
    m_pBTOk->Enable();

    m_pTVInstructions->SetText( instructions );
    m_pEDUrl->SetText( url );
}

AuthFallbackDlg::~AuthFallbackDlg()
{
    disposeOnce();
}

void AuthFallbackDlg::dispose()
{
    m_pTVInstructions.clear();
    m_pEDUrl.clear();
    m_pEDCode.clear();
    m_pBTOk.clear();
    m_pBTCancel.clear();
    ModalDialog::dispose();
}

IMPL_LINK_NOARG_TYPED ( AuthFallbackDlg,  OKHdl, Button *, void)
{
    EndDialog( RET_OK );
}

IMPL_LINK_NOARG_TYPED ( AuthFallbackDlg,  CancelHdl, Button *, void)
{
    EndDialog();
}
