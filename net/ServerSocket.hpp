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
 * Non-blocking server socket for accepting incoming connections.
 * Classes: ServerSocket, SocketFactory
 */

#pragma once

#include <common/Log.hpp>
#include <net/NetUtil.hpp>
#include <net/Socket.hpp>

#ifndef _WIN32
#include <sys/un.h>
#endif

#include <memory>

class SocketFactory
{
public:
    virtual std::shared_ptr<Socket> create(int fd, Socket::Type type) = 0;
};

/// A non-blocking, streaming socket.
class ServerSocket : public Socket
{
public:
    ServerSocket(Socket::Type type,
                 std::chrono::steady_clock::time_point creationTime,
                 SocketPoll& clientPoller, std::shared_ptr<SocketFactory> sockFactory)
        : Socket(type, creationTime)
        , _sockFactory(std::move(sockFactory))
        , _clientPoller(clientPoller)
#if !MOBILEAPP
        , _type(type)
#endif
    {
    }

    /// Control access to a bound TCP socket
    STATE_ENUM(Type, Local, Public);

    /// Create a new server socket - accepted sockets will be added
    /// to the @clientSockets' poll when created with @factory.
    static std::shared_ptr<ServerSocket> create(ServerSocket::Type type,
                                                int port,
                                                Socket::Type socketType,
                                                std::chrono::steady_clock::time_point creationTime,
                                                SocketPoll& clientSocket,
                                                std::shared_ptr<SocketFactory> factory)
    {
        auto serverSocket = std::make_shared<ServerSocket>(socketType, creationTime, clientSocket, std::move(factory));

        if (serverSocket && serverSocket->bind(type, port) && serverSocket->listen())
            return serverSocket;

        return nullptr;
    }

    /// Binds to a local address (Servers only).
    /// Does not retry on error.
    /// Returns true only on success.
    virtual bool bind(Type type, int port);

    /// Listen to incoming connections (Servers only).
    /// Does not retry on error.
    /// Returns true on success only.
    bool listen(const int backlog = 64)
    {
#if !MOBILEAPP
        const int rc = ::listen(getFD(), backlog);
#else
        (void) backlog;
        const int rc = fakeSocketListen(getFD());
#endif
        if (rc)
            LOG_SYS("Failed to listen");
        else
            LOG_TRC("Listening");
        return rc == 0;
    }

    /// Accepts an incoming connection (Servers only).
    /// Does not retry on error.
    /// Returns a valid Socket shared_ptr on success only.
    virtual std::shared_ptr<Socket> accept();

    int getPollEvents(std::chrono::steady_clock::time_point /* now */,
                      int64_t & /* timeoutMaxMicroS */) override
    {
        return POLLIN;
    }

    void dumpState(std::ostream& os) override;

    void handlePoll(SocketDisposition &,
                    std::chrono::steady_clock::time_point /* now */,
                    int events) override
    {
        if (events & POLLIN)
        {
            std::shared_ptr<Socket> clientSocket = accept();
            if (clientSocket)
            {
                LOGA_TRC(Socket, "Accepted client #" << clientSocket->getFD() << ", " << *clientSocket);
                _clientPoller.insertNewSocket(std::move(clientSocket));
            }
        }
    }

protected:
    bool isUnrecoverableAcceptError(int cause) const;
    /// Create a Socket instance from the accepted socket FD.
    std::shared_ptr<Socket> createSocketFromAccept(int fd, Socket::Type type) const
    {
        return _sockFactory->create(fd, type);
    }

private:
    std::shared_ptr<SocketFactory> _sockFactory;
    SocketPoll& _clientPoller;
#if !MOBILEAPP
    Socket::Type _type;
#endif
};

#if !MOBILEAPP

/// Class to remember the socket name, and abstract the fact if it's an abstract unix socket, or real one (backed by a file).
class UnxSocketPath {

private:
    std::string _name;

public:
    UnxSocketPath() : _name() {}
    UnxSocketPath(const std::string& name) : _name(name) {}

    /// Make sure we construct the name correctly, based on whether we create the
    /// abstract socket, or a normal one.
    void fillInto(struct sockaddr_un& addrunix) const {
        assert(!_name.empty() && "Trying to use an invalid unx socket");

#ifdef HAVE_ABSTRACT_UNIX_SOCKETS
        std::memcpy(&addrunix.sun_path[1], _name.c_str(), _name.length());
        addrunix.sun_path[0] = '\0'; // this is an abstract name
#else
        std::memcpy(addrunix.sun_path, _name.c_str(), _name.length());
#endif
        LOG_ASSERT_MSG(addrunix.sun_path[sizeof(addrunix.sun_path) - 1] == '\0',
                       "addrunix.sun_path is not null terminated");
    }

    /// Is this a valid path?
    bool isValid() const {
        return !_name.empty();
    }

    /// Return the name - either for debugging printount, or for passing as --masterport=
    const std::string& getName() const {
        return _name;
    }
};

inline std::ostream& operator<<(std::ostream& os, const UnxSocketPath &s) {
    return os << s.getName();
}

/// A non-blocking, streaming Unix Domain Socket for local use
class LocalServerSocket : public ServerSocket
{
public:
    LocalServerSocket(std::chrono::steady_clock::time_point creationTime,
                      SocketPoll& clientPoller, std::shared_ptr<SocketFactory> sockFactory) :
        ServerSocket(Socket::Type::Unix, creationTime, clientPoller, std::move(sockFactory))
    {
    }
    ~LocalServerSocket() override;

    bool bind(Type, int) override { assert(false); return false; }
    std::shared_ptr<Socket> accept() override;
    UnxSocketPath bind();

    /// Links the socket to the location 'toPath', in case it is not an abstract socket. For abstract sockets, it return 'true' right away.
    bool linkTo(const std::string& toPath);

private:
    UnxSocketPath _id;
#ifndef HAVE_ABSTRACT_UNIX_SOCKETS
    std::string _linkName;
#endif
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
