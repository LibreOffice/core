/*************************************************************************
 *
 *  $RCSfile: xltracer.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2003-11-05 13:37:17 $
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

using ::rtl::OUString;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::beans::PropertyValue;

// ============================================================================

static const XclTracerDetails pTracerDetails[] =
{
    { eUnKnown,          "UNKNOWN",         "UNKNOWN",         "Unknown trace property."            },
    { eRowLimitExceeded, "Row Limits",      "Sheet",           "Row limit exceeded."                },
    { eTabLimitExceeded, "Sheet Limits",    "Sheet",           "Sheet limit exceeded."              },
    { ePassword,         "Protection",      "Password",        "Document is password protected."    },
    { ePrintRange,       "Print",           "Print Range",     "Print Range present."               },
    { eHorizontalPrint,  "Print",           "Pages Wide",      "Horizontal pages not supported."    },
    { eShortDate,        "Cell Formatting", "Short Dates",     "Possible Date format issue."        },
    { eShrinkToFit,      "Cell Formatting", "Shrink To Fit",   "Shrink to fit not supported"        },
    { eAlignmentFill,    "Cell Formatting", "Alignment Fill",  "Alignment Fill not supported"       },
    { eDiagonalBorder,   "Cell Formatting", "Border",          "Diagonal border present."           },
    { eBorderLineStyle,  "Cell Formatting", "Border",          "Line style not supported.",         },
    { eFillPattern,      "Cell Formatting", "Pattern",         "Fill Pattern not supported.",       },
    { eInvisibleGrid,    "Properties",      "Grid Invisible",  "Grid invisible on first sheet"      },
    { eFormattedNote,    "Notes",           "Formatting",      "Text may be formatted."             },
    { eFormulaExtName,   "Formula",         "External Name",   "External names not supported."      },
    { ePivotDataSource,  "Chart",           "Pivot",           "External data source not supported."}
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
        sID += rtl::OUString::valueOf(static_cast<sal_Int32>(eProblem));
        OUString sProblem  = rtl::OUString ::createFromAscii(pTracerDetails[eProblem].mpProblem);

        switch (eProblem)
        {
           case eRowLimitExceeded:
               Context(eProblem,static_cast<sal_uInt16>(nValue));
               break;
           case eTabLimitExceeded:
               Context(eProblem,static_cast<sal_uInt16>(nValue));
               break;
           default:
               Context(eProblem);
               break;
        }
        Trace(sID, sProblem);
    }
}

void XclTracer::Context( XclTracerId eProblem, sal_uInt16 nTab )
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

void XclTracer::ProcessTraceOnce(XclTracerId eProblem, sal_uInt16 nTab)
{
    if( maFirstTimes[eProblem] )
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

void XclTracer::TraceInvalidRow( sal_uInt16 nTab, sal_uInt32 nRow, sal_uInt32 nMaxRow )
{
    if(nRow > nMaxRow)
        ProcessTraceOnce(eRowLimitExceeded, nTab);
}

void XclTracer::TraceInvalidTab( sal_uInt16 nTab, sal_uInt16 nMaxTab )
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
        TraceLog( eHorizontalPrint);  // will only be called once.
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

void XclTracer::TracePivotDataSource( bool bExternal)
{
    if(bExternal)
        ProcessTraceOnce(ePivotDataSource);
}

// ============================================================================

