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
#include <comphelper/processfactory.hxx>

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
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>

#include <unotools/tempfile.hxx>

#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/chart2/XAnyDescriptionAccess.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XDiagram.hpp>
#include <com/sun/star/chart2/XDataSeries.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#include <com/sun/star/chart/XChartDataArray.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/chart/XChartDocument.hpp>
#include <iostream>

#include <libxml/xmlwriter.h>
#include <libxml/xpath.h>

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

using namespace com::sun::star;
using namespace com::sun::star::uno;


class ChartTest : public test::BootstrapFixture, public unotest::MacrosTest
{
public:
    void load( const OUString& rDir, const OUString& rFileName );
    boost::shared_ptr<utl::TempFile> reload( const OUString& rFileName );
    uno::Sequence < OUString > getImpressChartColumnDescriptions( const char* pDir, const char* pName );
    OUString getFileExtension( const OUString& rFileName );

    uno::Reference< chart::XChartDocument > getChartDocFromImpress( const char* pDir, const char* pName );

    void loadDocx(const char* pDir, const char* pName);
    virtual void setUp();
    virtual void tearDown();

protected:
    Reference< lang::XComponent > mxComponent;
    OUString maServiceName;
};

OUString ChartTest::getFileExtension( const OUString& aFileName )
{
    sal_Int32 nDotLocation = aFileName.lastIndexOf('.');
    CPPUNIT_ASSERT(nDotLocation != -1);
    return aFileName.copy(nDotLocation);
}

void ChartTest::load( const OUString& aDir, const OUString& aName )
{
    OUString extension = getFileExtension(aName);
    if(extension == "ods" || extension == "xlsx")
    {
        maServiceName = "com.sun.star.sheet.SpreadsheetDocument";
    }
    else if(extension == "docx")
    {
        maServiceName = "com.sun.star.text.TextDocument";
    }

    mxComponent = loadFromDesktop(getURLFromSrc(aDir) + aName, maServiceName);
    CPPUNIT_ASSERT(mxComponent.is());
}

boost::shared_ptr<utl::TempFile> ChartTest::reload(const OUString& rFilterName)
{
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aArgs(1);
    aArgs[0].Name = "FilterName";
    aArgs[0].Value <<= rFilterName;
    boost::shared_ptr<utl::TempFile> pTempFile = boost::make_shared<utl::TempFile>();
    pTempFile->EnableKillingFile();
    xStorable->storeToURL(pTempFile->GetURL(), aArgs);
    mxComponent->dispose();
    mxComponent = loadFromDesktop(pTempFile->GetURL(), maServiceName);
    std::cout << pTempFile->GetURL();
    if(rFilterName == "Calc Office Open XML")
    {
        validate(pTempFile->GetFileName(), test::OOXML);
    }
    else if(rFilterName == "Office Open XML Text")
    {
        // validate(pTempFile->GetFileName(), test::OOXML);
    }
    else if(rFilterName == "calc8")
    {
        // validate(pTempFile->GetFileName(), test::ODF);
    }

    CPPUNIT_ASSERT(mxComponent.is());
    return pTempFile;
}

void ChartTest::setUp()
{
    test::BootstrapFixture::setUp();

    mxDesktop.set( com::sun::star::frame::Desktop::create( comphelper::getComponentContext(getMultiServiceFactory()) ) );
}

void ChartTest::tearDown()
{
    if(mxComponent.is())
        mxComponent->dispose();

    test::BootstrapFixture::tearDown();

}
Reference< lang::XComponent > getChartCompFromSheet( sal_Int32 nSheet, uno::Reference< lang::XComponent > xComponent )
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

    return xChartComp;

}
Reference< chart2::XChartDocument > getChartDocFromSheet( sal_Int32 nSheet, uno::Reference< lang::XComponent > xComponent )
{
    uno::Reference< chart2::XChartDocument > xChartDoc ( getChartCompFromSheet(nSheet, xComponent), UNO_QUERY_THROW );
    CPPUNIT_ASSERT(xChartDoc.is());
    return xChartDoc;
}

Reference< chart2::XChartType > getChartTypeFromDoc( Reference< chart2::XChartDocument > xChartDoc,
                                                                sal_Int32 nChartType, sal_Int32 nCooSys = 0 )
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

Reference< chart2::XDataSeries > getDataSeriesFromDoc( uno::Reference< chart2::XChartDocument > xChartDoc,
                                                                sal_Int32 nDataSeries, sal_Int32 nChartType = 0, sal_Int32 nCooSys = 0 )
{
    Reference< chart2::XChartType > xChartType = getChartTypeFromDoc( xChartDoc, nChartType, nCooSys );
    Reference< chart2::XDataSeriesContainer > xDataSequenceContainer( xChartType, UNO_QUERY_THROW );
    CPPUNIT_ASSERT ( xDataSequenceContainer.is() );

    Sequence< Reference< chart2::XDataSeries > > xSeriesSequence( xDataSequenceContainer->getDataSeries() );
    CPPUNIT_ASSERT( xSeriesSequence.getLength() > nDataSeries );

    Reference< chart2::XDataSeries > xSeries = xSeriesSequence[nDataSeries];

    return xSeries;
}


uno::Sequence < OUString > getWriterChartColumnDescriptions( Reference< lang::XComponent > mxComponent )
{
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    uno::Reference<drawing::XShape> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT( xShape.is() );
    uno::Reference<beans::XPropertySet> xPropertySet(xShape, uno::UNO_QUERY);
    uno::Reference< chart2::XChartDocument > xChartDoc;
    xChartDoc.set( xPropertySet->getPropertyValue( "Model" ), uno::UNO_QUERY );
    CPPUNIT_ASSERT( xChartDoc.is() );
    CPPUNIT_ASSERT( xChartDoc->getDataProvider().is() );
    uno::Reference<beans::XPropertySet> xProp(xChartDoc->getDataProvider(), uno::UNO_QUERY );
    uno::Reference< chart2::XAnyDescriptionAccess > xAnyDescriptionAccess ( xChartDoc->getDataProvider(), uno::UNO_QUERY_THROW );
    CPPUNIT_ASSERT( xAnyDescriptionAccess.is() );
    uno::Sequence< OUString > seriesList = xAnyDescriptionAccess->getColumnDescriptions();
    return seriesList;
}

uno::Reference< chart::XChartDocument > ChartTest::getChartDocFromImpress( const char* pDir, const char* pName )
{
    mxComponent = loadFromDesktop(getURLFromSrc(pDir) + OUString::createFromAscii(pName), "com.sun.star.comp.Draw.PresentationDocument");
    uno::Reference< drawing::XDrawPagesSupplier > xDoc(mxComponent, uno::UNO_QUERY_THROW );
    uno::Reference< drawing::XDrawPage > xPage(
        xDoc->getDrawPages()->getByIndex(0), uno::UNO_QUERY_THROW );
    CPPUNIT_ASSERT(xPage.is());
    uno::Reference< beans::XPropertySet > xShapeProps(
        xPage->getByIndex(0), uno::UNO_QUERY );
    CPPUNIT_ASSERT(xShapeProps.is());
    uno::Reference< frame::XModel > xDocModel;
    xShapeProps->getPropertyValue("Model") >>= xDocModel;
    CPPUNIT_ASSERT(xDocModel.is());
    uno::Reference< chart::XChartDocument > xChartDoc( xDocModel, uno::UNO_QUERY_THROW );

    return xChartDoc;
}

uno::Sequence < OUString > ChartTest::getImpressChartColumnDescriptions( const char* pDir, const char* pName )
{
    uno::Reference< chart::XChartDocument > xChartDoc = getChartDocFromImpress( pDir, pName );
    uno::Reference< chart::XChartDataArray > xChartData ( xChartDoc->getData(), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xChartData.is());
    uno::Sequence < OUString > seriesList = xChartData->getColumnDescriptions();
    return seriesList;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
