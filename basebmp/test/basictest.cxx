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

#include <sal/config.h>

#include <iomanip>
#include <ios>
#include <ostream>
#include <type_traits>

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <basegfx/vector/b2isize.hxx>
#include <basegfx/point/b2ipoint.hxx>

#include <basebmp/color.hxx>
#include <basebmp/scanlineformats.hxx>
#include <basebmp/bitmapdevice.hxx>
#include "tools.hxx"

using namespace ::basebmp;

namespace basebmp {

template<typename charT, typename traits>
std::basic_ostream<charT, traits> & operator <<(
    std::basic_ostream<charT, traits> & stream, Color const & value)
{
    auto f = stream.flags();
    return stream
        << std::hex << std::uppercase << std::setw(8) << std::setfill('0')
        << value.toInt32() << std::setiosflags(f);
}

template<typename charT, typename traits>
std::basic_ostream<charT, traits> & operator <<(
    std::basic_ostream<charT, traits> & stream, Format value)
{
     return stream
         << static_cast<typename std::underlying_type<Format>::type>(value);
}

}

namespace
{

class BasicTest : public CppUnit::TestFixture
{
public:
    void colorTest()
    {
        Color aTestColor;

        aTestColor = Color(0xDEADBEEF);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("unary constructor",
                               sal_uInt32(0xDEADBEEF), aTestColor.toInt32() );

        aTestColor = Color( 0x10, 0x20, 0xFF );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("ternary constructor",
                               sal_uInt32(0x001020FF), aTestColor.toInt32() );

        aTestColor.setRed( 0x0F );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("setRed()",
                               sal_uInt32(0x00F20FF), aTestColor.toInt32() );

        aTestColor.setGreen( 0x0F );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("setGreen()",
                               sal_uInt32(0x00F0FFF), aTestColor.toInt32() );

        aTestColor.setBlue( 0x10 );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("setBlue()",
                               sal_uInt32(0x00F0F10), aTestColor.toInt32() );

        aTestColor.setGrey( 0x13 );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("setGrey()",
                               sal_uInt32(0x00131313), aTestColor.toInt32() );

        aTestColor = Color( 0x10, 0x20, 0xFF );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("getRed()",
                               sal_uInt8(0x10), aTestColor.getRed() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("getGreen()",
                               sal_uInt8(0x20), aTestColor.getGreen() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("getBlue()",
                               sal_uInt8(0xFF), aTestColor.getBlue() );

    }

    void testConstruction()
    {
        const basegfx::B2ISize aSize(101,101);
        basegfx::B2ISize       aSize2(aSize);
        BitmapDeviceSharedPtr pDevice( createBitmapDevice( aSize,
                                                           true,
                                                           Format::OneBitMsbPal ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("right size",
                               aSize2, pDevice->getSize() );
        CPPUNIT_ASSERT_MESSAGE("Top down format",
                               pDevice->isTopDown() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Scanline format",
                               Format::OneBitMsbPal, pDevice->getScanlineFormat() );
        sal_Int32 nExpectedStride = (aSize2.getY() + 7)/8;
        sal_Int32 nAlign = sizeof(sal_uInt32);
        nExpectedStride = ((nExpectedStride + nAlign-1) / nAlign) * nAlign;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Scanline len",
                               nExpectedStride, pDevice->getScanlineStride() );
        CPPUNIT_ASSERT_MESSAGE("Palette existence",
                               pDevice->getPalette() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Palette entry 0 is black",
                               Color(0), (*pDevice->getPalette())[0] );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Palette entry 1 is white",
                               Color(0xFFFFFFFF), (*pDevice->getPalette())[1] );
    }

    void testClone()
    {
        const basegfx::B2ISize aSize(101,101);
        basegfx::B2ISize       aSize2(3,3);
        BitmapDeviceSharedPtr  pDevice( createBitmapDevice( aSize,
                                                            true,
                                                            Format::OneBitMsbPal ) );

        BitmapDeviceSharedPtr  pClone( cloneBitmapDevice(
                                           aSize2,
                                           pDevice ));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("right size",
                               aSize2, pClone->getSize() );
    }

    void testPixelFuncs()
    {
        // 1bpp
        const basegfx::B2ISize aSize(64,64);
        BitmapDeviceSharedPtr pDevice( createBitmapDevice( aSize,
                                                           true,
                                                           Format::OneBitMsbPal ) );

        const basegfx::B2IPoint aPt(3,3);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("getPixelData for virgin device",
                               sal_uInt32(0), pDevice->getPixelData(aPt));

        const Color aCol(0xFFFFFFFF);
        pDevice->setPixel( aPt, aCol, DrawMode::Paint );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("get/setPixel roundtrip #1",
                               aCol, pDevice->getPixel(aPt));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("getPixelData for white pixel",
                               sal_uInt32(1), pDevice->getPixelData(aPt));

        const basegfx::B2IPoint aPt2(0,0);
        const Color aCol2(0xFFFFFFFF);
        pDevice->setPixel( aPt2, aCol2, DrawMode::Paint );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("get/setPixel roundtrip #2",
                               aCol2, pDevice->getPixel(aPt2));

        const basegfx::B2IPoint aPt3(aSize.getX()-1,aSize.getY()-1);
        const Color aCol3(0x00000000);
        pDevice->setPixel( aPt3, aCol3, DrawMode::Paint );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("get/setPixel roundtrip #3",
                               aCol3, pDevice->getPixel(aPt3));

        pDevice->setPixel( aPt3, aCol2, DrawMode::Paint );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("get/setPixel roundtrip #3.5",
                               aCol2, pDevice->getPixel(aPt3));

        const basegfx::B2IPoint aPt4(-100000,-100000);
        pDevice->setPixel( aPt4, aCol3, DrawMode::Paint );
        const basegfx::B2IPoint aPt5(100000,100000);
        pDevice->setPixel( aPt5, aCol3, DrawMode::Paint );

        auto nPixel(countPixel(pDevice, aCol2));
        const basegfx::B2IPoint aPt6(aSize.getX(),aSize.getY());
        pDevice->setPixel( aPt6, aCol2, DrawMode::Paint );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("setPixel clipping",
                               nPixel, countPixel(pDevice, aCol2));

        CPPUNIT_ASSERT_EQUAL_MESSAGE("raw pixel value #1",
                               sal_uInt8(0x80), pDevice->getBuffer()[0]);

        // 1bit LSB
        {
            pDevice = createBitmapDevice( aSize,
                                          true,
                                          Format::OneBitLsbPal );

            pDevice->setPixel( aPt2, aCol, DrawMode::Paint );
            CPPUNIT_ASSERT_EQUAL_MESSAGE("get/setPixel roundtrip #4",
                                   aCol, pDevice->getPixel(aPt2));

            const basegfx::B2IPoint aPt222(1,1);
            pDevice->setPixel( aPt222, aCol, DrawMode::Paint );
            CPPUNIT_ASSERT_EQUAL_MESSAGE("get/setPixel roundtrip #5",
                                   aCol, pDevice->getPixel(aPt222));

            pDevice->setPixel( aPt3, aCol, DrawMode::Paint );
            CPPUNIT_ASSERT_EQUAL_MESSAGE("get/setPixel roundtrip #6",
                                   aCol, pDevice->getPixel(aPt3));

            CPPUNIT_ASSERT_EQUAL_MESSAGE("raw pixel value #2",
                                   sal_uInt8(0x01), pDevice->getBuffer()[0]);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("raw pixel value #3",
                                   sal_uInt8(0x02), pDevice->getBuffer()[8]);
        }

        // 8bit alpha
        {
            pDevice = createBitmapDevice( aSize,
                                          true,
                                          Format::EightBitGrey );

            const Color aCol4(0x010101);
            pDevice->setPixel( aPt, aCol4, DrawMode::Paint );
            CPPUNIT_ASSERT_EQUAL_MESSAGE("get/setPixel roundtrip #4",
                                   aCol4, pDevice->getPixel(aPt));

            const Color aCol5(0x0F0F0F);
            pDevice->setPixel( aPt2, aCol5, DrawMode::Paint );
            CPPUNIT_ASSERT_EQUAL_MESSAGE("get/setPixel roundtrip #5",
                                   aCol5, pDevice->getPixel(aPt2));

            const Color aCol6(0xFFFFFF);
            pDevice->setPixel( aPt3, aCol6, DrawMode::Paint );
            CPPUNIT_ASSERT_EQUAL_MESSAGE("get/setPixel roundtrip #6",
                                   aCol6, pDevice->getPixel(aPt3));
        }

        // 16bpp
        {
            pDevice = createBitmapDevice( aSize,
                                          true,
                                          Format::SixteenBitLsbTcMask );
            const Color aCol7(0);
            pDevice->clear( aCol7 );

            const Color aCol4(0x00101010);
            pDevice->setPixel( aPt, aCol4, DrawMode::Paint );
            CPPUNIT_ASSERT_EQUAL_MESSAGE("get/setPixel roundtrip #7",
                                   aCol4, pDevice->getPixel(aPt));

            const Color aCol5(0x00F0F0F0);
            pDevice->setPixel( aPt2, aCol5, DrawMode::Paint );
            CPPUNIT_ASSERT_MESSAGE("get/setPixel roundtrip #8",
                                   pDevice->getPixel(aPt2) != aCol7);

            const Color aCol6(0x00FFFFFF);
            pDevice->setPixel( aPt3, aCol6, DrawMode::Paint );
            CPPUNIT_ASSERT_EQUAL_MESSAGE("get/setPixel roundtrip #9",
                                   aCol6, pDevice->getPixel(aPt3));
        }

        // 24bpp
        {
            pDevice = createBitmapDevice( aSize,
                                          true,
                                          Format::TwentyFourBitTcMask );

            const Color aCol4(0x01010101);
            pDevice->setPixel( aPt, aCol4, DrawMode::Paint );
            CPPUNIT_ASSERT_EQUAL_MESSAGE("get/setPixel roundtrip #10",
                                   aCol4, pDevice->getPixel(aPt));

            const Color aCol5(0x0F3F2F1F);
            pDevice->setPixel( aPt2, aCol5, DrawMode::Paint );
            CPPUNIT_ASSERT_EQUAL_MESSAGE("get/setPixel roundtrip #11",
                                   aCol5, pDevice->getPixel(aPt2));

            const Color aCol6(0xFFFFFFFF);
            pDevice->setPixel( aPt3, aCol6, DrawMode::Paint );
            CPPUNIT_ASSERT_EQUAL_MESSAGE("get/setPixel roundtrip #12",
                                   aCol6, pDevice->getPixel(aPt3));

            CPPUNIT_ASSERT_EQUAL_MESSAGE("raw pixel value #4",
                                   sal_uInt8(0x3F), pDevice->getBuffer()[2]);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("raw pixel value #4",
                                   sal_uInt8(0x2F), pDevice->getBuffer()[1]);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("raw pixel value #4",
                                   sal_uInt8(0x1F), pDevice->getBuffer()[0]);
        }

        // 32bpp
        {
            pDevice = createBitmapDevice( aSize,
                                          true,
                                          Format::ThirtyTwoBitTcMaskBGRA );

            const Color aCol4(0x01010101);
            pDevice->setPixel( aPt, aCol4, DrawMode::Paint );
            CPPUNIT_ASSERT_EQUAL_MESSAGE("get/setPixel roundtrip #13",
                                   aCol4, pDevice->getPixel(aPt));

            const Color aCol5(0x0F0F0F0F);
            pDevice->setPixel( aPt2, aCol5, DrawMode::Paint );
            CPPUNIT_ASSERT_EQUAL_MESSAGE("get/setPixel roundtrip #14",
                                   aCol5, pDevice->getPixel(aPt2));

            const Color aCol6(0xFFFFFFFF);
            pDevice->setPixel( aPt3, aCol6, DrawMode::Paint );
            CPPUNIT_ASSERT_EQUAL_MESSAGE("get/setPixel roundtrip #15",
                                   aCol6, pDevice->getPixel(aPt3));
        }
    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(BasicTest);
    CPPUNIT_TEST(colorTest);
    CPPUNIT_TEST(testConstruction);
    CPPUNIT_TEST(testClone);
    CPPUNIT_TEST(testPixelFuncs);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(BasicTest);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
