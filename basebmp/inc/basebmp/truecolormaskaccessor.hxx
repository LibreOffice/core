/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: truecolormaskaccessor.hxx,v $
 * $Revision: 1.2 $
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

#ifndef INCLUDED_BASEBMP_TRUECOLORMASKACCESSOR_HXX
#define INCLUDED_BASEBMP_TRUECOLORMASKACCESSOR_HXX

#include <basebmp/colortraits.hxx>
#include <basebmp/accessortraits.hxx>
#include <basebmp/metafunctions.hxx>
#include <basebmp/endian.hxx>

#include <vigra/numerictraits.hxx>
#include <vigra/metaprogramming.hxx>

namespace basebmp
{

namespace
{
    /// Shift left for positive shift value, and right otherwise
    template< typename T > inline T shiftLeft( T v, int shift )
    {
        return shift > 0 ? v << shift : v >> (-shift);
    }

    /// Shift right for positive shift value, and left otherwise
    template< typename T > inline T shiftRight( T v, int shift )
    {
        return shift > 0 ? v >> shift : v << (-shift);
    }
}

/** Access true color data, which is pixel-packed into a POD.

    @tpl Accessor
    Wrapped accessor, used to access the actual pixel values

    @tpl ColorType
    Underlying color type, to convert the pixel values into

    @tpl RedMask
    Bitmask, to access the red bits in the data type

    @tpl GreenMask
    Bitmask, to access the green bits in the data type

    @tpl BlueMask
    Bitmask, to access the blue bits in the data type

    @tpl SwapBytes
    When true, the final pixel values will be byte-swapped before
    passed to/from the iterator.
 */
template< class    Accessor,
          typename ColorType,
          int      RedMask,
          int      GreenMask,
          int      BlueMask,
          bool     SwapBytes > class TrueColorMaskAccessor
{
public:
    typedef typename Accessor::value_type                   data_type;
    typedef ColorType                                       value_type;
    typedef typename make_unsigned<data_type>::type         unsigned_data_type;
    typedef typename ColorTraits<ColorType>::component_type component_type;

#ifndef BOOST_NO_MEMBER_TEMPLATE_FRIENDS
// making all members public, if no member template friends
private:
    template<class A, typename C, int R, int G, int B, bool S> friend class TrueColorMaskAccessor;
#endif

    Accessor     maAccessor;

public:
    // calc corrective shifts for all three channels in advance
    enum {
        red_shift   = numberOfTrailingZeros<RedMask>::value,
        green_shift = numberOfTrailingZeros<GreenMask>::value,
        blue_shift  = numberOfTrailingZeros<BlueMask>::value,

        red_bits    = bitcount<RedMask>::value,
        green_bits  = bitcount<GreenMask>::value,
        blue_bits   = bitcount<BlueMask>::value
    };

    // -------------------------------------------------------

    TrueColorMaskAccessor() :
        maAccessor()
    {}

    template< class A > explicit
    TrueColorMaskAccessor( TrueColorMaskAccessor< A,
                                                  ColorType,
                                                  RedMask,
                                                  GreenMask,
                                                  BlueMask,
                                                  SwapBytes > const& rSrc ) :
        maAccessor( rSrc.maAccessor )
    {}

    template< class T > explicit TrueColorMaskAccessor( T accessor ) :
        maAccessor(accessor)
    {}

    // -------------------------------------------------------

    Accessor const& getWrappedAccessor() const { return maAccessor; }
    Accessor&       getWrappedAccessor() { return maAccessor; }

    // -------------------------------------------------------

    value_type toValue( unsigned_data_type v ) const
    {
        v = SwapBytes ? byteSwap(v) : v;

        const unsigned_data_type red  (v & RedMask);
        const unsigned_data_type green(v & GreenMask);
        const unsigned_data_type blue (v & BlueMask);

        value_type res( (shiftRight(red,
                                    red_shift-8*sizeof(component_type)+red_bits)) |
                        (shiftRight(red,
                                    red_shift-8*sizeof(component_type)+2*red_bits)),

                        (shiftRight(green,
                                    green_shift-8*sizeof(component_type)+green_bits)) |
                        (shiftRight(green,
                                    green_shift-8*sizeof(component_type)+2*green_bits)),

                        (shiftRight(blue,
                                    blue_shift-8*sizeof(component_type)+blue_bits)) |
                        (shiftRight(blue,
                                    blue_shift-8*sizeof(component_type)+2*blue_bits)) );
        return res;
    }

    data_type toPacked( value_type v ) const
    {
        const unsigned_data_type red  (v.getRed());
        const unsigned_data_type green(v.getGreen());
        const unsigned_data_type blue (v.getBlue());

        unsigned_data_type res(
            (shiftLeft(red,
                       red_shift-8*sizeof(component_type)+red_bits) & RedMask) |
            (shiftLeft(green,
                       green_shift-8*sizeof(component_type)+green_bits) & GreenMask) |
            (shiftLeft(blue,
                       blue_shift-8*sizeof(component_type)+blue_bits) & BlueMask) );

        return SwapBytes ? byteSwap(res) : res;
    }

    // -------------------------------------------------------

    template< class Iterator >
    value_type operator()(Iterator const& i) const
    {
        return toValue(
            unsigned_cast<data_type>( maAccessor(i)) );
    }

    template< class Iterator, class Difference >
    value_type operator()(Iterator const& i, Difference const& diff) const
    {
        return toValue(
            unsigned_cast<data_type>( maAccessor(i,diff)) );
    }

    // -------------------------------------------------------

    template< typename V, class Iterator >
    void set(V const& value, Iterator const& i) const
    {
        maAccessor.set(
            toPacked(
                vigra::detail::RequiresExplicitCast<value_type>::cast(
                    value) ),
            i);
    }

    template< typename V, class Iterator, class Difference >
    void set(V const& value, Iterator const& i, Difference const& diff) const
    {
        maAccessor.set(
            toPacked(
                vigra::detail::RequiresExplicitCast<value_type>::cast(
                    value)),
            i,
            diff );
    }
};

//-----------------------------------------------------------------------------

/** Convert Color to packed true color value for TrueColorMaskAccessor
 */
template< class Accessor > struct ColorConvert
{
    typename Accessor::data_type operator()( const Accessor&               acc,
                                             typename Accessor::value_type v )
    {
        return acc.toPacked(v);
    }
};

//-----------------------------------------------------------------------------

// partial specialization for TrueColorMaskAccessor
template< class    Accessor,
          typename ColorType,
          int      RedMask,
          int      GreenMask,
          int      BlueMask,
          bool     SwapBytes > struct AccessorTraits<
    TrueColorMaskAccessor< Accessor,
                           ColorType,
                           RedMask,
                           GreenMask,
                           BlueMask,
                           SwapBytes > >
{
    /// value type of described accessor
    typedef typename TrueColorMaskAccessor< Accessor,
                                            ColorType,
                                            RedMask,
                                            GreenMask,
                                            BlueMask,
                                            SwapBytes >::value_type  value_type;

    /// Retrieve stand-alone color lookup function for given Accessor type
    typedef ColorConvert< TrueColorMaskAccessor< Accessor,
                                                 ColorType,
                                                 RedMask,
                                                 GreenMask,
                                                 BlueMask,
                                                 SwapBytes > >        color_lookup;

    /// Retrieve raw pixel data accessor for given Accessor type
    typedef Accessor                                                 raw_accessor;

    /** accessor for XOR setter access is disabled, since the results
     *  are usually completely unintended - you'll usually want to
     *  wrap an xor_accessor with a TrueColorMaskAccessor, not the
     *  other way around.
     */
    typedef vigra::VigraFalseType                                    xor_accessor;

    /** accessor for masked setter access is disabled, since the
     *  results are usually completely unintended - you'll usually
     *  want to wrap a masked_accessor with a TrueColorMaskAccessor,
     *  not the other way around.
     */
    template< class MaskAccessor,
              class Iterator,
              class MaskIterator > struct                            masked_accessor
    {
        typedef vigra::VigraFalseType type;
    };
};

} // namespace basebmp

#endif /* INCLUDED_BASEBMP_TRUECOLORMASKACCESSOR_HXX */
