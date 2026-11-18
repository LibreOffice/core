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

#pragma once

#include <test/testlog.hpp>

#include <common/Common.hpp>
#include <common/Log.hpp>
#include <common/Protocol.hpp>

#include <Poco/Net/WebSocket.h>

#include <iomanip>

/// Deprecated: do not use ... replaced by net/Socket.hpp
///
/// WebSocket that is thread safe, and handles large frames transparently.
/// Careful - sendFrame and receiveFrame are _not_ virtual,
/// we need to make sure that we use COOLWebSocket all over the place.
/// It would be a kind of more natural to encapsulate Poco::Net::WebSocket
/// instead of inheriting (from that reason,) but that would requite much
/// larger code changes.
class COOLWebSocket : public Poco::Net::WebSocket
{
public:
    COOLWebSocket(const Socket& socket) :
        Poco::Net::WebSocket(socket)
    {
    }

    COOLWebSocket(Poco::Net::HTTPClientSession& cs,
                  Poco::Net::HTTPRequest& request,
                  Poco::Net::HTTPResponse& response) :
        Poco::Net::WebSocket(cs, request, response)
    {
    }

    /// Wrapper for Poco::Net::WebSocket::receiveFrame() that handles PING frames
    /// (by replying with a PONG frame) and PONG frames. PONG frames are ignored.

    /// Returns number of bytes received, or 0 if the Poco receiveFrame() returned 0,
    /// or -1 if no "interesting" (not PING or PONG) frame was actually received).

    /// Should we also factor out the handling of non-final and continuation frames into this?
    int receiveFrame(char* buffer, const int length, int& flags,
                     [[maybe_unused]] const std::string_view testname = std::string_view())
    {
        // Timeout is in microseconds. We don't need this, except to yield the cpu.
        static const Poco::Timespan waitTime(POLL_TIMEOUT_MICRO_S / 10);

        while (poll(waitTime, Poco::Net::Socket::SELECT_READ))
        {
            const int n = Poco::Net::WebSocket::receiveFrame(buffer, length, flags);

            if (n <= 0)
                TST_LOG("Got nothing (" << n << ')');
            else
                TST_LOG("Got frame: " << getAbbreviatedFrameDump(buffer, n, flags));

            if ((flags & WebSocket::FRAME_OP_BITMASK) == WebSocket::FRAME_OP_CLOSE)
            {
                // Nothing to do.
                return n;
            }

            if ((flags & WebSocket::FRAME_OP_BITMASK) == WebSocket::FRAME_OP_PING)
            {
                // Echo back the ping message.
                if (Poco::Net::WebSocket::sendFrame(buffer, n,
                                                    static_cast<int>(WebSocket::FRAME_FLAG_FIN)
                                                        | WebSocket::FRAME_OP_PONG)
                    != n)
                {
                    TST_LOG("WARN: Sending Pong failed.");
                    return -1;
                }
            }
            else if ((flags & WebSocket::FRAME_OP_BITMASK) == WebSocket::FRAME_OP_PONG)
            {
                // In case we do send pings in the future.
            }
            else
            {
                return n;
            }
        }

        // Not ready for read.
        return -1;
    }

    /// Wrapper for Poco::Net::WebSocket::sendFrame() that handles large frames.
    int sendFrame(const char* buffer, const int length, const int flags = FRAME_TEXT,
                  [[maybe_unused]] const std::string& testname = std::string())
    {
        const int result = Poco::Net::WebSocket::sendFrame(buffer, length, flags);

        if (result != length)
        {
            TST_LOG("ERROR: Sent incomplete message, expected "
                    << length << " bytes but sent " << result
                    << " for: " << getAbbreviatedFrameDump(buffer, length, flags));
        }
        else
        {
            TST_LOG("Sent frame: " << getAbbreviatedFrameDump(buffer, length, flags));
        }

        return result;
    }

    /// Safe shutdown by sending a normal close frame, if socket is not in error,
    /// or, otherwise, close the socket without sending close frame, if it is.
    void shutdown(const std::string_view testname = std::string_view())
    {
        shutdown(Poco::Net::WebSocket::StatusCodes::WS_NORMAL_CLOSE,
                 /*statusMessage=*/std::string(), testname);
    }

    /// Safe shutdown by sending a specific close frame, if socket is not in error,
    /// or, otherwise, close the socket without sending close frame, if it is.
    void shutdown(Poco::UInt16 statusCode, const std::string& statusMessage = std::string(),
                  [[maybe_unused]] const std::string_view testname = std::string_view())
    {
        try
        {
            // Calling shutdown, in case of error, would try to send a 'close' frame
            // which won't work in case of broken pipe or timeout from peer. Just close the
            // socket in that case preventing 'close' frame from being sent.
            if (Poco::Net::WebSocket::poll(Poco::Timespan(0), Socket::SelectMode::SELECT_ERROR))
            {
                Poco::Net::WebSocket::close();
            }
            else
            {
                Poco::Net::WebSocket::shutdown(statusCode, statusMessage);
            }
        }
        catch (const Poco::Exception& exc)
        {
            TST_LOG("WARN: COOLWebSocket::shutdown: Exception: "
                    << exc.displayText()
                    << (exc.nested() ? " (" + exc.nested()->displayText() + ')' : ""));

            // Just close it.
            try
            {
                Poco::Net::WebSocket::close();
            }
            catch (const std::exception&)
            {
                // Nothing we can do.
            }
        }
    }

    // Return a string dump of a WebSocket frame: Its opcode, length, first line (if present),
    // flags. For human-readable logging purposes. Format not guaranteed to be stable. Not to be
    // inspected programmatically.
    static std::string getAbbreviatedFrameDump(const char* message, const int length,
                                               const int flags)
    {
        std::ostringstream result;
        switch (flags & Poco::Net::WebSocket::FRAME_OP_BITMASK)
        {
#define CASE(x)                                                                                    \
    case Poco::Net::WebSocket::FRAME_OP_##x:                                                       \
        result << #x << " (0x" << std::hex << flags << ')';                                        \
        break
            CASE(CONT);
            CASE(TEXT);
            CASE(BINARY);
            CASE(CLOSE);
            CASE(PING);
            CASE(PONG);
#undef CASE
            default:
                result << "UNKNOWN (0x" << std::hex << flags << ')';
                break;
        }

        result << ' ' << std::setw(3) << std::dec << length << " bytes"
               << (flags & Poco::Net::WebSocket::FRAME_FLAG_FIN ? " (FIN)" : "");

        if (length > 0 &&
            ((flags & Poco::Net::WebSocket::FRAME_OP_BITMASK) == Poco::Net::WebSocket::FRAME_OP_TEXT ||
             (flags & Poco::Net::WebSocket::FRAME_OP_BITMASK) == Poco::Net::WebSocket::FRAME_OP_BINARY ||
             (flags & Poco::Net::WebSocket::FRAME_OP_BITMASK) == Poco::Net::WebSocket::FRAME_OP_PING ||
             (flags & Poco::Net::WebSocket::FRAME_OP_BITMASK) == Poco::Net::WebSocket::FRAME_OP_PONG))
            result << ": '" << COOLProtocol::getAbbreviatedMessage(message, length) << '\'';
        return result.str();
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
