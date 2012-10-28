/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#include <vcl/event.hxx>
#include <vcl/imgctrl.hxx>
#include <tools/rcid.h>

#include <com/sun/star/awt/ImageScaleMode.hpp>

namespace ImageScaleMode = ::com::sun::star::awt::ImageScaleMode;

// -----------------------------------------------------------------------

ImageControl::ImageControl( Window* pParent, WinBits nStyle )
    :FixedImage( pParent, nStyle )
    ,mnScaleMode( ImageScaleMode::Anisotropic )
{
}

// -----------------------------------------------------------------------

ImageControl::ImageControl( Window* pParent, const ResId& rResId )
    :FixedImage( pParent, rResId )
    ,mnScaleMode( ImageScaleMode::Anisotropic )
{
}

// -----------------------------------------------------------------------

void ImageControl::SetScaleMode( const ::sal_Int16 _nMode )
{
    if ( _nMode != mnScaleMode )
    {
        mnScaleMode = _nMode;
        Invalidate();
    }
}

// -----------------------------------------------------------------------

void ImageControl::Resize()
{
    Invalidate();
}

// -----------------------------------------------------------------------
namespace
{
    static Size lcl_calcPaintSize( const Rectangle& _rPaintRect, const Size& _rBitmapSize )
    {
        const Size aPaintSize = _rPaintRect.GetSize();

        const double nRatioX = 1.0 * aPaintSize.Width() / _rBitmapSize.Width();
        const double nRatioY = 1.0 * aPaintSize.Height() / _rBitmapSize.Height();
        const double nRatioMin = ::std::min( nRatioX, nRatioY );

        return Size( long( _rBitmapSize.Width() * nRatioMin ), long( _rBitmapSize.Height() * nRatioMin ) );
    }

    static Point lcl_centerWithin( const Rectangle& _rArea, const Size& _rObjectSize )
    {
        Point aPos( _rArea.TopLeft() );
        aPos.X() += ( _rArea.GetWidth() - _rObjectSize.Width() ) / 2;
        aPos.Y() += ( _rArea.GetHeight() - _rObjectSize.Height() ) / 2;
        return aPos;
    }
}

// -----------------------------------------------------------------------

void ImageControl::ImplDraw( OutputDevice& rDev, sal_uLong nDrawFlags, const Point& rPos, const Size& rSize ) const
{
    sal_uInt16 nStyle = 0;
    if ( !(nDrawFlags & WINDOW_DRAW_NODISABLE) )
    {
        if ( !IsEnabled() )
            nStyle |= IMAGE_DRAW_DISABLE;
    }

    const Image& rImage( GetModeImage() );
    const Image* pImage = &rImage;
    const Rectangle aDrawRect( rPos, rSize );
    if ( !*pImage )
    {
        String  sText( GetText() );
        if ( !sText.Len() )
            return;

        WinBits nWinStyle = GetStyle();
        sal_uInt16 nTextStyle = FixedText::ImplGetTextStyle( nWinStyle );
        if ( !(nDrawFlags & WINDOW_DRAW_NODISABLE) )
            if ( !IsEnabled() )
                nTextStyle |= TEXT_DRAW_DISABLE;

        rDev.DrawText( aDrawRect, sText, nTextStyle );
        return;
    }

    const Size&      rBitmapSize = pImage->GetSizePixel();

    switch ( mnScaleMode )
    {
    case ImageScaleMode::None:
    {
        rDev.DrawImage( lcl_centerWithin( aDrawRect, rBitmapSize ), *pImage, nStyle );
    }
    break;

    case ImageScaleMode::Isotropic:
    {
        const Size aPaintSize = lcl_calcPaintSize( aDrawRect, rBitmapSize );
        rDev.DrawImage(
            lcl_centerWithin( aDrawRect, aPaintSize ),
            aPaintSize,
            *pImage, nStyle );
    }
    break;

    case ImageScaleMode::Anisotropic:
    {
        rDev.DrawImage(
            aDrawRect.TopLeft(),
            aDrawRect.GetSize(),
            *pImage, nStyle );
    }
    break;

    default:
        OSL_ENSURE( false, "ImageControl::ImplDraw: unhandled scale mode!" );
        break;

    }   // switch ( mnScaleMode )
}

// -----------------------------------------------------------------------

void ImageControl::Paint( const Rectangle& /*rRect*/ )
{
    ImplDraw( *this, 0, Point(), GetOutputSizePixel() );

    if( HasFocus() )
    {
        Window *pWin = GetWindow( WINDOW_BORDER );

        sal_Bool bFlat = (GetBorderStyle() == 2);
        Rectangle aRect( Point(0,0), pWin->GetOutputSizePixel() );
        Color oldLineCol = pWin->GetLineColor();
        Color oldFillCol = pWin->GetFillColor();
        pWin->SetFillColor();
        pWin->SetLineColor( bFlat ? COL_WHITE : COL_BLACK );
        pWin->DrawRect( aRect );
        ++aRect.Left();
        --aRect.Right();
        ++aRect.Top();
        --aRect.Bottom();
        pWin->SetLineColor( bFlat ? COL_BLACK : COL_WHITE );
        pWin->DrawRect( aRect );
        pWin->SetLineColor( oldLineCol );
        pWin->SetFillColor( oldFillCol );
    }
}

// -----------------------------------------------------------------------
void ImageControl::Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, sal_uLong nFlags )
{
    const Point     aPos  = pDev->LogicToPixel( rPos );
    const Size      aSize = pDev->LogicToPixel( rSize );
          Rectangle aRect( aPos, aSize );

    pDev->Push();
    pDev->SetMapMode();

    // Border
    if ( !(nFlags & WINDOW_DRAW_NOBORDER) && (GetStyle() & WB_BORDER) )
    {
        ImplDrawFrame( pDev, aRect );
    }
    pDev->IntersectClipRegion( aRect );
    ImplDraw( *pDev, nFlags, aRect.TopLeft(), aRect.GetSize() );

    pDev->Pop();
}

// -----------------------------------------------------------------------

void ImageControl::GetFocus()
{
    FixedImage::GetFocus();
    GetWindow( WINDOW_BORDER )->Invalidate();
}

// -----------------------------------------------------------------------

void ImageControl::LoseFocus()
{
    FixedImage::GetFocus();
    GetWindow( WINDOW_BORDER )->Invalidate();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
