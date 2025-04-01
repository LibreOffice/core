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

#include <sal/config.h>
#include <sal/log.hxx>
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

static BufferedDecompositionFlusher* getInstance()
{
    static std::unique_ptr<BufferedDecompositionFlusher> gaTimer(new BufferedDecompositionFlusher);
    return gaTimer.get();
}

// static
void BufferedDecompositionFlusher::shutdown()
{
    SAL_WARN("drawinglayer", "tearing down BufferedDecompositionFlusher");
    BufferedDecompositionFlusher* pFlusher = getInstance();
    pFlusher->onTeardown();
    // We have to wait for the thread to exit, otherwise we might end up with the background thread
    // trying to process stuff while it has things ripped out underneath it.
    pFlusher->join();
}

// static
void BufferedDecompositionFlusher::update(const BufferedDecompositionPrimitive2D* p)
{
    getInstance()->updateImpl(p);
}

// static
void BufferedDecompositionFlusher::update(const BufferedDecompositionGroupPrimitive2D* p)
{
    getInstance()->updateImpl(p);
}

BufferedDecompositionFlusher::BufferedDecompositionFlusher()
{
    setName("BufferedDecompositionFlusher");
    create();
}

void BufferedDecompositionFlusher::updateImpl(const BufferedDecompositionPrimitive2D* p)
{
    std::unique_lock l(maMutex);
    if (!mbShutdown)
        maRegistered1.insert(const_cast<BufferedDecompositionPrimitive2D*>(p));
}

void BufferedDecompositionFlusher::updateImpl(const BufferedDecompositionGroupPrimitive2D* p)
{
    std::unique_lock l(maMutex);
    if (!mbShutdown)
        maRegistered2.insert(const_cast<BufferedDecompositionGroupPrimitive2D*>(p));
}

void SAL_CALL BufferedDecompositionFlusher::run()
{
    for (;;)
    {
        auto aNow = std::chrono::steady_clock::now();
        std::vector<rtl::Reference<BufferedDecompositionPrimitive2D>> aRemoved1;
        std::vector<rtl::Reference<BufferedDecompositionGroupPrimitive2D>> aRemoved2;
        {
            std::unique_lock l1(maMutex);
            // exit if we have been shutdown
            if (mbShutdown)
                break;
            for (auto it = maRegistered1.begin(); it != maRegistered1.end();)
            {
                if (aNow - (*it)->maLastAccess > std::chrono::seconds(10))
                {
                    aRemoved1.push_back(*it);
                    it = maRegistered1.erase(it);
                }
                else
                    ++it;
            }
            for (auto it = maRegistered2.begin(); it != maRegistered2.end();)
            {
                if (aNow - (*it)->maLastAccess > std::chrono::seconds(10))
                {
                    aRemoved2.push_back(*it);
                    it = maRegistered2.erase(it);
                }
                else
                    ++it;
            }
        }

        {
            // some parts of skia do not take kindly to being accessed from multiple threads
            osl::Guard<comphelper::SolarMutex> aGuard(comphelper::SolarMutex::get());

            for (const auto& r : aRemoved1)
                r->setBuffered2DDecomposition(nullptr);
            for (const auto& r : aRemoved2)
                r->setBuffered2DDecomposition(Primitive2DContainer{});
        }

        wait(TimeValue(2, 0));
    }
}

/// Only called by FlusherDeinit
void BufferedDecompositionFlusher::onTeardown()
{
    std::unordered_set<rtl::Reference<BufferedDecompositionPrimitive2D>> aRemoved1;
    std::unordered_set<rtl::Reference<BufferedDecompositionGroupPrimitive2D>> aRemoved2;
    {
        std::unique_lock l2(maMutex);
        mbShutdown = true;
        aRemoved1 = std::move(maRegistered1);
        aRemoved2 = std::move(maRegistered2);
    }
    // let the destruction happen outside the lock
}

} // end of namespace drawinglayer::primitive2d

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
