/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/types.h>
#include <rtl/strbuf.hxx>
#include <vcl/timer.hxx>

#include <memory>
#include <mutex>
#include <chrono>
#include <o3tl/sorted_vector.hxx>

namespace vcl::graphic
{
class MemoryManaged;

class VCL_DLLPUBLIC MemoryManager final
{
private:
    o3tl::sorted_vector<MemoryManaged*> maObjectList;
    sal_Int64 mnTotalSize = 0;
    std::mutex maMutex; // instead of SolarMutex because graphics can live past vcl main

    std::chrono::seconds mnAllowedIdleTime = std::chrono::seconds(1);
    bool mbSwapEnabled = true;
    bool mbReducingGraphicMemory = false;
    sal_Int64 mnMemoryLimit = 10'000'000;
    Timer maSwapOutTimer;
    sal_Int32 mnTimeout = 1'000;
    sal_Int64 mnSmallFrySize = 100'000;

    DECL_LINK(ReduceMemoryTimerHandler, Timer*, void);

public:
    MemoryManager();
    void registerObject(MemoryManaged* pObject);
    void unregisterObject(MemoryManaged* pObject);
    void changeExisting(MemoryManaged* pObject, sal_Int64 nNewSize);

    void swappedIn(MemoryManaged* pObject, sal_Int64 nNewSize);
    void swappedOut(MemoryManaged* pObject, sal_Int64 nNewSize);

    static MemoryManager& get();
    o3tl::sorted_vector<MemoryManaged*> const& getManagedObjects() { return maObjectList; }
    sal_Int64 getTotalSize() { return mnTotalSize; }

    void reduceMemory(std::unique_lock<std::mutex>& rGuard, bool bDropAll = false);
    void loopAndReduceMemory(std::unique_lock<std::mutex>& rGuard, bool bDropAll = false);
    void reduceAllAndNow();
    void dumpState(rtl::OStringBuffer& rState);
};

} // end namespace vcl::graphic

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
