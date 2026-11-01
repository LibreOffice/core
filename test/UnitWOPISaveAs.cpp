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
 * Unit test for WOPI save-as functionality.
 */

#include <config.h>

#include <common/Log.hpp>
#include <common/Unit.hpp>
#include <test/UnitHTTP.hpp>
#include <test/WopiTestServer.hpp>
#include <test/helpers.hpp>

#include <Poco/Net/HTTPRequest.h>
#include <Poco/Util/LayeredConfiguration.h>

class UnitWOPISaveAs : public WopiTestServer
{
    STATE_ENUM(Phase, LoadAndSaveAs, Polling) _phase;

public:
    UnitWOPISaveAs()
        : WopiTestServer("UnitWOPISaveAs")
        , _phase(Phase::LoadAndSaveAs)
    {
    }

    void assertPutRelativeFileRequest(const Poco::Net::HTTPRequest& request) override
    {
        // spec says UTF-7...
        LOK_ASSERT_EQUAL_STR("/jan/hole+AWE-ovsk+AP0-/hello world+ACU-1.pdf",
                             request.get("X-WOPI-SuggestedTarget"));

        // make sure it is a pdf - or at least that it is larger than what it
        // used to be
        LOK_ASSERT(std::stoul(request.get("X-WOPI-Size")) > getFileContent().size());
    }

    bool onFilterSendWebSocketMessage(const std::string_view message, const WSOpCode /* code */,
                                      const bool /* flush */, int& /*unitReturn*/) override
    {
        if (message.find("saveas: url=") != std::string::npos &&
            message.find(helpers::getTestServerURI()) != std::string::npos &&
            message.find("filename=hello%20world%251.pdf") != std::string::npos)
        {
            // successfully exit the test if we also got the outgoing message
            // notifying about saving the file
            exitTest(TestResult::Ok);
        }

        return false;
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::LoadAndSaveAs:
            {
                initWebsocket("/wopi/files/0?access_token=anything");

                TRANSITION_STATE(_phase, Phase::Polling);

                WSD_CMD("load url=" + getWopiSrc());
                WSD_CMD("saveas url=wopi:///jan/hole%C5%A1ovsk%C3%BD/hello%20world%251.pdf");
                break;
            }
            case Phase::Polling:
            {
                // just wait for the results
                break;
            }
        }
    }
};

class UnitWOPISaveAsWithEncodedFileName : public WopiTestServer
{
    STATE_ENUM(Phase, LoadAndSaveAs, Done)
    _phase;

public:
    UnitWOPISaveAsWithEncodedFileName()
        : WopiTestServer("UnitWOPISaveAsWithEncodedFileName")
        , _phase(Phase::LoadAndSaveAs)
    {
    }

    void assertPutRelativeFileRequest(const Poco::Net::HTTPRequest& request) override
    {
        // spec says UTF-7...
        LOK_ASSERT_EQUAL_STR("/path/to/hello+ACU-20world.pdf",
                             request.get("X-WOPI-SuggestedTarget"));

        // make sure it is a pdf - or at least that it is larger than what it
        // used to be
        LOK_ASSERT(std::stoul(request.get("X-WOPI-Size")) > getFileContent().size());
    }

    bool onFilterSendWebSocketMessage(const std::string_view message, const WSOpCode /* code */,
                                      const bool /* flush */, int& /*unitReturn*/) override
    {
        if (message.find("saveas: url=") != std::string::npos &&
            message.find(helpers::getTestServerURI()) != std::string::npos &&
            message.find("filename=hello%20world%251.pdf") != std::string::npos)
        {
            // successfully exit the test if we also got the outgoing message
            // notifying about saving the file
            exitTest(TestResult::Ok);
        }

        return false;
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::LoadAndSaveAs:
            {
                initWebsocket("/wopi/files/0?access_token=anything");

                WSD_CMD("load url=" + getWopiSrc());

                // file name we want to save as is = hello%20world.pdf -> it is not encoded! and in the end we must expect like this.
                // we would send it encoded like hello%2520world.pdf
                WSD_CMD("saveas url=wopi:///path/to/hello%2520world.pdf");

                TRANSITION_STATE(_phase, Phase::Done);
                break;
            }
            case Phase::Done:
            {
                // just wait for the results
                break;
            }
        }
    }
};

/// A Save As to a name without an extension gets the extension of the document's own format,
/// and the new file is uploaded under that name.
class UnitWOPISaveAsWithoutExtension : public WopiTestServer
{
    STATE_ENUM(Phase, LoadAndSaveAs, WaitUpload, Done)
    _phase;

public:
    UnitWOPISaveAsWithoutExtension()
        : WopiTestServer("UnitWOPISaveAsWithoutExtension")
        , _phase(Phase::LoadAndSaveAs)
    {
    }

    void assertPutRelativeFileRequest(const Poco::Net::HTTPRequest& request) override
    {
        LOK_ASSERT_STATE(_phase, Phase::WaitUpload);
        LOK_ASSERT_EQUAL_STR("/path/to/renamed.odt", request.get("X-WOPI-SuggestedTarget"));
        LOK_ASSERT(std::stoul(request.get("X-WOPI-Size")) > 0);
        TRANSITION_STATE(_phase, Phase::Done);
    }

    bool onFilterSendWebSocketMessage(const std::string_view message, const WSOpCode /* code */,
                                      const bool /* flush */, int& /*unitReturn*/) override
    {
        // The client hears of the new file under the name the host gave it.
        if (_phase == Phase::Done && message.starts_with("saveas: url=") &&
            message.find(helpers::getTestServerURI()) != std::string::npos)
        {
            passTest("The new file has the extension of the document's format");
        }

        return false;
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::LoadAndSaveAs:
            {
                initWebsocket("/wopi/files/0?access_token=anything");

                WSD_CMD("load url=" + getWopiSrc());
                WSD_CMD("saveas url=wopi:///path/to/renamed");

                TRANSITION_STATE(_phase, Phase::WaitUpload);
                break;
            }
            case Phase::WaitUpload:
            case Phase::Done:
                break;
        }
    }
};

UnitBase** unit_create_wsd_multi(void)
{
    return new UnitBase*[4]{ new UnitWOPISaveAs(), new UnitWOPISaveAsWithEncodedFileName(),
                             new UnitWOPISaveAsWithoutExtension(), nullptr };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
