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
 * The ProxyProtocol creates a web-socket like connection over HTTP
 * requests. URLs are formed like this:
 *      0              1           2      3          4         5
 *   /cool/<encoded-document-url>/ws/<session-id>/<command>/<serial>
 * <session-id> can be 'unknown'
 * <command> can be 'open', 'write', 'wait', or 'close'
 */

#include <config.h>

#include "ProxyProtocol.hpp"

#include <common/Util.hpp>
#include <net/Socket.hpp>
#include <wsd/COOLWSD.hpp>
#include <wsd/ClientSession.hpp>
#include <wsd/DocumentBroker.hpp>
#include <wsd/Exceptions.hpp>

#include <memory>
#include <string>

void DocumentBroker::handleProxyRequest(
    const std::string& id,
    const Poco::URI& uriPublic,
    const bool isReadOnly,
    const RequestDetails &requestDetails,
    const std::shared_ptr<StreamSocket> &socket)
{
    std::shared_ptr<ClientSession> clientSession;
    if (requestDetails.equals(RequestDetails::Field::Command, "open"))
    {
        return proxyOpenRequest(socket, clientSession, id, uriPublic, isReadOnly, requestDetails);
    }

    const std::string sessionId = requestDetails.getField(RequestDetails::Field::SessionId);
    LOG_TRC("proxy: find session for " << _docKey << " with id " << sessionId);
    for (const auto& it : _sessions)
    {
        if (it.second->getOrCreateProxyAccess() == sessionId)
        {
            clientSession = it.second;
            break;
        }
    }

    if (!clientSession)
    {
        LOG_ERR("Invalid session id used " << sessionId);
        throw BadRequestException("invalid session id");
    }

    auto protocol = clientSession->getProtocol();
    socket->setHandler(protocol);

    // this DocumentBroker's poll handles reading & writing
    addSocketToPoll(socket);

    auto proxy = std::static_pointer_cast<ProxyProtocolHandler>(protocol);
    if (requestDetails.equals(RequestDetails::Field::Command, "close"))
    {
        LOG_TRC("Close session");
        proxy->notifyDisconnected();
        return;
    }

    proxy->handleRequest(socket);
}

void DocumentBroker::proxyOpenRequest(const std::shared_ptr<StreamSocket>& socket,
                                      std::shared_ptr<ClientSession>& clientSession,
                                      const std::string& id, const Poco::URI& uriPublic,
                                      const bool isReadOnly, const RequestDetails& requestDetails)
{
    const bool isLocal = Util::isDebugEnabled() ? true : socket->isLocal();

    LOG_TRC("proxy: validate that socket is from localhost: " << isLocal);
    if (!isLocal)
        throw BadRequestException("invalid host - only connect from localhost");

    LOG_TRC("proxy: Create session for " << _docKey);
    clientSession = createNewClientSession(std::make_shared<ProxyProtocolHandler>(), id,
                                            uriPublic, isReadOnly, requestDetails);
    if (!clientSession)
    {
        LOG_ERR("proxy: Failed to create session");
        throw BadRequestException("Invalid session details");
    }
    addSession(clientSession);
    COOLWSD::checkDiskSpaceAndWarnClients(true);
    COOLWSD::checkSessionLimitsAndWarnClients();

    const std::string& sessionId = clientSession->getOrCreateProxyAccess();
    LOG_TRC("proxy: Returning sessionId " << sessionId);

    http::Response httpResponse(http::StatusCode::OK);
    httpResponse.set("Last-Modified", Util::getHttpTimeNow());
    httpResponse.add("X-Content-Type-Options", "nosniff");
    httpResponse.set("Connection", "close");
    httpResponse.setBody(sessionId, "application/json; charset=utf-8");

    socket->send(httpResponse);
    socket->asyncShutdown();
}

bool ProxyProtocolHandler::hasCompleteMessage(const Buffer& in)
{
    if (in.size() < 2)
    {
        // Beware - the terminator
        if (in.size() == 1 && in[0] == '.')
            return true;
        return false;
    }

    // Find serial end
    size_t pos = 1;
    while (pos < in.size() && in[pos] != '\n')
        pos++;
    if (pos >= in.size())
        return false;
    pos++; // Skip \n

    // Find length end
    size_t lengthStart = pos;
    while (pos < in.size() && in[pos] != '\n')
        pos++;
    if (pos >= in.size())
        return false;

    char lengthStr[32];
    size_t lengthFieldSize = pos - lengthStart;
    if (lengthFieldSize >= sizeof(lengthStr))
        return false;

    std::memcpy(lengthStr, in.data() + lengthStart, lengthFieldSize);
    lengthStr[lengthFieldSize] = '\0';
    uint64_t contentLength = strtoull(lengthStr, nullptr, 16);

    pos++; // Skip length's \n
    size_t frameSize = pos + contentLength + 1; // +1 for final \n
    if (in.size() >= frameSize)
        return true;

    return false;
}

ProxyProtocolHandler::ParseStatus
ProxyProtocolHandler::parseEmitIncoming(const std::shared_ptr<StreamSocket>& socket)
{
    Buffer& in = socket->getInBuffer();

#if 0 // protocol debugging.
    std::ostringstream oss(Util::makeDumpStateStream());
    socket->dumpState(oss);
    LOG_TRC("Parse message:\n" << oss.str());
#endif
    while (in.size() > 0)
    {
        if (!hasCompleteMessage(in))
        {
            LOG_TRC("proxy: incomplete message with input " << in.size());
            return ParseStatus::AGAIN; // Wait for more data
        }

        if (in[0] == '.')
        {
            // Consume the terminator too, so the input buffer is empty once the reply is sent.
            in.eraseFirst(1);
            return ParseStatus::COMPLETE;
        }

        if (in[0] != 'T' && in[0] != 'B')
        {
            LOG_ERR("Invalid message type " << in[0]);
            return ParseStatus::PROTOCOL_ERROR;
        }

        assert(in.size() >= 2);
        auto it = in.begin() + 1;

        for (; it != in.end() && *it != '\n'; ++it)
            ;
        *it = '\0';

        uint64_t serial = strtoull(&in[1], nullptr, 16);
        in.erase(in.begin(), it + 1);

        it = in.begin();
        for (; it != in.end() && *it != '\n'; ++it)
            ;
        *it = '\0';
        uint64_t len = strtoull(in.data(), nullptr, 16);
        in.erase(in.begin(), it + 1);

        std::vector<char> data(in.begin(), in.begin() + len);
        in.eraseFirst(len);

        if (in.size() < 1 || in[0] != '\n')
            return ParseStatus::PROTOCOL_ERROR;
        in.eraseFirst(1);

        LOG_TRC("Adding message with serial[" << serial << "] to queue");
        _serialQueue[serial] = std::make_unique<BufferedMessage>(serial, data);
        processBufferedMessages();
    }

    return ParseStatus::AGAIN;
}

void ProxyProtocolHandler::processBufferedMessages()
{
    while (!_serialQueue.empty())
    {
        auto it = _serialQueue.find(_inSerial);
        if (it == _serialQueue.end())
        {
            LOG_DBG("proxy: cannot find serial[" << (_inSerial + 1) << "] in queue of size " << _serialQueue.size());
            break;
        }

        // Process the buffered message
        LOG_TRC("Processing serial[" << it->second->serial << ']');
        _inSerial++;

        if (_msgHandler)
            _msgHandler->handleMessage(it->second->data);

        _serialQueue.erase(it);
    }
}

void ProxyProtocolHandler::handleRequest(const std::shared_ptr<StreamSocket> &streamSocket)
{
    LOG_INF("proxy: handle request on socket #" << streamSocket->getFD());

    if (!_msgHandler)
        LOG_WRN("proxy: unusual - incoming message with no-one to handle it");
    else
    {
        ParseStatus result = parseEmitIncoming(streamSocket);
        switch (result)
        {
        case ParseStatus::COMPLETE:
            sendAndClose(streamSocket);
            break;
        case ParseStatus::AGAIN:
            break;
        case ParseStatus::PROTOCOL_ERROR:
        {
            std::ostringstream oss(Util::makeDumpStateStream());
            streamSocket->dumpState(oss);
            // TODO: We should shut down this connection and client session quite hard
            LOG_ERR("proxy: bad socket structure " << oss.str());
            break;
        }
        };
    }
}

void ProxyProtocolHandler::sendAndClose(const std::shared_ptr<StreamSocket> &streamSocket)
{
    bool sentMsg = flushQueueTo(streamSocket);
    // FIXME: we should really restore a blocking 'wait' message
    if (!sentMsg)
    {
        LOG_TRC("Nothing to send - closing immediately");

        http::Response httpResponse(http::StatusCode::OK);
        httpResponse.set("Last-Modified", Util::getHttpTimeNow());
        httpResponse.add("X-Content-Type-Options", "nosniff");
        httpResponse.setContentLength(0);
        streamSocket->send(httpResponse);
    }
    else
        LOG_TRC("Returned a reply immediately");

    streamSocket->asyncShutdown();
}

// This continues reading input for larger messages that get split up
void ProxyProtocolHandler::handleIncomingMessage(SocketDisposition &disposition)
{
    auto streamSocket = std::static_pointer_cast<StreamSocket>(disposition.getSocket());
    ParseStatus result = parseEmitIncoming(streamSocket);
    switch (result)
    {
    case ParseStatus::PROTOCOL_ERROR:
    {
        std::ostringstream oss(Util::makeDumpStateStream());
        streamSocket->dumpState(oss);
        LOG_ERR("proxy: bad socket structure " << oss.str());
        disposition.setClosed();
        break;
    }
    case ParseStatus::COMPLETE:
        sendAndClose(streamSocket);
        break;
    case ParseStatus::AGAIN:
        break;
    };
}

void ProxyProtocolHandler::notifyDisconnected()
{
    if (_msgHandler)
        _msgHandler->onDisconnect();
}

int ProxyProtocolHandler::sendMessage(const char *msg, const size_t len, bool text, bool flush)
{
    _writeQueue.push_back(std::make_shared<Message>(msg, len, text, _outSerial++));
    if (flush)
    {
        auto sock = popOutSocket();
        if (sock)
        {
            flushQueueTo(sock);
            sock->asyncShutdown();
        }
    }

    return len;
}

int ProxyProtocolHandler::sendTextMessage(std::string_view msg, bool flush) const
{
    ASSERT_CORRECT_THREAD();
    LOG_TRC("ProxyHack - send text msg " << msg);
    return const_cast<ProxyProtocolHandler *>(this)->sendMessage(msg.data(), msg.size(), true, flush);
}

int ProxyProtocolHandler::sendBinaryMessage(const std::string_view data, bool flush) const
{
    ASSERT_CORRECT_THREAD();
    LOG_TRC("ProxyHack - send binary msg len " << data.size());
    return const_cast<ProxyProtocolHandler *>(this)->sendMessage(data.data(), data.size(), false, flush);
}

void ProxyProtocolHandler::shutdown(bool goingAway, const std::string_view statusMessage)
{
    LOG_TRC("ProxyHack - shutdown " << goingAway << ": " << statusMessage);
}

void ProxyProtocolHandler::getIOStats(uint64_t &sent, uint64_t &recv)
{
    sent = recv = 0;
}

void ProxyProtocolHandler::dumpProxyState(std::ostream& os)
{
    os << "proxy protocol sockets: " << _outSockets.size() << " writeQueue: " << _writeQueue.size() << ":\n";
    os << '\t';
    for (const auto &it : _outSockets)
    {
        auto sock = it.lock();
        os << '#' << (sock ? sock->getFD() : -2) << ' ';
    }
    os << '\n';
    for (const auto& it : _writeQueue)
        HexUtil::dumpHex(os, *it, "\twrite queue entry:", "\t\t");
    if (_msgHandler)
        _msgHandler->dumpState(os);
}

int ProxyProtocolHandler::getPollEvents(std::chrono::steady_clock::time_point /* now */,
                                        int64_t &/* timeoutMaxMs */)
{
    ASSERT_CORRECT_THREAD();
    int events = POLLIN;
    if (_msgHandler && _msgHandler->hasQueuedMessages())
        events |= POLLOUT;
    return events;
}

/// slurp from the core to us, @returns true if there are messages to send
bool ProxyProtocolHandler::slurpHasMessages(std::size_t capacity)
{
    if (_msgHandler)
        _msgHandler->writeQueuedMessages(capacity);

    return _writeQueue.size() > 0;
}

void ProxyProtocolHandler::performWrites(std::size_t capacity)
{
    ASSERT_CORRECT_THREAD();
    if (!slurpHasMessages(capacity))
        return;

    auto sock = popOutSocket();
    if (sock)
    {
        LOG_TRC("proxy: performWrites");
        flushQueueTo(sock);
        sock->asyncShutdown();
    }
}

bool ProxyProtocolHandler::flushQueueTo(const std::shared_ptr<StreamSocket> &socket)
{
    if (!slurpHasMessages(socket->getSendBufferCapacity()))
        return false;

    size_t totalSize = 0;
    for (const auto& it : _writeQueue)
        totalSize += it->size();

    if (!totalSize)
        return false;

    LOG_TRC("proxy: flushQueue of size " << totalSize << " to socket #" << socket->getFD() << " & close");

    uint binaryDataCount = 0, textDataCount = 0;
    for (const auto& it : _writeQueue)
    {
        char messageType = it->front();
        if (messageType == 'T')
            textDataCount++;
        else if (messageType == 'B')
            binaryDataCount++;
    }

    http::Response httpResponse(http::StatusCode::OK);
    httpResponse.set("Last-Modified", Util::getHttpTimeNow());
    httpResponse.add("X-Content-Type-Options", "nosniff");
    httpResponse.setContentLength(totalSize);
    if (textDataCount >= binaryDataCount)
        httpResponse.set("Content-Type", "text/plain; charset=utf-8");
    else
        httpResponse.set("Content-Type", "application/octet-stream");

    socket->send(httpResponse);

    for (const auto& it : _writeQueue)
        socket->send(it->data(), it->size(), false);
    _writeQueue.clear();

    return true;
}

// LRU-ness ...
std::shared_ptr<StreamSocket> ProxyProtocolHandler::popOutSocket()
{
    std::weak_ptr<StreamSocket> sock;
    while (!_outSockets.empty())
    {
        sock = _outSockets.front();
        _outSockets.erase(_outSockets.begin());
        auto realSock = sock.lock();
        if (realSock)
        {
            LOG_TRC("proxy: popped an out socket #" << realSock->getFD() << " leaving: " << _outSockets.size());
            return realSock;
        }
    }
    LOG_TRC("proxy: no out sockets to pop.");
    return std::shared_ptr<StreamSocket>();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
