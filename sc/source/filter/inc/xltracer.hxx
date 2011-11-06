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



// ============================================================================

#ifndef SC_XLTRACER_HXX
#define SC_XLTRACER_HXX

#include "global.hxx"        // ScAddress
#include "xltools.hxx"

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

    /** Creates an element including all attributes set up to this call.
        @descr  Removes all attributes after the element is traced. */
    void                        Trace( const ::rtl::OUString& rElementID, const ::rtl::OUString& rMessage );

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
    void                        TraceFormulaMissingArg();
    void                        TracePivotDataSource(bool bExternal);
    void                        TracePivotChartExists();
    void                        TraceChartUnKnownType();
    void                        TraceChartOnlySheet();
    void                        TraceChartDataTable();
    void                        TraceChartLegendPosition();
    void                        TraceChartEmbeddedObj();
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

