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

#include <ChartTypeTemplate.hxx>
#include <DataInterpreter.hxx>
#include <CommonConverters.hxx>
#include <ChartTypeHelper.hxx>
#include <ChartType.hxx>
#include <DataSeriesProperties.hxx>
#include <DataSource.hxx>

#include <Axis.hxx>
#include <AxisHelper.hxx>
#include <Diagram.hxx>
#include <DiagramHelper.hxx>
#include <AxisIndexDefines.hxx>
#include <BaseCoordinateSystem.hxx>
#include <unonames.hxx>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/chart2/AxisType.hpp>
#include <com/sun/star/chart2/StackingDirection.hpp>
#include <com/sun/star/chart2/XColorScheme.hpp>
#include <comphelper/diagnose_ex.hxx>
#include <comphelper/property.hxx>

#include <algorithm>
#include <cstddef>
#include <utility>

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using namespace ::chart::DataSeriesProperties;

using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;

namespace
{

void lcl_applyDefaultStyle(
    const rtl::Reference< ::chart::DataSeries > & xSeries,
    sal_Int32 nIndex,
    const rtl::Reference< ::chart::Diagram > & xDiagram )
{
    // @deprecated: correct default color should be found by view without
    // setting color as hard attribute
    if( xSeries.is() && xDiagram.is())
    {
        Reference< chart2::XColorScheme > xColorScheme( xDiagram->getDefaultColorScheme());
        if( xColorScheme.is() )
            xSeries->setPropertyValue(
                "Color",
                uno::Any( xColorScheme->getColorByIndex( nIndex )));
    }
}

void lcl_ensureCorrectLabelPlacement( const Reference< beans::XPropertySet >& xProp, const uno::Sequence < sal_Int32 >& rAvailablePlacements )
{
    sal_Int32 nLabelPlacement=0;
    if( !(xProp.is() && (xProp->getPropertyValue( "LabelPlacement" ) >>= nLabelPlacement)) )
        return;

    bool bValid = false;
    for( sal_Int32 i : rAvailablePlacements )
    {
        if( i == nLabelPlacement )
        {
            bValid = true;
            break;
        }
    }
    if( !bValid )
    {
        uno::Any aNewValue;
        //otherwise use the first supported one
        if( rAvailablePlacements.hasElements() )
            aNewValue <<=rAvailablePlacements[0];
        xProp->setPropertyValue( "LabelPlacement", aNewValue );
    }
}

void lcl_resetLabelPlacementIfDefault( const Reference< beans::XPropertySet >& xProp, sal_Int32 nDefaultPlacement )
{

    sal_Int32 nLabelPlacement=0;
    if( xProp.is() && (xProp->getPropertyValue( "LabelPlacement" ) >>= nLabelPlacement) )
    {
        if( nDefaultPlacement == nLabelPlacement )
            xProp->setPropertyValue( "LabelPlacement", uno::Any() );
    }
}

void lcl_ensureCorrectMissingValueTreatment( const rtl::Reference< ::chart::Diagram >& xDiagram, const rtl::Reference< ::chart::ChartType >& xChartType )
{
    if( xDiagram.is() )
    {
        uno::Sequence < sal_Int32 > aAvailableMissingValueTreatment(
            ::chart::ChartTypeHelper::getSupportedMissingValueTreatments( xChartType ) );

        if( aAvailableMissingValueTreatment.hasElements() )
            xDiagram->setPropertyValue( "MissingValueTreatment", uno::Any( aAvailableMissingValueTreatment[0] ) );
        else
            xDiagram->setPropertyValue( "MissingValueTreatment", uno::Any() );
    }
}

} // anonymous namespace

namespace chart
{

ChartTypeTemplate::ChartTypeTemplate(
    Reference< uno::XComponentContext > const & xContext,
    OUString aServiceName ) :
        m_xContext( xContext ),
        m_aServiceName(std::move( aServiceName ))
{
}

ChartTypeTemplate::~ChartTypeTemplate()
{}

// ____ ChartTypeTemplate ____
rtl::Reference< Diagram > ChartTypeTemplate::createDiagramByDataSource2(
    const uno::Reference< data::XDataSource >& xDataSource,
    const uno::Sequence< beans::PropertyValue >& aArguments )
{
    rtl::Reference< Diagram > xDia;

    try
    {
        // create diagram
        xDia = new Diagram(GetComponentContext());

        // modify diagram
        rtl::Reference< DataInterpreter > xInterpreter( getDataInterpreter2());
        InterpretedData aData(
            xInterpreter->interpretDataSource(
                xDataSource, aArguments, {} ));

        sal_Int32 nCount = 0;
        for( auto const & i : aData.Series )
            for( auto const & j : i )
                lcl_applyDefaultStyle( j, nCount++, xDia );

        std::vector< rtl::Reference< ChartType > > aOldChartTypesSeq;
        FillDiagram( xDia, aData.Series, aData.Categories, aOldChartTypesSeq );
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }

    return xDia;
}

sal_Bool SAL_CALL ChartTypeTemplate::supportsCategories()
{
    return true;
}

void ChartTypeTemplate::changeDiagram( const rtl::Reference< Diagram >& xDiagram )
{
    if( ! xDiagram.is())
        return;

    try
    {
        std::vector< std::vector< rtl::Reference< DataSeries > > > aSeriesSeq =
            xDiagram->getDataSeriesGroups();
        std::vector< rtl::Reference< DataSeries > > aFlatSeriesSeq( FlattenSequence( aSeriesSeq ));
        const sal_Int32 nFormerSeriesCount = aFlatSeriesSeq.size();

        // chart-type specific interpretation of existing data series
        rtl::Reference< DataInterpreter > xInterpreter( getDataInterpreter2());
        InterpretedData aData;
        aData.Series = aSeriesSeq;
        aData.Categories = xDiagram->getCategories();

        if( xInterpreter->isDataCompatible( aData ) )
        {
            aData = xInterpreter->reinterpretDataSeries( aData );
        }
        else
        {
            rtl::Reference< DataSource > xSource = DataInterpreter::mergeInterpretedData( aData );
            // todo: get a "range-union" from the data provider by calling
            // OUString aRange = getRangeRepresentationByData( xSource );
            // xSource.set( getDataByRangeRepresentation( aRange, aParam ));
            // where aParam == ??
            Sequence< beans::PropertyValue > aParam;
            if( aData.Categories.is())
            {
                aParam = { beans::PropertyValue( "HasCategories", -1, uno::Any( true ),
                                                 beans::PropertyState_DIRECT_VALUE ) };
            }
            aData = xInterpreter->interpretDataSource( xSource, aParam, aFlatSeriesSeq );
        }
        aSeriesSeq = aData.Series;

        sal_Int32 nIndex = 0;
        for (auto const& i : aSeriesSeq)
            for( auto const & j : i )
            {
                if( nIndex >= nFormerSeriesCount )
                    lcl_applyDefaultStyle( j, nIndex++, xDiagram );
            }

        // remove charttype groups from all coordinate systems
        std::vector< rtl::Reference< ChartType > > aOldChartTypesSeq =
            xDiagram->getChartTypes();

        for( rtl::Reference< BaseCoordinateSystem > const & coords : xDiagram->getBaseCoordinateSystems() )
        {
            coords->setChartTypes( Sequence< Reference< XChartType > >() );
        }

        FillDiagram( xDiagram, aSeriesSeq, aData.Categories, aOldChartTypesSeq );
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

void ChartTypeTemplate::changeDiagramData(
    const rtl::Reference< Diagram >& xDiagram,
    const Reference< chart2::data::XDataSource >& xDataSource,
    const Sequence< beans::PropertyValue >& aArguments )
{
    if( ! (xDiagram.is() &&
           xDataSource.is()) )
        return;

    try
    {
        // interpret new data and re-use existing series
        std::vector< rtl::Reference< DataSeries > > aFlatSeriesSeq =
            xDiagram->getDataSeries();
        const sal_Int32 nFormerSeriesCount = aFlatSeriesSeq.size();
        rtl::Reference< DataInterpreter > xInterpreter( getDataInterpreter2());
        InterpretedData aData =
            xInterpreter->interpretDataSource( xDataSource, aArguments, aFlatSeriesSeq );

        // data series
        sal_Int32 nIndex = 0;
        for( std::size_t i=0; i<aData.Series.size(); ++i )
            for( std::size_t j=0; j<aData.Series[i].size(); ++j, ++nIndex )
            {
                if( nIndex >= nFormerSeriesCount )
                {
                    lcl_applyDefaultStyle( aData.Series[i][j], nIndex, xDiagram );
                    applyStyle2( aData.Series[i][j], i, j, aData.Series[i].size() );
                }
            }

        // categories
        xDiagram->setCategories( aData.Categories, true, supportsCategories() );

        std::vector< rtl::Reference< ChartType > > aChartTypes =
            xDiagram->getChartTypes();
        sal_Int32 nMax = std::min( aChartTypes.size(), aData.Series.size());
        for( sal_Int32 i=0; i<nMax; ++i )
        {
            aChartTypes[i]->setDataSeries( aData.Series[i] );
        }
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

bool ChartTypeTemplate::matchesTemplate2(
    const rtl::Reference< ::chart::Diagram >& xDiagram,
    bool /* bAdaptProperties */ )
{
    bool bResult = false;

    if( ! xDiagram.is())
        return bResult;

    try
    {
        const std::vector< rtl::Reference< BaseCoordinateSystem > > & aCooSysSeq(
            xDiagram->getBaseCoordinateSystems());

        // need to have at least one coordinate system
        bResult = !aCooSysSeq.empty();
        if( bResult )
        {
            std::vector< rtl::Reference< ChartType > > aFormerlyUsedChartTypes;
            rtl::Reference<ChartType> xOldCT = getChartTypeForNewSeries2(aFormerlyUsedChartTypes);
            if (!xOldCT.is())
                return false;

            const OUString aChartTypeToMatch = xOldCT->getChartType();
            const sal_Int32 nDimensionToMatch = getDimension();
            for( std::size_t nCooSysIdx=0; bResult && (nCooSysIdx < aCooSysSeq.size()); ++nCooSysIdx )
            {
                // match dimension
                bResult = bResult && (aCooSysSeq[nCooSysIdx]->getDimension() == nDimensionToMatch);

                const std::vector< rtl::Reference< ChartType > > & aChartTypeSeq( aCooSysSeq[nCooSysIdx]->getChartTypes2());
                for( std::size_t nCTIdx=0; bResult && (nCTIdx < aChartTypeSeq.size()); ++nCTIdx )
                {
                    // match chart type
                    bResult = bResult && aChartTypeSeq[nCTIdx]->getChartType() == aChartTypeToMatch;
                    bool bFound=false;
                    bool bAmbiguous=false;
                    // match stacking mode
                    bResult = bResult &&
                        ( DiagramHelper::getStackModeFromChartType(
                            aChartTypeSeq[nCTIdx], bFound, bAmbiguous,
                            aCooSysSeq[nCooSysIdx] )
                          == getStackMode( nCTIdx ) );
                }
            }
        }
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }

    return bResult;
}

rtl::Reference< DataInterpreter > ChartTypeTemplate::getDataInterpreter2()
{
    if( ! m_xDataInterpreter.is())
        m_xDataInterpreter.set( new DataInterpreter );

    return m_xDataInterpreter;
}

void ChartTypeTemplate::applyStyle2(
    const rtl::Reference< DataSeries >& xSeries,
    ::sal_Int32 nChartTypeIndex,
    ::sal_Int32 /* nSeriesIndex */,
    ::sal_Int32 /* nSeriesCount */ )
{
    // sset stacking mode
    if( !xSeries.is())
        return;

    try
    {
        StackMode eStackMode = getStackMode( nChartTypeIndex );
        const uno::Any aPropValue(
            ( (eStackMode == StackMode::YStacked) ||
              (eStackMode == StackMode::YStackedPercent) )
            ? chart2::StackingDirection_Y_STACKING
            : (eStackMode == StackMode::ZStacked )
            ? chart2::StackingDirection_Z_STACKING
            : chart2::StackingDirection_NO_STACKING );
        xSeries->setPropertyValue( "StackingDirection", aPropValue );

        //ensure valid label placement
        {
            uno::Sequence < sal_Int32 > aAvailablePlacements( ChartTypeHelper::getSupportedLabelPlacements(
                        getChartTypeForIndex( nChartTypeIndex ), isSwapXAndY(), xSeries ) );
            lcl_ensureCorrectLabelPlacement( xSeries, aAvailablePlacements );

            uno::Sequence< sal_Int32 > aAttributedDataPointIndexList;
            // "AttributedDataPoints"
            if( xSeries->getFastPropertyValue( PROP_DATASERIES_ATTRIBUTED_DATA_POINTS ) >>= aAttributedDataPointIndexList )
                for(sal_Int32 nN=aAttributedDataPointIndexList.getLength();nN--;)
                    lcl_ensureCorrectLabelPlacement( xSeries->getDataPointByIndex(aAttributedDataPointIndexList[nN]), aAvailablePlacements );
        }
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

void ChartTypeTemplate::applyStyles( const rtl::Reference< ::chart::Diagram >& xDiagram )
{
    // apply chart-type specific styles, like "symbols on" for example
    std::vector< std::vector< rtl::Reference< DataSeries > > > aNewSeriesSeq(
        xDiagram->getDataSeriesGroups());
    for( std::size_t i=0; i<aNewSeriesSeq.size(); ++i )
    {
        const sal_Int32 nNumSeries = aNewSeriesSeq[i].size();
        for( sal_Int32 j=0; j<nNumSeries; ++j )
            applyStyle2( aNewSeriesSeq[i][j], i, j, nNumSeries );
    }

    //ensure valid empty cell handling (for first chart type...)
    lcl_ensureCorrectMissingValueTreatment( xDiagram, getChartTypeForIndex( 0 ) );
}

void ChartTypeTemplate::resetStyles2( const rtl::Reference< ::chart::Diagram >& xDiagram )
{
    // reset number format if we had percent stacking on
    bool bPercent = (getStackMode(0) == StackMode::YStackedPercent);
    if( bPercent )
    {
        const std::vector< rtl::Reference< Axis > > aAxisSeq( AxisHelper::getAllAxesOfDiagram( xDiagram ) );
        for( rtl::Reference< Axis > const & axis : aAxisSeq )
        {
            if( AxisHelper::getDimensionIndexOfAxis( axis, xDiagram )== 1 )
            {
                // set number format to source format
                axis->setPropertyValue(CHART_UNONAME_LINK_TO_SRC_NUMFMT, uno::Any(true));
                axis->setPropertyValue(CHART_UNONAME_NUMFMT, uno::Any());
            }
        }
    }

    //reset label placement if default
    for( rtl::Reference< BaseCoordinateSystem > const & xCooSys : xDiagram->getBaseCoordinateSystems() )
    {
        //iterate through all chart types in the current coordinate system
        for( rtl::Reference< ChartType > const & xChartType : xCooSys->getChartTypes2() )
        {
            //iterate through all series in this chart type
            for( rtl::Reference< DataSeries > const & xSeries : xChartType->getDataSeries2() )
            {
                uno::Sequence < sal_Int32 > aAvailablePlacements( ChartTypeHelper::getSupportedLabelPlacements(
                    xChartType, isSwapXAndY(), xSeries ) );
                if(!aAvailablePlacements.hasElements())
                    continue;

                sal_Int32 nDefaultPlacement = aAvailablePlacements[0];

                lcl_resetLabelPlacementIfDefault( xSeries, nDefaultPlacement );

                uno::Sequence< sal_Int32 > aAttributedDataPointIndexList;
                // "AttributedDataPoints"
                if( xSeries->getFastPropertyValue( PROP_DATASERIES_ATTRIBUTED_DATA_POINTS ) >>= aAttributedDataPointIndexList )
                    for(sal_Int32 nN=aAttributedDataPointIndexList.getLength();nN--;)
                        lcl_resetLabelPlacementIfDefault( xSeries->getDataPointByIndex(aAttributedDataPointIndexList[nN]), nDefaultPlacement );
            }
        }
    }

}

// ____ XServiceName ____
    OUString SAL_CALL ChartTypeTemplate::getServiceName()
{
    return m_aServiceName;
}

sal_Int32 ChartTypeTemplate::getDimension() const
{
    return 2;
}

StackMode ChartTypeTemplate::getStackMode( sal_Int32 /* nChartTypeIndex */ ) const
{
    return StackMode::NONE;
}

bool ChartTypeTemplate::isSwapXAndY() const
{
    return false;
}

void ChartTypeTemplate::createCoordinateSystems(
    const rtl::Reference< ::chart::Diagram > & xDiagram )
{
    if( ! xDiagram.is())
        return;
    std::vector< rtl::Reference< ChartType > > aFormerlyUsedChartTypes;
    rtl::Reference< ChartType > xChartType( getChartTypeForNewSeries2(aFormerlyUsedChartTypes));
    if( ! xChartType.is())
        return;
    rtl::Reference< BaseCoordinateSystem > xCooSys = xChartType->createCoordinateSystem2( getDimension());
    if( ! xCooSys.is())
    {
        // chart type wants no coordinate systems
        xDiagram->setCoordinateSystems( Sequence< Reference< XCoordinateSystem > >());
        return;
    }
    // #i69680# make grid of first y-axis visible (was in the CooSys CTOR before)
    if( xCooSys->getDimension() >= 2 )
    {
        rtl::Reference< Axis > xAxis = xCooSys->getAxisByDimension2( 1, 0 );
        if( xAxis.is())
            AxisHelper::makeGridVisible( xAxis->getGridProperties2() );
    }

    std::vector< rtl::Reference< BaseCoordinateSystem > > aCoordinateSystems(
        xDiagram->getBaseCoordinateSystems());

    if( !aCoordinateSystems.empty() )
    {
        bool bOk = true;
        for( std::size_t i=0; bOk && i<aCoordinateSystems.size(); ++i )
            bOk = bOk && ( xCooSys->getCoordinateSystemType() == aCoordinateSystems[i]->getCoordinateSystemType() &&
                           (xCooSys->getDimension() == aCoordinateSystems[i]->getDimension()) );
        // coordinate systems are ok
        if( bOk )
            return;
        // there are coordinate systems but they do not fit.  So overwrite them.
    }

    //copy as much info from former coordinate system as possible:
    if( !aCoordinateSystems.empty() )
    {
        rtl::Reference< BaseCoordinateSystem > xOldCooSys( aCoordinateSystems[0] );
        sal_Int32 nMaxDimensionCount = std::min( xCooSys->getDimension(), xOldCooSys->getDimension() );

        for(sal_Int32 nDimensionIndex=0; nDimensionIndex<nMaxDimensionCount; nDimensionIndex++)
        {
            const sal_Int32 nMaximumAxisIndex = xOldCooSys->getMaximumAxisIndexByDimension(nDimensionIndex);
            for(sal_Int32 nAxisIndex=0; nAxisIndex<=nMaximumAxisIndex; ++nAxisIndex)
            {
                rtl::Reference< Axis > xAxis = xOldCooSys->getAxisByDimension2( nDimensionIndex, nAxisIndex );
                if( xAxis.is())
                {
                    xCooSys->setAxisByDimension( nDimensionIndex, xAxis, nAxisIndex );
                }
            }
        }
    }

    // set new coordinate systems
    aCoordinateSystems = { xCooSys };

    xDiagram->setCoordinateSystems( aCoordinateSystems );
}

void ChartTypeTemplate::adaptScales(
    const std::vector< rtl::Reference< BaseCoordinateSystem > > & aCooSysSeq,
    const Reference< data::XLabeledDataSequence > & xCategories //@todo: in future there may be more than one sequence of categories (e.g. charttype with categories at x and y axis )
    )
{
    bool bSupportsCategories( supportsCategories() );
    for( rtl::Reference< BaseCoordinateSystem > const & xCooSys : aCooSysSeq )
    {
        try
        {
            // attach categories to first axis
            sal_Int32 nDim( xCooSys->getDimension());
            if( nDim > 0 )
            {
                const sal_Int32 nDimensionX = 0;
                const sal_Int32 nMaxIndex = xCooSys->getMaximumAxisIndexByDimension(nDimensionX);
                for(sal_Int32 nI=0; nI<=nMaxIndex; ++nI)
                {
                    rtl::Reference< Axis > xAxis = xCooSys->getAxisByDimension2(nDimensionX,nI);
                    if( xAxis.is())
                    {
                        ScaleData aData( xAxis->getScaleData() );
                        aData.Categories = xCategories;
                        if(bSupportsCategories)
                        {
                            rtl::Reference< ChartType > xChartType = getChartTypeForNewSeries2({});
                            if( aData.AxisType == AxisType::CATEGORY )
                            {
                                // Shift for Column, Hi-Lo-Close, and regular
                                // Bar types, but not BarOfPie
                                aData.ShiftedCategoryPosition =
                                    m_aServiceName.indexOf("Column") != -1 ||
                                    (m_aServiceName.indexOf("Bar") != -1 &&
                                     !m_aServiceName.indexOf("BarOfPie")) ||
                                    m_aServiceName.endsWith("Close");
                            }
                            bool bSupportsDates = ::chart::ChartTypeHelper::isSupportingDateAxis( xChartType, nDimensionX );
                            if( aData.AxisType != AxisType::CATEGORY && ( aData.AxisType != AxisType::DATE || !bSupportsDates) )
                            {
                                aData.AxisType = AxisType::CATEGORY;
                                aData.AutoDateAxis = true;
                                AxisHelper::removeExplicitScaling( aData );
                            }
                        }
                        else
                            aData.AxisType = AxisType::REALNUMBER;

                        xAxis->setScaleData( aData );
                    }
                }
            }
            // set percent stacking mode at second axis
            if( nDim > 1 )
            {
                const sal_Int32 nMaxIndex = xCooSys->getMaximumAxisIndexByDimension(1);
                for(sal_Int32 nI=0; nI<=nMaxIndex; ++nI)
                {
                    rtl::Reference< Axis > xAxis = xCooSys->getAxisByDimension2( 1,nI );
                    if( xAxis.is())
                    {
                        bool bPercent = (getStackMode(0) == StackMode::YStackedPercent);
                        chart2::ScaleData aScaleData = xAxis->getScaleData();

                        if( bPercent != (aScaleData.AxisType==AxisType::PERCENT) )
                        {
                            if( bPercent )
                                aScaleData.AxisType = AxisType::PERCENT;
                            else
                                aScaleData.AxisType = AxisType::REALNUMBER;
                            xAxis->setScaleData( aScaleData );
                        }
                    }
                }
            }
        }
        catch( const uno::Exception & )
        {
            DBG_UNHANDLED_EXCEPTION("chart2");
        }
    }
}

void ChartTypeTemplate::adaptDiagram( const rtl::Reference< ::chart::Diagram > & /* xDiagram */ )
{
}

void ChartTypeTemplate::createAxes(
    const std::vector< rtl::Reference< BaseCoordinateSystem > > & rCoordSys )
{
    //create missing axes
    if( rCoordSys.empty() )
        return;

    rtl::Reference< BaseCoordinateSystem > xCooSys( rCoordSys[0] );
    if(!xCooSys.is())
        return;

    //create main axis in first coordinate system
    sal_Int32 nDimCount = xCooSys->getDimension();
    sal_Int32 nDim=0;
    for( nDim=0; nDim<nDimCount; ++nDim )
    {
        sal_Int32 nAxisCount = getAxisCountByDimension( nDim );
        if( nDim == 1 &&
            nAxisCount < 2 && AxisHelper::isSecondaryYAxisNeeded( xCooSys ))
            nAxisCount = 2;
        for( sal_Int32 nAxisIndex = 0; nAxisIndex < nAxisCount; ++nAxisIndex )
        {
            Reference< XAxis > xAxis = AxisHelper::getAxis( nDim, nAxisIndex, xCooSys );
            if( !xAxis.is())
            {
                // create and add axis
                xAxis.set( AxisHelper::createAxis(
                               nDim, nAxisIndex, xCooSys, GetComponentContext() ));
            }
        }
    }
}

void ChartTypeTemplate::adaptAxes(
    const std::vector< rtl::Reference< BaseCoordinateSystem > > & rCoordSys )
{
    //adapt properties of existing axes and remove superfluous axes

    if( rCoordSys.empty() )
        return;

    for( rtl::Reference< BaseCoordinateSystem > const & xCooSys : rCoordSys )
    {
        if( !xCooSys.is() )
            continue;
        sal_Int32 nDimCount = xCooSys->getDimension();
        for( sal_Int32 nDim=0; nDim<nDimCount; ++nDim )
        {
            sal_Int32 nMaxAxisIndex = xCooSys->getMaximumAxisIndexByDimension( nDim );
            for( sal_Int32 nAxisIndex=0; nAxisIndex<=nMaxAxisIndex; nAxisIndex++ )
            {
                rtl::Reference< Axis > xAxis = AxisHelper::getAxis( nDim, nAxisIndex, xCooSys );
                if( !xAxis.is() )
                    continue;

                if( nAxisIndex == MAIN_AXIS_INDEX || nAxisIndex == SECONDARY_AXIS_INDEX )
                {
                    // adapt scales
                    bool bPercent = (getStackMode(0) == StackMode::YStackedPercent);
                    if( bPercent && nDim == 1 )
                    {
                        // set number format to source format
                        xAxis->setPropertyValue(CHART_UNONAME_LINK_TO_SRC_NUMFMT, uno::Any(true));
                        xAxis->setPropertyValue(CHART_UNONAME_NUMFMT, uno::Any());
                    }
                }
            }
        }
    }
}

sal_Int32 ChartTypeTemplate::getAxisCountByDimension( sal_Int32 nDimension )
{
    return (nDimension < getDimension()) ? 1 : 0;
}

void ChartTypeTemplate::FillDiagram(
    const rtl::Reference< ::chart::Diagram >& xDiagram,
    const std::vector< std::vector< rtl::Reference< DataSeries > > >& aSeriesSeq,
    const uno::Reference< chart2::data::XLabeledDataSequence >& xCategories,
    const std::vector< rtl::Reference< ChartType > >& aOldChartTypesSeq )
{
    adaptDiagram( xDiagram );

    try
    {
        // create coordinate systems and scales
        createCoordinateSystems( xDiagram );
        std::vector< rtl::Reference< BaseCoordinateSystem > > aCoordinateSystems( xDiagram->getBaseCoordinateSystems());
        createAxes( aCoordinateSystems );
        adaptAxes( aCoordinateSystems );
        adaptScales( aCoordinateSystems, xCategories );

        // chart types
        createChartTypes( aSeriesSeq, aCoordinateSystems, aOldChartTypesSeq );
        applyStyles( xDiagram );
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

void ChartTypeTemplate::createChartTypes(
    const std::vector< std::vector< rtl::Reference< DataSeries > > > & aSeriesSeq,
    const std::vector< rtl::Reference< BaseCoordinateSystem > > & rCoordSys,
    const std::vector< rtl::Reference< ChartType > >& aOldChartTypesSeq )
{
    if( rCoordSys.empty() )
        return;

    try
    {
        std::size_t nCooSysIdx=0;
        rtl::Reference< ChartType > xCT;
        if( aSeriesSeq.empty() )
        {
            // we need a new chart type
            xCT = getChartTypeForNewSeries2( aOldChartTypesSeq );
            rCoordSys[nCooSysIdx]->setChartTypes(std::vector{ xCT });
        }
        else
        {
            for( std::size_t nSeriesIdx=0; nSeriesIdx<aSeriesSeq.size(); ++nSeriesIdx )
            {
                if( nSeriesIdx == nCooSysIdx )
                {
                    // we need a new chart type
                    xCT = getChartTypeForNewSeries2( aOldChartTypesSeq );
                    std::vector< rtl::Reference< ChartType > > aCTSeq( rCoordSys[nCooSysIdx]->getChartTypes2());
                    if( !aCTSeq.empty())
                    {
                        aCTSeq[0] = xCT;
                        rCoordSys[nCooSysIdx]->setChartTypes( aCTSeq );
                    }
                    else
                        rCoordSys[nCooSysIdx]->addChartType( xCT );

                    xCT->setDataSeries( aSeriesSeq[nSeriesIdx] );
                }
                else
                {
                    // reuse existing chart type
                    OSL_ASSERT( xCT.is());
                    std::vector< rtl::Reference< DataSeries > > aNewSeriesSeq = xCT->getDataSeries2();
                    sal_Int32 nNewStartIndex = aNewSeriesSeq.size();
                    aNewSeriesSeq.resize( nNewStartIndex + aSeriesSeq[nSeriesIdx].size() );
                    std::copy( aSeriesSeq[nSeriesIdx].begin(),
                                 aSeriesSeq[nSeriesIdx].end(),
                                 aNewSeriesSeq.begin() + nNewStartIndex );
                    xCT->setDataSeries( aNewSeriesSeq );
                }

                // spread the series over the available coordinate systems
                if( rCoordSys.size() > (nCooSysIdx + 1) )
                    ++nCooSysIdx;
            }
        }
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

void ChartTypeTemplate::copyPropertiesFromOldToNewCoordinateSystem(
                    const std::vector< rtl::Reference< ChartType > > & rOldChartTypesSeq,
                    const rtl::Reference< ChartType > & xNewChartType )
{
    if( !xNewChartType.is() )
        return;

    OUString aNewChartType( xNewChartType->getChartType() );

    Reference< beans::XPropertySet > xSource;
    for( rtl::Reference< ChartType > const & xOldType : rOldChartTypesSeq )
    {
        if( xOldType.is() && xOldType->getChartType() == aNewChartType )
        {
            xSource = xOldType;
            if( xSource.is() )
                break;
        }
    }
    if( xSource.is() )
        comphelper::copyProperties( xSource, xNewChartType );
}

css::uno::Reference< css::uno::XInterface > ChartTypeTemplate::getDataInterpreter()
{
    return static_cast<cppu::OWeakObject*>(getDataInterpreter2().get());
}
css::uno::Reference< css::chart2::XDiagram > ChartTypeTemplate::createDiagramByDataSource(
    const css::uno::Reference< css::chart2::data::XDataSource >& xDataSource,
    const css::uno::Sequence< css::beans::PropertyValue >& aArguments )
{
    return createDiagramByDataSource2(xDataSource, aArguments);
}
void ChartTypeTemplate::changeDiagram(
    const css::uno::Reference< css::chart2::XDiagram >& xDiagram )
{
    changeDiagram(rtl::Reference<Diagram>(dynamic_cast<Diagram*>(xDiagram.get())));
}
void ChartTypeTemplate::changeDiagramData(
    const css::uno::Reference< css::chart2::XDiagram >& xDiagram,
    const css::uno::Reference< css::chart2::data::XDataSource >& xDataSource,
    const css::uno::Sequence< css::beans::PropertyValue >& aArguments )
{
    changeDiagramData(rtl::Reference<Diagram>(dynamic_cast<Diagram*>(xDiagram.get())), xDataSource, aArguments);
}
sal_Bool ChartTypeTemplate::matchesTemplate(
    const css::uno::Reference<css::chart2::XDiagram >& xDiagram,
    sal_Bool bAdaptProperties )
{
    return matchesTemplate2(dynamic_cast<Diagram*>(xDiagram.get()), static_cast<bool>(bAdaptProperties));
}
css::uno::Reference< ::css::chart2::XChartType > ChartTypeTemplate::getChartTypeForNewSeries(
    const css::uno::Sequence< css::uno::Reference< css::chart2::XChartType > >& aFormerlyUsedChartTypes )
{
    std::vector< rtl::Reference< ::chart::ChartType > > aTmp;
    aTmp.reserve(aFormerlyUsedChartTypes.getLength());
    for (auto const & rxChartType : aFormerlyUsedChartTypes)
        aTmp.push_back(dynamic_cast<ChartType*>(rxChartType.get()));
    return getChartTypeForNewSeries2(aTmp);
}
void ChartTypeTemplate::applyStyle(
    const css::uno::Reference< css::chart2::XDataSeries >& xSeries,
    ::sal_Int32 nChartTypeIndex,
    ::sal_Int32 nSeriesIndex,
    ::sal_Int32 nSeriesCount )
{
    applyStyle2(dynamic_cast<DataSeries*>(xSeries.get()), nChartTypeIndex, nSeriesIndex, nSeriesCount);
}
void ChartTypeTemplate::resetStyles(
    const css::uno::Reference< css::chart2::XDiagram >& xDiagram )
{
    resetStyles2(dynamic_cast<Diagram*>(xDiagram.get()));
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
