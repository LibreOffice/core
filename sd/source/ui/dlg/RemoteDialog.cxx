/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sdattr.hxx"
#include "sdresid.hxx"
#include "cusshow.hxx"

#include "RemoteDialog.hxx"
#include "RemoteServer.hxx"

using namespace ::sd;
using namespace ::std;

RemoteDialog::RemoteDialog( Window *pWindow )
    : ModalDialog(pWindow, "RemoteDialog",
        "modules/simpress/ui/remotedialog.ui")
{
    get(m_pButtonConnect, "connect");
    get(m_pButtonClose, "close");
    get(m_pClientBox, "tree");

#ifdef ENABLE_SDREMOTE
    RemoteServer::ensureDiscoverable();

    vector<::boost::shared_ptr<ClientInfo>> aClients( RemoteServer::getClients() );

    const vector<::boost::shared_ptr<ClientInfo>>::const_iterator aEnd( aClients.end() );

    for ( vector<::boost::shared_ptr<ClientInfo>>::const_iterator aIt( aClients.begin() );
        aIt != aEnd; ++aIt )
    {
        m_pClientBox->addEntry( *aIt );
    }
#endif

    m_pButtonConnect->SetClickHdl( LINK( this, RemoteDialog, HandleConnectButton ) );
    SetCloseHdl( LINK( this, RemoteDialog, CloseHdl ) );
    m_pButtonClose->SetClickHdl( LINK( this, RemoteDialog, CloseHdl ) );
}

IMPL_LINK_NOARG(RemoteDialog, HandleConnectButton)
{
//     setBusy( true );
    // Fixme: Try and connect
#if defined(ENABLE_SDREMOTE) && defined(ENABLE_SDREMOTE_BLUETOOTH)
    long aSelected = m_pClientBox->GetActiveEntryIndex();
    if ( aSelected < 0 )
        return 1;
    TClientBoxEntry aEntry = m_pClientBox->GetEntryData(aSelected);
    OUString aPin ( m_pClientBox->getPin() );
    if ( RemoteServer::connectClient( aEntry->m_pClientInfo, aPin ) )
    {
        return CloseHdl( 0 );
    }
    else
    {
        return 1;
    }
#else
    return 0;
#endif
}

IMPL_LINK_NOARG( RemoteDialog, CloseHdl )
{
#ifdef ENABLE_SDREMOTE
    RemoteServer::restoreDiscoverable();
#endif
    Close();
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
