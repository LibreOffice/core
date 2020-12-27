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
#include <tools/degree.hxx>

#include <vcl/vclenum.hxx>
#include <o3tl/cow_wrapper.hxx>


namespace tools { class Rectangle; }

class Point;
class SvStream;

class VCL_DLLPUBLIC Gradient
{
private:
    class Impl;
    ::o3tl::cow_wrapper<Impl>  mpImplGradient;

public:
                    Gradient();
                    Gradient( const Gradient& rGradient );
                    Gradient( Gradient&& rGradient );
                    Gradient( GradientStyle eStyle,
                              const Color& rStartColor,
                              const Color& rEndColor );
                    ~Gradient();

    void            SetStyle( GradientStyle eStyle );
    GradientStyle   GetStyle() const;

    void            SetStartColor( const Color& rColor );
    const Color&    GetStartColor() const;
    void            SetEndColor( const Color& rColor );
    const Color&    GetEndColor() const;
    void            MakeGrayscale();

    void            SetAngle( Degree10 nAngle );
    Degree10        GetAngle() const;

    void            SetBorder( sal_uInt16 nBorder );
    sal_uInt16      GetBorder() const;
    void            SetOfsX( sal_uInt16 nOfsX );
    sal_uInt16      GetOfsX() const;
    void            SetOfsY( sal_uInt16 nOfsY );
    sal_uInt16      GetOfsY() const;

    void            SetStartIntensity( sal_uInt16 nIntens );
    sal_uInt16      GetStartIntensity() const;
    void            SetEndIntensity( sal_uInt16 nIntens );
    sal_uInt16      GetEndIntensity() const;

    void            SetSteps( sal_uInt16 nSteps );
    sal_uInt16      GetSteps() const;

    void            GetBoundRect( const tools::Rectangle& rRect, tools::Rectangle &rBoundRect, Point& rCenter ) const;

    Gradient&       operator=( const Gradient& rGradient );
    Gradient&       operator=( Gradient&& rGradient );
    bool            operator==( const Gradient& rGradient ) const;
    bool            operator!=( const Gradient& rGradient ) const
                        { return !(Gradient::operator==( rGradient )); }
};

#endif // INCLUDED_VCL_GRADIENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
