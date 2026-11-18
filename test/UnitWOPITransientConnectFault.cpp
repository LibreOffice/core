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
 * Unit test for WOPI transient connection fault handling.
 */

#include <config.h>

#include <HttpRequest.hpp>
#include <lokassert.hpp>

#include <WopiTestServer.hpp>
#include <common/Log.hpp>
#include <Unit.hpp>
#include <UnitHTTP.hpp>
#include <helpers.hpp>
#include <common/Message.hpp>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Util/LayeredConfiguration.h>

/// Test saving with simulated failing.
/// We modify the document and close.
/// The document must then be saved and uploaded but
/// if the initial upload fails for a transitory dns/network
/// issue a retry should be successful,
class UnitWOPITransientConnectFault : public WopiTestServer
{
    STATE_ENUM(Phase, Load, WaitLoadStatus, WaitFailedUpload, WaitSuccessUpload)
    _phase;

    bool _injectDNSFault;
    int _uploadFailures;
public:
    UnitWOPITransientConnectFault()
        : WopiTestServer("UnitWOPITransientConnectFault")
        , _phase(Phase::Load)
        , _injectDNSFault(false)
        , _uploadFailures(0)
    {
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        WopiTestServer::configure(config);

        config.setUInt("per_document.min_time_between_uploads_ms", 0);
        config.setUInt("per_document.min_time_between_saves_ms", 0);
    }

    /// The document is loaded.
    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("onDocumentLoaded: [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitLoadStatus);

        TRANSITION_STATE(_phase, Phase::WaitFailedUpload);

        // deliberately make dns fail for the first upload attempt after save
        _injectDNSFault = true;

        WSD_CMD("key type=input char=97 key=0");
        WSD_CMD("key type=up char=0 key=512");
        WSD_CMD("save");

        return true;
    }

    bool onDocumentError(const std::string& message) override
    {
        // We expect this to happen once for the injected fault and not again
        LOK_ASSERT_EQUAL_MESSAGE(
            "Expect only documentconflict errors after the second CheckFileInfo",
            std::string("error: cmd=storage kind=savefailed"), message);

        ++_uploadFailures;

        LOK_ASSERT_EQUAL(1, _uploadFailures);

        TRANSITION_STATE(_phase, Phase::WaitSuccessUpload);

        WSD_CMD("key type=input char=97 key=0");
        WSD_CMD("key type=up char=0 key=512");
        WSD_CMD("save");

        return true;
    }

    bool handleWopiUpload(const Poco::Net::HTTPRequest& request,
                          std::istream& message,
                          const std::shared_ptr<StreamSocket>& socket) override
    {
        LOK_ASSERT_STATE(_phase, Phase::WaitSuccessUpload);
        // we should have had an initial save failure, and this retry should
        // happen after that, if so all is well
        LOK_ASSERT_EQUAL(1, _uploadFailures);
        bool ret = WopiTestServer::handleWopiUpload(request, message, socket);
        passTest("Finished, upload retry arrived.");
        return ret;
    }

    void filterResolveDNS(std::string& query) override
    {
        // Inject a single dns failure, a retry then should succeed before fix
        // the failure poisoned the session with an active upload that never
        // completed.
        if (_injectDNSFault)
        {
            _injectDNSFault = false;
            // .invalid – guaranteed to be invalid for DNS resolution
            query = "something.invalid";
        }
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::Load:
            {
                TRANSITION_STATE(_phase, Phase::WaitLoadStatus);

                TST_LOG("Load: initWebsocket");
                initWebsocket("/wopi/files/0?access_token=anything");

                WSD_CMD("load url=" + getWopiSrc());
                break;
            }
            case Phase::WaitLoadStatus:
                break;
            case Phase::WaitFailedUpload:
            case Phase::WaitSuccessUpload:
                break;
        }
    }
};

UnitBase* unit_create_wsd(void) { return new UnitWOPITransientConnectFault(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
