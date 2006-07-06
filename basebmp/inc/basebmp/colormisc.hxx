/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: colormisc.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: thb $ $Date: 2006-07-06 10:00:39 $
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

#ifndef INCLUDED_BASEBMP_COLORMISC_HXX
#define INCLUDED_BASEBMP_COLORMISC_HXX

#include <osl/diagnose.h>
#include <basebmp/color.hxx>
#include <basebmp/accessoradapters.hxx>
#include <vigra/mathutil.hxx>

// Contents of this header moved out of color.hxx, as it is not useful
// for the general public (drags in vigra and other template
// functionality, that shouldn't be necessary for the ordinary client
// of BitmapDevice etc.)

namespace basebmp
{

struct ColorBitmaskOutputMaskFunctor
{
    Color operator()( Color v1, sal_uInt8 m, Color v2 ) const
    {
        OSL_ASSERT(m<=1);

        return Color(v1.toInt32()*(sal_uInt8)(1-m) + v2.toInt32()*m);
    }
};

/// Specialized output mask functor for Color value type
template<> struct outputMaskFunctorSelector< Color, sal_uInt8, FastMask >
{
    typedef ColorBitmaskOutputMaskFunctor type;
};

struct ColorBlendFunctor
{
    Color operator()( sal_uInt8 alpha,
                      Color     v1,
                      Color     v2 ) const
    {
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
    template< typename AlphaType > struct blend_functor;

    /// Calculate normalized distance between color c1 and c2
    static double distance( const Color& c1,
                            const Color& c2 )
    {
        return (c1 - c2).magnitude();
    }
};

/// Only defined for 8 bit alpha, currently
template<> template<> struct ColorTraits< Color >::blend_functor< sal_uInt8 >
{
    typedef ColorBlendFunctor type;
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
