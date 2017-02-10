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

#ifndef INCLUDED_SYSTOOLS_WIN32_SNPRINTF_H
#define INCLUDED_SYSTOOLS_WIN32_SNPRINTF_H

#if     !defined(_WIN32)
#error ERROR: Only Win32 target supported!
#endif

/* Macros for Unicode/ANSI support like in TCHAR.H */

#ifdef _UNICODE
#define sntprintf   snwprintf
#define vsntprintf  vsnwprintf
#else
#define sntprintf   snprintf
#define vsntprintf  vsnprintf
#endif

/* Define needed types if they are not yet defined */


#   ifndef _VA_LIST_DEFINED
    typedef char *  va_list;
#   define _VA_LIST_DEFINED
#   endif


#   ifndef _WCHAR_T_DEFINED
    typedef unsigned short wchar_t;
#   define _WCHAR_T_DEFINED
#   endif


#ifndef LO_SNPRINTF_DLLIMPORT
#define LO_SNPRINTF_DLLIMPORT __declspec( dllimport )
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*  Implementations of snprintf following the ISO/IEC 9899:1999 (ISO C99)
    standard.
    The difference compared to _snprintf is that the buffer always is zero
    terminated (unless count is zero) and the return value is the number of
    characters (not including terminating zero) that would have been written
    even if the buffer wasn't large
    enough to hold the string. */

/* VS 2015 and above support ISO C snprintf */
#if _MSC_VER < 1900

/* UNICODE version */
LO_SNPRINTF_DLLIMPORT int __cdecl snwprintf( wchar_t *buffer, size_t count, const wchar_t *format, ... );

/* SBCS and MBCS version */
LO_SNPRINTF_DLLIMPORT int __cdecl snprintf( char *buffer, size_t count, const char *format, ... );

#endif

/* Conflict with STL_port inline implementation */

#ifdef __cplusplus
}
#endif

#endif // INCLUDED_SYSTOOLS_WIN32_SNPRINTF_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
