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
 * WebSocket protocol handler for frame parsing, masking, and message handling.
 * Classes: WebSocketHandler
 */

#pragma once

#include <common/HexUtil.hpp>
#include <common/Log.hpp>
#include <common/NumUtil.hpp>
#include <common/Protocol.hpp>
#include <common/Unit.hpp>
#include <common/Util.hpp>
#include <net/HttpHelper.hpp>
#if !MOBILEAPP
#include <net/HttpRequest.hpp>
#endif
#include <net/NetUtil.hpp>
#include <net/Socket.hpp>

#include <Poco/Net/HTTPResponse.h>

#include <chrono>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

class WebSocketHandler : public ProtocolHandlerInterface
{
private:
    // The embedded app exchanges length-prefixed messages and leaves the framing state below at
    // its initial values, so both flavours agree on the class layout.

    /// The security key. Meaningful only for clients.
    const std::string _key;
    std::vector<char> _wsPayload;
    /// The socket that owns us (we can't own it).
    std::weak_ptr<StreamSocket> _socket;
    std::chrono::steady_clock::time_point _lastPingSentTime;
    [[maybe_unused]] int _pingTimeUs;
    [[maybe_unused]] bool _isMasking;
    bool _inFragmentBlock;
    [[maybe_unused]] unsigned char _lastFlags; ///< The flags in the last frame.
    std::atomic<bool> _shuttingDown;
    const bool _isClient;

    // Last member.
    /// The UnitBase instance. We capture it here since
    /// this is our instance, but the test framework
    /// has a single global instance via UnitWSD::get().
    UnitBase* const _unit;

protected:
    struct WSFrameMask
    {
        static constexpr unsigned char Fin = 0x80;
        static constexpr unsigned char Mask = 0x80;
    };

    static constexpr std::chrono::microseconds InitialPingDelayMicroS = std::chrono::milliseconds(25);
    static constexpr std::chrono::microseconds PingFrequencyMicroS = std::chrono::seconds(18);

public:
    /// Perform upgrade ourselves, or select a client web socket.
    /// Parameters:
    /// isClient: the instance should behave like a client (true) or like a server (false)
    ///           (from websocket perspective)
    /// isMasking: a client should mask (true) or not (false) outgoing frames
    WebSocketHandler(bool isClient, bool isMasking)
        : _key(isClient ? generateKey() : std::string())
        , _lastPingSentTime(std::chrono::steady_clock::now() -
                           PingFrequencyMicroS +
                           InitialPingDelayMicroS)
        , _pingTimeUs(0)
        , _isMasking(isClient && isMasking)
        , _inFragmentBlock(false)
        , _lastFlags(0)
        , _shuttingDown(false)
        , _isClient(isClient)
        , _unit(UnitBase::isUnitTesting() ? &UnitBase::get() : nullptr)
    {
    }

    /// Upgrades itself to a websocket directly.
    /// Parameters:
    /// socket: the TCP socket which received the upgrade request
    /// request: the HTTP upgrade request to WebSocket
    /// expectedOrigin: derived from serverurl
    /// allowedOrigin: if origin is allowed using setting "indirection_endpoint.geolocation_setup.allowed_websocket_origins"
    template <typename T>
    WebSocketHandler(const std::shared_ptr<StreamSocket>& socket, const T& request,
                     bool allowedOrigin)
        : WebSocketHandler(/*isClient=*/false, /*isMasking=*/false)
    {
        if (!socket)
            throw std::runtime_error("Invalid socket while upgrading to WebSocket.");

        _socket = socket;
        setLogContext(socket->getFD());

        // As a server, respond with 101 protocol-upgrade.
        assert(!_isClient);
        upgradeToWebSocket(socket, request, allowedOrigin);
    }

    /// Status codes sent to peer on shutdown.
    enum class StatusCodes : unsigned short
    {
        NORMAL_CLOSE            = 1000,
        ENDPOINT_GOING_AWAY     = 1001,
        PROTOCOL_ERROR          = 1002,
        PAYLOAD_NOT_ACCEPTABLE  = 1003,
        RESERVED                = 1004,
        RESERVED_NO_STATUS_CODE = 1005,
        RESERVED_ABNORMAL_CLOSE = 1006,
        MALFORMED_PAYLOAD       = 1007,
        POLICY_VIOLATION        = 1008,
        PAYLOAD_TOO_BIG         = 1009,
        EXTENSION_REQUIRED      = 1010,
        UNEXPECTED_CONDITION    = 1011,
        RESERVED_TLS_FAILURE    = 1015
    };

#if !MOBILEAPP

    /// Returns the Web-Socket Security Key generated for this instance.
    const std::string& getWebSocketKey() const { return _key; }

    /// Returns the flags of the last received WS frame.
    unsigned char lastFlags() const { return _lastFlags; }

    /// Create a WebSocket connection to the given @host
    /// and @port and add the socket to @poll.
    bool wsRequest(http::Request& req, const std::string& host, const std::string& port,
                   bool isSecure, SocketPoll& poll)
    {
        const std::string hostAndPort = host + ':' + port;
        LOGA_TRC(WebSocket, "Web-Socket request: " << hostAndPort);

        auto socket = net::connect(host, port, isSecure, shared_from_this());
        if (!socket)
        {
            LOG_ERR("Failed to connect to " << host << ':' << port);
            return false;
        }

        LOGA_TRC(WebSocket,
                 "Connected #" << socket->getFD() << " to " << hostAndPort << ", sending request");
        onConnect(socket);

        req.set("Host", hostAndPort); // Make sure the host is set.

        // Set a consistent Origin
        std::string protocol = isSecure ? "https" : "http";
        req.set("Origin", protocol + "://" + hostAndPort);

        req.setConnectionToken(http::Header::ConnectionToken::Upgrade);
        req.set("Upgrade", "websocket");
        req.set("Sec-WebSocket-Version", "13");
        req.set("Sec-WebSocket-Key", getWebSocketKey());

        if (socket->send(req))
        {
            poll.insertNewSocket(socket);
            return true;
        }

        LOG_ERR("Failed to make WebSocket request");
        return false;
    }
#endif

protected:
    /// Implementation of the ProtocolHandlerInterface.
    void onConnect(const std::shared_ptr<StreamSocket>& socket) override
    {
        ASSERT_CORRECT_THREAD();
        LOG_ASSERT_MSG(socket, "Invalid socket passed to WebSocketHandler::onConnect");

        _socket = socket;
        setLogContext(socket->getFD());
        LOGA_TRC(WebSocket, "Connected to WS Handler " << this);
    }

    /// Sends WS Close frame to the peer.
    void sendCloseFrame(const StatusCodes statusCode = StatusCodes::NORMAL_CLOSE,
                        const std::string_view statusMessage = std::string_view())
    {
        std::shared_ptr<StreamSocket> socket = _socket.lock();
        if (!socket)
        {
            LOG_ERR("No socket associated with WebSocketHandler " << this
                                                                  << " to send Close Frame to");
            return;
        }

        if (!socket->isOpen())
        {
            LOG_DBG("Socket is not open. Cannot send Close Frame");
            return;
        }

        // Don't send close-frame more than once.
        if (!_shuttingDown)
        {
            LOGA_TRC(WebSocket, "Shutdown websocket, code: "
                     << static_cast<unsigned>(statusCode) << ", message: " << statusMessage);
            _shuttingDown = true;

            if constexpr (!Util::isMobileApp())
            {
                const size_t len = statusMessage.size();
                std::vector<char> buf(2 + len);
                buf[0] = ((int(statusCode) >> 8) & 0xff);
                buf[1] = ((int(statusCode) >> 0) & 0xff);
                std::copy(statusMessage.begin(), statusMessage.end(), buf.begin() + 2);
                const unsigned char flags = WSFrameMask::Fin | static_cast<char>(WSOpCode::Close);

                sendFrame(socket, buf.data(), buf.size(), flags);
            }
        }
    }

    void shutdown(bool goingAway, const std::string_view statusMessage) override
    {
        ASSERT_CORRECT_THREAD();
        shutdownImpl(_socket.lock(),
                     goingAway ? WebSocketHandler::StatusCodes::ENDPOINT_GOING_AWAY :
                     WebSocketHandler::StatusCodes::NORMAL_CLOSE, statusMessage,
                     /*hardShutdown=*/ false, /*silentShutdown=*/ false);
    }

    void getIOStats(uint64_t &sent, uint64_t &recv) override
    {
        std::shared_ptr<StreamSocket> socket = getSocket().lock();
        if (socket)
            socket->getIOStats(sent, recv);
        else
        {
            sent = 0;
            recv = 0;
        }
    }

public:
    void shutdown(const StatusCodes statusCode = StatusCodes::NORMAL_CLOSE,
                  const std::string_view statusMessage = std::string_view(),
                  bool hardShutdown = false)
    {
        shutdownImpl(_socket.lock(),
                     statusCode, statusMessage, hardShutdown, false);
    }

    /// Don't wait for the remote Websocket to handshake with us; go down fast.
    void shutdownAfterWriting()
    {
        shutdownImpl(_socket.lock(), WebSocketHandler::StatusCodes::NORMAL_CLOSE,
                     std::string_view(), true /* hard async shutdown & close */, false);
    }

    /// Returns true if the underlying socket is connected.
    bool isConnected() const
    {
        std::shared_ptr<StreamSocket> socket = _socket.lock();
        return socket && socket->isOpen();
    }

private:
    void shutdownSilent(const std::shared_ptr<StreamSocket>& socket)
    {
        shutdownImpl(socket, WebSocketHandler::StatusCodes::POLICY_VIOLATION /* ignored */,
                     std::string_view(), true /* hard async shutdown & close */, true);
    }

    void shutdownImpl(const std::shared_ptr<StreamSocket>& socket, const StatusCodes statusCode,
                      const std::string_view statusMessage, bool hardShutdown, bool silentShutdown)
    {
        if (socket)
        {
            const bool silent = _shuttingDown || silentShutdown;
            LOGA_TRC(WebSocket, "Shutdown: Closing Connection " << (silent ? "(silent)" : "(coop)"));
            if (!silent)
                sendCloseFrame(statusCode, statusMessage);
            socket->asyncShutdown();
            socket->ignoreInput();
            assert(socket->getInBuffer().empty() &&
                   "Socket buffer must be empty after ignoreInput");

            // force close after writing this message (real shutdown)
            if (hardShutdown)
                socket->shutdownConnection();
        }

        _wsPayload.clear();
        _inFragmentBlock = false;
        _shuttingDown = false;
    }
    bool handleTCPStream(const std::shared_ptr<StreamSocket>& socket)
    {
        assert(socket && "Expected a valid socket instance.");

        // websocket fun !
        const size_t len = socket->getInBuffer().size();

        if (len == 0)
            return false; // avoid logging.

#if !MOBILEAPP
        if (len < 2) // partial read
        {
            LOGA_TRC(WebSocket, "Still incomplete WebSocket message, have " << len << " bytes");
            return false;
        }

        unsigned char *p = reinterpret_cast<unsigned char*>(socket->getInBuffer().data());
        _lastFlags = p[0];
        const bool fin = _lastFlags & 0x80;
        const WSOpCode code = static_cast<WSOpCode>(_lastFlags & 0x0f);
        const bool hasMask = p[1] & 0x80;
        size_t payloadLen = p[1] & 0x7f;
        size_t headerLen = 2;

        // normally - 7 bit length.
        if (payloadLen == 126) // 2 byte length
        {
            if (len < 2 + 2)
            {
                LOGA_TRC(WebSocket, "Still incomplete WebSocket message, have " << len << " bytes");
                return false;
            }

            payloadLen = (unsigned(p[2]) << 8) | unsigned(p[3]);
            headerLen += 2;
        }
        else if (payloadLen == 127) // 8 byte length
        {
            if (len < 2 + 8)
            {
                LOGA_TRC(WebSocket, "Still incomplete WebSocket message, have " << len << " bytes");
                return false;
            }
            payloadLen = ((uint64_t(p[9]) <<  0) + (uint64_t(p[8]) <<  8) +
                          (uint64_t(p[7]) << 16) + (uint64_t(p[6]) << 24) +
                          (uint64_t(p[5]) << 32) + (uint64_t(p[4]) << 40) +
                          (uint64_t(p[3]) << 48) + (uint64_t(p[2]) << 56));
            // FIXME: crop read length to remove top / sign bits.
            headerLen += 8;
        }

        unsigned char *mask = nullptr;

        if (hasMask)
        {
            mask = p + headerLen;
            headerLen += 4;
        }

        if (headerLen > len || payloadLen > len - headerLen)
        { // partial read wait for more data.
            LOGA_TRC(WebSocket, "Still incomplete WebSocket frame, have "
                    << len << " bytes, frame is " << payloadLen + headerLen << " bytes");
            return false;
        }

        if (hasMask && _isClient)
        {
            LOG_ERR("Servers should not send masked frames. Only clients");
            shutdown(StatusCodes::PROTOCOL_ERROR);
            return true;
        }

        LOGA_TRC(WebSocket, "Incoming WebSocket data of "
                                << len << " bytes: "
                                << HexUtil::stringifyHexLine(socket->getInBuffer(), 0,
                                                             std::min(size_t(32), len)));

        unsigned char *data = p + headerLen;

        if (isControlFrame(code))
        {
            //Process control frames

            std::vector<char> ctrlPayload;

            readPayload(data, payloadLen, mask, ctrlPayload);
            socket->getInBuffer().eraseFirst(headerLen + payloadLen);
            LOGA_TRC(WebSocket, "Incoming WebSocket frame code "
                    << static_cast<unsigned>(code) << ", fin? " << fin << ", mask? " << hasMask
                    << ", payload length: " << payloadLen
                    << ", residual socket data: " << socket->getInBuffer().size() << " bytes");

            // All control frames MUST NOT be fragmented and MUST have a payload length of 125 bytes or less
            if (!fin)
            {
                LOG_ERR("A control frame cannot be fragmented");
                shutdown(StatusCodes::PROTOCOL_ERROR);
                return true;
            }
            if (payloadLen > 125)
            {
                LOG_ERR("The payload length of a control frame must not exceed 125 bytes");
                shutdown(StatusCodes::PROTOCOL_ERROR);
                return true;
            }

            switch (code)
            {
            case WSOpCode::Pong:
                {
                    if (_isClient)
                        LOG_WRN("Servers should not send pongs, only clients");

                    _pingTimeUs = std::chrono::duration_cast<std::chrono::microseconds>
                        (std::chrono::steady_clock::now() - _lastPingSentTime).count();
                    LOGA_TRC(WebSocket, "Pong received: " << _pingTimeUs << " microseconds");
                    gotPing(code, _pingTimeUs);
                }
                break;
            case WSOpCode::Ping:
                {
                    if (!_isClient)
                        LOG_DBG("Clients should not send pings, only servers");

                    const auto now = std::chrono::steady_clock::now();
                    _pingTimeUs = std::chrono::duration_cast<std::chrono::microseconds>
                                            (now - _lastPingSentTime).count();
                    sendPong(now, ctrlPayload.data(), payloadLen, socket);
                    gotPing(code, _pingTimeUs);
                }
                break;
            case WSOpCode::Close:
                {
                    std::string message;
                    StatusCodes statusCode = StatusCodes::NORMAL_CLOSE;
                    if (!_shuttingDown)
                    {
                        // Peer-initiated shutdown must be echoed.
                        // Otherwise, this is the echo to _our_ shutdown message, which we should ignore.
                        if (ctrlPayload.size())
                        {
                            statusCode = static_cast<StatusCodes>((uint64_t(static_cast<unsigned char>(ctrlPayload[0])) << 8) +
                                                                (uint64_t(static_cast<unsigned char>(ctrlPayload[1])) << 0));
                            if (ctrlPayload.size() > 2)
                                message.assign(&ctrlPayload[2], &ctrlPayload[2] + ctrlPayload.size() - 2);
                        }

                        LOG_TRC("Peer initiated socket shutdown. Code: "
                                << static_cast<int>(statusCode));
                    }
                    shutdown(statusCode, message);
                    return true;
                }
            default:
                    LOG_ERR("Received unknown control code");
                    shutdown(StatusCodes::PROTOCOL_ERROR);
                    break;
            }

            return true;
        }

        // Check data frames for errors
        if (_inFragmentBlock)
        {
            if (code != WSOpCode::Continuation)
            {
                LOG_ERR("A fragment that is not the first fragment of a message must have the opcode "
                        "equal to 0");
                shutdown(StatusCodes::PROTOCOL_ERROR);
                return true;
            }
        }
        else if (code == WSOpCode::Continuation)
        {
            LOG_ERR("An unfragmented message or the first fragment of a fragmented message must "
                    "have the opcode different than 0");
            shutdown(StatusCodes::PROTOCOL_ERROR);
            return true;
        }

        //Process data frame
        readPayload(data, payloadLen, mask, _wsPayload);
#else
        // Unwrap what StreamSocket::readIncomingData() did
        const size_t headerLen = sizeof(ssize_t);
        ssize_t payloadLen;
        memcpy(&payloadLen, socket->getInBuffer().data(), headerLen);
        unsigned char * const p = reinterpret_cast<unsigned char*>(socket->getInBuffer().data() + headerLen);
        _wsPayload.insert(_wsPayload.end(), p, p + payloadLen);
#endif

        socket->eraseFirstInputBytes(headerLen + payloadLen);

#if !MOBILEAPP

        LOGA_TRC(WebSocket, "Incoming WebSocket frame code "
                                << static_cast<unsigned>(code) << ", fin? " << fin << ", mask? "
                                << hasMask << ", payload length: " << payloadLen
                                << ", residual socket data: " << socket->getInBuffer().size()
                                << " bytes, unmasked data: " +
                                       HexUtil::stringifyHexLine(
                                           _wsPayload, 0, std::min(size_t(32), _wsPayload.size())));

        if (fin)
        {
            // If is final fragment then process the accumulated message.

            try
            {
                handleMessage(_wsPayload);
            }
            catch (const Poco::Exception& ex)
            {
                LOG_ERR("Error during handleMessage: " << ex.displayText());
            }
            catch (const std::exception& exception)
            {
                LOG_ERR("Error during handleMessage: " << exception.what());
            }
            catch (...)
            {
                LOG_ERR("Error during handleMessage");
            }

            _inFragmentBlock = false;
        }
        else
        {
            _inFragmentBlock = true;
            // If is not final fragment then wait for next fragment.
            return true;
        }
#else
        try
        {
            handleMessage(_wsPayload);
        }
        catch (const std::exception& exception)
        {
            LOG_ERR("Error during handleMessage: " << exception.what());
        }
        catch (...)
        {
            LOG_ERR("Error during handleMessage");
        }
#endif

        _wsPayload.clear();

        return true;
    }

protected:
    /// Implementation of the ProtocolHandlerInterface.
    void handleIncomingMessage(SocketDisposition&) override
    {
        ASSERT_CORRECT_THREAD();
        std::shared_ptr<StreamSocket> socket = _socket.lock();

        if constexpr (Util::isMobileApp())
        {
            // No separate "upgrade" is going on
            if (socket && !socket->isWebSocket())
                socket->setWebSocket();
        }

        if (!socket)
        {
            LOG_ERR("No socket associated with WebSocketHandler " << this);
        }
#if !MOBILEAPP
        else if (_isClient && !socket->isWebSocket())
        {
            try
            {
                handleClientUpgrade(socket);
            }
            catch (const std::exception& ex)
            {
                LOG_DBG("handleClientUpgrade exception caught: " << ex.what());
            }
        }
#endif
        else
        {
            while (socket->processInputEnabled() && handleTCPStream(socket))
                ; // might have multiple messages in the accumulated buffer.
        }
    }

    int getPollEvents([[maybe_unused]] std::chrono::steady_clock::time_point now,
                      [[maybe_unused]] int64_t& timeoutMaxMicroS) override
    {
        ASSERT_CORRECT_THREAD();
#if !MOBILEAPP
        if (!_isClient)
        {
            const auto timeSincePingMicroS
                = std::chrono::duration_cast<std::chrono::microseconds>(now - _lastPingSentTime);
            timeoutMaxMicroS
                = std::min(timeoutMaxMicroS, int64_t((PingFrequencyMicroS - timeSincePingMicroS).count()));
        }
#endif
        int events = POLLIN;
        if (_msgHandler && _msgHandler->hasQueuedMessages())
            events |= POLLOUT;
        return events;
    }

#if !MOBILEAPP
private:
    /// Sends a native control-frame ping or pong message
    void sendPingOrPong(std::chrono::steady_clock::time_point now,
                        const char* data, const size_t len,
                        const WSOpCode code,
                        const std::shared_ptr<StreamSocket>& socket)
    {
        assert(socket && "Expected a valid socket instance.");

        // Must not send this before we're upgraded.
        if (!socket->isWebSocket())
        {
            LOG_WRN("Attempted ping on non-upgraded websocket! #" << socket->getFD());
            _lastPingSentTime = now; // Pretend we sent it to avoid timing out immediately.
            return;
        }

        LOGA_TRC(WebSocket, "Sending " << (code == WSOpCode::Ping ? "ping" : "pong"));
        // FIXME: allow an empty payload.
        sendMessage(data, len, code, false);
        _lastPingSentTime = now;
    }

public:
    /// Sends a native control-frame ping message
    void sendPing(std::chrono::steady_clock::time_point now,
                  const std::shared_ptr<StreamSocket>& socket)
    {
        if (_isClient)
            LOG_DBG("Clients should not send pings, only servers");
        sendPingOrPong(now, "", 1, WSOpCode::Ping, socket);
    }

    /// Sends a native control-frame pong message
    void sendPong(std::chrono::steady_clock::time_point now,
                  const char* data, const size_t len,
                  const std::shared_ptr<StreamSocket>& socket)
    {
        if (!_isClient)
            LOG_WRN("Servers should not send pongs, only clients");
        sendPingOrPong(now, data, len, WSOpCode::Pong, socket);
    }
#endif

    /// Do we need to handle a timeout ?
    bool checkTimeout([[maybe_unused]] std::chrono::steady_clock::time_point now) override
    {
        ASSERT_CORRECT_THREAD();
#if !MOBILEAPP
        if (_isClient)
            return false;

        const auto timeSincePingMicroS
            = std::chrono::duration_cast<std::chrono::microseconds>(now - _lastPingSentTime);
        if (timeSincePingMicroS >= PingFrequencyMicroS)
        {
            const std::shared_ptr<StreamSocket> socket = _socket.lock();
            if (socket)
                sendPing(now, socket);
        }
#endif
        return false;
    }

public:
    void performWrites(std::size_t capacity) override
    {
        ASSERT_CORRECT_THREAD();
        if (_msgHandler)
            _msgHandler->writeQueuedMessages(capacity);
    }

    void onDisconnect() override
    {
        ASSERT_CORRECT_THREAD();
        if (_msgHandler)
            _msgHandler->onDisconnect();
    }

    /// Implementation of the ProtocolHandlerInterface.
    int sendTextMessage(const std::string_view msg, bool flush = false) const override
    {
        ASSERT_CORRECT_THREAD();
        return sendMessage(msg.data(), msg.size(), WSOpCode::Text, flush);
    }

    /// Implementation of the ProtocolHandlerInterface.
    int sendBinaryMessage(const std::string_view data, bool flush = false) const override
    {
        ASSERT_CORRECT_THREAD();
        return sendMessage(data.data(), data.size(), WSOpCode::Binary, flush);
    }

    /// Sends a WebSocket message of WPOpCode type.
    /// Returns the number of bytes written (including frame overhead) on success,
    /// 0 for closed socket, and -1 for other errors.
    int sendMessage(const char* data, const size_t len, const WSOpCode code, const bool flush) const
    {
        if (UnitBase::isUnitTesting() && !Util::isFuzzing())
        {
            int unitReturn = -1;
            if (_unit->filterSendWebSocketMessage(std::string_view(data, len), code, flush,
                                                  unitReturn))
                return unitReturn;
        }

        //TODO: Support fragmented messages.

        std::shared_ptr<StreamSocket> socket = _socket.lock();
        return sendFrame(socket, data, len, WSFrameMask::Fin | static_cast<unsigned char>(code), flush);
    }

    bool processInputEnabled() const override
    {
        ASSERT_CORRECT_THREAD();
        std::shared_ptr<StreamSocket> socket = _socket.lock();
        if (socket)
            return socket->processInputEnabled();

        return true;
    }

    void flush() const
    {
        std::shared_ptr<StreamSocket> socket = _socket.lock();
        if (socket)
            socket->attemptWrites();
    }

protected:

#if !MOBILEAPP
    /// Builds a websocket frame based on data and flags received as parameters.
    /// The frame is output in 'out' parameter
    void buildFrame(const char* data, const uint64_t len, unsigned char flags, Buffer &out) const
    {
        int slen = 0;
        char scratch[16];

        // All unfragmented frames must have the Fin bit.
        scratch[slen++] = static_cast<char>(WSFrameMask::Fin | flags);

        int maskFlag = _isMasking ? 0x80 : 0;
        if (len < 126)
        {
            scratch[slen++] = static_cast<char>(len | maskFlag);
        }
        else if (len <= 0xffff)
        {
            scratch[slen++] = static_cast<char>(126 | maskFlag);
            scratch[slen++] = static_cast<char>((len >> 8) & 0xff);
            scratch[slen++] = static_cast<char>((len >> 0) & 0xff);
        }
        else
        {
            scratch[slen++] = static_cast<char>(127 | maskFlag);
            scratch[slen++] = static_cast<char>((len >> 56) & 0xff);
            scratch[slen++] = static_cast<char>((len >> 48) & 0xff);
            scratch[slen++] = static_cast<char>((len >> 40) & 0xff);
            scratch[slen++] = static_cast<char>((len >> 32) & 0xff);
            scratch[slen++] = static_cast<char>((len >> 24) & 0xff);
            scratch[slen++] = static_cast<char>((len >> 16) & 0xff);
            scratch[slen++] = static_cast<char>((len >> 8) & 0xff);
            scratch[slen++] = static_cast<char>((len >> 0) & 0xff);
        }

        assert(slen <= static_cast<int>(sizeof(scratch)));
        out.append(scratch, slen);

        if (_isMasking)
        { // flip some top bits - perhaps it helps.
            char mask[4];

            mask[0] = static_cast<char>(0x81);
            mask[1] = static_cast<char>(0x76);
            mask[2] = static_cast<char>(0x81);
            mask[3] = static_cast<char>(0x76);
            out.append(mask, 4);

            // copy and mask the data
            char copy[16384];
            ssize_t i = 0, toSend;
            while (true)
            {
                toSend = std::min(static_cast<uint64_t>(sizeof(copy)), len - i);
                if (toSend == 0)
                    break;
                for (ssize_t j = 0; j < toSend; ++j, ++i)
                    copy[j] = data[i] ^ mask[i%4];
                out.append(copy, toSend);
            }
        }
        else
        {
            // Copy the data.
            out.append(data, len);
        }
    }
#endif

    /// Sends a WebSocket frame given the data, length, and flags.
    /// Returns the number of bytes written (including frame overhead) on success,
    /// 0 for closed/invalid socket, and -1 for other errors.
    int sendFrame(const std::shared_ptr<StreamSocket>& socket, const char* data, const uint64_t len,
                  [[maybe_unused]] unsigned char flags, bool flush = true) const
    {
        if (!socket || data == nullptr || len == 0)
        {
            LOG_DBG("Socket or data missing. Cannot send WS frame");
            return -1;
        }

        if (!socket->isOpen())
        {
            LOG_DBG("Socket is not open. Cannot send WS frame");
            return 0;
        }

        ASSERT_CORRECT_SOCKET_THREAD(socket);
        Buffer& out = socket->getOutBuffer();

        LOGA_TRC(WebSocket, "WebSocketHandler: Writing " << len << " bytes to #" << socket->getFD()
                 << " in addition to " << out.size()
                 << " bytes buffered");

#if ENABLE_DEBUG
        if ((flags & 0xf) == int(WSOpCode::Text)) // utf8 validate
        {
            size_t offset = Util::isValidUtf8(reinterpret_cast<unsigned char const*>(data), len);
            if (offset < len)
            {
                std::string hex, raw;
                if (len < 256)
                {
                    raw = std::string(data, len);
                    hex = "whole string:" + HexUtil::dumpHex(raw);
                }
                else
                {
                    // 64 bytes before & after ...
                    size_t cropstart, croplen;
                    if (offset < 64)
                        cropstart = 0;
                    else
                        cropstart = offset - 64;
                    croplen = std::min<size_t>(len - cropstart, 128);
                    assert (cropstart + croplen <= len);
                    raw = std::string(data + cropstart, croplen);
                    hex = "msg: " + COOLProtocol::getAbbreviatedMessage(data, len) +
                          " string region error at byte " + std::to_string(offset - cropstart) +
                          ": " + HexUtil::dumpHex(raw);
                };
                std::cerr << "attempting to send invalid UTF-8 message '" << raw << "' "
                             " error at offset " << std::hex << "0x" << offset << std::dec
                          << " bytes, " << hex << '\n';
                assert(false && "invalid utf-8 - check Message::detectType()");
            }
        }
#endif

        // This would generate huge amounts of "instant" Trace Events. Is that what we want? If so,
        // it would be good to include in the args some identificating information about the sender
        // and recipient of the frame (and what message it is related to, if any).

        // TraceEvent::emitInstantEvent("WebSocketHandler::sendFrame", { { "length", std::to_string(len) } });

#if !MOBILEAPP
        const size_t oldSize = out.size();

        buildFrame(data, len, flags, out);

        // Return the number of bytes we wrote to the *buffer*.
        const size_t size = out.size() - oldSize;
#else
        // We ignore the flush parameter and always flush in the MOBILEAPP case because there is no
        // WebSocket framing, we put the messages as such into the FakeSocket queue.
        flush = true;
        out.append(data, len);
        const size_t size = out.size();
#endif

        assert(size >= len && "Expected to have data in outBuffer to send");

        if (flush || _shuttingDown)
        {
            socket->writeOutgoingData();

            // Retry if we are shutting down and failed.
            // This is particularly relevant when we simulate socket error
            // during unit-tests. Dropping WS frames results in random test failures.
            // But more important is to flush the data we have before closing the socket.
            // There is a FIXME item in Session::shutdown specifically to address this case.
            // When we terminate a client's connection in DocumentBroker::finalRemoveSession,
            // we send the close frame and close the socket via Socket::shutdownConnection(),
            // which is called immediately after *this* function (see shutdown() above).
            // So, a common scenario is when we want to shutdown all clients. The stack
            // trace looks like this:
            //
            // WebSocketHandler::sendFrame at ./net/WebSocketHandler.hpp:678 (this function)
            // WebSocketHandler::sendCloseFrame at ./net/WebSocketHandler.hpp:149
            // WebSocketHandler::shutdown at ./net/WebSocketHandler.hpp:175
            // WebSocketHandler::shutdown at ./net/WebSocketHandler.hpp:155
            // Session::shutdown at common/Session.cpp:235
            // Session::shutdownGoingAway at ./common/Session.hpp:152 (this will close the socket)
            // DocumentBroker::shutdownClients at wsd/DocumentBroker.cpp:2386
            // DocumentBroker::terminateChild at wsd/DocumentBroker.cpp:2421
            //
            // The proper fix is to flag the socket(s) for shutdown, but continue
            // polling until we completely flush the buffered data, then we close
            // the socket in question. This isn't possible in the above scenario,
            // and a proper fix is to modify DocumentBroker's poll to take this
            // flushing into account (note that currently terminateChild is called
            // *after* the poll loop exists). This will be done in a follow up later.
            // For now, we just do a second write, and hope for the best.
            if (_shuttingDown && !out.empty())
            {
                socket->writeOutgoingData();
                if (!out.empty())
                {
                    LOG_WRN("Shutting down but "
                            << out.size()
                            << " bytes couldn't be flushed and still remain in the output buffer");
                }
            }
        }

        return size;
    }

    static bool isControlFrame(WSOpCode code) { return code >= WSOpCode::Close; }

    static void readPayload(unsigned char *data, size_t dataLen, unsigned char* mask, std::vector<char>& payload)
    {
        if (dataLen == 0)
            return;

        if (mask)
        {
            size_t end = payload.size();
            payload.resize(end + dataLen);
            if (dataLen > 0)
            {
                char* wsData = &payload[end];
                for (size_t i = 0; i < dataLen; ++i)
                    *wsData++ = data[i] ^ mask[i % 4];
            }
        }
        else
            payload.insert(payload.end(), data, data + dataLen);
    }

    /// To be overridden to handle the websocket messages the way you need.
    virtual void handleMessage(const std::vector<char> &data)
    {
        if (_msgHandler)
            _msgHandler->handleMessage(data);
    }

    const std::weak_ptr<StreamSocket>& getSocket() const
    {
        return _socket;
    }

    void ignoreInput() const
    {
        std::shared_ptr<StreamSocket> socket = _socket.lock();
        if (socket)
            socket->ignoreInput();
    }

    /// Implementation of the ProtocolHandlerInterface.
    void dumpState(std::ostream& os, const std::string& indent) const override;

    static std::string generateKey();
    static std::string computeAccept(const std::string &key);

    /// Upgrade the http(s) connection to a websocket.
    template <typename T>
    void upgradeToWebSocket(const std::shared_ptr<StreamSocket>& socket,
                            [[maybe_unused]] const T& req,
                            [[maybe_unused]] bool allowedOrigin)
    {
        assert(socket && "Must have a valid socket");
        LOGA_TRC(WebSocket, "Upgrading to WebSocket");
        assert(!socket->isWebSocket());
        assert(!_isClient && "Accepting upgrade requests are done by servers only.");

#if !MOBILEAPP
        // create our websocket goodness ...
        const int wsVersion = NumUtil::stoi(req.get("Sec-WebSocket-Version", "13"));
        const std::string wsKey = req.get("Sec-WebSocket-Key", "");
        const std::string wsProtocol = req.get("Sec-WebSocket-Protocol", "chat");
        // FIXME: other sanity checks ...
        LOG_INF("WebSocket version: " << wsVersion << ", key: [" << wsKey << "], protocol: ["
                                      << wsProtocol << ']');

        /* SHOULD verify the Origin field is an origin they expect. If the origin indicated is
         * unacceptable to the server, then it SHOULD respond ... with a reply containing HTTP
         * 403 Forbidden status code.
         */
        if (!allowedOrigin)
        {
            LOG_ERR("Rejecting WebSocket upgrade due to disallowed origin");
            HttpHelper::sendErrorAndShutdown(http::StatusCode::Forbidden, socket);
            return;
        }
#if ENABLE_DEBUG
        if (std::getenv("COOL_ZERO_BUFFER_SIZE"))
            socket->setSocketBufferSize(0);
#endif

        http::Response httpResponse(http::StatusCode::SwitchingProtocols, socket->getFD());
        httpResponse.set("Upgrade", "websocket");
        httpResponse.setConnectionToken(http::Header::ConnectionToken::Upgrade);
        httpResponse.set("Sec-WebSocket-Accept", computeAccept(wsKey));
        LOGA_TRC(WebSocket, "Sending WS Upgrade response: " << httpResponse.header().toString());
        socket->send(httpResponse);
#endif
        setWebSocket(socket);
    }

#if !MOBILEAPP
    // Handle incoming upgrade to full socket as client WS.
    void handleClientUpgrade(const std::shared_ptr<StreamSocket>& socket)
    {
        assert(socket && "socket must be valid");
        assert(_isClient && "Upgrade handshakes are finished by clients.");

        Buffer& data = socket->getInBuffer();

        LOGA_TRC(WebSocket, "Incoming client websocket upgrade response: "
                 << std::string(data.data(), data.size()));

        // Consume the incoming data by parsing and processing the body.
        http::Response response(
            [&]()
            {
                if (response.statusLine().statusCode() == http::StatusCode::SwitchingProtocols &&
                    Util::iequal(response.get("Upgrade"), "websocket") &&
                    response.header().getConnectionToken() == http::Header::ConnectionToken::Upgrade &&
                    response.get("Sec-WebSocket-Accept", "") == computeAccept(_key))
                {
                    LOGA_TRC(WebSocket, "Accepted incoming websocket response");
                    setWebSocket(socket);
                }
                else
                {
                    LOG_ERR("Server returned invalid accept token during handshake. Disconnecting");
                    socket->asyncShutdown();
                }
            },
            socket->getFD());

        const int64_t read = response.readData(data.data(), data.size());
        if (read < 0)
        {
            // Error: Interrupt the transfer.
            LOG_ERR("Error in client websocket upgrade response. Disconnecting");
            socket->asyncShutdown();
            return;
        }

        if (read > 0)
        {
            // Remove consumed data.
            socket->eraseFirstInputBytes(read);
            return;
        }

        // Nothing to do, not enough data to parse.
        assert(read == 0 && "Need more more data to parse.");
    }
#endif

    void setWebSocket(const std::shared_ptr<StreamSocket>& socket)
    {
        assert(socket && "Must have a valid socket");
        socket->setWebSocket();
        // No need to ping right upon connection/upgrade,
        // but do reset the time to avoid pinging immediately after.
        _lastPingSentTime = std::chrono::steady_clock::now();
    }

    void enableProcessInput(bool enable = true) override
    {
        std::shared_ptr<StreamSocket> socket = _socket.lock();
        if (socket)
            socket->enableProcessInput(enable);
    }

    virtual void gotPing(WSOpCode /* code */, int /* pingTimeUs */)
    {
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
