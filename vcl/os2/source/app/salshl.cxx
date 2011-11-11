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



#include <svpm.h>

#define _SV_SALSHL_CXX
#include <os2/saldata.hxx>
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

sal_Bool ImplLoadSalIcon( int nId, HPOINTER& rIcon)
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

