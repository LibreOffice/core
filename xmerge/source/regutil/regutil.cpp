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

/*
 * Simple Application which calls the DllRegisterServer or DllUnregisterServer functions
 * of the XMerge ActiveSync plugin.
 */


#include <stdio.h>
#include <string.h>
#include <windows.h>


typedef HRESULT (STDAPICALLTYPE *DLLREGISTERSERVER)(void);
typedef HRESULT (STDAPICALLTYPE *DLLUNREGISTERSERVER)(void);

int main(int argc, char* argv[])
{
    BOOL  bUninstall = FALSE;
    int   nPathIndex = 1;

    if (argc < 2 || argc > 3)
    {
        printf("\nUsage:  regutil [/u] <Full Path of XMergeSync.dll>\n\n");
        return -1;
    }


    if (argc == 3)
    {
        if (strcmp("/u", argv[1]))
        {
            printf("\nUnrecognised option: %s\n", argv[1]);
            return -1;
        }

        bUninstall = TRUE;
        nPathIndex = 2;
    }


    // Dynamically load the library
    HMODULE hmXMDll = LoadLibrary(argv[nPathIndex]);

    if (hmXMDll == NULL)
    {
        printf("\nUnable to load the library %s\n", argv[nPathIndex]);
        return -1;
    }


    // Get an offset to the either the DllRegisterServer or DllUnregisterServer functions
    if (!bUninstall)
    {
        printf("\nRegistering %s ... ", argv[nPathIndex]);

        DLLREGISTERSERVER DllRegisterServer = (DLLREGISTERSERVER)GetProcAddress(hmXMDll, "DllRegisterServer");

        if (DllRegisterServer == NULL)
        {
            printf("failed.\n\nDllRegisterServer is not present in library.\n");
            return -1;
        }

        // Now call the procedure ...
        HRESULT regResult = DllRegisterServer() ;

        if (regResult != S_OK)
        {
            printf("failed.\n");
            return -1;
        }
    }
    else
    {
        printf("\nUnregistering %s ... ", argv[nPathIndex]);

        DLLUNREGISTERSERVER DllUnregisterServer = (DLLUNREGISTERSERVER)GetProcAddress(hmXMDll, "DllUnregisterServer");

        if (DllUnregisterServer == NULL)
        {
            printf("failed.\n\nDllUnregisterServer is not present in library.\n");
            return -1;
        }

        // Now call the procedure ...
        HRESULT regResult = DllUnregisterServer() ;

        if (regResult != S_OK)
        {
            printf("failed.\n");
            return -1;
        }

    }

    printf("done.\n");


    // Clean up
    FreeLibrary(hmXMDll);

    return 0;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
