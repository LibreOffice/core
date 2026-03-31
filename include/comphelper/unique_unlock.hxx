/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

#include <cassert>
#include <mutex>

namespace comphelper
{
/// RAII guard: unlocks a unique_lock on construction, relocks on destruction.
template <typename Mutex = std::mutex> class unique_unlock
{
    std::unique_lock<Mutex>& m_rLock;

public:
    explicit unique_unlock(std::unique_lock<Mutex>& rLock)
        : m_rLock(rLock)
    {
        assert(m_rLock.owns_lock());
        if (m_rLock.owns_lock())
            m_rLock.unlock();
    }
    ~unique_unlock()
    {
        if (!m_rLock.owns_lock())
            m_rLock.lock();
    }
    unique_unlock(const unique_unlock&) = delete;
    unique_unlock& operator=(const unique_unlock&) = delete;
};
} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
