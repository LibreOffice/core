/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: debug.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 13:47:41 $
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
