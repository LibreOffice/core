/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <titlectrl.hxx>

using namespace padmin;

TitleImage::TitleImage( Window* pParent, const ResId& rResId ) :
        Control( pParent, rResId ),
        m_bArranged( false )
{
    Font aFont = GetFont();
    aFont.SetHeight( aFont.GetHeight()*3/2 );
    SetFont( aFont );
}



TitleImage::~TitleImage()
{
}



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



void TitleImage::SetText( const OUString& rText )
{
    m_aText = rText;
    m_bArranged = false;
    Invalidate();
}



void TitleImage::SetImage( const Image& rImage )
{
    m_aImage = rImage;
    m_bArranged = false;
    Invalidate();
}



void TitleImage::SetBackgroundColor( const Color& rColor )
{
    m_aBGColor = rColor;
    Invalidate();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
