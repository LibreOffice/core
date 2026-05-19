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
 * Unit test for paste operations.
 */

#include <config.h>

#include <memory>
#include <string>

#include <Poco/URI.h>
#include <test/lokassert.hpp>

#include <Unit.hpp>
#include <common/HexUtil.hpp>
#include <common/Util.hpp>
#include <helpers.hpp>

/// Paste testcase.
class UnitPaste : public UnitWSD
{
public:
    UnitPaste()
        : UnitWSD("UnitPaste")
    {
    }

    void invokeWSDTest() override;
};

void UnitPaste::invokeWSDTest()
{
    // Load a document and make it empty, then paste some text into it.
    std::string documentPath;
    std::string documentURL;
    helpers::getDocumentPathAndURL("hello.odt", documentPath, documentURL, testname);

    std::shared_ptr<SocketPoll> socketPoll = std::make_shared<SocketPoll>("PastePoll");
    socketPoll->startThread();

    std::shared_ptr<http::WebSocketSession> socket = helpers::loadDocAndGetSession(
        socketPoll, Poco::URI(helpers::getTestServerURI()), documentURL, testname);

    for (int i = 0; i < 5; ++i)
    {
        const std::string text = std::to_string(i + 1) + "sh9le[;\"CFD7U[#B+nW=$kXgx{sv9QE#\"l1y\"hr" + HexUtil::encodeId(Util::rng::getNext());
        TST_LOG("Pasting text #" << i + 1 << ": " << text);

        // Always delete everything to have an empty doc.
        helpers::sendTextFrame(socket, "uno .uno:SelectAll", testname);
        helpers::sendTextFrame(socket, "uno .uno:Delete", testname);

        // Paste some text into it.
        helpers::sendTextFrame(socket, "paste mimetype=text/plain;charset=utf-8\n" + text, testname);
        const std::string expected = "textselectioncontent: " + text;

        // Check if the document contains the pasted text.
        helpers::sendTextFrame(socket, "uno .uno:SelectAll", testname);
        helpers::sendTextFrame(socket, "gettextselection mimetype=text/plain;charset=utf-8", testname);
        const auto selection = helpers::assertResponseString(socket, "textselectioncontent:", testname);
        LOK_ASSERT_EQUAL(expected, selection);
    }

    exitTest(TestResult::Ok);
}

UnitBase* unit_create_wsd(void) { return new UnitPaste(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
