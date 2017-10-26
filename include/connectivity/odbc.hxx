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


#ifndef INCLUDED_CONNECTIVITY_SOURCE_INC_ODBC_OFUNCTIONDEFS_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_INC_ODBC_OFUNCTIONDEFS_HXX

#if defined(_WIN32)

#include <prewin.h>

// just to go with calling convention of windows
#if SYSTEM_ODBC_HEADERS
#if !defined WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#define SQL_API __stdcall
#include <sqlext.h>
#else
#define SQL_API __stdcall
#include <odbc/sqlext.h>
#endif
#undef SQL_API
#define SQL_API __stdcall

#ifndef SQL_C_BOOKMARK
#define SQL_C_BOOKMARK   SQL_C_ULONG                     /* BOOKMARK         */
#endif

#ifndef SQL_OPT_TRACE_OFF
#define SQL_OPT_TRACE_OFF               0UL
#endif

#define SDB_ODBC_CHAR UCHAR

#include <postwin.h>

#endif


#ifdef UNX

#ifndef ODBC_UNX
#define ODBC_UNX
#endif
#define CALLBACK
#define EXPORT
#ifdef SYSTEM_ODBC_HEADERS
#include <sqlext.h>
#else
#include <odbc/sqlext.h>
#endif

#define SDB_ODBC_CHAR UCHAR

#endif // UNX


#ifndef SQL_WCHAR
#define SQL_WCHAR           (-8)
#endif
#ifndef SQL_WVARCHAR
#define SQL_WVARCHAR        (-9)
#endif
#ifndef SQL_WLONGVARCHAR
#define SQL_WLONGVARCHAR    (-10)
#endif
#ifndef SQL_C_WCHAR
#define SQL_C_WCHAR         SQL_WCHAR
#endif

#ifndef SQL_C_TCHAR
#ifdef UNICODE
#define SQL_C_TCHAR     SQL_C_WCHAR
#else
#define SQL_C_TCHAR     SQL_C_CHAR
#endif
#endif

#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_ODBC_OFUNCTIONDEFS_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
