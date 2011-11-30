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




#include "macros.h"

EXTERN_C DWORD WINAPI GetUserDomainA_NT( LPSTR lpBuffer, DWORD nSize )
#include "GetUserDomain_NT.cpp"

EXTERN_C DWORD WINAPI GetUserDomainA_WINDOWS( LPSTR lpBuffer, DWORD nSize )
#include "GetUserDomain_WINDOWS.cpp"

EXTERN_C void WINAPI ResolveThunk_GetUserDomainA( FARPROC *lppfn, LPCSTR lpLibFileName, LPCSTR lpFuncName )
{
    if ( (LONG)GetVersion() < 0 )
        *lppfn = (FARPROC)GetUserDomainA_WINDOWS;
    else
        *lppfn = (FARPROC)GetUserDomainA_NT;
}

DEFINE_CUSTOM_THUNK( kernel32, GetUserDomainA, DWORD, WINAPI, GetUserDomainA, ( LPSTR lpBuffer, DWORD nSize ) );

