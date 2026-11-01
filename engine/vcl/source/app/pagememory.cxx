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

#include <sal/config.h>

#include <pagememory.hxx>

#if defined SAL_UNX
#include <unistd.h>
#include <sys/mman.h>
#elif defined _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#error Unsupported platform
#endif

namespace
{
std::size_t querySystemPageSize()
{
#if defined SAL_UNX
#if defined FREEBSD || defined NETBSD || defined OPENBSD || defined DRAGONFLY || defined HAIKU
    return static_cast<std::size_t>(getpagesize());
#else
    return static_cast<std::size_t>(sysconf(_SC_PAGESIZE));
#endif
#elif defined _WIN32
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    return static_cast<std::size_t>(info.dwPageSize);
#endif
}
}

namespace vcl::pagememory
{
std::size_t pageSize()
{
    static const std::size_t nPageSize = querySystemPageSize();
    return nPageSize;
}

void* allocate(std::size_t& rSize)
{
    const std::size_t nPageSize = pageSize();
    std::size_t n = (rSize + nPageSize - 1) & ~(nPageSize - 1);
    void* p;
#if defined SAL_UNX
    p = mmap(nullptr, n, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
    if (p == MAP_FAILED)
        p = nullptr;
#elif defined _WIN32
    p = VirtualAlloc(nullptr, n, MEM_COMMIT, PAGE_READWRITE);
#endif
    if (p != nullptr)
        rSize = n;
    return p;
}

void release(void* pAddress, std::size_t nSize)
{
#if defined SAL_UNX
    munmap(pAddress, nSize);
#elif defined _WIN32
    (void)nSize; // unused
    VirtualFree(pAddress, 0, MEM_RELEASE);
#endif
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
