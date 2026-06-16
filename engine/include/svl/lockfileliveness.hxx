/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#ifndef INCLUDED_SVL_LOCKFILELIVENESS_HXX
#define INCLUDED_SVL_LOCKFILELIVENESS_HXX

#include <svl/svldllapi.h>
#include <rtl/ustring.hxx>
#include <string_view>

namespace svt {

enum class LockLivenessProbe { Free, Held, Unsupported };

enum class StaleLockDecision
{
    // The host matches and no live process holds the advisory lock. The stored
    // owner is gone and the lock is stale.
    TakeOverSilently,
    // A live process on this host still holds the advisory lock.
    LiveKeepLock,
    // Liveness could not be determined: the stored host differs from this host,
    // or the probe is unsupported on this platform or filesystem.
    UseIdentityHeuristic,
};

SVL_DLLPUBLIC StaleLockDecision decideStaleLock(std::u16string_view rStoredHost,
                                                std::u16string_view rOwnHost,
                                                LockLivenessProbe eProbe);

class SVL_DLLPUBLIC LockFileLivenessHandle
{
public:
    LockFileLivenessHandle() = default;
    ~LockFileLivenessHandle();
    LockFileLivenessHandle(LockFileLivenessHandle&& rOther) noexcept;
    LockFileLivenessHandle& operator=(LockFileLivenessHandle&& rOther) noexcept;
    LockFileLivenessHandle(const LockFileLivenessHandle&) = delete;
    LockFileLivenessHandle& operator=(const LockFileLivenessHandle&) = delete;

    bool is() const { return m_nFd >= 0; }
    void reset();

private:
    explicit LockFileLivenessHandle(int nFd) : m_nFd(nFd) {}
    int m_nFd = -1;
    friend SVL_DLLPUBLIC LockFileLivenessHandle acquireLockFileLiveness(const OUString&);
};

// Acquire a write lock on the dot-lock file at the given URL and hold it.
// The handle is empty (is() == false) when the platform or filesystem cannot
// support the lock, or when the lock is already held by someone else.
SVL_DLLPUBLIC LockFileLivenessHandle acquireLockFileLiveness(const OUString& rLockFileURL);

// Non-blocking probe of the dot-lock file at the given URL. Returns Held when
// another open file description holds the advisory lock, Free when the lock is
// available, and Unsupported when the file does not exist or the platform or
// filesystem cannot take the lock.
SVL_DLLPUBLIC LockLivenessProbe probeLockFileLiveness(const OUString& rLockFileURL);

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
