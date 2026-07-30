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
 * HAProxy protocol support for client IP forwarding.
 * Functions: parseProxyHeader()
 */

#pragma once

#include <common/StateEnum.hpp>
#include <net/Socket.hpp>

#include <map>
#include <memory>

/**
 * Implementation that builds a websocket like protocol from many
 * individual proxied HTTP requests back to back.
 *
 * we use a trivial framing: [T(ext)|B(inary)]<hex-serial->\n<hex-length>\n<content>\n
 */
class ProxyProtocolHandler final : public ProtocolHandlerInterface
{
public:
    ProxyProtocolHandler() :
        _inSerial(1),
        _outSerial(1)
    {
    }

    /// Will be called exactly once by setHandler
    void onConnect(const std::shared_ptr<StreamSocket>& /* socket */) override {}

    /// Called after successful socket reads.
    void handleIncomingMessage(SocketDisposition &/* disposition */) override;

    int getPollEvents(std::chrono::steady_clock::time_point /* now */,
                      int64_t &/* timeoutMaxMs */) override;

    void performWrites(std::size_t capacity) override;

    void onDisconnect() override
    {
        // connections & sockets come and go a lot.
    }

    STATE_ENUM(
        ParseStatus,
        AGAIN,         // we need to wait for more data to arrive
        COMPLETE,      // we got a complete stream, and we're done
        PROTOCOL_ERROR // the stream is mangled - terminate ...
    );

    /// Clear all external references
    void dispose() override { _msgHandler.reset(); }

    int sendTextMessage(std::string_view msg, bool flush = false) const override;
    int sendBinaryMessage(std::string_view data, bool flush = false) const override;
    void shutdown(bool goingAway = false,
                  const std::string_view statusMessage = std::string_view()) override;
    void getIOStats(uint64_t &sent, uint64_t &recv) override;
    // don't duplicate ourselves for every socket
    void dumpState(std::ostream&, const std::string&) const override {}
    // instead do it centrally.
    void dumpProxyState(std::ostream& os);

    // Non-destructive message parsing
    ParseStatus parseEmitIncoming(const std::shared_ptr<StreamSocket>& socket);
    static bool hasCompleteMessage(const Buffer& in);
    void processBufferedMessages();

    void handleRequest(const std::shared_ptr<StreamSocket> &socket);
    void sendAndClose(const std::shared_ptr<StreamSocket> &socket);

    /// tell our handler we've received a close.
    void notifyDisconnected();

private:
    std::shared_ptr<StreamSocket> popOutSocket();
    /// can we find anything to send back if we try ?
    bool slurpHasMessages(std::size_t capacity);
    int sendMessage(const char* msg, size_t len, bool text, bool flush);
    bool flushQueueTo(const std::shared_ptr<StreamSocket> &socket);

    struct Message : public std::vector<char>
    {
        Message(const char *msg, const std::size_t len, bool text, uint64_t serial)
        {
            const char *type = text ? "T" : "B";
            insert(end(), type, type + 1);
            std::ostringstream os;
            os << std::hex << "0x" << serial << "\n0x" << len << '\n';
            const std::string str = os.str();
            insert(end(), str.c_str(), str.c_str() + str.size());
            insert(end(), msg, msg + len);
            static constexpr const char *terminator = "\n";
            insert(end(), terminator, terminator + 1);
        }
    };

    struct BufferedMessage
    {
        uint64_t serial;
        std::vector<char> data;

        BufferedMessage(uint64_t s, const std::vector<char>& d)
            : serial(s)
            , data(d)
        {
        }
    };

    std::map<uint64_t, std::unique_ptr<BufferedMessage>> _serialQueue;

    /// queue things when we have no socket to hand.
    std::vector<std::shared_ptr<Message>> _writeQueue;
    std::vector<std::weak_ptr<StreamSocket>> _outSockets;
    uint64_t _inSerial;
    uint64_t _outSerial;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
