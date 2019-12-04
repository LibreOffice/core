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

#include <comphelper/meminfo.hxx>
#include <algorithm>

#if defined(_WIN32)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#elif defined UNX

#if defined MACOSX
#include <sys/types.h>
#include <mach/host_info.h>
#include <mach/mach_host.h>
#else
#include <unistd.h>
#endif

#endif

namespace comphelper
{
sal_Int64 getUsableMemory()
{
    sal_Int64 nUsable = -1;

#if defined UNX

#if defined MACOSX
    vm_size_t nPageSize;
    if (host_page_size(mach_host_self(), &nPageSize) == KERN_SUCCESS)
    {
        mach_msg_type_number_t nCount = HOST_VM_INFO_COUNT;
        vm_statistics_data_t aVMStat;
        if (host_statistics(mach_host_self(), HOST_VM_INFO, (host_info_t)&aVMStat, &nCount)
            == KERN_SUCCESS)
            nUsable = static_cast<sal_Int64>(aVMStat.free_count) * nPageSize;
    }
#else
    // TODO: specialize for other unixes here.
    // Report the physical memory as the usable memory remaining in case of Linux.
    sal_Int64 nPages = sysconf(_SC_AVPHYS_PAGES);
    sal_Int64 nPageSize = sysconf(_SC_PAGESIZE);
    if (nPages > 0 && nPageSize > 0)
        nUsable = nPages * nPageSize;
#endif

#elif defined(_WIN32)
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    GlobalMemoryStatusEx(&statex);
    nUsable = std::min(statex.ullAvailPhys, statex.ullAvailVirtual);
#endif

    return nUsable;
}

} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
