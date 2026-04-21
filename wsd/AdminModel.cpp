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
 * Implementation of admin data model and document statistics.
 * Classes: Document, Subscriber, AdminModel
 */

#include <config.h>

#include "AdminModel.hpp"

#include <common/ConfigUtil.hpp>
#include <common/Log.hpp>
#include <common/Protocol.hpp>
#include <common/Unit.hpp>
#include <common/Util.hpp>
#include <net/WebSocketHandler.hpp>
#include <wsd/COOLWSD.hpp>
#include <wsd/Exceptions.hpp>

#include <chrono>
#include <cmath>
#include <csignal>
#include <cstdint>
#include <limits>
#include <memory>
#include <set>
#include <sstream>
#include <string>

#include <fnmatch.h>
#include <dirent.h>

namespace
{
std::ostream& print(std::ostream& oss, const std::string_view prefix, const std::string_view name,
                    const std::string_view suffix)
{
    oss << prefix << (prefix.empty() ? "" : "_") << name << (suffix.empty() ? "" : "_") << suffix;
    return oss;
}

std::string concat(const std::string_view prefix, const std::string_view name,
                   const std::string_view suffix)
{
    std::ostringstream oss;
    print(oss, prefix, name, suffix);
    return oss.str();
}

} // namespace

void AdminDocument::addView(const std::string& sessionId, const std::string& userName,
                       const std::string& userId, bool readOnly)
{
    const auto ret = _views.emplace(sessionId, View(sessionId, userName, userId, readOnly));
    if (!ret.second)
    {
        LOG_WRN("View with SessionID [" << sessionId << "] already exists.");
    }
    else
    {
        ++_activeViews;
    }
}

int AdminDocument::expireView(const std::string& sessionId)
{
    auto it = _views.find(sessionId);
    if (it != _views.end())
    {
        it->second.expire();

        // If last view, expire the Document also
        if (--_activeViews == 0)
            _end = std::time(nullptr);
    }
    takeSnapshot();

    return _activeViews;
}

void AdminDocument::setViewLoadDuration(const std::string& sessionId, std::chrono::milliseconds viewLoadDuration)
{
    std::map<std::string, View>::iterator it = _views.find(sessionId);
    if (it != _views.end())
        it->second.setLoadDuration(viewLoadDuration);
}

std::string AdminDocument::getSnapshot(std::time_t now) const
{
    std::ostringstream oss;
    oss << '{';
    oss << "\"creationTime\"" << ':' << now << ',';
    oss << "\"memoryDirty\"" << ':' << getMemoryDirty() << ',';
    oss << "\"activeViews\"" << ':' << getActiveViews() << ',';

    oss << "\"views\"" << ":[";
    std::string separator;
    for (const auto& view : getViews())
    {
        oss << separator << '"';
        if(view.second.isExpired())
        {
            oss << '-';
        }
        oss << view.first << '"';
        separator = ",";
    }
    oss << "],";

    oss << "\"lastActivity\"" << ':' << _lastActivity;
    oss << '}';
    return oss.str();
}

const std::string AdminDocument::getHistory() const
{
    std::ostringstream oss;
    oss << "{";
    oss << "\"docKey\"" << ":\"" << _docKey << "\",";
    oss << "\"filename\"" << ":\"" << COOLWSD::anonymizeUrl(getFilename()) << "\",";
    oss << "\"start\"" << ':' << _start << ',';
    oss << "\"end\"" << ':' << _end << ',';
    oss << "\"pid\"" << ':' << getPid() << ',';
    oss << "\"snapshots\"" << ":[";
    std::string separator;
    for (const auto& s : _snapshots)
    {
        oss << separator << s;
        separator = ",";
    }
    oss << "]}";
    return oss.str();
}

void AdminDocument::takeSnapshot()
{
    std::time_t now = std::time(nullptr);
    if (now == _lastSnapshotTime)
        return;
    _snapshots.push_back(getSnapshot(now));
    _lastSnapshotTime = now;
}

std::string AdminDocument::to_string() const
{
    std::ostringstream oss;
    std::string encodedFilename;
    Poco::URI::encode(getFilename(), " ", encodedFilename);
    oss << getPid() << ' '
        << encodedFilename << ' '
        << getActiveViews() << ' '
        << getMemoryDirty() << ' '
        << getElapsedTime() << ' '
        << getIdleTime() << ' ';
    return oss.str();
}

void AdminDocument::updateMemoryDirty()
{
    // Avoid accessing smaps too often
    const time_t now = std::time(nullptr);
    if (now - _lastTimeSMapsRead >= 5)
    {
        size_t lastMemDirty = _memoryDirty;
        auto procSMaps = _procSMaps.lock();
        _memoryDirty = procSMaps ? ProcUtil::getPssAndDirtyFromSMaps(procSMaps.get()).second : 0;
        _lastTimeSMapsRead = now;
        if (lastMemDirty != _memoryDirty)
            _hasMemDirtyChanged = true;
    }
}

void AdminDocument::setLastJiffies(size_t newJ)
{
    const auto now = std::chrono::steady_clock::now();
    auto sinceMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - _lastJiffyTime).count();
    if (_lastJiffy && sinceMs > 0)
        _lastCpuPercentage = (100 * 1000 * (newJ - _lastJiffy) / ::sysconf(_SC_CLK_TCK)) / sinceMs;
    _lastJiffy = newJ;
    _lastJiffyTime = now;
}

bool Subscriber::notify(const std::string& message)
{
    // If there is no socket, then return false to
    // signify we're disconnected.
    std::shared_ptr<WebSocketHandler> webSocket = _ws.lock();
    if (webSocket)
    {
        if (_subscriptions.find(COOLProtocol::getFirstToken(message)) == _subscriptions.end())
        {
            // No subscribers for the given message.
            return true;
        }

        try
        {
            UNITWSD_CALL(onAdminNotifyMessage(message));
            webSocket->sendTextMessage(message);
            return true;
        }
        catch (const std::exception& ex)
        {
            LOG_ERR("Failed to notify Admin subscriber with message [" <<
                    message << "] due to [" << ex.what() << "].");
        }
    }

    return false;
}

bool Subscriber::subscribe(const std::string& command)
{
    auto ret = _subscriptions.insert(command);
    return ret.second;
}

void Subscriber::unsubscribe(const std::string& command)
{
    _subscriptions.erase(command);
}

AdminModel::~AdminModel()
{
    LOG_TRC("History:\n\n" << getAllHistory() << '\n');
    LOG_INF("AdminModel dtor.");
}

std::string AdminModel::getAllHistory() const
{
    std::ostringstream oss;
    oss << "{ \"documents\" : [";
    std::string separator1;
    for (const auto& d : _documents)
    {
        oss << separator1;
        oss << d.second.getHistory();
        separator1 = ",";
    }

    oss << "], \"expiredDocuments\" : [";

    long count = 0;
    for (const std::string& history : _expiredDocumentsHistories)
    {
        oss << (count ? "," : "") << history;
        ++count;
    }

    oss << "]}";
    return oss.str();
}

std::string AdminModel::query(const std::string& command)
{
    ASSERT_CORRECT_THREAD_OWNER(_owner);

    const auto token = COOLProtocol::getFirstToken(command);
    if (token == "documents")
    {
        return getDocuments();
    }
    else if (token == "active_users_count")
    {
        return std::to_string(getTotalActiveViews());
    }
    else if (token == "active_docs_count")
    {
        return std::to_string(_documents.size());
    }
    else if (token == "mem_stats")
    {
        return getMemStats();
    }
    else if (token == "mem_stats_size")
    {
        return std::to_string(_memStatsSize);
    }
    else if (token == "cpu_stats")
    {
        return getCpuStats();
    }
    else if (token == "cpu_stats_size")
    {
        return std::to_string(_cpuStatsSize);
    }
    else if (token == "sent_activity")
    {
        return getSentActivity();
    }
    else if (token == "recv_activity")
    {
        return getRecvActivity();
    }
    else if (token == "net_stats_size")
    {
        return std::to_string(std::max(_sentStatsSize, _recvStatsSize));
    }
    else if (token == "connection_activity")
    {
        return getConnectionActivity();
    }
    else if (token == "connection_stats_size")
    {
        return std::to_string(_connStatsSize);
    }

    return std::string("");
}

/// Returns memory consumed by all active coolkit processes
unsigned AdminModel::getKitsMemoryUsage() const
{
    ASSERT_CORRECT_THREAD_OWNER(_owner);

    unsigned totalMem = 0;
    unsigned docs = 0;
    for (const auto& it : _documents)
    {
        if (!it.second.isExpired())
        {
            const int bytes = it.second.getMemoryDirty();
            if (bytes > 0)
            {
                totalMem += bytes;
                ++docs;
            }
        }
    }

    if (docs > 0)
    {
        LOGA_TRC(Admin, "Got total Kits memory of " << totalMem << " bytes for " << docs <<
                 " docs, avg: " << static_cast<double>(totalMem) / docs << " bytes / doc.");
    }

    return totalMem;
}

size_t AdminModel::getKitsJiffies() const
{
    ASSERT_CORRECT_THREAD_OWNER(_owner);

    size_t totalJ = 0;
    for (auto& it : _documents)
    {
        if (!it.second.isExpired())
        {
            const int pid = it.second.getPid();
            if (pid > 0)
            {
                unsigned newJ = ProcUtil::getCpuUsage(pid);
                unsigned prevJ = it.second.getLastJiffies();
                if(newJ >= prevJ)
                {
                    totalJ += (newJ - prevJ);
                    const_cast<AdminDocument&>(it.second).setLastJiffies(newJ);
                }
            }
        }
    }
    return totalJ;
}

void AdminModel::subscribe(int sessionId, const std::weak_ptr<WebSocketHandler>& ws)
{
    ASSERT_CORRECT_THREAD_OWNER(_owner);

    const auto ret = _subscribers.emplace(sessionId, Subscriber(ws));
    if (!ret.second)
    {
        LOG_WRN("Subscriber already exists");
    }
}

void AdminModel::subscribe(int sessionId, const std::string& command)
{
    ASSERT_CORRECT_THREAD_OWNER(_owner);

    auto subscriber = _subscribers.find(sessionId);
    if (subscriber != _subscribers.end())
    {
        subscriber->second.subscribe(command);
    }
}

void AdminModel::unsubscribe(int sessionId, const std::string& command)
{
    ASSERT_CORRECT_THREAD_OWNER(_owner);

    auto subscriber = _subscribers.find(sessionId);
    if (subscriber != _subscribers.end())
        subscriber->second.unsubscribe(command);
}

void AdminModel::addMemStats(unsigned memUsage)
{
    ASSERT_CORRECT_THREAD_OWNER(_owner);

    _memStats.push_back(memUsage);
    if (_memStats.size() > _memStatsSize)
        _memStats.pop_front();

    notify("mem_stats " + std::to_string(memUsage));
}

void AdminModel::addCpuStats(unsigned cpuUsage)
{
    ASSERT_CORRECT_THREAD_OWNER(_owner);

    _cpuStats.push_back(cpuUsage);
    if (_cpuStats.size() > _cpuStatsSize)
        _cpuStats.pop_front();

    notify("cpu_stats " + std::to_string(cpuUsage));
}

void AdminModel::addSentStats(uint64_t sent)
{
    ASSERT_CORRECT_THREAD_OWNER(_owner);

    _sentStats.push_back(sent);
    if (_sentStats.size() > _sentStatsSize)
        _sentStats.pop_front();

    notify("sent_activity " + std::to_string(sent));
}

void AdminModel::addRecvStats(uint64_t recv)
{
    ASSERT_CORRECT_THREAD_OWNER(_owner);

    _recvStats.push_back(recv);
    if (_recvStats.size() > _recvStatsSize)
        _recvStats.pop_front();

    notify("recv_activity " + std::to_string(recv));
}

void AdminModel::addConnectionStats(size_t connections)
{
    ASSERT_CORRECT_THREAD_OWNER(_owner);

    _connStats.push_back(connections);
    if (_connStats.size() > _connStatsSize)
        _connStats.pop_front();

    notify("connection_activity " + std::to_string(connections));
}

void AdminModel::setCpuStatsSize(unsigned size)
{
    ASSERT_CORRECT_THREAD_OWNER(_owner);

    int wasteValuesLen = _cpuStats.size() - size;
    while (wasteValuesLen-- > 0)
    {
        if (_cpuStats.empty())
        {
            break;
        }

        _cpuStats.pop_front();
    }
    _cpuStatsSize = size;

    notify("settings cpu_stats_size=" + std::to_string(_cpuStatsSize));
}

void AdminModel::setMemStatsSize(unsigned size)
{
    ASSERT_CORRECT_THREAD_OWNER(_owner);

    int wasteValuesLen = _memStats.size() - size;
    while (wasteValuesLen-- > 0)
    {
        if (_memStats.empty())
        {
            break;
        }

        _memStats.pop_front();
    }
    _memStatsSize = size;

    notify("settings mem_stats_size=" + std::to_string(_memStatsSize));
}

void AdminModel::notify(const std::string& message)
{
    ASSERT_CORRECT_THREAD_OWNER(_owner);

    if (!_subscribers.empty())
    {
        LOG_TRC("Message to admin console: " << message);
        for (auto it = std::begin(_subscribers); it != std::end(_subscribers); )
        {
            if (!it->second.notify(message))
            {
                LOG_INF("Failed to notify admin [" << it->first << "]; unsubscribing");
                it = _subscribers.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }
}

void AdminModel::addBytes(const std::string& docKey, uint64_t sent, uint64_t recv)
{
    ASSERT_CORRECT_THREAD_OWNER(_owner);

    auto doc = _documents.find(docKey);
    if(doc != _documents.end())
        doc->second.addBytes(sent, recv);

    _sentBytesTotal += sent;
    _recvBytesTotal += recv;
}

void AdminModel::modificationAlert(const std::string& docKey, pid_t pid, bool value)
{
    ASSERT_CORRECT_THREAD_OWNER(_owner);

    auto doc = _documents.find(docKey);
    if (doc != _documents.end())
        doc->second.setModified(value);

    std::ostringstream oss;
    oss << "modifications "
        << pid << ' '
        << (value?"Yes":"No");

    notify(oss.str());
}

void AdminModel::uploadedAlert(const std::string& docKey, pid_t pid, bool value)
{
    ASSERT_CORRECT_THREAD_OWNER(_owner);

    auto doc = _documents.find(docKey);
    if (doc != _documents.end())
        doc->second.setUploaded(value);

    std::ostringstream oss;
    oss << "uploaded " << pid << ' ' << (value ? "Yes" : "No");
    notify(oss.str());
}

void AdminModel::addDocument(const std::string& docKey, pid_t pid,
                             const std::string& filename, const std::string& sessionId,
                             const std::string& userName, const std::string& userId,
                             const std::weak_ptr<FILE>& smapsFp, const Poco::URI& wopiSrc, bool isViewReadOnly)
{
    ASSERT_CORRECT_THREAD_OWNER(_owner);
    const auto ret =
        _documents.emplace(docKey, AdminDocument(docKey, pid, filename, wopiSrc));
    ret.first->second.setProcSMapsFp(smapsFp);
    ret.first->second.takeSnapshot();
    ret.first->second.addView(sessionId, userName, userId, isViewReadOnly);
    LOG_DBG("Added admin document [" << docKey << "].");

    std::string memoryAllocated;
    std::string encodedUsername;
    std::string encodedFilename;
    std::string encodedUserId;
    Poco::URI::encode(userId, " ", encodedUserId);
    Poco::URI::encode(filename, " ", encodedFilename);
    Poco::URI::encode(userName, " ", encodedUsername);

    // Notify the subscribers
    std::ostringstream oss;
    oss << "adddoc "
        << pid << ' '
        << encodedFilename << ' '
        << sessionId << ' '
        << encodedUsername << ' '
        << encodedUserId << ' ';

    // We have to wait until the kit sends us its PSS.
    // Here we guestimate until we get an update.
    if (_documents.size() < 2) // If we aren't the only one.
    {
        if (_memStats.empty())
        {
            memoryAllocated = "0";
        }
        else
        {
            // Estimate half as much as wsd+forkit.
            memoryAllocated = std::to_string(_memStats.front() / 2);
        }
    }
    else
    {
        memoryAllocated = std::to_string(_documents.begin()->second.getMemoryDirty());
    }

    const std::string& wopiHost = wopiSrc.getHost();
    oss << memoryAllocated << ' ' << wopiHost << ' ' << isViewReadOnly << ' ' << wopiSrc.toString()
        << ' ' << docKey;

    CONFIG_STATIC const bool log = ConfigUtil::getConfigValue<bool>("logging.docstats", false);
    if (log)
    {
        LOG_ANY("docstats : adding a document : "
                << filename << ", created by : " << COOLWSD::anonymizeUsername(userName)
                << ", using WopiHost : " << COOLWSD::anonymizeUrl(wopiHost)
                << ", allocating memory of : " << memoryAllocated);
    }
    notify(oss.str());
}

void AdminModel::removeDocument(const std::string& docKey, const std::string& sessionId)
{
    ASSERT_CORRECT_THREAD_OWNER(_owner);

    auto docIt = _documents.find(docKey);
    if (docIt != _documents.end() && !docIt->second.isExpired())
    {
        // Notify the subscribers
        std::ostringstream oss;
        oss << "rmdoc "
            << docIt->second.getPid() << ' '
            << sessionId;
        notify(oss.str());

        // The idea is to only expire the document and keep the history
        // of documents open and close, to be able to give a detailed summary
        // to the admin console with views.
        if (docIt->second.expireView(sessionId) == 0)
            doRemove(docIt);
    }
}

void AdminModel::removeDocument(const std::string& docKey)
{
    ASSERT_CORRECT_THREAD_OWNER(_owner);

    auto docIt = _documents.find(docKey);
    if (docIt != _documents.end())
    {
        std::ostringstream oss;
        oss << "rmdoc "
            << docIt->second.getPid() << ' ';
        const std::string msg = oss.str();

        for (const auto& pair : docIt->second.getViews())
        {
            // Notify the subscribers
            notify(msg + pair.first);
            docIt->second.expireView(pair.first);
        }

        LOG_DBG("Removed admin document [" << docKey << "].");
        doRemove(docIt);
    }
}

std::string AdminModel::getMemStats() const
{
    ASSERT_CORRECT_THREAD_OWNER(_owner);

    std::ostringstream oss;
    for (const auto& i: _memStats)
    {
        oss << i << ',';
    }

    return oss.str();
}

std::string AdminModel::getCpuStats() const
{
    ASSERT_CORRECT_THREAD_OWNER(_owner);

    std::ostringstream oss;
    for (const auto& i: _cpuStats)
    {
        oss << i << ',';
    }

    return oss.str();
}

std::string AdminModel::getSentActivity() const
{
    ASSERT_CORRECT_THREAD_OWNER(_owner);

    std::ostringstream oss;
    for (const auto& i: _sentStats)
    {
        oss << i << ',';
    }

    return oss.str();
}

std::string AdminModel::getRecvActivity() const
{
    ASSERT_CORRECT_THREAD_OWNER(_owner);

    std::ostringstream oss;
    for (const auto& i: _recvStats)
    {
        oss << i << ',';
    }

    return oss.str();
}

std::string AdminModel::getConnectionActivity() const
{
    ASSERT_CORRECT_THREAD_OWNER(_owner);

    std::ostringstream oss;
    for (const auto& i: _connStats)
    {
        oss << i << ',';
    }

    return oss.str();
}

unsigned AdminModel::getTotalActiveViews() const
{
    ASSERT_CORRECT_THREAD_OWNER(_owner);

    unsigned numTotalViews = 0;
    for (const auto& it: _documents)
    {
        if (!it.second.isExpired())
        {
            numTotalViews += it.second.getActiveViews();
        }
    }

    return numTotalViews;
}

std::vector<DocBasicInfo> AdminModel::getDocumentsSortedByIdle() const
{
    ASSERT_CORRECT_THREAD_OWNER(_owner);

    std::vector<DocBasicInfo> docs;
    docs.reserve(_documents.size());
    for (const auto& it: _documents)
    {
        docs.emplace_back(it.second.getDocKey(),
                          it.second.getIdleTime(),
                          it.second.getMemoryDirty(),
                          !it.second.getModifiedStatus());
    }

    // Sort the list by idle times;
    std::sort(std::begin(docs), std::end(docs),
              [](const DocBasicInfo& a, const DocBasicInfo& b)
              {
                return a.getIdleTime() > b.getIdleTime();
              });

    return docs;
}

void AdminModel::cleanupResourceConsumingDocs()
{
    ASSERT_CORRECT_THREAD_OWNER(_owner);

    DocCleanupSettings& settings = _defDocProcSettings.getCleanupSettings();

    for (auto& it: _documents)
    {
        AdminDocument& doc = it.second;
        if (!doc.isExpired())
        {
            size_t idleTime = doc.getIdleTime();
            size_t memDirty = doc.getMemoryDirty();
            unsigned cpuPercentage = doc.getLastCpuPercentage();

            if (idleTime >= settings.getIdleTime() &&
                (memDirty >= settings.getLimitDirtyMem() * 1024 ||
                 cpuPercentage >= settings.getLimitCpu()))
            {
                time_t now = std::time(nullptr);
                const size_t badBehaviorDuration = now - doc.getBadBehaviorDetectionTime();
                if (!doc.getBadBehaviorDetectionTime())
                {
                    LOG_WRN("Detected resource consuming doc [" << doc.getDocKey() << "]: idle="
                            << idleTime << " s, memory=" << memDirty << " KB, CPU=" << cpuPercentage << "%.");
                    doc.setBadBehaviorDetectionTime(now);
                }
                else if (badBehaviorDuration >= settings.getBadBehaviorPeriod())
                {
                    // We should not try to close it nicely (closeDocument) because
                    // we could lose it: it will be removed from our internal lists
                    // but the process itself can hang and continue to exist and
                    // consume resources.
                    // Also, try first to SIGABRT the kit process so that a stack trace
                    // could be dumped. If the process is still alive then, at next
                    // iteration, try to SIGKILL it.
                    if (SigUtil::killChild(doc.getPid(), doc.getAbortTime() ? SIGKILL : SIGABRT))
                        LOG_ERR((doc.getAbortTime() ? "Killed" : "Aborted") << " resource consuming doc [" << doc.getDocKey() << "]");
                    else
                        LOG_ERR("Cannot " << (doc.getAbortTime() ? "kill" : "abort") << " resource consuming doc [" << doc.getDocKey() << "]");
                    if (!doc.getAbortTime())
                        doc.setAbortTime(std::time(nullptr));
                }
            }
            else if (doc.getBadBehaviorDetectionTime())
            {
                doc.setBadBehaviorDetectionTime(0);
                LOG_WRN("Removed doc [" << doc.getDocKey() << "] from resource consuming monitoring list: idle="
                        << idleTime << " s, memory=" << memDirty << " KB, CPU=" << cpuPercentage << "%.");
            }
        }
    }
}

std::string AdminModel::getDocuments() const
{
    ASSERT_CORRECT_THREAD_OWNER(_owner);

    std::ostringstream oss;
    oss << '{' << "\"documents\"" << ':' << '[';
    std::string separator1;
    for (const auto& it: _documents)
    {
        if (!it.second.isExpired())
        {
            std::string encodedFilename;
            Poco::URI::encode(it.second.getFilename(), " ", encodedFilename); // Is encoded name needed?
            oss << separator1 << '{' << ' '
                << "\"pid\"" << ':' << it.second.getPid() << ','
                << "\"docKey\"" << ':' << '"' << it.second.getDocKey() << '"' << ','
                << "\"fileName\"" << ':' << '"' << encodedFilename << '"' << ','
                << "\"wopiHost\"" << ':' << '"' << it.second.getHostName() << '"' << ','
                << "\"activeViews\"" << ':' << it.second.getActiveViews() << ','
                << "\"memory\"" << ':' << it.second.getMemoryDirty() << ','
                << "\"elapsedTime\"" << ':' << it.second.getElapsedTime() << ','
                << "\"idleTime\"" << ':' << it.second.getIdleTime() << ','
                << "\"modified\"" << ':' << '"' << (it.second.getModifiedStatus() ? "Yes" : "No") << '"' << ','
                << "\"uploaded\"" << ':' << '"' << (it.second.getUploadedStatus() ? "Yes" : "No") << '"' << ','
                << "\"wopiSrc\"" << ':' << '"' << it.second.getWopiSrc() << '"' << ','
                << "\"views\"" << ':' << '[';
            std::map<std::string, View> viewers = it.second.getViews();
            std::string separator;
            for(const auto& viewIt: viewers)
            {
                if(!viewIt.second.isExpired()) {
                    oss << separator << '{'
                        << "\"userName\"" << ':' << '"' << viewIt.second.getUserName() << '"' << ','
                        << "\"userId\"" << ':' << '"' << viewIt.second.getUserId() << '"' << ','
                        << "\"sessionid\"" << ':' << '"' << viewIt.second.getSessionId() << '"' << ','
                        << "\"readonly\"" << ':' << '"' << viewIt.second.isReadOnly() << '"' << '}';
                        separator = ',';
                }
            }
            oss << ']'
                << '}';
            separator1 = ',';
        }
    }

    oss << ']' << '}';
    return oss.str();
}

void AdminModel::updateLastActivityTime(const std::string& docKey)
{
    ASSERT_CORRECT_THREAD_OWNER(_owner);

    _lastActivity = std::time(nullptr);

    auto docIt = _documents.find(docKey);
    if (docIt != _documents.end())
    {
        if (docIt->second.getIdleTime() >= 10)
        {
            docIt->second.takeSnapshot(); // I would like to keep the idle time
            docIt->second.updateLastActivityTime(_lastActivity);
            notify("resetidle " + std::to_string(docIt->second.getPid()));
        }
    }
}

double AdminModel::getServerUptimeSecs()
{
    const auto currentTime = std::chrono::steady_clock::now();
    const std::chrono::milliseconds uptime
        = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - COOLWSD::StartTime);
    return uptime.count() / 1000.0; // Convert to seconds and fractions.
}

void AdminModel::setViewLoadDuration(const std::string& docKey, const std::string& sessionId, std::chrono::milliseconds viewLoadDuration)
{
    auto it = _documents.find(docKey);
    if (it != _documents.end())
        it->second.setViewLoadDuration(sessionId, viewLoadDuration);
}

void AdminModel::setDocWopiDownloadDuration(const std::string& docKey, std::chrono::milliseconds wopiDownloadDuration)
{
    auto it = _documents.find(docKey);
    if (it != _documents.end())
        it->second.setWopiDownloadDuration(wopiDownloadDuration);
}

void AdminModel::setDocWopiUploadDuration(const std::string& docKey, const std::chrono::milliseconds wopiUploadDuration)
{
    auto it = _documents.find(docKey);
    if (it != _documents.end())
        it->second.setWopiUploadDuration(wopiUploadDuration);
}

void AdminModel::addErrorExitCounters(unsigned segFaultCount, unsigned killedCount,
                                      unsigned oomKilledCount)
{
    _segFaultCount += segFaultCount;
    _killedCount += killedCount;
    _oomKilledCount += oomKilledCount;
}

void AdminModel::addLostKitsTerminated(unsigned lostKitsTerminated)
{
    _lostKitsTerminatedCount += lostKitsTerminated;
}

int filterNumberName(const struct dirent *dir)
{
    return !fnmatch("[0-9]*", dir->d_name, 0);
}

// coverity[ -taint_source : arg-1 ] 2024.6.1
int AdminModel::getPidsFromProcName(const std::regex& procNameRegEx, std::vector<int> *pids)
{
    struct dirent **namelist = NULL;
    int n = scandir("/proc", &namelist, filterNumberName, 0);
    int pidCount = 0;

    if (n < 0)
        return n;

    std::string comm;
    char line[256] = { 0 }; //Here we need only 16 bytes but for safety reasons we use file name max length

    while (n--)
    {
        comm = "/proc/";
        comm += namelist[n]->d_name;
        comm += "/comm";
        FILE* fp = fopen(comm.c_str(), "r");
        if (fp != nullptr)
        {
            if (fgets(line, sizeof (line), fp))
            {
                char *nl = strchr(line, '\n');
                if (nl != NULL)
                    *nl = 0;
                if (regex_match(line, procNameRegEx))
                {
                    pidCount ++;
                    if (pids)
                        pids->push_back(strtol(namelist[n]->d_name, NULL, 10));
                }
            }
            fclose(fp);
        }
        free(namelist[n]);
    }
    free(namelist);

    return pidCount;
}

int AdminModel::getAssignedKitPids(std::vector<int> *pids)
{
    return getPidsFromProcName(std::regex("kitbroker_.*"), pids);
}

int AdminModel::getUnassignedKitPids(std::vector<int> *pids)
{
    return getPidsFromProcName(std::regex("kit_spare_.*"), pids);
}

void AdminModel::getKitPidsFromSystem(std::vector<int> *pids)
{
    getAssignedKitPids(pids);
    getUnassignedKitPids(pids);
}

class AggregateStats final
{
public:
    AggregateStats()
        : _total(0)
        , _min(std::numeric_limits<uint64_t>::max())
        , _max(0)
        , _count(0)
    {}

    void Update(uint64_t value)
    {
        _total += value;
        _min = (_min > value ? value : _min);
        _max = (_max < value ? value : _max);
        _count ++;
    }

    uint64_t getIntAverage() const { return _count ? std::round(_total / (double)_count) : 0; }
    uint64_t getMin() const { return _count == 0 ? 0 : _min; }
    uint64_t getMax() const { return _max; }
    uint64_t getTotal() const { return _total; }
    uint64_t getCount() const { return _count; }

    void Print(std::ostream& oss, const std::string_view prefix, const std::string_view unit) const
    {
        print(oss, prefix, "total", unit) << ' ' << _total << '\n';
        print(oss, prefix, "average", unit) << ' ' << getIntAverage() << '\n';
        print(oss, prefix, "min", unit) << ' ' << getMin() << '\n';
        print(oss, prefix, "max", unit) << ' ' << getMax() << '\n';
    }

private:
    uint64_t _total;
    uint64_t _min;
    uint64_t _max;
    uint64_t _count; ///< The number of samples. We are 8-byte aligned, so make this 64-bits.
};

class ActiveExpiredStats final
{
public:
    const AggregateStats& active() const { return _active; }

    void Update(uint64_t value, bool active)
    {
        _all.Update(value);
        if (active)
            _active.Update(value);
        else
            _expired.Update(value);
    }

    void Print(std::ostream& oss, const char* prefix, const char* name, const char* unit) const
    {
        _all.Print(oss, concat(prefix, "all", name), unit);
        _active.Print(oss, concat(prefix, "active", name), unit);
        _expired.Print(oss, concat(prefix, "expired", name), unit);
    }

private:
    AggregateStats _all;
    AggregateStats _active;
    AggregateStats _expired;
};

struct DocumentAggregateStats final
{
    DocumentAggregateStats()
    : _resConsCount(0), _resConsAbortCount(0), _resConsAbortPendingCount(0)
    {}

    void Update(const AdminDocument &d, bool active)
    {
        _kitUsedMemory.Update(d.getMemoryDirty() * 1024, active);
        _viewsCount.Update(d.getViews().size(), active);
        _activeViewsCount.Update(d.getActiveViews(), active);
        _expiredViewsCount.Update(d.getViews().size() - d.getActiveViews(), active);
        _openedTime.Update(d.getOpenTime(), active);
        _bytesSentToClients.Update(d.getSentBytes(), active);
        _bytesRecvFromClients.Update(d.getRecvBytes(), active);
        _wopiDownloadDuration.Update(d.getWopiDownloadDuration().count(), active);
        _wopiUploadDuration.Update(d.getWopiUploadDuration().count(), active);

        //View load duration
        for (const auto& v : d.getViews())
            _viewLoadDuration.Update(v.second.getLoadDuration().count(), active);

        if (d.getBadBehaviorDetectionTime())
        {
            if (active)
                _resConsCount ++;
        }
        if (d.getAbortTime())
        {
            if (active)
                _resConsAbortPendingCount ++;
            else
                _resConsAbortCount ++;
        }
    }

    ActiveExpiredStats _kitUsedMemory;
    ActiveExpiredStats _viewsCount;
    ActiveExpiredStats _activeViewsCount;
    ActiveExpiredStats _expiredViewsCount;
    ActiveExpiredStats _openedTime;
    ActiveExpiredStats _bytesSentToClients;
    ActiveExpiredStats _bytesRecvFromClients;
    ActiveExpiredStats _wopiDownloadDuration;
    ActiveExpiredStats _wopiUploadDuration;
    ActiveExpiredStats _viewLoadDuration;

    int _resConsCount;
    int _resConsAbortCount;
    int _resConsAbortPendingCount;
};

struct KitProcStats
{
    void UpdateAggregateStats(int pid)
    {
        _threadCount.Update(ProcUtil::getStatFromPid(pid, 19));
        _cpuTime.Update(ProcUtil::getCpuUsage(pid) / sysconf (_SC_CLK_TCK));
    }

    int unassignedCount;
    int assignedCount;
    AggregateStats _threadCount;
    AggregateStats _cpuTime;
};

/// The aggregate stats of expired documents.
/// Since expired documents don't change their stats,
/// we don't need to keep the Document instances around.
static DocumentAggregateStats ExpiredDocStats;

void AdminModel::doRemove(std::map<std::string, AdminDocument>::iterator& docIt)
{
    ASSERT_CORRECT_THREAD_OWNER(_owner);

    // don't send the routing_rmdoc if document is migrating
    if (getCurrentMigDoc() != docIt->first)
    {
        std::ostringstream ostream;
        ostream << "routing_rmdoc " << docIt->second.getWopiSrc();
        notify(ostream.str());
    }
    else
    {
        resetMigratingInfo();
    }

    // Update the expired-documents' stats.
    ExpiredDocStats.Update(docIt->second, false);

    // Serialize the history of the expired document.
    _expiredDocumentsHistories.emplace_back(docIt->second.getHistory());

    // We have no need for the document anymore.
    _documents.erase(docIt);
}

void AdminModel::CalcDocAggregateStats(DocumentAggregateStats& stats) const
{
    stats = ExpiredDocStats;

    for (const auto& d : _documents)
        stats.Update(d.second, true);
}

void CalcKitStats(KitProcStats& stats)
{
    std::vector<int> childProcs;
    stats.unassignedCount = AdminModel::getUnassignedKitPids(&childProcs);
    stats.assignedCount = AdminModel::getAssignedKitPids(&childProcs);
    for (int pid : childProcs)
    {
        stats.UpdateAggregateStats(pid);
    }
}

void PrintDocActExpMetrics(std::ostream& oss, const char* name, const char* unit,
                           const ActiveExpiredStats& values)
{
    values.Print(oss, "document", name, unit);
}

void PrintKitAggregateMetrics(std::ostream& oss, const char* name, const char* unit,
                              const AggregateStats& values)
{
    const std::string prefix = std::string("kit_") + name;
    values.Print(oss, prefix, unit);
}

void AdminModel::getMetrics(std::ostream& oss) const
{
    ASSERT_CORRECT_THREAD_OWNER(_owner);

    oss << "coolwsd_count " << getPidsFromProcName(std::regex("coolwsd"), nullptr) << '\n';
    oss << "coolwsd_thread_count " << ProcUtil::getStatFromPid(ProcUtil::getProcessId(), 19)
        << '\n';
    oss << "coolwsd_cpu_time_seconds "
        << ProcUtil::getCpuUsage(ProcUtil::getProcessId()) / sysconf(_SC_CLK_TCK) << '\n';
    oss << "coolwsd_memory_used_bytes " << ProcUtil::getMemoryUsagePSS(ProcUtil::getProcessId()) * 1024
        << '\n';
    oss << "coolwsd_tcp_connections_used " << StreamSocket::getExternalConnectionCount() << '\n';
    oss << '\n';

    oss << "forkit_count " << getPidsFromProcName(std::regex("forkit"), nullptr) << '\n';
    oss << "forkit_thread_count " << ProcUtil::getStatFromPid(_forKitPid, 19) << '\n';
    oss << "forkit_cpu_time_seconds " << ProcUtil::getCpuUsage(_forKitPid) / sysconf (_SC_CLK_TCK) << '\n';
    oss << "forkit_memory_used_bytes " << ProcUtil::getMemoryUsageRSS(_forKitPid) * 1024 << '\n';
    oss << '\n';

    DocumentAggregateStats docStats;
    KitProcStats kitStats;

    CalcDocAggregateStats(docStats);
    CalcKitStats(kitStats);

    oss << "kit_count " << kitStats.unassignedCount + kitStats.assignedCount << '\n';
    oss << "kit_unassigned_count " << kitStats.unassignedCount << '\n';
    oss << "kit_assigned_count " << kitStats.assignedCount << '\n';
    oss << "kit_segfault_count " << _segFaultCount << '\n';
    oss << "kit_lost_terminated_count " << _lostKitsTerminatedCount << '\n';
    oss << "kit_killed_count " << _killedCount << '\n';
    oss << "kit_killed_oom_count " << _oomKilledCount << '\n';
    PrintKitAggregateMetrics(oss, "thread_count", "", kitStats._threadCount);
    PrintKitAggregateMetrics(oss, "memory_used", "bytes", docStats._kitUsedMemory.active());
    PrintKitAggregateMetrics(oss, "cpu_time", "seconds", kitStats._cpuTime);
    oss << '\n';

    oss << "document_resource_consuming_count " << docStats._resConsCount << '\n';
    oss << "document_resource_consuming_abort_started_count " << docStats._resConsAbortPendingCount << '\n';
    oss << "document_resource_consuming_aborted_count " << docStats._resConsAbortCount << '\n';
    oss << '\n';

    PrintDocActExpMetrics(oss, "views_all_count", "", docStats._viewsCount);
    docStats._activeViewsCount.active().Print(oss, "document_active_views_active_count", "");
    docStats._expiredViewsCount.active().Print(oss, "document_active_views_expired_count", "");
    oss << '\n';

    PrintDocActExpMetrics(oss, "opened_time", "seconds", docStats._openedTime);
    oss << '\n';
    PrintDocActExpMetrics(oss, "sent_to_clients", "bytes", docStats._bytesSentToClients);
    oss << '\n';
    PrintDocActExpMetrics(oss, "received_from_clients", "bytes", docStats._bytesRecvFromClients);
    oss << '\n';
    PrintDocActExpMetrics(oss, "wopi_upload_duration", "milliseconds", docStats._wopiUploadDuration);
    oss << '\n';
    PrintDocActExpMetrics(oss, "wopi_download_duration", "milliseconds", docStats._wopiDownloadDuration);
    oss << '\n';
    PrintDocActExpMetrics(oss, "view_load_duration", "milliseconds", docStats._viewLoadDuration);

    oss << '\n';
    oss << "error_storage_space_low " << StorageSpaceLowException::count << "\n";
    oss << "error_storage_connection " << StorageConnectionException::count << "\n";
    oss << "error_bad_request " << (BadRequestException::count - BadArgumentException::count) << "\n";
    oss << "error_bad_argument " << BadArgumentException::count << "\n";
    oss << "error_unauthorized_request " << UnauthorizedRequestException::count << "\n";
    oss << "error_service_unavailable " << ServiceUnavailableException::count << "\n";
    oss << "error_parse_error " << ParseError::count << "\n";
    oss << '\n';

    int tick_per_sec = sysconf(_SC_CLK_TCK);
    // dump document data
    for (const auto& it : _documents)
    {
        const AdminDocument &doc = it.second;
        std::string pid = std::to_string(doc.getPid());

        std::string encodedFilename;
        Poco::URI::encode(doc.getFilename(), " ", encodedFilename);
        oss << "doc_info{host=\"" << doc.getHostName() << "\","
               "key=\"" << doc.getDocKey() << "\","
               "filename=\"" << encodedFilename << "\","
               "pid=\"" << pid << "\"} 1\n";

        std::string suffix = "{pid=\"" + pid + "\"} ";
        oss << "doc_views" << suffix << doc.getViews().size() << "\n";
        oss << "doc_views_active" << suffix << doc.getActiveViews() << "\n";
        oss << "doc_is_modified" << suffix << doc.getModifiedStatus() << "\n";
        oss << "doc_memory_used_bytes" << suffix << doc.getMemoryDirty() * 1024 << "\n";
        oss << "doc_cpu_used_seconds" << suffix << ((double)doc.getLastJiffies()/tick_per_sec) << "\n";
        oss << "doc_open_time_seconds" << suffix << doc.getOpenTime() << "\n";
        oss << "doc_idle_time_seconds" << suffix << doc.getIdleTime() << "\n";
        oss << "doc_download_time_seconds" << suffix << ((double)doc.getWopiDownloadDuration().count() / 1000) << "\n";
        oss << "doc_upload_time_seconds" << suffix << ((double)doc.getWopiUploadDuration().count() / 1000) << "\n";
        oss << '\n';
    }
}

std::set<pid_t> AdminModel::getDocumentPids() const
{
    std::set<pid_t> pids;

    for (const auto& it : _documents)
        pids.insert(it.second.getPid());

    return pids;
}

void AdminModel::UpdateMemoryDirty()
{
    for (auto& it: _documents)
    {
        it.second.updateMemoryDirty();
    }
}

void AdminModel::notifyDocsMemDirtyChanged()
{
    for (auto& [name, doc] : _documents)
    {
        if (doc.hasMemDirtyChanged())
        {
            std::ostringstream msg;
            msg << "propchange " << doc.getPid() << " mem " << doc.getMemoryDirty();
            notify(msg.str());
            doc.setMemDirtyChanged(false);
        }
    }
}

bool AdminModel::isDocSaved(const std::string& docKey)
{
    auto doc = _documents.find(docKey);
    if (doc != _documents.end())
        return !doc->second.getModifiedStatus();
    LOG_DBG("cannot find document with docKey " << docKey);
    return false;
}

bool AdminModel::isDocReadOnly(const std::string& docKey)
{
    auto doc = _documents.find(docKey);
    if (doc != _documents.end())
    {
        bool isReadOnly = true;
        for (const auto& view : doc->second.getViews())
        {
            if (!view.second.isReadOnly())
            {
                isReadOnly = false;
                break;
            }
        }
        return isReadOnly;
    }
    LOG_DBG("cannot find document with docKey " << docKey);
    return false;
}

void AdminModel::sendMigrateMsgAfterSave(bool lastSaveSuccessful, const std::string& docKey)
{
    if (getCurrentMigDoc() != docKey)
    {
        return;
    }
    std::ostringstream oss;
    std::string saveSuccessful = lastSaveSuccessful ? "true" : "false";
    oss << "migrate: {";
    oss << "\"afterSave\""
        << ":true,";
    oss << "\"saved\":" << saveSuccessful;
    if (lastSaveSuccessful)
    {
        oss << ',';
        oss << "\"routeToken\"" << ':' << '"' << getCurrentMigToken() << '"' << ',';
        oss << "\"serverId\"" << ':' << '"' << getTargetMigServerId() << '"' << '}';
    }
    else
    {
        oss << '}';
        resetMigratingInfo();
    }
    COOLWSD::alertUserInternal(docKey, oss.str());
}

std::string AdminModel::getWopiSrcMap() const
{
    std::ostringstream oss;
    oss << "wopiSrcMap: {";
    oss << "\"routeToken\": \"" << COOLWSD::RouteToken << "\",";
    oss << "\"wopiSrc\": [";
    size_t count = 0;
    for (const auto& it : _documents)
    {
        if (!it.second.isExpired())
        {
            oss << "\"" << it.second.getWopiSrc() << "\"";
            if (count < _documents.size() - 1)
            {
                oss << ',';
            }
        }
        count++;
    }
    oss << "]}";
    return oss.str();
}

void AdminModel::setMigratingInfo(const std::string& docKey, const std::string& routeToken, const std::string& serverId)
{
    _currentMigDoc = docKey;
    _currentMigToken = routeToken;
    _targetMigServerId = serverId;
}

void AdminModel::resetMigratingInfo()
{
    _currentMigDoc = std::string();
    _currentMigToken = std::string();
    _targetMigServerId = std::string();
}

std::string AdminModel::getFilename(int pid) const
{
    for (const auto& it : _documents)
    {
        if (it.second.getPid() == pid)
        {
            return it.second.getFilename();
        }
    }
    return std::string();
}

void AdminModel::routeTokenSanityCheck()
{
    ASSERT_CORRECT_THREAD_OWNER(_owner);

    std::ostringstream oss;
    oss << "routetoken_sanity_check";
    notify(oss.str());
}

void AdminModel::sendShutdownReceivedMsg()
{
    ASSERT_CORRECT_THREAD_OWNER(_owner);
    notify("shutdown_received");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
