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

#include "cppunit/TestAssert.h"
#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"

#include <basegfx/vector/b2isize.hxx>
#include <basegfx/point/b2ipoint.hxx>

#include <basebmp/color.hxx>
#include <basebmp/scanlineformats.hxx>
#include <basebmp/bitmapdevice.hxx>
#include "tools.hxx"

using namespace ::basebmp;

namespace
{
class LineTest : public CppUnit::TestFixture
{
private:
    BitmapDeviceSharedPtr mpDevice1bpp;
    BitmapDeviceSharedPtr mpDevice32bpp;

    void implTestBasicDiagonalLines(const BitmapDeviceSharedPtr& rDevice)
    {
        rDevice->clear(Color(0));

        const basegfx::B2IPoint aPt1(1,1);
        const basegfx::B2IPoint aPt2(9,9);
        const Color aCol(0xFFFFFFFF);
        rDevice->drawLine( aPt1, aPt2, aCol, DrawMode_PAINT );
        CPPUNIT_ASSERT_MESSAGE("first pixel set",
                               rDevice->getPixel(aPt1) == aCol);
        CPPUNIT_ASSERT_MESSAGE("last pixel set",
                               rDevice->getPixel(aPt2) == aCol);
        const basegfx::B2IPoint aPt3(0,0);
        CPPUNIT_ASSERT_MESSAGE("topmost pixel not set",
                               rDevice->getPixel(aPt3) != aCol);
        const basegfx::B2IPoint aPt4(10,10);
        CPPUNIT_ASSERT_MESSAGE("bottommost pixel not set",
                               rDevice->getPixel(aPt4) != aCol);

        CPPUNIT_ASSERT_MESSAGE("number of rendered pixel is not 9",
                               countPixel( rDevice, aCol ) == 9);

        rDevice->drawLine( aPt2, aPt1, aCol, DrawMode_PAINT );

        CPPUNIT_ASSERT_MESSAGE("number of rendered pixel after "
                               "reversed paint is not 9",
                               countPixel( rDevice, aCol ) == 9);
    }

    void implTestBasicHorizontalLines(const BitmapDeviceSharedPtr& rDevice)
    {
        rDevice->clear(Color(0));

        const basegfx::B2IPoint aPt1(10,10);
        const basegfx::B2IPoint aPt2(0,10);
        const Color aCol(0xFFFFFFFF);
        rDevice->drawLine( aPt1, aPt2, aCol, DrawMode_PAINT );
        CPPUNIT_ASSERT_MESSAGE("first pixel set",
                               rDevice->getPixel(aPt1) == aCol);
        CPPUNIT_ASSERT_MESSAGE("last pixel set",
                               rDevice->getPixel(aPt2) == aCol);
        CPPUNIT_ASSERT_MESSAGE("number of rendered pixel is not 11",
                               countPixel( rDevice, aCol ) == 11);

        rDevice->clear(Color(0));
        rDevice->drawLine( aPt2, aPt1, aCol, DrawMode_PAINT );
        CPPUNIT_ASSERT_MESSAGE("first pixel set",
                               rDevice->getPixel(aPt1) == aCol);
        CPPUNIT_ASSERT_MESSAGE("last pixel set",
                               rDevice->getPixel(aPt2) == aCol);
        CPPUNIT_ASSERT_MESSAGE("number of rendered pixel is not 11",
                               countPixel( rDevice, aCol ) == 11);
    }

    void implTestBasicVerticalLines(const BitmapDeviceSharedPtr& rDevice)
    {
        rDevice->clear(Color(0));

        const basegfx::B2IPoint aPt1(1,1);
        const basegfx::B2IPoint aPt2(1,9);
        const Color aCol(0xFFFFFFFF);
        rDevice->drawLine( aPt1, aPt2, aCol, DrawMode_PAINT );
        CPPUNIT_ASSERT_MESSAGE("first pixel set",
                               rDevice->getPixel(aPt1) == aCol);
        CPPUNIT_ASSERT_MESSAGE("last pixel set",
                               rDevice->getPixel(aPt2) == aCol);
        const basegfx::B2IPoint aPt3(0,0);
        CPPUNIT_ASSERT_MESSAGE("topmost pixel not set",
                               rDevice->getPixel(aPt3) != aCol);
        const basegfx::B2IPoint aPt4(0,10);
        CPPUNIT_ASSERT_MESSAGE("bottommost pixel not set",
                               rDevice->getPixel(aPt4) != aCol);

        CPPUNIT_ASSERT_MESSAGE("number of rendered pixel is not 9",
                               countPixel( rDevice, aCol ) == 9);
    }

    // test pixel rounding (should always tend towards start point of
    // the line)
    void implTestTieBreaking(const BitmapDeviceSharedPtr& rDevice)
    {
        rDevice->clear(Color(0));

        const basegfx::B2IPoint aPt1(1,1);
        const basegfx::B2IPoint aPt2(3,2);
        const Color aCol(0xFFFFFFFF);
        rDevice->drawLine( aPt1, aPt2, aCol, DrawMode_PAINT );
        CPPUNIT_ASSERT_MESSAGE("first pixel set",
                               rDevice->getPixel(aPt1) == aCol);
        CPPUNIT_ASSERT_MESSAGE("second pixel set",
                               rDevice->getPixel(basegfx::B2IPoint(2,1)) == aCol);
        CPPUNIT_ASSERT_MESSAGE("last pixel set",
                               rDevice->getPixel(aPt2) == aCol);
        CPPUNIT_ASSERT_MESSAGE("number of rendered pixel after "
                               "reversed paint is not 3",
                               countPixel( rDevice, aCol ) == 3);

        rDevice->drawLine( aPt2, aPt1, aCol, DrawMode_PAINT );
        CPPUNIT_ASSERT_MESSAGE("alternate second pixel set",
                               rDevice->getPixel(basegfx::B2IPoint(2,2)) == aCol);

        CPPUNIT_ASSERT_MESSAGE("number of rendered pixel after "
                               "reversed paint is not 4",
                               countPixel( rDevice, aCol ) == 4);
    }

public:
    void setUp()
    {
        const basegfx::B2ISize aSize(11,11);
        mpDevice1bpp = createBitmapDevice( aSize,
                                           true,
                                           Format::ONE_BIT_MSB_PAL );
        mpDevice32bpp = createBitmapDevice( aSize,
                                           true,
                                           Format::THIRTYTWO_BIT_TC_MASK_BGRA );
    }

    void testCornerCases()
    {
        const basegfx::B2ISize aSize(1,1);
        BitmapDeviceSharedPtr pDevice = createBitmapDevice(
            aSize,
            true,
            Format::ONE_BIT_MSB_PAL );

        const basegfx::B2IPoint aPt1(0,0);
        const basegfx::B2IPoint aPt2(10,10);
        CPPUNIT_ASSERT_MESSAGE("only pixel cleared",
                                pDevice->getPixelData(aPt1) == 0);

        const Color aCol(0xFFFFFFFF);
        pDevice->drawLine( aPt1, aPt2, aCol, DrawMode_PAINT );
        CPPUNIT_ASSERT_MESSAGE("only pixel set",
                               pDevice->getPixelData(aPt1) == 1);

        const basegfx::B2ISize aSize2(1,0);
        pDevice = createBitmapDevice(
            aSize2,
            true,
            Format::ONE_BIT_MSB_PAL );

        CPPUNIT_ASSERT_MESSAGE("only pixel cleared",
                                pDevice->getPixelData(aPt1) == 0);

        pDevice->drawLine( aPt1, aPt2, aCol, DrawMode_PAINT );
        CPPUNIT_ASSERT_MESSAGE("only pixel still cleared",
                               pDevice->getPixelData(aPt1) == 0);
    }

    void testBasicDiagonalLines()
    {
        implTestBasicDiagonalLines( mpDevice1bpp );
        implTestBasicDiagonalLines( mpDevice32bpp );
    }

    void testBasicHorizontalLines()
    {
        implTestBasicHorizontalLines( mpDevice1bpp );
        implTestBasicHorizontalLines( mpDevice32bpp );
    }

    void testBasicVerticalLines()
    {
        implTestBasicVerticalLines( mpDevice1bpp );
        implTestBasicVerticalLines( mpDevice32bpp );
    }

    // test pixel rounding (should always tend towards start point of
    // the line)
    void testTieBreaking()
    {
        implTestTieBreaking( mpDevice1bpp );
        implTestTieBreaking( mpDevice32bpp );
    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(LineTest);
    CPPUNIT_TEST(testCornerCases);
    CPPUNIT_TEST(testBasicDiagonalLines);
    CPPUNIT_TEST(testBasicHorizontalLines);
    CPPUNIT_TEST(testBasicVerticalLines);
    CPPUNIT_TEST(testTieBreaking);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(LineTest);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
