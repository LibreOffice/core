/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <vcl/bitmap.hxx>

#include <tools/stream.hxx>
#include <vcl/graphicfilter.hxx>

#include <BitmapSymmetryCheck.hxx>
#include <vcl/BitmapWriteAccess.hxx>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>

namespace
{
class BitmapScaleTest : public CppUnit::TestFixture
{
    void testScaleImpl(vcl::PixelFormat ePixelFormat);
    void testScale2Impl(vcl::PixelFormat ePixelFormat);
    void testLargeReductionKeepsColorImpl(vcl::PixelFormat ePixelFormat);

    void testScale() { testScaleImpl(vcl::PixelFormat::N24_BPP); }
    void testScaleN32() { testScaleImpl(vcl::PixelFormat::N32_BPP); }
    void testScale2() { testScale2Impl(vcl::PixelFormat::N24_BPP); }
    void testScale2N32() { testScale2Impl(vcl::PixelFormat::N32_BPP); }
    void testLargeReductionKeepsColor()
    {
        testLargeReductionKeepsColorImpl(vcl::PixelFormat::N24_BPP);
    }
    void testLargeReductionKeepsColorN32()
    {
        testLargeReductionKeepsColorImpl(vcl::PixelFormat::N32_BPP);
    }
    void testLargeReductionKeepsColorN8()
    {
        testLargeReductionKeepsColorImpl(vcl::PixelFormat::N8_BPP);
    }
    void testLargeReductionInOneDirectionAveragesColumns();
    void testLargeReductionIncludesEdgePixels();
    void testMirrorOnlyKeepsPixels();
    void testScaleSymmetry();
    void benchmarkScale();

    CPPUNIT_TEST_SUITE(BitmapScaleTest);
    CPPUNIT_TEST(testScale);
    CPPUNIT_TEST(testScaleN32);
    CPPUNIT_TEST(testScale2);
    CPPUNIT_TEST(testScale2N32);
    CPPUNIT_TEST(testLargeReductionKeepsColor);
    CPPUNIT_TEST(testLargeReductionKeepsColorN32);
    CPPUNIT_TEST(testLargeReductionKeepsColorN8);
    CPPUNIT_TEST(testLargeReductionInOneDirectionAveragesColumns);
    CPPUNIT_TEST(testLargeReductionIncludesEdgePixels);
    CPPUNIT_TEST(testMirrorOnlyKeepsPixels);
    CPPUNIT_TEST(testScaleSymmetry);
    CPPUNIT_TEST(benchmarkScale);
    CPPUNIT_TEST_SUITE_END();
};

bool checkBitmapColor(Bitmap const& rBitmap, Color const& rExpectedColor)
{
    bool bResult = true;
    Bitmap aBitmap(rBitmap);
    BitmapScopedReadAccess pReadAccess(aBitmap);
    tools::Long nHeight = pReadAccess->Height();
    tools::Long nWidth = pReadAccess->Width();
    for (tools::Long y = 0; y < nHeight; ++y)
    {
        Scanline pScanlineRead = pReadAccess->GetScanline(y);
        for (tools::Long x = 0; x < nWidth; ++x)
        {
            Color aColor = pReadAccess->GetColorFromData(pScanlineRead, x);
            if (aColor != rExpectedColor)
                bResult = false;
        }
    }

    return bResult;
}

void assertColorsAreSimilar(int maxDifference, int line, const BitmapColor& expected,
                            const BitmapColor& actual)
{
    // Check that the two colors match or are reasonably similar.
    if (expected.GetColorError(actual) <= maxDifference)
        return;

    std::stringstream stream;
    stream << "Line: " << line;

    CPPUNIT_ASSERT_EQUAL_MESSAGE(stream.str(), expected, actual);
}

void BitmapScaleTest::testScaleImpl(vcl::PixelFormat ePixelFormat)
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
                { Size(14, 14), Size(18 * 20, 18 * 20) },
                // Boundary cases.
                { Size(1, 1), Size(1, 1) },
                { Size(16, 1), Size(12, 1) },
                { Size(1, 16), Size(1, 12) }
              };
        for (const ScaleSize& scaleSize : scaleSizes)
        {
            OString testStr = "Testing scale (" + scaleSize.srcSize.toString() + ")->("
                              + scaleSize.destSize.toString() + "), method "
                              + OString::number(int(scaleMethod)) + ", "
                              + OString::number(vcl::pixelFormatBitCount(ePixelFormat)) + " bpp";
            fprintf(stderr, "%s\n", testStr.getStr());
            Bitmap bitmap(scaleSize.srcSize, ePixelFormat);
            {
                // Fill each quarter of the source bitmap with a different color,
                // and center with yet another color.
                BitmapScopedWriteAccess writeAccess(bitmap);
                const int halfW = scaleSize.srcSize.getWidth() / 2;
                const int halfH = scaleSize.srcSize.getHeight() / 2;
                const Size aSize(std::max(halfW, 1), std::max(halfH, 1));

                writeAccess->SetFillColor(COL_GREEN);
                writeAccess->FillRect(Rectangle(Point(0, 0), aSize));
                writeAccess->SetFillColor(COL_RED);
                writeAccess->FillRect(Rectangle(Point(0, halfH), aSize));
                writeAccess->SetFillColor(COL_YELLOW);
                writeAccess->FillRect(Rectangle(Point(halfW, 0), aSize));
                writeAccess->SetFillColor(COL_BLACK);
                writeAccess->FillRect(Rectangle(Point(halfW, halfH), aSize));
                writeAccess->SetFillColor(COL_BLUE);
                writeAccess->FillRect(Rectangle(Point(halfW / 2, halfH / 2), aSize));
            }
            if (bExportBitmap)
            {
                SvFileStream aStream(u"~/scale_before.png"_ustr,
                                     StreamMode::WRITE | StreamMode::TRUNC);
                GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
                rFilter.compressAsPNG(bitmap, aStream);
            }
            CPPUNIT_ASSERT(bitmap.Scale(scaleSize.destSize, scaleMethod));
            if (bExportBitmap)
            {
                SvFileStream aStream(u"~/scale_after.png"_ustr,
                                     StreamMode::WRITE | StreamMode::TRUNC);
                GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
                rFilter.compressAsPNG(bitmap, aStream);
            }
            CPPUNIT_ASSERT_EQUAL(scaleSize.destSize, bitmap.GetSizePixel());
            const int lastW = scaleSize.destSize.getWidth() - 1;
            const int lastH = scaleSize.destSize.getHeight() - 1;
            if (scaleSize.srcSize.getWidth() == 1 && scaleSize.srcSize.getHeight() == 1)
            {
                BitmapReadAccess readAccess(bitmap);
                assertColorsAreSimilar(2, __LINE__, COL_BLUE, readAccess.GetColor(0, 0));
                assertColorsAreSimilar(2, __LINE__, COL_BLUE, readAccess.GetColor(lastH, 0));
                assertColorsAreSimilar(2, __LINE__, COL_BLUE, readAccess.GetColor(0, lastW));
                assertColorsAreSimilar(2, __LINE__, COL_BLUE, readAccess.GetColor(lastH, lastW));
                assertColorsAreSimilar(2, __LINE__, COL_BLUE,
                                       readAccess.GetColor(lastH / 2, lastW / 2));
            }
            else if (lastW && lastH)
            {
                // Scaling should keep each quarter of the resulting bitmap have the same color,
                // so check that color in each corner of the result bitmap is the same color,
                // or reasonably close (some algorithms may alter the color very slightly).
                BitmapReadAccess readAccess(bitmap);
                assertColorsAreSimilar(2, __LINE__, COL_GREEN, readAccess.GetColor(0, 0));
                assertColorsAreSimilar(2, __LINE__, COL_RED, readAccess.GetColor(lastH, 0));
                assertColorsAreSimilar(2, __LINE__, COL_YELLOW, readAccess.GetColor(0, lastW));
                assertColorsAreSimilar(2, __LINE__, COL_BLACK, readAccess.GetColor(lastH, lastW));
                assertColorsAreSimilar(2, __LINE__, COL_BLUE,
                                       readAccess.GetColor(lastH / 2, lastW / 2));
            }
            else if (lastW)
            {
                BitmapReadAccess readAccess(bitmap);
                assertColorsAreSimilar(2, __LINE__, COL_RED, readAccess.GetColor(0, 0));
                assertColorsAreSimilar(2, __LINE__, COL_BLACK, readAccess.GetColor(0, lastW));
                // Lanczos ringing from a color edge 3 source pixels away adds about 1.4.
                assertColorsAreSimilar(3, __LINE__, COL_BLUE, readAccess.GetColor(0, lastW / 2));
            }
            else if (lastH)
            {
                BitmapReadAccess readAccess(bitmap);
                assertColorsAreSimilar(2, __LINE__, COL_YELLOW, readAccess.GetColor(0, 0));
                assertColorsAreSimilar(2, __LINE__, COL_BLACK, readAccess.GetColor(lastH, 0));
                // Lanczos ringing from a color edge 3 source pixels away adds about 1.4.
                assertColorsAreSimilar(3, __LINE__, COL_BLUE, readAccess.GetColor(lastH / 2, 0));
            }
        }
    }
}

void BitmapScaleTest::testScale2Impl(vcl::PixelFormat ePixelFormat)
{
    const bool bExportBitmap(false);

    Bitmap aBitmap(Size(4096, 4096), ePixelFormat);
    CPPUNIT_ASSERT_EQUAL(ePixelFormat, aBitmap.getPixelFormat());
    Color aBitmapColor = COL_YELLOW;
    {
        BitmapScopedWriteAccess aWriteAccess(aBitmap);
        aWriteAccess->Erase(aBitmapColor);
    }

    if (bExportBitmap)
    {
        SvFileStream aStream(u"scale_before.png"_ustr, StreamMode::WRITE | StreamMode::TRUNC);
        GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
        rFilter.compressAsPNG(aBitmap, aStream);
    }

    // Scale - 65x65
    CPPUNIT_ASSERT_EQUAL(tools::Long(4096), aBitmap.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(4096), aBitmap.GetSizePixel().Height());
    Bitmap aScaledBitmap = aBitmap;
    aScaledBitmap.Scale(Size(65, 65));

    if (bExportBitmap)
    {
        SvFileStream aStream(u"scale_after_65x65.png"_ustr, StreamMode::WRITE | StreamMode::TRUNC);
        GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
        rFilter.compressAsPNG(aScaledBitmap, aStream);
    }

    CPPUNIT_ASSERT_EQUAL(static_cast<tools::Long>(65), aScaledBitmap.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(static_cast<tools::Long>(65), aScaledBitmap.GetSizePixel().Height());
    CPPUNIT_ASSERT(checkBitmapColor(aScaledBitmap, aBitmapColor));

    // Scale - 64x64
    CPPUNIT_ASSERT_EQUAL(tools::Long(4096), aBitmap.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(4096), aBitmap.GetSizePixel().Height());
    aScaledBitmap = aBitmap;
    aScaledBitmap.Scale(Size(64, 64));

    if (bExportBitmap)
    {
        SvFileStream aStream(u"scale_after_64x64.png"_ustr, StreamMode::WRITE | StreamMode::TRUNC);
        GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
        rFilter.compressAsPNG(aScaledBitmap, aStream);
    }

    CPPUNIT_ASSERT_EQUAL(static_cast<tools::Long>(64), aScaledBitmap.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(static_cast<tools::Long>(64), aScaledBitmap.GetSizePixel().Height());
    CPPUNIT_ASSERT(checkBitmapColor(aScaledBitmap, aBitmapColor));

    // Scale - 63x63
    CPPUNIT_ASSERT_EQUAL(tools::Long(4096), aBitmap.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(4096), aBitmap.GetSizePixel().Height());
    aScaledBitmap = aBitmap;
    aScaledBitmap.Scale(Size(63, 63));

    if (bExportBitmap)
    {
        SvFileStream aStream(u"scale_after_63x63.png"_ustr, StreamMode::WRITE | StreamMode::TRUNC);
        GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
        rFilter.compressAsPNG(aScaledBitmap, aStream);
    }

    CPPUNIT_ASSERT_EQUAL(static_cast<tools::Long>(63), aScaledBitmap.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(static_cast<tools::Long>(63), aScaledBitmap.GetSizePixel().Height());
    CPPUNIT_ASSERT(checkBitmapColor(aScaledBitmap, aBitmapColor));
}

void BitmapScaleTest::testLargeReductionKeepsColorImpl(vcl::PixelFormat ePixelFormat)
{
    // A single-color bitmap keeps its exact color after a large reduction. The odd size leaves
    // partial blocks at the right and bottom edge.
    Bitmap aBitmap(Size(4099, 4097), ePixelFormat);
    {
        BitmapScopedWriteAccess pWriteAccess(aBitmap);
        pWriteAccess->Erase(COL_YELLOW);
    }

    for (BmpScaleFlag eScaleFlag :
         { BmpScaleFlag::Lanczos, BmpScaleFlag::BiCubic, BmpScaleFlag::BiLinear })
    {
        Bitmap aScaled(aBitmap);
        CPPUNIT_ASSERT(aScaled.Scale(Size(63, 65), eScaleFlag));
        CPPUNIT_ASSERT_EQUAL(Size(63, 65), aScaled.GetSizePixel());
        CPPUNIT_ASSERT_EQUAL(ePixelFormat, aScaled.getPixelFormat());
        CPPUNIT_ASSERT(checkBitmapColor(aScaled, COL_YELLOW));
    }
}

void BitmapScaleTest::testLargeReductionInOneDirectionAveragesColumns()
{
    // Alternating black and white columns, reduced 5000 times in width only, average to gray.
    Bitmap aBitmap(Size(40000, 2), vcl::PixelFormat::N24_BPP);
    {
        BitmapScopedWriteAccess pWriteAccess(aBitmap);
        for (tools::Long y = 0; y < 2; y++)
            for (tools::Long x = 0; x < 40000; x++)
                pWriteAccess->SetPixel(y, x, x % 2 ? COL_WHITE : COL_BLACK);
    }

    for (BmpScaleFlag eScaleFlag :
         { BmpScaleFlag::Lanczos, BmpScaleFlag::BiCubic, BmpScaleFlag::BiLinear })
    {
        Bitmap aScaled(aBitmap);
        CPPUNIT_ASSERT(aScaled.Scale(Size(8, 2), eScaleFlag));
        CPPUNIT_ASSERT_EQUAL(Size(8, 2), aScaled.GetSizePixel());

        BitmapScopedReadAccess pReadAccess(aScaled);
        for (tools::Long x = 0; x < 8; x++)
            assertColorsAreSimilar(3, __LINE__, Color(128, 128, 128), pReadAccess->GetColor(0, x));
    }
}

void BitmapScaleTest::testLargeReductionIncludesEdgePixels()
{
    // 507 is 3 more than 8 times 63, so the block average of the reduction to 63 has a partial
    // block at the far edge. The 3 black source rows or columns there darken the last destination
    // row or column, while the first one stays white.
    auto checkEdge = [](const Size& rSourceSize, const Size& rTargetSize)
    {
        Bitmap aBitmap(rSourceSize, vcl::PixelFormat::N24_BPP);
        {
            BitmapScopedWriteAccess pWriteAccess(aBitmap);
            pWriteAccess->Erase(COL_WHITE);
            for (tools::Long y = 0; y < rSourceSize.Height(); y++)
            {
                for (tools::Long x = 0; x < rSourceSize.Width(); x++)
                {
                    const bool bLastColumns = rSourceSize.Width() == 507 && x >= 504;
                    const bool bLastRows = rSourceSize.Height() == 507 && y >= 504;
                    if (bLastColumns || bLastRows)
                        pWriteAccess->SetPixel(y, x, COL_BLACK);
                }
            }
        }

        for (BmpScaleFlag eScaleFlag :
             { BmpScaleFlag::Lanczos, BmpScaleFlag::BiCubic, BmpScaleFlag::BiLinear })
        {
            Bitmap aScaled(aBitmap);
            CPPUNIT_ASSERT(aScaled.Scale(rTargetSize, eScaleFlag));
            CPPUNIT_ASSERT_EQUAL(rTargetSize, aScaled.GetSizePixel());

            BitmapScopedReadAccess pReadAccess(aScaled);
            const tools::Long nLastX = rTargetSize.Width() - 1;
            const tools::Long nLastY = rTargetSize.Height() - 1;
            assertColorsAreSimilar(1, __LINE__, COL_WHITE, pReadAccess->GetColor(0, 0));
            if (rSourceSize.Width() == 507)
                CPPUNIT_ASSERT_LESS(sal_uInt8(240), pReadAccess->GetColor(0, nLastX).GetRed());
            if (rSourceSize.Height() == 507)
                CPPUNIT_ASSERT_LESS(sal_uInt8(240), pReadAccess->GetColor(nLastY, 0).GetRed());
        }
    };

    checkEdge(Size(507, 507), Size(63, 63));
    checkEdge(Size(507, 2), Size(63, 2));
    checkEdge(Size(2, 507), Size(2, 63));
}

void BitmapScaleTest::testMirrorOnlyKeepsPixels()
{
    // A negative scale of 1 mirrors without resampling, so every pixel keeps its exact color.
    Bitmap aBitmap(Size(4, 2), vcl::PixelFormat::N24_BPP);
    {
        BitmapScopedWriteAccess pWriteAccess(aBitmap);
        pWriteAccess->SetPixel(0, 0, COL_RED);
        pWriteAccess->SetPixel(0, 1, COL_GREEN);
        pWriteAccess->SetPixel(0, 2, COL_BLUE);
        pWriteAccess->SetPixel(0, 3, COL_YELLOW);
        pWriteAccess->SetPixel(1, 0, COL_BLACK);
        pWriteAccess->SetPixel(1, 1, COL_WHITE);
        pWriteAccess->SetPixel(1, 2, COL_CYAN);
        pWriteAccess->SetPixel(1, 3, COL_MAGENTA);
    }

    Bitmap aMirroredX(aBitmap);
    CPPUNIT_ASSERT(aMirroredX.Scale(-1.0, 1.0, BmpScaleFlag::Lanczos));
    CPPUNIT_ASSERT_EQUAL(Size(4, 2), aMirroredX.GetSizePixel());
    {
        BitmapScopedReadAccess pReadAccess(aMirroredX);
        CPPUNIT_ASSERT_EQUAL(BitmapColor(COL_YELLOW), pReadAccess->GetColor(0, 0));
        CPPUNIT_ASSERT_EQUAL(BitmapColor(COL_RED), pReadAccess->GetColor(0, 3));
        CPPUNIT_ASSERT_EQUAL(BitmapColor(COL_MAGENTA), pReadAccess->GetColor(1, 0));
        CPPUNIT_ASSERT_EQUAL(BitmapColor(COL_BLACK), pReadAccess->GetColor(1, 3));
    }

    Bitmap aMirroredY(aBitmap);
    CPPUNIT_ASSERT(aMirroredY.Scale(1.0, -1.0, BmpScaleFlag::BiCubic));
    CPPUNIT_ASSERT_EQUAL(Size(4, 2), aMirroredY.GetSizePixel());
    {
        BitmapScopedReadAccess pReadAccess(aMirroredY);
        CPPUNIT_ASSERT_EQUAL(BitmapColor(COL_BLACK), pReadAccess->GetColor(0, 0));
        CPPUNIT_ASSERT_EQUAL(BitmapColor(COL_MAGENTA), pReadAccess->GetColor(0, 3));
        CPPUNIT_ASSERT_EQUAL(BitmapColor(COL_RED), pReadAccess->GetColor(1, 0));
        CPPUNIT_ASSERT_EQUAL(BitmapColor(COL_YELLOW), pReadAccess->GetColor(1, 3));
    }
}

// Times the default scaler against the convolution scalers on a photo-like bitmap and prints a
// table to stderr. It runs only with VCL_SCALE_BENCHMARK set, since it takes a few seconds and
// asserts nothing. Set VCL_NO_THREAD_SCALE as well to compare on one thread.
void BitmapScaleTest::benchmarkScale()
{
    if (!std::getenv("VCL_SCALE_BENCHMARK"))
        return;

    auto makePhotoLikeBitmap = [](const Size& rSize, vcl::PixelFormat ePixelFormat)
    {
        Bitmap aBitmap(rSize, ePixelFormat);
        BitmapScopedWriteAccess pWriteAccess(aBitmap);
        for (tools::Long y = 0; y < rSize.Height(); y++)
        {
            for (tools::Long x = 0; x < rSize.Width(); x++)
            {
                const sal_uInt8 nRed
                    = sal_uInt8(128 + 100 * std::sin(x * 0.01) * std::cos(y * 0.013));
                const sal_uInt8 nGreen = sal_uInt8((x * 7 + y * 3) & 255);
                const sal_uInt8 nBlue = ((x / 37 + y / 41) & 1) ? 230 : 20;
                pWriteAccess->SetPixel(y, x, BitmapColor(nRed, nGreen, nBlue));
            }
        }
        return aBitmap;
    };

    // The best of 3 runs, in milliseconds.
    auto timeScale = [](const Bitmap& rBitmap, const Size& rTargetSize, BmpScaleFlag eScaleFlag)
    {
        double fBest = 1e30;
        for (int i = 0; i < 3; i++)
        {
            Bitmap aScaled(rBitmap);
            const auto aStart = std::chrono::steady_clock::now();
            aScaled.Scale(rTargetSize, eScaleFlag);
            const std::chrono::duration<double, std::milli> aTime
                = std::chrono::steady_clock::now() - aStart;
            fBest = std::min(fBest, aTime.count());
        }
        return fBest;
    };

    struct Case
    {
        Size aSourceSize;
        Size aTargetSize;
    };
    const Case aCases[] = {
        { Size(4000, 3000), Size(3200, 2400) }, { Size(4000, 3000), Size(2800, 2100) },
        { Size(4000, 3000), Size(2000, 1500) }, { Size(4000, 3000), Size(1000, 750) },
        { Size(4000, 3000), Size(256, 192) },   { Size(1000, 750), Size(2000, 1500) },
        { Size(400, 300), Size(1600, 1200) },
    };

    fprintf(stderr, "time in ms, best of 3%s\n",
            std::getenv("VCL_NO_THREAD_SCALE") ? ", one thread" : "");
    fprintf(stderr, "%-4s %-22s %9s %9s %9s %9s\n", "bpp", "scale", "default", "lanczos", "bicubic",
            "bilinear");
    for (vcl::PixelFormat ePixelFormat : { vcl::PixelFormat::N24_BPP, vcl::PixelFormat::N32_BPP })
    {
        for (const Case& rCase : aCases)
        {
            const Bitmap aBitmap = makePhotoLikeBitmap(rCase.aSourceSize, ePixelFormat);
            const OString aName
                = rCase.aSourceSize.toString() + " to " + rCase.aTargetSize.toString();
            fprintf(stderr, "%-4d %-22s %9.1f %9.1f %9.1f %9.1f\n",
                    int(vcl::pixelFormatBitCount(ePixelFormat)), aName.getStr(),
                    timeScale(aBitmap, rCase.aTargetSize, BmpScaleFlag::Default),
                    timeScale(aBitmap, rCase.aTargetSize, BmpScaleFlag::Lanczos),
                    timeScale(aBitmap, rCase.aTargetSize, BmpScaleFlag::BiCubic),
                    timeScale(aBitmap, rCase.aTargetSize, BmpScaleFlag::BiLinear));
        }
    }
}

void BitmapScaleTest::testScaleSymmetry()
{
    const bool bExportBitmap(false);

    Bitmap aBitmap24Bit(Size(10, 10), vcl::PixelFormat::N24_BPP);
    CPPUNIT_ASSERT_EQUAL(vcl::PixelFormat::N24_BPP, aBitmap24Bit.getPixelFormat());

    {
        BitmapScopedWriteAccess aWriteAccess(aBitmap24Bit);
        aWriteAccess->Erase(COL_WHITE);
        aWriteAccess->SetLineColor(COL_BLACK);
        aWriteAccess->DrawRect(tools::Rectangle(1, 1, 8, 8));
        aWriteAccess->DrawRect(tools::Rectangle(3, 3, 6, 6));
    }

    BitmapSymmetryCheck aBitmapSymmetryCheck;

    CPPUNIT_ASSERT_EQUAL(static_cast<tools::Long>(10), aBitmap24Bit.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(static_cast<tools::Long>(10), aBitmap24Bit.GetSizePixel().Height());

    // Check symmetry of the bitmap
    CPPUNIT_ASSERT(BitmapSymmetryCheck::check(aBitmap24Bit));

    if (bExportBitmap)
    {
        SvFileStream aStream(u"~/scale_before.png"_ustr, StreamMode::WRITE | StreamMode::TRUNC);
        GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
        rFilter.compressAsPNG(aBitmap24Bit, aStream);
    }

    aBitmap24Bit.Scale(2, 2, BmpScaleFlag::Fast);

    CPPUNIT_ASSERT_EQUAL(static_cast<tools::Long>(20), aBitmap24Bit.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(static_cast<tools::Long>(20), aBitmap24Bit.GetSizePixel().Height());

    // After scaling the bitmap should still be symmetrical. This check guarantees that
    // scaling doesn't misalign the bitmap.
    CPPUNIT_ASSERT(BitmapSymmetryCheck::check(aBitmap24Bit));

    if (bExportBitmap)
    {
        SvFileStream aStream(u"~/scale_after.png"_ustr, StreamMode::WRITE | StreamMode::TRUNC);
        GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
        rFilter.compressAsPNG(aBitmap24Bit, aStream);
    }
}

} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(BitmapScaleTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
