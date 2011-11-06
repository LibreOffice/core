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
#include "precompiled_extensions.hxx"
#if defined _MSC_VER && ( _MSC_VER >= 1200 )
#define _WIN32_WINNT  0x0400
#endif

#include "ole2uno.hxx"

#include <tools/presys.h>
#include <tools/postsys.h>

#include <vos/thread.hxx>

using namespace vos;
using namespace std;

namespace ole_adapter
{
// CoInitializeEx *
typedef DECLSPEC_IMPORT HRESULT (STDAPICALLTYPE *ptrCoInitEx)( LPVOID, DWORD);
// CoInitialize *
typedef DECLSPEC_IMPORT HRESULT (STDAPICALLTYPE *ptrCoInit)( LPVOID);

void o2u_attachCurrentThread()
{
    static OThreadData oleThreadData;

    if ((sal_Bool)(sal_IntPtr)oleThreadData.getData() != sal_True)
    {
        HINSTANCE inst= LoadLibrary( _T("ole32.dll"));
        if( inst )
        {
            HRESULT hr;
            // Try DCOM
            ptrCoInitEx initFuncEx= (ptrCoInitEx)GetProcAddress( inst, _T("CoInitializeEx"));
            if( initFuncEx)
                hr= initFuncEx( NULL, COINIT_MULTITHREADED);
            // No DCOM, try COM
            else
            {
                ptrCoInit initFunc= (ptrCoInit)GetProcAddress( inst,_T("CoInitialize"));
                if( initFunc)
                    hr= initFunc( NULL);
            }
        }
        oleThreadData.setData((void*)sal_True);
    }
}

} // end namespace
