/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tracer.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 23:24:27 $
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

#ifndef _CONFIGMGR_TRACER_HXX_
#define _CONFIGMGR_TRACER_HXX_

#if OSL_DEBUG_LEVEL > 0
#define CFG_ENABLE_TRACING
#endif

#ifdef CFG_ENABLE_TRACING

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#include <rtl/string.hxx>
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#include <stdarg.h>
#include <stdio.h>

#ifdef WNT
#include <sys/timeb.h>
#else
#include <sys/time.h>
#endif

#define OUSTRING2ASCII(rtlOUString) ::rtl::OString((rtlOUString).getStr(), (rtlOUString).getLength(), RTL_TEXTENCODING_ASCII_US).getStr()

#define CFG_TRACE_INFO          OTraceIndent aIndent; OConfigTracer::traceInfo
// FIXME: do something more intelligent here
// the CFG_TRACE_INFO2 macro exists only to have a unique name of aIndent2
// so it does not hide aIndent in an outer scope (which would produce a
// a shadowing warning
#define CFG_TRACE_INFO2         OTraceIndent aIndent2; OConfigTracer::traceInfo
#define CFG_TRACE_WARNING       OTraceIndent aIndent; OConfigTracer::traceWarning
#define CFG_TRACE_ERROR         OTraceIndent aIndent; OConfigTracer::traceError
#define CFG_TRACE_INFO_NI       OConfigTracer::traceInfo
#define CFG_TRACE_WARNING_NI    OConfigTracer::traceWarning
#define CFG_TRACE_ERROR_NI      OConfigTracer::traceError


#define CFG_TRACE_TO_DEVICE     OConfigTracer::traceToVirtualDevice

namespace configmgr
{

struct OTracerSetup;
class OConfigTracer
{
    friend class OTraceIndent;

protected:
    static  ::osl::Mutex &  getMutex();
    static OTracerSetup*    s_pImpl;
#ifdef WNT
    static timeb            s_aStartTime;
#else
    static timeval          s_aStartTime;
#endif

private:
    OConfigTracer();    // never implemented, no instantiation of this class allowed, only static members

public:
    static void traceInfo(const sal_Char* _pFormat, ...);
    static void traceWarning(const sal_Char* _pFormat, ...);
    static void traceError(const sal_Char* _pFormat, ...);
    static void traceToVirtualDevice(const sal_Char* _pDeviceName, const sal_Char* _pFormat, ...);

    static ::rtl::OString getTimeStamp();

protected:
    static void trace(const sal_Char* _pFormat, ...);
    static void implTrace(const sal_Char* _pType, const sal_Char* _pFormat, va_list args);
    static void startGlobalTimer();
    static sal_uInt32 getGlobalTimer();

    static void inc();
    static void dec();

    static void indent();

    static void ensureData();
    static void ensureInitalized();
};

class OTraceIndent
{
public:
    OTraceIndent() { OConfigTracer::inc(); }
    ~OTraceIndent() { OConfigTracer::dec(); }
};

}   // namespace configmgr

#else   // !CFG_ENABLE_TRACING

#include <stdio.h>

#define OUSTRING2ASCII(rtlOUString) "nothing"

namespace configmgr {
inline static void dont_trace(const char*,...) {}
}

#define CFG_TRACE_INFO          dont_trace
#define CFG_TRACE_INFO2         dont_trace
#define CFG_TRACE_WARNING       dont_trace
#define CFG_TRACE_ERROR         dont_trace
#define CFG_TRACE_INFO_NI       dont_trace
#define CFG_TRACE_WARNING_NI        dont_trace
#define CFG_TRACE_ERROR_NI      dont_trace
#define CFG_TRACE_TO_DEVICE     dont_trace

#endif  // CFG_ENABLE_TRACING

#endif // _CONFIGMGR_TRACER_HXX_

