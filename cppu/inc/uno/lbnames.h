/*************************************************************************
 *
 *  $RCSfile: lbnames.h,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: obo $ $Date: 2003-09-04 10:52:15 $
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
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _UNO_LBNAMES_H_
#define _UNO_LBNAMES_H_

#ifdef __cplusplus

/* environment type names for supported compilers */
#if defined (_MSC_VER)
#if (_MSC_VER < 1000)
#error "ms visual c++ version must be at least 4.2"
#elif (_MSC_VER < 1100) // MSVC 4.x
#pragma warning( disable: 4290 )
#define TMP_CPPU_ENV msci
#elif (_MSC_VER < 1400) // MSVC 5-7
#define TMP_CPPU_ENV msci
#else
#error "ms visual c++ version must be between 4.2 and 7.x"
#endif /* (_MSC_VER < 1000) */
/* sunpro cc */
#elif defined (__SUNPRO_CC)
#if (__SUNPRO_CC < 0x500)
#error "sunpro cc must be at least 5.x"
provoking error here, because PP ignores #error
#elif (__SUNPRO_CC < 0x600)
#define TMP_CPPU_ENV sunpro5
#else
#error "sunpro cc version must be 5.x"
provoking error here, because PP ignores #error
#endif /* defined (__SUNPRO_CC) */
/* g++ 2.x, 3.0 */
#elif defined (__GNUC__)
#if (__GNUC__ == 2 && __GNUC_MINOR__ == 91)
#define TMP_CPPU_ENV gcc2
#elif (__GNUC__ == 2 && __GNUC_MINOR__ == 95)
#define TMP_CPPU_ENV gcc2
#elif (__GNUC__ == 2)
#error "Tested gcc 2 versions are 2.91 and 2.95.  Patch uno/lbnames.h to try your gcc 2 version."
#elif (__GNUC__ == 3 && __GNUC_MINOR__ <= 3)
#define __CPPU_ENV gcc3
#elif (__GNUC__ == 3)
#error "Tested gcc 3 version is <= 3.3.  Patch uno/lbnames.h to try your gcc 3 version."
#else
#error "Supported gcc majors are 2 and 3.  Unsupported gcc major version."
#endif /* defined (__GNUC__) */
#endif /* defined (_MSC_VER) */

#if (! defined (CPPU_ENV) && defined (TMP_CPPU_ENV))
#define CPPU_ENV TMP_CPPU_ENV
#endif

#ifdef CPPU_ENV
/* test: whether given CPPU_ENV matches expected one */
#if (CPPU_ENV != TMP_CPPU_ENV)
#error "CPPU_ENV: unexpected env!"
provoking error here, because PP ignores #error
#endif

#define CPPU_STRINGIFY_EX( x ) #x
#define CPPU_STRINGIFY( x ) CPPU_STRINGIFY_EX( x )
/** Name for C++ compiler/ platform, e.g. "gcc3", "msci" */
#define CPPU_CURRENT_LANGUAGE_BINDING_NAME CPPU_STRINGIFY( CPPU_ENV )

#else
#error "No supported C++ compiler environment."
provoking error here, because PP ignores #error
#endif /* CPPU_ENV */

#endif /** __cplusplus */

/** Environment type name for binary C UNO. */
#define UNO_LB_UNO "uno"
/** Environment type name for ANSI C compilers. */
#define UNO_LB_C "c"
/** Environment type name for Java 1.3.1 compatible virtual machine. */
#define UNO_LB_JAVA "java"
/** Environment type name for CLI (Common Language Infrastructure). */
#define UNO_LB_CLI "cli"

#endif
