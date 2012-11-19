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
#ifndef _RTL_LOGFILE_H_
#define _RTL_LOGFILE_H_

#include "sal/config.h"

#include "sal/saldllapi.h"
#include "sal/types.h"

#ifdef __cplusplus
extern "C" {
#endif


/** This function allows to log arbitrary messages even in a product-environment.

    The logfile is created on first access and closed, when the sal-library gets unloaded.
    The file is line buffered. A log file is not created if no log messages are
    written.

    The first time, rtl_logfile_trace is called, it checks for the bootstrap variable
    RTL_LOGFILE. If the variable is not empty, it creates a file with the name
    $(RTL_LOGFILE)_$(PID).log, where $(PID) is the process id of the running process.

    @param pszFormat A format string with fprintf-syntax
    @param ...       An arbitrary number of arguments for fprintf, matching the
                     format string.
*/
SAL_DLLPUBLIC void SAL_CALL rtl_logfile_trace( const sal_Char* pszFormat, ... );

/** Like rtl_logfile_trace, but prefixing every log entry with the current time
    and thread ID.

    @param format
    a format string with fprintf-like syntax

    @param ...
    an arbitrary number of arguments for fprintf, matching the given format
    string

    @since UDK 3.2.0
*/
SAL_DLLPUBLIC void SAL_CALL rtl_logfile_longTrace(char const * format, ...);

/** Return if a log file is written.

    @return true if a log file is written

    @since UDK 3.2.11
*/
SAL_DLLPUBLIC sal_Bool SAL_CALL rtl_logfile_hasLogFile( void );

#ifdef __cplusplus
}
#endif

#ifdef TIMELOG
#define RTL_LOGFILE_TRACE( string )  \
             rtl_logfile_longTrace( "| : %s\n", string )
#define RTL_LOGFILE_TRACE1( frmt, arg1 ) \
             rtl_logfile_longTrace( "| : " ); \
             rtl_logfile_trace( frmt, arg1 ); \
             rtl_logfile_trace( "\n" )

#define RTL_LOGFILE_TRACE2( frmt, arg1 , arg2 ) \
             rtl_logfile_longTrace( "| : " ); \
             rtl_logfile_trace( frmt, arg1 , arg2 ); \
             rtl_logfile_trace( "\n" )
#define RTL_LOGFILE_TRACE3( frmt, arg1 , arg2 , arg3 ) \
             rtl_logfile_longTrace( "| : " ); \
             rtl_logfile_trace( frmt, arg1 , arg2 , arg3 ); \
             rtl_logfile_trace( "\n" )

//  Now the macros with project and author arguments.  The strings
//  are formatted in a way, so that the log file can be parsed by
//  post processing scripts.
#define RTL_LOGFILE_TRACE_AUTHOR( project, author, string )  \
             rtl_logfile_longTrace( "| %s (%s) : %s\n", \
                                project,\
                                author,\
                                string )
#define RTL_LOGFILE_TRACE_AUTHOR1( project, author, frmt, arg1 ) \
             rtl_logfile_longTrace( "| %s (%s) : ", \
                                project,\
                                author );\
             rtl_logfile_trace( frmt, arg1 ); \
             rtl_logfile_trace( "\n" )

#define RTL_LOGFILE_TRACE_AUTHOR2( project, author, frmt, arg1 , arg2 ) \
             rtl_logfile_longTrace( "| %s (%s) : ", \
                                project,\
                                author ); \
             rtl_logfile_trace( frmt, arg1 , arg2 ); \
             rtl_logfile_trace( "\n" )
#define RTL_LOGFILE_TRACE_AUTHOR3( project, author, frmt, arg1 , arg2 , arg3 ) \
             rtl_logfile_longTrace( "| %s (%s) : ", \
                                project,\
                                author ); \
             rtl_logfile_trace( frmt, arg1 , arg2 , arg3 ); \
             rtl_logfile_trace( "\n" )
#else
#define RTL_LOGFILE_TRACE( string )  ((void)0)
#define RTL_LOGFILE_TRACE1( frmt, arg1 ) ((void)0)
#define RTL_LOGFILE_TRACE2( frmt, arg1 , arg2 ) ((void)0)
#define RTL_LOGFILE_TRACE3( frmt, arg1 , arg2 , arg3 ) ((void)0)

#define RTL_LOGFILE_TRACE_AUTHOR( project, author, string )  ((void)0)
#define RTL_LOGFILE_TRACE_AUTHOR1( project, author, frmt, arg1 ) ((void)0)
#define RTL_LOGFILE_TRACE_AUTHOR2( project, author, frmt, arg1 , arg2 ) ((void)0)
#define RTL_LOGFILE_TRACE_AUTHOR3( project, author, frmt, arg1 , arg2 , arg3 ) ((void)0)
#endif // TIMELOG
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
