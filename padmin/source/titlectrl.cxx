/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: titlectrl.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 12:17:27 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _PAD_NEWPPDLG_HXX_
#include <titlectrl.hxx>
#endif

using namespace padmin;

TitleImage::TitleImage( Window* pParent, const ResId& rResId ) :
        Control( pParent, rResId ),
        m_bArranged( false )
{
    Font aFont = GetFont();
    aFont.SetHeight( aFont.GetHeight()*3/2 );
    SetFont( aFont );
}

// -----------------------------------------------------------------------

TitleImage::~TitleImage()
{
}

// -----------------------------------------------------------------------

void TitleImage::arrange()
{
    m_bArranged = true;
    Size aCtrlSize( GetSizePixel() );
    Size aImageSize( m_aImage.GetSizePixel() );
    Size aTextSize( GetTextWidth( m_aText ), GetTextHeight() );

    m_aImagePos.Y() = ( aCtrlSize.Height() - aImageSize.Height() ) / 2;
    m_aImagePos.X() = m_aImagePos.Y() < 0 ? -m_aImagePos.Y() : m_aImagePos.Y();
    m_aTextPos.X() = m_aImagePos.X() + aImageSize.Width() + aTextSize.Height()/2;
    m_aTextPos.Y() = ( aCtrlSize.Height() - aTextSize.Height() ) / 2;
}

// -----------------------------------------------------------------------

void TitleImage::Paint( const Rectangle& )
{
    if( ! m_bArranged )
        arrange();

    SetLineColor( m_aBGColor );
    SetFillColor( m_aBGColor );
    DrawRect( Rectangle( Point( 0, 0 ), Size( GetSizePixel() ) ) );
    DrawImage( m_aImagePos, m_aImage );
    DrawText( m_aTextPos, m_aText );
}

// -----------------------------------------------------------------------

void TitleImage::SetText( const String& rText )
{
    m_aText = rText;
    m_bArranged = false;
    Invalidate();
}

// -----------------------------------------------------------------------

void TitleImage::SetImage( const Image& rImage )
{
    m_aImage = rImage;
    m_bArranged = false;
    Invalidate();
}

// -----------------------------------------------------------------------

void TitleImage::SetBackgroundColor( const Color& rColor )
{
    m_aBGColor = rColor;
    Invalidate();
}
