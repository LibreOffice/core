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

#include <net/HttpRequest.hpp>
#include <test/MockStreamSocket.hpp>
#include <wsd/COOLWSD.hpp>
#include <wsd/COOLWSDServer.hpp>
#include <wsd/ClientRequestDispatcher.hpp>
#include <wsd/ContentType.hpp>
#include <wsd/FileServer.hpp>
#include <wsd/RequestDetails.hpp>
#include <wsd/ServerURL.hpp>

#include <test/lokassert.hpp>
#include <test/testlog.hpp>

#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>

#include <memory>
#include <string>

#include <Poco/Net/HTTPRequest.h>

/// Unit tests for ClientRequestDispatcher.
class ClientRequestDispatcherTests : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(ClientRequestDispatcherTests);

    CPPUNIT_TEST(testGetContentType_Writer);
    CPPUNIT_TEST(testGetContentType_Calc);
    CPPUNIT_TEST(testGetContentType_Impress);
    CPPUNIT_TEST(testGetContentType_Draw);
    CPPUNIT_TEST(testGetContentType_MSOffice);
    CPPUNIT_TEST(testGetContentType_OOXML);
    CPPUNIT_TEST(testGetContentType_Images);
    CPPUNIT_TEST(testGetContentType_Other);
    CPPUNIT_TEST(testGetContentType_Unknown);
    CPPUNIT_TEST(testGetContentType_PathWithDirs);
    CPPUNIT_TEST(testGetContentType_CaseExtension);

    CPPUNIT_TEST(testIsSpreadsheet_Calc);
    CPPUNIT_TEST(testIsSpreadsheet_Excel);
    CPPUNIT_TEST(testIsSpreadsheet_OOXML);
    CPPUNIT_TEST(testIsSpreadsheet_NotSpreadsheet);

    CPPUNIT_TEST(testMockStreamSocket_Send);
    CPPUNIT_TEST(testMockStreamSocket_HttpResponse);
    CPPUNIT_TEST(testRobotsTxtResponse);
    CPPUNIT_TEST(testJsonResultResponse);
    CPPUNIT_TEST(testHandleIncomingMessage_RobotsTxt);
    CPPUNIT_TEST(testHandleIncomingMessage_Capabilities);

    CPPUNIT_TEST(testServerURL_ProxyPrefixNegativePort);
    CPPUNIT_TEST(testServerURL_ProxyPrefixValidPort);

    CPPUNIT_TEST_SUITE_END();

public:
    /// Provide a non-null FileRequestHandler so the browser/wopi file-server
    /// branch does not dereference a null pointer. An empty root yields an
    /// empty file hash, which is enough to route without serving real files.
    void setUp() override
    {
        if (!COOLWSD::FileRequestHandler)
        {
            COOLWSD::FileRequestHandler =
                std::make_unique<FileServerRequestHandler>(COOLWSD::FileServerRoot);
        }

        if (!COOLWSDServer::WebServerPoll)
        {
            COOLWSDServer::WebServerPoll = std::make_unique<TerminatingPoll>("websrv_poll");
        }
    }

    void tearDown() override
    {
        COOLWSDServer::WebServerPoll.reset();
        COOLWSD::FileRequestHandler.reset();
    }

private:
    void testGetContentType_Writer()
    {
        constexpr std::string_view testname = __func__;
        LOK_ASSERT_EQUAL_STR("application/vnd.oasis.opendocument.text",
                             ContentType::fromFileName("test.odt"));
        LOK_ASSERT_EQUAL_STR("application/vnd.oasis.opendocument.text-flat-xml",
                             ContentType::fromFileName("test.fodt"));
        LOK_ASSERT_EQUAL_STR("application/vnd.sun.xml.writer",
                             ContentType::fromFileName("test.sxw"));
    }

    void testGetContentType_Calc()
    {
        constexpr std::string_view testname = __func__;
        LOK_ASSERT_EQUAL_STR("application/vnd.oasis.opendocument.spreadsheet",
                             ContentType::fromFileName("test.ods"));
        LOK_ASSERT_EQUAL_STR("application/vnd.oasis.opendocument.spreadsheet-flat-xml",
                             ContentType::fromFileName("test.fods"));
        LOK_ASSERT_EQUAL_STR("application/vnd.sun.xml.calc",
                             ContentType::fromFileName("test.sxc"));
    }

    void testGetContentType_Impress()
    {
        constexpr std::string_view testname = __func__;
        LOK_ASSERT_EQUAL_STR("application/vnd.oasis.opendocument.presentation",
                             ContentType::fromFileName("test.odp"));
        LOK_ASSERT_EQUAL_STR("application/vnd.oasis.opendocument.presentation-flat-xml",
                             ContentType::fromFileName("test.fodp"));
        LOK_ASSERT_EQUAL_STR("application/vnd.sun.xml.impress",
                             ContentType::fromFileName("test.sxi"));
    }

    void testGetContentType_Draw()
    {
        constexpr std::string_view testname = __func__;
        LOK_ASSERT_EQUAL_STR("application/vnd.oasis.opendocument.graphics",
                             ContentType::fromFileName("test.odg"));
        LOK_ASSERT_EQUAL_STR("application/vnd.oasis.opendocument.graphics-flat-xml",
                             ContentType::fromFileName("test.fodg"));
    }

    void testGetContentType_MSOffice()
    {
        constexpr std::string_view testname = __func__;
        LOK_ASSERT_EQUAL_STR("application/msword",
                             ContentType::fromFileName("test.doc"));
        LOK_ASSERT_EQUAL_STR("application/msword",
                             ContentType::fromFileName("test.dot"));
        LOK_ASSERT_EQUAL_STR("application/vnd.ms-excel",
                             ContentType::fromFileName("test.xls"));
        LOK_ASSERT_EQUAL_STR("application/vnd.ms-powerpoint",
                             ContentType::fromFileName("test.ppt"));
    }

    void testGetContentType_OOXML()
    {
        constexpr std::string_view testname = __func__;
        LOK_ASSERT_EQUAL_STR(
            "application/vnd.openxmlformats-officedocument.wordprocessingml.document",
            ContentType::fromFileName("test.docx"));
        LOK_ASSERT_EQUAL_STR(
            "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet",
            ContentType::fromFileName("test.xlsx"));
        LOK_ASSERT_EQUAL_STR(
            "application/vnd.openxmlformats-officedocument.presentationml.presentation",
            ContentType::fromFileName("test.pptx"));
        LOK_ASSERT_EQUAL_STR("application/vnd.ms-excel.sheet.macroEnabled.12",
                             ContentType::fromFileName("test.xlsm"));
        LOK_ASSERT_EQUAL_STR("application/vnd.ms-excel.sheet.binary.macroEnabled.12",
                             ContentType::fromFileName("test.xlsb"));
    }

    void testGetContentType_Images()
    {
        constexpr std::string_view testname = __func__;
        LOK_ASSERT_EQUAL_STR("image/svg+xml", ContentType::fromFileName("drawing.svg"));
        LOK_ASSERT_EQUAL_STR("image/png", ContentType::fromFileName("photo.png"));
        LOK_ASSERT_EQUAL_STR("image/jpeg", ContentType::fromFileName("photo.jpeg"));
        LOK_ASSERT_EQUAL_STR("image/jpeg", ContentType::fromFileName("photo.jpg"));
        LOK_ASSERT_EQUAL_STR("image/bmp", ContentType::fromFileName("photo.bmp"));
        LOK_ASSERT_EQUAL_STR("image/gif", ContentType::fromFileName("photo.gif"));
        LOK_ASSERT_EQUAL_STR("image/tiff", ContentType::fromFileName("photo.tiff"));
    }

    void testGetContentType_Other()
    {
        constexpr std::string_view testname = __func__;
        LOK_ASSERT_EQUAL_STR("application/pdf", ContentType::fromFileName("document.pdf"));
        LOK_ASSERT_EQUAL_STR("text/rtf", ContentType::fromFileName("document.rtf"));
        LOK_ASSERT_EQUAL_STR("text/plain", ContentType::fromFileName("readme.txt"));
        LOK_ASSERT_EQUAL_STR("text/csv", ContentType::fromFileName("data.csv"));
        LOK_ASSERT_EQUAL_STR("text/tab-separated-values", ContentType::fromFileName("data.tsv"));
    }

    void testGetContentType_Unknown()
    {
        constexpr std::string_view testname = __func__;
        LOK_ASSERT_EQUAL_STR("application/octet-stream", ContentType::fromFileName("file.xyz"));
        LOK_ASSERT_EQUAL_STR("application/octet-stream", ContentType::fromFileName("noextension"));
        LOK_ASSERT_EQUAL_STR("application/octet-stream", ContentType::fromFileName(""));
    }

    void testGetContentType_PathWithDirs()
    {
        constexpr std::string_view testname = __func__;
        LOK_ASSERT_EQUAL_STR("application/vnd.oasis.opendocument.text",
                             ContentType::fromFileName("/path/to/test.odt"));
        LOK_ASSERT_EQUAL_STR(
            "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet",
            ContentType::fromFileName("/some/dir/budget.xlsx"));
    }

    void testGetContentType_CaseExtension()
    {
        constexpr std::string_view testname = __func__;
        LOK_ASSERT_EQUAL_STR("application/vnd.oasis.opendocument.text",
                             ContentType::fromFileName("test.ODT"));
    }

    void testIsSpreadsheet_Calc()
    {
        constexpr std::string_view testname = __func__;
        LOK_ASSERT(ContentType::isSpreadsheet("budget.ods"));
    }

    void testIsSpreadsheet_Excel()
    {
        constexpr std::string_view testname = __func__;
        LOK_ASSERT(ContentType::isSpreadsheet("budget.xls"));
    }

    void testIsSpreadsheet_OOXML()
    {
        constexpr std::string_view testname = __func__;
        LOK_ASSERT(ContentType::isSpreadsheet("budget.xlsx"));
    }

    void testIsSpreadsheet_NotSpreadsheet()
    {
        constexpr std::string_view testname = __func__;
        LOK_ASSERT(!ContentType::isSpreadsheet("document.odt"));
        LOK_ASSERT(!ContentType::isSpreadsheet("slides.pptx"));
        LOK_ASSERT(!ContentType::isSpreadsheet("drawing.odg"));
        LOK_ASSERT(!ContentType::isSpreadsheet("photo.png"));
        LOK_ASSERT(!ContentType::isSpreadsheet("unknown.xyz"));

        // xlsm and xlsb have different content types not in the isSpreadsheet check
        LOK_ASSERT(!ContentType::isSpreadsheet("macro.xlsm"));
    }

    /// Verify MockStreamSocket captures raw send() data.
    void testMockStreamSocket_Send()
    {
        constexpr std::string_view testname = __func__;
        auto socket = std::make_shared<MockStreamSocket>();

        LOK_ASSERT(socket->getOutput().empty());

        socket->send(std::string_view("hello"), false);
        LOK_ASSERT_EQUAL_STR("hello", socket->getOutput());

        socket->send(std::string_view(" world"), false);
        LOK_ASSERT_EQUAL_STR("hello world", socket->getOutput());

        socket->clearOutput();
        LOK_ASSERT(socket->getOutput().empty());
    }

    /// Verify MockStreamSocket captures http::Response output.
    void testMockStreamSocket_HttpResponse()
    {
        constexpr std::string_view testname = __func__;
        auto socket = std::make_shared<MockStreamSocket>();

        http::Response response(http::StatusCode::OK);
        response.setBody("test body", "text/plain");
        response.writeData(socket->getOutBuffer());

        const std::string output = socket->getOutput();
        LOK_ASSERT(output.find("HTTP/1.1 200") != std::string::npos);
        LOK_ASSERT(output.find("Content-Type: text/plain") != std::string::npos);
        LOK_ASSERT(output.find("test body") != std::string::npos);
    }

    /// Reproduce the robots.txt response pattern from handleRobotsTxtRequest.
    void testRobotsTxtResponse()
    {
        constexpr std::string_view testname = __func__;
        auto socket = std::make_shared<MockStreamSocket>();

        // Same pattern as handleStaticRequest in ClientRequestDispatcher.cpp
        const std::string body = "User-agent: *\nDisallow: /\n";
        http::Response response(http::StatusCode::OK);
        response.set("Last-Modified", "Sat, 29 Mar 2026 00:00:00 GMT");
        response.setContentLength(body.size());
        response.set("Content-Type", "text/plain");
        response.writeData(socket->getOutBuffer());
        socket->send(std::string_view(body), false);

        const std::string output = socket->getOutput();
        LOK_ASSERT(output.find("HTTP/1.1 200") != std::string::npos);
        LOK_ASSERT(output.find("Content-Type: text/plain") != std::string::npos);
        LOK_ASSERT(output.find("User-agent: *\nDisallow: /\n") != std::string::npos);

        // Verify Content-Length matches body
        const std::string clHeader = "Content-Length: " + std::to_string(body.size());
        LOK_ASSERT(output.find(clHeader) != std::string::npos);
    }

    /// Reproduce the JSON result pattern from sendResult in ClientRequestDispatcher.cpp.
    void testJsonResultResponse()
    {
        constexpr std::string_view testname = __func__;
        auto socket = std::make_shared<MockStreamSocket>();

        // Same pattern as sendResult() in ClientRequestDispatcher.cpp
        const std::string jsonBody = "{\"status\": \"Ok\"}\n";
        http::Response response(http::StatusCode::OK);
        response.setBody(std::string(jsonBody), "application/json");
        response.set("X-Content-Type-Options", "nosniff");
        response.writeData(socket->getOutBuffer());
        socket->send(std::string_view(jsonBody), false);

        const std::string output = socket->getOutput();
        LOK_ASSERT(output.find("HTTP/1.1 200") != std::string::npos);
        LOK_ASSERT(output.find("application/json") != std::string::npos);
        LOK_ASSERT(output.find("X-Content-Type-Options: nosniff") != std::string::npos);
        LOK_ASSERT(output.find(R"("status": "Ok")") != std::string::npos);
    }

    /// Helper: create a CRD wired to a MockStreamSocket, inject an HTTP request,
    /// and call handleIncomingMessage. Returns the raw HTTP response.
    std::string dispatchRequest(const std::string& httpRequest)
    {
        auto handler = std::make_shared<ClientRequestDispatcher>();
        auto socket = std::make_shared<MockStreamSocket>();
        socket->setHandler(handler);

        // Call through base — onConnect and handleIncomingMessage are private overrides.
        std::shared_ptr<ProtocolHandlerInterface> base = handler;
        base->onConnect(socket);

        // Inject the HTTP request into the socket's input buffer.
        auto& inBuf = socket->getInBuffer();
        inBuf.append(httpRequest.data(), httpRequest.size());

        SocketDisposition disposition(socket);
        base->handleIncomingMessage(disposition);

        return socket->getOutput();
    }

    void testHandleIncomingMessage_RobotsTxt()
    {
        constexpr std::string_view testname = __func__;
        const std::string response =
            dispatchRequest("GET /robots.txt HTTP/1.1\r\nHost: localhost\r\n\r\n");

        LOK_ASSERT(response.find("HTTP/1.1 200") != std::string::npos);
        LOK_ASSERT(response.find("User-agent: *") != std::string::npos);
        LOK_ASSERT(response.find("Disallow: /") != std::string::npos);
    }

    void testHandleIncomingMessage_Capabilities()
    {
        constexpr std::string_view testname = __func__;
        const std::string response =
            dispatchRequest("GET /hosting/capabilities HTTP/1.1\r\nHost: localhost\r\n\r\n");

        // /hosting/capabilities is returned asynchronously, so we can't test it
        // with MockStreamSocket. It used to work because Application::instance()
        // threw, and we returned an error.
        LOK_ASSERT(response.empty());
    }

    // Verify that a ProxyPrefix containing port -1 (produced by some reverse
    // proxies when the Host header omits the port for a default HTTPS
    // connection) does not appear in the URLs served to the browser.
    void testServerURL_ProxyPrefixNegativePort()
    {
        constexpr std::string_view testname = __func__;
        const std::string saved = COOLWSD::ServerName;
        COOLWSD::ServerName.clear();

        Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET,
                                       "/cool/ws?WOPISrc=http%3A%2F%2Fexample.com%2Fdoc",
                                       Poco::Net::HTTPMessage::HTTP_1_1);
        request.setHost("example.com");
        request.set("ProxyPrefix", "https://example.com:-1/wf");
        const RequestDetails details(request, "");
        const ServerURL url(details);

        LOK_ASSERT(url.getWebSocketUrl().find(":-1") == std::string::npos);
        LOK_ASSERT(url.getWebServerUrl().find(":-1") == std::string::npos);
        LOK_ASSERT_EQUAL(std::string("https://example.com"), url.getWebSocketUrl());
        LOK_ASSERT_EQUAL(std::string("https://example.com"), url.getWebServerUrl());

        COOLWSD::ServerName = saved;
    }

    // Verify that a ProxyPrefix with a valid explicit port is preserved.
    void testServerURL_ProxyPrefixValidPort()
    {
        constexpr std::string_view testname = __func__;
        const std::string saved = COOLWSD::ServerName;
        COOLWSD::ServerName.clear();

        Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET,
                                       "/cool/ws?WOPISrc=http%3A%2F%2Fexample.com%2Fdoc",
                                       Poco::Net::HTTPMessage::HTTP_1_1);
        request.setHost("example.com:8443");
        request.set("ProxyPrefix", "https://example.com:8443/wf");
        const RequestDetails details(request, "");
        const ServerURL url(details);

        LOK_ASSERT_EQUAL(std::string("https://example.com:8443"), url.getWebSocketUrl());
        LOK_ASSERT_EQUAL(std::string("https://example.com:8443"), url.getWebServerUrl());

        COOLWSD::ServerName = saved;
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(ClientRequestDispatcherTests);

/* vim:set shiftwidth=4 expandtab: */
