/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: gradient.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 17:55:11 $
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

#ifndef _SV_GRADIENT_HXX
#define _SV_GRADIENT_HXX

#ifndef _VCL_DLLAPI_H
#include <vcl/dllapi.h>
#endif

#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif

#include <vcl/vclenum.hxx>

// ------------------
// - Gradient-Types -
// ------------------

/*
#ifndef ENUM_GRADIENTSTYLE_DECLARED
#define ENUM_GRADIENTSTYLE_DECLARED
enum GradientStyle { GRADIENT_LINEAR, GRADIENT_AXIAL, GRADIENT_RADIAL,
                     GRADIENT_ELLIPTICAL, GRADIENT_SQUARE, GRADIENT_RECT };
#endif
*/

// ----------------
// - Impl_Gradient -
// ----------------

class SvStream;

class Impl_Gradient
{
public:
    ULONG           mnRefCount;
    GradientStyle   meStyle;
    Color           maStartColor;
    Color           maEndColor;
    USHORT          mnAngle;
    USHORT          mnBorder;
    USHORT          mnOfsX;
    USHORT          mnOfsY;
    USHORT          mnIntensityStart;
    USHORT          mnIntensityEnd;
    USHORT          mnStepCount;

    friend SvStream& operator>>( SvStream& rIStm, Impl_Gradient& rImplGradient );
    friend SvStream& operator<<( SvStream& rOStm, const Impl_Gradient& rImplGradient );

                    Impl_Gradient();
                    Impl_Gradient( const Impl_Gradient& rImplGradient );
};

// ------------
// - Gradient -
// ------------

class VCL_DLLPUBLIC Gradient
{
private:
    Impl_Gradient*  mpImplGradient;
    void            MakeUnique();

public:
                    Gradient();
                    Gradient( const Gradient& rGradient );
                    Gradient( GradientStyle eStyle );
                    Gradient( GradientStyle eStyle,
                              const Color& rStartColor,
                              const Color& rEndColor );
                    ~Gradient();

    void            SetStyle( GradientStyle eStyle );
    GradientStyle   GetStyle() const { return mpImplGradient->meStyle; }

    void            SetStartColor( const Color& rColor );
    const Color&    GetStartColor() const { return mpImplGradient->maStartColor; }
    void            SetEndColor( const Color& rColor );
    const Color&    GetEndColor() const { return mpImplGradient->maEndColor; }

    void            SetAngle( USHORT nAngle );
    USHORT          GetAngle() const { return mpImplGradient->mnAngle; }

    void            SetBorder( USHORT nBorder );
    USHORT          GetBorder() const { return mpImplGradient->mnBorder; }
    void            SetOfsX( USHORT nOfsX );
    USHORT          GetOfsX() const { return mpImplGradient->mnOfsX; }
    void            SetOfsY( USHORT nOfsY );
    USHORT          GetOfsY() const { return mpImplGradient->mnOfsY; }

    void            SetStartIntensity( USHORT nIntens );
    USHORT          GetStartIntensity() const { return mpImplGradient->mnIntensityStart; }
    void            SetEndIntensity( USHORT nIntens );
    USHORT          GetEndIntensity() const { return mpImplGradient->mnIntensityEnd; }

    void            SetSteps( USHORT nSteps );
    USHORT          GetSteps() const { return mpImplGradient->mnStepCount; }

    Gradient&       operator=( const Gradient& rGradient );
    BOOL            operator==( const Gradient& rGradient ) const;
    BOOL            operator!=( const Gradient& rGradient ) const
                        { return !(Gradient::operator==( rGradient )); }
    BOOL            IsSameInstance( const Gradient& rGradient ) const
                        { return (mpImplGradient == rGradient.mpImplGradient); }

    friend VCL_DLLPUBLIC SvStream& operator>>( SvStream& rIStm, Gradient& rGradient );
    friend VCL_DLLPUBLIC SvStream& operator<<( SvStream& rOStm, const Gradient& rGradient );
};

#endif  // _SV_GRADIENT_HXX
