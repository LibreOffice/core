/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: colormisc.hxx,v $
 * $Revision: 1.7 $
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

#ifndef INCLUDED_BASEBMP_COLORMISC_HXX
#define INCLUDED_BASEBMP_COLORMISC_HXX

#include <osl/diagnose.h>
#include <basebmp/color.hxx>
#include <basebmp/colortraits.hxx>
#include <basebmp/accessortraits.hxx>
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

//-----------------------------------------------------------------------------

template<> struct ColorTraits< Color >
{
    /// @return number of color channels
    static int numChannels() { return 3; }

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

    static Type zero() { return Type(); }
    static Type one() { return Type(0x01010101); }
    static Type nonZero() { return Type(0x01010101); }

    static Promote toPromote(const Type& v) { return v; }
    static RealPromote toRealPromote(const Type& v) { return v; }
    static Type fromPromote(const Promote& v) { return v; }
    static Type fromRealPromote(const RealPromote& v) { return v; }
};

} // namespace vigra

#endif /* INCLUDED_BASEBMP_COLORMISC_HXX */
