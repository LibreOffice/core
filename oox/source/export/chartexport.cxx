/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <oox/token/namespaces.hxx>
#include <oox/token/properties.hxx>
#include <oox/token/tokens.hxx>
#include <oox/core/xmlfilterbase.hxx>
#include <oox/export/chartexport.hxx>
#include <oox/token/relationship.hxx>
#include <oox/export/utils.hxx>
#include <drawingml/chart/typegroupconverter.hxx>
#include <basegfx/utils/gradienttools.hxx>
#include <docmodel/uno/UnoGradientTools.hxx>

#include <cstdio>
#include <limits>

#include <com/sun/star/awt/Gradient2.hpp>
#include <com/sun/star/chart/XChartDocument.hpp>
#include <com/sun/star/chart/ChartLegendPosition.hpp>
#include <com/sun/star/chart/XTwoAxisXSupplier.hpp>
#include <com/sun/star/chart/XTwoAxisYSupplier.hpp>
#include <com/sun/star/chart/XAxisZSupplier.hpp>
#include <com/sun/star/chart/ChartDataRowSource.hpp>
#include <com/sun/star/chart/X3DDisplay.hpp>
#include <com/sun/star/chart/XStatisticDisplay.hpp>
#include <com/sun/star/chart/XSecondAxisTitleSupplier.hpp>
#include <com/sun/star/chart/ChartSymbolType.hpp>
#include <com/sun/star/chart/ChartAxisMarks.hpp>
#include <com/sun/star/chart/ChartAxisLabelPosition.hpp>
#include <com/sun/star/chart/ChartAxisPosition.hpp>
#include <com/sun/star/chart/ChartSolidType.hpp>
#include <com/sun/star/chart/DataLabelPlacement.hpp>
#include <com/sun/star/chart/ErrorBarStyle.hpp>
#include <com/sun/star/chart/MissingValueTreatment.hpp>
#include <com/sun/star/chart/XDiagramPositioning.hpp>
#include <com/sun/star/chart/TimeIncrement.hpp>
#include <com/sun/star/chart/TimeInterval.hpp>
#include <com/sun/star/chart/TimeUnit.hpp>

#include <com/sun/star/chart2/RelativePosition.hpp>
#include <com/sun/star/chart2/RelativeSize.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XDiagram.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XRegressionCurveContainer.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#include <com/sun/star/chart2/DataPointLabel.hpp>
#include <com/sun/star/chart2/XDataPointCustomLabelField.hpp>
#include <com/sun/star/chart2/DataPointCustomLabelFieldType.hpp>
#include <com/sun/star/chart2/PieChartSubType.hpp>
#include <com/sun/star/chart2/Symbol.hpp>
#include <com/sun/star/chart2/data/XDataSource.hpp>
#include <com/sun/star/chart2/data/XDataProvider.hpp>
#include <com/sun/star/chart2/data/XTextualDataSequence.hpp>
#include <com/sun/star/chart2/data/XNumericalDataSequence.hpp>
#include <com/sun/star/chart2/data/XLabeledDataSequence.hpp>
#include <com/sun/star/chart2/XAnyDescriptionAccess.hpp>
#include <com/sun/star/chart2/AxisType.hpp>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceName.hpp>

#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/sheet/XFormulaParser.hpp>
#include <com/sun/star/sheet/FormulaToken.hpp>
#include <com/sun/star/sheet/AddressConvention.hpp>

#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/embed/XVisualObject.hpp>
#include <com/sun/star/embed/Aspects.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/random.hxx>
#include <utility>
#include <xmloff/SchXMLSeriesHelper.hxx>
#include "ColorPropertySet.hxx"

#include <svl/numformat.hxx>
#include <svl/numuno.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <sal/log.hxx>

#include <set>
#include <unordered_set>

#include <frozen/bits/defines.h>
#include <frozen/bits/elsa_std.h>
#include <frozen/unordered_map.h>

#include <o3tl/temporary.hxx>
#include <o3tl/sorted_vector.hxx>

using namespace css;
using namespace css::uno;
using namespace css::drawing;
using namespace ::oox::core;
using css::beans::PropertyValue;
using css::beans::XPropertySet;
using css::container::XNamed;
using css::table::CellAddress;
using css::sheet::XFormulaParser;
using ::oox::core::XmlFilterBase;
using ::sax_fastparser::FSHelperPtr;

namespace cssc = css::chart;

namespace oox::drawingml {

namespace {

bool isPrimaryAxes(sal_Int32 nIndex)
{
    assert(nIndex == 0 || nIndex == 1);
    return nIndex != 1;
}

class lcl_MatchesRole
{
public:
    explicit lcl_MatchesRole( OUString aRole ) :
            m_aRole(std::move( aRole ))
    {}

    bool operator () ( const Reference< chart2::data::XLabeledDataSequence > & xSeq ) const
    {
        if( !xSeq.is() )
            return  false;
        Reference< beans::XPropertySet > xProp( xSeq->getValues(), uno::UNO_QUERY );
        OUString aRole;

        return ( xProp.is() &&
                 (xProp->getPropertyValue( "Role" ) >>= aRole ) &&
                 m_aRole == aRole );
    }

private:
    OUString m_aRole;
};

}

static Reference< chart2::data::XLabeledDataSequence > lcl_getCategories( const Reference< chart2::XDiagram > & xDiagram, bool& bHasDateCategories )
{
    bHasDateCategories = false;
    Reference< chart2::data::XLabeledDataSequence >  xResult;
    try
    {
        Reference< chart2::XCoordinateSystemContainer > xCooSysCnt(
            xDiagram, uno::UNO_QUERY_THROW );
        const Sequence< Reference< chart2::XCoordinateSystem > > aCooSysSeq(
            xCooSysCnt->getCoordinateSystems());
        for( const auto& xCooSys : aCooSysSeq )
        {
            OSL_ASSERT( xCooSys.is());
            for( sal_Int32 nN = xCooSys->getDimension(); nN--; )
            {
                const sal_Int32 nMaxAxisIndex = xCooSys->getMaximumAxisIndexByDimension(nN);
                for(sal_Int32 nI=0; nI<=nMaxAxisIndex; ++nI)
                {
                    Reference< chart2::XAxis > xAxis = xCooSys->getAxisByDimension( nN, nI );
                    OSL_ASSERT( xAxis.is());
                    if( xAxis.is())
                    {
                        chart2::ScaleData aScaleData = xAxis->getScaleData();
                        if( aScaleData.Categories.is())
                        {
                            bHasDateCategories = aScaleData.AxisType == chart2::AxisType::DATE;
                            xResult.set( aScaleData.Categories );
                            break;
                        }
                    }
                }
            }
        }
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("oox");
    }

    return xResult;
}

static Reference< chart2::data::XLabeledDataSequence >
    lcl_getDataSequenceByRole(
        const Sequence< Reference< chart2::data::XLabeledDataSequence > > & aLabeledSeq,
        const OUString & rRole )
{
    Reference< chart2::data::XLabeledDataSequence > aNoResult;

    const Reference< chart2::data::XLabeledDataSequence > * pBegin = aLabeledSeq.getConstArray();
    const Reference< chart2::data::XLabeledDataSequence > * pEnd = pBegin + aLabeledSeq.getLength();
    const Reference< chart2::data::XLabeledDataSequence > * pMatch =
        ::std::find_if( pBegin, pEnd, lcl_MatchesRole( rRole ));

    if( pMatch != pEnd )
        return *pMatch;

    return aNoResult;
}

static bool lcl_hasCategoryLabels( const Reference< chart2::XChartDocument >& xChartDoc )
{
    //categories are always the first sequence
    Reference< chart2::XDiagram > xDiagram( xChartDoc->getFirstDiagram());
    bool bDateCategories;
    Reference< chart2::data::XLabeledDataSequence > xCategories( lcl_getCategories( xDiagram, bDateCategories ) );
    return xCategories.is();
}

static bool lcl_isCategoryAxisShifted( const Reference< chart2::XDiagram >& xDiagram )
{
    bool bCategoryPositionShifted = false;
    try
    {
        Reference< chart2::XCoordinateSystemContainer > xCooSysCnt(
            xDiagram, uno::UNO_QUERY_THROW);
        const Sequence< Reference< chart2::XCoordinateSystem > > aCooSysSeq(
            xCooSysCnt->getCoordinateSystems());
        for (const auto& xCooSys : aCooSysSeq)
        {
            OSL_ASSERT(xCooSys.is());
            if( 0 < xCooSys->getDimension() && 0 <= xCooSys->getMaximumAxisIndexByDimension(0) )
            {
                Reference< chart2::XAxis > xAxis = xCooSys->getAxisByDimension(0, 0);
                OSL_ASSERT(xAxis.is());
                if (xAxis.is())
                {
                    chart2::ScaleData aScaleData = xAxis->getScaleData();
                    bCategoryPositionShifted = aScaleData.ShiftedCategoryPosition;
                    break;
                }
            }
        }
    }
    catch (const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("oox");
    }

    return bCategoryPositionShifted;
}

static sal_Int32 lcl_getCategoryAxisType( const Reference< chart2::XDiagram >& xDiagram, sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex )
{
    sal_Int32 nAxisType = -1;
    try
    {
        Reference< chart2::XCoordinateSystemContainer > xCooSysCnt(
            xDiagram, uno::UNO_QUERY_THROW);
        const Sequence< Reference< chart2::XCoordinateSystem > > aCooSysSeq(
            xCooSysCnt->getCoordinateSystems());
        for( const auto& xCooSys : aCooSysSeq )
        {
            OSL_ASSERT(xCooSys.is());
            if( nDimensionIndex < xCooSys->getDimension() && nAxisIndex <= xCooSys->getMaximumAxisIndexByDimension(nDimensionIndex) )
            {
                Reference< chart2::XAxis > xAxis = xCooSys->getAxisByDimension(nDimensionIndex, nAxisIndex);
                OSL_ASSERT(xAxis.is());
                if( xAxis.is() )
                {
                    chart2::ScaleData aScaleData = xAxis->getScaleData();
                    nAxisType = aScaleData.AxisType;
                    break;
                }
            }
        }
    }
    catch (const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("oox");
    }

    return nAxisType;
}

static OUString lclGetTimeUnitToken( sal_Int32 nTimeUnit )
{
    switch( nTimeUnit )
    {
        case cssc::TimeUnit::DAY:      return "days";
        case cssc::TimeUnit::MONTH:    return "months";
        case cssc::TimeUnit::YEAR:     return "years";
        default:                       OSL_ENSURE(false, "lclGetTimeUnitToken - unexpected time unit");
    }
    return "days";
}

static cssc::TimeIncrement lcl_getDateTimeIncrement( const Reference< chart2::XDiagram >& xDiagram, sal_Int32 nAxisIndex )
{
    cssc::TimeIncrement aTimeIncrement;
    try
    {
        Reference< chart2::XCoordinateSystemContainer > xCooSysCnt(
            xDiagram, uno::UNO_QUERY_THROW);
        const Sequence< Reference< chart2::XCoordinateSystem > > aCooSysSeq(
            xCooSysCnt->getCoordinateSystems());
        for( const auto& xCooSys : aCooSysSeq )
        {
            OSL_ASSERT(xCooSys.is());
            if( 0 < xCooSys->getDimension() && nAxisIndex <= xCooSys->getMaximumAxisIndexByDimension(0) )
            {
                Reference< chart2::XAxis > xAxis = xCooSys->getAxisByDimension(0, nAxisIndex);
                OSL_ASSERT(xAxis.is());
                if( xAxis.is() )
                {
                    chart2::ScaleData aScaleData = xAxis->getScaleData();
                    aTimeIncrement = aScaleData.TimeIncrement;
                    break;
                }
            }
        }
    }
    catch (const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("oox");
    }

    return aTimeIncrement;
}

static bool lcl_isSeriesAttachedToFirstAxis(
    const Reference< chart2::XDataSeries > & xDataSeries )
{
    bool bResult=true;

    try
    {
        sal_Int32 nAxisIndex = 0;
        Reference< beans::XPropertySet > xProp( xDataSeries, uno::UNO_QUERY_THROW );
        xProp->getPropertyValue("AttachedAxisIndex") >>= nAxisIndex;
        bResult = (0==nAxisIndex);
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("oox");
    }

    return bResult;
}

static OUString lcl_flattenStringSequence( const Sequence< OUString > & rSequence )
{
    OUStringBuffer aResult;
    bool bPrecedeWithSpace = false;
    for( const auto& rString : rSequence )
    {
        if( !rString.isEmpty())
        {
            if( bPrecedeWithSpace )
                aResult.append( ' ' );
            aResult.append( rString );
            bPrecedeWithSpace = true;
        }
    }
    return aResult.makeStringAndClear();
}

static Sequence< OUString > lcl_getLabelSequence( const Reference< chart2::data::XDataSequence > & xLabelSeq )
{
    Sequence< OUString > aLabels;

    uno::Reference< chart2::data::XTextualDataSequence > xTextualDataSequence( xLabelSeq, uno::UNO_QUERY );
    if( xTextualDataSequence.is())
    {
        aLabels = xTextualDataSequence->getTextualData();
    }
    else if( xLabelSeq.is())
    {
        const Sequence< uno::Any > aAnies( xLabelSeq->getData());
        aLabels.realloc( aAnies.getLength());
        auto pLabels = aLabels.getArray();
        for( sal_Int32 i=0; i<aAnies.getLength(); ++i )
            aAnies[i] >>= pLabels[i];
    }

    return aLabels;
}

static void lcl_fillCategoriesIntoStringVector(
    const Reference< chart2::data::XDataSequence > & xCategories,
    ::std::vector< OUString > & rOutCategories )
{
    OSL_ASSERT( xCategories.is());
    if( !xCategories.is())
        return;
    Reference< chart2::data::XTextualDataSequence > xTextualDataSequence( xCategories, uno::UNO_QUERY );
    if( xTextualDataSequence.is())
    {
        rOutCategories.clear();
        const Sequence< OUString > aTextData( xTextualDataSequence->getTextualData());
        rOutCategories.insert( rOutCategories.end(), aTextData.begin(), aTextData.end() );
    }
    else
    {
        Sequence< uno::Any > aAnies( xCategories->getData());
        rOutCategories.resize( aAnies.getLength());
        for( sal_Int32 i=0; i<aAnies.getLength(); ++i )
            aAnies[i] >>= rOutCategories[i];
    }
}

static ::std::vector< double > lcl_getAllValuesFromSequence( const Reference< chart2::data::XDataSequence > & xSeq )
{
    ::std::vector< double > aResult;

    Reference< chart2::data::XNumericalDataSequence > xNumSeq( xSeq, uno::UNO_QUERY );
    if( xNumSeq.is())
    {
        const Sequence< double > aValues( xNumSeq->getNumericalData());
        aResult.insert( aResult.end(), aValues.begin(), aValues.end() );
    }
    else if( xSeq.is())
    {
        Sequence< uno::Any > aAnies( xSeq->getData());
        aResult.resize( aAnies.getLength(), std::numeric_limits<double>::quiet_NaN() );
        for( sal_Int32 i=0; i<aAnies.getLength(); ++i )
            aAnies[i] >>= aResult[i];
    }
    return aResult;
}

namespace
{

constexpr auto constChartTypeMap = frozen::make_unordered_map<std::u16string_view, chart::TypeId>(
{
    { u"com.sun.star.chart.BarDiagram", chart::TYPEID_BAR },
    { u"com.sun.star.chart2.ColumnChartType",  chart::TYPEID_BAR },
    { u"com.sun.star.chart.AreaDiagram",  chart::TYPEID_AREA },
    { u"com.sun.star.chart2.AreaChartType",  chart::TYPEID_AREA },
    { u"com.sun.star.chart.LineDiagram",  chart::TYPEID_LINE },
    { u"com.sun.star.chart2.LineChartType",  chart::TYPEID_LINE },
    { u"com.sun.star.chart.PieDiagram",  chart::TYPEID_PIE },
    { u"com.sun.star.chart2.PieChartType",  chart::TYPEID_PIE },
    { u"com.sun.star.chart.DonutDiagram",  chart::TYPEID_DOUGHNUT },
    { u"com.sun.star.chart2.DonutChartType",  chart::TYPEID_DOUGHNUT },
    { u"com.sun.star.chart.XYDiagram",  chart::TYPEID_SCATTER },
    { u"com.sun.star.chart2.ScatterChartType",  chart::TYPEID_SCATTER },
    { u"com.sun.star.chart.NetDiagram",  chart::TYPEID_RADARLINE },
    { u"com.sun.star.chart2.NetChartType",  chart::TYPEID_RADARLINE },
    { u"com.sun.star.chart.FilledNetDiagram",  chart::TYPEID_RADARAREA },
    { u"com.sun.star.chart2.FilledNetChartType",  chart::TYPEID_RADARAREA },
    { u"com.sun.star.chart.StockDiagram",  chart::TYPEID_STOCK },
    { u"com.sun.star.chart2.CandleStickChartType",  chart::TYPEID_STOCK },
    { u"com.sun.star.chart.BubbleDiagram",  chart::TYPEID_BUBBLE },
    { u"com.sun.star.chart2.BubbleChartType",  chart::TYPEID_BUBBLE },
});

} // end anonymous namespace

static sal_Int32 lcl_getChartType(std::u16string_view sChartType)
{
    auto aIterator = constChartTypeMap.find(sChartType);
    if (aIterator == constChartTypeMap.end())
        return chart::TYPEID_UNKNOWN;
    return aIterator->second;
}

static sal_Int32 lcl_generateRandomValue()
{
    return comphelper::rng::uniform_int_distribution(0, 100000000-1);
}

bool DataLabelsRange::empty() const
{
    return maLabels.empty();
}

size_t DataLabelsRange::count() const
{
    return maLabels.size();
}

bool DataLabelsRange::hasLabel(sal_Int32 nIndex) const
{
    return maLabels.find(nIndex) != maLabels.end();
}

const OUString & DataLabelsRange::getRange() const
{
    return maRange;
}

void DataLabelsRange::setRange(const OUString& rRange)
{
    maRange = rRange;
}

void DataLabelsRange::setLabel(sal_Int32 nIndex, const OUString& rText)
{
    maLabels.emplace(nIndex, rText);
}

DataLabelsRange::LabelsRangeMap::const_iterator DataLabelsRange::begin() const
{
    return maLabels.begin();
}

DataLabelsRange::LabelsRangeMap::const_iterator DataLabelsRange::end() const
{
    return maLabels.end();
}

ChartExport::ChartExport( sal_Int32 nXmlNamespace, FSHelperPtr pFS, Reference< frame::XModel > const & xModel, XmlFilterBase* pFB, DocumentType eDocumentType )
    : DrawingML( std::move(pFS), pFB, eDocumentType )
    , mnXmlNamespace( nXmlNamespace )
    , mnSeriesCount(0)
    , mxChartModel( xModel )
    , mpURLTransformer(std::make_shared<URLTransformer>())
    , mbHasCategoryLabels( false )
    , mbHasZAxis( false )
    , mbIs3DChart( false )
    , mbStacked(false)
    , mbPercent(false)
    , mbHasDateCategories(false)
{
}

void ChartExport::SetURLTranslator(const std::shared_ptr<URLTransformer>& pTransformer)
{
    mpURLTransformer = pTransformer;
}

sal_Int32 ChartExport::getChartType( )
{
    OUString sChartType = mxDiagram->getDiagramType();
    return lcl_getChartType( sChartType );
}

namespace {

uno::Sequence< beans::PropertyValue > createArguments(
    const OUString & rRangeRepresentation, bool bUseColumns)
{
    css::chart::ChartDataRowSource eRowSource = css::chart::ChartDataRowSource_ROWS;
    if (bUseColumns)
        eRowSource = css::chart::ChartDataRowSource_COLUMNS;

    uno::Sequence<beans::PropertyValue> aArguments{
        { "DataRowSource", -1, uno::Any(eRowSource), beans::PropertyState_DIRECT_VALUE },
        { "FirstCellAsLabel", -1, uno::Any(false), beans::PropertyState_DIRECT_VALUE },
        { "HasCategories", -1, uno::Any(false), beans::PropertyState_DIRECT_VALUE },
        { "CellRangeRepresentation", -1, uno::Any(rRangeRepresentation),
          beans::PropertyState_DIRECT_VALUE }
    };

    return aArguments;
}

Reference<chart2::XDataSeries> getPrimaryDataSeries(const Reference<chart2::XChartType>& xChartType)
{
    Reference< chart2::XDataSeriesContainer > xDSCnt(xChartType, uno::UNO_QUERY_THROW);

    // export dataseries for current chart-type
    const Sequence< Reference< chart2::XDataSeries > > aSeriesSeq(xDSCnt->getDataSeries());
    for (const auto& rSeries : aSeriesSeq)
    {
        Reference<chart2::XDataSeries> xSource(rSeries, uno::UNO_QUERY);
        if (xSource.is())
            return xSource;
    }

    return Reference<chart2::XDataSeries>();
}

}

Sequence< Sequence< OUString > > ChartExport::getSplitCategoriesList( const OUString& rRange )
{
    Reference< chart2::XChartDocument > xChartDoc(getModel(), uno::UNO_QUERY);
    OSL_ASSERT(xChartDoc.is());
    if (xChartDoc.is())
    {
        Reference< chart2::data::XDataProvider > xDataProvider(xChartDoc->getDataProvider());
        OSL_ENSURE(xDataProvider.is(), "No DataProvider");
        if (xDataProvider.is())
        {
            //detect whether the first series is a row or a column
            bool bSeriesUsesColumns = true;
            Reference< chart2::XDiagram > xDiagram(xChartDoc->getFirstDiagram());
            try
            {
                Reference< chart2::XCoordinateSystemContainer > xCooSysCnt(xDiagram, uno::UNO_QUERY_THROW);
                const Sequence< Reference< chart2::XCoordinateSystem > > aCooSysSeq(xCooSysCnt->getCoordinateSystems());
                for (const auto& rCooSys : aCooSysSeq)
                {
                    const Reference< chart2::XChartTypeContainer > xCTCnt(rCooSys, uno::UNO_QUERY_THROW);
                    const Sequence< Reference< chart2::XChartType > > aChartTypeSeq(xCTCnt->getChartTypes());
                    for (const auto& rChartType : aChartTypeSeq)
                    {
                        Reference< chart2::XDataSeries > xDataSeries = getPrimaryDataSeries(rChartType);
                        if (xDataSeries.is())
                        {
                            uno::Reference< chart2::data::XDataSource > xSeriesSource(xDataSeries, uno::UNO_QUERY);
                            const uno::Sequence< beans::PropertyValue > rArguments = xDataProvider->detectArguments(xSeriesSource);
                            for (const beans::PropertyValue& rProperty : rArguments)
                            {
                                if (rProperty.Name == "DataRowSource")
                                {
                                    css::chart::ChartDataRowSource eRowSource;
                                    if (rProperty.Value >>= eRowSource)
                                    {
                                        bSeriesUsesColumns = (eRowSource == css::chart::ChartDataRowSource_COLUMNS);
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            catch (const uno::Exception &)
            {
                DBG_UNHANDLED_EXCEPTION("chart2");
            }
            // detect we have an inner data table or not
            if (xChartDoc->hasInternalDataProvider() && rRange == "categories")
            {
                try
                {
                    css::uno::Reference< css::chart2::XAnyDescriptionAccess > xDataAccess(xChartDoc->getDataProvider(), uno::UNO_QUERY);
                    const Sequence< Sequence< uno::Any > >aAnyCategories(bSeriesUsesColumns ? xDataAccess->getAnyRowDescriptions() : xDataAccess->getAnyColumnDescriptions());
                    auto pMax = std::max_element(aAnyCategories.begin(), aAnyCategories.end(),
                        [](const Sequence<uno::Any>& a, const Sequence<uno::Any>& b) {
                            return a.getLength() < b.getLength(); });

                    //minimum is 1!
                    if (pMax != aAnyCategories.end() && pMax->getLength() > 1)
                    {
                        sal_Int32 nLevelCount = pMax->getLength();
                        //we have complex categories
                        //sort the categories name
                        Sequence<Sequence<OUString>>aFinalSplitSource(nLevelCount);
                        auto pFinalSplitSource = aFinalSplitSource.getArray();
                        for (sal_Int32 i = 0; i < nLevelCount; i++)
                        {
                            sal_Int32 nElemLabel = 0;
                            pFinalSplitSource[nLevelCount - i - 1].realloc(aAnyCategories.getLength());
                            auto pSeq = pFinalSplitSource[nLevelCount - i - 1].getArray();
                            for (auto const& elemLabel : aAnyCategories)
                            {
                                // make sure elemLabel[i] exists!
                                if (elemLabel.getLength() > i)
                                {
                                    pSeq[nElemLabel] = elemLabel[i].get<OUString>();
                                    nElemLabel++;
                                }
                            }
                        }
                        return aFinalSplitSource;
                    }
                }
                catch (const uno::Exception &)
                {
                    DBG_UNHANDLED_EXCEPTION("oox");
                }
            }
            else
            {
                try
                {
                    uno::Reference< chart2::data::XDataSource > xCategoriesSource(xDataProvider->createDataSource(
                        createArguments(rRange, bSeriesUsesColumns)));

                    if (xCategoriesSource.is())
                    {
                        const Sequence< Reference< chart2::data::XLabeledDataSequence >> aCategories = xCategoriesSource->getDataSequences();
                        if (aCategories.getLength() > 1)
                        {
                            //we have complex categories
                            //sort the categories name
                            Sequence<Sequence<OUString>> aFinalSplitSource(aCategories.getLength());
                            std::transform(aCategories.begin(), aCategories.end(),
                                std::reverse_iterator(asNonConstRange(aFinalSplitSource).end()),
                                [](const Reference<chart2::data::XLabeledDataSequence>& xCat) {
                                    return lcl_getLabelSequence(xCat->getValues()); });
                            return aFinalSplitSource;
                        }
                    }
                }
                catch (const uno::Exception &)
                {
                    DBG_UNHANDLED_EXCEPTION("oox");
                }
            }
        }
    }

    return Sequence< Sequence< OUString>>(0);
}

OUString ChartExport::parseFormula( const OUString& rRange )
{
    OUString aResult;
    Reference< XFormulaParser > xParser;
    uno::Reference< lang::XMultiServiceFactory > xSF = GetFB()->getModelFactory();
    if( xSF.is() )
    {
        try
        {
            xParser.set( xSF->createInstance("com.sun.star.sheet.FormulaParser"), UNO_QUERY );
        }
        catch( Exception& )
        {
        }
    }

    SAL_WARN_IF(!xParser.is(), "oox", "creating formula parser failed");

    if( xParser.is() )
    {
        Reference< XPropertySet > xParserProps( xParser, uno::UNO_QUERY );
        // rRange is the result of a
        // css::chart2::data::XDataSequence::getSourceRangeRepresentation()
        // call that returns the range in the document's current UI notation.
        // Creating a FormulaParser defaults to the same notation, for
        // parseFormula() do not attempt to override the FormulaConvention
        // property with css::sheet::AddressConvention::OOO or some such.
        /* TODO: it would be much better to introduce a
         * getSourceRangeRepresentation(css::sheet::AddressConvention) to
         * return the ranges in a specific convention than converting them with
         * the overhead of creating an XFormulaParser for each... */
        uno::Sequence<sheet::FormulaToken> aTokens = xParser->parseFormula( rRange, CellAddress( 0, 0, 0 ) );
        if( xParserProps.is() )
        {
            xParserProps->setPropertyValue("FormulaConvention", uno::Any(css::sheet::AddressConvention::XL_OOX) );
            // For referencing named ranges correctly with special excel chart syntax.
            xParserProps->setPropertyValue("RefConventionChartOOXML", uno::Any(true) );
        }
        aResult = xParser->printFormula( aTokens, CellAddress( 0, 0, 0 ) );
    }
    else
    {
        //FIXME: currently just using simple converter, e.g $Sheet1.$A$1:$C$1 -> Sheet1!$A$1:$C$1
        OUString aRange( rRange );
        if( aRange.startsWith("$") )
            aRange = aRange.copy(1);
        aRange = aRange.replaceAll(".$", "!$" );
        aResult = aRange;
    }

    return aResult;
}

void ChartExport::WriteChartObj( const Reference< XShape >& xShape, sal_Int32 nID, sal_Int32 nChartCount )
{
    FSHelperPtr pFS = GetFS();

    Reference< XPropertySet > xShapeProps( xShape, UNO_QUERY );

    pFS->startElementNS(mnXmlNamespace, XML_graphicFrame);

    pFS->startElementNS(mnXmlNamespace, XML_nvGraphicFramePr);

    // TODO: get the correct chart name chart id
    OUString sName = "Object 1";
    Reference< XNamed > xNamed( xShape, UNO_QUERY );
    if (xNamed.is())
        sName = xNamed->getName();

    pFS->startElementNS( mnXmlNamespace, XML_cNvPr,
                          XML_id,     OString::number(nID),
                          XML_name,   sName);

    OUString sURL;
    if ( GetProperty( xShapeProps, "URL" ) )
        mAny >>= sURL;
    if( !sURL.isEmpty() )
    {
        OUString sRelId = mpFB->addRelation( mpFS->getOutputStream(),
                oox::getRelationship(Relationship::HYPERLINK),
                mpURLTransformer->getTransformedString(sURL),
                mpURLTransformer->isExternalURL(sURL));

        mpFS->singleElementNS(XML_a, XML_hlinkClick, FSNS(XML_r, XML_id), sRelId);
    }
    pFS->endElementNS(mnXmlNamespace, XML_cNvPr);

    pFS->singleElementNS(mnXmlNamespace, XML_cNvGraphicFramePr);

    if( GetDocumentType() == DOCUMENT_PPTX )
        pFS->singleElementNS(mnXmlNamespace, XML_nvPr);
    pFS->endElementNS( mnXmlNamespace, XML_nvGraphicFramePr );

    // visual chart properties
    WriteShapeTransformation( xShape, mnXmlNamespace );

    // writer chart object
    pFS->startElement(FSNS(XML_a, XML_graphic));
    pFS->startElement( FSNS( XML_a, XML_graphicData ),
                       XML_uri, "http://schemas.openxmlformats.org/drawingml/2006/chart" );
    OUString sId;
    const char* sFullPath = nullptr;
    const char* sRelativePath = nullptr;
    switch( GetDocumentType() )
    {
        case DOCUMENT_DOCX:
        {
            sFullPath = "word/charts/chart";
            sRelativePath = "charts/chart";
            break;
        }
        case DOCUMENT_PPTX:
        {
            sFullPath = "ppt/charts/chart";
            sRelativePath = "../charts/chart";
            break;
        }
        case DOCUMENT_XLSX:
        {
            sFullPath = "xl/charts/chart";
            sRelativePath = "../charts/chart";
            break;
        }
        default:
        {
            sFullPath = "charts/chart";
            sRelativePath = "charts/chart";
            break;
        }
    }
    OUString sFullStream = OUStringBuffer()
                            .appendAscii(sFullPath)
                            .append(OUString::number(nChartCount) + ".xml")
                            .makeStringAndClear();
    OUString sRelativeStream = OUStringBuffer()
                            .appendAscii(sRelativePath)
                            .append(OUString::number(nChartCount) + ".xml" )
                            .makeStringAndClear();
    FSHelperPtr pChart = CreateOutputStream(
            sFullStream,
            sRelativeStream,
            pFS->getOutputStream(),
            "application/vnd.openxmlformats-officedocument.drawingml.chart+xml",
            oox::getRelationship(Relationship::CHART),
            &sId );

    XmlFilterBase* pFB = GetFB();
    pFS->singleElement(  FSNS( XML_c, XML_chart ),
            FSNS(XML_xmlns, XML_c), pFB->getNamespaceURL(OOX_NS(dmlChart)),
            FSNS(XML_xmlns, XML_r), pFB->getNamespaceURL(OOX_NS(officeRel)),
            FSNS(XML_r, XML_id), sId );

    pFS->endElement( FSNS( XML_a, XML_graphicData ) );
    pFS->endElement( FSNS( XML_a, XML_graphic ) );
    pFS->endElementNS( mnXmlNamespace, XML_graphicFrame );

    SetFS( pChart );
    ExportContent();
    SetFS( pFS );
    pChart->endDocument();
}

void ChartExport::InitRangeSegmentationProperties( const Reference< chart2::XChartDocument > & xChartDoc )
{
    if( !xChartDoc.is())
        return;

    try
    {
        Reference< chart2::data::XDataProvider > xDataProvider( xChartDoc->getDataProvider() );
        OSL_ENSURE( xDataProvider.is(), "No DataProvider" );
        if( xDataProvider.is())
        {
            mbHasCategoryLabels = lcl_hasCategoryLabels( xChartDoc );
        }
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("oox");
    }
}

void ChartExport::ExportContent()
{
    Reference< chart2::XChartDocument > xChartDoc( getModel(), uno::UNO_QUERY );
    OSL_ASSERT( xChartDoc.is() );
    if( !xChartDoc.is() )
        return;
    InitRangeSegmentationProperties( xChartDoc );
    // TODO: export chart
    ExportContent_( );
}

void ChartExport::ExportContent_()
{
    Reference< css::chart::XChartDocument > xChartDoc( getModel(), uno::UNO_QUERY );
    if( xChartDoc.is())
    {
        // determine if data comes from the outside
        bool bIncludeTable = true;

        Reference< chart2::XChartDocument > xNewDoc( xChartDoc, uno::UNO_QUERY );
        if( xNewDoc.is())
        {
            // check if we have own data.  If so we must not export the complete
            // range string, as this is our only indicator for having own or
            // external data. @todo: fix this in the file format!
            Reference< lang::XServiceInfo > xDPServiceInfo( xNewDoc->getDataProvider(), uno::UNO_QUERY );
            if( ! (xDPServiceInfo.is() && xDPServiceInfo->getImplementationName() == "com.sun.star.comp.chart.InternalDataProvider" ))
            {
                bIncludeTable = false;
            }
        }
        exportChartSpace( xChartDoc, bIncludeTable );
    }
    else
    {
        OSL_FAIL( "Couldn't export chart due to wrong XModel" );
    }
}

void ChartExport::exportChartSpace( const Reference< css::chart::XChartDocument >& xChartDoc,
                                    bool bIncludeTable )
{
    FSHelperPtr pFS = GetFS();
    XmlFilterBase* pFB = GetFB();
    pFS->startElement( FSNS( XML_c, XML_chartSpace ),
            FSNS( XML_xmlns, XML_c ), pFB->getNamespaceURL(OOX_NS(dmlChart)),
            FSNS( XML_xmlns, XML_a ), pFB->getNamespaceURL(OOX_NS(dml)),
            FSNS( XML_xmlns, XML_r ), pFB->getNamespaceURL(OOX_NS(officeRel)));
    // TODO: get the correct editing language
    pFS->singleElement(FSNS(XML_c, XML_lang), XML_val, "en-US");

    pFS->singleElement(FSNS(XML_c, XML_roundedCorners), XML_val, "0");

    if( !bIncludeTable )
    {
        // TODO:external data
    }
    //XML_chart
    exportChart(xChartDoc);

    // TODO: printSettings
    // TODO: style
    // TODO: text properties
    // TODO: shape properties
    Reference< XPropertySet > xPropSet = xChartDoc->getArea();
    if( xPropSet.is() )
        exportShapeProps( xPropSet );

    //XML_externalData
    exportExternalData(xChartDoc);

    // export additional shapes in chart
    exportAdditionalShapes(xChartDoc);

    pFS->endElement( FSNS( XML_c, XML_chartSpace ) );
}

void ChartExport::exportExternalData( const Reference< css::chart::XChartDocument >& xChartDoc )
{
    // Embedded external data is grab bagged for docx file hence adding export part of
    // external data for docx files only.
    if(GetDocumentType() != DOCUMENT_DOCX)
        return;

    OUString externalDataPath;
    Reference< beans::XPropertySet > xDocPropSet( xChartDoc->getDiagram(), uno::UNO_QUERY );
    if( xDocPropSet.is())
    {
        try
        {
            Any aAny( xDocPropSet->getPropertyValue( "ExternalData" ));
            aAny >>= externalDataPath;
        }
        catch( beans::UnknownPropertyException & )
        {
            SAL_WARN("oox", "Required property not found in ChartDocument");
        }
    }
    if(externalDataPath.isEmpty())
        return;

    // Here adding external data entry to relationship.
    OUString relationPath = externalDataPath;
    // Converting absolute path to relative path.
    if( externalDataPath[ 0 ] != '.' && externalDataPath[ 1 ] != '.')
    {
        sal_Int32 nSepPos = externalDataPath.indexOf( '/', 0 );
        if( nSepPos > 0)
        {
            relationPath = relationPath.copy( nSepPos,  ::std::max< sal_Int32 >( externalDataPath.getLength(), 0 ) -  nSepPos );
            relationPath = ".." + relationPath;
        }
    }
    FSHelperPtr pFS = GetFS();
    OUString type = oox::getRelationship(Relationship::PACKAGE);
    if (relationPath.endsWith(".bin"))
        type = oox::getRelationship(Relationship::OLEOBJECT);

    OUString sRelId = GetFB()->addRelation(pFS->getOutputStream(),
                    type,
                    relationPath);
    pFS->singleElementNS(XML_c, XML_externalData, FSNS(XML_r, XML_id), sRelId);
}

void ChartExport::exportAdditionalShapes( const Reference< css::chart::XChartDocument >& xChartDoc )
{
    Reference< beans::XPropertySet > xDocPropSet(xChartDoc, uno::UNO_QUERY);
    if (!xDocPropSet.is())
        return;

    css::uno::Reference< css::drawing::XShapes > mxAdditionalShapes;
    // get a sequence of non-chart shapes
    try
    {
        Any aShapesAny = xDocPropSet->getPropertyValue("AdditionalShapes");
        if( (aShapesAny >>= mxAdditionalShapes) && mxAdditionalShapes.is() )
        {
            OUString sId;
            const char* sFullPath = nullptr;
            const char* sRelativePath = nullptr;
            sal_Int32 nDrawing = getNewDrawingUniqueId();

            switch (GetDocumentType())
            {
                case DOCUMENT_DOCX:
                {
                    sFullPath = "word/drawings/drawing";
                    sRelativePath = "../drawings/drawing";
                    break;
                }
                case DOCUMENT_PPTX:
                {
                    sFullPath = "ppt/drawings/drawing";
                    sRelativePath = "../drawings/drawing";
                    break;
                }
                case DOCUMENT_XLSX:
                {
                    sFullPath = "xl/drawings/drawing";
                    sRelativePath = "../drawings/drawing";
                    break;
                }
                default:
                {
                    sFullPath = "drawings/drawing";
                    sRelativePath = "drawings/drawing";
                    break;
                }
            }
            OUString sFullStream = OUStringBuffer()
                .appendAscii(sFullPath)
                .append(OUString::number(nDrawing) + ".xml")
                .makeStringAndClear();
            OUString sRelativeStream = OUStringBuffer()
                .appendAscii(sRelativePath)
                .append(OUString::number(nDrawing) + ".xml")
                .makeStringAndClear();

            sax_fastparser::FSHelperPtr pDrawing = CreateOutputStream(
                sFullStream,
                sRelativeStream,
                GetFS()->getOutputStream(),
                "application/vnd.openxmlformats-officedocument.drawingml.chartshapes+xml",
                oox::getRelationship(Relationship::CHARTUSERSHAPES),
                &sId);

            GetFS()->singleElementNS(XML_c, XML_userShapes, FSNS(XML_r, XML_id), sId);

            XmlFilterBase* pFB = GetFB();
            pDrawing->startElement(FSNS(XML_c, XML_userShapes),
                FSNS(XML_xmlns, XML_cdr), pFB->getNamespaceURL(OOX_NS(dmlChartDr)),
                FSNS(XML_xmlns, XML_a), pFB->getNamespaceURL(OOX_NS(dml)),
                FSNS(XML_xmlns, XML_c), pFB->getNamespaceURL(OOX_NS(dmlChart)),
                FSNS(XML_xmlns, XML_r), pFB->getNamespaceURL(OOX_NS(officeRel)));

            const sal_Int32 nShapeCount(mxAdditionalShapes->getCount());
            for (sal_Int32 nShapeId = 0; nShapeId < nShapeCount; nShapeId++)
            {
                Reference< drawing::XShape > xShape;
                mxAdditionalShapes->getByIndex(nShapeId) >>= xShape;
                SAL_WARN_IF(!xShape.is(), "xmloff.chart", "Shape without an XShape?");
                if (!xShape.is())
                    continue;

                // TODO: absSizeAnchor: we import both (absSizeAnchor and relSizeAnchor), but there is no essential difference between them.
                pDrawing->startElement(FSNS(XML_cdr, XML_relSizeAnchor));
                uno::Reference< beans::XPropertySet > xShapeProperties(xShape, uno::UNO_QUERY);
                if( xShapeProperties.is() )
                {
                    Reference<embed::XVisualObject> xVisObject(mxChartModel, uno::UNO_QUERY);
                    awt::Size aPageSize = xVisObject->getVisualAreaSize(embed::Aspects::MSOLE_CONTENT);
                    WriteFromTo( xShape, aPageSize, pDrawing );

                    ShapeExport aExport(XML_cdr, pDrawing, nullptr, GetFB(), GetDocumentType(), nullptr, true);
                    aExport.WriteShape(xShape);
                }
                pDrawing->endElement(FSNS(XML_cdr, XML_relSizeAnchor));
            }
            pDrawing->endElement(FSNS(XML_c, XML_userShapes));
            pDrawing->endDocument();
        }
    }
    catch (const uno::Exception&)
    {
        TOOLS_INFO_EXCEPTION("xmloff.chart", "AdditionalShapes not found");
    }
}

void ChartExport::exportChart( const Reference< css::chart::XChartDocument >& xChartDoc )
{
    Reference< chart2::XChartDocument > xNewDoc( xChartDoc, uno::UNO_QUERY );
    mxDiagram.set( xChartDoc->getDiagram() );
    if( xNewDoc.is())
        mxNewDiagram.set( xNewDoc->getFirstDiagram());

    // get Properties of ChartDocument
    bool bHasMainTitle = false;
    bool bHasLegend = false;
    Reference< beans::XPropertySet > xDocPropSet( xChartDoc, uno::UNO_QUERY );
    if( xDocPropSet.is())
    {
        try
        {
            Any aAny( xDocPropSet->getPropertyValue("HasMainTitle"));
            aAny >>= bHasMainTitle;
            aAny = xDocPropSet->getPropertyValue("HasLegend");
            aAny >>= bHasLegend;
        }
        catch( beans::UnknownPropertyException & )
        {
            SAL_WARN("oox", "Required property not found in ChartDocument");
        }
    } // if( xDocPropSet.is())

    Sequence< uno::Reference< chart2::XFormattedString > > xFormattedSubTitle;
    Reference< beans::XPropertySet > xPropSubTitle( xChartDoc->getSubTitle(), UNO_QUERY );
    if( xPropSubTitle.is())
    {
        try
        {
            xPropSubTitle->getPropertyValue("FormattedStrings") >>= xFormattedSubTitle;
        }
        catch( beans::UnknownPropertyException & )
        {
        }
    }

    // chart element
    FSHelperPtr pFS = GetFS();
    pFS->startElement(FSNS(XML_c, XML_chart));

    // titles
    if( bHasMainTitle )
    {
        exportTitle( xChartDoc->getTitle(), xFormattedSubTitle);
        pFS->singleElement(FSNS(XML_c, XML_autoTitleDeleted), XML_val, "0");
    }
    else if( xFormattedSubTitle.hasElements() )
    {
        exportTitle( xChartDoc->getSubTitle() );
        pFS->singleElement(FSNS(XML_c, XML_autoTitleDeleted), XML_val, "0");
    }
    else
    {
        pFS->singleElement(FSNS(XML_c, XML_autoTitleDeleted), XML_val, "1");
    }

    InitPlotArea( );
    if( mbIs3DChart )
    {
        exportView3D();

        // floor
        Reference< beans::XPropertySet > xFloor = mxNewDiagram->getFloor();
        if( xFloor.is() )
        {
            pFS->startElement(FSNS(XML_c, XML_floor));
            exportShapeProps( xFloor );
            pFS->endElement( FSNS( XML_c, XML_floor ) );
        }

        // LibreOffice doesn't distinguish between sideWall and backWall (both are using the same color).
        // It is controlled by the same Wall property.
        Reference< beans::XPropertySet > xWall = mxNewDiagram->getWall();
        if( xWall.is() )
        {
            // sideWall
            pFS->startElement(FSNS(XML_c, XML_sideWall));
            exportShapeProps( xWall );
            pFS->endElement( FSNS( XML_c, XML_sideWall ) );

            // backWall
            pFS->startElement(FSNS(XML_c, XML_backWall));
            exportShapeProps( xWall );
            pFS->endElement( FSNS( XML_c, XML_backWall ) );
        }

    }
    // plot area
    exportPlotArea( xChartDoc );
    // legend
    if( bHasLegend )
        exportLegend( xChartDoc );

    uno::Reference<beans::XPropertySet> xDiagramPropSet(xChartDoc->getDiagram(), uno::UNO_QUERY);
    uno::Any aPlotVisOnly = xDiagramPropSet->getPropertyValue("IncludeHiddenCells");
    bool bIncludeHiddenCells = false;
    aPlotVisOnly >>= bIncludeHiddenCells;
    pFS->singleElement(FSNS(XML_c, XML_plotVisOnly), XML_val, ToPsz10(!bIncludeHiddenCells));

    exportMissingValueTreatment(Reference<beans::XPropertySet>(mxDiagram, uno::UNO_QUERY));

    pFS->endElement( FSNS( XML_c, XML_chart ) );
}

void ChartExport::exportMissingValueTreatment(const uno::Reference<beans::XPropertySet>& xPropSet)
{
    if (!xPropSet.is())
        return;

    sal_Int32 nVal = 0;
    uno::Any aAny = xPropSet->getPropertyValue("MissingValueTreatment");
    if (!(aAny >>= nVal))
        return;

    const char* pVal = nullptr;
    switch (nVal)
    {
        case cssc::MissingValueTreatment::LEAVE_GAP:
            pVal = "gap";
        break;
        case cssc::MissingValueTreatment::USE_ZERO:
            pVal = "zero";
        break;
        case cssc::MissingValueTreatment::CONTINUE:
            pVal = "span";
        break;
        default:
            SAL_WARN("oox", "unknown MissingValueTreatment value");
        break;
    }

    FSHelperPtr pFS = GetFS();
    pFS->singleElement(FSNS(XML_c, XML_dispBlanksAs), XML_val, pVal);
}

void ChartExport::exportLegend( const Reference< css::chart::XChartDocument >& xChartDoc )
{
    FSHelperPtr pFS = GetFS();
    pFS->startElement(FSNS(XML_c, XML_legend));

    Reference< beans::XPropertySet > xProp( xChartDoc->getLegend(), uno::UNO_QUERY );
    if( xProp.is() )
    {
        // position
        css::chart::ChartLegendPosition aLegendPos = css::chart::ChartLegendPosition_NONE;
        try
        {
            Any aAny( xProp->getPropertyValue( "Alignment" ));
            aAny >>= aLegendPos;
        }
        catch( beans::UnknownPropertyException & )
        {
            SAL_WARN("oox", "Property Align not found in ChartLegend");
        }

        const char* strPos = nullptr;
        switch( aLegendPos )
        {
            case css::chart::ChartLegendPosition_LEFT:
                strPos = "l";
                break;
            case css::chart::ChartLegendPosition_RIGHT:
                strPos = "r";
                break;
            case css::chart::ChartLegendPosition_TOP:
                strPos = "t";
                break;
            case css::chart::ChartLegendPosition_BOTTOM:
                strPos = "b";
                break;
            case css::chart::ChartLegendPosition_NONE:
            case css::chart::ChartLegendPosition::ChartLegendPosition_MAKE_FIXED_SIZE:
                // nothing
                break;
        }

        if( strPos != nullptr )
        {
            pFS->singleElement(FSNS(XML_c, XML_legendPos), XML_val, strPos);
        }

        // legendEntry
        Reference<chart2::XCoordinateSystemContainer> xCooSysContainer(mxNewDiagram, UNO_QUERY_THROW);
        const Sequence<Reference<chart2::XCoordinateSystem>> xCooSysSequence(xCooSysContainer->getCoordinateSystems());

        sal_Int32 nIndex = 0;
        bool bShowLegendEntry;
        for (const auto& rCooSys : xCooSysSequence)
        {
            PropertySet aCooSysProp(rCooSys);
            bool bSwapXAndY = aCooSysProp.getBoolProperty(PROP_SwapXAndYAxis);

            Reference<chart2::XChartTypeContainer> xChartTypeContainer(rCooSys, UNO_QUERY_THROW);
            const Sequence<Reference<chart2::XChartType>> xChartTypeSequence(xChartTypeContainer->getChartTypes());
            if (!xChartTypeSequence.hasElements())
                continue;

            for (const auto& rCT : xChartTypeSequence)
            {
                Reference<chart2::XDataSeriesContainer> xDSCont(rCT, UNO_QUERY);
                if (!xDSCont.is())
                    continue;

                OUString aChartType(rCT->getChartType());
                bool bIsPie = lcl_getChartType(aChartType) == chart::TYPEID_PIE;
                if (bIsPie)
                {
                    PropertySet xChartTypeProp(rCT);
                    bIsPie = !xChartTypeProp.getBoolProperty(PROP_UseRings);
                }
                const Sequence<Reference<chart2::XDataSeries>> aDataSeriesSeq = xDSCont->getDataSeries();
                if (bSwapXAndY)
                    nIndex += aDataSeriesSeq.getLength() - 1;
                for (const auto& rDataSeries : aDataSeriesSeq)
                {
                    PropertySet aSeriesProp(rDataSeries);
                    bool bVaryColorsByPoint = aSeriesProp.getBoolProperty(PROP_VaryColorsByPoint);
                    if (bVaryColorsByPoint || bIsPie)
                    {
                        Sequence<sal_Int32> deletedLegendEntriesSeq;
                        aSeriesProp.getProperty(deletedLegendEntriesSeq, PROP_DeletedLegendEntries);
                        for (const auto& deletedLegendEntry : deletedLegendEntriesSeq)
                        {
                            pFS->startElement(FSNS(XML_c, XML_legendEntry));
                            pFS->singleElement(FSNS(XML_c, XML_idx), XML_val,
                                               OString::number(nIndex + deletedLegendEntry));
                            pFS->singleElement(FSNS(XML_c, XML_delete), XML_val, "1");
                            pFS->endElement(FSNS(XML_c, XML_legendEntry));
                        }
                        Reference<chart2::data::XDataSource> xDSrc(rDataSeries, UNO_QUERY);
                        if (!xDSrc.is())
                            continue;

                        const Sequence<Reference<chart2::data::XLabeledDataSequence>> aDataSeqs = xDSrc->getDataSequences();
                        for (const auto& rDataSeq : aDataSeqs)
                        {
                            Reference<chart2::data::XDataSequence> xValues = rDataSeq->getValues();
                            if (!xValues.is())
                                continue;

                            sal_Int32 nDataSeqSize = xValues->getData().getLength();
                            nIndex += nDataSeqSize;
                        }
                    }
                    else
                    {
                        bShowLegendEntry = aSeriesProp.getBoolProperty(PROP_ShowLegendEntry);
                        if (!bShowLegendEntry)
                        {
                            pFS->startElement(FSNS(XML_c, XML_legendEntry));
                            pFS->singleElement(FSNS(XML_c, XML_idx), XML_val,
                                               OString::number(nIndex));
                            pFS->singleElement(FSNS(XML_c, XML_delete), XML_val, "1");
                            pFS->endElement(FSNS(XML_c, XML_legendEntry));
                        }
                        bSwapXAndY ? nIndex-- : nIndex++;
                    }
                }
                if (bSwapXAndY)
                    nIndex += aDataSeriesSeq.getLength() + 1;
            }
        }

        uno::Any aRelativePos = xProp->getPropertyValue("RelativePosition");
        if (aRelativePos.hasValue())
        {
            pFS->startElement(FSNS(XML_c, XML_layout));
            pFS->startElement(FSNS(XML_c, XML_manualLayout));

            pFS->singleElement(FSNS(XML_c, XML_xMode), XML_val, "edge");
            pFS->singleElement(FSNS(XML_c, XML_yMode), XML_val, "edge");
            chart2::RelativePosition aPos = aRelativePos.get<chart2::RelativePosition>();

            const double x = aPos.Primary;
            const double y = aPos.Secondary;

            pFS->singleElement(FSNS(XML_c, XML_x), XML_val, OString::number(x));
            pFS->singleElement(FSNS(XML_c, XML_y), XML_val, OString::number(y));

            uno::Any aRelativeSize = xProp->getPropertyValue("RelativeSize");
            if (aRelativeSize.hasValue())
            {
                chart2::RelativeSize aSize = aRelativeSize.get<chart2::RelativeSize>();

                const double w = aSize.Primary;
                const double h = aSize.Secondary;

                pFS->singleElement(FSNS(XML_c, XML_w), XML_val, OString::number(w));

                pFS->singleElement(FSNS(XML_c, XML_h), XML_val, OString::number(h));
            }

            SAL_WARN_IF(aPos.Anchor != css::drawing::Alignment_TOP_LEFT, "oox", "unsupported anchor position");

            pFS->endElement(FSNS(XML_c, XML_manualLayout));
            pFS->endElement(FSNS(XML_c, XML_layout));
        }

        if (strPos != nullptr)
        {
            uno::Any aOverlay = xProp->getPropertyValue("Overlay");
            if(aOverlay.get<bool>())
                pFS->singleElement(FSNS(XML_c, XML_overlay), XML_val, "1");
            else
                pFS->singleElement(FSNS(XML_c, XML_overlay), XML_val, "0");
        }

        // shape properties
        exportShapeProps( xProp );

        // draw-chart:txPr text properties
        exportTextProps( xProp );
    }

    pFS->endElement( FSNS( XML_c, XML_legend ) );
}

void ChartExport::exportTitle( const Reference< XShape >& xShape,
    const css::uno::Sequence< uno::Reference< css::chart2::XFormattedString > >& xFormattedSubTitle )
{
    Sequence< uno::Reference< chart2::XFormattedString > > xFormattedTitle;
    Reference< beans::XPropertySet > xPropSet( xShape, uno::UNO_QUERY );
    if( xPropSet.is())
    {
        xPropSet->getPropertyValue("FormattedStrings") >>= xFormattedTitle;
    }

    // tdf#101322: add subtitle to title
    if (xFormattedSubTitle.hasElements())
    {
        if (!xFormattedTitle.hasElements())
        {
            xFormattedTitle = xFormattedSubTitle;
        }
        else
        {
            sal_uInt32 nLength = xFormattedTitle.size();
            const OUString aLastString = xFormattedTitle.getArray()[nLength - 1]->getString();
            xFormattedTitle.getArray()[nLength - 1]->setString(aLastString + OUStringChar('\n'));
            for (const uno::Reference<chart2::XFormattedString>& rxFS : xFormattedSubTitle)
            {
                if (!rxFS->getString().isEmpty())
                {
                    xFormattedTitle.realloc(nLength + 1);
                    xFormattedTitle.getArray()[nLength++] = rxFS;
                }
            }
        }
    }

    if (!xFormattedTitle.hasElements())
        return;

    FSHelperPtr pFS = GetFS();
    pFS->startElement(FSNS(XML_c, XML_title));

    pFS->startElement(FSNS(XML_c, XML_tx));
    pFS->startElement(FSNS(XML_c, XML_rich));

    // TODO: bodyPr
    const char* sWritingMode = nullptr;
    bool bVertical = false;
    xPropSet->getPropertyValue("StackedText") >>= bVertical;
    if( bVertical )
        sWritingMode = "wordArtVert";

    sal_Int32 nRotation = 0;
    xPropSet->getPropertyValue("TextRotation") >>= nRotation;

    pFS->singleElement( FSNS( XML_a, XML_bodyPr ),
            XML_vert, sWritingMode,
            XML_rot, oox::drawingml::calcRotationValue(nRotation) );
    // TODO: lstStyle
    pFS->singleElement(FSNS(XML_a, XML_lstStyle));
    pFS->startElement(FSNS(XML_a, XML_p));

    pFS->startElement(FSNS(XML_a, XML_pPr));

    bool bDummy = false;
    sal_Int32 nDummy;
    WriteRunProperties(xPropSet, false, XML_defRPr, true, bDummy, nDummy );

    pFS->endElement( FSNS( XML_a, XML_pPr ) );

    for (const uno::Reference<chart2::XFormattedString>& rxFS : xFormattedTitle)
    {
        pFS->startElement(FSNS(XML_a, XML_r));
        bDummy = false;
        Reference< beans::XPropertySet > xRunPropSet(rxFS, uno::UNO_QUERY);
        WriteRunProperties(xRunPropSet, false, XML_rPr, true, bDummy, nDummy);
        pFS->startElement(FSNS(XML_a, XML_t));

        // the linebreak should always be at the end of the XFormattedString text
        bool bNextPara = rxFS->getString().endsWith(u"\n");
        if (!bNextPara)
            pFS->writeEscaped(rxFS->getString());
        else
        {
            sal_Int32 nEnd = rxFS->getString().lastIndexOf('\n');
            pFS->writeEscaped(rxFS->getString().replaceAt(nEnd, 1, u""));
        }
        pFS->endElement(FSNS(XML_a, XML_t));
        pFS->endElement(FSNS(XML_a, XML_r));

        if (bNextPara)
        {
            pFS->endElement(FSNS(XML_a, XML_p));

            pFS->startElement(FSNS(XML_a, XML_p));
            pFS->startElement(FSNS(XML_a, XML_pPr));
            bDummy = false;
            WriteRunProperties(xPropSet, false, XML_defRPr, true, bDummy, nDummy);
            pFS->endElement(FSNS(XML_a, XML_pPr));
        }
    }

    pFS->endElement( FSNS( XML_a, XML_p ) );

    pFS->endElement( FSNS( XML_c, XML_rich ) );
    pFS->endElement( FSNS( XML_c, XML_tx ) );

    uno::Any aManualLayout = xPropSet->getPropertyValue("RelativePosition");
    if (aManualLayout.hasValue())
    {
        pFS->startElement(FSNS(XML_c, XML_layout));
        pFS->startElement(FSNS(XML_c, XML_manualLayout));
        pFS->singleElement(FSNS(XML_c, XML_xMode), XML_val, "edge");
        pFS->singleElement(FSNS(XML_c, XML_yMode), XML_val, "edge");

        Reference<embed::XVisualObject> xVisObject(mxChartModel, uno::UNO_QUERY);
        awt::Size aPageSize = xVisObject->getVisualAreaSize(embed::Aspects::MSOLE_CONTENT);

        awt::Size aSize = xShape->getSize();
        awt::Point aPos2 = xShape->getPosition();
        // rotated shapes need special handling...
        double fSin = fabs(sin(basegfx::deg2rad<100>(nRotation)));
        // remove part of height from X direction, if title is rotated down
        if( nRotation*0.01 > 180.0 )
            aPos2.X -= static_cast<sal_Int32>(fSin * aSize.Height + 0.5);
        // remove part of width from Y direction, if title is rotated up
        else if( nRotation*0.01 > 0.0 )
            aPos2.Y -= static_cast<sal_Int32>(fSin * aSize.Width + 0.5);

        double x = static_cast<double>(aPos2.X) / static_cast<double>(aPageSize.Width);
        double y = static_cast<double>(aPos2.Y) / static_cast<double>(aPageSize.Height);
        /*
        pFS->singleElement(FSNS(XML_c, XML_wMode), XML_val, "edge");
        pFS->singleElement(FSNS(XML_c, XML_hMode), XML_val, "edge");
                */
        pFS->singleElement(FSNS(XML_c, XML_x), XML_val, OString::number(x));
        pFS->singleElement(FSNS(XML_c, XML_y), XML_val, OString::number(y));
        /*
        pFS->singleElement(FSNS(XML_c, XML_w), XML_val, "");
        pFS->singleElement(FSNS(XML_c, XML_h), XML_val, "");
                */
        pFS->endElement(FSNS(XML_c, XML_manualLayout));
        pFS->endElement(FSNS(XML_c, XML_layout));
    }

    pFS->singleElement(FSNS(XML_c, XML_overlay), XML_val, "0");

    // shape properties
    if( xPropSet.is() )
    {
        exportShapeProps( xPropSet );
    }

    pFS->endElement( FSNS( XML_c, XML_title ) );
}

namespace {

    std::vector<Sequence<Reference<chart2::XDataSeries> > > splitDataSeriesByAxis(const Reference< chart2::XChartType >& xChartType)
    {
        std::vector<Sequence<Reference<chart2::XDataSeries> > > aSplitSeries;
        std::map<sal_Int32, size_t> aMapAxisToIndex;

        Reference< chart2::XDataSeriesContainer > xDSCnt(xChartType, uno::UNO_QUERY);
        if (xDSCnt.is())
        {
            sal_Int32 nAxisIndexOfFirstSeries = -1;
            const Sequence< Reference< chart2::XDataSeries > > aSeriesSeq(xDSCnt->getDataSeries());
            for (const uno::Reference<chart2::XDataSeries>& xSeries : aSeriesSeq)
            {
                Reference<beans::XPropertySet> xPropSet(xSeries, uno::UNO_QUERY);
                if (!xPropSet.is())
                    continue;

                sal_Int32 nAxisIndex = -1;
                uno::Any aAny = xPropSet->getPropertyValue("AttachedAxisIndex");
                aAny >>= nAxisIndex;
                size_t nVectorPos = 0;
                if (nAxisIndexOfFirstSeries == -1)
                {
                    nAxisIndexOfFirstSeries = nAxisIndex;
                }

                auto it = aMapAxisToIndex.find(nAxisIndex);
                if (it == aMapAxisToIndex.end())
                {
                    aSplitSeries.emplace_back();
                    nVectorPos = aSplitSeries.size() - 1;
                    aMapAxisToIndex.insert(std::pair<sal_Int32, size_t>(nAxisIndex, nVectorPos));
                }
                else
                {
                    nVectorPos = it->second;
                }

                uno::Sequence<Reference<chart2::XDataSeries> >& rAxisSeriesSeq = aSplitSeries[nVectorPos];
                sal_Int32 nLength = rAxisSeriesSeq.getLength();
                rAxisSeriesSeq.realloc(nLength + 1);
                rAxisSeriesSeq.getArray()[nLength] = xSeries;
            }
            // if the first series attached to secondary axis, then export those series first, which are attached to primary axis
            // also the MS Office export every time in this order
            if (aSplitSeries.size() > 1 && nAxisIndexOfFirstSeries == 1)
            {
                std::swap(aSplitSeries[0], aSplitSeries[1]);
            }
        }

        return aSplitSeries;
    }

}

void ChartExport::exportPlotArea(const Reference< css::chart::XChartDocument >& xChartDoc)
{
    Reference< chart2::XCoordinateSystemContainer > xBCooSysCnt( mxNewDiagram, uno::UNO_QUERY );
    if( ! xBCooSysCnt.is())
        return;

    // plot-area element

    FSHelperPtr pFS = GetFS();
    pFS->startElement(FSNS(XML_c, XML_plotArea));

    Reference<beans::XPropertySet> xWall(mxNewDiagram, uno::UNO_QUERY);
    if( xWall.is() )
    {
        uno::Any aAny = xWall->getPropertyValue("RelativePosition");
        if (aAny.hasValue())
        {
            chart2::RelativePosition aPos = aAny.get<chart2::RelativePosition>();
            aAny = xWall->getPropertyValue("RelativeSize");
            chart2::RelativeSize aSize = aAny.get<chart2::RelativeSize>();
            uno::Reference< css::chart::XDiagramPositioning > xDiagramPositioning( xChartDoc->getDiagram(), uno::UNO_QUERY );
            exportManualLayout(aPos, aSize, xDiagramPositioning->isExcludingDiagramPositioning() );
        }
    }

    // chart type
    const Sequence< Reference< chart2::XCoordinateSystem > >
        aCooSysSeq( xBCooSysCnt->getCoordinateSystems());

    // tdf#123647 Save empty chart as empty bar chart.
    if (!aCooSysSeq.hasElements())
    {
        pFS->startElement(FSNS(XML_c, XML_barChart));
        pFS->singleElement(FSNS(XML_c, XML_barDir), XML_val, "col");
        pFS->singleElement(FSNS(XML_c, XML_grouping), XML_val, "clustered");
        pFS->singleElement(FSNS(XML_c, XML_varyColors), XML_val, "0");
        exportAxesId(true);
        pFS->endElement(FSNS(XML_c, XML_barChart));
    }

    for( const auto& rCS : aCooSysSeq )
    {
        Reference< chart2::XChartTypeContainer > xCTCnt( rCS, uno::UNO_QUERY );
        if( ! xCTCnt.is())
            continue;
        mnSeriesCount=0;
        const Sequence< Reference< chart2::XChartType > > aCTSeq( xCTCnt->getChartTypes());
        for( const auto& rCT : aCTSeq )
        {
            Reference< chart2::XDataSeriesContainer > xDSCnt( rCT, uno::UNO_QUERY );
            if( ! xDSCnt.is())
                return;
            Reference< chart2::XChartType > xChartType( rCT, uno::UNO_QUERY );
            if( ! xChartType.is())
                continue;
            // note: if xDSCnt.is() then also aCTSeq[nCTIdx]
            OUString aChartType( xChartType->getChartType());
            sal_Int32 eChartType = lcl_getChartType( aChartType );
            switch( eChartType )
            {
                case chart::TYPEID_BAR:
                    {
                        exportBarChart( xChartType );
                        break;
                    }
                case chart::TYPEID_AREA:
                    {
                        exportAreaChart( xChartType );
                        break;
                    }
                case chart::TYPEID_LINE:
                    {
                        exportLineChart( xChartType );
                        break;
                    }
                case chart::TYPEID_BUBBLE:
                    {
                        exportBubbleChart( xChartType );
                        break;
                    }
                case chart::TYPEID_DOUGHNUT: // doesn't currently happen
                case chart::TYPEID_OFPIE:    // doesn't currently happen
                case chart::TYPEID_PIE:
                    {
                        sal_Int32 eCT = getChartType( );
                        if(eCT == chart::TYPEID_DOUGHNUT)
                        {
                            exportDoughnutChart( xChartType );
                        }
                        else
                        {

                            PropertySet xChartTypeProp(rCT);
                            chart2::PieChartSubType subtype(chart2::PieChartSubType_NONE);
                            if (!xChartTypeProp.getProperty(subtype, PROP_SubPieType))
                            {
                                subtype = chart2::PieChartSubType_NONE;
                            }
                            if (subtype != chart2::PieChartSubType_NONE)
                            {
                                const char* sSubType = "pie";   // default
                                switch (subtype) {
                                    case chart2::PieChartSubType_PIE:
                                        sSubType = "pie";
                                        break;
                                    case chart2::PieChartSubType_BAR:
                                        sSubType = "bar";
                                        break;
                                    default:
                                        assert(false);
                                }

                                exportOfPieChart(xChartType, sSubType);
                            } else {
                                exportPieChart( xChartType );
                            }
                        }
                        break;
                    }
                case chart::TYPEID_RADARLINE:
                case chart::TYPEID_RADARAREA:
                    {
                        exportRadarChart( xChartType );
                        break;
                    }
                case chart::TYPEID_SCATTER:
                    {
                        exportScatterChart( xChartType );
                        break;
                    }
                case chart::TYPEID_STOCK:
                    {
                        exportStockChart( xChartType );
                        break;
                    }
                case chart::TYPEID_SURFACE:
                    {
                        exportSurfaceChart( xChartType );
                        break;
                    }
                default:
                    {
                        SAL_WARN("oox", "ChartExport::exportPlotArea -- not support chart type");
                        break;
                    }
            }

        }
    }
    //Axis Data
    exportAxes( );

    // Data Table
    exportDataTable();

    // shape properties
    /*
     * Export the Plot area Shape Properties
     * eg: Fill and Outline
     */
    Reference< css::chart::X3DDisplay > xWallFloorSupplier( mxDiagram, uno::UNO_QUERY );
    // tdf#114139 For 2D charts Plot Area equivalent is Chart Wall.
    // Unfortunately LibreOffice doesn't have Plot Area equivalent for 3D charts.
    // It means that Plot Area couldn't be displayed and changed for 3D chars in LibreOffice.
    // We cannot write Wall attributes into Plot Area for 3D charts, because Wall us used as background wall.
    if( !mbIs3DChart && xWallFloorSupplier.is() )
    {
        Reference< beans::XPropertySet > xWallPropSet = xWallFloorSupplier->getWall();
        if( xWallPropSet.is() )
        {
            uno::Any aAny = xWallPropSet->getPropertyValue("LineStyle");
            sal_Int32 eChartType = getChartType( );
            // Export LineStyle_NONE instead of default linestyle of PlotArea border, because LibreOffice
            // make invisible the Wall shape properties, in case of these charts. Or in the future set
            // the default LineStyle of these charts to LineStyle_NONE.
            bool noSupportWallProp = ( (eChartType == chart::TYPEID_PIE) || (eChartType == chart::TYPEID_RADARLINE) || (eChartType == chart::TYPEID_RADARAREA) );
            if ( noSupportWallProp && (aAny != drawing::LineStyle_NONE) )
            {
                xWallPropSet->setPropertyValue( "LineStyle", uno::Any(drawing::LineStyle_NONE) );
            }
            exportShapeProps( xWallPropSet );
        }
    }

    pFS->endElement( FSNS( XML_c, XML_plotArea ) );

}

void ChartExport::exportManualLayout(const css::chart2::RelativePosition& rPos,
                                     const css::chart2::RelativeSize& rSize,
                                     const bool bIsExcludingDiagramPositioning)
{
    FSHelperPtr pFS = GetFS();
    pFS->startElement(FSNS(XML_c, XML_layout));
    pFS->startElement(FSNS(XML_c, XML_manualLayout));

    // By default layoutTarget is set to "outer" and we shouldn't save it in that case
    if ( bIsExcludingDiagramPositioning )
    {
        pFS->singleElement(FSNS(XML_c, XML_layoutTarget), XML_val, "inner");
    }
    pFS->singleElement(FSNS(XML_c, XML_xMode), XML_val, "edge");
    pFS->singleElement(FSNS(XML_c, XML_yMode), XML_val, "edge");

    double x = rPos.Primary;
    double y = rPos.Secondary;
    const double w = rSize.Primary;
    const double h = rSize.Secondary;
    switch (rPos.Anchor)
    {
        case drawing::Alignment_LEFT:
            y -= (h/2);
        break;
        case drawing::Alignment_TOP_LEFT:
        break;
        case drawing::Alignment_BOTTOM_LEFT:
            y -= h;
        break;
        case drawing::Alignment_TOP:
            x -= (w/2);
        break;
        case drawing::Alignment_CENTER:
            x -= (w/2);
            y -= (h/2);
        break;
        case drawing::Alignment_BOTTOM:
            x -= (w/2);
            y -= h;
        break;
        case drawing::Alignment_TOP_RIGHT:
            x -= w;
        break;
        case drawing::Alignment_BOTTOM_RIGHT:
            x -= w;
            y -= h;
        break;
        case drawing::Alignment_RIGHT:
            y -= (h/2);
            x -= w;
        break;
        default:
            SAL_WARN("oox", "unhandled alignment case for manual layout export " << static_cast<sal_uInt16>(rPos.Anchor));
    }

    pFS->singleElement(FSNS(XML_c, XML_x), XML_val, OString::number(x));

    pFS->singleElement(FSNS(XML_c, XML_y), XML_val, OString::number(y));

    pFS->singleElement(FSNS(XML_c, XML_w), XML_val, OString::number(w));

    pFS->singleElement(FSNS(XML_c, XML_h), XML_val, OString::number(h));

    pFS->endElement(FSNS(XML_c, XML_manualLayout));
    pFS->endElement(FSNS(XML_c, XML_layout));
}

void ChartExport::exportFill( const Reference< XPropertySet >& xPropSet )
{
    // Similar to DrawingML::WriteFill, but gradient access via name
    if (!GetProperty( xPropSet, "FillStyle" ))
        return;
    FillStyle aFillStyle(FillStyle_NONE);
    mAny >>= aFillStyle;

    // map full transparent background to no fill
    if (aFillStyle == FillStyle_SOLID && GetProperty( xPropSet, "FillTransparence" ))
    {
        sal_Int16 nVal = 0;
        mAny >>= nVal;
        if ( nVal == 100 )
            aFillStyle = FillStyle_NONE;
    }
    OUString sFillTransparenceGradientName;
    if (aFillStyle == FillStyle_SOLID
        && GetProperty(xPropSet, "FillTransparenceGradientName") && (mAny >>= sFillTransparenceGradientName)
        && !sFillTransparenceGradientName.isEmpty())
    {
        awt::Gradient aTransparenceGradient;
        uno::Reference< lang::XMultiServiceFactory > xFact( getModel(), uno::UNO_QUERY );
        uno::Reference< container::XNameAccess > xTransparenceGradient(xFact->createInstance("com.sun.star.drawing.TransparencyGradientTable"), uno::UNO_QUERY);
        uno::Any rTransparenceValue = xTransparenceGradient->getByName(sFillTransparenceGradientName);
        rTransparenceValue >>= aTransparenceGradient;
        if (aTransparenceGradient.StartColor == 0xffffff && aTransparenceGradient.EndColor == 0xffffff)
            aFillStyle = FillStyle_NONE;
    }
    switch( aFillStyle )
    {
        case FillStyle_SOLID:
            exportSolidFill(xPropSet);
            break;
        case FillStyle_GRADIENT :
            exportGradientFill( xPropSet );
            break;
        case FillStyle_BITMAP :
            exportBitmapFill( xPropSet );
            break;
        case FillStyle_HATCH:
            exportHatch(xPropSet);
            break;
        case FillStyle_NONE:
            mpFS->singleElementNS(XML_a, XML_noFill);
            break;
        default:
            ;
    }
}

void ChartExport::exportSolidFill(const Reference< XPropertySet >& xPropSet)
{
    // Similar to DrawingML::WriteSolidFill, but gradient access via name
    // and currently no InteropGrabBag
    // get fill color
    sal_uInt32 nFillColor = 0;
    if (!GetProperty(xPropSet, "FillColor") || !(mAny >>= nFillColor))
        return;

    sal_Int32 nAlpha = MAX_PERCENT;
    if (GetProperty( xPropSet, "FillTransparence" ))
    {
        sal_Int32 nTransparency = 0;
        mAny >>= nTransparency;
        // Calculate alpha value (see oox/source/drawingml/color.cxx : getTransparency())
        nAlpha = (MAX_PERCENT - ( PER_PERCENT * nTransparency ) );
    }
    // OOXML has no separate transparence gradient but uses transparency in the gradient stops.
    // So we merge transparency and color and use gradient fill in such case.
    basegfx::BGradient aTransparenceGradient;
    bool bNeedGradientFill(false);
    OUString sFillTransparenceGradientName;

    if (GetProperty(xPropSet, "FillTransparenceGradientName")
        && (mAny >>= sFillTransparenceGradientName)
        && !sFillTransparenceGradientName.isEmpty())
    {
        uno::Reference< lang::XMultiServiceFactory > xFact( getModel(), uno::UNO_QUERY );
        uno::Reference< container::XNameAccess > xTransparenceGradient(xFact->createInstance("com.sun.star.drawing.TransparencyGradientTable"), uno::UNO_QUERY);
        const uno::Any rTransparenceAny = xTransparenceGradient->getByName(sFillTransparenceGradientName);

        aTransparenceGradient = model::gradient::getFromAny(rTransparenceAny);
        basegfx::BColor aSingleColor;
        bNeedGradientFill = !aTransparenceGradient.GetColorStops().isSingleColor(aSingleColor);

        if (!bNeedGradientFill)
        {
            // Our alpha is a single gray color value.
            const sal_uInt8 nRed(aSingleColor.getRed() * 255.0);

            // drawingML alpha is a percentage on a 0..100000 scale.
            nAlpha = (255 - nRed) * oox::drawingml::MAX_PERCENT / 255;
        }
    }
    // write XML
    if (bNeedGradientFill)
    {
        // no longer create copy/PseudoColorGradient, use new API of
        // WriteGradientFill to express fix fill color
        mpFS->startElementNS(XML_a, XML_gradFill, XML_rotWithShape, "0");
        WriteGradientFill(nullptr, nFillColor, &aTransparenceGradient);
        mpFS->endElementNS(XML_a, XML_gradFill);
    }
    else
        WriteSolidFill(::Color(ColorTransparency, nFillColor & 0xffffff), nAlpha);
}

void ChartExport::exportHatch( const Reference< XPropertySet >& xPropSet )
{
    if (!xPropSet.is())
        return;

    if (GetProperty(xPropSet, "FillHatchName"))
    {
        OUString aHatchName;
        mAny >>= aHatchName;
        uno::Reference< lang::XMultiServiceFactory > xFact( getModel(), uno::UNO_QUERY );
        uno::Reference< container::XNameAccess > xHatchTable( xFact->createInstance("com.sun.star.drawing.HatchTable"), uno::UNO_QUERY );
        uno::Any rValue = xHatchTable->getByName(aHatchName);
        css::drawing::Hatch aHatch;
        rValue >>= aHatch;
        WritePattFill(xPropSet, aHatch);
    }

}

void ChartExport::exportBitmapFill( const Reference< XPropertySet >& xPropSet )
{
    if( !xPropSet.is() )
        return;

    OUString sFillBitmapName;
    xPropSet->getPropertyValue("FillBitmapName") >>= sFillBitmapName;

    uno::Reference< lang::XMultiServiceFactory > xFact( getModel(), uno::UNO_QUERY );
    try
    {
        uno::Reference< container::XNameAccess > xBitmapTable( xFact->createInstance("com.sun.star.drawing.BitmapTable"), uno::UNO_QUERY );
        uno::Any rValue = xBitmapTable->getByName( sFillBitmapName );
        if (rValue.has<uno::Reference<awt::XBitmap>>())
        {
            uno::Reference<awt::XBitmap> xBitmap = rValue.get<uno::Reference<awt::XBitmap>>();
            uno::Reference<graphic::XGraphic> xGraphic(xBitmap, uno::UNO_QUERY);
            if (xGraphic.is())
            {
                WriteXGraphicBlipFill(xPropSet, xGraphic, XML_a, true, true);
            }
        }
    }
    catch (const uno::Exception &)
    {
        TOOLS_WARN_EXCEPTION("oox", "ChartExport::exportBitmapFill");
    }
}

void ChartExport::exportGradientFill( const Reference< XPropertySet >& xPropSet )
{
    if( !xPropSet.is() )
        return;

    OUString sFillGradientName;
    xPropSet->getPropertyValue("FillGradientName") >>= sFillGradientName;

    uno::Reference< lang::XMultiServiceFactory > xFact( getModel(), uno::UNO_QUERY );
    try
    {
        uno::Reference< container::XNameAccess > xGradient( xFact->createInstance("com.sun.star.drawing.GradientTable"), uno::UNO_QUERY );
        const uno::Any rGradientAny(xGradient->getByName( sFillGradientName ));
        const basegfx::BGradient aGradient = model::gradient::getFromAny(rGradientAny);
        basegfx::BColor aSingleColor;

        if (!aGradient.GetColorStops().isSingleColor(aSingleColor))
        {
            basegfx::BGradient aTransparenceGradient;
            mpFS->startElementNS(XML_a, XML_gradFill);
            OUString sFillTransparenceGradientName;

            if( (xPropSet->getPropertyValue("FillTransparenceGradientName") >>= sFillTransparenceGradientName) && !sFillTransparenceGradientName.isEmpty())
            {
                uno::Reference< container::XNameAccess > xTransparenceGradient(xFact->createInstance("com.sun.star.drawing.TransparencyGradientTable"), uno::UNO_QUERY);
                const uno::Any rTransparenceAny(xTransparenceGradient->getByName(sFillTransparenceGradientName));

                aTransparenceGradient = model::gradient::getFromAny(rTransparenceAny);

                WriteGradientFill(&aGradient, 0, &aTransparenceGradient);
            }
            else if (GetProperty(xPropSet, "FillTransparence") )
            {
                // no longer create PseudoTransparencyGradient, use new API of
                // WriteGradientFill to express fix transparency
                sal_Int32 nTransparency = 0;
                mAny >>= nTransparency;
                // nTransparency is [0..100]%
                WriteGradientFill(&aGradient, 0, nullptr, nTransparency * 0.01);
            }
            else
            {
                WriteGradientFill(&aGradient, 0, nullptr);
            }

            mpFS->endElementNS(XML_a, XML_gradFill);
        }
    }
    catch (const uno::Exception &)
    {
        TOOLS_INFO_EXCEPTION("oox", "ChartExport::exportGradientFill");
    }
}

void ChartExport::exportDataTable( )
{
    auto xDataTable = mxNewDiagram->getDataTable();
    if (!xDataTable.is())
        return;

    FSHelperPtr pFS = GetFS();
    uno::Reference<beans::XPropertySet> aPropSet(xDataTable, uno::UNO_QUERY);

    bool bShowVBorder = false;
    bool bShowHBorder = false;
    bool bShowOutline = false;
    bool bShowKeys = false;

    if (GetProperty(aPropSet, "HBorder"))
        mAny >>= bShowHBorder;
    if (GetProperty(aPropSet, "VBorder"))
        mAny >>= bShowVBorder;
    if (GetProperty(aPropSet, "Outline"))
        mAny >>= bShowOutline;
    if (GetProperty(aPropSet, "Keys"))
        mAny >>= bShowKeys;

    pFS->startElement(FSNS(XML_c, XML_dTable));

    if (bShowHBorder)
        pFS->singleElement(FSNS(XML_c, XML_showHorzBorder), XML_val, "1" );
    if (bShowVBorder)
        pFS->singleElement(FSNS(XML_c, XML_showVertBorder), XML_val, "1");
    if (bShowOutline)
        pFS->singleElement(FSNS(XML_c, XML_showOutline), XML_val, "1");
    if (bShowKeys)
        pFS->singleElement(FSNS(XML_c, XML_showKeys), XML_val, "1");

    exportShapeProps(aPropSet);
    exportTextProps(aPropSet);

    pFS->endElement(FSNS(XML_c, XML_dTable));
}

void ChartExport::exportAreaChart( const Reference< chart2::XChartType >& xChartType )
{
    FSHelperPtr pFS = GetFS();
    const std::vector<Sequence<Reference<chart2::XDataSeries> > > aSplitDataSeries = splitDataSeriesByAxis(xChartType);
    for (const auto& splitDataSeries : aSplitDataSeries)
    {
        if (!splitDataSeries.hasElements())
            continue;

        sal_Int32 nTypeId = XML_areaChart;
        if (mbIs3DChart)
            nTypeId = XML_area3DChart;
        pFS->startElement(FSNS(XML_c, nTypeId));

        exportGrouping();
        bool bPrimaryAxes = true;
        exportSeries(xChartType, splitDataSeries, bPrimaryAxes);
        exportAxesId(bPrimaryAxes);

        pFS->endElement(FSNS(XML_c, nTypeId));
    }
}

void ChartExport::exportBarChart(const Reference< chart2::XChartType >& xChartType)
{
    sal_Int32 nTypeId = XML_barChart;
    if (mbIs3DChart)
        nTypeId = XML_bar3DChart;
    FSHelperPtr pFS = GetFS();

    const std::vector<Sequence<Reference<chart2::XDataSeries> > > aSplitDataSeries = splitDataSeriesByAxis(xChartType);
    for (const auto& splitDataSeries : aSplitDataSeries)
    {
        if (!splitDataSeries.hasElements())
            continue;

        pFS->startElement(FSNS(XML_c, nTypeId));
        // bar direction
        bool bVertical = false;
        Reference< XPropertySet > xPropSet(mxDiagram, uno::UNO_QUERY);
        if (GetProperty(xPropSet, "Vertical"))
            mAny >>= bVertical;

        const char* bardir = bVertical ? "bar" : "col";
        pFS->singleElement(FSNS(XML_c, XML_barDir), XML_val, bardir);

        exportGrouping(true);

        exportVaryColors(xChartType);

        bool bPrimaryAxes = true;
        exportSeries(xChartType, splitDataSeries, bPrimaryAxes);

        Reference< XPropertySet > xTypeProp(xChartType, uno::UNO_QUERY);

        if (xTypeProp.is() && GetProperty(xTypeProp, "GapwidthSequence"))
        {
            uno::Sequence< sal_Int32 > aBarPositionSequence;
            mAny >>= aBarPositionSequence;
            if (aBarPositionSequence.hasElements())
            {
                sal_Int32 nGapWidth = aBarPositionSequence[0];
                pFS->singleElement(FSNS(XML_c, XML_gapWidth), XML_val, OString::number(nGapWidth));
            }
        }

        if (mbIs3DChart)
        {
            // Shape
            namespace cssc = css::chart;
            sal_Int32 nGeom3d = cssc::ChartSolidType::RECTANGULAR_SOLID;
            if (xPropSet.is() && GetProperty(xPropSet, "SolidType"))
                mAny >>= nGeom3d;
            const char* sShapeType = nullptr;
            switch (nGeom3d)
            {
            case cssc::ChartSolidType::RECTANGULAR_SOLID:
                sShapeType = "box";
                break;
            case cssc::ChartSolidType::CONE:
                sShapeType = "cone";
                break;
            case cssc::ChartSolidType::CYLINDER:
                sShapeType = "cylinder";
                break;
            case cssc::ChartSolidType::PYRAMID:
                sShapeType = "pyramid";
                break;
            }
            pFS->singleElement(FSNS(XML_c, XML_shape), XML_val, sShapeType);
        }

        //overlap
        if (!mbIs3DChart && xTypeProp.is() && GetProperty(xTypeProp, "OverlapSequence"))
        {
            uno::Sequence< sal_Int32 > aBarPositionSequence;
            mAny >>= aBarPositionSequence;
            if (aBarPositionSequence.hasElements())
            {
                sal_Int32 nOverlap = aBarPositionSequence[0];
                // Stacked/Percent Bar/Column chart Overlap-workaround
                // Export the Overlap value with 100% for stacked charts,
                // because the default overlap value of the Bar/Column chart is 0% and
                // LibreOffice do nothing with the overlap value in Stacked charts case,
                // unlike the MS Office, which is interpreted differently.
                if ((mbStacked || mbPercent) && nOverlap != 100)
                {
                    nOverlap = 100;
                    pFS->singleElement(FSNS(XML_c, XML_overlap), XML_val, OString::number(nOverlap));
                }
                else // Normal bar chart
                {
                    pFS->singleElement(FSNS(XML_c, XML_overlap), XML_val, OString::number(nOverlap));
                }
            }
        }

        exportAxesId(bPrimaryAxes);

        pFS->endElement(FSNS(XML_c, nTypeId));
    }
}

void ChartExport::exportBubbleChart( const Reference< chart2::XChartType >& xChartType )
{
    FSHelperPtr pFS = GetFS();
    const std::vector<Sequence<Reference<chart2::XDataSeries> > > aSplitDataSeries = splitDataSeriesByAxis(xChartType);
    for (const auto& splitDataSeries : aSplitDataSeries)
    {
        if (!splitDataSeries.hasElements())
            continue;

        pFS->startElement(FSNS(XML_c, XML_bubbleChart));

        exportVaryColors(xChartType);

        bool bPrimaryAxes = true;
        exportSeries(xChartType, splitDataSeries, bPrimaryAxes);

        exportAxesId(bPrimaryAxes);

        pFS->endElement(FSNS(XML_c, XML_bubbleChart));
    }
}

void ChartExport::exportDoughnutChart( const Reference< chart2::XChartType >& xChartType )
{
    FSHelperPtr pFS = GetFS();
    pFS->startElement(FSNS(XML_c, XML_doughnutChart));

    exportVaryColors(xChartType);

    bool bPrimaryAxes = true;
    exportAllSeries(xChartType, bPrimaryAxes);
    // firstSliceAng
    exportFirstSliceAng( );
    //FIXME: holeSize
    pFS->singleElement(FSNS(XML_c, XML_holeSize), XML_val, OString::number(50));

    pFS->endElement( FSNS( XML_c, XML_doughnutChart ) );
}

void ChartExport::exportOfPieChart(
        const Reference< chart2::XChartType >& xChartType,
        const char* sSubType )
{
    FSHelperPtr pFS = GetFS();
    pFS->startElement(FSNS(XML_c, XML_ofPieChart));

    pFS->singleElement(FSNS(XML_c, XML_ofPieType), XML_val, sSubType);

    exportVaryColors(xChartType);

    bool bPrimaryAxes = true;
    exportAllSeries(xChartType, bPrimaryAxes);

    pFS->endElement( FSNS( XML_c, XML_ofPieChart ) );
}

namespace {

void writeDataLabelsRange(const FSHelperPtr& pFS, const XmlFilterBase* pFB, DataLabelsRange& rDLblsRange)
{
    if (rDLblsRange.empty())
        return;

    pFS->startElement(FSNS(XML_c, XML_extLst));
    pFS->startElement(FSNS(XML_c, XML_ext), XML_uri, "{02D57815-91ED-43cb-92C2-25804820EDAC}", FSNS(XML_xmlns, XML_c15), pFB->getNamespaceURL(OOX_NS(c15)));
    pFS->startElement(FSNS(XML_c15, XML_datalabelsRange));

    // Write cell range.
    pFS->startElement(FSNS(XML_c15, XML_f));
    pFS->writeEscaped(rDLblsRange.getRange());
    pFS->endElement(FSNS(XML_c15, XML_f));

    // Write all labels.
    pFS->startElement(FSNS(XML_c15, XML_dlblRangeCache));
    pFS->singleElement(FSNS(XML_c, XML_ptCount), XML_val, OString::number(rDLblsRange.count()));
    for (const auto& rLabelKV: rDLblsRange)
    {
        pFS->startElement(FSNS(XML_c, XML_pt), XML_idx, OString::number(rLabelKV.first));
        pFS->startElement(FSNS(XML_c, XML_v));
        pFS->writeEscaped(rLabelKV.second);
        pFS->endElement(FSNS( XML_c, XML_v ));
        pFS->endElement(FSNS(XML_c, XML_pt));
    }

    pFS->endElement(FSNS(XML_c15, XML_dlblRangeCache));

    pFS->endElement(FSNS(XML_c15, XML_datalabelsRange));
    pFS->endElement(FSNS(XML_c, XML_ext));
    pFS->endElement(FSNS(XML_c, XML_extLst));
}

}

void ChartExport::exportLineChart( const Reference< chart2::XChartType >& xChartType )
{
    FSHelperPtr pFS = GetFS();
    const std::vector<Sequence<Reference<chart2::XDataSeries> > > aSplitDataSeries = splitDataSeriesByAxis(xChartType);
    for (const auto& splitDataSeries : aSplitDataSeries)
    {
        if (!splitDataSeries.hasElements())
            continue;

        sal_Int32 nTypeId = XML_lineChart;
        if( mbIs3DChart )
            nTypeId = XML_line3DChart;
        pFS->startElement(FSNS(XML_c, nTypeId));

        exportGrouping( );

        exportVaryColors(xChartType);
        // TODO: show marker symbol in series?
        bool bPrimaryAxes = true;
        exportSeries(xChartType, splitDataSeries, bPrimaryAxes);

        // show marker?
        sal_Int32 nSymbolType = css::chart::ChartSymbolType::NONE;
        Reference< XPropertySet > xPropSet( mxDiagram , uno::UNO_QUERY);
        if( GetProperty( xPropSet, "SymbolType" ) )
            mAny >>= nSymbolType;

        if( !mbIs3DChart )
        {
            exportHiLowLines();
            exportUpDownBars(xChartType);
            const char* marker = nSymbolType == css::chart::ChartSymbolType::NONE? "0":"1";
            pFS->singleElement(FSNS(XML_c, XML_marker), XML_val, marker);
        }

        exportAxesId(bPrimaryAxes, true);

        pFS->endElement( FSNS( XML_c, nTypeId ) );
    }
}

void ChartExport::exportPieChart( const Reference< chart2::XChartType >& xChartType )
{
    FSHelperPtr pFS = GetFS();
    sal_Int32 nTypeId = XML_pieChart;
    if( mbIs3DChart )
        nTypeId = XML_pie3DChart;
    pFS->startElement(FSNS(XML_c, nTypeId));

    exportVaryColors(xChartType);

    bool bPrimaryAxes = true;
    exportAllSeries(xChartType, bPrimaryAxes);

    if( !mbIs3DChart )
    {
        // firstSliceAng
        exportFirstSliceAng( );
    }

    pFS->endElement( FSNS( XML_c, nTypeId ) );
}

void ChartExport::exportRadarChart( const Reference< chart2::XChartType >& xChartType)
{
    FSHelperPtr pFS = GetFS();
    pFS->startElement(FSNS(XML_c, XML_radarChart));

    // radarStyle
    sal_Int32 eChartType = getChartType( );
    const char* radarStyle = nullptr;
    if( eChartType == chart::TYPEID_RADARAREA )
        radarStyle = "filled";
    else
        radarStyle = "marker";
    pFS->singleElement(FSNS(XML_c, XML_radarStyle), XML_val, radarStyle);

    exportVaryColors(xChartType);
    bool bPrimaryAxes = true;
    exportAllSeries(xChartType, bPrimaryAxes);
    exportAxesId(bPrimaryAxes);

    pFS->endElement( FSNS( XML_c, XML_radarChart ) );
}

void ChartExport::exportScatterChartSeries( const Reference< chart2::XChartType >& xChartType,
        const css::uno::Sequence<css::uno::Reference<chart2::XDataSeries>>* pSeries)
{
    FSHelperPtr pFS = GetFS();
    pFS->startElement(FSNS(XML_c, XML_scatterChart));
    // TODO:scatterStyle

    sal_Int32 nSymbolType = css::chart::ChartSymbolType::NONE;
    Reference< XPropertySet > xPropSet( mxDiagram , uno::UNO_QUERY);
    if( GetProperty( xPropSet, "SymbolType" ) )
        mAny >>= nSymbolType;

    const char* scatterStyle = "lineMarker";
    if (nSymbolType == css::chart::ChartSymbolType::NONE)
    {
        scatterStyle = "line";
    }

    pFS->singleElement(FSNS(XML_c, XML_scatterStyle), XML_val, scatterStyle);

    exportVaryColors(xChartType);
    // FIXME: should export xVal and yVal
    bool bPrimaryAxes = true;
    if (pSeries)
        exportSeries(xChartType, *pSeries, bPrimaryAxes);
    exportAxesId(bPrimaryAxes);

    pFS->endElement( FSNS( XML_c, XML_scatterChart ) );
}

void ChartExport::exportScatterChart( const Reference< chart2::XChartType >& xChartType )
{
    const std::vector<Sequence<Reference<chart2::XDataSeries> > > aSplitDataSeries = splitDataSeriesByAxis(xChartType);
    bool bExported = false;
    for (const auto& splitDataSeries : aSplitDataSeries)
    {
        if (!splitDataSeries.hasElements())
            continue;

        bExported = true;
        exportScatterChartSeries(xChartType, &splitDataSeries);
    }
    if (!bExported)
        exportScatterChartSeries(xChartType, nullptr);
}

void ChartExport::exportStockChart( const Reference< chart2::XChartType >& xChartType )
{
    FSHelperPtr pFS = GetFS();
    const std::vector<Sequence<Reference<chart2::XDataSeries> > > aSplitDataSeries = splitDataSeriesByAxis(xChartType);
    for (const auto& splitDataSeries : aSplitDataSeries)
    {
        if (!splitDataSeries.hasElements())
            continue;

        pFS->startElement(FSNS(XML_c, XML_stockChart));

        bool bPrimaryAxes = true;
        exportCandleStickSeries(splitDataSeries, bPrimaryAxes);

        // export stock properties
        Reference< css::chart::XStatisticDisplay > xStockPropProvider(mxDiagram, uno::UNO_QUERY);
        if (xStockPropProvider.is())
        {
            exportHiLowLines();
            exportUpDownBars(xChartType);
        }

        exportAxesId(bPrimaryAxes);

        pFS->endElement(FSNS(XML_c, XML_stockChart));
    }
}

void ChartExport::exportHiLowLines()
{
    FSHelperPtr pFS = GetFS();
    // export the chart property
    Reference< css::chart::XStatisticDisplay > xChartPropProvider( mxDiagram, uno::UNO_QUERY );

    if (!xChartPropProvider.is())
        return;

    Reference< beans::XPropertySet > xStockPropSet = xChartPropProvider->getMinMaxLine();
    if( !xStockPropSet.is() )
        return;

    pFS->startElement(FSNS(XML_c, XML_hiLowLines));
    exportShapeProps( xStockPropSet );
    pFS->endElement( FSNS( XML_c, XML_hiLowLines ) );
}

void ChartExport::exportUpDownBars( const Reference< chart2::XChartType >& xChartType)
{
    if(xChartType->getChartType() != "com.sun.star.chart2.CandleStickChartType")
        return;

    FSHelperPtr pFS = GetFS();
    // export the chart property
    Reference< css::chart::XStatisticDisplay > xChartPropProvider( mxDiagram, uno::UNO_QUERY );
    if(!xChartPropProvider.is())
        return;

    //  updownbar
    pFS->startElement(FSNS(XML_c, XML_upDownBars));
    // TODO: gapWidth
    pFS->singleElement(FSNS(XML_c, XML_gapWidth), XML_val, OString::number(150));

    Reference< beans::XPropertySet > xChartPropSet = xChartPropProvider->getUpBar();
    if( xChartPropSet.is() )
    {
        pFS->startElement(FSNS(XML_c, XML_upBars));
        // For Linechart with UpDownBars, spPr is not getting imported
        // so no need to call the exportShapeProps() for LineChart
        if(xChartType->getChartType() == "com.sun.star.chart2.CandleStickChartType")
        {
            exportShapeProps(xChartPropSet);
        }
        pFS->endElement( FSNS( XML_c, XML_upBars ) );
    }
    xChartPropSet = xChartPropProvider->getDownBar();
    if( xChartPropSet.is() )
    {
        pFS->startElement(FSNS(XML_c, XML_downBars));
        if(xChartType->getChartType() == "com.sun.star.chart2.CandleStickChartType")
        {
            exportShapeProps(xChartPropSet);
        }
        pFS->endElement( FSNS( XML_c, XML_downBars ) );
    }
    pFS->endElement( FSNS( XML_c, XML_upDownBars ) );
}

void ChartExport::exportSurfaceChart( const Reference< chart2::XChartType >& xChartType )
{
    FSHelperPtr pFS = GetFS();
    sal_Int32 nTypeId = XML_surfaceChart;
    if( mbIs3DChart )
        nTypeId = XML_surface3DChart;
    pFS->startElement(FSNS(XML_c, nTypeId));
    exportVaryColors(xChartType);
    bool bPrimaryAxes = true;
    exportAllSeries(xChartType, bPrimaryAxes);
    exportAxesId(bPrimaryAxes);

    pFS->endElement( FSNS( XML_c, nTypeId ) );
}

void ChartExport::exportAllSeries(const Reference<chart2::XChartType>& xChartType, bool& rPrimaryAxes)
{
    Reference< chart2::XDataSeriesContainer > xDSCnt( xChartType, uno::UNO_QUERY );
    if( ! xDSCnt.is())
        return;

    // export dataseries for current chart-type
    Sequence< Reference< chart2::XDataSeries > > aSeriesSeq( xDSCnt->getDataSeries());
    exportSeries(xChartType, aSeriesSeq, rPrimaryAxes);
}

void ChartExport::exportVaryColors(const Reference<chart2::XChartType>& xChartType)
{
    FSHelperPtr pFS = GetFS();
    try
    {
        Reference<chart2::XDataSeries> xDataSeries = getPrimaryDataSeries(xChartType);
        Reference<beans::XPropertySet> xDataSeriesProps(xDataSeries, uno::UNO_QUERY_THROW);
        Any aAnyVaryColors = xDataSeriesProps->getPropertyValue("VaryColorsByPoint");
        bool bVaryColors = false;
        aAnyVaryColors >>= bVaryColors;
        pFS->singleElement(FSNS(XML_c, XML_varyColors), XML_val, ToPsz10(bVaryColors));
    }
    catch (...)
    {
        pFS->singleElement(FSNS(XML_c, XML_varyColors), XML_val, "0");
    }
}

void ChartExport::exportSeries( const Reference<chart2::XChartType>& xChartType,
        const Sequence<Reference<chart2::XDataSeries> >& rSeriesSeq, bool& rPrimaryAxes )
{
    OUString aLabelRole = xChartType->getRoleOfSequenceForSeriesLabel();
    OUString aChartType( xChartType->getChartType());
    sal_Int32 eChartType = lcl_getChartType( aChartType );

    for( const auto& rSeries : rSeriesSeq )
    {
        // export series
        Reference< chart2::data::XDataSource > xSource( rSeries, uno::UNO_QUERY );
        if( xSource.is())
        {
            Reference< chart2::XDataSeries > xDataSeries( xSource, uno::UNO_QUERY );
            Sequence< Reference< chart2::data::XLabeledDataSequence > > aSeqCnt(
                xSource->getDataSequences());
            // search for main sequence and create a series element
            {
                sal_Int32 nMainSequenceIndex = -1;
                sal_Int32 nSeriesLength = 0;
                Reference< chart2::data::XDataSequence > xValuesSeq;
                Reference< chart2::data::XDataSequence > xLabelSeq;
                sal_Int32 nSeqIdx=0;
                for( ; nSeqIdx<aSeqCnt.getLength(); ++nSeqIdx )
                {
                    Reference< chart2::data::XDataSequence > xTempValueSeq( aSeqCnt[nSeqIdx]->getValues() );
                    if( nMainSequenceIndex==-1 )
                    {
                        Reference< beans::XPropertySet > xSeqProp( xTempValueSeq, uno::UNO_QUERY );
                        OUString aRole;
                        if( xSeqProp.is())
                            xSeqProp->getPropertyValue("Role") >>= aRole;
                        // "main" sequence
                        if( aRole == aLabelRole )
                        {
                            xValuesSeq.set( xTempValueSeq );
                            xLabelSeq.set( aSeqCnt[nSeqIdx]->getLabel());
                            nMainSequenceIndex = nSeqIdx;
                        }
                    }
                    sal_Int32 nSequenceLength = (xTempValueSeq.is()? xTempValueSeq->getData().getLength() : sal_Int32(0));
                    if( nSeriesLength < nSequenceLength )
                        nSeriesLength = nSequenceLength;
                }

                // have found the main sequence, then xValuesSeq and
                // xLabelSeq contain those.  Otherwise both are empty
                {
                    FSHelperPtr pFS = GetFS();

                    pFS->startElement(FSNS(XML_c, XML_ser));

                    // TODO: idx and order
                    pFS->singleElement( FSNS( XML_c, XML_idx ),
                        XML_val, OString::number(mnSeriesCount) );
                    pFS->singleElement( FSNS( XML_c, XML_order ),
                        XML_val, OString::number(mnSeriesCount++) );

                    // export label
                    if( xLabelSeq.is() )
                        exportSeriesText( xLabelSeq );

                    Reference<XPropertySet> xPropSet(xDataSeries, UNO_QUERY_THROW);
                    if( GetProperty( xPropSet, "AttachedAxisIndex") )
                    {
                        sal_Int32 nLocalAttachedAxis = 0;
                        mAny >>= nLocalAttachedAxis;
                        rPrimaryAxes = isPrimaryAxes(nLocalAttachedAxis);
                    }

                    // export shape properties
                    Reference< XPropertySet > xOldPropSet = SchXMLSeriesHelper::createOldAPISeriesPropertySet(
                        rSeries, getModel() );
                    if( xOldPropSet.is() )
                    {
                        exportShapeProps( xOldPropSet );
                    }

                    switch( eChartType )
                    {
                        case chart::TYPEID_BUBBLE:
                        case chart::TYPEID_HORBAR:
                        case chart::TYPEID_BAR:
                        {
                            pFS->singleElement(FSNS(XML_c, XML_invertIfNegative), XML_val, "0");
                        }
                        break;
                        case chart::TYPEID_LINE:
                        {
                            exportMarker(xOldPropSet);
                            break;
                        }
                        case chart::TYPEID_PIE:
                        case chart::TYPEID_DOUGHNUT:
                        {
                            if( xOldPropSet.is() && GetProperty( xOldPropSet, "SegmentOffset") )
                            {
                                sal_Int32 nOffset = 0;
                                mAny >>= nOffset;
                                pFS->singleElement( FSNS( XML_c, XML_explosion ),
                                    XML_val, OString::number( nOffset ) );
                            }
                            break;
                        }
                        case chart::TYPEID_SCATTER:
                        {
                            exportMarker(xOldPropSet);
                            break;
                        }
                        case chart::TYPEID_RADARLINE:
                        {
                            exportMarker(xOldPropSet);
                            break;
                        }
                    }

                    // export data points
                    exportDataPoints( uno::Reference< beans::XPropertySet >( rSeries, uno::UNO_QUERY ), nSeriesLength, eChartType );

                    DataLabelsRange aDLblsRange;
                    // export data labels
                    exportDataLabels(rSeries, nSeriesLength, eChartType, aDLblsRange);

                    exportTrendlines( rSeries );

                    if( eChartType != chart::TYPEID_PIE &&
                            eChartType != chart::TYPEID_RADARLINE )
                    {
                        //export error bars here
                        Reference< XPropertySet > xSeriesPropSet( xSource, uno::UNO_QUERY );
                        Reference< XPropertySet > xErrorBarYProps;
                        xSeriesPropSet->getPropertyValue("ErrorBarY") >>= xErrorBarYProps;
                        if(xErrorBarYProps.is())
                            exportErrorBar(xErrorBarYProps, true);
                        if (eChartType != chart::TYPEID_BAR &&
                                eChartType != chart::TYPEID_HORBAR)
                        {
                            Reference< XPropertySet > xErrorBarXProps;
                            xSeriesPropSet->getPropertyValue("ErrorBarX") >>= xErrorBarXProps;
                            if(xErrorBarXProps.is())
                                exportErrorBar(xErrorBarXProps, false);
                        }
                    }

                    // export categories
                    if( eChartType != chart::TYPEID_SCATTER && eChartType != chart::TYPEID_BUBBLE && mxCategoriesValues.is() )
                        exportSeriesCategory( mxCategoriesValues );

                    if( (eChartType == chart::TYPEID_SCATTER)
                        || (eChartType == chart::TYPEID_BUBBLE) )
                    {
                        // export xVal
                        Reference< chart2::data::XLabeledDataSequence > xSequence( lcl_getDataSequenceByRole( aSeqCnt, "values-x" ) );
                        if( xSequence.is() )
                        {
                            Reference< chart2::data::XDataSequence > xValues( xSequence->getValues() );
                            if( xValues.is() )
                                exportSeriesValues( xValues, XML_xVal );
                        }
                        else if( mxCategoriesValues.is() )
                            exportSeriesCategory( mxCategoriesValues, XML_xVal );
                    }

                    if( eChartType == chart::TYPEID_BUBBLE )
                    {
                        // export yVal
                        Reference< chart2::data::XLabeledDataSequence > xSequence( lcl_getDataSequenceByRole( aSeqCnt, "values-y" ) );
                        if( xSequence.is() )
                        {
                            Reference< chart2::data::XDataSequence > xValues( xSequence->getValues() );
                            if( xValues.is() )
                                exportSeriesValues( xValues, XML_yVal );
                        }
                    }

                    // export values
                    if( xValuesSeq.is() )
                    {
                        sal_Int32 nYValueType = XML_val;
                        if( eChartType == chart::TYPEID_SCATTER )
                            nYValueType = XML_yVal;
                        else if( eChartType == chart::TYPEID_BUBBLE )
                            nYValueType = XML_bubbleSize;
                        exportSeriesValues( xValuesSeq, nYValueType );
                    }

                    if( eChartType == chart::TYPEID_SCATTER
                            || eChartType == chart::TYPEID_LINE )
                        exportSmooth();

                    // tdf103988: "corrupted" files with Bubble chart opening in MSO
                    if( eChartType == chart::TYPEID_BUBBLE )
                        pFS->singleElement(FSNS(XML_c, XML_bubble3D), XML_val, "0");

                    if (!aDLblsRange.empty())
                        writeDataLabelsRange(pFS, GetFB(), aDLblsRange);

                    pFS->endElement( FSNS( XML_c, XML_ser ) );
                }
            }
        }
    }
}

void ChartExport::exportCandleStickSeries(
    const Sequence< Reference< chart2::XDataSeries > > & aSeriesSeq,
    bool& rPrimaryAxes)
{
    for( const Reference< chart2::XDataSeries >& xSeries : aSeriesSeq )
    {
        rPrimaryAxes = lcl_isSeriesAttachedToFirstAxis(xSeries);

        Reference< chart2::data::XDataSource > xSource( xSeries, uno::UNO_QUERY );
        if( xSource.is())
        {
            // export series in correct order (as we don't store roles)
            // with japanese candlesticks: open, low, high, close
            // otherwise: low, high, close
            Sequence< Reference< chart2::data::XLabeledDataSequence > > aSeqCnt(
                xSource->getDataSequences());

            const char* sSeries[] = {"values-first","values-max","values-min","values-last",nullptr};

            for( sal_Int32 idx = 0; sSeries[idx] != nullptr ; idx++ )
            {
                Reference< chart2::data::XLabeledDataSequence > xLabeledSeq( lcl_getDataSequenceByRole( aSeqCnt, OUString::createFromAscii(sSeries[idx]) ) );
                if( xLabeledSeq.is())
                {
                    Reference< chart2::data::XDataSequence > xLabelSeq( xLabeledSeq->getLabel());
                    Reference< chart2::data::XDataSequence > xValueSeq( xLabeledSeq->getValues());
                    {
                        FSHelperPtr pFS = GetFS();
                        pFS->startElement(FSNS(XML_c, XML_ser));

                        // TODO: idx and order
                        // idx attribute should start from 1 and not from 0.
                        pFS->singleElement( FSNS( XML_c, XML_idx ),
                                XML_val, OString::number(idx+1) );
                        pFS->singleElement( FSNS( XML_c, XML_order ),
                                XML_val, OString::number(idx+1) );

                        // export label
                        if( xLabelSeq.is() )
                            exportSeriesText( xLabelSeq );

                        // TODO:export shape properties

                        // export categories
                        if( mxCategoriesValues.is() )
                            exportSeriesCategory( mxCategoriesValues );

                        // export values
                        if( xValueSeq.is() )
                            exportSeriesValues( xValueSeq );

                        pFS->endElement( FSNS( XML_c, XML_ser ) );
                    }
                }
            }
        }
    }
}

void ChartExport::exportSeriesText( const Reference< chart2::data::XDataSequence > & xValueSeq )
{
    FSHelperPtr pFS = GetFS();
    pFS->startElement(FSNS(XML_c, XML_tx));

    OUString aCellRange =  xValueSeq->getSourceRangeRepresentation();
    aCellRange = parseFormula( aCellRange );
    pFS->startElement(FSNS(XML_c, XML_strRef));

    pFS->startElement(FSNS(XML_c, XML_f));
    pFS->writeEscaped( aCellRange );
    pFS->endElement( FSNS( XML_c, XML_f ) );

    OUString aLabelString = lcl_flattenStringSequence(lcl_getLabelSequence(xValueSeq));
    pFS->startElement(FSNS(XML_c, XML_strCache));
    pFS->singleElement(FSNS(XML_c, XML_ptCount), XML_val, "1");
    pFS->startElement(FSNS(XML_c, XML_pt), XML_idx, "0");
    pFS->startElement(FSNS(XML_c, XML_v));
    pFS->writeEscaped( aLabelString );
    pFS->endElement( FSNS( XML_c, XML_v ) );
    pFS->endElement( FSNS( XML_c, XML_pt ) );
    pFS->endElement( FSNS( XML_c, XML_strCache ) );
    pFS->endElement( FSNS( XML_c, XML_strRef ) );
    pFS->endElement( FSNS( XML_c, XML_tx ) );
}

void ChartExport::exportSeriesCategory( const Reference< chart2::data::XDataSequence > & xValueSeq, sal_Int32 nValueType )
{
    FSHelperPtr pFS = GetFS();
    pFS->startElement(FSNS(XML_c, nValueType));

    OUString aCellRange = xValueSeq.is() ? xValueSeq->getSourceRangeRepresentation() : OUString();
    const Sequence< Sequence< OUString >> aFinalSplitSource = (nValueType == XML_cat) ? getSplitCategoriesList(aCellRange) : Sequence< Sequence< OUString>>(0);
    aCellRange = parseFormula( aCellRange );

    if(aFinalSplitSource.getLength() > 1)
    {
        // export multi level category axis labels
        pFS->startElement(FSNS(XML_c, XML_multiLvlStrRef));

        pFS->startElement(FSNS(XML_c, XML_f));
        pFS->writeEscaped(aCellRange);
        pFS->endElement(FSNS(XML_c, XML_f));

        pFS->startElement(FSNS(XML_c, XML_multiLvlStrCache));
        pFS->singleElement(FSNS(XML_c, XML_ptCount), XML_val, OString::number(aFinalSplitSource[0].getLength()));
        for(const auto& rSeq : aFinalSplitSource)
        {
            pFS->startElement(FSNS(XML_c, XML_lvl));
            for(sal_Int32 j = 0; j < rSeq.getLength(); j++)
            {
                if(!rSeq[j].isEmpty())
                {
                    pFS->startElement(FSNS(XML_c, XML_pt), XML_idx, OString::number(j));
                    pFS->startElement(FSNS(XML_c, XML_v));
                    pFS->writeEscaped(rSeq[j]);
                    pFS->endElement(FSNS(XML_c, XML_v));
                    pFS->endElement(FSNS(XML_c, XML_pt));
                }
            }
            pFS->endElement(FSNS(XML_c, XML_lvl));
        }

        pFS->endElement(FSNS(XML_c, XML_multiLvlStrCache));
        pFS->endElement(FSNS(XML_c, XML_multiLvlStrRef));
    }
    else
    {
        // export single category axis labels
        bool bWriteDateCategories = mbHasDateCategories && (nValueType == XML_cat);
        OUString aNumberFormatString;
        if (bWriteDateCategories)
        {
            Reference< css::chart::XAxisXSupplier > xAxisXSupp( mxDiagram, uno::UNO_QUERY );
            if( xAxisXSupp.is())
            {
                Reference< XPropertySet > xAxisProp = xAxisXSupp->getXAxis();
                if (GetProperty(xAxisProp, "NumberFormat"))
                {
                    sal_Int32 nKey = 0;
                    mAny >>= nKey;
                    aNumberFormatString = getNumberFormatCode(nKey);
                }
            }
            if (aNumberFormatString.isEmpty())
                bWriteDateCategories = false;
        }

        pFS->startElement(FSNS(XML_c, bWriteDateCategories ? XML_numRef : XML_strRef));

        pFS->startElement(FSNS(XML_c, XML_f));
        pFS->writeEscaped(aCellRange);
        pFS->endElement(FSNS(XML_c, XML_f));

        ::std::vector< OUString > aCategories;
        lcl_fillCategoriesIntoStringVector(xValueSeq, aCategories);
        sal_Int32 ptCount = aCategories.size();
        pFS->startElement(FSNS(XML_c, bWriteDateCategories ? XML_numCache : XML_strCache));
        if (bWriteDateCategories)
        {
            pFS->startElement(FSNS(XML_c, XML_formatCode));
            pFS->writeEscaped(aNumberFormatString);
            pFS->endElement(FSNS(XML_c, XML_formatCode));
        }

        pFS->singleElement(FSNS(XML_c, XML_ptCount), XML_val, OString::number(ptCount));
        for (sal_Int32 i = 0; i < ptCount; i++)
        {
            pFS->startElement(FSNS(XML_c, XML_pt), XML_idx, OString::number(i));
            pFS->startElement(FSNS(XML_c, XML_v));
            pFS->writeEscaped(aCategories[i]);
            pFS->endElement(FSNS(XML_c, XML_v));
            pFS->endElement(FSNS(XML_c, XML_pt));
        }

        pFS->endElement(FSNS(XML_c, bWriteDateCategories ? XML_numCache : XML_strCache));
        pFS->endElement(FSNS(XML_c, bWriteDateCategories ? XML_numRef : XML_strRef));
    }

    pFS->endElement( FSNS( XML_c, nValueType ) );
}

void ChartExport::exportSeriesValues( const Reference< chart2::data::XDataSequence > & xValueSeq, sal_Int32 nValueType )
{
    FSHelperPtr pFS = GetFS();
    pFS->startElement(FSNS(XML_c, nValueType));

    OUString aCellRange = xValueSeq.is() ? xValueSeq->getSourceRangeRepresentation() : OUString();
    aCellRange = parseFormula( aCellRange );
    // TODO: need to handle XML_multiLvlStrRef according to aCellRange
    pFS->startElement(FSNS(XML_c, XML_numRef));

    pFS->startElement(FSNS(XML_c, XML_f));
    pFS->writeEscaped( aCellRange );
    pFS->endElement( FSNS( XML_c, XML_f ) );

    ::std::vector< double > aValues = lcl_getAllValuesFromSequence( xValueSeq );
    sal_Int32 ptCount = aValues.size();
    pFS->startElement(FSNS(XML_c, XML_numCache));
    pFS->startElement(FSNS(XML_c, XML_formatCode));
    OUString sNumberFormatString("General");
    const sal_Int32 nKey = xValueSeq.is() ? xValueSeq->getNumberFormatKeyByIndex(-1) : 0;
    if (nKey > 0)
        sNumberFormatString = getNumberFormatCode(nKey);
    pFS->writeEscaped(sNumberFormatString);
    pFS->endElement( FSNS( XML_c, XML_formatCode ) );
    pFS->singleElement(FSNS(XML_c, XML_ptCount), XML_val, OString::number(ptCount));

    for( sal_Int32 i = 0; i < ptCount; i++ )
    {
        if (!std::isnan(aValues[i]))
        {
            pFS->startElement(FSNS(XML_c, XML_pt), XML_idx, OString::number(i));
            pFS->startElement(FSNS(XML_c, XML_v));
            pFS->write(aValues[i]);
            pFS->endElement(FSNS(XML_c, XML_v));
            pFS->endElement(FSNS(XML_c, XML_pt));
        }
    }

    pFS->endElement( FSNS( XML_c, XML_numCache ) );
    pFS->endElement( FSNS( XML_c, XML_numRef ) );
    pFS->endElement( FSNS( XML_c, nValueType ) );
}

void ChartExport::exportShapeProps( const Reference< XPropertySet >& xPropSet )
{
    FSHelperPtr pFS = GetFS();
    pFS->startElement(FSNS(XML_c, XML_spPr));

    exportFill( xPropSet );
    WriteOutline( xPropSet, getModel() );

    pFS->endElement( FSNS( XML_c, XML_spPr ) );
}

void ChartExport::exportTextProps(const Reference<XPropertySet>& xPropSet)
{
    FSHelperPtr pFS = GetFS();
    pFS->startElement(FSNS(XML_c, XML_txPr));

    sal_Int32 nRotation = 0;
    const char* textWordWrap = nullptr;

    if (auto xServiceInfo = uno::Reference<lang::XServiceInfo>(xPropSet, uno::UNO_QUERY))
    {
        double fMultiplier = 0.0;
        // We have at least two possible units of returned value: degrees (e.g., for data labels),
        // and 100ths of degree (e.g., for axes labels). The latter is returned as an Any wrapping
        // a sal_Int32 value (see WrappedTextRotationProperty::convertInnerToOuterValue), while
        // the former is double. So we could test the contained type to decide which multiplier to
        // use. But testing the service info should be more robust.
        if (xServiceInfo->supportsService("com.sun.star.chart.ChartAxis"))
            fMultiplier = -600.0;
        else if (xServiceInfo->supportsService("com.sun.star.chart2.DataSeries") || xServiceInfo->supportsService("com.sun.star.chart2.DataPointProperties"))
        {
            fMultiplier = -60000.0;
            bool bTextWordWrap = false;
            if ((xPropSet->getPropertyValue("TextWordWrap") >>= bTextWordWrap) && bTextWordWrap)
                textWordWrap = "square";
            else
                textWordWrap = "none";
        }

        if (fMultiplier)
        {
            double fTextRotation = 0.0;
            uno::Any aAny = xPropSet->getPropertyValue("TextRotation");
            if (aAny.hasValue() && (aAny >>= fTextRotation))
            {
                fTextRotation *= fMultiplier;
                // The MS Office UI allows values only in range of [-90,90].
                if (fTextRotation < -5400000.0 && fTextRotation > -16200000.0)
                {
                    // Reflect the angle if the value is between 90° and 270°
                    fTextRotation += 10800000.0;
                }
                else if (fTextRotation <= -16200000.0)
                {
                    fTextRotation += 21600000.0;
                }
                nRotation = std::round(fTextRotation);
            }
        }
    }

    if (nRotation)
        pFS->singleElement(FSNS(XML_a, XML_bodyPr), XML_rot, OString::number(nRotation), XML_wrap, textWordWrap);
    else
        pFS->singleElement(FSNS(XML_a, XML_bodyPr), XML_wrap, textWordWrap);

    pFS->singleElement(FSNS(XML_a, XML_lstStyle));

    pFS->startElement(FSNS(XML_a, XML_p));
    pFS->startElement(FSNS(XML_a, XML_pPr));

    WriteRunProperties(xPropSet, false, XML_defRPr, true, o3tl::temporary(false),
                       o3tl::temporary(sal_Int32()));

    pFS->endElement(FSNS(XML_a, XML_pPr));
    pFS->endElement(FSNS(XML_a, XML_p));
    pFS->endElement(FSNS(XML_c, XML_txPr));
}

void ChartExport::InitPlotArea( )
{
    Reference< XPropertySet > xDiagramProperties (mxDiagram, uno::UNO_QUERY);

    //    Check for supported services and then the properties provided by this service.
    Reference<lang::XServiceInfo> xServiceInfo (mxDiagram, uno::UNO_QUERY);
    if (xServiceInfo.is())
    {
        if (xServiceInfo->supportsService("com.sun.star.chart.ChartAxisZSupplier"))
        {
            xDiagramProperties->getPropertyValue("HasZAxis") >>= mbHasZAxis;
        }
    }

    xDiagramProperties->getPropertyValue("Dim3D") >>=  mbIs3DChart;

    if( mbHasCategoryLabels && mxNewDiagram.is())
    {
        Reference< chart2::data::XLabeledDataSequence > xCategories( lcl_getCategories( mxNewDiagram, mbHasDateCategories ) );
        if( xCategories.is() )
        {
            mxCategoriesValues.set( xCategories->getValues() );
        }
    }
}

void ChartExport::exportAxes( )
{
    sal_Int32 nSize = maAxes.size();
    // let's export the axis types in the right order
    for ( sal_Int32 nSortIdx = AXIS_PRIMARY_X; nSortIdx <= AXIS_SECONDARY_Y; nSortIdx++ )
    {
        for ( sal_Int32 nIdx = 0; nIdx < nSize; nIdx++ )
        {
            if (nSortIdx == maAxes[nIdx].nAxisType)
                exportAxis( maAxes[nIdx] );
        }
    }
}

namespace {

sal_Int32 getXAxisTypeByChartType(sal_Int32 eChartType)
{
    if( (eChartType == chart::TYPEID_SCATTER)
            || (eChartType == chart::TYPEID_BUBBLE) )
        return  XML_valAx;
    else if( eChartType == chart::TYPEID_STOCK )
        return  XML_dateAx;

    return XML_catAx;
}

sal_Int32 getRealXAxisType(sal_Int32 nAxisType)
{
    if( nAxisType == chart2::AxisType::CATEGORY )
        return XML_catAx;
    else if( nAxisType == chart2::AxisType::DATE )
        return XML_dateAx;
    else if( nAxisType == chart2::AxisType::SERIES )
        return XML_serAx;

    return XML_valAx;
}

}

void ChartExport::exportAxis(const AxisIdPair& rAxisIdPair)
{
    // get some properties from document first
    bool bHasXAxisTitle = false,
         bHasYAxisTitle = false,
         bHasZAxisTitle = false,
         bHasSecondaryXAxisTitle = false,
         bHasSecondaryYAxisTitle = false;
    bool bHasXAxisMajorGrid = false,
         bHasXAxisMinorGrid = false,
         bHasYAxisMajorGrid = false,
         bHasYAxisMinorGrid = false,
         bHasZAxisMajorGrid = false,
         bHasZAxisMinorGrid = false;

    Reference< XPropertySet > xDiagramProperties (mxDiagram, uno::UNO_QUERY);

    xDiagramProperties->getPropertyValue("HasXAxisTitle") >>= bHasXAxisTitle;
    xDiagramProperties->getPropertyValue("HasYAxisTitle") >>= bHasYAxisTitle;
    xDiagramProperties->getPropertyValue("HasZAxisTitle") >>= bHasZAxisTitle;
    xDiagramProperties->getPropertyValue("HasSecondaryXAxisTitle") >>=  bHasSecondaryXAxisTitle;
    xDiagramProperties->getPropertyValue("HasSecondaryYAxisTitle") >>=  bHasSecondaryYAxisTitle;

    xDiagramProperties->getPropertyValue("HasXAxisGrid") >>=  bHasXAxisMajorGrid;
    xDiagramProperties->getPropertyValue("HasYAxisGrid") >>=  bHasYAxisMajorGrid;
    xDiagramProperties->getPropertyValue("HasZAxisGrid") >>=  bHasZAxisMajorGrid;

    xDiagramProperties->getPropertyValue("HasXAxisHelpGrid") >>=  bHasXAxisMinorGrid;
    xDiagramProperties->getPropertyValue("HasYAxisHelpGrid") >>=  bHasYAxisMinorGrid;
    xDiagramProperties->getPropertyValue("HasZAxisHelpGrid") >>=  bHasZAxisMinorGrid;

    Reference< XPropertySet > xAxisProp;
    Reference< drawing::XShape > xAxisTitle;
    Reference< beans::XPropertySet > xMajorGrid;
    Reference< beans::XPropertySet > xMinorGrid;
    sal_Int32 nAxisType = XML_catAx;
    const char* sAxPos = nullptr;

    switch( rAxisIdPair.nAxisType )
    {
        case AXIS_PRIMARY_X:
        {
            Reference< css::chart::XAxisXSupplier > xAxisXSupp( mxDiagram, uno::UNO_QUERY );
            if( xAxisXSupp.is())
                xAxisProp = xAxisXSupp->getXAxis();
            if( bHasXAxisTitle )
                xAxisTitle = xAxisXSupp->getXAxisTitle();
            if( bHasXAxisMajorGrid )
                xMajorGrid = xAxisXSupp->getXMainGrid();
            if( bHasXAxisMinorGrid )
                xMinorGrid = xAxisXSupp->getXHelpGrid();

            nAxisType = lcl_getCategoryAxisType(mxNewDiagram, 0, 0);
            if( nAxisType != -1 )
                nAxisType = getRealXAxisType(nAxisType);
            else
                nAxisType = getXAxisTypeByChartType( getChartType() );
            // FIXME: axPos, need to check axis direction
            sAxPos = "b";
            break;
        }
        case AXIS_PRIMARY_Y:
        {
            Reference< css::chart::XAxisYSupplier > xAxisYSupp( mxDiagram, uno::UNO_QUERY );
            if( xAxisYSupp.is())
                xAxisProp = xAxisYSupp->getYAxis();
            if( bHasYAxisTitle )
                xAxisTitle = xAxisYSupp->getYAxisTitle();
            if( bHasYAxisMajorGrid )
                xMajorGrid = xAxisYSupp->getYMainGrid();
            if( bHasYAxisMinorGrid )
                xMinorGrid = xAxisYSupp->getYHelpGrid();

            nAxisType = XML_valAx;
            // FIXME: axPos, need to check axis direction
            sAxPos = "l";
            break;
        }
        case AXIS_PRIMARY_Z:
        {
            Reference< css::chart::XAxisZSupplier > xAxisZSupp( mxDiagram, uno::UNO_QUERY );
            if( xAxisZSupp.is())
                xAxisProp = xAxisZSupp->getZAxis();
            if( bHasZAxisTitle )
                xAxisTitle = xAxisZSupp->getZAxisTitle();
            if( bHasZAxisMajorGrid )
                xMajorGrid = xAxisZSupp->getZMainGrid();
            if( bHasZAxisMinorGrid )
                xMinorGrid = xAxisZSupp->getZHelpGrid();

            sal_Int32 eChartType = getChartType( );
            if( (eChartType == chart::TYPEID_SCATTER)
                || (eChartType == chart::TYPEID_BUBBLE) )
                nAxisType = XML_valAx;
            else if( eChartType == chart::TYPEID_STOCK )
                nAxisType = XML_dateAx;
            else if( eChartType == chart::TYPEID_BAR || eChartType == chart::TYPEID_AREA )
                nAxisType = XML_serAx;
            // FIXME: axPos, need to check axis direction
            sAxPos = "b";
            break;
        }
        case AXIS_SECONDARY_X:
        {
            Reference< css::chart::XTwoAxisXSupplier > xAxisTwoXSupp( mxDiagram, uno::UNO_QUERY );
            if( xAxisTwoXSupp.is())
                xAxisProp = xAxisTwoXSupp->getSecondaryXAxis();
            if( bHasSecondaryXAxisTitle )
            {
                Reference< css::chart::XSecondAxisTitleSupplier > xAxisSupp( mxDiagram, uno::UNO_QUERY );
                xAxisTitle = xAxisSupp->getSecondXAxisTitle();
            }

            nAxisType = lcl_getCategoryAxisType(mxNewDiagram, 0, 1);
            if( nAxisType != -1 )
                nAxisType = getRealXAxisType(nAxisType);
            else
                nAxisType = getXAxisTypeByChartType( getChartType() );
            // FIXME: axPos, need to check axis direction
            sAxPos = "t";
            break;
        }
        case AXIS_SECONDARY_Y:
        {
            Reference< css::chart::XTwoAxisYSupplier > xAxisTwoYSupp( mxDiagram, uno::UNO_QUERY );
            if( xAxisTwoYSupp.is())
                xAxisProp = xAxisTwoYSupp->getSecondaryYAxis();
            if( bHasSecondaryYAxisTitle )
            {
                Reference< css::chart::XSecondAxisTitleSupplier > xAxisSupp( mxDiagram, uno::UNO_QUERY );
                xAxisTitle = xAxisSupp->getSecondYAxisTitle();
            }

            nAxisType = XML_valAx;
            // FIXME: axPos, need to check axis direction
            sAxPos = "r";
            break;
        }
    }

    _exportAxis(xAxisProp, xAxisTitle, xMajorGrid, xMinorGrid, nAxisType, sAxPos, rAxisIdPair);
}

void ChartExport::_exportAxis(
    const Reference< XPropertySet >& xAxisProp,
    const Reference< drawing::XShape >& xAxisTitle,
    const Reference< XPropertySet >& xMajorGrid,
    const Reference< XPropertySet >& xMinorGrid,
    sal_Int32 nAxisType,
    const char* sAxisPos,
    const AxisIdPair& rAxisIdPair )
{
    FSHelperPtr pFS = GetFS();
    pFS->startElement(FSNS(XML_c, nAxisType));
    pFS->singleElement(FSNS(XML_c, XML_axId), XML_val, OString::number(rAxisIdPair.nAxisId));

    pFS->startElement(FSNS(XML_c, XML_scaling));

    // logBase, min, max
    if(GetProperty( xAxisProp, "Logarithmic" ) )
    {
        bool bLogarithmic = false;
        mAny >>= bLogarithmic;
        if( bLogarithmic )
        {
            // default value is 10?
            pFS->singleElement(FSNS(XML_c, XML_logBase), XML_val, OString::number(10));
        }
    }

    // orientation: minMax, maxMin
    bool bReverseDirection = false;
    if(GetProperty( xAxisProp, "ReverseDirection" ) )
        mAny >>= bReverseDirection;

    const char* orientation = bReverseDirection ? "maxMin":"minMax";
    pFS->singleElement(FSNS(XML_c, XML_orientation), XML_val, orientation);

    bool bAutoMax = false;
    if(GetProperty( xAxisProp, "AutoMax" ) )
        mAny >>= bAutoMax;

    if( !bAutoMax && (GetProperty( xAxisProp, "Max" ) ) )
    {
        double dMax = 0;
        mAny >>= dMax;
        pFS->singleElement(FSNS(XML_c, XML_max), XML_val, OString::number(dMax));
    }

    bool bAutoMin = false;
    if(GetProperty( xAxisProp, "AutoMin" ) )
        mAny >>= bAutoMin;

    if( !bAutoMin && (GetProperty( xAxisProp, "Min" ) ) )
    {
        double dMin = 0;
        mAny >>= dMin;
        pFS->singleElement(FSNS(XML_c, XML_min), XML_val, OString::number(dMin));
    }

    pFS->endElement( FSNS( XML_c, XML_scaling ) );

    bool bVisible = true;
    if( xAxisProp.is() )
    {
        xAxisProp->getPropertyValue("Visible") >>=  bVisible;
    }

    // only export each axis only once non-deleted
    auto aItInsertedPair = maExportedAxis.insert(rAxisIdPair.nAxisType);
    bool bDeleted = !aItInsertedPair.second;

    pFS->singleElement(FSNS(XML_c, XML_delete), XML_val, !bDeleted && bVisible ? "0" : "1");

    // FIXME: axPos, need to check the property "ReverseDirection"
    pFS->singleElement(FSNS(XML_c, XML_axPos), XML_val, sAxisPos);
    // major grid line
    if( xMajorGrid.is())
    {
        pFS->startElement(FSNS(XML_c, XML_majorGridlines));
        exportShapeProps( xMajorGrid );
        pFS->endElement( FSNS( XML_c, XML_majorGridlines ) );
    }

    // minor grid line
    if( xMinorGrid.is())
    {
        pFS->startElement(FSNS(XML_c, XML_minorGridlines));
        exportShapeProps( xMinorGrid );
        pFS->endElement( FSNS( XML_c, XML_minorGridlines ) );
    }

    // title
    if( xAxisTitle.is() )
        exportTitle( xAxisTitle );

    bool bLinkedNumFmt = true;
    if (GetProperty(xAxisProp, "LinkNumberFormatToSource"))
        mAny >>= bLinkedNumFmt;

    OUString aNumberFormatString("General");
    if (GetProperty(xAxisProp, "NumberFormat"))
    {
        sal_Int32 nKey = 0;
        mAny >>= nKey;
        aNumberFormatString = getNumberFormatCode(nKey);
    }

    pFS->singleElement(FSNS(XML_c, XML_numFmt),
            XML_formatCode, aNumberFormatString,
            XML_sourceLinked, bLinkedNumFmt ? "1" : "0");

    // majorTickMark
    sal_Int32 nValue = 0;
    if(GetProperty( xAxisProp, "Marks" ) )
    {
        mAny >>= nValue;
        bool bInner = nValue & css::chart::ChartAxisMarks::INNER;
        bool bOuter = nValue & css::chart::ChartAxisMarks::OUTER;
        const char* majorTickMark = nullptr;
        if( bInner && bOuter )
            majorTickMark = "cross";
        else if( bInner )
            majorTickMark = "in";
        else if( bOuter )
            majorTickMark = "out";
        else
            majorTickMark = "none";
        pFS->singleElement(FSNS(XML_c, XML_majorTickMark), XML_val, majorTickMark);
    }
    // minorTickMark
    if(GetProperty( xAxisProp, "HelpMarks" ) )
    {
        mAny >>= nValue;
        bool bInner = nValue & css::chart::ChartAxisMarks::INNER;
        bool bOuter = nValue & css::chart::ChartAxisMarks::OUTER;
        const char* minorTickMark = nullptr;
        if( bInner && bOuter )
            minorTickMark = "cross";
        else if( bInner )
            minorTickMark = "in";
        else if( bOuter )
            minorTickMark = "out";
        else
            minorTickMark = "none";
        pFS->singleElement(FSNS(XML_c, XML_minorTickMark), XML_val, minorTickMark);
    }
    // tickLblPos
    const char* sTickLblPos = nullptr;
    bool bDisplayLabel = true;
    if(GetProperty( xAxisProp, "DisplayLabels" ) )
        mAny >>= bDisplayLabel;
    if( bDisplayLabel && (GetProperty( xAxisProp, "LabelPosition" ) ) )
    {
        css::chart::ChartAxisLabelPosition eLabelPosition = css::chart::ChartAxisLabelPosition_NEAR_AXIS;
        mAny >>= eLabelPosition;
        switch( eLabelPosition )
        {
            case css::chart::ChartAxisLabelPosition_NEAR_AXIS:
            case css::chart::ChartAxisLabelPosition_NEAR_AXIS_OTHER_SIDE:
                sTickLblPos = "nextTo";
                break;
            case css::chart::ChartAxisLabelPosition_OUTSIDE_START:
                sTickLblPos = "low";
                break;
            case css::chart::ChartAxisLabelPosition_OUTSIDE_END:
                sTickLblPos = "high";
                break;
            default:
                sTickLblPos = "nextTo";
                break;
        }
    }
    else
    {
        sTickLblPos = "none";
    }
    pFS->singleElement(FSNS(XML_c, XML_tickLblPos), XML_val, sTickLblPos);

    // shape properties
    exportShapeProps( xAxisProp );

    exportTextProps(xAxisProp);

    pFS->singleElement(FSNS(XML_c, XML_crossAx), XML_val, OString::number(rAxisIdPair.nCrossAx));

    // crosses & crossesAt
    bool bCrossesValue = false;
    const char* sCrosses = nullptr;
    // do not export the CrossoverPosition/CrossoverValue, if the axis is deleted and not visible
    if( GetProperty( xAxisProp, "CrossoverPosition" ) && !bDeleted && bVisible )
    {
        css::chart::ChartAxisPosition ePosition( css::chart::ChartAxisPosition_ZERO );
        mAny >>= ePosition;
        switch( ePosition )
        {
            case css::chart::ChartAxisPosition_START:
                sCrosses = "min";
                break;
            case css::chart::ChartAxisPosition_END:
                sCrosses = "max";
                break;
            case css::chart::ChartAxisPosition_ZERO:
                sCrosses = "autoZero";
                break;
            default:
                bCrossesValue = true;
                break;
        }
    }

    if( bCrossesValue && GetProperty( xAxisProp, "CrossoverValue" ) )
    {
        double dValue = 0;
        mAny >>= dValue;
        pFS->singleElement(FSNS(XML_c, XML_crossesAt), XML_val, OString::number(dValue));
    }
    else
    {
        if(sCrosses)
        {
            pFS->singleElement(FSNS(XML_c, XML_crosses), XML_val, sCrosses);
        }
    }

    if( ( nAxisType == XML_catAx )
        || ( nAxisType == XML_dateAx ) )
    {
        // FIXME: seems not support? use default value,
        const char* const isAuto = "1";
        pFS->singleElement(FSNS(XML_c, XML_auto), XML_val, isAuto);

        if( nAxisType == XML_catAx )
        {
            // FIXME: seems not support? lblAlgn
            const char* const sLblAlgn = "ctr";
            pFS->singleElement(FSNS(XML_c, XML_lblAlgn), XML_val, sLblAlgn);
        }

        // FIXME: seems not support? lblOffset
        pFS->singleElement(FSNS(XML_c, XML_lblOffset), XML_val, OString::number(100));

        // export baseTimeUnit, majorTimeUnit, minorTimeUnit of Date axis
        if( nAxisType == XML_dateAx )
        {
            sal_Int32 nAxisIndex = -1;
            if( rAxisIdPair.nAxisType == AXIS_PRIMARY_X )
                nAxisIndex = 0;
            else if( rAxisIdPair.nAxisType == AXIS_SECONDARY_X )
                nAxisIndex = 1;

            cssc::TimeIncrement aTimeIncrement = lcl_getDateTimeIncrement( mxNewDiagram, nAxisIndex );
            sal_Int32 nTimeResolution = css::chart::TimeUnit::DAY;
            if( aTimeIncrement.TimeResolution >>= nTimeResolution )
                pFS->singleElement(FSNS(XML_c, XML_baseTimeUnit), XML_val, lclGetTimeUnitToken(nTimeResolution));

            cssc::TimeInterval aInterval;
            if( aTimeIncrement.MajorTimeInterval >>= aInterval )
            {
                pFS->singleElement(FSNS(XML_c, XML_majorUnit), XML_val, OString::number(aInterval.Number));
                pFS->singleElement(FSNS(XML_c, XML_majorTimeUnit), XML_val, lclGetTimeUnitToken(aInterval.TimeUnit));
            }
            if( aTimeIncrement.MinorTimeInterval >>= aInterval )
            {
                pFS->singleElement(FSNS(XML_c, XML_minorUnit), XML_val, OString::number(aInterval.Number));
                pFS->singleElement(FSNS(XML_c, XML_minorTimeUnit), XML_val, lclGetTimeUnitToken(aInterval.TimeUnit));
            }
        }

        // FIXME: seems not support? noMultiLvlLbl
        pFS->singleElement(FSNS(XML_c, XML_noMultiLvlLbl), XML_val, OString::number(0));
    }

    // crossBetween
    if( nAxisType == XML_valAx )
    {
        if( lcl_isCategoryAxisShifted( mxNewDiagram ))
            pFS->singleElement(FSNS(XML_c, XML_crossBetween), XML_val, "between");
        else
            pFS->singleElement(FSNS(XML_c, XML_crossBetween), XML_val, "midCat");
    }

    // majorUnit
    bool bAutoStepMain = false;
    if(GetProperty( xAxisProp, "AutoStepMain" ) )
        mAny >>= bAutoStepMain;

    if( !bAutoStepMain && (GetProperty( xAxisProp, "StepMain" ) ) )
    {
        double dMajorUnit = 0;
        mAny >>= dMajorUnit;
        pFS->singleElement(FSNS(XML_c, XML_majorUnit), XML_val, OString::number(dMajorUnit));
    }
    // minorUnit
    bool bAutoStepHelp = false;
    if(GetProperty( xAxisProp, "AutoStepHelp" ) )
        mAny >>= bAutoStepHelp;

    if( !bAutoStepHelp && (GetProperty( xAxisProp, "StepHelp" ) ) )
    {
        double dMinorUnit = 0;
        mAny >>= dMinorUnit;
        if( GetProperty( xAxisProp, "StepHelpCount" ) )
        {
            sal_Int32 dMinorUnitCount = 0;
            mAny >>= dMinorUnitCount;
            // tdf#114168 Don't save minor unit if number of step help count is 5 (which is default for MS Excel),
            // to allow proper .xlsx import. If minorUnit is set and majorUnit not, then it is impossible
            // to calculate StepHelpCount.
            if( dMinorUnitCount != 5 )
            {
                pFS->singleElement( FSNS( XML_c, XML_minorUnit ),
                    XML_val, OString::number( dMinorUnit ) );
            }
        }
    }

    if( nAxisType == XML_valAx && GetProperty( xAxisProp, "DisplayUnits" ) )
    {
        bool bDisplayUnits = false;
        mAny >>= bDisplayUnits;
        if(bDisplayUnits)
        {
            if(GetProperty( xAxisProp, "BuiltInUnit" ))
            {
                OUString aVal;
                mAny >>= aVal;
                if(!aVal.isEmpty())
                {
                    pFS->startElement(FSNS(XML_c, XML_dispUnits));

                    pFS->singleElement(FSNS(XML_c, XML_builtInUnit), XML_val, aVal);

                    pFS->singleElement(FSNS( XML_c, XML_dispUnitsLbl ));
                    pFS->endElement( FSNS( XML_c, XML_dispUnits ) );
                }
            }
        }
    }

    pFS->endElement( FSNS( XML_c, nAxisType ) );
}

namespace {

struct LabelPlacementParam
{
    bool mbExport;
    sal_Int32 meDefault;

    std::unordered_set<sal_Int32> maAllowedValues;

    LabelPlacementParam(bool bExport, sal_Int32 nDefault) :
        mbExport(bExport),
        meDefault(nDefault),
        maAllowedValues(
          {
           css::chart::DataLabelPlacement::OUTSIDE,
           css::chart::DataLabelPlacement::INSIDE,
           css::chart::DataLabelPlacement::CENTER,
           css::chart::DataLabelPlacement::NEAR_ORIGIN,
           css::chart::DataLabelPlacement::TOP,
           css::chart::DataLabelPlacement::BOTTOM,
           css::chart::DataLabelPlacement::LEFT,
           css::chart::DataLabelPlacement::RIGHT,
           css::chart::DataLabelPlacement::AVOID_OVERLAP
          }
        )
    {}
};

const char* toOOXMLPlacement( sal_Int32 nPlacement )
{
    switch (nPlacement)
    {
        case css::chart::DataLabelPlacement::OUTSIDE:       return "outEnd";
        case css::chart::DataLabelPlacement::INSIDE:        return "inEnd";
        case css::chart::DataLabelPlacement::CENTER:        return "ctr";
        case css::chart::DataLabelPlacement::NEAR_ORIGIN:   return "inBase";
        case css::chart::DataLabelPlacement::TOP:           return "t";
        case css::chart::DataLabelPlacement::BOTTOM:        return "b";
        case css::chart::DataLabelPlacement::LEFT:          return "l";
        case css::chart::DataLabelPlacement::RIGHT:         return "r";
        case css::chart::DataLabelPlacement::CUSTOM:
        case css::chart::DataLabelPlacement::AVOID_OVERLAP: return "bestFit";
        default:
            ;
    }

    return "outEnd";
}

OUString getFieldTypeString( const chart2::DataPointCustomLabelFieldType aType )
{
    switch (aType)
    {
    case chart2::DataPointCustomLabelFieldType_CATEGORYNAME:
        return "CATEGORYNAME";

    case chart2::DataPointCustomLabelFieldType_SERIESNAME:
        return "SERIESNAME";

    case chart2::DataPointCustomLabelFieldType_VALUE:
        return "VALUE";

    case chart2::DataPointCustomLabelFieldType_CELLREF:
        return "CELLREF";

    case chart2::DataPointCustomLabelFieldType_CELLRANGE:
        return "CELLRANGE";

    default:
        break;
    }
    return OUString();
}

void writeRunProperties( ChartExport* pChartExport, Reference<XPropertySet> const & xPropertySet )
{
    bool bDummy = false;
    sal_Int32 nDummy;
    pChartExport->WriteRunProperties(xPropertySet, false, XML_rPr, true, bDummy, nDummy);
}

void writeCustomLabel( const FSHelperPtr& pFS, ChartExport* pChartExport,
                       const Sequence<Reference<chart2::XDataPointCustomLabelField>>& rCustomLabelFields,
                       sal_Int32 nLabelIndex, DataLabelsRange& rDLblsRange )
{
    pFS->startElement(FSNS(XML_c, XML_tx));
    pFS->startElement(FSNS(XML_c, XML_rich));

    // TODO: body properties?
    pFS->singleElement(FSNS(XML_a, XML_bodyPr));

    OUString sFieldType;
    OUString sContent;
    pFS->startElement(FSNS(XML_a, XML_p));

    for (auto& rField : rCustomLabelFields)
    {
        Reference<XPropertySet> xPropertySet(rField, UNO_QUERY);
        chart2::DataPointCustomLabelFieldType aType = rField->getFieldType();
        sFieldType.clear();
        sContent.clear();
        bool bNewParagraph = false;

        if (aType == chart2::DataPointCustomLabelFieldType_CELLRANGE &&
            rField->getDataLabelsRange())
        {
            if (rDLblsRange.getRange().isEmpty())
                rDLblsRange.setRange(rField->getCellRange());

            if (!rDLblsRange.hasLabel(nLabelIndex))
                rDLblsRange.setLabel(nLabelIndex, rField->getString());

            sContent = "[CELLRANGE]";
        }
        else
        {
            sContent = rField->getString();
        }

        if (aType == chart2::DataPointCustomLabelFieldType_NEWLINE)
            bNewParagraph = true;
        else if (aType != chart2::DataPointCustomLabelFieldType_TEXT)
            sFieldType = getFieldTypeString(aType);

        if (bNewParagraph)
        {
            pFS->endElement(FSNS(XML_a, XML_p));
            pFS->startElement(FSNS(XML_a, XML_p));
            continue;
        }

        if (sFieldType.isEmpty())
        {
            // Normal text run
            pFS->startElement(FSNS(XML_a, XML_r));
            writeRunProperties(pChartExport, xPropertySet);

            pFS->startElement(FSNS(XML_a, XML_t));
            pFS->writeEscaped(sContent);
            pFS->endElement(FSNS(XML_a, XML_t));

            pFS->endElement(FSNS(XML_a, XML_r));
        }
        else
        {
            // Field
            pFS->startElement(FSNS(XML_a, XML_fld), XML_id, rField->getGuid(), XML_type,
                              sFieldType);
            writeRunProperties(pChartExport, xPropertySet);

            pFS->startElement(FSNS(XML_a, XML_t));
            pFS->writeEscaped(sContent);
            pFS->endElement(FSNS(XML_a, XML_t));

            pFS->endElement(FSNS(XML_a, XML_fld));
        }
    }

    pFS->endElement(FSNS(XML_a, XML_p));
    pFS->endElement(FSNS(XML_c, XML_rich));
    pFS->endElement(FSNS(XML_c, XML_tx));
}

void writeLabelProperties( const FSHelperPtr& pFS, ChartExport* pChartExport,
    const uno::Reference<beans::XPropertySet>& xPropSet, const LabelPlacementParam& rLabelParam,
    sal_Int32 nLabelIndex, DataLabelsRange& rDLblsRange )
{
    if (!xPropSet.is())
        return;

    chart2::DataPointLabel aLabel;
    Sequence<Reference<chart2::XDataPointCustomLabelField>> aCustomLabelFields;
    sal_Int32 nLabelBorderWidth = 0;
    sal_Int32 nLabelBorderColor = 0x00FFFFFF;
    sal_Int32 nLabelFillColor = -1;

    xPropSet->getPropertyValue("Label") >>= aLabel;
    xPropSet->getPropertyValue("CustomLabelFields") >>= aCustomLabelFields;
    xPropSet->getPropertyValue("LabelBorderWidth") >>= nLabelBorderWidth;
    xPropSet->getPropertyValue("LabelBorderColor") >>= nLabelBorderColor;
    xPropSet->getPropertyValue("LabelFillColor") >>= nLabelFillColor;

    if (nLabelBorderWidth > 0 || nLabelFillColor != -1)
    {
        pFS->startElement(FSNS(XML_c, XML_spPr));

        if (nLabelFillColor != -1)
        {
            pFS->startElement(FSNS(XML_a, XML_solidFill));

            OString aStr = OString::number(nLabelFillColor, 16).toAsciiUpperCase();
            pFS->singleElement(FSNS(XML_a, XML_srgbClr), XML_val, aStr);

            pFS->endElement(FSNS(XML_a, XML_solidFill));
        }

        if (nLabelBorderWidth > 0)
        {
            pFS->startElement(FSNS(XML_a, XML_ln), XML_w,
                              OString::number(convertHmmToEmu(nLabelBorderWidth)));

            if (nLabelBorderColor != -1)
            {
                pFS->startElement(FSNS(XML_a, XML_solidFill));

                OString aStr = OString::number(nLabelBorderColor, 16).toAsciiUpperCase();
                pFS->singleElement(FSNS(XML_a, XML_srgbClr), XML_val, aStr);

                pFS->endElement(FSNS(XML_a, XML_solidFill));
            }

            pFS->endElement(FSNS(XML_a, XML_ln));
        }

        pFS->endElement(FSNS(XML_c, XML_spPr));
    }

    pChartExport->exportTextProps(xPropSet);

    if (aCustomLabelFields.hasElements())
        writeCustomLabel(pFS, pChartExport, aCustomLabelFields, nLabelIndex, rDLblsRange);

    if (rLabelParam.mbExport)
    {
        sal_Int32 nLabelPlacement = rLabelParam.meDefault;
        if (xPropSet->getPropertyValue("LabelPlacement") >>= nLabelPlacement)
        {
            if (!rLabelParam.maAllowedValues.count(nLabelPlacement))
                nLabelPlacement = rLabelParam.meDefault;
            pFS->singleElement(FSNS(XML_c, XML_dLblPos), XML_val, toOOXMLPlacement(nLabelPlacement));
        }
    }

    pFS->singleElement(FSNS(XML_c, XML_showLegendKey), XML_val, ToPsz10(aLabel.ShowLegendSymbol));
    pFS->singleElement(FSNS(XML_c, XML_showVal), XML_val, ToPsz10(aLabel.ShowNumber));
    pFS->singleElement(FSNS(XML_c, XML_showCatName), XML_val, ToPsz10(aLabel.ShowCategoryName));
    pFS->singleElement(FSNS(XML_c, XML_showSerName), XML_val, ToPsz10(aLabel.ShowSeriesName));
    pFS->singleElement(FSNS(XML_c, XML_showPercent), XML_val, ToPsz10(aLabel.ShowNumberInPercent));

    // Export the text "separator" if exists
    uno::Any aAny = xPropSet->getPropertyValue("LabelSeparator");
    if( aAny.hasValue() )
    {
        OUString nLabelSeparator;
        aAny >>= nLabelSeparator;
        pFS->startElement(FSNS(XML_c, XML_separator));
        pFS->writeEscaped( nLabelSeparator );
        pFS->endElement( FSNS( XML_c, XML_separator ) );
    }

    if (rDLblsRange.hasLabel(nLabelIndex))
    {
        pFS->startElement(FSNS(XML_c, XML_extLst));
        pFS->startElement(FSNS(XML_c, XML_ext), XML_uri,
            "{CE6537A1-D6FC-4f65-9D91-7224C49458BB}", FSNS(XML_xmlns, XML_c15),
            pChartExport->GetFB()->getNamespaceURL(OOX_NS(c15)));

        pFS->singleElement(FSNS(XML_c15, XML_showDataLabelsRange), XML_val, "1");

        pFS->endElement(FSNS(XML_c, XML_ext));
        pFS->endElement(FSNS(XML_c, XML_extLst));
    }
}

}

void ChartExport::exportDataLabels(
    const uno::Reference<chart2::XDataSeries> & xSeries, sal_Int32 nSeriesLength, sal_Int32 eChartType,
    DataLabelsRange& rDLblsRange)
{
    if (!xSeries.is() || nSeriesLength <= 0)
        return;

    uno::Reference<beans::XPropertySet> xPropSet(xSeries, uno::UNO_QUERY);
    if (!xPropSet.is())
        return;

    FSHelperPtr pFS = GetFS();
    pFS->startElement(FSNS(XML_c, XML_dLbls));

    bool bLinkedNumFmt = true;
    if (GetProperty(xPropSet, "LinkNumberFormatToSource"))
        mAny >>= bLinkedNumFmt;

    chart2::DataPointLabel aLabel;
    bool bLabelIsNumberFormat = true;
    if( xPropSet->getPropertyValue("Label") >>= aLabel )
        bLabelIsNumberFormat = aLabel.ShowNumber;

    if (GetProperty(xPropSet, bLabelIsNumberFormat ? OUString("NumberFormat") : OUString("PercentageNumberFormat")))
    {
        sal_Int32 nKey = 0;
        mAny >>= nKey;

        OUString aNumberFormatString = getNumberFormatCode(nKey);

        pFS->singleElement(FSNS(XML_c, XML_numFmt),
            XML_formatCode, aNumberFormatString,
            XML_sourceLinked, ToPsz10(bLinkedNumFmt));
    }

    uno::Sequence<sal_Int32> aAttrLabelIndices;
    xPropSet->getPropertyValue("AttributedDataPoints") >>= aAttrLabelIndices;

    // We must not export label placement property when the chart type doesn't
    // support this option in MS Office, else MS Office would think the file
    // is corrupt & refuse to open it.

    const chart::TypeGroupInfo& rInfo = chart::GetTypeGroupInfo(static_cast<chart::TypeId>(eChartType));
    LabelPlacementParam aParam(!mbIs3DChart, rInfo.mnDefLabelPos);
    switch (eChartType) // diagram chart type
    {
        case chart::TYPEID_PIE:
            if(getChartType() == chart::TYPEID_DOUGHNUT)
                aParam.mbExport = false;
            else
            // All pie charts support label placement.
            aParam.mbExport = true;
        break;
        case chart::TYPEID_AREA:
        case chart::TYPEID_RADARLINE:
        case chart::TYPEID_RADARAREA:
            // These chart types don't support label placement.
            aParam.mbExport = false;
        break;
        case chart::TYPEID_BAR:
            if (mbStacked || mbPercent)
            {
                aParam.maAllowedValues.clear();
                aParam.maAllowedValues.insert(css::chart::DataLabelPlacement::CENTER);
                aParam.maAllowedValues.insert(css::chart::DataLabelPlacement::INSIDE);
                aParam.maAllowedValues.insert(css::chart::DataLabelPlacement::NEAR_ORIGIN);
                aParam.meDefault = css::chart::DataLabelPlacement::CENTER;
            }
            else  // Clustered bar chart
            {
                aParam.maAllowedValues.clear();
                aParam.maAllowedValues.insert(css::chart::DataLabelPlacement::CENTER);
                aParam.maAllowedValues.insert(css::chart::DataLabelPlacement::INSIDE);
                aParam.maAllowedValues.insert(css::chart::DataLabelPlacement::OUTSIDE);
                aParam.maAllowedValues.insert(css::chart::DataLabelPlacement::NEAR_ORIGIN);
                aParam.meDefault = css::chart::DataLabelPlacement::OUTSIDE;
            }
        break;
        default:
            ;
    }

    for (const sal_Int32 nIdx : aAttrLabelIndices)
    {
        uno::Reference<beans::XPropertySet> xLabelPropSet = xSeries->getDataPointByIndex(nIdx);

        if (!xLabelPropSet.is())
            continue;

        pFS->startElement(FSNS(XML_c, XML_dLbl));
        pFS->singleElement(FSNS(XML_c, XML_idx), XML_val, OString::number(nIdx));

        // export custom position of data label
        if( eChartType != chart::TYPEID_PIE )
        {
            chart2::RelativePosition aCustomLabelPosition;
            if( xLabelPropSet->getPropertyValue("CustomLabelPosition") >>= aCustomLabelPosition )
            {
                pFS->startElement(FSNS(XML_c, XML_layout));
                pFS->startElement(FSNS(XML_c, XML_manualLayout));

                pFS->singleElement(FSNS(XML_c, XML_x), XML_val, OString::number(aCustomLabelPosition.Primary));
                pFS->singleElement(FSNS(XML_c, XML_y), XML_val, OString::number(aCustomLabelPosition.Secondary));

                SAL_WARN_IF(aCustomLabelPosition.Anchor != css::drawing::Alignment_TOP_LEFT, "oox", "unsupported anchor position");

                pFS->endElement(FSNS(XML_c, XML_manualLayout));
                pFS->endElement(FSNS(XML_c, XML_layout));
            }
        }

        if( GetProperty(xLabelPropSet, "LinkNumberFormatToSource") )
            mAny >>= bLinkedNumFmt;

        if( xLabelPropSet->getPropertyValue("Label") >>= aLabel )
            bLabelIsNumberFormat = aLabel.ShowNumber;
        else
            bLabelIsNumberFormat = true;

        if (GetProperty(xLabelPropSet, bLabelIsNumberFormat ? OUString("NumberFormat") : OUString("PercentageNumberFormat")))
        {
            sal_Int32 nKey = 0;
            mAny >>= nKey;

            OUString aNumberFormatString = getNumberFormatCode(nKey);

            pFS->singleElement(FSNS(XML_c, XML_numFmt), XML_formatCode, aNumberFormatString,
                               XML_sourceLinked, ToPsz10(bLinkedNumFmt));
        }

        // Individual label property that overwrites the baseline.
        writeLabelProperties(pFS, this, xLabelPropSet, aParam, nIdx, rDLblsRange);
        pFS->endElement(FSNS(XML_c, XML_dLbl));
    }

    // Baseline label properties for all labels.
    writeLabelProperties(pFS, this, xPropSet, aParam, -1, rDLblsRange);

    bool bShowLeaderLines = false;
    xPropSet->getPropertyValue("ShowCustomLeaderLines") >>= bShowLeaderLines;
    pFS->singleElement(FSNS(XML_c, XML_showLeaderLines), XML_val, ToPsz10(bShowLeaderLines));

    // Export leader line
    if( eChartType != chart::TYPEID_PIE )
    {
        pFS->startElement(FSNS(XML_c, XML_extLst));
        pFS->startElement(FSNS(XML_c, XML_ext), XML_uri, "{CE6537A1-D6FC-4f65-9D91-7224C49458BB}", FSNS(XML_xmlns, XML_c15), GetFB()->getNamespaceURL(OOX_NS(c15)));
        pFS->singleElement(FSNS(XML_c15, XML_showLeaderLines), XML_val, ToPsz10(bShowLeaderLines));
        pFS->endElement(FSNS(XML_c, XML_ext));
        pFS->endElement(FSNS(XML_c, XML_extLst));
    }
    pFS->endElement(FSNS(XML_c, XML_dLbls));
}

void ChartExport::exportDataPoints(
    const uno::Reference< beans::XPropertySet > & xSeriesProperties,
    sal_Int32 nSeriesLength, sal_Int32 eChartType )
{
    uno::Reference< chart2::XDataSeries > xSeries( xSeriesProperties, uno::UNO_QUERY );
    bool bVaryColorsByPoint = false;
    Sequence< sal_Int32 > aDataPointSeq;
    if( xSeriesProperties.is())
    {
        Any aAny = xSeriesProperties->getPropertyValue( "AttributedDataPoints" );
        aAny >>= aDataPointSeq;
        xSeriesProperties->getPropertyValue( "VaryColorsByPoint" ) >>= bVaryColorsByPoint;
    }

    const sal_Int32 * pPoints = aDataPointSeq.getConstArray();
    sal_Int32 nElement;
    Reference< chart2::XColorScheme > xColorScheme;
    if( mxNewDiagram.is())
        xColorScheme.set( mxNewDiagram->getDefaultColorScheme());

    if( bVaryColorsByPoint && xColorScheme.is() )
    {
        o3tl::sorted_vector< sal_Int32 > aAttrPointSet;
        aAttrPointSet.reserve(aDataPointSeq.getLength());
        for (auto p = pPoints; p < pPoints + aDataPointSeq.getLength(); ++p)
            aAttrPointSet.insert(*p);
        const auto aEndIt = aAttrPointSet.end();
        for( nElement = 0; nElement < nSeriesLength; ++nElement )
        {
            uno::Reference< beans::XPropertySet > xPropSet;
            if( aAttrPointSet.find( nElement ) != aEndIt )
            {
                try
                {
                    xPropSet = SchXMLSeriesHelper::createOldAPIDataPointPropertySet(
                            xSeries, nElement, getModel() );
                }
                catch( const uno::Exception & )
                {
                    DBG_UNHANDLED_EXCEPTION( "oox", "Exception caught during Export of data point" );
                }
            }
            else
            {
                // property set only containing the color
                xPropSet.set( new ColorPropertySet( ColorTransparency, xColorScheme->getColorByIndex( nElement )));
            }

            if( xPropSet.is() )
            {
                FSHelperPtr pFS = GetFS();
                pFS->startElement(FSNS(XML_c, XML_dPt));
                pFS->singleElement(FSNS(XML_c, XML_idx), XML_val, OString::number(nElement));

                switch (eChartType)
                {
                    case chart::TYPEID_PIE:
                    case chart::TYPEID_DOUGHNUT:
                    {
                        if( xPropSet.is() && GetProperty( xPropSet, "SegmentOffset") )
                        {
                            sal_Int32 nOffset = 0;
                            mAny >>= nOffset;
                            if (nOffset)
                                pFS->singleElement( FSNS( XML_c, XML_explosion ),
                                        XML_val, OString::number( nOffset ) );
                        }
                        break;
                    }
                    default:
                        break;
                }
                exportShapeProps( xPropSet );

                pFS->endElement( FSNS( XML_c, XML_dPt ) );
            }
        }
    }

    // Export Data Point Property in Charts even if the VaryColors is false
    if( bVaryColorsByPoint )
        return;

    o3tl::sorted_vector< sal_Int32 > aAttrPointSet;
    aAttrPointSet.reserve(aDataPointSeq.getLength());
    for (auto p = pPoints; p < pPoints + aDataPointSeq.getLength(); ++p)
        aAttrPointSet.insert(*p);
    const auto aEndIt = aAttrPointSet.end();
    for( nElement = 0; nElement < nSeriesLength; ++nElement )
    {
        uno::Reference< beans::XPropertySet > xPropSet;
        if( aAttrPointSet.find( nElement ) != aEndIt )
        {
            try
            {
                xPropSet = SchXMLSeriesHelper::createOldAPIDataPointPropertySet(
                        xSeries, nElement, getModel() );
            }
            catch( const uno::Exception & )
            {
                DBG_UNHANDLED_EXCEPTION( "oox", "Exception caught during Export of data point" );
            }
        }

        if( xPropSet.is() )
        {
            FSHelperPtr pFS = GetFS();
            pFS->startElement(FSNS(XML_c, XML_dPt));
            pFS->singleElement(FSNS(XML_c, XML_idx), XML_val, OString::number(nElement));

            switch( eChartType )
            {
                case chart::TYPEID_BUBBLE:
                case chart::TYPEID_HORBAR:
                case chart::TYPEID_BAR:
                    pFS->singleElement(FSNS(XML_c, XML_invertIfNegative), XML_val, "0");
                    exportShapeProps(xPropSet);
                    break;

                case chart::TYPEID_LINE:
                case chart::TYPEID_SCATTER:
                case chart::TYPEID_RADARLINE:
                    exportMarker(xPropSet);
                    break;

                default:
                    exportShapeProps(xPropSet);
                    break;
            }

            pFS->endElement( FSNS( XML_c, XML_dPt ) );
        }
    }
}

void ChartExport::exportAxesId(bool bPrimaryAxes, bool bCheckCombinedAxes)
{
    sal_Int32 nAxisIdx, nAxisIdy;
    bool bPrimaryAxisExists = false;
    bool bSecondaryAxisExists = false;
    // let's check which axis already exists and which axis is attached to the actual dataseries
    if (maAxes.size() >= 2)
    {
        bPrimaryAxisExists = bPrimaryAxes && maAxes[1].nAxisType == AXIS_PRIMARY_Y;
        bSecondaryAxisExists = !bPrimaryAxes && maAxes[1].nAxisType == AXIS_SECONDARY_Y;
    }
    // tdf#114181 keep axes of combined charts
    if ( bCheckCombinedAxes && ( bPrimaryAxisExists || bSecondaryAxisExists ) )
    {
        nAxisIdx = maAxes[0].nAxisId;
        nAxisIdy = maAxes[1].nAxisId;
    }
    else
    {
        nAxisIdx = lcl_generateRandomValue();
        nAxisIdy = lcl_generateRandomValue();
        AxesType eXAxis = bPrimaryAxes ? AXIS_PRIMARY_X : AXIS_SECONDARY_X;
        AxesType eYAxis = bPrimaryAxes ? AXIS_PRIMARY_Y : AXIS_SECONDARY_Y;
        maAxes.emplace_back( eXAxis, nAxisIdx, nAxisIdy );
        maAxes.emplace_back( eYAxis, nAxisIdy, nAxisIdx );
    }
    FSHelperPtr pFS = GetFS();
    pFS->singleElement(FSNS(XML_c, XML_axId), XML_val, OString::number(nAxisIdx));
    pFS->singleElement(FSNS(XML_c, XML_axId), XML_val, OString::number(nAxisIdy));
    if (mbHasZAxis)
    {
        sal_Int32 nAxisIdz = 0;
        if( isDeep3dChart() )
        {
            nAxisIdz = lcl_generateRandomValue();
            maAxes.emplace_back( AXIS_PRIMARY_Z, nAxisIdz, nAxisIdy );
        }
        pFS->singleElement(FSNS(XML_c, XML_axId), XML_val, OString::number(nAxisIdz));
    }
}

void ChartExport::exportGrouping( bool isBar )
{
    FSHelperPtr pFS = GetFS();
    Reference< XPropertySet > xPropSet( mxDiagram , uno::UNO_QUERY);
    // grouping
    if( GetProperty( xPropSet, "Stacked" ) )
        mAny >>= mbStacked;
    if( GetProperty( xPropSet, "Percent" ) )
        mAny >>= mbPercent;

    const char* grouping = nullptr;
    if (mbStacked)
        grouping = "stacked";
    else if (mbPercent)
        grouping = "percentStacked";
    else
    {
        if( isBar && !isDeep3dChart() )
        {
            grouping = "clustered";
        }
        else
            grouping = "standard";
    }
    pFS->singleElement(FSNS(XML_c, XML_grouping), XML_val, grouping);
}

void ChartExport::exportTrendlines( const Reference< chart2::XDataSeries >& xSeries )
{
    FSHelperPtr pFS = GetFS();
    Reference< chart2::XRegressionCurveContainer > xRegressionCurveContainer( xSeries, UNO_QUERY );
    if( !xRegressionCurveContainer.is() )
        return;

    const Sequence< Reference< chart2::XRegressionCurve > > aRegCurveSeq = xRegressionCurveContainer->getRegressionCurves();
    for( const Reference< chart2::XRegressionCurve >& xRegCurve : aRegCurveSeq )
    {
        if (!xRegCurve.is())
            continue;

        Reference< XPropertySet > xProperties( xRegCurve , uno::UNO_QUERY );

        OUString aService;
        Reference< lang::XServiceName > xServiceName( xProperties, UNO_QUERY );
        if( !xServiceName.is() )
            continue;

        aService = xServiceName->getServiceName();

        if(aService != "com.sun.star.chart2.LinearRegressionCurve" &&
                aService != "com.sun.star.chart2.ExponentialRegressionCurve" &&
                aService != "com.sun.star.chart2.LogarithmicRegressionCurve" &&
                aService != "com.sun.star.chart2.PotentialRegressionCurve" &&
                aService != "com.sun.star.chart2.PolynomialRegressionCurve" &&
                aService != "com.sun.star.chart2.MovingAverageRegressionCurve")
            continue;

        pFS->startElement(FSNS(XML_c, XML_trendline));

        OUString aName;
        xProperties->getPropertyValue("CurveName") >>= aName;
        if(!aName.isEmpty())
        {
            pFS->startElement(FSNS(XML_c, XML_name));
            pFS->writeEscaped(aName);
            pFS->endElement( FSNS( XML_c, XML_name) );
        }

        exportShapeProps( xProperties );

        if( aService == "com.sun.star.chart2.LinearRegressionCurve" )
        {
            pFS->singleElement(FSNS(XML_c, XML_trendlineType), XML_val, "linear");
        }
        else if( aService == "com.sun.star.chart2.ExponentialRegressionCurve" )
        {
            pFS->singleElement(FSNS(XML_c, XML_trendlineType), XML_val, "exp");
        }
        else if( aService == "com.sun.star.chart2.LogarithmicRegressionCurve" )
        {
            pFS->singleElement(FSNS(XML_c, XML_trendlineType), XML_val, "log");
        }
        else if( aService == "com.sun.star.chart2.PotentialRegressionCurve" )
        {
            pFS->singleElement(FSNS(XML_c, XML_trendlineType), XML_val, "power");
        }
        else if( aService == "com.sun.star.chart2.PolynomialRegressionCurve" )
        {
            pFS->singleElement(FSNS(XML_c, XML_trendlineType), XML_val, "poly");

            sal_Int32 aDegree = 2;
            xProperties->getPropertyValue( "PolynomialDegree") >>= aDegree;
            pFS->singleElement(FSNS(XML_c, XML_order), XML_val, OString::number(aDegree));
        }
        else if( aService == "com.sun.star.chart2.MovingAverageRegressionCurve" )
        {
            pFS->singleElement(FSNS(XML_c, XML_trendlineType), XML_val, "movingAvg");

            sal_Int32 aPeriod = 2;
            xProperties->getPropertyValue( "MovingAveragePeriod") >>= aPeriod;

            pFS->singleElement(FSNS(XML_c, XML_period), XML_val, OString::number(aPeriod));
        }
        else
        {
            // should never happen
            // This would produce invalid OOXML files so we check earlier for the type
            assert(false);
        }

        double fExtrapolateForward = 0.0;
        double fExtrapolateBackward = 0.0;

        xProperties->getPropertyValue("ExtrapolateForward") >>= fExtrapolateForward;
        xProperties->getPropertyValue("ExtrapolateBackward") >>= fExtrapolateBackward;

        pFS->singleElement( FSNS( XML_c, XML_forward ),
                XML_val, OString::number(fExtrapolateForward) );

        pFS->singleElement( FSNS( XML_c, XML_backward ),
                XML_val, OString::number(fExtrapolateBackward) );

        bool bForceIntercept = false;
        xProperties->getPropertyValue("ForceIntercept") >>= bForceIntercept;

        if (bForceIntercept)
        {
            double fInterceptValue = 0.0;
            xProperties->getPropertyValue("InterceptValue") >>= fInterceptValue;

            pFS->singleElement( FSNS( XML_c, XML_intercept ),
                XML_val, OString::number(fInterceptValue) );
        }

        // Equation properties
        Reference< XPropertySet > xEquationProperties( xRegCurve->getEquationProperties() );

        // Show Equation
        bool bShowEquation = false;
        xEquationProperties->getPropertyValue("ShowEquation") >>= bShowEquation;

        // Show R^2
        bool bShowCorrelationCoefficient = false;
        xEquationProperties->getPropertyValue("ShowCorrelationCoefficient") >>= bShowCorrelationCoefficient;

        pFS->singleElement( FSNS( XML_c, XML_dispRSqr ),
                XML_val, ToPsz10(bShowCorrelationCoefficient) );

        pFS->singleElement(FSNS(XML_c, XML_dispEq), XML_val, ToPsz10(bShowEquation));

        pFS->endElement( FSNS( XML_c, XML_trendline ) );
    }
}

void ChartExport::exportMarker(const Reference< XPropertySet >& xPropSet)
{
    chart2::Symbol aSymbol;
    if( GetProperty( xPropSet, "Symbol" ) )
        mAny >>= aSymbol;

    if(aSymbol.Style != chart2::SymbolStyle_STANDARD && aSymbol.Style != chart2::SymbolStyle_NONE)
        return;

    FSHelperPtr pFS = GetFS();
    pFS->startElement(FSNS(XML_c, XML_marker));

    sal_Int32 nSymbol = aSymbol.StandardSymbol;
    // TODO: more properties support for marker
    const char* pSymbolType; // no initialization here, to let compiler warn if we have a code path
                             // where it stays uninitialized
    switch( nSymbol )
    {
        case 0:
            pSymbolType = "square";
            break;
        case 1:
            pSymbolType = "diamond";
            break;
        case 2:
        case 3:
        case 4:
        case 5:
            pSymbolType = "triangle";
            break;
        case 8:
            pSymbolType = "circle";
            break;
        case 9:
            pSymbolType = "star";
            break;
        case 10:
            pSymbolType = "x"; // in MS office 2010 built in symbol marker 'X' is represented as 'x'
            break;
        case 11:
            pSymbolType = "plus";
            break;
        case 13:
            pSymbolType = "dash";
            break;
        default:
            pSymbolType = "square";
            break;
    }

    bool bSkipFormatting = false;
    if (aSymbol.Style == chart2::SymbolStyle_NONE)
    {
        bSkipFormatting = true;
        pSymbolType = "none";
    }

    pFS->singleElement(FSNS(XML_c, XML_symbol), XML_val, pSymbolType);

    if (!bSkipFormatting)
    {
        awt::Size aSymbolSize = aSymbol.Size;
        sal_Int32 nSize = std::max( aSymbolSize.Width, aSymbolSize.Height );

        nSize = nSize/250.0*7.0 + 1; // just guessed based on some test cases,
        //the value is always 1 less than the actual value.
        nSize = std::clamp( int(nSize), 2, 72 );
        pFS->singleElement(FSNS(XML_c, XML_size), XML_val, OString::number(nSize));

        pFS->startElement(FSNS(XML_c, XML_spPr));

        util::Color aColor = aSymbol.FillColor;
        if (GetProperty(xPropSet, "Color"))
            mAny >>= aColor;

        if (aColor == -1)
        {
            pFS->singleElement(FSNS(XML_a, XML_noFill));
        }
        else
            WriteSolidFill(::Color(ColorTransparency, aColor));

        pFS->endElement( FSNS( XML_c, XML_spPr ) );
    }

    pFS->endElement( FSNS( XML_c, XML_marker ) );
}

void ChartExport::exportSmooth()
{
    FSHelperPtr pFS = GetFS();
    Reference< XPropertySet > xPropSet( mxDiagram , uno::UNO_QUERY );
    sal_Int32 nSplineType = 0;
    if( GetProperty( xPropSet, "SplineType" ) )
        mAny >>= nSplineType;
    const char* pVal = nSplineType != 0 ? "1" : "0";
    pFS->singleElement(FSNS(XML_c, XML_smooth), XML_val, pVal);
}

void ChartExport::exportFirstSliceAng( )
{
    FSHelperPtr pFS = GetFS();
    sal_Int32 nStartingAngle = 0;
    Reference< XPropertySet > xPropSet( mxDiagram , uno::UNO_QUERY);
    if( GetProperty( xPropSet, "StartingAngle" ) )
        mAny >>= nStartingAngle;

    // convert to ooxml angle
    nStartingAngle = (450 - nStartingAngle ) % 360;
    pFS->singleElement(FSNS(XML_c, XML_firstSliceAng), XML_val, OString::number(nStartingAngle));
}

namespace {

const char* getErrorBarStyle(sal_Int32 nErrorBarStyle)
{
    switch(nErrorBarStyle)
    {
        case cssc::ErrorBarStyle::NONE:
            return nullptr;
        case cssc::ErrorBarStyle::VARIANCE:
            break;
        case cssc::ErrorBarStyle::STANDARD_DEVIATION:
            return "stdDev";
        case cssc::ErrorBarStyle::ABSOLUTE:
            return "fixedVal";
        case cssc::ErrorBarStyle::RELATIVE:
            return "percentage";
        case cssc::ErrorBarStyle::ERROR_MARGIN:
            break;
        case cssc::ErrorBarStyle::STANDARD_ERROR:
            return "stdErr";
        case cssc::ErrorBarStyle::FROM_DATA:
            return "cust";
        default:
            assert(false && "can't happen");
    }
    return nullptr;
}

Reference< chart2::data::XDataSequence>  getLabeledSequence(
        const uno::Sequence< uno::Reference< chart2::data::XLabeledDataSequence > >& aSequences,
        bool bPositive )
{
    OUString aDirection;
    if(bPositive)
        aDirection = "positive";
    else
        aDirection = "negative";

    for( const auto& rSequence : aSequences )
    {
        if( rSequence.is())
        {
            uno::Reference< chart2::data::XDataSequence > xSequence( rSequence->getValues());
            uno::Reference< beans::XPropertySet > xSeqProp( xSequence, uno::UNO_QUERY_THROW );
            OUString aRole;
            if( ( xSeqProp->getPropertyValue( "Role" ) >>= aRole ) &&
                    aRole.match( "error-bars" ) && aRole.indexOf(aDirection) >= 0 )
            {
                return xSequence;
            }
        }
    }

    return Reference< chart2::data::XDataSequence > ();
}

}

void ChartExport::exportErrorBar(const Reference< XPropertySet>& xErrorBarProps, bool bYError)
{
    sal_Int32 nErrorBarStyle = cssc::ErrorBarStyle::NONE;
    xErrorBarProps->getPropertyValue("ErrorBarStyle") >>= nErrorBarStyle;
    const char* pErrorBarStyle = getErrorBarStyle(nErrorBarStyle);
    if(!pErrorBarStyle)
        return;

    FSHelperPtr pFS = GetFS();
    pFS->startElement(FSNS(XML_c, XML_errBars));
    pFS->singleElement(FSNS(XML_c, XML_errDir), XML_val, bYError ? "y" : "x");
    bool bPositive = false, bNegative = false;
    xErrorBarProps->getPropertyValue("ShowPositiveError") >>= bPositive;
    xErrorBarProps->getPropertyValue("ShowNegativeError") >>= bNegative;
    const char* pErrBarType;
    if(bPositive && bNegative)
        pErrBarType = "both";
    else if(bPositive)
        pErrBarType = "plus";
    else if(bNegative)
        pErrBarType = "minus";
    else
    {
        // what the hell should we do now?
        // at least this makes the file valid
        pErrBarType = "both";
    }
    pFS->singleElement(FSNS(XML_c, XML_errBarType), XML_val, pErrBarType);
    pFS->singleElement(FSNS(XML_c, XML_errValType), XML_val, pErrorBarStyle);
    pFS->singleElement(FSNS(XML_c, XML_noEndCap), XML_val, "0");
    if(nErrorBarStyle == cssc::ErrorBarStyle::FROM_DATA)
    {
        uno::Reference< chart2::data::XDataSource > xDataSource(xErrorBarProps, uno::UNO_QUERY);
        Sequence< Reference < chart2::data::XLabeledDataSequence > > aSequences =
            xDataSource->getDataSequences();

        if(bPositive)
        {
            exportSeriesValues(getLabeledSequence(aSequences, true), XML_plus);
        }

        if(bNegative)
        {
            exportSeriesValues(getLabeledSequence(aSequences, false), XML_minus);
        }
    }
    else
    {
        double nVal = 0.0;
        if(nErrorBarStyle == cssc::ErrorBarStyle::STANDARD_DEVIATION)
        {
            xErrorBarProps->getPropertyValue("Weight") >>= nVal;
        }
        else
        {
            if(bPositive)
                xErrorBarProps->getPropertyValue("PositiveError") >>= nVal;
            else
                xErrorBarProps->getPropertyValue("NegativeError") >>= nVal;
        }

        pFS->singleElement(FSNS(XML_c, XML_val), XML_val, OString::number(nVal));
    }

    exportShapeProps( xErrorBarProps );

    pFS->endElement( FSNS( XML_c, XML_errBars) );
}

void ChartExport::exportView3D()
{
    Reference< XPropertySet > xPropSet( mxDiagram , uno::UNO_QUERY);
    if( !xPropSet.is() )
        return;
    FSHelperPtr pFS = GetFS();
    pFS->startElement(FSNS(XML_c, XML_view3D));
    sal_Int32 eChartType = getChartType( );
    // rotX
    if( GetProperty( xPropSet, "RotationHorizontal" ) )
    {
        sal_Int32 nRotationX = 0;
        mAny >>= nRotationX;
        if( nRotationX < 0 )
        {
            if(eChartType == chart::TYPEID_PIE)
            {
            /* In OOXML we get value in 0..90 range for pie chart X rotation , whereas we expect it to be in -90..90 range,
               so we convert that during import. It is modified in View3DConverter::convertFromModel()
               here we convert it back to 0..90 as we received in import */
               nRotationX += 90;  // X rotation (map Chart2 [-179,180] to OOXML [0..90])
            }
            else
                nRotationX += 360; // X rotation (map Chart2 [-179,180] to OOXML [-90..90])
        }
        pFS->singleElement(FSNS(XML_c, XML_rotX), XML_val, OString::number(nRotationX));
    }
    // rotY
    if( GetProperty( xPropSet, "RotationVertical" ) )
    {
        // Y rotation (map Chart2 [-179,180] to OOXML [0..359])
        if( eChartType == chart::TYPEID_PIE && GetProperty( xPropSet, "StartingAngle" ) )
        {
         // Y rotation used as 'first pie slice angle' in 3D pie charts
            sal_Int32 nStartingAngle=0;
            mAny >>= nStartingAngle;
            // convert to ooxml angle
            nStartingAngle = (450 - nStartingAngle ) % 360;
            pFS->singleElement(FSNS(XML_c, XML_rotY), XML_val, OString::number(nStartingAngle));
        }
        else
        {
            sal_Int32 nRotationY = 0;
            mAny >>= nRotationY;
            // Y rotation (map Chart2 [-179,180] to OOXML [0..359])
            if( nRotationY < 0 )
                nRotationY += 360;
            pFS->singleElement(FSNS(XML_c, XML_rotY), XML_val, OString::number(nRotationY));
        }
    }
    // rAngAx
    if( GetProperty( xPropSet, "RightAngledAxes" ) )
    {
        bool bRightAngled = false;
        mAny >>= bRightAngled;
        const char* sRightAngled = bRightAngled ? "1":"0";
        pFS->singleElement(FSNS(XML_c, XML_rAngAx), XML_val, sRightAngled);
    }
    // perspective
    if( GetProperty( xPropSet, "Perspective" ) )
    {
        sal_Int32 nPerspective = 0;
        mAny >>= nPerspective;
        // map Chart2 [0,100] to OOXML [0..200]
        nPerspective *= 2;
        pFS->singleElement(FSNS(XML_c, XML_perspective), XML_val, OString::number(nPerspective));
    }
    pFS->endElement( FSNS( XML_c, XML_view3D ) );
}

bool ChartExport::isDeep3dChart()
{
    bool isDeep = false;
    if( mbIs3DChart )
    {
        Reference< XPropertySet > xPropSet( mxDiagram , uno::UNO_QUERY);
        if( GetProperty( xPropSet, "Deep" ) )
            mAny >>= isDeep;
    }
    return isDeep;
}

OUString ChartExport::getNumberFormatCode(sal_Int32 nKey) const
{
    /* XXX if this was called more than one or two times per export the two
     * SvNumberFormatter instances and NfKeywordTable should be member
     * variables and initialized only once. */

    OUString aCode("General");  // init with fallback
    uno::Reference<util::XNumberFormatsSupplier> xNumberFormatsSupplier(mxChartModel, uno::UNO_QUERY_THROW);
    SvNumberFormatsSupplierObj* pSupplierObj = comphelper::getFromUnoTunnel<SvNumberFormatsSupplierObj>( xNumberFormatsSupplier);
    if (!pSupplierObj)
        return aCode;

    SvNumberFormatter* pNumberFormatter = pSupplierObj->GetNumberFormatter();
    if (!pNumberFormatter)
        return aCode;

    SvNumberFormatter aTempFormatter( comphelper::getProcessComponentContext(), LANGUAGE_ENGLISH_US);
    NfKeywordTable aKeywords;
    aTempFormatter.FillKeywordTableForExcel( aKeywords);
    aCode = pNumberFormatter->GetFormatStringForExcel( nKey, aKeywords, aTempFormatter);

    return aCode;
}

}// oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
