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
 * Unit test for hide_legacy_script_warning, which suppresses the notice
 * shown when an embedded script touches the legacy com.sun.star UNO API.
 */

#include <config.h>

#include <chrono>
#include <memory>
#include <string>

#include <Poco/Exception.h>
#include <Poco/URI.h>
#include <Poco/Util/LayeredConfiguration.h>
#include <test/lokassert.hpp>

#include <Unit.hpp>
#include <helpers.hpp>

using namespace std::literals;

namespace
{
/// Calls the bundled NamedRanges.py example script, which touches the legacy
/// com.sun.star UNO API via uno.createUnoStruct(), then reports whether a
/// command result and the legacy UNO API notice arrived within the given
/// window. Collects both flags from a single pass over the socket, so which
/// message arrives first does not matter.
void callScriptAndObserve(const std::shared_ptr<http::WebSocketSession>& socket,
                          const std::string& testname, bool& sawResult, bool& sawNotice,
                          std::chrono::milliseconds window)
{
    helpers::sendTextFrame(socket,
                           "uno vnd.sun.star.script:NamedRanges.py$NamedRanges"
                           "?language=Python&location=share",
                           testname);

    sawResult = false;
    sawNotice = false;
    helpers::SocketProcessor(testname, socket,
        [&](const std::string& msg)
        {
            if (msg.starts_with("unocommandresult:"))
                sawResult = true;
            else if (msg.starts_with("legacyunoapinotice:"))
                sawNotice = true;

            return !(sawResult && sawNotice);
        },
        window);
}
}

/// Test suite for hide_legacy_script_warning suppressing the legacy UNO API notice.
class UnitLegacyUnoApiWarningSuppressed : public UnitWSD
{
    TestResult testWarningSuppressedForLegacyApiUse();

public:
    UnitLegacyUnoApiWarningSuppressed()
        : UnitWSD("UnitLegacyUnoApiWarningSuppressed")
    {
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        UnitWSD::configure(config);
        config.setBool("security.enable_macros_execution", true);
        config.setBool("hide_legacy_script_warning", true);
    }

    void invokeWSDTest() override;
};

UnitBase::TestResult UnitLegacyUnoApiWarningSuppressed::testWarningSuppressedForLegacyApiUse()
{
    try
    {
        Poco::URI uri(helpers::getTestServerURI());

        std::shared_ptr<SocketPoll> socketPoll = std::make_shared<SocketPoll>("LegacyUnoApiPoll");
        socketPoll->startThread();

        std::shared_ptr<http::WebSocketSession> socket =
            helpers::loadDocAndGetSession(socketPoll, "hello.odt", uri, testname);

        bool sawResult = false;
        bool sawNotice = false;
        // The full window elapses whenever the notice never arrives, so this
        // waits it out rather than exiting the moment the command result is seen.
        callScriptAndObserve(socket, testname, sawResult, sawNotice, 5s);

        LOK_ASSERT_MESSAGE("Expected a command result for the script call", sawResult);
        LOK_ASSERT_MESSAGE("Did not expect the legacy UNO API notice", !sawNotice);
    }
    catch (const Poco::Exception& exc)
    {
        LOK_ASSERT_FAIL(exc.displayText());
    }
    return TestResult::Ok;
}

void UnitLegacyUnoApiWarningSuppressed::invokeWSDTest()
{
    UnitBase::TestResult result = testWarningSuppressedForLegacyApiUse();
    exitTest(result);
}

UnitBase* unit_create_wsd(void) { return new UnitLegacyUnoApiWarningSuppressed(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
