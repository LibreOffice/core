/*************************************************************************
 *
 *  $RCSfile: tracer.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: jb $ $Date: 2001-02-27 14:27:25 $
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

// SUNPRO5 does not like the following to be done after including stdio.h, that's why it's here at the very
// beginning of the file
#undef _TIME_T_DEFINED
#include <time.h>
#include <rtl/string.hxx>
#include <map>

namespace configmgr
{
    typedef ::std::map< ::rtl::OString, void*, ::std::less< ::rtl::OString > > VirtualDevices;
}

#ifndef _CONFIGMGR_TRACER_HXX_
#include "tracer.hxx"
#endif

#ifdef CFG_ENABLE_TRACING

#include <cstdarg>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include <osl/process.h>
#include <osl/thread.h>
#include <osl/diagnose.h>
#include <rtl/ustring.hxx>

namespace configmgr
{

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

    VirtualDevices  m_aDevices;

    OTracerSetup()
        :m_nTraceMask(WARNING | ERROR)
        ,m_pOutputMedium(NULL)
        ,m_bInitialized(sal_False)
    {
        m_nThreadKey = ::osl_createThreadKey(&freeThreadData);
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
    static void SAL_CALL freeThreadData(void*p);
};

//==========================================================================
//= OConfigTracer
//==========================================================================
::osl::Mutex    OConfigTracer::s_aMutex;
OTracerSetup*   OConfigTracer::s_pImpl = NULL;
timeb           OConfigTracer::s_aStartTime;


//--------------------------------------------------------------------------
void OConfigTracer::startGlobalTimer()
{
    ftime( &s_aStartTime );
}

//--------------------------------------------------------------------------
sal_uInt32 OConfigTracer::getGlobalTimer()
{
    struct timeb currentTime;
    sal_uInt32 nSeconds;
    ftime( &currentTime );
    nSeconds = (sal_uInt32)( currentTime.time - s_aStartTime.time );
    return ( nSeconds * 1000 ) + (long)( currentTime.millitm - s_aStartTime.millitm );
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
    ::osl::MutexGuard aGuard(s_aMutex);
    ensureData();
    ++s_pImpl->indentDepth();
}

//--------------------------------------------------------------------------
void OConfigTracer::dec()
{
    ::osl::MutexGuard aGuard(s_aMutex);
    ensureData();
    --s_pImpl->indentDepth();
}

//--------------------------------------------------------------------------
void OConfigTracer::traceInfo(const sal_Char* _pFormat, ...)
{
    ::osl::MutexGuard aGuard(s_aMutex);
    ensureData();
    if (s_pImpl->isTracing(OTracerSetup::INFO) )
    {


        va_list args;
        va_start(args, _pFormat);
        implTrace("info", _pFormat, args);
        va_end(args);
    }
}

//--------------------------------------------------------------------------
void OConfigTracer::traceWarning(const sal_Char* _pFormat, ...)
{
    ::osl::MutexGuard aGuard(s_aMutex);
    ensureData();
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
    ::osl::MutexGuard aGuard(s_aMutex);
    ensureData();
    if (s_pImpl->isTracing(OTracerSetup::ERROR))
    {
        va_list args;
        va_start(args, _pFormat);
        implTrace("error", _pFormat, args);
        va_end(args);
    }
}

//--------------------------------------------------------------------------
void OConfigTracer::trace(const sal_Char* _pFormat, ...)
{
    va_list args;
    va_start(args, _pFormat);
    implTrace("", _pFormat, args);
    va_end(args);
}

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
void OConfigTracer::traceToVirtualDevice(const sal_Char* _pDeviceName, const sal_Char* _pFormat, ...)
{
    ::osl::MutexGuard aGuard(s_aMutex);
    ensureData();
    ensureInitalized();

    VirtualDevices::const_iterator aDeviceMediumPos = s_pImpl->m_aDevices.find(::rtl::OString(_pDeviceName));
    if (aDeviceMediumPos != s_pImpl->m_aDevices.end())
    {
        FILE* pDeviceMedium = (FILE*)aDeviceMediumPos->second;

        va_list args;
        va_start(args, _pFormat);
        vfprintf(pDeviceMedium, _pFormat, args);
        fflush(pDeviceMedium);
        va_end(args);
    }
}

//--------------------------------------------------------------------------
::rtl::OString OConfigTracer::getTimeStamp()
{
    time_t aTime = time(NULL);
    tm* pStructuredTime = gmtime(&aTime);
    ::rtl::OString sTimeStamp(asctime(pStructuredTime));
    // cut the trainling linefeed (asctime is defined to contain such a line feed)
    sal_Int32 nStampLen = sTimeStamp.getLength();
    if ((0 != nStampLen) && ('\n' == sTimeStamp.getStr()[nStampLen - 1]))
        sTimeStamp = sTimeStamp.copy(0, nStampLen - 1);

    return sTimeStamp;
}

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
            static sal_Char szMessage[1024] = "";
            fprintf(s_pImpl->m_pOutputMedium, "(%06lu)", getGlobalTimer());
        }
    }
    fprintf(s_pImpl->m_pOutputMedium, ": ");

    indent();

    vfprintf(s_pImpl->m_pOutputMedium, _pFormat, args);
    fprintf(s_pImpl->m_pOutputMedium,"\n");
    fflush(s_pImpl->m_pOutputMedium);
}

}   // namespace configmgr

#endif // defined(DEBUG) || defined(_DEBUG)

//**************************************************************************
// history:
//  $Log: not supported by cvs2svn $
//  Revision 1.5  2001/02/26 15:53:00  jb
//  Add thread-sensitive tracing
//
//  Revision 1.4  2001/02/13 09:48:02  dg
//  #83239# timing output
//
//  Revision 1.3  2000/11/29 12:45:31  fs
//  #80122# additional traces upon initialization (process id / executable name)
//
//  Revision 1.2  2000/11/07 12:14:37  hr
//  #65293#: includes
//
//  Revision 1.1.1.1  2000/09/18 16:13:41  hr
//  initial import
//
//  Revision 1.7  2000/09/15 09:51:51  willem.vandorp
//  OpenOffice header added
//
//  Revision 1.6  2000/08/31 10:00:21  fs
//  time_t unknown
//
//  Revision 1.5  2000/08/30 14:34:09  fs
//  getTimeStamp
//
//  Revision 1.4  2000/08/20 12:55:42  fs
//  #77860# introduced an impl class; introduces virtual trace devices
//
//  Revision 1.3  2000/08/17 07:18:02  lla
//  im/export
//
//  Revision 1.2  2000/08/10 06:53:45  fs
//  read settings from the ENVCFGFLAGS environment variable
//
//  Revision 1.1  2000/08/09 18:52:46  fs
//  helper classes for tracing
//
//
//  Revision 1.0 09.08.00 13:10:05  fs
//**************************************************************************

