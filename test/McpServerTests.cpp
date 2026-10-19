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
    CPPUNIT_TEST(testToolsListDescribesTheDesignTemplateTool);
    CPPUNIT_TEST(testToolsListDescribesThePresentationTool);
    CPPUNIT_TEST(testConvertArgumentsRequireAFormat);
    CPPUNIT_TEST(testTransformArgumentsRequireParsableJson);
    CPPUNIT_TEST(testToolsCallUnknownToolIsRejected);
    CPPUNIT_TEST(testToolsCallWithoutNameIsRejected);
    CPPUNIT_TEST(testToolsCallWithNonObjectArgumentsIsRejected);
    CPPUNIT_TEST(testToolFailureIsReportedAsAReadableResult);
    CPPUNIT_TEST(testToolSuccessIsReportedAsTextContent);
    CPPUNIT_TEST(testPictureAndNoteFollowTheResult);
    CPPUNIT_TEST(testUnknownMethodIsRejected);
    CPPUNIT_TEST(testInvalidRequestsAreRejected);
    CPPUNIT_TEST(testUnsupportedProtocolHeaderIsRejected);
    CPPUNIT_TEST(testToolsListMarksEveryToolReadOnly);
    CPPUNIT_TEST(testDescriptionsNameTheEndpointAddress);
    CPPUNIT_TEST(testCommandArgumentsAreSingleTokens);
    CPPUNIT_TEST(testResourceAddressEncodesTheFileName);
    CPPUNIT_TEST(testExtractArgumentsNameAViewAndACellRange);
    CPPUNIT_TEST(testDownloadNameIsOfferedInPlainAndInFull);
    CPPUNIT_TEST_SUITE_END();

    void testInitializeNegotiatesProtocolVersion();
    void testInitializeDescribesTheServer();
    void testRequestIdKeepsItsJsonType();
    void testNotificationIsAcknowledgedWithNoBody();
    void testPingIsAnswered();
    void testToolsListDescribesBothDocumentTools();
    void testToolsListDescribesTheWritePathTools();
    void testToolsListDescribesTheDesignTemplateTool();
    void testToolsListDescribesThePresentationTool();
    void testConvertArgumentsRequireAFormat();
    void testTransformArgumentsRequireParsableJson();
    void testToolsCallUnknownToolIsRejected();
    void testToolsCallWithoutNameIsRejected();
    void testToolsCallWithNonObjectArgumentsIsRejected();
    void testToolFailureIsReportedAsAReadableResult();
    void testToolSuccessIsReportedAsTextContent();
    void testPictureAndNoteFollowTheResult();
    void testUnknownMethodIsRejected();
    void testInvalidRequestsAreRejected();
    void testUnsupportedProtocolHeaderIsRejected();
    void testToolsListMarksEveryToolReadOnly();
    void testDescriptionsNameTheEndpointAddress();
    void testCommandArgumentsAreSingleTokens();
    void testResourceAddressEncodesTheFileName();
    void testExtractArgumentsNameAViewAndACellRange();
    void testDownloadNameIsOfferedInPlainAndInFull();

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

    const McpServer::McpResponse response = listTools();
    LOK_ASSERT_EQUAL(200, response.statusCode);

    // A conversion requires the format to produce, and a transform requires the transform and takes
    // the format as an option. Both can return the document as a link.
    const Poco::JSON::Object::Ptr convert = inputSchemaOf(response.body, "convert_document");
    LOK_ASSERT(convert);
    LOK_ASSERT(hasProperty(convert, "format"));
    LOK_ASSERT(hasProperty(convert, "output"));
    LOK_ASSERT(requiredOf(convert) == std::vector<std::string>{ "format" });

    const Poco::JSON::Object::Ptr transform =
        inputSchemaOf(response.body, "transform_document_structure");
    LOK_ASSERT(transform);
    LOK_ASSERT(hasProperty(transform, "transform"));
    LOK_ASSERT(hasProperty(transform, "format"));
    LOK_ASSERT(hasProperty(transform, "output"));
    LOK_ASSERT(requiredOf(transform) == std::vector<std::string>{ "transform" });
}

void McpServerTests::testToolsListDescribesTheDesignTemplateTool()
{
    constexpr std::string_view testname = __func__;

    const McpServer::McpResponse response = listTools();
    LOK_ASSERT_EQUAL(200, response.statusCode);

    // The design listing takes no argument at all, so its schema carries an empty property set and
    // names nothing as required.
    const Poco::JSON::Object::Ptr schema = inputSchemaOf(response.body, "list_design_templates");
    LOK_ASSERT(schema);
    LOK_ASSERT(schema->getObject("properties"));
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(0), schema->getObject("properties")->size());
    LOK_ASSERT(requiredOf(schema).empty());
}

void McpServerTests::testToolsListDescribesThePresentationTool()
{
    constexpr std::string_view testname = __func__;

    const McpServer::McpResponse response = listTools();
    LOK_ASSERT_EQUAL(200, response.statusCode);

    // The deck is the one argument a caller must give; the design and the file format are optional.
    const Poco::JSON::Object::Ptr schema = inputSchemaOf(response.body, "create_presentation");
    LOK_ASSERT(schema);
    LOK_ASSERT(requiredOf(schema) == std::vector<std::string>{ "spec" });
    LOK_ASSERT(hasProperty(schema, "design"));
    LOK_ASSERT(hasProperty(schema, "format"));
    // The description carries the slide shape a caller writes the deck in.
    LOK_ASSERT(contains(response.body, R"(\"part\": one of)"));
    // So does the schema, so a caller that builds its arguments from the schema alone knows what a
    // slide holds and which names a part and an intent take.
    const std::string text = schemaText(schema);
    LOK_ASSERT(contains(text, R"("required":["slides"])"));
    LOK_ASSERT(contains(text, R"("required":["part","intent","title"])"));
    LOK_ASSERT(contains(text, R"("enum":["opening","divider","body","closing"])"));
    LOK_ASSERT(contains(text, R"("enum":["bullets","text"])"));
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

void McpServerTests::testPictureAndNoteFollowTheResult()
{
    constexpr std::string_view testname = __func__;

    // A picture of the result comes after the main block as an image a model can view, and a note
    // comes after that as text.
    std::vector<McpServer::McpTool> tools = McpServer::documentToolDefinitions();
    for (McpServer::McpTool& tool : tools)
    {
        tool.executor = [](const Poco::JSON::Object::Ptr&,
                           const McpServer::McpToolCompletion& complete)
        {
            McpServer::McpToolResult outcome(/*error=*/false, "the deck");
            outcome.previewPng = "the png bytes";
            outcome.note = "a remark";
            complete(std::move(outcome));
            return true;
        };
    }

    const McpServer::McpResponse response =
        call(R"({"jsonrpc":"2.0","id":10,"method":"tools/call",)"
             R"("params":{"name":"create_presentation","arguments":{}}})",
             std::string(), tools);
    LOK_ASSERT_EQUAL(200, response.statusCode);

    const std::size_t deck = response.body.find("the deck");
    const std::size_t image = response.body.find(R"("type":"image")");
    const std::size_t note = response.body.find("a remark");
    LOK_ASSERT(deck != std::string::npos);
    LOK_ASSERT(image != std::string::npos && image > deck);
    LOK_ASSERT(note != std::string::npos && note > image);
    LOK_ASSERT(contains(response.body, R"("mimeType":"image/png")"));
    LOK_ASSERT(contains(response.body, R"("data":")"));
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

void McpServerTests::testToolsListMarksEveryToolReadOnly()
{
    constexpr std::string_view testname = __func__;

    // Every tool reads what it is given and returns a new document, so each is marked read-only and
    // carries a title. No tool reaches outside the server.
    const McpServer::McpResponse response =
        call(R"({"jsonrpc":"2.0","id":3,"method":"tools/list"})", std::string(),
             toolsWithStubExecutor(true));
    Poco::JSON::Object::Ptr root;
    LOK_ASSERT(JsonUtil::parseJSON(response.body, root));
    Poco::JSON::Array::Ptr tools = root->getObject("result")->getArray("tools");
    LOK_ASSERT(tools && tools->size() == 6);
    for (std::size_t i = 0; i < tools->size(); ++i)
    {
        Poco::JSON::Object::Ptr tool = tools->getObject(i);
        LOK_ASSERT(!tool->optValue<std::string>("title", std::string()).empty());
        Poco::JSON::Object::Ptr annotations = tool->getObject("annotations");
        LOK_ASSERT(annotations);
        LOK_ASSERT(annotations->optValue<bool>("readOnlyHint", false));
        LOK_ASSERT(!annotations->optValue<bool>("openWorldHint", true));
    }
}

void McpServerTests::testDescriptionsNameTheEndpointAddress()
{
    constexpr std::string_view testname = __func__;

    // The description of a tool that takes a document names the upload address of the endpoint the
    // client reaches, once that address is put in.
    bool found = false;
    for (const McpServer::McpTool& tool : McpServer::documentToolDefinitions())
    {
        if (tool.name != McpServer::ToolName::ConvertDocument)
            continue;
        found = true;
        const std::string description = McpServer::withEndpointAddress(
            tool.description, "https://office.example/root/cool/mcp");
        LOK_ASSERT(contains(description, "https://office.example/root/cool/mcp/upload"));
        LOK_ASSERT(!contains(description, "{mcp_endpoint}"));
    }
    LOK_ASSERT(found);
}

void McpServerTests::testCommandArgumentsAreSingleTokens()
{
    constexpr std::string_view testname = __func__;

    const auto check = [](const std::string& json)
    {
        Poco::JSON::Object::Ptr arguments;
        JsonUtil::parseJSON(json, arguments);
        return McpServer::validateCommandArguments(arguments);
    };

    // A format is an extension and a lang is a language tag, and options sit on one line.
    LOK_ASSERT(!check(R"({"format":"pdf","lang":"en-US","options":"{\"a\": 1}"})"));
    LOK_ASSERT(!check(R"({})"));

    const std::optional<McpServer::McpToolResult> format = check(R"({"format":"pdf options=x"})");
    LOK_ASSERT(format && format->isError && contains(format->text, "format"));
    const std::optional<McpServer::McpToolResult> lang = check(R"({"lang":"en US"})");
    LOK_ASSERT(lang && lang->isError && contains(lang->text, "lang"));
    const std::optional<McpServer::McpToolResult> options = check(R"({"options":"a\nb"})");
    LOK_ASSERT(options && options->isError && contains(options->text, "options"));
}

void McpServerTests::testResourceAddressEncodesTheFileName()
{
    constexpr std::string_view testname = __func__;

    // A produced document whose name holds a space comes back under a valid address.
    std::vector<McpServer::McpTool> tools = McpServer::documentToolDefinitions();
    for (McpServer::McpTool& tool : tools)
    {
        tool.executor = [](const Poco::JSON::Object::Ptr&,
                           const McpServer::McpToolCompletion& complete)
        {
            McpServer::McpToolResult outcome;
            outcome.isDocument = true;
            outcome.documentBytes = "%PDF-";
            outcome.documentFilename = "my report.pdf";
            outcome.documentMimeType = "application/pdf";
            complete(std::move(outcome));
            return true;
        };
    }

    const McpServer::McpResponse response =
        call(R"({"jsonrpc":"2.0","id":11,"method":"tools/call",)"
             R"("params":{"name":"convert_document","arguments":{}}})",
             std::string(), tools);
    LOK_ASSERT(contains(response.body, "cool://converted/my%20report.pdf"));
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

void McpServerTests::testDownloadNameIsOfferedInPlainAndInFull()
{
    constexpr std::string_view testname = __func__;

    LOK_ASSERT_EQUAL_STR(R"(attachment; filename="report.pdf"; filename*=UTF-8''report.pdf)",
                         McpServer::contentDisposition("report.pdf"));

    // A name with letters outside ASCII and a backslash is plain ASCII in filename, and in full as
    // percent-encoded UTF-8 in filename*.
    LOK_ASSERT_EQUAL_STR(
        R"(attachment; filename="R__sum__ a_b.odt"; filename*=UTF-8''R%C3%A9sum%C3%A9%20a%5Cb.odt)",
        McpServer::contentDisposition("R\xc3\xa9sum\xc3\xa9 a\\b.odt"));
}

CPPUNIT_TEST_SUITE_REGISTRATION(McpServerTests);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
