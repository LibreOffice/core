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

#include <svl/lstner.hxx>

#include <svl/SfxBroadcaster.hxx>
#include <sal/backtrace.hxx>
#include <sal/log.hxx>

#include <algorithm>
#include <cassert>
#include <vector>
#include <memory>

// copy ctor of class SfxListener

SfxListener::SfxListener( const SfxListener &rOther )
    : maBCs( rOther.maBCs )
{
    for ( size_t n = 0; n < maBCs.size(); ++n )
    {
        maBCs[n]->AddListener(*this);
#ifdef DBG_UTIL
        maCallStacks.emplace( maBCs[n], sal::backtrace_get(10) );
#endif
    }
}

// unregisters the SfxListener from its SfxBroadcasters

SfxListener::~SfxListener() COVERITY_NOEXCEPT_FALSE
{
    // unregister at all remaining broadcasters
    EndListeningAll();
}


// unregisters a specific SfxBroadcaster

void SfxListener::RemoveBroadcaster_Impl( SfxBroadcaster& rBroadcaster )
{
    std::erase(maBCs, &rBroadcaster);
#ifdef DBG_UTIL
    maCallStacks.erase( &rBroadcaster );
#endif
}



/**
 Registers a specific SfxBroadcaster.

 Some code uses duplicates as a kind of ref-counting thing i.e. they add and remove listeners
 on different code paths, and they only really stop listening when the last EndListening() is called.
*/
void SfxListener::StartListening(SfxBroadcaster& rBroadcaster, DuplicateHandling eDuplicateHanding)
{
    bool bListeningAlready = eDuplicateHanding != DuplicateHandling::Allow && IsListening( rBroadcaster );

#ifdef DBG_UTIL
    if (bListeningAlready && eDuplicateHanding == DuplicateHandling::Unexpected)
    {
        auto f = maCallStacks.find( &rBroadcaster );
        SAL_WARN("svl", "previous StartListening call came from: " << sal::backtrace_to_string(f->second.get()));
    }
#endif
    assert(!(bListeningAlready && eDuplicateHanding == DuplicateHandling::Unexpected) && "duplicate listener, try building with DBG_UTIL to find the other insert site.");

    if (!bListeningAlready || eDuplicateHanding != DuplicateHandling::Prevent)
    {
        rBroadcaster.AddListener(*this);
        maBCs.push_back( &rBroadcaster );
#ifdef DBG_UTIL
        maCallStacks.emplace( &rBroadcaster, sal::backtrace_get(10) );
#endif
        assert(IsListening(rBroadcaster) && "StartListening failed");
    }
}

// unregisters a specific SfxBroadcaster

void SfxListener::EndListening( SfxBroadcaster& rBroadcaster, bool bRemoveAllDuplicates )
{
    for (auto it = std::find(maBCs.begin(), maBCs.end(), &rBroadcaster); it != maBCs.end();
         it = std::find(it, maBCs.end(), &rBroadcaster))
    {
        rBroadcaster.RemoveListener(*this);
        it = maBCs.erase(it);
        if (!bRemoveAllDuplicates)
            break;
    }
#ifdef DBG_UTIL
    maCallStacks.erase(&rBroadcaster);
#endif
}


// unregisters all Broadcasters

void SfxListener::EndListeningAll()
{
    for (SfxBroadcaster *pBC : std::vector(std::move(maBCs)))
        pBC->RemoveListener(*this);
#ifdef DBG_UTIL
    maCallStacks.clear();
#endif
}


bool SfxListener::IsListening( SfxBroadcaster& rBroadcaster ) const
{
    return maBCs.end() != std::find( maBCs.begin(), maBCs.end(), &rBroadcaster );
}

sal_uInt16 SfxListener::GetBroadcasterCount() const
{
    return maBCs.size();
}

SfxBroadcaster* SfxListener::GetBroadcasterJOE( sal_uInt16 nNo ) const
{
    return maBCs[nNo];
}


// base implementation of notification handler

void SfxListener::Notify( SfxBroadcaster& rBroadcaster, const SfxHint& )
{
    (void) rBroadcaster;
    assert(IsListening(rBroadcaster));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
