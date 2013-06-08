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

DBG_NAME( Gradient )

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
    DBG_CTOR( Gradient, NULL );

    mpImplGradient = new Impl_Gradient;
}

Gradient::Gradient( const Gradient& rGradient )
{
    DBG_CTOR( Gradient, NULL );
    DBG_CHKOBJ( &rGradient, Gradient, NULL );

    // Take over instance data and increment refcount
    mpImplGradient = rGradient.mpImplGradient;
    mpImplGradient->mnRefCount++;
}

Gradient::Gradient( GradientStyle eStyle,
                    const Color& rStartColor, const Color& rEndColor )
{
    DBG_CTOR( Gradient, NULL );

    mpImplGradient                  = new Impl_Gradient;
    mpImplGradient->meStyle         = eStyle;
    mpImplGradient->maStartColor    = rStartColor;
    mpImplGradient->maEndColor      = rEndColor;
}

Gradient::~Gradient()
{
    DBG_DTOR( Gradient, NULL );

    // If it's the last reference, delete it, otherwise
    // decrement refcount
    if ( mpImplGradient->mnRefCount == 1 )
        delete mpImplGradient;
    else
        mpImplGradient->mnRefCount--;
}

void Gradient::SetStyle( GradientStyle eStyle )
{
    DBG_CHKTHIS( Gradient, NULL );

    MakeUnique();
    mpImplGradient->meStyle = eStyle;
}

void Gradient::SetStartColor( const Color& rColor )
{
    DBG_CHKTHIS( Gradient, NULL );

    MakeUnique();
    mpImplGradient->maStartColor = rColor;
}

void Gradient::SetEndColor( const Color& rColor )
{
    DBG_CHKTHIS( Gradient, NULL );

    MakeUnique();
    mpImplGradient->maEndColor = rColor;
}

void Gradient::SetAngle( sal_uInt16 nAngle )
{
    DBG_CHKTHIS( Gradient, NULL );

    MakeUnique();
    mpImplGradient->mnAngle = nAngle;
}

void Gradient::SetBorder( sal_uInt16 nBorder )
{
    DBG_CHKTHIS( Gradient, NULL );

    MakeUnique();
    mpImplGradient->mnBorder = nBorder;
}

void Gradient::SetOfsX( sal_uInt16 nOfsX )
{
    DBG_CHKTHIS( Gradient, NULL );

    MakeUnique();
    mpImplGradient->mnOfsX = nOfsX;
}

void Gradient::SetOfsY( sal_uInt16 nOfsY )
{
    DBG_CHKTHIS( Gradient, NULL );

    MakeUnique();
    mpImplGradient->mnOfsY = nOfsY;
}

void Gradient::SetStartIntensity( sal_uInt16 nIntens )
{
    DBG_CHKTHIS( Gradient, NULL );

    MakeUnique();
    mpImplGradient->mnIntensityStart = nIntens;
}

void Gradient::SetEndIntensity( sal_uInt16 nIntens )
{
    DBG_CHKTHIS( Gradient, NULL );

    MakeUnique();
    mpImplGradient->mnIntensityEnd = nIntens;
}

void Gradient::SetSteps( sal_uInt16 nSteps )
{
    DBG_CHKTHIS( Gradient, NULL );

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
    DBG_CHKTHIS( Gradient, NULL );
    DBG_CHKOBJ( &rGradient, Gradient, NULL );

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

sal_Bool Gradient::operator==( const Gradient& rGradient ) const
{
    DBG_CHKTHIS( Gradient, NULL );
    DBG_CHKOBJ( &rGradient, Gradient, NULL );

    if ( mpImplGradient == rGradient.mpImplGradient )
        return sal_True;

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
         return sal_True;
    else
        return sal_False;
}

SvStream& operator>>( SvStream& rIStm, Impl_Gradient& rImpl_Gradient )
{
    VersionCompat   aCompat( rIStm, STREAM_READ );
    sal_uInt16          nTmp16;

    rIStm >> nTmp16; rImpl_Gradient.meStyle = (GradientStyle) nTmp16;

    rIStm >> rImpl_Gradient.maStartColor >>
             rImpl_Gradient.maEndColor >>
             rImpl_Gradient.mnAngle >>
             rImpl_Gradient.mnBorder >>
             rImpl_Gradient.mnOfsX >>
             rImpl_Gradient.mnOfsY >>
             rImpl_Gradient.mnIntensityStart >>
             rImpl_Gradient.mnIntensityEnd >>
             rImpl_Gradient.mnStepCount;

    return rIStm;
}

SvStream& operator<<( SvStream& rOStm, const Impl_Gradient& rImpl_Gradient )
{
    VersionCompat aCompat( rOStm, STREAM_WRITE, 1 );

    rOStm << (sal_uInt16) rImpl_Gradient.meStyle <<
             rImpl_Gradient.maStartColor <<
             rImpl_Gradient.maEndColor <<
             rImpl_Gradient.mnAngle <<
             rImpl_Gradient.mnBorder <<
             rImpl_Gradient.mnOfsX <<
             rImpl_Gradient.mnOfsY <<
             rImpl_Gradient.mnIntensityStart <<
             rImpl_Gradient.mnIntensityEnd <<
             rImpl_Gradient.mnStepCount;

    return rOStm;
}

SvStream& operator>>( SvStream& rIStm, Gradient& rGradient )
{
    rGradient.MakeUnique();
    return( rIStm >> *rGradient.mpImplGradient );
}

SvStream& operator<<( SvStream& rOStm, const Gradient& rGradient )
{
    return( rOStm << *rGradient.mpImplGradient );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
