/*************************************************************************
 *
 *  $RCSfile: salshl.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:49 $
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

#ifndef _SVWIN_H
#include <tools/svwin.h>
#endif

#define _SV_SALSHL_CXX

#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif

#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif

// =======================================================================

SalShlData aSalShlData;

// =======================================================================

#ifdef WIN

extern "C"
{

int CALLBACK LibMain( HINSTANCE hInst, WORD, WORD nHeap, LPSTR )
{
    if ( nHeap )
        UnlockData( 0 );

    aSalShlData.mhInst = hInst;

    return 1;
}

// -----------------------------------------------------------------------

int CALLBACK WEP( int )
{
    return 1;
}

}

#endif

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

#if !defined ( __BORLANDC__ )
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

BOOL ImplLoadSalIcon( int nId, HICON& rIcon, HICON& rSmallIcon )
{
    DBG_ASSERT( aSalShlData.mhInst, "no DLL instance handle" );

    // Try at first to load the icons from the application exe file
    SalData* pSalData = GetSalData();
    rIcon = LoadIcon( pSalData->mhInst, MAKEINTRESOURCE( nId ) );
    if ( !rIcon )
    {
        // If the application don't provide these icons, then we try
        // to load the icon from the VCL resource
        rIcon = LoadIcon( aSalShlData.mhInst, MAKEINTRESOURCE( nId ) );
        if ( rIcon )
        {
            rSmallIcon = (HICON)LoadImage( aSalShlData.mhInst, MAKEINTRESOURCE( nId ),
                                           IMAGE_ICON, 16, 16, 0 );
        }
        else
            rSmallIcon = 0;
    }
    else
    {
        rSmallIcon = (HICON)LoadImage( pSalData->mhInst, MAKEINTRESOURCE( nId ),
                                       IMAGE_ICON, 16, 16, 0 );
    }

    return (rSmallIcon != 0);
}
