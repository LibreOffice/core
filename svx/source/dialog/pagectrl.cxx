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
#include "precompiled_svx.hxx"

// include ---------------------------------------------------------------


#include <vcl/bitmap.hxx>
#include <editeng/frmdiritem.hxx>
#include <svx/pageitem.hxx>
#include <svx/pagectrl.hxx>
#include <editeng/boxitem.hxx>

#include <algorithm>

// struct PageWindow_Impl ------------------------------------------------

struct PageWindow_Impl
{
    SvxBoxItem*     pBorder;
    Bitmap          aBitmap;
    bool            bBitmap;
    sal_Bool        bResetBackground;
    sal_Bool        bFrameDirection;
    sal_Int32       nFrameDirection;


    PageWindow_Impl() :
        pBorder(0),
        bBitmap(FALSE),
        bResetBackground(sal_False),
        bFrameDirection(sal_False),
        nFrameDirection(0) {}

    void        EnableFrameDirection(sal_Bool bEnable){bFrameDirection = bEnable;}
    void        SetFrameDirection(sal_Int32 nDirection){nFrameDirection = nDirection;}

};

// STATIC DATA -----------------------------------------------------------

#define CELL_WIDTH      1600L
#define CELL_HEIGHT      800L

// class SvxPageWindow ---------------------------------------------------

SvxPageWindow::SvxPageWindow( Window* pParent, const ResId& rId ) :

    Window( pParent, rId ),

    nTop        ( 0 ),
    nBottom     ( 0 ),
    nLeft       ( 0 ),
    nRight      ( 0 ),
    aColor      ( COL_TRANSPARENT ),
    nHdLeft     ( 0 ),
    nHdRight    ( 0 ),
    nHdDist     ( 0 ),
    nHdHeight   ( 0 ),
    aHdColor    ( COL_TRANSPARENT ),
    pHdBorder   ( 0 ),
    nFtLeft     ( 0 ),
    nFtRight    ( 0 ),
    nFtDist     ( 0 ),
    nFtHeight   ( 0 ),
    aFtColor    ( COL_TRANSPARENT ),
    pFtBorder   ( 0 ),
    bFooter     ( FALSE ),
    bHeader     ( FALSE ),
    bTable      ( FALSE ),
    bHorz       ( FALSE ),
    bVert       ( FALSE ),
    eUsage      ( SVX_PAGE_ALL )

{
    pImpl = new PageWindow_Impl;

    // Count in Twips by default
    SetMapMode( MapMode( MAP_TWIP ) );
    aWinSize = GetOutputSizePixel();
    aWinSize.Height() -= 4;
    aWinSize.Width() -= 4;

    aWinSize = PixelToLogic( aWinSize );
    SetBackground();
}

// -----------------------------------------------------------------------

SvxPageWindow::~SvxPageWindow()
{
    delete pImpl;
    delete pHdBorder;
    delete pFtBorder;
}

// -----------------------------------------------------------------------

void SvxPageWindow::Paint( const Rectangle& )
{
    Fraction aXScale( aWinSize.Width(), std::max( (long) (aSize.Width() * 2 + aSize.Width() / 8), 1L ) );
    Fraction aYScale( aWinSize.Height(), std::max( aSize.Height(), 1L ) );
    MapMode aMapMode( GetMapMode() );

    if ( aYScale < aXScale )
    {
        aMapMode.SetScaleX( aYScale );
        aMapMode.SetScaleY( aYScale );
    }
    else
    {
        aMapMode.SetScaleX( aXScale );
        aMapMode.SetScaleY( aXScale );
    }
    SetMapMode( aMapMode );
    Size aSz( PixelToLogic( GetSizePixel() ) );
    long nYPos = ( aSz.Height() - aSize.Height() ) / 2;

    if ( eUsage == SVX_PAGE_ALL )
    {
        // all pages are equal -> draw one page
        if ( aSize.Width() > aSize.Height() )
        {
            // Draw Landscape page of the same size
            Fraction aX = aMapMode.GetScaleX();
            Fraction aY = aMapMode.GetScaleY();
            Fraction a2( 1.5 );
            aX *= a2;
            aY *= a2;
            aMapMode.SetScaleX( aX );
            aMapMode.SetScaleY( aY );
            SetMapMode( aMapMode );
            aSz = PixelToLogic( GetSizePixel() );
            nYPos = ( aSz.Height() - aSize.Height() ) / 2;
            long nXPos = ( aSz.Width() - aSize.Width() ) / 2;
            DrawPage( Point( nXPos, nYPos ), TRUE, TRUE );
        }
        else
            // Portrait
            DrawPage( Point( ( aSz.Width() - aSize.Width() ) / 2, nYPos ), TRUE, TRUE );
    }
    else
    {
        // Left and right page are different -> draw two pages if possible
        DrawPage( Point( 0, nYPos ), FALSE, (BOOL)( eUsage & SVX_PAGE_LEFT ) );
        DrawPage( Point( aSize.Width() + aSize.Width() / 8, nYPos ), TRUE,
                  (BOOL)( eUsage & SVX_PAGE_RIGHT ) );
    }
}

// -----------------------------------------------------------------------
void SvxPageWindow::DrawPage( const Point& rOrg, const BOOL bSecond, const BOOL bEnabled )
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    const Color& rFieldColor = rStyleSettings.GetFieldColor();
    const Color& rFieldTextColor = rStyleSettings.GetFieldTextColor();
    const Color& rDisableColor = rStyleSettings.GetDisableColor();
    const Color& rDlgColor = rStyleSettings.GetDialogColor();

    // background
    if(!bSecond || pImpl->bResetBackground)
    {
        SetLineColor( Color(COL_TRANSPARENT) );
        SetFillColor( rDlgColor );
        Size winSize(GetOutputSize());
        DrawRect( Rectangle( Point(0,0), winSize ) );

        if ( pImpl->bResetBackground )
            pImpl->bResetBackground = sal_False;
    }
    SetLineColor( rFieldTextColor );
    // Shadow
    Size aTempSize = aSize;
    // Page
    if ( !bEnabled )
    {
        SetFillColor( rDisableColor );
        DrawRect( Rectangle( rOrg, aTempSize ) );
        return;
    }
    SetFillColor( rFieldColor );
    DrawRect( Rectangle( rOrg, aTempSize ) );

    // Border Top Bottom Left Right
    Point aBegin( rOrg );
    Point aEnd( rOrg );

    long nL = nLeft;
    long nR = nRight;

    if ( eUsage == SVX_PAGE_MIRROR && !bSecond )
    {
        // turn for mirrored
        nL = nRight;
        nR = nLeft;
    }

    Rectangle aRect;

    aRect.Left()  = rOrg.X() + nL;
    aRect.Right() = rOrg.X() + aTempSize.Width() - nR;
    aRect.Top()   = rOrg.Y() + nTop;
    aRect.Bottom()= rOrg.Y() + aTempSize.Height() - nBottom;

    Rectangle aHdRect( aRect );
    Rectangle aFtRect( aRect );

    if ( bHeader )
    {
        // show headers if possible
        aHdRect.Left() += nHdLeft;
        aHdRect.Right() -= nHdRight;
        aHdRect.Bottom() = aRect.Top() + nHdHeight;
        aRect.Top() += nHdHeight + nHdDist;
        SetFillColor( aHdColor );
        DrawRect( aHdRect );
    }

    if ( bFooter )
    {
        // show footer if possible
        aFtRect.Left() += nFtLeft;
        aFtRect.Right() -= nFtRight;
        aFtRect.Top() = aRect.Bottom() - nFtHeight;
        aRect.Bottom() -= nFtHeight + nFtDist;
        SetFillColor( aFtColor );
        DrawRect( aFtRect );
    }

    // Paint Body
    SetFillColor( aColor );
    if ( pImpl->bBitmap )
    {
        DrawRect( aRect );
        Point aBmpPnt = aRect.TopLeft();
        Size aBmpSiz = aRect.GetSize();
        long nDeltaX = aBmpSiz.Width() / 15;
        long nDeltaY = aBmpSiz.Height() / 15;
        aBmpPnt.X() += nDeltaX;
        aBmpPnt.Y() += nDeltaY;
        aBmpSiz.Width() -= nDeltaX * 2;
        aBmpSiz.Height() -= nDeltaY * 2;
        DrawBitmap( aBmpPnt, aBmpSiz, pImpl->aBitmap );
    }
    else
        DrawRect( aRect );

    if(pImpl->bFrameDirection && !bTable)
    {
        Point aPos;
        Font aFont(GetFont());
        const Size aSaveSize = aFont.GetSize();
        Size aDrawSize( 0, aRect.GetHeight() / 6);
        aFont.SetSize(aDrawSize);
        SetFont(aFont);
        String sText(String::CreateFromAscii("ABC"));
        Point aMove(1, GetTextHeight());
        sal_Unicode cArrow = 0x2193;
        long nAWidth = GetTextWidth(String(sText.GetChar(0)));
        switch(pImpl->nFrameDirection)
        {
            case FRMDIR_HORI_LEFT_TOP:
                aPos = aRect.TopLeft();
                aPos.X() += PixelToLogic(Point(1,1)).X();
                aMove.Y() = 0;
                cArrow = 0x2192;
            break;
            case FRMDIR_HORI_RIGHT_TOP:
                aPos = aRect.TopRight();
                aPos.X() -= nAWidth;
                aMove.Y() = 0;
                aMove.X() *= -1;
                cArrow = 0x2190;
            break;
            case FRMDIR_VERT_TOP_LEFT:
                aPos = aRect.TopLeft();
                aPos.X() += PixelToLogic(Point(1,1)).X();
                aMove.X() = 0;
            break;
            case FRMDIR_VERT_TOP_RIGHT:
                aPos = aRect.TopRight();
                aPos.X() -= nAWidth;
                aMove.X() = 0;
            break;
        }
        sText.Append(cArrow);
        for(USHORT i = 0; i < sText.Len(); i++)
        {
            String sDraw(sText.GetChar(i));
            long nHDiff = 0;
            long nCharWidth = GetTextWidth(sDraw);
            BOOL bHorizontal = 0 == aMove.Y();
            if(!bHorizontal)
            {
                nHDiff = (nAWidth - nCharWidth)/2;
                aPos.X() += nHDiff;
            }
            DrawText(aPos, sDraw);
            if(bHorizontal)
            {
                aPos.X() += aMove.X() < 0 ? - nCharWidth : nCharWidth;
            }
            else
            {
                aPos.X() -= nHDiff;
                aPos.Y() += aMove.Y();
            }
        }
        aFont.SetSize(aSaveSize);
        SetFont(aFont);

    }
    if ( bTable )
    {
        // Paint Table, if necessary center it
        SetLineColor( Color(COL_LIGHTGRAY) );

        long nW = aRect.GetWidth(), nH = aRect.GetHeight();
        long nTW = CELL_WIDTH * 3, nTH = CELL_HEIGHT * 3;
        long _nLeft = bHorz ? aRect.Left() + ((nW - nTW) / 2) : aRect.Left();
        long _nTop = bVert ? aRect.Top() + ((nH - nTH) / 2) : aRect.Top();
        Rectangle aCellRect( Point( _nLeft, _nTop ), Size( CELL_WIDTH, CELL_HEIGHT ) );

        for ( USHORT i = 0; i < 3; ++i )
        {
            aCellRect.Left() = _nLeft;
            aCellRect.Right() = _nLeft + CELL_WIDTH;
            if ( i > 0 )
                aCellRect.Move( 0, CELL_HEIGHT );

            for ( USHORT j = 0; j < 3; ++j )
            {
                if ( j > 0 )
                    aCellRect.Move( CELL_WIDTH, 0 );
                DrawRect( aCellRect );
            }
        }
    }
}

// -----------------------------------------------------------------------

void SvxPageWindow::SetBorder( const SvxBoxItem& rNew )
{
    delete pImpl->pBorder;
    pImpl->pBorder = new SvxBoxItem( rNew );
}

// -----------------------------------------------------------------------

void SvxPageWindow::SetBitmap( Bitmap* pBmp )
{
    if ( pBmp )
    {
        pImpl->aBitmap = *pBmp;
        pImpl->bBitmap = TRUE;
    }
    else
        pImpl->bBitmap = FALSE;
}

// -----------------------------------------------------------------------

void SvxPageWindow::SetHdBorder( const SvxBoxItem& rNew )
{
    delete pHdBorder;
    pHdBorder = new SvxBoxItem( rNew );
}
// -----------------------------------------------------------------------

void SvxPageWindow::SetFtBorder( const SvxBoxItem& rNew )
{
    delete pFtBorder;
    pFtBorder = new SvxBoxItem( rNew );
}

void  SvxPageWindow::EnableFrameDirection(sal_Bool bEnable)
{
    pImpl->EnableFrameDirection(bEnable);
}

void  SvxPageWindow::SetFrameDirection(sal_Int32 nFrameDirection)
{
    pImpl->SetFrameDirection(nFrameDirection);
}

void SvxPageWindow::ResetBackground()
{
    pImpl->bResetBackground = sal_True;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
