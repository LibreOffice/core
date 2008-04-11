/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: rgbmaskpixelformats.hxx,v $
 * $Revision: 1.4 $
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

#ifndef INCLUDED_BASEBMP_RGBMASKPIXELFORMATS_HXX
#define INCLUDED_BASEBMP_RGBMASKPIXELFORMATS_HXX

#include <basebmp/color.hxx>
#include <basebmp/colortraits.hxx>
#include <basebmp/accessor.hxx>
#include <basebmp/pixeliterator.hxx>
#include <basebmp/pixelformatadapters.hxx>
#include <basebmp/metafunctions.hxx>
#include <basebmp/endian.hxx>

#include <vigra/numerictraits.hxx>
#include <vigra/metaprogramming.hxx>

#include <functional>

namespace basebmp
{

/** Base class operating on RGB truecolor mask pixel

    Use this template, if you have an (integer) pixel type, and three
    bitmasks denoting where the channel bits are.

    @tpl PixelType
    Input pixel type to operate on

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
    passed on.
 */
template< typename     PixelType,
          typename     ColorType,
          unsigned int RedMask,
          unsigned int GreenMask,
          unsigned int BlueMask,
          bool         SwapBytes > struct RGBMaskFunctorBase
{
    typedef PixelType                                       pixel_type;
    typedef ColorType                                       color_type;
    typedef typename make_unsigned<pixel_type>::type        unsigned_pixel_type;
    typedef typename ColorTraits<ColorType>::component_type component_type;

    // calc corrective shifts for all three channels in advance
    enum {
        red_shift   = numberOfTrailingZeros<RedMask>::value,
        green_shift = numberOfTrailingZeros<GreenMask>::value,
        blue_shift  = numberOfTrailingZeros<BlueMask>::value,

        red_bits    = bitcount<RedMask>::value,
        green_bits  = bitcount<GreenMask>::value,
        blue_bits   = bitcount<BlueMask>::value
    };
};

template< typename     PixelType,
          typename     ColorType,
          unsigned int RedMask,
          unsigned int GreenMask,
          unsigned int BlueMask,
          bool         SwapBytes > struct RGBMaskGetter :
        public RGBMaskFunctorBase<PixelType,
                                  ColorType,
                                  RedMask,
                                  GreenMask,
                                  BlueMask,
                                  SwapBytes>,
        public std::unary_function<PixelType, ColorType>
{
    typedef RGBMaskFunctorBase<PixelType,
                               ColorType,
                               RedMask,
                               GreenMask,
                               BlueMask,
                               SwapBytes> base_type;

    ColorType operator()( PixelType v ) const
    {
        v = SwapBytes ? byteSwap(v) : v;

        const typename base_type::unsigned_pixel_type red  (v & RedMask);
        const typename base_type::unsigned_pixel_type green(v & GreenMask);
        const typename base_type::unsigned_pixel_type blue (v & BlueMask);

        // shift color nibbles to right-aligend position. ORing it
        // channel value shifted twice the number of channel bits, to
        // spread the value into the component_type range
        ColorType res( (shiftRight(red,
                                   base_type::red_shift-8*
                                   (signed)sizeof(typename base_type::component_type)+
                                   base_type::red_bits)) |
                       (shiftRight(red,
                                   base_type::red_shift-8*
                                   (signed)sizeof(typename base_type::component_type)+
                                   2*base_type::red_bits)),

                       (shiftRight(green,
                                   base_type::green_shift-8*
                                   (signed)sizeof(typename base_type::component_type)+
                                   base_type::green_bits)) |
                       (shiftRight(green,
                                   base_type::green_shift-8*
                                   (signed)sizeof(typename base_type::component_type)+
                                   2*base_type::green_bits)),

                       (shiftRight(blue,
                                   base_type::blue_shift-8*
                                   (signed)sizeof(typename base_type::component_type)+
                                   base_type::blue_bits)) |
                       (shiftRight(blue,
                                   base_type::blue_shift-8*
                                   (signed)sizeof(typename base_type::component_type)+
                                   2*base_type::blue_bits)) );
        return res;
    }
};

template< typename     PixelType,
          typename     ColorType,
          unsigned int RedMask,
          unsigned int GreenMask,
          unsigned int BlueMask,
          bool         SwapBytes > struct RGBMaskSetter :
        public RGBMaskFunctorBase<PixelType,
                                  ColorType,
                                  RedMask,
                                  GreenMask,
                                  BlueMask,
                                  SwapBytes>,
        public std::unary_function<ColorType, PixelType>
{
    typedef RGBMaskFunctorBase<PixelType,
                               ColorType,
                               RedMask,
                               GreenMask,
                               BlueMask,
                               SwapBytes> base_type;

    PixelType operator()( ColorType const& c ) const
    {
        const typename base_type::unsigned_pixel_type red  (c.getRed());
        const typename base_type::unsigned_pixel_type green(c.getGreen());
        const typename base_type::unsigned_pixel_type blue (c.getBlue());

        typename base_type::unsigned_pixel_type res(
            (shiftLeft(red,
                       base_type::red_shift-8*
                       (signed)sizeof(typename base_type::component_type)+
                       base_type::red_bits) & RedMask) |
            (shiftLeft(green,
                       base_type::green_shift-8*
                       (signed)sizeof(typename base_type::component_type)+
                       base_type::green_bits) & GreenMask) |
            (shiftLeft(blue,
                       base_type::blue_shift-8*
                       (signed)sizeof(typename base_type::component_type)+
                       base_type::blue_bits) & BlueMask) );

        return SwapBytes ? byteSwap(res) : res;
    }
};

//-----------------------------------------------------------------------------

template< typename     PixelType,
          unsigned int RedMask,
          unsigned int GreenMask,
          unsigned int BlueMask,
          bool         SwapBytes > struct PixelFormatTraitsTemplate_RGBMask
{
    typedef PixelType                           pixel_type;

    typedef RGBMaskGetter<pixel_type,
                          Color,
                          RedMask,
                          GreenMask,
                          BlueMask,
                          SwapBytes>            getter_type;
    typedef RGBMaskSetter<pixel_type,
                          Color,
                          RedMask,
                          GreenMask,
                          BlueMask,
                          SwapBytes>            setter_type;

    typedef PixelIterator<pixel_type>           iterator_type;
    typedef StandardAccessor<pixel_type>        raw_accessor_type;
    typedef AccessorSelector<
        getter_type, setter_type>               accessor_selector;
};

//-----------------------------------------------------------------------------

#ifdef OSL_LITENDIAN
# define BASEBMP_TRUECOLORMASK_LSB_SWAP false
# define BASEBMP_TRUECOLORMASK_MSB_SWAP true
#else
# ifdef OSL_BIGENDIAN
#  define BASEBMP_TRUECOLORMASK_LSB_SWAP true
#  define BASEBMP_TRUECOLORMASK_MSB_SWAP false
# else
#  error Undetermined endianness!
# endif
#endif

//-----------------------------------------------------------------------------

// 16bpp MSB RGB
typedef PixelFormatTraitsTemplate_RGBMask<
    sal_uInt16,
    0xF800,
    0x07E0,
    0x001F,
    BASEBMP_TRUECOLORMASK_MSB_SWAP >            PixelFormatTraits_RGB16_565_MSB;
BASEBMP_SPECIALIZE_ACCESSORTRAITS(PixelFormatTraits_RGB16_565_MSB::getter_type,
                                  PixelFormatTraits_RGB16_565_MSB::setter_type);

// 16bpp LSB RGB
typedef PixelFormatTraitsTemplate_RGBMask<
    sal_uInt16,
    0xF800,
    0x07E0,
    0x001F,
    BASEBMP_TRUECOLORMASK_LSB_SWAP >            PixelFormatTraits_RGB16_565_LSB;
BASEBMP_SPECIALIZE_ACCESSORTRAITS(PixelFormatTraits_RGB16_565_LSB::getter_type,
                                  PixelFormatTraits_RGB16_565_LSB::setter_type);

// 32bpp endian-sensitive RGB
typedef PixelFormatTraitsTemplate_RGBMask<
    sal_uInt32,
    0xFF0000,
    0x00FF00,
    0x0000FF,
    BASEBMP_TRUECOLORMASK_LSB_SWAP >            PixelFormatTraits_RGB32_888;
BASEBMP_SPECIALIZE_ACCESSORTRAITS(PixelFormatTraits_RGB32_888::getter_type,
                                  PixelFormatTraits_RGB32_888::setter_type);

// 32bpp endian-sensitive BGR
typedef PixelFormatTraitsTemplate_RGBMask<
    sal_uInt32,
    0xFF0000,
    0x00FF00,
    0x0000FF,
    BASEBMP_TRUECOLORMASK_MSB_SWAP >            PixelFormatTraits_BGR32_888;
BASEBMP_SPECIALIZE_ACCESSORTRAITS(PixelFormatTraits_BGR32_888::getter_type,
                                  PixelFormatTraits_BGR32_888::setter_type);

} // namespace basebmp

#endif /* INCLUDED_BASEBMP_RGBMASKPIXELFORMATS_HXX */
