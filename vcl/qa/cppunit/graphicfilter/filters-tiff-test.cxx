/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <unotest/filters-test.hxx>
#include <test/bootstrapfixture.hxx>
#include <vcl/FilterConfigItem.hxx>
#include <tools/stream.hxx>
#include <vcl/graph.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/BitmapWriteAccess.hxx>
#include <graphic/GraphicFormatDetector.hxx>

#include <filter/TiffReader.hxx>

using namespace ::com::sun::star;

/* Implementation of Filters test */

class TiffFilterTest : public test::FiltersTest, public test::BootstrapFixture
{
public:
    TiffFilterTest()
        : BootstrapFixture(true, false)
    {
    }

    virtual bool load(const OUString&, const OUString& rURL, const OUString&, SfxFilterFlags,
                      SotClipboardFormatId, unsigned int) override;

    OUString getUrl() const
    {
        return m_directories.getURLFromSrc(u"/vcl/qa/cppunit/graphicfilter/data/tiff/");
    }

    /**
     * Ensure CVEs remain unbroken
     */
    void testCVEs();
    void testTdf126460();
    void testTdf115863();
    void testTdf138818();
    void testTdf149418();
    void testTdf74331();
    void testRoundtrip();
    void testRGB8bits();
    void testRGB16bits();

    CPPUNIT_TEST_SUITE(TiffFilterTest);
    CPPUNIT_TEST(testCVEs);
    CPPUNIT_TEST(testTdf126460);
    CPPUNIT_TEST(testTdf115863);
    CPPUNIT_TEST(testTdf138818);
    CPPUNIT_TEST(testTdf149418);
    CPPUNIT_TEST(testTdf74331);
    CPPUNIT_TEST(testRoundtrip);
    CPPUNIT_TEST(testRGB8bits);
    CPPUNIT_TEST(testRGB16bits);
    CPPUNIT_TEST_SUITE_END();
};

bool TiffFilterTest::load(const OUString&, const OUString& rURL, const OUString&, SfxFilterFlags,
                          SotClipboardFormatId, unsigned int)
{
    SvFileStream aFileStream(rURL, StreamMode::READ);
    Graphic aGraphic;
    return ImportTiffGraphicImport(aFileStream, aGraphic);
}

void TiffFilterTest::testCVEs()
{
#ifndef DISABLE_CVE_TESTS
    testDir(OUString(), getUrl());
#endif
}

void TiffFilterTest::testTdf126460()
{
    OUString aURL = getUrl() + "tdf126460.tif";
    SvFileStream aFileStream(aURL, StreamMode::READ);
    Graphic aGraphic;
    GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();

    ErrCode bResult = rFilter.ImportGraphic(aGraphic, aURL, aFileStream);

    CPPUNIT_ASSERT_EQUAL(ERRCODE_NONE, bResult);

    // Without the fix in place, the following asserts would have failed
    CPPUNIT_ASSERT(aGraphic.IsAlpha());
    CPPUNIT_ASSERT(aGraphic.IsTransparent());
}

void TiffFilterTest::testTdf115863()
{
    OUString aURL = getUrl() + "tdf115863.tif";
    SvFileStream aFileStream(aURL, StreamMode::READ);
    Graphic aGraphic;
    GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();

    ErrCode bResult = rFilter.ImportGraphic(aGraphic, aURL, aFileStream);

    // Without the fix in place, this test would have failed with
    // - Expected: 0x0(Error Area:Io Class:NONE Code:0)
    // - Actual  : 0x8203(Error Area:Vcl Class:General Code:3)
    CPPUNIT_ASSERT_EQUAL(ERRCODE_NONE, bResult);

    Bitmap aBitmap = aGraphic.GetBitmapEx().GetBitmap();
    Size aSize = aBitmap.GetSizePixel();
    CPPUNIT_ASSERT_EQUAL(tools::Long(528), aSize.Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(618), aSize.Height());
}

void TiffFilterTest::testTdf138818()
{
    OUString aURL = getUrl() + "tdf138818.tif";
    SvFileStream aFileStream(aURL, StreamMode::READ);
    Graphic aGraphic;
    GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();

    ErrCode bResult = rFilter.ImportGraphic(aGraphic, aURL, aFileStream);

    CPPUNIT_ASSERT_EQUAL(ERRCODE_NONE, bResult);

    // Without the fix in place, this test would have failed with
    // - Expected: 46428
    // - Actual  : 45951
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(46428), aGraphic.GetGfxLink().GetDataSize());
}

void TiffFilterTest::testTdf149418()
{
    OUString aURL = getUrl() + "tdf149418.tif";
    SvFileStream aFileStream(aURL, StreamMode::READ);
    Graphic aGraphic;
    GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();

    ErrCode bResult = rFilter.ImportGraphic(aGraphic, aURL, aFileStream);

    CPPUNIT_ASSERT_EQUAL(ERRCODE_NONE, bResult);

    // Without the fix in place, this test would have failed with
    // - Expected: 50938
    // - Actual  : 50029
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(50938), aGraphic.GetGfxLink().GetDataSize());
}

void TiffFilterTest::testTdf74331()
{
    OUString aURL = getUrl() + "tdf74331.tif";
    SvFileStream aFileStream(aURL, StreamMode::READ);
    Graphic aGraphic;
    GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();

    ErrCode bResult = rFilter.ImportGraphic(aGraphic, aURL, aFileStream);

    CPPUNIT_ASSERT_EQUAL(ERRCODE_NONE, bResult);

    Bitmap aBitmap = aGraphic.GetBitmapEx().GetBitmap();
    Size aSize = aBitmap.GetSizePixel();
    CPPUNIT_ASSERT_EQUAL(tools::Long(200), aSize.Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(200), aSize.Height());

    BitmapScopedReadAccess pReadAccess(aBitmap);

    // Check the image contains different kinds of grays
    int nGrayCount = 0;
    int nGray3Count = 0;
    int nGray7Count = 0;
    int nLightGrayCount = 0;

    for (tools::Long nX = 1; nX < aSize.Width() - 1; ++nX)
    {
        for (tools::Long nY = 1; nY < aSize.Height() - 1; ++nY)
        {
            const Color aColor = pReadAccess->GetColor(nY, nX);
            if (aColor == COL_GRAY)
                ++nGrayCount;
            else if (aColor == COL_GRAY3)
                ++nGray3Count;
            else if (aColor == COL_GRAY7)
                ++nGray7Count;
            else if (aColor == COL_LIGHTGRAY)
                ++nLightGrayCount;
        }
    }

    // Without the fix in place, this test would have failed with
    // - Expected: 313
    // - Actual  : 0
    CPPUNIT_ASSERT_EQUAL(313, nGrayCount);
    CPPUNIT_ASSERT_EQUAL(71, nGray3Count);
    CPPUNIT_ASSERT_EQUAL(227, nGray7Count);
    CPPUNIT_ASSERT_EQUAL(165, nLightGrayCount);
}

void TiffFilterTest::testRoundtrip()
{
    Bitmap aBitmap(Size(2, 2), vcl::PixelFormat::N24_BPP);
    {
        BitmapScopedWriteAccess pAccess(aBitmap);
        pAccess->SetPixel(0, 0, COL_WHITE);
        pAccess->SetPixel(0, 1, COL_BLACK);
        pAccess->SetPixel(1, 0, COL_LIGHTRED);
        pAccess->SetPixel(1, 1, COL_LIGHTGREEN);
    }

    SvMemoryStream aStream;
    GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
    sal_uInt16 nFilterFormat = rFilter.GetExportFormatNumberForShortName(u"tif");
    rFilter.ExportGraphic(Graphic(BitmapEx(aBitmap)), u"none", aStream, nFilterFormat);
    aStream.Seek(STREAM_SEEK_TO_BEGIN);

    Graphic aGraphic;
    ErrCode bResult = rFilter.ImportGraphic(aGraphic, u"none", aStream);
    CPPUNIT_ASSERT_EQUAL(ERRCODE_NONE, bResult);
    CPPUNIT_ASSERT_EQUAL(GfxLinkType::NativeTif, aGraphic.GetGfxLink().GetType());
    Bitmap aResultBitmap = aGraphic.GetBitmapEx().GetBitmap();
    CPPUNIT_ASSERT_EQUAL(Size(2, 2), aResultBitmap.GetSizePixel());

    {
        BitmapScopedReadAccess pAccess(aResultBitmap);
        CPPUNIT_ASSERT_EQUAL(COL_WHITE, Color(pAccess->GetPixel(0, 0)));
        CPPUNIT_ASSERT_EQUAL(COL_BLACK, Color(pAccess->GetPixel(0, 1)));
        CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, Color(pAccess->GetPixel(1, 0)));
        CPPUNIT_ASSERT_EQUAL(COL_LIGHTGREEN, Color(pAccess->GetPixel(1, 1)));
    }

    aStream.Seek(STREAM_SEEK_TO_BEGIN);
    vcl::GraphicFormatDetector aDetector(aStream, u""_ustr);

    CPPUNIT_ASSERT_EQUAL(true, aDetector.detect());
    CPPUNIT_ASSERT_EQUAL(true, aDetector.checkTIF());
    CPPUNIT_ASSERT_EQUAL(u"TIF"_ustr,
                         vcl::getImportFormatShortName(aDetector.getMetadata().mnFormat));
}

void TiffFilterTest::testRGB8bits()
{
    const std::initializer_list<std::u16string_view> aNames = {
        u"red8.tif",
        u"green8.tif",
        u"blue8.tif",
    };

    for (const auto& rName : aNames)
    {
        OUString aURL = getUrl() + rName;
        SvFileStream aFileStream(aURL, StreamMode::READ);
        Graphic aGraphic;
        GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();

        ErrCode bResult = rFilter.ImportGraphic(aGraphic, aURL, aFileStream);
        CPPUNIT_ASSERT_EQUAL(ERRCODE_NONE, bResult);

        Bitmap aBitmap = aGraphic.GetBitmapEx().GetBitmap();
        Size aSize = aBitmap.GetSizePixel();
        CPPUNIT_ASSERT_EQUAL(tools::Long(10), aSize.Width());
        CPPUNIT_ASSERT_EQUAL(tools::Long(10), aSize.Height());

        BitmapScopedReadAccess pReadAccess(aBitmap);
        const Color aColor = pReadAccess->GetColor(5, 5);

        if (rName == u"red8.tif")
            CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, aColor);
        else if (rName == u"green8.tif")
            CPPUNIT_ASSERT_EQUAL(COL_LIGHTGREEN, aColor);
        else
            CPPUNIT_ASSERT_EQUAL(COL_LIGHTBLUE, aColor);
    }
}

void TiffFilterTest::testRGB16bits()
{
    const std::initializer_list<std::u16string_view> aNames = {
        u"red16.tif",
        u"green16.tif",
        u"blue16.tif",
    };

    for (const auto& rName : aNames)
    {
        OUString aURL = getUrl() + rName;
        SvFileStream aFileStream(aURL, StreamMode::READ);
        Graphic aGraphic;
        GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();

        ErrCode bResult = rFilter.ImportGraphic(aGraphic, aURL, aFileStream);
        CPPUNIT_ASSERT_EQUAL(ERRCODE_NONE, bResult);

        Bitmap aBitmap = aGraphic.GetBitmapEx().GetBitmap();
        Size aSize = aBitmap.GetSizePixel();
        CPPUNIT_ASSERT_EQUAL(tools::Long(10), aSize.Width());
        CPPUNIT_ASSERT_EQUAL(tools::Long(10), aSize.Height());

        BitmapScopedReadAccess pReadAccess(aBitmap);
        const Color aColor = pReadAccess->GetColor(5, 5);

        if (rName == u"red16.tif")
            CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, aColor);
        else if (rName == u"green16.tif")
            // tdf#142151: Without the fix in place, this test would have failed with
            // - Expected: rgba[00ff00ff]
            // - Actual  : rgba[000000ff]
            CPPUNIT_ASSERT_EQUAL(COL_LIGHTGREEN, aColor);
        else
            CPPUNIT_ASSERT_EQUAL(COL_LIGHTBLUE, aColor);
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(TiffFilterTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
