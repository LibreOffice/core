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
#include "precompiled_vcl.hxx"
#include <tools/stream.hxx>
#include <tools/vcompat.hxx>
#include <tools/debug.hxx>
#include <vcl/gradient.hxx>

// =======================================================================

DBG_NAME( Gradient )

// -----------------------------------------------------------------------

Impl_Gradient::Impl_Gradient() :
    maStartColor( COL_BLACK ),
    maEndColor( COL_WHITE )
{
    mnRefCount          = 1;
    meStyle             = GRADIENT_LINEAR;
    mnAngle             = 0;
    mnBorder            = 0;
    mnOfsX              = 50;
    mnOfsY              = 50;
    mnIntensityStart    = 100;
    mnIntensityEnd      = 100;
    mnStepCount         = 0;
}

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

void Gradient::MakeUnique()
{
    // Falls noch andere Referenzen bestehen, dann kopieren
    if ( mpImplGradient->mnRefCount != 1 )
    {
        if( mpImplGradient->mnRefCount )
            mpImplGradient->mnRefCount--;

        mpImplGradient = new Impl_Gradient( *mpImplGradient );
    }
}

// -----------------------------------------------------------------------

Gradient::Gradient()
{
    DBG_CTOR( Gradient, NULL );

    mpImplGradient = new Impl_Gradient;
}

// -----------------------------------------------------------------------

Gradient::Gradient( const Gradient& rGradient )
{
    DBG_CTOR( Gradient, NULL );
    DBG_CHKOBJ( &rGradient, Gradient, NULL );

    // Instance Daten uebernehmen und Referenzcounter erhoehen
    mpImplGradient = rGradient.mpImplGradient;
    mpImplGradient->mnRefCount++;
}

// -----------------------------------------------------------------------

Gradient::Gradient( GradientStyle eStyle )
{
    DBG_CTOR( Gradient, NULL );

    mpImplGradient          = new Impl_Gradient;
    mpImplGradient->meStyle = eStyle;
}

// -----------------------------------------------------------------------

Gradient::Gradient( GradientStyle eStyle,
                    const Color& rStartColor, const Color& rEndColor )
{
    DBG_CTOR( Gradient, NULL );

    mpImplGradient                  = new Impl_Gradient;
    mpImplGradient->meStyle         = eStyle;
    mpImplGradient->maStartColor    = rStartColor;
    mpImplGradient->maEndColor      = rEndColor;
}

// -----------------------------------------------------------------------

Gradient::~Gradient()
{
    DBG_DTOR( Gradient, NULL );

    // Wenn es die letzte Referenz ist, loeschen,
    // sonst Referenzcounter decrementieren
    if ( mpImplGradient->mnRefCount == 1 )
        delete mpImplGradient;
    else
        mpImplGradient->mnRefCount--;
}

// -----------------------------------------------------------------------

void Gradient::SetStyle( GradientStyle eStyle )
{
    DBG_CHKTHIS( Gradient, NULL );

    MakeUnique();
    mpImplGradient->meStyle = eStyle;
}

// -----------------------------------------------------------------------

void Gradient::SetStartColor( const Color& rColor )
{
    DBG_CHKTHIS( Gradient, NULL );

    MakeUnique();
    mpImplGradient->maStartColor = rColor;
}

// -----------------------------------------------------------------------

void Gradient::SetEndColor( const Color& rColor )
{
    DBG_CHKTHIS( Gradient, NULL );

    MakeUnique();
    mpImplGradient->maEndColor = rColor;
}

// -----------------------------------------------------------------------

void Gradient::SetAngle( sal_uInt16 nAngle )
{
    DBG_CHKTHIS( Gradient, NULL );

    MakeUnique();
    mpImplGradient->mnAngle = nAngle;
}

// -----------------------------------------------------------------------

void Gradient::SetBorder( sal_uInt16 nBorder )
{
    DBG_CHKTHIS( Gradient, NULL );

    MakeUnique();
    mpImplGradient->mnBorder = nBorder;
}

// -----------------------------------------------------------------------

void Gradient::SetOfsX( sal_uInt16 nOfsX )
{
    DBG_CHKTHIS( Gradient, NULL );

    MakeUnique();
    mpImplGradient->mnOfsX = nOfsX;
}

// -----------------------------------------------------------------------

void Gradient::SetOfsY( sal_uInt16 nOfsY )
{
    DBG_CHKTHIS( Gradient, NULL );

    MakeUnique();
    mpImplGradient->mnOfsY = nOfsY;
}

// -----------------------------------------------------------------------

void Gradient::SetStartIntensity( sal_uInt16 nIntens )
{
    DBG_CHKTHIS( Gradient, NULL );

    MakeUnique();
    mpImplGradient->mnIntensityStart = nIntens;
}

// -----------------------------------------------------------------------

void Gradient::SetEndIntensity( sal_uInt16 nIntens )
{
    DBG_CHKTHIS( Gradient, NULL );

    MakeUnique();
    mpImplGradient->mnIntensityEnd = nIntens;
}

// -----------------------------------------------------------------------

void Gradient::SetSteps( sal_uInt16 nSteps )
{
    DBG_CHKTHIS( Gradient, NULL );

    MakeUnique();
    mpImplGradient->mnStepCount = nSteps;
}

// -----------------------------------------------------------------------

void Gradient::GetBoundRect( const Rectangle& rRect, Rectangle& rBoundRect, Point& rCenter ) const
{
    Rectangle aRect( rRect );
    sal_uInt16 nAngle = GetAngle() % 3600;

    if( GetStyle() == GRADIENT_LINEAR || GetStyle() == GRADIENT_AXIAL )
    {
        aRect.Left()--;
        aRect.Top()--;
        aRect.Right()++;
        aRect.Bottom()++;

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

        rBoundRect = aRect;
        rCenter = rRect.Center();
    }
    else
    {

        if( GetStyle() == GRADIENT_SQUARE || GetStyle() == GRADIENT_RECT )
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

        if( GetStyle() == GRADIENT_RADIAL )
        {
            // Radien-Berechnung fuer Kreis
            aSize.Width() = (long)(0.5 + sqrt((double)aSize.Width()*(double)aSize.Width() + (double)aSize.Height()*(double)aSize.Height()));
            aSize.Height() = aSize.Width();
        }
        else if( GetStyle() == GRADIENT_ELLIPTICAL )
        {
            // Radien-Berechnung fuer Ellipse
            aSize.Width() = (long)( 0.5 + (double) aSize.Width()  * 1.4142 );
            aSize.Height() = (long)( 0.5 + (double) aSize.Height() * 1.4142 );
        }
        else if( GetStyle() == GRADIENT_SQUARE )
        {
            if ( aSize.Width() > aSize.Height() )
                aSize.Height() = aSize.Width();
            else
                aSize.Width() = aSize.Height();
        }

        // neue Mittelpunkte berechnen
        long    nZWidth = aRect.GetWidth() * (long) GetOfsX() / 100;
        long    nZHeight = aRect.GetHeight() * (long) GetOfsY() / 100;
        long    nBorderX = (long) GetBorder() * aSize.Width()  / 100;
        long    nBorderY = (long) GetBorder() * aSize.Height() / 100;
        rCenter = Point( aRect.Left() + nZWidth, aRect.Top() + nZHeight );

        // Rand beruecksichtigen
        aSize.Width() -= nBorderX;
        aSize.Height() -= nBorderY;

        // Ausgaberechteck neu setzen
        aRect.Left() = rCenter.X() - ( aSize.Width() >> 1 );
        aRect.Top() = rCenter.Y() - ( aSize.Height() >> 1 );

        aRect.SetSize( aSize );
        rBoundRect = rRect;
    }
}

// -----------------------------------------------------------------------

Gradient& Gradient::operator=( const Gradient& rGradient )
{
    DBG_CHKTHIS( Gradient, NULL );
    DBG_CHKOBJ( &rGradient, Gradient, NULL );

    // Zuerst Referenzcounter erhoehen, damit man sich selbst zuweisen kann
    rGradient.mpImplGradient->mnRefCount++;

    // Wenn es die letzte Referenz ist, loeschen,
    // sonst Referenzcounter decrementieren
    if ( mpImplGradient->mnRefCount == 1 )
        delete mpImplGradient;
    else
        mpImplGradient->mnRefCount--;
    mpImplGradient = rGradient.mpImplGradient;

    return *this;
}

// -----------------------------------------------------------------------

sal_Bool Gradient::operator==( const Gradient& rGradient ) const
{
    DBG_CHKTHIS( Gradient, NULL );
    DBG_CHKOBJ( &rGradient, Gradient, NULL );

    if ( mpImplGradient == rGradient.mpImplGradient )
        return sal_True;

    if ( (mpImplGradient->meStyle           == rGradient.mpImplGradient->meStyle)           ||
         (mpImplGradient->mnAngle           == rGradient.mpImplGradient->mnAngle)           ||
         (mpImplGradient->mnBorder          == rGradient.mpImplGradient->mnBorder)          ||
         (mpImplGradient->mnOfsX            == rGradient.mpImplGradient->mnOfsX)            ||
         (mpImplGradient->mnOfsY            == rGradient.mpImplGradient->mnOfsY)            ||
         (mpImplGradient->mnStepCount       == rGradient.mpImplGradient->mnStepCount)       ||
         (mpImplGradient->mnIntensityStart  == rGradient.mpImplGradient->mnIntensityStart)  ||
         (mpImplGradient->mnIntensityEnd    == rGradient.mpImplGradient->mnIntensityEnd)    ||
         (mpImplGradient->maStartColor      == rGradient.mpImplGradient->maStartColor)      ||
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

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

SvStream& operator>>( SvStream& rIStm, Gradient& rGradient )
{
    rGradient.MakeUnique();
    return( rIStm >> *rGradient.mpImplGradient );
}

// -----------------------------------------------------------------------

SvStream& operator<<( SvStream& rOStm, const Gradient& rGradient )
{
    return( rOStm << *rGradient.mpImplGradient );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
