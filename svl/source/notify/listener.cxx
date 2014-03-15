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
#include <tools/debug.hxx>

SvtListener::SvtListener() {}

SvtListener::SvtListener( const SvtListener &r ) :
    maBroadcasters(r.maBroadcasters) {}

SvtListener::~SvtListener()
{
    // Unregister itself from all broadcasters it's listening to.
    EndListeningAll();
}

// registeres at a specific SvtBroadcaster

bool SvtListener::StartListening( SvtBroadcaster& rBroadcaster )
{
    std::pair<BroadcastersType::iterator, bool> r =
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
    BroadcastersType::iterator it = maBroadcasters.find(&rBroadcaster);
    if (it == maBroadcasters.end())
        // Not listening to this broadcaster.
        return false;

    rBroadcaster.Remove(this);
    maBroadcasters.erase(it);
    return true;
}

void SvtListener::EndListeningAll()
{
    BroadcastersType::iterator it = maBroadcasters.begin(), itEnd = maBroadcasters.end();
    for (; it != itEnd; ++it)
    {
        SvtBroadcaster& rBC = **it;
        rBC.Remove(this);
    }
    maBroadcasters.clear();
}


bool SvtListener::IsListening( SvtBroadcaster& rBroadcaster ) const
{
    return maBroadcasters.count(&rBroadcaster) > 0;
}

bool SvtListener::HasBroadcaster() const
{
    return !maBroadcasters.empty();
}

void SvtListener::Notify( const SfxHint& ) {}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
