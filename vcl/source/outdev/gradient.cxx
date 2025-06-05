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

#include <vcl/gradient.hxx>
#include <vcl/metaact.hxx>
#include <vcl/settings.hxx>
#include <vcl/virdev.hxx>
#include <vcl/window.hxx>

#include <salgdi.hxx>

#include <cassert>
#include <memory>

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

    if (mbInitClipRegion)
        InitClipRegion();
    // don't return on mbOutputClipped here, as we may need to draw the clipped metafile, even if the output is clipped

    if ( rPolyPoly.Count() && rPolyPoly[ 0 ].GetSize() )
    {
        if ( mnDrawMode & ( DrawModeFlags::BlackGradient | DrawModeFlags::WhiteGradient | DrawModeFlags::SettingsGradient) )
        {
            Color aColor = GetSingleColorGradientFill();

            Push( vcl::PushFlags::LINECOLOR | vcl::PushFlags::FILLCOLOR );
            SetLineColor( aColor );
            SetFillColor( aColor );
            DrawPolyPolygon( rPolyPoly );
            Pop();
            return;
        }

        Gradient aGradient( rGradient );

        if ( mnDrawMode & DrawModeFlags::GrayGradient )
            aGradient.MakeGrayscale();

        DrawGradientToMetafile( rPolyPoly, rGradient );

        if( !IsDeviceOutputNecessary() || ImplIsRecordLayout() )
            return;

        // Clip and then draw the gradient
        if( !tools::Rectangle( PixelToLogic( Point() ), GetOutputSize() ).IsEmpty() )
        {
            const tools::Rectangle aBoundRect( rPolyPoly.GetBoundRect() );

            // convert rectangle to pixels
            tools::Rectangle aRect( ImplLogicToDevicePixel( aBoundRect ) );
            aRect.Normalize();

            // do nothing if the rectangle is empty
            if ( !aRect.IsEmpty() )
            {
                tools::PolyPolygon aClixPolyPoly( ImplLogicToDevicePixel( rPolyPoly ) );
                bool bDrawn = false;

                if( !mpGraphics && !AcquireGraphics() )
                    return;

                // secure clip region
                Push( vcl::PushFlags::CLIPREGION );
                IntersectClipRegion( aBoundRect );

                if (mbInitClipRegion)
                    InitClipRegion();

                // try to draw gradient natively
                if (!mbOutputClipped)
                    bDrawn = mpGraphics->DrawGradient( aClixPolyPoly, aGradient, *this );

                if (!bDrawn && !mbOutputClipped)
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
                    if( aGradient.GetStyle() == css::awt::GradientStyle_LINEAR || rGradient.GetStyle() == css::awt::GradientStyle_AXIAL )
                        DrawLinearGradient( aRect, aGradient, aClixPolyPoly.IsRect() ? nullptr : &aClixPolyPoly );
                    else
                        DrawComplexGradient( aRect, aGradient, aClixPolyPoly.IsRect() ? nullptr : &aClixPolyPoly );
                }

                Pop();
            }
        }
    }

    if( mpAlphaVDev )
    {
        mpAlphaVDev->Push(vcl::PushFlags::FILLCOLOR);
        mpAlphaVDev->SetFillColor( COL_ALPHA_OPAQUE );
        mpAlphaVDev->DrawPolyPolygon( rPolyPoly );
        mpAlphaVDev->Pop();
    }
}

void OutputDevice::ClipAndDrawGradientMetafile ( const Gradient &rGradient, const tools::PolyPolygon &rPolyPoly )
{
    const bool  bOldOutput = IsOutputEnabled();
    EnableOutput( false );

    Push( vcl::PushFlags::CLIPREGION );
    SetClipRegion( vcl::Region( rPolyPoly ) );
    DrawGradient( rPolyPoly.GetBoundRect(), rGradient );
    Pop();

    EnableOutput( bOldOutput );
}

void OutputDevice::DrawGradientToMetafile ( const tools::PolyPolygon& rPolyPoly,
                                            const Gradient& rGradient )
{
    assert(!is_double_buffered_window());

    if ( !mpMetaFile )
        return;

    if ( !(rPolyPoly.Count() && rPolyPoly[ 0 ].GetSize()) )
        return;

    const tools::Rectangle aBoundRect( rPolyPoly.GetBoundRect() );

    if (aBoundRect.IsEmpty())
        return;

    Gradient aGradient( rGradient );

    if (mnDrawMode & DrawModeFlags::GrayGradient)
        aGradient.MakeGrayscale();

    if ( rPolyPoly.IsRect() )
    {
        mpMetaFile->AddAction( new MetaGradientAction( aBoundRect, std::move(aGradient) ) );
    }
    else
    {
        mpMetaFile->AddAction( new MetaCommentAction( "XGRAD_SEQ_BEGIN"_ostr ) );
        mpMetaFile->AddAction( new MetaGradientExAction( rPolyPoly, rGradient ) );

        ClipAndDrawGradientMetafile ( rGradient, rPolyPoly );

        mpMetaFile->AddAction( new MetaCommentAction( "XGRAD_SEQ_END"_ostr ) );
    }
}

namespace
{
    sal_uInt8 GetGradientColorValue( tools::Long nValue )
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
    Degree10  nAngle = rGradient.GetAngle() % 3600_deg10;

    rGradient.GetBoundRect( rRect, aRect, aCenter );

    bool bLinear = (rGradient.GetStyle() == css::awt::GradientStyle_LINEAR);
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
    tools::Long    nFactor;
    Color   aStartCol   = rGradient.GetStartColor();
    Color   aEndCol     = rGradient.GetEndColor();
    tools::Long    nStartRed   = aStartCol.GetRed();
    tools::Long    nStartGreen = aStartCol.GetGreen();
    tools::Long    nStartBlue  = aStartCol.GetBlue();
    tools::Long    nEndRed     = aEndCol.GetRed();
    tools::Long    nEndGreen   = aEndCol.GetGreen();
    tools::Long    nEndBlue    = aEndCol.GetBlue();
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
        std::swap( nStartRed, nEndRed );
        std::swap( nStartGreen, nEndGreen );
        std::swap( nStartBlue, nEndBlue );
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

        aBorderRect.SetBottom( static_cast<tools::Long>( aBorderRect.Top() + fBorder ) );
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
            aBorderRect.SetTop( static_cast<tools::Long>( aBorderRect.Bottom() - fBorder ) );
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
    tools::Long nStepCount = GetGradientSteps(rGradient, aRect);

    // minimal three steps and maximal as max color steps
    tools::Long   nAbsRedSteps   = std::abs( nEndRed   - nStartRed );
    tools::Long   nAbsGreenSteps = std::abs( nEndGreen - nStartGreen );
    tools::Long   nAbsBlueSteps  = std::abs( nEndBlue  - nStartBlue );
    tools::Long   nMaxColorSteps = std::max( nAbsRedSteps , nAbsGreenSteps );
    nMaxColorSteps = std::max( nMaxColorSteps, nAbsBlueSteps );
    tools::Long nSteps = std::min( nStepCount, nMaxColorSteps );
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

    for ( tools::Long i = 0; i < nSteps; i++ )
    {
        // linear interpolation of color
        const double fAlpha = static_cast<double>(i) / fStepsMinus1;
        double fTempColor = static_cast<double>(nStartRed) * (1.0-fAlpha) + static_cast<double>(nEndRed) * fAlpha;
        nRed = GetGradientColorValue(static_cast<tools::Long>(fTempColor));
        fTempColor = static_cast<double>(nStartGreen) * (1.0-fAlpha) + static_cast<double>(nEndGreen) * fAlpha;
        nGreen = GetGradientColorValue(static_cast<tools::Long>(fTempColor));
        fTempColor = static_cast<double>(nStartBlue) * (1.0-fAlpha) + static_cast<double>(nEndBlue) * fAlpha;
        nBlue = GetGradientColorValue(static_cast<tools::Long>(fTempColor));

        mpGraphics->SetFillColor( Color( nRed, nGreen, nBlue ) );

        // Polygon for this color step
        aRect.SetTop( static_cast<tools::Long>( fGradientLine + static_cast<double>(i) * fScanInc ) );
        aRect.SetBottom( static_cast<tools::Long>( fGradientLine + ( static_cast<double>(i) + 1.0 ) * fScanInc ) );
        aPoly[0] = aRect.TopLeft();
        aPoly[1] = aRect.TopRight();
        aPoly[2] = aRect.BottomRight();
        aPoly[3] = aRect.BottomLeft();
        aPoly.Rotate( aCenter, nAngle );

        ImplDrawPolygon( aPoly, pClixPolyPoly );

        if ( !bLinear )
        {
            aMirrorRect.SetBottom( static_cast<tools::Long>( fMirrorGradientLine - static_cast<double>(i) * fScanInc ) );
            aMirrorRect.SetTop( static_cast<tools::Long>( fMirrorGradientLine - (static_cast<double>(i) + 1.0)* fScanInc ) );
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

    aRect.SetTop( static_cast<tools::Long>( fGradientLine + static_cast<double>(nSteps) * fScanInc ) );
    aRect.SetBottom( static_cast<tools::Long>( fMirrorGradientLine - static_cast<double>(nSteps) * fScanInc ) );
    aPoly[0] = aRect.TopLeft();
    aPoly[1] = aRect.TopRight();
    aPoly[2] = aRect.BottomRight();
    aPoly[3] = aRect.BottomLeft();
    aPoly.Rotate( aCenter, nAngle );

    ImplDrawPolygon( aPoly, pClixPolyPoly );

}

bool OutputDevice::is_double_buffered_window() const
{
    auto pOwnerWindow = GetOwnerWindow();
    return pOwnerWindow && pOwnerWindow->SupportsDoubleBuffering();
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

    std::optional<tools::PolyPolygon> xPolyPoly;
    tools::Rectangle       aRect;
    Point           aCenter;
    Color           aStartCol( rGradient.GetStartColor() );
    Color           aEndCol( rGradient.GetEndColor() );
    tools::Long            nStartRed = ( static_cast<tools::Long>(aStartCol.GetRed()) * rGradient.GetStartIntensity() ) / 100;
    tools::Long            nStartGreen = ( static_cast<tools::Long>(aStartCol.GetGreen()) * rGradient.GetStartIntensity() ) / 100;
    tools::Long            nStartBlue = ( static_cast<tools::Long>(aStartCol.GetBlue()) * rGradient.GetStartIntensity() ) / 100;
    tools::Long            nEndRed = ( static_cast<tools::Long>(aEndCol.GetRed()) * rGradient.GetEndIntensity() ) / 100;
    tools::Long            nEndGreen = ( static_cast<tools::Long>(aEndCol.GetGreen()) * rGradient.GetEndIntensity() ) / 100;
    tools::Long            nEndBlue = ( static_cast<tools::Long>(aEndCol.GetBlue()) * rGradient.GetEndIntensity() ) / 100;
    tools::Long            nRedSteps = nEndRed - nStartRed;
    tools::Long            nGreenSteps = nEndGreen - nStartGreen;
    tools::Long            nBlueSteps = nEndBlue   - nStartBlue;
    Degree10       nAngle = rGradient.GetAngle() % 3600_deg10;

    rGradient.GetBoundRect( rRect, aRect, aCenter );

    if ( UsePolyPolygonForComplexGradient() )
        xPolyPoly = tools::PolyPolygon( 2 );

    tools::Long nStepCount = GetGradientSteps(rGradient, rRect);

    // at least three steps and at most the number of colour differences
    tools::Long nSteps = std::max( nStepCount, tools::Long(2) );
    tools::Long nCalcSteps  = std::abs( nRedSteps );
    tools::Long nTempSteps = std::abs( nGreenSteps );
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
    if( rGradient.GetStyle() != css::awt::GradientStyle_SQUARE )
    {
        fScanIncY = std::min( fScanIncY, fScanIncX );
        fScanIncX = fScanIncY;
    }
    sal_uInt8   nRed = static_cast<sal_uInt8>(nStartRed), nGreen = static_cast<sal_uInt8>(nStartGreen), nBlue = static_cast<sal_uInt8>(nStartBlue);
    bool    bPaintLastPolygon( false ); // #107349# Paint last polygon only if loop has generated any output

    mpGraphics->SetFillColor( Color( nRed, nGreen, nBlue ) );

    if( xPolyPoly )
    {
        aPoly = tools::Polygon(rRect);
        xPolyPoly->Insert( aPoly );
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

        aPoly = tools::Polygon(aExtRect);
        ImplDrawPolygon( aPoly, pClixPolyPoly );
    }

    // loop to output Polygon/PolyPolygon sequentially
    for( tools::Long i = 1; i < nSteps; i++ )
    {
        // calculate new Polygon
        fScanLeft += fScanIncX;
        aRect.SetLeft( static_cast<tools::Long>( fScanLeft ) );
        fScanTop += fScanIncY;
        aRect.SetTop( static_cast<tools::Long>( fScanTop ) );
        fScanRight -= fScanIncX;
        aRect.SetRight( static_cast<tools::Long>( fScanRight ) );
        fScanBottom -= fScanIncY;
        aRect.SetBottom( static_cast<tools::Long>( fScanBottom ) );

        if( ( aRect.GetWidth() < 2 ) || ( aRect.GetHeight() < 2 ) )
            break;

        if( rGradient.GetStyle() == css::awt::GradientStyle_RADIAL || rGradient.GetStyle() == css::awt::GradientStyle_ELLIPTICAL )
            aPoly = tools::Polygon( aRect.Center(), aRect.GetWidth() >> 1, aRect.GetHeight() >> 1 );
        else
            aPoly = tools::Polygon( aRect );

        aPoly.Rotate( aCenter, nAngle );

        // adapt colour accordingly
        const tools::Long nStepIndex = ( xPolyPoly ? i : ( i + 1 ) );
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
    if( !xPolyPoly )
        return;

    const tools::Polygon& rPoly = xPolyPoly->GetObject( 1 );

    if( rPoly.GetBoundRect().IsEmpty() )
        return;

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

tools::Long OutputDevice::GetGradientStepCount( tools::Long nMinRect )
{
    tools::Long nInc = (nMinRect < 50) ? 2 : 4;

    return nInc;
}

tools::Long OutputDevice::GetGradientSteps(Gradient const& rGradient, tools::Rectangle const& rRect)
{
    // calculate step count
    tools::Long nStepCount = rGradient.GetSteps();

    if (nStepCount)
        return nStepCount;

    tools::Long nMinRect = 0;

    if (rGradient.GetStyle() == css::awt::GradientStyle_LINEAR || rGradient.GetStyle() == css::awt::GradientStyle_AXIAL)
        nMinRect = rRect.GetHeight();
    else
        nMinRect = std::min(rRect.GetWidth(), rRect.GetHeight());

    tools::Long nInc = GetGradientStepCount(nMinRect);

    if (!nInc)
        nInc = 1;

    return nMinRect / nInc;
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
    {
        if (mnDrawMode & DrawModeFlags::SettingsForSelection)
            aColor = GetSettings().GetStyleSettings().GetHighlightColor();
        else
            aColor = GetSettings().GetStyleSettings().GetWindowColor();
    }

    return aColor;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
