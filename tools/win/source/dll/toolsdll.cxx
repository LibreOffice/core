/*************************************************************************
 *
 *  $RCSfile: toolsdll.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:03:11 $
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
#include <svwin.h>
#endif

#ifndef _DLL_HXX
#include <dll.hxx>
#endif
#ifndef _TOOLSDLL_HXX
#include <toolsdll.hxx>
#endif
#ifndef _SHL_HXX
#include <shl.hxx>
#endif

// =======================================================================

#ifdef WIN

extern "C" int CALLBACK LibMain( HINSTANCE, WORD, WORD nHeap, LPSTR )
{
    if ( nHeap )
        UnlockData( 0 );

    return TRUE;
}

// -----------------------------------------------------------------------

extern "C" int CALLBACK WEP( int )
{
    return 1;
}

// -----------------------------------------------------------------------

ToolsData* ImpGetToolsData()
{
    ToolsData* pData = ImpGetAppData();

    // Tools-Init
    if ( !pData )
    {
        // Speicher anlegen
        HANDLE hMem = GlobalAlloc( GMEM_MOVEABLE | GMEM_ZEROINIT,
                                   (DWORD)sizeof( ToolsData ) );

        if ( !hMem )
            return NULL;

        pData = (ToolsData*)GlobalLock( hMem );

        if ( !pData )
        {
            GlobalFree( hMem );
            return NULL;
        }

        // ToolsData setzen
        ImpSetAppData( pData );
    }

    return pData;
}

// -----------------------------------------------------------------------

void** GetAppData( USHORT nSharedLib )
{
    ToolsData* pData = ImpGetToolsData();
    return &(pData->aAppData[nSharedLib]);
}

// -----------------------------------------------------------------------

void SetSVData( SVDATA* pSVData )
{
    ToolsData* pData = ImpGetToolsData();
    pData->pSVData = pSVData;
}

// -----------------------------------------------------------------------

void EnterMultiThread( int bEnter )
{
    ToolsData* pData = ImpGetToolsData();
    if ( bEnter )
        pData->aMemD.nMultiThread++;
    else if ( pData->aMemD.nMultiThread )
        pData->aMemD.nMultiThread--;
}

// -----------------------------------------------------------------------

int IsMultiThread()
{
    return (ImpGetToolsData()->aMemD.nMultiThread != 0);
}

#endif

// =======================================================================

#ifdef WNT

static void* aAppData[SHL_COUNT];

// -----------------------------------------------------------------------

void** GetAppData( USHORT nSharedLib )
{
    return &(aAppData[nSharedLib]);
}

// -----------------------------------------------------------------------

// EnterMultiThread()/IsMultiThread()/... in MEMWNT.CXX, damit inline

#endif
