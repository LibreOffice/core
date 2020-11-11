/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <tools/urlobj.hxx>
#include <vcl/svapp.hxx>
#include "breadcrumb.hxx"

Breadcrumb::Breadcrumb(weld::Container* pParent)
    : m_pParent(pParent)
    , m_nMaxWidth(m_pParent->get_preferred_size().Width())
{
    m_pParent->connect_size_allocate(LINK(this, Breadcrumb, SizeAllocHdl));
    m_eMode = SvtBreadcrumbMode::ONLY_CURRENT_PATH;
    appendField(); // root
}

IMPL_LINK(Breadcrumb, SizeAllocHdl, const Size&, rSize, void)
{
    m_nMaxWidth = rSize.Width();
}

Breadcrumb::~Breadcrumb()
{
    m_pParent->connect_size_allocate(Link<const Size&, void>());
}

void Breadcrumb::EnableFields( bool bEnable )
{
    if( bEnable )
    {
        INetURLObject aURL( m_aCurrentURL );
        int nSegments = aURL.getSegmentCount();
        m_aSegments[nSegments]->m_xLink->set_sensitive(false);
    }
}

void Breadcrumb::connect_clicked( const Link<Breadcrumb*,bool>& rLink )
{
    m_aClickHdl = rLink;
}

const OUString& Breadcrumb::GetHdlURL() const
{
    return m_sClickedURL;
}

void Breadcrumb::SetRootName( const OUString& rURL )
{
    m_sRootName = rURL;

    // we changed root - clear all fields
    for (size_t i = 1; i < m_aSegments.size(); ++i)
    {
        m_aSegments[i]->m_xLink->set_label("");

        m_aSegments[i]->m_xLink->hide();
        m_aSegments[i]->m_xSeparator->hide();
        m_aSegments[i]->m_xLink->set_sensitive(true);
    }
}

void Breadcrumb::SetURL( const OUString& rURL )
{
    m_aCurrentURL = rURL;
    INetURLObject aURL( rURL );
    aURL.setFinalSlash();
    //prepare the Host port
    OUString sHostPort;

    if( aURL.HasPort() )
    {
        sHostPort += ":" + OUString::number( aURL.GetPort() );
    }

    OUString sUser = aURL.GetUser( INetURLObject::DecodeMechanism::NONE );
    OUString sPath = aURL.GetURLPath(INetURLObject::DecodeMechanism::WithCharset);
    OUString sRootPath = INetURLObject::GetScheme( aURL.GetProtocol() )
                        + sUser
                        + ( sUser.isEmpty() ? OUString() : "@" )
                        + aURL.GetHost()
                        + sHostPort;

    int nSegments = aURL.getSegmentCount();
    unsigned int nPos = 0;

    bool bClear = ( m_eMode == SvtBreadcrumbMode::ONLY_CURRENT_PATH );

    // root field
    m_aSegments[0]->m_xLink->set_label( m_sRootName );
    m_aSegments[0]->m_xLink->set_sensitive(true);
    m_aSegments[0]->m_xLink->set_uri(sRootPath);
    m_aUris[m_aSegments[0]->m_xLink.get()] = sRootPath;

    // fill the other fields

    for( unsigned int i = 1; i < static_cast<unsigned int>(nSegments) + 1; i++ )
    {
        if( i >= m_aSegments.size() )
            appendField();

        unsigned int nEnd = sPath.indexOf( '/', nPos + 1 );
        OUString sLabel = sPath.copy( nPos + 1, nEnd - nPos - 1 );

        if( m_eMode == SvtBreadcrumbMode::ALL_VISITED )
        {
            if( m_aSegments[i]->m_xLink->get_label() != sLabel )
                bClear = true;
        }

        m_aSegments[i]->m_xLink->set_label( sLabel );
        m_aUris[m_aSegments[i]->m_xLink.get()] = sRootPath + sPath.subView(0, nEnd);
        m_aSegments[i]->m_xLink->hide();
        m_aSegments[i]->m_xLink->set_sensitive(true);

        m_aSegments[i]->m_xSeparator->hide();

        nPos = nEnd;
    }

    // clear unused fields
    for (size_t i = nSegments + 1; i < m_aSegments.size(); i++ )
    {
        if( bClear )
            m_aSegments[i]->m_xLink->set_label( "" );

        m_aSegments[i]->m_xLink->hide();
        m_aSegments[i]->m_xSeparator->hide();
        m_aSegments[i]->m_xLink->set_sensitive(true);
    }

    // show fields
    unsigned int nSeparatorWidth = m_aSegments[0]->m_xSeparator->get_preferred_size().Width();
    unsigned int nCurrentWidth = 0;
    unsigned int nLastVisible = nSegments;

    bool bRight = ( m_eMode == SvtBreadcrumbMode::ALL_VISITED );
    bool bLeft = true;

    int i = 0;

    while( bLeft || bRight )
    {
        if( nSegments - i == -1 )
            bLeft = false;

        if( bLeft )
        {
            unsigned int nIndex = nSegments - i;

            if( showField( nIndex, m_nMaxWidth - nCurrentWidth ) )
            {
                nCurrentWidth += m_aSegments[nIndex]->m_xLink->get_preferred_size().Width()
                                + nSeparatorWidth + 2*SPACING;
            }
            else
            {
                // label is too long
                if( nSegments != 0 )
                {
                    m_aSegments[0]->m_xLink->set_label("...");
                    m_aSegments[0]->m_xLink->set_sensitive(false);
                }
                bLeft = false;
            }
        }

        if( nSegments + i == static_cast<int>(m_aSegments.size()) )
            bRight = false;

        if( i != 0 && bRight )
        {
            unsigned int nIndex = nSegments + i;

            if( m_aSegments[nIndex]->m_xLink->get_label().isEmpty() )
            {
                bRight = false;
            }
            else if( showField( nIndex, m_nMaxWidth - nCurrentWidth ) )
            {
                nCurrentWidth += m_aSegments[nIndex]->m_xLink->get_preferred_size().Width()
                                + nSeparatorWidth + 3*SPACING;
                nLastVisible = nIndex;
            }
            else
            {
                bRight = false;
            }
        }

        i++;
    }

    // current dir should be inactive
    m_aSegments[nSegments]->m_xLink->set_sensitive(false);

    // hide last separator
    m_aSegments[nLastVisible]->m_xSeparator->hide();
}

void Breadcrumb::SetMode( SvtBreadcrumbMode eMode )
{
    m_eMode = eMode;
}

void Breadcrumb::appendField()
{
    m_aSegments.emplace_back(std::make_unique<BreadcrumbPath>(m_pParent));
    size_t nIndex = m_aSegments.size() - 1;
    m_aSegments[nIndex]->m_xLink->hide();
    m_aSegments[nIndex]->m_xLink->connect_activate_link(LINK(this, Breadcrumb, ClickLinkHdl));
    m_aSegments[nIndex]->m_xSeparator->set_label( ">" );
    m_aSegments[nIndex]->m_xSeparator->hide();
}

bool Breadcrumb::showField( unsigned int nIndex, unsigned int nWidthMax )
{
    m_aSegments[nIndex]->m_xLink->show();
    m_aSegments[nIndex]->m_xSeparator->show();

    unsigned int nSeparatorWidth = m_aSegments[0]->m_xSeparator->get_preferred_size().Width();
    unsigned int nWidth = m_aSegments[nIndex]->m_xLink->get_preferred_size().Width()
            + nSeparatorWidth + 3*SPACING;

    if( nWidth > nWidthMax )
    {
        if( nIndex != 0 )
        {
            m_aSegments[nIndex]->m_xLink->hide();
            m_aSegments[nIndex]->m_xSeparator->hide();
        }

        return false;
    }

    return true;
}

IMPL_LINK(Breadcrumb, ClickLinkHdl, weld::LinkButton&, rLink, bool)
{
    m_sClickedURL = m_aUris[&rLink];
    return m_aClickHdl.Call(this);
}

BreadcrumbPath::BreadcrumbPath(weld::Container* pContainer)
    : m_xBuilder(Application::CreateBuilder(pContainer, "fps/ui/breadcrumb.ui"))
    , m_xContainer(m_xBuilder->weld_container("container"))
    , m_xLink(m_xBuilder->weld_link_button("link"))
    , m_xSeparator(m_xBuilder->weld_label("label"))
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
