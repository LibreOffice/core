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
#include <config.h>

#include "Util.hpp"

#include <fcntl.h>
#include <mach/mach.h>
#include <unistd.h>

#include <Poco/Exception.h>
#include "Log.hpp"

namespace Util
{

/** Nothing to do for macOS, everything is implemented directly in the count() methods. */
class CounterImpl {};

ThreadCounter::ThreadCounter() : _impl(new CounterImpl()) {}

ThreadCounter::~ThreadCounter() {}

int ThreadCounter::count() {
    // use the Mach task_threads approach:
    mach_msg_type_number_t threadCount = 0;
    thread_act_array_t threadList;

    kern_return_t kr = task_threads(mach_task_self(), &threadList, &threadCount);
    if (kr != KERN_SUCCESS) {
        return -1;
    }

    // deallocate the array not to leak memory
    vm_deallocate(mach_task_self(), (vm_address_t)threadList, threadCount * sizeof(thread_t));

    return (int)threadCount;
}

FDCounter::FDCounter() : _impl(new CounterImpl()) {}

FDCounter::~FDCounter() {}

int FDCounter::count() {
    // there's no /proc/self/fd, let's use the naive approach:
    // Iterate from 0..getdtablesize()-1 and call fcntl(fd, F_GETFD).
    // NB. a bit slower but hopefully workable for a typical range.
    int maxFD = getdtablesize();
    if (maxFD < 0) {
        return -1;
    }

    int count = 0;
    for (int fd = 0; fd < maxFD; ++fd) {
        // If fcntl works, the FD is in use
        if (fcntl(fd, F_GETFD) != -1) {
            count++;
        }
    }

    return count;
}

} // namespace Util

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
