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


#include <tools/poly.hxx>

#include <vcl/salbtype.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/region.hxx>
#include <vcl/bmpacc.hxx>

#include <bmpfast.hxx>

void BitmapWriteAccess::SetLineColor( const Color& rColor )
{
    delete mpLineColor;

    if( rColor.GetTransparency() == 255 )
        mpLineColor = NULL;
    else
        mpLineColor = ( HasPalette() ? new BitmapColor(  (sal_uInt8) GetBestPaletteIndex( rColor ) ) : new BitmapColor( rColor ) );
}

void BitmapWriteAccess::SetFillColor( const Color& rColor )
{
    delete mpFillColor;

    if( rColor.GetTransparency() == 255 )
        mpFillColor = NULL;
    else
        mpFillColor = ( HasPalette() ? new BitmapColor(  (sal_uInt8) GetBestPaletteIndex( rColor ) ) : new BitmapColor( rColor ) );
}

void BitmapWriteAccess::Erase( const Color& rColor )
{
    // convert the color format from RGB to palette index if needed
    // TODO: provide and use Erase( BitmapColor& method)
    BitmapColor aColor = rColor;
    if( HasPalette() )
        aColor = BitmapColor( (sal_uInt8)GetBestPaletteIndex( rColor) );
    // try fast bitmap method first
    if( ImplFastEraseBitmap( *mpBuffer, aColor ) )
        return;

    // use the canonical method to clear the bitmap
    BitmapColor*    pOldFillColor = mpFillColor ? new BitmapColor( *mpFillColor ) : NULL;
    const Point     aPoint;
    const Rectangle aRect( aPoint, maBitmap.GetSizePixel() );

    SetFillColor( rColor );
    FillRect( aRect );
    delete mpFillColor;
    mpFillColor = pOldFillColor;
}

void BitmapWriteAccess::DrawLine( const Point& rStart, const Point& rEnd )
{
    if( mpLineColor )
    {
        const BitmapColor&  rLineColor = *mpLineColor;
        long                nX, nY;

        if ( rStart.X() == rEnd.X() )
        {
            // Vertical Line
            const long nEndY = rEnd.Y();

            nX = rStart.X();
            nY = rStart.Y();

            if ( nEndY > nY )
            {
                for (; nY <= nEndY; nY++ )
                    SetPixel( nY, nX, rLineColor );
            }
            else
            {
                for (; nY >= nEndY; nY-- )
                    SetPixel( nY, nX, rLineColor );
            }
        }
        else if ( rStart.Y() == rEnd.Y() )
        {
            // Horizontal Line
            const long nEndX = rEnd.X();

            nX = rStart.X();
            nY = rStart.Y();

            if ( nEndX > nX )
            {
                for (; nX <= nEndX; nX++ )
                    SetPixel( nY, nX, rLineColor );
            }
            else
            {
                for (; nX >= nEndX; nX-- )
                    SetPixel( nY, nX, rLineColor );
            }
        }
        else
        {
            const long  nDX = labs( rEnd.X() - rStart.X() );
            const long  nDY = labs( rEnd.Y() - rStart.Y() );
            long        nX1;
            long        nY1;
            long        nX2;
            long        nY2;

            if ( nDX >= nDY )
            {
                if ( rStart.X() < rEnd.X() )
                {
                    nX1 = rStart.X();
                    nY1 = rStart.Y();
                    nX2 = rEnd.X();
                    nY2 = rEnd.Y();
                }
                else
                {
                    nX1 = rEnd.X();
                    nY1 = rEnd.Y();
                    nX2 = rStart.X();
                    nY2 = rStart.Y();
                }

                const long  nDYX = ( nDY - nDX ) << 1;
                const long  nDY2 = nDY << 1;
                long        nD = nDY2 - nDX;
                bool        bPos = nY1 < nY2;

                for ( nX = nX1, nY = nY1; nX <= nX2; nX++ )
                {
                    SetPixel( nY, nX, rLineColor );

                    if ( nD < 0 )
                        nD += nDY2;
                    else
                    {
                        nD += nDYX;

                        if ( bPos )
                            nY++;
                        else
                            nY--;
                    }
                }
            }
            else
            {
                if ( rStart.Y() < rEnd.Y() )
                {
                    nX1 = rStart.X();
                    nY1 = rStart.Y();
                    nX2 = rEnd.X();
                    nY2 = rEnd.Y();
                }
                else
                {
                    nX1 = rEnd.X();
                    nY1 = rEnd.Y();
                    nX2 = rStart.X();
                    nY2 = rStart.Y();
                }

                const long  nDYX = ( nDX - nDY ) << 1;
                const long  nDY2 = nDX << 1;
                long        nD = nDY2 - nDY;
                bool        bPos = nX1 < nX2;

                for ( nX = nX1, nY = nY1; nY <= nY2; nY++ )
                {
                    SetPixel( nY, nX, rLineColor );

                    if ( nD < 0 )
                        nD += nDY2;
                    else
                    {
                        nD += nDYX;

                        if ( bPos )
                            nX++;
                        else
                            nX--;
                    }
                }
            }
        }
    }
}

void BitmapWriteAccess::FillRect( const Rectangle& rRect )
{
    if( mpFillColor )
    {
        const BitmapColor&  rFillColor = *mpFillColor;
        Point               aPoint;
        Rectangle           aRect( aPoint, maBitmap.GetSizePixel() );

        aRect.Intersection( rRect );

        if( !aRect.IsEmpty() )
        {
            const long  nStartX = rRect.Left();
            const long  nStartY = rRect.Top();
            const long  nEndX = rRect.Right();
            const long  nEndY = rRect.Bottom();

            for( long nY = nStartY; nY <= nEndY; nY++ )
                for( long nX = nStartX; nX <= nEndX; nX++ )
                    SetPixel( nY, nX, rFillColor );
        }
    }
}

void BitmapWriteAccess::DrawRect( const Rectangle& rRect )
{
    if( mpFillColor )
        FillRect( rRect );

    if( mpLineColor && ( !mpFillColor || ( *mpFillColor != *mpLineColor ) ) )
    {
        DrawLine( rRect.TopLeft(), rRect.TopRight() );
        DrawLine( rRect.TopRight(), rRect.BottomRight() );
        DrawLine( rRect.BottomRight(), rRect.BottomLeft() );
        DrawLine( rRect.BottomLeft(), rRect.TopLeft() );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
