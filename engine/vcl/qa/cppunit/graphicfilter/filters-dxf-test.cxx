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
#include <vcl/metaactiontypes.hxx>
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
    void testColors();
    void testLineTypes();

    CPPUNIT_TEST_SUITE(DxfFilterTest);
    CPPUNIT_TEST(testCVEs);
    CPPUNIT_TEST(testExtents);
    CPPUNIT_TEST(testColors);
    CPPUNIT_TEST(testLineTypes);
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

void DxfFilterTest::testColors()
{
    GDIMetaFile aMTF = importFile(u"colors.dxf");
    MetafileXmlDump aDumper;
    aDumper.filterAllActionTypes();
    aDumper.filterActionType(MetaActionType::POLYLINE, false);
    aDumper.filterActionType(MetaActionType::LINECOLOR, false);
    xmlDocUniquePtr pDoc = dumpAndParse(aDumper, aMTF);

    // the lines on the frozen and on the switched off layer, and the invisible one are not drawn
    assertXPath(pDoc, "/metafile/polyline", 5);
    assertXPath(pDoc, "/metafile/linecolor", 5);
    // the colour of the layer
    assertXPath(pDoc, "/metafile/linecolor[1]", "color", u"#ff0000");
    // the true colour of the entity wins over its colour index
    assertXPath(pDoc, "/metafile/linecolor[2]", "color", u"#00ff80");
    // the true colour of the layer
    assertXPath(pDoc, "/metafile/linecolor[3]", "color", u"#123456");
    // in the block, layer 0 is the layer of the insert, BYBLOCK is the colour of the insert
    assertXPath(pDoc, "/metafile/linecolor[4]", "color", u"#ff0000");
    assertXPath(pDoc, "/metafile/linecolor[5]", "color", u"#00ffff");
}

void DxfFilterTest::testLineTypes()
{
    // the drawing is 100 units wide, one unit is 100 in the metafile
    GDIMetaFile aMTF = importFile(u"linetypes.dxf");
    MetafileXmlDump aDumper;
    aDumper.filterAllActionTypes();
    aDumper.filterActionType(MetaActionType::POLYLINE, false);
    xmlDocUniquePtr pDoc = dumpAndParse(aDumper, aMTF);

    assertXPath(pDoc, "/metafile/polyline", 4);
    // the pattern is scaled by $LTSCALE and by the line type scale of the entity
    assertXPath(pDoc, "/metafile/polyline[1]", "style", u"dash");
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        1000.0, getXPath(pDoc, "/metafile/polyline[1]", "dotlen").toDouble(), 1.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        500.0, getXPath(pDoc, "/metafile/polyline[1]", "distance").toDouble(), 1.0);
    // a pattern too dense to see
    assertXPath(pDoc, "/metafile/polyline[2]", "style", u"solid");
    // the line weight of the layer, in 1/100 mm
    assertXPath(pDoc, "/metafile/polyline[3]", "width", u"50");
    // thin line weights are hairlines
    assertXPath(pDoc, "/metafile/polyline[4]", "width", u"0");
}

CPPUNIT_TEST_SUITE_REGISTRATION(DxfFilterTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
