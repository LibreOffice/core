/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
