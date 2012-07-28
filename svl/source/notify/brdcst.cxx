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
#include <osl/diagnose.h>

#include <svl/hint.hxx>
#include <svl/smplhint.hxx>
#include <svl/lstner.hxx>

#include <svl/brdcst.hxx>
#include <algorithm>

//====================================================================
DBG_NAME(SfxBroadcaster)
TYPEINIT0(SfxBroadcaster);

//====================================================================

//====================================================================
// broadcast immediately


void SfxBroadcaster::Broadcast( const SfxHint &rHint )
{
    DBG_CHKTHIS(SfxBroadcaster, 0);

    // notify all registered listeners exactly once
    for (size_t n = 0; n < aListeners.size(); ++n)
    {
        SfxListener* pListener = aListeners[n];
        pListener->Notify( *this, rHint );
    }
}

// unregister all listeners

SfxBroadcaster::~SfxBroadcaster()
{
    DBG_DTOR(SfxBroadcaster, 0);

    Broadcast( SfxSimpleHint(SFX_HINT_DYING) );

    // remove all still registered listeners
    for (size_t nPos = 0; nPos < aListeners.size(); ++nPos)
    {
        SfxListener *pListener = aListeners[nPos];
        pListener->RemoveBroadcaster_Impl(*this);
    }
}

//--------------------------------------------------------------------

// simple ctor of class SfxBroadcaster

SfxBroadcaster::SfxBroadcaster()
{
    DBG_CTOR(SfxBroadcaster, 0);
}

//--------------------------------------------------------------------

// copy ctor of class SfxBroadcaster


SfxBroadcaster::SfxBroadcaster( const SfxBroadcaster &rBC )
{
    DBG_CTOR(SfxBroadcaster, 0);

    for (size_t n = 0; n < rBC.aListeners.size(); ++n)
    {
        SfxListener *pListener = rBC.aListeners[n];
        pListener->StartListening( *this );
    }
}

//--------------------------------------------------------------------

// add a new SfxListener to the list

void SfxBroadcaster::AddListener( SfxListener& rListener )
{
    DBG_CHKTHIS(SfxBroadcaster, 0);

    aListeners.push_back(&rListener);
}

//--------------------------------------------------------------------

// called, if no more listeners exists

void SfxBroadcaster::ListenersGone()
{
    DBG_CHKTHIS(SfxBroadcaster,0);
}

//--------------------------------------------------------------------

// forward a notification to all registered listeners

void SfxBroadcaster::Forward(SfxBroadcaster& rBC, const SfxHint& rHint)
{
    for (size_t i = 0; i < aListeners.size(); ++i)
    {
        SfxListener *pListener = aListeners[i];
        pListener->Notify( rBC, rHint );
    }
}

//--------------------------------------------------------------------

// remove one SfxListener from the list

void SfxBroadcaster::RemoveListener( SfxListener& rListener )
{
    {DBG_CHKTHIS(SfxBroadcaster, 0);}
    const SfxListener *pListener = &rListener;

    SfxListenerArr_Impl::iterator aIter = std::remove(aListeners.begin(), aListeners.end(), pListener);
    DBG_ASSERT( aIter != aListeners.end(), "RemoveListener: Listener unknown" );
    aListeners.erase(aIter, aListeners.end());

    if ( !HasListeners() )
        ListenersGone();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
