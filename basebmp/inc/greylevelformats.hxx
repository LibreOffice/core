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

#ifndef INCLUDED_BASEBMP_INC_GREYLEVELFORMATS_HXX
#define INCLUDED_BASEBMP_INC_GREYLEVELFORMATS_HXX

#include <basebmp/color.hxx>
#include <colortraits.hxx>
#include <accessor.hxx>
#include <pixeliterator.hxx>
#include <packedpixeliterator.hxx>
#include <pixelformatadapters.hxx>
#include <metafunctions.hxx>

#include <vigra/numerictraits.hxx>
#include <vigra/metaprogramming.hxx>

#include <functional>

namespace basebmp
{

template< typename PixelType,
          typename ColorType,
          int      UsedRange > struct GreylevelGetter :
        public std::unary_function<PixelType, ColorType>
{
    ColorType operator()( PixelType const& c ) const
    {
        return ColorTraits<ColorType>::fromGreyscale(
            vigra::NumericTraits<PixelType>::toPromote(c) *
            vigra::NumericTraits<PixelType>::maxConst / UsedRange );
    }
};

template< typename PixelType,
          typename ColorType,
          int      UsedRange > struct GreylevelSetter :
    public std::unary_function<ColorType, PixelType>
{
    PixelType operator()( ColorType const& c ) const
    {
        return vigra::NumericTraits<PixelType>::toPromote(
            ColorTraits<ColorType>::toGreyscale(c)) *
            UsedRange /
            vigra::NumericTraits<PixelType>::maxConst;
    }
};



template< class Iterator,
          class Accessor,
          int   UsedRange > struct PixelFormatTraitsTemplate_Greylevel
{
    typedef typename Iterator::value_type       pixel_type;

    typedef GreylevelGetter<pixel_type,
                            Color,
                            UsedRange>          getter_type;
    typedef GreylevelSetter<pixel_type,
                            Color,
                            UsedRange>          setter_type;

    typedef Iterator                            iterator_type;
    typedef Accessor                            raw_accessor_type;
    typedef AccessorSelector<
        getter_type,
        setter_type >                           accessor_selector;
};

template< int BitsPerPixel,
          bool MsbFirst > struct PixelFormatTraitsTemplate_PackedGreylevel :
    public PixelFormatTraitsTemplate_Greylevel<
               PackedPixelIterator< sal_uInt8,
                                    BitsPerPixel,
                                    true >,
               NonStandardAccessor< sal_uInt8 >,
               (1UL << BitsPerPixel)-1 >
{};



// 1bpp MSB
typedef PixelFormatTraitsTemplate_PackedGreylevel<1, true> PixelFormatTraits_GREY1_MSB;

// 1bpp LSB
typedef PixelFormatTraitsTemplate_PackedGreylevel<1, false> PixelFormatTraits_GREY1_LSB;
BASEBMP_SPECIALIZE_ACCESSORTRAITS(PixelFormatTraits_GREY1_MSB::getter_type,
                                  PixelFormatTraits_GREY1_MSB::setter_type);


// 4bpp MSB
typedef PixelFormatTraitsTemplate_PackedGreylevel<4, true> PixelFormatTraits_GREY4_MSB;

// 4bpp LSB
typedef PixelFormatTraitsTemplate_PackedGreylevel<4, false> PixelFormatTraits_GREY4_LSB;
BASEBMP_SPECIALIZE_ACCESSORTRAITS(PixelFormatTraits_GREY4_MSB::getter_type,
                                  PixelFormatTraits_GREY4_MSB::setter_type);

// 8bpp
typedef PixelFormatTraitsTemplate_Greylevel<
    PixelIterator< sal_uInt8 >,
    StandardAccessor< sal_uInt8 >,
    255 >                                                   PixelFormatTraits_GREY8;
BASEBMP_SPECIALIZE_ACCESSORTRAITS(PixelFormatTraits_GREY8::getter_type,
                                  PixelFormatTraits_GREY8::setter_type);

} // namespace basebmp

#endif /* INCLUDED_BASEBMP_INC_GREYLEVELFORMATS_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
