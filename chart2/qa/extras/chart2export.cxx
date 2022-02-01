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

class Chart2ExportTest : public ChartTest
{
protected:

    virtual void registerNamespaces(xmlXPathContextPtr& pXmlXPathCtx) override;
public:
    Chart2ExportTest() : ChartTest() {}

    void testErrorBarXLSX();
    void testErrorBarPropXLSX();
    void testTrendline();
    void testTrendlineOOXML();
    void testTrendlineXLS();
    void testMovingAverage();
    void testStockChart();
    void testBarChart();
    void testCrosses();
    void testScatterChartTextXValues();
    void testScatterXAxisValues();
    void testScatterXAxisCategories();
    void testChartDataTable();
    void testChartExternalData();
    void testEmbeddingsGrabBag();
    void testAreaChartLoad();
    void testUpDownBars();
    void testDoughnutChart();
    void testDisplayUnits();
    // void testFdo74115WallGradientFill();
    void testFdo74115WallBitmapFill();
    void testPieChartWallLineStyle();
    void testBarChartRotation();
    void testShapeFollowedByChart();
    void testPieChartDataLabels();
    void testSeriesIdxOrder();
    void testScatterPlotLabels();
    void testErrorBarDataRangeODS();
    void testChartCrash();
    void testPieChartRotation();
    void testEmbeddingsOleObjectGrabBag();
    void testGapWidthXLSX();
    void testSmoothedLines();
    void testLabelStringODS();
    void testFdo78290LineChartMarkerX();
    void testFdo78290ScatterChartMarkerX();
    void testFdo78290CombinationChartMarkerX();
    void testTdf126115IndividualMarker();
    void testAxisNumberFormatODS();
    void testAxisNumberFormatXLS();
    void testDataLabelBordersDOCX();
    void testDataLabel3DChartDOCX();
    void testDataLabelBarChartDOCX();
    void testDataLabelClusteredBarChartDOCX();
    void testDataLabelRadarChartDOCX();
    void testDataLabelDoughnutChartDOCX();
    void testDataLabelAreaChartDOCX();
    void testDataLabelDefaultLineChartDOCX();
    void testIndividualDataLabelProps();
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
    void testMultipleAxisXLSX();
    void testSecondaryAxisXLSX();
    void testBarChartSecondaryAxisXLSX();

    CPPUNIT_TEST_SUITE(Chart2ExportTest);
    CPPUNIT_TEST(testErrorBarXLSX);
    CPPUNIT_TEST(testErrorBarPropXLSX);
    CPPUNIT_TEST(testTrendline);
    CPPUNIT_TEST(testTrendlineOOXML);
    CPPUNIT_TEST(testTrendlineXLS);
    CPPUNIT_TEST(testMovingAverage);
    CPPUNIT_TEST(testStockChart);
    CPPUNIT_TEST(testBarChart);
    CPPUNIT_TEST(testCrosses);
    CPPUNIT_TEST(testScatterChartTextXValues);
    CPPUNIT_TEST(testScatterXAxisValues);
    CPPUNIT_TEST(testScatterXAxisCategories);
    CPPUNIT_TEST(testChartDataTable);
    CPPUNIT_TEST(testChartExternalData);
    CPPUNIT_TEST(testEmbeddingsGrabBag);
    CPPUNIT_TEST(testAreaChartLoad);
    CPPUNIT_TEST(testUpDownBars);
    CPPUNIT_TEST(testDoughnutChart);
    CPPUNIT_TEST(testDisplayUnits);
    // CPPUNIT_TEST(testFdo74115WallGradientFill);
    CPPUNIT_TEST(testFdo74115WallBitmapFill);
    CPPUNIT_TEST(testPieChartWallLineStyle);
    CPPUNIT_TEST(testBarChartRotation);
    CPPUNIT_TEST(testShapeFollowedByChart);
    CPPUNIT_TEST(testPieChartDataLabels);
    CPPUNIT_TEST(testSeriesIdxOrder);
    CPPUNIT_TEST(testScatterPlotLabels);
    CPPUNIT_TEST(testErrorBarDataRangeODS);
    CPPUNIT_TEST(testChartCrash);
    CPPUNIT_TEST(testPieChartRotation);
    CPPUNIT_TEST(testEmbeddingsOleObjectGrabBag);
    CPPUNIT_TEST(testGapWidthXLSX);
    CPPUNIT_TEST(testSmoothedLines);
    CPPUNIT_TEST(testLabelStringODS);
    CPPUNIT_TEST(testFdo78290LineChartMarkerX);
    CPPUNIT_TEST(testFdo78290ScatterChartMarkerX);
    CPPUNIT_TEST(testFdo78290CombinationChartMarkerX);
    CPPUNIT_TEST(testTdf126115IndividualMarker);
    CPPUNIT_TEST(testAxisNumberFormatODS);
    CPPUNIT_TEST(testAxisNumberFormatXLS);
    CPPUNIT_TEST(testDataLabelBordersDOCX);
    CPPUNIT_TEST(testDataLabel3DChartDOCX);
    CPPUNIT_TEST(testDataLabelBarChartDOCX);
    CPPUNIT_TEST(testDataLabelClusteredBarChartDOCX);
    CPPUNIT_TEST(testDataLabelRadarChartDOCX);
    CPPUNIT_TEST(testDataLabelDoughnutChartDOCX);
    CPPUNIT_TEST(testDataLabelAreaChartDOCX);
    CPPUNIT_TEST(testDataLabelDefaultLineChartDOCX);
    CPPUNIT_TEST(testIndividualDataLabelProps);
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
    CPPUNIT_TEST(testMultipleAxisXLSX);
    CPPUNIT_TEST(testSecondaryAxisXLSX);
    CPPUNIT_TEST(testBarChartSecondaryAxisXLSX);

    CPPUNIT_TEST_SUITE_END();
};

void Chart2ExportTest::registerNamespaces(xmlXPathContextPtr& pXmlXPathCtx)
{
    XmlTestTools::registerOOXMLNamespaces(pXmlXPathCtx);
}

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
        bool aExpectedShowEquation, bool aExpectedR2)
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
        /*aExpectedShowEquation*/true, /*aExpectedR2*/false);
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
        /*aExpectedShowEquation*/true, /*aExpectedR2*/true);
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
void Chart2ExportTest::testErrorBarXLSX()
{
    load(u"/chart2/qa/extras/data/ods/", "error_bar.ods");
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

    reload("Calc Office Open XML");
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

void Chart2ExportTest::testErrorBarPropXLSX()
{
    load(u"/chart2/qa/extras/data/xlsx/", "testErrorBarProp.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart","Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);

    // test y error bars property
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:scatterChart/c:ser/c:errBars[1]/c:errDir", "val", "y");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:scatterChart/c:ser/c:errBars[1]/c:spPr/a:ln", "w", "12600");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:scatterChart/c:ser/c:errBars[1]/c:spPr/a:ln/a:solidFill/a:srgbClr", "val", "ff0000");

    // test x error bars property
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:scatterChart/c:ser/c:errBars[2]/c:errDir", "val", "x");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:scatterChart/c:ser/c:errBars[2]/c:spPr/a:ln", "w", "9360");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:scatterChart/c:ser/c:errBars[2]/c:spPr/a:ln/a:solidFill/a:srgbClr", "val", "595959");
}

// This method tests the preservation of properties for trendlines / regression curves
// in an export -> import cycle using different file formats - ODS, XLS and XLSX.
void Chart2ExportTest::testTrendline()
{
    mbSkipValidation = true;
    load(u"/chart2/qa/extras/data/ods/", "trendline.ods");
    checkTrendlinesInChart(getChartDocFromSheet( 0, mxComponent));
    reload("calc8");
    checkTrendlinesInChart(getChartDocFromSheet( 0, mxComponent));
}

void Chart2ExportTest::testTrendlineOOXML()
{
    load(u"/chart2/qa/extras/data/ods/", "trendline.ods");
    checkTrendlinesInChart(getChartDocFromSheet( 0, mxComponent));
    reload("Calc Office Open XML");
    checkTrendlinesInChart(getChartDocFromSheet( 0, mxComponent));
}

void Chart2ExportTest::testTrendlineXLS()
{
    mbSkipValidation = true;
    load(u"/chart2/qa/extras/data/ods/", "trendline.ods");
    checkTrendlinesInChart(getChartDocFromSheet( 0, mxComponent));
    reload("MS Excel 97");
    checkTrendlinesInChart(getChartDocFromSheet( 0, mxComponent));
}

void Chart2ExportTest::testMovingAverage()
{
    mbSkipValidation = true;
    load(u"/chart2/qa/extras/data/ods/", "moving-type.ods");
    reload("calc8");

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

void Chart2ExportTest::testStockChart()
{
    /*  For attached file Stock_Chart.docx, in chart1.xml,
     * <c:stockChart>, there are four types of series as
     * Open,Low,High and Close.
     * For Open series, in <c:idx val="0" />
     * an attribute val of index should start from 1 and not from 0.
     * Which was problem area.
     */
    load(u"/chart2/qa/extras/data/docx/", "testStockChart.docx");

    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart", "Office Open XML Text");
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:stockChart/c:ser[1]/c:idx", "val", "1");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:stockChart/c:ser[1]/c:order", "val", "1");
    assertXPathContent(
        pXmlDoc,
        "/c:chartSpace/c:chart/c:plotArea/c:stockChart/c:ser[1]/c:tx/c:strRef/c:strCache/c:pt/c:v",
        "Open");
}

void Chart2ExportTest::testBarChart()
{
    load(u"/chart2/qa/extras/data/docx/", "testBarChart.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart", "Office Open XML Text");
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:barDir", "val", "col");
}

void Chart2ExportTest::testCrosses()
{
    // test crosses val="autoZero" with DOCX
    {
        load(u"/chart2/qa/extras/data/docx/", "Bar_horizontal_cone.docx");
        xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart", "Office Open XML Text");

        assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:catAx/c:crosses", "val", "autoZero");
    }
    // tdf#142351: test crossesAt val="-50" with XLSX
    {
        load(u"/chart2/qa/extras/data/xlsx/", "tdf142351.xlsx");
        xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
        CPPUNIT_ASSERT(pXmlDoc);

        assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:catAx/c:crossesAt", "val", "-50");
    }
}

void Chart2ExportTest::testScatterChartTextXValues()
{
    load(u"/chart2/qa/extras/data/docx/", "scatter-chart-text-x-values.docx");

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
    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart", "Office Open XML Text");
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPathContent(pXmlDoc, "//c:scatterChart/c:ser[1]/c:xVal[1]/c:numRef[1]/c:numCache[1]/c:pt[1]/c:v[1]", "1");
}

void Chart2ExportTest::testScatterXAxisValues()
{
    load(u"/chart2/qa/extras/data/odt/", "tdf114657.odt");

    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart", "Office Open XML Text");
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "//c:scatterChart/c:ser/c:xVal/c:numRef/c:numCache/c:ptCount", "val", "5");
    assertXPathContent(pXmlDoc, "//c:scatterChart/c:ser/c:xVal/c:numRef/c:numCache/c:pt[1]/c:v", "15");
    assertXPathContent(pXmlDoc, "//c:scatterChart/c:ser/c:xVal/c:numRef/c:numCache/c:pt[2]/c:v", "11");
    assertXPathContent(pXmlDoc, "//c:scatterChart/c:ser/c:xVal/c:numRef/c:numCache/c:pt[3]/c:v", "20");
    assertXPathContent(pXmlDoc, "//c:scatterChart/c:ser/c:xVal/c:numRef/c:numCache/c:pt[4]/c:v", "16");
}

void Chart2ExportTest::testScatterXAxisCategories()
{
    load(u"/chart2/qa/extras/data/odt/", "tdf131143.odt");

    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart", "Office Open XML Text");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "//c:scatterChart/c:ser[1]/c:xVal/c:strRef/c:strCache/c:ptCount", "val", "4");
    assertXPathContent(pXmlDoc, "//c:scatterChart/c:ser[1]/c:xVal/c:strRef/c:strCache/c:pt[1]/c:v", "Row 1");
    assertXPathContent(pXmlDoc, "//c:scatterChart/c:ser[1]/c:xVal/c:strRef/c:strCache/c:pt[2]/c:v", "Row 2");
}

void Chart2ExportTest::testChartDataTable()
{
    load(u"/chart2/qa/extras/data/docx/", "testChartDataTable.docx");

    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart", "Office Open XML Text");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:dTable/c:showHorzBorder", "val", "1");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:dTable/c:showVertBorder", "val", "1");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:dTable/c:showOutline", "val", "1");
}

void Chart2ExportTest::testChartExternalData()
{
    load(u"/chart2/qa/extras/data/docx/", "testMultipleChart.docx");

    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart", "Office Open XML Text");
    CPPUNIT_ASSERT(pXmlDoc);
    xmlXPathObjectPtr pXmlPathObj = getXPathNode(pXmlDoc, "/c:chartSpace/c:externalData");
    xmlNodeSetPtr pXmlNodes = pXmlPathObj->nodesetval;
    CPPUNIT_ASSERT(pXmlNodes);
}

void Chart2ExportTest::testEmbeddingsGrabBag()
{
   // The problem was that .xlsx files were missing from docx file from embeddings folder
   // after saving file.
   // This test case tests whether embeddings files grabbagged properly in correct object.

   load(u"/chart2/qa/extras/data/docx/", "testMultiplechartembeddings.docx" );
   uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
   uno::Reference<beans::XPropertySet> xTextDocumentPropertySet(xTextDocument, uno::UNO_QUERY);
   uno::Sequence<beans::PropertyValue> aGrabBag(0);
   xTextDocumentPropertySet->getPropertyValue("InteropGrabBag") >>= aGrabBag;
   CPPUNIT_ASSERT(aGrabBag.hasElements()); // Grab Bag not empty
   bool bEmbeddings = false;
   const char* const testEmbeddedFileNames[] {"word/embeddings/Microsoft_Excel_Worksheet3.xlsx",
                                        "word/embeddings/Microsoft_Excel_Worksheet2.xlsx",
                                        "word/embeddings/Microsoft_Excel_Worksheet1.xlsx"};
   for(beans::PropertyValue const & prop : std::as_const(aGrabBag))
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

void Chart2ExportTest::testAreaChartLoad()
{
    load (u"/chart2/qa/extras/data/docx/", "testAreaChartLoad.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart","Office Open XML Text");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:areaChart/c:ser/c:dLbls/c:showVal", "val", "1");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:areaChart/c:ser/c:dLbls/c:dLbl", 0);
}

void Chart2ExportTest::testUpDownBars()
{
    /*
    load("/chart2/qa/extras/data/docx/", "UpDownBars.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart", "Office Open XML Text");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:upDownBars");
    */
}

void Chart2ExportTest::testDoughnutChart()
{
    load(u"/chart2/qa/extras/data/docx/", "doughnutChart.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart", "Office Open XML Text");
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:doughnutChart");
}

void Chart2ExportTest::testDisplayUnits()
{
    load(u"/chart2/qa/extras/data/docx/", "DisplayUnits.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart", "Office Open XML Text");
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx/c:dispUnits/c:builtInUnit", "val", "billions");
}

// void Chart2ExportTest::testFdo74115WallGradientFill()
// {
//     load("/chart2/qa/extras/data/docx/", "fdo74115_WallGradientFill.docx");
//     xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart", "Office Open XML Text");
//     CPPUNIT_ASSERT(pXmlDoc);
//
//     assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:spPr/a:gradFill");
// }

void Chart2ExportTest::testFdo74115WallBitmapFill()
{
    load(u"/chart2/qa/extras/data/docx/", "fdo74115_WallBitmapFill.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart", "Office Open XML Text");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:spPr/a:blipFill");
}

void Chart2ExportTest::testPieChartWallLineStyle()
{
    load(u"/chart2/qa/extras/data/odt/", "testPieChartWallLineStyle.odt");
    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart", "Office Open XML Text");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:spPr/a:ln/a:noFill");
}

//The below test case tests the built in marker 'x' for Office 2010 in Line charts

void Chart2ExportTest::testFdo78290LineChartMarkerX()
{
    load(u"/chart2/qa/extras/data/docx/", "fdo78290_Line_Chart_Marker_x.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart", "Office Open XML Text");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace[1]/c:chart[1]/c:plotArea[1]/c:lineChart[1]/c:ser[1]/c:marker[1]/c:symbol[1]","val","x");
    assertXPath(pXmlDoc, "/c:chartSpace[1]/c:chart[1]/c:plotArea[1]/c:lineChart[1]/c:ser[1]/c:marker[1]/c:size[1]","val","7");
}

// We can also use the built in marker 'x' in scatter chart, hence writing the test case for the same.

void Chart2ExportTest::testFdo78290ScatterChartMarkerX()
{
    load(u"/chart2/qa/extras/data/docx/", "fdo78290_Scatter_Chart_Marker_x.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart", "Office Open XML Text");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace[1]/c:chart[1]/c:plotArea[1]/c:scatterChart[1]/c:ser[1]/c:marker[1]/c:symbol[1]","val","x");
    assertXPath(pXmlDoc, "/c:chartSpace[1]/c:chart[1]/c:plotArea[1]/c:scatterChart[1]/c:ser[1]/c:marker[1]/c:size[1]","val","7");
}

// Also in a combination of charts like a column chart and line chart, we can use the built in marker 'x'
// for the line chart too. hence put a test case for the combination chart also.

void Chart2ExportTest::testFdo78290CombinationChartMarkerX()
{
    load(u"/chart2/qa/extras/data/docx/", "fdo78290_Combination_Chart_Marker_x.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart", "Office Open XML Text");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace[1]/c:chart[1]/c:plotArea[1]/c:lineChart[1]/c:ser[1]/c:marker[1]/c:symbol[1]","val","x");
    assertXPath(pXmlDoc, "/c:chartSpace[1]/c:chart[1]/c:plotArea[1]/c:lineChart[1]/c:ser[1]/c:marker[1]/c:size[1]","val","7");
}

void Chart2ExportTest::testTdf126115IndividualMarker()
{
    // Check individual marker properties.
    load(u"/chart2/qa/extras/data/xlsx/", "tdf126115.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);
    // 1. series
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:scatterChart/c:ser[1]/c:dPt/c:marker/c:symbol", "val", "square");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:scatterChart/c:ser[1]/c:dPt/c:marker/c:size", "val", "8");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:scatterChart/c:ser[1]/c:dPt/c:marker/c:spPr/a:solidFill/a:srgbClr", "val", "ff0000");
    // 2. series
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:scatterChart/c:ser[2]/c:dPt/c:marker/c:symbol", "val", "x");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:scatterChart/c:ser[2]/c:dPt/c:marker/c:size", "val", "15");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:scatterChart/c:ser[2]/c:dPt/c:marker/c:spPr/a:solidFill/a:srgbClr", "val", "7030a0");
}

void Chart2ExportTest::testAxisNumberFormatODS()
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

    load(u"/chart2/qa/extras/data/ods/", "axis-numformats-linked.ods");

    Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    aTest.check(xChartDoc);

    // Reload the document and make sure everything remains intact.
    reload("calc8");
    xChartDoc = getChartDocFromSheet(0, mxComponent);
    aTest.check(xChartDoc);
}

void Chart2ExportTest::testAxisNumberFormatXLS()
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

    load( u"/chart2/qa/extras/data/xls/", "axis_sourceformatting.xls" );

    Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet( 0, mxComponent );
    aTest.check( xChartDoc, true, util::NumberFormat::PERCENT );

    aTest.change( xChartDoc, util::NumberFormat::NUMBER );
    // Write the document(xls) with changes made close it, load it and check if changes are intact
    reload( "MS Excel 97" );
    xChartDoc = getChartDocFromSheet( 0, mxComponent );
    aTest.check( xChartDoc, false, util::NumberFormat::NUMBER );
}

void Chart2ExportTest::testDataLabelBordersDOCX()
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

    load(u"/chart2/qa/extras/data/docx/", "data-label-borders.docx");

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

    reload("Office Open XML Text");

    xChartDoc.set(getChartDocFromWriter(0), uno::UNO_QUERY);
    aTest.checkObject1(xChartDoc);
    xChartDoc.set(getChartDocFromWriter(1), uno::UNO_QUERY);
    aTest.checkObject2(xChartDoc);
}

void Chart2ExportTest::testDataLabel3DChartDOCX()
{
    load(u"/chart2/qa/extras/data/docx/", "3d-bar-label.docx");

    Reference<chart2::XChartDocument> xChartDoc(getChartDocFromWriter(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());

    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart","Office Open XML Text");
    CPPUNIT_ASSERT(pXmlDoc);

    // We must not export label position attributes for 3D bar charts. The
    // same rule also applies to several other 3D charts, apparently.
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:bar3DChart/c:ser/c:dLbls/c:dLblPos", 0);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:bar3DChart/c:ser/c:dLbls/c:dLbl/c:dLblPos", 0);
}

void Chart2ExportTest::testDataLabelBarChartDOCX()
{
    load(u"/chart2/qa/extras/data/docx/", "bar-chart-labels.docx");

    Reference<chart2::XChartDocument> xChartDoc(getChartDocFromWriter(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());

    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart","Office Open XML Text");
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[1]/c:dLbls/c:dLblPos", "val", "ctr");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[2]/c:dLbls/c:dLblPos", "val", "inEnd");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[3]/c:dLbls/c:dLblPos", "val", "inBase");
}

void Chart2ExportTest::testDataLabelClusteredBarChartDOCX()
{
    load(u"/chart2/qa/extras/data/docx/", "clustered-bar-chart-labels.docx");

    Reference<chart2::XChartDocument> xChartDoc(getChartDocFromWriter(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());

    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart","Office Open XML Text");
    CPPUNIT_ASSERT(pXmlDoc);

    // This was "t", should be one of the allowed values.
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[1]/c:dLbls/c:dLbl[2]/c:dLblPos", "val", "outEnd");
}

void Chart2ExportTest::testDataLabelRadarChartDOCX()
{
    load(u"/chart2/qa/extras/data/docx/", "radar-chart-labels.docx");

    Reference<chart2::XChartDocument> xChartDoc(getChartDocFromWriter(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());

    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart","Office Open XML Text");
    CPPUNIT_ASSERT(pXmlDoc);

    // We must not export label position attributes for radar charts.
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:radarChart/c:ser/c:dLbls/c:dLblPos", 0);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:radarChart/c:ser/c:dLbls/c:dLbl/c:dLblPos", 0);
}

void Chart2ExportTest::testDataLabelDoughnutChartDOCX()
{
    load(u"/chart2/qa/extras/data/docx/", "doughnut-chart-labels.docx");

    Reference<chart2::XChartDocument> xChartDoc(getChartDocFromWriter(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());

    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart","Office Open XML Text");
    CPPUNIT_ASSERT(pXmlDoc);

    // We must not export label position attributes for doughnut charts.
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:doughnutChart/c:ser/c:dLbls/c:dLblPos", 0);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:doughnutChart/c:ser/c:dLbls/c:dLbl/c:dLblPos", 0);
}

void Chart2ExportTest::testDataLabelAreaChartDOCX()
{
    load(u"/chart2/qa/extras/data/docx/", "area-chart-labels.docx");

    Reference<chart2::XChartDocument> xChartDoc(getChartDocFromWriter(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());

    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart","Office Open XML Text");
    CPPUNIT_ASSERT(pXmlDoc);

    // We must not export label position attributes for area charts.
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:areaChart/c:ser/c:dLbls/c:dLblPos", 0);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:areaChart/c:ser/c:dLbls/c:dLbl/c:dLblPos", 0);
}

void Chart2ExportTest::testDataLabelDefaultLineChartDOCX()
{
    // This file was created by Word 2007, which doesn't provide default data
    // label position (2010 does).  Make sure its default data label position
    // is RIGHT when exporting.

    load(u"/chart2/qa/extras/data/docx/", "line-chart-label-default-placement.docx");

    Reference<chart2::XChartDocument> xChartDoc(getChartDocFromWriter(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());

    reload("Office Open XML Text");

    xChartDoc.set(getChartDocFromWriter(0), uno::UNO_QUERY);
    Reference<chart2::XDataSeries> xDataSeries = getDataSeriesFromDoc(xChartDoc, 0);
    Reference<beans::XPropertySet> xPropSet(xDataSeries, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xPropSet.is());
    sal_Int32 nLabelPlacement = -1;
    if (xPropSet->getPropertyValue("LabelPlacement") >>= nLabelPlacement)
        // This option may not be set.  Check its value only when it's set.
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Line chart's default label placement should be 'right'.", chart::DataLabelPlacement::RIGHT, nLabelPlacement );
}

void Chart2ExportTest::testIndividualDataLabelProps()
{
    load(u"/chart2/qa/extras/data/xlsx/", "tdf122915.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart","Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:scatterChart/c:ser[3]/c:dLbls/c:dLbl/c:txPr/a:p/a:pPr/a:defRPr", "b", "1");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:scatterChart/c:ser[3]/c:dLbls/c:dLbl/c:txPr/a:p/a:pPr/a:defRPr", "sz", "1600");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:scatterChart/c:ser[3]/c:dLbls/c:dLbl/c:txPr/a:p/a:pPr/a:defRPr/a:solidFill/a:srgbClr", "val", "ff0000");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:scatterChart/c:ser[3]/c:dLbls/c:dLbl/c:txPr/a:p/a:pPr/a:defRPr/a:latin", "typeface", "Times New Roman");
}

void Chart2ExportTest::testTdf108107()
{
    load(u"/chart2/qa/extras/data/xlsx/", "tdf108107.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:ser/c:dLbls/c:dLbl[1]/c:idx", "val", "1");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:ser/c:dLbls/c:dLbl[1]/c:txPr/a:p/a:pPr/a:defRPr", "b", "1");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:ser/c:dLbls/c:dLbl[1]/c:txPr/a:p/a:pPr/a:defRPr", "sz", "2000");
}

void Chart2ExportTest::testTdf114139()
{
    load(u"/chart2/qa/extras/data/xlsx/", "tdf114139.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:pie3DChart", 1);
    //no fill
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:spPr", 0);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:spPr/a:solidFill", 0);
}

void Chart2ExportTest::testTdf64224()
{
    load(u"/chart2/qa/extras/data/ods/", "tdf64224.ods");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);

    //no fill
    assertXPath(pXmlDoc, "/c:chartSpace/c:spPr/a:noFill", 1);
    assertXPath(pXmlDoc, "/c:chartSpace/c:spPr/a:solidFill", 0);
}

void Chart2ExportTest::testChartTitlePropertiesColorFillDOCX()
{
    load(u"/chart2/qa/extras/data/docx/", "testChartTitlePropertiesColorFill.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart","Office Open XML Text");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:solidFill/a:srgbClr", "val", "ff0000");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:ln/a:noFill", 1);
}

void Chart2ExportTest::testChartTitlePropertiesGradientFillDOCX()
{
    load(u"/chart2/qa/extras/data/docx/", "testChartTitlePropertiesGradientFill.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart","Office Open XML Text");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:gradFill/a:gsLst/a:gs[1]/a:srgbClr", "val", "cccccc");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:gradFill/a:gsLst/a:gs[2]/a:srgbClr", "val", "666666");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:ln/a:noFill", 1);
}

void Chart2ExportTest::testChartTitlePropertiesBitmapFillDOCX()
{
    load(u"/chart2/qa/extras/data/docx/", "testChartTitlePropertiesBitmapFill.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart","Office Open XML Text");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:blipFill/a:blip", "embed", "rId1");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:ln/a:noFill", 1);
}

void Chart2ExportTest::testColorGradientWithTransparencyDOCX()
{
    // Test color gradient (two color) with gradient transparency
    load(u"/chart2/qa/extras/data/docx/", "testColorGradientWithTransparency.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart", "Office Open XML Text");
    CPPUNIT_ASSERT(pXmlDoc);
    // Test the transparency of the first color
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:spPr/a:gradFill/a:gsLst/a:gs[1]/a:srgbClr/a:alpha", "val", "60000");
    // Test the transparency of the second color
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:spPr/a:gradFill/a:gsLst/a:gs[2]/a:srgbClr/a:alpha", "val", "90196");
}

void Chart2ExportTest::testColorGradientWithTransparencyODS()
{
    // Test color gradient (two color) with simple transparency
    load(u"/chart2/qa/extras/data/ods/", "testColorGradientWithTransparency.ods");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);
    // Test the transparency of the first color
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:spPr/a:gradFill/a:gsLst/a:gs[1]/a:srgbClr/a:alpha", "val", "60000");
    // Test the transparency of the second color
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:spPr/a:gradFill/a:gsLst/a:gs[2]/a:srgbClr/a:alpha", "val", "60000");
}

void Chart2ExportTest::testColorGradientStopXLSX()
{
    // Test color gradient (two color) stop of the first color
    load(u"/chart2/qa/extras/data/xlsx/", "tdf128619.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);
    // Test the position of the first color
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:spPr/a:gradFill/a:gsLst/a:gs[1]", "pos", "45000");
    // Test the position of the second color
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:spPr/a:gradFill/a:gsLst/a:gs[2]", "pos", "100000");
}

void Chart2ExportTest::testRadialColorGradientDOCX()
{
    load(u"/chart2/qa/extras/data/docx/", "tdf128794.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart", "Office Open XML Text");
    CPPUNIT_ASSERT(pXmlDoc);
    // Test the gradient style (if there is no 'a:path' attribute, it is a linear gradient)
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:spPr/a:gradFill/a:path", 0);
    // Test the linear gradient angle
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:spPr/a:gradFill/a:lin", "ang", "13500000");
}

void Chart2ExportTest::testBarChartDataPointPropDOCX()
{
    load(u"/chart2/qa/extras/data/docx/", "testBarChartDataPointPropDOCX.docx");
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

void Chart2ExportTest::testBarChartRotation()
{
    load (u"/chart2/qa/extras/data/docx/", "barChartRotation.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart","Office Open XML Text");
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:view3D/c:rotX", "val", "30");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:view3D/c:rotY", "val", "50");
}

void Chart2ExportTest::testShapeFollowedByChart()
{
    /* If there is a scenario where a chart is followed by a shape
       which is being exported as an alternate content then, the
       docPr Id is being repeated, ECMA 20.4.2.5 says that the
       docPr Id should be unique, ensuring the same here.
    */
    load(u"/chart2/qa/extras/data/docx/", "FDO74430.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document", "Office Open XML Text" );
    CPPUNIT_ASSERT(pXmlDoc);

    OUString aValueOfFirstDocPR = getXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[1]/w:drawing[1]/wp:inline[1]/wp:docPr[1]", "id");
    OUString aValueOfSecondDocPR = getXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[1]/mc:AlternateContent[1]/mc:Choice[1]/w:drawing[1]/wp:anchor[1]/wp:docPr[1]", "id");

    CPPUNIT_ASSERT( aValueOfFirstDocPR != aValueOfSecondDocPR );
}

void Chart2ExportTest::testPieChartDataLabels()
{
    load(u"/chart2/qa/extras/data/docx/", "PieChartDataLabels.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart", "Office Open XML Text");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:pie3DChart/c:ser[1]/c:dLbls/c:dLbl[1]/c:dLblPos", "val", "outEnd");
}

void Chart2ExportTest::testSeriesIdxOrder()
{
    load(u"/chart2/qa/extras/data/docx/", "testSeriesIdxOrder.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart", "Office Open XML Text");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace[1]/c:chart[1]/c:plotArea[1]/c:lineChart[1]/c:ser[1]/c:idx[1]", "val", "1");
    assertXPath(pXmlDoc, "/c:chartSpace[1]/c:chart[1]/c:plotArea[1]/c:lineChart[1]/c:ser[1]/c:order[1]", "val", "1");
}

void Chart2ExportTest::testScatterPlotLabels()
{
    load(u"/chart2/qa/extras/data/odt/", "scatter-plot-labels.odt");
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
    reload("writer8");

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

void Chart2ExportTest::testErrorBarDataRangeODS()
{
    load(u"/chart2/qa/extras/data/ods/", "ErrorBarRange.ods");
    reload("calc8");

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

void Chart2ExportTest::testChartCrash()
{
    load(u"/chart2/qa/extras/data/docx/", "FDO75975.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart", "Office Open XML Text");
    CPPUNIT_ASSERT(pXmlDoc);
}

void Chart2ExportTest::testPieChartRotation()
{
    load (u"/chart2/qa/extras/data/docx/", "pieChartRotation.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart","Office Open XML Text");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:view3D/c:rotX", "val", "40");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:view3D/c:rotY", "val", "30");
}

void Chart2ExportTest::testEmbeddingsOleObjectGrabBag()
{
   // The problem was that .bin files were missing from docx file from embeddings folder
   // after saving file.
   // This test case tests whether embeddings files grabbagged properly in correct object.

   load(u"/chart2/qa/extras/data/docx/", "testchartoleobjectembeddings.docx" );
   uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
   uno::Reference<beans::XPropertySet> xTextDocumentPropertySet(xTextDocument, uno::UNO_QUERY);
   uno::Sequence<beans::PropertyValue> aGrabBag(0);
   xTextDocumentPropertySet->getPropertyValue("InteropGrabBag") >>= aGrabBag;
   CPPUNIT_ASSERT(aGrabBag.hasElements()); // Grab Bag not empty
   bool bEmbeddings = false;
   const char* const testEmbeddedFileNames[] = {"word/embeddings/oleObject1.bin"};
   for(beans::PropertyValue const & prop : std::as_const(aGrabBag))
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

void Chart2ExportTest::testGapWidthXLSX()
{
    load(u"/chart2/qa/extras/data/xlsx/", "gapWidth.xlsx");

    uno::Reference< chart2::XChartDocument > xChartDoc = getChartDocFromSheet( 0, mxComponent );
    checkSheetForGapWidthAndOverlap(xChartDoc, 120, -60);

    xChartDoc = getChartDocFromSheet( 1, mxComponent );
    checkSheetForGapWidthAndOverlap(xChartDoc, 50, 30);

    reload("Calc Office Open XML");

    xChartDoc = getChartDocFromSheet( 0, mxComponent );
    checkSheetForGapWidthAndOverlap(xChartDoc, 120, -60);

    xChartDoc = getChartDocFromSheet( 1, mxComponent );
    checkSheetForGapWidthAndOverlap(xChartDoc, 50, 30);
}

void Chart2ExportTest::testSmoothedLines()
{
    load(u"/chart2/qa/extras/data/ods/", "smoothedLines.ods");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:ser[1]/c:smooth", "val", "0");
}

void Chart2ExportTest::testLabelStringODS()
{
    load(u"/chart2/qa/extras/data/ods/", "labelString.ods");

    uno::Reference< chart2::XChartDocument > xChartDoc = getChartDocFromSheet( 0, mxComponent );
    Reference< chart2::data::XDataSequence > xLabelSeq =
        getLabelDataSequenceFromDoc(xChartDoc);
    CPPUNIT_ASSERT(xLabelSeq.is());

    OUString aLabelString = xLabelSeq->getSourceRangeRepresentation();
    CPPUNIT_ASSERT_EQUAL(OUString("\"LabelName\""), aLabelString);

    reload("calc8");

    xChartDoc = getChartDocFromSheet( 0, mxComponent );
    xLabelSeq = getLabelDataSequenceFromDoc(xChartDoc);
    CPPUNIT_ASSERT(xLabelSeq.is());

    aLabelString = xLabelSeq->getSourceRangeRepresentation();
    CPPUNIT_ASSERT_EQUAL(OUString("\"LabelName\""), aLabelString);
}

void Chart2ExportTest::testFdo83058dlblPos()
{
    load (u"/chart2/qa/extras/data/docx/", "fdo83058_dlblPos.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/charts/chart","Office Open XML Text");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[1]/c:dLbls[1]/c:dLbl[2]/c:dLblPos", "val", "outEnd");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[1]/c:dLbls[1]/c:dLbl[3]/c:dLblPos", "val", "outEnd");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[1]/c:dLbls[1]/c:dLbl[4]/c:dLblPos", "val", "outEnd");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[1]/c:dLbls[1]/c:dLbl[5]/c:dLblPos", "val", "outEnd");
}

void Chart2ExportTest::testAutoTitleDelXLSX()
{
    load(u"/chart2/qa/extras/data/xlsx/", "autotitledel_2007.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart","Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:autoTitleDeleted", "val", "0");
}

void Chart2ExportTest::testDispBlanksAsXLSX()
{
    load(u"/chart2/qa/extras/data/xlsx/", "dispBlanksAs_2007.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart","Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:dispBlanksAs", "val", "gap");
}

void Chart2ExportTest::testMarkerColorXLSX()
{
    load(u"/chart2/qa/extras/data/xlsx/", "markerColor.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:scatterChart/c:ser/c:marker/c:spPr/a:solidFill/a:srgbClr", "val", "92d050");
}

void Chart2ExportTest::testRoundedCornersXLSX()
{
    load(u"/chart2/qa/extras/data/xlsx/", "markerColor.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:roundedCorners", "val", "0");
}

void Chart2ExportTest::testAxisNumberFormatXLSX()
{
    load(u"/chart2/qa/extras/data/ods/", "axis_number_format.ods");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx", 2);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx[1]/c:numFmt", "formatCode", "0.00E+000");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx[1]/c:numFmt", "sourceLinked", "0");

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx[2]/c:numFmt", "formatCode", "[$$-409]#,##0;\\-[$$-409]#,##0");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx[2]/c:numFmt", "sourceLinked", "1");
}

void Chart2ExportTest::testDataPointLabelNumberFormatXLSX()
{
    load(u"/chart2/qa/extras/data/ods/", "tdf123774.ods");
    {
        xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
        CPPUNIT_ASSERT(pXmlDoc);
        assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:pieChart/c:ser/c:dLbls/c:numFmt", "formatCode", "[$-40E]0.00%");
        assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:pieChart/c:ser/c:dLbls/c:numFmt", "sourceLinked", "0");

        assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:pieChart/c:ser/c:dLbls/c:dLbl[1]/c:numFmt", "formatCode", "[$-40E]0.00%");
        assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:pieChart/c:ser/c:dLbls/c:dLbl[1]/c:numFmt", "sourceLinked", "0");
    }

    load(u"/chart2/qa/extras/data/xlsx/", "tdf130986.xlsx");
    {
        xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
        CPPUNIT_ASSERT(pXmlDoc);
        assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dLbls/c:dLbl/c:idx", "val", "1");
        assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dLbls/c:dLbl/c:numFmt", "formatCode", "0.00E+00");
        assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dLbls/c:dLbl/c:numFmt", "sourceLinked", "0");
    }
}

void Chart2ExportTest::testDataLabelDefaultValuesXLSX()
{
    load(u"/chart2/qa/extras/data/xlsx/", "data_label.xlsx");
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

void Chart2ExportTest::testDataLabelFillColor()
{
    load(u"/chart2/qa/extras/data/xlsx/", "data_labels_fill_color.xlsx");
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

void Chart2ExportTest::testTitleOverlayXLSX()
{
    load(u"/chart2/qa/extras/data/xlsx/", "chart_title.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:overlay", "val", "0");
}

void Chart2ExportTest::testInvertIfNegativeXLSX()
{
    load(u"/chart2/qa/extras/data/xlsx/", "bar_chart_simple.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:invertIfNegative", "val", "0");
}

void Chart2ExportTest::testBubble3DXLSX()
{
    load(u"/chart2/qa/extras/data/xlsx/", "bubble_chart_simple.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:bubbleChart/c:ser[1]/c:bubble3D", "val", "0");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:bubbleChart/c:ser[2]/c:bubble3D", "val", "0");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:bubbleChart/c:ser[3]/c:bubble3D", "val", "0");
}

void Chart2ExportTest::testNoMarkerXLSX()
{
    load(u"/chart2/qa/extras/data/xlsx/", "no_marker.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:ser[1]/c:marker/c:symbol", "val", "none");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:ser[2]/c:marker/c:symbol", "val", "none");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:marker", "val", "0");
}

void Chart2ExportTest::testTitleManualLayoutXLSX()
{
    load(u"/chart2/qa/extras/data/xlsx/", "title_manual_layout.xlsx");
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

void Chart2ExportTest::testPlotAreaManualLayoutXLSX()
{
    load(u"/chart2/qa/extras/data/xlsx/", "plot_area_manual_layout.xlsx");
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

void Chart2ExportTest::testLegendManualLayoutXLSX()
{
    load(u"/chart2/qa/extras/data/xlsx/", "legend_manual_layout.xlsx");
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

void Chart2ExportTest::testChartSubTitle()
{
    load(u"/chart2/qa/extras/data/ods/", "testChartSubTitle.ods");
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

void Chart2ExportTest::testChartMainWithSubTitle()
{
    load(u"/chart2/qa/extras/data/ods/", "testChartMainWithSubTitle.ods");
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

void Chart2ExportTest::testAutoTitleDeleted()
{
    load(u"/chart2/qa/extras/data/xlsx/", "testAutoTitleDeleted.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:autoTitleDeleted", "val", "1");
}

void Chart2ExportTest::testChartTitlePropertiesColorFillXLSX()
{
    load(u"/chart2/qa/extras/data/xlsx/", "testChartTitlePropertiesColorFill.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:solidFill/a:srgbClr", "val", "ff0000");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:ln/a:noFill", 1);
}

void Chart2ExportTest::testChartTitlePropertiesGradientFillXLSX()
{
    load(u"/chart2/qa/extras/data/xlsx/", "testChartTitlePropertiesGradientFill.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:gradFill/a:gsLst/a:gs[1]/a:srgbClr", "val", "cccccc");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:gradFill/a:gsLst/a:gs[2]/a:srgbClr", "val", "666666");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:ln/a:noFill", 1);
}

void Chart2ExportTest::testChartTitlePropertiesBitmapFillXLSX()
{
    load(u"/chart2/qa/extras/data/xlsx/", "testChartTitlePropertiesBitmapFill.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:blipFill/a:blip", "embed", "rId1");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:ln/a:noFill", 1);
}

void Chart2ExportTest::testBarChartDataPointPropXLSX()
{
    load(u"/chart2/qa/extras/data/xlsx/", "testBarChartDataPointPropXLSX.xlsx");
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

void Chart2ExportTest::testDataseriesOverlapStackedChartXLSX()
{
    load(u"/chart2/qa/extras/data/xlsx/", "testDataseriesOverlapStackedChart.xlsx");

    // test the overlap value of a simple Stacked Column Chart
    uno::Reference< chart2::XChartDocument > xChartDoc = getChartDocFromSheet( 0, mxComponent );
    checkSheetForGapWidthAndOverlap(xChartDoc, 100, 0);

    // test the overlap value of a Percent Stacked Bar Chart
    xChartDoc = getChartDocFromSheet( 1, mxComponent );
    checkSheetForGapWidthAndOverlap(xChartDoc, 100, 35);

    reload("Calc Office Open XML");

    xChartDoc = getChartDocFromSheet( 0, mxComponent );
    checkSheetForGapWidthAndOverlap(xChartDoc, 100, 100);

    xChartDoc = getChartDocFromSheet( 1, mxComponent );
    checkSheetForGapWidthAndOverlap(xChartDoc, 100, 100);
}

void Chart2ExportTest::testAxisCharacterPropertiesXLSX()
{
    load(u"/chart2/qa/extras/data/xlsx/", "axis_character_properties.xlsx");
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

void Chart2ExportTest::testTitleCharacterPropertiesXLSX()
{
    load(u"/chart2/qa/extras/data/xlsx/", "title_character_properties.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p/a:pPr/a:defRPr", "sz", "2400");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p/a:pPr/a:defRPr", "b", "1");

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p/a:r/a:rPr", "sz", "2400");
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:tx/c:rich/a:p/a:r/a:rPr", "b", "1");
}

void Chart2ExportTest::testPlotVisOnlyXLSX()
{
    load(u"/chart2/qa/extras/data/xlsx/", "hidden_cells.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotVisOnly", "val", "0");
}

void Chart2ExportTest::testBarChartVaryColorsXLSX()
{
    load(u"/chart2/qa/extras/data/xlsx/", "tdf90876.xlsx");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:varyColors", "val", "0");
}

void Chart2ExportTest::testTdf96161()
{
    load(u"/chart2/qa/extras/data/ods/", "tdf96161.ods");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/charts/chart", "Calc Office Open XML");
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:varyColors", "val", "0");
}

void Chart2ExportTest::testMultipleAxisXLSX()
{
    load(u"/chart2/qa/extras/data/ods/", "multiple_axis.ods");
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

void Chart2ExportTest::testSecondaryAxisXLSX()
{
    load(u"/chart2/qa/extras/data/ods/", "secondary_axis.ods");
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

void Chart2ExportTest::testBarChartSecondaryAxisXLSX()
{
    load(u"/chart2/qa/extras/data/xlsx/", "testSecondaryAxis.xlsx");
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

CPPUNIT_TEST_SUITE_REGISTRATION(Chart2ExportTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
