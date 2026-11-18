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
 * Unit test for convert-to handling of password-protected and failed documents.
 * Verifies that:
 * - Password-protected documents return HTTP 200 with a locked-document icon PNG.
 * - Other load failures (e.g. corrupted documents) return HTTP 500 (not 502).
 */

#include <config.h>

#include <iostream>

#include <Common.hpp>
#include <Protocol.hpp>
#include <Unit.hpp>
#include <common/Util.hpp>
#include <common/FileUtil.hpp>
#include <helpers.hpp>

#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTMLForm.h>
#include <Poco/Net/FilePartSource.h>
#include <Poco/Path.h>
#include <Poco/StreamCopier.h>
#include <Poco/Util/LayeredConfiguration.h>

using namespace std::literals;

// Inside the WSD process
class UnitConvertToPasswordProtected : public UnitWSD
{
    bool _workerStarted;
    std::thread _worker;

    /// Send a convert-to PNG request for the given test document.
    void sendConvertTo(std::unique_ptr<Poco::Net::HTTPClientSession>& session,
                       const std::string& filename)
    {
        Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_POST,
                                       "/cool/convert-to/png");
        Poco::Net::HTMLForm form;
        form.setEncoding(Poco::Net::HTMLForm::ENCODING_MULTIPART);
        form.set("format", "png");
        std::string srcPath = Poco::Path(TDOC, filename).toString();
        form.addPart("data", new Poco::Net::FilePartSource(srcPath));
        form.prepareSubmit(request);
        form.write(session->sendRequest(request));
    }

    /// Receive the convert-to response. Returns status code.
    int receiveResponse(std::unique_ptr<Poco::Net::HTTPClientSession>& session,
                        std::string& body, std::string& errorKind)
    {
        Poco::Net::HTTPResponse response;
        try
        {
            std::istream& rs = session->receiveResponse(response);
            std::ostringstream oss;
            Poco::StreamCopier::copyStream(rs, oss);
            body = oss.str();
            if (response.has("X-ERROR-KIND"))
                errorKind = response.get("X-ERROR-KIND");
            else
                errorKind.clear();
            return static_cast<int>(response.getStatus());
        }
        catch (const std::exception& ex)
        {
            TST_LOG("Exception reading response: " << ex.what());
            return -1;
        }
    }

    /// Test: password-protected ODS via convert-to returns HTTP 200 with PNG icon.
    bool testPasswordProtectedOds(std::unique_ptr<Poco::Net::HTTPClientSession>& session)
    {
        TST_LOG("testPasswordProtectedOds: start");

        sendConvertTo(session, "password-protected.ods");

        std::string body, errorKind;
        const int status = receiveResponse(session, body, errorKind);

        if (status != 200)
        {
            TST_LOG("testPasswordProtectedOds: expected HTTP 200, got " << status);
            return false;
        }

        // Check that we got a PNG image (magic bytes: 0x89 'P' 'N' 'G').
        if (body.size() < 8 ||
            static_cast<unsigned char>(body[0]) != 0x89 ||
            body[1] != 'P' || body[2] != 'N' || body[3] != 'G')
        {
            TST_LOG("testPasswordProtectedOds: response is not a PNG (size="
                    << body.size() << ")");
            return false;
        }

        // Verify the error kind header indicates a password was required.
        if (errorKind.find("passwordrequired") == std::string::npos)
        {
            TST_LOG("testPasswordProtectedOds: expected passwordrequired in X-ERROR-KIND, got ["
                    << errorKind << ']');
            return false;
        }

        TST_LOG("testPasswordProtectedOds: passed (got " << body.size() << " byte PNG)");
        return true;
    }

    /// Test: password-protected DOCX via convert-to returns HTTP 200 with PNG icon.
    bool testPasswordProtectedDocx(std::unique_ptr<Poco::Net::HTTPClientSession>& session)
    {
        TST_LOG("testPasswordProtectedDocx: start");

        sendConvertTo(session, "password-protected.docx");

        std::string body, errorKind;
        const int status = receiveResponse(session, body, errorKind);

        if (status != 200)
        {
            TST_LOG("testPasswordProtectedDocx: expected HTTP 200, got " << status);
            return false;
        }

        // Check PNG magic bytes.
        if (body.size() < 8 ||
            static_cast<unsigned char>(body[0]) != 0x89 ||
            body[1] != 'P' || body[2] != 'N' || body[3] != 'G')
        {
            TST_LOG("testPasswordProtectedDocx: response is not a PNG (size="
                    << body.size() << ")");
            return false;
        }

        if (errorKind.find("passwordrequired") == std::string::npos)
        {
            TST_LOG("testPasswordProtectedDocx: expected passwordrequired in X-ERROR-KIND, got ["
                    << errorKind << ']');
            return false;
        }

        TST_LOG("testPasswordProtectedDocx: passed (got " << body.size() << " byte PNG)");
        return true;
    }

    /// Test: corrupted document via convert-to returns HTTP 500 (not 502 timeout).
    bool testCorruptedDocument(std::unique_ptr<Poco::Net::HTTPClientSession>& session)
    {
        TST_LOG("testCorruptedDocument: start");

        sendConvertTo(session, "corrupted.odt");

        std::string body, errorKind;
        const int status = receiveResponse(session, body, errorKind);

        if (status == -1)
        {
            TST_LOG("testCorruptedDocument: connection error (possible 502 timeout)");
            return false;
        }

        if (status != 500)
        {
            TST_LOG("testCorruptedDocument: expected HTTP 500, got " << status);
            return false;
        }

        if (errorKind.empty())
        {
            TST_LOG("testCorruptedDocument: missing X-ERROR-KIND header");
            return false;
        }

        TST_LOG("testCorruptedDocument: passed (HTTP 500, errorKind=" << errorKind << ")");
        return true;
    }

public:
    UnitConvertToPasswordProtected()
        : UnitWSD("UnitConvertToPasswordProtected")
        , _workerStarted(false)
    {
        setHasKitHooks();
        setTimeout(1h);
    }

    ~UnitConvertToPasswordProtected()
    {
        LOG_INF("Joining test worker thread\n");
        _worker.join();
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        UnitWSD::configure(config);

        config.setBool("ssl.enable", true);
        config.setInt("per_document.limit_load_secs", 30);
        config.setBool("storage.filesystem[@allow]", false);
    }

    void invokeWSDTest() override
    {
        if (_workerStarted)
            return;
        _workerStarted = true;

        _worker = std::thread([this]{
            std::unique_ptr<Poco::Net::HTTPClientSession> session(
                helpers::createSession(Poco::URI(helpers::getTestServerURI())));
            session->setTimeout(Poco::Timespan(30, 0)); // 30 seconds.

            if (!testPasswordProtectedOds(session))
            {
                exitTest(TestResult::Failed);
                return;
            }

            // Recreate session for next request.
            session = helpers::createSession(Poco::URI(helpers::getTestServerURI()));
            session->setTimeout(Poco::Timespan(30, 0));

            if (!testPasswordProtectedDocx(session))
            {
                exitTest(TestResult::Failed);
                return;
            }

            session = helpers::createSession(Poco::URI(helpers::getTestServerURI()));
            session->setTimeout(Poco::Timespan(30, 0));

            if (!testCorruptedDocument(session))
            {
                exitTest(TestResult::Failed);
                return;
            }

            exitTest(TestResult::Ok);
        });
    }
};

// Inside the forkit & kit processes
class UnitKitConvertToPasswordProtected : public UnitKit
{
public:
    UnitKitConvertToPasswordProtected()
        : UnitKit("UnitKitConvertToPasswordProtected")
    {
        setTimeout(1h);
    }
};

UnitBase *unit_create_wsd(void)
{
    return new UnitConvertToPasswordProtected();
}

UnitBase *unit_create_kit(void)
{
    return new UnitKitConvertToPasswordProtected();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
