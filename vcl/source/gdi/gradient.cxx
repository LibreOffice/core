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

#include <tools/stream.hxx>
#include <tools/vcompat.hxx>
#include <tools/gen.hxx>
#include <vcl/gradient.hxx>

Impl_Gradient::Impl_Gradient() :
    maStartColor( COL_BLACK ),
    maEndColor( COL_WHITE )
{
    meStyle             = GradientStyle::Linear;
    mnAngle             = 0;
    mnBorder            = 0;
    mnOfsX              = 50;
    mnOfsY              = 50;
    mnIntensityStart    = 100;
    mnIntensityEnd      = 100;
    mnStepCount         = 0;
}

Impl_Gradient::Impl_Gradient( const Impl_Gradient& rImplGradient ) :
    maStartColor( rImplGradient.maStartColor ),
    maEndColor( rImplGradient.maEndColor )
{
    meStyle             = rImplGradient.meStyle;
    mnAngle             = rImplGradient.mnAngle;
    mnBorder            = rImplGradient.mnBorder;
    mnOfsX              = rImplGradient.mnOfsX;
    mnOfsY              = rImplGradient.mnOfsY;
    mnIntensityStart    = rImplGradient.mnIntensityStart;
    mnIntensityEnd      = rImplGradient.mnIntensityEnd;
    mnStepCount         = rImplGradient.mnStepCount;
}

bool Impl_Gradient::operator==( const Impl_Gradient& rImpl_Gradient ) const
{
    return (meStyle           == rImpl_Gradient.meStyle)           &&
         (mnAngle           == rImpl_Gradient.mnAngle)           &&
         (mnBorder          == rImpl_Gradient.mnBorder)          &&
         (mnOfsX            == rImpl_Gradient.mnOfsX)            &&
         (mnOfsY            == rImpl_Gradient.mnOfsY)            &&
         (mnStepCount       == rImpl_Gradient.mnStepCount)       &&
         (mnIntensityStart  == rImpl_Gradient.mnIntensityStart)  &&
         (mnIntensityEnd    == rImpl_Gradient.mnIntensityEnd)    &&
         (maStartColor      == rImpl_Gradient.maStartColor)      &&
         (maEndColor        == rImpl_Gradient.maEndColor);
}

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

void Gradient::SetStyle( GradientStyle eStyle )
{
    mpImplGradient->meStyle = eStyle;
}

void Gradient::SetStartColor( const Color& rColor )
{
    mpImplGradient->maStartColor = rColor;
}

void Gradient::SetEndColor( const Color& rColor )
{
    mpImplGradient->maEndColor = rColor;
}

void Gradient::SetAngle( sal_uInt16 nAngle )
{
    mpImplGradient->mnAngle = nAngle;
}

void Gradient::SetBorder( sal_uInt16 nBorder )
{
    mpImplGradient->mnBorder = nBorder;
}

void Gradient::SetOfsX( sal_uInt16 nOfsX )
{
    mpImplGradient->mnOfsX = nOfsX;
}

void Gradient::SetOfsY( sal_uInt16 nOfsY )
{
    mpImplGradient->mnOfsY = nOfsY;
}

void Gradient::SetStartIntensity( sal_uInt16 nIntens )
{
    mpImplGradient->mnIntensityStart = nIntens;
}

void Gradient::SetEndIntensity( sal_uInt16 nIntens )
{
    mpImplGradient->mnIntensityEnd = nIntens;
}

void Gradient::SetSteps( sal_uInt16 nSteps )
{
    mpImplGradient->mnStepCount = nSteps;
}

void Gradient::GetBoundRect( const tools::Rectangle& rRect, tools::Rectangle& rBoundRect, Point& rCenter ) const
{
    tools::Rectangle aRect( rRect );
    sal_uInt16 nAngle = GetAngle() % 3600;

    if( GetStyle() == GradientStyle::Linear || GetStyle() == GradientStyle::Axial )
    {
        const double    fAngle = nAngle * F_PI1800;
        const double    fWidth = aRect.GetWidth();
        const double    fHeight = aRect.GetHeight();
        double  fDX     = fWidth  * fabs( cos( fAngle ) ) +
                          fHeight * fabs( sin( fAngle ) );
        double  fDY     = fHeight * fabs( cos( fAngle ) ) +
                          fWidth  * fabs( sin( fAngle ) );
        fDX     = (fDX - fWidth)  * 0.5 + 0.5;
        fDY     = (fDY - fHeight) * 0.5 + 0.5;
        aRect.AdjustLeft( -static_cast<long>(fDX) );
        aRect.AdjustRight(static_cast<long>(fDX) );
        aRect.AdjustTop( -static_cast<long>(fDY) );
        aRect.AdjustBottom(static_cast<long>(fDY) );

        rBoundRect = aRect;
        rCenter = rRect.Center();
    }
    else
    {
        if( GetStyle() == GradientStyle::Square || GetStyle() == GradientStyle::Rect )
        {
            const double    fAngle = nAngle * F_PI1800;
            const double    fWidth = aRect.GetWidth();
            const double    fHeight = aRect.GetHeight();
            double          fDX = fWidth  * fabs( cos( fAngle ) ) + fHeight * fabs( sin( fAngle ) );
            double          fDY = fHeight * fabs( cos( fAngle ) ) + fWidth  * fabs( sin( fAngle ) );

            fDX = ( fDX - fWidth  ) * 0.5 + 0.5;
            fDY = ( fDY - fHeight ) * 0.5 + 0.5;

            aRect.AdjustLeft( -static_cast<long>(fDX) );
            aRect.AdjustRight(static_cast<long>(fDX) );
            aRect.AdjustTop( -static_cast<long>(fDY) );
            aRect.AdjustBottom(static_cast<long>(fDY) );
        }

        Size aSize( aRect.GetSize() );

        if( GetStyle() == GradientStyle::Radial )
        {
            // Calculation of radii for circle
            aSize.setWidth( static_cast<long>(0.5 + sqrt(static_cast<double>(aSize.Width())*static_cast<double>(aSize.Width()) + static_cast<double>(aSize.Height())*static_cast<double>(aSize.Height()))) );
            aSize.setHeight( aSize.Width() );
        }
        else if( GetStyle() == GradientStyle::Elliptical )
        {
            // Calculation of radii for ellipse
            aSize.setWidth( static_cast<long>( 0.5 + static_cast<double>(aSize.Width())  * 1.4142 ) );
            aSize.setHeight( static_cast<long>( 0.5 + static_cast<double>(aSize.Height()) * 1.4142 ) );
        }

        // Calculate new centers
        long    nZWidth = aRect.GetWidth() * static_cast<long>(GetOfsX()) / 100;
        long    nZHeight = aRect.GetHeight() * static_cast<long>(GetOfsY()) / 100;
        long    nBorderX = static_cast<long>(GetBorder()) * aSize.Width()  / 100;
        long    nBorderY = static_cast<long>(GetBorder()) * aSize.Height() / 100;
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

Gradient& Gradient::operator=( const Gradient& ) = default;

Gradient& Gradient::operator=( Gradient&& ) = default;

bool Gradient::operator==( const Gradient& rGradient ) const
{
    return mpImplGradient == rGradient.mpImplGradient;
}

SvStream& ReadGradient( SvStream& rIStm, Gradient& rGradient )
{
    VersionCompat   aCompat( rIStm, StreamMode::READ );
    sal_uInt16          nTmp16;

    rIStm.ReadUInt16( nTmp16 ); rGradient.mpImplGradient->meStyle = static_cast<GradientStyle>(nTmp16);

    ReadColor( rIStm, rGradient.mpImplGradient->maStartColor );
    ReadColor( rIStm, rGradient.mpImplGradient->maEndColor );
    rIStm.ReadUInt16( rGradient.mpImplGradient->mnAngle )
         .ReadUInt16( rGradient.mpImplGradient->mnBorder )
         .ReadUInt16( rGradient.mpImplGradient->mnOfsX )
         .ReadUInt16( rGradient.mpImplGradient->mnOfsY )
         .ReadUInt16( rGradient.mpImplGradient->mnIntensityStart )
         .ReadUInt16( rGradient.mpImplGradient->mnIntensityEnd )
         .ReadUInt16( rGradient.mpImplGradient->mnStepCount );

    return rIStm;
}

SvStream& WriteGradient( SvStream& rOStm, const Gradient& rGradient )
{
    VersionCompat aCompat( rOStm, StreamMode::WRITE, 1 );

    rOStm.WriteUInt16( static_cast<sal_uInt16>(rGradient.mpImplGradient->meStyle) );
    WriteColor( rOStm, rGradient.mpImplGradient->maStartColor );
    WriteColor( rOStm, rGradient.mpImplGradient->maEndColor );
    rOStm.WriteUInt16( rGradient.mpImplGradient->mnAngle )
         .WriteUInt16( rGradient.mpImplGradient->mnBorder )
         .WriteUInt16( rGradient.mpImplGradient->mnOfsX )
         .WriteUInt16( rGradient.mpImplGradient->mnOfsY )
         .WriteUInt16( rGradient.mpImplGradient->mnIntensityStart )
         .WriteUInt16( rGradient.mpImplGradient->mnIntensityEnd )
         .WriteUInt16( rGradient.mpImplGradient->mnStepCount );

    return rOStm;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
