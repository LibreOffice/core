/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ChartDebugTrace.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 18:12:43 $
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
#ifndef CHART2_CHARTDEBUGTRACE_HXX
#define CHART2_CHARTDEBUGTRACE_HXX

#ifndef _COM_SUN_STAR_CHART2_XCHARTDOCUMENT_HPP_
#include <com/sun/star/chart2/XChartDocument.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XDIAGRAM_HPP_
#include <com/sun/star/chart2/XDiagram.hpp>
#endif

#define CHART_TRACE_OSL_DEBUG_LEVEL 2

namespace chart
{
namespace debug
{

/** Does several OSL_TRACEs that show information about the given chart document
    (e.g. the chart-type manager, diagrams etc.)

    If OSL_DEBUG_LEVEL is less than CHART_TRACE_OSL_DEBUG_LEVEL this
    function does nothing.
 */
void ChartDebugTraceDocument(
    const ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XChartDocument > & xDoc,
    int nIndent = 0 );

/** Does several OSL_TRACEs that show information about the given diagram
    (e.g. coordinate systems, data series etc.)

    If OSL_DEBUG_LEVEL is less than CHART_TRACE_OSL_DEBUG_LEVEL this
    function does nothing.
 */
void ChartDebugTraceDiagram(
    const ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XDiagram > & xDiagram,
    int nIndent = 0 );

} // namespace debug
} //  namespace chart

// CHART2_CHARTDEBUGTRACE_HXX
#endif
