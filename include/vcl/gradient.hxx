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
#include <tools/color.hxx>
#include <tools/degree.hxx>
#include <tools/long.hxx>
#include <o3tl/cow_wrapper.hxx>

#include <vcl/dllapi.h>
#include <com/sun/star/awt/GradientStyle.hpp>

namespace tools { class Rectangle; }

class Point;
class SvStream;
class GDIMetaFile;

class VCL_DLLPUBLIC Gradient
{
private:
    class Impl;
    ::o3tl::cow_wrapper<Impl>  mpImplGradient;

public:
                    Gradient();
                    Gradient( const Gradient& rGradient );
                    Gradient( Gradient&& rGradient );
                    Gradient( css::awt::GradientStyle eStyle,
                              const Color& rStartColor,
                              const Color& rEndColor );
                    ~Gradient();

    void            SetStyle( css::awt::GradientStyle eStyle );
    css::awt::GradientStyle   GetStyle() const;

    void            SetStartColor( const Color& rColor );
    const Color&    GetStartColor() const;
    void            SetEndColor( const Color& rColor );
    const Color&    GetEndColor() const;
    SAL_DLLPRIVATE void MakeGrayscale();

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

    void AddGradientActions(tools::Rectangle const& rRect, GDIMetaFile& rMetaFile);

    Gradient&       operator=( const Gradient& rGradient );
    Gradient&       operator=( Gradient&& rGradient );
    bool            operator==( const Gradient& rGradient ) const;
    bool            operator!=( const Gradient& rGradient ) const
                        { return !(Gradient::operator==( rGradient )); }

private:
    SAL_DLLPRIVATE tools::Long GetMetafileSteps(tools::Rectangle const& rRect) const;

    SAL_DLLPRIVATE void DrawComplexGradientToMetafile(tools::Rectangle const& rRect, GDIMetaFile& rMetaFile) const;
    SAL_DLLPRIVATE void DrawLinearGradientToMetafile(tools::Rectangle const& rRect, GDIMetaFile& rMetaFile) const;
};

#endif // INCLUDED_VCL_GRADIENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
