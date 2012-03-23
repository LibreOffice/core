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

ScCollaboration::ScCollaboration( const Link& rLink )
    :
        maLink( rLink),
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
    delete mpManager;
}


bool ScCollaboration::initManager()
{
    mpManager = new TeleManager( maLink);
    bool bOk = mpManager->connect();
    return bOk;
}


bool ScCollaboration::initAccountContact()
{
    ContactList aContacts;
    AccountContactPairV aVec( aContacts.getContacts());
    if (aVec.empty())
        return false;

    /* TODO: select a pair, for now just take the first */
    mpAccount = aVec[0].first;
    mpContact = aVec[0].second;
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


bool ScCollaboration::recvPacket( rtl::OString& rString, TeleConference* pConference )
{
    TelePacket aPacket;
    bool bOk = (pConference ? pConference->popPacket( aPacket) : mpManager->popPacket( aPacket));
    rString = rtl::OString( aPacket.getData(), aPacket.getSize());
    return bOk;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
