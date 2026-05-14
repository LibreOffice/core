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

#include <common/ProcUtil.hpp>

#include <cassert>
#include <chrono>
#include <condition_variable>
#include <csignal>
#include <mutex>
#include <thread>
#include <vector>

extern "C"
{
    void handleUserProfileSignal(const int /* signal */);
}

/*
 * A class to watch to see when threads are not making progress.
 */
class Watchdog final : private std::thread
{
    std::condition_variable _condition;
    std::mutex _lock;
    typedef std::pair<std::atomic<uint64_t>*, ProcUtil::ThreadId*> WatchDetail;
    std::vector<WatchDetail> _times;
    std::unique_ptr<std::thread> _thread;
    std::atomic<bool> _exit;

    static const uint64_t MsToTrigger = 75;

public:
    Watchdog()
    {
        startThread();

        // Steal SIGUSR2 from the backtrace handler for profiling
        struct sigaction action;

        sigemptyset(&action.sa_mask);
        action.sa_flags = 0;
        action.sa_handler = handleUserProfileSignal;
        sigaction(SIGUSR2, &action, nullptr);
    }

    ~Watchdog()
    {
        if (_thread)
            joinThread();
    }

    static uint64_t getDisableStamp() { return 0; }

    static uint64_t getTimestamp()
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
                   std::chrono::steady_clock::now().time_since_epoch())
            .count();
    }

    void startThread()
    {
        assert(!_thread);
        _exit = false;
        _thread.reset(new std::thread(&Watchdog::checkTime, this));
    }

    void joinThread()
    {
        if (_thread)
        {
            _exit = true;
            _condition.notify_all();
            _thread->join();
            _thread.reset();
        }
    }

    void checkTime()
    {
        std::unique_lock<std::mutex> guard(_lock);
        while (!_exit)
        {
            {
                uint64_t msSinceEpoc = getTimestamp();
                for (const auto& [time, tid] : _times)
                {
                    const uint64_t snapshot = *time; // one atomic read
                    if (snapshot == 0) // sleeping / polling
                        continue;
                    // out of the poll for longer than threshold:
                    if (msSinceEpoc - snapshot > MsToTrigger)
                    {
                        // Signal the poorly behaved thread to profile it
                        ProcUtil::killThreadById(*tid, SIGUSR2);
                        break;
                    }
                }
            }
            _condition.wait_for(guard, std::chrono::milliseconds(50));
        }
    }

    void addTime(std::atomic<uint64_t>* timeRef, ProcUtil::ThreadId* threadIdRef)
    {
        std::lock_guard<std::mutex> guard(_lock);
        _times.emplace_back(timeRef, threadIdRef);
    }

    void removeTime(std::atomic<uint64_t> *timeRef)
    {
        std::lock_guard<std::mutex> guard(_lock);
        for (auto it = _times.begin(); it != _times.end(); it++)
        {
            if (it->first == timeRef)
            {
                it = _times.erase(it);
                return;
            }
        }
        assert("missing time to remove" && false);
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
