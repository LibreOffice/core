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
 * Unit test for HTTP request details and authorization parsing.
 */

#include <config.h>

#include <test/lokassert.hpp>

#include <Common.hpp>
#include <common/Authorization.hpp>
#include <HttpRequest.hpp>
#include <RequestDetails.hpp>

#include <Poco/Net/HTTPRequest.h>

#include <cppunit/extensions/HelperMacros.h>

/// RequestDetails unit-tests.
class RequestDetailsTests : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(RequestDetailsTests);

    CPPUNIT_TEST(testDownloadURI);
    CPPUNIT_TEST(testCoolURI);
    CPPUNIT_TEST(testLocal);
    CPPUNIT_TEST(testLocalHexified);
    CPPUNIT_TEST(testRequestDetails);
    CPPUNIT_TEST(testCoolWs);
    CPPUNIT_TEST(testAuthorization);
    CPPUNIT_TEST(testAuthorizationExpiry);
    CPPUNIT_TEST(testSanitizePercent);

    CPPUNIT_TEST_SUITE_END();

    void testDownloadURI();
    void testCoolURI();
    void testLocal();
    void testLocalHexified();
    void testRequestDetails();
    void testCoolWs();
    void testAuthorization();
    void testAuthorizationExpiry();
    void testSanitizePercent();
};

void RequestDetailsTests::testDownloadURI()
{
    constexpr std::string_view testname = __func__;

    static const std::string Root = "localhost:9980";

    {
        static const std::string URI = "/browser/49c225146/src/map/Clipboard.js";

        Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, URI,
                                       Poco::Net::HTTPMessage::HTTP_1_1);
        request.setHost(Root);

        RequestDetails details(request, "");

        // LOK_ASSERT_EQUAL(URI, details.getDocumentURI());

        LOK_ASSERT_EQUAL(static_cast<std::size_t>(5), details.size());
        LOK_ASSERT_EQUAL_STR("browser", details[0]);
        LOK_ASSERT_EQUAL_STR("browser", details.getField(RequestDetails::Field::Type));
        LOK_ASSERT(details.equals(RequestDetails::Field::Type, "browser"));
        LOK_ASSERT(details.equals(0, "browser"));
        LOK_ASSERT_EQUAL_STR("49c225146", details[1]);
        LOK_ASSERT_EQUAL_STR("src", details[2]);
        LOK_ASSERT_EQUAL_STR("map", details[3]);
        LOK_ASSERT_EQUAL_STR("Clipboard.js", details[4]);

        http::Request request2(URI);
        request2.set("Host", Root);
        http::RequestParser reqParser(request2);
        LOK_ASSERT_EQUAL(details, RequestDetails(reqParser, ""));
    }

    {
        static const std::string URI = "/browser/49c225146/cool.html";

        Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, URI,
                                       Poco::Net::HTTPMessage::HTTP_1_1);
        request.setHost(Root);

        RequestDetails details(request, "");

        // LOK_ASSERT_EQUAL(URI, details.getDocumentURI());

        LOK_ASSERT_EQUAL(static_cast<std::size_t>(3), details.size());
        LOK_ASSERT_EQUAL_STR("browser", details[0]);
        LOK_ASSERT_EQUAL_STR("browser", details.getField(RequestDetails::Field::Type));
        LOK_ASSERT(details.equals(RequestDetails::Field::Type, "browser"));
        LOK_ASSERT(details.equals(0, "browser"));
        LOK_ASSERT_EQUAL_STR("49c225146", details[1]);
        LOK_ASSERT_EQUAL_STR("cool.html", details[2]);

        http::Request request2(URI);
        request2.set("Host", Root);
        http::RequestParser reqParser(request2);
        LOK_ASSERT_EQUAL(details, RequestDetails(reqParser, ""));
    }
}

void RequestDetailsTests::testCoolURI()
{
    constexpr std::string_view testname = __func__;

    static const std::string Root = "localhost:9980";

    static const std::string URI
        = "/browser/49c225146/"
          "cool.html?WOPISrc=http%3A%2F%2Flocalhost%2Fnextcloud%2Findex.php%2Fapps%"
          "2Frichdocuments%2Fwopi%2Ffiles%2F593_ocqiesh0cngs&title=empty.odt&lang=en-us&"
          "closebutton=1&revisionhistory=1";

    Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, URI,
                                   Poco::Net::HTTPMessage::HTTP_1_1);
    request.setHost(Root);

    RequestDetails details(request, "");

    const std::string wopiSrc
        = "http://localhost/nextcloud/index.php/apps/richdocuments/wopi/files/593_ocqiesh0cngs";

    LOK_ASSERT_EQUAL(wopiSrc, details.getField(RequestDetails::Field::WOPISrc));

    LOK_ASSERT_EQUAL(static_cast<std::size_t>(4), details.size());
    LOK_ASSERT_EQUAL_STR("browser", details[0]);
    LOK_ASSERT_EQUAL_STR("browser", details.getField(RequestDetails::Field::Type));
    LOK_ASSERT(details.equals(RequestDetails::Field::Type, "browser"));
    LOK_ASSERT(details.equals(0, "browser"));
    LOK_ASSERT_EQUAL_STR("49c225146", details[1]);
    LOK_ASSERT_EQUAL_STR("cool.html", details[2]);
    LOK_ASSERT_EQUAL(std::string("WOPISrc=http%3A%2F%2Flocalhost%2Fnextcloud%2Findex.php%"
                                 "2Fapps%2Frichdocuments%2Fwopi%2Ffiles%2F593_ocqiesh0cngs&"
                                 "title=empty.odt&lang=en-us&closebutton=1&revisionhistory=1"),
                     details[3]);

    http::Request request2(URI);
    request2.set("Host", Root);
    http::RequestParser reqParser(request2);
    LOK_ASSERT_EQUAL(details, RequestDetails(reqParser, ""));
}

void RequestDetailsTests::testLocal()
{
    constexpr std::string_view testname = __func__;

    static const std::string Root = "localhost:9980";

    static const std::string ProxyPrefix
        = "http://localhost/nextcloud/apps/richdocuments/proxy.php?req=";

    {
        static const std::string URI = "/cool/"
                                       "file%3A%2F%2F%2Fhome%2Fash%2Fprj%2Flo%2Fonline%2Ftest%"
                                       "2Fdata%2Fhello-world.odt/ws/open/open/0";

        Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, URI,
                                       Poco::Net::HTTPMessage::HTTP_1_1);
        request.setHost(Root);
        request.set("User-Agent", http::getAgentString());
        request.set("ProxyPrefix", ProxyPrefix);

        RequestDetails details(request, "");
        LOK_ASSERT_EQUAL(true, details.isProxy());
        LOK_ASSERT_EQUAL(ProxyPrefix, details.getProxyPrefix());

        LOK_ASSERT_EQUAL(Root, details.getHostUntrusted());
        LOK_ASSERT_EQUAL(false, details.isWebSocket());
        LOK_ASSERT_EQUAL(true, details.isGet());

        const std::string docUri = "file:///home/ash/prj/lo/online/test/data/hello-world.odt";

        LOK_ASSERT_EQUAL(docUri, details.getDocumentURI());

        LOK_ASSERT_EQUAL(static_cast<std::size_t>(6), details.size());
        LOK_ASSERT_EQUAL_STR("cool", details[0]);
        LOK_ASSERT(details.equals(0, "cool"));
        LOK_ASSERT_EQUAL(
            std::string(
                "file%3A%2F%2F%2Fhome%2Fash%2Fprj%2Flo%2Fonline%2Ftest%2Fdata%2Fhello-world.odt"),
            details[1]);
        LOK_ASSERT_EQUAL_STR("ws", details[2]);
        LOK_ASSERT_EQUAL_STR("open", details[3]);
        LOK_ASSERT_EQUAL_STR("open", details[4]);
        LOK_ASSERT_EQUAL_STR("0", details[5]);

        LOK_ASSERT_EQUAL_STR("cool", details.getField(RequestDetails::Field::Type));
        LOK_ASSERT(details.equals(RequestDetails::Field::Type, "cool"));
        LOK_ASSERT_EQUAL_STR("open", details.getField(RequestDetails::Field::SessionId));
        LOK_ASSERT(details.equals(RequestDetails::Field::SessionId, "open"));
        LOK_ASSERT_EQUAL_STR("open", details.getField(RequestDetails::Field::Command));
        LOK_ASSERT(details.equals(RequestDetails::Field::Command, "open"));
        LOK_ASSERT_EQUAL_STR("0", details.getField(RequestDetails::Field::Serial));
        LOK_ASSERT(details.equals(RequestDetails::Field::Serial, "0"));

        http::Request request2(URI);
        request2.set("Host", Root);
        request2.set("User-Agent", http::getAgentString());
        request2.set("ProxyPrefix", ProxyPrefix);
        http::RequestParser reqParser(request2);
        LOK_ASSERT_EQUAL(details, RequestDetails(reqParser, ""));
    }

    {
        // Blank entries are skipped.
        static const std::string URI = "/cool/"
                                       "file%3A%2F%2F%2Fhome%2Fash%2Fprj%2Flo%2Fonline%2Ftest%"
                                       "2Fdata%2Fhello-world.odt/ws//write/2";

        Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, URI,
                                       Poco::Net::HTTPMessage::HTTP_1_1);
        request.setHost(Root);
        request.set("User-Agent", http::getAgentString());
        request.set("ProxyPrefix", ProxyPrefix);

        RequestDetails details(request, "");
        LOK_ASSERT_EQUAL(true, details.isProxy());
        LOK_ASSERT_EQUAL(ProxyPrefix, details.getProxyPrefix());

        LOK_ASSERT_EQUAL(Root, details.getHostUntrusted());
        LOK_ASSERT_EQUAL(false, details.isWebSocket());
        LOK_ASSERT_EQUAL(true, details.isGet());

        const std::string docUri = "file:///home/ash/prj/lo/online/test/data/hello-world.odt";

        LOK_ASSERT_EQUAL(docUri, details.getDocumentURI());

        LOK_ASSERT_EQUAL(static_cast<std::size_t>(5), details.size());
        LOK_ASSERT_EQUAL_STR("cool", details[0]);
        LOK_ASSERT(details.equals(0, "cool"));
        LOK_ASSERT_EQUAL(
            std::string(
                "file%3A%2F%2F%2Fhome%2Fash%2Fprj%2Flo%2Fonline%2Ftest%2Fdata%2Fhello-world.odt"),
            details[1]);
        LOK_ASSERT_EQUAL_STR("ws", details[2]);
        LOK_ASSERT_EQUAL_STR("write", details[3]); // SessionId, since the real SessionId is blank.
        LOK_ASSERT_EQUAL_STR("2", details[4]); // Command, since SessionId was blank.

        LOK_ASSERT_EQUAL_STR("cool", details.getField(RequestDetails::Field::Type));
        LOK_ASSERT(details.equals(RequestDetails::Field::Type, "cool"));
        LOK_ASSERT_EQUAL_STR("write", details.getField(RequestDetails::Field::SessionId));
        LOK_ASSERT(details.equals(RequestDetails::Field::SessionId, "write"));
        LOK_ASSERT_EQUAL_STR("2", details.getField(RequestDetails::Field::Command));
        LOK_ASSERT(details.equals(RequestDetails::Field::Command, "2"));
        LOK_ASSERT_EQUAL_STR("", details.getField(RequestDetails::Field::Serial));
        LOK_ASSERT(details.equals(RequestDetails::Field::Serial, ""));

        http::Request request2(URI);
        request2.set("Host", Root);
        request2.set("User-Agent", http::getAgentString());
        request2.set("ProxyPrefix", ProxyPrefix);
        http::RequestParser reqParser(request2);
        LOK_ASSERT_EQUAL(details, RequestDetails(reqParser, ""));
    }

    {
        // Apparently, the initial / can be missing -- all the tests do that.
        static const std::string URI = "cool/"
                                       "file%3A%2F%2F%2Fhome%2Fash%2Fprj%2Flo%2Fonline%2Ftest%"
                                       "2Fdata%2Fhello-world.odt/ws//write/2";

        Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, URI,
                                       Poco::Net::HTTPMessage::HTTP_1_1);
        request.setHost(Root);
        request.set("User-Agent", http::getAgentString());
        request.set("ProxyPrefix", ProxyPrefix);

        RequestDetails details(request, "");
        LOK_ASSERT_EQUAL(true, details.isProxy());
        LOK_ASSERT_EQUAL(ProxyPrefix, details.getProxyPrefix());

        LOK_ASSERT_EQUAL(Root, details.getHostUntrusted());
        LOK_ASSERT_EQUAL(false, details.isWebSocket());
        LOK_ASSERT_EQUAL(true, details.isGet());

        const std::string docUri = "file:///home/ash/prj/lo/online/test/data/hello-world.odt";

        LOK_ASSERT_EQUAL(docUri, details.getDocumentURI());

        LOK_ASSERT_EQUAL(static_cast<std::size_t>(5), details.size());
        LOK_ASSERT_EQUAL_STR("cool", details[0]);
        LOK_ASSERT(details.equals(0, "cool"));
        LOK_ASSERT_EQUAL(
            std::string(
                "file%3A%2F%2F%2Fhome%2Fash%2Fprj%2Flo%2Fonline%2Ftest%2Fdata%2Fhello-world.odt"),
            details[1]);
        LOK_ASSERT_EQUAL_STR("ws", details[2]);
        LOK_ASSERT_EQUAL_STR("write", details[3]); // SessionId, since the real SessionId is blank.
        LOK_ASSERT_EQUAL_STR("2", details[4]); // Command, since SessionId was blank.

        LOK_ASSERT_EQUAL_STR("cool", details.getField(RequestDetails::Field::Type));
        LOK_ASSERT(details.equals(RequestDetails::Field::Type, "cool"));
        LOK_ASSERT_EQUAL_STR("write", details.getField(RequestDetails::Field::SessionId));
        LOK_ASSERT(details.equals(RequestDetails::Field::SessionId, "write"));
        LOK_ASSERT_EQUAL_STR("2", details.getField(RequestDetails::Field::Command));
        LOK_ASSERT(details.equals(RequestDetails::Field::Command, "2"));
        LOK_ASSERT_EQUAL_STR("", details.getField(RequestDetails::Field::Serial));
        LOK_ASSERT(details.equals(RequestDetails::Field::Serial, ""));

        http::Request request2(URI);
        request2.set("Host", Root);
        request2.set("User-Agent", http::getAgentString());
        request2.set("ProxyPrefix", ProxyPrefix);
        http::RequestParser reqParser(request2);
        LOK_ASSERT_EQUAL(details, RequestDetails(reqParser, ""));
    }
}

void RequestDetailsTests::testLocalHexified()
{
    constexpr std::string_view testname = __func__;

    static const std::string Root = "localhost:9980";

    static const std::string ProxyPrefix
        = "http://localhost/nextcloud/apps/richdocuments/proxy.php?req=";

    static const std::string docUri = "file:///home/ash/prj/lo/online/test/data/hello-world.odt";
    static const std::string fileUrl =
        "file%3A%2F%2F%2Fhome%2Fash%2Fprj%2Flo%2Fonline%2Ftest%2Fdata%2Fhello-world.odt";

    const std::string fileUrlHex = "0x" + HexUtil::dataToHexString(fileUrl, 0, fileUrl.size());

    {
        static const std::string URI = "/cool/" + fileUrlHex + "/ws/open/open/0";

        Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, URI,
                                       Poco::Net::HTTPMessage::HTTP_1_1);
        request.setHost(Root);
        request.set("User-Agent", http::getAgentString());
        request.set("ProxyPrefix", ProxyPrefix);

        RequestDetails details(request, "");
        LOK_ASSERT_EQUAL(true, details.isProxy());
        LOK_ASSERT_EQUAL(ProxyPrefix, details.getProxyPrefix());

        LOK_ASSERT_EQUAL(Root, details.getHostUntrusted());
        LOK_ASSERT_EQUAL(false, details.isWebSocket());
        LOK_ASSERT_EQUAL(true, details.isGet());

        LOK_ASSERT_EQUAL(docUri, details.getDocumentURI());

        LOK_ASSERT_EQUAL(static_cast<std::size_t>(6), details.size());
        LOK_ASSERT_EQUAL_STR("cool", details[0]);
        LOK_ASSERT(details.equals(0, "cool"));
        LOK_ASSERT_EQUAL(fileUrl, details[1]);
        LOK_ASSERT_EQUAL_STR("ws", details[2]);
        LOK_ASSERT_EQUAL_STR("open", details[3]);
        LOK_ASSERT_EQUAL_STR("open", details[4]);
        LOK_ASSERT_EQUAL_STR("0", details[5]);

        LOK_ASSERT_EQUAL_STR("cool", details.getField(RequestDetails::Field::Type));
        LOK_ASSERT(details.equals(RequestDetails::Field::Type, "cool"));
        LOK_ASSERT_EQUAL_STR("open", details.getField(RequestDetails::Field::SessionId));
        LOK_ASSERT(details.equals(RequestDetails::Field::SessionId, "open"));
        LOK_ASSERT_EQUAL_STR("open", details.getField(RequestDetails::Field::Command));
        LOK_ASSERT(details.equals(RequestDetails::Field::Command, "open"));
        LOK_ASSERT_EQUAL_STR("0", details.getField(RequestDetails::Field::Serial));
        LOK_ASSERT(details.equals(RequestDetails::Field::Serial, "0"));

        http::Request request2(URI);
        request2.set("Host", Root);
        request2.set("User-Agent", http::getAgentString());
        request2.set("ProxyPrefix", ProxyPrefix);
        http::RequestParser reqParser(request2);
        LOK_ASSERT_EQUAL(details, RequestDetails(reqParser, ""));
    }

    {
        // Blank entries are skipped.
        static const std::string URI = "/cool/" + fileUrlHex + "/ws//write/2";

        Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, URI,
                                       Poco::Net::HTTPMessage::HTTP_1_1);
        request.setHost(Root);
        request.set("User-Agent", http::getAgentString());
        request.set("ProxyPrefix", ProxyPrefix);

        RequestDetails details(request, "");
        LOK_ASSERT_EQUAL(true, details.isProxy());
        LOK_ASSERT_EQUAL(ProxyPrefix, details.getProxyPrefix());

        LOK_ASSERT_EQUAL(Root, details.getHostUntrusted());
        LOK_ASSERT_EQUAL(false, details.isWebSocket());
        LOK_ASSERT_EQUAL(true, details.isGet());

        LOK_ASSERT_EQUAL(docUri, details.getDocumentURI());

        LOK_ASSERT_EQUAL(static_cast<std::size_t>(5), details.size());
        LOK_ASSERT_EQUAL_STR("cool", details[0]);
        LOK_ASSERT(details.equals(0, "cool"));
        LOK_ASSERT_EQUAL(fileUrl, details[1]);
        LOK_ASSERT_EQUAL_STR("ws", details[2]);
        LOK_ASSERT_EQUAL_STR("write", details[3]); // SessionId, since the real SessionId is blank.
        LOK_ASSERT_EQUAL_STR("2", details[4]); // Command, since SessionId was blank.

        LOK_ASSERT_EQUAL_STR("cool", details.getField(RequestDetails::Field::Type));
        LOK_ASSERT(details.equals(RequestDetails::Field::Type, "cool"));
        LOK_ASSERT_EQUAL_STR("write", details.getField(RequestDetails::Field::SessionId));
        LOK_ASSERT(details.equals(RequestDetails::Field::SessionId, "write"));
        LOK_ASSERT_EQUAL_STR("2", details.getField(RequestDetails::Field::Command));
        LOK_ASSERT(details.equals(RequestDetails::Field::Command, "2"));
        LOK_ASSERT_EQUAL_STR("", details.getField(RequestDetails::Field::Serial));
        LOK_ASSERT(details.equals(RequestDetails::Field::Serial, ""));

        http::Request request2(URI);
        request2.set("Host", Root);
        request2.set("User-Agent", http::getAgentString());
        request2.set("ProxyPrefix", ProxyPrefix);
        http::RequestParser reqParser(request2);
        LOK_ASSERT_EQUAL(details, RequestDetails(reqParser, ""));
    }

    {
        // Apparently, the initial / can be missing -- all the tests do that.
        static const std::string URI = "cool/" + fileUrlHex + "/ws//write/2";

        Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, URI,
                                       Poco::Net::HTTPMessage::HTTP_1_1);
        request.setHost(Root);
        request.set("User-Agent", http::getAgentString());
        request.set("ProxyPrefix", ProxyPrefix);

        RequestDetails details(request, "");
        LOK_ASSERT_EQUAL(true, details.isProxy());
        LOK_ASSERT_EQUAL(ProxyPrefix, details.getProxyPrefix());

        LOK_ASSERT_EQUAL(Root, details.getHostUntrusted());
        LOK_ASSERT_EQUAL(false, details.isWebSocket());
        LOK_ASSERT_EQUAL(true, details.isGet());

        LOK_ASSERT_EQUAL(docUri, details.getDocumentURI());

        LOK_ASSERT_EQUAL(static_cast<std::size_t>(5), details.size());
        LOK_ASSERT_EQUAL_STR("cool", details[0]);
        LOK_ASSERT(details.equals(0, "cool"));
        LOK_ASSERT_EQUAL(fileUrl, details[1]);
        LOK_ASSERT_EQUAL_STR("ws", details[2]);
        LOK_ASSERT_EQUAL_STR("write", details[3]); // SessionId, since the real SessionId is blank.
        LOK_ASSERT_EQUAL_STR("2", details[4]); // Command, since SessionId was blank.

        LOK_ASSERT_EQUAL_STR("cool", details.getField(RequestDetails::Field::Type));
        LOK_ASSERT(details.equals(RequestDetails::Field::Type, "cool"));
        LOK_ASSERT_EQUAL_STR("write", details.getField(RequestDetails::Field::SessionId));
        LOK_ASSERT(details.equals(RequestDetails::Field::SessionId, "write"));
        LOK_ASSERT_EQUAL_STR("2", details.getField(RequestDetails::Field::Command));
        LOK_ASSERT(details.equals(RequestDetails::Field::Command, "2"));
        LOK_ASSERT_EQUAL_STR("", details.getField(RequestDetails::Field::Serial));
        LOK_ASSERT(details.equals(RequestDetails::Field::Serial, ""));

        http::Request request2(URI);
        request2.set("Host", Root);
        request2.set("User-Agent", http::getAgentString());
        request2.set("ProxyPrefix", ProxyPrefix);
        http::RequestParser reqParser(request2);
        LOK_ASSERT_EQUAL(details, RequestDetails(reqParser, ""));
    }
}

void RequestDetailsTests::testRequestDetails()
{
    constexpr std::string_view testname = __func__;

    static const std::string Root = "localhost:9980";

    static const std::string ProxyPrefix
        = "http://localhost/nextcloud/apps/richdocuments/proxy.php?req=";

    {
        static const std::string URI
            = "/cool/"
              "http%3A%2F%2Flocalhost%2Fnextcloud%2Findex.php%2Fapps%2Frichdocuments%2Fwopi%"
              "2Ffiles%"
              "2F593_ocqiesh0cngs%3Faccess_token%3DMN0KXXDv9GJ1wCCLnQcjVQT2T7WrfYpA%26access_token_"
              "ttl%"
              "3D0%26reuse_cookies%3Doc_sessionPassphrase%"
              "253D8nFRqycbs7bP97yxCuJviBbVKdCXmuiXp6ZYH0DfUoy5UZDCTQgLwluvbgRbKrdKodJteG3uNE19KNUA"
              "oE5t"
              "ypf4oBGwJdFY%25252F5W9RNST8wEHWkUVIjZy7vmY0ZX38PlS%253Anc_sameSiteCookielax%"
              "253Dtrue%"
              "253Anc_sameSiteCookiestrict%253Dtrue%253Aocqiesh0cngs%"
              "253Dr5ujg4tpvgu9paaf5bguiokgjl%"
              "253AXCookieName%253DXCookieValue%253ASuperCookieName%253DBAZINGA/"
              "ws?WOPISrc=http%3A%2F%2Flocalhost%2Fnextcloud%2Findex.php%2Fapps%2Frichdocuments%"
              "2Fwopi%"
              "2Ffiles%2F593_ocqiesh0cngs&compat=/ws/b26112ab1b6f2ed98ce1329f0f344791/close/31";

        Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, URI,
                                       Poco::Net::HTTPMessage::HTTP_1_1);
        request.setHost(Root);
        request.set("User-Agent", http::getAgentString());
        request.set("ProxyPrefix", ProxyPrefix);

        RequestDetails details(request, "");
        LOK_ASSERT_EQUAL(true, details.isProxy());
        LOK_ASSERT_EQUAL(ProxyPrefix, details.getProxyPrefix());

        LOK_ASSERT_EQUAL(Root, details.getHostUntrusted());
        LOK_ASSERT_EQUAL(false, details.isWebSocket());
        LOK_ASSERT_EQUAL(true, details.isGet());

        LOK_ASSERT_EQUAL_STR("b26112ab1b6f2ed98ce1329f0f344791",
                             details.getField(RequestDetails::Field::SessionId));
        LOK_ASSERT_EQUAL_STR("close", details.getField(RequestDetails::Field::Command));
        LOK_ASSERT_EQUAL_STR("31", details.getField(RequestDetails::Field::Serial));

        const std::string docUri
            = "http://localhost/nextcloud/index.php/apps/richdocuments/wopi/files/"
              "593_ocqiesh0cngs?access_token=MN0KXXDv9GJ1wCCLnQcjVQT2T7WrfYpA&access_token_ttl=0&"
              "reuse_"
              "cookies=oc_sessionPassphrase%"
              "3D8nFRqycbs7bP97yxCuJviBbVKdCXmuiXp6ZYH0DfUoy5UZDCTQgLwluvbgRbKrdKodJteG3uNE19KNUAoE"
              "5typ"
              "f4oBGwJdFY%252F5W9RNST8wEHWkUVIjZy7vmY0ZX38PlS%3Anc_sameSiteCookielax%3Dtrue%3Anc_"
              "sameSiteCookiestrict%3Dtrue%3Aocqiesh0cngs%3Dr5ujg4tpvgu9paaf5bguiokgjl%"
              "3AXCookieName%"
              "3DXCookieValue%3ASuperCookieName%3DBAZINGA";

        LOK_ASSERT_EQUAL(docUri, details.getDocumentURI());

        const std::string wopiSrc
            = "http://localhost/nextcloud/index.php/apps/richdocuments/wopi/files/593_ocqiesh0cngs";

        LOK_ASSERT_EQUAL(wopiSrc, details.getField(RequestDetails::Field::WOPISrc));

        LOK_ASSERT_EQUAL(static_cast<std::size_t>(8), details.size());
        LOK_ASSERT_EQUAL_STR("cool", details[0]);
        LOK_ASSERT_EQUAL_STR("cool", details.getField(RequestDetails::Field::Type));
        LOK_ASSERT(details.equals(RequestDetails::Field::Type, "cool"));
        LOK_ASSERT(details.equals(0, "cool"));
        LOK_ASSERT_EQUAL(
            std::string(
                "http%3A%2F%2Flocalhost%2Fnextcloud%2Findex.php%2Fapps%2Frichdocuments%2Fwopi%"
                "2Ffiles%2F593_ocqiesh0cngs%3Faccess_token%3DMN0KXXDv9GJ1wCCLnQcjVQT2T7WrfYpA%"
                "26access_token_ttl%3D0%26reuse_cookies%3Doc_sessionPassphrase%"
                "253D8nFRqycbs7bP97yxCuJviBbVKdCXmuiXp6ZYH0DfUoy5UZDCTQgLwluvbgRbKrdKodJteG3uNE"
                "19KNUAoE5typf4oBGwJdFY%25252F5W9RNST8wEHWkUVIjZy7vmY0ZX38PlS%253Anc_"
                "sameSiteCookielax%253Dtrue%253Anc_sameSiteCookiestrict%253Dtrue%"
                "253Aocqiesh0cngs%253Dr5ujg4tpvgu9paaf5bguiokgjl%253AXCookieName%"
                "253DXCookieValue%253ASuperCookieName%253DBAZINGA"),
            details[1]);
        LOK_ASSERT_EQUAL_STR("ws", details[2]);
        LOK_ASSERT_EQUAL_STR("WOPISrc=http%3A%2F%2Flocalhost%2Fnextcloud%2Findex.php%2Fapps%"
                             "2Frichdocuments%2Fwopi%2Ffiles%2F593_ocqiesh0cngs&compat=",
                             details[3]);
        LOK_ASSERT_EQUAL_STR("ws", details[4]);
        LOK_ASSERT_EQUAL_STR("b26112ab1b6f2ed98ce1329f0f344791", details[5]);
        LOK_ASSERT_EQUAL_STR("close", details[6]);
        LOK_ASSERT_EQUAL_STR("31", details[7]);

        LOK_ASSERT_EQUAL_STR("cool", details.getField(RequestDetails::Field::Type));
        LOK_ASSERT(details.equals(RequestDetails::Field::Type, "cool"));
        LOK_ASSERT_EQUAL_STR("b26112ab1b6f2ed98ce1329f0f344791",
                             details.getField(RequestDetails::Field::SessionId));
        LOK_ASSERT(details.equals(RequestDetails::Field::SessionId, "b26112ab1b6f2ed98ce1329f0f344791"));
        LOK_ASSERT_EQUAL_STR("close", details.getField(RequestDetails::Field::Command));
        LOK_ASSERT(details.equals(RequestDetails::Field::Command, "close"));
        LOK_ASSERT_EQUAL_STR("31", details.getField(RequestDetails::Field::Serial));
        LOK_ASSERT(details.equals(RequestDetails::Field::Serial, "31"));

        http::Request request2(URI);
        request2.set("Host", Root);
        request2.set("User-Agent", http::getAgentString());
        request2.set("ProxyPrefix", ProxyPrefix);
        http::RequestParser reqParser(request2);
        LOK_ASSERT_EQUAL(details, RequestDetails(reqParser, ""));
    }

    {
        static const std::string URI
            = "/cool/"
              "http%3A%2F%2Flocalhost%2Fowncloud%2Findex.php%2Fapps%2Frichdocuments%2Fwopi%2Ffiles%"
              "2F165_ocgdpzbkm39u%3Faccess_token%3DODhIXdJdbsVYQoKKCuaYofyzrovxD3MQ%26access_token_"
              "ttl%"
              "3D0%26reuse_cookies%3DXCookieName%253DXCookieValue%253ASuperCookieName%253DBAZINGA/"
              "ws?WOPISrc=http%3A%2F%2Flocalhost%2Fowncloud%2Findex.php%2Fapps%2Frichdocuments%"
              "2Fwopi%"
              "2Ffiles%2F165_ocgdpzbkm39u&compat=/ws/1c99a7bcdbf3209782d7eb38512e6564/write/2";

        Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, URI,
                                       Poco::Net::HTTPMessage::HTTP_1_1);
        request.setHost(Root);
        request.set("User-Agent", http::getAgentString());
        request.set("ProxyPrefix", ProxyPrefix);

        RequestDetails details(request, "");
        LOK_ASSERT_EQUAL(true, details.isProxy());
        LOK_ASSERT_EQUAL(ProxyPrefix, details.getProxyPrefix());

        LOK_ASSERT_EQUAL(Root, details.getHostUntrusted());
        LOK_ASSERT_EQUAL(false, details.isWebSocket());
        LOK_ASSERT_EQUAL(true, details.isGet());

        const std::string docUri
            = "http://localhost/owncloud/index.php/apps/richdocuments/wopi/files/"
              "165_ocgdpzbkm39u?access_token=ODhIXdJdbsVYQoKKCuaYofyzrovxD3MQ&access_token_ttl=0&"
              "reuse_cookies=XCookieName%3DXCookieValue%3ASuperCookieName%3DBAZINGA";

        LOK_ASSERT_EQUAL(docUri, details.getDocumentURI());

        const std::string wopiSrc
            = "http://localhost/owncloud/index.php/apps/richdocuments/wopi/files/"
              "165_ocgdpzbkm39u";

        LOK_ASSERT_EQUAL(wopiSrc, details.getField(RequestDetails::Field::WOPISrc));

        LOK_ASSERT_EQUAL(static_cast<std::size_t>(8), details.size());
        LOK_ASSERT_EQUAL_STR("cool", details[0]);
        LOK_ASSERT(details.equals(0, "cool"));
        LOK_ASSERT_EQUAL_STR(
            "http%3A%2F%2Flocalhost%2Fowncloud%2Findex.php%2Fapps%2Frichdocuments%2Fwopi%2Ffiles%"
            "2F165_ocgdpzbkm39u%3Faccess_token%3DODhIXdJdbsVYQoKKCuaYofyzrovxD3MQ%26access_token_"
            "ttl%3D0%26reuse_cookies%3DXCookieName%253DXCookieValue%253ASuperCookieName%"
            "253DBAZINGA",
            details[1]);
        LOK_ASSERT_EQUAL_STR("ws", details[2]);
        LOK_ASSERT_EQUAL_STR("WOPISrc=http%3A%2F%2Flocalhost%2Fowncloud%2Findex.php%2Fapps%"
                             "2Frichdocuments%2Fwopi%2Ffiles%2F165_ocgdpzbkm39u&compat=",
                             details[3]);
        LOK_ASSERT_EQUAL_STR("ws", details[4]);
        LOK_ASSERT_EQUAL_STR("1c99a7bcdbf3209782d7eb38512e6564", details[5]);
        LOK_ASSERT_EQUAL_STR("write", details[6]);
        LOK_ASSERT_EQUAL_STR("2", details[7]);

        LOK_ASSERT_EQUAL_STR("cool", details.getField(RequestDetails::Field::Type));
        LOK_ASSERT(details.equals(RequestDetails::Field::Type, "cool"));
        LOK_ASSERT_EQUAL_STR("1c99a7bcdbf3209782d7eb38512e6564",
                             details.getField(RequestDetails::Field::SessionId));
        LOK_ASSERT(details.equals(RequestDetails::Field::SessionId, "1c99a7bcdbf3209782d7eb38512e6564"));
        LOK_ASSERT_EQUAL_STR("write", details.getField(RequestDetails::Field::Command));
        LOK_ASSERT(details.equals(RequestDetails::Field::Command, "write"));
        LOK_ASSERT_EQUAL_STR("2", details.getField(RequestDetails::Field::Serial));
        LOK_ASSERT(details.equals(RequestDetails::Field::Serial, "2"));

        http::Request request2(URI);
        request2.set("Host", Root);
        request2.set("User-Agent", http::getAgentString());
        request2.set("ProxyPrefix", ProxyPrefix);
        http::RequestParser reqParser(request2);
        LOK_ASSERT_EQUAL(details, RequestDetails(reqParser, ""));
    }

    {
        static const std::string URI
            = "/cool/%2Ftmp%2Fslideshow_b8c3225b_setclientpart.odp/Ar3M1X89mVaryYkh/"
              "UjaCGP4cYHlU6TvUGdnFTPi8hjOS87uFym7ruWMq3F3jBr0kSPgVhbKz5CwUyV8R/slideshow.svg";

        Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, URI,
                                       Poco::Net::HTTPMessage::HTTP_1_1);
        request.setHost(Root);
        request.set("User-Agent", http::getAgentString());
        request.set("ProxyPrefix", ProxyPrefix);

        RequestDetails details(request, "");
        LOK_ASSERT_EQUAL(true, details.isProxy());
        LOK_ASSERT_EQUAL(ProxyPrefix, details.getProxyPrefix());

        LOK_ASSERT_EQUAL(Root, details.getHostUntrusted());
        LOK_ASSERT_EQUAL(false, details.isWebSocket());
        LOK_ASSERT_EQUAL(true, details.isGet());

        const std::string docUri
            = "/tmp/slideshow_b8c3225b_setclientpart.odp";

        LOK_ASSERT_EQUAL(docUri, details.getDocumentURI());

        LOK_ASSERT_EQUAL(std::string(), details.getField(RequestDetails::Field::WOPISrc));

        LOK_ASSERT_EQUAL(static_cast<std::size_t>(5), details.size());
        LOK_ASSERT_EQUAL_STR("cool", details[0]);
        LOK_ASSERT(details.equals(0, "cool"));
        LOK_ASSERT_EQUAL_STR("%2Ftmp%2Fslideshow_b8c3225b_setclientpart.odp", details[1]);
        LOK_ASSERT_EQUAL_STR("Ar3M1X89mVaryYkh", details[2]);
        LOK_ASSERT_EQUAL_STR("UjaCGP4cYHlU6TvUGdnFTPi8hjOS87uFym7ruWMq3F3jBr0kSPgVhbKz5CwUyV8R",
                             details[3]);
        LOK_ASSERT_EQUAL_STR("slideshow.svg", details[4]);

        LOK_ASSERT_EQUAL_STR("cool", details.getField(RequestDetails::Field::Type));
        LOK_ASSERT(details.equals(RequestDetails::Field::Type, "cool"));
        LOK_ASSERT_EQUAL_STR("", details.getField(RequestDetails::Field::SessionId));
        LOK_ASSERT(details.equals(RequestDetails::Field::SessionId, ""));
        LOK_ASSERT_EQUAL_STR("", details.getField(RequestDetails::Field::Command));
        LOK_ASSERT(details.equals(RequestDetails::Field::Command, ""));
        LOK_ASSERT_EQUAL_STR("", details.getField(RequestDetails::Field::Serial));
        LOK_ASSERT(details.equals(RequestDetails::Field::Serial, ""));

        http::Request request2(URI);
        request2.set("Host", Root);
        request2.set("User-Agent", http::getAgentString());
        request2.set("ProxyPrefix", ProxyPrefix);
        http::RequestParser reqParser(request2);
        LOK_ASSERT_EQUAL(details, RequestDetails(reqParser, ""));
    }

    {
        static const std::string URI = "/cool/"
                                       "clipboard?WOPISrc=file%3A%2F%2F%2Ftmp%2Fcopypasteef324307_"
                                       "empty.ods&ServerId=7add98ed&ViewId=0&Tag=5f7972ce4e6a37dd";

        Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, URI,
                                       Poco::Net::HTTPMessage::HTTP_1_1);
        request.setHost(Root);
        request.set("User-Agent", http::getAgentString());
        request.set("ProxyPrefix", ProxyPrefix);

        RequestDetails details(request, "");
        LOK_ASSERT_EQUAL(true, details.isProxy());
        LOK_ASSERT_EQUAL(ProxyPrefix, details.getProxyPrefix());

        LOK_ASSERT_EQUAL(Root, details.getHostUntrusted());
        LOK_ASSERT_EQUAL(false, details.isWebSocket());
        LOK_ASSERT_EQUAL(true, details.isGet());

        const std::string docUri = "clipboard";

        LOK_ASSERT_EQUAL(docUri, details.getDocumentURI());

        LOK_ASSERT_EQUAL(static_cast<std::size_t>(3), details.size());
        LOK_ASSERT_EQUAL_STR("cool", details[0]);
        LOK_ASSERT(details.equals(0, "cool"));
        LOK_ASSERT_EQUAL_STR("clipboard", details[1]);

        LOK_ASSERT_EQUAL_STR("cool", details.getField(RequestDetails::Field::Type));
        LOK_ASSERT(details.equals(RequestDetails::Field::Type, "cool"));
        LOK_ASSERT_EQUAL_STR("", details.getField(RequestDetails::Field::SessionId));
        LOK_ASSERT(details.equals(RequestDetails::Field::SessionId, ""));
        LOK_ASSERT_EQUAL_STR("", details.getField(RequestDetails::Field::Command));
        LOK_ASSERT(details.equals(RequestDetails::Field::Command, ""));
        LOK_ASSERT_EQUAL_STR("", details.getField(RequestDetails::Field::Serial));
        LOK_ASSERT(details.equals(RequestDetails::Field::Serial, ""));

        http::Request request2(URI);
        request2.set("Host", Root);
        request2.set("User-Agent", http::getAgentString());
        request2.set("ProxyPrefix", ProxyPrefix);
        http::RequestParser reqParser(request2);
        LOK_ASSERT_EQUAL(details, RequestDetails(reqParser, ""));
    }

    {
        static const std::string URI =
            "/cool/"
            "https%3A%2F%2Fexample.com%3A8443%2Frest%2Ffiles%2Fwopi%2Ffiles%"
            "2F8ac75551de4d89e60002%3Faccess_header%3DAuthorization%253A%252520Bearer%"
            "252520poiuytrewq%25250D%25250A%25250D%25250AX-Requested-"
            "With%253A%252520XMLHttpRequest%26reuse_cookies%3Dlang%253Den-us%253A_ga_"
            "LMX4TVJ02K%253DGS1.1%"
            "253AToken%253DeyJhbGciOiJIUzUxMiJ9.vajknfkfajksdljfiwjek-"
            "W90fmgVb3C-00-eSkJBDqDNSYA%253APublicToken%"
            "253Dabc%253AZNPCQ003-32383700%253De9c71c3b%"
            "253AJSESSIONID%253Dnode0.node0%26permission%3Dedit/"
            "ws?WOPISrc=https%3A%2F%2Fexample.com%3A8443%2Frest%2Ffiles%2Fwopi%2Ffiles%"
            "2F8c74c1deff7dede002&compat=/ws";

        Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, URI,
                                       Poco::Net::HTTPMessage::HTTP_1_1);
        request.setHost(Root);
        request.set("User-Agent", http::getAgentString());
        request.set("ProxyPrefix", ProxyPrefix);

        RequestDetails details(request, "");
        LOK_ASSERT_EQUAL(true, details.isProxy());
        LOK_ASSERT_EQUAL(ProxyPrefix, details.getProxyPrefix());

        LOK_ASSERT_EQUAL(Root, details.getHostUntrusted());
        LOK_ASSERT_EQUAL(false, details.isWebSocket());
        LOK_ASSERT_EQUAL(true, details.isGet());

        const std::string docUri
            = "https://example.com:8443/rest/files/wopi/files/"
              "8ac75551de4d89e60002?access_header=Authorization%3A%2520Bearer%2520poiuytrewq%250D%"
              "250A%250D%250AX-Requested-With%3A%2520XMLHttpRequest&reuse_cookies=lang%3Den-us%3A_"
              "ga_LMX4TVJ02K%3DGS1.1%3AToken%3DeyJhbGciOiJIUzUxMiJ9.vajknfkfajksdljfiwjek-"
              "W90fmgVb3C-00-eSkJBDqDNSYA%3APublicToken%3Dabc%3AZNPCQ003-32383700%3De9c71c3b%"
              "3AJSESSIONID%3Dnode0.node0&permission=edit";

        LOK_ASSERT_EQUAL(docUri, details.getDocumentURI());

        const std::map<std::string, std::string>& params = details.getDocumentURIParams();
        LOK_ASSERT_EQUAL(static_cast<std::size_t>(3), params.size());
        auto it = params.find("access_header");
        const std::string access_header =
            "Authorization:%20Bearer%20poiuytrewq%0D%0A%0D%0AX-Requested-With:%20XMLHttpRequest";
        LOK_ASSERT_EQUAL(access_header, it != params.end() ? it->second : "");
        it = params.find("reuse_cookies");
        const std::string reuse_cookies
            = "lang=en-us:_ga_LMX4TVJ02K=GS1.1:Token=eyJhbGciOiJIUzUxMiJ9.vajknfkfajksdljfiwjek-"
              "W90fmgVb3C-00-eSkJBDqDNSYA:PublicToken=abc:ZNPCQ003-32383700=e9c71c3b:JSESSIONID="
              "node0.node0";
        LOK_ASSERT_EQUAL(reuse_cookies, it != params.end() ? it->second : "");
        it = params.find("permission");
        const std::string permission = "edit";
        LOK_ASSERT_EQUAL(permission, it != params.end() ? it->second : "");

        LOK_ASSERT_EQUAL(static_cast<std::size_t>(5), details.size());
        LOK_ASSERT_EQUAL_STR("cool", details[0]);
        LOK_ASSERT(details.equals(0, "cool"));

        const std::string encodedDocUri
            = "https%3A%2F%2Fexample.com%3A8443%2Frest%2Ffiles%2Fwopi%2Ffiles%"
              "2F8ac75551de4d89e60002%3Faccess_header%3DAuthorization%253A%252520Bearer%"
              "252520poiuytrewq%25250D%25250A%25250D%25250AX-Requested-With%253A%"
              "252520XMLHttpRequest%26reuse_cookies%3Dlang%253Den-us%253A_ga_LMX4TVJ02K%253DGS1.1%"
              "253AToken%253DeyJhbGciOiJIUzUxMiJ9.vajknfkfajksdljfiwjek-W90fmgVb3C-00-eSkJBDqDNSYA%"
              "253APublicToken%253Dabc%253AZNPCQ003-32383700%253De9c71c3b%253AJSESSIONID%253Dnode0."
              "node0%26permission%3Dedit";

        LOK_ASSERT_EQUAL(encodedDocUri, details[1]);

        LOK_ASSERT_EQUAL_STR("cool", details.getField(RequestDetails::Field::Type));
        LOK_ASSERT(details.equals(RequestDetails::Field::Type, "cool"));
        LOK_ASSERT_EQUAL_STR("", details.getField(RequestDetails::Field::SessionId));
        LOK_ASSERT(details.equals(RequestDetails::Field::SessionId, ""));
        LOK_ASSERT_EQUAL_STR("", details.getField(RequestDetails::Field::Command));
        LOK_ASSERT(details.equals(RequestDetails::Field::Command, ""));
        LOK_ASSERT_EQUAL_STR("", details.getField(RequestDetails::Field::Serial));
        LOK_ASSERT(details.equals(RequestDetails::Field::Serial, ""));

        http::Request request2(URI);
        request2.set("Host", Root);
        request2.set("User-Agent", http::getAgentString());
        request2.set("ProxyPrefix", ProxyPrefix);
        http::RequestParser reqParser(request2);
        LOK_ASSERT_EQUAL(details, RequestDetails(reqParser, ""));
    }
}

/// Tests the Cool URI 2.0.
void RequestDetailsTests::testCoolWs()
{
    constexpr std::string_view testname = __func__;

    static const std::string Root = "localhost:9980";

    static const std::string ProxyPrefix =
        "http://localhost/nextcloud/apps/richdocuments/proxy.php?req=";

    {
        static const std::string URI =
            "/cool/"
            "ws?WOPISrc=http%3A%2F%2Flocalhost%2Fnextcloud%2Findex.php%2Fapps%2Frichdocuments%"
            "2Fwopi%2Ffiles%2F593_ocqiesh0cngs&access_token=MN0KXXDv9GJ1wCCLnQcjVQT2T7WrfYpA&"
            "access_token_ttl=0&reuse_cookies=oc_sessionPassphrase%"
            "3D8nFRqycbs7bP97yxCuJviBbVKdCXmuiXp6ZYH0DfUoy5UZDCTQgLwluvbgRbKrdKodJteG3uNE19KNUAoE5t"
            "ypf4oBGwJdFY%252F5W9RNST8wEHWkUVIjZy7vmY0ZX38PlS%3Anc_sameSiteCookielax%3Dtrue%3Anc_"
            "sameSiteCookiestrict%3Dtrue%3Aocqiesh0cngs%3Dr5ujg4tpvgu9paaf5bguiokgjl%3AXCookieName%"
            "3DXCookieValue%3ASuperCookieName%3DBAZINGA&compat=b26112ab1b6f2ed98ce1329f0f344791%"
            "2Fclose%2F31";

        Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, URI,
                                       Poco::Net::HTTPMessage::HTTP_1_1);
        request.setHost(Root);
        request.set("User-Agent", http::getAgentString());
        request.set("ProxyPrefix", ProxyPrefix);

        RequestDetails details(request, "");
        LOK_ASSERT_EQUAL(true, details.isProxy());
        LOK_ASSERT_EQUAL(ProxyPrefix, details.getProxyPrefix());

        LOK_ASSERT_EQUAL(Root, details.getHostUntrusted());
        LOK_ASSERT_EQUAL(false, details.isWebSocket());
        LOK_ASSERT_EQUAL(true, details.isGet());

        LOK_ASSERT_EQUAL_STR("b26112ab1b6f2ed98ce1329f0f344791",
                             details.getField(RequestDetails::Field::SessionId));
        LOK_ASSERT_EQUAL_STR("close", details.getField(RequestDetails::Field::Command));
        LOK_ASSERT_EQUAL_STR("31", details.getField(RequestDetails::Field::Serial));

        const std::string docUri =
            "http://localhost/nextcloud/index.php/apps/richdocuments/wopi/files/"
            "593_ocqiesh0cngs?access_token=MN0KXXDv9GJ1wCCLnQcjVQT2T7WrfYpA&access_token_ttl=0&"
            "reuse_cookies=oc_sessionPassphrase%"
            "3D8nFRqycbs7bP97yxCuJviBbVKdCXmuiXp6ZYH0DfUoy5UZDCTQgLwluvbgRbKrdKodJteG3uNE19KNUAoE5t"
            "ypf4oBGwJdFY%252F5W9RNST8wEHWkUVIjZy7vmY0ZX38PlS%3Anc_sameSiteCookielax%3Dtrue%3Anc_"
            "sameSiteCookiestrict%3Dtrue%3Aocqiesh0cngs%3Dr5ujg4tpvgu9paaf5bguiokgjl%3AXCookieName%"
            "3DXCookieValue%3ASuperCookieName%3DBAZINGA";
        LOK_ASSERT_EQUAL(docUri, details.getDocumentURI());

        const std::string wopiSrc =
            "http://localhost/nextcloud/index.php/apps/richdocuments/wopi/files/593_ocqiesh0cngs";

        LOK_ASSERT_EQUAL(wopiSrc, details.getField(RequestDetails::Field::WOPISrc));

        LOK_ASSERT_EQUAL(static_cast<std::size_t>(3), details.size());
        LOK_ASSERT_EQUAL_STR("cool", details[0]);
        LOK_ASSERT_EQUAL_STR("cool", details.getField(RequestDetails::Field::Type));
        LOK_ASSERT(details.equals(RequestDetails::Field::Type, "cool"));
        LOK_ASSERT(details.equals(0, "cool"));
        LOK_ASSERT_EQUAL_STR("ws", details[1]);
        LOK_ASSERT_EQUAL_STR(
            "WOPISrc=http%3A%2F%2Flocalhost%2Fnextcloud%2Findex.php%2Fapps%2Frichdocuments%"
            "2Fwopi%2Ffiles%2F593_ocqiesh0cngs&access_token=MN0KXXDv9GJ1wCCLnQcjVQT2T7WrfYpA&"
            "access_token_ttl=0&reuse_cookies=oc_sessionPassphrase%"
            "3D8nFRqycbs7bP97yxCuJviBbVKdCXmuiXp6ZYH0DfUoy5UZDCTQgLwluvbgRbKrdKodJteG3uNE19KNUAoE5t"
            "ypf4oBGwJdFY%252F5W9RNST8wEHWkUVIjZy7vmY0ZX38PlS%3Anc_sameSiteCookielax%3Dtrue%3Anc_"
            "sameSiteCookiestrict%3Dtrue%3Aocqiesh0cngs%3Dr5ujg4tpvgu9paaf5bguiokgjl%3AXCookieName%"
            "3DXCookieValue%3ASuperCookieName%3DBAZINGA&compat=b26112ab1b6f2ed98ce1329f0f344791%"
            "2Fclose%2F31",
            details[2]);

        LOK_ASSERT_EQUAL_STR("cool", details.getField(RequestDetails::Field::Type));
        LOK_ASSERT(details.equals(RequestDetails::Field::Type, "cool"));
        LOK_ASSERT_EQUAL_STR("b26112ab1b6f2ed98ce1329f0f344791",
                             details.getField(RequestDetails::Field::SessionId));
        LOK_ASSERT(
            details.equals(RequestDetails::Field::SessionId, "b26112ab1b6f2ed98ce1329f0f344791"));
        LOK_ASSERT_EQUAL_STR("close", details.getField(RequestDetails::Field::Command));
        LOK_ASSERT(details.equals(RequestDetails::Field::Command, "close"));
        LOK_ASSERT_EQUAL_STR("31", details.getField(RequestDetails::Field::Serial));
        LOK_ASSERT(details.equals(RequestDetails::Field::Serial, "31"));

        http::Request request2(URI);
        request2.set("Host", Root);
        request2.set("User-Agent", http::getAgentString());
        request2.set("ProxyPrefix", ProxyPrefix);
        http::RequestParser reqParser(request2);
        LOK_ASSERT_EQUAL(details, RequestDetails(reqParser, ""));
    }

    {
        static const std::string URI =
            "/cool/"
            "ws?WOPISrc=http%3A%2F%2Flocalhost%2Fnextcloud%2Findex.php%2Fapps%2Frichdocuments%"
            "2Fwopi%2Ffiles%2F6734_ocqiesh0cngs&access_token%3DO87cwh0WlwIawoDkafkqOtVNTygxbiBN%"
            "26access_token_ttl%3D0%26no_a";

        Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, URI,
                                       Poco::Net::HTTPMessage::HTTP_1_1);
        request.setHost(Root);
        request.set("User-Agent", http::getAgentString());
        request.set("ProxyPrefix", ProxyPrefix);
        request.set("Upgrade", "websocket");

        RequestDetails details(request, "");
        LOK_ASSERT_EQUAL(true, details.isProxy());
        LOK_ASSERT_EQUAL(ProxyPrefix, details.getProxyPrefix());

        LOK_ASSERT_EQUAL(Root, details.getHostUntrusted());
        LOK_ASSERT(details.isWebSocket());
        LOK_ASSERT_EQUAL(true, details.isGet());

        LOK_ASSERT_EQUAL_STR(std::string(), details.getField(RequestDetails::Field::SessionId));
        LOK_ASSERT_EQUAL_STR(std::string(), details.getField(RequestDetails::Field::Command));
        LOK_ASSERT_EQUAL_STR(std::string(), details.getField(RequestDetails::Field::Serial));

        const std::string docUri =
            "http://localhost/nextcloud/index.php/apps/richdocuments/wopi/files/"
            "6734_ocqiesh0cngs?access_token=O87cwh0WlwIawoDkafkqOtVNTygxbiBN&access_token_ttl=0&no_"
            "a=";
        LOK_ASSERT_EQUAL(docUri, details.getDocumentURI());

        const std::string wopiSrc =
            "http://localhost/nextcloud/index.php/apps/richdocuments/wopi/files/6734_ocqiesh0cngs";

        LOK_ASSERT_EQUAL(wopiSrc, details.getField(RequestDetails::Field::WOPISrc));

        LOK_ASSERT_EQUAL(static_cast<std::size_t>(3), details.size());
        LOK_ASSERT_EQUAL_STR("cool", details[0]);
        LOK_ASSERT_EQUAL_STR("cool", details.getField(RequestDetails::Field::Type));
        LOK_ASSERT(details.equals(RequestDetails::Field::Type, "cool"));
        LOK_ASSERT(details.equals(0, "cool"));
        LOK_ASSERT(details.equals(1, "ws"));

        LOK_ASSERT_EQUAL_STR("ws", details[1]);
        LOK_ASSERT_EQUAL_STR(
            "WOPISrc=http%3A%2F%2Flocalhost%2Fnextcloud%2Findex.php%2Fapps%2Frichdocuments%"
            "2Fwopi%2Ffiles%2F6734_ocqiesh0cngs&access_token%3DO87cwh0WlwIawoDkafkqOtVNTygxbiBN%"
            "26access_token_ttl%3D0%26no_a",
            details[2]);

        LOK_ASSERT_EQUAL_STR("cool", details.getField(RequestDetails::Field::Type));
        LOK_ASSERT(details.equals(RequestDetails::Field::Type, "cool"));
        LOK_ASSERT_EQUAL_STR(std::string(), details.getField(RequestDetails::Field::SessionId));
        LOK_ASSERT(details.equals(RequestDetails::Field::SessionId, std::string()));
        LOK_ASSERT_EQUAL_STR(std::string(), details.getField(RequestDetails::Field::Command));
        LOK_ASSERT(details.equals(RequestDetails::Field::Command, std::string()));
        LOK_ASSERT_EQUAL_STR(std::string(), details.getField(RequestDetails::Field::Serial));
        LOK_ASSERT(details.equals(RequestDetails::Field::Serial, std::string()));

        http::Request request2(URI);
        request2.set("Host", Root);
        request2.set("User-Agent", http::getAgentString());
        request2.set("ProxyPrefix", ProxyPrefix);
        request2.set("Upgrade", "WebSocket");
        http::RequestParser reqParser(request2);
        LOK_ASSERT_EQUAL(details, RequestDetails(reqParser, ""));
    }

    {
        static const std::string URI =
            "/cool/"
            "http%3A%2F%2Flocalhost%2Fowncloud%2Findex.php%2Fapps%2Frichdocuments%2Fwopi%2Ffiles%"
            "2F165_ocgdpzbkm39u%3Faccess_token%3DODhIXdJdbsVYQoKKCuaYofyzrovxD3MQ%26access_token_"
            "ttl%"
            "3D0%26reuse_cookies%3DXCookieName%253DXCookieValue%253ASuperCookieName%253DBAZINGA/"
            "ws?WOPISrc=http%3A%2F%2Flocalhost%2Fowncloud%2Findex.php%2Fapps%2Frichdocuments%"
            "2Fwopi%"
            "2Ffiles%2F165_ocgdpzbkm39u&compat=/ws/1c99a7bcdbf3209782d7eb38512e6564/write/2";

        Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, URI,
                                       Poco::Net::HTTPMessage::HTTP_1_1);
        request.setHost(Root);
        request.set("User-Agent", http::getAgentString());
        request.set("ProxyPrefix", ProxyPrefix);

        RequestDetails details(request, "");
        LOK_ASSERT_EQUAL(true, details.isProxy());
        LOK_ASSERT_EQUAL(ProxyPrefix, details.getProxyPrefix());

        LOK_ASSERT_EQUAL(Root, details.getHostUntrusted());
        LOK_ASSERT_EQUAL(false, details.isWebSocket());
        LOK_ASSERT_EQUAL(true, details.isGet());

        const std::string docUri =
            "http://localhost/owncloud/index.php/apps/richdocuments/wopi/files/"
            "165_ocgdpzbkm39u?access_token=ODhIXdJdbsVYQoKKCuaYofyzrovxD3MQ&access_token_ttl=0&"
            "reuse_cookies=XCookieName%3DXCookieValue%3ASuperCookieName%3DBAZINGA";

        LOK_ASSERT_EQUAL(docUri, details.getDocumentURI());

        const std::string wopiSrc =
            "http://localhost/owncloud/index.php/apps/richdocuments/wopi/files/"
            "165_ocgdpzbkm39u";

        LOK_ASSERT_EQUAL(wopiSrc, details.getField(RequestDetails::Field::WOPISrc));

        LOK_ASSERT_EQUAL(static_cast<std::size_t>(8), details.size());
        LOK_ASSERT_EQUAL_STR("cool", details[0]);
        LOK_ASSERT(details.equals(0, "cool"));
        LOK_ASSERT_EQUAL_STR(
            "http%3A%2F%2Flocalhost%2Fowncloud%2Findex.php%2Fapps%2Frichdocuments%2Fwopi%2Ffiles%"
            "2F165_ocgdpzbkm39u%3Faccess_token%3DODhIXdJdbsVYQoKKCuaYofyzrovxD3MQ%26access_token_"
            "ttl%3D0%26reuse_cookies%3DXCookieName%253DXCookieValue%253ASuperCookieName%"
            "253DBAZINGA",
            details[1]);
        LOK_ASSERT_EQUAL_STR("ws", details[2]);
        LOK_ASSERT_EQUAL_STR("WOPISrc=http%3A%2F%2Flocalhost%2Fowncloud%2Findex.php%2Fapps%"
                             "2Frichdocuments%2Fwopi%2Ffiles%2F165_ocgdpzbkm39u&compat=",
                             details[3]);
        LOK_ASSERT_EQUAL_STR("ws", details[4]);
        LOK_ASSERT_EQUAL_STR("1c99a7bcdbf3209782d7eb38512e6564", details[5]);
        LOK_ASSERT_EQUAL_STR("write", details[6]);
        LOK_ASSERT_EQUAL_STR("2", details[7]);

        LOK_ASSERT_EQUAL_STR("cool", details.getField(RequestDetails::Field::Type));
        LOK_ASSERT(details.equals(RequestDetails::Field::Type, "cool"));
        LOK_ASSERT_EQUAL_STR("1c99a7bcdbf3209782d7eb38512e6564",
                             details.getField(RequestDetails::Field::SessionId));
        LOK_ASSERT(
            details.equals(RequestDetails::Field::SessionId, "1c99a7bcdbf3209782d7eb38512e6564"));
        LOK_ASSERT_EQUAL_STR("write", details.getField(RequestDetails::Field::Command));
        LOK_ASSERT(details.equals(RequestDetails::Field::Command, "write"));
        LOK_ASSERT_EQUAL_STR("2", details.getField(RequestDetails::Field::Serial));
        LOK_ASSERT(details.equals(RequestDetails::Field::Serial, "2"));

        http::Request request2(URI);
        request2.set("Host", Root);
        request2.set("User-Agent", http::getAgentString());
        request2.set("ProxyPrefix", ProxyPrefix);
        http::RequestParser reqParser(request2);
        LOK_ASSERT_EQUAL(details, RequestDetails(reqParser, ""));
    }

    {
        static const std::string URI =
            "/cool/%2Ftmp%2Fslideshow_b8c3225b_setclientpart.odp/Ar3M1X89mVaryYkh/"
            "UjaCGP4cYHlU6TvUGdnFTPi8hjOS87uFym7ruWMq3F3jBr0kSPgVhbKz5CwUyV8R/slideshow.svg";

        Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, URI,
                                       Poco::Net::HTTPMessage::HTTP_1_1);
        request.setHost(Root);
        request.set("User-Agent", http::getAgentString());
        request.set("ProxyPrefix", ProxyPrefix);

        RequestDetails details(request, "");
        LOK_ASSERT_EQUAL(true, details.isProxy());
        LOK_ASSERT_EQUAL(ProxyPrefix, details.getProxyPrefix());

        LOK_ASSERT_EQUAL(Root, details.getHostUntrusted());
        LOK_ASSERT_EQUAL(false, details.isWebSocket());
        LOK_ASSERT_EQUAL(true, details.isGet());

        const std::string docUri = "/tmp/slideshow_b8c3225b_setclientpart.odp";

        LOK_ASSERT_EQUAL(docUri, details.getDocumentURI());

        LOK_ASSERT_EQUAL(std::string(), details.getField(RequestDetails::Field::WOPISrc));

        LOK_ASSERT_EQUAL(static_cast<std::size_t>(5), details.size());
        LOK_ASSERT_EQUAL_STR("cool", details[0]);
        LOK_ASSERT(details.equals(0, "cool"));
        LOK_ASSERT_EQUAL_STR("%2Ftmp%2Fslideshow_b8c3225b_setclientpart.odp", details[1]);
        LOK_ASSERT_EQUAL_STR("Ar3M1X89mVaryYkh", details[2]);
        LOK_ASSERT_EQUAL_STR("UjaCGP4cYHlU6TvUGdnFTPi8hjOS87uFym7ruWMq3F3jBr0kSPgVhbKz5CwUyV8R",
                             details[3]);
        LOK_ASSERT_EQUAL_STR("slideshow.svg", details[4]);

        LOK_ASSERT_EQUAL_STR("cool", details.getField(RequestDetails::Field::Type));
        LOK_ASSERT(details.equals(RequestDetails::Field::Type, "cool"));
        LOK_ASSERT_EQUAL_STR("", details.getField(RequestDetails::Field::SessionId));
        LOK_ASSERT(details.equals(RequestDetails::Field::SessionId, ""));
        LOK_ASSERT_EQUAL_STR("", details.getField(RequestDetails::Field::Command));
        LOK_ASSERT(details.equals(RequestDetails::Field::Command, ""));
        LOK_ASSERT_EQUAL_STR("", details.getField(RequestDetails::Field::Serial));
        LOK_ASSERT(details.equals(RequestDetails::Field::Serial, ""));

        http::Request request2(URI);
        request2.set("Host", Root);
        request2.set("User-Agent", http::getAgentString());
        request2.set("ProxyPrefix", ProxyPrefix);
        http::RequestParser reqParser(request2);
        LOK_ASSERT_EQUAL(details, RequestDetails(reqParser, ""));
    }

    {
        static const std::string URI = "/cool/"
                                       "clipboard?WOPISrc=file%3A%2F%2F%2Ftmp%2Fcopypasteef324307_"
                                       "empty.ods&ServerId=7add98ed&ViewId=0&Tag=5f7972ce4e6a37dd";

        Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, URI,
                                       Poco::Net::HTTPMessage::HTTP_1_1);
        request.setHost(Root);
        request.set("User-Agent", http::getAgentString());
        request.set("ProxyPrefix", ProxyPrefix);

        RequestDetails details(request, "");
        LOK_ASSERT_EQUAL(true, details.isProxy());
        LOK_ASSERT_EQUAL(ProxyPrefix, details.getProxyPrefix());

        LOK_ASSERT_EQUAL(Root, details.getHostUntrusted());
        LOK_ASSERT_EQUAL(false, details.isWebSocket());
        LOK_ASSERT_EQUAL(true, details.isGet());

        const std::string docUri = "clipboard";

        LOK_ASSERT_EQUAL(docUri, details.getDocumentURI());

        LOK_ASSERT_EQUAL(static_cast<std::size_t>(3), details.size());
        LOK_ASSERT_EQUAL_STR("cool", details[0]);
        LOK_ASSERT(details.equals(0, "cool"));
        LOK_ASSERT_EQUAL_STR("clipboard", details[1]);

        LOK_ASSERT_EQUAL_STR("cool", details.getField(RequestDetails::Field::Type));
        LOK_ASSERT(details.equals(RequestDetails::Field::Type, "cool"));
        LOK_ASSERT_EQUAL_STR("", details.getField(RequestDetails::Field::SessionId));
        LOK_ASSERT(details.equals(RequestDetails::Field::SessionId, ""));
        LOK_ASSERT_EQUAL_STR("", details.getField(RequestDetails::Field::Command));
        LOK_ASSERT(details.equals(RequestDetails::Field::Command, ""));
        LOK_ASSERT_EQUAL_STR("", details.getField(RequestDetails::Field::Serial));
        LOK_ASSERT(details.equals(RequestDetails::Field::Serial, ""));

        http::Request request2(URI);
        request2.set("Host", Root);
        request2.set("User-Agent", http::getAgentString());
        request2.set("ProxyPrefix", ProxyPrefix);
        http::RequestParser reqParser(request2);
        LOK_ASSERT_EQUAL(details, RequestDetails(reqParser, ""));
    }

    {
        static const std::string URI =
            "/cool/"
            "ws?WOPISrc=https%3A%2F%2Fexample.com%3A8443%2Frest%2Ffiles%2Fwopi%2Ffiles%"
            "2F8ac75551de4d89e60002&access_header=Authorization%3A%2520Bearer%2520poiuytrewq%250D%"
            "250A%250D%250AX-Requested-With%3A%2520XMLHttpRequest&reuse_cookies=lang%3Den-us%3A_ga_"
            "LMX4TVJ02K%3DGS1.1%3AToken%3DeyJhbGciOiJIUzUxMiJ9.vajknfkfajksdljfiwjek-W90fmgVb3C-00-"
            "eSkJBDqDNSYA%3APublicToken%3Dabc%3AZNPCQ003-32383700%3De9c71c3b%3AJSESSIONID%3Dnode0."
            "node0&permission=edit&compat=/ws";

        Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, URI,
                                       Poco::Net::HTTPMessage::HTTP_1_1);
        request.setHost(Root);
        request.set("User-Agent", http::getAgentString());
        request.set("ProxyPrefix", ProxyPrefix);

        RequestDetails details(request, "");
        LOK_ASSERT_EQUAL(true, details.isProxy());
        LOK_ASSERT_EQUAL(ProxyPrefix, details.getProxyPrefix());

        LOK_ASSERT_EQUAL(Root, details.getHostUntrusted());
        LOK_ASSERT_EQUAL(false, details.isWebSocket());
        LOK_ASSERT_EQUAL(true, details.isGet());

        const std::string docUri =
            "https://example.com:8443/rest/files/wopi/files/"
            "8ac75551de4d89e60002?access_header=Authorization%3A%2520Bearer%2520poiuytrewq%250D%"
            "250A%250D%250AX-Requested-With%3A%2520XMLHttpRequest&permission=edit&reuse_cookies="
            "lang%3Den-us%3A_ga_LMX4TVJ02K%3DGS1.1%3AToken%3DeyJhbGciOiJIUzUxMiJ9."
            "vajknfkfajksdljfiwjek-W90fmgVb3C-00-eSkJBDqDNSYA%3APublicToken%3Dabc%3AZNPCQ003-"
            "32383700%3De9c71c3b%3AJSESSIONID%3Dnode0.node0";

        LOK_ASSERT_EQUAL(docUri, details.getDocumentURI());

        const std::map<std::string, std::string>& params = details.getDocumentURIParams();
        LOK_ASSERT_EQUAL(static_cast<std::size_t>(3), params.size());
        auto it = params.find("access_header");
        const std::string access_header =
            "Authorization:%20Bearer%20poiuytrewq%0D%0A%0D%0AX-Requested-With:%20XMLHttpRequest";
        LOK_ASSERT_EQUAL(access_header, it != params.end() ? it->second : "");
        it = params.find("reuse_cookies");
        const std::string reuse_cookies =
            "lang=en-us:_ga_LMX4TVJ02K=GS1.1:Token=eyJhbGciOiJIUzUxMiJ9.vajknfkfajksdljfiwjek-"
            "W90fmgVb3C-00-eSkJBDqDNSYA:PublicToken=abc:ZNPCQ003-32383700=e9c71c3b:JSESSIONID="
            "node0.node0";
        LOK_ASSERT_EQUAL(reuse_cookies, it != params.end() ? it->second : "");
        it = params.find("permission");
        const std::string permission = "edit";
        LOK_ASSERT_EQUAL(permission, it != params.end() ? it->second : "");

        LOK_ASSERT_EQUAL(static_cast<std::size_t>(4), details.size());
        LOK_ASSERT_EQUAL_STR("cool", details[0]);
        LOK_ASSERT(details.equals(0, "cool"));
        LOK_ASSERT_EQUAL_STR("ws", details[1]);
        LOK_ASSERT(details.equals(1, "ws"));

        const std::string encodedDocUri =
            "WOPISrc=https%3A%2F%2Fexample.com%3A8443%2Frest%2Ffiles%2Fwopi%2Ffiles%"
            "2F8ac75551de4d89e60002&access_header=Authorization%3A%2520Bearer%2520poiuytrewq%250D%"
            "250A%250D%250AX-Requested-With%3A%2520XMLHttpRequest&reuse_cookies=lang%3Den-us%3A_ga_"
            "LMX4TVJ02K%3DGS1.1%3AToken%3DeyJhbGciOiJIUzUxMiJ9.vajknfkfajksdljfiwjek-W90fmgVb3C-00-"
            "eSkJBDqDNSYA%3APublicToken%3Dabc%3AZNPCQ003-32383700%3De9c71c3b%3AJSESSIONID%3Dnode0."
            "node0&permission=edit&compat=";

        LOK_ASSERT_EQUAL(encodedDocUri, details[2]);

        LOK_ASSERT_EQUAL_STR("cool", details.getField(RequestDetails::Field::Type));
        LOK_ASSERT(details.equals(RequestDetails::Field::Type, "cool"));
        LOK_ASSERT_EQUAL_STR("ws", details.getField(RequestDetails::Field::SessionId));
        LOK_ASSERT(details.equals(RequestDetails::Field::SessionId, "ws"));
        LOK_ASSERT_EQUAL_STR("", details.getField(RequestDetails::Field::Command));
        LOK_ASSERT(details.equals(RequestDetails::Field::Command, ""));
        LOK_ASSERT_EQUAL_STR("", details.getField(RequestDetails::Field::Serial));
        LOK_ASSERT(details.equals(RequestDetails::Field::Serial, ""));

        http::Request request2(URI);
        request2.set("Host", Root);
        request2.set("User-Agent", http::getAgentString());
        request2.set("ProxyPrefix", ProxyPrefix);
        http::RequestParser reqParser(request2);
        LOK_ASSERT_EQUAL(details, RequestDetails(reqParser, ""));
    }
}
void RequestDetailsTests::testAuthorization()
{
    constexpr std::string_view testname = __func__;

    Authorization auth1(Authorization::Type::Token, "abc", false);
    Poco::URI uri1("http://localhost");
    auth1.authorizeURI(uri1);
    LOK_ASSERT_EQUAL_STR("http://localhost/?access_token=abc", uri1.toString());
    Poco::Net::HTTPRequest req1;
    auth1.authorizeRequest(req1);
    LOK_ASSERT_EQUAL_STR("Bearer abc", req1.get("Authorization"));

    Authorization auth1modify(Authorization::Type::Token, "modified", false);
    // still the same uri1, currently "http://localhost/?access_token=abc"
    auth1modify.authorizeURI(uri1);
    LOK_ASSERT_EQUAL_STR("http://localhost/?access_token=modified", uri1.toString());

    Authorization auth2(Authorization::Type::Header, "def", false);
    Poco::Net::HTTPRequest req2;
    auth2.authorizeRequest(req2);
    LOK_ASSERT(!req2.has("Authorization"));

    Authorization auth3(Authorization::Type::Header, "Authorization: Basic huhu== ", false);
    Poco::URI uri2("http://localhost");
    auth3.authorizeURI(uri2);
    // nothing added with the Authorization header approach
    LOK_ASSERT_EQUAL_STR("http://localhost", uri2.toString());
    Poco::Net::HTTPRequest req3;
    auth3.authorizeRequest(req3);
    LOK_ASSERT_EQUAL_STR("Basic huhu==", req3.get("Authorization"));

    Authorization auth4(Authorization::Type::Header, "  Authorization: Basic blah== \n\rX-Something:   additional  ", false);
    Poco::Net::HTTPRequest req4;
    auth4.authorizeRequest(req4);
    LOK_ASSERT_MESSAGE("Exected request to have Authorization header", req4.has("Authorization"));
    LOK_ASSERT_EQUAL_STR("Basic blah==", req4.get("Authorization"));
    LOK_ASSERT_MESSAGE("Exected request to have X-Something header", req4.has("X-Something"));
    LOK_ASSERT_EQUAL_STR("additional", req4.get("X-Something"));

    Authorization auth5(Authorization::Type::Header, "  Authorization: Basic huh== \n\rX-Something-More:   else  \n\r", false);
    Poco::Net::HTTPRequest req5;
    auth5.authorizeRequest(req5);
    LOK_ASSERT_EQUAL_STR("Basic huh==", req5.get("Authorization"));
    LOK_ASSERT_EQUAL_STR("else", req5.get("X-Something-More"));

    Authorization auth6(Authorization::Type::None, "Authorization: basic huh==", false);
    Poco::Net::HTTPRequest req6;
    CPPUNIT_ASSERT_NO_THROW(auth6.authorizeRequest(req6));

    {
        const std::string WorkingDocumentURI
            = "https://example.com:8443/rest/files/wopi/files/"
              "8ac75551de4d89e60002?access_header=Authorization%3A%2520Bearer%25201hpoiuytrewq%"
              "250D%250A%250D%250AX-Requested-With%3A%2520XMLHttpRequest&reuse_cookies=lang%3Den-"
              "us%3A_xx_%3DGS1.1.%3APublicToken%"
              "3DeyJzdWIiOiJhZG1pbiIsImV4cCI6MTU4ODkxNzc3NCwiaWF0IjoxNTg4OTE2ODc0LCJqdGkiOiI4OGZhN2"
              "E3ZC1lMzU5LTQ2OWEtYjg3Zi02NmFhNzI0ZGFkNTcifQ%3AZNPCQ003-32383700%3De9c71c3b%"
              "3AJSESSIONID%3Dnode019djohorurnaf1eo6f57ejhg0520.node0&permission=edit";

        const std::string AuthorizationParam = "Bearer 1hpoiuytrewq";

        Authorization auth(Authorization::create(WorkingDocumentURI));
        Poco::Net::HTTPRequest req;
        auth.authorizeRequest(req);
        LOK_ASSERT_EQUAL(AuthorizationParam, req.get("Authorization"));
        LOK_ASSERT_EQUAL_STR("XMLHttpRequest", req.get("X-Requested-With"));
    }

    {
        const std::string URI
            = "https://example.com:8443/rest/files/wopi/files/"
              "24e3f0a17230cca5017230fb6861000c?access_header=Authorization%3A%20Bearer%"
              "201hpoiuytrewq%0D%0A%0D%0AX-Requested-With%3A%20XMLHttpRequest";

        const std::string AuthorizationParam = "Bearer 1hpoiuytrewq";

        Authorization auth7(Authorization::create(URI));
        Poco::Net::HTTPRequest req7;
        auth7.authorizeRequest(req7);
        LOK_ASSERT_EQUAL(AuthorizationParam, req7.get("Authorization"));
        LOK_ASSERT_EQUAL_STR("XMLHttpRequest", req7.get("X-Requested-With"));
    }

    {
        const std::string URI
            = "https://example.com:8443/rest/files/wopi/files/"
              "8ac75551de4d89e60002?reuse_cookies=lang%3Den-us%3A_xx_%3DGS1.1.%3APublicToken%"
              "3DeyJzdWIiOiJhZG1pbiIsImV4cCI6MTU4ODkxNzc3NCwiaWF0IjoxNTg4OTE2ODc0LCJqdGkiOiI4OGZhN2"
              "E3ZC1lMzU5LTQ2OWEtYjg3Zi02NmFhNzI0ZGFkNTcifQ%3AZNPCQ003-32383700%3De9c71c3b%"
              "3AJSESSIONID%3Dnode019djohorurnaf1eo6f57ejhg0520.node0&permission=edit";

        Authorization auth7(Authorization::create(URI));
        Poco::Net::HTTPRequest req;
        auth7.authorizeRequest(req);
    }
}

void RequestDetailsTests::testAuthorizationExpiry()
{
    constexpr std::string_view testname = __func__;

    using duration = std::chrono::milliseconds;

    // A token with no expiry should not be expired.
    {
        Authorization auth(Authorization::Type::Token, "tok1", false);
        LOK_ASSERT(!auth.isExpired());
        LOK_ASSERT(!auth.needTokenRefresh());
    }

    // A token with a far-future expiry should not be expired.
    {
        Authorization auth(Authorization::Type::Token, "tok2", false);
        const auto futureMs = std::chrono::system_clock::now().time_since_epoch() +
                              std::chrono::hours(1);
        auth.setExpiryEpoch(std::chrono::duration_cast<duration>(futureMs));
        LOK_ASSERT(!auth.isExpired());
        LOK_ASSERT(!auth.needTokenRefresh());
    }

    // A token with a past expiry should be expired.
    {
        Authorization auth(Authorization::Type::Token, "tok3", false);
        const auto pastMs = std::chrono::system_clock::now().time_since_epoch() -
                            std::chrono::seconds(1);
        auth.setExpiryEpoch(std::chrono::duration_cast<duration>(pastMs));
        LOK_ASSERT(auth.isExpired());
        LOK_ASSERT(auth.needTokenRefresh());
        // Regression: a naturally-expired Token (startTokenRefresh never called) must
        // not report a refresh-wait timeout, otherwise the poll-loop kills the session.
        LOK_ASSERT(!auth.isRefreshingToken());
        LOK_ASSERT(!auth.isTokenRefreshTimedOut());
    }

    // expire() should mark as expired regardless of TTL.
    {
        Authorization auth(Authorization::Type::Token, "tok4", false);
        LOK_ASSERT(!auth.isExpired());
        auth.expire();
        LOK_ASSERT(auth.isExpired());
    }

    // resetAccessToken should clear expired state.
    {
        Authorization auth(Authorization::Type::Token, "tok5", false);
        auth.expire();
        LOK_ASSERT(auth.isExpired());
        const auto futureMs = std::chrono::system_clock::now().time_since_epoch() +
                              std::chrono::hours(1);
        auth.resetAccessToken("tok5_new",
                              std::chrono::duration_cast<duration>(futureMs));
        LOK_ASSERT(!auth.isExpired());
        LOK_ASSERT(!auth.needTokenRefresh());
    }

    // resetAccessToken with a past expiry should be expired.
    {
        Authorization auth(Authorization::Type::Token, "tok6", false);
        const auto pastMs = std::chrono::system_clock::now().time_since_epoch() -
                            std::chrono::seconds(1);
        auth.resetAccessToken("tok6_new",
                              std::chrono::duration_cast<duration>(pastMs));
        LOK_ASSERT(auth.isExpired());
    }

    // Authorization::create with access_token_ttl should set expiry.
    {
        const auto futureMs = std::chrono::duration_cast<duration>(
            std::chrono::system_clock::now().time_since_epoch() + std::chrono::hours(1));
        const std::string uri = "http://host/wopi/files/0?access_token=secret&access_token_ttl=" +
                                std::to_string(futureMs.count());
        Authorization auth = Authorization::create(uri);
        LOK_ASSERT(!auth.isExpired());
    }

    // Authorization::create with past access_token_ttl should be expired.
    {
        const auto pastMs = std::chrono::duration_cast<duration>(
            std::chrono::system_clock::now().time_since_epoch() - std::chrono::seconds(1));
        const std::string uri = "http://host/wopi/files/0?access_token=secret&access_token_ttl=" +
                                std::to_string(pastMs.count());
        Authorization auth = Authorization::create(uri);
        LOK_ASSERT(auth.isExpired());
    }

    // Token refresh: startTokenRefresh, isRefreshingToken, isTokenRefreshTimedOut.
    {
        Authorization auth(Authorization::Type::Token, "tok7", false);
        LOK_ASSERT(!auth.isRefreshingToken());

        auth.startTokenRefresh(std::chrono::seconds(1));
        LOK_ASSERT(auth.isRefreshingToken());
        LOK_ASSERT(auth.needTokenRefresh());

        // Should not have timed out yet.
        LOK_ASSERT(!auth.isTokenRefreshTimedOut());

        // Reset should clear the refreshing state.
        const auto futureMs = std::chrono::system_clock::now().time_since_epoch() +
                              std::chrono::hours(1);
        auth.resetAccessToken("tok7_new",
                              std::chrono::duration_cast<duration>(futureMs));
        LOK_ASSERT(!auth.isRefreshingToken());
        LOK_ASSERT(!auth.needTokenRefresh());
    }

    // Type::None should not be expired.
    {
        Authorization auth(Authorization::Type::None, "", false);
        LOK_ASSERT(!auth.isExpired());
        LOK_ASSERT(!auth.needTokenRefresh());
    }
}

void RequestDetailsTests::testSanitizePercent()
{
    constexpr std::string_view testname = __func__;

    // sanitizeURI with file:// scheme: Uri::decode turns %25 -> %, then sanitizeLocalPath
    // treats % as literal.
    {
        Poco::URI result = RequestDetails::sanitizeURI("file:///tmp/he%25llo.odt");
        LOK_ASSERT_EQUAL(std::string("/tmp/he%llo.odt"), result.getPath());
    }

    // sanitizeURI with relative path: Uri::decode turns %2525 -> %25, then sanitizeLocalPath
    // treats %25 as literal.
    {
        Poco::URI result = RequestDetails::sanitizeURI("/tmp/hello%2525world.odt");
        LOK_ASSERT_EQUAL(std::string("/tmp/hello%25world.odt"), result.getPath());
    }

    // sanitizeLocalPath: '%' in path is always literal, never URI encoding.
    {
        Poco::URI result = RequestDetails::sanitizeLocalPath("/tmp/he%llo.odt");
        LOK_ASSERT_EQUAL(std::string("/tmp/he%llo.odt"), result.getPath());
    }

    // sanitizeLocalPath: '%25' in path is literal text, not an encoded '%'.
    {
        Poco::URI result = RequestDetails::sanitizeLocalPath("/tmp/hello%25world.odt");
        LOK_ASSERT_EQUAL(std::string("/tmp/hello%25world.odt"), result.getPath());
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(RequestDetailsTests);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
