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

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/util/XUpdatable.hpp>
#include <rtl/ref.hxx>
#include <osl/thread.hxx>
#include <salhelper/thread.hxx>
#include <condition_variable>
#include <chrono>
#include <vector>
#include <mutex>

#include <helper/wakeupthread.hxx>

using namespace std::chrono_literals;

/// We only need one thread to wake everyone up.

namespace
{
class SharedWakeUpThread final : public salhelper::Thread
{
    static std::vector<css::uno::WeakReference<css::util::XUpdatable>> updatables;
    std::condition_variable condition;
    bool terminate;

public:
    static rtl::Reference<SharedWakeUpThread> wakeupThread;

    static std::mutex& getMutex()
    {
        static std::mutex mutex;
        return mutex;
    }

    SharedWakeUpThread()
        : Thread("WakeUpThread")
        , terminate(false)
    {
        assert(!wakeupThread);
        launch();
    }

    void execute() override
    {
        while (true)
        {
            std::unique_lock g(getMutex());
            condition.wait_for(g, 25ms, [this] { return terminate; });
            if (terminate || updatables.empty())
                break;

            auto copyOfUpdatables = updatables;
            g.unlock();

            for (auto& it : copyOfUpdatables)
            {
                css::uno::Reference<css::util::XUpdatable> up(it);
                if (up.is()) // check weak
                    up->update();
            }
        }

        std::unique_lock g(getMutex());
        if (updatables.empty())
        {
            terminate = false;
            wakeupThread.clear();
        }
    }

    static void startThread()
    {
        std::unique_lock g(getMutex());
        if (!updatables.empty() && !wakeupThread)
            wakeupThread = new SharedWakeUpThread();
    }

    void stopWithLock(std::unique_lock<std::mutex>& g)
    {
        terminate = true;
        condition.notify_one();
        g.unlock();

        join();
    }

    static void disposeThreadWithLock(std::unique_lock<std::mutex>& g)
    {
        if (wakeupThread)
        {
            auto holdRef = wakeupThread;
            wakeupThread.clear();
            holdRef->stopWithLock(g);
        }
        assert(!wakeupThread);
    }

    static void add(css::uno::WeakReference<css::util::XUpdatable> up)
    {
        std::unique_lock g(getMutex());
        updatables.push_back(up);
        if (!wakeupThread)
            wakeupThread = new SharedWakeUpThread();
    }

    static void remove(css::uno::WeakReference<css::util::XUpdatable> up)
    {
        std::unique_lock g(getMutex());
        auto it = updatables.begin();
        bool found = false;
        for (; it != updatables.end(); ++it)
        {
            css::uno::Reference<css::util::XUpdatable> itValid(*it);
            if (!itValid || *it == up)
            {
                it = updatables.erase(it);
                found = true;
                break;
            }
        }
        (void)found; assert(found);
        if (updatables.empty())
            disposeThreadWithLock(g);
    }

    static void joinThread()
    {
        std::unique_lock g(getMutex());
        disposeThreadWithLock(g);
    }
};

rtl::Reference<SharedWakeUpThread> SharedWakeUpThread::wakeupThread;
std::vector<css::uno::WeakReference<css::util::XUpdatable>> SharedWakeUpThread::updatables;
}

namespace framework
{
/* static */ void WakeUpThread::startThread() { SharedWakeUpThread::startThread(); }

WakeUpThread::WakeUpThread(css::uno::Reference<css::util::XUpdatable> const& up)
    : _updatable(up)
{
    assert(_updatable);
    SharedWakeUpThread::add(_updatable);
}

void WakeUpThread::stop() { SharedWakeUpThread::remove(_updatable); }

/* static */ void WakeUpThread::joinThread() { SharedWakeUpThread::joinThread(); }

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
