/*************************************************************************
 *
 *  $RCSfile: xltracer.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2003-11-05 13:43:53 $
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
#define SC_XLTRACER_HXX

#ifndef SC_SCGLOB_HXX
#include "global.hxx"        // ScAddress
#endif

#ifndef SC_XLTOOLS_HXX
#include "xltools.hxx"
#endif

enum XclTracerId
{
    eUnKnown  = 0,          /// unused but allows us to set the correct index
    eRowLimitExceeded = 1,
    eTabLimitExceeded = 2,
    ePassword = 3,
    ePrintRange = 4,
    eHorizontalPrint = 5,
    eShortDate = 6,
    eShrinkToFit = 7,
    eAlignmentFill = 8,
    eDiagonalBorder = 9,
    eBorderLineStyle = 10,
    eFillPattern = 11,
    eInvisibleGrid = 12,
    eFormattedNote = 13,
    eFormulaExtName = 14,
    ePivotDataSource = 15,
    eTraceLength = 16        /// this *should* always be the final entry
};

struct XclTracerDetails
{
    XclTracerId                 meProblemId;    /// Excel Import Trace index.
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
    void                        Context( XclTracerId eProblem, sal_uInt16 nTab = 0 );

    /** Ensure that particular traces are logged once per document. */
    void                        ProcessTraceOnce(XclTracerId eProblem, sal_uInt16 nTab = 0);

    void                        TraceInvalidAddress(const ScAddress& rPos, const ScAddress& rMaxPos);
    void                        TraceInvalidRow( sal_uInt16 nTab,  sal_uInt32 nRow, sal_uInt32 nMaxrow );
    void                        TraceInvalidTab( sal_uInt16 nTab, sal_uInt16 nMaxTab);
    void                        TracePrintRange();
    void                        TracePrintFitToPages(sal_uInt16 nFitWidth);
    void                        TraceDates(sal_uInt16 nNumFmt);
    void                        TraceShrinkToFit(bool bShrinkToFit);
    void                        TraceAlignmentFill(bool bAlignmentFill);
    void                        TraceDiagonalBorder(bool bDiagonalBorder);
    void                        TraceBorderLineStyle(bool bBorderLineStyle);
    void                        TraceFillPattern(bool bFillPattern);
    void                        TraceInvisibleGrid(bool bVisibleGrid);
    void                        TraceFormattedNote(bool bFormattedNote);
    void                        TraceFormulaExtName();
    void                        TracePivotDataSource(bool bExternal);

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

