/*************************************************************************
 *
 *  $RCSfile: imgctrl.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: ssa $ $Date: 2002-08-15 14:48:40 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#define _SV_IMGCTRL_CXX

#ifndef _SV_EVENT_HXX
#include <event.hxx>
#endif
#include <imgctrl.hxx>

// -----------------------------------------------------------------------

ImageControl::ImageControl( Window* pParent, WinBits nStyle ) :
    FixedImage( pParent, nStyle )
{
    mnDummy1_mbScaleImage = TRUE;
}

// -----------------------------------------------------------------------

void ImageControl::SetScaleImage( BOOL bScale )
{
    if ( bScale != mnDummy1_mbScaleImage )
    {
        mnDummy1_mbScaleImage = bScale;
        Invalidate();
    }
}

// -----------------------------------------------------------------------

BOOL ImageControl::IsScaleImage() const
{
    // Make inline when changing member from dummy...
    return (BOOL)mnDummy1_mbScaleImage;
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
            if ( mnDummy1_mbScaleImage )
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
        if ( mnDummy1_mbScaleImage )
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

