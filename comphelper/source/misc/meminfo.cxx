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

#include <sal/types.h>
#include <comphelper/meminfo.hxx>

#if defined(_WIN32)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "psapi.h"

#elif defined(UNX)

#if defined(MACOSX)
#include <sys/types.h>
#include <mach/mach.h>
#include <mach/host_info.h>
#include <mach/mach_host.h>

#else // Linux
#include <fstream>
#include <unistd.h>
#endif

#endif

namespace comphelper
{
// Returns the virtual memory used currently by the process.
static sal_Int64 getMemUsedBySelf()
{
#if defined(UNX)
#if defined(MACOSX)

    vm_size_t pageSize;
    struct task_basic_info tInfo;
    mach_msg_type_number_t tInfoCount = TASK_BASIC_INFO_COUNT;

    if (host_page_size(mach_host_self(), &pageSize) != KERN_SUCCESS)
        return -1;

    if (task_info(mach_task_self(), TASK_BASIC_INFO, (task_info_t)&tInfo, &tInfoCount)
        != KERN_SUCCESS)
        return -1;

    return static_cast<sal_Int64>(tInfo.virtual_size) * pageSize;

#elif defined(LINUX)

    sal_Int64 pageSize = sysconf(_SC_PAGESIZE);
    if (pageSize <= 0)
        return -1;

    std::ifstream ifs("/proc/self/statm", std::ifstream::in);
    if (!ifs.is_open())
        return -1;

    sal_Int64 vmSize;
    if (ifs >> vmSize)
        return vmSize * pageSize;

    return -1;

#else

    return -1;

#endif

#elif defined(_WIN32)

    PROCESS_MEMORY_COUNTERS_EX pmc;
    GetProcessMemoryInfo(GetCurrentProcess(), reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc),
                         sizeof(pmc));
    return static_cast<sal_Int64>(pmc.PrivateUsage);

#else

    return -1;

#endif
}

static sal_Int64 getMaxAddressableMemLimit()
{
#if defined(_WIN64)
    return 8796093022208ll; // 8 TB
#elif defined(_WIN32)
    return 2147483648ll; // 2 GB
#elif defined(__x86_64__)
    // TODO: check for artificial limits imposed by 'ulimit -Sv' too for UNX ?
    return 140737488355328ll; // 128 TB
#elif defined(__i386__)
    return 3221225472ll; // 3 GB
#else
    return 2147483648ll; // 2 GB
#endif
}

// canAlloc() checks whether allocSize bytes can be allocated without exceeding
// addressable memory limit. This is useful in case of 32-bit systems where
// process addressable memory limit is less than physical memory limit.
bool canAlloc(sal_Int64 allocSize)
{
    if (allocSize <= 0)
        return true;

    sal_Int64 vmSize = getMemUsedBySelf();
    if (vmSize < 0)
        return true;

    sal_Int64 maxSize = getMaxAddressableMemLimit();

    if (vmSize + allocSize <= maxSize)
        return true;

    return false;
}

} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
