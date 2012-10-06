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

#include "xltracer.hxx"
#include <com/sun/star/beans/PropertyValue.hpp>
#include "address.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::beans::PropertyValue;

XclTracer::XclTracer( const String& rDocUrl )
    : mbEnabled(false)
    , maFirstTimes(eTraceLength,true)
{
    Sequence< PropertyValue > aConfigData( 1 );
    aConfigData[ 0 ].Name = "DocumentURL";
    aConfigData[ 0 ].Value <<= OUString( rDocUrl );
}

XclTracer::~XclTracer()
{
}

void XclTracer::ProcessTraceOnce(XclTracerId eProblem)
{
    if( mbEnabled && maFirstTimes[eProblem])
    {
        maFirstTimes[eProblem] = false;
    }
}

void XclTracer::TraceInvalidAddress( const ScAddress& rPos, const ScAddress& rMaxPos )
{
    TraceInvalidRow(rPos.Row(), rMaxPos.Row());
    TraceInvalidTab(rPos.Tab(), rMaxPos.Tab());
}

void XclTracer::TraceInvalidRow( sal_uInt32 nRow, sal_uInt32 nMaxRow )
{
    if(nRow > nMaxRow)
        ProcessTraceOnce(eRowLimitExceeded);
}

void XclTracer::TraceInvalidTab( SCTAB nTab, SCTAB nMaxTab )
{
    if(nTab > nMaxTab)
        ProcessTraceOnce(eTabLimitExceeded);
}

void XclTracer::TracePrintRange()
{
    ProcessTraceOnce( ePrintRange);
}

void XclTracer::TraceDates( sal_uInt16 nNumFmt)
{
    // Short Date = 14 and Short Date+Time = 22
    if(nNumFmt == 14 || nNumFmt == 22)
        ProcessTraceOnce(eShortDate);
}

void XclTracer::TraceBorderLineStyle( bool bBorderLineStyle)
{
    if(bBorderLineStyle)
        ProcessTraceOnce(eBorderLineStyle);
}

void XclTracer::TraceFillPattern( bool bFillPattern)
{
    if(bFillPattern)
        ProcessTraceOnce(eFillPattern);
}

void XclTracer::TraceFormulaMissingArg()
{
    // missing parameter in Formula record
    ProcessTraceOnce(eFormulaMissingArg);
}

void XclTracer::TracePivotDataSource( bool bExternal)
{
    if(bExternal)
        ProcessTraceOnce(ePivotDataSource);
}

void XclTracer::TracePivotChartExists()
{
    // Pivot Charts not currently displayed.
    ProcessTraceOnce(ePivotChartExists);
}

void XclTracer::TraceChartUnKnownType()
{
    ProcessTraceOnce(eChartUnKnownType);
}

void XclTracer::TraceChartOnlySheet()
{
    ProcessTraceOnce(eChartOnlySheet);
}

void XclTracer::TraceChartDataTable()
{
    // Data table is not supported.
    ProcessTraceOnce(eChartDataTable);
}

void XclTracer::TraceChartLegendPosition()
{
    // If position is set to "not docked or inside the plot area" then
    // we cannot guarantee the legend position.
    ProcessTraceOnce(eChartLegendPosition);
}

void XclTracer::TraceUnsupportedObjects()
{
    // Called from Excel 5.0 - limited Graphical object support.
    ProcessTraceOnce(eUnsupportedObject);
}

void XclTracer::TraceObjectNotPrintable()
{
    ProcessTraceOnce(eObjectNotPrintable);
}

void XclTracer::TraceDVType(  bool bType)
{
    // Custom types work if 'Data->validity dialog' is not OKed.
    if(bType)
        ProcessTraceOnce(eDVType);
}

// ============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
