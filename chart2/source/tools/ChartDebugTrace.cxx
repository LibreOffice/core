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

#include "ChartDebugTrace.hxx"
#include "macros.hxx"
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/chart2/AxisType.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#include <com/sun/star/chart2/StackingDirection.hpp>
#include <rtl/math.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace chart
{
namespace debug
{

#if OSL_DEBUG_LEVEL >= CHART_TRACE_OSL_DEBUG_LEVEL

void ChartDebugTraceDocument(
    const Reference< XChartDocument > & /*xDoc*/,
    int /*nIndent*/ )
{

}

void ChartDebugTraceDiagram(
    const Reference< XDiagram > & /*xDiagram*/,
    int /*nIndent*/ )
{

}
#endif

} // namespace debug
} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
