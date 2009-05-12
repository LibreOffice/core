/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: intconversion.hxx,v $
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

#ifndef INCLUDED_BASEBMP_INTCONVERSION_HXX
#define INCLUDED_BASEBMP_INTCONVERSION_HXX

#include <vigra/rgbvalue.hxx>
#include <functional>

namespace basebmp
{
    // metafunctions to retrieve correct POD from/to basebmp::Color
    //------------------------------------------------------------------------

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

    /// Get converter from given data type to sal_uInt32
    template< typename DataType > struct uInt32Converter
    {
        typedef std::identity<DataType> to;
        typedef std::identity<DataType> from;
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
