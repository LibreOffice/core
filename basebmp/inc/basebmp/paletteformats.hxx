/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: paletteformats.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: thb $ $Date: 2006-07-12 22:47:20 $
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

#ifndef INCLUDED_BASEBMP_PACKEDPIXELFORMATS_HXX
#define INCLUDED_BASEBMP_PACKEDPIXELFORMATS_HXX

#include <basebmp/color.hxx>
#include <basebmp/colortraits.hxx>
#include <basebmp/accessor.hxx>
#include <basebmp/pixeliterator.hxx>
#include <basebmp/packedpixeliterator.hxx>
#include <basebmp/pixelformatadapters.hxx>
#include <basebmp/paletteimageaccessor.hxx>
#include <basebmp/metafunctions.hxx>

#include <vigra/numerictraits.hxx>
#include <vigra/metaprogramming.hxx>

#include <functional>

namespace basebmp
{

//-----------------------------------------------------------------------------

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

//-----------------------------------------------------------------------------

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

//-----------------------------------------------------------------------------

template< typename ColorType > struct PaletteAccessorSelector
{
    template< class Accessor > struct wrap_accessor
    {
        typedef PaletteImageAccessor< Accessor, ColorType > type;
    };
};

//-----------------------------------------------------------------------------

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

//-----------------------------------------------------------------------------

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

#endif /* INCLUDED_BASEBMP_PACKEDPIXELFORMATS_HXX */
