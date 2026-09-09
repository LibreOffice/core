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
 * WOPI test server implementation for unit tests.
 */

#pragma once

#include <common/JsonUtil.hpp>
#include <common/Log.hpp>
#include <common/Protocol.hpp>
#include <common/StringVector.hpp>
#include <common/Unit.hpp>
#include <common/Util.hpp>
#include <net/HttpRequest.hpp>

#include <test/UnitWSDClient.hpp>
#include <test/helpers.hpp>
#include <test/lokassert.hpp>

#include <Poco/Net/HTTPRequest.h>
#include <Poco/SHA2Engine.h>
#include <Poco/URI.h>
#include <Poco/Util/LayeredConfiguration.h>

#include <cctype>
#include <sstream>
#include <string_view>
#include <vector>

/// Simulates a WOPI server for testing purposes.
/// Currently only serves one file contents.
/// Furthermore, the file URI doesn't contain the
/// real filename (in most tests), instead filenames
/// 1 to 9 are considered special.
class WopiTestServer : public UnitWSDClient
{
private:

    enum class COOLStatusCode
    {
        DocChanged = 1010
    };

    /// Content of the file.
    std::string _fileContent;

    /// The filename. TODO: Support multiple ones.
    std::string _filename;

    /// Last modified time of the file
    std::chrono::system_clock::time_point _fileLastModifiedTime;

    /// The number of CheckFileInfo invocations.
    std::size_t _countCheckFileInfo;
    /// The number of GetFile invocations.
    std::size_t _countGetFile;
    /// The number of rename invocations.
    std::size_t _countPutRelative;
    /// The number of upload invocations.
    std::size_t _countPutFile;

    /// The default filename when only content is given.
    static constexpr auto DefaultFilename = "hello.txt";

protected:

    const std::string& getFileContent() const { return _fileContent; }

    /// Sets the file content to a given value and update the last file modified time
    void setFileContent(const std::string& fileContent)
    {
        _fileContent = fileContent;
        const auto oldTime = _fileLastModifiedTime;
        _fileLastModifiedTime = std::chrono::system_clock::now();
        TST_LOG("setFileContent at " << Util::getIso8601FracformatTime(getFileLastModifiedTime())
                                     << ": [" << COOLProtocol::getAbbreviatedMessage(_fileContent)
                                     << "], previous version was at "
                                     << Util::getIso8601FracformatTime(oldTime));
    }

    /// The SHA-256 of the current file content, Base64-encoded, as the WOPI
    /// SHA256 field carries it. Tests that report a hash opt in via
    /// configCheckFileInfo(); the default payload has no SHA256, so hosts that
    /// report none stay covered too.
    std::string getFileContentSha256Base64() const
    {
        Poco::SHA2Engine engine(Poco::SHA2Engine::SHA_256);
        engine.update(_fileContent.data(), _fileContent.size());
        return Util::base64Encode(engine.digest());
    }

    const std::chrono::system_clock::time_point& getFileLastModifiedTime() const
    {
        return _fileLastModifiedTime;
    }

    WopiTestServer(const std::string& name, const std::string& filenameOrContents = "Hello, world")
        : UnitWSDClient(name)
        , _filename(DefaultFilename)
        , _countCheckFileInfo(0)
        , _countGetFile(0)
        , _countPutRelative(0)
        , _countPutFile(0)
    {
        TST_LOG("WopiTestServer created for [" << getTestname() << ']');

        // Read the document data and store as string in memory.
        const auto data = helpers::readDataFromFile(filenameOrContents);
        if (!data.empty())
        {
            // That was a filename, set its contents.
            TST_LOG("WopiTestServer created with " << data.size() << " bytes from file ["
                                                   << filenameOrContents << ']');
            _filename = filenameOrContents; // Capture the real filename.
            setFileContent(std::string(data.begin(), data.end()));
        }
        else
        {
            // Not a valid filename, assume it's some data.
            TST_LOG("WopiTestServer created with " << filenameOrContents.size()
                                                   << " bytes from data.");
            setFileContent(filenameOrContents);
        }
    }

    std::size_t getCountCheckFileInfo() const { return _countCheckFileInfo; }
    void resetCountCheckFileInfo()
    {
        TST_LOG("Resetting countCheckFileInfo [" << _countCheckFileInfo << "] to zero");
        _countCheckFileInfo = 0;
    }

    std::size_t getCountGetFile() const { return _countGetFile; }
    void resetCountGetFile()
    {
        TST_LOG("Resetting countGetFile [" << _countGetFile << "] to zero");
        _countGetFile = 0;
    }

    std::size_t getCountPutRelative() const { return _countPutRelative; }
    void resetCountPutRelative()
    {
        TST_LOG("Resetting countPutRelative [" << _countPutRelative << "] to zero");
        _countPutRelative = 0;
    }

    std::size_t getCountPutFile() const { return _countPutFile; }
    void resetCountPutFile()
    {
        TST_LOG("Resetting countPutFile [" << _countPutFile << "] to zero");
        _countPutFile = 0;
    }

    virtual std::unique_ptr<http::Response>
    assertCheckFileInfoRequest(const Poco::Net::HTTPRequest& /*request*/)
    {
        return nullptr; // Success.
    }

    virtual std::unique_ptr<http::Response>
    assertGetFileRequest(const Poco::Net::HTTPRequest& /*request*/)
    {
        return nullptr; // Success.
    }

    /// Assert the PutFile request is valid and optionally return a response.
    virtual std::unique_ptr<http::Response>
    assertPutFileRequest(const Poco::Net::HTTPRequest& /*request*/)
    {
        return nullptr; // Success.
    }

    virtual void assertPutRelativeFileRequest(const Poco::Net::HTTPRequest& /*request*/)
    {
    }

    virtual void assertRenameFileRequest(const Poco::Net::HTTPRequest& /*request*/)
    {
    }

    /// Called when the server receives a Lock or Unlock request.
    virtual std::unique_ptr<http::Response>
    assertLockRequest(const Poco::Net::HTTPRequest& /*request*/)
    {
        return nullptr; // Success.
    }

    /// Given a URI, returns the filename.
    ///FIXME: this should be remove when we support multiple files properly.
    virtual std::string getFilename(const Poco::URI& uri) const
    {
        std::string filename = extractFilenameFromWopiUri(uri.getPath());

        // Note: This is a fake implementation.
        if (filename == "3")
        {
            // Test '%' in the filename.
            //FIXME: pass this in the URI.
            return "he%llo.txt";
        }

        if (filename.size() == 1 && std::isdigit(filename[0]))
        {
            const auto number = NumUtil::stoi(filename);
            if (number >= 1 && number <= 9)
            {
                // Fake filename, depends on implicit filename.
                return DefaultFilename;
            }
        }

        // Return the filename given in the URI.
        return filename;
    }

    /// Returns the virtual root-path that we serve.
    static const std::string& getURIRootPath()
    {
        static const std::string RootPath = "/wopi/files/";
        return RootPath;
    }

    /// Given a wopi URI, extracts the filename.
    static std::string extractFilenameFromWopiUri(const std::string& uriPath)
    {
        if (uriPath.starts_with(getURIRootPath()))
        {
            const auto first = getURIRootPath().size();
            const auto it = uriPath.find_first_of('/', first);
            return uriPath.substr(first, it);
        }

        return std::string();
    }

    /// Returns true iff @uriPath is a Wopi path but not to the contents.
    static bool isWopiInfoRequest(const std::string_view uriPath)
    {
        return uriPath.starts_with(getURIRootPath()) && !uriPath.ends_with("/contents");
    }

    /// Returns true iff @uriPath is a Wopi path to the contents of a file.
    static bool isWopiContentRequest(const std::string_view uriPath)
    {
        return uriPath.starts_with(getURIRootPath()) && uriPath.ends_with("/contents");
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        UnitWSD::configure(config);
        // we're still internally confused as to https vs. http in places.
        config.setBool("storage.ssl.as_scheme", false);

        // Reset to default.
        config.setBool("storage.wopi.is_legacy_server", false);

        // Reduce the save retry time.
        config.setUInt("per_document.min_time_between_saves_ms", 100);
        // Reduce the upload retry time.
        config.setUInt("per_document.min_time_between_uploads_ms", 500);
    }

    /// Returns the default CheckFileInfo json.
    Poco::JSON::Object::Ptr getDefaultCheckFileInfoPayload(const Poco::URI& uri)
    {
        Poco::JSON::Object::Ptr fileInfo = new Poco::JSON::Object();
        fileInfo->set("BaseFileName", getFilename(uri));
        fileInfo->set("Size", getFileContent().size());
        fileInfo->set("Version", "1.0");
        fileInfo->set("OwnerId", "tuser");
        fileInfo->set("UserId", "tuser");
        fileInfo->set("UserFriendlyName", "Test User");
        fileInfo->set("UserCanWrite", "true");
        fileInfo->set("PostMessageOrigin", "localhost");
        fileInfo->set("LastModifiedTime",
                      Util::getIso8601FracformatTime(getFileLastModifiedTime()));
        fileInfo->set("EnableOwnerTermination", "true");
        fileInfo->set("SupportsLocks", "false");

        return fileInfo;
    }

    /// Returns the default CheckFileInfo json.
    Poco::JSON::Object::Ptr getDefaultCheckFileInfoPayload(const std::string& uri)
    {
        return getDefaultCheckFileInfoPayload(Poco::URI(uri));
    }

    /// Handles WOPI CheckFileInfo requests.
    virtual bool handleCheckFileInfoRequest(const Poco::Net::HTTPRequest& request,
                                            const std::shared_ptr<StreamSocket>& socket)
    {
        std::unique_ptr<http::Response> httpResponse = assertCheckFileInfoRequest(request);
        if (!httpResponse)
            httpResponse = std::make_unique<http::Response>(http::StatusCode::OK);

        if (httpResponse->statusLine().statusCategory() ==
            http::StatusLine::StatusCodeClass::Successful)
        {
            Poco::JSON::Object::Ptr fileInfo = getDefaultCheckFileInfoPayload(request.getURI());
            configCheckFileInfo(request, fileInfo);

            std::ostringstream jsonStream;
            fileInfo->stringify(jsonStream);
            std::string json = jsonStream.str();
            TST_LOG("FakeWOPIHost: Response to CheckFileInfo "
                    << Poco::URI(request.getURI()).getPath() << ": 200 OK: " << json);

            httpResponse->set("Last-Modified", Util::getHttpTime(getFileLastModifiedTime()));
            httpResponse->setBody(std::move(json), "application/json; charset=utf-8");
        }
        else
        {
            TST_LOG("FakeWOPIHost: Response to CheckFileInfo "
                    << Poco::URI(request.getURI()).getPath()
                    << httpResponse->statusLine().statusCode() << ' '
                    << httpResponse->statusLine().reasonPhrase());
        }

        socket->sendAndShutdown(*httpResponse);
        return true;
    }

    /// Override to set the CheckFileInfo attributes.
    virtual void configCheckFileInfo(const Poco::Net::HTTPRequest& /*request*/,
                                     Poco::JSON::Object::Ptr& /*fileInfo*/)
    {
    }

    virtual bool handleGetFileRequest(const Poco::Net::HTTPRequest& request,
                                      const std::shared_ptr<StreamSocket>& socket)
    {
        std::unique_ptr<http::Response> httpResponse = assertGetFileRequest(request);
        if (!httpResponse)
            httpResponse = std::make_unique<http::Response>(http::StatusCode::OK);

        if (httpResponse->statusLine().statusCategory() ==
            http::StatusLine::StatusCodeClass::Successful)
        {
            TST_LOG("FakeWOPIHost: Response to GetFile " << Poco::URI(request.getURI()).getPath()
                                                         << ": 200 OK (" << getFileContent().size()
                                                         << " bytes)");
            httpResponse->set("Last-Modified", Util::getHttpTime(getFileLastModifiedTime()));
            httpResponse->setBody(getFileContent(), "application/octet-stream");
        }
        else
        {
            TST_LOG("FakeWOPIHost: Response to GetFile "
                    << Poco::URI(request.getURI()).getPath()
                    << httpResponse->statusLine().statusCode() << ' '
                    << httpResponse->statusLine().reasonPhrase());
        }

        socket->sendAndShutdown(*httpResponse);
        return true;
    }

    virtual bool handleHttpGetRequest(const Poco::Net::HTTPRequest& request,
                                      const std::shared_ptr<StreamSocket>& socket)
    {
        LOG_ASSERT_MSG(request.getMethod() == "GET", "Expect an HTTP GET request");

        const Poco::URI uriReq(request.getURI());

        if (isWopiInfoRequest(uriReq.getPath())) // CheckFileInfo
        {
            ++_countCheckFileInfo;
            TST_LOG("FakeWOPIHost: Handling CheckFileInfo (#" << _countCheckFileInfo
                                                              << "): " << uriReq.getPath());

            return handleCheckFileInfoRequest(request, socket);
        }
        else if (isWopiContentRequest(uriReq.getPath())) // GetFile
        {
            ++_countGetFile;
            TST_LOG("FakeWOPIHost: Handling GetFile (#" << _countGetFile
                                                        << "): " << uriReq.getPath());

            return handleGetFileRequest(request, socket);
        }

        TST_LOG("FakeWOPIHost: not a wopi request, skipping: " << uriReq.getPath());
        return false;
    }

    virtual bool handleHttpPostRequest(const Poco::Net::HTTPRequest& request,
                                       std::istream& message,
                                       const std::shared_ptr<StreamSocket>& socket)
    {
        LOG_ASSERT_MSG(request.getMethod() == "POST", "Expect an HTTP POST request");

        const Poco::URI uriReq(request.getURI());
        if (isWopiInfoRequest(uriReq.getPath()))
        {
            if (!request.get("X-WOPI-Lock", std::string()).empty())
            {
                const std::string op = request.get("X-WOPI-Override", std::string());
                if (op == "LOCK" || op == "UNLOCK")
                {
                    std::unique_ptr<http::Response> response = assertLockRequest(request);
                    if (!response)
                    {
                        TST_LOG("FakeWOPIHost: " << op << " operation on [" << uriReq.getPath()
                                                 << "] succeeded 200 OK");
                        http::Response httpResponse(http::StatusCode::OK);
                        socket->sendAndShutdown(httpResponse);
                    }
                    else
                    {
                        TST_LOG("FakeWOPIHost: " << op << " operation on [" << uriReq.getPath()
                                                 << "]: " << response->statusLine().statusCode()
                                                 << ' ' << response->statusLine().reasonPhrase());
                        socket->sendAndShutdown(*response);
                    }
                }
                else
                {
                    http::Response httpResponse(http::StatusCode::Conflict);
                    httpResponse.set("X-WOPI-LockFailureReason", "Invalid lock operation");
                    socket->sendAndShutdown(httpResponse);
                }
            }
            else
            {
                const std::string wopiURL =
                    helpers::getTestServerURI() +
                    "/something wopi/files/1?access_token=anything&reuse_cookies=cook=well";

                std::string content;
                if (request.get("X-WOPI-Override") == std::string("PUT_RELATIVE"))
                {
                    ++_countPutRelative;
                    TST_LOG("FakeWOPIHost: Handling PutRelativeFile (#"
                            << _countPutRelative << "): " << uriReq.getPath());

                    LOK_ASSERT_EQUAL_STR("PUT_RELATIVE", request.get("X-WOPI-Override"));
                    assertPutRelativeFileRequest(request);
                    content = "{ \"Name\":\"hello world%1.pdf\", \"Url\":\"" + wopiURL + "\" }";
                }
                else
                {
                    // rename file; response should be the file name without the url and the extension
                    LOK_ASSERT_EQUAL_STR("RENAME_FILE", request.get("X-WOPI-Override"));
                    assertRenameFileRequest(request);
                    content = "{ \"Name\":\"hello\", \"Url\":\"" + wopiURL + "\" }";
                }

                http::Response httpResponse(http::StatusCode::OK);
                httpResponse.set("Last-Modified", Util::getHttpTime(getFileLastModifiedTime()));
                httpResponse.setBody(std::move(content), "application/json; charset=utf-8");
                socket->sendAndShutdown(httpResponse);
            }

            return true;
        }
        else if (isWopiContentRequest(uriReq.getPath()))
        {
            ++_countPutFile;
            TST_LOG("FakeWOPIHost: Handling PutFile (#" << _countPutFile
                                                        << "): " << uriReq.getPath());

            return handleWopiUpload(request, message, socket);
        }

        return false;
    }

    /// Handles the uploading of a document to wopi.
    virtual bool handleWopiUpload(const Poco::Net::HTTPRequest& request,
                                  std::istream& message,
                                  const std::shared_ptr<StreamSocket>& socket)
    {
        const std::string wopiTimestamp = request.get("X-COOL-WOPI-Timestamp", std::string());
        if (!wopiTimestamp.empty())
        {
            const std::string fileModifiedTime =
                Util::getIso8601FracformatTime(getFileLastModifiedTime());
            if (wopiTimestamp != fileModifiedTime)
            {
                TST_LOG("FakeWOPIHost: Document conflict detected, Stored ModifiedTime: ["
                        << fileModifiedTime << "], Upload ModifiedTime: [" << wopiTimestamp << ']');
                http::Response httpResponse(http::StatusCode::Conflict);
                httpResponse.setBody("{\"COOLStatusCode\":" +
                                     std::to_string(static_cast<int>(COOLStatusCode::DocChanged)) +
                                     '}');
                socket->sendAndShutdown(httpResponse);
                return true;
            }
        }
        else
        {
            TST_LOG("FakeWOPIHost: Forced document upload");
        }

        std::unique_ptr<http::Response> response = assertPutFileRequest(request);
        if (!response || response->statusLine().statusCategory() ==
                             http::StatusLine::StatusCodeClass::Successful)
        {
            const std::streamsize size = request.getContentLength();
            TST_LOG("FakeWOPIHost: Writing document contents in storage (" << size << "bytes)");
            std::vector<char> buffer(size);
            message.read(buffer.data(), size);
            setFileContent(Util::toString(buffer));
        }

        const Poco::URI uriReq(request.getURI());
        if (response)
        {
            TST_LOG("FakeWOPIHost: Response to POST " << uriReq.getPath() << ": "
                                                      << response->statusLine().statusCode() << ' '
                                                      << response->statusLine().reasonPhrase());
            socket->sendAndShutdown(*response);
        }
        else
        {
            // By default we return success.
            std::string body = "{\"LastModifiedTime\": \"" +
                               Util::getIso8601FracformatTime(getFileLastModifiedTime()) +
                               "\" }";
            TST_LOG("FakeWOPIHost: Response (default) to POST " << uriReq.getPath() << ": 200 OK "
                                                                << body);
            http::Response httpResponse(http::StatusCode::OK);
            httpResponse.setBody(std::move(body), "application/json; charset=utf-8");
            socket->sendAndShutdown(httpResponse);
        }

        return true;
    }

    /// In some very rare cases we may get requests from other tests.
    /// This asserts that the URI in question is for our test.
    void assertTargetTest(const Poco::URI& uri)
    {
        const auto params = uri.getQueryParameters();
        const auto testnameIt = std::find_if(params.begin(), params.end(),
                                             [](const std::pair<std::string, std::string>& pair)
                                             { return pair.first == "testname"; });

        // Some URLs might not have the testname parameter.
        LOK_CHECK_MESSAGE_SILENT("Request belongs to an unknown test", testnameIt != params.end());
        if (testnameIt != params.end())
        {
            const std::string target = StringVector::tokenize(testnameIt->second, '/')[0];
            LOK_ASSERT_EQUAL_MESSAGE("Request belongs to a different test", getTestname(), target);
        }
    }

    /// Here we act as a WOPI server, so that we have a server that responds to
    /// the wopi requests without additional expensive setup.
    bool handleHttpRequest(const Poco::Net::HTTPRequest& request, std::istream& message,
                           const std::shared_ptr<StreamSocket>& socket) override
    {
        Poco::URI uriReq(Uri::decode(request.getURI()));

        TST_LOG("FakeWOPIHost: " << request.getMethod() << " request URI [" << uriReq.toString()
                                 << "]: " <<
                [&](auto& log)
                {
                    Util::joinPair(log, request, " / ");
                } << (UnitBase::get().isFinished() ? " - Ignoring as test has finished" : ""));

        if (UnitBase::get().isFinished())
            return false;

        assertTargetTest(uriReq);

        if (request.getMethod() == "GET")
        {
            return handleHttpGetRequest(request, socket);
        }
        else if (request.getMethod() == "POST")
        {
            return handleHttpPostRequest(request, message, socket);
        }
        else if (!uriReq.getPath().starts_with("/cool/")) // Skip requests to the websrv.
        {
            // Complain if we are expected to handle something that we don't.
            TST_LOG("ERROR: FakeWOPIHost: Request, cannot handle request: " << uriReq.getPath());
        }

        return false;
    }

};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
