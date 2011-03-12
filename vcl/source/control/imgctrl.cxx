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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include <vcl/event.hxx>
#include <vcl/imgctrl.hxx>

#include <com/sun/star/awt/ImageScaleMode.hdl>

namespace ImageScaleMode = ::com::sun::star::awt::ImageScaleMode;

// -----------------------------------------------------------------------

ImageControl::ImageControl( Window* pParent, WinBits nStyle ) :
    FixedImage( pParent, nStyle )
{
    mnScaleMode = ImageScaleMode::Anisotropic;
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

void ImageControl::UserDraw( const UserDrawEvent& rUDEvt )
{
    USHORT nStyle = 0;
    BitmapEx* pBitmap = &maBmp;
    if ( !*pBitmap )
    {
        String  sText( GetText() );
        if ( !sText.Len() )
            return;

        WinBits nWinStyle = GetStyle();
        USHORT nTextStyle = FixedText::ImplGetTextStyle( nWinStyle );
        if ( !IsEnabled() )
            nTextStyle |= TEXT_DRAW_DISABLE;

        DrawText( rUDEvt.GetRect(), sText, nTextStyle );
        return;
    }

    const Rectangle& rPaintRect = rUDEvt.GetRect();
    const Size&      rBitmapSize = maBmp.GetSizePixel();

    if( nStyle & IMAGE_DRAW_COLORTRANSFORM )
    {
        // only images support IMAGE_DRAW_COLORTRANSFORM
        Image aImage( *pBitmap );
        if ( !!aImage )
        {
            switch ( mnScaleMode )
            {
            case ImageScaleMode::None:
            {
                rUDEvt.GetDevice()->DrawImage(
                    lcl_centerWithin( rPaintRect, rBitmapSize ), aImage, nStyle );
            }
            break;

            case ImageScaleMode::Isotropic:
            {
                const Size aPaintSize = lcl_calcPaintSize( rPaintRect, rBitmapSize );
                rUDEvt.GetDevice()->DrawImage(
                    lcl_centerWithin( rPaintRect, aPaintSize ),
                    aPaintSize,
                    aImage, nStyle );
            }
            break;

            case ImageScaleMode::Anisotropic:
            {
                rUDEvt.GetDevice()->DrawImage(
                    rPaintRect.TopLeft(),
                    rPaintRect.GetSize(),
                    aImage, nStyle );
            }
            break;

            default:
                OSL_FAIL( "ImageControl::UserDraw: unhandled scale mode!" );
                break;

            }   // switch ( mnScaleMode )
        }
    }
    else
    {
        switch ( mnScaleMode )
        {
        case ImageScaleMode::None:
        {
            pBitmap->Draw( rUDEvt.GetDevice(), lcl_centerWithin( rPaintRect, rBitmapSize ) );
        }
        break;

        case ImageScaleMode::Isotropic:
        {
            const Size aPaintSize = lcl_calcPaintSize( rPaintRect, rBitmapSize );
            pBitmap->Draw( rUDEvt.GetDevice(),
                           lcl_centerWithin( rPaintRect, aPaintSize ),
                           aPaintSize );
        }
        break;

        case ImageScaleMode::Anisotropic:
        {
            pBitmap->Draw( rUDEvt.GetDevice(),
                           rPaintRect.TopLeft(),
                           rPaintRect.GetSize() );
        }
        break;

        default:
            OSL_FAIL( "ImageControl::UserDraw: unhandled scale mode!" );
            break;

        }   // switch ( mnScaleMode )
    }
}

// -----------------------------------------------------------------------

void ImageControl::SetBitmap( const BitmapEx& rBmp )
{
    maBmp = rBmp;
    StateChanged( STATE_CHANGE_DATA );
}

// -----------------------------------------------------------------------

BOOL ImageControl::SetModeBitmap( const BitmapEx& rBitmap )
{
    SetBitmap( rBitmap );
    return TRUE;
}

// -----------------------------------------------------------------------

const BitmapEx& ImageControl::GetModeBitmap( ) const
{
    return maBmp;
}

// -----------------------------------------------------------------------

void    ImageControl::Paint( const Rectangle& rRect )
{
    FixedImage::Paint( rRect );
    if( HasFocus() )
    {
        Window *pWin = GetWindow( WINDOW_BORDER );

        BOOL bFlat = (GetBorderStyle() == 2);
        Rectangle aRect( Point(0,0), pWin->GetOutputSizePixel() );
        Color oldLineCol = pWin->GetLineColor();
        Color oldFillCol = pWin->GetFillColor();
        pWin->SetFillColor();
        pWin->SetLineColor( bFlat ? COL_WHITE : COL_BLACK );
        pWin->DrawRect( aRect );
        aRect.nLeft++;
        aRect.nRight--;
        aRect.nTop++;
        aRect.nBottom--;
        pWin->SetLineColor( bFlat ? COL_BLACK : COL_WHITE );
        pWin->DrawRect( aRect );
        pWin->SetLineColor( oldLineCol );
        pWin->SetFillColor( oldFillCol );
    }
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
