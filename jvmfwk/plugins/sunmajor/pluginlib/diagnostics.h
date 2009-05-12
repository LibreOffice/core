/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: diagnostics.h,v $
 * $Revision: 1.7 $
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
#if !defined INCLUDED_JFW_PLUGIN_DIAGNOSTICS_HXX
#define INCLUDED_JFW_PLUGIN_DIAGNOSTICS_HXX
#include "osl/diagnose.h"
#include "rtl/ustring.hxx"
#include <stdio.h>

#if OSL_DEBUG_LEVEL >= 1
#define JFW_ENSURE(c, m)  _JFW_ENSURE(c, OSL_THIS_FILE, __LINE__,  m)
#else
#define JFW_ENSURE(c, m)  ((void) 0)
#endif

#if OSL_DEBUG_LEVEL >= 2
#define JFW_WARNING2(c, m)  _JFW_WARNING2(c, OSL_THIS_FILE, __LINE__, m)
#else
#define JFW_WARNING2(c, m)  ((void) 0)
#endif


#if OSL_DEBUG_LEVEL >= 0
#define JFW_TRACE0(m)  jfw_trace(m)
#else
#define JFW_TRACE0(m)  ((void) 0)
#endif

#if OSL_DEBUG_LEVEL >= 1
#define JFW_TRACE1(m)  jfw_trace(m)
#else
#define JFW_TRACE1(m)  ((void) 0)
#endif

#if OSL_DEBUG_LEVEL >= 2
#define JFW_TRACE2(m)  jfw_trace(m)
#else
#define JFW_TRACE2(m)  ((void) 0)
#endif



#define _JFW_ENSURE(c, f, l, m)  jfw_ensure(c, f, l, m)
#define _JFW_WARNING(c, f, l, m)  jfw_warning2(c, f, l, m);


namespace jfw_plugin
{

inline void jfw_ensure(bool
                       #if OSL_DEBUG_LEVEL > 0 /* prevent warning in pro version */
                       condition
                       #endif
                       , const sal_Char *
                       #if OSL_DEBUG_LEVEL > 0 /* prevent warning in pro version */
                       pzFile
                       #endif
                       , sal_Int32
                       #if OSL_DEBUG_LEVEL > 0 /* prevent warning in pro version */
                       line
                       #endif
                       , const rtl::OUString& message )
{
    rtl::OString oMsg = rtl::OUStringToOString(message, osl_getThreadTextEncoding());
    _OSL_ENSURE(condition, pzFile, line, oMsg.getStr());
}

inline void jfw_warning2(bool condition, const sal_Char * pzFile, sal_Int32 line,
                         sal_Char * pzMessage)
{
    if (! condition)
        fprintf(
            stderr, "%s\n File: %s\n Line: %ld", pzMessage, pzFile,
            sal::static_int_cast< unsigned long >(line));
}

inline void jfw_trace(rtl::OUString message)
{
    rtl::OString oMsg = rtl::OUStringToOString(message, osl_getThreadTextEncoding());
    fprintf(stderr,"%s", oMsg.getStr());
}

inline void jfw_trace(const sal_Char * pzMessage)
{
    if (pzMessage)
        fprintf(stderr,"%s", pzMessage);
}

inline void jfw_trace(const rtl::OString& message)
{
    if (message.getLength() > 0)
        fprintf(stderr,"%s", message.getStr());
}

}

#endif
