/*************************************************************************
 *
 *  $RCSfile: ooofiltproxy.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: pjunck $ $Date: 2004-11-03 08:01:14 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <windows.h>
#include <tchar.h>

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
