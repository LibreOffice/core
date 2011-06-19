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
#include <svx/rectenum.hxx>
#include <vcl/svapp.hxx>
#include <vcl/outdev.hxx>
#include <vcl/bitmap.hxx>

///////////////////////////////////////////////////////////////////////////////

void ImpCalcBmpFillSizes( Size&            rStartOffset,
                          Size&            rBmpOutputSize,
                          const Rectangle& rOutputRect,
                          const MapMode&   rOutputMapMode,
                          const Bitmap&    rFillBitmap,
                          const Size&      rBmpSize,
                          const Size&      rBmpPerCent,
                          const Size&      rBmpOffPerCent,
                          sal_Bool             bBmpLogSize,
                          sal_Bool             bBmpTile,
                          sal_Bool             bBmpStretch,
                          RECT_POINT       eBmpRectPoint )
{
    sal_Bool    bOriginalSize = sal_False, bScaleSize = sal_False;

    // Falls keine Groessen gegeben sind ( z.B. alte Dokumente )
    // berechnen wir uns die Groesse selber aus der Bitmap
    // ==> altes Verhalten;
    // wenn nur eine Groesse gegeben ist, wird die andere
    // Groesse angepasst berechnet
    if( bBmpLogSize )
    {
        if( !rBmpSize.Width() && !rBmpSize.Height() )
            bOriginalSize = sal_True;
        else if( !rBmpSize.Width() || !rBmpSize.Height() )
            bScaleSize = sal_True;
    }
    else
    {
        if( !rBmpPerCent.Width() && !rBmpPerCent.Height() )
            bOriginalSize = sal_True;
        else if( !rBmpPerCent.Width() || !rBmpPerCent.Height() )
            bScaleSize = sal_True;
    }

    // entweder Originalgroesse oder angepasste Groesse
    if( bOriginalSize || bScaleSize )
    {
        MapMode aBmpPrefMapMode( rFillBitmap.GetPrefMapMode() );
        Size    aBmpPrefSize( rFillBitmap.GetPrefSize() );

        // Falls keine gesetzt ist, nehmen wir Pixel
        if( !aBmpPrefSize.Width() || !aBmpPrefSize.Height() )
        {
            aBmpPrefSize = rFillBitmap.GetSizePixel();
            aBmpPrefMapMode = MAP_PIXEL;
        }

        if( bOriginalSize )
        {
            if( MAP_PIXEL == aBmpPrefMapMode.GetMapUnit() )
                rBmpOutputSize = Application::GetDefaultDevice()->PixelToLogic( aBmpPrefSize, rOutputMapMode );
            else
                rBmpOutputSize = OutputDevice::LogicToLogic( aBmpPrefSize, aBmpPrefMapMode, rOutputMapMode );
        }
        else
        {
            if( bBmpLogSize )
            {
                rBmpOutputSize = rBmpSize;

                if( !rBmpSize.Width() )
                    rBmpOutputSize.Width() = basegfx::fround( (double) rBmpSize.Height() * aBmpPrefSize.Width() / aBmpPrefSize.Height() );
                else
                    rBmpOutputSize.Height() = basegfx::fround( (double) rBmpSize.Width() * aBmpPrefSize.Height() / aBmpPrefSize.Width() );
            }
            else
            {
                if( !rBmpPerCent.Width() )
                {
                    rBmpOutputSize.Height() = basegfx::fround( (double) rOutputRect.GetHeight() * rBmpPerCent.Height() / 100. );
                    rBmpOutputSize.Width() = basegfx::fround( (double) rBmpOutputSize.Height() * aBmpPrefSize.Width() / aBmpPrefSize.Height() );
                }
                else
                {
                    rBmpOutputSize.Width() = basegfx::fround( (double) rOutputRect.GetWidth() * rBmpPerCent.Width() / 100. );
                    rBmpOutputSize.Height() = basegfx::fround( (double) rBmpOutputSize.Width() * aBmpPrefSize.Height() / aBmpPrefSize.Width() );
                }
            }
        }
    }
    // ansonsten koennen wir die Groesse leicht selber berechnen
    else
    {
        if( bBmpLogSize )
            rBmpOutputSize = rBmpSize;
        else
        {
            rBmpOutputSize.Width() = basegfx::fround( (double) rOutputRect.GetWidth() * rBmpPerCent.Width() / 100. );
            rBmpOutputSize.Height() = basegfx::fround( (double) rOutputRect.GetHeight() * rBmpPerCent.Height() / 100. );
        }
    }

    // nur bei Kachelung die anderen Positionen berechnen
    if( bBmpTile )
    {
        Point aStartPoint;

        // Grundposition der ersten Kachel berechen;
        // Diese Position wird spaeter zur Berechnung der absoluten
        // Startposition links oberhalb des Objektes benutzt
        switch( eBmpRectPoint )
        {
            case( RP_MT ):
            {
                aStartPoint.X() = rOutputRect.Left() + ( ( rOutputRect.GetWidth() - rBmpOutputSize.Width() ) >> 1 );
                aStartPoint.Y() = rOutputRect.Top();
            }
            break;

            case( RP_RT ):
            {
                aStartPoint.X() = rOutputRect.Right() - rBmpOutputSize.Width();
                aStartPoint.Y() = rOutputRect.Top();
            }
            break;

            case( RP_LM ):
            {
                aStartPoint.X() = rOutputRect.Left();
                aStartPoint.Y() = rOutputRect.Top() + ( ( rOutputRect.GetHeight() - rBmpOutputSize.Height() ) >> 1  );
            }
            break;

            case( RP_MM ):
            {
                aStartPoint.X() = rOutputRect.Left() + ( ( rOutputRect.GetWidth() - rBmpOutputSize.Width() ) >> 1 );
                aStartPoint.Y() = rOutputRect.Top() + ( ( rOutputRect.GetHeight() - rBmpOutputSize.Height() ) >> 1 );
            }
            break;

            case( RP_RM ):
            {
                aStartPoint.X() = rOutputRect.Right() - rBmpOutputSize.Width();
                aStartPoint.Y() = rOutputRect.Top() + ( ( rOutputRect.GetHeight() - rBmpOutputSize.Height() ) >> 1 );
            }
            break;

            case( RP_LB ):
            {
                aStartPoint.X() = rOutputRect.Left();
                aStartPoint.Y() = rOutputRect.Bottom() - rBmpOutputSize.Height();
            }
            break;

            case( RP_MB ):
            {
                aStartPoint.X() = rOutputRect.Left() + ( ( rOutputRect.GetWidth() - rBmpOutputSize.Width() ) >> 1 );
                aStartPoint.Y() = rOutputRect.Bottom() - rBmpOutputSize.Height();
            }
            break;

            case( RP_RB ):
            {
                aStartPoint.X() = rOutputRect.Right() - rBmpOutputSize.Width();
                aStartPoint.Y() = rOutputRect.Bottom() - rBmpOutputSize.Height();
            }
            break;

            // default linke obere Ecke
            default:
                aStartPoint = rOutputRect.TopLeft();
            break;
        }

        // X- oder Y-Positionsoffset beruecksichtigen
        if( rBmpOffPerCent.Width() )
            aStartPoint.X() += ( rBmpOutputSize.Width() * rBmpOffPerCent.Width() / 100 );

        if( rBmpOffPerCent.Height() )
            aStartPoint.Y() += ( rBmpOutputSize.Height() * rBmpOffPerCent.Height() / 100 );

        // echten Startpunkt berechnen ( links oben )
        if( rBmpOutputSize.Width() && rBmpOutputSize.Height() )
        {
            const long nDiffX = aStartPoint.X() - rOutputRect.Left();
            const long nDiffY = aStartPoint.Y() - rOutputRect.Top();

            if ( nDiffX )
            {
                long nCount = nDiffX / rBmpOutputSize.Width() + 1;

                if ( rBmpOffPerCent.Height() && ( nCount & 1L ) )
                    nCount++;

                aStartPoint.X() -= ( nCount * rBmpOutputSize.Width() );
            }

            if ( nDiffY )
            {
                long nCount = nDiffY / rBmpOutputSize.Height() + 1;

                if ( rBmpOffPerCent.Width() && ( nCount & 1L ) )
                    nCount++;

                aStartPoint.Y() -= ( nCount * rBmpOutputSize.Height() );
            }
        }

        rStartOffset = Size( aStartPoint.X() - rOutputRect.Left(),
                             aStartPoint.Y() - rOutputRect.Top() );
    }
    else
    {
        if( bBmpStretch )
        {
            rStartOffset = Size(0, 0);
            rBmpOutputSize = rOutputRect.GetSize();
        }
        else
        {
            rStartOffset = Size( ( rOutputRect.GetWidth() - rBmpOutputSize.Width() ) >> 1,
                                 ( rOutputRect.GetHeight() - rBmpOutputSize.Height() ) >> 1 );
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
