/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <unotest/filters-test.hxx>
#include <test/bootstrapfixture.hxx>
#include <test/xmltesttools.hxx>
#include <tools/stream.hxx>
#include <vcl/filter/ImportOutput.hxx>
#include <vcl/gdimtf.hxx>
#include <filter/DxfReader.hxx>

using namespace css;

/* Implementation of Filters test */

class DxfFilterTest
    : public test::FiltersTest
    , public test::BootstrapFixture
    , public XmlTestTools
{
public:
    DxfFilterTest() : BootstrapFixture(true, false) {}

    virtual bool load(const OUString &,
        const OUString &rURL, const OUString &,
        SfxFilterFlags, SotClipboardFormatId, unsigned int) override;

    /**
     * Ensure CVEs remain unbroken
     */
    void testCVEs();

    void testExtents();

    CPPUNIT_TEST_SUITE(DxfFilterTest);
    CPPUNIT_TEST(testCVEs);
    CPPUNIT_TEST(testExtents);
    CPPUNIT_TEST_SUITE_END();

private:
    GDIMetaFile importFile(std::u16string_view rName);
};

bool DxfFilterTest::load(const OUString &,
    const OUString &rURL, const OUString &,
    SfxFilterFlags, SotClipboardFormatId, unsigned int)
{
    SvFileStream aFileStream(rURL, StreamMode::READ);
    ImportOutput aImportOutput;
    return ImportDxfGraphic(aFileStream, aImportOutput);
}

GDIMetaFile DxfFilterTest::importFile(std::u16string_view rName)
{
    SvFileStream aFileStream(
        m_directories.getURLFromSrc(u"/vcl/qa/cppunit/graphicfilter/data/dxf/import/") + rName,
        StreamMode::READ);
    ImportOutput aImportOutput;
    CPPUNIT_ASSERT(ImportDxfGraphic(aFileStream, aImportOutput));
    CPPUNIT_ASSERT(aImportOutput.moGDIMetaFile);
    return *aImportOutput.moGDIMetaFile;
}

void DxfFilterTest::testCVEs()
{
#ifndef DISABLE_CVE_TESTS
    testDir(OUString(),
        m_directories.getURLFromSrc(u"/vcl/qa/cppunit/graphicfilter/data/dxf/"));
#endif
}

void DxfFilterTest::testExtents()
{
    // The header has the placeholder extents of an empty drawing, the real extents come from
    // a 100 x 50 rectangle and a block line rotated upwards from its top right corner.
    GDIMetaFile aMTF = importFile(u"extents.dxf");
    CPPUNIT_ASSERT_EQUAL(Size(10100, 6100), aMTF.GetPrefSize());

    MetafileXmlDump aDumper;
    xmlDocUniquePtr pDoc = dumpAndParse(aDumper, aMTF);
    assertXPath(pDoc, "/metafile/polyline", 2);
    // the rectangle, closed
    assertXPath(pDoc, "/metafile/polyline[1]/point", 5);
    assertXPath(pDoc, "/metafile/polyline[1]/point[1]", "x", u"50");
    assertXPath(pDoc, "/metafile/polyline[1]/point[1]", "y", u"6050");
    assertXPath(pDoc, "/metafile/polyline[1]/point[3]", "x", u"10050");
    assertXPath(pDoc, "/metafile/polyline[1]/point[3]", "y", u"1050");
    // the rotated line of the block
    assertXPath(pDoc, "/metafile/polyline[2]/point[2]", "x", u"10050");
    assertXPath(pDoc, "/metafile/polyline[2]/point[2]", "y", u"50");
}

CPPUNIT_TEST_SUITE_REGISTRATION(DxfFilterTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
