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

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

class BackgroundSaveWatchdog
{
public:
    BackgroundSaveWatchdog(unsigned mobileAppDocId, ProcUtil::ThreadId savingTid);
    ~BackgroundSaveWatchdog();
    void complete();

    /// The exit code of a background save child that the watchdog ended. Distinct
    /// from the sysexits(3) codes, so the parent can tell why the child is gone.
    static constexpr int TimeoutExitCode = 79;

private:
    std::atomic_bool _saveCompleted; ///< Defend against spurious wakes.
    std::condition_variable _watchdogCV;
    std::mutex _watchdogMutex;
    std::thread _watchdogThread;

public:
    static std::unique_ptr<BackgroundSaveWatchdog> Instance;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
