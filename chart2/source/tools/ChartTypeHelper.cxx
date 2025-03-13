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

#include <ChartTypeHelper.hxx>
#include <ChartType.hxx>
#include <BaseCoordinateSystem.hxx>
#include <DataSeriesProperties.hxx>
#include <DiagramHelper.hxx>
#include <servicenames_charttypes.hxx>

#include <com/sun/star/chart/DataLabelPlacement.hpp>
#include <com/sun/star/chart/MissingValueTreatment.hpp>
#include <com/sun/star/chart2/AxisType.hpp>
#include <com/sun/star/chart2/StackingDirection.hpp>
#include <comphelper/diagnose_ex.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using namespace ::chart::DataSeriesProperties;

namespace chart
{

uno::Sequence < sal_Int32 > ChartTypeHelper::getSupportedLabelPlacements( const rtl::Reference< ChartType >& xChartType
                                                                         , bool bSwapXAndY
                                                                         , const rtl::Reference< DataSeries >& xSeries )
{
    uno::Sequence < sal_Int32 > aRet;
    if( !xChartType.is() )
        return aRet;

    OUString aChartTypeName = xChartType->getChartType();
    if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_PIE) )
    {
        bool bDonut = false;
        xChartType->getFastPropertyValue( PROP_PIECHARTTYPE_USE_RINGS ) >>= bDonut; // "UseRings"

        if(!bDonut)
        {
            aRet.realloc(5);
            sal_Int32* pSeq = aRet.getArray();
            *pSeq++ = css::chart::DataLabelPlacement::AVOID_OVERLAP;
            *pSeq++ = css::chart::DataLabelPlacement::OUTSIDE;
            *pSeq++ = css::chart::DataLabelPlacement::INSIDE;
            *pSeq++ = css::chart::DataLabelPlacement::CENTER;
            *pSeq++ = css::chart::DataLabelPlacement::CUSTOM;
        }
        else
        {
            aRet.realloc(1);
            sal_Int32* pSeq = aRet.getArray();
            *pSeq++ = css::chart::DataLabelPlacement::CENTER;
        }
    }
    else if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_SCATTER)
        || aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_LINE)
        || aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_BUBBLE)
        || aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_FUNNEL)   // TODO: check this
        )
    {
        aRet.realloc(5);
        sal_Int32* pSeq = aRet.getArray();
        *pSeq++ = css::chart::DataLabelPlacement::TOP;
        *pSeq++ = css::chart::DataLabelPlacement::BOTTOM;
        *pSeq++ = css::chart::DataLabelPlacement::LEFT;
        *pSeq++ = css::chart::DataLabelPlacement::RIGHT;
        *pSeq++ = css::chart::DataLabelPlacement::CENTER;
    }
    else if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_COLUMN)
        || aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_BAR) )
    {

        bool bStacked = false;
        {
            chart2::StackingDirection eStacking = chart2::StackingDirection_NO_STACKING;
            xSeries->getFastPropertyValue( PROP_DATASERIES_STACKING_DIRECTION ) >>= eStacking; // "StackingDirection"
            bStacked = (eStacking == chart2::StackingDirection_Y_STACKING);
        }

        aRet.realloc( bStacked ? 3 : 6 );
        sal_Int32* pSeq = aRet.getArray();
        if(!bStacked)
        {
            if(bSwapXAndY)
            {
                *pSeq++ = css::chart::DataLabelPlacement::RIGHT;
                *pSeq++ = css::chart::DataLabelPlacement::LEFT;
            }
            else
            {
                *pSeq++ = css::chart::DataLabelPlacement::TOP;
                *pSeq++ = css::chart::DataLabelPlacement::BOTTOM;
            }
        }
        *pSeq++ = css::chart::DataLabelPlacement::CENTER;
        if(!bStacked)
            *pSeq++ = css::chart::DataLabelPlacement::OUTSIDE;
        *pSeq++ = css::chart::DataLabelPlacement::INSIDE;
        *pSeq++ = css::chart::DataLabelPlacement::NEAR_ORIGIN;
    }
    else if (aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_HISTOGRAM))
    {
        aRet.realloc(6);
        sal_Int32* pSeq = aRet.getArray();

        *pSeq++ = css::chart::DataLabelPlacement::TOP;
        *pSeq++ = css::chart::DataLabelPlacement::BOTTOM;
        *pSeq++ = css::chart::DataLabelPlacement::CENTER;
        *pSeq++ = css::chart::DataLabelPlacement::OUTSIDE;
        *pSeq++ = css::chart::DataLabelPlacement::INSIDE;
        *pSeq++ = css::chart::DataLabelPlacement::NEAR_ORIGIN;
    }
    else if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_AREA) )
    {
        bool bStacked = false;
        {
            chart2::StackingDirection eStacking = chart2::StackingDirection_NO_STACKING;
            xSeries->getFastPropertyValue(PROP_DATASERIES_STACKING_DIRECTION) >>= eStacking; // "StackingDirection"
            bStacked = (eStacking == chart2::StackingDirection_Y_STACKING);
        }

        aRet.realloc(2);
        sal_Int32* pSeq = aRet.getArray();
        if (bStacked)
        {
            *pSeq++ = css::chart::DataLabelPlacement::CENTER;
            *pSeq++ = css::chart::DataLabelPlacement::TOP;
        }
        else
        {
            *pSeq++ = css::chart::DataLabelPlacement::TOP;
            *pSeq++ = css::chart::DataLabelPlacement::CENTER;
        }
    }
    else if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_NET) )
    {
        aRet.realloc(6);
        sal_Int32* pSeq = aRet.getArray();
        *pSeq++ = css::chart::DataLabelPlacement::OUTSIDE;
        *pSeq++ = css::chart::DataLabelPlacement::TOP;
        *pSeq++ = css::chart::DataLabelPlacement::BOTTOM;
        *pSeq++ = css::chart::DataLabelPlacement::LEFT;
        *pSeq++ = css::chart::DataLabelPlacement::RIGHT;
        *pSeq++ = css::chart::DataLabelPlacement::CENTER;
    }
    else if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_FILLED_NET) )
    {
        aRet.realloc(1);
        sal_Int32* pSeq = aRet.getArray();
        *pSeq++ = css::chart::DataLabelPlacement::OUTSIDE;
    }
    else if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_CANDLESTICK) )
    {
        aRet.realloc( 1 );
        sal_Int32* pSeq = aRet.getArray();
        *pSeq++ = css::chart::DataLabelPlacement::OUTSIDE;
    }
    else
    {
        OSL_FAIL( "unknown charttype" );
    }

    return aRet;
}

bool ChartTypeHelper::shiftCategoryPosAtXAxisPerDefault( const rtl::Reference< ChartType >& xChartType )
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

bool ChartTypeHelper::noBordersForSimpleScheme( const rtl::Reference< ChartType >& xChartType )
{
    if(xChartType.is())
    {
        OUString aChartTypeName = xChartType->getChartType();
        if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_PIE) )
            return true;
    }
    return false;
}

sal_Int32 ChartTypeHelper::getDefaultDirectLightColor( bool bSimple, const rtl::Reference< ChartType >& xChartType )
{
    sal_Int32 nRet = static_cast< sal_Int32 >( 0x808080 ); // grey
    if( xChartType .is() )
    {
        OUString aChartType = xChartType->getChartType();
        if( aChartType == CHART2_SERVICE_NAME_CHARTTYPE_PIE )
        {
            if( bSimple )
                nRet = static_cast< sal_Int32 >( 0x333333 ); // grey80
            else
                nRet = static_cast< sal_Int32 >( 0xb3b3b3 ); // grey30
        }
        else if( aChartType == CHART2_SERVICE_NAME_CHARTTYPE_LINE
            || aChartType == CHART2_SERVICE_NAME_CHARTTYPE_SCATTER )
            nRet = static_cast< sal_Int32 >( 0x666666 ); // grey60
    }
    return nRet;
}

sal_Int32 ChartTypeHelper::getDefaultAmbientLightColor( bool bSimple, const rtl::Reference< ChartType >& xChartType )
{
    sal_Int32 nRet = static_cast< sal_Int32 >( 0x999999 ); // grey40
    if( xChartType .is() )
    {
        OUString aChartType = xChartType->getChartType();
        if( aChartType == CHART2_SERVICE_NAME_CHARTTYPE_PIE )
        {
            if( bSimple )
                nRet = static_cast< sal_Int32 >( 0xcccccc ); // grey20
            else
                nRet = static_cast< sal_Int32 >( 0x666666 ); // grey60
        }
    }
    return nRet;
}

drawing::Direction3D ChartTypeHelper::getDefaultSimpleLightDirection( const rtl::Reference< ChartType >& xChartType )
{
    drawing::Direction3D aRet(0.0, 0.0, 1.0);
    if( xChartType .is() )
    {
        OUString aChartType = xChartType->getChartType();
        if( aChartType == CHART2_SERVICE_NAME_CHARTTYPE_PIE )
            aRet = drawing::Direction3D(0.0, 0.8, 0.5);
        else if( aChartType == CHART2_SERVICE_NAME_CHARTTYPE_LINE
            || aChartType == CHART2_SERVICE_NAME_CHARTTYPE_SCATTER )
            aRet = drawing::Direction3D(0.9, 0.5, 0.05);
    }
    return aRet;
}

drawing::Direction3D ChartTypeHelper::getDefaultRealisticLightDirection( const rtl::Reference< ChartType >& xChartType )
{
    drawing::Direction3D aRet(0.0, 0.0, 1.0);
    if( xChartType .is() )
    {
        OUString aChartType = xChartType->getChartType();
        if( aChartType == CHART2_SERVICE_NAME_CHARTTYPE_PIE )
            aRet = drawing::Direction3D(0.6, 0.6, 0.6);
        else if( aChartType == CHART2_SERVICE_NAME_CHARTTYPE_LINE
            || aChartType == CHART2_SERVICE_NAME_CHARTTYPE_SCATTER )
            aRet = drawing::Direction3D(0.9, 0.5, 0.05);
    }
    return aRet;
}

sal_Int32 ChartTypeHelper::getNumberOfDisplayedSeries(
    const rtl::Reference< ChartType >& xChartType,
    sal_Int32 nNumberOfSeries )
{
    if( xChartType.is() )
    {
        try
        {
            OUString aChartTypeName = xChartType->getChartType();
            if( aChartTypeName == CHART2_SERVICE_NAME_CHARTTYPE_PIE )
            {
                bool bDonut = false;
                if( (xChartType->getFastPropertyValue( PROP_PIECHARTTYPE_USE_RINGS ) >>= bDonut) // "UseRings"
                    && !bDonut )
                {
                    return nNumberOfSeries>0 ? 1 : 0;
                }
            }
        }
        catch( const uno::Exception & )
        {
            DBG_UNHANDLED_EXCEPTION("chart2");
        }
    }
    return nNumberOfSeries;
}

uno::Sequence < sal_Int32 > ChartTypeHelper::getSupportedMissingValueTreatments( const rtl::Reference< ChartType >& xChartType )
{
    uno::Sequence < sal_Int32 > aRet;
    if( !xChartType.is() )
        return aRet;

    bool bFound=false;
    bool bAmbiguous=false;
    StackMode eStackMode = DiagramHelper::getStackModeFromChartType( xChartType, bFound, bAmbiguous, nullptr );
    bool bStacked = bFound && (eStackMode == StackMode::YStacked);

    OUString aChartTypeName = xChartType->getChartType();
    if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_COLUMN) ||
        aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_BAR) ||
        aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_FUNNEL) ||
        aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_BUBBLE) )
    {
        aRet.realloc( 2 );
        sal_Int32* pSeq = aRet.getArray();
        *pSeq++ = css::chart::MissingValueTreatment::LEAVE_GAP;
        *pSeq++ = css::chart::MissingValueTreatment::USE_ZERO;
    }
    else if (aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_HISTOGRAM))
    {
        // Assuming histograms typically use zero for missing values
        aRet.realloc(1);
        sal_Int32* pSeq = aRet.getArray();
        *pSeq++ = css::chart::MissingValueTreatment::USE_ZERO;
    }
    else if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_AREA) )
    {
        aRet.realloc( bStacked ? 1 : 2 );
        sal_Int32* pSeq = aRet.getArray();
        *pSeq++ = css::chart::MissingValueTreatment::USE_ZERO;
        if( !bStacked )
            *pSeq++ = css::chart::MissingValueTreatment::CONTINUE;
    }
    else if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_LINE) ||
        aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_NET) ||
        aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_FILLED_NET) )
    {
        aRet.realloc( bStacked ? 2 : 3 );
        sal_Int32* pSeq = aRet.getArray();
        *pSeq++ = css::chart::MissingValueTreatment::LEAVE_GAP;
        *pSeq++ = css::chart::MissingValueTreatment::USE_ZERO;
        if( !bStacked )
            *pSeq++ = css::chart::MissingValueTreatment::CONTINUE;
    }
    else if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_SCATTER) )
    {
        aRet.realloc( 3 );
        sal_Int32* pSeq = aRet.getArray();
        *pSeq++ = css::chart::MissingValueTreatment::CONTINUE;
        *pSeq++ = css::chart::MissingValueTreatment::LEAVE_GAP;
        *pSeq++ = css::chart::MissingValueTreatment::USE_ZERO;
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

OUString ChartTypeHelper::getRoleOfSequenceForYAxisNumberFormatDetection( const rtl::Reference< ChartType >& xChartType )
{
    OUString aRet( u"values-y"_ustr );
    if( !xChartType.is() )
        return aRet;
    OUString aChartTypeName = xChartType->getChartType();
    if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_CANDLESTICK) )
        aRet = xChartType->getRoleOfSequenceForSeriesLabel();
    return aRet;
}

OUString ChartTypeHelper::getRoleOfSequenceForDataLabelNumberFormatDetection( const rtl::Reference< ChartType >& xChartType )
{
    OUString aRet( u"values-y"_ustr );
    if( !xChartType.is() )
        return aRet;
    OUString aChartTypeName = xChartType->getChartType();
    if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_CANDLESTICK)
        || aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_BUBBLE) )
        aRet = xChartType->getRoleOfSequenceForSeriesLabel();
    return aRet;
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
