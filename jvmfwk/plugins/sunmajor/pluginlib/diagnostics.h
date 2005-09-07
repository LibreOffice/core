/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: diagnostics.h,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:29:26 $
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

inline void jfw_ensure(bool condition, const sal_Char * pzFile, sal_Int32 line, const rtl::OUString& message)
{
    rtl::OString oMsg = rtl::OUStringToOString(message, osl_getThreadTextEncoding());
    _OSL_ENSURE(condition, pzFile, line, oMsg.getStr());
}

inline void jfw_warning2(bool condition, const sal_Char * pzFile, sal_Int32 line,
                         sal_Char * pzMessage)
{
    if (! condition)
        fprintf(stderr, "%s\n File: %s\n Line: %i", pzMessage, pzFile, line);
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
