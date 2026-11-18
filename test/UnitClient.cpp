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

// Runs old-style CPPUNIT tests in their own thread inside a WSD process.
// Depending which cppunit objects this is linked with this runs different
// tests.

#include <config.h>

#define TST_LOG_REDIRECT
#include <Unit.hpp>
#include <wsd/COOLWSD.hpp>

#include <test.hpp>

// Inside the WSD process
class UnitClient : public UnitWSD
{
    bool _workerStarted;
    std::thread _worker;

public:
    UnitClient()
        : UnitWSD("UnitClient")
        , _workerStarted(false)
    {
        constexpr std::chrono::minutes timeout_minutes(5);
        setTimeout(timeout_minutes);
    }
    ~UnitClient()
    {
        LOG_INF("Joining test worker thread\n");
        _worker.join();
    }

    bool filterAlertAllusers(const std::string & msg) override
    {
        std::cout << "Alert: " << msg << '\n';
        return false;
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        UnitWSD::configure(config);
        // force HTTPS - to test harder
        config.setBool("ssl.enable", true);
        // The client tests include those of the /co/collab endpoint:
        config.setBool("client_document_loading.enable", true);
    }

    void invokeWSDTest() override
    {
        // this method gets called every few seconds.
        if (_workerStarted)
            return;
        _workerStarted = true;

        _worker = std::thread([this]{
                if (runClientTests("", false, true))
                    exitTest(TestResult::Ok);
                else
                    exitTest(TestResult::Failed);
            });
    }
};

UnitBase *unit_create_wsd(void)
{
    return new UnitClient();
}

// Allows re-use of UnitClient in test.cpp impls.
#ifdef STANDALONE_CPPUNIT
#  error "Should never be compiled this way";
#endif
#include <test.cpp>

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
