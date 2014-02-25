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

#ifndef INCLUDED_VCL_GRADIENT_HXX
#define INCLUDED_VCL_GRADIENT_HXX

#include <sal/types.h>
#include <vcl/dllapi.h>
#include <tools/color.hxx>

#include <vcl/vclenum.hxx>


// - Impl_Gradient -


class Rectangle;
class Point;
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

    friend SvStream& ReadImpl_Gradient( SvStream& rIStm, Impl_Gradient& rImplGradient );
    friend SvStream& WriteImpl_Gradient( SvStream& rOStm, const Impl_Gradient& rImplGradient );

                    Impl_Gradient();
                    Impl_Gradient( const Impl_Gradient& rImplGradient );
};


// - Gradient -


class VCL_DLLPUBLIC Gradient
{
private:
    Impl_Gradient*  mpImplGradient;
    void            MakeUnique();

public:
                    Gradient();
                    Gradient( const Gradient& rGradient );
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
    bool            operator==( const Gradient& rGradient ) const;
    bool            operator!=( const Gradient& rGradient ) const
                        { return !(Gradient::operator==( rGradient )); }
    bool            IsSameInstance( const Gradient& rGradient ) const
                        { return (mpImplGradient == rGradient.mpImplGradient); }

    friend VCL_DLLPUBLIC SvStream& ReadGradient( SvStream& rIStm, Gradient& rGradient );
    friend VCL_DLLPUBLIC SvStream& WriteGradient( SvStream& rOStm, const Gradient& rGradient );
};

#endif // INCLUDED_VCL_GRADIENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
