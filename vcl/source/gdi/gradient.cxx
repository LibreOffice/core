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
#include <tools/debug.hxx>
#include <tools/gen.hxx>
#include <vcl/gradient.hxx>


Impl_Gradient::Impl_Gradient() :
    maStartColor( COL_BLACK ),
    maEndColor( COL_WHITE )
{
    mnRefCount          = 1;
    meStyle             = GradientStyle_LINEAR;
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
    mnRefCount          = 1;
    meStyle             = rImplGradient.meStyle;
    mnAngle             = rImplGradient.mnAngle;
    mnBorder            = rImplGradient.mnBorder;
    mnOfsX              = rImplGradient.mnOfsX;
    mnOfsY              = rImplGradient.mnOfsY;
    mnIntensityStart    = rImplGradient.mnIntensityStart;
    mnIntensityEnd      = rImplGradient.mnIntensityEnd;
    mnStepCount         = rImplGradient.mnStepCount;
}

void Gradient::MakeUnique()
{
    // If there are still other references, copy
    if ( mpImplGradient->mnRefCount != 1 )
    {
        if( mpImplGradient->mnRefCount )
            mpImplGradient->mnRefCount--;

        mpImplGradient = new Impl_Gradient( *mpImplGradient );
    }
}

Gradient::Gradient()
{

    mpImplGradient = new Impl_Gradient;
}

Gradient::Gradient( const Gradient& rGradient )
{

    // Take over instance data and increment refcount
    mpImplGradient = rGradient.mpImplGradient;
    mpImplGradient->mnRefCount++;
}

Gradient::Gradient( GradientStyle eStyle,
                    const Color& rStartColor, const Color& rEndColor )
{

    mpImplGradient                  = new Impl_Gradient;
    mpImplGradient->meStyle         = eStyle;
    mpImplGradient->maStartColor    = rStartColor;
    mpImplGradient->maEndColor      = rEndColor;
}

Gradient::~Gradient()
{

    // If it's the last reference, delete it, otherwise
    // decrement refcount
    if ( mpImplGradient->mnRefCount == 1 )
        delete mpImplGradient;
    else
        mpImplGradient->mnRefCount--;
}

void Gradient::SetStyle( GradientStyle eStyle )
{

    MakeUnique();
    mpImplGradient->meStyle = eStyle;
}

void Gradient::SetStartColor( const Color& rColor )
{

    MakeUnique();
    mpImplGradient->maStartColor = rColor;
}

void Gradient::SetEndColor( const Color& rColor )
{

    MakeUnique();
    mpImplGradient->maEndColor = rColor;
}

void Gradient::SetAngle( sal_uInt16 nAngle )
{

    MakeUnique();
    mpImplGradient->mnAngle = nAngle;
}

void Gradient::SetBorder( sal_uInt16 nBorder )
{

    MakeUnique();
    mpImplGradient->mnBorder = nBorder;
}

void Gradient::SetOfsX( sal_uInt16 nOfsX )
{

    MakeUnique();
    mpImplGradient->mnOfsX = nOfsX;
}

void Gradient::SetOfsY( sal_uInt16 nOfsY )
{

    MakeUnique();
    mpImplGradient->mnOfsY = nOfsY;
}

void Gradient::SetStartIntensity( sal_uInt16 nIntens )
{

    MakeUnique();
    mpImplGradient->mnIntensityStart = nIntens;
}

void Gradient::SetEndIntensity( sal_uInt16 nIntens )
{

    MakeUnique();
    mpImplGradient->mnIntensityEnd = nIntens;
}

void Gradient::SetSteps( sal_uInt16 nSteps )
{

    MakeUnique();
    mpImplGradient->mnStepCount = nSteps;
}

void Gradient::GetBoundRect( const Rectangle& rRect, Rectangle& rBoundRect, Point& rCenter ) const
{
    Rectangle aRect( rRect );
    sal_uInt16 nAngle = GetAngle() % 3600;

    if( GetStyle() == GradientStyle_LINEAR || GetStyle() == GradientStyle_AXIAL )
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
        aRect.Left()   -= (long) fDX;
        aRect.Right()  += (long) fDX;
        aRect.Top()    -= (long) fDY;
        aRect.Bottom() += (long) fDY;

        rBoundRect = aRect;
        rCenter = rRect.Center();
    }
    else
    {
        if( GetStyle() == GradientStyle_SQUARE || GetStyle() == GradientStyle_RECT )
        {
            const double    fAngle = nAngle * F_PI1800;
            const double    fWidth = aRect.GetWidth();
            const double    fHeight = aRect.GetHeight();
            double          fDX = fWidth  * fabs( cos( fAngle ) ) + fHeight * fabs( sin( fAngle ) );
            double          fDY = fHeight * fabs( cos( fAngle ) ) + fWidth  * fabs( sin( fAngle ) );

            fDX = ( fDX - fWidth  ) * 0.5 + 0.5;
            fDY = ( fDY - fHeight ) * 0.5 + 0.5;

            aRect.Left()   -= (long) fDX;
            aRect.Right()  += (long) fDX;
            aRect.Top()    -= (long) fDY;
            aRect.Bottom() += (long) fDY;
        }

        Size aSize( aRect.GetSize() );

        if( GetStyle() == GradientStyle_RADIAL )
        {
            // Calculation of radii for circle
            aSize.Width() = (long)(0.5 + sqrt((double)aSize.Width()*(double)aSize.Width() + (double)aSize.Height()*(double)aSize.Height()));
            aSize.Height() = aSize.Width();
        }
        else if( GetStyle() == GradientStyle_ELLIPTICAL )
        {
            // Calculation of radii for ellipse
            aSize.Width() = (long)( 0.5 + (double) aSize.Width()  * 1.4142 );
            aSize.Height() = (long)( 0.5 + (double) aSize.Height() * 1.4142 );
        }

        // Calculate new centers
        long    nZWidth = aRect.GetWidth() * (long) GetOfsX() / 100;
        long    nZHeight = aRect.GetHeight() * (long) GetOfsY() / 100;
        long    nBorderX = (long) GetBorder() * aSize.Width()  / 100;
        long    nBorderY = (long) GetBorder() * aSize.Height() / 100;
        rCenter = Point( aRect.Left() + nZWidth, aRect.Top() + nZHeight );

        // Respect borders
        aSize.Width() -= nBorderX;
        aSize.Height() -= nBorderY;

        // Recalculate output rectangle
        aRect.Left() = rCenter.X() - ( aSize.Width() >> 1 );
        aRect.Top() = rCenter.Y() - ( aSize.Height() >> 1 );

        aRect.SetSize( aSize );
        rBoundRect = aRect;
    }
}

Gradient& Gradient::operator=( const Gradient& rGradient )
{

    // Increment refcount first so that we can reference ourselves
    rGradient.mpImplGradient->mnRefCount++;

    // If it's the last reference, delete it, otherwise decrement
    if ( mpImplGradient->mnRefCount == 1 )
        delete mpImplGradient;
    else
        mpImplGradient->mnRefCount--;
    mpImplGradient = rGradient.mpImplGradient;

    return *this;
}

bool Gradient::operator==( const Gradient& rGradient ) const
{

    if ( mpImplGradient == rGradient.mpImplGradient )
        return true;

    if ( (mpImplGradient->meStyle           == rGradient.mpImplGradient->meStyle)           &&
         (mpImplGradient->mnAngle           == rGradient.mpImplGradient->mnAngle)           &&
         (mpImplGradient->mnBorder          == rGradient.mpImplGradient->mnBorder)          &&
         (mpImplGradient->mnOfsX            == rGradient.mpImplGradient->mnOfsX)            &&
         (mpImplGradient->mnOfsY            == rGradient.mpImplGradient->mnOfsY)            &&
         (mpImplGradient->mnStepCount       == rGradient.mpImplGradient->mnStepCount)       &&
         (mpImplGradient->mnIntensityStart  == rGradient.mpImplGradient->mnIntensityStart)  &&
         (mpImplGradient->mnIntensityEnd    == rGradient.mpImplGradient->mnIntensityEnd)    &&
         (mpImplGradient->maStartColor      == rGradient.mpImplGradient->maStartColor)      &&
         (mpImplGradient->maEndColor        == rGradient.mpImplGradient->maEndColor) )
         return true;
    else
        return false;
}

SvStream& ReadImpl_Gradient( SvStream& rIStm, Impl_Gradient& rImpl_Gradient )
{
    VersionCompat   aCompat( rIStm, STREAM_READ );
    sal_uInt16          nTmp16;

    rIStm.ReadUInt16( nTmp16 ); rImpl_Gradient.meStyle = (GradientStyle) nTmp16;

    ReadColor( rIStm, rImpl_Gradient.maStartColor );
    ReadColor( rIStm, rImpl_Gradient.maEndColor );
    rIStm.ReadUInt16( rImpl_Gradient.mnAngle ).             ReadUInt16( rImpl_Gradient.mnBorder ).             ReadUInt16( rImpl_Gradient.mnOfsX ).             ReadUInt16( rImpl_Gradient.mnOfsY ).             ReadUInt16( rImpl_Gradient.mnIntensityStart ).             ReadUInt16( rImpl_Gradient.mnIntensityEnd ).             ReadUInt16( rImpl_Gradient.mnStepCount );

    return rIStm;
}

SvStream& WriteImpl_Gradient( SvStream& rOStm, const Impl_Gradient& rImpl_Gradient )
{
    VersionCompat aCompat( rOStm, STREAM_WRITE, 1 );

    rOStm.WriteUInt16( (sal_uInt16) rImpl_Gradient.meStyle );
    WriteColor( rOStm, rImpl_Gradient.maStartColor );
    WriteColor( rOStm, rImpl_Gradient.maEndColor );
    rOStm.WriteUInt16( rImpl_Gradient.mnAngle )
         .WriteUInt16( rImpl_Gradient.mnBorder )
         .WriteUInt16( rImpl_Gradient.mnOfsX )
         .WriteUInt16( rImpl_Gradient.mnOfsY )
         .WriteUInt16( rImpl_Gradient.mnIntensityStart )
         .WriteUInt16( rImpl_Gradient.mnIntensityEnd )
         .WriteUInt16( rImpl_Gradient.mnStepCount );

    return rOStm;
}

SvStream& ReadGradient( SvStream& rIStm, Gradient& rGradient )
{
    rGradient.MakeUnique();
    return ReadImpl_Gradient( rIStm, *rGradient.mpImplGradient );
}

SvStream& WriteGradient( SvStream& rOStm, const Gradient& rGradient )
{
    return WriteImpl_Gradient( rOStm, *rGradient.mpImplGradient );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
