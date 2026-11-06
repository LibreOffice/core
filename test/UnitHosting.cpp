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
 * Unit test for hosting-related functionality.
 */

#include <config.h>

#include <common/Png.hpp>
#include <common/Unit.hpp>
#include <net/HttpRequest.hpp>
#include <test/helpers.hpp>
#include <test/lokassert.hpp>

#include <Poco/DOM/AutoPtr.h>
#include <Poco/DOM/DOMParser.h>
#include <Poco/DOM/Document.h>
#include <Poco/DOM/NodeList.h>

#include <memory>
#include <string>

/// Test suite for /hosting, etc.
class UnitHosting : public UnitWSD
{
    TestResult testDiscovery();
    TestResult testCapabilities();
    TestResult testActionUrlsAreAppendReady();

public:
    UnitHosting()
        : UnitWSD("UnitHosting")
    {
    }

    void invokeWSDTest() override;
};

UnitBase::TestResult UnitHosting::testDiscovery()
{
    TST_LOG("Getting /hosting/discovery the first time.");
    const std::shared_ptr<const http::Response> httpResponse
        = http::get(helpers::getTestServerURI(), "/hosting/discovery");

    LOK_ASSERT(httpResponse->done());
    LOK_ASSERT_EQUAL(http::Response::State::Complete, httpResponse->state());

    LOK_ASSERT(!httpResponse->statusLine().httpVersion().empty());
    LOK_ASSERT(!httpResponse->statusLine().reasonPhrase().empty());
    LOK_ASSERT_EQUAL(http::StatusCode::OK, httpResponse->statusLine().statusCode());
    LOK_ASSERT(httpResponse->statusLine().statusCategory()
               == http::StatusLine::StatusCodeClass::Successful);
    LOK_ASSERT_EQUAL_STR("HTTP/1.1", httpResponse->statusLine().httpVersion());
    LOK_ASSERT_EQUAL_STR("OK", httpResponse->statusLine().reasonPhrase());
    LOK_ASSERT_EQUAL_STR("text/xml", httpResponse->header().getContentType());

    // Repeat, with a trailing slash in the URL.
    TST_LOG("Getting /hosting/discovery the second time.");
    const std::shared_ptr<const http::Response> httpResponse2
        = http::get(helpers::getTestServerURI(), "/hosting/discovery/");

    LOK_ASSERT(httpResponse2->done());
    LOK_ASSERT_EQUAL(http::Response::State::Complete, httpResponse2->state());

    LOK_ASSERT(!httpResponse2->statusLine().httpVersion().empty());
    LOK_ASSERT(!httpResponse2->statusLine().reasonPhrase().empty());
    LOK_ASSERT_EQUAL(http::StatusCode::OK, httpResponse2->statusLine().statusCode());
    LOK_ASSERT(httpResponse2->statusLine().statusCategory()
               == http::StatusLine::StatusCodeClass::Successful);
    LOK_ASSERT_EQUAL_STR("HTTP/1.1", httpResponse2->statusLine().httpVersion());
    LOK_ASSERT_EQUAL_STR("OK", httpResponse2->statusLine().reasonPhrase());
    LOK_ASSERT_EQUAL_STR("text/xml", httpResponse2->header().getContentType());

    TST_LOG("Comparing /hosting/discovery from both requests.");
    LOK_ASSERT_EQUAL(httpResponse2->getBody(), httpResponse->getBody());

    return TestResult::Ok;
}

UnitBase::TestResult UnitHosting::testCapabilities()
{
    auto httpSession = http::Session::create(helpers::getTestServerURI());
    std::shared_ptr<const http::Response> httpResponse
        = httpSession->syncRequest(http::Request("/hosting/discovery"));

    LOK_ASSERT(httpResponse->done());
    LOK_ASSERT_EQUAL(http::Response::State::Complete, httpResponse->state());

    // Get discovery first and extract the urlsrc of the capabilities end point
    std::string capabilitiesURI;
    {
        LOK_ASSERT_EQUAL(http::StatusCode::OK, httpResponse->statusLine().statusCode());
        LOK_ASSERT_EQUAL_STR("text/xml", httpResponse->header().getContentType());

        const std::string& discoveryXML = httpResponse->getBody();

        Poco::XML::DOMParser parser;
        Poco::XML::AutoPtr<Poco::XML::Document> docXML = parser.parseString(discoveryXML);
        Poco::XML::AutoPtr<Poco::XML::NodeList> listNodes = docXML->getElementsByTagName("action");
        bool foundCapabilities = false;
        for (unsigned long index = 0; index < listNodes->length(); ++index)
        {
            Poco::XML::Element* elem = static_cast<Poco::XML::Element*>(listNodes->item(index));
            Poco::XML::Element* parent = elem->parentNode()
                                             ? static_cast<Poco::XML::Element*>(elem->parentNode())
                                             : nullptr;
            if (parent && parent->getAttribute("name") == "Capabilities")
            {
                foundCapabilities = true;
                capabilitiesURI = elem->getAttribute("urlsrc");
                break;
            }
        }

        LOK_ASSERT(foundCapabilities);
        LOK_ASSERT_EQUAL(helpers::getTestServerURI() + CAPABILITIES_END_POINT, capabilitiesURI);
    }

    // Then get the capabilities json
    {
        httpResponse = httpSession->syncRequest(http::Request(CAPABILITIES_END_POINT));

        LOK_ASSERT(httpResponse->done());
        LOK_ASSERT_EQUAL(http::Response::State::Complete, httpResponse->state());

        LOK_ASSERT_EQUAL(http::StatusCode::OK, httpResponse->statusLine().statusCode());
        LOK_ASSERT_EQUAL_STR("application/json", httpResponse->header().getContentType());

        const std::string& responseString = httpResponse->getBody();

        Poco::JSON::Parser parser;
        Poco::Dynamic::Var jsonFile = parser.parse(responseString);
        const Poco::JSON::Object::Ptr& features = jsonFile.extract<Poco::JSON::Object::Ptr>();
        LOK_ASSERT(features);
        LOK_ASSERT(features->has("convert-to"));

        Poco::JSON::Object::Ptr convert_to
            = features->get("convert-to").extract<Poco::JSON::Object::Ptr>();
        LOK_ASSERT(convert_to->has("available"));
        LOK_ASSERT(convert_to->get("available").convert<bool>());
    }
    return TestResult::Ok;
}

// WOPI hosts append "WOPISrc=..." to the urlsrc of the action they pick, so every urlsrc must end
// with a query separator. Naive hosts do not insert one, and end up requesting a URL whose path is
// e.g. "cool-preview.htmlWOPISrc=...", which the static file server cannot find (404).
UnitBase::TestResult UnitHosting::testActionUrlsAreAppendReady()
{
    TST_LOG("Checking that every action's urlsrc is append-ready.");

    auto httpSession = http::Session::create(helpers::getTestServerURI());
    std::shared_ptr<const http::Response> httpResponse
        = httpSession->syncRequest(http::Request("/hosting/discovery"));

    LOK_ASSERT(httpResponse->done());
    LOK_ASSERT_EQUAL(http::Response::State::Complete, httpResponse->state());
    LOK_ASSERT_EQUAL(http::StatusCode::OK, httpResponse->statusLine().statusCode());

    Poco::XML::DOMParser parser;
    Poco::XML::AutoPtr<Poco::XML::Document> docXML = parser.parseString(httpResponse->getBody());
    Poco::XML::AutoPtr<Poco::XML::NodeList> listNodes = docXML->getElementsByTagName("action");

    unsigned long checkedActions = 0;
    for (unsigned long index = 0; index < listNodes->length(); ++index)
    {
        Poco::XML::Element* elem = static_cast<Poco::XML::Element*>(listNodes->item(index));
        Poco::XML::Element* parent = elem->parentNode()
                                         ? static_cast<Poco::XML::Element*>(elem->parentNode())
                                         : nullptr;

        // The capabilities and the settings iframe are not document actions; they are fetched
        // as-is, without appending any query string.
        if (parent && (parent->getAttribute("name") == "Capabilities" ||
                       parent->getAttribute("name") == "Settings"))
            continue;

        const std::string name = elem->getAttribute("name");
        const std::string urlsrc = elem->getAttribute("urlsrc");
        LOK_ASSERT_MESSAGE("action '" << name << "' urlsrc is not append-ready: " << urlsrc,
                           !urlsrc.empty() && (urlsrc.back() == '?' || urlsrc.back() == '&'));
        ++checkedActions;
    }
    LOK_ASSERT(checkedActions > 0);

    return TestResult::Ok;
}

void UnitHosting::invokeWSDTest()
{
    UnitBase::TestResult result = testDiscovery();
    if (result != TestResult::Ok)
        exitTest(result);

    result = testCapabilities();
    if (result != TestResult::Ok)
        exitTest(result);

    result = testActionUrlsAreAppendReady();
    if (result != TestResult::Ok)
        exitTest(result);

    exitTest(TestResult::Ok);
}

UnitBase* unit_create_wsd(void) { return new UnitHosting(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
