/*************************************************************************
 *
 *  $RCSfile: xltracer.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2004-06-04 10:48:54 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

// ============================================================================

#ifndef SC_XLTRACER_HXX
#include "xltracer.hxx"
#endif

#ifndef _MS_FILTERTRACER_HXX
#include <svx/msfiltertracer.hxx>
#endif

#ifndef SC_ADDRESS_HXX
#include "address.hxx"
#endif

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
    { eHorizontalPrint,     3001,  "Print",          "Pages Wide",      "Horizontal pages not supported."    },
    { eShortDate,           4000,  "CellFormatting", "Short Dates",     "Possible Date format issue."        },
    { eShrinkToFit,         4001,  "CellFormatting", "Shrink To Fit",   "Shrink to fit not supported."        },
    { eAlignmentFill,       4002,  "CellFormatting", "Alignment Fill",  "Alignment Fill not supported."       },
    { eDiagonalBorder,      4003,  "CellFormatting", "Border",          "Diagonal border present."           },
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
    { eChartRange,          8005,  "Chart",          "Source Data",     "Category and Value range not symmetrical."},
    { eChartDSName,         8006,  "Chart",          "Source Data",     "Name not linked to cell."},
    { eChartDataTable,      8007,  "Chart",          "Legend",          "Data table not supported."},
    { eChartLegendPosition, 8008,  "Chart",          "Legend",          "Position not guaranteed."},
    { eChartTextFormatting, 8009,  "Chart",          "Formatting",      "Text formatting present."},
    { eChartEmbeddedObj,    8010,  "Chart",          "Area",            "Object inside chart."},
    { eChartBorderAuto,     8011,  "Chart",          "Area",            "Automatic border in chart area."},
    { eChartAxisAuto,       8012,  "Chart",          "Axis",            "Axis interval is automatic."},
    { eChartInvalidXY,      8013,  "Chart",          "Scatter",         "Unsupported data range."},
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

void XclTracer::AddAttribute( const ::rtl::OUString& rName, sal_Int32 nValue )
{
    if( mbEnabled )
        mpTracer->AddAttribute( rName, OUString::valueOf( nValue ) );
}

void XclTracer::Trace( const OUString& rElementID, const OUString& rMessage )
{
    if( mbEnabled )
    {
        mpTracer->Trace( rElementID, rMessage );
        mpTracer->ClearAttributes();
    }
}

void XclTracer::Trace( const ::rtl::OUString& rElementID, sal_Int32 nMessage )
{
    if( mbEnabled )
    {
        mpTracer->Trace( rElementID, OUString::valueOf( nMessage ) );
        mpTracer->ClearAttributes();
    }
}

void XclTracer::TraceLog( XclTracerId eProblem, sal_Int32 nValue )
{
    if( mbEnabled )
    {
        OUString sID(CREATE_STRING("SC"));
        sID += rtl::OUString::valueOf(static_cast<sal_Int32>(pTracerDetails[eProblem].mnID));
        OUString sProblem  = rtl::OUString ::createFromAscii(pTracerDetails[eProblem].mpProblem);

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
    OUString sContext = rtl::OUString ::createFromAscii(pTracerDetails[eProblem].mpContext);
    OUString sDetail  = rtl::OUString ::createFromAscii(pTracerDetails[eProblem].mpDetail);

    switch (eProblem)
    {
        case eRowLimitExceeded:
        case eTabLimitExceeded:
            sDetail += rtl::OUString::valueOf(static_cast<sal_Int32>(nTab+1));
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

void XclTracer::TracePrintFitToPages( sal_uInt16 nFitWidth)
{
    if(nFitWidth > 1)
        ProcessTraceOnce( eHorizontalPrint);
}

void XclTracer::TraceDates( sal_uInt16 nNumFmt)
{
    // Short Date = 14 and Short Date+Time = 22
    if(nNumFmt == 14 || nNumFmt == 22)
        ProcessTraceOnce(eShortDate);
}

void XclTracer::TraceShrinkToFit(bool bShrinkToFit)
{
    if(bShrinkToFit)
        ProcessTraceOnce(eShrinkToFit);
}

void XclTracer::TraceAlignmentFill( bool bAlignmentFill)
{
    if(bAlignmentFill)
        ProcessTraceOnce(eAlignmentFill);
}

void XclTracer::TraceDiagonalBorder( bool bDiagonalBorder)
{
    if(bDiagonalBorder)
        ProcessTraceOnce(eDiagonalBorder);
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

void XclTracer::TraceInvisibleGrid( bool bVisibleGrid)
{
    // Excel supports Grid lines on a per sheet basis while
    // Calc displays grid lines on a per doc basis based on the
    // first visible sheet. If the first visible sheet has the
    // grid turned off then potentially we may have a problem.

    if(!bVisibleGrid)
        ProcessTraceOnce(eInvisibleGrid);
}

void XclTracer::TraceFormattedNote( bool bFormattedNote)
{
    if(bFormattedNote)
        ProcessTraceOnce(eFormattedNote);
}

void XclTracer::TraceFormulaExtName( )
{
    // import cannot access Excel External name ranges in
    // Formulas - see #i3740#.
    ProcessTraceOnce(eFormulaExtName);
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

void XclTracer::TraceChartTrendLines()
{
    ProcessTraceOnce(eChartTrendLines);
}

void XclTracer::TraceChartOnlySheet()
{
    ProcessTraceOnce(eChartOnlySheet);
}

void XclTracer::TraceChartRange()
{
    // Chart range symmetry is essential to display a chart. If the count
    // of category values is not equal to the count of values or the start row/column
    // depending on type, is not the same, then the chart range is said to be not
    // symmetrical and will not display correctly.
    ProcessTraceOnce(eChartRange);
}

void XclTracer::TraceChartDSName()
{
    // Data series names must be linked to a cell to appear. Hard
    // coded strings contained in the ChartSeriestext() do not appear.
    ProcessTraceOnce(eChartDSName);
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

void XclTracer::TraceChartTextFormatting()
{
    // text formatting in titles or data labels not supported.
    ProcessTraceOnce(eChartTextFormatting);
}

void XclTracer::TraceChartEmbeddedObj()
{
    // drawing objects e.g. text boxes etc not supported inside charts
    ProcessTraceOnce(eChartEmbeddedObj);
}

void XclTracer::TraceChartBorderAuto()
{
    // Excel Automatic Chart Area borders use a different
    // default style to Calc.
    ProcessTraceOnce(eChartBorderAuto);
}

void XclTracer::TraceChartAxisAuto()
{
    // Axis intervals generated automatically may not
    // be the same.
    ProcessTraceOnce(eChartAxisAuto);
}

void XclTracer::TraceChartInvalidXY()
{
    // Scatter charts will ony appear if the category(X) data range is
    // located in the left most column in relation to the value range.
    ProcessTraceOnce(eChartInvalidXY);
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

