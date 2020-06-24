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
#include <o3tl/safeint.hxx>
#include <cassert>
#include <algorithm>

void SvtBroadcaster::Normalize() const
{
    if (!mbNormalized)
    {
        // Add() only appends new values, so often the container will be sorted expect for one
        // or few last items. For larger containers it is much more efficient to just sort
        // the unsorted part and then merge.
        if(maListeners.size() > 100)
        {
            auto sortedEnd = std::is_sorted_until(maListeners.begin(),maListeners.end());
            if( o3tl::make_unsigned( sortedEnd - maListeners.begin()) > maListeners.size() * 3 / 4 )
            {
                std::sort( sortedEnd, maListeners.end());
                std::inplace_merge( maListeners.begin(), sortedEnd, maListeners.end());
                mbNormalized = true;
            }
        }
        if (!mbNormalized)
        {
            std::sort(maListeners.begin(), maListeners.end());
            mbNormalized = true;
        }
    }

    if (!mbDestNormalized)
    {
        std::sort(maDestructedListeners.begin(), maDestructedListeners.end());
        mbDestNormalized = true;
    }
}

void SvtBroadcaster::Add( SvtListener* p )
{
    assert(!mbDisposing && "called inside my own destructor?");
    assert(!mbAboutToDie && "called after PrepareForDestruction()?");
    if (mbDisposing || mbAboutToDie)
        return;
    // only reset mbNormalized if we are going to become unsorted
    if (!maListeners.empty() && maListeners.back() > p)
        mbNormalized = false;
    maListeners.push_back(p);
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

    auto it = std::lower_bound(maListeners.begin(), maListeners.end(), p);
    assert (it != maListeners.end() && *it == p);
    if (it != maListeners.end() && *it == p)
    {
        maListeners.erase(it);
    }

    if (maListeners.empty())
        ListenersGone();
}

SvtBroadcaster::SvtBroadcaster() : mbAboutToDie(false), mbDisposing(false), mbNormalized(true), mbDestNormalized(true) {}

SvtBroadcaster::SvtBroadcaster( const SvtBroadcaster &rBC ) :
    mbAboutToDie(false), mbDisposing(false),
    mbNormalized(true), mbDestNormalized(true)
{
    assert(!rBC.mbAboutToDie && "copying an object marked with PrepareForDestruction()?");
    assert(!rBC.mbDisposing && "copying an object that is in its destructor?");

    rBC.Normalize(); // so that insert into ourself is in-order, and therefore we do not need to Normalize()
    maListeners.reserve(rBC.maListeners.size());
    for (SvtListener* p : rBC.maListeners)
         p->StartListening(*this); // this will call back into this->Add()
}

SvtBroadcaster::~SvtBroadcaster()
{
    mbDisposing = true;
    Broadcast( SfxHint(SfxHintId::Dying) );

    Normalize();

    // now when both lists are sorted, we can linearly unregister all
    // listeners, with the exception of those that already asked to be removed
    // during their own destruction
    ListenersType::const_iterator dest(maDestructedListeners.begin());
    for (auto& rpListener : maListeners)
    {
        // skip the destructed ones
        while (dest != maDestructedListeners.end() && (*dest < rpListener))
            ++dest;

        if (dest == maDestructedListeners.end() || *dest != rpListener)
            rpListener->BroadcasterDying(*this);
    }
}

void SvtBroadcaster::Broadcast( const SfxHint &rHint )
{
    Normalize();

    ListenersType::const_iterator dest(maDestructedListeners.begin());
    ListenersType aListeners(maListeners); // this copy is important to avoid erasing entries while iterating
    for (auto& rpListener : aListeners)
    {
        // skip the destructed ones
        while (dest != maDestructedListeners.end() && (*dest < rpListener))
            ++dest;

        if (dest == maDestructedListeners.end() || *dest != rpListener)
            rpListener->Notify(rHint);
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
    // the reserve() serves two purpose (1) performance (2) makes sure our iterators do not become invalid
    maDestructedListeners.reserve(maListeners.size());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
