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

#include <svl/listener.hxx>
#include <svl/broadcast.hxx>

SvtListener::QueryBase::QueryBase( sal_uInt16 nId ) : mnId(nId) {}
SvtListener::QueryBase::~QueryBase() {}

sal_uInt16 SvtListener::QueryBase::getId() const
{
    return mnId;
}

SvtListener::SvtListener() {}

SvtListener::SvtListener( const SvtListener & )  {}

SvtListener::~SvtListener() COVERITY_NOEXCEPT_FALSE
{
    // Unregister itself from all broadcasters it's listening to.
    EndListeningAll();
}

// registers at a specific SvtBroadcaster

bool SvtListener::StartListening( SvtBroadcaster& rBroadcaster )
{
    std::pair<BroadcastersType::const_iterator, bool> r =
        maBroadcasters.insert(&rBroadcaster);
    if (r.second)
    {
        // This is a new broadcaster.
        rBroadcaster.Add(this);
    }
    return r.second;
}

bool SvtListener::EndListening( SvtBroadcaster& rBroadcaster )
{
    BroadcastersType::const_iterator it = maBroadcasters.find(&rBroadcaster);
    if (it == maBroadcasters.end())
        // Not listening to this broadcaster.
        return false;

    rBroadcaster.Remove(this);
    maBroadcasters.erase(it);
    return true;
}

// called from the SvtBroadcaster destructor, used to avoid calling
// back into the broadcaster again
void SvtListener::BroadcasterDying( SvtBroadcaster& rBroadcaster )
{
    BroadcastersType::const_iterator it = maBroadcasters.find(&rBroadcaster);
    if (it != maBroadcasters.end())
        maBroadcasters.erase(it);
}

void SvtListener::EndListeningAll()
{
    for (SvtBroadcaster* p : maBroadcasters)
    {
        SvtBroadcaster& rBC = *p;
        rBC.Remove(this);
    }
    maBroadcasters.clear();
}


void SvtListener::CopyAllBroadcasters( const SvtListener& r )
{
    EndListeningAll();
    BroadcastersType aCopy(r.maBroadcasters);
    maBroadcasters.swap(aCopy);
    for (SvtBroadcaster* p : maBroadcasters)
    {
        p->Add(this);
    }
}

bool SvtListener::HasBroadcaster() const
{
    return !maBroadcasters.empty();
}

void SvtListener::Notify( const SfxHint& /*rHint*/ ) {}

void SvtListener::Query( QueryBase& /*rQuery*/ ) const {}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
