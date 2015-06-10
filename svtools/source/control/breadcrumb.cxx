/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <svtools/breadcrumb.hxx>

Breadcrumb::Breadcrumb( vcl::Window* pParent, WinBits nWinStyle ) : VclHBox( pParent, nWinStyle )
{
    m_eMode = SvtBreadcrumbMode::ONLY_CURRENT_PATH;
    set_spacing( 6 );
    appendField(); // root
}

Breadcrumb::~Breadcrumb()
{
    disposeOnce();
}

void Breadcrumb::dispose()
{
    for( unsigned int i = 0; i < m_aLinks.size(); i++ )
    {
        m_aSeparators[i].disposeAndClear();
        m_aLinks[i].disposeAndClear();
    }

    VclHBox::dispose();
}

void Breadcrumb::SetClickHdl( const Link<>& rLink )
{
    m_aClickHdl = rLink;
}

OUString Breadcrumb::GetHdlURL()
{
    return m_sClickedURL;
}

void Breadcrumb::SetRootName( const OUString& rURL )
{
    m_sRootName = rURL;
}

void Breadcrumb::SetURL( const OUString& rURL )
{
    INetURLObject aURL( rURL );
    aURL.setFinalSlash();
    OUString sPath = aURL.GetURLPath(INetURLObject::DECODE_WITH_CHARSET);

    unsigned int nSegments = aURL.getSegmentCount();
    unsigned int nPos = 0;
    unsigned int i;

    bool bClear = ( m_eMode == SvtBreadcrumbMode::ONLY_CURRENT_PATH );

    m_aLinks[0]->SetText( m_sRootName );
    m_aLinks[0]->Show();
    m_aLinks[0]->Enable( true );
    m_aLinks[0]->SetURL( INetURLObject::GetScheme( aURL.GetProtocol() )
                                + aURL.GetHost() );
    m_aSeparators[0]->Show();

    for( i = 1; i < nSegments + 1; i++ )
    {
        if( i >= m_aLinks.size() )
            appendField();

        unsigned int nEnd = sPath.indexOf( '/', nPos + 1 );
        OUString sLabel = OUString( sPath.getStr() + nPos + 1, nEnd - nPos - 1 );

        if( m_eMode == SvtBreadcrumbMode::ALL_VISITED )
        {
            if( m_aLinks[i]->GetText() != sLabel )
                bClear = true;
        }


        m_aLinks[i]->SetText( sLabel );
        m_aLinks[i]->SetURL( INetURLObject::GetScheme( aURL.GetProtocol() )
                                + aURL.GetHost()
                                + OUString( sPath.getStr(), nEnd ) );
        m_aLinks[i]->Show();
        m_aLinks[i]->Enable( true );
        m_aSeparators[i]->Show();

        nPos = nEnd;
    }

    m_aLinks[i - 1]->Enable( false );
    m_aSeparators[i - 1]->Hide();

    if( bClear )
    {
        clearFields( i );
    }
    else
    {
        for( ; i < m_aLinks.size(); i++ )
            m_aLinks[i]->Enable( true );
    }
}

void Breadcrumb::SetMode( SvtBreadcrumbMode eMode )
{
    m_eMode = eMode;
}

void Breadcrumb::appendField()
{
    m_aLinks.push_back( VclPtr< FixedHyperlink >::Create( this ) );
    m_aLinks[m_aLinks.size() - 1]->Hide();
    m_aLinks[m_aLinks.size() - 1]->SetClickHdl( LINK( this, Breadcrumb, ClickLinkHdl ) );

    m_aSeparators.push_back( VclPtr< FixedText >::Create( this ) );
    m_aSeparators[m_aLinks.size() - 1]->SetText( ">" );
    m_aSeparators[m_aLinks.size() - 1]->Hide();
}

void Breadcrumb::clearFields( unsigned int nStartIndex )
{
    for( unsigned int i = nStartIndex; i < m_aLinks.size(); i++ )
    {
        m_aLinks[i]->Hide();
        m_aSeparators[i]->Hide();
    }
}

IMPL_LINK ( Breadcrumb, ClickLinkHdl, FixedHyperlink*, pLink )
{
    m_sClickedURL = pLink->GetURL();
    m_aClickHdl.Call( this );

    return 1;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
