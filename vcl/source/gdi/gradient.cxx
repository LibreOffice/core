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

#include <tools/gen.hxx>

#include <vcl/gradient.hxx>
#include <vcl/metaact.hxx>
#include <cmath>

class Gradient::Impl
{
public:
    css::awt::GradientStyle       meStyle;
    Color               maStartColor;
    Color               maEndColor;
    Degree10            mnAngle;
    sal_uInt16          mnBorder;
    sal_uInt16          mnOfsX;
    sal_uInt16          mnOfsY;
    sal_uInt16          mnIntensityStart;
    sal_uInt16          mnIntensityEnd;
    sal_uInt16          mnStepCount;

    Impl()
        : meStyle (css::awt::GradientStyle_LINEAR)
        , maStartColor(COL_BLACK)
        , maEndColor(COL_WHITE)
        , mnAngle(0)
        , mnBorder(0)
        , mnOfsX(50)
        , mnOfsY(50)
        , mnIntensityStart(100)
        , mnIntensityEnd(100)
        , mnStepCount(0)
    {
    }

    Impl(const Impl& rImplGradient)
        : meStyle (rImplGradient.meStyle)
        , maStartColor(rImplGradient.maStartColor)
        , maEndColor(rImplGradient.maEndColor)
        , mnAngle(rImplGradient.mnAngle)
        , mnBorder(rImplGradient.mnBorder)
        , mnOfsX(rImplGradient.mnOfsX)
        , mnOfsY(rImplGradient.mnOfsY)
        , mnIntensityStart(rImplGradient.mnIntensityStart)
        , mnIntensityEnd(rImplGradient.mnIntensityEnd)
        , mnStepCount(rImplGradient.mnStepCount)
    {
    }

    bool operator==(const Impl& rImpl_Gradient) const
    {
        return (meStyle == rImpl_Gradient.meStyle)
            && (mnAngle  == rImpl_Gradient.mnAngle)
            && (mnBorder == rImpl_Gradient.mnBorder)
            && (mnOfsX == rImpl_Gradient.mnOfsX)
            && (mnOfsY == rImpl_Gradient.mnOfsY)
            && (mnStepCount == rImpl_Gradient.mnStepCount)
            && (mnIntensityStart == rImpl_Gradient.mnIntensityStart)
            && (mnIntensityEnd == rImpl_Gradient.mnIntensityEnd)
            && (maStartColor == rImpl_Gradient.maStartColor)
            && (maEndColor == rImpl_Gradient.maEndColor);
    }
};

Gradient::Gradient() = default;

Gradient::Gradient( const Gradient& ) = default;

Gradient::Gradient( Gradient&& ) = default;

Gradient::Gradient( css::awt::GradientStyle eStyle,
                    const Color& rStartColor, const Color& rEndColor )
{
    mpImplGradient->meStyle         = eStyle;
    mpImplGradient->maStartColor    = rStartColor;
    mpImplGradient->maEndColor      = rEndColor;
}

Gradient::~Gradient() = default;


css::awt::GradientStyle Gradient::GetStyle() const
{
    return mpImplGradient->meStyle;
}

void Gradient::SetStyle( css::awt::GradientStyle eStyle )
{
    mpImplGradient->meStyle = eStyle;
}

const Color& Gradient::GetStartColor() const
{
    return mpImplGradient->maStartColor;
}

void Gradient::SetStartColor( const Color& rColor )
{
    mpImplGradient->maStartColor = rColor;
}

const Color& Gradient::GetEndColor() const
{
    return mpImplGradient->maEndColor;
}

void Gradient::SetEndColor( const Color& rColor )
{
    mpImplGradient->maEndColor = rColor;
}

Degree10 Gradient::GetAngle() const
{
    return mpImplGradient->mnAngle;
}

void Gradient::SetAngle( Degree10 nAngle )
{
    mpImplGradient->mnAngle = nAngle;
}

sal_uInt16 Gradient::GetBorder() const
{
    return mpImplGradient->mnBorder;
}

void Gradient::SetBorder( sal_uInt16 nBorder )
{
    mpImplGradient->mnBorder = nBorder;
}

sal_uInt16 Gradient::GetOfsX() const
{
    return mpImplGradient->mnOfsX;
}

void Gradient::SetOfsX( sal_uInt16 nOfsX )
{
    mpImplGradient->mnOfsX = nOfsX;
}

sal_uInt16 Gradient::GetOfsY() const
{
    return mpImplGradient->mnOfsY;
}

void Gradient::SetOfsY( sal_uInt16 nOfsY )
{
    mpImplGradient->mnOfsY = nOfsY;
}

sal_uInt16 Gradient::GetStartIntensity() const
{
    return mpImplGradient->mnIntensityStart;
}

void Gradient::SetStartIntensity( sal_uInt16 nIntens )
{
    mpImplGradient->mnIntensityStart = nIntens;
}

sal_uInt16 Gradient::GetEndIntensity() const
{
    return mpImplGradient->mnIntensityEnd;
}

void Gradient::SetEndIntensity( sal_uInt16 nIntens )
{
    mpImplGradient->mnIntensityEnd = nIntens;
}

sal_uInt16 Gradient::GetSteps() const
{
    return mpImplGradient->mnStepCount;
}

void Gradient::SetSteps( sal_uInt16 nSteps )
{
    mpImplGradient->mnStepCount = nSteps;
}

void Gradient::GetBoundRect( const tools::Rectangle& rRect, tools::Rectangle& rBoundRect, Point& rCenter ) const
{
    tools::Rectangle aRect( rRect );
    Degree10 nAngle = GetAngle() % 3600_deg10;

    if( GetStyle() == css::awt::GradientStyle_LINEAR || GetStyle() == css::awt::GradientStyle_AXIAL )
    {
        const double    fAngle = toRadians(nAngle);
        const double    fWidth = aRect.GetWidth();
        const double    fHeight = aRect.GetHeight();
        double  fDX     = fWidth  * fabs( cos( fAngle ) ) +
                          fHeight * fabs( sin( fAngle ) );
        double  fDY     = fHeight * fabs( cos( fAngle ) ) +
                          fWidth  * fabs( sin( fAngle ) );
        fDX     = (fDX - fWidth)  * 0.5 + 0.5;
        fDY     = (fDY - fHeight) * 0.5 + 0.5;
        aRect.AdjustLeft( -static_cast<tools::Long>(fDX) );
        aRect.AdjustRight(static_cast<tools::Long>(fDX) );
        aRect.AdjustTop( -static_cast<tools::Long>(fDY) );
        aRect.AdjustBottom(static_cast<tools::Long>(fDY) );

        rBoundRect = aRect;
        rCenter = rRect.Center();
    }
    else
    {
        if( GetStyle() == css::awt::GradientStyle_SQUARE || GetStyle() == css::awt::GradientStyle_RECT )
        {
            const double    fAngle = toRadians(nAngle);
            const double    fWidth = aRect.GetWidth();
            const double    fHeight = aRect.GetHeight();
            double          fDX = fWidth  * fabs( cos( fAngle ) ) + fHeight * fabs( sin( fAngle ) );
            double          fDY = fHeight * fabs( cos( fAngle ) ) + fWidth  * fabs( sin( fAngle ) );

            fDX = ( fDX - fWidth  ) * 0.5 + 0.5;
            fDY = ( fDY - fHeight ) * 0.5 + 0.5;

            aRect.AdjustLeft( -static_cast<tools::Long>(fDX) );
            aRect.AdjustRight(static_cast<tools::Long>(fDX) );
            aRect.AdjustTop( -static_cast<tools::Long>(fDY) );
            aRect.AdjustBottom(static_cast<tools::Long>(fDY) );
        }

        Size aSize( aRect.GetSize() );

        if( GetStyle() == css::awt::GradientStyle_RADIAL )
        {
            // Calculation of radii for circle
            aSize.setWidth( static_cast<tools::Long>(0.5 + std::hypot(aSize.Width(), aSize.Height())) );
            aSize.setHeight( aSize.Width() );
        }
        else if( GetStyle() == css::awt::GradientStyle_ELLIPTICAL )
        {
            // Calculation of radii for ellipse
            aSize.setWidth( static_cast<tools::Long>( 0.5 + static_cast<double>(aSize.Width())  * M_SQRT2 ) );
            aSize.setHeight( static_cast<tools::Long>( 0.5 + static_cast<double>(aSize.Height()) * M_SQRT2) );
        }

        // Calculate new centers
        tools::Long    nZWidth = aRect.GetWidth() * static_cast<tools::Long>(GetOfsX()) / 100;
        tools::Long    nZHeight = aRect.GetHeight() * static_cast<tools::Long>(GetOfsY()) / 100;
        tools::Long    nBorderX = static_cast<tools::Long>(GetBorder()) * aSize.Width()  / 100;
        tools::Long    nBorderY = static_cast<tools::Long>(GetBorder()) * aSize.Height() / 100;
        rCenter = Point( aRect.Left() + nZWidth, aRect.Top() + nZHeight );

        // Respect borders
        aSize.AdjustWidth( -nBorderX );
        aSize.AdjustHeight( -nBorderY );

        // Recalculate output rectangle
        aRect.SetLeft( rCenter.X() - ( aSize.Width() >> 1 ) );
        aRect.SetTop( rCenter.Y() - ( aSize.Height() >> 1 ) );

        aRect.SetSize( aSize );
        rBoundRect = aRect;
    }
}

void Gradient::MakeGrayscale()
{
    Color aStartCol(GetStartColor());
    Color aEndCol(GetEndColor());
    sal_uInt8 cStartLum = aStartCol.GetLuminance();
    sal_uInt8 cEndLum = aEndCol.GetLuminance();

    aStartCol = Color(cStartLum, cStartLum, cStartLum);
    aEndCol = Color(cEndLum, cEndLum, cEndLum);

    SetStartColor(aStartCol);
    SetEndColor(aEndCol);
}

Gradient& Gradient::operator=( const Gradient& ) = default;

Gradient& Gradient::operator=( Gradient&& ) = default;

bool Gradient::operator==( const Gradient& rGradient ) const
{
    return mpImplGradient == rGradient.mpImplGradient;
}

const sal_uInt32 GRADIENT_DEFAULT_STEPCOUNT = 0;

void Gradient::AddGradientActions(tools::Rectangle const& rRect, GDIMetaFile& rMetaFile)
{
    tools::Rectangle aRect(rRect);
    aRect.Normalize();

    // do nothing if the rectangle is empty
    if (aRect.IsEmpty())
        return;

    rMetaFile.AddAction(new MetaPushAction(vcl::PushFlags::ALL));
    rMetaFile.AddAction(new MetaISectRectClipRegionAction( aRect));
    rMetaFile.AddAction(new MetaLineColorAction(Color(), false));

    // because we draw with no border line, we have to expand gradient
    // rect to avoid missing lines on the right and bottom edge
    aRect.AdjustLeft( -1 );
    aRect.AdjustTop( -1 );
    aRect.AdjustRight( 1 );
    aRect.AdjustBottom( 1 );

    // calculate step count if necessary
    if (!GetSteps())
        SetSteps(GRADIENT_DEFAULT_STEPCOUNT);

    if (GetStyle() == css::awt::GradientStyle_LINEAR || GetStyle() == css::awt::GradientStyle_AXIAL)
        DrawLinearGradientToMetafile(aRect, rMetaFile);
    else
        DrawComplexGradientToMetafile(aRect, rMetaFile);

    rMetaFile.AddAction(new MetaPopAction());
}

void Gradient::AddGradientActionsConst(tools::Rectangle const& rRect, GDIMetaFile& rMetaFile) const
{
    tools::Rectangle aRect(rRect);
    aRect.Normalize();

    // do nothing if the rectangle is empty
    if (aRect.IsEmpty())
        return;

    rMetaFile.AddAction(new MetaPushAction(vcl::PushFlags::ALL));
    rMetaFile.AddAction(new MetaISectRectClipRegionAction(aRect));
    rMetaFile.AddAction(new MetaLineColorAction(Color(), false));

    // because we draw with no border line, we have to expand gradient
    // rect to avoid missing lines on the right and bottom edge
    aRect.AdjustLeft(-1);
    aRect.AdjustTop(-1);
    aRect.AdjustRight(1);
    aRect.AdjustBottom(1);

    // we can't mutate the stepcount as that would lose us our const qualifier, so we need it to already be set...
    assert(GetSteps());

    if (GetStyle() == css::awt::GradientStyle_LINEAR || GetStyle() == css::awt::GradientStyle_AXIAL)
        DrawLinearGradientToMetafile(aRect, rMetaFile);
    else
        DrawComplexGradientToMetafile(aRect, rMetaFile);

    rMetaFile.AddAction(new MetaPopAction());
}

tools::Long Gradient::GetMetafileSteps(tools::Rectangle const& rRect) const
{
    // calculate step count
    tools::Long nStepCount = GetSteps();

    if (nStepCount)
        return nStepCount;

    if (GetStyle() == css::awt::GradientStyle_LINEAR || GetStyle() == css::awt::GradientStyle_AXIAL)
        return rRect.GetHeight();
    else
        return std::min(rRect.GetWidth(), rRect.GetHeight());
}


static sal_uInt8 GetGradientColorValue(tools::Long nValue)
{
    if ( nValue < 0 )
        return 0;
    else if ( nValue > 0xFF )
        return 0xFF;
    else
        return static_cast<sal_uInt8>(nValue);
}

void Gradient::DrawLinearGradientToMetafile(tools::Rectangle const& rRect, GDIMetaFile& rMetaFile) const
{
    // get BoundRect of rotated rectangle
    tools::Rectangle aRect;
    Point aCenter;
    Degree10 nAngle = GetAngle() % 3600_deg10;

    GetBoundRect(rRect, aRect, aCenter);

    bool bLinear = (GetStyle() == css::awt::GradientStyle_LINEAR);
    double fBorder = GetBorder() * aRect.GetHeight() / 100.0;
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
    Color   aStartCol   = GetStartColor();
    Color   aEndCol     = GetEndColor();
    tools::Long    nStartRed   = aStartCol.GetRed();
    tools::Long    nStartGreen = aStartCol.GetGreen();
    tools::Long    nStartBlue  = aStartCol.GetBlue();
    tools::Long    nEndRed     = aEndCol.GetRed();
    tools::Long    nEndGreen   = aEndCol.GetGreen();
    tools::Long    nEndBlue    = aEndCol.GetBlue();
    nFactor     = GetStartIntensity();
    nStartRed   = (nStartRed   * nFactor) / 100;
    nStartGreen = (nStartGreen * nFactor) / 100;
    nStartBlue  = (nStartBlue  * nFactor) / 100;
    nFactor     = GetEndIntensity();
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

        rMetaFile.AddAction( new MetaFillColorAction( Color( nRed, nGreen, nBlue ), true ) );

        aBorderRect.SetBottom( static_cast<tools::Long>( aBorderRect.Top() + fBorder ) );
        aRect.SetTop( aBorderRect.Bottom() );
        aPoly[0] = aBorderRect.TopLeft();
        aPoly[1] = aBorderRect.TopRight();
        aPoly[2] = aBorderRect.BottomRight();
        aPoly[3] = aBorderRect.BottomLeft();
        aPoly.Rotate( aCenter, nAngle );

        rMetaFile.AddAction( new MetaPolygonAction( aPoly ) );

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

            rMetaFile.AddAction( new MetaPolygonAction( aPoly ) );
        }
    }

    tools::Long nStepCount = GetMetafileSteps(aRect);

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
        double fAlpha = static_cast<double>(i) / fStepsMinus1;
        double fTempColor = static_cast<double>(nStartRed) * (1.0-fAlpha) + static_cast<double>(nEndRed) * fAlpha;
        nRed = GetGradientColorValue(static_cast<tools::Long>(fTempColor));
        fTempColor = static_cast<double>(nStartGreen) * (1.0-fAlpha) + static_cast<double>(nEndGreen) * fAlpha;
        nGreen = GetGradientColorValue(static_cast<tools::Long>(fTempColor));
        fTempColor = static_cast<double>(nStartBlue) * (1.0-fAlpha) + static_cast<double>(nEndBlue) * fAlpha;
        nBlue = GetGradientColorValue(static_cast<tools::Long>(fTempColor));

        rMetaFile.AddAction( new MetaFillColorAction( Color( nRed, nGreen, nBlue ), true ) );

        // Polygon for this color step
        aRect.SetTop( static_cast<tools::Long>( fGradientLine + static_cast<double>(i) * fScanInc ) );
        aRect.SetBottom( static_cast<tools::Long>( fGradientLine + ( static_cast<double>(i) + 1.0 ) * fScanInc ) );
        aPoly[0] = aRect.TopLeft();
        aPoly[1] = aRect.TopRight();
        aPoly[2] = aRect.BottomRight();
        aPoly[3] = aRect.BottomLeft();
        aPoly.Rotate( aCenter, nAngle );

        rMetaFile.AddAction( new MetaPolygonAction( aPoly ) );

        if ( !bLinear )
        {
            aMirrorRect.SetBottom( static_cast<tools::Long>( fMirrorGradientLine - static_cast<double>(i) * fScanInc ) );
            aMirrorRect.SetTop( static_cast<tools::Long>( fMirrorGradientLine - (static_cast<double>(i) + 1.0)* fScanInc ) );
            aPoly[0] = aMirrorRect.TopLeft();
            aPoly[1] = aMirrorRect.TopRight();
            aPoly[2] = aMirrorRect.BottomRight();
            aPoly[3] = aMirrorRect.BottomLeft();
            aPoly.Rotate( aCenter, nAngle );

            rMetaFile.AddAction( new MetaPolygonAction( aPoly ) );
        }
    }
    if ( bLinear)
        return;

    // draw middle polygon with end color
    nRed = GetGradientColorValue(nEndRed);
    nGreen = GetGradientColorValue(nEndGreen);
    nBlue = GetGradientColorValue(nEndBlue);

    rMetaFile.AddAction( new MetaFillColorAction( Color( nRed, nGreen, nBlue ), true ) );

    aRect.SetTop( static_cast<tools::Long>( fGradientLine + static_cast<double>(nSteps) * fScanInc ) );
    aRect.SetBottom( static_cast<tools::Long>( fMirrorGradientLine - static_cast<double>(nSteps) * fScanInc ) );
    aPoly[0] = aRect.TopLeft();
    aPoly[1] = aRect.TopRight();
    aPoly[2] = aRect.BottomRight();
    aPoly[3] = aRect.BottomLeft();
    aPoly.Rotate( aCenter, nAngle );

    rMetaFile.AddAction( new MetaPolygonAction( std::move(aPoly) ) );

}

void Gradient::DrawComplexGradientToMetafile(tools::Rectangle const& rRect, GDIMetaFile& rMetaFile) const
{
    // Determine if we output via Polygon or PolyPolygon
    // For all rasteroperations other than Overpaint always use PolyPolygon,
    // as we will get wrong results if we output multiple times on top of each other.
    // Also for printers always use PolyPolygon, as not all printers
    // can print polygons on top of each other.

    tools::Rectangle aRect;
    Point aCenter;
    GetBoundRect(rRect, aRect, aCenter);

    std::optional<tools::PolyPolygon> xPolyPoly;
    xPolyPoly = tools::PolyPolygon( 2 );

    // last parameter - true if complex gradient, false if linear
    tools::Long nStepCount = GetMetafileSteps(rRect);

    // at least three steps and at most the number of colour differences
    tools::Long nSteps = std::max(nStepCount, tools::Long(2));

    Color aStartCol(GetStartColor());
    Color aEndCol(GetEndColor());

    tools::Long nStartRed = (static_cast<tools::Long>(aStartCol.GetRed()) * GetStartIntensity()) / 100;
    tools::Long nStartGreen = (static_cast<tools::Long>(aStartCol.GetGreen()) * GetStartIntensity()) / 100;
    tools::Long nStartBlue = (static_cast<tools::Long>(aStartCol.GetBlue()) * GetStartIntensity()) / 100;

    tools::Long nEndRed = (static_cast<tools::Long>(aEndCol.GetRed()) * GetEndIntensity()) / 100;
    tools::Long nEndGreen = (static_cast<tools::Long>(aEndCol.GetGreen()) * GetEndIntensity()) / 100;
    tools::Long nEndBlue = (static_cast<tools::Long>(aEndCol.GetBlue()) * GetEndIntensity()) / 100;

    tools::Long nRedSteps = nEndRed - nStartRed;
    tools::Long nGreenSteps = nEndGreen - nStartGreen;
    tools::Long nBlueSteps = nEndBlue - nStartBlue;

    tools::Long nCalcSteps  = std::abs(nRedSteps);
    tools::Long nTempSteps = std::abs(nGreenSteps);

    if (nTempSteps > nCalcSteps)
        nCalcSteps = nTempSteps;

    nTempSteps = std::abs( nBlueSteps );

    if (nTempSteps > nCalcSteps)
        nCalcSteps = nTempSteps;

    if (nCalcSteps < nSteps)
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
    if (GetStyle() != css::awt::GradientStyle_SQUARE)
    {
        fScanIncY = std::min( fScanIncY, fScanIncX );
        fScanIncX = fScanIncY;
    }
    sal_uInt8   nRed = static_cast<sal_uInt8>(nStartRed), nGreen = static_cast<sal_uInt8>(nStartGreen), nBlue = static_cast<sal_uInt8>(nStartBlue);
    bool    bPaintLastPolygon( false ); // #107349# Paint last polygon only if loop has generated any output

    rMetaFile.AddAction( new MetaFillColorAction( Color( nRed, nGreen, nBlue ), true ) );

    aPoly = tools::Polygon(rRect);
    xPolyPoly->Insert( aPoly );
    xPolyPoly->Insert( aPoly );

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

        if (GetStyle() == css::awt::GradientStyle_RADIAL || GetStyle() == css::awt::GradientStyle_ELLIPTICAL)
            aPoly = tools::Polygon( aRect.Center(), aRect.GetWidth() >> 1, aRect.GetHeight() >> 1 );
        else
            aPoly = tools::Polygon( aRect );

        aPoly.Rotate(aCenter, GetAngle() % 3600_deg10);

        // adapt colour accordingly
        const tools::Long nStepIndex = ( xPolyPoly ? i : ( i + 1 ) );
        nRed = GetGradientColorValue( nStartRed + ( ( nRedSteps * nStepIndex ) / nSteps ) );
        nGreen = GetGradientColorValue( nStartGreen + ( ( nGreenSteps * nStepIndex ) / nSteps ) );
        nBlue = GetGradientColorValue( nStartBlue + ( ( nBlueSteps * nStepIndex ) / nSteps ) );

        bPaintLastPolygon = true; // #107349# Paint last polygon only if loop has generated any output

        xPolyPoly->Replace( xPolyPoly->GetObject( 1 ), 0 );
        xPolyPoly->Replace( aPoly, 1 );

        rMetaFile.AddAction( new MetaPolyPolygonAction( *xPolyPoly ) );

        // #107349# Set fill color _after_ geometry painting:
        // xPolyPoly's geometry is the band from last iteration's
        // aPoly to current iteration's aPoly. The window outdev
        // path (see else below), on the other hand, paints the
        // full aPoly. Thus, here, we're painting the band before
        // the one painted in the window outdev path below. To get
        // matching colors, have to delay color setting here.
        rMetaFile.AddAction( new MetaFillColorAction( Color( nRed, nGreen, nBlue ), true ) );
    }

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

    rMetaFile.AddAction( new MetaFillColorAction( Color( nRed, nGreen, nBlue ), true ) );
    rMetaFile.AddAction( new MetaPolygonAction( rPoly ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
