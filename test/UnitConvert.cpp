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
 * Unit test for document conversion functionality.
 */

#include <config.h>

#include <iostream>

#include <Common.hpp>
#include <Protocol.hpp>
#include <Unit.hpp>
#include <common/Util.hpp>
#include <common/FileUtil.hpp>
#include <helpers.hpp>

#include <Poco/Util/LayeredConfiguration.h>

using namespace std::literals;

// Inside the WSD process
class UnitConvert : public UnitWSD
{
    bool _workerStarted;
    std::thread _worker;
    std::shared_ptr<const http::Response> _lastResponse;

public:
    UnitConvert()
        : UnitWSD("UnitConvert")
        , _workerStarted(false)
    {
        setHasKitHooks();
        setTimeout(1h);
    }

    ~UnitConvert()
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

    void sendConvertTo(std::shared_ptr<http::Session>& session, const std::string& filename, bool isTemplate = false, bool isCompare = false)
    {
        std::string uri;
        if (isTemplate || isCompare)
            uri = "/cool/convert-to";
        else
            uri = "/cool/convert-to/pdf";

        http::Request request(uri, http::Request::VERB_POST);
        helpers::MultipartFormBody form;
        if (isTemplate)
        {
            form.addField("format", "html");
            std::string dataPath = Poco::Path(TDOC, filename).toString();
            form.addFile("data", dataPath);
            std::string templatePath = Poco::Path(TDOC, "template.docx").toString();
            form.addFile("template", templatePath);
        }
        else if (isCompare)
        {
            form.addField("format", "rtf");
            std::string dataPath = Poco::Path(TDOC, filename).toString();
            form.addFile("data", dataPath);
            std::string comparePath = Poco::Path(TDOC, "old.docx").toString();
            form.addFile("compare", comparePath);
        }
        else
        {
            form.addField("format", "txt");
            form.addStringPart("data", "Hello World Content", "text/plain", filename);
        }
        form.applyTo(request);
        _lastResponse = session->syncRequest(request);
    }

    bool checkConvertTo(bool isTemplate = false, bool isCompare = false)
    {
        if (!_lastResponse || _lastResponse->state() != http::Response::State::Complete)
            return false;

        bool ret = _lastResponse->statusCode() == http::StatusCode::OK;
        if (!ret || !(isTemplate || isCompare))
        {
            if (!ret)
            {
                TST_LOG("checkConvertTo: bad status");
            }

            return ret;
        }

        const std::string responseString(_lastResponse->getBody());
        if (isTemplate)
        {
            if (responseString.find("DOCTYPE html") == std::string::npos)
            {
                TST_LOG("checkConvertTo: output is not HTML");
                return false;
            }

            if (responseString.find("background: #156082") == std::string::npos)
            {
                TST_LOG("checkConvertTo: no template color in output");
                return false;
            }
        }
        else if (isCompare)
        {
            if (!responseString.starts_with("{\\rtf"))
            {
                TST_LOG("checkConvertTo: output is not RTF");
                return false;
            }

            if (responseString.find("\\deleted") == std::string::npos)
            {
                TST_LOG("checkConvertTo: no old content in output");
                return false;
            }
        }

        return true;
    }

    bool checkEmbeddedFontConvert(std::shared_ptr<http::Session>& session)
    {
        http::Request request("/cool/convert-to/pdf", http::Request::VERB_POST);
        helpers::MultipartFormBody form;
        form.addFile("data", Poco::Path(TDOC, "embedded_font.odt").toString());
        form.applyTo(request);
        _lastResponse = session->syncRequest(request);

        if (!_lastResponse || _lastResponse->state() != http::Response::State::Complete
            || _lastResponse->statusCode() != http::StatusCode::OK)
        {
            TST_LOG("checkEmbeddedFontConvert: bad status");
            return false;
        }

        // embedded_font.odt embeds "Caprasimo", which is neither installed in the jail nor a
        // common (substitutable) font, so its name appears in the output only if the kit
        // actually extracted and used the embedded font. We look for that name as plain text
        // in the PDF rather than parsing the file (the exporter writes /BaseFont uncompressed);
        // this mirrors the substring checks the other cases in this test already do on their
        // output.
        if (_lastResponse->getBody().find("Caprasimo") == std::string::npos)
        {
            TST_LOG("checkEmbeddedFontConvert: embedded font 'Caprasimo' missing from output; "
                    "it was substituted rather than extracted and used");
            return false;
        }
        return true;
    }

    void invokeWSDTest() override
    {
        if (_workerStarted)
            return;
        _workerStarted = true;
        std::cerr << "Starting thread ...\n";
        _worker = std::thread([this]{
                std::cerr << "Now started thread ...\n";
                auto session = http::Session::create(helpers::getTestServerURI());
                session->setTimeout(std::chrono::seconds(30));

                sendConvertTo(session, "foo.txt");
                if(!checkConvertTo())
                {
                    exitTest(TestResult::Failed);
                    return;
                }

                sendConvertTo(session, "test___á.txt");
                if(!checkConvertTo())
                {
                    exitTest(TestResult::Failed);
                    return;
                }

                // Test filename with bare percent character (not valid
                // percent-encoding).
                sendConvertTo(session, "he%llo.txt");
                if(!checkConvertTo())
                {
                    exitTest(TestResult::Failed);
                    return;
                }

                // Test filename with literal %25 (percent-encoded percent)
                // — ensures local paths treat '%' as literal, not URI encoding.
                sendConvertTo(session, "hello%25world.txt");
                if(!checkConvertTo())
                {
                    exitTest(TestResult::Failed);
                    return;
                }

                // Given a markdown input + docx template:
                // When converting that to HTML:
                sendConvertTo(session, "test.md", /*isTemplate=*/true);
                // Then make sure the output has a color from the template:
                // Without the accompanying fix in place, this test would have failed with:
                // checkConvertTo: no template color in output
                if(!checkConvertTo(/*isTemplate=*/true))
                {
                    exitTest(TestResult::Failed);
                    return;
                }

                // Given a current docx + old docx:
                // When comparing those documents and saving the result as RTF:
                sendConvertTo(session, "new.docx", /*isTemplate=*/false, /*isCompare=*/true);
                // Then make sure the output has content from the old document, too:
                // Without the accompanying fix in place, this test would have failed with:
                // checkConvertTo: no old content in output
                if(!checkConvertTo(/*isTemplate=*/false, /*isCompare=*/true))
                {
                    exitTest(TestResult::Failed);
                    return;
                }

                // Given a document that embeds a font not installed in the jail:
                // When converting it to PDF in the sandboxed kit:
                // Then the embedded font must be used, not substituted.
                // Without the accompanying fix in place, this test would have failed here:
                // the embedded fonts directory was resolved during the parent's preinit, so it
                // pointed outside the kit's sandbox and the font could not be extracted there.
                if (!checkEmbeddedFontConvert(session))
                {
                    exitTest(TestResult::Failed);
                    return;
                }

                exitTest(TestResult::Ok);
            });
    }
};

// Inside the forkit & kit processes
class UnitKitConvert : public UnitKit
{
public:
    UnitKitConvert()
        : UnitKit("UnitKitConvert")
    {
        setTimeout(1h);
    }
};

UnitBase *unit_create_wsd(void)
{
    return new UnitConvert();
}

UnitBase *unit_create_kit(void)
{
    return new UnitKitConvert();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
