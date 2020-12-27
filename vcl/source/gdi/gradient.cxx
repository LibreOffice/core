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

class Gradient::Impl
{
public:
    GradientStyle       meStyle;
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
        : meStyle (GradientStyle::Linear)
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

Gradient::Gradient( GradientStyle eStyle,
                    const Color& rStartColor, const Color& rEndColor ) :
    mpImplGradient()
{
    mpImplGradient->meStyle         = eStyle;
    mpImplGradient->maStartColor    = rStartColor;
    mpImplGradient->maEndColor      = rEndColor;
}

Gradient::~Gradient() = default;


GradientStyle Gradient::GetStyle() const
{
    return mpImplGradient->meStyle;
}

void Gradient::SetStyle( GradientStyle eStyle )
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

    if( GetStyle() == GradientStyle::Linear || GetStyle() == GradientStyle::Axial )
    {
        const double    fAngle = nAngle.get() * F_PI1800;
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
        if( GetStyle() == GradientStyle::Square || GetStyle() == GradientStyle::Rect )
        {
            const double    fAngle = nAngle.get() * F_PI1800;
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

        if( GetStyle() == GradientStyle::Radial )
        {
            // Calculation of radii for circle
            aSize.setWidth( static_cast<tools::Long>(0.5 + sqrt(static_cast<double>(aSize.Width())*static_cast<double>(aSize.Width()) + static_cast<double>(aSize.Height())*static_cast<double>(aSize.Height()))) );
            aSize.setHeight( aSize.Width() );
        }
        else if( GetStyle() == GradientStyle::Elliptical )
        {
            // Calculation of radii for ellipse
            aSize.setWidth( static_cast<tools::Long>( 0.5 + static_cast<double>(aSize.Width())  * 1.4142 ) );
            aSize.setHeight( static_cast<tools::Long>( 0.5 + static_cast<double>(aSize.Height()) * 1.4142 ) );
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
