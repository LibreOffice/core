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

#include <config.h>
#include <config_version.h>

#include "Socket.hpp"

#include <common/HexUtil.hpp>
#include <common/Log.hpp>
#include <common/SigUtil.hpp>
#include <common/Syscall.hpp>
#include <common/Unit.hpp>
#include <common/Util.hpp>
#include <net/HttpRequest.hpp>
#include <net/NetUtil.hpp>
#include <net/ServerSocket.hpp>
#include <net/WebSocketHandler.hpp>

#include <cerrno>
#include <chrono>
#include <cstring>
#include <memory>
#include <ostream>
#include <sstream>
#include <string>

#include <sys/stat.h>
#include <sys/types.h>

#ifndef _WIN32
#include <sysexits.h>
#include <unistd.h>
#include <sys/un.h>
#endif

#ifdef __FreeBSD__
#include <sys/ucred.h>
#endif

#include <Poco/MemoryStream.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/NetException.h>
#include <Poco/URI.h>

#ifndef __APPLE__
#define SOCKET_ABSTRACT_UNIX_NAME "coolwsd-"
#else
#define SOCKET_ABSTRACT_UNIX_NAME "/tmp/coolwsd-"
#endif

bool StreamSocket::socketpair(const std::chrono::steady_clock::time_point creationTime,
                              std::shared_ptr<StreamSocket>& parent,
                              std::shared_ptr<StreamSocket>& child)
{
    int pair[2];
    int rc = Syscall::socketpair_cloexec_nonblock(AF_UNIX, SOCK_STREAM /*| SOCK_NONBLOCK | SOCK_CLOEXEC*/, 0, pair);
    if (rc != 0)
        return false;
    child = std::make_shared<StreamSocket>("save-child", pair[0], Socket::Type::Unix, true, HostType::Other, ReadType::NormalRead, creationTime);
    child->setNoShutdown();
    child->setClientAddress("save-child");
    child->resetThreadOwner(); // The parent will set the owner when it inserts into its poller.
    parent = std::make_shared<StreamSocket>("save-kit-parent", pair[1], Socket::Type::Unix, true, HostType::Other, ReadType::NormalRead, creationTime);
    parent->setNoShutdown();
    parent->setClientAddress("save-parent");
    parent->resetThreadOwner(); // The child will set the owner when it inserts into its poller.

    return true;
}

bool ServerSocket::isUnrecoverableAcceptError(const int cause) const
{
    static constexpr const char * messagePrefix = "Failed to accept. (errno: ";
    switch(cause)
    {
        case EINTR:
        case EAGAIN:        // == EWOULDBLOCK
        case ENETDOWN:
        case EPROTO:
        case ENOPROTOOPT:
        case EHOSTDOWN:
#ifdef ENONET
        case ENONET:
#endif
        case EHOSTUNREACH:
        case EOPNOTSUPP:
        case ENETUNREACH:
        case ECONNABORTED:
        case ETIMEDOUT:
        case EMFILE:
        case ENFILE:
        case ENOMEM:
        case ENOBUFS:
        {
            LOG_DBG(messagePrefix << Util::symbolicErrno(cause) << ", " << std::strerror(cause)
                                  << ')');
            return false;
        }
        default:
        {
            LOG_FTL(messagePrefix << Util::symbolicErrno(cause) << ", " << std::strerror(cause)
                                  << ')');
            return true;
        }
    }
}

int Socket::getPid() const
{
    int pid = Syscall::get_peer_pid(_fd);
    if (pid < 0)
        LOG_SYS("Failed to get pid via peer creds on " << _fd);

    return pid;
}

// Does this socket come from the localhost ?
bool Socket::isLocal() const
{
    if (_clientAddress.size() < 1)
        return false;
    if (_clientAddress[0] == '/') // Unix socket
        return true;
    if (_clientAddress == "::1")
        return true;
    return  _clientAddress.rfind("::ffff:127.0.0.", 0) != std::string::npos ||
                _clientAddress.rfind("127.0.0.", 0) != std::string::npos;
}

std::shared_ptr<Socket> LocalServerSocket::accept()
{
    const int rc = Syscall::accept_cloexec_nonblock(getFD(), nullptr, nullptr);
    if (rc < 0)
    {
        if (isUnrecoverableAcceptError(errno))
            Util::forcedExit(EX_SOFTWARE);
        return nullptr;
    }
    try
    {
        LOG_DBG("Accepted prisoner socket #" << rc << ", creating socket object.");

        std::shared_ptr<Socket> _socket = createSocketFromAccept(rc, Socket::Type::Unix);
        // Sanity check this incoming socket
#ifdef __linux__
#define CREDS_UID(c) c.uid
#define CREDS_GID(c) c.gid
#define CREDS_PID(c) c.pid
        struct ucred creds;
        socklen_t credSize = sizeof(struct ucred);
        if (getsockopt(rc, SOL_SOCKET, SO_PEERCRED, &creds, &credSize) < 0)
        {
            LOG_SYS("Failed to get peer creds on " << rc);
            ::close(rc);
            return std::shared_ptr<Socket>(nullptr);
        }
#elif defined(__FreeBSD__)
#define CREDS_UID(c) c.cr_uid
#define CREDS_GID(c) c.cr_groups[0]
#define CREDS_PID(c) c.cr_pid
        struct xucred creds;
        socklen_t credSize = sizeof(struct xucred);
        if (getsockopt(rc, SOL_LOCAL, LOCAL_PEERCRED, &creds, &credSize) < 0)
        {
            LOG_SYS("Failed to get peer creds on " << rc);
            ::close(rc);
            return std::shared_ptr<Socket>(nullptr);
        }
#elif defined(__APPLE__)

        // On macOS, there's no single struct for all three,
        // so define our own 'apple_creds' combining UID/GID/PID.
        struct apple_creds {
            uid_t uid;
            gid_t gid;
            pid_t pid;
        } creds;

        // Macros to unify usage in the rest of the code:
        #define CREDS_UID(c)  ((c).uid)
        #define CREDS_GID(c)  ((c).gid)
        #define CREDS_PID(c)  ((c).pid)

        // Get the effective UID/GID via getpeereid():
        if (getpeereid(rc, &creds.uid, &creds.gid) != 0)
        {
            LOG_SYS("Failed to get peer creds (uid/gid) on " << rc);
            ::close(rc);
            return std::shared_ptr<Socket>(nullptr);
        }

        // Get the peer PID via LOCAL_PEERPID:
        socklen_t pidLen = sizeof(creds.pid);
        if (getsockopt(rc, SOL_LOCAL, LOCAL_PEERPID, &creds.pid, &pidLen) < 0)
        {
            LOG_SYS("Failed to get peer pid on " << rc);
            ::close(rc);
            return std::shared_ptr<Socket>(nullptr);
        }
#else
#error Implement for your platform
#endif

        uid_t uid = getuid();
        uid_t gid = getgid();
        if (CREDS_UID(creds) != uid || CREDS_GID(creds) != gid)
        {
            LOG_ERR("Peercred mis-match on domain socket - closing connection. uid: " <<
                    CREDS_UID(creds) << "vs." << uid << " gid: " << CREDS_GID(creds) << "vs." << gid);
            ::close(rc);
            return std::shared_ptr<Socket>(nullptr);
        }
        std::string addr("uds-to-pid-");
        addr.append(std::to_string(CREDS_PID(creds)));
        _socket->setClientAddress(addr);

        LOG_DBG("Accepted socket #" << rc << " is UDS - address " << addr << " and uid/gid "
                                    << CREDS_UID(creds) << '/' << CREDS_GID(creds));
        return _socket;
    }
    catch (const std::exception& ex)
    {
        LOG_ERR("Failed to create client socket #" << rc << ". Error: " << ex.what());
    }
    return nullptr;
}

/// Returns true on success only.
UnxSocketPath LocalServerSocket::bind()
{
    int rc;
    struct sockaddr_un addrunix;

    // snap needs a specific socket name
    std::string socketAbstractUnixName(SOCKET_ABSTRACT_UNIX_NAME);
    const char* snapInstanceName = std::getenv("SNAP_INSTANCE_NAME");
    if (snapInstanceName && snapInstanceName[0])
        socketAbstractUnixName = std::string("snap.") + snapInstanceName + ".coolwsd-";

    LOG_INF("Binding to Unix socket for local server with base name: " << socketAbstractUnixName);

    constexpr auto RandomSuffixLength = 8;
    constexpr auto MaxSocketAbstractUnixNameLength =
        sizeof(addrunix.sun_path) - RandomSuffixLength - 2; // 1 byte for null termination, 1 byte for abstract's leading \0
    LOG_ASSERT_MSG(socketAbstractUnixName.size() < MaxSocketAbstractUnixNameLength,
                   "SocketAbstractUnixName is too long. Max: " << MaxSocketAbstractUnixNameLength
                                                               << ", actual: "
                                                               << socketAbstractUnixName.size());

    int last_errno = 0;
    do
    {
        std::memset(&addrunix, 0, sizeof(addrunix));
        addrunix.sun_family = AF_UNIX;

        const std::string socketName = socketAbstractUnixName + Util::rng::getFilename(RandomSuffixLength);
        UnxSocketPath socketPath(socketName);
        socketPath.fillInto(addrunix);

        rc = ::bind(getFD(), reinterpret_cast<const sockaddr *>(&addrunix), sizeof(struct sockaddr_un));
        last_errno = errno;
        LOG_TRC("Binding to Unix socket location ["
                << socketPath << "], result: " << rc
                << ((rc >= 0) ? std::string()
                              : '\t' + Util::symbolicErrno(last_errno) + ": " +
                                    std::strerror(last_errno)));
        if (rc >= 0)
        {
            _id = socketPath;
            return socketPath;
        }
    } while (rc < 0 && errno == EADDRINUSE);

    LOG_ERR_ERRNO(last_errno, "Failed to bind to Unix socket");
    return std::string();
}

bool LocalServerSocket::linkTo([[maybe_unused]] const std::string& toPath)
{
#ifndef HAVE_ABSTRACT_UNIX_SOCKETS
    _linkName = toPath + "/" + _id.getName();
    return 0 == ::link(_id.getName().c_str(), _linkName.c_str());
#else
    return true;
#endif
}

LocalServerSocket::~LocalServerSocket()
{
#ifndef HAVE_ABSTRACT_UNIX_SOCKETS
    ::unlink(_id.getName().c_str());
    if (!_linkName.empty())
        ::unlink(_linkName.c_str());
#endif
}

// For a verbose life, tweak here:
#if 0
#  define LOG_CHUNK(X) LOG_TRC(X)
#else
#  define LOG_CHUNK(X)
#endif

ssize_t StreamSocket::readHeader(const std::string_view clientName, std::istream& message,
                                 size_t messagesize,
                                 Poco::Net::HTTPRequest& request,
                                 std::chrono::duration<float, std::milli> delayMs)
{
    static constexpr std::chrono::duration<float, std::milli> delayMax =
        std::chrono::duration_cast<std::chrono::milliseconds>(SocketPoll::DefaultPollTimeoutMicroS);

    // Find the end of the header, if any.
    static constexpr std::string_view marker("\r\n\r\n");
    if (!Util::seekToMatch(message, marker))
    {
        LOG_TRC("parseHeader: " << clientName << " doesn't have enough data for the header yet. delay " << delayMs.count() << "ms");
        return -1;
    }

    // Skip the marker.
    ssize_t headerSize = static_cast<ssize_t>(message.tellg()) + marker.size();
    message.seekg(0, std::ios_base::beg);

    try
    {
        request.read(message);
    }
    catch (const Poco::Net::NotAuthenticatedException& exc)
    {
        LOG_DBG("parseHeader: Exception caught with "
                << messagesize << " bytes, shutdown: " << exc.displayText() << ", delay "
                << delayMs.count() << "ms");
        asyncShutdown();
        return -1;
    }
    catch (const Poco::Net::UnsupportedRedirectException& exc)
    {
        LOG_DBG("parseHeader: Exception caught with "
                << messagesize << " bytes, shutdown: " << exc.displayText() << ", delay "
                << delayMs.count() << "ms");
        asyncShutdown();
        return -1;
    }
    catch (const Poco::Net::HTTPException& exc)
    {
        LOG_DBG("parseHeader: Exception caught with "
                << messagesize << " bytes, shutdown: " << exc.displayText() << ", delay "
                << delayMs.count() << "ms");
        asyncShutdown();
        return -1;
    }
    catch (const Poco::Exception& exc)
    {
        if (delayMs > delayMax)
        {
            LOG_DBG("parseHeader: Exception caught with "
                    << messagesize << " bytes, shutdown: " << exc.displayText() << ", delay "
                    << delayMs.count() << "ms");
            asyncShutdown();
        }
        else
        {
            LOG_DBG("parseHeader: Exception caught with "
                    << messagesize << " bytes, continue: " << exc.displayText() << ", delay "
                    << delayMs.count() << "ms");
        }
        return -1;
    }
    catch (const std::exception& exc)
    {
        if (delayMs > delayMax)
        {
            LOG_DBG("parseHeader: Exception caught with "
                    << messagesize << " bytes, shutdown: " << exc.what() << ", delay "
                    << delayMs.count() << "ms");
            asyncShutdown();
        }
        else
        {
            LOG_DBG("parseHeader: Exception caught with "
                    << messagesize << " bytes, continue: " << exc.what() << ", delay "
                    << delayMs.count() << "ms");
        }
        return -1;
    }

    return headerSize;
}

void StreamSocket::handleExpect(const std::string_view expect)
{
    if (!_sentHTTPContinue && Util::iequal(expect, "100-continue"))
    {
        LOG_TRC("parseHeader: Got Expect: 100-continue, sending Continue");
        // FIXME: should validate authentication headers early too.
        send("HTTP/1.1 100 Continue\r\n\r\n",
             sizeof("HTTP/1.1 100 Continue\r\n\r\n") - 1);
        _sentHTTPContinue = true;
    }
}

bool StreamSocket::checkChunks(const Poco::Net::HTTPRequest& request, size_t headerSize, MessageMap& map,
                               std::chrono::duration<float, std::milli> delayMs)
{
    if (!request.getChunkedTransferEncoding())
        return true;

    auto itBody = _inBuffer.begin() + headerSize;

    // keep the header
    map._spans.emplace_back(0, headerSize);

    int chunk = 0;
    while (itBody != _inBuffer.end())
    {
        const auto chunkStart = itBody;

        // skip whitespace
        for (; itBody != _inBuffer.end() && isascii(*itBody) && isspace(*itBody); ++itBody)
            ; // skip.

        // each chunk is preceded by its length in hex.
        size_t chunkLen = 0;
        bool haveHexDigits = false;
        for (; itBody != _inBuffer.end(); ++itBody)
        {
            int digit = HexUtil::hexDigitFromChar(*itBody);
            if (digit >= 0)
            {
                haveHexDigits = true;
                chunkLen = chunkLen * 16 + digit;
                if (chunkLen > http::MaxChunkLen)
                {
                    LOG_ERR("Invalid chunk length (" << chunkLen << ") exceeds limit of "
                                                     << http::MaxChunkLen / 1024 / 1024 << " MB");
                    return false;
                }
            }
            else
                break;
        }

        LOG_CHUNK("parseHeader: Chunk of length " << chunkLen);

        if (chunkLen == 0 && !haveHexDigits)
        {
            LOG_ERR("Invalid chunk with no length");
            return false;
        }

        for (; itBody != _inBuffer.end() && *itBody != '\n'; ++itBody)
            ; // skip to end of line

        if (itBody != _inBuffer.end())
            itBody++; /* \n */;

        // skip the chunk.
        const auto chunkOffset = itBody - _inBuffer.begin();
        const auto chunkAvailable = _inBuffer.size() - chunkOffset;

        if (chunkLen == 0) // we're complete.
        {
            map._messageSize = chunkOffset;
            return true;
        }

        if (chunkLen + 2 > chunkAvailable)
        {
            LOG_DBG("parseHeader: Not enough content yet in chunk " << chunk <<
                    " starting at offset " << (chunkStart - _inBuffer.begin()) <<
                    " chunk len: " << chunkLen << ", available: " << chunkAvailable << ", delay " << delayMs.count() << "ms");
            return false;
        }
        itBody += chunkLen;

        map._spans.emplace_back(chunkOffset, chunkLen);

        if (*itBody != '\r' || *(itBody + 1) != '\n')
        {
            LOG_ERR("parseHeader: Missing \\r\\n at end of chunk " << chunk << " of length " << chunkLen << ", delay " << delayMs.count() << "ms");
            LOG_CHUNK("Chunk " << chunk << " is: \n"
                               << HexUtil::dumpHex("", "", chunkStart, itBody + 1, false));
            asyncShutdown();
            return false; // TODO: throw something sensible in this case
        }

        LOG_CHUNK("parseHeader: Chunk "
                  << chunk << " is: \n"
                  << HexUtil::dumpHex("", "", chunkStart, itBody + 1, false));

        itBody+=2;
        chunk++;
    }
    LOG_TRC("parseHeader: Not enough chunks yet, so far " << chunk << " chunks of total length " << (itBody - _inBuffer.begin()) << ", delay " << delayMs.count() << "ms");
    return false;
}

bool StreamSocket::parseHeader(const std::string_view clientName, size_t headerSize, size_t bufferSize,
                               const Poco::Net::HTTPRequest& request,
                               std::chrono::duration<float, std::milli> delayMs,
                               MessageMap& map)
{
    assert(map._headerSize == 0 && map._messageSize == 0);

    map._headerSize = headerSize;
    map._messageSize = map._headerSize;

    const std::streamsize contentLength = request.getContentLength();

    LOG_INF("parseHeader: " << clientName << " HTTP Request: " << request << ", sz[header "
                            << map._headerSize << "], offset " << headerSize
                            << ", contentLength: " << contentLength);

    if (contentLength != Poco::Net::HTTPMessage::UNKNOWN_CONTENT_LENGTH)
    {
        // The only valid -ve value is -1 for "unknown content length."
        if (contentLength < 0 || contentLength > http::MaxBodyLen)
        {
            LOG_WRN("parseHeader: Invalid content length ("
                    << contentLength << "), limit: " << http::MaxBodyLen / 1024 / 1024 << " MB");
            throw std::out_of_range("Invalid content length: " + std::to_string(contentLength));
        }

        // Note: The bufferSize (i.e. the data received in the socket) may be
        // far less than the contentLength, and we may never get all the data.
        if (bufferSize < contentLength + headerSize)
        {
            LOG_DBG("parseHeader: Not enough content yet: ContentLength: "
                    << contentLength << " (+ headerSize: " << headerSize << " = "
                    << contentLength + headerSize << "), available: " << bufferSize
                    << " bytes (missing " << (contentLength + headerSize - bufferSize)
                    << " bytes), delay " << delayMs.count() << "ms");
            return false;
        }

        // messageSize includes both the header and the content sizes.
        map._messageSize += contentLength;
    }

    return true;
}

bool StreamSocket::compactChunks(MessageMap& map)
{
    if (!map._spans.size())
        return false; // single message.

    LOG_CHUNK(
        "Pre-compact " << map._spans.size() << " chunks: \n"
                       << HexUtil::dumpHex("", "", _inBuffer.begin(), _inBuffer.end(), false));

    char *first = _inBuffer.data();
    char *dest = first;
    for (const auto& [offset, length] : map._spans)
    {
        assert(length > 0);
        assert(offset < _inBuffer.size());
        assert(offset + length <= _inBuffer.size());
        std::memmove(dest, &_inBuffer[offset], length);
        dest += length;
    }

    // Erase the duplicate bits.
    size_t newEnd = dest - first;
    size_t gap = map._messageSize - newEnd;
    _inBuffer.erase(_inBuffer.begin() + newEnd, _inBuffer.begin() + map._messageSize);

    LOG_CHUNK("Post-compact with erase of "
              << newEnd << " to " << map._messageSize << " giving: \n"
              << HexUtil::dumpHex("", "", _inBuffer.begin(), _inBuffer.end(), false));

    // shrink our size to fit
    map._messageSize -= gap;

#if ENABLE_DEBUG
    LOG_TRC("Socket state: " <<
            [this](auto& oss)
            {
                oss.setf(std::ios_base::boolalpha);
                dumpState(oss);
            });
#endif

    return true;
}

bool StreamSocket::sniffSSL() const
{
    // Only sniffing the first bytes of a socket.
    if (bytesSent() > 0 || bytesRcvd() != _inBuffer.size() || bytesRcvd() < 6)
        return false;

    // 0x0000  16 03 01 02 00 01 00 01
    return (_inBuffer[0] == 0x16 && // HANDSHAKE
            _inBuffer[1] == 0x03 && // SSL 3.0 / TLS 1.x
            _inBuffer[5] == 0x01);  // Handshake: CLIENT_HELLO
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
