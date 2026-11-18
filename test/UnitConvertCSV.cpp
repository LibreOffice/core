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
 * Unit test for CSV convert-to with combined infilterOptions and options.
 * Verifies that infilterOptions (import) and options (export) are correctly
 * split and applied to the load and saveas commands respectively.
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
#include <Poco/Net/StringPartSource.h>
#include <Poco/StreamCopier.h>
#include <Poco/Util/LayeredConfiguration.h>

using namespace std::literals;

// Inside the WSD process
class UnitConvertCSV : public UnitWSD
{
    bool _workerStarted;
    std::thread _worker;

    /// Send a convert-to request with a CSV file and filter options.
    void sendConvertTo(std::unique_ptr<Poco::Net::HTTPClientSession>& session,
                       const std::string& csvContent,
                       const std::string& options = std::string(),
                       const std::string& infilterOptions = std::string())
    {
        Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_POST,
                                       "/cool/convert-to");
        Poco::Net::HTMLForm form;
        form.setEncoding(Poco::Net::HTMLForm::ENCODING_MULTIPART);
        form.set("format", "csv");
        if (!options.empty())
            form.set("options", options);
        if (!infilterOptions.empty())
            form.set("infilterOptions", infilterOptions);
        form.addPart("data",
                     new Poco::Net::StringPartSource(csvContent, "text/csv", "test.csv"));
        form.prepareSubmit(request);
        form.write(session->sendRequest(request));
    }

    /// Receive the convert-to response and return the body; empty on failure.
    std::string receiveConvertTo(std::unique_ptr<Poco::Net::HTTPClientSession>& session)
    {
        Poco::Net::HTTPResponse response;
        try
        {
            std::istream& rs = session->receiveResponse(response);
            std::ostringstream oss;
            Poco::StreamCopier::copyStream(rs, oss);
            if (response.getStatus() != Poco::Net::HTTPResponse::HTTP_OK)
            {
                TST_LOG("Convert-to failed: " << response.getStatus() << ' '
                                              << response.getReason());
                return std::string();
            }
            return oss.str();
        }
        catch (const std::exception& ex)
        {
            TST_LOG("Exception reading response: " << ex.what());
            return std::string();
        }
    }

    /// Test CSV-to-CSV conversion with both infilterOptions and options.
    /// Input: semicolon-separated CSV; Import option: FieldSeparator=59 (semicolon).
    /// Export option: FieldSeparator=9 (tab). Expected output: tab-separated CSV.
    bool testCsvSemicolonToTab(std::unique_ptr<Poco::Net::HTTPClientSession>& session)
    {
        TST_LOG("testCsvSemicolonToTab: start");

        // Semicolon-delimited input.
        const std::string input = "Name;Value;Note\nAlpha;100;first\nBeta;200;second\n";

        // Legacy CSV filter options: field_sep, text_delim, charset, start_row, ...
        // 59 = semicolon, 34 = double-quote, 76 = UTF-8, 1 = first row
        const std::string infilter = "59,34,76,1";
        // 9 = tab, 34 = double-quote, 76 = UTF-8, 1 = first row
        const std::string outfilter = "9,34,76,1";

        sendConvertTo(session, input, outfilter, infilter);
        std::string output = receiveConvertTo(session);

        if (output.empty())
        {
            TST_LOG("testCsvSemicolonToTab: empty response");
            return false;
        }

        // Verify tab characters are present in the output.
        if (output.find('\t') == std::string::npos)
        {
            TST_LOG("testCsvSemicolonToTab: no tab found in output: [" << output << "]");
            return false;
        }

        // Verify semicolons are NOT present as delimiters (they were in the input).
        // Note: semicolons could appear in quoted cell values, but our test data has none.
        if (output.find(';') != std::string::npos)
        {
            TST_LOG("testCsvSemicolonToTab: semicolon still in output: [" << output << "]");
            return false;
        }

        // Verify the actual data is preserved.
        if (output.find("Alpha") == std::string::npos ||
            output.find("100") == std::string::npos ||
            output.find("Beta") == std::string::npos)
        {
            TST_LOG("testCsvSemicolonToTab: data not preserved in output: [" << output << "]");
            return false;
        }

        TST_LOG("testCsvSemicolonToTab: passed");
        return true;
    }

    /// Test CSV conversion with only infilterOptions (no export options).
    /// The export should use default CSV settings.
    bool testCsvInfilterOnly(std::unique_ptr<Poco::Net::HTTPClientSession>& session)
    {
        TST_LOG("testCsvInfilterOnly: start");

        // Pipe-delimited input.
        const std::string input = "A|B|C\n1|2|3\n";

        // 124 = pipe character
        const std::string infilter = "124,34,76,1";

        sendConvertTo(session, input, /*options=*/std::string(), infilter);
        std::string output = receiveConvertTo(session);

        if (output.empty())
        {
            TST_LOG("testCsvInfilterOnly: empty response");
            return false;
        }

        // Data should be preserved.
        if (output.find('A') == std::string::npos ||
            output.find('1') == std::string::npos)
        {
            TST_LOG("testCsvInfilterOnly: data not preserved: [" << output << "]");
            return false;
        }

        TST_LOG("testCsvInfilterOnly: passed");
        return true;
    }

    /// Test CSV conversion with only export options (no infilterOptions).
    /// The import should auto-detect settings.
    bool testCsvOptionsOnly(std::unique_ptr<Poco::Net::HTTPClientSession>& session)
    {
        TST_LOG("testCsvOptionsOnly: start");

        // Comma-delimited input (standard CSV).
        const std::string input = "X,Y,Z\n10,20,30\n";

        // Export with tab separator.
        const std::string outfilter = "9,34,76,1";

        sendConvertTo(session, input, outfilter);
        std::string output = receiveConvertTo(session);

        if (output.empty())
        {
            TST_LOG("testCsvOptionsOnly: empty response");
            return false;
        }

        // Verify tab in output.
        if (output.find('\t') == std::string::npos)
        {
            TST_LOG("testCsvOptionsOnly: no tab in output: [" << output << "]");
            return false;
        }

        // Data should be preserved.
        if (output.find("10") == std::string::npos ||
            output.find("20") == std::string::npos)
        {
            TST_LOG("testCsvOptionsOnly: data not preserved: [" << output << "]");
            return false;
        }

        TST_LOG("testCsvOptionsOnly: passed");
        return true;
    }

public:
    UnitConvertCSV()
        : UnitWSD("UnitConvertCSV")
        , _workerStarted(false)
    {
        setHasKitHooks();
        setTimeout(1h);
    }

    ~UnitConvertCSV()
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
            session->setTimeout(Poco::Timespan(30, 0));

            if (!testCsvSemicolonToTab(session))
            {
                exitTest(TestResult::Failed);
                return;
            }

            // Create a fresh session for each test.
            session = helpers::createSession(Poco::URI(helpers::getTestServerURI()));
            session->setTimeout(Poco::Timespan(30, 0));

            if (!testCsvInfilterOnly(session))
            {
                exitTest(TestResult::Failed);
                return;
            }

            session = helpers::createSession(Poco::URI(helpers::getTestServerURI()));
            session->setTimeout(Poco::Timespan(30, 0));

            if (!testCsvOptionsOnly(session))
            {
                exitTest(TestResult::Failed);
                return;
            }

            exitTest(TestResult::Ok);
        });
    }
};

// Inside the forkit & kit processes
class UnitKitConvertCSV : public UnitKit
{
public:
    UnitKitConvertCSV()
        : UnitKit("UnitKitConvertCSV")
    {
        setTimeout(1h);
    }
};

UnitBase *unit_create_wsd(void)
{
    return new UnitConvertCSV();
}

UnitBase *unit_create_kit(void)
{
    return new UnitKitConvertCSV();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
