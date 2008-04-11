/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: debug.cxx,v $
 * $Revision: 1.8 $
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

#include <osl/diagnose.h>

#include <basegfx/point/b2ipoint.hxx>
#include <basegfx/vector/b2ivector.hxx>

#include <basebmp/scanlineformats.hxx>
#include <basebmp/color.hxx>
#include <basebmp/bitmapdevice.hxx>
#include <basebmp/debug.hxx>

#include <iomanip>

namespace basebmp
{
    namespace
    {
        static const char* getFormatString( sal_Int32 nScanlineFormat )
        {
            switch( nScanlineFormat )
            {
                case Format::ONE_BIT_MSB_GREY:
                    return "ONE_BIT_MSB_GREY";
                case Format::ONE_BIT_LSB_GREY:
                    return "ONE_BIT_LSB_GREY";
                case Format::ONE_BIT_MSB_PAL:
                    return "ONE_BIT_MSB_PAL";
                case Format::ONE_BIT_LSB_PAL:
                    return "ONE_BIT_LSB_PAL";
                case Format::FOUR_BIT_MSB_GREY:
                    return "FOUR_BIT_MSB_GREY";
                case Format::FOUR_BIT_LSB_GREY:
                    return "FOUR_BIT_LSB_GREY";
                case Format::FOUR_BIT_MSB_PAL:
                    return "FOUR_BIT_MSB_PAL";
                case Format::FOUR_BIT_LSB_PAL:
                    return "FOUR_BIT_LSB_PAL";
                case Format::EIGHT_BIT_PAL:
                    return "EIGHT_BIT_PAL";
                case Format::EIGHT_BIT_GREY:
                    return "EIGHT_BIT_GREY";
                case Format::SIXTEEN_BIT_LSB_TC_MASK:
                    return "SIXTEEN_BIT_LSB_TC_MASK";
                case Format::SIXTEEN_BIT_MSB_TC_MASK:
                    return "SIXTEEN_BIT_MSB_TC_MASK";
                case Format::TWENTYFOUR_BIT_TC_MASK:
                    return "TWENTYFOUR_BIT_TC_MASK";
                case Format::THIRTYTWO_BIT_TC_MASK:
                    return "THIRTYTWO_BIT_TC_MASK";
                default:
                    return "<unknown>";
            }
        }
    }

    void debugDump( const BitmapDeviceSharedPtr& rDevice,
                    std::ostream&                rOutputStream )
    {
        const basegfx::B2IVector aSize( rDevice->getSize() );
        const bool               bTopDown( rDevice->isTopDown() );
        const sal_Int32          nScanlineFormat( rDevice->getScanlineFormat() );

        rOutputStream
            << "/* basebmp::BitmapDevice content dump */" << std::endl
            << "/* Width   = " << aSize.getX() << " */" << std::endl
            << "/* Height  = " << aSize.getY() << " */" << std::endl
            << "/* TopDown = " << bTopDown << " */" << std::endl
            << "/* Format  = " << getFormatString(nScanlineFormat) << " */" << std::endl
            << "/* (dumped entries are already mapped RGBA color values) */" << std::endl
            << std::endl;

        rOutputStream << std::hex;
        for( int y=0; y<aSize.getY(); ++y )
        {
            for( int x=0; x<aSize.getX(); ++x )
                rOutputStream << std::setw(8) << (sal_uInt32)rDevice->getPixel( basegfx::B2IPoint(x,y) ).toInt32() << " ";
            rOutputStream << std::endl;
        }
    }
}
