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

#ifndef _SV_GRADIENT_HXX
#define _SV_GRADIENT_HXX

#include <vcl/dllapi.h>
#include <tools/color.hxx>
#include <tools/gen.hxx>

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
    sal_uLong           mnRefCount;
    GradientStyle   meStyle;
    Color           maStartColor;
    Color           maEndColor;
    sal_uInt16          mnAngle;
    sal_uInt16          mnBorder;
    sal_uInt16          mnOfsX;
    sal_uInt16          mnOfsY;
    sal_uInt16          mnIntensityStart;
    sal_uInt16          mnIntensityEnd;
    sal_uInt16          mnStepCount;

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

    void            SetAngle( sal_uInt16 nAngle );
    sal_uInt16          GetAngle() const { return mpImplGradient->mnAngle; }

    void            SetBorder( sal_uInt16 nBorder );
    sal_uInt16          GetBorder() const { return mpImplGradient->mnBorder; }
    void            SetOfsX( sal_uInt16 nOfsX );
    sal_uInt16          GetOfsX() const { return mpImplGradient->mnOfsX; }
    void            SetOfsY( sal_uInt16 nOfsY );
    sal_uInt16          GetOfsY() const { return mpImplGradient->mnOfsY; }

    void            SetStartIntensity( sal_uInt16 nIntens );
    sal_uInt16          GetStartIntensity() const { return mpImplGradient->mnIntensityStart; }
    void            SetEndIntensity( sal_uInt16 nIntens );
    sal_uInt16          GetEndIntensity() const { return mpImplGradient->mnIntensityEnd; }

    void            SetSteps( sal_uInt16 nSteps );
    sal_uInt16          GetSteps() const { return mpImplGradient->mnStepCount; }

    void            GetBoundRect( const Rectangle& rRect, Rectangle &rBoundRect, Point& rCenter ) const;

    Gradient&       operator=( const Gradient& rGradient );
    sal_Bool            operator==( const Gradient& rGradient ) const;
    sal_Bool            operator!=( const Gradient& rGradient ) const
                        { return !(Gradient::operator==( rGradient )); }
    sal_Bool            IsSameInstance( const Gradient& rGradient ) const
                        { return (mpImplGradient == rGradient.mpImplGradient); }

    friend VCL_DLLPUBLIC SvStream& operator>>( SvStream& rIStm, Gradient& rGradient );
    friend VCL_DLLPUBLIC SvStream& operator<<( SvStream& rOStm, const Gradient& rGradient );
};

#endif  // _SV_GRADIENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
