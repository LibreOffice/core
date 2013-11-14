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

#ifndef __ACCACT_HXX
#define __ACCACT_HXX

#include <windows.h>
#include <tchar.h>


static bool IsXPOrLater()
{
    OSVERSIONINFO osvi;

    ZeroMemory(&osvi, sizeof(OSVERSIONINFO));

    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    GetVersionEx(&osvi);
    return ((osvi.dwMajorVersion > 5) ||
        ((osvi.dwMajorVersion == 5) && (osvi.dwMinorVersion >= 1)));
}

static HANDLE pActCtx = INVALID_HANDLE_VALUE;
static ULONG_PTR lpCookie;

static bool ActivateActContext()
{
    if(!IsXPOrLater())
        return false;

    ACTCTX actctx;

    ZeroMemory(&actctx, sizeof(actctx));

    actctx.cbSize = sizeof(actctx);

    TCHAR szDllDirPath[1024];
    ::GetCurrentDirectory(1024,szDllDirPath);
    LPTSTR szDllPath = szDllDirPath;
    lstrcat(szDllPath, _T("\\UAccCOM.dll"));

    actctx.lpSource = szDllPath;
    actctx.lpResourceName = MAKEINTRESOURCE(97);
    actctx.dwFlags = ACTCTX_FLAG_RESOURCE_NAME_VALID;

    HANDLE pActCtx = CreateActCtx(&actctx);

    if(pActCtx != INVALID_HANDLE_VALUE)
    {
        if(ActivateActCtx(pActCtx, &lpCookie))
        {
            return true;
        }
    }
    pActCtx = INVALID_HANDLE_VALUE;
    lpCookie = 0;
    return false;
}

static void DeactivateActContext()
{
    if(!IsXPOrLater())
        return;

    if(lpCookie)
        DeactivateActCtx(0, lpCookie);

    if(pActCtx!=INVALID_HANDLE_VALUE)
        ReleaseActCtx(pActCtx);
}
#endif