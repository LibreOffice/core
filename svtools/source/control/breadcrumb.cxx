/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <svtools/breadcrumb.hxx>
#include <tools/urlobj.hxx>
#include <vcl/fixedhyper.hxx>

class CustomLink : public FixedHyperlink
{
public:
    CustomLink( vcl::Window* pParent, WinBits nWinStyle )
    : FixedHyperlink( pParent, nWinStyle )
    {
        vcl::Font aFont = GetControlFont( );
        aFont.SetUnderline( LINESTYLE_NONE );
        SetControlFont( aFont );
    }

protected:
    virtual void MouseMove( const MouseEvent& rMEvt ) override
    {
        // changes the style if the control is enabled
        if ( !rMEvt.IsLeaveWindow() && IsEnabled() )
        {
            vcl::Font aFont = GetControlFont( );
            aFont.SetUnderline( LINESTYLE_SINGLE );
            SetControlFont( aFont );
        }
        else
        {
            vcl::Font aFont = GetControlFont( );
            aFont.SetUnderline( LINESTYLE_NONE );
            SetControlFont( aFont );
        }

        FixedHyperlink::MouseMove( rMEvt );
    }
};

Breadcrumb::Breadcrumb( vcl::Window* pParent ) : VclHBox( pParent )
{
    m_eMode = SvtBreadcrumbMode::ONLY_CURRENT_PATH;
    set_spacing( SPACING );
    appendField(); // root
}

Breadcrumb::~Breadcrumb()
{
    disposeOnce();
}

void Breadcrumb::dispose()
{
    for( std::vector<VclPtr<FixedHyperlink>>::size_type i = 0; i < m_aLinks.size(); i++ )
    {
        m_aSeparators[i].disposeAndClear();
        m_aLinks[i].disposeAndClear();
    }

    VclHBox::dispose();
}

void Breadcrumb::EnableFields( bool bEnable )
{
    VclHBox::Enable( bEnable );
    if( bEnable )
    {
        INetURLObject aURL( m_aCurrentURL );
        int nSegments = aURL.getSegmentCount();
        m_aLinks[nSegments]->Enable( false );
    }
}

void Breadcrumb::SetClickHdl( const Link<Breadcrumb*,void>& rLink )
{
    m_aClickHdl = rLink;
}

const OUString& Breadcrumb::GetHdlURL()
{
    return m_sClickedURL;
}

void Breadcrumb::SetRootName( const OUString& rURL )
{
    m_sRootName = rURL;

    // we changed root - clear all fields
    for( std::vector<VclPtr<FixedHyperlink>>::size_type i = 1; i < m_aLinks.size(); i++ )
    {
        m_aLinks[i]->SetText( "" );

        m_aLinks[i]->Hide();
        m_aSeparators[i]->Hide();
        m_aLinks[i]->Enable();
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
        sHostPort += ":";
        sHostPort += OUString::number( aURL.GetPort() );
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

    m_aLinks[0]->SetText( m_sRootName );
    m_aLinks[0]->Enable();
    m_aLinks[0]->SetURL( sRootPath );

    // fill the other fields

    for( unsigned int i = 1; i < static_cast<unsigned int>(nSegments) + 1; i++ )
    {
        if( i >= m_aLinks.size() )
            appendField();

        unsigned int nEnd = sPath.indexOf( '/', nPos + 1 );
        OUString sLabel = sPath.copy( nPos + 1, nEnd - nPos - 1 );

        if( m_eMode == SvtBreadcrumbMode::ALL_VISITED )
        {
            if( m_aLinks[i]->GetText() != sLabel )
                bClear = true;
        }

        m_aLinks[i]->SetText( sLabel );
        m_aLinks[i]->SetURL( sRootPath + sPath.copy( 0, nEnd ) );
        m_aLinks[i]->Hide();
        m_aLinks[i]->Enable();

        m_aSeparators[i]->Hide();

        nPos = nEnd;
    }

    // clear unused fields

    for( std::vector<VclPtr<FixedHyperlink>>::size_type i = nSegments + 1; i < m_aLinks.size(); i++ )
    {
        if( bClear )
            m_aLinks[i]->SetText( "" );

        m_aLinks[i]->Hide();
        m_aSeparators[i]->Hide();
        m_aLinks[i]->Enable();
    }

    // show fields

    Resize();
    unsigned int nMaxWidth = GetSizePixel().Width();
    unsigned int nSeparatorWidth = m_aSeparators[0]->GetSizePixel().Width();
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

            if( showField( nIndex, nMaxWidth - nCurrentWidth ) )
            {
                nCurrentWidth += m_aLinks[nIndex]->GetSizePixel().Width()
                                + nSeparatorWidth + 2*SPACING;
            }
            else
            {
                // label is too long
                if( nSegments != 0 )
                {
                    m_aLinks[0]->SetText( "..." );
                    m_aLinks[0]->Enable( false );
                }
                bLeft = false;
            }
        }

        if( nSegments + i == static_cast<int>(m_aLinks.size()) )
            bRight = false;

        if( i != 0 && bRight )
        {
            unsigned int nIndex = nSegments + i;

            if( m_aLinks[nIndex]->GetText().isEmpty() )
            {
                bRight = false;
            }
            else if( showField( nIndex, nMaxWidth - nCurrentWidth ) )
            {
                nCurrentWidth += m_aLinks[nIndex]->GetSizePixel().Width()
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
    m_aLinks[nSegments]->Enable( false );

    // hide last separator
    m_aSeparators[nLastVisible]->Hide();
}

void Breadcrumb::SetMode( SvtBreadcrumbMode eMode )
{
    m_eMode = eMode;
}

void Breadcrumb::appendField()
{
    m_aLinks.push_back( VclPtr< CustomLink >::Create( this, WB_TABSTOP ) );
    m_aLinks[m_aLinks.size() - 1]->Hide();
    m_aLinks[m_aLinks.size() - 1]->SetClickHdl( LINK( this, Breadcrumb, ClickLinkHdl ) );

    m_aSeparators.push_back( VclPtr< FixedText >::Create( this ) );
    m_aSeparators[m_aLinks.size() - 1]->SetText( ">" );
    m_aSeparators[m_aLinks.size() - 1]->Hide();
}

bool Breadcrumb::showField( unsigned int nIndex, unsigned int nWidthMax )
{
    m_aLinks[nIndex]->Show();
    m_aSeparators[nIndex]->Show();

    unsigned int nSeparatorWidth = m_aSeparators[0]->GetSizePixel().Width();
    unsigned int nWidth = m_aLinks[nIndex]->GetSizePixel().Width()
            + nSeparatorWidth + 3*SPACING;

    if( nWidth > nWidthMax )
    {
        if( nIndex != 0 )
        {
            m_aLinks[nIndex]->Hide();
            m_aSeparators[nIndex]->Hide();
        }

        return false;
    }

    return true;
}

IMPL_LINK( Breadcrumb, ClickLinkHdl, FixedHyperlink&, rLink, void )
{
    m_sClickedURL = rLink.GetURL();
    m_aClickHdl.Call( this );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
