/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sysdir_win.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:33:10 $
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

////////////////////////////////////////////////////////////////////////////
////
////                  Windows ONLY
////
////////////////////////////////////////////////////////////////////////////


#include <tools/prewin.h>
#ifndef _SHOBJ_H
#include <shlobj.h>
#endif
#include <tools/postwin.h>
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
        aFolder = UniString( lpFolderW );

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


