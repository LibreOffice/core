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
#include <tubes/contacts.hxx>
#include <tubes/manager.hxx>
#include <unotools/confignode.hxx>
#include <vcl/fixed.hxx>
#include <vcl/dialog.hxx>
#include <vcl/unohelp.hxx>

#include <map>
#include <vector>
#include <boost/ptr_container/ptr_vector.hpp>
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
    PushButton              maBtnConnect;
    PushButton              maBtnGroup;
    PushButton              maBtnInvite;
    PushButton              maBtnListen;
    SvxSimpleTableContainer maListContainer;
    SvxSimpleTable          maList;
    TeleManager*            mpManager;
    Collaboration*          mpCollaboration;

    DECL_LINK( BtnConnectHdl, void * );
    DECL_LINK( BtnGroupHdl, void * );
    DECL_LINK( BtnInviteHdl, void * );
    DECL_LINK( BtnListenHdl, void * );

    struct AccountContact
    {
        TpAccount* mpAccount;
        TpContact* mpContact;
        AccountContact( TpAccount* pAccount, TpContact* pContact ):
            mpAccount(pAccount), mpContact(pContact) {}
    };
    boost::ptr_vector<AccountContact> maACs;

    void Invite()
    {
        AccountContact *pAC = NULL;
        if (maList.FirstSelected())
            pAC = static_cast<AccountContact*> (maList.FirstSelected()->GetUserData());
        if (pAC)
        {
            if (mpCollaboration->GetConference())
            {
                TpContact* pContact = pAC->mpContact;
                mpCollaboration->GetConference()->invite( pContact );
                mpCollaboration->SendFile( pContact, OStringToOUString(
                            mpCollaboration->GetConference()->getUuid(), RTL_TEXTENCODING_UTF8 ) );
            }
        }
    }

    void Listen()
    {
        if (!mpManager->registerClients())
            SAL_INFO( "sc.tubes", "Could not register client handlers." );
    }

    void StartBuddySession()
    {
        AccountContact *pAC = NULL;
        if (maList.FirstSelected())
            pAC = static_cast<AccountContact*> (maList.FirstSelected()->GetUserData());
        if (pAC)
        {
            TpAccount* pAccount = pAC->mpAccount;
            TpContact* pContact = pAC->mpContact;
            fprintf( stderr, "picked %s\n", tp_contact_get_identifier( pContact ) );
            TeleConference* pConference = mpManager->startBuddySession( pAccount, pContact );
            if (!pConference)
                fprintf( stderr, "could not start session with %s\n",
                        tp_contact_get_identifier( pContact ) );
            else
            {
                mpCollaboration->SetCollaboration( pConference );
                mpCollaboration->SendFile( pContact, OStringToOUString(
                            pConference->getUuid(), RTL_TEXTENCODING_UTF8 ) );
            }
        }
    }

    void StartGroupSession()
    {
        AccountContact *pAC = NULL;
        if (maList.FirstSelected())
            pAC = static_cast<AccountContact*> (maList.FirstSelected()->GetUserData());
        if (pAC)
        {
            TpAccount* pAccount = pAC->mpAccount;
            fprintf( stderr, "picked %s\n", tp_account_get_display_name( pAccount ) );
            TeleConference* pConference = mpManager->startGroupSession( pAccount,
                    rtl::OUString("liboroom"), rtl::OUString("conference.jabber.org") );
            if (!pConference)
                fprintf( stderr, "could not start group session\n" );
            else
            {
                mpCollaboration->SetCollaboration( pConference );
            }
        }
    }

public:
    TubeContacts( Collaboration* pCollaboration ) :
        ModelessDialog( NULL, TubesResId( RID_TUBES_DLG_CONTACTS ) ),
        maLabel( this, TubesResId( FL_LABEL ) ),
        maBtnConnect( this, TubesResId( BTN_CONNECT ) ),
        maBtnGroup( this, TubesResId( BTN_GROUP ) ),
        maBtnInvite( this, TubesResId( BTN_INVITE ) ),
        maBtnListen( this, TubesResId( BTN_LISTEN ) ),
        maListContainer( this, TubesResId( CTL_LIST ) ),
        maList( maListContainer ),
        mpManager( new TeleManager() ),
        mpCollaboration( pCollaboration )
    {
        Hide();
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
        delete mpCollaboration;
        delete mpManager;
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
        ContactList *pContacts = mpManager->getContactList();
        if ( pContacts )
        {
            AccountContactPairV aPairs = pContacts->getContacts();
            AccountContactPairV::iterator it;
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
                maACs.push_back( new AccountContact( it->first, it->second ) );
                pEntry->SetUserData( &maACs.back() );
            }
        }
        Show();
    }
};

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

// Mapping contacts dialog instance for each document
typedef std::map< sal_uInt64, TubeContacts* > DialogsMap;
static DialogsMap aDialogsMap;

TubeContacts* ContactsFactory( Collaboration* pCollaboration )
{
    sal_uInt64 Id = pCollaboration->GetId();
    if (aDialogsMap.find( Id ) == aDialogsMap.end())
        aDialogsMap[ Id ] = new TubeContacts( pCollaboration );
    return aDialogsMap[ Id ];
}

} // anonymous namespace

namespace tubes {
void createContacts( Collaboration* pCollaboration )
{
    TubeContacts* pContacts = ContactsFactory( pCollaboration );
    pContacts->Populate();
}

void reDrawAllContacts()
{
    for (DialogsMap::const_iterator it = aDialogsMap.begin();
            it != aDialogsMap.end(); ++it)
        it->second->Populate();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
