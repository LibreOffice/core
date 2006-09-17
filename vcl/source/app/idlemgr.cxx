/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: idlemgr.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 11:48:16 $
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

#ifndef _LIST_HXX
#include <tools/list.hxx>
#endif

#ifndef _SV_IDLEMGR_HXX
#include <idlemgr.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <svapp.hxx>
#endif

// =======================================================================

struct ImplIdleData
{
    Link        maIdleHdl;
    USHORT      mnPriority;
    BOOL        mbTimeout;
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

BOOL ImplIdleMgr::InsertIdleHdl( const Link& rLink, USHORT nPriority )
{
    ULONG           nPos = LIST_APPEND;
    ImplIdleData*   pIdleData = mpIdleList->First();
    while ( pIdleData )
    {
        // Wenn Link schon existiert, dann gebe FALSE zurueck
        if ( pIdleData->maIdleHdl == rLink )
            return FALSE;

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
    pIdleData->mbTimeout    = FALSE;
    mpIdleList->Insert( pIdleData, nPos );

    // Wenn Timer noch nicht gestartet ist, dann starten
    if ( !maTimer.IsActive() )
        maTimer.Start();

    return TRUE;
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
            pIdleData->mbTimeout = TRUE;
            pIdleData->maIdleHdl.Call( GetpApp() );
            // Kann im Handler entfernt worden sein
            if ( mpIdleList->GetPos( pIdleData ) != LIST_ENTRY_NOTFOUND )
                pIdleData->mbTimeout = FALSE;
        }

        pIdleData = mpIdleList->Next();
    }

    return 0;
}
