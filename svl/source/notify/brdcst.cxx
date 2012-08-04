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

#include <assert.h>

#include <tools/debug.hxx>

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
    for (size_t n = 0; n < m_Listeners.size(); ++n)
    {
        SfxListener *const pListener = m_Listeners[n];
        if (pListener) {
            pListener->Notify( *this, rHint );
        }
    }
}

// unregister all listeners

SfxBroadcaster::~SfxBroadcaster()
{
    DBG_DTOR(SfxBroadcaster, 0);

    Broadcast( SfxSimpleHint(SFX_HINT_DYING) );

    // remove all still registered listeners
    for (size_t nPos = 0; nPos < m_Listeners.size(); ++nPos)
    {
        SfxListener *const pListener = m_Listeners[nPos];
        if (pListener) {
            pListener->RemoveBroadcaster_Impl(*this);
        }
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

    for (size_t n = 0; n < rBC.m_Listeners.size(); ++n)
    {
        SfxListener *const pListener = rBC.m_Listeners[n];
        if (pListener) {
            pListener->StartListening( *this );
        }
    }
}

//--------------------------------------------------------------------

// add a new SfxListener to the list

void SfxBroadcaster::AddListener( SfxListener& rListener )
{
    DBG_CHKTHIS(SfxBroadcaster, 0);

    for (size_t i = 0; i < m_Listeners.size(); ++i)
    {
        if (!m_Listeners[i]) // removed by RemoveListener?
        {
            m_Listeners[i] = &rListener;
            return;
        }
    }
    m_Listeners.push_back(&rListener);
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
    for (size_t i = 0; i < m_Listeners.size(); ++i)
    {
        SfxListener *const pListener = m_Listeners[i];
        if (pListener) {
            pListener->Notify( rBC, rHint );
        }
    }
}

//--------------------------------------------------------------------

// remove one SfxListener from the list

void SfxBroadcaster::RemoveListener( SfxListener& rListener )
{
    {DBG_CHKTHIS(SfxBroadcaster, 0);}
    SfxListenerArr_Impl::iterator aIter = std::find(
            m_Listeners.begin(), m_Listeners.end(), &rListener);
    assert(aIter != m_Listeners.end()); // "RemoveListener: Listener unknown"
    // DO NOT erase the listener, set the pointer to 0
    // because the current continuation may contain this->Broadcast
    *aIter = 0;

    if ( !HasListeners() )
        ListenersGone();
}

bool SfxBroadcaster::HasListeners() const
{
    for (size_t i = 0; i < m_Listeners.size(); ++i)
    {
        if (m_Listeners[i]) {
            return true;
        }
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
