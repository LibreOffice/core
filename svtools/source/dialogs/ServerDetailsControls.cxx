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

DetailsContainer::DetailsContainer( VclBuilderContainer* pBuilder ) :
    m_bIsActive ( true )
{
    pBuilder->get( m_pDetailsGrid, "Details" );
    pBuilder->get( m_pHostBox, "HostDetails" );
    pBuilder->get( m_pEDHost, "host" );
    pBuilder->get( m_pFTHost, "hostLabel" );
    pBuilder->get( m_pEDPort, "port-nospin" );
    pBuilder->get( m_pFTPort, "portLabel" );
    pBuilder->get( m_pEDRoot, "path" );
    pBuilder->get( m_pFTRoot, "pathLabel" );
}

DetailsContainer::~DetailsContainer( )
{
}

void DetailsContainer::show( bool )
{
    m_pDetailsGrid->Enable( m_bIsActive );

    m_pEDHost->SetModifyHdl( LINK( this, DetailsContainer, ValueChangeHdl ) );
    m_pEDPort->SetModifyHdl( LINK( this, DetailsContainer, ValueChangeHdl ) );
    m_pEDRoot->SetModifyHdl( LINK( this, DetailsContainer, ValueChangeHdl ) );
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

void DetailsContainer::setActive( bool bActive )
{
    m_bIsActive = bActive;
}

IMPL_LINK_NOARG( DetailsContainer, ValueChangeHdl )
{
    notifyChange( );
    return 0;
}

HostDetailsContainer::HostDetailsContainer( VclBuilderContainer* pBuilder, sal_uInt16 nPort, const OUString& sScheme ) :
    DetailsContainer( pBuilder ),
    m_nDefaultPort( nPort ),
    m_sScheme( sScheme )
{
    show( false );
}

void HostDetailsContainer::show( bool bShow )
{
    m_pFTHost->Show( bShow );
    m_pHostBox->Show( bShow );
    m_pEDRoot->Show( bShow );
    m_pFTRoot->Show( bShow );

    DetailsContainer::show( bShow );

    if ( bShow )
    {
        m_pEDPort->SetValue( m_nDefaultPort );
        m_pEDHost->SetText( m_sHost );
    }
}

INetURLObject HostDetailsContainer::getUrl( )
{
    OUString sHost = m_pEDHost->GetText().trim( );
    sal_Int64 nPort = m_pEDPort->GetValue();
    OUString sPath = m_pEDRoot->GetText().trim( );

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
        m_pEDRoot->SetText( rUrl.GetURLPath() );
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
        m_pCBDavs->Check();
    }
    return bValid;
}

IMPL_LINK_TYPED( DavDetailsContainer, ToggledDavsHdl, CheckBox&, rCheckBox, void )
{
    // Change default port if needed
    bool bCheckedDavs = rCheckBox.IsChecked();
    if ( m_pEDPort->GetValue() == 80 && bCheckedDavs )
        m_pEDPort->SetValue( 443 );
    else if ( m_pEDPort->GetValue() == 443 && !bCheckedDavs )
        m_pEDPort->SetValue( 80 );

    OUString sScheme( "http" );
    if ( bCheckedDavs )
        sScheme = "https";
    setScheme( sScheme );

    notifyChange( );
}

SmbDetailsContainer::SmbDetailsContainer( VclBuilderContainer* pBuilder ) :
    DetailsContainer( pBuilder )
{
    pBuilder->get( m_pEDShare, "share" );
    pBuilder->get( m_pFTShare, "shareLabel" );

    m_pEDShare->SetModifyHdl( LINK( this, DetailsContainer, ValueChangeHdl ) );

    show( false );
}

INetURLObject SmbDetailsContainer::getUrl( )
{
    OUString sHost = m_pEDHost->GetText().trim( );
    OUString sShare = m_pEDShare->GetText().trim( );
    OUString sPath = m_pEDRoot->GetText().trim( );

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
        m_pEDRoot->SetText( sPath );
    }

    return bSuccess;
}

void SmbDetailsContainer::show( bool bShow )
{
    m_pEDShare->Show( bShow );
    m_pFTShare->Show( bShow );
    m_pEDRoot->Show( bShow );
    m_pFTRoot->Show( bShow );

    m_pFTHost->Show( bShow );
    m_pHostBox->Show( bShow );
    m_pEDPort->Enable( !bShow );
    m_pFTPort->Enable( !bShow );
}

CmisDetailsContainer::CmisDetailsContainer( VclBuilderContainer* pBuilder, OUString const & sBinding ) :
    DetailsContainer( pBuilder ),
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

    pBuilder->get( m_pFTRepository, "repositoryLabel" );
    pBuilder->get( m_pLBRepository, "repositories" );
    pBuilder->get( m_pBTRepoRefresh, "repositoriesRefresh" );
    pBuilder->get( m_pRepositoryBox, "RepositoryDetails" );

    show( false );
}

void CmisDetailsContainer::show( bool bShow )
{
    m_pLBRepository->SetSelectHdl( LINK( this, CmisDetailsContainer, SelectRepoHdl ) );
    m_pBTRepoRefresh->SetClickHdl( LINK( this, CmisDetailsContainer, RefreshReposHdl ) );

    m_pEDHost->SetText( m_sBinding );

    if( ( m_sBinding == GDRIVE_BASE_URL )
            || m_sBinding.startsWith( ALFRESCO_CLOUD_BASE_URL )
            || ( m_sBinding == ONEDRIVE_BASE_URL ) )
    {
        m_pFTHost->Show( false );
        m_pHostBox->Show( false );
        m_pFTRepository->Show( false );
        m_pRepositoryBox->Show( false );
        m_pEDRoot->Show( false );
        m_pFTRoot->Show( false );
    }
    else
    {
        m_pFTHost->Show( bShow );
        m_pHostBox->Show( bShow );
        m_pFTRepository->Show( bShow );
        m_pRepositoryBox->Show( bShow );
        m_pEDRoot->Show( bShow );
        m_pFTRoot->Show( bShow );
    }

    DetailsContainer::show( bShow );
    m_pEDPort->Enable( !bShow );
    m_pFTPort->Enable( !bShow );
}

INetURLObject CmisDetailsContainer::getUrl( )
{
    OUString sBindingUrl = m_pEDHost->GetText().trim( );
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
        OUString sDecodedHost = rUrl.GetHost( INetURLObject::DECODE_WITH_CHARSET );
        INetURLObject aHostUrl( sDecodedHost );
        m_sBinding = aHostUrl.GetURLNoMark( );
        m_sRepoId = aHostUrl.GetMark( );

        m_pEDHost->SetText( m_sBinding );
        m_pEDRoot->SetText( rUrl.GetURLPath() );
    }
    return bSuccess;
}

void CmisDetailsContainer::setUsername( const OUString& rUsername )
{
    m_sUsername = rUsername;
}

void CmisDetailsContainer::setPassword( const OUString& rPass )
{
    m_sPassword = rPass;
}

void CmisDetailsContainer::selectRepository( )
{
    // Get the repo ID and call the Change listener
    const sal_Int32 nPos = m_pLBRepository->GetSelectEntryPos( );
    if( static_cast<size_t>(nPos) < m_aRepoIds.size() )
    {
        m_sRepoId = m_aRepoIds[nPos];
        notifyChange( );
    }
}

IMPL_LINK_NOARG_TYPED( CmisDetailsContainer, RefreshReposHdl, Button*, void  )
{
    Reference< XComponentContext > xContext = ::comphelper::getProcessComponentContext();
    Reference< XPasswordContainer2 > xMasterPasswd = PasswordContainer::create( xContext );


    OUString sBindingUrl = m_pEDHost->GetText().trim( );

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

    // temporary remember the password
    try
    {
        if( !sUrl.isEmpty() && !m_sUsername.isEmpty() && !m_sPassword.isEmpty() )
        {
            Reference< XInteractionHandler > xInteractionHandler(
                InteractionHandler::createWithParent( xContext, 0 ),
                UNO_QUERY );

            Sequence< OUString > aPasswd( 1 );
            aPasswd[0] = m_sPassword;

            xMasterPasswd->add(
                sUrl, m_sUsername, aPasswd, xInteractionHandler );
        }
    }
    catch( const Exception& )
    {}

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
    }

    // Auto-select the first one
    if ( m_pLBRepository->GetEntryCount( ) > 0 )
    {
        m_pLBRepository->SelectEntryPos( 0 );
        selectRepository( );
    }

    // remove temporary password
    try
    {
        xMasterPasswd->remove( sUrl, m_sUsername );
    }
    catch( const Exception& )
    {}
}

IMPL_LINK_NOARG( CmisDetailsContainer, SelectRepoHdl  )
{
    selectRepository( );
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
