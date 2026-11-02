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
#include <wsd/Process.hpp>

#include <test/lokassert.hpp>

#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>

#include <chrono>
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
    CPPUNIT_TEST(testClosedChildThatStillRunsIsKilled);
    CPPUNIT_TEST(testClosedChildThatExitedIsNotKilled);
    CPPUNIT_TEST_SUITE_END();

    void testProcessHandleSeesExitedChildAsGone();
    void testSignalReachesOnlyTheProcess();
    void testClosedChildThatStillRunsIsKilled();
    void testClosedChildThatExitedIsNotKilled();

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

/// Fork a child that runs until a signal ends it.
pid_t forkStuck()
{
    const pid_t pid = ::fork();
    if (pid == 0)
    {
        for (;;)
            ::pause();
    }
    return pid;
}

/// Fork a child that exits with code 0 once the parent closes the returned descriptor.
pid_t forkExitingOnClose(int& fd)
{
    int fds[2];
    if (::pipe(fds) != 0)
        return -1;

    const pid_t pid = ::fork();
    if (pid == 0)
    {
        ::close(fds[1]);
        char c;
        while (::read(fds[0], &c, 1) > 0)
        {
        }
        ::_exit(0);
    }

    ::close(fds[0]);
    fd = fds[1];
    return pid;
}

/// A child process that WSD talks to, without a socket to talk on.
class TestProcess final : public WSProcess
{
public:
    explicit TestProcess(pid_t pid)
        : WSProcess("TestProcess", pid, nullptr, nullptr)
    {
    }
};
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

// A child that doesn't exit when asked to, as a Kit whose main thread is stuck, is still seen
// running after it was closed, and is killed.
void ProcessWhiteBoxTests::testClosedChildThatStillRunsIsKilled()
{
    constexpr std::string_view testname = __func__;

    const pid_t pid = forkStuck();
    LOK_ASSERT(pid > 0);

    {
        TestProcess process(pid);
        process.close();
        LOK_ASSERT_EQUAL(-1, static_cast<int>(process.getPid()));
        LOK_ASSERT_MESSAGE("A closed child that ignores the request to exit still runs",
                           process.isProcessRunning());

        process.killIfRunning();
        LOK_ASSERT(waitUntilExited(pid));
        // Only a pidfd tells a child that exited from one that nobody reaped yet.
        if (process.hasProcessPidFd())
            LOK_ASSERT_MESSAGE("A killed child is gone", !process.isProcessRunning());
    }

    int status = 0;
    LOK_ASSERT_EQUAL(pid, ::waitpid(pid, &status, 0));
    LOK_ASSERT_MESSAGE("The stuck child is killed", WIFSIGNALED(status));
}

// A child that exits by itself once asked to is seen as gone, and is left to end the way it chose.
void ProcessWhiteBoxTests::testClosedChildThatExitedIsNotKilled()
{
    constexpr std::string_view testname = __func__;

    int fd = -1;
    const pid_t pid = forkExitingOnClose(fd);
    LOK_ASSERT(pid > 0);

    {
        TestProcess process(pid);
        process.close();
        ::close(fd);
        LOK_ASSERT(waitUntilExited(pid));
        if (process.hasProcessPidFd())
            LOK_ASSERT_MESSAGE("A closed child that exited is gone", !process.isProcessRunning());

        process.killIfRunning();
    }

    int status = 0;
    LOK_ASSERT_EQUAL(pid, ::waitpid(pid, &status, 0));
    LOK_ASSERT_MESSAGE("The child exits by itself", WIFEXITED(status));
    LOK_ASSERT_EQUAL(0, WEXITSTATUS(status));
}

CPPUNIT_TEST_SUITE_REGISTRATION(ProcessWhiteBoxTests);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
