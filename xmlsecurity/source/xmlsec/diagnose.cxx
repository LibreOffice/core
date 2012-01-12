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



#include "diagnose.hxx"
#include <stdio.h>
#include <stdarg.h>
#include "rtl/instance.hxx"
#include "rtl/bootstrap.hxx"

namespace xmlsecurity {

struct UseDiagnose : public rtl::StaticWithInit<
    const bool, UseDiagnose>
{
    bool operator () () const
    {
        ::rtl::OUString value;
        sal_Bool res = rtl::Bootstrap::get(
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("XMLSECURITY_TRACE")), value);
        return res == sal_True ? true : false;
    }
};

/* the function will print the string when
   - build with debug
   - the bootstrap variable XMLSECURITY_TRACE is set.
 */
void xmlsec_trace(const char* pszFormat, ...)
{
    bool bDebug = false;

#if OSL_DEBUG_LEVEL > 1
    bDebug = true;
#endif
    if (bDebug || UseDiagnose::get())
    {
        va_list args;
        fprintf(stderr, "[xmlsecurity] ");
        va_start(args, pszFormat);
        vfprintf(stderr, pszFormat, args);
        va_end(args);

        fprintf(stderr,"\n");
        fflush(stderr);
    }
}



}
