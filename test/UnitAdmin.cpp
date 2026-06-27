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
 * Unit test for admin console functionality.
 */

#include <config.h>

#include <common/Common.hpp>
#include <common/Log.hpp>
#include <common/Unit.hpp>
#include <common/Util.hpp>

#include <test/UnitHTTP.hpp>
#include <test/helpers.hpp>

#include <Poco/Net/HTTPBasicCredentials.h>
#include <Poco/Net/HTTPCookie.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/NameValueCollection.h>
#include <Poco/Net/NetException.h>
#include <Poco/URI.h>

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>

#define UNIT_URI "/coolwsd/unit-admin"

using Poco::Net::HTTPBasicCredentials;
using Poco::Net::HTTPCookie;
using Poco::Net::HTTPRequest;
using Poco::Net::HTTPResponse;
using Poco::Net::HTTPClientSession;

// Inside the WSD process
class UnitAdmin : public UnitWSD
{
private:
    unsigned _testCounter = 0;
    std::string _jwtCookie;
    bool _isTestRunning = false;
    const Poco::URI _uri;
    std::shared_ptr<COOLWebSocket> _adminWs;

    typedef TestResult (UnitAdmin::*AdminTest)(void);
    std::vector<AdminTest> _tests;

    std::shared_ptr<COOLWebSocket> _docWs1;
    std::shared_ptr<COOLWebSocket> _docWs2;
    std::shared_ptr<COOLWebSocket> _docWs3;
    int _docPid1;
    int _docPid2;
    int _docPid3;
    int _usersCount = 0;
    int _docsCount = 0;

    int  _messageTimeoutMilliSeconds = 5000;
    std::condition_variable _messageReceivedCV;
    std::mutex _messageReceivedMutex;
    std::string _messageReceived;

// Tests
private:
    TestResult testIncorrectPassword()
    {
        HTTPResponse response;
        std::string path(_uri.getPathAndQuery());
        HTTPRequest request(HTTPRequest::HTTP_GET, path);
        std::unique_ptr<HTTPClientSession> session(UnitHTTP::createSession());

        session->sendRequest(request);
        session->receiveResponse(response);
        TestResult res = TestResult::Failed;
        if (response.getStatus() == HTTPResponse::HTTP_UNAUTHORIZED)
            res = TestResult::Ok;

        LOG_INF("testIncorrectPassword: " << (res == TestResult::Ok ? "OK" : "FAIL"));
        return res;
    }

    TestResult testCorrectPassword()
    {
        HTTPResponse response;
        std::string path(_uri.getPathAndQuery());
        HTTPRequest request(HTTPRequest::HTTP_GET, path);
        std::unique_ptr<HTTPClientSession> session(UnitHTTP::createSession());
        HTTPBasicCredentials credentials("admin", "admin");
        credentials.authenticate(request);

        session->sendRequest(request);
        session->receiveResponse(response);
        std::vector<HTTPCookie> cookies;
        response.getCookies(cookies);

        // For now we only set one cookie
        LOK_ASSERT_EQUAL(1, static_cast<int>(cookies.size()));
        // and it is jwt=
        LOK_ASSERT_EQUAL_STR("jwt", cookies[0].getName());

        // Check cookie properties
        const std::string cookiePath = cookies[0].getPath();
        const bool secure = cookies[0].getSecure();
        const std::string value = cookies[0].getValue();
        TestResult res = TestResult::Failed;
        if (cookiePath.find("/browser/dist/admin/") == 0 && !value.empty() && secure)
        {
            // Set JWT cookie to be used for subsequent tests
            _jwtCookie = value;
            res = TestResult::Ok;
        }
        else
        {
            LOG_INF("testCorrectPassword: Invalid cookie properties");
        }

        LOG_INF("testCorrectPassword: " << (res == TestResult::Ok ? "OK" : "FAIL"));
        return res;
    }

    TestResult testWebSocketWithoutAuthToken()
    {
        // try connecting without authentication; should result in NotAuthenticated
        HTTPResponse response;
        HTTPRequest request(HTTPRequest::HTTP_GET, "/cool/adminws/");
        std::unique_ptr<HTTPClientSession> session(UnitHTTP::createSession());

        _adminWs = std::make_shared<COOLWebSocket>(*session, request, response);
        const std::string testMessage = "documents";
        std::unique_lock<std::mutex> lock(_messageReceivedMutex);
        _messageReceived.clear();
        _adminWs->sendFrame(testMessage.data(), testMessage.size());
        if (_messageReceivedCV.wait_for(lock, std::chrono::milliseconds(_messageTimeoutMilliSeconds)) == std::cv_status::timeout)
        {
            LOG_INF("testWebSocketWithoutAuth: Timed out waiting for admin console message");
            return TestResult::TimedOut;
        }
        lock.unlock();

        StringVector tokens(StringVector::tokenize(_messageReceived, ' '));
        if (tokens.size() != 1 ||
            tokens[0] != "NotAuthenticated")
        {
            LOG_INF("testWebSocketWithoutAuth: Unrecognized message format");
            return TestResult::Failed;
        }

        LOG_INF("testWebSocketWithoutAuth: OK");
        return TestResult::Ok;
    }

    TestResult testWebSocketWithIncorrectAuthToken()
    {
        // try connecting with incorrect auth token; should result in InvalidToken
        HTTPResponse response;
        HTTPRequest request(HTTPRequest::HTTP_GET, "/cool/adminws/");
        std::unique_ptr<HTTPClientSession> session(UnitHTTP::createSession());

        _adminWs = std::make_shared<COOLWebSocket>(*session, request, response);
        const std::string testMessage = "auth jwt=incorrectJWT";
        std::unique_lock<std::mutex> lock(_messageReceivedMutex);
        _messageReceived.clear();
        _adminWs->sendFrame(testMessage.data(), testMessage.size());
        if (_messageReceivedCV.wait_for(lock, std::chrono::milliseconds(_messageTimeoutMilliSeconds)) == std::cv_status::timeout)
        {
            LOG_INF("testWebSocketWithIncorrectAuthToken: Timed out waiting for admin console message");
            return TestResult::TimedOut;
        }
        lock.unlock();

        StringVector tokens(StringVector::tokenize(_messageReceived, ' '));
        if (tokens.size() != 1 ||
            tokens[0] != "InvalidAuthToken")
        {
            LOG_INF("testWebSocketWithIncorrectAuthToken: Unrecognized message format");
            return TestResult::Failed;
        }

        LOG_INF("testWebSocketWithIncorrectAuthToken: OK");
        return TestResult::Ok;
    }

    TestResult testAddDocNotify()
    {
        // Authenticate first
        HTTPResponse response;
        HTTPRequest request(HTTPRequest::HTTP_GET, "/cool/adminws/");
        std::unique_ptr<HTTPClientSession> session(UnitHTTP::createSession());

        _adminWs = std::make_shared<COOLWebSocket>(*session, request, response);
        const std::string authMessage = "auth jwt=" + _jwtCookie;
        _adminWs->sendFrame(authMessage.data(), authMessage.size());

        // subscribe notification on admin websocket
        const std::string subscribeMessage = "subscribe adddoc";
        _adminWs->sendFrame(subscribeMessage.data(), subscribeMessage.size());

        // FIXME: we really should wait for the subscription to be
        // registered and have a reply to avoid a race here.
        std::this_thread::sleep_for(250ms);

        std::string documentPath1, documentURL1;
        helpers::getDocumentPathAndURL("hello.odt", documentPath1, documentURL1, "unitAdmin-hello.odt ");
        HTTPRequest request1(HTTPRequest::HTTP_GET, documentURL1);
        HTTPResponse response1;
        const Poco::URI docUri1(helpers::getTestServerURI());
        const std::string loadMessage1 = "load url=" + documentURL1;
        std::unique_ptr<HTTPClientSession> session1(UnitHTTP::createSession());
        std::unique_ptr<HTTPClientSession> session2(UnitHTTP::createSession());

        std::unique_lock<std::mutex> lock(_messageReceivedMutex);
        _messageReceived.clear();
        _docWs1 = std::make_shared<COOLWebSocket>(*session1, request1, response1);
        _docWs1->sendFrame(loadMessage1.data(), loadMessage1.size());
        if (_messageReceivedCV.wait_for(lock, std::chrono::milliseconds(_messageTimeoutMilliSeconds)) == std::cv_status::timeout)
        {
            LOG_INF("testAddDocNotify: Timed out waiting for admin console message");
            return TestResult::TimedOut;
        }
        lock.unlock();

        {
            StringVector tokens(StringVector::tokenize(_messageReceived, ' '));
            if (tokens.size() != 5 ||
                tokens[0] != "adddoc" ||
                tokens[2] != documentPath1.substr(documentPath1.find_last_of('/') + 1) )
            {
                LOG_INF("testAddDocNotify: Unrecognized message format");
                return TestResult::Failed;
            }

            // store document pid
            _docPid1 = NumUtil::stoi(tokens[1]);
            _usersCount++;
        }
        _docsCount++;

        // Open another view of same document
        lock.lock(); // lock _messageReceivedMutex
        _messageReceived.clear();
        _docWs2 = std::make_shared<COOLWebSocket>(*session2, request1, response1);
        _docWs2->sendFrame(loadMessage1.data(), loadMessage1.size());
        if (_messageReceivedCV.wait_for(lock, std::chrono::milliseconds(_messageTimeoutMilliSeconds)) == std::cv_status::timeout)
        {
            LOG_INF("testAddDocNotify: Timed out waiting for admin console message");
            return TestResult::TimedOut;
        }
        lock.unlock();

        {
            StringVector tokens(StringVector::tokenize(_messageReceived, ' '));
            if (tokens.size() != 5 ||
                tokens[0] != "adddoc" ||
                tokens[2] != documentPath1.substr(documentPath1.find_last_of('/') + 1) )
            {
                LOG_INF("testAddDocNotify: Unrecognized message format");
                return TestResult::Failed;
            }

            // store document pid
            _docPid2 = NumUtil::stoi(tokens[1]);
            _usersCount++;
        }

        // Open another document (different)
        std::string documentPath2, documentURL2;
        helpers::getDocumentPathAndURL("insert-delete.odp", documentPath2, documentURL2, "unitAdmin-insert-delete.odp ");
        HTTPRequest request2(HTTPRequest::HTTP_GET, documentURL2);
        HTTPResponse response2;
        const Poco::URI docUri2(helpers::getTestServerURI());
        const std::string loadMessage2 = "load url=" + documentURL2;
        std::unique_ptr<HTTPClientSession> session3(UnitHTTP::createSession());

        lock.lock(); // lock _messageReceivedMutex
        _messageReceived.clear();
        _docWs3 = std::make_shared<COOLWebSocket>(*session3, request2, response2);
        _docWs3->sendFrame(loadMessage2.data(), loadMessage2.size());
        if (_messageReceivedCV.wait_for(lock, std::chrono::milliseconds(_messageTimeoutMilliSeconds)) == std::cv_status::timeout)
        {
            LOG_INF("testAddDocNotify: Timed out waiting for admin console message");
            return TestResult::TimedOut;
        }
        lock.unlock();

        {
            StringVector tokens(StringVector::tokenize(_messageReceived, ' '));
            if (tokens.size() != 5 ||
                tokens[0] != "adddoc" ||
                tokens[2] != documentPath2.substr(documentPath2.find_last_of('/') + 1) )
            {
                LOG_INF("testAddDocNotify: Unrecognized message format");
                return TestResult::Failed;
            }

            // store document pid
            _docPid3 = NumUtil::stoi(tokens[1]);
            _usersCount++;
        }
        _docsCount++;

        LOG_INF("testAddDocNotify: OK");
        return TestResult::Ok;
    }

    TestResult testUsersCount()
    {
        const std::string queryMessage = "active_users_count";
        _messageReceived.clear();

        std::unique_lock<std::mutex> lock(_messageReceivedMutex);
        _adminWs->sendFrame(queryMessage.data(), queryMessage.size());
        if (_messageReceivedCV.wait_for(lock, std::chrono::milliseconds(_messageTimeoutMilliSeconds)) == std::cv_status::timeout)
        {
            LOG_INF("testUsersCount: Timed out waiting for admin console message");
            return TestResult::TimedOut;
        }
        lock.unlock();

        StringVector tokens(StringVector::tokenize(_messageReceived, ' '));
        if (tokens.size() != 2 ||
            tokens[0] != "active_users_count")
        {
            LOG_INF("testUsersCount: Unrecognized message format");
            return TestResult::Failed;
        }
        else if (NumUtil::stoi(tokens[1]) != _usersCount)
        {
            LOG_INF("testUsersCount: Incorrect users count "
                      ", expected: " + std::to_string(_usersCount) +
                      ", actual: " + tokens[1]);
            return TestResult::Failed;
        }

        LOG_INF("testUsersCount: OK");
        return TestResult::Ok;
    }

    TestResult testDocCount()
    {
        const std::string queryMessage = "active_docs_count";
        _messageReceived.clear();

        std::unique_lock<std::mutex> lock(_messageReceivedMutex);
        _adminWs->sendFrame(queryMessage.data(), queryMessage.size());
        if (_messageReceivedCV.wait_for(lock, std::chrono::milliseconds(_messageTimeoutMilliSeconds)) == std::cv_status::timeout)
        {
            LOG_INF("testDocCount: Timed out waiting for admin console message");
            return TestResult::TimedOut;
        }
        lock.unlock();

        StringVector tokens(StringVector::tokenize(_messageReceived, ' '));
        if (tokens.size() != 2 || tokens[0] != "active_docs_count" ||
            NumUtil::stoi(tokens[1]) != _docsCount)
        {
            LOG_INF("testDocCount: Unrecognized message format");
            return TestResult::Failed;
        }
        else if (NumUtil::stoi(tokens[1]) != _docsCount)
        {
            LOG_INF("testDocCount: Incorrect doc count "
                      ", expected: " + std::to_string(_docsCount) +
                      ", actual: " + tokens[1]);
            return TestResult::Failed;
        }

        LOG_INF("testDocCount: OK");
        return TestResult::Ok;
    }

    TestResult testRmDocNotify()
    {
        // subscribe to rmdoc notification on admin websocket
        const std::string subscribeMessage = "subscribe rmdoc";
        _adminWs->sendFrame(subscribeMessage.data(), subscribeMessage.size());
        _messageReceived.clear();

        std::unique_lock<std::mutex> lock(_messageReceivedMutex);
        _docWs1->close();
        if (_messageReceivedCV.wait_for(lock, std::chrono::milliseconds(_messageTimeoutMilliSeconds)) == std::cv_status::timeout)
        {
            LOG_INF("testRmDocNotify: Timed out waiting for admin console message");
            return TestResult::TimedOut;
        }
        lock.unlock();

        StringVector tokens(StringVector::tokenize(_messageReceived, ' '));
        if (tokens.size() != 3 ||
            tokens[0] != "rmdoc" ||
            stoi(tokens[1]) != _docPid1)
        {
            LOG_INF("testRmDocNotify: Invalid message format");
            return TestResult::Failed;
        }
        _usersCount--;

        LOG_INF("testRmDocNotify: OK");
        return TestResult::Ok;
    }

public:
    UnitAdmin()
        : _uri(helpers::getTestServerURI() + "/browser/dist/admin/admin.html")
    {
        setTimeout(60s);

        // Register tests here.
        _tests.push_back(&UnitAdmin::testIncorrectPassword);
#if ENABLE_SSL
        _tests.push_back(&UnitAdmin::testCorrectPassword);
#endif
        _tests.push_back(&UnitAdmin::testWebSocketWithoutAuthToken);
#if ENABLE_SSL
        _tests.push_back(&UnitAdmin::testWebSocketWithIncorrectAuthToken);
        _tests.push_back(&UnitAdmin::testAddDocNotify);
        _tests.push_back(&UnitAdmin::testUsersCount);
        _tests.push_back(&UnitAdmin::testDocCount);
        _tests.push_back(&UnitAdmin::testRmDocNotify);
        _tests.push_back(&UnitAdmin::testUsersCount);
        _tests.push_back(&UnitAdmin::testDocCount);
#endif
    }

    // Runs tests sequentially in _tests
    virtual void invokeWSDTest()
    {
        if (!_isTestRunning)
        {
            _isTestRunning = true;
            AdminTest test = _tests[_testCounter++];
            LOG_INF("UnitAdmin:: Starting test #" << _testCounter);
            TestResult res = ((*this).*(test))();
            LOG_INF("UnitAdmin:: Finished test #" << _testCounter);
            if (res != TestResult::Ok)
            {
                LOG_INF("Exiting with " <<
                        (res == TestResult::Failed
                              ? "FAIL"
                              : (res == TestResult::TimedOut)
                                      ? "TIMEOUT"
                                      : "??? (" + std::to_string((int)res) + ')'));
                exitTest(res);
                assert(false);
                return;
            }

            // End this when all tests are finished
            if (_tests.size() == _testCounter)
            {
                LOG_INF("Exiting with OK");
                exitTest(TestResult::Ok);
            }

            _isTestRunning = false;
        }
    }

    virtual void onAdminNotifyMessage(const std::string& message)
    {
        std::unique_lock<std::mutex> lock(_messageReceivedMutex);
        _messageReceived = message;
        _messageReceivedCV.notify_all();
        LOG_INF("UnitAdmin:: onAdminNotifyMessage: " << message);
    }

    virtual void onAdminQueryMessage(const std::string& message)
    {
        std::unique_lock<std::mutex> lock(_messageReceivedMutex);
        _messageReceived = message;
        _messageReceivedCV.notify_all();
        LOG_INF("UnitAdmin:: onAdminQueryMessage: " << message);
    }
};

UnitBase *unit_create_wsd(void)
{
    return new UnitAdmin();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
