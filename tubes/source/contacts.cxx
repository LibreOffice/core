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
 * Copyright (C) 2012 Michael Meeks <michael.meeks@suse.com> (initial developer)
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

#include <sal/config.h>

#include "contacts.hrc"
#include <svtools/filter.hxx>
#include <svx/simptabl.hxx>
#include <tools/resid.hxx>
#include <tubes/conference.hxx>
#include <tubes/collaboration.hxx>
#include <tubes/contact-list.hxx>
#include <tubes/manager.hxx>
#include <unotools/confignode.hxx>
#include <vcl/fixed.hxx>
#include <vcl/dialog.hxx>
#include <vcl/unohelp.hxx>

#include <map>
#include <vector>
#include <telepathy-glib/telepathy-glib.h>

namespace {

ResId TubesResId( sal_uInt32 nId )
{
    static ResMgr* pResMgr = NULL;
    if (!pResMgr)
    {
        pResMgr = ResMgr::CreateResMgr( "tubes" );
    }
    return ResId( nId, *pResMgr );
}

class TubeContacts : public ModelessDialog
{
    FixedLine               maLabel;
    PushButton              maBtnDemo;
    PushButton              maBtnConnect;
    PushButton              maBtnGroup;
    PushButton              maBtnInvite;
    PushButton              maBtnListen;
    SvxSimpleTableContainer maListContainer;
    SvxSimpleTable          maList;
    Collaboration*          mpCollaboration;

    DECL_LINK( BtnDemoHdl, void * );
    DECL_LINK( BtnConnectHdl, void * );
    DECL_LINK( BtnGroupHdl, void * );
    DECL_LINK( BtnInviteHdl, void * );
    DECL_LINK( BtnListenHdl, void * );

    AccountContactPairV maACs;

    void Invite()
    {
        AccountContactPair *pAC = NULL;
        if (maList.FirstSelected())
            pAC = static_cast<AccountContactPair*> (maList.FirstSelected()->GetUserData());
        if (pAC)
        {
            mpCollaboration->Invite( pAC->second );
        }
    }

    void Listen()
    {
        if (!TeleManager::registerClients())
            SAL_INFO( "sc.tubes", "Could not register client handlers." );
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
        AccountContactPair *pAC = NULL;
        if (maList.FirstSelected())
            pAC = static_cast<AccountContactPair*> (maList.FirstSelected()->GetUserData());
        if (pAC)
        {
            TpAccount* pAccount = pAC->first;
            TpContact* pContact = pAC->second;
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
        AccountContactPair *pAC = NULL;
        if (maList.FirstSelected())
            pAC = static_cast<AccountContactPair*> (maList.FirstSelected()->GetUserData());
        if (pAC)
        {
            TpAccount* pAccount = pAC->first;
            SAL_INFO( "tubes", "picked " << tp_account_get_display_name( pAccount ) );
            TeleConference* pConference = TeleManager::startGroupSession( pAccount,
                    rtl::OUString("liboroom"), rtl::OUString("conference.jabber.org") );
            if (!pConference)
                SAL_WARN( "tubes", "Could not start group session." );
            else
            {
                mpCollaboration->StartCollaboration( pConference );
            }
        }
    }

public:
    TubeContacts( Collaboration* pCollaboration ) :
        ModelessDialog( NULL, TubesResId( RID_TUBES_DLG_CONTACTS ) ),
        maLabel( this, TubesResId( FL_LABEL ) ),
        maBtnDemo( this, TubesResId( BTN_DEMO ) ),
        maBtnConnect( this, TubesResId( BTN_CONNECT ) ),
        maBtnGroup( this, TubesResId( BTN_GROUP ) ),
        maBtnInvite( this, TubesResId( BTN_INVITE ) ),
        maBtnListen( this, TubesResId( BTN_LISTEN ) ),
        maListContainer( this, TubesResId( CTL_LIST ) ),
        maList( maListContainer ),
        mpCollaboration( pCollaboration )
    {
        Hide();
        maBtnDemo.SetClickHdl( LINK( this, TubeContacts, BtnDemoHdl ) );
        maBtnConnect.SetClickHdl( LINK( this, TubeContacts, BtnConnectHdl ) );
        maBtnGroup.SetClickHdl( LINK( this, TubeContacts, BtnGroupHdl ) );
        maBtnInvite.SetClickHdl( LINK( this, TubeContacts, BtnInviteHdl ) );
        maBtnListen.SetClickHdl( LINK( this, TubeContacts, BtnListenHdl ) );

        static long aStaticTabs[]=
        {
            3 /* count */, 0, 20, 100, 150, 200
        };

        maList.SvxSimpleTable::SetTabs( aStaticTabs );
        String sHeader( '\t' );
        sHeader += String( TubesResId( STR_HEADER_ALIAS ) );
        sHeader += '\t';
        sHeader += String( TubesResId( STR_HEADER_NAME ) );
        sHeader += '\t';
        maList.InsertHeaderEntry( sHeader, HEADERBAR_APPEND, HIB_LEFT );
    }
    virtual ~TubeContacts()
    {
    }

    static rtl::OUString fromUTF8( const char *pStr )
    {
        return rtl::OStringToOUString( rtl::OString( pStr, strlen( pStr ) ),
                                       RTL_TEXTENCODING_UTF8 );
    }

    void Populate()
    {
        SAL_INFO( "sc.tubes", "Populating contact list dialog" );
        maList.Clear();
        maACs.clear();
        ContactList *pContacts = TeleManager::getContactList();
        if ( pContacts )
        {
            AccountContactPairV aPairs = pContacts->getContacts();
            AccountContactPairV::iterator it;
            // make sure we have enough memory to not need re-allocation
            // which would invalidate pointers stored in maList entries
            maACs.reserve( aPairs.size() );
            for( it = aPairs.begin(); it != aPairs.end(); ++it )
            {
                Image aImage;
                GFile *pAvatarFile = tp_contact_get_avatar_file( it->second );
                if( pAvatarFile )
                {
                    const rtl::OUString sAvatarFileUrl = fromUTF8( g_file_get_path ( pAvatarFile ) );
                    Graphic aGraphic;
                    if( GRFILTER_OK == GraphicFilter::LoadGraphic( sAvatarFileUrl, rtl::OUString(""), aGraphic ) )
                    {
                        BitmapEx aBitmap = aGraphic.GetBitmapEx();
                        double fScale = 30.0 / aBitmap.GetSizePixel().Height();
                        aBitmap.Scale( fScale, fScale );
                        aImage = Image( aBitmap );
                    }
                }
                rtl::OUStringBuffer aEntry( 128 );
                aEntry.append( sal_Unicode( '\t' ) );
                aEntry.append( fromUTF8 ( tp_contact_get_alias( it->second ) ) );
                aEntry.append( sal_Unicode( '\t' ) );
                aEntry.append( fromUTF8 ( tp_contact_get_identifier( it->second ) ) );
                aEntry.append( sal_Unicode( '\t' ) );
                SvLBoxEntry* pEntry = maList.InsertEntry( aEntry.makeStringAndClear(), aImage, aImage );
                // FIXME: ref the TpAccount, TpContact ...
                maACs.push_back( AccountContactPair( it->first, it->second ) );
                pEntry->SetUserData( &maACs.back() );

                g_object_unref (it->first);
                g_object_unref (it->second);
            }
        }
        Show();
    }
};

IMPL_LINK_NOARG( TubeContacts, BtnDemoHdl )
{
    StartDemoSession();
    return 0;
}

IMPL_LINK_NOARG( TubeContacts, BtnConnectHdl )
{
    StartBuddySession();
    return 0;
}

IMPL_LINK_NOARG( TubeContacts, BtnGroupHdl )
{
    StartGroupSession();
    return 0;
}

IMPL_LINK_NOARG( TubeContacts, BtnInviteHdl )
{
    Invite();
    return 0;
}

IMPL_LINK_NOARG( TubeContacts, BtnListenHdl )
{
    Listen();
    return 0;
}

} // anonymous namespace

void Collaboration::DisplayContacts()
{
    if (!mpContacts)
        mpContacts = new TubeContacts( this );
    reinterpret_cast<TubeContacts*> (mpContacts)->Populate();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
