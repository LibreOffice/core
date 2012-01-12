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
