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
#include <vcl/bitmapaccess.hxx>

#include <tools/stream.hxx>
#include <vcl/graphicfilter.hxx>

#include <vcl/BitmapFilterStackBlur.hxx>
#include "BitmapSymmetryCheck.hxx"

#include <chrono>

namespace
{

const bool constWriteResultBitmap(false);
const bool constEnablePerformanceTest(false);

class BitmapFilterTest : public CppUnit::TestFixture
{

    void testBlurCorrectness();
    void testPerformance();

    CPPUNIT_TEST_SUITE(BitmapFilterTest);
    CPPUNIT_TEST(testBlurCorrectness);
    CPPUNIT_TEST(testPerformance);
    CPPUNIT_TEST_SUITE_END();
};

void BitmapFilterTest::testBlurCorrectness()
{
    // Setup test bitmap
    Size aSize(41, 31);
    Bitmap aBitmap24Bit(aSize, 24);

    sal_uLong nScanlineFormat = 0;
    sal_uInt16 nBPP = aBitmap24Bit.GetBitCount();

    {
        long aMargin1 = 1;
        long aMargin2 = 3;
        Bitmap::ScopedWriteAccess aWriteAccess(aBitmap24Bit);
        nScanlineFormat = aWriteAccess->GetScanlineFormat();
        aWriteAccess->Erase(COL_WHITE);
        aWriteAccess->SetLineColor(COL_BLACK);

        Rectangle aRectangle1(
                    aMargin1,
                    aMargin1,
                    aSize.Width() - 1 - aMargin1,
                    aSize.Height() - 1 - aMargin1);

        Rectangle aRectangle2(
                    aMargin2,
                    aMargin2,
                    aSize.Width() - 1 - aMargin2,
                    aSize.Height() - 1 - aMargin2);

        Rectangle aRectangle3(
                    aSize.Width()  / 2,
                    aSize.Height() / 2,
                    aSize.Width()  / 2,
                    aSize.Height() / 2);

        aWriteAccess->DrawRect(aRectangle1);
        aWriteAccess->DrawRect(aRectangle2);
        aWriteAccess->DrawRect(aRectangle3);
    }

    if (constWriteResultBitmap)
    {
        SvFileStream aStream(OUString("~/blurBefore.png"), StreamMode::WRITE | StreamMode::TRUNC);
        GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
        rFilter.compressAsPNG(aBitmap24Bit, aStream, 1);
    }

    // Perform blur
    BitmapFilterStackBlur aBlurFilter(2);
    aBlurFilter.filter(aBitmap24Bit);

    // Check the result

    if (constWriteResultBitmap)
    {
        SvFileStream aStream(OUString("~/blurAfter.png"), StreamMode::WRITE | StreamMode::TRUNC);
        GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
        rFilter.compressAsPNG(aBitmap24Bit, aStream, 1);
    }

    // Check blurred bitmap parameters
    CPPUNIT_ASSERT_EQUAL(static_cast<long>(45), aBitmap24Bit.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(static_cast<long>(35), aBitmap24Bit.GetSizePixel().Height());

    CPPUNIT_ASSERT_EQUAL(nBPP, aBitmap24Bit.GetBitCount());

    // Check that the bitmap is horizontally and vertically symmetrical
    BitmapSymmetryCheck symmetryCheck;
    CPPUNIT_ASSERT(BitmapSymmetryCheck::check(aBitmap24Bit));

    {
        Bitmap::ScopedReadAccess aReadAccess(aBitmap24Bit);
        CPPUNIT_ASSERT_EQUAL(nScanlineFormat, aReadAccess->GetScanlineFormat());
    }
}

void BitmapFilterTest::testPerformance()
{
    if (!constEnablePerformanceTest)
        return;

    Size aSize(4000, 3000); // A rather common picture size

    // Prepare bitmap
    Bitmap aBigBitmap(aSize, 24);
    {
        long aMargin = 500;
        Bitmap::ScopedWriteAccess aWriteAccess(aBigBitmap);
        aWriteAccess->Erase(COL_WHITE);
        aWriteAccess->SetLineColor(COL_BLACK);
        aWriteAccess->SetFillColor(COL_BLACK);
        Rectangle aRectangle(
                    aMargin,
                    aMargin,
                    aSize.Width() - 1 - aMargin,
                    aSize.Height() - 1 - aMargin);

        aWriteAccess->DrawRect(aRectangle);
    }

    int nIterations = 10;
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < nIterations; i++)
    {
        {
            BitmapFilterStackBlur aBlurFilter(250, false); // don't extend the image
            aBlurFilter.filter(aBigBitmap);
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = (end - start) / nIterations;

    if (constWriteResultBitmap)
    {
        std::unique_ptr<SvFileStream> pStream(
            new SvFileStream(OUString("~/BlurBigPerformance.png"), StreamMode::WRITE | StreamMode::TRUNC));
        GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
        rFilter.compressAsPNG(aBigBitmap, *pStream, 1);

        pStream.reset(
            new SvFileStream(OUString("~/BlurBigPerformance.txt"), StreamMode::WRITE));
        pStream->WriteOString(OString("Blur average time: "));
        pStream->WriteOString(OString::number(std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count()));
        pStream->WriteOString(OString("\n"));
    }
}

} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(BitmapFilterTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
