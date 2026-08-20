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
 * White box unit test for HTTP request parsing and clipboard functionality.
 */

#include <config.h>

#include <string>

#include <common/Clipboard.hpp>
#include <net/HttpRequest.hpp>

#include <test/lokassert.hpp>

#include <cppunit/extensions/HelperMacros.h>

#include <cstdint>
#include <string>

/// HTTP WhiteBox unit-tests.
class HttpWhiteBoxTests : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(HttpWhiteBoxTests);

    CPPUNIT_TEST(testStatusLineParserValidComplete);
    CPPUNIT_TEST(testStatusLineParserValidComplete_NoReason);
    CPPUNIT_TEST(testStatusLineParserValidIncomplete);
    CPPUNIT_TEST(testStatusLineSerialize);

    CPPUNIT_TEST(testHeader);
    CPPUNIT_TEST(testHeaderFieldWithControlCharacter);
    CPPUNIT_TEST(testCookies);

    CPPUNIT_TEST(testRequestParserValidComplete);
    CPPUNIT_TEST(testRequestParserValidIncomplete);
    CPPUNIT_TEST(testRequestParserValidPostComplete);
    CPPUNIT_TEST(testRequestParserValidPostIncomplete);
    CPPUNIT_TEST(testRequestParserChunkedPostComplete);
    CPPUNIT_TEST(testClipboardIsOwnFormat);
    CPPUNIT_TEST(testMultiPartDataParser);
    CPPUNIT_TEST(testInsertFile);
    CPPUNIT_TEST(testGetFavicon);
    CPPUNIT_TEST(testPostWopi);

    CPPUNIT_TEST_SUITE_END();

    void testStatusLineParserValidComplete();
    void testStatusLineParserValidComplete_NoReason();
    void testStatusLineParserValidIncomplete();
    void testStatusLineSerialize();
    void testHeader();
    void testHeaderFieldWithControlCharacter();
    void testCookies();
    void testRequestParserValidComplete();
    void testRequestParserValidIncomplete();
    void testRequestParserValidPostComplete();
    void testRequestParserValidPostIncomplete();
    void testRequestParserChunkedPostComplete();
    void testClipboardIsOwnFormat();
    void testMultiPartDataParser();
    void testInsertFile();
    void testGetFavicon();
    void testPostWopi();
};

void HttpWhiteBoxTests::testStatusLineParserValidComplete()
{
    constexpr std::string_view testname = __func__;

    const unsigned expVersionMajor = 1;
    const unsigned expVersionMinor = 1;
    const std::string expVersion
        = "HTTP/" + std::to_string(expVersionMajor) + '.' + std::to_string(expVersionMinor);
    const http::StatusCode expStatusCode = http::StatusCode::SwitchingProtocols;
    const std::string expReasonPhrase = "Something Something";

    std::ostringstream oss;
    oss << expVersion << ' ' << static_cast<unsigned>(expStatusCode) << ' ' << expReasonPhrase
        << "\r\n";
    const std::string data = oss.str();

    http::StatusLine statusLine;

    int64_t len = data.size();
    LOK_ASSERT_EQUAL(http::FieldParseState::Valid, statusLine.parse(data.c_str(), len));
    LOK_ASSERT_EQUAL_STR(expVersion, statusLine.httpVersion());
    LOK_ASSERT_EQUAL(expVersionMajor, statusLine.versionMajor());
    LOK_ASSERT_EQUAL(expVersionMinor, statusLine.versionMinor());
    LOK_ASSERT_EQUAL(expStatusCode, statusLine.statusCode());
    LOK_ASSERT_EQUAL_STR(expReasonPhrase, statusLine.reasonPhrase());
}

void HttpWhiteBoxTests::testStatusLineParserValidComplete_NoReason()
{
    constexpr std::string_view testname = __func__;

    const unsigned expVersionMajor = 1;
    const unsigned expVersionMinor = 1;
    const std::string expVersion
        = "HTTP/" + std::to_string(expVersionMajor) + '.' + std::to_string(expVersionMinor);
    const http::StatusCode expStatusCode = http::StatusCode::SwitchingProtocols;
    const std::string expReasonPhrase;

    std::ostringstream oss;
    oss << expVersion << ' ' << static_cast<unsigned>(expStatusCode) << ' ' << expReasonPhrase
        << "\r\n";
    const std::string data = oss.str();

    http::StatusLine statusLine;

    int64_t len = data.size();
    LOK_ASSERT_EQUAL(http::FieldParseState::Valid, statusLine.parse(data.c_str(), len));
    LOK_ASSERT_EQUAL_STR(expVersion, statusLine.httpVersion());
    LOK_ASSERT_EQUAL(expVersionMajor, statusLine.versionMajor());
    LOK_ASSERT_EQUAL(expVersionMinor, statusLine.versionMinor());
    LOK_ASSERT_EQUAL(expStatusCode, statusLine.statusCode());
    LOK_ASSERT_EQUAL_STR(expReasonPhrase, statusLine.reasonPhrase());
}

void HttpWhiteBoxTests::testStatusLineParserValidIncomplete()
{
    constexpr std::string_view testname = __func__;

    const unsigned expVersionMajor = 1;
    const unsigned expVersionMinor = 1;
    const std::string expVersion
        = "HTTP/" + std::to_string(expVersionMajor) + '.' + std::to_string(expVersionMinor);
    const http::StatusCode expStatusCode = http::StatusCode::SwitchingProtocols;
    const std::string expReasonPhrase = "Something Something";

    std::ostringstream oss;
    oss << expVersion << ' ' << static_cast<unsigned>(expStatusCode) << ' ' << expReasonPhrase
        << "\r\n";
    const std::string data = oss.str();

    http::StatusLine statusLine;

    // Pass incomplete data to the reader.
    for (std::size_t i = 0; i < data.size(); ++i)
    {
        // Should return 0 to signify data is incomplete.
        int64_t len = i;
        LOK_ASSERT_EQUAL_MESSAGE("i = " + std::to_string(i), http::FieldParseState::Incomplete,
                                 statusLine.parse(data.c_str(), len));
    }

    int64_t len = data.size();
    LOK_ASSERT_EQUAL(http::FieldParseState::Valid, statusLine.parse(data.c_str(), len));
    LOK_ASSERT_EQUAL_STR(expVersion, statusLine.httpVersion());
    LOK_ASSERT_EQUAL(expVersionMajor, statusLine.versionMajor());
    LOK_ASSERT_EQUAL(expVersionMinor, statusLine.versionMinor());
    LOK_ASSERT_EQUAL(expStatusCode, statusLine.statusCode());
    LOK_ASSERT_EQUAL_STR(expReasonPhrase, statusLine.reasonPhrase());
}

void HttpWhiteBoxTests::testStatusLineSerialize()
{
    constexpr std::string_view testname = __func__;

    http::StatusLine statusLine(200);
    Buffer buf;
    statusLine.writeData(buf);
    const std::string out(buf.getBlock(), buf.getBlockSize());
    LOK_ASSERT_EQUAL_STR("HTTP/1.1 200 OK\r\n", out);
}

void HttpWhiteBoxTests::testHeader()
{
    constexpr std::string_view testname = __func__;

    http::Header header;

    const std::string data = "\r\na=\r\n\r\n";
    LOK_ASSERT_EQUAL(static_cast<std::int64_t>(8), header.parse(data.c_str(), data.size()));
    LOK_ASSERT_EQUAL(0UL, header.size());
}

void HttpWhiteBoxTests::testHeaderFieldWithControlCharacter()
{
    constexpr std::string_view testname = __func__;

    // A value that carries a carriage return and a newline does not become a second header,
    // and the caller is told the field was not set.
    http::Header header;
    LOK_ASSERT(!header.set("X-Sample", "one\r\nRange: bytes=0-1"));
    LOK_ASSERT(!header.has("X-Sample"));
    LOK_ASSERT(!header.has("Range"));
    LOK_ASSERT_EQUAL(0UL, header.size());

    // The same holds for a name, and for a lone newline.
    LOK_ASSERT(!header.add("X-Custom\r\nRange", "bytes=0-1"));
    LOK_ASSERT(!header.add("X-Custom", "value\nRange: bytes=0-1"));
    LOK_ASSERT_EQUAL(0UL, header.size());

    // A refused value takes the earlier one with it, so the field holds what the caller
    // passed or nothing at all.
    LOK_ASSERT(header.set("X-Replaced", "first"));
    LOK_ASSERT_EQUAL_STR("first", header.get("X-Replaced"));
    LOK_ASSERT(!header.set("X-Replaced", "second\r\nRange: bytes=0-1"));
    LOK_ASSERT(!header.has("X-Replaced"));
    LOK_ASSERT_EQUAL(0UL, header.size());

    // The other bytes RFC 9110 section 5.5 keeps out of a field value are refused too: NUL,
    // DEL, and the last control character below the printable range.
    LOK_ASSERT(!header.set("X-Nul", std::string("one\0two", 7)));
    LOK_ASSERT(!header.set("X-Del", "one\x7f" "two"));
    LOK_ASSERT(!header.set("X-UnitSeparator", "one\x1f" "two"));
    LOK_ASSERT_EQUAL(0UL, header.size());

    // The bytes at the edges of the allowed range are kept: HTAB, SP, 0x7e and 0x80.
    LOK_ASSERT(header.set("X-Tabbed", "one\ttwo"));
    LOK_ASSERT(header.set("X-Spaced", "one two"));
    LOK_ASSERT(header.set("X-Tilde", "one~two"));
    LOK_ASSERT(header.set("X-High", "one\x80" "two"));
    LOK_ASSERT(header.set("X-Accented", "caf\xc3\xa9"));
    LOK_ASSERT_EQUAL(5UL, header.size());
    LOK_ASSERT_EQUAL_STR("one\ttwo", header.get("X-Tabbed"));
    LOK_ASSERT_EQUAL_STR("one two", header.get("X-Spaced"));
    LOK_ASSERT_EQUAL_STR("one~two", header.get("X-Tilde"));
    LOK_ASSERT_EQUAL_STR("one\x80" "two", header.get("X-High"));
    LOK_ASSERT_EQUAL_STR("caf\xc3\xa9", header.get("X-Accented"));

    // The request that goes out on the wire carries no injected field.
    http::Request request;
    LOK_ASSERT(!request.set("X-Sample", "one\r\nIf-Modified-Since: Thu, 01 Jan 1970 00:00:00 GMT"));
    Buffer out;
    request.writeData(out, INT_MAX);
    const std::string wire(out.data(), out.size());
    LOK_ASSERT(wire.find("If-Modified-Since") == std::string::npos);
    LOK_ASSERT(wire.find("X-Sample") == std::string::npos);
}

void HttpWhiteBoxTests::testCookies()
{
    constexpr std::string_view testname = __func__;

    http::Header header;

    header.addCookie("Expire=Now");
    header.addCookie("Why=Question;When=Not");

    for (const auto& cookie : header.getCookies())
    {
        if (cookie.first == "Expire")
        {
            LOK_ASSERT_EQUAL_STR("Now", cookie.second);
        }
        else if (cookie.first == "Why")
        {
            LOK_ASSERT_EQUAL_STR("Question", cookie.second);
        }
        else if (cookie.first == "When")
        {
            LOK_ASSERT_EQUAL_STR("Not", cookie.second);
        }
    }
}

void HttpWhiteBoxTests::testRequestParserValidComplete()
{
    constexpr std::string_view testname = __func__;

    const std::string expVerb = "GET";
    const std::string expUrl = "/path/to/data";
    const std::string expVersion = "HTTP/1.1";
    const std::string data = expVerb + ' ' + expUrl + ' ' + expVersion + "\r\n" + "EmptyKey:\r\n"
                             + "Host: localhost.com\r\n\r\n";

    http::RequestParser req;

    LOK_ASSERT(req.readData(data.c_str(), data.size()) > 0);
    LOK_ASSERT_EQUAL_STR(expVerb, req.getVerb());
    LOK_ASSERT_EQUAL_STR(expUrl, req.getUrl());
    LOK_ASSERT_EQUAL_STR(expVersion, req.getVersion());
    LOK_ASSERT_EQUAL_STR(std::string(), req.get("emptykey"));
    LOK_ASSERT_EQUAL_STR("localhost.com", req.get("Host"));
    LOK_ASSERT_EQUAL(2UL, req.header().size());
}

void HttpWhiteBoxTests::testRequestParserValidIncomplete()
{
    constexpr std::string_view testname = __func__;

    const std::string expVerb = "GET";
    const std::string expUrl = "/long/path/to/data";
    const std::string expVersion = "HTTP/1.1";
    const std::string expHost = "localhost.com";
    const std::string data
        = expVerb + ' ' + expUrl + ' ' + expVersion + "\r\n" + "Host: " + expHost + "\r\n\r\n";

    http::RequestParser req;

    // Pass incomplete data to the reader.
    for (std::size_t i = 0; i < 33; ++i)
    {
        // Should return 0 to signify that data is incomplete.
        LOK_ASSERT_EQUAL_MESSAGE("i = " << i << " of " << data.size() - 1, static_cast<std::int64_t>(0),
                                 req.readData(data.c_str(), i));
    }

    // Offset of the end of first line.
    const int64_t off = 33;

    // Parse the first line.
    LOK_ASSERT_EQUAL_MESSAGE("Parsing the first line failed.", off,
                             req.readData(data.c_str(), off));
    LOK_ASSERT_EQUAL(http::Request::Stage::Header, req.stage());

    // Skip the first line and parse the header.
    for (std::size_t i = off; i < data.size(); ++i)
    {
        // Should return 0 to signify that data is incomplete.
        LOK_ASSERT_EQUAL_MESSAGE("i = " << i << " of " << data.size() - 1, static_cast<std::int64_t>(0),
                                 req.readData(data.c_str() + off, i - off));
    }

    // Parse the header.
    LOK_ASSERT_EQUAL_MESSAGE("Parsing the header failed.",
                             static_cast<int64_t>(expHost.size() + 10),
                             req.readData(data.c_str() + off, data.size() - off));

    LOK_ASSERT_EQUAL_STR(expVerb, req.getVerb());
    LOK_ASSERT_EQUAL_STR(expUrl, req.getUrl());
    LOK_ASSERT_EQUAL_STR(expVersion, req.getVersion());
    LOK_ASSERT_EQUAL_STR(expHost, req.get("Host"));
    LOK_ASSERT_EQUAL(1UL, req.header().size());
}

void HttpWhiteBoxTests::testRequestParserValidPostComplete()
{
    constexpr std::string_view testname = __func__;

    const std::string expVerb = "POST";
    const std::string expUrl = "/path/to/data";
    const std::string expVersion = "HTTP/1.1";
    constexpr std::string_view payload =
        "Some random string of data that has no particular purpose other "
        "than being a test payload.";

    std::ostringstream oss;
    oss << expVerb << ' ' << expUrl << ' ' << expVersion << "\r\n"
        << "EmptyKey:\r\n"
        << "Content-Length: " << payload.size() << "\r\n"
        << "Host: localhost.com\r\n\r\n"
        << payload;
    const std::string data = oss.str();

    http::RequestParser req;

    LOK_ASSERT(req.readData(data.c_str(), data.size()) > 0);
    LOK_ASSERT_EQUAL_STR(expVerb, req.getVerb());
    LOK_ASSERT_EQUAL_STR(expUrl, req.getUrl());
    LOK_ASSERT_EQUAL_STR(expVersion, req.getVersion());
    LOK_ASSERT_EQUAL_STR(std::string(), req.get("emptykey"));
    LOK_ASSERT_EQUAL_STR("localhost.com", req.get("Host"));
    LOK_ASSERT_EQUAL_STR(std::to_string(payload.size()), req.get("Content-Length"));
    LOK_ASSERT_EQUAL(3UL, req.header().size());
    LOK_ASSERT_EQUAL_STR(payload, req.getBody());
}

void HttpWhiteBoxTests::testRequestParserValidPostIncomplete()
{
    constexpr std::string_view testname = __func__;

    const std::string expVerb = "POST";
    const std::string expUrl = "/path/to/data";
    const std::string expVersion = "HTTP/1.1";
    constexpr std::string_view payload =
        "Some random string of data that has no particular purpose other "
        "than being a test payload.";

    std::ostringstream oss;
    oss << expVerb << ' ' << expUrl << ' ' << expVersion << "\r\n"
        << "EmptyKey:\r\n"
        << "Content-Length: " << payload.size() << "\r\n"
        << "Host: localhost.com\r\n\r\n"
        << payload;
    std::string data = oss.str();

    http::RequestParser req;

    // Pass incomplete data to the reader.
    const int64_t lenRequestLine = 29;
    for (std::size_t i = 0; i < lenRequestLine; ++i)
    {
        // Should return 0 to signify that data is incomplete.
        LOK_ASSERT_EQUAL(http::Request::Stage::RequestLine, req.stage());
        LOK_ASSERT_EQUAL_MESSAGE("i = " << i << " of " << data.size() - 1, 0L,
                                 req.readData(data.c_str(), i));
    }

    // Parse the request-line.
    LOK_ASSERT_EQUAL_MESSAGE("Parsing the request-line failed.", lenRequestLine,
                             req.readData(data.c_str(), lenRequestLine));
    LOK_ASSERT_EQUAL(http::Request::Stage::Header, req.stage());
    LOK_ASSERT_EQUAL_STR(expVerb, req.getVerb());
    LOK_ASSERT_EQUAL_STR(expUrl, req.getUrl());
    LOK_ASSERT_EQUAL_STR(expVersion, req.getVersion());

    // Continue by parsing the header. Simulate erasing read data (lenRequestLine).
    data = data.substr(lenRequestLine);

    // Pass incomplete data to the reader.
    const int64_t lenHeader = 54;
    for (std::size_t i = 0; i < lenHeader; ++i)
    {
        // Should return 0 to signify that data is incomplete.
        LOK_ASSERT_EQUAL(http::Request::Stage::Header, req.stage());
        LOK_ASSERT_EQUAL_MESSAGE("i = " << i << " of " << data.size() - 1, 0L,
                                 req.readData(data.c_str(), i));
    }

    // Parse the header.
    LOK_ASSERT_EQUAL_MESSAGE("Parsing the header failed.", lenHeader,
                             req.readData(data.c_str(), lenHeader));
    LOK_ASSERT_EQUAL(http::Request::Stage::Body, req.stage());

    LOK_ASSERT_EQUAL(std::string(), req.get("emptykey"));
    LOK_ASSERT_EQUAL_STR("localhost.com", req.get("Host"));
    LOK_ASSERT_EQUAL_STR(payload.size(), req.get("Content-Length"));
    LOK_ASSERT_EQUAL(3UL, req.header().size());

    // Continue by parsing the data. Simulate erasing read data (lenHeader).
    data = data.substr(lenHeader);
    for (int64_t i = 0; static_cast<uint64_t>(i) < data.size(); ++i)
    {
        // Should return the number of characters consumed.
        LOK_ASSERT_EQUAL(http::Request::Stage::Body, req.stage());
        LOK_ASSERT_EQUAL_MESSAGE("i = " << i << " of " << data.size() - 1, i,
                                 req.readData(data.c_str(), i));
        data = data.substr(i);
    }

    // Parse the data.
    LOK_ASSERT_EQUAL_MESSAGE("Parsing the data failed.", static_cast<int64_t>(data.size()),
                             req.readData(data.c_str(), data.size()));
    LOK_ASSERT_EQUAL(http::Request::Stage::Finished, req.stage());
    LOK_ASSERT_EQUAL_STR(payload, req.getBody());
}

void HttpWhiteBoxTests::testRequestParserChunkedPostComplete()
{
    constexpr std::string_view testname = __func__;

    const std::string expVerb = "POST";
    const std::string expUrl = "/";
    const std::string expVersion = "HTTP/1.1";
    constexpr std::string_view payload =
        "Some random string of data that has no particular purpose other "
        "than being a test payload.";

    constexpr std::string_view data = "POST / HTTP/1.1\r\n"
                                      "Transfer-Encoding: chunked\r\n"
                                      "Content-Type: text/html;charset=utf-8\r\n"
                                      "Content-Length: 90\r\n"
                                      "\r\n"
                                      "5a\r\n"
                                      "Some random string of data that has no particular purpose "
                                      "other than being a test payload.\r\n"
                                      "0\r\n"
                                      "\r\n";

    http::Request request;
    request.setVerb(http::Request::VERB_POST);
    request.set("Transfer-Encoding", "chunked");
    request.setBody(std::string(payload));
    Buffer out;
    request.writeData(out, INT_MAX);
    LOK_ASSERT_EQUAL_STR(data, std::string(out.data(), out.size()));

    http::RequestParser req;
    LOK_ASSERT(req.readData(data.data(), data.size()) > 0);
    LOK_ASSERT_EQUAL(expVerb, req.getVerb());
    LOK_ASSERT_EQUAL(expUrl, req.getUrl());
    LOK_ASSERT_EQUAL(expVersion, req.getVersion());
    LOK_ASSERT_EQUAL_STR("chunked", req.get("Transfer-Encoding"));
    LOK_ASSERT_EQUAL_STR("text/html;charset=utf-8", req.get("Content-Type"));
    LOK_ASSERT_EQUAL_STR(payload.size(), req.get("Content-Length"));
    LOK_ASSERT_EQUAL(3UL, req.header().size());
    LOK_ASSERT_EQUAL_STR(payload, req.getBody());
}

void HttpWhiteBoxTests::testClipboardIsOwnFormat()
{
    constexpr std::string_view testname = __func__;
    {
        std::string body = R"x(application/x-openoffice-embed-source-xml;windows_formatname="Star Embed Source (XML)"
1def
PK)x";
        std::istringstream stream(body);

        LOK_ASSERT_EQUAL(true, ClipboardData::isOwnFormat(stream));
    }
    {
        std::string body = R"(<!DOCTYPE html>
<html>
<head>)";
        std::istringstream stream(body);

        // This is expected to fail: format is mimetype-length-bytes tuples and here the second line
        // is not a hex size.
        LOK_ASSERT_EQUAL(false, ClipboardData::isOwnFormat(stream));
    }
}

/// Validate that the internal RequestParser correctly parses the given raw HTTP data.
static inline void comparePostContent(const std::string_view testname, const std::string_view data)
{
    http::RequestParser req;
    LOK_ASSERT(req.readData(data.data(), data.size()) > 0);

    // Validate basic request-line fields are non-empty and sensible.
    LOK_ASSERT(!req.getVerb().empty());
    LOK_ASSERT(!req.getUrl().empty());
    LOK_ASSERT_EQUAL_STR("HTTP/1.1", req.getVersion());
    LOK_ASSERT(!req.get("Host").empty());

    // Validate keep-alive detection.
    if (req.get("Connection") == "close")
        LOK_ASSERT_EQUAL(false, req.isKeepAlive());
    else
        LOK_ASSERT_EQUAL(true, req.isKeepAlive()); // HTTP/1.1 default

    // Validate all parsed headers have non-empty keys and retrievable values.
    std::size_t headerCount = 0;
    for (const auto& header : req.header())
    {
        ++headerCount;
        LOK_ASSERT(!header.first.empty());
        // Verify the value is retrievable via get().
        LOK_ASSERT_EQUAL_STR(header.second, req.get(header.first));
    }
    LOK_ASSERT_EQUAL(headerCount, req.header().size());
    LOK_ASSERT(headerCount > 0);
}

void HttpWhiteBoxTests::testMultiPartDataParser()
{
    constexpr std::string_view testname = __func__;

    const std::string header = "POST / HTTP/1.1\r\n"
                               "Content-type: multipart/mixed; boundary=\"simple boundary\"\r\n"
                               "\r\n";
    const std::string body =
        "This is the preamble.  It is to be ignored, though it\r\n"
        "is a handy place for composition agents to include an\r\n"
        "explanatory note to non-MIME conformant readers.\r\n"
        "\r\n"
        "--simple boundary\r\n"
        "Content-Disposition: file; name=\"attachment1\"; filename=\"att1.txt\"\r\n"
        "Content-Type: text/plain\r\n"
        "\r\n"
        "This is implicitly typed plain US-ASCII text.\r\n"
        "It does NOT end with a linebreak.\r\n"
        "--simple boundary\r\n"
        "Content-Disposition: file; name=\"attachment2\"; filename=\"att2.txt\"\r\n"
        "Content-type: text/plain; charset=us-ascii\r\n"
        "\r\n"
        "This is explicitly typed plain US-ASCII text.\r\n"
        "It DOES end with a linebreak.\r\n"
        "\r\n"
        "--simple boundary--\r\n"
        "\r\n"
        "This is the epilogue.  It is also to be ignored.\r\n"
        "\r\n";
    const std::string data = header + body;

    const std::string boundary = "simple boundary";

    const std::string firstContentDisposition = "file; name=\"attachment1\"; filename=\"att1.txt\"";
    const std::string firstContentType = "text/plain";
    const std::string firstPartData = "This is implicitly typed plain US-ASCII text.\r\n"
                                      "It does NOT end with a linebreak.";

    const std::string secondContentDisposition =
        "file; name=\"attachment2\"; filename=\"att2.txt\"";
    const std::string secondContentType = "text/plain; charset=us-ascii";
    const std::string secondPartData = "This is explicitly typed plain US-ASCII text.\r\n"
                                       "It DOES end with a linebreak.\r\n";

    // First, validate the test data by doing a full (non-incremental) parse.
    {
        http::MultipartDataParser fullParser(boundary);
        http::Header h;
        std::string_view partBody;
        std::string remaining(body);

        int64_t consumed = fullParser.readPart(remaining, h, partBody);
        LOK_ASSERT(consumed > 0);
        LOK_ASSERT_EQUAL(2UL, h.size());
        LOK_ASSERT_EQUAL(firstContentDisposition, h.get("Content-Disposition"));
        LOK_ASSERT_EQUAL(firstContentType, h.getContentType());
        LOK_ASSERT_EQUAL_STR(firstPartData, partBody);

        remaining = remaining.substr(static_cast<std::size_t>(consumed));
        h = http::Header();
        consumed = fullParser.readPart(remaining, h, partBody);
        LOK_ASSERT(consumed > 0);
        LOK_ASSERT_EQUAL(2UL, h.size());
        LOK_ASSERT_EQUAL(secondContentDisposition, h.get("Content-Disposition"));
        LOK_ASSERT_EQUAL(secondContentType, h.getContentType());
        LOK_ASSERT_EQUAL_STR(secondPartData, partBody);
        LOK_ASSERT(fullParser.isLast());
    }

    // Now test the incremental parsing.
    http::MultipartDataParser multipart(boundary);
    LOK_ASSERT_EQUAL_STR(boundary, multipart.boundary());

    // Incomplete until we reach the beginning of the *second* part.
    for (std::size_t i = 0; i < 456; ++i)
    {
        http::Header multipartHeader;
        std::string_view multipartBody;
        std::string fragment = data.substr(0, i);
        LOK_ASSERT_EQUAL_MESSAGE("At " << i << ": " << fragment
                                       << "\n----\nPart: " << multipartBody,
                                 0L, multipart.readPart(fragment, multipartHeader, multipartBody));
    }

    constexpr int64_t firstPartOffset = 435;
    constexpr int64_t secondPartOffset = 213;

    // Parse the first part.
    for (std::size_t i = 456; i < data.size(); ++i)
    {
        http::Header multipartHeader;
        std::string_view multipartBody;
        std::string fragment = data.substr(0, i);
        LOK_ASSERT_EQUAL(firstPartOffset,
                         multipart.readPart(fragment, multipartHeader, multipartBody));
        LOK_ASSERT_EQUAL(2UL, multipartHeader.size());
        LOK_ASSERT_EQUAL_STR(firstContentDisposition, multipartHeader.get("Content-Disposition"));
        LOK_ASSERT_EQUAL_STR(firstContentType, multipartHeader.getContentType());
        LOK_ASSERT_EQUAL_STR(firstPartData, multipartBody);
    }

    // Incomplete until we reach the beginning of the *last* part.
    for (std::size_t i = 0; i < 236; ++i)
    {
        http::Header multipartHeader;
        std::string_view multipartBody;
        std::string fragment = data.substr(firstPartOffset, i);
        LOK_ASSERT_EQUAL_MESSAGE("At " << i << ": " << fragment
                                       << "\n----\nPart: " << multipartBody,
                                 0L, multipart.readPart(fragment, multipartHeader, multipartBody));
    }

    // Parse the second part.
    {
        http::Header multipartHeader;
        std::string_view multipartBody;
        std::string fragment = data.substr(firstPartOffset, 236);
        LOK_ASSERT_EQUAL_MESSAGE("At " << 236, secondPartOffset,
                                 multipart.readPart(fragment, multipartHeader, multipartBody));
        LOK_ASSERT_EQUAL(2UL, multipartHeader.size());
        LOK_ASSERT_EQUAL_STR(secondContentDisposition, multipartHeader.get("Content-Disposition"));
        LOK_ASSERT_EQUAL_STR(secondContentType, multipartHeader.getContentType());
        LOK_ASSERT_EQUAL_STR(secondPartData, multipartBody);
        LOK_ASSERT(multipart.isLast());
    }

    // After the last part, everything is disposable.
    for (std::size_t i = 0; i < data.size(); ++i)
    {
        http::Header multipartHeader;
        std::string_view multipartBody;
        std::string fragment = data.substr(firstPartOffset + secondPartOffset, i);
        LOK_ASSERT_EQUAL_MESSAGE("At " << i << ": " << fragment
                                       << "\n----\nPart: " << multipartBody,
                                 static_cast<int64_t>(fragment.size()),
                                 multipart.readPart(fragment, multipartHeader, multipartBody));
        LOK_ASSERT_EQUAL(0UL, multipartHeader.size());
        LOK_ASSERT_EQUAL_STR(std::string(), multipartBody);
        LOK_ASSERT(multipart.isLast());
    }
}

void HttpWhiteBoxTests::testInsertFile()
{
    constexpr std::string_view testname = __func__;

    using namespace std::string_literals;

    const std::string header =
        "POST "
        "/cool/"
        "http%3A%2F%2Flocalhost%2Fnextcloud%2Findex.php%2Fapps%2Frichdocuments%2Fwopi%2Ffiles%"
        "2F6734_ocqiesh0cngs/"
        "insertfile?WOPISrc=http%3A%2F%2Flocalhost%2Fnextcloud%2Findex.php%2Fapps%2Frichdocuments%"
        "2Fwopi%2Ffiles%2F6734_ocqiesh0cngs&compat=/ws HTTP/1.1\r\n"
        "Host: localhost:9980\r\n"
        "Connection: keep-alive\r\n"
        "Content-Length: 2237\r\n"
        "sec-ch-ua-platform: \"Linux\"\r\n"
        "User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) "
        "Chrome/132.0.0.0 Safari/537.36\r\n"
        "sec-ch-ua: \"Not A(Brand\";v=\"8\", \"Chromium\";v=\"132\", \"Google "
        "Chrome\";v=\"132\"\r\n"
        "Content-Type: multipart/form-data; boundary=----WebKitFormBoundaryVNBIDV9swREloOQh\r\n"
        "sec-ch-ua-mobile: ?0\r\n"
        "Accept: */*\r\n"
        "Origin: http://localhost:9980\r\n"
        "Sec-Fetch-Site: same-origin\r\n"
        "Sec-Fetch-Mode: cors\r\n"
        "Sec-Fetch-Dest: empty\r\n"
        "Accept-Encoding: gzip, deflate, br, zstd\r\n"
        "Accept-Language: en-US,en;q=0.9\r\n"
        "\r\n";

    const std::string body =
        "------WebKitFormBoundaryVNBIDV9swREloOQh\r\n"
        "Content-Disposition: form-data; name=\"name\"\r\n"
        "\r\n"
        "1755424155510\r\n"
        "------WebKitFormBoundaryVNBIDV9swREloOQh\r\n"
        "Content-Disposition: form-data; name=\"childid\"\r\n"
        "\r\n"
        "OyQKcYniZdMUeuDl\r\n"
        "------WebKitFormBoundaryVNBIDV9swREloOQh\r\n"
        "Content-Disposition: form-data; name=\"file\"; filename=\"Screenshot from 2024-01-30 "
        "06-57-17.png\"\r\n"
        "Content-Type: image/png\r\n"
        "\r\n"
        "\211PNG\r\n"
        "\32\n"
        "\0\0\0\rIHDR\0\0\2\26\0\0\1~\10\2\0\0\0Qj\304."
        "\0\0\0\3sBIT\10\10\10\333\341O\340\0\0\0\31tEXtSoftware\0gnome-screenshot\357\3\277>"
        "\0\0\6\246IDATx\234\355\334\241\256\34U\34\300\341\273\313\205V "
        "\20\204'\300\21\34\10\22\236\1\203 "
        "\200F\3624H$\340\220<"
        "A\r\242\26Q\211\303\220\220\220\24\350\335\2313\210\206\252\222\206\337\356\236\345\336~"
        "\237\235\3159\1775\277\234\231\314\356\376zrs\5\0\377\335\376\322\3\0p["
        "I\10\0\221\204\0\20I\10\0\221\204\0\20I\10\0\221\204\0\20I\10\0\221\204\0\20I\10\0\221\204"
        "\0\20I\10\0\221\204\0\20I\10\0\221\204\0\20I\10\0\221\204\0\20I\10\0\221\204\0\20I\10\0"
        "\221\204\0\20I\10\0\221\204\0\20]_v\3731\306\30c\333\266\313\216\1@"
        "p\231\204\2141\226e\31c\254\353\272\375\343\"\223\0\220\315N\310\266m\313\262\34\16\207eY"
        "\306\272\216\355\351!D?\0n\237y\11\371\356\333o\246\355\5\300\4SO!\237~\366\371\277]"
        "\332\357\275\330\7\270e&%\344\331\243\252{\257\275\372\334\37H\10\300\2553\351\306}ss3g#"
        "\0\246\231\221\220eY\226e\231\260\21\03\315J\310\3410a#\0f:{B\326u]"
        "\327uY\235B\0\356\232I\11Y\327q\356\215\0\230\354\354\11\31c\214u\35CB\0\356\232\263'"
        "d\333\266\355j\223\20\200\273\347\274\337\205<"
        "\373\377\253\247\11\331\357\367\276\377\0\2703\334\320\1\210$\4\200HB\0\210$"
        "\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$"
        "\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$"
        "\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$"
        "\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$"
        "\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$"
        "\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$"
        "\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$"
        "\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$"
        "\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$"
        "\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$"
        "\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$"
        "\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$"
        "\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$"
        "\204\27\330\355v\227\36\1\370\237\222\20\0\"\11\1 \222\20\0\"\11\1 \222\20\0\"\11\1 "
        "\222\20\0\242\353K\17\300K\341\317\337\36mc9~\235\373o\274\275\177\345\376\361\353\0'!!"
        "\314\360\323\367\37\216\345\217\253\335Q\247\336qx\374\356'\17_"
        "\177\353\275SM\5\34IB\230\344\235\217\37\34y\367\177\370\365\233\247\32\68\11\357B\0\210$"
        "\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$"
        "\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200\350\372\322\3\360\2628<"
        "\376\345\311\357?\37\265\3046N3\n"
        "p\"\22\302$\217~\370\350\322#\0'&!\314\360\376\27\277^z\4\340\364\274\13\1 "
        "\222\20\0\"\17\2628\273\37\277\332\235p\265\17\276\334N\270\32p\14\247\20\0\"\247\20\316"
        "\316\271\1\356*\247\20\0\"\11\1 "
        "\222\20^`\333<\206\2\236OB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$"
        "\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$"
        "\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$"
        "\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$"
        "\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$"
        "\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$"
        "\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$"
        "\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$"
        "\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$"
        "\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$"
        "\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$"
        "\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210$"
        "\4\200HB\0\210$\4\200HB\0\210$\4\200HB\0\210\376\6\332E\243\373\267\206)"
        "\r\0\0\0\0IEND\256B`\202\r\n"
        "------WebKitFormBoundaryVNBIDV9swREloOQh--\r\n"s;

    const std::string data = header + body;

    http::RequestParser req;
    LOK_ASSERT(req.readData(data.data(), data.size()) > 0);
    LOK_ASSERT_EQUAL_STR(body, req.getBody());
    LOK_ASSERT_EQUAL(body.size(), req.getBody().size());

    comparePostContent(testname, data);
}

void HttpWhiteBoxTests::testGetFavicon()
{
    constexpr std::string_view testname = __func__;

    const std::string data = "GET /favicon.ico HTTP/1.1\r\n"
                             "Host: 127.0.0.1:9984\r\n"
                             "Date: Sat, 06 Sep 2025 12:37:58\r\n"
                             "User-Agent: COOLWSD HTTP Agent 25.04.5.1\r\n"
                             "\r\n";

    http::RequestParser req;
    LOK_ASSERT(req.readData(data.data(), data.size()) > 0);
    LOK_ASSERT_EQUAL_STR(std::string(), req.getBody());
    LOK_ASSERT_EQUAL_STR("127.0.0.1:9984", req.getHost());
    LOK_ASSERT_EQUAL_STR("Sat, 06 Sep 2025 12:37:58", req.get("Date"));
    LOK_ASSERT_EQUAL_STR("COOLWSD HTTP Agent 25.04.5.1", req.get("User-Agent"));
    LOK_ASSERT_EQUAL_STR("/favicon.ico", req.getUrl());
    LOK_ASSERT_EQUAL(http::RequestParser::Stage::Finished, req.stage());
    LOK_ASSERT_EQUAL(true, req.isKeepAlive()); // Because HTTP/1.1 is keep-alive by default.

    comparePostContent(testname, data);
}

void HttpWhiteBoxTests::testPostWopi()
{
    constexpr std::string_view testname = __func__;

    const std::string header = "POST "
                               "/wopi/files/UnitWOPIExpiredToken/"
                               "contents?access_token=anything&testname=UnitWOPIExpiredToken "
                               "HTTP/1.1\r\n"
                               "User-Agent: COOLWSD HTTP Agent 25.04.5.1\r\n"
                               "Authorization: Bearer anything\r\n"
                               "X-COOL-WOPI-ServerId: c5f25dc8\r\n"
                               "X-WOPI-Override: PUT\r\n"
                               "X-COOL-WOPI-IsModifiedByUser: true\r\n"
                               "X-COOL-WOPI-IsAutosave: false\r\n"
                               "X-COOL-WOPI-IsExitSave: true\r\n"
                               "Connection: close\r\n"
                               "X-COOL-WOPI-Timestamp: 2025-09-06T22:17:49.868004Z\r\n"
                               "Content-Type: application/octet-stream\r\n"
                               "Content-Length: 17\r\n"
                               "Host: 127.0.0.1:9981\r\n"
                               "Date: Sat, 06 Sep 2025 22:19:03\r\n"
                               "\r\n";
    const std::string body = "\357\273\277aHello, world\n";

    const std::string data = header + body;

    http::RequestParser req;
    LOK_ASSERT(req.readData(header.data(), header.size()) > 0);
    LOK_ASSERT_EQUAL_STR(std::string(), req.getBody());
    LOK_ASSERT_EQUAL_STR(http::RequestParser::name(http::RequestParser::Stage::Body),
                         http::RequestParser::name(req.stage()));
    LOK_ASSERT_EQUAL_STR("127.0.0.1:9981", req.getHost());
    LOK_ASSERT_EQUAL_STR("Sat, 06 Sep 2025 22:19:03", req.get("Date"));
    LOK_ASSERT_EQUAL_STR("COOLWSD HTTP Agent 25.04.5.1", req.get("User-Agent"));
    LOK_ASSERT_EQUAL_STR("/wopi/files/UnitWOPIExpiredToken/"
                         "contents?access_token=anything&testname=UnitWOPIExpiredToken",
                         req.getUrl());

    LOK_ASSERT_EQUAL_STR("Bearer anything", req.get("Authorization"));
    LOK_ASSERT_EQUAL_STR("c5f25dc8", req.get("X-COOL-WOPI-ServerId"));
    LOK_ASSERT_EQUAL_STR("PUT", req.get("X-WOPI-Override"));
    LOK_ASSERT_EQUAL_STR("true", req.get("X-COOL-WOPI-IsModifiedByUser"));
    LOK_ASSERT_EQUAL_STR("false", req.get("X-COOL-WOPI-IsAutosave"));
    LOK_ASSERT_EQUAL_STR("true", req.get("X-COOL-WOPI-IsExitSave"));
    LOK_ASSERT_EQUAL_STR("close", req.get("Connection"));
    LOK_ASSERT_EQUAL_STR("2025-09-06T22:17:49.868004Z", req.get("X-COOL-WOPI-Timestamp"));
    LOK_ASSERT_EQUAL_STR("application/octet-stream", req.get("Content-Type"));
    LOK_ASSERT_EQUAL_STR("17", req.get("Content-Length"));
    LOK_ASSERT_EQUAL(false, req.isKeepAlive());
}

CPPUNIT_TEST_SUITE_REGISTRATION(HttpWhiteBoxTests);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
