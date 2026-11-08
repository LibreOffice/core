/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <svtools/PlaceEditDialog.hxx>
#include <utility>

#include "ServerDetailsControls.hxx"

DetailsContainer::DetailsContainer(PlaceEditDialog* pDialog)
    : m_pDialog(pDialog)
{
    m_pDialog->m_xEDPort->set_value_formatter(LINK(this, DetailsContainer, FormatPortHdl));
}

//format without thousand separator
IMPL_STATIC_LINK(DetailsContainer, FormatPortHdl, sal_Int64, nValue, OUString)
{
    return OUString::number(nValue);
}

DetailsContainer::~DetailsContainer( )
{
}

void DetailsContainer::set_visible( bool )
{
    m_pDialog->m_xDetailsGrid->set_sensitive(true);

    m_pDialog->m_xEDHost->connect_changed( LINK( this, DetailsContainer, ValueChangeHdl ) );
    m_pDialog->m_xEDPort->connect_changed( LINK( this, DetailsContainer, ValueChangeHdl ) );
    m_pDialog->m_xEDRoot->connect_changed( LINK( this, DetailsContainer, ValueChangeHdl ) );
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

IMPL_LINK_NOARG( DetailsContainer, ValueChangeHdl, weld::Entry&, void )
{
    notifyChange( );
}

HostDetailsContainer::HostDetailsContainer(PlaceEditDialog* pDialog, sal_uInt16 nPort, OUString sScheme) :
    DetailsContainer( pDialog ),
    m_nDefaultPort( nPort ),
    m_sScheme(std::move( sScheme ))
{
    set_visible( false );
}

void HostDetailsContainer::set_visible( bool bShow )
{
    m_pDialog->m_xFTHost->set_visible( bShow );
    m_pDialog->m_xHostBox->set_visible( bShow );
    m_pDialog->m_xEDRoot->set_visible( bShow );
    m_pDialog->m_xFTRoot->set_visible( bShow );

    DetailsContainer::set_visible( bShow );

    if ( bShow )
    {
        if (m_pDialog->m_xEDPort->get_value() == 0)
            m_pDialog->m_xEDPort->set_value( m_nDefaultPort );
        m_pDialog->m_xEDHost->set_text( m_sHost );
    }
    else
        m_pDialog->m_xEDPort->set_value( 0 );
}

INetURLObject HostDetailsContainer::getUrl( )
{
    OUString sHost = m_pDialog->m_xEDHost->get_text().trim();
    sal_Int64 nPort = m_pDialog->m_xEDPort->get_value();
    OUString sPath = m_pDialog->m_xEDRoot->get_text().trim();

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
        m_sHost = rUrl.GetHost( );
        m_pDialog->m_xEDHost->set_text( rUrl.GetHost( ) );
        m_pDialog->m_xEDPort->set_value( rUrl.GetPort( ) );
        m_pDialog->m_xEDRoot->set_text( rUrl.GetURLPath() );
    }

    return bSuccess;
}

bool HostDetailsContainer::verifyScheme( const OUString& sScheme )
{
    return sScheme == Concat2View( m_sScheme + "://" );
}

DavDetailsContainer::DavDetailsContainer(PlaceEditDialog* pBuilder)
    : HostDetailsContainer(pBuilder, 80, u"http"_ustr)
{
    m_pDialog->m_xCBDavs->connect_toggled(LINK(this, DavDetailsContainer, ToggledDavsHdl));

    set_visible( false );
}

void DavDetailsContainer::set_visible( bool bShow )
{
    HostDetailsContainer::set_visible( bShow );

    if ( !bShow )
        m_pDialog->m_xCBDavs->set_active(false);

    m_pDialog->m_xCBDavs->set_visible(bShow);
}

bool DavDetailsContainer::verifyScheme( const OUString& rScheme )
{
    bool bValid = false;
    if ( rScheme == "http://" )
    {
        bValid = true;
        m_pDialog->m_xCBDavs->set_active(false);
        ToggledDavsHdl(*m_pDialog->m_xCBDavs);
    }
    else if ( rScheme == "https://" )
    {
        bValid = true;
        m_pDialog->m_xCBDavs->set_active(true);
        ToggledDavsHdl(*m_pDialog->m_xCBDavs);
    }
    return bValid;
}

IMPL_LINK( DavDetailsContainer, ToggledDavsHdl, weld::Toggleable&, rCheckBox, void )
{
    // Change default port if needed
    bool bCheckedDavs = rCheckBox.get_active();
    if ( m_pDialog->m_xEDPort->get_value() == 80 && bCheckedDavs )
        m_pDialog->m_xEDPort->set_value( 443 );
    else if ( m_pDialog->m_xEDPort->get_value() == 443 && !bCheckedDavs )
        m_pDialog->m_xEDPort->set_value( 80 );

    OUString sScheme( u"http"_ustr );
    if ( bCheckedDavs )
        sScheme = u"https"_ustr;
    setScheme( sScheme );

    notifyChange( );
}

SmbDetailsContainer::SmbDetailsContainer(PlaceEditDialog* pDialog)
    : DetailsContainer(pDialog)
{
    m_pDialog->m_xEDShare->connect_changed( LINK( this, DetailsContainer, ValueChangeHdl ) );

    set_visible( false );
}

INetURLObject SmbDetailsContainer::getUrl( )
{
    OUString sHost = m_pDialog->m_xEDHost->get_text().trim( );
    OUString sShare = m_pDialog->m_xEDShare->get_text().trim( );
    OUString sPath = m_pDialog->m_xEDRoot->get_text().trim( );

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

        m_sHost = rUrl.GetHost( );
        m_pDialog->m_xEDHost->set_text( m_sHost );
        m_pDialog->m_xEDShare->set_text( sShare );
        m_pDialog->m_xEDRoot->set_text( sPath );
    }

    return bSuccess;
}

void SmbDetailsContainer::set_visible( bool bShow )
{
    m_pDialog->m_xEDShare->set_visible( bShow );
    m_pDialog->m_xFTShare->set_visible( bShow );
    m_pDialog->m_xEDRoot->set_visible( bShow );
    m_pDialog->m_xFTRoot->set_visible( bShow );

    m_pDialog->m_xFTHost->set_visible( bShow );
    m_pDialog->m_xHostBox->set_visible( bShow );
    m_pDialog->m_xEDPort->set_sensitive( !bShow );
    m_pDialog->m_xFTPort->set_sensitive( !bShow );

    if ( bShow )
        m_pDialog->m_xEDHost->set_text( m_sHost );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
