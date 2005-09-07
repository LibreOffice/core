/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: LegendHelper.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 00:41:51 $
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
#ifndef _CHART2_TOOLS_LEGENDHELPER_HXX
#define _CHART2_TOOLS_LEGENDHELPER_HXX

// header for class OUString
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XLEGEND_HPP_
#include <com/sun/star/chart2/XLegend.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XDIAGRAM_HPP_
#include <com/sun/star/chart2/XDiagram.hpp>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/

class LegendHelper
{
public:
    static rtl::OUString getIdentifierForLegend();

    static ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XLegend >
        getLegend( const ::com::sun::star::uno::Reference<
                       ::com::sun::star::frame::XModel >& xModel );

    /** fills the legend given with XChartTypeGroup elements as XLegendEntry
        objects.

        Note: In, e.g., a standard bar chart the legend contains only one entry,
        which is the chart type group.  To display all series in the legend, you
        have to get all XDataSeries in the tree starting at the chart type group
     */
    static void defaultFillEmptyLegend(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XLegend > & xLegend,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDiagram > & xDiagram );

    /** removes all legend entries
     */
    static void flushLegend(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XLegend > & xLegend );
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
