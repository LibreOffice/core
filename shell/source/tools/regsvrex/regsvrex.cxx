/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: regsvrex.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 01:41:41 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
