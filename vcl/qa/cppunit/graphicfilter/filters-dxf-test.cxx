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
    void testPolylines();
    void testPolygonMesh();
    void testEllipse();
    void testWorldCoordinates();
    void testSpline();
    void testSplineKnots();

    CPPUNIT_TEST_SUITE(DxfFilterTest);
    CPPUNIT_TEST(testCVEs);
    CPPUNIT_TEST(testExtents);
    CPPUNIT_TEST(testColors);
    CPPUNIT_TEST(testLineTypes);
    CPPUNIT_TEST(testPolylines);
    CPPUNIT_TEST(testPolygonMesh);
    CPPUNIT_TEST(testEllipse);
    CPPUNIT_TEST(testWorldCoordinates);
    CPPUNIT_TEST(testSpline);
    CPPUNIT_TEST(testSplineKnots);
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

void DxfFilterTest::testPolylines()
{
    // The bulges reach from y = -10 to y = 10 and the widening polyline is 4 wide at y = 30:
    // the drawing is 50 x 42, one unit is 200 in the metafile.
    GDIMetaFile aMTF = importFile(u"polylines.dxf");
    CPPUNIT_ASSERT_EQUAL(Size(10100, 8500), aMTF.GetPrefSize());

    MetafileXmlDump aDumper;
    aDumper.filterAllActionTypes();
    aDumper.filterActionType(MetaActionType::POLYLINE, false);
    aDumper.filterActionType(MetaActionType::POLYGON, false);
    xmlDocUniquePtr pDoc = dumpAndParse(aDumper, aMTF);

    // the bulge is a curve from (0, 0) to (20, 0)
    assertXPath(pDoc, "/metafile/polyline[1]/point[1]", "x", u"50");
    assertXPath(pDoc, "/metafile/polyline[1]/point[1]", "y", u"6450");
    assertXPath(pDoc, "/metafile/polyline[1]/point[@flags='control']", 4);
    assertXPath(pDoc, "/metafile/polyline[1]/point[last()]", "x", u"4050");
    assertXPath(pDoc, "/metafile/polyline[1]/point[last()]", "y", u"6450");
    // the constant width is the line width
    assertXPath(pDoc, "/metafile/polyline[2]", "width", u"400");
    // the widening segment is filled
    assertXPath(pDoc, "/metafile/polygon", 1);
    // the five polylines, with the centre line of the widening one, and the three visible
    // edges of the polyface mesh
    assertXPath(pDoc, "/metafile/polyline", 7);
}

void DxfFilterTest::testPolygonMesh()
{
    // the drawing is 20 x 20, one unit is 500 in the metafile
    GDIMetaFile aMTF = importFile(u"mesh.dxf");
    CPPUNIT_ASSERT_EQUAL(Size(10100, 10100), aMTF.GetPrefSize());
    MetafileXmlDump aDumper;
    aDumper.filterAllActionTypes();
    aDumper.filterActionType(MetaActionType::POLYLINE, false);
    xmlDocUniquePtr pDoc = dumpAndParse(aDumper, aMTF);
    // the two rows and the three columns of the mesh, and the polyline
    assertXPath(pDoc, "/metafile/polyline", 6);
    assertXPath(pDoc, "/metafile/polyline[1]/point", 3);
    assertXPath(pDoc, "/metafile/polyline[3]/point", 2);
    // the vertex beyond the declared two changes neither the polyline nor the extents
    assertXPath(pDoc, "/metafile/polyline[6]/point", 2);
    assertXPath(pDoc, "/metafile/polyline[6]/point[2]", "x", u"10050");
    assertXPath(pDoc, "/metafile/polyline[6]/point[2]", "y", u"50");
}

void DxfFilterTest::testEllipse()
{
    // A full ellipse from y = -5 to 15 and two arcs; the one with the opposite extrusion is
    // mirrored and goes down to y = -20: the drawing is 140 x 35.
    GDIMetaFile aMTF = importFile(u"ellipse.dxf");
    CPPUNIT_ASSERT_EQUAL(Size(10100, 2600), aMTF.GetPrefSize());

    MetafileXmlDump aDumper;
    aDumper.filterAllActionTypes();
    aDumper.filterActionType(MetaActionType::POLYLINE, false);
    xmlDocUniquePtr pDoc = dumpAndParse(aDumper, aMTF);
    assertXPath(pDoc, "/metafile/polyline", 3);
    // the full ellipse is closed: four curve segments back to the start
    assertXPath(pDoc, "/metafile/polyline[1]/point[@flags='control']", 8);
    assertXPath(pDoc, "/metafile/polyline[1]/point[1]", "x", u"2907");
    assertXPath(pDoc, "/metafile/polyline[1]/point[last()]", "x", u"2907");
    // the arc with the vertical major axis starts at its top, (50, 10)
    assertXPath(pDoc, "/metafile/polyline[2]/point[1]", "x", u"5050");
    assertXPath(pDoc, "/metafile/polyline[2]/point[1]", "y", u"407");
}

void DxfFilterTest::testWorldCoordinates()
{
    // A line with the opposite extrusion direction is not mirrored: its coordinates are world
    // coordinates. The drawing is 30 x 10, one unit is 333.3 in the metafile.
    GDIMetaFile aMTF = importFile(u"world-coordinates.dxf");
    MetafileXmlDump aDumper;
    aDumper.filterAllActionTypes();
    aDumper.filterActionType(MetaActionType::POLYLINE, false);
    xmlDocUniquePtr pDoc = dumpAndParse(aDumper, aMTF);
    assertXPath(pDoc, "/metafile/polyline[1]/point[1]", "x", u"3383");
    assertXPath(pDoc, "/metafile/polyline[1]/point[2]", "x", u"6717");
}

void DxfFilterTest::testSpline()
{
    // the drawing is 80 wide and goes up to y = 10, one unit is 125 in the metafile
    GDIMetaFile aMTF = importFile(u"spline.dxf");
    MetafileXmlDump aDumper;
    aDumper.filterAllActionTypes();
    aDumper.filterActionType(MetaActionType::POLYLINE, false);
    xmlDocUniquePtr pDoc = dumpAndParse(aDumper, aMTF);
    assertXPath(pDoc, "/metafile/polyline", 3);

    // a clamped spline starts and ends on its end control points, (0, 0) and (30, 0)
    assertXPath(pDoc, "/metafile/polyline[1]/point[1]", "x", u"50");
    assertXPath(pDoc, "/metafile/polyline[1]/point[1]", "y", u"1300");
    assertXPath(pDoc, "/metafile/polyline[1]/point[last()]", "x", u"3800");
    assertXPath(pDoc, "/metafile/polyline[1]/point[last()]", "y", u"1300");

    // the weighted spline is a quarter of the circle around (40, 0) with a radius of 10
    const int nPoints = countXPathNodes(pDoc, "/metafile/polyline[2]/point");
    CPPUNIT_ASSERT_GREATER(4, nPoints);
    for (int i = 1; i <= nPoints; ++i)
    {
        const OString aPath("/metafile/polyline[2]/point[" + OString::number(i) + "]");
        const double fX = getXPath(pDoc, aPath, "x").toDouble() - 5050;
        const double fY = getXPath(pDoc, aPath, "y").toDouble() - 1300;
        CPPUNIT_ASSERT_DOUBLES_EQUAL(1250.0, std::hypot(fX, fY), 2.0);
    }

    // without control points the curve passes through the fit points
    assertXPath(pDoc, "/metafile/polyline[3]/point[1]", "x", u"7550");
    assertXPath(pDoc, "/metafile/polyline[3]/point[last()]", "x", u"10050");
    assertXPath(pDoc, "/metafile/polyline[3]/point[@flags='control']", 4);
}

void DxfFilterTest::testSplineKnots()
{
    // A wrong knot vector is replaced by a clamped uniform one, and the degree is at most the
    // number of control points less one: both curves go from their first to their last control
    // point. The curves reach y = 25: the drawing is 20 x 25, one unit is 400 in the metafile.
    GDIMetaFile aMTF = importFile(u"spline-knots.dxf");
    MetafileXmlDump aDumper;
    aDumper.filterAllActionTypes();
    aDumper.filterActionType(MetaActionType::POLYLINE, false);
    xmlDocUniquePtr pDoc = dumpAndParse(aDumper, aMTF);
    assertXPath(pDoc, "/metafile/polyline", 2);
    assertXPath(pDoc, "/metafile/polyline[1]/point[1]", "x", u"50");
    assertXPath(pDoc, "/metafile/polyline[1]/point[1]", "y", u"10050");
    assertXPath(pDoc, "/metafile/polyline[1]/point[last()]", "x", u"8050");
    assertXPath(pDoc, "/metafile/polyline[2]/point[1]", "y", u"2050");
    assertXPath(pDoc, "/metafile/polyline[2]/point[last()]", "x", u"8050");
}

CPPUNIT_TEST_SUITE_REGISTRATION(DxfFilterTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
