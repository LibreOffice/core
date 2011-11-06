/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
