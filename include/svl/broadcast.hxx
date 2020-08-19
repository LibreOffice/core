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
#ifndef INCLUDED_SVL_BROADCAST_HXX
#define INCLUDED_SVL_BROADCAST_HXX

#include <svl/svldllapi.h>

#include <vector>

class SvtListener;
class SfxHint;

class SVL_DLLPUBLIC SvtBroadcaster
{
public:
    friend class SvtListener;

    typedef std::vector<SvtListener*> ListenersType;

private:
    const SvtBroadcaster&   operator=(const SvtBroadcaster &) = delete;

    /**
     * Ensure that the container doesn't contain any duplicated listener
     * entries. As a side effect, the listeners get sorted by pointer values
     * after this call.
     */
    void Normalize() const;

    void Add( SvtListener* p );
    void Remove( SvtListener* p );

protected:
    virtual void            ListenersGone();

public:
                            SvtBroadcaster();
                            SvtBroadcaster( const SvtBroadcaster &rBC );
    virtual                 ~SvtBroadcaster();

    void                    Broadcast( const SfxHint &rHint );

    ListenersType& GetAllListeners();
    const ListenersType& GetAllListeners() const;

    bool HasListeners() const;

    /**
     * Listeners and broadcasters are M:N relationship.  If you want to
     * destruct them, you easily end up in O(M*N) situation; where for every
     * listener, you iterate all broadcasters, to remove that one listener.
     *
     * To avoid that, use this call to announce to the broadcaster it is going
     * to die, and the listeners do not have to bother with removing
     * themselves from the broadcaster - the broadcaster will not broadcast
     * anything after the PrepareForDestruction() call anyway.
     */
    void PrepareForDestruction();

private:
    /// contains only one of each listener, which is enforced by SvtListener
    mutable ListenersType maListeners;

    /// When the broadcaster is about to die, collect listeners that asked for removal.
    mutable ListenersType maDestructedListeners;

    mutable sal_Int32 mnEmptySlots;
    // The first item in maListeners that is not sorted. The container can become large, so this optimizes sorting.
    mutable sal_Int32 mnListenersFirstUnsorted;
    /// Indicate that this broadcaster will be destructed (we indicate this on all ScColumn's broadcasters during the ScTable destruction, eg.)
    bool mbAboutToDie:1;
    bool mbDisposing:1;
    // Whether maDestructedListeners is sorted or not.
    mutable bool mbDestNormalized:1;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
