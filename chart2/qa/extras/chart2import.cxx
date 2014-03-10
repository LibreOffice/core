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
#include <com/sun/star/chart/ErrorBarStyle.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart/XChartDocument.hpp>
#include <com/sun/star/chart/XChartData.hpp>
#include <com/sun/star/chart2/XInternalDataProvider.hpp>
#include <com/sun/star/chart/XChartDataArray.hpp>

class Chart2ImportTest : public ChartTest
{
public:
    void Fdo60083();
    void testSteppedLines();
    void testErrorBarRange();
    void testODSChartSeries();
    void testXLSXChartSeries();
    void testXLSChartSeries();
    void testODTChartSeries();
    void testDOCChartSeries();
    void testDOCXChartSeries();
    void testPPTXChartSeries();
    void testPPTChartSeries();
    void testODPChartSeries();
    void testBnc864396();
    void testSimpleStrictXLSX();

    CPPUNIT_TEST_SUITE(Chart2ImportTest);
    CPPUNIT_TEST(Fdo60083);
    CPPUNIT_TEST(testSteppedLines);
    CPPUNIT_TEST(testErrorBarRange);
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
    CPPUNIT_TEST(testODPChartSeries);
    CPPUNIT_TEST(testBnc864396);
    CPPUNIT_TEST(testSimpleStrictXLSX);
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
    xPropSet->getPropertyValue("ErrorBarY") >>= xErrorBarYProps;
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
    xPropSet->getPropertyValue("ErrorBarX") >>= xErrorBarXProps;
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
    xPropSet->getPropertyValue("ErrorBarY") >>= xErrorBarYProps;
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
    uno::Sequence< OUString > seriesList = getWriterChartColumnDescriptions(mxComponent);
    CPPUNIT_ASSERT_EQUAL(OUString("Series 1"), seriesList[0]);
    CPPUNIT_ASSERT_EQUAL(OUString("Series 2"), seriesList[1]);
    CPPUNIT_ASSERT_EQUAL(OUString("Series 3"), seriesList[2]);
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
    uno::Sequence < OUString > seriesList = getImpressChartColumnDescriptions("/chart2/qa/extras/data/pptx/", "chart.pptx");
    CPPUNIT_ASSERT_EQUAL(OUString("Column 1"), seriesList[1]);
    CPPUNIT_ASSERT_EQUAL(OUString("Column 2"), seriesList[2]);
    CPPUNIT_ASSERT_EQUAL(OUString("Column 3"), seriesList[3]);

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

void Chart2ImportTest::testSimpleStrictXLSX()
{
    load("/chart2/qa/extras/data/xlsx/", "strict_chart.xlsx");
    uno::Reference< chart2::XChartDocument > xChartDoc = getChartDocFromSheet( 0, mxComponent );
    CPPUNIT_ASSERT(xChartDoc.is());

    Reference< chart2::XDataSeries > xDataSeries = getDataSeriesFromDoc( xChartDoc, 0 );
    CPPUNIT_ASSERT(xDataSeries.is());

}

CPPUNIT_TEST_SUITE_REGISTRATION(Chart2ImportTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
