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
                                                           Format::OneBitMsbPal ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("right size",
                               aSize2, pDevice->getSize() );
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
        BitmapDeviceSharedPtr  pDevice( createBitmapDevice( aSize,
                                                            Format::OneBitMsbPal ) );

        BitmapDeviceSharedPtr  pClone( cloneBitmapDevice(
                                           pDevice ));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("right size",
                               aSize, pClone->getSize() );
    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(BasicTest);
    CPPUNIT_TEST(colorTest);
    CPPUNIT_TEST(testConstruction);
    CPPUNIT_TEST(testClone);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(BasicTest);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
