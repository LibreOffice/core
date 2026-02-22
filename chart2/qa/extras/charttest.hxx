/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <test/unoapi_test.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>

#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/table/XTableChartsSupplier.hpp>
#include <com/sun/star/table/XTableChart.hpp>
#include <com/sun/star/table/XTablePivotChartsSupplier.hpp>
#include <com/sun/star/table/XTablePivotChart.hpp>
#include <com/sun/star/document/XEmbeddedObjectSupplier.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XStorable.hpp>

#include <o3tl/string_view.hxx>
#include <rtl/math.hxx>
#include <svx/charthelper.hxx>

#include <com/sun/star/chart2/AxisType.hpp>
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
#include <com/sun/star/chart2/XInternalDataProvider.hpp>
#include <com/sun/star/chart/XDateCategories.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/chart/XChartDocument.hpp>
#include <com/sun/star/text/XTextEmbeddedObjectsSupplier.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/util/NumberFormat.hpp>
#include <com/sun/star/util/NumberFormatter.hpp>

#include <unonames.hxx>

#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/embed/XVisualObject.hpp>
#include <com/sun/star/chart2/RelativeSize.hpp>

using namespace css;
using namespace css::uno;

class ChartTest : public UnoApiTest
{
public:
    ChartTest(const OUString& path)
        : UnoApiTest(path)
    {
    }


    Reference<lang::XComponent> getChartCompFromSheet(sal_Int32 nSheet, sal_Int32 nChart);
    Reference<chart2::XChartDocument> getChartDocFromSheet(sal_Int32 nSheet);
    uno::Reference<table::XTablePivotCharts> getTablePivotChartsFromSheet(sal_Int32 nSheet);
    Reference<chart2::XChartDocument> getPivotChartDocFromSheet(sal_Int32 nSheet);
    uno::Reference<chart2::XChartDocument>
    getPivotChartDocFromSheet(uno::Reference<table::XTablePivotCharts> const& xTablePivotCharts,
                              sal_Int32 nIndex);
    Reference<chart2::XChartType>
    getChartTypeFromDoc(Reference<chart2::XChartDocument> const& xChartDoc, sal_Int32 nChartType,
                        sal_Int32 nCooSys = 0);
    Reference<chart2::XAxis> getAxisFromDoc(const Reference<chart2::XChartDocument>& xChartDoc,
                                            sal_Int32 nCooSys, sal_Int32 nAxisDim,
                                            sal_Int32 nAxisIndex);
    sal_Int32 getNumberOfDataSeries(uno::Reference<chart2::XChartDocument> const& xChartDoc,
                                    sal_Int32 nChartType = 0, sal_Int32 nCooSys = 0);
    Reference<chart2::XDataSeries>
    getDataSeriesFromDoc(uno::Reference<chart2::XChartDocument> const& xChartDoc,
                         sal_Int32 nDataSeries, sal_Int32 nChartType = 0, sal_Int32 nCooSys = 0);
    Reference<chart2::data::XDataSequence>
    getLabelDataSequenceFromDoc(Reference<chart2::XChartDocument> const& xChartDoc,
                                sal_Int32 nDataSeries = 0, sal_Int32 nChartType = 0);
    Reference<chart2::data::XDataSequence>
    getDataSequenceFromDocByRole(Reference<chart2::XChartDocument> const& xChartDoc,
                                 std::u16string_view rRole, sal_Int32 nDataSeries = 0,
                                 sal_Int32 nChartType = 0);
    uno::Sequence<OUString> getWriterChartColumnDescriptions();
    std::vector<std::vector<double>>
    getDataSeriesYValuesFromChartType(const Reference<chart2::XChartType>& xCT);
    std::vector<uno::Sequence<uno::Any>>
    getDataSeriesLabelsFromChartType(const Reference<chart2::XChartType>& xCT);
    uno::Reference<chart::XChartDocument> getChartDocFromDrawImpress(sal_Int32 nPage,
                                                                     sal_Int32 nShape);
    uno::Reference<chart::XChartDocument> getChartDocFromWriter(sal_Int32 nShape);
    OUString getTitleString(const Reference<chart2::XTitled>& xTitled);
    sal_Int32 getNumberFormatFromAxis(const Reference<chart2::XAxis>& xAxis);
    sal_Int16 getNumberFormatType(const Reference<chart2::XChartDocument>& xChartDoc,
                                  sal_Int32 nNumberFormat);
    uno::Reference<drawing::XShape>
    getShapeByName(const uno::Reference<drawing::XShapes>& rShapes, const OUString& rName,
                   const std::function<bool(const uno::Reference<drawing::XShape>&)>& pCondition
                   = nullptr);
    Reference<chart2::XChartStyle>
        getStyleFromDoc(Reference<chart2::XChartDocument> const& xChartDoc);
};

Reference< lang::XComponent > ChartTest::getChartCompFromSheet( sal_Int32 nSheet, sal_Int32 nChart )
{
    // let us assume that we only have one chart per sheet

    uno::Reference< sheet::XSpreadsheetDocument > xDoc(mxComponent, UNO_QUERY_THROW);

    uno::Reference< container::XIndexAccess > xIA(xDoc->getSheets(), UNO_QUERY_THROW);

    uno::Reference< table::XTableChartsSupplier > xChartSupplier( xIA->getByIndex(nSheet), UNO_QUERY_THROW);

    uno::Reference< table::XTableCharts > xCharts = xChartSupplier->getCharts();
    CPPUNIT_ASSERT(xCharts.is());

    uno::Reference< container::XIndexAccess > xIACharts(xCharts, UNO_QUERY_THROW);
    uno::Reference< table::XTableChart > xChart( xIACharts->getByIndex(nChart), UNO_QUERY_THROW);

    uno::Reference< document::XEmbeddedObjectSupplier > xEmbObjectSupplier(xChart, UNO_QUERY_THROW);

    uno::Reference< lang::XComponent > xChartComp( xEmbObjectSupplier->getEmbeddedObject(), UNO_SET_THROW );

    return xChartComp;

}

Reference< chart2::XChartDocument > ChartTest::getChartDocFromSheet( sal_Int32 nSheet )
{
    uno::Reference< chart2::XChartDocument > xChartDoc ( getChartCompFromSheet(nSheet, 0), UNO_QUERY_THROW );

    // Update the chart view, so that its draw page is updated and ready for the test
    css::uno::Reference<css::frame::XModel> xModel(xChartDoc, css::uno::UNO_QUERY_THROW);
    ChartHelper::updateChart(xModel);

    return xChartDoc;
}

uno::Reference<table::XTablePivotCharts> ChartTest::getTablePivotChartsFromSheet(sal_Int32 nSheet)
{
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, UNO_QUERY_THROW);

    uno::Reference<container::XIndexAccess> xIA(xDoc->getSheets(), UNO_QUERY_THROW);

    uno::Reference<table::XTablePivotChartsSupplier> xChartSupplier(xIA->getByIndex(nSheet), UNO_QUERY_THROW);

    uno::Reference<table::XTablePivotCharts> xTablePivotCharts = xChartSupplier->getPivotCharts();
    CPPUNIT_ASSERT(xTablePivotCharts.is());

    return xTablePivotCharts;
}

Reference<chart2::XChartDocument> ChartTest::getPivotChartDocFromSheet(sal_Int32 nSheet)
{
    uno::Reference<table::XTablePivotCharts> xTablePivotCharts = getTablePivotChartsFromSheet(nSheet);

    uno::Reference<container::XIndexAccess> xIACharts(xTablePivotCharts, UNO_QUERY_THROW);
    uno::Reference<table::XTablePivotChart> xTablePivotChart(xIACharts->getByIndex(0), UNO_QUERY_THROW);

    uno::Reference<document::XEmbeddedObjectSupplier> xEmbObjectSupplier(xTablePivotChart, UNO_QUERY_THROW);

    uno::Reference<lang::XComponent> xChartComp(xEmbObjectSupplier->getEmbeddedObject(), UNO_SET_THROW);
    uno::Reference<chart2::XChartDocument> xChartDoc(xChartComp, UNO_QUERY_THROW);
    return xChartDoc;
}

Reference<chart2::XChartDocument> ChartTest::getPivotChartDocFromSheet(uno::Reference<table::XTablePivotCharts> const & xTablePivotCharts, sal_Int32 nIndex)
{
    uno::Reference<container::XIndexAccess> xIACharts(xTablePivotCharts, UNO_QUERY_THROW);
    uno::Reference<table::XTablePivotChart> xTablePivotChart(xIACharts->getByIndex(nIndex), UNO_QUERY_THROW);

    uno::Reference<document::XEmbeddedObjectSupplier> xEmbObjectSupplier(xTablePivotChart, UNO_QUERY_THROW);

    uno::Reference<lang::XComponent> xChartComp(xEmbObjectSupplier->getEmbeddedObject(), UNO_SET_THROW);

    uno::Reference<chart2::XChartDocument> xChartDoc(xChartComp, UNO_QUERY_THROW);
    return xChartDoc;
}

Reference< chart2::XChartType > ChartTest::getChartTypeFromDoc( Reference< chart2::XChartDocument > const & xChartDoc,
                                                                sal_Int32 nChartType, sal_Int32 nCooSys )
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

Reference<chart2::XAxis> ChartTest::getAxisFromDoc(
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

sal_Int32 ChartTest::getNumberOfDataSeries(uno::Reference<chart2::XChartDocument> const & xChartDoc,
                                sal_Int32 nChartType, sal_Int32 nCooSys)
{
    Reference<chart2::XChartType> xChartType = getChartTypeFromDoc(xChartDoc, nChartType, nCooSys);
    Reference<chart2::XDataSeriesContainer> xDataSeriesContainer(xChartType, UNO_QUERY_THROW);

    uno::Sequence<uno::Reference<chart2::XDataSeries>> xSeriesSequence(xDataSeriesContainer->getDataSeries());
    return xSeriesSequence.getLength();
}

Reference< chart2::XDataSeries > ChartTest::getDataSeriesFromDoc(uno::Reference<chart2::XChartDocument> const & xChartDoc,
                                                      sal_Int32 nDataSeries, sal_Int32 nChartType,
                                                      sal_Int32 nCooSys)
{
    Reference< chart2::XChartType > xChartType = getChartTypeFromDoc( xChartDoc, nChartType, nCooSys );
    Reference< chart2::XDataSeriesContainer > xDataSeriesContainer( xChartType, UNO_QUERY_THROW );

    Sequence< Reference< chart2::XDataSeries > > xSeriesSequence( xDataSeriesContainer->getDataSeries() );
    CPPUNIT_ASSERT( xSeriesSequence.getLength() > nDataSeries );

    Reference< chart2::XDataSeries > xSeries = xSeriesSequence[nDataSeries];

    return xSeries;
}

Reference< chart2::data::XDataSequence > ChartTest::getLabelDataSequenceFromDoc(
        Reference< chart2::XChartDocument > const & xChartDoc,
        sal_Int32 nDataSeries, sal_Int32 nChartType )
{
    Reference< chart2::XDataSeries > xDataSeries =
        getDataSeriesFromDoc( xChartDoc, nDataSeries, nChartType );
    CPPUNIT_ASSERT(xDataSeries.is());
    Reference< chart2::data::XDataSource > xDataSource( xDataSeries, uno::UNO_QUERY_THROW );
    const Sequence< Reference< chart2::data::XLabeledDataSequence > > xDataSequences =
        xDataSource->getDataSequences();
    for(auto const & lds : xDataSequences)
    {
        Reference< chart2::data::XDataSequence> xLabelSeq = lds->getLabel();
        if(!xLabelSeq.is())
            continue;

        return xLabelSeq;
    }

    CPPUNIT_FAIL("no Label sequence found");
}

Reference< chart2::data::XDataSequence > ChartTest::getDataSequenceFromDocByRole(
        Reference< chart2::XChartDocument > const & xChartDoc, std::u16string_view rRole,
        sal_Int32 nDataSeries, sal_Int32 nChartType )
{
    Reference< chart2::XDataSeries > xDataSeries =
        getDataSeriesFromDoc( xChartDoc, nDataSeries, nChartType );
    CPPUNIT_ASSERT(xDataSeries.is());
    Reference< chart2::data::XDataSource > xDataSource( xDataSeries, uno::UNO_QUERY_THROW );
    const Sequence< Reference< chart2::data::XLabeledDataSequence > > xDataSequences =
        xDataSource->getDataSequences();
    for(auto const & lds : xDataSequences)
    {
        Reference< chart2::data::XDataSequence> xLabelSeq = lds->getValues();
        uno::Reference< beans::XPropertySet > xProps(xLabelSeq, uno::UNO_QUERY);
        if(!xProps.is())
            continue;

        OUString aRoleName = xProps->getPropertyValue(u"Role"_ustr).get<OUString>();

        if(aRoleName == rRole)
            return xLabelSeq;
    }

    return Reference< chart2::data::XDataSequence > ();
}

uno::Sequence < OUString > ChartTest::getWriterChartColumnDescriptions()
{
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    uno::Reference<drawing::XShape> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT( xShape.is() );
    uno::Reference<beans::XPropertySet> xPropertySet(xShape, uno::UNO_QUERY);
    uno::Reference< chart2::XChartDocument > xChartDoc;
    xChartDoc.set( xPropertySet->getPropertyValue( u"Model"_ustr ), uno::UNO_QUERY );
    CPPUNIT_ASSERT( xChartDoc.is() );
    CPPUNIT_ASSERT( xChartDoc->getDataProvider().is() );
    uno::Reference< chart2::XAnyDescriptionAccess > xAnyDescriptionAccess ( xChartDoc->getDataProvider(), uno::UNO_QUERY_THROW );
    uno::Sequence< OUString > seriesList = xAnyDescriptionAccess->getColumnDescriptions();
    return seriesList;
}

std::vector<std::vector<double> > ChartTest::getDataSeriesYValuesFromChartType( const Reference<chart2::XChartType>& xCT )
{
    Reference<chart2::XDataSeriesContainer> xDSCont(xCT, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDSCont.is());
    const Sequence<uno::Reference<chart2::XDataSeries> > aDataSeriesSeq = xDSCont->getDataSeries();

    std::vector<std::vector<double> > aRet;
    for (uno::Reference<chart2::XDataSeries> const & ds : aDataSeriesSeq)
    {
        uno::Reference<chart2::data::XDataSource> xDSrc(ds, uno::UNO_QUERY);
        CPPUNIT_ASSERT(xDSrc.is());
        const uno::Sequence<Reference<chart2::data::XLabeledDataSequence> > aDataSeqs = xDSrc->getDataSequences();
        for (auto const & lds : aDataSeqs)
        {
            Reference<chart2::data::XDataSequence> xValues = lds->getValues();
            CPPUNIT_ASSERT(xValues.is());
            Reference<beans::XPropertySet> xPropSet(xValues, uno::UNO_QUERY);
            if (!xPropSet.is())
                continue;

            OUString aRoleName;
            xPropSet->getPropertyValue(u"Role"_ustr) >>= aRoleName;
            if (aRoleName == "values-y")
            {
                const uno::Sequence<uno::Any> aData = xValues->getData();
                std::vector<double> aValues;
                aValues.reserve(aData.getLength());
                for (uno::Any const & any : aData)
                {
                    double fVal;
                    if (any >>= fVal)
                        aValues.push_back(fVal);
                    else
                        aValues.push_back(std::numeric_limits<double>::quiet_NaN());
                }
                aRet.push_back(aValues);
            }
        }
    }

    return aRet;
}

std::vector<uno::Sequence<uno::Any> > ChartTest::getDataSeriesLabelsFromChartType( const Reference<chart2::XChartType>& xCT )
{
    OUString aLabelRole = xCT->getRoleOfSequenceForSeriesLabel();

    Reference<chart2::XDataSeriesContainer> xDSCont(xCT, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDSCont.is());
    const Sequence<uno::Reference<chart2::XDataSeries> > aDataSeriesSeq = xDSCont->getDataSeries();

    std::vector<uno::Sequence<uno::Any> > aRet;
    for (auto const & ds : aDataSeriesSeq)
    {
        uno::Reference<chart2::data::XDataSource> xDSrc(ds, uno::UNO_QUERY);
        CPPUNIT_ASSERT(xDSrc.is());
        const uno::Sequence<Reference<chart2::data::XLabeledDataSequence> > aDataSeqs = xDSrc->getDataSequences();
        for (auto const & lds : aDataSeqs)
        {
            Reference<chart2::data::XDataSequence> xValues = lds->getValues();
            CPPUNIT_ASSERT(xValues.is());
            Reference<beans::XPropertySet> xPropSet(xValues, uno::UNO_QUERY);
            if (!xPropSet.is())
                continue;

            OUString aRoleName;
            xPropSet->getPropertyValue(u"Role"_ustr) >>= aRoleName;
            if (aRoleName == aLabelRole)
            {
                Reference<chart2::data::XLabeledDataSequence> xLabel = lds;
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
    xShapeProps->getPropertyValue(u"Model"_ustr) >>= xDocModel;
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
    xShapeProps->getPropertyValue(u"Model"_ustr) >>= xDocModel;
    CPPUNIT_ASSERT(xDocModel.is());

    uno::Reference<chart::XChartDocument> xChartDoc(xDocModel, uno::UNO_QUERY);
    return xChartDoc;
}

OUString ChartTest::getTitleString( const Reference<chart2::XTitled>& xTitled )
{
    uno::Reference<chart2::XTitle> xTitle = xTitled->getTitleObject();
    CPPUNIT_ASSERT(xTitle.is());
    const uno::Sequence<uno::Reference<chart2::XFormattedString> > aFSSeq = xTitle->getText();
    OUStringBuffer aText;
    for (auto const & fs : aFSSeq)
        aText.append(fs->getString());

    return aText.makeStringAndClear();
}

sal_Int32 ChartTest::getNumberFormatFromAxis( const Reference<chart2::XAxis>& xAxis )
{
    Reference<beans::XPropertySet> xPS(xAxis, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xPS.is());
    sal_Int32 nNumberFormat = -1;
    bool bSuccess = xPS->getPropertyValue(CHART_UNONAME_NUMFMT) >>= nNumberFormat;
    CPPUNIT_ASSERT(bSuccess);

    return nNumberFormat;
}

sal_Int16 ChartTest::getNumberFormatType( const Reference<chart2::XChartDocument>& xChartDoc, sal_Int32 nNumberFormat )
{
    Reference<util::XNumberFormatsSupplier> xNFS(xChartDoc, uno::UNO_QUERY_THROW);
    Reference<util::XNumberFormats> xNumberFormats = xNFS->getNumberFormats();
    CPPUNIT_ASSERT(xNumberFormats.is());

    Reference<beans::XPropertySet> xNumPS = xNumberFormats->getByKey(nNumberFormat);
    CPPUNIT_ASSERT(xNumPS.is());

    sal_Int16 nType = util::NumberFormat::UNDEFINED;
    xNumPS->getPropertyValue(u"Type"_ustr) >>= nType;

    return nType;
}

uno::Reference<drawing::XShape>
ChartTest::getShapeByName(const uno::Reference<drawing::XShapes>& rShapes, const OUString& rName,
               const std::function<bool(const uno::Reference<drawing::XShape>&)>& pCondition)
{
    for (sal_Int32 i = 0; i < rShapes->getCount(); ++i)
    {
        uno::Reference<drawing::XShapes> xShapes(rShapes->getByIndex(i), uno::UNO_QUERY);
        if (xShapes.is())
        {
            uno::Reference<drawing::XShape> xRet = getShapeByName(xShapes, rName, pCondition);
            if (xRet.is())
                return xRet;
        }
        uno::Reference<container::XNamed> xNamedShape(rShapes->getByIndex(i), uno::UNO_QUERY);
        if (xNamedShape->getName() == rName)
        {
            uno::Reference<drawing::XShape> xShape(xNamedShape, uno::UNO_QUERY);
            if (pCondition == nullptr || pCondition(xShape))
                return xShape;
        }
    }
    return uno::Reference<drawing::XShape>();
}

Reference<chart2::XChartStyle>
    getStyleFromDoc(Reference<chart2::XChartDocument> const& xChartDoc)
{
    CPPUNIT_ASSERT( xChartDoc.is() );

    Reference <chart2::XChartStyle > xStyle = xChartDoc->getStyles();
    CPPUNIT_ASSERT( xStyle.is() );

    return xStyle;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
