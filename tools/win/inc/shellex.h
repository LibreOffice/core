/*************************************************************************
 *
 *  $RCSfile: shellex.h,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:03:11 $
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

