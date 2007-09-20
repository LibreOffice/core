/*************************************************************************
 *
 *  $RCSfile: salshl.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2007-09-20 16:06:07 $
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

#include <svpm.h>

#define _SV_SALSHL_CXX

#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

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

