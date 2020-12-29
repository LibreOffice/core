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
#include <bitmap/BitmapWriteAccess.hxx>
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

    OUString getUrl()
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
    void testRoundtrip();

    CPPUNIT_TEST_SUITE(TiffFilterTest);
    CPPUNIT_TEST(testCVEs);
    CPPUNIT_TEST(testTdf126460);
    CPPUNIT_TEST(testTdf115863);
    CPPUNIT_TEST(testTdf138818);
    CPPUNIT_TEST(testRoundtrip);
    CPPUNIT_TEST_SUITE_END();
};

bool TiffFilterTest::load(const OUString&, const OUString& rURL, const OUString&, SfxFilterFlags,
                          SotClipboardFormatId, unsigned int)
{
    SvFileStream aFileStream(rURL, StreamMode::READ);
    Graphic aGraphic;
    return ImportTiffGraphicImport(aFileStream, aGraphic);
}

void TiffFilterTest::testCVEs() { testDir(OUString(), getUrl()); }

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

void TiffFilterTest::testRoundtrip()
{
    Bitmap aBitmap(Size(2, 2), 24);
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
    rFilter.ExportGraphic(Graphic(BitmapEx(aBitmap)), "none", aStream, nFilterFormat);
    aStream.Seek(STREAM_SEEK_TO_BEGIN);

    Graphic aGraphic;
    ErrCode bResult = rFilter.ImportGraphic(aGraphic, "none", aStream);
    CPPUNIT_ASSERT_EQUAL(ERRCODE_NONE, bResult);
    CPPUNIT_ASSERT_EQUAL(GfxLinkType::NativeTif, aGraphic.GetGfxLink().GetType());
    Bitmap aResultBitmap = aGraphic.GetBitmapEx().GetBitmap();
    CPPUNIT_ASSERT_EQUAL(Size(2, 2), aResultBitmap.GetSizePixel());

    {
        Bitmap::ScopedReadAccess pAccess(aResultBitmap);
        CPPUNIT_ASSERT_EQUAL(COL_WHITE, Color(pAccess->GetPixel(0, 0)));
        CPPUNIT_ASSERT_EQUAL(COL_BLACK, Color(pAccess->GetPixel(0, 1)));
        CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, Color(pAccess->GetPixel(1, 0)));
        CPPUNIT_ASSERT_EQUAL(COL_LIGHTGREEN, Color(pAccess->GetPixel(1, 1)));
    }

    aStream.Seek(STREAM_SEEK_TO_BEGIN);
    vcl::GraphicFormatDetector aDetector(aStream, "");

    CPPUNIT_ASSERT_EQUAL(true, aDetector.detect());
    CPPUNIT_ASSERT_EQUAL(true, aDetector.checkTIF());
    CPPUNIT_ASSERT_EQUAL(OUString(u"TIF"), aDetector.msDetectedFormat);
}

CPPUNIT_TEST_SUITE_REGISTRATION(TiffFilterTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
