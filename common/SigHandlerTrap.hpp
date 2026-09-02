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

#include <atomic>
#include <chrono>
#include <thread>

// Lives in Util.cpp for dependency reasons.
namespace SigUtil
{
    /// This traps the signal-handler so we don't _Exit
    /// while dumping stack trace. It's re-entrant.
    /// Used to safely increment and decrement the signal-handler trap.
    class SigHandlerTrap
    {
        static std::atomic<int> SigHandling;
    public:
        SigHandlerTrap() { ++SigHandlerTrap::SigHandling; }
        ~SigHandlerTrap() { --SigHandlerTrap::SigHandling; }

        /// Check that we have exclusive access to the trap.
        /// Otherwise, there is another signal in progress.
        static bool isExclusive()
        {
            // Return true if we are alone.
            return SigHandlerTrap::SigHandling == 1;
        }

        /// Wait for the trap to clear.
        static void wait()
        {
            while (SigHandlerTrap::SigHandling)
                std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    };

} // end namespace SigUtil

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
