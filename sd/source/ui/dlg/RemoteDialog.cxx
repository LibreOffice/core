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
#include "RemoteDialog.hrc"
#include "RemoteServer.hxx"

using namespace ::sd;
using namespace ::std;

RemoteDialog::RemoteDialog( Window *pWindow ) :
    ModalDialog( pWindow, SdResId( DLG_PAIR_REMOTE ) ),
    mButtonConnect(     this, SdResId( BTN_CONNECT ) ),
    mButtonCancel(      this, SdResId( BTN_CANCEL ) ),
    mClientBox(         this, NULL, SdResId( LB_SERVERS ) )
{
    FreeResource();

    vector<ClientInfo*> aClients( RemoteServer::getClients() );

    for ( vector<ClientInfo*>::const_iterator aIt( aClients.begin() );
        aIt < aClients.end(); aIt++ )
    {
        mClientBox.addEntry( *aIt );
    }

    mButtonConnect.SetClickHdl( LINK( this, RemoteDialog, HandleConnectButton ) );
}

RemoteDialog::~RemoteDialog()
{
}

// -----------------------------------------------------------------------
IMPL_LINK_NOARG(RemoteDialog, HandleConnectButton)
{
//     setBusy( true );
    // Fixme: Try and connect
    long aSelected = mClientBox.GetActiveEntryIndex();
    if ( aSelected < 0 )
        return 1;
    TClientBoxEntry aEntry = mClientBox.GetEntryData(aSelected);
    OUString aPin ( mClientBox.getPin() );
    if ( RemoteServer::connectClient( aEntry->m_pClientInfo, aPin ) )
    {
        Close();
        return 0;
    }
    else
        return 1;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */