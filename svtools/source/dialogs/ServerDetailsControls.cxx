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
 * [ Copyright (C) 2012 SUSE <cbosdonnat@suse.com> (initial developer) ]
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

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>

#include <comphelper/processfactory.hxx>
#include <rtl/uri.hxx>
#include <ucbhelper/content.hxx>
#include <ucbhelper/commandenvironment.hxx>

#include "PlaceEditDialog.hrc"

#include "PlaceEditDialog.hxx"
#include "ServerDetailsControls.hxx"

using namespace std;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::task;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;

DetailsContainer::DetailsContainer( ) :
    m_aControls( ),
    m_bShown( false )
{
}

DetailsContainer::~DetailsContainer( )
{
    m_aControls.clear( );
}

void DetailsContainer::addControl( sal_uInt16 nId, Control* pControl )
{
    m_aControls.insert( pair< sal_uInt16, Control* >( nId, pControl ) );
    pControl->Show( m_bShown );

    if ( pControl->GetType( ) == WINDOW_EDIT )
        static_cast< Edit* >( pControl )->SetModifyHdl( LINK( this, DetailsContainer, ValueChangeHdl ) );
    else if ( pControl->GetType( ) == WINDOW_NUMERICFIELD )
        static_cast< NumericField* >( pControl )->SetModifyHdl( LINK( this, DetailsContainer, ValueChangeHdl ) );
    else if ( pControl->GetType( ) == WINDOW_CHECKBOX )
        static_cast< CheckBox* >( pControl )->SetToggleHdl( LINK( this, DetailsContainer, ValueChangeHdl ) );
}

Control* DetailsContainer::getControl( sal_uInt16 nId )
{
    Control* pControl = NULL;
    map< sal_uInt16, Control* >::iterator it = m_aControls.find( nId );
    if ( it != m_aControls.end( ) )
        pControl = it->second;
    return pControl;
}

Rectangle DetailsContainer::getBounds( )
{
    Rectangle aBounds;
    for ( map< sal_uInt16, Control* >::iterator it = m_aControls.begin( ); it != m_aControls.end( ); ++it )
    {
        Rectangle aControlBounds( it->second->GetPosPixel(), it->second->GetSizePixel() );
        aBounds = aBounds.GetUnion( aControlBounds );
    }

    return aBounds;
}

void DetailsContainer::show( bool bShow )
{
    m_bShown = bShow;
    for ( map< sal_uInt16, Control* >::iterator it = m_aControls.begin( ); it != m_aControls.end( ); ++it )
    {
        it->second->Show( m_bShown );
    }
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

HostDetailsContainer::HostDetailsContainer( sal_uInt16 nPort, rtl::OUString sScheme ) :
    DetailsContainer( ),
    m_nDefaultPort( nPort ),
    m_sScheme( sScheme )
{
}

void HostDetailsContainer::show( bool bShow )
{
    DetailsContainer::show( bShow );
    if ( bShow )
        static_cast< NumericField* >( getControl( ED_ADDPLACE_PORT ) )->SetValue( m_nDefaultPort );
}

INetURLObject HostDetailsContainer::getUrl( )
{
    rtl::OUString sHost = rtl::OUString( static_cast< Edit* >( getControl( ED_ADDPLACE_HOST ) )->GetText() ).trim( );
    sal_Int64 nPort = static_cast< NumericField* >( getControl( ED_ADDPLACE_PORT ) )->GetValue();
    rtl::OUString sPath = rtl::OUString( static_cast< Edit* >( getControl( ED_ADDPLACE_PATH ) )->GetText() ).trim( );

    rtl::OUString sUrl;
    if ( !sHost.isEmpty( ) )
    {
        sUrl = m_sScheme + "://" + sHost;
        if ( nPort != m_nDefaultPort )
            sUrl += ":" + rtl::OUString::valueOf( nPort );
        if ( !sPath.isEmpty( ) )
            if ( sPath.indexOf( '/' ) != 0 )
                sUrl += "/";
            sUrl += sPath;
    }

    return INetURLObject( sUrl );
}

bool HostDetailsContainer::setUrl( const INetURLObject& rUrl )
{
    bool bSuccess = false;

    bSuccess = verifyScheme( INetURLObject::GetScheme( rUrl.GetProtocol( ) ) );

    if ( bSuccess )
    {
        static_cast< Edit* >( getControl( ED_ADDPLACE_HOST ) )->SetText( rUrl.GetHost( ) );
        static_cast< NumericField* >( getControl( ED_ADDPLACE_PORT ) )->SetValue( rUrl.GetPort( ) );
        static_cast< Edit* >( getControl( ED_ADDPLACE_PATH ) )->SetText( rUrl.GetURLPath() );
    }

    return bSuccess;
}

bool HostDetailsContainer::verifyScheme( const rtl::OUString& sScheme )
{
    return sScheme.equals( m_sScheme + "://" );
}

DavDetailsContainer::DavDetailsContainer( ) :
    HostDetailsContainer( 80, "http" )
{
}

void DavDetailsContainer::addControl( sal_uInt16 nId, Control* pControl )
{
    DetailsContainer::addControl( nId, pControl );

    // Add listener on CB_ADDPLACE_DAVS
    if ( nId == CB_ADDPLACE_DAVS )
        static_cast< CheckBox* >( pControl )->SetToggleHdl( LINK( this, DavDetailsContainer, ToggledDavsHdl ) );
}

void DavDetailsContainer::show( bool bShow )
{
    HostDetailsContainer::show( bShow );
    if ( bShow )
        static_cast< CheckBox* >( getControl( CB_ADDPLACE_DAVS ) )->Check( false );
}

bool DavDetailsContainer::verifyScheme( const rtl::OUString& rScheme )
{
    bool bValid = false;
    if ( rScheme.equals( "http://" ) )
    {
        bValid = true;
        static_cast< CheckBox* >( getControl( CB_ADDPLACE_DAVS ) )->Check( false );
    }
    else if ( rScheme.equals( "https://" ) )
    {
        bValid = true;
        static_cast< CheckBox* >( getControl( CB_ADDPLACE_DAVS ) )->Check( true );
    }
    return bValid;
}

IMPL_LINK( DavDetailsContainer, ToggledDavsHdl, CheckBox*, pCheckBox )
{
    // Change default port if needed
    sal_Bool bCheckedDavs = pCheckBox->IsChecked();
    NumericField* pPortField = static_cast< NumericField* >( getControl( ED_ADDPLACE_PORT ) );
    if ( pPortField->GetValue() == 80 && bCheckedDavs == sal_True)
        pPortField->SetValue( 443 );
    else if ( pPortField->GetValue() == 443 && bCheckedDavs == sal_False )
        pPortField->SetValue( 80 );

    rtl::OUString sScheme( "http" );
    if ( bCheckedDavs )
        sScheme = "https";
    setScheme( sScheme );

    notifyChange( );

    return 0;
}

INetURLObject SmbDetailsContainer::getUrl( )
{
    rtl::OUString sHost = rtl::OUString( static_cast< Edit* >( getControl( ED_ADDPLACE_SMBHOST ) )->GetText() ).trim( );
    rtl::OUString sShare = rtl::OUString( static_cast< Edit* >( getControl( ED_ADDPLACE_SHARE ) )->GetText() ).trim( );
    rtl::OUString sPath = rtl::OUString( static_cast< Edit* >( getControl( ED_ADDPLACE_SMBPATH ) )->GetText() ).trim( );

    rtl::OUString sUrl;
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
        rtl::OUString sShare = rUrl.getName( 0 );
        rtl::OUString sFullPath = rUrl.GetURLPath( );
        rtl::OUString sPath;
        if ( sFullPath.getLength( ) > sShare.getLength( ) )
        {
            sal_Int32 nPos = sShare.getLength( );
            if ( nPos != 0 )
                ++nPos;
            sPath = sFullPath.copy( nPos );
        }

        static_cast< Edit* >( getControl( ED_ADDPLACE_SMBHOST ) )->SetText( rUrl.GetHost( ) );
        static_cast< Edit* >( getControl( ED_ADDPLACE_SHARE ) )->SetText( sShare );
        static_cast< Edit* >( getControl( ED_ADDPLACE_SMBPATH ) )->SetText( sPath );
    }

    return bSuccess;
}

CmisDetailsContainer::CmisDetailsContainer( ) :
    DetailsContainer( ),
    m_sUsername( ),
    m_xCmdEnv( )
{
    Reference< XComponentContext > xContext = ::comphelper::getProcessComponentContext();
    Reference< XInteractionHandler > xGlobalInteractionHandler(
        InteractionHandler::createWithParent(xContext, 0), UNO_QUERY );
    m_xCmdEnv = new ucbhelper::CommandEnvironment( xGlobalInteractionHandler, Reference< XProgressHandler >() );
}

INetURLObject CmisDetailsContainer::getUrl( )
{
    rtl::OUString sBindingUrl = rtl::OUString( static_cast< Edit* >( getControl( ED_ADDPLACE_CMIS_BINDING ) )->GetText() ).trim( );

    rtl::OUString sUrl;
    if ( !sBindingUrl.isEmpty( ) && !m_sRepoId.isEmpty() )
    {
        rtl::OUString sEncodedBinding = rtl::Uri::encode(
                sBindingUrl + "#" + m_sRepoId,
                rtl_UriCharClassRelSegment,
                rtl_UriEncodeKeepEscapes,
                RTL_TEXTENCODING_UTF8 );
        sUrl = "vnd.libreoffice.cmis+atom://" + sEncodedBinding;
    }

    return INetURLObject( sUrl );
}

bool CmisDetailsContainer::setUrl( const INetURLObject& rUrl )
{
    bool bSuccess =  rUrl.GetProtocol() == INET_PROT_CMIS_ATOM;

    if ( bSuccess )
    {
        rtl::OUString sBindingUrl;
        rtl::OUString sRepositoryId;

        rtl::OUString sDecodedHost = rUrl.GetHost( INetURLObject::DECODE_WITH_CHARSET );
        INetURLObject aHostUrl( sDecodedHost );
        sBindingUrl = aHostUrl.GetURLNoMark( );
        sRepositoryId = aHostUrl.GetMark( );

        static_cast< Edit* >( getControl( ED_ADDPLACE_CMIS_BINDING ) )->SetText( sBindingUrl );
    }

    return bSuccess;
}

void CmisDetailsContainer::setUsername( const rtl::OUString& rUsername )
{
    m_sUsername = rtl::OUString( rUsername );
}

void CmisDetailsContainer::addControl( sal_uInt16 nId, Control* pControl )
{
    DetailsContainer::addControl( nId, pControl );

    // Add listener on BT_ADDPLACE_CMIS_REPOREFRESH
    if ( nId == BT_ADDPLACE_CMIS_REPOREFRESH )
        static_cast< ImageButton* >( pControl )->SetClickHdl( LINK( this, CmisDetailsContainer, RefreshReposHdl ) );
    if ( nId == LB_ADDPLACE_CMIS_REPOSITORY )
        static_cast< ListBox* >( pControl )->SetSelectHdl( LINK( this, CmisDetailsContainer, SelectRepoHdl ) );
}

void CmisDetailsContainer::selectRepository( )
{
    // Get the repo ID and call the Change listener
    ListBox* pReposList = static_cast< ListBox* >( getControl( LB_ADDPLACE_CMIS_REPOSITORY ) );
    sal_uInt16 nPos = pReposList->GetSelectEntryPos( );
    m_sRepoId = m_aRepoIds[nPos];

    notifyChange( );
}

IMPL_LINK( CmisDetailsContainer, RefreshReposHdl, void *, EMPTYARG  )
{
    rtl::OUString sBindingUrl = rtl::OUString( static_cast< Edit* >( getControl( ED_ADDPLACE_CMIS_BINDING ) )->GetText() ).trim( );

    // Clean the listbox
    ListBox* pReposList = static_cast< ListBox* >( getControl( LB_ADDPLACE_CMIS_REPOSITORY ) );
    pReposList->Clear( );
    m_aRepoIds.clear( );

    // Compute the URL
    rtl::OUString sUrl;
    if ( !sBindingUrl.isEmpty( ) )
    {
        rtl::OUString sEncodedBinding = rtl::Uri::encode(
                sBindingUrl,
                rtl_UriCharClassRelSegment,
                rtl_UriEncodeKeepEscapes,
                RTL_TEXTENCODING_UTF8 );
        sUrl = "vnd.libreoffice.cmis+atom://" + sEncodedBinding;
    }

    // Get the Content
    ::ucbhelper::Content aCnt( sUrl, m_xCmdEnv, comphelper::getProcessComponentContext() );
    Sequence< rtl::OUString > aProps( 1 );
    aProps[0] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Title" ) );

    try
    {
        Reference< XResultSet > xResultSet( aCnt.createCursor( aProps ), UNO_QUERY_THROW );
        Reference< XContentAccess > xAccess( xResultSet, UNO_QUERY_THROW );
        while ( xResultSet->next() )
        {
            rtl::OUString sURL = xAccess->queryContentIdentifierString( );
            INetURLObject aURL( sURL );
            rtl::OUString sId = aURL.GetURLPath( INetURLObject::DECODE_WITH_CHARSET );
            sId = sId.copy( 1 );
            m_aRepoIds.push_back( sId );

            Reference< XRow > xRow( xResultSet, UNO_QUERY );
            rtl::OUString sName = xRow->getString( 1 );
            pReposList->InsertEntry( sName );
        }
    }
    catch ( const Exception& )
    {
    }

    // Auto-select the first one
    if ( pReposList->GetEntryCount( ) > 0 )
    {
        pReposList->SelectEntryPos( 0 );
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
