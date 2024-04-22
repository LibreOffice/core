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
#include <vcl/BitmapWriteAccess.hxx>
#include <tools/stream.hxx>
#include <vcl/graph.hxx>
#include <vcl/graphicfilter.hxx>
#include <graphic/GraphicFormatDetector.hxx>
#include <filter/WebpReader.hxx>
#include <comphelper/propertyvalue.hxx>

using namespace css;

/* Implementation of Filters test */

class WebpFilterTest : public test::FiltersTest, public test::BootstrapFixture
{
public:
    WebpFilterTest()
        : BootstrapFixture(true, false)
    {
    }

    virtual bool load(const OUString&, const OUString& rURL, const OUString&, SfxFilterFlags,
                      SotClipboardFormatId, unsigned int) override;

    /**
     * Ensure CVEs remain unbroken
     */
    void testCVEs();

    void testRoundtripLossless();
    void testRoundtripLossy();
    void testReadAlphaLossless();
    void testReadAlphaLossy();
    void testReadNoAlphaLossless();
    void testReadNoAlphaLossy();

    CPPUNIT_TEST_SUITE(WebpFilterTest);
    CPPUNIT_TEST(testCVEs);
    CPPUNIT_TEST(testRoundtripLossless);
    CPPUNIT_TEST(testRoundtripLossy);
    CPPUNIT_TEST(testReadAlphaLossless);
    CPPUNIT_TEST(testReadAlphaLossy);
    CPPUNIT_TEST(testReadNoAlphaLossless);
    CPPUNIT_TEST(testReadNoAlphaLossy);
    CPPUNIT_TEST_SUITE_END();

private:
    void testRoundtrip(bool lossy);
    void testRead(std::u16string_view rName, bool lossy, bool alpha);
};

bool WebpFilterTest::load(const OUString&, const OUString& rURL, const OUString&, SfxFilterFlags,
                          SotClipboardFormatId, unsigned int)
{
    SvFileStream aFileStream(rURL, StreamMode::READ);
    Graphic aGraphic;
    return ImportWebpGraphic(aFileStream, aGraphic);
}

void WebpFilterTest::testCVEs()
{
#ifndef DISABLE_CVE_TESTS
    testDir(OUString(), m_directories.getURLFromSrc(u"/vcl/qa/cppunit/graphicfilter/data/webp/"));
#endif
}

void WebpFilterTest::testRoundtripLossless() { testRoundtrip(false); }

void WebpFilterTest::testRoundtripLossy() { testRoundtrip(true); }

void WebpFilterTest::testRoundtrip(bool lossy)
{
    // Do not use just 2x2, lossy saving would change colors.
    Bitmap aBitmap(Size(20, 20), vcl::PixelFormat::N24_BPP);
    AlphaMask aAlpha(Size(20, 20));
    {
        BitmapScopedWriteAccess pAccess(aBitmap);
        pAccess->SetFillColor(COL_WHITE);
        pAccess->FillRect(tools::Rectangle(Point(0, 0), Size(10, 10)));
        pAccess->SetFillColor(COL_BLACK);
        pAccess->FillRect(tools::Rectangle(Point(10, 0), Size(10, 10)));
        pAccess->SetFillColor(COL_LIGHTRED);
        pAccess->FillRect(tools::Rectangle(Point(0, 10), Size(10, 10)));
        pAccess->SetFillColor(COL_BLUE);
        pAccess->FillRect(tools::Rectangle(Point(10, 10), Size(10, 10)));
        BitmapScopedWriteAccess pAccessAlpha(aAlpha);
        pAccessAlpha->SetFillColor(BitmapColor(0)); // opaque
        pAccessAlpha->FillRect(tools::Rectangle(Point(0, 0), Size(10, 10)));
        pAccessAlpha->FillRect(tools::Rectangle(Point(10, 0), Size(10, 10)));
        pAccessAlpha->FillRect(tools::Rectangle(Point(0, 10), Size(10, 10)));
        pAccessAlpha->SetFillColor(BitmapColor(64, 64, 64));
        pAccessAlpha->FillRect(tools::Rectangle(Point(10, 10), Size(10, 10)));
    }
    BitmapEx aBitmapEx(aBitmap, aAlpha);

    SvMemoryStream aStream;
    GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
    sal_uInt16 nFilterFormat = rFilter.GetExportFormatNumberForShortName(u"webp");
    css::uno::Sequence<css::beans::PropertyValue> aFilterData{
        comphelper::makePropertyValue("Lossless", !lossy),
        comphelper::makePropertyValue("Quality", sal_Int32(100))
    };
    rFilter.ExportGraphic(Graphic(aBitmapEx), u"none", aStream, nFilterFormat, &aFilterData);
    aStream.Seek(STREAM_SEEK_TO_BEGIN);

    Graphic aGraphic;
    ErrCode bResult = rFilter.ImportGraphic(aGraphic, u"none", aStream);
    CPPUNIT_ASSERT_EQUAL(ERRCODE_NONE, bResult);
    CPPUNIT_ASSERT_EQUAL(GfxLinkType::NativeWebp, aGraphic.GetGfxLink().GetType());
    BitmapEx aResultBitmap = aGraphic.GetBitmapEx();
    CPPUNIT_ASSERT_EQUAL(Size(20, 20), aResultBitmap.GetSizePixel());
    CPPUNIT_ASSERT(aResultBitmap.IsAlpha());

    {
        Bitmap tmpBitmap = aResultBitmap.GetBitmap();
        BitmapScopedReadAccess pAccess(tmpBitmap);
        // Note that x,y are swapped.
        CPPUNIT_ASSERT_EQUAL(COL_WHITE, Color(pAccess->GetPixel(0, 0)));
        CPPUNIT_ASSERT_EQUAL(COL_BLACK, Color(pAccess->GetPixel(0, 19)));
        if (lossy)
        {
            CPPUNIT_ASSERT_LESS(sal_uInt16(3),
                                pAccess->GetPixel(19, 0).GetColorError(COL_LIGHTRED));
            CPPUNIT_ASSERT_LESS(sal_uInt16(3), pAccess->GetPixel(19, 19).GetColorError(COL_BLUE));
        }
        else
        {
            CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, Color(pAccess->GetPixel(19, 0)));
            CPPUNIT_ASSERT_EQUAL(COL_BLUE, Color(pAccess->GetPixel(19, 19)));
        }
        AlphaMask tmpAlpha = aResultBitmap.GetAlphaMask();
        BitmapScopedReadAccess pAccessAlpha(tmpAlpha);
        CPPUNIT_ASSERT_EQUAL(sal_uInt8(255), pAccessAlpha->GetPixelIndex(0, 0));
        CPPUNIT_ASSERT_EQUAL(sal_uInt8(255), pAccessAlpha->GetPixelIndex(0, 19));
        CPPUNIT_ASSERT_EQUAL(sal_uInt8(255), pAccessAlpha->GetPixelIndex(19, 0));
        CPPUNIT_ASSERT_EQUAL(sal_uInt8(191), pAccessAlpha->GetPixelIndex(19, 19));
    }

    aStream.Seek(STREAM_SEEK_TO_BEGIN);
    vcl::GraphicFormatDetector aDetector(aStream, "");

    CPPUNIT_ASSERT_EQUAL(true, aDetector.detect());
    CPPUNIT_ASSERT_EQUAL(true, aDetector.checkWEBP());
    CPPUNIT_ASSERT_EQUAL(u"WEBP"_ustr,
                         vcl::getImportFormatShortName(aDetector.getMetadata().mnFormat));
}

void WebpFilterTest::testReadAlphaLossless() { testRead(u"alpha_lossless.webp", false, true); }

void WebpFilterTest::testReadAlphaLossy() { testRead(u"alpha_lossy.webp", true, true); }

void WebpFilterTest::testReadNoAlphaLossless() { testRead(u"noalpha_lossless.webp", false, false); }

void WebpFilterTest::testReadNoAlphaLossy() { testRead(u"noalpha_lossy.webp", true, false); }

void WebpFilterTest::testRead(std::u16string_view rName, bool lossy, bool alpha)
{
    // Read a file created in GIMP and check it's read correctly.
    OUString file
        = m_directories.getURLFromSrc(u"/vcl/qa/cppunit/graphicfilter/data/webp/") + rName;
    SvFileStream aFileStream(file, StreamMode::READ);
    Graphic aGraphic;
    GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
    ErrCode bResult = rFilter.ImportGraphic(aGraphic, u"none", aFileStream);
    CPPUNIT_ASSERT_EQUAL(ERRCODE_NONE, bResult);
    CPPUNIT_ASSERT_EQUAL(GfxLinkType::NativeWebp, aGraphic.GetGfxLink().GetType());
    BitmapEx aResultBitmap = aGraphic.GetBitmapEx();
    CPPUNIT_ASSERT_EQUAL(Size(10, 10), aResultBitmap.GetSizePixel());
    CPPUNIT_ASSERT_EQUAL(alpha, aResultBitmap.IsAlpha());

    {
        Bitmap tmpBitmap = aResultBitmap.GetBitmap();
        BitmapScopedReadAccess pAccess(tmpBitmap);
        // Note that x,y are swapped.
        if (lossy)
            CPPUNIT_ASSERT_LESS(sal_uInt16(2), pAccess->GetPixel(0, 0).GetColorError(COL_LIGHTRED));
        else
            CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, Color(pAccess->GetPixel(0, 0)));
        CPPUNIT_ASSERT_EQUAL(COL_LIGHTBLUE, Color(pAccess->GetPixel(9, 9)));
        if (alpha)
        {
            AlphaMask tmpAlpha = aResultBitmap.GetAlphaMask();
            BitmapScopedReadAccess pAccessAlpha(tmpAlpha);
            CPPUNIT_ASSERT_EQUAL(sal_uInt8(255), pAccessAlpha->GetPixelIndex(0, 0));
            CPPUNIT_ASSERT_EQUAL(sal_uInt8(0), pAccessAlpha->GetPixelIndex(0, 9));
        }
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(WebpFilterTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
