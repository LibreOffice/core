/*************************************************************************
 *
 *  $RCSfile: shutil.h,v $
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

#ifndef _SHUTIL_H_
#define _SHUTIL_H_

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

#endif  // _SHUTIL_H_

