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
#include <com/sun/star/chart2/XRegressionCurveContainer.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart/XChartDocument.hpp>
#include <com/sun/star/chart/XChartData.hpp>
#include <com/sun/star/chart2/XInternalDataProvider.hpp>
#include <com/sun/star/chart/XChartDataArray.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/chart/XTwoAxisXSupplier.hpp>
#include <com/sun/star/chart/MissingValueTreatment.hpp>
#include <com/sun/star/chart2/TickmarkStyle.hpp>

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
    void testFdo54361_1();
    void testTdf86624(); // manually placed legends
    void testAutoBackgroundXLSX();
    void testChartAreaStyleBackgroundXLSX();
    void testAxisTextRotationXLSX();
    // void testTextCanOverlapXLSX(); // TODO : temporarily disabled.
    void testNumberFormatsXLSX();

    void testTransparentBackground(OUString const & filename);

    // below are OOXML default value tests for cases
    // where we fixed the handling of MSO 2007 vs OOXML
    void testAutoTitleDelDefaultValue2007XLSX();
    void testAutoTitleDelDefaultValue2013XLSX();
    void testDispBlanksAsDefaultValue2007XLSX();
    void testDispBlanksAsDefaultValue2013XLSX();

    void testSmoothDefaultValue2007XLSX();
    void testSmoothDefaultValue2013XLSX();
    void testTrendlineDefaultValue2007XLSX();
    void testTrendlineDefaultValue2013XLSX();
    void testVaryColorDefaultValues2007XLSX();
    void testVaryColorDefaultValues2013XLSX();
    void testPlotVisOnlyDefaultValue2013XLSX();
    void testRAngAxDefaultValue2013XLSX();
    void testMajorTickMarksDefaultValue2013XLSX();
    void testMinorTickMarksDefaultValue2013XLSX();

    void testAxisTitleDefaultRotationXLSX();
    void testSecondaryAxisTitleDefaultRotationXLSX();
    void testAxisTitleRotationXLSX();

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
    CPPUNIT_TEST(testFdo54361_1);
    CPPUNIT_TEST(testTdf86624);
    CPPUNIT_TEST(testAutoBackgroundXLSX);
    CPPUNIT_TEST(testChartAreaStyleBackgroundXLSX);
    CPPUNIT_TEST(testAxisTextRotationXLSX);
    // CPPUNIT_TEST(testTextCanOverlapXLSX); // TODO : temporarily disabled.
    CPPUNIT_TEST(testNumberFormatsXLSX);
    CPPUNIT_TEST(testAutoTitleDelDefaultValue2007XLSX);
    CPPUNIT_TEST(testAutoTitleDelDefaultValue2013XLSX);
    CPPUNIT_TEST(testDispBlanksAsDefaultValue2007XLSX);
    CPPUNIT_TEST(testDispBlanksAsDefaultValue2013XLSX);
    CPPUNIT_TEST(testSmoothDefaultValue2007XLSX);
    CPPUNIT_TEST(testSmoothDefaultValue2013XLSX);
    CPPUNIT_TEST(testTrendlineDefaultValue2007XLSX);
    CPPUNIT_TEST(testTrendlineDefaultValue2013XLSX);
    CPPUNIT_TEST(testVaryColorDefaultValues2007XLSX);
    CPPUNIT_TEST(testVaryColorDefaultValues2013XLSX);
    CPPUNIT_TEST(testPlotVisOnlyDefaultValue2013XLSX);
    CPPUNIT_TEST(testRAngAxDefaultValue2013XLSX);
    CPPUNIT_TEST(testMajorTickMarksDefaultValue2013XLSX);
    CPPUNIT_TEST(testMinorTickMarksDefaultValue2013XLSX);
    CPPUNIT_TEST(testAxisTitleDefaultRotationXLSX);
    CPPUNIT_TEST(testSecondaryAxisTitleDefaultRotationXLSX);
    CPPUNIT_TEST(testAxisTitleRotationXLSX);
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

    Reference<chart2::XChartType> xCT = getChartTypeFromDoc(xChartDoc, 0);
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

    Reference<chart2::XChartType> xCT = getChartTypeFromDoc(xChartDoc, 0);
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

    // "Automatic" chart background fill in pptx should be loaded as no fill.
    Reference<beans::XPropertySet> xPropSet = xChartDoc->getPageBackground();
    CPPUNIT_ASSERT(xPropSet.is());
    drawing::FillStyle eStyle = xPropSet->getPropertyValue("FillStyle").get<drawing::FillStyle>();
    CPPUNIT_ASSERT_MESSAGE("'Automatic' chart background fill in pptx should be loaded as no fill (transparent).",
        eStyle == drawing::FillStyle_NONE);

    Reference<chart2::XChartType> xCT = getChartTypeFromDoc(xChartDoc, 0);
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
        OUString aExpected = "cat" + OUString::number(i+1);
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

    Reference<chart2::XChartType> xChartType = getChartTypeFromDoc(xChartDoc, 0);
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

void Chart2ImportTest::testTdf86624()
{
    load("/chart2/qa/extras/data/ods/", "tdf86624.ods");
    uno::Reference< chart2::XChartDocument > xChart2Doc = getChartDocFromSheet(0, mxComponent);
    uno::Reference< chart::XChartDocument > xChartDoc (xChart2Doc, uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xLegend = xChartDoc->getLegend();
    awt::Point aPos = xLegend->getPosition();
    CPPUNIT_ASSERT(aPos.X > 5000); // real value for me is above 8000 but before bug fix is below 1000
    CPPUNIT_ASSERT(aPos.Y > 4000); // real value for ms is above 7000
}

void Chart2ImportTest::testTransparentBackground(OUString const & filename)
{
    load("/chart2/qa/extras/data/xlsx/", filename);
    uno::Reference< chart2::XChartDocument > xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChartDoc.is());

    uno::Reference< chart::XChartDocument > xChart2Doc (xChartDoc, uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChart2Doc.is());

    Reference< beans::XPropertySet > xPropSet( xChart2Doc->getArea(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("failed to get Area", xPropSet.is());

    css::drawing::FillStyle aStyle;
    xPropSet -> getPropertyValue("FillStyle") >>= aStyle;

    CPPUNIT_ASSERT_MESSAGE("Background needs to be with solid fill style", aStyle == 1);
}
// 2 test methods here so that tearDown() can dispose the document
void Chart2ImportTest::testFdo54361()
{
    testTransparentBackground("fdo54361.xlsx");
}
void Chart2ImportTest::testFdo54361_1()
{
    testTransparentBackground("fdo54361-1.xlsx");
}

void Chart2ImportTest::testAutoBackgroundXLSX()
{
    load("/chart2/qa/extras/data/xlsx/", "chart-auto-background.xlsx");
    uno::Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChartDoc.is());

    // "Automatic" chart background fill in xlsx should be loaded as solid white.
    Reference<beans::XPropertySet> xPropSet = xChartDoc->getPageBackground();
    CPPUNIT_ASSERT(xPropSet.is());
    drawing::FillStyle eStyle = xPropSet->getPropertyValue("FillStyle").get<drawing::FillStyle>();
    sal_Int32 nColor = xPropSet->getPropertyValue("FillColor").get<sal_Int32>();
    CPPUNIT_ASSERT_MESSAGE("'Automatic' chart background fill in xlsx should be loaded as solid fill.",
        eStyle == drawing::FillStyle_SOLID);
    CPPUNIT_ASSERT_MESSAGE("'Automatic' chart background fill in xlsx should be loaded as solid white.",
        (nColor & 0x00FFFFFF) == 0x00FFFFFF); // highest 2 bytes are transparency which we ignore here.
}

void Chart2ImportTest::testChartAreaStyleBackgroundXLSX()
{
    load("/chart2/qa/extras/data/xlsx/", "chart-area-style-background.xlsx");
    uno::Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChartDoc.is());

    // "Automatic" chart background fill in xlsx should be loaded as solid white.
    Reference<beans::XPropertySet> xPropSet = xChartDoc->getPageBackground();
    CPPUNIT_ASSERT(xPropSet.is());
    drawing::FillStyle eStyle = xPropSet->getPropertyValue("FillStyle").get<drawing::FillStyle>();
    sal_Int32 nColor = xPropSet->getPropertyValue("FillColor").get<sal_Int32>();
    CPPUNIT_ASSERT_MESSAGE("'Automatic' chart background fill in xlsx should be loaded as solid fill.",
        eStyle == drawing::FillStyle_SOLID);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("'Automatic' chart background fill in xlsx should be loaded as solid white.",
        sal_Int32(0), nColor);
}

void Chart2ImportTest::testAxisTextRotationXLSX()
{
    load("/chart2/qa/extras/data/xlsx/", "axis-label-rotation.xlsx");
    uno::Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChartDoc.is());

    Reference<chart2::XAxis> xYAxis = getAxisFromDoc(xChartDoc, 0, 0, 0);
    CPPUNIT_ASSERT(xYAxis.is());

    Reference<beans::XPropertySet> xPS(xYAxis, uno::UNO_QUERY_THROW);
    double nRotation = 0;
    bool bSuccess = xPS->getPropertyValue("TextRotation") >>= nRotation;

    CPPUNIT_ASSERT(bSuccess);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(90, nRotation, 1e-10);
}

/* TODO : temporarily disabled.
void Chart2ImportTest::testTextCanOverlapXLSX()
{
    // fdo#84647 : To check textoverlap value is imported correclty.
    load("/chart2/qa/extras/data/xlsx/", "chart-text-can-overlap.xlsx");
    uno::Reference< chart::XDiagram > mxDiagram;
    uno::Reference< beans::XPropertySet > xAxisProp;
    bool textCanOverlap = false;
    uno::Reference< chart::XChartDocument > xChartDoc ( getChartCompFromSheet( 0, mxComponent ), UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xChartDoc.is());
    mxDiagram.set(xChartDoc->getDiagram());
    CPPUNIT_ASSERT(mxDiagram.is());
    uno::Reference< chart::XAxisXSupplier > xAxisXSupp( mxDiagram, uno::UNO_QUERY );
    CPPUNIT_ASSERT(xAxisXSupp.is());
    xAxisProp = xAxisXSupp->getXAxis();
    xAxisProp->getPropertyValue("TextCanOverlap") >>= textCanOverlap;
    // Expected value of 'TextCanOverlap' is true
    CPPUNIT_ASSERT(textCanOverlap);
}
*/

void Chart2ImportTest::testNumberFormatsXLSX()
{
    load("/chart2/qa/extras/data/xlsx/", "number-formats.xlsx");
    Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChartDoc.is());

    uno::Reference<chart2::XDataSeries> xDataSeries(getDataSeriesFromDoc(xChartDoc, 0));
    CPPUNIT_ASSERT(xDataSeries.is());
    uno::Reference<beans::XPropertySet> xPropertySet;
    chart2::DataPointLabel aLabel;
    sal_Int32 nNumberFormat;
    bool bLinkNumberFormatToSource = false;
    const sal_Int32 nChartDataNumberFormat = getNumberFormat(
            xChartDoc, "_(\"$\"* #,##0_);_(\"$\"* \\(#,##0\\);_(\"$\"* \"-\"??_);_(@_)");

    xPropertySet.set(xDataSeries->getDataPointByIndex(0), uno::UNO_QUERY_THROW);
    xPropertySet->getPropertyValue("Label") >>= aLabel;
    CPPUNIT_ASSERT_EQUAL(sal_True, aLabel.ShowNumber);
    CPPUNIT_ASSERT_EQUAL(sal_True, aLabel.ShowNumberInPercent);
    xPropertySet->getPropertyValue(CHART_UNONAME_NUMFMT) >>= nNumberFormat;
    CPPUNIT_ASSERT_EQUAL(nChartDataNumberFormat, nNumberFormat);
    bool bSuccess = xPropertySet->getPropertyValue("PercentageNumberFormat") >>= nNumberFormat;
    CPPUNIT_ASSERT_EQUAL(false, bSuccess);
    bSuccess = xPropertySet->getPropertyValue(CHART_UNONAME_LINK_TO_SRC_NUMFMT) >>= bLinkNumberFormatToSource;
    CPPUNIT_ASSERT_MESSAGE("\"LinkNumberFormatToSource\" should be set to true.", bSuccess && bLinkNumberFormatToSource);

    xPropertySet.set(xDataSeries->getDataPointByIndex(1), uno::UNO_QUERY_THROW);
    xPropertySet->getPropertyValue("Label") >>= aLabel;
    CPPUNIT_ASSERT_EQUAL(sal_True, aLabel.ShowNumber);
    CPPUNIT_ASSERT_EQUAL(sal_False, aLabel.ShowNumberInPercent);
    xPropertySet->getPropertyValue(CHART_UNONAME_NUMFMT) >>= nNumberFormat;
    CPPUNIT_ASSERT_EQUAL(nChartDataNumberFormat, nNumberFormat);
    bSuccess = xPropertySet->getPropertyValue("PercentageNumberFormat") >>= nNumberFormat;
    CPPUNIT_ASSERT_EQUAL(false, bSuccess);
    bSuccess = xPropertySet->getPropertyValue(CHART_UNONAME_LINK_TO_SRC_NUMFMT) >>= bLinkNumberFormatToSource;
    CPPUNIT_ASSERT_MESSAGE("\"LinkNumberFormatToSource\" should be set to true.", bSuccess && bLinkNumberFormatToSource);

    xPropertySet.set(xDataSeries->getDataPointByIndex(2), uno::UNO_QUERY_THROW);
    xPropertySet->getPropertyValue("Label") >>= aLabel;
    CPPUNIT_ASSERT_EQUAL(sal_False, aLabel.ShowNumber);
    CPPUNIT_ASSERT_EQUAL(sal_True, aLabel.ShowNumberInPercent);
    xPropertySet->getPropertyValue(CHART_UNONAME_NUMFMT) >>= nNumberFormat;
    CPPUNIT_ASSERT_EQUAL(nChartDataNumberFormat, nNumberFormat);
    bSuccess = xPropertySet->getPropertyValue("PercentageNumberFormat") >>= nNumberFormat;
    CPPUNIT_ASSERT_EQUAL(false, bSuccess);
    bSuccess = xPropertySet->getPropertyValue(CHART_UNONAME_LINK_TO_SRC_NUMFMT) >>= bLinkNumberFormatToSource;
    CPPUNIT_ASSERT_MESSAGE("\"LinkNumberFormatToSource\" should be set to true.", bSuccess && bLinkNumberFormatToSource);
}

void Chart2ImportTest::testAutoTitleDelDefaultValue2007XLSX()
{
    load("/chart2/qa/extras/data/xlsx/", "autotitledel_2007.xlsx");
    Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChartDoc.is());

    Reference<chart2::XTitled> xTitled(xChartDoc, uno::UNO_QUERY_THROW);
    OUString aTitle = getTitleString(xTitled);
    CPPUNIT_ASSERT_MESSAGE("autoTitleDel default value is false in MSO 2007 documents",
            !aTitle.isEmpty());
}

void Chart2ImportTest::testAutoTitleDelDefaultValue2013XLSX()
{
    load("/chart2/qa/extras/data/xlsx/", "autotitledel_2013.xlsx");
    Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChartDoc.is());

    Reference<chart2::XTitled> xTitled(xChartDoc, uno::UNO_QUERY_THROW);
    uno::Reference<chart2::XTitle> xTitle = xTitled->getTitleObject();
    CPPUNIT_ASSERT_MESSAGE("autoTitleDel default value is true in the OOXML spec",
            !xTitle.is());
}

void Chart2ImportTest::testDispBlanksAsDefaultValue2007XLSX()
{
    load("/chart2/qa/extras/data/xlsx/", "dispBlanksAs_2007.xlsx");
    Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChartDoc.is());

    Reference<beans::XPropertySet> xDiagram(xChartDoc->getFirstDiagram(), UNO_QUERY);
    CPPUNIT_ASSERT(xDiagram.is());
    uno::Any aAny = xDiagram->getPropertyValue("MissingValueTreatment");
    sal_Int32 nMissingValueTreatment = -2;
    CPPUNIT_ASSERT(aAny >>= nMissingValueTreatment);
    CPPUNIT_ASSERT_EQUAL(chart::MissingValueTreatment::LEAVE_GAP, nMissingValueTreatment);
}

void Chart2ImportTest::testDispBlanksAsDefaultValue2013XLSX()
{
    load("/chart2/qa/extras/data/xlsx/", "dispBlanksAs_2013.xlsx");
    Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChartDoc.is());

    Reference<beans::XPropertySet> xDiagram(xChartDoc->getFirstDiagram(), UNO_QUERY);
    CPPUNIT_ASSERT(xDiagram.is());
    uno::Any aAny = xDiagram->getPropertyValue("MissingValueTreatment");
    sal_Int32 nMissingValueTreatment = -2;
    CPPUNIT_ASSERT(aAny >>= nMissingValueTreatment);
    CPPUNIT_ASSERT_EQUAL(chart::MissingValueTreatment::USE_ZERO, nMissingValueTreatment);
}

void Chart2ImportTest::testSmoothDefaultValue2007XLSX()
{
    load("/chart2/qa/extras/data/xlsx/", "smoothed_series2007.xlsx");
    Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChartDoc.is());
    CPPUNIT_ASSERT(xChartDoc.is());

    Reference< chart2::XChartType > xChartType = getChartTypeFromDoc( xChartDoc, 0 );
    CPPUNIT_ASSERT(xChartType.is());

    Reference< beans::XPropertySet > xPropSet( xChartType, UNO_QUERY );
    CPPUNIT_ASSERT(xPropSet.is());

    chart2::CurveStyle eCurveStyle;
    xPropSet->getPropertyValue("CurveStyle") >>= eCurveStyle;
    CPPUNIT_ASSERT_EQUAL(eCurveStyle, chart2::CurveStyle_LINES);
}

void Chart2ImportTest::testSmoothDefaultValue2013XLSX()
{
    load("/chart2/qa/extras/data/xlsx/", "smoothed_series.xlsx");
    Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChartDoc.is());
    CPPUNIT_ASSERT(xChartDoc.is());

    Reference< chart2::XChartType > xChartType = getChartTypeFromDoc( xChartDoc, 0 );
    CPPUNIT_ASSERT(xChartType.is());

    Reference< beans::XPropertySet > xPropSet( xChartType, UNO_QUERY );
    CPPUNIT_ASSERT(xPropSet.is());

    chart2::CurveStyle eCurveStyle;
    xPropSet->getPropertyValue("CurveStyle") >>= eCurveStyle;
    CPPUNIT_ASSERT(eCurveStyle != chart2::CurveStyle_LINES);
}

void Chart2ImportTest::testTrendlineDefaultValue2007XLSX()
{
    load("/chart2/qa/extras/data/xlsx/", "trendline2007.xlsx");
    Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChartDoc.is());
    CPPUNIT_ASSERT(xChartDoc.is());
    Reference<chart2::XDataSeries> xDataSeries = getDataSeriesFromDoc(xChartDoc, 0);
    CPPUNIT_ASSERT(xDataSeries.is());
    Reference<chart2::XRegressionCurveContainer> xRegressionCurveContainer(xDataSeries, UNO_QUERY_THROW);
    Sequence< Reference<chart2::XRegressionCurve> > xRegressionCurveSequence = xRegressionCurveContainer->getRegressionCurves();
    CPPUNIT_ASSERT_EQUAL((sal_Int32) 1, xRegressionCurveSequence.getLength());

    Reference<chart2::XRegressionCurve> xCurve = xRegressionCurveSequence[0];

    Reference<beans::XPropertySet> xPropSet(xCurve->getEquationProperties(), uno::UNO_QUERY_THROW);
    uno::Any aAny = xPropSet->getPropertyValue("ShowEquation");
    bool bShowEquation = true;
    CPPUNIT_ASSERT(aAny >>= bShowEquation);
    CPPUNIT_ASSERT(!bShowEquation);

    aAny = xPropSet->getPropertyValue("ShowCorrelationCoefficient");
    bool bShowCorrelation = true;
    CPPUNIT_ASSERT(aAny >>= bShowCorrelation);
    CPPUNIT_ASSERT(!bShowCorrelation);
}

void Chart2ImportTest::testTrendlineDefaultValue2013XLSX()
{
    load("/chart2/qa/extras/data/xlsx/", "trendline.xlsx");
    Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChartDoc.is());
    CPPUNIT_ASSERT(xChartDoc.is());
    Reference<chart2::XDataSeries> xDataSeries = getDataSeriesFromDoc(xChartDoc, 0);
    CPPUNIT_ASSERT(xDataSeries.is());
    Reference<chart2::XRegressionCurveContainer> xRegressionCurveContainer(xDataSeries, UNO_QUERY_THROW);
    Sequence< Reference<chart2::XRegressionCurve> > xRegressionCurveSequence = xRegressionCurveContainer->getRegressionCurves();
    CPPUNIT_ASSERT_EQUAL((sal_Int32) 1, xRegressionCurveSequence.getLength());

    Reference<chart2::XRegressionCurve> xCurve = xRegressionCurveSequence[0];

    Reference<beans::XPropertySet> xPropSet(xCurve->getEquationProperties(), uno::UNO_QUERY_THROW);
    uno::Any aAny = xPropSet->getPropertyValue("ShowEquation");
    bool bShowEquation = false;
    CPPUNIT_ASSERT(aAny >>= bShowEquation);
    CPPUNIT_ASSERT(bShowEquation);

    aAny = xPropSet->getPropertyValue("ShowCorrelationCoefficient");
    bool bShowCorrelation = false;
    CPPUNIT_ASSERT(aAny >>= bShowCorrelation);
    CPPUNIT_ASSERT(bShowCorrelation);
}

void Chart2ImportTest::testVaryColorDefaultValues2007XLSX()
{
    load("/chart2/qa/extras/data/xlsx/", "vary_color2007.xlsx");
    Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChartDoc.is());
    CPPUNIT_ASSERT(xChartDoc.is());
    Reference<chart2::XDataSeries> xDataSeries = getDataSeriesFromDoc(xChartDoc, 0);
    CPPUNIT_ASSERT(xDataSeries.is());
    Reference<beans::XPropertySet> xPropSet(xDataSeries, uno::UNO_QUERY_THROW);
    uno::Any aAny = xPropSet->getPropertyValue("VaryColorsByPoint");
    bool bVaryColor = true;
    CPPUNIT_ASSERT(aAny >>= bVaryColor);
    CPPUNIT_ASSERT(!bVaryColor);
}

void Chart2ImportTest::testVaryColorDefaultValues2013XLSX()
{
    load("/chart2/qa/extras/data/xlsx/", "vary_color.xlsx");
    Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChartDoc.is());
    CPPUNIT_ASSERT(xChartDoc.is());
    Reference<chart2::XDataSeries> xDataSeries = getDataSeriesFromDoc(xChartDoc, 0);
    CPPUNIT_ASSERT(xDataSeries.is());
    Reference<beans::XPropertySet> xPropSet(xDataSeries, uno::UNO_QUERY_THROW);
    uno::Any aAny = xPropSet->getPropertyValue("VaryColorsByPoint");
    bool bVaryColor = false;
    CPPUNIT_ASSERT(aAny >>= bVaryColor);
    CPPUNIT_ASSERT(bVaryColor);
}

void Chart2ImportTest::testPlotVisOnlyDefaultValue2013XLSX()
{
    load("/chart2/qa/extras/data/xlsx/", "plotVisOnly.xlsx");
    uno::Reference< chart::XChartDocument > xChart1Doc ( getChartCompFromSheet( 0, mxComponent ), UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChart1Doc.is());
    Reference<beans::XPropertySet> xPropSet(xChart1Doc->getDiagram(), uno::UNO_QUERY_THROW);
    uno::Any aAny = xPropSet->getPropertyValue("IncludeHiddenCells");
    CPPUNIT_ASSERT(aAny.hasValue());
    bool bShowHiddenValues = true;
    CPPUNIT_ASSERT(aAny >>= bShowHiddenValues);
    CPPUNIT_ASSERT(!bShowHiddenValues);
}

void Chart2ImportTest::testRAngAxDefaultValue2013XLSX()
{
    load("/chart2/qa/extras/data/xlsx/", "rAngAx.xlsx");
    uno::Reference< chart::XChartDocument > xChart1Doc ( getChartCompFromSheet( 0, mxComponent ), UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChart1Doc.is());
    Reference<beans::XPropertySet> xPropSet(xChart1Doc->getDiagram(), uno::UNO_QUERY_THROW);
    uno::Any aAny = xPropSet->getPropertyValue("RightAngledAxes");
    CPPUNIT_ASSERT(aAny.hasValue());
    bool bRightAngleAxes = false;
    CPPUNIT_ASSERT(aAny >>= bRightAngleAxes);
    CPPUNIT_ASSERT(bRightAngleAxes);
}

void Chart2ImportTest::testMajorTickMarksDefaultValue2013XLSX()
{
    load("/chart2/qa/extras/data/xlsx/", "majorTickMark.xlsx");
    Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChartDoc.is());
    Reference<chart2::XAxis> xXAxis = getAxisFromDoc(xChartDoc, 0, 0, 0);
    CPPUNIT_ASSERT(xXAxis.is());
    Reference<beans::XPropertySet> xPropSet(xXAxis, uno::UNO_QUERY_THROW);
    uno::Any aAny = xPropSet->getPropertyValue("MajorTickmarks");
    sal_Int32 nMajorTickmarks = chart2::TickmarkStyle::NONE;
    CPPUNIT_ASSERT(aAny.hasValue());
    CPPUNIT_ASSERT(aAny >>= nMajorTickmarks);
    CPPUNIT_ASSERT_EQUAL(chart2::TickmarkStyle::INNER | chart2::TickmarkStyle::OUTER, nMajorTickmarks);
}

void Chart2ImportTest::testMinorTickMarksDefaultValue2013XLSX()
{
    load("/chart2/qa/extras/data/xlsx/", "minorTickMark.xlsx");
    Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChartDoc.is());
    Reference<chart2::XAxis> xXAxis = getAxisFromDoc(xChartDoc, 0, 0, 0);
    CPPUNIT_ASSERT(xXAxis.is());
    Reference<beans::XPropertySet> xPropSet(xXAxis, uno::UNO_QUERY_THROW);
    uno::Any aAny = xPropSet->getPropertyValue("MinorTickmarks");
    sal_Int32 nMajorTickmarks = chart2::TickmarkStyle::NONE;
    CPPUNIT_ASSERT(aAny.hasValue());
    CPPUNIT_ASSERT(aAny >>= nMajorTickmarks);
    CPPUNIT_ASSERT_EQUAL(chart2::TickmarkStyle::INNER | chart2::TickmarkStyle::OUTER, nMajorTickmarks);
}

void Chart2ImportTest::testAxisTitleDefaultRotationXLSX()
{
    load("/chart2/qa/extras/data/xlsx/", "axis_title_default_rotation.xlsx");
    Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChartDoc.is());
    Reference<chart2::XAxis> xYAxis = getAxisFromDoc(xChartDoc, 0, 1, 0);
    CPPUNIT_ASSERT(xYAxis.is());
    Reference<chart2::XTitled> xTitled(xYAxis, uno::UNO_QUERY_THROW);
    Reference<chart2::XTitle> xTitle = xTitled->getTitleObject();
    CPPUNIT_ASSERT(xTitle.is());
    Reference<beans::XPropertySet> xPropSet(xTitle, uno::UNO_QUERY_THROW);
    uno::Any aAny = xPropSet->getPropertyValue("TextRotation");
    double nRotation = 0;
    CPPUNIT_ASSERT(aAny >>= nRotation);
    CPPUNIT_ASSERT_EQUAL(90.0, nRotation);
}

void Chart2ImportTest::testSecondaryAxisTitleDefaultRotationXLSX()
{
    load("/chart2/qa/extras/data/xlsx/", "secondary_axis_title_default_rotation.xlsx");
    Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChartDoc.is());
    Reference<chart2::XAxis> xYAxis = getAxisFromDoc(xChartDoc, 0, 1, 1);
    CPPUNIT_ASSERT(xYAxis.is());
    Reference<chart2::XTitled> xTitled(xYAxis, uno::UNO_QUERY_THROW);
    Reference<chart2::XTitle> xTitle = xTitled->getTitleObject();
    CPPUNIT_ASSERT(xTitle.is());
    Reference<beans::XPropertySet> xPropSet(xTitle, uno::UNO_QUERY_THROW);
    uno::Any aAny = xPropSet->getPropertyValue("TextRotation");
    double nRotation = 0;
    CPPUNIT_ASSERT(aAny >>= nRotation);
    CPPUNIT_ASSERT_EQUAL(90.0, nRotation);
}

void Chart2ImportTest::testAxisTitleRotationXLSX()
{
    load("/chart2/qa/extras/data/xlsx/", "axis_title_rotated.xlsx");
    Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChartDoc.is());
    {
        Reference<chart2::XAxis> xYAxis = getAxisFromDoc(xChartDoc, 0, 1, 0);
        CPPUNIT_ASSERT(xYAxis.is());
        Reference<chart2::XTitled> xTitled(xYAxis, uno::UNO_QUERY_THROW);
        Reference<chart2::XTitle> xTitle = xTitled->getTitleObject();
        CPPUNIT_ASSERT(xTitle.is());
        Reference<beans::XPropertySet> xPropSet(xTitle, uno::UNO_QUERY_THROW);
        uno::Any aAny = xPropSet->getPropertyValue("TextRotation");
        double nRotation = 0;
        CPPUNIT_ASSERT(aAny >>= nRotation);
        CPPUNIT_ASSERT_EQUAL(340.0, nRotation);
    }
    {
        Reference<chart2::XAxis> xYAxis = getAxisFromDoc(xChartDoc, 0, 1, 1);
        CPPUNIT_ASSERT(xYAxis.is());
        Reference<chart2::XTitled> xTitled(xYAxis, uno::UNO_QUERY_THROW);
        Reference<chart2::XTitle> xTitle = xTitled->getTitleObject();
        CPPUNIT_ASSERT(xTitle.is());
        Reference<beans::XPropertySet> xPropSet(xTitle, uno::UNO_QUERY_THROW);
        uno::Any aAny = xPropSet->getPropertyValue("TextRotation");
        double nRotation = 0;
        CPPUNIT_ASSERT(aAny >>= nRotation);
        CPPUNIT_ASSERT_EQUAL(270.0, nRotation);
    }

}

CPPUNIT_TEST_SUITE_REGISTRATION(Chart2ImportTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
