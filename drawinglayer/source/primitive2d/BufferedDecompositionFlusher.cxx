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

#include <drawinglayer/primitive2d/BufferedDecompositionFlusher.hxx>
#include <tools/lazydelete.hxx>

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
*/
namespace
{
class FlusherDeinit : public tools::DeleteOnDeinitBase
{
    rtl::Reference<BufferedDecompositionFlusher> m_xTimer;
    virtual void doCleanup() override
    {
        m_xTimer->stop();
        m_xTimer = nullptr;
    }

public:
    FlusherDeinit()
    {
        m_xTimer = new BufferedDecompositionFlusher;
        addDeinitContainer(this);
    }

    BufferedDecompositionFlusher* get() { return m_xTimer.get(); }
};
}

static BufferedDecompositionFlusher* getInstance()
{
    static FlusherDeinit gaTimer;
    return gaTimer.get();
}

// static
void BufferedDecompositionFlusher::update(const BufferedDecompositionPrimitive2D* p)
{
    if (auto flusher = getInstance())
        flusher->updateImpl(p);
}

// static
void BufferedDecompositionFlusher::update(const BufferedDecompositionGroupPrimitive2D* p)
{
    if (auto flusher = getInstance())
        flusher->updateImpl(p);
}

BufferedDecompositionFlusher::BufferedDecompositionFlusher()
{
    setRemainingTime(salhelper::TTimeValue(2, 0));
    start();
}

void BufferedDecompositionFlusher::updateImpl(const BufferedDecompositionPrimitive2D* p)
{
    std::unique_lock l(maMutex);
    maRegistered1.insert(const_cast<BufferedDecompositionPrimitive2D*>(p));
}

void BufferedDecompositionFlusher::updateImpl(const BufferedDecompositionGroupPrimitive2D* p)
{
    std::unique_lock l(maMutex);
    maRegistered2.insert(const_cast<BufferedDecompositionGroupPrimitive2D*>(p));
}

void SAL_CALL BufferedDecompositionFlusher::onShot()
{
    auto aNow = std::chrono::steady_clock::now();
    std::vector<rtl::Reference<BufferedDecompositionPrimitive2D>> aRemoved1;
    std::vector<rtl::Reference<BufferedDecompositionGroupPrimitive2D>> aRemoved2;
    {
        std::unique_lock l(maMutex);
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
    for (const auto& r : aRemoved1)
        r->setBuffered2DDecomposition(nullptr);
    for (const auto& r : aRemoved2)
        r->setBuffered2DDecomposition(Primitive2DContainer{});
    setRemainingTime(salhelper::TTimeValue(2, 0));
    start();
}

} // end of namespace drawinglayer::primitive2d

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
