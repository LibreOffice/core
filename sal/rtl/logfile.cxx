/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <cstdarg>
#include <cstdio>
#include <stdio.h>
#include <stdarg.h>

#include <rtl/logfile.h>
#include <osl/process.h>
#include <osl/time.h>
#include <osl/mutex.hxx>
#include <rtl/bootstrap.h>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/alloc.h>
#include <rtl/instance.hxx>
#include <sal/log.hxx>
#include "osl/thread.h"
#include "osl/thread.hxx"

#include <algorithm>

#ifdef _MSC_VER
#define vsnprintf _vsnprintf
#endif

using namespace osl;
using namespace std;

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

namespace {

static oslFileHandle g_aFile = 0;
static sal_Bool g_bHasBeenCalled = sal_False;
static const sal_Int32 g_BUFFERSIZE = 4096;
static sal_Char *g_buffer = 0;

class   LoggerGuard
{
public:
    ~LoggerGuard();
};

LoggerGuard::~LoggerGuard()
{
    if( g_buffer )
    {
        sal_Int64 nWritten, nConverted =
            sprintf( g_buffer, "closing log file at %06" SAL_PRIuUINT32, osl_getGlobalTimer() );
        if( nConverted > 0 )
            osl_writeFile( g_aFile, g_buffer, nConverted, (sal_uInt64 *)&nWritten );
        osl_closeFile( g_aFile );
        g_aFile = 0;

        rtl_freeMemory( g_buffer );
        g_buffer = 0;
        g_bHasBeenCalled = sal_False;
    }
}

// The destructor of this static LoggerGuard is "activated" by the assignment to
// g_buffer in init():
LoggerGuard loggerGuard;

namespace
{
    class theLogMutex : public rtl::Static<osl::Mutex, theLogMutex>{};
}

static Mutex & getLogMutex()
{
    return theLogMutex::get();
}

OUString getFileUrl( const OUString &name )
{
    OUString aRet;
    if ( osl_getFileURLFromSystemPath( name.pData, &aRet.pData )
         != osl_File_E_None )
    {
        SAL_WARN(
            "sal.rtl", "osl_getFileURLFromSystemPath failed for \"" << name << '"');
    }

    OUString aWorkingDirectory;
    osl_getProcessWorkingDir( &(aWorkingDirectory.pData) );
    osl_getAbsoluteFileURL( aWorkingDirectory.pData, aRet.pData, &(aRet.pData) );

    return aRet;
}

void init() {
    if( !g_bHasBeenCalled )
    {
        MutexGuard guard( getLogMutex() );
        if( ! g_bHasBeenCalled )
        {
            OUString name( "RTL_LOGFILE" );
            OUString value;
            if( rtl_bootstrap_get( name.pData, &value.pData, 0 ) )
            {
                //  Obtain process id.
                oslProcessIdentifier aProcessId = 0;
                oslProcessInfo info;
                info.Size = sizeof (oslProcessInfo);
                if (osl_getProcessInfo (0, osl_Process_IDENTIFIER, &info) == osl_Process_E_None)
                    aProcessId = info.Ident;

                //  Construct name of log file and open the file.
                OUStringBuffer buf( 128 );
                buf.append( value );

                // if the filename ends with .nopid, the incoming filename is not modified
                if( value.getLength() < 6 /* ".nopid" */ ||
                    rtl_ustr_ascii_compare_WithLength(
                        value.getStr() + (value.getLength()-6) , 6 , ".nopid" ) )
                {
                    buf.appendAscii( "_" );
                    buf.append( (sal_Int32) aProcessId );
                    buf.appendAscii( ".log" );
                }

                OUString o = getFileUrl( buf.makeStringAndClear() );
                oslFileError e = osl_openFile(
                    o.pData, &g_aFile, osl_File_OpenFlag_Write|osl_File_OpenFlag_Create);

                if( osl_File_E_None == e )
                {
                    TimeValue aCurrentTime;
                    g_buffer = ( sal_Char * ) rtl_allocateMemory( g_BUFFERSIZE );
                    sal_Int64 nConverted = 0;
                    if (osl_getSystemTime (&aCurrentTime))
                    {
                        nConverted = (sal_Int64 ) sprintf (
                                g_buffer,
                                "opening log file %f seconds past January 1st 1970\n"
                                "corresponding to %" SAL_PRIuUINT32 " ms after timer start\n",
                                aCurrentTime.Seconds + 1e-9 * aCurrentTime.Nanosec,
                                osl_getGlobalTimer());

                        if( nConverted > 0 )
                        {
                            sal_Int64 nWritten;
                            osl_writeFile( g_aFile, g_buffer, nConverted , (sal_uInt64 *)&nWritten );
                        }
                    }

                    nConverted = sprintf (g_buffer, "Process id is %" SAL_PRIuUINT32 "\n", aProcessId);
                    if( nConverted )
                    {
                        sal_Int64 nWritten;
                        osl_writeFile( g_aFile, g_buffer, nConverted, (sal_uInt64 *)&nWritten );
                    }
                }
                else
                {
                    SAL_WARN(
                        "sal.rtl",
                        "Couldn't open logfile " << o << '(' << +e << ')');
                }
            }
            g_bHasBeenCalled = sal_True;
        }
    }
}

}

extern "C" void SAL_CALL rtl_logfile_trace  ( const char *pszFormat, ... )
{
    init();
    if( g_buffer )
    {
        va_list args;
        va_start(args, pszFormat);
        {
            sal_Int64 nConverted, nWritten;
            MutexGuard guard( getLogMutex() );
            nConverted = vsnprintf( g_buffer , g_BUFFERSIZE, pszFormat, args );
            nConverted = (nConverted > g_BUFFERSIZE ? g_BUFFERSIZE : nConverted );
            if( nConverted > 0 )
                osl_writeFile( g_aFile, g_buffer, nConverted, (sal_uInt64*)&nWritten );
        }
        va_end(args);
    }
}

extern "C" void SAL_CALL rtl_logfile_longTrace(char const * format, ...) {
    init();
    if (g_buffer != 0) {
        sal_uInt32 time = osl_getGlobalTimer();
        oslThreadIdentifier threadId = osl::Thread::getCurrentIdentifier();
        va_list args;
        va_start(args, format);
        {
            MutexGuard g(getLogMutex());
            int n1 = snprintf(
                g_buffer, g_BUFFERSIZE, "%06" SAL_PRIuUINT32 " %" SAL_PRIuUINT32 " ", time, threadId);
            if (n1 >= 0) {
                sal_uInt64 n2;
                osl_writeFile(
                    g_aFile, g_buffer,
                    static_cast< sal_uInt64 >(
                        std::min(n1, static_cast< int >(g_BUFFERSIZE))),
                    &n2);
                n1 = vsnprintf(g_buffer, g_BUFFERSIZE, format, args);
                if (n1 > 0) {
                    osl_writeFile(
                        g_aFile, g_buffer,
                        static_cast< sal_uInt64 >(
                            std::min(n1, static_cast< int >(g_BUFFERSIZE))),
                        &n2);
                }
            }
        }
        va_end(args);
    }
}

extern "C" sal_Bool SAL_CALL rtl_logfile_hasLogFile( void ) {
    init();
    return g_buffer != 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
