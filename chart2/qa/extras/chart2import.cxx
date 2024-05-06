/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "charttest.hxx"
#include <com/sun/star/chart2/CurveStyle.hpp>
#include <com/sun/star/chart2/DataPointLabel.hpp>
#include <com/sun/star/chart2/XDataPointCustomLabelField.hpp>
#include <com/sun/star/chart2/DataPointCustomLabelFieldType.hpp>
#include <com/sun/star/chart/ErrorBarStyle.hpp>
#include <com/sun/star/chart2/XRegressionCurveContainer.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart/XChartDocument.hpp>
#include <com/sun/star/chart2/XInternalDataProvider.hpp>
#include <com/sun/star/chart/XChartDataArray.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/chart/XAxisXSupplier.hpp>
#include <com/sun/star/chart/XAxisYSupplier.hpp>
#include <com/sun/star/chart/MissingValueTreatment.hpp>
#include <com/sun/star/chart2/TickmarkStyle.hpp>
#include <com/sun/star/chart2/SymbolStyle.hpp>
#include <com/sun/star/chart2/Symbol.hpp>
#include <com/sun/star/chart2/data/XTextualDataSequence.hpp>
#include <com/sun/star/chart/DataLabelPlacement.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/qa/XDumper.hpp>
#include <iterator>

#include <com/sun/star/util/Color.hpp>
#include <com/sun/star/awt/Gradient2.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <docmodel/uno/UnoGradientTools.hxx>

namespace
{
class Chart2ImportTest : public ChartTest
{
public:
    Chart2ImportTest() : ChartTest(u"/chart2/qa/extras/data/"_ustr) {}

protected:
    void testTransparentBackground(std::u16string_view filename);
};

OUString getShapeDump(css::uno::Reference<css::chart::XChartDocument> const& doc)
{
    return css::uno::Reference<css::qa::XDumper>(doc, css::uno::UNO_QUERY_THROW)->dump(u"shapes"_ustr);
}

// error bar import
// split method up into smaller chunks for more detailed tests
CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testFdo60083)
{
    loadFromFile(u"ods/fdo60083.ods");
    uno::Reference< chart2::XChartDocument > xChartDoc = getChartDocFromSheet( 0, mxComponent );
    CPPUNIT_ASSERT(xChartDoc.is());

    Reference< chart2::XDataSeries > xDataSeries = getDataSeriesFromDoc( xChartDoc, 0 );
    CPPUNIT_ASSERT( xDataSeries.is() );

    Reference< beans::XPropertySet > xPropSet( xDataSeries, UNO_QUERY_THROW );

    // test that y error bars are there
    Reference< beans::XPropertySet > xErrorBarYProps;
    xPropSet->getPropertyValue(CHART_UNONAME_ERRORBAR_Y) >>= xErrorBarYProps;
    CPPUNIT_ASSERT(xErrorBarYProps.is());
    {
        sal_Int32 nErrorBarStyle;
        CPPUNIT_ASSERT(
            xErrorBarYProps->getPropertyValue(u"ErrorBarStyle"_ustr)
            >>= nErrorBarStyle);
        CPPUNIT_ASSERT_EQUAL(
            chart::ErrorBarStyle::RELATIVE,
            nErrorBarStyle);

        double nVal = 0.0;
        CPPUNIT_ASSERT(
            xErrorBarYProps->getPropertyValue(u"PositiveError"_ustr) >>= nVal);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0, nVal, 1e-8);

        CPPUNIT_ASSERT(
            xErrorBarYProps->getPropertyValue(u"NegativeError"_ustr) >>= nVal);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0, nVal, 1e-8);

        bool bVal;
        CPPUNIT_ASSERT(
            xErrorBarYProps->getPropertyValue(u"ShowPositiveError"_ustr) >>= bVal);
        CPPUNIT_ASSERT_EQUAL(true, bVal);

        CPPUNIT_ASSERT(
            xErrorBarYProps->getPropertyValue(u"ShowNegativeError"_ustr) >>= bVal);
        CPPUNIT_ASSERT_EQUAL(true, bVal);
    }

    // test that x error bars are not imported
    Reference< beans::XPropertySet > xErrorBarXProps;
    xPropSet->getPropertyValue(CHART_UNONAME_ERRORBAR_X) >>= xErrorBarXProps;
    CPPUNIT_ASSERT(!xErrorBarXProps.is());
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testErrorBarRange)
{
    loadFromFile(u"ods/error_bar_range.ods");
    uno::Reference< chart2::XChartDocument > xChartDoc = getChartDocFromSheet( 0, mxComponent );
    CPPUNIT_ASSERT(xChartDoc.is());

    Reference< chart2::XDataSeries > xDataSeries = getDataSeriesFromDoc( xChartDoc, 0 );
    CPPUNIT_ASSERT( xDataSeries.is() );

    Reference< beans::XPropertySet > xPropSet( xDataSeries, UNO_QUERY_THROW );

    // test that y error bars are there
    Reference< beans::XPropertySet > xErrorBarYProps;
    xPropSet->getPropertyValue(CHART_UNONAME_ERRORBAR_Y) >>= xErrorBarYProps;
    CPPUNIT_ASSERT(xErrorBarYProps.is());

    sal_Int32 nErrorBarStyle;
    CPPUNIT_ASSERT(
            xErrorBarYProps->getPropertyValue(u"ErrorBarStyle"_ustr)
            >>= nErrorBarStyle);
    CPPUNIT_ASSERT_EQUAL(
            chart::ErrorBarStyle::FROM_DATA,
            nErrorBarStyle);

    OUString aRangePos;
    CPPUNIT_ASSERT(xErrorBarYProps->getPropertyValue(u"ErrorBarRangePositive"_ustr) >>= aRangePos);
    CPPUNIT_ASSERT_EQUAL(u"$Sheet1.$C$2:$C$4"_ustr, aRangePos);
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testErrorBarFormatting)
{
    loadFromFile(u"ods/error_bar_properties.ods");
    uno::Reference< chart2::XChartDocument > xChartDoc = getChartDocFromSheet( 0, mxComponent );
    CPPUNIT_ASSERT(xChartDoc.is());

    Reference< chart2::XDataSeries > xDataSeries = getDataSeriesFromDoc( xChartDoc, 0 );
    CPPUNIT_ASSERT( xDataSeries.is() );

    Reference< beans::XPropertySet > xPropSet( xDataSeries, UNO_QUERY_THROW );

    // test that y error bars are there
    Reference< beans::XPropertySet > xErrorBarYProps;
    xPropSet->getPropertyValue(CHART_UNONAME_ERRORBAR_Y) >>= xErrorBarYProps;
    CPPUNIT_ASSERT(xErrorBarYProps.is());

    util::Color aColor(0);
    xErrorBarYProps->getPropertyValue(u"LineColor"_ustr) >>= aColor;
    sal_uInt32 nColorValue = aColor;
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(0xff3333), nColorValue);
}

// stepped line interpolation
CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testSteppedLines)
{
    const sal_Int32 MAXSHEET = 14;
    chart2::CurveStyle const curveStyle[] = {
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

    loadFromFile(u"ods/stepped_lines.ods");
    for(sal_Int32 nSheet = 0; nSheet < MAXSHEET; ++nSheet)
    {
        uno::Reference< chart2::XChartDocument > xChart2Doc = getChartDocFromSheet( nSheet, mxComponent );
        CPPUNIT_ASSERT(xChart2Doc.is());

        Reference< chart2::XChartType > xChartType = getChartTypeFromDoc( xChart2Doc, 0 );
        CPPUNIT_ASSERT(xChartType.is());

        Reference< beans::XPropertySet > xPropSet( xChartType, UNO_QUERY );
        CPPUNIT_ASSERT(xPropSet.is());

        chart2::CurveStyle eCurveStyle;
        xPropSet->getPropertyValue(u"CurveStyle"_ustr) >>= eCurveStyle;
        CPPUNIT_ASSERT_EQUAL(eCurveStyle, curveStyle[nSheet]);
    }
}

uno::Sequence < OUString > getChartColumnDescriptions( uno::Reference< chart::XChartDocument > const & xChart1Doc)
{
    CPPUNIT_ASSERT(xChart1Doc.is());
    uno::Reference< chart::XChartDataArray > xChartData ( xChart1Doc->getData(), UNO_QUERY_THROW);
    uno::Sequence < OUString > seriesList = xChartData->getColumnDescriptions();
    return seriesList;
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testODSChartSeries)
{
    loadFromFile(u"ods/chart.ods");
    uno::Reference< chart::XChartDocument > xChart1Doc ( getChartCompFromSheet( 0, 0, mxComponent ), UNO_QUERY_THROW);
    uno::Sequence < OUString > seriesList = getChartColumnDescriptions( xChart1Doc);
    CPPUNIT_ASSERT_EQUAL(u"Col 1"_ustr, seriesList[0]);
    CPPUNIT_ASSERT_EQUAL(u"Col2"_ustr, seriesList[1]);
    CPPUNIT_ASSERT_EQUAL(u"Col 33"_ustr, seriesList[2]);

}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testXLSXChartSeries)
{
    loadFromFile(u"xlsx/chart.xlsx");
    uno::Reference< chart::XChartDocument > xChart1Doc ( getChartCompFromSheet( 0, 0, mxComponent ), UNO_QUERY_THROW);
    uno::Sequence < OUString > seriesList = getChartColumnDescriptions(xChart1Doc );
    CPPUNIT_ASSERT_EQUAL(u"Col 1"_ustr, seriesList[0]);
    CPPUNIT_ASSERT_EQUAL(u"Col2"_ustr, seriesList[1]);
    CPPUNIT_ASSERT_EQUAL(u"Col 33"_ustr, seriesList[2]);

}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testXLSChartSeries)
{
    loadFromFile(u"xls/chart.xls");
    uno::Reference< chart::XChartDocument > xChart1Doc ( getChartCompFromSheet( 0, 0, mxComponent ), UNO_QUERY_THROW);
    uno::Sequence < OUString > seriesList = getChartColumnDescriptions(xChart1Doc );
    CPPUNIT_ASSERT_EQUAL(u"Col 1"_ustr, seriesList[0]);
    CPPUNIT_ASSERT_EQUAL(u"Col 2"_ustr, seriesList[1]);
    CPPUNIT_ASSERT_EQUAL(u"Col 3"_ustr, seriesList[2]);

}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testODTChartSeries)
{
    loadFromFile(u"odt/chart.odt");
    uno::Sequence< OUString > seriesList = getWriterChartColumnDescriptions(mxComponent);
    CPPUNIT_ASSERT_EQUAL(u"Column 1"_ustr, seriesList[0]);
    CPPUNIT_ASSERT_EQUAL(u"Column 2"_ustr, seriesList[1]);
    CPPUNIT_ASSERT_EQUAL(u"Column 3"_ustr, seriesList[2]);

}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testDOCChartSeries)
{
    loadFromFile(u"doc/chart.doc");
    uno::Sequence< OUString > seriesList = getWriterChartColumnDescriptions(mxComponent);
    CPPUNIT_ASSERT_EQUAL(u"Column 1"_ustr, seriesList[0]);
    CPPUNIT_ASSERT_EQUAL(u"Column 2"_ustr, seriesList[1]);
    CPPUNIT_ASSERT_EQUAL(u"Column 3"_ustr, seriesList[2]);
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testDOCXChartSeries)
{
    loadFromFile(u"docx/chart.docx");
    Reference<chart2::XChartDocument> xChartDoc(getChartDocFromWriter(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());

    Reference<chart2::XChartType> xCT = getChartTypeFromDoc(xChartDoc, 0);
    CPPUNIT_ASSERT(xCT.is());

    std::vector<uno::Sequence<uno::Any> > aLabels = getDataSeriesLabelsFromChartType(xCT);
    CPPUNIT_ASSERT_EQUAL(size_t(3), aLabels.size());
    CPPUNIT_ASSERT_EQUAL(u"Series 1"_ustr, aLabels[0][0].get<OUString>());
    CPPUNIT_ASSERT_EQUAL(u"Series 2"_ustr, aLabels[1][0].get<OUString>());
    CPPUNIT_ASSERT_EQUAL(u"Series 3"_ustr, aLabels[2][0].get<OUString>());
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testDOCXChartEmptySeries)
{
    loadFromFile(u"docx/tdf125337.docx");
    Reference<chart2::XChartDocument> xChartDoc(getChartDocFromWriter(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());

    Reference<chart2::XChartType> xCT = getChartTypeFromDoc(xChartDoc, 0);
    CPPUNIT_ASSERT(xCT.is());

    std::vector<uno::Sequence<uno::Any> > aLabels = getDataSeriesLabelsFromChartType(xCT);
    CPPUNIT_ASSERT_EQUAL(size_t(3), aLabels.size());
    CPPUNIT_ASSERT_EQUAL(u"1. dataseries"_ustr, aLabels[0][0].get<OUString>());
    CPPUNIT_ASSERT_EQUAL(u"2. dataseries"_ustr, aLabels[1][0].get<OUString>());
    CPPUNIT_ASSERT_EQUAL(u"Column 3"_ustr, aLabels[2][0].get<OUString>());

    //test chart series sparse data for docx
    std::vector<std::vector<double> > aValues = getDataSeriesYValuesFromChartType(xCT);
    CPPUNIT_ASSERT_EQUAL(size_t(3), aValues.size());
    //test the second series values
    CPPUNIT_ASSERT_EQUAL(2.4, aValues[1][0]);
    CPPUNIT_ASSERT_EQUAL(4.4, aValues[1][1]);
    //test the third series (empty) values
    CPPUNIT_ASSERT(std::isnan(aValues[2][0]));
    CPPUNIT_ASSERT(std::isnan(aValues[2][1]));
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testTdf81396)
{
    loadFromFile(u"xlsx/tdf81396.xlsx");
    Reference<chart::XChartDocument> xChartDoc(getChartDocFromSheet(0, mxComponent),
                                               UNO_QUERY_THROW);

    Reference<chart2::XChartDocument> xChartDoc2(xChartDoc, UNO_QUERY_THROW);
    Reference<chart2::XChartType> xChartType(getChartTypeFromDoc(xChartDoc2, 0), UNO_SET_THROW);
    std::vector aDataSeriesYValues = getDataSeriesYValuesFromChartType(xChartType);
    CPPUNIT_ASSERT_EQUAL(size_t(1), aDataSeriesYValues.size());

    // Without the fix in place, this test would have failed with
    // - Expected: 105.210801910481
    // - Actual  : nan
    CPPUNIT_ASSERT_EQUAL(105.210801910481, aDataSeriesYValues[0][0]);
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testPPTXChartErrorBars)
{
    loadFromFile(u"pptx/tdf127720.pptx");
    Reference<chart2::XChartDocument> xChartDoc(getChartDocFromDrawImpress(0, 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());

    uno::Reference< chart::XChartDataArray > xDataArray(xChartDoc->getDataProvider(), UNO_QUERY_THROW);
    Sequence<OUString> aColumnDesc = xDataArray->getColumnDescriptions();
    // Number of columns = 4 (Y-values, X-values and positive/negative error bars).
    // Without the fix there would only be 2 columns (no error range).
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There must be 4 columns and descriptions", static_cast<sal_Int32>(4), aColumnDesc.getLength());
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testDOCXChartValuesSize)
{
    loadFromFile(u"docx/bubblechart.docx" );
    Reference<chart2::XChartDocument> xChartDoc( getChartDocFromWriter(0), uno::UNO_QUERY );
    CPPUNIT_ASSERT( xChartDoc.is() );

    uno::Reference< chart::XChartDataArray > xDataArray( xChartDoc->getDataProvider(), UNO_QUERY_THROW );
    Sequence<OUString> aColumnDesc = xDataArray->getColumnDescriptions();
    // Number of columns = 3 (Y-values, X-values and bubble sizes).
    // Without the fix there would only be 2 columns (no bubble sizes).
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "There must be 3 columns and descriptions", static_cast<sal_Int32>(3), aColumnDesc.getLength() );
    Sequence<Sequence<double>> aData = xDataArray->getData();
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "There must be exactly 3 data points", static_cast<sal_Int32>(3), aData.getLength() );

    std::vector<std::vector<double>> aExpected = { { 2.7, 0.7, 10.0 }, { 3.2, 1.8, 4.0 }, { 0.8, 2.6, 8.0 } };

    for ( sal_Int32 nRowIdx = 0; nRowIdx < 3; ++nRowIdx )
        for( sal_Int32 nColIdx = 0; nColIdx < 3; ++nColIdx )
            CPPUNIT_ASSERT_DOUBLES_EQUAL( aExpected[nRowIdx][nColIdx], aData[nRowIdx][nColIdx], 1e-1 );
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testPPTChartSeries)
{
    //test chart series names for ppt
    loadFromFile(u"ppt/chart.ppt");
    uno::Sequence < OUString > seriesList = getImpressChartColumnDescriptions(0, 0);

    CPPUNIT_ASSERT_EQUAL(u"Column 1"_ustr, seriesList[0]);
    CPPUNIT_ASSERT_EQUAL(u"Column 2"_ustr, seriesList[1]);
    CPPUNIT_ASSERT_EQUAL(u"Column 3"_ustr, seriesList[2]);

}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testPPTXChartSeries)
{
    //test chart series names for pptx
    loadFromFile(u"pptx/chart.pptx");
    Reference<chart2::XChartDocument> xChartDoc(getChartDocFromDrawImpress(0, 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());

    Reference<chart2::XChartType> xCT = getChartTypeFromDoc(xChartDoc, 0);
    CPPUNIT_ASSERT(xCT.is());

    std::vector<uno::Sequence<uno::Any> > aLabels = getDataSeriesLabelsFromChartType(xCT);
    CPPUNIT_ASSERT_EQUAL(size_t(3), aLabels.size());
    CPPUNIT_ASSERT_EQUAL(u"Column 1"_ustr, aLabels[0][0].get<OUString>());
    CPPUNIT_ASSERT_EQUAL(u"Column 2"_ustr, aLabels[1][0].get<OUString>());
    CPPUNIT_ASSERT_EQUAL(u"Column 3"_ustr, aLabels[2][0].get<OUString>());
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testPPTXSparseChartSeries)
{
    //test chart series sparse data for pptx
    loadFromFile(u"pptx/sparse-chart.pptx");
    Reference<chart2::XChartDocument> xChartDoc(getChartDocFromDrawImpress(0, 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());

    Reference<chart2::XChartType> xCT = getChartTypeFromDoc(xChartDoc, 0);
    CPPUNIT_ASSERT(xCT.is());

    std::vector<std::vector<double> > aValues = getDataSeriesYValuesFromChartType(xCT);
    CPPUNIT_ASSERT_EQUAL(size_t(2), aValues.size());
    CPPUNIT_ASSERT( std::isnan( aValues[0][0] ) );
    CPPUNIT_ASSERT_EQUAL(2.5,  aValues[0][1]);
    CPPUNIT_ASSERT_EQUAL(3.5,  aValues[0][2]);
    CPPUNIT_ASSERT( std::isnan( aValues[0][3] ) );
    CPPUNIT_ASSERT_EQUAL(-2.4, aValues[1][0]);
    CPPUNIT_ASSERT( std::isnan( aValues[1][1] ) );
    CPPUNIT_ASSERT( std::isnan( aValues[1][2] ) );
    CPPUNIT_ASSERT_EQUAL(-2.8, aValues[1][3]);
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testPPTXHiddenDataSeries)
{
    /**
     * Original data contains 3 series but 2 of them are hidden. For now, we
     * detect and skip those hidden series on import (since we don't support
     * hidden columns for internal data table yet).
     */
    loadFromFile(u"pptx/stacked-bar-chart-hidden-series.pptx");
    Reference<chart2::XChartDocument> xChartDoc(getChartDocFromDrawImpress(0, 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());

    // "Automatic" chart background fill in pptx should be loaded as no fill.
    Reference<beans::XPropertySet> xPropSet = xChartDoc->getPageBackground();
    CPPUNIT_ASSERT(xPropSet.is());
    drawing::FillStyle eStyle = xPropSet->getPropertyValue(u"FillStyle"_ustr).get<drawing::FillStyle>();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("'Automatic' chart background fill in pptx should be loaded as no fill (transparent).",
        drawing::FillStyle_NONE, eStyle);

    Reference<chart2::XChartType> xCT = getChartTypeFromDoc(xChartDoc, 0);
    CPPUNIT_ASSERT(xCT.is());

    // There should be only one data series present.
    std::vector<uno::Sequence<uno::Any> > aLabels = getDataSeriesLabelsFromChartType(xCT);
    CPPUNIT_ASSERT_EQUAL(size_t(1), aLabels.size());
    CPPUNIT_ASSERT_EQUAL(u"Series 3"_ustr, aLabels[0][0].get<OUString>());

    // Test the internal data.
    CPPUNIT_ASSERT(xChartDoc->hasInternalDataProvider());

    Reference<chart2::XInternalDataProvider> xInternalProvider(xChartDoc->getDataProvider(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xInternalProvider.is());

    Reference<chart::XComplexDescriptionAccess> xDescAccess(xInternalProvider, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDescAccess.is());

    // Get the category labels.
    Sequence<Sequence<OUString> > aCategories = xDescAccess->getComplexRowDescriptions();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), aCategories.getLength());
    CPPUNIT_ASSERT_EQUAL(u"Category 1"_ustr, aCategories[0][0]);
    CPPUNIT_ASSERT_EQUAL(u"Category 2"_ustr, aCategories[1][0]);
    CPPUNIT_ASSERT_EQUAL(u"Category 3"_ustr, aCategories[2][0]);
    CPPUNIT_ASSERT_EQUAL(u"Category 4"_ustr, aCategories[3][0]);
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testPPTXPercentageNumberFormats)
{
    loadFromFile(u"pptx/percentage-number-formats.pptx");

    // 1st chart
    Reference<chart2::XChartDocument> xChartDoc(getChartDocFromDrawImpress(0, 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());

    uno::Reference<chart2::XDataSeries> xDataSeries(getDataSeriesFromDoc(xChartDoc, 0));
    CPPUNIT_ASSERT(xDataSeries.is());
    uno::Reference<beans::XPropertySet> xPropertySet;
    chart2::DataPointLabel aLabel;
    sal_Int32 nNumberFormat;
    const sal_Int32 nPercentFormatSimple = getNumberFormat(xChartDoc, u"0%"_ustr);
    const sal_Int32 nPercentFormatDecimal = getNumberFormat(xChartDoc, u"0.00%"_ustr);

    xPropertySet.set(xDataSeries->getDataPointByIndex(0), uno::UNO_SET_THROW);
    xPropertySet->getPropertyValue(u"Label"_ustr) >>= aLabel;
    CPPUNIT_ASSERT_EQUAL(sal_True, aLabel.ShowNumber);
    CPPUNIT_ASSERT_EQUAL(sal_True, aLabel.ShowNumberInPercent);
    xPropertySet->getPropertyValue(u"PercentageNumberFormat"_ustr) >>= nNumberFormat;
    CPPUNIT_ASSERT_EQUAL(nPercentFormatSimple, nNumberFormat);

    xPropertySet.set(xDataSeries->getDataPointByIndex(1), uno::UNO_SET_THROW);
    xPropertySet->getPropertyValue(u"Label"_ustr) >>= aLabel;
    CPPUNIT_ASSERT_EQUAL(sal_True, aLabel.ShowNumber);
    CPPUNIT_ASSERT_EQUAL(sal_True, aLabel.ShowNumberInPercent);
    xPropertySet->getPropertyValue(u"PercentageNumberFormat"_ustr) >>= nNumberFormat;
    CPPUNIT_ASSERT_EQUAL(nPercentFormatDecimal, nNumberFormat);

    xPropertySet.set(xDataSeries->getDataPointByIndex(2), uno::UNO_SET_THROW);
    xPropertySet->getPropertyValue(u"Label"_ustr) >>= aLabel;
    CPPUNIT_ASSERT_EQUAL(sal_False, aLabel.ShowNumber);
    CPPUNIT_ASSERT_EQUAL(sal_True, aLabel.ShowNumberInPercent);
    xPropertySet->getPropertyValue(u"PercentageNumberFormat"_ustr) >>= nNumberFormat;
    CPPUNIT_ASSERT_EQUAL(nPercentFormatSimple, nNumberFormat);

    xPropertySet.set(xDataSeries->getDataPointByIndex(3), uno::UNO_SET_THROW);
    xPropertySet->getPropertyValue(u"Label"_ustr) >>= aLabel;
    CPPUNIT_ASSERT_EQUAL(sal_False, aLabel.ShowNumber);
    CPPUNIT_ASSERT_EQUAL(sal_True, aLabel.ShowNumberInPercent);
    xPropertySet->getPropertyValue(u"PercentageNumberFormat"_ustr) >>= nNumberFormat;
    CPPUNIT_ASSERT_EQUAL(nPercentFormatDecimal, nNumberFormat);

    // 2nd chart
    xChartDoc.set(getChartDocFromDrawImpress(1, 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());

    Reference<chart2::XAxis> xYAxis = getAxisFromDoc(xChartDoc, 0, 1, 0);
    CPPUNIT_ASSERT(xYAxis.is());

    Reference<beans::XPropertySet> xPS(xYAxis, uno::UNO_QUERY_THROW);
    bool bLinkNumberFormatToSource = true;
    bool bSuccess = xPS->getPropertyValue(CHART_UNONAME_LINK_TO_SRC_NUMFMT) >>= bLinkNumberFormatToSource;
    CPPUNIT_ASSERT_MESSAGE("\"LinkNumberFormatToSource\" should be set to false.", bSuccess);
    CPPUNIT_ASSERT_MESSAGE("\"LinkNumberFormatToSource\" should be set to false.", !bLinkNumberFormatToSource);

    // FIXME: This should be in fact "0.00%".
    // see TODO in oox/source/drawingml/chart/modelbase.cxx
    const sal_Int32 nPercentFormatDecimalShort = getNumberFormat(xChartDoc, u"0.0%"_ustr);
    nNumberFormat = getNumberFormatFromAxis(xYAxis);
    CPPUNIT_ASSERT_EQUAL(nPercentFormatDecimalShort, nNumberFormat);
    sal_Int16 nType = getNumberFormatType(xChartDoc, nNumberFormat);
    CPPUNIT_ASSERT_MESSAGE("Y axis should be a percent format.", (nType & util::NumberFormat::PERCENT));
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testPieChartLabelsNumFormat)
{
    loadFromFile(u"xlsx/tdfPieNumFormat.xlsx");
    uno::Reference< chart::XChartDocument > xChartDoc(getChartCompFromSheet(0, 0, mxComponent), UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xChartDoc.is());
    // test data point labels format
    Reference<beans::XPropertySet> xDataPointPropSet(xChartDoc->getDiagram()->getDataPointProperties(0, 0), uno::UNO_SET_THROW);
    chart2::DataPointLabel aLabel;
    xDataPointPropSet->getPropertyValue(u"Label"_ustr) >>= aLabel;
    CPPUNIT_ASSERT_EQUAL(sal_True, aLabel.ShowNumber);
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testPPTXStackedNonStackedYAxis)
{
    loadFromFile(u"pptx/stacked-non-stacked-mix-y-axis.pptx");

    // 1st chart is a normal stacked column.
    Reference<chart2::XChartDocument> xChartDoc(getChartDocFromDrawImpress(0, 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());

    Reference<chart2::XTitled> xTitled(xChartDoc, uno::UNO_QUERY_THROW);
    OUString aTitle = getTitleString(xTitled);
    CPPUNIT_ASSERT_EQUAL(u"Stacked"_ustr, aTitle);

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
    CPPUNIT_ASSERT_EQUAL(u"100% Stacked"_ustr, aTitle);

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
    CPPUNIT_ASSERT_EQUAL(u"Stacked column mixed with 100% stacked area"_ustr, aTitle);

    // Get the Y-axis.
    xYAxis = getAxisFromDoc(xChartDoc, 0, 1, 0);
    CPPUNIT_ASSERT(xYAxis.is());

    // Get the number format of the Y-axis.
    nNumberFormat = getNumberFormatFromAxis(xYAxis);
    nType = getNumberFormatType(xChartDoc, nNumberFormat);
    CPPUNIT_ASSERT_MESSAGE("Y axis should be a normal number format.", (nType & util::NumberFormat::NUMBER));
    CPPUNIT_ASSERT_MESSAGE("Y axis should NOT be a percent format.", !(nType & util::NumberFormat::PERCENT));
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testODPChartSeries)
{
    //test chart series names for odp
    loadFromFile(u"odp/chart.odp");
    uno::Sequence < OUString > seriesList = getImpressChartColumnDescriptions(0, 0);
    CPPUNIT_ASSERT_EQUAL(u"Column 1"_ustr, seriesList[0]);
    CPPUNIT_ASSERT_EQUAL(u"Column 2"_ustr, seriesList[1]);
    CPPUNIT_ASSERT_EQUAL(u"Column 3"_ustr, seriesList[2]);

}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testBnc864396)
{
    loadFromFile(u"pptx/bnc864396.pptx");
    uno::Reference< chart2::XChartDocument > xChartDoc(getChartDocFromDrawImpress(0,0), uno::UNO_QUERY_THROW);
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

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testBnc889755)
{
    loadFromFile(u"pptx/bnc889755.pptx");
    uno::Reference<chart2::XChartDocument> xChartDoc(getChartDocFromDrawImpress(0, 5), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xChartDoc->hasInternalDataProvider());

    constexpr sal_Int32 nNumCategories = 16;
    Sequence<OUString> aDateSeq = getFormattedDateCategories(xChartDoc);

    CPPUNIT_ASSERT_EQUAL(nNumCategories, aDateSeq.getLength());

    const OUString aExpectedDateCategories[nNumCategories] = {
        u"Oct-12"_ustr, u"Nov-12"_ustr, u"Dec-12"_ustr, u"Jan-13"_ustr,
        u"Feb-13"_ustr, u"Mar-13"_ustr, u"Apr-13"_ustr, u"May-13"_ustr,
        u"Jun-13"_ustr, u"Jul-13"_ustr, u"Aug-13"_ustr, u"Sep-13"_ustr,
        u"Oct-13"_ustr, u"Nov-13"_ustr, u"Dec-13"_ustr, u"Jan-14"_ustr,
    };

    for (size_t nIdx = 0; nIdx < nNumCategories; ++nIdx)
        CPPUNIT_ASSERT_EQUAL(aExpectedDateCategories[nIdx], aDateSeq[nIdx]);

    //tdf#139940 - the title's gradient was lost and was filled with solid blue, instead of a "blue underline".
    uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(0), uno::UNO_QUERY_THROW);

    // Shape "Title 3"
    // MCGR: Use the whole completely imported transparency gradient to check for correctness
    uno::Reference<beans::XPropertySet> xShapeProps(xPage->getByIndex(4), uno::UNO_QUERY_THROW);
    awt::Gradient2 aTransparence;
    xShapeProps->getPropertyValue(u"FillTransparenceGradient"_ustr) >>= aTransparence;
    const basegfx::BColorStops aColorStops = model::gradient::getColorStopsFromUno(aTransparence.ColorStops);

    CPPUNIT_ASSERT_EQUAL(size_t(3), aColorStops.size());
    CPPUNIT_ASSERT(basegfx::fTools::equal(aColorStops[0].getStopOffset(), 0.0));
    CPPUNIT_ASSERT_EQUAL(Color(0x404040), Color(aColorStops[0].getStopColor()));
    CPPUNIT_ASSERT(basegfx::fTools::equal(aColorStops[1].getStopOffset(), 0.070000000000000007));
    CPPUNIT_ASSERT_EQUAL(Color(0x404040), Color(aColorStops[1].getStopColor()));
    CPPUNIT_ASSERT(basegfx::fTools::equal(aColorStops[2].getStopOffset(), 0.080000000000000002));
    CPPUNIT_ASSERT_EQUAL(COL_WHITE,  Color(aColorStops[2].getStopColor()));
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testBnc882383)
{
    loadFromFile(u"pptx/bnc882383.pptx");
    uno::Reference<chart2::XChartDocument> xChartDoc(getChartDocFromDrawImpress(0, 0), uno::UNO_QUERY_THROW);
    uno::Reference<chart2::XDataSeries> xDataSeries(getDataSeriesFromDoc(xChartDoc, 0));
    CPPUNIT_ASSERT(xDataSeries.is());

    uno::Reference<beans::XPropertySet> xPropertySet(xDataSeries->getDataPointByIndex(0), uno::UNO_SET_THROW);
    OUString sGradientName;
    xPropertySet->getPropertyValue(u"GradientName"_ustr) >>= sGradientName;
    CPPUNIT_ASSERT(!sGradientName.isEmpty());
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testTransparencyGradientValue)
{
    loadFromFile(u"xlsx/tdf128732.xlsx");
    uno::Reference< chart2::XChartDocument > xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT(xChartDoc.is());
    uno::Reference<chart2::XDataSeries> xDataSeries(getDataSeriesFromDoc(xChartDoc, 0));
    CPPUNIT_ASSERT(xDataSeries.is());

    uno::Reference<beans::XPropertySet> xPropertySet(xDataSeries->getDataPointByIndex(0), uno::UNO_SET_THROW);
    OUString sTranspGradientName;
    xPropertySet->getPropertyValue(u"FillTransparenceGradientName"_ustr) >>= sTranspGradientName;
    CPPUNIT_ASSERT(!sTranspGradientName.isEmpty());

    awt::Gradient2 aTransparenceGradient;
    uno::Reference< lang::XMultiServiceFactory > xFact(xChartDoc, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xFact.is());
    uno::Reference< container::XNameAccess > xTransparenceGradient(xFact->createInstance(u"com.sun.star.drawing.TransparencyGradientTable"_ustr), uno::UNO_QUERY);
    uno::Any rTransparenceValue = xTransparenceGradient->getByName(sTranspGradientName);
    CPPUNIT_ASSERT(rTransparenceValue >>= aTransparenceGradient);
    const basegfx::BColorStops aColorStops = model::gradient::getColorStopsFromUno(aTransparenceGradient.ColorStops);

    // MCGR: Use the whole completely imported transparency gradient to check for correctness
    CPPUNIT_ASSERT_EQUAL(size_t(2), aColorStops.size());
    CPPUNIT_ASSERT(basegfx::fTools::equal(aColorStops[0].getStopOffset(), 0.0));
    CPPUNIT_ASSERT_EQUAL(Color(0x4d4d4d), Color(aColorStops[0].getStopColor()));
    CPPUNIT_ASSERT(basegfx::fTools::equal(aColorStops[1].getStopOffset(), 1.0));
    CPPUNIT_ASSERT_EQUAL(Color(0x333333), Color(aColorStops[1].getStopColor()));
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testSimpleStrictXLSX)
{
    loadFromFile(u"xlsx/strict_chart.xlsx");
    uno::Reference< chart2::XChartDocument > xChartDoc = getChartDocFromSheet( 0, mxComponent );
    CPPUNIT_ASSERT(xChartDoc.is());

    Reference< chart2::XDataSeries > xDataSeries = getDataSeriesFromDoc( xChartDoc, 0 );
    CPPUNIT_ASSERT(xDataSeries.is());

}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testDelayedCellImport)
{
    // chart range referencing content on later sheets
    loadFromFile(u"xlsx/fdo70609.xlsx");
    uno::Reference< chart2::XChartDocument > xChartDoc = getChartDocFromSheet( 0, mxComponent );
    Reference< chart2::data::XDataSequence > xDataSeq =
        getDataSequenceFromDocByRole(xChartDoc, u"values-x");

    OUString aRange = xDataSeq->getSourceRangeRepresentation();
    CPPUNIT_ASSERT_EQUAL(u"$Sheet2.$C$5:$C$9"_ustr, aRange);
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testFlatODSStackedColumnChart)
{
    loadFromFile(u"fods/stacked-column-chart.fods");
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

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testFdo78080)
{
    loadFromFile(u"xlsx/fdo78080.xlsx");
    Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT(xChartDoc.is());

    Reference<chart2::XTitled> xTitled(xChartDoc, uno::UNO_QUERY_THROW);
    Reference<chart2::XTitle> xTitle = xTitled->getTitleObject();
    CPPUNIT_ASSERT(!xTitle.is());
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testTdf127811)
{
    loadFromFile(u"pptx/tdf127811.pptx");
    Reference<chart2::XChartDocument> xChartDoc(getChartDocFromDrawImpress(0, 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());

    Reference<chart2::XChartType> xCT = getChartTypeFromDoc(xChartDoc, 0);
    CPPUNIT_ASSERT(xCT.is());

    std::vector<uno::Sequence<uno::Any> > aLabels = getDataSeriesLabelsFromChartType(xCT);
    CPPUNIT_ASSERT_EQUAL(size_t(2), aLabels.size());

    // Without the fix in place, this test would have failed with
    // - Expected: 1. first
    // - Actual  : 2. second
    CPPUNIT_ASSERT_EQUAL(u"1. first"_ustr, aLabels[0][0].get<OUString>());
    CPPUNIT_ASSERT_EQUAL(u"2. second"_ustr, aLabels[1][0].get<OUString>());
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testTdf86624)
{
    // manually placed legends
    loadFromFile(u"ods/tdf86624.ods");
    uno::Reference< chart2::XChartDocument > xChart2Doc = getChartDocFromSheet(0, mxComponent);
    uno::Reference< chart::XChartDocument > xChartDoc (xChart2Doc, uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xLegend = xChartDoc->getLegend();
    awt::Point aPos = xLegend->getPosition();
    CPPUNIT_ASSERT(aPos.X > 5000); // real value for me is above 8000 but before bug fix is below 1000
    CPPUNIT_ASSERT(aPos.Y > 4000); // real value for ms is above 7000
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testTdf105517)
{
    loadFromFile(u"pptx/tdf105517.pptx");
    Reference<chart2::XChartDocument> xChartDoc(getChartDocFromDrawImpress(0, 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());

    Reference<chart2::XCoordinateSystemContainer> xCoordContainer(xChartDoc->getFirstDiagram(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xCoordContainer.is());
    Reference<chart2::XChartTypeContainer> xChartTypeContainer(xCoordContainer->getCoordinateSystems()[0], uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartTypeContainer.is());
    Reference<chart2::XDataSeriesContainer> xDSContainer(xChartTypeContainer->getChartTypes()[0], uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDSContainer.is());
    Reference<beans::XPropertySet> xPropSet1(xDSContainer->getDataSeries()[0], uno::UNO_QUERY);
    CPPUNIT_ASSERT(xPropSet1.is());

    tools::Long lineColor;
    xPropSet1->getPropertyValue(u"Color"_ustr) >>= lineColor;
    // incorrect line color was 0x4a7ebb due to not handling themeOverride
    CPPUNIT_ASSERT_EQUAL(tools::Long(0xeaa700), lineColor);

    Reference<beans::XPropertySet> xPropSet2(xDSContainer->getDataSeries()[1], uno::UNO_QUERY);
    CPPUNIT_ASSERT(xPropSet2.is());

    xPropSet2->getPropertyValue(u"Color"_ustr) >>= lineColor;
    // incorrect line color was 0x98b855
    CPPUNIT_ASSERT_EQUAL(tools::Long(0x1e69a8), lineColor);
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testTdf106217)
{
    loadFromFile(u"pptx/tdf106217.pptx");
    uno::Reference< chart::XChartDocument > xChartDoc = getChartDocFromDrawImpress(0, 0);
    CPPUNIT_ASSERT(xChartDoc.is());

    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xChartDoc, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    uno::Reference<drawing::XShape> xCircle(xDrawPage->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xCircle.is());

    uno::Reference<container::XNamed> xNamedShape(xCircle, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Oval 1"_ustr, xNamedShape->getName());

    awt::Point aPosition = xCircle->getPosition();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6870), aPosition.X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(7261), aPosition.Y);
    awt::Size aSize = xCircle->getSize();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2701), aSize.Width);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2700), aSize.Height);
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testTdf108021)
{
    // Tdf108021 : To check TextBreak value is true.
    loadFromFile(u"ods/tdf108021.ods");
    uno::Reference< chart::XDiagram > mxDiagram;
    uno::Reference< beans::XPropertySet > xAxisProp;
    bool bTextBreak = false;
    uno::Reference< chart::XChartDocument > xChartDoc ( getChartCompFromSheet( 0, 0, mxComponent ), UNO_QUERY_THROW);
    mxDiagram.set(xChartDoc->getDiagram());
    CPPUNIT_ASSERT(mxDiagram.is());
    uno::Reference< chart::XAxisXSupplier > xAxisXSupp( mxDiagram, uno::UNO_QUERY );
    CPPUNIT_ASSERT(xAxisXSupp.is());
    xAxisProp = xAxisXSupp->getXAxis();
    xAxisProp->getPropertyValue(u"TextBreak"_ustr) >>= bTextBreak;
    // Expected value of 'TextBreak' is true
    CPPUNIT_ASSERT(bTextBreak);
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testTdf100084)
{
    // The test file was created with IBM Cognos, make sure there is a diagram.
    loadFromFile(u"xlsx/tdf100084.xlsx");
    Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChartDoc.is());
    Reference<beans::XPropertySet> xDiagram(xChartDoc->getFirstDiagram(), UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("There should be a Diagram.", xDiagram.is());
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testTdf124817)
{
    loadFromFile(u"xlsx/tdf124817.xlsx");
    Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChartDoc.is());

    uno::Reference<chart2::XDataSeries> xDataSeries;
    chart2::Symbol aSymblProp;

    // Check the symbol of data series 1 (marker style none)
    xDataSeries = getDataSeriesFromDoc(xChartDoc, 0);
    CPPUNIT_ASSERT(xDataSeries.is());
    uno::Reference<beans::XPropertySet> xPropSet_0(xDataSeries, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT((xPropSet_0->getPropertyValue(u"Symbol"_ustr) >>= aSymblProp));
    CPPUNIT_ASSERT_EQUAL(chart2::SymbolStyle_NONE, aSymblProp.Style);

    // Check the symbol of data series 2 (marker style square)
    xDataSeries = getDataSeriesFromDoc(xChartDoc, 1);
    CPPUNIT_ASSERT(xDataSeries.is());
    uno::Reference<beans::XPropertySet> xPropSet_1(xDataSeries, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT((xPropSet_1->getPropertyValue(u"Symbol"_ustr) >>= aSymblProp));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0xED7D31), aSymblProp.FillColor);

    // Check the symbol of data series 3 (marker style diagonal cross)
    xDataSeries = getDataSeriesFromDoc(xChartDoc, 2);
    CPPUNIT_ASSERT(xDataSeries.is());
    uno::Reference<beans::XPropertySet> xPropSet_2(xDataSeries, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT((xPropSet_2->getPropertyValue(u"Symbol"_ustr) >>= aSymblProp));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0xFF0000), aSymblProp.BorderColor);
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testTdf126033)
{
    loadFromFile(u"xlsx/tdf126033.xlsx");
    Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChartDoc.is());

    // Check symbol style and size of data points
    chart2::Symbol aSymblProp;
    uno::Reference<chart2::XDataSeries> xDataSeries(getDataSeriesFromDoc(xChartDoc, 0));
    CPPUNIT_ASSERT(xDataSeries.is());
    uno::Reference<beans::XPropertySet> xPropertySet(xDataSeries->getDataPointByIndex(0), uno::UNO_SET_THROW);
    CPPUNIT_ASSERT(xPropertySet->getPropertyValue(u"Symbol"_ustr) >>= aSymblProp);
    CPPUNIT_ASSERT_EQUAL(chart2::SymbolStyle_NONE, aSymblProp.Style);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(176), aSymblProp.Size.Width);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(176), aSymblProp.Size.Height);
}

void Chart2ImportTest::testTransparentBackground(std::u16string_view filename)
{
    loadFromFile(filename);
    uno::Reference< chart2::XChartDocument > xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChartDoc.is());

    uno::Reference< chart::XChartDocument > xChart2Doc (xChartDoc, uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChart2Doc.is());

    Reference< beans::XPropertySet > xPropSet = xChart2Doc->getArea();
    CPPUNIT_ASSERT_MESSAGE("failed to get Area", xPropSet.is());

    css::drawing::FillStyle aStyle;
    xPropSet -> getPropertyValue(u"FillStyle"_ustr) >>= aStyle;

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Background needs to be with solid fill style", css::drawing::FillStyle_SOLID, aStyle);
}

// 2 test methods here so that tearDown() can dispose the document
CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testFdo54361)
{
    testTransparentBackground(u"xlsx/fdo54361.xlsx");
}
CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testFdo54361_1)
{
    testTransparentBackground(u"xlsx/fdo54361-1.xlsx");
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testAutoBackgroundXLSX)
{
    loadFromFile(u"xlsx/chart-auto-background.xlsx");
    uno::Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChartDoc.is());

    // "Automatic" chart background fill in xlsx should be loaded as solid white.
    Reference<beans::XPropertySet> xPropSet = xChartDoc->getPageBackground();
    CPPUNIT_ASSERT(xPropSet.is());
    drawing::FillStyle eStyle = xPropSet->getPropertyValue(u"FillStyle"_ustr).get<drawing::FillStyle>();
    sal_Int32 nColor = xPropSet->getPropertyValue(u"FillColor"_ustr).get<sal_Int32>();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("'Automatic' chart background fill in xlsx should be loaded as solid fill.",
        drawing::FillStyle_SOLID, eStyle);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("'Automatic' chart background fill in xlsx should be loaded as solid white.",
        sal_Int32(0x00FFFFFF), sal_Int32(nColor & 0x00FFFFFF)); // highest 2 bytes are transparency which we ignore here.
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testAutoChartAreaBorderPropXLSX)
{
    loadFromFile(u"xlsx/chart-area-style-border.xlsx");
    uno::Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChartDoc.is());

    // Test "Automatic" chartarea border style/color/width.
    Reference<beans::XPropertySet> xPropSet = xChartDoc->getPageBackground();
    CPPUNIT_ASSERT(xPropSet.is());
    drawing::LineStyle eStyle = xPropSet->getPropertyValue(u"LineStyle"_ustr).get<drawing::LineStyle>();
    sal_Int32 nColor = xPropSet->getPropertyValue(u"LineColor"_ustr).get<sal_Int32>();
    sal_Int32 nWidth = xPropSet->getPropertyValue(u"LineWidth"_ustr).get<sal_Int32>();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("'Automatic' chartarea border should be loaded as solid style.",
        drawing::LineStyle_SOLID, eStyle);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("'Automatic' chartarea border color should be loaded as light gray.",
        sal_Int32(0xD9D9D9), nColor);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("'Automatic' chartarea border width should be loaded as 0.75 pt (~0.026 cm)",
        sal_Int32(26), nWidth);
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testAutoChartAreaBorderPropPPTX)
{
    loadFromFile(u"pptx/tdf150176.pptx");
    Reference<chart2::XChartDocument> xChartDoc(getChartDocFromDrawImpress(0, 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChartDoc.is());

    // Test "Automatic" chartarea border style/color/width.
    Reference<beans::XPropertySet> xPropSet = xChartDoc->getPageBackground();
    CPPUNIT_ASSERT(xPropSet.is());
    drawing::LineStyle eStyle = xPropSet->getPropertyValue(u"LineStyle"_ustr).get<drawing::LineStyle>();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("'Automatic' chartarea border should be loaded as none style for pptx.",
        drawing::LineStyle_NONE, eStyle);
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testChartAreaStyleBackgroundXLSX)
{
    loadFromFile(u"xlsx/chart-area-style-background.xlsx");
    uno::Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChartDoc.is());

    // "Automatic" chart background fill in xlsx should be loaded as solid white.
    Reference<beans::XPropertySet> xPropSet = xChartDoc->getPageBackground();
    CPPUNIT_ASSERT(xPropSet.is());
    drawing::FillStyle eStyle = xPropSet->getPropertyValue(u"FillStyle"_ustr).get<drawing::FillStyle>();
    sal_Int32 nColor = xPropSet->getPropertyValue(u"FillColor"_ustr).get<sal_Int32>();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("'Automatic' chart background fill in xlsx should be loaded as solid fill.",
        drawing::FillStyle_SOLID, eStyle);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("'Automatic' chart background fill in xlsx should be loaded as solid white.",
        sal_Int32(0), nColor);
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testChartHatchFillXLSX)
{
    loadFromFile(u"xlsx/chart-hatch-fill.xlsx");
    uno::Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChartDoc.is());

    // Check the chart background FillStyle is HATCH
    Reference<beans::XPropertySet> xPropSet = xChartDoc->getPageBackground();
    CPPUNIT_ASSERT(xPropSet.is());
    drawing::FillStyle eStyle = xPropSet->getPropertyValue(u"FillStyle"_ustr).get<drawing::FillStyle>();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Chart background fill in this xlsx should be loaded as hatch fill.",
        drawing::FillStyle_HATCH, eStyle);

    // Check the FillBackground of chart background
    bool bBackgroundFill = false;
    xPropSet->getPropertyValue(u"FillBackground"_ustr) >>= bBackgroundFill;
    CPPUNIT_ASSERT(bBackgroundFill);

    Color nBackgroundColor;
    xPropSet->getPropertyValue(u"FillColor"_ustr) >>= nBackgroundColor;
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, nBackgroundColor);

    // Check the datapoint has HatchName value
    uno::Reference<chart2::XDataSeries> xDataSeries(getDataSeriesFromDoc(xChartDoc, 0));
    CPPUNIT_ASSERT(xDataSeries.is());

    uno::Reference<beans::XPropertySet> xPropertySet(xDataSeries->getDataPointByIndex(1), uno::UNO_SET_THROW);
    OUString sHatchName;
    xPropertySet->getPropertyValue(u"HatchName"_ustr) >>= sHatchName;
    CPPUNIT_ASSERT(!sHatchName.isEmpty());

    // Check the FillBackground of datapoint
    bool bBackgroundFillofDatapoint = false;
    xPropertySet->getPropertyValue(u"FillBackground"_ustr) >>= bBackgroundFillofDatapoint;
    CPPUNIT_ASSERT(bBackgroundFillofDatapoint);

    sal_Int32 nBackgroundColorofDatapoint;
    xPropertySet->getPropertyValue(u"FillColor"_ustr) >>= nBackgroundColorofDatapoint;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0x00B050), nBackgroundColorofDatapoint);
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testAxisTextRotationXLSX)
{
    loadFromFile(u"xlsx/axis-label-rotation.xlsx");
    uno::Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChartDoc.is());

    Reference<chart2::XAxis> xYAxis = getAxisFromDoc(xChartDoc, 0, 0, 0);
    CPPUNIT_ASSERT(xYAxis.is());

    Reference<beans::XPropertySet> xPS(xYAxis, uno::UNO_QUERY_THROW);
    double nRotation = 0;
    bool bSuccess = xPS->getPropertyValue(u"TextRotation"_ustr) >>= nRotation;

    CPPUNIT_ASSERT(bSuccess);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(90, nRotation, 1e-10);
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testTextCanOverlapXLSX)
{
    // fdo#84647 : To check textoverlap value is imported correctly.
    loadFromFile(u"xlsx/chart-text-can-overlap.xlsx");
    uno::Reference< chart::XDiagram > mxDiagram;
    uno::Reference< beans::XPropertySet > xAxisProp;
    bool bTextCanOverlap = false;
    uno::Reference< chart::XChartDocument > xChartDoc ( getChartCompFromSheet( 0, 0, mxComponent ), UNO_QUERY_THROW);
    mxDiagram.set(xChartDoc->getDiagram());
    CPPUNIT_ASSERT(mxDiagram.is());
    uno::Reference< chart::XAxisXSupplier > xAxisXSupp( mxDiagram, uno::UNO_QUERY );
    CPPUNIT_ASSERT(xAxisXSupp.is());
    xAxisProp = xAxisXSupp->getXAxis();
    xAxisProp->getPropertyValue(u"TextCanOverlap"_ustr) >>= bTextCanOverlap;
    CPPUNIT_ASSERT(!bTextCanOverlap);
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testTextBreakXLSX)
{
    // tdf#122091: To check textbreak value is true in case of 0° degree of Axis label rotation.
    loadFromFile(u"xlsx/chart_label_text_break.xlsx");
    uno::Reference< chart::XDiagram > mxDiagram;
    uno::Reference< beans::XPropertySet > xAxisProp;
    bool textBreak = false;
    uno::Reference< chart::XChartDocument > xChartDoc ( getChartCompFromSheet( 0, 0, mxComponent ), UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xChartDoc.is());
    mxDiagram.set(xChartDoc->getDiagram());
    CPPUNIT_ASSERT(mxDiagram.is());
    uno::Reference< chart::XAxisXSupplier > xAxisXSupp( mxDiagram, uno::UNO_QUERY );
    CPPUNIT_ASSERT(xAxisXSupp.is());
    xAxisProp = xAxisXSupp->getXAxis();
    xAxisProp->getPropertyValue(u"TextBreak"_ustr) >>= textBreak;
    // Expected value of 'TextBreak' is true
    CPPUNIT_ASSERT(textBreak);
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testNumberFormatsXLSX)
{
    loadFromFile(u"xlsx/number-formats.xlsx");
    Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChartDoc.is());

    uno::Reference<chart2::XDataSeries> xDataSeries(getDataSeriesFromDoc(xChartDoc, 0));
    CPPUNIT_ASSERT(xDataSeries.is());
    uno::Reference<beans::XPropertySet> xPropertySet;
    chart2::DataPointLabel aLabel;
    sal_Int32 nNumberFormat;
    bool bLinkNumberFormatToSource = false;
    const sal_Int32 nChartDataNumberFormat = getNumberFormat(
            xChartDoc, u"_(\"$\"* #,##0_);_(\"$\"* \\(#,##0\\);_(\"$\"* \"-\"??_);_(@_)"_ustr);

    xPropertySet.set(xDataSeries->getDataPointByIndex(0), uno::UNO_SET_THROW);
    xPropertySet->getPropertyValue(u"Label"_ustr) >>= aLabel;
    CPPUNIT_ASSERT_EQUAL(sal_True, aLabel.ShowNumber);
    CPPUNIT_ASSERT_EQUAL(sal_True, aLabel.ShowNumberInPercent);
    xPropertySet->getPropertyValue(CHART_UNONAME_NUMFMT) >>= nNumberFormat;
    CPPUNIT_ASSERT_EQUAL(nChartDataNumberFormat, nNumberFormat);
    bool bSuccess = xPropertySet->getPropertyValue(u"PercentageNumberFormat"_ustr) >>= nNumberFormat;
    CPPUNIT_ASSERT_EQUAL(false, bSuccess);
    bSuccess = xPropertySet->getPropertyValue(CHART_UNONAME_LINK_TO_SRC_NUMFMT) >>= bLinkNumberFormatToSource;
    CPPUNIT_ASSERT_MESSAGE("\"LinkNumberFormatToSource\" should be set to true.", bSuccess);
    CPPUNIT_ASSERT_MESSAGE("\"LinkNumberFormatToSource\" should be set to true.", bLinkNumberFormatToSource);

    xPropertySet.set(xDataSeries->getDataPointByIndex(1), uno::UNO_SET_THROW);
    xPropertySet->getPropertyValue(u"Label"_ustr) >>= aLabel;
    CPPUNIT_ASSERT_EQUAL(sal_True, aLabel.ShowNumber);
    CPPUNIT_ASSERT_EQUAL(sal_False, aLabel.ShowNumberInPercent);
    xPropertySet->getPropertyValue(CHART_UNONAME_NUMFMT) >>= nNumberFormat;
    CPPUNIT_ASSERT_EQUAL(nChartDataNumberFormat, nNumberFormat);
    bSuccess = xPropertySet->getPropertyValue(u"PercentageNumberFormat"_ustr) >>= nNumberFormat;
    CPPUNIT_ASSERT_EQUAL(false, bSuccess);
    bSuccess = xPropertySet->getPropertyValue(CHART_UNONAME_LINK_TO_SRC_NUMFMT) >>= bLinkNumberFormatToSource;
    CPPUNIT_ASSERT_MESSAGE("\"LinkNumberFormatToSource\" should be set to true.", bSuccess);
    CPPUNIT_ASSERT_MESSAGE("\"LinkNumberFormatToSource\" should be set to true.", bLinkNumberFormatToSource);

    xPropertySet.set(xDataSeries->getDataPointByIndex(2), uno::UNO_SET_THROW);
    xPropertySet->getPropertyValue(u"Label"_ustr) >>= aLabel;
    CPPUNIT_ASSERT_EQUAL(sal_False, aLabel.ShowNumber);
    CPPUNIT_ASSERT_EQUAL(sal_True, aLabel.ShowNumberInPercent);
    xPropertySet->getPropertyValue(CHART_UNONAME_NUMFMT) >>= nNumberFormat;
    CPPUNIT_ASSERT_EQUAL(nChartDataNumberFormat, nNumberFormat);
    bSuccess = xPropertySet->getPropertyValue(u"PercentageNumberFormat"_ustr) >>= nNumberFormat;
    CPPUNIT_ASSERT_EQUAL(false, bSuccess);
    bSuccess = xPropertySet->getPropertyValue(CHART_UNONAME_LINK_TO_SRC_NUMFMT) >>= bLinkNumberFormatToSource;
    CPPUNIT_ASSERT_MESSAGE("\"LinkNumberFormatToSource\" should be set to true.", bSuccess);
    CPPUNIT_ASSERT_MESSAGE("\"LinkNumberFormatToSource\" should be set to true.", bLinkNumberFormatToSource);
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testNumberFormatsDOCX)
{
    loadFromFile(u"docx/tdf132174.docx");
    {
        uno::Reference< chart2::XChartDocument > xChartDoc(getChartDocFromWriter(0), uno::UNO_QUERY);
        CPPUNIT_ASSERT(xChartDoc.is());

        css::uno::Reference<chart2::XDiagram> xDiagram(xChartDoc->getFirstDiagram(), UNO_SET_THROW);
        Reference<chart2::XDataSeries> xDataSeries = getDataSeriesFromDoc(xChartDoc, 0);
        uno::Reference<beans::XPropertySet> xPropertySet(xDataSeries, uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT(xPropertySet.is());

        sal_Int32 nNumberFormat;
        bool bLinkNumberFormatToSource = true;
        const sal_Int32 nChartDataNumberFormat = getNumberFormat(xChartDoc, u"0%"_ustr);
        xPropertySet->getPropertyValue(CHART_UNONAME_NUMFMT) >>= nNumberFormat;
        CPPUNIT_ASSERT_EQUAL(nChartDataNumberFormat, nNumberFormat);
        xPropertySet->getPropertyValue(CHART_UNONAME_LINK_TO_SRC_NUMFMT) >>= bLinkNumberFormatToSource;
        // LinkNumberFormatToSource should be set to false even if the original OOXML contain a true value,
        // because the inner data table of charts have no own number format!
        CPPUNIT_ASSERT_MESSAGE("\"LinkNumberFormatToSource\" should be set to false.", !bLinkNumberFormatToSource);
    }

    loadFromFile(u"docx/tdf136650.docx");
    {
        uno::Reference< chart2::XChartDocument > xChartDoc(getChartDocFromWriter(0), uno::UNO_QUERY);
        CPPUNIT_ASSERT(xChartDoc.is());

        css::uno::Reference<chart2::XDiagram> xDiagram(xChartDoc->getFirstDiagram(), UNO_SET_THROW);
        Reference<chart2::XDataSeries> xDataSeries = getDataSeriesFromDoc(xChartDoc, 0);
        CPPUNIT_ASSERT(xDataSeries.is());
        Reference<beans::XPropertySet> xPropertySet(xDataSeries->getDataPointByIndex(1), uno::UNO_SET_THROW);

        sal_Int32 nNumberFormat;
        bool bLinkNumberFormatToSource = true;
        const sal_Int32 nChartDataNumberFormat = getNumberFormat(xChartDoc, u"0%"_ustr);
        xPropertySet->getPropertyValue(CHART_UNONAME_NUMFMT) >>= nNumberFormat;
        CPPUNIT_ASSERT_EQUAL(nChartDataNumberFormat, nNumberFormat);
        xPropertySet->getPropertyValue(CHART_UNONAME_LINK_TO_SRC_NUMFMT) >>= bLinkNumberFormatToSource;
        // LinkNumberFormatToSource should be set to false even if the original OOXML file contain a true value,
        // because the inner data table of charts have no own number format!
        CPPUNIT_ASSERT_MESSAGE("\"LinkNumberFormatToSource\" should be set to false.", !bLinkNumberFormatToSource);
    }
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testPercentageNumberFormatsDOCX)
{
    loadFromFile(u"docx/tdf133632.docx");
    uno::Reference< chart2::XChartDocument > xChartDoc(getChartDocFromWriter(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());

    css::uno::Reference<chart2::XDiagram> xDiagram(xChartDoc->getFirstDiagram(), UNO_SET_THROW);
    Reference<chart2::XDataSeries> xDataSeries = getDataSeriesFromDoc(xChartDoc, 0);
    uno::Reference<beans::XPropertySet> xPropertySet(xDataSeries, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xPropertySet.is());

    bool bLinkNumberFormatToSource = false;
    chart2::DataPointLabel aLabel;
    xPropertySet->getPropertyValue(u"Label"_ustr) >>= aLabel;
    CPPUNIT_ASSERT_EQUAL(sal_False, aLabel.ShowNumber);
    CPPUNIT_ASSERT_EQUAL(sal_True, aLabel.ShowNumberInPercent);
    bool bSuccess = xPropertySet->getPropertyValue(CHART_UNONAME_LINK_TO_SRC_NUMFMT) >>= bLinkNumberFormatToSource;
    CPPUNIT_ASSERT_MESSAGE("\"LinkNumberFormatToSource\" should be set to true.", bSuccess);
    CPPUNIT_ASSERT_MESSAGE("\"LinkNumberFormatToSource\" should be set to true.", bLinkNumberFormatToSource);
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testAutoTitleDelDefaultValue2007XLSX)
{
    // below are OOXML default value tests for cases
    // where we fixed the handling of MSO 2007 vs OOXML
    loadFromFile(u"xlsx/autotitledel_2007.xlsx");
    Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChartDoc.is());

    Reference<chart2::XTitled> xTitled(xChartDoc, uno::UNO_QUERY_THROW);
    OUString aTitle = getTitleString(xTitled);
    CPPUNIT_ASSERT_MESSAGE("autoTitleDel default value is false in MSO 2007 documents",
            !aTitle.isEmpty());
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testAutoTitleDelDefaultValue2013XLSX)
{
    loadFromFile(u"xlsx/autotitledel_2013.xlsx");
    Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChartDoc.is());

    Reference<chart2::XTitled> xTitled(xChartDoc, uno::UNO_QUERY_THROW);
    uno::Reference<chart2::XTitle> xTitle = xTitled->getTitleObject();
    CPPUNIT_ASSERT_MESSAGE("autoTitleDel default value is true in the OOXML spec",
            !xTitle.is());
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testDispBlanksAsDefaultValue2007XLSX)
{
    loadFromFile(u"xlsx/dispBlanksAs_2007.xlsx");
    Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChartDoc.is());

    Reference<beans::XPropertySet> xDiagram(xChartDoc->getFirstDiagram(), UNO_QUERY);
    CPPUNIT_ASSERT(xDiagram.is());
    uno::Any aAny = xDiagram->getPropertyValue(u"MissingValueTreatment"_ustr);
    sal_Int32 nMissingValueTreatment = -2;
    CPPUNIT_ASSERT(aAny >>= nMissingValueTreatment);
    CPPUNIT_ASSERT_EQUAL(chart::MissingValueTreatment::LEAVE_GAP, nMissingValueTreatment);
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testDispBlanksAsDefaultValue2013XLSX)
{
    loadFromFile(u"xlsx/dispBlanksAs_2013.xlsx");
    Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChartDoc.is());

    Reference<beans::XPropertySet> xDiagram(xChartDoc->getFirstDiagram(), UNO_QUERY);
    CPPUNIT_ASSERT(xDiagram.is());
    uno::Any aAny = xDiagram->getPropertyValue(u"MissingValueTreatment"_ustr);
    sal_Int32 nMissingValueTreatment = -2;
    CPPUNIT_ASSERT(aAny >>= nMissingValueTreatment);
    CPPUNIT_ASSERT_EQUAL(chart::MissingValueTreatment::USE_ZERO, nMissingValueTreatment);
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testSmoothDefaultValue2007XLSX)
{
    loadFromFile(u"xlsx/smoothed_series2007.xlsx");
    Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChartDoc.is());
    CPPUNIT_ASSERT(xChartDoc.is());

    Reference< chart2::XChartType > xChartType = getChartTypeFromDoc( xChartDoc, 0 );
    CPPUNIT_ASSERT(xChartType.is());

    Reference< beans::XPropertySet > xPropSet( xChartType, UNO_QUERY );
    CPPUNIT_ASSERT(xPropSet.is());

    chart2::CurveStyle eCurveStyle;
    xPropSet->getPropertyValue(u"CurveStyle"_ustr) >>= eCurveStyle;
    CPPUNIT_ASSERT_EQUAL(chart2::CurveStyle_LINES, eCurveStyle);
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testSmoothDefaultValue2013XLSX)
{
    loadFromFile(u"xlsx/smoothed_series.xlsx");
    Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChartDoc.is());
    CPPUNIT_ASSERT(xChartDoc.is());

    Reference< chart2::XChartType > xChartType = getChartTypeFromDoc( xChartDoc, 0 );
    CPPUNIT_ASSERT(xChartType.is());

    Reference< beans::XPropertySet > xPropSet( xChartType, UNO_QUERY );
    CPPUNIT_ASSERT(xPropSet.is());

    chart2::CurveStyle eCurveStyle;
    xPropSet->getPropertyValue(u"CurveStyle"_ustr) >>= eCurveStyle;
    CPPUNIT_ASSERT(eCurveStyle != chart2::CurveStyle_LINES);
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testTrendlineDefaultValue2007XLSX)
{
    loadFromFile(u"xlsx/trendline2007.xlsx");
    Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChartDoc.is());
    CPPUNIT_ASSERT(xChartDoc.is());
    Reference<chart2::XDataSeries> xDataSeries = getDataSeriesFromDoc(xChartDoc, 0);
    CPPUNIT_ASSERT(xDataSeries.is());
    Reference<chart2::XRegressionCurveContainer> xRegressionCurveContainer(xDataSeries, UNO_QUERY_THROW);
    Sequence< Reference<chart2::XRegressionCurve> > xRegressionCurveSequence = xRegressionCurveContainer->getRegressionCurves();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xRegressionCurveSequence.getLength());

    Reference<chart2::XRegressionCurve> xCurve = xRegressionCurveSequence[0];

    Reference<beans::XPropertySet> xPropSet(xCurve->getEquationProperties(), uno::UNO_SET_THROW);
    uno::Any aAny = xPropSet->getPropertyValue(u"ShowEquation"_ustr);
    bool bShowEquation = true;
    CPPUNIT_ASSERT(aAny >>= bShowEquation);
    CPPUNIT_ASSERT(!bShowEquation);

    aAny = xPropSet->getPropertyValue(u"ShowCorrelationCoefficient"_ustr);
    bool bShowCorrelation = true;
    CPPUNIT_ASSERT(aAny >>= bShowCorrelation);
    CPPUNIT_ASSERT(!bShowCorrelation);
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testTrendlineDefaultValue2013XLSX)
{
    loadFromFile(u"xlsx/trendline.xlsx");
    Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChartDoc.is());
    CPPUNIT_ASSERT(xChartDoc.is());
    Reference<chart2::XDataSeries> xDataSeries = getDataSeriesFromDoc(xChartDoc, 0);
    CPPUNIT_ASSERT(xDataSeries.is());
    Reference<chart2::XRegressionCurveContainer> xRegressionCurveContainer(xDataSeries, UNO_QUERY_THROW);
    Sequence< Reference<chart2::XRegressionCurve> > xRegressionCurveSequence = xRegressionCurveContainer->getRegressionCurves();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xRegressionCurveSequence.getLength());

    Reference<chart2::XRegressionCurve> xCurve = xRegressionCurveSequence[0];

    Reference<beans::XPropertySet> xPropSet(xCurve->getEquationProperties(), uno::UNO_SET_THROW);
    uno::Any aAny = xPropSet->getPropertyValue(u"ShowEquation"_ustr);
    bool bShowEquation = false;
    CPPUNIT_ASSERT(aAny >>= bShowEquation);
    CPPUNIT_ASSERT(bShowEquation);

    aAny = xPropSet->getPropertyValue(u"ShowCorrelationCoefficient"_ustr);
    bool bShowCorrelation = false;
    CPPUNIT_ASSERT(aAny >>= bShowCorrelation);
    CPPUNIT_ASSERT(bShowCorrelation);
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testVaryColorDefaultValues2007XLSX)
{
    loadFromFile(u"xlsx/vary_color2007.xlsx");
    Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChartDoc.is());
    CPPUNIT_ASSERT(xChartDoc.is());
    Reference<chart2::XDataSeries> xDataSeries = getDataSeriesFromDoc(xChartDoc, 0);
    CPPUNIT_ASSERT(xDataSeries.is());
    Reference<beans::XPropertySet> xPropSet(xDataSeries, uno::UNO_QUERY_THROW);
    uno::Any aAny = xPropSet->getPropertyValue(u"VaryColorsByPoint"_ustr);
    bool bVaryColor = true;
    CPPUNIT_ASSERT(aAny >>= bVaryColor);
    CPPUNIT_ASSERT(!bVaryColor);
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testVaryColorDefaultValues2013XLSX)
{
    loadFromFile(u"xlsx/vary_color.xlsx");
    Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChartDoc.is());
    CPPUNIT_ASSERT(xChartDoc.is());
    Reference<chart2::XDataSeries> xDataSeries = getDataSeriesFromDoc(xChartDoc, 0);
    CPPUNIT_ASSERT(xDataSeries.is());
    Reference<beans::XPropertySet> xPropSet(xDataSeries, uno::UNO_QUERY_THROW);
    uno::Any aAny = xPropSet->getPropertyValue(u"VaryColorsByPoint"_ustr);
    bool bVaryColor = false;
    CPPUNIT_ASSERT(aAny >>= bVaryColor);
    CPPUNIT_ASSERT(!bVaryColor);
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testPlotVisOnlyDefaultValue2013XLSX)
{
    loadFromFile(u"xlsx/plotVisOnly.xlsx");
    uno::Reference< chart::XChartDocument > xChart1Doc ( getChartCompFromSheet( 0, 0, mxComponent ), UNO_QUERY_THROW);
    Reference<beans::XPropertySet> xPropSet(xChart1Doc->getDiagram(), uno::UNO_QUERY_THROW);
    uno::Any aAny = xPropSet->getPropertyValue(u"IncludeHiddenCells"_ustr);
    CPPUNIT_ASSERT(aAny.hasValue());
    bool bShowHiddenValues = true;
    CPPUNIT_ASSERT(aAny >>= bShowHiddenValues);
    CPPUNIT_ASSERT(!bShowHiddenValues);
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testRAngAxDefaultValue2013XLSX)
{
    loadFromFile(u"xlsx/rAngAx.xlsx");
    uno::Reference< chart::XChartDocument > xChart1Doc ( getChartCompFromSheet( 0, 0, mxComponent ), UNO_QUERY_THROW);
    Reference<beans::XPropertySet> xPropSet(xChart1Doc->getDiagram(), uno::UNO_QUERY_THROW);
    uno::Any aAny = xPropSet->getPropertyValue(u"RightAngledAxes"_ustr);
    CPPUNIT_ASSERT(aAny.hasValue());
    bool bRightAngleAxes = false;
    CPPUNIT_ASSERT(aAny >>= bRightAngleAxes);
    CPPUNIT_ASSERT(bRightAngleAxes);
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testMajorTickMarksDefaultValue2013XLSX)
{
    loadFromFile(u"xlsx/majorTickMark.xlsx");
    Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChartDoc.is());
    Reference<chart2::XAxis> xXAxis = getAxisFromDoc(xChartDoc, 0, 0, 0);
    CPPUNIT_ASSERT(xXAxis.is());
    Reference<beans::XPropertySet> xPropSet(xXAxis, uno::UNO_QUERY_THROW);
    uno::Any aAny = xPropSet->getPropertyValue(u"MajorTickmarks"_ustr);
    sal_Int32 nMajorTickmarks = chart2::TickmarkStyle::NONE;
    CPPUNIT_ASSERT(aAny.hasValue());
    CPPUNIT_ASSERT(aAny >>= nMajorTickmarks);
    CPPUNIT_ASSERT_EQUAL(chart2::TickmarkStyle::INNER | chart2::TickmarkStyle::OUTER, nMajorTickmarks);
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testMinorTickMarksDefaultValue2013XLSX)
{
    loadFromFile(u"xlsx/minorTickMark.xlsx");
    Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChartDoc.is());
    Reference<chart2::XAxis> xXAxis = getAxisFromDoc(xChartDoc, 0, 0, 0);
    CPPUNIT_ASSERT(xXAxis.is());
    Reference<beans::XPropertySet> xPropSet(xXAxis, uno::UNO_QUERY_THROW);
    uno::Any aAny = xPropSet->getPropertyValue(u"MinorTickmarks"_ustr);
    sal_Int32 nMajorTickmarks = chart2::TickmarkStyle::NONE;
    CPPUNIT_ASSERT(aAny.hasValue());
    CPPUNIT_ASSERT(aAny >>= nMajorTickmarks);
    CPPUNIT_ASSERT_EQUAL(chart2::TickmarkStyle::INNER | chart2::TickmarkStyle::OUTER, nMajorTickmarks);
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testAxisTitleDefaultRotationXLSX)
{
    loadFromFile(u"xlsx/axis_title_default_rotation.xlsx");
    Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChartDoc.is());
    Reference<chart2::XAxis> xYAxis = getAxisFromDoc(xChartDoc, 0, 1, 0);
    CPPUNIT_ASSERT(xYAxis.is());
    Reference<chart2::XTitled> xTitled(xYAxis, uno::UNO_QUERY_THROW);
    Reference<chart2::XTitle> xTitle = xTitled->getTitleObject();
    CPPUNIT_ASSERT(xTitle.is());
    Reference<beans::XPropertySet> xPropSet(xTitle, uno::UNO_QUERY_THROW);
    uno::Any aAny = xPropSet->getPropertyValue(u"TextRotation"_ustr);
    double nRotation = 0;
    CPPUNIT_ASSERT(aAny >>= nRotation);
    CPPUNIT_ASSERT_EQUAL(90.0, nRotation);
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testSecondaryAxisTitleDefaultRotationXLSX)
{
    loadFromFile(u"xlsx/secondary_axis_title_default_rotation.xlsx");
    Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChartDoc.is());
    Reference<chart2::XAxis> xYAxis = getAxisFromDoc(xChartDoc, 0, 1, 1);
    CPPUNIT_ASSERT(xYAxis.is());
    Reference<chart2::XTitled> xTitled(xYAxis, uno::UNO_QUERY_THROW);
    Reference<chart2::XTitle> xTitle = xTitled->getTitleObject();
    CPPUNIT_ASSERT(xTitle.is());
    Reference<beans::XPropertySet> xPropSet(xTitle, uno::UNO_QUERY_THROW);
    uno::Any aAny = xPropSet->getPropertyValue(u"TextRotation"_ustr);
    double nRotation = 0;
    CPPUNIT_ASSERT(aAny >>= nRotation);
    CPPUNIT_ASSERT_EQUAL(90.0, nRotation);
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testAxisTitleRotationXLSX)
{
    loadFromFile(u"xlsx/axis_title_rotated.xlsx");
    Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChartDoc.is());
    {
        Reference<chart2::XAxis> xYAxis = getAxisFromDoc(xChartDoc, 0, 1, 0);
        CPPUNIT_ASSERT(xYAxis.is());
        Reference<chart2::XTitled> xTitled(xYAxis, uno::UNO_QUERY_THROW);
        Reference<chart2::XTitle> xTitle = xTitled->getTitleObject();
        CPPUNIT_ASSERT(xTitle.is());
        Reference<beans::XPropertySet> xPropSet(xTitle, uno::UNO_QUERY_THROW);
        uno::Any aAny = xPropSet->getPropertyValue(u"TextRotation"_ustr);
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
        uno::Any aAny = xPropSet->getPropertyValue(u"TextRotation"_ustr);
        double nRotation = 0;
        CPPUNIT_ASSERT(aAny >>= nRotation);
        CPPUNIT_ASSERT_EQUAL(270.0, nRotation);
    }

}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testAxisTitlePositionDOCX)
{
    loadFromFile(u"docx/testAxisTitlePosition.docx");
    uno::Reference< chart::XDiagram > mxDiagram;
    uno::Reference< drawing::XShape > xAxisTitle;
    uno::Reference< chart::XChartDocument > xChartDoc = getChartDocFromWriter(0);
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChartDoc.is());
    mxDiagram.set(xChartDoc->getDiagram());
    CPPUNIT_ASSERT(mxDiagram.is());
    // test X Axis title position
    uno::Reference< chart::XAxisXSupplier > xAxisXSupp(mxDiagram, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xAxisXSupp.is());

    xAxisTitle = xAxisXSupp->getXAxisTitle();
    CPPUNIT_ASSERT(xAxisTitle.is());

    awt::Point aPos = xAxisTitle->getPosition();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(10640, aPos.X, 2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(7157, aPos.Y, 2);

    // test Y Axis title position
    uno::Reference< chart::XAxisYSupplier > xAxisYSupp(mxDiagram, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xAxisYSupp.is());

    xAxisTitle = xAxisYSupp->getYAxisTitle();
    CPPUNIT_ASSERT(xAxisTitle.is());

    aPos = xAxisTitle->getPosition();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(387, aPos.X, 2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6378, aPos.Y, 300);
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testCombinedChartAttachedAxisXLSX)
{
    loadFromFile(u"xlsx/testCombinedChartAxis.xlsx");
    Reference< chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    // First series
    Reference<chart2::XDataSeries> xSeries = getDataSeriesFromDoc(xChartDoc, 0);
    CPPUNIT_ASSERT(xSeries.is());

    Reference<beans::XPropertySet> xPropSet(xSeries, uno::UNO_QUERY_THROW);
    sal_Int32 nAxisIndex = -1;
    // First series (column chart) should be attached to secondary axis!
    uno::Any aAny = xPropSet->getPropertyValue(u"AttachedAxisIndex"_ustr);
    CPPUNIT_ASSERT(aAny >>= nAxisIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), nAxisIndex);

    // Second series
    xSeries = getDataSeriesFromDoc(xChartDoc, 0, 1);
    CPPUNIT_ASSERT(xSeries.is());

    xPropSet.set(xSeries, uno::UNO_QUERY_THROW);
    // Second series (line chart) should be attached to primary axis!
    aAny = xPropSet->getPropertyValue(u"AttachedAxisIndex"_ustr);
    CPPUNIT_ASSERT(aAny >>= nAxisIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nAxisIndex);
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testTdf140489MultiSeriesChartAxisXLSX)
{
    loadFromFile(u"xlsx/tdf140489.xlsx");
    Reference< chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    // First series
    Reference<chart2::XDataSeries> xSeries = getDataSeriesFromDoc(xChartDoc, 0);
    CPPUNIT_ASSERT(xSeries.is());

    Reference<beans::XPropertySet> xPropSet(xSeries, uno::UNO_QUERY_THROW);
    sal_Int32 nAxisIndex = -1;
    uno::Any aAny = xPropSet->getPropertyValue(u"AttachedAxisIndex"_ustr);
    CPPUNIT_ASSERT(aAny >>= nAxisIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nAxisIndex);

    // Second series
    xSeries = getDataSeriesFromDoc(xChartDoc, 0, 1);
    CPPUNIT_ASSERT(xSeries.is());

    xPropSet.set(xSeries, uno::UNO_QUERY_THROW);
    aAny = xPropSet->getPropertyValue(u"AttachedAxisIndex"_ustr);
    CPPUNIT_ASSERT(aAny >>= nAxisIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nAxisIndex);

    // Third series
    xSeries = getDataSeriesFromDoc(xChartDoc, 0, 2);
    CPPUNIT_ASSERT(xSeries.is());

    xPropSet.set(xSeries, uno::UNO_QUERY_THROW);
    aAny = xPropSet->getPropertyValue(u"AttachedAxisIndex"_ustr);
    CPPUNIT_ASSERT(aAny >>= nAxisIndex);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), nAxisIndex);
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testInternalDataProvider)
{
    loadFromFile(u"odp/chart.odp");
    uno::Reference< chart2::XChartDocument > xChartDoc(getChartDocFromDrawImpress(0,0), uno::UNO_QUERY_THROW);
    const uno::Reference< chart2::data::XDataProvider >& rxDataProvider = xChartDoc->getDataProvider();

    // Parse 42 array
    Reference<chart2::data::XDataSequence> xDataSeq = rxDataProvider->createDataSequenceByValueArray(u"values-y"_ustr, u"{42;42;42;42}"_ustr, u""_ustr);
    Sequence<Any> xSequence = xDataSeq->getData();
    CPPUNIT_ASSERT_EQUAL(uno::Any(sal_Int32(42)), xSequence[0]);
    CPPUNIT_ASSERT_EQUAL(uno::Any(sal_Int32(42)), xSequence[1]);
    CPPUNIT_ASSERT_EQUAL(uno::Any(sal_Int32(42)), xSequence[2]);
    CPPUNIT_ASSERT_EQUAL(uno::Any(sal_Int32(42)), xSequence[3]);

    // Parse empty first and last
    xDataSeq = rxDataProvider->createDataSequenceByValueArray(u"values-y"_ustr, u"{\"\";42;42;\"\"}"_ustr, u""_ustr);
    xSequence = xDataSeq->getData();
    CPPUNIT_ASSERT( std::isnan( *static_cast<const double*>(xSequence[0].getValue())));
    CPPUNIT_ASSERT_EQUAL(uno::Any(sal_Int32(42)), xSequence[1]);
    CPPUNIT_ASSERT_EQUAL(uno::Any(sal_Int32(42)), xSequence[2]);
    CPPUNIT_ASSERT( std::isnan( *static_cast<const double*>(xSequence[3].getValue())));

    // Parse empty middle
    xDataSeq = rxDataProvider->createDataSequenceByValueArray(u"values-y"_ustr, u"{42;\"\";\"\";42}"_ustr, u""_ustr);
    xSequence = xDataSeq->getData();
    CPPUNIT_ASSERT_EQUAL(uno::Any(sal_Int32(42)), xSequence[0]);
    CPPUNIT_ASSERT( std::isnan( *static_cast<const double*>(xSequence[1].getValue())) );
    CPPUNIT_ASSERT( std::isnan( *static_cast<const double*>(xSequence[2].getValue())) );
    CPPUNIT_ASSERT_EQUAL(uno::Any(sal_Int32(42)), xSequence[3]);

    // Parse mixed types, numeric only role
    xDataSeq = rxDataProvider->createDataSequenceByValueArray(u"values-y"_ustr, u"{42;\"hello\";0;\"world\"}"_ustr, u""_ustr);
    xSequence = xDataSeq->getData();
    CPPUNIT_ASSERT_EQUAL(uno::Any(sal_Int32(42)), xSequence[0]);
    CPPUNIT_ASSERT_EQUAL(uno::Any(sal_Int32(0)),  xSequence[1]);
    CPPUNIT_ASSERT_EQUAL(uno::Any(sal_Int32(0)),  xSequence[2]);
    CPPUNIT_ASSERT_EQUAL(uno::Any(sal_Int32(0)),  xSequence[3]);

    // Parse mixed types, mixed role
    xDataSeq = rxDataProvider->createDataSequenceByValueArray(u"categories"_ustr, u"{42;\"hello\";0;\"world\"}"_ustr, u""_ustr);
    xSequence = xDataSeq->getData();
    CPPUNIT_ASSERT_EQUAL(uno::Any(u"Row 1 42"_ustr), xSequence[0]);
    CPPUNIT_ASSERT_EQUAL(uno::Any(u"Row 2 hello"_ustr), xSequence[1]);
    CPPUNIT_ASSERT_EQUAL(uno::Any(u"Row 3 0"_ustr), xSequence[2]);
    CPPUNIT_ASSERT_EQUAL(uno::Any(u"Row 4 world"_ustr), xSequence[3]);
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testTdf90510)
{
    // Pie chart label placement settings(XLS)
    loadFromFile(u"xls/piechart_outside.xls");
    uno::Reference< chart::XChartDocument > xChart1Doc( getChartCompFromSheet( 0, 0, mxComponent ), UNO_QUERY_THROW );
    Reference<beans::XPropertySet> xPropSet( xChart1Doc->getDiagram()->getDataPointProperties( 0, 0 ), uno::UNO_SET_THROW );
    uno::Any aAny = xPropSet->getPropertyValue( u"LabelPlacement"_ustr );
    CPPUNIT_ASSERT( aAny.hasValue() );
    sal_Int32 nLabelPlacement = 0;
    CPPUNIT_ASSERT( aAny >>= nLabelPlacement );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Data labels should be placed outside", chart::DataLabelPlacement::OUTSIDE, nLabelPlacement );
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testTdf109858)
{
    // Pie chart label placement settings(XLSX)
    loadFromFile(u"xlsx/piechart_outside.xlsx");
    uno::Reference< chart::XChartDocument > xChart1Doc( getChartCompFromSheet( 0, 0, mxComponent ), UNO_QUERY_THROW );

    // test data point labels position
    Reference<beans::XPropertySet> xDataPointPropSet( xChart1Doc->getDiagram()->getDataPointProperties( 0, 0 ), uno::UNO_SET_THROW );
    uno::Any aAny = xDataPointPropSet->getPropertyValue( u"LabelPlacement"_ustr );
    CPPUNIT_ASSERT( aAny.hasValue() );
    sal_Int32 nLabelPlacement = 0;
    CPPUNIT_ASSERT( aAny >>= nLabelPlacement );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Data point label should be placed bestfit", chart::DataLabelPlacement::CUSTOM, nLabelPlacement );

    // test data series label position
    Reference<beans::XPropertySet> xSeriesPropSet(xChart1Doc->getDiagram()->getDataRowProperties(0), uno::UNO_SET_THROW);
    aAny = xSeriesPropSet->getPropertyValue( u"LabelPlacement"_ustr );
    CPPUNIT_ASSERT( aAny >>= nLabelPlacement );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Data series labels should be placed outside", chart::DataLabelPlacement::OUTSIDE, nLabelPlacement );
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testTdf130105)
{
    loadFromFile(u"xlsx/barchart_outend.xlsx");
    uno::Reference< chart2::XChartDocument > xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT(xChartDoc.is());
    uno::Reference<chart2::XDataSeries> xDataSeries(getDataSeriesFromDoc(xChartDoc, 0));
    CPPUNIT_ASSERT(xDataSeries.is());

    uno::Reference<beans::XPropertySet> xPropertySet(xDataSeries->getDataPointByIndex(0), uno::UNO_SET_THROW);
    uno::Any aAny = xPropertySet->getPropertyValue(u"LabelPlacement"_ustr);
    CPPUNIT_ASSERT(aAny.hasValue());
    sal_Int32 nLabelPlacement = 0;
    CPPUNIT_ASSERT(aAny >>= nLabelPlacement);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Data label should be placed outend", chart::DataLabelPlacement::OUTSIDE, nLabelPlacement);
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testTdf111173)
{
    loadFromFile(u"xlsx/tdf111173.xlsx");
    uno::Reference< chart::XChartDocument > xChart1Doc( getChartCompFromSheet( 0, 0, mxComponent ), UNO_QUERY_THROW );
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testTdf122226)
{
    loadFromFile(u"docx/testTdf122226.docx" );
    uno::Reference< chart2::XChartDocument > xChartDoc ( getChartDocFromWriter(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT( xChartDoc.is() );

    css::uno::Reference<chart2::XDiagram> xDiagram(xChartDoc->getFirstDiagram(), UNO_SET_THROW);
    Reference<chart2::XDataSeries> xDataSeries = getDataSeriesFromDoc(xChartDoc, 0);
    uno::Reference<beans::XPropertySet> xPropertySet(xDataSeries->getDataPointByIndex(0), uno::UNO_SET_THROW);
    CPPUNIT_ASSERT(xPropertySet.is());

    uno::Any aAny = xPropertySet->getPropertyValue( u"LabelSeparator"_ustr );
    CPPUNIT_ASSERT( aAny.hasValue() );
    OUString nLabelSeparator;
    CPPUNIT_ASSERT( aAny >>= nLabelSeparator );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Data labels should be separated into new lines", u"\n"_ustr, nLabelSeparator );
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testTdf115107)
{
    // import complex data point labels
    loadFromFile(u"pptx/tdf115107.pptx");

    Reference<chart2::XChartDocument> xChartDoc(getChartDocFromDrawImpress(0, 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());

    uno::Reference<chart2::XDataSeries> xDataSeries(getDataSeriesFromDoc(xChartDoc, 0));
    CPPUNIT_ASSERT(xDataSeries.is());
    float nFontSize;
    sal_Int64 nFontColor;
    sal_Int32 nCharUnderline;
    uno::Reference<beans::XPropertySet> xPropertySet;
    uno::Sequence<uno::Reference<chart2::XDataPointCustomLabelField>> aFields;

    // 1
    xPropertySet.set(xDataSeries->getDataPointByIndex(0), uno::UNO_SET_THROW);
    xPropertySet->getPropertyValue(u"CustomLabelFields"_ustr) >>= aFields;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), aFields.getLength());

    CPPUNIT_ASSERT_EQUAL(chart2::DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_TEXT, aFields[0]->getFieldType());
    CPPUNIT_ASSERT_EQUAL(u"90.0 = "_ustr, aFields[0]->getString());
    aFields[0]->getPropertyValue(u"CharHeight"_ustr) >>= nFontSize;
    aFields[0]->getPropertyValue(u"CharColor"_ustr) >>= nFontColor;
    CPPUNIT_ASSERT_EQUAL(static_cast<float>(18), nFontSize);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int64>(0xed7d31), nFontColor);

    CPPUNIT_ASSERT_EQUAL(chart2::DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_VALUE, aFields[1]->getFieldType());
    CPPUNIT_ASSERT_EQUAL(u"90"_ustr, aFields[1]->getString());

    // 2
    xPropertySet.set(xDataSeries->getDataPointByIndex(1), uno::UNO_SET_THROW);
    xPropertySet->getPropertyValue(u"CustomLabelFields"_ustr) >>= aFields;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(8), aFields.getLength());

    CPPUNIT_ASSERT_EQUAL(chart2::DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_TEXT, aFields[0]->getFieldType());
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr, aFields[0]->getString());

    CPPUNIT_ASSERT_EQUAL(chart2::DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_TEXT, aFields[1]->getFieldType());
    CPPUNIT_ASSERT_EQUAL(u" : "_ustr, aFields[1]->getString());

    CPPUNIT_ASSERT_EQUAL(chart2::DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_CATEGORYNAME, aFields[2]->getFieldType());
    CPPUNIT_ASSERT_EQUAL(u"B"_ustr, aFields[2]->getString());
    aFields[2]->getPropertyValue(u"CharHeight"_ustr) >>= nFontSize;
    aFields[2]->getPropertyValue(u"CharColor"_ustr) >>= nFontColor;
    CPPUNIT_ASSERT_EQUAL(static_cast<float>(16), nFontSize);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int64>(0xed7d31), nFontColor);

    CPPUNIT_ASSERT_EQUAL(chart2::DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_NEWLINE, aFields[3]->getFieldType());

    CPPUNIT_ASSERT_EQUAL(chart2::DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_TEXT, aFields[4]->getFieldType());
    CPPUNIT_ASSERT_EQUAL(u"Multi"_ustr, aFields[4]->getString());

    CPPUNIT_ASSERT_EQUAL(chart2::DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_TEXT, aFields[5]->getFieldType());
    CPPUNIT_ASSERT_EQUAL(u"line"_ustr, aFields[5]->getString());
    aFields[5]->getPropertyValue(u"CharHeight"_ustr) >>= nFontSize;
    aFields[5]->getPropertyValue(u"CharColor"_ustr) >>= nFontColor;
    CPPUNIT_ASSERT_EQUAL(static_cast<float>(11.97), nFontSize);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int64>(0xbf9000), nFontColor);

    CPPUNIT_ASSERT_EQUAL(chart2::DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_NEWLINE, aFields[6]->getFieldType());

    CPPUNIT_ASSERT_EQUAL(chart2::DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_TEXT, aFields[7]->getFieldType());
    CPPUNIT_ASSERT_EQUAL(u"Abc"_ustr, aFields[7]->getString());
    aFields[7]->getPropertyValue(u"CharHeight"_ustr) >>= nFontSize;
    aFields[7]->getPropertyValue(u"CharColor"_ustr) >>= nFontColor;
    aFields[7]->getPropertyValue(u"CharUnderline"_ustr) >>= nCharUnderline;
    CPPUNIT_ASSERT_EQUAL(static_cast<float>(12), nFontSize);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int64>(0xa9d18e), nFontColor);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), nCharUnderline);

    // 3
    xPropertySet.set(xDataSeries->getDataPointByIndex(2), uno::UNO_SET_THROW);
    xPropertySet->getPropertyValue(u"CustomLabelFields"_ustr) >>= aFields;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), aFields.getLength());

    CPPUNIT_ASSERT_EQUAL(chart2::DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_SERIESNAME, aFields[0]->getFieldType());
    CPPUNIT_ASSERT_EQUAL(u"DATA"_ustr, aFields[0]->getString());

    // 4
    xPropertySet.set(xDataSeries->getDataPointByIndex(3), uno::UNO_SET_THROW);
    xPropertySet->getPropertyValue(u"CustomLabelFields"_ustr) >>= aFields;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), aFields.getLength());

    CPPUNIT_ASSERT_EQUAL(chart2::DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_CELLREF, aFields[0]->getFieldType());
    //CPPUNIT_ASSERT_EQUAL(OUString("70"), aFields[0]->getString()); TODO: Not implemented yet

    CPPUNIT_ASSERT_EQUAL(chart2::DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_TEXT, aFields[1]->getFieldType());
    CPPUNIT_ASSERT_EQUAL(u" <CELLREF"_ustr, aFields[1]->getString());
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testTdf115107_2)
{
    // import complex data point labels in cobo charts with multiple data series
    loadFromFile(u"pptx/tdf115107-2.pptx");

    Reference<chart2::XChartDocument> xChartDoc(getChartDocFromDrawImpress(0, 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());

    uno::Reference<chart2::XDataSeries> xDataSeries(getDataSeriesFromDoc(xChartDoc, 0));
    CPPUNIT_ASSERT(xDataSeries.is());
    float nFontSize;
    sal_Int64 nFontColor;
    uno::Reference<beans::XPropertySet> xPropertySet;
    uno::Sequence<uno::Reference<chart2::XDataPointCustomLabelField>> aFields;

    // First series
    xPropertySet.set(xDataSeries->getDataPointByIndex(0), uno::UNO_SET_THROW);
    xPropertySet->getPropertyValue(u"CustomLabelFields"_ustr) >>= aFields;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3), aFields.getLength());

    CPPUNIT_ASSERT_EQUAL(chart2::DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_VALUE, aFields[0]->getFieldType());
    CPPUNIT_ASSERT_EQUAL(u"4.3"_ustr, aFields[0]->getString());
    aFields[0]->getPropertyValue(u"CharHeight"_ustr) >>= nFontSize;
    aFields[0]->getPropertyValue(u"CharColor"_ustr) >>= nFontColor;
    CPPUNIT_ASSERT_EQUAL(static_cast<float>(18), nFontSize);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int64>(0xc00000), nFontColor);

    CPPUNIT_ASSERT_EQUAL(chart2::DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_TEXT, aFields[1]->getFieldType());
    CPPUNIT_ASSERT_EQUAL(u" "_ustr, aFields[1]->getString());

    CPPUNIT_ASSERT_EQUAL(chart2::DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_SERIESNAME, aFields[2]->getFieldType());
    CPPUNIT_ASSERT_EQUAL(u"Bars"_ustr, aFields[2]->getString());

    // Second series
    xDataSeries = getDataSeriesFromDoc(xChartDoc, 0, 1);
    CPPUNIT_ASSERT(xDataSeries.is());

    xPropertySet.set(xDataSeries->getDataPointByIndex(0), uno::UNO_SET_THROW);
    xPropertySet->getPropertyValue(u"CustomLabelFields"_ustr) >>= aFields;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3), aFields.getLength());

    CPPUNIT_ASSERT_EQUAL(chart2::DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_VALUE, aFields[0]->getFieldType());
    CPPUNIT_ASSERT_EQUAL(u"2"_ustr, aFields[0]->getString());
    aFields[0]->getPropertyValue(u"CharHeight"_ustr) >>= nFontSize;
    aFields[0]->getPropertyValue(u"CharColor"_ustr) >>= nFontColor;
    CPPUNIT_ASSERT_EQUAL(static_cast<float>(18), nFontSize);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int64>(0xffd966), nFontColor);

    CPPUNIT_ASSERT_EQUAL(chart2::DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_TEXT, aFields[1]->getFieldType());
    CPPUNIT_ASSERT_EQUAL(u" "_ustr, aFields[1]->getString());

    CPPUNIT_ASSERT_EQUAL(chart2::DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_SERIESNAME, aFields[2]->getFieldType());
    CPPUNIT_ASSERT_EQUAL(u"Line"_ustr, aFields[2]->getString());

}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testTdf116163)
{
    loadFromFile(u"pptx/tdf116163.pptx");

    Reference<chart2::XChartDocument> xChartDoc(getChartDocFromDrawImpress(0, 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());

    Reference<chart2::XAxis> xHAxis = getAxisFromDoc(xChartDoc, 0, 0, 0);
    CPPUNIT_ASSERT(xHAxis.is());

    chart2::ScaleData aScaleData = xHAxis->getScaleData();
    CPPUNIT_ASSERT(aScaleData.Categories.is());

    Reference<chart2::data::XLabeledDataSequence> xLabeledDataSequence = aScaleData.Categories;
    CPPUNIT_ASSERT(xLabeledDataSequence.is());

    Reference<chart2::data::XDataSequence> xDataSequence = xLabeledDataSequence->getValues();
    CPPUNIT_ASSERT(xDataSequence.is());

    Reference<chart2::data::XTextualDataSequence> xTextualDataSequence(xDataSequence, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xTextualDataSequence.is());

    std::vector<OUString> aCategories;
    const Sequence<OUString> aTextData(xTextualDataSequence->getTextualData());
    ::std::copy(aTextData.begin(), aTextData.end(),
        ::std::back_inserter(aCategories));

    CPPUNIT_ASSERT_EQUAL(u"Aaaa"_ustr, aCategories[0]);
    CPPUNIT_ASSERT_EQUAL(u"Bbbbbbb"_ustr, aCategories[1]);
    CPPUNIT_ASSERT_EQUAL(u"Ccc"_ustr, aCategories[2]);
    CPPUNIT_ASSERT_EQUAL(u"Ddddddddddddd"_ustr, aCategories[3]);

    // Check visible text

    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xChartDoc, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    uno::Reference<drawing::XShapes> xShapes(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xShapes.is());

    uno::Reference<drawing::XShape> xXAxis = getShapeByName(xShapes, u"CID/D=0:CS=0:Axis=0,0"_ustr,
        // Axis occurs twice in chart xshape representation so need to get the one related to labels
        [](const uno::Reference<drawing::XShape>& rXShape) -> bool
    {
        uno::Reference<drawing::XShapes> xAxisShapes(rXShape, uno::UNO_QUERY);
        CPPUNIT_ASSERT(xAxisShapes.is());
        uno::Reference<drawing::XShape> xChildShape(xAxisShapes->getByIndex(0), uno::UNO_QUERY);
        uno::Reference< drawing::XShapeDescriptor > xShapeDescriptor(xChildShape, uno::UNO_QUERY_THROW);
        return (xShapeDescriptor->getShapeType() == "com.sun.star.drawing.TextShape");
    });
    CPPUNIT_ASSERT(xXAxis.is());

    uno::Reference<container::XIndexAccess> xIndexAccess(xXAxis, UNO_QUERY_THROW);

    // Check text
    uno::Reference<text::XTextRange> xLabel0(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Aaaa"_ustr, xLabel0->getString());
    uno::Reference<text::XTextRange> xLabel1(xIndexAccess->getByIndex(1), uno::UNO_QUERY);
    // If there is space for 3 chars only then don't show "..."
    CPPUNIT_ASSERT_EQUAL(u"Bbb"_ustr, xLabel1->getString());
    uno::Reference<text::XTextRange> xLabel2(xIndexAccess->getByIndex(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Ccc"_ustr, xLabel2->getString());
    uno::Reference<text::XTextRange> xLabel3(xIndexAccess->getByIndex(3), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Dddd..."_ustr, xLabel3->getString());
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testTdf48041)
{
    loadFromFile(u"pptx/tdf48041.pptx");

    Reference<chart2::XChartDocument> xChartDoc(getChartDocFromDrawImpress(0, 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());

    Reference<chart2::XAxis> xYAxis = getAxisFromDoc(xChartDoc, 0, 1, 0);
    CPPUNIT_ASSERT(xYAxis.is());

    chart2::ScaleData aScaleData = xYAxis->getScaleData();
    CPPUNIT_ASSERT(aScaleData.Scaling.is());

    // Check visible text
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xChartDoc, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    uno::Reference<drawing::XShapes> xShapes(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xShapes.is());

    uno::Reference<drawing::XShape> xYAxisShape = getShapeByName(xShapes, u"CID/D=0:CS=0:Axis=1,0"_ustr, // Y Axis
        // Axis occurs twice in chart xshape representation so need to get the one related to labels
        [](const uno::Reference<drawing::XShape>& rXShape) -> bool
    {
        uno::Reference<drawing::XShapes> xAxisShapes(rXShape, uno::UNO_QUERY);
        CPPUNIT_ASSERT(xAxisShapes.is());
        uno::Reference<drawing::XShape> xChildShape(xAxisShapes->getByIndex(0), uno::UNO_QUERY);
        uno::Reference< drawing::XShapeDescriptor > xShapeDescriptor(xChildShape, uno::UNO_QUERY_THROW);
        return (xShapeDescriptor->getShapeType() == "com.sun.star.drawing.TextShape");
    });
    CPPUNIT_ASSERT(xYAxisShape.is());

    // Check label count
    uno::Reference<container::XIndexAccess> xIndexAccess(xYAxisShape, UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(6), xIndexAccess->getCount());

    // Check text
    uno::Reference<text::XTextRange> xLabel0(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"0"_ustr, xLabel0->getString());
    uno::Reference<text::XTextRange> xLabel1(xIndexAccess->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"1"_ustr, xLabel1->getString());
    uno::Reference<text::XTextRange> xLabel2(xIndexAccess->getByIndex(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"2"_ustr, xLabel2->getString());
    uno::Reference<text::XTextRange> xLabel3(xIndexAccess->getByIndex(3), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"3"_ustr, xLabel3->getString());
    uno::Reference<text::XTextRange> xLabel4(xIndexAccess->getByIndex(4), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"4"_ustr, xLabel4->getString());
    uno::Reference<text::XTextRange> xLabel5(xIndexAccess->getByIndex(5), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"5"_ustr, xLabel5->getString());
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testTdf121205)
{
    loadFromFile(u"pptx/tdf121205.pptx");
    Reference<chart2::XChartDocument> xChartDoc(getChartDocFromDrawImpress(0, 0), uno::UNO_QUERY);

    uno::Reference<chart2::XTitled> xTitled(xChartDoc, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("chart doc does not have title", xTitled.is());
    OUString aTitle = getTitleString(xTitled);

    // We expect title split in 3 lines
    CPPUNIT_ASSERT_EQUAL(u"Firstline\nSecondline\nThirdline"_ustr, aTitle);
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testTdf146487)
{
    loadFromFile(u"pptx/tdf146487.pptx");
    Reference<chart2::XChartDocument> xChartDoc(getChartDocFromDrawImpress(0, 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChartDoc.is());

    Reference<chart2::XTitled> xTitled(xChartDoc, uno::UNO_QUERY_THROW);
    uno::Reference<chart2::XTitle> xTitle = xTitled->getTitleObject();
    CPPUNIT_ASSERT_MESSAGE("chart doc should not have a title", !xTitle.is());

    // tdf#146756 use manualLayout Width that was provided (so Green;  $7,654,321 is not wrapped
    if (!IsDefaultDPI())
        return;
    uno::Reference<chart::XChartDocument> xDoc = getChartDocFromDrawImpress(0, 0);
    OString aXmlDump = OUStringToOString(getShapeDump(xDoc), RTL_TEXTENCODING_UTF8);
    xmlDocUniquePtr pXmlDoc(xmlParseDoc(reinterpret_cast<const xmlChar*>(aXmlDump.getStr())));
    OString aPath("//XShape[@text='Green;  $7,654,321 ']"_ostr);
    assertXPath(pXmlDoc, aPath, 1);
    // Expected - 1 line tall(371), not 4 lines(1481).
    CPPUNIT_ASSERT_EQUAL(u"371"_ustr, getXPath(pXmlDoc, aPath, "sizeY"_ostr));
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testTdf146756)
{
    // FIXME: the DPI check should be removed when either (1) the test is fixed to work with
    // non-default DPI; or (2) unit tests on Windows are made to use svp VCL plugin.
    if (!IsDefaultDPI())
        return;

    // given a chart on page 2
    loadFromFile(u"pptx/tdf146756_bestFit.pptx");
    Reference<chart::XChartDocument> xChartDoc = getChartDocFromDrawImpress(1, 0);
    OString aXmlDump = OUStringToOString(getShapeDump(xChartDoc), RTL_TEXTENCODING_UTF8);
    xmlDocUniquePtr pXmlDoc(xmlParseDoc(reinterpret_cast<const xmlChar*>(aXmlDump.getStr())));
    OString aPath("//XShape[@text='New service request and approval; 18%']"_ostr);
    assertXPath(pXmlDoc, aPath, 1);
    // Expected something like 4 lines tall(1697), not 11 lines(3817).
    CPPUNIT_ASSERT_EQUAL(u"1697"_ustr, getXPath(pXmlDoc, aPath, "sizeY"_ostr));
    // Expected some reasonable maximum text length for the label like 2350, not 881.
    sal_Int32 nTextLength = getXPath(pXmlDoc, aPath, "textMaximumFrameWidth"_ostr).toInt32();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2350), nTextLength);
    // MSO doesn't allow much more than 1/5 of the total chart width, so never go higher than that
    CPPUNIT_ASSERT_LESS(sal_Int32(2370.6), nTextLength);
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testFixedSizeBarChartVeryLongLabel)
{
    // Bar chart area size is fixed (not automatic) so we can't resize
    // the chart area to let the label break into multiple lines. In this
    // case the best course of action is to just crop the label text. This
    // test checks that the rendered text is actually cropped.

    loadFromFile(u"odp/BarChartVeryLongLabel.odp");

    // Select shape 0 which has fixed size chart
    Reference<chart2::XChartDocument> xChartDoc(getChartDocFromDrawImpress(0, 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());

    Reference<chart2::XAxis> xHAxis = getAxisFromDoc(xChartDoc, 0, 0, 0);
    CPPUNIT_ASSERT(xHAxis.is());

    chart2::ScaleData aScaleData = xHAxis->getScaleData();
    CPPUNIT_ASSERT(aScaleData.Categories.is());

    Reference<chart2::data::XLabeledDataSequence> xLabeledDataSequence = aScaleData.Categories;
    CPPUNIT_ASSERT(xLabeledDataSequence.is());

    Reference<chart2::data::XDataSequence> xDataSequence = xLabeledDataSequence->getValues();
    CPPUNIT_ASSERT(xDataSequence.is());

    Reference<chart2::data::XTextualDataSequence> xTextualDataSequence(xDataSequence, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xTextualDataSequence.is());

    std::vector<OUString> aCategories;
    const Sequence<OUString> aTextData(xTextualDataSequence->getTextualData());
    ::std::copy(aTextData.begin(), aTextData.end(),
        ::std::back_inserter(aCategories));

    // Check that we have a very very long label text
    CPPUNIT_ASSERT_EQUAL(u"Very very very very very very very very very very very loooooooooooong label"_ustr, aCategories[0]);

    // Check visible text
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xChartDoc, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    uno::Reference<drawing::XShapes> xShapes(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xShapes.is());

    uno::Reference<drawing::XShape> xXAxis = getShapeByName(xShapes, u"CID/D=0:CS=0:Axis=0,0"_ustr,
        // Axis occurs twice in chart xshape representation so need to get the one related to labels
        [](const uno::Reference<drawing::XShape>& rXShape) -> bool
    {
        uno::Reference<drawing::XShapes> xAxisShapes(rXShape, uno::UNO_QUERY);
        CPPUNIT_ASSERT(xAxisShapes.is());
        uno::Reference<drawing::XShape> xChildShape(xAxisShapes->getByIndex(0), uno::UNO_QUERY);
        uno::Reference< drawing::XShapeDescriptor > xShapeDescriptor(xChildShape, uno::UNO_QUERY_THROW);
        return (xShapeDescriptor->getShapeType() == "com.sun.star.drawing.TextShape");
    });
    CPPUNIT_ASSERT(xXAxis.is());

    uno::Reference<container::XIndexAccess> xIndexAccess(xXAxis, UNO_QUERY_THROW);

    // Check text is actually cropped. Depending on DPI,
    // it may be "Very very very very very very..." or "Very very very very very ver..."
    uno::Reference<text::XTextRange> xLabel(xIndexAccess->getByIndex(0), uno::UNO_QUERY_THROW);
    const OUString aLabelString = xLabel->getString();
    CPPUNIT_ASSERT_LESSEQUAL(sal_Int32(32), aLabelString.getLength());
    CPPUNIT_ASSERT(aLabelString.endsWith(u"..."));

    uno::Reference<drawing::XShape> xChartWall = getShapeByName(xShapes, u"CID/DiagramWall="_ustr);
    CPPUNIT_ASSERT(xChartWall.is());

    // The text shape width should be smaller than the chart wall
    CPPUNIT_ASSERT_LESS(xChartWall->getSize().Width, xXAxis->getSize().Width);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(7113, xChartWall->getSize().Height, 100);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(398, xXAxis->getSize().Height, 100);
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testAutomaticSizeBarChartVeryLongLabel)
{
    // Bar chart area size is automatic so we expect the label to be broken
    // into multiple lines.

    loadFromFile(u"odp/BarChartVeryLongLabel.odp");

    // Select shape 1, which has an automatic sized chart
    Reference<chart2::XChartDocument> xChartDoc(getChartDocFromDrawImpress(0, 1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());

    Reference<chart2::XAxis> xHAxis = getAxisFromDoc(xChartDoc, 0, 0, 0);
    CPPUNIT_ASSERT(xHAxis.is());

    chart2::ScaleData aScaleData = xHAxis->getScaleData();
    CPPUNIT_ASSERT(aScaleData.Categories.is());

    Reference<chart2::data::XLabeledDataSequence> xLabeledDataSequence = aScaleData.Categories;
    CPPUNIT_ASSERT(xLabeledDataSequence.is());

    Reference<chart2::data::XDataSequence> xDataSequence = xLabeledDataSequence->getValues();
    CPPUNIT_ASSERT(xDataSequence.is());

    Reference<chart2::data::XTextualDataSequence> xTextualDataSequence(xDataSequence, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xTextualDataSequence.is());

    std::vector<OUString> aCategories;
    const Sequence<OUString> aTextData(xTextualDataSequence->getTextualData());
    ::std::copy(aTextData.begin(), aTextData.end(),
        ::std::back_inserter(aCategories));

    // Check that we have a very very long label text
    CPPUNIT_ASSERT_EQUAL(u"Very very very very very very very very very very very loooooooooooong label"_ustr, aCategories[0]);

    // Check visible text
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xChartDoc, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    uno::Reference<drawing::XShapes> xShapes(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xShapes.is());

    uno::Reference<drawing::XShape> xXAxis = getShapeByName(xShapes, u"CID/D=0:CS=0:Axis=0,0"_ustr,
        // Axis occurs twice in chart xshape representation so need to get the one related to labels
        [](const uno::Reference<drawing::XShape>& rXShape) -> bool
    {
        uno::Reference<drawing::XShapes> xAxisShapes(rXShape, uno::UNO_QUERY);
        CPPUNIT_ASSERT(xAxisShapes.is());
        uno::Reference<drawing::XShape> xChildShape(xAxisShapes->getByIndex(0), uno::UNO_QUERY);
        uno::Reference< drawing::XShapeDescriptor > xShapeDescriptor(xChildShape, uno::UNO_QUERY_THROW);
        return (xShapeDescriptor->getShapeType() == "com.sun.star.drawing.TextShape");
    });
    CPPUNIT_ASSERT(xXAxis.is());

    uno::Reference<container::XIndexAccess> xIndexAccess(xXAxis, UNO_QUERY_THROW);

    // Check text is unmodified
    uno::Reference<text::XTextRange> xLabel(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Very very very very very very very very very very very loooooooooooong label"_ustr, xLabel->getString());

    uno::Reference<drawing::XShape> xChartWall = getShapeByName(xShapes, u"CID/DiagramWall="_ustr);
    CPPUNIT_ASSERT(xChartWall.is());

    // The text shape width should be smaller than the chart wall
    CPPUNIT_ASSERT_LESS(xChartWall->getSize().Width, xXAxis->getSize().Width);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(7200, xChartWall->getSize().Height, 100);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1192, xXAxis->getSize().Height, 100);
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testTotalsRowIgnored)
{
    loadFromFile(u"xlsx/barchart_totalsrow.xlsx");
    {
        uno::Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
        CPPUNIT_ASSERT(xChartDoc.is());

        Reference<chart2::data::XDataSequence> xDataSeq =
            getDataSequenceFromDocByRole(xChartDoc, u"values-y");
        CPPUNIT_ASSERT(xDataSeq.is());

        // Table data range is D2:D9 (8 rows) but because last row is totals row it is ignored
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(7u), xDataSeq->getData().size());
    }
    {
        uno::Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(1, mxComponent);
        CPPUNIT_ASSERT(xChartDoc.is());

        Reference<chart2::data::XDataSequence> xDataSeq =
            getDataSequenceFromDocByRole(xChartDoc, u"values-y");
        CPPUNIT_ASSERT(xDataSeq.is());

        // Table data range is D2:D10 (9 rows) and totals row isn't the last row so it's not ignored
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(9u), xDataSeq->getData().size());
    }
}

CPPUNIT_TEST_FIXTURE(Chart2ImportTest, testPieChartPlotAreaMarginWithAutomaticLayout)
{
    // tdf#91265
    // Checks the margin and calculation of the plot area for the pie chart inside the chart area.

    loadFromFile(u"pptx/PieChartWithAutomaticLayout_SizeAndPosition.pptx");

    OUString aCheckShapeName = u"CID/D=0:CS=0:CT=0:Series=0"_ustr;
    // Chart Wuse case Width == Height
    {
        // Load chart Chart_2_2 - 2cm x 2cm -
        auto xDocument = getChartDocFromDrawImpressNamed(0, u"Chart_2_2");
        CPPUNIT_ASSERT(xDocument.is());

        uno::Reference<chart2::XChartDocument>xChartDocument(xDocument, uno::UNO_QUERY);
        CPPUNIT_ASSERT(xChartDocument.is());

        // Get the shape of the diagram / chart
        uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xChartDocument, uno::UNO_QUERY);
        CPPUNIT_ASSERT(xDrawPageSupplier.is());
        uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
        uno::Reference<drawing::XShapes> xShapes(xDrawPage->getByIndex(0), uno::UNO_QUERY);
        CPPUNIT_ASSERT(xShapes.is());

        uno::Reference<drawing::XShape> xChartDiagramShape = getShapeByName(xShapes, aCheckShapeName);
        CPPUNIT_ASSERT(xChartDiagramShape.is());

        // Size
        CPPUNIT_ASSERT_DOUBLES_EQUAL(1300, xChartDiagramShape->getSize().Width, 5);  // calculated chart area size - 2 * margin
        CPPUNIT_ASSERT_DOUBLES_EQUAL(1300, xChartDiagramShape->getSize().Height, 5); // calculated chart area size - 2 * margin
        // Position
        CPPUNIT_ASSERT_DOUBLES_EQUAL(350, xChartDiagramShape->getPosition().X, 5); // margin
        CPPUNIT_ASSERT_DOUBLES_EQUAL(350, xChartDiagramShape->getPosition().Y, 5); // margin
    }

    // Chart use case - Width < Height
    {
        // Load chart Chart_3_4 - 3cm x 4cm
        auto xDocument = getChartDocFromDrawImpressNamed(0, u"Chart_3_4");
        CPPUNIT_ASSERT(xDocument.is());

        uno::Reference<chart2::XChartDocument>xChartDocument(xDocument, uno::UNO_QUERY);
        CPPUNIT_ASSERT(xChartDocument.is());

        // Get the shape of the diagram / chart
        uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xChartDocument, uno::UNO_QUERY);
        CPPUNIT_ASSERT(xDrawPageSupplier.is());
        uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
        uno::Reference<drawing::XShapes> xShapes(xDrawPage->getByIndex(0), uno::UNO_QUERY);
        CPPUNIT_ASSERT(xShapes.is());

        uno::Reference<drawing::XShape> xChartDiagramShape = getShapeByName(xShapes, aCheckShapeName);
        CPPUNIT_ASSERT(xChartDiagramShape.is());

        // Size
        CPPUNIT_ASSERT_DOUBLES_EQUAL(2300, xChartDiagramShape->getSize().Width, 5);  // calculated chart area size - 2 * margin
        CPPUNIT_ASSERT_DOUBLES_EQUAL(2300, xChartDiagramShape->getSize().Height, 5); // calculated chart area size - 2 * margin
        // Position
        CPPUNIT_ASSERT_DOUBLES_EQUAL(350, xChartDiagramShape->getPosition().X, 5); // margin
        CPPUNIT_ASSERT_DOUBLES_EQUAL(850, xChartDiagramShape->getPosition().Y, 5); // margin + calculated centering
    }

    // Chart use case - Width > Height
    {
        // Load chart Chart_3_2 - 3cm x 2cm
        auto xDocument = getChartDocFromDrawImpressNamed(0, u"Chart_3_2");
        CPPUNIT_ASSERT(xDocument.is());

        uno::Reference<chart2::XChartDocument>xChartDocument(xDocument, uno::UNO_QUERY);
        CPPUNIT_ASSERT(xChartDocument.is());

        // Get the shape of the diagram / chart
        uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xChartDocument, uno::UNO_QUERY);
        CPPUNIT_ASSERT(xDrawPageSupplier.is());
        uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
        uno::Reference<drawing::XShapes> xShapes(xDrawPage->getByIndex(0), uno::UNO_QUERY);
        CPPUNIT_ASSERT(xShapes.is());

        uno::Reference<drawing::XShape> xChartDiagramShape = getShapeByName(xShapes, aCheckShapeName);
        CPPUNIT_ASSERT(xChartDiagramShape.is());

        // Size
        CPPUNIT_ASSERT_DOUBLES_EQUAL(1300, xChartDiagramShape->getSize().Width, 5);  // calculated chart area size - 2 * margin
        CPPUNIT_ASSERT_DOUBLES_EQUAL(1300, xChartDiagramShape->getSize().Height, 5); // calculated chart area size - 2 * margin
        // Position
        CPPUNIT_ASSERT_DOUBLES_EQUAL(850, xChartDiagramShape->getPosition().X, 5); // margin + calculated centering
        CPPUNIT_ASSERT_DOUBLES_EQUAL(350, xChartDiagramShape->getPosition().Y, 5); // margin
    }
}

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
