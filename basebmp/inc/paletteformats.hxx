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

#ifndef INCLUDED_BASEBMP_INC_PALETTEFORMATS_HXX
#define INCLUDED_BASEBMP_INC_PALETTEFORMATS_HXX

#include <basebmp/color.hxx>
#include <colortraits.hxx>
#include <accessor.hxx>
#include <pixeliterator.hxx>
#include <packedpixeliterator.hxx>
#include <pixelformatadapters.hxx>
#include <paletteimageaccessor.hxx>
#include <metafunctions.hxx>

#include <vigra/numerictraits.hxx>
#include <vigra/metaprogramming.hxx>

namespace basebmp
{



/** Lookup index value for given color value in a PaletteImageAccessor
 */
template< class Accessor > struct ColorLookup
{
    typename Accessor::data_type operator()( const Accessor&               acc,
                                             typename Accessor::value_type v ) const
    {
        return acc.lookup(v);
    }
};



// partial specialization of AccessorTraits for PaletteAccessor
template< class Accessor, typename ColorType > struct AccessorTraits<
    PaletteImageAccessor< Accessor, ColorType > >
{
    /// value type of described accessor
    typedef typename PaletteImageAccessor< Accessor, ColorType >::value_type  value_type;

    /// Retrieve stand-alone color lookup function for given Accessor type
    typedef ColorLookup< PaletteImageAccessor< Accessor, ColorType > >        color_lookup;

    /// Retrieve raw pixel data accessor for given Accessor type
    typedef Accessor                                                          raw_accessor;

    /** accessor for XOR setter access is disabled, since the results
     *  are usually completely unintended - you'll usually want to
     *  wrap an xor_accessor with a PaletteAccessor, not the other way
     *  around.
     */
    typedef vigra::VigraFalseType                                             xor_accessor;

    /** accessor for masked setter access is disabled, since the
     *  results are usually completely unintended - you'll usually
     *  want to wrap a masked_accessor with a PaletteAccessor, not the
     *  other way around.
     */
    template< class MaskAccessor,
              class Iterator,
              class MaskIterator > struct                                     masked_accessor
    {
        typedef vigra::VigraFalseType type;
    };
};



template< typename ColorType > struct PaletteAccessorSelector
{
    template< class Accessor > struct wrap_accessor
    {
        typedef PaletteImageAccessor< Accessor, ColorType > type;
    };
};



template< class Iterator,
          class Accessor > struct PixelFormatTraitsTemplate_Palette
{
    typedef typename Iterator::value_type       pixel_type;
    typedef Iterator                            iterator_type;
    typedef Accessor                            raw_accessor_type;
    typedef PaletteAccessorSelector<Color>      accessor_selector;
};

template< int BitsPerPixel,
          bool MsbFirst > struct PixelFormatTraitsTemplate_PackedPalette :
    public PixelFormatTraitsTemplate_Palette<
               PackedPixelIterator< sal_uInt8,
                                    BitsPerPixel,
                                    MsbFirst >,
               NonStandardAccessor< sal_uInt8 > >
{};



// 1bpp MSB
typedef PixelFormatTraitsTemplate_PackedPalette<1, true>  PixelFormatTraits_PAL1_MSB;

// 1bpp LSB
typedef PixelFormatTraitsTemplate_PackedPalette<1, false> PixelFormatTraits_PAL1_LSB;

// 4bpp MSB
typedef PixelFormatTraitsTemplate_PackedPalette<4, true>  PixelFormatTraits_PAL4_MSB;

// 4bpp LSB
typedef PixelFormatTraitsTemplate_PackedPalette<4, false> PixelFormatTraits_PAL4_LSB;

// 8bpp
typedef PixelFormatTraitsTemplate_Palette<
    PixelIterator< sal_uInt8 >,
    StandardAccessor< sal_uInt8 > >                       PixelFormatTraits_PAL8;

} // namespace basebmp

#endif // INCLUDED_BASEBMP_INC_PALETTEFORMATS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
