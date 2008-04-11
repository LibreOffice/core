/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: salshl.cxx,v $
 * $Revision: 1.5 $
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

#include <svpm.h>

#define _SV_SALSHL_CXX
#include <saldata.hxx>
#include <tools/debug.hxx>

// =======================================================================

SalShlData aSalShlData;

HMODULE ImplGetModule(void);
static HMODULE mhMod = ImplGetModule();

// =======================================================================

APIRET APIENTRY DosQueryModFromEIP (HMODULE *phMod, ULONG *pObjNum,
          ULONG BuffLen, PCHAR pBuff, ULONG *pOffset, ULONG Address);

HMODULE ImplGetModule(void)
{
    HMODULE hMod;
    ULONG   ObjNum;
    CHAR    Buff[2*_MAX_PATH];
    ULONG   Offset;
    APIRET  rc;

    // get module handle (and name)
    rc = DosQueryModFromEIP( &hMod, &ObjNum, sizeof( Buff), Buff, &Offset, (ULONG)ImplGetModule);
    if (rc)
        return NULL;
    // return module handle
    aSalShlData.mhMod = hMod;
    return hMod;
}

// =======================================================================

HPOINTER ImplLoadSalCursor( int nId )
{
    DBG_ASSERT( aSalShlData.mhMod, "no DLL instance handle" );

    HPOINTER hPointer = WinLoadPointer( HWND_DESKTOP, aSalShlData.mhMod, nId );

    DBG_ASSERT( hPointer, "pointer not found in sal resource" );
#if OSL_DEBUG_LEVEL>0
    if (!hPointer)
        debug_printf( "ImplLoadSalCursor: pointer %d not found in sal resource\n", nId);
#endif
    return hPointer;
}

// -----------------------------------------------------------------------

BOOL ImplLoadSalIcon( int nId, HPOINTER& rIcon)
{
    DBG_ASSERT( aSalShlData.mhMod, "no DLL instance handle" );

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
            return (rIcon != 0);
        }
    }

    // Try at first to load the icons from the application exe file
    rIcon = WinLoadPointer( HWND_DESKTOP, NULL, nId );
    if ( !rIcon )
    {
        // If the application don't provide these icons, then we try
        // to load the icon from the VCL resource
        rIcon = WinLoadPointer( HWND_DESKTOP, aSalShlData.mhMod, nId );
    }

    if( rIcon )
    {
        // add to icon cache
        pSalIcon = new SalIcon();
        pSalIcon->nId = nId;
        pSalIcon->hIcon = rIcon;
        pSalIcon->pNext = pSalData->mpFirstIcon;
        pSalData->mpFirstIcon = pSalIcon;
    }

    return (rIcon != 0);
}

// =======================================================================

