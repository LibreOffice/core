/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ChartTypeHelper.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: vg $ $Date: 2007-09-18 15:07:49 $
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
#include "macros.hxx"
#include "servicenames_charttypes.hxx"

// header for define DBG_ASSERT
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

//.............................................................................
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;


//.............................................................................
namespace chart
{

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

        StackMode eStackMode = DiagramHelper::getStackModeFromChartType( xChartType, 0 );
        if( eStackMode != StackMode_Y_STACKED )
            return sal_False;

        rtl::OUString aChartTypeName = xChartType->getChartType();
        if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_COLUMN) )
            return sal_True;
        if( aChartTypeName.match(CHART2_SERVICE_NAME_CHARTTYPE_BAR) )
            return sal_True;  // note: old chart was false here
    }
    return sal_False;
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
//.............................................................................
} //namespace chart
//.............................................................................
