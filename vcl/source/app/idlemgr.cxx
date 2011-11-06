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
