/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "charttest.hxx"
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/chart2/CurveStyle.hpp>
#include <com/sun/star/chart/ErrorBarStyle.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart/XChartDocument.hpp>
#include <com/sun/star/chart/XChartData.hpp>
#include <com/sun/star/chart2/XInternalDataProvider.hpp>
#include <com/sun/star/chart/XChartDataArray.hpp>

#include <com/sun/star/qa/XDumper.hpp>

#include <test/xmldiff.hxx>
#include <test/xmltesttools.hxx>

#include <fstream>

class Chart2XShapeTest : public ChartTest, public XmlTestTools
{
public:

    void testFdo75075();
    void testPropertyMappingBarChart();
    void testPieChartLabels1();
    void testTdf76649TrendLineBug();

    CPPUNIT_TEST_SUITE(Chart2XShapeTest);
    CPPUNIT_TEST(testFdo75075);
    CPPUNIT_TEST(testPropertyMappingBarChart);
    CPPUNIT_TEST(testPieChartLabels1);
    CPPUNIT_TEST(testTdf76649TrendLineBug);
    CPPUNIT_TEST_SUITE_END();

private:

    void compareAgainstReference(const OUString& rReferenceFile, bool bCreateReference = false);
    OUString getXShapeDumpString();
    xmlDocPtr getXShapeDumpXmlDoc();
};

namespace {

bool checkDumpAgainstFile( const OUString& rDump, const OUString& aFilePath)
{
    OString aOFile = OUStringToOString(aFilePath, RTL_TEXTENCODING_UTF8);

    CPPUNIT_ASSERT_MESSAGE("dump is empty", !rDump.isEmpty());

    OString aDump = OUStringToOString(rDump, RTL_TEXTENCODING_UTF8);
    return doXMLDiff(aOFile.getStr(), aDump.getStr(),
            static_cast<int>(rDump.getLength()), NULL);
}

}

OUString Chart2XShapeTest::getXShapeDumpString()
{
    uno::Reference< chart::XChartDocument > xChartDoc ( getChartCompFromSheet( 0, mxComponent ), UNO_QUERY_THROW);
    uno::Reference< qa::XDumper > xDumper( xChartDoc, UNO_QUERY_THROW );
    return xDumper->dump();
}

xmlDocPtr Chart2XShapeTest::getXShapeDumpXmlDoc()
{
    OUString rDump = getXShapeDumpString();
    OString aXmlDump = OUStringToOString(rDump, RTL_TEXTENCODING_UTF8);
    return xmlParseDoc(reinterpret_cast<const xmlChar*>(aXmlDump.getStr()));
}

void Chart2XShapeTest::compareAgainstReference(const OUString& rReferenceFile, bool bCreateReference)
{
    OUString aDump = getXShapeDumpString();

    OUString aReference = getPathFromSrc("/chart2/qa/extras/xshape/data/reference/") + rReferenceFile;
    if(bCreateReference)
    {
        OString aOFile = OUStringToOString(aReference, RTL_TEXTENCODING_UTF8);
        OString aODump = OUStringToOString(aDump, RTL_TEXTENCODING_UTF8);
        std::ofstream aReferenceFile(aOFile.getStr());
        aReferenceFile << aODump.getStr();
    }
    else
    {
        checkDumpAgainstFile(aDump, aReference);
    }
}

void Chart2XShapeTest::testFdo75075()
{
    load("chart2/qa/extras/xshape/data/ods/", "fdo75075.ods");
    compareAgainstReference("fdo75075.xml");
}

void Chart2XShapeTest::testPropertyMappingBarChart()
{
    load("chart2/qa/extras/xshape/data/ods/", "property-mapping-bar.ods");
    compareAgainstReference("property-mapping-bar.xml");
}

void Chart2XShapeTest::testPieChartLabels1()
{
    // inside placement for the best fit case
    load("chart2/qa/extras/xshape/data/xlsx/", "tdf90839-1.xlsx");
    compareAgainstReference("tdf90839-1.xml");
}

void Chart2XShapeTest::testTdf76649TrendLineBug()
{
    // This bug prevents that the trendline (regression curve) is drawn
    // if the first cell is empty. See tdf#76649 for details.

    load("chart2/qa/extras/xshape/data/ods/", "tdf76649_TrendLineBug.ods");

    xmlDocPtr pXmlDoc = getXShapeDumpXmlDoc();

    // Check if the regression curve exists (which means a XShape with a certain
    // name should exist in the dump)
    assertXPath(pXmlDoc, "//XShape[@name='CID/D=0:CS=0:CT=0:Series=0:Curve=0']", 1);
}

CPPUNIT_TEST_SUITE_REGISTRATION(Chart2XShapeTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
