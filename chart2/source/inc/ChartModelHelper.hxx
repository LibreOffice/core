/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ChartModelHelper.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 11:55:41 $
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
#ifndef _CHART2_CONTROLLER_CHARTMODELHELPER_HXX
#define _CHART2_CONTROLLER_CHARTMODELHELPER_HXX

#ifndef _COM_SUN_STAR_CHART2_XCHARTTYPE_HPP_
#include <com/sun/star/chart2/XChartType.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XDATASERIES_HPP_
#include <com/sun/star/chart2/XDataSeries.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XDIAGRAM_HPP_
#include <com/sun/star/chart2/XDiagram.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XCHARTDOCUMENT_HPP_
#include <com/sun/star/chart2/XChartDocument.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_SIZE_HPP_
#include <com/sun/star/awt/Size.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif

#include <vector>

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/

class ChartModelHelper
{
public:
    static ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDiagram >
        findDiagram( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xModel );

    static ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDiagram >
        findDiagram( const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartDocument >& xChartDoc );

    static ::std::vector< ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XDataSeries > > getDataSeries(
            const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartDocument > & xChartDoc );
    static ::std::vector< ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XDataSeries > > getDataSeries(
            const ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XModel > & xModel );

    static ::com::sun::star::uno::Reference<
    ::com::sun::star::chart2::XChartType >
        getChartTypeOfSeries(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::frame::XModel >& xModel
            , const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XDataSeries >& xGivenDataSeries );

    static ::com::sun::star::awt::Size getPageSize(
        const ::com::sun::star::uno::Reference<
                ::com::sun::star::frame::XModel >& xModel );

    static void setPageSize( const ::com::sun::star::awt::Size& rSize
                           , const ::com::sun::star::uno::Reference<
                                ::com::sun::star::frame::XModel >& xModel );

    static void triggerRangeHighlighting( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::frame::XModel >& xModel );
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
