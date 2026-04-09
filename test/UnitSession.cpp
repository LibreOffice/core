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
 * Unit test for session-related functionality including XML/DOM parsing.
 */

#include <config.h>

#include <common/Unit.hpp>
#include <common/Util.hpp>
#include <net/HttpRequest.hpp>
#include <net/Socket.hpp>
#include <wsd/UserMessages.hpp>

#include <test/helpers.hpp>
#include <test/lokassert.hpp>

#include <Poco/DOM/DOMParser.h>
#include <Poco/DOM/Document.h>
#include <Poco/DOM/Node.h>
#include <Poco/DOM/NodeFilter.h>
#include <Poco/DOM/NodeIterator.h>
#include <Poco/SAX/InputSource.h>
#include <Poco/URI.h>

#include <memory>
#include <regex>
#include <string>
#include <vector>

using namespace std::literals;

namespace
{
int findInDOM(Poco::XML::Document* doc, const char* string, bool checkName,
              unsigned long nodeFilter = Poco::XML::NodeFilter::SHOW_ALL)
{
    int count = 0;
    Poco::XML::NodeIterator itCode(doc, nodeFilter);
    while (Poco::XML::Node* node = itCode.nextNode())
    {
        if (checkName)
        {
            if (node->nodeName() == string)
                count++;
        }
        else
        {
            if (node->getNodeValue().find(string) != std::string::npos)
                count++;
        }
    }
    return count;
}
} // namespace

/// Test suite that uses a HTTP session (and not just a socket) directly.
class UnitSession : public UnitWSD
{
    TestResult testBadRequest();
    TestResult testHandshake();
    TestResult testFilesOpenConnection();
    TestResult testFilesCloseConnection();
    TestResult testFileServer();
    TestResult testSlideShow();
    TestResult testSlideShowMultiDL();
    TestResult testGetMetrics();

public:
    UnitSession()
        : UnitWSD("UnitSession")
    {
    }

    void invokeWSDTest() override;
};

UnitBase::TestResult UnitSession::testBadRequest()
{
    setTestname(__func__);

    // Keep alive socket, avoid forced socket disconnect via dtor
    std::shared_ptr<TerminatingPoll> socketPoller = std::make_shared<TerminatingPoll>(testname);
    socketPoller->runOnClientThread();

    TST_LOG("Starting Test: " << testname);
    try
    {
        // Try to load a bogus url.
        const std::string documentURL = "/lol/file%3A%2F%2F%2Ffake.doc";
        std::shared_ptr<http::Session> session = http::Session::create(helpers::getTestServerURI());
        http::Request request(documentURL, http::Request::VERB_GET);
        request.setConnectionToken(http::Header::ConnectionToken::Upgrade);
        request.set("Upgrade", "websocket");
        request.set("Sec-WebSocket-Version", "13");
        request.set("Sec-WebSocket-Key", "");
        // request.setChunkedTransferEncoding(false);
        const std::shared_ptr<const http::Response> response =
            session->syncRequest(request, *socketPoller);
        // TST_LOG("Response: " << response->header().toString());
        LOK_ASSERT_EQUAL(http::StatusCode::BadRequest, response->statusCode());
        LOK_ASSERT_EQUAL(false, session->isConnected());
        LOK_ASSERT_EQUAL(http::Header::ConnectionToken::Close, response->header().getConnectionToken());
    }
    catch (const Poco::Exception& exc)
    {
        LOK_ASSERT_FAIL(exc.displayText());
    }
    return TestResult::Ok;
}

UnitBase::TestResult UnitSession::testHandshake()
{
    setTestname(__func__);
    TST_LOG("Starting Test: " << testname);

    std::string documentPath, documentURL;
    helpers::getDocumentPathAndURL("hello.odt", documentPath, documentURL, testname);

    // NOTE: Do not replace with wrappers. This has to be explicit.
    auto wsSession = http::WebSocketSession::create(helpers::getTestServerURI());
    http::Request req(documentURL);
    wsSession->asyncRequest(req, socketPoll());

    wsSession->sendMessage("load url=" + documentURL);

    auto assertMessage = [&wsSession, this](const std::string expectedId)
    {
        wsSession->poll(
            [this, expectedId](const std::vector<char>& message)
            {
                const std::string msg(std::string(message.begin(), message.end()));
                if (!msg.starts_with("error:"))
                {
                    LOK_ASSERT_EQUAL(true, COOLProtocol::matchPrefix("progress:", msg));
                    LOK_ASSERT(helpers::getProgressWithIdValue(msg, expectedId));
                }
                else
                {
                    // check error message
                    LOK_ASSERT_EQUAL(std::string(SERVICE_UNAVAILABLE_INTERNAL_ERROR), msg);

                    // close frame message
                    //TODO: check that the socket is closed.
                }

                return true;
            },
            10s, testname);
    };

    assertMessage("find");
    assertMessage("connect");
    assertMessage("ready");

    return TestResult::Ok;
}

UnitBase::TestResult UnitSession::testFilesOpenConnection()
{
    setTestname(__func__);

    const std::vector<std::string> documentURLs = {
        "/", // <
        "/favicon.ico",
        "/hosting/discovery",
        "/hosting/capabilities",
        "/robots.txt",
        "/cool/signature",
        // "/cool/media",
        // "/cool/clipboard",
        // "/cool/file:\/\/.../ws",
    };
    // Reused http session, keep-alive
    std::shared_ptr<http::Session> session = http::Session::create(helpers::getTestServerURI());

    // Keep alive socket, avoid forced socket disconnect via dtor
    std::shared_ptr<TerminatingPoll> socketPoller = std::make_shared<TerminatingPoll>(testname);
    socketPoller->runOnClientThread();

    int docIdx = 0;
    try
    {
        for (const std::string& documentURL : documentURLs)
        {
            if (docIdx > 0)
            {
                LOK_ASSERT_EQUAL(true, session->isConnected());
            }
            TST_LOG("Test " << docIdx << "]: `" << documentURL << "`");
            http::Request request(documentURL, http::Request::VERB_GET);
            const std::shared_ptr<const http::Response> response =
                session->syncRequest(request, *socketPoller);
            TST_LOG("Response: " << response->header().toString());
            TST_LOG("Response size " << docIdx << "]: `" << documentURL
                                     << "`: " << response->header().getContentLength());
            LOK_ASSERT_EQUAL(http::StatusCode::OK, response->statusCode());
            LOK_ASSERT_EQUAL(true, session->isConnected());
            LOK_ASSERT(http::Header::ConnectionToken::None ==
                       response->header().getConnectionToken());
            LOK_ASSERT(0 < response->header().getContentLength());
            ++docIdx;
        }
    }
    catch (const Poco::Exception& exc)
    {
        LOK_ASSERT_FAIL(exc.displayText());
    }
    return TestResult::Ok;
}

UnitBase::TestResult UnitSession::testFilesCloseConnection()
{
    setTestname(__func__);

    const std::vector<std::string> documentURLs = {
        "/", // <
        "/favicon.ico",
        "/hosting/discovery",
        "/hosting/capabilities",
        "/robots.txt",
        "/cool/signature",
        // "/cool/media",
        // "/cool/clipboard",
        // "/cool/file:\/\/.../ws",
    };
    std::shared_ptr<TerminatingPoll> socketPoller = std::make_shared<TerminatingPoll>(testname);
    socketPoller->runOnClientThread();

    int docIdx = 0;
    try
    {
        for (const std::string& documentURL : documentURLs)
        {
            TST_LOG("Test " << docIdx << "]: `" << documentURL << "`");
            http::Request request(documentURL, http::Request::VERB_GET);
            request.setConnectionToken(http::Header::ConnectionToken::Close);
            std::shared_ptr<http::Session> session = http::Session::create(helpers::getTestServerURI());
            const std::shared_ptr<const http::Response> response =
                session->syncRequest(request, *socketPoller);
            TST_LOG("Response: " << response->header().toString());
            TST_LOG("Response size " << docIdx << "]: `" << documentURL
                                     << "`: " << response->header().getContentLength());
            LOK_ASSERT_EQUAL(http::StatusCode::OK, response->statusCode());
            LOK_ASSERT_EQUAL(false, session->isConnected());
            LOK_ASSERT(http::Header::ConnectionToken::Close ==
                       response->header().getConnectionToken());
            LOK_ASSERT(0 < response->header().getContentLength());
            ++docIdx;
        }
    }
    catch (const Poco::Exception& exc)
    {
        LOK_ASSERT_FAIL(exc.displayText());
    }
    return TestResult::Ok;
}

UnitBase::TestResult UnitSession::testFileServer()
{
    setTestname(__func__);

    const std::vector<std::string> documentURLs = {
        // "/browser/welcome/welcome.html", // < '/browser/welcome/` not in FileServer's FileHash(!)
        "/browser/dist/cool.html", // < special preprocessFile(..)
        "/browser/dist/cool.css", // < fall-through general case
        // "/browser/dist/admin/admin.html", // < Requires Admin
    };
    // Reused http session, keep-alive
    std::shared_ptr<http::Session> session = http::Session::create(helpers::getTestServerURI());

    // Keep alive socket, avoid forced socket disconnect via dtor
    std::shared_ptr<TerminatingPoll> socketPoller = std::make_shared<TerminatingPoll>(testname);
    socketPoller->runOnClientThread();

    int docIdx = 0;
    try
    {
        for (const std::string& documentURL : documentURLs)
        {
            if (docIdx > 0)
            {
                LOK_ASSERT_EQUAL(true, session->isConnected());
            }
            TST_LOG("Test " << docIdx << "]: `" << documentURL << "`");
            http::Request request(documentURL, http::Request::VERB_GET);
            const std::shared_ptr<const http::Response> response =
                session->syncRequest(request, *socketPoller);
            TST_LOG("Response: " << response->header().toString());
            TST_LOG("Response size " << docIdx << "]: " << documentURL << ": "
                                     << response->header().getContentLength());
            LOK_ASSERT_EQUAL(http::StatusCode::OK, response->statusCode());
            LOK_ASSERT_EQUAL(true, session->isConnected());
            LOK_ASSERT(http::Header::ConnectionToken::None ==
                       response->header().getConnectionToken());
            LOK_ASSERT(0 < response->header().getContentLength());
            ++docIdx;
        }
    }
    catch (const Poco::Exception& exc)
    {
        LOK_ASSERT_FAIL(exc.displayText());
    }
    return TestResult::Ok;
}

UnitBase::TestResult UnitSession::testSlideShow()
{
    setTestname(__func__);
    TST_LOG("Starting Test: " << testname);
    try
    {
        // Load a document
        std::string documentPath, documentURL;
        std::string docResponse;
        helpers::getDocumentPathAndURL("setclientpart.odp", documentPath, documentURL, testname);

        std::shared_ptr<http::WebSocketSession> wsdSession = helpers::loadDocAndGetSession(
            socketPoll(), Poco::URI(helpers::getTestServerURI()), documentURL, testname);

        // request slide show
        helpers::sendTextFrame(
            wsdSession, "downloadas name=slideshow.svg id=slideshow format=svg options=", testname);
        docResponse = helpers::getResponseString(wsdSession, "downloadas:", testname);
        LOK_ASSERT_MESSAGE("did not receive a downloadas: message as expected",
                           !docResponse.empty());

        StringVector tokens(StringVector::tokenize(docResponse.substr(11), ' '));
        // "downloadas: downloadId= port= id=slideshow"
        const std::string downloadId = tokens[0].substr(std::string_view("downloadId=").size());
        const int port = NumUtil::stoi(tokens[1].substr(std::string_view("port=").size()));
        const std::string id = tokens[2].substr(std::string_view("id=").size());
        LOK_ASSERT(!downloadId.empty());
        LOK_ASSERT_EQUAL(static_cast<int>(Poco::URI(helpers::getTestServerURI()).getPort()), port);
        LOK_ASSERT_EQUAL_STR("slideshow", id);

        std::string encodedDoc;
        Poco::URI::encode(documentPath, ":/?", encodedDoc);
        const std::string ignoredSuffix = "%3FWOPISRC=madness"; // cf. iPhone.
        const std::string path =
            "/cool/" + encodedDoc + "/download/" + downloadId + '/' + ignoredSuffix;

        // Keep alive socket, avoid forced socket disconnect via dtor
        std::shared_ptr<TerminatingPoll> dlSocketPoller = std::make_shared<TerminatingPoll>(testname + "-dl");
        dlSocketPoller->runOnClientThread();
        std::shared_ptr<http::Session> dlSession =
            http::Session::create(helpers::getTestServerURI());
        http::Request requestSVG(path, http::Request::VERB_GET);
        TST_LOG("Requesting SVG from " << path);
        const std::shared_ptr<const http::Response> responseSVG =
            dlSession->syncRequest(requestSVG, *dlSocketPoller);
        // TST_LOG("Response (SVG): " << responseSVG->header().toString());
        LOK_ASSERT_EQUAL(http::StatusCode::OK, responseSVG->statusCode());
        LOK_ASSERT_EQUAL(true, dlSession->isConnected());
        LOK_ASSERT(http::Header::ConnectionToken::None ==
                   responseSVG->header().getConnectionToken());

        LOK_ASSERT_EQUAL_STR("image/svg+xml", responseSVG->header().getContentType());
        LOK_ASSERT(0 < responseSVG->header().getContentLength());
        TST_LOG("SVG file size: " << responseSVG->header().getContentLength());

        //        std::ofstream ofs("/tmp/slide.svg");
        //        Poco::StreamCopier::copyStream(rs, ofs);
        //        ofs.close();

        // Asserting on the size of the stream is really unhelpful;
        // lets checkout the contents instead ...
        // (TODO: Use an async dlSession request w/ async parsing chunks?)
        Poco::XML::DOMParser parser;
        // std::istream& rs = session->receiveResponse(responseSVG);
        // Poco::XML::InputSource svgSrc(rs);
        // Poco::AutoPtr<Poco::XML::Document> doc = parser.parse(&svgSrc);
        Poco::AutoPtr<Poco::XML::Document> doc = parser.parseString(responseSVG->getBody());

        // Do we have our automation / scripting
        LOK_ASSERT(
            findInDOM(doc, "jessyinkstart", false, Poco::XML::NodeFilter::SHOW_CDATA_SECTION));
        LOK_ASSERT(findInDOM(doc, "jessyinkend", false, Poco::XML::NodeFilter::SHOW_CDATA_SECTION));
        LOK_ASSERT(
            findInDOM(doc, "libreofficestart", false, Poco::XML::NodeFilter::SHOW_CDATA_SECTION));
        LOK_ASSERT(
            findInDOM(doc, "libreofficeend", false, Poco::XML::NodeFilter::SHOW_CDATA_SECTION));

        // Do we have plausible content ?
        int countText = findInDOM(doc, "text", true, Poco::XML::NodeFilter::SHOW_ELEMENT);
        LOK_ASSERT_EQUAL(countText, 93);
    }
    catch (const Poco::Exception& exc)
    {
        LOK_ASSERT_FAIL(exc.displayText());
    }
    return TestResult::Ok;
}

/// Load a document on server and download it multiple to this client using 2 reused channels:
/// - wsd-session for commands (websocket)
/// - http-session to download it, as well as to load the favicon
UnitBase::TestResult UnitSession::testSlideShowMultiDL()
{
    setTestname(__func__);
    TST_LOG("Starting Test: " << testname);
    const int dlCount = 10;
    int dlIter = -1;
    try
    {
        std::string documentPath, documentURL;
        std::string response;
        helpers::getDocumentPathAndURL("setclientpart.odp", documentPath, documentURL, testname);

        // Reused http wsd session to send commands to server
        std::shared_ptr<http::WebSocketSession> wsdSession = helpers::loadDocAndGetSession(
            socketPoll(), Poco::URI(helpers::getTestServerURI()), documentURL, testname);

        // Keep alive socket, avoid forced socket disconnect via dtor
        std::shared_ptr<TerminatingPoll> dlSocketPoller = std::make_shared<TerminatingPoll>(testname + "-dl");
        dlSocketPoller->runOnClientThread();

        // Reused http download-session for document and favicon download from server
        std::shared_ptr<http::Session> dlSession =
            http::Session::create(helpers::getTestServerURI());
        // dlSession->setKeepAlive(true);

        for (dlIter = 0; dlIter < dlCount; ++dlIter)
        {
            // Still connected on sub-sequent commands and downloads?
            if (dlIter > 0)
            {
                LOK_ASSERT_EQUAL(true, wsdSession->isConnected());
                LOK_ASSERT_EQUAL(true, dlSession->isConnected());
            }

            // download favicon
            {
                const std::string path = "/favicon.ico";
                http::Request requestICO(path, http::Request::VERB_GET);
                TST_LOG("Favicon requesting from " << path);
                std::shared_ptr<const http::Response> responseICO = dlSession->syncDownload(requestICO, "/tmp/favicon.ico", *dlSocketPoller);
                // TST_LOG("Response (ICO): " << responseICO->header().toString());
                LOK_ASSERT_EQUAL(http::StatusCode::OK, responseICO->statusCode());
                LOK_ASSERT_EQUAL(true, dlSession->isConnected());
                LOK_ASSERT(http::Header::ConnectionToken::None ==
                           responseICO->header().getConnectionToken());

                LOK_ASSERT_EQUAL_STR("image/vnd.microsoft.icon",
                                     responseICO->header().getContentType());
                LOK_ASSERT(0 < responseICO->header().getContentLength());
                TST_LOG("Favicon file size: " << responseICO->header().getContentLength());
            }
            // request downloading document as SVG
            std::string id_req = "slideshow" + std::to_string(dlIter);
            TST_LOG(testname << ": Download Request: " << id_req << ": count " << dlIter << "/"
                             << dlCount);
            helpers::sendTextFrame(wsdSession,
                                   "downloadas name=" + id_req + ".svg id=" + id_req +
                                       " format=svg options=",
                                   testname);
            response = helpers::getResponseString(wsdSession, "downloadas:", testname);
            LOK_ASSERT_MESSAGE("did not receive a downloadas: message as expected",
                               !response.empty());

            StringVector tokens(StringVector::tokenize(response.substr(11), ' '));
            // "downloadas: downloadId= port= id=slideshow"
            const std::string downloadId = tokens[0].substr(std::string_view("downloadId=").size());
            const int port =
                NumUtil::stoi(std::string_view(tokens[1]).substr(std::string_view("port=").size()));
            const std::string id_has = tokens[2].substr(std::string_view("id=").size());
            LOK_ASSERT(!downloadId.empty());
            LOK_ASSERT_EQUAL(static_cast<int>(Poco::URI(helpers::getTestServerURI()).getPort()),
                             port);
            LOK_ASSERT_EQUAL(id_req, id_has);
            TST_LOG(testname << ": Download Response: " << id_has << ": count " << dlIter << "/"
                             << dlCount << ": " << downloadId);
            {
                std::string encodedDoc;
                Poco::URI::encode(documentPath, ":/?", encodedDoc);
                const std::string ignoredSuffix = "%3FWOPISRC=madness"; // cf. iPhone.
                const std::string path =
                    "/cool/" + encodedDoc + "/download/" + downloadId + '/' + ignoredSuffix;
                http::Request requestSVG(path, http::Request::VERB_GET);
                TST_LOG("Requesting SVG from " << path);
                const std::shared_ptr<const http::Response> responseSVG = dlSession->syncRequest(requestSVG, *dlSocketPoller);
                // TST_LOG("Response (SVG): " << responseSVG->header().toString());
                LOK_ASSERT_EQUAL(http::StatusCode::OK, responseSVG->statusCode());
                LOK_ASSERT_EQUAL(true, dlSession->isConnected());
                LOK_ASSERT(http::Header::ConnectionToken::None ==
                           responseSVG->header().getConnectionToken());

                LOK_ASSERT_EQUAL_STR("image/svg+xml", responseSVG->header().getContentType());
                LOK_ASSERT(0 < responseSVG->header().getContentLength());
                TST_LOG("SVG file size: " << responseSVG->header().getContentLength());

                //        std::ofstream ofs("/tmp/slide.svg");
                //        Poco::StreamCopier::copyStream(rs, ofs);
                //        ofs.close();

                // Asserting on the size of the stream is really unhelpful;
                // lets checkout the contents instead ...
                // (TODO: Use an async dlSession request w/ async parsing chunks?)
                Poco::XML::DOMParser parser;
                // std::istream& rs = session->receiveResponse(responseSVG);
                // Poco::XML::InputSource svgSrc(rs);
                // Poco::AutoPtr<Poco::XML::Document> doc = parser.parse(&svgSrc);
                Poco::AutoPtr<Poco::XML::Document> doc = parser.parseString(responseSVG->getBody());

                // Do we have our automation / scripting
                LOK_ASSERT(findInDOM(doc, "jessyinkstart", false,
                                     Poco::XML::NodeFilter::SHOW_CDATA_SECTION));
                LOK_ASSERT(findInDOM(doc, "jessyinkend", false,
                                     Poco::XML::NodeFilter::SHOW_CDATA_SECTION));
                LOK_ASSERT(findInDOM(doc, "libreofficestart", false,
                                     Poco::XML::NodeFilter::SHOW_CDATA_SECTION));
                LOK_ASSERT(findInDOM(doc, "libreofficeend", false,
                                     Poco::XML::NodeFilter::SHOW_CDATA_SECTION));

                // Do we have plausible content ?
                int countText = findInDOM(doc, "text", true, Poco::XML::NodeFilter::SHOW_ELEMENT);
                LOK_ASSERT_EQUAL(countText, 93);
            }
        }
    }
    catch (const Poco::Exception& exc)
    {
        LOK_ASSERT_FAIL(exc.displayText());
    }
    return TestResult::Ok;
}

UnitBase::TestResult UnitSession::testGetMetrics()
{
    setTestname(__func__);

    // Reused http session, keep-alive
    std::shared_ptr<http::Session> session = http::Session::create(helpers::getTestServerURI());

    // Keep alive socket, avoid forced socket disconnect via dtor
    std::shared_ptr<TerminatingPoll> socketPoller = std::make_shared<TerminatingPoll>(testname);
    socketPoller->runOnClientThread();

    std::vector<std::string> check_exists = {"document_all_views_all_count_total", "document_all_views_all_count_average", "document_all_views_all_count_min", "document_all_views_all_count_max"};

    TST_LOG("Test: " << testname);
    try
    {
        std::string documentURL = "/cool/getMetrics";
        // getMetrics is not keepAlive
        LOK_ASSERT_EQUAL(false, session->isConnected());

        http::Request request(documentURL);
        request.setBasicAuth("admin", "admin");
        LOK_ASSERT_EQUAL_STR("admin", request.getBasicAuth().first);
        LOK_ASSERT_EQUAL_STR("admin", request.getBasicAuth().second);

        const std::shared_ptr<const http::Response> response =
            session->syncRequest(request, *socketPoller);
        LOK_ASSERT_EQUAL(http::Response::State::Complete, response->state());
        LOK_ASSERT(response->header().hasContentLength());
        LOK_ASSERT(0 < response->header().getContentLength());
        LOK_ASSERT_EQUAL(http::StatusCode::OK, response->statusCode());
        LOK_ASSERT_EQUAL(http::Header::ConnectionToken::Close, response->header().getConnectionToken());

        // check metrics format and a few key values
        auto body = std::istringstream(response->getBody());
        std::string line;

        // line examples:
        // coolwsd_count 1
        // doc_info{host=\"\",key=\"%2Ftmp%2FtestHandshake6cb43aac_hello.odt\",filename=\"testHandshake6cb43aac_hello.odt\",pid=\"2267723\"} 1
        const std::regex line_regex(R"(([\w_]+(\{([\w_]+="[\w_%\.]*",?)+\})?) (\d+(\.\d+)?))");
        std::smatch match;
        int line_count = 0;
        while (std::getline(body, line)) {
            if (line.empty()) {
                continue;
            }

            ++line_count;
            auto found = std::regex_match(line, match, line_regex);
            LOK_ASSERT(found);

            if (check_exists.empty()) {
                continue;
            }
            auto it = std::find(check_exists.begin(), check_exists.end(), match[1]);
            if (it != check_exists.end()) {
                check_exists.erase(it);
            }
        }
        LOK_ASSERT(line_count > 10);
        LOK_ASSERT(check_exists.empty());
    }
    catch (const Poco::Exception& exc)
    {
        LOK_ASSERT_FAIL(exc.displayText());
    }
    return TestResult::Ok;
}

void UnitSession::invokeWSDTest()
{
    UnitBase::TestResult result = testBadRequest();
    if (result != TestResult::Ok)
        exitTest(result);

    result = testHandshake();
    if (result != TestResult::Ok)
        exitTest(result);

    result = testFilesOpenConnection();
    if (result != TestResult::Ok)
        exitTest(result);

    result = testFilesCloseConnection();
    if (result != TestResult::Ok)
        exitTest(result);

    result = testFileServer();
    if (result != TestResult::Ok)
    {
        exitTest(result);
    }

    result = testSlideShow();
    if (result != TestResult::Ok)
        exitTest(result);

    result = testSlideShowMultiDL();
    if (result != TestResult::Ok)
        exitTest(result);

    result = testGetMetrics();
    if (result != TestResult::Ok)
        exitTest(result);

    exitTest(TestResult::Ok);
}

UnitBase* unit_create_wsd(void) { return new UnitSession(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
