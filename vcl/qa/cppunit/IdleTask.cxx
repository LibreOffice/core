/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <vcl/svapp.hxx>
#include <vcl/idletask.hxx>
#include <salhelper/thread.hxx>

// This tests that IdleThread::waitUntilIdleDispatched actually waits until the main thread is idle
// and all event processing is actually completed instead of just waiting until all the events have
// just been popped off the queue.

namespace
{
void eventCallback(void*, void* pData)
{
    bool* pDone = static_cast<bool*>(pData);

    // Release the solar mutex for a little while. This shouldn’t
    // cause waitUntilIdleDispatched to return
    {
        SolarMutexReleaser aRelease;
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }

    *pDone = true;
}

struct OtherThread : public salhelper::Thread
{
    bool m_bDone;

    OtherThread()
        : salhelper::Thread("IdleTask test")
        , m_bDone(false)
    {
    }

protected:
    void execute() override;

private:
    DECL_LINK(setDone, void*, void);
};

void OtherThread::execute()
{
    bool bEventCallbackDone = false;

    Application::PostUserEvent(LINK_NONMEMBER(nullptr, eventCallback), &bEventCallbackDone);

    // Sleep for a little bit to make sure the main thread picks up
    // the pending event before this thread does
    std::this_thread::sleep_for(std::chrono::seconds(5));

    // Wait for idle. This should wait for eventCallback to actually
    // complete, not just that the main loop has started executing it.
    ::IdleTask::waitUntilIdleDispatched();

    // If this is false then eventCallback hasn’t actually completed
    CPPUNIT_ASSERT(bEventCallbackDone);

    // Post another event to wake up the main thread
    Application::PostUserEvent(LINK(this, OtherThread, setDone), nullptr);
}

IMPL_LINK_NOARG(OtherThread, setDone, void*, void) { m_bDone = true; }

CPPUNIT_TEST_FIXTURE(CppUnit::TestFixture, IdleTaskOtherThread)
{
    CPPUNIT_ASSERT(Application::IsMainThread());

    rtl::Reference<OtherThread> pOtherThread(new OtherThread);

    pOtherThread->launch();

    SolarMutexGuard aGuard;

    while (!pOtherThread->m_bDone)
        Application::Yield();

    pOtherThread->join();
}

CPPUNIT_TEST_FIXTURE(CppUnit::TestFixture, IdleTaskMainThread)
{
    CPPUNIT_ASSERT(Application::IsMainThread());

    bool bEventCallbackDone = false;

    Application::PostUserEvent(LINK_NONMEMBER(nullptr, eventCallback), &bEventCallbackDone);

    ::IdleTask::waitUntilIdleDispatched();

    CPPUNIT_ASSERT(bEventCallbackDone);
}
} // namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
