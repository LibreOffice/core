/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include <vcl/idlemgr.hxx>
#include <vcl/svapp.hxx>

// =======================================================================

struct ImplIdleData
{
    Link        maIdleHdl;
    sal_uInt16      mnPriority;
    sal_Bool        mbTimeout;
};

#define IMPL_IDLETIMEOUT         350

// =======================================================================

ImplIdleMgr::ImplIdleMgr()
{
    mpIdleList  = new ImplIdleList();

    maTimer.SetTimeout( IMPL_IDLETIMEOUT );
    maTimer.SetTimeoutHdl( LINK( this, ImplIdleMgr, TimeoutHdl ) );
}

// -----------------------------------------------------------------------

ImplIdleMgr::~ImplIdleMgr()
{
    // Liste loeschen
    for ( size_t i = 0, n = mpIdleList->size(); i < n; ++i ) {
        delete (*mpIdleList)[ i ];
    }
    mpIdleList->clear();
    delete mpIdleList;
}

// -----------------------------------------------------------------------

sal_Bool ImplIdleMgr::InsertIdleHdl( const Link& rLink, sal_uInt16 nPriority )
{
    size_t nPos = (size_t)-1;
    size_t n = mpIdleList->size();
    for ( size_t i = 0; i < n; ++i ) {
        // we need to check each element to verify that rLink isn't in the array
        if ( (*mpIdleList)[ i ]->maIdleHdl == rLink ) {
            return sal_False;
        }
        if ( nPriority <= (*mpIdleList)[ i ]->mnPriority ) {
            nPos = i;
        }
    }

    ImplIdleData* pIdleData = new ImplIdleData;
    pIdleData->maIdleHdl    = rLink;
    pIdleData->mnPriority   = nPriority;
    pIdleData->mbTimeout    = sal_False;

    if ( nPos < mpIdleList->size() ) {
        ImplIdleList::iterator it = mpIdleList->begin();
        ::std::advance( it, nPos );
        mpIdleList->insert( it, pIdleData );
    } else {
        mpIdleList->push_back( pIdleData );
    }

    // Wenn Timer noch nicht gestartet ist, dann starten
    if ( !maTimer.IsActive() )
        maTimer.Start();

    return sal_True;
}

// -----------------------------------------------------------------------

void ImplIdleMgr::RemoveIdleHdl( const Link& rLink )
{
    for ( ImplIdleList::iterator it = mpIdleList->begin(); it < mpIdleList->end(); ++it ) {
        if ( (*it)->maIdleHdl == rLink ) {
            delete *it;
            mpIdleList->erase( it );
            break;
        }
    }

    // keine Handdler mehr da
    if ( mpIdleList->empty() )
        maTimer.Stop();
}

// -----------------------------------------------------------------------

IMPL_LINK( ImplIdleMgr, TimeoutHdl, Timer*, EMPTYARG )
{
    for ( size_t i = 0; i < mpIdleList->size(); ++i ) {
        ImplIdleData* pIdleData = (*mpIdleList)[ i ];
        if ( !pIdleData->mbTimeout ) {
            pIdleData->mbTimeout = sal_True;
            pIdleData->maIdleHdl.Call( GetpApp() );
            // May have been removed in the handler
            for ( size_t j = 0; j < mpIdleList->size(); ++j ) {
                if ( (*mpIdleList)[ j ] == pIdleData ) {
                    pIdleData->mbTimeout = sal_False;
                    break;
                }
            }
        }
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
