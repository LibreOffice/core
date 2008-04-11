/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: imgctrl.cxx,v $
 * $Revision: 1.12 $
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

// -----------------------------------------------------------------------

ImageControl::ImageControl( Window* pParent, WinBits nStyle ) :
    FixedImage( pParent, nStyle )
{
    mbScaleImage = TRUE;
}

// -----------------------------------------------------------------------

void ImageControl::SetScaleImage( BOOL bScale )
{
    if ( bScale != mbScaleImage )
    {
        mbScaleImage = bScale;
        Invalidate();
    }
}

// -----------------------------------------------------------------------

BOOL ImageControl::IsScaleImage() const
{
    // Make inline when changing member from dummy...
    return mbScaleImage;
}


// -----------------------------------------------------------------------

void ImageControl::Resize()
{
    Invalidate();
}

// -----------------------------------------------------------------------

void ImageControl::UserDraw( const UserDrawEvent& rUDEvt )
{
    USHORT nStyle = 0;
    BitmapEx* pBitmap = &maBmp;
    Color aCol;
    if( !!maBmpHC && ImplGetCurrentBackgroundColor( aCol ) )
    {
        if( aCol.IsDark() )
            pBitmap = &maBmpHC;
        // #99902 no col transform required
        //if( aCol.IsBright() )
        //  nStyle |= IMAGE_DRAW_COLORTRANSFORM;
    }

    if( nStyle & IMAGE_DRAW_COLORTRANSFORM )
    {
        // only images support IMAGE_DRAW_COLORTRANSFORM
        Image aImage( maBmp );
        if ( !!aImage )
        {
            if ( mbScaleImage )
                rUDEvt.GetDevice()->DrawImage( rUDEvt.GetRect().TopLeft(),
                                                rUDEvt.GetRect().GetSize(),
                                                aImage, nStyle );
            else
            {
                // Center...
                Point aPos( rUDEvt.GetRect().TopLeft() );
                aPos.X() += ( rUDEvt.GetRect().GetWidth() - maBmp.GetSizePixel().Width() ) / 2;
                aPos.Y() += ( rUDEvt.GetRect().GetHeight() - maBmp.GetSizePixel().Height() ) / 2;
                rUDEvt.GetDevice()->DrawImage( aPos, aImage, nStyle );
            }
        }
    }
    else
    {
        if ( mbScaleImage )
        {
            maBmp.Draw( rUDEvt.GetDevice(),
                        rUDEvt.GetRect().TopLeft(),
                        rUDEvt.GetRect().GetSize() );
        }
        else
        {
            // Center...
            Point aPos( rUDEvt.GetRect().TopLeft() );
            aPos.X() += ( rUDEvt.GetRect().GetWidth() - maBmp.GetSizePixel().Width() ) / 2;
            aPos.Y() += ( rUDEvt.GetRect().GetHeight() - maBmp.GetSizePixel().Height() ) / 2;
            maBmp.Draw( rUDEvt.GetDevice(), aPos );
        }
    }
}

// -----------------------------------------------------------------------

void ImageControl::SetBitmap( const BitmapEx& rBmp )
{
    maBmp = rBmp;
    StateChanged( STATE_CHANGE_DATA );
}

// -----------------------------------------------------------------------

BOOL ImageControl::SetModeBitmap( const BitmapEx& rBitmap, BmpColorMode eMode )
{
    if( eMode == BMP_COLOR_NORMAL )
        SetBitmap( rBitmap );
    else if( eMode == BMP_COLOR_HIGHCONTRAST )
    {
        maBmpHC = rBitmap;
        StateChanged( STATE_CHANGE_DATA );
    }
    else
        return FALSE;
    return TRUE;
}

// -----------------------------------------------------------------------

const BitmapEx& ImageControl::GetModeBitmap( BmpColorMode eMode ) const
{
    if( eMode == BMP_COLOR_HIGHCONTRAST )
        return maBmpHC;
    else
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

