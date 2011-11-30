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
#include "win95sys.h"
#include <tlhelp32.h>
static FARPROC WINAPI GetRealProcAddress( HMODULE hModule, LPCSTR lpProcName )
{
    FARPROC lpfn = GetProcAddress( hModule, lpProcName );

    if ( lpfn )
    {
        if ( 0x68 == *(LPBYTE)lpfn )
        {
            /*
            82C9F460 68 36 49 F8 BF       push        0BFF84936h
            82C9F465 E9 41 62 2F 3D       jmp         BFF956AB
            */

            lpfn = (FARPROC)*(LPDWORD)((LPBYTE)lpfn + 1);

            /*
            BFF956AB 9C                   pushfd
            BFF956AC FC                   cld
            BFF956AD 50                   push        eax
            BFF956AE 53                   push        ebx
            BFF956AF 52                   push        edx
            BFF956B0 64 8B 15 20 00 00 00 mov         edx,dword ptr fs:[20h]
            BFF956B7 0B D2                or          edx,edx
            BFF956B9 74 09                je          BFF956C4
            BFF956BB 8B 42 04             mov         eax,dword ptr [edx+4]
            BFF956BE 0B C0                or          eax,eax
            BFF956C0 74 07                je          BFF956C9
            BFF956C2 EB 42                jmp         BFF95706
            BFF956C4 5A                   pop         edx
            BFF956C5 5B                   pop         ebx
            BFF956C6 58                   pop         eax
            BFF956C7 9D                   popfd
            BFF956C8 C3                   ret
            */
        }
    }

    return lpfn;
}


typedef DWORD (WINAPI OBFUSCATE)( DWORD dwPTID );
typedef OBFUSCATE *LPOBFUSCATE;

static DWORD WINAPI Obfuscate( DWORD dwPTID )
{
    static LPOBFUSCATE lpfnObfuscate = NULL;

    if ( !lpfnObfuscate )
    {
        LPBYTE lpCode = (LPBYTE)GetRealProcAddress( GetModuleHandleA("KERNEL32"), "GetCurrentThreadId" );

        if ( lpCode )
        {
            /*
            GetCurrentThreadId:
            lpCode + 00 BFF84936 A1 DC 9C FC BF       mov         eax,[BFFC9CDC]    ; This is the real thread id
            lpcode + 05 BFF8493B FF 30                push        dword ptr [eax]
            lpCode + 07 BFF8493D E8 17 C5 FF FF       call        BFF80E59          ; call Obfuscate function
            lpcode + 0C BFF84942 C3                   ret
            */

            DWORD   dwOffset = *(LPDWORD)(lpCode + 0x08);

            lpfnObfuscate = (LPOBFUSCATE)(lpCode + 0x0C + dwOffset);
            /*
            Obfuscate:
            BFF80E59 A1 CC 98 FC BF       mov         eax,[BFFC98CC]
            BFF80E5E 85 C0                test        eax,eax
            BFF80E60 75 04                jne         BFF80E66
            BFF80E62 33 C0                xor         eax,eax
            BFF80E64 EB 04                jmp         BFF80E6A
            BFF80E66 33 44 24 04          xor         eax,dword ptr [esp+4]
            BFF80E6A C2 04 00             ret         4
            */
        }

    }

    return lpfnObfuscate ? lpfnObfuscate( dwPTID ) : 0;
}


EXTERN_C DWORD WINAPI GetProcessId_WINDOWS( HANDLE hProcess )
{
    if ( GetCurrentProcess() == hProcess )
        return GetCurrentProcessId();

    DWORD   dwProcessId = 0;
    PPROCESS_DATABASE   pPDB = (PPROCESS_DATABASE)Obfuscate( GetCurrentProcessId() );

    if ( pPDB && K32OBJ_PROCESS == pPDB->Type )
    {
        DWORD   dwHandleNumber = (DWORD)hProcess >> 2;

        if ( 0 == ((DWORD)hProcess & 0x03) && dwHandleNumber < pPDB->pHandleTable->cEntries )
        {
            if (
                pPDB->pHandleTable->array[dwHandleNumber].pObject &&
                K32OBJ_PROCESS == pPDB->pHandleTable->array[dwHandleNumber].pObject->Type
                )
            dwProcessId = Obfuscate( (DWORD)pPDB->pHandleTable->array[dwHandleNumber].pObject );
        }

        SetLastError( ERROR_INVALID_HANDLE );
    }

    return dwProcessId;
}


EXTERN_C DWORD WINAPI GetProcessId_NT( HANDLE hProcess )
{
    SetLastError( ERROR_CALL_NOT_IMPLEMENTED );
    return 0;
}


EXTERN_C void WINAPI ResolveThunk_GetProcessId( FARPROC *lppfn, LPCSTR lpLibFileName, LPCSTR lpFuncName )
{
    if ( (LONG)GetVersion() < 0 )
        *lppfn = (FARPROC)GetProcessId_WINDOWS;
    else
    {
        FARPROC lpfnResult = GetProcAddress( LoadLibraryA( lpLibFileName ), lpFuncName );
        if ( !lpfnResult )
            lpfnResult = (FARPROC)GetProcessId_NT;

        *lppfn = lpfnResult;
    }
}


DEFINE_CUSTOM_THUNK( kernel32, GetProcessId, DWORD, WINAPI, GetProcessId, ( HANDLE hProcess ) );
