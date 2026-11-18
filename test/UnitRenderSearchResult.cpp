/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * Unit test for rendering search results.
 */

#include <config.h>

#include <memory>
#include <string>

#include <Poco/URI.h>
#include <test/lokassert.hpp>

#include <Unit.hpp>
#include <common/Util.hpp>
#include <helpers.hpp>

class UnitRenderSearchResult : public UnitWSD
{
public:
    UnitRenderSearchResult()
        : UnitWSD("UnitRenderSearchResult")
    {
    }

    void invokeWSDTest() override;
};

void UnitRenderSearchResult::invokeWSDTest()
{
    try
    {
        std::string documentPath;
        std::string documentURL;
        helpers::getDocumentPathAndURL("RenderSearchResultTest.odt", documentPath, documentURL, testname);

        std::shared_ptr<SocketPoll> socketPoll =
            std::make_shared<SocketPoll>("RenderSearchResultPoll");
        socketPoll->startThread();

        std::shared_ptr<http::WebSocketSession> socket = helpers::loadDocAndGetSession(
            socketPoll, Poco::URI(helpers::getTestServerURI()), documentURL, testname);

        helpers::sendTextFrame(socket,
                               "rendersearchresult <indexing><paragraph node_type=\"writer\" "
                               "index=\"19\"/></indexing>",
                               testname);
        std::vector<char> responseMessage = helpers::getResponseMessage(socket, "rendersearchresult:", testname);

       // LOK_ASSERT(responseMessage.size() >= 100);
       // LOK_ASSERT_EQUAL(responseMessage[1], 'P');
       // LOK_ASSERT_EQUAL(responseMessage[2], 'N');
       // LOK_ASSERT_EQUAL(responseMessage[3], 'G');
    }
    catch (const Poco::Exception& exc)
    {
        LOK_ASSERT_FAIL(exc.displayText());
    }

    exitTest(TestResult::Ok);
}

UnitBase* unit_create_wsd(void) { return new UnitRenderSearchResult(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
