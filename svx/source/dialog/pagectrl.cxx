/*************************************************************************
 *
 *  $RCSfile: pagectrl.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:10 $
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

// include ---------------------------------------------------------------

#pragma hdrstop

#define ITEMID_BOX          0

#ifndef _SV_BITMAP_HXX
#include <vcl/bitmap.hxx>
#endif

#include "pageitem.hxx"
#include "pagectrl.hxx"
#include "boxitem.hxx"

// struct PageWindow_Impl ------------------------------------------------

struct PageWindow_Impl
{
    SvxBoxItem*     pBorder;
    Bitmap          aBitmap;
    FASTBOOL        bBitmap;

    PageWindow_Impl() : pBorder(0), bBitmap(FALSE) {}
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
    aColor      ( COL_WHITE ),
    nHdLeft     ( 0 ),
    nHdRight    ( 0 ),
    nHdDist     ( 0 ),
    nHdHeight   ( 0 ),
    aHdColor    ( COL_WHITE ),
    pHdBorder   ( 0 ),
    nFtLeft     ( 0 ),
    nFtRight    ( 0 ),
    nFtDist     ( 0 ),
    nFtHeight   ( 0 ),
    aFtColor    ( COL_WHITE ),
    pFtBorder   ( 0 ),
    bFooter     ( FALSE ),
    bHeader     ( FALSE ),
    bTable      ( FALSE ),
    bHorz       ( FALSE ),
    bVert       ( FALSE ),
    eUsage      ( SVX_PAGE_ALL )

{
    pImpl = new PageWindow_Impl;

    // defaultmaessing in Twips rechnen
    SetMapMode( MapMode( MAP_TWIP ) );
    aWinSize = GetOutputSizePixel();
    aWinSize.Height() -= 4;
    aWinSize.Width() -= 4;

    aWinSize = PixelToLogic( aWinSize );

    aSolidLineColor = Color( COL_BLACK );
    aDotLineColor   = Color( COL_BLACK );
    aGrayLineColor  = Color( COL_GRAY );

    aNormalFillColor   = GetFillColor();
    aDisabledFillColor = Color( COL_GRAY );
    aGrayFillColor     = Color( COL_LIGHTGRAY );
}

// -----------------------------------------------------------------------

SvxPageWindow::~SvxPageWindow()
{
    delete pImpl;
    delete pHdBorder;
    delete pFtBorder;
}

// -----------------------------------------------------------------------

void __EXPORT SvxPageWindow::Paint( const Rectangle& rRect )
{
    Fraction aXScale( aWinSize.Width(), max( aSize.Width() * 2 + aSize.Width() / 8, 1 ) );
    Fraction aYScale( aWinSize.Height(), max( aSize.Height(), 1 ) );
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
        // alle Seiten gleich -> eine Seite malen
        if ( aSize.Width() > aSize.Height() )
        {
            // Querformat in gleicher Gr"osse zeichnen
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
            // Hochformat
            DrawPage( Point( ( aSz.Width() - aSize.Width() ) / 2, nYPos ), TRUE, TRUE );
    }
    else
    {
        // Linke und rechte Seite unterschiedlich -> ggf. zwei Seiten malen
        DrawPage( Point( 0, nYPos ), FALSE, (BOOL)( eUsage & SVX_PAGE_LEFT ) );
        DrawPage( Point( aSize.Width() + aSize.Width() / 8, nYPos ), TRUE,
                  (BOOL)( eUsage & SVX_PAGE_RIGHT ) );
    }
}

// -----------------------------------------------------------------------

void SvxPageWindow::DrawPage( const Point& rOrg, const BOOL bSecond, const BOOL bEnabled )
{
    // Schatten
    Size aTempSize = aSize;
    if ( aTempSize.Height() > aTempSize.Width() )
        // Beim Hochformat die H"ohe etwas verkleinern, damit der Schatten passt.
        aTempSize.Height() -= PixelToLogic( Size( 0, 2 ) ).Height();
    Point aShadowPt( rOrg );
    aShadowPt += PixelToLogic( Point( 2, 2 ) );
    SetLineColor( Color( COL_GRAY ) );
    SetFillColor( Color( COL_GRAY ) );
    DrawRect( Rectangle( aShadowPt, aTempSize ) );

    // Seite
    SetLineColor( Color( COL_BLACK ) );

    if ( !bEnabled )
    {
        SetFillColor( Color( COL_GRAY ) );
        DrawRect( Rectangle( rOrg, aTempSize ) );
        return;
    }

    SetFillColor( Color( COL_WHITE ) );
    DrawRect( Rectangle( rOrg, aTempSize ) );

    // Border Top Bottom Left Right
    Point aBegin( rOrg );
    Point aEnd( rOrg );

    long nL = nLeft;
    long nR = nRight;

    if ( eUsage == SVX_PAGE_MIRROR && !bSecond )
    {
        // f"ur gespiegelt drehen
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
        // ggf. Header anzeigen
        aHdRect.Left() += nHdLeft;
        aHdRect.Right() -= nHdRight;
        aHdRect.Bottom() = aRect.Top() + nHdHeight;
        aRect.Top() += nHdHeight + nHdDist;
        SetFillColor( aHdColor );
        DrawRect( aHdRect );
    }

    if ( bFooter )
    {
        // ggf. Footer anzeigen
        aFtRect.Left() += nFtLeft;
        aFtRect.Right() -= nFtRight;
        aFtRect.Top() = aRect.Bottom() - nFtHeight;
        aRect.Bottom() -= nFtHeight + nFtDist;
        SetFillColor( aFtColor );
        DrawRect( aFtRect );
    }

    // Body malen
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

    if ( bTable )
    {
        // Tabelle malen, ggf. zentrieren
        SetLineColor( Color(COL_LIGHTGRAY) );

        long nW = aRect.GetWidth(), nH = aRect.GetHeight();
        long nTW = CELL_WIDTH * 3, nTH = CELL_HEIGHT * 3;
        long nLeft = bHorz ? aRect.Left() + ((nW - nTW) / 2) : aRect.Left();
        long nTop = bVert ? aRect.Top() + ((nH - nTH) / 2) : aRect.Top();
        Rectangle aCellRect( Point( nLeft, nTop ), Size( CELL_WIDTH, CELL_HEIGHT ) );

        for ( USHORT i = 0; i < 3; ++i )
        {
            aCellRect.Left() = nLeft;
            aCellRect.Right() = nLeft + CELL_WIDTH;
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


