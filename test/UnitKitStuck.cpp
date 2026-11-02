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
 * A Kit whose main thread is stuck, so that it never exits when asked to, is killed once its
 * document closes, rather than left running.
 */

#include <config.h>

#include <common/Common.hpp>
#include <common/Unit.hpp>

#include <test/helpers.hpp>
#include <test/lokassert.hpp>

#include <Poco/URI.h>

#include <chrono>
#include <memory>
#include <string>
#include <thread>

class UnitKitStuck : public UnitWSD
{
    bool _started = false;
    std::shared_ptr<SocketPoll> _socketPoll;
    std::shared_ptr<http::WebSocketSession> _socket;

public:
    UnitKitStuck()
        : UnitWSD("UnitKitStuck")
    {
        setHasKitHooks();
        setTimeout(std::chrono::minutes(2));
    }

    void invokeWSDTest() override
    {
        if (_started)
            return;
        _started = true;

#if VALGRIND_COOLFORKIT
        // A Kit under valgrind is left to finish its leak check, so it is not killed.
        passTest("A Kit under valgrind is not killed");
        return;
#endif

        std::string documentPath;
        std::string documentURL;
        helpers::getDocumentPathAndURL("hello.odt", documentPath, documentURL, testname);
        _socketPoll = std::make_shared<SocketPoll>("KitStuckPoll");
        _socketPoll->startThread();
        _socket = helpers::loadDocAndGetSession(
            _socketPoll, Poco::URI(helpers::getTestServerURI()), documentURL, testname);

        // Before the host restores an older version, WSD closes the document by itself, while the
        // view stays.
        TST_LOG("Asking WSD to close the document");
        helpers::sendTextFrame(_socket, "versionrestore prerestore", testname);
    }

    void kitKilled(int count) override
    {
        TST_LOG("Killed " << count << " Kit(s)");
        if (count > 0)
            passTest("The stuck Kit was killed");
    }
};

class UnitKitStuckKit : public UnitKit
{
public:
    UnitKitStuckKit()
        : UnitKit("UnitKitStuckKit")
    {
        setTimeout(std::chrono::minutes(2));
    }

    bool filterKitMessage(WebSocketHandler*, std::string& message) override
    {
        // Leaving a view and the request to exit are both handled on the main thread, which then
        // never returns, like one stuck in a loop in the document.
        const bool disconnect = message.starts_with("child-") && message.ends_with(" disconnect");
        if (disconnect || message == "exit")
        {
            TST_LOG("Getting stuck on [" << message << ']');
            for (;;)
                std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        return false;
    }
};

UnitBase* unit_create_wsd(void) { return new UnitKitStuck(); }

UnitBase* unit_create_kit(void) { return new UnitKitStuckKit(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
