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
#include <svl/smplhint.hxx>

void SvtBroadcaster::Normalize()
{
    if (!mbNormalized)
    {
        std::sort(maListeners.begin(), maListeners.end());
        ListenersType::iterator itUniqueEnd = std::unique(maListeners.begin(), maListeners.end());
        maListeners.erase(itUniqueEnd, maListeners.end());
        mbNormalized = true;
    }

    if (!mbDestNormalized)
    {
        std::sort(maDestructedListeners.begin(), maDestructedListeners.end());
        ListenersType::iterator itUniqueEnd = std::unique(maDestructedListeners.begin(), maDestructedListeners.end());
        maDestructedListeners.erase(itUniqueEnd, maDestructedListeners.end());
        mbDestNormalized = true;
    }
}

void SvtBroadcaster::Add( SvtListener* p )
{
    maListeners.push_back(p);
    mbNormalized = false;
}

void SvtBroadcaster::Remove( SvtListener* p )
{
    if (mbDisposing)
        return;

    if (mbAboutToDie)
    {
        maDestructedListeners.push_back(p);
        mbDestNormalized = false;
        return;
    }

    Normalize();
    std::pair<ListenersType::iterator,ListenersType::iterator> r =
        std::equal_range(maListeners.begin(), maListeners.end(), p);

    if (r.first != r.second)
        maListeners.erase(r.first, r.second);
    if (maListeners.empty())
        ListenersGone();
}

SvtBroadcaster::SvtBroadcaster() : mbAboutToDie(false), mbDisposing(false), mbNormalized(false), mbDestNormalized(false) {}

SvtBroadcaster::SvtBroadcaster( const SvtBroadcaster &rBC ) :
    maListeners(rBC.maListeners), maDestructedListeners(rBC.maDestructedListeners),
    mbAboutToDie(rBC.mbAboutToDie), mbDisposing(false),
    mbNormalized(rBC.mbNormalized), mbDestNormalized(rBC.mbDestNormalized)
{
    if (mbAboutToDie)
        Normalize();

    ListenersType::iterator dest(maDestructedListeners.begin());
    for (ListenersType::iterator it(maListeners.begin()); it != maListeners.end(); ++it)
    {
        bool bStart = true;

        if (mbAboutToDie)
        {
            // skip the destructed ones
            while (dest != maDestructedListeners.end() && (*dest < *it))
                ++dest;

            bStart = (dest == maDestructedListeners.end() || *dest != *it);
        }

        if (bStart)
            (*it)->StartListening(*this);
    }
}

SvtBroadcaster::~SvtBroadcaster()
{
    mbDisposing = true;
    Broadcast( SfxSimpleHint(SFX_HINT_DYING) );

    Normalize();

    // now when both lists are sorted, we can linearly unregister all
    // listeners, with the exception of those that already asked to be removed
    // during their own destruction
    ListenersType::iterator dest(maDestructedListeners.begin());
    for (ListenersType::iterator it(maListeners.begin()); it != maListeners.end(); ++it)
    {
        // skip the destructed ones
        while (dest != maDestructedListeners.end() && (*dest < *it))
            ++dest;

        if (dest == maDestructedListeners.end() || *dest != *it)
            (*it)->EndListening(*this);
    }
}

void SvtBroadcaster::Broadcast( const SfxHint &rHint )
{
    Normalize();

    ListenersType::iterator dest(maDestructedListeners.begin());
    ListenersType aListeners(maListeners); // this copy is important to avoid erasing entries while iterating
    for (ListenersType::iterator it(aListeners.begin()); it != aListeners.end(); ++it)
    {
        // skip the destructed ones
        while (dest != maDestructedListeners.end() && (*dest < *it))
            ++dest;

        if (dest == maDestructedListeners.end() || *dest != *it)
            (*it)->Notify(rHint);
    }
}

void SvtBroadcaster::ListenersGone() {}

SvtBroadcaster::ListenersType& SvtBroadcaster::GetAllListeners()
{
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
