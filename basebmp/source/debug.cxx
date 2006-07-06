/*************************************************************************
 *
 *  $RCSfile: debug.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: thb $ $Date: 2006-07-06 10:00:41 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
                case Format::ONE_BIT_MSB_GRAY:
                    return "ONE_BIT_MSB_GRAY";
                case Format::ONE_BIT_LSB_GRAY:
                    return "ONE_BIT_LSB_GRAY";
                case Format::ONE_BIT_MSB_PAL:
                    return "ONE_BIT_MSB_PAL";
                case Format::ONE_BIT_LSB_PAL:
                    return "ONE_BIT_LSB_PAL";
                case Format::FOUR_BIT_MSB_GRAY:
                    return "FOUR_BIT_MSB_GRAY";
                case Format::FOUR_BIT_LSB_GRAY:
                    return "FOUR_BIT_LSB_GRAY";
                case Format::FOUR_BIT_MSB_PAL:
                    return "FOUR_BIT_MSB_PAL";
                case Format::FOUR_BIT_LSB_PAL:
                    return "FOUR_BIT_LSB_PAL";
                case Format::EIGHT_BIT_PAL:
                    return "EIGHT_BIT_PAL";
                case Format::EIGHT_BIT_GRAY:
                    return "EIGHT_BIT_GRAY";
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
