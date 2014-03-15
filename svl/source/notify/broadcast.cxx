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

namespace {

class StartListeningHandler : std::unary_function<SvtListener*, void>
{
    SvtBroadcaster& mrBC;
public:
    StartListeningHandler( SvtBroadcaster& rBC ) : mrBC(rBC) {}

    void operator() ( SvtListener* p )
    {
        p->StartListening(mrBC);
    }
};

class EndListeningHandler : std::unary_function<SvtListener*, void>
{
    SvtBroadcaster& mrBC;
public:
    EndListeningHandler( SvtBroadcaster& rBC ) : mrBC(rBC) {}

    void operator() ( SvtListener* p )
    {
        p->EndListening(mrBC);
    }
};

class NotifyHandler : std::unary_function<SvtListener*, void>
{
    SvtBroadcaster& mrBC;
    const SfxHint& mrHint;
public:
    NotifyHandler( SvtBroadcaster& rBC, const SfxHint& rHint ) : mrBC(rBC), mrHint(rHint) {}

    void operator() ( SvtListener* p )
    {
        p->Notify(mrHint);
    }
};

}

void SvtBroadcaster::Normalize()
{
    if (mbNormalized)
        return;

    std::sort(maListeners.begin(), maListeners.end());
    ListenersType::iterator itUniqueEnd = std::unique(maListeners.begin(), maListeners.end());
    maListeners.erase(itUniqueEnd, maListeners.end());
    mbNormalized = true;
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

    Normalize();
    std::pair<ListenersType::iterator,ListenersType::iterator> r =
        std::equal_range(maListeners.begin(), maListeners.end(), p);

    if (r.first != r.second)
        maListeners.erase(r.first, r.second);
    if (maListeners.empty())
        ListenersGone();
}

SvtBroadcaster::SvtBroadcaster() : mbDisposing(false), mbNormalized(false) {}

SvtBroadcaster::SvtBroadcaster( const SvtBroadcaster &rBC ) :
    maListeners(rBC.maListeners), mbDisposing(false), mbNormalized(rBC.mbNormalized)
{
    std::for_each(maListeners.begin(), maListeners.end(), StartListeningHandler(*this));
}

SvtBroadcaster::~SvtBroadcaster()
{
    mbDisposing = true;
    Broadcast( SfxSimpleHint(SFX_HINT_DYING) );

    // unregister all listeners.
    std::for_each(maListeners.begin(), maListeners.end(), EndListeningHandler(*this));
}

void SvtBroadcaster::Broadcast( const SfxHint &rHint )
{
    Normalize();
    ListenersType listeners(maListeners);
    std::for_each(listeners.begin(), listeners.end(), NotifyHandler(*this, rHint));
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
