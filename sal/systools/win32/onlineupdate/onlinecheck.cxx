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
#ifdef _MSC_VER
#pragma warning( disable: 4668 )
#endif

#include <sal/types.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wininet.h>

#ifdef UNICODE
#define _UNICODE
#endif
#include <tchar.h>
#ifdef __MINGW32__
#include <excpt.h>
#endif

#define elementsof(a) (sizeof(a)/sizeof((a)[0]))

// #i71984
extern "C" sal_Bool SAL_CALL hasInternetConnection()
{
    DWORD   dwFlags;
    TCHAR   szConnectionName[1024];

#ifdef __MINGW32__
        jmp_buf jmpbuf;
        __SEHandler han;
        if (__builtin_setjmp(jmpbuf) == 0)
        {
        han.Set(jmpbuf, NULL, (__SEHandler::PF)EXCEPTION_EXECUTE_HANDLER);
#else
    __try {
#endif
    BOOL fIsConnected = InternetGetConnectedStateEx(
        &dwFlags,
        szConnectionName,
        elementsof(szConnectionName),
        0 );

    return fIsConnected ? sal_True : sal_False;

#ifdef __MINGW32__
        }
        else return sal_False;
        han.Reset();
#else
    } __except( EXCEPTION_EXECUTE_HANDLER ) {
        return sal_False;
    }
#endif
}
