/*************************************************************************
*
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2008 by Sun Microsystems, Inc.
*
* OpenOffice.org - a multi-platform office productivity suite
*
* $RCSfile: shutdowniconw32.cxx,v $
* $Revision: 1.48 $
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
#include "precompiled_sfx2.hxx"


#ifdef WNT
#ifdef _MSC_VER
#pragma warning(disable:4917)
#endif
#include <shlobj.h>

static bool _SHGetSpecialFolderW32( int nFolderID, WCHAR* pszFolder, int nSize )
{
    LPITEMIDLIST    pidl;
    HRESULT         hHdl = SHGetSpecialFolderLocation( NULL, nFolderID, &pidl );

    if( hHdl == NOERROR )
    {
        WCHAR *lpFolder = static_cast< WCHAR* >( HeapAlloc( GetProcessHeap(), 0, 16000 ));

        SHGetPathFromIDListW( pidl, lpFolder );
        wcsncpy( pszFolder, lpFolder, nSize );

        HeapFree( GetProcessHeap(), 0, lpFolder );
        IMalloc *pMalloc;
        if( NOERROR == SHGetMalloc(&pMalloc) )
        {
            pMalloc->Free( pidl );
            pMalloc->Release();
        }
    }
    return true;
}

#endif

// Copied from sal/types.h to circumvent problems with precompiled headers
// and redefinitions of BOOL, INT32 and other types. Unfortunately tools
// also define these type incompatible with Win32 types which leads from
// time to time to very nasty compilation errors. If someone finds a better
// way to solve these probs please remove this copied part!
typedef unsigned short sal_uInt16;
#if ( defined(WIN32) && !defined(__MINGW32__) )
    typedef wchar_t             sal_Unicode;
#else
    typedef sal_uInt16          sal_Unicode;
#endif

extern "C" bool GetUserTemplateLocation(sal_Unicode* pFolder, int nSize)
{
#ifdef WNT
    return _SHGetSpecialFolderW32( CSIDL_TEMPLATES, reinterpret_cast<LPWSTR>(pFolder), nSize );
#else
    (void)pFolder;
    (void)nSize;
    return false;
#endif
}
