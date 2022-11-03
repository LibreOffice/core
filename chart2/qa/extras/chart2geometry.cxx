/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "charttest.hxx"

#include <test/xmltesttools.hxx>

#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XDataSeries.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>

#include <unotools/saveopt.hxx>

#include <string_view>

using uno::Reference;
using beans::XPropertySet;

class Chart2GeometryTest : public ChartTest
{
protected:
    virtual void registerNamespaces(xmlXPathContextPtr& pXmlXPathCtx) override;

public:
    Chart2GeometryTest()
        : ChartTest("/chart2/qa/extras/data/")
    {
    }
    // Mostly tests for line and fill properties
    void testTdf135184RoundLineCap();
    void testTdf135184RoundLineCap2();
    void testTdf135184RoundLineCap3();
    void testTdf135184RoundLineCap4();
    void testTdf128345ChartArea_CG_TS_export();
    void testTdf128345ChartArea_CG_TS_import();
    void testTdf128345ChartWall_CS_TG_export();
    void testTdf128345ChartWall_CS_TG_import();
    void testTdf128345Legend_CS_TG_axial_export();
    void testTdf128345Legend_CS_TG_axial_import();
    void testTdf135366LabelOnSeries();
    void testTdf135366LabelOnPoint();
    void testTdf135366LabelExport();
    void testTdf135366_CustomLabelText();

    CPPUNIT_TEST_SUITE(Chart2GeometryTest);
    CPPUNIT_TEST(testTdf135184RoundLineCap);
    CPPUNIT_TEST(testTdf135184RoundLineCap2);
    CPPUNIT_TEST(testTdf135184RoundLineCap3);
    CPPUNIT_TEST(testTdf135184RoundLineCap4);
    CPPUNIT_TEST(testTdf128345ChartArea_CG_TS_export);
    CPPUNIT_TEST(testTdf128345ChartArea_CG_TS_import);
    CPPUNIT_TEST(testTdf128345ChartWall_CS_TG_export);
    CPPUNIT_TEST(testTdf128345ChartWall_CS_TG_import);
    CPPUNIT_TEST(testTdf128345Legend_CS_TG_axial_export);
    CPPUNIT_TEST(testTdf128345Legend_CS_TG_axial_import);
    CPPUNIT_TEST(testTdf135366LabelOnSeries);
    CPPUNIT_TEST(testTdf135366LabelOnPoint);
    CPPUNIT_TEST(testTdf135366LabelExport);
    CPPUNIT_TEST(testTdf135366_CustomLabelText);

    CPPUNIT_TEST_SUITE_END();
};

void Chart2GeometryTest::registerNamespaces(xmlXPathContextPtr& pXmlXPathCtx)
{
    XmlTestTools::registerOOXMLNamespaces(pXmlXPathCtx);
    XmlTestTools::registerODFNamespaces(pXmlXPathCtx);
}

static OString OU2O(std::u16string_view sOUSource)
{
    return rtl::OUStringToOString(sOUSource, RTL_TEXTENCODING_UTF8);
}

// Without the patch for tdf#135184, charts were not able to use linecap at all.
// These two tests verify, that round linecaps in the xlsx file are saved in ods.
void Chart2GeometryTest::testTdf135184RoundLineCap()
{
    // It tests chart area, data series line and regression-curve line.
    loadFromURL(u"xlsx/tdf135184RoundLineCap.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("Object 1/content.xml", "calc8");
    CPPUNIT_ASSERT(pXmlDoc);

    const OString sStyleStart("/office:document-content/office:automatic-styles");
    const OString sCap("/style:graphic-properties[@svg:stroke-linecap='round']");
    const OString sChartStart("/office:document-content/office:body/office:chart/chart:chart");
    OString sPredicate;
    // chart area
    const OUString sOUAreaStyleName = getXPathContent(pXmlDoc, sChartStart + "/@chart:style-name");
    sPredicate = "[@style:name='" + OU2O(sOUAreaStyleName) + "']";
    assertXPath(pXmlDoc, sStyleStart + "/style:style" + sPredicate + sCap);
    // data series line
    const OString sSeries(sChartStart + "/chart:plot-area/chart:series");
    const OUString sOUSeriesStyleName = getXPathContent(pXmlDoc, sSeries + "/@chart:style-name");
    sPredicate = "[@style:name='" + OU2O(sOUSeriesStyleName) + "']";
    assertXPath(pXmlDoc, sStyleStart + "/style:style" + sPredicate + sCap);
    // regression-curve (trend line)
    const OString sTrend(sChartStart + "/chart:plot-area/chart:series/chart:regression-curve");
    const OUString sOUTrendStyleName = getXPathContent(pXmlDoc, sTrend + "/@chart:style-name");
    sPredicate = "[@style:name='" + OU2O(sOUTrendStyleName) + "']";
    assertXPath(pXmlDoc, sStyleStart + "/style:style" + sPredicate + sCap);
}

void Chart2GeometryTest::testTdf135184RoundLineCap2()
{
    // It tests legend, data series sector and title.
    loadFromURL(u"xlsx/tdf135184RoundLineCap2.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("Object 1/content.xml", "calc8");
    CPPUNIT_ASSERT(pXmlDoc);

    const OString sStyleStart("/office:document-content/office:automatic-styles");
    const OString sCap("/style:graphic-properties[@svg:stroke-linecap='round']");
    const OString sChartStart("/office:document-content/office:body/office:chart/chart:chart");
    OString sPredicate;
    // legend
    const OString sLegend(sChartStart + "/chart:legend");
    const OUString sOULegendStyleName = getXPathContent(pXmlDoc, sLegend + "/@chart:style-name");
    sPredicate = "[@style:name='" + OU2O(sOULegendStyleName) + "']";
    assertXPath(pXmlDoc, sStyleStart + "/style:style" + sPredicate + sCap);
    // title
    const OString sTitle(sChartStart + "/chart:title");
    const OUString sOUTitleStyleName = getXPathContent(pXmlDoc, sTitle + "/@chart:style-name");
    sPredicate = "[@style:name='" + OU2O(sOUTitleStyleName) + "']";
    assertXPath(pXmlDoc, sStyleStart + "/style:style" + sPredicate + sCap);
    // sector
    const OString sSector(sChartStart + "/chart:plot-area/chart:series/chart:data-point[3]");
    const OUString sOUSectorStyleName = getXPathContent(pXmlDoc, sSector + "/@chart:style-name");
    sPredicate = "[@style:name='" + OU2O(sOUSectorStyleName) + "']";
    assertXPath(pXmlDoc, sStyleStart + "/style:style" + sPredicate + sCap);
}

// These two tests verify the round-trip of preset dash styles in the xlsx file.
void Chart2GeometryTest::testTdf135184RoundLineCap3()
{
    // It tests chart area, data series line and regression-curve line.
    loadFromURL(u"xlsx/tdf135184RoundLineCap.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);

    const OString sDash("/c:spPr/a:ln/a:prstDash");
    // chart area
    assertXPath(pXmlDoc, "/c:chartSpace" + sDash, "val", "dashDot");
    // data series line
    const OString sStart("/c:chartSpace/c:chart/c:plotArea/c:scatterChart/c:ser");
    assertXPath(pXmlDoc, sStart + sDash, "val", "dash");
    // regression-curve (trendline)
    assertXPath(pXmlDoc, sStart + "/c:trendline" + sDash, "val", "sysDot");
}

void Chart2GeometryTest::testTdf135184RoundLineCap4()
{
    // It tests legend, data series sector and title.
    loadFromURL(u"xlsx/tdf135184RoundLineCap2.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);

    const OString sChartStart("/c:chartSpace/c:chart");
    const OString sDash("/c:spPr/a:ln/a:prstDash");
    assertXPath(pXmlDoc, sChartStart + "/c:legend" + sDash, "val", "sysDot");
    const OString sSeries(sChartStart + "/c:plotArea/c:pieChart/c:ser/c:dPt[3]");
    assertXPath(pXmlDoc, sSeries + sDash, "val", "dash");
    assertXPath(pXmlDoc, sChartStart + "/c:title" + sDash, "val", "dashDot");
}

void Chart2GeometryTest::testTdf128345ChartArea_CG_TS_export()
{
    // chart area with color gradient and solid transparency
    // Without the patch the transparency was lost in saved pptx file.
    loadFromURL(u"odp/tdf128345_ChartArea_CG_TS.odp");

    // Make sure the chart area has a transparency in gradient stops in saved pptx file.
    xmlDocUniquePtr pXmlDoc = parseExport("ppt/charts/chart", "Impress MS PowerPoint 2007 XML");
    CPPUNIT_ASSERT(pXmlDoc);

    OString sPathStart("//c:chartSpace/c:spPr/a:gradFill");
    assertXPath(pXmlDoc, sPathStart + "/a:gsLst/a:gs", 2);
    assertXPath(pXmlDoc, sPathStart + "/a:gsLst/a:gs[1]/a:srgbClr/a:alpha", "val", "30000");
    assertXPath(pXmlDoc, sPathStart + "/a:gsLst/a:gs[2]/a:srgbClr/a:alpha", "val", "30000");
}

void Chart2GeometryTest::testTdf128345ChartArea_CG_TS_import()
{
    // This works on the file, which was exported from file tdf128345_ChartArea_CG_TS.odp to pptx.
    // It has gradient stops with identical transparency values.
    // Make sure chart area has transparency when pptx document is opened and resaved as odp.
    // As of Aug 2020, the import generates a transparency gradient. When import is changed to
    // generate solid transparency, the test needs to be adapted.
    loadFromURL(u"pptx/tdf128345_ChartArea_CG_TS.pptx");

    // Find transparency gradient name
    xmlDocUniquePtr pXmlDoc = parseExport("Object 1/content.xml", "impress8");
    CPPUNIT_ASSERT(pXmlDoc);
    const OUString sOUChartStyleName = getXPathContent(
        pXmlDoc,
        "//office:document-content/office:body/office:chart/chart:chart/@chart:style-name");
    const OString sStylePath(
        "//office:document-content/office:automatic-styles/style:style[@style:name='"
        + OU2O(sOUChartStyleName) + "']");
    assertXPath(pXmlDoc, sStylePath, 1);
    assertXPath(pXmlDoc, sStylePath + "/style:graphic-properties/@draw:opacity-name", 1);
    const OUString sOUOpacityName
        = getXPathContent(pXmlDoc, sStylePath + "/style:graphic-properties/@draw:opacity-name");

    // Verify the content of the opacity definition
    xmlDocUniquePtr pXmlDoc2 = parseExport("Object 1/styles.xml", "impress8");
    CPPUNIT_ASSERT(pXmlDoc2);
    const OString sAttribute("@draw:name='" + OU2O(sOUOpacityName) + "'");
    const OString sStart("//office:document-styles/office:styles/draw:opacity[" + sAttribute);
    assertXPath(pXmlDoc2, sStart + "]", 1);
    assertXPath(pXmlDoc2, sStart + " and @draw:style='linear']");
    assertXPath(pXmlDoc2, sStart + " and @draw:start='30%']");
    assertXPath(pXmlDoc2, sStart + " and @draw:end='30%']");
    assertXPath(pXmlDoc2, sStart + " and @draw:angle='30deg']");
    assertXPath(pXmlDoc2, sStart + " and @draw:border='20%']");
}

void Chart2GeometryTest::testTdf128345ChartWall_CS_TG_export()
{
    // chart wall with solid color and transparency gradient
    // Without the patch the transparency was lost.
    loadFromURL(u"odp/tdf128345_ChartWall_CS_TG.odp");

    // Make sure the chart has a gradient with transparency in gradient stops in saved pptx file.
    xmlDocUniquePtr pXmlDoc = parseExport("ppt/charts/chart", "Impress MS PowerPoint 2007 XML");
    CPPUNIT_ASSERT(pXmlDoc);

    OString sPathStart("//c:chartSpace/c:chart/c:plotArea/c:spPr/a:gradFill");
    assertXPath(pXmlDoc, sPathStart + "/a:gsLst/a:gs", 2); //linear
    // MS Office has opacity, so 100% transparency is val="0"
    assertXPath(pXmlDoc, sPathStart + "/a:gsLst/a:gs[1]/a:srgbClr/a:alpha", "val", "0");
    // no element for 0% transparent
    assertXPath(pXmlDoc, sPathStart + "/a:gsLst/a:gs[2]/a:srgbClr/a:alpha", 0);
}

void Chart2GeometryTest::testTdf128345ChartWall_CS_TG_import()
{
    // This works on the file, which was exported from file tdf128345_ChartWall_CS_TG.odp to pptx.
    // Make sure chart wall has transparency when pptx document is resaved as odp.
    loadFromURL(u"pptx/tdf128345_ChartWall_CS_TG.pptx");

    // Find transparency gradient name
    xmlDocUniquePtr pXmlDoc = parseExport("Object 1/content.xml", "impress8");
    CPPUNIT_ASSERT(pXmlDoc);
    const OUString sOUChartStyleName
        = getXPathContent(pXmlDoc, "//office:document-content/office:body/office:chart/chart:chart/"
                                   "chart:plot-area/chart:wall/@chart:style-name");
    const OString sStylePath(
        "//office:document-content/office:automatic-styles/style:style[@style:name='"
        + OU2O(sOUChartStyleName) + "']");
    assertXPath(pXmlDoc, sStylePath, 1);
    assertXPath(pXmlDoc, sStylePath + "/style:graphic-properties/@draw:opacity-name", 1);
    const OUString sOUOpacityName
        = getXPathContent(pXmlDoc, sStylePath + "/style:graphic-properties/@draw:opacity-name");

    // Verify content of the opacity definition
    xmlDocUniquePtr pXmlDoc2 = parseExport("Object 1/styles.xml", "impress8");
    CPPUNIT_ASSERT(pXmlDoc2);
    const OString sAttribute("@draw:name='" + OU2O(sOUOpacityName) + "'");
    const OString sStart("//office:document-styles/office:styles/draw:opacity[" + sAttribute);
    assertXPath(pXmlDoc2, sStart + "]", 1);
    assertXPath(pXmlDoc2, sStart + " and @draw:style='linear']");
    assertXPath(pXmlDoc2, sStart + " and @draw:start='0%']");
    assertXPath(pXmlDoc2, sStart + " and @draw:end='100%']");
}

void Chart2GeometryTest::testTdf128345Legend_CS_TG_axial_export()
{
    // legend with solid color and transparency gradient
    // Without the patch the transparency was lost.
    loadFromURL(u"odp/tdf128345_Legend_CS_TG_axial.odp");

    // Make sure the chart has a gradient with transparency in gradient stops in saved pptx file.
    xmlDocUniquePtr pXmlDoc = parseExport("ppt/charts/chart", "Impress MS PowerPoint 2007 XML");
    CPPUNIT_ASSERT(pXmlDoc);

    OString sPathStart("//c:chartSpace/c:chart/c:legend/c:spPr/a:gradFill");
    assertXPath(pXmlDoc, sPathStart + "/a:gsLst/a:gs", 3); // axial
    // no element for 0% transparent
    assertXPath(pXmlDoc, sPathStart + "/a:gsLst/a:gs[1]/a:srgbClr/a:alpha", 0);
    // 100% transparent = opacity 0. It comes from "axial", therefore it is in the middle.
    assertXPath(pXmlDoc, sPathStart + "/a:gsLst/a:gs[2]/a:srgbClr/a:alpha", "val", "0");
    assertXPath(pXmlDoc, sPathStart + "/a:gsLst/a:gs[2]", "pos", "50000");
    assertXPath(pXmlDoc, sPathStart + "/a:gsLst/a:gs[3]/a:srgbClr/a:alpha", 0);
}

void Chart2GeometryTest::testTdf128345Legend_CS_TG_axial_import()
{
    // This works on the file, which was exported from file tdf128345_Legend_CS_TG_axial.odp to pptx.
    // Error was, that in case of axial not the middle value was taken but start and end value.
    loadFromURL(u"pptx/tdf128345_Legend_CS_TG_axial.pptx");

    // Find transparency gradient name
    xmlDocUniquePtr pXmlDoc = parseExport("Object 1/content.xml", "impress8");
    CPPUNIT_ASSERT(pXmlDoc);
    const OUString sOUChartStyleName
        = getXPathContent(pXmlDoc, "//office:document-content/office:body/office:chart/chart:chart/"
                                   "chart:legend/@chart:style-name");
    const OString sStylePath(
        "//office:document-content/office:automatic-styles/style:style[@style:name='"
        + OU2O(sOUChartStyleName) + "']");
    assertXPath(pXmlDoc, sStylePath, 1);
    assertXPath(pXmlDoc, sStylePath + "/style:graphic-properties/@draw:opacity-name", 1);
    const OUString sOUOpacityName
        = getXPathContent(pXmlDoc, sStylePath + "/style:graphic-properties/@draw:opacity-name");

    // Verify content of the opacity definition
    xmlDocUniquePtr pXmlDoc2 = parseExport("Object 1/styles.xml", "impress8");
    CPPUNIT_ASSERT(pXmlDoc2);
    const OString sAttribute("@draw:name='" + OU2O(sOUOpacityName) + "'");
    const OString sStart("//office:document-styles/office:styles/draw:opacity[" + sAttribute);
    assertXPath(pXmlDoc2, sStart + "]", 1);
    assertXPath(pXmlDoc2, sStart + " and @draw:style='axial']");
    assertXPath(pXmlDoc2, sStart + " and @draw:start='0%']");
    assertXPath(pXmlDoc2, sStart + " and @draw:end='100%']");
}

void Chart2GeometryTest::testTdf135366LabelOnSeries()
{
    // Error was, that the fill and line properties of a <chart:data-label> were not
    // imported at all. Here they should be at the series.
    loadFromURL(u"ods/tdf135366_data_label_series.ods");
    uno::Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT(xChartDoc.is());
    Reference<chart2::XDataSeries> xDataSeries = getDataSeriesFromDoc(xChartDoc, 0);
    CPPUNIT_ASSERT(xDataSeries.is());
    Reference<beans::XPropertySet> xPropSet(xDataSeries, UNO_QUERY_THROW);
    uno::Any aAny;

    aAny = xPropSet->getPropertyValue("LabelBorderStyle");
    drawing::LineStyle eLineStyle;
    CPPUNIT_ASSERT_MESSAGE("No LabelBorderStyle set.", aAny >>= eLineStyle);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("solid line expected", drawing::LineStyle_SOLID, eLineStyle);

    sal_Int32 nBorderWidth;
    sal_Int32 nExpectedWidth = 95;
    xPropSet->getPropertyValue("LabelBorderWidth") >>= nBorderWidth;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("LabelBorderWidth", nExpectedWidth, nBorderWidth);

    sal_Int32 nLineColor;
    sal_Int32 nExpectedLineColor = 255;
    xPropSet->getPropertyValue("LabelBorderColor") >>= nLineColor;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("line color blue, 255=#0000FF", nExpectedLineColor, nLineColor);

    aAny = xPropSet->getPropertyValue("LabelFillStyle");
    drawing::FillStyle eFillStyle;
    CPPUNIT_ASSERT_MESSAGE("No LabelFillStyle set", aAny >>= eFillStyle);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("solid fill expected", drawing::FillStyle_SOLID, eFillStyle);

    sal_Int32 nFillColor;
    sal_Int32 nExpectedFillColor = 65280;
    xPropSet->getPropertyValue("LabelFillColor") >>= nFillColor;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("fill color green, 65280=#00FF00", nExpectedFillColor, nFillColor);
}

void Chart2GeometryTest::testTdf135366LabelOnPoint()
{
    // Error was, that the fill and line properties of a <chart:data-label> were not
    // imported at all. Here they should be at point 2.
    loadFromURL(u"odt/tdf135366_data_label_point.odt");
    uno::Reference<chart2::XChartDocument> xChartDoc(getChartDocFromWriter(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());
    Reference<chart2::XDataSeries> xDataSeries = getDataSeriesFromDoc(xChartDoc, 0);
    CPPUNIT_ASSERT(xDataSeries.is());
    Reference<beans::XPropertySet> xPropSet(xDataSeries->getDataPointByIndex(2),
                                            uno::UNO_SET_THROW);
    uno::Any aAny;

    aAny = xPropSet->getPropertyValue("LabelBorderStyle");
    drawing::LineStyle eLineStyle;
    CPPUNIT_ASSERT_MESSAGE("No LabelBorderStyle set.", aAny >>= eLineStyle);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("solid line expected", drawing::LineStyle_SOLID, eLineStyle);

    sal_Int32 nBorderWidth;
    sal_Int32 nExpectedWidth = 381;
    xPropSet->getPropertyValue("LabelBorderWidth") >>= nBorderWidth;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("LabelBorderWidth", nExpectedWidth, nBorderWidth);

    sal_Int32 nLineTransparency;
    sal_Int32 nExpectedTransparency = 30;
    xPropSet->getPropertyValue("LabelBorderTransparency") >>= nLineTransparency;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("line transparency", nExpectedTransparency, nLineTransparency);

    sal_Int32 nLineColor;
    sal_Int32 nExpectedLineColor = 10206041;
    xPropSet->getPropertyValue("LabelBorderColor") >>= nLineColor;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("line color greenish, 10206041=#9BBB59", nExpectedLineColor,
                                 nLineColor);

    aAny = xPropSet->getPropertyValue("LabelFillStyle");
    drawing::FillStyle eFillStyle;
    CPPUNIT_ASSERT_MESSAGE("No LabelFillStyle set", aAny >>= eFillStyle);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("solid fill expected", drawing::FillStyle_SOLID, eFillStyle);

    sal_Int32 nFillColor;
    sal_Int32 nExpectedFillColor = 14277081;
    xPropSet->getPropertyValue("LabelFillColor") >>= nFillColor;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("fill color gray, 14277081=#d9d9d9", nExpectedFillColor,
                                 nFillColor);
}

void Chart2GeometryTest::testTdf135366LabelExport()
{
    // Error was, that line and fill properties were not exported as
    // graphic-properties of a <chart:data-label> element, but only
    // as loext chart-properties of the <chart:data-point> element.
    loadFromURL(u"odt/tdf135366_data_label_export.odt");

    // FIXME: Error: unexpected attribute "loext:label-stroke-color"
    skipValidation();

    xmlDocUniquePtr pXmlDoc = parseExport("Object 1/content.xml", "writer8");
    CPPUNIT_ASSERT(pXmlDoc);

    // Find label style
    const OUString sOULabelStyleName = getXPathContent(
        pXmlDoc, "//office:document-content/office:body/office:chart/chart:chart/chart:plot-area"
                 "/chart:series/chart:data-point[1]/chart:data-label/@chart:style-name");

    // Verify content of graphic properties of label style
    const OString sStylePath(
        "//office:document-content/office:automatic-styles/style:style[@style:name='"
        + OU2O(sOULabelStyleName) + "']/style:graphic-properties");
    assertXPath(pXmlDoc, sStylePath, 1);
    assertXPath(pXmlDoc, sStylePath + "[@draw:fill='solid']");
    assertXPath(pXmlDoc, sStylePath + "[@draw:fill-color='#5050a0']");
    assertXPath(pXmlDoc, sStylePath + "[@draw:stroke='solid']");
    assertXPath(pXmlDoc, sStylePath + "[@svg:stroke-width='0.254cm']");
    assertXPath(pXmlDoc, sStylePath + "[@svg:stroke-color='#00ffff']");
}

void Chart2GeometryTest::testTdf135366_CustomLabelText()
{
    // Error was, that custom text in a data label was only exported in ODF extended,
    // although the used <chart:data-label> element exists since ODF 1.2.
    const SvtSaveOptions::ODFDefaultVersion nCurrentODFVersion(GetODFDefaultVersion());
    SetODFDefaultVersion(SvtSaveOptions::ODFVER_012);
    loadFromURL(u"pptx/tdf135366_CustomLabelText.pptx");
    xmlDocUniquePtr pXmlDoc = parseExport("Object 1/content.xml", "impress8");
    CPPUNIT_ASSERT(pXmlDoc);

    // Find custom text. As of version 7.0 it is in a <text:span> element.
    const OString sCustomTextPath(
        "//office:document-content/office:body/office:chart/chart:chart/chart:plot-area"
        "/chart:series/chart:data-point[2]/chart:data-label/text:p/text:span");
    assertXPath(pXmlDoc, sCustomTextPath, 1);

    // Verify text content
    const OUString sOUTextContent = getXPathContent(pXmlDoc, sCustomTextPath);
    CPPUNIT_ASSERT_EQUAL(OUString("Custom"), sOUTextContent);

    SetODFDefaultVersion(nCurrentODFVersion);
}

CPPUNIT_TEST_SUITE_REGISTRATION(Chart2GeometryTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
