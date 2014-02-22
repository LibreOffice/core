/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "xltracer.hxx"
#include <com/sun/star/beans/PropertyValue.hpp>
#include "address.hxx"

using ::com::sun::star::uno::Sequence;
using ::com::sun::star::beans::PropertyValue;

XclTracer::XclTracer( const OUString& rDocUrl )
    : mbEnabled(false)
    , maFirstTimes(eTraceLength,true)
{
    Sequence< PropertyValue > aConfigData( 1 );
    aConfigData[ 0 ].Name = "DocumentURL";
    aConfigData[ 0 ].Value <<= rDocUrl;
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
    
    ProcessTraceOnce(eFormulaMissingArg);
}

void XclTracer::TracePivotDataSource( bool bExternal)
{
    if(bExternal)
        ProcessTraceOnce(ePivotDataSource);
}

void XclTracer::TracePivotChartExists()
{
    
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
    
    ProcessTraceOnce(eChartDataTable);
}

void XclTracer::TraceChartLegendPosition()
{
    
    
    ProcessTraceOnce(eChartLegendPosition);
}

void XclTracer::TraceUnsupportedObjects()
{
    
    ProcessTraceOnce(eUnsupportedObject);
}

void XclTracer::TraceObjectNotPrintable()
{
    ProcessTraceOnce(eObjectNotPrintable);
}

void XclTracer::TraceDVType(  bool bType)
{
    
    if(bType)
        ProcessTraceOnce(eDVType);
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
