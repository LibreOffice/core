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
 * Main server application class and entry point.
 * Classes: COOLWSD
 */

#pragma once

#include <common/ConfigUtil.hpp>
#include <common/FileUtil.hpp>
#include <common/Unit.hpp>
#include <common/Util.hpp>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <string>

#include <Poco/Path.h>
#include <Poco/Util/AbstractConfiguration.h>
#include <Poco/Util/OptionSet.h>
#include <Poco/Util/ServerApplication.h>

class WSProcess;
class ForKitProcess;
class ChildProcess;
class ClipboardCache;
class DocumentBroker;
class FileServerRequestHandler;
class ForKitProcess;
class SocketPoll;
class TraceFileWriter;

std::shared_ptr<ChildProcess> getNewChild_Blocks(const std::shared_ptr<SocketPoll>& destPoll,
                                                 const std::string& configId,
                                                 unsigned mobileAppDocId);

/// The Server class which is responsible for all
/// external interactions.
class COOLWSD final : public Poco::Util::ServerApplication,
                      public UnitWSDInterface
{
public:
    COOLWSD();
    ~COOLWSD();

    // An Application is a singleton anyway,
    // so just keep these as statics.
    static unsigned int NumPreSpawnedChildren;
#if !MOBILEAPP
    static bool NoCapsForKit;
    static bool NoSeccomp;
    static bool AdminEnabled;
    static bool UnattendedRun; ///< True when run from an unattended test, not interactive.
    static bool SignalParent;
    static bool UseEnvVarOptions;
    static std::string RouteToken;
#if ENABLE_DEBUG
    static bool SingleKit;
    static bool ForceCaching;
#endif
    static std::shared_ptr<ForKitProcess> ForKitProc;
    static std::atomic<int> ForKitProcId;
#endif
    static std::string UserInterface;
    static std::string ConfigFile;
    static std::string ConfigDir;
    static std::string SysTemplate;
    static std::string LoTemplate;
    static std::string CleanupChildRoot;
    static std::string ChildRoot;
    static std::string ServerName;
    static std::string FileServerRoot;
    static std::string ServiceRoot; ///< There are installations that need prefixing every page with some path.
    static std::string TmpFontDir;
    static std::string LOKitVersion;
    static std::string LOKitVersionNumber;
    static std::string LOKitVersionHash;
    static bool EnableTraceEventLogging;
    static bool EnableAccessibility;
    static bool EnableMountNamespaces;
    static FILE *TraceEventFile;
    static void writeTraceEventRecording(const char *data, std::size_t nbytes);
    static void writeTraceEventRecording(const std::string &recording);
    static std::string LogLevel;
    static std::string LogLevelStartup;
    static std::string LogDisabledAreas;
    static std::string LogToken;
    static std::string MostVerboseLogLevelSettableFromClient;
    static std::string LeastVerboseLogLevelSettableFromClient;
    static bool AnonymizeUserData;
    static bool CheckCoolUser;
    static bool CleanupOnly;
#if ENABLE_DEBUG
    static bool FindFreePort;
#endif
    static bool IsProxyPrefixEnabled;
    static std::atomic<unsigned> NumConnections;
    static std::unique_ptr<TraceFileWriter> TraceDumper;
    static bool IndirectionServerEnabled;
    static bool GeolocationSetup;
#if !MOBILEAPP
    static std::unique_ptr<ClipboardCache> SavedClipboards;

    /// The file request handler used for file-serving.
    static std::unique_ptr<FileServerRequestHandler> FileRequestHandler;

    /// The WASM support/activation state.
    enum class WASMActivationState : std::uint8_t
    {
        Disabled,
        Enabled
#if ENABLE_DEBUG
        ,
        Forced ///< When Forced, only WASM is served.
#endif
    };
    static WASMActivationState WASMState;

    /// Tracks the URIs that are switching to Disconnected (WASM) Mode.
    /// The time is when the switch request was made. We expire the request after a certain
    /// time, in case the user fails to load WASM, it will revert to Collaborative mode.
    static std::unordered_map<std::string, std::chrono::steady_clock::time_point> Uri2WasmModeMap;
#endif

    static std::unordered_set<std::string> EditFileExtensions;
    static std::string ViewModeFileExtensions;
    static unsigned MaxConnections;
    static unsigned MaxDocuments;
    static std::string HardwareResourceWarning;
    static std::string OverrideWatermark;
    static std::set<const Poco::Util::AbstractConfiguration*> PluginConfigurations;
    static std::chrono::steady_clock::time_point StartTime;
    static std::string BuyProductUrl;
    static std::string LatestVersion;
    static std::mutex FetchUpdateMutex;
    static bool IsBindMountingEnabled;
    static std::mutex RemoteConfigMutex;

    /// For testing only [!]
    static int getClientPortNumber();
    /// For testing only [!] DocumentBrokers are mostly single-threaded with their own thread
    static std::vector<std::shared_ptr<DocumentBroker>> getBrokersTestOnly();

    // Return a map for fast searches. Used in testing and in admin for cleanup
    static std::set<pid_t> getKitPids();
    static std::set<pid_t> getSpareKitPids();
    static std::set<pid_t> getDocKitPids();

    static const std::string& getHardwareResourceWarning()
    {
        return HardwareResourceWarning;
    }

    static bool isSSLTermination() { return ConfigUtil::isSSLTermination(); }

    static std::shared_ptr<TerminatingPoll> getWebServerPoll();

    /// Return true if extension is marked as view action in discovery.xml.
    static bool IsViewFileExtension(const std::string& extension)
    {
        std::string lowerCaseExtension = extension;
        std::transform(lowerCaseExtension.begin(), lowerCaseExtension.end(), lowerCaseExtension.begin(), ::tolower);
        if constexpr (Util::isMobileApp())
        {
            if (lowerCaseExtension == "pdf")
                return true; // true for only pdf - it is not editable
            return false; // mark everything else editable on mobile
        }
        return EditFileExtensions.find(lowerCaseExtension) == EditFileExtensions.end();
    }


    /// Trace a new session and take a snapshot of the file.
    static void dumpNewSessionTrace(const std::string& id, const std::string& sessionId, const std::string& uri, const std::string& path);

    /// Trace the end of a session.
    static void dumpEndSessionTrace(const std::string& id, const std::string& sessionId, const std::string& uri);

    static void dumpEventTrace(const std::string& id, const std::string& sessionId, const std::string& data);

    static void dumpIncomingTrace(const std::string& id, const std::string& sessionId, const std::string& data);

    static void dumpOutgoingTrace(const std::string& id, const std::string& sessionId, const std::string& data);

    /// Waits on Forkit and reaps if it dies, then restores.
    /// Return true if wait succeeds.
    static bool checkAndRestoreForKit();

    /// Creates a new instance of Forkit.
    /// Return true when successful.
    static bool createForKit();

    /// Sends a message to ForKit through PrisonerPoll. An empty configId is the primordial forkit.
    static bool sendMessageToForKit(const std::string& message, const std::string& configId=std::string());

    /// Terminates spare kits that aren't assigned a document yet.
    static void requestTerminateSpareKits();

    // Request creation of a subForKit if it doesn't already exist
    static bool ensureSubForKit(const std::string& id);

    /// Checks forkit (and respawns), rebalances
    /// child kit processes and cleans up DocBrokers.
    static void doHousekeeping();

    /// Clean up spent DocBrokers.
    static void cleanupDocBrokers();

    static void checkDiskSpaceAndWarnClients(bool cacheLastCheck);

    static void checkSessionLimitsAndWarnClients();

    /// Close document with @docKey and a @message
    static void closeDocument(const std::string& docKey, const std::string& message);

    /// Autosave a given document (currently only called from Admin).
    static void autoSave(const std::string& docKey);

    /// Sets the log level of current kits.
    static void setLogLevelsOfKits(const std::string& level);

    /// Anonymize the basename of filenames, preserving the path and extension.
    static std::string anonymizeUrl(const std::string& url)
    {
        return FileUtil::anonymizeUrl(url);
    }

    /// Anonymize user names and IDs.
    /// Will use the Obfuscated User ID if one is provided via WOPI.
    static std::string anonymizeUsername(const std::string& username)
    {
        return FileUtil::anonymizeUsername(username);
    }
    static void alertAllUsersInternal(const std::string& msg);
    static void alertUserInternal(const std::string& dockey, const std::string& msg);
    static void setMigrationMsgReceived(const std::string& docKey);
    static void setAllMigrationMsgReceived();
#if !MOBILEAPP
    static void syncUsersBrowserSettings(const std::string& userId, pid_t childPid,
                                         const std::string& json);
#endif

    static void setLokitEnvironmentVariables(const Poco::Util::LayeredConfiguration& conf);

#if ENABLE_DEBUG
    /// get correct server URL with protocol + port number for this running server
    static std::string getServerURL();
#endif

protected:
    void initialize(Poco::Util::Application& self) override
    {
        try
        {
            innerInitialize(self);
        }
        catch (const Poco::Exception& ex)
        {
            LOG_FTL("Failed to initialize COOLWSD: "
                    << ex.displayText()
                    << (ex.nested() ? " (" + ex.nested()->displayText() + ')' : ""));
            throw; // Nothing further to do.
        }
        catch (const std::exception& ex)
        {
            LOG_FTL("Failed to initialize COOLWSD: " << ex.what());
            throw; // Nothing further to do.
        }
    }

    void defineOptions(Poco::Util::OptionSet& options) override;
    void handleOption(const std::string& name, const std::string& value) override;
    int main(const std::vector<std::string>& args) override;

    /// Handle various global static destructors.
    static int cleanup(int returnValue);

private:
#if !MOBILEAPP
    void processFetchUpdate(const std::shared_ptr<SocketPoll>& poll);
    static bool testMountingNSInFork();
    static void setupChildRoot(bool UseMountNamespaces);
    void initializeEnvOptions();
#endif // !MOBILEAPP

    void initializeSSL();
    void displayHelp();

    /// The actual initialize implementation.
    void innerInitialize(Application& self);

    /// The actual main implementation.
    void innerMain();

    static void appendAllowedHostsFrom(const Poco::Util::LayeredConfiguration& conf, const std::string& root, std::vector<std::string>& allowed);
    static void appendAllowedAliasGroups(const Poco::Util::LayeredConfiguration& conf, std::vector<std::string>& allowed);

private:
    /// UnitWSDInterface
    virtual std::string getJailRoot(int pid) override;

    /// Settings passed from the command-line to override those in the config file.
    std::unordered_map<std::string, std::string> _overrideSettings;
};

int createForkit(const std::string& forKitPath, const StringVector& args);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
