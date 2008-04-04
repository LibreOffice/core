/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ChartTypeHelper.cxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-04 11:00:32 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "ChartTypeHelper.hxx"
#include "DiagramHelper.hxx"
#include "DataSeriesHelper.hxx"
#include "macros.hxx"
#include "servicenames_charttypes.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/chart/DataLabelPlacement.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>

//.............................................................................
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

//.............................................................................
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
            rtl::OUString aChartTypeName = xChartType->getChartType();
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
            rtl::OUString aChartTypeName = xChartType->getChartType();
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

        rtl::OUString aChartTypeName = xChartType->getChartType();
        if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_PIE) )
            return sal_False;
        if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_NET) )
            return sal_False;
        if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_CANDLESTICK) )
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
            rtl::OUString aChartTypeName = xChartType->getChartType();
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

        rtl::OUString aChartTypeName = xChartType->getChartType();
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
        rtl::OUString aChartTypeName = xChartType->getChartType();
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

        rtl::OUString aChartTypeName = xChartType->getChartType();
        if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_PIE) )
            return sal_False;
        if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_NET) )
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

        rtl::OUString aChartTypeName = xChartType->getChartType();
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

        rtl::OUString aChartTypeName = xChartType->getChartType();
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

    rtl::OUString aChartTypeName = xChartType->getChartType();
    if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_PIE) )
    {
        bool bDonut = false;
        uno::Reference< beans::XPropertySet > xChartTypeProp( xChartType, uno::UNO_QUERY_THROW );
        if(xChartTypeProp.is())
            xChartTypeProp->getPropertyValue( C2U("UseRings")) >>= bDonut;

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
            xSeriesProp->getPropertyValue( C2U("StackingDirection") ) >>= eStacking;
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
        aRet.realloc(5);
        sal_Int32* pSeq = aRet.getArray();
        *pSeq++ = ::com::sun::star::chart::DataLabelPlacement::TOP;
        *pSeq++ = ::com::sun::star::chart::DataLabelPlacement::BOTTOM;
        *pSeq++ = ::com::sun::star::chart::DataLabelPlacement::LEFT;
        *pSeq++ = ::com::sun::star::chart::DataLabelPlacement::RIGHT;
        *pSeq++ = ::com::sun::star::chart::DataLabelPlacement::CENTER;
    }
    else if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_CANDLESTICK) )
    {
        aRet.realloc( 1 );
        sal_Int32* pSeq = aRet.getArray();
        *pSeq++ = ::com::sun::star::chart::DataLabelPlacement::OUTSIDE;
    }
    else
    {
        OSL_ENSURE( false, "unknown charttype" );
    }

    return aRet;
}

sal_Bool ChartTypeHelper::isSupportingRightAngledAxes( const uno::Reference< chart2::XChartType >& xChartType )
{
    if(xChartType.is())
    {
        rtl::OUString aChartTypeName = xChartType->getChartType();
        if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_PIE) )
            return sal_False;
    }
    return sal_True;
}

bool ChartTypeHelper::isSupportingStartingAngle( const uno::Reference< chart2::XChartType >& xChartType )
{
    if(xChartType.is())
    {
        rtl::OUString aChartTypeName = xChartType->getChartType();
        if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_PIE) )
            return true;
    }
    return false;
}

bool ChartTypeHelper::noBordersForSimpleScheme( const uno::Reference< chart2::XChartType >& xChartType )
{
    if(xChartType.is())
    {
        rtl::OUString aChartTypeName = xChartType->getChartType();
        if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_PIE) )
            return sal_True;
    }
    return sal_False;
}

//static
sal_Int32 ChartTypeHelper::getDefaultDirectLightColor( bool bSimple, const uno::Reference< chart2::XChartType >& xChartType )
{
    if( bSimple )
    {
        sal_Int32 nRet = static_cast< sal_Int32 >( 0x999999 ); // grey40
        if( xChartType .is() )
        {
            rtl::OUString aChartType = xChartType->getChartType();
            if( aChartType.equals(CHART2_SERVICE_NAME_CHARTTYPE_PIE) )
                nRet = static_cast< sal_Int32 >( 0x333333 ); // grey80
            else if( aChartType.equals(CHART2_SERVICE_NAME_CHARTTYPE_LINE)
                || aChartType.equals(CHART2_SERVICE_NAME_CHARTTYPE_SCATTER) )
                nRet = static_cast< sal_Int32 >( 0x666666 ); // grey60
        }
        return nRet;
    }
    return static_cast< sal_Int32 >( 0xb3b3b3 ); // grey30
}

//static
sal_Int32 ChartTypeHelper::getDefaultAmbientLightColor( bool bSimple, const uno::Reference< chart2::XChartType >& xChartType )
{
    if( bSimple )
    {
        sal_Int32 nRet = static_cast< sal_Int32 >( 0x999999 ); // grey40
        if( xChartType .is() )
        {
            rtl::OUString aChartType = xChartType->getChartType();
            if( aChartType.equals(CHART2_SERVICE_NAME_CHARTTYPE_PIE) )
                nRet = static_cast< sal_Int32 >( 0xcccccc ); // grey20
        }
        return nRet;
    }
    return static_cast< sal_Int32 >( 0x666666 ); // grey60
}

drawing::Direction3D ChartTypeHelper::getDefaultSimpleLightDirection( const uno::Reference< chart2::XChartType >& xChartType )
{
    //drawing::Direction3D aRet(0.0, 0.0, 1.0);
    drawing::Direction3D aRet(-0.2, 0.7, 0.6);
    if( xChartType .is() )
    {
        rtl::OUString aChartType = xChartType->getChartType();
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
    drawing::Direction3D aRet(-0.1, 0.6, 0.8);
    if( xChartType .is() )
    {
        rtl::OUString aChartType = xChartType->getChartType();
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

    rtl::OUString aChartTypeName = xChartType->getChartType();
    if(2==nDimensionIndex)//z-axis
        return AxisType::SERIES;
    if(1==nDimensionIndex)//y-axis
        return AxisType::REALNUMBER;
    if(0==nDimensionIndex)//x-axis
    {
        if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_SCATTER) )
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
            rtl::OUString aChartTypeName = xChartType->getChartType();
            if( aChartTypeName.equals(CHART2_SERVICE_NAME_CHARTTYPE_PIE))
            {
                uno::Reference< beans::XPropertySet > xChartTypeProp( xChartType, uno::UNO_QUERY_THROW );
                bool bDonut = false;
                if( (xChartTypeProp->getPropertyValue( C2U("UseRings")) >>= bDonut)
                    && !bDonut )
                {
                    return 1;
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

bool ChartTypeHelper::allSeriesAttachedToSameAxis(
    const uno::Reference< XChartType >& xChartType,
    sal_Int32 & rOutAxisIndex )
{
    try
    {
        uno::Reference< chart2::XDataSeriesContainer > xDataSeriesContainer( xChartType, uno::UNO_QUERY_THROW );
        uno::Sequence< uno::Reference< chart2::XDataSeries > > aSeriesSeq( xDataSeriesContainer->getDataSeries());

        const sal_Int32 nSeriesCount( aSeriesSeq.getLength());
        // AxisIndex can only be 0 or 1
        sal_Int32 nSeriesAtFirstAxis = 0;
        sal_Int32 nSeriesAtSecondAxis = 0;

        for( sal_Int32 nI = 0; nI < nSeriesCount; ++nI )
        {
            uno::Reference< chart2::XDataSeries > xSeries( aSeriesSeq[nI], uno::UNO_QUERY );
            sal_Int32 nAxisIndex = DataSeriesHelper::getAttachedAxisIndex( xSeries );
            if( nAxisIndex == 0 )
                ++nSeriesAtFirstAxis;
            else if( nAxisIndex == 1 )
                ++nSeriesAtSecondAxis;
        }
        OSL_ENSURE( nSeriesAtFirstAxis + nSeriesAtSecondAxis == nSeriesCount, "Invalid axis index found" );

        if( nSeriesAtFirstAxis == nSeriesCount )
            rOutAxisIndex = 0;
        else if( nSeriesAtSecondAxis == nSeriesCount )
            rOutAxisIndex = 1;

        return ( nSeriesAtFirstAxis == nSeriesCount ||
                 nSeriesAtSecondAxis == nSeriesCount );
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
        return false;
    }
}


//.............................................................................
} //namespace chart
//.............................................................................
