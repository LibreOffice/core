/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <sharedstringpoolpurge.hxx>

namespace sc
{
SharedStringPoolPurge::SharedStringPoolPurge()
    : mTimer("SharedStringPoolPurge")
{
    mTimer.SetPriority(TaskPriority::LOWEST);
    mTimer.SetTimeout(10000); // 10 sec
    mTimer.SetInvokeHandler(LINK(this, SharedStringPoolPurge, timerHandler));
}

SharedStringPoolPurge::~SharedStringPoolPurge() { cleanup(); }

void SharedStringPoolPurge::delayedPurge(const std::shared_ptr<svl::SharedStringPool>& pool)
{
    if (std::find(mPoolsToPurge.begin(), mPoolsToPurge.end(), pool) == mPoolsToPurge.end())
    {
        mPoolsToPurge.push_back(pool);
        mTimer.Start();
    }
}

void SharedStringPoolPurge::cleanup()
{
    for (std::shared_ptr<svl::SharedStringPool>& pool : mPoolsToPurge)
    {
        if (pool.use_count() > 1)
            pool->purge();
    }
    mPoolsToPurge.clear();
}

IMPL_LINK_NOARG(SharedStringPoolPurge, timerHandler, Timer*, void)
{
    mTimer.Stop();
    cleanup();
}

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
