/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salshl.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 12:43:49 $
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
#include "precompiled_vcl.hxx"

#ifndef _SVWIN_H
#include <tools/svwin.h>
#endif

#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif

#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif

// =======================================================================

SalShlData aSalShlData;

// =======================================================================

#ifdef WNT

extern "C"
{

#ifdef ICC
int _CRT_init(void);
#else
WIN_BOOL WINAPI _CRT_INIT( HINSTANCE hInst, DWORD nReason, LPVOID pReserved );
#endif

WIN_BOOL WINAPI LibMain( HINSTANCE hInst, DWORD nReason, LPVOID pReserved )
{
    // Unsere DLL-Initialisierung
    if ( nReason == DLL_PROCESS_ATTACH )
        aSalShlData.mhInst = hInst;

#ifdef ICC
    if ( _CRT_init() == -1 )
#else
    if ( !_CRT_INIT( hInst, nReason, pReserved ) )
#endif
        return 0;

    return 1;
}

}

#endif

// =======================================================================

HCURSOR ImplLoadSalCursor( int nId )
{
    DBG_ASSERT( aSalShlData.mhInst, "no DLL instance handle" );

    HCURSOR hCursor = LoadCursor( aSalShlData.mhInst, MAKEINTRESOURCE( nId ) );

    DBG_ASSERT( hCursor, "cursor not found in sal resource" );

    return hCursor;
}

// -----------------------------------------------------------------------

HBITMAP ImplLoadSalBitmap( int nId )
{
    DBG_ASSERT( aSalShlData.mhInst, "no DLL instance handle" );

    HBITMAP hBitmap = LoadBitmap( aSalShlData.mhInst, MAKEINTRESOURCE( nId ) );

    DBG_ASSERT( hBitmap, "bitmap not found in sal resource" );

    return hBitmap;
}

// -----------------------------------------------------------------------

BOOL ImplLoadSalIcon( int nId, HICON& rIcon, HICON& rSmallIcon )
{
    DBG_ASSERT( aSalShlData.mhInst, "no DLL instance handle" );

    SalData* pSalData = GetSalData();

    // check the cache first
    SalIcon *pSalIcon = pSalData->mpFirstIcon;
    while( pSalIcon )
    {
        if( pSalIcon->nId != nId )
            pSalIcon = pSalIcon->pNext;
        else
        {
            rIcon       = pSalIcon->hIcon;
            rSmallIcon  = pSalIcon->hSmallIcon;
            return (rSmallIcon != 0);
        }
    }

    // Try at first to load the icons from the application exe file
    rIcon = (HICON)LoadImage( pSalData->mhInst, MAKEINTRESOURCE( nId ),
                                           IMAGE_ICON, GetSystemMetrics( SM_CXICON ), GetSystemMetrics( SM_CYICON ),
                                           LR_DEFAULTCOLOR );
    if ( !rIcon )
    {
        // If the application don't provide these icons, then we try
        // to load the icon from the VCL resource
        rIcon = (HICON)LoadImage( aSalShlData.mhInst, MAKEINTRESOURCE( nId ),
                                           IMAGE_ICON, GetSystemMetrics( SM_CXICON ), GetSystemMetrics( SM_CYICON ),
                                           LR_DEFAULTCOLOR );
        if ( rIcon )
        {
            rSmallIcon = (HICON)LoadImage( aSalShlData.mhInst, MAKEINTRESOURCE( nId ),
                                           IMAGE_ICON, GetSystemMetrics( SM_CXSMICON ), GetSystemMetrics( SM_CYSMICON ),
                                           LR_DEFAULTCOLOR );
        }
        else
            rSmallIcon = 0;
    }
    else
    {
        rSmallIcon = (HICON)LoadImage( pSalData->mhInst, MAKEINTRESOURCE( nId ),
                                       IMAGE_ICON, GetSystemMetrics( SM_CXSMICON ), GetSystemMetrics( SM_CYSMICON ),
                                       LR_DEFAULTCOLOR );
    }

    if( rIcon )
    {
        // add to icon cache
        pSalIcon = new SalIcon();
        pSalIcon->nId = nId;
        pSalIcon->hIcon = rIcon;
        pSalIcon->hSmallIcon = rSmallIcon;
        pSalIcon->pNext = pSalData->mpFirstIcon;
        pSalData->mpFirstIcon = pSalIcon;
    }

    return (rSmallIcon != 0);
}
