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

// Test various copy/paste pieces ...

#include <config.h>

#include <HttpRequest.hpp>
#include <Unit.hpp>
#include <UnitHTTP.hpp>
#include <WebSocketSession.hpp>
#include <common/Clipboard.hpp>
#include <helpers.hpp>
#include <lokassert.hpp>
#include <test.hpp>
#include <wsd/COOLWSD.hpp>
#include <wsd/ClientSession.hpp>

#include <Poco/URI.h>
#include <Poco/Util/LayeredConfiguration.h>

#include <sstream>

using namespace std::literals;

// Inside the WSD process
class UnitCopyPaste : public UnitWSD
{
    STATE_ENUM(Phase, RunTest, WaitDocClose, PostCloseTest, Done) _phase;

    std::string _clipURI;
    std::string _clipURI2;

public:
    UnitCopyPaste()
        : UnitWSD("UnitCopyPaste")
        , _phase(Phase::RunTest)
    {
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        UnitWSD::configure(config);
        // force HTTPS - to test harder
        config.setBool("ssl.enable", true);
    }

    std::string getRawClipboard(const std::string &clipURIstr)
    {
        auto httpSession = http::Session::create(clipURIstr);
        std::shared_ptr<const http::Response> httpResponse =
            httpSession->syncRequest(http::Request(Poco::URI(clipURIstr).getPathAndQuery()));
        return httpResponse->getBody();
    }

    std::shared_ptr<ClipboardData> getClipboard(const std::string& clipURIstr,
                                                http::StatusCode expected)
    {
        TST_LOG("getClipboard: connect to " << clipURIstr);
        const Poco::URI clipURI(clipURIstr);
        const std::string clipPathAndQuery = clipURI.getPathAndQuery();

        auto httpSession = http::Session::create(clipURIstr);
        std::shared_ptr<const http::Response> httpResponse =
            httpSession->syncRequest(http::Request(clipPathAndQuery));

        // Note that this is expected for both living and closed documents.
        // This failed when either case didn't add the custom header.
        LOK_ASSERT_EQUAL_STR("true", httpResponse->get("X-COOL-Clipboard"));

        // We should mark clipboard responses as non-cacheable.
        LOK_ASSERT_EQUAL_STR("no-cache", httpResponse->get("Cache-Control"));

        TST_LOG("getClipboard: sent request: " << clipPathAndQuery);

        try {
            TST_LOG("getClipboard: HTTP get request returned: "
                    << httpResponse->statusLine().statusCode());

            if (httpResponse->statusLine().statusCode() != expected)
            {
                LOK_ASSERT_EQUAL_MESSAGE("clipboard status mismatches", expected,
                                         httpResponse->statusLine().statusCode());
                exitTest(TestResult::Failed);
                return std::shared_ptr<ClipboardData>();
            }

            LOK_ASSERT_EQUAL_MESSAGE("getClipboard: clipboard content-type mismatches expected",
                                     std::string("application/octet-stream"),
                                     httpResponse->header().getContentType());

            std::string body = httpResponse->getBody();
            removeSessionClipboardMeta(body);
            std::istringstream responseStream(body);
            auto clipboard = std::make_shared<ClipboardData>();
            if (!clipboard->read(responseStream))
                throw ParseError("error during reading the stream");
            std::ostringstream oss(Util::makeDumpStateStream());
            clipboard->dumpState(oss);

            TST_LOG("getClipboard: got response. State:\n" << oss.str());
            return clipboard;
        } catch (Poco::Exception &e) {
            TST_LOG("Poco exception: " << e.message());
            exitTest(TestResult::Failed);
            return std::shared_ptr<ClipboardData>();
        }
    }

    bool assertClipboard(const std::shared_ptr<ClipboardData> &clipboard,
                         const std::string &mimeType, const std::string &content)
    {
        std::string value;

        // allow empty clipboards
        if (clipboard && mimeType.empty() && content.empty())
            return true;

        if (!clipboard || !clipboard->findType(mimeType, value))
        {
            TST_LOG("Error: missing clipboard or missing clipboard mime type '" << mimeType
                                                                                << '\'');
            LOK_ASSERT_FAIL("Missing clipboard mime type");
            exitTest(TestResult::Failed);
            return false;
        }
        else if (value != content)
        {
            TST_LOG("Error: clipboard content mismatch "
                    << value.length() << " bytes vs. " << content.length() << " bytes. Clipboard:\n"
                    << HexUtil::dumpHex(value) << "Expected:\n"
                    << HexUtil::dumpHex(content));
            LOK_ASSERT_EQUAL_MESSAGE("Clipboard content mismatch", content, value);
            exitTest(TestResult::Failed);
            return false;
        }

        return true;
    }

    bool fetchClipboardAssert(const std::string& clipURI, const std::string& mimeType,
                              const std::string& content,
                              http::StatusCode expected = http::StatusCode::OK)
    {
        try
        {
            std::shared_ptr<ClipboardData> clipboard = getClipboard(clipURI, expected);
            return assertClipboard(clipboard, mimeType, content);
        }
        catch (const ParseError& err)
        {
            TST_LOG("Error fetching clipboard: parse error: " << err.toString());
            exitTest(TestResult::Failed);
            return false;
        }
        catch (const std::exception& ex)
        {
            TST_LOG("Error fetching clipboard: " << ex.what());
            exitTest(TestResult::Failed);
            return false;
        }
        catch (...)
        {
            TST_LOG("Error fetching clipboard: unknown exception during read / parse");
            exitTest(TestResult::Failed);
            return false;
        }

        return true;
    }

    bool setClipboard(const std::string &clipURIstr, const std::string &rawData,
                      http::StatusCode expected)
    {
        TST_LOG("connect to " << clipURIstr);
        Poco::URI clipURI(clipURIstr);

        auto httpSession = http::Session::create(clipURIstr);
        http::Request request(clipURI.getPathAndQuery(), http::Request::VERB_POST);
        helpers::MultipartFormBody form;
        form.addField("format", "txt");
        form.addStringPart("data", rawData, "application/octet-stream", "clipboard");
        form.applyTo(request);
        const auto response = httpSession->syncRequest(request);

        if (!response || response->state() == http::Response::State::Timeout)
        {
            TST_LOG("Error: No response from setting clipboard.");
            exitTest(TestResult::Failed);
            return false;
        }

        if (response->statusCode() != expected)
        {
            TST_LOG("Error: response for clipboard " << response->statusCode() << " != expected "
                                                     << expected);
            exitTest(TestResult::Failed);
            return false;
        }

        return true;
    }

    std::shared_ptr<ClientSession> getChildSession(size_t session)
    {
        std::shared_ptr<DocumentBroker> broker;
        std::shared_ptr<ClientSession> clientSession;

        std::vector<std::shared_ptr<DocumentBroker>> brokers = COOLWSD::getBrokersTestOnly();
        assert(brokers.size() > 0);
        broker = brokers[0];
        auto sessions = broker->getSessionsTestOnlyUnsafe();
        assert(sessions.size() > 0 && session < sessions.size());
        return sessions[session];
    }

    std::string getSessionClipboardURI(size_t session)
    {
            std::shared_ptr<ClientSession> clientSession = getChildSession(session);

            std::string tag = clientSession->getClipboardURI(false); // nominally thread unsafe
            TST_LOG("Got tag '" << tag << "' for session " << session);
            return tag;
    }

    void removeSessionClipboardMeta(std::string& payload)
    {
        if (COOLWSD::getBrokersTestOnly().empty())
        {
            return;
        }

        // The removal doesn't depend on the clipboard URL, so just ask the first session to do the
        // removal for us.
        std::shared_ptr<ClientSession> clientSession = getChildSession(0);

        std::istringstream ifs(payload);
        std::ostringstream ofs;
        clientSession->preProcessSetClipboardPayload(ifs, ofs);
        payload = ofs.str();
    }

    std::string buildClipboardText(const std::string &text)
    {
        std::stringstream clipData;
        clipData << "text/plain;charset=utf-8\n"
                 << std::hex << text.length() << '\n'
                 << text << '\n';
        return clipData.str();
    }

    void onDocBrokerDestroy(const std::string& docKey) override
    {
        TST_LOG("Destroyed dockey [" << docKey << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitDocClose);

        TRANSITION_STATE(_phase, Phase::PostCloseTest);
    }

    void runTest()
    {
        // NOTE: This code has multiple race-conditions!
        // The main one is that the fetching of clipboard
        // data (via fetchClipboardAssert) is done via
        // independent HTTP GET requests that have nothing
        // whatever with the long-lived WebSocket that
        // processes all the client commands below.
        // This suffers a very interesting race, which is
        // that the getClipboard command may end up
        // anywhere within the Kit queue, even before the
        // other commands preceding it in this test!
        // As an example: in the 'Inject content' case we
        // send .uno:Deselect, paste, .uno:SelectAll,
        // .uno:Copy (all through the WebSocket), and an
        // HTTP GET for the getClipboard. But, in DocBroker,
        // the WebSocket commands might be interleaved with
        // the getClipboard, such that the Kit might receive
        // .uno:Deselect, paste, .uno:SelectAll, getClipboard,
        // .uno:Copy! This has been observed in practice.
        //
        // There are other race-conditions, too.
        // For example, even if Kit gets these commands in
        // the correct order (i.e. getClipboard last, after
        // .uno:Copy), getClipboard is executed immediately
        // while all the .uno commands are queued for async
        // execution. This means that when getClipboard is
        // executed in Core, the .uno:Copy command might not
        // have yet been executed and, therefore, will not
        // contain the expected contents, failing the test.
        //
        // To combat these races, we drain the events coming
        // from Core before we read the clipboard through
        // getClipboard. This way we are reasonably in
        // control of the state. Notice that the client
        // code will suffer these races, if the getClipboard
        // HTTP GET is done "too soon" after a copy, but
        // that is unlikely in practice because the URL for
        // getClipboard is generated as a link to the user,
        // which is clicked to download the clipboard contents.

        // Load a doc with the cursor saved at a top row.
        // NOTE: This load a spreadsheet, not a writer doc!
        std::string documentPath, documentURL;
        helpers::getDocumentPathAndURL("empty.ods", documentPath, documentURL, testname);

        std::shared_ptr<http::WebSocketSession> socket = helpers::loadDocAndGetSession(
            socketPoll(), Poco::URI(helpers::getTestServerURI()), documentURL, testname);

        _clipURI = getSessionClipboardURI(0);

        TST_LOG("Fetch empty clipboard content after loading");
        if (!fetchClipboardAssert(_clipURI, "", ""))
            return;

        // Check existing content
        TST_LOG("Fetch pristine content from the document");
        helpers::sendTextFrame(socket, "uno .uno:SelectAll", testname);
        helpers::sendAndDrain(socket, testname, "uno .uno:Copy", "statechanged:");
        const std::string oneColumn = "2\n3\n5\n";
        if (!fetchClipboardAssert(_clipURI, "text/plain;charset=utf-8", oneColumn))
            return;

        TST_LOG("Open second connection");
        std::shared_ptr<http::WebSocketSession> socket2 = helpers::loadDocAndGetSession(
            socketPoll(), Poco::URI(helpers::getTestServerURI()), documentURL, testname);
        _clipURI2 = getSessionClipboardURI(1);

        TST_LOG("Check no clipboard content on second view");
        if (!fetchClipboardAssert(_clipURI2, "", ""))
            return;

        TST_LOG("Inject content through first view");
        helpers::sendTextFrame(socket, "uno .uno:Deselect", testname);
        const std::string text = "This is some content?&*/\\!!";
        helpers::sendTextFrame(socket, "paste mimetype=text/plain;charset=utf-8\n" + text, testname);
        helpers::sendTextFrame(socket, "uno .uno:SelectAll", testname);
        helpers::sendAndDrain(socket, testname, "uno .uno:Copy", "statechanged:");

        const std::string existing = "2\t\n3\t\n5\t";
        if (!fetchClipboardAssert(_clipURI, "text/plain;charset=utf-8", existing + text + '\n'))
            return;

        TST_LOG("re-check no clipboard content");
        if (!fetchClipboardAssert(_clipURI2, "", ""))
            return;

        TST_LOG("Push new clipboard content");
        const std::string newcontent = "1234567890";
        helpers::sendAndWait(socket, testname, "uno .uno:Deselect", "statechanged:");
        if (!setClipboard(_clipURI, buildClipboardText(newcontent), http::StatusCode::OK))
            return;
        helpers::sendAndWait(socket, testname, "uno .uno:Paste", "statechanged:");

        if (!fetchClipboardAssert(_clipURI, "text/plain;charset=utf-8", newcontent))
            return;

        TST_LOG("Check the result.");
        helpers::sendTextFrame(socket, "uno .uno:SelectAll", testname);
        helpers::sendAndDrain(socket, testname, "uno .uno:Copy", "statechanged:");
        if (!fetchClipboardAssert(_clipURI, "text/plain;charset=utf-8",
                                  existing + newcontent + '\n'))
            return;

        // Test setting HTML clipboard:
        std::string html("<!DOCTYPE html><html><body>myword</body></html>");
        // Intentionally no buildClipboardText() here, just raw HTML.
        if (!setClipboard(_clipURI, html, http::StatusCode::OK))
            return;
        // This failed with: ERROR: Forced failure: Missing clipboard mime type, because we tried to
        // parse HTML when we expected a list of mimetype-size-bytes entries.
        if (!fetchClipboardAssert(_clipURI, "text/html", html))
            return;

        // Setup state that will be also asserted in postCloseTest():
        TST_LOG("Setup clipboards:");
        if (!setClipboard(_clipURI2, buildClipboardText("kippers"), http::StatusCode::OK))
            return;
        if (!setClipboard(_clipURI, buildClipboardText("herring"), http::StatusCode::OK))
            return;
        TST_LOG("Fetch clipboards:");
        if (!fetchClipboardAssert(_clipURI2, "text/plain;charset=utf-8", "kippers"))
            return;
        if (!fetchClipboardAssert(_clipURI, "text/plain;charset=utf-8", "herring"))
            return;

        TRANSITION_STATE(_phase, Phase::WaitDocClose);

        TST_LOG("Close sockets:");
        socket2->asyncShutdown();
        socket->asyncShutdown();

        LOK_ASSERT_MESSAGE("Expected successful disconnection of the WebSocket 2",
                           socket2->waitForDisconnection(5s));
        LOK_ASSERT_MESSAGE("Expected successful disconnection of the WebSocket 0",
                           socket->waitForDisconnection(5s));
    }

    void postCloseTest()
    {
        sleep(1); // paranoia.

        TST_LOG("Fetch clipboards after shutdown:");
        LOK_ASSERT_MESSAGE("Failed to get Session #2 clipboard content 'kippers'",
                           fetchClipboardAssert(_clipURI2, "text/plain;charset=utf-8", "kippers"));
        LOK_ASSERT_MESSAGE("Failed to get Session #1 clipboard content 'herring'",
                           fetchClipboardAssert(_clipURI, "text/plain;charset=utf-8", "herring"));

        TRANSITION_STATE(_phase, Phase::Done);

        TST_LOG("Clipboard tests succeeded");
        passTest("Got clipboard contents after shutdown");
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::RunTest:
            {
                runTest();
                break;
            }
            case Phase::WaitDocClose:
                break;
            case Phase::PostCloseTest:
                postCloseTest();
                break;
            case Phase::Done:
                break;
        }
    }
};

UnitBase *unit_create_wsd(void)
{
    return new UnitCopyPaste();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
