/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/
#ifndef _SNPRINTF_H
#define _SNPRINTF_H

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

#if 0
#   ifndef _INC_STDARG
#   include <stdarg.h>
#   endif
#else
#   ifndef _VA_LIST_DEFINED
    typedef char *  va_list;
#   define _VA_LIST_DEFINED
#   endif
#endif

#if 0
#   ifndef _INC_WCHAR
#   include <wchar.h>
#   endif
#else
#   ifndef _WCHAR_T_DEFINED
    typedef unsigned short wchar_t;
#   define _WCHAR_T_DEFINED
#   endif
#endif

#ifndef _SNPRINTF_DLLIMPORT
#define _SNPRINTF_DLLIMPORT __declspec( dllimport )
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



/* UNICODE version */
_SNPRINTF_DLLIMPORT int __cdecl snwprintf( wchar_t *buffer, size_t count, const wchar_t *format, ... );

/* SBCS and MBCS version */
_SNPRINTF_DLLIMPORT int __cdecl snprintf( char *buffer, size_t count, const char *format, ... );

/* Conflict with STL_port inline implementation */
#if 0
/* UNICODE version */
_SNPRINTF_DLLIMPORT int __cdecl vsnwprintf( wchar_t *buffer, size_t count, const wchar_t *format, va_list ap );

/* SBCS and MBCS version */
_SNPRINTF_DLLIMPORT int __cdecl vsnprintf( char *buffer, size_t count, const char *format, va_list ap );
#endif

#ifdef __cplusplus
}
#endif

#endif /* _SNPRINTF_H */
