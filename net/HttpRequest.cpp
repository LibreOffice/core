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
 * Implementation of asynchronous HTTP/1.1 client with header parsing and state management.
 * Classes: http::Session, http::Request, http::Response, http::Header
 */

#include <config.h>

#include "HttpRequest.hpp"

#include <common/HexUtil.hpp>
#include <common/Log.hpp>
#include <common/NumUtil.hpp>
#include <common/Util.hpp>

#include <Poco/MemoryStream.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <sys/types.h>
#include <utility>

namespace
{
/// Returns true iff the character given is a whitespace.
/// FIXME: Technically, we should skip: SP, HTAB, VT (%x0B),
///         FF (%x0C), or bare CR.
inline bool isWhitespace(const char ch) { return ch == ' ' || ch == '\t' || ch == '\r'; }

/// Skips over space and tab characters starting at off.
/// Returns the offset of the first match, otherwise, len.
inline int64_t skipSpaceAndTab(const char* p, int64_t off, int64_t len)
{
    for (; off < len; ++off)
    {
        if (!isWhitespace(p[off]))
            return off;
    }

    return len;
}

inline int64_t skipCRLF(const char* p, int64_t off, int64_t len)
{
    for (; off < len; ++off)
    {
        if (p[off] != '\r' && p[off] != '\n')
            return off;
    }

    return len;
}

/// Find the line-break.
/// Returns the offset to the first LF character,
/// if found, otherwise, len.
/// Ex.: for [xxxCRLFCRLF] the offset to the second LF is returned.
inline int64_t findLineBreak(const char* p, int64_t off, int64_t len)
{
    // Find the line break, which ends the status line.
    for (; off < len; ++off)
    {
        // We expect CRLF, but LF alone is enough.
        if (p[off] == '\n')
            return off;
    }

    return len;
}

inline int64_t findLineBreak(const std::string_view data, int64_t off)
{
    return findLineBreak(data.data(), off, data.size());
}

/// Finds the double CRLF that signifies the end
/// of a block, such as a header. The second CRLF
/// is for a blank line, and that's what we seek.
inline int64_t findBlankLine(const char* p, int64_t off, int64_t len)
{
    for (; off < len;)
    {
        off = findLineBreak(p, off, len);
        // off is at the first LF, and we expect LFCRLF.
        if (off + 2 >= len)
        {
            return len; // Not found.
        }

        if (p[off + 1] == '\r' && p[off + 2] == '\n')
        {
            return off + 2; // Return the second LF.
        }

        off += 3; // Skip over the mismatch.
    }

    return len;
}

/// Find the end of text.
/// Returns the offset to the first whitespace or
/// line-break character if found, otherwise, len.
inline int64_t findEndOfToken(const char* p, int64_t off, int64_t len)
{
    for (; off < len; ++off)
    {
        if (isWhitespace(p[off]) || p[off] == '\n')
            return off;
    }

    return len;
}

} // namespace

namespace http
{

int64_t Header::parse(const char* p, int64_t len)
{
    LOG_TRC("Parsing header given " << len << " bytes: " << std::string(p, std::min<int64_t>(len, 80L)));
    if (len < 4)
    {
        // Incomplete; we need at least \r\n\r\n.
        return 0;
    }

    // Make sure we have the full header before parsing.
    const int64_t endPos = findBlankLine(p, 0, len);
    if (endPos == len)
    {
        return 0; // Incomplete.
    }

    try
    {
        //FIXME: implement http header parser!

        // Now parse to preserve folded headers and other
        // corner cases that is conformant to the rfc,
        // detecting any errors and/or invalid entries.
        // NB: request.read() expects full message and will fail.
        Poco::Net::MessageHeader msgHeader;
        Poco::MemoryInputStream data(p, len);
        msgHeader.read(data);
        if (data.tellg() < 0)
        {
            LOG_DBG("Failed to parse http header.");
            return -1;
        }

        // Copy the header entries over to us. The sender chose these bytes, so a field we
        // cannot carry is not our own error and is logged quietly.
        for (const auto& pair : msgHeader)
        {
            std::string name = Util::trimmed(pair.first);
            std::string value = Util::trimmed(pair.second);
            if (!hasOnlyValidFieldBytes(name) || !hasOnlyValidFieldBytes(value))
            {
                LOG_DBG("Skipping received header field with a byte a field cannot carry");
                continue;
            }

            set(name, std::move(value));
        }

        _chunked = getTransferEncoding() == "chunked";

        LOG_TRC("Read " << static_cast<std::size_t>(data.tellg())
                        << " bytes of header. hasContentLength: " << hasContentLength()
                        << ", contentLength: " << (hasContentLength() ? getContentLength() : -1)
                        << ", chunked: " << getChunkedTransferEncoding() << ":\n"
                        << std::string_view(p, data.tellg()));

        // We consumed the full header, including the blank line.
        return endPos + 1;
    }
    catch (const Poco::Exception& exc)
    {
        LOG_TRC("ERROR while parsing http header: " << exc.displayText());
    }

    return 0;
}

int64_t Header::getContentLength() const
{
    std::string contentLength = get(CONTENT_LENGTH);
    if (contentLength.empty() || contentLength[0] < '0' || contentLength[0] > '9')
    {
        return -1;
    }

    try
    {
        return std::stoll(contentLength);
    }
    catch (std::out_of_range&)
    {
        return -1;
    }
}

/// Parses a Status Line.
/// Returns the state and clobbers the len on success to the number of bytes read.
FieldParseState StatusLine::parse(const char* p, int64_t& len)
{
#ifdef DEBUG_HTTP
    LOG_TRC("StatusLine::parse: " << len << " bytes available\n"
                                  << HexUtil::dumpHex(std::string(p, std::min(len, 10 * 1024L))));
#endif //DEBUG_HTTP

    // First line is the status line.
    if (p == nullptr || len < MinStatusLineLen)
        return FieldParseState::Incomplete;

    int64_t off = skipSpaceAndTab(p, 0, len);
    if (off >= MaxStatusLineLen)
        return FieldParseState::Invalid;

    // We still expect the minimum amount of data.
    if ((len - off) < MinStatusLineLen)
        return FieldParseState::Incomplete;

    // We should have the version now.
    assert(off + VersionLen < len && "Expected to have more data.");
    const char* version = &p[off];
    constexpr int VersionMajPos = sizeof("HTTP/") - 1;
    constexpr int VersionDotPos = VersionMajPos + 1;
    constexpr int VersionMinPos = VersionDotPos + 1;
    constexpr int VersionBreakPos = VersionMinPos + 1; // Whitespace past the version.
    const int versionMaj = version[VersionMajPos] - '0';
    const int versionMin = version[VersionMinPos] - '0';
    // Version may not be null-terminated.
    if (!std::string(version, VersionLen).starts_with("HTTP/") ||
        (versionMaj < 0 || versionMaj > 9) || version[VersionDotPos] != '.' ||
        (versionMin < 0 || versionMin > 9) || !isWhitespace(version[VersionBreakPos]))
    {
        LOG_ERR("StatusLine::parse: Invalid HTTP version [" << std::string(version, VersionLen)
                                                            << "]");
        return FieldParseState::Invalid;
    }

    _httpVersion = std::string(version, VersionLen);
    _versionMajor = versionMaj;
    _versionMinor = versionMin;

    // Find the Status Code.
    off = skipSpaceAndTab(p, off + VersionLen, len);
    if (off >= MaxStatusLineLen)
        return FieldParseState::Invalid;

    // We still expect the Status Code and CRLF.
    if ((len - off) < (MinStatusLineLen - VersionLen))
        return FieldParseState::Incomplete;

    // Read the Status Code now.
    assert(off + StatusCodeLen < len && "Expected to have more data.");
    if (p[off] < '0' || p[off] > '9')
    {
        LOG_ERR("StatusLine::parse: expected valid integer number");
        return FieldParseState::Invalid;
    }

    bool res = false;
    std::tie(_statusCode, res) = NumUtil::u32FromString(std::string(&p[off], len - off));
    if (!res || _statusCode < MinValidStatusCode || _statusCode > MaxValidStatusCode)
    {
        LOG_ERR("StatusLine::parse: Invalid StatusCode [" << _statusCode << "]");
        return FieldParseState::Invalid;
    }

    // Find the Reason Phrase.
    off = skipSpaceAndTab(p, off + StatusCodeLen, len);
    if (off >= MaxStatusLineLen)
    {
        LOG_ERR("StatusLine::parse: StatusCode is too long: " << off);
        return FieldParseState::Invalid;
    }

    const int64_t reasonOff = off;

    // Find the line break, which ends the status line.
    off = findLineBreak(p, off, len);
    if (off >= len)
        return FieldParseState::Incomplete;

    for (; off < len; ++off)
    {
        if (p[off] == '\r' || p[off] == '\n')
            break;

        if (off >= MaxStatusLineLen)
        {
            LOG_ERR("StatusLine::parse: StatusCode is too long: " << off);
            return FieldParseState::Invalid;
        }
    }

    const int64_t stringSize = off - reasonOff - 1; // Exclude '\r'.
    if (stringSize > 0)
    {
        _reasonPhrase = std::string(&p[reasonOff], stringSize);
    }

    // Consume the line breaks.
    for (; off < len; ++off)
    {
        if (p[off] != '\r' && p[off] != '\n')
            break;
    }

    len = off;
    return FieldParseState::Valid;
}

bool Request::writeData(Buffer& out, std::size_t capacity)
{
    const std::size_t buffered_size = out.size();
    if (stage() == Stage::RequestLine)
    {
        if (!hasOnlyValidRequestLineBytes(getVerb()) || !hasOnlyValidRequestLineBytes(getUrl()) ||
            !hasOnlyValidRequestLineBytes(getVersion()))
        {
            LOG_ERR("Not sending a request whose request line holds a byte it cannot carry: ["
                    << HexUtil::stringifyHexLine(
                           getUrl(), 0, std::min(getUrl().size(), Header::MaxLoggedFieldBytes))
                    << ']');
            return false;
        }

        LOG_TRC("performWrites (request header)");

        out.append(getVerb());
        out.append(" ");
        out.append(getUrl());
        out.append(" ");
        out.append(getVersion());
        out.append("\r\n");

        header().writeData(out);
        out.append("\r\n"); // End the header.

        setStage(Stage::Body); // We've written both request-line and header.
    }

    if (stage() == Stage::Body)
    {
        LOG_TRC("performWrites (request body)");

        // Get the data to write into the socket
        // from the client's callback. This is
        // used to upload files, or other data.
        constexpr std::size_t BlockSize = 64 * 1024;
        std::size_t wrote = 0;
        const bool chunked =
            Util::toLower(get("transfer-encoding")).find("chunked") != std::string::npos;
        do
        {
            int64_t read;
            if (chunked)
            {
                // Chunked encoding needs to prepend the size header before the
                // data, so we must read into a temporary buffer first.
                char buffer[BlockSize];
                read = _bodyReaderCb(buffer, sizeof(buffer));
                if (read > 0)
                {
                    std::stringstream ss;
                    ss << std::hex << read;
                    out.append(ss.str());
                    out.append("\r\n");
                    out.append(buffer, read);
                    out.append("\r\n");
                }
            }
            else
            {
                // Read directly into the output buffer.
                const auto provisioned = std::min(BlockSize, capacity - wrote);
                char* buffer = out.provision(provisioned);
                read = _bodyReaderCb(buffer, provisioned);
                out.commit(provisioned, read > 0 ? read : 0);
            }

            if (read < 0)
            {
                LOG_ERR("Error reading the data to send as the HTTP request body: " << read);
                return false;
            }

            if (read == 0)
            {
                LOG_TRC("performWrites (request body): finished, total: " << out.size() -
                                                                                 buffered_size);
                setStage(Stage::Finished);
                if (chunked)
                {
                    out.append("0\r\n\r\n"); // Ending chunk.
                }

                break;
            }

            wrote += read;
            LOG_TRC("performWrites (request body): " << read << " bytes, total: "
                                                     << out.size() - buffered_size);
        } while (wrote < capacity);
    }

#ifdef DEBUG_HTTP
    LOG_TRC("Request::writeData: " << buffered_size << " bytes buffered\n"
                                   << HexUtil::dumpHex(out));
#endif //DEBUG_HTTP

    return true;
}

std::tuple<int64_t, int64_t, bool>
MultipartDataParser::findBoundary(const std::string_view data, const std::string_view delimiter,
                                  int64_t off)
{
    // Per RFC 2046, 5.1.1.  Common Syntax, we can have a preamble
    // between the header and the first boundary marker. So we
    // can and should skip anything until we hit a boundary.

    //  multipart-body := [preamble CRLF]
    //                    dash-boundary transport-padding CRLF
    //                    body-part *encapsulation
    //                    close-delimiter transport-padding
    //                    [CRLF epilogue]

    // Find the delimiter.
    const std::size_t pos = data.find(delimiter, off);
    if (pos == std::string::npos)
    {
        return { -1, -1, false }; // Not enough data.
    }

    // Expect at least 2 bytes after the delimiter, either CRLF
    // or '--' to signal last part.
    if (data.size() < pos + delimiter.size() + 2)
    {
        return { pos, 0, false }; // Incomplete.
    }

    //  close-delimiter := delimiter "--"
    const bool last =
        data[pos + delimiter.size()] == '-' && data[pos + delimiter.size() + 1] == '-';

    // Find the CRLF ending the boundary.
    off = findLineBreak(data, pos + delimiter.size());
    if (static_cast<std::size_t>(off) == data.size())
    {
        // If it's too long, fail with -1. Otherwise, 0 for incomplete.
        off = data.size() - pos - delimiter.size() > MaxLineLength ? -1 : 0;
    }

    return { pos, off, last };
}

int64_t MultipartDataParser::parsePart(std::string_view data, Header& header,
                                       std::string_view& body)
{
    if (isLast())
    {
        return data.size(); // Consume everything, since it's epilogue.
    }

    // The very first boundary could be at the very start of the body.
    // In that case, there will not be CRLF, because there is no preamble.
    // However, if that's not the case, there must be CRLF, so we search for that.
    const std::string_view delimiter =
        (_state == State::FirstPart && data.starts_with(_dashBoundary)) ? _dashBoundary
                                                                        : _delimiter;
    auto [start, end, last] = findBoundary(data, delimiter, 0);
    if (start < 0 || (!last && end == 0))
    {
        return 0; // Incomplete.
    }

    if (end < 0)
    {
        return -1; // Invalid.
    }

    ++end; // Skip the last char ('\n').

    // Find the *next* boundary, or closing one.
    auto [nextStart, nextEnd, nextLast] = findBoundary(data, _delimiter, end);
    if (nextStart < 0 || (!last && nextEnd == 0))
    {
        return 0; // Incomplete.
    }

    if (nextEnd < 0)
    {
        return -1; // Invalid.
    }

    _state = nextLast ? State::LastPart : State::NextPart;
    data = data.substr(end); // Skip the boundary.

    int64_t off = header.parse(data.data(), data.size());
    if (off <= 0)
    {
        return off; // Not enough or invalid data.
    }

    // The body is everything from the end of the header to
    // the beginning of the next boundary.
    body = std::string_view(data.data() + off, nextStart - end - off);
    return nextStart;
}

int64_t MultipartDataParser::readPart(std::string_view data, Header& header, std::string_view& body)
{
    return parsePart(data, header, body);
}

int64_t RequestParser::readData(const char* p, const int64_t len)
{
    uint64_t available = len;
    if (stage() == Stage::RequestLine)
    {
        // First line is the status line.
        // Fix infinite loop on mobile by skipping the minimum request header
        // length check
        if (p == nullptr || (len < MinRequestHeaderLen && !Util::isMobileApp()))
        {
            LOG_TRC("RequestParser::readData: len < MinRequestHeaderLen");
            return 0;
        }

        // Verb.
        uint64_t off = skipSpaceAndTab(p, 0, available);
        uint64_t end = findEndOfToken(p, off, available);
        if (end == available)
        {
            // Incomplete data.
            return 0;
        }

        setVerb(std::string(&p[off], end - off));

        // URL.
        off = skipSpaceAndTab(p, end, available);
        end = findEndOfToken(p, off, available);
        if (end == available)
        {
            // Incomplete data.
            return 0;
        }

        setUrl(std::string(&p[off], end - off));

        // Version.
        off = skipSpaceAndTab(p, end, available);
        if (off + VersionLen >= available)
        {
            // Incomplete data.
            return 0;
        }

        // We should have the version now.
        assert(off + VersionLen < available && "Expected to have more data.");
        const char* version = &p[off];
        constexpr int VersionMajPos = sizeof("HTTP/") - 1;
        constexpr int VersionDotPos = VersionMajPos + 1;
        constexpr int VersionMinPos = VersionDotPos + 1;
        constexpr int VersionBreakPos = VersionMinPos + 1; // Whitespace past the version.
        const int versionMaj = version[VersionMajPos] - '0';
        const int versionMin = version[VersionMinPos] - '0';
        // Version may not be null-terminated.
        if (!std::string(version, VersionLen).starts_with("HTTP/") ||
            (versionMaj < 0 || versionMaj > 9) || version[VersionDotPos] != '.' ||
            (versionMin < 0 || versionMin > 9) || !isWhitespace(version[VersionBreakPos]))
        {
            LOG_ERR("RequestParser::dataRead: Invalid HTTP version ["
                    << std::string(version, VersionLen) << "]");
            return -1;
        }

        setVersion(std::string(version, VersionLen));

        off += VersionLen;
        end = findLineBreak(p, off, available);
        if (end >= available)
        {
            // Incomplete data.
            return 0;
        }

        ++end; // Skip the LF character.

        // LOG_TRC("performWrites (header): " << headerStr.size() << ": " << headerStr);
        setStage(Stage::Header);
        p += end;
        available -= end;
    }

    if (stage() == Stage::Header)
    {
        const int64_t read = editHeader().parse(p, available);
        if (read < 0)
        {
            return read;
        }

        if (read > 0)
        {
            setStage(Stage::Body);
            p += read;
            available -= read;

#ifdef DEBUG_HTTP
            LOG_TRC("After Header: "
                    << available << " bytes availble\n"
                    << HexUtil::dumpHex(std::string(p, std::min(available, 1 * 1024UL))));
#endif //DEBUG_HTTP
        }
    }

    if (stage() == Stage::Body)
    {
        if (getVerb() == VERB_GET)
        {
            // A payload in a GET request "has no defined semantics".
            setStage(Stage::Finished);
            return len - available;
        }

        if (getVerb() == VERB_POST)
        {
            LOG_TRC("RequestParser::POST: " << available);

            if (!header().hasContentLength() && !header().getChunkedTransferEncoding())
            {
                LOG_ERR("HTTP POST request must provide either content-length or chunked "
                        "transfer-encoding");
                return -1; // Fail.
            }

            if (header().getChunkedTransferEncoding())
            {
                // This is a chunked transfer.
                // Find the start of the chunk, which is
                // the length of the chunk in hex.
                // each chunk is preceded by its length in hex.
                while (available)
                {
#ifdef DEBUG_HTTP
                    LOG_TRC("New Chunk, "
                            << available << " bytes available\n"
                            << HexUtil::dumpHex(std::string(p, std::min(available, 10 * 1024UL))));
#endif //DEBUG_HTTP

                    // Read ahead to see if we have enough data
                    // to consume the chunk length.
                    int64_t off = findLineBreak(p, 0, available);
                    if (off == static_cast<int64_t>(available))
                    {
                        LOG_TRC("Not enough data for chunk size");
                        // Not enough data.
                        return len - available; // Don't remove.
                    }

                    ++off; // Skip the LF itself.

                    // Read the chunk length.
                    int64_t chunkLen = 0;
                    std::int64_t chunkLenSize = 0;
                    for (; chunkLenSize < static_cast<int64_t>(available); ++chunkLenSize)
                    {
                        const int digit = HexUtil::hexDigitFromChar(p[chunkLenSize]);
                        if (digit < 0)
                            break;

                        // Can assume that digit is always less than 16.
                        if (chunkLen >= std::numeric_limits<int64_t>::max() / 16)
                        {
                            // Would not fit into chunkLen.
                            LOG_ERR("Unexpected chunk length: " << chunkLen);
                            return -1;
                        }
                        chunkLen = chunkLen * 16 + digit;
                    }

                    LOG_TRC("ChunkLen: " << chunkLen);
                    if (chunkLen > 0)
                    {
                        // Do we have enough data for this chunk?
                        if (static_cast<int64_t>(available) - off < chunkLen + 2) // + CRLF.
                        {
                            // Not enough data.
                            LOG_TRC("Not enough chunk data. Need "
                                    << chunkLen + 2 << " but have only " << available - off);
                            return len - available; // Don't remove.
                        }

                        // Skip the chunkLen bytes and any chunk extensions.
                        available -= off;
                        p += off;

                        const int64_t wrote = _onBodyWriteCb(p, chunkLen);
                        if (wrote != chunkLen)
                        {
                            LOG_ERR("Error writing http response payload. Write "
                                    "handler returned "
                                    << wrote << " instead of " << chunkLen);
                            return -1;
                        }

                        available -= chunkLen;
                        p += chunkLen;
                        _recvBodySize += chunkLen;
                        LOG_TRC("Wrote " << chunkLen << " bytes for a total of " << _recvBodySize);

                        // Skip blank lines.
                        off = skipCRLF(p, 0, available);
                        p += off;
                        available -= off;
                    }
                    else
                    {
                        // That was the last chunk!
                        setStage(Stage::Finished);
                        available = 0; // Consume all.
                        LOG_TRC("Got LastChunk, finished.");
                        break;
                    }
                }
            }
            else
            {
                // Non-chunked payload.
                // Write the body into the output, returns the
                // number of bytes read from the given buffer.
                const int64_t wrote = _onBodyWriteCb(p, available);
                if (wrote < 0)
                {
                    LOG_ERR("Error writing received http response payload into the body-callback. "
                            "Write handler returned "
                            << wrote << " instead of " << available);
                    return wrote;
                }

                if (wrote > 0)
                {
                    available -= wrote;
                    _recvBodySize += wrote;
                    if (header().hasContentLength() && _recvBodySize >= header().getContentLength())
                    {
                        LOG_TRC("Wrote all received content ("
                                << _recvBodySize << " bytes) into the body-callback, finished.");
                        setStage(Stage::Finished);
                    }
                }
            }

            return len - available;
        }

        // TODO: Implement HEAD support.
        LOG_ERR("Unsupported HTTP Method [" << getVerb() << ']');
        return -1;
    }

    return len - available;
}

/// Handles incoming data.
/// Returns the number of bytes consumed, or -1 for error
/// and/or to interrupt transmission.
int64_t Response::readData(const char* p, int64_t len)
{
    LOG_TRC("Response::readData: " << len << " bytes");

    // We got some data.
    _state = State::Incomplete;

    int64_t available = len;
    if (_parserStage == ParserStage::StatusLine)
    {
        int64_t read = available;
        switch (_statusLine.parse(p, read))
        {
            case FieldParseState::Unknown:
            case FieldParseState::Incomplete:
                return 0;
            case FieldParseState::Invalid:
                return -1;
            case FieldParseState::Valid:
                if (read <= 0)
                    return read; // Unexpected, really.
                if (read > 0)
                {
                    //FIXME: Don't consume what we read until we have our header parser.
                    // available -= read;
                    // p += read;
                    _parserStage = ParserStage::Header;
                }
                break;
        }
    }

    if (_parserStage == ParserStage::Header && available)
    {
        const int64_t read = _header.parse(p, available);
        if (read < 0)
        {
            return read;
        }

        if (read > 0)
        {
            available -= read;
            p += read;

#ifdef DEBUG_HTTP
            LOG_TRC("After Header: "
                    << available << " bytes available\n"
                    << HexUtil::dumpHex(std::string(p, std::min(available, 1 * 1024L))));
#endif //DEBUG_HTTP

            // Assume we have a body unless we have reason to expect otherwise.
            _parserStage = ParserStage::Body;

            if (_statusLine.statusCode() == http::StatusCode::Continue)
            {
                // 100 Continue is an intermediate response; the final response follows.
                // Reset parser state to read the actual final response.
                LOG_TRC("Got 100 Continue, resetting parser for final response");
                _statusLine = StatusLine();
                _header = Header();
                _parserStage = ParserStage::StatusLine;
                _recvBodySize = 0;
            }
            else if (_statusLine.statusCategory() == StatusLine::StatusCodeClass::Informational ||
                     _statusLine.statusCode() == http::StatusCode::NoContent ||
                     _statusLine.statusCode() == http::StatusCode::NotModified) // || HEAD request
            // || 2xx on CONNECT request
            {
                // No body, we are done (101 Switching Protocols, 204, 304, etc.).
                _parserStage = ParserStage::Finished;
            }
            else
            {
                // We can possibly have a body.
                if (_statusLine.statusCategory() != StatusLine::StatusCodeClass::Successful)
                {
                    // Failed: Store the body (if any) in memory.
                    saveBodyToMemory();
                }

                if (_header.hasContentLength())
                {
                    if (_header.getContentLength() < 0 || !_header.getTransferEncoding().empty())
                    {
                        // Invalid Content-Length or have Transfer-Encoding too.
                        // 3.3.2.  Content-Length
                        // A sender MUST NOT send a Content-Length header field in any message
                        // that contains a Transfer-Encoding header field.
                        LOG_ERR("Unexpected Content-Length header in response: "
                                << _header.getContentLength()
                                << ", Transfer-Encoding: " << _header.getTransferEncoding());
                        return -1;
                    }
                    else if (_bodySizeLimit > 0 && _header.getContentLength() > _bodySizeLimit)
                    {
                        LOG_ERR("Response Content-Length " << _header.getContentLength()
                                                           << " passes the body size limit of "
                                                           << _bodySizeLimit << " bytes");
                        return -1;
                    }
                    else if (_header.getContentLength() == 0)
                        _parserStage = ParserStage::Finished; // No body, we are done.
                }

                if (_parserStage != ParserStage::Finished)
                    _parserStage = ParserStage::Body;
            }
        }
    }

    if (_parserStage == ParserStage::Body && available)
    {
        LOG_TRC("ParserStage::Body: " << available);

        if (_header.getChunkedTransferEncoding())
        {
            // This is a chunked transfer.
            // Find the start of the chunk, which is
            // the length of the chunk in hex.
            // each chunk is preceded by its length in hex.
            while (available)
            {
#ifdef DEBUG_HTTP
                LOG_TRC("New Chunk, "
                        << available << " bytes available\n"
                        << HexUtil::dumpHex(std::string(p, std::min(available, 10 * 1024L))));
#endif //DEBUG_HTTP

                // Read ahead to see if we have enough data
                // to consume the chunk length.
                int64_t off = findLineBreak(p, 0, available);
                if (off == available)
                {
                    LOG_TRC("Not enough data for chunk size");
                    // Not enough data.
                    return len - available; // Don't remove.
                }

                ++off; // Skip the LF itself.

                // Read the chunk length.
                int64_t chunkLen = 0;
                std::int64_t chunkLenSize = 0;
                for (; chunkLenSize < available; ++chunkLenSize)
                {
                    const int digit = HexUtil::hexDigitFromChar(p[chunkLenSize]);
                    if (digit < 0)
                        break;

                    // Can assume that digit is always less than 16.
                    if (chunkLen >= std::numeric_limits<int64_t>::max() / 16)
                    {
                        // Would not fit into chunkLen.
                        LOG_ERR("Unexpected chunk length: " << chunkLen);
                        return -1;
                    }
                    chunkLen = chunkLen * 16 + digit;
                }

                LOG_TRC("ChunkLen: " << chunkLen);
                if (chunkLen > 0)
                {
                    // Do we have enough data for this chunk?
                    if (available - off < chunkLen + 2) // + CRLF.
                    {
                        // Not enough data.
                        LOG_TRC("Not enough chunk data. Need " << chunkLen + 2 << " but have only "
                                                               << available - off);
                        return len - available; // Don't remove.
                    }

                    // Skip the chunkLen bytes and any chunk extensions.
                    available -= off;
                    p += off;

                    const int64_t wrote = _onBodyWriteCb(p, chunkLen);
                    if (wrote != chunkLen)
                    {
                        LOG_ERR("Error writing http response payload. Write "
                                "handler returned "
                                << wrote << " instead of " << chunkLen);
                        return -1;
                    }

                    available -= chunkLen;
                    p += chunkLen;
                    _recvBodySize += chunkLen;
                    LOG_TRC("Wrote " << chunkLen << " bytes for a total of " << _recvBodySize);

                    if (_bodySizeLimit > 0 && _recvBodySize > _bodySizeLimit)
                    {
                        LOG_ERR("Response body of " << _recvBodySize
                                                    << " bytes passes the body size limit of "
                                                    << _bodySizeLimit << " bytes");
                        return -1;
                    }

                    // Skip blank lines.
                    off = skipCRLF(p, 0, available);
                    p += off;
                    available -= off;
                }
                else
                {
                    // That was the last chunk!
                    _parserStage = ParserStage::Finished;
                    available = 0; // Consume all.
                    LOG_TRC("Got LastChunk, finished.");
                    break;
                }
            }
        }
        else
        {
            // Non-chunked payload.
            // Write the body into the output, returns the
            // number of bytes read from the given buffer.
            const int64_t wrote = _onBodyWriteCb(p, available);
            if (wrote < 0)
            {
                LOG_ERR("Error writing received http response payload into the body-callback. "
                        "Write handler returned "
                        << wrote << " instead of " << available);
                return wrote;
            }

            if (wrote > 0)
            {
                available -= wrote;
                _recvBodySize += wrote;

                if (_bodySizeLimit > 0 && _recvBodySize > _bodySizeLimit)
                {
                    LOG_ERR("Response body of " << _recvBodySize
                                                << " bytes passes the body size limit of "
                                                << _bodySizeLimit << " bytes");
                    return -1;
                }

                if (_header.hasContentLength() && _recvBodySize >= _header.getContentLength())
                {
                    LOG_TRC("Wrote all received content into the body-callback, finished.");
                    _parserStage = ParserStage::Finished;
                }
            }
        }
    }

    if (_parserStage == ParserStage::Finished)
    {
        complete();
    }

    LOG_TRC("Done consuming response, had " << len << " bytes, consumed " << len - available
                                            << " leaving " << available << " unused.");
    return len - available;
}

std::shared_ptr<Session> Session::create(std::string host, Protocol protocol, int port)
{
    std::string scheme;
    std::string hostname;
    std::string portString;
    if (!net::parseUri(host, scheme, hostname, portString))
    {
        LOG_ERR_S("Invalid URI [" << host << "] to http::Session::create");
        throw std::runtime_error("Invalid URI [" + host + "] to http::Session::create.");
    }

    if (!scheme.empty())
    {
        switch (protocol)
        {
            case Protocol::HttpUnencrypted:
                assert((Util::iequal(scheme, "http://") || Util::iequal(scheme, "ws://")) &&
                       "createHttp has a conflicting scheme.");
                break;
            case Protocol::HttpSsl:
                assert((Util::iequal(scheme, "https://") || Util::iequal(scheme, "wss://")) &&
                       "createHttp has a conflicting scheme.");
                break;
        }
    }

    if (!hostname.empty())
        host.swap(hostname);

    if (!portString.empty())
    {
        const auto [portInt, res] = NumUtil::i32FromString(portString);
        assert((port == 0 || port == portInt) && "Two conflicting port numbers given.");
        if (res && portInt > 0)
            port = portInt;
    }

    port = (port > 0 ? port : getDefaultPort(protocol));
    return std::shared_ptr<Session>(new Session(std::move(host), protocol, port));
}

void Request::setBodyFile(const std::string& path)
{
    auto ifs = std::make_shared<std::ifstream>(path, std::ios::binary);

    ifs->seekg(0, std::ios_base::end);
    const int64_t size = ifs->tellg();
    ifs->seekg(0, std::ios_base::beg);

    setBodySource(
        [ifs = std::move(ifs)](char* buf, int64_t len) -> int64_t
        {
            ifs->read(buf, len);
            return ifs->gcount();
        },
        size);
}

void Request::setBody(std::string body, std::string contentType)
{
    if (!body.empty()) // Type is only meaningful if there is a body.
        editHeader().setContentType(std::move(contentType));

    editHeader().setContentLength(body.size());

    const size_t bodySize = body.size();

    auto iss = std::make_shared<std::istringstream>(std::move(body), std::ios::binary);

    setBodySource(
        [iss = std::move(iss)](char* buf, int64_t len) -> int64_t
        {
            iss->read(buf, len);
            return iss->gcount();
        },
        bodySize);
}

std::pair<std::string, std::string> Request::getBasicAuth() const
{
    const auto [scheme, param] = getCredentials();
    if (Util::iequal(scheme, "Basic"))
    {
        return Util::split(Util::base64Decode(param), ':');
    }

    return {};
}

void Response::saveBodyToFile(const std::string& path)
{
    _bodyFile.open(path, std::ios_base::out | std::ios_base::binary);
    if (!_bodyFile.good())
        LOG_ERR("Unable to open [" << path << "] for saveBodyToFile");
    _onBodyWriteCb = [this](const char* p, int64_t len)
    {
        LOG_TRC("Writing " << len << " bytes");
        if (_bodyFile.good())
            _bodyFile.write(p, len);
        return _bodyFile.good() ? len : -1;
    };
}

void Response::appendChunk(std::string_view chunk)
{
    assert(get("transfer-encoding").find("chunked") != std::string::npos &&
           "Expected to have chunked transfer-encoding header");
    assert(!_header.has("content-length") &&
           "Unexpected to have content-length header with transfer-encoding defined");

    _body.reserve(_body.size() + chunk.size() + 32);

    std::stringstream ss;
    ss << std::hex << chunk.size();
    _body.append(ss.str());
    _body.append("\r\n");
    _body.append(chunk);
    _body.append("\r\n");
}

bool Response::writeData(Buffer& out) const
{
    assert(!get("Date").empty() && "Date is always set in http::Response ctor");
    assert(get("Server") == http::getServerString() &&
           "Server Agent is always set in http::Response ctor");

    _statusLine.writeData(out);
    _header.writeData(out);
    out.append("\r\n"); // End of header.
    out.append(_body);
    return true;
}

void Response::dumpState(std::ostream& os, const std::string& indent) const
{
    os << indent << "http::Response: #" << _fd;
    os << indent << "\tstatusLine: " << _statusLine.httpVersion() << ' '
       << getReasonPhraseForCode(_statusLine.statusCode()) << ' ' << _statusLine.reasonPhrase();
    os << indent << "\tstate: " << name(_state);
    os << indent << "\tparseStage: " << name(_parserStage);
    os << indent << "\trecvBodySize: " << _recvBodySize;
    os << indent << "\tbodySizeLimit: " << _bodySizeLimit;
    os << indent << "\theaders: ";

    std::string childIndent = indent + '\t';
    Util::joinPair(os, _header, childIndent);
    os << indent;
    HexUtil::dumpHex(os, _body, "\tbody:\n", Util::replace(std::move(childIndent), "\n", "").c_str());
}

void Response::finish(State newState)
{
    if (!done())
    {
        LOG_TRC("Finishing: " << name(newState));
        _bodyFile.close();
        _state = newState;
        if (_finishedCallback)
            _finishedCallback();
    }
}

Session::Session(std::string hostname, Protocol protocolType, int portNumber)
    : _host(std::move(hostname))
    , _port(std::to_string(portNumber))
    , _protocol(protocolType)
    , _fd(-1)
    , _handshakeSslVerifyFailure(0)
    , _timeout(getDefaultTimeout())
    , _connected(false)
    , _asyncShutdownOnFinish(false)
    , _result(net::AsyncConnectResult::Ok)
{
    assert(!_host.empty() && portNumber > 0 && !_port.empty() &&
           "Invalid hostname and portNumber for http::Sesssion");

    if constexpr (Util::isDebugEnabled())
    {
        std::string scheme;
        std::string hostString;
        std::string portString;
        assert(net::parseUri(_host, scheme, hostString, portString) && scheme.empty() &&
               portString.empty() && hostString == _host &&
               "http::Session expects a hostname and not a URI");
    }
}

const char* Session::getProtocolScheme(Protocol protocol)
{
    switch (protocol)
    {
        case Protocol::HttpUnencrypted:
            return "http";
        case Protocol::HttpSsl:
            return "https";
    }

    return "";
}

std::shared_ptr<Session> Session::create(const std::string& uri)
{
    std::string scheme;
    std::string hostname;
    std::string portString;
    if (!net::parseUri(uri, scheme, hostname, portString))
    {
        LOG_ERR_S("Invalid URI [" << uri << "] to http::Session::create");
        return nullptr;
    }

    const bool secure = (Util::iequal(scheme, "https://") || Util::iequal(scheme, "wss://"));
    const auto protocol = secure ? Protocol::HttpSsl : Protocol::HttpUnencrypted;
    if (portString.empty())
        return create(std::move(hostname), protocol, getDefaultPort(protocol));

    const auto [port, success] = NumUtil::i32FromString(portString);
    if (success && port > 0)
        return create(std::move(hostname), protocol, port);

    LOG_ERR_S("Invalid port [" << portString << "] in URI [" << uri
                               << "] to http::Session::create");
    return nullptr;
}

int Session::getDefaultPort(Protocol protocol)
{
    switch (protocol)
    {
        case Protocol::HttpUnencrypted:
            return 80;
        case Protocol::HttpSsl:
            return 443;
    }

    return 0;
}

std::shared_ptr<const Response>
Session::syncDownload(const Request& req, const std::string& saveToFilePath, SocketPoll& poller,
                      int64_t bodySizeLimit)
{
    LOG_TRC_S("syncDownload: " << req.getVerb() << ' ' << host() << ':' << port() << ' '
                               << req.getUrl());

    newRequest(req, false);

    if (!saveToFilePath.empty())
        _response->saveBodyToFile(saveToFilePath);
    _response->setBodySizeLimit(bodySizeLimit);

    syncRequestImpl(poller);
    return _response;
}

std::shared_ptr<const Response> Session::syncRequest(const Request& req,
                                                     std::chrono::milliseconds timeout)
{
    LOG_TRC("syncRequest: " << req.getVerb() << ' ' << host() << ':' << port() << ' '
                            << req.getUrl());

    const auto origTimeout = getTimeout();
    setTimeout(timeout);

    auto responsePtr = syncRequest(req);

    setTimeout(origTimeout);

    return responsePtr;
}

#if !MOBILEAPP

bool Session::asyncRequest(const Request& req, const std::weak_ptr<SocketPoll>& poll,
                           bool asyncShutdownOnFinish)
{
    std::shared_ptr<SocketPoll> socketPoll(poll.lock());
    if (!socketPoll)
    {
        LOG_ERR("Cannot start new asyncRequest without a valid SocketPoll: "
                << req.getVerb() << ' ' << host() << ':' << port() << ' ' << req.getUrl());

        if (_onConnectFail)
        {
            // Call directly since we haven't started the async
            // connect to pass the validation in callOnConnectFail().
            _onConnectFail(shared_from_this());
        }

        return false;
    }

    LOG_TRC("New asyncRequest on [" << socketPoll->name() << "]: " << req.getVerb() << ' '
                                    << host() << ':' << port() << ' ' << req.getUrl());

    newRequest(req, asyncShutdownOnFinish);

    if (!isConnected())
    {
        asyncConnect(poll);
    }
    else
    {
        // Technically, there is a race here. The socket can
        // get disconnected and removed right after isConnected.
        // In that case, we will timeout and no request will be sent.
        socketPoll->wakeup();
    }

    LOG_DBG("Starting asyncRequest on [" << socketPoll->name() << "]: " << req.getVerb() << ' '
                                         << host() << ':' << port() << ' ' << req.getUrl());
    return true;
}

#endif // !MOBILEAPP

std::string Session::getSslVerifyMessage() const
{
#if ENABLE_SSL
    std::shared_ptr<StreamSocket> socket = _socket.lock();
    if (socket)
        return SslStreamSocket::getSslVerifyString(socket->getSslVerifyResult());
    return SslStreamSocket::getSslVerifyString(_handshakeSslVerifyFailure);
#else
    return std::string();
#endif
}

long Session::getSslVerifyResult() const
{
#if ENABLE_SSL
    std::shared_ptr<StreamSocket> socket = _socket.lock();
    if (socket)
        return socket->getSslVerifyResult();
    return _handshakeSslVerifyFailure;
#else
    return 0; // X509_V_OK
#endif
}

std::string Session::getSslCert(std::string& subjectHash) const
{
#if ENABLE_SSL
    std::shared_ptr<StreamSocket> socket = _socket.lock();
    if (socket)
        return socket->getSslCert(subjectHash);
#else
    (void) subjectHash;
#endif
    return std::string();
}

void Session::dumpState(std::ostream& os, const std::string& indent) const
{
    const auto now = std::chrono::steady_clock::now();
    os << indent << "http::Session: #" << _fd << " (" << (_socket.lock() ? "have" : "no")
       << " socket)";
    os << indent << "\tconnected: " << _connected;
    os << indent << "\tasyncShutdownOnFinish: " << _asyncShutdownOnFinish;
    os << indent << "\ttimeout: " << _timeout;
    os << indent << "\thost: " << _host;
    os << indent << "\tport: " << _port;
    os << indent << "\tprotocol: " << name(_protocol);
    os << indent << "\taddressFilter: " << (_addressFilter ? "set" : "none");
    os << indent << "\thandshakeSslVerifyFailure: " << _handshakeSslVerifyFailure;
    os << indent << "\tstartTime: " << Util::getTimeForLog(now, _startTime);
    _request.dumpState(os, indent + '\t');
    if (_response)
        _response->dumpState(os, indent + '\t');
    else
        os << indent << "\tresponse: null";

    os << '\n';

    // We are typically called from the StreamSocket, so don't
    // recurse back by calling dumpState on the socket again.
}

bool Session::syncRequestImpl(SocketPoll& poller)
{
    const std::chrono::microseconds timeout = getTimeout();
    const auto deadline = std::chrono::steady_clock::now() + timeout;

    assert(!!_response && "Response must be set!");

    if (!isConnected())
    {
        std::shared_ptr<StreamSocket> socket = connect();
        if (!socket)
        {
            LOG_ERR("Failed to connect to " << _host << ':' << _port);
            return false;
        }

        poller.insertNewSocket(socket);
    }

    LOG_TRC("Starting syncRequest: " << _request.getVerb() << ' ' << host() << ':' << port()
                                     << ' ' << _request.getUrl());

    poller.poll(timeout);
    while (!_response->done())
    {
        const auto now = std::chrono::steady_clock::now();
        if (checkTimeout(now))
            return false;

        const auto remaining =
            std::chrono::duration_cast<std::chrono::microseconds>(deadline - now);
        poller.poll(remaining);
    }

    return _response->state() == Response::State::Complete;
}

void Session::callOnFinished()
{
    if (_asyncShutdownOnFinish)
        asyncShutdown();

    if (!_onFinished)
        return;

    LOG_TRC("onFinished calling client");
    std::shared_ptr<Session> self = shared_from_this();
    try
    {
        [[maybe_unused]] const long references = self.use_count();
        assert(references > 1 && "Expected more than 1 reference to http::Session.");

        _onFinished(self);

        assert(self.use_count() > 1 &&
                "Erroneously onFinish reset 'this'. Use 'addCallback()' on the "
                "SocketPoll to reset on idle instead.");
    }
    catch (const std::exception& exc)
    {
        LOG_ERR("Error while invoking onFinished client callback: " << exc.what());
    }
}

void Session::newRequest(const Request& req, bool asyncShutdownOnFinish)
{
    _startTime = std::chrono::steady_clock::now();

    // Called when the response is finished.
    // We really need only delegate it to our client.
    // We need to do this extra hop because Response
    // doesn't have our (Session) reference. Also,
    // it's good that we are notified that the request
    // has retired, so we can perform housekeeping.
    Response::FinishedCallback onFinished = [this]()
    {
        LOG_TRC("onFinished");
        assert(_response && "Must have response object");
        assert(_response->state() != Response::State::New &&
               "Unexpected response in New state");
        assert(_response->state() != Response::State::Incomplete &&
               "Unexpected response in Incomplete state");
        assert(_response->done() && "Must have response in done state");

        callOnFinished();

        if (_response->header().getConnectionToken() == Header::ConnectionToken::Close)
        {
            LOG_TRC("Our peer has sent the 'Connection: close' token. Disconnecting.");
            onDisconnect();
            assert(isConnected() == false);
        }
    };

    _response.reset();
    _response = std::make_shared<Response>(onFinished, _fd);

    _request = req;

    _asyncShutdownOnFinish = asyncShutdownOnFinish;

    // Bracket an IPv6 literal so the Host header is well-formed, e.g.
    // "[::1]:9980". _host is a bare host (no scheme, no port), so a colon
    // in it can only be part of an IPv6 address.
    const bool isIPv6 = _host.find(':') != std::string::npos;
    std::string host = isIPv6 ? '[' + _host + ']' : _host;

    if (_port != "80" && _port != "443")
    {
        host.push_back(':');
        host.append(_port);
    }
    _request.set("Host", std::move(host)); // Make sure the host is set.
    _request.set("Date", Util::getHttpTimeNow());
    _request.set("User-Agent", http::getAgentString());
}

void Session::onConnect(const std::shared_ptr<StreamSocket>& socket)
{
    ASSERT_CORRECT_THREAD();

    if (socket)
    {
        _fd = socket->getFD();
        _response->setLogContext(_fd);
        LOG_TRC("Connected");
        _connected = true;
    }
    else
    {
        LOG_DBG("Error: onConnect without a valid socket");
        _fd = -1;
        _handshakeSslVerifyFailure = 0;
        _connected = false;
    }
}

void Session::getIOStats(uint64_t& sent, uint64_t& recv)
{
    LOG_TRC("getIOStats");
    std::shared_ptr<StreamSocket> socket = _socket.lock();
    if (socket)
        socket->getIOStats(sent, recv);
    else
    {
        sent = 0;
        recv = 0;
    }
}

void Session::handleIncomingMessage(SocketDisposition& disposition)
{
    LOG_TRC("handleIncomingMessage");
    ASSERT_CORRECT_THREAD();
    std::shared_ptr<StreamSocket> socket = _socket.lock();
    if (isConnected() && socket)
    {
        // Consume the incoming data by parsing and processing the body.
        Buffer& data = socket->getInBuffer();
        if (data.empty())
        {
            LOG_DBG("No data to process from the socket");
            return;
        }

        LOG_TRC("HandleIncomingMessage: buffer has:\n"
                << HexUtil::dumpHex(
                       std::string(data.data(), std::min<size_t>(data.size(), 256UL))));

#if !(defined QTAPP || defined _WIN32 || defined(MACOS))
        // Response::readData is not build with CODA.
        const int64_t read = _response->readData(data.data(), data.size());
        if (read >= 0)
        {
            // Remove consumed data.
            if (read)
                data.eraseFirst(read);
            return;
        }
#endif
    }
    else
    {
        LOG_ERR("handleIncomingMessage called when not connected");
        assert(!socket && "Expected no socket when not connected");
        assert(!isConnected() && "Expected not connected when no socket");
    }

    // Protocol error: Interrupt the transfer.
    disposition.setClosed();
    onDisconnect();
}

void Session::performWrites(std::size_t capacity)
{
    ASSERT_CORRECT_THREAD();
    // We may get called after disconnecting and freeing the Socket instance.
    std::shared_ptr<StreamSocket> socket = _socket.lock();
    if (socket)
    {
        Buffer& out = socket->getOutBuffer();
        LOG_TRC("performWrites: sending request (buffered: "
                << out.size() << " bytes, capacity: " << capacity << ')');

#if !(defined QTAPP || defined _WIN32 || defined(MACOS))
        // StreamSocket::send(...) is excluded in CODA.
        if (!socket->send(_request))
        {
            _result = net::AsyncConnectResult::SocketError;
            LOG_ERR("Error while writing to socket");
        }
#endif
    }
}

void Session::callOnConnectFail()
{
    if (!_onConnectFail)
        return;

    std::shared_ptr<Session> self = shared_from_this();
    try
    {
        [[maybe_unused]] const long references = self.use_count();
        assert(references > 1 && "Expected more than 1 reference to http::Session.");

        _onConnectFail(self);

        assert(self.use_count() > 1 &&
                "Erroneously onConnectFail reset 'this'. Use 'addCallback()' on the "
                "SocketPoll to reset on idle instead.");
    }
    catch (const std::exception& exc)
    {
        LOG_ERR("Error while invoking onConnectFail client callback: " << exc.what());
    }
}

void Session::onHandshakeFail()
{
    ASSERT_CORRECT_THREAD();
    std::shared_ptr<StreamSocket> socket = _socket.lock();
    if (socket)
    {
        LOG_TRC("onHandshakeFail");
        _handshakeSslVerifyFailure = socket->getSslVerifyResult();
        _result = net::AsyncConnectResult::SSLHandShakeFailure;
    }

    callOnConnectFail();
}

void Session::onDisconnect()
{
    ASSERT_CORRECT_THREAD();
    // Make sure the socket is disconnected and released.
    std::shared_ptr<StreamSocket> socket = _socket.lock();
    if (socket)
    {
        LOG_TRC("onDisconnect");
        socket->asyncShutdown(); // Flag for shutdown for housekeeping in SocketPoll.
        socket->shutdownConnection(); // Immediately disconnect.
        _socket.reset();
    }

    _connected = false;
    if (_response)
        _response->error();

    _fd = -1; // No longer our socket fd.
}

std::shared_ptr<StreamSocket> Session::connect()
{
    ASSERT_CORRECT_THREAD();
    _socket.reset(); // Reset to make sure we are disconnected.
#if !MOBILEAPP
    std::shared_ptr<StreamSocket> socket =
        net::connect(_host, _port, isSecure(), shared_from_this(), _addressFilter);
#else
    // The mobile apps have no network connections, so there is no socket.
    std::shared_ptr<StreamSocket> socket;
#endif
    assert((!socket || _fd == socket->getFD()) &&
           "The socket FD must have been set in onConnect");

    // When used with proxy.php we may indeed get nullptr here.
    // assert(socket && "Unexpected nullptr returned from net::connect");
    _socket = socket; // Hold a weak pointer to it.
    return socket; // Return the shared pointer.
}

void Session::asyncConnectSuccess(const std::shared_ptr<StreamSocket>& socket,
                                  net::AsyncConnectResult result)
{
    ASSERT_CORRECT_THREAD();
    assert(socket && _fd == socket->getFD() && "The socket FD must have been set in onConnect");

    _socket = socket; // Hold a weak pointer to it.
    _result = result;

    LOG_ASSERT_MSG(_socket.lock(), "Connect must set the _socket member.");
    LOG_ASSERT_MSG(_socket.lock()->getFD() == socket->getFD(),
                   "Socket FD's mismatch after connect().");
}

#if !MOBILEAPP

void Session::asyncConnect(const std::weak_ptr<SocketPoll>& poll)
{
    ASSERT_CORRECT_THREAD();
    _socket.reset(); // Reset to make sure we are disconnected.

    auto pushConnectCompleteToPoll =
        [this, poll](std::shared_ptr<StreamSocket> socket, net::AsyncConnectResult result)
    {
        std::shared_ptr<SocketPoll> socketPoll(poll.lock());
        if (!socketPoll || !socketPoll->isAlive())
        {
            LOG_WRN("asyncConnect completed after poll " << (!socketPoll ? "destroyed" : "finished"));
            return;
        }

        if (!socket)
        {
            // When used with proxy.php we may indeed get nullptr here.
            socketPoll->addCallback([selfLifecycle = shared_from_this(), this, result]()
                                    { asyncConnectFailed(result); });
            return;
        }

        SocketDisposition disposition(socket);
        disposition.setTransfer(*socketPoll,
                                [selfLifecycle = shared_from_this(), this,
                                 socket = std::move(socket),
                                 result]([[maybe_unused]] const std::shared_ptr<Socket>& moveSocket)
                                {
                                    assert(socket == moveSocket);
                                    asyncConnectSuccess(socket, result);
                                });
        disposition.execute();
    };

    net::asyncConnect(_host, _port, isSecure(), shared_from_this(), pushConnectCompleteToPoll,
                      _addressFilter);
}

#endif // !MOBILEAPP

bool Session::checkTimeout(std::chrono::steady_clock::time_point now)
{
    ASSERT_CORRECT_THREAD();
    if (!_response || _response->done())
        return false;

    const std::chrono::microseconds timeout = getTimeout();
    const auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(now - _startTime);

    if (now < _startTime ||
        (timeout > std::chrono::microseconds::zero() && duration > timeout) ||
        SigUtil::getTerminationFlag())
    {
        LOG_WRN("CheckTimeout: Timeout while requesting [" << _request.getVerb() << ' ' << _host
                                                           << _request.getUrl() << "] after " << duration);

        // Flag that we timed out.
        _response->timeout();

        // Disconnect and trigger the right events and handlers.
        // Note that this is the right way to end a request in HTTP, it's also
        // no good maintaining a poor connection (if that's the issue).
        onDisconnect(); // Trigger manually (why wait for poll to do it?).
        assert(isConnected() == false);
        return true;
    }
    return false;
}

} // namespace http

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
