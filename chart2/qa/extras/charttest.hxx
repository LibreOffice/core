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
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/util/NumberFormat.hpp>

#include <iostream>

using namespace com::sun::star;
using namespace com::sun::star::uno;

class ChartTest : public test::BootstrapFixture, public unotest::MacrosTest
{
public:
    void load( const char* pDir, const char* pName );
    void reload( const OUString& rFilterName );
    uno::Sequence < OUString > getImpressChartColumnDescriptions( const char* pDir, const char* pName );

    uno::Reference<chart::XChartDocument> getChartDocFromWriter( sal_Int32 nShape );

    uno::Reference<chart::XChartDocument> getChartDocFromDrawImpress( sal_Int32 nPage, sal_Int32 nShape );

    virtual void setUp();
    virtual void tearDown();
protected:
    Reference< lang::XComponent > mxComponent;
};

void ChartTest::load( const char* pDir, const char* pName )
{
    mxComponent = loadFromDesktop(getURLFromSrc(pDir) + OUString::createFromAscii(pName), "com.sun.star.sheet.SpreadsheetDocument");
    CPPUNIT_ASSERT(mxComponent.is());
}

void ChartTest::reload(const OUString& rFilterName)
{
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aArgs(1);
    aArgs[0].Name = "FilterName";
    aArgs[0].Value <<= rFilterName;
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    xStorable->storeToURL(aTempFile.GetURL(), aArgs);
    mxComponent->dispose();
    mxComponent = loadFromDesktop(aTempFile.GetURL(), "com.sun.star.sheet.SpreadsheetDocument");
    std::cout << aTempFile.GetURL();
    CPPUNIT_ASSERT(mxComponent.is());
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

uno::Sequence < OUString > ChartTest::getImpressChartColumnDescriptions( const char* pDir, const char* pName )
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
    uno::Reference< chart::XChartDocument > xChart1Doc( xDocModel, uno::UNO_QUERY_THROW );
    uno::Reference< chart::XChartDataArray > xChartData ( xChart1Doc->getData(), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xChartData.is());
    uno::Sequence < OUString > seriesList = xChartData->getColumnDescriptions();
    return seriesList;
}

uno::Reference<chart::XChartDocument> ChartTest::getChartDocFromWriter( sal_Int32 nShape )
{
    Reference<drawing::XDrawPageSupplier> xPageSupp(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xPageSupp.is());

    Reference<drawing::XDrawPage> xPage = xPageSupp->getDrawPage();
    CPPUNIT_ASSERT(xPage.is());

    Reference<beans::XPropertySet> xShapeProps(xPage->getByIndex(nShape), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xShapeProps.is());

    Reference<frame::XModel> xDocModel;
    xShapeProps->getPropertyValue("Model") >>= xDocModel;
    CPPUNIT_ASSERT(xDocModel.is());

    uno::Reference<chart::XChartDocument> xChartDoc(xDocModel, uno::UNO_QUERY);
    return xChartDoc;
}

uno::Reference<chart::XChartDocument> ChartTest::getChartDocFromDrawImpress(
    sal_Int32 nPage, sal_Int32 nShape )
{
    uno::Reference<chart::XChartDocument> xEmpty;

    uno::Reference<drawing::XDrawPagesSupplier> xPages(mxComponent, uno::UNO_QUERY);
    if (!xPages.is())
        return xEmpty;

    uno::Reference<drawing::XDrawPage> xPage(
        xPages->getDrawPages()->getByIndex(nPage), uno::UNO_QUERY_THROW);
    if (!xPage.is())
        return xEmpty;

    uno::Reference<beans::XPropertySet> xShapeProps(xPage->getByIndex(nShape), uno::UNO_QUERY);
    if (!xShapeProps.is())
        return xEmpty;

    uno::Reference<frame::XModel> xDocModel;
    xShapeProps->getPropertyValue("Model") >>= xDocModel;
    if (!xDocModel.is())
        return xEmpty;

    uno::Reference<chart::XChartDocument> xChartDoc(xDocModel, uno::UNO_QUERY);
    return xChartDoc;
}

std::vector<uno::Sequence<uno::Any> > getDataSeriesLabelsFromChartType( const Reference<chart2::XChartType>& xCT )
{
    OUString aLabelRole = xCT->getRoleOfSequenceForSeriesLabel();

    Reference<chart2::XDataSeriesContainer> xDSCont(xCT, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDSCont.is());
    Sequence<uno::Reference<chart2::XDataSeries> > aDataSeriesSeq = xDSCont->getDataSeries();

    std::vector<uno::Sequence<uno::Any> > aRet;
    for (sal_Int32 i = 0; i < aDataSeriesSeq.getLength(); ++i)
    {
        uno::Reference<chart2::data::XDataSource> xDSrc(aDataSeriesSeq[i], uno::UNO_QUERY);
        CPPUNIT_ASSERT(xDSrc.is());
        uno::Sequence<Reference<chart2::data::XLabeledDataSequence> > aDataSeqs = xDSrc->getDataSequences();
        for (sal_Int32 j = 0; j < aDataSeqs.getLength(); ++j)
        {
            Reference<chart2::data::XDataSequence> xValues = aDataSeqs[j]->getValues();
            CPPUNIT_ASSERT(xValues.is());
            Reference<beans::XPropertySet> xPropSet(xValues, uno::UNO_QUERY);
            if (!xPropSet.is())
                continue;

            OUString aRoleName;
            xPropSet->getPropertyValue("Role") >>= aRoleName;
            if (aRoleName == aLabelRole)
            {
                Reference<chart2::data::XLabeledDataSequence> xLabel = aDataSeqs[j];
                CPPUNIT_ASSERT(xLabel.is());
                Reference<chart2::data::XDataSequence> xDS2 = xLabel->getLabel();
                CPPUNIT_ASSERT(xDS2.is());
                uno::Sequence<uno::Any> aData = xDS2->getData();
                aRet.push_back(aData);
            }
        }
    }

    return aRet;
}

sal_Int32 getNumberFormat( const Reference<chart2::XChartDocument>& xChartDoc, const OUString& sFormat )
{
    Reference<util::XNumberFormatsSupplier> xNFS(xChartDoc, uno::UNO_QUERY_THROW);
    Reference<util::XNumberFormats> xNumberFormats = xNFS->getNumberFormats();
    CPPUNIT_ASSERT(xNumberFormats.is());

    return xNumberFormats->queryKey(sFormat, css::lang::Locale(), sal_False);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
