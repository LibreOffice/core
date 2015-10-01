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

#ifndef INCLUDED_BASEBMP_INC_COLORMISC_HXX
#define INCLUDED_BASEBMP_INC_COLORMISC_HXX

#include <osl/diagnose.h>
#include <basebmp/color.hxx>
#include <colortraits.hxx>
#include <accessortraits.hxx>
#include <vigra/mathutil.hxx>

// Contents of this header moved out of color.hxx, as it is not useful
// for the general public (drags in vigra and other template
// functionality, that shouldn't be necessary for the ordinary client
// of BitmapDevice etc.)

namespace basebmp
{

template< bool polarity > struct ColorBitmaskOutputMaskFunctor;
template<> struct ColorBitmaskOutputMaskFunctor<true> : MaskFunctorBase<Color,sal_uInt8>
{
    Color operator()( Color v1, sal_uInt8 m, Color v2 ) const
    {
        OSL_ASSERT(m<=1);

        return Color(v1.toInt32()*(sal_uInt8)(1-m) + v2.toInt32()*m);
    }
};
template<> struct ColorBitmaskOutputMaskFunctor<false> : MaskFunctorBase<Color,sal_uInt8>
{
    Color operator()( Color v1, sal_uInt8 m, Color v2 ) const
    {
        OSL_ASSERT(m<=1);

        return Color(v1.toInt32()*m + v2.toInt32()*(sal_uInt8)(1-m));
    }
};

/// Specialized output mask functor for Color value type
template<bool polarity> struct outputMaskFunctorSelector< Color, sal_uInt8, polarity, FastMask >
{
    typedef ColorBitmaskOutputMaskFunctor<polarity> type;
};

template< bool polarity > struct ColorBlendFunctor8
  : public TernaryFunctorBase<sal_uInt8,Color,Color,Color>
{
    Color operator()( sal_uInt8 alpha,
                      Color     v1,
                      Color     v2 ) const
    {
        alpha = polarity ? alpha : 255 - alpha;

        const sal_uInt8 v1_red( v1.getRed() );
        const sal_uInt8 v1_green( v1.getGreen() );
        const sal_uInt8 v1_blue( v1.getBlue() );

        // using '>> 8' instead of '/ 0x100' is ill-advised (shifted
        // value might be negative). Better rely on decent optimizer
        // here...
        return Color(((((sal_Int32)v2.getRed() - v1_red)*alpha) / 0x100) + v1_red,
                     ((((sal_Int32)v2.getGreen() - v1_green)*alpha) / 0x100) + v1_green,
                     ((((sal_Int32)v2.getBlue() - v1_blue)*alpha) / 0x100) + v1_blue);
    }
};

template< bool polarity > struct ColorBlendFunctor32
  : public TernaryFunctorBase<Color,Color,Color,Color>
{
    Color operator()( Color input,
                      Color v1,
                      Color v2 ) const
    {
        sal_uInt8 alpha = input.getGreyscale();
        alpha = polarity ? alpha : 255 - alpha;

        const sal_uInt8 v1_red( v1.getRed() );
        const sal_uInt8 v1_green( v1.getGreen() );
        const sal_uInt8 v1_blue( v1.getBlue() );

        // using '>> 8' instead of '/ 0x100' is ill-advised (shifted
        // value might be negative). Better rely on decent optimizer
        // here...
        return Color(((((sal_Int32)v2.getRed() - v1_red)*alpha) / 0x100) + v1_red,
                     ((((sal_Int32)v2.getGreen() - v1_green)*alpha) / 0x100) + v1_green,
                     ((((sal_Int32)v2.getBlue() - v1_blue)*alpha) / 0x100) + v1_blue);
    }
};



template<> struct ColorTraits< Color >
{
    /// Type of a color component (i.e. the type of an individual channel)
    typedef sal_uInt8 component_type;

    /// Metafunction to select blend functor from color and alpha type
    template< typename AlphaType, bool polarity > struct blend_functor;

    /// Calculate normalized distance between color c1 and c2
    static inline double distance( const Color& c1,
                                   const Color& c2 )
    {
        return (c1 - c2).magnitude();
    }

    static inline component_type toGreyscale( const Color& c )
    {
        return c.getGreyscale();
    }

    static inline Color fromGreyscale( component_type c )
    {
        return Color(c,c,c);
    }
};

/// The version for plain 8 bit alpha
template<bool polarity> struct ColorTraits< Color >::blend_functor< sal_uInt8, polarity >
{
    typedef ColorBlendFunctor8<polarity> type;
};

/// The version taking grey value of a Color
template<bool polarity> struct ColorTraits< Color >::blend_functor< Color, polarity >
{
    typedef ColorBlendFunctor32<polarity> type;
};

} // namespace basebmp

namespace vigra
{

template<>
struct NumericTraits<basebmp::Color>
{
    typedef basebmp::Color Type;
    typedef basebmp::Color Promote;
    typedef basebmp::Color RealPromote;
    typedef std::complex<basebmp::Color> ComplexPromote;
    typedef sal_uInt8 ValueType;

    typedef VigraTrueType  isIntegral;
    typedef VigraFalseType isScalar;
    typedef VigraTrueType  isSigned;
    typedef VigraTrueType  isOrdered;
    typedef VigraFalseType isComplex;
};

} // namespace vigra

#endif /* INCLUDED_BASEBMP_INC_COLORMISC_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
