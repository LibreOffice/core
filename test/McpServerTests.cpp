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

#include <wsd/McpServer.hpp>

#include <test/lokassert.hpp>

#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>

#include <string>
#include <string_view>

/// Whitebox tests that drive the transport-agnostic MCP core directly, with no HTTP layer in the
/// way.
class McpServerTests : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(McpServerTests);
    CPPUNIT_TEST(testInitializeNegotiatesProtocolVersion);
    CPPUNIT_TEST(testInitializeDescribesTheServer);
    CPPUNIT_TEST(testRequestIdKeepsItsJsonType);
    CPPUNIT_TEST(testNotificationIsAcknowledgedWithNoBody);
    CPPUNIT_TEST(testPingIsAnswered);
    CPPUNIT_TEST(testToolsListIsEmpty);
    CPPUNIT_TEST(testToolsCallHasNoTool);
    CPPUNIT_TEST(testUnknownMethodIsRejected);
    CPPUNIT_TEST(testInvalidRequestsAreRejected);
    CPPUNIT_TEST(testUnsupportedProtocolHeaderIsRejected);
    CPPUNIT_TEST_SUITE_END();

    void testInitializeNegotiatesProtocolVersion();
    void testInitializeDescribesTheServer();
    void testRequestIdKeepsItsJsonType();
    void testNotificationIsAcknowledgedWithNoBody();
    void testPingIsAnswered();
    void testToolsListIsEmpty();
    void testToolsCallHasNoTool();
    void testUnknownMethodIsRejected();
    void testInvalidRequestsAreRejected();
    void testUnsupportedProtocolHeaderIsRejected();

    /// Run one message through the core and return the single response it produces. Fails the test
    /// if the core does not answer.
    static McpServer::McpResponse call(const std::string& body,
                                       const std::string& protocolVersionHeader = std::string())
    {
        constexpr std::string_view testname = "McpServerTests";
        McpServer::McpResponse captured;
        bool answered = false;
        McpServer::handleMessage(body, protocolVersionHeader,
                                 [&captured, &answered](McpServer::McpResponse response)
                                 {
                                     captured = std::move(response);
                                     answered = true;
                                 });
        LOK_ASSERT_MESSAGE("the core must answer every message", answered);
        return captured;
    }

    static bool contains(const std::string& haystack, const std::string& needle)
    {
        return haystack.find(needle) != std::string::npos;
    }
};

void McpServerTests::testInitializeNegotiatesProtocolVersion()
{
    constexpr std::string_view testname = __func__;

    // A supported version the client asks for is echoed back unchanged.
    {
        const McpServer::McpResponse response =
            call(R"({"jsonrpc":"2.0","id":1,"method":"initialize",)"
                 R"("params":{"protocolVersion":"2025-06-18"}})");
        LOK_ASSERT_EQUAL(200, response.statusCode);
        LOK_ASSERT(contains(response.body, R"("protocolVersion":"2025-06-18")"));
    }
    {
        const McpServer::McpResponse response =
            call(R"({"jsonrpc":"2.0","id":1,"method":"initialize",)"
                 R"("params":{"protocolVersion":"2025-11-25"}})");
        LOK_ASSERT(contains(response.body, R"("protocolVersion":"2025-11-25")"));
    }

    // A version that takes batches of messages is not offered, so the newest supported one is.
    {
        const McpServer::McpResponse response =
            call(R"({"jsonrpc":"2.0","id":1,"method":"initialize",)"
                 R"("params":{"protocolVersion":"2025-03-26"}})");
        LOK_ASSERT(contains(response.body, R"("protocolVersion":"2025-11-25")"));
    }

    // A version we do not know falls back to the newest one we support.
    {
        const McpServer::McpResponse response =
            call(R"({"jsonrpc":"2.0","id":1,"method":"initialize",)"
                 R"("params":{"protocolVersion":"1999-01-01"}})");
        LOK_ASSERT(contains(response.body, R"("protocolVersion":"2025-11-25")"));
    }

    // No requested version also falls back to the newest supported one.
    {
        const McpServer::McpResponse response =
            call(R"({"jsonrpc":"2.0","id":1,"method":"initialize"})");
        LOK_ASSERT(contains(response.body, R"("protocolVersion":"2025-11-25")"));
    }
}

void McpServerTests::testInitializeDescribesTheServer()
{
    constexpr std::string_view testname = __func__;

    const McpServer::McpResponse response =
        call(R"({"jsonrpc":"2.0","id":1,"method":"initialize"})");
    LOK_ASSERT_EQUAL(200, response.statusCode);
    LOK_ASSERT(contains(response.body, R"("serverInfo")"));
    LOK_ASSERT(contains(response.body, R"("name":"Collabora Online")"));
    // Tools are advertised as a capability even though none are registered.
    LOK_ASSERT(contains(response.body, R"("tools":{})"));
}

void McpServerTests::testRequestIdKeepsItsJsonType()
{
    constexpr std::string_view testname = __func__;

    // A numeric id comes back as a JSON number, not a quoted string.
    {
        const McpServer::McpResponse response =
            call(R"({"jsonrpc":"2.0","id":7,"method":"ping"})");
        LOK_ASSERT(contains(response.body, R"("id":7)"));
        LOK_ASSERT(!contains(response.body, R"("id":"7")"));
    }

    // A string id comes back quoted.
    {
        const McpServer::McpResponse response =
            call(R"({"jsonrpc":"2.0","id":"abc","method":"ping"})");
        LOK_ASSERT(contains(response.body, R"("id":"abc")"));
    }

    // A message that cannot be parsed is answered with a null id.
    {
        const McpServer::McpResponse response = call(R"({not valid json)");
        LOK_ASSERT_EQUAL(200, response.statusCode);
        LOK_ASSERT(contains(response.body, R"("id":null)"));
        LOK_ASSERT(contains(response.body, "-32700"));
    }
}

void McpServerTests::testNotificationIsAcknowledgedWithNoBody()
{
    constexpr std::string_view testname = __func__;

    {
        const McpServer::McpResponse response =
            call(R"({"jsonrpc":"2.0","method":"notifications/initialized"})");
        LOK_ASSERT_EQUAL(202, response.statusCode);
        LOK_ASSERT(response.body.empty());
    }

    // Any message without an id is a notification, whatever its method.
    {
        const McpServer::McpResponse response = call(R"({"jsonrpc":"2.0","method":"tools/list"})");
        LOK_ASSERT_EQUAL(202, response.statusCode);
        LOK_ASSERT(response.body.empty());
    }

    // An answer from the client carries no method and gets no answer back.
    {
        const McpServer::McpResponse response =
            call(R"({"jsonrpc":"2.0","id":"s1","result":{}})");
        LOK_ASSERT_EQUAL(202, response.statusCode);
        LOK_ASSERT(response.body.empty());
    }
}

void McpServerTests::testPingIsAnswered()
{
    constexpr std::string_view testname = __func__;

    const McpServer::McpResponse response =
        call(R"({"jsonrpc":"2.0","id":2,"method":"ping"})");
    LOK_ASSERT_EQUAL(200, response.statusCode);
    LOK_ASSERT(contains(response.body, R"("result":{})"));
}

void McpServerTests::testToolsListIsEmpty()
{
    constexpr std::string_view testname = __func__;

    const McpServer::McpResponse response =
        call(R"({"jsonrpc":"2.0","id":3,"method":"tools/list"})");
    LOK_ASSERT_EQUAL(200, response.statusCode);
    LOK_ASSERT(contains(response.body, R"("tools":[])"));
}

void McpServerTests::testToolsCallHasNoTool()
{
    constexpr std::string_view testname = __func__;

    // With no tools registered yet, any call is answered as invalid params.
    const McpServer::McpResponse response =
        call(R"({"jsonrpc":"2.0","id":4,"method":"tools/call",)"
             R"("params":{"name":"whatever"}})");
    LOK_ASSERT_EQUAL(200, response.statusCode);
    LOK_ASSERT(contains(response.body, "-32602"));
}

void McpServerTests::testUnknownMethodIsRejected()
{
    constexpr std::string_view testname = __func__;

    const McpServer::McpResponse response =
        call(R"({"jsonrpc":"2.0","id":5,"method":"does/not/exist"})");
    LOK_ASSERT_EQUAL(200, response.statusCode);
    LOK_ASSERT(contains(response.body, "-32601"));
    LOK_ASSERT(contains(response.body, R"("id":5)"));
}

void McpServerTests::testInvalidRequestsAreRejected()
{
    constexpr std::string_view testname = __func__;

    // The wrong jsonrpc version is an invalid request.
    {
        const McpServer::McpResponse response =
            call(R"({"jsonrpc":"1.0","id":6,"method":"ping"})");
        LOK_ASSERT_EQUAL(200, response.statusCode);
        LOK_ASSERT(contains(response.body, "-32600"));
    }

    // An id must be a string or an integer, and the error names no id.
    {
        const McpServer::McpResponse response =
            call(R"({"jsonrpc":"2.0","id":null,"method":"ping"})");
        LOK_ASSERT_EQUAL(200, response.statusCode);
        LOK_ASSERT(contains(response.body, "-32600"));
        LOK_ASSERT(contains(response.body, R"("id":null)"));
    }
    {
        const McpServer::McpResponse response =
            call(R"({"jsonrpc":"2.0","id":{"a":1},"method":"ping"})");
        LOK_ASSERT(contains(response.body, "-32600"));
        LOK_ASSERT(contains(response.body, R"("id":null)"));
    }

    // A method that is not a string is an invalid request, not an unknown method.
    {
        const McpServer::McpResponse response =
            call(R"({"jsonrpc":"2.0","id":7,"method":7})");
        LOK_ASSERT_EQUAL(200, response.statusCode);
        LOK_ASSERT(contains(response.body, "-32600"));
        LOK_ASSERT(contains(response.body, R"("id":7)"));
    }

    // A batch, sent as a JSON array, is not supported.
    {
        const McpServer::McpResponse response =
            call(R"([{"jsonrpc":"2.0","id":1,"method":"ping"}])");
        LOK_ASSERT_EQUAL(200, response.statusCode);
        LOK_ASSERT(contains(response.body, "-32600"));
    }
}

void McpServerTests::testUnsupportedProtocolHeaderIsRejected()
{
    constexpr std::string_view testname = __func__;

    // A protocol version named in the header that we do not support is turned away before any
    // JSON-RPC processing.
    {
        const McpServer::McpResponse response =
            call(R"({"jsonrpc":"2.0","id":1,"method":"ping"})", "1999-09-09");
        LOK_ASSERT_EQUAL(400, response.statusCode);
    }

    // A supported version in the header lets the request through.
    {
        const McpServer::McpResponse response =
            call(R"({"jsonrpc":"2.0","id":1,"method":"ping"})", "2025-06-18");
        LOK_ASSERT_EQUAL(200, response.statusCode);
        LOK_ASSERT(contains(response.body, R"("result":{})"));
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(McpServerTests);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
