/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>
#include <unotest/macros_test.hxx>

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/table/XTableChartsSupplier.hpp>
#include <com/sun/star/table/XTableCharts.hpp>
#include <com/sun/star/table/XTableChart.hpp>
#include <com/sun/star/document/XEmbeddedObjectSupplier.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XDiagram.hpp>
#include <com/sun/star/chart2/XDataSeries.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#include <com/sun/star/chart2/CurveStyle.hpp>
#include <com/sun/star/chart/ErrorBarStyle.hpp>
#include <com/sun/star/chart/XChartDocument.hpp>

#include <comphelper/processfactory.hxx>

using namespace com::sun::star;
using namespace com::sun::star::uno;

class Chart2ImportTest : public test::BootstrapFixture, public unotest::MacrosTest
{
public:
    void Fdo60083();
    void testSteppedLines();

    virtual void setUp();
    virtual void tearDown();

    CPPUNIT_TEST_SUITE(Chart2ImportTest);
    CPPUNIT_TEST(Fdo60083);
    CPPUNIT_TEST(testSteppedLines);
    CPPUNIT_TEST_SUITE_END();

private:
    void load( const char* pDir, const char* pName );

    Reference< chart2::XChartDocument > getChartDocFromSheet( sal_Int32 nSheet, uno::Reference< lang::XComponent > xComponent );
    Reference< chart2::XDataSeries > getDataSeriesFromDoc( Reference< chart2::XChartDocument > xChartDoc,
            sal_Int32 nDataSeries, sal_Int32 nChartType = 0, sal_Int32 nCooSys = 0 );
    Reference< chart2::XChartType > getChartTypeFromDoc( Reference< chart2::XChartDocument > xChartDoc,
            sal_Int32 nChartType, sal_Int32 nCooSys = 0 );

    Reference< lang::XComponent > mxComponent;
};

// error bar import
// split method up into smaller chunks for more detailed tests
void Chart2ImportTest::Fdo60083()
{
    load("/chart2/qa/extras/data/ods/", "error_bar_simple.ods");
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
        xErrorBarYProps->getPropertyValue("ErrorBarStyle") >>= nErrorBarStyle;
        CPPUNIT_ASSERT_EQUAL(nErrorBarStyle, static_cast<sal_Int32>(chart::ErrorBarStyle::RELATIVE));

        double nVal;
        xErrorBarYProps->getPropertyValue("PositiveError") >>= nVal;
        CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0, nVal, 1e-8);

        xErrorBarYProps->getPropertyValue("NegativeError") >>= nVal;
        CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0, nVal, 1e-8);

        sal_Bool bVal;
        xErrorBarYProps->getPropertyValue("ShowPositiveError") >>= bVal;
        CPPUNIT_ASSERT_EQUAL(static_cast<bool>(bVal), true);

        xErrorBarYProps->getPropertyValue("ShowNegativeError") >>= bVal;
        CPPUNIT_ASSERT_EQUAL(static_cast<bool>(bVal), true);
    }

    // test that x error bars are not imported
    Reference< beans::XPropertySet > xErrorBarXProps;
    xPropSet->getPropertyValue("ErrorBarX") >>= xErrorBarXProps;
    CPPUNIT_ASSERT(!xErrorBarXProps.is());
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

Reference< chart2::XChartDocument > Chart2ImportTest::getChartDocFromSheet( sal_Int32 nSheet, uno::Reference< lang::XComponent > xComponent )
{
    // let us assume that we only have one chart per sheet

    uno::Reference< sheet::XSpreadsheetDocument > xDoc(xComponent, UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xDoc.is());

    uno::Reference< container::XIndexAccess > xIA(xDoc->getSheets(), UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xIA.is());

    uno::Reference< table::XTableChartsSupplier > xChartSupplier( xIA->getByIndex(nSheet), UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xChartSupplier.is());

    uno::Reference< table::XTableCharts > xCharts = xChartSupplier->getCharts();
    CPPUNIT_ASSERT(xCharts.is());

    uno::Reference< container::XIndexAccess > xIACharts(xCharts, UNO_QUERY_THROW);
    uno::Reference< table::XTableChart > xChart( xIACharts->getByIndex(0), UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xChart.is());

    uno::Reference< document::XEmbeddedObjectSupplier > xEmbObjectSupplier(xChart, UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xEmbObjectSupplier.is());

    uno::Reference< lang::XComponent > xChartComp( xEmbObjectSupplier->getEmbeddedObject(), UNO_QUERY_THROW );
    CPPUNIT_ASSERT(xChartComp.is());

    uno::Reference< chart2::XChartDocument > xChartDoc ( xChartComp, UNO_QUERY_THROW );

    CPPUNIT_ASSERT(xChartDoc.is());
    return xChartDoc;
}

Reference< chart2::XChartType > Chart2ImportTest::getChartTypeFromDoc( Reference< chart2::XChartDocument > xChartDoc,
                                                                sal_Int32 nChartType, sal_Int32 nCooSys )
{
    CPPUNIT_ASSERT( xChartDoc.is() );

    Reference <chart2::XDiagram > xDiagram = xChartDoc->getFirstDiagram();
    CPPUNIT_ASSERT( xDiagram.is() );

    Reference< chart2::XCoordinateSystemContainer > xCooSysContainer( xDiagram, UNO_QUERY_THROW );
    CPPUNIT_ASSERT( xCooSysContainer.is() );

    Sequence< Reference< chart2::XCoordinateSystem > > xCooSysSequence( xCooSysContainer->getCoordinateSystems());
    CPPUNIT_ASSERT( xCooSysSequence.getLength() > nCooSys );

    Reference< chart2::XChartTypeContainer > xChartTypeContainer( xCooSysSequence[nCooSys], UNO_QUERY_THROW );
    CPPUNIT_ASSERT( xChartTypeContainer.is() );

    Sequence< Reference< chart2::XChartType > > xChartTypeSequence( xChartTypeContainer->getChartTypes() );
    CPPUNIT_ASSERT( xChartTypeSequence.getLength() > nChartType );

    return xChartTypeSequence[nChartType];
}

Reference< chart2::XDataSeries > Chart2ImportTest::getDataSeriesFromDoc( uno::Reference< chart2::XChartDocument > xChartDoc,
                                                                sal_Int32 nDataSeries, sal_Int32 nChartType, sal_Int32 nCooSys )
{
    Reference< chart2::XChartType > xChartType = getChartTypeFromDoc( xChartDoc, nChartType, nCooSys );
    Reference< chart2::XDataSeriesContainer > xDataSequenceContainer( xChartType, UNO_QUERY_THROW );
    CPPUNIT_ASSERT ( xDataSequenceContainer.is() );

    Sequence< Reference< chart2::XDataSeries > > xSeriesSequence( xDataSequenceContainer->getDataSeries() );
    CPPUNIT_ASSERT( xSeriesSequence.getLength() > nDataSeries );

    Reference< chart2::XDataSeries > xSeries = xSeriesSequence[nDataSeries];

    return xSeries;
}

void Chart2ImportTest::load( const char* pDir, const char* pName )
{
    mxComponent = loadFromDesktop(getURLFromSrc(pDir) + OUString::createFromAscii(pName), "com.sun.star.sheet.SpreadsheetDocument");
    CPPUNIT_ASSERT(mxComponent.is());
}

void Chart2ImportTest::setUp()
{
    test::BootstrapFixture::setUp();

    mxDesktop.set( com::sun::star::frame::Desktop::create( comphelper::getComponentContext(getMultiServiceFactory()) ) );
}

void Chart2ImportTest::tearDown()
{
    if(mxComponent.is())
        mxComponent->dispose();

    test::BootstrapFixture::tearDown();

}

CPPUNIT_TEST_SUITE_REGISTRATION(Chart2ImportTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
