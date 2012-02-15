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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sal.hxx"


#include "rtl/ustring.hxx"
#include "osl/time.h"
#include <stdio.h>


/*
 *   mfe : maybe it would be wishful to include initialization
 *         of the global timer in dllmain or _init directly.
 *         But noneoftheless this (should) work too.
 */
namespace osl
{

class OGlobalTimer
{

public:

    OGlobalTimer() {
        getTime();
    }

    sal_uInt32 getTime()
    {
        return osl_getGlobalTimer();
    }


};

static OGlobalTimer aGlobalTimer;

}


extern "C"
{
void debug_ustring(rtl_uString* ustr)
{
    sal_Char* psz=0;
    rtl_String* str=0;

    if ( ustr != 0 )
    {
        rtl_uString2String( &str,
                            rtl_uString_getStr(ustr),
                            rtl_uString_getLength(ustr),
                            RTL_TEXTENCODING_UTF8,
                            OUSTRING_TO_OSTRING_CVTFLAGS );

        psz = rtl_string_getStr(str);
    }

    fprintf(stderr,"'%s'\n",psz);

    if ( str != 0 )
    {
        rtl_string_release(str);
    }

    return;
}

}

void debug_oustring(rtl::OUString& ustr)
{

    debug_ustring(ustr.pData);

    return;
}
