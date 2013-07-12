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
#include <basegfx/range/b2ibox.hxx>
#include <basegfx/point/b2ipoint.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>

#include <basebmp/color.hxx>
#include <basebmp/scanlineformats.hxx>
#include <basebmp/bitmapdevice.hxx>
#include "tools.hxx"

using namespace ::basebmp;

namespace
{
class BmpTest : public CppUnit::TestFixture
{
private:
    BitmapDeviceSharedPtr mpDevice1bpp;
    BitmapDeviceSharedPtr mpBmp1bpp;
    BitmapDeviceSharedPtr mpDevice32bpp;
    BitmapDeviceSharedPtr mpBmp32bpp;

    void implTestBmpBasics(const BitmapDeviceSharedPtr& rDevice,
                           const BitmapDeviceSharedPtr& rBmp)
    {
        rDevice->clear(Color(0));
        const Color aCol(0xFFFFFFFF);

        const basegfx::B2IBox aSourceRect(0,0,10,10);
        const basegfx::B2IBox aDestLeftTop(0,0,4,4);
        const basegfx::B2IBox aDestRightTop(6,0,10,4);
        const basegfx::B2IBox aDestLeftBottom(0,6,4,10);
        const basegfx::B2IBox aDestRightBottom(6,6,10,10);

        rDevice->drawBitmap(
            rBmp,
            aSourceRect,
            aDestLeftTop,
            DrawMode_PAINT );
        CPPUNIT_ASSERT_MESSAGE("number of rendered pixel is not 8",
                               countPixel( rDevice, aCol ) == 8);

        rDevice->drawBitmap(
            rBmp,
            aSourceRect,
            aDestRightTop,
            DrawMode_PAINT );
        CPPUNIT_ASSERT_MESSAGE("number of rendered pixel is not 16",
                               countPixel( rDevice, aCol ) == 16);

        rDevice->drawBitmap(
            rBmp,
            aSourceRect,
            aDestLeftBottom,
            DrawMode_PAINT );
        CPPUNIT_ASSERT_MESSAGE("number of rendered pixel is not 24",
                               countPixel( rDevice, aCol ) == 24);

        rDevice->drawBitmap(
            rBmp,
            aSourceRect,
            aDestRightBottom,
            DrawMode_PAINT );
        CPPUNIT_ASSERT_MESSAGE("number of rendered pixel is not 32",
                               countPixel( rDevice, aCol ) == 32);

        BitmapDeviceSharedPtr pClone = subsetBitmapDevice(
            rBmp, aSourceRect );

        // two overlapping areas within the same memory block, check
        // if we clobber the mem or properly detect the case
        const basegfx::B2IBox aSourceOverlap(0,0,6,10);
        const basegfx::B2IBox aDestOverlap(3,0,9,10);
        rBmp->drawBitmap(
            pClone,
            aSourceOverlap,
            aDestOverlap,
            DrawMode_PAINT );
        CPPUNIT_ASSERT_MESSAGE("clobbertest - number of set pixel is not 50",
                               countPixel( rBmp, aCol ) == 50);

    }

    void implTestBmpClip(const BitmapDeviceSharedPtr& rDevice,
                         const BitmapDeviceSharedPtr& rBmp)
    {
        rDevice->clear(Color(0));
        const Color aCol(0xFFFFFFFF);

        const basegfx::B2IBox aSourceRect(0,0,10,10);
        const basegfx::B2IBox aDestLeftTop(-2,-2,2,2);
        const basegfx::B2IBox aDestRightTop(8,-2,12,2);
        const basegfx::B2IBox aDestLeftBottom(-2,8,2,12);
        const basegfx::B2IBox aDestRightBottom(8,8,12,12);

        rDevice->drawBitmap(
            rBmp,
            aSourceRect,
            aDestLeftTop,
            DrawMode_PAINT );
        CPPUNIT_ASSERT_MESSAGE("number of rendered pixel is not 4",
                               countPixel( rDevice, aCol ) == 4);

        rDevice->drawBitmap(
            rBmp,
            aSourceRect,
            aDestLeftBottom,
            DrawMode_PAINT );
        CPPUNIT_ASSERT_MESSAGE("number of rendered pixel is not 4(c)",
                               countPixel( rDevice, aCol ) == 4);

        rDevice->drawBitmap(
            rBmp,
            aSourceRect,
            aDestRightBottom,
            DrawMode_PAINT );
        CPPUNIT_ASSERT_MESSAGE("number of rendered pixel is not 8",
                               countPixel( rDevice, aCol ) == 8);
    }

public:
    void setUp()
    {
        const basegfx::B2ISize aSize(10,10);
        mpDevice1bpp = createBitmapDevice( aSize,
                                           true,
                                           Format::ONE_BIT_MSB_PAL );
        mpDevice32bpp = createBitmapDevice( aSize,
                                            true,
                                            Format::THIRTYTWO_BIT_TC_MASK_BGRA );

        mpBmp1bpp = createBitmapDevice( aSize,
                                        true,
                                        Format::ONE_BIT_MSB_PAL );
        mpBmp32bpp = createBitmapDevice( aSize,
                                         true,
                                         Format::THIRTYTWO_BIT_TC_MASK_BGRA );

        OUString aSvg( "m 0 0h5v10h5v-5h-10z" );

        basegfx::B2DPolyPolygon aPoly;
        basegfx::tools::importFromSvgD( aPoly, aSvg );
        const Color aCol(0xFFFFFFFF);
        mpBmp1bpp->fillPolyPolygon(
            aPoly,
            aCol,
            DrawMode_PAINT );
        mpBmp32bpp->fillPolyPolygon(
            aPoly,
            aCol,
            DrawMode_PAINT );
    }

    void testBmpBasics()
    {
        implTestBmpBasics( mpDevice1bpp, mpBmp1bpp );
        implTestBmpBasics( mpDevice32bpp, mpBmp32bpp );
    }

    void testBmpClip()
    {
        implTestBmpClip( mpDevice1bpp, mpBmp1bpp );
        implTestBmpClip( mpDevice32bpp, mpBmp32bpp );
    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(BmpTest);
    CPPUNIT_TEST(testBmpBasics);
    CPPUNIT_TEST(testBmpClip);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(BmpTest);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
