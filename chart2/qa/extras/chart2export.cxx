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
#include <com/sun/star/lang/XServiceName.hpp>

using uno::Reference;
using beans::XPropertySet;

class Chart2ExportTest : public ChartTest
{
public:
    void test();
    void testErrorBarXLSX();
    void testTrendline();

    void testStockChart();

    CPPUNIT_TEST_SUITE(Chart2ExportTest);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST(testErrorBarXLSX);
    CPPUNIT_TEST(testTrendline);
//    CPPUNIT_TEST(testStockChart); disable pending necessary patch from gerrit 6957
    CPPUNIT_TEST_SUITE_END();

private:
};

void Chart2ExportTest::test()
{
    load("/chart2/qa/extras/data/ods/", "simple_export_chart.ods");
    reload("Calc Office Open XML");
}

namespace {

void testErrorBar( Reference< XPropertySet > xErrorBar )
{
    sal_Int32 nErrorBarStyle;
    CPPUNIT_ASSERT(
        xErrorBar->getPropertyValue("ErrorBarStyle") >>= nErrorBarStyle);
    CPPUNIT_ASSERT_EQUAL(nErrorBarStyle, chart::ErrorBarStyle::RELATIVE);
    bool bShowPositive = bool(), bShowNegative = bool();
    CPPUNIT_ASSERT(
        xErrorBar->getPropertyValue("ShowPositiveError") >>= bShowPositive);
    CPPUNIT_ASSERT(bShowPositive);
    CPPUNIT_ASSERT(
        xErrorBar->getPropertyValue("ShowNegativeError") >>= bShowNegative);
    CPPUNIT_ASSERT(bShowNegative);
    double nVal = 0.0;
    CPPUNIT_ASSERT(xErrorBar->getPropertyValue("PositiveError") >>= nVal);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(nVal, 10.0, 1e-10);
}

void checkCommonTrendline(
        Reference<chart2::XRegressionCurve> xCurve,
        double aExpectedExtrapolateForward, double aExpectedExtrapolateBackward,
        sal_Bool aExpectedForceIntercept, double aExpectedInterceptValue,
        sal_Bool aExpectedShowEquation, sal_Bool aExpectedR2)
{
    Reference<XPropertySet> xProperties( xCurve , uno::UNO_QUERY );
    CPPUNIT_ASSERT(xProperties.is());

    double aExtrapolateForward = 0.0;
    CPPUNIT_ASSERT(xProperties->getPropertyValue("ExtrapolateForward") >>= aExtrapolateForward);
    CPPUNIT_ASSERT_EQUAL(aExpectedExtrapolateForward, aExtrapolateForward);

    double aExtrapolateBackward = 0.0;
    CPPUNIT_ASSERT(xProperties->getPropertyValue("ExtrapolateBackward") >>= aExtrapolateBackward);
    CPPUNIT_ASSERT_EQUAL(aExpectedExtrapolateBackward, aExtrapolateBackward);

    sal_Bool aForceIntercept = false;
    CPPUNIT_ASSERT(xProperties->getPropertyValue("ForceIntercept") >>= aForceIntercept);
    CPPUNIT_ASSERT_EQUAL(aExpectedForceIntercept, aForceIntercept);

    if (aForceIntercept)
    {
        double aInterceptValue = 0.0;
        CPPUNIT_ASSERT(xProperties->getPropertyValue("InterceptValue") >>= aInterceptValue);
        CPPUNIT_ASSERT_EQUAL(aExpectedInterceptValue, aInterceptValue);
    }

    Reference< XPropertySet > xEquationProperties( xCurve->getEquationProperties() );
    CPPUNIT_ASSERT(xEquationProperties.is());

    sal_Bool aShowEquation = false;
    CPPUNIT_ASSERT(xEquationProperties->getPropertyValue("ShowEquation") >>= aShowEquation);
    CPPUNIT_ASSERT_EQUAL(aExpectedShowEquation, aShowEquation);

    sal_Bool aShowCorrelationCoefficient = false;
    CPPUNIT_ASSERT(xEquationProperties->getPropertyValue("ShowCorrelationCoefficient") >>= aShowCorrelationCoefficient);
    CPPUNIT_ASSERT_EQUAL(aExpectedR2, aShowCorrelationCoefficient);
}

void checkNameAndType(Reference<XPropertySet> xProperties, OUString aExpectedName, OUString aExpectedServiceName)
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
        Reference<chart2::XRegressionCurve> xCurve, OUString aExpectedName,
        double aExpectedExtrapolateForward, double aExpectedExtrapolateBackward,
        sal_Bool aExpectedForceIntercept, double aExpectedInterceptValue,
        sal_Bool aExpectedShowEquation, sal_Bool aExpectedR2)
{
    Reference<XPropertySet> xProperties( xCurve , uno::UNO_QUERY );
    CPPUNIT_ASSERT(xProperties.is());

    checkNameAndType(xProperties, aExpectedName, "com.sun.star.chart2.LinearRegressionCurve");

    checkCommonTrendline(
        xCurve,
        aExpectedExtrapolateForward, aExpectedExtrapolateBackward,
        aExpectedForceIntercept, aExpectedInterceptValue,
        aExpectedShowEquation, aExpectedR2);
}

void checkPolynomialTrendline(
        Reference<chart2::XRegressionCurve> xCurve, OUString aExpectedName,
        sal_Int32 aExpectedDegree,
        double aExpectedExtrapolateForward, double aExpectedExtrapolateBackward,
        sal_Bool aExpectedForceIntercept, double aExpectedInterceptValue,
        sal_Bool aExpectedShowEquation, sal_Bool aExpectedR2)
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
        aExpectedForceIntercept, aExpectedInterceptValue,
        aExpectedShowEquation, aExpectedR2);
}

void checkMovingAverageTrendline(
        Reference<chart2::XRegressionCurve> xCurve, OUString aExpectedName, sal_Int32 aExpectedPeriod)
{
    Reference<XPropertySet> xProperties( xCurve , uno::UNO_QUERY );
    CPPUNIT_ASSERT(xProperties.is());

    checkNameAndType(xProperties, aExpectedName, "com.sun.star.chart2.MovingAverageRegressionCurve");

    sal_Int32 aPeriod = 2;
    CPPUNIT_ASSERT(xProperties->getPropertyValue("MovingAveragePeriod") >>= aPeriod);
    CPPUNIT_ASSERT_EQUAL(aExpectedPeriod, aPeriod);
}

void checkTrendlinesInChart(uno::Reference< chart2::XChartDocument > xChartDoc)
{
    CPPUNIT_ASSERT(xChartDoc.is());

    Reference< chart2::XDataSeries > xDataSeries = getDataSeriesFromDoc( xChartDoc, 0 );
    CPPUNIT_ASSERT( xDataSeries.is() );

    Reference< chart2::XRegressionCurveContainer > xRegressionCurveContainer( xDataSeries, UNO_QUERY );
    CPPUNIT_ASSERT( xRegressionCurveContainer.is() );

    Sequence< Reference< chart2::XRegressionCurve > > xRegressionCurveSequence = xRegressionCurveContainer->getRegressionCurves();
    CPPUNIT_ASSERT_EQUAL((sal_Int32) 3, xRegressionCurveSequence.getLength());

    Reference<chart2::XRegressionCurve> xCurve;

    xCurve = xRegressionCurveSequence[0];
    CPPUNIT_ASSERT(xCurve.is());
    checkPolynomialTrendline(xCurve, "col2_poly", 3, 0.1, -0.1, true, -1.0, true, true);

    xCurve = xRegressionCurveSequence[1];
    CPPUNIT_ASSERT(xCurve.is());
    checkLinearTrendline(xCurve, "col2_linear", -0.5, -0.5, false, 0.0, true, false);

    xCurve = xRegressionCurveSequence[2];
    CPPUNIT_ASSERT(xCurve.is());
    checkMovingAverageTrendline(xCurve, "col2_moving_avg", 3);
}

}

// improve the test
void Chart2ExportTest::testErrorBarXLSX()
{
    load("/chart2/qa/extras/data/ods/", "error_bar.ods");
    {
        // make sure the ODS import was successful
        uno::Reference< chart2::XChartDocument > xChartDoc = getChartDocFromSheet( 0, mxComponent );
        CPPUNIT_ASSERT(xChartDoc.is());

        Reference< chart2::XDataSeries > xDataSeries = getDataSeriesFromDoc( xChartDoc, 0 );
        CPPUNIT_ASSERT( xDataSeries.is() );

        Reference< beans::XPropertySet > xPropSet( xDataSeries, UNO_QUERY_THROW );
        CPPUNIT_ASSERT( xPropSet.is() );

        // test that y error bars are there
        Reference< beans::XPropertySet > xErrorBarYProps;
        xPropSet->getPropertyValue("ErrorBarY") >>= xErrorBarYProps;
        testErrorBar(xErrorBarYProps);
    }

    reload("Calc Office Open XML");
    {
        uno::Reference< chart2::XChartDocument > xChartDoc = getChartDocFromSheet( 0, mxComponent );
        CPPUNIT_ASSERT(xChartDoc.is());

        Reference< chart2::XDataSeries > xDataSeries = getDataSeriesFromDoc( xChartDoc, 0 );
        CPPUNIT_ASSERT( xDataSeries.is() );

        Reference< beans::XPropertySet > xPropSet( xDataSeries, UNO_QUERY_THROW );
        CPPUNIT_ASSERT( xPropSet.is() );

        // test that y error bars are there
        Reference< beans::XPropertySet > xErrorBarYProps;
        xPropSet->getPropertyValue("ErrorBarY") >>= xErrorBarYProps;
        testErrorBar(xErrorBarYProps);
    }
}

// This method tests the preservation of properties for trendlines / regression curves
// in an export -> import cycle using different file formats - ODS, XLS and XLSX.
void Chart2ExportTest::testTrendline()
{
    load("/chart2/qa/extras/data/ods/", "trendline.ods");
    checkTrendlinesInChart(getChartDocFromSheet( 0, mxComponent));
    reload("calc8");
    checkTrendlinesInChart(getChartDocFromSheet( 0, mxComponent));

    load("/chart2/qa/extras/data/ods/", "trendline.ods");
    checkTrendlinesInChart(getChartDocFromSheet( 0, mxComponent));
    reload("Calc Office Open XML");
    checkTrendlinesInChart(getChartDocFromSheet( 0, mxComponent));

    load("/chart2/qa/extras/data/ods/", "trendline.ods");
    checkTrendlinesInChart(getChartDocFromSheet( 0, mxComponent));
    reload("MS Excel 97");
    checkTrendlinesInChart(getChartDocFromSheet( 0, mxComponent));

}

#if 0  // disable until gerrit 6957 is merged in some form */
void Chart2ExportTest::testStockChart()
{
    /*  For attached file Stock_Chart.docx, in chart1.xml,
     * <c:stockChart>, there are four types of series as
     * Open,Low,High and Close.
     * For Open series, in <c:idx val="0" />
     * an attribute val of index should start from 1 and not from 0.
     * Which was problem area.
     */
    load("/chart2/qa/extras/data/docx/", "testStockChart.docx");
    {
        xmlDocPtr pXmlDoc = parseExport("word/charts/chart1.xml");
        if (!pXmlDoc)
           return;
      assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:stockChart/c:ser[1]/c:idx", "val", "1");
      assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:stockChart/c:ser[1]/c:order", "val", "1");
      assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:stockChart/c:ser[1]/c:tx/c:strRef/c:strCache/c:pt/c:v", "Open");
    }
}
#endif
CPPUNIT_TEST_SUITE_REGISTRATION(Chart2ExportTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
