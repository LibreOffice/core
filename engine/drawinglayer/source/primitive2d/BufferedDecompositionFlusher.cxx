/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <sal/config.h>
#include <comphelper/solarmutex.hxx>
#include <drawinglayer/primitive2d/BufferedDecompositionFlusher.hxx>

namespace drawinglayer::primitive2d
{
/**
    This is a "garbage collection" approach to flushing.

    We store entries in a set. Every 2 seconds, we scan the set for entries that have not
    been used for 10 seconds or more, and if so, we flush the buffer primitives in those entries.

    This mechanism is __deliberately__ not perfect.
    Sometimes things will be flushed a little too soon, sometimes things will wait a little too long,
    since we only have a granularity of 2 seconds.
    But what is gains from not being perfect, is scalability.

    It is very simple, scales to lots and lots of primitives without needing lots of timers, and performs
    very little work in the common case.

    Shutdown notes
    --------------------
    The process of handling shutdown is more complicated here than it should be, because we are interacting with
    various vcl-level things (by virtue of calling into drawinglayer primitives that use vcl facilities), but we
    do not have access to vcl-level API here (like SolarMutexReleaser and vcl::Timer).
*/

namespace
{
std::unique_ptr<BufferedDecompositionFlusher> g_pInstance;

BufferedDecompositionFlusher* getOrCreateInstance()
{
    if (!g_pInstance)
        g_pInstance.reset(new BufferedDecompositionFlusher);
    return g_pInstance.get();
}
}

// static
void BufferedDecompositionFlusher::shutdown()
{
    if (!g_pInstance)
        return;

    g_pInstance->onTeardown();
    g_pInstance->join();
    g_pInstance->flushPending();
    g_pInstance.reset();
}

// static
void BufferedDecompositionFlusher::update(const BufferedDecompositionPrimitive2D* p)
{
    getOrCreateInstance()->updateImpl(p);
}

// static
void BufferedDecompositionFlusher::update(const BufferedDecompositionGroupPrimitive2D* p)
{
    getOrCreateInstance()->updateImpl(p);
}

// static
void BufferedDecompositionFlusher::remove(const BufferedDecompositionPrimitive2D* p)
{
    if (g_pInstance)
        g_pInstance->removeImpl(p);
}

// static
void BufferedDecompositionFlusher::remove(const BufferedDecompositionGroupPrimitive2D* p)
{
    if (g_pInstance)
        g_pInstance->removeImpl(p);
}

BufferedDecompositionFlusher::BufferedDecompositionFlusher() { create(); }

void BufferedDecompositionFlusher::updateImpl(const BufferedDecompositionPrimitive2D* p)
{
    std::unique_lock l(maMutex);
    if (!mbShutdown)
    {
        unotools::WeakReference<BufferedDecompositionPrimitive2D> xRef(
            const_cast<BufferedDecompositionPrimitive2D*>(p));
        maRegistered1.insert({ p, std::move(xRef) });
    }
}

void BufferedDecompositionFlusher::updateImpl(const BufferedDecompositionGroupPrimitive2D* p)
{
    std::unique_lock l(maMutex);
    if (!mbShutdown)
    {
        unotools::WeakReference<BufferedDecompositionGroupPrimitive2D> xRef(
            const_cast<BufferedDecompositionGroupPrimitive2D*>(p));
        maRegistered2.insert({ p, std::move(xRef) });
    }
}

void BufferedDecompositionFlusher::removeImpl(const BufferedDecompositionPrimitive2D* p)
{
    std::unique_lock l(maMutex);
    if (!mbShutdown)
        maRegistered1.erase(p);
}

void BufferedDecompositionFlusher::removeImpl(const BufferedDecompositionGroupPrimitive2D* p)
{
    std::unique_lock l(maMutex);
    if (!mbShutdown)
        maRegistered2.erase(p);
}

BufferedDecompositionFlusher::FlushBatch
BufferedDecompositionFlusher::collectRemoved(std::chrono::steady_clock::duration aMinIdleAge)
{
    auto aNow = std::chrono::steady_clock::now();
    FlushBatch aBatch;
    std::vector<rtl::Reference<BasePrimitive2D>> aDelayRelease;
    {
        std::unique_lock l(maMutex);
        for (auto it = maRegistered1.begin(); it != maRegistered1.end();)
        {
            rtl::Reference<BufferedDecompositionPrimitive2D> xPrim = it->second.get();
            if (!xPrim)
                it = maRegistered1.erase(it);
            else if (aNow - xPrim->maLastAccess.load() >= aMinIdleAge)
            {
                aBatch.mRemoved1.push_back(std::move(xPrim));
                it = maRegistered1.erase(it);
            }
            else
            {
                aDelayRelease.push_back(std::move(xPrim));
                ++it;
            }
        }
        for (auto it = maRegistered2.begin(); it != maRegistered2.end();)
        {
            rtl::Reference<BufferedDecompositionGroupPrimitive2D> xPrim = it->second.get();
            if (!xPrim)
                it = maRegistered2.erase(it);
            else if (aNow - xPrim->maLastAccess.load() >= aMinIdleAge)
            {
                aBatch.mRemoved2.push_back(std::move(xPrim));
                it = maRegistered2.erase(it);
            }
            else
            {
                aDelayRelease.push_back(std::move(xPrim));
                ++it;
            }
        }
    }
    // There is a very very small window where, if :
    // This-thread: we create a strong reference from a weak reference inside the loop
    // Another-thread: releases the second last strong reference to the the object
    // This-thread: we clear the reference, which triggers object destruction, which tries to call back
    //  into BufferedDecompositionFlusher and then deadlocks because the mutex is already acquired.
    aDelayRelease.clear();
    return aBatch;
}

// static
void BufferedDecompositionFlusher::flushRemoved(FlushBatch& rBatch)
{
    // some parts of skia do not take kindly to being accessed from multiple threads
    osl::Guard<comphelper::SolarMutex> aGuard(comphelper::SolarMutex::get());
    for (const auto& xPrim : rBatch.mRemoved1)
        xPrim->setBuffered2DDecomposition(nullptr);
    for (const auto& xPrim : rBatch.mRemoved2)
        xPrim->setBuffered2DDecomposition(Primitive2DContainer{});
    // Clear under SolarMutex, in case we are the sole surviving reference and
    // destruction would touch vcl resources.
    rBatch.mRemoved1.clear();
    rBatch.mRemoved2.clear();
}

void SAL_CALL BufferedDecompositionFlusher::run()
{
    setName("BufferedDecompositionFlusher");
    while (true)
    {
        FlushBatch aBatch = collectRemoved(std::chrono::seconds(10));
        flushRemoved(aBatch);

        std::unique_lock l(maMutex);
        if (mbShutdown || maDelayOrTerminate.wait_for(l, std::chrono::seconds(2), [this] {
                return mbShutdown;
            }))
            break;
    }
}

void BufferedDecompositionFlusher::onTeardown()
{
    {
        std::unique_lock l2(maMutex);
        mbShutdown = true;
    }
    maDelayOrTerminate.notify_all();
}

/// Flush every registered entry. Must be called only after the run loop has
/// exited (i.e. after join()), so we do not contend with it for SolarMutex
/// or for the registration maps.
void BufferedDecompositionFlusher::flushPending()
{
    FlushBatch aBatch = collectRemoved(std::chrono::steady_clock::duration::zero());
    flushRemoved(aBatch);
}

} // end of namespace drawinglayer::primitive2d

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
