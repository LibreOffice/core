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

#include <com/sun/star/chart2/XRegressionCurveContainer.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/packages/zip/ZipFileAccess.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>

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
    void testTdf135184RoundLineCap();
    void testTdf135184RoundLineCap2();
    void testTdf135184RoundLineCap3();
    void testTdf135184RoundLineCap4();

    CPPUNIT_TEST_SUITE(Chart2GeometryTest);
    CPPUNIT_TEST(testTdf135184RoundLineCap);
    CPPUNIT_TEST(testTdf135184RoundLineCap2);
    CPPUNIT_TEST(testTdf135184RoundLineCap3);
    CPPUNIT_TEST(testTdf135184RoundLineCap4);

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
    } const aNamespaces[]
        = { { "w", "http://schemas.openxmlformats.org/wordprocessingml/2006/main" },
            { "v", "urn:schemas-microsoft-com:vml" },
            { "c", "http://schemas.openxmlformats.org/drawingml/2006/chart" },
            { "a", "http://schemas.openxmlformats.org/drawingml/2006/main" },
            { "mc", "http://schemas.openxmlformats.org/markup-compatibility/2006" },
            { "wps", "http://schemas.microsoft.com/office/word/2010/wordprocessingShape" },
            { "wpg", "http://schemas.microsoft.com/office/word/2010/wordprocessingGroup" },
            { "wp", "http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing" },
            { "c15", "http://schemas.microsoft.com/office/drawing/2012/chart" },
            { "office", "urn:oasis:names:tc:opendocument:xmlns:office:1.0" },
            { "chart", "urn:oasis:names:tc:opendocument:xmlns:chart:1.0" },
            { "style", "urn:oasis:names:tc:opendocument:xmlns:style:1.0" },
            { "svg", "urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0" },
            { "table", "urn:oasis:names:tc:opendocument:xmlns:table:1.0" },
            { "text", "urn:oasis:names:tc:opendocument:xmlns:text:1.0" },
            { "xlink", "http://www.w3c.org/1999/xlink" },
            { "c15", "http://schemas.microsoft.com/office/drawing/2012/chart" } };
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

CPPUNIT_TEST_SUITE_REGISTRATION(Chart2GeometryTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
