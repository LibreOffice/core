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

#include "sal/config.h"

#include <vector>
#include "contacts.hrc"
#include "sendfunc.hxx"
#include "docsh.hxx"
#include "scresid.hxx"
#include <svtools/filter.hxx>
#include <tubes/manager.hxx>
#include <vcl/fixed.hxx>
#include <vcl/dialog.hxx>
#include <svx/simptabl.hxx>

#define CONTACTS_DLG

#ifdef CONTACTS_DLG
namespace {
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
    ScDocFuncSend*          mpSender;

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
        if (pAC && mpSender->GetConference())
        {
            TpContact* pContact = pAC->mpContact;
            fprintf( stderr, "inviting %s\n", tp_contact_get_identifier( pContact ) );
            mpSender->GetConference()->invite( pContact );
        }
    }

    void Listen()
    {
        if (!mpManager)
            return ;

        if (!mpManager->registerClients())
        {
            fprintf( stderr, "Could not register client handlers.\n" );
        }
        else
        {
            // FIXME: These signals should not be bind to a document specific code.

            // Receiving file is not related to any document.
            mpManager->sigFileReceived.connect( boost::bind(
                        &ScDocFuncRecv::fileReceived, mpSender->GetReceiver(), _1 ) );

            // TODO: It's still not clear to me who should take care of this signal
            // and what exactly it is supposed to happen.
            mpManager->sigConferenceCreated.connect( boost::bind(
                        &ScDocFuncSend::SetCollaboration, mpSender, _1 ) );
        }
    }

    void StartBuddySession()
    {
        if (!mpManager)
            return ;

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
                mpSender->SetCollaboration( pConference );
        }
    }

    void StartGroupSession()
    {
        if (!mpManager)
            return ;

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
                mpSender->SetCollaboration( pConference );
        }
    }

public:
    TubeContacts() :
        ModelessDialog( NULL, ScResId( RID_SCDLG_CONTACTS ) ),
        maLabel( this, ScResId( FL_LABEL ) ),
        maBtnConnect( this, ScResId( BTN_CONNECT ) ),
        maBtnGroup( this, ScResId( BTN_GROUP ) ),
        maBtnInvite( this, ScResId( BTN_INVITE ) ),
        maBtnListen( this, ScResId( BTN_LISTEN ) ),
        maListContainer( this, ScResId( CTL_LIST ) ),
        maList( maListContainer ),
        mpManager( TeleManager::get() )
    {
        Hide();
        ScDocShell *pScDocShell = dynamic_cast<ScDocShell*> (SfxObjectShell::Current());
        ScDocFunc *pDocFunc = pScDocShell ? &pScDocShell->GetDocFunc() : NULL;
        mpSender = dynamic_cast<ScDocFuncSend*> (pDocFunc);
        if (!mpSender)
        {
            // This means pDocFunc has to be ScDocFuncDirect* and we are not collaborating yet.
            ScDocFuncDirect *pDirect = dynamic_cast<ScDocFuncDirect*> (pDocFunc);
            ScDocFuncRecv *pReceiver = new ScDocFuncRecv( pDirect );
            mpSender = new ScDocFuncSend( *pScDocShell, pReceiver );
            pScDocShell->SetDocFunc( mpSender );

            // FIXME: Who should really own TeleManager and where it can be destroyed ?
            // Take reference, so TeleManager does not get destroyed after closing dialog:
            mpManager = TeleManager::get();

            if (mpManager->createAccountManager())
            {
                mpManager->prepareAccountManager();
            }
            else
            {
                fprintf( stderr, "Could not create AccountManager.\n" );
                mpManager->unref();
                mpManager = NULL;
            }
        }
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
        sHeader += String( ScResId( STR_HEADER_ALIAS ) );
        sHeader += '\t';
        sHeader += String( ScResId( STR_HEADER_NAME ) );
        sHeader += '\t';
        maList.InsertHeaderEntry( sHeader, HEADERBAR_APPEND, HIB_LEFT );
    }
    virtual ~TubeContacts()
    {
        if (mpManager)
            mpManager->unref();
    }

    static rtl::OUString fromUTF8( const char *pStr )
    {
        return rtl::OStringToOUString( rtl::OString( pStr, strlen( pStr ) ),
                                       RTL_TEXTENCODING_UTF8 );
    }

    void Populate()
    {
        if (!mpManager)
            return ;
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

} // anonymous namespace
#endif

namespace tubes {
void createContacts()
{
#ifdef CONTACTS_DLG
    TubeContacts *pContacts = new TubeContacts();
    pContacts->Populate();
#endif
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
