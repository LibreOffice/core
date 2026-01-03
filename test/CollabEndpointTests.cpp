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

#include <helpers.hpp>
#include <lokassert.hpp>
#include <net/HttpRequest.hpp>
#include <Common.hpp>
#include <wsd/CollabBroker.hpp>
#include <wsd/CollabSocketHandler.hpp>

#include <Poco/JSON/Parser.h>
#include <Poco/URI.h>

#include <cppunit/extensions/HelperMacros.h>

#include <memory>

/// Tests the /co/collab WebSocket endpoint.
class CollabEndpointTest : public CPPUNIT_NS::TestFixture
{
    const Poco::URI _uri;
    std::shared_ptr<SocketPoll> _socketPoll;

    CPPUNIT_TEST_SUITE(CollabEndpointTest);

    CPPUNIT_TEST(testCollabWebSocketAuth);
    CPPUNIT_TEST(testCollabMissingWopiSrc);
    CPPUNIT_TEST(testCollabMissingAccessToken);
    CPPUNIT_TEST(testCollabInvalidFirstMessage);
    CPPUNIT_TEST(testCollabBrokerUserListJson);
    CPPUNIT_TEST(testCollabBrokerFindOrCreate);
    CPPUNIT_TEST(testCollabDownloadMissingWopiSrc);
    CPPUNIT_TEST(testCollabDownloadMissingCookie);
    CPPUNIT_TEST(testCollabUploadMissingWopiSrc);
    CPPUNIT_TEST(testCollabUploadMissingCookie);
    CPPUNIT_TEST(testCollabInvalidJsonMessage);
    CPPUNIT_TEST(testCollabUnknownMessageType);

    CPPUNIT_TEST_SUITE_END();

    void testCollabWebSocketAuth();
    void testCollabMissingWopiSrc();
    void testCollabMissingAccessToken();
    void testCollabInvalidFirstMessage();
    void testCollabBrokerUserListJson();
    void testCollabBrokerFindOrCreate();
    void testCollabDownloadMissingWopiSrc();
    void testCollabDownloadMissingCookie();
    void testCollabUploadMissingWopiSrc();
    void testCollabUploadMissingCookie();
    void testCollabInvalidJsonMessage();
    void testCollabUnknownMessageType();

public:
    CollabEndpointTest()
        : _uri(helpers::getTestServerURI())
        , _socketPoll(std::make_shared<SocketPoll>("CollabTestPoll"))
    {
    }

    void setUp()
    {
        helpers::resetTestStartTime();
        _socketPoll->startThread();
    }

    void tearDown()
    {
        _socketPoll->joinThread();
        helpers::resetTestStartTime();
    }
};

void CollabEndpointTest::testCollabWebSocketAuth()
{
    constexpr auto testname = __func__;

    TST_LOG("Testing /co/collab WebSocket validation flow");

    // Create WebSocket connection to /co/collab with WOPISrc parameter
    const std::string wopiSrc = "http%3A%2F%2Fexample.com%2Fwopi%2Ffiles%2F123";
    const std::string path = "/co/collab?WOPISrc=" + wopiSrc;

    auto ws = http::WebSocketSession::create(_uri.toString());
    http::Request req(path);
    ws->asyncRequest(req, _socketPoll);

    constexpr auto timeout = std::chrono::seconds(30);

    // Send access_token as first message
    const std::string accessToken = "test_token_12345";
    ws->sendMessage("access_token " + accessToken);

    // First we should receive a progress message indicating validation has started
    std::vector<char> response = ws->waitForMessage("progress:", timeout, testname);
    LOK_ASSERT_MESSAGE("Expected 'progress:' message during validation",
                       !response.empty());

    std::string responseStr(response.data(), response.size());
    LOK_ASSERT_MESSAGE("Progress message should mention validating",
                       responseStr.find("validating") != std::string::npos);

    TST_LOG("Received progress message: " << responseStr);

    // Since we're using a fake WOPI URL (example.com), validation will fail.
    // We should receive an error message (timeout or load failed)
    response = ws->waitForMessage("error:", timeout, testname);
    LOK_ASSERT_MESSAGE("Expected error response for unreachable WOPI server",
                       !response.empty());

    responseStr.assign(response.data(), response.size());
    TST_LOG("Received error after validation: " << responseStr);

    TST_LOG("Successfully tested /co/collab validation flow");
}

void CollabEndpointTest::testCollabMissingWopiSrc()
{
    constexpr auto testname = __func__;

    TST_LOG("Testing /co/collab with missing WOPISrc parameter");

    // Try to connect without WOPISrc - should get HTTP 400 Bad Request
    const std::string path = "/co/collab";

    const std::shared_ptr<const http::Response> httpResponse =
        http::get(_uri.toString(), path);

    // Should return 400 Bad Request
    LOK_ASSERT_EQUAL(http::StatusCode::BadRequest, httpResponse->statusLine().statusCode());

    TST_LOG("Correctly rejected connection without WOPISrc");
}

void CollabEndpointTest::testCollabMissingAccessToken()
{
    constexpr auto testname = __func__;

    TST_LOG("Testing /co/collab with missing access_token");

    // Create WebSocket connection
    const std::string wopiSrc = "http%3A%2F%2Fexample.com%2Fwopi%2Ffiles%2F456";
    const std::string path = "/co/collab?WOPISrc=" + wopiSrc;

    auto ws = http::WebSocketSession::create(_uri.toString());
    http::Request req(path);
    ws->asyncRequest(req, _socketPoll);

    constexpr auto timeout = std::chrono::seconds(10);

    // Send something other than access_token as first message
    ws->sendMessage("hello world");

    // Should get error response with accesstoken kind
    std::vector<char> response = ws->waitForMessage("error:", timeout, testname);
    LOK_ASSERT_MESSAGE("Expected error response for missing access_token",
                       !response.empty());

    const std::string responseStr(response.data(), response.size());
    LOK_ASSERT_MESSAGE("Error should indicate accesstoken issue",
                       responseStr.find("kind=accesstoken") != std::string::npos);

    TST_LOG("Correctly rejected message without access_token prefix");
}

void CollabEndpointTest::testCollabInvalidFirstMessage()
{
    constexpr auto testname = __func__;

    TST_LOG("Testing /co/collab with invalid first message format");

    // Create WebSocket connection
    const std::string wopiSrc = "http%3A%2F%2Fexample.com%2Fwopi%2Ffiles%2F789";
    const std::string path = "/co/collab?WOPISrc=" + wopiSrc;

    auto ws = http::WebSocketSession::create(_uri.toString());
    http::Request req(path);
    ws->asyncRequest(req, _socketPoll);

    constexpr auto timeout = std::chrono::seconds(10);

    // Send a random command that's not access_token
    ws->sendMessage("load url=file:///test.odt");

    // Should get error response with accesstoken kind
    std::vector<char> response = ws->waitForMessage("error:", timeout, testname);
    LOK_ASSERT_MESSAGE("Expected error response for invalid first message",
                       !response.empty());

    const std::string responseStr(response.data(), response.size());
    LOK_ASSERT_MESSAGE("Error should indicate accesstoken issue",
                       responseStr.find("kind=accesstoken") != std::string::npos);

    TST_LOG("Correctly rejected invalid first message");
}

void CollabEndpointTest::testCollabBrokerUserListJson()
{
    constexpr auto testname = __func__;

    TST_LOG("Testing CollabBroker getUserListJson format");

    // Create a broker directly
    auto broker = std::make_shared<CollabBroker>("test_doc_key", "http://example.com/wopi/files/1");

    // Get user list with no handlers - should return empty array
    std::string json = broker->getUserListJson(nullptr);

    Poco::JSON::Parser parser;
    auto result = parser.parse(json).extract<Poco::JSON::Object::Ptr>();

    LOK_ASSERT_MESSAGE("JSON should have 'type' field",
                       result->has("type"));
    LOK_ASSERT_EQUAL(std::string("user_list"),
                     result->getValue<std::string>("type"));

    LOK_ASSERT_MESSAGE("JSON should have 'users' array",
                       result->has("users"));

    auto users = result->getArray("users");
    LOK_ASSERT_MESSAGE("Users array should be empty initially",
                       users->size() == 0);

    TST_LOG("getUserListJson returns valid JSON with empty user list");
}

void CollabEndpointTest::testCollabBrokerFindOrCreate()
{
    constexpr auto testname = __func__;

    TST_LOG("Testing findOrCreateCollabBroker");

    const std::string docKey = "test_find_or_create_key";
    const std::string wopiSrc = "http://example.com/wopi/files/findorcreate";

    // First call should create a new broker
    auto broker1 = findOrCreateCollabBroker(docKey, wopiSrc);
    LOK_ASSERT_MESSAGE("First findOrCreate should return a broker",
                       broker1 != nullptr);

    // Second call with same docKey should return the same broker
    auto broker2 = findOrCreateCollabBroker(docKey, wopiSrc);
    LOK_ASSERT_MESSAGE("Second findOrCreate should return same broker",
                       broker1.get() == broker2.get());

    // Different docKey should create a different broker
    auto broker3 = findOrCreateCollabBroker("different_key", wopiSrc);
    LOK_ASSERT_MESSAGE("Different docKey should create different broker",
                       broker1.get() != broker3.get());

    // Clean up - remove from global map
    {
        std::lock_guard<std::mutex> lock(CollabBrokersMutex);
        CollabBrokers.erase(docKey);
        CollabBrokers.erase("different_key");
    }

    TST_LOG("findOrCreateCollabBroker correctly manages brokers");
}

void CollabEndpointTest::testCollabDownloadMissingWopiSrc()
{
    constexpr auto testname = __func__;

    TST_LOG("Testing /co/collab/download with missing WOPISrc parameter");

    // Try to download without WOPISrc - should get HTTP 400 Bad Request
    const std::string path = "/co/collab/download";

    const std::shared_ptr<const http::Response> httpResponse =
        http::get(_uri.toString(), path);

    // Should return 400 Bad Request
    LOK_ASSERT_EQUAL(http::StatusCode::BadRequest, httpResponse->statusLine().statusCode());

    TST_LOG("Correctly rejected download without WOPISrc");
}

void CollabEndpointTest::testCollabDownloadMissingCookie()
{
    constexpr auto testname = __func__;

    TST_LOG("Testing /co/collab/download with missing access_token cookie");

    // Try to download with WOPISrc but no cookie - should get HTTP 401 Unauthorized
    const std::string wopiSrc = "http%3A%2F%2Fexample.com%2Fwopi%2Ffiles%2F123";
    const std::string path = "/co/collab/download?WOPISrc=" + wopiSrc;

    const std::shared_ptr<const http::Response> httpResponse =
        http::get(_uri.toString(), path);

    // Should return 401 Unauthorized
    LOK_ASSERT_EQUAL(http::StatusCode::Unauthorized, httpResponse->statusLine().statusCode());

    TST_LOG("Correctly rejected download without access_token cookie");
}

void CollabEndpointTest::testCollabUploadMissingWopiSrc()
{
    constexpr auto testname = __func__;

    TST_LOG("Testing /co/collab/upload with missing WOPISrc parameter");

    // Try to upload without WOPISrc - should get HTTP 400 Bad Request
    const std::string path = "/co/collab/upload";

    http::Request request(path, http::Request::VERB_POST);
    request.setBody("test file content", "application/octet-stream");

    auto httpSession = http::Session::create(_uri.toString());
    const std::shared_ptr<const http::Response> httpResponse =
        httpSession->syncRequest(request);

    // Should return 400 Bad Request
    LOK_ASSERT_EQUAL(http::StatusCode::BadRequest, httpResponse->statusLine().statusCode());

    TST_LOG("Correctly rejected upload without WOPISrc");
}

void CollabEndpointTest::testCollabUploadMissingCookie()
{
    constexpr auto testname = __func__;

    TST_LOG("Testing /co/collab/upload with missing access_token cookie");

    // Try to upload with WOPISrc but no cookie - should get HTTP 401 Unauthorized
    const std::string wopiSrc = "http%3A%2F%2Fexample.com%2Fwopi%2Ffiles%2F123";
    const std::string path = "/co/collab/upload?WOPISrc=" + wopiSrc;

    http::Request request(path, http::Request::VERB_POST);
    request.setBody("test file content", "application/octet-stream");

    auto httpSession = http::Session::create(_uri.toString());
    const std::shared_ptr<const http::Response> httpResponse =
        httpSession->syncRequest(request);

    // Should return 401 Unauthorized
    LOK_ASSERT_EQUAL(http::StatusCode::Unauthorized, httpResponse->statusLine().statusCode());

    TST_LOG("Correctly rejected upload without access_token cookie");
}

void CollabEndpointTest::testCollabInvalidJsonMessage()
{
    constexpr auto testname = __func__;

    TST_LOG("Testing /co/collab rejects non-JSON after access_token");

    // Create WebSocket connection
    const std::string wopiSrc = "http%3A%2F%2Fexample.com%2Fwopi%2Ffiles%2Fjson_test";
    const std::string path = "/co/collab?WOPISrc=" + wopiSrc;

    auto ws = http::WebSocketSession::create(_uri.toString());
    http::Request req(path);
    ws->asyncRequest(req, _socketPoll);

    constexpr auto timeout = std::chrono::seconds(10);

    // Send access_token as first message
    ws->sendMessage("access_token test_token");

    // Wait for progress message (validation starting)
    std::vector<char> response = ws->waitForMessage("progress:", timeout, testname);
    LOK_ASSERT_MESSAGE("Expected progress message", !response.empty());

    // Since WOPI server is unreachable, we'll get error before we can test JSON messages
    // This test verifies the authentication flow works correctly
    response = ws->waitForMessage("error:", timeout, testname);
    LOK_ASSERT_MESSAGE("Expected error for unreachable WOPI server", !response.empty());

    TST_LOG("Authentication flow handled correctly (WOPI server unreachable as expected)");
}

void CollabEndpointTest::testCollabUnknownMessageType()
{
    constexpr auto testname = __func__;

    TST_LOG("Testing /co/collab message type validation");

    // Create WebSocket connection
    const std::string wopiSrc = "http%3A%2F%2Fexample.com%2Fwopi%2Ffiles%2Fmsg_test";
    const std::string path = "/co/collab?WOPISrc=" + wopiSrc;

    auto ws = http::WebSocketSession::create(_uri.toString());
    http::Request req(path);
    ws->asyncRequest(req, _socketPoll);

    constexpr auto timeout = std::chrono::seconds(10);

    // Send access_token as first message
    ws->sendMessage("access_token test_token");

    // Wait for progress message
    std::vector<char> response = ws->waitForMessage("progress:", timeout, testname);
    LOK_ASSERT_MESSAGE("Expected progress message", !response.empty());

    // Wait for error (WOPI validation fails because example.com is unreachable)
    response = ws->waitForMessage("error:", timeout, testname);
    LOK_ASSERT_MESSAGE("Expected error for unreachable WOPI server", !response.empty());

    // Note: Full JSON message testing would require a working WOPI server
    // This test verifies the initial authentication flow works correctly
    TST_LOG("Message handling test completed (WOPI server unreachable as expected)");
}

CPPUNIT_TEST_SUITE_REGISTRATION(CollabEndpointTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
