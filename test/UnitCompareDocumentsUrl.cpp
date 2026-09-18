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
 * Integration test for comparing the open document against another one that the integrator names
 * by URL. The URL is the only thing the engine is given, so every query parameter the integrator
 * put in it, the access token above all, has to come back out of the kit unchanged. The test
 * drives the wire message that the browser sends and reads the command in the kit, where the URL
 * is handed on to the engine.
 */

#include <config.h>

#include <Unit.hpp>
#include <common/JsonUtil.hpp>
#include <helpers.hpp>
#include <net/Socket.hpp>
#include <test/testlog.hpp>

#include <Poco/JSON/Object.h>
#include <Poco/URI.h>

#include <cstddef>
#include <iterator>
#include <memory>
#include <string>
#include <string_view>

namespace
{
/// One comparison that the browser asks for, and the URL it names, written the way an integrator
/// writes it.
struct CompareCase
{
    const char* what;
    const char* url;
};

/// Nothing ever connects to the host in these URLs. The URL is read in the kit, at the last point
/// on the way to the engine that online owns.
constexpr CompareCase Cases[] = {
    { "an access token beside percent-escaped parameters",
      "http://127.0.0.1:9980/wopi/files/7/contents"
      "?access_token=eyJhbGciOiJIUzI1NiJ9.eyJzdWIiOiI3In0.aQ-_bZ0"
      "&access_token_ttl=0"
      "&filename=Q3%20report%20%26%20notes.odt" },
    { "a token that holds a double quote",
      "http://127.0.0.1:9980/wopi/files/7/contents?access_token=say\"hello\"&permission=edit" },
};

/// Percent-encodes the way the encodeURIComponent function of the browser does, which is what
/// turns the whole URL into a single token with no spaces for the wire message.
std::string encodeUriComponent(const std::string& text)
{
    static constexpr std::string_view Unreserved = "-_.!~*'()";
    static constexpr char HexDigits[] = "0123456789ABCDEF";

    std::string encoded;
    encoded.reserve(text.size() * 3);
    for (const char c : text)
    {
        const unsigned char byte = static_cast<unsigned char>(c);
        if ((byte >= '0' && byte <= '9') || (byte >= 'A' && byte <= 'Z') ||
            (byte >= 'a' && byte <= 'z') || Unreserved.find(c) != std::string_view::npos)
        {
            encoded += c;
        }
        else
        {
            encoded += '%';
            encoded += HexDigits[(byte >> 4) & 0x0F];
            encoded += HexDigits[byte & 0x0F];
        }
    }

    return encoded;
}
}

class UnitCompareDocumentsUrl : public UnitWSD
{
    bool _asked = false;

    /// Held for as long as the test runs, so the requests stay on their way while the kit reads
    /// them.
    std::shared_ptr<SocketPoll> _socketPoll;
    std::shared_ptr<http::WebSocketSession> _socket;

public:
    UnitCompareDocumentsUrl()
        : UnitWSD("UnitCompareDocumentsUrl")
    {
        setHasKitHooks();
    }

    void invokeWSDTest() override;
};

void UnitCompareDocumentsUrl::invokeWSDTest()
{
    // The kit sends the verdict once it has seen every request, so there is nothing more to drive.
    if (_asked)
        return;

    _asked = true;

    std::string documentPath, documentURL;
    helpers::getDocumentPathAndURL("hello.odt", documentPath, documentURL, testname);

    _socketPoll = std::make_shared<SocketPoll>(testname);
    _socketPoll->startThread();
    _socket = helpers::loadDocAndGetSession(_socketPoll, Poco::URI(helpers::getTestServerURI()),
                                            documentURL, testname);

    for (const CompareCase& testCase : Cases)
    {
        TST_LOG("Asking to compare against a URL with " << testCase.what);
        helpers::sendTextFrame(_socket,
                               "insertfile name=" + encodeUriComponent(testCase.url) +
                                   " type=comparedocumentsurl",
                               testname);
    }
}

/// Inside the kit process, where the URL reaches the engine.
class UnitKitCompareDocumentsUrl : public UnitKit
{
    /// How many of the cases have been read, which is also the index of the next one.
    std::size_t _read = 0;

public:
    UnitKitCompareDocumentsUrl()
        : UnitKit("UnitKitCompareDocumentsUrl")
    {
    }

    void initialize() override
    {
        // Empty, so the socket poll thread is not started, which is unused by this test.
    }

    bool filterInsertCommand(const std::string& command, const std::string& arguments) override
    {
        if (isFinished())
            return true;

        const CompareCase& testCase = Cases[_read];
        TST_LOG("Read the command for " << testCase.what << ": " << command << ' ' << arguments);

        if (command != ".uno:CompareDocuments")
        {
            TST_LOG("Comparing against a URL became [" << command << ']');
            failTest("wrong-command");
            return true;
        }

        // A character that the escaping leaves raw, a double quote above all, makes the arguments
        // unreadable rather than changing the URL, so the parse stands for its own check.
        Poco::JSON::Object::Ptr object;
        if (!JsonUtil::parseJSON(arguments, object))
        {
            TST_LOG("The arguments given for " << testCase.what << " are not a JSON object");
            failTest("unreadable-arguments");
            return true;
        }

        std::string url;
        const Poco::JSON::Object::Ptr value = object->getObject("URL");
        if (value)
            url = value->optValue<std::string>("value", std::string());

        if (url != testCase.url)
        {
            TST_LOG("The engine was given [" << url << "] for " << testCase.what << ", not ["
                                             << testCase.url << ']');
            failTest("url-changed");
            return true;
        }

        ++_read;
        if (_read == std::size(Cases))
            passTest("every-url-unchanged");

        return true;
    }
};

UnitBase* unit_create_wsd(void) { return new UnitCompareDocumentsUrl(); }

UnitBase* unit_create_kit(void) { return new UnitKitCompareDocumentsUrl(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
