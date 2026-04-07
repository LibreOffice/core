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
#include <Poco/Net/HTTPResponse.h>

#include <cstdint>
#include <memory>
#include <netdb.h>
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

        // Copy the header entries over to us.
        for (const auto& pair : msgHeader)
        {
            set(Util::trimmed(pair.first), Util::trimmed(pair.second));
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
                    int chunkLenSize = 0;
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
                int chunkLenSize = 0;
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

} // namespace http

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
