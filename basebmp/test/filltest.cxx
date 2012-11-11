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
#include <basegfx/range/b2drange.hxx>
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
class FillTest : public CppUnit::TestFixture
{
private:
    BitmapDeviceSharedPtr mpDevice1bpp;
    BitmapDeviceSharedPtr mpDevice32bpp;

    void implTestRectFill(const BitmapDeviceSharedPtr& rDevice)
    {
        rDevice->clear(Color(0));

        const basegfx::B2DRange   aRect(1,1,10,10);

        const Color aCol(0xFFFFFFFF);
        rDevice->fillPolyPolygon(
            basegfx::B2DPolyPolygon(
                basegfx::tools::createPolygonFromRect( aRect )),
            aCol,
            DrawMode_PAINT );

        const basegfx::B2IPoint aPt1(1,1);
        CPPUNIT_ASSERT_MESSAGE("first pixel set",
                               rDevice->getPixel(aPt1) == aCol);
        const basegfx::B2IPoint aPt2(9,9);
        CPPUNIT_ASSERT_MESSAGE("last pixel set",
                               rDevice->getPixel(aPt2) == aCol);
        const basegfx::B2IPoint aPt3(0,0);
        CPPUNIT_ASSERT_MESSAGE("topmost pixel not set",
                               rDevice->getPixel(aPt3) != aCol);
        const basegfx::B2IPoint aPt4(10,10);
        CPPUNIT_ASSERT_MESSAGE("bottommost pixel not set",
                               rDevice->getPixel(aPt4) != aCol);

        CPPUNIT_ASSERT_MESSAGE("number of rendered pixel is not 81",
                               countPixel( rDevice, aCol ) == 81);
    }

    void implTestCornerCases(const BitmapDeviceSharedPtr& rDevice)
    {
        rDevice->clear(Color(0));

        const basegfx::B2DRange aEmpty1(0,0,0,11);
        const basegfx::B2DRange aEmpty2(0,0,11,0);
        const basegfx::B2DRange aVertLineLeft(0,0,1,11);
        const basegfx::B2DRange aVertLineRight(10,0,11,11);
        const basegfx::B2DRange aHorzLineTop(0,0,11,1);
        const basegfx::B2DRange aHorzLineBottom(0,10,11,11);

        const Color aCol(0xFFFFFFFF);
        rDevice->fillPolyPolygon(
            basegfx::B2DPolyPolygon(
                basegfx::tools::createPolygonFromRect( aEmpty1 )),
            aCol,
            DrawMode_PAINT );
        CPPUNIT_ASSERT_MESSAGE("number of rendered pixel is not 0",
                               countPixel( rDevice, aCol ) == 0);

        rDevice->fillPolyPolygon(
            basegfx::B2DPolyPolygon(
                basegfx::tools::createPolygonFromRect( aEmpty2 )),
            aCol,
            DrawMode_PAINT );
        CPPUNIT_ASSERT_MESSAGE("number of rendered pixel is not 0",
                               countPixel( rDevice, aCol ) == 0);

        rDevice->fillPolyPolygon(
            basegfx::B2DPolyPolygon(
                basegfx::tools::createPolygonFromRect( aVertLineLeft )),
            aCol,
            DrawMode_PAINT );
        CPPUNIT_ASSERT_MESSAGE("number of rendered pixel is not 11",
                               countPixel( rDevice, aCol ) == 11);
        const basegfx::B2IPoint aPt1(0,0);
        CPPUNIT_ASSERT_MESSAGE("first pixel set",
                               rDevice->getPixel(aPt1) == aCol);

        rDevice->fillPolyPolygon(
            basegfx::B2DPolyPolygon(
                basegfx::tools::createPolygonFromRect( aVertLineRight )),
            aCol,
            DrawMode_PAINT );
        CPPUNIT_ASSERT_MESSAGE("number of rendered pixel is not 22",
                               countPixel( rDevice, aCol ) == 22);
        const basegfx::B2IPoint aPt2(10,10);
        CPPUNIT_ASSERT_MESSAGE("last pixel set",
                               rDevice->getPixel(aPt2) == aCol);

        rDevice->fillPolyPolygon(
            basegfx::B2DPolyPolygon(
                basegfx::tools::createPolygonFromRect( aHorzLineTop )),
            aCol,
            DrawMode_PAINT );
        CPPUNIT_ASSERT_MESSAGE("number of rendered pixel is not 31",
                               countPixel( rDevice, aCol ) == 31);
        const basegfx::B2IPoint aPt3(5,0);
        CPPUNIT_ASSERT_MESSAGE("top-middle pixel set",
                               rDevice->getPixel(aPt3) == aCol);

        rDevice->fillPolyPolygon(
            basegfx::B2DPolyPolygon(
                basegfx::tools::createPolygonFromRect( aHorzLineBottom )),
            aCol,
            DrawMode_PAINT );
        CPPUNIT_ASSERT_MESSAGE("number of rendered pixel is not 40",
                               countPixel( rDevice, aCol ) == 40);
        const basegfx::B2IPoint aPt4(5,10);
        CPPUNIT_ASSERT_MESSAGE("bottom-middle pixel set",
                               rDevice->getPixel(aPt4) == aCol);

        OUString aSvg( "m 0 0l7 7h-1z" );

        basegfx::B2DPolyPolygon aPoly;
        basegfx::tools::importFromSvgD( aPoly, aSvg );
        rDevice->fillPolyPolygon(
            aPoly,
            aCol,
            DrawMode_PAINT );
        CPPUNIT_ASSERT_MESSAGE("number of rendered pixel is not 43",
                               countPixel( rDevice, aCol ) == 43);
    }

    void implTestClipping(const BitmapDeviceSharedPtr& rDevice)
    {
        rDevice->clear(Color(0));

        const basegfx::B2DRange   aLeftTop(-10,-10,1,1);
        const basegfx::B2DRange   aRightTop(10,-10,20,1);
        const basegfx::B2DRange   aLeftBottom(-10,10,1,20);
        const basegfx::B2DRange   aRightBottom(10,10,20,20);
        const basegfx::B2DRange   aAllOver(-10,-10,20,20);

        const Color aCol(0xFFFFFFFF);
        rDevice->fillPolyPolygon( basegfx::B2DPolyPolygon(
                                      basegfx::tools::createPolygonFromRect(aLeftTop)),
                                  aCol,
                                  DrawMode_PAINT );
        CPPUNIT_ASSERT_MESSAGE("number of rendered pixel is not 1",
                               countPixel( rDevice, aCol ) == 1);

        rDevice->fillPolyPolygon( basegfx::B2DPolyPolygon(
                                      basegfx::tools::createPolygonFromRect(aRightTop)),
                                  aCol,
                                  DrawMode_PAINT );
        CPPUNIT_ASSERT_MESSAGE("number of rendered pixel is not 2",
                               countPixel( rDevice, aCol ) == 2);

        rDevice->fillPolyPolygon( basegfx::B2DPolyPolygon(
                                      basegfx::tools::createPolygonFromRect(aLeftBottom)),
                                  aCol,
                                  DrawMode_PAINT );
        CPPUNIT_ASSERT_MESSAGE("number of rendered pixel is not 3",
                               countPixel( rDevice, aCol ) == 3);

        rDevice->fillPolyPolygon( basegfx::B2DPolyPolygon(
                                      basegfx::tools::createPolygonFromRect(aRightBottom)),
                                  aCol,
                                  DrawMode_PAINT );
        CPPUNIT_ASSERT_MESSAGE("number of rendered pixel is not 4",
                               countPixel( rDevice, aCol ) == 4);

        rDevice->fillPolyPolygon( basegfx::B2DPolyPolygon(
                                      basegfx::tools::createPolygonFromRect(aAllOver)),
                                  aCol,
                                  DrawMode_PAINT );
        CPPUNIT_ASSERT_MESSAGE("number of rendered pixel is not 121",
                               countPixel( rDevice, aCol ) == 121);
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
                                           Format::THIRTYTWO_BIT_TC_MASK );
    }

    void testRectFill()
    {
        implTestRectFill( mpDevice1bpp );
        implTestRectFill( mpDevice32bpp );
    }

    void testClipping()
    {
        implTestClipping( mpDevice1bpp );
        implTestClipping( mpDevice32bpp );
    }

    void testCornerCases()
    {
        implTestCornerCases( mpDevice1bpp );
        implTestCornerCases( mpDevice32bpp );
    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(FillTest);
    CPPUNIT_TEST(testRectFill);
    CPPUNIT_TEST(testClipping);
    CPPUNIT_TEST(testCornerCases);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(FillTest);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
