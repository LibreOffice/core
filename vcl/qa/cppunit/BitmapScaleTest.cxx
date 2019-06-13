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

#include <vcl/bitmap.hxx>
#include <vcl/bitmapaccess.hxx>

#include <tools/stream.hxx>
#include <vcl/graphicfilter.hxx>

#include <BitmapSymmetryCheck.hxx>
#include <bitmapwriteaccess.hxx>

namespace
{
class BitmapScaleTest : public CppUnit::TestFixture
{
    void testScale();
    void testScale2();
    void testScaleSymmetry();

    CPPUNIT_TEST_SUITE(BitmapScaleTest);
    CPPUNIT_TEST(testScale);
    CPPUNIT_TEST(testScale2);
    CPPUNIT_TEST(testScaleSymmetry);
    CPPUNIT_TEST_SUITE_END();
};

bool checkBitmapColor(Bitmap const& rBitmap, Color const& rExpectedColor)
{
    bool bResult = true;
    Bitmap aBitmap(rBitmap);
    Bitmap::ScopedReadAccess pReadAccess(aBitmap);
    long nHeight = pReadAccess->Height();
    long nWidth = pReadAccess->Width();
    for (long y = 0; y < nHeight; ++y)
    {
        Scanline pScanlineRead = pReadAccess->GetScanline(y);
        for (long x = 0; x < nWidth; ++x)
        {
            Color aColor = pReadAccess->GetPixelFromData(pScanlineRead, x);
            if (aColor != rExpectedColor)
                bResult = false;
        }
    }

    return bResult;
}

void assertColorsAreSimilar(int maxDifference, const std::string& message,
                            const BitmapColor& expected, const BitmapColor& actual)
{
    // Check that the two colors match or are reasonably similar.
    if (expected == actual)
        return;
    if (abs(expected.GetRed() - actual.GetRed()) <= maxDifference
        && abs(expected.GetGreen() - actual.GetGreen()) <= maxDifference
        && abs(expected.GetBlue() - actual.GetBlue()) <= maxDifference
        && abs(expected.GetAlpha() - actual.GetAlpha()) <= maxDifference)
    {
        return;
    }
    CPPUNIT_ASSERT_EQUAL_MESSAGE(message, expected, actual);
}

void assertColorsAreSimilar(int maxDifference, int line, const BitmapColor& expected,
                            const BitmapColor& actual)
{
    std::stringstream stream;
    stream << "Line: " << line;
    assertColorsAreSimilar(maxDifference, stream.str(), expected, actual);
}

void BitmapScaleTest::testScale()
{
    const bool bExportBitmap(false);
    using tools::Rectangle;

    static const BmpScaleFlag scaleMethods[]
        = { BmpScaleFlag::Default,     BmpScaleFlag::Fast,    BmpScaleFlag::BestQuality,
            BmpScaleFlag::Interpolate, BmpScaleFlag::Lanczos, BmpScaleFlag::BiCubic,
            BmpScaleFlag::BiLinear };
    for (BmpScaleFlag scaleMethod : scaleMethods)
    {
        struct ScaleSize
        {
            Size srcSize;
            Size destSize;
        };
        static const ScaleSize scaleSizes[]
            = { // test no-op
                { Size(16, 16), Size(16, 16) },
                // powers of 2 (OpenGL may use texture atlas)
                { Size(16, 16), Size(14, 14) },
                { Size(14, 14), Size(16, 16) }, // both upscaling and downscaling
                // "random" sizes
                { Size(18, 18), Size(14, 14) },
                { Size(14, 14), Size(18, 18) },
                // different x/y ratios
                { Size(16, 30), Size(14, 18) },
                { Size(14, 18), Size(16, 30) },
                // ratio larger than 16 (triggers different paths in some OpenGL algorithms)
                { Size(18 * 20, 18 * 20), Size(14, 14) },
                { Size(14, 14), Size(18 * 20, 18 * 20) }
              };
        for (const ScaleSize& scaleSize : scaleSizes)
        {
            OString testStr = "Testing scale (" + scaleSize.srcSize.toString() + ")->("
                              + scaleSize.destSize.toString() + "), method "
                              + OString::number(static_cast<int>(scaleMethod));
            fprintf(stderr, "%s\n", testStr.getStr());
            Bitmap bitmap(scaleSize.srcSize, 24);
            {
                // Fill each quarter of the source bitmap with a different color,
                // and center with yet another color.
                BitmapScopedWriteAccess writeAccess(bitmap);
                const int halfW = scaleSize.srcSize.getWidth() / 2;
                const int halfH = scaleSize.srcSize.getHeight() / 2;
                writeAccess->SetFillColor(COL_GREEN);
                writeAccess->FillRect(Rectangle(Point(0, 0), Size(halfW, halfH)));
                writeAccess->SetFillColor(COL_RED);
                writeAccess->FillRect(Rectangle(Point(0, halfH), Size(halfW, halfH)));
                writeAccess->SetFillColor(COL_YELLOW);
                writeAccess->FillRect(Rectangle(Point(halfW, 0), Size(halfW, halfH)));
                writeAccess->SetFillColor(COL_BLACK);
                writeAccess->FillRect(Rectangle(Point(halfW, halfH), Size(halfW, halfH)));
                writeAccess->SetFillColor(COL_BLUE);
                writeAccess->FillRect(Rectangle(Point(halfW / 2, halfH / 2), Size(halfW, halfH)));
            }
            if (bExportBitmap)
            {
                SvFileStream aStream("~/scale_before.png", StreamMode::WRITE | StreamMode::TRUNC);
                GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
                rFilter.compressAsPNG(bitmap, aStream);
            }
            CPPUNIT_ASSERT(bitmap.Scale(scaleSize.destSize, scaleMethod));
            if (bExportBitmap)
            {
                SvFileStream aStream("~/scale_after.png", StreamMode::WRITE | StreamMode::TRUNC);
                GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
                rFilter.compressAsPNG(bitmap, aStream);
            }
            CPPUNIT_ASSERT_EQUAL(scaleSize.destSize, bitmap.GetSizePixel());
            {
                // Scaling should keep each quarter of the resulting bitmap have the same color,
                // so check that color in each corner of the result bitmap is the same color,
                // or reasonably close (some algorithms may alter the color very slightly).
                BitmapReadAccess readAccess(bitmap);
                const int lastW = scaleSize.destSize.getWidth() - 1;
                const int lastH = scaleSize.destSize.getHeight() - 1;
                assertColorsAreSimilar(2, __LINE__, COL_GREEN, readAccess.GetColor(0, 0));
                assertColorsAreSimilar(2, __LINE__, COL_RED, readAccess.GetColor(lastH, 0));
                assertColorsAreSimilar(2, __LINE__, COL_YELLOW, readAccess.GetColor(0, lastW));
                assertColorsAreSimilar(2, __LINE__, COL_BLACK, readAccess.GetColor(lastH, lastW));
                assertColorsAreSimilar(2, __LINE__, COL_BLUE,
                                       readAccess.GetColor(lastH / 2, lastW / 2));
            }
        }
    }
}

void BitmapScaleTest::testScale2()
{
    const bool bExportBitmap(false);

    Bitmap aBitmap24Bit(Size(4096, 4096), 24);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(24), aBitmap24Bit.GetBitCount());
    Color aBitmapColor = COL_YELLOW;
    {
        BitmapScopedWriteAccess aWriteAccess(aBitmap24Bit);
        aWriteAccess->Erase(aBitmapColor);
    }

    if (bExportBitmap)
    {
        SvFileStream aStream("scale_before.png", StreamMode::WRITE | StreamMode::TRUNC);
        GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
        rFilter.compressAsPNG(aBitmap24Bit, aStream);
    }

    // Scale - 65x65
    CPPUNIT_ASSERT_EQUAL(static_cast<long>(4096), aBitmap24Bit.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(static_cast<long>(4096), aBitmap24Bit.GetSizePixel().Height());
    Bitmap aScaledBitmap = aBitmap24Bit;
    aScaledBitmap.Scale(Size(65, 65));

    if (bExportBitmap)
    {
        SvFileStream aStream("scale_after_65x65.png", StreamMode::WRITE | StreamMode::TRUNC);
        GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
        rFilter.compressAsPNG(aScaledBitmap, aStream);
    }

    CPPUNIT_ASSERT_EQUAL(static_cast<long>(65), aScaledBitmap.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(static_cast<long>(65), aScaledBitmap.GetSizePixel().Height());
    CPPUNIT_ASSERT(checkBitmapColor(aScaledBitmap, aBitmapColor));

    // Scale - 64x64
    CPPUNIT_ASSERT_EQUAL(static_cast<long>(4096), aBitmap24Bit.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(static_cast<long>(4096), aBitmap24Bit.GetSizePixel().Height());
    aScaledBitmap = aBitmap24Bit;
    aScaledBitmap.Scale(Size(64, 64));

    if (bExportBitmap)
    {
        SvFileStream aStream("scale_after_64x64.png", StreamMode::WRITE | StreamMode::TRUNC);
        GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
        rFilter.compressAsPNG(aScaledBitmap, aStream);
    }

    CPPUNIT_ASSERT_EQUAL(static_cast<long>(64), aScaledBitmap.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(static_cast<long>(64), aScaledBitmap.GetSizePixel().Height());
    CPPUNIT_ASSERT(checkBitmapColor(aScaledBitmap, aBitmapColor));

    // Scale - 63x63
    CPPUNIT_ASSERT_EQUAL(static_cast<long>(4096), aBitmap24Bit.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(static_cast<long>(4096), aBitmap24Bit.GetSizePixel().Height());
    aScaledBitmap = aBitmap24Bit;
    aScaledBitmap.Scale(Size(63, 63));

    if (bExportBitmap)
    {
        SvFileStream aStream("scale_after_63x63.png", StreamMode::WRITE | StreamMode::TRUNC);
        GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
        rFilter.compressAsPNG(aScaledBitmap, aStream);
    }

    CPPUNIT_ASSERT_EQUAL(static_cast<long>(63), aScaledBitmap.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(static_cast<long>(63), aScaledBitmap.GetSizePixel().Height());
    CPPUNIT_ASSERT(checkBitmapColor(aScaledBitmap, aBitmapColor));
}

void BitmapScaleTest::testScaleSymmetry()
{
    const bool bExportBitmap(false);

    Bitmap aBitmap24Bit(Size(10, 10), 24);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(24), aBitmap24Bit.GetBitCount());

    {
        BitmapScopedWriteAccess aWriteAccess(aBitmap24Bit);
        aWriteAccess->Erase(COL_WHITE);
        aWriteAccess->SetLineColor(COL_BLACK);
        aWriteAccess->DrawRect(tools::Rectangle(1, 1, 8, 8));
        aWriteAccess->DrawRect(tools::Rectangle(3, 3, 6, 6));
    }

    BitmapSymmetryCheck aBitmapSymmetryCheck;

    CPPUNIT_ASSERT_EQUAL(static_cast<long>(10), aBitmap24Bit.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(static_cast<long>(10), aBitmap24Bit.GetSizePixel().Height());

    // Check symmetry of the bitmap
    CPPUNIT_ASSERT(BitmapSymmetryCheck::check(aBitmap24Bit));

    if (bExportBitmap)
    {
        SvFileStream aStream("~/scale_before.png", StreamMode::WRITE | StreamMode::TRUNC);
        GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
        rFilter.compressAsPNG(aBitmap24Bit, aStream);
    }

    aBitmap24Bit.Scale(2, 2, BmpScaleFlag::Fast);

    CPPUNIT_ASSERT_EQUAL(static_cast<long>(20), aBitmap24Bit.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(static_cast<long>(20), aBitmap24Bit.GetSizePixel().Height());

    // After scaling the bitmap should still be symmetrical. This check guarantees that
    // scaling doesn't misalign the bitmap.
    CPPUNIT_ASSERT(BitmapSymmetryCheck::check(aBitmap24Bit));

    if (bExportBitmap)
    {
        SvFileStream aStream("~/scale_after.png", StreamMode::WRITE | StreamMode::TRUNC);
        GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
        rFilter.compressAsPNG(aBitmap24Bit, aStream);
    }
}

} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(BitmapScaleTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
