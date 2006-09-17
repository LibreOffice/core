/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: gradient.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 12:01:06 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _VCOMPAT_HXX
#include <tools/vcompat.hxx>
#endif
#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SV_GRADIENT_HXX
#include <gradient.hxx>
#endif

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

void Gradient::SetAngle( USHORT nAngle )
{
    DBG_CHKTHIS( Gradient, NULL );

    MakeUnique();
    mpImplGradient->mnAngle = nAngle;
}

// -----------------------------------------------------------------------

void Gradient::SetBorder( USHORT nBorder )
{
    DBG_CHKTHIS( Gradient, NULL );

    MakeUnique();
    mpImplGradient->mnBorder = nBorder;
}

// -----------------------------------------------------------------------

void Gradient::SetOfsX( USHORT nOfsX )
{
    DBG_CHKTHIS( Gradient, NULL );

    MakeUnique();
    mpImplGradient->mnOfsX = nOfsX;
}

// -----------------------------------------------------------------------

void Gradient::SetOfsY( USHORT nOfsY )
{
    DBG_CHKTHIS( Gradient, NULL );

    MakeUnique();
    mpImplGradient->mnOfsY = nOfsY;
}

// -----------------------------------------------------------------------

void Gradient::SetStartIntensity( USHORT nIntens )
{
    DBG_CHKTHIS( Gradient, NULL );

    MakeUnique();
    mpImplGradient->mnIntensityStart = nIntens;
}

// -----------------------------------------------------------------------

void Gradient::SetEndIntensity( USHORT nIntens )
{
    DBG_CHKTHIS( Gradient, NULL );

    MakeUnique();
    mpImplGradient->mnIntensityEnd = nIntens;
}

// -----------------------------------------------------------------------

void Gradient::SetSteps( USHORT nSteps )
{
    DBG_CHKTHIS( Gradient, NULL );

    MakeUnique();
    mpImplGradient->mnStepCount = nSteps;
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

BOOL Gradient::operator==( const Gradient& rGradient ) const
{
    DBG_CHKTHIS( Gradient, NULL );
    DBG_CHKOBJ( &rGradient, Gradient, NULL );

    if ( mpImplGradient == rGradient.mpImplGradient )
        return TRUE;

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
         return TRUE;
    else
        return FALSE;
}

SvStream& operator>>( SvStream& rIStm, Impl_Gradient& rImpl_Gradient )
{
    VersionCompat   aCompat( rIStm, STREAM_READ );
    UINT16          nTmp16;

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

    rOStm << (UINT16) rImpl_Gradient.meStyle <<
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
