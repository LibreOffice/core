/*************************************************************************
 *
 *  $RCSfile: ChartTypeHelper.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: iha $ $Date: 2003-11-10 19:32:18 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "ChartTypeHelper.hxx"
#include "macros.hxx"

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star::chart2;

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

sal_Bool ChartTypeHelper::isSupportingSecondaryYAxis( const uno::Reference< XChartType >& xChartType )
{
    //3D, pie and net charts do not support a second y-axis

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
            //@todo add the more complicated line charts
        }
    }
    return sal_True;
}

sal_Bool ChartTypeHelper::isSupportingSymbolProperties( const uno::Reference< XChartType >& xChartType )
{
    //only special 2D symbol charts do support symbols

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
        if( aChartTypeName.indexOf(C2U("symbol"))!=-1 )
            return sal_True;
    }
    return sal_False;
}

//.............................................................................
} //namespace chart
//.............................................................................
