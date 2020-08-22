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
#include <com/sun/star/chart2/XRegressionCurveContainer.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/packages/zip/ZipFileAccess.hpp>

#include <unotools/ucbstreamhelper.hxx>

#include <libxml/xpathInternals.h>

#include <algorithm>

using uno::Reference;
using beans::XPropertySet;

class Chart2GeometryTest : public ChartTest, public XmlTestTools
{
protected:
    virtual void registerNamespaces(xmlXPathContextPtr& pXmlXPathCtx) override;

public:
    Chart2GeometryTest()
        : ChartTest()
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

    CPPUNIT_TEST_SUITE_END();

protected:
    /**
     * Given that some problem doesn't affect the result in the importer, we
     * test the resulting file directly, by opening the zip file, parsing an
     * xml stream, and asserting an XPath expression. This method returns the
     * xml stream, so that you can do the asserting.
     */
    xmlDocUniquePtr parseExport(const OUString& rDir, const OUString& rFilterFormat);
};

namespace
{
// This is copied from Chart2ExportTest. It allows to access the chart from a MS Office document
// without knowing whether the file is a chart1.xml or chart2.xml... As of August 2020, Calc
// and Impress use a static variable for the number and therefore the number depends on whether
// there had already been savings before.
struct CheckForChartName
{
private:
    OUString aDir;

public:
    explicit CheckForChartName(const OUString& rDir)
        : aDir(rDir)
    {
    }

    bool operator()(const OUString& rName)
    {
        if (!rName.startsWith(aDir))
            return false;

        if (!rName.endsWith(".xml"))
            return false;

        return true;
    }
};

OUString findChartFile(const OUString& rDir, uno::Reference<container::XNameAccess> const& xNames)
{
    uno::Sequence<OUString> aNames = xNames->getElementNames();
    OUString* pElement = std::find_if(aNames.begin(), aNames.end(), CheckForChartName(rDir));

    CPPUNIT_ASSERT(pElement != aNames.end());
    return *pElement;
}
}

xmlDocUniquePtr Chart2GeometryTest::parseExport(const OUString& rDir, const OUString& rFilterFormat)
{
    std::shared_ptr<utl::TempFile> pTempFile = save(rFilterFormat);

    // Read the XML stream we're interested in.
    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess
        = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(m_xSFactory),
                                                      pTempFile->GetURL());
    uno::Reference<io::XInputStream> xInputStream(
        xNameAccess->getByName(findChartFile(rDir, xNameAccess)), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xInputStream.is());
    std::unique_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(xInputStream, true));

    return parseXmlStream(pStream.get());
}

void Chart2GeometryTest::registerNamespaces(xmlXPathContextPtr& pXmlXPathCtx)
{
    static struct
    {
        char const* pPrefix;
        char const* pURI;
    } const aNamespaces[] = {
        // OOXML
        { "w", "http://schemas.openxmlformats.org/wordprocessingml/2006/main" },
        { "v", "urn:schemas-microsoft-com:vml" },
        { "c", "http://schemas.openxmlformats.org/drawingml/2006/chart" },
        { "a", "http://schemas.openxmlformats.org/drawingml/2006/main" },
        { "mc", "http://schemas.openxmlformats.org/markup-compatibility/2006" },
        { "wps", "http://schemas.microsoft.com/office/word/2010/wordprocessingShape" },
        { "wpg", "http://schemas.microsoft.com/office/word/2010/wordprocessingGroup" },
        { "wp", "http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing" },
        { "c15", "http://schemas.microsoft.com/office/drawing/2012/chart" },
        // ODF
        { "office", "urn:oasis:names:tc:opendocument:xmlns:office:1.0" },
        { "chart", "urn:oasis:names:tc:opendocument:xmlns:chart:1.0" },
        { "draw", "urn:oasis:names:tc:opendocument:xmlns:drawing:1.0" },
        { "style", "urn:oasis:names:tc:opendocument:xmlns:style:1.0" },
        { "svg", "urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0" },
        { "table", "urn:oasis:names:tc:opendocument:xmlns:table:1.0" },
        { "text", "urn:oasis:names:tc:opendocument:xmlns:text:1.0" },
        { "xlink", "http://www.w3c.org/1999/xlink" },
    };
    for (size_t i = 0; i < SAL_N_ELEMENTS(aNamespaces); ++i)
    {
        xmlXPathRegisterNs(pXmlXPathCtx, reinterpret_cast<xmlChar const*>(aNamespaces[i].pPrefix),
                           reinterpret_cast<xmlChar const*>(aNamespaces[i].pURI));
    }
}

static OString OU2O(const OUString& sOUSource)
{
    return rtl::OUStringToOString(sOUSource, RTL_TEXTENCODING_UTF8);
}

// Without the patch for tdf#135184, charts were not able to use linecap at all.
// These two tests verify, that round linecaps in the xlsx file are saved in ods.
void Chart2GeometryTest::testTdf135184RoundLineCap()
{
    // It tests chart area, data series line and regression-curve line.
    load("/chart2/qa/extras/data/xlsx/", "tdf135184RoundLineCap.xlsx");
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
    load("/chart2/qa/extras/data/xlsx/", "tdf135184RoundLineCap2.xlsx");
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
    load("/chart2/qa/extras/data/xlsx/", "tdf135184RoundLineCap.xlsx");
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
    load("/chart2/qa/extras/data/xlsx/", "tdf135184RoundLineCap2.xlsx");
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
    load("/chart2/qa/extras/data/odp/", "tdf128345_ChartArea_CG_TS.odp");

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
    load("/chart2/qa/extras/data/pptx/", "tdf128345_ChartArea_CG_TS.pptx");

    // Find transparency gradient name
    xmlDocUniquePtr pXmlDoc = parseExport("Object 1/content.xml", "impress8");
    CPPUNIT_ASSERT(pXmlDoc);
    const OString sChartPath(
        "//office:document-content/office:body/office:chart/chart:chart/@chart:style-name");
    const OUString sOUChartStyleName = getXPathContent(pXmlDoc, sChartPath);
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
    const OString sOpacityPath("//office:document-styles/office:styles/draw:opacity");
    const OString sAttribute("@draw:name='" + OU2O(sOUOpacityName) + "'");
    const OString sStart(sOpacityPath + "[" + sAttribute);
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
    load("/chart2/qa/extras/data/odp/", "tdf128345_ChartWall_CS_TG.odp");

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
    load("/chart2/qa/extras/data/pptx/", "tdf128345_ChartWall_CS_TG.pptx");

    // Find transparency gradient name
    xmlDocUniquePtr pXmlDoc = parseExport("Object 1/content.xml", "impress8");
    CPPUNIT_ASSERT(pXmlDoc);
    const OString sChartPath("//office:document-content/office:body/office:chart/chart:chart/"
                             "chart:plot-area/chart:wall/@chart:style-name");
    const OUString sOUChartStyleName = getXPathContent(pXmlDoc, sChartPath);
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
    const OString sOpacityPath("//office:document-styles/office:styles/draw:opacity");
    const OString sAttribute("@draw:name='" + OU2O(sOUOpacityName) + "'");
    const OString sStart(sOpacityPath + "[" + sAttribute);
    assertXPath(pXmlDoc2, sStart + "]", 1);
    assertXPath(pXmlDoc2, sStart + " and @draw:style='linear']");
    assertXPath(pXmlDoc2, sStart + " and @draw:start='0%']");
    assertXPath(pXmlDoc2, sStart + " and @draw:end='100%']");
}

void Chart2GeometryTest::testTdf128345Legend_CS_TG_axial_export()
{
    // legend with solid color and transparency gradient
    // Without the patch the transparency was lost.
    load("/chart2/qa/extras/data/odp/", "tdf128345_Legend_CS_TG_axial.odp");

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
    load("/chart2/qa/extras/data/pptx/", "tdf128345_Legend_CS_TG_axial.pptx");

    // Find transparency gradient name
    xmlDocUniquePtr pXmlDoc = parseExport("Object 1/content.xml", "impress8");
    CPPUNIT_ASSERT(pXmlDoc);
    const OString sChartPath("//office:document-content/office:body/office:chart/chart:chart/"
                             "chart:legend/@chart:style-name");
    const OUString sOUChartStyleName = getXPathContent(pXmlDoc, sChartPath);
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
    const OString sOpacityPath("//office:document-styles/office:styles/draw:opacity");
    const OString sAttribute("@draw:name='" + OU2O(sOUOpacityName) + "'");
    const OString sStart(sOpacityPath + "[" + sAttribute);
    assertXPath(pXmlDoc2, sStart + "]", 1);
    assertXPath(pXmlDoc2, sStart + " and @draw:style='axial']");
    assertXPath(pXmlDoc2, sStart + " and @draw:start='0%']");
    assertXPath(pXmlDoc2, sStart + " and @draw:end='100%']");
}

void Chart2GeometryTest::testTdf135366LabelOnSeries()
{
    // Error was, that the fill and line properties of a <chart:data-label> were not
    // imported at all. Here they should be at the series.
    load("/chart2/qa/extras/data/ods/", "tdf135366_data_label_series.ods");
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
    load("/chart2/qa/extras/data/odt/", "tdf135366_data_label_point.odt");
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

CPPUNIT_TEST_SUITE_REGISTRATION(Chart2GeometryTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
