/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>

#include <vcl/bitmap.hxx>
#include <bitmap/BitmapWriteAccess.hxx>

#include <tools/stream.hxx>
#include <vcl/graphicfilter.hxx>

#include <vcl/BitmapBasicMorphologyFilter.hxx>
#include <vcl/BitmapFilterStackBlur.hxx>
#include <BitmapSymmetryCheck.hxx>

#include <chrono>

namespace
{
constexpr bool constWriteResultBitmap(false);
constexpr bool constEnablePerformanceTest(false);

class BitmapFilterTest : public test::BootstrapFixture
{
public:
    BitmapFilterTest()
        : test::BootstrapFixture(true, false)
    {
    }

    void testBlurCorrectness();
    void testBasicMorphology();
    void testPerformance();
    void testGenerateStripRanges();

    CPPUNIT_TEST_SUITE(BitmapFilterTest);
    CPPUNIT_TEST(testBlurCorrectness);
    CPPUNIT_TEST(testBasicMorphology);
    CPPUNIT_TEST(testPerformance);
    CPPUNIT_TEST(testGenerateStripRanges);
    CPPUNIT_TEST_SUITE_END();

private:
    OUString getFullUrl(const OUString& sFileName)
    {
        return m_directories.getURLFromSrc("vcl/qa/cppunit/data/") + sFileName;
    }

    BitmapEx loadBitmap(const OUString& sFileName)
    {
        Graphic aGraphic;
        const OUString aURL(getFullUrl(sFileName));
        SvFileStream aFileStream(aURL, StreamMode::READ);
        GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
        ErrCode aResult = rFilter.ImportGraphic(aGraphic, aURL, aFileStream);
        CPPUNIT_ASSERT_EQUAL(ERRCODE_NONE, aResult);
        return aGraphic.GetBitmapEx();
    }

    template <class BitmapT> // handle both Bitmap and BitmapEx
    void savePNG(const OUString& sWhere, const BitmapT& rBmp)
    {
        SvFileStream aStream(sWhere, StreamMode::WRITE | StreamMode::TRUNC);
        GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
        rFilter.compressAsPNG(BitmapEx(rBmp), aStream);
    }
};

void BitmapFilterTest::testBlurCorrectness()
{
    // Setup test bitmap
    Size aSize(41, 31);
    Bitmap aBitmap24Bit(aSize, 24);

    ScanlineFormat scanlineFormat = ScanlineFormat::NONE;
    sal_uInt16 nBPP = aBitmap24Bit.GetBitCount();

    {
        tools::Long aMargin1 = 1;
        tools::Long aMargin2 = 3;
        BitmapScopedWriteAccess aWriteAccess(aBitmap24Bit);
        scanlineFormat = aWriteAccess->GetScanlineFormat();
        aWriteAccess->Erase(COL_WHITE);
        aWriteAccess->SetLineColor(COL_BLACK);

        tools::Rectangle aRectangle1(aMargin1, aMargin1, aSize.Width() - 1 - aMargin1,
                                     aSize.Height() - 1 - aMargin1);

        tools::Rectangle aRectangle2(aMargin2, aMargin2, aSize.Width() - 1 - aMargin2,
                                     aSize.Height() - 1 - aMargin2);

        tools::Rectangle aRectangle3(aSize.Width() / 2, aSize.Height() / 2, aSize.Width() / 2,
                                     aSize.Height() / 2);

        aWriteAccess->DrawRect(aRectangle1);
        aWriteAccess->DrawRect(aRectangle2);
        aWriteAccess->DrawRect(aRectangle3);
    }

    if (constWriteResultBitmap)
    {
        savePNG("~/blurBefore.png", aBitmap24Bit);
    }

    // Perform blur
    BitmapFilterStackBlur aBlurFilter(2);
    aBitmap24Bit = aBlurFilter.filter(aBitmap24Bit);

    // Check the result

    if (constWriteResultBitmap)
    {
        savePNG("~/blurAfter.png", aBitmap24Bit);
    }

    // Check blurred bitmap parameters
    CPPUNIT_ASSERT_EQUAL(static_cast<tools::Long>(41), aBitmap24Bit.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(static_cast<tools::Long>(31), aBitmap24Bit.GetSizePixel().Height());

    CPPUNIT_ASSERT_EQUAL(nBPP, aBitmap24Bit.GetBitCount());

    // Check that the bitmap is horizontally and vertically symmetrical
    CPPUNIT_ASSERT(BitmapSymmetryCheck::check(aBitmap24Bit));

    {
        Bitmap::ScopedReadAccess aReadAccess(aBitmap24Bit);
        CPPUNIT_ASSERT_EQUAL(scanlineFormat, aReadAccess->GetScanlineFormat());
    }
}

void BitmapFilterTest::testBasicMorphology()
{
    const BitmapEx aOrigBitmap = loadBitmap("testBasicMorphology.png");
    const BitmapEx aRefBitmapDilated1 = loadBitmap("testBasicMorphologyDilated1.png");
    const BitmapEx aRefBitmapDilated1Eroded1 = loadBitmap("testBasicMorphologyDilated1Eroded1.png");
    const BitmapEx aRefBitmapDilated2 = loadBitmap("testBasicMorphologyDilated2.png");
    const BitmapEx aRefBitmapDilated2Eroded1 = loadBitmap("testBasicMorphologyDilated2Eroded1.png");

    BitmapEx aTransformBitmap = aOrigBitmap;
    BitmapFilter::Filter(aTransformBitmap, BitmapDilateFilter(1));
    if (constWriteResultBitmap)
        savePNG("~/Dilated1.png", aTransformBitmap);
    CPPUNIT_ASSERT_EQUAL(aRefBitmapDilated1.GetChecksum(), aTransformBitmap.GetChecksum());
    BitmapFilter::Filter(aTransformBitmap, BitmapErodeFilter(1));
    if (constWriteResultBitmap)
        savePNG("~/Dilated1Eroded1.png", aTransformBitmap);
    CPPUNIT_ASSERT_EQUAL(aRefBitmapDilated1Eroded1.GetChecksum(), aTransformBitmap.GetChecksum());

    aTransformBitmap = aOrigBitmap;
    BitmapFilter::Filter(aTransformBitmap, BitmapDilateFilter(2));
    if (constWriteResultBitmap)
        savePNG("~/Dilated2.png", aTransformBitmap);
    CPPUNIT_ASSERT_EQUAL(aRefBitmapDilated2.GetChecksum(), aTransformBitmap.GetChecksum());
    BitmapFilter::Filter(aTransformBitmap, BitmapErodeFilter(1));
    if (constWriteResultBitmap)
        savePNG("~/Dilated2Eroded1.png", aTransformBitmap);
    CPPUNIT_ASSERT_EQUAL(aRefBitmapDilated2Eroded1.GetChecksum(), aTransformBitmap.GetChecksum());
}

void BitmapFilterTest::testPerformance()
{
    if (!constEnablePerformanceTest)
        return;

    Size aSize(4000, 3000); // A rather common picture size

    // Prepare bitmap
    Bitmap aBigBitmap(aSize, 24);
    {
        tools::Long aMargin = 500;
        BitmapScopedWriteAccess aWriteAccess(aBigBitmap);
        aWriteAccess->Erase(COL_WHITE);
        aWriteAccess->SetLineColor(COL_BLACK);
        aWriteAccess->SetFillColor(COL_BLACK);
        tools::Rectangle aRectangle(aMargin, aMargin, aSize.Width() - 1 - aMargin,
                                    aSize.Height() - 1 - aMargin);

        aWriteAccess->DrawRect(aRectangle);
    }

    int nIterations = 10;
    auto start = std::chrono::high_resolution_clock::now();
    Bitmap aResult;
    for (int i = 0; i < nIterations; i++)
    {
        BitmapFilterStackBlur aBlurFilter(250);
        aResult = aBlurFilter.filter(aBigBitmap);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = (end - start) / nIterations;

    if (constWriteResultBitmap)
    {
        std::unique_ptr<SvFileStream> pStream(
            new SvFileStream("~/BlurBigPerformance.png", StreamMode::WRITE | StreamMode::TRUNC));
        GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
        rFilter.compressAsPNG(BitmapEx(aResult), *pStream);

        pStream.reset(new SvFileStream("~/BlurBigPerformance.txt", StreamMode::WRITE));
        pStream->WriteOString("Blur average time: ");
        pStream->WriteOString(OString::number(
            std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count()));
        pStream->WriteOString("\n");
    }
}

void BitmapFilterTest::testGenerateStripRanges()
{
    {
        constexpr tools::Long nFirstIndex = 0;
        constexpr tools::Long nLastIndex = 100;
        constexpr tools::Long nStripSize = 32;

        std::vector<std::tuple<tools::Long, tools::Long, bool>> aRanges;

        vcl::bitmap::generateStripRanges<nStripSize>(
            nFirstIndex, nLastIndex,
            [&](tools::Long const nStart, tools::Long const nEnd, bool const bLast) {
                aRanges.emplace_back(nStart, nEnd, bLast);
            });

        CPPUNIT_ASSERT_EQUAL(size_t(4), aRanges.size());

        CPPUNIT_ASSERT_EQUAL(tools::Long(0), std::get<0>(aRanges[0]));
        CPPUNIT_ASSERT_EQUAL(tools::Long(31), std::get<1>(aRanges[0]));
        CPPUNIT_ASSERT_EQUAL(false, std::get<2>(aRanges[0]));

        CPPUNIT_ASSERT_EQUAL(tools::Long(32), std::get<0>(aRanges[1]));
        CPPUNIT_ASSERT_EQUAL(tools::Long(63), std::get<1>(aRanges[1]));
        CPPUNIT_ASSERT_EQUAL(false, std::get<2>(aRanges[1]));

        CPPUNIT_ASSERT_EQUAL(tools::Long(64), std::get<0>(aRanges[2]));
        CPPUNIT_ASSERT_EQUAL(tools::Long(95), std::get<1>(aRanges[2]));
        CPPUNIT_ASSERT_EQUAL(false, std::get<2>(aRanges[2]));

        CPPUNIT_ASSERT_EQUAL(tools::Long(96), std::get<0>(aRanges[3]));
        CPPUNIT_ASSERT_EQUAL(tools::Long(100), std::get<1>(aRanges[3]));
        CPPUNIT_ASSERT_EQUAL(true, std::get<2>(aRanges[3]));
    }

    {
        constexpr tools::Long nFirstIndex = 0;
        constexpr tools::Long nLastIndex = 95;
        constexpr tools::Long nStripSize = 32;

        std::vector<std::tuple<tools::Long, tools::Long, bool>> aRanges;

        vcl::bitmap::generateStripRanges<nStripSize>(
            nFirstIndex, nLastIndex,
            [&](tools::Long const nStart, tools::Long const nEnd, bool const bLast) {
                aRanges.emplace_back(nStart, nEnd, bLast);
            });

        CPPUNIT_ASSERT_EQUAL(size_t(3), aRanges.size());

        CPPUNIT_ASSERT_EQUAL(tools::Long(0), std::get<0>(aRanges[0]));
        CPPUNIT_ASSERT_EQUAL(tools::Long(31), std::get<1>(aRanges[0]));
        CPPUNIT_ASSERT_EQUAL(false, std::get<2>(aRanges[0]));

        CPPUNIT_ASSERT_EQUAL(tools::Long(32), std::get<0>(aRanges[1]));
        CPPUNIT_ASSERT_EQUAL(tools::Long(63), std::get<1>(aRanges[1]));
        CPPUNIT_ASSERT_EQUAL(false, std::get<2>(aRanges[1]));

        CPPUNIT_ASSERT_EQUAL(tools::Long(64), std::get<0>(aRanges[2]));
        CPPUNIT_ASSERT_EQUAL(tools::Long(95), std::get<1>(aRanges[2]));
        CPPUNIT_ASSERT_EQUAL(true, std::get<2>(aRanges[2]));
    }
}

} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(BitmapFilterTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
