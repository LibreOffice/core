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

#include "HealthCheck.hpp"

#if !MOBILEAPP

#include <wsd/Admin.hpp>
#include <wsd/COOLWSD.hpp>
#include <wsd/FileServer.hpp>

#include <common/FileUtil.hpp>
#include <common/Log.hpp>
#include <common/SigUtil.hpp>
#include <common/Util.hpp>
#include <net/HttpRequest.hpp>
#include <net/Socket.hpp>

#include <Poco/Net/HTTPRequest.h>
#include <Poco/URI.h>

#include <algorithm>
#include <cassert>
#include <cerrno>
#include <sstream>
#include <string>
#include <vector>

#include <signal.h>

namespace
{

struct CheckResult
{
    bool passed;
    std::string reason;
};

CheckResult pass() { return { true, std::string() }; }

CheckResult fail(std::string reason) { return { false, std::move(reason) }; }

CheckResult checkPing() { return pass(); }

CheckResult checkForkit()
{
    if (Util::isKitInProcess())
        return pass();

    const pid_t pid = COOLWSD::ForKitProcId;
    if (pid <= 0)
        return fail("the forkit process is not running");

    if (kill(pid, 0) != 0 && errno == ESRCH)
        return fail("the forkit process has died");

    return pass();
}

CheckResult checkShutdown()
{
    if (SigUtil::getShutdownRequestFlag())
        return fail("the server is shutting down");

    return pass();
}

CheckResult checkKitSpares()
{
    if (Util::isKitInProcess())
        return pass();

    if (!COOLWSD::getSpareKitPids().empty())
        return pass();

    if (COOLWSD::getOutstandingForksCount() > 0)
        return pass();

    return fail("no spare kit process is available and none is being spawned");
}

CheckResult checkDiskSpace()
{
    const std::string fullFileSystem = FileUtil::checkDiskSpaceOnRegisteredFileSystems();
    if (!fullFileSystem.empty())
    {
        LOG_WRN("Health check: low disk space on [" << fullFileSystem << ']');
        return fail("a monitored file system is low on free space");
    }

    return pass();
}

CheckResult checkMemory()
{
    if (!Admin::isInitialized())
        return pass();

    const size_t usedKb = Admin::instance().getLastTotalMemoryUsage();
    const size_t availableKb = Admin::instance().getTotalAvailableMemory();
    if (usedKb > 0 && availableKb > 0 && usedKb > availableKb)
        return fail("memory usage exceeds the configured limit");

    return pass();
}

CheckResult checkConnections()
{
    const unsigned limit = COOLWSD::MaxConnections;
    const unsigned count = COOLWSD::NumConnections;
    if (limit > 0 && count >= limit)
        return fail("the connection limit has been reached");

    return pass();
}

CheckResult checkDocuments()
{
    // A limit of 10000 or more means an unlimited build, as in the session limit warnings.
    const unsigned limit = COOLWSD::MaxDocuments;
    if (limit == 0 || limit >= 10000)
        return pass();

    if (COOLWSD::getInteractiveDocBrokerCount() >= static_cast<ssize_t>(limit))
        return fail("the document limit has been reached");

    return pass();
}

/// /readyz runs every check; /livez runs only those flagged livez.
struct Check
{
    std::string_view name;
    bool livez;
    CheckResult (*run)();
};

constexpr Check Checks[] = {
    { "ping", /*livez=*/true, checkPing },
    { "forkit", /*livez=*/true, checkForkit },
    { "shutdown", /*livez=*/false, checkShutdown },
    { "kit-spares", /*livez=*/false, checkKitSpares },
    { "disk-space", /*livez=*/false, checkDiskSpace },
    { "memory", /*livez=*/false, checkMemory },
    { "connections", /*livez=*/false, checkConnections },
    { "documents", /*livez=*/false, checkDocuments },
};

bool isCheckInEndpoint(const Check& check, const std::string& endpoint)
{
    return endpoint == "livez" ? check.livez : true;
}

void sendResponse(const Poco::Net::HTTPRequest& request,
                  const std::shared_ptr<StreamSocket>& socket, http::StatusCode statusCode,
                  const std::string& body)
{
    http::Response httpResponse(statusCode);
    FileServerRequestHandler::hstsHeaders(httpResponse);
    httpResponse.setContentLength(body.size());
    httpResponse.setContentType("text/plain; charset=utf-8");
    httpResponse.set("Cache-Control", "no-cache");
    httpResponse.set("X-Content-Type-Options", "nosniff");
    if (!request.getKeepAlive())
        httpResponse.setConnectionToken(http::Header::ConnectionToken::Close);
    httpResponse.writeData(socket->getOutBuffer());

    if (request.getMethod() != Poco::Net::HTTPRequest::HTTP_HEAD)
        socket->send(body);

    socket->attemptWrites();
}

void handleSingleCheck(const Poco::Net::HTTPRequest& request,
                       const std::shared_ptr<StreamSocket>& socket, const std::string& endpoint,
                       const std::string& checkName)
{
    for (const Check& check : Checks)
    {
        if (check.name != checkName || !isCheckInEndpoint(check, endpoint))
            continue;

        const CheckResult result = check.run();
        if (result.passed)
        {
            sendResponse(request, socket, http::StatusCode::OK, "ok");
        }
        else
        {
            LOG_WRN("Health check [" << checkName << "] failed: " << result.reason);
            sendResponse(request, socket, http::StatusCode::InternalServerError,
                         "internal server error: reason withheld\n");
        }

        return;
    }

    sendResponse(request, socket, http::StatusCode::NotFound, "404 page not found\n");
}

void handleAggregate(const Poco::Net::HTTPRequest& request,
                     const std::shared_ptr<StreamSocket>& socket, const std::string& endpoint,
                     bool verbose, std::vector<std::string> excluded)
{
    std::ostringstream perCheckOutput;
    std::vector<std::string> failed;

    for (const Check& check : Checks)
    {
        if (!isCheckInEndpoint(check, endpoint))
            continue;

        const auto excludedIt = std::find(excluded.begin(), excluded.end(), check.name);
        if (excludedIt != excluded.end())
        {
            excluded.erase(excludedIt);
            perCheckOutput << "[+]" << check.name << " excluded: ok\n";
            continue;
        }

        const CheckResult result = check.run();
        if (result.passed)
        {
            perCheckOutput << "[+]" << check.name << " ok\n";
        }
        else
        {
            // The reason goes to the log only; the public body carries no detail.
            LOG_WRN("Health check [" << check.name << "] failed: " << result.reason);
            perCheckOutput << "[-]" << check.name << " failed: reason withheld\n";
            failed.emplace_back(check.name);
        }
    }

    if (!excluded.empty())
    {
        perCheckOutput << "warn: some health checks cannot be excluded: no matches for ";
        for (std::size_t i = 0; i < excluded.size(); ++i)
        {
            if (i > 0)
                perCheckOutput << ", ";
            perCheckOutput << '"' << excluded[i] << '"';
        }
        perCheckOutput << '\n';
    }

    if (!failed.empty())
    {
        sendResponse(request, socket, http::StatusCode::InternalServerError,
                     perCheckOutput.str() + endpoint + " check failed\n");
        return;
    }

    if (!verbose)
    {
        sendResponse(request, socket, http::StatusCode::OK, "ok");
        return;
    }

    sendResponse(request, socket, http::StatusCode::OK,
                 perCheckOutput.str() + endpoint + " check passed\n");
}

} // anonymous namespace

namespace HealthCheck
{

bool handleRequest(const Poco::Net::HTTPRequest& request,
                   const std::shared_ptr<StreamSocket>& socket)
{
    assert(socket && "Must have a valid socket");

    LOG_DBG("Health check request: " << request.getURI());

    if (request.getMethod() != Poco::Net::HTTPRequest::HTTP_GET &&
        request.getMethod() != Poco::Net::HTTPRequest::HTTP_HEAD)
    {
        sendResponse(request, socket, http::StatusCode::MethodNotAllowed,
                     "405 method not allowed\n");
        return true;
    }

    Poco::URI uri;
    std::vector<std::string> segments;
    try
    {
        uri = Poco::URI(request.getURI());
        uri.getPathSegments(segments);
    }
    catch (const Poco::Exception&)
    {
        sendResponse(request, socket, http::StatusCode::NotFound, "404 page not found\n");
        return true;
    }

    if (segments.empty() || segments.size() > 2)
    {
        sendResponse(request, socket, http::StatusCode::NotFound, "404 page not found\n");
        return true;
    }

    const std::string& endpoint = segments[0];

    if (segments.size() == 2)
    {
        handleSingleCheck(request, socket, endpoint, segments[1]);
        return true;
    }

    bool verbose = false;
    std::vector<std::string> excluded;
    for (const auto& param : uri.getQueryParameters())
    {
        if (param.first == "verbose")
            verbose = true;
        else if (param.first == "exclude" && !param.second.empty())
            excluded.push_back(param.second);
    }

    handleAggregate(request, socket, endpoint, verbose, std::move(excluded));
    return true;
}

} // namespace HealthCheck

#endif // !MOBILEAPP

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
