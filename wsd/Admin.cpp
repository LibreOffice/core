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
 * Implementation of administrative interface and websocket handlers.
 * Classes: AdminSocketHandler, MonitorSocketHandler, Admin
 */

#include <config.h>

#include "Admin.hpp"

#include <common/Common.hpp>
#include <common/ConfigUtil.hpp>
#include <common/JsonUtil.hpp>
#include <common/Log.hpp>
#include <common/NumUtil.hpp>
#include <common/Protocol.hpp>
#include <common/SigUtil.hpp>
#include <common/StringVector.hpp>
#include <common/Unit.hpp>
#include <common/Uri.hpp>
#include <common/Util.hpp>
#include <net/Socket.hpp>
#if ENABLE_SSL
#include <net/SslSocket.hpp>
#endif
#include <net/WebSocketHandler.hpp>
#include <wsd/AdminModel.hpp>
#include <wsd/Auth.hpp>
#include <wsd/COOLWSD.hpp>

#include <Poco/Net/HTTPRequest.h>

#include <chrono>
#include <csignal>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>
#include <poll.h>
#include <unistd.h>

using namespace COOLProtocol;

using Poco::Util::Application;

/// Process incoming websocket messages
void AdminSocketHandler::handleMessage(const std::vector<char> &payload)
{
    // FIXME: check fin, code etc.
    const std::string firstLine = getFirstLine(payload.data(), payload.size());
    StringVector tokens(StringVector::tokenize(firstLine, ' '));
    LOG_TRC("Recv: " << firstLine << " tokens " << tokens.size());

    if (tokens.empty())
    {
        LOG_TRC("too few tokens");
        return;
    }

    AdminModel& model = _admin->getModel();

    if (tokens.equals(0, "auth"))
    {
        if (tokens.size() < 2)
        {
            LOG_DBG("Auth command without any token");
            sendTextMessage("InvalidAuthToken");
            shutdown();
            ignoreInput();
            return;
        }
        std::string jwtToken;
        COOLProtocol::getTokenString(tokens[1], "jwt", jwtToken);

        bool decoded = true;
        try
        {
            jwtToken = Uri::decode(jwtToken);
        }
        catch (const Poco::URISyntaxException&)
        {
            decoded = false;
        }
        LOG_INF("Verifying JWT token: " << jwtToken);
        JWTAuth authAgent("admin", "admin", "admin");
        if (decoded && authAgent.verify(jwtToken))
        {
            LOG_TRC("JWT token is valid");
            _isAuthenticated = true;
            return;
        }
        else
        {
            LOG_DBG("Invalid auth token");
            sendTextMessage("InvalidAuthToken");
            shutdown();
            ignoreInput();
            return;
        }
    }

    if (!_isAuthenticated)
    {
        LOG_DBG("Not authenticated - message is '" << firstLine << "' " <<
                tokens.size() << " first: '" << tokens[0] << '\'');
        sendTextMessage("NotAuthenticated");
        shutdown();
        ignoreInput();
        return;
    }
    else if (tokens.equals(0, "documents") ||
             tokens.equals(0, "active_users_count") ||
             tokens.equals(0, "active_docs_count") ||
             tokens.equals(0, "mem_stats") ||
             tokens.equals(0, "cpu_stats") ||
             tokens.equals(0, "sent_activity") ||
             tokens.equals(0, "recv_activity") ||
             tokens.equals(0, "connection_activity"))
    {
        const std::string result = model.query(tokens[0]);
        if (!result.empty())
            sendTextFrame(tokens[0] + ' ' + result);
    }
    else if (tokens.equals(0, "history"))
    {
        sendTextFrame("{ \"History\": " + model.getAllHistory() + '}');
    }
    else if (tokens.equals(0, "version"))
    {
        // Send COOL version information
        std::string timezoneName;
        if (COOLWSD::IndirectionServerEnabled && COOLWSD::GeolocationSetup)
            timezoneName =
                ConfigUtil::getString("indirection_endpoint.geolocation_setup.timezone", "");

        sendTextFrame("coolserver " + Util::getVersionJSON(EnableExperimental, timezoneName));

        // Send LOKit version information
        sendTextFrame("lokitversion " + COOLWSD::LOKitVersion);
    }
    else if (tokens.equals(0, "subscribe") && tokens.size() > 1)
    {
        for (std::size_t i = 0; i < tokens.size() - 1; i++)
        {
            model.subscribe(_sessionId, tokens[i + 1]);
        }
    }
    else if (tokens.equals(0, "unsubscribe") && tokens.size() > 1)
    {
        for (std::size_t i = 0; i < tokens.size() - 1; i++)
        {
            model.unsubscribe(_sessionId, tokens[i + 1]);
        }
    }
    else if (tokens.equals(0, "mem_consumed"))
        sendTextFrame("mem_consumed " + std::to_string(_admin->getTotalMemoryUsage()));

    else if (tokens.equals(0, "total_avail_mem"))
        sendTextFrame("total_avail_mem " + std::to_string(_admin->getTotalAvailableMemory()));

    else if (tokens.equals(0, "sent_bytes"))
        sendTextFrame("sent_bytes " + std::to_string(model.getSentBytesTotal() / 1024));

    else if (tokens.equals(0, "recv_bytes"))
        sendTextFrame("recv_bytes " + std::to_string(model.getRecvBytesTotal() / 1024));

    else if (tokens.equals(0, "uptime"))
        sendTextFrame("uptime " + std::to_string(model.getServerUptimeSecs()));

    else if (tokens.equals(0, "log_lines"))
        sendTextFrame("log_lines " + _admin->getLogLines());

    else if (tokens.equals(0, "kill") && tokens.size() == 2)
    {
        try
        {
            const int pid = NumUtil::stoi(tokens[1]);
            LOG_INF("Admin request to kill PID: " << pid);

            std::set<pid_t> pids = model.getDocumentPids();
            if (pids.find(pid) != pids.end())
            {
                if (Admin::instance().logAdminAction())
                {
                    LOG_ANY("Admin request to kill document ["
                            << COOLWSD::anonymizeUrl(model.getFilename(pid)) << "] with pid ["
                            << pid << "] and source IPAddress [" << _clientIPAdress << ']');
                }
                SigUtil::killChild(pid, SIGKILL);
            }
            else
            {
                LOG_ERR("Invalid PID to kill (not a document pid)");
            }
        }
        catch (std::invalid_argument& exc)
        {
            LOG_ERR("Invalid PID to kill (invalid argument): " << tokens[1]);
        }
        catch (std::out_of_range& exc)
        {
            LOG_ERR("Invalid PID to kill (out of range): " << tokens[1]);
        }
    }
    else if (tokens.equals(0, "settings"))
    {
        // for now, we have only these settings
        std::ostringstream oss;
        oss << "settings "
            << "mem_stats_size=" << model.query("mem_stats_size") << ' '
            << "mem_stats_interval=" << _admin->getMemStatsInterval().count() << ' '
            << "cpu_stats_size="  << model.query("cpu_stats_size") << ' '
            << "cpu_stats_interval=" << _admin->getCpuStatsInterval().count() << ' '
            << "net_stats_size=" << model.query("net_stats_size") << ' '
            << "net_stats_interval=" << _admin->getNetStatsInterval().count() << ' '
            << "connection_stats_size=" << model.query("connection_stats_size") << ' '
            << "global_host_tcp_connections=" << net::Defaults.maxExtConnections << ' ';

        const DocProcSettings& docProcSettings = _admin->getDefDocProcSettings();
        oss << "limit_virt_mem_mb=" << docProcSettings.getLimitVirtMemMb() << ' '
            << "limit_stack_mem_kb=" << docProcSettings.getLimitStackMemKb() << ' '
            << "limit_file_size_mb=" << docProcSettings.getLimitFileSizeMb() << ' '
            << "limit_num_open_files=" << docProcSettings.getLimitNumberOpenFiles() << ' ';

        sendTextFrame(oss.str());
    }
    else if (tokens.equals(0, "channel_list"))
    {
        sendTextFrame("channel_list " + _admin->getChannelLogLevels());
    }
    else if (tokens.equals(0, "shutdown"))
    {
        LOG_INF("Setting ShutdownRequestFlag: Shutdown requested by admin.");
        if (Admin::instance().logAdminAction())
        {
            LOG_ANY("Shutdown requested by admin with source IPAddress [" << _clientIPAdress
                                                                          << ']');
        }
        SigUtil::requestShutdown();
        return;
    }
    else if (tokens.equals(0, "set") && tokens.size() > 1)
    {
        for (size_t i = 1; i < tokens.size(); i++)
        {
            StringVector setting(StringVector::tokenize(tokens[i], '='));
            int settingVal = 0;
            try
            {
                settingVal = NumUtil::stoi(setting[1]);
            }
            catch (const std::exception& exc)
            {
                LOG_ERR("Invalid setting value: " << setting[1] <<
                        " for " << setting[0]);
                return;
            }

            const std::string settingName = setting[0];
            if (settingName == "mem_stats_size")
            {
                if (settingVal != std::stol(model.query(settingName)))
                {
                    model.setMemStatsSize(settingVal);
                }
            }
            else if (settingName == "mem_stats_interval")
            {
                const std::chrono::milliseconds interval{ settingVal };
                if (interval != _admin->getMemStatsInterval())
                {
                    _admin->rescheduleMemTimer(interval);
                    model.clearMemStats();
                    model.notify("settings mem_stats_interval=" + std::to_string(_admin->getMemStatsInterval().count()));
                }
            }
            else if (settingName == "cpu_stats_size")
            {
                if (settingVal != std::stol(model.query(settingName)))
                {
                    model.setCpuStatsSize(settingVal);
                }
            }
            else if (settingName == "cpu_stats_interval")
            {
                const std::chrono::milliseconds interval{ settingVal };
                if (interval != _admin->getCpuStatsInterval())
                {
                    _admin->rescheduleCpuTimer(interval);
                    model.clearCpuStats();
                    model.notify("settings cpu_stats_interval=" + std::to_string(_admin->getCpuStatsInterval().count()));
                }
            }
            else if (COOLProtocol::matchPrefix("limit_", settingName))
            {
                DocProcSettings docProcSettings = _admin->getDefDocProcSettings();
                if (settingName == "limit_virt_mem_mb")
                    docProcSettings.setLimitVirtMemMb(settingVal);
                else if (settingName == "limit_stack_mem_kb")
                    docProcSettings.setLimitStackMemKb(settingVal);
                else if (settingName == "limit_file_size_mb")
                    docProcSettings.setLimitFileSizeMb(settingVal);
                else if (settingName == "limit_num_open_files")
                    docProcSettings.setLimitNumberOpenFiles(settingVal);
                else
                    LOG_ERR("Unknown limit: " << settingName);

                model.notify("settings " + settingName + '=' + std::to_string(settingVal));
                _admin->setDefDocProcSettings(docProcSettings, true);
            }
        }
    }
    else if (tokens.equals(0, "update-log-levels") && tokens.size() > 1)
    {
        for (size_t i = 1; i < tokens.size(); i++)
        {
            StringVector _channel(StringVector::tokenize(tokens[i], '='));
            if (_channel.size() == 2)
            {
                _admin->setChannelLogLevel((_channel[0] != "?" ? _channel[0]: ""), _channel[1]);
            }
        }
        // Let's send back the current log levels in return. So the user can be sure of the values.
        sendTextFrame("channel_list " + _admin->getChannelLogLevels());
    }
    else if (tokens.equals(0, "updateroutetoken") && tokens.size() > 1)
    {
        // parse the json object of serverId to routeToken
        Poco::JSON::Object::Ptr object;
        if (JsonUtil::parseJSON(tokens[1], object))
        {
            std::string routeToken =
                JsonUtil::getJSONValue<std::string>(object, Util::getProcessIdentifier());
            if (!routeToken.empty())
            {
                COOLWSD::alertAllUsersInternal("updateroutetoken " + routeToken);
                COOLWSD::RouteToken = std::move(routeToken);
            }
            else
            {
                LOG_ERR("Failed to update the route token, invalid serverId to routeToken json : "
                        << tokens[1]);
            }
        }
        else
        {
            LOG_ERR("Failed to update the route token, invalid JSON parsing: " << tokens[1]);
        }
    }
    else if (tokens.equals(0, "migrate") && tokens.size() > 1)
    {
        const std::string& docStatus = tokens[1];
        const std::string& dockey = tokens[2];
        const std::string& routeToken = tokens[3];
        const std::string& serverId = tokens[4];
        if (!dockey.empty() && !routeToken.empty() && !serverId.empty())
        {
            model.setMigratingInfo(dockey, routeToken, serverId);
            std::ostringstream oss;
            oss << "migrate: {";
            oss << "\"afterSave\"" << ":false,";
            if (docStatus == "unsaved" && !model.isDocSaved(dockey))
            {
                COOLWSD::autoSave(dockey);
                oss << "\"saved\"" << ":false,";
            }
            else if ((docStatus == "readonly" && model.isDocReadOnly(dockey)) ||
                     (docStatus == "saved" && model.isDocSaved(dockey)))
            {
                oss << "\"saved\"" << ":true,";
            }
            oss << "\"routeToken\"" << ':' << '"' << routeToken << '"' << ',';
            oss << "\"serverId\"" << ':' << '"' << serverId << '"' << '}';
            COOLWSD::alertUserInternal(dockey, oss.str());
            if (SigUtil::getShutdownRequestFlag())
                COOLWSD::setMigrationMsgReceived(dockey);
        }
        else
        {
            LOG_WRN("Document migration failed for dockey:" << dockey <<
                        ", reason has been changed");
        }
    }
    else if (tokens.equals(0, "wopiSrcMap"))
    {
        sendTextFrame(model.getWopiSrcMap());
    }
    else if(tokens.equals(0, "verifyauth"))
    {
        if (tokens.size() < 2)
        {
            LOG_DBG("Auth command without any token");
            sendTextFrame("InvalidAuthToken");
        }
        std::string jwtToken, id;
        COOLProtocol::getTokenString(tokens[1], "jwt", jwtToken);
        COOLProtocol::getTokenString(tokens[2], "id", id);

        try
        {
            jwtToken = Uri::decode(jwtToken);
        }
        catch (const Poco::URISyntaxException& exception)
        {
            LOG_DBG("Invalid URI syntax: " << exception.what());
        }

        LOG_INF("Verifying JWT token: " << jwtToken);
        JWTAuth authAgent("admin", "admin", "admin");
        if (authAgent.verify(jwtToken))
        {
            LOG_TRC("JWT token is valid");
            sendTextFrame("ValidAuthToken " + id);
        }
        else
        {
            LOG_DBG("Invalid auth token");
            sendTextFrame("InvalidAuthToken " + id);
        }
    }
    else if(tokens.equals(0, "closemonitor"))
    {
       _admin->setCloseMonitorFlag();
    }
}

std::atomic<uint64_t> AdminSocketHandler::NextSessionId(1);

AdminSocketHandler::AdminSocketHandler(Admin* adminManager,
                                       const std::weak_ptr<StreamSocket>& socket,
                                       const Poco::Net::HTTPRequest& request,
                                       bool allowedOrigin)
    : WebSocketHandler(socket.lock(), request, allowedOrigin)
    , _admin(adminManager)
    , _isAuthenticated(false)
{
    _sessionId = NextSessionId++;
    _clientIPAdress = socket.lock()->clientAddress();
}

AdminSocketHandler::AdminSocketHandler(Admin* adminManager)
    : WebSocketHandler(/* isClient = */ true, /* isMasking = */ true),
      _admin(adminManager),
      _isAuthenticated(true)
{
    _sessionId = NextSessionId++;
}

void AdminSocketHandler::sendTextFrame(const std::string& message)
{
    if constexpr (!Util::isFuzzing())
    {
        UNITWSD_CALL(onAdminQueryMessage(message));
    }

    if (_isAuthenticated)
    {
        LOG_TRC("send admin text frame '" << message << '\'');
        sendTextMessage(message);
    }
    else
        LOG_TRC("Skip sending message to non-authenticated client: '" << message << '\'');
}

void AdminSocketHandler::subscribeAsync(const std::shared_ptr<AdminSocketHandler>& handler)
{
    Admin &admin = Admin::instance();

    admin.addCallback([handler]
        {
            Admin &adminIn = Admin::instance();
            adminIn.getModel().subscribe(handler->_sessionId, handler);
        });
}

bool AdminSocketHandler::handleInitialRequest(
    const std::weak_ptr<StreamSocket> &socketWeak,
    const Poco::Net::HTTPRequest& request,
    bool allowedOrigin)
{
    if (!COOLWSD::AdminEnabled)
    {
        LOG_ERR_S("Request for disabled admin console");
        return false;
    }

    std::shared_ptr<StreamSocket> socket = socketWeak.lock();
    if (!socket)
    {
        LOG_ERR_S("Invalid socket while reading initial request");
        return false;
    }

    const std::string& requestURI = request.getURI();
    StringVector pathTokens(StringVector::tokenize(requestURI, '/'));

    if (request.has("Upgrade") && Util::iequal(request["Upgrade"], "websocket"))
    {
        Admin &admin = Admin::instance();
        auto handler = std::make_shared<AdminSocketHandler>(&admin, socketWeak,
                                                            request, allowedOrigin);
        socket->setHandler(handler);

        AdminSocketHandler::subscribeAsync(handler);

        return true;
    }

    http::Response response(http::StatusCode::BadRequest);
    response.setContentLength(0);
    LOG_INF_S("Admin::handleInitialRequest bad request");
    socket->send(response);

    return false;
}

std::unique_ptr<Admin> Admin::Instance;

/// An admin command processor.
Admin::Admin()
    : SocketPoll("admin")
    , _totalSysMemKb(Util::getTotalSystemMemoryKb())
    , _totalAvailMemKb(_totalSysMemKb)
    , _lastTotalMemory(0)
    , _lastJiffies(0)
    , _lastSentCount(0)
    , _lastRecvCount(0)
    , _cpuStatsTaskIntervalMs(DefStatsIntervalMs)
    , _memStatsTaskIntervalMs(DefStatsIntervalMs * 2)
    , _netStatsTaskIntervalMs(DefStatsIntervalMs * 2)
    , _cleanupIntervalMs(DefStatsIntervalMs * 10)
    , _forKitPid(-1)
{
    LOG_INF("Admin ctor");

    LOG_TRC("Total system memory:  " << _totalSysMemKb << " KB");

    // If there is a cgroup limit that is smaller still, apply it.
    const std::size_t cgroupMemLimitKb = Util::getCGroupMemLimit() / 1024;
    if (cgroupMemLimitKb > 0 && cgroupMemLimitKb < _totalAvailMemKb)
    {
        LOG_TRC("cgroup memory limit: " << cgroupMemLimitKb << " KB");
        _totalAvailMemKb = cgroupMemLimitKb;
    }
    else
        LOG_TRC("no cgroup memory limit");

    // If there is a cgroup soft-limit that is smaller still, apply that.
    const std::size_t cgroupMemSoftLimitKb = Util::getCGroupMemSoftLimit() / 1024;
    if (cgroupMemSoftLimitKb > 0 && cgroupMemSoftLimitKb < _totalAvailMemKb)
    {
        LOG_TRC("cgroup memory soft limit: " << cgroupMemSoftLimitKb << " KB");
        _totalAvailMemKb = cgroupMemSoftLimitKb;
    }
    else
        LOG_TRC("no cgroup memory soft limit");

    // Reserve some minimum memory (1 MB, arbitrarily)
    // as headroom. Otherwise, coolwsd might fail to
    // clean-up Kits when we run out, and by then we die.
    // This should be enough to update DocBroker containers,
    // take locks, print logs, etc. during cleaning up.
    std::size_t minHeadroomKb = 1024;

    // If we have a manual percentage cap, apply it.
    const double memLimit = ConfigUtil::getConfigValue<double>("memproportion", 0.0);
    if (memLimit > 0.0)
    {
        const double headroom = _totalAvailMemKb * (100. - memLimit) / 100.;
        if (minHeadroomKb < headroom)
            minHeadroomKb = static_cast<std::size_t>(headroom);
    }

    if (_totalAvailMemKb > minHeadroomKb)
    {
        _totalAvailMemKb -= minHeadroomKb;
    }

    const size_t totalUsedMemKb = getTotalMemoryUsage();
    _model.addMemStats(totalUsedMemKb);

    LOG_INF(
        std::setprecision(2)
        << "Total available memory: " << _totalAvailMemKb << " KB ("
        << _totalAvailMemKb / (1024 * 1024.) << " GB), System memory: " << _totalSysMemKb << " KB ("
        << _totalSysMemKb / (1024 * 1024.) << " GB), cgroup limit: " << cgroupMemLimitKb
        << " KB, cgroup soft-limit: " << cgroupMemSoftLimitKb
        << " KB, configured memproportion: " << memLimit << "%, actual percentage of system total: "
        << (_totalSysMemKb ? (_totalAvailMemKb * 100. / _totalSysMemKb) : 100)
        << "%, current usage: " << totalUsedMemKb << " KB ("
        << (_totalAvailMemKb ? (totalUsedMemKb * 100. / _totalAvailMemKb) : 100) << "% of limit)");

    if (_totalAvailMemKb < 1000 * 1024)
        LOG_ERR("Low memory condition detected: only " << _totalAvailMemKb / 1024
                                                       << " MB of RAM available");

    LOG_INF("Hardware threads: " << std::thread::hardware_concurrency());
}

Admin::~Admin()
{
    LOG_INF("~Admin dtor.");
}

void Admin::pollingThread()
{
    _model.setThreadOwner(ProcUtil::getThreadId());

    std::chrono::steady_clock::time_point lastCPU = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point lastMem = lastCPU;
    std::chrono::steady_clock::time_point lastNet = lastCPU;
    std::chrono::steady_clock::time_point lastCleanup = lastCPU;

    const static auto SystemClockTicks = sysconf(_SC_CLK_TCK);
    constexpr auto MaxProximityToTargetPeriod = MinStatsIntervalMs / 8;

    while (!isStop() && !SigUtil::getShutdownRequestFlag())
    {
        const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();

        auto cpuWait = _cpuStatsTaskIntervalMs -
            std::chrono::duration_cast<std::chrono::milliseconds>(now - lastCPU);
        if (cpuWait <= MaxProximityToTargetPeriod) // Close enough
        {
            const size_t currentJiffies = getTotalCpuUsage();
            const size_t cpuPercent =
                100 * 1000 * currentJiffies / (SystemClockTicks * _cpuStatsTaskIntervalMs.count());
            _model.addCpuStats(cpuPercent);

            cpuWait += _cpuStatsTaskIntervalMs;
            lastCPU = now;
        }

        auto memWait = _memStatsTaskIntervalMs -
            std::chrono::duration_cast<std::chrono::milliseconds>(now - lastMem);
        if (memWait <= MaxProximityToTargetPeriod) // Close enough
        {
            // disable watchdog to avoid Document::updateMemoryDirty noise
            disableWatchdog();
            _model.UpdateMemoryDirty();
            enableWatchdog();

            const size_t totalMem = getTotalMemoryUsage();
            _model.addMemStats(totalMem);

            if (totalMem != _lastTotalMemory)
            {
                // If our total memory consumption is above limit, cleanup
                triggerMemoryCleanup(totalMem);

                _lastTotalMemory = totalMem;
            }

            notifyDocsMemDirtyChanged();

            memWait += _memStatsTaskIntervalMs;
            lastMem = now;
        }

        auto netWait = _netStatsTaskIntervalMs -
            std::chrono::duration_cast<std::chrono::milliseconds>(now - lastNet);
        if (netWait <= MaxProximityToTargetPeriod) // Close enough
        {
            const uint64_t sentCount = _model.getSentBytesTotal();
            const uint64_t recvCount = _model.getRecvBytesTotal();

            _model.addSentStats(sentCount - _lastSentCount);
            _model.addRecvStats(recvCount - _lastRecvCount);
            _model.addConnectionStats(StreamSocket::getExternalConnectionCount());

            if (_lastRecvCount != recvCount || _lastSentCount != sentCount)
            {
                LOGA_TRC(Admin, "Total Data sent: " << sentCount << ", recv: " << recvCount);
                _lastRecvCount = recvCount;
                _lastSentCount = sentCount;
            }

            netWait += _netStatsTaskIntervalMs;
            lastNet = now;
        }

        auto cleanupWait = _cleanupIntervalMs;
        if (_defDocProcSettings.getCleanupSettings().getEnable())
        {
            if (now > lastCleanup)
            {
                cleanupWait -=
                    std::chrono::duration_cast<std::chrono::milliseconds>(now - lastCleanup);
            }

            if (cleanupWait <= MaxProximityToTargetPeriod) // Close enough
            {
                cleanupResourceConsumingDocs();
                if (_defDocProcSettings.getCleanupSettings().getLostKitGracePeriod())
                    cleanupLostKits();

                cleanupWait += _cleanupIntervalMs;
                lastCleanup = now;
            }
        }

        // (re)-connect (with sync. DNS - urk) to one monitor at a time
        if (_pendingConnects.size())
        {
            MonitorConnectRecord rec = _pendingConnects[0];
            if (rec.getWhen() < now)
            {
                _pendingConnects.erase(_pendingConnects.begin());
                connectToMonitorSync(rec.getUri());
            }
        }

        bool dumpMetrics = true;
        if (_dumpMetrics.compare_exchange_strong(dumpMetrics, false))
        {
            std::ostringstream oss(Util::makeDumpStateStream());
            oss << "Start Admin " << getpid() << " Dump State:\n";

            dumpState(oss);

            oss << "End Admin " << getpid() << " Dump State.\n";

            const std::string str = oss.str();
            fprintf(stderr, "%s", str.c_str());
            LOG_WRN(str);
        }

        // Handle websockets & other work.
        const auto timeout =
            capAndRoundInterval(std::min({ cpuWait, memWait, netWait, cleanupWait }));
        LOGA_TRC(Admin, "Admin poll for " << timeout);
        pollUntilDeadline(now + timeout); // continue with ms for admin, settings etc.
    }

    if (!COOLWSD::IndirectionServerEnabled)
        return;

    // if don't have monitor connection to the controller we set the _migrateMsgReceived
    // for each docbroker so that docbroker can cleanup the documents
    bool controllerMonitorConnection = false;
    for (const auto& pair : _monitorSockets)
    {
        if (pair.first.find("controller") != std::string::npos)
        {
            controllerMonitorConnection = true;
            break;
        }
    }

    if (!controllerMonitorConnection)
    {
        LOG_WRN("Monitor connection to the controller doesn't exist, skipping shutdown migration");
        COOLWSD::setAllMigrationMsgReceived();
        return;
    }

    _model.sendShutdownReceivedMsg();

    static const std::chrono::microseconds closeMonitorMsgTimeout = ConfigUtil::getConfigValue(
        "indirection_endpoint.migration_timeout_secs", std::chrono::seconds(180));

    std::chrono::time_point<std::chrono::steady_clock> closeMonitorMsgStartTime =
        std::chrono::steady_clock::now();
    while (!_closeMonitor)
    {
        LOG_DBG("Waiting for migration to complete before closing the monitor");
        const auto now = std::chrono::steady_clock::now();
        const auto elapsedMicroS =
            std::chrono::duration_cast<std::chrono::microseconds>(now - closeMonitorMsgStartTime);
        if (elapsedMicroS > closeMonitorMsgTimeout)
        {
            LOG_WRN("Timed out waiting for the migration server to respond within the configured "
                    "timeout of "
                    << closeMonitorMsgTimeout);
            break;
        }
        poll(closeMonitorMsgTimeout - elapsedMicroS);
    }

    // if monitor closes early we set the _migrateMsgReceived for each docbroker
    // so that docbroker can cleanup the documents
    if (_closeMonitor)
        COOLWSD::setAllMigrationMsgReceived();
}

void Admin::modificationAlert(const std::string& docKey, pid_t pid, bool value){
    addCallback([this, docKey, pid, value] { _model.modificationAlert(docKey, pid, value); });
}

void Admin::uploadedAlert(const std::string& docKey, pid_t pid, bool value)
{
    addCallback([this, docKey, pid, value] { _model.uploadedAlert(docKey, pid, value); });
}

void Admin::addDoc(const std::string& docKey, pid_t pid, const std::string& filename,
                   const std::string& sessionId, const std::string& userName, const std::string& userId,
                   const std::weak_ptr<FILE>& smapsFp, const std::string& wopiSrc, bool readOnly)
{
    addCallback([this, docKey, pid, filename, sessionId, userName, userId, smapsFp, wopiSrc, readOnly] {
        _model.addDocument(docKey, pid, filename, sessionId, userName, userId, smapsFp, Poco::URI(wopiSrc), readOnly);
    });
}

void Admin::rmDoc(const std::string& docKey, const std::string& sessionId)
{
    addCallback([this, docKey, sessionId] { _model.removeDocument(docKey, sessionId); });
}

void Admin::rmDoc(const std::string& docKey)
{
    LOG_INF("Removing complete doc [" << docKey << "] from Admin.");
    addCallback([this, docKey]{ _model.removeDocument(docKey); });
}

void Admin::rescheduleMemTimer(std::chrono::milliseconds interval)
{
    _memStatsTaskIntervalMs = capAndRoundInterval(interval);
    LOG_INF("Memory stats interval changed - New interval: " << _memStatsTaskIntervalMs);
    _netStatsTaskIntervalMs = capAndRoundInterval(interval); // Until we support modifying this.
    LOG_INF("Network stats interval changed - New interval: " << _netStatsTaskIntervalMs);
    wakeup();
}

void Admin::rescheduleCpuTimer(std::chrono::milliseconds interval)
{
    _cpuStatsTaskIntervalMs = capAndRoundInterval(interval);
    LOG_INF("CPU stats interval changed - New interval: " << _cpuStatsTaskIntervalMs);
    wakeup();
}

std::time_t Admin::getLastActivityTime() const
{
    return _model.getLastActivityTime();
}

size_t Admin::getTotalMemoryUsage() const
{
    // To simplify and clarify this; since load, link and pre-init all
    // inside the forkit - we should account all of our fixed cost of
    // memory to the forkit; and then count only dirty pages in the clients
    // since we know that they share everything else with the forkit.
    const size_t forkitRssKb = ProcUtil::getMemoryUsageRSS(_forKitPid);
    const size_t wsdPssKb = ProcUtil::getMemoryUsagePSS(ProcUtil::getProcessId());
    const size_t kitsDirtyKb = _model.getKitsMemoryUsage();
    const size_t totalMem = wsdPssKb + forkitRssKb + kitsDirtyKb;

    return totalMem;
}

size_t Admin::getTotalCpuUsage() const
{
    const size_t forkitJ = ProcUtil::getCpuUsage(_forKitPid);
    const size_t wsdJ = ProcUtil::getCpuUsage(ProcUtil::getProcessId());

    if (_lastJiffies == 0)
    {
        _lastJiffies = forkitJ + wsdJ;
        return 0;
    }

    const size_t kitsJ = _model.getKitsJiffies();
    const size_t totalJ = ((forkitJ + wsdJ) - _lastJiffies) + kitsJ;
    _lastJiffies = forkitJ + wsdJ;

    return totalJ;
}

std::chrono::milliseconds Admin::getMemStatsInterval() const
{
    ASSERT_CORRECT_THREAD();
    return _memStatsTaskIntervalMs;
}

std::chrono::milliseconds Admin::getCpuStatsInterval() const
{
    ASSERT_CORRECT_THREAD();
    return _cpuStatsTaskIntervalMs;
}

std::chrono::milliseconds Admin::getNetStatsInterval() const
{
    ASSERT_CORRECT_THREAD();
    return _netStatsTaskIntervalMs;
}

std::string Admin::getChannelLogLevels() const
{
    std::string result = "wsd=" + Log::getLogLevelName("wsd");

    result += " kit=" + (_forkitLogLevel.empty() != true ? _forkitLogLevel: Log::getLogLevelName("wsd"));

    return result;
}

void Admin::setChannelLogLevel(const std::string& channelName, std::string level)
{
    ASSERT_CORRECT_THREAD();

    if (channelName == "wsd")
        Log::setLogLevelByName("wsd", level);

    else if (channelName == "kit")
    {
        COOLWSD::setLogLevelsOfKits(level); // For current kits.
        COOLWSD::sendMessageToForKit("setloglevel " + level); // For forkit and future kits.
        _forkitLogLevel = std::move(level); // We will remember this setting rather than asking forkit its loglevel.
    }
}

std::string Admin::getLogLines() const
{
    ASSERT_CORRECT_THREAD();

    try
    {
        static const std::string fName = ConfigUtil::getPathFromConfig("logging.file.property[0]");
        std::ifstream infile(fName);

        std::size_t lineCount = 500;
        std::string line;
        std::deque<std::string> lines;

        while (std::getline(infile, line))
        {
            std::istringstream iss(line);
            lines.push_back(line);
            if (lines.size() > lineCount)
            {
                lines.pop_front();
            }
        }

        infile.close();

        if (lines.size() < lineCount)
        {
            lineCount = lines.size();
        }

        line.clear(); // Use the same variable to include result.
        if (lineCount > 0)
        {
            line.reserve(lineCount * 128); // Avoid repeated resizing.
            // Newest will be on top.
            for (int i = static_cast<int>(lineCount) - 1; i >= 0; i--)
            {
                line += '\n';
                line += lines[i];
            }
        }

        return line;
    }
    catch (const std::exception& e) {
        return "Could not read the log file.";
    }
}

AdminModel& Admin::getModel()
{
    return _model;
}

void Admin::updateLastActivityTime(const std::string& docKey)
{
    addCallback([this, docKey]{ _model.updateLastActivityTime(docKey); });
}


void Admin::addBytes(const std::string& docKey, uint64_t sent, uint64_t recv)
{
    addCallback([this, docKey, sent, recv] { _model.addBytes(docKey, sent, recv); });
}

void Admin::setViewLoadDuration(const std::string& docKey, const std::string& sessionId, std::chrono::milliseconds viewLoadDuration)
{
    addCallback([this, docKey, sessionId, viewLoadDuration]{ _model.setViewLoadDuration(docKey, sessionId, viewLoadDuration); });
}

void Admin::setDocWopiDownloadDuration(const std::string& docKey, std::chrono::milliseconds wopiDownloadDuration)
{
    addCallback([this, docKey, wopiDownloadDuration]{ _model.setDocWopiDownloadDuration(docKey, wopiDownloadDuration); });
}

void Admin::setDocWopiUploadDuration(const std::string& docKey, const std::chrono::milliseconds uploadDuration)
{
    addCallback([this, docKey, uploadDuration]{ _model.setDocWopiUploadDuration(docKey, uploadDuration); });
}

void Admin::addErrorExitCounters(unsigned segFaultCount, unsigned killedCount,
                                 unsigned oomKilledCount)
{
    addCallback([this, segFaultCount, killedCount, oomKilledCount]
                { _model.addErrorExitCounters(segFaultCount, killedCount, oomKilledCount); });
}

void Admin::addLostKitsTerminated(unsigned lostKitsTerminated)
{
    addCallback([this, lostKitsTerminated]{ _model.addLostKitsTerminated(lostKitsTerminated); });
}

void Admin::routeTokenSanityCheck()
{
    addCallback([this] { _model.routeTokenSanityCheck(); });
}

void Admin::sendShutdownReceivedMsg()
{
    addCallback([this] { _model.sendShutdownReceivedMsg(); });
}

void Admin::notifyForkit()
{
    std::ostringstream oss;
    oss << "setconfig limit_virt_mem_mb " << _defDocProcSettings.getLimitVirtMemMb() << '\n'
        << "setconfig limit_stack_mem_kb " << _defDocProcSettings.getLimitStackMemKb() << '\n'
        << "setconfig limit_file_size_mb " << _defDocProcSettings.getLimitFileSizeMb() << '\n'
        << "setconfig limit_num_open_files " << _defDocProcSettings.getLimitNumberOpenFiles() << '\n';

    COOLWSD::sendMessageToForKit(oss.str());
}

namespace
{
/// Similar to std::clamp(), old libstdc++ doesn't have it.
template <typename T> T clamp(const T& n, const T& lower, const T& upper)
{
    return std::max(lower, std::min(n, upper));
}
} // namespace

void Admin::triggerMemoryCleanup(const size_t totalMem)
{
    // Trigger mem cleanup when we are consuming too much memory (as configured by sysadmin)
    static const double memLimit = ConfigUtil::getConfigValue<double>("memproportion", 0.0);
    if (memLimit == 0.0 || _totalSysMemKb == 0)
    {
        LOGA_TRC(Admin, "Total memory consumed: " << totalMem <<
                " KB. Not configured to do memory cleanup. Skipping memory cleanup.");
        return;
    }

    LOGA_TRC(Admin, "Total memory consumed: " << totalMem << " KB. Configured COOL memory proportion: " <<
            memLimit << "% (" << static_cast<size_t>(_totalSysMemKb * memLimit / 100.) << " KB).");

    const double memToFreePercentage = (totalMem / static_cast<double>(_totalSysMemKb)) - memLimit / 100.;
    int64_t memToFreeKb =
        clamp<double>(memToFreePercentage * _totalSysMemKb, 0, std::numeric_limits<int>::max());

    // Don't kill documents to save a MB or two.
    constexpr int64_t MinMemToFreeKB = 1024;
    if (memToFreeKb <= MinMemToFreeKB)
    {
        return;
    }

    // prepare document list sorted by most idle times
    const std::vector<DocBasicInfo> docList = _model.getDocumentsSortedByIdle();

    LOG_TRC("OOM: Memory to free: " << memToFreePercentage << "% (" << memToFreeKb << " KB) from "
                                    << docList.size() << " docs");

    for (const auto& doc : docList)
    {
        LOG_TRC("OOM Document: DocKey: [" << doc.getDocKey() << "], Idletime: ["
                                          << doc.getIdleTime() << "]," << " Saved: ["
                                          << doc.getSaved() << "], Mem: [" << doc.getMem() << ']');
        if (doc.getSaved())
        {
            // Kill the saved documents first.
            LOG_WRN("OOM: Killing saved document with DocKey ["
                    << doc.getDocKey() << "], Idletime: [" << doc.getIdleTime() << "] using "
                    << doc.getMem() << " KB");
            COOLWSD::closeDocument(doc.getDocKey(), "oom");
            memToFreeKb -= doc.getMem();
            if (memToFreeKb <= MinMemToFreeKB)
                break;
        }
        else
        {
            // Save unsaved documents.
            LOG_DBG("Saving document: DocKey [" << doc.getDocKey() << ']');
            COOLWSD::autoSave(doc.getDocKey());
        }
    }
}

void Admin::notifyDocsMemDirtyChanged()
{
    _model.notifyDocsMemDirtyChanged();
}

void Admin::cleanupResourceConsumingDocs()
{
    _model.cleanupResourceConsumingDocs();
}

void Admin::cleanupLostKits()
{
    if constexpr (Util::isKitInProcess())
        return; // we might look lost ourselves.

    static std::map<pid_t, std::time_t> mapKitsLost;
    std::set<pid_t> internalKitPids;
    std::vector<int> kitPids;
    int pid;
    unsigned lostKitsTerminated = 0;
    size_t gracePeriod = _defDocProcSettings.getCleanupSettings().getLostKitGracePeriod();

    internalKitPids = COOLWSD::getKitPids();
    AdminModel::getKitPidsFromSystem(&kitPids);

    for (auto itProc = kitPids.begin(); itProc != kitPids.end(); itProc ++)
    {
        pid = *itProc;
        if (internalKitPids.find(pid) == internalKitPids.end())
        {
            // Check if this is our kit process (forked from our ForKit process)
            if (ProcUtil::getStatFromPid(pid, 3) == (size_t)_forKitPid)
                mapKitsLost.insert(std::pair<pid_t, std::time_t>(pid, std::time(nullptr)));
        }
        else
            mapKitsLost.erase(pid);
    }

    for (auto itLost = mapKitsLost.begin(); itLost != mapKitsLost.end();)
    {
        if (std::time(nullptr) - itLost->second > (time_t)gracePeriod)
        {
            pid = itLost->first;
            if (::kill(pid, 0) == 0)
            {
                if (::kill(pid, SIGKILL) == -1)
                    LOG_ERR("Detected lost kit [" << pid << "]. Failed to send SIGKILL.");
                else
                {
                    lostKitsTerminated ++;
                    LOG_ERR("Detected lost kit [" << pid << "]. Sent SIGKILL for termination.");
                }
            }

            itLost = mapKitsLost.erase(itLost);
        }
        else
            itLost ++;
    }

    if (lostKitsTerminated)
        Admin::instance().addLostKitsTerminated(lostKitsTerminated);
}

void Admin::dumpState(std::ostream& os) const
{
    SocketPoll::dumpState(os);

    os << "Monitor sockets: " << _monitorSockets.size() << ":\n";
    if (!_monitorSockets.empty())
    {
        for (const auto& socket : _monitorSockets)
        {
            os << socket.first << ": " << (socket.second->isConnected() ? "" : "dis")
               << "connected\n";
        }
    }

    os << "Admin Metrics:\n";
    getMetrics(os);
    os << '\n';
}

MonitorSocketHandler::MonitorSocketHandler(Admin *admin, const std::string &uri)
    : AdminSocketHandler(admin)
    , _uri(uri)
    , _connecting(true)
{
}

int MonitorSocketHandler::getPollEvents(std::chrono::steady_clock::time_point now,
                    int64_t &timeoutMaxMicroS)
{
    if (_connecting)
    {
        LOG_TRC("Waiting for outbound connection to complete");
        return POLLOUT;
    }
    else
        return AdminSocketHandler::getPollEvents(now, timeoutMaxMicroS);
}

void MonitorSocketHandler::performWrites(std::size_t capacity)
{
    LOG_TRC("Outbound monitor - connected");
    _connecting = false;
    return AdminSocketHandler::performWrites(capacity);
}

void MonitorSocketHandler::onDisconnect()
{
    bool reconnect = false;
    // schedule monitor reconnect only if monitor uri exist in configuration
    for (const auto& monitor : Admin::instance().getMonitorList())
    {
        const std::string uriWithoutParam = _uri.substr(0, _uri.find('?'));
        if (Util::iequal(monitor.first, uriWithoutParam))
        {
            LOG_ERR("Monitor " << _uri << " dis-connected, re-trying in " << monitor.second  << " seconds");
            Admin::instance().scheduleMonitorConnect(
                _uri, std::chrono::steady_clock::now() + std::chrono::seconds(monitor.second));
            Admin::instance().deleteMonitorSocket(uriWithoutParam);
            reconnect = true;
            break;
        }
    }

    if (!reconnect)
        LOG_TRC("Remove monitor " << _uri);
}

void Admin::connectToMonitorSync(const std::string &uri)
{
    const std::string uriWithoutParam = uri.substr(0, uri.find('?'));
    if (_monitorSockets.find(uriWithoutParam) != _monitorSockets.end())
    {
        LOG_TRC("Monitor connection with uri:" << uri << " already exist");
        return;
    }

    LOG_TRC("Add monitor " << uri);
    static const bool logMonitorConnect =
        ConfigUtil::getConfigValue<bool>("admin_console.logging.monitor_connect", true);
    if (logMonitorConnect)
    {
        LOG_ANY("Connected to remote monitor with uri [" << uriWithoutParam << ']');
    }

    auto handler = std::make_shared<MonitorSocketHandler>(this, uri);
    _monitorSockets.insert({uriWithoutParam, handler});
    insertNewWebSocketSync(Poco::URI(uri), handler);
    AdminSocketHandler::subscribeAsync(handler);
}

void Admin::scheduleMonitorConnect(const std::string &uri, std::chrono::steady_clock::time_point when)
{
    ASSERT_CORRECT_THREAD();

    MonitorConnectRecord todo;
    todo.setWhen(when);
    todo.setUri(uri);
    _pendingConnects.push_back(std::move(todo));
}

void Admin::getMetrics(std::ostream& metrics) const
{
    const size_t memAvail = getTotalAvailableMemory();
    const size_t memUsed = getTotalMemoryUsage();

    metrics << "global_host_system_memory_bytes " << _totalSysMemKb * 1024 << '\n';
    metrics << "global_host_tcp_connections " << net::Defaults.maxExtConnections << '\n';
    metrics << "global_memory_available_bytes " << memAvail * 1024 << '\n';
    metrics << "global_memory_used_bytes " << memUsed * 1024 << '\n';
    metrics << "global_memory_free_bytes " << (memAvail - memUsed) * 1024 << '\n';
    metrics << '\n';

    _model.getMetrics(metrics);
}

void Admin::sendMetrics(const std::shared_ptr<StreamSocket>& socket,
                        const std::shared_ptr<http::Response>& response) const
{
    std::ostringstream oss;
    getMetrics(oss);

    response->setConnectionToken(http::Header::ConnectionToken::Close);
    response->setBody(oss.str(), "text/plain");

    socket->send(*response);
    socket->asyncShutdown();

    static bool skipAuthentication =
        ConfigUtil::getConfigValue<bool>("security.enable_metrics_unauthenticated", false);
    static bool showLog =
        ConfigUtil::getConfigValue<bool>("admin_console.logging.metrics_fetch", true);
    if (!skipAuthentication && showLog)
    {
        LOG_ANY("Metrics endpoint has been accessed by source IPAddress [" << socket->clientAddress() << ']');
    }
}

void Admin::start()
{
    startMonitors();
    startThread();
}

std::vector<std::pair<std::string, int>> Admin::getMonitorList() const
{
    const auto& config = Application::instance().config();
    std::vector<std::pair<std::string, int>> monitorList;
    for (size_t i = 0;; ++i)
    {
        const std::string path = "monitors.monitor[" + std::to_string(i) + ']';
        const std::string uri = config.getString(path, "");
        const auto retryInterval = ConfigUtil::getConfigValue<int>(path + "[@retryInterval]", 20);
        if (!config.has(path))
            break;
        if (!uri.empty())
        {
            Poco::URI monitor(uri);
            if (monitor.getScheme() == "wss" || monitor.getScheme() == "ws")
                monitorList.push_back(std::make_pair(uri, retryInterval));
            else
                LOG_ERR("Unhandled monitor URI: '" << uri << "' should be \"wss://foo:1234/baa\"");
        }
    }
    return monitorList;
}

void Admin::startMonitors()
{
    bool haveMonitors = false;
    for (const auto& monitor : getMonitorList())
    {
        addCallback(
            [this, monitor]
            {
                scheduleMonitorConnect(monitor.first + "?ServerId=" + Util::getProcessIdentifier(),
                                       std::chrono::steady_clock::now());
            });
        haveMonitors = true;
    }

    if (!haveMonitors)
        LOG_TRC("No monitors configured.");
}

void Admin::updateMonitors(std::vector<std::pair<std::string,int>>& oldMonitors)
{
    if (oldMonitors.empty())
    {
        startMonitors();
        return;
    }

    std::unordered_map<std::string, bool> currentMonitorMap;
    for (const auto& monitor : getMonitorList())
    {
        currentMonitorMap[monitor.first] = true;
    }

    // shutdown monitors which does not not exist in currentMonitorMap
    for (const auto& monitor : oldMonitors)
    {
        if (!currentMonitorMap[monitor.first])
        {
            auto socketHandler = _monitorSockets[monitor.first];
            if (socketHandler != nullptr)
            {
                socketHandler->shutdown();
                _monitorSockets.erase(monitor.first);
            }
        }
    }

    startMonitors();
}

void Admin::deleteMonitorSocket(const std::string& uriWithoutParam)
{
    if (_monitorSockets.find(uriWithoutParam) != _monitorSockets.end())
    {
        _monitorSockets.erase(uriWithoutParam);
    }
}

void Admin::stop()
{
    joinThread();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
