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
#include <com/sun/star/chart2/MovingAverageType.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/chart2/DataPointLabel.hpp>
#include <com/sun/star/chart/DataLabelPlacement.hpp>

using uno::Reference;
using beans::XPropertySet;

class Chart2ExportTest3 : public ChartTest
{
protected:

    virtual void registerNamespaces(xmlXPathContextPtr& pXmlXPathCtx) override;
public:
    Chart2ExportTest3() : ChartTest("/chart2/qa/extras/data/") {}

    void testTdf108107();
    void testTdf114139();
    void testTdf64224();
    void testChartTitlePropertiesColorFillDOCX();
    void testChartTitlePropertiesGradientFillDOCX();
    void testChartTitlePropertiesBitmapFillDOCX();
    void testColorGradientWithTransparencyDOCX();
    void testColorGradientWithTransparencyODS();
    void testColorGradientStopXLSX();
    void testRadialColorGradientDOCX();
    void testBarChartDataPointPropDOCX();
    void testFdo83058dlblPos();
    void testAutoTitleDelXLSX();
    void testDispBlanksAsXLSX();
    void testMarkerColorXLSX();
    void testRoundedCornersXLSX();
    void testAxisNumberFormatXLSX();
    void testDataPointLabelNumberFormatXLSX();
    void testDataLabelDefaultValuesXLSX();
    void testDataLabelFillColor();
    void testTitleOverlayXLSX();
    void testInvertIfNegativeXLSX();
    void testBubble3DXLSX();
    void testNoMarkerXLSX();
    void testTitleManualLayoutXLSX();
    void testPlotAreaManualLayoutXLSX();
    void testLegendManualLayoutXLSX();
    void testChartSubTitle();
    void testChartMainWithSubTitle();
    void testAutoTitleDeleted();
    void testChartTitlePropertiesColorFillXLSX();
    void testChartTitlePropertiesGradientFillXLSX();
    void testChartTitlePropertiesBitmapFillXLSX();
    void testBarChartDataPointPropXLSX();
    void testDataseriesOverlapStackedChartXLSX();
    void testAxisCharacterPropertiesXLSX();
    void testTitleCharacterPropertiesXLSX();
    void testPlotVisOnlyXLSX();
    void testBarChartVaryColorsXLSX();
    void testTdf96161();
    void testTableOnPage3();
    void testMultipleAxisXLSX();
    void testSecondaryAxisXLSX();
    void testBarChartSecondaryAxisXLSX();

    CPPUNIT_TEST_SUITE(Chart2ExportTest3);
    CPPUNIT_TEST(testTdf108107);
    CPPUNIT_TEST(testTdf114139);
    CPPUNIT_TEST(testTdf64224);
    CPPUNIT_TEST(testChartTitlePropertiesColorFillDOCX);
    CPPUNIT_TEST(testChartTitlePropertiesGradientFillDOCX);
    CPPUNIT_TEST(testChartTitlePropertiesBitmapFillDOCX);
    CPPUNIT_TEST(testColorGradientWithTransparencyDOCX);
    CPPUNIT_TEST(testColorGradientWithTransparencyODS);
    CPPUNIT_TEST(testColorGradientStopXLSX);
    CPPUNIT_TEST(testRadialColorGradientDOCX);
    CPPUNIT_TEST(testBarChartDataPointPropDOCX);
    CPPUNIT_TEST(testFdo83058dlblPos);
    CPPUNIT_TEST(testAutoTitleDelXLSX);
    CPPUNIT_TEST(testDispBlanksAsXLSX);
    CPPUNIT_TEST(testMarkerColorXLSX);
    CPPUNIT_TEST(testRoundedCornersXLSX);
    CPPUNIT_TEST(testAxisNumberFormatXLSX);
    CPPUNIT_TEST(testDataPointLabelNumberFormatXLSX);
    CPPUNIT_TEST(testDataLabelDefaultValuesXLSX);
    CPPUNIT_TEST(testDataLabelFillColor);
    CPPUNIT_TEST(testTitleOverlayXLSX);
    CPPUNIT_TEST(testInvertIfNegativeXLSX);
    CPPUNIT_TEST(testBubble3DXLSX);
    CPPUNIT_TEST(testNoMarkerXLSX);
    CPPUNIT_TEST(testTitleManualLayoutXLSX);
    CPPUNIT_TEST(testPlotAreaManualLayoutXLSX);
    CPPUNIT_TEST(testLegendManualLayoutXLSX);
    CPPUNIT_TEST(testChartSubTitle);
    CPPUNIT_TEST(testChartMainWithSubTitle);
    CPPUNIT_TEST(testAutoTitleDeleted);
    CPPUNIT_TEST(testChartTitlePropertiesColorFillXLSX);
    CPPUNIT_TEST(testChartTitlePropertiesGradientFillXLSX);
    CPPUNIT_TEST(testChartTitlePropertiesBitmapFillXLSX);
    CPPUNIT_TEST(testBarChartDataPointPropXLSX);
    CPPUNIT_TEST(testDataseriesOverlapStackedChartXLSX);
    CPPUNIT_TEST(testAxisCharacterPropertiesXLSX);
    CPPUNIT_TEST(testTitleCharacterPropertiesXLSX);
    CPPUNIT_TEST(testPlotVisOnlyXLSX);
    CPPUNIT_TEST(testBarChartVaryColorsXLSX);
    CPPUNIT_TEST(testTdf96161);
    CPPUNIT_TEST(testTableOnPage3);
    CPPUNIT_TEST(testMultipleAxisXLSX);
    CPPUNIT_TEST(testSecondaryAxisXLSX);
    CPPUNIT_TEST(testBarChartSecondaryAxisXLSX);
    CPPUNIT_TEST_SUITE_END();
};

void Chart2ExportTest3::registerNamespaces(xmlXPathContextPtr& pXmlXPathCtx)
{
    XmlTestTools::registerOOXMLNamespaces(pXmlXPathCtx);
}

void Chart2ExportTest3::testTdf108107()
{
    loadFromURL(u"xlsx/tdf108107.xlsx");

    // FIXME: validation error in OOXML export: Errors: 1
    skipValidation();

    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:ser/c:dLbls/c:dLbl[1]/c:idx", "val", "1");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:ser/c:dLbls/c:dLbl[1]/c:txPr/a:p/a:pPr/a:defRPr", "b", "1");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:ser/c:dLbls/c:dLbl[1]/c:txPr/a:p/a:pPr/a:defRPr", "sz", "2000");
}

void Chart2ExportTest3::testTdf114139()
{
    loadFromURL(u"xlsx/tdf114139.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:pie3DChart", 1);
    //no fill
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:spPr", 0);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:spPr/a:solidFill", 0);
}

void Chart2ExportTest3::testTdf64224()
{
    loadFromURL(u"ods/tdf64224.ods");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);

    //no fill
    assertXPath(pXmlDoc, "/c:chartSpace/c:spPr/a:noFill", 1);
    assertXPath(pXmlDoc, "/c:chartSpace/c:spPr/a:solidFill", 0);
}

void Chart2ExportTest3::testChartTitlePropertiesColorFillDOCX()
{
    loadFromURL(u"docx/testChartTitlePropertiesColorFill.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart","Office Open XML Text");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:solidFill/a:srgbClr", "val", "ff0000");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:ln/a:noFill", 1);
}

void Chart2ExportTest3::testChartTitlePropertiesGradientFillDOCX()
{
    loadFromURL(u"docx/testChartTitlePropertiesGradientFill.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart","Office Open XML Text");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:gradFill/a:gsLst/a:gs[1]/a:srgbClr", "val", "cccccc");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:gradFill/a:gsLst/a:gs[2]/a:srgbClr", "val", "666666");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:ln/a:noFill", 1);
}

void Chart2ExportTest3::testChartTitlePropertiesBitmapFillDOCX()
{
    loadFromURL(u"docx/testChartTitlePropertiesBitmapFill.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart","Office Open XML Text");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:blipFill/a:blip", "embed", "rId1");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:ln/a:noFill", 1);
}


void Chart2ExportTest3::testColorGradientWithTransparencyDOCX()
{
    // Test color gradient (two color) with gradient transparency
    loadFromURL(u"docx/testColorGradientWithTransparency.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart", "Office Open XML Text");
    CPPUNIT_ASSERT(pXmlDoc);
    // Test the transparency of the first color
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:spPr/a:gradFill/a:gsLst/a:gs[1]/a:srgbClr/a:alpha", "val", "60000");
    // Test the transparency of the second color
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:spPr/a:gradFill/a:gsLst/a:gs[2]/a:srgbClr/a:alpha", "val", "90196");
}

void Chart2ExportTest3::testColorGradientWithTransparencyODS()
{
    // Test color gradient (two color) with simple transparency
    loadFromURL(u"ods/testColorGradientWithTransparency.ods");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);
    // Test the transparency of the first color
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:spPr/a:gradFill/a:gsLst/a:gs[1]/a:srgbClr/a:alpha", "val", "60000");
    // Test the transparency of the second color
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:spPr/a:gradFill/a:gsLst/a:gs[2]/a:srgbClr/a:alpha", "val", "60000");
}

void Chart2ExportTest3::testColorGradientStopXLSX()
{
    // Test color gradient (two color) stop of the first color
    loadFromURL(u"xlsx/tdf128619.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);
    // Test the position of the first color
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:spPr/a:gradFill/a:gsLst/a:gs[1]", "pos", "45000");
    // Test the position of the second color
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:spPr/a:gradFill/a:gsLst/a:gs[2]", "pos", "100000");
}

void Chart2ExportTest3::testRadialColorGradientDOCX()
{
    loadFromURL(u"docx/tdf128794.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart", "Office Open XML Text");
    CPPUNIT_ASSERT(pXmlDoc);
    // Test the gradient style (if there is no 'a:path' attribute, it is a linear gradient)
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:spPr/a:gradFill/a:path", 0);
    // Test the linear gradient angle
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:spPr/a:gradFill/a:lin", "ang", "13500000");
}

void Chart2ExportTest3::testBarChartDataPointPropDOCX()
{
    loadFromURL(u"docx/testBarChartDataPointPropDOCX.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart", "Office Open XML Text");
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:varyColors", "val", "0");

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dPt[1]/c:idx", "val", "1");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dPt[1]/c:spPr/a:gradFill/a:gsLst/a:gs[1]/a:srgbClr", "val", "f6f8fc");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dPt[1]/c:spPr/a:gradFill/a:gsLst/a:gs[2]/a:srgbClr", "val", "c7d5ed");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dPt[1]/c:spPr/a:ln/a:solidFill/a:srgbClr", "val", "70ad47");

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dPt[2]/c:idx", "val", "2");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dPt[2]/c:spPr/a:solidFill/a:srgbClr", "val", "ff0000");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dPt[2]/c:spPr/a:ln/a:solidFill/a:srgbClr", "val", "000000");
}

void Chart2ExportTest3::testFdo83058dlblPos()
{
    loadFromURL(u"docx/fdo83058_dlblPos.docx");

    // FIXME: validation error in OOXML export: Errors: 1
    skipValidation();

    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart","Office Open XML Text");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[1]/c:dLbls[1]/c:dLbl[2]/c:dLblPos", "val", "outEnd");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[1]/c:dLbls[1]/c:dLbl[3]/c:dLblPos", "val", "outEnd");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[1]/c:dLbls[1]/c:dLbl[4]/c:dLblPos", "val", "outEnd");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[1]/c:dLbls[1]/c:dLbl[5]/c:dLblPos", "val", "outEnd");
}

void Chart2ExportTest3::testAutoTitleDelXLSX()
{
    loadFromURL(u"xlsx/autotitledel_2007.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart","Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:autoTitleDeleted", "val", "0");
}

void Chart2ExportTest3::testDispBlanksAsXLSX()
{
    loadFromURL(u"xlsx/dispBlanksAs_2007.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart","Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:dispBlanksAs", "val", "gap");
}

void Chart2ExportTest3::testMarkerColorXLSX()
{
    loadFromURL(u"xlsx/markerColor.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:scatterChart/c:ser/c:marker/c:spPr/a:solidFill/a:srgbClr", "val", "92d050");
}

void Chart2ExportTest3::testRoundedCornersXLSX()
{
    loadFromURL(u"xlsx/markerColor.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:roundedCorners", "val", "0");
}

void Chart2ExportTest3::testAxisNumberFormatXLSX()
{
    loadFromURL(u"ods/axis_number_format.ods");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx", 2);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx[1]/c:numFmt", "formatCode", "0.00E+000");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx[1]/c:numFmt", "sourceLinked", "0");

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx[2]/c:numFmt", "formatCode", "[$$-409]#,##0;\\-[$$-409]#,##0");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx[2]/c:numFmt", "sourceLinked", "1");
}

void Chart2ExportTest3::testDataPointLabelNumberFormatXLSX()
{
    // FIXME: validation error in OOXML export: Errors: 1
    skipValidation();

    loadFromURL(u"ods/tdf123774.ods");
    {
        xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
        CPPUNIT_ASSERT(pXmlDoc);
        assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:pieChart/c:ser/c:dLbls/c:numFmt", "formatCode", "[$-40E]0.00%");
        assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:pieChart/c:ser/c:dLbls/c:numFmt", "sourceLinked", "0");

        assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:pieChart/c:ser/c:dLbls/c:dLbl[1]/c:numFmt", "formatCode", "[$-40E]0.00%");
        assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:pieChart/c:ser/c:dLbls/c:dLbl[1]/c:numFmt", "sourceLinked", "0");
    }

    loadFromURL(u"xlsx/tdf130986.xlsx");
    {
        xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
        CPPUNIT_ASSERT(pXmlDoc);
        assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dLbls/c:dLbl/c:idx", "val", "1");
        assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dLbls/c:dLbl/c:numFmt", "formatCode", "0.00E+00");
        assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dLbls/c:dLbl/c:numFmt", "sourceLinked", "0");
    }
}

void Chart2ExportTest3::testDataLabelDefaultValuesXLSX()
{
    loadFromURL(u"xlsx/data_label.xlsx");
    Reference< chart2::XChartDocument> xDoc = getChartDocFromSheet(0, mxComponent);
    Reference<chart2::XDataSeries> xSeries = getDataSeriesFromDoc(xDoc, 0);
    Reference<beans::XPropertySet> xPropSet(xSeries, uno::UNO_QUERY_THROW);
    uno::Any aAny = xPropSet->getPropertyValue("Label");
    chart2::DataPointLabel aLabel;
    CPPUNIT_ASSERT(aAny >>= aLabel);
    CPPUNIT_ASSERT(aLabel.ShowNumber);

    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dLbls/c:showVal", "val", "1");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dLbls/c:dLblPos", "val", "outEnd");
}

void Chart2ExportTest3::testDataLabelFillColor()
{
    loadFromURL(u"xlsx/data_labels_fill_color.xlsx");
    Reference< chart2::XChartDocument> xDoc = getChartDocFromSheet(0, mxComponent);
    Reference<chart2::XDataSeries> xSeries = getDataSeriesFromDoc(xDoc, 0);
    Reference<beans::XPropertySet> xPropSet(xSeries, uno::UNO_QUERY_THROW);
    uno::Any aAny = xPropSet->getPropertyValue("LabelFillColor");
    sal_Int32 nLabelFillColor;
    CPPUNIT_ASSERT(aAny >>= nLabelFillColor);

    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dLbls/c:spPr/a:solidFill/a:srgbClr", "val", "F79646");
}

void Chart2ExportTest3::testTitleOverlayXLSX()
{
    loadFromURL(u"xlsx/chart_title.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:overlay", "val", "0");
}

void Chart2ExportTest3::testInvertIfNegativeXLSX()
{
    loadFromURL(u"xlsx/bar_chart_simple.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:invertIfNegative", "val", "0");
}

void Chart2ExportTest3::testBubble3DXLSX()
{
    loadFromURL(u"xlsx/bubble_chart_simple.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:bubbleChart/c:ser[1]/c:bubble3D", "val", "0");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:bubbleChart/c:ser[2]/c:bubble3D", "val", "0");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:bubbleChart/c:ser[3]/c:bubble3D", "val", "0");
}

void Chart2ExportTest3::testNoMarkerXLSX()
{
    loadFromURL(u"xlsx/no_marker.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:ser[1]/c:marker/c:symbol", "val", "none");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:ser[2]/c:marker/c:symbol", "val", "none");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:marker", "val", "0");
}

void Chart2ExportTest3::testTitleManualLayoutXLSX()
{
    loadFromURL(u"xlsx/title_manual_layout.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:layout/c:manualLayout/c:layoutTarget", 0);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:layout/c:manualLayout/c:xMode", "val", "edge");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:layout/c:manualLayout/c:yMode", "val", "edge");

    OUString aXVal = getXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:layout/c:manualLayout/c:x", "val");
    double nX = aXVal.toDouble();
    CPPUNIT_ASSERT(nX > 0);
    CPPUNIT_ASSERT(nX < 1);

    OUString aYVal = getXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:layout/c:manualLayout/c:y", "val");
    double nY = aYVal.toDouble();
    CPPUNIT_ASSERT(nY > 0);
    CPPUNIT_ASSERT(nY < 1);
    CPPUNIT_ASSERT(nX != nY);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:bodyPr", "rot", "1200000");
}

void Chart2ExportTest3::testPlotAreaManualLayoutXLSX()
{
    loadFromURL(u"xlsx/plot_area_manual_layout.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:layout/c:manualLayout/c:layoutTarget", "val", "inner");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:layout/c:manualLayout/c:xMode", "val", "edge");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:layout/c:manualLayout/c:yMode", "val", "edge");

    OUString aXVal = getXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:layout/c:manualLayout/c:x", "val");
    double nX = aXVal.toDouble();
    CPPUNIT_ASSERT(nX > 0);
    CPPUNIT_ASSERT(nX < 1);

    OUString aYVal = getXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:layout/c:manualLayout/c:y", "val");
    double nY = aYVal.toDouble();
    CPPUNIT_ASSERT(nY > 0);
    CPPUNIT_ASSERT(nY < 1);
    CPPUNIT_ASSERT(nX != nY);

    OUString aWVal = getXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:layout/c:manualLayout/c:w", "val");
    double nW = aWVal.toDouble();
    CPPUNIT_ASSERT(nW > 0);
    CPPUNIT_ASSERT(nW < 1);

    OUString aHVal = getXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:layout/c:manualLayout/c:h", "val");
    double nH = aHVal.toDouble();
    CPPUNIT_ASSERT(nH > 0);
    CPPUNIT_ASSERT(nH < 1);
    CPPUNIT_ASSERT(nH != nW);
}

void Chart2ExportTest3::testLegendManualLayoutXLSX()
{
    loadFromURL(u"xlsx/legend_manual_layout.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:layout/c:manualLayout/c:layoutTarget", 0);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:legend/c:layout/c:manualLayout/c:xMode", "val", "edge");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:legend/c:layout/c:manualLayout/c:yMode", "val", "edge");

    OUString aXVal = getXPath(pXmlDoc, "/c:chartSpace/c:chart/c:legend/c:layout/c:manualLayout/c:x", "val");
    double nX = aXVal.toDouble();
    CPPUNIT_ASSERT(nX > 0);
    CPPUNIT_ASSERT(nX < 1);

    OUString aYVal = getXPath(pXmlDoc, "/c:chartSpace/c:chart/c:legend/c:layout/c:manualLayout/c:y", "val");
    double nY = aYVal.toDouble();
    CPPUNIT_ASSERT(nY > 0);
    CPPUNIT_ASSERT(nY < 1);
    CPPUNIT_ASSERT(nX != nY);

    OUString aWVal = getXPath(pXmlDoc, "/c:chartSpace/c:chart/c:legend/c:layout/c:manualLayout/c:w", "val");
    double nW = aWVal.toDouble();
    CPPUNIT_ASSERT(nW > 0);
    CPPUNIT_ASSERT(nW < 1);

    OUString aHVal = getXPath(pXmlDoc, "/c:chartSpace/c:chart/c:legend/c:layout/c:manualLayout/c:h", "val");
    double nH = aHVal.toDouble();
    CPPUNIT_ASSERT(nH > 0);
    CPPUNIT_ASSERT(nH < 1);
    CPPUNIT_ASSERT(nH != nW);

    // Make sure that default text font size is preserved after export
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:legend/c:txPr/a:p/a:pPr/a:defRPr", "sz", "900");
}

void Chart2ExportTest3::testChartSubTitle()
{
    loadFromURL(u"ods/testChartSubTitle.ods");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);
    // test properties of subtitle
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p/a:pPr/a:defRPr", "sz", "1100");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p/a:pPr/a:defRPr", "b", "1");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p/a:pPr/a:defRPr/a:solidFill/a:srgbClr", "val", "00a933");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p/a:r/a:rPr/a:latin", "typeface", "Times New Roman");
    assertXPathContent(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p/a:r/a:t", "It is a Subtitle");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:solidFill/a:srgbClr", "val", "b2b2b2");
}

void Chart2ExportTest3::testChartMainWithSubTitle()
{
    loadFromURL(u"ods/testChartMainWithSubTitle.ods");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);
    // test properties of title
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p/a:pPr/a:defRPr", "sz", "1300");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p/a:pPr/a:defRPr", "b", "0");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p/a:pPr/a:defRPr", "i", "1");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p/a:pPr/a:defRPr/a:solidFill/a:srgbClr", "val", "f10d0c");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p/a:r/a:rPr/a:latin", "typeface", "Arial");
    assertXPathContent(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p/a:r/a:t", "It is a Maintitle\nIt is a Subtitle");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:solidFill/a:srgbClr", "val", "81d41a");
}

void Chart2ExportTest3::testAutoTitleDeleted()
{
    loadFromURL(u"xlsx/testAutoTitleDeleted.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:autoTitleDeleted", "val", "1");
}

void Chart2ExportTest3::testChartTitlePropertiesColorFillXLSX()
{
    loadFromURL(u"xlsx/testChartTitlePropertiesColorFill.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:solidFill/a:srgbClr", "val", "ff0000");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:ln/a:noFill", 1);
}

void Chart2ExportTest3::testChartTitlePropertiesGradientFillXLSX()
{
    loadFromURL(u"xlsx/testChartTitlePropertiesGradientFill.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:gradFill/a:gsLst/a:gs[1]/a:srgbClr", "val", "cccccc");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:gradFill/a:gsLst/a:gs[2]/a:srgbClr", "val", "666666");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:ln/a:noFill", 1);
}

void Chart2ExportTest3::testChartTitlePropertiesBitmapFillXLSX()
{
    loadFromURL(u"xlsx/testChartTitlePropertiesBitmapFill.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:blipFill/a:blip", "embed", "rId1");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:ln/a:noFill", 1);
}

void Chart2ExportTest3::testBarChartDataPointPropXLSX()
{
    loadFromURL(u"xlsx/testBarChartDataPointPropXLSX.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:varyColors", "val", "0");

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dPt[1]/c:idx", "val", "1");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dPt[1]/c:spPr/a:solidFill/a:srgbClr", "val", "ff0000");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dPt[1]/c:spPr/a:ln/a:solidFill/a:srgbClr", "val", "000000");

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dPt[2]/c:idx", "val", "2");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dPt[2]/c:spPr/a:gradFill/a:gsLst/a:gs[1]/a:srgbClr", "val", "f6f8fc");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dPt[2]/c:spPr/a:gradFill/a:gsLst/a:gs[2]/a:srgbClr", "val", "c7d5ed");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dPt[2]/c:spPr/a:ln/a:solidFill/a:srgbClr", "val", "70ad47");
}

namespace {

void checkGapWidth(Reference<beans::XPropertySet> const & xPropSet, sal_Int32 nValue)
{
    uno::Any aAny = xPropSet->getPropertyValue("GapwidthSequence");
    CPPUNIT_ASSERT(aAny.hasValue());
    uno::Sequence< sal_Int32 > aSequence;
    aAny >>= aSequence;
    CPPUNIT_ASSERT(aSequence.hasElements());
    CPPUNIT_ASSERT_EQUAL(nValue, aSequence[0]);
}

void checkOverlap(Reference<beans::XPropertySet> const & xPropSet, sal_Int32 nValue)
{
    uno::Any aAny = xPropSet->getPropertyValue("OverlapSequence");
    CPPUNIT_ASSERT(aAny.hasValue());
    uno::Sequence< sal_Int32 > aSequence;
    aAny >>= aSequence;
    CPPUNIT_ASSERT(aSequence.hasElements());
    CPPUNIT_ASSERT_EQUAL(nValue, aSequence[0]);
}

void checkSheetForGapWidthAndOverlap(uno::Reference< chart2::XChartDocument > const & xChartDoc,
        sal_Int32 nExpectedGapWidth, sal_Int32 nExpectedOverlap)
{
    CPPUNIT_ASSERT(xChartDoc.is());

    Reference< chart2::XChartType > xChartType = getChartTypeFromDoc( xChartDoc, 0 );
    CPPUNIT_ASSERT(xChartType.is());

    Reference< beans::XPropertySet > xPropSet( xChartType, uno::UNO_QUERY_THROW );
    checkGapWidth(xPropSet, nExpectedGapWidth);
    checkOverlap(xPropSet, nExpectedOverlap);

}

}

void Chart2ExportTest3::testDataseriesOverlapStackedChartXLSX()
{
    loadFromURL(u"xlsx/testDataseriesOverlapStackedChart.xlsx");

    // test the overlap value of a simple Stacked Column Chart
    uno::Reference< chart2::XChartDocument > xChartDoc = getChartDocFromSheet( 0, mxComponent );
    checkSheetForGapWidthAndOverlap(xChartDoc, 100, 0);

    // test the overlap value of a Percent Stacked Bar Chart
    xChartDoc = getChartDocFromSheet( 1, mxComponent );
    checkSheetForGapWidthAndOverlap(xChartDoc, 100, 35);

    saveAndReload("Calc Office Open XML");

    xChartDoc = getChartDocFromSheet( 0, mxComponent );
    checkSheetForGapWidthAndOverlap(xChartDoc, 100, 100);

    xChartDoc = getChartDocFromSheet( 1, mxComponent );
    checkSheetForGapWidthAndOverlap(xChartDoc, 100, 100);
}

void Chart2ExportTest3::testAxisCharacterPropertiesXLSX()
{
    loadFromURL(u"xlsx/axis_character_properties.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:catAx/c:txPr/a:p/a:pPr/a:defRPr", "sz", "1000");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:catAx/c:txPr/a:p/a:pPr/a:defRPr", "b", "1");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:catAx/c:txPr/a:p/a:pPr/a:defRPr", "i", "1");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:catAx/c:txPr/a:p/a:pPr/a:defRPr", "u", "sng");

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx/c:txPr/a:p/a:pPr/a:defRPr", "sz", "900");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx/c:txPr/a:p/a:pPr/a:defRPr", "b", "1");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx/c:txPr/a:p/a:pPr/a:defRPr", "strike", "sngStrike");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx/c:txPr/a:p/a:pPr/a:defRPr/a:solidFill/a:srgbClr", "val", "ff0000");
}

void Chart2ExportTest3::testTitleCharacterPropertiesXLSX()
{
    loadFromURL(u"xlsx/title_character_properties.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p/a:pPr/a:defRPr", "sz", "2400");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p/a:pPr/a:defRPr", "b", "1");

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p/a:r/a:rPr", "sz", "2400");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p/a:r/a:rPr", "b", "1");
}

void Chart2ExportTest3::testPlotVisOnlyXLSX()
{
    loadFromURL(u"xlsx/hidden_cells.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotVisOnly", "val", "0");
}

void Chart2ExportTest3::testBarChartVaryColorsXLSX()
{
    loadFromURL(u"xlsx/tdf90876.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:varyColors", "val", "0");
}

void Chart2ExportTest3::testTdf96161()
{
    loadFromURL(u"ods/tdf96161.ods");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:varyColors", "val", "0");
}

void Chart2ExportTest3::testTableOnPage3()
{
    loadFromURL(u"docx/TableOnPage3.docx");

    // FIXME: validation error in OOXML export: Errors: 2
    skipValidation();

    saveAndReload("Office Open XML Text");

    Reference<chart2::XChartDocument> xChartDoc(getChartDocFromWriter(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());
    uno::Reference< chart::XChartDataArray > xDataArray(xChartDoc->getDataProvider(), UNO_QUERY_THROW);
    Sequence<OUString> aColumnDesc = xDataArray->getColumnDescriptions();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There must be 4 columns and descriptions", static_cast<sal_Int32>(4), aColumnDesc.getLength());
    CPPUNIT_ASSERT_EQUAL(OUString("If oversubscription relative to allowance increases at the same average rate B15-B17"), aColumnDesc[0]);
    CPPUNIT_ASSERT_EQUAL(OUString("Known requirements"), aColumnDesc[1]);
    CPPUNIT_ASSERT_EQUAL(OUString("Allowance"), aColumnDesc[2]);
    CPPUNIT_ASSERT_EQUAL(OUString("If oversubscription relative to allowance holds steady at average oversubscription level B15-B17"), aColumnDesc[3]);
}

void Chart2ExportTest3::testMultipleAxisXLSX()
{
    loadFromURL(u"ods/multiple_axis.ods");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:scatterChart", 2);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:scatterChart[1]/c:ser", 1);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:scatterChart[2]/c:ser", 1);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx", 4);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx/c:delete[@val='1']", 1);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx/c:axPos[@val='l']", 1);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx/c:axPos[@val='r']", 1);
}

void Chart2ExportTest3::testSecondaryAxisXLSX()
{
    loadFromURL(u"ods/secondary_axis.ods");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart", 2);
    // test there is just those series in the first <lineChart> tag which are attached to the primary axis
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart[1]/c:ser", 2);
    assertXPathContent(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart[1]/c:ser[1]/c:tx/c:strRef/c:strCache/c:pt/c:v", "b");
    assertXPathContent(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart[1]/c:ser[2]/c:tx/c:strRef/c:strCache/c:pt/c:v", "c");
    // test there is just those series in the second <lineChart> tag which are attached to the secondary axis
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart[2]/c:ser", 1);
    assertXPathContent(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart[2]/c:ser[1]/c:tx/c:strRef/c:strCache/c:pt/c:v", "a");
}

void Chart2ExportTest3::testBarChartSecondaryAxisXLSX()
{
    loadFromURL(u"xlsx/testSecondaryAxis.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);
    // Collect barchart axID on primary Axis
    OUString XValueIdOf1Barchart = getXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart[1]/c:axId[1]", "val");
    OUString YValueIdOf1Barchart = getXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart[1]/c:axId[2]", "val");
    // Collect barchart axID on secondary Axis
    OUString XValueIdOf2Barchart = getXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart[2]/c:axId[1]", "val");
    OUString YValueIdOf2Barchart = getXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart[2]/c:axId[2]", "val");
    // Check which c:catAx and c:valAx contain the AxisId of barcharts
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:catAx[1]/c:axId", "val", XValueIdOf1Barchart);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx[1]/c:axId", "val", YValueIdOf1Barchart);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:catAx[2]/c:axId", "val", XValueIdOf2Barchart);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx[2]/c:axId", "val", YValueIdOf2Barchart);
}

CPPUNIT_TEST_SUITE_REGISTRATION(Chart2ExportTest3);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
