/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "charttest.hxx"

#include <com/sun/star/chart2/DataPointLabel.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>

using uno::Reference;
using beans::XPropertySet;

class Chart2ExportTest3 : public ChartTest
{
public:
    Chart2ExportTest3() : ChartTest(u"/chart2/qa/extras/data/"_ustr) {}
};

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testTdf108107)
{
    loadFromFile(u"xlsx/tdf108107.xlsx");

    // FIXME: validation error in OOXML export: Errors: 1
    skipValidation();

    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:ser/c:dLbls/c:dLbl[1]/c:idx", "val", u"1");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:ser/c:dLbls/c:dLbl[1]/c:txPr/a:p/a:pPr/a:defRPr", "b", u"1");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:ser/c:dLbls/c:dLbl[1]/c:txPr/a:p/a:pPr/a:defRPr", "sz", u"2000");
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testTdf114139)
{
    loadFromFile(u"xlsx/tdf114139.xlsx");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:pie3DChart", 1);
    //no fill
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:spPr", 0);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:spPr/a:solidFill", 0);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testTdf64224)
{
    loadFromFile(u"ods/tdf64224.ods");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    //no fill
    assertXPath(pXmlDoc, "/c:chartSpace/c:spPr/a:noFill", 1);
    assertXPath(pXmlDoc, "/c:chartSpace/c:spPr/a:solidFill", 0);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testChartTitlePropertiesColorFillDOCX)
{
    loadFromFile(u"docx/testChartTitlePropertiesColorFill.docx");
    save(u"Office Open XML Text"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:solidFill/a:srgbClr", "val", u"ff0000");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:ln/a:noFill", 1);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testChartTitlePropertiesGradientFillDOCX)
{
    loadFromFile(u"docx/testChartTitlePropertiesGradientFill.docx");
    save(u"Office Open XML Text"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:gradFill/a:gsLst/a:gs[1]/a:srgbClr", "val", u"cccccc");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:gradFill/a:gsLst/a:gs[2]/a:srgbClr", "val", u"666666");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:ln/a:noFill", 1);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testChartTitlePropertiesBitmapFillDOCX)
{
    loadFromFile(u"docx/testChartTitlePropertiesBitmapFill.docx");
    save(u"Office Open XML Text"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:blipFill/a:blip", "embed", u"rId1");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:ln/a:noFill", 1);
}


CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testColorGradientWithTransparencyDOCX)
{
    // Test color gradient (two color) with gradient transparency
    loadFromFile(u"docx/testColorGradientWithTransparency.docx");
    save(u"Office Open XML Text"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    // Test the transparency of the first color
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:spPr/a:gradFill/a:gsLst/a:gs[1]/a:srgbClr/a:alpha", "val", u"60000");
    // Test the transparency of the second color
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:spPr/a:gradFill/a:gsLst/a:gs[2]/a:srgbClr/a:alpha", "val", u"90000");
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testColorGradientWithTransparencyODS)
{
    // Test color gradient (two color) with simple transparency
    loadFromFile(u"ods/testColorGradientWithTransparency.ods");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    // Test the transparency of the first color
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:spPr/a:gradFill/a:gsLst/a:gs[1]/a:srgbClr/a:alpha", "val", u"60000");
    // Test the transparency of the second color
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:spPr/a:gradFill/a:gsLst/a:gs[2]/a:srgbClr/a:alpha", "val", u"60000");
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testColorGradientStopXLSX)
{
    // Test color gradient (two color) stop of the first color
    loadFromFile(u"xlsx/tdf128619.xlsx");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    // Test the position of the first color
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:spPr/a:gradFill/a:gsLst/a:gs[1]", "pos", u"45000");
    // Test the position of the second color
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:spPr/a:gradFill/a:gsLst/a:gs[2]", "pos", u"100000");
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testRadialColorGradientDOCX)
{
    loadFromFile(u"docx/tdf128794.docx");
    save(u"Office Open XML Text"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    // Test the gradient style (if there is no 'a:path' attribute, it is a linear gradient)
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:spPr/a:gradFill/a:path", "path", u"circle");
    // Test the linear gradient angle
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:spPr/a:gradFill/a:gsLst/a:gs[1]", "pos", u"0");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:spPr/a:gradFill/a:gsLst/a:gs[1]/a:srgbClr", "val", u"ffffff");

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:spPr/a:gradFill/a:gsLst/a:gs[2]", "pos", u"100000");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:spPr/a:gradFill/a:gsLst/a:gs[2]/a:srgbClr", "val", u"5b9bd5");
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testBarChartDataPointPropDOCX)
{
    loadFromFile(u"docx/testBarChartDataPointPropDOCX.docx");
    save(u"Office Open XML Text"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:varyColors", "val", u"0");

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dPt[1]/c:idx", "val", u"1");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dPt[1]/c:spPr/a:gradFill/a:gsLst/a:gs[1]/a:srgbClr", "val", u"f6f8fc");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dPt[1]/c:spPr/a:gradFill/a:gsLst/a:gs[2]/a:srgbClr", "val", u"c7d5ed");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dPt[1]/c:spPr/a:ln/a:solidFill/a:srgbClr", "val", u"70ad47");

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dPt[2]/c:idx", "val", u"2");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dPt[2]/c:spPr/a:solidFill/a:srgbClr", "val", u"ff0000");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dPt[2]/c:spPr/a:ln/a:solidFill/a:srgbClr", "val", u"000000");
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testFdo83058dlblPos)
{
    loadFromFile(u"docx/fdo83058_dlblPos.docx");

    // FIXME: validation error in OOXML export: Errors: 1
    skipValidation();

    save(u"Office Open XML Text"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[1]/c:dLbls[1]/c:dLbl[2]/c:dLblPos", "val", u"outEnd");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[1]/c:dLbls[1]/c:dLbl[3]/c:dLblPos", "val", u"outEnd");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[1]/c:dLbls[1]/c:dLbl[4]/c:dLblPos", "val", u"outEnd");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[1]/c:dLbls[1]/c:dLbl[5]/c:dLblPos", "val", u"outEnd");
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testAutoTitleDelXLSX)
{
    loadFromFile(u"xlsx/autotitledel_2007.xlsx");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:autoTitleDeleted", "val", u"0");
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testDispBlanksAsXLSX)
{
    loadFromFile(u"xlsx/dispBlanksAs_2007.xlsx");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:dispBlanksAs", "val", u"gap");
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testMarkerColorXLSX)
{
    loadFromFile(u"xlsx/markerColor.xlsx");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:scatterChart/c:ser/c:marker/c:spPr/a:solidFill/a:srgbClr", "val", u"92d050");
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testRoundedCornersXLSX)
{
    loadFromFile(u"xlsx/markerColor.xlsx");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:roundedCorners", "val", u"0");
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testAxisNumberFormatXLSX)
{
    loadFromFile(u"ods/axis_number_format.ods");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx", 2);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx[1]/c:numFmt", "formatCode", u"0.00E+000");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx[1]/c:numFmt", "sourceLinked", u"0");

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx[2]/c:numFmt", "formatCode", u"[$$-409]#,##0;\\-[$$-409]#,##0");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx[2]/c:numFmt", "sourceLinked", u"1");
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testDataPointLabelNumberFormatXLSX)
{
    // FIXME: validation error in OOXML export: Errors: 1
    skipValidation();

    loadFromFile(u"ods/tdf123774.ods");
    {
        save(u"Calc Office Open XML"_ustr);
        xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
        CPPUNIT_ASSERT(pXmlDoc);
        assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:pieChart/c:ser/c:dLbls/c:numFmt", "formatCode", u"[$-40E]0.00%");
        assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:pieChart/c:ser/c:dLbls/c:numFmt", "sourceLinked", u"0");

        assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:pieChart/c:ser/c:dLbls/c:dLbl[1]/c:numFmt", "formatCode", u"[$-40E]0.00%");
        assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:pieChart/c:ser/c:dLbls/c:dLbl[1]/c:numFmt", "sourceLinked", u"0");
    }

    loadFromFile(u"xlsx/tdf130986.xlsx");
    {
        save(u"Calc Office Open XML"_ustr);
        xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
        CPPUNIT_ASSERT(pXmlDoc);
        assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dLbls/c:dLbl/c:idx", "val", u"1");
        assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dLbls/c:dLbl/c:numFmt", "formatCode", u"0.00E+00");
        assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dLbls/c:dLbl/c:numFmt", "sourceLinked", u"0");
    }
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testDataLabelDefaultValuesXLSX)
{
    loadFromFile(u"xlsx/data_label.xlsx");
    Reference< chart2::XChartDocument> xDoc = getChartDocFromSheet(0);
    Reference<chart2::XDataSeries> xSeries = getDataSeriesFromDoc(xDoc, 0);
    Reference<beans::XPropertySet> xPropSet(xSeries, uno::UNO_QUERY_THROW);
    uno::Any aAny = xPropSet->getPropertyValue(u"Label"_ustr);
    chart2::DataPointLabel aLabel;
    CPPUNIT_ASSERT(aAny >>= aLabel);
    CPPUNIT_ASSERT(aLabel.ShowNumber);

    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dLbls/c:showVal", "val", u"1");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dLbls/c:dLblPos", "val", u"outEnd");
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testDataLabelFillColor)
{
    loadFromFile(u"xlsx/data_labels_fill_color.xlsx");
    Reference< chart2::XChartDocument> xDoc = getChartDocFromSheet(0);
    Reference<chart2::XDataSeries> xSeries = getDataSeriesFromDoc(xDoc, 0);
    Reference<beans::XPropertySet> xPropSet(xSeries, uno::UNO_QUERY_THROW);
    uno::Any aAny = xPropSet->getPropertyValue(u"LabelFillColor"_ustr);
    sal_Int32 nLabelFillColor;
    CPPUNIT_ASSERT(aAny >>= nLabelFillColor);

    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dLbls/c:spPr/a:solidFill/a:srgbClr", "val", u"F79646");
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testTitleOverlayXLSX)
{
    loadFromFile(u"xlsx/chart_title.xlsx");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:overlay", "val", u"0");
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testInvertIfNegativeXLSX)
{
    loadFromFile(u"xlsx/bar_chart_simple.xlsx");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:invertIfNegative", "val", u"0");
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testBubble3DXLSX)
{
    loadFromFile(u"xlsx/bubble_chart_simple.xlsx");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:bubbleChart/c:ser[1]/c:bubble3D", "val", u"0");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:bubbleChart/c:ser[2]/c:bubble3D", "val", u"0");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:bubbleChart/c:ser[3]/c:bubble3D", "val", u"0");
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testNoMarkerXLSX)
{
    loadFromFile(u"xlsx/no_marker.xlsx");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:ser[1]/c:marker/c:symbol", "val", u"none");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:ser[2]/c:marker/c:symbol", "val", u"none");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:marker", "val", u"0");
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testTitleManualLayoutXLSX)
{
    loadFromFile(u"xlsx/title_manual_layout.xlsx");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:layout/c:manualLayout/c:layoutTarget", 0);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:layout/c:manualLayout/c:xMode", "val", u"edge");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:layout/c:manualLayout/c:yMode", "val", u"edge");

    OUString aXVal = getXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:layout/c:manualLayout/c:x", "val");
    double nX = aXVal.toDouble();
    CPPUNIT_ASSERT(nX > 0);
    CPPUNIT_ASSERT(nX < 1);

    OUString aYVal = getXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:layout/c:manualLayout/c:y", "val");
    double nY = aYVal.toDouble();
    CPPUNIT_ASSERT(nY > 0);
    CPPUNIT_ASSERT(nY < 1);
    CPPUNIT_ASSERT(nX != nY);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:bodyPr", "rot", u"1200000");
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testPlotAreaManualLayoutXLSX)
{
    loadFromFile(u"xlsx/plot_area_manual_layout.xlsx");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:layout/c:manualLayout/c:layoutTarget", "val", u"inner");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:layout/c:manualLayout/c:xMode", "val", u"edge");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:layout/c:manualLayout/c:yMode", "val", u"edge");

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

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testLegendManualLayoutXLSX)
{
    loadFromFile(u"xlsx/legend_manual_layout.xlsx");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:layout/c:manualLayout/c:layoutTarget", 0);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:legend/c:layout/c:manualLayout/c:xMode", "val", u"edge");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:legend/c:layout/c:manualLayout/c:yMode", "val", u"edge");

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
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:legend/c:txPr/a:p/a:pPr/a:defRPr", "sz", u"900");
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testChartSubTitle)
{
    loadFromFile(u"ods/testChartSubTitle.ods");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    // test properties of subtitle
    // paragraph props
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p/a:pPr/a:defRPr", "sz", u"1100");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p/a:pPr/a:defRPr", "b", u"1");
    // run props
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p/a:r/a:rPr", "sz", u"1100");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p/a:r/a:rPr", "b", u"1");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p/a:r/a:rPr/a:solidFill/a:srgbClr", "val", u"00a933");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p/a:r/a:rPr/a:latin", "typeface", u"Times New Roman");
    // text
    assertXPathContent(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p/a:r/a:t", u"It is a Subtitle");
    // shape props
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:solidFill/a:srgbClr", "val", u"b2b2b2");
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testChartMainWithSubTitle)
{
    loadFromFile(u"ods/testChartMainWithSubTitle.ods");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    // test properties of title
    // paragraph props
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:pPr/a:defRPr", "sz", u"1300");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:pPr/a:defRPr", "b", u"0");
    // run props
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:r/a:rPr", "sz", u"1300");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:r/a:rPr", "b", u"0");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:r/a:rPr", "i", u"1");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:r/a:rPr/a:solidFill/a:srgbClr", "val", u"f10d0c");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:r/a:rPr/a:latin", "typeface", u"Arial");
    // text
    assertXPathContent(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:r/a:t", u"It is a Maintitle");
    assertXPathContent(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[2]/a:r/a:t", u"It is a Subtitle");
    // shape props
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:solidFill/a:srgbClr", "val", u"81d41a");
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testAutoTitleDeleted)
{
    loadFromFile(u"xlsx/testAutoTitleDeleted.xlsx");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:autoTitleDeleted", "val", u"1");
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testChartTitlePropertiesColorFillXLSX)
{
    loadFromFile(u"xlsx/testChartTitlePropertiesColorFill.xlsx");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:solidFill/a:srgbClr", "val", u"ff0000");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:ln/a:noFill", 1);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testChartTitlePropertiesGradientFillXLSX)
{
    loadFromFile(u"xlsx/testChartTitlePropertiesGradientFill.xlsx");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:gradFill/a:gsLst/a:gs[1]/a:srgbClr", "val", u"cccccc");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:gradFill/a:gsLst/a:gs[2]/a:srgbClr", "val", u"666666");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:ln/a:noFill", 1);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testChartTitlePropertiesBitmapFillXLSX)
{
    loadFromFile(u"xlsx/testChartTitlePropertiesBitmapFill.xlsx");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:blipFill/a:blip", "embed", u"rId1");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:ln/a:noFill", 1);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testBarChartDataPointPropXLSX)
{
    loadFromFile(u"xlsx/testBarChartDataPointPropXLSX.xlsx");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:varyColors", "val", u"0");

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dPt[1]/c:idx", "val", u"1");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dPt[1]/c:spPr/a:solidFill/a:srgbClr", "val", u"ff0000");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dPt[1]/c:spPr/a:ln/a:solidFill/a:srgbClr", "val", u"000000");

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dPt[2]/c:idx", "val", u"2");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dPt[2]/c:spPr/a:gradFill/a:gsLst/a:gs[1]/a:srgbClr", "val", u"f6f8fc");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dPt[2]/c:spPr/a:gradFill/a:gsLst/a:gs[2]/a:srgbClr", "val", u"c7d5ed");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dPt[2]/c:spPr/a:ln/a:solidFill/a:srgbClr", "val", u"70ad47");
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testAxisCharacterPropertiesXLSX)
{
    loadFromFile(u"xlsx/axis_character_properties.xlsx");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:catAx/c:txPr/a:p/a:pPr/a:defRPr", "sz", u"1000");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:catAx/c:txPr/a:p/a:pPr/a:defRPr", "b", u"1");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:catAx/c:txPr/a:p/a:pPr/a:defRPr", "i", u"1");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:catAx/c:txPr/a:p/a:pPr/a:defRPr", "u", u"sng");

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx/c:txPr/a:p/a:pPr/a:defRPr", "sz", u"900");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx/c:txPr/a:p/a:pPr/a:defRPr", "b", u"1");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx/c:txPr/a:p/a:pPr/a:defRPr", "strike", u"sngStrike");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx/c:txPr/a:p/a:pPr/a:defRPr/a:solidFill/a:srgbClr", "val", u"ff0000");
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testTitleCharacterPropertiesXLSX)
{
    loadFromFile(u"xlsx/title_character_properties.xlsx");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p/a:pPr/a:defRPr", "sz", u"1300");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p/a:pPr/a:defRPr", "b", u"0");

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p/a:r/a:rPr", "sz", u"2400");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p/a:r/a:rPr", "b", u"1");
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testPlotVisOnlyXLSX)
{
    loadFromFile(u"xlsx/hidden_cells.xlsx");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotVisOnly", "val", u"0");
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testBarChartVaryColorsXLSX)
{
    loadFromFile(u"xlsx/tdf90876.xlsx");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:varyColors", "val", u"0");
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testTdf96161)
{
    loadFromFile(u"ods/tdf96161.ods");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:varyColors", "val", u"0");
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testTableOnPage3)
{
    loadFromFile(u"docx/TableOnPage3.docx");

    // FIXME: validation error in OOXML export: Errors: 2
    skipValidation();

    saveAndReload(u"Office Open XML Text"_ustr);

    Reference<chart2::XChartDocument> xChartDoc(getChartDocFromWriter(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());
    uno::Reference< chart::XChartDataArray > xDataArray(xChartDoc->getDataProvider(), UNO_QUERY_THROW);
    Sequence<OUString> aColumnDesc = xDataArray->getColumnDescriptions();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There must be 4 columns and descriptions", static_cast<sal_Int32>(4), aColumnDesc.getLength());
    CPPUNIT_ASSERT_EQUAL(u"If oversubscription relative to allowance increases at the same average rate B15-B17"_ustr, aColumnDesc[0]);
    CPPUNIT_ASSERT_EQUAL(u"Known requirements"_ustr, aColumnDesc[1]);
    CPPUNIT_ASSERT_EQUAL(u"Allowance"_ustr, aColumnDesc[2]);
    CPPUNIT_ASSERT_EQUAL(u"If oversubscription relative to allowance holds steady at average oversubscription level B15-B17"_ustr, aColumnDesc[3]);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, tdf137691)
{
    // given a doc where the banana negative data formats as ($123) and the pineapple data as $(123)
    loadFromFile(u"pptx/tdf137691_dataTable.pptx");
    saveAndReload(u"Impress MS PowerPoint 2007 XML"_ustr);

    Reference<chart2::XChartDocument> xChartDoc(getChartDocFromDrawImpress(0, 0), uno::UNO_QUERY);

    Reference< chart2::data::XDataSequence > xDataSeq;
    xDataSeq.set(getDataSequenceFromDocByRole(xChartDoc, u"values-y", 0));
    const sal_Int32 nKey_bananas = xDataSeq->getNumberFormatKeyByIndex(-1);
    // This should not be General format (0), but a defined format (129)
    CPPUNIT_ASSERT(nKey_bananas);

    xDataSeq.set(getDataSequenceFromDocByRole(xChartDoc, u"values-y", 1));
    const sal_Int32 nKey_pineapples = xDataSeq->getNumberFormatKeyByIndex(-1);
    // This should not be General format (0), but a defined format (130)
    CPPUNIT_ASSERT(nKey_pineapples);
    CPPUNIT_ASSERT(nKey_pineapples != nKey_bananas);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testMultipleAxisXLSX)
{
    loadFromFile(u"ods/multiple_axis.ods");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:scatterChart", 2);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:scatterChart[1]/c:ser", 1);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:scatterChart[2]/c:ser", 1);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx", 4);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx/c:delete[@val='1']", 1);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx/c:axPos[@val='l']", 1);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx/c:axPos[@val='r']", 1);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testSecondaryAxisXLSX)
{
    loadFromFile(u"ods/secondary_axis.ods");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart", 2);
    // test there is just those series in the first <lineChart> tag which are attached to the primary axis
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart[1]/c:ser", 2);
    assertXPathContent(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart[1]/c:ser[1]/c:tx/c:strRef/c:strCache/c:pt/c:v", u"b");
    assertXPathContent(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart[1]/c:ser[2]/c:tx/c:strRef/c:strCache/c:pt/c:v", u"c");
    // test there is just those series in the second <lineChart> tag which are attached to the secondary axis
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart[2]/c:ser", 1);
    assertXPathContent(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart[2]/c:ser[1]/c:tx/c:strRef/c:strCache/c:pt/c:v", u"a");
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testBarChartSecondaryAxisXLSX)
{
    loadFromFile(u"xlsx/testSecondaryAxis.xlsx");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
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

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testTdf148142)
{
    // The document contains a line chart with "Between tick marks" X axis position.
    loadFromFile(u"ods/tdf148142.ods");
    Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0);
    CPPUNIT_ASSERT(xChartDoc.is());
    Reference<chart2::XAxis> xAxis = getAxisFromDoc(xChartDoc, 0, 0, 0);
    CPPUNIT_ASSERT(xAxis.is());
    chart2::ScaleData aScaleData = xAxis->getScaleData();
    CPPUNIT_ASSERT(aScaleData.ShiftedCategoryPosition);

    // Set the X axis position to "On tick marks".
    aScaleData.ShiftedCategoryPosition = false;
    xAxis->setScaleData(aScaleData);

    // Check the X axis position after export.
    saveAndReload(u"calc8"_ustr);
    Reference<chart2::XChartDocument> xChartDoc2 = getChartDocFromSheet(0);
    CPPUNIT_ASSERT(xChartDoc2.is());
    Reference<chart2::XAxis> xAxis2 = getAxisFromDoc(xChartDoc2, 0, 0, 0);
    CPPUNIT_ASSERT(xAxis2.is());
    chart2::ScaleData aScaleData2 = xAxis2->getScaleData();
    CPPUNIT_ASSERT(!aScaleData2.ShiftedCategoryPosition);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testFormattedChartTitles)
{
    loadFromFile(u"xlsx/tdf39052.xlsx");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    // Check run level properties [1] - first paragraph
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:r[1]/a:rPr", "b", u"1");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:r[1]/a:rPr", "sz", u"1400");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:r[1]/a:rPr/a:solidFill/a:srgbClr", "val", u"ff0000");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:r[1]/a:rPr/a:latin", "typeface", u"Aptos Narrow");
    assertXPathContent(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:r[1]/a:t", u"This");
    // Check run level properties [2]
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:r[2]/a:rPr", "b", u"0");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:r[2]/a:rPr", "sz", u"1400");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:r[2]/a:rPr/a:solidFill/a:srgbClr", "val", u"595959");
    assertXPathContent(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:r[2]/a:t", u" is");
    // Check run level properties [3]
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:r[3]/a:rPr", "b", u"0");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:r[3]/a:rPr", "sz", u"1400");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:r[3]/a:rPr", "baseline", u"30000");
    assertXPathContent(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:r[3]/a:t", u"3");
    // Check run level properties [4]
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:r[4]/a:rPr", "b", u"0");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:r[4]/a:rPr", "sz", u"1400");
    assertXPathContent(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:r[4]/a:t", u" a ");
    // Check run level properties [5]
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:r[5]/a:rPr", "b", u"0");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:r[5]/a:rPr", "i", u"1");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:r[5]/a:rPr", "sz", u"2000");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:r[5]/a:rPr", "u", u"sng");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:r[5]/a:rPr/a:solidFill/a:srgbClr", "val", u"4ea72e");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:r[5]/a:rPr/a:uFillTx", 1);
    assertXPathContent(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:r[5]/a:t", u"custom");
    // Check run level properties [6]
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:r[6]/a:rPr", "b", u"0");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:r[6]/a:rPr", "sz", u"1400");
    assertXPathContent(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:r[6]/a:t", u" erte1");
    // Check run level properties [1] - second paragraph
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[2]/a:r[1]/a:rPr", "b", u"0");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[2]/a:r[1]/a:rPr", "sz", u"1400");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[2]/a:r[1]/a:rPr/a:solidFill/a:srgbClr", "val", u"595959");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[2]/a:r[1]/a:rPr/a:latin", "typeface", u"Aptos Narrow");
    assertXPathContent(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[2]/a:r[1]/a:t", u"2dfgd ch");
    // Check run level properties [2]
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[2]/a:r[2]/a:rPr", "b", u"1");
    assertXPathContent(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[2]/a:r[2]/a:t", u"ar");
    // Check run level properties [3]
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[2]/a:r[3]/a:rPr", "b", u"0");;
    assertXPathContent(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[2]/a:r[3]/a:t", u"t ");
    // Check run level properties [4]
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[2]/a:r[4]/a:rPr", "b", u"0");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[2]/a:r[4]/a:rPr", "strike", u"sngStrike");
    assertXPathContent(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[2]/a:r[4]/a:t", u"title");
}

namespace {

void checkCharacterProps(Reference<beans::XPropertySet> const & xTitleProp)
{
    Sequence< uno::Reference< chart2::XFormattedString > > xFormattedSubTitle;
    CPPUNIT_ASSERT(xTitleProp->getPropertyValue(u"FormattedStrings"_ustr) >>= xFormattedSubTitle);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(10), xFormattedSubTitle.getLength());
    // check texts
    std::vector<OUString> aValues = { u"This"_ustr, u" is"_ustr, u"3"_ustr, u" a "_ustr, u"custom"_ustr, u" erte1\n"_ustr, u"2dfgd ch"_ustr, u"ar"_ustr, u"t "_ustr, u"title"_ustr };
    for (sal_Int32 i = 0; i < xFormattedSubTitle.getLength(); i++)
    {
        const OUString aText = xFormattedSubTitle[i]->getString();
        CPPUNIT_ASSERT_EQUAL(aValues[i], aText);
        Reference< beans::XPropertySet > xRunPropSet(xFormattedSubTitle[i], uno::UNO_QUERY);
        // common props
        uno::Any aAny = xRunPropSet->getPropertyValue(u"CharFontName"_ustr);
        CPPUNIT_ASSERT_EQUAL(uno::Any(u"Aptos Narrow"_ustr), aAny);
        // unique props
        if (aText == aValues[0])
        {
            aAny = xRunPropSet->getPropertyValue(u"CharWeight"_ustr);
            CPPUNIT_ASSERT_EQUAL(uno::Any(awt::FontWeight::BOLD), aAny);
            aAny = xRunPropSet->getPropertyValue(u"CharHeight"_ustr);
            CPPUNIT_ASSERT_EQUAL(uno::Any(14.0f), aAny);
            aAny = xRunPropSet->getPropertyValue(u"CharColor"_ustr);
            CPPUNIT_ASSERT_EQUAL(uno::Any(Color(0xff0000)), aAny);
            aAny = xRunPropSet->getPropertyValue(u"CharEscapement"_ustr);
            CPPUNIT_ASSERT_EQUAL(uno::Any(short(0)), aAny);
            aAny = xRunPropSet->getPropertyValue(u"CharEscapementHeight"_ustr);
            CPPUNIT_ASSERT_EQUAL(uno::Any(short(100)), aAny);
        }
        else if (aText == aValues[1] || aText == aValues[3] || aText == aValues[5] ||
            aText == aValues[6] || aText == aValues[8])
        {
            aAny = xRunPropSet->getPropertyValue(u"CharWeight"_ustr);
            CPPUNIT_ASSERT_EQUAL(uno::Any(awt::FontWeight::NORMAL), aAny);
            aAny = xRunPropSet->getPropertyValue(u"CharHeight"_ustr);
            CPPUNIT_ASSERT_EQUAL(uno::Any(14.0f), aAny);
            aAny = xRunPropSet->getPropertyValue(u"CharColor"_ustr);
            CPPUNIT_ASSERT_EQUAL(uno::Any(Color(0x595959)), aAny);
        }
        else if (aText == aValues[2])
        {
            aAny = xRunPropSet->getPropertyValue(u"CharWeight"_ustr);
            CPPUNIT_ASSERT_EQUAL(uno::Any(awt::FontWeight::NORMAL), aAny);
            aAny = xRunPropSet->getPropertyValue(u"CharHeight"_ustr);
            CPPUNIT_ASSERT_EQUAL(uno::Any(14.0f), aAny);
            aAny = xRunPropSet->getPropertyValue(u"CharColor"_ustr);
            CPPUNIT_ASSERT_EQUAL(uno::Any(Color(0x595959)), aAny);
            aAny = xRunPropSet->getPropertyValue(u"CharEscapement"_ustr);
            CPPUNIT_ASSERT_EQUAL(uno::Any(short(30)), aAny);
            aAny = xRunPropSet->getPropertyValue(u"CharEscapementHeight"_ustr);
            CPPUNIT_ASSERT_EQUAL(uno::Any(short(58)), aAny);
        }
        else if (aText == aValues[4])
        {
            aAny = xRunPropSet->getPropertyValue(u"CharWeight"_ustr);
            CPPUNIT_ASSERT_EQUAL(uno::Any(awt::FontWeight::NORMAL), aAny);
            aAny = xRunPropSet->getPropertyValue(u"CharHeight"_ustr);
            CPPUNIT_ASSERT_EQUAL(uno::Any(20.0f), aAny);
            aAny = xRunPropSet->getPropertyValue(u"CharColor"_ustr);
            CPPUNIT_ASSERT_EQUAL(uno::Any(Color(0x4ea72e)), aAny);
            aAny = xRunPropSet->getPropertyValue(u"CharPosture"_ustr);
            CPPUNIT_ASSERT_EQUAL(uno::Any(awt::FontSlant_ITALIC), aAny);
            aAny = xRunPropSet->getPropertyValue(u"CharUnderline"_ustr);
            CPPUNIT_ASSERT_EQUAL(uno::Any(awt::FontUnderline::SINGLE), aAny);
        }
        else if (aText == aValues[7])
        {
            aAny = xRunPropSet->getPropertyValue(u"CharWeight"_ustr);
            CPPUNIT_ASSERT_EQUAL(uno::Any(awt::FontWeight::BOLD), aAny);
            aAny = xRunPropSet->getPropertyValue(u"CharHeight"_ustr);
            CPPUNIT_ASSERT_EQUAL(uno::Any(14.0f), aAny);
            aAny = xRunPropSet->getPropertyValue(u"CharColor"_ustr);
            CPPUNIT_ASSERT_EQUAL(uno::Any(Color(0x595959)), aAny);
            aAny = xRunPropSet->getPropertyValue(u"CharPosture"_ustr);
            CPPUNIT_ASSERT_EQUAL(uno::Any(awt::FontSlant_NONE), aAny);
        }
        else // aText == aValues[9]
        {
            aAny = xRunPropSet->getPropertyValue(u"CharWeight"_ustr);
            CPPUNIT_ASSERT_EQUAL(uno::Any(awt::FontWeight::NORMAL), aAny);
            aAny = xRunPropSet->getPropertyValue(u"CharHeight"_ustr);
            CPPUNIT_ASSERT_EQUAL(uno::Any(14.0f), aAny);
            aAny = xRunPropSet->getPropertyValue(u"CharColor"_ustr);
            CPPUNIT_ASSERT_EQUAL(uno::Any(Color(0x595959)), aAny);
            aAny = xRunPropSet->getPropertyValue(u"CharPosture"_ustr);
            CPPUNIT_ASSERT_EQUAL(uno::Any(awt::FontSlant_ITALIC), aAny);
            aAny = xRunPropSet->getPropertyValue(u"CharOverline"_ustr);
            CPPUNIT_ASSERT_EQUAL(uno::Any(awt::FontUnderline::NONE), aAny);
        }
    }
}

}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testODSFormattedChartTitles)
{
    // The document contains a line chart with "Between tick marks" X axis position.
    loadFromFile(u"ods/tdf39052.ods");
    // Check formatted strings after export.
    saveAndReload(u"calc8"_ustr);

    Reference<chart2::XChartDocument> xChart2Doc = getChartDocFromSheet(0);
    CPPUNIT_ASSERT(xChart2Doc.is());
    Reference< chart::XChartDocument > xChartDoc(xChart2Doc, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());
    uno::Reference< beans::XPropertySet > xTitleProp(xChartDoc->getTitle(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xTitleProp.is());

    checkCharacterProps(xTitleProp);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
