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
#include <com/sun/star/chart2/PieChartSubType.hpp>

using uno::Reference;
using beans::XPropertySet;

class Chart2ExportTest : public ChartTest
{
public:
    Chart2ExportTest() : ChartTest("/chart2/qa/extras/data/") {}
};

namespace {

void testErrorBar( Reference< XPropertySet > const & xErrorBar )
{
    sal_Int32 nErrorBarStyle;
    CPPUNIT_ASSERT(
        xErrorBar->getPropertyValue("ErrorBarStyle") >>= nErrorBarStyle);
    CPPUNIT_ASSERT_EQUAL(chart::ErrorBarStyle::RELATIVE, nErrorBarStyle);
    bool bShowPositive = bool(), bShowNegative = bool();
    CPPUNIT_ASSERT(
        xErrorBar->getPropertyValue("ShowPositiveError") >>= bShowPositive);
    CPPUNIT_ASSERT(bShowPositive);
    CPPUNIT_ASSERT(
        xErrorBar->getPropertyValue("ShowNegativeError") >>= bShowNegative);
    CPPUNIT_ASSERT(bShowNegative);
    double nVal = 0.0;
    CPPUNIT_ASSERT(xErrorBar->getPropertyValue("PositiveError") >>= nVal);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(10.0, nVal, 1e-10);
}

void checkCommonTrendline(
        Reference<chart2::XRegressionCurve> const & xCurve,
        double aExpectedExtrapolateForward, double aExpectedExtrapolateBackward,
        bool aExpectedForceIntercept, double aExpectedInterceptValue,
        bool aExpectedShowEquation, bool aExpectedR2, bool aExpectedMayHaveR2)
{
    Reference<XPropertySet> xProperties( xCurve , uno::UNO_QUERY );
    CPPUNIT_ASSERT(xProperties.is());

    double aExtrapolateForward = 0.0;
    CPPUNIT_ASSERT(xProperties->getPropertyValue("ExtrapolateForward") >>= aExtrapolateForward);
    CPPUNIT_ASSERT_EQUAL(aExpectedExtrapolateForward, aExtrapolateForward);

    double aExtrapolateBackward = 0.0;
    CPPUNIT_ASSERT(xProperties->getPropertyValue("ExtrapolateBackward") >>= aExtrapolateBackward);
    CPPUNIT_ASSERT_EQUAL(aExpectedExtrapolateBackward, aExtrapolateBackward);

    bool bForceIntercept = false;
    CPPUNIT_ASSERT(xProperties->getPropertyValue("ForceIntercept") >>= bForceIntercept);
    CPPUNIT_ASSERT_EQUAL(aExpectedForceIntercept, bForceIntercept);

    if (bForceIntercept)
    {
        double aInterceptValue = 0.0;
        CPPUNIT_ASSERT(xProperties->getPropertyValue("InterceptValue") >>= aInterceptValue);
        CPPUNIT_ASSERT_EQUAL(aExpectedInterceptValue, aInterceptValue);
    }

    Reference< XPropertySet > xEquationProperties( xCurve->getEquationProperties() );
    CPPUNIT_ASSERT(xEquationProperties.is());

    bool bShowEquation = false;
    CPPUNIT_ASSERT(xEquationProperties->getPropertyValue("ShowEquation") >>= bShowEquation);
    CPPUNIT_ASSERT_EQUAL(aExpectedShowEquation, bShowEquation);

    bool bShowCorrelationCoefficient = false;
    CPPUNIT_ASSERT(xEquationProperties->getPropertyValue("ShowCorrelationCoefficient") >>= bShowCorrelationCoefficient);
    CPPUNIT_ASSERT_EQUAL(aExpectedR2, bShowCorrelationCoefficient);

    bool bMayHaveR2 = false;
    CPPUNIT_ASSERT(xEquationProperties->getPropertyValue("MayHaveCorrelationCoefficient") >>= bMayHaveR2);
    CPPUNIT_ASSERT_EQUAL(aExpectedMayHaveR2, bMayHaveR2);
}

void checkNameAndType(Reference<XPropertySet> const & xProperties, const OUString& aExpectedName, const OUString& aExpectedServiceName)
{
    Reference< lang::XServiceName > xServiceName( xProperties, UNO_QUERY );
    CPPUNIT_ASSERT(xServiceName.is());

    OUString aServiceName = xServiceName->getServiceName();
    CPPUNIT_ASSERT_EQUAL(aExpectedServiceName, aServiceName);

    OUString aCurveName;
    CPPUNIT_ASSERT(xProperties->getPropertyValue("CurveName") >>= aCurveName);
    CPPUNIT_ASSERT_EQUAL(aExpectedName, aCurveName);
}

void checkLinearTrendline(
        Reference<chart2::XRegressionCurve> const & xCurve, const OUString& aExpectedName,
        double aExpectedExtrapolateForward, double aExpectedExtrapolateBackward,
        double aExpectedInterceptValue)
{
    Reference<XPropertySet> xProperties( xCurve , uno::UNO_QUERY );
    CPPUNIT_ASSERT(xProperties.is());

    checkNameAndType(xProperties, aExpectedName, "com.sun.star.chart2.LinearRegressionCurve");

    checkCommonTrendline(
        xCurve,
        aExpectedExtrapolateForward, aExpectedExtrapolateBackward,
        /*aExpectedForceIntercept*/false, aExpectedInterceptValue,
        /*aExpectedShowEquation*/true, /*aExpectedR2*/false, /*aExpectedMayHaveR2*/true);
}

void checkPolynomialTrendline(
        Reference<chart2::XRegressionCurve> const & xCurve, const OUString& aExpectedName,
        sal_Int32 aExpectedDegree,
        double aExpectedExtrapolateForward, double aExpectedExtrapolateBackward,
        double aExpectedInterceptValue)
{
    Reference<XPropertySet> xProperties( xCurve , uno::UNO_QUERY );
    CPPUNIT_ASSERT(xProperties.is());

    checkNameAndType(xProperties, aExpectedName, "com.sun.star.chart2.PolynomialRegressionCurve");

    sal_Int32 aDegree = 2;
    CPPUNIT_ASSERT(xProperties->getPropertyValue("PolynomialDegree") >>= aDegree);
    CPPUNIT_ASSERT_EQUAL(aExpectedDegree, aDegree);

    checkCommonTrendline(
        xCurve,
        aExpectedExtrapolateForward, aExpectedExtrapolateBackward,
        /*aExpectedForceIntercept*/true, aExpectedInterceptValue,
        /*aExpectedShowEquation*/true, /*aExpectedR2*/true, /*aExpectedMayHaveR2*/true);
}

void checkMovingAverageTrendline(
        Reference<chart2::XRegressionCurve> const & xCurve, const OUString& aExpectedName, sal_Int32 aExpectedPeriod)
{
    Reference<XPropertySet> xProperties( xCurve , uno::UNO_QUERY );
    CPPUNIT_ASSERT(xProperties.is());

    checkNameAndType(xProperties, aExpectedName, "com.sun.star.chart2.MovingAverageRegressionCurve");

    sal_Int32 aPeriod = 2;
    CPPUNIT_ASSERT(xProperties->getPropertyValue("MovingAveragePeriod") >>= aPeriod);
    CPPUNIT_ASSERT_EQUAL(aExpectedPeriod, aPeriod);

    checkCommonTrendline(
        xCurve,
        /*aExpectedExtrapolateForward*/0.0, /*aExpectedExtrapolateBackward*/0.0,
        /*aExpectedForceIntercept*/false, /*aExpectedInterceptValue*/0.0,
        /*aExpectedShowEquation*/false, /*aExpectedR2*/false, /*aExpectedMayHaveR2*/false);
}

void checkTrendlinesInChart(uno::Reference< chart2::XChartDocument > const & xChartDoc)
{
    CPPUNIT_ASSERT(xChartDoc.is());

    Reference< chart2::XDataSeries > xDataSeries = getDataSeriesFromDoc( xChartDoc, 0 );
    CPPUNIT_ASSERT( xDataSeries.is() );

    Reference< chart2::XRegressionCurveContainer > xRegressionCurveContainer( xDataSeries, UNO_QUERY );
    CPPUNIT_ASSERT( xRegressionCurveContainer.is() );

    Sequence< Reference< chart2::XRegressionCurve > > xRegressionCurveSequence = xRegressionCurveContainer->getRegressionCurves();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xRegressionCurveSequence.getLength());

    Reference<chart2::XRegressionCurve> xCurve;

    xCurve = xRegressionCurveSequence[0];
    CPPUNIT_ASSERT(xCurve.is());
    checkPolynomialTrendline(xCurve, "col2_poly", 3, 0.1, -0.1, -1.0);

    xCurve = xRegressionCurveSequence[1];
    CPPUNIT_ASSERT(xCurve.is());
    checkLinearTrendline(xCurve, "col2_linear", -0.5, -0.5, 0.0);

    xCurve = xRegressionCurveSequence[2];
    CPPUNIT_ASSERT(xCurve.is());
    checkMovingAverageTrendline(xCurve, "col2_moving_avg", 3);
}

}

// improve the test
CPPUNIT_TEST_FIXTURE(Chart2ExportTest, testErrorBarXLSX)
{
    loadFromFile(u"ods/error_bar.ods");
    {
        // make sure the ODS import was successful
        uno::Reference< chart2::XChartDocument > xChartDoc = getChartDocFromSheet( 0, mxComponent );
        CPPUNIT_ASSERT(xChartDoc.is());

        Reference< chart2::XDataSeries > xDataSeries = getDataSeriesFromDoc( xChartDoc, 0 );
        CPPUNIT_ASSERT( xDataSeries.is() );

        Reference< beans::XPropertySet > xPropSet( xDataSeries, UNO_QUERY_THROW );

        // test that y error bars are there
        Reference< beans::XPropertySet > xErrorBarYProps;
        xPropSet->getPropertyValue(CHART_UNONAME_ERRORBAR_Y) >>= xErrorBarYProps;
        testErrorBar(xErrorBarYProps);
    }

    saveAndReload("Calc Office Open XML");
    {
        uno::Reference< chart2::XChartDocument > xChartDoc = getChartDocFromSheet( 0, mxComponent );
        CPPUNIT_ASSERT(xChartDoc.is());

        Reference< chart2::XDataSeries > xDataSeries = getDataSeriesFromDoc( xChartDoc, 0 );
        CPPUNIT_ASSERT( xDataSeries.is() );

        Reference< beans::XPropertySet > xPropSet( xDataSeries, UNO_QUERY_THROW );

        // test that y error bars are there
        Reference< beans::XPropertySet > xErrorBarYProps;
        xPropSet->getPropertyValue(CHART_UNONAME_ERRORBAR_Y) >>= xErrorBarYProps;
        testErrorBar(xErrorBarYProps);
    }
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest, testErrorBarPropXLSX)
{
    loadFromFile(u"xlsx/testErrorBarProp.xlsx");
    save("Calc Office Open XML");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart1.xml");
    CPPUNIT_ASSERT(pXmlDoc);

    // test y error bars property
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:scatterChart/c:ser/c:errBars[1]/c:errDir"_ostr, "val"_ostr, "y");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:scatterChart/c:ser/c:errBars[1]/c:spPr/a:ln"_ostr, "w"_ostr, "12600");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:scatterChart/c:ser/c:errBars[1]/c:spPr/a:ln/a:solidFill/a:srgbClr"_ostr, "val"_ostr, "ff0000");

    // test x error bars property
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:scatterChart/c:ser/c:errBars[2]/c:errDir"_ostr, "val"_ostr, "x");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:scatterChart/c:ser/c:errBars[2]/c:spPr/a:ln"_ostr, "w"_ostr, "9360");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:scatterChart/c:ser/c:errBars[2]/c:spPr/a:ln/a:solidFill/a:srgbClr"_ostr, "val"_ostr, "595959");
}

// This method tests the preservation of properties for trendlines / regression curves
// in an export -> import cycle using different file formats - ODS, XLS and XLSX.
CPPUNIT_TEST_FIXTURE(Chart2ExportTest, testTrendline)
{
    // Validation fails with
    // Error: tag name "chart:symbol-image" is not allowed. Possible tag names are: <label-separator>
    skipValidation();
    loadFromFile(u"ods/trendline.ods");
    checkTrendlinesInChart(getChartDocFromSheet( 0, mxComponent));
    saveAndReload("calc8");
    checkTrendlinesInChart(getChartDocFromSheet( 0, mxComponent));
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest, testTrendlineOOXML)
{
    loadFromFile(u"ods/trendline.ods");
    checkTrendlinesInChart(getChartDocFromSheet( 0, mxComponent));
    saveAndReload("Calc Office Open XML");
    checkTrendlinesInChart(getChartDocFromSheet( 0, mxComponent));
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest, testTrendlineXLS)
{
    loadFromFile(u"ods/trendline.ods");
    checkTrendlinesInChart(getChartDocFromSheet( 0, mxComponent));
    saveAndReload("MS Excel 97");
    checkTrendlinesInChart(getChartDocFromSheet( 0, mxComponent));
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest, testMovingAverage)
{
    loadFromFile(u"ods/moving-type.ods");
    saveAndReload("calc8");

    uno::Reference< chart2::XChartDocument > xChartDoc = getChartDocFromSheet( 0, mxComponent);
    CPPUNIT_ASSERT(xChartDoc.is());

    Reference< chart2::XDataSeries > xDataSeries = getDataSeriesFromDoc( xChartDoc, 0 );
    CPPUNIT_ASSERT( xDataSeries.is() );

    Reference< chart2::XRegressionCurveContainer > xRegressionCurveContainer( xDataSeries, UNO_QUERY );
    CPPUNIT_ASSERT( xRegressionCurveContainer.is() );

    Sequence< Reference< chart2::XRegressionCurve > > xRegressionCurveSequence = xRegressionCurveContainer->getRegressionCurves();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xRegressionCurveSequence.getLength());

    Reference<chart2::XRegressionCurve> xCurve = xRegressionCurveSequence[0];
    CPPUNIT_ASSERT(xCurve.is());

    Reference<XPropertySet> xProperties( xCurve , uno::UNO_QUERY );
    CPPUNIT_ASSERT(xProperties.is());

    sal_Int32 nMovingAverageType = 0;
    xProperties->getPropertyValue("MovingAverageType") >>= nMovingAverageType;
    CPPUNIT_ASSERT_EQUAL(chart2::MovingAverageType::Central, nMovingAverageType);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest, testStockChart)
{
    /*  For attached file Stock_Chart.docx, in chart1.xml,
     * <c:stockChart>, there are four types of series as
     * Open,Low,High and Close.
     * For Open series, in <c:idx val="0" />
     * an attribute val of index should start from 1 and not from 0.
     * Which was problem area.
     */
    loadFromFile(u"docx/testStockChart.docx");

    save("Office Open XML Text");
    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart1.xml");
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:stockChart/c:ser[1]/c:idx"_ostr, "val"_ostr, "1");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:stockChart/c:ser[1]/c:order"_ostr, "val"_ostr, "1");
    assertXPathContent(
        pXmlDoc,
        "/c:chartSpace/c:chart/c:plotArea/c:stockChart/c:ser[1]/c:tx/c:strRef/c:strCache/c:pt/c:v"_ostr,
        "Open");
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest, testBarChart)
{
    loadFromFile(u"docx/testBarChart.docx");
    save("Office Open XML Text");
    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart1.xml");
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:barDir"_ostr, "val"_ostr, "col");
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest, testCrosses)
{
    // test crosses val="autoZero" with DOCX
    {
        loadFromFile(u"docx/Bar_horizontal_cone.docx");
        save("Office Open XML Text");
        xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart1.xml");

        assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:catAx/c:crosses"_ostr, "val"_ostr, "autoZero");
    }
    // tdf#142351: test crossesAt val="-50" with XLSX
    {
        loadFromFile(u"xlsx/tdf142351.xlsx");
        save("Calc Office Open XML");
        xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart1.xml");
        CPPUNIT_ASSERT(pXmlDoc);

        assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:catAx/c:crossesAt"_ostr, "val"_ostr, "-50");
    }
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest, testScatterChartTextXValues)
{
    loadFromFile(u"docx/scatter-chart-text-x-values.docx");

    Reference<chart2::XChartDocument> xChartDoc(getChartDocFromWriter(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());

    Reference<chart2::XChartType> xCT = getChartTypeFromDoc(xChartDoc, 0);
    CPPUNIT_ASSERT(xCT.is());

    // Make sure we have exactly 3 data series.
    std::vector<uno::Sequence<uno::Any> > aLabels = getDataSeriesLabelsFromChartType(xCT);
    CPPUNIT_ASSERT_EQUAL(size_t(3), aLabels.size());
    CPPUNIT_ASSERT_EQUAL(OUString("Series 1"), aLabels[0][0].get<OUString>());
    CPPUNIT_ASSERT_EQUAL(OUString("Series 2"), aLabels[1][0].get<OUString>());
    CPPUNIT_ASSERT_EQUAL(OUString("Series 3"), aLabels[2][0].get<OUString>());

    std::vector<std::vector<double> > aYValues = getDataSeriesYValuesFromChartType(xCT);
    CPPUNIT_ASSERT_EQUAL(size_t(3), aYValues.size());

    // Check the Y values of "Series 1".
    CPPUNIT_ASSERT_EQUAL(size_t(4), aYValues[0].size());
    CPPUNIT_ASSERT_EQUAL(4.3, aYValues[0][0]);
    CPPUNIT_ASSERT_EQUAL(2.5, aYValues[0][1]);
    CPPUNIT_ASSERT_EQUAL(3.5, aYValues[0][2]);
    CPPUNIT_ASSERT_EQUAL(4.5, aYValues[0][3]);

    // And "Series 2".
    CPPUNIT_ASSERT_EQUAL(size_t(4), aYValues[1].size());
    CPPUNIT_ASSERT_EQUAL(2.4, aYValues[1][0]);
    CPPUNIT_ASSERT_EQUAL(4.4, aYValues[1][1]);
    CPPUNIT_ASSERT_EQUAL(1.8, aYValues[1][2]);
    CPPUNIT_ASSERT_EQUAL(2.8, aYValues[1][3]);

    // And "Series 3".
    CPPUNIT_ASSERT_EQUAL(size_t(4), aYValues[2].size());
    CPPUNIT_ASSERT_EQUAL(2.0, aYValues[2][0]);
    CPPUNIT_ASSERT_EQUAL(2.0, aYValues[2][1]);
    CPPUNIT_ASSERT_EQUAL(3.0, aYValues[2][2]);
    CPPUNIT_ASSERT_EQUAL(5.0, aYValues[2][3]);

    // Test the export.
    save("Office Open XML Text");
    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart1.xml");
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPathContent(pXmlDoc, "//c:scatterChart/c:ser[1]/c:xVal[1]/c:numRef[1]/c:numCache[1]/c:pt[1]/c:v[1]"_ostr, "1");
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest, testScatterXAxisValues)
{
    loadFromFile(u"odt/tdf114657.odt");

    save("Office Open XML Text");
    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart1.xml");
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "//c:scatterChart/c:ser/c:xVal/c:numRef/c:numCache/c:ptCount"_ostr, "val"_ostr, "5");
    assertXPathContent(pXmlDoc, "//c:scatterChart/c:ser/c:xVal/c:numRef/c:numCache/c:pt[1]/c:v"_ostr, "15");
    assertXPathContent(pXmlDoc, "//c:scatterChart/c:ser/c:xVal/c:numRef/c:numCache/c:pt[2]/c:v"_ostr, "11");
    assertXPathContent(pXmlDoc, "//c:scatterChart/c:ser/c:xVal/c:numRef/c:numCache/c:pt[3]/c:v"_ostr, "20");
    assertXPathContent(pXmlDoc, "//c:scatterChart/c:ser/c:xVal/c:numRef/c:numCache/c:pt[4]/c:v"_ostr, "16");
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest, testScatterXAxisCategories)
{
    loadFromFile(u"odt/tdf131143.odt");

    save("Office Open XML Text");
    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart1.xml");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "//c:scatterChart/c:ser[1]/c:xVal/c:strRef/c:strCache/c:ptCount"_ostr, "val"_ostr, "4");
    assertXPathContent(pXmlDoc, "//c:scatterChart/c:ser[1]/c:xVal/c:strRef/c:strCache/c:pt[1]/c:v"_ostr, "Row 1");
    assertXPathContent(pXmlDoc, "//c:scatterChart/c:ser[1]/c:xVal/c:strRef/c:strCache/c:pt[2]/c:v"_ostr, "Row 2");
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest, testChartDataTable)
{
    loadFromFile(u"docx/testChartDataTable.docx");

    save("Office Open XML Text");
    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart1.xml");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:dTable/c:showHorzBorder"_ostr, "val"_ostr, "1");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:dTable/c:showVertBorder"_ostr, "val"_ostr, "1");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:dTable/c:showOutline"_ostr, "val"_ostr, "1");
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest, testChartExternalData)
{
    loadFromFile(u"docx/testMultipleChart.docx");

    save("Office Open XML Text");
    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart1.xml");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:externalData"_ostr);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest, testEmbeddingsGrabBag)
{
   // The problem was that .xlsx files were missing from docx file from embeddings folder
   // after saving file.
   // This test case tests whether embeddings files grabbagged properly in correct object.

   loadFromFile(u"docx/testMultiplechartembeddings.docx" );
   uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
   uno::Reference<beans::XPropertySet> xTextDocumentPropertySet(xTextDocument, uno::UNO_QUERY);
   uno::Sequence<beans::PropertyValue> aGrabBag(0);
   xTextDocumentPropertySet->getPropertyValue("InteropGrabBag") >>= aGrabBag;
   CPPUNIT_ASSERT(aGrabBag.hasElements()); // Grab Bag not empty
   bool bEmbeddings = false;
   const char* const testEmbeddedFileNames[] {"word/embeddings/Microsoft_Excel_Worksheet3.xlsx",
                                        "word/embeddings/Microsoft_Excel_Worksheet2.xlsx",
                                        "word/embeddings/Microsoft_Excel_Worksheet1.xlsx"};
   for (beans::PropertyValue const& prop : aGrabBag)
   {
       if (prop.Name == "OOXEmbeddings")
       {
           bEmbeddings = true;
           uno::Sequence<beans::PropertyValue> aEmbeddingsList(0);
           uno::Reference<io::XInputStream> aEmbeddingXlsxStream;
           OUString aEmbeddedfileName;
           CPPUNIT_ASSERT(prop.Value >>= aEmbeddingsList); // PropertyValue of proper type
           sal_Int32 length = aEmbeddingsList.getLength();
           CPPUNIT_ASSERT_EQUAL(sal_Int32(3), length);
           for(int j = 0; j < length; ++j)
           {
               aEmbeddingsList[j].Value >>= aEmbeddingXlsxStream;
               aEmbeddedfileName = aEmbeddingsList[j].Name;
               CPPUNIT_ASSERT(aEmbeddingXlsxStream); // Reference not empty
               CPPUNIT_ASSERT_EQUAL(OUString::createFromAscii(testEmbeddedFileNames[j]),aEmbeddedfileName);
           }
       }
   }
   CPPUNIT_ASSERT(bEmbeddings); // Grab Bag has all the expected elements
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest, testAreaChartLoad)
{
    loadFromFile(u"docx/testAreaChartLoad.docx");

    // FIXME: validation error in OOXML export: Errors: 1
    skipValidation();

    save("Office Open XML Text");
    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart1.xml");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:areaChart/c:ser/c:dLbls/c:showVal"_ostr, "val"_ostr, "1");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:areaChart/c:ser/c:dLbls/c:dLbl"_ostr, 0);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest, testUpDownBars)
{
    loadFromFile(u"docx/UpDownBars.docx");
    save("Office Open XML Text");
    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart1.xml");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:upDownBars"_ostr, 0);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest, testDoughnutChart)
{
    loadFromFile(u"docx/doughnutChart.docx");
    save("Office Open XML Text");
    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart1.xml");
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:doughnutChart"_ostr);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest, testPieOfPieChart)
{
    loadFromFile(u"xlsx/pieOfPieChart.xlsx");
    save("Calc Office Open XML");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart1.xml");
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:ofPieChart"_ostr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:ofPieChart/c:ofPieType[1]"_ostr, "val"_ostr, "pie");
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest, testBarOfPieChart)
{
    loadFromFile(u"xlsx/barOfPieChart.xlsx");
    save("Calc Office Open XML");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart1.xml");
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:ofPieChart"_ostr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:ofPieChart/c:ofPieType[1]"_ostr, "val"_ostr, "bar");
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest, testDisplayUnits)
{
    loadFromFile(u"docx/DisplayUnits.docx");
    save("Office Open XML Text");
    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart1.xml");
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx/c:dispUnits/c:builtInUnit"_ostr, "val"_ostr, "billions");
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest, testFdo74115WallGradientFill)
{
    loadFromFile(u"docx/fdo74115_WallGradientFill.docx");
    save("Office Open XML Text");
    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart1.xml");
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:spPr/a:gradFill"_ostr);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest, testFdo74115WallBitmapFill)
{
    loadFromFile(u"docx/fdo74115_WallBitmapFill.docx");
    save("Office Open XML Text");
    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart1.xml");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:spPr/a:blipFill"_ostr);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest, testPieChartWallLineStyle)
{
    loadFromFile(u"odt/testPieChartWallLineStyle.odt");

    // FIXME: validation error in OOXML export: Errors: 9
    skipValidation();

    save("Office Open XML Text");
    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart1.xml");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:spPr/a:ln/a:noFill"_ostr);
}

//The below test case tests the built in marker 'x' for Office 2010 in Line charts

CPPUNIT_TEST_FIXTURE(Chart2ExportTest, testFdo78290LineChartMarkerX)
{
    loadFromFile(u"docx/fdo78290_Line_Chart_Marker_x.docx");
    save("Office Open XML Text");
    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart1.xml");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace[1]/c:chart[1]/c:plotArea[1]/c:lineChart[1]/c:ser[1]/c:marker[1]/c:symbol[1]"_ostr,"val"_ostr,"x");
    assertXPath(pXmlDoc, "/c:chartSpace[1]/c:chart[1]/c:plotArea[1]/c:lineChart[1]/c:ser[1]/c:marker[1]/c:size[1]"_ostr,"val"_ostr,"7");
}

// We can also use the built in marker 'x' in scatter chart, hence writing the test case for the same.

CPPUNIT_TEST_FIXTURE(Chart2ExportTest, testFdo78290ScatterChartMarkerX)
{
    loadFromFile(u"docx/fdo78290_Scatter_Chart_Marker_x.docx");
    save("Office Open XML Text");
    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart1.xml");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace[1]/c:chart[1]/c:plotArea[1]/c:scatterChart[1]/c:ser[1]/c:marker[1]/c:symbol[1]"_ostr,"val"_ostr,"x");
    assertXPath(pXmlDoc, "/c:chartSpace[1]/c:chart[1]/c:plotArea[1]/c:scatterChart[1]/c:ser[1]/c:marker[1]/c:size[1]"_ostr,"val"_ostr,"7");
}

// Also in a combination of charts like a column chart and line chart, we can use the built in marker 'x'
// for the line chart too. hence put a test case for the combination chart also.

CPPUNIT_TEST_FIXTURE(Chart2ExportTest, testFdo78290CombinationChartMarkerX)
{
    loadFromFile(u"docx/fdo78290_Combination_Chart_Marker_x.docx");
    save("Office Open XML Text");
    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart1.xml");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace[1]/c:chart[1]/c:plotArea[1]/c:lineChart[1]/c:ser[1]/c:marker[1]/c:symbol[1]"_ostr,"val"_ostr,"x");
    assertXPath(pXmlDoc, "/c:chartSpace[1]/c:chart[1]/c:plotArea[1]/c:lineChart[1]/c:ser[1]/c:marker[1]/c:size[1]"_ostr,"val"_ostr,"7");
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest, testTdf126115IndividualMarker)
{
    // Check individual marker properties.
    loadFromFile(u"xlsx/tdf126115.xlsx");
    save("Calc Office Open XML");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart1.xml");
    CPPUNIT_ASSERT(pXmlDoc);
    // 1. series
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:scatterChart/c:ser[1]/c:dPt/c:marker/c:symbol"_ostr, "val"_ostr, "square");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:scatterChart/c:ser[1]/c:dPt/c:marker/c:size"_ostr, "val"_ostr, "8");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:scatterChart/c:ser[1]/c:dPt/c:marker/c:spPr/a:solidFill/a:srgbClr"_ostr, "val"_ostr, "ff0000");
    // 2. series
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:scatterChart/c:ser[2]/c:dPt/c:marker/c:symbol"_ostr, "val"_ostr, "x");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:scatterChart/c:ser[2]/c:dPt/c:marker/c:size"_ostr, "val"_ostr, "15");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:scatterChart/c:ser[2]/c:dPt/c:marker/c:spPr/a:solidFill/a:srgbClr"_ostr, "val"_ostr, "7030a0");
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest, testAxisNumberFormatODS)
{
    struct
    {
        void check( const Reference<chart2::XChartDocument>& xChartDoc )
        {
            Reference<chart2::XAxis> xAxisX = getAxisFromDoc(xChartDoc, 0, 0, 0);
            Reference<chart2::XTitled> xTitle(xAxisX, UNO_QUERY_THROW);
            OUString aTitleText = getTitleString(xTitle);
            CPPUNIT_ASSERT_EQUAL(OUString("Linked To Source"), aTitleText);

            sal_Int32 nNumFmt = getNumberFormatFromAxis(xAxisX);
            sal_Int16 nType = getNumberFormatType(xChartDoc, nNumFmt);
            CPPUNIT_ASSERT_MESSAGE("X axis should be percentage format.", (nType & util::NumberFormat::PERCENT));

            bool bNumFmtLinked = false;
            Reference<beans::XPropertySet> xPS(xAxisX, uno::UNO_QUERY_THROW);
            xPS->getPropertyValue("LinkNumberFormatToSource") >>= bNumFmtLinked;
            CPPUNIT_ASSERT_MESSAGE("X axis should have its number format linked to source.", bNumFmtLinked);

            Reference<chart2::XAxis> xAxisY = getAxisFromDoc(xChartDoc, 0, 1, 0);
            xTitle.set(xAxisY, UNO_QUERY_THROW);
            aTitleText = getTitleString(xTitle);
            CPPUNIT_ASSERT_EQUAL(OUString("Not Linked"), aTitleText);

            nNumFmt = getNumberFormatFromAxis(xAxisY);
            nType = getNumberFormatType(xChartDoc, nNumFmt);
            CPPUNIT_ASSERT_MESSAGE("Y axis should be a normal number format.", (nType & util::NumberFormat::NUMBER));

            bNumFmtLinked = true;
            xPS.set(xAxisY, uno::UNO_QUERY_THROW);
            xPS->getPropertyValue("LinkNumberFormatToSource") >>= bNumFmtLinked;
            CPPUNIT_ASSERT_MESSAGE("Y axis should not have its number format linked to source.", !bNumFmtLinked);
        }

    } aTest;

    loadFromFile(u"ods/axis-numformats-linked.ods");

    Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    aTest.check(xChartDoc);

    // Reload the document and make sure everything remains intact.
    saveAndReload("calc8");
    xChartDoc = getChartDocFromSheet(0, mxComponent);
    aTest.check(xChartDoc);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest, testAxisNumberFormatXLS)
{
    struct
    {
        void check( const Reference<chart2::XChartDocument>& xChartDoc, bool bNumFmtLinkedActual, sal_Int16 nNumFmtTypeFlag ) const
        {
            Reference<chart2::XAxis> xAxisY = getAxisFromDoc( xChartDoc, 0, 1, 0 );
            bool bNumFmtLinked = false;
            Reference<beans::XPropertySet> xPS( xAxisY, uno::UNO_QUERY_THROW );
            xPS->getPropertyValue( "LinkNumberFormatToSource" ) >>= bNumFmtLinked;

            if ( bNumFmtLinkedActual )
                CPPUNIT_ASSERT_MESSAGE( "Y axis should have its number format linked to source.", bNumFmtLinked );
            else
            {
                CPPUNIT_ASSERT_MESSAGE( "Y axis should not have its number format linked to source.", !bNumFmtLinked );

                sal_Int32 nNumFmt = getNumberFormatFromAxis( xAxisY );
                sal_Int16 nType = getNumberFormatType( xChartDoc, nNumFmt );
                if ( nNumFmtTypeFlag == util::NumberFormat::PERCENT )
                    CPPUNIT_ASSERT_MESSAGE( "Y axis should be percentage format.", ( nType & util::NumberFormat::PERCENT ) );
                else
                    CPPUNIT_ASSERT_MESSAGE( "Y axis should be number format.", ( nType & util::NumberFormat::NUMBER ) );
            }
        }

        void change( const Reference<chart2::XChartDocument>& xChartDoc, sal_Int16 nNumFmtTypeFlag )
        {
            Reference<chart2::XAxis> xAxisY = getAxisFromDoc( xChartDoc, 0, 1, 0 );
            Reference<beans::XPropertySet> xPS( xAxisY, uno::UNO_QUERY_THROW );
            Any aAny( false );
            xPS->setPropertyValue( "LinkNumberFormatToSource", aAny );

            Reference<util::XNumberFormatsSupplier> xNFS( xChartDoc, uno::UNO_QUERY_THROW );
            Reference<util::XNumberFormats> xNumberFormats = xNFS->getNumberFormats();
            CPPUNIT_ASSERT( xNumberFormats.is() );
            lang::Locale aLocale{ "en", "US", "" };
            Sequence<sal_Int32> aNumFmts = xNumberFormats->queryKeys( nNumFmtTypeFlag, aLocale, false );
            CPPUNIT_ASSERT( aNumFmts.hasElements() );
            aAny <<= aNumFmts[0];
            xPS->setPropertyValue( CHART_UNONAME_NUMFMT, aAny );
        }

    } aTest;

    loadFromFile(u"xls/axis_sourceformatting.xls" );

    Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet( 0, mxComponent );
    aTest.check( xChartDoc, true, util::NumberFormat::PERCENT );

    aTest.change( xChartDoc, util::NumberFormat::NUMBER );
    // Write the document(xls) with changes made close it, load it and check if changes are intact
    saveAndReload( "MS Excel 97" );
    xChartDoc = getChartDocFromSheet( 0, mxComponent );
    aTest.check( xChartDoc, false, util::NumberFormat::NUMBER );
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest, testDataLabelBordersDOCX)
{
    struct Check
    {
        sal_Int32 mnIndex;
        css::drawing::LineStyle meStyle;
        Color mnColor;
    };

    struct
    {
        /**
         * Chart 1 has 4 bars of which 1st and 3rd have labels with borders
         * around them.
         */
        void checkObject1( const Reference<chart2::XChartDocument>& xChartDoc )
        {
            CPPUNIT_ASSERT(xChartDoc.is());

            Reference<chart2::XDataSeries> xDataSeries = getDataSeriesFromDoc(xChartDoc, 0);
            CPPUNIT_ASSERT(xDataSeries.is());

            // Check to make sure that data points 0 and 2 have local properties.
            Reference<beans::XPropertySet> xPropSet(xDataSeries, uno::UNO_QUERY);
            CPPUNIT_ASSERT(xPropSet.is());

            Sequence<sal_Int32> aIndices;
            xPropSet->getPropertyValue("AttributedDataPoints") >>= aIndices;
            /*
            CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be 2 data points with local properties.", sal_Int32(2), aIndices.getLength());
            CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aIndices[0]);
            CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aIndices[1]);
            */

            static const Check aDataPoints[] =
            {
                { 0, css::drawing::LineStyle_SOLID, 0x00FFFF00 }, // solid yellow
                { 2, css::drawing::LineStyle_SOLID, 0x00FF0000 }  // solid red
            };

            for (size_t i = 0; i < std::size(aDataPoints); ++i)
            {
                xPropSet = xDataSeries->getDataPointByIndex(aDataPoints[i].mnIndex);
                CPPUNIT_ASSERT(xPropSet.is());

                css::drawing::LineStyle eLineStyle = css::drawing::LineStyle_NONE;
                xPropSet->getPropertyValue(CHART_UNONAME_LABEL_BORDER_STYLE) >>= eLineStyle;
                CPPUNIT_ASSERT_EQUAL(aDataPoints[i].meStyle, eLineStyle);

                sal_Int32 nWidth = -1;
                xPropSet->getPropertyValue(CHART_UNONAME_LABEL_BORDER_WIDTH) >>= nWidth;
                CPPUNIT_ASSERT(nWidth > 0);

                Color nColor;
                xPropSet->getPropertyValue(CHART_UNONAME_LABEL_BORDER_COLOR) >>= nColor;
                CPPUNIT_ASSERT_EQUAL_MESSAGE("Border color is wrong.", aDataPoints[i].mnColor, nColor);
            }
        }

        /**
         * Chart 2 has all its data labels with identical borders.
         */
        void checkObject2( const Reference<chart2::XChartDocument>& xChartDoc )
        {
            CPPUNIT_ASSERT(xChartDoc.is());

            Reference<chart2::XDataSeries> xDataSeries = getDataSeriesFromDoc(xChartDoc, 0);
            CPPUNIT_ASSERT(xDataSeries.is());

            Reference<beans::XPropertySet> xPropSet(xDataSeries, uno::UNO_QUERY);
            CPPUNIT_ASSERT(xPropSet.is());

            css::drawing::LineStyle eLineStyle = css::drawing::LineStyle_NONE;
            xPropSet->getPropertyValue(CHART_UNONAME_LABEL_BORDER_STYLE) >>= eLineStyle;
            CPPUNIT_ASSERT_EQUAL(css::drawing::LineStyle_SOLID, eLineStyle);

            sal_Int32 nWidth = -1;
            xPropSet->getPropertyValue(CHART_UNONAME_LABEL_BORDER_WIDTH) >>= nWidth;
            CPPUNIT_ASSERT(nWidth > 0);

            Color nColor;
            xPropSet->getPropertyValue(CHART_UNONAME_LABEL_BORDER_COLOR) >>= nColor;
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Border color should be green.", COL_LIGHTGREEN, nColor);
        }

    } aTest;

    loadFromFile(u"docx/data-label-borders.docx");

    Reference<chart2::XChartDocument> xChartDoc(getChartDocFromWriter(0), uno::UNO_QUERY);

    // "Automatic" chart background fill in docx should be loaded as solid white.
    Reference<beans::XPropertySet> xPropSet = xChartDoc->getPageBackground();
    CPPUNIT_ASSERT(xPropSet.is());
    drawing::FillStyle eStyle = xPropSet->getPropertyValue("FillStyle").get<drawing::FillStyle>();
    sal_Int32 nColor = xPropSet->getPropertyValue("FillColor").get<sal_Int32>();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("'Automatic' chart background fill in docx should be loaded as solid fill.",
        drawing::FillStyle_SOLID, eStyle);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("'Automatic' chart background fill in docx should be loaded as solid white.",
         sal_Int32(0x00FFFFFF), sal_Int32(nColor & 0x00FFFFFF)); // highest 2 bytes are transparency which we ignore here.

    aTest.checkObject1(xChartDoc);
    xChartDoc.set(getChartDocFromWriter(1), uno::UNO_QUERY);
    aTest.checkObject2(xChartDoc);

    // FIXME: validation error in OOXML export: Errors: 3
    skipValidation();

    saveAndReload("Office Open XML Text");

    xChartDoc.set(getChartDocFromWriter(0), uno::UNO_QUERY);
    aTest.checkObject1(xChartDoc);
    xChartDoc.set(getChartDocFromWriter(1), uno::UNO_QUERY);
    aTest.checkObject2(xChartDoc);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest, testDataLabel3DChartDOCX)
{
    loadFromFile(u"docx/3d-bar-label.docx");

    Reference<chart2::XChartDocument> xChartDoc(getChartDocFromWriter(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());

    // FIXME: validation error in OOXML export: Errors: 1
    skipValidation();

    save("Office Open XML Text");
    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart1.xml");
    CPPUNIT_ASSERT(pXmlDoc);

    // We must not export label position attributes for 3D bar charts. The
    // same rule also applies to several other 3D charts, apparently.
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:bar3DChart/c:ser/c:dLbls/c:dLblPos"_ostr, 0);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:bar3DChart/c:ser/c:dLbls/c:dLbl/c:dLblPos"_ostr, 0);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest, testDataLabelBarChartDOCX)
{
    loadFromFile(u"docx/bar-chart-labels.docx");

    Reference<chart2::XChartDocument> xChartDoc(getChartDocFromWriter(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());

    save("Office Open XML Text");
    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart1.xml");
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[1]/c:dLbls/c:dLblPos"_ostr, "val"_ostr, "ctr");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[2]/c:dLbls/c:dLblPos"_ostr, "val"_ostr, "inEnd");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[3]/c:dLbls/c:dLblPos"_ostr, "val"_ostr, "inBase");
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest, testDataLabelClusteredBarChartDOCX)
{
    loadFromFile(u"docx/clustered-bar-chart-labels.docx");

    Reference<chart2::XChartDocument> xChartDoc(getChartDocFromWriter(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());

    // FIXME: validation error in OOXML export: Errors: 9
    skipValidation();

    save("Office Open XML Text");
    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart1.xml");
    CPPUNIT_ASSERT(pXmlDoc);

    // This was "t", should be one of the allowed values.
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[1]/c:dLbls/c:dLbl[2]/c:dLblPos"_ostr, "val"_ostr, "outEnd");
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest, testDataLabelRadarChartDOCX)
{
    loadFromFile(u"docx/radar-chart-labels.docx");

    Reference<chart2::XChartDocument> xChartDoc(getChartDocFromWriter(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());

    save("Office Open XML Text");
    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart1.xml");
    CPPUNIT_ASSERT(pXmlDoc);

    // We must not export label position attributes for radar charts.
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:radarChart/c:ser/c:dLbls/c:dLblPos"_ostr, 0);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:radarChart/c:ser/c:dLbls/c:dLbl/c:dLblPos"_ostr, 0);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest, testDataLabelDoughnutChartDOCX)
{
    loadFromFile(u"docx/doughnut-chart-labels.docx");

    Reference<chart2::XChartDocument> xChartDoc(getChartDocFromWriter(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());

    // FIXME: validation error in OOXML export: Errors: 1
    skipValidation();

    save("Office Open XML Text");
    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart1.xml");
    CPPUNIT_ASSERT(pXmlDoc);

    // We must not export label position attributes for doughnut charts.
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:doughnutChart/c:ser/c:dLbls/c:dLblPos"_ostr, 0);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:doughnutChart/c:ser/c:dLbls/c:dLbl/c:dLblPos"_ostr, 0);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest, testDataLabelAreaChartDOCX)
{
    loadFromFile(u"docx/area-chart-labels.docx");

    Reference<chart2::XChartDocument> xChartDoc(getChartDocFromWriter(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());

    // FIXME: validation error in OOXML export: Errors: 1
    skipValidation();

    save("Office Open XML Text");
    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart1.xml");
    CPPUNIT_ASSERT(pXmlDoc);

    // We must not export label position attributes for area charts.
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:areaChart/c:ser/c:dLbls/c:dLblPos"_ostr, 0);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:areaChart/c:ser/c:dLbls/c:dLbl/c:dLblPos"_ostr, 0);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest, testDataLabelDefaultLineChartDOCX)
{
    // This file was created by Word 2007, which doesn't provide default data
    // label position (2010 does).  Make sure its default data label position
    // is RIGHT when exporting.

    loadFromFile(u"docx/line-chart-label-default-placement.docx");

    Reference<chart2::XChartDocument> xChartDoc(getChartDocFromWriter(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());

    saveAndReload("Office Open XML Text");

    xChartDoc.set(getChartDocFromWriter(0), uno::UNO_QUERY);
    Reference<chart2::XDataSeries> xDataSeries = getDataSeriesFromDoc(xChartDoc, 0);
    Reference<beans::XPropertySet> xPropSet(xDataSeries, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xPropSet.is());
    sal_Int32 nLabelPlacement = -1;
    if (xPropSet->getPropertyValue("LabelPlacement") >>= nLabelPlacement)
        // This option may not be set.  Check its value only when it's set.
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Line chart's default label placement should be 'right'.", chart::DataLabelPlacement::RIGHT, nLabelPlacement );
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest, testIndividualDataLabelProps)
{
    loadFromFile(u"xlsx/tdf122915.xlsx");

    // FIXME: validation error in OOXML export: Errors: 1
    skipValidation();

    save("Calc Office Open XML");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart1.xml");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:scatterChart/c:ser[3]/c:dLbls/c:dLbl/c:txPr/a:p/a:pPr/a:defRPr"_ostr, "b"_ostr, "1");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:scatterChart/c:ser[3]/c:dLbls/c:dLbl/c:txPr/a:p/a:pPr/a:defRPr"_ostr, "sz"_ostr, "1600");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:scatterChart/c:ser[3]/c:dLbls/c:dLbl/c:txPr/a:p/a:pPr/a:defRPr/a:solidFill/a:srgbClr"_ostr, "val"_ostr, "ff0000");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:scatterChart/c:ser[3]/c:dLbls/c:dLbl/c:txPr/a:p/a:pPr/a:defRPr/a:latin"_ostr, "typeface"_ostr, "Times New Roman");
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest, testBarChartRotation)
{
    loadFromFile(u"docx/barChartRotation.docx");
    save("Office Open XML Text");
    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart1.xml");
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:view3D/c:rotX"_ostr, "val"_ostr, "30");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:view3D/c:rotY"_ostr, "val"_ostr, "50");
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest, testShapeFollowedByChart)
{
    /* If there is a scenario where a chart is followed by a shape
       which is being exported as an alternate content then, the
       docPr Id is being repeated, ECMA 20.4.2.5 says that the
       docPr Id should be unique, ensuring the same here.
    */
    loadFromFile(u"docx/FDO74430.docx");

    // FIXME: validation error in OOXML export: Errors: 5
    skipValidation();

    save("Office Open XML Text" );
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    CPPUNIT_ASSERT(pXmlDoc);

    OUString aValueOfFirstDocPR = getXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[1]/w:drawing[1]/wp:inline[1]/wp:docPr[1]"_ostr, "id"_ostr);
    OUString aValueOfSecondDocPR = getXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/wp:docPr[1]"_ostr, "id"_ostr);

    CPPUNIT_ASSERT( aValueOfFirstDocPR != aValueOfSecondDocPR );
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest, testPieChartDataLabels)
{
    loadFromFile(u"docx/PieChartDataLabels.docx");

    // FIXME: validation error in OOXML export: Errors: 19
    skipValidation();

    save("Office Open XML Text");
    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart1.xml");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:pie3DChart/c:ser[1]/c:dLbls/c:dLbl[1]/c:dLblPos"_ostr, "val"_ostr, "outEnd");
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest, testSeriesIdxOrder)
{
    loadFromFile(u"docx/testSeriesIdxOrder.docx");
    save("Office Open XML Text");
    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart1.xml");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace[1]/c:chart[1]/c:plotArea[1]/c:lineChart[1]/c:ser[1]/c:idx[1]"_ostr, "val"_ostr, "1");
    assertXPath(pXmlDoc, "/c:chartSpace[1]/c:chart[1]/c:plotArea[1]/c:lineChart[1]/c:ser[1]/c:order[1]"_ostr, "val"_ostr, "1");
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest, testScatterPlotLabels)
{
    loadFromFile(u"odt/scatter-plot-labels.odt");
    Reference<chart2::XChartDocument> xChartDoc(getChartDocFromWriter(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());

    Reference<chart2::XChartType> xCT = getChartTypeFromDoc(xChartDoc, 0);
    CPPUNIT_ASSERT(xCT.is());

    // Make sure the original chart has 'a', 'b', 'c' as its data labels.
    std::vector<uno::Sequence<uno::Any> > aLabels = getDataSeriesLabelsFromChartType(xCT);
    CPPUNIT_ASSERT_EQUAL(size_t(3), aLabels.size());
    CPPUNIT_ASSERT_EQUAL(OUString("a"), aLabels[0][0].get<OUString>());
    CPPUNIT_ASSERT_EQUAL(OUString("b"), aLabels[1][0].get<OUString>());
    CPPUNIT_ASSERT_EQUAL(OUString("c"), aLabels[2][0].get<OUString>());

    // Reload the doc and check again.  The labels should not change.
    saveAndReload("writer8");

    xChartDoc.set(getChartDocFromWriter(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());

    xCT = getChartTypeFromDoc(xChartDoc, 0);
    CPPUNIT_ASSERT(xCT.is());

    aLabels = getDataSeriesLabelsFromChartType(xCT);
    CPPUNIT_ASSERT_EQUAL(size_t(3), aLabels.size());
    CPPUNIT_ASSERT_EQUAL(OUString("a"), aLabels[0][0].get<OUString>());
    CPPUNIT_ASSERT_EQUAL(OUString("b"), aLabels[1][0].get<OUString>());
    CPPUNIT_ASSERT_EQUAL(OUString("c"), aLabels[2][0].get<OUString>());
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest, testErrorBarDataRangeODS)
{
    loadFromFile(u"ods/ErrorBarRange.ods");
    saveAndReload("calc8");

    uno::Reference< chart2::XChartDocument > xChartDoc = getChartDocFromSheet( 0, mxComponent );
    CPPUNIT_ASSERT(xChartDoc.is());

    Reference< chart2::XDataSeries > xDataSeries = getDataSeriesFromDoc( xChartDoc, 0 );
    CPPUNIT_ASSERT( xDataSeries.is() );

    Reference< beans::XPropertySet > xPropSet( xDataSeries, UNO_QUERY_THROW );

    // test that y error bars are there
    Reference< beans::XPropertySet > xErrorBarYProps;
    xPropSet->getPropertyValue(CHART_UNONAME_ERRORBAR_Y) >>= xErrorBarYProps;
    uno::Any aAny = xErrorBarYProps->getPropertyValue("ErrorBarRangePositive");
    CPPUNIT_ASSERT(aAny.hasValue());
    OUString aPosRange;
    aAny >>= aPosRange;
    CPPUNIT_ASSERT_EQUAL(OUString("$Sheet1.$B$1:$B$3"), aPosRange);

    aAny = xErrorBarYProps->getPropertyValue("ErrorBarRangeNegative");
    CPPUNIT_ASSERT(aAny.hasValue());
    OUString aNegRange;
    aAny >>= aNegRange;
    CPPUNIT_ASSERT_EQUAL(OUString("$Sheet1.$C$1:$C$3"), aNegRange);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest, tdf50934_barOfPie)
{
    loadFromFile(u"ods/tdf50934_barOfPie.ods");
    saveAndReload("calc8");

    uno::Reference< chart2::XChartDocument > xChartDoc = getChartDocFromSheet( 0, mxComponent );
    CPPUNIT_ASSERT(xChartDoc.is());

    Reference< chart2::XChartType > xChartType = getChartTypeFromDoc( xChartDoc, 0 );
    CPPUNIT_ASSERT(xChartType.is());

    CPPUNIT_ASSERT_EQUAL(u"com.sun.star.chart2.PieChartType"_ustr,
            xChartType->getChartType());

    // Verify that it saves and loads as bar-of-pie
    Reference< chart2::XDiagram> xDia(xChartDoc->getFirstDiagram());
    CPPUNIT_ASSERT(xDia.is());
    uno::Reference< beans::XPropertySet > xDiaProp( xDia, uno::UNO_QUERY );
    CPPUNIT_ASSERT(xDiaProp.is());
    uno::Any aAny = xDiaProp->getPropertyValue("SubPieType");
    CPPUNIT_ASSERT(aAny.hasValue());
    chart2::PieChartSubType subPieType;
    aAny >>= subPieType;
    CPPUNIT_ASSERT_EQUAL(chart2::PieChartSubType_BAR, subPieType);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest, tdf50934_pieOfPie)
{
    loadFromFile(u"ods/tdf50934_pieOfPie.ods");
    saveAndReload("calc8");

    uno::Reference< chart2::XChartDocument > xChartDoc = getChartDocFromSheet( 0, mxComponent );
    CPPUNIT_ASSERT(xChartDoc.is());

    Reference< chart2::XChartType > xChartType = getChartTypeFromDoc( xChartDoc, 0 );
    CPPUNIT_ASSERT(xChartType.is());

    CPPUNIT_ASSERT_EQUAL(u"com.sun.star.chart2.PieChartType"_ustr,
            xChartType->getChartType());

    // Verify that it saves and loads as pie-of-pie
    Reference< chart2::XDiagram> xDia(xChartDoc->getFirstDiagram());
    CPPUNIT_ASSERT(xDia.is());
    uno::Reference< beans::XPropertySet > xDiaProp( xDia, uno::UNO_QUERY );
    CPPUNIT_ASSERT(xDiaProp.is());
    uno::Any aAny = xDiaProp->getPropertyValue("SubPieType");
    CPPUNIT_ASSERT(aAny.hasValue());
    chart2::PieChartSubType subPieType;
    aAny >>= subPieType;
    CPPUNIT_ASSERT_EQUAL(chart2::PieChartSubType_PIE, subPieType);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest, testChartCrash)
{
    loadFromFile(u"docx/FDO75975.docx");
    save("Office Open XML Text");
    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart1.xml");
    CPPUNIT_ASSERT(pXmlDoc);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest, testPieChartRotation)
{
    loadFromFile(u"docx/pieChartRotation.docx");
    save("Office Open XML Text");
    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart1.xml");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:view3D/c:rotX"_ostr, "val"_ostr, "40");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:view3D/c:rotY"_ostr, "val"_ostr, "30");
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest, testEmbeddingsOleObjectGrabBag)
{
   // The problem was that .bin files were missing from docx file from embeddings folder
   // after saving file.
   // This test case tests whether embeddings files grabbagged properly in correct object.

   loadFromFile(u"docx/testchartoleobjectembeddings.docx" );
   uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
   uno::Reference<beans::XPropertySet> xTextDocumentPropertySet(xTextDocument, uno::UNO_QUERY);
   uno::Sequence<beans::PropertyValue> aGrabBag(0);
   xTextDocumentPropertySet->getPropertyValue("InteropGrabBag") >>= aGrabBag;
   CPPUNIT_ASSERT(aGrabBag.hasElements()); // Grab Bag not empty
   bool bEmbeddings = false;
   const char* const testEmbeddedFileNames[] = {"word/embeddings/oleObject1.bin"};
   for (beans::PropertyValue const& prop : aGrabBag)
   {
       if (prop.Name == "OOXEmbeddings")
       {
           bEmbeddings = true;
           uno::Sequence<beans::PropertyValue> aEmbeddingsList(0);
           uno::Reference<io::XInputStream> aEmbeddingXlsxStream;
           OUString aEmbeddedfileName;
           CPPUNIT_ASSERT(prop.Value >>= aEmbeddingsList); // PropertyValue of proper type
           sal_Int32 length = aEmbeddingsList.getLength();
           CPPUNIT_ASSERT_EQUAL(sal_Int32(1), length);
           for(int j = 0; j < length; ++j)
           {
               aEmbeddingsList[j].Value >>= aEmbeddingXlsxStream;
               aEmbeddedfileName = aEmbeddingsList[j].Name;
               CPPUNIT_ASSERT(aEmbeddingXlsxStream); // Reference not empty
               CPPUNIT_ASSERT_EQUAL(OUString::createFromAscii(testEmbeddedFileNames[j]),aEmbeddedfileName);
           }
       }
   }
   CPPUNIT_ASSERT(bEmbeddings); // Grab Bag has all the expected elements
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

CPPUNIT_TEST_FIXTURE(Chart2ExportTest, testGapWidthXLSX)
{
    loadFromFile(u"xlsx/gapWidth.xlsx");

    uno::Reference< chart2::XChartDocument > xChartDoc = getChartDocFromSheet( 0, mxComponent );
    checkSheetForGapWidthAndOverlap(xChartDoc, 120, -60);

    xChartDoc = getChartDocFromSheet( 1, mxComponent );
    checkSheetForGapWidthAndOverlap(xChartDoc, 50, 30);

    saveAndReload("Calc Office Open XML");

    xChartDoc = getChartDocFromSheet( 0, mxComponent );
    checkSheetForGapWidthAndOverlap(xChartDoc, 120, -60);

    xChartDoc = getChartDocFromSheet( 1, mxComponent );
    checkSheetForGapWidthAndOverlap(xChartDoc, 50, 30);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest, testSmoothedLines)
{
    loadFromFile(u"ods/smoothedLines.ods");
    save("Calc Office Open XML");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart1.xml");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:ser[1]/c:smooth"_ostr, "val"_ostr, "0");
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest, testLabelStringODS)
{
    loadFromFile(u"ods/labelString.ods");

    uno::Reference< chart2::XChartDocument > xChartDoc = getChartDocFromSheet( 0, mxComponent );
    Reference< chart2::data::XDataSequence > xLabelSeq =
        getLabelDataSequenceFromDoc(xChartDoc);
    CPPUNIT_ASSERT(xLabelSeq.is());

    OUString aLabelString = xLabelSeq->getSourceRangeRepresentation();
    CPPUNIT_ASSERT_EQUAL(OUString("\"LabelName\""), aLabelString);

    saveAndReload("calc8");

    xChartDoc = getChartDocFromSheet( 0, mxComponent );
    xLabelSeq = getLabelDataSequenceFromDoc(xChartDoc);
    CPPUNIT_ASSERT(xLabelSeq.is());

    aLabelString = xLabelSeq->getSourceRangeRepresentation();
    CPPUNIT_ASSERT_EQUAL(OUString("\"LabelName\""), aLabelString);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
