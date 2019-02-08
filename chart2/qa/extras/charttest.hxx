/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CHART2_QA_EXTRAS_CHARTTEST_HXX
#define INCLUDED_CHART2_QA_EXTRAS_CHARTTEST_HXX

#include <test/bootstrapfixture.hxx>
#include <unotest/macros_test.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/frame/Desktop.hpp>

#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/table/XTableChartsSupplier.hpp>
#include <com/sun/star/table/XTableChart.hpp>
#include <com/sun/star/table/XTablePivotChartsSupplier.hpp>
#include <com/sun/star/table/XTablePivotChart.hpp>
#include <com/sun/star/document/XEmbeddedObjectSupplier.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XStorable.hpp>

#include <unotools/tempfile.hxx>

#include <com/sun/star/chart2/XAnyDescriptionAccess.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#include <com/sun/star/chart2/XFormattedString.hpp>
#include <com/sun/star/chart2/XTitle.hpp>
#include <com/sun/star/chart2/XTitled.hpp>
#include <com/sun/star/chart2/data/XLabeledDataSequence.hpp>
#include <com/sun/star/chart2/data/XDataSource.hpp>
#include <com/sun/star/chart/XChartDataArray.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/chart/XChartDocument.hpp>
#include <com/sun/star/text/XTextEmbeddedObjectsSupplier.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/util/NumberFormat.hpp>

#include <unonames.hxx>

#include <iostream>
#include <memory>

#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/embed/XVisualObject.hpp>
#include <com/sun/star/chart2/RelativeSize.hpp>

using namespace css;
using namespace css::uno;

namespace com { namespace sun { namespace star { namespace chart2 { class XDataSeries; } } } }
namespace com { namespace sun { namespace star { namespace chart2 { class XDiagram; } } } }
namespace com { namespace sun { namespace star { namespace table { class XTableCharts; } } } }
namespace com { namespace sun { namespace star { namespace table { class XTablePivotCharts; } } } }

class ChartTest : public test::BootstrapFixture, public unotest::MacrosTest
{
public:
    ChartTest():mbSkipValidation(false) {}
    void load( const OUString& rDir, const OUString& rFileName );
    std::shared_ptr<utl::TempFile> save( const OUString& rFileName );
    std::shared_ptr<utl::TempFile> reload( const OUString& rFileName );
    uno::Sequence < OUString > getImpressChartColumnDescriptions( const OUString& pDir, const char* pName );
    OUString getFileExtension( const OUString& rFileName );

    uno::Reference< chart::XChartDocument > getChartDocFromImpress( const OUString& pDir, const char* pName );

    uno::Reference<chart::XChartDocument> getChartDocFromDrawImpress( sal_Int32 nPage, sal_Int32 nShape );

    uno::Reference<chart::XChartDocument> getChartDocFromWriter( sal_Int32 nShape );
    awt::Size getPageSize( const Reference< chart2::XChartDocument > & xChartDoc );
    awt::Size getSize(css::uno::Reference<chart2::XDiagram> xDiagram, const awt::Size& rPageSize);

    virtual void setUp() override;
    virtual void tearDown() override;

protected:
    Reference< lang::XComponent > mxComponent;
    OUString maServiceName;
    bool mbSkipValidation; // if you set this flag for a new test I'm going to haunt you!
};

OUString ChartTest::getFileExtension( const OUString& aFileName )
{
    sal_Int32 nDotLocation = aFileName.lastIndexOf('.');
    CPPUNIT_ASSERT(nDotLocation != -1);
    return aFileName.copy(nDotLocation+1); // Skip the dot.
}

void ChartTest::load( const OUString& aDir, const OUString& aName )
{
    OUString extension = getFileExtension(aName);
    if (extension == "ods" || extension == "xlsx" || extension == "fods")
    {
        maServiceName = "com.sun.star.sheet.SpreadsheetDocument";
    }
    else if (extension == "docx")
    {
        maServiceName = "com.sun.star.text.TextDocument";
    }
    else if (extension == "odg")
    {
        maServiceName = "com.sun.star.drawing.DrawingDocument";
    }
    if (mxComponent.is())
        mxComponent->dispose();
    mxComponent = loadFromDesktop(m_directories.getURLFromSrc(aDir) + aName, maServiceName);
    CPPUNIT_ASSERT(mxComponent.is());
}

std::shared_ptr<utl::TempFile> ChartTest::save(const OUString& rFilterName)
{
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    auto aArgs(::comphelper::InitPropertySequence({
        { "FilterName", Any(rFilterName) }
    }));
    std::shared_ptr<utl::TempFile> pTempFile = std::make_shared<utl::TempFile>();
    pTempFile->EnableKillingFile();
    xStorable->storeToURL(pTempFile->GetURL(), aArgs);

    return pTempFile;
}

std::shared_ptr<utl::TempFile> ChartTest::reload(const OUString& rFilterName)
{
    std::shared_ptr<utl::TempFile> pTempFile = save(rFilterName);
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
        if(!mbSkipValidation)
            validate(pTempFile->GetFileName(), test::ODF);
    }
    else if(rFilterName == "MS Excel 97")
    {
        if(!mbSkipValidation)
            validate(pTempFile->GetFileName(), test::MSBINARY);
    }
    CPPUNIT_ASSERT(mxComponent.is());
    return pTempFile;
}

void ChartTest::setUp()
{
    test::BootstrapFixture::setUp();

    mxDesktop.set( css::frame::Desktop::create( comphelper::getComponentContext(getMultiServiceFactory()) ) );
}

void ChartTest::tearDown()
{
    if(mxComponent.is())
        mxComponent->dispose();

    test::BootstrapFixture::tearDown();

}

Reference< lang::XComponent > getChartCompFromSheet( sal_Int32 nSheet, uno::Reference< lang::XComponent > const & xComponent )
{
    // let us assume that we only have one chart per sheet

    uno::Reference< sheet::XSpreadsheetDocument > xDoc(xComponent, UNO_QUERY_THROW);

    uno::Reference< container::XIndexAccess > xIA(xDoc->getSheets(), UNO_QUERY_THROW);

    uno::Reference< table::XTableChartsSupplier > xChartSupplier( xIA->getByIndex(nSheet), UNO_QUERY_THROW);

    uno::Reference< table::XTableCharts > xCharts = xChartSupplier->getCharts();
    CPPUNIT_ASSERT(xCharts.is());

    uno::Reference< container::XIndexAccess > xIACharts(xCharts, UNO_QUERY_THROW);
    uno::Reference< table::XTableChart > xChart( xIACharts->getByIndex(0), UNO_QUERY_THROW);

    uno::Reference< document::XEmbeddedObjectSupplier > xEmbObjectSupplier(xChart, UNO_QUERY_THROW);

    uno::Reference< lang::XComponent > xChartComp( xEmbObjectSupplier->getEmbeddedObject(), UNO_QUERY_THROW );

    return xChartComp;

}

Reference< chart2::XChartDocument > getChartDocFromSheet( sal_Int32 nSheet, uno::Reference< lang::XComponent > const & xComponent )
{
    uno::Reference< chart2::XChartDocument > xChartDoc ( getChartCompFromSheet(nSheet, xComponent), UNO_QUERY_THROW );
    return xChartDoc;
}

uno::Reference<table::XTablePivotCharts> getTablePivotChartsFromSheet(sal_Int32 nSheet, uno::Reference<lang::XComponent> const & xComponent)
{
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(xComponent, UNO_QUERY_THROW);

    uno::Reference<container::XIndexAccess> xIA(xDoc->getSheets(), UNO_QUERY_THROW);

    uno::Reference<table::XTablePivotChartsSupplier> xChartSupplier(xIA->getByIndex(nSheet), UNO_QUERY_THROW);

    uno::Reference<table::XTablePivotCharts> xTablePivotCharts = xChartSupplier->getPivotCharts();
    CPPUNIT_ASSERT(xTablePivotCharts.is());

    return xTablePivotCharts;
}

Reference<lang::XComponent> getPivotChartCompFromSheet(sal_Int32 nSheet, uno::Reference<lang::XComponent> const & xComponent)
{
    uno::Reference<table::XTablePivotCharts> xTablePivotCharts = getTablePivotChartsFromSheet(nSheet, xComponent);

    uno::Reference<container::XIndexAccess> xIACharts(xTablePivotCharts, UNO_QUERY_THROW);
    uno::Reference<table::XTablePivotChart> xTablePivotChart(xIACharts->getByIndex(0), UNO_QUERY_THROW);

    uno::Reference<document::XEmbeddedObjectSupplier> xEmbObjectSupplier(xTablePivotChart, UNO_QUERY_THROW);

    uno::Reference<lang::XComponent> xChartComp(xEmbObjectSupplier->getEmbeddedObject(), UNO_QUERY_THROW);

    return xChartComp;
}

Reference<chart2::XChartDocument> getPivotChartDocFromSheet(sal_Int32 nSheet, uno::Reference<lang::XComponent> const & xComponent)
{
    uno::Reference<chart2::XChartDocument> xChartDoc(getPivotChartCompFromSheet(nSheet, xComponent), UNO_QUERY_THROW);
    return xChartDoc;
}

Reference<chart2::XChartDocument> getPivotChartDocFromSheet(uno::Reference<table::XTablePivotCharts> const & xTablePivotCharts, sal_Int32 nIndex)
{
    uno::Reference<container::XIndexAccess> xIACharts(xTablePivotCharts, UNO_QUERY_THROW);
    uno::Reference<table::XTablePivotChart> xTablePivotChart(xIACharts->getByIndex(nIndex), UNO_QUERY_THROW);

    uno::Reference<document::XEmbeddedObjectSupplier> xEmbObjectSupplier(xTablePivotChart, UNO_QUERY_THROW);

    uno::Reference<lang::XComponent> xChartComp(xEmbObjectSupplier->getEmbeddedObject(), UNO_QUERY_THROW);

    uno::Reference<chart2::XChartDocument> xChartDoc(xChartComp, UNO_QUERY_THROW);
    return xChartDoc;
}

Reference< chart2::XChartType > getChartTypeFromDoc( Reference< chart2::XChartDocument > const & xChartDoc,
                                                                sal_Int32 nChartType, sal_Int32 nCooSys = 0 )
{
    CPPUNIT_ASSERT( xChartDoc.is() );

    Reference <chart2::XDiagram > xDiagram = xChartDoc->getFirstDiagram();
    CPPUNIT_ASSERT( xDiagram.is() );

    Reference< chart2::XCoordinateSystemContainer > xCooSysContainer( xDiagram, UNO_QUERY_THROW );

    Sequence< Reference< chart2::XCoordinateSystem > > xCooSysSequence( xCooSysContainer->getCoordinateSystems());
    CPPUNIT_ASSERT( xCooSysSequence.getLength() > nCooSys );

    Reference< chart2::XChartTypeContainer > xChartTypeContainer( xCooSysSequence[nCooSys], UNO_QUERY_THROW );

    Sequence< Reference< chart2::XChartType > > xChartTypeSequence( xChartTypeContainer->getChartTypes() );
    CPPUNIT_ASSERT( xChartTypeSequence.getLength() > nChartType );

    return xChartTypeSequence[nChartType];
}

Reference<chart2::XAxis> getAxisFromDoc(
    const Reference<chart2::XChartDocument>& xChartDoc, sal_Int32 nCooSys, sal_Int32 nAxisDim, sal_Int32 nAxisIndex )
{
    Reference<chart2::XDiagram> xDiagram = xChartDoc->getFirstDiagram();
    CPPUNIT_ASSERT(xDiagram.is());

    Reference<chart2::XCoordinateSystemContainer> xCooSysContainer(xDiagram, UNO_QUERY_THROW);

    Sequence<Reference<chart2::XCoordinateSystem> > xCooSysSequence = xCooSysContainer->getCoordinateSystems();
    CPPUNIT_ASSERT(xCooSysSequence.getLength() > nCooSys);

    Reference<chart2::XCoordinateSystem> xCoord = xCooSysSequence[nCooSys];
    CPPUNIT_ASSERT(xCoord.is());

    Reference<chart2::XAxis> xAxis = xCoord->getAxisByDimension(nAxisDim, nAxisIndex);
    CPPUNIT_ASSERT(xAxis.is());

    return xAxis;
}

sal_Int32 getNumberOfDataSeries(uno::Reference<chart2::XChartDocument> const & xChartDoc,
                                sal_Int32 nChartType = 0, sal_Int32 nCooSys = 0)
{
    Reference<chart2::XChartType> xChartType = getChartTypeFromDoc(xChartDoc, nChartType, nCooSys);
    Reference<chart2::XDataSeriesContainer> xDataSeriesContainer(xChartType, UNO_QUERY_THROW);

    uno::Sequence<uno::Reference<chart2::XDataSeries>> xSeriesSequence(xDataSeriesContainer->getDataSeries());
    return xSeriesSequence.getLength();
}

Reference< chart2::XDataSeries > getDataSeriesFromDoc(uno::Reference<chart2::XChartDocument> const & xChartDoc,
                                                      sal_Int32 nDataSeries, sal_Int32 nChartType = 0,
                                                      sal_Int32 nCooSys = 0)
{
    Reference< chart2::XChartType > xChartType = getChartTypeFromDoc( xChartDoc, nChartType, nCooSys );
    Reference< chart2::XDataSeriesContainer > xDataSeriesContainer( xChartType, UNO_QUERY_THROW );

    Sequence< Reference< chart2::XDataSeries > > xSeriesSequence( xDataSeriesContainer->getDataSeries() );
    CPPUNIT_ASSERT( xSeriesSequence.getLength() > nDataSeries );

    Reference< chart2::XDataSeries > xSeries = xSeriesSequence[nDataSeries];

    return xSeries;
}

Reference< chart2::data::XDataSequence > getLabelDataSequenceFromDoc(
        Reference< chart2::XChartDocument > const & xChartDoc,
        sal_Int32 nDataSeries = 0, sal_Int32 nChartType = 0 )
{
    Reference< chart2::XDataSeries > xDataSeries =
        getDataSeriesFromDoc( xChartDoc, nDataSeries, nChartType );
    CPPUNIT_ASSERT(xDataSeries.is());
    Reference< chart2::data::XDataSource > xDataSource( xDataSeries, uno::UNO_QUERY_THROW );
    Sequence< Reference< chart2::data::XLabeledDataSequence > > xDataSequences =
        xDataSource->getDataSequences();
    for(sal_Int32 i = 0; i < xDataSequences.getLength(); ++i)
    {
        Reference< chart2::data::XDataSequence> xLabelSeq = xDataSequences[i]->getLabel();
        if(!xLabelSeq.is())
            continue;

        return xLabelSeq;
    }

    CPPUNIT_FAIL("no Label sequence found");
}

Reference< chart2::data::XDataSequence > getDataSequenceFromDocByRole(
        Reference< chart2::XChartDocument > const & xChartDoc, const OUString& rRole,
        sal_Int32 nDataSeries = 0, sal_Int32 nChartType = 0 )
{
    Reference< chart2::XDataSeries > xDataSeries =
        getDataSeriesFromDoc( xChartDoc, nDataSeries, nChartType );
    CPPUNIT_ASSERT(xDataSeries.is());
    Reference< chart2::data::XDataSource > xDataSource( xDataSeries, uno::UNO_QUERY_THROW );
    Sequence< Reference< chart2::data::XLabeledDataSequence > > xDataSequences =
        xDataSource->getDataSequences();
    for(sal_Int32 i = 0; i < xDataSequences.getLength(); ++i)
    {
        Reference< chart2::data::XDataSequence> xLabelSeq = xDataSequences[i]->getValues();
        uno::Reference< beans::XPropertySet > xProps(xLabelSeq, uno::UNO_QUERY);
        if(!xProps.is())
            continue;

        OUString aRoleName = xProps->getPropertyValue("Role").get<OUString>();

        if(aRoleName == rRole)
            return xLabelSeq;
    }

    return Reference< chart2::data::XDataSequence > ();
}

uno::Sequence < OUString > getWriterChartColumnDescriptions( Reference< lang::XComponent > const & mxComponent )
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
    uno::Sequence< OUString > seriesList = xAnyDescriptionAccess->getColumnDescriptions();
    return seriesList;
}

std::vector<std::vector<double> > getDataSeriesYValuesFromChartType( const Reference<chart2::XChartType>& xCT )
{
    Reference<chart2::XDataSeriesContainer> xDSCont(xCT, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDSCont.is());
    Sequence<uno::Reference<chart2::XDataSeries> > aDataSeriesSeq = xDSCont->getDataSeries();

    double fNan;
    rtl::math::setNan(&fNan);

    std::vector<std::vector<double> > aRet;
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
            if (aRoleName == "values-y")
            {
                uno::Sequence<uno::Any> aData = xValues->getData();
                std::vector<double> aValues;
                aValues.reserve(aData.getLength());
                for (sal_Int32 nVal = 0; nVal < aData.getLength(); ++nVal)
                {
                    double fVal;
                    if (aData[nVal] >>= fVal)
                        aValues.push_back(fVal);
                    else
                        aValues.push_back(fNan);
                }
                aRet.push_back(aValues);
            }
        }
    }

    return aRet;
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

uno::Reference< chart::XChartDocument > ChartTest::getChartDocFromImpress( const OUString& pDir, const char* pName )
{
    mxComponent = loadFromDesktop(m_directories.getURLFromSrc(pDir) + OUString::createFromAscii(pName), "com.sun.star.comp.Draw.PresentationDocument");
    uno::Reference< drawing::XDrawPagesSupplier > xDoc(mxComponent, uno::UNO_QUERY_THROW );
    uno::Reference< drawing::XDrawPage > xPage(
        xDoc->getDrawPages()->getByIndex(0), uno::UNO_QUERY_THROW );
    uno::Reference< beans::XPropertySet > xShapeProps(
        xPage->getByIndex(0), uno::UNO_QUERY );
    CPPUNIT_ASSERT(xShapeProps.is());
    uno::Reference< frame::XModel > xDocModel;
    xShapeProps->getPropertyValue("Model") >>= xDocModel;
    CPPUNIT_ASSERT(xDocModel.is());
    uno::Reference< chart::XChartDocument > xChartDoc( xDocModel, uno::UNO_QUERY_THROW );

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

uno::Reference<chart::XChartDocument> ChartTest::getChartDocFromWriter( sal_Int32 nShape )
{
    // DO NOT use XDrawPageSupplier since SwVirtFlyDrawObj are not created
    // during import, only in layout!
    Reference<text::XTextEmbeddedObjectsSupplier> xEOS(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xEOS.is());
    Reference<container::XIndexAccess> xEmbeddeds(xEOS->getEmbeddedObjects(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xEmbeddeds.is());

    Reference<beans::XPropertySet> xShapeProps(xEmbeddeds->getByIndex(nShape), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xShapeProps.is());

    Reference<frame::XModel> xDocModel;
    xShapeProps->getPropertyValue("Model") >>= xDocModel;
    CPPUNIT_ASSERT(xDocModel.is());

    uno::Reference<chart::XChartDocument> xChartDoc(xDocModel, uno::UNO_QUERY);
    return xChartDoc;
}

uno::Sequence < OUString > ChartTest::getImpressChartColumnDescriptions( const OUString& pDir, const char* pName )
{
    uno::Reference< chart::XChartDocument > xChartDoc = getChartDocFromImpress( pDir, pName );
    uno::Reference< chart::XChartDataArray > xChartData ( xChartDoc->getData(), uno::UNO_QUERY_THROW);
    uno::Sequence < OUString > seriesList = xChartData->getColumnDescriptions();
    return seriesList;
}

OUString getTitleString( const Reference<chart2::XTitled>& xTitled )
{
    uno::Reference<chart2::XTitle> xTitle = xTitled->getTitleObject();
    CPPUNIT_ASSERT(xTitle.is());
    uno::Sequence<uno::Reference<chart2::XFormattedString> > aFSSeq = xTitle->getText();
    OUString aText;
    for (sal_Int32 i = 0; i < aFSSeq.getLength(); ++i)
        aText += aFSSeq[i]->getString();

    return aText;
}

sal_Int32 getNumberFormat( const Reference<chart2::XChartDocument>& xChartDoc, const OUString& sFormat )
{
    Reference<util::XNumberFormatsSupplier> xNFS(xChartDoc, uno::UNO_QUERY_THROW);
    Reference<util::XNumberFormats> xNumberFormats = xNFS->getNumberFormats();
    CPPUNIT_ASSERT(xNumberFormats.is());

    return xNumberFormats->queryKey(sFormat, css::lang::Locale(), false);
}

sal_Int32 getNumberFormatFromAxis( const Reference<chart2::XAxis>& xAxis )
{
    Reference<beans::XPropertySet> xPS(xAxis, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xPS.is());
    sal_Int32 nNumberFormat = -1;
    bool bSuccess = xPS->getPropertyValue(CHART_UNONAME_NUMFMT) >>= nNumberFormat;
    CPPUNIT_ASSERT(bSuccess);

    return nNumberFormat;
}

sal_Int16 getNumberFormatType( const Reference<chart2::XChartDocument>& xChartDoc, sal_Int32 nNumberFormat )
{
    Reference<util::XNumberFormatsSupplier> xNFS(xChartDoc, uno::UNO_QUERY_THROW);
    Reference<util::XNumberFormats> xNumberFormats = xNFS->getNumberFormats();
    CPPUNIT_ASSERT(xNumberFormats.is());

    Reference<beans::XPropertySet> xNumPS = xNumberFormats->getByKey(nNumberFormat);
    CPPUNIT_ASSERT(xNumPS.is());

    sal_Int16 nType = util::NumberFormat::UNDEFINED;
    xNumPS->getPropertyValue("Type") >>= nType;

    return nType;
}

awt::Size ChartTest::getPageSize( const Reference< chart2::XChartDocument > & xChartDoc )
{
    awt::Size aSize( 0, 0 );
    uno::Reference< com::sun::star::embed::XVisualObject > xVisualObject( xChartDoc, uno::UNO_QUERY );
    CPPUNIT_ASSERT( xVisualObject.is() );
    aSize = xVisualObject->getVisualAreaSize( com::sun::star::embed::Aspects::MSOLE_CONTENT );
    return aSize;
}

awt::Size ChartTest::getSize(css::uno::Reference<chart2::XDiagram> xDiagram, const awt::Size& rPageSize)
{
    Reference< beans::XPropertySet > xProp(xDiagram, uno::UNO_QUERY);
    chart2::RelativeSize aRelativeSize;
    xProp->getPropertyValue( "RelativeSize" ) >>= aRelativeSize;
    double fX = aRelativeSize.Primary * rPageSize.Width;
    double fY = aRelativeSize.Secondary * rPageSize.Height;
    awt::Size aSize;
    aSize.Width = static_cast< sal_Int32 >( ::rtl::math::round( fX ) );
    aSize.Height = static_cast< sal_Int32 >( ::rtl::math::round( fY ) );
    return aSize;
}

#endif // INCLUDED_CHART2_QA_EXTRAS_CHARTTEST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
