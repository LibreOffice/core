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
 * [ Copyright (C) 2012 Julien Levesy <jlevesy@gmail.com> (initial developer) ]
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

#include "PlaceEditDialog.hrc"
#include "PlaceEditDialog.hxx"
#include "ServerDetailsControls.hxx"

#include <officecfg/Office/Common.hxx>
#include <svtools/svtresid.hxx>
#include <vcl/msgbox.hxx>

using namespace boost;

PlaceEditDialog::PlaceEditDialog(	Window* pParent ) :
    ModalDialog( pParent, SvtResId( DLG_FPICKER_PLACE_EDIT ) ),
    m_aFTServerName( this, SvtResId( FT_ADDPLACE_SERVERNAME ) ),
    m_aFTServerType( this, SvtResId( FT_ADDPLACE_SERVERTYPE ) ),
    m_aEDServerName ( this, SvtResId( ED_ADDPLACE_SERVERNAME ) ),
    m_aLBServerType ( this, SvtResId( LB_ADDPLACE_SERVERTYPE ) ),
    m_pCurrentDetails( ),
    m_aFTHost( this, SvtResId( FT_ADDPLACE_HOST ) ),
    m_aEDHost( this, SvtResId( ED_ADDPLACE_HOST ) ),
    m_aFTPort( this, SvtResId( FT_ADDPLACE_PORT ) ),
    m_aEDPort( this, SvtResId( ED_ADDPLACE_PORT ) ),
    m_aFTPath( this, SvtResId( FT_ADDPLACE_PATH ) ),
    m_aEDPath( this, SvtResId( ED_ADDPLACE_PATH ) ),
    m_aCBDavs( this, SvtResId( CB_ADDPLACE_DAVS ) ),
    m_aEDSmbHost( this, SvtResId( ED_ADDPLACE_SMBHOST ) ),
    m_aFTShare( this, SvtResId( FT_ADDPLACE_SHARE ) ),
    m_aEDShare( this, SvtResId( ED_ADDPLACE_SHARE ) ),
    m_aFTSmbPath( this, SvtResId( FT_ADDPLACE_SMBPATH ) ),
    m_aEDSmbPath( this, SvtResId( ED_ADDPLACE_SMBPATH ) ),
    m_aFTCmisBinding( this, SvtResId( FT_ADDPLACE_CMIS_BINDING ) ),
    m_aEDCmisBinding( this, SvtResId( ED_ADDPLACE_CMIS_BINDING ) ),
    m_aFTCmisRepository( this, SvtResId( FT_ADDPLACE_CMIS_REPOSITORY ) ),
    m_aLBCmisRepository( this, SvtResId( LB_ADDPLACE_CMIS_REPOSITORY ) ),
    m_aBTCmisRepoRefresh( this, SvtResId( BT_ADDPLACE_CMIS_REPOREFRESH ) ),
    m_aFTUsername( this, SvtResId( FT_ADDPLACE_USERNAME ) ),
    m_aEDUsername( this, SvtResId( ED_ADDPLACE_USERNAME ) ),
    m_aBTOk( this, SvtResId( BT_ADDPLACE_OK ) ),
    m_aBTCancel ( this, SvtResId ( BT_ADDPLACE_CANCEL ) ),
    m_aBTDelete ( this, SvtResId (BT_ADDPLACE_DELETE ) )
{
    m_aBTOk.SetClickHdl( LINK( this, PlaceEditDialog, OKHdl) );
    m_aBTOk.Enable( sal_False );

    m_aEDServerName.SetModifyHdl( LINK( this, PlaceEditDialog, EditHdl) );

    // This constructor is called when user request a place creation, so
    // delete button is hidden.
    m_aBTDelete.Hide();

    m_aLBServerType.SetSelectHdl( LINK( this, PlaceEditDialog, SelectTypeHdl ) );
    m_aEDUsername.SetModifyHdl( LINK( this, PlaceEditDialog, EditUsernameHdl ) );

    InitDetails( );
}

PlaceEditDialog::PlaceEditDialog( Window* pParent, const boost::shared_ptr<Place>& pPlace ) :
    ModalDialog( pParent, SvtResId( DLG_FPICKER_PLACE_EDIT ) ),
    m_aFTServerName( this, SvtResId( FT_ADDPLACE_SERVERNAME ) ),
    m_aFTServerType( this, SvtResId( FT_ADDPLACE_SERVERTYPE ) ),
    m_aEDServerName ( this, SvtResId( ED_ADDPLACE_SERVERNAME ) ),
    m_aLBServerType ( this, SvtResId( LB_ADDPLACE_SERVERTYPE ) ),
    m_pCurrentDetails( ),
    m_aFTHost( this, SvtResId( FT_ADDPLACE_HOST ) ),
    m_aEDHost( this, SvtResId( ED_ADDPLACE_HOST ) ),
    m_aFTPort( this, SvtResId( FT_ADDPLACE_PORT ) ),
    m_aEDPort( this, SvtResId( ED_ADDPLACE_PORT ) ),
    m_aFTPath( this, SvtResId( FT_ADDPLACE_PATH ) ),
    m_aEDPath( this, SvtResId( ED_ADDPLACE_PATH ) ),
    m_aCBDavs( this, SvtResId( CB_ADDPLACE_DAVS ) ),
    m_aEDSmbHost( this, SvtResId( ED_ADDPLACE_SMBHOST ) ),
    m_aFTShare( this, SvtResId( FT_ADDPLACE_SHARE ) ),
    m_aEDShare( this, SvtResId( ED_ADDPLACE_SHARE ) ),
    m_aFTSmbPath( this, SvtResId( FT_ADDPLACE_SMBPATH ) ),
    m_aEDSmbPath( this, SvtResId( ED_ADDPLACE_SMBPATH ) ),
    m_aFTCmisBinding( this, SvtResId( FT_ADDPLACE_CMIS_BINDING ) ),
    m_aEDCmisBinding( this, SvtResId( ED_ADDPLACE_CMIS_BINDING ) ),
    m_aFTCmisRepository( this, SvtResId( FT_ADDPLACE_CMIS_REPOSITORY ) ),
    m_aLBCmisRepository( this, SvtResId( LB_ADDPLACE_CMIS_REPOSITORY ) ),
    m_aBTCmisRepoRefresh( this, SvtResId( BT_ADDPLACE_CMIS_REPOREFRESH ) ),
    m_aFTUsername( this, SvtResId( FT_ADDPLACE_USERNAME ) ),
    m_aEDUsername( this, SvtResId( ED_ADDPLACE_USERNAME ) ),
    m_aBTOk( this, SvtResId( BT_ADDPLACE_OK ) ),
    m_aBTCancel ( this, SvtResId ( BT_ADDPLACE_CANCEL ) ),
    m_aBTDelete ( this, SvtResId (BT_ADDPLACE_DELETE ) )
{
    m_aBTOk.SetClickHdl( LINK( this, PlaceEditDialog, OKHdl) );
    m_aBTDelete.SetClickHdl ( LINK( this, PlaceEditDialog, DelHdl) );

    m_aEDServerName.SetModifyHdl( LINK( this, PlaceEditDialog, EditHdl) );
    m_aLBServerType.SetSelectHdl( LINK( this, PlaceEditDialog, SelectTypeHdl ) );

    InitDetails( );

    m_aEDServerName.SetText( pPlace->GetName() );

    // Fill the boxes with the URL parts
    bool bSuccess = false;
    for ( size_t i = 0 ; i < m_aDetailsContainers.size( ) && !bSuccess; ++i )
    {
        INetURLObject& rUrl = pPlace->GetUrlObject( );
        bSuccess = m_aDetailsContainers[i]->setUrl( rUrl );
        if ( bSuccess )
        {
            m_aLBServerType.SelectEntryPos( i );
            SelectTypeHdl( &m_aLBServerType );

            // Fill the Username field
            if ( rUrl.HasUserData( ) )
                m_aEDUsername.SetText( rUrl.GetUser( ) );
        }
    }
}

PlaceEditDialog::~PlaceEditDialog()
{
}

rtl::OUString PlaceEditDialog::GetServerUrl()
{
    rtl::OUString sUrl;
    if ( m_pCurrentDetails.get( ) )
    {
        INetURLObject aUrl = m_pCurrentDetails->getUrl();
        rtl::OUString sUsername = rtl::OUString( m_aEDUsername.GetText( ) ).trim( );
        if ( !sUsername.isEmpty( ) )
            aUrl.SetUser( sUsername );
        if ( !aUrl.HasError( ) )
            sUrl = aUrl.GetMainURL( INetURLObject::NO_DECODE );
    }

    return sUrl;
}

boost::shared_ptr<Place> PlaceEditDialog::GetPlace()
{
    boost::shared_ptr<Place> newPlace( new Place( m_aEDServerName.GetText(), GetServerUrl(), true ) );
    return newPlace;
}

void PlaceEditDialog::InitDetails( )
{
    // Create WebDAV / FTP / SSH details control
    shared_ptr< DetailsContainer > pDavDetails( new DavDetailsContainer( ) );
    pDavDetails->addControl( FT_ADDPLACE_HOST, &m_aFTHost );
    pDavDetails->addControl( ED_ADDPLACE_HOST, &m_aEDHost );
    pDavDetails->addControl( FT_ADDPLACE_PORT, &m_aFTPort );
    pDavDetails->addControl( ED_ADDPLACE_PORT, &m_aEDPort );
    pDavDetails->addControl( FT_ADDPLACE_PATH, &m_aFTPath );
    pDavDetails->addControl( ED_ADDPLACE_PATH, &m_aEDPath );
    pDavDetails->addControl( CB_ADDPLACE_DAVS, &m_aCBDavs );
    pDavDetails->setChangeHdl( LINK( this, PlaceEditDialog, EditHdl ) );
    m_aDetailsContainers.push_back( pDavDetails );

    shared_ptr< DetailsContainer > pFtpDetails( new HostDetailsContainer( 21, "ftp" ) );
    pFtpDetails->addControl( FT_ADDPLACE_HOST, &m_aFTHost );
    pFtpDetails->addControl( ED_ADDPLACE_HOST, &m_aEDHost );
    pFtpDetails->addControl( FT_ADDPLACE_PORT, &m_aFTPort );
    pFtpDetails->addControl( ED_ADDPLACE_PORT, &m_aEDPort );
    pFtpDetails->addControl( FT_ADDPLACE_PATH, &m_aFTPath );
    pFtpDetails->addControl( ED_ADDPLACE_PATH, &m_aEDPath );
    pFtpDetails->setChangeHdl( LINK( this, PlaceEditDialog, EditHdl ) );
    m_aDetailsContainers.push_back( pFtpDetails );

    shared_ptr< DetailsContainer > pSshDetails( new HostDetailsContainer( 22, "ssh" ) );
    pSshDetails->addControl( FT_ADDPLACE_HOST, &m_aFTHost );
    pSshDetails->addControl( ED_ADDPLACE_HOST, &m_aEDHost );
    pSshDetails->addControl( FT_ADDPLACE_PORT, &m_aFTPort );
    pSshDetails->addControl( ED_ADDPLACE_PORT, &m_aEDPort );
    pSshDetails->addControl( FT_ADDPLACE_PATH, &m_aFTPath );
    pSshDetails->addControl( ED_ADDPLACE_PATH, &m_aEDPath );
    pSshDetails->setChangeHdl( LINK( this, PlaceEditDialog, EditHdl ) );
    m_aDetailsContainers.push_back( pSshDetails );

    // Create Windows Share control
    shared_ptr< DetailsContainer > pSmbDetails( new SmbDetailsContainer( ) );
    pSmbDetails->addControl( FT_ADDPLACE_HOST, &m_aFTHost );
    pSmbDetails->addControl( ED_ADDPLACE_SMBHOST, &m_aEDSmbHost );
    pSmbDetails->addControl( FT_ADDPLACE_SHARE, &m_aFTShare );
    pSmbDetails->addControl( ED_ADDPLACE_SHARE, &m_aEDShare );
    pSmbDetails->addControl( FT_ADDPLACE_SMBPATH, &m_aFTSmbPath );
    pSmbDetails->addControl( ED_ADDPLACE_SMBPATH, &m_aEDSmbPath );
    pSmbDetails->setChangeHdl( LINK( this, PlaceEditDialog, EditHdl ) );
    m_aDetailsContainers.push_back( pSmbDetails );

    // Create CMIS control
    shared_ptr< DetailsContainer > pCmisDetails( new CmisDetailsContainer( ) );
    pCmisDetails->addControl( FT_ADDPLACE_CMIS_BINDING, &m_aFTCmisBinding );
    pCmisDetails->addControl( ED_ADDPLACE_CMIS_BINDING, &m_aEDCmisBinding );
    pCmisDetails->addControl( FT_ADDPLACE_CMIS_REPOSITORY, &m_aFTCmisRepository );
    pCmisDetails->addControl( LB_ADDPLACE_CMIS_REPOSITORY, &m_aLBCmisRepository );
    pCmisDetails->addControl( BT_ADDPLACE_CMIS_REPOREFRESH, &m_aBTCmisRepoRefresh );
    pCmisDetails->setChangeHdl( LINK( this, PlaceEditDialog, EditHdl ) );

    if ( officecfg::Office::Common::Misc::ExperimentalMode::get() )
        m_aDetailsContainers.push_back( pCmisDetails );
    else
    {
        // Remove the CMIS entry, left it in src file for l10n
        m_aLBServerType.RemoveEntry( m_aLBServerType.GetEntryCount( ) - 1 );
    }

    // Set default to first value
    m_aLBServerType.SelectEntryPos( 0 );
    SelectTypeHdl( &m_aLBServerType );
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
    rtl::OUString sUrl = GetServerUrl( );
    rtl::OUString sName = rtl::OUString( m_aEDServerName.GetText() ).trim( );
    m_aBTOk.Enable( !sName.isEmpty( ) && !sUrl.isEmpty( ) );
    return 1;
}

IMPL_LINK ( PlaceEditDialog, EditUsernameHdl, void *, EMPTYARG )
{
    for ( std::vector< boost::shared_ptr< DetailsContainer > >::iterator it = m_aDetailsContainers.begin( );
            it != m_aDetailsContainers.end( ); ++it )
    {
        ( *it )->setUsername( rtl::OUString( m_aEDUsername.GetText() ) );
    }
    return 1;
}

IMPL_LINK( PlaceEditDialog, SelectTypeHdl, void*, EMPTYARG )
{
    // Compute the vertical space between two rows
    long nRowDelta = m_aLBServerType.GetPosPixel().getY() - m_aEDServerName.GetPosPixel().getY();
    long nRowSpace = nRowDelta - m_aEDServerName.GetSizePixel().getHeight();

    long nOldHeight = 0;
    if ( m_pCurrentDetails.get( ) )
    {
        m_pCurrentDetails->show( false );
        Rectangle aOldBounds = m_pCurrentDetails->getBounds( );
        if ( !aOldBounds.IsEmpty() )
            nOldHeight = aOldBounds.getHeight();
    }

    sal_uInt16 nPos = m_aLBServerType.GetSelectEntryPos( );
    m_pCurrentDetails = m_aDetailsContainers[nPos];

    m_pCurrentDetails->show( true );
    Rectangle aNewBounds = m_pCurrentDetails->getBounds();

    long nNewHeight = 0;
    if ( !aNewBounds.IsEmpty() )
    {
        nNewHeight = aNewBounds.getHeight();

        // Add row space if old height was 0
        if ( nOldHeight == 0 )
            nNewHeight += nRowSpace;
    }

    // If the new height is 0, but not the old one, then remove the doubled row space
    if ( nNewHeight == 0 && nOldHeight > 0 )
        nNewHeight -= nRowSpace;

    long nHeightDelta = nNewHeight - nOldHeight;
    Control* pToMove[] =
    {
        &m_aFTUsername,
        &m_aEDUsername,
        &m_aBTOk,
        &m_aBTCancel,
        &m_aBTDelete
    };

    Control** pCurrent = pToMove;
    for ( sal_Int32 i = 0; i < sal_Int32(SAL_N_ELEMENTS( pToMove )); ++i, ++pCurrent )
    {
        Point aPos = ( *pCurrent )->GetPosPixel( );
        aPos.setY( aPos.getY( ) + nHeightDelta );
        ( *pCurrent )->SetPosPixel( aPos );
    }

    // Resize the dialog too
    Size aDlgSize = GetSizePixel( );
    aDlgSize.setHeight( aDlgSize.getHeight( ) + nHeightDelta );
    SetSizePixel( aDlgSize );


    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
