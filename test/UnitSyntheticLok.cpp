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
 * Unit test for synthetic COKit functionality.
 */

#include <config.h>

#include <Unit.hpp>
#include <common/Util.hpp>
#include <helpers.hpp>
#include <common/StringVector.hpp>
#include <WebSocketSession.hpp>
#include <test/testlog.hpp>
#include <test/lokassert.hpp>

#include <COKit/COKit.hxx>

#include <string>
#include <thread>

using namespace std::literals;

bool testCompletedSuccess = false;

class UnitSyntheticLok : public UnitWSD
{
    void loadAndSynthesize(const std::string& name, const std::string& docName);

public:
    UnitSyntheticLok();
    void invokeWSDTest() override;
    void endTest(const std::string& reason) override;
};

void UnitSyntheticLok::loadAndSynthesize(
    const std::string& name, const std::string& docName)
{
    auto timeout = 10s;

    std::string documentPath, documentURL;
    helpers::getDocumentPathAndURL(docName, documentPath, documentURL, name);

    TST_LOG("Starting test on " << documentURL << ' ' << documentPath);

    std::shared_ptr<SocketPoll> poll = std::make_shared<SocketPoll>("WebSocketPoll");
    poll->startThread();

    Poco::URI uri(helpers::getTestServerURI());
    auto wsSession = helpers::loadDocAndGetSession(poll, docName, uri, testname, true, false);

    // If we have already exitTest successfully when this returns, then that's fine.
    if (testCompletedSuccess)
        return;

    std::vector<char> message
        = wsSession->waitForMessage("status:", timeout, name);
    const std::string status = COOLProtocol::getFirstLine(message);

    // Kit will signal success through unitresult: to wsd in its own time.
}

UnitSyntheticLok::UnitSyntheticLok()
    : UnitWSD("UnitSyntheticLok")
{
    setHasKitHooks();
    // Double of the default.
    constexpr std::chrono::minutes timeout_minutes(1);
    setTimeout(timeout_minutes);
}

void UnitSyntheticLok::invokeWSDTest()
{
    const auto name = "syntheticLok";

    static bool started = false;
    if (!started)
    {
        started = true;
        loadAndSynthesize(name, "empty.ods");
    }
    // wait for result from the Kit process
}

void UnitSyntheticLok::endTest(const std::string& reason)
{
    UnitWSD::endTest(reason);
    testCompletedSuccess = !failed();
}

// Inside the forkit & kit processes
class UnitKitSyntheticLok : public UnitKit
{
    COKitCallback _docCallback;
    void* _docCallbackData;

    bool isDocumentCreated() const { return _docCallback != nullptr; }

public:
    UnitKitSyntheticLok()
        : UnitKit("SyntheticLok")
        , _docCallback(nullptr)
        , _docCallbackData(nullptr)
    {
        TST_LOG("SyntheticLOK kit bootstrap\n");
        setTimeout(1h);
    }

    void postViewCallbackRegistered(COKitCallback callback, void* data) override
    {
        _docCallback = callback;
        _docCallbackData = data;
    }

    void postLOKDocumentEvent(COKitCallbackType eType, const char* payload)
    {
        assert(_docCallback);
        _docCallback(eType, payload, _docCallbackData);
    }

    /// Once a view has a callback, feed one synthetic cell cursor event into the kit.
    void preKitPollCallback() override
    {
        if (isFinished() || !isDocumentCreated())
            return;

        TST_LOG("Send test event");
        postLOKDocumentEvent(COKitCallbackType::CELL_CURSOR, "EMPTY");
        exitTest(TestResult::Ok);
    }
};

UnitBase* unit_create_wsd(void) { return new UnitSyntheticLok(); }

UnitBase *unit_create_kit(void) { return new UnitKitSyntheticLok(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
