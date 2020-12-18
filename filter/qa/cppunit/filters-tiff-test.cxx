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

extern "C"
{
    SAL_DLLPUBLIC_EXPORT bool SAL_CALL
        itiGraphicImport(SvStream & rStream, Graphic & rGraphic,
        FilterConfigItem*);
}

using namespace ::com::sun::star;

/* Implementation of Filters test */

class TiffFilterTest
    : public test::FiltersTest
    , public test::BootstrapFixture
{
public:
    TiffFilterTest() : BootstrapFixture(true, false) {}

    virtual bool load(const OUString &,
        const OUString &rURL, const OUString &,
        SfxFilterFlags, SotClipboardFormatId, unsigned int) override;

    OUString getUrl()
    {
        return m_directories.getURLFromSrc("/filter/qa/cppunit/data/tiff/");
    }

    /**
     * Ensure CVEs remain unbroken
     */
    void testCVEs();
    void testTdf126460();
    void testTdf115863();
    void testTdf138818();

    CPPUNIT_TEST_SUITE(TiffFilterTest);
    CPPUNIT_TEST(testCVEs);
    CPPUNIT_TEST(testTdf126460);
    CPPUNIT_TEST(testTdf115863);
    CPPUNIT_TEST(testTdf138818);
    CPPUNIT_TEST_SUITE_END();
};

bool TiffFilterTest::load(const OUString &,
    const OUString &rURL, const OUString &,
    SfxFilterFlags, SotClipboardFormatId, unsigned int)
{
    SvFileStream aFileStream(rURL, StreamMode::READ);
    Graphic aGraphic;
    return itiGraphicImport(aFileStream, aGraphic, nullptr);
}

void TiffFilterTest::testCVEs()
{
    testDir(OUString(),
        getUrl());
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

CPPUNIT_TEST_SUITE_REGISTRATION(TiffFilterTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
