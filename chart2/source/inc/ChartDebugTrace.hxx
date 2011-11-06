/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef CHART2_CHARTDEBUGTRACE_HXX
#define CHART2_CHARTDEBUGTRACE_HXX

#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XDiagram.hpp>
#include "charttoolsdllapi.hxx"

#define CHART_TRACE_OSL_DEBUG_LEVEL 2

namespace chart
{
namespace debug
{

#if OSL_DEBUG_LEVEL >= CHART_TRACE_OSL_DEBUG_LEVEL
/** Does several OSL_TRACEs that show information about the given chart document
    (e.g. the chart-type manager, diagrams etc.)

    If OSL_DEBUG_LEVEL is less than CHART_TRACE_OSL_DEBUG_LEVEL this
    function does nothing.
 */
OOO_DLLPUBLIC_CHARTTOOLS void ChartDebugTraceDocument(
    const ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XChartDocument > & xDoc,
    int nIndent = 0 );

/** Does several OSL_TRACEs that show information about the given diagram
    (e.g. coordinate systems, data series etc.)

    If OSL_DEBUG_LEVEL is less than CHART_TRACE_OSL_DEBUG_LEVEL this
    function does nothing.
 */
OOO_DLLPUBLIC_CHARTTOOLS void ChartDebugTraceDiagram(
    const ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XDiagram > & xDiagram,
    int nIndent = 0 );
#endif

} // namespace debug
} //  namespace chart

// CHART2_CHARTDEBUGTRACE_HXX
#endif
