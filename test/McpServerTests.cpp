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

#include <common/JsonUtil.hpp>
#include <wsd/McpServer.hpp>

#include <test/lokassert.hpp>

#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>

#include <cstddef>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

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
    CPPUNIT_TEST(testToolsListDescribesBothDocumentTools);
    CPPUNIT_TEST(testToolsListDescribesTheWritePathTools);
    CPPUNIT_TEST(testConvertArgumentsRequireAFormat);
    CPPUNIT_TEST(testTransformArgumentsRequireParsableJson);
    CPPUNIT_TEST(testToolsCallUnknownToolIsRejected);
    CPPUNIT_TEST(testToolsCallWithoutNameIsRejected);
    CPPUNIT_TEST(testToolsCallWithNonObjectArgumentsIsRejected);
    CPPUNIT_TEST(testToolFailureIsReportedAsAReadableResult);
    CPPUNIT_TEST(testToolSuccessIsReportedAsTextContent);
    CPPUNIT_TEST(testUnknownMethodIsRejected);
    CPPUNIT_TEST(testInvalidRequestsAreRejected);
    CPPUNIT_TEST(testUnsupportedProtocolHeaderIsRejected);
    CPPUNIT_TEST(testExtractArgumentsNameAViewAndACellRange);
    CPPUNIT_TEST_SUITE_END();

    void testInitializeNegotiatesProtocolVersion();
    void testInitializeDescribesTheServer();
    void testRequestIdKeepsItsJsonType();
    void testNotificationIsAcknowledgedWithNoBody();
    void testPingIsAnswered();
    void testToolsListDescribesBothDocumentTools();
    void testToolsListDescribesTheWritePathTools();
    void testConvertArgumentsRequireAFormat();
    void testTransformArgumentsRequireParsableJson();
    void testToolsCallUnknownToolIsRejected();
    void testToolsCallWithoutNameIsRejected();
    void testToolsCallWithNonObjectArgumentsIsRejected();
    void testToolFailureIsReportedAsAReadableResult();
    void testToolSuccessIsReportedAsTextContent();
    void testUnknownMethodIsRejected();
    void testInvalidRequestsAreRejected();
    void testUnsupportedProtocolHeaderIsRejected();
    void testExtractArgumentsNameAViewAndACellRange();

    /// Run one message through the core and return the single response it produces. Fails the test
    /// if the core does not answer.
    static McpServer::McpResponse call(const std::string& body,
                                       const std::string& protocolVersionHeader = std::string(),
                                       const std::vector<McpServer::McpTool>& tools = {})
    {
        constexpr std::string_view testname = "McpServerTests";
        McpServer::McpResponse captured;
        bool answered = false;
        McpServer::handleMessage(body, protocolVersionHeader, tools,
                                 [&captured, &answered](McpServer::McpResponse response)
                                 {
                                     captured = std::move(response);
                                     answered = true;
                                 });
        LOK_ASSERT_MESSAGE("the core must answer every message", answered);
        return captured;
    }

    /// The document tools with a stub executor attached to each that finishes at once, reporting
    /// either a success payload or a readable failure.
    static std::vector<McpServer::McpTool> toolsWithStubExecutor(bool succeed)
    {
        std::vector<McpServer::McpTool> tools = McpServer::documentToolDefinitions();
        for (McpServer::McpTool& tool : tools)
        {
            tool.executor = [succeed](const Poco::JSON::Object::Ptr&,
                                      const McpServer::McpToolCompletion& complete)
            {
                if (succeed)
                    complete({ /*isError=*/false, "the extracted document body" });
                else
                    complete({ /*isError=*/true, "the document could not be read" });
                return true;
            };
        }
        return tools;
    }

    static bool contains(const std::string& haystack, const std::string& needle)
    {
        return haystack.find(needle) != std::string::npos;
    }

    /// The tools/list answer for the document tools.
    static McpServer::McpResponse listTools()
    {
        return call(R"({"jsonrpc":"2.0","id":3,"method":"tools/list"})", std::string(),
                    McpServer::documentToolDefinitions());
    }

    /// The input schema of the tool named name in a tools/list response body, or null when the
    /// listing has no tool of that name.
    static Poco::JSON::Object::Ptr inputSchemaOf(const std::string& body, const std::string& name)
    {
        Poco::JSON::Object::Ptr root;
        if (!JsonUtil::parseJSON(body, root) || !root->getObject("result"))
            return nullptr;
        Poco::JSON::Array::Ptr tools = root->getObject("result")->getArray("tools");
        for (std::size_t index = 0; tools && index < tools->size(); ++index)
        {
            Poco::JSON::Object::Ptr tool = tools->getObject(index);
            if (tool && tool->optValue<std::string>("name", std::string()) == name)
                return tool->getObject("inputSchema");
        }
        return nullptr;
    }

    /// True when schema names property among its properties.
    static bool hasProperty(const Poco::JSON::Object::Ptr& schema, const std::string& property)
    {
        Poco::JSON::Object::Ptr properties = schema ? schema->getObject("properties") : nullptr;
        return properties && properties->has(property);
    }

    /// The names schema lists as required, in their order, or none when it lists none.
    static std::vector<std::string> requiredOf(const Poco::JSON::Object::Ptr& schema)
    {
        std::vector<std::string> names;
        Poco::JSON::Array::Ptr required = schema ? schema->getArray("required") : nullptr;
        for (std::size_t index = 0; required && index < required->size(); ++index)
            names.push_back(required->getElement<std::string>(index));
        return names;
    }

    /// The schema as JSON text.
    static std::string schemaText(const Poco::JSON::Object::Ptr& schema)
    {
        std::ostringstream stream;
        if (schema)
            schema->stringify(stream);
        return stream.str();
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

void McpServerTests::testToolsListDescribesBothDocumentTools()
{
    constexpr std::string_view testname = __func__;

    const McpServer::McpResponse response = listTools();
    LOK_ASSERT_EQUAL(200, response.statusCode);

    // Both document tools are listed, each with a description and an argument schema that takes the
    // document as data or as a file_id.
    LOK_ASSERT(contains(response.body, R"("description":)"));
    const Poco::JSON::Object::Ptr structure =
        inputSchemaOf(response.body, "extract_document_structure");
    const Poco::JSON::Object::Ptr targets = inputSchemaOf(response.body, "extract_link_targets");
    LOK_ASSERT(structure && targets);
    for (const Poco::JSON::Object::Ptr& schema : { structure, targets })
    {
        LOK_ASSERT(hasProperty(schema, "data"));
        LOK_ASSERT(hasProperty(schema, "file_id"));
        LOK_ASSERT(hasProperty(schema, "filename"));
    }

    // The structure tool also takes a view and the part of the document to read, and the link
    // targets tool takes neither.
    for (const char* property : { "filter", "range", "sheet", "target" })
    {
        LOK_ASSERT(hasProperty(structure, property));
        LOK_ASSERT(!hasProperty(targets, property));
    }
}

void McpServerTests::testToolsListDescribesTheWritePathTools()
{
    constexpr std::string_view testname = __func__;

    const McpServer::McpResponse response =
        call(R"({"jsonrpc":"2.0","id":10,"method":"tools/list"})", std::string(),
             McpServer::documentToolDefinitions());
    LOK_ASSERT_EQUAL(200, response.statusCode);

    // Both write-path tools are listed with their distinguishing arguments.
    LOK_ASSERT(contains(response.body, R"("name":"convert_document")"));
    LOK_ASSERT(contains(response.body, R"("name":"transform_document_structure")"));
    LOK_ASSERT(contains(response.body, R"("format")"));
    LOK_ASSERT(contains(response.body, R"("transform")"));
    // Each names its required argument in the schema.
    LOK_ASSERT(contains(response.body, R"("required":["format"])"));
    LOK_ASSERT(contains(response.body, R"("required":["transform"])"));
}

void McpServerTests::testConvertArgumentsRequireAFormat()
{
    constexpr std::string_view testname = __func__;

    // A convert call with no target format is a readable failure, not a usable request, so
    // validation reports it before any document work starts.
    {
        Poco::JSON::Object::Ptr arguments = new Poco::JSON::Object();
        arguments->set("data", "abc");
        const auto invalid = McpServer::validateConvertArguments(arguments);
        LOK_ASSERT(invalid.has_value());
        LOK_ASSERT(invalid->isError);
        LOK_ASSERT(contains(invalid->text, "format"));
    }

    // A convert call that names a format passes validation.
    {
        Poco::JSON::Object::Ptr arguments = new Poco::JSON::Object();
        arguments->set("data", "abc");
        arguments->set("format", "pdf");
        LOK_ASSERT(!McpServer::validateConvertArguments(arguments).has_value());
    }
}

void McpServerTests::testTransformArgumentsRequireParsableJson()
{
    constexpr std::string_view testname = __func__;

    // A transform that is not valid JSON is a readable failure so the calling model gets actionable
    // feedback before a document is loaded.
    {
        Poco::JSON::Object::Ptr arguments = new Poco::JSON::Object();
        arguments->set("data", "abc");
        arguments->set("transform", "{not valid json");
        const auto invalid = McpServer::validateTransformArguments(arguments);
        LOK_ASSERT(invalid.has_value());
        LOK_ASSERT(invalid->isError);
        LOK_ASSERT(contains(invalid->text, "JSON"));
    }

    // A missing transform is also reported.
    {
        Poco::JSON::Object::Ptr arguments = new Poco::JSON::Object();
        arguments->set("data", "abc");
        LOK_ASSERT(McpServer::validateTransformArguments(arguments).has_value());
    }

    // A transform that parses as JSON passes validation.
    {
        Poco::JSON::Object::Ptr arguments = new Poco::JSON::Object();
        arguments->set("data", "abc");
        arguments->set("transform", R"({"Transforms":{"SlideCommands":[]}})");
        LOK_ASSERT(!McpServer::validateTransformArguments(arguments).has_value());
    }
}

void McpServerTests::testToolsCallUnknownToolIsRejected()
{
    constexpr std::string_view testname = __func__;

    // A tool name that is not registered is a protocol error, not a tool result, so it comes back
    // as a JSON-RPC invalid-params error.
    const McpServer::McpResponse response =
        call(R"({"jsonrpc":"2.0","id":4,"method":"tools/call",)"
             R"("params":{"name":"no_such_tool","arguments":{}}})",
             std::string(), McpServer::documentToolDefinitions());
    LOK_ASSERT_EQUAL(200, response.statusCode);
    LOK_ASSERT(contains(response.body, "-32602"));
    LOK_ASSERT(!contains(response.body, R"("result")"));
}

void McpServerTests::testToolsCallWithoutNameIsRejected()
{
    constexpr std::string_view testname = __func__;

    // A call that names no tool is a protocol error.
    const McpServer::McpResponse response =
        call(R"({"jsonrpc":"2.0","id":4,"method":"tools/call","params":{}})", std::string(),
             McpServer::documentToolDefinitions());
    LOK_ASSERT_EQUAL(200, response.statusCode);
    LOK_ASSERT(contains(response.body, "-32602"));
}

void McpServerTests::testToolsCallWithNonObjectArgumentsIsRejected()
{
    constexpr std::string_view testname = __func__;

    // Arguments that are present but not an object are a protocol error, decided before any tool
    // runs.
    const McpServer::McpResponse response =
        call(R"({"jsonrpc":"2.0","id":4,"method":"tools/call",)"
             R"("params":{"name":"extract_link_targets","arguments":"oops"}})",
             std::string(), McpServer::documentToolDefinitions());
    LOK_ASSERT_EQUAL(200, response.statusCode);
    LOK_ASSERT(contains(response.body, "-32602"));
}

void McpServerTests::testToolFailureIsReportedAsAReadableResult()
{
    constexpr std::string_view testname = __func__;

    // A tool that cannot do its work reports a normal result with isError set and readable text, so
    // the calling model can read and react to it. It is not a JSON-RPC error.
    const McpServer::McpResponse response =
        call(R"({"jsonrpc":"2.0","id":8,"method":"tools/call",)"
             R"("params":{"name":"extract_document_structure","arguments":{}}})",
             std::string(), toolsWithStubExecutor(/*succeed=*/false));
    LOK_ASSERT_EQUAL(200, response.statusCode);
    LOK_ASSERT(!contains(response.body, R"("error")"));
    LOK_ASSERT(contains(response.body, R"("isError":true)"));
    LOK_ASSERT(contains(response.body, R"("type":"text")"));
    LOK_ASSERT(contains(response.body, "could not be read"));
}

void McpServerTests::testToolSuccessIsReportedAsTextContent()
{
    constexpr std::string_view testname = __func__;

    // A successful tool result carries its output as a text content block and is not marked as an
    // error.
    const McpServer::McpResponse response =
        call(R"({"jsonrpc":"2.0","id":9,"method":"tools/call",)"
             R"("params":{"name":"extract_link_targets","arguments":{}}})",
             std::string(), toolsWithStubExecutor(/*succeed=*/true));
    LOK_ASSERT_EQUAL(200, response.statusCode);
    LOK_ASSERT(contains(response.body, R"("isError":false)"));
    LOK_ASSERT(contains(response.body, R"("type":"text")"));
    LOK_ASSERT(contains(response.body, "the extracted document body"));
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

void McpServerTests::testExtractArgumentsNameAViewAndACellRange()
{
    constexpr std::string_view testname = __func__;

    // No filter, a documented view and a cell range all pass.
    for (const char* filter : { "", "text", "slides", "trackchanges" })
    {
        Poco::JSON::Object::Ptr arguments = new Poco::JSON::Object();
        if (*filter)
            arguments->set("filter", std::string(filter));
        arguments->set("range", "$A$1:D100");
        LOK_ASSERT(!McpServer::validateExtractArguments(arguments).has_value());
    }

    // A filter that is not a view, or that carries more after the view, is a failure that names the
    // filter.
    for (const char* filter : { "text&x=1", "text,range:A1", "nosuchview" })
    {
        Poco::JSON::Object::Ptr arguments = new Poco::JSON::Object();
        arguments->set("filter", std::string(filter));
        const auto invalid = McpServer::validateExtractArguments(arguments);
        LOK_ASSERT(invalid.has_value());
        LOK_ASSERT(contains(invalid->text, "filter argument"));
    }

    // A range with a space, a separator or a query character is a failure that names the range.
    for (const char* range : { "A1:B5 ", "A1,sheet:x", "A1&x=1" })
    {
        Poco::JSON::Object::Ptr arguments = new Poco::JSON::Object();
        arguments->set("range", std::string(range));
        const auto invalid = McpServer::validateExtractArguments(arguments);
        LOK_ASSERT(invalid.has_value());
        LOK_ASSERT(contains(invalid->text, "range argument"));
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(McpServerTests);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
