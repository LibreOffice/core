/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: regsvrex.cxx,v $
 * $Revision: 1.6 $
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

typedef HRESULT (__stdcall *lpfnDllRegisterServer)();
typedef HRESULT (__stdcall *lpfnDllUnregisterServer)();

/**
*/
bool IsUnregisterParameter(const char* Param)
{
    return ((0 == _stricmp(Param, "/u")) ||
            (0 == _stricmp(Param, "-u")));
}

/**
*/
int main(int argc, char* argv[])
{
    HMODULE hmod;
    HRESULT hr = E_FAIL;
    lpfnDllRegisterServer   lpfn_register;
    lpfnDllUnregisterServer lpfn_unregister;

    if (2 == argc)
    {
        hmod = LoadLibraryA(argv[1]);

        if (hmod)
        {
            lpfn_register = (lpfnDllRegisterServer)GetProcAddress(
                hmod, "DllRegisterServer");

            if (lpfn_register)
                hr = lpfn_register();

            FreeLibrary(hmod);
        }
    }
    else if (3 == argc && IsUnregisterParameter(argv[1]))
    {
        hmod = LoadLibraryA(argv[2]);

        if (hmod)
        {
            lpfn_unregister = (lpfnDllUnregisterServer)GetProcAddress(
                hmod, "DllUnregisterServer");

            if (lpfn_unregister)
                hr = lpfn_unregister();

            FreeLibrary(hmod);
        }
    }

    return 0;
}
