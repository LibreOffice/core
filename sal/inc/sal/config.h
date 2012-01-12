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



#ifndef _SAL_CONFIG_H_
#define _SAL_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#ifndef INCLUDED_STDLIB_H
#include <stdlib.h>
#define INCLUDED_STDLIB_H
#endif

#ifdef WIN32
#define SAL_W32
#define SAL_DLLEXTENSION ".dll"
#define SAL_PRGEXTENSION ".exe"
#define SAL_PATHSEPARATOR ';'
#define SAL_PATHDELIMITER '\\'
#define SAL_CONFIGFILE( name ) name ".ini"
#define SAL_SYSCONFIGFILE( name ) name ".ini"

#ifdef _MSC_VER

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES // needed by Visual C++ for math constants
#endif

/* warnings ought to be disabled using /wd:n option of msvc.net compiler */
#if _MSC_VER < 1300
/* no performance warning: int to bool */
#pragma warning( disable : 4800 )

/* No warning for: C++ Exception Specification ignored */
#pragma warning( disable : 4290 )

#if OSL_DEBUG_LEVEL > 0
/* No warning for: identifier was truncated to '255' characters in the browser information */
#pragma warning( disable : 4786 )
#endif
#endif /* defined _MSC_VER < 1300 */

#endif /* defined _MSC_VER */

/* Provide ISO C99 compatible versions of snprint and vsnprintf */
#ifdef __MINGW32__
#define _SNPRINTF_DLLIMPORT
#endif
#ifndef _SNPRINTF_H
#include <systools/win32/snprintf.h>
#endif

#endif /* defined WIN32 */

/* BR: 16bit fuer Borland-Compiler */
#ifdef __BORLANDC__
#define SAL_W16
#define SAL_DLLEXTENSION ".dll"
#endif
/* BR: 16bit fuer Borland-Compiler */

#ifdef OS2
#define SAL_OS2
#define SAL_DLLEXTENSION ".dll"
#define SAL_PRGEXTENSION ".exe"
#define SAL_PATHSEPARATOR ';'
#define SAL_PATHDELIMITER '/'
#define SAL_CONFIGFILE( name ) name ".ini"
#define SAL_SYSCONFIGFILE( name ) name ".ini"
#endif

#if defined(SOLARIS) || defined(LINUX) || defined(NETBSD) || defined(FREEBSD) || defined(SCO)
#define SAL_UNX
#define SAL_DLLEXTENSION ".so"
#define SAL_DLLPREFIX "lib"
#define SAL_PRGEXTENSION ".bin"
#define SAL_PATHSEPARATOR ':'
#define SAL_PATHDELIMITER '/'
#define SAL_CONFIGFILE( name ) name "rc"
#define SAL_SYSCONFIGFILE( name ) "." name "rc"
#endif

#ifdef MACOSX
#define SAL_UNX
#define SAL_DLLEXTENSION ".dylib"
#define SAL_DLLPREFIX "lib"
#define SAL_PRGEXTENSION ".bin"
#define SAL_PATHSEPARATOR ':'
#define SAL_PATHDELIMITER '/'
#define SAL_CONFIGFILE( name ) name "rc"
#define SAL_SYSCONFIGFILE( name ) "." name "rc"
#endif

#ifdef HPUX
#define SAL_UNX
#define SAL_DLLEXTENSION ".sl"
#define SAL_DLLPREFIX "lib"
#define SAL_PRGEXTENSION ".bin"
#define SAL_CONFIGFILE( name ) name "rc"
#define SAL_SYSCONFIGFILE( name ) "." name "rc"
#endif

#ifdef sun
#undef sun
#define sun sun
#endif

/* This is to work around a gcc 3.3 error that fixing actually breaks other
 * compilers.  This will create a dummy variable specifically for gcc 3.3 that
 * allows it to compile and not break the others.  Other compilers may follow
 * with this eror later. */
#if defined __GNUC__
#if ((__GNUC__ == 3) && (__GNUC_MINOR__ > 2))
#  define SAL_ISO_CONST const
#else
#  define SAL_ISO_CONST
#endif
#endif

#endif /*_SAL_CONFIG_H_ */


