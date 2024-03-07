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

#include <svl/hint.hxx>
#include <svl/lstner.hxx>
#include <tools/debug.hxx>

#include <algorithm>
#include <cassert>
#include <vector>

// broadcast immediately

void SfxBroadcaster::Broadcast(const SfxHint& rHint)
{
    // notify all registered listeners exactly once
    size_t nSize = m_Listeners.size();
    for (size_t i = 0; i < nSize; ++i)
    {
        SfxListener* const pListener = m_Listeners[i];
        if (pListener)
            pListener->Notify(*this, rHint);
    }
}

// unregister all listeners

SfxBroadcaster::~SfxBroadcaster() COVERITY_NOEXCEPT_FALSE
{
    Broadcast(SfxHint(SfxHintId::Dying));

    // remove all still registered listeners
    for (size_t i = 0; i < m_Listeners.size(); ++i)
    {
        SfxListener* const pListener = m_Listeners[i];
        if (pListener)
            pListener->RemoveBroadcaster_Impl(*this);
    }
}

// copy ctor of class SfxBroadcaster

SfxBroadcaster::SfxBroadcaster(const SfxBroadcaster& rOther)
{
    for (size_t i = 0; i < rOther.m_Listeners.size(); ++i)
    {
        SfxListener* const pListener = rOther.m_Listeners[i];
        if (pListener)
            pListener->StartListening(*this);
    }
}

// add a new SfxListener to the list

void SfxBroadcaster::AddListener(SfxListener& rListener)
{
    DBG_TESTSOLARMUTEX();
    if (m_RemovedPositions.empty())
    {
        m_Listeners.push_back(&rListener);
    }
    else
    {
        size_t targetPosition = m_RemovedPositions.back();
        m_RemovedPositions.pop_back();
        assert(m_Listeners[targetPosition] == nullptr);
        m_Listeners[targetPosition] = &rListener;
    }
}

// forward a notification to all registered listeners

void SfxBroadcaster::Forward(SfxBroadcaster& rBC, const SfxHint& rHint)
{
    for (size_t i = 0; i < m_Listeners.size(); ++i)
    {
        SfxListener* const pListener = m_Listeners[i];
        if (pListener)
            pListener->Notify(rBC, rHint);
    }
}

// remove one SfxListener from the list

void SfxBroadcaster::RemoveListener(SfxListener& rListener)
{
    DBG_TESTSOLARMUTEX();

    // First, check the slots either side of the last removed slot, makes a significant
    // difference when the list is large.
    int positionOfRemovedElement = -1;
    if (!m_RemovedPositions.empty())
    {
        auto i = m_RemovedPositions.back();
        if (i < m_Listeners.size() - 2 && m_Listeners[i + 1] == &rListener)
        {
            positionOfRemovedElement = i + 1;
        }
        else if (i > 0 && m_Listeners[i - 1] == &rListener)
        {
            positionOfRemovedElement = i - 1;
        }
    }
    // then scan the whole list if we didn't find it
    if (positionOfRemovedElement == -1)
    {
        auto aIter = std::find(m_Listeners.begin(), m_Listeners.end(), &rListener);
        positionOfRemovedElement = std::distance(m_Listeners.begin(), aIter);
    }
    // DO NOT erase the listener, set the pointer to 0
    // because the current continuation may contain this->Broadcast
    m_Listeners[positionOfRemovedElement] = nullptr;
    m_RemovedPositions.push_back(positionOfRemovedElement);
}

void SfxBroadcaster::ForAllListeners(std::function<bool(SfxListener*)> f) const
{
    for (size_t i = 0; i < m_Listeners.size(); ++i)
    {
        SfxListener* const pListener = m_Listeners[i];
        if (pListener)
            if (f(pListener))
                break;
    }
}

bool SfxBroadcaster::HasListeners() const { return GetListenerCount() != 0; }

size_t SfxBroadcaster::GetListenerCount() const
{
    return m_Listeners.size() - m_RemovedPositions.size();
}

bool SfxBroadcaster::IsSfxStyleSheet() const { return false; }
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
