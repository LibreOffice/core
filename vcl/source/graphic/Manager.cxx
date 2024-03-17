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

#include <graphic/Manager.hxx>
#include <impgraph.hxx>
#include <sal/log.hxx>

#include <officecfg/Office/Common.hxx>
#include <unotools/configmgr.hxx>

using namespace css;

namespace
{
void setupConfigurationValuesIfPossible(sal_Int64& rMemoryLimit,
                                        std::chrono::seconds& rAllowedIdleTime, bool& bSwapEnabled)
{
    if (comphelper::IsFuzzing())
        return;

    try
    {
        using officecfg::Office::Common::Cache;

        rMemoryLimit = Cache::GraphicManager::GraphicMemoryLimit::get();
        rAllowedIdleTime
            = std::chrono::seconds(Cache::GraphicManager::GraphicAllowedIdleTime::get());
        bSwapEnabled = Cache::GraphicManager::GraphicSwappingEnabled::get();
    }
    catch (...)
    {
    }
}
}

namespace vcl::graphic
{
MemoryManager::MemoryManager()
    : maSwapOutTimer("MemoryManager::MemoryManager maSwapOutTimer")
{
    setupConfigurationValuesIfPossible(mnMemoryLimit, mnAllowedIdleTime, mbSwapEnabled);

    if (mbSwapEnabled)
    {
        maSwapOutTimer.SetInvokeHandler(LINK(this, MemoryManager, ReduceMemoryTimerHandler));
        maSwapOutTimer.SetTimeout(mnTimeout);
        maSwapOutTimer.Start();
    }
}

MemoryManager& MemoryManager::get()
{
    static MemoryManager gStaticManager;
    return gStaticManager;
}

IMPL_LINK(MemoryManager, ReduceMemoryTimerHandler, Timer*, pTimer, void)
{
    std::unique_lock aGuard(maMutex);
    pTimer->Stop();
    reduceMemory(aGuard);
    pTimer->Start();
}

void MemoryManager::registerObject(MemoryManaged* pMemoryManaged)
{
    std::unique_lock aGuard(maMutex);

    // Insert and update the used size (bytes)
    assert(aGuard.owns_lock() && aGuard.mutex() == &maMutex);
    // coverity[missing_lock: FALSE] - as above assert
    mnTotalSize += pMemoryManaged->getCurrentSizeInBytes();
    maObjectList.insert(pMemoryManaged);
}

void MemoryManager::unregisterObject(MemoryManaged* pMemoryManaged)
{
    std::unique_lock aGuard(maMutex);
    mnTotalSize -= pMemoryManaged->getCurrentSizeInBytes();
    maObjectList.erase(pMemoryManaged);
}

void MemoryManager::changeExisting(MemoryManaged* pMemoryManaged, sal_Int64 nNewSize)
{
    std::scoped_lock aGuard(maMutex);
    sal_Int64 nOldSize = pMemoryManaged->getCurrentSizeInBytes();
    mnTotalSize -= nOldSize;
    mnTotalSize += nNewSize;
    pMemoryManaged->setCurrentSizeInBytes(nNewSize);
}

void MemoryManager::swappedIn(MemoryManaged* pMemoryManaged, sal_Int64 nNewSize)
{
    changeExisting(pMemoryManaged, nNewSize);
}

void MemoryManager::swappedOut(MemoryManaged* pMemoryManaged, sal_Int64 nNewSize)
{
    changeExisting(pMemoryManaged, nNewSize);
}

void MemoryManager::reduceAllAndNow()
{
    std::unique_lock aGuard(maMutex);
    reduceMemory(aGuard, true);
}

void MemoryManager::dumpState(rtl::OStringBuffer& rState)
{
    std::unique_lock aGuard(maMutex);

    rState.append("\nMemory Manager items:\t");
    rState.append(static_cast<sal_Int32>(maObjectList.size()));
    rState.append("\tsize:\t");
    rState.append(static_cast<sal_Int64>(mnTotalSize / 1024));
    rState.append("\tkb");

    for (MemoryManaged* pMemoryManaged : maObjectList)
    {
        pMemoryManaged->dumpState(rState);
    }
}

void MemoryManager::reduceMemory(std::unique_lock<std::mutex>& rGuard, bool bDropAll)
{
    // maMutex is locked in callers

    if (!mbSwapEnabled)
        return;

    if (mnTotalSize < mnMemoryLimit && !bDropAll)
        return;

    // avoid recursive reduceGraphicMemory on reexport of tdf118346-1.odg to odg
    if (mbReducingGraphicMemory)
        return;

    mbReducingGraphicMemory = true;

    loopAndReduceMemory(rGuard, bDropAll);

    mbReducingGraphicMemory = false;
}

void MemoryManager::loopAndReduceMemory(std::unique_lock<std::mutex>& rGuard, bool bDropAll)
{
    // make a copy of m_pImpGraphicList because if we swap out a svg, the svg
    // filter may create more temp Graphics which are auto-added to
    // m_pImpGraphicList invalidating a loop over m_pImpGraphicList, e.g.
    // reexport of tdf118346-1.odg

    o3tl::sorted_vector<MemoryManaged*> aObjectListCopy = maObjectList;

    for (MemoryManaged* pMemoryManaged : aObjectListCopy)
    {
        if (!pMemoryManaged->canReduceMemory())
            continue;

        sal_Int64 nCurrentSizeInBytes = pMemoryManaged->getCurrentSizeInBytes();
        if (nCurrentSizeInBytes > mnSmallFrySize || bDropAll) // ignore small-fry
        {
            auto aCurrent = std::chrono::high_resolution_clock::now();
            auto aDeltaTime = aCurrent - pMemoryManaged->getLastUsed();
            auto aSeconds = std::chrono::duration_cast<std::chrono::seconds>(aDeltaTime);

            if (aSeconds > mnAllowedIdleTime)
            {
                // unlock because svgio can call back into us
                rGuard.unlock();
                pMemoryManaged->reduceMemory();
                rGuard.lock();
            }
        }
    }
}

} // end vcl::graphic

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
