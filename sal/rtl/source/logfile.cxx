/*************************************************************************
 *
 *  $RCSfile: logfile.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2004-06-17 13:28:10 $
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
 *  Copyright: 2001 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include <cstdarg>
#include <cstdio>
#include <stdio.h>
#include <stdarg.h>

#include <rtl/logfile.h>

#ifndef _OSL_PROCESS_H_
#include <osl/process.h>
#endif
#ifndef _OSL_FILE_H_
#include <osl/time.h>
#endif
#ifndef _OSL_TIME_H_
#include <osl/time.h>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _RTL_BOOTSTRAP_H_
#include <rtl/bootstrap.h>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _RTL_STRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef _RTL_ALLOC_H_
#include <rtl/alloc.h>
#endif

#ifdef _MSC_VER
#define vsnprintf _vsnprintf
#endif


using namespace osl;
using namespace std;

namespace rtl
{
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
            sprintf( g_buffer, "closing log file at %06lu", osl_getGlobalTimer() );
        if( nConverted > 0 )
            osl_writeFile( g_aFile, g_buffer, nConverted, (sal_uInt64 *)&nWritten );
        osl_closeFile( g_aFile );
        g_aFile = 0;

        rtl_freeMemory( g_buffer );
        g_buffer = 0;
        g_bHasBeenCalled = sal_False;
    }
}

static LoggerGuard guard;
}
using namespace rtl;

static Mutex & getLogMutex()
{
    static Mutex *pMutex = 0;
    if( !pMutex )
    {
        MutexGuard guard( Mutex::getGlobalMutex() );
        if( ! pMutex )
        {
            static Mutex mutex;
            pMutex = &mutex;
        }
    }
    return *pMutex;
}

static OUString getFileUrl( const OUString &name )
{
    OUString aRet;
    OSL_VERIFY( osl_getFileURLFromSystemPath( name.pData, &aRet.pData ) == osl_File_E_None );

    OUString aWorkingDirectory;
    osl_getProcessWorkingDir( &(aWorkingDirectory.pData) );
    osl_getAbsoluteFileURL( aWorkingDirectory.pData, aRet.pData, &(aRet.pData) );

    return aRet;
}

extern "C" void SAL_CALL rtl_logfile_trace  ( const char *pszFormat, ... )
{
    if( !g_bHasBeenCalled )
    {
        MutexGuard guard( getLogMutex() );
        if( ! g_bHasBeenCalled )
        {
            OUString name( RTL_CONSTASCII_USTRINGPARAM( "RTL_LOGFILE" ) );
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
                    sal_Int64 nWritten = 0;
                    if (osl_getSystemTime (&aCurrentTime))
                    {
                        nConverted = (sal_Int64 ) sprintf (
                                g_buffer,
                                "opening log file %f seconds past January 1st 1970\n"
                                "corresponding to %lu ms after timer start\n",
                                aCurrentTime.Seconds + 1e-9 * aCurrentTime.Nanosec,
                                osl_getGlobalTimer());

                        sal_Int64 nWritten;
                        if( nConverted > 0 )
                            osl_writeFile( g_aFile, g_buffer, nConverted , (sal_uInt64 *)&nWritten );
                    }

                    nConverted = sprintf (g_buffer, "Process id is %lu\n", aProcessId);
                    if( nConverted )
                        osl_writeFile( g_aFile, g_buffer, nConverted, (sal_uInt64 *)&nWritten );
                }
                else
                {
                    OSL_TRACE( "Couldn't open logfile %s(%d)" , o.getStr(), e );
                }
            }
            g_bHasBeenCalled = sal_True;
        }
    }

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
