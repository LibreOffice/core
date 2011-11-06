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
#include "precompiled_ucb.hxx"
#include <osl/process.h>
#include "odma_provider.hxx"

#ifdef WNT
#define SOFFICE "soffice.exe"
#else
#define SOFFICE "soffice"
#endif

/** our main program to convert ODMAIDs to ODMA URLs
*/

#if (defined UNX) || (defined OS2)
void main( int argc, char * argv[] )
#else
void _cdecl main( int argc, char * argv[] )
#endif
{
    static ::rtl::OUString sProcess(RTL_CONSTASCII_USTRINGPARAM(SOFFICE));
    if(argc > 1)
    {
        ::rtl::OUString* pArguments = new ::rtl::OUString[argc-1];
        for(int i = 0; i < argc-1; ++i)
        {
            pArguments[i] = ::rtl::OUString::createFromAscii(argv[i+1]);
            if( pArguments[i].matchIgnoreAsciiCaseAsciiL(
                    RTL_CONSTASCII_STRINGPARAM(ODMA_URL_ODMAID)))
            {
                ::rtl::OUString sArgument
                      = ::rtl::OUString(
                          RTL_CONSTASCII_USTRINGPARAM(
                              ODMA_URL_SCHEME ODMA_URL_SHORT "/"));
                sArgument += pArguments[i];
                pArguments[i] = sArgument;
            }
        }

        rtl_uString ** ustrArgumentList = new rtl_uString * [argc-1];
        for (int i = 0; i < argc-1; i++)
            ustrArgumentList[i] = pArguments[i].pData;

        oslProcess  aProcess;

        if ( osl_Process_E_None == osl_executeProcess(
                 sProcess.pData,
                 ustrArgumentList,
                 argc-1,
                 osl_Process_DETACHED,
                 NULL,
                 NULL,
                 NULL,
                 0,
                 &aProcess )
        )
            osl_freeProcessHandle( aProcess );

        delete [] ustrArgumentList;
        delete [] pArguments;
    }
}

