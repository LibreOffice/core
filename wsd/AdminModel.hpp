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
 * Data model for admin console views and document tracking.
 * Classes: View, DocCleanupSettings, DocProcSettings, Document, Subscriber, AdminModel
 */

#pragma once

#include <common/Log.hpp>
#include <net/WebSocketHandler.hpp>

#include <ctime>
#include <deque>
#include <memory>
#include <regex>
#include <set>
#include <string>
#include <utility>

#include <Poco/URI.h>

struct DocumentAggregateStats;

/// A client view in Admin controller.
class View final
{
public:
    View(std::string sessionId, std::string userName, std::string userId, bool readOnly)
        : _sessionId(std::move(sessionId))
        , _userName(std::move(userName))
        , _userId(std::move(userId))
        , _loadDuration(0)
        , _readOnly(readOnly)
    {
    }

    void expire() { _end = std::time(nullptr); }
    const std::string& getUserName() const { return _userName; }
    const std::string& getUserId() const { return _userId; }
    const std::string& getSessionId() const { return _sessionId; }
    bool isExpired() const { return _end != 0 && std::time(nullptr) >= _end; }
    std::chrono::milliseconds getLoadDuration() const { return _loadDuration; }
    void setLoadDuration(std::chrono::milliseconds loadDuration) { _loadDuration = loadDuration; }
    bool isReadOnly() const { return _readOnly; }

private:
    const std::string _sessionId;
    const std::string _userName;
    const std::string _userId;
    std::time_t _end = 0;
    std::chrono::milliseconds _loadDuration;
    bool _readOnly = false;
};

struct DocCleanupSettings
{
    DocCleanupSettings()
        : _cleanupInterval(0)
        , _badBehaviorPeriod(0)
        , _idleTime(0)
        , _limitDirtyMem(0)
        , _limitCpu(0)
        , _lostKitGracePeriod(0)
        , _enable(false)
    {
    }

    void setEnable(bool enable) { _enable = enable; }
    bool getEnable() const { return _enable; }
    void setCleanupInterval(size_t cleanupInterval) { _cleanupInterval = cleanupInterval; }
    size_t getCleanupInterval() const { return _cleanupInterval; }
    void setBadBehaviorPeriod(size_t badBehaviorPeriod) { _badBehaviorPeriod = badBehaviorPeriod; }
    size_t getBadBehaviorPeriod() const { return _badBehaviorPeriod; }
    void setIdleTime(size_t idleTime) { _idleTime = idleTime; }
    size_t getIdleTime() const { return _idleTime; }
    void setLimitDirtyMem(size_t limitDirtyMem) { _limitDirtyMem = limitDirtyMem; }
    size_t getLimitDirtyMem() const { return _limitDirtyMem; }
    void setLimitCpu(size_t limitCpu) { _limitCpu = limitCpu; }
    size_t getLimitCpu() const { return _limitCpu; }
    void setLostKitGracePeriod(size_t lostKitGracePeriod) { _lostKitGracePeriod = lostKitGracePeriod; }
    size_t getLostKitGracePeriod() const { return _lostKitGracePeriod; }

private:
    size_t _cleanupInterval;
    size_t _badBehaviorPeriod;
    size_t _idleTime;
    size_t _limitDirtyMem;
    size_t _limitCpu;
    size_t _lostKitGracePeriod;
    bool _enable;
};

struct DocProcSettings
{
    DocProcSettings()
        : _limitVirtMemMb(0)
        , _limitStackMemKb(0)
        , _limitFileSizeMb(0)
        , _limitNumberOpenFiles(0)
    {
    }

    void setLimitVirtMemMb(size_t limitVirtMemMb) { _limitVirtMemMb = limitVirtMemMb; }
    size_t getLimitVirtMemMb() const { return _limitVirtMemMb; }
    void setLimitStackMemKb(size_t limitStackMemKb) { _limitStackMemKb = limitStackMemKb; }
    size_t getLimitStackMemKb() const { return _limitStackMemKb; }
    void setLimitFileSizeMb(size_t limitFileSizeMb) { _limitFileSizeMb = limitFileSizeMb; }
    size_t getLimitFileSizeMb() const { return _limitFileSizeMb; }
    void setLimitNumberOpenFiles(size_t limitNumberOpenFiles) { _limitNumberOpenFiles = limitNumberOpenFiles; }
    size_t getLimitNumberOpenFiles() const { return _limitNumberOpenFiles; }

    DocCleanupSettings& getCleanupSettings() { return _docCleanupSettings; }

private:
    DocCleanupSettings _docCleanupSettings;

    size_t _limitVirtMemMb;
    size_t _limitStackMemKb;
    size_t _limitFileSizeMb;
    size_t _limitNumberOpenFiles;
};

/// Containing basic information about document
class DocBasicInfo final
{
    std::string _docKey;
    std::time_t _idleTime;
    int _mem;
    bool _saved;

public:
    DocBasicInfo(std::string docKey, std::time_t idleTime, int mem, bool saved)
        : _docKey(std::move(docKey))
        , _idleTime(idleTime)
        , _mem(mem)
        , _saved(saved)
    {
    }

    const std::string& getDocKey() const { return _docKey; }

    std::time_t getIdleTime() const { return _idleTime; }

    int getMem() const { return _mem; }

    bool getSaved() const { return _saved; }
};

/// A document in Admin controller.
class AdminDocument final
{
public:
    AdminDocument(const std::string& docKey, pid_t pid, const std::string& filename,
                  const Poco::URI& wopiSrc)
        : _wopiSrc(wopiSrc.toString())
        , _hostName(wopiSrc.getHost())
        , _docKey(docKey)
        , _filename(filename)
        , _memoryDirty(0)
        , _lastJiffy(0)
        , _start(std::time(nullptr))
        , _lastActivity(_start)
        , _end(0)
        , _lastSnapshotTime(0)
        , _sentBytes(0)
        , _recvBytes(0)
        , _wopiDownloadDuration(0)
        , _wopiUploadDuration(0)
        , _lastTimeSMapsRead(0)
        , _badBehaviorDetectionTime(0)
        , _abortTime(0)
        , _pid(pid)
        , _activeViews(0)
        , _lastCpuPercentage(0)
        , _isModified(false)
        , _hasMemDirtyChanged(true)
        , _isUploaded(false)
    {
    }

    const std::string& getDocKey() const { return _docKey; }

    pid_t getPid() const { return _pid; }

    const std::string& getFilename() const { return _filename; }

    const std::string& getHostName() const { return _hostName; }

    const std::string& getWopiSrc() const { return _wopiSrc; }

    bool isExpired() const { return _end != 0 && std::time(nullptr) >= _end; }

    std::time_t getElapsedTime() const { return std::time(nullptr) - _start; }

    std::time_t getIdleTime() const { return std::time(nullptr) - _lastActivity; }

    void addView(const std::string& sessionId, const std::string& userName, const std::string& userId, bool readOnly);

    int expireView(const std::string& sessionId);

    unsigned getActiveViews() const { return _activeViews; }

    size_t getLastJiffies() const { return _lastJiffy; }
    void setLastJiffies(size_t newJ);
    unsigned getLastCpuPercentage() const { return _lastCpuPercentage; }

    const std::map<std::string, View>& getViews() const { return _views; }

    void updateLastActivityTime(std::time_t lastActivity) { _lastActivity = lastActivity; }
    std::time_t getLastActivityTime() const { return _lastActivity; }
    void updateMemoryDirty();
    size_t getMemoryDirty() const { return _memoryDirty; }

    std::string getSnapshot(std::time_t now) const;
    const std::string getHistory() const;
    void takeSnapshot();

    void setModified(bool value) { _isModified = value; }
    bool getModifiedStatus() const { return _isModified; }

    void setUploaded(bool value) { _isUploaded = value; }
    bool getUploadedStatus() const { return _isUploaded; }

    void addBytes(uint64_t sent, uint64_t recv)
    {
        _sentBytes += sent;
        _recvBytes += recv;
    }

    std::time_t getOpenTime() const { return isExpired() ? _end - _start : getElapsedTime(); }
    uint64_t getSentBytes() const { return _sentBytes; }
    uint64_t getRecvBytes() const { return _recvBytes; }
    void setViewLoadDuration(const std::string& sessionId, std::chrono::milliseconds viewLoadDuration);
    void setWopiDownloadDuration(std::chrono::milliseconds wopiDownloadDuration) { _wopiDownloadDuration = wopiDownloadDuration; }
    std::chrono::milliseconds getWopiDownloadDuration() const { return _wopiDownloadDuration; }
    void setWopiUploadDuration(const std::chrono::milliseconds wopiUploadDuration) { _wopiUploadDuration = wopiUploadDuration; }
    std::chrono::milliseconds getWopiUploadDuration() const { return _wopiUploadDuration; }
    void setProcSMapsFp(std::weak_ptr<FILE> procSMaps) { _procSMaps = std::move(procSMaps); }
    bool hasMemDirtyChanged() const { return _hasMemDirtyChanged; }
    void setMemDirtyChanged(bool changeStatus) { _hasMemDirtyChanged = changeStatus; }
    time_t getBadBehaviorDetectionTime() const { return _badBehaviorDetectionTime; }
    void setBadBehaviorDetectionTime(time_t badBehaviorDetectionTime){ _badBehaviorDetectionTime = badBehaviorDetectionTime;}
    time_t getAbortTime() const { return _abortTime; }
    void setAbortTime(time_t abortTime) { _abortTime = abortTime; }

    std::string to_string() const;

private:
    std::string _wopiSrc;
    std::string _hostName;
    /// SessionId mapping to View object
    std::map<std::string, View> _views;
    std::vector<std::string> _snapshots;
    std::string _docKey;
    /// Hosted filename
    std::string _filename;
    /// The dirty (ie. un-shared) memory of the document's Kit process.
    size_t _memoryDirty;
    /// Last noted Jiffy count
    size_t _lastJiffy;
    std::chrono::steady_clock::time_point _lastJiffyTime;
    std::time_t _start;
    std::time_t _lastActivity;
    std::time_t _end;
    std::time_t _lastSnapshotTime;

    /// Total bytes sent and recv'd by this document.
    uint64_t _sentBytes, _recvBytes;

    //Download/upload duration from/to storage for this document
    std::chrono::milliseconds _wopiDownloadDuration;
    std::chrono::milliseconds _wopiUploadDuration;

    std::weak_ptr<FILE> _procSMaps;
    std::time_t _lastTimeSMapsRead;

    std::time_t _badBehaviorDetectionTime;
    std::time_t _abortTime;

    pid_t _pid;
    /// Total number of active views
    unsigned _activeViews;

    unsigned _lastCpuPercentage;

    bool _isModified;
    bool _hasMemDirtyChanged;

    bool _isUploaded;
};

/// An Admin session subscriber.
class Subscriber final
{
public:
    explicit Subscriber(std::weak_ptr<WebSocketHandler> ws)
        : _ws(std::move(ws))
    {
        LOG_DBG("Subscriber ctor");
    }

    ~Subscriber() { LOG_DBG("Subscriber dtor"); }

    bool notify(const std::string& message);

    bool subscribe(const std::string& command);

    void unsubscribe(const std::string& command);

    void expire() { _end = std::time(nullptr); }

    bool isExpired() const { return _end != 0 && std::time(nullptr) >= _end; }

private:
    /// The underlying AdminRequestHandler
    std::weak_ptr<WebSocketHandler> _ws;

    std::set<std::string> _subscriptions;

    std::time_t _end = 0;
};

/// The Admin controller implementation.
class AdminModel final
{
    AdminModel(const AdminModel &) = delete;
    AdminModel& operator = (const AdminModel &) = delete;
public:
    AdminModel()
        : _segFaultCount(0)
        , _owner(ProcUtil::getThreadId())
    {
        LOG_INF("AdminModel ctor.");
    }

    ~AdminModel();

    /// All methods here must be called from the Admin socket-poll
    void setThreadOwner(const ProcUtil::ThreadId id) { _owner = id; }

    std::string query(const std::string& command);
    std::string getAllHistory() const;

    /// Returns memory consumed by all active coolkit processes
    unsigned getKitsMemoryUsage() const;
    size_t getKitsJiffies() const;

    void subscribe(int sessionId, const std::weak_ptr<WebSocketHandler>& ws);
    void subscribe(int sessionId, const std::string& command);

    void unsubscribe(int sessionId, const std::string& command);

    void modificationAlert(const std::string& docKey, pid_t pid, bool value);

    void uploadedAlert(const std::string& docKey, pid_t pid, bool value);

    void clearMemStats() { _memStats.clear(); }

    void clearCpuStats() { _cpuStats.clear(); }

    void addMemStats(unsigned memUsage);

    void addCpuStats(unsigned cpuUsage);

    void addSentStats(uint64_t sent);

    void addRecvStats(uint64_t recv);

    void addConnectionStats(size_t connections);

    void setCpuStatsSize(unsigned size);

    void setMemStatsSize(unsigned size);

    void notify(const std::string& message);

    void addDocument(const std::string& docKey, pid_t pid, const std::string& filename,
                     const std::string& sessionId, const std::string& userName,
                     const std::string& userId, const std::weak_ptr<FILE>& smapsFp,
                     const Poco::URI& wopiSrc, bool readOnly);

    void removeDocument(const std::string& docKey, const std::string& sessionId);
    void removeDocument(const std::string& docKey);

    void updateLastActivityTime(const std::string& docKey);
    std::time_t getLastActivityTime() const { return _lastActivity; }

    void addBytes(const std::string& docKey, uint64_t sent, uint64_t recv);

    uint64_t getSentBytesTotal() const { return _sentBytesTotal; }
    uint64_t getRecvBytesTotal() const { return _recvBytesTotal; }

    static double getServerUptimeSecs();

    /// Document basic info list sorted by most idle time
    std::vector<DocBasicInfo> getDocumentsSortedByIdle() const;
    void cleanupResourceConsumingDocs();

    void setViewLoadDuration(const std::string& docKey, const std::string& sessionId, std::chrono::milliseconds viewLoadDuration);
    void setDocWopiDownloadDuration(const std::string& docKey, std::chrono::milliseconds wopiDownloadDuration);
    void setDocWopiUploadDuration(const std::string& docKey,
                                  std::chrono::milliseconds wopiUploadDuration);
    void addErrorExitCounters(unsigned segFaultCount, unsigned killedCount,
                              unsigned oomKilledCount);
    void setForKitPid(pid_t pid) { _forKitPid = pid; }
    void addLostKitsTerminated(unsigned lostKitsTerminated);

    void getMetrics(std::ostream& oss) const;

    std::set<pid_t> getDocumentPids() const;
    void UpdateMemoryDirty();
    void notifyDocsMemDirtyChanged();

    const DocProcSettings& getDefDocProcSettings() const { return _defDocProcSettings; }
    void setDefDocProcSettings(const DocProcSettings& docProcSettings) { _defDocProcSettings = docProcSettings; }

    static int getPidsFromProcName(const std::regex& procNameRegEx, std::vector<int> *pids);
    static int getAssignedKitPids(std::vector<int> *pids);
    static int getUnassignedKitPids(std::vector<int> *pids);
    static void getKitPidsFromSystem(std::vector<int> *pids);
    bool isDocSaved(const std::string&);
    bool isDocReadOnly(const std::string&);
    void setMigratingInfo(const std::string& docKey, const std::string& routeToken, const std::string& serverId);
    void resetMigratingInfo();
    const std::string& getCurrentMigDoc() const { return _currentMigDoc; }
    const std::string& getCurrentMigToken() const { return _currentMigToken; }
    const std::string& getTargetMigServerId() const { return _targetMigServerId; }
    void sendMigrateMsgAfterSave(bool lastSaveSuccessful, const std::string& docKey);
    std::string getWopiSrcMap() const;
    std::string getFilename(int pid) const;
    void routeTokenSanityCheck();
    void sendShutdownReceivedMsg();

private:
    void doRemove(std::map<std::string, AdminDocument>::iterator &docIt);

    std::string getMemStats() const;

    std::string getSentActivity() const;

    std::string getRecvActivity() const;

    std::string getConnectionActivity() const;

    std::string getCpuStats() const;

    unsigned getTotalActiveViews() const;

    std::string getDocuments() const;

    void CalcDocAggregateStats(DocumentAggregateStats& stats) const;

private:
    DocProcSettings _defDocProcSettings;

    std::map<int, Subscriber> _subscribers;
    std::map<std::string, AdminDocument> _documents;

    /// The serialized histories of all expired documents.
    std::vector<std::string> _expiredDocumentsHistories;

    /// The last N total memory Dirty size.
    std::deque<unsigned> _memStats;
    std::deque<unsigned> _cpuStats;
    std::deque<unsigned> _sentStats;
    std::deque<unsigned> _recvStats;
    std::deque<size_t> _connStats;

    uint64_t _sentBytesTotal = 0;
    uint64_t _recvBytesTotal = 0;

    uint64_t _segFaultCount = 0;
    uint64_t _lostKitsTerminatedCount = 0;
    uint64_t _killedCount = 0;
    uint64_t _oomKilledCount = 0;

    std::time_t _lastActivity = 0;

    /// We check the owner even in the release builds, needs to be always correct.
    ProcUtil::ThreadId _owner;

    std::string _currentMigDoc;

    std::string _currentMigToken;

    std::string _targetMigServerId;

    unsigned _memStatsSize = 100;
    unsigned _cpuStatsSize = 100;
    unsigned _sentStatsSize = 200;
    unsigned _recvStatsSize = 200;
    unsigned _connStatsSize = 200;

    pid_t _forKitPid = 0;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
