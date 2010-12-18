/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#ifndef _SHELLEX_H_
#define _SHELLEX_H_

#ifndef _SHLOBJ_H_
#include <shlobj.h>
#endif

#ifdef  __cplusplus
extern "C" {
#define WINSHELLCALL    inline
#else
#define WINSHELLCALL    static
#endif

#define SHChangeNotifyRegister_PROC_STR         MAKEINTRESOURCE(2)
#define SHChangeNotifyDeregister_PROC_STR       MAKEINTRESOURCE(4)

#define SHCNF_ACCEPT_INTERRUPTS         0x0001
#define SHCNF_ACCEPT_NON_INTERRUPTS     0x0002
#define SHCNF_NO_PROXY                  0x8000

#define SHCNF_ACCEPT_ALL                (SHCNF_ACCEPT_INTERRUPTS | SHCNF_ACCEPT_NON_INTERRUPTS)

typedef struct tagNOTIFYREGISTER {
    LPCITEMIDLIST   pidlPath;
    BOOL            bWatchSubtree;
} NOTIFYREGISTER;

typedef NOTIFYREGISTER *LPNOTIFYREGISTER;
typedef NOTIFYREGISTER const *LPCNOTIFYREGISTER;

typedef HANDLE (WINAPI *SHChangeNotifyRegister_PROC)(
    HWND    hWnd,
    DWORD   dwFlags,
    LONG    wEventMask,
    UINT    uMsg,
    ULONG   cItems,
    LPCNOTIFYREGISTER   lpItems);


WINSHELLCALL HANDLE WINAPI SHChangeNotifyRegister(
    HWND    hWnd,
    DWORD   dwFlags,
    LONG    wEventMask,
    UINT    uMsg,
    ULONG   cItems,
    LPCNOTIFYREGISTER   lpItems)

{
    HMODULE hModule = GetModuleHandle( "SHELL32" );
    HANDLE  hNotify = NULL;

    if ( hModule )
    {
        SHChangeNotifyRegister_PROC lpfnSHChangeNotifyRegister = (SHChangeNotifyRegister_PROC)GetProcAddress( hModule, SHChangeNotifyRegister_PROC_STR );
        if ( lpfnSHChangeNotifyRegister )
            hNotify = lpfnSHChangeNotifyRegister( hWnd, dwFlags, wEventMask, uMsg, cItems, lpItems );
    }

    return hNotify;
}



typedef BOOL (WINAPI *SHChangeNotifyDeregister_PROC)(
    HANDLE  hNotify);

WINSHELLCALL BOOL WINAPI SHChangeNotifyDeregister( HANDLE hNotify )
{
    HMODULE hModule = GetModuleHandle( "SHELL32" );
    BOOL    fSuccess = FALSE;

    if ( hModule )
    {
        SHChangeNotifyDeregister_PROC   lpfnSHChangeNotifyDeregister = (SHChangeNotifyDeregister_PROC)GetProcAddress( hModule, SHChangeNotifyDeregister_PROC_STR );
        if ( lpfnSHChangeNotifyDeregister )
            fSuccess = lpfnSHChangeNotifyDeregister( hNotify );
    }

    return fSuccess;
}


#ifdef  __cplusplus
}
#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
