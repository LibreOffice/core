/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ChartTypeHelper.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 00:38:24 $
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

#ifndef _CHART2_CHARTTYPEHELPER_HXX
#define _CHART2_CHARTTYPEHELPER_HXX

#ifndef _COM_SUN_STAR_CHART2_XCHARTTYPE_HPP_
#include <com/sun/star/chart2/XChartType.hpp>
#endif

#ifndef _COM_SUN_STAR_CHART2_AXISTYPE_HPP_
#include <com/sun/star/chart2/AxisType.hpp>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/

class ChartTypeHelper
{
public:
    static sal_Bool isSupportingGeometryProperties(  const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType >& xChartType );
    static sal_Bool isSupportingStatisticProperties( const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType >& xChartType );
    static sal_Bool isSupportingRegressionProperties(const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType >& xChartType );
    static sal_Bool isSupportingMainAxis(            const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType >& xChartType, sal_Int32 nDimensionIndex );
    static sal_Bool isSupportingSecondaryAxis(       const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType >& xChartType, sal_Int32 nDimensionIndex );
    static sal_Bool isSupportingAreaProperties(      const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType >& xChartType );
    static sal_Bool isSupportingSymbolProperties(    const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType >& xChartType );

    static ::com::sun::star::chart2::AxisType
                    getAxisType( const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType >& xChartType
                                , sal_Int32 nDimensionIndex );

    static sal_Int32 getDimensionCount( const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType >& xChartType );
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
