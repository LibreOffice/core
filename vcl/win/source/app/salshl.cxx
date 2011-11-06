/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include <tools/debug.hxx>
#include <tools/svwin.h>
#include <win/saldata.hxx>

// =======================================================================

SalShlData aSalShlData;

// =======================================================================

#ifdef WNT

extern "C"
{

#ifdef __MINGW32__
sal_Bool WINAPI DllMain( HINSTANCE hInst, DWORD nReason, LPVOID pReserved )
#else
#ifdef ICC
int _CRT_init(void);
#else
BOOL WINAPI _CRT_INIT( HINSTANCE hInst, DWORD nReason, LPVOID pReserved );
#endif

BOOL WINAPI LibMain( HINSTANCE hInst, DWORD nReason, LPVOID pReserved )
#endif
{
    // Unsere DLL-Initialisierung
    if ( nReason == DLL_PROCESS_ATTACH )
        aSalShlData.mhInst = hInst;

#ifndef __MINGW32__
#ifdef ICC
    if ( _CRT_init() == -1 )
#else
    if ( !_CRT_INIT( hInst, nReason, pReserved ) )
#endif
        return 0;
#endif

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

sal_Bool ImplLoadSalIcon( int nId, HICON& rIcon, HICON& rSmallIcon )
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
