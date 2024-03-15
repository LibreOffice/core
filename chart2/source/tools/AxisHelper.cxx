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

#include <AxisHelper.hxx>
#include <DiagramHelper.hxx>
#include <Diagram.hxx>
#include <ChartTypeHelper.hxx>
#include <ChartType.hxx>
#include <Axis.hxx>
#include <AxisIndexDefines.hxx>
#include <DataSource.hxx>
#include <LinePropertiesHelper.hxx>
#include <servicenames_coosystems.hxx>
#include <DataSeries.hxx>
#include <DataSeriesHelper.hxx>
#include <Scaling.hxx>
#include <ChartModel.hxx>
#include <ChartModelHelper.hxx>
#include <DataSourceHelper.hxx>
#include <ReferenceSizeProvider.hxx>
#include <ExplicitCategoriesProvider.hxx>
#include <unonames.hxx>
#include <BaseCoordinateSystem.hxx>
#include <GridProperties.hxx>

#include <o3tl/safeint.hxx>
#include <unotools/saveopt.hxx>

#include <com/sun/star/chart/ChartAxisPosition.hpp>
#include <com/sun/star/chart2/AxisType.hpp>

#include <sal/log.hxx>

#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <comphelper/diagnose_ex.hxx>

#include <cstddef>
#include <map>

namespace chart
{
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

Reference< chart2::XScaling > AxisHelper::createLinearScaling()
{
    return new LinearScaling( 1.0, 0.0 );
}

Reference< chart2::XScaling > AxisHelper::createLogarithmicScaling( double fBase )
{
    return new LogarithmicScaling( fBase );
}

ScaleData AxisHelper::createDefaultScale()
{
    ScaleData aScaleData;
    aScaleData.AxisType = chart2::AxisType::REALNUMBER;
    aScaleData.AutoDateAxis = true;
    aScaleData.ShiftedCategoryPosition = false;
    aScaleData.IncrementData.SubIncrements = { SubIncrement() };
    return aScaleData;
}

void AxisHelper::removeExplicitScaling( ScaleData& rScaleData )
{
    rScaleData.Minimum = rScaleData.Maximum = rScaleData.Origin = uno::Any();
    rScaleData.Scaling = nullptr;
    ScaleData aDefaultScale( createDefaultScale() );
    rScaleData.IncrementData = aDefaultScale.IncrementData;
    rScaleData.TimeIncrement = aDefaultScale.TimeIncrement;
}

bool AxisHelper::isLogarithmic( const Reference< XScaling >& xScaling )
{
    Reference< lang::XServiceName > xServiceName( xScaling, uno::UNO_QUERY );
    return xServiceName.is()
        && xServiceName->getServiceName() == "com.sun.star.chart2.LogarithmicScaling";
}

chart2::ScaleData AxisHelper::getDateCheckedScale( const rtl::Reference< Axis >& xAxis, ChartModel& rModel )
{
    ScaleData aScale = xAxis->getScaleData();
    rtl::Reference< BaseCoordinateSystem > xCooSys( ChartModelHelper::getFirstCoordinateSystem( &rModel ) );
    if( aScale.AutoDateAxis && aScale.AxisType == AxisType::CATEGORY )
    {
        sal_Int32 nDimensionIndex=0; sal_Int32 nAxisIndex=0;
        AxisHelper::getIndicesForAxis(xAxis, xCooSys, nDimensionIndex, nAxisIndex );
        bool bChartTypeAllowsDateAxis = ChartTypeHelper::isSupportingDateAxis( AxisHelper::getChartTypeByIndex( xCooSys, 0 ), nDimensionIndex );
        if( bChartTypeAllowsDateAxis )
            aScale.AxisType = AxisType::DATE;
    }
    if( aScale.AxisType == AxisType::DATE )
    {
        ExplicitCategoriesProvider aExplicitCategoriesProvider( xCooSys, rModel );
        if( !aExplicitCategoriesProvider.isDateAxis() )
            aScale.AxisType = AxisType::CATEGORY;
    }
    return aScale;
}

void AxisHelper::checkDateAxis( chart2::ScaleData& rScale, ExplicitCategoriesProvider* pExplicitCategoriesProvider, bool bChartTypeAllowsDateAxis )
{
    if( rScale.AutoDateAxis && rScale.AxisType == AxisType::CATEGORY && bChartTypeAllowsDateAxis )
    {
        rScale.AxisType = AxisType::DATE;
        removeExplicitScaling( rScale );
    }
    if( rScale.AxisType == AxisType::DATE && (!pExplicitCategoriesProvider || !pExplicitCategoriesProvider->isDateAxis()) )
    {
        rScale.AxisType = AxisType::CATEGORY;
        removeExplicitScaling( rScale );
    }
}

sal_Int32 AxisHelper::getExplicitNumberFormatKeyForAxis(
                  const rtl::Reference< Axis >& xAxis
                , const rtl::Reference< BaseCoordinateSystem > & xCorrespondingCoordinateSystem
                , const rtl::Reference<ChartModel>& xChartDoc
                , bool bSearchForParallelAxisIfNothingIsFound )
{
    sal_Int32 nNumberFormatKey(0);
    sal_Int32 nAxisIndex = 0;
    sal_Int32 nDimensionIndex = 1;
    AxisHelper::getIndicesForAxis( xAxis, xCorrespondingCoordinateSystem, nDimensionIndex, nAxisIndex );

    if (!xAxis.is())
        return 0;

    bool bLinkToSource = true;
    xAxis->getPropertyValue(CHART_UNONAME_LINK_TO_SRC_NUMFMT) >>= bLinkToSource;
    xAxis->getPropertyValue(CHART_UNONAME_NUMFMT) >>= nNumberFormatKey;

    if (bLinkToSource)
    {
        bool bFormatSet = false;
        //check whether we have a percent scale -> use percent format
        if (xChartDoc)
        {
            ScaleData aData = AxisHelper::getDateCheckedScale( xAxis, *xChartDoc );
            if( aData.AxisType==AxisType::PERCENT )
            {
                sal_Int32 nPercentFormat = DiagramHelper::getPercentNumberFormat( xChartDoc );
                if( nPercentFormat != -1 )
                {
                    nNumberFormatKey = nPercentFormat;
                    bFormatSet = true;
                }
            }
            else if( aData.AxisType==AxisType::DATE )
            {
                if( aData.Categories.is() )
                {
                    Reference< data::XDataSequence > xSeq( aData.Categories->getValues());
                    if( xSeq.is() && !( xChartDoc.is() && xChartDoc->hasInternalDataProvider()) )
                        nNumberFormatKey = xSeq->getNumberFormatKeyByIndex( -1 );
                    else
                        nNumberFormatKey = DiagramHelper::getDateNumberFormat( xChartDoc );
                    bFormatSet = true;
                }
            }
            else if( xChartDoc.is() && xChartDoc->hasInternalDataProvider() && nDimensionIndex == 0 ) //maybe date axis
            {
                rtl::Reference< Diagram > xDiagram( xChartDoc->getFirstChartDiagram() );
                if( xDiagram->isSupportingDateAxis() )
                {
                    nNumberFormatKey = DiagramHelper::getDateNumberFormat( xChartDoc );
                }
                else
                {
                    rtl::Reference< DataSource > xSource = DataSourceHelper::getUsedData( *xChartDoc );
                    if( xSource.is() )
                    {
                        std::vector< uno::Reference< chart2::data::XLabeledDataSequence > > aXValues(
                            DataSeriesHelper::getAllDataSequencesByRole( xSource->getDataSequences(), "values-x" ) );
                        if( aXValues.empty() )
                        {
                            uno::Reference< chart2::data::XLabeledDataSequence > xCategories( xDiagram->getCategories() );
                            if( xCategories.is() )
                            {
                                Reference< data::XDataSequence > xSeq( xCategories->getValues());
                                if( xSeq.is() )
                                {
                                    bool bHasValidDoubles = false;
                                    double fTest=0.0;
                                    Sequence< uno::Any > aCats( xSeq->getData() );
                                    sal_Int32 nCount = aCats.getLength();
                                    for( sal_Int32 i = 0; i < nCount; ++i )
                                    {
                                        if( (aCats[i]>>=fTest) && !std::isnan(fTest) )
                                        {
                                            bHasValidDoubles=true;
                                            break;
                                        }
                                    }
                                    if( bHasValidDoubles )
                                        nNumberFormatKey = DiagramHelper::getDateNumberFormat( xChartDoc );
                                }
                            }
                        }
                    }
                }
                bFormatSet = true;
            }
        }

        if( !bFormatSet )
        {
            std::map< sal_Int32, sal_Int32 > aKeyMap;
            bool bNumberFormatKeyFoundViaAttachedData = false;

            try
            {
                OUString aRoleToMatch;
                if( nDimensionIndex == 0 )
                    aRoleToMatch = "values-x";
                const std::vector< rtl::Reference< ChartType > > & aChartTypes( xCorrespondingCoordinateSystem->getChartTypes2());
                for( rtl::Reference< ChartType > const & chartType : aChartTypes )
                {
                    if( nDimensionIndex != 0 )
                        aRoleToMatch = ChartTypeHelper::getRoleOfSequenceForYAxisNumberFormatDetection( chartType );
                    for( rtl::Reference< DataSeries > const & xDataSeries : chartType->getDataSeries2() )
                    {
                        if( nDimensionIndex == 1 )
                        {
                            //only take those series into account that are attached to this axis
                            sal_Int32 nAttachedAxisIndex = DataSeriesHelper::getAttachedAxisIndex(xDataSeries);
                            if( nAttachedAxisIndex != nAxisIndex )
                                continue;
                        }

                        Reference< data::XLabeledDataSequence > xLabeledSeq(
                            DataSeriesHelper::getDataSequenceByRole( xDataSeries, aRoleToMatch ) );

                        if( !xLabeledSeq.is() && nDimensionIndex==0 )
                        {
                            ScaleData aData = xAxis->getScaleData();
                            xLabeledSeq = aData.Categories;
                        }

                        if( xLabeledSeq.is() )
                        {
                            Reference< data::XDataSequence > xSeq( xLabeledSeq->getValues());
                            if( xSeq.is() )
                            {
                                sal_Int32 nKey = xSeq->getNumberFormatKeyByIndex( -1 );
                                // increase frequency
                                aKeyMap[ nKey ] ++;
                            }
                        }
                    }
                }
            }
            catch( const uno::Exception & )
            {
                DBG_UNHANDLED_EXCEPTION("chart2");
            }

            if( ! aKeyMap.empty())
            {
                sal_Int32 nMaxFreq = 0;
                // find most frequent key
                for (auto const& elem : aKeyMap)
                {
                    SAL_INFO(
                        "chart2.tools",
                        "NumberFormatKey " << elem.first << " appears "
                            << elem.second << " times");
                    // all values must at least be 1
                    if( elem.second > nMaxFreq )
                    {
                        nNumberFormatKey = elem.first;
                        bNumberFormatKeyFoundViaAttachedData = true;
                        nMaxFreq = elem.second;
                    }
                }
            }

            if( bSearchForParallelAxisIfNothingIsFound )
            {
                //no format is set to this axis and no data is set to this axis
                //--> try to obtain the format from the parallel y-axis
                if( !bNumberFormatKeyFoundViaAttachedData && nDimensionIndex == 1 )
                {
                    sal_Int32 nParallelAxisIndex = (nAxisIndex==1) ?0 :1;
                    rtl::Reference< Axis > xParallelAxis = AxisHelper::getAxis( 1, nParallelAxisIndex, xCorrespondingCoordinateSystem );
                    nNumberFormatKey = AxisHelper::getExplicitNumberFormatKeyForAxis(xParallelAxis, xCorrespondingCoordinateSystem, xChartDoc, false);
                }
            }
        }
    }

    return nNumberFormatKey;
}

rtl::Reference< Axis > AxisHelper::createAxis(
          sal_Int32 nDimensionIndex
        , sal_Int32 nAxisIndex // 0==main or 1==secondary axis
        , const rtl::Reference< BaseCoordinateSystem >& xCooSys
        , const Reference< uno::XComponentContext > & xContext
        , ReferenceSizeProvider * pRefSizeProvider )
{
    if( !xContext.is() || !xCooSys.is() )
        return nullptr;
    if( nDimensionIndex >= xCooSys->getDimension() )
        return nullptr;

    rtl::Reference< Axis > xAxis = new Axis();

    xCooSys->setAxisByDimension( nDimensionIndex, xAxis, nAxisIndex );

    if( nAxisIndex>0 )//when inserting secondary axes copy some things from the main axis
    {
        css::chart::ChartAxisPosition eNewAxisPos( css::chart::ChartAxisPosition_END );

        rtl::Reference< Axis > xMainAxis = xCooSys->getAxisByDimension2( nDimensionIndex, 0 );
        if( xMainAxis.is() )
        {
            ScaleData aScale = xAxis->getScaleData();
            ScaleData aMainScale = xMainAxis->getScaleData();

            aScale.AxisType = aMainScale.AxisType;
            aScale.AutoDateAxis = aMainScale.AutoDateAxis;
            aScale.Categories = aMainScale.Categories;
            aScale.Orientation = aMainScale.Orientation;
            aScale.ShiftedCategoryPosition = aMainScale.ShiftedCategoryPosition;

            xAxis->setScaleData( aScale );

            //ensure that the second axis is not placed on the main axis
            css::chart::ChartAxisPosition eMainAxisPos( css::chart::ChartAxisPosition_ZERO );
            xMainAxis->getPropertyValue("CrossoverPosition") >>= eMainAxisPos;
            if( eMainAxisPos == css::chart::ChartAxisPosition_END )
                eNewAxisPos = css::chart::ChartAxisPosition_START;
        }

        xAxis->setPropertyValue("CrossoverPosition", uno::Any(eNewAxisPos) );
    }

    try
    {
        // set correct initial AutoScale
        if( pRefSizeProvider )
            pRefSizeProvider->setValuesAtPropertySet( xAxis );
    }
    catch( const uno::Exception& )
    {
        TOOLS_WARN_EXCEPTION("chart2", "" );
    }
    return xAxis;
}

rtl::Reference< Axis > AxisHelper::createAxis( sal_Int32 nDimensionIndex, bool bMainAxis
                , const rtl::Reference< Diagram >& xDiagram
                , const Reference< uno::XComponentContext >& xContext
                , ReferenceSizeProvider * pRefSizeProvider )
{
    OSL_ENSURE( xContext.is(), "need a context to create an axis" );
    if( !xContext.is() )
        return nullptr;

    sal_Int32 nAxisIndex = bMainAxis ? MAIN_AXIS_INDEX : SECONDARY_AXIS_INDEX;
    rtl::Reference< BaseCoordinateSystem > xCooSys = AxisHelper::getCoordinateSystemByIndex( xDiagram, 0 );

    // create axis
    return AxisHelper::createAxis(
        nDimensionIndex, nAxisIndex, xCooSys, xContext, pRefSizeProvider );
}

void AxisHelper::showAxis( sal_Int32 nDimensionIndex, bool bMainAxis
                , const rtl::Reference< Diagram >& xDiagram
                , const Reference< uno::XComponentContext >& xContext
                , ReferenceSizeProvider * pRefSizeProvider )
{
    if( !xDiagram.is() )
        return;

    bool bNewAxisCreated = false;
    rtl::Reference< Axis > xAxis = AxisHelper::getAxis( nDimensionIndex, bMainAxis, xDiagram );
    if( !xAxis.is() && xContext.is() )
    {
        // create axis
        bNewAxisCreated = true;
        xAxis = AxisHelper::createAxis( nDimensionIndex, bMainAxis, xDiagram, xContext, pRefSizeProvider );
    }

    OSL_ASSERT( xAxis.is());
    if( !bNewAxisCreated ) //default is true already if created
        AxisHelper::makeAxisVisible( xAxis );
}

void AxisHelper::showGrid( sal_Int32 nDimensionIndex, sal_Int32 nCooSysIndex, bool bMainGrid
                , const rtl::Reference< Diagram >& xDiagram )
{
    if( !xDiagram.is() )
        return;

    rtl::Reference< BaseCoordinateSystem > xCooSys = AxisHelper::getCoordinateSystemByIndex( xDiagram, nCooSysIndex );
    if(!xCooSys.is())
        return;

    rtl::Reference< Axis > xAxis = AxisHelper::getAxis( nDimensionIndex, MAIN_AXIS_INDEX, xCooSys );
    if(!xAxis.is())
    {
        //hhhh todo create axis without axis visibility
    }
    if(!xAxis.is())
        return;

    if( bMainGrid )
        AxisHelper::makeGridVisible( xAxis->getGridProperties2() );
    else
    {
        std::vector< rtl::Reference< GridProperties > > aSubGrids( xAxis->getSubGridProperties2() );
        for( auto const & i : aSubGrids )
            AxisHelper::makeGridVisible( i );
    }
}

void AxisHelper::makeAxisVisible( const rtl::Reference< Axis >& xAxis )
{
    if( xAxis.is() )
    {
        xAxis->setPropertyValue( "Show", uno::Any( true ) );
        LinePropertiesHelper::SetLineVisible( xAxis );
        xAxis->setPropertyValue( "DisplayLabels", uno::Any( true ) );
    }
}

void AxisHelper::makeGridVisible( const rtl::Reference< GridProperties >& xGridProperties )
{
    if( xGridProperties.is() )
    {
        xGridProperties->setPropertyValue( "Show", uno::Any( true ) );
        LinePropertiesHelper::SetLineVisible( xGridProperties );
    }
}

void AxisHelper::hideAxis( sal_Int32 nDimensionIndex, bool bMainAxis
                , const rtl::Reference< Diagram >& xDiagram )
{
    AxisHelper::makeAxisInvisible( AxisHelper::getAxis( nDimensionIndex, bMainAxis, xDiagram ) );
}

void AxisHelper::makeAxisInvisible( const rtl::Reference< Axis >& xAxis )
{
    if( xAxis.is() )
    {
        xAxis->setPropertyValue( "Show", uno::Any( false ) );
    }
}

void AxisHelper::hideAxisIfNoDataIsAttached( const rtl::Reference< Axis >& xAxis, const rtl::Reference< Diagram >& xDiagram )
{
    //axis is hidden if no data is attached anymore but data is available
    bool bOtherSeriesAttachedToThisAxis = false;
    std::vector< rtl::Reference< DataSeries > > aSeriesVector = xDiagram->getDataSeries();
    for (auto const& series : aSeriesVector)
    {
        rtl::Reference< Axis > xCurrentAxis = xDiagram->getAttachedAxis(series);
        if( xCurrentAxis==xAxis )
        {
            bOtherSeriesAttachedToThisAxis = true;
            break;
        }
    }
    if(!bOtherSeriesAttachedToThisAxis && !aSeriesVector.empty() )
        AxisHelper::makeAxisInvisible( xAxis );
}

void AxisHelper::hideGrid( sal_Int32 nDimensionIndex, sal_Int32 nCooSysIndex, bool bMainGrid
                , const rtl::Reference< Diagram >& xDiagram )
{
    if( !xDiagram.is() )
        return;

    rtl::Reference< BaseCoordinateSystem > xCooSys = AxisHelper::getCoordinateSystemByIndex( xDiagram, nCooSysIndex );
    if(!xCooSys.is())
        return;

    rtl::Reference< Axis > xAxis = AxisHelper::getAxis( nDimensionIndex, MAIN_AXIS_INDEX, xCooSys );
    if(!xAxis.is())
        return;

    if( bMainGrid )
        AxisHelper::makeGridInvisible( xAxis->getGridProperties2() );
    else
    {
        std::vector< rtl::Reference< ::chart::GridProperties > > aSubGrids( xAxis->getSubGridProperties2() );
        for( auto const & i : aSubGrids)
            AxisHelper::makeGridInvisible( i );
    }
}

void AxisHelper::makeGridInvisible( const rtl::Reference< ::chart::GridProperties >& xGridProperties )
{
    if( xGridProperties.is() )
    {
        xGridProperties->setPropertyValue( "Show", uno::Any( false ) );
    }
}

bool AxisHelper::isGridShown( sal_Int32 nDimensionIndex, sal_Int32 nCooSysIndex, bool bMainGrid
                , const rtl::Reference< Diagram >& xDiagram )
{
    bool bRet = false;

    rtl::Reference< BaseCoordinateSystem > xCooSys = AxisHelper::getCoordinateSystemByIndex( xDiagram, nCooSysIndex );
    if(!xCooSys.is())
        return bRet;

    rtl::Reference< Axis > xAxis = AxisHelper::getAxis( nDimensionIndex, MAIN_AXIS_INDEX, xCooSys );
    if(!xAxis.is())
        return bRet;

    if( bMainGrid )
        bRet = AxisHelper::isGridVisible( xAxis->getGridProperties2() );
    else
    {
        std::vector< rtl::Reference< ::chart::GridProperties > > aSubGrids( xAxis->getSubGridProperties2() );
        if( !aSubGrids.empty() )
            bRet = AxisHelper::isGridVisible( aSubGrids[0] );
    }

    return bRet;
}

rtl::Reference< ::chart::BaseCoordinateSystem > AxisHelper::getCoordinateSystemByIndex(
    const rtl::Reference< Diagram >& xDiagram, sal_Int32 nIndex )
{
    if(!xDiagram.is())
        return nullptr;
    auto aCooSysList = xDiagram->getBaseCoordinateSystems();
    if(0<=nIndex && o3tl::make_unsigned(nIndex) < aCooSysList.size())
        return aCooSysList[nIndex];
    return nullptr;
}

rtl::Reference< Axis > AxisHelper::getAxis( sal_Int32 nDimensionIndex, bool bMainAxis
            , const rtl::Reference< Diagram >& xDiagram )
{
    rtl::Reference< Axis > xRet;
    try
    {
        rtl::Reference< BaseCoordinateSystem > xCooSys = AxisHelper::getCoordinateSystemByIndex( xDiagram, 0 );
        xRet = AxisHelper::getAxis( nDimensionIndex, bMainAxis ? 0 : 1, xCooSys );
    }
    catch( const uno::Exception & )
    {
    }
    return xRet;
}

rtl::Reference< Axis > AxisHelper::getAxis( sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex
            , const rtl::Reference< BaseCoordinateSystem >& xCooSys )
{
    rtl::Reference< Axis > xRet;
    if(!xCooSys.is())
        return xRet;

    if(nDimensionIndex >= xCooSys->getDimension())
        return xRet;

    if(nAxisIndex > xCooSys->getMaximumAxisIndexByDimension(nDimensionIndex))
        return xRet;

    assert(nAxisIndex >= 0);
    assert(nDimensionIndex >= 0);
    xRet = xCooSys->getAxisByDimension2( nDimensionIndex, nAxisIndex );
    return xRet;
}

rtl::Reference< Axis > AxisHelper::getCrossingMainAxis( const rtl::Reference< Axis >& xAxis
            , const rtl::Reference< BaseCoordinateSystem >& xCooSys )
{
    sal_Int32 nDimensionIndex = 0;
    sal_Int32 nAxisIndex = 0;
    AxisHelper::getIndicesForAxis( xAxis, xCooSys, nDimensionIndex, nAxisIndex );
    if( nDimensionIndex==2 )
    {
        nDimensionIndex=1;
        bool bSwapXY = false;
        if( (xCooSys->getPropertyValue( "SwapXAndYAxis" ) >>= bSwapXY) && bSwapXY )
            nDimensionIndex=0;
    }
    else if( nDimensionIndex==1 )
        nDimensionIndex=0;
    else
        nDimensionIndex=1;
    return AxisHelper::getAxis( nDimensionIndex, 0, xCooSys );
}

rtl::Reference< Axis > AxisHelper::getParallelAxis( const rtl::Reference< Axis >& xAxis
            , const rtl::Reference< Diagram >& xDiagram )
{
    try
    {
        sal_Int32 nCooSysIndex=-1;
        sal_Int32 nDimensionIndex=-1;
        sal_Int32 nAxisIndex=-1;
        if( getIndicesForAxis( xAxis, xDiagram, nCooSysIndex, nDimensionIndex, nAxisIndex ) )
        {
            sal_Int32 nParallelAxisIndex = (nAxisIndex==1) ?0 :1;
            return getAxis( nDimensionIndex, nParallelAxisIndex, getCoordinateSystemByIndex( xDiagram, nCooSysIndex ) );
        }
    }
    catch( const uno::RuntimeException& )
    {
    }
    return nullptr;
}

bool AxisHelper::isAxisShown( sal_Int32 nDimensionIndex, bool bMainAxis
            , const rtl::Reference< Diagram >& xDiagram )
{
    return AxisHelper::isAxisVisible( AxisHelper::getAxis( nDimensionIndex, bMainAxis, xDiagram ) );
}

bool AxisHelper::isAxisVisible( const rtl::Reference< Axis >& xAxis )
{
    bool bRet = false;

    if( xAxis.is() )
    {
        xAxis->getPropertyValue( "Show" ) >>= bRet;
        bRet = bRet && ( LinePropertiesHelper::IsLineVisible( xAxis )
            || areAxisLabelsVisible( xAxis ) );
    }

    return bRet;
}

bool AxisHelper::areAxisLabelsVisible( const rtl::Reference< Axis >& xAxis )
{
    bool bRet = false;
    if( xAxis.is() )
    {
        xAxis->getPropertyValue( "DisplayLabels" ) >>= bRet;
    }
    return bRet;
}

bool AxisHelper::isGridVisible( const rtl::Reference< ::chart::GridProperties >& xGridproperties )
{
    bool bRet = false;

    if( xGridproperties.is() )
    {
        xGridproperties->getPropertyValue( "Show" ) >>= bRet;
        bRet = bRet && LinePropertiesHelper::IsLineVisible( xGridproperties );
    }

    return bRet;
}

rtl::Reference< GridProperties > AxisHelper::getGridProperties(
            const rtl::Reference< BaseCoordinateSystem >& xCooSys
        , sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex, sal_Int32 nSubGridIndex )
{
    rtl::Reference< GridProperties > xRet;

    rtl::Reference< Axis > xAxis( AxisHelper::getAxis( nDimensionIndex, nAxisIndex, xCooSys ) );
    if( xAxis.is() )
    {
        if( nSubGridIndex<0 )
            xRet = xAxis->getGridProperties2();
        else
        {
            std::vector< rtl::Reference< GridProperties > > aSubGrids( xAxis->getSubGridProperties2() );
            if (nSubGridIndex < static_cast<sal_Int32>(aSubGrids.size()))
                xRet = aSubGrids[nSubGridIndex];
        }
    }

    return xRet;
}

sal_Int32 AxisHelper::getDimensionIndexOfAxis(
              const rtl::Reference< Axis >& xAxis
            , const rtl::Reference< Diagram >& xDiagram )
{
    sal_Int32 nDimensionIndex = -1;
    sal_Int32 nCooSysIndex = -1;
    sal_Int32 nAxisIndex = -1;
    AxisHelper::getIndicesForAxis( xAxis, xDiagram, nCooSysIndex , nDimensionIndex, nAxisIndex );
    return nDimensionIndex;
}

bool AxisHelper::getIndicesForAxis(
              const rtl::Reference< Axis >& xAxis
            , const rtl::Reference< BaseCoordinateSystem >& xCooSys
            , sal_Int32& rOutDimensionIndex, sal_Int32& rOutAxisIndex )
{
    //returns true if indices are found

    rOutDimensionIndex = -1;
    rOutAxisIndex = -1;

    if( !xCooSys || !xAxis )
        return false;

    rtl::Reference< Axis > xCurrentAxis;
    sal_Int32 nDimensionCount( xCooSys->getDimension() );
    for( sal_Int32 nDimensionIndex = 0; nDimensionIndex < nDimensionCount; nDimensionIndex++ )
    {
        sal_Int32 nMaxAxisIndex = xCooSys->getMaximumAxisIndexByDimension(nDimensionIndex);
        for( sal_Int32 nAxisIndex = 0; nAxisIndex <= nMaxAxisIndex; nAxisIndex++ )
        {
             xCurrentAxis = xCooSys->getAxisByDimension2(nDimensionIndex,nAxisIndex);
             if( xCurrentAxis == xAxis )
             {
                 rOutDimensionIndex = nDimensionIndex;
                 rOutAxisIndex = nAxisIndex;
                 return true;
             }
        }
    }
    return false;
}

bool AxisHelper::getIndicesForAxis( const rtl::Reference< Axis >& xAxis, const rtl::Reference< Diagram >& xDiagram
            , sal_Int32& rOutCooSysIndex, sal_Int32& rOutDimensionIndex, sal_Int32& rOutAxisIndex )
{
    //returns true if indices are found

    rOutCooSysIndex = -1;
    rOutDimensionIndex = -1;
    rOutAxisIndex = -1;

    const std::vector< rtl::Reference< BaseCoordinateSystem > > & aCooSysList = xDiagram->getBaseCoordinateSystems();
    for( std::size_t nC=0; nC < aCooSysList.size(); ++nC )
    {
        if( AxisHelper::getIndicesForAxis( xAxis, aCooSysList[nC], rOutDimensionIndex, rOutAxisIndex ) )
        {
            rOutCooSysIndex = nC;
            return true;
        }
    }

    return false;
}

std::vector< rtl::Reference< Axis > > AxisHelper::getAllAxesOfCoordinateSystem(
      const rtl::Reference< BaseCoordinateSystem >& xCooSys
    , bool bOnlyVisible /* = false */ )
{
    std::vector< rtl::Reference< Axis > > aAxisVector;

    if(xCooSys.is())
    {
        sal_Int32 nMaxDimensionIndex = xCooSys->getDimension() -1;
        if( nMaxDimensionIndex>=0 )
        {
            sal_Int32 nDimensionIndex = 0;
            for(; nDimensionIndex<=nMaxDimensionIndex; ++nDimensionIndex)
            {
                const sal_Int32 nMaximumAxisIndex = xCooSys->getMaximumAxisIndexByDimension(nDimensionIndex);
                for(sal_Int32 nAxisIndex=0; nAxisIndex<=nMaximumAxisIndex; ++nAxisIndex)
                {
                    try
                    {
                        rtl::Reference< Axis > xAxis = xCooSys->getAxisByDimension2( nDimensionIndex, nAxisIndex );
                        if( xAxis.is() )
                        {
                            bool bAddAxis = true;
                            if( bOnlyVisible )
                            {
                                if( !(xAxis->getPropertyValue( "Show") >>= bAddAxis) )
                                    bAddAxis = false;
                            }
                            if( bAddAxis )
                                aAxisVector.push_back( xAxis );
                        }
                    }
                    catch( const uno::Exception & )
                    {
                        DBG_UNHANDLED_EXCEPTION("chart2");
                    }
                }
            }
        }
    }

    return aAxisVector;
}

std::vector< rtl::Reference< Axis > > AxisHelper::getAllAxesOfDiagram(
      const rtl::Reference< Diagram >& xDiagram
    , bool bOnlyVisible )
{
    std::vector< rtl::Reference< Axis > > aAxisVector;

    for( rtl::Reference< BaseCoordinateSystem > const & coords : xDiagram->getBaseCoordinateSystems() )
    {
        std::vector< rtl::Reference< Axis > > aAxesPerCooSys = AxisHelper::getAllAxesOfCoordinateSystem( coords, bOnlyVisible );
        aAxisVector.insert( aAxisVector.end(), aAxesPerCooSys.begin(), aAxesPerCooSys.end() );
    }

    return aAxisVector;
}

std::vector< rtl::Reference< GridProperties > > AxisHelper::getAllGrids( const rtl::Reference< Diagram >& xDiagram )
{
    const std::vector< rtl::Reference< Axis > > aAllAxes = AxisHelper::getAllAxesOfDiagram( xDiagram );
    std::vector< rtl::Reference< GridProperties > > aGridVector;

    for( rtl::Reference< Axis > const & xAxis : aAllAxes )
    {
        rtl::Reference< GridProperties > xGridProperties( xAxis->getGridProperties2() );
        if( xGridProperties.is() )
            aGridVector.push_back( xGridProperties );

        std::vector< rtl::Reference< GridProperties > > aSubGrids( xAxis->getSubGridProperties2() );
        for( rtl::Reference< GridProperties > const & xSubGrid : aSubGrids )
        {
            if( xSubGrid.is() )
                aGridVector.push_back( xSubGrid );
        }
    }

    return aGridVector;
}

void AxisHelper::getAxisOrGridPossibilities( Sequence< sal_Bool >& rPossibilityList
        , const rtl::Reference< Diagram>& xDiagram, bool bAxis )
{
    rPossibilityList.realloc(6);
    sal_Bool* pPossibilityList = rPossibilityList.getArray();

    sal_Int32 nDimensionCount = -1;
    if (xDiagram)
        nDimensionCount = xDiagram->getDimension();

    //set possibilities:
    sal_Int32 nIndex=0;
    rtl::Reference< ChartType > xChartType;
    if (xDiagram)
        xChartType = xDiagram->getChartTypeByIndex( 0 );
    for(nIndex=0;nIndex<3;nIndex++)
        pPossibilityList[nIndex]=ChartTypeHelper::isSupportingMainAxis(xChartType,nDimensionCount,nIndex);
    for(nIndex=3;nIndex<6;nIndex++)
        if( bAxis )
            pPossibilityList[nIndex]=ChartTypeHelper::isSupportingSecondaryAxis(xChartType,nDimensionCount);
        else
            pPossibilityList[nIndex] = rPossibilityList[nIndex-3];
}

bool AxisHelper::isSecondaryYAxisNeeded( const rtl::Reference< BaseCoordinateSystem >& xCooSys )
{
    if( !xCooSys.is() )
        return false;

    const std::vector< rtl::Reference< ChartType > > & aChartTypes( xCooSys->getChartTypes2() );
    for( rtl::Reference< ChartType > const & chartType : aChartTypes )
    {
        const std::vector< rtl::Reference< DataSeries > > & aSeriesList = chartType->getDataSeries2();
        for( sal_Int32 nS = aSeriesList.size(); nS-- ; )
        {
            sal_Int32 nAttachedAxisIndex = 0;
            if( ( aSeriesList[nS]->getPropertyValue( "AttachedAxisIndex" ) >>= nAttachedAxisIndex ) &&
                    nAttachedAxisIndex>0 )
                return true;
        }
    }
    return false;
}

bool AxisHelper::shouldAxisBeDisplayed( const rtl::Reference< Axis >& xAxis
                                       , const rtl::Reference< BaseCoordinateSystem >& xCooSys )
{
    bool bRet = false;

    if( xAxis.is() && xCooSys.is() )
    {
        sal_Int32 nDimensionIndex=-1;
        sal_Int32 nAxisIndex=-1;
        if( AxisHelper::getIndicesForAxis( xAxis, xCooSys, nDimensionIndex, nAxisIndex ) )
        {
            sal_Int32 nDimensionCount = xCooSys->getDimension();
            rtl::Reference< ChartType > xChartType( AxisHelper::getChartTypeByIndex( xCooSys, 0 ) );

            bool bMainAxis = (nAxisIndex==MAIN_AXIS_INDEX);
            if( bMainAxis )
                bRet = ChartTypeHelper::isSupportingMainAxis(xChartType,nDimensionCount,nDimensionIndex);
            else
                bRet = ChartTypeHelper::isSupportingSecondaryAxis(xChartType,nDimensionCount);
        }
    }

    return bRet;
}

void AxisHelper::getAxisOrGridExistence( Sequence< sal_Bool >& rExistenceList
        , const rtl::Reference< Diagram>& xDiagram, bool bAxis )
{
    rExistenceList.realloc(6);
    sal_Bool* pExistenceList = rExistenceList.getArray();

    if(bAxis)
    {
        sal_Int32 nN;
        for(nN=0;nN<3;nN++)
            pExistenceList[nN] = AxisHelper::isAxisShown( nN, true, xDiagram );
        for(nN=3;nN<6;nN++)
            pExistenceList[nN] = AxisHelper::isAxisShown( nN%3, false, xDiagram );
    }
    else
    {
        sal_Int32 nN;

        for(nN=0;nN<3;nN++)
            pExistenceList[nN] = AxisHelper::isGridShown( nN, 0, true, xDiagram );
        for(nN=3;nN<6;nN++)
            pExistenceList[nN] = AxisHelper::isGridShown( nN%3, 0, false, xDiagram );
    }
}

bool AxisHelper::changeVisibilityOfAxes( const rtl::Reference< Diagram >& xDiagram
                        , const Sequence< sal_Bool >& rOldExistenceList
                        , const Sequence< sal_Bool >& rNewExistenceList
                        , const Reference< uno::XComponentContext >& xContext
                        , ReferenceSizeProvider * pRefSizeProvider )
{
    bool bChanged = false;
    for(sal_Int32 nN=0;nN<6;nN++)
    {
        if(rOldExistenceList[nN]!=rNewExistenceList[nN])
        {
            bChanged = true;
            if(rNewExistenceList[nN])
            {
                AxisHelper::showAxis( nN%3, nN<3, xDiagram, xContext, pRefSizeProvider );
            }
            else
                AxisHelper::hideAxis( nN%3, nN<3, xDiagram );
        }
    }
    return bChanged;
}

bool AxisHelper::changeVisibilityOfGrids( const rtl::Reference< Diagram >& xDiagram
                        , const Sequence< sal_Bool >& rOldExistenceList
                        , const Sequence< sal_Bool >& rNewExistenceList )
{
    bool bChanged = false;
    for(sal_Int32 nN=0;nN<6;nN++)
    {
        if(rOldExistenceList[nN]!=rNewExistenceList[nN])
        {
            bChanged = true;
            if(rNewExistenceList[nN])
                AxisHelper::showGrid( nN%3, 0, nN<3, xDiagram );
            else
                AxisHelper::hideGrid( nN%3, 0, nN<3, xDiagram );
        }
    }
    return bChanged;
}

rtl::Reference< BaseCoordinateSystem > AxisHelper::getCoordinateSystemOfAxis(
              const rtl::Reference< Axis >& xAxis
            , const rtl::Reference< Diagram >& xDiagram )
{
    if (!xDiagram)
        return nullptr;

    rtl::Reference< BaseCoordinateSystem > xRet;
    for( rtl::Reference< BaseCoordinateSystem > const & xCooSys : xDiagram->getBaseCoordinateSystems() )
    {
        std::vector< rtl::Reference< Axis > > aAllAxis = AxisHelper::getAllAxesOfCoordinateSystem( xCooSys );

        auto aFound = std::find( aAllAxis.begin(), aAllAxis.end(), xAxis );
        if( aFound != aAllAxis.end())
        {
            xRet = xCooSys;
            break;
        }
    }
    return xRet;
}

rtl::Reference< ChartType > AxisHelper::getChartTypeByIndex( const rtl::Reference< BaseCoordinateSystem >& xCooSys, sal_Int32 nIndex )
{
    rtl::Reference< ChartType > xChartType;

    if( xCooSys.is() )
    {
        const std::vector< rtl::Reference< ChartType > > aChartTypeList( xCooSys->getChartTypes2() );
        if( nIndex >= 0 && o3tl::make_unsigned(nIndex) < aChartTypeList.size() )
            xChartType = aChartTypeList[nIndex];
    }

    return xChartType;
}

void AxisHelper::setRTLAxisLayout( const rtl::Reference< BaseCoordinateSystem >& xCooSys )
{
    if( !xCooSys.is() )
        return;

    bool bCartesian = xCooSys->getViewServiceName() == CHART2_COOSYSTEM_CARTESIAN_VIEW_SERVICE_NAME;
    if( !bCartesian )
        return;

    bool bVertical = false;
    xCooSys->getPropertyValue( "SwapXAndYAxis" ) >>= bVertical;

    sal_Int32 nHorizontalAxisDimension = bVertical ? 1 : 0;
    sal_Int32 nVerticalAxisDimension = bVertical ? 0 : 1;

    try
    {
        //reverse direction for horizontal main axis
        rtl::Reference< Axis > xHorizontalMainAxis = AxisHelper::getAxis( nHorizontalAxisDimension, MAIN_AXIS_INDEX, xCooSys );
        if( xHorizontalMainAxis.is() )
        {
            chart2::ScaleData aScale = xHorizontalMainAxis->getScaleData();
            aScale.Orientation = chart2::AxisOrientation_REVERSE;
            xHorizontalMainAxis->setScaleData(aScale);
        }

        //mathematical direction for vertical main axis
        rtl::Reference< Axis > xVerticalMainAxis = AxisHelper::getAxis( nVerticalAxisDimension, MAIN_AXIS_INDEX, xCooSys );
        if( xVerticalMainAxis.is() )
        {
            chart2::ScaleData aScale = xVerticalMainAxis->getScaleData();
            aScale.Orientation = chart2::AxisOrientation_MATHEMATICAL;
            xVerticalMainAxis->setScaleData(aScale);
        }
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2" );
    }

    try
    {
        //reverse direction for horizontal secondary axis
        rtl::Reference< Axis > xHorizontalSecondaryAxis = AxisHelper::getAxis( nHorizontalAxisDimension, SECONDARY_AXIS_INDEX, xCooSys );
        if( xHorizontalSecondaryAxis.is() )
        {
            chart2::ScaleData aScale = xHorizontalSecondaryAxis->getScaleData();
            aScale.Orientation = chart2::AxisOrientation_REVERSE;
            xHorizontalSecondaryAxis->setScaleData(aScale);
        }

        //mathematical direction for vertical secondary axis
        rtl::Reference< Axis > xVerticalSecondaryAxis = AxisHelper::getAxis( nVerticalAxisDimension, SECONDARY_AXIS_INDEX, xCooSys );
        if( xVerticalSecondaryAxis.is() )
        {
            chart2::ScaleData aScale = xVerticalSecondaryAxis->getScaleData();
            aScale.Orientation = chart2::AxisOrientation_MATHEMATICAL;
            xVerticalSecondaryAxis->setScaleData(aScale);
        }
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

rtl::Reference< ChartType > AxisHelper::getFirstChartTypeWithSeriesAttachedToAxisIndex( const rtl::Reference< Diagram >& xDiagram, const sal_Int32 nAttachedAxisIndex )
{
    rtl::Reference< ChartType > xChartType;
    std::vector< rtl::Reference< DataSeries > > aSeriesVector = xDiagram->getDataSeries();
    for (auto const& series : aSeriesVector)
    {
        sal_Int32 nCurrentIndex = DataSeriesHelper::getAttachedAxisIndex(series);
        if( nAttachedAxisIndex == nCurrentIndex )
        {
            xChartType = xDiagram->getChartTypeOfSeries(series);
            if(xChartType.is())
                break;
        }
    }
    return xChartType;
}

bool AxisHelper::isAxisPositioningEnabled()
{
    const SvtSaveOptions::ODFSaneDefaultVersion nCurrentVersion(GetODFSaneDefaultVersion());
    return nCurrentVersion >= SvtSaveOptions::ODFSVER_012;
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
