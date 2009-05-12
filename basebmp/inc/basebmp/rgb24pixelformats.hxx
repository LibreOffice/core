/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: rgb24pixelformats.hxx,v $
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

#ifndef INCLUDED_BASEBMP_RGB24PIXELFORMATS_HXX
#define INCLUDED_BASEBMP_RGB24PIXELFORMATS_HXX

#include <basebmp/color.hxx>
#include <basebmp/accessor.hxx>
#include <basebmp/pixeliterator.hxx>
#include <basebmp/pixelformatadapters.hxx>

#include <vigra/rgbvalue.hxx>

#include <functional>

namespace basebmp
{

template< typename PixelType, typename ColorType > struct RGBValueGetter :
        public std::unary_function<PixelType, ColorType>
{
    ColorType operator()( PixelType const& c ) const
    {
        return ColorType(c.red(),c.green(),c.blue());
    }
};

template< typename PixelType, typename ColorType > struct RGBValueSetter :
    public std::unary_function<ColorType, PixelType>
{
    PixelType operator()( ColorType const& c ) const
    {
        PixelType res;
        res.setRed(c.getRed());
        res.setGreen(c.getGreen());
        res.setBlue(c.getBlue());
        return res;
    }
};

//-----------------------------------------------------------------------------

template< typename PixelType > struct PixelFormatTraitsTemplate_RGBValue
{
    typedef PixelType                     pixel_type;

    typedef RGBValueGetter<pixel_type,
                           Color>         getter_type;
    typedef RGBValueSetter<pixel_type,
                           Color>         setter_type;

    typedef PixelIterator<pixel_type>     iterator_type;
    typedef StandardAccessor<pixel_type>  raw_accessor_type;
    typedef AccessorSelector<
        getter_type, setter_type>         accessor_selector;
};

//-----------------------------------------------------------------------------

// 24bpp RGB
typedef PixelFormatTraitsTemplate_RGBValue<
    vigra::RGBValue<sal_uInt8> >            PixelFormatTraits_RGB24;
BASEBMP_SPECIALIZE_ACCESSORTRAITS(PixelFormatTraits_RGB24::getter_type,
                                  PixelFormatTraits_RGB24::setter_type);

// 24bpp BGR
typedef PixelFormatTraitsTemplate_RGBValue<
    vigra::RGBValue<sal_uInt8,2,1,0> >      PixelFormatTraits_BGR24;
BASEBMP_SPECIALIZE_ACCESSORTRAITS(PixelFormatTraits_BGR24::getter_type,
                                  PixelFormatTraits_BGR24::setter_type);

} // namespace basebmp

#endif /* INCLUDED_BASEBMP_RGB24PIXELFORMATS_HXX */
