/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "charttest.hxx"

#include <com/sun/star/chart/ErrorBarStyle.hpp>
#include <com/sun/star/chart2/XRegressionCurveContainer.hpp>
#include <com/sun/star/chart2/XDataPointCustomLabelField.hpp>
#include <com/sun/star/chart2/DataPointCustomLabelFieldType.hpp>
#include <com/sun/star/chart2/RelativePosition.hpp>
#include <com/sun/star/chart2/MovingAverageType.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/chart2/DataPointLabel.hpp>
#include <com/sun/star/chart/DataLabelPlacement.hpp>

#include <libxml/xpathInternals.h>

#include <algorithm>

using uno::Reference;
using beans::XPropertySet;

class Chart2ExportTest2 : public ChartTest
{
protected:
    virtual void registerNamespaces(xmlXPathContextPtr& pXmlXPathCtx) override;

public:
    Chart2ExportTest2()
        : ChartTest()
    {
    }
    void testSetSeriesToSecondaryAxisXLSX();
    void testCombinedChartSecondaryAxisXLSX();
    void testCombinedChartSecondaryAxisODS();
    void testCrossBetweenXLSX();
    void testCrossBetweenWithDeletedAxis();
    void testCrossBetweenODS();
    void testAxisTitleRotationXLSX();
    void testAxisTitlePositionDOCX();
    void testAxisCrossBetweenDOCX();
    void testPieChartDataPointExplosionXLSX();
    void testCustomDataLabel();
    void testDataSeriesName();
    void testCustomPositionofDataLabel();
    void testCustomDataLabelMultipleSeries();
    void testLeaderLines();
    void testNumberFormatExportPPTX();
    void testLabelSeparatorExportDOCX();
    void testChartTitlePropertiesColorFillPPTX();
    void testChartTitlePropertiesGradientFillPPTX();
    void testChartTitlePropertiesBitmapFillPPTX();
    void testxAxisLabelsRotation();
    void testMultipleCategoryAxisLablesXLSX();
    void testMultipleCategoryAxisLablesDOCX();
    void testTdf116163();
    void testTdf111824();
    void test3DAreaChartZAxis();
    void testTdf119029();
    void testTdf108022();
    void testTdf121744();
    void testTdf121189();
    void testTdf122031();
    void testTdf115012();
    void testTdf134118();
    void testTdf123206_customLabelText();
    void testCustomLabelText();
    void testDeletedLegendEntries();
    void testTdf60316();
    void testTdf130225();
    void testTdf59857();
    void testTdf126076();
    void testTdf75330();
    void testTdf127792();
    void testTdf131979();
    void testTdf132076();
    void testTdf125812();
    void testTdf133190();
    void testTdf133191();
    void testTdf132594();
    void testTdf134255();
    void testTdf134977();
    void testTdf123647();
    void testTdf136267();
    void testDataLabelPlacementPieChart();
    void testTdf137917();
    void testTdf138204();
    void testTdf138181();
    void testCustomShapeText();
    void testuserShapesXLSX();
    void testNameRangeXLSX();

    CPPUNIT_TEST_SUITE(Chart2ExportTest2);
    CPPUNIT_TEST(testSetSeriesToSecondaryAxisXLSX);
    CPPUNIT_TEST(testCombinedChartSecondaryAxisXLSX);
    CPPUNIT_TEST(testCombinedChartSecondaryAxisODS);
    CPPUNIT_TEST(testCrossBetweenXLSX);
    CPPUNIT_TEST(testCrossBetweenWithDeletedAxis);
    CPPUNIT_TEST(testCrossBetweenODS);
    CPPUNIT_TEST(testAxisTitleRotationXLSX);
    CPPUNIT_TEST(testAxisTitlePositionDOCX);
    CPPUNIT_TEST(testAxisCrossBetweenDOCX);
    CPPUNIT_TEST(testPieChartDataPointExplosionXLSX);
    CPPUNIT_TEST(testCustomDataLabel);
    CPPUNIT_TEST(testDataSeriesName);
    CPPUNIT_TEST(testCustomPositionofDataLabel);
    CPPUNIT_TEST(testCustomDataLabelMultipleSeries);
    CPPUNIT_TEST(testLeaderLines);
    CPPUNIT_TEST(testNumberFormatExportPPTX);
    CPPUNIT_TEST(testLabelSeparatorExportDOCX);
    CPPUNIT_TEST(testChartTitlePropertiesColorFillPPTX);
    CPPUNIT_TEST(testChartTitlePropertiesGradientFillPPTX);
    CPPUNIT_TEST(testChartTitlePropertiesBitmapFillPPTX);
    CPPUNIT_TEST(testxAxisLabelsRotation);
    CPPUNIT_TEST(testMultipleCategoryAxisLablesXLSX);
    CPPUNIT_TEST(testMultipleCategoryAxisLablesDOCX);
    CPPUNIT_TEST(testTdf116163);
    CPPUNIT_TEST(testTdf111824);
    CPPUNIT_TEST(test3DAreaChartZAxis);
    CPPUNIT_TEST(testTdf119029);
    CPPUNIT_TEST(testTdf108022);
    CPPUNIT_TEST(testTdf121744);
    CPPUNIT_TEST(testTdf121189);
    CPPUNIT_TEST(testTdf122031);
    CPPUNIT_TEST(testTdf115012);
    CPPUNIT_TEST(testTdf134118);
    CPPUNIT_TEST(testTdf123206_customLabelText);
    CPPUNIT_TEST(testCustomLabelText);
    CPPUNIT_TEST(testDeletedLegendEntries);
    CPPUNIT_TEST(testTdf60316);
    CPPUNIT_TEST(testTdf130225);
    CPPUNIT_TEST(testTdf59857);
    CPPUNIT_TEST(testTdf126076);
    CPPUNIT_TEST(testTdf75330);
    CPPUNIT_TEST(testTdf127792);
    CPPUNIT_TEST(testTdf131979);
    CPPUNIT_TEST(testTdf132076);
    CPPUNIT_TEST(testTdf125812);
    CPPUNIT_TEST(testTdf133190);
    CPPUNIT_TEST(testTdf133191);
    CPPUNIT_TEST(testTdf132594);
    CPPUNIT_TEST(testTdf134255);
    CPPUNIT_TEST(testTdf134977);
    CPPUNIT_TEST(testTdf123647);
    CPPUNIT_TEST(testTdf136267);
    CPPUNIT_TEST(testDataLabelPlacementPieChart);
    CPPUNIT_TEST(testTdf137917);
    CPPUNIT_TEST(testTdf138204);
    CPPUNIT_TEST(testTdf138181);
    CPPUNIT_TEST(testCustomShapeText);
    CPPUNIT_TEST(testuserShapesXLSX);
    CPPUNIT_TEST(testNameRangeXLSX);
    CPPUNIT_TEST_SUITE_END();
};

void Chart2ExportTest2::registerNamespaces(xmlXPathContextPtr& pXmlXPathCtx)
{
    XmlTestTools::registerOOXMLNamespaces(pXmlXPathCtx);
}

void Chart2ExportTest2::testSetSeriesToSecondaryAxisXLSX()
{
    load(u"/chart2/qa/extras/data/xlsx/", "add_series_secondary_axis.xlsx");
    Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    // Second series
    Reference<chart2::XDataSeries> xSeries = getDataSeriesFromDoc(xChartDoc, 1);
    CPPUNIT_ASSERT(xSeries.is());

    Reference<beans::XPropertySet> xPropSet(xSeries, uno::UNO_QUERY_THROW);
    sal_Int32 AxisIndex = 1;
    // Attach the second series to the secondary axis. (The third series is already attached.)
    xPropSet->setPropertyValue("AttachedAxisIndex", uno::Any(AxisIndex));

    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);
    // Check there are only two <lineChart> tag in the XML, one for the primary and one for the secondary axis.
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart", 2);
}

void Chart2ExportTest2::testCombinedChartSecondaryAxisXLSX()
{
    // Original file was created with MS Office
    load(u"/chart2/qa/extras/data/xlsx/", "combined_chart_secondary_axis.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);
    // Collect barchart axID on secondary Axis
    OUString XValueIdOfBarchart
        = getXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:axId[1]", "val");
    OUString YValueIdOfBarchart
        = getXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:axId[2]", "val");
    // Collect linechart axID on primary Axis
    OUString XValueIdOfLinechart
        = getXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:axId[1]", "val");
    OUString YValueIdOfLinechart
        = getXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:axId[2]", "val");
    // Check which c:catAx and c:valAx contain the AxisId of charttypes
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:catAx[1]/c:axId", "val",
                XValueIdOfLinechart);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx[1]/c:axId", "val",
                YValueIdOfLinechart);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:catAx[2]/c:axId", "val",
                XValueIdOfBarchart);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx[2]/c:axId", "val",
                YValueIdOfBarchart);
}

void Chart2ExportTest2::testCombinedChartSecondaryAxisODS()
{
    // Original file was created with LibreOffice
    load(u"/chart2/qa/extras/data/ods/", "combined_chart_secondary_axis.ods");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);
    // Collect barchart axID on secondary Axis
    OUString XValueIdOfBarchart
        = getXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:axId[1]", "val");
    OUString YValueIdOfBarchart
        = getXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:axId[2]", "val");
    // Collect linechart axID on primary Axis
    OUString XValueIdOfLinechart
        = getXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:axId[1]", "val");
    OUString YValueIdOfLinechart
        = getXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:axId[2]", "val");
    // Check which c:catAx and c:valAx contain the AxisId of charttypes
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:catAx[1]/c:axId", "val",
                XValueIdOfLinechart);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx[1]/c:axId", "val",
                YValueIdOfLinechart);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:catAx[2]/c:axId", "val",
                XValueIdOfBarchart);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx[2]/c:axId", "val",
                YValueIdOfBarchart);
    // do not need CT_crosses tag if the actual axis is deleted, so we need to make sure it is not saved
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:catAx[2]/c:crosses", 0);
}

void Chart2ExportTest2::testCrossBetweenXLSX()
{
    // Original files were created with MS Office
    {
        load(u"/chart2/qa/extras/data/xlsx/", "tdf127777.xlsx");
        xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
        CPPUNIT_ASSERT(pXmlDoc);
        assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx/c:crossBetween", "val",
                    "between");
    }
    {
        load(u"/chart2/qa/extras/data/xlsx/", "tdf132076.xlsx");
        xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
        CPPUNIT_ASSERT(pXmlDoc);
        assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx/c:crossBetween", "val",
                    "between");
    }
}

void Chart2ExportTest2::testCrossBetweenWithDeletedAxis()
{
    // Original file was created with MS Office (the category axis is deleted in the file)
    load(u"/chart2/qa/extras/data/xlsx/", "tdf128633.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx/c:crossBetween", "val",
                "between");
}

void Chart2ExportTest2::testCrossBetweenODS()
{
    // Original file was created with LibreOffice
    load(u"/chart2/qa/extras/data/ods/", "test_CrossBetween.ods");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx/c:crossBetween", "val",
                "between");
}

void Chart2ExportTest2::testAxisTitleRotationXLSX()
{
    load(u"/chart2/qa/extras/data/xlsx/", "axis_title_rotation.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx/c:title/c:tx/c:rich/a:bodyPr",
                "rot", "0");
}

void Chart2ExportTest2::testAxisTitlePositionDOCX()
{
    load(u"/chart2/qa/extras/data/docx/", "testAxisTitlePosition.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart", "Office Open XML Text");
    CPPUNIT_ASSERT(pXmlDoc);

    // test X Axis title position
    OUString aXVal = getXPath(
        pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:catAx/c:title/c:layout/c:manualLayout/c:x",
        "val");
    double nX = aXVal.toDouble();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.698208543867708, nX, 1e-3);
    OUString aYVal = getXPath(
        pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:catAx/c:title/c:layout/c:manualLayout/c:y",
        "val");
    double nY = aYVal.toDouble();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.805152435594555, nY, 1e-3);

    // test Y Axis title position
    aXVal = getXPath(pXmlDoc,
                     "/c:chartSpace/c:chart/c:plotArea/c:valAx/c:title/c:layout/c:manualLayout/c:x",
                     "val");
    nX = aXVal.toDouble();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0253953671500755, nX, 1e-3);
    aYVal = getXPath(pXmlDoc,
                     "/c:chartSpace/c:chart/c:plotArea/c:valAx/c:title/c:layout/c:manualLayout/c:y",
                     "val");
    nY = aYVal.toDouble();
    // just test the first two decimal digits because it is not perfect in docx yet.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.384070199122511, nY, 1e-2);
}

void Chart2ExportTest2::testAxisCrossBetweenDOCX()
{
    load(u"/chart2/qa/extras/data/odt/", "axis-position.odt");
    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart", "Office Open XML Text");
    assertXPath(pXmlDoc, "(//c:crossBetween)[1]", "val", "midCat");
}

void Chart2ExportTest2::testPieChartDataPointExplosionXLSX()
{
    load(u"/chart2/qa/extras/data/xlsx/", "pie_chart_datapoint_explosion.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:pieChart/c:ser/c:dPt/c:explosion",
                "val", "28");
}

void Chart2ExportTest2::testCustomDataLabel()
{
    load(u"/chart2/qa/extras/data/pptx/", "tdf115107.pptx");
    xmlDocUniquePtr pXmlDoc = parseExport("ppt/charts/chart1", "Impress MS PowerPoint 2007 XML");
    CPPUNIT_ASSERT(pXmlDoc);
    // Check the data labels font color for the complete data series
    assertXPath(pXmlDoc,
                "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dLbls/c:txPr/a:p/a:pPr/"
                "a:defRPr/a:solidFill/a:srgbClr",
                "val", "404040");

    Reference<chart2::XChartDocument> xChartDoc(getChartDocFromDrawImpress(0, 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());

    uno::Reference<chart2::XDataSeries> xDataSeries(getDataSeriesFromDoc(xChartDoc, 0));
    CPPUNIT_ASSERT(xDataSeries.is());
    float nFontSize;
    sal_Int64 nFontColor;
    sal_Int32 nCharUnderline;
    uno::Reference<beans::XPropertySet> xPropertySet;
    uno::Sequence<uno::Reference<chart2::XDataPointCustomLabelField>> aFields;

    // 1
    xPropertySet.set(xDataSeries->getDataPointByIndex(0), uno::UNO_SET_THROW);
    xPropertySet->getPropertyValue("CustomLabelFields") >>= aFields;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), aFields.getLength());

    CPPUNIT_ASSERT_EQUAL(chart2::DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_TEXT,
                         aFields[0]->getFieldType());
    CPPUNIT_ASSERT_EQUAL(OUString("90.0 = "), aFields[0]->getString());
    aFields[0]->getPropertyValue("CharHeight") >>= nFontSize;
    aFields[0]->getPropertyValue("CharColor") >>= nFontColor;
    CPPUNIT_ASSERT_EQUAL(static_cast<float>(18), nFontSize);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int64>(0xed7d31), nFontColor);

    CPPUNIT_ASSERT_EQUAL(chart2::DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_VALUE,
                         aFields[1]->getFieldType());
    CPPUNIT_ASSERT_EQUAL(OUString("90"), aFields[1]->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("{0C576297-5A9F-4B4E-A675-B6BA406B7D87}"), aFields[1]->getGuid());

    // 2
    xPropertySet.set(xDataSeries->getDataPointByIndex(1), uno::UNO_SET_THROW);
    xPropertySet->getPropertyValue("CustomLabelFields") >>= aFields;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(8), aFields.getLength());

    CPPUNIT_ASSERT_EQUAL(chart2::DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_TEXT,
                         aFields[0]->getFieldType());
    CPPUNIT_ASSERT_EQUAL(OUString("Text"), aFields[0]->getString());

    CPPUNIT_ASSERT_EQUAL(chart2::DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_TEXT,
                         aFields[1]->getFieldType());
    CPPUNIT_ASSERT_EQUAL(OUString(" : "), aFields[1]->getString());

    CPPUNIT_ASSERT_EQUAL(
        chart2::DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_CATEGORYNAME,
        aFields[2]->getFieldType());
    CPPUNIT_ASSERT_EQUAL(OUString("B"), aFields[2]->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("{0CCAAACD-B393-42CE-8DBD-82F9F9ADC852}"), aFields[2]->getGuid());
    aFields[2]->getPropertyValue("CharHeight") >>= nFontSize;
    aFields[2]->getPropertyValue("CharColor") >>= nFontColor;
    CPPUNIT_ASSERT_EQUAL(static_cast<float>(16), nFontSize);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int64>(0xed7d31), nFontColor);

    CPPUNIT_ASSERT_EQUAL(
        chart2::DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_NEWLINE,
        aFields[3]->getFieldType());

    CPPUNIT_ASSERT_EQUAL(chart2::DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_TEXT,
                         aFields[4]->getFieldType());
    CPPUNIT_ASSERT_EQUAL(OUString("Multi"), aFields[4]->getString());

    CPPUNIT_ASSERT_EQUAL(chart2::DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_TEXT,
                         aFields[5]->getFieldType());
    CPPUNIT_ASSERT_EQUAL(OUString("line"), aFields[5]->getString());
    aFields[5]->getPropertyValue("CharHeight") >>= nFontSize;
    aFields[5]->getPropertyValue("CharColor") >>= nFontColor;
    CPPUNIT_ASSERT_EQUAL(static_cast<float>(11.97), nFontSize);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int64>(0xbf9000), nFontColor);

    CPPUNIT_ASSERT_EQUAL(
        chart2::DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_NEWLINE,
        aFields[6]->getFieldType());

    CPPUNIT_ASSERT_EQUAL(chart2::DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_TEXT,
                         aFields[7]->getFieldType());
    CPPUNIT_ASSERT_EQUAL(OUString("Abc"), aFields[7]->getString());
    aFields[7]->getPropertyValue("CharHeight") >>= nFontSize;
    aFields[7]->getPropertyValue("CharColor") >>= nFontColor;
    aFields[7]->getPropertyValue("CharUnderline") >>= nCharUnderline;
    CPPUNIT_ASSERT_EQUAL(static_cast<float>(12), nFontSize);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int64>(0xa9d18e), nFontColor);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), nCharUnderline);

    // 3
    xPropertySet.set(xDataSeries->getDataPointByIndex(2), uno::UNO_SET_THROW);
    xPropertySet->getPropertyValue("CustomLabelFields") >>= aFields;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), aFields.getLength());

    CPPUNIT_ASSERT_EQUAL(
        chart2::DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_SERIESNAME,
        aFields[0]->getFieldType());
    CPPUNIT_ASSERT_EQUAL(OUString("DATA"), aFields[0]->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("{C8F3EB90-8960-4F9A-A3AD-B4FAC4FE4566}"), aFields[0]->getGuid());

    // 4
    xPropertySet.set(xDataSeries->getDataPointByIndex(3), uno::UNO_SET_THROW);
    xPropertySet->getPropertyValue("CustomLabelFields") >>= aFields;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), aFields.getLength());

    CPPUNIT_ASSERT_EQUAL(
        chart2::DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_CELLREF,
        aFields[0]->getFieldType());
    //CPPUNIT_ASSERT_EQUAL(OUString("70"), aFields[0]->getString()); TODO: Not implemented yet

    CPPUNIT_ASSERT_EQUAL(chart2::DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_TEXT,
                         aFields[1]->getFieldType());
    CPPUNIT_ASSERT_EQUAL(OUString(" <CELLREF"), aFields[1]->getString());
}

/// Test for tdf#94235
void Chart2ExportTest2::testDataSeriesName()
{
    // ODF
    {
        load(u"/chart2/qa/extras/data/ods/", "ser_labels.ods");
        reload("calc8");
        uno::Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
        uno::Reference<chart2::XDataSeries> xDataSeries(getDataSeriesFromDoc(xChartDoc, 0));
        CPPUNIT_ASSERT(xDataSeries.is());
        uno::Reference<beans::XPropertySet> xPropertySet;
        chart2::DataPointLabel aDataPointLabel;
        xPropertySet.set(xDataSeries->getDataPointByIndex(0), uno::UNO_SET_THROW);
        xPropertySet->getPropertyValue("Label") >>= aDataPointLabel;
        CPPUNIT_ASSERT_EQUAL(sal_True, aDataPointLabel.ShowSeriesName);
    }

    // OOXML
    {
        load(u"/chart2/qa/extras/data/xlsx/", "ser_labels.xlsx");
        reload("Calc Office Open XML");
        uno::Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
        uno::Reference<chart2::XDataSeries> xDataSeries(getDataSeriesFromDoc(xChartDoc, 0));
        CPPUNIT_ASSERT(xDataSeries.is());
        uno::Reference<beans::XPropertySet> xPropertySet;
        chart2::DataPointLabel aDataPointLabel;
        xPropertySet.set(xDataSeries->getDataPointByIndex(0), uno::UNO_SET_THROW);
        xPropertySet->getPropertyValue("Label") >>= aDataPointLabel;
        CPPUNIT_ASSERT_EQUAL(sal_True, aDataPointLabel.ShowSeriesName);
    }
}

void Chart2ExportTest2::testCustomPositionofDataLabel()
{
    load(u"/chart2/qa/extras/data/xlsx/", "testCustomPosDataLabels.xlsx");
    {
        xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
        CPPUNIT_ASSERT(pXmlDoc);

        // test custom position of data label (xlsx)
        assertXPath(pXmlDoc,
                    "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:ser/c:dLbls/c:dLbl[1]/c:idx",
                    "val", "2");
        OUString aXVal = getXPath(pXmlDoc,
                                  "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:ser/c:dLbls/"
                                  "c:dLbl[1]/c:layout/c:manualLayout/c:x",
                                  "val");
        double nX = aXVal.toDouble();
        CPPUNIT_ASSERT_DOUBLES_EQUAL(-0.11027682973075476, nX, 1e-7);

        OUString aYVal = getXPath(pXmlDoc,
                                  "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:ser/c:dLbls/"
                                  "c:dLbl[1]/c:layout/c:manualLayout/c:y",
                                  "val");
        double nY = aYVal.toDouble();
        CPPUNIT_ASSERT_DOUBLES_EQUAL(-0.0742140311063737, nY, 1e-7);
    }

    load(u"/chart2/qa/extras/data/docx/", "testTdf108110.docx");
    {
        xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart", "Office Open XML Text");
        CPPUNIT_ASSERT(pXmlDoc);

        // test custom position of data label (docx)
        assertXPath(pXmlDoc,
                    "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:ser/c:dLbls/c:dLbl[2]/c:idx",
                    "val", "2");
        OUString aXVal = getXPath(pXmlDoc,
                                  "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:ser/c:dLbls/"
                                  "c:dLbl[2]/c:layout/c:manualLayout/c:x",
                                  "val");
        double nX = aXVal.toDouble();
        CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0227256488772236, nX, 1e-7);

        OUString aYVal = getXPath(pXmlDoc,
                                  "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:ser/c:dLbls/"
                                  "c:dLbl[2]/c:layout/c:manualLayout/c:y",
                                  "val");
        double nY = aYVal.toDouble();
        CPPUNIT_ASSERT_DOUBLES_EQUAL(0.172648731408574, nY, 1e-7);
    }

    load(u"/chart2/qa/extras/data/ods/", "tdf136024.ods");
    {
        reload("calc8");
        // tdf#136024: test custom position of pie chart data label after an ods export
        Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
        CPPUNIT_ASSERT(xChartDoc.is());

        Reference<chart2::XDataSeries> xDataSeries(getDataSeriesFromDoc(xChartDoc, 0));
        CPPUNIT_ASSERT(xDataSeries.is());
        uno::Reference<beans::XPropertySet> xPropertySet(xDataSeries->getDataPointByIndex(0),
                                                         uno::UNO_SET_THROW);

        chart2::RelativePosition aCustomLabelPosition;
        CPPUNIT_ASSERT(xPropertySet->getPropertyValue("CustomLabelPosition")
                       >>= aCustomLabelPosition);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(aCustomLabelPosition.Primary, -0.0961935120945059, 1e-5);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(aCustomLabelPosition.Secondary, 0.209578842093566, 1e-5);
    }
}

void Chart2ExportTest2::testCustomDataLabelMultipleSeries()
{
    load(u"/chart2/qa/extras/data/pptx/", "tdf115107-2.pptx");
    xmlDocUniquePtr pXmlDoc = parseExport("ppt/charts/chart2", "Impress MS PowerPoint 2007 XML");
    CPPUNIT_ASSERT(pXmlDoc);

    Reference<chart2::XChartDocument> xChartDoc(getChartDocFromDrawImpress(0, 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());

    uno::Reference<chart2::XDataSeries> xDataSeries(getDataSeriesFromDoc(xChartDoc, 0));
    CPPUNIT_ASSERT(xDataSeries.is());
    float nFontSize;
    sal_Int64 nFontColor;
    uno::Reference<beans::XPropertySet> xPropertySet;
    uno::Sequence<uno::Reference<chart2::XDataPointCustomLabelField>> aFields;

    // First series
    xPropertySet.set(xDataSeries->getDataPointByIndex(0), uno::UNO_SET_THROW);
    xPropertySet->getPropertyValue("CustomLabelFields") >>= aFields;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3), aFields.getLength());

    CPPUNIT_ASSERT_EQUAL(chart2::DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_VALUE,
                         aFields[0]->getFieldType());
    CPPUNIT_ASSERT_EQUAL(OUString("4.3"), aFields[0]->getString());
    aFields[0]->getPropertyValue("CharHeight") >>= nFontSize;
    aFields[0]->getPropertyValue("CharColor") >>= nFontColor;
    CPPUNIT_ASSERT_EQUAL(static_cast<float>(18), nFontSize);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int64>(0xc00000), nFontColor);

    CPPUNIT_ASSERT_EQUAL(chart2::DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_TEXT,
                         aFields[1]->getFieldType());
    CPPUNIT_ASSERT_EQUAL(OUString(" "), aFields[1]->getString());

    CPPUNIT_ASSERT_EQUAL(
        chart2::DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_SERIESNAME,
        aFields[2]->getFieldType());
    CPPUNIT_ASSERT_EQUAL(OUString("Bars"), aFields[2]->getString());

    // Second series
    xDataSeries = getDataSeriesFromDoc(xChartDoc, 0, 1);
    CPPUNIT_ASSERT(xDataSeries.is());

    xPropertySet.set(xDataSeries->getDataPointByIndex(0), uno::UNO_SET_THROW);
    xPropertySet->getPropertyValue("CustomLabelFields") >>= aFields;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3), aFields.getLength());

    CPPUNIT_ASSERT_EQUAL(chart2::DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_VALUE,
                         aFields[0]->getFieldType());
    CPPUNIT_ASSERT_EQUAL(OUString("2"), aFields[0]->getString());
    aFields[0]->getPropertyValue("CharHeight") >>= nFontSize;
    aFields[0]->getPropertyValue("CharColor") >>= nFontColor;
    CPPUNIT_ASSERT_EQUAL(static_cast<float>(18), nFontSize);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int64>(0xffd966), nFontColor);

    CPPUNIT_ASSERT_EQUAL(chart2::DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_TEXT,
                         aFields[1]->getFieldType());
    CPPUNIT_ASSERT_EQUAL(OUString(" "), aFields[1]->getString());

    CPPUNIT_ASSERT_EQUAL(
        chart2::DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_SERIESNAME,
        aFields[2]->getFieldType());
    CPPUNIT_ASSERT_EQUAL(OUString("Line"), aFields[2]->getString());
}

void Chart2ExportTest2::testLeaderLines()
{
    load(u"/chart2/qa/extras/data/xlsx/", "testTdf90749.xlsx");
    {
        xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
        CPPUNIT_ASSERT(pXmlDoc);
        assertXPath(pXmlDoc,
                    "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:ser[1]/c:dLbls/c:extLst/c:ext/"
                    "c15:showLeaderLines",
                    "val", "1");
        assertXPath(pXmlDoc,
                    "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:ser[2]/c:dLbls/c:extLst/c:ext/"
                    "c15:showLeaderLines",
                    "val", "0");
    }
    load(u"/chart2/qa/extras/data/docx/", "MSO_Custom_Leader_Line.docx");
    {
        xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart1", "Office Open XML Text");
        CPPUNIT_ASSERT(pXmlDoc);
        // tdf#134571: Check the leader line is switch off.
        assertXPath(pXmlDoc,
                    "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dLbls/c:extLst/c:ext/"
                    "c15:showLeaderLines",
                    "val", "0");
    }
}

void Chart2ExportTest2::testNumberFormatExportPPTX()
{
    load(u"/chart2/qa/extras/data/pptx/", "tdf115859.pptx");
    xmlDocUniquePtr pXmlDoc = parseExport("ppt/charts/chart", "Impress MS PowerPoint 2007 XML");
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dLbls/c:numFmt",
                "formatCode", "#,##0.00,\\K");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dLbls/c:numFmt",
                "sourceLinked", "0");
}

void Chart2ExportTest2::testLabelSeparatorExportDOCX()
{
    load(u"/chart2/qa/extras/data/docx/", "testLabelSeparator.docx");

    Reference<chart2::XChartDocument> xChartDoc(getChartDocFromWriter(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());

    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart", "Office Open XML Text");
    CPPUNIT_ASSERT(pXmlDoc);

    // The text separator should be a new line
    assertXPathContent(
        pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[1]/c:dLbls/c:separator", "\n");
    // The text separator should be a comma
    assertXPathContent(
        pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[2]/c:dLbls/c:separator", ", ");
    // The text separator should be a semicolon
    assertXPathContent(
        pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[3]/c:dLbls/c:separator", "; ");
}

void Chart2ExportTest2::testChartTitlePropertiesColorFillPPTX()
{
    load(u"/chart2/qa/extras/data/pptx/", "testChartTitlePropertiesColorFill.pptx");
    xmlDocUniquePtr pXmlDoc = parseExport("ppt/charts/chart", "Impress MS PowerPoint 2007 XML");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:solidFill/a:srgbClr", "val",
                "ff0000");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:ln/a:noFill", 1);
}

void Chart2ExportTest2::testChartTitlePropertiesGradientFillPPTX()
{
    load(u"/chart2/qa/extras/data/pptx/", "testChartTitlePropertiesGradientFill.pptx");
    xmlDocUniquePtr pXmlDoc = parseExport("ppt/charts/chart", "Impress MS PowerPoint 2007 XML");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc,
                "/c:chartSpace/c:chart/c:title/c:spPr/a:gradFill/a:gsLst/a:gs[1]/a:srgbClr", "val",
                "f6f8fc");
    assertXPath(pXmlDoc,
                "/c:chartSpace/c:chart/c:title/c:spPr/a:gradFill/a:gsLst/a:gs[2]/a:srgbClr", "val",
                "c7d5ed");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:ln/a:noFill", 1);
}

void Chart2ExportTest2::testChartTitlePropertiesBitmapFillPPTX()
{
    load(u"/chart2/qa/extras/data/pptx/", "testChartTitlePropertiesBitmapFill.pptx");
    xmlDocUniquePtr pXmlDoc = parseExport("ppt/charts/chart", "Impress MS PowerPoint 2007 XML");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:blipFill/a:blip", "embed", "rId1");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:ln/a:noFill", 1);
}

void Chart2ExportTest2::testxAxisLabelsRotation()
{
    load(u"/chart2/qa/extras/data/xlsx/", "xAxisLabelsRotation.xlsx");
    xmlDocUniquePtr pXmlDoc1 = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc1);

    // Chart1 xAxis labels should be 45 degree
    assertXPath(pXmlDoc1, "/c:chartSpace/c:chart/c:plotArea/c:catAx/c:txPr/a:bodyPr", "rot",
                "2700000");
}

void Chart2ExportTest2::testMultipleCategoryAxisLablesXLSX()
{
    load(u"/chart2/qa/extras/data/ods/", "multilevelcat.ods");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);
    // check category axis labels number of first level
    assertXPath(pXmlDoc,
                "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[1]/c:cat/c:multiLvlStrRef/"
                "c:multiLvlStrCache/c:ptCount",
                "val", "6");
    // check category axis labels text of first level
    assertXPathContent(pXmlDoc,
                       "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[1]/c:cat/"
                       "c:multiLvlStrRef/c:multiLvlStrCache/c:lvl[1]/c:pt[1]/c:v",
                       "Categoria 1");
    assertXPathContent(pXmlDoc,
                       "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[1]/c:cat/"
                       "c:multiLvlStrRef/c:multiLvlStrCache/c:lvl[1]/c:pt[6]/c:v",
                       "Categoria 6");
    // check category axis labels text of second level
    assertXPathContent(pXmlDoc,
                       "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[1]/c:cat/"
                       "c:multiLvlStrRef/c:multiLvlStrCache/c:lvl[2]/c:pt[1]/c:v",
                       "2011");
    assertXPathContent(pXmlDoc,
                       "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[1]/c:cat/"
                       "c:multiLvlStrRef/c:multiLvlStrCache/c:lvl[2]/c:pt[3]/c:v",
                       "2013");
    // check the 'noMultiLvlLbl' tag - ChartExport.cxx:2950 FIXME: seems not support, so check the default noMultiLvlLbl value.
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:catAx/c:noMultiLvlLbl", "val", "0");
}

void Chart2ExportTest2::testMultipleCategoryAxisLablesDOCX()
{
    load(u"/chart2/qa/extras/data/odt/", "multilevelcat.odt");
    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart", "Office Open XML Text");
    CPPUNIT_ASSERT(pXmlDoc);
    // check category axis labels number of first level
    assertXPath(pXmlDoc,
                "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[1]/c:cat/c:multiLvlStrRef/"
                "c:multiLvlStrCache/c:ptCount",
                "val", "4");
    // check category axis labels text of first level
    assertXPathContent(pXmlDoc,
                       "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[1]/c:cat/"
                       "c:multiLvlStrRef/c:multiLvlStrCache/c:lvl[1]/c:pt[1]/c:v",
                       "Categoria 1");
    assertXPathContent(pXmlDoc,
                       "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[1]/c:cat/"
                       "c:multiLvlStrRef/c:multiLvlStrCache/c:lvl[1]/c:pt[4]/c:v",
                       "Categoria 4");
    // check category axis labels text of second level
    assertXPathContent(pXmlDoc,
                       "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[1]/c:cat/"
                       "c:multiLvlStrRef/c:multiLvlStrCache/c:lvl[2]/c:pt[1]/c:v",
                       "2011");
    assertXPathContent(pXmlDoc,
                       "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[1]/c:cat/"
                       "c:multiLvlStrRef/c:multiLvlStrCache/c:lvl[2]/c:pt[2]/c:v",
                       "2012");
    // check the 'noMultiLvlLbl' tag - ChartExport.cxx:2950 FIXME: seems not support, so check the default noMultiLvlLbl value.
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:catAx/c:noMultiLvlLbl", "val", "0");
}

void Chart2ExportTest2::testTdf116163()
{
    load(u"/chart2/qa/extras/data/pptx/", "tdf116163.pptx");
    xmlDocUniquePtr pXmlDoc = parseExport("ppt/charts/chart", "Impress MS PowerPoint 2007 XML");
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:catAx/c:txPr/a:bodyPr", "rot",
                "-5400000");
}

void Chart2ExportTest2::testTdf111824()
{
    load(u"/chart2/qa/extras/data/xlsx/", "tdf111824.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);

    // Collect 3D barchart Z axID
    OUString zAxisIdOf3DBarchart
        = getXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:bar3DChart/c:axId[3]", "val");
    // 3D barchart Z axis properties should be in a serAx OOXML tag instead of catAx
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:serAx/c:axId", "val",
                zAxisIdOf3DBarchart);
}

void Chart2ExportTest2::test3DAreaChartZAxis()
{
    load(u"/chart2/qa/extras/data/xlsx/", "test3DAreaChartZAxis.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);

    // Collect 3D area chart Z axID
    OUString zAxisIdOf3DAreachart
        = getXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:area3DChart/c:axId[3]", "val");
    // 3D area chart z-axis properties should be in a serAx OOXML element instead of catAx
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:serAx/c:axId", "val",
                zAxisIdOf3DAreachart);
}

void Chart2ExportTest2::testTdf119029()
{
    load(u"/chart2/qa/extras/data/odp/", "tdf119029.odp");
    // Only use "chart", without number, because the number depends on the previous tests
    xmlDocUniquePtr pXmlDoc = parseExport("ppt/charts/chart", "Impress MS PowerPoint 2007 XML");
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc,
                "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dLbls/c:txPr/a:bodyPr", "rot",
                "-5400000");
}

void Chart2ExportTest2::testTdf108022()
{
    load(u"/chart2/qa/extras/data/odt/", "tdf108022.odt");
    reload("Office Open XML Text");

    // assert we really have two charts
    Reference<chart2::XChartDocument> xChartDoc1(getChartDocFromWriter(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc1.is());
    Reference<chart2::XChartDocument> xChartDoc2(getChartDocFromWriter(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc2.is());
}

void Chart2ExportTest2::testTdf121744()
{
    load(u"/chart2/qa/extras/data/docx/", "tdf121744.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart", "Office Open XML Text");
    CPPUNIT_ASSERT(pXmlDoc);

    OUString XValueId
        = getXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:axId[1]", "val");
    OUString YValueId
        = getXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:axId[2]", "val");

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:axId[1]", "val", XValueId);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:axId[2]", "val", YValueId);
}

void Chart2ExportTest2::testTdf121189()
{
    load(u"/chart2/qa/extras/data/odp/", "tdf121189.odp");
    reload("Impress Office Open XML");

    uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(0),
                                             uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xPage->getCount());
    uno::Reference<drawing::XShape> xShape(xPage->getByIndex(0), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("com.sun.star.drawing.OLE2Shape"), xShape->getShapeType());
}

void Chart2ExportTest2::testTdf122031()
{
    //Checks pie chart data label format.

    load(u"/chart2/qa/extras/data/xlsx/", "tdf122031.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:pieChart/c:ser/c:dLbls/c:numFmt",
                "formatCode", "0.000%");
    assertXPath(pXmlDoc,
                "/c:chartSpace/c:chart/c:plotArea/c:pieChart/c:ser/c:dLbls/c:dLbl[1]/c:numFmt",
                "formatCode", "0.000%");
    assertXPath(pXmlDoc,
                "/c:chartSpace/c:chart/c:plotArea/c:pieChart/c:ser/c:dLbls/c:dLbl[2]/c:numFmt",
                "formatCode", "0.000%");
    assertXPath(pXmlDoc,
                "/c:chartSpace/c:chart/c:plotArea/c:pieChart/c:ser/c:dLbls/c:dLbl[3]/c:numFmt",
                "formatCode", "0.000%");
}

void Chart2ExportTest2::testTdf115012()
{
    load(u"/chart2/qa/extras/data/xlsx/", "tdf115012.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);
    // workaround: use-zero instead of leave-gap to show the original line chart
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:dispBlanksAs", "val", "zero");
}

void Chart2ExportTest2::testTdf134118()
{
    load(u"/chart2/qa/extras/data/xlsx/", "tdf134118.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);
    // workaround: use leave-gap instead of zero to show the original line chart
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:dispBlanksAs", "val", "gap");
}

void Chart2ExportTest2::testTdf123206_customLabelText()
{
    load(u"/chart2/qa/extras/data/docx/", "tdf123206.docx");

    Reference<chart2::XChartDocument> xChartDoc(getChartDocFromWriter(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());

    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart", "Office Open XML Text");
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPathContent(pXmlDoc,
                       "/c:chartSpace/c:chart/c:plotArea/c:pieChart/c:ser/c:dLbls/c:dLbl[2]/c:tx/"
                       "c:rich/a:p/a:r/a:t",
                       "kiscica");
}

void Chart2ExportTest2::testCustomLabelText()
{
    load(u"/chart2/qa/extras/data/docx/", "testCustomlabeltext.docx");

    Reference<chart2::XChartDocument> xChartDoc(getChartDocFromWriter(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());

    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart", "Office Open XML Text");
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc,
                "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dLbls/c:dLbl[1]/c:idx", "val",
                "2");
    assertXPathContent(pXmlDoc,
                       "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dLbls/c:dLbl[1]/c:tx/"
                       "c:rich/a:p/a:r[1]/a:t",
                       "3.5");
    assertXPathContent(pXmlDoc,
                       "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dLbls/c:dLbl[1]/c:tx/"
                       "c:rich/a:p/a:r[3]/a:t",
                       "CustomLabel 1");

    assertXPath(pXmlDoc,
                "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dLbls/c:dLbl[2]/c:idx", "val",
                "3");
    assertXPathContent(pXmlDoc,
                       "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dLbls/c:dLbl[2]/c:tx/"
                       "c:rich/a:p/a:r[1]/a:t",
                       "4.5");
    assertXPathContent(pXmlDoc,
                       "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dLbls/c:dLbl[2]/c:tx/"
                       "c:rich/a:p/a:r[3]/a:t",
                       "CustomLabel 2");
}

void Chart2ExportTest2::testDeletedLegendEntries()
{
    load(u"/chart2/qa/extras/data/xlsx/", "deleted_legend_entry.xlsx");
    {
        reload("Calc Office Open XML");
        Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
        CPPUNIT_ASSERT(xChartDoc.is());
        Reference<chart2::XDataSeries> xDataSeries(getDataSeriesFromDoc(xChartDoc, 1));
        CPPUNIT_ASSERT(xDataSeries.is());
        Reference<beans::XPropertySet> xPropertySet(xDataSeries, uno::UNO_QUERY_THROW);
        bool bShowLegendEntry = true;
        CPPUNIT_ASSERT(xPropertySet->getPropertyValue("ShowLegendEntry") >>= bShowLegendEntry);
        CPPUNIT_ASSERT(!bShowLegendEntry);
    }

    load(u"/chart2/qa/extras/data/xlsx/", "deleted_legend_entry2.xlsx");
    {
        reload("Calc Office Open XML");
        Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
        CPPUNIT_ASSERT(xChartDoc.is());
        Reference<chart2::XDataSeries> xDataSeries(getDataSeriesFromDoc(xChartDoc, 0));
        CPPUNIT_ASSERT(xDataSeries.is());
        Reference<beans::XPropertySet> xPropertySet(xDataSeries, uno::UNO_QUERY_THROW);
        bool bShowLegendEntry = true;
        CPPUNIT_ASSERT(xPropertySet->getPropertyValue("ShowLegendEntry") >>= bShowLegendEntry);
        CPPUNIT_ASSERT(!bShowLegendEntry);

        Reference<chart2::XChartDocument> xChartDoc2 = getChartDocFromSheet(1, mxComponent);
        CPPUNIT_ASSERT(xChartDoc.is());
        Reference<chart2::XDataSeries> xDataSeries2(getDataSeriesFromDoc(xChartDoc2, 0));
        CPPUNIT_ASSERT(xDataSeries2.is());
        Reference<beans::XPropertySet> xPropertySet2(xDataSeries2, uno::UNO_QUERY_THROW);
        Sequence<sal_Int32> deletedLegendEntriesSeq;
        CPPUNIT_ASSERT(xPropertySet2->getPropertyValue("DeletedLegendEntries")
                       >>= deletedLegendEntriesSeq);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), deletedLegendEntriesSeq[0]);
    }
}

void Chart2ExportTest2::testTdf60316()
{
    load(u"/chart2/qa/extras/data/pptx/", "tdf60316.pptx");
    xmlDocUniquePtr pXmlDoc = parseExport("ppt/charts/chart", "Impress MS PowerPoint 2007 XML");
    CPPUNIT_ASSERT(pXmlDoc);

    // Without the fix in place, the shape would have had a solidFill background
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:spPr/a:noFill", 1);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:spPr/a:solidFill", 0);
}

void Chart2ExportTest2::testTdf130225()
{
    load(u"/chart2/qa/extras/data/docx/", "piechart_deleted_legend_entry.docx");
    reload("Office Open XML Text");
    Reference<chart2::XChartDocument> xChartDoc(getChartDocFromWriter(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());
    Reference<chart2::XDataSeries> xDataSeries(getDataSeriesFromDoc(xChartDoc, 0));
    CPPUNIT_ASSERT(xDataSeries.is());
    Reference<beans::XPropertySet> xPropertySet(xDataSeries, uno::UNO_QUERY_THROW);
    Sequence<sal_Int32> deletedLegendEntriesSeq;
    CPPUNIT_ASSERT(xPropertySet->getPropertyValue("DeletedLegendEntries")
                   >>= deletedLegendEntriesSeq);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), deletedLegendEntriesSeq[0]);
}

void Chart2ExportTest2::testTdf59857()
{
    load(u"/chart2/qa/extras/data/ods/", "tdf59857.ods");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:floor/c:spPr/a:ln/a:noFill", 1);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:floor/c:spPr/a:solidFill/a:srgbClr", "val",
                "cccccc");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:backWall/c:spPr/a:ln/a:noFill", 0);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:backWall/c:spPr/a:ln/a:solidFill/a:srgbClr",
                "val", "b3b3b3");
}

void Chart2ExportTest2::testTdf126076()
{
    load(u"/chart2/qa/extras/data/xlsx/", "auto_marker_excel10.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);

    // This was 12: all series exported with square markers
    assertXPath(
        pXmlDoc,
        "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:ser/c:marker/c:symbol[@val='square']", 0);
    // instead of skipping markers
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:ser/c:marker", 0);
}

void Chart2ExportTest2::testTdf75330()
{
    load(u"/chart2/qa/extras/data/ods/", "legend_overlay.ods");
    reload("calc8");
    {
        uno::Reference<chart2::XChartDocument> xChart2Doc = getChartDocFromSheet(0, mxComponent);
        uno::Reference<chart::XChartDocument> xChartDoc(xChart2Doc, uno::UNO_QUERY);
        uno::Reference<drawing::XShape> xLegend = xChartDoc->getLegend();
        Reference<beans::XPropertySet> xPropertySet(xLegend, uno::UNO_QUERY_THROW);
        bool bOverlay = false;
        CPPUNIT_ASSERT(xPropertySet->getPropertyValue("Overlay") >>= bOverlay);
        CPPUNIT_ASSERT(bOverlay);
    }
    reload("Calc Office Open XML");
    {
        uno::Reference<chart2::XChartDocument> xChart2Doc = getChartDocFromSheet(0, mxComponent);
        uno::Reference<chart::XChartDocument> xChartDoc(xChart2Doc, uno::UNO_QUERY);
        uno::Reference<drawing::XShape> xLegend = xChartDoc->getLegend();
        Reference<beans::XPropertySet> xPropertySet(xLegend, uno::UNO_QUERY_THROW);
        bool bOverlay = false;
        CPPUNIT_ASSERT(xPropertySet->getPropertyValue("Overlay") >>= bOverlay);
        CPPUNIT_ASSERT(bOverlay);
    }
}

void Chart2ExportTest2::testTdf127792()
{
    load(u"/chart2/qa/extras/data/docx/", "MSO_axis_position.docx");
    {
        xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart1", "Office Open XML Text");
        CPPUNIT_ASSERT(pXmlDoc);
        assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx/c:crossBetween", "val",
                    "between");
    }
    {
        xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart2", "Office Open XML Text");
        CPPUNIT_ASSERT(pXmlDoc);
        assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx/c:crossBetween", "val",
                    "midCat");
    }
}

void Chart2ExportTest2::testTdf131979()
{
    load(u"/chart2/qa/extras/data/ods/", "tdf131115.ods");
    {
        reload("calc8");
        Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
        CPPUNIT_ASSERT(xChartDoc.is());
        Reference<chart2::XDataSeries> xDataSeries(getDataSeriesFromDoc(xChartDoc, 0));
        CPPUNIT_ASSERT(xDataSeries.is());
        Reference<beans::XPropertySet> xPropertySet;
        xPropertySet.set(xDataSeries->getDataPointByIndex(2), uno::UNO_SET_THROW);
        bool blinknumberformattosource = true;
        CPPUNIT_ASSERT(xPropertySet->getPropertyValue(CHART_UNONAME_LINK_TO_SRC_NUMFMT)
                       >>= blinknumberformattosource);
        CPPUNIT_ASSERT_MESSAGE("\"LinkNumberFormatToSource\" should be set to false.",
                               !blinknumberformattosource);
    }

    load(u"/chart2/qa/extras/data/ods/", "tdf131979.ods");
    {
        reload("calc8");
        Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
        CPPUNIT_ASSERT(xChartDoc.is());
        Reference<chart2::XDataSeries> xDataSeries(getDataSeriesFromDoc(xChartDoc, 0));
        CPPUNIT_ASSERT(xDataSeries.is());
        Reference<beans::XPropertySet> xPropertySet;
        xPropertySet.set(xDataSeries->getDataPointByIndex(2), uno::UNO_SET_THROW);
        bool blinknumberformattosource = true;
        CPPUNIT_ASSERT(xPropertySet->getPropertyValue(CHART_UNONAME_LINK_TO_SRC_NUMFMT)
                       >>= blinknumberformattosource);
        CPPUNIT_ASSERT_MESSAGE("\"LinkNumberFormatToSource\" should be set to true.",
                               blinknumberformattosource);
    }
}

void Chart2ExportTest2::testTdf132076()
{
    {
        load(u"/chart2/qa/extras/data/ods/", "tdf132076.ods");
        xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
        CPPUNIT_ASSERT(pXmlDoc);
        assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:catAx/c:numFmt", "formatCode",
                    "dd");
        assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:catAx/c:numFmt", "sourceLinked",
                    "0");
    }
    {
        load(u"/chart2/qa/extras/data/xlsx/", "tdf132076.xlsx");
        xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
        CPPUNIT_ASSERT(pXmlDoc);
        assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:dateAx/c:numFmt", "formatCode",
                    "dd");
        assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:dateAx/c:numFmt", "sourceLinked",
                    "0");
    }
}

void Chart2ExportTest2::testTdf125812()
{
    load(u"/chart2/qa/extras/data/odp/", "ellipticalGradientFill.odp");
    xmlDocUniquePtr pXmlDoc = parseExport("ppt/charts/chart", "Impress MS PowerPoint 2007 XML");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:spPr/a:gradFill/a:path", "path",
                "circle");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:spPr/a:gradFill/a:path/a:fillToRect",
                "l", "50000");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:spPr/a:gradFill/a:path/a:fillToRect",
                "t", "49000");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:spPr/a:gradFill/a:path/a:fillToRect",
                "r", "50000");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:spPr/a:gradFill/a:path/a:fillToRect",
                "b", "51000");
}

void Chart2ExportTest2::testTdf133190()
{
    load(u"/chart2/qa/extras/data/xlsx/", "tdf133190_tdf133191.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);

    // Test word wrap of data point label
    assertXPath(
        pXmlDoc,
        "/c:chartSpace/c:chart/c:plotArea/c:pieChart/c:ser/c:dLbls/c:dLbl[1]/c:txPr/a:bodyPr",
        "wrap", "none");
    assertXPath(
        pXmlDoc,
        "/c:chartSpace/c:chart/c:plotArea/c:pieChart/c:ser/c:dLbls/c:dLbl[2]/c:txPr/a:bodyPr",
        "wrap", "square");
}

void Chart2ExportTest2::testTdf133191()
{
    load(u"/chart2/qa/extras/data/xlsx/", "tdf133190_tdf133191.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);

    // Test rotation of data point label
    assertXPath(
        pXmlDoc,
        "/c:chartSpace/c:chart/c:plotArea/c:pieChart/c:ser/c:dLbls/c:dLbl[3]/c:txPr/a:bodyPr",
        "rot", "-4500000");
}

void Chart2ExportTest2::testTdf132594()
{
    load(u"/chart2/qa/extras/data/xlsx/", "chart_pie2007.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:pieChart/c:ser/c:cat", 1);
}

void Chart2ExportTest2::testTdf134255()
{
    load(u"/chart2/qa/extras/data/docx/", "tdf134255.docx");
    Reference<chart2::XChartDocument> xChartDoc(getChartDocFromWriter(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());

    // import test
    Reference<chart2::XDataSeries> xDataSeries = getDataSeriesFromDoc(xChartDoc, 0);
    CPPUNIT_ASSERT(xDataSeries.is());
    Reference<beans::XPropertySet> xPropSet(xDataSeries, UNO_QUERY_THROW);
    bool bWrap = false;
    CPPUNIT_ASSERT((xPropSet->getPropertyValue("TextWordWrap") >>= bWrap));
    CPPUNIT_ASSERT(bWrap);

    // export test
    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart", "Office Open XML Text");
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc,
                "/c:chartSpace/c:chart/c:plotArea/c:pieChart/c:ser/c:dLbls/c:txPr/a:bodyPr", "wrap",
                "square");
}

void Chart2ExportTest2::testTdf134977()
{
    load(u"/chart2/qa/extras/data/xlsx/", "custom_data_label.xlsx");

    //import test
    uno::Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT(xChartDoc.is());
    Reference<chart2::XDataSeries> xDataSeries = getDataSeriesFromDoc(xChartDoc, 0);
    CPPUNIT_ASSERT(xDataSeries.is());
    uno::Reference<beans::XPropertySet> xPropertySet(xDataSeries->getDataPointByIndex(0),
                                                     uno::UNO_SET_THROW);
    uno::Sequence<uno::Reference<chart2::XDataPointCustomLabelField>> aFields;
    float nFontSize;
    xPropertySet->getPropertyValue("CustomLabelFields") >>= aFields;
    aFields[0]->getPropertyValue("CharHeight") >>= nFontSize;
    CPPUNIT_ASSERT_EQUAL(static_cast<float>(9), nFontSize);

    //export test
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc,
                "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dLbls/c:dLbl/c:tx/c:rich/a:p/"
                "a:r/a:rPr",
                "sz", "900");
}

void Chart2ExportTest2::testTdf123647()
{
    load(u"/chart2/qa/extras/data/xlsx/", "empty_chart.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart", 1);
}

void Chart2ExportTest2::testTdf136267()
{
    load(u"/chart2/qa/extras/data/xlsx/", "tdf136267.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPathContent(
        pXmlDoc,
        "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:cat/c:strRef/c:strCache/c:pt/c:v",
        "John");
}

void Chart2ExportTest2::testDataLabelPlacementPieChart()
{
    load(u"/chart2/qa/extras/data/xlsx/", "tdf134978.xlsx");
    reload("calc8");
    uno::Reference<chart::XChartDocument> xChartDoc(getChartCompFromSheet(0, mxComponent),
                                                    UNO_QUERY_THROW);
    // test the placement of the manually positioned label
    Reference<beans::XPropertySet> xDataPointPropSet(
        xChartDoc->getDiagram()->getDataPointProperties(2, 0), uno::UNO_SET_THROW);
    uno::Any aAny = xDataPointPropSet->getPropertyValue("LabelPlacement");
    CPPUNIT_ASSERT(aAny.hasValue());
    sal_Int32 nLabelPlacement = 0;
    CPPUNIT_ASSERT(aAny >>= nLabelPlacement);
    CPPUNIT_ASSERT_EQUAL(chart::DataLabelPlacement::OUTSIDE, nLabelPlacement);
}

void Chart2ExportTest2::testTdf137917()
{
    load(u"/chart2/qa/extras/data/xlsx/", "tdf137917.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:dateAx/c:baseTimeUnit", "val", "days");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:dateAx/c:majorUnit", "val", "1");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:dateAx/c:majorTimeUnit", "val",
                "months");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:dateAx/c:minorUnit", "val", "7");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:dateAx/c:minorTimeUnit", "val",
                "days");
}

void Chart2ExportTest2::testTdf138204()
{
    load(u"/chart2/qa/extras/data/xlsx/", "tdf138204.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);

    // Check the first data label field type
    assertXPath(
        pXmlDoc,
        "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[1]/c:dLbls/c:dLbl/c:tx/c:rich/a:p/a:fld",
        "type", "CELLRANGE");

    Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT(xChartDoc.is());

    uno::Reference<chart2::XDataSeries> xDataSeries(getDataSeriesFromDoc(xChartDoc, 1));
    CPPUNIT_ASSERT(xDataSeries.is());

    uno::Reference<beans::XPropertySet> xPropertySet;
    uno::Sequence<uno::Reference<chart2::XDataPointCustomLabelField>> aFields;
    xPropertySet.set(xDataSeries->getDataPointByIndex(0), uno::UNO_SET_THROW);
    xPropertySet->getPropertyValue("CustomLabelFields") >>= aFields;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), aFields.getLength());

    CPPUNIT_ASSERT_EQUAL(
        chart2::DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_CELLRANGE,
        aFields[0]->getFieldType());
    //CPPUNIT_ASSERT_EQUAL(OUString("67.5%"), aFields[0]->getString()); TODO: Not implemented yet
}

void Chart2ExportTest2::testTdf138181()
{
    load(u"/chart2/qa/extras/data/xlsx/", "piechart_deleted_legendentry.xlsx");
    Reference<chart::XChartDocument> xChartDoc(getChartDocFromSheet(0, mxComponent),
                                               UNO_QUERY_THROW);
    Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xChartDoc, UNO_QUERY_THROW);
    Reference<drawing::XDrawPage> xDrawPage(xDrawPageSupplier->getDrawPage(), UNO_SET_THROW);
    Reference<drawing::XShapes> xShapes(xDrawPage->getByIndex(0), UNO_QUERY_THROW);
    Reference<drawing::XShape> xLegendEntry1, xLegendEntry2, xLegendEntry3;

    // first legend entry is visible
    xLegendEntry1
        = getShapeByName(xShapes, "CID/MultiClick/D=0:CS=0:CT=0:Series=0:Point=0:LegendEntry=0");
    CPPUNIT_ASSERT(xLegendEntry1.is());

    // second legend entry is not visible
    xLegendEntry2
        = getShapeByName(xShapes, "CID/MultiClick/D=0:CS=0:CT=0:Series=0:Point=1:LegendEntry=0");
    CPPUNIT_ASSERT(!xLegendEntry2.is());

    // third legend entry is visible
    xLegendEntry3
        = getShapeByName(xShapes, "CID/MultiClick/D=0:CS=0:CT=0:Series=0:Point=2:LegendEntry=0");
    CPPUNIT_ASSERT(xLegendEntry3.is());
}

void Chart2ExportTest2::testCustomShapeText()
{
    load(u"/chart2/qa/extras/data/ods/", "tdf72776.ods");
    reload("calc8");
    Reference<chart::XChartDocument> xChartDoc(getChartDocFromSheet(0, mxComponent),
                                               UNO_QUERY_THROW);
    // test that the text of custom shape exists inside the chart
    Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xChartDoc, UNO_QUERY_THROW);
    Reference<drawing::XDrawPage> xDrawPage(xDrawPageSupplier->getDrawPage(), UNO_SET_THROW);
    Reference<drawing::XShape> xCustomShape(xDrawPage->getByIndex(1), UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xCustomShape.is());

    Reference<text::XText> xRange(xCustomShape, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(!xRange->getString().isEmpty());
}

void Chart2ExportTest2::testuserShapesXLSX()
{
    load(u"/chart2/qa/extras/data/xlsx/", "tdf128621.xlsx");
    reload("Calc Office Open XML");

    Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT(xChartDoc.is());

    // test that the custom shape exists
    Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xChartDoc, UNO_QUERY_THROW);
    Reference<drawing::XDrawPage> xDrawPage(xDrawPageSupplier->getDrawPage(), UNO_SET_THROW);
    Reference<drawing::XShape> xCustomShape(xDrawPage->getByIndex(1), UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xCustomShape.is());
    // test type of shape
    CPPUNIT_ASSERT(xCustomShape->getShapeType().endsWith("CustomShape"));
    // test custom shape position
    awt::Point aPosition = xCustomShape->getPosition();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1356, aPosition.X, 300);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(9107, aPosition.Y, 300);
    // test custom shape size
    awt::Size aSize = xCustomShape->getSize();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(9520, aSize.Width, 300);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1805, aSize.Height, 300);
    // test custom shape text
    Reference<text::XText> xRange(xCustomShape, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(!xRange->getString().isEmpty());
}

void Chart2ExportTest2::testNameRangeXLSX()
{
    load(u"/chart2/qa/extras/data/xlsx/", "chart_with_name_range.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);
    // test the syntax of local range name on the the local sheet.
    assertXPathContent(pXmlDoc,
                       "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:cat/c:strRef/c:f",
                       "Sheet1!local_name_range");
    // test the syntax of a global range name.
    assertXPathContent(pXmlDoc,
                       "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:val/c:numRef/c:f",
                       "[0]!series1");
}

CPPUNIT_TEST_SUITE_REGISTRATION(Chart2ExportTest2);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
