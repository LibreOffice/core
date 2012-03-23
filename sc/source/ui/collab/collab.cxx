/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2012 Red Hat, Inc., Eike Rathke <erack@redhat.com>
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include "collab.hxx"
#include <tubes/manager.hxx>
#include <tubes/conference.hxx>
#include <tubes/contact-list.hxx>

ScCollaboration::ScCollaboration()
    :
        mpAccount( NULL),
        mpContact( NULL),
        mpManager( NULL)
{
}


ScCollaboration::~ScCollaboration()
{
    if (mpAccount)
        g_object_unref( mpAccount);
    if (mpContact)
        g_object_unref( mpContact);

    mpManager->unref();
    mpManager = NULL;
}


void ScCollaboration::receivedFile( rtl::OUString &rFileURL )
{
    fprintf( stderr, "file recieved '%s'\n",
             rtl::OUStringToOString( rFileURL, RTL_TEXTENCODING_UTF8 ).getStr() );
    sigFileReceived( &rFileURL );
}

void ScCollaboration::packetReceivedCallback( TeleConference *pConference, TelePacket &rPacket )
{
    rtl::OString aString( rPacket.getData(), rPacket.getSize());
    /* Relay the signal out… */
    sigPacketReceived( pConference, aString);
}

bool ScCollaboration::initManager()
{
    mpManager = TeleManager::get();
    mpManager->sigPacketReceived.connect(
        boost::bind( &ScCollaboration::packetReceivedCallback, this, _1, _2 ));
    mpManager->connect();
    mpManager->prepareAccountManager();
    mpManager->sigFileReceived.connect(
        boost::bind( &ScCollaboration::receivedFile, this, _1 ));
    return true;
}


bool ScCollaboration::initAccountContact()
{
    ContactList* pContactList = mpManager->getContactList();
    AccountContactPairV aVec( pContactList->getContacts());

    fprintf( stderr, "%u contacts\n", (int) aVec.size() );
    if (aVec.empty())
        return false;

    /* TODO: select a pair, for now just take the first */
    mpAccount = aVec[0].first;
    mpContact = aVec[0].second;
    fprintf( stderr, "picked %s\n", tp_contact_get_identifier( mpContact ) );
    return true;
}


bool ScCollaboration::startCollaboration()
{
    bool bOk = mpManager->startBuddySession( mpAccount, mpContact);
    return bOk;
}


bool ScCollaboration::sendPacket( const rtl::OString& rString )
{
    TelePacket aPacket( "sender", rString.getStr(), rString.getLength());
    bool bOk = mpManager->sendPacket( aPacket);
    return bOk;
}


extern "C" {
    static void file_sent_cb( bool aSuccess, void* /* pUserData */ )
    {
        fprintf( stderr, "File send %s\n", aSuccess ? "success" : "failed" );
    }
}

void ScCollaboration::sendFile( rtl::OUString &rFileURL )
{
    mpManager->sendFile( rFileURL, file_sent_cb, NULL );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
