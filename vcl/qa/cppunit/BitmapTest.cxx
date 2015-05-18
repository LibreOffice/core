/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <vcl/bitmap.hxx>
#include <vcl/bmpacc.hxx>

#include <tools/stream.hxx>
#include <vcl/graphicfilter.hxx>

#include "BitmapSymmetryCheck.hxx"

namespace
{

class BitmapTest : public CppUnit::TestFixture
{
    void testConvert();
    void testScale();

    CPPUNIT_TEST_SUITE(BitmapTest);
    CPPUNIT_TEST(testConvert);
    CPPUNIT_TEST(testScale);
    CPPUNIT_TEST_SUITE_END();
};

void BitmapTest::testConvert()
{
    Bitmap aBitmap(Size(10, 10), 8);

    aBitmap.Erase(COL_LIGHTGRAYBLUE);

    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(8), aBitmap.GetBitCount());
    {
        Bitmap::ScopedReadAccess pReadAccess(aBitmap);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(8), pReadAccess->GetBitCount());
#if defined WNT
        // Scanlines padded to DWORD multiples, it seems
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uLong>(12), pReadAccess->GetScanlineSize());
#else
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uLong>(10), pReadAccess->GetScanlineSize());
#endif
        CPPUNIT_ASSERT(pReadAccess->HasPalette());
        const BitmapColor& rColor = pReadAccess->GetPaletteColor(pReadAccess->GetPixelIndex(1, 1));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(204), sal_Int32(rColor.GetRed()));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(204), sal_Int32(rColor.GetGreen()));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(255), sal_Int32(rColor.GetBlue()));
    }

    aBitmap.Convert(BMP_CONVERSION_24BIT);

    CPPUNIT_ASSERT_EQUAL(sal_uInt16(24), aBitmap.GetBitCount());
    {
        Bitmap::ScopedReadAccess pReadAccess(aBitmap);
#if defined LINUX
        // 24 bit Bitmap on SVP backend uses 32bit BGRX format
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(32), pReadAccess->GetBitCount());
        CPPUNIT_ASSERT_EQUAL(sal_uLong(40), pReadAccess->GetScanlineSize());
#else
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(24), pReadAccess->GetBitCount());
#if defined WNT
        CPPUNIT_ASSERT_EQUAL(sal_uLong(32), pReadAccess->GetScanlineSize());
#else
        CPPUNIT_ASSERT_EQUAL(sal_uLong(30), pReadAccess->GetScanlineSize());
#endif
#endif
        CPPUNIT_ASSERT(!pReadAccess->HasPalette());
        Color aColor = pReadAccess->GetPixel(0, 0);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(204), sal_Int32(aColor.GetRed()));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(204), sal_Int32(aColor.GetGreen()));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(255), sal_Int32(aColor.GetBlue()));
    }
}

void BitmapTest::testScale()
{
    const bool bExportBitmap(false);

    Bitmap aBitmap24Bit(Size(10, 10), 24);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(24), aBitmap24Bit.GetBitCount());

    {
        Bitmap::ScopedWriteAccess aWriteAccess(aBitmap24Bit);
        aWriteAccess->Erase(COL_WHITE);
        aWriteAccess->SetLineColor(COL_BLACK);
        aWriteAccess->DrawRect(Rectangle(1, 1, 8, 8));
        aWriteAccess->DrawRect(Rectangle(3, 3, 6, 6));
    }

    BitmapSymmetryCheck aBitmapSymmetryCheck;

    CPPUNIT_ASSERT_EQUAL(static_cast<long>(10), aBitmap24Bit.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(static_cast<long>(10), aBitmap24Bit.GetSizePixel().Height());

    // Check symmetry of the bitmap
    CPPUNIT_ASSERT(aBitmapSymmetryCheck.check(aBitmap24Bit));

    if (bExportBitmap)
    {
        SvFileStream aStream(OUString("~/scale_before.png"), StreamMode::WRITE | StreamMode::TRUNC);
        GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
        rFilter.compressAsPNG(aBitmap24Bit, aStream, 9);
    }

    aBitmap24Bit.Scale(2, 2, BmpScaleFlag::Fast);

    CPPUNIT_ASSERT_EQUAL(static_cast<long>(20), aBitmap24Bit.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(static_cast<long>(20), aBitmap24Bit.GetSizePixel().Height());

    // After scaling the bitmap should still be symmetrical. This check guarantees that
    // scaling doesn't misalign the bitmap.
    CPPUNIT_ASSERT(aBitmapSymmetryCheck.check(aBitmap24Bit));

    if (bExportBitmap)
    {
        SvFileStream aStream(OUString("~/scale_after.png"), StreamMode::WRITE | StreamMode::TRUNC);
        GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
        rFilter.compressAsPNG(aBitmap24Bit, aStream, 9);
    }
}

}

CPPUNIT_TEST_SUITE_REGISTRATION(BitmapTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
