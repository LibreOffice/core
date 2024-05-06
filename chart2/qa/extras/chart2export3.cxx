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
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:ser/c:dLbls/c:dLbl[1]/c:idx"_ostr, "val"_ostr, u"1"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:ser/c:dLbls/c:dLbl[1]/c:txPr/a:p/a:pPr/a:defRPr"_ostr, "b"_ostr, u"1"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:ser/c:dLbls/c:dLbl[1]/c:txPr/a:p/a:pPr/a:defRPr"_ostr, "sz"_ostr, u"2000"_ustr);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testTdf114139)
{
    loadFromFile(u"xlsx/tdf114139.xlsx");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:pie3DChart"_ostr, 1);
    //no fill
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:spPr"_ostr, 0);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:spPr/a:solidFill"_ostr, 0);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testTdf64224)
{
    loadFromFile(u"ods/tdf64224.ods");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    //no fill
    assertXPath(pXmlDoc, "/c:chartSpace/c:spPr/a:noFill"_ostr, 1);
    assertXPath(pXmlDoc, "/c:chartSpace/c:spPr/a:solidFill"_ostr, 0);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testChartTitlePropertiesColorFillDOCX)
{
    loadFromFile(u"docx/testChartTitlePropertiesColorFill.docx");
    save(u"Office Open XML Text"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:solidFill/a:srgbClr"_ostr, "val"_ostr, u"ff0000"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:ln/a:noFill"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testChartTitlePropertiesGradientFillDOCX)
{
    loadFromFile(u"docx/testChartTitlePropertiesGradientFill.docx");
    save(u"Office Open XML Text"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:gradFill/a:gsLst/a:gs[1]/a:srgbClr"_ostr, "val"_ostr, u"cccccc"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:gradFill/a:gsLst/a:gs[2]/a:srgbClr"_ostr, "val"_ostr, u"666666"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:ln/a:noFill"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testChartTitlePropertiesBitmapFillDOCX)
{
    loadFromFile(u"docx/testChartTitlePropertiesBitmapFill.docx");
    save(u"Office Open XML Text"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:blipFill/a:blip"_ostr, "embed"_ostr, u"rId1"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:ln/a:noFill"_ostr, 1);
}


CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testColorGradientWithTransparencyDOCX)
{
    // Test color gradient (two color) with gradient transparency
    loadFromFile(u"docx/testColorGradientWithTransparency.docx");
    save(u"Office Open XML Text"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    // Test the transparency of the first color
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:spPr/a:gradFill/a:gsLst/a:gs[1]/a:srgbClr/a:alpha"_ostr, "val"_ostr, u"60000"_ustr);
    // Test the transparency of the second color
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:spPr/a:gradFill/a:gsLst/a:gs[2]/a:srgbClr/a:alpha"_ostr, "val"_ostr, u"90000"_ustr);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testColorGradientWithTransparencyODS)
{
    // Test color gradient (two color) with simple transparency
    loadFromFile(u"ods/testColorGradientWithTransparency.ods");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    // Test the transparency of the first color
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:spPr/a:gradFill/a:gsLst/a:gs[1]/a:srgbClr/a:alpha"_ostr, "val"_ostr, u"60000"_ustr);
    // Test the transparency of the second color
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:spPr/a:gradFill/a:gsLst/a:gs[2]/a:srgbClr/a:alpha"_ostr, "val"_ostr, u"60000"_ustr);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testColorGradientStopXLSX)
{
    // Test color gradient (two color) stop of the first color
    loadFromFile(u"xlsx/tdf128619.xlsx");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    // Test the position of the first color
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:spPr/a:gradFill/a:gsLst/a:gs[1]"_ostr, "pos"_ostr, u"45000"_ustr);
    // Test the position of the second color
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:spPr/a:gradFill/a:gsLst/a:gs[2]"_ostr, "pos"_ostr, u"100000"_ustr);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testRadialColorGradientDOCX)
{
    loadFromFile(u"docx/tdf128794.docx");
    save(u"Office Open XML Text"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    // Test the gradient style (if there is no 'a:path' attribute, it is a linear gradient)
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:spPr/a:gradFill/a:path"_ostr, 0);
    // Test the linear gradient angle
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:spPr/a:gradFill/a:lin"_ostr, "ang"_ostr, u"13500000"_ustr);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testBarChartDataPointPropDOCX)
{
    loadFromFile(u"docx/testBarChartDataPointPropDOCX.docx");
    save(u"Office Open XML Text"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:varyColors"_ostr, "val"_ostr, u"0"_ustr);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dPt[1]/c:idx"_ostr, "val"_ostr, u"1"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dPt[1]/c:spPr/a:gradFill/a:gsLst/a:gs[1]/a:srgbClr"_ostr, "val"_ostr, u"f6f8fc"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dPt[1]/c:spPr/a:gradFill/a:gsLst/a:gs[2]/a:srgbClr"_ostr, "val"_ostr, u"c7d5ed"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dPt[1]/c:spPr/a:ln/a:solidFill/a:srgbClr"_ostr, "val"_ostr, u"70ad47"_ustr);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dPt[2]/c:idx"_ostr, "val"_ostr, u"2"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dPt[2]/c:spPr/a:solidFill/a:srgbClr"_ostr, "val"_ostr, u"ff0000"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dPt[2]/c:spPr/a:ln/a:solidFill/a:srgbClr"_ostr, "val"_ostr, u"000000"_ustr);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testFdo83058dlblPos)
{
    loadFromFile(u"docx/fdo83058_dlblPos.docx");

    // FIXME: validation error in OOXML export: Errors: 1
    skipValidation();

    save(u"Office Open XML Text"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[1]/c:dLbls[1]/c:dLbl[2]/c:dLblPos"_ostr, "val"_ostr, u"outEnd"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[1]/c:dLbls[1]/c:dLbl[3]/c:dLblPos"_ostr, "val"_ostr, u"outEnd"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[1]/c:dLbls[1]/c:dLbl[4]/c:dLblPos"_ostr, "val"_ostr, u"outEnd"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[1]/c:dLbls[1]/c:dLbl[5]/c:dLblPos"_ostr, "val"_ostr, u"outEnd"_ustr);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testAutoTitleDelXLSX)
{
    loadFromFile(u"xlsx/autotitledel_2007.xlsx");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:autoTitleDeleted"_ostr, "val"_ostr, u"0"_ustr);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testDispBlanksAsXLSX)
{
    loadFromFile(u"xlsx/dispBlanksAs_2007.xlsx");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:dispBlanksAs"_ostr, "val"_ostr, u"gap"_ustr);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testMarkerColorXLSX)
{
    loadFromFile(u"xlsx/markerColor.xlsx");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:scatterChart/c:ser/c:marker/c:spPr/a:solidFill/a:srgbClr"_ostr, "val"_ostr, u"92d050"_ustr);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testRoundedCornersXLSX)
{
    loadFromFile(u"xlsx/markerColor.xlsx");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:roundedCorners"_ostr, "val"_ostr, u"0"_ustr);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testAxisNumberFormatXLSX)
{
    loadFromFile(u"ods/axis_number_format.ods");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx"_ostr, 2);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx[1]/c:numFmt"_ostr, "formatCode"_ostr, u"0.00E+000"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx[1]/c:numFmt"_ostr, "sourceLinked"_ostr, u"0"_ustr);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx[2]/c:numFmt"_ostr, "formatCode"_ostr, u"[$$-409]#,##0;\\-[$$-409]#,##0"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx[2]/c:numFmt"_ostr, "sourceLinked"_ostr, u"1"_ustr);
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
        assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:pieChart/c:ser/c:dLbls/c:numFmt"_ostr, "formatCode"_ostr, u"[$-40E]0.00%"_ustr);
        assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:pieChart/c:ser/c:dLbls/c:numFmt"_ostr, "sourceLinked"_ostr, u"0"_ustr);

        assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:pieChart/c:ser/c:dLbls/c:dLbl[1]/c:numFmt"_ostr, "formatCode"_ostr, u"[$-40E]0.00%"_ustr);
        assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:pieChart/c:ser/c:dLbls/c:dLbl[1]/c:numFmt"_ostr, "sourceLinked"_ostr, u"0"_ustr);
    }

    loadFromFile(u"xlsx/tdf130986.xlsx");
    {
        save(u"Calc Office Open XML"_ustr);
        xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
        CPPUNIT_ASSERT(pXmlDoc);
        assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dLbls/c:dLbl/c:idx"_ostr, "val"_ostr, u"1"_ustr);
        assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dLbls/c:dLbl/c:numFmt"_ostr, "formatCode"_ostr, u"0.00E+00"_ustr);
        assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dLbls/c:dLbl/c:numFmt"_ostr, "sourceLinked"_ostr, u"0"_ustr);
    }
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testDataLabelDefaultValuesXLSX)
{
    loadFromFile(u"xlsx/data_label.xlsx");
    Reference< chart2::XChartDocument> xDoc = getChartDocFromSheet(0, mxComponent);
    Reference<chart2::XDataSeries> xSeries = getDataSeriesFromDoc(xDoc, 0);
    Reference<beans::XPropertySet> xPropSet(xSeries, uno::UNO_QUERY_THROW);
    uno::Any aAny = xPropSet->getPropertyValue(u"Label"_ustr);
    chart2::DataPointLabel aLabel;
    CPPUNIT_ASSERT(aAny >>= aLabel);
    CPPUNIT_ASSERT(aLabel.ShowNumber);

    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dLbls/c:showVal"_ostr, "val"_ostr, u"1"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dLbls/c:dLblPos"_ostr, "val"_ostr, u"outEnd"_ustr);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testDataLabelFillColor)
{
    loadFromFile(u"xlsx/data_labels_fill_color.xlsx");
    Reference< chart2::XChartDocument> xDoc = getChartDocFromSheet(0, mxComponent);
    Reference<chart2::XDataSeries> xSeries = getDataSeriesFromDoc(xDoc, 0);
    Reference<beans::XPropertySet> xPropSet(xSeries, uno::UNO_QUERY_THROW);
    uno::Any aAny = xPropSet->getPropertyValue(u"LabelFillColor"_ustr);
    sal_Int32 nLabelFillColor;
    CPPUNIT_ASSERT(aAny >>= nLabelFillColor);

    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dLbls/c:spPr/a:solidFill/a:srgbClr"_ostr, "val"_ostr, u"F79646"_ustr);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testTitleOverlayXLSX)
{
    loadFromFile(u"xlsx/chart_title.xlsx");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:overlay"_ostr, "val"_ostr, u"0"_ustr);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testInvertIfNegativeXLSX)
{
    loadFromFile(u"xlsx/bar_chart_simple.xlsx");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:invertIfNegative"_ostr, "val"_ostr, u"0"_ustr);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testBubble3DXLSX)
{
    loadFromFile(u"xlsx/bubble_chart_simple.xlsx");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:bubbleChart/c:ser[1]/c:bubble3D"_ostr, "val"_ostr, u"0"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:bubbleChart/c:ser[2]/c:bubble3D"_ostr, "val"_ostr, u"0"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:bubbleChart/c:ser[3]/c:bubble3D"_ostr, "val"_ostr, u"0"_ustr);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testNoMarkerXLSX)
{
    loadFromFile(u"xlsx/no_marker.xlsx");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:ser[1]/c:marker/c:symbol"_ostr, "val"_ostr, u"none"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:ser[2]/c:marker/c:symbol"_ostr, "val"_ostr, u"none"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:marker"_ostr, "val"_ostr, u"0"_ustr);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testTitleManualLayoutXLSX)
{
    loadFromFile(u"xlsx/title_manual_layout.xlsx");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:layout/c:manualLayout/c:layoutTarget"_ostr, 0);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:layout/c:manualLayout/c:xMode"_ostr, "val"_ostr, u"edge"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:layout/c:manualLayout/c:yMode"_ostr, "val"_ostr, u"edge"_ustr);

    OUString aXVal = getXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:layout/c:manualLayout/c:x"_ostr, "val"_ostr);
    double nX = aXVal.toDouble();
    CPPUNIT_ASSERT(nX > 0);
    CPPUNIT_ASSERT(nX < 1);

    OUString aYVal = getXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:layout/c:manualLayout/c:y"_ostr, "val"_ostr);
    double nY = aYVal.toDouble();
    CPPUNIT_ASSERT(nY > 0);
    CPPUNIT_ASSERT(nY < 1);
    CPPUNIT_ASSERT(nX != nY);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:bodyPr"_ostr, "rot"_ostr, u"1200000"_ustr);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testPlotAreaManualLayoutXLSX)
{
    loadFromFile(u"xlsx/plot_area_manual_layout.xlsx");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:layout/c:manualLayout/c:layoutTarget"_ostr, "val"_ostr, u"inner"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:layout/c:manualLayout/c:xMode"_ostr, "val"_ostr, u"edge"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:layout/c:manualLayout/c:yMode"_ostr, "val"_ostr, u"edge"_ustr);

    OUString aXVal = getXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:layout/c:manualLayout/c:x"_ostr, "val"_ostr);
    double nX = aXVal.toDouble();
    CPPUNIT_ASSERT(nX > 0);
    CPPUNIT_ASSERT(nX < 1);

    OUString aYVal = getXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:layout/c:manualLayout/c:y"_ostr, "val"_ostr);
    double nY = aYVal.toDouble();
    CPPUNIT_ASSERT(nY > 0);
    CPPUNIT_ASSERT(nY < 1);
    CPPUNIT_ASSERT(nX != nY);

    OUString aWVal = getXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:layout/c:manualLayout/c:w"_ostr, "val"_ostr);
    double nW = aWVal.toDouble();
    CPPUNIT_ASSERT(nW > 0);
    CPPUNIT_ASSERT(nW < 1);

    OUString aHVal = getXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:layout/c:manualLayout/c:h"_ostr, "val"_ostr);
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

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:layout/c:manualLayout/c:layoutTarget"_ostr, 0);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:legend/c:layout/c:manualLayout/c:xMode"_ostr, "val"_ostr, u"edge"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:legend/c:layout/c:manualLayout/c:yMode"_ostr, "val"_ostr, u"edge"_ustr);

    OUString aXVal = getXPath(pXmlDoc, "/c:chartSpace/c:chart/c:legend/c:layout/c:manualLayout/c:x"_ostr, "val"_ostr);
    double nX = aXVal.toDouble();
    CPPUNIT_ASSERT(nX > 0);
    CPPUNIT_ASSERT(nX < 1);

    OUString aYVal = getXPath(pXmlDoc, "/c:chartSpace/c:chart/c:legend/c:layout/c:manualLayout/c:y"_ostr, "val"_ostr);
    double nY = aYVal.toDouble();
    CPPUNIT_ASSERT(nY > 0);
    CPPUNIT_ASSERT(nY < 1);
    CPPUNIT_ASSERT(nX != nY);

    OUString aWVal = getXPath(pXmlDoc, "/c:chartSpace/c:chart/c:legend/c:layout/c:manualLayout/c:w"_ostr, "val"_ostr);
    double nW = aWVal.toDouble();
    CPPUNIT_ASSERT(nW > 0);
    CPPUNIT_ASSERT(nW < 1);

    OUString aHVal = getXPath(pXmlDoc, "/c:chartSpace/c:chart/c:legend/c:layout/c:manualLayout/c:h"_ostr, "val"_ostr);
    double nH = aHVal.toDouble();
    CPPUNIT_ASSERT(nH > 0);
    CPPUNIT_ASSERT(nH < 1);
    CPPUNIT_ASSERT(nH != nW);

    // Make sure that default text font size is preserved after export
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:legend/c:txPr/a:p/a:pPr/a:defRPr"_ostr, "sz"_ostr, u"900"_ustr);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testChartSubTitle)
{
    loadFromFile(u"ods/testChartSubTitle.ods");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    // test properties of subtitle
    // paragraph props
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p/a:pPr/a:defRPr"_ostr, "sz"_ostr, u"1100"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p/a:pPr/a:defRPr"_ostr, "b"_ostr, u"1"_ustr);
    // run props
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p/a:r/a:rPr"_ostr, "sz"_ostr, u"1100"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p/a:r/a:rPr"_ostr, "b"_ostr, u"1"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p/a:r/a:rPr/a:solidFill/a:srgbClr"_ostr, "val"_ostr, u"00a933"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p/a:r/a:rPr/a:latin"_ostr, "typeface"_ostr, u"Times New Roman"_ustr);
    // text
    assertXPathContent(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p/a:r/a:t"_ostr, u"It is a Subtitle"_ustr);
    // shape props
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:solidFill/a:srgbClr"_ostr, "val"_ostr, u"b2b2b2"_ustr);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testChartMainWithSubTitle)
{
    loadFromFile(u"ods/testChartMainWithSubTitle.ods");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    // test properties of title
    // paragraph props
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:pPr/a:defRPr"_ostr, "sz"_ostr, u"1300"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:pPr/a:defRPr"_ostr, "b"_ostr, u"0"_ustr);
    // run props
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:r/a:rPr"_ostr, "sz"_ostr, u"1300"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:r/a:rPr"_ostr, "b"_ostr, u"0"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:r/a:rPr"_ostr, "i"_ostr, u"1"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:r/a:rPr/a:solidFill/a:srgbClr"_ostr, "val"_ostr, u"f10d0c"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:r/a:rPr/a:latin"_ostr, "typeface"_ostr, u"Arial"_ustr);
    // text
    assertXPathContent(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:r/a:t"_ostr, u"It is a Maintitle"_ustr);
    assertXPathContent(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[2]/a:r/a:t"_ostr, u"It is a Subtitle"_ustr);
    // shape props
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:solidFill/a:srgbClr"_ostr, "val"_ostr, u"81d41a"_ustr);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testAutoTitleDeleted)
{
    loadFromFile(u"xlsx/testAutoTitleDeleted.xlsx");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:autoTitleDeleted"_ostr, "val"_ostr, u"1"_ustr);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testChartTitlePropertiesColorFillXLSX)
{
    loadFromFile(u"xlsx/testChartTitlePropertiesColorFill.xlsx");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:solidFill/a:srgbClr"_ostr, "val"_ostr, u"ff0000"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:ln/a:noFill"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testChartTitlePropertiesGradientFillXLSX)
{
    loadFromFile(u"xlsx/testChartTitlePropertiesGradientFill.xlsx");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:gradFill/a:gsLst/a:gs[1]/a:srgbClr"_ostr, "val"_ostr, u"cccccc"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:gradFill/a:gsLst/a:gs[2]/a:srgbClr"_ostr, "val"_ostr, u"666666"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:ln/a:noFill"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testChartTitlePropertiesBitmapFillXLSX)
{
    loadFromFile(u"xlsx/testChartTitlePropertiesBitmapFill.xlsx");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:blipFill/a:blip"_ostr, "embed"_ostr, u"rId1"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:ln/a:noFill"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testBarChartDataPointPropXLSX)
{
    loadFromFile(u"xlsx/testBarChartDataPointPropXLSX.xlsx");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:varyColors"_ostr, "val"_ostr, u"0"_ustr);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dPt[1]/c:idx"_ostr, "val"_ostr, u"1"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dPt[1]/c:spPr/a:solidFill/a:srgbClr"_ostr, "val"_ostr, u"ff0000"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dPt[1]/c:spPr/a:ln/a:solidFill/a:srgbClr"_ostr, "val"_ostr, u"000000"_ustr);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dPt[2]/c:idx"_ostr, "val"_ostr, u"2"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dPt[2]/c:spPr/a:gradFill/a:gsLst/a:gs[1]/a:srgbClr"_ostr, "val"_ostr, u"f6f8fc"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dPt[2]/c:spPr/a:gradFill/a:gsLst/a:gs[2]/a:srgbClr"_ostr, "val"_ostr, u"c7d5ed"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dPt[2]/c:spPr/a:ln/a:solidFill/a:srgbClr"_ostr, "val"_ostr, u"70ad47"_ustr);
}

namespace {

void checkGapWidth(Reference<beans::XPropertySet> const & xPropSet, sal_Int32 nValue)
{
    uno::Any aAny = xPropSet->getPropertyValue(u"GapwidthSequence"_ustr);
    CPPUNIT_ASSERT(aAny.hasValue());
    uno::Sequence< sal_Int32 > aSequence;
    aAny >>= aSequence;
    CPPUNIT_ASSERT(aSequence.hasElements());
    CPPUNIT_ASSERT_EQUAL(nValue, aSequence[0]);
}

void checkOverlap(Reference<beans::XPropertySet> const & xPropSet, sal_Int32 nValue)
{
    uno::Any aAny = xPropSet->getPropertyValue(u"OverlapSequence"_ustr);
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

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testDataseriesOverlapStackedChartXLSX)
{
    loadFromFile(u"xlsx/testDataseriesOverlapStackedChart.xlsx");

    // test the overlap value of a simple Stacked Column Chart
    uno::Reference< chart2::XChartDocument > xChartDoc = getChartDocFromSheet( 0, mxComponent );
    checkSheetForGapWidthAndOverlap(xChartDoc, 100, 0);

    // test the overlap value of a Percent Stacked Bar Chart
    xChartDoc = getChartDocFromSheet( 1, mxComponent );
    checkSheetForGapWidthAndOverlap(xChartDoc, 100, 35);

    saveAndReload(u"Calc Office Open XML"_ustr);

    xChartDoc = getChartDocFromSheet( 0, mxComponent );
    checkSheetForGapWidthAndOverlap(xChartDoc, 100, 100);

    xChartDoc = getChartDocFromSheet( 1, mxComponent );
    checkSheetForGapWidthAndOverlap(xChartDoc, 100, 100);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testAxisCharacterPropertiesXLSX)
{
    loadFromFile(u"xlsx/axis_character_properties.xlsx");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:catAx/c:txPr/a:p/a:pPr/a:defRPr"_ostr, "sz"_ostr, u"1000"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:catAx/c:txPr/a:p/a:pPr/a:defRPr"_ostr, "b"_ostr, u"1"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:catAx/c:txPr/a:p/a:pPr/a:defRPr"_ostr, "i"_ostr, u"1"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:catAx/c:txPr/a:p/a:pPr/a:defRPr"_ostr, "u"_ostr, u"sng"_ustr);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx/c:txPr/a:p/a:pPr/a:defRPr"_ostr, "sz"_ostr, u"900"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx/c:txPr/a:p/a:pPr/a:defRPr"_ostr, "b"_ostr, u"1"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx/c:txPr/a:p/a:pPr/a:defRPr"_ostr, "strike"_ostr, u"sngStrike"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx/c:txPr/a:p/a:pPr/a:defRPr/a:solidFill/a:srgbClr"_ostr, "val"_ostr, u"ff0000"_ustr);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testTitleCharacterPropertiesXLSX)
{
    loadFromFile(u"xlsx/title_character_properties.xlsx");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p/a:pPr/a:defRPr"_ostr, "sz"_ostr, u"1300"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p/a:pPr/a:defRPr"_ostr, "b"_ostr, u"0"_ustr);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p/a:r/a:rPr"_ostr, "sz"_ostr, u"2400"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p/a:r/a:rPr"_ostr, "b"_ostr, u"1"_ustr);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testPlotVisOnlyXLSX)
{
    loadFromFile(u"xlsx/hidden_cells.xlsx");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotVisOnly"_ostr, "val"_ostr, u"0"_ustr);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testBarChartVaryColorsXLSX)
{
    loadFromFile(u"xlsx/tdf90876.xlsx");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:varyColors"_ostr, "val"_ostr, u"0"_ustr);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testTdf96161)
{
    loadFromFile(u"ods/tdf96161.ods");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:varyColors"_ostr, "val"_ostr, u"0"_ustr);
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

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:scatterChart"_ostr, 2);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:scatterChart[1]/c:ser"_ostr, 1);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:scatterChart[2]/c:ser"_ostr, 1);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx"_ostr, 4);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx/c:delete[@val='1']"_ostr, 1);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx/c:axPos[@val='l']"_ostr, 1);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx/c:axPos[@val='r']"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testSecondaryAxisXLSX)
{
    loadFromFile(u"ods/secondary_axis.ods");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart"_ostr, 2);
    // test there is just those series in the first <lineChart> tag which are attached to the primary axis
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart[1]/c:ser"_ostr, 2);
    assertXPathContent(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart[1]/c:ser[1]/c:tx/c:strRef/c:strCache/c:pt/c:v"_ostr, u"b"_ustr);
    assertXPathContent(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart[1]/c:ser[2]/c:tx/c:strRef/c:strCache/c:pt/c:v"_ostr, u"c"_ustr);
    // test there is just those series in the second <lineChart> tag which are attached to the secondary axis
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart[2]/c:ser"_ostr, 1);
    assertXPathContent(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart[2]/c:ser[1]/c:tx/c:strRef/c:strCache/c:pt/c:v"_ostr, u"a"_ustr);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testBarChartSecondaryAxisXLSX)
{
    loadFromFile(u"xlsx/testSecondaryAxis.xlsx");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    // Collect barchart axID on primary Axis
    OUString XValueIdOf1Barchart = getXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart[1]/c:axId[1]"_ostr, "val"_ostr);
    OUString YValueIdOf1Barchart = getXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart[1]/c:axId[2]"_ostr, "val"_ostr);
    // Collect barchart axID on secondary Axis
    OUString XValueIdOf2Barchart = getXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart[2]/c:axId[1]"_ostr, "val"_ostr);
    OUString YValueIdOf2Barchart = getXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart[2]/c:axId[2]"_ostr, "val"_ostr);
    // Check which c:catAx and c:valAx contain the AxisId of barcharts
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:catAx[1]/c:axId"_ostr, "val"_ostr, XValueIdOf1Barchart);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx[1]/c:axId"_ostr, "val"_ostr, YValueIdOf1Barchart);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:catAx[2]/c:axId"_ostr, "val"_ostr, XValueIdOf2Barchart);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx[2]/c:axId"_ostr, "val"_ostr, YValueIdOf2Barchart);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest3, testTdf148142)
{
    // The document contains a line chart with "Between tick marks" X axis position.
    loadFromFile(u"ods/tdf148142.ods");
    Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
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
    Reference<chart2::XChartDocument> xChartDoc2 = getChartDocFromSheet(0, mxComponent);
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
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:r[1]/a:rPr"_ostr, "b"_ostr, u"1"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:r[1]/a:rPr"_ostr, "sz"_ostr, u"1400"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:r[1]/a:rPr/a:solidFill/a:srgbClr"_ostr, "val"_ostr, u"ff0000"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:r[1]/a:rPr/a:latin"_ostr, "typeface"_ostr, u"Aptos Narrow"_ustr);
    assertXPathContent(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:r[1]/a:t"_ostr, u"This"_ustr);
    // Check run level properties [2]
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:r[2]/a:rPr"_ostr, "b"_ostr, u"0"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:r[2]/a:rPr"_ostr, "sz"_ostr, u"1400"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:r[2]/a:rPr/a:solidFill/a:srgbClr"_ostr, "val"_ostr, u"595959"_ustr);
    assertXPathContent(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:r[2]/a:t"_ostr, u" is"_ustr);
    // Check run level properties [3]
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:r[3]/a:rPr"_ostr, "b"_ostr, u"0"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:r[3]/a:rPr"_ostr, "sz"_ostr, u"1400"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:r[3]/a:rPr"_ostr, "baseline"_ostr, u"30000"_ustr);
    assertXPathContent(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:r[3]/a:t"_ostr, u"3"_ustr);
    // Check run level properties [4]
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:r[4]/a:rPr"_ostr, "b"_ostr, u"0"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:r[4]/a:rPr"_ostr, "sz"_ostr, u"1400"_ustr);
    assertXPathContent(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:r[4]/a:t"_ostr, u" a "_ustr);
    // Check run level properties [5]
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:r[5]/a:rPr"_ostr, "b"_ostr, u"0"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:r[5]/a:rPr"_ostr, "i"_ostr, u"1"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:r[5]/a:rPr"_ostr, "sz"_ostr, u"2000"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:r[5]/a:rPr"_ostr, "u"_ostr, u"sng"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:r[5]/a:rPr/a:solidFill/a:srgbClr"_ostr, "val"_ostr, u"4ea72e"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:r[5]/a:rPr/a:uFillTx"_ostr, 1);
    assertXPathContent(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:r[5]/a:t"_ostr, u"custom"_ustr);
    // Check run level properties [6]
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:r[6]/a:rPr"_ostr, "b"_ostr, u"0"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:r[6]/a:rPr"_ostr, "sz"_ostr, u"1400"_ustr);
    assertXPathContent(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[1]/a:r[6]/a:t"_ostr, u" erte1"_ustr);
    // Check run level properties [1] - second paragraph
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[2]/a:r[1]/a:rPr"_ostr, "b"_ostr, u"0"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[2]/a:r[1]/a:rPr"_ostr, "sz"_ostr, u"1400"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[2]/a:r[1]/a:rPr/a:solidFill/a:srgbClr"_ostr, "val"_ostr, u"595959"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[2]/a:r[1]/a:rPr/a:latin"_ostr, "typeface"_ostr, u"Aptos Narrow"_ustr);
    assertXPathContent(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[2]/a:r[1]/a:t"_ostr, u"2dfgd ch"_ustr);
    // Check run level properties [2]
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[2]/a:r[2]/a:rPr"_ostr, "b"_ostr, u"1"_ustr);
    assertXPathContent(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[2]/a:r[2]/a:t"_ostr, u"ar"_ustr);
    // Check run level properties [3]
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[2]/a:r[3]/a:rPr"_ostr, "b"_ostr, u"0"_ustr);;
    assertXPathContent(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[2]/a:r[3]/a:t"_ostr, u"t "_ustr);
    // Check run level properties [4]
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[2]/a:r[4]/a:rPr"_ostr, "b"_ostr, u"0"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[2]/a:r[4]/a:rPr"_ostr, "strike"_ostr, u"sngStrike"_ustr);
    assertXPathContent(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p[2]/a:r[4]/a:t"_ostr, u"title"_ustr);
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

    Reference<chart2::XChartDocument> xChart2Doc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT(xChart2Doc.is());
    Reference< chart::XChartDocument > xChartDoc(xChart2Doc, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());
    uno::Reference< beans::XPropertySet > xTitleProp(xChartDoc->getTitle(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xTitleProp.is());

    checkCharacterProps(xTitleProp);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
