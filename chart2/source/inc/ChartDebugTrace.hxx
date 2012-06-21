/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
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
