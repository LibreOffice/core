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
 * Unit test for WOPI file URL handling.
 */

#include <config.h>

#include <lokassert.hpp>

#include <regex>

#include <Poco/Net/HTTPRequest.h>

#include <WopiTestServer.hpp>
#include <common/Log.hpp>
#include <Unit.hpp>
#include <UnitHTTP.hpp>
#include <helpers.hpp>
#include <net/HttpHelper.hpp>

/// Test the FileUrl WOPI CheckFileInfo property.
/// When FileUrl is set, it must be used for
/// WOPI::GetFile requests, and only when that
/// fails do we request the default file.
///
/// This test loads the document three times.
/// 1. CheckFileInfo returns a valid FileUrl,
///    and is downloaded successfully.
/// 2. CheckFileInfo gives an invalid FileUrl,
///    download fails, default must be used.
/// 3. CheckFileInfo gives no FileUrl,
///    the default must be used.
class UnitWOPIFileUrl : public WopiTestServer
{
    /// The main phases of the test, repeated
    /// once for each test case (3 times).
    /// First, we load a document, modify it,
    /// then save. Validations are done during
    /// GetFile and PutFile to check that FileUrl
    /// is used, or not, as expected.
    STATE_ENUM(Phase, Load, WaitLoadStatus, WaitModifiedStatus, WaitPutFile, Polling) _phase;

    /// We have three tests, one for each
    /// of the following cases of FileUrl:
    /// 1. When it's provided and valid.
    /// 2. When it's provided and invalid.
    /// 3. When it's missing/blank.
    enum class FileUrlState
    {
        Valid,
        Invalid,
        Absent
    } _fileUrlState;

    /// We use different documents because
    /// we can't reload the same document so rapidly.
    int _docId;

    static constexpr auto FileUrlFilename = "/hello.odt";
    static constexpr auto InvalidFilename = "/invalid_file";

public:
    UnitWOPIFileUrl()
        : WopiTestServer("UnitWOPIFileUrl")
        , _phase(Phase::Load)
        , _fileUrlState(FileUrlState::Valid)
        , _docId(1)
    {
    }

    virtual bool handleHttpRequest(const Poco::Net::HTTPRequest& request,
                                   std::istream& message,
                                   const std::shared_ptr<StreamSocket>& socket) override
    {
        Poco::URI uriReq(request.getURI());
        TST_LOG("FakeWOPIHost: Handling HTTP " << request.getMethod()
                                               << " request: " << uriReq.toString());

        constexpr auto DefaultUrlFilename = "empty.odt";
        static const std::regex regContent("/wopi/files/[0-9]/contents");

        if (request.getMethod() == "GET")
        {
            static const std::regex regInfo("/wopi/files/[0-9]");

            // CheckFileInfo
            if (std::regex_match(uriReq.getPath(), regInfo))
            {
                TST_LOG("FakeWOPIHost: Handling WOPI::CheckFileInfo: " << uriReq.getPath());

                Poco::JSON::Object::Ptr fileInfo = getDefaultCheckFileInfoPayload(uriReq);
                fileInfo->set("BaseFileName", DefaultUrlFilename);
                fileInfo->set("FileUrl", getFileUrl());

                std::ostringstream jsonStream;
                fileInfo->stringify(jsonStream);

                http::Response httpResponse(http::StatusCode::OK);
                httpResponse.set("Last-Modified", Util::getHttpTime(getFileLastModifiedTime()));
                httpResponse.setBody(jsonStream.str(), "application/json; charset=utf-8");
                socket->sendAndShutdown(httpResponse);

                return true;
            }

            if (uriReq.getPath() == FileUrlFilename)
            {
                const std::string filename = std::string(TDOC) + FileUrlFilename;
                TST_LOG("FakeWOPIHost: Request, WOPI::GetFile sending FileUrl: " << filename);

                http::Response response(http::StatusCode::OK);
                HttpHelper::sendFileAndShutdown(socket, filename, response);
                return true;
            }

            if (uriReq.getPath() == InvalidFilename)
            {
                const std::string filename = std::string(TDOC) + InvalidFilename;
                TST_LOG("FakeWOPIHost: Request, WOPI::GetFile returning 404 for: " << filename);

                HttpHelper::sendErrorAndShutdown(http::StatusCode::NotFound, socket);

                return true;
            }

            if (std::regex_match(uriReq.getPath(), regContent))
            {
                if (_fileUrlState == FileUrlState::Valid)
                {
                    LOK_ASSERT_FAIL(
                        "The default URL should not be used when a valid FileUrl is provided.");
                    exitTest(TestResult::Failed);
                    return false;
                }

                const std::string filename = std::string(TDOC) + '/' + DefaultUrlFilename;
                TST_LOG("FakeWOPIHost: Request, WOPI::GetFile sending Default: " << filename);

                http::Response response(http::StatusCode::OK);
                HttpHelper::sendFileAndShutdown(socket, filename, response);
                return true;
            }
        }
        else if (request.getMethod() == "POST")
        {
            TST_LOG("FakeWOPIHost: Handling PutFile: " << uriReq.getPath());

            LOK_ASSERT_STATE(_phase, Phase::WaitPutFile);

            LOK_ASSERT_MESSAGE("Always the default URI must be used for PutFile",
                               std::regex_match(uriReq.getPath(), regContent));

            std::streamsize size = request.getContentLength();
            LOK_ASSERT(size > 0);

            http::Response httpResponse(http::StatusCode::OK);
            httpResponse.setBody("{\"LastModifiedTime\": \"" +
                                     Util::getHttpTime(getFileLastModifiedTime()) + "\" }",
                                 "application/json; charset=utf-8");
            socket->sendAndShutdown(httpResponse);

            TST_LOG("Closing document after PutFile");
            WSD_CMD("closedocument");
            ++_docId; // Load the next file.

            if (_fileUrlState == FileUrlState::Valid)
            {
                TST_LOG("Valid FileUrl test successful. Now testing Invalid FileUrl.");
                _fileUrlState = FileUrlState::Invalid;
                TRANSITION_STATE(_phase, Phase::Load);
            }
            else if (_fileUrlState == FileUrlState::Invalid)
            {
                TST_LOG("Invalid FileUrl test successful. Now testing without FileUrl.");
                _fileUrlState = FileUrlState::Absent;
                TRANSITION_STATE(_phase, Phase::Load);
            }
            else if (_fileUrlState == FileUrlState::Absent)
            {
                TST_LOG("Testing of FileUrl completed successfully.");
                TRANSITION_STATE(_phase, Phase::Polling);
                exitTest(TestResult::Ok);
            }

            return true;
        }

        return WopiTestServer::handleHttpRequest(request, message, socket);
    }

    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("onDocumentLoaded: [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitLoadStatus);

        TST_LOG(
            "onDocumentLoaded: Modifying the document and switching to Phase::WaitModifiedStatus");
        TRANSITION_STATE(_phase, Phase::WaitModifiedStatus);

        WSD_CMD("key type=input char=97 key=0");
        WSD_CMD("key type=up char=0 key=512");

        return true;
    }

    bool onDocumentModified(const std::string& message) override
    {
        TST_LOG("onDocumentModified: [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitModifiedStatus);

        TST_LOG("onDocumentModified: Saving document and switching to Phase::WaitPutFile");
        TRANSITION_STATE(_phase, Phase::WaitPutFile);

        WSD_CMD("save dontTerminateEdit=0 dontSaveIfUnmodified=0 "
                "extendedData=CustomFlag%3DCustom%20Value%3BAnotherFlag%3DAnotherValue");

        return true;
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::Load:
            {
                TST_LOG("Phase::Load");
                TRANSITION_STATE(_phase, Phase::WaitLoadStatus);

                initWebsocket("/wopi/files/" + std::to_string(_docId) + "?access_token=anything");

                WSD_CMD("load url=" + getWopiSrc());
                break;
            }
            case Phase::WaitLoadStatus:
            case Phase::WaitModifiedStatus:
            case Phase::WaitPutFile:
            {
                break;
            }
            case Phase::Polling:
            {
                exitTest(TestResult::Ok);
                break;
            }
        }
    }

private:
    std::string getFileUrl() const
    {
        if (_fileUrlState == FileUrlState::Valid)
        {
            return helpers::getTestServerURI() + FileUrlFilename;
        }
        else if (_fileUrlState == FileUrlState::Invalid)
        {
            return helpers::getTestServerURI() + InvalidFilename;
        }

        return std::string();
    }
};

UnitBase* unit_create_wsd(void) { return new UnitWOPIFileUrl(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
