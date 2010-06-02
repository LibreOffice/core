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

#include "syspath.hxx"

using namespace ::rtl;

#ifdef WNT
#ifdef _MSC_VER
#pragma warning(disable:4917)
#endif
#include <shlobj.h>

#define ALLOC(type, n) ((type *) HeapAlloc(GetProcessHeap(), 0, sizeof(type) * n ))
#define FREE(p) HeapFree(GetProcessHeap(), 0, p)

static OUString _SHGetSpecialFolderW32( int nFolderID )
{
    LPITEMIDLIST    pidl;
    HRESULT         hHdl = SHGetSpecialFolderLocation( NULL, nFolderID, &pidl );
    OUString        aFolder;

    if( hHdl == NOERROR )
    {
        WCHAR *lpFolderA;
        lpFolderA = ALLOC( WCHAR, 16000 );

        SHGetPathFromIDListW( pidl, lpFolderA );
        aFolder = OUString( reinterpret_cast<const sal_Unicode*>(lpFolderA) );

        FREE( lpFolderA );
        IMalloc *pMalloc;
        if( NOERROR == SHGetMalloc(&pMalloc) )
        {
            pMalloc->Free( pidl );
            pMalloc->Release();
        }
    }
    return aFolder;
}

#endif

OUString SystemPath::GetUserTemplateLocation()
{
#ifdef WNT
    return _SHGetSpecialFolderW32(CSIDL_TEMPLATES);
#endif
#ifdef UNX
    return OUString();
#endif
}
