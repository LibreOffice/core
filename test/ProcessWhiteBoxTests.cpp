/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config.h>

#include <common/ProcUtil.hpp>

#include <test/lokassert.hpp>

#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>

#include <csignal>
#include <string_view>

#include <sys/wait.h>
#include <unistd.h>

/// Unit-tests for how WSD refers to the processes it watches. These run real child processes.
class ProcessWhiteBoxTests : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(ProcessWhiteBoxTests);
    CPPUNIT_TEST(testProcessHandleSeesExitedChildAsGone);
    CPPUNIT_TEST(testSignalReachesOnlyTheProcess);
    CPPUNIT_TEST_SUITE_END();

    void testProcessHandleSeesExitedChildAsGone();
    void testSignalReachesOnlyTheProcess();

public:
    void setUp() override
    {
        // These tests reap their children themselves.
        _oldSigChld = ::signal(SIGCHLD, SIG_DFL);
    }

    void tearDown() override { ::signal(SIGCHLD, _oldSigChld); }

private:
    void (*_oldSigChld)(int) = SIG_DFL;
};

namespace
{
/// Block until the child has exited, leaving it a zombie for the code under test to see.
bool waitUntilExited(pid_t pid)
{
    siginfo_t info{};
    return ::waitid(P_PID, pid, &info, WEXITED | WNOWAIT) == 0;
}
} // namespace

// A process that exited is gone, even while nobody reaped it yet, as when ForKit
// hasn't got to a Kit that WSD watches. kill(pid, 0) still finds it.
void ProcessWhiteBoxTests::testProcessHandleSeesExitedChildAsGone()
{
    constexpr std::string_view testname = __func__;

    int fds[2];
    LOK_ASSERT_EQUAL(0, ::pipe(fds));
    const pid_t pid = ::fork();
    LOK_ASSERT(pid >= 0);
    if (pid == 0)
    {
        // Exit once the parent closes its end.
        ::close(fds[1]);
        char c;
        while (::read(fds[0], &c, 1) > 0)
        {
        }
        ::_exit(0);
    }
    ::close(fds[0]);

    const ProcUtil::ProcessHandle process(pid);
#if defined(__linux__) && defined(SYS_pidfd_open)
    LOK_ASSERT_MESSAGE("Expected a pidfd on Linux", process.hasPidFd());
#endif
    LOK_ASSERT(process.isAlive());

    ::close(fds[1]);
    LOK_ASSERT(waitUntilExited(pid));

    const bool killFindsIt = ::kill(pid, 0) == 0;
    const bool alive = process.isAlive();

    int status = 0;
    ::waitpid(pid, &status, 0);

    LOK_ASSERT_MESSAGE("An unreaped child is still found by kill(2)", killFindsIt);
    if (process.hasPidFd())
        LOK_ASSERT_MESSAGE("A child that exited is gone, even while unreaped", !alive);
}

// Signals reach the process; once it is gone and reaped, signalling it is a no-op
// that says it is gone, rather than reaching whatever reuses its pid.
void ProcessWhiteBoxTests::testSignalReachesOnlyTheProcess()
{
    constexpr std::string_view testname = __func__;

    const pid_t pid = ::fork();
    LOK_ASSERT(pid >= 0);
    if (pid == 0)
    {
        ::pause();
        ::_exit(0);
    }

    const ProcUtil::ProcessHandle process(pid);
    LOK_ASSERT(process.isAlive());
    LOK_ASSERT(process.signal(SIGKILL));

    int status = 0;
    LOK_ASSERT_EQUAL(pid, ::waitpid(pid, &status, 0));
    LOK_ASSERT(WIFSIGNALED(status));
    LOK_ASSERT_EQUAL(SIGKILL, WTERMSIG(status));

    LOK_ASSERT(!process.isAlive());
    if (process.hasPidFd())
        LOK_ASSERT_MESSAGE("Signalling a process that is gone says so", process.signal(SIGTERM));
}

CPPUNIT_TEST_SUITE_REGISTRATION(ProcessWhiteBoxTests);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
