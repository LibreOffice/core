/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef _RTL_LOGFILE_HXX_
#define _RTL_LOGFILE_HXX_

#include <rtl/logfile.h>
#include <rtl/string.hxx>

namespace rtl
{
/**
@descr  The intended use for class Logfile is to write time stamp information
        for profiling purposes.

        Profiling output should only be generated for a special product version of OpenOffice
        which is compiled with a defined preprocessor symbol 'TIMELOG'.
        Therefore we have provided a set of macros that uses the class Logfile only if
        this symbol is defined.  If the macros are not sufficient, i.e. you need more
        then three arguments for a printf style message, then you have to insert an
        \#ifdef TIMELOG/\#endif brace yourself.

        Additionally the environment variable RTL_LOGFILE has to be defined in order to generate
        logging information. If the variable is not empty, it creates a file with the name
        $(RTL_LOGFILE)_$(PID).log, where $(PID) is the process id of the running process.
        It can be used as a run time switch for enabling or disabling the logging.
        Note that this variable is evaluated only once at the first attempt to write a message.

        The class LogFile collects runtime data within its constructor and destructor. It can be
        used for timing whole functions.
        If you want to write timing data without context you can use the RTL_LOGFILE_TRACE-macros
        which are defined inside <rtl/logfile.h>.

        The class LogFile should not be used directly, instead use the RTL_LOGFILE_CONTEXT/
        RTL_LOGFILE_TRACE-macros.

        Macro usage:
        ------------
        RTL_LOGFILE_CONTEXT( instance, name );
        This macro creates an instance of class LogFile with the name "instance" and writes the current time,
        thread id and "name" to the log file.

        Example: RTL_LOGFILE_CONTEXT( aLog, "Timing for foo-method" );

        RTL_LOGFILE_CONTEXT_TRACE( instance, mesage );
        RTL_LOGFILE_CONTEXT_TRACEn( instance, frmt, arg1, .., arg3 );
        These macros can be used to log information in a "instance" context. The "instance" object
        is used to log message informations. All macros with "frmt" uses printf notation to log timing infos.

        Example: RTL_LOGFILE_CONTEXT_TRACE( aLog, "Now we call an expensive function" );
                 RTL_LOGFIlE_CONTEXT_TRACE1( aLog, "Config entries read: %u", (unsigned short)i );

        RTL_LOGFILE_TRACE( string );
        RTL_LOGFILE_TRACEn( frmt, arg1, .., arg3 );
        These macros can be used to log information outside a context. The macro directly calls
        rtl_logfile_trace to write the info to the log file. All macros with "frmt" uses printf
        notation to log timing infos.

        Example: RTL_LOGFILE_TRACE( "Timing for loading a file" );
                 RTL_LOGFILE_TRACE1( aLog, "Timing for loading file: %s", aFileName );

        The lines written to the log file consist of the following space separated elements:
        1.  The time relative to the start of the global timer in milliseconds.  The times is
            started typically for the first logged line.
        2.  Thread id.  It's absolut value is probably of less interest than providing a way to
            distinguish different threads.
        3.  a.  An opening or closing curly brace indicating the start or end of a scope.
                4a. Function name or general scope identifier.
            b.  A vertical line indicating an arbitrary message.
                4b optional function name or general scope identifier.
                5b A colon followed by a space and a free form message terminated by a newline.

        There is a second version of creating a context. RTL_LOGFILE_CONTEXT_AUTHOR takes
        two more arguments, the name of the project and the author's sign who is responsible
        for the code in which the macro is used.
*/
    class Logfile
    {
    public:
        inline Logfile( const sal_Char *name );
        /** @descr  Create a log file context where the message field consists of a project
                name, the author's shortcut, and the actual message.  These three strings
                are written in a format that is understood by script that later parses the
                log file and that so can extract the three strings.
            @param  project Short name of the project, like sw for writer or sc for calc.
            @param  author  The sign of the person responsible for the code.
            @param  name    The actual message, typically a method name.
        */
        inline Logfile( const sal_Char *project, const sal_Char *author, const sal_Char *name );
        inline ~Logfile();
        inline const sal_Char *getName();
    private:
        ::rtl::OString m_sName;
    };

    inline Logfile::Logfile( const sal_Char *name )
        : m_sName( name )
    {
        rtl_logfile_longTrace( "{ %s\n", name );
    }

    inline Logfile::Logfile( const sal_Char *project, const sal_Char *author, const sal_Char *name )
        : m_sName( project)
    {
        m_sName += " (";
        m_sName += author;
        m_sName += ") ";
        m_sName += name;
        rtl_logfile_longTrace( "{ %s\n", m_sName.pData->buffer );
    }

    inline Logfile::~Logfile()
    {
        rtl_logfile_longTrace( "} %s\n", m_sName.pData->buffer );
    }

    inline const sal_Char * Logfile::getName()
    {
        return m_sName.getStr();
    }
}

#ifdef TIMELOG
#define RTL_LOGFILE_CONTEXT( instance, name ) ::rtl::Logfile instance( name )
#define RTL_LOGFILE_CONTEXT_AUTHOR( instance, project, author, name ) ::rtl::Logfile instance(project, author, name )
#define RTL_LOGFILE_CONTEXT_TRACE( instance, message ) \
        rtl_logfile_longTrace( "| %s : %s\n", \
                           instance.getName(), \
                           message )
#define RTL_LOGFILE_CONTEXT_TRACE1( instance , frmt, arg1 ) \
        rtl_logfile_longTrace( "| %s : ", \
                           instance.getName() ); \
        rtl_logfile_trace( frmt , arg1 ); \
        rtl_logfile_trace( "\n" )
#define RTL_LOGFILE_CONTEXT_TRACE2( instance , frmt, arg1 , arg2 ) \
        rtl_logfile_longTrace( "| %s : ", \
                           instance.getName() ); \
        rtl_logfile_trace( frmt , arg1 , arg2 ); \
        rtl_logfile_trace( "\n" )
#define RTL_LOGFILE_CONTEXT_TRACE3( instance , frmt, arg1 , arg2 , arg3 ) \
        rtl_logfile_longTrace( "| %s : ", \
                           instance.getName() ); \
        rtl_logfile_trace( frmt , arg1 , arg2 , arg3 ); \
        rtl_logfile_trace( "\n" )

#else
#define RTL_LOGFILE_CONTEXT( instance, name )  ((void)0)
#define RTL_LOGFILE_CONTEXT_AUTHOR( instance, project, author, name )  ((void)0)
#define RTL_LOGFILE_CONTEXT_TRACE( instance, message )  ((void)0)
#define RTL_LOGFILE_CONTEXT_TRACE1( instance, frmt, arg1 ) ((void)arg1,(void)0)
#define RTL_LOGFILE_CONTEXT_TRACE2( instance, frmt, arg1, arg2 ) ((void)arg1,(void)arg2,(void)0)
#define RTL_LOGFILE_CONTEXT_TRACE3( instance, frmt, arg1, arg2 , arg3 ) ((void)arg1,(void)arg2,(void)arg3,(void)0)
#endif

// Normal RTL_LOGFILE_* entries will not make it into release versions,
// TIMELOG is disabled a few versions prior relase build.
//
// We need some logs also in these builds, eg. for making performance regression tests.
//
// POLICY: Don't use RTL_LOGFILE_PRODUCT_* for your personal logging information.
//         Be aware that these logs make it into the product shipped to customers.
//         If you have good reasons for doing this, please contact product management.

#define RTL_LOGFILE_PRODUCT_TRACE( string )  \
            rtl_logfile_longTrace( "| : %s\n", string )
#define RTL_LOGFILE_PRODUCT_TRACE1( frmt, arg1 ) \
             rtl_logfile_longTrace( "| : " ); \
             rtl_logfile_trace( frmt, arg1 ); \
             rtl_logfile_trace( "\n" )
#define RTL_LOGFILE_PRODUCT_CONTEXT( instance, name ) \
            ::rtl::Logfile instance( name )
#define RTL_LOGFILE_PRODUCT_CONTEXT_TRACE1( instance, frmt, arg1 ) \
        rtl_logfile_longTrace( "| %s : ", \
                           instance.getName() ); \
             rtl_logfile_trace( frmt, arg1 ); \
             rtl_logfile_trace( "\n" )
#define RTL_LOGFILE_HASLOGFILE() \
             rtl_logfile_hasLogFile()


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
