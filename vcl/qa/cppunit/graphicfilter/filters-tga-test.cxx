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
#include <tools/stream.hxx>
#include <vcl/BitmapWriteAccess.hxx>
#include <vcl/filter/ImportOutput.hxx>
#include <vcl/graphicfilter.hxx>
#include <filter/TgaReader.hxx>

using namespace ::com::sun::star;

/* Implementation of Filters test */

class TgaFilterTest
    : public test::FiltersTest
    , public test::BootstrapFixture
{
public:
    TgaFilterTest() : BootstrapFixture(true, false) {}

    virtual bool load(const OUString &,
        const OUString &rURL, const OUString &,
        SfxFilterFlags, SotClipboardFormatId, unsigned int) override;

    /**
     * Ensure CVEs remain unbroken
     */
    void testCVEs();
    void testTdf174105();

    CPPUNIT_TEST_SUITE(TgaFilterTest);
    CPPUNIT_TEST(testCVEs);
    CPPUNIT_TEST(testTdf174105);
    CPPUNIT_TEST_SUITE_END();
};

bool TgaFilterTest::load(const OUString &,
    const OUString &rURL, const OUString &,
    SfxFilterFlags, SotClipboardFormatId, unsigned int)
{
    SvFileStream aFileStream(rURL, StreamMode::READ);
    ImportOutput aImportOutput;
    return ImportTgaGraphic(aFileStream, aImportOutput);
}

void TgaFilterTest::testCVEs()
{
#ifndef DISABLE_CVE_TESTS
    testDir(OUString(), m_directories.getURLFromSrc(u"/vcl/qa/cppunit/graphicfilter/data/tga/"));
#endif
}

CPPUNIT_TEST_SUITE_REGISTRATION(TgaFilterTest);

void TgaFilterTest::testTdf174105()
{
    OUString aURL
        = m_directories.getURLFromSrc(u"/vcl/qa/cppunit/graphicfilter/data/tga/tdf174105.tga");
    SvFileStream aFileStream(aURL, StreamMode::READ);
    Graphic aGraphic;
    GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();

    ErrCode bResult = rFilter.ImportGraphic(aGraphic, aURL, aFileStream);

    CPPUNIT_ASSERT_EQUAL(ERRCODE_NONE, bResult);

    Bitmap aBitmap = aGraphic.GetBitmap().CreateColorBitmap();
    BitmapScopedReadAccess pReadAccess(aBitmap);

    // Check the image contains white in position 0,0
    const Color aColor = pReadAccess->GetColor(0, 0);

    // Without the fix in place, this test would have failed with
    // - Expected: rgba[fefefeff]
    // - Actual  : rgba[0000ffff]
    CPPUNIT_ASSERT_EQUAL(Color(254, 254, 254), aColor);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
