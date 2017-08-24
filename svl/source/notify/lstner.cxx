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

#include <svl/hint.hxx>
#include <svl/SfxBroadcaster.hxx>
#include <sal/backtrace.hxx>

#include <algorithm>
#include <cassert>
#include <deque>
#include <memory>
#include <map>

typedef std::deque<SfxBroadcaster*> SfxBroadcasterArr_Impl;

struct SfxListener::Impl
{
    SfxBroadcasterArr_Impl maBCs;
#ifdef DBG_UTIL
    std::map<SfxBroadcaster*, std::unique_ptr<BacktraceState>> maCallStacks;
#endif
};

// simple ctor of class SfxListener

SfxListener::SfxListener() : mpImpl(new Impl)
{
}

// copy ctor of class SfxListener

SfxListener::SfxListener( const SfxListener &rListener ) : mpImpl(new Impl)
{
    for ( size_t n = 0; n < rListener.mpImpl->maBCs.size(); ++n )
        StartListening( *rListener.mpImpl->maBCs[n] );
}

// unregisters the SfxListener from its SfxBroadcasters

SfxListener::~SfxListener() COVERITY_NOEXCEPT_FALSE
{
    // unregister at all remaining broadcasters
    for ( size_t nPos = 0; nPos < mpImpl->maBCs.size(); ++nPos )
    {
        SfxBroadcaster *pBC = mpImpl->maBCs[nPos];
        pBC->RemoveListener(*this);
    }
}


// unregisters a specific SfxBroadcaster

void SfxListener::RemoveBroadcaster_Impl( SfxBroadcaster& rBroadcaster )
{
    auto it = std::find( mpImpl->maBCs.begin(), mpImpl->maBCs.end(), &rBroadcaster );
    if (it != mpImpl->maBCs.end()) {
        mpImpl->maBCs.erase( it );
#ifdef DBG_UTIL
        mpImpl->maCallStacks.erase( &rBroadcaster );
#endif
    }
}



/**
 Registers a specific SfxBroadcaster.

 Some code uses duplicates as a kind of ref-counting thing i.e. they add and remove listeners
 on different code paths, and they only really stop listening when the last EndListening() is called.
*/
void SfxListener::StartListening( SfxBroadcaster& rBroadcaster, bool bPreventDuplicates )
{
    bool bListeningAlready = IsListening( rBroadcaster );

#ifdef DBG_UTIL
    if (bListeningAlready && !bPreventDuplicates)
    {
        auto f = mpImpl->maCallStacks.find( &rBroadcaster );
        SAL_WARN("svl", "previous StartListening call came from: " << sal_backtrace_to_string(f->second.get()));
    }
#endif
    assert(!(bListeningAlready && !bPreventDuplicates) && "duplicate listener, try building with DBG_UTIL to find the other insert site.");

    if ( !bListeningAlready || !bPreventDuplicates )
    {
        rBroadcaster.AddListener(*this);
        mpImpl->maBCs.push_back( &rBroadcaster );
#ifdef DBG_UTIL
        mpImpl->maCallStacks.emplace( &rBroadcaster, sal_backtrace_get(10) );
#endif
        assert(IsListening(rBroadcaster) && "StartListening failed");
    }
}

// unregisters a specific SfxBroadcaster

void SfxListener::EndListening( SfxBroadcaster& rBroadcaster, bool bRemoveAllDuplicates )
{
    SfxBroadcasterArr_Impl::iterator beginIt = mpImpl->maBCs.begin();
    do
    {
        SfxBroadcasterArr_Impl::iterator it = std::find( beginIt, mpImpl->maBCs.end(), &rBroadcaster );
        if ( it == mpImpl->maBCs.end() )
        {
            break;
        }
        rBroadcaster.RemoveListener(*this);
        beginIt = mpImpl->maBCs.erase( it );
#ifdef DBG_UTIL
        mpImpl->maCallStacks.erase( &rBroadcaster );
#endif
    }
    while ( bRemoveAllDuplicates );
}


// unregisters all Broadcasters

void SfxListener::EndListeningAll()
{
    // Attention: when optimizing this: respect side effects of RemoveListener!
    while ( !mpImpl->maBCs.empty() )
    {
        SfxBroadcaster *pBC = mpImpl->maBCs.front();
        pBC->RemoveListener(*this);
        mpImpl->maBCs.pop_front();
    }
#ifdef DBG_UTIL
    mpImpl->maCallStacks.clear();
#endif
}


bool SfxListener::IsListening( SfxBroadcaster& rBroadcaster ) const
{
    return mpImpl->maBCs.end() != std::find( mpImpl->maBCs.begin(), mpImpl->maBCs.end(), &rBroadcaster );
}

sal_uInt16 SfxListener::GetBroadcasterCount() const
{
    return mpImpl->maBCs.size();
}

SfxBroadcaster* SfxListener::GetBroadcasterJOE( sal_uInt16 nNo ) const
{
    return mpImpl->maBCs[nNo];
}


// base implementation of notification handler

void SfxListener::Notify( SfxBroadcaster& rBroadcaster, const SfxHint& )
{
    (void) rBroadcaster;
    assert(IsListening(rBroadcaster));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
