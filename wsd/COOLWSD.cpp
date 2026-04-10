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
 * Implementation of main server application logic.
 * Classes: COOLWSD
 */

#include <config.h>
#include <config_version.h>

#include "COOLWSD.hpp"

/* Default host used in the start test URI */
#define COOLWSD_TEST_HOST "localhost"

/* Default cool UI used in the admin console URI */
#define COOLWSD_TEST_ADMIN_CONSOLE "/browser/dist/admin/admin.html"

/* Default cool UI used in for monitoring URI */
#define COOLWSD_TEST_METRICS "/cool/getMetrics"

/* Default cool UI used in the start test URI */
#define COOLWSD_TEST_COOL_UI "/browser/" COOLWSD_VERSION_HASH "/debug.html"

/* Default ciphers used, when not specified otherwise */
#define DEFAULT_CIPHER_SET "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH"

// This is the main source for the coolwsd program. COOL uses several coolwsd processes: one main
// parent process that listens on the TCP port and accepts connections from COOL clients, and a
// number of child processes, each which handles a viewing (editing) session for one document.

#include <common/Anonymizer.hpp>
#include <common/Clipboard.hpp>
#include <common/Common.hpp>
#if ENABLE_FEATURE_LOCK
#include <common/CommandControl.hpp>
#endif
#include <common/ConfigUtil.hpp>
#include <common/Crypto.hpp>
#include <common/FileUtil.hpp>
#include <common/HexUtil.hpp>
#include <common/JailUtil.hpp>
#include <common/JsonUtil.hpp>
#include <common/Log.hpp>
#include <common/MobileApp.hpp>
#include <common/NumUtil.hpp>
#include <common/Protocol.hpp>
#include <common/RegexUtil.hpp>
#include <common/Session.hpp>
#include <common/SigUtil.hpp>
#include <common/Unit.hpp>
#include <common/Util.hpp>
#include <net/AsyncDNS.hpp>
#include <net/DelaySocket.hpp>
#include <net/ServerSocket.hpp>
#include <wsd/COOLWSDServer.hpp>
#include <wsd/ClientRequestDispatcher.hpp>
#include <wsd/DocumentBroker.hpp>
#include <wsd/PlatformDesktop.hpp>
#include <wsd/PlatformMobile.hpp>
#include <wsd/PlatformUnix.hpp>
#include <wsd/Process.hpp>
#include <wsd/TraceFile.hpp>
#include <wsd/wopi/StorageConnectionManager.hpp>

#include <Poco/DirectoryIterator.h>
#include <Poco/Exception.h>
#include <Poco/File.h>
#include <Poco/Path.h>
#include <Poco/URI.h>
#include <Poco/Util/AbstractConfiguration.h>
#include <Poco/Util/HelpFormatter.h>
#include <Poco/Util/MapConfiguration.h>
#include <Poco/Util/Option.h>
#include <Poco/Util/OptionException.h>
#include <Poco/Util/OptionSet.h>
#include <Poco/Util/ServerApplication.h>
#include <Poco/Util/XMLConfiguration.h>

#include <cassert>
#include <chrono>
#include <clocale>
#include <condition_variable>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>

#include <sys/types.h>

#ifndef _WIN32
#include <sys/resource.h>
#include <sys/wait.h>
#include <sysexits.h>
#include <unistd.h>
#endif

using Poco::Util::LayeredConfiguration;
using Poco::Util::Option;

/// Port for external clients to connect to
int ClientPortNumber = 0;

#if !MOBILEAPP
/// UDS address for kits to connect to.
UnxSocketPath MasterLocation;

std::string COOLWSD::BuyProductUrl;
std::string COOLWSD::LatestVersion;
std::mutex COOLWSD::FetchUpdateMutex;
std::mutex COOLWSD::RemoteConfigMutex;
#endif

/// The DocBrokers container; used from elsewhere as well.
std::map<std::string, std::shared_ptr<DocumentBroker>> DocBrokers;
std::mutex DocBrokersMutex; ///< Protects DocBrokers.

namespace
{

/// Protocols to listen on
Socket::Type ClientPortProto = Socket::Type::All;

/// INET address to listen on
ServerSocket::Type ClientListenAddr = ServerSocket::Type::Public;

// Tracks the set of prisoners / children waiting to be used.
std::mutex NewChildrenMutex;
std::condition_variable NewChildrenCV;
std::vector<std::shared_ptr<ChildProcess>> NewChildren;

std::atomic<int> TotalOutstandingForks(0);
std::map<std::string, int> OutstandingForks;
std::map<std::string, std::chrono::steady_clock::time_point> LastForkRequestTimes;
using SubForKitMap = std::map<std::string, std::shared_ptr<ForKitProcess>>;
SubForKitMap SubForKitProcs;
std::map<std::string, std::chrono::steady_clock::time_point> LastSubForKitBrokerExitTimes;
Poco::AutoPtr<Poco::Util::XMLConfiguration> KitXmlConfig;
std::string LoggableConfigEntries;

#if MOBILEAPP

// Or can this be retrieved in some other way?
int PrisonerServerSocketFD;

#else // MOBILEAPP

/// Funky latency simulation basic delay (ms)
std::size_t SimulatedLatencyMs = 0;
std::shared_ptr<http::Session> FetchHttpSession;

#if ENABLE_DEBUG
std::chrono::milliseconds careerSpanMs(std::chrono::milliseconds::zero());
#endif

#endif // !MOBILEAPP

/// The timeout for a child to spawn, initially high, then reset to the default.
std::atomic<std::chrono::milliseconds> ChildSpawnTimeoutMs =
    std::chrono::milliseconds(CHILD_SPAWN_TIMEOUT_MS);

} // namespace

std::atomic<unsigned> COOLWSD::NumConnections;
std::unordered_set<std::string> COOLWSD::EditFileExtensions;
std::string COOLWSD::ViewModeFileExtensions;

extern "C"
{
    void dump_state(void); /* easy for gdb */
    void forwardSigUsr2();
}

void COOLWSD::appendAllowedHostsFrom(const LayeredConfiguration& conf, const std::string& root, std::vector<std::string>& allowed)
{
    for (size_t i = 0; ; ++i)
    {
        const std::string path = root + ".host[" + std::to_string(i) + ']';
        if (!conf.has(path))
        {
            break;
        }
        std::string host = ConfigUtil::getConfigValue<std::string>(conf, path, "");
        if (!host.empty())
        {
            LOG_INF_S("Adding trusted LOK_ALLOW host: [" << host << ']');
            allowed.push_back(std::move(host));
        }
    }
}

namespace {
std::string removeProtocolAndPort(const std::string& host)
{
    std::string result;

    // protocol
    size_t pos = host.find("//");
    if (pos != std::string::npos)
        result = host.substr(pos + 2);
    else
        result = host;

    // port
    pos = result.find(':');
    if (pos != std::string::npos)
    {
        if (pos == 0)
            return std::string();

        result = result.substr(0, pos);
    }

    return result;
}
}

void COOLWSD::appendAllowedAliasGroups(const LayeredConfiguration& conf, std::vector<std::string>& allowed)
{
    for (size_t i = 0;; i++)
    {
        const std::string path = "storage.wopi.alias_groups.group[" + std::to_string(i) + ']';
        if (!conf.has(path + ".host"))
        {
            break;
        }

        std::string host = conf.getString(path + ".host", "");
        bool allow = conf.getBool(path + ".host[@allow]", false);
        if (!allow)
        {
            break;
        }

        host = removeProtocolAndPort(host);

        if (!host.empty())
        {
            LOG_INF_S("Adding trusted LOK_ALLOW host: [" << host << ']');
            allowed.push_back(std::move(host));
        }

        for (size_t j = 0;; j++)
        {
            const std::string aliasPath = path + ".alias[" + std::to_string(j) + ']';
            if (!conf.has(aliasPath))
            {
                break;
            }

            std::string alias = ConfigUtil::getConfigValue<std::string>(conf, aliasPath, "");

            alias = removeProtocolAndPort(alias);
            if (!alias.empty())
            {
                LOG_INF_S("Adding trusted LOK_ALLOW alias: [" << alias << ']');
                allowed.push_back(std::move(alias));
            }
        }
    }
}

/// Internal implementation to alert all clients
/// connected to any document.
void COOLWSD::alertAllUsersInternal(const std::string& msg)
{
    if constexpr (Util::isMobileApp())
        return;
    std::lock_guard<std::mutex> docBrokersLock(DocBrokersMutex);

    LOG_INF("Alerting all users: [" << msg << ']');
    SigUtil::addActivity("alert all users: " + msg);

    if (UnitWSD::get().filterAlertAllusers(msg))
        return;

    for (const auto& brokerIt : DocBrokers)
    {
        std::shared_ptr<DocumentBroker> docBroker = brokerIt.second;
        docBroker->addCallback([msg, docBroker](){ docBroker->alertAllUsers(msg); });
    }
}

#if !MOBILEAPP
void COOLWSD::syncUsersBrowserSettings(const std::string& userId, const pid_t childPid, const std::string& json)
{
    if constexpr (Util::isMobileApp())
        return;
    std::lock_guard<std::mutex> docBrokersLock(DocBrokersMutex);

    LOG_INF("Syncing browsersettings for all the users");

    for (const auto& brokerIt : DocBrokers)
    {
        std::shared_ptr<DocumentBroker> docBroker = brokerIt.second;
        if (docBroker->getPid() == childPid)
            continue;
        docBroker->addCallback([userId, json, docBroker]()
                               { docBroker->syncBrowserSettings(userId, json); });
    }
}
#endif

void COOLWSD::alertUserInternal(const std::string& dockey, const std::string& msg)
{
    if constexpr (Util::isMobileApp())
        return;
    std::lock_guard<std::mutex> docBrokersLock(DocBrokersMutex);

    LOG_INF("Alerting document users with dockey: [" << dockey << ']' << " msg: [" << msg << ']');

    for (const auto& brokerIt : DocBrokers)
    {
        std::shared_ptr<DocumentBroker> docBroker = brokerIt.second;
        if (docBroker->getDocKey() == dockey)
            docBroker->addCallback([msg, docBroker](){ docBroker->alertAllUsers(msg); });
    }
}

void COOLWSD::writeTraceEventRecording(const char *data, std::size_t nbytes)
{
    static std::mutex traceEventFileMutex;

    std::unique_lock<std::mutex> lock(traceEventFileMutex);

    fwrite(data, nbytes, 1, COOLWSD::TraceEventFile);
}

void COOLWSD::writeTraceEventRecording(const std::string &recording)
{
    writeTraceEventRecording(recording.data(), recording.length());
}

#if !MOBILEAPP
static ssize_t getInteractiveDocBrokerCount()
{
    std::lock_guard<std::mutex> docBrokersLock(DocBrokersMutex);
    return DocBrokers.size() - ConvertToBroker::getInstanceCount();
}
#endif

void COOLWSD::checkSessionLimitsAndWarnClients()
{
#if !MOBILEAPP
    if constexpr (ConfigUtil::isSupportKeyEnabled())
        return;

    if (COOLWSD::MaxDocuments >= 10000)
        return;

    if (getInteractiveDocBrokerCount() > static_cast<ssize_t>(COOLWSD::MaxDocuments) ||
        COOLWSD::NumConnections >= COOLWSD::MaxConnections)
    {
        std::ostringstream oss;
        oss << "info: cmd=socket kind=limitreached params=" << COOLWSD::MaxDocuments << ","
            << COOLWSD::MaxConnections;
        const std::string info = oss.str();
        LOG_INF("Sending client 'limitreached' message: " << info);

        try
        {
            Util::alertAllUsers(info);
        }
        catch (const std::exception& ex)
        {
            LOG_ERR("Error while shutting down socket on reaching limit: " << ex.what());
        }
    }
#endif
}

void COOLWSD::checkDiskSpaceAndWarnClients(const bool cacheLastCheck)
{
#if !MOBILEAPP
    try
    {
        const std::string fs = FileUtil::checkDiskSpaceOnRegisteredFileSystems(cacheLastCheck);
        if (!fs.empty())
        {
            LOG_WRN("Filesystem [" << fs << "] is dangerously low on disk space");
            COOLWSD::alertAllUsersInternal("error: cmd=internal kind=diskfull");
        }
    }
    catch (const std::exception& exc)
    {
        LOG_ERR("Exception while checking disk-space and warning clients: " << exc.what());
    }
#else
    (void) cacheLastCheck;
#endif
}

namespace {

SubForKitMap::iterator dropSubForKit(SubForKitMap::iterator it)
{
    // copy as it will be used after erase()
    std::string configId = it->first;

    LastSubForKitBrokerExitTimes.erase(configId);
    OutstandingForks.erase(configId);
    it = SubForKitProcs.erase(it);
    UNITWSD_CALL(killSubForKit(configId));

    return it;
}

}

/// Remove dead and idle DocBrokers.
/// The client of idle document should've greyed-out long ago.
void COOLWSD::cleanupDocBrokers()
{
    Util::assertIsLocked(DocBrokersMutex);

    const size_t count = DocBrokers.size();

    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();

    std::set<std::string> activeConfigs;

    for (auto it = DocBrokers.begin(); it != DocBrokers.end(); )
    {
        std::shared_ptr<DocumentBroker> docBroker = it->second;

        // Time out DocBrokers that never loaded
        docBroker->timeoutNotLoaded(now);

        // Remove only when not alive.
        if (!docBroker->isAlive())
        {
            LastSubForKitBrokerExitTimes[docBroker->getConfigId()] = now;
            LOG_INF("Removing DocumentBroker for docKey ["
                    << it->first << "], " << docBroker.use_count() << " references");
            docBroker->dispose();
            it = DocBrokers.erase(it);
            continue;
        }

        activeConfigs.insert(docBroker->getConfigId());
        ++it;
    }

    if (count != DocBrokers.size())
    {
        LOG_TRC("Have " << DocBrokers.size() << " DocBrokers after cleanup" <<
                [&](auto& log)
                {
                    int i = 1;
                    for (const auto& pair : DocBrokers)
                    {
                        log << "\nDocumentBroker #" << i++ << " [" << pair.first << ']';
                    }
                });

        CONFIG_STATIC const std::chrono::seconds IdleServerSettingsTimeoutSecs =
            ConfigUtil::getConfigValue<std::chrono::seconds>("serverside_config.idle_timeout_secs",
                                                             3600);

        // consider shutting down unused subforkits
        // Always drop those older than IdleServerSettingsTimeoutSecs, and cap
        // the remainder to a reasonable number of recently-used entries.
        CONFIG_STATIC const size_t MaxRecentlyUsedSubForKits = []{
            size_t value = ConfigUtil::getConfigValue<size_t>("serverside_config.max_idle_subforkits", 5);
            if (value < 1)
            {
                LOG_WRN("max_idle_subforkits is 0, clamping to 1.");
                value = 1;
            }
            return value;
        }();

        // idle candidates: pair of (idle duration, configId)
        std::vector<std::pair<std::chrono::steady_clock::duration, std::string>> idleCandidates;

        for (const auto& [configId, subForKitProc] : SubForKitProcs)
        {

            if (configId.empty()) {
                // ignore primordial forkit
            } else if (activeConfigs.contains(configId)) {
                LOG_DBG("subforkit " << configId << " has active document, keep it");
            } else if (OutstandingForks[configId] > 0) {
                LOG_DBG("subforkit " << configId << " has a pending fork underway, keep it");
            } else {
                auto idleDuration = now - LastSubForKitBrokerExitTimes[configId];
                idleCandidates.emplace_back(idleDuration, configId);
            }
        }

        // sort shortest-idle first so we keep the most recently used
        std::sort(idleCandidates.begin(), idleCandidates.end());

        size_t recentlyUsedKept = 0;
        for (const auto& [idleDuration, configId] : idleCandidates)
        {
            if (idleDuration >= IdleServerSettingsTimeoutSecs)
            {
                LOG_DBG("subforkit " << configId << " is unused, dropping it");
                auto it = SubForKitProcs.find(configId);
                assert(it != SubForKitProcs.end());
                dropSubForKit(it);
            }
            else if (recentlyUsedKept >= MaxRecentlyUsedSubForKits)
            {
                LOG_DBG("subforkit " << configId << " recently used but excess idle subforkit, dropping it");
                auto it = SubForKitProcs.find(configId);
                assert(it != SubForKitProcs.end());
                dropSubForKit(it);
            }
            else
            {
                LOG_DBG("subforkit " << configId << " recently used, keep it");
                ++recentlyUsedKept;
            }
        }

#if !MOBILEAPP && ENABLE_DEBUG
        if (COOLWSD::SingleKit && DocBrokers.empty())
        {
            LOG_DBG("Setting ShutdownRequestFlag: No more docs left in single-kit mode.");
            SigUtil::requestShutdown();
        }
#endif
    }
}

#if !MOBILEAPP

/// Forks as many children as requested.
static void forkChildren(const std::string& configId, const int number)
{
    if (Util::isKitInProcess())
        return;

    LOG_TRC("Request forkit to spawn " << number << " new child(ren)");
    Util::assertIsLocked(NewChildrenMutex);

    if (number > 0)
    {
        COOLWSD::checkDiskSpaceAndWarnClients(false);

        const std::string message = "spawn " + std::to_string(number) + '\n';
        LOG_DBG("MasterToForKit: " << message.substr(0, message.length() - 1));

        if (COOLWSD::sendMessageToForKit(message, configId))
        {
            TotalOutstandingForks += number;
            OutstandingForks[configId] += number;
            LastForkRequestTimes[configId] = std::chrono::steady_clock::now();
        }
    }
}

static bool queueMessageToForKit(const std::string& message);

bool COOLWSD::ensureSubForKit(const std::string& configId)
{
    if (Util::isKitInProcess())
        return false;

    LOG_TRC("Request forkit to spawn subForKit " << configId);

    auto it = SubForKitProcs.find(configId);
    if (it != SubForKitProcs.end())
    {
        LOG_TRC("subForKit " << configId << " already running");
        return false;
    }

    COOLWSD::checkDiskSpaceAndWarnClients(false);

    const std::string answerMessage = "addforkit " + configId + '\n';
    LOG_DBG("MasterToForKit: " << answerMessage.substr(0, answerMessage.length() - 1));
    return queueMessageToForKit(answerMessage);
}

/// Cleans up dead children.
static void cleanupChildren()
{
    if (Util::isKitInProcess())
        return;

    Util::assertIsLocked(NewChildrenMutex);

    const int count = NewChildren.size();
    for (int i = count - 1; i >= 0; --i)
    {
        if (!NewChildren[i]->isAlive())
        {
            LOG_WRN("Removing dead spare child [" << NewChildren[i]->getPid() << "].");
            NewChildren.erase(NewChildren.begin() + i);
        }
    }

    if (static_cast<int>(NewChildren.size()) != count)
        SigUtil::addActivity("removed " + std::to_string(count - NewChildren.size()) +
                             " children");
}

/// Decides how many children need spawning and spawns.
static void rebalanceChildren(const std::string& configId, int64_t balance)
{
    Util::assertIsLocked(NewChildrenMutex);

    // Remove dead children first so the available count is accurate.
    cleanupChildren();

    int64_t available = 0;
    for (const auto& elem : NewChildren)
    {
        if (elem->getConfigId() == configId)
            ++available;
    }

    LOG_TRC("Rebalance children to " << balance << ", have " << available << " and "
                                     << OutstandingForks[configId] << " outstanding requests");

    const auto duration = (std::chrono::steady_clock::now() - LastForkRequestTimes[configId]);
    const auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
    if (OutstandingForks[configId] != 0 && durationMs >= ChildSpawnTimeoutMs.load())
    {
        // Children taking too long to spawn.
        // Forget we had requested any, and request anew.
        LOG_WRN("ForKit not responsive for " << durationMs << " forking " << OutstandingForks[configId]
                                             << " children. Resetting.");
        TotalOutstandingForks -= OutstandingForks[configId];
        OutstandingForks[configId] = 0;
    }

    if (OutstandingForks[configId] != 0)
    {
        LOG_DBG("prespawnChildren ["
                << configId << "]: Have " << OutstandingForks[configId]
                << " outstanding fork requests. Time since last request: " << durationMs);
        return;
    }

    balance -= available;

    if (balance > 0)
    {
        LOG_DBG("prespawnChildren ["
                << configId << "]: Have " << available << " spare "
                << (available == 1 ? "child" : "children")
                << " (total: " << NewChildren.size() << "), forking " << balance
                << " more. Time since last request: " << durationMs);
        forkChildren(configId, balance);
    }
}

/// Proactively spawn children processes
/// to load documents with alacrity.
static void prespawnChildren()
{
    // Rebalance if not forking already.
    std::unique_lock<std::mutex> lock(NewChildrenMutex, std::defer_lock);
    if (lock.try_lock())
    {
        rebalanceChildren("", COOLWSD::NumPreSpawnedChildren);
    }
}

#else // MOBILEAPP
void prespawnChildren();
#endif // MOBILEAPP

static size_t addNewChild(std::shared_ptr<ChildProcess> child)
{
    assert(child && "Adding null child");
    const auto pid = child->getPid();
    const std::string& configId = child->getConfigId();

    std::unique_lock<std::mutex> lock(NewChildrenMutex);

    --TotalOutstandingForks;
    --OutstandingForks[configId];
    // Prevent from going -ve if we have unexpected children.
    if (OutstandingForks[configId] < 0)
    {
        ++TotalOutstandingForks;
        ++OutstandingForks[configId];
    }

    if (COOLWSD::IsBindMountingEnabled)
    {
        // Reset the child-spawn timeout to the default, now that we're set.
        // But only when mounting is enabled. Otherwise, copying is always slow.
        ChildSpawnTimeoutMs = std::chrono::milliseconds(CHILD_TIMEOUT_MS);
    }

    LOG_TRC("Adding a new child " << pid << " with config " << configId
                                  << " to NewChildren, have "
                                  << OutstandingForks[configId]
                                  << " outstanding requests");
    SigUtil::addActivity("added child " + std::to_string(pid));
    NewChildren.emplace_back(std::move(child));
    const size_t count = NewChildren.size();
    lock.unlock();

    LOG_INF("Have " << count << " spare " << (count == 1 ? "child" : "children")
                    << " after adding [" << pid << "]. Notifying.");

    NewChildrenCV.notify_one();
    return count;
}

#if !MOBILEAPP

namespace
{

#if ENABLE_DEBUG
inline std::string getLaunchBase(bool asAdmin = false)
{
    std::ostringstream oss;
    oss << "    ";
    oss << ((ConfigUtil::isSslEnabled() || ConfigUtil::isSSLTermination()) ? "https://"
                                                                           : "http://");

    if (asAdmin)
    {
        auto user = ConfigUtil::getConfigValue<std::string>("admin_console.username", "");
        auto passwd = ConfigUtil::getConfigValue<std::string>("admin_console.password", "");

        if (user.empty() || passwd.empty())
            return std::string();

        oss << user << ':' << passwd << '@';
    }

    oss << COOLWSD_TEST_HOST ":";
    oss << ClientPortNumber;

    return oss.str();
}

inline std::string getLaunchURI(const std::string &document, bool readonly = false)
{
    std::ostringstream oss;

    oss << getLaunchBase();
    oss << COOLWSD::ServiceRoot;
    oss << COOLWSD_TEST_COOL_UI;
    oss << "?file_path=";
    const std::string dir = DEBUG_ABSSRCDIR "/";
    oss << Uri::encode(dir + document);
    if (readonly)
        oss << "&permission=readonly";

    return oss.str();
}

inline std::string getServiceURI(const std::string &sub, bool asAdmin = false)
{
    std::ostringstream oss;

    oss << getLaunchBase(asAdmin);
    oss << COOLWSD::ServiceRoot;
    oss << sub;

    return oss.str();
}

#endif

} // anonymous namespace

#endif // MOBILEAPP

#if !MOBILEAPP
std::atomic<int> COOLWSD::ForKitProcId(-1);
std::shared_ptr<ForKitProcess> COOLWSD::ForKitProc;
bool COOLWSD::NoCapsForKit = false;
bool COOLWSD::NoSeccomp = false;
bool COOLWSD::AdminEnabled = true;
bool COOLWSD::UnattendedRun = false;
bool COOLWSD::SignalParent = false;
bool COOLWSD::UseEnvVarOptions = false;
std::string COOLWSD::RouteToken;
#if ENABLE_DEBUG
bool COOLWSD::SingleKit = false;
bool COOLWSD::ForceCaching = false;
#endif
COOLWSD::WASMActivationState COOLWSD::WASMState = COOLWSD::WASMActivationState::Disabled;
std::unordered_map<std::string, std::chrono::steady_clock::time_point> COOLWSD::Uri2WasmModeMap;
#endif
std::string COOLWSD::SysTemplate;
std::string COOLWSD::LoTemplate = LO_PATH;
std::string COOLWSD::CleanupChildRoot;
std::string COOLWSD::ChildRoot;
std::string COOLWSD::ServerName;
std::string COOLWSD::FileServerRoot;
std::string COOLWSD::ServiceRoot;
std::string COOLWSD::TmpFontDir;
std::string COOLWSD::LOKitVersion;
std::string COOLWSD::LOKitVersionNumber;
std::string COOLWSD::LOKitVersionHash;
std::string COOLWSD::ConfigFile =
#if defined(MACOS) && MOBILEAPP
    getResourcePath("coolwsd", "xml");
#else
    COOLWSD_CONFIGDIR "/coolwsd.xml";
#endif
std::string COOLWSD::ConfigDir = COOLWSD_CONFIGDIR "/conf.d";
bool COOLWSD::EnableTraceEventLogging = false;
bool COOLWSD::EnableAccessibility = false;
bool COOLWSD::EnableMountNamespaces = false;
FILE *COOLWSD::TraceEventFile = NULL;
std::string COOLWSD::LogLevel = "trace";
std::string COOLWSD::LogLevelStartup = "trace";
std::string COOLWSD::LogDisabledAreas = "Socket,WebSocket,Admin,Pixel";
std::string COOLWSD::LogToken;
std::string COOLWSD::MostVerboseLogLevelSettableFromClient = "notice";
std::string COOLWSD::LeastVerboseLogLevelSettableFromClient = "fatal";
std::string COOLWSD::UserInterface = "default";
bool COOLWSD::AnonymizeUserData = false;
bool COOLWSD::CheckCoolUser = true;
bool COOLWSD::CleanupOnly = false; ///< If we should cleanup and exit.
#if ENABLE_DEBUG
bool COOLWSD::FindFreePort = false; ///< If we should find a free port to listen on.
#endif
bool COOLWSD::IsProxyPrefixEnabled = false;
unsigned COOLWSD::MaxConnections;
unsigned COOLWSD::MaxDocuments;
std::string COOLWSD::HardwareResourceWarning = "ok";
std::string COOLWSD::OverrideWatermark;
std::set<const Poco::Util::AbstractConfiguration*> COOLWSD::PluginConfigurations;
std::chrono::steady_clock::time_point COOLWSD::StartTime;
bool COOLWSD::IsBindMountingEnabled = true;
bool COOLWSD::IndirectionServerEnabled = false;
bool COOLWSD::GeolocationSetup = false;

// If you add global state please update dumpState below too

static std::string UnitTestLibrary;

unsigned int COOLWSD::NumPreSpawnedChildren = 0;
std::unique_ptr<TraceFileWriter> COOLWSD::TraceDumper;

class PrisonPoll : public TerminatingPoll
{
public:
    PrisonPoll() : TerminatingPoll("prisoner_poll") {}

    /// Check prisoners are still alive and balanced.
    void wakeupHook() override;

#if !MOBILEAPP
    // Resets the forkit process object
    void setForKitProcess(const std::shared_ptr<ForKitProcess>& forKitProc,
                          LOG_CAPTURE_CALLER_DECLARATION)
    {
        assertCorrectThread(LOG_PASS_PARENT_CALLER);
        _forKitProc = forKitProc;
        if (forKitProc && !_queuedSendMessages.empty())
        {
            for (const auto& msg : _queuedSendMessages)
                sendMessageToForKit(msg, forKitProc);
            _queuedSendMessages.clear();
        }
    }

    void sendMessageToForKit(const std::string& msg,
                             const std::weak_ptr<ForKitProcess>& proc,
                             bool queueIfUnavailable = false)
    {
        if (ProcUtil::getThreadId() == getThreadOwner())
        {
            // Speed up sending the message if the request comes from owner thread
            doSendMessage(msg, proc, queueIfUnavailable);
        }
        else
        {
            // Put the message in the owner's thread queue to be send later
            // because WebSocketHandler is not thread safe and otherwise we
            // should synchronize inside WebSocketHandler.
            addCallback([this, msg, proc, queueIfUnavailable]{
                doSendMessage(msg, proc, queueIfUnavailable);
            });
        }
    }

    void sendMessageToForKit(const std::string& msg)
    {
        sendMessageToForKit(msg, _forKitProc, false);
    }

    void queueMessageToForKit(const std::string& msg)
    {
        sendMessageToForKit(msg, _forKitProc, true);
    }

private:
    std::weak_ptr<ForKitProcess> _forKitProc;
    std::vector<std::string> _queuedSendMessages;

    void doSendMessage(const std::string& msg,
                       const std::weak_ptr<ForKitProcess>& proc,
                       bool queueIfUnavailable)
    {
        std::shared_ptr<ForKitProcess> forKitProc = proc.lock();
        if (forKitProc)
            forKitProc->sendTextFrame(msg);
        else if (queueIfUnavailable)
        {
            _queuedSendMessages.push_back(msg);
            LOG_TRC("queued forkit message: [" << msg << "]. " << _queuedSendMessages.size() << " in queue.");
        }
        else
            LOG_DBG("dropping forkit message: " << msg);
    }

#endif
};

/// This thread listens for and accepts prisoner kit processes.
/// And also cleans up and balances the correct number of children.
static std::shared_ptr<PrisonPoll> PrisonerPoll;

std::shared_ptr<ChildProcess> getNewChild_Blocks(const std::shared_ptr<SocketPoll>& destPoll,
                                                 const std::string& configId,
                                                 unsigned mobileAppDocId)
{
    (void)mobileAppDocId;
    const auto startTime = std::chrono::steady_clock::now();

    std::unique_lock<std::mutex> lock(NewChildrenMutex);

#if !MOBILEAPP
    assert(mobileAppDocId == 0 && "Unexpected to have mobileAppDocId in the non-mobile build");

    std::chrono::milliseconds spawnTimeoutMs = ChildSpawnTimeoutMs.load() / 2;

    if (configId.empty() || SubForKitProcs.contains(configId))
    {
        int numPreSpawn = COOLWSD::NumPreSpawnedChildren;
        ++numPreSpawn; // Replace the one we'll dispatch just now.
        LOG_DBG("getNewChild: Rebalancing children of config[" << configId << "] to " << numPreSpawn);
        rebalanceChildren(configId, numPreSpawn);
    }
    else
    {
        // configId exists, and no SubForKitProcs for it seen yet, be more generous for startup time.
        spawnTimeoutMs = std::chrono::milliseconds(CHILD_SPAWN_TIMEOUT_MS);
        LOG_DBG("getNewChild: awaiting subforkit[" << configId << "], timeout of " << spawnTimeoutMs << "ms");
    }

    const std::chrono::milliseconds timeout = spawnTimeoutMs;
    LOG_TRC("Waiting for a new child for a max of " << timeout);
#else // MOBILEAPP
    const auto timeout = std::chrono::hours(100);

#if defined(IOS) || defined(QTAPP) || defined(MACOS) || defined(_WIN32)
    assert(mobileAppDocId > 0 && "Unexpected to have no mobileAppDocId in the mobile build");
#endif

    std::thread([&]
                {
                    ProcUtil::setThreadName("lokit_main_" + Util::encodeId(mobileAppDocId, 3));

                    // Ugly to have that static global PrisonerServerSocketFD, Otoh we know
                    // there is just one COOLWSD object. (Even in real Online.)
                    lokit_main(PrisonerServerSocketFD, COOLWSD::UserInterface, mobileAppDocId);
                }).detach();
#endif // MOBILEAPP

    // FIXME: blocks ...
    // Unfortunately we need to wait after spawning children to avoid bombing the system.
    // If we fail fast and return, the next document will spawn more children without knowing
    // there are some on the way already. And if the system is slow already, that wouldn't help.
    LOG_TRC("Waiting for NewChildrenCV");
    if (NewChildrenCV.wait_for(lock, timeout, [configId]()
                               {
                                   LOG_TRC("Predicate for NewChildrenCV wait: NewChildren.size()=" << NewChildren.size());

                                   // find a candidate with matching configId
                                   auto found = std::find_if(
                                       NewChildren.begin(), NewChildren.end(),
                                       [configId](const auto& candidate) -> bool
                                       { return candidate->getConfigId() == configId; });

                                   const bool candidateMatch = found != NewChildren.end();
                                   // move this candidate into the last position
                                   if (candidateMatch)
                                        std::swap(*found, NewChildren.back());
                                   return candidateMatch;
                               }))
    {
        LOG_TRC("NewChildrenCV wait successful");
        std::shared_ptr<ChildProcess> child = NewChildren.back();
        NewChildren.pop_back();
        const size_t available = NewChildren.size();

        // Release early before moving sockets.
        lock.unlock();

        // Validate before returning.
        if (child && child->isAlive())
        {
            LOG_DBG("getNewChild: Have "
                    << available << " spare " << (available == 1 ? "child" : "children")
                    << " after popping [" << child->getPid() << "] to return in "
                    << std::chrono::duration_cast<std::chrono::milliseconds>(
                           std::chrono::steady_clock::now() - startTime));

            // Change ownership now.
            child->moveSocketFromTo(PrisonerPoll, destPoll);

            return child;
        }

        LOG_WRN("getNewChild: popped dead child, need to find another.");
    }
    else
    {
        LOG_TRC("NewChildrenCV wait failed");
        LOG_WRN("getNewChild: No child available. Sending spawn request to forkit and failing.");
    }

    LOG_DBG("getNewChild: Timed out while waiting for new child.");

    if (!configId.empty())
    {
        auto it = SubForKitProcs.find(configId);
        if (it != SubForKitProcs.end())
        {
            LOG_WRN("subForKit " << configId << " failed to respond, resetting it");
            dropSubForKit(it);
        }
    }

    return nullptr;
}

#ifdef __linux__
#if !MOBILEAPP
class InotifySocket : public Socket
{
public:
    InotifySocket(std::chrono::steady_clock::time_point creationTime):
        Socket(inotify_init1(IN_NONBLOCK), Socket::Type::Unix, creationTime)
        , _stopOnConfigChange(true)
    {
        if (getFD() == -1)
        {
            LOG_WRN("Inotify - Failed to start a watcher for the configuration, disabling "
                    "stop_on_config_change");
            _stopOnConfigChange = false;
            return;
        }

        watch(COOLWSD_CONFIGDIR);
    }

    /// Check for file changes, stop the server if we find any
    void handlePoll(SocketDisposition &disposition, std::chrono::steady_clock::time_point now, int events) override;

    int getPollEvents(std::chrono::steady_clock::time_point /* now */,
                      int64_t & /* timeoutMaxMicroS */) override
    {
        return POLLIN;
    }

    bool watch(std::string_view configFile);

private:
    int _watchedCount = 0;
    bool _stopOnConfigChange;
};

bool InotifySocket::watch(const std::string_view configFile)
{
    LOG_TRC("Inotify - Attempting to watch " << configFile << ", in addition to current "
                                             << _watchedCount << " watched files");

    if (getFD() == -1)
    {
        LOG_WRN("Inotify - Trying to watch config file " << configFile
                                                         << " without an inotify file descriptor");
        return false;
    }

    int watchedStatus;
    watchedStatus = inotify_add_watch(getFD(), configFile.data(), IN_MODIFY);

    if (watchedStatus == -1)
        LOG_WRN("Inotify - Failed to watch config file " << configFile);
    else
        _watchedCount++;

    return watchedStatus != -1;
}

void InotifySocket::handlePoll(SocketDisposition & /* disposition */, std::chrono::steady_clock::time_point /* now */, int /* events */)
{
    LOG_TRC("InotifyPoll - woken up. Reload on config change: "
            << _stopOnConfigChange << ", Watching " << _watchedCount << " files");
    if (!_stopOnConfigChange)
        return;

    char buf[4096];

    static_assert(sizeof(buf) >= sizeof(struct inotify_event) + NAME_MAX + 1, "see man 7 inotify");

    const struct inotify_event* event;

    LOG_TRC("InotifyPoll - Checking for config changes...");

    while (true)
    {
        ssize_t len = read(getFD(), buf, sizeof(buf));

        if (len == -1 && errno != EAGAIN)
        {
            // Some read error, EAGAIN is when there is no data so let's not warn for it
            LOG_WRN_SYS("InotifyPoll - Read error when trying to get events");
        }
        else if (len == -1)
        {
            LOG_TRC("InotifyPoll - Got to end of data when reading inotify");
        }

        if (len <= 0)
            break;

        assert(buf[len - 1] == 0 && "see man 7 inotify");

        for (char* ptr = buf; ptr < buf + len; ptr += sizeof(struct inotify_event) + event->len)
        {
            event = (const struct inotify_event*)ptr;

            LOG_WRN("InotifyPoll - Config file " << event->name << " was modified, stopping COOLWSD");
            SigUtil::requestShutdown();
        }
    }
}

#endif // if !MOBILEAPP
#endif // #ifdef __linux__

#if !MOBILEAPP

void ForKitProcWSHandler::handleMessage(const std::vector<char> &data)
{
    LOG_TRC("ForKitProcWSHandler: handling incoming [" << COOLProtocol::getAbbreviatedMessage(data.data(), data.size()) << "].");
    const std::string firstLine = COOLProtocol::getFirstLine(data.data(), data.size());
    const StringVector tokens = StringVector::tokenize(firstLine.data(), firstLine.size());

    if (tokens.startsWith(0, "segfaultcount"))
    {
        int segFaultcount = 0;
        int killedCount = 0;
        int oomKilledCount = 0;
        if (COOLProtocol::getNonNegTokenInteger(tokens[0], "segfaultcount", segFaultcount)
            && COOLProtocol::getNonNegTokenInteger(tokens[1], "killedcount", killedCount)
            && COOLProtocol::getNonNegTokenInteger(tokens[2], "oomkilledcount", oomKilledCount))
        {
            Admin::instance().addErrorExitCounters(segFaultcount, killedCount, oomKilledCount);

            if (segFaultcount)
            {
                LOG_INF(segFaultcount << " coolkit processes crashed with segmentation fault.");
                SigUtil::addActivity("coolkit(s) crashed");
                UNITWSD_CALL(kitSegfault(segFaultcount));
            }

            if (killedCount)
            {
                LOG_INF(killedCount << " coolkit processes killed.");
                SigUtil::addActivity("coolkit(s) killed");
                UNITWSD_CALL(kitKilled(killedCount));
            }

            if (oomKilledCount)
            {
                LOG_INF(oomKilledCount << " coolkit processes killed by oom.");
                SigUtil::addActivity("coolkit(s) killed by oom");
                UNITWSD_CALL(kitOomKilled(oomKilledCount));
            }
        }
        else
        {
            LOG_WRN(
                "ForKitProcWSHandler: Invalid 'segfaultcount' message received. Got:" << firstLine);
        }
    }
    else
    {
        LOG_ERR("ForKitProcWSHandler: unknown command: " << tokens[0]);
    }
}

#endif

COOLWSD::COOLWSD()
{
}

COOLWSD::~COOLWSD()
{
    if (UnitBase::isUnitTesting())
    {
        // We won't have a valid UnitWSD::get() when not testing.
        UNITWSD_CALL(setWSD(nullptr));
    }
}

#if !MOBILEAPP

bool queueMessageToForKit(const std::string& message)
{
    if (!PrisonerPoll)
        return false;
    PrisonerPoll->queueMessageToForKit(message);
    return true;
}

void COOLWSD::requestTerminateSpareKits()
{
    // Request existing spare kits to quit, to get replaced with ones that
    // include the new fonts.
    if (PrisonerPoll)
    {
        PrisonerPoll->addCallback(
            []
            {
                std::unique_lock<std::mutex> lock(NewChildrenMutex);
                const int count = NewChildren.size();
                for (int i = count - 1; i >= 0; --i)
                    NewChildren[i]->requestTermination();
            });
    }
}

// Due to the possibility of enterMountingNS failing at an intermediate stage
// after entering a usernamespace, but unable to enter a useful mounting namespace
// do a test mount in another separate child whose failure don't affect the parent
bool COOLWSD::testMountingNSInFork()
{
    Log::preFork();

    pid_t pid = fork();
    if (!pid)
    {
        // Child
        Log::postFork();

        // setupChildRoot does a test bind mount + umount to see if that fully works
        // so we have a mount namespace here just for the purposes of that test
        LOG_DBG("Test moving into user namespace as uid 0 in level 2 child");

        int ret = JailUtil::enterMountingNS(geteuid(), getegid());

        LOG_DBG("Level 2 child enterMountingNS result is: " << ret);

        _exit(ret);
    }

    // Parent

    if (pid == -1)
    {
        LOG_SYS("testMountingNSInFork fork failed");
        return false;
    }

    int wstatus;
    const int rc = waitpid(pid, &wstatus, 0);
    if (rc == -1)
    {
        LOG_SYS("testMountingNSInFork waitpid failed");
        return false;
    }

    if (!WIFEXITED(wstatus))
    {
        LOG_SYS("testMountingNSInFork abnormal termination");
        return false;
    }

    int status = WEXITSTATUS(wstatus);
    LOG_DBG("testMountingNSInFork status: " << std::hex << status << std::dec);
    return status == 1;
}

void COOLWSD::setupChildRoot(const bool UseMountNamespaces)
{
    JailUtil::disableBindMounting(); // Default to assume failure
    JailUtil::disableMountNamespaces();

#if ENABLE_CHILDROOTS
    Log::preFork();

    pid_t pid = fork();
    if (!pid)
    {
        // Child
        Log::postFork();

        int ret = 0;

        // Do the setup in a fork so we have no other threads running which
        // disrupt creation of linux namespaces

        if (UseMountNamespaces)
        {
            // setupChildRoot does a test bind mount + umount to see if that fully works
            // so we have a mount namespace here just for the purposes of that test

            // First see if it works in (another) throw away child so a successful
            // NEWUSER, but a failed NEWNS, or an unusable one, doesn't affect this
            // process. So on failure we can skip the enterMountingNS at this level.
            const bool childMountWorked = COOLWSD::testMountingNSInFork();

            LOG_DBG("Move into user namespace as uid 0");
            if (childMountWorked && JailUtil::enterMountingNS(geteuid(), getegid()))
                JailUtil::enableMountNamespaces();
            else
                LOG_ERR("creating usernamespace for mount user failed.");
        }

        // Setup the jails.
        JailUtil::cleanupJails(CleanupChildRoot);
        JailUtil::setupChildRoot(IsBindMountingEnabled, ChildRoot, SysTemplate);

        if (JailUtil::isMountNamespacesEnabled())
            ret |= (1 << 0);
        if (JailUtil::isBindMountingEnabled())
            ret |= (1 << 1);

        _exit(ret);
    }

    // Parent

    if (pid == -1)
    {
        LOG_SYS("setupChildRoot fork failed");
        return;
    }

    int wstatus;
    const int rc = waitpid(pid, &wstatus, 0);
    if (rc == -1)
    {
        LOG_SYS("setupChildRoot waitpid failed");
        return;
    }

    if (!WIFEXITED(wstatus))
    {
        LOG_SYS("setupChildRoot abnormal termination");
        return;
    }

    int status = WEXITSTATUS(wstatus);
    LOG_DBG("setupChildRoot status: " << std::hex << status << std::dec);
    IsBindMountingEnabled = (status & (1 << 1));
    LOG_INF("Using Bind Mounting: " << IsBindMountingEnabled);
    EnableMountNamespaces = (status & (1 << 0));
    LOG_INF("Using Mount Namespaces: " << EnableMountNamespaces);
    if (IsBindMountingEnabled)
        JailUtil::enableBindMounting();
    if (EnableMountNamespaces)
        JailUtil::enableMountNamespaces();
    if (ConfigUtil::getConfigValue<bool>("mount_jail_tree", true))
        JailUtil::enableBindMountingConfigured();
    else
        JailUtil::disableBindMountingConfigured();
#else
    (void) UseMountNamespaces;
#endif
}

#endif

void COOLWSD::innerInitialize(Poco::Util::Application& self)
{
#if !MOBILEAPP
    if (geteuid() == 0 && CheckCoolUser)
    {
        throw std::runtime_error("Do not run as root. Please run as cool user.");
    }
#endif

    Util::setApplicationPath(
        Poco::Path(Poco::Util::Application::instance().commandPath()).parent().toString());

    StartTime = std::chrono::steady_clock::now();

    // Initialize the config subsystem.
    LayeredConfiguration& conf = config();

    const std::unordered_map<std::string, std::string>& defAppConfig =
        ConfigUtil::getDefaultAppConfig();

    // Set default values, in case they are missing from the config file.
    Poco::AutoPtr<ConfigUtil::AppConfigMap> defConfig(new ConfigUtil::AppConfigMap(defAppConfig));
    conf.addWriteable(defConfig, PRIO_SYSTEM); // Lowest priority

#if !MOBILEAPP

    // Load default configuration files, with name independent
    // of Poco's view of app-name, from local file if present.
    // Fallback to the COOLWSD_CONFIGDIR or --config-file path.
    Poco::Path configPath("coolwsd.xml");
    const std::string configFilePath =
        Poco::Util::Application::findFile(configPath) ? configPath.toString() : ConfigFile;
    loadConfiguration(configFilePath, PRIO_DEFAULT);

    // Override any settings passed on the command-line or via environment variables
    if (UseEnvVarOptions)
        initializeEnvOptions();
    Poco::AutoPtr<ConfigUtil::AppConfigMap> overrideConfig(
        new ConfigUtil::AppConfigMap(_overrideSettings));
    conf.addWriteable(overrideConfig, PRIO_APPLICATION); // Highest priority

    // This caches some oft-used settings and must come after overriding.
    ConfigUtil::initialize(&config());

    // Load extra ("plug-in") configuration files, if present
    Poco::File dir(ConfigDir);
    if (dir.exists() && dir.isDirectory())
    {
        const Poco::DirectoryIterator end;
        for (Poco::DirectoryIterator configFileIterator(dir); configFileIterator != end;
             ++configFileIterator)
        {
            // Only accept configuration files ending in .xml
            const std::string configFile = configFileIterator.path().getFileName();
            if (configFile.length() > 4 && strcasecmp(configFile.substr(configFile.length() - 4).data(), ".xml") == 0)
            {
                const std::string fullFileName = dir.path() + "/" + configFile;
                PluginConfigurations.insert(new Poco::Util::XMLConfiguration(fullFileName));
            }
        }
    }

    if (!UnitTestLibrary.empty())
    {
        UnitWSD::defaultConfigure(conf);
    }

    // Experimental features.
    EnableExperimental = ConfigUtil::getConfigValue<bool>(conf, "experimental_features", false);

    EnableAccessibility = ConfigUtil::getConfigValue<bool>(conf, "accessibility.enable", false);

    // Setup user interface mode
    UserInterface = ConfigUtil::getConfigValue<std::string>(conf, "user_interface.mode", "default");

    if (UserInterface == "compact")
        UserInterface = "classic";

    if (UserInterface == "tabbed")
        UserInterface = "notebookbar";

    if (EnableAccessibility)
        UserInterface = "notebookbar";

    // Load view mode file extensions configuration
    COOLWSD::ViewModeFileExtensions = ConfigUtil::getConfigValue<std::string>(
        conf, "view_mode.file_extensions", "");

    // Set the log-level after complete initialization to force maximum details at startup.
    LogLevel = ConfigUtil::getConfigValue<std::string>(conf, "logging.level", "trace");
    LogDisabledAreas = ConfigUtil::getConfigValue<std::string>(conf, "logging.disabled_areas",
                                                               "Socket,WebSocket,Admin");
    MostVerboseLogLevelSettableFromClient = ConfigUtil::getConfigValue<std::string>(
        conf, "logging.most_verbose_level_settable_from_client", "notice");
    LeastVerboseLogLevelSettableFromClient = ConfigUtil::getConfigValue<std::string>(
        conf, "logging.least_verbose_level_settable_from_client", "fatal");

    setenv("COOL_LOGLEVEL", LogLevel.c_str(), true);
    setenv("COOL_LOGDISABLED_AREAS", LogDisabledAreas.c_str(), true);

    if constexpr (!Util::isDebugEnabled())
    {
        const std::string salLog =
            ConfigUtil::getConfigValue<std::string>(conf, "logging.lokit_sal_log", "-INFO-WARN");
        setenv("SAL_LOG", salLog.c_str(), 0);
    }

#if WASMAPP
    // In WASM, we want to log to the Log Console.
    // Disable logging to file to log to stdout and
    // disable color since this isn't going to the terminal.
    constexpr bool withColor = false;
    constexpr bool logToFile = false;
    constexpr bool logToFileUICmd = false;
#else
    const bool withColor =
        ConfigUtil::getConfigValue<bool>(conf, "logging.color", true) && isatty(fileno(stderr));
    if (withColor)
    {
        setenv("COOL_LOGCOLOR", "1", true);
    }

    const auto logToFile = ConfigUtil::getConfigValue<bool>(conf, "logging.file[@enable]", false);
    std::map<std::string, std::string> logProperties;
    if (logToFile)
    {
        for (std::size_t i = 0;; ++i)
        {
            const std::string confPath = "logging.file.property[" + std::to_string(i) + ']';
            const std::string confName = config().getString(confPath + "[@name]", "");
            if (!confName.empty())
            {
                const std::string value = config().getString(confPath, "");
                logProperties.emplace(confName, value);
            }
            else if (!config().has(confPath))
            {
                break;
            }
        }

        // Setup the logfile envar for the kit processes.
        const auto it = logProperties.find("path");
        if (it != logProperties.end())
        {
            setenv("COOL_LOGFILE", "1", true);
            setenv("COOL_LOGFILENAME", it->second.c_str(), true);
            if (!CleanupOnly)
            {
                std::cerr << "\nLogging at " << LogLevel << " level to file: " << it->second.c_str()
                    << std::endl;
            }
        }
    }

    // Do the same for ui command logging
    const bool logToFileUICmd =
        ConfigUtil::getConfigValue<bool>(conf, "logging_ui_cmd.file[@enable]", false);
    std::map<std::string, std::string> logPropertiesUICmd;
    if (logToFileUICmd)
    {
        for (std::size_t i = 0;; ++i)
        {
            const std::string confPath = "logging_ui_cmd.file.property[" + std::to_string(i) + ']';
            const std::string confName = config().getString(confPath + "[@name]", "");
            if (!confName.empty())
            {
                const std::string value = config().getString(confPath, "");
                logPropertiesUICmd.emplace(confName, value);
            }
            else if (!config().has(confPath))
            {
                break;
            }
        }

        // Setup the logfile envar for the kit processes.
        const auto it = logPropertiesUICmd.find("path");
        if (it != logPropertiesUICmd.end())
        {
            setenv("COOL_LOGFILE_UICMD", "1", true);
            setenv("COOL_LOGFILENAME_UICMD", it->second.c_str(), true);
            if (!CleanupOnly)
            {
                std::cerr << "\nLogging UI Commands to file: " << it->second.c_str() << std::endl;
            }
        }
        const bool merge = ConfigUtil::getConfigValue<bool>(conf, "logging_ui_cmd.merge", true);
        const bool logEndtime =
            ConfigUtil::getConfigValue<bool>(conf, "logging_ui_cmd.merge_display_end_time", false);
        if (merge)
        {
            setenv("COOL_LOG_UICMD_MERGE", "1", true);
        }
        if (logEndtime)
        {
            setenv("COOL_LOG_UICMD_END_TIME", "1", true);
        }
    }
#endif

    // Log at trace level until we complete the initialization.
    LogLevelStartup =
        ConfigUtil::getConfigValue<std::string>(conf, "logging.level_startup", "trace");
    setenv("COOL_LOGLEVEL_STARTUP", LogLevelStartup.c_str(), true);

    Log::initialize("wsd", LogLevelStartup, withColor, logToFile, logProperties, logToFileUICmd, logPropertiesUICmd);
    if (LogLevel != LogLevelStartup)
    {
        LOG_INF("Setting log-level to [" << LogLevelStartup << "] and delaying setting to ["
                << LogLevel << "] until after WSD initialization.");
    }

    Util::sleepFromEnvIfSet("Coolwsd", "SLEEPFORDEBUGGER");

    if (ConfigUtil::getConfigValue<bool>(conf, "browser_logging", false))
    {
        LogToken = Util::rng::getHexString(16);
    }

    // First log entry.
    ServerName = config().getString("server_name");
    LOG_INF("Initializing coolwsd " << Util::getCoolVersion() << " server [" << ServerName
                                    << "]. Experimental features are "
                                    << (EnableExperimental ? "enabled." : "disabled."));

    std::ostringstream ossConfig;
    ossConfig << "Loaded config file [" << configFilePath << "] (non-default values):\n";
    ossConfig << ConfigUtil::getLoggableConfig(conf);
    LOG_DBG_S("View mode extensions: [" << COOLWSD::ViewModeFileExtensions << ']');

    LoggableConfigEntries = ossConfig.str();
    LOG_INF(LoggableConfigEntries);

    // Initialize the UnitTest subsystem.
    if (!UnitWSD::init(UnitWSD::UnitType::Wsd, UnitTestLibrary))
    {
        throw std::runtime_error("Failed to load wsd unit test library.");
    }
    UNITWSD_CALL(setWSD(this));

    // Allow UT to manipulate before using configuration values.
    UNITWSD_CALL(configure(conf));

    // Trace Event Logging.
    EnableTraceEventLogging = ConfigUtil::getConfigValue<bool>(conf, "trace_event[@enable]", false);

    if (EnableTraceEventLogging)
    {
        const auto traceEventFile = ConfigUtil::getConfigValue<std::string>(
            conf, "trace_event.path", COOLWSD_TRACEEVENTFILE);
        LOG_INF("Trace Event file is " << traceEventFile << ".");
        TraceEventFile = fopen(traceEventFile.c_str(), "w");
        if (TraceEventFile != NULL)
        {
            if (fcntl(fileno(TraceEventFile), F_SETFD, FD_CLOEXEC) == -1)
            {
                fclose(TraceEventFile);
                TraceEventFile = NULL;
            }
            else
            {
                fprintf(TraceEventFile, "[\n");
                // Output a metadata event that tells that this is the WSD process
                fprintf(TraceEventFile,
                        "{\"name\":\"process_name\",\"ph\":\"M\",\"args\":{\"name\":\"WSD\"},"
                        "\"pid\":%ld,\"tid\":%ld},\n",
                        ProcUtil::getProcessId(), ProcUtil::getThreadId());
                fprintf(TraceEventFile,
                        "{\"name\":\"thread_name\",\"ph\":\"M\",\"args\":{\"name\":\"Main\"},"
                        "\"pid\":%ld,\"tid\":%ld},\n",
                        ProcUtil::getProcessId(), ProcUtil::getThreadId());
            }
        }
    }

    // Check deprecated settings.
    if (ConfigUtil::hasProperty("storage.wopi.reuse_cookies"))
        LOG_WRN("NOTE: Deprecated config option storage.wopi.reuse_cookies is no longer supported");

    COOLWSD::WASMState = ConfigUtil::getConfigValue<bool>(conf, "wasm.enable", false)
                             ? COOLWSD::WASMActivationState::Enabled
                             : COOLWSD::WASMActivationState::Disabled;

#if ENABLE_DEBUG
    if (ConfigUtil::getConfigValue<bool>(conf, "wasm.force", false))
    {
        if (COOLWSD::WASMState != COOLWSD::WASMActivationState::Enabled)
        {
            LOG_FTL(
                "WASM is not enabled; cannot force serving WASM. Please set wasm.enabled to true "
                "in coolwsd.xml first");
            Util::forcedExit(EX_SOFTWARE);
        }

        LOG_INF("WASM is force-enabled. All documents will be loaded through WASM");
        COOLWSD::WASMState = COOLWSD::WASMActivationState::Forced;
    }
#endif

    // Get anonymization settings.
#if COOLWSD_ANONYMIZE_USER_DATA
    AnonymizeUserData = true;
    LOG_INF("Anonymization of user-data is permanently enabled.");
#else
    LOG_INF("Anonymization of user-data is configurable.");
    const bool haveAnonymizeUserDataConfig =
        ConfigUtil::getRawConfig(conf, "logging.anonymize.anonymize_user_data", AnonymizeUserData);

    bool anonymizeFilenames = false;
    bool anonymizeUsernames = false;
    if (ConfigUtil::getRawConfig(conf, "logging.anonymize.usernames", anonymizeFilenames) ||
        ConfigUtil::getRawConfig(conf, "logging.anonymize.filenames", anonymizeUsernames))
    {
        LOG_WRN("NOTE: both logging.anonymize.usernames and logging.anonymize.filenames are deprecated and superseded by "
                "logging.anonymize.anonymize_user_data. Please remove username and filename entries from the config and use only anonymize_user_data.");

        if (haveAnonymizeUserDataConfig)
            LOG_WRN("Since logging.anonymize.anonymize_user_data is provided (" << AnonymizeUserData << ") in the config, it will be used.");
        else
        {
            AnonymizeUserData = (anonymizeFilenames || anonymizeUsernames);
        }
    }
#endif

    if (AnonymizeUserData && LogLevel == "trace" && !CleanupOnly)
    {
        if (ConfigUtil::getConfigValue<bool>(conf, "logging.anonymize.allow_logging_user_data",
                                             false))
        {
            LOG_WRN("Enabling trace logging while anonymization is enabled due to logging.anonymize.allow_logging_user_data setting. "
                    "This will leak user-data!");

            // Disable anonymization as it's useless now.
            AnonymizeUserData = false;
        }
        else
        {
            static const char failure[] = "Anonymization and trace-level logging are incompatible. "
                "Please reduce logging level to debug or lower in coolwsd.xml to prevent leaking sensitive user data.";
            LOG_FTL(failure);
            std::cerr << '\n' << failure << std::endl;

            if constexpr (Util::isDebugEnabled())
            {
                std::cerr << "\nIf you have used 'make run', edit coolwsd.xml and make sure you "
                             "have removed "
                             "'--o:logging.level=trace' from the command line in Makefile.am.\n"
                          << std::endl;
            }

            Util::forcedExit(EX_SOFTWARE);
        }
    }

    std::uint64_t anonymizationSalt = 82589933;
    bool highStrengthAnonymize = false;
    LOG_INF("Anonymization of user-data is " << (AnonymizeUserData ? "enabled." : "disabled."));
    if (AnonymizeUserData)
    {
        // Get the salt, if set, otherwise default, and set as envar, so the kits inherit it.
        anonymizationSalt = ConfigUtil::getConfigValue<std::uint64_t>(
            conf, "logging.anonymize.anonymization_salt", 82589933);
        const std::string anonymizationSaltStr = std::to_string(anonymizationSalt);
        setenv("COOL_ANONYMIZATION_SALT", anonymizationSaltStr.c_str(), true);

        highStrengthAnonymize = ConfigUtil::getConfigValue<bool>(
            conf, "logging.anonymize.high_strength", false);
        if (highStrengthAnonymize)
        {
            LOG_INF("Using high-strength cryptographic anonymization (PBKDF2-HMAC-SHA512).");
            setenv("COOL_ANONYMIZATION_HIGH_STRENGTH", "1", true);
        }
    }

    Anonymizer::initialize(AnonymizeUserData, anonymizationSalt, highStrengthAnonymize);

    {
        bool enableWebsocketURP =
            ConfigUtil::getConfigValue<bool>("security.enable_websocket_urp", false);
        if (enableWebsocketURP)
            LOG_WRN("NOTE: Deprecated config option security.enable_websocket_urp is enabled. "
                    "This feature is deprecated and will be removed in a future release.");
        setenv("ENABLE_WEBSOCKET_URP", enableWebsocketURP ? "true" : "false", 1);
    }

    {
        std::string proto = ConfigUtil::getConfigValue<std::string>(conf, "net.proto", "");
        if (Util::iequal(proto, "ipv4"))
            ClientPortProto = Socket::Type::IPv4;
        else if (Util::iequal(proto, "ipv6"))
            ClientPortProto = Socket::Type::IPv6;
        else if (Util::iequal(proto, "all"))
            ClientPortProto = Socket::Type::All;
        else
            LOG_WRN("Invalid protocol: " << proto);
    }

    {
        std::string listen = ConfigUtil::getConfigValue<std::string>(conf, "net.listen", "");
        if (Util::iequal(listen, "any"))
            ClientListenAddr = ServerSocket::Type::Public;
        else if (Util::iequal(listen, "loopback"))
            ClientListenAddr = ServerSocket::Type::Local;
        else
            LOG_WRN("Invalid listen address: " << listen << ". Falling back to default: 'any'" );
    }

    // Prefix for the coolwsd pages; should not end with a '/'
    ServiceRoot = ConfigUtil::getPathFromConfig("net.service_root");
    while (ServiceRoot.length() > 0 && ServiceRoot[ServiceRoot.length() - 1] == '/')
        ServiceRoot.pop_back();

    IsProxyPrefixEnabled = ConfigUtil::getConfigValue<bool>(conf, "net.proxy_prefix", false);

    LOG_INF("SSL support: SSL is " << (ConfigUtil::isSslEnabled() ? "enabled." : "disabled."));
    LOG_INF("SSL support: termination is "
            << (ConfigUtil::isSSLTermination() ? "enabled." : "disabled."));

    std::string allowedLanguages(config().getString("allowed_languages"));
    setenv("KIT_ALLOWLIST_LANGUAGES", allowedLanguages.c_str(), 1);

#endif // !MOBILEAPP

#if defined(DEBUG)
    // Enable if you need more logging from core
    //setenv("SAL_LOG", "+INFO+WARN", 0);

    // Enable if you need to see the top left corner of tile that was rendered
    //setenv("LOK_DEBUG_TILES", "1", 0);
#endif

    if (ConfigUtil::hasProperty("per_document.pdf_resolution_dpi"))
        LOG_WRN("NOTE: Deprecated config option per_document.pdf_resolution_dpi is no longer supported");

    SysTemplate = ConfigUtil::getPathFromConfig("sys_template_path");
    if (SysTemplate.empty())
    {
        LOG_FTL("Missing sys_template_path config entry.");
        throw Poco::Util::MissingOptionException("systemplate");
    }

    ChildRoot = ConfigUtil::getPathFromConfig("child_root_path");
    if (ChildRoot.empty())
    {
        LOG_FTL("Missing child_root_path config entry.");
        throw Poco::Util::MissingOptionException("childroot");
    }
    else
    {
#if !MOBILEAPP
        if (CleanupOnly)
        {
            // Cleanup and exit.
            JailUtil::cleanupJails(ChildRoot);
            Util::forcedExit(EX_OK);
        }
#endif

        if (ChildRoot[ChildRoot.size() - 1] != '/')
            ChildRoot += '/';

#if CODE_COVERAGE
        ::setenv("BASE_CHILD_ROOT", Poco::Path(ChildRoot).absolute().toString().c_str(), 1);
#endif

        // We need to cleanup other people's expired jails
        CleanupChildRoot = ChildRoot;

        // Encode the process id into the path for parallel re-use of jails/
        ChildRoot +=
            std::to_string(ProcUtil::getProcessId()) + '-' + Util::rng::getHexString(8) + '/';

        LOG_DBG("Normalizing childroot: " << ChildRoot);
        ChildRoot = Poco::Path(ChildRoot).makeDirectory().makeAbsolute().toString();
        LOG_DBG("Childroot: " << ChildRoot);
    }

#if !MOBILEAPP

    // Copy and serialize the config into XML to pass to forkit.
    KitXmlConfig.reset(new Poco::Util::XMLConfiguration);
    for (const auto& pair : defAppConfig)
    {
        try
        {
            KitXmlConfig->setString(pair.first, config().getRawString(pair.first));
        }
        catch (const std::exception&)
        {
            // Nothing to do.
        }
    }

    // Fixup some config entries to match out decisions/overrides.
    KitXmlConfig->setBool("ssl.enable", ConfigUtil::isSslEnabled());
    KitXmlConfig->setBool("ssl.termination", ConfigUtil::isSSLTermination());

    // We don't pass the config via command-line
    // to avoid dealing with escaping and other traps.
    std::ostringstream oss;
    KitXmlConfig->save(oss);
    setenv("COOL_CONFIG", oss.str().c_str(), true);

    // For some reason I can't get at this setting in ChildSession::loKitCallback().
    std::string fontsMissingHandling = ConfigUtil::getString("fonts_missing.handling", "log");
    setenv("FONTS_MISSING_HANDLING", fontsMissingHandling.c_str(), 1);

    IsBindMountingEnabled = ConfigUtil::getConfigValue<bool>(conf, "mount_jail_tree", true);
#if CODE_COVERAGE
    // Code coverage is not supported with bind-mounting.
    if (IsBindMountingEnabled)
    {
        LOG_WRN("Mounting is not compatible with code-coverage. Disabling.");
        IsBindMountingEnabled = false;
    }
#endif // CODE_COVERAGE

    // Setup the jails.
    bool UseMountNamespaces = true;

    NoCapsForKit = Util::isKitInProcess() ||
                   !ConfigUtil::getConfigValue<bool>(conf, "security.capabilities", true);
    if (NoCapsForKit && UseMountNamespaces)
    {
        // With NoCapsForKit we don't chroot. If Linux namespaces are available, we could
        // chroot without capabilities, but the richdocumentscode AppImage layout isn't
        // compatible with the systemplate expectations for setting up the chroot so
        // disable MountNamespaces in NoCapsForKit mode for now.
        LOG_WRN("MountNamespaces is not compatible with NoCapsForKit. Disabling.");
        UseMountNamespaces = false;
    }

    LOG_INF("Creating childroot: [" << ChildRoot << "] with" << (UseMountNamespaces ? "" : "out")
                                    << " mount-namespaces");
    setupChildRoot(UseMountNamespaces);

    LOG_DBG("FileServerRoot before config: " << FileServerRoot);
    FileServerRoot = ConfigUtil::getPathFromConfig("file_server_root_path");
    LOG_DBG("FileServerRoot after config: " << FileServerRoot);

    //creating quarantine directory
    if (ConfigUtil::getConfigValue<bool>(conf, "quarantine_files[@enable]", false))
    {
        std::string path = Util::trimmed(ConfigUtil::getPathFromConfig("quarantine_files.path"));
        LOG_INF("Quarantine path is set to [" << path << "] in config");
        if (path.empty())
        {
            LOG_WRN("Quarantining is enabled via quarantine_files config, but no path is set in "
                    "quarantine_files.path. Disabling quarantine");
        }
        else
        {
            if (path[path.size() - 1] != '/')
                path += '/';

            if (path[0] != '/')
                LOG_WRN("Quarantine path is relative. Please use an absolute path for better "
                        "reliability");

            LOG_DBG("Initializing quarantine at [" << path << ']');
            Quarantine::initialize(path);
        }
    }
    else
    {
        LOG_INF("Quarantine is disabled in config");
    }

    {
        // creating cache directory
        std::string path = Util::trimmed(ConfigUtil::getPathFromConfig("cache_files.path"));
        LOG_INF("Cache path is set to [" << path << "] in config");
        if (path.empty())
        {
            LOG_WRN("No cache path is set in cache_files.path. Disabling cache");
        }
        else
        {
            Poco::File p(path);
            try
            {
                LOG_TRC("Creating cache directory [" << path << ']');
                p.createDirectories();

                LOG_DBG("Created cache directory [" << path << ']');
            }
            catch (const std::exception& ex)
            {
                LOG_WRN("Failed to create cache directory [" << path << "]");
            }

            if (FileUtil::Stat(path).exists())
                Cache::initialize(path);
        }
    }

    NumPreSpawnedChildren = ConfigUtil::getConfigValue<int>(conf, "num_prespawn_children", 1);
    if (NumPreSpawnedChildren < 1)
    {
        LOG_WRN("Invalid num_prespawn_children in config (" << NumPreSpawnedChildren << "). Resetting to 1.");
        NumPreSpawnedChildren = 1;
    }
    LOG_INF("NumPreSpawnedChildren set to " << NumPreSpawnedChildren << '.');

    FileUtil::registerFileSystemForDiskSpaceChecks(ChildRoot);

    int threads = std::max<int>(std::thread::hardware_concurrency(), 1);
    int maxConcurrency = ConfigUtil::getConfigValue<int>(conf, "per_document.max_concurrency", 4);

    if (maxConcurrency > 16)
    {
        LOG_WRN("Using a large number of threads for every document puts pressure on "
                "the scheduler, and consumes memory, while providing marginal gains "
                "consider lowering max_concurrency from " << maxConcurrency);
    }
    if (maxConcurrency > threads)
    {
        LOG_ERR("Setting concurrency above the number of physical "
                "threads yields extra latency and memory usage for no benefit. "
                "Clamping " << maxConcurrency << " to " << threads << " threads.");
        maxConcurrency = threads;
    }
    if (maxConcurrency > 0)
    {
        setenv("MAX_CONCURRENCY", std::to_string(maxConcurrency).c_str(), 1);
    }
    LOG_INF("MAX_CONCURRENCY set to " << maxConcurrency << '.');

    // It is worth avoiding configuring with a large number of under-weight
    // containers / VMs - better to have fewer, stronger ones.
    if (threads < 4)
    {
        LOG_WRN("Fewer threads than recommended. Having at least four threads for "
                "provides significant parallelism that can be used for burst "
                "compression of newly visible document pages, giving lower latency.");
        HardwareResourceWarning = "lowresources";
    }

#endif

    const auto redlining =
        ConfigUtil::getConfigValue<bool>(conf, "per_document.redlining_as_comments", false);
    if (!redlining)
    {
        setenv("DISABLE_REDLINE", "1", 1);
        LOG_INF("DISABLE_REDLINE set");
    }

    // Otherwise we profile the soft-device at jail creation time.
    setenv("SAL_DISABLE_OPENCL", "true", 1);
    // Disable getting the OS print queue and default printer
    setenv("SAL_DISABLE_PRINTERLIST", "true", 1);
    setenv("SAL_DISABLE_DEFAULTPRINTER", "true", 1);
    // Disable fsync - we're a state-less container
    setenv("SAL_DISABLE_FSYNC", "true", 1);
    // Staticize our configuration to increase sharing
    setenv("SAL_CONFIG_STATICIZE", "true", 1);

    // Log the connection and document limits.
#if ENABLE_WELCOME_MESSAGE
    if (ConfigUtil::getConfigValue<bool>(conf, "home_mode.enable", false))
    {
        COOLWSD::MaxConnections = 20;
        COOLWSD::MaxDocuments = 10;
    }
    else
    {
        conf.setString("welcome.enable", "true");
        COOLWSD::MaxConnections = MAX_CONNECTIONS;
        COOLWSD::MaxDocuments = MAX_DOCUMENTS;
    }
#else
    {
        COOLWSD::MaxConnections = MAX_CONNECTIONS;
        COOLWSD::MaxDocuments = MAX_DOCUMENTS;
    }
#endif

    LOG_DBG("net::Defaults: Socket[inactivityTimeout " << net::Defaults.inactivityTimeout
                                                       << ", maxExtConnections "
                                                       << net::Defaults.maxExtConnections << ']');

#if !MOBILEAPP
    NoSeccomp =
        Util::isKitInProcess() || !ConfigUtil::getConfigValue<bool>(conf, "security.seccomp", true);
    NoCapsForKit = Util::isKitInProcess() ||
                   !ConfigUtil::getConfigValue<bool>(conf, "security.capabilities", true);
    AdminEnabled = ConfigUtil::getConfigValue<bool>(conf, "admin_console.enable", true);
    IndirectionServerEnabled =
        !ConfigUtil::getConfigValue<std::string>(conf, "indirection_endpoint.url", "").empty();
    GeolocationSetup =
        ConfigUtil::getConfigValue("indirection_endpoint.geolocation_setup.enable", false);

#if ENABLE_DEBUG
    if (Util::isKitInProcess())
        SingleKit = true;
#endif
#endif

    // LanguageTool configuration
    bool enableLanguageTool = ConfigUtil::getConfigValue<bool>(conf, "languagetool.enabled", false);
    setenv("LANGUAGETOOL_ENABLED", enableLanguageTool ? "true" : "false", 1);
    const std::string baseAPIUrl =
        ConfigUtil::getConfigValue<std::string>(conf, "languagetool.base_url", "");
    setenv("LANGUAGETOOL_BASEURL", baseAPIUrl.c_str(), 1);
    const std::string userName =
        ConfigUtil::getConfigValue<std::string>(conf, "languagetool.user_name", "");
    setenv("LANGUAGETOOL_USERNAME", userName.c_str(), 1);
    const std::string apiKey =
        ConfigUtil::getConfigValue<std::string>(conf, "languagetool.api_key", "");
    setenv("LANGUAGETOOL_APIKEY", apiKey.c_str(), 1);
    bool sslVerification =
        ConfigUtil::getConfigValue<bool>(conf, "languagetool.ssl_verification", true);
    setenv("LANGUAGETOOL_SSL_VERIFICATION", sslVerification ? "true" : "false", 1);
    const std::string restProtocol =
        ConfigUtil::getConfigValue<std::string>(conf, "languagetool.rest_protocol", "");
    setenv("LANGUAGETOOL_RESTPROTOCOL", restProtocol.c_str(), 1);

    // DeepL configuration
    const std::string apiURL = ConfigUtil::getConfigValue<std::string>(conf, "deepl.api_url", "");
    const std::string authKey = ConfigUtil::getConfigValue<std::string>(conf, "deepl.auth_key", "");
    setenv("DEEPL_API_URL", apiURL.c_str(), 1);
    setenv("DEEPL_AUTH_KEY", authKey.c_str(), 1);

#if !MOBILEAPP
    const std::string helpUrl = ConfigUtil::getConfigValue<std::string>(conf, "help_url", HELP_URL);
    setenv("KIT_HELP_URL", helpUrl.c_str(), 1);
#else
    // On mobile UI there should be no tunnelled dialogs. But if there are some, by mistake,
    // at least they should not have a non-working Help button.
    setenv("KIT_HELP_URL", "", 1);
#endif

    if constexpr (ConfigUtil::isSupportKeyEnabled())
    {
        const std::string supportKeyString =
            ConfigUtil::getConfigValue<std::string>(conf, "support_key", "");

        if (supportKeyString.empty())
        {
            LOG_WRN("Support key not set, please use 'coolconfig set-support-key'.");
            std::cerr << "Support key not set, please use 'coolconfig set-support-key'." << std::endl;
            COOLWSD::OverrideWatermark = "Unsupported, the support key is missing.";
        }
        else
        {
            SupportKey key(supportKeyString);

            if (!key.verify())
            {
                LOG_WRN("Invalid support key, please use 'coolconfig set-support-key'.");
                std::cerr << "Invalid support key, please use 'coolconfig set-support-key'." << std::endl;
                COOLWSD::OverrideWatermark = "Unsupported, the support key is invalid.";
            }
            else
            {
                int validDays =  key.validDaysRemaining();
                if (validDays <= 0)
                {
                    LOG_WRN("Your support key has expired, please ask for a new one, and use 'coolconfig set-support-key'.");
                    std::cerr << "Your support key has expired, please ask for a new one, and use 'coolconfig set-support-key'." << std::endl;
                    COOLWSD::OverrideWatermark = "Unsupported, the support key has expired.";
                }
                else
                {
                    LOG_INF("Your support key is valid for " << validDays << " days");
                    COOLWSD::MaxConnections = 1000;
                    COOLWSD::MaxDocuments = 200;
                    COOLWSD::OverrideWatermark.clear();
                }
            }
        }
    }

    if (COOLWSD::MaxConnections < 3)
    {
        LOG_ERR("MAX_CONNECTIONS must be at least 3");
        COOLWSD::MaxConnections = 3;
    }

    if (COOLWSD::MaxDocuments > COOLWSD::MaxConnections)
    {
        LOG_ERR("MAX_DOCUMENTS cannot be bigger than MAX_CONNECTIONS");
        COOLWSD::MaxDocuments = COOLWSD::MaxConnections;
    }

#if !WASMAPP && !defined(_WIN32)
    struct rlimit rlim;
    if (::getrlimit(RLIMIT_NOFILE, &rlim) == 0)
    {
        LOG_INF("Maximum file descriptor supported by the system: " << rlim.rlim_cur - 1);
        // 4 fds per document are used for client connection, Kit process communication, and
        // a wakeup pipe with 2 fds. 32 fds (i.e. 8 documents) are reserved.
        LOG_INF("Maximum number of open documents supported by the system: " << rlim.rlim_cur / 4 - 8);
    }
    else
        LOG_SYS("Failed to get RLIMIT_NOFILE");
#endif

    LOG_INF("Maximum concurrent open Documents limit: " << COOLWSD::MaxDocuments);
    LOG_INF("Maximum concurrent client Connections limit: " << COOLWSD::MaxConnections);

    COOLWSD::NumConnections = 0;

    // Command Tracing.
    if (ConfigUtil::getConfigValue<bool>(conf, "trace[@enable]", false))
    {
        const auto& path = ConfigUtil::getConfigValue<std::string>(conf, "trace.path", "");
        const auto recordOutgoing =
            ConfigUtil::getConfigValue<bool>(conf, "trace.outgoing.record", false);
        std::vector<std::string> filters;
        for (size_t i = 0; ; ++i)
        {
            const std::string confPath = "trace.filter.message[" + std::to_string(i) + ']';
            std::string regex = config().getString(confPath, "");
            if (!regex.empty())
            {
                filters.push_back(std::move(regex));
            }
            else if (!config().has(confPath))
            {
                break;
            }
        }

        const auto compress =
            ConfigUtil::getConfigValue<bool>(conf, "trace.path[@compress]", false);
        const auto takeSnapshot =
            ConfigUtil::getConfigValue<bool>(conf, "trace.path[@snapshot]", false);
        TraceDumper = std::make_unique<TraceFileWriter>(path, recordOutgoing, compress,
                                                        takeSnapshot, filters);
    }

    setLokitEnvironmentVariables(conf);

#if !MOBILEAPP
    SavedClipboards = std::make_unique<ClipboardCache>();

    LOG_TRC("Initialize FileServerRequestHandler");
    COOLWSD::FileRequestHandler =
        std::make_unique<FileServerRequestHandler>(COOLWSD::FileServerRoot);
#endif

    COOLWSDServer::WebServerPoll = std::make_unique<TerminatingPoll>("websrv_poll");

#if !MOBILEAPP
    net::AsyncDNS::startAsyncDNS();

    LOG_TRC("Initialize StorageConnectionManager");
    StorageConnectionManager::initialize();

    LOG_TRC("Initialize Admin");
    Admin::initialize();
#endif

    PrisonerPoll = std::make_unique<PrisonPoll>();

    COOLWSDServer::Instance = std::make_unique<COOLWSDServer>();

    LOG_TRC("Initialize StorageBase");
    StorageBase::initialize();

#if !MOBILEAPP

#ifdef __linux__
    // Check for smaps_rollup bug where rewinding and rereading gives
    // bogus doubled results
    if (FILE* fp = fopen("/proc/self/smaps_rollup", "r"))
    {
        std::size_t memoryDirty1 = ProcUtil::getPssAndDirtyFromSMaps(fp).second;
        (void)ProcUtil::getPssAndDirtyFromSMaps(fp); // interleave another rewind+read to margin
        std::size_t memoryDirty2 = ProcUtil::getPssAndDirtyFromSMaps(fp).second;
        LOG_TRC("Comparing smaps_rollup read and rewind+read: " << memoryDirty1 << " vs " << memoryDirty2);
        if (memoryDirty2 >= memoryDirty1 * 2)
        {
            // Believed to be fixed in >= v4.19, bug seen in 4.15.0 and not in 6.5.10
            // https://github.com/torvalds/linux/commit/258f669e7e88c18edbc23fe5ce00a476b924551f
            LOG_WRN("Reading smaps_rollup twice reports Private_Dirty doubled, smaps_rollup is unreliable on this kernel");
            setenv("COOL_DISABLE_SMAPS_ROLLUP", "1", true);
        }
        fclose(fp);
    }
#endif

    ServerApplication::initialize(self);

    DocProcSettings docProcSettings;
    docProcSettings.setLimitVirtMemMb(
        ConfigUtil::getConfigValue<int>("per_document.limit_virt_mem_mb", 0));
    docProcSettings.setLimitStackMemKb(
        ConfigUtil::getConfigValue<int>("per_document.limit_stack_mem_kb", 0));
    docProcSettings.setLimitFileSizeMb(
        ConfigUtil::getConfigValue<int>("per_document.limit_file_size_mb", 0));
    docProcSettings.setLimitNumberOpenFiles(
        ConfigUtil::getConfigValue<int>("per_document.limit_num_open_files", 0));

    DocCleanupSettings &docCleanupSettings = docProcSettings.getCleanupSettings();
    docCleanupSettings.setEnable(
        ConfigUtil::getConfigValue<bool>("per_document.cleanup[@enable]", true));
    docCleanupSettings.setCleanupInterval(
        ConfigUtil::getConfigValue<int>("per_document.cleanup.cleanup_interval_ms", 10000));
    docCleanupSettings.setBadBehaviorPeriod(
        ConfigUtil::getConfigValue<int>("per_document.cleanup.bad_behavior_period_secs", 60));
    docCleanupSettings.setIdleTime(
        ConfigUtil::getConfigValue<int>("per_document.cleanup.idle_time_secs", 300));
    docCleanupSettings.setLimitDirtyMem(
        ConfigUtil::getConfigValue<int>("per_document.cleanup.limit_dirty_mem_mb", 3072));
    docCleanupSettings.setLimitCpu(
        ConfigUtil::getConfigValue<int>("per_document.cleanup.limit_cpu_per", 85));
    docCleanupSettings.setLostKitGracePeriod(
        ConfigUtil::getConfigValue<int>("per_document.cleanup.lost_kit_grace_period_secs", 120));

    Admin::instance().setDefDocProcSettings(docProcSettings, false);

#else
    (void) self;
#endif
}

void COOLWSD::setLokitEnvironmentVariables(const Poco::Util::LayeredConfiguration& conf)
{
    // Allowed hosts for being external data source in the documents
    std::vector<std::string> lokAllowedHosts;
    appendAllowedHostsFrom(conf, "net.lok_allow", lokAllowedHosts);
    // For backward compatibility post_allow hosts are also allowed
    bool postAllowed = conf.getBool("net.post_allow[@allow]", false);
    if (postAllowed)
        appendAllowedHostsFrom(conf, "net.post_allow", lokAllowedHosts);
    // For backward compatibility wopi hosts are also allowed
    bool wopiAllowed = conf.getBool("storage.wopi[@allow]", false);
    if (wopiAllowed)
    {
        appendAllowedHostsFrom(conf, "storage.wopi", lokAllowedHosts);
        appendAllowedAliasGroups(conf, lokAllowedHosts);
    }

    if (lokAllowedHosts.size())
    {
        std::string allowedRegex;
        for (size_t i = 0; i < lokAllowedHosts.size(); i++)
        {
            if (RegexUtil::isRegexValid(lokAllowedHosts[i]))
                allowedRegex += (i != 0 ? "|" : "") + lokAllowedHosts[i];
            else
                LOG_ERR("Invalid regular expression for allowed host: \"" << lokAllowedHosts[i] << "\"");
        }

        setenv("KIT_HOST_ALLOWLIST", allowedRegex.c_str(), true);

#if !MOBILEAPP
        if (!ConfigUtil::getConfigValue<bool>(conf, "ssl.ssl_verification", true))
        {
            // also disable host verification for allowed hosts
            ::setenv("KIT_HOST_ALLOWLIST_EXEMPT_VERIFY_HOST", "1", true);
        }
#endif
    }

#if !MOBILEAPP
    setenv("KIT_ALLOWED_EXTREF_PATHS", "", true);
#endif
}

void COOLWSD::initializeSSL()
{
#if ENABLE_SSL
    if (!ConfigUtil::isSslEnabled())
        return;

    const std::string ssl_cert_file_path = ConfigUtil::getPathFromConfig("ssl.cert_file_path");
    LOG_INF("SSL Cert file: " << ssl_cert_file_path);

    const std::string ssl_key_file_path = ConfigUtil::getPathFromConfig("ssl.key_file_path");
    LOG_INF("SSL Key file: " << ssl_key_file_path);

    const std::string ssl_ca_file_path = ConfigUtil::getPathFromConfig("ssl.ca_file_path");
    LOG_INF("SSL CA file: " << ssl_ca_file_path);

    std::string ssl_cipher_list = config().getString("ssl.cipher_list", "");
    if (ssl_cipher_list.empty())
            ssl_cipher_list = DEFAULT_CIPHER_SET;
    LOG_INF("SSL Cipher list: " << ssl_cipher_list);

    // Initialize the non-blocking server socket SSL context.
    ssl::Manager::initializeServerContext(ssl_cert_file_path, ssl_key_file_path, ssl_ca_file_path,
                                          ssl_cipher_list, ssl::CertificateVerification::Disabled);

    if (!ssl::Manager::isServerContextInitialized())
        LOG_ERR("Failed to initialize Server SSL.");
    else
    {
        LOG_INF("Initialized Server SSL.");
        SigUtil::addActivity("initialized SSL");
    }
#else
    LOG_INF("SSL is unavailable in this build.");
#endif
}

void COOLWSD::defineOptions(Poco::Util::OptionSet& optionSet)
{
    if constexpr (Util::isMobileApp())
        return;
    ServerApplication::defineOptions(optionSet);

    optionSet.addOption(Option("help", "", "Display help information on command line arguments.")
                        .required(false)
                        .repeatable(false));

    optionSet.addOption(Option("version-hash", "", "Display product version-hash information and exit.")
                        .required(false)
                        .repeatable(false));

    optionSet.addOption(Option("version", "", "Display version and hash information.")
                        .required(false)
                        .repeatable(false));

    optionSet.addOption(Option("cleanup", "", "Cleanup jails and other temporary data and exit.")
                        .required(false)
                        .repeatable(false));

    optionSet.addOption(Option("port", "", "Port number to listen to (default: " +
                               std::to_string(DEFAULT_CLIENT_PORT_NUMBER) + "),")
                        .required(false)
                        .repeatable(false)
                        .argument("port_number"));

    if constexpr (Util::isDebugEnabled())
    {
        optionSet.addOption(Option("find-free-port", "",
                                   "Find a free port to listen on, starting from the default.")
                                .required(false)
                                .repeatable(false));
    }

    optionSet.addOption(Option("disable-ssl", "", "Disable SSL security layer.")
                        .required(false)
                        .repeatable(false));

    optionSet.addOption(Option("disable-cool-user-checking", "", "Don't check whether coolwsd is running under the user 'cool'.  NOTE: This is insecure, use only when you know what you are doing!")
                        .required(false)
                        .repeatable(false));

    optionSet.addOption(Option("override", "o", "Override any setting by providing full xmlpath=value.")
                        .required(false)
                        .repeatable(true)
                        .argument("xmlpath"));

    optionSet.addOption(Option("config-file", "", "Override configuration file path.")
                        .required(false)
                        .repeatable(false)
                        .argument("path"));

    optionSet.addOption(Option("config-dir", "", "Override extra configuration directory path.")
                        .required(false)
                        .repeatable(false)
                        .argument("path"));

    optionSet.addOption(Option("lo-template-path", "", "Override the COKit core installation directory path.")
                        .required(false)
                        .repeatable(false)
                        .argument("path"));

    optionSet.addOption(Option("unattended", "", "Unattended run, won't wait for a debugger on faulting.")
                        .required(false)
                        .repeatable(false));

    optionSet.addOption(Option("signal", "", "Send signal SIGUSR2 to parent process when server is ready to accept connections")
                        .required(false)
                        .repeatable(false));

    optionSet.addOption(Option("use-env-vars", "",
                               "Use the environment variables defined on "
                               "https://sdk.collaboraonline.com/docs/installation/"
                               "CODE_Docker_image.html#setting-the-application-configuration-"
                               "dynamically-via-environment-variables to set options. "
                               "'DONT_GEN_SSL_CERT' is forcibly enabled and 'extra_params' is "
                               "ignored even when using this option.")
                            .required(false)
                            .repeatable(false));

    if constexpr (Util::isDebugEnabled())
    {
        optionSet.addOption(Option("unitlib", "", "Unit testing library path.")
                                .required(false)
                                .repeatable(false)
                                .argument("unitlib"));

        optionSet.addOption(Option("careerspan", "", "How many seconds to run.")
                                .required(false)
                                .repeatable(false)
                                .argument("seconds"));

        optionSet.addOption(Option("singlekit", "", "Spawn one libreoffice kit.")
                                .required(false)
                                .repeatable(false));

        optionSet.addOption(
            Option("forcecaching", "",
                   "Force HTML & asset caching even in debug mode: accelerates cypress.")
                .required(false)
                .repeatable(false));
    }
}

void COOLWSD::handleOption(const std::string& optionName,
                           const std::string& value)
{
#if !MOBILEAPP
    ServerApplication::handleOption(optionName, value);

    if (optionName == "help")
    {
        displayHelp();
        Util::forcedExit(EX_OK);
    }
    else if (optionName == "version-hash")
    {
        std::cout << Util::getCoolVersionHash() << std::endl;
        Util::forcedExit(EX_OK);
    }
    else if (optionName == "version")
        ; // ignore for compatibility
    else if (optionName == "cleanup")
        CleanupOnly = true; // Flag for later as we need the config.
    else if (optionName == "port")
        ClientPortNumber = NumUtil::stoi(value);
#if ENABLE_DEBUG
    else if (optionName == "find-free-port")
        FindFreePort = true;
#endif
    else if (optionName == "disable-ssl")
        _overrideSettings["ssl.enable"] = "false";
    else if (optionName == "disable-cool-user-checking")
        CheckCoolUser = false;
    else if (optionName == "override")
    {
        std::string optName;
        std::string optValue;
        COOLProtocol::parseNameValuePair(value, optName, optValue);
        _overrideSettings[optName] = std::move(optValue);
    }
    else if (optionName == "config-file")
        ConfigFile = value;
    else if (optionName == "config-dir")
        ConfigDir = value;
    else if (optionName == "lo-template-path")
        LoTemplate = value;
    else if (optionName == "signal")
        SignalParent = true;
    else if (optionName == "use-env-vars")
        UseEnvVarOptions = true;

#if ENABLE_DEBUG
    else if (optionName == "unitlib")
        UnitTestLibrary = value;
    else if (optionName == "unattended")
    {
        UnattendedRun = true;
        SigUtil::setUnattended();
    }
    else if (optionName == "careerspan")
        careerSpanMs = std::chrono::seconds(NumUtil::stoi(value)); // Convert second to ms
    else if (optionName == "singlekit")
    {
        SingleKit = true;
        NumPreSpawnedChildren = 1;
    }
    else if (optionName == "forcecaching")
        ForceCaching = true;

    static const char* latencyMs = std::getenv("COOL_DELAY_SOCKET_MS");
    if (latencyMs)
        SimulatedLatencyMs = NumUtil::stoi(latencyMs);
#endif

#else
    (void) optionName;
    (void) value;
#endif
}

#if !MOBILEAPP

void COOLWSD::initializeEnvOptions()
{
    int n = 0;
    char* aliasGroup;
    while ((aliasGroup = std::getenv(("aliasgroup" + std::to_string(n + 1)).c_str())) != nullptr)
    {
        bool first = true;
        std::istringstream aliasGroupStream;
        aliasGroupStream.str(aliasGroup);
        int j = 0;
        for (std::string alias; std::getline(aliasGroupStream, alias, ',');)
        {
            if (first)
            {
                const std::string path = "storage.wopi.alias_groups.group[" + std::to_string(n) + "].host";
                _overrideSettings[path] = alias;
                _overrideSettings[path + "[@allow]"] = "true";
                first = false;
            }
            else
            {
                _overrideSettings["storage.wopi.alias_groups.group[" + std::to_string(n) +
                                  "].alias[" + std::to_string(j) + ']'] = alias;
                j++;
            }
        }

        n++;
    }
    if (n >= 1)
    {
        _overrideSettings["alias_groups[@mode]"] = "groups";
    }

    char* optionValue;
    if ((optionValue = std::getenv("username")) != nullptr) _overrideSettings["admin_console.username"] = optionValue;
    if ((optionValue = std::getenv("password")) != nullptr) _overrideSettings["admin_console.password"] = optionValue;
    if ((optionValue = std::getenv("server_name")) != nullptr) _overrideSettings["server_name"] = optionValue;
    if ((optionValue = std::getenv("dictionaries")) != nullptr) _overrideSettings["allowed_languages"] = optionValue;
    if ((optionValue = std::getenv("remoteconfigurl")) != nullptr) _overrideSettings["remote_config.remote_url"] = optionValue;
}

void COOLWSD::displayHelp()
{
    Poco::Util::HelpFormatter helpFormatter(options());
    helpFormatter.setCommand(commandName());
    helpFormatter.setUsage("OPTIONS");
    helpFormatter.setHeader("Collabora Online WebSocket server.");
    helpFormatter.format(std::cout);
}

bool COOLWSD::checkAndRestoreForKit()
{
// clang issues warning for WIF*() macro usages below:
// "equality comparison with extraneous parentheses [-Werror,-Wparentheses-equality]"
// https://bugs.llvm.org/show_bug.cgi?id=22949

#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wparentheses-equality"
#endif

    if (ForKitProcId == -1)
    {
        // Fire the ForKit process for the first time.
        if (!SigUtil::getShutdownRequestFlag() && !createForKit())
        {
            // Should never fail.
            LOG_FTL("Setting ShutdownRequestFlag: Failed to spawn coolforkit.");
            SigUtil::requestShutdown();
        }
    }

    if (Util::isKitInProcess())
        return true;

    int status;
    const pid_t pid = waitpid(ForKitProcId, &status, WUNTRACED | WNOHANG);
    if (pid > 0)
    {
        if (pid == ForKitProcId)
        {
            if (WIFEXITED(status) || WIFSIGNALED(status))
            {
                if (WIFEXITED(status))
                {
                    LOG_INF("Forkit process [" << pid << "] exited with code: " <<
                            WEXITSTATUS(status) << '.');
                }
                else
                {
                    LOG_ERR("Forkit process [" << pid << "] " <<
                            (WCOREDUMP(status) ? "core-dumped" : "died") <<
                            " with " << SigUtil::signalName(WTERMSIG(status)));
                }

                // subforkits exit when their parent forkit, so we can
                // immediately consider these obsolete
                for (auto it = SubForKitProcs.begin(); it != SubForKitProcs.end(); )
                {
                    LOG_DBG("dropping subforkit " << it->first);
                    it = dropSubForKit(it);
                }

                // Spawn a new forkit and try to dust it off and resume.
                if (!SigUtil::getShutdownRequestFlag() && !createForKit())
                {
                    LOG_FTL("Setting ShutdownRequestFlag: Failed to spawn forkit instance.");
                    SigUtil::requestShutdown();
                }
            }
            else if (WIFSTOPPED(status))
            {
                LOG_INF("Forkit process [" << pid << "] stopped with " <<
                        SigUtil::signalName(WSTOPSIG(status)));
            }
            else if (WIFCONTINUED(status))
            {
                LOG_INF("Forkit process [" << pid << "] resumed with SIGCONT.");
            }
            else
            {
                LOG_WRN("Unknown status returned by waitpid: " << std::hex << status << std::dec);
            }

            return true;
        }
        else
        {
            LOG_ERR("An unknown child process [" << pid << "] died.");
        }
    }
    else if (pid < 0)
    {
        LOG_SYS("Forkit waitpid failed");
        if (errno == ECHILD)
        {
            // No child processes.
            // Spawn a new forkit and try to dust it off and resume.
            if (!SigUtil::getShutdownRequestFlag()  && !createForKit())
            {
                LOG_FTL("Setting ShutdownRequestFlag: Failed to spawn forkit instance.");
                SigUtil::requestShutdown();
            }
        }

        return true;
    }

    return false;

#if defined __clang__
#pragma clang diagnostic pop
#endif
}

#endif

void COOLWSD::doHousekeeping()
{
    if (PrisonerPoll)
    {
        PrisonerPoll->wakeup();
    }
}

void COOLWSD::closeDocument(const std::string& docKey, const std::string& message)
{
    std::unique_lock<std::mutex> docBrokersLock(DocBrokersMutex);
    auto docBrokerIt = DocBrokers.find(docKey);
    if (docBrokerIt != DocBrokers.end())
    {
        std::shared_ptr<DocumentBroker> docBroker = docBrokerIt->second;
        docBroker->addCallback([docBroker, message]() {
                docBroker->closeDocument(message);
            });
    }
}

void COOLWSD::autoSave(const std::string& docKey)
{
    std::unique_lock<std::mutex> docBrokersLock(DocBrokersMutex);
    auto docBrokerIt = DocBrokers.find(docKey);
    if (docBrokerIt != DocBrokers.end())
    {
        std::shared_ptr<DocumentBroker> docBroker = docBrokerIt->second;
        docBroker->addCallback(
            [docBroker]() { docBroker->autoSave(/*force=*/true, /*dontSaveIfUnmodified=*/true); });
    }
}

void COOLWSD::setMigrationMsgReceived(const std::string& docKey)
{
    std::unique_lock<std::mutex> docBrokersLock(DocBrokersMutex);
    auto docBrokerIt = DocBrokers.find(docKey);
    if (docBrokerIt != DocBrokers.end())
    {
        std::shared_ptr<DocumentBroker> docBroker = docBrokerIt->second;
        docBroker->addCallback([docBroker]() { docBroker->setMigrationMsgReceived(); });
    }
}

void COOLWSD::setAllMigrationMsgReceived()
{
    std::unique_lock<std::mutex> docBrokersLock(DocBrokersMutex);
    for (const auto& brokerIt : DocBrokers)
    {
        std::shared_ptr<DocumentBroker> docBroker = brokerIt.second;
        docBroker->addCallback([docBroker]() { docBroker->setMigrationMsgReceived(); });
    }
}

void COOLWSD::setLogLevelsOfKits(const std::string& level)
{
    std::lock_guard<std::mutex> docBrokersLock(DocBrokersMutex);

    LOG_INF("Changing kits' log levels: [" << level << ']');

    for (const auto& brokerIt : DocBrokers)
    {
        std::shared_ptr<DocumentBroker> docBroker = brokerIt.second;
        docBroker->addCallback([docBroker, level]() {
            docBroker->setKitLogLevel(level);
        });
    }
}

/// Really do the house-keeping
void PrisonPoll::wakeupHook()
{
    if constexpr (!Util::isMobileApp())
    {
        THREAD_UNSAFE_DUMP_BEGIN
        LOG_TRC("PrisonerPoll - wakes up with " << NewChildren.size() << " new children and "
                                                << DocBrokers.size() << " brokers and "
                                                << TotalOutstandingForks << " kits forking");
        THREAD_UNSAFE_DUMP_END

        if (!COOLWSD::checkAndRestoreForKit())
        {
            // No children have died.
            // Make sure we have sufficient reserves.
            prespawnChildren();
        }
    }

    std::unique_lock<std::mutex> docBrokersLock(DocBrokersMutex, std::defer_lock);
    if (docBrokersLock.try_lock())
    {
        COOLWSD::cleanupDocBrokers();
        SigUtil::checkForwardSigUsr2(forwardSigUsr2);
    }
}

#if !MOBILEAPP

bool COOLWSD::createForKit()
{
    LOG_INF("Creating new forkit process.");

    SigUtil::addActivity("spawning new forkit");

    // Creating a new forkit is always a slow process.
    ChildSpawnTimeoutMs = std::chrono::milliseconds(CHILD_SPAWN_TIMEOUT_MS);

    std::unique_lock<std::mutex> newChildrenLock(NewChildrenMutex);

    StringVector args;
    std::string parentPath =
        Poco::Path(Poco::Util::Application::instance().commandPath()).parent().toString();

#if STRACE_COOLFORKIT
    // if you want to use this, you need to sudo setcap cap_fowner,cap_chown,cap_sys_chroot=ep /usr/bin/strace
    args.push_back("-o");
    args.push_back("strace.log");
    args.push_back("-f");
    args.push_back("-tt");
    args.push_back("-s");
    args.push_back("256");
    args.push_back(parentPath + "coolforkit-caps");
#elif VALGRIND_COOLFORKIT
    NoCapsForKit = true;
    NoSeccomp = true;
//    args.push_back("--log-file=valgrind.log");
//    args.push_back("--track-fds=all");

//  for massif: can connect with (gdb) target remote | vgdb
//  and then monitor snapshot <filename> before kit exit
//    args.push_back("--tool=massif");
//    args.push_back("--vgdb=yes");
//    args.push_back("--vgdb-error=0");

    args.push_back("--trace-children=yes");
    args.push_back("--error-limit=no");
    args.push_back("--num-callers=128");
    std::string nocapsCopy = parentPath + "coolforkit-nocaps";
    FileUtil::copy(parentPath + "coolforkit-caps", nocapsCopy, true, true);
    args.push_back(nocapsCopy);
#endif
    args.push_back("--systemplate=" + SysTemplate);
    args.push_back("--lotemplate=" + LoTemplate);
    args.push_back("--childroot=" + ChildRoot);
    args.push_back("--clientport=" + std::to_string(ClientPortNumber));
    args.push_back("--masterport=" + MasterLocation.getName());

    const DocProcSettings& docProcSettings = Admin::instance().getDefDocProcSettings();
    std::ostringstream ossRLimits;
    ossRLimits << "limit_virt_mem_mb:" << docProcSettings.getLimitVirtMemMb();
    ossRLimits << ";limit_stack_mem_kb:" << docProcSettings.getLimitStackMemKb();
    ossRLimits << ";limit_file_size_mb:" << docProcSettings.getLimitFileSizeMb();
    ossRLimits << ";limit_num_open_files:" << docProcSettings.getLimitNumberOpenFiles();
    args.push_back("--rlimits=" + ossRLimits.str());

    if (UnitWSD::get().hasKitHooks())
        args.push_back("--unitlib=" + UnitTestLibrary);

    args.push_back("--version");

    if (NoCapsForKit)
        args.push_back("--nocaps");

    if (NoSeccomp)
        args.push_back("--noseccomp");

    args.push_back("--ui=" + UserInterface);

    if (!CheckCoolUser)
        args.push_back("--disable-cool-user-checking");

    if (UnattendedRun)
        args.push_back("--unattended");

#if ENABLE_DEBUG
    if (SingleKit)
        args.push_back("--singlekit");
#endif

#if STRACE_COOLFORKIT
    std::string forKitPath = "/usr/bin/strace";
#elif VALGRIND_COOLFORKIT
    std::string forKitPath = "/usr/bin/valgrind";
#else
    std::string forKitPath = std::move(parentPath);
    if (EnableMountNamespaces || NoCapsForKit)
    {
        forKitPath += "coolforkit-ns";
        if (EnableMountNamespaces)
            args.push_back("--namespace");
    }
    else
    {
        forKitPath += "coolforkit-caps";
        if (!FileUtil::Stat(forKitPath).exists())
            LOG_FTL("coolforkit-caps does not exist, install coolwsd-deprecated package");
    }
#endif

    // Always reap first, in case we haven't done so yet.
    if (ForKitProcId != -1)
    {
        if (Util::isKitInProcess())
            return true;
        int status;
        waitpid(ForKitProcId, &status, WUNTRACED | WNOHANG);
        ForKitProcId = -1;
        Admin::instance().setForKitPid(ForKitProcId);
    }

    // Below line will be executed by PrisonerPoll thread.
    ForKitProc = nullptr;
    PrisonerPoll->setForKitProcess(ForKitProc);

    const std::string defaultConfigId;

    // ForKit always spawns one.
    ++TotalOutstandingForks;
    ++OutstandingForks[defaultConfigId];

    LOG_INF("Launching forkit process: " << forKitPath << ' ' << args.cat(' ', 0));

    LastForkRequestTimes[defaultConfigId] = std::chrono::steady_clock::now();
    int child = createForkit(forKitPath, args);
    ForKitProcId = child;

    LOG_INF("Forkit process launched: " << ForKitProcId);

    // Init the Admin manager
    Admin::instance().setForKitPid(ForKitProcId);

    const int balance = COOLWSD::NumPreSpawnedChildren - OutstandingForks[defaultConfigId];
    if (balance > 0)
        rebalanceChildren(defaultConfigId, balance);

    return ForKitProcId != -1;
}

bool COOLWSD::sendMessageToForKit(const std::string& message, const std::string& configId)
{
    if (!PrisonerPoll)
        return false;

    if (configId.empty())
    {
        PrisonerPoll->sendMessageToForKit(message);
        return true;
    }

    auto it = SubForKitProcs.find(configId);
    if (it == SubForKitProcs.end())
    {
        LOG_WRN("subforkit: " << configId << " doesn't exist yet, dropping message: " << message);
        return false;
    }
    PrisonerPoll->sendMessageToForKit(message, it->second);
    return true;
}

#endif // !MOBILEAPP

/// Handles the socket that the prisoner kit connected to WSD on.
class PrisonerRequestDispatcher final : public WebSocketHandler
{
    std::weak_ptr<ChildProcess> _childProcess;
    int _pid; ///< The Kit's PID (for logging).
    int _socketFD; ///< The socket FD to the Kit (for logging).
    bool _associatedWithDoc; ///< True when/if we get a DocBroker.

public:
    PrisonerRequestDispatcher()
        : WebSocketHandler(/* isClient = */ false, /* isMasking = */ true)
        , _pid(0)
        , _socketFD(0)
        , _associatedWithDoc(false)
    {
        LOG_TRC_S("PrisonerRequestDispatcher");
    }
    ~PrisonerRequestDispatcher()
    {
        LOG_TRC("~PrisonerRequestDispatcher");

        // Notify the broker that we're done.
        // Note: since this class is the default WebScoketHandler
        // for all incoming connections, for ForKit we have to
        // replace it (once we receive 'GET /coolws/forkit') with
        // ForKitProcWSHandler (see ForKitProcess) and nothing to disconnect.
        std::shared_ptr<ChildProcess> child = _childProcess.lock();
        if (child && child->getPid() > 0)
            onDisconnect();
    }

private:
    /// Keep our socket around ...
    void onConnect(const std::shared_ptr<StreamSocket>& socket) override
    {
        WebSocketHandler::onConnect(socket);
        LOG_TRC("Prisoner connected");
    }

    void onDisconnect() override
    {
        LOG_DBG("Prisoner connection disconnected");

        // Notify the broker that we're done.
        std::shared_ptr<ChildProcess> child = _childProcess.lock();
        std::shared_ptr<DocumentBroker> docBroker =
            child && child->getPid() > 0 ? child->getDocumentBroker() : nullptr;
        if (docBroker)
        {
            assert(child->getPid() == _pid && "Child PID changed unexpectedly");
            const bool unexpected = !docBroker->isUnloading() && !SigUtil::getShutdownRequestFlag();
            if (unexpected)
            {
                LOG_WRN("DocBroker [" << docBroker->getDocKey()
                                      << "] got disconnected from its Kit (" << child->getPid()
                                      << ") unexpectedly. Closing");
            }
            else
            {
                LOG_DBG("DocBroker [" << docBroker->getDocKey() << "] disconnected from its Kit ("
                                      << child->getPid() << ") as expected");
            }

            docBroker->disconnectedFromKit(unexpected);
        }
        else if (!_associatedWithDoc && !SigUtil::getShutdownRequestFlag())
        {
            LOG_WRN("Unassociated Kit (" << _pid << ") disconnected unexpectedly");

            std::string configId;
            std::unique_lock<std::mutex> lock(NewChildrenMutex);
            auto it = std::find(NewChildren.begin(), NewChildren.end(), child);
            if (it != NewChildren.end())
            {
                configId = (*it)->getConfigId();
                NewChildren.erase(it);
            }
            else
                LOG_WRN("Unknown Kit process closed with pid " << (child ? child->getPid() : -1));
#if !MOBILEAPP
            rebalanceChildren(configId, COOLWSD::NumPreSpawnedChildren);
#endif
        }
    }

    /// Called after successful socket reads.
    void handleIncomingMessage(SocketDisposition &disposition) override
    {
        if (_childProcess.lock())
        {
            // FIXME: inelegant etc. - derogate to websocket code
            WebSocketHandler::handleIncomingMessage(disposition);
            return;
        }

        std::shared_ptr<StreamSocket> socket = getSocket().lock();
        if (!socket)
        {
            LOG_ERR("Invalid socket while reading incoming message");
            return;
        }

        Buffer& data = socket->getInBuffer();
        if (data.empty())
        {
            LOG_DBG("No data to process from the socket");
            return;
        }

#ifdef LOG_SOCKET_DATA
        LOG_TRC("HandleIncomingMessage: buffer has:\n"
                << HexUtil::dumpHex(std::string(data.data(), std::min(data.size(), 256UL))));
#endif

        // Consume the incoming data by parsing and processing the body.
        http::RequestParser request;
#if !MOBILEAPP
        const int64_t read = request.readData(data.data(), data.size());
        if (read < 0)
        {
            LOG_ERR("Error parsing prisoner socket data");
            return;
        }

        if (read == 0)
        {
            // Not enough data.
            return;
        }

        assert(read > 0 && "Must have read some data!");

        // Remove consumed data.
        data.eraseFirst(read);
#endif

        try
        {
            std::string jailId;
            std::string configId;
            std::map<std::string, std::string> admsProps;
#if !MOBILEAPP
            LOG_TRC("Child connection with URI [" << COOLWSD::anonymizeUrl(request.getUrl())
                                                  << ']');
            Poco::URI requestURI(request.getUrl());
            if (requestURI.getPath() == FORKIT_URI)
            {
                // New ForKit is spawned.
                const Poco::URI::QueryParameters params = requestURI.getQueryParameters();
                const int pid = socket->getPid();
                for (const auto& param : params)
                {
                    if (param.first == "configid")
                        configId = param.second;
                }

                if (configId.empty()) // primordial forkit
                {
                    if (pid != COOLWSD::ForKitProcId)
                    {
                        LOG_WRN("Connection request received on "
                                << FORKIT_URI << " endpoint from unexpected ForKit process. Skipped");
                        return;
                    }
                    COOLWSD::ForKitProc = std::make_shared<ForKitProcess>(COOLWSD::ForKitProcId, socket, request);
                    LOG_ASSERT_MSG(socket->getInBuffer().empty(), "Unexpected data in prisoner socket");
                    socket->getInBuffer().clear();
                    PrisonerPoll->setForKitProcess(COOLWSD::ForKitProc);
                }
                else
                {
                    LOG_INF("subforkit [" << configId << "], seen as created.");
                    SubForKitProcs[configId] = std::make_shared<ForKitProcess>(pid, socket, request);
                    LOG_ASSERT_MSG(socket->getInBuffer().empty(), "Unexpected data in prisoner socket");
                    socket->getInBuffer().clear();
                    // created subforkit for a reason, create spare early
                    std::unique_lock<std::mutex> lock(NewChildrenMutex);
                    rebalanceChildren(configId, COOLWSD::NumPreSpawnedChildren);

                    UNITWSD_CALL(newSubForKit(SubForKitProcs[configId], configId));
                }

                return;
            }
            if (requestURI.getPath() != NEW_CHILD_URI)
            {
                LOG_ERR("Invalid incoming child URI [" << requestURI.getPath() << ']');
                return;
            }

            const auto duration = (std::chrono::steady_clock::now() - LastForkRequestTimes[configId]);
            const auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
            LOG_TRC("New child spawned after " << durationMs << " of requesting");

            // New Child is spawned.
            const Poco::URI::QueryParameters params = requestURI.getQueryParameters();
            const int pid = socket->getPid();
            for (const auto& param : params)
            {
                if (param.first == "jailid")
                    jailId = param.second;
                else if (param.first == "configid")
                    configId = param.second;
                else if (param.first == "version")
                {
                    COOLWSD::LOKitVersion = param.second;
                    Poco::JSON::Object::Ptr object;
                    if (JsonUtil::parseJSON(COOLWSD::LOKitVersion, object))
                    {
                        COOLWSD::LOKitVersionNumber = JsonUtil::getJSONValue<std::string>(object, "ProductVersion") + JsonUtil::getJSONValue<std::string>(object, "ProductExtension");
                        COOLWSD::LOKitVersionHash = JsonUtil::getJSONValue<std::string>(object, "BuildId").substr(0, 8);
                    }
                }
                else if (param.first.size() > 6 &&
                         param.first.compare(0, 5, "adms_") == 0)
                    admsProps[param.first.substr(5)] = param.second;
            }

            if (pid <= 0)
            {
                LOG_ERR("Invalid PID in child URI [" << COOLWSD::anonymizeUrl(request.getUrl())
                                                     << ']');
                return;
            }

            if (jailId.empty())
            {
                LOG_ERR("Invalid JailId in child URI [" << COOLWSD::anonymizeUrl(request.getUrl())
                                                        << ']');
                return;
            }

            LOG_ASSERT_MSG(socket->getInBuffer().empty(), "Unexpected data in prisoner socket");
            socket->getInBuffer().clear();

            LOG_INF("New child [" << pid << "], jailId: " << jailId << ", configId: " << configId);
#else
            pid_t pid = 100;
            jailId = "jail";
            socket->getInBuffer().clear();
#endif
            LOG_TRC("Calling make_shared<ChildProcess>, for NewChildren?");

            auto child = std::make_shared<ChildProcess>(pid, jailId, configId, socket, request, admsProps);

            if constexpr (!Util::isMobileApp())
                UNITWSD_CALL(newChild(child));

            _pid = pid;
            _socketFD = socket->getFD();
#if !MOBILEAPP
            child->setSMapsFD(socket->getIncomingFD(SharedFDType::SMAPS));
#endif
            _childProcess = child; // weak

            addNewChild(std::move(child));
        }
        catch (const std::bad_weak_ptr&)
        {
            // Using shared_from_this() from a constructor is not good.
            assert(!"Got std::bad_weak_ptr. Are we using shared_from_this() from a constructor?");
        }
        catch (const std::exception& exc)
        {
            // Probably don't have enough data just yet.
            // TODO: timeout if we never get enough.
        }
    }

    /// Prisoner websocket fun ... (for now)
    virtual void handleMessage(const std::vector<char> &data) override
    {
        if (UnitWSD::isUnitTesting() && UnitWSD::get().filterChildMessage(data))
            return;

        auto message = std::make_shared<Message>(data.data(), data.size(), Message::Dir::Out);
        std::shared_ptr<StreamSocket> socket = getSocket().lock();
        if (socket)
        {
            assert(socket->getFD() == _socketFD && "Socket FD changed unexpectedly");
            LOG_TRC("Prisoner message [" << message->abbr() << ']');
        }
        else
            LOG_WRN("Message handler called but without valid socket. Expected #" << _socketFD);

        std::shared_ptr<ChildProcess> child = _childProcess.lock();
        std::shared_ptr<DocumentBroker> docBroker =
            child && child->getPid() > 0 ? child->getDocumentBroker() : nullptr;
        if (docBroker)
        {
            assert(child->getPid() == _pid && "Child PID changed unexpectedly");
            _associatedWithDoc = true;
            docBroker->handleInput(message);
        }
        else if (child && child->getPid() > 0)
        {
            const std::string abbreviatedMessage = COOLWSD::AnonymizeUserData ? "..." : message->abbr();
            LOG_WRN("Child " << child->getPid() << " has no DocBroker to handle message: ["
                             << abbreviatedMessage << ']');
        }
        else
        {
            const std::string abbreviatedMessage = COOLWSD::AnonymizeUserData ? "..." : message->abbr();
            LOG_ERR("Cannot handle message with unassociated Kit (PID " << _pid << "): ["
                                                                        << abbreviatedMessage);
        }
    }

    int getPollEvents(std::chrono::steady_clock::time_point /* now */,
                      int64_t & /* timeoutMaxMs */) override
    {
        return POLLIN;
    }

    void performWrites(std::size_t /*capacity*/) override {}
};

class PlainSocketFactory final : public SocketFactory
{
    std::shared_ptr<Socket> create(const int physicalFd, Socket::Type type) override
    {
        int fd = physicalFd;
#if !MOBILEAPP
        if (SimulatedLatencyMs > 0)
        {
            int delayfd = Delay::create(SimulatedLatencyMs, physicalFd);
            if (delayfd == -1)
                LOG_ERR("DelaySocket creation failed, using physicalFd " << physicalFd << " instead.");
            else
                fd = delayfd;
        }
#endif
        return StreamSocket::create<StreamSocket>(
            std::string(), fd, type, false, HostType::Other,
            std::make_shared<ClientRequestDispatcher>());
    }
};

#if ENABLE_SSL
class SslSocketFactory final : public SocketFactory
{
    std::shared_ptr<Socket> create(const int physicalFd, Socket::Type type) override
    {
        int fd = physicalFd;

#if !MOBILEAPP
        if (SimulatedLatencyMs > 0)
        {
            int delayFd = Delay::create(SimulatedLatencyMs, physicalFd);
            if (delayFd == -1)
                LOG_ERR("Delay creation failed, fallback to original fd");
            else
                fd = delayFd;
        }
#endif

        return StreamSocket::create<SslStreamSocket>(std::string(), fd, type, false, HostType::Other,
                                                     std::make_shared<ClientRequestDispatcher>());
    }
};
#endif

class PrisonerSocketFactory final : public SocketFactory
{
    std::shared_ptr<Socket> create(const int fd, Socket::Type type) override
    {
        // No local delay.
        return StreamSocket::create<StreamSocket>(std::string(), fd, type, false, HostType::Other,
                                                  std::make_shared<PrisonerRequestDispatcher>(),
                                                  StreamSocket::ReadType::UseRecvmsgExpectFD);
    }
};

COOLWSDServer::COOLWSDServer()
    : _acceptPoll("accept_poll")
#if !MOBILEAPP
    , _admin(Admin::instance())
#endif
{
}

COOLWSDServer::~COOLWSDServer()
{
    stop();
}

std::shared_ptr<ServerSocket> COOLWSDServer::findClientPort()
{
    return findServerPort();
}

void COOLWSDServer::startPrisoners()
{
    PrisonerPoll->startThread();
    PrisonerPoll->insertNewSocket(findPrisonerServerPort());
}

// static
void COOLWSDServer::stopPrisoners()
{
    PrisonerPoll->joinThread();
}

void COOLWSDServer::start(std::shared_ptr<ServerSocket>&& serverSocket)
{
#if MOBILEAPP
    coolwsd_server_socket_fd = serverSocket->getFD();
#endif

    _acceptPoll.startThread();
    _acceptPoll.insertNewSocket(std::move(serverSocket));

    WebServerPoll->startThread();

#if !MOBILEAPP
    _admin.start();
#endif
}

void COOLWSDServer::stop()
{
    _acceptPoll.joinThread();
    if (WebServerPoll)
        WebServerPoll->joinThread();
#if !MOBILEAPP
    _admin.stop();
#endif
}

void COOLWSDServer::dumpState(std::ostream& os) const
{
    // FIXME: add some stop-world magic before doing the dump(?)
    ThreadChecks::Inhibit = true;

    std::string version, hash;
    Util::getVersionInfo(version, hash);

    THREAD_UNSAFE_DUMP_BEGIN
    os << "COOLWSDServer: " << version << " - " << hash << " state dumping"
#if !MOBILEAPP
       << "\n  Kit version: " << COOLWSD::LOKitVersion << "\n  Ports: server " << ClientPortNumber
       << " prisoner " << MasterLocation
       << "\n  SSL: " << (ConfigUtil::isSslEnabled() ? "https" : "http")
       << "\n  SSL-Termination: " << (ConfigUtil::isSSLTermination() ? "yes" : "no")
       << "\n  Security " << (COOLWSD::NoCapsForKit ? "no" : "") << " chroot, "
       << (COOLWSD::NoSeccomp ? "no" : "") << " api lockdown"
       << "\n  Admin: " << (COOLWSD::AdminEnabled ? "enabled" : "disabled")
       << "\n  RouteToken: " << COOLWSD::RouteToken
#endif
       << "\n  Uptime (seconds): "
       << std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() -
                                                           COOLWSD::StartTime)
              .count()
       << "\n  TerminationFlag: " << SigUtil::getTerminationFlag()
       << "\n  isShuttingDown: " << SigUtil::getShutdownRequestFlag()
       << "\n  NewChildren: " << NewChildren.size() << " (" << NewChildren.capacity() << ')'
       << "\n  OutstandingForks: " << TotalOutstandingForks
       << "\n  NumPreSpawnedChildren: " << COOLWSD::NumPreSpawnedChildren
       << "\n  ChildSpawnTimeoutMs: " << ChildSpawnTimeoutMs.load()
#if !MOBILEAPP
       << "\n  of which ConvertTo: " << ConvertToBroker::getInstanceCount()
#endif
       << "\n  vs. MaxDocuments: " << COOLWSD::MaxDocuments
       << "\n  NumConnections: " << COOLWSD::NumConnections
       << "\n  vs. MaxConnections: " << COOLWSD::MaxConnections
       << "\n  SysTemplate: " << COOLWSD::SysTemplate << "\n  LoTemplate: " << COOLWSD::LoTemplate
       << "\n  ChildRoot: " << COOLWSD::ChildRoot
       << "\n  FileServerRoot: " << COOLWSD::FileServerRoot
       << "\n  ServiceRoot: " << COOLWSD::ServiceRoot
       << "\n  LOKitVersion: " << COOLWSD::LOKitVersion
       << "\n  HostIdentifier: " << Util::getProcessIdentifier()
       << "\n  ConfigFile: " << COOLWSD::ConfigFile << "\n  ConfigDir: " << COOLWSD::ConfigDir
       << "\n  LogLevel: " << COOLWSD::LogLevel
       << "\n  LogDisabledAreas: " << COOLWSD::LogDisabledAreas
       << "\n  AnonymizeUserData: " << (COOLWSD::AnonymizeUserData ? "yes" : "no")
       << "\n  CheckCoolUser: " << (COOLWSD::CheckCoolUser ? "yes" : "no")
       << "\n  IsProxyPrefixEnabled: " << (COOLWSD::IsProxyPrefixEnabled ? "yes" : "no")
       << "\n  OverrideWatermark: " << COOLWSD::OverrideWatermark
       << "\n  UserInterface: " << COOLWSD::UserInterface
       << "\n  Total PSS: " << ProcUtil::getProcessTreePss(ProcUtil::getProcessId()) << " KB"
       << "\n  Config: " << LoggableConfigEntries;
    THREAD_UNSAFE_DUMP_END

    std::string smap;
    if (const ssize_t size = FileUtil::readFile("/proc/self/smaps_rollup", smap); size <= 0)
        os << "\n  smaps_rollup: <unavailable>";
    else
        os << "\n  smaps_rollup: " << Util::replace(std::move(smap), "\n", "\n\t");

#if !MOBILEAPP
    if (FetchHttpSession)
    {
        os << "\nFetchHttpSession:\n";
        FetchHttpSession->dumpState(os, "\n  ");
    }
    else
#endif // !MOBILEAPP
    {
        os << "\nFetchHttpSession: null\n";
    }

    os << "\nServer poll:\n";
    _acceptPoll.dumpState(os);

    os << "\nWeb Server poll:\n";
    WebServerPoll->dumpState(os);

    os << "\nPrisoner poll:\n";
    PrisonerPoll->dumpState(os);

#if !MOBILEAPP
    _admin.dumpMetrics(); // Dump the state from the Admin poll thread.

    // If we have any delaying work going on.
    os << '\n';
    Delay::dumpState(os);

    // If we have any DNS work going on.
    os << '\n';
    net::AsyncDNS::dumpState(os);

    os << '\n';
    COOLWSD::SavedClipboards->dumpState(os);

    os << '\n';
    COOLWSD::FileRequestHandler->dumpState(os);
#endif

    {
        std::lock_guard<std::mutex> docBrokerLock(DocBrokersMutex);
        os << "\nDocument Broker polls " << "[ " << DocBrokers.size() << " ]:\n";
        for (const auto& i : DocBrokers)
            i.second->dumpState(os);
    }

#if !MOBILEAPP
    os << "\nConverter count: " << ConvertToBroker::getInstanceCount() << '\n';
#endif

    os << "\nDone COOLWSDServer state dumping.\n";

    ThreadChecks::Inhibit = false;
}

/// Create the internal only, local socket for forkit / kits prisoners to talk to.
std::shared_ptr<ServerSocket> COOLWSDServer::findPrisonerServerPort()
{
    std::shared_ptr<SocketFactory> factory = std::make_shared<PrisonerSocketFactory>();
#if !MOBILEAPP
    auto socket = std::make_shared<LocalServerSocket>(
                    std::chrono::steady_clock::now(), *PrisonerPoll, factory);

    const UnxSocketPath location = socket->bind();
    if (!location.isValid())
    {
        LOG_FTL("Failed to create local unix domain socket. Exiting.");
        Util::forcedExit(EX_SOFTWARE);
        return nullptr;
    }

    if (!socket->listen())
    {
        LOG_FTL("Failed to listen on local unix domain socket at " << location << ". Exiting.");
        Util::forcedExit(EX_SOFTWARE);
    }

    LOG_INF("Listening to prisoner connections on " << location);
    MasterLocation = std::move(location);
#if ENABLE_CHILDROOTS
    if(!socket->linkTo(COOLWSD::SysTemplate))
    {
        LOG_FTL("Failed to hardlink local unix domain socket into a jail. Exiting.");
        Util::forcedExit(EX_SOFTWARE);
    }
#endif
#else
    constexpr int UNUSED_PORT_NUMBER = 0;
    std::shared_ptr<ServerSocket> socket
        = ServerSocket::create(ServerSocket::Type::Public, UNUSED_PORT_NUMBER,
                               ClientPortProto, std::chrono::steady_clock::now(), *PrisonerPoll, factory);

    PrisonerServerSocketFD = socket->getFD();
    LOG_INF("Listening to prisoner connections on #" << PrisonerServerSocketFD);
#endif
    return socket;
}

/// Create the externally listening public socket
std::shared_ptr<ServerSocket> COOLWSDServer::findServerPort()
{
    std::shared_ptr<SocketFactory> factory;
    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();

    if (ClientPortNumber <= 0)
    {
        // Avoid using the default port for unit-tests altogether.
        // This avoids interfering with a running test instance.
        ClientPortNumber = DEFAULT_CLIENT_PORT_NUMBER + (UnitWSD::isUnitTesting() ? 1 : 0);
    }

#if ENABLE_SSL
    if (ConfigUtil::isSslEnabled())
        factory = std::make_shared<SslSocketFactory>();
    else
#endif
        factory = std::make_shared<PlainSocketFactory>();

    std::shared_ptr<ServerSocket> socket = ServerSocket::create(
        ClientListenAddr, ClientPortNumber, ClientPortProto, now, *WebServerPoll, factory);

#if !MOBILEAPP
    const int firstPortNumber = ClientPortNumber;
#endif
    while (!socket &&
#ifdef BUILDING_TESTS
           true
#else
           (UnitWSD::isUnitTesting()
#if ENABLE_DEBUG
            || COOLWSD::FindFreePort
#endif
           )
#endif
        )
    {
        ++ClientPortNumber;
        LOG_INF("Client port " << (ClientPortNumber - 1) << " is busy, trying "
                               << ClientPortNumber);
        socket = ServerSocket::create(ClientListenAddr, ClientPortNumber, ClientPortProto,
                                      now, *WebServerPoll, factory);
    }

#if !MOBILEAPP
    if (!socket)
    {
        LOG_FTL("Failed to listen on Server port(s) (" << firstPortNumber << '-'
                                                       << ClientPortNumber << "). Exiting");
        Util::forcedExit(EX_SOFTWARE);
    }

    LOG_INF('#' << socket->getFD() << " Listening to client connections on port "
                << ClientPortNumber);
#else
    LOG_INF("Listening to client connections on #" << socket->getFD());
#endif
    return socket;
}

void COOLWSDServer::AcceptPoll::wakeupHook()
{
    SigUtil::checkDumpGlobalState(dump_state);
}

#if !MOBILEAPP
void COOLWSD::processFetchUpdate(const std::shared_ptr<SocketPoll>& poll)
{
    try
    {
        const std::string url(INFOBAR_URL);
        if (url.empty())
            return; // No url, nothing to do.

        if (FetchHttpSession)
            return;

        Poco::URI uriFetch(url);
        uriFetch.addQueryParameter("product", ConfigUtil::getString("product_name", APP_NAME));
        uriFetch.addQueryParameter("version", Util::getCoolVersion());
        LOG_TRC("Infobar update request from " << uriFetch.toString());
        FetchHttpSession = StorageConnectionManager::getHttpSession(uriFetch);
        if (!FetchHttpSession)
            return;

        http::Request request(uriFetch.getPathAndQuery());
        request.add("Accept", "application/json");

        FetchHttpSession->setFinishedHandler([](const std::shared_ptr<http::Session>& httpSession) {

            std::shared_ptr<http::Response> httpResponse = httpSession->response();

            FetchHttpSession.reset();
            if (httpResponse->statusLine().statusCode() == http::StatusCode::OK)
            {
                LOG_DBG("Infobar update returned: " << httpResponse->getBody());

                std::lock_guard<std::mutex> lock(COOLWSD::FetchUpdateMutex);
                COOLWSD::LatestVersion = httpResponse->getBody();
            }
            else
                LOG_WRN("Failed to update the infobar. Got: "
                        << httpResponse->statusLine().statusCode() << ' '
                        << httpResponse->statusLine().reasonPhrase());
        });

        FetchHttpSession->asyncRequest(request, poll);
    }
    catch(const Poco::Exception& exc)
    {
        LOG_DBG("FetchUpdate: " << exc.displayText());
    }
    catch(std::exception& exc)
    {
        LOG_DBG("FetchUpdate: " << exc.what());
    }
    catch(...)
    {
        LOG_DBG("FetchUpdate: Unknown exception");
    }
}

#if ENABLE_DEBUG
std::string COOLWSD::getServerURL()
{
    return getServiceURI("");
}
#endif
#endif

void COOLWSD::innerMain()
{
#if !MOBILEAPP
#  ifdef __linux__
    // down-pay all the forkit linking cost once & early.
    setenv("LD_BIND_NOW", "1", 1);
#  endif

    std::string version, hash;
    Util::getVersionInfo(version, hash);
    LOG_INF("Coolwsd version details: " << version << " - " << hash << " - id " << Util::getProcessIdentifier() << " - on " << Util::getLinuxVersion());
#endif
    SigUtil::addActivity("coolwsd init");

    initializeSSL();

#if !MOBILEAPP
    // Fetch remote settings from server if configured
    std::shared_ptr<RemoteConfigPoll> remoteConfigThread(std::make_shared<RemoteConfigPoll>(config()));
    remoteConfigThread->start();
#endif

#if !defined(IOS) && !defined(MACOS) && !defined(_WIN32) && !defined(QTAPP)
    // Force a uniform UTF-8 locale for ourselves & our children.
    char* locale = std::setlocale(LC_ALL, "C.UTF-8");
    if (!locale)
    {
        // rhbz#1590680 - C.UTF-8 is unsupported on RH7
        LOG_WRN("Could not set locale to C.UTF-8, will try en_US.UTF-8");
        locale = std::setlocale(LC_ALL, "en_US.UTF-8");
        if (!locale)
            LOG_WRN("Could not set locale to en_US.UTF-8. Without UTF-8 support documents with non-ASCII file names cannot be opened.");
    }
    if (locale)
    {
        LOG_INF("Locale is set to " << std::string(locale));
        ::setenv("LC_ALL", locale, 1);
    }
#endif // !IOS && !MACOS && !_WIN32 && !QTAPP

#if !MOBILEAPP
    // We use the same option set for both parent and child coolwsd,
    // so must check options required in the parent (but not in the
    // child) separately now. Also check for options that are
    // meaningless for the parent.
    if (LoTemplate.empty())
    {
        LOG_FTL("Missing --lo-template-path option");
        throw Poco::Util::MissingOptionException("lotemplate");
    }

    if (FileServerRoot.empty())
        FileServerRoot = Util::getApplicationPath();
    FileServerRoot = Poco::Path(FileServerRoot).absolute().toString();
    LOG_DBG("FileServerRoot: " << FileServerRoot);

    LOG_DBG("Initializing DelaySocket with " << SimulatedLatencyMs << "ms.");
    Delay delay(SimulatedLatencyMs);

    const auto fetchUpdateCheck = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::hours(std::max(ConfigUtil::getConfigValue<int>("fetch_update_check", 10), 0)));

#endif

    ClientRequestDispatcher::InitStaticFileContentCache();

    // Allocate our port - passed to prisoners.
    assert(COOLWSDServer::Instance && "The COOLWSDServer instance does not exist.");
    // allocate port & hold temporarily.
    std::shared_ptr<ServerSocket> serverPort = COOLWSDServer::Instance->findClientPort();

#if !MOBILEAPP
    TmpFontDir = ChildRoot + JailUtil::CHILDROOT_TMP_INCOMING_PATH;
#endif

    // Start the internal prisoner server and spawn forkit,
    // which in turn forks first child.
    COOLWSDServer::Instance->startPrisoners();

// No need to "have at least one child" beforehand on mobile
#if !MOBILEAPP

    if (!Util::isKitInProcess())
    {
        // Make sure we have at least one child before moving forward.
        std::unique_lock<std::mutex> lock(NewChildrenMutex);
        // If we are debugging, it's not uncommon to wait for several minutes before first
        // child is born. Don't use an expiry timeout in that case.
        const bool debugging = std::getenv("SLEEPFORDEBUGGER") || std::getenv("SLEEPKITFORDEBUGGER");
        if (debugging)
        {
            LOG_DBG("Waiting for new child without timeout.");
            NewChildrenCV.wait(lock, []() { return !NewChildren.empty(); });
        }
        else
        {
            int retry = (COOLWSD::NoCapsForKit ? 150 : 50);
            const auto timeout = ChildSpawnTimeoutMs.load();
            while (retry-- > 0 && !SigUtil::getShutdownRequestFlag())
            {
                LOG_INF("Waiting for a new child for a max of " << timeout);
                if (NewChildrenCV.wait_for(lock, timeout, []() { return !NewChildren.empty(); }))
                {
                    break;
                }
            }
        }

        // Check we have at least one.
        LOG_TRC("Have " << NewChildren.size() << " new children.");
        if (NewChildren.empty())
        {
            if (SigUtil::getShutdownRequestFlag())
                LOG_FTL("Shutdown requested while starting up. Exiting.");
            else
                LOG_FTL("No child process could be created. Exiting.");
            Util::forcedExit(EX_SOFTWARE);
        }

        assert(NewChildren.size() > 0);
    }

    if (LogLevel != "trace")
    {
        LOG_INF("WSD initialization complete: setting log-level to [" << LogLevel << "] as configured.");
        Log::setLevel(LogLevel);
    }
    Log::setDisabledAreas(LogDisabledAreas);

    if (Log::getLevel() >= Log::Level::INF)
        LOG_ERR("Log level is set very high to '" << LogLevel << "' this will have a "
                "significant performance impact. Do not use this in production.");

    // Start the remote font downloading polling thread.
    std::unique_ptr<RemoteFontConfigPoll> remoteFontConfigThread;
    try
    {
        // Fetch font settings from server if configured
        remoteFontConfigThread = std::make_unique<RemoteFontConfigPoll>(config());
        remoteFontConfigThread->start();
    }
    catch (const Poco::Exception&)
    {
        LOG_DBG("No remote_font_config");
    }

#elif defined __EMSCRIPTEN__

    // Hard-code a somewhat random log level:
    Log::setLevel("information");

#endif

    // URI with /contents are public and we don't need to anonymize them.
    Anonymizer::mapAnonymized("contents", "contents");

    // Start the server.
    COOLWSDServer::Instance->start(std::move(serverPort));

#if WASMAPP
    // It is not at all obvious that this is the ideal place to do the HULLO thing and call onopen
    // on TheFakeWebSocket. But it seems to work.
    handle_cool_message("HULLO");
    MAIN_THREAD_EM_ASM(globalThis.TheFakeWebSocket.onopen());
#endif

    /// The main-poll does next to nothing:
    std::shared_ptr<SocketPoll> mainWait = std::make_shared<SocketPoll>("main");
    mainWait->runOnClientThread();

    SigUtil::addActivity("coolwsd accepting connections");

#if !MOBILEAPP
    std::cerr << "Ready to accept connections on port " << ClientPortNumber <<  ".\n" << std::endl;
    if (SignalParent)
    {
        kill(getppid(), SIGUSR2);
    }
#endif

#if !MOBILEAPP && ENABLE_DEBUG
    const std::string postMessageFilePath = Uri::encode(DEBUG_ABSSRCDIR "/test/samples/writer-edit.fodt");
    const std::string postMessageURI =
        getServiceURI("/browser/dist/framed.doc.html?file_path=" + postMessageFilePath);
    std::ostringstream oss;
    std::ostringstream ossRO;
    oss << "\nLaunch one of these in your browser:\n\n"
        << "Edit mode:" << '\n';

    auto names = FileUtil::getDirEntries(DEBUG_ABSSRCDIR "/test/samples");
    for (const auto &i : names)
    {
        if (i.find("-edit") != std::string::npos)
        {
            std::string padded(i);
            constexpr int width = 22;
            if (padded.size() < width)
            {
                padded.insert(padded.size(), width - padded.size(), ' ');
            }
            oss   << "    " << padded << getLaunchURI(std::string("test/samples/") + i) << "\n";
            ossRO << "    " << padded << getLaunchURI(std::string("test/samples/") + i, true) << "\n";
        }
    }

    oss << "\nReadonly mode:" << '\n'
        << ossRO.str()
        << "\npostMessage: " << postMessageURI << std::endl;

    const std::string adminURI = getServiceURI(COOLWSD_TEST_ADMIN_CONSOLE, true);
    if (!adminURI.empty())
        oss << "\nOr for the admin, monitoring, capabilities & discovery:\n\n"
            << adminURI << '\n'
            << getServiceURI(COOLWSD_TEST_METRICS, true) << '\n'
            << getServiceURI("/hosting/capabilities") << '\n'
            << getServiceURI("/hosting/discovery") << '\n';

    oss << std::endl;
    std::cerr << oss.str();
#endif

    const auto startStamp = std::chrono::steady_clock::now();
#if !MOBILEAPP
    auto stampFetch = startStamp - (fetchUpdateCheck - std::chrono::milliseconds(60000));

#ifdef __linux__
    if (ConfigUtil::getConfigValue<bool>("stop_on_config_change", false))
    {
        std::shared_ptr<InotifySocket> inotifySocket = std::make_shared<InotifySocket>(startStamp);
        mainWait->insertNewSocket(inotifySocket);
    }
#endif

    LOG_DBG("trimming memory post startup");
    Util::trimMalloc();
    time_t prevTrimTrigger = 0;
#endif

    SigUtil::addActivity("coolwsd running");

    while (!SigUtil::getShutdownRequestFlag())
    {
        // This timeout affects the recovery time of prespawned children.
        std::chrono::microseconds waitMicroS = SocketPoll::DefaultPollTimeoutMicroS * 4;

        if (UnitWSD::isUnitTesting() && !SigUtil::getShutdownRequestFlag())
        {
            UNITWSD_CALL(invokeTest());

            // More frequent polling while testing, to reduce total test time.
            waitMicroS =
                std::min(UnitWSD::get().getTimeoutMilliSeconds(), std::chrono::milliseconds(1000));
            waitMicroS /= 4;
        }

        mainWait->poll(waitMicroS);

        // Wake the prisoner poll to spawn some children, if necessary.
        PrisonerPoll->wakeup();

        if (SigUtil::getShutdownRequestFlag())
        {
            // The code below can be unsafe as other threads start shutting down.
            break;
        }

        const auto timeNow = std::chrono::steady_clock::now();
        const std::chrono::milliseconds timeSinceStartMs
            = std::chrono::duration_cast<std::chrono::milliseconds>(timeNow - startStamp);
        // Unit test timeout
        if (UnitWSD::isUnitTesting() && !SigUtil::getShutdownRequestFlag())
        {
            if (auto const unit = UnitWSD::getMaybeNull()) {
                unit->checkTimeout(timeSinceStartMs);
            }
        }

#if !MOBILEAPP
        SavedClipboards->checkexpiry(timeNow);

        const std::chrono::milliseconds durationFetch
            = std::chrono::duration_cast<std::chrono::milliseconds>(timeNow - stampFetch);
        if (fetchUpdateCheck > std::chrono::milliseconds::zero() && durationFetch > fetchUpdateCheck)
        {
            processFetchUpdate(mainWait);
            stampFetch = timeNow;
        }

        // if Admin hasn't seen any document activity for over 10 mins them malloc_trim
        constexpr time_t idleTrimCheck(10 * 60);

        const time_t lastAdminActivity = Admin::instance().getLastActivityTime();
        if (lastAdminActivity != prevTrimTrigger)
        {
            const time_t adminIdle = time(nullptr) - lastAdminActivity;
            if (adminIdle > idleTrimCheck)
            {
                LOG_DBG("trimming memory on idle");
                Util::trimMalloc();
                // Don't bother repeating until LastActivityTime changes.
                prevTrimTrigger = lastAdminActivity;
            }
        }
#endif

#if ENABLE_DEBUG && !MOBILEAPP
        if (careerSpanMs > std::chrono::milliseconds::zero() && timeSinceStartMs > careerSpanMs)
        {
            LOG_INF("Setting ShutdownRequestFlag: " << timeSinceStartMs << " gone, career of "
                                                    << careerSpanMs << " expired.");
            SigUtil::requestShutdown();
        }
#endif
    }

#if !defined(IOS) // SigUtil::getShutdownRequestFlag() always returns false on iOS, thus the above while
            // loop never exits.

    COOLWSD::alertAllUsersInternal("close: shuttingdown");

    SigUtil::addActivity("shutting down");

#if !MOBILEAPP
    // Lots of polls will stop; stop watching them first.
    SocketPoll::PollWatchdog.reset();
#endif

    // Stop the listening to new connections
    // and wait until sockets close.
    LOG_INF("Stopping server socket listening. ShutdownRequestFlag: " <<
            SigUtil::getShutdownRequestFlag() << ", TerminationFlag: " << SigUtil::getTerminationFlag());

    if (!UnitWSD::isUnitTesting())
    {
        // When running unit-tests the listening port will
        // get recycled and another test will be listening.
        // This is very problematic if a DocBroker here is
        // saving and uploading before shutting down, because
        // the test that gets the same port will receive this
        // unexpected upload and fail.

        // Otherwise, in production, we should probably respond
        // with some error that we are recycling. But for now,
        // don't change the behavior and stop listening.
        COOLWSDServer::Instance->stop();
    }

    // atexit handlers tend to free Admin before Documents
    LOG_INF("Exiting. Cleaning up lingering documents.");

#if !MOBILEAPP
    if (remoteFontConfigThread)
    {
        LOG_DBG("Stopping remote font config thread");
        remoteFontConfigThread->stop();
    }
#endif

    if (!SigUtil::getShutdownRequestFlag())
    {
        // This shouldn't happen, but it's fail safe to always cleanup properly.
        LOG_WRN("Setting ShutdownRequestFlag: Exiting WSD without ShutdownRequestFlag. Setting it "
                "now.");
        SigUtil::requestShutdown();
    }

    SigUtil::addActivity("wait save & close");

    // Wait until documents are saved and sessions closed.
    // Don't stop the DocBroker, they will exit.
    constexpr size_t sleepMs = 200;
    constexpr size_t count = (COMMAND_TIMEOUT_MS * 6) / sleepMs;
    for (size_t i = 0; i < count; ++i)
    {
        std::unique_lock<std::mutex> docBrokersLock(DocBrokersMutex);
        if (DocBrokers.empty())
            break;

        LOG_DBG("Waiting for " << DocBrokers.size() << " documents to stop.");
        cleanupDocBrokers();
        docBrokersLock.unlock();

        // Give them time to save and cleanup.
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepMs));
    }

    if (UnitWSD::isUnitTesting() && !SigUtil::getTerminationFlag())
    {
        LOG_INF("Setting TerminationFlag to avoid deadlocking unittest.");
        SigUtil::setTerminationFlag();
    }

    // Disable thread checking - we'll now cleanup lots of things if we can
    ThreadChecks::Inhibit = true;

    // Wait for the DocumentBrokers. They must be saving/uploading now.
    // Do not stop them! Otherwise they might not save/upload the document.
    // We block until they finish, or the service stopping times out.
    {
        std::unique_lock<std::mutex> docBrokersLock(DocBrokersMutex);
        for (const auto& docBrokerIt : DocBrokers)
        {
            const std::shared_ptr<DocumentBroker>& docBroker = docBrokerIt.second;
            if (docBroker && docBroker->isAlive())
            {
                LOG_DBG("Joining docBroker [" << docBrokerIt.first << "].");
                docBroker->joinThread();
            }
        }

        // Now should be safe to destroy what's left.
        cleanupDocBrokers();
        DocBrokers.clear();
    }

    SigUtil::addActivity("save traces");

    if (TraceEventFile != NULL)
    {
        // If we have written any objects to it, it ends with a comma and newline. Back over those.
        if (ftell(TraceEventFile) > 2)
            (void)fseek(TraceEventFile, -2, SEEK_CUR);
        // Close the JSON array.
        fprintf(TraceEventFile, "\n]\n");
        fclose(TraceEventFile);
        TraceEventFile = NULL;
    }

#if !MOBILEAPP
    if (!Util::isKitInProcess())
    {
        // Terminate child processes
        LOG_INF("Requesting forkit process " << ForKitProcId << " to terminate.");
#if CODE_COVERAGE || VALGRIND_COOLFORKIT
        constexpr auto signal = SIGTERM;
#else
        constexpr auto signal = SIGKILL;
#endif
        SigUtil::killChild(ForKitProcId, signal);
    }
#endif

    COOLWSDServer::Instance->stopPrisoners();

    SigUtil::addActivity("prisoners stopped");

    if (UnitWSD::isUnitTesting())
    {
        COOLWSDServer::Instance->stop();
        COOLWSDServer::Instance.reset();
    }

    PrisonerPoll.reset();

#if !MOBILEAPP
    net::AsyncDNS::stopAsyncDNS();
#endif

    SigUtil::addActivity("async DNS stopped");

    COOLWSDServer::WebServerPoll.reset();

    // Terminate child processes
    LOG_INF("Requesting child processes to terminate.");
    for (const auto& child : NewChildren)
    {
        child->terminate();
    }

    NewChildren.clear();

    SigUtil::addActivity("terminated unused children");

#if !MOBILEAPP
    if (!Util::isKitInProcess())
    {
        SigUtil::addActivity("waiting for forkit to exit");

        // Wait for forkit process finish.
        LOG_INF("Waiting for forkit process to exit");
        int status = 0;
        waitpid(ForKitProcId, &status, WUNTRACED);
        ForKitProcId = -1;
        ForKitProc.reset();
    }

    JailUtil::cleanupJails(CleanupChildRoot);
#endif // !MOBILEAPP

    if constexpr (Util::isMobileApp())
    {
        LOG_INF("Process [coolwsd] finished with exit status: " << EXIT_OK);
        Util::forcedExit(EXIT_OK);
    }

#endif // !IOS
}

std::shared_ptr<TerminatingPoll> COOLWSD:: getWebServerPoll ()
{
    return COOLWSDServer::WebServerPoll;
}

int COOLWSD::cleanup(int returnValue)
{
    try
    {
#ifndef IOS
        if (UnitBase::isUnitTesting())
        {
            LOG_DBG("Coolwsd finished with exit status " << returnValue
                                                         << "; uninitializing UnitBase");
            const int unitReturnValue = UnitBase::uninit();
            if (unitReturnValue != EXIT_OK)
            {
                // Overwrite the return value if the unit-test failed.
                LOG_INF("Overwriting process [coolwsd] exit status ["
                        << returnValue << "] with unit-test status: " << unitReturnValue);
                returnValue = unitReturnValue;
            }
        }

        LOG_INF("Process [coolwsd] finished with exit status: " << returnValue);

        SigUtil::addActivity("finished with status " + std::to_string(returnValue));
#endif // !IOS

        COOLWSDServer::Instance.reset();

        PrisonerPoll.reset();

        COOLWSDServer::WebServerPoll.reset();

        ClientRequestDispatcher::uninitialize();

#if !MOBILEAPP
        SavedClipboards.reset();

        FileRequestHandler.reset();
        JWTAuth::cleanup();

        Admin::uninitialize();

        Util::forcedExit(returnValue);

        TraceDumper.reset();

        ThreadChecks::Inhibit = true;

        // Delete these while the static Admin instance is still alive.
        {
            std::lock_guard<std::mutex> docBrokersLock(DocBrokersMutex);
            DocBrokers.clear();
        }

        SigUtil::uninitialize();

#if ENABLE_SSL
        // Finally, we no longer need SSL.
        if (ConfigUtil::isSslEnabled())
        {
#if !ENABLE_DEBUG
            // At least on centos7, Poco deadlocks while
            // cleaning up its SSL context singleton.
            Util::forcedExit(returnValue);
#endif // !ENABLE_DEBUG

            Poco::Net::uninitializeSSL();
            Poco::Crypto::uninitializeCrypto();
            ssl::Manager::uninitializeClientContext();
            ssl::Manager::uninitializeServerContext();
        }
#endif
#endif
    }
    catch (const std::exception& ex)
    {
        LOG_ERR("Failed to uninitialize: " << ex.what());
        throw;
    }

    return returnValue;
}

int COOLWSD::main(const std::vector<std::string>& /*args*/)
{
    SigUtil::resetTerminationFlags();

    int returnValue = EXIT_SOFTWARE;

    try
    {
        innerMain();
    }
    catch (const std::exception& e)
    {
        LOG_FTL("Exception: " << e.what());
        cleanup(EXIT_SOFTWARE);
        throw;
    }
    catch (...)
    {
        cleanup(EXIT_SOFTWARE);
        throw;
    }

    returnValue = cleanup(EXIT_OK);
    LOG_INF("Process [coolwsd] finished with exit status: " << returnValue);

#if CODE_COVERAGE
    __gcov_dump();
#endif

    return returnValue;
}

int COOLWSD::getClientPortNumber()
{
    return ClientPortNumber;
}

/// Only for unit testing ...
std::string COOLWSD::getJailRoot(int pid)
{
    std::lock_guard<std::mutex> docBrokersLock(DocBrokersMutex);
    for (const auto &it : DocBrokers)
    {
        if (pid < 0 || it.second->getPid() == pid)
            return it.second->getJailRoot();
    }
    return std::string();
}

#if !MOBILEAPP

std::vector<std::shared_ptr<DocumentBroker>> COOLWSD::getBrokersTestOnly()
{
    std::lock_guard<std::mutex> docBrokersLock(DocBrokersMutex);
    std::vector<std::shared_ptr<DocumentBroker>> result;

    result.reserve(DocBrokers.size());
    for (const auto& brokerIt : DocBrokers)
        result.push_back(brokerIt.second);
    return result;
}

std::set<pid_t> COOLWSD::getKitPids()
{
    std::set<pid_t> pids = getSpareKitPids();
    pids.merge(getDocKitPids());
    return pids;
}

std::set<pid_t> COOLWSD::getSpareKitPids()
{
    std::set<pid_t> pids;
    pid_t pid;
    {
        std::unique_lock<std::mutex> lock(NewChildrenMutex);
        for (const auto &child : NewChildren)
        {
            pid = child->getPid();
            if (pid > 0)
                pids.emplace(pid);
        }
    }
    return pids;
}

std::set<pid_t> COOLWSD::getDocKitPids()
{
    std::set<pid_t> pids;
    pid_t pid;
    {
        std::unique_lock<std::mutex> lock(DocBrokersMutex);
        for (const auto &it : DocBrokers)
        {
            pid = it.second->getPid();
            if (pid > 0)
                pids.emplace(pid);
        }
    }
    return pids;
}

#if !defined(BUILDING_TESTS)
namespace Util
{

void alertAllUsers(const std::string& cmd, const std::string& kind)
{
    alertAllUsers("error: cmd=" + cmd + " kind=" + kind);
}

void alertAllUsers(const std::string& msg)
{
    COOLWSD::alertAllUsersInternal(msg);
}

}
#endif

static void forwardSignal(int signum);

#endif

void dump_state()
{
    std::ostringstream oss(Util::makeDumpStateStream());
    oss << "Start WSD " << ProcUtil::getProcessId() << " Dump State:\n";

    if (COOLWSDServer::Instance)
        COOLWSDServer::Instance->dumpState(oss);

    oss << "\nMalloc info [" << ProcUtil::getProcessId() << "]: \n\t"
        << Util::replace(Util::getMallocInfo(), "\n", "\n\t") << '\n';
    oss << "\nEnd WSD " << ProcUtil::getProcessId() << " Dump State.\n";

    const std::string msg = oss.str();
    fprintf(stderr, "%s", msg.c_str()); // Log in the journal.
    LOG_WRN(msg);

#if !MOBILEAPP
    Admin::dumpMetrics();

    std::lock_guard<std::mutex> docBrokerLock(DocBrokersMutex);
    std::lock_guard<std::mutex> newChildLock(NewChildrenMutex);
    forwardSignal(SIGUSR1);
#endif
}

#if !MOBILEAPP

// The intent is that this function can be called from a debugger. It is not used otherwise.

void lslr_childroot()
{
    std::cout << "lslr: " << COOLWSD::ChildRoot << "\n";
    FileUtil::lslr(COOLWSD::ChildRoot);
    std::cout << std::flush;
}

#endif

void forwardSigUsr2()
{
#if !MOBILEAPP
    LOG_TRC("forwardSigUsr2");

    if (Util::isKitInProcess())
        return;

    Util::assertIsLocked(DocBrokersMutex);
    std::lock_guard<std::mutex> newChildLock(NewChildrenMutex);

    forwardSignal(SIGUSR2);
#endif
}

#if !MOBILEAPP
void forwardSignal(const int signum)
{
    Util::assertIsLocked(DocBrokersMutex);
    Util::assertIsLocked(NewChildrenMutex);

    const char* name = SigUtil::signalName(signum);

    if (COOLWSD::ForKitProcId > 0)
    {
        LOG_INF("Sending " << name << " to forkit " << COOLWSD::ForKitProcId);
        ::kill(COOLWSD::ForKitProcId, signum);
    }

    for (const auto& child : NewChildren)
    {
        if (child && child->getPid() > 0)
        {
            LOG_INF("Sending " << name << " to child " << child->getPid());
            ::kill(child->getPid(), signum);
        }
    }

    for (const auto& pair : DocBrokers)
    {
        const std::shared_ptr<DocumentBroker>& docBroker = pair.second;
        if (docBroker && docBroker->getPid() > 0)
        {
            LOG_INF("Sending " << name << " to docBroker " << docBroker->getPid());
            ::kill(docBroker->getPid(), signum);
        }
    }
}
#endif

// Avoid this in the Util::isFuzzing() case because libfuzzer defines its own main().
#if !MOBILEAPP && !LIBFUZZER && !defined(STANDALONE_CPPUNIT)

int main(int argc, char** argv)
{
    SigUtil::setUserSignals();
    SigUtil::setFatalSignals("wsd " + Util::getCoolVersion() + ' ' + Util::getCoolVersionHash());
    setKitInProcess();

    try
    {
        COOLWSD app;
        return app.run(argc, argv);
    }
    catch (Poco::Exception& exc)
    {
        std::cerr << exc.displayText() << std::endl;
        return EX_SOFTWARE;
    }
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
