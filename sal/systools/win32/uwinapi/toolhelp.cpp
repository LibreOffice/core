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
#include <tlhelp32.h>


IMPLEMENT_THUNK( kernel32, TRYLOAD, BOOL, WINAPI, Module32First, (HANDLE hSnapshot, LPMODULEENTRY32 lpme ) )
{
    SetLastError( ERROR_CALL_NOT_IMPLEMENTED );
    return FALSE;
}

IMPLEMENT_THUNK( kernel32, TRYLOAD, BOOL, WINAPI, Module32FirstW, (HANDLE hSnapshot, LPMODULEENTRY32W lpme ) )
{
    SetLastError( ERROR_CALL_NOT_IMPLEMENTED );
    return FALSE;
}

IMPLEMENT_THUNK( kernel32, TRYLOAD, BOOL, WINAPI, Module32Next, (HANDLE hSnapshot, LPMODULEENTRY32 lpme ) )
{
    SetLastError( ERROR_CALL_NOT_IMPLEMENTED );
    return FALSE;
}

IMPLEMENT_THUNK( kernel32, TRYLOAD, BOOL, WINAPI, Module32NextW, (HANDLE hSnapshot, LPMODULEENTRY32W lpme ) )
{
    SetLastError( ERROR_CALL_NOT_IMPLEMENTED );
    return FALSE;
}

IMPLEMENT_THUNK( kernel32, TRYLOAD, BOOL, WINAPI, Process32First, (HANDLE hSnapshot, LPPROCESSENTRY32 lppe ) )
{
    SetLastError( ERROR_CALL_NOT_IMPLEMENTED );
    return FALSE;
}

IMPLEMENT_THUNK( kernel32, TRYLOAD, BOOL, WINAPI, Process32FirstW, (HANDLE hSnapshot, LPPROCESSENTRY32W lppe ) )
{
    SetLastError( ERROR_CALL_NOT_IMPLEMENTED );
    return FALSE;
}

IMPLEMENT_THUNK( kernel32, TRYLOAD, BOOL, WINAPI, Process32Next, (HANDLE hSnapshot, LPPROCESSENTRY32 lppe ) )
{
    SetLastError( ERROR_CALL_NOT_IMPLEMENTED );
    return FALSE;
}

IMPLEMENT_THUNK( kernel32, TRYLOAD, BOOL, WINAPI, Process32NextW, (HANDLE hSnapshot, LPPROCESSENTRY32W lppe ) )
{
    SetLastError( ERROR_CALL_NOT_IMPLEMENTED );
    return FALSE;
}

IMPLEMENT_THUNK( kernel32, TRYLOAD, HANDLE, WINAPI, CreateToolhelp32Snapshot, (DWORD dwFlags, DWORD th32ProcessID ) )
{
    SetLastError( ERROR_CALL_NOT_IMPLEMENTED );
    return NULL;
}
