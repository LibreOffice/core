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
#include <tubes/manager.hxx>
#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/lstbox.hxx>

#include <telepathy-glib/telepathy-glib.h>

namespace {

class TubeContacts : public ModelessDialog
{
    VclPtr<PushButton>             mpBtnDemo;
    VclPtr<PushButton>             mpBtnBuddy;
    VclPtr<PushButton>             mpBtnGroup;
    VclPtr<PushButton>             mpBtnInvite;
    VclPtr<PushButton>             mpBtnListen;
    VclPtr<ListBox>                mpList;
    Collaboration*          mpCollaboration;

    DECL_LINK_TYPED( BtnDemoHdl, Button*, void );
    DECL_LINK_TYPED( BtnConnectHdl, Button*, void );
    DECL_LINK_TYPED( BtnGroupHdl, Button*, void );
    DECL_LINK_TYPED( BtnInviteHdl, Button*, void );
    DECL_LINK_TYPED( BtnListenHdl, Button*, void );

    AccountContactPairV maACs;

    void Invite()
    {
        if (mpList->GetSelectEntryCount())
        {
            sal_uInt16 i = mpList->GetSelectEntryPos();
            TpContact* pContact = maACs[i].second;
            mpCollaboration->Invite( pContact );
        }
    }

    void Listen()
    {
        if (!TeleManager::registerClients())
            SAL_INFO( "tubes", "Could not register client handlers." );
    }

    void StartDemoSession()
    {
        TeleConference* pConference = TeleManager::startDemoSession();
        if (!pConference)
            SAL_WARN( "tubes", "Could not start demo session!" );
        else
        {
            mpCollaboration->StartCollaboration( pConference );
            mpCollaboration->SaveAndSendFile( NULL );
        }
    }

    void StartBuddySession()
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

    void StartGroupSession()
    {
        if (mpList->GetSelectEntryCount())
        {
            sal_uInt16 i = mpList->GetSelectEntryPos();
            TpAccount* pAccount = maACs[i].first;
            SAL_INFO( "tubes", "picked " << tp_account_get_display_name( pAccount ) );
            TeleConference* pConference = TeleManager::startGroupSession( pAccount,
                    OUString("liboroom"), OUString("conference.jabber.org") );
            if (!pConference)
                SAL_WARN( "tubes", "Could not start group session." );
            else
            {
                mpCollaboration->StartCollaboration( pConference );
            }
        }
    }

public:
    explicit TubeContacts( Collaboration* pCollaboration ) :
        ModelessDialog( NULL, "ContactsDialog", "tubes/ui/contacts.ui" ),
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
    virtual ~TubeContacts()
    {
        dispose();
    }

    virtual void dispose() override
    {
        mpBtnListen.clear();
        mpBtnGroup.clear();
        mpBtnDemo.clear();
        mpBtnBuddy.clear();
        mpBtnGroup.clear();
        mpList.clear();
    }

    static OUString fromUTF8( const char *pStr )
    {
        return OStringToOUString( OString( pStr, strlen( pStr ) ),
                                       RTL_TEXTENCODING_UTF8 );
    }

    void Populate()
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
                if( GRFILTER_OK == GraphicFilter::LoadGraphic( sAvatarFileUrl, OUString(""), aGraphic ) )
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
};

IMPL_LINK_NOARG_TYPED( TubeContacts, BtnDemoHdl, Button*, void )
{
    StartDemoSession();
}

IMPL_LINK_NOARG_TYPED( TubeContacts, BtnConnectHdl, Button*, void )
{
    StartBuddySession();
}

IMPL_LINK_NOARG_TYPED( TubeContacts, BtnGroupHdl, Button*, void )
{
    StartGroupSession();
}

IMPL_LINK_NOARG_TYPED( TubeContacts, BtnInviteHdl, Button*, void )
{
    Invite();
}

IMPL_LINK_NOARG_TYPED( TubeContacts, BtnListenHdl, Button*, void )
{
    Listen();
}

} // anonymous namespace

void Collaboration::DisplayContacts()
{
    if (!mpContacts)
        mpContacts = new TubeContacts( this );
    reinterpret_cast<TubeContacts*> (mpContacts)->Populate();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
