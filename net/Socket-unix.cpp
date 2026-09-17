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

#include <algorithm>
#include <cerrno>
#include <cstring>

#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sysexits.h>

#include <common/Log.hpp>
#include <common/Syscall.hpp>
#include <common/Util.hpp>
#include <net/Socket.hpp>

namespace net
{

void shutdownDescriptor(int descriptor) { ::shutdown(descriptor, SHUT_RDWR); }

void closeDescriptor(int descriptor) { ::close(descriptor); }

ssize_t writeDescriptor(int descriptor, const void* buffer, std::size_t length)
{
    return ::write(descriptor, buffer, length);
}

void disableNagleAlgorithm(int descriptor)
{
    const int value = 1;
    if (::setsockopt(descriptor, IPPROTO_TCP, TCP_NODELAY, &value, sizeof(value)) == -1)
    {
        LOG_WRN_ONCE("Failed setsockopt TCP_NODELAY. Will not report further "
                     "failures to set TCP_NODELAY: "
                     << strerror(errno));
    }
}

ssize_t readDescriptor(int descriptor, void* buffer, std::size_t length)
{
    return ::read(descriptor, buffer, length);
}

int createPipe(int descriptors[2])
{
    return Syscall::pipe2(descriptors, O_CLOEXEC | O_NONBLOCK);
}

int pollDescriptors(struct pollfd* descriptors, std::size_t count, int64_t timeoutMicroseconds)
{
#if HAVE_PPOLL
    timeoutMicroseconds = std::max(timeoutMicroseconds, int64_t(0));
    struct timespec timeout;
    timeout.tv_sec = timeoutMicroseconds / (1000 * 1000);
    timeout.tv_nsec = (timeoutMicroseconds % (1000 * 1000)) * 1000;
    return ::ppoll(descriptors, count, &timeout, nullptr);
#else
    const int timeoutMilliseconds = (timeoutMicroseconds + 999) / 1000;
    return ::poll(descriptors, count, std::max(timeoutMilliseconds, 0));
#endif
}

int openStreamSocket(Socket::Type type)
{
    int domain = AF_UNSPEC;
    switch (type)
    {
    case Socket::Type::IPv4: domain = AF_INET;  break;
    case Socket::Type::IPv6: domain = AF_INET6; break;
    case Socket::Type::All:  domain = AF_INET6; break;
    case Socket::Type::Unix: domain = AF_UNIX;  break;
    default: assert(!"Unknown Socket::Type"); break;
    }

    return Syscall::socket_cloexec_nonblock(domain, SOCK_STREAM, 0);
}

bool bindToPort(int descriptor, Socket::Type socketType, bool publicly, int port)
{
    // Enable address reuse to avoid stalling after
    // recycling, when previous socket is TIME_WAIT.
    //TODO: Might be worth refactoring out.
    const int reuseAddress = 1;
    constexpr unsigned int len = sizeof(reuseAddress);
    if (::setsockopt(descriptor, SOL_SOCKET, SO_REUSEADDR, &reuseAddress, len) == -1)
        LOG_SYS("Failed setsockopt SO_REUSEADDR on socket fd " << descriptor << ": "
                                                               << strerror(errno));

    int rc;

    assert(socketType != Socket::Type::Unix);
    if (socketType == Socket::Type::IPv4)
    {
        struct sockaddr_in addrv4;
        std::memset(&addrv4, 0, sizeof(addrv4));
        addrv4.sin_family = AF_INET;
        addrv4.sin_port = htons(port);
        addrv4.sin_addr.s_addr = htonl(publicly ? INADDR_ANY : INADDR_LOOPBACK);

        rc = ::bind(descriptor, reinterpret_cast<const sockaddr*>(&addrv4), sizeof(addrv4));
    }
    else
    {
        struct sockaddr_in6 addrv6;
        std::memset(&addrv6, 0, sizeof(addrv6));
        addrv6.sin6_family = AF_INET6;
        addrv6.sin6_port = htons(port);
        addrv6.sin6_addr = publicly ? in6addr_any : in6addr_loopback;

        const int ipv6only = (socketType == Socket::Type::All ? 0 : 1);
        if (::setsockopt(descriptor, IPPROTO_IPV6, IPV6_V6ONLY, &ipv6only, sizeof(ipv6only)) == -1)
            LOG_SYS("Failed set ipv6 socket to " << ipv6only);

        rc = ::bind(descriptor, reinterpret_cast<const sockaddr*>(&addrv6), sizeof(addrv6));
    }

    if (rc)
        LOG_SYS("Failed to bind to: " << (socketType == Socket::Type::IPv4 ? "IPv4" : "IPv6")
                                      << " port: " << port);
    else
        LOG_TRC("Bind to: " << (socketType == Socket::Type::IPv4 ? "IPv4" : "IPv6")
                            << " port: " << port);

    return rc == 0;
}

bool isUnrecoverableAcceptError(const int cause)
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

int acceptConnection(int descriptor, PeerAddress& peer)
{
    struct sockaddr_in6 clientInfo;
    socklen_t addrlen = sizeof(clientInfo);
    const int rc = Syscall::accept_cloexec_nonblock(
        descriptor, reinterpret_cast<struct sockaddr*>(&clientInfo), &addrlen);
    if (rc < 0)
    {
        if (isUnrecoverableAcceptError(errno))
            Util::forcedExit(EX_SOFTWARE);
        return -1;
    }

    const void* inAddr;
    if (clientInfo.sin6_family == AF_INET)
    {
        const struct sockaddr_in* ipv4 = reinterpret_cast<const struct sockaddr_in*>(&clientInfo);
        inAddr = &(ipv4->sin_addr);
        peer.type = Socket::Type::IPv4;
    }
    else
    {
        const struct sockaddr_in6* ipv6 = &clientInfo;
        inAddr = &(ipv6->sin6_addr);
        peer.type = Socket::Type::IPv6;
    }

    // inet_ntop leaves the buffer alone when it fails, so start from an empty string.
    char address[INET6_ADDRSTRLEN] = {};
    ::inet_ntop(clientInfo.sin6_family, inAddr, address, sizeof(address));
    peer.address = address;
    peer.port = clientInfo.sin6_port;
    peer.family = clientInfo.sin6_family;

    return rc;
}

} // namespace net

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
