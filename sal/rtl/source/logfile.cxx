/*************************************************************************
 *
 *  $RCSfile: logfile.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jbu $ $Date: 2001-07-06 09:54:20 $
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
#include <stdio.h>
#include <stdarg.h>

#include<rtl/logfile.h>

#ifndef _OSL_PROCESS_H_
#include <osl/process.h>
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
#include <rtl/strbuf.hxx>
#endif

using namespace osl;

namespace rtl
{
static FILE *g_pFile = 0;
static sal_Bool g_bHasBeenCalled = sal_False;

class   LoggerGuard
{
public:
    ~LoggerGuard();
};

LoggerGuard::~LoggerGuard()
{
    if( g_pFile )
    {
        fprintf( g_pFile, "closing log file at %06lu", osl_getGlobalTimer() );
        fclose( g_pFile );
        g_pFile = 0;
        g_bHasBeenCalled = sal_False;
    }
}

static LoggerGuard guard;
}
using namespace rtl;

static Mutex & getInitMutex()
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

extern "C" void SAL_CALL rtl_logfile_trace  ( const char *pszFormat, ... )
{
    if( !g_bHasBeenCalled )
    {
        MutexGuard guard( getInitMutex() );
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
                OStringBuffer buf( 128 );
                buf.append( OUStringToOString( value, RTL_TEXTENCODING_ASCII_US ) );
                buf.append( "_" );
                buf.append( (sal_Int32) aProcessId );
                buf.append( ".log" );
                OString o = buf.makeStringAndClear();

                g_pFile = fopen( o.getStr() , "w" );
                if( g_pFile )
                {
                    setvbuf( g_pFile, 0 , _IOLBF , 0 );  // set line buffering
                    TimeValue aCurrentTime;
                    if (osl_getSystemTime (&aCurrentTime))
                        fprintf (g_pFile, "opening log file %f seconds past January 1st 1970\n"
                                 "corresponding to %lu ms after timer start\n",
                                 aCurrentTime.Seconds + 1e-9 * aCurrentTime.Nanosec,
                                 osl_getGlobalTimer());
                    fprintf (g_pFile, "Process id is %lu\n", aProcessId);
                }
                else
                {
                    OSL_TRACE( "Couldn't open logfile %s" , o.getStr() );
                }
            }
            g_bHasBeenCalled = sal_True;
        }
    }

    if( g_pFile )
    {
        va_list args;
        va_start(args, pszFormat);
        vfprintf(g_pFile,pszFormat, args);
        va_end(args);
    }
}
