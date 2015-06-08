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

#include <svl/SfxBroadcaster.hxx>

#include <sal/log.hxx>
#include <svl/hint.hxx>
#include <svl/smplhint.hxx>
#include <svl/lstner.hxx>
#include <tools/debug.hxx>

#include <algorithm>
#include <cassert>
#include <unordered_map>
#include <vector>

TYPEINIT0(SfxBroadcaster);

typedef std::vector<SfxListener*> SfxListenerArr_Impl;

struct SfxBroadcaster::Impl
{
    /** Contains the positions of removed listeners. */
    std::vector<size_t>     m_RemovedPositions;
    SfxListenerArr_Impl     m_Listeners;

    /** Contains for each listener the position(s) in the vector m_Listeners.
     * This is a multimap because the same listener may be added twice. */
    std::unordered_multimap<SfxListener*, size_t> m_PositionsByListener;
};

// broadcast immediately

void SfxBroadcaster::Broadcast( const SfxHint &rHint )
{
    // notify all registered listeners exactly once
    for (size_t i = 0; i < mpImpl->m_Listeners.size(); ++i)
    {
        SfxListener *const pListener = mpImpl->m_Listeners[i];
        if (pListener)
            pListener->Notify( *this, rHint );
    }
}

// unregister all listeners

SfxBroadcaster::~SfxBroadcaster()
{
    Broadcast( SfxSimpleHint(SFX_HINT_DYING) );

    // remove all still registered listeners
    for (size_t i = 0; i < mpImpl->m_Listeners.size(); ++i)
    {
        SfxListener *const pListener = mpImpl->m_Listeners[i];
        if (pListener)
            pListener->RemoveBroadcaster_Impl(*this);
    }

    delete mpImpl;
}


// simple ctor of class SfxBroadcaster

SfxBroadcaster::SfxBroadcaster() : mpImpl(new Impl)
{
}


// copy ctor of class SfxBroadcaster


SfxBroadcaster::SfxBroadcaster( const SfxBroadcaster &rBC ) : mpImpl(new Impl)
{
    for (size_t i = 0; i < rBC.mpImpl->m_Listeners.size(); ++i)
    {
        SfxListener *const pListener = rBC.mpImpl->m_Listeners[i];
        if (pListener)
            pListener->StartListening( *this );
    }
}


// add a new SfxListener to the list

void SfxBroadcaster::AddListener( SfxListener& rListener )
{
    size_t positionOfAddedListener;
    DBG_TESTSOLARMUTEX();
    auto it = mpImpl->m_PositionsByListener.equal_range(&rListener);
    if (it.first != mpImpl->m_PositionsByListener.end() && it.second != mpImpl->m_PositionsByListener.end()) {
        SAL_WARN("svl", "Adding the same listener twice to the same SfxBroadcaster.");
    }
    if (mpImpl->m_RemovedPositions.empty())
    {
        mpImpl->m_Listeners.push_back(&rListener);
        positionOfAddedListener = mpImpl->m_Listeners.size() - 1;
    }
    else
    {
        size_t targetPosition = mpImpl->m_RemovedPositions.back();
        mpImpl->m_RemovedPositions.pop_back();
        assert(mpImpl->m_Listeners[targetPosition] == NULL);
        mpImpl->m_Listeners[targetPosition] = &rListener;
        positionOfAddedListener = targetPosition;
    }
    mpImpl->m_PositionsByListener.insert(it.second, std::make_pair<>(&rListener, positionOfAddedListener));
}


// forward a notification to all registered listeners

void SfxBroadcaster::Forward(SfxBroadcaster& rBC, const SfxHint& rHint)
{
    for (size_t i = 0; i < mpImpl->m_Listeners.size(); ++i)
    {
        SfxListener *const pListener = mpImpl->m_Listeners[i];
        if (pListener)
            pListener->Notify( rBC, rHint );
    }
}


// remove one SfxListener from the list

void SfxBroadcaster::RemoveListener( SfxListener& rListener )
{
    DBG_TESTSOLARMUTEX();
    auto it = mpImpl->m_PositionsByListener.find(&rListener);
    assert(it != mpImpl->m_PositionsByListener.end()); // "RemoveListener: Listener unknown"

    size_t positionOfRemovedElement = it->second;
    // DO NOT erase the listener, set the pointer to 0
    // because the current continuation may contain this->Broadcast
    mpImpl->m_PositionsByListener.erase(it);
    mpImpl->m_Listeners[positionOfRemovedElement] = NULL;
    mpImpl->m_RemovedPositions.push_back(positionOfRemovedElement);
}

bool SfxBroadcaster::HasListeners() const
{
    return (GetListenerCount() != 0);
}

size_t SfxBroadcaster::GetListenerCount() const
{
    assert(mpImpl->m_Listeners.size() >= mpImpl->m_RemovedPositions.size());
    return mpImpl->m_Listeners.size() - mpImpl->m_RemovedPositions.size();
}

size_t SfxBroadcaster::GetSizeOfVector() const
{
    return mpImpl->m_Listeners.size();
}

SfxListener* SfxBroadcaster::GetListener( size_t nNo ) const
{
    return mpImpl->m_Listeners[nNo];
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
