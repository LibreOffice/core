/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <vcl/graphicfilter.hxx>
#include <tubes/conference.hxx>
#include <tubes/collaboration.hxx>

#include <telepathy-glib/telepathy-glib.h>

#include <contacts.hxx>

void tubes::TubeContacts::Invite()
{
    if (mpList->GetSelectEntryCount())
    {
        sal_uInt16 i = mpList->GetSelectEntryPos();
        TpContact* pContact = maACs[i].second;
        mpCollaboration->Invite( pContact );
    }
}

void tubes::TubeContacts::StartDemoSession()
{
    TeleConference* pConference = TeleManager::startDemoSession();
    if (!pConference)
        SAL_WARN( "tubes", "Could not start demo session!" );
    else
    {
        mpCollaboration->StartCollaboration( pConference );
        mpCollaboration->SaveAndSendFile( nullptr );
    }
}

void tubes::TubeContacts::StartBuddySession()
{
    if (mpList->GetSelectEntryCount())
    {
        sal_uInt16 i = mpList->GetSelectEntryPos();
        TpAccount* pAccount = maACs[i].first;
        TpContact* pContact = maACs[i].second;
        SAL_INFO( "tubes", "picked " << tp_contact_get_identifier( pContact ) );
        TeleConference* pConference = TeleManager::startBuddySession( pAccount, pContact );
        if (!pConference)
            SAL_WARN( "tubes", "Could not start session with " <<
                    tp_contact_get_identifier( pContact ) );
        else
        {
            mpCollaboration->StartCollaboration( pConference );
            mpCollaboration->SaveAndSendFile( pContact );
        }
    }
}

void tubes::TubeContacts::StartGroupSession()
{
    if (mpList->GetSelectEntryCount())
    {
        sal_uInt16 i = mpList->GetSelectEntryPos();
        TpAccount* pAccount = maACs[i].first;
        SAL_INFO( "tubes", "picked " << tp_account_get_display_name( pAccount ) );
        TeleConference* pConference = TeleManager::startGroupSession( pAccount,
                "liboroom", "conference.jabber.org" );
        if (!pConference)
            SAL_WARN( "tubes", "Could not start group session." );
        else
        {
            mpCollaboration->StartCollaboration( pConference );
        }
    }
}

tubes::TubeContacts::TubeContacts( Collaboration* pCollaboration ) :
    ModelessDialog( nullptr, "ContactsDialog", "tubes/ui/contacts.ui" ),
    mpCollaboration( pCollaboration )
{
    get( mpBtnListen, "listen");
    get( mpBtnInvite, "invite");
    get( mpBtnDemo, "demo");
    get( mpBtnBuddy, "buddy");
    get( mpBtnGroup, "group");
    get( mpList, "contacts");
    mpBtnListen->SetClickHdl( LINK( this, TubeContacts, BtnListenHdl ) );
    mpBtnInvite->SetClickHdl( LINK( this, TubeContacts, BtnInviteHdl ) );
    mpBtnDemo->SetClickHdl( LINK( this, TubeContacts, BtnDemoHdl ) );
    mpBtnBuddy->SetClickHdl( LINK( this, TubeContacts, BtnConnectHdl ) );
    mpBtnGroup->SetClickHdl( LINK( this, TubeContacts, BtnGroupHdl ) );
}

tubes::TubeContacts::~TubeContacts()
{
    disposeOnce();
}

void tubes::TubeContacts::dispose()
{
    mpBtnListen.disposeAndClear();
    mpBtnInvite.disposeAndClear();
    mpBtnDemo.disposeAndClear();
    mpBtnBuddy.disposeAndClear();
    mpBtnGroup.disposeAndClear();
    mpList.disposeAndClear();
    ModelessDialog::dispose();
}

namespace {

OUString fromUTF8( const char *pStr )
{
    return OStringToOUString( OString( pStr, strlen( pStr ) ),
                                   RTL_TEXTENCODING_UTF8 );
}

}

void tubes::TubeContacts::Populate()
{
    SAL_INFO( "tubes", "Populating contact list dialog" );
    mpList->Clear();
    maACs.clear();

    AccountContactPairV aPairs = TeleManager::getContacts();
    AccountContactPairV::iterator it;
    // make sure we have enough memory to not need re-allocation
    // which would invalidate pointers stored in mpList entries
    maACs.reserve( aPairs.size() );
    for( it = aPairs.begin(); it != aPairs.end(); ++it )
    {
        Image aImage;
        GFile *pAvatarFile = tp_contact_get_avatar_file( it->second );
        if( pAvatarFile )
        {
            const OUString sAvatarFileUrl = fromUTF8( g_file_get_path ( pAvatarFile ) );
            Graphic aGraphic;
            if( GRFILTER_OK == GraphicFilter::LoadGraphic( sAvatarFileUrl, "", aGraphic ) )
            {
                BitmapEx aBitmap = aGraphic.GetBitmapEx();
                double fScale = 30.0 / aBitmap.GetSizePixel().Height();
                aBitmap.Scale( fScale, fScale );
                aImage = Image( aBitmap );
            }
        }
        OUStringBuffer aEntry( 128 );
        aEntry.append( "    " );
        aEntry.append( fromUTF8 ( tp_contact_get_alias( it->second ) ) );
        aEntry.append( "    -    " );
        aEntry.append( fromUTF8 ( tp_contact_get_identifier( it->second ) ) );
        mpList->InsertEntry( aEntry.makeStringAndClear(), aImage);
        // FIXME: ref the TpAccount, TpContact ...
        maACs.push_back( AccountContactPair( it->first, it->second ) );

        g_object_unref (it->first);
        g_object_unref (it->second);
    }
    Show();
}

IMPL_LINK_NOARG( tubes::TubeContacts, BtnDemoHdl, Button*, void )
{
    StartDemoSession();
}

IMPL_LINK_NOARG( tubes::TubeContacts, BtnConnectHdl, Button*, void )
{
    StartBuddySession();
}

IMPL_LINK_NOARG( tubes::TubeContacts, BtnGroupHdl, Button*, void )
{
    StartGroupSession();
}

IMPL_LINK_NOARG( tubes::TubeContacts, BtnInviteHdl, Button*, void )
{
    Invite();
}

IMPL_STATIC_LINK_NOARG( tubes::TubeContacts, BtnListenHdl, Button*, void )
{
    if (!TeleManager::registerClients())
        SAL_INFO( "tubes", "Could not register client handlers." );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
