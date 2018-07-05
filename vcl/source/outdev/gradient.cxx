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

#include <memory>
#include <cassert>

#include <tools/poly.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/gradient.hxx>
#include <vcl/metaact.hxx>
#include <vcl/settings.hxx>
#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>
#include <vcl/window.hxx>

#include <salgdi.hxx>

#define GRADIENT_DEFAULT_STEPCOUNT  0

void OutputDevice::DrawGradient( const tools::Rectangle& rRect,
                                 const Gradient& rGradient )
{
    assert(!is_double_buffered_window());

    // Convert rectangle to a tools::PolyPolygon by first converting to a Polygon
    tools::Polygon aPolygon ( rRect );
    tools::PolyPolygon aPolyPoly ( aPolygon );

    DrawGradient ( aPolyPoly, rGradient );
}

void OutputDevice::DrawGradient( const tools::PolyPolygon& rPolyPoly,
                                 const Gradient& rGradient )
{
    assert(!is_double_buffered_window());

    if ( mnDrawMode & DrawModeFlags::NoGradient )
        return;     // nothing to draw!

    if ( mbInitClipRegion )
        InitClipRegion();

    if ( mbOutputClipped )
        return;

    if ( rPolyPoly.Count() && rPolyPoly[ 0 ].GetSize() )
    {
        if ( mnDrawMode & ( DrawModeFlags::BlackGradient | DrawModeFlags::WhiteGradient | DrawModeFlags::SettingsGradient) )
        {
            Color aColor = GetSingleColorGradientFill();

            Push( PushFlags::LINECOLOR | PushFlags::FILLCOLOR );
            SetLineColor( aColor );
            SetFillColor( aColor );
            DrawPolyPolygon( rPolyPoly );
            Pop();
            return;
        }

        Gradient aGradient( rGradient );

        if ( mnDrawMode & ( DrawModeFlags::GrayGradient | DrawModeFlags::GhostedGradient ) )
        {
            SetGrayscaleColors( aGradient );
        }

        DrawGradientToMetafile( rPolyPoly, rGradient );

        if( !IsDeviceOutputNecessary() || ImplIsRecordLayout() )
            return;

        // Clip and then draw the gradient
        if( !tools::Rectangle( PixelToLogic( Point() ), GetOutputSize() ).IsEmpty() )
        {
            const tools::Rectangle aBoundRect( rPolyPoly.GetBoundRect() );

            // convert rectangle to pixels
            tools::Rectangle aRect( ImplLogicToDevicePixel( aBoundRect ) );
            aRect.Justify();

            // do nothing if the rectangle is empty
            if ( !aRect.IsEmpty() )
            {
                tools::PolyPolygon aClixPolyPoly( ImplLogicToDevicePixel( rPolyPoly ) );
                bool bDrawn = false;

                if( !mpGraphics && !AcquireGraphics() )
                    return;

                // secure clip region
                Push( PushFlags::CLIPREGION );
                IntersectClipRegion( aBoundRect );

                if( mbInitClipRegion )
                    InitClipRegion();

                // try to draw gradient natively
                bDrawn = mpGraphics->DrawGradient( aClixPolyPoly, aGradient );

                if( !bDrawn && !mbOutputClipped )
                {
                    // draw gradients without border
                    if( mbLineColor || mbInitLineColor )
                    {
                        mpGraphics->SetLineColor();
                        mbInitLineColor = true;
                    }

                    mbInitFillColor = true;

                    // calculate step count if necessary
                    if ( !aGradient.GetSteps() )
                        aGradient.SetSteps( GRADIENT_DEFAULT_STEPCOUNT );

                    if ( rPolyPoly.IsRect() )
                    {
                        // because we draw with no border line, we have to expand gradient
                        // rect to avoid missing lines on the right and bottom edge
                        aRect.AdjustLeft( -1 );
                        aRect.AdjustTop( -1 );
                        aRect.AdjustRight( 1 );
                        aRect.AdjustBottom( 1 );
                    }

                    // if the clipping polypolygon is a rectangle, then it's the same size as the bounding of the
                    // polypolygon, so pass in a NULL for the clipping parameter
                    if( aGradient.GetStyle() == GradientStyle::Linear || rGradient.GetStyle() == GradientStyle::Axial )
                        DrawLinearGradient( aRect, aGradient, aClixPolyPoly.IsRect() ? nullptr : &aClixPolyPoly );
                    else
                        DrawComplexGradient( aRect, aGradient, aClixPolyPoly.IsRect() ? nullptr : &aClixPolyPoly );
                }

                Pop();
            }
        }
    }

    if( mpAlphaVDev )
        mpAlphaVDev->DrawPolyPolygon( rPolyPoly );
}

void OutputDevice::ClipAndDrawGradientMetafile ( const Gradient &rGradient, const tools::PolyPolygon &rPolyPoly )
{
    const tools::Rectangle aBoundRect( rPolyPoly.GetBoundRect() );
    const bool  bOldOutput = IsOutputEnabled();

    EnableOutput( false );
    Push( PushFlags::RASTEROP );
    SetRasterOp( RasterOp::Xor );
    DrawGradient( aBoundRect, rGradient );
    SetFillColor( COL_BLACK );
    SetRasterOp( RasterOp::N0 );
    DrawPolyPolygon( rPolyPoly );
    SetRasterOp( RasterOp::Xor );
    DrawGradient( aBoundRect, rGradient );
    Pop();
    EnableOutput( bOldOutput );
}

void OutputDevice::DrawGradientToMetafile ( const tools::PolyPolygon& rPolyPoly,
                                            const Gradient& rGradient )
{
    assert(!is_double_buffered_window());

    if ( !mpMetaFile )
        return;

    if ( rPolyPoly.Count() && rPolyPoly[ 0 ].GetSize() )
    {
        Gradient aGradient( rGradient );

        if ( mnDrawMode & ( DrawModeFlags::GrayGradient | DrawModeFlags::GhostedGradient ) )
        {
            SetGrayscaleColors( aGradient );
        }

        const tools::Rectangle aBoundRect( rPolyPoly.GetBoundRect() );

        if ( rPolyPoly.IsRect() )
        {
            mpMetaFile->AddAction( new MetaGradientAction( aBoundRect, aGradient ) );
        }
        else
        {
            mpMetaFile->AddAction( new MetaCommentAction( "XGRAD_SEQ_BEGIN" ) );
            mpMetaFile->AddAction( new MetaGradientExAction( rPolyPoly, rGradient ) );

            ClipAndDrawGradientMetafile ( rGradient, rPolyPoly );

            mpMetaFile->AddAction( new MetaCommentAction( "XGRAD_SEQ_END" ) );
        }

        if( !IsDeviceOutputNecessary() || ImplIsRecordLayout() )
            return;

        // Clip and then draw the gradient
        if( !tools::Rectangle( PixelToLogic( Point() ), GetOutputSize() ).IsEmpty() )
        {
            // convert rectangle to pixels
            tools::Rectangle aRect( ImplLogicToDevicePixel( aBoundRect ) );
            aRect.Justify();

            // do nothing if the rectangle is empty
            if ( !aRect.IsEmpty() )
            {
                if( !mbOutputClipped )
                {
                    // calculate step count if necessary
                    if ( !aGradient.GetSteps() )
                        aGradient.SetSteps( GRADIENT_DEFAULT_STEPCOUNT );

                    if ( rPolyPoly.IsRect() )
                    {
                        // because we draw with no border line, we have to expand gradient
                        // rect to avoid missing lines on the right and bottom edge
                        aRect.AdjustLeft( -1 );
                        aRect.AdjustTop( -1 );
                        aRect.AdjustRight( 1 );
                        aRect.AdjustBottom( 1 );
                    }

                    // if the clipping polypolygon is a rectangle, then it's the same size as the bounding of the
                    // polypolygon, so pass in a NULL for the clipping parameter
                    if( aGradient.GetStyle() == GradientStyle::Linear || rGradient.GetStyle() == GradientStyle::Axial )
                        DrawLinearGradientToMetafile( aRect, aGradient );
                    else
                        DrawComplexGradientToMetafile( aRect, aGradient );
                }
            }
        }
    }
}

namespace
{
    inline sal_uInt8 GetGradientColorValue( long nValue )
    {
        if ( nValue < 0 )
            return 0;
        else if ( nValue > 0xFF )
            return 0xFF;
        else
            return static_cast<sal_uInt8>(nValue);
    }
}

void OutputDevice::DrawLinearGradient( const tools::Rectangle& rRect,
                                       const Gradient& rGradient,
                                       const tools::PolyPolygon* pClixPolyPoly )
{
    assert(!is_double_buffered_window());

    // get BoundRect of rotated rectangle
    tools::Rectangle aRect;
    Point     aCenter;
    sal_uInt16    nAngle = rGradient.GetAngle() % 3600;

    rGradient.GetBoundRect( rRect, aRect, aCenter );

    bool bLinear = (rGradient.GetStyle() == GradientStyle::Linear);
    double fBorder = rGradient.GetBorder() * aRect.GetHeight() / 100.0;
    if ( !bLinear )
    {
        fBorder /= 2.0;
    }
    tools::Rectangle aMirrorRect = aRect; // used in style axial
    aMirrorRect.SetTop( ( aRect.Top() + aRect.Bottom() ) / 2 );
    if ( !bLinear )
    {
        aRect.SetBottom( aMirrorRect.Top() );
    }

    // colour-intensities of start- and finish; change if needed
    long    nFactor;
    Color   aStartCol   = rGradient.GetStartColor();
    Color   aEndCol     = rGradient.GetEndColor();
    long    nStartRed   = aStartCol.GetRed();
    long    nStartGreen = aStartCol.GetGreen();
    long    nStartBlue  = aStartCol.GetBlue();
    long    nEndRed     = aEndCol.GetRed();
    long    nEndGreen   = aEndCol.GetGreen();
    long    nEndBlue    = aEndCol.GetBlue();
            nFactor     = rGradient.GetStartIntensity();
            nStartRed   = (nStartRed   * nFactor) / 100;
            nStartGreen = (nStartGreen * nFactor) / 100;
            nStartBlue  = (nStartBlue  * nFactor) / 100;
            nFactor     = rGradient.GetEndIntensity();
            nEndRed     = (nEndRed   * nFactor) / 100;
            nEndGreen   = (nEndGreen * nFactor) / 100;
            nEndBlue    = (nEndBlue  * nFactor) / 100;

    // gradient style axial has exchanged start and end colors
    if ( !bLinear)
    {
        long nTempColor = nStartRed;
        nStartRed = nEndRed;
        nEndRed = nTempColor;
        nTempColor = nStartGreen;
        nStartGreen = nEndGreen;
        nEndGreen = nTempColor;
        nTempColor = nStartBlue;
        nStartBlue = nEndBlue;
        nEndBlue = nTempColor;
    }

    sal_uInt8   nRed;
    sal_uInt8   nGreen;
    sal_uInt8   nBlue;

    // Create border
    tools::Rectangle aBorderRect = aRect;
    tools::Polygon aPoly( 4 );
    if (fBorder > 0.0)
    {
        nRed        = static_cast<sal_uInt8>(nStartRed);
        nGreen      = static_cast<sal_uInt8>(nStartGreen);
        nBlue       = static_cast<sal_uInt8>(nStartBlue);

        mpGraphics->SetFillColor( Color( nRed, nGreen, nBlue ) );

        aBorderRect.SetBottom( static_cast<long>( aBorderRect.Top() + fBorder ) );
        aRect.SetTop( aBorderRect.Bottom() );
        aPoly[0] = aBorderRect.TopLeft();
        aPoly[1] = aBorderRect.TopRight();
        aPoly[2] = aBorderRect.BottomRight();
        aPoly[3] = aBorderRect.BottomLeft();
        aPoly.Rotate( aCenter, nAngle );

        ImplDrawPolygon( aPoly, pClixPolyPoly );

        if ( !bLinear)
        {
            aBorderRect = aMirrorRect;
            aBorderRect.SetTop( static_cast<long>( aBorderRect.Bottom() - fBorder ) );
            aMirrorRect.SetBottom( aBorderRect.Top() );
            aPoly[0] = aBorderRect.TopLeft();
            aPoly[1] = aBorderRect.TopRight();
            aPoly[2] = aBorderRect.BottomRight();
            aPoly[3] = aBorderRect.BottomLeft();
            aPoly.Rotate( aCenter, nAngle );

            ImplDrawPolygon( aPoly, pClixPolyPoly );
        }
    }

    // calculate step count
    long    nStepCount  = GetGradientSteps( rGradient, aRect, false/*bMtf*/ );

    // minimal three steps and maximal as max color steps
    long   nAbsRedSteps   = std::abs( nEndRed   - nStartRed );
    long   nAbsGreenSteps = std::abs( nEndGreen - nStartGreen );
    long   nAbsBlueSteps  = std::abs( nEndBlue  - nStartBlue );
    long   nMaxColorSteps = std::max( nAbsRedSteps , nAbsGreenSteps );
    nMaxColorSteps = std::max( nMaxColorSteps, nAbsBlueSteps );
    long nSteps = std::min( nStepCount, nMaxColorSteps );
    if ( nSteps < 3)
    {
        nSteps = 3;
    }

    double fScanInc = static_cast<double>(aRect.GetHeight()) / static_cast<double>(nSteps);
    double fGradientLine = static_cast<double>(aRect.Top());
    double fMirrorGradientLine = static_cast<double>(aMirrorRect.Bottom());

    const double fStepsMinus1 = static_cast<double>(nSteps) - 1.0;
    if ( !bLinear)
    {
        nSteps -= 1; // draw middle polygons as one polygon after loop to avoid gap
    }
    for ( long i = 0; i < nSteps; i++ )
    {
        // linear interpolation of color
        const double fAlpha = static_cast<double>(i) / fStepsMinus1;
        double fTempColor = static_cast<double>(nStartRed) * (1.0-fAlpha) + static_cast<double>(nEndRed) * fAlpha;
        nRed = GetGradientColorValue(static_cast<long>(fTempColor));
        fTempColor = static_cast<double>(nStartGreen) * (1.0-fAlpha) + static_cast<double>(nEndGreen) * fAlpha;
        nGreen = GetGradientColorValue(static_cast<long>(fTempColor));
        fTempColor = static_cast<double>(nStartBlue) * (1.0-fAlpha) + static_cast<double>(nEndBlue) * fAlpha;
        nBlue = GetGradientColorValue(static_cast<long>(fTempColor));

        mpGraphics->SetFillColor( Color( nRed, nGreen, nBlue ) );

        // Polygon for this color step
        aRect.SetTop( static_cast<long>( fGradientLine + static_cast<double>(i) * fScanInc ) );
        aRect.SetBottom( static_cast<long>( fGradientLine + ( static_cast<double>(i) + 1.0 ) * fScanInc ) );
        aPoly[0] = aRect.TopLeft();
        aPoly[1] = aRect.TopRight();
        aPoly[2] = aRect.BottomRight();
        aPoly[3] = aRect.BottomLeft();
        aPoly.Rotate( aCenter, nAngle );

        ImplDrawPolygon( aPoly, pClixPolyPoly );

        if ( !bLinear )
        {
            aMirrorRect.SetBottom( static_cast<long>( fMirrorGradientLine - static_cast<double>(i) * fScanInc ) );
            aMirrorRect.SetTop( static_cast<long>( fMirrorGradientLine - (static_cast<double>(i) + 1.0)* fScanInc ) );
            aPoly[0] = aMirrorRect.TopLeft();
            aPoly[1] = aMirrorRect.TopRight();
            aPoly[2] = aMirrorRect.BottomRight();
            aPoly[3] = aMirrorRect.BottomLeft();
            aPoly.Rotate( aCenter, nAngle );

            ImplDrawPolygon( aPoly, pClixPolyPoly );
        }
    }
    if ( bLinear)
        return;

    // draw middle polygon with end color
    nRed = GetGradientColorValue(nEndRed);
    nGreen = GetGradientColorValue(nEndGreen);
    nBlue = GetGradientColorValue(nEndBlue);

    mpGraphics->SetFillColor( Color( nRed, nGreen, nBlue ) );

    aRect.SetTop( static_cast<long>( fGradientLine + static_cast<double>(nSteps) * fScanInc ) );
    aRect.SetBottom( static_cast<long>( fMirrorGradientLine - static_cast<double>(nSteps) * fScanInc ) );
    aPoly[0] = aRect.TopLeft();
    aPoly[1] = aRect.TopRight();
    aPoly[2] = aRect.BottomRight();
    aPoly[3] = aRect.BottomLeft();
    aPoly.Rotate( aCenter, nAngle );

    ImplDrawPolygon( aPoly, pClixPolyPoly );

}

bool OutputDevice::is_double_buffered_window() const
{
    const vcl::Window *pWindow = dynamic_cast<const vcl::Window*>(this);
    return pWindow && pWindow->SupportsDoubleBuffering();
}

void OutputDevice::DrawComplexGradient( const tools::Rectangle& rRect,
                                        const Gradient& rGradient,
                                        const tools::PolyPolygon* pClixPolyPoly )
{
    assert(!is_double_buffered_window());

    // Determine if we output via Polygon or PolyPolygon
    // For all rasteroperations other than Overpaint always use PolyPolygon,
    // as we will get wrong results if we output multiple times on top of each other.
    // Also for printers always use PolyPolygon, as not all printers
    // can print polygons on top of each other.

    std::unique_ptr<tools::PolyPolygon> xPolyPoly;
    tools::Rectangle       aRect;
    Point           aCenter;
    Color           aStartCol( rGradient.GetStartColor() );
    Color           aEndCol( rGradient.GetEndColor() );
    long            nStartRed = ( static_cast<long>(aStartCol.GetRed()) * rGradient.GetStartIntensity() ) / 100;
    long            nStartGreen = ( static_cast<long>(aStartCol.GetGreen()) * rGradient.GetStartIntensity() ) / 100;
    long            nStartBlue = ( static_cast<long>(aStartCol.GetBlue()) * rGradient.GetStartIntensity() ) / 100;
    long            nEndRed = ( static_cast<long>(aEndCol.GetRed()) * rGradient.GetEndIntensity() ) / 100;
    long            nEndGreen = ( static_cast<long>(aEndCol.GetGreen()) * rGradient.GetEndIntensity() ) / 100;
    long            nEndBlue = ( static_cast<long>(aEndCol.GetBlue()) * rGradient.GetEndIntensity() ) / 100;
    long            nRedSteps = nEndRed - nStartRed;
    long            nGreenSteps = nEndGreen - nStartGreen;
    long            nBlueSteps = nEndBlue   - nStartBlue;
    sal_uInt16      nAngle = rGradient.GetAngle() % 3600;

    rGradient.GetBoundRect( rRect, aRect, aCenter );

    if ( UsePolyPolygonForComplexGradient() )
        xPolyPoly.reset(new tools::PolyPolygon( 2 ));

    long nStepCount = GetGradientSteps( rGradient, rRect, false/*bMtf*/, true/*bComplex*/ );

    // at least three steps and at most the number of colour differences
    long nSteps = std::max( nStepCount, 2L );
    long nCalcSteps  = std::abs( nRedSteps );
    long nTempSteps = std::abs( nGreenSteps );
    if ( nTempSteps > nCalcSteps )
        nCalcSteps = nTempSteps;
    nTempSteps = std::abs( nBlueSteps );
    if ( nTempSteps > nCalcSteps )
        nCalcSteps = nTempSteps;
    if ( nCalcSteps < nSteps )
        nSteps = nCalcSteps;
    if ( !nSteps )
        nSteps = 1;

    // determine output limits and stepsizes for all directions
    tools::Polygon aPoly;
    double  fScanLeft = aRect.Left();
    double  fScanTop = aRect.Top();
    double  fScanRight = aRect.Right();
    double  fScanBottom = aRect.Bottom();
    double fScanIncX = static_cast<double>(aRect.GetWidth()) / static_cast<double>(nSteps) * 0.5;
    double fScanIncY = static_cast<double>(aRect.GetHeight()) / static_cast<double>(nSteps) * 0.5;

    // all gradients are rendered as nested rectangles which shrink
    // equally in each dimension - except for 'square' gradients
    // which shrink to a central vertex but are not per-se square.
    if( rGradient.GetStyle() != GradientStyle::Square )
    {
        fScanIncY = std::min( fScanIncY, fScanIncX );
        fScanIncX = fScanIncY;
    }
    sal_uInt8   nRed = static_cast<sal_uInt8>(nStartRed), nGreen = static_cast<sal_uInt8>(nStartGreen), nBlue = static_cast<sal_uInt8>(nStartBlue);
    bool    bPaintLastPolygon( false ); // #107349# Paint last polygon only if loop has generated any output

    mpGraphics->SetFillColor( Color( nRed, nGreen, nBlue ) );

    if( xPolyPoly )
    {
        xPolyPoly->Insert( aPoly = rRect );
        xPolyPoly->Insert( aPoly );
    }
    else
    {
        // extend rect, to avoid missing bounding line
        tools::Rectangle aExtRect( rRect );

        aExtRect.AdjustLeft( -1 );
        aExtRect.AdjustTop( -1 );
        aExtRect.AdjustRight(1 );
        aExtRect.AdjustBottom(1 );

        ImplDrawPolygon( aPoly = aExtRect, pClixPolyPoly );
    }

    // loop to output Polygone/PolyPolygone sequentially
    for( long i = 1; i < nSteps; i++ )
    {
        // calculate new Polygon
        aRect.SetLeft( static_cast<long>( fScanLeft += fScanIncX ) );
        aRect.SetTop( static_cast<long>( fScanTop += fScanIncY ) );
        aRect.SetRight( static_cast<long>( fScanRight -= fScanIncX ) );
        aRect.SetBottom( static_cast<long>( fScanBottom -= fScanIncY ) );

        if( ( aRect.GetWidth() < 2 ) || ( aRect.GetHeight() < 2 ) )
            break;

        if( rGradient.GetStyle() == GradientStyle::Radial || rGradient.GetStyle() == GradientStyle::Elliptical )
            aPoly = tools::Polygon( aRect.Center(), aRect.GetWidth() >> 1, aRect.GetHeight() >> 1 );
        else
            aPoly = tools::Polygon( aRect );

        aPoly.Rotate( aCenter, nAngle );

        // adapt colour accordingly
        const long nStepIndex = ( xPolyPoly ? i : ( i + 1 ) );
        nRed = GetGradientColorValue( nStartRed + ( ( nRedSteps * nStepIndex ) / nSteps ) );
        nGreen = GetGradientColorValue( nStartGreen + ( ( nGreenSteps * nStepIndex ) / nSteps ) );
        nBlue = GetGradientColorValue( nStartBlue + ( ( nBlueSteps * nStepIndex ) / nSteps ) );

        // either slow tools::PolyPolygon output or fast Polygon-Painting
        if( xPolyPoly )
        {
            bPaintLastPolygon = true; // #107349# Paint last polygon only if loop has generated any output

            xPolyPoly->Replace( xPolyPoly->GetObject( 1 ), 0 );
            xPolyPoly->Replace( aPoly, 1 );

            ImplDrawPolyPolygon( *xPolyPoly, pClixPolyPoly );

            // #107349# Set fill color _after_ geometry painting:
            // xPolyPoly's geometry is the band from last iteration's
            // aPoly to current iteration's aPoly. The window outdev
            // path (see else below), on the other hand, paints the
            // full aPoly. Thus, here, we're painting the band before
            // the one painted in the window outdev path below. To get
            // matching colors, have to delay color setting here.
            mpGraphics->SetFillColor( Color( nRed, nGreen, nBlue ) );
        }
        else
        {
            // #107349# Set fill color _before_ geometry painting
            mpGraphics->SetFillColor( Color( nRed, nGreen, nBlue ) );

            ImplDrawPolygon( aPoly, pClixPolyPoly );
        }
    }

    // we should draw last inner Polygon if we output PolyPolygon
    if( xPolyPoly )
    {
        const tools::Polygon& rPoly = xPolyPoly->GetObject( 1 );

        if( !rPoly.GetBoundRect().IsEmpty() )
        {
            // #107349# Paint last polygon with end color only if loop
            // has generated output. Otherwise, the current
            // (i.e. start) color is taken, to generate _any_ output.
            if( bPaintLastPolygon )
            {
                nRed = GetGradientColorValue( nEndRed );
                nGreen = GetGradientColorValue( nEndGreen );
                nBlue = GetGradientColorValue( nEndBlue );
            }

            mpGraphics->SetFillColor( Color( nRed, nGreen, nBlue ) );
            ImplDrawPolygon( rPoly, pClixPolyPoly );
        }
    }
}

void OutputDevice::DrawLinearGradientToMetafile( const tools::Rectangle& rRect,
                                                 const Gradient& rGradient )
{
    assert(!is_double_buffered_window());

    // get BoundRect of rotated rectangle
    tools::Rectangle aRect;
    Point     aCenter;
    sal_uInt16    nAngle = rGradient.GetAngle() % 3600;

    rGradient.GetBoundRect( rRect, aRect, aCenter );

    bool bLinear = (rGradient.GetStyle() == GradientStyle::Linear);
    double fBorder = rGradient.GetBorder() * aRect.GetHeight() / 100.0;
    if ( !bLinear )
    {
        fBorder /= 2.0;
    }
    tools::Rectangle aMirrorRect = aRect; // used in style axial
    aMirrorRect.SetTop( ( aRect.Top() + aRect.Bottom() ) / 2 );
    if ( !bLinear )
    {
        aRect.SetBottom( aMirrorRect.Top() );
    }

    // colour-intensities of start- and finish; change if needed
    long    nFactor;
    Color   aStartCol   = rGradient.GetStartColor();
    Color   aEndCol     = rGradient.GetEndColor();
    long    nStartRed   = aStartCol.GetRed();
    long    nStartGreen = aStartCol.GetGreen();
    long    nStartBlue  = aStartCol.GetBlue();
    long    nEndRed     = aEndCol.GetRed();
    long    nEndGreen   = aEndCol.GetGreen();
    long    nEndBlue    = aEndCol.GetBlue();
            nFactor     = rGradient.GetStartIntensity();
            nStartRed   = (nStartRed   * nFactor) / 100;
            nStartGreen = (nStartGreen * nFactor) / 100;
            nStartBlue  = (nStartBlue  * nFactor) / 100;
            nFactor     = rGradient.GetEndIntensity();
            nEndRed     = (nEndRed   * nFactor) / 100;
            nEndGreen   = (nEndGreen * nFactor) / 100;
            nEndBlue    = (nEndBlue  * nFactor) / 100;

    // gradient style axial has exchanged start and end colors
    if ( !bLinear)
    {
        long nTempColor = nStartRed;
        nStartRed = nEndRed;
        nEndRed = nTempColor;
        nTempColor = nStartGreen;
        nStartGreen = nEndGreen;
        nEndGreen = nTempColor;
        nTempColor = nStartBlue;
        nStartBlue = nEndBlue;
        nEndBlue = nTempColor;
    }

    sal_uInt8   nRed;
    sal_uInt8   nGreen;
    sal_uInt8   nBlue;

    // Create border
    tools::Rectangle aBorderRect = aRect;
    tools::Polygon aPoly( 4 );
    if (fBorder > 0.0)
    {
        nRed        = static_cast<sal_uInt8>(nStartRed);
        nGreen      = static_cast<sal_uInt8>(nStartGreen);
        nBlue       = static_cast<sal_uInt8>(nStartBlue);

        mpMetaFile->AddAction( new MetaFillColorAction( Color( nRed, nGreen, nBlue ), true ) );

        aBorderRect.SetBottom( static_cast<long>( aBorderRect.Top() + fBorder ) );
        aRect.SetTop( aBorderRect.Bottom() );
        aPoly[0] = aBorderRect.TopLeft();
        aPoly[1] = aBorderRect.TopRight();
        aPoly[2] = aBorderRect.BottomRight();
        aPoly[3] = aBorderRect.BottomLeft();
        aPoly.Rotate( aCenter, nAngle );

        mpMetaFile->AddAction( new MetaPolygonAction( aPoly ) );

        if ( !bLinear)
        {
            aBorderRect = aMirrorRect;
            aBorderRect.SetTop( static_cast<long>( aBorderRect.Bottom() - fBorder ) );
            aMirrorRect.SetBottom( aBorderRect.Top() );
            aPoly[0] = aBorderRect.TopLeft();
            aPoly[1] = aBorderRect.TopRight();
            aPoly[2] = aBorderRect.BottomRight();
            aPoly[3] = aBorderRect.BottomLeft();
            aPoly.Rotate( aCenter, nAngle );

            mpMetaFile->AddAction( new MetaPolygonAction( aPoly ) );
        }
    }

    long    nStepCount  = GetGradientSteps( rGradient, aRect, true/*bMtf*/ );

    // minimal three steps and maximal as max color steps
    long   nAbsRedSteps   = std::abs( nEndRed   - nStartRed );
    long   nAbsGreenSteps = std::abs( nEndGreen - nStartGreen );
    long   nAbsBlueSteps  = std::abs( nEndBlue  - nStartBlue );
    long   nMaxColorSteps = std::max( nAbsRedSteps , nAbsGreenSteps );
    nMaxColorSteps = std::max( nMaxColorSteps, nAbsBlueSteps );
    long nSteps = std::min( nStepCount, nMaxColorSteps );
    if ( nSteps < 3)
    {
        nSteps = 3;
    }

    double fScanInc = static_cast<double>(aRect.GetHeight()) / static_cast<double>(nSteps);
    double fGradientLine = static_cast<double>(aRect.Top());
    double fMirrorGradientLine = static_cast<double>(aMirrorRect.Bottom());

    const double fStepsMinus1 = static_cast<double>(nSteps) - 1.0;
    if ( !bLinear)
    {
        nSteps -= 1; // draw middle polygons as one polygon after loop to avoid gap
    }
    for ( long i = 0; i < nSteps; i++ )
    {
        // linear interpolation of color
        double fAlpha = static_cast<double>(i) / fStepsMinus1;
        double fTempColor = static_cast<double>(nStartRed) * (1.0-fAlpha) + static_cast<double>(nEndRed) * fAlpha;
        nRed = GetGradientColorValue(static_cast<long>(fTempColor));
        fTempColor = static_cast<double>(nStartGreen) * (1.0-fAlpha) + static_cast<double>(nEndGreen) * fAlpha;
        nGreen = GetGradientColorValue(static_cast<long>(fTempColor));
        fTempColor = static_cast<double>(nStartBlue) * (1.0-fAlpha) + static_cast<double>(nEndBlue) * fAlpha;
        nBlue = GetGradientColorValue(static_cast<long>(fTempColor));

        mpMetaFile->AddAction( new MetaFillColorAction( Color( nRed, nGreen, nBlue ), true ) );

        // Polygon for this color step
        aRect.SetTop( static_cast<long>( fGradientLine + static_cast<double>(i) * fScanInc ) );
        aRect.SetBottom( static_cast<long>( fGradientLine + ( static_cast<double>(i) + 1.0 ) * fScanInc ) );
        aPoly[0] = aRect.TopLeft();
        aPoly[1] = aRect.TopRight();
        aPoly[2] = aRect.BottomRight();
        aPoly[3] = aRect.BottomLeft();
        aPoly.Rotate( aCenter, nAngle );

        mpMetaFile->AddAction( new MetaPolygonAction( aPoly ) );

        if ( !bLinear )
        {
            aMirrorRect.SetBottom( static_cast<long>( fMirrorGradientLine - static_cast<double>(i) * fScanInc ) );
            aMirrorRect.SetTop( static_cast<long>( fMirrorGradientLine - (static_cast<double>(i) + 1.0)* fScanInc ) );
            aPoly[0] = aMirrorRect.TopLeft();
            aPoly[1] = aMirrorRect.TopRight();
            aPoly[2] = aMirrorRect.BottomRight();
            aPoly[3] = aMirrorRect.BottomLeft();
            aPoly.Rotate( aCenter, nAngle );

            mpMetaFile->AddAction( new MetaPolygonAction( aPoly ) );
        }
    }
    if ( bLinear)
        return;

    // draw middle polygon with end color
    nRed = GetGradientColorValue(nEndRed);
    nGreen = GetGradientColorValue(nEndGreen);
    nBlue = GetGradientColorValue(nEndBlue);

    mpMetaFile->AddAction( new MetaFillColorAction( Color( nRed, nGreen, nBlue ), true ) );

    aRect.SetTop( static_cast<long>( fGradientLine + static_cast<double>(nSteps) * fScanInc ) );
    aRect.SetBottom( static_cast<long>( fMirrorGradientLine - static_cast<double>(nSteps) * fScanInc ) );
    aPoly[0] = aRect.TopLeft();
    aPoly[1] = aRect.TopRight();
    aPoly[2] = aRect.BottomRight();
    aPoly[3] = aRect.BottomLeft();
    aPoly.Rotate( aCenter, nAngle );

    mpMetaFile->AddAction( new MetaPolygonAction( aPoly ) );

}

void OutputDevice::DrawComplexGradientToMetafile( const tools::Rectangle& rRect,
                                                  const Gradient& rGradient )
{
    assert(!is_double_buffered_window());

    // Determine if we output via Polygon or PolyPolygon
    // For all rasteroperations other than Overpaint always use PolyPolygon,
    // as we will get wrong results if we output multiple times on top of each other.
    // Also for printers always use PolyPolygon, as not all printers
    // can print polygons on top of each other.

    std::unique_ptr<tools::PolyPolygon> xPolyPoly;
    tools::Rectangle       aRect;
    Point           aCenter;
    Color           aStartCol( rGradient.GetStartColor() );
    Color           aEndCol( rGradient.GetEndColor() );
    long            nStartRed = ( static_cast<long>(aStartCol.GetRed()) * rGradient.GetStartIntensity() ) / 100;
    long            nStartGreen = ( static_cast<long>(aStartCol.GetGreen()) * rGradient.GetStartIntensity() ) / 100;
    long            nStartBlue = ( static_cast<long>(aStartCol.GetBlue()) * rGradient.GetStartIntensity() ) / 100;
    long            nEndRed = ( static_cast<long>(aEndCol.GetRed()) * rGradient.GetEndIntensity() ) / 100;
    long            nEndGreen = ( static_cast<long>(aEndCol.GetGreen()) * rGradient.GetEndIntensity() ) / 100;
    long            nEndBlue = ( static_cast<long>(aEndCol.GetBlue()) * rGradient.GetEndIntensity() ) / 100;
    long            nRedSteps = nEndRed - nStartRed;
    long            nGreenSteps = nEndGreen - nStartGreen;
    long            nBlueSteps = nEndBlue   - nStartBlue;
    sal_uInt16      nAngle = rGradient.GetAngle() % 3600;

    rGradient.GetBoundRect( rRect, aRect, aCenter );

    xPolyPoly.reset(new tools::PolyPolygon( 2 ));

    // last parameter - true if complex gradient, false if linear
    long nStepCount = GetGradientSteps( rGradient, rRect, true, true );

    // at least three steps and at most the number of colour differences
    long nSteps = std::max( nStepCount, 2L );
    long nCalcSteps  = std::abs( nRedSteps );
    long nTempSteps = std::abs( nGreenSteps );
    if ( nTempSteps > nCalcSteps )
        nCalcSteps = nTempSteps;
    nTempSteps = std::abs( nBlueSteps );
    if ( nTempSteps > nCalcSteps )
        nCalcSteps = nTempSteps;
    if ( nCalcSteps < nSteps )
        nSteps = nCalcSteps;
    if ( !nSteps )
        nSteps = 1;

    // determine output limits and stepsizes for all directions
    tools::Polygon aPoly;
    double  fScanLeft = aRect.Left();
    double  fScanTop = aRect.Top();
    double  fScanRight = aRect.Right();
    double  fScanBottom = aRect.Bottom();
    double fScanIncX = static_cast<double>(aRect.GetWidth()) / static_cast<double>(nSteps) * 0.5;
    double fScanIncY = static_cast<double>(aRect.GetHeight()) / static_cast<double>(nSteps) * 0.5;

    // all gradients are rendered as nested rectangles which shrink
    // equally in each dimension - except for 'square' gradients
    // which shrink to a central vertex but are not per-se square.
    if( rGradient.GetStyle() != GradientStyle::Square )
    {
        fScanIncY = std::min( fScanIncY, fScanIncX );
        fScanIncX = fScanIncY;
    }
    sal_uInt8   nRed = static_cast<sal_uInt8>(nStartRed), nGreen = static_cast<sal_uInt8>(nStartGreen), nBlue = static_cast<sal_uInt8>(nStartBlue);
    bool    bPaintLastPolygon( false ); // #107349# Paint last polygon only if loop has generated any output

    mpMetaFile->AddAction( new MetaFillColorAction( Color( nRed, nGreen, nBlue ), true ) );

    xPolyPoly->Insert( aPoly = rRect );
    xPolyPoly->Insert( aPoly );

    // loop to output Polygone/PolyPolygone sequentially
    for( long i = 1; i < nSteps; i++ )
    {
        // calculate new Polygon
        aRect.SetLeft( static_cast<long>( fScanLeft += fScanIncX ) );
        aRect.SetTop( static_cast<long>( fScanTop += fScanIncY ) );
        aRect.SetRight( static_cast<long>( fScanRight -= fScanIncX ) );
        aRect.SetBottom( static_cast<long>( fScanBottom -= fScanIncY ) );

        if( ( aRect.GetWidth() < 2 ) || ( aRect.GetHeight() < 2 ) )
            break;

        if( rGradient.GetStyle() == GradientStyle::Radial || rGradient.GetStyle() == GradientStyle::Elliptical )
            aPoly = tools::Polygon( aRect.Center(), aRect.GetWidth() >> 1, aRect.GetHeight() >> 1 );
        else
            aPoly = tools::Polygon( aRect );

        aPoly.Rotate( aCenter, nAngle );

        // adapt colour accordingly
        const long nStepIndex = ( xPolyPoly ? i : ( i + 1 ) );
        nRed = GetGradientColorValue( nStartRed + ( ( nRedSteps * nStepIndex ) / nSteps ) );
        nGreen = GetGradientColorValue( nStartGreen + ( ( nGreenSteps * nStepIndex ) / nSteps ) );
        nBlue = GetGradientColorValue( nStartBlue + ( ( nBlueSteps * nStepIndex ) / nSteps ) );

        bPaintLastPolygon = true; // #107349# Paint last polygon only if loop has generated any output

        xPolyPoly->Replace( xPolyPoly->GetObject( 1 ), 0 );
        xPolyPoly->Replace( aPoly, 1 );

        mpMetaFile->AddAction( new MetaPolyPolygonAction( *xPolyPoly ) );

        // #107349# Set fill color _after_ geometry painting:
        // xPolyPoly's geometry is the band from last iteration's
        // aPoly to current iteration's aPoly. The window outdev
        // path (see else below), on the other hand, paints the
        // full aPoly. Thus, here, we're painting the band before
        // the one painted in the window outdev path below. To get
        // matching colors, have to delay color setting here.
        mpMetaFile->AddAction( new MetaFillColorAction( Color( nRed, nGreen, nBlue ), true ) );
    }

    const tools::Polygon& rPoly = xPolyPoly->GetObject( 1 );

    if( !rPoly.GetBoundRect().IsEmpty() )
    {
        // #107349# Paint last polygon with end color only if loop
        // has generated output. Otherwise, the current
        // (i.e. start) color is taken, to generate _any_ output.
        if( bPaintLastPolygon )
        {
            nRed = GetGradientColorValue( nEndRed );
            nGreen = GetGradientColorValue( nEndGreen );
            nBlue = GetGradientColorValue( nEndBlue );
        }

        mpMetaFile->AddAction( new MetaFillColorAction( Color( nRed, nGreen, nBlue ), true ) );
        mpMetaFile->AddAction( new MetaPolygonAction( rPoly ) );
    }
}

long OutputDevice::GetGradientStepCount( long nMinRect )
{
    long nInc = (nMinRect < 50) ? 2 : 4;

    return nInc;
}

long OutputDevice::GetGradientSteps( const Gradient& rGradient, const tools::Rectangle& rRect, bool bMtf, bool bComplex )
{
    // calculate step count
    long nStepCount  = rGradient.GetSteps();
    long nMinRect;

    // generate nStepCount, if not passed
    if (bComplex)
        nMinRect = std::min( rRect.GetWidth(), rRect.GetHeight() );
    else
        nMinRect = rRect.GetHeight();

    if ( !nStepCount )
    {
        long nInc;

        nInc = GetGradientStepCount (nMinRect);
        if ( !nInc || bMtf )
            nInc = 1;
        nStepCount = nMinRect / nInc;
    }

    return nStepCount;
}

Color OutputDevice::GetSingleColorGradientFill()
{
    Color aColor;

    // we should never call on this function if any of these aren't set!
    assert( mnDrawMode & ( DrawModeFlags::BlackGradient | DrawModeFlags::WhiteGradient | DrawModeFlags::SettingsGradient) );

    if ( mnDrawMode & DrawModeFlags::BlackGradient )
        aColor = COL_BLACK;
    else if ( mnDrawMode & DrawModeFlags::WhiteGradient )
        aColor = COL_WHITE;
    else if ( mnDrawMode & DrawModeFlags::SettingsGradient )
        aColor = GetSettings().GetStyleSettings().GetWindowColor();

    if ( mnDrawMode & DrawModeFlags::GhostedGradient )
    {
        aColor = Color( ( aColor.GetRed() >> 1 ) | 0x80,
                        ( aColor.GetGreen() >> 1 ) | 0x80,
                        ( aColor.GetBlue() >> 1 ) | 0x80 );
    }

    return aColor;
}

void OutputDevice::SetGrayscaleColors( Gradient &rGradient )
{
    // this should only be called with the drawing mode is for grayscale or ghosted gradients
    assert ( mnDrawMode & ( DrawModeFlags::GrayGradient | DrawModeFlags::GhostedGradient ) );

    Color aStartCol( rGradient.GetStartColor() );
    Color aEndCol( rGradient.GetEndColor() );

    if ( mnDrawMode & DrawModeFlags::GrayGradient )
    {
        sal_uInt8 cStartLum = aStartCol.GetLuminance(), cEndLum = aEndCol.GetLuminance();
        aStartCol = Color( cStartLum, cStartLum, cStartLum );
        aEndCol = Color( cEndLum, cEndLum, cEndLum );
    }

    if ( mnDrawMode & DrawModeFlags::GhostedGradient )
    {
        aStartCol = Color( ( aStartCol.GetRed() >> 1 ) | 0x80,
                           ( aStartCol.GetGreen() >> 1 ) | 0x80,
                           ( aStartCol.GetBlue() >> 1 ) | 0x80 );

        aEndCol = Color( ( aEndCol.GetRed() >> 1 ) | 0x80,
                         ( aEndCol.GetGreen() >> 1 ) | 0x80,
                         ( aEndCol.GetBlue() >> 1 ) | 0x80 );
    }

    rGradient.SetStartColor( aStartCol );
    rGradient.SetEndColor( aEndCol );
}

void OutputDevice::AddGradientActions( const tools::Rectangle& rRect, const Gradient& rGradient,
                                       GDIMetaFile& rMtf )
{

    tools::Rectangle aRect( rRect );

    aRect.Justify();

    // do nothing if the rectangle is empty
    if ( aRect.IsEmpty() )
        return;

    Gradient        aGradient( rGradient );
    GDIMetaFile*    pOldMtf = mpMetaFile;

    mpMetaFile = &rMtf;
    mpMetaFile->AddAction( new MetaPushAction( PushFlags::ALL ) );
    mpMetaFile->AddAction( new MetaISectRectClipRegionAction( aRect ) );
    mpMetaFile->AddAction( new MetaLineColorAction( Color(), false ) );

    // because we draw with no border line, we have to expand gradient
    // rect to avoid missing lines on the right and bottom edge
    aRect.AdjustLeft( -1 );
    aRect.AdjustTop( -1 );
    aRect.AdjustRight( 1 );
    aRect.AdjustBottom( 1 );

    // calculate step count if necessary
    if ( !aGradient.GetSteps() )
        aGradient.SetSteps( GRADIENT_DEFAULT_STEPCOUNT );

    if( aGradient.GetStyle() == GradientStyle::Linear || aGradient.GetStyle() == GradientStyle::Axial )
        DrawLinearGradientToMetafile( aRect, aGradient );
    else
        DrawComplexGradientToMetafile( aRect, aGradient );

    mpMetaFile->AddAction( new MetaPopAction() );
    mpMetaFile = pOldMtf;

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
