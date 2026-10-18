/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config.h>

#include <common/Log.hpp>
#include <common/Message.hpp>
#include <common/Unit.hpp>
#include <common/Util.hpp>
#include <net/HttpRequest.hpp>
#include <net/NetUtil.hpp>
#include <net/Socket.hpp>
#include <test/helpers.hpp>
#include <test/lokassert.hpp>
#include <wsd/COOLWSD.hpp>

#include <Poco/Util/LayeredConfiguration.h>

#include <atomic>
#include <chrono>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

/// A browser that loses its connection and comes back with the token the server gave it gets its
/// session and view back. One that does not come back loses them after the park timeout. One that
/// leaves on purpose, or never asked to reconnect, is not parked.
class UnitReconnect : public UnitWSD
{
    /// Above the ping interval of 18 seconds, so a client that answers is never dropped.
    static constexpr std::chrono::seconds PongTimeout{ 30 };
    static constexpr int ParkTimeoutSeconds = 3;
    /// Pong timeout plus one ping interval, with slack.
    static constexpr std::chrono::seconds DropNoticed{ 60 };

    std::vector<std::shared_ptr<TerminatingPoll>> _pollers;

    /// How many clipboard replies the kit sent. A session's removal asks for one, so this counts
    /// the removals.
    std::atomic<int> _clipboardReplies{ 0 };
    std::atomic<int> _uploads{ 0 };
    std::atomic<bool> _docDestroyed{ false };

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        UnitWSD::configure(config);
        config.setInt("net.session_park_timeout_secs", ParkTimeoutSeconds);
        net::Defaults.pongTimeout = PongTimeout;
    }

    bool onFilterLOKitMessage(const std::shared_ptr<Message>& message) override
    {
        const std::string_view text(message->data().data(), message->size());
        if (text.starts_with("clipboardcontent:"))
            ++_clipboardReplies;
        return false;
    }

    void onDocumentUploaded(bool success) override
    {
        if (success)
            ++_uploads;
    }

    void onDocBrokerDestroy(const std::string& docKey) override
    {
        // Each test's document is named after the test, so a document left over from an earlier
        // test does not count.
        if (docKey.find(getTestname()) != std::string::npos)
            _docDestroyed = true;
    }

    std::shared_ptr<SocketPoll> newPoller()
    {
        auto poller = std::make_shared<TerminatingPoll>(testname);
        poller->startThread();
        _pollers.push_back(poller);
        return poller;
    }

    /// A well-formed token that the server never gave out.
    static std::string makeToken() { return Util::toLower(Util::rng::getHexString(64)); }

    /// Connects and loads. A reconnectable browser loads with reconnect=true. The token, when
    /// given, rides on the URL.
    std::shared_ptr<http::WebSocketSession> connect(const std::string& documentURL,
                                                    bool reconnectable,
                                                    const std::string& token = std::string(),
                                                    const std::string& loadSuffix = std::string())
    {
        auto session = http::WebSocketSession::create(helpers::getTestServerURI());
        http::Request request(token.empty() ? documentURL
                                            : documentURL + "?ReconnectToken=" + token);
        session->asyncRequest(request, newPoller());
        session->sendMessage("load url=" + documentURL +
                             (reconnectable ? " reconnect=true" : "") + loadSuffix);
        return session;
    }

    /// The token of the next reconnecttoken: message, or empty. The server sends it before the
    /// view's status, so call this first. When sawCanonicalId is given, it is set if a
    /// canonicalidchange: message went by on the way to the token.
    std::string waitForToken(http::WebSocketSession& session, bool* sawCanonicalId = nullptr)
    {
        static constexpr std::string_view prefix = "reconnecttoken: ";
        const std::vector<char> message = session.poll(
            [&](const std::vector<char>& candidate) -> bool
            {
                const std::string_view text(candidate.data(), candidate.size());
                if (sawCanonicalId && text.starts_with("canonicalidchange:"))
                    *sawCanonicalId = true;
                return text.starts_with(prefix);
            },
            std::chrono::seconds(30), testname);
        if (message.size() <= prefix.size())
            return std::string();
        return std::string(message.begin() + prefix.size(), message.end());
    }

    /// Waits for the loaded: message, which comes after the view's status and its viewinfo. The
    /// session is live from then on, and the document is loaded. Until then the server takes a
    /// view for loading, and fails it when the last loaded view goes.
    bool waitForLoaded(http::WebSocketSession& session)
    {
        return !session.waitForMessage("loaded:", std::chrono::seconds(10), testname).empty();
    }

    /// Waits for a viewinfo with the given number of views. When parkedCount is not negative, that
    /// many of them must say parked:"true". sawParked, when given, is set if any viewinfo on the
    /// way had a parked view.
    bool waitForViews(http::WebSocketSession& session, size_t viewCount, int parkedCount,
                      std::chrono::milliseconds timeout, bool* sawParked = nullptr)
    {
        const std::vector<char> matched = session.poll(
            [&](const std::vector<char>& message) -> bool
            {
                const std::string_view text(message.data(), message.size());
                if (!text.starts_with("viewinfo: "))
                    return false;

                size_t count = 0;
                for (size_t pos = text.find("\"id\":"); pos != std::string_view::npos;
                     pos = text.find("\"id\":", pos + 1))
                    ++count;
                size_t parked = 0;
                for (size_t pos = text.find("\"parked\":\"true\""); pos != std::string_view::npos;
                     pos = text.find("\"parked\":\"true\"", pos + 1))
                    ++parked;

                TST_LOG("viewinfo with " << count << " views, " << parked << " parked: " << text);
                if (sawParked && parked > 0)
                    *sawParked = true;
                return count == viewCount &&
                       (parkedCount < 0 || parked == static_cast<size_t>(parkedCount));
            },
            timeout, testname);
        return !matched.empty();
    }

    /// The viewid in the next status: message, or -1. When sawCanonicalId is given, it is set if a
    /// canonicalidchange: message went by on the way to the status.
    static int waitForStatusViewId(http::WebSocketSession& session,
                                   std::chrono::milliseconds timeout,
                                   bool* sawCanonicalId = nullptr)
    {
        const std::vector<char> status = session.poll(
            [&](const std::vector<char>& message) -> bool
            {
                const std::string_view text(message.data(), message.size());
                if (sawCanonicalId && text.starts_with("canonicalidchange:"))
                    *sawCanonicalId = true;
                return text.starts_with("status:");
            },
            timeout, "status");
        if (status.empty())
            return -1;
        const std::string text(status.begin(), status.end());
        const auto pos = text.find("\"viewid\":");
        return pos == std::string::npos ? -1 : std::atoi(text.c_str() + pos + 9);
    }

    /// doc_views_active for the one open document, from the admin metrics.
    int activeViewsFromMetrics()
    {
        std::shared_ptr<http::Session> session = http::Session::create(helpers::getTestServerURI());
        auto poller = std::make_shared<TerminatingPoll>(testname);
        poller->runOnClientThread();
        http::Request request("/cool/getMetrics");
        request.setBasicAuth("admin", "admin");
        const std::shared_ptr<const http::Response> response =
            session->syncRequest(request, *poller);
        LOK_ASSERT_EQUAL(http::StatusCode::OK, response->statusCode());
        std::istringstream body(response->getBody());
        std::string line;
        while (std::getline(body, line))
            if (line.starts_with("doc_views_active{"))
                return std::atoi(line.substr(line.find("} ") + 2).c_str());
        return -1;
    }

    void joinPollers()
    {
        for (const auto& poller : _pollers)
            poller->joinThread();
        _pollers.clear();
    }

    TestResult testReconnectWithToken();
    TestResult testTokenRotatesOnReattach();
    TestResult testNotReconnectableIsNotParked();
    TestResult testParkExpires();
    TestResult testWrongToken();
    TestResult testUserExitIsNotParked();
    TestResult testOtherCloseIsParked();
    TestResult testTakeoverOfLiveSession();
    TestResult testLastEditorParkExpires();

public:
    UnitReconnect()
        : UnitWSD("UnitReconnect")
    {
        setTimeout(std::chrono::minutes(6));
    }

    /// Every case runs, so one failure does not hide another. The suite fails if any did.
    void invokeWSDTest() override
    {
        std::vector<std::string> failed;
        for (TestResult (UnitReconnect::*test)() :
             { &UnitReconnect::testUserExitIsNotParked,
               &UnitReconnect::testOtherCloseIsParked,
               &UnitReconnect::testTakeoverOfLiveSession,
               &UnitReconnect::testReconnectWithToken,
               &UnitReconnect::testTokenRotatesOnReattach,
               &UnitReconnect::testNotReconnectableIsNotParked,
               &UnitReconnect::testWrongToken,
               &UnitReconnect::testParkExpires,
               &UnitReconnect::testLastEditorParkExpires })
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

// (d) A close frame with status USER_EXIT means the user left. Nothing is parked.
UnitBase::TestResult UnitReconnect::testUserExitIsNotParked()
{
    setTestname(__func__);
    TST_LOG("Starting Test: " << testname);

    std::string documentPath, documentURL;
    helpers::getDocumentPathAndURL("hello.odt", documentPath, documentURL, testname);

    auto sessionA = connect(documentURL, true);
    LOK_ASSERT(!waitForToken(*sessionA).empty());
    LOK_ASSERT(waitForViews(*sessionA, 1, 0, std::chrono::seconds(30)));
    auto sessionB = connect(documentURL, false);
    LOK_ASSERT(waitForViews(*sessionB, 2, 0, std::chrono::seconds(30)));
    LOK_ASSERT(waitForLoaded(*sessionB));

    sessionA->exitWS();
    bool sawParked = false;
    LOK_ASSERT_MESSAGE("B did not see A leave at once",
                       waitForViews(*sessionB, 1, 0, std::chrono::seconds(5), &sawParked));
    LOK_ASSERT_MESSAGE("A view whose user left was parked", !sawParked);

    sessionB->shutdownWS();
    joinPollers();
    return TestResult::Ok;
}

// (k) A close frame with any other status can come from a proxy on the way. The view is parked, and
// the browser gets it back with its token.
UnitBase::TestResult UnitReconnect::testOtherCloseIsParked()
{
    setTestname(__func__);
    TST_LOG("Starting Test: " << testname);

    std::string documentPath, documentURL;
    helpers::getDocumentPathAndURL("hello.odt", documentPath, documentURL, testname);

    auto sessionA = connect(documentURL, true);
    const std::string token = waitForToken(*sessionA);
    LOK_ASSERT(!token.empty());
    const int viewIdA = waitForStatusViewId(*sessionA, std::chrono::seconds(30));
    LOK_ASSERT(viewIdA >= 0);
    LOK_ASSERT(waitForViews(*sessionA, 1, 0, std::chrono::seconds(30)));
    auto sessionB = connect(documentURL, false);
    LOK_ASSERT(waitForViews(*sessionB, 2, 0, std::chrono::seconds(30)));
    LOK_ASSERT(waitForLoaded(*sessionB));

    // shutdownWS sends ENDPOINT_GOING_AWAY.
    sessionA->shutdownWS();
    LOK_ASSERT_MESSAGE("B did not see A parked",
                       waitForViews(*sessionB, 2, 1, std::chrono::seconds(5)));

    auto sessionA2 = connect(documentURL, true, token, " part=0");
    LOK_ASSERT(!waitForToken(*sessionA2).empty());
    LOK_ASSERT_EQUAL(viewIdA, waitForStatusViewId(*sessionA2, std::chrono::seconds(10)));
    LOK_ASSERT_MESSAGE("B did not see A unparked",
                       waitForViews(*sessionB, 2, 0, std::chrono::seconds(10)));

    sessionA2->exitWS();
    sessionB->shutdownWS();
    joinPollers();
    return TestResult::Ok;
}

// (e) The browser is back before wsd noticed the dead socket. The live session with the same token
// takes the new connection and the old socket is closed.
UnitBase::TestResult UnitReconnect::testTakeoverOfLiveSession()
{
    setTestname(__func__);
    TST_LOG("Starting Test: " << testname);

    std::string documentPath, documentURL;
    helpers::getDocumentPathAndURL("hello.odt", documentPath, documentURL, testname);

    auto sessionA = connect(documentURL, true);
    const std::string token = waitForToken(*sessionA);
    LOK_ASSERT(!token.empty());
    const int viewIdA = waitForStatusViewId(*sessionA, std::chrono::seconds(30));
    LOK_ASSERT(viewIdA >= 0);
    LOK_ASSERT(waitForViews(*sessionA, 1, 0, std::chrono::seconds(30)));
    auto sessionB = connect(documentURL, false);
    LOK_ASSERT(waitForViews(*sessionB, 2, 0, std::chrono::seconds(30)));
    LOK_ASSERT(waitForLoaded(*sessionB));

    auto sessionA2 = connect(documentURL, true, token, " part=0");
    bool sawCanonicalId = false;
    const std::string newToken = waitForToken(*sessionA2, &sawCanonicalId);
    LOK_ASSERT_MESSAGE("The reconnected browser got no new token",
                       !newToken.empty() && newToken != token);
    const int viewIdA2 = waitForStatusViewId(*sessionA2, std::chrono::seconds(10), &sawCanonicalId);
    LOK_ASSERT_EQUAL(viewIdA, viewIdA2);
    // The browser rebuilt its tile layer, so it needs its canonical view id again.
    LOK_ASSERT_MESSAGE("The reconnected browser got no canonicalidchange",
                       sawCanonicalId || !sessionA2->waitForMessage("canonicalidchange:",
                                                                     std::chrono::seconds(10))
                                              .empty());
    LOK_ASSERT_MESSAGE("The old connection was not closed",
                       sessionA->waitForDisconnection(std::chrono::seconds(5)));
    // B saw no third view: the next viewinfo it gets still has two, none parked.
    LOK_ASSERT(waitForViews(*sessionB, 2, 0, std::chrono::seconds(10)));
    LOK_ASSERT_EQUAL(2, activeViewsFromMetrics());

    sessionA2->exitWS();
    sessionB->shutdownWS();
    joinPollers();
    return TestResult::Ok;
}

// (a) A frozen connection is parked, B sees it parked, A comes back and B sees it unparked.
UnitBase::TestResult UnitReconnect::testReconnectWithToken()
{
    setTestname(__func__);
    TST_LOG("Starting Test: " << testname);

    std::string documentPath, documentURL;
    helpers::getDocumentPathAndURL("hello.odt", documentPath, documentURL, testname);

    auto sessionA = connect(documentURL, true);
    const std::string token = waitForToken(*sessionA);
    LOK_ASSERT(!token.empty());
    const int viewIdA = waitForStatusViewId(*sessionA, std::chrono::seconds(30));
    LOK_ASSERT(viewIdA >= 0);
    LOK_ASSERT(waitForViews(*sessionA, 1, 0, std::chrono::seconds(30)));
    auto sessionB = connect(documentURL, false);
    LOK_ASSERT(waitForViews(*sessionB, 2, 0, std::chrono::seconds(30)));
    LOK_ASSERT(waitForLoaded(*sessionB));

    TST_LOG("Freezing A");
    sessionA->freeze();
    LOK_ASSERT_MESSAGE("B did not see A parked", waitForViews(*sessionB, 2, 1, DropNoticed));

    auto sessionA2 = connect(documentURL, true, token, " part=0");
    bool sawCanonicalId = false;
    const std::string newToken = waitForToken(*sessionA2, &sawCanonicalId);
    LOK_ASSERT_MESSAGE("The reconnected browser got no new token",
                       !newToken.empty() && newToken != token);
    const int viewIdA2 = waitForStatusViewId(*sessionA2, std::chrono::seconds(10), &sawCanonicalId);
    LOK_ASSERT_EQUAL(viewIdA, viewIdA2);
    LOK_ASSERT_MESSAGE("The reconnected browser got no canonicalidchange",
                       sawCanonicalId || !sessionA2->waitForMessage("canonicalidchange:",
                                                                     std::chrono::seconds(10))
                                              .empty());
    LOK_ASSERT_MESSAGE("B did not see A unparked",
                       waitForViews(*sessionB, 2, 0, std::chrono::seconds(10)));
    LOK_ASSERT_EQUAL(2, activeViewsFromMetrics());

    sessionA2->exitWS();
    sessionB->shutdownWS();
    joinPollers();
    return TestResult::Ok;
}

// (i) Each reattach gives the browser a new token. The token from just before it still works once,
// as the previous token, and a token two rotations old makes a new view.
UnitBase::TestResult UnitReconnect::testTokenRotatesOnReattach()
{
    setTestname(__func__);
    TST_LOG("Starting Test: " << testname);

    std::string documentPath, documentURL;
    helpers::getDocumentPathAndURL("hello.odt", documentPath, documentURL, testname);

    auto sessionA = connect(documentURL, true);
    const std::string token1 = waitForToken(*sessionA);
    LOK_ASSERT(!token1.empty());
    const int viewIdA = waitForStatusViewId(*sessionA, std::chrono::seconds(30));
    LOK_ASSERT(viewIdA >= 0);
    LOK_ASSERT(waitForLoaded(*sessionA));
    sessionA->dropWS();
    LOK_ASSERT(sessionA->waitForDisconnection(std::chrono::seconds(10)));

    auto sessionA2 = connect(documentURL, true, token1, " part=0");
    const std::string token2 = waitForToken(*sessionA2);
    LOK_ASSERT(!token2.empty() && token2 != token1);
    LOK_ASSERT_EQUAL(viewIdA, waitForStatusViewId(*sessionA2, std::chrono::seconds(10)));
    sessionA2->dropWS();
    LOK_ASSERT(sessionA2->waitForDisconnection(std::chrono::seconds(10)));

    // token1 is now the previous token.
    auto sessionA3 = connect(documentURL, true, token1, " part=0");
    const std::string token3 = waitForToken(*sessionA3);
    LOK_ASSERT(!token3.empty() && token3 != token2);
    LOK_ASSERT_EQUAL(viewIdA, waitForStatusViewId(*sessionA3, std::chrono::seconds(10)));
    sessionA3->dropWS();
    LOK_ASSERT(sessionA3->waitForDisconnection(std::chrono::seconds(10)));

    // Two rotations later, token1 names no view any more.
    auto sessionA4 = connect(documentURL, true, token1);
    LOK_ASSERT(!waitForToken(*sessionA4).empty());
    const int viewIdA4 = waitForStatusViewId(*sessionA4, std::chrono::seconds(30));
    LOK_ASSERT(viewIdA4 >= 0);
    LOK_ASSERT_MESSAGE("A token two rotations old took over the view", viewIdA4 != viewIdA);

    sessionA4->exitWS();
    joinPollers();
    return TestResult::Ok;
}

// (j) A browser whose load did not ask to reconnect has its dropped view go at once instead of
// being parked.
UnitBase::TestResult UnitReconnect::testNotReconnectableIsNotParked()
{
    setTestname(__func__);
    TST_LOG("Starting Test: " << testname);

    std::string documentPath, documentURL;
    helpers::getDocumentPathAndURL("hello.odt", documentPath, documentURL, testname);

    auto sessionA = connect(documentURL, false);
    LOK_ASSERT(waitForViews(*sessionA, 1, 0, std::chrono::seconds(30)));
    auto sessionB = connect(documentURL, false);
    LOK_ASSERT(waitForViews(*sessionB, 2, 0, std::chrono::seconds(30)));
    LOK_ASSERT(waitForLoaded(*sessionB));

    sessionA->dropWS();
    bool sawParked = false;
    LOK_ASSERT(waitForViews(*sessionB, 1, 0, std::chrono::seconds(ParkTimeoutSeconds + 10),
                            &sawParked));
    LOK_ASSERT_MESSAGE("A view that never asked to reconnect was parked", !sawParked);

    sessionB->shutdownWS();
    joinPollers();
    return TestResult::Ok;
}

// (c) A token the server never gave is a new user: a third view, until A's park expires.
UnitBase::TestResult UnitReconnect::testWrongToken()
{
    setTestname(__func__);
    TST_LOG("Starting Test: " << testname);

    std::string documentPath, documentURL;
    helpers::getDocumentPathAndURL("hello.odt", documentPath, documentURL, testname);

    auto sessionA = connect(documentURL, true);
    LOK_ASSERT(waitForViews(*sessionA, 1, 0, std::chrono::seconds(30)));
    auto sessionB = connect(documentURL, false);
    LOK_ASSERT(waitForViews(*sessionB, 2, 0, std::chrono::seconds(30)));
    LOK_ASSERT(waitForLoaded(*sessionB));

    sessionA->freeze();
    LOK_ASSERT(waitForViews(*sessionB, 2, 1, DropNoticed));

    auto sessionA2 = connect(documentURL, true, makeToken());
    LOK_ASSERT(waitForViews(*sessionB, 3, 1, std::chrono::seconds(30)));
    LOK_ASSERT(waitForViews(*sessionB, 2, 0, std::chrono::seconds(ParkTimeoutSeconds + 10)));

    sessionA2->exitWS();
    sessionB->shutdownWS();
    joinPollers();
    return TestResult::Ok;
}

// (b) Nobody comes back. The parked view goes after the park timeout.
UnitBase::TestResult UnitReconnect::testParkExpires()
{
    setTestname(__func__);
    TST_LOG("Starting Test: " << testname);

    std::string documentPath, documentURL;
    helpers::getDocumentPathAndURL("hello.odt", documentPath, documentURL, testname);

    auto sessionA = connect(documentURL, true);
    LOK_ASSERT(waitForViews(*sessionA, 1, 0, std::chrono::seconds(30)));
    auto sessionB = connect(documentURL, false);
    LOK_ASSERT(waitForViews(*sessionB, 2, 0, std::chrono::seconds(30)));
    LOK_ASSERT(waitForLoaded(*sessionB));

    sessionA->freeze();
    LOK_ASSERT(waitForViews(*sessionB, 2, 1, DropNoticed));
    LOK_ASSERT(waitForViews(*sessionB, 1, 0, std::chrono::seconds(ParkTimeoutSeconds + 10)));
    LOK_ASSERT_EQUAL(1, activeViewsFromMetrics());

    sessionB->shutdownWS();
    joinPollers();
    return TestResult::Ok;
}

// (f) The parked session was the only editor and the document has unsaved changes. Its expiry saves
// the document once, then the document unloads. The session is removed once.
UnitBase::TestResult UnitReconnect::testLastEditorParkExpires()
{
    setTestname(__func__);
    TST_LOG("Starting Test: " << testname);

    std::string documentPath, documentURL;
    helpers::getDocumentPathAndURL("hello.odt", documentPath, documentURL, testname);

    auto sessionA = connect(documentURL, true);
    LOK_ASSERT(waitForViews(*sessionA, 1, 0, std::chrono::seconds(30)));
    sessionA->sendMessage("key type=input char=97 key=0");
    LOK_ASSERT_MESSAGE("The document did not become modified",
                       !sessionA->waitForMessage("statechanged: .uno:ModifiedStatus=true",
                                                 std::chrono::seconds(10))
                            .empty());

    _clipboardReplies = 0;
    _uploads = 0;
    _docDestroyed = false;
    sessionA->dropWS();
    LOK_ASSERT(sessionA->waitForDisconnection(std::chrono::seconds(10)));

    const auto deadline =
        std::chrono::steady_clock::now() + std::chrono::seconds(ParkTimeoutSeconds + 60);
    while (!_docDestroyed && std::chrono::steady_clock::now() < deadline)
    {
        // A finished broker is removed the next time the prisoner poll wakes up, and nothing
        // else in this test wakes it.
        COOLWSD::doHousekeeping();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    LOK_ASSERT_MESSAGE("The document did not unload after the park expired",
                       _docDestroyed.load());
    LOK_ASSERT_MESSAGE("The last editor's changes were not saved", _uploads.load() >= 1);
    LOK_ASSERT_EQUAL_MESSAGE("The session was removed more than once", 1,
                             _clipboardReplies.load());

    joinPollers();
    return TestResult::Ok;
}

UnitBase* unit_create_wsd(void) { return new UnitReconnect(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
