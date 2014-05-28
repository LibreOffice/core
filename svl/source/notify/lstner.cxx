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


#include <tools/debug.hxx>

#include <svl/hint.hxx>
#include <svl/brdcst.hxx>

#include <svl/lstner.hxx>
#include <algorithm>

TYPEINIT0(SfxListener);

// simple ctor of class SfxListener

SfxListener::SfxListener()
{
}

// copy ctor of class SfxListener

SfxListener::SfxListener( const SfxListener &rListener )
{
    for ( sal_uInt16 n = 0; n < rListener.aBCs.size(); ++n )
        StartListening( *rListener.aBCs[n] );
}

// unregisters the SfxListener from its SfxBroadcasters

SfxListener::~SfxListener()
{
    // unregister at all remaining broadcasters
    for ( sal_uInt16 nPos = 0; nPos < aBCs.size(); ++nPos )
    {
        SfxBroadcaster *pBC = aBCs[nPos];
        pBC->RemoveListener(*this);
    }
}


// unregisters a specific SfxBroadcaster

void SfxListener::RemoveBroadcaster_Impl( SfxBroadcaster& rBroadcaster )
{
    aBCs.erase( std::find( aBCs.begin(), aBCs.end(), &rBroadcaster ) );
}


// registers a specific SfxBroadcaster

bool SfxListener::StartListening( SfxBroadcaster& rBroadcaster, bool bPreventDups )
{
    if ( !bPreventDups || !IsListening( rBroadcaster ) )
    {
        rBroadcaster.AddListener(*this);
        aBCs.push_back( &rBroadcaster );

        DBG_ASSERT( IsListening(rBroadcaster), "StartListening failed" );
        return true;
    }

    return false;
}


// unregisters a specific SfxBroadcaster

bool SfxListener::EndListening( SfxBroadcaster& rBroadcaster, bool bAllDups )
{
    if ( !IsListening( rBroadcaster ) )
        return false;

    do
    {
        rBroadcaster.RemoveListener(*this);
        aBCs.erase( std::find( aBCs.begin(), aBCs.end(), &rBroadcaster ) );
    }
    while ( bAllDups && IsListening( rBroadcaster ) );
    return true;
}


// unregisters all Broadcasters

void SfxListener::EndListeningAll()
{
    // MI: bei Optimierung beachten: Seiteneffekte von RemoveListener beachten!
    while ( !aBCs.empty() )
    {
        SfxBroadcaster *pBC = aBCs.front();
        pBC->RemoveListener(*this);
        aBCs.pop_front();
    }
}


bool SfxListener::IsListening( SfxBroadcaster& rBroadcaster ) const
{
    return aBCs.end() != std::find( aBCs.begin(), aBCs.end(), &rBroadcaster );
}


// base implementation of notification handler

#ifdef DBG_UTIL
void SfxListener::Notify( SfxBroadcaster& rBroadcaster, const SfxHint& )
#else
void SfxListener::Notify( SfxBroadcaster&, const SfxHint& )
#endif
{
    #ifdef DBG_UTIL
    DBG_ASSERT(aBCs.end() != std::find(aBCs.begin(), aBCs.end(), &rBroadcaster),
                "notification from unregistered broadcaster" );
    #endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
