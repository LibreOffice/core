/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config.h>

#include <common/Log.hpp>
#include <common/Unit.hpp>
#include <common/Util.hpp>
#include <net/HttpRequest.hpp>
#include <net/Socket.hpp>
#include <test/WopiTestServer.hpp>
#include <test/helpers.hpp>
#include <test/lokassert.hpp>

#include <Poco/Net/HTTPRequest.h>
#include <Poco/URI.h>
#include <Poco/Util/LayeredConfiguration.h>

#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

/// A reconnecting browser gets its session back only when the host still gives it the same
/// permissions, and the session then uses the access token the browser brought.
class UnitReconnectWopi : public WopiTestServer
{
    static constexpr int ParkTimeoutSeconds = 3;

    std::vector<std::shared_ptr<TerminatingPoll>> _pollers;
    std::mutex _mutex;
    std::string _lastPutFileToken;
    std::atomic<int> _putFiles{ 0 };

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        WopiTestServer::configure(config);
        config.setInt("net.session_park_timeout_secs", ParkTimeoutSeconds);
    }

    /// The access_token query parameter of a WOPI request.
    static std::string accessTokenOf(const Poco::Net::HTTPRequest& request)
    {
        for (const auto& [key, value] : Poco::URI(request.getURI()).getQueryParameters())
            if (key == "access_token")
                return value;
        return std::string();
    }

    /// The host names the user by the part of the access token before the first '-', so "alice" and
    /// "alice-renewed" are the same user.
    void configCheckFileInfo(const Poco::Net::HTTPRequest& request,
                             Poco::JSON::Object::Ptr& fileInfo) override
    {
        const std::string accessToken = accessTokenOf(request);
        const std::string userId = accessToken.substr(0, accessToken.find('-'));
        fileInfo->set("UserId", userId);
        fileInfo->set("UserFriendlyName", userId);
    }

    std::unique_ptr<http::Response>
    assertPutFileRequest(const Poco::Net::HTTPRequest& request) override
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _lastPutFileToken = accessTokenOf(request);
        ++_putFiles;
        return nullptr;
    }

    std::string lastPutFileToken()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return _lastPutFileToken;
    }

    std::shared_ptr<SocketPoll> newPoller()
    {
        auto poller = std::make_shared<TerminatingPoll>(testname);
        poller->startThread();
        _pollers.push_back(poller);
        return poller;
    }

    /// The encoded WOPI document URL for a file name and an access token. Each case uses its own
    /// file name, so a session left parked by one case does not meet the next one.
    std::string wopiSrc(const std::string& fileName, const std::string& accessToken,
                        const std::string& extraParams = std::string())
    {
        return Uri::encode(helpers::getTestServerURI() + "/wopi/files/" + fileName +
                           "?access_token=" + accessToken + extraParams);
    }

    /// Connects and loads with reconnect=true. The token, when given, rides on the URL.
    std::shared_ptr<http::WebSocketSession> connect(const std::string& documentURL,
                                                    const std::string& token = std::string())
    {
        auto session = http::WebSocketSession::create(helpers::getTestServerURI());
        http::Request request("/cool/" + documentURL + "/ws" +
                              (token.empty() ? std::string() : "?ReconnectToken=" + token));
        session->asyncRequest(request, newPoller());
        session->sendMessage("load url=" + documentURL + " reconnect=true");
        return session;
    }

    /// The token of the next reconnecttoken: message, or empty. The server sends it before the
    /// view's status, so call this first.
    std::string waitForToken(http::WebSocketSession& session)
    {
        static constexpr std::string_view prefix = "reconnecttoken: ";
        const std::vector<char> message =
            session.waitForMessage(prefix, std::chrono::seconds(30), testname);
        if (message.size() <= prefix.size())
            return std::string();
        return std::string(message.begin() + prefix.size(), message.end());
    }

    /// The viewid in the next status: message, or -1.
    static int waitForStatusViewId(http::WebSocketSession& session,
                                   std::chrono::milliseconds timeout)
    {
        const std::vector<char> status = session.waitForMessage("status:", timeout);
        if (status.empty())
            return -1;
        const std::string text(status.begin(), status.end());
        const auto pos = text.find("\"viewid\":");
        return pos == std::string::npos ? -1 : std::atoi(text.c_str() + pos + 9);
    }

    void joinPollers()
    {
        for (const auto& poller : _pollers)
            poller->joinThread();
        _pollers.clear();
    }

    TestResult testChangedPermissionIsNotReattached();
    TestResult testReconnectRenewsAccessToken();

public:
    UnitReconnectWopi()
        : WopiTestServer("UnitReconnectWopi", "hello.odt")
    {
        setTimeout(std::chrono::minutes(4));
    }

    /// Every case runs, so one failure does not hide another. The suite fails if any did.
    void invokeWSDTest() override
    {
        std::vector<std::string> failed;
        for (TestResult (UnitReconnectWopi::*test)() :
             { &UnitReconnectWopi::testChangedPermissionIsNotReattached,
               &UnitReconnectWopi::testReconnectRenewsAccessToken })
        {
            if ((this->*test)() != TestResult::Ok)
                failed.push_back(testname);
            joinPollers();
        }
        if (!failed.empty())
        {
            for (const std::string& name : failed)
                TST_LOG("Failed case: " << name);
            exitTest(TestResult::Failed);
        }
        exitTest(TestResult::Ok);
    }
};

// (g) The same user comes back with permission=readonly. The editable session is not the view the
// browser asked for, so it gets a new one.
UnitBase::TestResult UnitReconnectWopi::testChangedPermissionIsNotReattached()
{
    setTestname(__func__);
    TST_LOG("Starting Test: " << testname);

    auto sessionA = connect(wopiSrc("permission.odt", "alice"));
    const std::string token = waitForToken(*sessionA);
    LOK_ASSERT(!token.empty());
    const int viewIdA = waitForStatusViewId(*sessionA, std::chrono::seconds(30));
    LOK_ASSERT(viewIdA >= 0);
    sessionA->dropWS();
    LOK_ASSERT(sessionA->waitForDisconnection(std::chrono::seconds(10)));

    auto sessionA2 = connect(wopiSrc("permission.odt", "alice", "&permission=readonly"), token);
    const int viewIdA2 = waitForStatusViewId(*sessionA2, std::chrono::seconds(30));
    LOK_ASSERT(viewIdA2 >= 0);
    LOK_ASSERT_MESSAGE("A readonly connection took over the editable session",
                       viewIdA != viewIdA2);

    sessionA2->exitWS();
    joinPollers();
    return TestResult::Ok;
}

// (h) The browser brings a renewed access token on its reconnect. The next upload uses it.
UnitBase::TestResult UnitReconnectWopi::testReconnectRenewsAccessToken()
{
    setTestname(__func__);
    TST_LOG("Starting Test: " << testname);

    auto sessionA = connect(wopiSrc("renewed-token.odt", "alice"));
    const std::string token = waitForToken(*sessionA);
    LOK_ASSERT(!token.empty());
    const int viewIdA = waitForStatusViewId(*sessionA, std::chrono::seconds(30));
    LOK_ASSERT(viewIdA >= 0);
    sessionA->sendMessage("key type=input char=97 key=0");
    LOK_ASSERT_MESSAGE("The document did not become modified",
                       !sessionA->waitForMessage("statechanged: .uno:ModifiedStatus=true",
                                                 std::chrono::seconds(10))
                            .empty());
    sessionA->dropWS();
    LOK_ASSERT(sessionA->waitForDisconnection(std::chrono::seconds(10)));

    auto sessionA2 = connect(wopiSrc("renewed-token.odt", "alice-renewed"), token);
    const int viewIdA2 = waitForStatusViewId(*sessionA2, std::chrono::seconds(30));
    LOK_ASSERT_EQUAL(viewIdA, viewIdA2);

    _putFiles = 0;
    sessionA2->sendMessage("save dontTerminateEdit=1 dontSaveIfUnmodified=0");
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(30);
    while (_putFiles == 0 && std::chrono::steady_clock::now() < deadline)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    LOK_ASSERT_MESSAGE("The save did not reach the host", _putFiles.load() >= 1);
    LOK_ASSERT_EQUAL_MESSAGE("The upload did not use the renewed access token",
                             std::string("alice-renewed"), lastPutFileToken());

    sessionA2->exitWS();
    joinPollers();
    return TestResult::Ok;
}

UnitBase* unit_create_wsd(void) { return new UnitReconnectWopi(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
