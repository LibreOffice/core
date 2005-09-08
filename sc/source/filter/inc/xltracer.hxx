/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xltracer.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 19:39:54 $
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

// ============================================================================

#ifndef SC_XLTRACER_HXX
#define SC_XLTRACER_HXX

#ifndef SC_SCGLOB_HXX
#include "global.hxx"        // ScAddress
#endif

#ifndef SC_XLTOOLS_HXX
#include "xltools.hxx"
#endif

// As Trace features become implemented, we can safely delete the enum entry as
// we use the member mnID to keep track of the actual trace tag ID value.
enum XclTracerId
{
    eUnKnown  ,          /// unused but allows us to set the correct index
    eRowLimitExceeded ,
    eTabLimitExceeded ,
    ePassword ,
    ePrintRange ,
    eShortDate ,
    eBorderLineStyle ,
    eFillPattern ,
    eInvisibleGrid ,
    eFormattedNote ,
    eFormulaExtName ,
    eFormulaMissingArg ,
    ePivotDataSource ,
    ePivotChartExists ,
    eChartUnKnownType ,
    eChartTrendLines ,
    eChartErrorBars ,
    eChartOnlySheet ,
    eChartRange ,
    eChartDSName,
    eChartDataTable,
    eChartLegendPosition,
    eChartTextFormatting,
    eChartEmbeddedObj,
    eChartAxisAuto,
    eChartAxisManual,
    eChartInvalidXY,
    eUnsupportedObject ,
    eObjectNotPrintable ,
    eDVType,
    eTraceLength         /// this *should* always be the final entry
};

struct XclTracerDetails
{
    XclTracerId                 meProblemId;    /// Excel Import Trace index.
    sal_uInt32                  mnID;           /// actual ID Index trace tag Value
    const sal_Char*             mpContext;      /// Context for problem e.g. Limits
    const sal_Char*             mpDetail;       /// Context Detail e.g. SheetX
    const sal_Char*             mpProblem;      /// Description of problem
};


// ============================================================================

class MSFilterTracer;

/** This class wraps an MSFilterTracer to create trace logs for import/export filters. */
class XclTracer
{
public:
    explicit                    XclTracer( const String& rDocUrl, const ::rtl::OUString& rConfigPath );
    virtual                     ~XclTracer();

    /** Returns true, if tracing is enabled. */
    inline bool                 IsEnabled() const { return mbEnabled; }

    /** Adds an attribute to be traced with the next Trace() call. */
    void                        AddAttribute( const ::rtl::OUString& rName, const ::rtl::OUString& rValue );
    /** Adds an attribute to be traced with the next Trace() call. */
    void                        AddAttribute( const ::rtl::OUString& rName, sal_Int32 nValue );

    /** Creates an element including all attributes set up to this call.
        @descr  Removes all attributes after the element is traced. */
    void                        Trace( const ::rtl::OUString& rElementID, const ::rtl::OUString& rMessage );
    /** Creates an element including all attributes set up to this call.
        @descr  Removes all attributes after the element is traced. */
    void                        Trace( const ::rtl::OUString& rElementID, sal_Int32 nMessage );

    /** Calls Trace() with a known document properties problem. */
    void                        TraceLog( XclTracerId eProblem, sal_Int32 nValue = 0 );

    /** Calls AddAttribute() to create the Context & Detail for known problems. */
    void                        Context( XclTracerId eProblem, SCTAB nTab = 0 );

    /** Ensure that particular traces are logged once per document. */
    void                        ProcessTraceOnce(XclTracerId eProblem, SCTAB nTab = 0);

    void                        TraceInvalidAddress(const ScAddress& rPos, const ScAddress& rMaxPos);
    void                        TraceInvalidRow( SCTAB nTab,  sal_uInt32 nRow, sal_uInt32 nMaxrow );
    void                        TraceInvalidTab( SCTAB nTab, SCTAB nMaxTab);
    void                        TracePrintRange();
    void                        TraceDates(sal_uInt16 nNumFmt);
    void                        TraceBorderLineStyle(bool bBorderLineStyle);
    void                        TraceFillPattern(bool bFillPattern);
    void                        TraceInvisibleGrid(bool bVisibleGrid);
    void                        TraceFormulaExtName();
    void                        TraceFormulaMissingArg();
    void                        TracePivotDataSource(bool bExternal);
    void                        TracePivotChartExists();
    void                        TraceChartUnKnownType();
    void                        TraceChartTrendLines();
    void                        TraceChartErrorBars();
    void                        TraceChartOnlySheet();
    void                        TraceChartRange();
    void                        TraceChartDSName();
    void                        TraceChartDataTable();
    void                        TraceChartLegendPosition();
    void                        TraceChartTextFormatting();
    void                        TraceChartEmbeddedObj();
    void                        TraceChartAxisAutoCross();
    void                        TraceChartAxisManualCross();
    void                        TraceChartInvalidXY();
    void                        TraceUnsupportedObjects();
    void                        TraceObjectNotPrintable();
    void                        TraceDVType(bool bType);

    /** Returns the SVX filter tracer for usage in external code (i.e. Escher). */
    inline MSFilterTracer&      GetBaseTracer() { return *mpTracer; }

private:
    typedef ::std::auto_ptr< MSFilterTracer > MSFilterTracerPtr;
    MSFilterTracerPtr           mpTracer;
    bool                        mbEnabled;
    typedef ::std::vector< bool >     BoolVec;
    /** array of flags corresponding to each entry in the XclTracerDetails table. */
    BoolVec                     maFirstTimes;
};


// ============================================================================

#endif

