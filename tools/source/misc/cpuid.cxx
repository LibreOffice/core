/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <tools/cpuid.hxx>
#include <cstdint>

namespace tools
{
namespace cpuid
{

namespace
{
#if defined(_MSC_VER)
#include <intrin.h>
void getCpuId(uint32_t array[4])
{
    __cpuid(reinterpret_cast<int*>(array), 1);
}
#else
#include <cpuid.h>
void getCpuId(uint32_t array[4])
{
    __get_cpuid(1, array + 0, array + 1, array + 2, array + 3);
}
#endif
}

#if defined(LO_SSE2_AVAILABLE)

bool hasSSE2()
{
    uint32_t cpuInfoArray[] = {0, 0, 0, 0};
    getCpuId(cpuInfoArray);
    return (cpuInfoArray[3] & (1 << 26)) != 0;
}

#else

bool hasSSE2() { return false; }

#endif

bool hasHyperThreading()
{
    uint32_t cpuInfoArray[] = {0, 0, 0, 0};
    getCpuId(cpuInfoArray);
    return (cpuInfoArray[3] & (1 << 28)) != 0;
}

}
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
