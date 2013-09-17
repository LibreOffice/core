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

IMPL_LINK( DetailsContainer, ValueChangeHdl, void *, EMPTYARG )
{
    notifyChange( );
    return 0;
}

HostDetailsContainer::HostDetailsContainer( VclBuilderContainer* pBuilder, sal_uInt16 nPort, OUString sScheme ) :
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
    OUString sHost = OUString( m_pEDHost->GetText() ).trim( );
    sal_Int64 nPort = m_pEDPort->GetValue();
    OUString sPath = OUString( m_pEDPath->GetText() ).trim( );

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
    if ( rScheme.equals( "http://" ) )
    {
        bValid = true;
        m_pCBDavs->Check( false );
    }
    else if ( rScheme.equals( "https://" ) )
    {
        bValid = true;
        m_pCBDavs->Check( true );
    }
    return bValid;
}

IMPL_LINK( DavDetailsContainer, ToggledDavsHdl, CheckBox*, pCheckBox )
{
    // Change default port if needed
    sal_Bool bCheckedDavs = pCheckBox->IsChecked();
    if ( m_pEDPort->GetValue() == 80 && bCheckedDavs == sal_True)
        m_pEDPort->SetValue( 443 );
    else if ( m_pEDPort->GetValue() == 443 && bCheckedDavs == sal_False )
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
    OUString sHost = OUString( m_pEDHost->GetText() ).trim( );
    OUString sShare = OUString( m_pEDShare->GetText() ).trim( );
    OUString sPath = OUString(  m_pEDPath->GetText() ).trim( );

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
    bool bSuccess =  rUrl.GetProtocol() == INET_PROT_SMB;

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

CmisDetailsContainer::CmisDetailsContainer( VclBuilderContainer* pBuilder ) :
    DetailsContainer( pBuilder, "CmisDetails" ),
    m_sUsername( ),
    m_xCmdEnv( ),
    m_aServerTypesURLs( ),
    m_aRepoIds( ),
    m_sRepoId( )
{
    Reference< XComponentContext > xContext = ::comphelper::getProcessComponentContext();
    Reference< XInteractionHandler > xGlobalInteractionHandler(
        InteractionHandler::createWithParent(xContext, 0), UNO_QUERY );
    m_xCmdEnv = new ucbhelper::CommandEnvironment( xGlobalInteractionHandler, Reference< XProgressHandler >() );

    pBuilder->get( m_pLBServerType, "serverType" );
    m_pLBServerType->SetSelectHdl( LINK( this, CmisDetailsContainer, SelectServerTypeHdl ) );

    pBuilder->get( m_pEDBinding, "binding" );
    m_pEDBinding->SetModifyHdl( LINK( this, DetailsContainer, ValueChangeHdl ) );

    pBuilder->get( m_pLBRepository, "repositories" );
    m_pLBRepository->SetSelectHdl( LINK( this, CmisDetailsContainer, SelectRepoHdl ) );

    pBuilder->get( m_pBTRepoRefresh, "repositoriesRefresh" );
    m_pBTRepoRefresh->SetClickHdl( LINK( this, CmisDetailsContainer, RefreshReposHdl ) );

    pBuilder->get( m_pEDPath, "cmisPath" );
    m_pEDPath->SetModifyHdl( LINK( this, DetailsContainer, ValueChangeHdl ) );

    show( false );

    // Load the ServerType entries
    Sequence< OUString > aTypesUrlsList( officecfg::Office::Common::Misc::CmisServersUrls::get( xContext ) );
    Sequence< OUString > aTypesNamesList( officecfg::Office::Common::Misc::CmisServersNames::get( xContext ) );
    for ( sal_Int32 i = 0; i < aTypesUrlsList.getLength( ) && aTypesNamesList.getLength( ); ++i )
    {
        m_pLBServerType->InsertEntry( aTypesNamesList[i] );
        m_aServerTypesURLs.push_back( aTypesUrlsList[i] );
    }
}

INetURLObject CmisDetailsContainer::getUrl( )
{
    OUString sBindingUrl = OUString( m_pEDBinding->GetText() ).trim( );
    OUString sPath = OUString( m_pEDPath->GetText() ).trim( );

    OUString sUrl;
    if ( !sBindingUrl.isEmpty( ) && !m_sRepoId.isEmpty() )
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
    bool bSuccess =  rUrl.GetProtocol() == INET_PROT_CMIS;

    if ( bSuccess )
    {
        OUString sBindingUrl;
        OUString sRepositoryId;

        OUString sDecodedHost = rUrl.GetHost( INetURLObject::DECODE_WITH_CHARSET );
        INetURLObject aHostUrl( sDecodedHost );
        sBindingUrl = aHostUrl.GetURLNoMark( );
        sRepositoryId = aHostUrl.GetMark( );

        m_pEDBinding->SetText( sBindingUrl );
        m_pEDPath->SetText( rUrl.GetURLPath() );
    }
    return bSuccess;
}

void CmisDetailsContainer::setUsername( const OUString& rUsername )
{
    m_sUsername = OUString( rUsername );
}

void CmisDetailsContainer::selectRepository( )
{
    // Get the repo ID and call the Change listener
    sal_uInt16 nPos = m_pLBRepository->GetSelectEntryPos( );
    m_sRepoId = m_aRepoIds[nPos];

    notifyChange( );
}

IMPL_LINK( CmisDetailsContainer, SelectServerTypeHdl, void *, EMPTYARG  )
{
    // Set a sample URL for the server
    sal_uInt16 nId = m_pLBServerType->GetSelectEntryPos( );
    m_pEDBinding->SetText( m_aServerTypesURLs[nId] );
    return 0;
}

IMPL_LINK( CmisDetailsContainer, RefreshReposHdl, void *, EMPTYARG  )
{
    OUString sBindingUrl = OUString( m_pEDBinding->GetText() ).trim( );

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
        sUrl = "vnd.libreoffice.cmis://" + sEncodedBinding;
    }

    // Get the Content
    ::ucbhelper::Content aCnt( sUrl, m_xCmdEnv, comphelper::getProcessComponentContext() );
    Sequence< OUString > aProps( 1 );
    aProps[0] = OUString( "Title" );

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
    }

    // Auto-select the first one
    if ( m_pLBRepository->GetEntryCount( ) > 0 )
    {
        m_pLBRepository->SelectEntryPos( 0 );
        selectRepository( );
    }

    return 0;
}

IMPL_LINK( CmisDetailsContainer, SelectRepoHdl, void *, EMPTYARG  )
{
    selectRepository( );
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
