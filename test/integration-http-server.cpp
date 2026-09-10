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
 * Test server for HTTP integration testing.
 */

#include <config.h>

#include <helpers.hpp>
#include <lokassert.hpp>
#include <net/HttpRequest.hpp>
#include <Session.hpp>
#include <Common.hpp>
#include <common/FileUtil.hpp>
#include <KitPidHelpers.hpp>

#include <regex>
#include <Poco/URI.h>

#include <cppunit/extensions/HelperMacros.h>

#include <memory>

/// Tests the HTTP GET API of coolwsd.
class HTTPServerTest : public CPPUNIT_NS::TestFixture
{
    const Poco::URI _uri;

    CPPUNIT_TEST_SUITE(HTTPServerTest);

    CPPUNIT_TEST(testCoolGet);
    CPPUNIT_TEST(testCoolPostPoco);
    CPPUNIT_TEST(testCoolPost);
    CPPUNIT_TEST(testIntegratorSettingsUnrestrictedFraming);
    CPPUNIT_TEST(testScriptsAndLinksGet);
    CPPUNIT_TEST(testScriptsAndLinksPost);
    CPPUNIT_TEST(testConvertTo);
    CPPUNIT_TEST(testConvertTo2);
    CPPUNIT_TEST(testConvertToWithForwardedIP_Deny);
    CPPUNIT_TEST(testConvertToWithForwardedIP_Allow);
    CPPUNIT_TEST(testConvertToWithForwardedIP_DenyMulti);
    CPPUNIT_TEST(testExtractDocStructure);
    CPPUNIT_TEST(testTransformDocStructure);
    CPPUNIT_TEST(testRenderSearchResult);

    CPPUNIT_TEST_SUITE_END();

    void testCoolGet();
    void testCoolPostPoco();
    void testCoolPost();
    void testIntegratorSettingsUnrestrictedFraming();
    void testScriptsAndLinksGet();
    void testScriptsAndLinksPost();
    void testConvertTo();
    void testConvertTo2();
    void testConvertToWithForwardedIP_Deny();
    void testConvertToWithForwardedIP_Allow();
    void testConvertToWithForwardedIP_DenyMulti();
    void testExtractDocStructure();
    void testTransformDocStructure();
    void testRenderSearchResult();

protected:
    void assertHTTPFilesExist(const Poco::URI& uri, const std::regex& expr,
                              const std::string& html, const std::string& mimetype,
                              const std::string_view testname);

public:
    HTTPServerTest()
        : _uri(helpers::getTestServerURI())
    {
    }

    ~HTTPServerTest()
    {
    }

    void setUp()
    {
        helpers::resetTestStartTime();
        helpers::waitForKitPidsReady("setUp");
        helpers::resetTestStartTime();
    }

    void tearDown()
    {
        helpers::resetTestStartTime();
        helpers::waitForKitPidsReady("tearDown");
        helpers::resetTestStartTime();
    }

    // A server URI which was not added to coolwsd.xml as post_allow IP or a wopi storage host
    Poco::URI getNotAllowedTestServerURI()
    {
        static std::string serverURI(
#if ENABLE_SSL
            "https://165.227.162.232:9980"
#else
            "http://165.227.162.232:9980"
#endif
            );

        return Poco::URI(serverURI);
    }
};

void HTTPServerTest::testCoolGet()
{
    constexpr std::string_view testname = __func__;

    const auto pathAndQuery = "/browser/dist/cool.html?access_token=111111111";
    const std::shared_ptr<const http::Response> httpResponse
        = http::get(_uri.toString(), pathAndQuery);

    LOK_ASSERT_EQUAL(http::StatusCode::OK, httpResponse->statusLine().statusCode());
    LOK_ASSERT_EQUAL_STR("text/html", httpResponse->header().getContentType());

    const std::string& html = httpResponse->getBody();
    LOK_ASSERT(html.find("111111111") != std::string::npos);
    LOK_ASSERT(html.find(_uri.getHost()) != std::string::npos);
    LOK_ASSERT(html.find(Util::getCoolVersionHash()) != std::string::npos);
}

void HTTPServerTest::testCoolPostPoco()
{
    constexpr std::string_view testname = __func__;

    auto httpSession = http::Session::create(_uri.toString());
    http::Request request("/browser/dist/cool.html", http::Request::VERB_POST);
    // URL-encode form data
    std::string body = "access_token=2222222222&buy_product=https%3A%2F%2Fjim%3Abob%40nowhere.com%2Fother%2Fstuff%3Fa%3Db%3Bc%3Dd%23somethingelse";
    request.setBody(body, "application/x-www-form-urlencoded");
    auto httpResponse = httpSession->syncRequest(request);

    LOK_ASSERT_EQUAL(http::StatusCode::OK, httpResponse->statusCode());

    const std::string& html = httpResponse->getBody();

    LOK_ASSERT(html.find("2222222222") != std::string::npos);
    LOK_ASSERT(html.find("https://jim:bob@nowhere.com/other/stuff?a=b;c=d#somethingelse") != std::string::npos);
    LOK_ASSERT(html.find(_uri.getHost()) != std::string::npos);

    std::string csp = httpResponse->get("Content-Security-Policy");
    StringVector lines = StringVector::tokenize(csp, ';');
    TST_LOG("CSP - " << csp << " tokens " << lines.size());
    for (size_t i = 0; i < lines.size(); ++i)
    {
        if(lines.startsWith(i, " connect-src") ||
           lines.startsWith(i, " frame-src") ||
           lines.startsWith(i, " img-src"))
        {
            StringVector split = StringVector::tokenize(lines[i], ' ');

            for (size_t j = 1; j < split.size(); ++j)
            {
                if (split[j] == "'self'" || split[j] == "data:" || split[j] == "blob:")
                    continue;

                Poco::URI uri(split[j]);
                TST_LOG("URL - " << split[j]);
                LOK_ASSERT_EQUAL_STR("", uri.getUserInfo());
                LOK_ASSERT(uri.getPath() == std::string("") ||
                           uri.getPath() == std::string("*"));
                LOK_ASSERT_EQUAL_STR("", uri.getQuery());
                LOK_ASSERT_EQUAL_STR("", uri.getFragment());
            }
        }
    }

    std::string wildcard = "https://*.collabora.com";
    LOK_ASSERT_EQUAL(wildcard, Util::trimURI(wildcard));
}

void HTTPServerTest::testCoolPost()
{
    constexpr std::string_view testname = __func__;

    const auto pathAndQuery = "/browser/dist/"
                              "cool.html?WOPISrc=https%3A%2F%2Flocalhost%2Fnextcloud%2Findex.php%"
                              "2Fapps%2Frichdocuments%2Fwopi%2Ffiles%2F8725_ocqiesh0cngs&title="
                              "Test.Weekly.odt&lang=en&closebutton=1&revisionhistory=1";

    http::Request httpRequest(pathAndQuery, http::Request::VERB_POST);

    httpRequest.set("Cache-Control", "max-age=0");
    httpRequest.set("sec-ch-ua",
                    "\"Not.A/Brand\";v=\"8\", \"Chromium\";v=\"114\", \"Google Chrome\";v=\"114\"");
    httpRequest.set("sec-ch-ua-mobile", "?0");
    httpRequest.set("sec-ch-ua-platform", "\"Linux\"");
    httpRequest.set("Upgrade-Insecure-Requests", "1");
    httpRequest.set("Origin", "null");
    httpRequest.set("User-Agent", "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like "
                                  "Gecko) Chrome/114.0.0.0 Safari/537.36");
    httpRequest.set("Accept",
                    "text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/"
                    "webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7");
    httpRequest.set("Sec-Fetch-Site", "same-site");
    httpRequest.set("Sec-Fetch-Mode", "navigate");
    httpRequest.set("Sec-Fetch-Dest", "iframe");
    httpRequest.set("Accept-Encoding", "gzip, deflate, br");
    httpRequest.set("Accept-Language", "en-US,en;q=0.9");

    httpRequest.setBody(
        "access_token=choMXq0rSMcsm0RoZZWDWsrgAcE5AHwc&ui_defaults=TextRuler%3Dfalse%3BTextSidebar%"
        "3Dfalse%3BTextStatusbar%3Dfalse%3BPresentationSidebar%3Dfalse%3BPresentationStatusbar%"
        "3Dfalse%3BSpreadsheetSidebar%3Dfalse%3BSpreadsheetStatusbar%3Dfalse%3BUIMode%3Dclassic%3B&"
        "postmessage_origin=https://www.example.com:8080&"
        "css_variables=--co-primary-text%3D%23ffffff%3B--co-primary-element%3D%230082c9%3B--co-"
        "text-accent%3D%230082c9%3B--co-primary-light%3D%23e6f3fa%3B--co-primary-element-light%3D%"
        "2317adff%3B--co-color-error%3D%23e9322d%3B--co-color-warning%3D%23eca700%3B--co-color-"
        "success%3D%2346ba61%3B--co-border-radius%3D3px%3B--co-border-radius-large%3D10px%3B--co-"
        "loading-light%3D%23ccc%3B--co-loading-dark%3D%23444%3B--co-box-shadow%3Drgba%2877%2C+77%"
        "2C+77%2C+0.5%29%3B--co-border%3D%23ededed%3B--co-border-dark%3D%23dbdbdb%3B--co-border-"
        "radius-pill%3D100px%3B&theme=nextcloud",
        "application/x-www-form-urlencoded");

    std::shared_ptr<http::Session> httpSession = http::Session::create(_uri.toString());
    const std::shared_ptr<const http::Response> httpResponse =
        httpSession->syncRequest(httpRequest, http::Session::getDefaultTimeout());

    LOK_ASSERT_EQUAL(http::StatusCode::OK, httpResponse->statusLine().statusCode());
    LOK_ASSERT_EQUAL_STR("text/html", httpResponse->header().getContentType());

    const std::string& html = httpResponse->getBody();
    fprintf(stderr, "%s\n", html.c_str());
    LOK_ASSERT(html.find(_uri.getHost()) != std::string::npos);
    LOK_ASSERT(html.find("data-version-path = \"" + Util::getCoolVersionHash() + '"') !=
               std::string::npos);
    LOK_ASSERT(html.find("data-coolwsd-version = \"" + Util::getCoolVersion() + '"') !=
               std::string::npos);
    LOK_ASSERT(html.find("choMXq0rSMcsm0RoZZWDWsrgAcE5AHwc") != std::string::npos);
    LOK_ASSERT(html.find("data-access-token = \"choMXq0rSMcsm0RoZZWDWsrgAcE5AHwc\"") !=
               std::string::npos);
    LOK_ASSERT(html.find("data-access-token-ttl = \"0\"") != std::string::npos);
    LOK_ASSERT(html.find("data-access-header = \"\"") != std::string::npos);
    LOK_ASSERT(html.find("data-post-message-origin-ext = \"https://www.example.com:8080\"") != std::string::npos);
    LOK_ASSERT(html.find("data-frame-ancestors = \"%20127.0.0.1:%2A%20localhost:%2A\"") != std::string::npos);
    LOK_ASSERT(html.find("data-ui-defaults = \"eyJwcmVzZW50YXRpb24iOnsiU2hvd1NpZGViYXIiOiJmYWxzZSIsIlNob3dTdGF0dXNiYXIiOiJmYWxzZSJ9LCJzcHJlYWRzaGVldCI6eyJTaG93U2lkZWJhciI6ImZhbHNlIiwiU2hvd1N0YXR1c2JhciI6ImZhbHNlIn0sInRleHQiOnsiU2hvd1J1bGVyIjoiZmFsc2UiLCJTaG93U2lkZWJhciI6ImZhbHNlIiwiU2hvd1N0YXR1c2JhciI6ImZhbHNlIn0sInVpTW9kZSI6ImNsYXNzaWMifQ==\"") != std::string::npos);
    LOK_ASSERT(html.find("OnJvb3Qgey0tY28tcHJpbWFyeS10ZXh0OiNmZmZmZmY7LS1jby1wcmltYXJ5LWVsZW1lbnQ6IzAwODJjOTstLWNvLXRleHQtYWNjZW50OiMwMDgyYzk7LS1jby1wcmltYXJ5LWxpZ2h0OiNlNmYzZmE7LS1jby1wcmltYXJ5LWVsZW1lbnQtbGlnaHQ6IzE3YWRmZjstLWNvLWNvbG9yLWVycm9yOiNlOTMyMmQ7LS1jby1jb2xvci13YXJuaW5nOiNlY2E3MDA7LS1jby1jb2xvci1zdWNjZXNzOiM0NmJhNjE7LS1jby1ib3JkZXItcmFkaXVzOjNweDstLWNvLWJvcmRlci1yYWRpdXMtbGFyZ2U6MTBweDstLWNvLWxvYWRpbmctbGlnaHQ6I2NjYzstLWNvLWxvYWRpbmctZGFyazojNDQ0Oy0tY28tYm94LXNoYWRvdzpyZ2JhKDc3LCA3NywgNzcsIDAuNSk7LS1jby1ib3JkZXI6I2VkZWRlZDstLWNvLWJvcmRlci1kYXJrOiNkYmRiZGI7LS1jby1ib3JkZXItcmFkaXVzLXBpbGw6MTAwcHg7fQ") != std::string::npos);
}

void HTTPServerTest::assertHTTPFilesExist(const Poco::URI& uri, const std::regex& expr,
                                          const std::string& html, const std::string& mimetype,
                                          const std::string_view testname)
{
    bool found = false;

    for (auto it = std::sregex_iterator(html.begin(), html.end(), expr); it != std::sregex_iterator(); ++it)
    {
        const std::smatch& matches = *it;
        found = true;
        LOK_ASSERT_EQUAL(2, (int)matches.size());
        Poco::URI uriScript(matches[1].str());
        if (uriScript.getHost().empty())
        {
            std::string scriptString(uriScript.toString());

            // ignore the branding bits, it's not an error when they aren't present.
            if (scriptString.find("/branding.") != std::string::npos)
                continue;

            auto scriptResponse = http::get(uri.toString(), scriptString);
            std::string msg("cool.html references: " + scriptString + " which should exist.");
            LOK_ASSERT_EQUAL_MESSAGE(msg, static_cast<unsigned>(http::StatusCode::OK), static_cast<unsigned>(scriptResponse->statusCode()));

            if (!mimetype.empty())
            LOK_ASSERT_EQUAL(mimetype, scriptResponse->header().getContentType());
        }
    }

    LOK_ASSERT_MESSAGE("No match found", found);
}

/// With no frame ancestor named in the config, the integrator settings page carries no
/// frame-ancestors directive at all and any integrator may frame it. Only a configured list is
/// widened with coolwsd's own host and the integrator's host, so this default has to stay
/// unrestricted.
void HTTPServerTest::testIntegratorSettingsUnrestrictedFraming()
{
    constexpr std::string_view testname = __func__;

    http::Request httpRequest("/browser/dist/adminIntegratorSettings.html",
                              http::Request::VERB_POST);
    httpRequest.setBody("wopi_setting_base_url=https%3A%2F%2Fintegrator.example.org%2Fsettings",
                        "application/x-www-form-urlencoded");

    std::shared_ptr<http::Session> httpSession = http::Session::create(_uri.toString());
    const std::shared_ptr<const http::Response> httpResponse =
        httpSession->syncRequest(httpRequest, http::Session::getDefaultTimeout());

    LOK_ASSERT_EQUAL(http::StatusCode::OK, httpResponse->statusLine().statusCode());

    const std::string csp =
        httpResponse->header().get("Content-Security-Policy", std::string());
    LOK_ASSERT_MESSAGE("The settings page must not restrict framing when the config names no "
                       "frame ancestor; its CSP was [" +
                           csp + ']',
                       csp.find("frame-ancestors") == std::string::npos);
}

void HTTPServerTest::testScriptsAndLinksGet()
{
    constexpr std::string_view testname = __func__;

    auto httpResponse = http::get(_uri.toString(), "/browser/dist/cool.html");
    LOK_ASSERT_EQUAL(http::StatusCode::OK, httpResponse->statusCode());
    const std::string& html = httpResponse->getBody();

    std::regex script("<script.*?src=\"(.*?)\"");
    assertHTTPFilesExist(_uri, script, html, "application/javascript", testname);

    std::regex link("<link.*?href=\"(.*?)\"");
    assertHTTPFilesExist(_uri, link, html, std::string(), testname);
}

void HTTPServerTest::testScriptsAndLinksPost()
{
    constexpr std::string_view testname = __func__;

    auto httpSession = http::Session::create(_uri.toString());
    http::Request request("/browser/dist/cool.html", http::Request::VERB_POST);
    request.setBody("", "text/plain");
    auto httpResponse = httpSession->syncRequest(request);
    LOK_ASSERT_EQUAL(http::StatusCode::OK, httpResponse->statusCode());
    const std::string& html = httpResponse->getBody();

    std::regex script("<script.*?src=\"(.*?)\"");
    assertHTTPFilesExist(_uri, script, html, "application/javascript", testname);

    std::regex link("<link.*?href=\"(.*?)\"");
    assertHTTPFilesExist(_uri, link, html, std::string(), testname);
}

void HTTPServerTest::testConvertTo()
{
    const char *testname = "testConvertTo";
    const std::string srcPath = helpers::getTempFileCopyPath(TDOC, "hello.odt", "convertTo_");

    TST_LOG("Convert-to odt -> txt");

    auto httpSession = http::Session::create(_uri.toString());
    httpSession->setTimeout(std::chrono::seconds(COMMAND_TIMEOUT_SECS));
    http::Request request("/cool/convert-to", http::Request::VERB_POST);
    helpers::MultipartFormBody form;
    form.addField("format", "txt");
    form.addFile("data", srcPath);
    form.applyTo(request);
    auto httpResponse = httpSession->syncRequest(request);
    if (httpResponse->state() != http::Response::State::Complete)
    {
        std::this_thread::sleep_for(std::chrono::seconds(COMMAND_TIMEOUT_SECS));
        httpSession = http::Session::create(_uri.toString());
        httpSession->setTimeout(std::chrono::seconds(COMMAND_TIMEOUT_SECS));
        httpResponse = httpSession->syncRequest(request);
    }

    std::ifstream fileStream(TDOC "/hello.txt");
    std::stringstream expectedStream;
    expectedStream << fileStream.rdbuf();

    // Remove the temp files.
    FileUtil::removeFile(srcPath);

    // In some cases the result is prefixed with (the UTF-8 encoding of) the Unicode BOM
    // (U+FEFF). Skip that.
    std::string actualString = httpResponse->getBody();
    if (actualString.size() > 3 && actualString[0] == '\xEF' && actualString[1] == '\xBB' && actualString[2] == '\xBF')
        actualString = actualString.substr(3);
    LOK_ASSERT_EQUAL(expectedStream.str(), actualString);
}

void HTTPServerTest::testConvertTo2()
{
    const char *testname = "testConvertTo2";
    const std::string srcPath = helpers::getTempFileCopyPath(TDOC, "convert-to.xlsx", "convertTo_");

    TST_LOG("Convert-to #2 xlsx -> png");

    auto httpSession = http::Session::create(_uri.toString());
    httpSession->setTimeout(std::chrono::seconds(COMMAND_TIMEOUT_SECS * 2));
    http::Request request("/cool/convert-to", http::Request::VERB_POST);
    helpers::MultipartFormBody form;
    form.addField("format", "png");
    form.addFile("data", srcPath);
    form.applyTo(request);
    auto httpResponse = httpSession->syncRequest(request);
    if (httpResponse->state() != http::Response::State::Complete)
    {
        std::this_thread::sleep_for(std::chrono::seconds(COMMAND_TIMEOUT_SECS));
        httpSession = http::Session::create(_uri.toString());
        httpSession->setTimeout(std::chrono::seconds(COMMAND_TIMEOUT_SECS * 2));
        httpResponse = httpSession->syncRequest(request);
    }

    // Remove the temp files.
    FileUtil::removeFile(srcPath);

    std::string actualString = httpResponse->getBody();
    LOK_ASSERT(actualString.size() >= 100);
//  LOK_ASSERT_EQUAL(actualString[0], 0x89);
    LOK_ASSERT_EQUAL(actualString[1], 'P');
    LOK_ASSERT_EQUAL(actualString[2], 'N');
    LOK_ASSERT_EQUAL(actualString[3], 'G');
}

void HTTPServerTest::testConvertToWithForwardedIP_Deny()
{
    const std::string testname = "convertToWithForwardedClientIP-Deny";
    constexpr int TimeoutSeconds = COMMAND_TIMEOUT_SECS * 2; // Sometimes dns resolving is slow.

    // Test a forwarded IP which is not allowed to use convert-to feature
    TST_LOG("Converting from a disallowed IP.");

    const std::string srcPath = helpers::getTempFileCopyPath(TDOC, "hello.odt", testname);

    auto httpSession = http::Session::create(_uri.toString());
    httpSession->setTimeout(std::chrono::seconds(TimeoutSeconds));
    http::Request request("/cool/convert-to", http::Request::VERB_POST);
    LOK_ASSERT(!request.has("X-Forwarded-For"));
    request.add("X-Forwarded-For", getNotAllowedTestServerURI().getHost() + ", " + _uri.getHost());
    helpers::MultipartFormBody form;
    form.addField("format", "txt");
    form.addFile("data", srcPath);
    form.applyTo(request);
    auto httpResponse = httpSession->syncRequest(request);
    if (httpResponse->state() != http::Response::State::Complete)
    {
        std::this_thread::sleep_for(std::chrono::seconds(COMMAND_TIMEOUT_SECS));
        httpSession = http::Session::create(_uri.toString());
        httpSession->setTimeout(std::chrono::seconds(TimeoutSeconds));
        httpResponse = httpSession->syncRequest(request);
    }

    // Remove the temp files.
    FileUtil::removeFile(srcPath);

    std::string actualString = httpResponse->getBody();
    LOK_ASSERT(actualString.empty()); // <- we did not get the converted file
}

void HTTPServerTest::testConvertToWithForwardedIP_Allow()
{
    const std::string testname = "convertToWithForwardedClientIP-Allow";
    constexpr int TimeoutSeconds = COMMAND_TIMEOUT_SECS * 2; // Sometimes dns resolving is slow.

    // Test a forwarded IP which is allowed to use convert-to feature
    TST_LOG("Converting from an allowed IP.");

    const std::string srcPath = helpers::getTempFileCopyPath(TDOC, "hello.odt", testname);

    auto httpSession = http::Session::create(_uri.toString());
    httpSession->setTimeout(std::chrono::seconds(TimeoutSeconds));
    http::Request request("/cool/convert-to", http::Request::VERB_POST);
    LOK_ASSERT(!request.has("X-Forwarded-For"));
    request.add("X-Forwarded-For", _uri.getHost() + ", " + _uri.getHost());
    helpers::MultipartFormBody form;
    form.addField("format", "txt");
    form.addFile("data", srcPath);
    form.applyTo(request);
    auto httpResponse = httpSession->syncRequest(request);
    if (httpResponse->state() != http::Response::State::Complete)
    {
        std::this_thread::sleep_for(std::chrono::seconds(COMMAND_TIMEOUT_SECS));
        httpSession = http::Session::create(_uri.toString());
        httpSession->setTimeout(std::chrono::seconds(TimeoutSeconds));
        httpResponse = httpSession->syncRequest(request);
    }

    std::ifstream fileStream(TDOC "/hello.txt");
    std::stringstream expectedStream;
    expectedStream << fileStream.rdbuf();

    // Remove the temp files.
    FileUtil::removeFile(srcPath);

    // In some cases the result is prefixed with (the UTF-8 encoding of) the Unicode BOM
    // (U+FEFF). Skip that.
    std::string actualString = httpResponse->getBody();
    if (actualString.size() > 3 && actualString[0] == '\xEF' && actualString[1] == '\xBB' && actualString[2] == '\xBF')
        actualString = actualString.substr(3);
    LOK_ASSERT_EQUAL(expectedStream.str(), actualString); // <- we got the converted file
}

void HTTPServerTest::testConvertToWithForwardedIP_DenyMulti()
{
    const std::string testname = "convertToWithForwardedClientIP-DenyMulti";
    constexpr int TimeoutSeconds = COMMAND_TIMEOUT_SECS * 2; // Sometimes dns resolving is slow.

    // Test a forwarded header with three IPs, one is not allowed -> request is denied.
    TST_LOG("Converting from multiple IPs, on disallowed.");

    const std::string srcPath = helpers::getTempFileCopyPath(TDOC, "hello.odt", testname);

    auto httpSession = http::Session::create(_uri.toString());
    httpSession->setTimeout(std::chrono::seconds(TimeoutSeconds));
    http::Request request("/cool/convert-to", http::Request::VERB_POST);
    LOK_ASSERT(!request.has("X-Forwarded-For"));
    request.add("X-Forwarded-For", _uri.getHost() + ", "
                                   + getNotAllowedTestServerURI().getHost() + ", "
                                   + _uri.getHost());
    helpers::MultipartFormBody form;
    form.addField("format", "txt");
    form.addFile("data", srcPath);
    form.applyTo(request);
    auto httpResponse = httpSession->syncRequest(request);
    if (httpResponse->state() != http::Response::State::Complete)
    {
        std::this_thread::sleep_for(std::chrono::seconds(COMMAND_TIMEOUT_SECS));
        httpSession = http::Session::create(_uri.toString());
        httpSession->setTimeout(std::chrono::seconds(TimeoutSeconds));
        httpResponse = httpSession->syncRequest(request);
    }

    // Remove the temp files.
    FileUtil::removeFile(srcPath);

    std::string actualString = httpResponse->getBody();
    LOK_ASSERT(actualString.empty()); // <- we did not get the converted file
}

void HTTPServerTest::testExtractDocStructure()
{
    const char *testname = "testExtractDocStructure";
    const std::string srcPath = helpers::getTempFileCopyPath(TDOC, "docStructure.docx", "docStructure_");

    TST_LOG("extract-document-structure");

    auto httpSession = http::Session::create(_uri.toString());
    httpSession->setTimeout(std::chrono::seconds(COMMAND_TIMEOUT_SECS * 2));
    http::Request request("/cool/extract-document-structure", http::Request::VERB_POST);
    helpers::MultipartFormBody form;
    form.addField("filter", "contentcontrol");
    form.addFile("data", srcPath);
    form.applyTo(request);
    auto httpResponse = httpSession->syncRequest(request);
    if (httpResponse->state() != http::Response::State::Complete)
    {
        std::this_thread::sleep_for(std::chrono::seconds(COMMAND_TIMEOUT_SECS));
        httpSession = http::Session::create(_uri.toString());
        httpSession->setTimeout(std::chrono::seconds(COMMAND_TIMEOUT_SECS * 2));
        httpResponse = httpSession->syncRequest(request);
    }

    // Remove the temp files.
    FileUtil::removeFile(srcPath);

    std::string actualString = httpResponse->getBody();
    std::string expectedString = " { \"DocStructure\": { \"ContentControls.ByIndex.0\": { \"id\": -428815899, \"tag\": \"machine-readable\", \"alias\": \"Human Readable\", \"content\": \"plain text value\", \"type\": \"plain-text\"}, \"ContentControls.ByIndex.1\": { \"id\": -1833055349, \"tag\": \"name\", \"alias\": \"Name\", \"content\": \"\", \"type\": \"plain-text\"}}}";

    LOK_ASSERT_EQUAL(expectedString, actualString );
}

void HTTPServerTest::testTransformDocStructure()
{
    const char *testname = "testTransformDocStructure";
    {
        const std::string srcPath = helpers::getTempFileCopyPath(TDOC, "docStructure.docx", "docStructure_");

        TST_LOG("transform-document-structure");

        auto httpSession = http::Session::create(_uri.toString());
        httpSession->setTimeout(std::chrono::seconds(COMMAND_TIMEOUT_SECS * 2));
        http::Request request("/cool/transform-document-structure", http::Request::VERB_POST);
        helpers::MultipartFormBody form;
        form.addField("format", "docx");
        form.addField("transform", "{\"Transforms\":{\"ContentControls.ByIndex.0\":{\"content\":\"Short text\"}}}");
        form.addFile("data", srcPath);
        form.applyTo(request);
        auto httpResponse = httpSession->syncRequest(request);
        if (httpResponse->state() != http::Response::State::Complete)
        {
            std::this_thread::sleep_for(std::chrono::seconds(COMMAND_TIMEOUT_SECS));
            httpSession = http::Session::create(_uri.toString());
            httpSession->setTimeout(std::chrono::seconds(COMMAND_TIMEOUT_SECS * 2));
            httpResponse = httpSession->syncRequest(request);
        }

        // Remove the temp files.
        FileUtil::removeFile(srcPath);

        std::string actualString = httpResponse->getBody();

        std::ofstream fileStream(TDOC "/docStructureTransformed.docx");
        fileStream << actualString;
    }
    //To check the result, extract Document Structure
    {
        const std::string srcPath2 = helpers::getTempFileCopyPath(TDOC, "docStructureTransformed.docx", "docStructureTransformed_");

        TST_LOG("transform-document-structure-check");

        auto httpSession = http::Session::create(_uri.toString());
        httpSession->setTimeout(std::chrono::seconds(COMMAND_TIMEOUT_SECS * 2));
        http::Request request("/cool/extract-document-structure", http::Request::VERB_POST);
        helpers::MultipartFormBody form;
        form.addField("format", "docx");
        form.addField("filter", "contentcontrol");
        form.addFile("data", srcPath2);
        form.applyTo(request);
        auto httpResponse = httpSession->syncRequest(request);
        if (httpResponse->state() != http::Response::State::Complete)
        {
            std::this_thread::sleep_for(std::chrono::seconds(COMMAND_TIMEOUT_SECS));
            httpSession = http::Session::create(_uri.toString());
            httpSession->setTimeout(std::chrono::seconds(COMMAND_TIMEOUT_SECS * 2));
            httpResponse = httpSession->syncRequest(request);
        }

        // Remove the temp files.
        FileUtil::removeFile(srcPath2);

        std::string actualString = httpResponse->getBody();
        std::string expectedString = " { \"DocStructure\": { \"ContentControls.ByIndex.0\": { \"id\": -428815899, \"tag\": \"machine-readable\", \"alias\": \"Human Readable\", \"content\": \"Short text\", \"type\": \"plain-text\"}, \"ContentControls.ByIndex.1\": { \"id\": -1833055349, \"tag\": \"name\", \"alias\": \"Name\", \"content\": \"\", \"type\": \"plain-text\"}}}";

        LOK_ASSERT_EQUAL(expectedString, actualString );
    }
}

void HTTPServerTest::testRenderSearchResult()
{
    const char* testname = "testRenderSearchResult";
    const std::string srcPathDoc = helpers::getTempFileCopyPath(TDOC, "RenderSearchResultTest.odt", testname);
    const std::string srcPathXml = helpers::getTempFileCopyPath(TDOC, "RenderSearchResultFragment.xml", testname);

    auto httpSession = http::Session::create(_uri.toString());
    httpSession->setTimeout(std::chrono::seconds(COMMAND_TIMEOUT_SECS * 2));
    http::Request request("/cool/render-search-result", http::Request::VERB_POST);
    helpers::MultipartFormBody form;
    form.addFile("document", srcPathDoc);
    form.addFile("result", srcPathXml);
    form.applyTo(request);
    auto httpResponse = httpSession->syncRequest(request);
    if (httpResponse->state() != http::Response::State::Complete)
    {
        std::this_thread::sleep_for(std::chrono::seconds(COMMAND_TIMEOUT_SECS));
        httpSession = http::Session::create(_uri.toString());
        httpSession->setTimeout(std::chrono::seconds(COMMAND_TIMEOUT_SECS * 2));
        httpResponse = httpSession->syncRequest(request);
    }

    // Remove the temp files.
    FileUtil::removeFile(srcPathDoc);
    FileUtil::removeFile(srcPathXml);

    std::string actualString = httpResponse->getBody();

    LOK_ASSERT(actualString.size() >= 100);
    LOK_ASSERT_EQUAL(actualString[1], 'P');
    LOK_ASSERT_EQUAL(actualString[2], 'N');
    LOK_ASSERT_EQUAL(actualString[3], 'G');
}

CPPUNIT_TEST_SUITE_REGISTRATION(HTTPServerTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
