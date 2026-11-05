/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sys/mman.h>

#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <functional>
#include <new>
#include <utility>

/// Matches the OOM kills of our cgroup to the kits that the forkit and its subforkits reap with
/// SIGKILL, so that each kill is counted once.
class OomKillCounter
{
public:
    /// A kill that no reap matches within matchWindow is dropped.
    OomKillCounter(std::function<std::uint64_t()> readCount, std::chrono::seconds matchWindow)
        : _readCount(std::move(readCount))
        , _matchWindow(matchWindow)
    {
    }

    ~OomKillCounter() { unmap(); }

    OomKillCounter(const OomKillCounter&) = delete;
    OomKillCounter& operator=(const OomKillCounter&) = delete;

    /// Map the handled count, shared with the processes forked afterwards. Kills from before the
    /// call are not counted. Returns false, with errno set, on failure.
    bool map()
    {
        static_assert(std::atomic<std::uint64_t>::is_always_lock_free,
                      "the shared OOM kill count is updated from several processes");

        void* const memory = mmap(nullptr, sizeof(std::atomic<std::uint64_t>),
                                  PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        if (memory == MAP_FAILED)
            return false;

        _lastSeenCount = _readCount();
        _handledCount = new (memory) std::atomic<std::uint64_t>(_lastSeenCount);
        return true;
    }

    /// take() is false from then on.
    void unmap()
    {
        if (_handledCount)
            munmap(_handledCount, sizeof(std::atomic<std::uint64_t>));
        _handledCount = nullptr;
    }

    void update(std::chrono::steady_clock::time_point now)
    {
        if (!_handledCount)
            return;

        const std::uint64_t count = _readCount();
        if (count > _lastSeenCount)
        {
            _lastSeenCount = count;
            _seen.emplace_back(count, now);
        }

        // An unmatched kill took a process that is not a kit, such as a background save.
        while (!_seen.empty() && now - _seen.front().second > _matchWindow)
        {
            const std::uint64_t expired = _seen.front().first;
            std::uint64_t handled = _handledCount->load();
            while (handled < expired && !_handledCount->compare_exchange_weak(handled, expired))
            {
            }
            _seen.pop_front();
        }
    }

    /// Take one kill for a kit reaped with SIGKILL. False when none is left.
    bool take()
    {
        if (!_handledCount)
            return false;

        std::uint64_t handled = _handledCount->load();
        while (handled < _lastSeenCount)
        {
            if (_handledCount->compare_exchange_weak(handled, handled + 1))
                return true;
        }

        return false;
    }

    std::uint64_t handledCount() const { return _handledCount ? _handledCount->load() : 0; }
    std::uint64_t lastSeenCount() const { return _lastSeenCount; }
    std::size_t seenCount() const { return _seen.size(); }

private:
    const std::function<std::uint64_t()> _readCount;
    const std::chrono::seconds _matchWindow;

    /// Kills already counted for a kit or dropped, in memory shared by all forkits. Null when
    /// unmapped.
    std::atomic<std::uint64_t>* _handledCount = nullptr;
    std::uint64_t _lastSeenCount = 0;
    /// Each kernel count seen, with the time this process first saw it.
    std::deque<std::pair<std::uint64_t, std::chrono::steady_clock::time_point>> _seen;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
