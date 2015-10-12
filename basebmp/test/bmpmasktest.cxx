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

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

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
class BmpMaskTest : public CppUnit::TestFixture
{
private:
    BitmapDeviceSharedPtr mpDevice1bpp;
    BitmapDeviceSharedPtr mpMaskBmp1bpp;
    BitmapDeviceSharedPtr mpBmp1bpp;
    BitmapDeviceSharedPtr mpDevice32bpp;
    BitmapDeviceSharedPtr mpBmp32bpp;

    void implTestBmpBasics(const BitmapDeviceSharedPtr& rDevice,
                           const BitmapDeviceSharedPtr& rBmp)
    {
        rDevice->clear(Color(0));
        const Color aCol(0xFFFFFFFF);

        const basegfx::B2IBox aSourceRect(0,0,10,10);
        const basegfx::B2IBox aDestAll(0,0,10,10);

        rDevice->drawMaskedBitmap(
            rBmp,
            mpMaskBmp1bpp,
            aSourceRect,
            aDestAll,
            DrawMode::Paint );
        CPPUNIT_ASSERT_MESSAGE("number of rendered pixel is not 30",
                               countPixel( rDevice, aCol ) == 30);
    }

    void implTestBmpScaledClip(const BitmapDeviceSharedPtr& rDevice,
                               const BitmapDeviceSharedPtr& rBmp)
    {
        rDevice->clear(Color(0));
        const Color aCol(0xFFFFFFFF);

        const basegfx::B2IBox aSourceRect(0,0,10,10);
        const basegfx::B2IBox aDestLeftTop(0,0,6,6);

        rDevice->drawMaskedBitmap(
            rBmp,
            mpMaskBmp1bpp,
            aSourceRect,
            aDestLeftTop,
            DrawMode::Paint );
        CPPUNIT_ASSERT_MESSAGE("number of rendered pixel is not 12",
                               countPixel( rDevice, aCol ) == 12);
    }

public:
    void setUp() override
    {
        const basegfx::B2ISize aSize(10,10);
        mpDevice1bpp = createBitmapDevice( aSize,
                                           true,
                                           Format::OneBitMsbPal,
                                           basebmp::getBitmapDeviceStrideForWidth(Format::OneBitMsbPal, aSize.getX()));
        mpDevice32bpp = createBitmapDevice( aSize,
                                            true,
                                            Format::ThirtyTwoBitTcMaskBGRA,
                                            basebmp::getBitmapDeviceStrideForWidth(Format::ThirtyTwoBitTcMaskBGRA, aSize.getX()));

        mpMaskBmp1bpp = createBitmapDevice( aSize,
                                            true,
                                            Format::OneBitMsbGrey,
                                            basebmp::getBitmapDeviceStrideForWidth(Format::OneBitMsbGrey, aSize.getX()));

        mpBmp1bpp = createBitmapDevice( aSize,
                                        true,
                                        Format::OneBitMsbPal,
                                        basebmp::getBitmapDeviceStrideForWidth(Format::OneBitMsbPal, aSize.getX()));
        mpBmp32bpp = createBitmapDevice( aSize,
                                         true,
                                         Format::ThirtyTwoBitTcMaskBGRA,
                                         basebmp::getBitmapDeviceStrideForWidth(Format::ThirtyTwoBitTcMaskBGRA, aSize.getX()));

        OUString aSvg( "m 0 0h5v10h5v-5h-10z" );

        basegfx::B2DPolyPolygon aPoly;
        basegfx::tools::importFromSvgD( aPoly, aSvg, false, NULL );
        const Color aColWhite(0xFFFFFFFF);
        const Color aColBlack(0);
        mpBmp1bpp->fillPolyPolygon(
            aPoly,
            aColWhite,
            DrawMode::Paint );
        mpBmp32bpp->fillPolyPolygon(
            aPoly,
            aColWhite,
            DrawMode::Paint );

        aSvg = "m 0 0 h6 v10 h-6z" ;

        aPoly.clear();
        basegfx::tools::importFromSvgD( aPoly, aSvg, false, NULL );
        mpMaskBmp1bpp->clear(aColWhite);
        mpMaskBmp1bpp->fillPolyPolygon(
            aPoly,
            aColBlack,
            DrawMode::Paint );
    }

    void testBmpBasics()
    {
        // mpDevice1bpp has a black rect. 0x0 -> 6x10
        implTestBmpBasics( mpDevice1bpp, mpBmp1bpp );
        implTestBmpBasics( mpDevice32bpp, mpBmp32bpp );
    }

    void testBmpClip()
    {
        implTestBmpScaledClip( mpDevice1bpp, mpBmp1bpp );
        implTestBmpScaledClip( mpDevice32bpp, mpBmp32bpp );
    }

    void testMasking()
    {
        BitmapDeviceSharedPtr xOutput;
        BitmapDeviceSharedPtr xBitmap;
        BitmapDeviceSharedPtr xMask;

        { // mpMask & mpBitmap
            const basegfx::B2ISize aSize(5, 5);
            std::vector< basebmp::Color > aDevPal;
            aDevPal.push_back( basebmp::Color( 0, 0, 0 ) );
            aDevPal.push_back( basebmp::Color( 0xff, 0xff, 0xff ) );

            basebmp::Format nFormat;

            nFormat = Format::OneBitMsbPal;
//            nFormat = Format::OneBitMsbGrey; // FIXME - un-comment me to crash hard.
            xMask = createBitmapDevice( aSize, false /* bTopDown */,
                                        nFormat,
                                         basebmp::getBitmapDeviceStrideForWidth( nFormat, aSize.getX()),
                                         PaletteMemorySharedVector(
                                                new std::vector< basebmp::Color >(aDevPal) ) );
            // wipe to copy everything.
            xMask->clear( basebmp::Color( 0x00, 0x00, 0x00 ) );

            // punch out another piece not to copy
            basegfx::B2DPolyPolygon aPoly;
            basegfx::tools::importFromSvgD( aPoly, "m 2 2 h4 v8 h-4z",
                                            false, NULL );
            xMask->fillPolyPolygon( aPoly, basebmp::Color( 0xff, 0xff, 0xff ),
                                    DrawMode::Paint );

            xBitmap = createBitmapDevice( aSize, false,
                                          Format::ThirtyTwoBitTcMaskBGRX,
                                          basebmp::getBitmapDeviceStrideForWidth(
                                                Format::ThirtyTwoBitTcMaskBGRX, aSize.getX()) );
            xBitmap->clear(Color(0x80808080));
        }
        { // mpOutput & mpBitmap
            const basegfx::B2ISize aSize(9, 9);
            xOutput = createBitmapDevice( aSize, false,
                                          Format::ThirtyTwoBitTcMaskBGRX,
                                          basebmp::getBitmapDeviceStrideForWidth(
                                                Format::ThirtyTwoBitTcMaskBGRX, aSize.getX()) );
            xOutput->clear(Color(0xffffffff));
        }

        const basegfx::B2IBox aSourceRect(0,0,4,4);
        const basegfx::B2IBox aDestAll(2,2,7,7);

        xOutput->drawMaskedBitmap(
            xBitmap, xMask,
            aSourceRect, aDestAll,
            DrawMode::Paint );

        CPPUNIT_ASSERT_MESSAGE( "output not cleared to white",
                                xOutput->getPixel( basegfx::B2IPoint( 0, 0 ) ) == Color(0xffffff) );
        CPPUNIT_ASSERT_MESSAGE( "bitmap not drawn",
                                xOutput->getPixel( basegfx::B2IPoint( 2, 2 ) ) == Color(0x808080) );
        CPPUNIT_ASSERT_MESSAGE( "mask not applied",
                                xOutput->getPixel( basegfx::B2IPoint( 6, 6 ) ) == Color(0xffffff) );
    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(BmpMaskTest);
    CPPUNIT_TEST(testMasking);
    CPPUNIT_TEST(testBmpBasics);
    CPPUNIT_TEST(testBmpClip);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(BmpMaskTest);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
