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
#include <vcl/alpha.hxx>
#include <vcl/BitmapWriteAccess.hxx>
#include <tools/stream.hxx>
#include <vcl/gfxlink.hxx>
#include <vcl/graph.hxx>
#include <vcl/graphicfilter.hxx>
#include <graphic/GraphicFormatDetector.hxx>
#include <filter/JxlReader.hxx>
#include <comphelper/propertyvalue.hxx>

using namespace css;

/* Implementation of Filters test */

class JxlFilterTest : public test::FiltersTest, public test::BootstrapFixture
{
public:
    JxlFilterTest()
        : BootstrapFixture(true, false)
    {
    }

    virtual bool load(const OUString&, const OUString& rURL, const OUString&, SfxFilterFlags,
                      SotClipboardFormatId, unsigned int) override;

    /**
     * Ensure CVEs remain unbroken
     */
    void testCVEs();

    void testReadAlphaLossless();
    void testReadAlphaLossy();
    void testReadNoAlphaLossless();
    void testReadNoAlphaLossy();

    CPPUNIT_TEST_SUITE(JxlFilterTest);
    CPPUNIT_TEST(testCVEs);
    CPPUNIT_TEST(testReadAlphaLossless);
    CPPUNIT_TEST(testReadAlphaLossy);
    CPPUNIT_TEST(testReadNoAlphaLossless);
    CPPUNIT_TEST(testReadNoAlphaLossy);
    CPPUNIT_TEST_SUITE_END();

private:
    void testRead(std::u16string_view rName, bool lossy, bool alpha);
};

bool JxlFilterTest::load(const OUString&, const OUString& rURL, const OUString&, SfxFilterFlags,
                         SotClipboardFormatId, unsigned int)
{
    SvFileStream aFileStream(rURL, StreamMode::READ);
    Graphic aGraphic;
    return ImportJxlGraphic(aFileStream, aGraphic);
}

void JxlFilterTest::testCVEs()
{
#ifndef DISABLE_CVE_TESTS
    testDir(OUString(), m_directories.getURLFromSrc(u"/vcl/qa/cppunit/graphicfilter/data/jxl/"));
#endif
}

void JxlFilterTest::testReadAlphaLossless() { testRead(u"alpha_lossless.jxl", false, true); }

void JxlFilterTest::testReadAlphaLossy() { testRead(u"alpha_lossy.jxl", true, true); }

void JxlFilterTest::testReadNoAlphaLossless() { testRead(u"noalpha_lossless.jxl", false, false); }

void JxlFilterTest::testReadNoAlphaLossy() { testRead(u"noalpha_lossy.jxl", true, false); }

void JxlFilterTest::testRead(std::u16string_view rName, bool lossy, bool alpha)
{
    // Read a file created in GIMP and check it's read correctly.
    OUString file = m_directories.getURLFromSrc(u"/vcl/qa/cppunit/graphicfilter/data/jxl/") + rName;
    SvFileStream aFileStream(file, StreamMode::READ);
    Graphic aGraphic;
    GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
    ErrCode bResult = rFilter.ImportGraphic(aGraphic, u"none", aFileStream);
    CPPUNIT_ASSERT_EQUAL(ERRCODE_NONE, bResult);
    CPPUNIT_ASSERT_EQUAL(GfxLinkType::NativeJxl, aGraphic.GetGfxLink().GetType());
    Bitmap aResultBitmap = aGraphic.GetBitmap();
    CPPUNIT_ASSERT_EQUAL(Size(10, 10), aResultBitmap.GetSizePixel());

    {
        Bitmap tmpBitmap = aResultBitmap.CreateColorBitmap();
        BitmapScopedReadAccess pAccess(tmpBitmap);
        // Note that x,y are swapped.
        if (lossy)
            CPPUNIT_ASSERT_LESS(sal_uInt16(2), pAccess->GetPixel(0, 0).GetColorError(COL_LIGHTRED));
        else
            CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, Color(pAccess->GetPixel(0, 0)));
        CPPUNIT_ASSERT_EQUAL(COL_LIGHTBLUE, Color(pAccess->GetPixel(9, 9)));

        // if bitmaps of non-alpha examples have an alpha channel,
        // the values should all be max (255)
        if (aResultBitmap.HasAlpha())
        {
            AlphaMask tmpAlpha = aResultBitmap.CreateAlphaMask();
            BitmapScopedReadAccess pAccessAlpha(tmpAlpha);
            CPPUNIT_ASSERT_EQUAL(sal_uInt8(255), pAccessAlpha->GetPixelIndex(0, 0));
            if (alpha)
                CPPUNIT_ASSERT_EQUAL(sal_uInt8(0), pAccessAlpha->GetPixelIndex(0, 9));
            else
                CPPUNIT_ASSERT_EQUAL(sal_uInt8(255), pAccessAlpha->GetPixelIndex(0, 9));
        }
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(JxlFilterTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
