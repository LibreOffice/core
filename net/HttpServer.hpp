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
 * HTTP server session for handling asynchronous responses with range support.
 * Classes: http::ServerSession
 */

#pragma once

#include <common/NumUtil.hpp>
#include <net/HttpRequest.hpp>

#include <o3tl/safeint.hxx>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <string>
#include <utility>

namespace http
{

#if !MOBILEAPP

/// A server http Session to make asynchronous HTTP responses.
class ServerSession final : public ProtocolHandlerInterface
{
public:
    /// Construct a ServerSession instance.
    ServerSession()
        : _timeout(getDefaultTimeout())
        , _pos(-1)
        , _size(0)
        , _fd(-1)
        , _connected(false)
        , _start(0)
        , _end(-1)
        , _startIsSuffix(false)
        , _statusCode(http::StatusCode::OK)
    {
    }

    /// Returns the default timeout.
    static constexpr std::chrono::milliseconds getDefaultTimeout()
    {
        return std::chrono::seconds(30);
    }

    bool isConnected() const { return _connected; };

    /// Set the timeout, in microseconds.
    void setTimeout(const std::chrono::microseconds timeout) { _timeout = timeout; }
    /// Get the timeout, in microseconds.
    std::chrono::microseconds getTimeout() const { return _timeout; }

    /// The onFinished callback handler signature.
    using FinishedCallback = std::function<void(const std::shared_ptr<ServerSession>& session)>;

    /// Set a callback to handle onFinished events from this session.
    /// onFinished is triggered whenever a request has finished,
    /// regardless of the reason (error, timeout, completion).
    void setFinishedHandler(FinishedCallback onFinished) { _onFinished = std::move(onFinished); }

    using ResponseHeaders = http::Header::Container;

    /// Start an asynchronous upload from a file.
    /// Return true when it dispatches the socket to the SocketPoll.
    /// Note: when reusing this ServerSession, it is assumed that the socket
    /// is already added to the SocketPoll on a previous call (do not
    /// use multiple SocketPoll instances on the same ServerSession).
    bool asyncUpload(std::string fromFile, ResponseHeaders responseHeaders, int start, int end,
                     bool startIsSuffix, http::StatusCode statusCode = http::StatusCode::OK)
    {
        _start = start;
        _end = end;
        _startIsSuffix = startIsSuffix;
        _statusCode = statusCode;

        LOG_TRC("asyncUpload from file [" << fromFile << ']');

        _fd = open(fromFile.c_str(), O_RDONLY);
        if (_fd == -1)
        {
            LOG_ERR("Failed to open file [" << fromFile << "] for uploading");
            return false;
        }

        struct stat sb;
        const int res = fstat(_fd, &sb);
        if (res == -1)
        {
            LOG_SYS("Failed to stat file [" << fromFile);
            close(_fd);
            _fd = -1;
            return false;
        }

        _size = sb.st_size;
        _filename = std::move(fromFile);
        _responseHeaders = std::move(responseHeaders);
        LOG_ASSERT_MSG(!getMimeType().empty(), "Missing Content-Type");

        int firstBytePos = getStart();

        if (lseek(_fd, firstBytePos, SEEK_SET) < 0)
            LOG_SYS("Failed to seek " << _filename << " to " << firstBytePos
                                      << " because: " << strerror(errno));
        else
            _pos = firstBytePos;

        return true;
    }

    /// Start an asynchronous upload of a whole file
    bool asyncUpload(std::string fromFile, ResponseHeaders responseHeaders)
    {
        return asyncUpload(std::move(fromFile), std::move(responseHeaders), 0, -1, false);
    }

    /// Start a partial asynchronous upload from a file based on the contents of a "Range" header
    bool asyncUpload(std::string fromFile, ResponseHeaders responseHeaders,
                     const std::string_view rangeHeader)
    {
        const size_t equalsPos = rangeHeader.find('=');
        if (equalsPos == std::string::npos)
            return asyncUpload(std::move(fromFile), std::move(responseHeaders));

        const std::string_view unit = rangeHeader.substr(0, equalsPos);
        if (unit != "bytes")
            return asyncUpload(std::move(fromFile), std::move(responseHeaders));

        const std::string_view range = rangeHeader.substr(equalsPos + 1);

        size_t dashPos = range.find('-');
        const std::string_view startString = range.substr(0, dashPos);
        std::string endString = "-1";

        if (dashPos != std::string::npos)
        {
            endString = range.substr(dashPos + 1);
        }

        if (startString.empty())
        {
            // Could be a suffix
            auto [start, success] = NumUtil::i32FromString(endString);
            const bool startIsSuffix = success;
            if (!success)
                start = 0;

            constexpr int end = -1;
            return asyncUpload(std::move(fromFile), std::move(responseHeaders), start, end,
                               startIsSuffix, http::StatusCode::PartialContent);
        }

        auto [start, startOk] = NumUtil::i32FromString(startString);
        auto end = NumUtil::i32FromString(endString, -1);
        if (!startOk)
        {
            start = 0;
            end = -1;
        }
        else if (end >= 0)
        {
            ++end; // The end is exclusive in our implementation.
        }

        // FIXME: does not support ranges that specify multiple comma-separated values
        return asyncUpload(std::move(fromFile), std::move(responseHeaders), start, end,
                           /*startIsSuffix=*/false, http::StatusCode::PartialContent);
    }

    int getStart() const
    {
        if (_startIsSuffix)
            return _size - _start;
        return _start;
    }

    int getEnd() const
    {
        if (_startIsSuffix)
            return _size;
        if (_end == -1)
            return _size;
        if (_end > _size)
            return _size;

        return _end;
    }

    /// Calculate how much we're going to send based on the file size and the range
    int getSendSize() const
    {
        int end = getEnd();
        int start = getStart();

        if (start > _size)
            return 0;

        return end - start;
    }

    void asyncShutdown() const
    {
        LOG_TRC("asyncShutdown");
        std::shared_ptr<StreamSocket> socket = _socket.lock();
        if (socket)
        {
            socket->asyncShutdown();
        }
    }

    void dumpState(std::ostream& os, const std::string& indent) const override
    {
        const auto now = std::chrono::steady_clock::now();
        os << indent << "http::ServerSession: #" << _fd << " (" << (_socket.lock() ? "have" : "no")
           << " socket)";
        os << indent << "\tconnected: " << _connected;
        os << indent << "\tstartTime: " << Util::getTimeForLog(now, _startTime);
        os << indent << "\tmimeType: " << getMimeType();
        os << indent << "\tstatusCode: " << getReasonPhraseForCode(_statusCode);
        os << indent << "\tsize: " << _size;
        os << indent << "\tpos: " << _pos;
        os << indent << "\tstart: " << _start;
        os << indent << "\tend: " << _end;
        os << indent << "\tstartIsSuffix: " << _startIsSuffix;
        os << indent << "\tfilename: " << _filename;
        os << '\n';

        // We are typically called from the StreamSocket, so don't
        // recurse back by calling dumpState on the socket again.
    }

private:
    void onConnect(const std::shared_ptr<StreamSocket>& socket) override
    {
        ASSERT_CORRECT_THREAD();
        _connected = false; // Assume disconnected by default.
        _socket = socket;
        if (socket)
        {
            setLogContext(socket->getFD());
            if (_fd >= 0 || _pos >= 0)
            {
                LOG_TRC("Connected");
                _connected = true;

                LOG_DBG("Sending header with size " << getSendSize());
                http::Response httpResponse(_statusCode);
                for (const auto& header : _responseHeaders)
                    httpResponse.set(header.first, header.second);
                httpResponse.setContentLength(getSendSize());
                httpResponse.set("Accept-Ranges", "bytes");
                httpResponse.set("Content-Range", "bytes " + std::to_string(getStart()) + "-" +
                                                      std::to_string(getEnd() - 1) + '/' +
                                                      std::to_string(_size));

                socket->send(httpResponse);
                return;
            }

            LOG_DBG("Has no data to send back");
            http::Response httpResponse(http::StatusCode::BadRequest);
            httpResponse.setContentLength(0);
            socket->sendAndShutdown(httpResponse);
        }
        else
        {
            LOG_DBG("Error: onConnect without a valid socket");
        }
    }

    void shutdown(bool /*goingAway*/, const std::string_view /*statusMessage*/) override
    {
        LOG_TRC("shutdown");
    }

    void getIOStats(uint64_t& sent, uint64_t& recv) override
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

    int getPollEvents(std::chrono::steady_clock::time_point /*now*/,
                      int64_t& /*timeoutMaxMicroS*/) override
    {
        ASSERT_CORRECT_THREAD();
        int events = POLLIN;
        if (_fd >= 0 || _pos >= 0)
            events |= POLLOUT;
        return events;
    }

    void handleIncomingMessage(SocketDisposition& /*disposition*/) override
    {
        ASSERT_CORRECT_THREAD();
        std::shared_ptr<StreamSocket> socket = _socket.lock();
        if (!isConnected())
        {
            LOG_ERR("handleIncomingMessage called when not connected.");
            assert(!socket && "Expected no socket when not connected.");
            return;
        }

        assert(socket && "No valid socket to handleIncomingMessage.");
        LOG_TRC("handleIncomingMessage");
    }

    void performWrites(std::size_t capacity) override
    {
        ASSERT_CORRECT_THREAD();
        // We may get called after disconnecting and freeing the Socket instance.
        std::shared_ptr<StreamSocket> socket = _socket.lock();
        if (socket)
        {
            Buffer& out = socket->getOutBuffer();
            LOG_TRC("performWrites: " << out.size() << " bytes, capacity: " << capacity);

            while (_fd >= 0)
            {
                const auto provisioned = std::min<size_t>(capacity, getEnd() - _pos);
                if (provisioned <= 0)
                    break;

                char* buffer = out.provision(provisioned);
                int n;
                while ((n = ::read(_fd, buffer, provisioned)) < 0 && errno == EINTR)
                    LOG_TRC("EINTR reading from " << _filename);

                if (n <= 0 || _pos >= getEnd())
                {
                    out.commit(provisioned, 0); // Rollback, nothing written.

                    if (n >= 0)
                    {
                        LOG_TRC("performWrites finished uploading");
                    }
                    else
                    {
                        LOG_SYS("Failed to upload file");
                    }

                    close(_fd);
                    _fd = -1;
                    socket->asyncShutdown(); // Trigger async shutdown.
                    break;
                }

                out.commit(provisioned, n);
                socket->writeOutgoingData();
                _pos += n;
                LOG_ASSERT(o3tl::make_unsigned(n) <= capacity);
                capacity -= n;
                LOG_TRC("performWrites wrote " << n << " bytes, capacity: " << capacity);
            }
        }
    }

    void onDisconnect() override
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
    }

    int sendTextMessage(std::string_view, bool) const override { return 0; }
    int sendBinaryMessage(std::string_view, bool) const override { return 0; }

    std::string getMimeType() const
    {
        const auto it = std::find_if(_responseHeaders.begin(), _responseHeaders.end(),
                                     [](const Header::Pair& pair) -> bool
                                     { return Util::iequal(pair.first, "Content-Type"); });
        if (it != _responseHeaders.end())
            return it->second;
        return std::string();
    }

private:
    http::Header::Container _responseHeaders; ///< The data Content-Type.
    std::chrono::microseconds _timeout;
    std::chrono::steady_clock::time_point _startTime;
    std::string _filename; ///< The input filename.
    int _pos; ///< The current position in the data string.
    int _size; ///< The size of the data in bytes.
    int _fd; ///< The descriptor of the file to upload.
    bool _connected;
    int _start; ///< The position we start reading from, the data includes this first byte
        //  If this is greater than _size we will return no bytes
        //  If this is less than 0 or greater than _end behavior is unspecified
    int _end; ///< The position we stop reading at, the data does not include this last byte
        //  If this is greater than or equal to _start we will only return bytes in the range
        //  If this is greater than _size we will return all bytes between _start and _size
        //  If this is -1 we will treat it as if it were equal to _size
    bool
        _startIsSuffix; ///< If this is true, we'll treat _start as an offset from the end, not from the start
    //  In that case, we'll ignore end entirely
    //  e.g. if this is true and start is 5, we will send the last 5 bytes
    http::StatusCode _statusCode;
    FinishedCallback _onFinished;
    /// Keep _socket as last member so it is destructed first, ensuring that
    /// the peer members it depends on are not destructed before it
    std::weak_ptr<StreamSocket> _socket;
};

#endif // !MOBILEAPP

} // namespace http

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
