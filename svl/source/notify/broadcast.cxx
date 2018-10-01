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

#include <svl/broadcast.hxx>
#include <svl/listener.hxx>
#include <svl/hint.hxx>
#include <cassert>
#include <algorithm>

void SvtBroadcaster::Normalize() const
{
    if (!mbNormalized)
    {
        std::sort(maListeners.begin(), maListeners.end());
        mbNormalized = true;
    }

    if (!mbDestNormalized)
    {
        std::sort(maDestructedListeners.begin(), maDestructedListeners.end());
        mbDestNormalized = true;
    }
}

void SvtBroadcaster::Add( SvtListener* p )
{
    assert(!mbAboutToDie && "called inside my own destructor / after PrepareForDestruction()?");
    if (mbAboutToDie)
        return;
    // only reset mbNormalized if we are going to become unsorted
    if (!maListeners.empty() && maListeners.back() > p)
        mbNormalized = false;
    maListeners.push_back(p);
}

void SvtBroadcaster::Remove( SvtListener* p )
{
    if (mbAboutToDie)
    {
        maDestructedListeners.push_back(p);
        mbDestNormalized = false;
        return;
    }

    Normalize();

    auto it = std::lower_bound(maListeners.begin(), maListeners.end(), p);
    assert (it != maListeners.end() && *it == p);
    if (it != maListeners.end() && *it == p)
    {
        maListeners.erase(it);
    }

    if (maListeners.empty())
        ListenersGone();
}

SvtBroadcaster::SvtBroadcaster() : mbAboutToDie(false), mbNormalized(false), mbDestNormalized(false) {}

SvtBroadcaster::SvtBroadcaster( const SvtBroadcaster &rBC ) :
    mbAboutToDie(false),
    mbNormalized(true), mbDestNormalized(true)
{
    assert(!rBC.mbAboutToDie && "copying an object marked with PrepareForDestruction() / that is in it's destructor?");

    maListeners.reserve(rBC.maListeners.size());
    for (ListenersType::iterator it(rBC.maListeners.begin()); it != rBC.maListeners.end(); ++it)
    {
         (*it)->StartListening(*this); // this will call back into this->Add()
    }
}

SvtBroadcaster::~SvtBroadcaster()
{
    PrepareForDestruction();

    Normalize();

    {
        SfxHint aHint(SfxHintId::Dying);
        ListenersType::const_iterator dest(maDestructedListeners.begin());
        for (ListenersType::iterator it(maListeners.begin()); it != maListeners.end(); ++it)
        {
            // skip the destructed ones
            while (dest != maDestructedListeners.end() && (*dest < *it))
                ++dest;

            if (dest == maDestructedListeners.end() || *dest != *it)
                (*it)->Notify(aHint);
        }
    }

    Normalize();

    // now when both lists are sorted, we can linearly unregister all
    // listeners, with the exception of those that already asked to be removed
    // during their own destruction
    {
        ListenersType::const_iterator dest(maDestructedListeners.begin());
        for (ListenersType::iterator it(maListeners.begin()); it != maListeners.end(); ++it)
        {
            // skip the destructed ones
            while (dest != maDestructedListeners.end() && (*dest < *it))
                ++dest;

            if (dest == maDestructedListeners.end() || *dest != *it)
                (*it)->BroadcasterDying(*this);
        }
    }
}

void SvtBroadcaster::Broadcast( const SfxHint &rHint )
{
    assert(!mbAboutToDie && "broadcasting after PrepareForDestruction() / from inside my own destructor?");
    if (mbAboutToDie)
        return;

    ListenersType aListeners(maListeners); // this copy is important to avoid erasing entries while iterating
    for (ListenersType::iterator it(aListeners.begin()); it != aListeners.end(); ++it)
    {
        (*it)->Notify(rHint);
    }
}

void SvtBroadcaster::ListenersGone() {}

SvtBroadcaster::ListenersType& SvtBroadcaster::GetAllListeners()
{
    Normalize();
    return maListeners;
}

const SvtBroadcaster::ListenersType& SvtBroadcaster::GetAllListeners() const
{
    Normalize();
    return maListeners;
}

bool SvtBroadcaster::HasListeners() const
{
    return !maListeners.empty();
}

void SvtBroadcaster::PrepareForDestruction()
{
    mbAboutToDie = true;
    maDestructedListeners.reserve(maListeners.size());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
