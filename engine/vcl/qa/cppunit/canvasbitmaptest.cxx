/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

// bootstrap stuff
#include <test/bootstrapfixture.hxx>

#include <com/sun/star/util/Endianness.hpp>
#include <com/sun/star/rendering/ColorComponentTag.hpp>
#include <com/sun/star/rendering/ColorSpaceType.hpp>
#include <com/sun/star/rendering/RenderingIntent.hpp>
#include <com/sun/star/rendering/XIntegerReadOnlyBitmap.hpp>
#include <com/sun/star/rendering/XIntegerBitmapColorSpace.hpp>
#include <com/sun/star/rendering/XBitmapPalette.hpp>

#include <cppuhelper/implbase.hxx>
#include <rtl/ref.hxx>
#include <sal/log.hxx>

#include <vcl/canvastools.hxx>
#include <vcl/bitmap.hxx>

#include <canvasbitmap.hxx>
#include <vcl/BitmapWriteAccess.hxx>

#include <algorithm>

using namespace ::com::sun::star;
using namespace vcl::unotools;

namespace com::sun::star::rendering
{

static bool operator==( const RGBColor& rLHS, const ARGBColor& rRHS )
{
    return rLHS.Red == rRHS.Red && rLHS.Green == rRHS.Green && rLHS.Blue == rRHS.Blue;
}

}

namespace
{

class CanvasBitmapTest : public test::BootstrapFixture
{
public:
    CanvasBitmapTest() : BootstrapFixture(true, false) {}

    void runTest();

    CPPUNIT_TEST_SUITE(CanvasBitmapTest);
    CPPUNIT_TEST(runTest);
    CPPUNIT_TEST_SUITE_END();
};

bool rangeCheck( const rendering::RGBColor& rColor )
{
    return rColor.Red < 0.0 || rColor.Red > 1.0 ||
        rColor.Green < 0.0 || rColor.Green > 1.0 ||
        rColor.Blue < 0.0 || rColor.Blue > 1.0;
}

void checkCanvasBitmap( const rtl::Reference<VclCanvasBitmap>& xBmp,
                        const char*                            msg,
                        int                                    nOriginalDepth )
{
    SAL_INFO("vcl", "Testing " << msg << ", with depth " << nOriginalDepth);

    Bitmap   aContainedBmp( xBmp->getBitmap() );
    int      nDepth;
    bool     bHasPalette;
    {
        BitmapScopedReadAccess pAcc( aContainedBmp );
        nDepth = pAcc->GetBitCount();
        bHasPalette = pAcc->HasPalette();
    }

    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Original bitmap size not (200,200)",
                            Size(200,200), aContainedBmp.GetSizePixel());

    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Original bitmap size via API not (200,200)",
                            sal_Int32(200), xBmp->getSize().Width);
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Original bitmap size via API not (200,200)",
                            sal_Int32(200), xBmp->getSize().Height);

    CPPUNIT_ASSERT_EQUAL_MESSAGE( "alpha state mismatch",
                            aContainedBmp.HasAlpha(), xBmp->hasAlpha());

    CPPUNIT_ASSERT_MESSAGE( "getScaledBitmap() failed",
                            xBmp->getScaledBitmap( geometry::RealSize2D(500,500), false ).is());

    rendering::IntegerBitmapLayout aLayout;
    cpo::uno::Sequence<sal_Int8> aPixelData = xBmp->getData(aLayout, geometry::IntegerRectangle2D(0,0,1,1));

    const sal_Int32 nExpectedBitsPerPixel(bHasPalette ? 8 : (aContainedBmp.HasAlpha() ? 32 : 24));
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "# scanlines not 1",
                            static_cast<sal_Int32>(1), aLayout.ScanLines);
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "# scanline bytes mismatch",
                            static_cast<sal_Int32>((nExpectedBitsPerPixel+7)/8), aLayout.ScanLineBytes);
    CPPUNIT_ASSERT_MESSAGE( "# scanline stride mismatch",
                            aLayout.ScanLineStride == (nExpectedBitsPerPixel+7)/8 ||
                            aLayout.ScanLineStride == -(nExpectedBitsPerPixel+7)/8);
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "# plane stride not 0",
                            static_cast<sal_Int32>(0), aLayout.PlaneStride);

    CPPUNIT_ASSERT_MESSAGE( "Color space not there",
                            aLayout.ColorSpace.is());

    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Palette existence does not conform to bitmap",
                            (nDepth <= 8), aLayout.Palette.is());

    cpo::uno::Sequence<sal_Int8> aPixelData2 = xBmp->getPixel( aLayout, geometry::IntegerPoint2D(0,0) );

    CPPUNIT_ASSERT_EQUAL_MESSAGE( "getData and getPixel did not return same amount of data",
                            aPixelData.getLength(), aPixelData2.getLength());

    aPixelData = xBmp->getData(aLayout, geometry::IntegerRectangle2D(0,0,/*X2*/200,/*Y2*/1));
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "# scanlines not 1 for getPixel",
                            static_cast<sal_Int32>(1), aLayout.ScanLines);
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "# scanline bytes mismatch for getPixel",
                            static_cast<sal_Int32>((200*nExpectedBitsPerPixel+7)/8), aLayout.ScanLineBytes);
    CPPUNIT_ASSERT_MESSAGE( "# scanline stride mismatch for getPixel",
                            aLayout.ScanLineStride == (200*nExpectedBitsPerPixel+7)/8 ||
                            aLayout.ScanLineStride == -(200*nExpectedBitsPerPixel+7)/8);

    const cpo::uno::Sequence<rendering::RGBColor> aRGBColors = xBmp->convertIntegerToRGB(aPixelData);
    const cpo::uno::Sequence<rendering::ARGBColor> aARGBColors = xBmp->convertIntegerToARGB(aPixelData);

    std::pair<const rendering::RGBColor*, const rendering::ARGBColor*> aRes
        = std::mismatch(aRGBColors.begin(), aRGBColors.end(), aARGBColors.begin());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("argb and rgb colors are not equal", aRGBColors.end(), aRes.first);

    CPPUNIT_ASSERT_MESSAGE( "rgb colors are not within [0,1] range",
                            std::none_of(aRGBColors.begin(), aRGBColors.end(), &rangeCheck));

    if( !aContainedBmp.HasAlpha() )
    {
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
            "First pixel is not white", 1.0, aRGBColors[0].Red, 1E-12);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
            "First pixel is not white", 1.0, aRGBColors[0].Green, 1E-12);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
            "First pixel is not white", 1.0, aRGBColors[0].Blue, 1E-12);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
            "Second pixel is not opaque", 1.0, aARGBColors[1].Alpha, 1E-12);
    }
    else
    {
        // with premultiplied alpha, the white becomes black
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
            "First pixel is not black", 0.0, aRGBColors[0].Red, 1E-12);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
            "First pixel is not black", 0.0, aRGBColors[0].Green, 1E-12);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
            "First pixel is not black", 0.0, aRGBColors[0].Blue, 1E-12);
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "First pixel is not fully transparent",
                                0.0, aARGBColors[0].Alpha);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
            "Second pixel is not opaque", 1.0, aARGBColors[1].Alpha, 1E-12);
    }

    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Second pixel is not black",
                            0.0, aRGBColors[1].Red);
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Second pixel is not black",
                            0.0, aRGBColors[1].Green);
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Second pixel is not black",
                            0.0, aRGBColors[1].Blue);

    if( !bHasPalette )
    {
        const Color aCol(COL_GREEN);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "Sixth pixel is not green (red component)",
            vcl::unotools::toDoubleColor(aCol.GetRed()), aRGBColors[5].Red);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "Sixth pixel is not green (green component)",
            vcl::unotools::toDoubleColor(aCol.GetGreen()), aRGBColors[5].Green);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "Sixth pixel is not green (blue component)",
            vcl::unotools::toDoubleColor(aCol.GetBlue()), aRGBColors[5].Blue);
    }
    else
    {
        uno::Reference<rendering::XBitmapPalette> xPal = xBmp->getPalette();
        CPPUNIT_ASSERT_MESSAGE( "8bit or less: missing palette",
                                xPal.is());
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "Palette incorrect entry count",
                                static_cast<sal_Int32>(1 << nOriginalDepth), xPal->getNumberOfEntries());
        cpo::uno::Sequence<double> aIndex;
        CPPUNIT_ASSERT_MESSAGE( "Palette is not read-only",
                                !xPal->setIndex(aIndex,true,0));
        CPPUNIT_ASSERT_MESSAGE( "Palette entry 0 is not opaque",
                                xPal->getIndex(aIndex,0));
        CPPUNIT_ASSERT_MESSAGE( "Palette has no valid color space",
                                xPal->getColorSpace().is());
    }

    if( nOriginalDepth <= 8 )
        return;

    cpo::uno::Sequence<sal_Int8> aPixel3, aPixel4;

    const Color aCol(COL_GREEN);
    cpo::uno::Sequence<rendering::ARGBColor> aARGBColor
    {
        {
            1.0,
            vcl::unotools::toDoubleColor(aCol.GetRed()),
            vcl::unotools::toDoubleColor(aCol.GetGreen()),
            vcl::unotools::toDoubleColor(aCol.GetBlue())
        }
    };

    aPixel3 = xBmp->convertIntegerFromARGB( aARGBColor );
    aPixel4 = xBmp->getPixel( aLayout, geometry::IntegerPoint2D(5,0) );
    CPPUNIT_ASSERT_MESSAGE( "Green pixel from bitmap mismatch with manually converted green pixel",
                            bool(aPixel3 == aPixel4));

    if( !aContainedBmp.HasAlpha() )
    {
        cpo::uno::Sequence<rendering::RGBColor>  aRGBColor
        {
            {
                vcl::unotools::toDoubleColor(aCol.GetRed()),
                vcl::unotools::toDoubleColor(aCol.GetGreen()),
                vcl::unotools::toDoubleColor(aCol.GetBlue())
            }
        };
        aPixel3 = xBmp->convertIntegerFromRGB( aRGBColor );
        CPPUNIT_ASSERT_MESSAGE( "Green pixel from bitmap mismatch with manually RGB-converted green pixel",
                                bool(aPixel3 == aPixel4));
    }

}

void CanvasBitmapTest::runTest()
{
    static vcl::PixelFormat ePixelFormatArray[] =
    {
        vcl::PixelFormat::N8_BPP,
        vcl::PixelFormat::N24_BPP
    };

    // Testing VclCanvasBitmap wrapper

    for (auto const pixelFormat : ePixelFormatArray)
    {
        const sal_uInt16 nDepth = sal_uInt16(pixelFormat);
        Bitmap aBitmap(Size(200,200), pixelFormat);
        aBitmap.Erase(COL_WHITE);
        {
            BitmapScopedWriteAccess pAcc(aBitmap);
            if( pAcc.get() )
            {
                BitmapColor aBlack(0);
                BitmapColor aWhite(0);
                if( pAcc->HasPalette() )
                {
                    aBlack.SetIndex( sal::static_int_cast<sal_Int8>(pAcc->GetBestPaletteIndex(BitmapColor(0,0,0))) );
                    aWhite.SetIndex( sal::static_int_cast<sal_Int8>(pAcc->GetBestPaletteIndex(BitmapColor(255,255,255))) );
                }
                else
                {
                    aBlack = COL_BLACK;
                    aWhite = COL_WHITE;
                }
                pAcc->SetFillColor(COL_GREEN);
                pAcc->FillRect(tools::Rectangle(0,0,100,100));
                pAcc->SetPixel(0,0,aWhite);
                pAcc->SetPixel(0,1,aBlack);
                pAcc->SetPixel(0,2,aWhite);
            }
        }

        rtl::Reference<VclCanvasBitmap> xBmp( new VclCanvasBitmap(aBitmap) );

        checkCanvasBitmap( xBmp, "single bitmap", nDepth );

        AlphaMask aMask(Size(200,200));
        aMask.Erase(255);
        {
            BitmapScopedWriteAccess pAcc(aMask);
            if( pAcc.get() )
            {
                pAcc->SetFillColor(COL_ALPHA_OPAQUE);
                pAcc->FillRect(tools::Rectangle(0,0,100,100));
                pAcc->SetPixel(0,0,BitmapColor(0));
                pAcc->SetPixel(0,1,BitmapColor(255));
                pAcc->SetPixel(0,2,BitmapColor(0));
            }
        }

        xBmp.set( new VclCanvasBitmap(Bitmap(aBitmap,aMask)) );

        checkCanvasBitmap( xBmp, "masked bitmap", nDepth );

        AlphaMask aAlpha(Size(200,200));
        aAlpha.Erase(0);
        {
            BitmapScopedWriteAccess pAcc(aAlpha);
            if( pAcc )
            {
                pAcc->SetFillColor(COL_ALPHA_OPAQUE);
                pAcc->FillRect(tools::Rectangle(0,0,100,100));
                pAcc->SetPixel(0,0,BitmapColor(0));
                pAcc->SetPixel(0,1,BitmapColor(255));
                pAcc->SetPixel(0,2,BitmapColor(0));
            }
        }

        xBmp.set( new VclCanvasBitmap(Bitmap(aBitmap,aAlpha)) );

        checkCanvasBitmap( xBmp, "alpha bitmap", nDepth );
    }
}

} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(CanvasBitmapTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
