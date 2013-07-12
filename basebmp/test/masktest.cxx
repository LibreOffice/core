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
class MaskTest : public CppUnit::TestFixture
{
private:
    BitmapDeviceSharedPtr mpDevice1bpp;
    BitmapDeviceSharedPtr mpDevice32bpp;
    BitmapDeviceSharedPtr mpMask;

    void implTestMaskBasics(const BitmapDeviceSharedPtr& rDevice,
                            const BitmapDeviceSharedPtr& rBmp)
    {
        const Color aCol(0);
        const Color aCol2(0xF0F0F0F0);

        const basegfx::B2IBox aSourceRect(0,0,10,10);
        const basegfx::B2IPoint aDestLeftTop(0,0);
        const basegfx::B2IPoint aDestRightTop(5,0);
        const basegfx::B2IPoint aDestLeftBottom(0,5);
        const basegfx::B2IPoint aDestRightBottom(5,5);

        rDevice->clear(aCol);
        rDevice->setPixel(
            basegfx::B2IPoint(1,1),
            aCol2,
            DrawMode_PAINT);
        rDevice->drawMaskedColor(
            aCol2,
            rBmp,
            aSourceRect,
            aDestLeftTop );
        CPPUNIT_ASSERT_MESSAGE("number of rendered pixel is not 50",
                               countPixel( rDevice, aCol ) == 100-50);

        rDevice->clear(aCol);
        rDevice->drawMaskedColor(
            aCol2,
            rBmp,
            aSourceRect,
            aDestRightTop );
        CPPUNIT_ASSERT_MESSAGE("number of rendered pixel is not 25",
                               countPixel( rDevice, aCol ) == 100-25);

        rDevice->clear(aCol);
        rDevice->drawMaskedColor(
            aCol2,
            rBmp,
            aSourceRect,
            aDestLeftBottom );
        CPPUNIT_ASSERT_MESSAGE("number of rendered pixel is not 25(b)",
                               countPixel( rDevice, aCol ) == 100-25);

        rDevice->clear(aCol);
        rDevice->drawMaskedColor(
            aCol2,
            rBmp,
            aSourceRect,
            aDestRightBottom );
        CPPUNIT_ASSERT_MESSAGE("number of rendered pixel is not 25(c)",
                               countPixel( rDevice, aCol ) == 100-25);
    }

public:
    void setUp()
    {
        const basegfx::B2ISize aSize(10,10);
        mpDevice1bpp = createBitmapDevice( aSize,
                                           true,
                                           FORMAT_ONE_BIT_MSB_PAL );
        mpDevice32bpp = createBitmapDevice( aSize,
                                            true,
                                            FORMAT_THIRTYTWO_BIT_TC_MASK_BGRA );

        mpMask = createBitmapDevice( aSize,
                                     true,
                                     FORMAT_EIGHT_BIT_GREY );

        OUString aSvg( "m 0 0h5v10h5v-5h-10z" );

        basegfx::B2DPolyPolygon aPoly;
        basegfx::tools::importFromSvgD( aPoly, aSvg );
        const Color aCol(0xFF);
        mpMask->fillPolyPolygon(
            aPoly,
            aCol,
            DrawMode_PAINT );
    }

    void testMaskBasics()
    {
        implTestMaskBasics( mpDevice32bpp, mpMask );
        implTestMaskBasics( mpDevice1bpp, mpMask );
    }

    void testMaskClip()
    {
    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(MaskTest);
    CPPUNIT_TEST(testMaskBasics);
    CPPUNIT_TEST(testMaskClip);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(MaskTest);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
