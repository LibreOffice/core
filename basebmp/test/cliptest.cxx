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
#include <basegfx/range/b2drange.hxx>
#include <basegfx/range/b2ibox.hxx>
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
class ClipTest : public CppUnit::TestFixture
{
private:
    BitmapDeviceSharedPtr mpClipMask;
    BitmapDeviceSharedPtr mpDevice1bpp;
    BitmapDeviceSharedPtr mpDevice32bpp;

    void implTestPixelClip(const BitmapDeviceSharedPtr& rDevice)
    {
        const Color aBgCol(0);
        rDevice->clear(aBgCol);

        const basegfx::B2IPoint aPt(0,0);
        const Color aCol(0xFFFFFFFF);
        rDevice->setPixel( aPt, aCol, DrawMode_PAINT, mpClipMask );
        CPPUNIT_ASSERT_MESSAGE("get/setPixel clip #1",
                               rDevice->getPixel(aPt) == aBgCol);

        const basegfx::B2IPoint aPt2(10,10);
        rDevice->setPixel( aPt2, aCol, DrawMode_PAINT, mpClipMask );
        CPPUNIT_ASSERT_MESSAGE("get/setPixel clip #2",
                               rDevice->getPixel(aPt2) == aBgCol);

        const basegfx::B2IPoint aPt1(10,0);
        rDevice->setPixel( aPt1, aCol, DrawMode_PAINT, mpClipMask );
        CPPUNIT_ASSERT_MESSAGE("get/setPixel clip #3",
                               rDevice->getPixel(aPt1) != aBgCol);

        const basegfx::B2IPoint aPt3(0,10);
        rDevice->setPixel( aPt3, aCol, DrawMode_PAINT, mpClipMask );
        CPPUNIT_ASSERT_MESSAGE("get/setPixel clip #4",
                               rDevice->getPixel(aPt3) != aBgCol);
    }

    void implTestLineClip(const BitmapDeviceSharedPtr& rDevice)
    {
        const Color aBgCol(0);
        rDevice->clear(aBgCol);

        const basegfx::B2IPoint aPt1(0,0);
        const basegfx::B2IPoint aPt2(1,9);
        const Color aCol(0xFFFFFFFF);
        rDevice->drawLine( aPt1, aPt2, aCol, DrawMode_PAINT, mpClipMask );

        const basegfx::B2IPoint aPt3(1,5);
        CPPUNIT_ASSERT_MESSAGE("get line pixel",
                               rDevice->getPixel(aPt3) != aBgCol);
        CPPUNIT_ASSERT_MESSAGE("number of rendered line pixel is not 4",
                               countPixel( rDevice,
                                           rDevice->getPixel(aPt3) ) == 4);

        rDevice->drawLine( aPt1, aPt2, aCol, DrawMode_XOR, mpClipMask );
        CPPUNIT_ASSERT_MESSAGE("number of xor-rendered line pixel is not 0",
                               countPixel( rDevice,
                                           rDevice->getPixel(aPt3) ) == 121);
    }

    void implTestFillClip(const BitmapDeviceSharedPtr& rDevice)
    {
        rDevice->clear(Color(0));

        const basegfx::B2DRange aAllOver(-10,-10,20,20);
        const Color aCol(0xFFFFFFFF);
        rDevice->fillPolyPolygon( basegfx::B2DPolyPolygon(
                                      basegfx::tools::createPolygonFromRect(aAllOver)),
                                  aCol,
                                  DrawMode_PAINT,
                                  mpClipMask );
        const basegfx::B2IPoint aPt(0,10);
        CPPUNIT_ASSERT_MESSAGE("number of clipped pixel is not 30",
                               countPixel( rDevice, rDevice->getPixel(aPt) ) == 121-30);

        rDevice->fillPolyPolygon( basegfx::B2DPolyPolygon(
                                      basegfx::tools::createPolygonFromRect(aAllOver)),
                                  aCol,
                                  DrawMode_PAINT );
        CPPUNIT_ASSERT_MESSAGE("number of filled pixel is not 121",
                               countPixel( rDevice, rDevice->getPixel(aPt) ) == 121);

        rDevice->fillPolyPolygon( basegfx::B2DPolyPolygon(
                                      basegfx::tools::createPolygonFromRect(aAllOver)),
                                  aCol,
                                  DrawMode_XOR,
                                  mpClipMask );
        CPPUNIT_ASSERT_MESSAGE("number of xor-cleared pixel is not 91",
                               countPixel( rDevice, rDevice->getPixel(aPt) ) == 121-30);
    }

    void implTestBmpClip(const BitmapDeviceSharedPtr& rDevice)
    {
        BitmapDeviceSharedPtr pBmp( cloneBitmapDevice(
                                        basegfx::B2IVector(3,3),
                                        rDevice ));
        Color aCol1(0);
        Color aCol2(0xFFFFFFFF);
        pBmp->clear(aCol1);
        pBmp->setPixel(basegfx::B2IPoint(0,0),aCol2,DrawMode_PAINT);
        pBmp->setPixel(basegfx::B2IPoint(1,1),aCol2,DrawMode_PAINT);
        pBmp->setPixel(basegfx::B2IPoint(2,2),aCol2,basebmp::DrawMode_PAINT);

        rDevice->clear(aCol1);
        rDevice->drawBitmap(pBmp,
                            basegfx::B2IBox(0,0,3,3),
                            basegfx::B2IBox(-1,-1,4,4),
                            DrawMode_PAINT,
                            mpClipMask);

        const basegfx::B2IPoint aPt(1,1);
        CPPUNIT_ASSERT_MESSAGE("number of clipped pixel is not 5",
                               countPixel( rDevice,
                                           rDevice->getPixel(aPt) ) == 5);
    }

    void implTestMaskColorClip(const BitmapDeviceSharedPtr& rDevice)
    {
        BitmapDeviceSharedPtr pBmp( createBitmapDevice( rDevice->getSize(),
                                                        true,
                                                        Format::EIGHT_BIT_GREY ));

        OUString aSvg( "m 0 0h5v10h5v-5h-10z" );

        basegfx::B2DPolyPolygon aPoly;
        basegfx::tools::importFromSvgD( aPoly, aSvg );
        const basebmp::Color aCol(0xFF);
        pBmp->clear( basebmp::Color(0) );
        pBmp->fillPolyPolygon(
            aPoly,
            aCol,
            basebmp::DrawMode_PAINT );

        const basegfx::B2IBox aSourceRect(0,0,10,10);
        const basegfx::B2IPoint aDestLeftTop(0,0);
        const Color aCol2(0xF0F0F0F0);
        rDevice->drawMaskedColor(
            aCol2,
            pBmp,
            aSourceRect,
            aDestLeftTop,
            mpClipMask );
        const basegfx::B2IPoint aPt(1,1);
        CPPUNIT_ASSERT_MESSAGE("number of rendered pixel is not 41",
                               countPixel( rDevice, rDevice->getPixel(aPt) ) == 41);

    }

public:
    void setUp()
    {
        const basegfx::B2ISize aSize(11,11);
        mpClipMask = createBitmapDevice( aSize,
                                         true,
                                         Format::ONE_BIT_MSB_GREY );
        mpDevice1bpp = createBitmapDevice( aSize,
                                           true,
                                           Format::ONE_BIT_MSB_PAL );
        mpDevice32bpp = createBitmapDevice( aSize,
                                           true,
                                           Format::THIRTYTWO_BIT_TC_MASK_BGRA );

        OUString aSvg( "m 0 0 h5 l5 5 v5 h-5 l-5-5 z" );
        basegfx::B2DPolyPolygon aPoly;
        basegfx::tools::importFromSvgD( aPoly, aSvg );
        mpClipMask->clear(Color(0));
        mpClipMask->drawPolygon(
            aPoly.getB2DPolygon(0),
            Color(0xFFFFFFFF),
            DrawMode_PAINT );
    }

    void testPixelClip()
    {
        implTestPixelClip( mpDevice1bpp );
        implTestPixelClip( mpDevice32bpp );
    }

    void testLineClip()
    {
        implTestLineClip( mpDevice1bpp );
        implTestLineClip( mpDevice32bpp );
    }

    void testFillClip()
    {
        implTestFillClip( mpDevice1bpp );
        implTestFillClip( mpDevice32bpp );
    }

    void testBmpClip()
    {
        implTestBmpClip( mpDevice1bpp );
        implTestBmpClip( mpDevice32bpp );
    }

    void testMaskColorClip()
    {
        implTestMaskColorClip( mpDevice1bpp );
        implTestMaskColorClip( mpDevice32bpp );
    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(ClipTest);
    CPPUNIT_TEST(testPixelClip);
    CPPUNIT_TEST(testLineClip);
    CPPUNIT_TEST(testFillClip);
    CPPUNIT_TEST(testBmpClip);
    CPPUNIT_TEST(testMaskColorClip);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ClipTest);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
