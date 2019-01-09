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

#include "ChartTypeTemplate.hxx"
#include "DataInterpreter.hxx"
#include <CommonConverters.hxx>
#include <ChartTypeHelper.hxx>

#include <AxisHelper.hxx>
#include <DiagramHelper.hxx>
#include <AxisIndexDefines.hxx>
#include <unonames.hxx>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/chart2/AxisType.hpp>
#include <com/sun/star/chart2/StackingDirection.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <tools/diagnose_ex.h>
#include <comphelper/property.hxx>
#include <comphelper/sequence.hxx>

#include <algorithm>

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;

namespace
{

void lcl_applyDefaultStyle(
    const Reference< XDataSeries > & xSeries,
    sal_Int32 nIndex,
    const Reference< XDiagram > & xDiagram )
{
    // @deprecated: correct default color should be found by view without
    // setting color as hard attribute
    if( xSeries.is() && xDiagram.is())
    {
        Reference< beans::XPropertySet > xSeriesProp( xSeries, uno::UNO_QUERY );
        Reference< chart2::XColorScheme > xColorScheme( xDiagram->getDefaultColorScheme());
        if( xSeriesProp.is() && xColorScheme.is() )
            xSeriesProp->setPropertyValue(
                "Color",
                uno::Any( xColorScheme->getColorByIndex( nIndex )));
    }
}

void lcl_ensureCorrectLabelPlacement( const Reference< beans::XPropertySet >& xProp, const uno::Sequence < sal_Int32 >& rAvailablePlacements )
{
    sal_Int32 nLabelPlacement=0;
    if( xProp.is() && (xProp->getPropertyValue( "LabelPlacement" ) >>= nLabelPlacement) )
    {
        bool bValid = false;
        for( sal_Int32 nN = 0; nN < rAvailablePlacements.getLength(); nN++ )
        {
            if( rAvailablePlacements[nN] == nLabelPlacement )
            {
                bValid = true;
                break;
            }
        }
        if( !bValid )
        {
            uno::Any aNewValue;
            //otherwise use the first supported one
            if( rAvailablePlacements.getLength() )
                aNewValue <<=rAvailablePlacements[0];
            xProp->setPropertyValue( "LabelPlacement", aNewValue );
        }
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

void lcl_ensureCorrectMissingValueTreatment( const Reference< chart2::XDiagram >& xDiagram, const Reference< XChartType >& xChartType )
{
    Reference< beans::XPropertySet > xDiaProp( xDiagram, uno::UNO_QUERY );
    if( xDiaProp.is() )
    {
        uno::Sequence < sal_Int32 > aAvailableMissingValueTreatment(
            ::chart::ChartTypeHelper::getSupportedMissingValueTreatments( xChartType ) );

        if( aAvailableMissingValueTreatment.getLength() )
            xDiaProp->setPropertyValue( "MissingValueTreatment", uno::Any( aAvailableMissingValueTreatment[0] ) );
        else
            xDiaProp->setPropertyValue( "MissingValueTreatment", uno::Any() );
    }
}

} // anonymous namespace

namespace chart
{

ChartTypeTemplate::ChartTypeTemplate(
    Reference< uno::XComponentContext > const & xContext,
    const OUString & rServiceName ) :
        m_xContext( xContext ),
        m_aServiceName( rServiceName )
{
}

ChartTypeTemplate::~ChartTypeTemplate()
{}

// ____ XChartTypeTemplate ____
uno::Reference< XDiagram > SAL_CALL ChartTypeTemplate::createDiagramByDataSource(
    const uno::Reference< data::XDataSource >& xDataSource,
    const uno::Sequence< beans::PropertyValue >& aArguments )
{
    Reference< XDiagram > xDia;

    try
    {
        // create diagram
        xDia.set(
            GetComponentContext()->getServiceManager()->createInstanceWithContext(
                "com.sun.star.chart2.Diagram",
                GetComponentContext() ),
            uno::UNO_QUERY_THROW );

        // modify diagram
        Reference< chart2::XDataInterpreter > xInterpreter( getDataInterpreter());
        chart2::InterpretedData aData(
            xInterpreter->interpretDataSource(
                xDataSource, aArguments, Sequence< Reference< XDataSeries > >() ));

        Sequence< Sequence< Reference< XDataSeries > > > aSeries( aData.Series );
        sal_Int32 i, j, nCount = 0;
        for( i=0; i<aSeries.getLength(); ++i )
        {
            for( j=0; j<aSeries[i].getLength(); ++j, ++nCount )
                lcl_applyDefaultStyle( aSeries[i][j], nCount, xDia );
        }

        Sequence< Reference< XChartType > > aOldChartTypesSeq;
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

void SAL_CALL ChartTypeTemplate::changeDiagram( const uno::Reference< XDiagram >& xDiagram )
{
    if( ! xDiagram.is())
        return;

    try
    {
        Sequence< Sequence< Reference< XDataSeries > > > aSeriesSeq(
            DiagramHelper::getDataSeriesGroups( xDiagram ));
        Sequence< Reference< XDataSeries > > aFlatSeriesSeq( FlattenSequence( aSeriesSeq ));
        const sal_Int32 nFormerSeriesCount = aFlatSeriesSeq.getLength();

        // chart-type specific interpretation of existing data series
        Reference< chart2::XDataInterpreter > xInterpreter( getDataInterpreter());
        chart2::InterpretedData aData;
        aData.Series = aSeriesSeq;
        aData.Categories = DiagramHelper::getCategoriesFromDiagram( xDiagram );

        if( xInterpreter->isDataCompatible( aData ) )
        {
            aData = xInterpreter->reinterpretDataSeries( aData );
        }
        else
        {
            Reference< data::XDataSource > xSource( xInterpreter->mergeInterpretedData( aData ));
            // todo: get a "range-union" from the data provider by calling
            // OUString aRange = getRangeRepresentationByData( xSource );
            // xSource.set( getDataByRangeRepresentation( aRange, aParam ));
            // where aParam == ??
            Sequence< beans::PropertyValue > aParam;
            if( aData.Categories.is())
            {
                aParam.realloc( 1 );
                aParam[0] = beans::PropertyValue( "HasCategories", -1, uno::Any( true ),
                                                  beans::PropertyState_DIRECT_VALUE );
            }
            aData = xInterpreter->interpretDataSource( xSource, aParam, aFlatSeriesSeq );
        }
        aSeriesSeq = aData.Series;

        sal_Int32 i, j, nIndex = 0;
        for( i=0; i<aSeriesSeq.getLength(); ++i )
            for( j=0; j<aSeriesSeq[i].getLength(); ++j, ++nIndex )
            {
                if( nIndex >= nFormerSeriesCount )
                    lcl_applyDefaultStyle( aSeriesSeq[i][j], nIndex, xDiagram );
            }

        // remove charttype groups from all coordinate systems
        Sequence< Reference< XChartType > > aOldChartTypesSeq(
            DiagramHelper::getChartTypesFromDiagram(xDiagram) );

        Reference< XCoordinateSystemContainer > xCoordSysCnt( xDiagram, uno::UNO_QUERY );
        OSL_ASSERT( xCoordSysCnt.is());
        if( xCoordSysCnt.is())
        {
            Sequence< Reference< XCoordinateSystem > > aCooSysSeq(
                xCoordSysCnt->getCoordinateSystems());
            for( sal_Int32 nCooSysIdx = 0; nCooSysIdx < aCooSysSeq.getLength(); ++nCooSysIdx )
            {
                Reference< XChartTypeContainer > xContainer( aCooSysSeq[nCooSysIdx], uno::UNO_QUERY );
                if( xContainer.is() )
                    xContainer->setChartTypes( Sequence< Reference< XChartType > >() );
            }
        }

        FillDiagram( xDiagram, aSeriesSeq, aData.Categories, aOldChartTypesSeq );
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

void SAL_CALL ChartTypeTemplate::changeDiagramData(
    const Reference< chart2::XDiagram >& xDiagram,
    const Reference< chart2::data::XDataSource >& xDataSource,
    const Sequence< beans::PropertyValue >& aArguments )
{
    if( ! (xDiagram.is() &&
           xDataSource.is()) )
        return;

    try
    {
        // interpret new data and re-use existing series
        Sequence< Reference< XDataSeries > > aFlatSeriesSeq(
            comphelper::containerToSequence( DiagramHelper::getDataSeriesFromDiagram( xDiagram )));
        const sal_Int32 nFormerSeriesCount = aFlatSeriesSeq.getLength();
        Reference< chart2::XDataInterpreter > xInterpreter( getDataInterpreter());
        chart2::InterpretedData aData =
            xInterpreter->interpretDataSource( xDataSource, aArguments, aFlatSeriesSeq );

        // data series
        Sequence< Sequence< Reference< XDataSeries > > > aSeriesSeq( aData.Series );

        sal_Int32 i, j, nIndex = 0;
        for( i=0; i<aSeriesSeq.getLength(); ++i )
            for( j=0; j<aSeriesSeq[i].getLength(); ++j, ++nIndex )
            {
                if( nIndex >= nFormerSeriesCount )
                {
                    lcl_applyDefaultStyle( aSeriesSeq[i][j], nIndex, xDiagram );
                    applyStyle( aSeriesSeq[i][j], i, j, aSeriesSeq[i].getLength() );
                }
            }

        // categories
        DiagramHelper::setCategoriesToDiagram( aData.Categories, xDiagram, true, supportsCategories() );

        Sequence< Reference< XChartType > > aChartTypes(
            DiagramHelper::getChartTypesFromDiagram( xDiagram ));
        sal_Int32 nMax = std::min( aChartTypes.getLength(), aSeriesSeq.getLength());
        for( i=0; i<nMax; ++i )
        {
            Reference< XDataSeriesContainer > xDSCnt( aChartTypes[i], uno::UNO_QUERY_THROW );
            xDSCnt->setDataSeries( aSeriesSeq[i] );
        }
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

sal_Bool SAL_CALL ChartTypeTemplate::matchesTemplate(
    const Reference< chart2::XDiagram >& xDiagram,
    sal_Bool /* bAdaptProperties */ )
{
    bool bResult = false;

    if( ! xDiagram.is())
        return bResult;

    try
    {
        Reference< XCoordinateSystemContainer > xCooSysCnt(
            xDiagram, uno::UNO_QUERY_THROW );
        Sequence< Reference< XCoordinateSystem > > aCooSysSeq(
            xCooSysCnt->getCoordinateSystems());

        // need to have at least one coordinate system
        bResult = (aCooSysSeq.getLength() > 0);
        if( bResult )
        {
            Sequence< Reference< XChartType > > aFormerlyUsedChartTypes;
            Reference<XChartType> xOldCT = getChartTypeForNewSeries(aFormerlyUsedChartTypes);
            if (!xOldCT.is())
                return false;

            const OUString aChartTypeToMatch = xOldCT->getChartType();
            const sal_Int32 nDimensionToMatch = getDimension();
            for( sal_Int32 nCooSysIdx=0; bResult && (nCooSysIdx < aCooSysSeq.getLength()); ++nCooSysIdx )
            {
                // match dimension
                bResult = bResult && (aCooSysSeq[nCooSysIdx]->getDimension() == nDimensionToMatch);

                Reference< XChartTypeContainer > xCTCnt( aCooSysSeq[nCooSysIdx], uno::UNO_QUERY_THROW );
                Sequence< Reference< XChartType > > aChartTypeSeq( xCTCnt->getChartTypes());
                for( sal_Int32 nCTIdx=0; bResult && (nCTIdx < aChartTypeSeq.getLength()); ++nCTIdx )
                {
                    if (!aChartTypeSeq[nCTIdx].is())
                        return false;

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

Reference< chart2::XDataInterpreter > SAL_CALL ChartTypeTemplate::getDataInterpreter()
{
    if( ! m_xDataInterpreter.is())
        m_xDataInterpreter.set( new DataInterpreter );

    return m_xDataInterpreter;
}

void SAL_CALL ChartTypeTemplate::applyStyle(
    const Reference< chart2::XDataSeries >& xSeries,
    ::sal_Int32 nChartTypeIndex,
    ::sal_Int32 /* nSeriesIndex */,
    ::sal_Int32 /* nSeriesCount */ )
{
    // sset stacking mode
    Reference< beans::XPropertySet > xSeriesProp( xSeries, uno::UNO_QUERY );
    if( xSeriesProp.is())
    {
        try
        {
            StackMode eStackMode = getStackMode( nChartTypeIndex );
            const uno::Any aPropValue = uno::Any(
                ( (eStackMode == StackMode::YStacked) ||
                  (eStackMode == StackMode::YStackedPercent) )
                ? chart2::StackingDirection_Y_STACKING
                : (eStackMode == StackMode::ZStacked )
                ? chart2::StackingDirection_Z_STACKING
                : chart2::StackingDirection_NO_STACKING );
            xSeriesProp->setPropertyValue( "StackingDirection", aPropValue );

            //ensure valid label placement
            {
                uno::Sequence < sal_Int32 > aAvailablePlacements( ChartTypeHelper::getSupportedLabelPlacements(
                            getChartTypeForIndex( nChartTypeIndex ), isSwapXAndY(), xSeries ) );
                lcl_ensureCorrectLabelPlacement( xSeriesProp, aAvailablePlacements );

                uno::Sequence< sal_Int32 > aAttributedDataPointIndexList;
                if( xSeriesProp->getPropertyValue( "AttributedDataPoints" ) >>= aAttributedDataPointIndexList )
                    for(sal_Int32 nN=aAttributedDataPointIndexList.getLength();nN--;)
                        lcl_ensureCorrectLabelPlacement( xSeries->getDataPointByIndex(aAttributedDataPointIndexList[nN]), aAvailablePlacements );
            }
        }
        catch( const uno::Exception & )
        {
            DBG_UNHANDLED_EXCEPTION("chart2");
        }
    }
}

void ChartTypeTemplate::applyStyles( const Reference< chart2::XDiagram >& xDiagram )
{
    // apply chart-type specific styles, like "symbols on" for example
    Sequence< Sequence< Reference< XDataSeries > > > aNewSeriesSeq(
        DiagramHelper::getDataSeriesGroups( xDiagram ));
    for( sal_Int32 i=0; i<aNewSeriesSeq.getLength(); ++i )
    {
        const sal_Int32 nNumSeries = aNewSeriesSeq[i].getLength();
        for( sal_Int32 j=0; j<nNumSeries; ++j )
            applyStyle( aNewSeriesSeq[i][j], i, j, nNumSeries );
    }

    //ensure valid empty cell handling (for first chart type...)
    lcl_ensureCorrectMissingValueTreatment( xDiagram, getChartTypeForIndex( 0 ) );
}

void SAL_CALL ChartTypeTemplate::resetStyles( const Reference< chart2::XDiagram >& xDiagram )
{
    // reset number format if we had percent stacking on
    bool bPercent = (getStackMode(0) == StackMode::YStackedPercent);
    if( bPercent )
    {
        Sequence< Reference< chart2::XAxis > > aAxisSeq( AxisHelper::getAllAxesOfDiagram( xDiagram ) );
        for( sal_Int32 i=0; i<aAxisSeq.getLength(); ++i )
        {
            if( AxisHelper::getDimensionIndexOfAxis( aAxisSeq[i], xDiagram )== 1 )
            {
                Reference< beans::XPropertySet > xAxisProp( aAxisSeq[i], uno::UNO_QUERY );
                if( xAxisProp.is())
                {
                    // set number format to source format
                    xAxisProp->setPropertyValue(CHART_UNONAME_LINK_TO_SRC_NUMFMT, uno::Any(true));
                    xAxisProp->setPropertyValue(CHART_UNONAME_NUMFMT, uno::Any());
                }
            }
        }
    }

    //reset label placement if default
    {
        uno::Reference< XCoordinateSystemContainer > xCooSysContainer( xDiagram, uno::UNO_QUERY );
        if( xCooSysContainer.is() )
        {
            uno::Sequence< uno::Reference< XCoordinateSystem > > aCooSysList( xCooSysContainer->getCoordinateSystems() );
            for( sal_Int32 nCS = 0; nCS < aCooSysList.getLength(); ++nCS )
            {
                uno::Reference< XCoordinateSystem > xCooSys( aCooSysList[nCS] );

                //iterate through all chart types in the current coordinate system
                uno::Reference< XChartTypeContainer > xChartTypeContainer( xCooSys, uno::UNO_QUERY );
                OSL_ASSERT( xChartTypeContainer.is());
                if( !xChartTypeContainer.is() )
                    continue;
                uno::Sequence< uno::Reference< XChartType > > aChartTypeList( xChartTypeContainer->getChartTypes() );
                for( sal_Int32 nT = 0; nT < aChartTypeList.getLength(); ++nT )
                {
                    uno::Reference< XChartType > xChartType( aChartTypeList[nT] );

                    //iterate through all series in this chart type
                    uno::Reference< XDataSeriesContainer > xDataSeriesContainer( xChartType, uno::UNO_QUERY );
                    OSL_ASSERT( xDataSeriesContainer.is());
                    if( !xDataSeriesContainer.is() )
                        continue;

                    uno::Sequence< uno::Reference< XDataSeries > > aSeriesList( xDataSeriesContainer->getDataSeries() );
                    for( sal_Int32 nS = 0; nS < aSeriesList.getLength(); ++nS )
                    {
                        Reference< XDataSeries > xSeries(aSeriesList[nS]);
                        Reference< beans::XPropertySet > xSeriesProp( xSeries, uno::UNO_QUERY );
                        if(!xSeries.is() || !xSeriesProp.is() )
                            continue;

                        uno::Sequence < sal_Int32 > aAvailablePlacements( ChartTypeHelper::getSupportedLabelPlacements(
                            xChartType, isSwapXAndY(), xSeries ) );
                        if(!aAvailablePlacements.getLength())
                            continue;

                        sal_Int32 nDefaultPlacement = aAvailablePlacements[0];

                        lcl_resetLabelPlacementIfDefault( xSeriesProp, nDefaultPlacement );

                        uno::Sequence< sal_Int32 > aAttributedDataPointIndexList;
                        if( xSeriesProp->getPropertyValue( "AttributedDataPoints" ) >>= aAttributedDataPointIndexList )
                            for(sal_Int32 nN=aAttributedDataPointIndexList.getLength();nN--;)
                                lcl_resetLabelPlacementIfDefault( xSeries->getDataPointByIndex(aAttributedDataPointIndexList[nN]), nDefaultPlacement );
                    }
                }
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
    const Reference< chart2::XCoordinateSystemContainer > & xOutCooSysCnt )
{
    if( ! xOutCooSysCnt.is())
        return;
    Sequence< Reference< XChartType > > aFormerlyUsedChartTypes;
    Reference< XChartType > xChartType( getChartTypeForNewSeries(aFormerlyUsedChartTypes));
    if( ! xChartType.is())
        return;
    Reference< XCoordinateSystem > xCooSys( xChartType->createCoordinateSystem( getDimension()));
    if( ! xCooSys.is())
    {
        // chart type wants no coordinate systems
        xOutCooSysCnt->setCoordinateSystems( Sequence< Reference< XCoordinateSystem > >());
        return;
    }
    // #i69680# make grid of first y-axis visible (was in the CooSys CTOR before)
    if( xCooSys->getDimension() >= 2 )
    {
        Reference< chart2::XAxis > xAxis( xCooSys->getAxisByDimension( 1, 0 ));
        if( xAxis.is())
            AxisHelper::makeGridVisible( xAxis->getGridProperties() );
    }

    Sequence< Reference< XCoordinateSystem > > aCoordinateSystems(
        xOutCooSysCnt->getCoordinateSystems());

    if( aCoordinateSystems.getLength())
    {
        bool bOk = true;
        for( sal_Int32 i=0; bOk && i<aCoordinateSystems.getLength(); ++i )
            bOk = bOk && ( xCooSys->getCoordinateSystemType() == aCoordinateSystems[i]->getCoordinateSystemType() &&
                           (xCooSys->getDimension() == aCoordinateSystems[i]->getDimension()) );
        // coordinate systems are ok
        if( bOk )
            return;
        // there are coordinate systems but they do not fit.  So overwrite them.
    }

    //copy as much info from former coordinate system as possible:
    if( aCoordinateSystems.getLength() )
    {
        Reference< XCoordinateSystem > xOldCooSys( aCoordinateSystems[0] );
        sal_Int32 nMaxDimensionCount = std::min( xCooSys->getDimension(), xOldCooSys->getDimension() );

        for(sal_Int32 nDimensionIndex=0; nDimensionIndex<nMaxDimensionCount; nDimensionIndex++)
        {
            const sal_Int32 nMaximumAxisIndex = xOldCooSys->getMaximumAxisIndexByDimension(nDimensionIndex);
            for(sal_Int32 nAxisIndex=0; nAxisIndex<=nMaximumAxisIndex; ++nAxisIndex)
            {
                uno::Reference< XAxis > xAxis( xOldCooSys->getAxisByDimension( nDimensionIndex, nAxisIndex ) );
                if( xAxis.is())
                {
                    xCooSys->setAxisByDimension( nDimensionIndex, xAxis, nAxisIndex );
                }
            }
        }
    }

    // set new coordinate systems
    aCoordinateSystems.realloc( 1 );
    aCoordinateSystems[0] = xCooSys;

    xOutCooSysCnt->setCoordinateSystems( aCoordinateSystems );
}

void ChartTypeTemplate::adaptScales(
    const Sequence< Reference< chart2::XCoordinateSystem > > & aCooSysSeq,
    const Reference< data::XLabeledDataSequence > & xCategories //@todo: in future there may be more than one sequence of categories (e.g. charttype with categories at x and y axis )
    )
{
    bool bSupportsCategories( supportsCategories() );
    for( sal_Int32 nCooSysIdx=0; nCooSysIdx<aCooSysSeq.getLength(); ++nCooSysIdx )
    {
        try
        {
            Reference< XCoordinateSystem > xCooSys( aCooSysSeq[nCooSysIdx] );
            if( !xCooSys.is() )
                continue;

            // attach categories to first axis
            sal_Int32 nDim( xCooSys->getDimension());
            if( nDim > 0 )
            {
                const sal_Int32 nDimensionX = 0;
                const sal_Int32 nMaxIndex = xCooSys->getMaximumAxisIndexByDimension(nDimensionX);
                for(sal_Int32 nI=0; nI<=nMaxIndex; ++nI)
                {
                    Reference< XAxis > xAxis( xCooSys->getAxisByDimension(nDimensionX,nI) );
                    if( xAxis.is())
                    {
                        ScaleData aData( xAxis->getScaleData() );
                        aData.Categories = xCategories;
                        if(bSupportsCategories)
                        {

                            Reference< XChartType > xChartType( getChartTypeForNewSeries(Sequence< Reference< XChartType > >() ));
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
                    Reference< chart2::XAxis > xAxis( xCooSys->getAxisByDimension( 1,nI ));
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

void ChartTypeTemplate::adaptDiagram( const Reference< XDiagram > & /* xDiagram */ )
{
    }

void ChartTypeTemplate::createAxes(
    const Sequence< Reference< XCoordinateSystem > > & rCoordSys )
{
    //create missing axes
    if( rCoordSys.getLength() > 0 )
    {
        Reference< XCoordinateSystem > xCooSys( rCoordSys[0] );
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
}

void ChartTypeTemplate::adaptAxes(
    const Sequence< Reference< XCoordinateSystem > > & rCoordSys )
{
    //adapt properties of existing axes and remove superfluous axes

    if( rCoordSys.getLength() > 0 )
    {
        for( sal_Int32 nCooSysIdx=0; nCooSysIdx < rCoordSys.getLength(); ++nCooSysIdx )
        {
            Reference< XCoordinateSystem > xCooSys( rCoordSys[nCooSysIdx] );
            if( !xCooSys.is() )
                continue;
            sal_Int32 nDimCount = xCooSys->getDimension();
            for( sal_Int32 nDim=0; nDim<nDimCount; ++nDim )
            {
                sal_Int32 nMaxAxisIndex = xCooSys->getMaximumAxisIndexByDimension( nDim );
                for( sal_Int32 nAxisIndex=0; nAxisIndex<=nMaxAxisIndex; nAxisIndex++ )
                {
                    Reference< XAxis > xAxis( AxisHelper::getAxis( nDim, nAxisIndex, xCooSys ) );
                    if( !xAxis.is() )
                        continue;

                    if( nAxisIndex == MAIN_AXIS_INDEX || nAxisIndex == SECONDARY_AXIS_INDEX )
                    {
                        // adapt scales
                        bool bPercent = (getStackMode(0) == StackMode::YStackedPercent);
                        if( bPercent && nDim == 1 )
                        {
                            Reference< beans::XPropertySet > xAxisProp( xAxis, uno::UNO_QUERY );
                            if( xAxisProp.is())
                            {
                                // set number format to source format
                                xAxisProp->setPropertyValue(CHART_UNONAME_LINK_TO_SRC_NUMFMT, uno::Any(true));
                                xAxisProp->setPropertyValue(CHART_UNONAME_NUMFMT, uno::Any());
                            }
                        }
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
    const Reference< XDiagram >& xDiagram,
    const Sequence< Sequence< Reference< XDataSeries > > >& aSeriesSeq,
    const Reference< data::XLabeledDataSequence >& xCategories,
    const Sequence< Reference< XChartType > >& aOldChartTypesSeq )
{
    adaptDiagram( xDiagram );

    try
    {
        // create coordinate systems and scales
        Reference< XCoordinateSystemContainer > xCoordSysCnt( xDiagram, uno::UNO_QUERY_THROW );
        createCoordinateSystems( xCoordSysCnt );
        Sequence< Reference< XCoordinateSystem > > aCoordinateSystems( xCoordSysCnt->getCoordinateSystems());
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
    const Sequence< Sequence< Reference< XDataSeries > > > & aSeriesSeq,
    const Sequence< Reference< XCoordinateSystem > > & rCoordSys,
    const Sequence< Reference< XChartType > >& aOldChartTypesSeq )
{
    if( rCoordSys.getLength() == 0 ||
        ! rCoordSys[0].is() )
        return;

    try
    {
        sal_Int32 nCooSysIdx=0;
        Reference< XChartType > xCT;
        if( aSeriesSeq.getLength() == 0 )
        {
            // we need a new chart type
            xCT.set( getChartTypeForNewSeries( aOldChartTypesSeq ));
            Reference< XChartTypeContainer > xCTCnt( rCoordSys[nCooSysIdx], uno::UNO_QUERY_THROW );
            Sequence< Reference< XChartType > > aCTSeq( xCTCnt->getChartTypes());
            aCTSeq.realloc( 1 );
            aCTSeq[0] = xCT;
            xCTCnt->setChartTypes( aCTSeq );
        }
        else
        {
            for( sal_Int32 nSeriesIdx=0; nSeriesIdx<aSeriesSeq.getLength(); ++nSeriesIdx )
            {
                if( nSeriesIdx == nCooSysIdx )
                {
                    // we need a new chart type
                    xCT.set( getChartTypeForNewSeries( aOldChartTypesSeq ));
                    Reference< XChartTypeContainer > xCTCnt( rCoordSys[nCooSysIdx], uno::UNO_QUERY_THROW );
                    Sequence< Reference< XChartType > > aCTSeq( xCTCnt->getChartTypes());
                    if( aCTSeq.getLength())
                    {
                        aCTSeq[0] = xCT;
                        xCTCnt->setChartTypes( aCTSeq );
                    }
                    else
                        xCTCnt->addChartType( xCT );

                    Reference< chart2::XDataSeriesContainer > xDSCnt( xCT, uno::UNO_QUERY_THROW );
                    xDSCnt->setDataSeries( aSeriesSeq[nSeriesIdx] );
                }
                else
                {
                    // reuse existing chart type
                    OSL_ASSERT( xCT.is());
                    Reference< chart2::XDataSeriesContainer > xDSCnt( xCT, uno::UNO_QUERY_THROW );
                    Sequence< Reference< XDataSeries > > aNewSeriesSeq( xDSCnt->getDataSeries());
                    sal_Int32 nNewStartIndex = aNewSeriesSeq.getLength();
                    aNewSeriesSeq.realloc( nNewStartIndex + aSeriesSeq[nSeriesIdx].getLength() );
                    std::copy( aSeriesSeq[nSeriesIdx].begin(),
                                 aSeriesSeq[nSeriesIdx].end(),
                                 aNewSeriesSeq.getArray() + nNewStartIndex );
                    xDSCnt->setDataSeries( aNewSeriesSeq );
                }

                // spread the series over the available coordinate systems
                if( rCoordSys.getLength() > (nCooSysIdx + 1) )
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
                    const Sequence< Reference< XChartType > > & rOldChartTypesSeq,
                    const Reference< XChartType > & xNewChartType )
{
    Reference< beans::XPropertySet > xDestination( xNewChartType, uno::UNO_QUERY );
    if( !xDestination.is() )
        return;

    OUString aNewChartType( xNewChartType->getChartType() );

    Reference< beans::XPropertySet > xSource;
    sal_Int32 nN=0;
    for( nN=0; nN<rOldChartTypesSeq.getLength();++nN)
    {
        Reference< XChartType > xOldType( rOldChartTypesSeq[nN] );
        if( xOldType.is() && xOldType->getChartType() == aNewChartType )
        {
            xSource.set( Reference< beans::XPropertySet >(xOldType, uno::UNO_QUERY ) );
            if( xSource.is() )
                break;
        }
    }
    if( xSource.is() )
        comphelper::copyProperties( xSource, xDestination );
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
