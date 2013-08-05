/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "PlaceEditDialog.hxx"
#include "ServerDetailsControls.hxx"

#include <officecfg/Office/Common.hxx>
#include <svtresid.hxx>
#include <vcl/msgbox.hxx>

using namespace boost;

PlaceEditDialog::PlaceEditDialog( Window* pParent ) :
    ModalDialog( pParent, "PlaceEditDialog", "svt/ui/placeedit.ui" ),
    m_pCurrentDetails( )
{
    get( m_pEDServerName, "name" );
    get( m_pLBServerType, "type" );
    get( m_pEDUsername, "login" );
    get( m_pBTOk, "ok" );
    get( m_pBTCancel, "cancel" );
    get( m_pBTDelete, "delete" );

    m_pBTOk->SetClickHdl( LINK( this, PlaceEditDialog, OKHdl) );
    m_pBTOk->Enable( sal_False );

    m_pEDServerName->SetModifyHdl( LINK( this, PlaceEditDialog, EditHdl) );

    // This constructor is called when user request a place creation, so
    // delete button is hidden.
    m_pBTDelete->Hide();

    m_pLBServerType->SetSelectHdl( LINK( this, PlaceEditDialog, SelectTypeHdl ) );
    m_pEDUsername->SetModifyHdl( LINK( this, PlaceEditDialog, EditUsernameHdl ) );

    InitDetails( );
}

PlaceEditDialog::PlaceEditDialog( Window* pParent, const boost::shared_ptr<Place>& pPlace ) :
    ModalDialog( pParent, "PlaceEditDialog", "svt/ui/placeedit.ui" ),
    m_pCurrentDetails( )
{
    get( m_pEDServerName, "name" );
    get( m_pLBServerType, "type" );
    get( m_pEDUsername, "login" );
    get( m_pBTOk, "ok" );
    get( m_pBTCancel, "cancel" );
    get( m_pBTDelete, "delete" );

    m_pBTOk->SetClickHdl( LINK( this, PlaceEditDialog, OKHdl) );
    m_pBTDelete->SetClickHdl ( LINK( this, PlaceEditDialog, DelHdl) );

    m_pEDServerName->SetModifyHdl( LINK( this, PlaceEditDialog, EditHdl) );
    m_pLBServerType->SetSelectHdl( LINK( this, PlaceEditDialog, SelectTypeHdl ) );

    InitDetails( );

    m_pEDServerName->SetText( pPlace->GetName() );

    // Fill the boxes with the URL parts
    bool bSuccess = false;
    for ( size_t i = 0 ; i < m_aDetailsContainers.size( ) && !bSuccess; ++i )
    {
        INetURLObject& rUrl = pPlace->GetUrlObject( );
        bSuccess = m_aDetailsContainers[i]->setUrl( rUrl );
        if ( bSuccess )
        {
            m_pLBServerType->SelectEntryPos( i );
            SelectTypeHdl( m_pLBServerType );

            // Fill the Username field
            if ( rUrl.HasUserData( ) )
                m_pEDUsername->SetText( rUrl.GetUser( ) );
        }
    }
}

PlaceEditDialog::~PlaceEditDialog()
{
}

OUString PlaceEditDialog::GetServerUrl()
{
    OUString sUrl;
    if ( m_pCurrentDetails.get( ) )
    {
        INetURLObject aUrl = m_pCurrentDetails->getUrl();
        OUString sUsername = OUString( m_pEDUsername->GetText( ) ).trim( );
        if ( !sUsername.isEmpty( ) )
            aUrl.SetUser( sUsername );
        if ( !aUrl.HasError( ) )
            sUrl = aUrl.GetMainURL( INetURLObject::NO_DECODE );
    }

    return sUrl;
}

boost::shared_ptr<Place> PlaceEditDialog::GetPlace()
{
    boost::shared_ptr<Place> newPlace( new Place( m_pEDServerName->GetText(), GetServerUrl(), true ) );
    return newPlace;
}

void PlaceEditDialog::InitDetails( )
{
    // Create WebDAV / FTP / SSH details control
    shared_ptr< DetailsContainer > pDavDetails( new DavDetailsContainer( this ) );
    pDavDetails->setChangeHdl( LINK( this, PlaceEditDialog, EditHdl ) );
    m_aDetailsContainers.push_back( pDavDetails );

    shared_ptr< DetailsContainer > pFtpDetails( new HostDetailsContainer( this, 21, "ftp" ) );
    pFtpDetails->setChangeHdl( LINK( this, PlaceEditDialog, EditHdl ) );
    m_aDetailsContainers.push_back( pFtpDetails );

    shared_ptr< DetailsContainer > pSshDetails( new HostDetailsContainer( this, 22, "ssh" ) );
    pSshDetails->setChangeHdl( LINK( this, PlaceEditDialog, EditHdl ) );
    m_aDetailsContainers.push_back( pSshDetails );

    // Create Windows Share control
    shared_ptr< DetailsContainer > pSmbDetails( new SmbDetailsContainer( this ) );
    pSmbDetails->setChangeHdl( LINK( this, PlaceEditDialog, EditHdl ) );
    m_aDetailsContainers.push_back( pSmbDetails );

    // Create CMIS control
    shared_ptr< DetailsContainer > pCmisDetails( new CmisDetailsContainer( this ) );
    pCmisDetails->setChangeHdl( LINK( this, PlaceEditDialog, EditHdl ) );
    m_aDetailsContainers.push_back( pCmisDetails );

    // Set default to first value
    m_pLBServerType->SelectEntryPos( 0 );
    SelectTypeHdl( m_pLBServerType );
}

IMPL_LINK ( PlaceEditDialog,  OKHdl, Button *, EMPTYARG )
{
    EndDialog( RET_OK );
    return 1;
}

IMPL_LINK ( PlaceEditDialog, DelHdl, Button *, EMPTYARG )
{
    // ReUsing existing symbols...
    EndDialog( RET_NO );
    return 1;
}

IMPL_LINK ( PlaceEditDialog, EditHdl, void *, EMPTYARG )
{
    OUString sUrl = GetServerUrl( );
    OUString sName = OUString( m_pEDServerName->GetText() ).trim( );
    m_pBTOk->Enable( !sName.isEmpty( ) && !sUrl.isEmpty( ) );
    return 1;
}

IMPL_LINK ( PlaceEditDialog, EditUsernameHdl, void *, EMPTYARG )
{
    for ( std::vector< boost::shared_ptr< DetailsContainer > >::iterator it = m_aDetailsContainers.begin( );
            it != m_aDetailsContainers.end( ); ++it )
    {
        ( *it )->setUsername( OUString( m_pEDUsername->GetText() ) );
    }
    return 1;
}

IMPL_LINK( PlaceEditDialog, SelectTypeHdl, void*, EMPTYARG )
{
    if ( m_pCurrentDetails.get( ) )
        m_pCurrentDetails->show( false );

    sal_uInt16 nPos = m_pLBServerType->GetSelectEntryPos( );
    m_pCurrentDetails = m_aDetailsContainers[nPos];

    m_pCurrentDetails->show( true );

    SetSizePixel(GetOptimalSize());
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
