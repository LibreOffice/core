/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ooofiltproxy.cxx,v $
 * $Revision: 1.8 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_shell.hxx"

#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif
#include <tchar.h>
#ifdef __MINGW32__
#include <basetyps.h>
#endif


/*
    The indexing filter library ooofilt.dll is linked against a couple
    of libaries (e.g. stlport, uwinapi, ...) which are not standard on
    a windows system. Unfortunately the library will be loaded by the
    indexing service via LoadLibrary so that the libraries ooofilt.dll
    depends on need to be in one of the standard search directories or
    the <office installation> directory need to be added to the PATH
    environment variable. In order to prevent failures while loading
    the ooofilt.dll we're installing and registering this proxy library
    which will load ooofilt.dll using LoadLibraryEx with the flag
    LOAD_WITH_ALTERED_SEARCH_PATH (see MSDN for details). This approach
    ensures that all libraries ooofilt.dll depends on will be found.
*/

typedef HRESULT (__stdcall * Forward_DllGetClassObject_t)(REFCLSID cid, REFIID iid, void** ppvObj);
typedef HRESULT (__stdcall * Forward_DllCanUnloadNow_t)();
typedef HRESULT (__stdcall * Forward_DllRegisterServer_t)();
typedef HRESULT (__stdcall * Forward_DllUnregisterServer_t)();

Forward_DllGetClassObject_t Forward_DllGetClassObject = NULL;
Forward_DllCanUnloadNow_t Forward_DllCanUnloadNow = NULL;
Forward_DllRegisterServer_t Forward_DllRegisterServer = NULL;
Forward_DllUnregisterServer_t Forward_DllUnregisterServer = NULL;

HMODULE hOoofilt = NULL;
HMODULE hThisLibrary = NULL;

/* Truncates the file name from an absolute path but
   leaves the final '\' */
void PathTruncateFileName(TCHAR* path)
{
    TCHAR* p = path + lstrlen(path);
    while (*p != _T('\\')) p--;
    p++;
    *p = 0;
}

void Init()
{
    if (Forward_DllGetClassObject == NULL)
    {
        TCHAR buff[MAX_PATH];
        GetModuleFileName(hThisLibrary, buff, (sizeof(buff)/sizeof(TCHAR)));
        PathTruncateFileName(buff);
        lstrcat(buff, TEXT("ooofilt.dll"));

        hOoofilt = LoadLibraryEx(buff, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);

        if (hOoofilt)
        {
            Forward_DllGetClassObject = reinterpret_cast<Forward_DllGetClassObject_t>(
                GetProcAddress(hOoofilt, "DllGetClassObject"));
            Forward_DllCanUnloadNow = reinterpret_cast<Forward_DllCanUnloadNow_t>(
                GetProcAddress(hOoofilt, "DllCanUnloadNow"));
            Forward_DllRegisterServer = reinterpret_cast<Forward_DllRegisterServer_t>(
                GetProcAddress(hOoofilt, "DllRegisterServer"));
            Forward_DllUnregisterServer = reinterpret_cast<Forward_DllUnregisterServer_t>(
                GetProcAddress(hOoofilt, "DllUnregisterServer"));
        }
    }
}

extern "C" BOOL WINAPI DllMain(HMODULE hInstance, DWORD fdwReason, LPVOID /*lpvReserved*/)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls( hInstance );
        hThisLibrary = hInstance;
        break;
    case DLL_PROCESS_DETACH:
        if (hOoofilt)
            FreeLibrary(hOoofilt);
        break;
    default:
        break;
    }
    return TRUE;
}

extern "C" HRESULT __stdcall DllGetClassObject(REFCLSID cid, REFIID iid, void** ppvObj)
{
    Init();

    if (Forward_DllGetClassObject)
        return Forward_DllGetClassObject(cid, iid, ppvObj);
    else
        return E_FAIL;
}

extern "C" HRESULT __stdcall DllCanUnloadNow()
{
    Init();

    if (Forward_DllCanUnloadNow)
        return Forward_DllCanUnloadNow();
    else
        return E_FAIL;
}

extern "C" HRESULT __stdcall DllRegisterServer()
{
    Init();

    if (Forward_DllRegisterServer)
        return Forward_DllRegisterServer();
    else
        return E_FAIL;
}

extern "C" HRESULT __stdcall DllUnregisterServer()
{
    Init();

    if (Forward_DllUnregisterServer)
        return Forward_DllUnregisterServer();
    else
        return E_FAIL;
}
