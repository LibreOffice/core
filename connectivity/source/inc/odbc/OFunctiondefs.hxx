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


#ifndef _CONNECTIVITY_OFUNCTIONDEFS_HXX_
#define _CONNECTIVITY_OFUNCTIONDEFS_HXX_

#if defined(WNT)

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4005)
#endif

// just to go with calling convention of windows
#define SQL_API __stdcall
#include <odbc/sqlext.h>
#undef SQL_API
#define SQL_API __stdcall

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#ifndef SQL_C_BOOKMARK
#define SQL_C_BOOKMARK   SQL_C_ULONG                     /* BOOKMARK         */
#endif

#ifndef SQL_OPT_TRACE_OFF
#define SQL_OPT_TRACE_OFF               0UL
#endif

#define SDB_ODBC_CHAR UCHAR

#endif



#ifdef UNX

// Currently, we directly use the ODBC shared library from Q+E (via the supplied lib)

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
#undef sal_Bool // Is defined in qeodbc.h, but gets redefined by solar.h

#define SDB_ODBC_CHAR UCHAR
#define SQL_WCHAR           (-8)
#define SQL_WVARCHAR        (-9)
#define SQL_WLONGVARCHAR    (-10)
#define SQL_C_WCHAR         SQL_WCHAR


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

#ifdef UNICODE
#define SQL_C_TCHAR     SQL_C_WCHAR
#else
#define SQL_C_TCHAR     SQL_C_CHAR
#endif

#endif // _CONNECTIVITY_OFUNCTIONDEFS_HXX_


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
