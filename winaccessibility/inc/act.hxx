/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef __ACCACT_HXX
#define __ACCACT_HXX

#include <windows.h>
#include <tchar.h>


static HANDLE pActCtx = INVALID_HANDLE_VALUE;
static ULONG_PTR lpCookie;

static bool ActivateActContext()
{
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
    if(lpCookie)
        DeactivateActCtx(0, lpCookie);

    if(pActCtx!=INVALID_HANDLE_VALUE)
        ReleaseActCtx(pActCtx);
}
#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
