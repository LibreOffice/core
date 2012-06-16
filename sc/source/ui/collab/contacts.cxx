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
    SvxSimpleTableContainer maListContainer;
    SvxSimpleTable          maList;

    DECL_LINK( BtnConnectHdl, void * );

    struct AccountContact
    {
        TpAccount* mpAccount;
        TpContact* mpContact;
        AccountContact( TpAccount* pAccount, TpContact* pContact ):
            mpAccount(pAccount), mpContact(pContact) {}
    };
    boost::ptr_vector<AccountContact> maACs;

    void StartBuddySession()
    {
        AccountContact *pAC = NULL;
        if (maList.FirstSelected())
            pAC = reinterpret_cast<AccountContact*> (maList.FirstSelected()->GetUserData());
        if (pAC)
        {
            TpAccount* pAccount = pAC->mpAccount;
            TpContact* pContact = pAC->mpContact;
            fprintf( stderr, "picked %s\n", tp_contact_get_identifier( pContact ) );
            // TeleManager has to exist already, false will be ignored:
            TeleManager *pManager = TeleManager::get( false );
            if (!pManager->startBuddySession( pAccount, pContact ))
                fprintf( stderr, "could not start session with %s\n",
                        tp_contact_get_identifier( pContact ) );
            pManager->unref();
        }
    }

public:
    TubeContacts() :
        ModelessDialog( NULL, ScResId( RID_SCDLG_CONTACTS ) ),
        maLabel( this, ScResId( FL_LABEL ) ),
        maBtnConnect( this, ScResId( BTN_CONNECT ) ),
        maListContainer( this, ScResId( CTL_LIST ) ),
        maList( maListContainer )
    {
        maBtnConnect.SetClickHdl( LINK( this, TubeContacts, BtnConnectHdl ) );

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
        Show();
    }
    virtual ~TubeContacts() {}

    static rtl::OUString fromUTF8( const char *pStr )
    {
        return rtl::OStringToOUString( rtl::OString( pStr, strlen( pStr ) ),
                                       RTL_TEXTENCODING_UTF8 );
    }

    void Populate( const TeleManager &rManager )
    {
        ContactList *pContacts = rManager.getContactList();
        if ( pContacts )
        {
            fprintf( stderr, "contacts !\n" );
            AccountContactPairV aPairs = pContacts->getContacts();
            AccountContactPairV::iterator it;
            for( it = aPairs.begin(); it != aPairs.end(); it++ )
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
                fprintf( stderr, "'%s' => '%s' '%s'\n",
                         tp_account_get_display_name( it->first ),
                         tp_contact_get_alias( it->second ),
                         tp_contact_get_identifier( it->second ) );
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
    }
};

IMPL_LINK_NOARG( TubeContacts, BtnConnectHdl )
{
    StartBuddySession();
    Close();
    return 0;
}

} // anonymous namespace
#endif

namespace tubes {
void createContacts( const TeleManager &rManager )
{
#ifdef CONTACTS_DLG
    TubeContacts *pContacts = new TubeContacts();
    pContacts->Populate( rManager );
#endif
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
