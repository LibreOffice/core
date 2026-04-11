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
 * Administrative interface for monitoring and managing server.
 * Classes: AdminSocketHandler, MonitorSocketHandler, Admin
 */

#pragma once

#include <AdminModel.hpp>

#include <net/WebSocketHandler.hpp>
#include <common/ConfigUtil.hpp>

#include <memory>

class Admin;

/// Handle admin client's Websocket requests & replies.
class AdminSocketHandler : public WebSocketHandler
{
public:
    /// Client connection to remote admin socket
    explicit AdminSocketHandler(Admin* adminManager);

    /// Connection from remote admin socket
    AdminSocketHandler(Admin* adminManager,
                       const std::weak_ptr<StreamSocket>& socket,
                       const Poco::Net::HTTPRequest& request,
                       bool allowedOrigin);

    /// Handle the initial Admin WS upgrade request.
    /// @returns true if we should give this socket to the Admin poll.
    static bool handleInitialRequest(const std::weak_ptr<StreamSocket> &socket,
                                     const Poco::Net::HTTPRequest& request,
                                     bool allowedOrigin);

    static void subscribeAsync(const std::shared_ptr<AdminSocketHandler>& handler);

    /// Process incoming websocket messages
    void handleMessage(const std::vector<char> &data) override;

private:
    /// Sends text frames simply to authenticated clients.
    void sendTextFrame(const std::string& message);

private:
    std::string _clientIPAdress;
    Admin* _admin;
    int _sessionId;
    bool _isAuthenticated;

    /// The next unique session-ID.
    static std::atomic<uint64_t> NextSessionId;
};

class MonitorSocketHandler : public AdminSocketHandler
{
public:
    MonitorSocketHandler(Admin *admin, const std::string &uri);

    int getPollEvents(std::chrono::steady_clock::time_point now,
                      int64_t &timeoutMaxMicroS) override;

    void performWrites(std::size_t capacity) override;

    void onDisconnect() override;

private:
    std::string _uri;
    bool _connecting;
};

class MemoryStatsTask;

/// An admin command processor.
class Admin final : public SocketPoll
{
    Admin(const Admin &) = delete;
    Admin& operator = (const Admin &) = delete;
    Admin();

    static std::unique_ptr<Admin> Instance;

public:
    ~Admin() override;

    static void initialize()
    {
        assert(Instance == nullptr && "Unexpected double initialization of Admin");
        Instance.reset(new Admin);
    }

    static void uninitialize()
    {
        if (Instance)
        {
            Instance->stop();
            Instance->joinThread();

            Instance.reset();
        }
    }
    static Admin& instance()
    {
        assert(Instance && "Expected a valid Admin instance");
        return *Instance;
    }

    void start();
    void stop();

    void startMonitors();

    void updateMonitors(std::vector<std::pair<std::string, int>>& oldMonitors);

    std::vector<std::pair<std::string, int>> getMonitorList() const;

    /// Custom poll thread function
    void pollingThread() override;

    size_t getTotalMemoryUsage() const;
    /// Takes into account the 'memproportion' property in config file to find the amount of memory
    /// available to us.
    size_t getTotalAvailableMemory() const { return _totalAvailMemKb; }
    size_t getTotalCpuUsage() const;
    std::time_t getLastActivityTime() const;

    void modificationAlert(const std::string& dockey, pid_t pid, bool value);

    void uploadedAlert(const std::string& dockey, pid_t pid, bool value);

    /// Update the Admin Model.
    void update(const std::string& message);

    /// Calls with same pid will increment view count, if pid already exists
    void addDoc(const std::string& docKey, pid_t pid, const std::string& filename,
                const std::string& sessionId, const std::string& userName,
                const std::string& userId, const std::weak_ptr<FILE>& smapsFD,
                const std::string& wopiSrc, bool readOnly);

    /// Decrement view count till becomes zero after which doc is removed
    void rmDoc(const std::string& docKey, const std::string& sessionId);

    /// Remove the document with all views. Used on termination or catastrophic failure.
    void rmDoc(const std::string& docKey);

    void setForKitPid(const int forKitPid) { _forKitPid = forKitPid; _model.setForKitPid(forKitPid);}

    /// Callers must ensure that modelMutex is acquired
    AdminModel& getModel();

    std::chrono::milliseconds getMemStatsInterval() const;

    std::chrono::milliseconds getCpuStatsInterval() const;

    std::chrono::milliseconds getNetStatsInterval() const;

    /// Returns the log levels of wsd and forkit & kits.
    std::string getChannelLogLevels() const;

    /// Sets the specified channel's log level (wsd or forkit and kits).
    void setChannelLogLevel(const std::string& channelName, std::string level);

    std::string getLogLines() const;

    void rescheduleMemTimer(std::chrono::milliseconds interval);

    void rescheduleCpuTimer(std::chrono::milliseconds interval);

    void updateLastActivityTime(const std::string& docKey);
    void addBytes(const std::string& docKey, uint64_t sent, uint64_t recv);

    void dumpState(std::ostream& os) const override;

    const DocProcSettings& getDefDocProcSettings() const { return _defDocProcSettings; }
    void setDefDocProcSettings(const DocProcSettings& docProcSettings, bool notifyKit)
    {
        _defDocProcSettings = docProcSettings;
        _model.setDefDocProcSettings(docProcSettings);
        _cleanupIntervalMs = std::chrono::milliseconds(
            _defDocProcSettings.getCleanupSettings().getCleanupInterval());
        if (notifyKit)
            notifyForkit();
    }

    /// Attempt a synchronous connection to a monitor with @uri @when that future comes
    void scheduleMonitorConnect(const std::string &uri, std::chrono::steady_clock::time_point when);

    void sendMetrics(const std::shared_ptr<StreamSocket>& socket,
                     const std::shared_ptr<http::Response>& response) const;

    void setViewLoadDuration(const std::string& docKey, const std::string& sessionId, std::chrono::milliseconds viewLoadDuration);
    void setDocWopiDownloadDuration(const std::string& docKey, std::chrono::milliseconds wopiDownloadDuration);
    void setDocWopiUploadDuration(const std::string& docKey,
                                  std::chrono::milliseconds uploadDuration);
    void addErrorExitCounters(unsigned segFaultCount, unsigned killedCount,
                              unsigned oomKilledCount);
    void addLostKitsTerminated(unsigned lostKitsTerminated);

    void getMetrics(std::ostream& metrics) const;

    /// Will dump the metrics in the log and stderr from the Admin SocketPoll.
    static void dumpMetrics() { instance()._dumpMetrics = true; }

    // delete entry from _monitorSocket map
    void deleteMonitorSocket(const std::string &uriWithoutParam);

    bool logAdminAction()
    {
        return ConfigUtil::getConfigValue<bool>("admin_console.logging.admin_action", true);
    }

    void routeTokenSanityCheck();

    void sendShutdownReceivedMsg();

    void setCloseMonitorFlag() { _closeMonitor = true; }

private:
    /// Notify Forkit of changed settings.
    void notifyForkit();

    /// Memory consumption has increased, start killing kits etc. till memory consumption gets back
    /// under @hardModeLimit
    void triggerMemoryCleanup(size_t hardModeLimit);
    void notifyDocsMemDirtyChanged();
    void cleanupResourceConsumingDocs();
    void cleanupLostKits();

    /// Round the interval up to multiples of MinStatsIntervalMs.
    /// This is to avoid arbitrarily small intervals that hammer the server.
    static std::chrono::milliseconds capAndRoundInterval(std::chrono::milliseconds interval)
    {
        const auto value = std::max(interval, MinStatsIntervalMs);
        constexpr auto d = MinStatsIntervalMs.count();
        return std::chrono::milliseconds(((value.count() + d - 1) / d) * d);
    }

    /// Synchronous connection setup to remote monitoring server
    void connectToMonitorSync(const std::string &uri);

private:
    /// The model is accessed only during startup & in
    /// the Admin Poll thread.
    AdminModel _model;
    DocProcSettings _defDocProcSettings;
    // map to make sure only connection with unique monitor uri exists
    std::map<std::string, std::shared_ptr<MonitorSocketHandler>> _monitorSockets;

    /// The total installed system memory (RAM).
    /// Technically, can be augmented at runtime, but we don't update it.
    const size_t _totalSysMemKb;
    /// The total available memory to our process, per memproportion.
    size_t _totalAvailMemKb;

    size_t _lastTotalMemory;
    mutable size_t _lastJiffies;
    uint64_t _lastSentCount;
    uint64_t _lastRecvCount;
    std::string _forkitLogLevel;

    struct MonitorConnectRecord
    {
        void setWhen(std::chrono::steady_clock::time_point when) { _when = when; }
        std::chrono::steady_clock::time_point getWhen() const { return _when; }

        void setUri(const std::string& uri) { _uri = uri; }
        const std::string& getUri() const { return _uri; }

    private:
        std::chrono::steady_clock::time_point _when;
        std::string _uri;
    };
    std::vector<MonitorConnectRecord> _pendingConnects;

    std::chrono::milliseconds _cpuStatsTaskIntervalMs;
    std::chrono::milliseconds _memStatsTaskIntervalMs;
    std::chrono::milliseconds _netStatsTaskIntervalMs;
    std::chrono::milliseconds _cleanupIntervalMs;

    int _forKitPid;

    /// When set, the metrics will be dumped into the log and stderr.
    std::atomic_bool _dumpMetrics;

    std::atomic<bool> _closeMonitor = false;

    // Don't update any more frequently than this since it's excessive.
    static constexpr std::chrono::milliseconds MinStatsIntervalMs{ 50 };
    static constexpr std::chrono::milliseconds DefStatsIntervalMs{ 1000 };
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
