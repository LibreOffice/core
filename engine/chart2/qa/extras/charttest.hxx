/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <test/unoapixml_test.hxx>
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

class ChartTest : public UnoApiXmlTest
{
public:
    ChartTest(const OUString& path)
        : UnoApiXmlTest(path)
    {
    }


    css::uno::Reference<css::lang::XComponent> getChartCompFromSheet(sal_Int32 nSheet, sal_Int32 nChart);
    css::uno::Reference<css::chart2::XChartDocument> getChartDocFromSheet(sal_Int32 nSheet);
    css::uno::Reference<css::table::XTablePivotCharts> getTablePivotChartsFromSheet(sal_Int32 nSheet);
    css::uno::Reference<css::chart2::XChartDocument> getPivotChartDocFromSheet(sal_Int32 nSheet);
    css::uno::Reference<css::chart2::XChartDocument>
    getPivotChartDocFromSheet(css::uno::Reference<css::table::XTablePivotCharts> const& xTablePivotCharts,
                              sal_Int32 nIndex);
    css::uno::Reference<css::chart2::XChartType>
    getChartTypeFromDoc(css::uno::Reference<css::chart2::XChartDocument> const& xChartDoc, sal_Int32 nChartType,
                        sal_Int32 nCooSys = 0);
    css::uno::Reference<css::chart2::XAxis> getAxisFromDoc(const css::uno::Reference<css::chart2::XChartDocument>& xChartDoc,
                                            sal_Int32 nCooSys, sal_Int32 nAxisDim,
                                            sal_Int32 nAxisIndex);
    sal_Int32 getNumberOfDataSeries(css::uno::Reference<css::chart2::XChartDocument> const& xChartDoc,
                                    sal_Int32 nChartType = 0, sal_Int32 nCooSys = 0);
    css::uno::Reference<css::chart2::XDataSeries>
    getDataSeriesFromDoc(css::uno::Reference<css::chart2::XChartDocument> const& xChartDoc,
                         sal_Int32 nDataSeries, sal_Int32 nChartType = 0, sal_Int32 nCooSys = 0);
    css::uno::Reference<css::chart2::data::XDataSequence>
    getLabelDataSequenceFromDoc(css::uno::Reference<css::chart2::XChartDocument> const& xChartDoc,
                                sal_Int32 nDataSeries = 0, sal_Int32 nChartType = 0);
    css::uno::Reference<css::chart2::data::XDataSequence>
    getDataSequenceFromDocByRole(css::uno::Reference<css::chart2::XChartDocument> const& xChartDoc,
                                 std::u16string_view rRole, sal_Int32 nDataSeries = 0,
                                 sal_Int32 nChartType = 0);
    css::uno::Sequence<OUString> getWriterChartColumnDescriptions();
    std::vector<std::vector<double>>
    getDataSeriesYValuesFromChartType(const css::uno::Reference<css::chart2::XChartType>& xCT);
    std::vector<css::uno::Sequence<css::uno::Any>>
    getDataSeriesLabelsFromChartType(const css::uno::Reference<css::chart2::XChartType>& xCT);
    css::uno::Reference<css::chart::XChartDocument> getChartDocFromDrawImpress(sal_Int32 nPage,
                                                                     sal_Int32 nShape);
    css::uno::Reference<css::chart::XChartDocument> getChartDocFromWriter(sal_Int32 nShape);
    OUString getTitleString(const css::uno::Reference<css::chart2::XTitled>& xTitled);
    sal_Int32 getNumberFormatFromAxis(const css::uno::Reference<css::chart2::XAxis>& xAxis);
    sal_Int16 getNumberFormatType(const css::uno::Reference<css::chart2::XChartDocument>& xChartDoc,
                                  sal_Int32 nNumberFormat);
    css::uno::Reference<css::drawing::XShape>
    getShapeByName(const css::uno::Reference<css::drawing::XShapes>& rShapes, const OUString& rName,
                   const std::function<bool(const css::uno::Reference<css::drawing::XShape>&)>& pCondition
                   = nullptr);
};

css::uno::Reference< css::lang::XComponent > ChartTest::getChartCompFromSheet( sal_Int32 nSheet, sal_Int32 nChart )
{
    // let us assume that we only have one chart per sheet

    css::uno::Reference< css::sheet::XSpreadsheetDocument > xDoc(mxComponent, css::uno::UNO_QUERY_THROW);

    css::uno::Reference< css::container::XIndexAccess > xIA(xDoc->getSheets(), css::uno::UNO_QUERY_THROW);

    css::uno::Reference< css::table::XTableChartsSupplier > xChartSupplier( xIA->getByIndex(nSheet), css::uno::UNO_QUERY_THROW);

    css::uno::Reference< css::table::XTableCharts > xCharts = xChartSupplier->getCharts();
    CPPUNIT_ASSERT(xCharts.is());

    css::uno::Reference< css::container::XIndexAccess > xIACharts(xCharts, css::uno::UNO_QUERY_THROW);
    css::uno::Reference< css::table::XTableChart > xChart( xIACharts->getByIndex(nChart), css::uno::UNO_QUERY_THROW);

    css::uno::Reference< css::document::XEmbeddedObjectSupplier > xEmbObjectSupplier(xChart, css::uno::UNO_QUERY_THROW);

    css::uno::Reference< css::lang::XComponent > xChartComp( xEmbObjectSupplier->getEmbeddedObject(), css::uno::UNO_SET_THROW );

    return xChartComp;

}

css::uno::Reference< css::chart2::XChartDocument > ChartTest::getChartDocFromSheet( sal_Int32 nSheet )
{
    css::uno::Reference< css::chart2::XChartDocument > xChartDoc ( getChartCompFromSheet(nSheet, 0), css::uno::UNO_QUERY_THROW );

    // Update the chart view, so that its draw page is updated and ready for the test
    css::uno::Reference<css::frame::XModel> xModel(xChartDoc, css::uno::UNO_QUERY_THROW);
    ChartHelper::updateChart(xModel);

    return xChartDoc;
}

css::uno::Reference<css::table::XTablePivotCharts> ChartTest::getTablePivotChartsFromSheet(sal_Int32 nSheet)
{
    css::uno::Reference<css::sheet::XSpreadsheetDocument> xDoc(mxComponent, css::uno::UNO_QUERY_THROW);

    css::uno::Reference<css::container::XIndexAccess> xIA(xDoc->getSheets(), css::uno::UNO_QUERY_THROW);

    css::uno::Reference<css::table::XTablePivotChartsSupplier> xChartSupplier(xIA->getByIndex(nSheet), css::uno::UNO_QUERY_THROW);

    css::uno::Reference<css::table::XTablePivotCharts> xTablePivotCharts = xChartSupplier->getPivotCharts();
    CPPUNIT_ASSERT(xTablePivotCharts.is());

    return xTablePivotCharts;
}

css::uno::Reference<css::chart2::XChartDocument> ChartTest::getPivotChartDocFromSheet(sal_Int32 nSheet)
{
    css::uno::Reference<css::table::XTablePivotCharts> xTablePivotCharts = getTablePivotChartsFromSheet(nSheet);

    css::uno::Reference<css::container::XIndexAccess> xIACharts(xTablePivotCharts, css::uno::UNO_QUERY_THROW);
    css::uno::Reference<css::table::XTablePivotChart> xTablePivotChart(xIACharts->getByIndex(0), css::uno::UNO_QUERY_THROW);

    css::uno::Reference<css::document::XEmbeddedObjectSupplier> xEmbObjectSupplier(xTablePivotChart, css::uno::UNO_QUERY_THROW);

    css::uno::Reference<css::lang::XComponent> xChartComp(xEmbObjectSupplier->getEmbeddedObject(), css::uno::UNO_SET_THROW);
    css::uno::Reference<css::chart2::XChartDocument> xChartDoc(xChartComp, css::uno::UNO_QUERY_THROW);
    return xChartDoc;
}

css::uno::Reference<css::chart2::XChartDocument> ChartTest::getPivotChartDocFromSheet(css::uno::Reference<css::table::XTablePivotCharts> const & xTablePivotCharts, sal_Int32 nIndex)
{
    css::uno::Reference<css::container::XIndexAccess> xIACharts(xTablePivotCharts, css::uno::UNO_QUERY_THROW);
    css::uno::Reference<css::table::XTablePivotChart> xTablePivotChart(xIACharts->getByIndex(nIndex), css::uno::UNO_QUERY_THROW);

    css::uno::Reference<css::document::XEmbeddedObjectSupplier> xEmbObjectSupplier(xTablePivotChart, css::uno::UNO_QUERY_THROW);

    css::uno::Reference<css::lang::XComponent> xChartComp(xEmbObjectSupplier->getEmbeddedObject(), css::uno::UNO_SET_THROW);

    css::uno::Reference<css::chart2::XChartDocument> xChartDoc(xChartComp, css::uno::UNO_QUERY_THROW);
    return xChartDoc;
}

css::uno::Reference< css::chart2::XChartType > ChartTest::getChartTypeFromDoc( css::uno::Reference< css::chart2::XChartDocument > const & xChartDoc,
                                                                sal_Int32 nChartType, sal_Int32 nCooSys )
{
    CPPUNIT_ASSERT( xChartDoc.is() );

    css::uno::Reference<css::chart2::XDiagram > xDiagram = xChartDoc->getFirstDiagram();
    CPPUNIT_ASSERT( xDiagram.is() );

    css::uno::Reference< css::chart2::XCoordinateSystemContainer > xCooSysContainer( xDiagram, css::uno::UNO_QUERY_THROW );

    css::uno::Sequence< css::uno::Reference< css::chart2::XCoordinateSystem > > xCooSysSequence( xCooSysContainer->getCoordinateSystems());
    CPPUNIT_ASSERT( xCooSysSequence.getLength() > nCooSys );

    css::uno::Reference< css::chart2::XChartTypeContainer > xChartTypeContainer( xCooSysSequence[nCooSys], css::uno::UNO_QUERY_THROW );

    css::uno::Sequence< css::uno::Reference< css::chart2::XChartType > > xChartTypeSequence( xChartTypeContainer->getChartTypes() );
    CPPUNIT_ASSERT( xChartTypeSequence.getLength() > nChartType );

    return xChartTypeSequence[nChartType];
}

css::uno::Reference<css::chart2::XAxis> ChartTest::getAxisFromDoc(
    const css::uno::Reference<css::chart2::XChartDocument>& xChartDoc, sal_Int32 nCooSys, sal_Int32 nAxisDim, sal_Int32 nAxisIndex )
{
    css::uno::Reference<css::chart2::XDiagram> xDiagram = xChartDoc->getFirstDiagram();
    CPPUNIT_ASSERT(xDiagram.is());

    css::uno::Reference<css::chart2::XCoordinateSystemContainer> xCooSysContainer(xDiagram, css::uno::UNO_QUERY_THROW);

    css::uno::Sequence<css::uno::Reference<css::chart2::XCoordinateSystem> > xCooSysSequence = xCooSysContainer->getCoordinateSystems();
    CPPUNIT_ASSERT(xCooSysSequence.getLength() > nCooSys);

    css::uno::Reference<css::chart2::XCoordinateSystem> xCoord = xCooSysSequence[nCooSys];
    CPPUNIT_ASSERT(xCoord.is());

    css::uno::Reference<css::chart2::XAxis> xAxis = xCoord->getAxisByDimension(nAxisDim, nAxisIndex);
    CPPUNIT_ASSERT(xAxis.is());

    return xAxis;
}

sal_Int32 ChartTest::getNumberOfDataSeries(css::uno::Reference<css::chart2::XChartDocument> const & xChartDoc,
                                sal_Int32 nChartType, sal_Int32 nCooSys)
{
    css::uno::Reference<css::chart2::XChartType> xChartType = getChartTypeFromDoc(xChartDoc, nChartType, nCooSys);
    css::uno::Reference<css::chart2::XDataSeriesContainer> xDataSeriesContainer(xChartType, css::uno::UNO_QUERY_THROW);

    css::uno::Sequence<css::uno::Reference<css::chart2::XDataSeries>> xSeriesSequence(xDataSeriesContainer->getDataSeries());
    return xSeriesSequence.getLength();
}

css::uno::Reference< css::chart2::XDataSeries > ChartTest::getDataSeriesFromDoc(css::uno::Reference<css::chart2::XChartDocument> const & xChartDoc,
                                                      sal_Int32 nDataSeries, sal_Int32 nChartType,
                                                      sal_Int32 nCooSys)
{
    css::uno::Reference< css::chart2::XChartType > xChartType = getChartTypeFromDoc( xChartDoc, nChartType, nCooSys );
    css::uno::Reference< css::chart2::XDataSeriesContainer > xDataSeriesContainer( xChartType, css::uno::UNO_QUERY_THROW );

    css::uno::Sequence< css::uno::Reference< css::chart2::XDataSeries > > xSeriesSequence( xDataSeriesContainer->getDataSeries() );
    CPPUNIT_ASSERT( xSeriesSequence.getLength() > nDataSeries );

    css::uno::Reference< css::chart2::XDataSeries > xSeries = xSeriesSequence[nDataSeries];

    return xSeries;
}

css::uno::Reference< css::chart2::data::XDataSequence > ChartTest::getLabelDataSequenceFromDoc(
        css::uno::Reference< css::chart2::XChartDocument > const & xChartDoc,
        sal_Int32 nDataSeries, sal_Int32 nChartType )
{
    css::uno::Reference< css::chart2::XDataSeries > xDataSeries =
        getDataSeriesFromDoc( xChartDoc, nDataSeries, nChartType );
    CPPUNIT_ASSERT(xDataSeries.is());
    css::uno::Reference< css::chart2::data::XDataSource > xDataSource( xDataSeries, css::uno::UNO_QUERY_THROW );
    const css::uno::Sequence< css::uno::Reference< css::chart2::data::XLabeledDataSequence > > xDataSequences =
        xDataSource->getDataSequences();
    for(auto const & lds : xDataSequences)
    {
        css::uno::Reference< css::chart2::data::XDataSequence> xLabelSeq = lds->getLabel();
        if(!xLabelSeq.is())
            continue;

        return xLabelSeq;
    }

    CPPUNIT_FAIL("no Label sequence found");
}

css::uno::Reference< css::chart2::data::XDataSequence > ChartTest::getDataSequenceFromDocByRole(
        css::uno::Reference< css::chart2::XChartDocument > const & xChartDoc, std::u16string_view rRole,
        sal_Int32 nDataSeries, sal_Int32 nChartType )
{
    css::uno::Reference< css::chart2::XDataSeries > xDataSeries =
        getDataSeriesFromDoc( xChartDoc, nDataSeries, nChartType );
    CPPUNIT_ASSERT(xDataSeries.is());
    css::uno::Reference< css::chart2::data::XDataSource > xDataSource( xDataSeries, css::uno::UNO_QUERY_THROW );
    const css::uno::Sequence< css::uno::Reference< css::chart2::data::XLabeledDataSequence > > xDataSequences =
        xDataSource->getDataSequences();
    for(auto const & lds : xDataSequences)
    {
        css::uno::Reference< css::chart2::data::XDataSequence> xLabelSeq = lds->getValues();
        css::uno::Reference< css::beans::XPropertySet > xProps(xLabelSeq, css::uno::UNO_QUERY);
        if(!xProps.is())
            continue;

        OUString aRoleName = xProps->getPropertyValue(u"Role"_ustr).get<OUString>();

        if(aRoleName == rRole)
            return xLabelSeq;
    }

    return css::uno::Reference< css::chart2::data::XDataSequence > ();
}

css::uno::Sequence < OUString > ChartTest::getWriterChartColumnDescriptions()
{
    css::uno::Reference<css::drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, css::uno::UNO_QUERY);
    css::uno::Reference<css::drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    css::uno::Reference<css::drawing::XShape> xShape(xDrawPage->getByIndex(0), css::uno::UNO_QUERY);
    CPPUNIT_ASSERT( xShape.is() );
    css::uno::Reference<css::beans::XPropertySet> xPropertySet(xShape, css::uno::UNO_QUERY);
    css::uno::Reference< css::chart2::XChartDocument > xChartDoc;
    xChartDoc.set( xPropertySet->getPropertyValue( u"Model"_ustr ), css::uno::UNO_QUERY );
    CPPUNIT_ASSERT( xChartDoc.is() );
    CPPUNIT_ASSERT( xChartDoc->getDataProvider().is() );
    css::uno::Reference< css::chart2::XAnyDescriptionAccess > xAnyDescriptionAccess ( xChartDoc->getDataProvider(), css::uno::UNO_QUERY_THROW );
    css::uno::Sequence< OUString > seriesList = xAnyDescriptionAccess->getColumnDescriptions();
    return seriesList;
}

std::vector<std::vector<double> > ChartTest::getDataSeriesYValuesFromChartType( const css::uno::Reference<css::chart2::XChartType>& xCT )
{
    css::uno::Reference<css::chart2::XDataSeriesContainer> xDSCont(xCT, css::uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDSCont.is());
    const css::uno::Sequence<css::uno::Reference<css::chart2::XDataSeries> > aDataSeriesSeq = xDSCont->getDataSeries();

    std::vector<std::vector<double> > aRet;
    for (css::uno::Reference<css::chart2::XDataSeries> const & ds : aDataSeriesSeq)
    {
        css::uno::Reference<css::chart2::data::XDataSource> xDSrc(ds, css::uno::UNO_QUERY);
        CPPUNIT_ASSERT(xDSrc.is());
        const css::uno::Sequence<css::uno::Reference<css::chart2::data::XLabeledDataSequence> > aDataSeqs = xDSrc->getDataSequences();
        for (auto const & lds : aDataSeqs)
        {
            css::uno::Reference<css::chart2::data::XDataSequence> xValues = lds->getValues();
            CPPUNIT_ASSERT(xValues.is());
            css::uno::Reference<css::beans::XPropertySet> xPropSet(xValues, css::uno::UNO_QUERY);
            if (!xPropSet.is())
                continue;

            OUString aRoleName;
            xPropSet->getPropertyValue(u"Role"_ustr) >>= aRoleName;
            if (aRoleName == "values-y")
            {
                const css::uno::Sequence<css::uno::Any> aData = xValues->getData();
                std::vector<double> aValues;
                aValues.reserve(aData.getLength());
                for (css::uno::Any const & any : aData)
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

std::vector<css::uno::Sequence<css::uno::Any> > ChartTest::getDataSeriesLabelsFromChartType( const css::uno::Reference<css::chart2::XChartType>& xCT )
{
    OUString aLabelRole = xCT->getRoleOfSequenceForSeriesLabel();

    css::uno::Reference<css::chart2::XDataSeriesContainer> xDSCont(xCT, css::uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDSCont.is());
    const css::uno::Sequence<css::uno::Reference<css::chart2::XDataSeries> > aDataSeriesSeq = xDSCont->getDataSeries();

    std::vector<css::uno::Sequence<css::uno::Any> > aRet;
    for (auto const & ds : aDataSeriesSeq)
    {
        css::uno::Reference<css::chart2::data::XDataSource> xDSrc(ds, css::uno::UNO_QUERY);
        CPPUNIT_ASSERT(xDSrc.is());
        const css::uno::Sequence<css::uno::Reference<css::chart2::data::XLabeledDataSequence> > aDataSeqs = xDSrc->getDataSequences();
        for (auto const & lds : aDataSeqs)
        {
            css::uno::Reference<css::chart2::data::XDataSequence> xValues = lds->getValues();
            CPPUNIT_ASSERT(xValues.is());
            css::uno::Reference<css::beans::XPropertySet> xPropSet(xValues, css::uno::UNO_QUERY);
            if (!xPropSet.is())
                continue;

            OUString aRoleName;
            xPropSet->getPropertyValue(u"Role"_ustr) >>= aRoleName;
            if (aRoleName == aLabelRole)
            {
                css::uno::Reference<css::chart2::data::XLabeledDataSequence> xLabel = lds;
                CPPUNIT_ASSERT(xLabel.is());
                css::uno::Reference<css::chart2::data::XDataSequence> xDS2 = xLabel->getLabel();
                CPPUNIT_ASSERT(xDS2.is());
                css::uno::Sequence<css::uno::Any> aData = xDS2->getData();
                aRet.push_back(aData);
            }
        }
    }

    return aRet;
}

css::uno::Reference<css::chart::XChartDocument> ChartTest::getChartDocFromDrawImpress(
    sal_Int32 nPage, sal_Int32 nShape )
{
    css::uno::Reference<css::chart::XChartDocument> xEmpty;

    css::uno::Reference<css::drawing::XDrawPagesSupplier> xPages(mxComponent, css::uno::UNO_QUERY);
    if (!xPages.is())
        return xEmpty;

    css::uno::Reference<css::drawing::XDrawPage> xPage(
        xPages->getDrawPages()->getByIndex(nPage), css::uno::UNO_QUERY_THROW);

    css::uno::Reference<css::beans::XPropertySet> xShapeProps(xPage->getByIndex(nShape), css::uno::UNO_QUERY);
    if (!xShapeProps.is())
        return xEmpty;

    css::uno::Reference<css::frame::XModel> xDocModel;
    xShapeProps->getPropertyValue(u"Model"_ustr) >>= xDocModel;
    if (!xDocModel.is())
        return xEmpty;

    css::uno::Reference<css::chart::XChartDocument> xChartDoc(xDocModel, css::uno::UNO_QUERY);
    return xChartDoc;
}


css::uno::Reference<css::chart::XChartDocument> ChartTest::getChartDocFromWriter( sal_Int32 nShape )
{
    // DO NOT use XDrawPageSupplier since SwVirtFlyDrawObj are not created
    // during import, only in layout!
    css::uno::Reference<css::text::XTextEmbeddedObjectsSupplier> xEOS(mxComponent, css::uno::UNO_QUERY);
    CPPUNIT_ASSERT(xEOS.is());
    css::uno::Reference<css::container::XIndexAccess> xEmbeddeds(xEOS->getEmbeddedObjects(), css::uno::UNO_QUERY);
    CPPUNIT_ASSERT(xEmbeddeds.is());

    css::uno::Reference<css::beans::XPropertySet> xShapeProps(xEmbeddeds->getByIndex(nShape), css::uno::UNO_QUERY);
    CPPUNIT_ASSERT(xShapeProps.is());

    css::uno::Reference<css::frame::XModel> xDocModel;
    xShapeProps->getPropertyValue(u"Model"_ustr) >>= xDocModel;
    CPPUNIT_ASSERT(xDocModel.is());

    css::uno::Reference<css::chart::XChartDocument> xChartDoc(xDocModel, css::uno::UNO_QUERY);
    return xChartDoc;
}

OUString ChartTest::getTitleString( const css::uno::Reference<css::chart2::XTitled>& xTitled )
{
    css::uno::Reference<css::chart2::XTitle> xTitle = xTitled->getTitleObject();
    CPPUNIT_ASSERT(xTitle.is());
    const css::uno::Sequence<css::uno::Reference<css::chart2::XFormattedString> > aFSSeq = xTitle->getText();
    OUStringBuffer aText;
    for (auto const & fs : aFSSeq)
        aText.append(fs->getString());

    return aText.makeStringAndClear();
}

sal_Int32 ChartTest::getNumberFormatFromAxis( const css::uno::Reference<css::chart2::XAxis>& xAxis )
{
    css::uno::Reference<css::beans::XPropertySet> xPS(xAxis, css::uno::UNO_QUERY);
    CPPUNIT_ASSERT(xPS.is());
    sal_Int32 nNumberFormat = -1;
    bool bSuccess = xPS->getPropertyValue(CHART_UNONAME_NUMFMT) >>= nNumberFormat;
    CPPUNIT_ASSERT(bSuccess);

    return nNumberFormat;
}

sal_Int16 ChartTest::getNumberFormatType( const css::uno::Reference<css::chart2::XChartDocument>& xChartDoc, sal_Int32 nNumberFormat )
{
    css::uno::Reference<css::util::XNumberFormatsSupplier> xNFS(xChartDoc, css::uno::UNO_QUERY_THROW);
    css::uno::Reference<css::util::XNumberFormats> xNumberFormats = xNFS->getNumberFormats();
    CPPUNIT_ASSERT(xNumberFormats.is());

    css::uno::Reference<css::beans::XPropertySet> xNumPS = xNumberFormats->getByKey(nNumberFormat);
    CPPUNIT_ASSERT(xNumPS.is());

    sal_Int16 nType = css::util::NumberFormat::UNDEFINED;
    xNumPS->getPropertyValue(u"Type"_ustr) >>= nType;

    return nType;
}

css::uno::Reference<css::drawing::XShape>
ChartTest::getShapeByName(const css::uno::Reference<css::drawing::XShapes>& rShapes, const OUString& rName,
               const std::function<bool(const css::uno::Reference<css::drawing::XShape>&)>& pCondition)
{
    for (sal_Int32 i = 0; i < rShapes->getCount(); ++i)
    {
        css::uno::Reference<css::drawing::XShapes> xShapes(rShapes->getByIndex(i), css::uno::UNO_QUERY);
        if (xShapes.is())
        {
            css::uno::Reference<css::drawing::XShape> xRet = getShapeByName(xShapes, rName, pCondition);
            if (xRet.is())
                return xRet;
        }
        css::uno::Reference<css::container::XNamed> xNamedShape(rShapes->getByIndex(i), css::uno::UNO_QUERY);
        if (xNamedShape->getName() == rName)
        {
            css::uno::Reference<css::drawing::XShape> xShape(xNamedShape, css::uno::UNO_QUERY);
            if (pCondition == nullptr || pCondition(xShape))
                return xShape;
        }
    }
    return css::uno::Reference<css::drawing::XShape>();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
