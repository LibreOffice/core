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

#include "ChartTypeHelper.hxx"
#include "DiagramHelper.hxx"
#include "DataSeriesHelper.hxx"
#include "macros.hxx"
#include "servicenames_charttypes.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/chart/DataLabelPlacement.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#include <com/sun/star/chart/MissingValueTreatment.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

namespace chart
{

bool ChartTypeHelper::isSupportingAxisSideBySide(
    const uno::Reference< chart2::XChartType >& xChartType, sal_Int32 nDimensionCount )
{
    bool bResult = false;

    if( xChartType.is() &&
        nDimensionCount < 3 )
    {
        bool bFound=false;
        bool bAmbiguous=false;
        StackMode eStackMode = DiagramHelper::getStackModeFromChartType( xChartType, bFound, bAmbiguous, 0 );
        if( eStackMode == StackMode_NONE && !bAmbiguous )
        {
            OUString aChartTypeName = xChartType->getChartType();
            bResult = ( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_COLUMN) ||
                        aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_BAR) );
        }
    }

    return bResult;
}

sal_Bool ChartTypeHelper::isSupportingGeometryProperties( const uno::Reference< XChartType >& xChartType, sal_Int32 nDimensionCount )
{
    //form tab only for 3D-bar and 3D-column charts.

    //@todo ask charttype itself --> need model change first
    if(xChartType.is())
    {
        if(nDimensionCount==3)
        {
            OUString aChartTypeName = xChartType->getChartType();
            if( aChartTypeName.equals(CHART2_SERVICE_NAME_CHARTTYPE_BAR) )
                return sal_True;
            if( aChartTypeName.equals(CHART2_SERVICE_NAME_CHARTTYPE_COLUMN) )
                return sal_True;
        }
    }
    return sal_False;
}

sal_Bool ChartTypeHelper::isSupportingStatisticProperties( const uno::Reference< XChartType >& xChartType, sal_Int32 nDimensionCount )
{
    //3D charts, pie, net and stock do not support statistic properties

    //@todo ask charttype itself (and series? --> stock chart?)  --> need model change first
    if(xChartType.is())
    {
        if(nDimensionCount==3)
            return sal_False;

        OUString aChartTypeName = xChartType->getChartType();
        if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_PIE) )
            return sal_False;
        if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_NET) )
            return sal_False;
        if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_FILLED_NET) )
            return sal_False;
        if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_CANDLESTICK) )
            return sal_False;
        if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_BUBBLE) ) //todo: BubbleChart support error bars and trend lines
            return sal_False;
    }
    return sal_True;
}

sal_Bool ChartTypeHelper::isSupportingRegressionProperties( const uno::Reference< XChartType >& xChartType, sal_Int32 nDimensionCount )
{
    // note: old chart: only scatter chart
    return isSupportingStatisticProperties( xChartType, nDimensionCount );
}

sal_Bool ChartTypeHelper::isSupportingAreaProperties( const uno::Reference< XChartType >& xChartType, sal_Int32 nDimensionCount )
{
    //2D line charts, net and stock do not support area properties

    //@todo ask charttype itself --> need model change first
    if(xChartType.is())
    {
         if(nDimensionCount==2)
        {
            OUString aChartTypeName = xChartType->getChartType();
            if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_LINE) )
                return sal_False;
            if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_SCATTER) )
                return sal_False;
            if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_NET) )
                return sal_False;
            if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_CANDLESTICK) )
                return sal_False;
        }
    }
    return sal_True;
}

sal_Bool ChartTypeHelper::isSupportingSymbolProperties( const uno::Reference< XChartType >& xChartType, sal_Int32 nDimensionCount )
{
    //2D line charts, 2D scatter charts and 2D net charts do support symbols

    //@todo ask charttype itself --> need model change first
    if(xChartType.is())
    {
        if(nDimensionCount==3)
            return sal_False;

        OUString aChartTypeName = xChartType->getChartType();
        if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_LINE) )
            return sal_True;
        if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_SCATTER) )
            return sal_True;
        if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_NET) )
            return sal_True;
    }
    return sal_False;
}

sal_Bool ChartTypeHelper::isSupportingMainAxis( const uno::Reference< XChartType >& xChartType, sal_Int32 nDimensionCount, sal_Int32 nDimensionIndex )
{
    //pie charts do not support axis at all
    //no 3rd axis for 2D charts

    //@todo ask charttype itself --> need model change first
    if(xChartType.is())
    {
        OUString aChartTypeName = xChartType->getChartType();
        if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_PIE) )
            return sal_False;

        if( nDimensionIndex == 2 )
            return nDimensionCount == 3;
    }
    return sal_True;
}

sal_Bool ChartTypeHelper::isSupportingSecondaryAxis( const uno::Reference< XChartType >& xChartType, sal_Int32 nDimensionCount, sal_Int32 /*nDimensionIndex*/ )
{
    //3D, pie and net charts do not support a secondary axis at all

    //@todo ask charttype itself --> need model change first
    if(xChartType.is())
    {
         if(nDimensionCount==3)
            return sal_False;

        OUString aChartTypeName = xChartType->getChartType();
        if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_PIE) )
            return sal_False;
        if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_NET) )
            return sal_False;
        if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_FILLED_NET) )
            return sal_False;
    }
    return sal_True;
}

sal_Bool ChartTypeHelper::isSupportingOverlapAndGapWidthProperties(
        const uno::Reference< XChartType >& xChartType, sal_Int32 nDimensionCount )
{
    //2D bar charts do support a this special properties

    //@todo ask charttype itself --> need model change first
    if(xChartType.is())
    {
         if(nDimensionCount==3)
            return sal_False;

        OUString aChartTypeName = xChartType->getChartType();
        if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_COLUMN) )
            return sal_True;
        if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_BAR) )
            return sal_True;
    }
    return sal_False;
}

sal_Bool ChartTypeHelper::isSupportingBarConnectors(
    const uno::Reference< chart2::XChartType >& xChartType, sal_Int32 nDimensionCount )
{
    //2D bar charts with stacked series support this

    //@todo ask charttype itself --> need model change first
    if(xChartType.is())
    {
        if(nDimensionCount==3)
            return sal_False;

        bool bFound=false;
        bool bAmbiguous=false;
        StackMode eStackMode = DiagramHelper::getStackModeFromChartType( xChartType, bFound, bAmbiguous, 0 );
        if( eStackMode != StackMode_Y_STACKED || bAmbiguous )
            return sal_False;

        OUString aChartTypeName = xChartType->getChartType();
        if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_COLUMN) )
            return sal_True;
        if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_BAR) )
            return sal_True;  // note: old chart was false here
    }
    return sal_False;
}

uno::Sequence < sal_Int32 > ChartTypeHelper::getSupportedLabelPlacements( const uno::Reference< chart2::XChartType >& xChartType
                                                                         , sal_Int32 nDimensionCount, sal_Bool bSwapXAndY
                                                                         , const uno::Reference< chart2::XDataSeries >& xSeries )
{
    (void)nDimensionCount;

    uno::Sequence < sal_Int32 > aRet;
    if( !xChartType.is() )
        return aRet;

    OUString aChartTypeName = xChartType->getChartType();
    if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_PIE) )
    {
        bool bDonut = false;
        uno::Reference< beans::XPropertySet > xChartTypeProp( xChartType, uno::UNO_QUERY_THROW );
        if(xChartTypeProp.is())
            xChartTypeProp->getPropertyValue( "UseRings") >>= bDonut;

        if(!bDonut)
        {
            aRet.realloc(4);
            sal_Int32* pSeq = aRet.getArray();
            *pSeq++ = ::com::sun::star::chart::DataLabelPlacement::AVOID_OVERLAP;
            *pSeq++ = ::com::sun::star::chart::DataLabelPlacement::OUTSIDE;
            *pSeq++ = ::com::sun::star::chart::DataLabelPlacement::INSIDE;
            *pSeq++ = ::com::sun::star::chart::DataLabelPlacement::CENTER;
        }
        else
        {
            aRet.realloc(1);
            sal_Int32* pSeq = aRet.getArray();
            *pSeq++ = ::com::sun::star::chart::DataLabelPlacement::CENTER;
        }
    }
    else if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_SCATTER)
        || aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_LINE)
        || aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_BUBBLE)
        )
    {
        aRet.realloc(5);
        sal_Int32* pSeq = aRet.getArray();
        *pSeq++ = ::com::sun::star::chart::DataLabelPlacement::TOP;
        *pSeq++ = ::com::sun::star::chart::DataLabelPlacement::BOTTOM;
        *pSeq++ = ::com::sun::star::chart::DataLabelPlacement::LEFT;
        *pSeq++ = ::com::sun::star::chart::DataLabelPlacement::RIGHT;
        *pSeq++ = ::com::sun::star::chart::DataLabelPlacement::CENTER;
    }
    else if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_COLUMN)
        || aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_BAR) )
    {

        bool bStacked = false;
        {
            uno::Reference< beans::XPropertySet > xSeriesProp( xSeries, uno::UNO_QUERY );
            chart2::StackingDirection eStacking = chart2::StackingDirection_NO_STACKING;
            xSeriesProp->getPropertyValue( "StackingDirection" ) >>= eStacking;
            bStacked = (chart2::StackingDirection_Y_STACKING == eStacking);
        }

        aRet.realloc( bStacked ? 3 : 6 );
        sal_Int32* pSeq = aRet.getArray();
        if(!bStacked)
        {
            if(bSwapXAndY)
            {
                *pSeq++ = ::com::sun::star::chart::DataLabelPlacement::RIGHT;
                *pSeq++ = ::com::sun::star::chart::DataLabelPlacement::LEFT;
            }
            else
            {
                *pSeq++ = ::com::sun::star::chart::DataLabelPlacement::TOP;
                *pSeq++ = ::com::sun::star::chart::DataLabelPlacement::BOTTOM;
            }
        }
        *pSeq++ = ::com::sun::star::chart::DataLabelPlacement::CENTER;
        if(!bStacked)
            *pSeq++ = ::com::sun::star::chart::DataLabelPlacement::OUTSIDE;
        *pSeq++ = ::com::sun::star::chart::DataLabelPlacement::INSIDE;
        *pSeq++ = ::com::sun::star::chart::DataLabelPlacement::NEAR_ORIGIN;
    }
    else if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_AREA) )
    {
        aRet.realloc(1);
        sal_Int32* pSeq = aRet.getArray();
        *pSeq++ = ::com::sun::star::chart::DataLabelPlacement::TOP;
    }
    else if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_NET) )
    {
        aRet.realloc(6);
        sal_Int32* pSeq = aRet.getArray();
        *pSeq++ = ::com::sun::star::chart::DataLabelPlacement::OUTSIDE;
        *pSeq++ = ::com::sun::star::chart::DataLabelPlacement::TOP;
        *pSeq++ = ::com::sun::star::chart::DataLabelPlacement::BOTTOM;
        *pSeq++ = ::com::sun::star::chart::DataLabelPlacement::LEFT;
        *pSeq++ = ::com::sun::star::chart::DataLabelPlacement::RIGHT;
        *pSeq++ = ::com::sun::star::chart::DataLabelPlacement::CENTER;
    }
    else if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_FILLED_NET) )
    {
        aRet.realloc(1);
        sal_Int32* pSeq = aRet.getArray();
        *pSeq++ = ::com::sun::star::chart::DataLabelPlacement::OUTSIDE;
    }
    else if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_CANDLESTICK) )
    {
        aRet.realloc( 1 );
        sal_Int32* pSeq = aRet.getArray();
        *pSeq++ = ::com::sun::star::chart::DataLabelPlacement::OUTSIDE;
    }
    else
    {
        OSL_FAIL( "unknown charttype" );
    }

    return aRet;
}

sal_Bool ChartTypeHelper::isSupportingRightAngledAxes( const uno::Reference< chart2::XChartType >& xChartType )
{
    if(xChartType.is())
    {
        OUString aChartTypeName = xChartType->getChartType();
        if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_PIE) )
            return sal_False;
    }
    return sal_True;
}

bool ChartTypeHelper::isSupportingStartingAngle( const uno::Reference< chart2::XChartType >& xChartType )
{
    if(xChartType.is())
    {
        OUString aChartTypeName = xChartType->getChartType();
        if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_PIE) )
            return true;
    }
    return false;
}
bool ChartTypeHelper::isSupportingBaseValue( const uno::Reference< chart2::XChartType >& xChartType )
{
    if(xChartType.is())
    {
        OUString aChartTypeName = xChartType->getChartType();
        if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_COLUMN)
            || aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_BAR)
            || aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_AREA)
            )
            return true;
    }
    return false;
}

bool ChartTypeHelper::isSupportingAxisPositioning( const uno::Reference< chart2::XChartType >& xChartType, sal_Int32 nDimensionCount, sal_Int32 nDimensionIndex )
{
    if(xChartType.is())
    {
        OUString aChartTypeName = xChartType->getChartType();
        if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_NET) )
            return false;
        if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_FILLED_NET) )
            return false;
    }
    if( nDimensionCount==3 )
        return nDimensionIndex<2;
    return true;
}

bool ChartTypeHelper::isSupportingDateAxis( const uno::Reference< chart2::XChartType >& xChartType, sal_Int32 /*nDimensionCount*/, sal_Int32 nDimensionIndex )
{
    if( nDimensionIndex!=0 )
        return false;
    if( xChartType.is() )
    {
        sal_Int32 nType = ChartTypeHelper::getAxisType( xChartType, nDimensionIndex );
        if( nType != AxisType::CATEGORY )
            return false;
        OUString aChartTypeName = xChartType->getChartType();
        if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_PIE) )
            return false;
        if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_NET) )
            return false;
        if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_FILLED_NET) )
            return false;
    }
    return true;
}

bool ChartTypeHelper::shiftCategoryPosAtXAxisPerDefault( const uno::Reference< chart2::XChartType >& xChartType )
{
    if(xChartType.is())
    {
        OUString aChartTypeName = xChartType->getChartType();
        if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_COLUMN)
            || aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_BAR)
            || aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_CANDLESTICK) )
            return true;
    }
    return false;
}

bool ChartTypeHelper::noBordersForSimpleScheme( const uno::Reference< chart2::XChartType >& xChartType )
{
    if(xChartType.is())
    {
        OUString aChartTypeName = xChartType->getChartType();
        if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_PIE) )
            return sal_True;
    }
    return sal_False;
}

sal_Int32 ChartTypeHelper::getDefaultDirectLightColor( bool bSimple, const uno::Reference< chart2::XChartType >& xChartType )
{
    sal_Int32 nRet = static_cast< sal_Int32 >( 0x808080 ); // grey
    if( xChartType .is() )
    {
        OUString aChartType = xChartType->getChartType();
        if( aChartType.equals(CHART2_SERVICE_NAME_CHARTTYPE_PIE) )
        {
            if( bSimple )
                nRet = static_cast< sal_Int32 >( 0x333333 ); // grey80
            else
                nRet = static_cast< sal_Int32 >( 0xb3b3b3 ); // grey30
        }
        else if( aChartType.equals(CHART2_SERVICE_NAME_CHARTTYPE_LINE)
            || aChartType.equals(CHART2_SERVICE_NAME_CHARTTYPE_SCATTER) )
            nRet = static_cast< sal_Int32 >( 0x666666 ); // grey60
    }
    return nRet;
}

sal_Int32 ChartTypeHelper::getDefaultAmbientLightColor( bool bSimple, const uno::Reference< chart2::XChartType >& xChartType )
{
    sal_Int32 nRet = static_cast< sal_Int32 >( 0x999999 ); // grey40
    if( xChartType .is() )
    {
        OUString aChartType = xChartType->getChartType();
        if( aChartType.equals(CHART2_SERVICE_NAME_CHARTTYPE_PIE) )
        {
            if( bSimple )
                nRet = static_cast< sal_Int32 >( 0xcccccc ); // grey20
            else
                nRet = static_cast< sal_Int32 >( 0x666666 ); // grey60
        }
    }
    return nRet;
}

drawing::Direction3D ChartTypeHelper::getDefaultSimpleLightDirection( const uno::Reference< chart2::XChartType >& xChartType )
{
    drawing::Direction3D aRet(0.0, 0.0, 1.0);
    if( xChartType .is() )
    {
        OUString aChartType = xChartType->getChartType();
        if( aChartType.equals(CHART2_SERVICE_NAME_CHARTTYPE_PIE) )
            aRet = drawing::Direction3D(0.0, 0.8, 0.5);
        else if( aChartType.equals(CHART2_SERVICE_NAME_CHARTTYPE_LINE)
            || aChartType.equals(CHART2_SERVICE_NAME_CHARTTYPE_SCATTER) )
            aRet = drawing::Direction3D(0.9, 0.5, 0.05);
    }
    return aRet;
}

drawing::Direction3D ChartTypeHelper::getDefaultRealisticLightDirection( const uno::Reference< chart2::XChartType >& xChartType )
{
    drawing::Direction3D aRet(0.0, 0.0, 1.0);
    if( xChartType .is() )
    {
        OUString aChartType = xChartType->getChartType();
        if( aChartType.equals(CHART2_SERVICE_NAME_CHARTTYPE_PIE) )
            aRet = drawing::Direction3D(0.6, 0.6, 0.6);
        else if( aChartType.equals(CHART2_SERVICE_NAME_CHARTTYPE_LINE)
            || aChartType.equals(CHART2_SERVICE_NAME_CHARTTYPE_SCATTER) )
            aRet = drawing::Direction3D(0.9, 0.5, 0.05);
    }
    return aRet;
}

sal_Int32 ChartTypeHelper::getAxisType( const uno::Reference<
            XChartType >& xChartType, sal_Int32 nDimensionIndex )
{
    //retruned is a constant from constant group ::com::sun::star::chart2::AxisType

    //@todo ask charttype itself --> need model change first
    if(!xChartType.is())
        return AxisType::CATEGORY;

    OUString aChartTypeName = xChartType->getChartType();
    if(2==nDimensionIndex)//z-axis
        return AxisType::SERIES;
    if(1==nDimensionIndex)//y-axis
        return AxisType::REALNUMBER;
    if(0==nDimensionIndex)//x-axis
    {
        if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_SCATTER)
         || aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_BUBBLE) )
            return AxisType::REALNUMBER;
        return AxisType::CATEGORY;
    }
    return AxisType::CATEGORY;
}

sal_Int32 ChartTypeHelper::getNumberOfDisplayedSeries(
    const uno::Reference< XChartType >& xChartType,
    sal_Int32 nNumberOfSeries )
{
    if( xChartType.is() )
    {
        try
        {
            OUString aChartTypeName = xChartType->getChartType();
            if( aChartTypeName.equals(CHART2_SERVICE_NAME_CHARTTYPE_PIE))
            {
                uno::Reference< beans::XPropertySet > xChartTypeProp( xChartType, uno::UNO_QUERY_THROW );
                bool bDonut = false;
                if( (xChartTypeProp->getPropertyValue( "UseRings") >>= bDonut)
                    && !bDonut )
                {
                    return nNumberOfSeries>0 ? 1 : 0;
                }
            }
        }
        catch( const uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }
    return nNumberOfSeries;
}

uno::Sequence < sal_Int32 > ChartTypeHelper::getSupportedMissingValueTreatments( const uno::Reference< XChartType >& xChartType )
{
    uno::Sequence < sal_Int32 > aRet;
    if( !xChartType.is() )
        return aRet;

    bool bFound=false;
    bool bAmbiguous=false;
    StackMode eStackMode = DiagramHelper::getStackModeFromChartType( xChartType, bFound, bAmbiguous, 0 );
    bool bStacked = bFound && (StackMode_Y_STACKED == eStackMode);

    OUString aChartTypeName = xChartType->getChartType();
    if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_COLUMN) ||
        aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_BAR) ||
        aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_BUBBLE) )
    {
        aRet.realloc( 2 );
        sal_Int32* pSeq = aRet.getArray();
        *pSeq++ = ::com::sun::star::chart::MissingValueTreatment::LEAVE_GAP;
        *pSeq++ = ::com::sun::star::chart::MissingValueTreatment::USE_ZERO;
    }
    else if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_AREA) )
    {
        aRet.realloc( bStacked ? 1 : 2 );
        sal_Int32* pSeq = aRet.getArray();
        *pSeq++ = ::com::sun::star::chart::MissingValueTreatment::USE_ZERO;
        if( !bStacked )
            *pSeq++ = ::com::sun::star::chart::MissingValueTreatment::CONTINUE;
    }
    else if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_LINE) ||
        aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_NET) ||
        aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_FILLED_NET) )
    {
        aRet.realloc( bStacked ? 2 : 3 );
        sal_Int32* pSeq = aRet.getArray();
        *pSeq++ = ::com::sun::star::chart::MissingValueTreatment::LEAVE_GAP;
        *pSeq++ = ::com::sun::star::chart::MissingValueTreatment::USE_ZERO;
        if( !bStacked )
            *pSeq++ = ::com::sun::star::chart::MissingValueTreatment::CONTINUE;
    }
    else if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_SCATTER) )
    {
        aRet.realloc( 3 );
        sal_Int32* pSeq = aRet.getArray();
        *pSeq++ = ::com::sun::star::chart::MissingValueTreatment::CONTINUE;
        *pSeq++ = ::com::sun::star::chart::MissingValueTreatment::LEAVE_GAP;
        *pSeq++ = ::com::sun::star::chart::MissingValueTreatment::USE_ZERO;
    }
    else if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_PIE) ||
        aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_CANDLESTICK) )
    {
        aRet.realloc( 0 );
    }
    else
    {
        OSL_FAIL( "unknown charttype" );
    }

    return aRet;
}

bool ChartTypeHelper::isSeriesInFrontOfAxisLine( const uno::Reference< XChartType >& xChartType )
{
    if( xChartType.is() )
    {
        OUString aChartTypeName = xChartType->getChartType();
        if( aChartTypeName.match( CHART2_SERVICE_NAME_CHARTTYPE_FILLED_NET ) )
            return false;
    }
    return true;
}

OUString ChartTypeHelper::getRoleOfSequenceForYAxisNumberFormatDetection( const uno::Reference< XChartType >& xChartType )
{
    OUString aRet( "values-y" );
    if( !xChartType.is() )
        return aRet;
    OUString aChartTypeName = xChartType->getChartType();
    if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_CANDLESTICK) )
        aRet = xChartType->getRoleOfSequenceForSeriesLabel();
    return aRet;
}

OUString ChartTypeHelper::getRoleOfSequenceForDataLabelNumberFormatDetection( const uno::Reference< XChartType >& xChartType )
{
    OUString aRet( "values-y" );
    if( !xChartType.is() )
        return aRet;
    OUString aChartTypeName = xChartType->getChartType();
    if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_CANDLESTICK)
        || aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_BUBBLE) )
        aRet = xChartType->getRoleOfSequenceForSeriesLabel();
    return aRet;
}

bool ChartTypeHelper::shouldLabelNumberFormatKeyBeDetectedFromYAxis( const uno::Reference< XChartType >& xChartType )
{
    bool bRet = true;
    OUString aChartTypeName = xChartType->getChartType();
    if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_BUBBLE) )
        bRet = false;
    return bRet;
}

bool ChartTypeHelper::isSupportingOnlyDeepStackingFor3D( const uno::Reference< XChartType >& xChartType )
{
    bool bRet = false;
    if( !xChartType.is() )
        return bRet;

    OUString aChartTypeName = xChartType->getChartType();
    if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_LINE) ||
        aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_SCATTER) ||
        aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_AREA) )
    {
        bRet = true;
    }
    return bRet;
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
