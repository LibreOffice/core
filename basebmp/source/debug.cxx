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
    const char* formatName( Format nScanlineFormat )
    {
        switch( nScanlineFormat )
        {
        case FORMAT_NONE:
            return "NONE";
        case FORMAT_ONE_BIT_MSB_GREY:
            return "ONE_BIT_MSB_GREY";
        case FORMAT_ONE_BIT_LSB_GREY:
            return "ONE_BIT_LSB_GREY";
        case FORMAT_ONE_BIT_MSB_PAL:
            return "ONE_BIT_MSB_PAL";
        case FORMAT_ONE_BIT_LSB_PAL:
            return "ONE_BIT_LSB_PAL";
        case FORMAT_FOUR_BIT_MSB_GREY:
            return "FOUR_BIT_MSB_GREY";
        case FORMAT_FOUR_BIT_LSB_GREY:
            return "FOUR_BIT_LSB_GREY";
        case FORMAT_FOUR_BIT_MSB_PAL:
            return "FOUR_BIT_MSB_PAL";
        case FORMAT_FOUR_BIT_LSB_PAL:
            return "FOUR_BIT_LSB_PAL";
        case FORMAT_EIGHT_BIT_PAL:
            return "EIGHT_BIT_PAL";
        case FORMAT_EIGHT_BIT_GREY:
            return "EIGHT_BIT_GREY";
        case FORMAT_SIXTEEN_BIT_LSB_TC_MASK:
            return "SIXTEEN_BIT_LSB_TC_MASK";
        case FORMAT_SIXTEEN_BIT_MSB_TC_MASK:
            return "SIXTEEN_BIT_MSB_TC_MASK";
        case FORMAT_TWENTYFOUR_BIT_TC_MASK:
            return "TWENTYFOUR_BIT_TC_MASK";
        case FORMAT_THIRTYTWO_BIT_TC_MASK_BGRA:
            return "THIRTYTWO_BIT_TC_MASK_BGRA";
        case FORMAT_THIRTYTWO_BIT_TC_MASK_ARGB:
            return "THIRTYTWO_BIT_TC_MASK_ARGB";
        case FORMAT_THIRTYTWO_BIT_TC_MASK_ABGR:
            return "THIRTYTWO_BIT_TC_MASK_ABGR";
        case FORMAT_THIRTYTWO_BIT_TC_MASK_RGBA:
            return "THIRTYTWO_BIT_TC_MASK_RGBA";
        default:
            return "<unknown>";
        }
    }

#if OSL_DEBUG_LEVEL > 2

    void debugDump( const BitmapDeviceSharedPtr& rDevice,
                    std::ostream&                rOutputStream )
    {
        const basegfx::B2IVector aSize( rDevice->getSize() );
        const bool               bTopDown( rDevice->isTopDown() );
        const Format             nScanlineFormat( rDevice->getScanlineFormat() );

        rOutputStream
            << "/* basebmp::BitmapDevice content dump */" << std::endl
            << "/* Width   = " << aSize.getX() << " */" << std::endl
            << "/* Height  = " << aSize.getY() << " */" << std::endl
            << "/* TopDown = " << bTopDown << " */" << std::endl
            << "/* Format  = " << formatName(nScanlineFormat) << " */" << std::endl
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

#endif // OSL_DEBUG_LEVEL > 2

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
