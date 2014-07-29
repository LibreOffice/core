/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "charttest.hxx"
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/chart2/CurveStyle.hpp>
#include <com/sun/star/chart2/DataPointLabel.hpp>
#include <com/sun/star/chart/ErrorBarStyle.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart/XChartDocument.hpp>
#include <com/sun/star/chart/XChartData.hpp>
#include <com/sun/star/chart2/XInternalDataProvider.hpp>
#include <com/sun/star/chart/XChartDataArray.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>

#include <com/sun/star/util/Color.hpp>

class Chart2ImportTest : public ChartTest
{
public:
    void Fdo60083();
    void testSteppedLines();
    void testErrorBarRange();
    void testErrorBarFormatting();
    void testODSChartSeries();
    void testXLSXChartSeries();
    void testXLSChartSeries();
    void testODTChartSeries();
    void testDOCChartSeries();
    void testDOCXChartSeries();
    void testPPTXChartSeries();
    /**
     * Original data contains 3 series but 2 of them are hidden. For now, we
     * detect and skip those hidden series on import (since we don't support
     * hidden columns for internal data table yet).
     */
    void testPPTXHiddenDataSeries();
    void testPPTXPercentageNumberFormats();
    void testPPTXStackedNonStackedYAxis();
    void testPPTChartSeries();
    void testODPChartSeries();
    void testBnc864396();
    void testBnc882383();
    void testSimpleStrictXLSX();
    void testDelayedCellImport(); // chart range referencing content on later sheets
    void testFlatODSStackedColumnChart();
    void testFdo78080();
    void testFdo54361();

    CPPUNIT_TEST_SUITE(Chart2ImportTest);
    CPPUNIT_TEST(Fdo60083);
    CPPUNIT_TEST(testSteppedLines);
    CPPUNIT_TEST(testErrorBarRange);
    CPPUNIT_TEST(testErrorBarFormatting);
    CPPUNIT_TEST(testODSChartSeries);
    CPPUNIT_TEST(testXLSXChartSeries);
    CPPUNIT_TEST(testXLSChartSeries);
    CPPUNIT_TEST(testODTChartSeries);
    CPPUNIT_TEST(testDOCChartSeries);
    CPPUNIT_TEST(testDOCXChartSeries);
/*
 *  Disabling Impress Uts.
 *  ChartTest::tearDown() calls dispose of mxComponent
 *  this causes the UT to crash in sd.
 *  sd::DrawView::Notify tries to reset by calling sd::DrawViewShell::ResetActualPage
 */
    CPPUNIT_TEST(testPPTChartSeries);
    CPPUNIT_TEST(testPPTXChartSeries);
    CPPUNIT_TEST(testPPTXHiddenDataSeries);
    CPPUNIT_TEST(testPPTXPercentageNumberFormats);
    CPPUNIT_TEST(testPPTXStackedNonStackedYAxis);
    CPPUNIT_TEST(testODPChartSeries);
    CPPUNIT_TEST(testBnc864396);
    CPPUNIT_TEST(testBnc882383);
    CPPUNIT_TEST(testSimpleStrictXLSX);
    CPPUNIT_TEST(testDelayedCellImport);
    CPPUNIT_TEST(testFlatODSStackedColumnChart);
    CPPUNIT_TEST(testFdo78080);
    CPPUNIT_TEST(testFdo54361);
    CPPUNIT_TEST_SUITE_END();

private:

};

// error bar import
// split method up into smaller chunks for more detailed tests
void Chart2ImportTest::Fdo60083()
{
    load("/chart2/qa/extras/data/ods/", "fdo60083.ods");
    uno::Reference< chart2::XChartDocument > xChartDoc = getChartDocFromSheet( 0, mxComponent );
    CPPUNIT_ASSERT(xChartDoc.is());

    Reference< chart2::XDataSeries > xDataSeries = getDataSeriesFromDoc( xChartDoc, 0 );
    CPPUNIT_ASSERT( xDataSeries.is() );

    Reference< beans::XPropertySet > xPropSet( xDataSeries, UNO_QUERY_THROW );
    CPPUNIT_ASSERT( xPropSet.is() );

    // test that y error bars are there
    Reference< beans::XPropertySet > xErrorBarYProps;
    xPropSet->getPropertyValue(CHART_UNONAME_ERRORBAR_Y) >>= xErrorBarYProps;
    CPPUNIT_ASSERT(xErrorBarYProps.is());
    {
        sal_Int32 nErrorBarStyle;
        CPPUNIT_ASSERT(
            xErrorBarYProps->getPropertyValue("ErrorBarStyle")
            >>= nErrorBarStyle);
        CPPUNIT_ASSERT_EQUAL(
            static_cast<sal_Int32>(chart::ErrorBarStyle::RELATIVE),
            nErrorBarStyle);

        double nVal = 0.0;
        CPPUNIT_ASSERT(
            xErrorBarYProps->getPropertyValue("PositiveError") >>= nVal);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0, nVal, 1e-8);

        CPPUNIT_ASSERT(
            xErrorBarYProps->getPropertyValue("NegativeError") >>= nVal);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0, nVal, 1e-8);

        bool bVal;
        CPPUNIT_ASSERT(
            xErrorBarYProps->getPropertyValue("ShowPositiveError") >>= bVal);
        CPPUNIT_ASSERT_EQUAL(bVal, true);

        CPPUNIT_ASSERT(
            xErrorBarYProps->getPropertyValue("ShowNegativeError") >>= bVal);
        CPPUNIT_ASSERT_EQUAL(bVal, true);
    }

    // test that x error bars are not imported
    Reference< beans::XPropertySet > xErrorBarXProps;
    xPropSet->getPropertyValue(CHART_UNONAME_ERRORBAR_X) >>= xErrorBarXProps;
    CPPUNIT_ASSERT(!xErrorBarXProps.is());
}

void Chart2ImportTest::testErrorBarRange()
{
    load("/chart2/qa/extras/data/ods/", "error_bar_range.ods");
    uno::Reference< chart2::XChartDocument > xChartDoc = getChartDocFromSheet( 0, mxComponent );
    CPPUNIT_ASSERT(xChartDoc.is());

    Reference< chart2::XDataSeries > xDataSeries = getDataSeriesFromDoc( xChartDoc, 0 );
    CPPUNIT_ASSERT( xDataSeries.is() );

    Reference< beans::XPropertySet > xPropSet( xDataSeries, UNO_QUERY_THROW );
    CPPUNIT_ASSERT( xPropSet.is() );

    // test that y error bars are there
    Reference< beans::XPropertySet > xErrorBarYProps;
    xPropSet->getPropertyValue(CHART_UNONAME_ERRORBAR_Y) >>= xErrorBarYProps;
    CPPUNIT_ASSERT(xErrorBarYProps.is());

    sal_Int32 nErrorBarStyle;
    CPPUNIT_ASSERT(
            xErrorBarYProps->getPropertyValue("ErrorBarStyle")
            >>= nErrorBarStyle);
    CPPUNIT_ASSERT_EQUAL(
            static_cast<sal_Int32>(chart::ErrorBarStyle::FROM_DATA),
            nErrorBarStyle);

    OUString aRangePos;
    CPPUNIT_ASSERT(xErrorBarYProps->getPropertyValue("ErrorBarRangePositive") >>= aRangePos);
    CPPUNIT_ASSERT_EQUAL(aRangePos, OUString("$Sheet1.$C$2:$C$4"));
}

void Chart2ImportTest::testErrorBarFormatting()
{
    load("/chart2/qa/extras/data/ods/", "error_bar_properties.ods");
    uno::Reference< chart2::XChartDocument > xChartDoc = getChartDocFromSheet( 0, mxComponent );
    CPPUNIT_ASSERT(xChartDoc.is());

    Reference< chart2::XDataSeries > xDataSeries = getDataSeriesFromDoc( xChartDoc, 0 );
    CPPUNIT_ASSERT( xDataSeries.is() );

    Reference< beans::XPropertySet > xPropSet( xDataSeries, UNO_QUERY_THROW );
    CPPUNIT_ASSERT( xPropSet.is() );

    // test that y error bars are there
    Reference< beans::XPropertySet > xErrorBarYProps;
    xPropSet->getPropertyValue(CHART_UNONAME_ERRORBAR_Y) >>= xErrorBarYProps;
    CPPUNIT_ASSERT(xErrorBarYProps.is());

    util::Color aColor(0);
    xErrorBarYProps->getPropertyValue("LineColor") >>= aColor;
    sal_uInt32 nColorValue = aColor;
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(0xff3333), nColorValue);
}

// stepped line interpolation
void Chart2ImportTest::testSteppedLines()
{
    const sal_Int32 MAXSHEET = 14;
    chart2::CurveStyle curveStyle[] = {
        chart2::CurveStyle_LINES,
        chart2::CurveStyle_CUBIC_SPLINES,
        chart2::CurveStyle_B_SPLINES,
        chart2::CurveStyle_STEP_START,
        chart2::CurveStyle_STEP_END,
        chart2::CurveStyle_STEP_CENTER_X,
        chart2::CurveStyle_STEP_CENTER_Y,
        chart2::CurveStyle_LINES,
        chart2::CurveStyle_CUBIC_SPLINES,
        chart2::CurveStyle_B_SPLINES,
        chart2::CurveStyle_STEP_START,
        chart2::CurveStyle_STEP_END,
        chart2::CurveStyle_STEP_CENTER_X,
        chart2::CurveStyle_STEP_CENTER_Y
    };

    load("/chart2/qa/extras/data/ods/", "stepped_lines.ods");
    for(sal_Int32 nSheet = 0; nSheet < MAXSHEET; ++nSheet)
    {
        uno::Reference< chart2::XChartDocument > xChart2Doc = getChartDocFromSheet( nSheet, mxComponent );
        CPPUNIT_ASSERT(xChart2Doc.is());

        Reference< chart2::XChartType > xChartType = getChartTypeFromDoc( xChart2Doc, 0 );
        CPPUNIT_ASSERT(xChartType.is());

        Reference< beans::XPropertySet > xPropSet( xChartType, UNO_QUERY );
        CPPUNIT_ASSERT(xPropSet.is());

        chart2::CurveStyle eCurveStyle;
        xPropSet->getPropertyValue("CurveStyle") >>= eCurveStyle;
        CPPUNIT_ASSERT_EQUAL(eCurveStyle, curveStyle[nSheet]);
    }
}

uno::Sequence < OUString > getChartColumnDescriptions( uno::Reference< chart::XChartDocument > xChart1Doc)
{
    CPPUNIT_ASSERT(xChart1Doc.is());
    uno::Reference< chart::XChartDataArray > xChartData ( xChart1Doc->getData(), UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xChartData.is());
    uno::Sequence < OUString > seriesList = xChartData->getColumnDescriptions();
    return seriesList;
}

void Chart2ImportTest::testODSChartSeries()
{
    load("/chart2/qa/extras/data/ods/", "chart.ods");
    uno::Reference< chart::XChartDocument > xChart1Doc ( getChartCompFromSheet( 0, mxComponent ), UNO_QUERY_THROW);
    uno::Sequence < OUString > seriesList = getChartColumnDescriptions( xChart1Doc);
    CPPUNIT_ASSERT_EQUAL(OUString("Col 1"), seriesList[0]);
    CPPUNIT_ASSERT_EQUAL(OUString("Col2"), seriesList[1]);
    CPPUNIT_ASSERT_EQUAL(OUString("Col 33"), seriesList[2]);

}

void Chart2ImportTest::testXLSXChartSeries()
{
    load("/chart2/qa/extras/data/xlsx/", "chart.xlsx");
    uno::Reference< chart::XChartDocument > xChart1Doc ( getChartCompFromSheet( 0, mxComponent ), UNO_QUERY_THROW);
    uno::Sequence < OUString > seriesList = getChartColumnDescriptions(xChart1Doc );
    CPPUNIT_ASSERT_EQUAL(OUString("Col 1"), seriesList[0]);
    CPPUNIT_ASSERT_EQUAL(OUString("Col2"), seriesList[1]);
    CPPUNIT_ASSERT_EQUAL(OUString("Col 33"), seriesList[2]);

}

void Chart2ImportTest::testXLSChartSeries()
{
    load("/chart2/qa/extras/data/xls/", "chart.xls");
    uno::Reference< chart::XChartDocument > xChart1Doc ( getChartCompFromSheet( 0, mxComponent ), UNO_QUERY_THROW);
    uno::Sequence < OUString > seriesList = getChartColumnDescriptions(xChart1Doc );
    CPPUNIT_ASSERT_EQUAL(OUString("Col 1"), seriesList[0]);
    CPPUNIT_ASSERT_EQUAL(OUString("Col 2"), seriesList[1]);
    CPPUNIT_ASSERT_EQUAL(OUString("Col 3"), seriesList[2]);

}

void Chart2ImportTest::testODTChartSeries()
{
    load("/chart2/qa/extras/data/odt/", "chart.odt");
    uno::Sequence< OUString > seriesList = getWriterChartColumnDescriptions(mxComponent);
    CPPUNIT_ASSERT_EQUAL(OUString("Column 1"), seriesList[0]);
    CPPUNIT_ASSERT_EQUAL(OUString("Column 2"), seriesList[1]);
    CPPUNIT_ASSERT_EQUAL(OUString("Column 3"), seriesList[2]);

}

void Chart2ImportTest::testDOCChartSeries()
{
    load("/chart2/qa/extras/data/doc/", "chart.doc");
    uno::Sequence< OUString > seriesList = getWriterChartColumnDescriptions(mxComponent);
    CPPUNIT_ASSERT_EQUAL(OUString("Column 1"), seriesList[0]);
    CPPUNIT_ASSERT_EQUAL(OUString("Column 2"), seriesList[1]);
    CPPUNIT_ASSERT_EQUAL(OUString("Column 3"), seriesList[2]);
}

void Chart2ImportTest::testDOCXChartSeries()
{
    load("/chart2/qa/extras/data/docx/", "chart.docx");
    Reference<chart2::XChartDocument> xChartDoc(getChartDocFromWriter(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());

    Reference<chart2::XChartType> xCT = getChartTypeFromDoc(xChartDoc, 0, 0);
    CPPUNIT_ASSERT(xCT.is());

    std::vector<uno::Sequence<uno::Any> > aLabels = getDataSeriesLabelsFromChartType(xCT);
    CPPUNIT_ASSERT_EQUAL(size_t(3), aLabels.size());
    CPPUNIT_ASSERT_EQUAL(OUString("Series 1"), aLabels[0][0].get<OUString>());
    CPPUNIT_ASSERT_EQUAL(OUString("Series 2"), aLabels[1][0].get<OUString>());
    CPPUNIT_ASSERT_EQUAL(OUString("Series 3"), aLabels[2][0].get<OUString>());
}

void Chart2ImportTest::testPPTChartSeries()
{
    //test chart series names for ppt
    uno::Sequence < OUString > seriesList = getImpressChartColumnDescriptions("/chart2/qa/extras/data/ppt/", "chart.ppt");

    CPPUNIT_ASSERT_EQUAL(OUString("Column 1"), seriesList[0]);
    CPPUNIT_ASSERT_EQUAL(OUString("Column 2"), seriesList[1]);
    CPPUNIT_ASSERT_EQUAL(OUString("Column 3"), seriesList[2]);

}

void Chart2ImportTest::testPPTXChartSeries()
{
    //test chart series names for pptx
    load("/chart2/qa/extras/data/pptx/", "chart.pptx");
    Reference<chart2::XChartDocument> xChartDoc(getChartDocFromDrawImpress(0, 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());

    Reference<chart2::XChartType> xCT = getChartTypeFromDoc(xChartDoc, 0, 0);
    CPPUNIT_ASSERT(xCT.is());

    std::vector<uno::Sequence<uno::Any> > aLabels = getDataSeriesLabelsFromChartType(xCT);
    CPPUNIT_ASSERT_EQUAL(size_t(3), aLabels.size());
    CPPUNIT_ASSERT_EQUAL(OUString("Column 1"), aLabels[0][0].get<OUString>());
    CPPUNIT_ASSERT_EQUAL(OUString("Column 2"), aLabels[1][0].get<OUString>());
    CPPUNIT_ASSERT_EQUAL(OUString("Column 3"), aLabels[2][0].get<OUString>());
}

void Chart2ImportTest::testPPTXHiddenDataSeries()
{
    load("/chart2/qa/extras/data/pptx/", "stacked-bar-chart-hidden-series.pptx");
    Reference<chart2::XChartDocument> xChartDoc(getChartDocFromDrawImpress(0, 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());

    Reference<chart2::XChartType> xCT = getChartTypeFromDoc(xChartDoc, 0, 0);
    CPPUNIT_ASSERT(xCT.is());

    // There should be only one data series present.
    std::vector<uno::Sequence<uno::Any> > aLabels = getDataSeriesLabelsFromChartType(xCT);
    CPPUNIT_ASSERT_EQUAL(size_t(1), aLabels.size());
    CPPUNIT_ASSERT_EQUAL(OUString("Series 3"), aLabels[0][0].get<OUString>());

    // Test the internal data.
    CPPUNIT_ASSERT(xChartDoc->hasInternalDataProvider());

    Reference<chart2::XInternalDataProvider> xInternalProvider(xChartDoc->getDataProvider(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xInternalProvider.is());

    Reference<chart::XComplexDescriptionAccess> xDescAccess(xInternalProvider, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDescAccess.is());

    // Get the category labels.
    Sequence<Sequence<OUString> > aCategories = xDescAccess->getComplexRowDescriptions();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), aCategories.getLength());
    CPPUNIT_ASSERT_EQUAL(OUString("Category 1"), aCategories[0][0]);
    CPPUNIT_ASSERT_EQUAL(OUString("Category 2"), aCategories[1][0]);
    CPPUNIT_ASSERT_EQUAL(OUString("Category 3"), aCategories[2][0]);
    CPPUNIT_ASSERT_EQUAL(OUString("Category 4"), aCategories[3][0]);
}

void Chart2ImportTest::testPPTXPercentageNumberFormats()
{
    load("/chart2/qa/extras/data/pptx/", "percentage-number-formats.pptx");

    // 1st chart
    Reference<chart2::XChartDocument> xChartDoc(getChartDocFromDrawImpress(0, 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());

    uno::Reference<chart2::XDataSeries> xDataSeries(getDataSeriesFromDoc(xChartDoc, 0));
    CPPUNIT_ASSERT(xDataSeries.is());
    uno::Reference<beans::XPropertySet> xPropertySet;
    chart2::DataPointLabel aLabel;
    sal_Int32 nNumberFormat;
    const sal_Int32 nPercentFormatSimple = getNumberFormat(xChartDoc, "0%");
    const sal_Int32 nPercentFormatDecimal = getNumberFormat(xChartDoc, "0.00%");

    xPropertySet.set(xDataSeries->getDataPointByIndex(0), uno::UNO_QUERY_THROW);
    xPropertySet->getPropertyValue("Label") >>= aLabel;
    CPPUNIT_ASSERT_EQUAL(sal_True, aLabel.ShowNumber);
    CPPUNIT_ASSERT_EQUAL(sal_True, aLabel.ShowNumberInPercent);
    xPropertySet->getPropertyValue("PercentageNumberFormat") >>= nNumberFormat;
    CPPUNIT_ASSERT_EQUAL(nPercentFormatSimple, nNumberFormat);

    xPropertySet.set(xDataSeries->getDataPointByIndex(1), uno::UNO_QUERY_THROW);
    xPropertySet->getPropertyValue("Label") >>= aLabel;
    CPPUNIT_ASSERT_EQUAL(sal_True, aLabel.ShowNumber);
    CPPUNIT_ASSERT_EQUAL(sal_True, aLabel.ShowNumberInPercent);
    xPropertySet->getPropertyValue("PercentageNumberFormat") >>= nNumberFormat;
    CPPUNIT_ASSERT_EQUAL(nPercentFormatDecimal, nNumberFormat);

    xPropertySet.set(xDataSeries->getDataPointByIndex(2), uno::UNO_QUERY_THROW);
    xPropertySet->getPropertyValue("Label") >>= aLabel;
    CPPUNIT_ASSERT_EQUAL(sal_False, aLabel.ShowNumber);
    CPPUNIT_ASSERT_EQUAL(sal_True, aLabel.ShowNumberInPercent);
    xPropertySet->getPropertyValue("PercentageNumberFormat") >>= nNumberFormat;
    CPPUNIT_ASSERT_EQUAL(nPercentFormatSimple, nNumberFormat);

    xPropertySet.set(xDataSeries->getDataPointByIndex(3), uno::UNO_QUERY_THROW);
    xPropertySet->getPropertyValue("Label") >>= aLabel;
    CPPUNIT_ASSERT_EQUAL(sal_False, aLabel.ShowNumber);
    CPPUNIT_ASSERT_EQUAL(sal_True, aLabel.ShowNumberInPercent);
    xPropertySet->getPropertyValue("PercentageNumberFormat") >>= nNumberFormat;
    CPPUNIT_ASSERT_EQUAL(nPercentFormatDecimal, nNumberFormat);

    // 2nd chart
    xChartDoc.set(getChartDocFromDrawImpress(1, 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());

    Reference<chart2::XAxis> xYAxis = getAxisFromDoc(xChartDoc, 0, 1, 0);
    CPPUNIT_ASSERT(xYAxis.is());

    Reference<beans::XPropertySet> xPS(xYAxis, uno::UNO_QUERY_THROW);
    bool bLinkNumberFormatToSource = true;
    bool bSuccess = xPS->getPropertyValue(CHART_UNONAME_LINK_TO_SRC_NUMFMT) >>= bLinkNumberFormatToSource;
    CPPUNIT_ASSERT_MESSAGE("\"LinkNumberFormatToSource\" should be set to false.", bSuccess && !bLinkNumberFormatToSource);

    // FIXME: This should be in fact "0.00%".
    // see TODO in oox/source/drawingml/chart/modelbase.cxx
    const sal_Int32 nPercentFormatDecimalShort = getNumberFormat(xChartDoc, "0.0%");
    nNumberFormat = getNumberFormatFromAxis(xYAxis);
    CPPUNIT_ASSERT_EQUAL(nPercentFormatDecimalShort, nNumberFormat);
    sal_Int16 nType = getNumberFormatType(xChartDoc, nNumberFormat);
    CPPUNIT_ASSERT_MESSAGE("Y axis should be a percent format.", (nType & util::NumberFormat::PERCENT));
}

void Chart2ImportTest::testPPTXStackedNonStackedYAxis()
{
    load("/chart2/qa/extras/data/pptx/", "stacked-non-stacked-mix-y-axis.pptx");

    // 1st chart is a normal stacked column.
    Reference<chart2::XChartDocument> xChartDoc(getChartDocFromDrawImpress(0, 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());

    Reference<chart2::XTitled> xTitled(xChartDoc, uno::UNO_QUERY_THROW);
    OUString aTitle = getTitleString(xTitled);
    CPPUNIT_ASSERT_EQUAL(OUString("Stacked"), aTitle);

    // Get the Y-axis.
    Reference<chart2::XAxis> xYAxis = getAxisFromDoc(xChartDoc, 0, 1, 0);
    CPPUNIT_ASSERT(xYAxis.is());

    sal_Int32 nNumberFormat = getNumberFormatFromAxis(xYAxis);
    sal_Int16 nType = getNumberFormatType(xChartDoc, nNumberFormat);
    CPPUNIT_ASSERT_MESSAGE("Y axis should be a normal number format.", (nType & util::NumberFormat::NUMBER));
    CPPUNIT_ASSERT_MESSAGE("Y axis should NOT be a percent format.", !(nType & util::NumberFormat::PERCENT));

    // 2nd chart is a percent-stacked column.
    xChartDoc.set(getChartDocFromDrawImpress(1, 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());

    xTitled.set(xChartDoc, uno::UNO_QUERY_THROW);
    aTitle = getTitleString(xTitled);
    CPPUNIT_ASSERT_EQUAL(OUString("100% Stacked"), aTitle);

    // Get the Y-axis.
    xYAxis = getAxisFromDoc(xChartDoc, 0, 1, 0);
    CPPUNIT_ASSERT(xYAxis.is());

    // Get the number format of the Y-axis.
    nNumberFormat = getNumberFormatFromAxis(xYAxis);
    nType = getNumberFormatType(xChartDoc, nNumberFormat);
    CPPUNIT_ASSERT_MESSAGE("Y axis should be a percent format.", (nType & util::NumberFormat::PERCENT));

    // 3rd chart is a mixture of normal-stacked column with a percent-stacked
    // area chart series.  Excel in this case sets the Y-axis to be
    // non-percent axis and we should do the same for interoperability.
    xChartDoc.set(getChartDocFromDrawImpress(2, 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());

    xTitled.set(xChartDoc, uno::UNO_QUERY_THROW);
    aTitle = getTitleString(xTitled);
    CPPUNIT_ASSERT_EQUAL(OUString("Stacked column mixed with 100% stacked area"), aTitle);

    // Get the Y-axis.
    xYAxis = getAxisFromDoc(xChartDoc, 0, 1, 0);
    CPPUNIT_ASSERT(xYAxis.is());

    // Get the number format of the Y-axis.
    nNumberFormat = getNumberFormatFromAxis(xYAxis);
    nType = getNumberFormatType(xChartDoc, nNumberFormat);
    CPPUNIT_ASSERT_MESSAGE("Y axis should be a normal number format.", (nType & util::NumberFormat::NUMBER));
    CPPUNIT_ASSERT_MESSAGE("Y axis should NOT be a percent format.", !(nType & util::NumberFormat::PERCENT));
}

void Chart2ImportTest::testODPChartSeries()
{
    //test chart series names for odp
    uno::Sequence < OUString > seriesList = getImpressChartColumnDescriptions("/chart2/qa/extras/data/odp/", "chart.odp");
    CPPUNIT_ASSERT_EQUAL(OUString("Column 1"), seriesList[0]);
    CPPUNIT_ASSERT_EQUAL(OUString("Column 2"), seriesList[1]);
    CPPUNIT_ASSERT_EQUAL(OUString("Column 3"), seriesList[2]);

}

void Chart2ImportTest::testBnc864396()
{
    uno::Reference< chart2::XChartDocument > xChartDoc(getChartDocFromImpress("/chart2/qa/extras/data/pptx/", "bnc864396.pptx"), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xChartDoc->hasInternalDataProvider());

    uno::Reference< chart2::XInternalDataProvider > xDataProvider( xChartDoc->getDataProvider(), uno::UNO_QUERY_THROW );
    uno::Reference< chart::XChartDataArray > xChartDataArray(xDataProvider, uno::UNO_QUERY_THROW);
    uno::Sequence< OUString > aRowLabels = xChartDataArray->getRowDescriptions();
    for(sal_Int32 i = 0; i < aRowLabels.getLength(); ++i)
    {
        OUString aExpected = OUString("cat") + OUString::number(i+1);
        CPPUNIT_ASSERT_EQUAL(aExpected, aRowLabels[i]);
    }
}

void Chart2ImportTest::testBnc882383()
{
    load("/chart2/qa/extras/data/pptx/", "bnc882383.pptx");
    uno::Reference<chart2::XChartDocument> xChartDoc(getChartDocFromDrawImpress(0, 0), uno::UNO_QUERY_THROW);
    uno::Reference<chart2::XDataSeries> xDataSeries(getDataSeriesFromDoc(xChartDoc, 0));
    CPPUNIT_ASSERT(xDataSeries.is());

    uno::Reference<beans::XPropertySet> xPropertySet(xDataSeries->getDataPointByIndex(0), uno::UNO_QUERY_THROW);
    OUString sGradientName;
    xPropertySet->getPropertyValue("GradientName") >>= sGradientName;
    CPPUNIT_ASSERT(!sGradientName.isEmpty());
}

void Chart2ImportTest::testSimpleStrictXLSX()
{
    load("/chart2/qa/extras/data/xlsx/", "strict_chart.xlsx");
    uno::Reference< chart2::XChartDocument > xChartDoc = getChartDocFromSheet( 0, mxComponent );
    CPPUNIT_ASSERT(xChartDoc.is());

    Reference< chart2::XDataSeries > xDataSeries = getDataSeriesFromDoc( xChartDoc, 0 );
    CPPUNIT_ASSERT(xDataSeries.is());

}

void Chart2ImportTest::testDelayedCellImport()
{
    load("/chart2/qa/extras/data/xlsx/", "fdo70609.xlsx");
    uno::Reference< chart2::XChartDocument > xChartDoc = getChartDocFromSheet( 0, mxComponent );
    Reference< chart2::data::XDataSequence > xDataSeq =
        getDataSequenceFromDocByRole(xChartDoc, "values-x");

    OUString aRange = xDataSeq->getSourceRangeRepresentation();
    CPPUNIT_ASSERT_EQUAL(OUString("$Sheet2.$C$5:$C$9"), aRange);
}

void Chart2ImportTest::testFlatODSStackedColumnChart()
{
    load("/chart2/qa/extras/data/fods/", "stacked-column-chart.fods");
    Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT(xChartDoc.is());

    Reference<chart2::XChartType> xChartType = getChartTypeFromDoc(xChartDoc, 0, 0);
    CPPUNIT_ASSERT(xChartType.is());

    Reference<chart2::XDataSeriesContainer> xDSCont(xChartType, UNO_QUERY);
    CPPUNIT_ASSERT(xDSCont.is());
    Sequence<Reference<chart2::XDataSeries> > aSeriesSeq = xDSCont->getDataSeries();

    // The stacked column chart should consist of 5 data series.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), aSeriesSeq.getLength());
}

void Chart2ImportTest::testFdo78080()
{
    load("/chart2/qa/extras/data/xlsx/", "fdo78080.xlsx");
    Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT(xChartDoc.is());

    Reference<chart2::XTitled> xTitled(xChartDoc, uno::UNO_QUERY_THROW);
    Reference<chart2::XTitle> xTitle = xTitled->getTitleObject();
    CPPUNIT_ASSERT(!xTitle.is());
}

void Chart2ImportTest::testFdo54361()
{
    load("/chart2/qa/extras/data/xlsx/", "fdo54361.xlsx");
    uno::Reference< chart2::XChartDocument > xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChartDoc.is());

    uno::Reference< chart::XChartDocument > xChart2Doc (xChartDoc, uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChart2Doc.is());

    Reference< beans::XPropertySet > xPropSet( xChart2Doc->getArea(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("failed to get Area", xPropSet.is());

    com::sun::star::drawing::FillStyle aStyle;
    xPropSet -> getPropertyValue("FillStyle") >>= aStyle;

    CPPUNIT_ASSERT_MESSAGE("Background needs to be with solid fill style", aStyle == 1);
}

CPPUNIT_TEST_SUITE_REGISTRATION(Chart2ImportTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
