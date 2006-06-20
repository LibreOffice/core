/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: config.h,v $
 *
 *  $Revision: 1.21 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 04:15:28 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

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

/* Provide ISO C99 compatible versions of snprint and vsnprintf */
#ifndef _SNPRINTF_H
#include <systools/win32/snprintf.h>
#endif

#endif /* defined _MSC_VER */
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
#define SAL_CONFIGFILE( name ) name ".ini"
#define SAL_SYSCONFIGFILE( name ) name ".ini"
#endif

#ifdef MAC
#define SAL_MAC
/* don't know what to put in here */
#endif

#if defined(SOLARIS) || defined(LINUX) || defined(NETBSD) || defined(FREEBSD) || defined(SCO) || defined(IRIX)
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


