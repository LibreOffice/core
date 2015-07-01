/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>

#include <comphelper/processfactory.hxx>
#include <officecfg/Office/Common.hxx>
#include <rtl/uri.hxx>
#include <ucbhelper/content.hxx>
#include <ucbhelper/commandenvironment.hxx>

#include <svtools/PlaceEditDialog.hxx>
#include <svtools/ServerDetailsControls.hxx>
#include <config_oauth2.h>

using namespace std;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::task;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;

DetailsContainer::DetailsContainer( VclBuilderContainer* pBuilder, const OString& rFrame )
{
    pBuilder->get( m_pFrame, rFrame );
}

DetailsContainer::~DetailsContainer( )
{
}

void DetailsContainer::show( bool bShow )
{
    m_pFrame->Show( bShow );
}

INetURLObject DetailsContainer::getUrl( )
{
    // Don't use that class directly: make it smarter by subclassing it.
    return INetURLObject( );
}

bool DetailsContainer::setUrl( const INetURLObject& )
{
    // That class doesn't contain any logic... it defers the dirty work
    // to the sub classes.
    return false;
}

void DetailsContainer::notifyChange( )
{
    m_aChangeHdl.Call( this );
}

IMPL_LINK_NOARG( DetailsContainer, ValueChangeHdl )
{
    notifyChange( );
    return 0;
}

HostDetailsContainer::HostDetailsContainer( VclBuilderContainer* pBuilder, sal_uInt16 nPort, const OUString& sScheme ) :
    DetailsContainer( pBuilder, "HostDetails" ),
    m_nDefaultPort( nPort ),
    m_sScheme( sScheme )
{
    pBuilder->get( m_pEDHost, "host" );
    m_pEDHost->SetModifyHdl( LINK( this, DetailsContainer, ValueChangeHdl ) );

    pBuilder->get( m_pEDPort, "port" );
    m_pEDPort->SetModifyHdl( LINK( this, DetailsContainer, ValueChangeHdl ) );

    pBuilder->get( m_pEDPath, "path" );
    m_pEDPath->SetModifyHdl( LINK( this, DetailsContainer, ValueChangeHdl ) );

    show( false );
}

void HostDetailsContainer::show( bool bShow )
{
    DetailsContainer::show( bShow );
    if ( bShow )
        m_pEDPort->SetValue( m_nDefaultPort );
}

INetURLObject HostDetailsContainer::getUrl( )
{
    OUString sHost = m_pEDHost->GetText().trim( );
    sal_Int64 nPort = m_pEDPort->GetValue();
    OUString sPath = m_pEDPath->GetText().trim( );

    OUString sUrl;
    if ( !sHost.isEmpty( ) )
    {
        sUrl = m_sScheme + "://" + sHost;
        if ( nPort != m_nDefaultPort )
            sUrl += ":" + OUString::number( nPort );
        if ( !sPath.isEmpty( ) )
            if ( sPath.indexOf( '/' ) != 0 )
                sUrl += "/";
        sUrl += sPath;
    }

    return INetURLObject( sUrl );
}

bool HostDetailsContainer::setUrl( const INetURLObject& rUrl )
{
    bool bSuccess = verifyScheme( INetURLObject::GetScheme( rUrl.GetProtocol( ) ) );

    if ( bSuccess )
    {
        m_pEDHost->SetText( rUrl.GetHost( ) );
        m_pEDPort->SetValue( rUrl.GetPort( ) );
        m_pEDPath->SetText( rUrl.GetURLPath() );
    }

    return bSuccess;
}

bool HostDetailsContainer::verifyScheme( const OUString& sScheme )
{
    return sScheme.equals( m_sScheme + "://" );
}

DavDetailsContainer::DavDetailsContainer( VclBuilderContainer* pBuilder ) :
    HostDetailsContainer( pBuilder, 80, "http" )
{
    pBuilder->get( m_pCBDavs, "webdavs" );
    m_pCBDavs->SetToggleHdl( LINK( this, DavDetailsContainer, ToggledDavsHdl ) );

    show( false );
}

void DavDetailsContainer::show( bool bShow )
{
    HostDetailsContainer::show( bShow );

    m_pCBDavs->Show( bShow );

    if ( bShow )
        m_pCBDavs->Check( false );
}

bool DavDetailsContainer::verifyScheme( const OUString& rScheme )
{
    bool bValid = false;
    if ( rScheme == "http://" )
    {
        bValid = true;
        m_pCBDavs->Check( false );
    }
    else if ( rScheme == "https://" )
    {
        bValid = true;
        m_pCBDavs->Check( true );
    }
    return bValid;
}

IMPL_LINK( DavDetailsContainer, ToggledDavsHdl, CheckBox*, pCheckBox )
{
    // Change default port if needed
    bool bCheckedDavs = pCheckBox->IsChecked();
    if ( m_pEDPort->GetValue() == 80 && bCheckedDavs )
        m_pEDPort->SetValue( 443 );
    else if ( m_pEDPort->GetValue() == 443 && !bCheckedDavs )
        m_pEDPort->SetValue( 80 );

    OUString sScheme( "http" );
    if ( bCheckedDavs )
        sScheme = "https";
    setScheme( sScheme );

    notifyChange( );

    return 0;
}

SmbDetailsContainer::SmbDetailsContainer( VclBuilderContainer* pBuilder ) :
    DetailsContainer( pBuilder, "SmbDetails" )
{
    pBuilder->get( m_pEDHost, "smbHost" );
    m_pEDHost->SetModifyHdl( LINK( this, DetailsContainer, ValueChangeHdl ) );

    pBuilder->get( m_pEDShare, "smbShare" );
    m_pEDShare->SetModifyHdl( LINK( this, DetailsContainer, ValueChangeHdl ) );

    pBuilder->get( m_pEDPath, "smbPath" );
    m_pEDPath->SetModifyHdl( LINK( this, DetailsContainer, ValueChangeHdl ) );

    show( false );
}

INetURLObject SmbDetailsContainer::getUrl( )
{
    OUString sHost = m_pEDHost->GetText().trim( );
    OUString sShare = m_pEDShare->GetText().trim( );
    OUString sPath = m_pEDPath->GetText().trim( );

    OUString sUrl;
    if ( !sHost.isEmpty( ) )
    {
        sUrl = "smb://" + sHost + "/";
        if ( !sShare.isEmpty( ) )
            sUrl += sShare;
        if ( !sPath.isEmpty( ) )
            if ( sPath.indexOf( '/' ) != 0 )
                sUrl += "/";
        sUrl += sPath;
    }

    return INetURLObject( sUrl );
}

bool SmbDetailsContainer::setUrl( const INetURLObject& rUrl )
{
    bool bSuccess =  rUrl.GetProtocol() == INetProtocol::Smb;

    if ( bSuccess )
    {
        OUString sShare = rUrl.getName( 0 );
        OUString sFullPath = rUrl.GetURLPath( );
        OUString sPath;
        if ( sFullPath.getLength( ) > sShare.getLength( ) )
        {
            sal_Int32 nPos = sShare.getLength( );
            if ( nPos != 0 )
                ++nPos;
            sPath = sFullPath.copy( nPos );
        }

        m_pEDHost->SetText( rUrl.GetHost( ) );
        m_pEDShare->SetText( sShare );
        m_pEDPath->SetText( sPath );
    }

    return bSuccess;
}

CmisDetailsContainer::CmisDetailsContainer( VclBuilderContainer* pBuilder, OUString sBinding ) :
    DetailsContainer( pBuilder, "CmisDetails" ),
    m_sUsername( ),
    m_xCmdEnv( ),
    m_aRepoIds( ),
    m_sRepoId( ),
    m_sBinding( sBinding )
{
    Reference< XComponentContext > xContext = ::comphelper::getProcessComponentContext();
    Reference< XInteractionHandler > xGlobalInteractionHandler(
        InteractionHandler::createWithParent(xContext, 0), UNO_QUERY );
    m_xCmdEnv = new ucbhelper::CommandEnvironment( xGlobalInteractionHandler, Reference< XProgressHandler >() );

    pBuilder->get( m_pEDBinding, "binding" );
    pBuilder->get( m_pLBRepository, "repositories" );
    pBuilder->get( m_pBTRepoRefresh, "repositoriesRefresh" );

    pBuilder->get( m_pEDRoot, "cmisPath" );

    show( false );
}

void CmisDetailsContainer::show( bool bShow )
{
    m_pEDBinding->SetModifyHdl( LINK( this, DetailsContainer, ValueChangeHdl ) );
    m_pLBRepository->SetSelectHdl( LINK( this, CmisDetailsContainer, SelectRepoHdl ) );
    m_pBTRepoRefresh->SetClickHdl( LINK( this, CmisDetailsContainer, RefreshReposHdl ) );
    m_pEDRoot->SetModifyHdl( LINK( this, DetailsContainer, ValueChangeHdl ) );

    m_pEDBinding->SetText( m_sBinding );

    if( ( m_sBinding == GDRIVE_BASE_URL )
            || m_sBinding.startsWith( ALFRESCO_CLOUD_BASE_URL )
            || ( m_sBinding == ONEDRIVE_BASE_URL ) )
    {
        DetailsContainer::show( false );
    }
    else
    {
        DetailsContainer::show( bShow );
    }
}

INetURLObject CmisDetailsContainer::getUrl( )
{
    OUString sBindingUrl = m_pEDBinding->GetText().trim( );
    OUString sPath = m_pEDRoot->GetText().trim( );

    bool bSkip = true;
    if( ( m_sBinding == GDRIVE_BASE_URL )
            || m_sBinding.startsWith( ALFRESCO_CLOUD_BASE_URL )
            || ( m_sBinding == ONEDRIVE_BASE_URL ) )
    {
        bSkip = m_sUsername.isEmpty();
    }
    else
    {
        bSkip = m_sRepoId.isEmpty();
    }

    OUString sUrl;
    if ( !sBindingUrl.isEmpty( ) && !bSkip )
    {
        OUString sEncodedBinding = rtl::Uri::encode(
                sBindingUrl + "#" + m_sRepoId,
                rtl_UriCharClassRelSegment,
                rtl_UriEncodeKeepEscapes,
                RTL_TEXTENCODING_UTF8 );
        sUrl = "vnd.libreoffice.cmis://" + sEncodedBinding;
    }
    sUrl += sPath;

    return INetURLObject( sUrl );
}

bool CmisDetailsContainer::setUrl( const INetURLObject& rUrl )
{
    bool bSuccess =  rUrl.GetProtocol() == INetProtocol::Cmis;

    if ( bSuccess )
    {
        OUString sRepositoryId;

        OUString sDecodedHost = rUrl.GetHost( INetURLObject::DECODE_WITH_CHARSET );
        INetURLObject aHostUrl( sDecodedHost );
        m_sBinding = aHostUrl.GetURLNoMark( );
        sRepositoryId = aHostUrl.GetMark( );

        m_pEDBinding->SetText( m_sBinding );
        m_pEDRoot->SetText( rUrl.GetURLPath() );
    }
    return bSuccess;
}

void CmisDetailsContainer::setUsername( const OUString& rUsername )
{
    m_sUsername = rUsername;
}

void CmisDetailsContainer::selectRepository( )
{
    // Get the repo ID and call the Change listener
    sal_uInt16 nPos = m_pLBRepository->GetSelectEntryPos( );
    if( nPos < m_aRepoIds.size() )
    {
        m_sRepoId = m_aRepoIds[nPos];
        notifyChange( );
    }
}

IMPL_LINK_NOARG( CmisDetailsContainer, RefreshReposHdl  )
{
    OUString sBindingUrl = m_pEDBinding->GetText().trim( );

    OUString sEncodedUsername = "";

    if ( !m_sUsername.isEmpty( ) )
    {
        sEncodedUsername = rtl::Uri::encode(m_sUsername,
                                            rtl_UriCharClassUserinfo,
                                            rtl_UriEncodeKeepEscapes,
                                            RTL_TEXTENCODING_UTF8 );
        sEncodedUsername += "@";
    }

    // Clean the listbox
    m_pLBRepository->Clear( );
    m_aRepoIds.clear( );

    // Compute the URL
    OUString sUrl;
    if ( !sBindingUrl.isEmpty( ) )
    {
        OUString sEncodedBinding = rtl::Uri::encode(
                sBindingUrl,
                rtl_UriCharClassRelSegment,
                rtl_UriEncodeKeepEscapes,
                RTL_TEXTENCODING_UTF8 );
        sUrl = "vnd.libreoffice.cmis://" + sEncodedUsername + sEncodedBinding;
    }

    // Get the Content
    ::ucbhelper::Content aCnt( sUrl, m_xCmdEnv, comphelper::getProcessComponentContext() );
    Sequence< OUString > aProps( 1 );
    aProps[0] = "Title";

    try
    {
        Reference< XResultSet > xResultSet( aCnt.createCursor( aProps ), UNO_QUERY_THROW );
        Reference< XContentAccess > xAccess( xResultSet, UNO_QUERY_THROW );
        while ( xResultSet->next() )
        {
            OUString sURL = xAccess->queryContentIdentifierString( );
            INetURLObject aURL( sURL );
            OUString sId = aURL.GetURLPath( INetURLObject::DECODE_WITH_CHARSET );
            sId = sId.copy( 1 );
            m_aRepoIds.push_back( sId );

            Reference< XRow > xRow( xResultSet, UNO_QUERY );
            OUString sName = xRow->getString( 1 );
            m_pLBRepository->InsertEntry( sName );
        }
    }
    catch ( const Exception& )
    {
        ScopedVclPtrInstance< ErrorBox > pErrorBox( static_cast< vcl::Window* >( NULL ),
                                                   WB_OK,
                                                   "Auth failed!" );
        pErrorBox->Execute( );
    }

    // Auto-select the first one
    if ( m_pLBRepository->GetEntryCount( ) > 0 )
    {
        m_pLBRepository->SelectEntryPos( 0 );
        selectRepository( );
    }

    return 0;
}

IMPL_LINK_NOARG( CmisDetailsContainer, SelectRepoHdl  )
{
    selectRepository( );
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
