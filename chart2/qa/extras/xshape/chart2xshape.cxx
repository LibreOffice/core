/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <charttest.hxx>
#include <com/sun/star/chart/XChartDocument.hpp>

#include <com/sun/star/qa/XDumper.hpp>

#include <test/xmldiff.hxx>
#include <test/xmltesttools.hxx>

#include <fstream>
#include <string_view>

class Chart2XShapeTest : public ChartTest
{
public:
    void testFdo75075();
    void testPropertyMappingBarChart();
    void testPieChartLabels1();
    void testPieChartLabels2();
    void testPieChartLabels3();
    void testPieChartLabels4();
    void testTdf76649TrendLineBug();
    void testTdf88154LabelRotatedLayout();

    CPPUNIT_TEST_SUITE(Chart2XShapeTest);
    CPPUNIT_TEST(testFdo75075);
    CPPUNIT_TEST(testPropertyMappingBarChart);
    CPPUNIT_TEST(testPieChartLabels1);
    CPPUNIT_TEST(testPieChartLabels2);
    CPPUNIT_TEST(testPieChartLabels3);
    CPPUNIT_TEST(testPieChartLabels4);
    CPPUNIT_TEST(testTdf76649TrendLineBug);
    CPPUNIT_TEST(testTdf88154LabelRotatedLayout);

    CPPUNIT_TEST_SUITE_END();

private:
    void compareAgainstReference(std::u16string_view rReferenceFile);
    OUString getXShapeDumpString();
    xmlDocUniquePtr getXShapeDumpXmlDoc();
};

namespace
{
bool checkDumpAgainstFile(const OUString& rDump, std::u16string_view aFilePath,
                          char const* toleranceFile)
{
    OString aOFile = OUStringToOString(aFilePath, RTL_TEXTENCODING_UTF8);

    CPPUNIT_ASSERT_MESSAGE("dump is empty", !rDump.isEmpty());

    OString aDump = OUStringToOString(rDump, RTL_TEXTENCODING_UTF8);
    return doXMLDiff(aOFile.getStr(), aDump.getStr(), static_cast<int>(rDump.getLength()),
                     toleranceFile);
}
}

OUString Chart2XShapeTest::getXShapeDumpString()
{
    uno::Reference<chart::XChartDocument> xChartDoc(getChartCompFromSheet(0, mxComponent),
                                                    UNO_QUERY_THROW);
    uno::Reference<qa::XDumper> xDumper(xChartDoc, UNO_QUERY_THROW);
    return xDumper->dump();
}

xmlDocUniquePtr Chart2XShapeTest::getXShapeDumpXmlDoc()
{
    OUString rDump = getXShapeDumpString();
    OString aXmlDump = OUStringToOString(rDump, RTL_TEXTENCODING_UTF8);
    return xmlDocUniquePtr(xmlParseDoc(reinterpret_cast<const xmlChar*>(aXmlDump.getStr())));
}

void Chart2XShapeTest::compareAgainstReference(std::u16string_view rReferenceFile)
{
    checkDumpAgainstFile(
        getXShapeDumpString(),
        OUStringConcatenation(
            m_directories.getPathFromSrc(u"/chart2/qa/extras/xshape/data/reference/")
            + rReferenceFile),
        OUStringToOString(
            m_directories.getPathFromSrc(u"/chart2/qa/extras/xshape/data/reference/tolerance.xml"),
            RTL_TEXTENCODING_UTF8)
            .getStr());
}

void Chart2XShapeTest::testFdo75075()
{
#if 0
    load("chart2/qa/extras/xshape/data/ods/", "fdo75075.ods");
    compareAgainstReference("fdo75075.xml");
#endif
}

void Chart2XShapeTest::testPropertyMappingBarChart()
{
#if 0
    load("chart2/qa/extras/xshape/data/ods/", "property-mapping-bar.ods");
    compareAgainstReference("property-mapping-bar.xml");
#endif
}

void Chart2XShapeTest::testPieChartLabels1()
{
    // FIXME: the DPI check should be removed when either (1) the test is fixed to work with
    // non-default DPI; or (2) unit tests on Windows are made to use svp VCL plugin.
    if (!IsDefaultDPI())
        return;

    // inside placement for the best fit case
    load(u"chart2/qa/extras/xshape/data/xlsx/", "tdf90839-1.xlsx");
    compareAgainstReference(u"tdf90839-1.xml");
}

void Chart2XShapeTest::testPieChartLabels2()
{
    // FIXME: the DPI check should be removed when either (1) the test is fixed to work with
    // non-default DPI; or (2) unit tests on Windows are made to use svp VCL plugin.
    if (!IsDefaultDPI())
        return;

    // text wrap: wrap all text labels except one
    load(u"chart2/qa/extras/xshape/data/xlsx/", "tdf90839-2.xlsx");
    compareAgainstReference(u"tdf90839-2.xml");
}

void Chart2XShapeTest::testPieChartLabels3()
{
    // FIXME: the DPI check should be removed when either (1) the test is fixed to work with
    // non-default DPI; or (2) unit tests on Windows are made to use svp VCL plugin.
    if (!IsDefaultDPI())
        return;

    // text wrap: wrap no text label except one
    load(u"chart2/qa/extras/xshape/data/xlsx/", "tdf90839-3.xlsx");
    compareAgainstReference(u"tdf90839-3.xml");
}

void Chart2XShapeTest::testPieChartLabels4()
{
    // FIXME: the DPI check should be removed when either (1) the test is fixed to work with
    // non-default DPI; or (2) unit tests on Windows are made to use svp VCL plugin.
    if (!IsDefaultDPI())
        return;

    // data value and percent value are centered horizontally
    load(u"chart2/qa/extras/xshape/data/ods/", "tdf90839-4.ods");
    compareAgainstReference(u"tdf90839-4.xml");
}

void Chart2XShapeTest::testTdf76649TrendLineBug()
{
    // This bug prevents that the trendline (regression curve) is drawn
    // if the first cell is empty. See tdf#76649 for details.

    load(u"chart2/qa/extras/xshape/data/ods/", "tdf76649_TrendLineBug.ods");

    xmlDocUniquePtr pXmlDoc = getXShapeDumpXmlDoc();

    // Check if the regression curve exists (which means a XShape with a certain
    // name should exist in the dump)
    assertXPath(pXmlDoc, "//XShape[@name='CID/D=0:CS=0:CT=0:Series=0:Curve=0']", 1);
}

void Chart2XShapeTest::testTdf88154LabelRotatedLayout()
{
    load(u"chart2/qa/extras/xshape/data/pptx/", "tdf88154_LabelRotatedLayout.pptx");
    uno::Reference<chart::XChartDocument> xChartDoc = getChartDocFromDrawImpress(0, 5);
    uno::Reference<qa::XDumper> xDumper(xChartDoc, UNO_QUERY_THROW);
    OUString rDump = xDumper->dump();
    OString aXmlDump = OUStringToOString(rDump, RTL_TEXTENCODING_UTF8);
    xmlDocUniquePtr pXmlDoc(xmlParseDoc(reinterpret_cast<const xmlChar*>(aXmlDump.getStr())));

    {
        OString aPath("//XShape[@text='Oct-12']/Transformation");
        assertXPath(pXmlDoc, aPath, 1);
        double fT11 = getXPath(pXmlDoc, aPath + "/Line1", "column1").toDouble();
        double fT12 = getXPath(pXmlDoc, aPath + "/Line1", "column2").toDouble();
        double fT21 = getXPath(pXmlDoc, aPath + "/Line2", "column1").toDouble();
        double fT22 = getXPath(pXmlDoc, aPath + "/Line2", "column2").toDouble();

        CPPUNIT_ASSERT_DOUBLES_EQUAL(fT11, -fT21, 1e-8);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fT12, fT22, 1e-8);
    }
    {
        OString aPath("//XShape[@text='Nov-12']/Transformation");
        assertXPath(pXmlDoc, aPath, 1);
        double fT11 = getXPath(pXmlDoc, aPath + "/Line1", "column1").toDouble();
        double fT12 = getXPath(pXmlDoc, aPath + "/Line1", "column2").toDouble();
        double fT21 = getXPath(pXmlDoc, aPath + "/Line2", "column1").toDouble();
        double fT22 = getXPath(pXmlDoc, aPath + "/Line2", "column2").toDouble();

        CPPUNIT_ASSERT_DOUBLES_EQUAL(fT11, -fT21, 1e-8);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fT12, fT22, 1e-8);
    }
    {
        OString aPath("//XShape[@text='Dec-12']/Transformation");
        assertXPath(pXmlDoc, aPath, 1);
        double fT11 = getXPath(pXmlDoc, aPath + "/Line1", "column1").toDouble();
        double fT12 = getXPath(pXmlDoc, aPath + "/Line1", "column2").toDouble();
        double fT21 = getXPath(pXmlDoc, aPath + "/Line2", "column1").toDouble();
        double fT22 = getXPath(pXmlDoc, aPath + "/Line2", "column2").toDouble();

        CPPUNIT_ASSERT_DOUBLES_EQUAL(fT11, -fT21, 1e-8);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fT12, fT22, 1e-8);
    }
    {
        OString aPath("//XShape[@text='May-13']/Transformation");
        assertXPath(pXmlDoc, aPath, 1);
        double fT11 = getXPath(pXmlDoc, aPath + "/Line1", "column1").toDouble();
        double fT12 = getXPath(pXmlDoc, aPath + "/Line1", "column2").toDouble();
        double fT21 = getXPath(pXmlDoc, aPath + "/Line2", "column1").toDouble();
        double fT22 = getXPath(pXmlDoc, aPath + "/Line2", "column2").toDouble();

        CPPUNIT_ASSERT_DOUBLES_EQUAL(fT11, -fT21, 1e-8);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fT12, fT22, 1e-8);
    }
    {
        OString aPath("//XShape[@text='Jan-14']/Transformation");
        assertXPath(pXmlDoc, aPath, 1);
        double fT11 = getXPath(pXmlDoc, aPath + "/Line1", "column1").toDouble();
        double fT12 = getXPath(pXmlDoc, aPath + "/Line1", "column2").toDouble();
        double fT21 = getXPath(pXmlDoc, aPath + "/Line2", "column1").toDouble();
        double fT22 = getXPath(pXmlDoc, aPath + "/Line2", "column2").toDouble();

        CPPUNIT_ASSERT_DOUBLES_EQUAL(fT11, -fT21, 1e-8);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fT12, fT22, 1e-8);
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(Chart2XShapeTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
