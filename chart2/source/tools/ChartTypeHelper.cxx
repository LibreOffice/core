/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ChartTypeHelper.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 01:27:12 $
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

#include "ChartTypeHelper.hxx"
#include "macros.hxx"

// header for define DBG_ASSERT
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

sal_Bool ChartTypeHelper::isSupportingGeometryProperties( const uno::Reference< XChartType >& xChartType )
{
    //form tab only for 3D-bar and 3D-column charts.

    //@todo ask charttype itself --> need model change first
    if(xChartType.is())
    {
        sal_Int32 nDimension=2;
        {
            uno::Reference< beans::XPropertySet > xChartTypeProp( xChartType, uno::UNO_QUERY );
            if(xChartTypeProp.is())
                xChartTypeProp->getPropertyValue( C2U( "Dimension" )) >>= nDimension;
        }
        if(nDimension==3)
        {
            rtl::OUString aChartTypeName = xChartType->getChartType();
            if( aChartTypeName.equalsIgnoreAsciiCase(C2U("com.sun.star.chart2.BarChart")) )
                return sal_True;
            if( aChartTypeName.equalsIgnoreAsciiCase(C2U("com.sun.star.chart2.ColumnChart")) )
                return sal_True;
        }
    }
    return sal_False;
}

sal_Bool ChartTypeHelper::isSupportingStatisticProperties( const uno::Reference< XChartType >& xChartType )
{
    //3D charts, pie, net and stock do not support statistic properties

    //@todo ask charttype itself (and series? --> stock chart?)  --> need model change first
    if(xChartType.is())
    {
        sal_Int32 nDimension=2;
        {
            uno::Reference< beans::XPropertySet > xChartTypeProp( xChartType, uno::UNO_QUERY );
            if(xChartTypeProp.is())
                xChartTypeProp->getPropertyValue( C2U( "Dimension" )) >>= nDimension;
        }
        if(nDimension==3)
            return sal_False;

        rtl::OUString aChartTypeName = xChartType->getChartType();
        if( aChartTypeName.match(C2U("com.sun.star.chart2.PieChart")) )
            return sal_False;
        if( aChartTypeName.match(C2U("com.sun.star.chart2.Net")) )
            return sal_False;
        if( aChartTypeName.match(C2U("com.sun.star.chart2.Stock")) )
            return sal_False;
    }
    return sal_True;
}

sal_Bool ChartTypeHelper::isSupportingRegressionProperties( const uno::Reference< XChartType >& xChartType )
{
    // note: old chart: only scatter chart
    return isSupportingStatisticProperties( xChartType );
}

sal_Bool ChartTypeHelper::isSupportingAreaProperties( const uno::Reference< XChartType >& xChartType )
{
    //2D line charts do not support area properties

    //@todo ask charttype itself --> need model change first
    if(xChartType.is())
    {
        sal_Int32 nDimension=2;
        {
            uno::Reference< beans::XPropertySet > xChartTypeProp( xChartType, uno::UNO_QUERY );
            if(xChartTypeProp.is())
                xChartTypeProp->getPropertyValue( C2U( "Dimension" )) >>= nDimension;
        }
        if(nDimension==2)
        {
            rtl::OUString aChartTypeName = xChartType->getChartType();
            if( aChartTypeName.match(C2U("com.sun.star.chart2.LineChart")) )
                return sal_False;
            if( aChartTypeName.match(C2U("com.sun.star.chart2.ScatterChart")) )
                return sal_False;
        }
    }
    return sal_True;
}

sal_Bool ChartTypeHelper::isSupportingSymbolProperties( const uno::Reference< XChartType >& xChartType )
{
    //2D line charts, 2D scatter charts and 2D net charts do support symbols

    //@todo ask charttype itself --> need model change first
    if(xChartType.is())
    {
        sal_Int32 nDimension=2;
        {
            uno::Reference< beans::XPropertySet > xChartTypeProp( xChartType, uno::UNO_QUERY );
            if(xChartTypeProp.is())
                xChartTypeProp->getPropertyValue( C2U( "Dimension" )) >>= nDimension;
        }
        if(nDimension==3)
            return sal_False;

        rtl::OUString aChartTypeName = xChartType->getChartType();
        if( aChartTypeName.match(C2U("com.sun.star.chart2.LineChart")) )
            return sal_True;
        if( aChartTypeName.match(C2U("com.sun.star.chart2.ScatterChart")) )
            return sal_True;
        if( aChartTypeName.match(C2U("com.sun.star.chart2.Net")) )
            return sal_True;
    }
    return sal_False;
}

sal_Bool ChartTypeHelper::isSupportingMainAxis( const uno::Reference< XChartType >& xChartType, sal_Int32 nDimensionIndex )
{
    //pie charts do not support axis at all
    //no 3rd axis for 2D charts

    //@todo ask charttype itself --> need model change first
    if(xChartType.is())
    {
        rtl::OUString aChartTypeName = xChartType->getChartType();
        if( aChartTypeName.match(C2U("com.sun.star.chart2.PieChart")) )
            return sal_False;

        if( nDimensionIndex == 2 )
            return ChartTypeHelper::getDimensionCount( xChartType ) == 3;
    }
    return sal_True;
}

sal_Bool ChartTypeHelper::isSupportingSecondaryAxis( const uno::Reference< XChartType >& xChartType, sal_Int32 nDimensionIndex )
{
    //3D, pie and net charts do not support a secondary axis at all

    //@todo ask charttype itself --> need model change first
    if(xChartType.is())
    {
        sal_Int32 nDimension=2;
        {
            uno::Reference< beans::XPropertySet > xChartTypeProp( xChartType, uno::UNO_QUERY );
            if(xChartTypeProp.is())
                xChartTypeProp->getPropertyValue( C2U( "Dimension" )) >>= nDimension;
        }
        if(nDimension==3)
            return sal_False;

        rtl::OUString aChartTypeName = xChartType->getChartType();
        if( aChartTypeName.match(C2U("com.sun.star.chart2.PieChart")) )
            return sal_False;
        if( aChartTypeName.match(C2U("com.sun.star.chart2.Net")) )
            return sal_False;
    }
    return sal_True;
}

AxisType ChartTypeHelper::getAxisType( const uno::Reference<
            XChartType >& xChartType, sal_Int32 nDimensionIndex )
{
    //@todo ask charttype itself --> need model change first
    if(!xChartType.is())
        return AxisType_CATEGORY;

    rtl::OUString aChartTypeName = xChartType->getChartType();
    if(2==nDimensionIndex)//z-axis
        return AxisType_CATEGORY;
    if(1==nDimensionIndex)//y-axis
        return AxisType_REALNUMBER;
    if(0==nDimensionIndex)//x-axis
    {
        if( aChartTypeName.match(C2U("com.sun.star.chart2.ScatterChart")) )
            return AxisType_REALNUMBER;
        return AxisType_CATEGORY;
    }
    return AxisType_CATEGORY;
}

sal_Int32 ChartTypeHelper::getDimensionCount( const uno::Reference< XChartType >& xChartType )
{
    sal_Int32 nDimension = 2;
    if( xChartType.is() )
    {
        uno::Reference< beans::XPropertySet > xChartTypeProp( xChartType, uno::UNO_QUERY );
        if( xChartTypeProp.is())
        {
            try
            {
                if( (xChartTypeProp->getPropertyValue( C2U( "Dimension" )) >>= nDimension) )
                {
                    return nDimension;
                }
                else
                {
                    DBG_ERROR( "Couldn't get Dimension from ChartTypeGroup" );
                }
            }
            catch( beans::UnknownPropertyException ex )
            {
                ASSERT_EXCEPTION( ex );
            }
        }
    }
    return nDimension;
}

//.............................................................................
} //namespace chart
//.............................................................................
