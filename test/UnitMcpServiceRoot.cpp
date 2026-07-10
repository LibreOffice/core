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
 * The Model Context Protocol endpoint served under a service root: the endpoint answers at the
 * address with the root in front, a download link it gives out carries the root and serves the
 * document, and other requests whose address mentions it are served as before.
 */

#include <config.h>

#include <common/Unit.hpp>
#include <common/Util.hpp>
#include <net/HttpRequest.hpp>
#include <test/helpers.hpp>
#include <test/lokassert.hpp>

#include <Poco/JSON/Array.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>
#include <Poco/URI.h>
#include <Poco/Util/LayeredConfiguration.h>

#include <chrono>
#include <exception>
#include <memory>
#include <string>
#include <thread>

using namespace std::literals;

namespace
{
const std::string ApiKey = "secret123";
const std::string ServiceRoot = "/collabora";

/// The text of the first content block of a tool result, or an empty string when there is none.
std::string textResultFrom(const std::string& body)
{
    try
    {
        Poco::JSON::Parser parser;
        auto root = parser.parse(body).extract<Poco::JSON::Object::Ptr>();
        auto content = root->getObject("result")->getArray("content");
        return content->getObject(0)->optValue<std::string>("text", std::string());
    }
    catch (const std::exception&)
    {
        return std::string();
    }
}

/// The value of the string member key of the JSON object in json, or an empty string.
std::string stringValueFrom(const std::string& json, const std::string& key)
{
    try
    {
        Poco::JSON::Parser parser;
        auto object = parser.parse(json).extract<Poco::JSON::Object::Ptr>();
        return object->optValue<std::string>(key, std::string());
    }
    catch (const std::exception&)
    {
        return std::string();
    }
}
}

class UnitMcpServiceRoot : public UnitWSD
{
    bool _workerStarted;
    std::thread _worker;

public:
    UnitMcpServiceRoot()
        : UnitWSD("UnitMcpServiceRoot")
        , _workerStarted(false)
    {
        setTimeout(5min);
    }

    ~UnitMcpServiceRoot()
    {
        if (_worker.joinable())
            _worker.join();
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        UnitWSD::configure(config);

        config.setString("net.service_root", ServiceRoot);
        config.setBool("ai.mcp.enable", true);
        config.setString("ai.mcp.api_key", ApiKey);
    }

    void runTest(const std::shared_ptr<http::Session>& session)
    {
        // The handshake at the endpoint under the service root is answered.
        {
            http::Request request(ServiceRoot + "/cool/mcp", http::Request::VERB_POST);
            request.set("Authorization", "Bearer " + ApiKey);
            request.setBody(R"({"jsonrpc":"2.0","id":1,"method":"initialize",)"
                            R"("params":{"protocolVersion":"2025-06-18"}})",
                            "application/json");
            auto response = session->syncRequest(request);
            LOK_ASSERT_EQUAL(http::StatusCode::OK, response->statusCode());
            LOK_ASSERT(response->getBody().find(R"("protocolVersion":"2025-06-18")") !=
                       std::string::npos);
        }

        // A document asked for as a link comes as an address under the service root, and that
        // address serves the document.
        {
            const std::string textData = Util::base64Encode(std::string("hello world"));
            http::Request request(ServiceRoot + "/cool/mcp", http::Request::VERB_POST);
            request.set("Authorization", "Bearer " + ApiKey);
            request.setBody(R"({"jsonrpc":"2.0","id":2,"method":"tools/call","params":)"
                            R"({"name":"convert_document","arguments":{"data":")" +
                                textData +
                                R"(","filename":"hello.txt","format":"pdf","output":"link"}}})",
                            "application/json");
            auto response = session->syncRequest(request);
            LOK_ASSERT_EQUAL(http::StatusCode::OK, response->statusCode());
            LOK_ASSERT(response->getBody().find(R"("isError":false)") != std::string::npos);

            const std::string downloadUrl =
                stringValueFrom(textResultFrom(response->getBody()), "download_url");
            TST_LOG("Download link [" << downloadUrl << ']');
            const std::string downloadPath = Poco::URI(downloadUrl).getPath();
            LOK_ASSERT(downloadPath.starts_with(ServiceRoot + "/cool/mcp/download/"));

            http::Request download(downloadPath, http::Request::VERB_GET);
            download.set("Authorization", "Bearer " + ApiKey);
            auto document = session->syncRequest(download);
            LOK_ASSERT_EQUAL(http::StatusCode::OK, document->statusCode());
            LOK_ASSERT(document->getBody().starts_with("%PDF-"));
        }

        // A request for another service whose query mentions the endpoint is served as usual.
        {
            http::Request request(ServiceRoot + "/hosting/discovery?from=/mcp",
                                  http::Request::VERB_GET);
            auto response = session->syncRequest(request);
            LOK_ASSERT_EQUAL(http::StatusCode::OK, response->statusCode());
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
                    LOG_TST("UnitMcpServiceRoot failed: " << exc.what());
                    exitTest(TestResult::Failed);
                }
            });
    }
};

UnitBase* unit_create_wsd(void) { return new UnitMcpServiceRoot(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
