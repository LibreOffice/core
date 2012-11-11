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
#include <basegfx/matrix/b2dhommatrix.hxx>
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
class PolyTest : public CppUnit::TestFixture
{
private:
    BitmapDeviceSharedPtr mpDevice1bpp;
    BitmapDeviceSharedPtr mpDevice32bpp;

    void implTestEmpty(const BitmapDeviceSharedPtr& rDevice)
    {
        const Color aCol(0xFFFFFFFF);
        const Color aBgCol(0);
        rDevice->clear(aBgCol);
        basegfx::B2DPolyPolygon aPoly;

        basegfx::tools::importFromSvgD(
            aPoly,
            OUString( "M2 2 l7 7 z" ) );
        rDevice->fillPolyPolygon(
            aPoly,
            aCol,
            DrawMode_PAINT );
        CPPUNIT_ASSERT_MESSAGE("number of rendered pixel is not 0",
                               countPixel( rDevice, aCol ) == 0);

        // --------------------------------------------------

        rDevice->clear(aBgCol);
        aPoly.clear();
        basegfx::tools::importFromSvgD(
            aPoly,
            OUString( "M7 2 l-6 6 z" ) );
        rDevice->fillPolyPolygon(
            aPoly,
            aCol,
            DrawMode_PAINT );
        CPPUNIT_ASSERT_MESSAGE("number of rendered pixel is not 0(b)",
                               countPixel( rDevice, aCol ) == 0);
    }

    void implTestHairline(const BitmapDeviceSharedPtr& rDevice)
    {
        const Color aCol(0xFFFFFFFF);
        const Color aBgCol(0);
        rDevice->clear(aBgCol);
        basegfx::B2DPolyPolygon aPoly;

        basegfx::tools::importFromSvgD(
            aPoly,
            OUString( "M2 2 h1 l7 7 h-1 z" ) );
        rDevice->fillPolyPolygon(
            aPoly,
            aCol,
            DrawMode_PAINT );
        CPPUNIT_ASSERT_MESSAGE("number of rendered pixel is not 7",
                               countPixel( rDevice, aCol ) == 7);

        // --------------------------------------------------

        rDevice->clear(aBgCol);
        aPoly.clear();
        basegfx::tools::importFromSvgD(
            aPoly,
            OUString( "M7 2 h-1 l-6 6 h1 z" ) );
        rDevice->fillPolyPolygon(
            aPoly,
            aCol,
            DrawMode_PAINT );
        CPPUNIT_ASSERT_MESSAGE("number of rendered pixel is not 6",
                               countPixel( rDevice, aCol ) == 6);

        // --------------------------------------------------

        rDevice->clear(aBgCol);
        aPoly.clear();
        basegfx::tools::importFromSvgD(
            aPoly,
            OUString( "M0 0 l7 7 h-1 l-5-7 z" ) );
        rDevice->fillPolyPolygon(
            aPoly,
            aCol,
            DrawMode_PAINT );
        CPPUNIT_ASSERT_MESSAGE("number of rendered pixel is not 3",
                               countPixel( rDevice, aCol ) == 3);
    }

    void implTestPolyPoly(const BitmapDeviceSharedPtr& rDevice)
    {
        const Color aCol(0xFFFFFFFF);
        const Color aBgCol(0);
        rDevice->clear(aBgCol);
        basegfx::B2DPolyPolygon aPoly;

        basegfx::tools::importFromSvgD( aPoly,
                                        OUString( "M0 0 h7 v7 h-7 z M2 2 v3 h3 v-3 z" ) );

        rDevice->fillPolyPolygon(
            aPoly,
            aCol,
            DrawMode_PAINT );
        CPPUNIT_ASSERT_MESSAGE("number of rendered pixel is not 40",
                               countPixel( rDevice, aCol ) == 40);
    }

    void implTestPolyPolyClip(const BitmapDeviceSharedPtr& rDevice)
    {
        const Color aCol(0xFFFFFFFF);
        const Color aBgCol(0);
        rDevice->clear(aBgCol);
        basegfx::B2DPolyPolygon aPoly;

        basegfx::tools::importFromSvgD( aPoly,
                                        OUString( "M0 0 h7 v7 h-7 z M2 2 v3 h3 v-3 z" ) );
        basegfx::B2DHomMatrix aMat;
        aMat.translate(-3,-3);
        aMat.rotate( 1.7 );
        aMat.translate(6,5);
        aPoly.transform(aMat);

        rDevice->fillPolyPolygon(
            aPoly,
            aCol,
            DrawMode_PAINT );

        CPPUNIT_ASSERT_MESSAGE("number of rendered pixel is not 39",
                               countPixel( rDevice, aCol ) == 39);

        BitmapDeviceSharedPtr pClippedDevice(
            subsetBitmapDevice( rDevice,
                                basegfx::B2IBox(3,3,5,8) ));
        CPPUNIT_ASSERT_MESSAGE("size of subsetted device is not (2,5)",
                               pClippedDevice->getSize() == basegfx::B2IVector(2,5));

        rDevice->clear(aBgCol);
        pClippedDevice->fillPolyPolygon(
            aPoly,
            aCol,
            DrawMode_PAINT );
        CPPUNIT_ASSERT_MESSAGE("number of rendered pixel is not 7",
                               countPixel( rDevice, aCol ) == 7);
    }

    void implTestLineDrawClip(const BitmapDeviceSharedPtr& rDevice)
    {
        const Color aCol(0xFFFFFFFF);
        const Color aBgCol(0);
        rDevice->clear(aBgCol);

        // create rectangular subset, such that we can 'see' extra
        // pixel outside
        BitmapDeviceSharedPtr pClippedDevice=(
            subsetBitmapDevice( rDevice,
                                basegfx::B2IBox(3,3,5,9) ));

        // trigger "alternate bresenham" case in
        // clippedlinerenderer.hxx, first point not clipped
        const basegfx::B2IPoint aPt1(3,3);
        const basegfx::B2IPoint aPt2(4,2);
        pClippedDevice->drawLine( aPt1, aPt2, aCol, DrawMode_PAINT );

        CPPUNIT_ASSERT_MESSAGE("number of rendered pixel is not 1",
                               countPixel( rDevice, aCol ) == 1);

        // trigger "alternate bresenham" case in
        // clippedlinerenderer.hxx, both start and endpoint clipped
        const basegfx::B2IPoint aPt3(0,4);
        pClippedDevice->drawLine( aPt3, aPt2, aCol, DrawMode_XOR );

        CPPUNIT_ASSERT_MESSAGE("number of rendered pixel is not 0",
                               countPixel( rDevice, aCol ) == 0);

        // trigger "standard bresenham" case in
        // clippedlinerenderer.hxx, first point not clipped
        const basegfx::B2IPoint aPt4(6,2);
        pClippedDevice->drawLine( aPt1, aPt4, aCol, DrawMode_PAINT );

        CPPUNIT_ASSERT_MESSAGE("number of rendered pixel is not 2",
                               countPixel( rDevice, aCol ) == 2);

        // trigger "clipCode1 & aMinFlag/bMinFlag" cases in
        // clippedlinerenderer.hxx (note1: needs forcing end point to
        // be clipped as well, otherwise optimisation kicks in. note2:
        // needs forcing end point to clip on two edges, not only on
        // one, otherwise swap kicks in)
        const basegfx::B2IPoint aPt5(1,1);
        const basegfx::B2IPoint aPt6(6,10);
        pClippedDevice->drawLine( aPt5, aPt6, aCol, DrawMode_XOR );

        CPPUNIT_ASSERT_MESSAGE("number of rendered pixel is not 6",
                               countPixel( rDevice, aCol ) == 6);

        // trigger "clipCode1 & (aMinFlag|aMaxFlag)" case in
        // clippedlinerenderer.hxx that was not taken for the test
        // above
        pClippedDevice->drawLine( aPt3, aPt6, aCol, DrawMode_XOR );

        CPPUNIT_ASSERT_MESSAGE("number of rendered pixel is not 8",
                               countPixel( rDevice, aCol ) == 8);

    }

    void implTestPolyDrawClip(const BitmapDeviceSharedPtr& rDevice)
    {
        const Color aCol(0xFFFFFFFF);
        const Color aBgCol(0);
        rDevice->clear(aBgCol);
        basegfx::B2DPolyPolygon aPoly;

        // test all corner-touching lines of our clip rect. note that
        // *all* of the four two-pixel lines in that polygon do *not*
        // generate a single pixel, due to the rasterization effect.
        basegfx::tools::importFromSvgD( aPoly,
                                        OUString( "M2 3 l1 -1 M4 2 l1 1 M2 8 l1 1 M5 8 l-1 1 M2 5 h4 M3 0 v10" ) );
        BitmapDeviceSharedPtr pClippedDevice=(
            subsetBitmapDevice( rDevice,
                                basegfx::B2IBox(3,3,5,9) ));

        for( unsigned int i=0; i<aPoly.count(); ++i )
            pClippedDevice->drawPolygon(
                aPoly.getB2DPolygon(i),
                aCol,
                DrawMode_PAINT );

        CPPUNIT_ASSERT_MESSAGE("number of rendered pixel is not 7",
                               countPixel( rDevice, aCol ) == 7);
    }

    void implTestPolyPolyCrissCross(const BitmapDeviceSharedPtr& rDevice)
    {
        const Color aCol(0xFFFFFFFF);
        const Color aBgCol(0);
        rDevice->clear(aBgCol);
        basegfx::B2DPolyPolygon aPoly;

        basegfx::tools::importFromSvgD( aPoly,
                                        OUString( "M0 0 v2 l10 2 v-2 z"
                                            "M10 6 v-2 l-10 2 v2 z"
                                            "M1 0 h1 v10 h-1 z"
                                            "M4 0 h1 v10 h-1 z"
                                            "M8 0 h1 v10 h-1 z" ) );
        rDevice->fillPolyPolygon(
            aPoly,
            aCol,
            DrawMode_PAINT );
        CPPUNIT_ASSERT_MESSAGE("number of rendered pixel is not 46",
                               countPixel( rDevice, aCol ) == 46);
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
                                            Format::THIRTYTWO_BIT_TC_MASK );
    }

    void testEmpty()
    {
        implTestEmpty( mpDevice1bpp );
        implTestEmpty( mpDevice32bpp );
    }

    void testHairline()
    {
        implTestHairline( mpDevice1bpp );
        implTestHairline( mpDevice32bpp );
    }

    void testPolyPoly()
    {
        implTestPolyPoly( mpDevice1bpp );
        implTestPolyPoly( mpDevice32bpp );
    }

    void testPolyPolyClip()
    {
        implTestPolyPolyClip(mpDevice1bpp);
        implTestPolyPolyClip(mpDevice32bpp);
    }

    void testLineDrawClip()
    {
        implTestLineDrawClip(mpDevice1bpp);
        implTestLineDrawClip(mpDevice32bpp);
    }

    void testPolyDrawClip()
    {
        implTestPolyDrawClip(mpDevice1bpp);
        implTestPolyDrawClip(mpDevice32bpp);
    }

    void testPolyPolyCrissCross()
    {
        implTestPolyPolyCrissCross(mpDevice1bpp);
        implTestPolyPolyCrissCross(mpDevice32bpp);
    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(PolyTest);
    CPPUNIT_TEST(testEmpty);
    CPPUNIT_TEST(testHairline);
    CPPUNIT_TEST(testPolyPoly);
    CPPUNIT_TEST(testPolyPolyClip);
    CPPUNIT_TEST(testLineDrawClip);
    CPPUNIT_TEST(testPolyDrawClip);
    CPPUNIT_TEST(testPolyPolyCrissCross);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(PolyTest);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
