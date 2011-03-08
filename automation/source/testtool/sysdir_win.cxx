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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_automation.hxx"

////////////////////////////////////////////////////////////////////////////
////
////                  Windows ONLY
////
////////////////////////////////////////////////////////////////////////////


#include <prewin.h>
#ifndef _SHOBJ_H
#if defined _MSC_VER
#pragma warning(push, 1)
#pragma warning(disable: 4917)
#endif
#include <shlobj.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif
#endif
#include <postwin.h>
// as we define it ourselves further down the line we remove it here
#ifdef IS_ERROR
    #undef IS_ERROR
#endif

#include <tchar.h>
#include "sysdir_win.hxx"


////////  copied this from setup2\win\source\system\winos.cxx

void _SHFree( void *pv )
{
    IMalloc *pMalloc;
    if( NOERROR == SHGetMalloc(&pMalloc) )
    {
        pMalloc->Free( pv );
        pMalloc->Release();
    }
}

#define ALLOC(type, n) ((type *) HeapAlloc(GetProcessHeap(), 0, sizeof(type) * n ))
#define FREE(p) HeapFree(GetProcessHeap(), 0, p)

UniString _SHGetSpecialFolder( int nFolderID )
{

    LPITEMIDLIST    pidl;
    HRESULT         hHdl = SHGetSpecialFolderLocation( NULL, nFolderID, &pidl );
    UniString       aFolder;

    if( hHdl == NOERROR )
    {
        WCHAR *lpFolderW;
        lpFolderW = ALLOC( WCHAR, 16000 );

        SHGetPathFromIDListW( pidl, lpFolderW );
        aFolder = UniString( reinterpret_cast<const sal_Unicode*>(lpFolderW) );

        FREE( lpFolderW );
        _SHFree( pidl );
    }
    return aFolder;
}


/////////////// end of copy



String _SHGetSpecialFolder_COMMON_APPDATA()
{
    return _SHGetSpecialFolder( CSIDL_COMMON_APPDATA );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
