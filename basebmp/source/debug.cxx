/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
