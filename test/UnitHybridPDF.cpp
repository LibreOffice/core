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
 * Unit test for a hybrid PDF, a PDF that carries the ODF document it was exported from. Such a
 * file opens for editing as that ODF document, and saving writes a hybrid PDF again.
 */

#include <config.h>

#include <Unit.hpp>
#include <WebSocketSession.hpp>
#include <common/JsonUtil.hpp>
#include <helpers.hpp>
#include <test/lokassert.hpp>

#include <Poco/URI.h>

#include <chrono>
#include <cstring>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <thread>

using namespace std::literals;

namespace
{
std::string readFile(const std::string& path)
{
    std::ifstream stream(path, std::ios::binary);
    std::ostringstream contents;
    contents << stream.rdbuf();
    return contents.str();
}
}

class UnitHybridPDF : public UnitWSD
{
public:
    UnitHybridPDF()
        : UnitWSD("UnitHybridPDF")
    {
        setTimeout(120s);
    }

    void invokeWSDTest() override;
};

void UnitHybridPDF::invokeWSDTest()
{
    std::string documentPath, documentURL;
    helpers::getDocumentPathAndURL("hybrid_writer.pdf", documentPath, documentURL, testname);
    const std::string originalContents = readFile(documentPath);

    std::shared_ptr<SocketPoll> socketPoll = std::make_shared<SocketPoll>(testname + "Poll");
    socketPoll->startThread();

    Poco::URI uri(helpers::getTestServerURI());
    std::shared_ptr<http::WebSocketSession> socket =
        helpers::connectLOKit(socketPoll, uri, documentURL, testname);

    // The file opens as the Writer document it carries, and the view may edit it.
    helpers::sendTextFrame(socket, "load url=" + documentURL, testname);
    helpers::assertResponseString(socket, "perm: edit", testname);
    helpers::assertResponseString(socket, "hybridpdf: edit", testname);
    const std::string status = helpers::assertResponseString(socket, "status:", testname);
    Poco::JSON::Object::Ptr object;
    LOK_ASSERT(JsonUtil::parseJSON(status.substr(strlen("status: ")), object));
    LOK_ASSERT_EQUAL(std::string("text"), JsonUtil::getJSONValue<std::string>(object, "type"));
    LOK_ASSERT_EQUAL(true, JsonUtil::getJSONValue<bool>(object, "hybridpdf"));

    // Typing changes the document.
    helpers::sendTextFrame(socket, "textinput id=0 text=x", testname);
    helpers::assertResponseString(socket, "statechanged: .uno:ModifiedStatus=true", testname);

    // Saving succeeds and leaves the document unmodified.
    helpers::sendTextFrame(socket, "save dontTerminateEdit=0 dontSaveIfUnmodified=0", testname);
    while (true)
    {
        const std::string response =
            helpers::assertResponseString(socket, "unocommandresult:", testname);
        LOK_ASSERT(JsonUtil::parseJSON(response.substr(strlen("unocommandresult: ")), object));
        if (JsonUtil::getJSONValue<std::string>(object, "commandName") == ".uno:Save")
        {
            LOK_ASSERT_EQUAL(true, JsonUtil::getJSONValue<bool>(object, "success"));
            break;
        }
    }
    helpers::assertResponseString(socket, "statechanged: .uno:ModifiedStatus=false", testname);

    // The stored file is a new PDF that still carries the ODF document as an attachment. It
    // reaches the storage shortly after the save.
    std::string savedContents;
    for (int attempt = 0; attempt < 100; ++attempt)
    {
        savedContents = readFile(documentPath);
        if (savedContents != originalContents)
            break;
        std::this_thread::sleep_for(100ms);
    }
    LOK_ASSERT_MESSAGE("The saved file did not reach the storage",
                       savedContents != originalContents);
    LOK_ASSERT(savedContents.starts_with("%PDF-"));
    LOK_ASSERT(savedContents.find("/Type /EmbeddedFile") != std::string::npos);
    LOK_ASSERT(savedContents.find("/AdditionalStreams") == std::string::npos);

    exitTest(TestResult::Ok);
}

UnitBase* unit_create_wsd(void) { return new UnitHybridPDF(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
