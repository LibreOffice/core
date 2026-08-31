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

/*
 * Server-side signal handling and process state management.
 * Functions: setTerminationSignals(), requestShutdown(), dumpState()
 */

#include <config.h>

#include "SigUtil.hpp"

#include <common/Common.hpp>
#include <common/Log.hpp>
#include <common/SigHandlerTrap.hpp>
#include <common/Util.hpp>
#include <net/Socket.hpp>
#include <test/testlog.hpp>

#include <array>
#include <atomic>
#include <cassert>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <poll.h>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>

#if defined(__GLIBC__)
#  include <execinfo.h>
#endif

#if !defined(ANDROID) && !defined(IOS) && !defined(MACOS) && !defined(__FreeBSD__)
#  include <sys/prctl.h>
#endif
#if defined(__FreeBSD__)
#  include <sys/procctl.h>
#endif


namespace
{
/// The valid states of the process.
enum class RunState : char
{
    Run = 0, ///< Normal up-and-running state.
    ShutDown, ///< Request to shut down gracefully.
    Terminate ///< Immediate termination.
};

/// Single flag to control the current run state.
std::atomic<RunState> RunStateFlag(RunState::Run);

std::atomic<bool> DumpGlobalState(false);
std::atomic<bool> ForwardSigUsr2Flag(false); ///< Flags to forward SIGUSR2 to children.

std::atomic<size_t> ActivityStringIndex = 0;
std::string ActivityHeader;
std::array<std::atomic<char*>, 16> ActivityStrings{};
bool UnattendedRun = false;
int SignalLogFD = STDERR_FILENO; ///< The FD where signalLogs are dumped.
char* VersionInfo = nullptr;
char FatalGdbString[256] = { '\0' };
SigUtil::SigChildHandler SigChildHandle;

} // namespace

namespace SigUtil
{
void triggerDumpState(const std::string &testname)
{
    TST_LOG("Dumping state");
    ::kill(getpid(), SIGUSR1);
}

void uninitialize()
{
    for (size_t i = 0; i < ActivityStrings.size(); ++i)
    {
        char* old = ActivityStrings[i].exchange(nullptr);
        free(old);
    }

    free(VersionInfo);
}

bool getShutdownRequestFlag() { return RunStateFlag >= RunState::ShutDown; }

bool getTerminationFlag() { return RunStateFlag >= RunState::Terminate; }

void setTerminationFlag()
{
    // While fuzzing, we never want to terminate.
    if constexpr (!Util::isFuzzing())
    {
        // Set the forced-termination flag.
        RunStateFlag = RunState::Terminate;
    }

    // And wake-up the thread.
    SocketPoll::wakeupWorld();
}

void requestShutdown()
{
    RunState oldState = RunState::Run;
    if (RunStateFlag.compare_exchange_strong(oldState, RunState::ShutDown))
        SocketPoll::wakeupWorld();
}

void resetTerminationFlags()
{
}

    void checkDumpGlobalState(GlobalDumpStateFn dumpState)
    {
        assert(dumpState && "Invalid callback for checkDumpGlobalState");
        if (DumpGlobalState)
        {
            DumpGlobalState = false;
            dumpState();
        }
    }

    void checkForwardSigUsr2(ForwardSigUsr2Fn forwardSigUsr2)
    {
        assert(forwardSigUsr2 && "Invalid callback for checkForwardSigUsr2");
        if (ForwardSigUsr2Flag)
        {
            ForwardSigUsr2Flag = false;
            forwardSigUsr2();
        }
    }

    void setActivityHeader(const std::string &message)
    {
        ActivityHeader = message;
    }

    void addActivity(const std::string &message)
    {
        char *old = ActivityStrings[ActivityStringIndex++ % ActivityStrings.size()].exchange(
            strdup(message.c_str()));
        if (old)
            free (old);
    }

    void addActivity(const std::string &viewId, const std::string &message)
    {
        addActivity("session: " + viewId + ": " + message);
    }

    void setUnattended()
    {
        UnattendedRun = true;
    }

    std::pair<int, int> reapZombieChild(int pid, bool sighandler)
    {
        if (!sighandler)
            LOG_TRC("Reaping " << pid << " with (WUNTRACED | WNOHANG)");

        int status = 0;
        pid_t ret = 0;
        if ((ret = ::waitpid(pid, &status, WUNTRACED | WNOHANG)) > 0)
        {
            if (!sighandler)
                LOG_DBG("Child " << ret << " terminated with status " << status);

            if (WIFSIGNALED(status) && (WTERMSIG(status) == SIGSEGV || WTERMSIG(status) == SIGBUS ||
                                        WTERMSIG(status) == SIGABRT))
            {
                if (!sighandler)
                    LOG_WRN("Zombie child " << ret << " has exited due to "
                                            << signalName(WTERMSIG(status)));
                return std::make_pair(ret, WTERMSIG(status));
            }
        }
        else if (pid > 0 && errno != 0) // Don't complain if the process is reaped already.
        {
            // Log errno if we had a child pid we expected to reap.
            if (!sighandler)
                LOG_WRN_SYS("Failed to reap child process " << pid);
        }

        return std::make_pair(ret, 0);
    }

    /// Open the signalLog file.
    void signalLogOpen()
    {
        // Always default to stderr.
        SignalLogFD = STDERR_FILENO;
    }

    /// Close the signalLog file.
    void signalLogClose()
    {
        // We cannot shutdown the logging subsystem
        // because freeing memory is not signal safe.

        // Flush the IO buffers.
        fflush(stdout);
        fflush(stderr);

        fsync(SignalLogFD);
    }

    void signalLogPrefix() { signalLog(Log::prefix("SIG").data()); }

    // We need a signal safe means of writing messages
    //   $ man 7 signal
    void signalLog(const char *message)
    {
        while (true)
        {
            const int length = std::strlen(message);
            const int written = write(SignalLogFD, message, length);
            if (written < 0)
            {
                if (errno == EINTR)
                    continue; // ignore.
                else
                    break;
            }

            message += written;
            if (message[0] == '\0')
                break;
        }
    }

    // We need a signal safe means of writing messages
    //   $ man 7 signal
    void signalLogNumber(std::size_t num, int base)
    {
        int i;
        char buf[22];
        if (num == 0)
        {
            signalLog("0");
            return;
        }
        buf[21] = '\0';
        assert (base == 10 || base == 16);
        for (i = 20; i > 0 && num > 0; --i)
        {
            int d = num % base;
            buf[i] = (d < 10) ? ('0' + d) : ('a' + d - 10);
            num /= base;
        }
        signalLog(buf + i + 1);
    }

    const char *signalName(const int signo)
    {
        // LCOV_EXCL_START Coverage for these is not very useful.
        switch (signo)
        {
#define CASE(x) case SIG##x: return "SIG" #x
            CASE(HUP);
            CASE(INT);
            CASE(QUIT);
            CASE(ILL);
            CASE(ABRT);
            CASE(FPE);
            CASE(KILL);
            CASE(SEGV);
            CASE(PIPE);
            CASE(ALRM);
            CASE(TERM);
            CASE(USR1);
            CASE(USR2);
            CASE(CHLD);
            CASE(CONT);
            CASE(STOP);
            CASE(TSTP);
            CASE(TTIN);
            CASE(TTOU);
            CASE(BUS);
#ifdef SIGPOLL
            CASE(POLL);
#endif
            CASE(PROF);
            CASE(SYS);
            CASE(TRAP);
            CASE(URG);
            CASE(VTALRM);
            CASE(XCPU);
            CASE(XFSZ);
#ifdef SIGEMT
            CASE(EMT);
#endif
#ifdef SIGSTKFLT
            CASE(STKFLT);
#endif
#if defined(SIGIO) && defined(SIGPOLL) && SIGIO != SIGPOLL
            CASE(IO);
#endif
#ifdef SIGPWR
            CASE(PWR);
#endif
#ifdef SIGLOST
            CASE(LOST);
#endif
            CASE(WINCH);
#if defined(SIGINFO) && defined(SIGPWR) && SIGINFO != SIGPWR
            CASE(INFO);
#endif
#undef CASE
        default:
            return "unknown";
        }
        // LCOV_EXCL_STOP Coverage for these is not very useful.
    }

    static
    void handleTerminationSignal(const int signal)
    {
        const auto onrre = errno; // Save.

        bool hardExit = false;
        const char* domain;
        RunState oldState = RunState::Run;
        if ((signal == SIGINT || signal == SIGTERM) &&
            RunStateFlag.compare_exchange_strong(oldState, RunState::ShutDown))
        {
            domain = " Shutdown signal received: ";
        }
        else
        {
            assert(RunStateFlag > RunState::Run && "Must have had Terminate flag");
            oldState = RunState::ShutDown;
            if (RunStateFlag.compare_exchange_strong(oldState, RunState::Terminate))
            {
                domain = " Forced-Termination signal received: ";
            }
            else
            {
                assert(RunStateFlag == RunState::Terminate && "Must have had Terminate flag");
                domain = " ok, ok - hard-termination signal received: ";
                hardExit = true;
            }
        }

        signalLogOpen();
        signalLogPrefix();
        signalLog(domain);
        signalLog(signalName(signal));
        signalLog("\n");
        signalLogClose();

        if (!hardExit)
            SocketPoll::wakeupWorld();
        else
        {
#if CODE_COVERAGE
            __gcov_dump();
#endif

            ::signal (signal, SIG_DFL);
            errno = onrre; // Restore.
            ::raise (signal);
        }

        errno = onrre; // Restore.
    }

    void signalLogActivity()
    {
        signalLog("Recent activity:\n");
        signalLog(ActivityHeader.c_str());
        size_t startIndex = ActivityStringIndex;
        for (size_t i = 0; i < ActivityStrings.size(); ++i)
        {
            size_t idx = (startIndex + i) % ActivityStrings.size();
            const char *str = ActivityStrings[idx];
            if (str && str[0] != '\0')
            {
                // no plausible impl. will heap allocate in c_str.
                signalLog("\t");
                signalLog(str);
                signalLog("\n");
            }
        }
    }

    static
    void handleFatalSignal(const int signal, siginfo_t *info, void * /* uctxt */)
    {
        SigHandlerTrap guard;
        const bool reEntered = !SigHandlerTrap::isExclusive();

        if (!reEntered)
            signalLogOpen();

        signalLogPrefix();

        // Heap corruption can re-enter through backtrace.
        if (reEntered)
            signalLog(" Fatal double signal received: ");
        else
            signalLog(" Fatal signal received: ");
        signalLog(signalName(signal));
        if (info)
        {
            signalLog(" code: ");
            signalLogNumber(static_cast<std::size_t>(info->si_code), 16);
            // Also print in decimal.
            if (info->si_code < 0)
            {
                signalLog("(-");
                signalLogNumber(static_cast<std::size_t>(-info->si_code), 10);
            }
            else
            {
                signalLog("(");
                signalLogNumber(static_cast<std::size_t>(info->si_code), 10);
            }

            signalLog(") for address: 0x");
            signalLogNumber(reinterpret_cast<std::size_t>(info->si_addr), 16);
        }
        signalLog("\n");

        signalLogActivity();

        struct sigaction action;

        sigemptyset(&action.sa_mask);
        action.sa_flags = 0;
        action.sa_handler = SIG_DFL;

        sigaction(signal, &action, nullptr);

        if (!reEntered)
        {
            dumpBacktrace();
            signalLogClose();
        }

        // let default handler process the signal
        ::raise(signal);
    }

    void dumpBacktrace()
    {
#if defined(__GLIBC__)
        signalLog("\nBacktrace ");
        signalLogNumber(static_cast<std::size_t>(getpid()));
        if (VersionInfo)
        {
            signalLog(" - ");
            signalLog(VersionInfo);
        }
        signalLog(":\n");

        const int maxSlots = 50;
        void *backtraceBuffer[maxSlots];
        const int numSlots = backtrace(backtraceBuffer, maxSlots);
        if (numSlots > 0)
        {
            backtrace_symbols_fd(backtraceBuffer, numSlots, SignalLogFD);
        }
#else
        LOG_INF("Backtrace not available on Android.");
#endif

#if !ENABLE_DEBUG
        if (std::getenv("COOL_DEBUG"))
#endif
        {
            if (UnattendedRun)
            {
                static constexpr std::string_view msg =
                    "Crashed in unattended run and won't wait for debugger. Re-run without "
                    "--unattended to attach a debugger.";
                std::cerr << msg << std::endl;
                char gdb[1024];
                snprintf(gdb, sizeof(gdb), "gdb --pid %d -batch -ex='thread apply all backtrace full'", getpid());
                if (system(gdb) != 0)
                    std::cerr << "Error when executing command: " << gdb << std::endl;
            }
            else
            {
                signalLog(FatalGdbString);
                std::cerr << "Sleeping 60s to allow debugging: attach " << getpid() << std::endl;
                sleep(60);
                std::cerr << "Finished sleeping to allow debugging of: " << getpid() << std::endl;
            }
        }
    }

    void setVersionInfo(const std::string &versionInfo)
    {
        if (VersionInfo)
            free (VersionInfo);
        VersionInfo = strdup(versionInfo.c_str());
    }

    void setFatalSignals(const std::string &versionInfo)
    {
        struct sigaction action;

        setVersionInfo(versionInfo);

        // Set up the fatal-signal handler. (N.B. three-argument handler)
        sigemptyset(&action.sa_mask);
        action.sa_flags = SA_SIGINFO;
        action.sa_sigaction = handleFatalSignal;

        sigaction(SIGSEGV, &action, nullptr);
        sigaction(SIGBUS, &action, nullptr);
        sigaction(SIGABRT, &action, nullptr);
        sigaction(SIGILL, &action, nullptr);
        sigaction(SIGFPE, &action, nullptr);

        // Set up the terminatio-signal handler. (N.B. single-argument handler)
        sigemptyset(&action.sa_mask);
        action.sa_flags = 0;
        action.sa_handler = handleTerminationSignal;

        sigaction(SIGINT, &action, nullptr);
        sigaction(SIGTERM, &action, nullptr);
        sigaction(SIGQUIT, &action, nullptr);
        sigaction(SIGHUP, &action, nullptr);

        // Prepare this in advance just in case.
        std::ostringstream stream;
        stream << "\nERROR: Fatal signal! Attach debugger with:\n"
                  "gdb -iex 'set sysroot /' --pid=" << getpid() << "\n or \n"
                  "gdb -iex 'set sysroot /' --q --n --ex 'thread apply all backtrace full' --batch --pid="
               << getpid() << '\n';
        std::string streamStr = stream.str();
        assert(sizeof(FatalGdbString) > streamStr.size() + 1);
        strncpy(FatalGdbString, streamStr.c_str(), sizeof(FatalGdbString)-1);
        FatalGdbString[sizeof(FatalGdbString)-1] = '\0';
    }

    static
    void handleSigChild(const int /* signal */, siginfo_t *info, void * /* uctxt */)
    {
        SigChildHandle(info ? info->si_pid : -1);
    }

    void setSigChildHandler(SigChildHandler fn)
    {
        struct sigaction action;

        SigChildHandle = fn;
        sigemptyset(&action.sa_mask);

        if (fn)
        {
            action.sa_flags = SA_SIGINFO;
            action.sa_sigaction = handleSigChild;
        }
        else
        {
            action.sa_flags = 0;
            action.sa_handler = SIG_DFL;
        }

        sigaction(SIGCHLD, &action, nullptr);
    }

    void dieOnParentDeath()
    {
#if defined(__linux__) && !defined(ANDROID)
        prctl(PR_SET_PDEATHSIG, SIGKILL);
#endif
#if defined(__FreeBSD__)
        int sig = SIGKILL;
        procctl(P_PID, 0, PROC_PDEATHSIG_CTL, &sig);
#endif
    }

    static
    void handleUserSignal(const int signal)
    {
        signalLogOpen();
        signalLogPrefix();
        signalLog(" User signal received: ");
        signalLog(signalName(signal));
        signalLog("\n");
        if (signal == SIGUSR1)
        {
            DumpGlobalState = true;
        }
        else if (signal == SIGUSR2)
        {
#if defined(__GLIBC__)
            constexpr int maxSlots = 250;
            void* backtraceBuffer[maxSlots];
            const int numSlots = backtrace(backtraceBuffer, maxSlots);
            if (numSlots > 0)
                backtrace_symbols_fd(backtraceBuffer, numSlots, SignalLogFD);
#endif

            ForwardSigUsr2Flag = true;
        }

        signalLogClose();
        SocketPoll::wakeupWorld();
    }

    static
    void handleDebuggerSignal(const int /*signal*/)
    {}

    void setUserSignals()
    {
        struct sigaction action;

        sigemptyset(&action.sa_mask);
        action.sa_flags = 0;
        action.sa_handler = handleUserSignal;

        sigaction(SIGUSR1, &action, nullptr);
        sigaction(SIGUSR2, &action, nullptr);

#if defined(__GLIBC__)
        // Prime backtrace to make sure libgcc is loaded.
        constexpr int maxSlots = 1;
        void* backtraceBuffer[maxSlots + 1];
        backtrace(backtraceBuffer, maxSlots);
#endif
    }

    void setDebuggerSignal()
    {
        struct sigaction action;

        sigemptyset(&action.sa_mask);
        action.sa_flags = 0;
        action.sa_handler = handleDebuggerSignal;

        sigaction(SIGUSR1, &action, nullptr);
    }

    /// Kill the given pid with SIGKILL as default. Returns true when the pid does not exist any more.
    bool killChild(const int pid, const int signal)
    {
        LOG_DBG("Killing PID: " << pid << " with " << signalName(signal));

        // Don't kill anything in the fuzzer case: pid == 0 would kill the fuzzer itself, and
        // killing random other processes is not a great idea, either.
        if (Util::isFuzzing() || kill(pid, signal) == 0 || errno == ESRCH)
        {
            // Killed or doesn't exist.
            return true;
        }

        LOG_SYS("Error when trying to kill PID: " << pid << ". Will wait for termination.");

        constexpr int sleepMs = 50;
        constexpr int count = std::max(CHILD_REBALANCE_INTERVAL_MS / sleepMs, 2);
        for (int i = 0; i < count; ++i)
        {
            if (kill(pid, 0) == 0 || errno == ESRCH)
            {
                // Doesn't exist.
                return true;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(sleepMs));
        }

        return false;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
