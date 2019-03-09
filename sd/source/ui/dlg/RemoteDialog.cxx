/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "RemoteDialog.hxx"
#include <RemoteServer.hxx>

using namespace ::sd;
using namespace ::std;

RemoteDialog::RemoteDialog( vcl::Window *pWindow )
    : ModalDialog(pWindow, "RemoteDialog",
        "modules/simpress/ui/remotedialog.ui")
{
    get(m_pButtonConnect, "connect");
    get(m_pButtonClose, "close");
    get(m_pClientBox, "tree");

    m_pButtonConnect->SetClickHdl( LINK( this, RemoteDialog, HandleConnectButton ) );
    SetCloseHdl( LINK( this, RemoteDialog, CloseHdl ) );
    m_pButtonClose->SetClickHdl( LINK( this, RemoteDialog, CloseClickHdl ) );
}

RemoteDialog::~RemoteDialog()
{
    disposeOnce();
}

void RemoteDialog::dispose()
{
    m_pButtonConnect.clear();
    m_pButtonClose.clear();
    m_pClientBox.clear();
    ModalDialog::dispose();
}

IMPL_LINK_NOARG(RemoteDialog, HandleConnectButton, Button*, void)
{
//     setBusy( true );
    // Fixme: Try and connect
#if defined(ENABLE_SDREMOTE) && defined(ENABLE_SDREMOTE_BLUETOOTH)
    long aSelected = m_pClientBox->GetActiveEntryIndex();
    if ( aSelected < 0 )
        return;
    TClientBoxEntry aEntry = m_pClientBox->GetEntryData(aSelected);
    OUString aPin ( m_pClientBox->getPin() );
    if ( RemoteServer::connectClient( aEntry->m_pClientInfo, aPin ) )
    {
        CloseHdl( *this );
    }
#else
    (void) this;
#endif
}

IMPL_LINK_NOARG( RemoteDialog, CloseClickHdl, Button*, void )
{
    CloseHdl(*this);
}
IMPL_LINK_NOARG( RemoteDialog, CloseHdl, SystemWindow&, void )
{
#ifdef ENABLE_SDREMOTE
    RemoteServer::restoreDiscoverable();
#endif
    Close();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
