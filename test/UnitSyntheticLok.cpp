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

namespace {
    void *memdup(const void *ptr, size_t size)
    {
        auto p = malloc(size);
        memcpy(p, ptr, size);
        return p;
    }
}

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

class UnitKitSyntheticLok;

UnitKitSyntheticLok *GlobalUnitKit;

// Inside the forkit & kit processes
class UnitKitSyntheticLok : public UnitKit
{
public:
    COKit *_kit;

    // Original and overridden vtables
    COKitClass *_kitClass;
    COKitClass *_kitClassClean;

    // Original and overridden vtables
    COKitDocumentClass *_docClass;
    COKitDocumentClass *_docClassClean;

    // Polling replacement
    COKitPollCallback _pollCallback;
    COKitWakeCallback _wakeCallback;
    void* _pollData;

    COKitCallback _docCallback;
    void *_docCallbackData;

    bool isDocumentCreated() const { return _docCallback != nullptr; }

    UnitKitSyntheticLok()
        : UnitKit("SyntheticLok")
        , _kit(nullptr)
        , _kitClass(nullptr)
        , _kitClassClean(nullptr)
        , _docClass(nullptr)
        , _docClassClean(nullptr)
        , _pollCallback(nullptr)
        , _wakeCallback(nullptr)
        , _pollData(nullptr)
        , _docCallback(nullptr)
        , _docCallbackData(nullptr)
    {
        TST_LOG("SyntheticLOK kit bootstrap\n");
        setTimeout(1h);
        GlobalUnitKit = this;
    }

    virtual COKit *cok_init(
        const char *instdir, const char *userdir,
        CokHookFunction2 fn) override;

    void postLOKDocumentEvent(int type, const char* payload)
    {
        assert(_docCallback);
        _docCallback(type, payload, _docCallbackData);
    }

    bool prePollCallback(int /* timeoutUs */)
    {
        return true;
    }

    virtual void doTest()
    {
        if (isDocumentCreated())
        {
            TST_LOG("Send test event");
            postLOKDocumentEvent(KIT_CALLBACK_CELL_CURSOR, "EMPTY");
            exitTest(TestResult::Ok);
        }
    }
};


extern "C" {

    int syn_pollCallback(void* /* data */, int timeoutUs)
    {
        assert(GlobalUnitKit);
        bool finished = UnitKit::get().isFinished();
        if (!finished && timeoutUs > 1000) // post initial setup we hope
            GlobalUnitKit->doTest();
        if (GlobalUnitKit->prePollCallback(timeoutUs))
            return GlobalUnitKit->_pollCallback(GlobalUnitKit->_pollData, timeoutUs);
        return 0;
    }

    void syn_wakeCallback(void* /* data */)
    {
        assert(GlobalUnitKit);
        GlobalUnitKit->_wakeCallback(GlobalUnitKit->_pollData);
    }

    void syn_registerCallback (COKitDocument* pThis,
                               COKitCallback callback,
                               void* data)
    {
        assert(GlobalUnitKit);
        GlobalUnitKit->_docCallback = callback;
        GlobalUnitKit->_docCallbackData = data;
        GlobalUnitKit->_docClassClean->registerCallback(pThis, callback, data);
    }

    COKitDocument* syn_documentLoadWithOptions (COKit* pThis,
                                                         const char* url,
                                                         const char* options)
    {
        assert(GlobalUnitKit);

        // chain to parent
        COKitDocument *doc = GlobalUnitKit->_kitClassClean->documentLoadWithOptions(pThis, url, options);

        GlobalUnitKit->_docClass = reinterpret_cast<COKitDocumentClass *>(memdup(doc->pClass, sizeof(*doc->pClass)));
        GlobalUnitKit->_docClassClean = reinterpret_cast<COKitDocumentClass *>(memdup(doc->pClass, sizeof(*doc->pClass)));
        doc->pClass = GlobalUnitKit->_docClass;

        GlobalUnitKit->_docClass->registerCallback = syn_registerCallback;

        return doc;
    }

    void syn_runLoop (COKit* pThis,
                      COKitPollCallback pollCallback,
                      COKitWakeCallback wakeCallback,
                      void* data)
    {
        assert(GlobalUnitKit);

        GlobalUnitKit->_pollCallback = pollCallback;
        GlobalUnitKit->_wakeCallback = wakeCallback;
        GlobalUnitKit->_pollData = data;

        GlobalUnitKit->_kitClassClean->runLoop(pThis, syn_pollCallback, syn_wakeCallback, data);
    }
};

COKit *UnitKitSyntheticLok::cok_init(const char *instdir,
                                              const char *userdir,
                                              CokHookFunction2 fn)
{
    // Let the parent have a go
    _kit = fn(instdir, userdir);
    if (!_kit || !_kit->pClass)
        LOK_ASSERT_FAIL("Failed to get kit initialized");

    _kitClass = reinterpret_cast<COKitClass *>(memdup(_kit->pClass, sizeof(*_kit->pClass)));
    _kitClassClean = reinterpret_cast<COKitClass *>(memdup(_kit->pClass, sizeof(*_kit->pClass)));

    // switch to our vtable
    _kit->pClass = _kitClass;

    _kitClass->runLoop = syn_runLoop;
    _kitClass->documentLoadWithOptions = syn_documentLoadWithOptions;

    return _kit;
}

UnitBase* unit_create_wsd(void) { return new UnitSyntheticLok(); }

UnitBase *unit_create_kit(void) { return new UnitKitSyntheticLok(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
