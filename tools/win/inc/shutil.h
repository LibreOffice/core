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

#ifndef _SHUTIL_H_
#define _SHUTIL_H_

#if defined WNT

#ifndef _SHOBJ_H
#include <shlobj.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define PROTOCOL_FILE   "file:"
#define MAX_URL         (MAX_PATH + sizeof(PROTOCOL_FILE))

#define SHUTIL_TO_DELIVER

//--------------------------------------------------------------------------

void *  WINAPI WIN_SHAlloc( ULONG cb );
void *  WINAPI WIN_SHRealloc( void *pv, ULONG cb );
void    WINAPI WIN_SHFree( void *pv );

//--------------------------------------------------------------------------

ULONG   WINAPI WIN_SHGetIDListSize( LPCITEMIDLIST pidl );
BOOL    WINAPI WIN_SHCloneIDList( LPCITEMIDLIST pidl, LPITEMIDLIST *ppidl );
BOOL    WINAPI WIN_SHAppendIDList( LPCITEMIDLIST pidl, LPITEMIDLIST *ppidl );
LONG    WINAPI WIN_SHCompareIDList( LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2 );

LONG    WINAPI WIN_SHGetIDListTokenCount( LPCITEMIDLIST pidl );
BOOL    WINAPI WIN_SHGetIDListToken( LPCITEMIDLIST pidl, ULONG nToken, LPITEMIDLIST *ppidl );

BOOL    WINAPI WIN_SHSplitIDList(
    LPCITEMIDLIST   pidl,
    LPITEMIDLIST *  pidlFolder,
    LPITEMIDLIST *  pidlItem
    );

BOOL    WINAPI WIN_SHSplitIDListEx(
    LPCITEMIDLIST   pidl,
    LPITEMIDLIST *  pidlParent,
    LPITEMIDLIST *  pidlChild,
    ULONG           nLevel
    );

//--------------------------------------------------------------------------

#define WIN_SHGetSpecialFolderLocation( nFolder, ppidl ) \
    ((BOOL)(NOERROR == SHGetSpecialFolderLocation( GetFocus(), nFolder, ppidl )))

#define WIN_SHGetPathFromIDList( pidl, pszPath ) \
    SHGetPathFromIDList( pidl, pszPath )

// #define  SHGP_CLSID  0x00000001

// BOOL WINAPI WIN_SHGetPathFromIDListEx( LPCITEMIDLIST pidl, LPSTR pszBuffer, UINT uFlags );

BOOL    WINAPI WIN_SHGetIDListFromPath( LPCSTR pszPath, LPITEMIDLIST *ppidl );

BOOL    WINAPI WIN_SHGetPathFromURL( LPCSTR pszURL, LPSTR pszPath );
BOOL    WINAPI WIN_SHGetURLFromPath( LPCSTR pszPath, LPSTR pszURL );

BOOL    WINAPI WIN_SHGetFolderFromIDList( LPCITEMIDLIST pidl, LPSHELLFOLDER *ppshf );
BOOL    WINAPI WIN_SHGetSpecialFolder( int nFolder, LPSHELLFOLDER *ppshf  );
BOOL    WINAPI WIN_SHGetFolderFromPath( LPCSTR pszPath, LPSHELLFOLDER *ppshf );

BOOL    WINAPI WIN_SHGetSpecialFolderPath( int nFolder, LPSTR pszPath );

HRESULT WINAPI WIN_SHGetDataFromIDList(
    LPSHELLFOLDER psf,
    LPCITEMIDLIST pidl,
    int nFormat,
    PVOID pv,
    int cb
   );


//--------------------------------------------------------------------------

#define SHIC_PIDL   0x00000001
#define SHIC_NO_UI  0x00000002

#define CMDSTR_OPENA        "open"
#define CMDSTR_EXPLOREA     "explore"
#define CMDSTR_FINDA        "find"

#define CMDSTR_OPENW        L"open"
#define CMDSTR_EXPLOREW     L"explore"
#define CMDSTR_FINDW        L"find"

#ifdef UNICODE
#define CMDSTR_OPEN     CMDSTR_OPENW
#define CMDSTR_EXPLORE  CMDSTR_EXPLOREW
#define CMDSTR_FIND     CMDSTR_FINDW
#else
#define CMDSTR_OPEN     CMDSTR_OPENA
#define CMDSTR_EXPLORE  CMDSTR_EXPLOREA
#define CMDSTR_FIND     CMDSTR_FINDA
#endif

#define CMDSTR_DEFAULT      MAKEINTRESOURCE(0x00)

#define CMDSTR_LINK         MAKEINTRESOURCE(0x10)
#define CMDSTR_DELETE       MAKEINTRESOURCE(0x11)
#define CMDSTR_RENAME       MAKEINTRESOURCE(0x12)
#define CMDSTR_PROPERTIES   MAKEINTRESOURCE(0x13)
#define CMDSTR_CUT          MAKEINTRESOURCE(0x18)
#define CMDSTR_COPY         MAKEINTRESOURCE(0x19)

BOOL WINAPI WIN_SHInvokeCommand(
    HWND    hwndOwner,
    DWORD   dwFlags,
    LPCTSTR lpPath,
    LPCSTR  lpVerb,
    LPCSTR  lpParameters,
    LPCSTR  lpDirectory,
    int     nShow
    );

//--------------------------------------------------------------------------

BOOL WINAPI WIN_SHStrRetToMultiByte(
    LPCITEMIDLIST   pidl,
    const STRRET *  pStr,
    LPSTR           lpMultiByte,
    int             cchMultiByte
    );

DWORD WIN_SHBuildCRC( LPVOID pBytes, ULONG nBytes );

DWORD WINAPI WIN_GetShellVersion(VOID);

HIMAGELIST WINAPI WIN_SHGetSystemImageList( UINT uFlags );

//--------------------------------------------------------------------------

/*

ULONG WINAPI WIN_CreateStringFromBinary (
    LPCVOID pv,
    ULONG   cbSize,
    LPSTR   pszString,
    ULONG   cbStringSize
    );

ULONG WINAPI WIN_CreateBinaryFromString(
    LPCSTR  pszStr,
    LPVOID  pBuffer,
    ULONG   cbSize
    );
*/

//--------------------------------------------------------------------------

DWORD WINAPI WIN_SHSetValue(
    HKEY    hKey,
    LPCTSTR pszSubKey,
    LPCTSTR pszValue,
    DWORD   dwType,
    LPCVOID pvData,
    DWORD   cbData
    );

DWORD WINAPI WIN_SHGetValue(
    HKEY    hKey,
    LPCTSTR pszSubKey,
    LPCTSTR pszValue,
    LPDWORD pdwType,
    LPVOID  pvData,
    LPDWORD pcbData
    );

DWORD WINAPI WIN_SHDeleteValue(
    HKEY    hKey,
    LPCTSTR pszSubKey,
    LPCTSTR pszValue
    );

#ifdef __cplusplus
}
#endif

#endif

#endif  // _SHUTIL_H_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
