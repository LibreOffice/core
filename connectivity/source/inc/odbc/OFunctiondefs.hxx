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

//--------------------------------------------------------------------------
#ifndef _CONNECTIVITY_OFUNCTIONDEFS_HXX_
#define _CONNECTIVITY_OFUNCTIONDEFS_HXX_

#if defined(WIN) || defined(WNT)

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

//--------------------------------------------------------------------------

#ifdef OS2
#define ALLREADY_HAVE_OS2_TYPES
#define DONT_TD_VOID
#include <svpm.h>
#include <odbc/sqlext.h>
#define SDB_ODBC_CHAR UCHAR
#endif // OS2

#ifdef OS2__00

#ifdef ODBCIMP

// Stub-Version: dynamische Bindung an die DLL zur Laufzeit.
// odbcstub definiert die in den Quellen benutzten NSQL...-Methoden
// als indirekte Funktionsaufrufe.
// odbcimp zieht sich selbst preos2, odbc und postos2 an.
//  #include "odbc3imp.hxx"

#else

// Zur Zeit verwenden wir die ODBC-DLL von Watcom-SQL direkt (ueber die
// mitgelieferte Lib).

#ifndef ODBC_OS2
#define ODBC_OS2
#endif

#include <svpm.h>
#include <odbc.h>
#define SQL_API __syscall
#ifndef SQL_MAX_MESSAGE_LENGTH
#define SQL_MAX_MESSAGE_LENGTH MAX_MESSAGE_LENGTH
#endif
#ifndef SQL_MAX_DSN_LENGTH
#define SQL_MAX_DSN_LENGTH MAX_DSN_LENGTH
#endif
#ifndef SQL_AUTOCOMMIT_ON
#define SQL_AUTOCOMMIT_ON 1UL
#endif
#ifndef SQL_AUTOCOMMIT_OFF
#define SQL_AUTOCOMMIT_OFF 0UL
#endif

#define SQL_FETCH_PRIOR SQL_FETCH_PREV
#define SQL_NO_TOTAL (-4)

#endif

// In der ODBC.H von Watcom werden Strings als char * erwartet
// (nicht, wie sonst bei ODBC ueblich, als UCHAR *).
#if defined( ICC )
#define SDB_ODBC_CHAR unsigned char
#else
#define SDB_ODBC_CHAR char
#endif

#endif

//--------------------------------------------------------------------------

#ifdef UNX

// Zur Zeit verwenden wir die ODBC-shared library von Q+E direkt (ueber die
// mitgelieferte Lib).

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
#undef sal_Bool // Ist in qeodbc.h definiert, wird aber von solar.h noch einmal
            // definiert.

#define SDB_ODBC_CHAR UCHAR
#define SQL_WCHAR           (-8)
#define SQL_WVARCHAR        (-9)
#define SQL_WLONGVARCHAR    (-10)
#define SQL_C_WCHAR         SQL_WCHAR


#endif // UNX

//--------------------------------------------------------------------------

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
