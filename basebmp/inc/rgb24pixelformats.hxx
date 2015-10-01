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

#ifndef INCLUDED_BASEBMP_INC_RGB24PIXELFORMATS_HXX
#define INCLUDED_BASEBMP_INC_RGB24PIXELFORMATS_HXX

#include <basebmp/color.hxx>
#include <accessor.hxx>
#include <pixeliterator.hxx>
#include <pixelformatadapters.hxx>

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

#endif /* INCLUDED_BASEBMP_INC_RGB24PIXELFORMATS_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
