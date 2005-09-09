/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: shellex.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 14:45:33 $
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

