/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: tracer.cxx,v $
 * $Revision: 1.21 $
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
#include "precompiled_configmgr.hxx"

// SUNPRO5 does not like the following to be done after including stdio.h, that's why it's here at the very
// beginning of the file
#undef _TIME_T_DEFINED
#include <time.h>
#include <rtl/string.hxx>
#include <map>

#include "tracer.hxx"

#ifdef CFG_ENABLE_TRACING

#include <cstdarg>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include <osl/process.h>
#include <osl/thread.h>
#include <osl/diagnose.h>
#include <rtl/ustring.hxx>
#include <rtl/instance.hxx>

namespace configmgr
{

extern "C"
{
    static void call_freeThreadData(void*);
}

struct OTracerSetup
{
    enum {
        INFO    = 0x01,
        WARNING = 0x02,
        ERROR   = 0x04,
        LEVEL_MASK = 0x0f,

        TIME    = 0x10,
        THREAD  = 0x20,
        DATA_MASK = 0xf0
    };

    sal_uInt32      m_nTraceMask;
    FILE*           m_pOutputMedium;
    sal_Bool        m_bInitialized;
    oslThreadKey    m_nThreadKey;

    ::std::map< ::rtl::OString, void*, ::std::less< ::rtl::OString > >  m_aDevices;

    OTracerSetup()
        :m_nTraceMask(WARNING | ERROR)
        ,m_pOutputMedium(NULL)
        ,m_bInitialized(sal_False)
    {
        m_nThreadKey = ::osl_createThreadKey(call_freeThreadData);
    }
    ~OTracerSetup()
    {
        ::osl_destroyThreadKey(m_nThreadKey);
    }

    bool isTracing(sal_uInt32 nTraceValue) const
    { return (nTraceValue & this->m_nTraceMask) == nTraceValue; }

    void setTracing(sal_uInt32 nTraceValue)
    { this->m_nTraceMask |= nTraceValue; }

    void setTracing(sal_uInt32 nTraceValue, sal_uInt32 nMask)
    {
        OSL_ENSURE( (nTraceValue&nMask) == nTraceValue, "Flags being set must be part of mask");
        this->m_nTraceMask &= ~nMask;
        this->m_nTraceMask |= nTraceValue;
    }

    sal_Int32& indentDepth();

    struct ThreadData
    {
        ThreadData() : m_nIndentDepth(0) {}
        sal_Int32  m_nIndentDepth;
    };

    ThreadData& ensureThreadData();
    static void freeThreadData(void*p);
};

//==========================================================================
//= OConfigTracer
//==========================================================================
OTracerSetup*   OConfigTracer::s_pImpl = NULL;
#ifdef WNT
timeb           OConfigTracer::s_aStartTime;
#else
timeval         OConfigTracer::s_aStartTime;
#endif

::osl::Mutex &  OConfigTracer::getMutex()
{
    return rtl::Static<osl::Mutex,OConfigTracer>::get();
}
//--------------------------------------------------------------------------
void OConfigTracer::startGlobalTimer()
{
#ifdef WNT
    ftime( &s_aStartTime );
#else
    gettimeofday( &s_aStartTime, NULL );
#endif
}

//--------------------------------------------------------------------------
sal_uInt32 OConfigTracer::getGlobalTimer()
{
#ifdef WNT
    struct timeb currentTime;
    sal_uInt32 nSeconds;
    ftime( &currentTime );
    nSeconds = (sal_uInt32)( currentTime.time - s_aStartTime.time );
    return ( nSeconds * 1000 ) + (long)( currentTime.millitm - s_aStartTime.millitm );
#else
    struct timeval currentTime;
    sal_uInt32 nSeconds;
    gettimeofday( &currentTime, NULL );
    nSeconds = (sal_uInt32)( currentTime.tv_sec - s_aStartTime.tv_sec );
    return ( nSeconds * 1000 ) + (long)( currentTime.tv_usec - s_aStartTime.tv_usec )/1000;
#endif
}

//--------------------------------------------------------------------------
sal_Int32& OTracerSetup::indentDepth()
{
    return ensureThreadData().m_nIndentDepth;
}

//--------------------------------------------------------------------------
OTracerSetup::ThreadData& OTracerSetup::ensureThreadData()
{
    void * pThreadData = ::osl_getThreadKeyData(m_nThreadKey);

    OTracerSetup::ThreadData* pRet
        = static_cast< OTracerSetup::ThreadData * >(pThreadData);

    if (pRet == NULL)
    {
        pThreadData = pRet = new ThreadData();

        if (!::osl_setThreadKeyData(m_nThreadKey,pThreadData))
        {
            OSL_ENSURE(false, "Cannot create per-thread data for tracing");
            freeThreadData(pThreadData);

            static ThreadData sharedThreadData;
            pRet = &sharedThreadData;
        }
        else
            OSL_ASSERT( pThreadData == ::osl_getThreadKeyData(m_nThreadKey) );

        OSL_ASSERT( pRet != NULL );
    }

    return *pRet;
}

static void call_freeThreadData( void* p )
{
    OTracerSetup::freeThreadData( p );
}

//--------------------------------------------------------------------------
void OTracerSetup::freeThreadData(void* p)
{
    delete static_cast< OTracerSetup::ThreadData * > (p);
}

//--------------------------------------------------------------------------
void OConfigTracer::ensureData()
{
    if (s_pImpl)
        return;
    s_pImpl = new OTracerSetup;
}

//--------------------------------------------------------------------------
void OConfigTracer::inc()
{
    ::osl::MutexGuard aGuard(getMutex());
    ensureData();
    ++s_pImpl->indentDepth();
}

//--------------------------------------------------------------------------
void OConfigTracer::dec()
{
    ::osl::MutexGuard aGuard(getMutex());
    ensureData();
    --s_pImpl->indentDepth();
}

//--------------------------------------------------------------------------
void OConfigTracer::traceInfo(const sal_Char* _pFormat, ...)
{
    ::osl::MutexGuard aGuard(getMutex());
    ensureData();
    ensureInitalized();
    if (s_pImpl->isTracing(OTracerSetup::INFO) )
    {


        va_list args;
        va_start(args, _pFormat);
        implTrace("info", _pFormat, args);
        va_end(args);
    }
}
#if OSL_DEBUG_LEVEL > 0
//--------------------------------------------------------------------------
void OConfigTracer::traceWarning(const sal_Char* _pFormat, ...)
{
    ::osl::MutexGuard aGuard(getMutex());
    ensureData();
    ensureInitalized();
    if (s_pImpl->isTracing(OTracerSetup::WARNING))
    {
        va_list args;
        va_start(args, _pFormat);
        implTrace("warning", _pFormat, args);
        va_end(args);
    }
}

//--------------------------------------------------------------------------
void OConfigTracer::traceError(const sal_Char* _pFormat, ...)
{
    ::osl::MutexGuard aGuard(getMutex());
    ensureData();
    ensureInitalized();
    if (s_pImpl->isTracing(OTracerSetup::ERROR))
    {
        va_list args;
        va_start(args, _pFormat);
        implTrace("error", _pFormat, args);
        va_end(args);
    }
}
#endif
//--------------------------------------------------------------------------
void OConfigTracer::indent()
{
    sal_Int32 nIndent = s_pImpl->indentDepth();
    for (sal_Int32 i=0; i<nIndent; ++i)
        fprintf(s_pImpl->m_pOutputMedium, " ");
}

//--------------------------------------------------------------------------
FILE* disambiguate(const ::rtl::OString& _rFileName)
{
    FILE* pExistenceCheck = NULL;
    sal_Int32 i = 1;
    ::rtl::OString sLoop;
    while (i <= 256)
    {
        sLoop = _rFileName;
        sLoop += ".";
        sLoop += ::rtl::OString::valueOf(i);

        pExistenceCheck = fopen(sLoop.getStr(), "r");
        if (!pExistenceCheck)
            // does not exist
            return fopen(sLoop.getStr(), "w+");

        // already exists, try the next name
        fclose(pExistenceCheck);
        ++i;
    }

    // could not open such a file
    return NULL;
}

//--------------------------------------------------------------------------
void OConfigTracer::ensureInitalized()
{
    if (s_pImpl->m_bInitialized)
        return;

    s_pImpl->m_bInitialized = sal_True;

    char* pSettings = getenv("ENVCFGFLAGS");
    if (!pSettings)
        return;

    /*  currently recognized structure :
        + switches have to be separated by whitespaces
        + valid switches are:
            -m[e|o|f<file>] -   output to stderr (e), stdout (o) or a file (f). In the latter case the whole rest
                                of the param ('til the next one, means 'til the next whitespace) is the filename
            -t{i,w,e,p,d}*  -   type of output : i includes infos, w includes warnings, e includes errors
                                content : p includes timestamp, d includes thread-id
    */

    s_pImpl->m_pOutputMedium = stderr;
    s_pImpl->setTracing(0, OTracerSetup::LEVEL_MASK);
    s_pImpl->setTracing(0, OTracerSetup::DATA_MASK);

    char* pParamLoop = pSettings;
    while (*pParamLoop)
    {
        while (!isspace(*pParamLoop) && *pParamLoop)
            ++pParamLoop;

        sal_Int32 nLen = pParamLoop - pSettings;
        if ((nLen > 1) && (*pSettings == '-'))
        {
            ++pSettings;
            switch (*pSettings)
            {
                case 'm':
                case 'w':
                    if (nLen > 2)
                    {
                        ++pSettings;
                        switch (*pSettings)
                        {
                            case 'e':
                                s_pImpl->m_pOutputMedium = stderr;
                                break;
                            case 'o':
                                s_pImpl->m_pOutputMedium = stdout;
                                break;
                            case 'f':
                            {
                                ++pSettings;
                                // copy the filename into an own buffer
                                ::rtl::OString sFileName(pSettings, pParamLoop - pSettings);

                                // open the file
                                s_pImpl->m_pOutputMedium = disambiguate(sFileName);

                                break;
                            }
                        }
                    }
                    break;
                case 'd':
                {   // assign a virtual device
                    // copy the device assingment description
                    ::rtl::OString sDescription(pSettings + 1, pParamLoop - pSettings - 1);
                    sal_Int32 nSep = sDescription.indexOf(':');
                    if (-1 == nSep)
                        break;  // invalid format

                    ::rtl::OString sVirtualDeviceName, sFileName;
                    sVirtualDeviceName = sDescription.copy(0, nSep);
                    sFileName = sDescription.copy(nSep + 1);

                    FILE* pVirtualDevice = disambiguate(sFileName);
                    if (pVirtualDevice)
                        s_pImpl->m_aDevices[sVirtualDeviceName] = pVirtualDevice;
                }
                case 't':
                {
                    ++pSettings;
                    while (pSettings != pParamLoop)
                    {
                        switch (*pSettings)
                        {
                            case 'i':   s_pImpl->setTracing( OTracerSetup::INFO ); break;
                            case 'w':   s_pImpl->setTracing( OTracerSetup::WARNING ); break;
                            case 'e':   s_pImpl->setTracing(  OTracerSetup::ERROR ); break;
                            case 'p':   s_pImpl->setTracing(  OTracerSetup::TIME );
                                        startGlobalTimer();
                                        break;
                            case 'd':   s_pImpl->setTracing(  OTracerSetup::THREAD ); break;
                        }
                        ++pSettings;
                    }
                }
            }
        }

        if (!*pParamLoop)
            break;

        ++pParamLoop;
        pSettings = pParamLoop;
    }

    // trace some initial information
    CFG_TRACE_INFO_NI("initialization: process id: 0x%08X", osl_getProcess(0));
    ::rtl::OUString sExecutable;
    osl_getExecutableFile(&sExecutable.pData);
    CFG_TRACE_INFO_NI("initialization: executable file name: %s", OUSTRING2ASCII(sExecutable));
}

//--------------------------------------------------------------------------
//-----------------------------------------------------------
// need raw unsigned int to safely printf a value
static inline
unsigned int getThreadID()
{
    oslThreadIdentifier nRealThreadID = ::osl_getThreadIdentifier(NULL);

    return nRealThreadID; // if this loses data, we can still hope that lsb is changing between thraeds
}

//--------------------------------------------------------------------------
void OConfigTracer::implTrace(const sal_Char* _pType, const sal_Char* _pFormat, va_list args)
{
    ensureInitalized();
    if (!s_pImpl->m_pOutputMedium)
        // no tracing enabled
        return;

    if (_pType && strlen(_pType))
    {
        if (s_pImpl->isTracing(OTracerSetup::THREAD))
        {
            fprintf(s_pImpl->m_pOutputMedium, "[%04x] ", getThreadID());
        }

        fprintf(s_pImpl->m_pOutputMedium, "%s ", _pType);

        if (s_pImpl->isTracing(OTracerSetup::TIME))
        {
            fprintf(s_pImpl->m_pOutputMedium, "(%06" SAL_PRIuUINT32 ")", getGlobalTimer());
        }
    }
    fprintf(s_pImpl->m_pOutputMedium, ": ");

    indent();

    vfprintf(s_pImpl->m_pOutputMedium, _pFormat, args);
    fprintf(s_pImpl->m_pOutputMedium,"\n");
    fflush(s_pImpl->m_pOutputMedium);
}

}   // namespace configmgr

#endif // CFG_ENABLE_TRACING

