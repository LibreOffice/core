/*************************************************************************
 *
 *  $RCSfile: config.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obr $ $Date: 2000-11-01 11:34:02 $
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

#ifndef _SAL_CONFIG_H_
#define _SAL_CONFIG_H_

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif


/** The define _USE_NO_NAMEPSACE disable the use of namespaces.
    default is that namespaces were used.
*/
#ifndef _USE_NO_NAMESPACE
#if defined(USE_NAMESPACE) || !defined(_USE_NAMESPACE)
#   define _USE_NAMESPACE
#endif
#endif

#ifdef WIN32
#define SAL_W32
#define SAL_DLLEXTENSION ".dll"

/* No warning for: C++ Exception Specification ignored */
#pragma warning( disable : 4290 )

    #if defined(_DEBUG)
    /* No warning for: identifier was truncated to '255' characters in the browser information */
    #pragma warning( disable : 4786 )
    #endif

#endif

/* BR: 16bit fuer Borland-Compiler */
#ifdef __BORLANDC__
#define SAL_W16
#define SAL_DLLEXTENSION ".dll"
#endif
/* BR: 16bit fuer Borland-Compiler */

#ifdef OS2
#define SAL_OS2
#define SAL_DLLEXTENSION ".dll"
#endif

#ifdef MAC
#define SAL_MAC
#define SAL_DLLEXTENSION ".dll"
#endif

#if defined(SOLARIS) || defined(LINUX) || defined(NETBSD) || defined(SCO)
#define SAL_UNX
#define SAL_DLLEXTENSION ".so"
#define SAL_DLLPREFIX "lib"
#endif

#ifdef MACOSX
#define SAL_UNX
#define SAL_DLLEXTENSION ".dylib.framework"
#define SAL_DLLPREFIX "lib"
#endif

#ifdef HPUX
#define SAL_UNX
#define SAL_DLLEXTENSION ".sl"
#define SAL_DLLPREFIX "lib"
#endif

#ifdef sun
#undef sun
#define sun sun
#endif

#endif /*_SAL_CONFIG_H_ */


