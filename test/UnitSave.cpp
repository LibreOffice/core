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
 * Unit test for document save functionality.
 */

#include <config.h>

#include <memory>
#include <string>

#include <Poco/URI.h>

#include <test/lokassert.hpp>
#include <wsd/DocumentBroker.hpp>

#include <Unit.hpp>
#include <helpers.hpp>

using namespace std::literals;

/// Save testcase.
class UnitSave : public UnitWSD
{
public:
    UnitSave();

    void invokeWSDTest() override;

    void testKeepOrigFile();
};

UnitSave::UnitSave()
    : UnitWSD("UnitSave")
{
}

void UnitSave::invokeWSDTest()
{
    testKeepOrigFile();
}

void UnitSave::testKeepOrigFile()
{
    // Given a loaded document:
    std::string name = "testKeepOrigFile";
    std::string docName = "empty.ods";
    std::string documentPath, documentURL;
    helpers::getDocumentPathAndURL(docName, documentPath, documentURL, name);
    std::shared_ptr<SocketPoll> poll = std::make_shared<SocketPoll>("WebSocketPoll");
    poll->startThread();
    Poco::URI uri(helpers::getTestServerURI());
    auto wsSession = helpers::loadDocAndGetSession(poll, docName, uri, testname);

    // When saving and waiting for the save to finish:
    wsSession->sendMessage(std::string("save dontTerminateEdit=0 dontSaveIfUnmodified=0"));
    while (!SigUtil::getShutdownRequestFlag())
    {
        std::chrono::seconds timeout = 10s;
        auto message = wsSession->waitForMessage("unocommandresult:", timeout, name);
        LOK_ASSERT(message.size() > 0);
        Poco::JSON::Object::Ptr object;
        LOK_ASSERT(JsonUtil::parseJSON(std::string(message.data(), message.size()), object));
        if (JsonUtil::getJSONValue<std::string>(object, "commandName") == ".uno:Save")
        {
            break;
        }
    }

    // Then make sure the original file is not removed:
    std::vector<std::shared_ptr<DocumentBroker>> brokers = COOLWSD::getBrokersTestOnly();
    LOK_ASSERT(brokers.size() > 0);
    std::shared_ptr<DocumentBroker> broker = brokers[0];
    StorageBase* storage = broker->getStorage();
    std::string rootFilePath = storage->getRootFilePath();
    LOK_ASSERT(FileUtil::Stat(rootFilePath).exists());
    poll->joinThread();
    exitTest(TestResult::Ok);
}

UnitBase* unit_create_wsd(void) { return new UnitSave(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
