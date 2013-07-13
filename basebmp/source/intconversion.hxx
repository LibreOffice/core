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

#ifndef INCLUDED_BASEBMP_INTCONVERSION_HXX
#define INCLUDED_BASEBMP_INTCONVERSION_HXX

#include <vigra/rgbvalue.hxx>
#include <functional>

namespace basebmp
{
    // metafunctions to retrieve correct POD from/to basebmp::Color

    /// type-safe conversion from RgbValue to packed int32
    template< class RgbVal > struct UInt32FromRgbValue
    {
        sal_uInt32 operator()( RgbVal const& c ) const
        {
            return (c[0] << 16) | (c[1] << 8) | c[2];
        }
    };

    /// type-safe conversion from packed int32 to RgbValue
    template< class RgbVal > struct RgbValueFromUInt32
    {
        RgbVal operator()( sal_uInt32 c ) const
        {
            return RgbVal((c >> 16) & 0xFF,
                          (c >> 8) & 0xFF,
                          c & 0xFF);
        }
    };

    //Current c++0x draft (apparently) has std::identity, but not operator()
    template<typename T> struct SGI_identity : public std::unary_function<T,T>
    {
        T& operator()(T& x) const { return x; }
        const T& operator()(const T& x) const { return x; }
    };

    /// Get converter from given data type to sal_uInt32
    template< typename DataType > struct uInt32Converter
    {
        typedef SGI_identity<DataType> to;
        typedef SGI_identity<DataType> from;
    };
    template< unsigned int RedIndex,
              unsigned int GreenIndex,
              unsigned int BlueIndex > struct uInt32Converter<
                  vigra::RGBValue< sal_uInt8,
                                   RedIndex,
                                   GreenIndex,
                                   BlueIndex > >
    {
        typedef UInt32FromRgbValue<
            vigra::RGBValue< sal_uInt8,
                             RedIndex,
                             GreenIndex,
                             BlueIndex > >
            to;
        typedef RgbValueFromUInt32<
            vigra::RGBValue< sal_uInt8,
                             RedIndex,
                             GreenIndex,
                             BlueIndex > >
            from;
    };
}

#endif /* INCLUDED_BASEBMP_INTCONVERSION_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
