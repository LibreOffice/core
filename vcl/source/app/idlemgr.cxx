/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "precompiled_vcl.hxx"

#include <tools/list.hxx>

#include <vcl/svapp.hxx>

#include <idlemgr.hxx>

// =======================================================================

struct ImplIdleData
{
    Link        maIdleHdl;
    sal_uInt16      mnPriority;
    sal_Bool        mbTimeout;
};

DECLARE_LIST( ImplIdleList, ImplIdleData* )

#define IMPL_IDLETIMEOUT         350

// =======================================================================

ImplIdleMgr::ImplIdleMgr()
{
    mpIdleList  = new ImplIdleList( 8, 8, 8 );

    maTimer.SetTimeout( IMPL_IDLETIMEOUT );
    maTimer.SetTimeoutHdl( LINK( this, ImplIdleMgr, TimeoutHdl ) );
}

// -----------------------------------------------------------------------

ImplIdleMgr::~ImplIdleMgr()
{
    // Liste loeschen
    ImplIdleData* pIdleData = mpIdleList->First();
    while ( pIdleData )
    {
        delete pIdleData;
        pIdleData = mpIdleList->Next();
    }

    delete mpIdleList;
}

// -----------------------------------------------------------------------

sal_Bool ImplIdleMgr::InsertIdleHdl( const Link& rLink, sal_uInt16 nPriority )
{
    sal_uLong           nPos = LIST_APPEND;
    ImplIdleData*   pIdleData = mpIdleList->First();
    while ( pIdleData )
    {
        // Wenn Link schon existiert, dann gebe sal_False zurueck
        if ( pIdleData->maIdleHdl == rLink )
            return sal_False;

        // Nach Prioritaet sortieren
        if ( nPriority <= pIdleData->mnPriority )
            nPos = mpIdleList->GetCurPos();

        // Schleife nicht beenden, da noch
        // geprueft werden muss, ob sich der Link
        // schon in der Liste befindet

        pIdleData = mpIdleList->Next();
    }

    pIdleData               = new ImplIdleData;
    pIdleData->maIdleHdl    = rLink;
    pIdleData->mnPriority   = nPriority;
    pIdleData->mbTimeout    = sal_False;
    mpIdleList->Insert( pIdleData, nPos );

    // Wenn Timer noch nicht gestartet ist, dann starten
    if ( !maTimer.IsActive() )
        maTimer.Start();

    return sal_True;
}

// -----------------------------------------------------------------------

void ImplIdleMgr::RemoveIdleHdl( const Link& rLink )
{
    ImplIdleData* pIdleData = mpIdleList->First();
    while ( pIdleData )
    {
        if ( pIdleData->maIdleHdl == rLink )
        {
            mpIdleList->Remove();
            delete pIdleData;
            break;
        }

        pIdleData = mpIdleList->Next();
    }

    // keine Handdler mehr da
    if ( !mpIdleList->Count() )
        maTimer.Stop();
}

// -----------------------------------------------------------------------

IMPL_LINK( ImplIdleMgr, TimeoutHdl, Timer*, EMPTYARG )
{
    ImplIdleData* pIdleData = mpIdleList->First();
    while ( pIdleData )
    {
        if ( !pIdleData->mbTimeout )
        {
            pIdleData->mbTimeout = sal_True;
            pIdleData->maIdleHdl.Call( GetpApp() );
            // Kann im Handler entfernt worden sein
            if ( mpIdleList->GetPos( pIdleData ) != LIST_ENTRY_NOTFOUND )
                pIdleData->mbTimeout = sal_False;
        }

        pIdleData = mpIdleList->Next();
    }

    return 0;
}
