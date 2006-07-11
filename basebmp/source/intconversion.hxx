/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: intconversion.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: thb $ $Date: 2006-07-11 11:38:56 $
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
