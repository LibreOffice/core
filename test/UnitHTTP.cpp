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
 * Unit test for HTTP protocol functionality.
 */

#include <config.h>

#include <helpers.hpp>
#include <Poco/Util/Application.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/Net/SecureStreamSocket.h>

#include <common/Log.hpp>
#include <common/Util.hpp>
#include <Unit.hpp>
#include <lokassert.hpp>

class UnitHTTP : public UnitWSD
{
public:
    UnitHTTP()
        : UnitWSD("UnitHTTP")
    {
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        UnitWSD::configure(config);
        // force HTTPS - to test harder
        config.setBool("ssl.enable", true);
    }

    void testContinue()
    {
        //FIXME: use logging
        TST_LOG("testContinue");
        for (int i = 0; i < 3; ++i)
        {
            auto httpSession = http::Session::create(helpers::getTestServerURI());

            std::string sent = "Hello world test\n";

            http::Request request("/cool/convert-to/txt", http::Request::VERB_POST);

            switch(i)
            {
            case 0:
                // No Expect header (default)
                break;
            case 1:
                request.set("Expect", "100-continue");
                break;
            default:
                break;
            }

            helpers::MultipartFormBody form;
            form.addField("format", "txt");
            form.addStringPart("data", sent, "text/plain", "foobaa.txt");
            form.applyTo(request);

            const auto httpResponse = httpSession->syncRequest(request);
            std::string responseStr(httpResponse->getBody());
            responseStr.erase(0,3); // remove utf-8 bom.

            if (sent != responseStr)
            {
                TST_LOG("Test " << i << " failed - mismatching string '" << responseStr << "' vs. '"
                                << sent << "'");
                exitTest(TestResult::Failed);
                return;
            }
        }
    }

    /// A conversion is refused when its format is not a single file extension.
    void testFormatIsOneExtension()
    {
        TST_LOG("testFormatIsOneExtension");
        auto httpSession = http::Session::create(helpers::getTestServerURI());

        http::Request request("/cool/convert-to", http::Request::VERB_POST);
        helpers::MultipartFormBody form;
        form.addField("format", "txt/txt");
        form.addStringPart("data", "Hello world test\n", "text/plain", "foobaa.txt");
        form.applyTo(request);

        const auto httpResponse = httpSession->syncRequest(request);
        LOK_ASSERT_EQUAL(http::StatusCode::BadRequest, httpResponse->statusCode());
    }

    void writeString(const std::shared_ptr<Poco::Net::StreamSocket> &socket, const std::string& str)
    {
        TST_LOG("Sending " << str.size() << " bytes:\n" << str);
        socket->sendBytes(str.c_str(), str.size());
    }

    bool expectString(const std::shared_ptr<Poco::Net::StreamSocket> &socket, const std::string& str)
    {
        TST_LOG("Expecting " << str.size() << " bytes:\n" << str);

        std::vector<char> buffer(str.size() + 64);
        const int got = socket->receiveBytes(buffer.data(), str.size());
        LOK_ASSERT_EQUAL(str, std::string(buffer.data(), got));

        if (got != (int)str.size() ||
            strncmp(buffer.data(), str.c_str(), got))
        {
            TST_LOG("testChunks got " << got << " mismatching strings '" << buffer.data()
                                      << " vs. expected '" << str << "'");
            exitTest(TestResult::Failed);
            return false;
        }
        else
            return true;
    }

    inline std::shared_ptr<Poco::Net::StreamSocket> createRawSocket()
    {
        auto socket =
#if ENABLE_SSL
            std::make_shared<Poco::Net::SecureStreamSocket>
#else
            std::make_shared<Poco::Net::StreamSocket>
#endif
            (Poco::Net::SocketAddress("127.0.0.1", ClientPortNumber));

        constexpr long seconds = 30;
        constexpr long microseconds = 0;
        socket->setReceiveTimeout(Poco::Timespan(seconds, microseconds));
        return socket;
    }

    void testChunks()
    {
        TST_LOG("testChunks");

        std::shared_ptr<Poco::Net::StreamSocket> socket = createRawSocket();

        writeString(
            socket,
            "POST /cool/convert-to/txt HTTP/1.1\r\n"
            "Host: localhost:9980\r\n"
            "User-Agent: cooltests/1.2.3\r\n"
            "Accept: */*\r\n"
            "Expect: 100-continue\r\n"
            "Transfer-Encoding: chunked\r\n"
            "Content-Type: multipart/form-data; "
            "boundary=------------------------5a0cd5c881663db4\r\n\r\n");
        if (!expectString(
                socket,
                "HTTP/1.1 100 Continue\r\n\r\n"))
            return;

#define START_CHUNK_HEX(len) len "\r\n"
#define END_CHUNK "\r\n"
        writeString(
            socket,
            START_CHUNK_HEX("8A")
            "--------------------------5a0cd5c881663db4\r\n"
            "Content-Disposition: form-data; name=\"data\"; filename=\"test.txt\"\r\n"
            "Content-Type: text/plain\r\n"
            "\r\n"
            END_CHUNK

            START_CHUNK_HEX("12")
            "This is some text."
            END_CHUNK

            START_CHUNK_HEX("1")
            "\n"
            END_CHUNK

            "  4 room:for expansion!! cf. leading spaces and nasties <>!\"\'?=)\r\n"
            "And "
            END_CHUNK

            START_CHUNK_HEX("1")
            "s"
            END_CHUNK

            START_CHUNK_HEX("a")
            "ome more.\n"
            END_CHUNK
            );
        writeString(
            socket,
            START_CHUNK_HEX("30")
            "\r\n"
            "--------------------------5a0cd5c881663db4--\r\n"
            END_CHUNK);

        writeString(socket, START_CHUNK_HEX("0"));

        TST_LOG("Receiving...");
        char buffer[4096] = { 0, };
        int got = socket->receiveBytes(buffer, 4096);

        http::Response httpResponse;
        LOK_ASSERT_MESSAGE("Expected to receive valid data",
                           httpResponse.readData(buffer, got) > 0);
        LOK_ASSERT(!httpResponse.statusLine().httpVersion().empty());
        LOK_ASSERT(!httpResponse.statusLine().reasonPhrase().empty());
        LOK_ASSERT_EQUAL(http::StatusCode::OK, httpResponse.statusLine().statusCode());
        LOK_ASSERT(httpResponse.statusLine().statusCategory() ==
                   http::StatusLine::StatusCodeClass::Successful);
        LOK_ASSERT_EQUAL_STR("HTTP/1.1", httpResponse.statusLine().httpVersion());
        LOK_ASSERT_EQUAL_STR("OK", httpResponse.statusLine().reasonPhrase());
        LOK_ASSERT_EQUAL_STR("attachment; filename=\"test.txt\"",
                             httpResponse.header().get("Content-Disposition"));

        // TODO: check content-length etc.

        const char *ptr = strstr(buffer, "\r\n\r\n");
        LOK_ASSERT_MESSAGE("Missing separator, got " + std::string(buffer), ptr);
        if (!ptr)
        {
            TST_LOG("missing separator " << got << " '" << buffer);
            exitTest(TestResult::Failed);
            return;
        }

        // Sometimes we get the content with the first receive.
        if (strstr(buffer, "\357\273\277This is some text.\nAnd some more.\n"))
        {
            return;
        }

        // Oddly we need another read to get the content.
        TST_LOG("Receiving...");
        got = socket->receiveBytes(buffer, 4096);
        LOK_ASSERT_MESSAGE("No content returned.", got >= 0);
        if (got >=0 )
            buffer[got] = '\0';
        else
        {
            TST_LOG("No content returned " << got);
            exitTest(TestResult::Failed);
            return;
        }

        if (buffer != std::string_view("\357\273\277This is some text.\nAnd some more.\n"))
        {
            TST_LOG("unexpected file content " << got << " '" << buffer);
            exitTest(TestResult::Failed);
            return;
        }
    }

    void invokeWSDTest() override
    {
        testChunks();
        testContinue();
        testFormatIsOneExtension();
        TST_LOG("All tests passed.");
        exitTest(TestResult::Ok);
    }
};

UnitBase *unit_create_wsd(void)
{
    return new UnitHTTP();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
