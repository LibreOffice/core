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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

// ============================================================================
#include "xltracer.hxx"
#include <filter/msfilter/msfiltertracer.hxx>
#include "address.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::beans::PropertyValue;

// ============================================================================

// Trace Table details are grouped by functionality using the context entry.
// Each separate context starts at the next 1000 interval. New entries should
// be added to their appropriate context. New contexts should be added onto
// the end. Any gaps in the 1000 sequence or within the 1000 are the result
// of trace events no longer present.
static const XclTracerDetails pTracerDetails[] =
{
    { eUnKnown,             0,     "UNKNOWN",        "UNKNOWN",         "Unknown trace property."            },
    { eRowLimitExceeded,    1000,  "Limits",         "Sheet",           "Row limit exceeded."                },
    { eTabLimitExceeded,    1001,  "Limits",         "Sheet",           "Sheet limit exceeded."              },
    { ePassword,            2000,  "Protection",     "Password",        "Document is password protected."    },
    { ePrintRange,          3000,  "Print",          "Print Range",     "Print Range present."               },
    { eShortDate,           4000,  "CellFormatting", "Short Dates",     "Possible Date format issue."        },
    { eBorderLineStyle,     4004,  "CellFormatting", "Border",          "Line style not supported.",         },
    { eFillPattern,         4005,  "CellFormatting", "Pattern",         "Fill Pattern not supported.",       },
    { eInvisibleGrid,       5000,  "Properties",     "Grid Invisible",  "Grid invisible on first sheet."      },
    { eFormattedNote,       6000,  "Notes",          "Formatting",      "Text may be formatted."             },
    { eFormulaExtName,      7000,  "Formula",        "External Name",   "External names not supported."      },
    { eFormulaMissingArg,   7001,  "Formula",        "Missing Argument","Parameter missing."      },
    { ePivotDataSource,     8000,  "Chart",          "Pivot",           "External data source not supported."},
    { ePivotChartExists,    8001,  "Chart",          "Pivot",           "Pivot Chart not supported."},
    { eChartUnKnownType,    8002,  "Chart",          "Type",            "Chart Type not supported."},
    { eChartTrendLines,     8003,  "Chart",          "Type",            "Chart trendlines not supported."},
    { eChartOnlySheet,      8004,  "Chart",          "Type",            "Chart only sheet not supported."},
    { eChartRange,          8005,  "Chart",          "Source Data",     "Chart source ranges too complex."},
    { eChartDSName,         8006,  "Chart",          "Source Data",     "Series titles not linked to cells."},
    { eChartDataTable,      8007,  "Chart",          "Legend",          "Data table not supported."},
    { eChartLegendPosition, 8008,  "Chart",          "Legend",          "Position not guaranteed."},
    { eChartTextFormatting, 8009,  "Chart",          "Formatting",      "Text formatting present."},
    { eChartEmbeddedObj,    8010,  "Chart",          "Area",            "Object inside chart."},
    { eChartAxisAuto,       8012,  "Chart",          "Axis",            "Axis interval is automatic."},
    { eChartInvalidXY,      8013,  "Chart",          "Scatter",         "Unsupported or invalid data range for XY chart."},
    { eChartErrorBars,      8014,  "Chart",          "Type",            "Chart error bars not supported."},
    { eChartAxisManual,     8015,  "Chart",          "Axis",            "Manual axis crossing point adjusted."},
    { eUnsupportedObject,   9000,  "Object",         "Type",            "Limited Object support."},
    { eObjectNotPrintable,  9001,  "Object",         "Print",           "Object not printable."},
    { eDVType,              10000, "DataValidation", "Type",            "Custom type present."}
};

XclTracer::XclTracer( const String& rDocUrl, const OUString& rConfigPath ) :
    maFirstTimes(eTraceLength,true)
{
    Sequence< PropertyValue > aConfigData( 1 );
    aConfigData[ 0 ].Name = CREATE_OUSTRING( "DocumentURL" );
    aConfigData[ 0 ].Value <<= OUString( rDocUrl );
    mpTracer.reset( new MSFilterTracer( rConfigPath, &aConfigData ) );
    mpTracer->StartTracing();
    mbEnabled = mpTracer->IsEnabled();
}

XclTracer::~XclTracer()
{
    mpTracer->EndTracing();
}

void XclTracer::AddAttribute( const OUString& rName, const OUString& rValue )
{
    if( mbEnabled )
        mpTracer->AddAttribute( rName, rValue );
}

void XclTracer::Trace( const OUString& rElementID, const OUString& rMessage )
{
    if( mbEnabled )
    {
        mpTracer->Trace( rElementID, rMessage );
        mpTracer->ClearAttributes();
    }
}

void XclTracer::TraceLog( XclTracerId eProblem, sal_Int32 nValue )
{
    if( mbEnabled )
    {
        OUString sID( CREATE_STRING( "SC" ) );
        sID += OUString::valueOf( static_cast< sal_Int32 >( pTracerDetails[ eProblem ].mnID ) );
        OUString sProblem = OUString::createFromAscii( pTracerDetails[ eProblem ].mpProblem );

        switch (eProblem)
        {
           case eRowLimitExceeded:
               Context(eProblem,static_cast<SCTAB>(nValue));
               break;
           case eTabLimitExceeded:
               Context(eProblem,static_cast<SCTAB>(nValue));
               break;
           default:
               Context(eProblem);
               break;
        }
        Trace(sID, sProblem);
    }
}

void XclTracer::Context( XclTracerId eProblem, SCTAB nTab )
{
    OUString sContext = OUString::createFromAscii( pTracerDetails[ eProblem ].mpContext );
    OUString sDetail  = OUString::createFromAscii( pTracerDetails[ eProblem ].mpDetail );

    switch (eProblem)
    {
        case eRowLimitExceeded:
        case eTabLimitExceeded:
            sDetail += OUString::valueOf( static_cast< sal_Int32 >( nTab + 1 ) );
            break;
        default:
            break;
    }
    AddAttribute(sContext, sDetail);
}

void XclTracer::ProcessTraceOnce(XclTracerId eProblem, SCTAB nTab)
{
    if( mbEnabled && maFirstTimes[eProblem])
    {
        TraceLog(pTracerDetails[eProblem].meProblemId, nTab);
        maFirstTimes[eProblem] = false;
    }
}

void XclTracer::TraceInvalidAddress( const ScAddress& rPos, const ScAddress& rMaxPos )
{
    TraceInvalidRow(rPos.Tab(), rPos.Row(), rMaxPos.Row());
    TraceInvalidTab(rPos.Tab(), rMaxPos.Tab());
}

void XclTracer::TraceInvalidRow( SCTAB nTab, sal_uInt32 nRow, sal_uInt32 nMaxRow )
{
    if(nRow > nMaxRow)
        ProcessTraceOnce(eRowLimitExceeded, nTab);
}

void XclTracer::TraceInvalidTab( SCTAB nTab, SCTAB nMaxTab )
{
    if(nTab > nMaxTab)
        ProcessTraceOnce(eTabLimitExceeded, nTab);
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
