/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <svsys.h>
#include <tools/debug.hxx>
#include <win/saldata.hxx>

SalShlData aSalShlData;

extern "C" BOOL WINAPI DllMain(HINSTANCE hInst, DWORD nReason, LPVOID)
{
    if ( nReason == DLL_PROCESS_ATTACH )
        aSalShlData.mhInst = hInst;
    return 1;
}

HCURSOR ImplLoadSalCursor( int nId )
{
    DBG_ASSERT( aSalShlData.mhInst, "no DLL instance handle" );

    HCURSOR hCursor = LoadCursor( aSalShlData.mhInst, MAKEINTRESOURCE( nId ) );

    DBG_ASSERT( hCursor, "cursor not found in sal resource" );

    return hCursor;
}

HBITMAP ImplLoadSalBitmap( int nId )
{
    DBG_ASSERT( aSalShlData.mhInst, "no DLL instance handle" );

    HBITMAP hBitmap = LoadBitmap( aSalShlData.mhInst, MAKEINTRESOURCE( nId ) );

    DBG_ASSERT( hBitmap, "bitmap not found in sal resource" );

    return hBitmap;
}

bool ImplLoadSalIcon( int nId, HICON& rIcon, HICON& rSmallIcon )
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
