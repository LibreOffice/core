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
 * A very simple, single threaded helper to efficiently pre-init and
 * spawn lots of kits as children.
 */

#include <config.h>

#include <common/Common.hpp>
#include <common/ConfigUtil.hpp>
#include <common/FileUtil.hpp>
#include <common/JailUtil.hpp>
#include <common/Log.hpp>
#include <common/NumUtil.hpp>
#include <common/Seccomp.hpp>
#include <common/SigUtil.hpp>
#include <common/Simd.hpp>
#include <common/Unit.hpp>
#include <common/Uri.hpp>
#include <common/Util.hpp>
#include <common/Watchdog.hpp>
#include <common/security.h>
#include <kit/DeltaSimd.h>
#include <kit/Kit.hpp>
#include <kit/SetupKitEnvironment.hpp>
#include <net/ServerSocket.hpp>
#include <net/WebSocketHandler.hpp>

#define KIT_USE_UNSTABLE_API
#include <COKit/COKit.hxx>

#include <Poco/Path.h>
#include <Poco/URI.h>

#if HAVE_LIBCAP
#include <sys/capability.h>
#endif
#include <sys/types.h>
#include <sys/wait.h>
#include <sysexits.h>

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <thread>
#include <utility>

namespace
{

bool NoCapsForKit = false;
bool NoSeccomp = false;
#if ENABLE_DEBUG
bool SingleKit = false;
#endif

int parentPid;

std::string ForKitIdent;

std::string UserInterface;

bool DisplayVersion = false;
std::string UnitTestLibrary;
std::string LogLevel;
std::string LogDisabledAreas;
std::string LogLevelStartup;
std::atomic<unsigned> ForkCounter(0);
std::vector<std::string> SubForKitRequests;

/// The [child pid -> jail path] map.
std::map<pid_t, std::string> childJails;
/// The jails that need cleaning up. This should be small.
std::vector<std::string> cleanupJailPaths;
/// The [subforkit pid -> subforkit id] map.
std::map<pid_t, std::string> subForKitPids;

/// The Main polling main-loop of this (single threaded) process
std::unique_ptr<SocketPoll> ForKitPoll;

} // namespace

extern "C" { void dump_forkit_state(void); /* easy for gdb */ }

void dump_forkit_state()
{
    std::ostringstream oss(Util::makeDumpStateStream());
    oss << "Start ForKit " << getpid() << " Dump State:\n";

    oss << "Forkit: " << ForkCounter << " forks\n"
        << "  LogLevel: " << LogLevel << "\n"
        << "  LogDisabledAreas: " << LogDisabledAreas << "\n"
        << "  LogLevelStartup: " << LogLevelStartup << "\n"
        << "  unit test: " << UnitTestLibrary << "\n"
        << "  NoCapsForKit: " << NoCapsForKit << "\n"
        << "  NoSeccomp: " << NoSeccomp << "\n"
#if ENABLE_DEBUG
        << "  SingleKit: " << SingleKit << "\n"
#endif
        << "  ClientPortNumber: " << ClientPortNumber << "\n"
        << "  MasterLocation: " << MasterLocation
        << "\n";

    oss << "\nMalloc info [" << getpid() << "]: \n\t"
        << Util::replace(Util::getMallocInfo(), "\n", "\n\t") << '\n';
    oss << "\nEnd ForKit " << getpid() << " Dump State.\n";

    const std::string msg = oss.str();
    fprintf(stderr, "%s", msg.c_str());
    LOG_WRN(msg);
}

class ServerWSHandler;

// We have a single thread and a single connection so we won't bother with
// access synchronization
std::shared_ptr<ServerWSHandler> WSHandler;

class ServerWSHandler final : public WebSocketHandler
{
    std::string _socketName;

public:
    explicit ServerWSHandler(std::string socketName)
        : WebSocketHandler(/* isClient = */ true, /* isMasking */ false)
        , _socketName(std::move(socketName))
    {
    }

protected:
    void handleMessage(const std::vector<char>& data) override
    {
        std::string message(data.data(), data.size());

        if (!Util::isMobileApp() && UnitKit::get().filterKitMessage(this, message))
            return;

        StringVector tokens = StringVector::tokenize(message);

        LOG_DBG(_socketName << ": recv [" <<
                [&](auto& log)
                {
                    for (const auto& token : tokens)
                    {
                        log << tokens.getParam(token) << ' ';
                    }
                });

        // Note: Syntax or parsing errors here are unexpected and fatal.
        if (SigUtil::getTerminationFlag())
        {
            LOG_DBG("Termination flag set: skip message processing");
        }
        else if (tokens.size() == 2 && tokens.equals(0, "spawn"))
        {
            const int count = NumUtil::stoi(tokens[1]);
            if (count > 0)
            {
                LOG_INF("Setting to spawn " << count << " child" << (count == 1 ? "" : "ren")
                                            << " per request");
                ForkCounter = count;
            }
            else
            {
                LOG_WRN("Cannot spawn [" << tokens[1] << "] children as requested");
            }
        }
        else if (tokens.size() == 2 && tokens.equals(0, "addforkit"))
        {
            std::string ident = tokens[1];
            LOG_INF("Setting to spawn subForKit with ident [" << ident << "] per request.");
            SubForKitRequests.emplace_back(ident);
        }
        else if (tokens.size() == 2 && tokens.equals(0, "setloglevel"))
        {
            // Set environment variable so that new children will also set their log levels accordingly.
            setenv("COOL_LOGLEVEL", tokens[1].c_str(), 1);
            Log::setLevel(tokens[1]);
        }
        else if (tokens.size() == 3 && tokens.equals(0, "setconfig"))
        {
            // Currently only rlimit entries are supported.
            if (!Rlimit::handleSetrlimitCommand(tokens))
            {
                LOG_ERR("Unknown setconfig command: " << message);
            }
        }
        else if (tokens.size() == 2 && tokens.equals(0, "addfont"))
        {
            // Tell core to use that font file
            std::string fontFile = tokens[1];

            assert(loKitPtr);
            loKitPtr->pClass->setOption(loKitPtr, "addfont", Poco::URI(Poco::Path(fontFile)).toString().c_str());
        }
        else if (tokens.equals(0, "exit"))
        {
            LOG_INF("Setting TerminationFlag due to 'exit' command from parent.");
            SigUtil::setTerminationFlag();
        }
        else
        {
            LOG_ERR("Bad or unknown token [" << tokens[0] << ']');
        }
    }

    void onDisconnect() override
    {
        if constexpr (Util::isMobileApp())
            return;
        LOG_ERR("ForKit connection lost without exit arriving from wsd. Setting TerminationFlag");
        SigUtil::setTerminationFlag();
    }
};

namespace
{

#if HAVE_LIBCAP
bool haveCapability(cap_value_t capability)
{
    using ScopedCaps = std::unique_ptr<std::remove_pointer<cap_t>::type, int (*)(void*)>;
    ScopedCaps caps(cap_get_proc(), cap_free);
    if (!caps)
    {
        LOG_SFL("cap_get_proc() failed");
        return false;
    }

    char *cap_name = cap_to_name(capability);
    cap_flag_value_t value;

    if (cap_get_flag(caps.get(), capability, CAP_EFFECTIVE, &value) == -1)
    {
        if (cap_name)
        {
            LOG_SFL("cap_get_flag failed for " << cap_name);
            cap_free(cap_name);
        }
        else
        {
            LOG_SFL("cap_get_flag failed for capability " << capability);
        }
        return false;
    }

    if (value != CAP_SET)
    {
        if (cap_name)
        {
            LOG_ERR("Capability " << cap_name << " is not set for the coolforkit program.");
            cap_free(cap_name);
        }
        else
        {
            LOG_ERR("Capability " << capability << " is not set for the coolforkit program.");
        }
        return false;
    }

    if (cap_name)
    {
        LOG_INF("Have capability " << cap_name);
        cap_free(cap_name);
    }
    else
    {
        LOG_INF("Have capability " << capability);
    }

    return true;
}

bool haveCorrectCapabilities()
{
    bool result = true;

    // Do check them all, don't shortcut with &&
    if (!haveCapability(CAP_SYS_CHROOT))
        result = false;
    if (!haveCapability(CAP_FOWNER))
        result = false;
    if (!haveCapability(CAP_CHOWN))
        result = false;

    return result;
}
#else
bool haveCorrectCapabilities()
{
    // chroot() can only be called by root
    return getuid() == 0;
}
#endif // HAVE_LIBCAP

/// Check if some previously forked kids have died.
void cleanupChildren(const std::string& childRoot)
{
    if constexpr (Util::isKitInProcess())
        return;

    pid_t exitedChildPid;
    int status = 0;
    int segFaultCount = 0;
    int killedCount = 0;
    int oomKilledCount = 0;

    siginfo_t info;
    memset(&info, 0, sizeof(info)); // Make sure no stale fields remain

    // Reap quickly without doing slow cleanup so WSD can spawn more rapidly.
    while (waitid(P_ALL, -1, &info, WEXITED | WNOHANG) == 0)
    {
        if (info.si_pid == 0)
        {
            // WNOHANG special case
            break;
        }

        exitedChildPid = info.si_pid;
        status = info.si_status;
        if (const auto it = childJails.find(exitedChildPid); it != childJails.end())
        {
            if (info.si_code == CLD_KILLED || info.si_code == CLD_DUMPED)
            {
                if (status == SIGSEGV || status == SIGBUS ||
                    status == SIGABRT)
                {
                    ++segFaultCount;

                    std::string noteCrashFile(it->second + "/tmp/kit-crashed");
                    int noteCrashFD = open(noteCrashFile.c_str(), O_CREAT | O_TRUNC | O_WRONLY,
                                           S_IRUSR | S_IWUSR);
                    if (noteCrashFD < 0)
                        LOG_SYS("Couldn't create file: " << noteCrashFile);
                    else
                        close(noteCrashFD);
                }
                else if (status == SIGKILL)
                {
#if !defined(MACOS)
                    // TODO differentiate with docker
                    if (info.si_code == SI_KERNEL)
                    {
                        ++oomKilledCount;
                        LOG_WRN("Child " << exitedChildPid << " was killed by OOM, with status "
                                         << status);
                    }
                    else
#endif
                    {
                        ++killedCount;
                        LOG_WRN("Child " << exitedChildPid << " was killed, with status "
                                         << status);
                    }
                }
                else
                {
                    LOG_ERR("Child " << exitedChildPid << " has terminated, with signal " << status);
                }
            }
            else if (info.si_code == CLD_EXITED && status != 0)
            {
                LOG_ERR("Child " << exitedChildPid << " has exited, with status " << status);
            }

            LOG_INF("Child " << exitedChildPid << " has exited, will remove its jail [" << it->second << "].");
            cleanupJailPaths.emplace_back(it->second);
            childJails.erase(it);
            if (childJails.empty() && !SigUtil::getTerminationFlag())
            {
                // We ran out of kits and we aren't terminating.
                LOG_WRN("No live Kits exist, and we are not terminating yet.");
            }
        }
        else if (const auto subit = subForKitPids.find(exitedChildPid); subit != subForKitPids.end())
        {
            LOG_INF("SubForKit " << exitedChildPid << " [" << subit->second
                    << "] has exited with status " << status << ".");

            // remove subforkit settings dir now
            Poco::Path sharedPresets(childRoot, JailUtil::CHILDROOT_TMP_SHARED_PRESETS_PATH);
            std::string presetsPath = Poco::Path(sharedPresets, Uri::encode(subit->second)).toString();
            FileUtil::removeFile(presetsPath, true);

            subForKitPids.erase(subit);
        }
        else
        {
            LOG_ERR("Unknown child " << exitedChildPid << " has exited, with status: " << status);
        }
    }

    if (Log::traceEnabled() && cleanupJailPaths.size() > 0)
    {
        std::ostringstream oss;
        for (const auto& pair : childJails)
            oss << pair.first << ' ';

        LOG_TRC("cleanupChildren reaped " << cleanupJailPaths.size() << " children to have "
                                          << childJails.size() << " left: " << oss.str());
    }

    if (segFaultCount || killedCount || oomKilledCount)
    {
        if (WSHandler)
        {
            std::stringstream stream;
            stream << "segfaultcount=" << segFaultCount << ' ' << "killedcount=" << killedCount
                    << ' ' << "oomkilledcount=" << oomKilledCount << '\n';

            const int ret = WSHandler->sendTextMessage(stream.str());
            if (ret == -1)
            {
                LOG_WRN("Could not send 'segfaultcount' message through websocket");
            }
            else
            {
                LOG_WRN("Successfully sent 'segfaultcount' message " << stream.str());
            }
        }
    }

    // Now delete the jails.
    auto i = cleanupJailPaths.size();
    while (i > 0)
    {
        --i;
        const std::string path = cleanupJailPaths[i];

        // Don't delete jails where there was a crash until it's ~3 minutes old.
        const FileUtil::Stat noteStat(path + "/tmp/kit-crashed");
        if (noteStat.good())
        {
            const time_t modifiedTimeSec = noteStat.modifiedTimeMs() / 1000;
            if (time(nullptr) < modifiedTimeSec + 180)
                continue;
        }

        JailUtil::tryRemoveJail(path);
        const FileUtil::Stat st(path);
        if (st.good() && st.isDirectory())
            LOG_DBG("Could not remove jail path [" << path << "]. Will retry later.");
        else
            cleanupJailPaths.erase(cleanupJailPaths.begin() + i);
    }
}

void sleepForDebugger()
{
    Util::sleepFromEnvIfSet("Kit", "SLEEPKITFORDEBUGGER");
}

int forkKit(const std::function<void()>& childFunc, const std::string& childProcessName,
            const std::function<void(pid_t)>& parentFunc)
{
    pid_t pid = 0;

    /* We are about to fork, but not exec. After a fork the child has
       only one thread, but a copy of the watchdog object.

       Stop the watchdog thread before fork, let the child discard
       its copy of the watchdog that is now in a discardable state,
       and allow it to create a new one on next SocketPoll ctor */
    const bool hasWatchDog(SocketPoll::PollWatchdog);
    if (hasWatchDog)
        SocketPoll::PollWatchdog->joinThread();

    Log::preFork();

    pid = fork();
    if (!pid)
    {
        sleepForDebugger();

        // Child
        Log::postFork();

        // sort out thread local variables to get logging right from
        // as early as possible.
        ProcUtil::setThreadName(childProcessName);

        // Close the pipe from coolwsd
        close(0);

        // Close the ForKit main-loop's sockets
        if (ForKitPoll)
            ForKitPoll->closeAllSockets();
        // else very first kit process spawned

        SigUtil::setSigChildHandler(nullptr);

        // Throw away inherited watchdog, which will let a new one for this
        // child be created on demand
        SocketPoll::PollWatchdog.reset();

        UnitKit::get().postFork();

        childFunc();
    }
    else
    {
        if (hasWatchDog)
        {
            // restart parent watchdog if there was one
            SocketPoll::PollWatchdog->startThread();
        }

        // Parent
        parentFunc(pid);

        UnitKit::get().launchedKit(pid);
    }

    return pid;
}

int createCOKit(const std::string& childRoot, const std::string& sysTemplate,
                         const std::string& loTemplate, const std::string& configId,
                         bool useMountNamespaces, bool queryVersion = false)
{
    // Generate a jail ID to be used for in the jail path.
    std::string jailId = Util::rng::getFilename(16);

    // Update the dynamic files as necessary.
#if ENABLE_CHILDROOTS
    const bool sysTemplateIncomplete = !JailUtil::SysTemplate::updateDynamicFiles(sysTemplate);
#else
    const bool sysTemplateIncomplete = false;
#endif

    // Used to label the spare kit instances
    static size_t spareKitId = 0;
    ++spareKitId;
    LOG_DBG("Forking a coolkit process with jailId: " << jailId << " as spare coolkit #"
                                                      << spareKitId << '.');
    const auto startForkingTime = std::chrono::steady_clock::now();

    pid_t childPid = 0;
    if constexpr (Util::isKitInProcess())
    {
        std::thread([childRoot, jailId = std::move(jailId), configId, sysTemplate,
                     loTemplate, queryVersion
#if ENABLE_CHILDROOTS
                     , sysTemplateIncomplete
#endif
                    ] {
            sleepForDebugger();
            lokit_main(childRoot, jailId, configId, sysTemplate, loTemplate, true,
                       true, false, queryVersion, DisplayVersion,
                       sysTemplateIncomplete, spareKitId);
        })
            .detach();
    }
    else
    {
        auto childFunc = [childRoot, jailId, configId, sysTemplate,
                          loTemplate, useMountNamespaces,
                          queryVersion, sysTemplateIncomplete]()
        {
            lokit_main(childRoot, jailId, configId, sysTemplate, loTemplate,
                       NoCapsForKit, NoSeccomp, useMountNamespaces, queryVersion,
                       DisplayVersion, sysTemplateIncomplete, spareKitId);
        };

        auto parentFunc = [childRoot, jailId = std::move(jailId)](int pid)
        {
            // Parent
            if (pid < 0)
            {
                LOG_SYS("Fork failed for kit");
            }
            else
            {
                LOG_INF("Forked kit [" << pid << ']');
                childJails[pid] = childRoot + jailId;
            }
        };

        std::string processName = "kit_spare_" + Util::encodeId(spareKitId, 3);
        childPid = forkKit(childFunc, processName, parentFunc);
    }

    const auto duration = (std::chrono::steady_clock::now() - startForkingTime);
    const auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
    LOG_TRC("Forking child took " << durationMs);

    return childPid;
}

int createSubForKit(const std::string& subForKitIdent, const std::string& childRoot,
                    const std::string& sysTemplate, const std::string& loTemplate,
                    bool useMountNamespaces)
{
    static size_t subForKitId = 0;
    ++subForKitId;
    LOG_DBG("Forking a forkit process with subForKitId: " << subForKitIdent <<
            " as subForKit #" << subForKitId << ".");
    const auto startForkingTime = std::chrono::steady_clock::now();

    pid_t childPid = 0;

    auto childFunc = [childRoot, sysTemplate, loTemplate,
                      subForKitIdent, useMountNamespaces]()
    {
        // reset parent of this subforkit to its forkit parent, main loop
        // detects a parentPid != getppid() as a cue to exit
        parentPid = getppid();

        ForKitIdent = subForKitIdent;

        // reset this global counter for this new subForKit
        ForkCounter = 0;

        // Apply core configmgr xcu settings to this forkit for its coolkits to inherit
        {
            Poco::Path sharedPresets(childRoot, JailUtil::CHILDROOT_TMP_SHARED_PRESETS_PATH);
            Poco::Path presetsPath = Poco::Path(sharedPresets, Uri::encode(subForKitIdent)).toString();
            assert(loKitPtr);
            loKitPtr->pClass->setOption(loKitPtr, "addconfig", Poco::URI(presetsPath).toString().c_str());
        }

        LOG_INF("SubForKit process is ready. Parent: " << parentPid);

        // launch first coolkit child of this subForKit
        const pid_t forKitPid = createCOKit(childRoot, sysTemplate,
                                                     loTemplate, ForKitIdent,
                                                     useMountNamespaces);
        if (forKitPid < 0)
        {
            LOG_FTL("Failed to create a kit process.");
            Util::forcedExit(EX_SOFTWARE);
        }

        std::string pathAndQuery(FORKIT_URI);
        pathAndQuery.append("?configid=");
        pathAndQuery.append(ForKitIdent);

        ForKitPoll->createWakeups();

        if (!ForKitPoll->insertNewUnixSocket(MasterLocation, pathAndQuery, WSHandler))
        {
            LOG_SFL("Failed to connect to WSD. Will exit.");
            Util::forcedExit(EX_SOFTWARE);
        }
    };

    auto parentFunc = [subForKitIdent](int pid)
    {
        // Parent
        if (pid < 0)
        {
            LOG_SYS("Fork failed for subForKit");
        }
        else
        {
            LOG_INF("Forked subForKit [" << pid << ']');
            subForKitPids[pid] = subForKitIdent;
        }
    };

    std::string processName = "subforkit_" + Util::encodeId(subForKitId, 3);
    childPid = forkKit(childFunc, processName, parentFunc);

    const auto duration = (std::chrono::steady_clock::now() - startForkingTime);
    const auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
    LOG_TRC("Forking subForKit took " << durationMs);

    return childPid;
}

void createSubForKits(const std::string& childRoot, const std::string& sysTemplate,
                      const std::string& loTemplate, bool useMountNamespaces)
{
    std::vector<std::string> subForKitRequests = std::move(SubForKitRequests);
    for (const auto& subForKitIdent : subForKitRequests)
    {
        if (createSubForKit(subForKitIdent, childRoot, sysTemplate, loTemplate,
                            useMountNamespaces) < 0)
        {
            LOG_ERR("Failed to create a subForKit process for ident: " << subForKitIdent);
        }
    }
}

} // namespace

void forkCOKit(const std::string& childRoot,
                        const std::string& sysTemplate,
                        const std::string& loTemplate,
                        bool useMountNamespaces)
{
    // Cleanup first, to reduce disk load.
    cleanupChildren(childRoot);

    if (ForkCounter > 0)
    {
        // Create as many as requested.
        const size_t count = ForkCounter;
        LOG_INF("Spawning " << count << " new child" << (count == 1 ? "." : "ren."));
        const size_t retry = count * 2;
        for (size_t i = 0; ForkCounter > 0 && i < retry; ++i)
        {
            if (ForkCounter-- <= 0 || createCOKit(childRoot, sysTemplate, loTemplate,
                                                           ForKitIdent, useMountNamespaces) < 0)
            {
                LOG_ERR("Failed to create a kit process.");
                ++ForkCounter;
            }
        }
    }
}

static void printArgumentHelp()
{
    std::cout << "Usage: coolforkit [OPTION]..." << std::endl;
    std::cout << "  Single-threaded process that spawns lok instances" << std::endl;
    std::cout << "  Note: Running this standalone is not possible. It is spawned by coolwsd" << std::endl;
    std::cout << "        and is controlled via a pipe." << std::endl;
    std::cout << "" << std::endl;
}

extern "C" {
    static void wakeupPoll(int /*pid*/)
    {
        if (ForKitPoll)
            ForKitPoll->wakeup();
    }
}

// Define in order to dump a trace whenever a thread is created.
// #define THREAD_TRACKER

#ifdef THREAD_TRACKER

#include <execinfo.h>

typedef int (*PThreadCreateFn) (pthread_t * __newthread, const pthread_attr_t * __attr,
                                void *(*__start_routine) (void *), void * __arg);
PThreadCreateFn OrigPThreadCreate;

int pthread_create (pthread_t *a, const pthread_attr_t *b, void *(*c) (void *), void *d)
{
    void *buffer[128];

    std::cerr << "\npthread_create from:\n";
    auto nptrs = backtrace(buffer, 128);
    backtrace_symbols_fd(buffer, nptrs, STDERR_FILENO);
    std::cerr << "\n";

    return OrigPThreadCreate(a, b, c, d);
}

__attribute__((constructor))
void init(void) {
    OrigPThreadCreate = reinterpret_cast<PThreadCreateFn>(dlsym(RTLD_NEXT, "pthread_create"));
}

#endif // THREAD_TRACKER

int forkit_main(int argc, char** argv)
{
    /*WARNING: PRIVILEGED CODE CHECKING START */

    /*WARNING*/ // early check for avoiding the security check for username 'cool'
    /*WARNING*/ // (deliberately only this, not moving the entire parameter parsing here)
    /*WARNING*/ bool checkCoolUser = true;
    /*WARNING*/ std::string disableCoolUserChecking("--disable-cool-user-checking");
    /*WARNING*/ for (int i = 1; checkCoolUser && (i < argc); ++i)
    /*WARNING*/ {
    /*WARNING*/     if (disableCoolUserChecking == argv[i])
    /*WARNING*/         checkCoolUser = false;
    /*WARNING*/ }

    /*WARNING*/ if (!hasCorrectUID(/* appName = */ "coolforkit"))
    /*WARNING*/ {
    /*WARNING*/     // don't allow if any capability is set (unless root; who runs this
    /*WARNING*/     // as root or runs this in a container and provides --disable-cool-user-checking knows what they
    /*WARNING*/     // are doing)
    /*WARNING*/     if (hasUID("root"))
    /*WARNING*/     {
    /*WARNING*/        // This is fine, the 'root' can do anything anyway
    /*WARNING*/     }
    /*WARNING*/     else if (isInContainer())
    /*WARNING*/     {
    /*WARNING*/         // This is fine, we are confined in the container anyway
    /*WARNING*/     }
    /*WARNING*/     else if (hasAnyCapability())
    /*WARNING*/     {
    /*WARNING*/         if (!checkCoolUser)
    /*WARNING*/             LOG_FTL("Security: --disable-cool-user-checking failed, coolforkit has some capabilities set.");

    /*WARNING*/         LOG_FTL("Aborting.");
    /*WARNING*/         return EX_SOFTWARE;
    /*WARNING*/     }

    /*WARNING*/     // even without the capabilities, don't run unless the user really knows
    /*WARNING*/     // what they are doing, and provided a --disable-cool-user-checking
    /*WARNING*/     if (checkCoolUser)
    /*WARNING*/     {
    /*WARNING*/         LOG_FTL("Aborting.");
    /*WARNING*/         return EX_SOFTWARE;
    /*WARNING*/     }

    /*WARNING*/     LOG_ERR("Security: Check for the 'cool' username overridden on the command line.");
    /*WARNING*/ }

    /*WARNING: PRIVILEGED CODE CHECKING END */

    // Continue in privileged mode, but only if:
    // * the user is 'cool' (privileged user)
    // * the user is 'root', and --disable-cool-user-checking was provided
    // Alternatively allow running in non-privileged mode (with --nocaps), if:
    // * the user is a non-privileged user, the binary is not privileged
    //   either (no caps set), and --disable-cool-user-checking was provided

    Util::sleepFromEnvIfSet("Forkit", "SLEEPFORDEBUGGER");

    if constexpr (!Util::isKitInProcess())
    {
        // Already set by COOLWSD.cpp in kit in process
        SigUtil::setFatalSignals("forkit startup of " + Util::getCoolVersion() + ' ' +
                                 Util::getCoolVersionHash());
    }
    else
    {
        // No capabilities by default for kit in process
        NoCapsForKit = true;
        NoSeccomp = true;
#if ENABLE_DEBUG
        SingleKit = true;
#endif
    }

    if (simd::init())
        simd_deltaInit();

    if constexpr (!Util::isKitInProcess())
        Util::setApplicationPath(Poco::Path(argv[0]).parent().toString());

    // Initialization
    const bool logToFile = std::getenv("COOL_LOGFILE");
    const char* logFilename = std::getenv("COOL_LOGFILENAME");
    const char* logLevel = std::getenv("COOL_LOGLEVEL");
    const char* logDisabledAreas = std::getenv("COOL_LOGDISABLED_AREAS");
    const char* logLevelStartup = std::getenv("COOL_LOGLEVEL_STARTUP");
    const char* logColor = std::getenv("COOL_LOGCOLOR");
    std::map<std::string, std::string> logProperties;
    if (logToFile && logFilename)
    {
        logProperties["path"] = std::string(logFilename);
    }
    const bool logToFileUICmd = std::getenv("COOL_LOGFILE_UICMD");
    const char* logFilenameUICmd = std::getenv("COOL_LOGFILENAME_UICMD");
    std::map<std::string, std::string> logPropertiesUICmd;
    if (logToFileUICmd && logFilenameUICmd)
    {
        logPropertiesUICmd["path"] = std::string(logFilenameUICmd);
    }

    LogLevelStartup = logLevelStartup ? logLevelStartup : "trace";
    Log::initialize("frk", LogLevelStartup, logColor != nullptr, logToFile, logProperties, logToFileUICmd, logPropertiesUICmd);

    if (logToFileUICmd)
    {
        const bool mergeUiCmd = std::getenv("COOL_LOG_UICMD_MERGE");
        const bool logTimeEndOfMergedUiCmd = std::getenv("COOL_LOG_UICMD_END_TIME");
        Log::setUILogMergeInfo(mergeUiCmd, logTimeEndOfMergedUiCmd);
    }

    LogLevel = logLevel ? logLevel : "trace";
    if (LogLevel != LogLevelStartup)
    {
        LOG_INF("Setting log-level to [" << LogLevelStartup << " and delaying "
                "setting to configured [" << LogLevel << "] until after Forkit initialization.");
    }
    LogDisabledAreas = logDisabledAreas ? logDisabledAreas : "";

    bool useMountNamespaces = false;
    std::string childRoot;
    std::string sysTemplate;
    std::string loTemplate;

    for (int i = 0; i < argc; ++i)
    {
        char *cmd = argv[i];
        char *eq;
        if (std::strstr(cmd, "--systemplate=") == cmd)
        {
            eq = std::strchr(cmd, '=');
            sysTemplate = std::string(eq+1);
        }
        else if (std::strstr(cmd, "--lotemplate=") == cmd)
        {
            eq = std::strchr(cmd, '=');
            loTemplate = std::string(eq+1);
        }
        else if (std::strstr(cmd, "--childroot=") == cmd)
        {
            eq = std::strchr(cmd, '=');
            childRoot = std::string(eq+1);
        }
        else if (std::strstr(cmd, "--clientport=") == cmd)
        {
            eq = std::strchr(cmd, '=');
            ClientPortNumber = std::stoll(std::string(eq+1));
        }
        else if (std::strstr(cmd, "--masterport=") == cmd)
        {
            eq = std::strchr(cmd, '=');
            MasterLocation = UnxSocketPath(std::string(eq+1));
        }
        else if (std::strstr(cmd, "--version") == cmd)
        {
            std::string version, hash;
            Util::getVersionInfo(version, hash);
            std::cout << "coolforkit version details: " << version << " - " << hash << std::endl;
            DisplayVersion = true;
        }
        else if (std::strstr(cmd, "--rlimits") == cmd)
        {
            eq = std::strchr(cmd, '=');
            StringVector tokens = StringVector::tokenize(std::string(eq+1), ';');
            for (const auto& cmdLimit : tokens)
            {
                const std::pair<std::string, std::string> pair = Util::split(tokens.getParam(cmdLimit), ':');
                StringVector tokensLimit;
                tokensLimit.push_back("setconfig");
                tokensLimit.push_back(pair.first);
                tokensLimit.push_back(pair.second);
                if (!Rlimit::handleSetrlimitCommand(tokensLimit))
                {
                    LOG_ERR("Unknown rlimits command: " << tokens.getParam(cmdLimit));
                }
            }
        }
        else if (std::strstr(cmd, "--unattended") == cmd)
        {
            SigUtil::setUnattended();
        }
#if ENABLE_DEBUG
        // this process has various privileges - don't run arbitrary code.
        else if (std::strstr(cmd, "--unitlib=") == cmd)
        {
            eq = std::strchr(cmd, '=');
            UnitTestLibrary = std::string(eq+1);
        }
        else if (std::strstr(cmd, "--singlekit") == cmd)
        {
            SingleKit = true;
        }
#endif
        // we are running in a lower-privilege mode - with no chroot
        else if (std::strstr(cmd, "--nocaps") == cmd)
        {
            LOG_ERR("Security: Running without the capability to enter a chroot jail is ill advised.");
            NoCapsForKit = true;
        }

        // we are running without seccomp protection
        else if (std::strstr(cmd, "--noseccomp") == cmd)
        {
            LOG_ERR("Security: Running without the ability to filter system calls is ill advised.");
            NoSeccomp = true;
        }

        else if (std::strstr(cmd, "--namespace") == cmd)
            useMountNamespaces = true;

        else if (std::strstr(cmd, "--ui") == cmd)
        {
            eq = std::strchr(cmd, '=');
            UserInterface = std::string(eq+1);
            if (UserInterface != "classic" && UserInterface != "notebookbar")
                UserInterface = "notebookbar";
        }

        else if (std::strstr(cmd, "--libversions") == cmd)
        {
            std::cout << Util::getVersionJSON(false, "") << std::endl;
            std::cerr << "Note: Library versions are compile-time versions, so they are accurate "
                         "when these libraries are statically linked (as in production builds)."
                      << std::endl;
            std::exit(EX_OK);
        }
    }

    if (sysTemplate.empty() || loTemplate.empty() || childRoot.empty())
    {
        printArgumentHelp();
        return EX_USAGE;
    }

    LOG_DBG("About to init Kit UnitBase with test [" << UnitTestLibrary << ']');
    if (!Util::isKitInProcess() && !UnitBase::init(UnitBase::UnitType::Kit, UnitTestLibrary))
    {
        LOG_FTL("Failed to load kit unit test library");
        return EX_USAGE;
    }

    setupKitEnvironment(UserInterface);

    if (!std::getenv("LD_BIND_NOW")) // must be set by parent.
        LOG_INF("Note: LD_BIND_NOW is not set.");

    if (!NoCapsForKit && !useMountNamespaces && !haveCorrectCapabilities())
    {
        LOG_FTL("Capabilities are not set for the coolforkit program.");
        LOG_FTL("Please make sure that the current partition was *not* mounted with the 'nosuid' option.");
        LOG_FTL("If you are on SLES11, please set 'file_caps=1' as kernel boot option.");
        return EX_SOFTWARE;
    }

    // Initialize LoKit
    if (!globalPreinit(loTemplate))
    {
        LOG_FTL("Failed to preinit lokit.");
        Util::forcedExit(EX_SOFTWARE);
    }

    if (Util::ThreadCounter().count() != 1)
        LOG_ERR("forkit has more than a single thread after pre-init" << Util::ThreadCounter().count());

#if ENABLE_CHILDROOTS
    // Link the network and system files in sysTemplate, if possible.
    JailUtil::SysTemplate::setupDynamicFiles(sysTemplate);

    // Make dev/[u]random point to the writable devices in tmp/dev/.
    JailUtil::SysTemplate::setupRandomDeviceLinks(sysTemplate);
#endif

    if constexpr (!Util::isKitInProcess())
    {
        // Parse the configuration.
        char* const conf = std::getenv("COOL_CONFIG");
        ConfigUtil::initialize(std::string(conf ? conf : std::string()));
        EnableExperimental = ConfigUtil::getBool("experimental_features", false);
    }

    ProcUtil::setThreadName("forkit");

    LOG_INF("Preinit stage OK.");

    // We must have at least one child, more are created dynamically.
    // Ask this first child to send version information to master process and trace startup.
    ::setenv("COOL_TRACE_STARTUP", "1", 1);
    const pid_t forKitPid = createCOKit(childRoot, sysTemplate, loTemplate,
                                                 ForKitIdent, useMountNamespaces, true);
    if (forKitPid < 0)
    {
        LOG_FTL("Failed to create a kit process.");
        Util::forcedExit(EX_SOFTWARE);
    }

    // No need to trace subsequent children.
    ::unsetenv("COOL_TRACE_STARTUP");
    if (LogLevel != LogLevelStartup)
    {
        LOG_INF("Forkit initialization complete: setting log-level to [" << LogLevel << "] as configured.");
        Log::setLevel(LogLevel);
    }
    Log::setDisabledAreas(LogDisabledAreas);

    // The SocketPoll ctor which may, depending on COOL_WATCHDOG env variable,
    // want to override the SIG2 handler so set user signal handlers before
    // that otherwise that choice is overwritten
    SigUtil::setUserSignals();

    ForKitPoll.reset(new SocketPoll (ProcUtil::getThreadName()));
    ForKitPoll->runOnClientThread(); // We will do the polling on this thread.

    // Reap zombies when we get the signal
    SigUtil::setSigChildHandler(wakeupPoll);

    WSHandler = std::make_shared<ServerWSHandler>("forkit_ws");

    if (!Util::isMobileApp() &&
        !ForKitPoll->insertNewUnixSocket(MasterLocation, FORKIT_URI, WSHandler))
    {
        LOG_SFL("Failed to connect to WSD. Will exit.");
        Util::forcedExit(EX_SOFTWARE);
    }

    parentPid = getppid();
    LOG_INF("ForKit process is ready. Parent: " << parentPid);

    while (!SigUtil::getShutdownRequestFlag())
    {
        UnitKit::get().invokeForKitTest();

        ForKitPoll->poll(std::chrono::seconds(POLL_FORKIT_TIMEOUT_SECS));

        SigUtil::checkDumpGlobalState(dump_forkit_state);

        // When our parent exits, we are assigned a new parent (typically init).
        if (getppid() != parentPid)
        {
            LOG_SFL("Parent process has died. Will exit now.");
            break;
        }

#if ENABLE_DEBUG
        if (!SingleKit)
#endif
            if (!Util::isKitInProcess() && !SigUtil::getTerminationFlag())
            {
                // new kits are launched primarily after a 'spawn' message
                forkCOKit(childRoot, sysTemplate, loTemplate, useMountNamespaces);
                // new sub forkits are launched after an 'addforkit' message
                createSubForKits(childRoot, sysTemplate, loTemplate, useMountNamespaces);
            }
    }

    const int returnValue = UnitBase::uninit();

    LOG_INF("ForKit process finished.");
    Util::forcedExit(returnValue);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
