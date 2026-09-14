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

#include <config.h>

#include <csignal>

#include <common/ProcUtil.hpp>
#include <common/Watchdog.hpp>
#include <net/Socket.hpp>

void Watchdog::installProfileSignalHandler()
{
    // Take SIGUSR2 over from the backtrace handler.
    struct sigaction action;

    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    action.sa_handler = handleUserProfileSignal;
    sigaction(SIGUSR2, &action, nullptr);
}

void Watchdog::requestThreadProfile(ProcUtil::ThreadId threadId)
{
    ProcUtil::killThreadById(threadId, SIGUSR2);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
