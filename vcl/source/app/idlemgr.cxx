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

#include <vcl/svapp.hxx>

#include <idlemgr.hxx>

struct ImplIdleData
{
    Link<Application*,void>  maIdleHdl;
    sal_uInt16               mnPriority;
    bool                     mbTimeout;
};

#define IMPL_IDLETIMEOUT         350

ImplIdleMgr::ImplIdleMgr():
    mbInDestruction(false)
{
    mpIdleList  = new ImplIdleList();

    maTimer.SetTimeout( IMPL_IDLETIMEOUT );
    maTimer.SetTimeoutHdl( LINK( this, ImplIdleMgr, TimeoutHdl ) );
}

ImplIdleMgr::~ImplIdleMgr()
{
    mbInDestruction = true;
    // Liste loeschen
    for ( size_t i = 0, n = mpIdleList->size(); i < n; ++i ) {
        ImplIdleData* pIdleData = (*mpIdleList)[ i ];
        pIdleData->maIdleHdl.Call( GetpApp() );
        delete pIdleData;
    }
    mpIdleList->clear();
    delete mpIdleList;
}

bool ImplIdleMgr::InsertIdleHdl( const Link<Application*,void>& rLink, sal_uInt16 nPriority )
{
    size_t nPos = (size_t)-1;
    size_t n = mpIdleList->size();
    for ( size_t i = 0; i < n; ++i ) {
        // we need to check each element to verify that rLink isn't in the array
        if ( (*mpIdleList)[ i ]->maIdleHdl == rLink ) {
            return false;
        }
        if ( nPriority <= (*mpIdleList)[ i ]->mnPriority ) {
            nPos = i;
        }
    }

    ImplIdleData* pIdleData = new ImplIdleData;
    pIdleData->maIdleHdl    = rLink;
    pIdleData->mnPriority   = nPriority;
    pIdleData->mbTimeout    = false;

    if ( nPos < mpIdleList->size() ) {
        ImplIdleList::iterator it = mpIdleList->begin();
        ::std::advance( it, nPos );
        mpIdleList->insert( it, pIdleData );
    } else {
        mpIdleList->push_back( pIdleData );
    }

    // if Timer was not started already then start it now
    if ( !maTimer.IsActive() )
        maTimer.Start();

    return true;
}

void ImplIdleMgr::RemoveIdleHdl( const Link<Application*,void>& rLink )
{
    if (mbInDestruction)
        return;

    for ( ImplIdleList::iterator it = mpIdleList->begin(); it != mpIdleList->end(); ++it ) {
        if ( (*it)->maIdleHdl == rLink ) {
            delete *it;
            mpIdleList->erase( it );
            break;
        }
    }

    // there are no more handlers...
    if ( mpIdleList->empty() )
        maTimer.Stop();
}

IMPL_LINK_NOARG_TYPED(ImplIdleMgr, TimeoutHdl, Timer *, void)
{
    for ( size_t i = 0; i < mpIdleList->size(); ++i ) {
        ImplIdleData* pIdleData = (*mpIdleList)[ i ];
        if ( !pIdleData->mbTimeout ) {
            pIdleData->mbTimeout = true;
            pIdleData->maIdleHdl.Call( GetpApp() );
            // May have been removed in the handler
            for ( size_t j = 0; j < mpIdleList->size(); ++j ) {
                if ( (*mpIdleList)[ j ] == pIdleData ) {
                    pIdleData->mbTimeout = false;
                    break;
                }
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
