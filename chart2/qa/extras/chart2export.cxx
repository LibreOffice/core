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

using uno::Reference;
using beans::XPropertySet;

class Chart2ExportTest : public ChartTest
{
public:
    void test();
    void testErrorBarXLSX();

    CPPUNIT_TEST_SUITE(Chart2ExportTest);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST(testErrorBarXLSX);
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
    xErrorBar->getPropertyValue("ErrorBarStyle") >>= nErrorBarStyle;
    CPPUNIT_ASSERT_EQUAL(nErrorBarStyle, chart::ErrorBarStyle::RELATIVE);
    bool bShowPositive, bShowNegative;
    xErrorBar->getPropertyValue("ShowPositiveError") >>= bShowPositive;
    CPPUNIT_ASSERT(bShowPositive);
    xErrorBar->getPropertyValue("ShowNegativeError") >>= bShowNegative;
    CPPUNIT_ASSERT(bShowNegative);
    double nVal;
    xErrorBar->getPropertyValue("PositiveError") >>= nVal;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(nVal, 10.0, 1e-10);
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

CPPUNIT_TEST_SUITE_REGISTRATION(Chart2ExportTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
