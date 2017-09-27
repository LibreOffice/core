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


#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

typedef HRESULT (__stdcall *lpfnDllRegisterServer)();
typedef HRESULT (__stdcall *lpfnDllUnregisterServer)();

bool IsUnregisterParameter(const wchar_t* Param)
{
    return ((0 == _wcsicmp(Param, L"/u")) ||
            (0 == _wcsicmp(Param, L"-u")));
}

int wmain(int argc, wchar_t* argv[])
{
    HMODULE hmod;

    if (2 == argc)
    {
        hmod = LoadLibraryW(argv[1]);

        if (hmod)
        {
            lpfnDllRegisterServer lpfn_register = reinterpret_cast<lpfnDllRegisterServer>(GetProcAddress(
                hmod, "DllRegisterServer"));

            if (lpfn_register)
                lpfn_register();

            FreeLibrary(hmod);
        }
    }
    else if (3 == argc && IsUnregisterParameter(argv[1]))
    {
        hmod = LoadLibraryW(argv[2]);

        if (hmod)
        {
            lpfnDllUnregisterServer lpfn_unregister = reinterpret_cast<lpfnDllUnregisterServer>(GetProcAddress(
                hmod, "DllUnregisterServer"));

            if (lpfn_unregister)
                lpfn_unregister();

            FreeLibrary(hmod);
        }
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
