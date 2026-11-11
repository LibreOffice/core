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

#include <memory>
#include <string>

#include <Poco/JSON/Object.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/URI.h>
#include <Poco/Util/LayeredConfiguration.h>

#include <WopiTestServer.hpp>
#include <common/Uri.hpp>
#include <helpers.hpp>
#include <lokassert.hpp>
#include <net/HttpRequest.hpp>

// A /wasm/ client gets the document only when its host allows a clean export, and uploads one only
// when its host lets it write:
class UnitWasmProxy: public WopiTestServer {
public:
    UnitWasmProxy(): WopiTestServer("UnitWasmProxy") {}

    void configure(Poco::Util::LayeredConfiguration & config) override {
        WopiTestServer::configure(config);
        config.setBool("wasm.enable", true);
    }

    void configCheckFileInfo(
        Poco::Net::HTTPRequest const &, Poco::JSON::Object::Ptr & fileInfo) override
    {
        fileInfo->set("UserCanWrite", userCanWrite_);
        fileInfo->set("DisableExport", disableExport_);
    }

    void invokeWSDTest() override {
        if (done_) {
            return;
        }
        done_ = true;
        Poco::URI const wopiURL(
            getWopiHostURI() + "/wopi/files/0?access_token=anything&testname=UnitWasmProxy");
        std::string const path = "/wasm/" + Uri::encode(wopiURL.toString());

        // A user whose host allows a clean export gets the document:
        {
            http::Request request(path);
            std::shared_ptr<http::Response const> const response
                = http::Session::create(helpers::getTestServerURI())->syncRequest(request);
            LOK_ASSERT_EQUAL(http::StatusCode::OK, response->statusLine().statusCode());
            LOK_ASSERT_EQUAL(std::string("Hello, world"), response->getBody());
        }

        // A user whose host disables export does not:
        disableExport_ = true;
        {
            http::Request request(path);
            std::shared_ptr<http::Response const> const response
                = http::Session::create(helpers::getTestServerURI())->syncRequest(request);
            LOK_ASSERT_EQUAL(http::StatusCode::Forbidden, response->statusLine().statusCode());
            LOK_ASSERT_MESSAGE(
                "Expected no document in the answer",
                response->getBody().find("Hello, world") == std::string::npos);
        }
        disableExport_ = false;

        // A user who cannot write uploads nothing:
        userCanWrite_ = false;
        {
            http::Request request(path, http::Request::VERB_POST);
            request.setBody("Uploaded by a reader", "application/octet-stream");
            std::shared_ptr<http::Response const> const response
                = http::Session::create(helpers::getTestServerURI())->syncRequest(request);
            LOK_ASSERT_EQUAL(http::StatusCode::Forbidden, response->statusLine().statusCode());
            LOK_ASSERT_EQUAL(std::string("Hello, world"), getFileContent());
        }

        // A user who can write uploads:
        userCanWrite_ = true;
        {
            http::Request request(path, http::Request::VERB_POST);
            request.setBody("Uploaded by a writer", "application/octet-stream");
            std::shared_ptr<http::Response const> const response
                = http::Session::create(helpers::getTestServerURI())->syncRequest(request);
            LOK_ASSERT_EQUAL(http::StatusCode::OK, response->statusLine().statusCode());
            LOK_ASSERT_EQUAL(std::string("Uploaded by a writer"), getFileContent());
        }

        passTest("/wasm/ followed the host's export and write settings");
    }

private:
    bool done_ = false;
    bool userCanWrite_ = true;
    bool disableExport_ = false;
};

UnitBase* unit_create_wsd(void) { return new UnitWasmProxy(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
