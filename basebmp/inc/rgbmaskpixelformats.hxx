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

#ifndef INCLUDED_BASEBMP_INC_RGBMASKPIXELFORMATS_HXX
#define INCLUDED_BASEBMP_INC_RGBMASKPIXELFORMATS_HXX

#include <basebmp/color.hxx>
#include <colortraits.hxx>
#include <accessor.hxx>
#include <pixeliterator.hxx>
#include <pixelformatadapters.hxx>
#include <metafunctions.hxx>
#include <endian.hxx>

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

        // shift color nibbles to right-aligned position. ORing it
        // channel value shifted twice the number of channel bits, to
        // spread the value into the component_type range
        ColorType res( (shiftRight(red,
                                   base_type::red_shift-8*
                                   static_cast<signed>(sizeof(typename base_type::component_type))+
                                   base_type::red_bits)) |
                       (shiftRight(red,
                                   base_type::red_shift-8*
                                   static_cast<signed>(sizeof(typename base_type::component_type))+
                                   2*base_type::red_bits)),

                       (shiftRight(green,
                                   base_type::green_shift-8*
                                   static_cast<signed>(sizeof(typename base_type::component_type))+
                                   base_type::green_bits)) |
                       (shiftRight(green,
                                   base_type::green_shift-8*
                                   static_cast<signed>(sizeof(typename base_type::component_type))+
                                   2*base_type::green_bits)),

                       (shiftRight(blue,
                                   base_type::blue_shift-8*
                                   static_cast<signed>(sizeof(typename base_type::component_type))+
                                   base_type::blue_bits)) |
                       (shiftRight(blue,
                                   base_type::blue_shift-8*
                                   static_cast<signed>(sizeof(typename base_type::component_type))+
                                   2*base_type::blue_bits)) );
        return res;
    }
};

template< typename     PixelType,
          typename     ColorType,
          unsigned int BaseValue,
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
            BaseValue |
            (shiftLeft(red,
                       base_type::red_shift-8*
                       static_cast<signed>(sizeof(typename base_type::component_type))+
                       base_type::red_bits) & RedMask) |
            (shiftLeft(green,
                       base_type::green_shift-8*
                       static_cast<signed>(sizeof(typename base_type::component_type))+
                       base_type::green_bits) & GreenMask) |
            (shiftLeft(blue,
                       base_type::blue_shift-8*
                       static_cast<signed>(sizeof(typename base_type::component_type))+
                       base_type::blue_bits) & BlueMask) );

        return SwapBytes ? byteSwap(res) : res;
    }
};



template< typename     PixelType,
          unsigned int BaseValue,
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
                          BaseValue,
                          RedMask,
                          GreenMask,
                          BlueMask,
                          SwapBytes>            setter_type;

    typedef PixelIterator<pixel_type>           iterator_type;
    typedef StandardAccessor<pixel_type>        raw_accessor_type;
    typedef AccessorSelector<
        getter_type, setter_type>               accessor_selector;
};



// Hopefully this is an understandable plaintext explanation that matches
// reality...

// BASEBMP_TRUECOLORMASK_LSB_SWAP means that on a big-endian platform, a pixel
// value when viewed as an integer (16 or 32 bits) has to be byte-swapped for
// the channels to match the masks. Or equivalently (I think), on a big-endian
// platform, the masks need to be byte-swapped to be correct.

// I.e. on a litte-endian platform the masks work as such.

// BASEBMP_TRUECOLORMASK_MSB_SWAP means the opposite. The masks work as such
// on big-endian platforms, on little-endian platforms the pixel needs to be
// byte-swapped for the masks to work.

// So in a sense these two names are "backward". It sounds to me as if
// BASEBMP_TRUECOLORMASK_LSB_SWAP would mean "when on LSB, swap" ;)

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



// 16bpp MSB RGB
typedef PixelFormatTraitsTemplate_RGBMask<
    sal_uInt16,
    0,
    0xF800,
    0x07E0,
    0x001F,
    BASEBMP_TRUECOLORMASK_MSB_SWAP >            PixelFormatTraits_RGB16_565_MSB;
BASEBMP_SPECIALIZE_ACCESSORTRAITS(PixelFormatTraits_RGB16_565_MSB::getter_type,
                                  PixelFormatTraits_RGB16_565_MSB::setter_type);

// 16bpp LSB RGB
typedef PixelFormatTraitsTemplate_RGBMask<
    sal_uInt16,
    0,
    0xF800,
    0x07E0,
    0x001F,
    BASEBMP_TRUECOLORMASK_LSB_SWAP >            PixelFormatTraits_RGB16_565_LSB;
BASEBMP_SPECIALIZE_ACCESSORTRAITS(PixelFormatTraits_RGB16_565_LSB::getter_type,
                                  PixelFormatTraits_RGB16_565_LSB::setter_type);


// 32bpp formats

// The intent is that the order of channel names in the names of the 32bpp
// format typedefs below correspond to the order of the channel bytes in
// memory, if I understand correctly.... I think the point with the below
// formats is that the channel byte order in memory is the same regardless of
// platform byte order.

// This one used to be called PixelFormatTraits_RGB32_888.

typedef PixelFormatTraitsTemplate_RGBMask<
    sal_uInt32,
    0xFF000000,
    0x00FF0000,
    0x0000FF00,
    0x000000FF,
    BASEBMP_TRUECOLORMASK_LSB_SWAP >            PixelFormatTraits_BGRA32_8888;
BASEBMP_SPECIALIZE_ACCESSORTRAITS(PixelFormatTraits_BGRA32_8888::getter_type,
                                  PixelFormatTraits_BGRA32_8888::setter_type);

// This one used to be called PixelFormatTraits_BGR32_888.

typedef PixelFormatTraitsTemplate_RGBMask<
    sal_uInt32,
    0xFF000000,
    0x00FF0000,
    0x0000FF00,
    0x000000FF,
    BASEBMP_TRUECOLORMASK_MSB_SWAP >            PixelFormatTraits_ARGB32_8888;
BASEBMP_SPECIALIZE_ACCESSORTRAITS(PixelFormatTraits_ARGB32_8888::getter_type,
                                  PixelFormatTraits_ARGB32_8888::setter_type);

// The following two ones were added for Android needs and for completeness

typedef PixelFormatTraitsTemplate_RGBMask<
    sal_uInt32,
    0x000000FF,
    0xFF000000,
    0x00FF0000,
    0x0000FF00,
    BASEBMP_TRUECOLORMASK_LSB_SWAP >            PixelFormatTraits_ABGR32_8888;
BASEBMP_SPECIALIZE_ACCESSORTRAITS(PixelFormatTraits_ABGR32_8888::getter_type,
                                  PixelFormatTraits_ABGR32_8888::setter_type);

typedef PixelFormatTraitsTemplate_RGBMask<
    sal_uInt32,
    0x000000FF,
    0xFF000000,
    0x00FF0000,
    0x0000FF00,
    BASEBMP_TRUECOLORMASK_MSB_SWAP >            PixelFormatTraits_RGBA32_8888;
BASEBMP_SPECIALIZE_ACCESSORTRAITS(PixelFormatTraits_RGBA32_8888::getter_type,
                                  PixelFormatTraits_RGBA32_8888::setter_type);

// Added for Cairo needs, perhaps Android should get an XRGB and replace
// some uses of ARGB with that instead ?

typedef PixelFormatTraitsTemplate_RGBMask<
    sal_uInt32,
    0x00000000,
    0x00FF0000,
    0x0000FF00,
    0x000000FF,
    BASEBMP_TRUECOLORMASK_LSB_SWAP >            PixelFormatTraits_BGRX32_8888;
BASEBMP_SPECIALIZE_ACCESSORTRAITS(PixelFormatTraits_BGRX32_8888::getter_type,
                                  PixelFormatTraits_BGRX32_8888::setter_type);


} // namespace basebmp

#endif /* INCLUDED_BASEBMP_INC_RGBMASKPIXELFORMATS_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
