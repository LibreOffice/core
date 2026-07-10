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
 * End to end test for the Model Context Protocol endpoint: the handshake, the tool listing, running
 * each document tool over a document passed as base64 bytes, and the failure and authentication
 * paths.
 */

#include <config.h>

#include <cstdint>
#include <fstream>
#include <string>
#include <thread>
#include <typeinfo>

#include <Common.hpp>
#include <Protocol.hpp>
#include <Unit.hpp>
#include <common/Util.hpp>
#include <helpers.hpp>
#include <wsd/McpTools.hpp>

#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Path.h>
#include <Poco/Util/LayeredConfiguration.h>

using namespace std::literals;

namespace
{
const std::string ApiKey = "secret123";

bool contains(const std::string& haystack, const std::string& needle)
{
    return haystack.find(needle) != std::string::npos;
}

std::string readWholeFile(const std::string& path)
{
    std::ifstream stream(path, std::ios::binary);
    return std::string(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>());
}
}

/// Inside the WSD process.
class UnitMcp : public UnitWSD
{
    bool _workerStarted;
    std::thread _worker;

public:
    UnitMcp()
        : UnitWSD("UnitMcp")
        , _workerStarted(false)
    {
        setHasKitHooks();
        setTimeout(1h);
    }

    ~UnitMcp()
    {
        if (_worker.joinable())
            _worker.join();
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        UnitWSD::configure(config);

        config.setBool("ssl.enable", true);
        config.setInt("per_document.limit_load_secs", 30);
        config.setBool("storage.filesystem[@allow]", false);

        // Switch the endpoint on and give it a key to require.
        config.setBool("ai.mcp.enable", true);
        config.setString("ai.mcp.api_key", ApiKey);
    }

    /// Send jsonBody to the message address path, with bearer as the Authorization header, or with
    /// no Authorization header when bearer is empty.
    std::shared_ptr<const http::Response>
    mcpCall(const std::shared_ptr<http::Session>& session, const std::string& jsonBody,
            const std::string& bearer = "Bearer " + ApiKey, const std::string& path = "/cool/mcp")
    {
        http::Request request(path, http::Request::VERB_POST);
        if (!bearer.empty())
            request.set("Authorization", bearer);
        request.setBody(jsonBody, "application/json");
        return session->syncRequest(request);
    }

    /// A POST to path with the key whose header states a body of length bytes. No byte of the body
    /// is ever sent.
    static http::Request headerOnlyPost(const std::string& path, std::int64_t length)
    {
        http::Request request(path, http::Request::VERB_POST);
        request.set("Authorization", "Bearer " + ApiKey);
        request.setBodySource([](char*, std::int64_t) { return 0; }, length);
        return request;
    }

    std::shared_ptr<const http::Response> mcpGet(const std::shared_ptr<http::Session>& session,
                                                 const std::string& path,
                                                 const std::string& bearer = "Bearer " + ApiKey)
    {
        http::Request request(path, http::Request::VERB_GET);
        request.set("Authorization", bearer);
        return session->syncRequest(request);
    }

    /// The content block at the given position in a tool result, as an object, or null.
    static Poco::JSON::Object::Ptr contentBlock(const std::string& body, std::size_t index)
    {
        try
        {
            Poco::JSON::Parser parser;
            auto root = parser.parse(body).extract<Poco::JSON::Object::Ptr>();
            auto result = root->getObject("result");
            if (!result)
                return nullptr;
            auto content = result->getArray("content");
            if (!content || content->size() <= index)
                return nullptr;
            return content->getObject(index);
        }
        catch (const std::exception&)
        {
            return nullptr;
        }
    }

    /// The first content block of a tool result, as an object, or null.
    static Poco::JSON::Object::Ptr firstContentBlock(const std::string& body)
    {
        return contentBlock(body, 0);
    }

    /// The text of a tool result carried in a text content block.
    static std::string textResultFrom(const std::string& body)
    {
        auto block = firstContentBlock(body);
        if (!block)
            return std::string();
        return block->optValue<std::string>("text", std::string());
    }

    void runTest(const std::shared_ptr<http::Session>& session)
    {
        const std::string helloPath = Poco::Path(TDOC, "hello.odt").toString();
        const std::string helloData = Util::base64Encode(readWholeFile(helloPath));

        // The handshake negotiates a supported protocol version and returns the numeric request id
        // unchanged.
        {
            auto response = mcpCall(
                session, R"({"jsonrpc":"2.0","id":1,"method":"initialize",)"
                         R"("params":{"protocolVersion":"2025-06-18"}})");
            LOK_ASSERT(response->statusCode() == http::StatusCode::OK);
            LOK_ASSERT(contains(response->getBody(), R"("protocolVersion":"2025-06-18")"));
            LOK_ASSERT(contains(response->getBody(), R"("id":1)"));
        }

        // The tool listing names both document tools.
        {
            auto response =
                mcpCall(session, R"({"jsonrpc":"2.0","id":2,"method":"tools/list"})");
            LOK_ASSERT(response->statusCode() == http::StatusCode::OK);
            LOK_ASSERT(contains(response->getBody(), R"("name":"extract_document_structure")"));
            LOK_ASSERT(contains(response->getBody(), R"("name":"extract_link_targets")"));
        }

        // Extracting the text of a document passed as base64 bytes returns its body, and is not
        // marked as an error.
        {
            auto response = mcpCall(
                session, R"({"jsonrpc":"2.0","id":3,"method":"tools/call","params":)"
                         R"({"name":"extract_document_structure","arguments":)"
                         R"({"data":")" + helloData + R"(","filename":"hello.odt",)"
                         R"("filter":"text"}}})");
            LOK_ASSERT(response->statusCode() == http::StatusCode::OK);
            LOK_ASSERT(contains(response->getBody(), R"("isError":false)"));
            LOK_ASSERT(contains(response->getBody(), "Hello world"));
        }

        // Listing link targets of a document passed as base64 bytes returns a categorised result.
        {
            auto response = mcpCall(
                session, R"({"jsonrpc":"2.0","id":5,"method":"tools/call","params":)"
                         R"({"name":"extract_link_targets","arguments":)"
                         R"({"data":")" + helloData + R"(","filename":"hello.odt"}}})");
            LOK_ASSERT(response->statusCode() == http::StatusCode::OK);
            LOK_ASSERT(contains(response->getBody(), R"("isError":false)"));
            LOK_ASSERT(contains(response->getBody(), "Headings"));
        }

        // Listing the link targets of a spreadsheet answers with its targets.
        {
            const std::string sheetPath = Poco::Path(TDOC, "convert-to.xlsx").toString();
            const std::string sheetData = Util::base64Encode(readWholeFile(sheetPath));
            auto targets = mcpCall(
                session, R"({"jsonrpc":"2.0","id":17,"method":"tools/call","params":)"
                         R"({"name":"extract_link_targets","arguments":)"
                         R"({"data":")" + sheetData + R"(","filename":"convert-to.xlsx"}}})");
            LOK_ASSERT(contains(targets->getBody(), R"("isError":false)"));
            LOK_ASSERT(contains(textResultFrom(targets->getBody()), "Targets"));

            // Extracting from a spreadsheet with no filter answers with a JSON-RPC response whose
            // result is the structure of the spreadsheet as a JSON object.
            auto extract = mcpCall(
                session, R"({"jsonrpc":"2.0","id":14,"method":"tools/call","params":)"
                         R"({"name":"extract_document_structure","arguments":)"
                         R"({"data":")" + sheetData + R"(","filename":"convert-to.xlsx"}}})");
            LOK_ASSERT(extract->statusCode() == http::StatusCode::OK);
            LOK_ASSERT(extract->get("Content-Type") == "application/json");
            LOK_ASSERT(contains(extract->getBody(), R"("id":14)"));
            LOK_ASSERT(contains(extract->getBody(), R"("isError":false)"));
            Poco::JSON::Parser parser;
            LOK_ASSERT(parser.parse(textResultFrom(extract->getBody()))
                           .type() == typeid(Poco::JSON::Object::Ptr));
        }

        // A filter that is not one of the views, and a range that is not a cell range, are readable
        // failures that name the argument.
        {
            auto response = mcpCall(
                session, R"({"jsonrpc":"2.0","id":20,"method":"tools/call","params":)"
                         R"({"name":"extract_document_structure","arguments":)"
                         R"({"data":")" + helloData + R"(","filename":"hello.odt",)"
                         R"("filter":"text&x=1"}}})");
            LOK_ASSERT(contains(response->getBody(), R"("isError":true)"));
            LOK_ASSERT(contains(textResultFrom(response->getBody()), "filter argument"));

            response = mcpCall(
                session, R"({"jsonrpc":"2.0","id":21,"method":"tools/call","params":)"
                         R"({"name":"extract_document_structure","arguments":)"
                         R"({"data":")" + helloData + R"(","filename":"hello.odt",)"
                         R"("range":"A1:B5 "}}})");
            LOK_ASSERT(contains(response->getBody(), R"("isError":true)"));
            LOK_ASSERT(contains(textResultFrom(response->getBody()), "range argument"));
        }

        const std::string initialize = R"({"jsonrpc":"2.0","id":30,"method":"initialize"})";

        // A request with the key from a web page, which carries an Origin header, is forbidden.
        {
            http::Request request("/cool/mcp", http::Request::VERB_POST);
            request.set("Authorization", "Bearer " + ApiKey);
            request.set("Origin", "https://example.com");
            request.setBody(initialize, "application/json");
            auto response = session->syncRequest(request);
            LOK_ASSERT_EQUAL(http::StatusCode::Forbidden, response->statusCode());
        }

        // A message without the key is refused and told to send a bearer token: with no
        // Authorization header, with the scheme and no key, with the key one character short or one
        // character long, and with the key under another scheme.
        const std::string shortKey = ApiKey.substr(0, ApiKey.size() - 1);
        for (const std::string& authorization :
             { std::string(), std::string("Bearer"), "Bearer " + shortKey, "Bearer " + ApiKey + 'x',
               "Basic " + ApiKey })
        {
            TST_LOG("Sending a message with authorization [" << authorization << ']');
            auto response = mcpCall(session, initialize, authorization);
            LOK_ASSERT_EQUAL(http::StatusCode::Unauthorized, response->statusCode());
            LOK_ASSERT_EQUAL_STR("Bearer", response->get("WWW-Authenticate"));
        }

        // The address written in hex is the same endpoint: it is refused without the key and
        // answered with it.
        {
            auto refused = mcpCall(session, initialize, std::string(), "/cool/0x6d6370");
            LOK_ASSERT_EQUAL(http::StatusCode::Unauthorized, refused->statusCode());
            LOK_ASSERT_EQUAL_STR("Bearer", refused->get("WWW-Authenticate"));

            auto answered = mcpCall(session, initialize, "Bearer " + ApiKey, "/cool/0x6d6370");
            LOK_ASSERT_EQUAL(http::StatusCode::OK, answered->statusCode());
            LOK_ASSERT(contains(answered->getBody(), R"("serverInfo")"));
        }

        // A POST must state the length of its body. One sent in chunks, or with no Content-Length,
        // is refused.
        {
            http::Request chunked("/cool/mcp", http::Request::VERB_POST);
            chunked.set("Authorization", "Bearer " + ApiKey);
            chunked.set("Transfer-Encoding", "chunked");
            auto response = session->syncRequest(chunked);
            LOK_ASSERT_EQUAL(http::StatusCode::LengthRequired, response->statusCode());

            http::Request unstated("/cool/mcp", http::Request::VERB_POST);
            unstated.set("Authorization", "Bearer " + ApiKey);
            response = session->syncRequest(unstated);
            LOK_ASSERT_EQUAL(http::StatusCode::LengthRequired, response->statusCode());
        }

        // A message over the most a message may hold is refused on its header alone. No byte of the
        // body is sent, so the answer shows the server decided from the header.
        {
            auto response = session->syncRequest(headerOnlyPost(
                "/cool/mcp", static_cast<std::int64_t>(McpTools::MaxMessageBytes) + 1));
            LOK_ASSERT_EQUAL(http::StatusCode::PayloadTooLarge, response->statusCode());
            LOK_ASSERT(contains(response->getBody(), "Upload a large document"));
        }

        // The message address takes only POST.
        {
            auto response = mcpGet(session, "/cool/mcp");
            LOK_ASSERT_EQUAL(http::StatusCode::MethodNotAllowed, response->statusCode());
            LOK_ASSERT_EQUAL_STR("POST", response->get("Allow"));
        }
    }

    void invokeWSDTest() override
    {
        if (_workerStarted)
            return;
        _workerStarted = true;

        _worker = std::thread(
            [this]
            {
                try
                {
                    auto session = http::Session::create(helpers::getTestServerURI());
                    session->setTimeout(std::chrono::seconds(30));
                    runTest(session);
                    exitTest(TestResult::Ok);
                }
                catch (const std::exception& exc)
                {
                    LOG_TST("UnitMcp failed: " << exc.what());
                    exitTest(TestResult::Failed);
                }
            });
    }
};

/// Inside the forkit and kit processes.
class UnitKitMcp : public UnitKit
{
public:
    UnitKitMcp()
        : UnitKit("UnitKitMcp")
    {
        setTimeout(1h);
    }
};

UnitBase* unit_create_wsd(void) { return new UnitMcp(); }

UnitBase* unit_create_kit(void) { return new UnitKitMcp(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
