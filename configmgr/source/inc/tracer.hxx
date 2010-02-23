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

#ifndef _CONFIGMGR_TRACER_HXX_
#define _CONFIGMGR_TRACER_HXX_

#if OSL_DEBUG_LEVEL > 0
#define CFG_ENABLE_TRACING
#endif

#ifdef CFG_ENABLE_TRACING

#include <sal/types.h>
#include <rtl/string.hxx>
#include <osl/mutex.hxx>

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
#if OSL_DEBUG_LEVEL > 0
    static void traceWarning(const sal_Char* _pFormat, ...);
    static void traceError(const sal_Char* _pFormat, ...);
#endif
protected:
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

