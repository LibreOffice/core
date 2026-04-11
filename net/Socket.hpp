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
 * Core socket abstractions: non-blocking I/O, polling, and protocol handling.
 * Classes: Socket, StreamSocket, SocketPoll, ProtocolHandlerInterface, SocketDisposition
 */

#pragma once

#include <common/Log.hpp>
#include <common/ProcUtil.hpp>
#include <common/SigUtil.hpp>
#include <common/StateEnum.hpp>
#include <common/Util.hpp>
#include <net/Buffer.hpp>
#include <net/FakeSocket.hpp>
#include <net/NetUtil.hpp>

#if !MOBILEAPP
#include <poll.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#endif

#include <atomic>
#include <cassert>
#include <cerrno>
#include <chrono>
#include <climits>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>

#ifdef __linux__
#define HAVE_ABSTRACT_UNIX_SOCKETS
#endif

// Enable to dump socket traffic as hex in logs.
// #define LOG_SOCKET_DATA ENABLE_DEBUG

#define ASSERT_CORRECT_SOCKET_THREAD(socket) socket->assertCorrectThread();

namespace http
{
class Request;
class Response;
}
namespace Poco
{
    class MemoryInputStream;
    namespace Net
    {
        class HTTPRequest;
        class HTTPResponse;
    }
    class URI;
}

class Socket;
std::ostream& operator<<(std::ostream& os, const Socket &s);

class Watchdog;
class SocketPoll;
class UnxSocketPath;

/// Helper to allow us to easily defer the movement of a socket
/// between polls to clarify thread ownership.
class SocketDisposition final
{
    STATE_ENUM(Type, CONTINUE, CLOSED, TRANSFER);

public:
    using MoveFunction = std::function<void(const std::shared_ptr<Socket>&)>;

    SocketDisposition(const std::shared_ptr<Socket> &socket)
        : _socket(socket)
        , _toPoll(nullptr)
        , _disposition(Type::CONTINUE)
    {}
    ~SocketDisposition()
    {
        assert (!_socketMove);
    }
    /** move, correctly change ownership of and insert into a new poll.
     * @transferFn is called as a callback inside the new poll, which
     * has its thread started if it is not already.
     * @moveFn can be used for optional setup.
     */
    void setTransfer(SocketPoll &poll, MoveFunction transferFn)
    {
        _socketMove = std::move(transferFn);
        _disposition = Type::TRANSFER;
        _toPoll = &poll; // rely on the closure to ensure lifetime
    }
    void setClosed()
    {
        _disposition = Type::CLOSED;
    }
    std::shared_ptr<Socket> getSocket() const
    {
        return _socket;
    }
    bool isClosed() const { return _disposition == Type::CLOSED; }
    bool isTransfer() const { return  _disposition == Type::TRANSFER; }
    bool isContinue() const { return _disposition == Type::CONTINUE; }

    /// Perform the queued up work.
    void execute();

private:
    MoveFunction _socketMove;
    std::shared_ptr<Socket> _socket;
    SocketPoll *_toPoll;
    Type _disposition;
};

class SocketThreadOwnerChange;

/// A non-blocking, streaming socket.
class Socket
{
public:
    static constexpr int DefaultSendBufferSize = 16 * 1024;
    static constexpr int MaximumSendBufferSize = 128 * 1024;

    enum class Type : uint8_t { IPv4, IPv6, All, Unix };
    static constexpr std::string_view toString(Type t);

    // NB. see other Socket::Socket by init below.
    Socket(Type type,
           std::chrono::steady_clock::time_point creationTime = std::chrono::steady_clock::now())
        : _creationTime(creationTime)
        , _lastSeenTime(_creationTime)
        , _bytesSent(0)
        , _bytesRcvd(0)
        , _clientPort(0)
        , _fd(createSocket(type))
        , _type(type)
        , _isShutdown(_fd < 0)
    {
        init();
    }

    virtual ~Socket()
    {
        LOG_TRC("Socket dtor");

        closeFD(); // In case we haven't closed yet.
    }

    /// Returns true iff we have a valid FD and haven't called shutdown(2).
    /// Note: this is needed because shutting down and closing are independent.
    bool isOpen() const { return _fd >= 0 && !_isShutdown; }

    /// Returns true if this socket FD has been shutdown, but not necessarily closed.
    bool isShutdown() const { return _isShutdown; }

    constexpr Type type() const { return _type; }
    constexpr bool isIPType() const { return Type::IPv4 == _type || Type::IPv6 == _type; }
    void setClientAddress(std::string address, unsigned int port = 0)
    {
        _clientAddress = std::move(address);
        _clientPort = port;
    }

    const std::string& clientAddress() const { return _clientAddress; }
    unsigned int clientPort() const { return _clientPort; }

    /// Returns the OS native socket fd.
    constexpr int getFD() const { return _fd; }

    std::ostream& streamStats(std::ostream& os, std::chrono::steady_clock::time_point now) const;
    std::string getStatsString(std::chrono::steady_clock::time_point now) const;

    virtual std::ostream& stream(std::ostream& os) const  { return streamImpl(os); }

    /// Returns monotonic creation timestamp
    constexpr std::chrono::steady_clock::time_point getCreationTime() const { return _creationTime; }
    /// Returns monotonic timestamp of last received signal from remote
    constexpr std::chrono::steady_clock::time_point getLastSeenTime() const { return _lastSeenTime; }

    /// Sets monotonic timestamp of last received signal from remote
    void setLastSeenTime(const std::chrono::steady_clock::time_point now) { _lastSeenTime = now; }

    /// Returns bytes sent statistic
    constexpr uint64_t bytesSent() const { return _bytesSent; }
    /// Returns bytes received statistic
    constexpr uint64_t bytesRcvd() const { return _bytesRcvd; }

    /// Get input/output statistics on this stream
    void getIOStats(uint64_t& sent, uint64_t& recv) const
    {
        sent = _bytesSent;
        recv = _bytesRcvd;
    }

    /// Shutdown the socket.
    /// TODO: Support separate read/write shutdown.
    void syncShutdown()
    {
        if (!_noShutdown)
        {
            LOG_TRC("Socket shutdown RDWR. " << *this);
            if (!_isShutdown)
            {
                setShutdown();
#if !MOBILEAPP
                ::shutdown(_fd, SHUT_RDWR);
#else
                fakeSocketShutdown(_fd);
#endif
            }
        }
    }

    /// Prepare our poll record; adjust @timeoutMaxMs downwards
    /// for timeouts, based on current time @now.
    /// @returns POLLIN and POLLOUT if output is expected.
    virtual int getPollEvents(std::chrono::steady_clock::time_point now,
                              int64_t &timeoutMaxMicroS) = 0;

    /// Handle results of events returned from poll
    virtual void handlePoll(SocketDisposition &disposition,
                            std::chrono::steady_clock::time_point now,
                            int events) = 0;

    /// Do we have internally queued incoming / outgoing data ?
    virtual bool hasBuffered() const { return false; }

    /// Returns the total capacity of all data buffers.
    virtual std::size_t totalBufferCapacity() const { return 0; }

#if !MOBILEAPP
    /// manage latency issues around packet aggregation
    void setNoDelay()
    {
        if constexpr (!Util::isMobileApp())
        {
            const int val = 1;
            if (::setsockopt(_fd, IPPROTO_TCP, TCP_NODELAY, (char*)&val, sizeof(val)) == -1)
            {
                LOG_WRN_ONCE("Failed setsockopt TCP_NODELAY. Will not report further "
                             "failures to set TCP_NODELAY: "
                             << strerror(errno));
            }
        }
    }

    /// Uses peercreds to get prisoner PID if present or -1
    int getPid() const;

#endif // !MOBILEAPP

    /// Sets the kernel socket send buffer in size bytes.
    /// Note: TCP will allocate twice this size for admin purposes,
    /// so a subsequent call to getSendBufferSize will return
    /// the larger (actual) buffer size, if this succeeds.
    /// Note: the upper limit is set via /proc/sys/net/core/wmem_max,
    /// and there is an unconfigurable lower limit as well.
    /// Returns true on success only.
    bool setSocketBufferSize([[maybe_unused]] const int size)
    {
#if !MOBILEAPP
        int rc = ::setsockopt(_fd, SOL_SOCKET, SO_SNDBUF, &size, sizeof(size));

        _sendBufferSize = getSocketBufferSize();
        if (rc != 0 || _sendBufferSize < 0)
        {
            _sendBufferSize = DefaultSendBufferSize;
            LOG_SYS("Error getting socket buffer size. Using default size of "
                    << _sendBufferSize << " bytes.");
            return false;
        }

        if (_sendBufferSize > MaximumSendBufferSize * 2)
        {
            LOG_TRC("Clamped send buffer size to " << MaximumSendBufferSize << " from "
                                                   << _sendBufferSize);
            _sendBufferSize = MaximumSendBufferSize;
        }
        else
            LOG_TRC("Set socket buffer size to " << _sendBufferSize);

        return true;
#else
        return false;
#endif
    }

    /// Gets the actual send buffer size in bytes, -1 for failure.
    [[nodiscard]] int getSocketBufferSize() const
    {
#if !MOBILEAPP
        int size;
        socklen_t len = sizeof(size);
        const int rc = ::getsockopt(_fd, SOL_SOCKET, SO_SNDBUF, &size, &len);
        return rc == 0 ? size : -1;
#else
        return -1;
#endif
    }

    /// Gets our fast cache of the socket buffer size
    int getSendBufferSize() const { return (Util::isMobileApp() ? INT_MAX : _sendBufferSize); }

#if !MOBILEAPP
    /// Sets the receive buffer size in bytes.
    /// Note: TCP will allocate twice this size for admin purposes,
    /// so a subsequent call to getReceieveBufferSize will return
    /// the larger (actual) buffer size, if this succeeds.
    /// Note: the upper limit is set via /proc/sys/net/core/rmem_max,
    /// and there is an unconfigurable lower limit as well.
    /// Returns true on success only.
    bool setReceiveBufferSize(const int size)
    {
        constexpr unsigned int len = sizeof(size);
        const int rc = ::setsockopt(_fd, SOL_SOCKET, SO_RCVBUF, &size, len);
        return rc == 0;
    }

    /// Gets the actual receive buffer size in bytes, -1 on error.
    int getReceiveBufferSize() const
    {
        int size;
        unsigned int len = sizeof(size);
        const int rc = ::getsockopt(_fd, SOL_SOCKET, SO_RCVBUF, &size, &len);
        return rc == 0 ? size : -1;
    }

    /// Gets the error code.
    /// Sets errno on success and returns it.
    /// Returns -1 on failure to get the error code.
    int getError() const
    {
        int error;
        unsigned int len = sizeof(error);
        const int rc = ::getsockopt(_fd, SOL_SOCKET, SO_ERROR, &error, &len);
        if (rc == 0)
        {
            // Set errno so client can use strerror etc.
            errno = error;
            return error;
        }

        return rc;
    }
#endif

    // Does this socket come from the localhost ?
    bool isLocal() const;

    virtual void dumpState(std::ostream&) {}

    /// Returns the owner thread's id.
    ProcUtil::ThreadId getThreadOwner() const { return _owner; }

    /// Asserts in the debug builds, otherwise just logs.
    void assertCorrectThread(LOG_CAPTURE_CALLER_DECLARATION) const
    {
        if (!ThreadChecks::Inhibit)
            ProcUtil::assertCorrectThread(_owner, LOG_PASS_PARENT_CALLER);
    }

    bool ignoringInput() const { return _ignoreInput; }

    // Ensure that no further input is processed from this socket
    virtual void ignoreInput()
    {
        LOG_TRC("Ignore further input on socket");
        _ignoreInput = true;
    }

    // arg to emphasize what is allowed do this
    // close in advance of the ctor
    void closeFD(const SocketPoll& /*rPoll*/) { closeFD(); }

protected:
    /// Construct based on an existing socket fd.
    /// Used by accept() only.
    Socket(const int fd, Type type,
           std::chrono::steady_clock::time_point creationTime = std::chrono::steady_clock::now())
        : _creationTime(creationTime)
        , _lastSeenTime(_creationTime)
        , _bytesSent(0)
        , _bytesRcvd(0)
        , _clientPort(0)
        , _fd(fd)
        , _type(type)
        , _isShutdown(_fd < 0)
    {
        init();
    }

    void logPrefix(std::ostream& os) const { os << '#' << _fd << ": "; }

    /// Adds `len` sent bytes to statistic
    void notifyBytesSent(uint64_t len) { _bytesSent += len; }
    /// Adds `len` received bytes to statistic
    void notifyBytesRcvd(uint64_t len) { _bytesRcvd += len; }

    /// avoid doing a shutdown before close
    void setNoShutdown() { _noShutdown = true; }

    /// Explicitly marks this socket FD as shut down, but not necessarily closed.
    void setShutdown() { _isShutdown = true; }

    /// Set the thread-id we're bound to
    virtual void setThreadOwner(const ProcUtil::ThreadId id, LOG_CAPTURE_CALLER_DECLARATION)
    {
        if (id != _owner)
        {
            LOG_TRC("Thread affinity of Socket #" << _fd << " set to " << id << " (was " << _owner << ')');
            _owner = id;
        }
    }

    /// Reset the thread-id while it's in transition.
    virtual void resetThreadOwner(LOG_CAPTURE_CALLER_DECLARATION)
    {
        if (ProcUtil::ThreadId() != _owner)
        {
            LOG_TRC("Resetting thread affinity of Socket #" << _fd << " while in transit (was " << _owner << ')');
            _owner = ProcUtil::ThreadId();
        }
    }

private:
    friend class SocketThreadOwnerChange;

    /// Create socket of the given type.
    /// return >= 0 for a successfully created socket, -1 on error
    static int createSocket(Type type);

    std::ostream& streamImpl(std::ostream& os) const;
    std::string toStringImpl() const;

    void init()
    {
#if !MOBILEAPP
        if (_type != Type::Unix && _fd >= 0)
            setNoDelay();
#endif
        _ignoreInput = false;
        _noShutdown = false;
        _sendBufferSize = DefaultSendBufferSize;
        _owner = ProcUtil::getThreadId();
        LOG_DBG("Created socket #" << _fd << ". Thread affinity set to " << _owner);

        if constexpr (!Util::isMobileApp())
        {
#if ENABLE_DEBUG
            if (std::getenv("COOL_ZERO_BUFFER_SIZE") && _fd >= 0)
            {
                const int oldSize = getSocketBufferSize();
                setSocketBufferSize(0);
                LOG_TRC("Buffer size: " << getSendBufferSize() << " (was " << oldSize << ')');
            }
#endif
        }
    }

    /// Close the socket FD.
    /// Internal implementation, always private.
    void closeFD()
    {
        // explicit closeFD called, or initial createSocket failure
        if (_fd < 0)
            return;

            // Doesn't block on sockets; no error handling needed.
#if !MOBILEAPP
        ::close(_fd);
#else
        fakeSocketClose(_fd);
#endif

        LOG_DBG("Closed socket " << toStringImpl()); // Should be logged exactly once.

        // Invalidate the FD by negating to preserve the original value.
        if (_fd > 0)
            _fd = -_fd;
        else if (_fd == 0) // Unlikely, but technically possible.
            _fd = -1;
    }

    std::string _clientAddress;

    const std::chrono::steady_clock::time_point _creationTime;
    std::chrono::steady_clock::time_point _lastSeenTime;
    uint64_t _bytesSent;
    uint64_t _bytesRcvd;

    /// We check the owner even in the release builds, needs to be always correct.
    ProcUtil::ThreadId _owner;

    unsigned int _clientPort;
    int _fd;
    int _sendBufferSize;

    const Type _type;

    /// True if this socket is shut down.
    bool _isShutdown;
    // If _ignoreInput is true no more input from this socket will be processed.
    bool _ignoreInput;
    bool _noShutdown;
};

// Allow SocketPoll and SocketDisposition to call Socket::setThreadOwner
// without exposing the entirety of Socket's internals to them
class SocketThreadOwnerChange
{
private:
    friend class SocketDisposition;
    friend class SocketPoll;

    static void setThreadOwner(Socket& socket, const ProcUtil::ThreadId id,
                               LOG_CAPTURE_CALLER_DECLARATION)
    {
        socket.setThreadOwner(id, LOG_PASS_PARENT_CALLER);
    }

    static void resetThreadOwner(Socket& socket, LOG_CAPTURE_CALLER_DECLARATION)
    {
        socket.resetThreadOwner(LOG_PASS_PARENT_CALLER);
    }
};

inline std::ostream& operator<<(std::ostream& os, const Socket &s) { return s.stream(os); }

class StreamSocket;
class MessageHandlerInterface;

/// Interface that decodes the actual incoming message.
class ProtocolHandlerInterface :
    public std::enable_shared_from_this<ProtocolHandlerInterface>
{
protected:
    /// We own a message handler, after decoding the socket data we pass it on as messages.
    std::shared_ptr<MessageHandlerInterface> _msgHandler;

    /// Sets the context used by logPrefix.
    void setLogContext(int fd) { _fdSocket = fd; }

    /// Returns the log prefix string for use outside of member context.
    std::string getLogPrefix() const { return '#' + std::to_string(_fdSocket) + ": "; }

    /// Used by the logging macros to automatically log a context prefix.
    void logPrefix(std::ostream& os) const { os << getLogPrefix(); }

public:
    ProtocolHandlerInterface()
        : _fdSocket(-1)
        , _owner(ProcUtil::getThreadId())
    {
    }

    // ------------------------------------------------------------------
    // Interface for implementing low level socket goodness from streams.
    // ------------------------------------------------------------------
    virtual ~ProtocolHandlerInterface() = default;

    /// Asserts in the debug builds, otherwise just logs.
    void assertCorrectThread(LOG_CAPTURE_CALLER_DECLARATION) const
    {
        if (!ThreadChecks::Inhibit)
            ProcUtil::assertCorrectThread(_owner, LOG_PASS_PARENT_CALLER);
    }

    /// Called when the socket is newly created to
    /// set the socket associated with this ResponseClient.
    /// Will be called exactly once.
    virtual void onConnect(const std::shared_ptr<StreamSocket>& socket) = 0;

    /// Enable/disable processing of incoming data at socket level.
    virtual void enableProcessInput(bool /*enable*/){};
    virtual bool processInputEnabled() const { return true; };

    /// Called after successful socket reads.
    virtual void handleIncomingMessage(SocketDisposition &disposition) = 0;

    /// Prepare our poll record; adjust @timeoutMaxMs downwards
    /// for timeouts, based on current time @now.
    /// @returns POLLIN and POLLOUT if output is expected.
    virtual int getPollEvents(std::chrono::steady_clock::time_point now,
                              int64_t &timeoutMaxMicroS) = 0;

    /// Checks whether a timeout has occurred. Method will shutdown connection and socket on timeout.
    /// Returns true in case of a timeout, caller shall stop processing
    virtual bool checkTimeout(std::chrono::steady_clock::time_point /* now */) { return false; }

    /// Do some of the queued writing.
    virtual void performWrites(std::size_t capacity) = 0;

    /// Called when the SSL Handshake fails.
    virtual void onHandshakeFail() {}

    /// Called when the socket is disconnected and will be destroyed.
    /// Will be called exactly once.
    virtual void onDisconnect() = 0;

    // -----------------------------------------------------------------
    //            Interface for external MessageHandlers
    // -----------------------------------------------------------------

    void setMessageHandler(const std::shared_ptr<MessageHandlerInterface> &msgHandler)
    {
        ASSERT_CORRECT_THREAD();
        _msgHandler = msgHandler;
    }

    /// Clear all external references
    virtual void dispose()
    {
        ASSERT_CORRECT_THREAD();
        _msgHandler.reset();
    }

    /// Sends a text message.
    /// Returns the number of bytes written (including frame overhead) on success,
    /// 0 for closed/invalid socket, and -1 for other errors.
    virtual int sendTextMessage(std::string_view msg, bool flush = false) const = 0;

    /// Sends a binary message.
    /// Returns the number of bytes written (including frame overhead) on success,
    /// 0 for closed/invalid socket, and -1 for other errors.
    virtual int sendBinaryMessage(std::string_view data, bool flush = false) const = 0;

    /// Shutdown the socket and specify if the endpoint is going away or not (useful for WS).
    /// Optionally provide a message sent in the close frame (useful for WS).
    virtual void shutdown(bool goingAway = false,
                          const std::string_view statusMessage = std::string_view()) = 0;

    virtual void getIOStats(uint64_t &sent, uint64_t &recv) = 0;

    void dumpState(std::ostream& os) const { dumpState(os, "\n\t"); }

    /// Append pretty printed internal state to a line
    virtual void dumpState(std::ostream& os, const std::string& indent) const
    {
        os << indent;
    }

private:
    friend class ProtocolThreadOwnerChange;

    void setThreadOwner(const ProcUtil::ThreadId id, LOG_CAPTURE_CALLER_DECLARATION)
    {
        if (id != _owner)
        {
            LOG_TRC("Thread affinity of ProtocolHandlerInterface set to " << id << " (was "
                                                                          << _owner << ')');
            _owner = id;
        }
    }

    void resetThreadOwner(LOG_CAPTURE_CALLER_DECLARATION)
    {
        if (ProcUtil::ThreadId() != _owner)
        {
            LOG_TRC("Resetting thread affinity of ProtocolHandlerInterface while in transit (was "
                    << _owner << ')');
            _owner = ProcUtil::ThreadId();
        }
    }

    int _fdSocket; ///< The socket file-descriptor.
    ProcUtil::ThreadId _owner;
};

class StreamSocket;

// Allow Socket to call ProtocolHandlerInterface::setThreadOwner
// without exposing the entirety of ProtocolHandlerInterface's internals to it
class ProtocolThreadOwnerChange
{
    friend class StreamSocket;

    static void setThreadOwner(ProtocolHandlerInterface& handler, const ProcUtil::ThreadId id,
                               LOG_CAPTURE_CALLER_DECLARATION)
    {
        handler.setThreadOwner(id, LOG_PASS_PARENT_CALLER);
    }

    static void resetThreadOwner(ProtocolHandlerInterface& handler, LOG_CAPTURE_CALLER_DECLARATION)
    {
        handler.resetThreadOwner(LOG_PASS_PARENT_CALLER);
    }

};

// Forward declare WebSocketHandler, which is inherited from ProtocolHandlerInterface.
class WebSocketHandler;

/// A ProtocolHandlerInterface with dummy sending API.
class SimpleSocketHandler : public ProtocolHandlerInterface
{
public:
    SimpleSocketHandler() = default;
    int sendTextMessage(std::string_view, bool) const override { return 0; }
    int sendBinaryMessage(std::string_view, bool) const override { return 0; }
    void shutdown(bool, const std::string_view) override {}
    void getIOStats(uint64_t &, uint64_t &) override {}
};

/// Interface that receives and sends incoming messages.
class MessageHandlerInterface :
    public std::enable_shared_from_this<MessageHandlerInterface>
{
protected:
    std::shared_ptr<ProtocolHandlerInterface> _protocol;
    explicit MessageHandlerInterface(std::shared_ptr<ProtocolHandlerInterface> protocol)
        : _protocol(std::move(protocol))
    {
    }

    virtual ~MessageHandlerInterface() = default;

public:
    /// Setup, after construction for shared_from_this
    void initialize()
    {
        if (_protocol)
            _protocol->setMessageHandler(shared_from_this());
    }

    /// Clear all external references
    virtual void dispose()
    {
        if (_protocol)
        {
            _protocol->dispose();
            _protocol.reset();
        }
    }

    std::shared_ptr<ProtocolHandlerInterface> getProtocol() const
    {
        return _protocol;
    }

    /// Do we have something to send ?
    virtual bool hasQueuedMessages() const = 0;
    /// Please send them to me then.
    /// Write queued messages into the socket, at least capacity bytes,
    /// and up to the next message boundary.
    virtual void writeQueuedMessages(std::size_t capacity) = 0;
    /// We just got a message - here it is
    virtual void handleMessage(const std::vector<char> &data) = 0;
    /// Get notified that the underlying transports disconnected
    virtual void onDisconnect() = 0;
    /// Append pretty printed internal state to a line
    virtual void dumpState(std::ostream& os) = 0;
};

class InputProcessingManager
{
public:
    InputProcessingManager(const std::shared_ptr<ProtocolHandlerInterface> &protocol, bool inputProcess)
        : _protocol(protocol)
        , _prevInputProcess(false)
    {
        if (_protocol)
        {
            // Save previous state to be restored in destructor
            _prevInputProcess = _protocol->processInputEnabled();
            protocol->enableProcessInput(inputProcess);
        }
    }

    ~InputProcessingManager()
    {
        // Restore previous state
        if (_protocol)
            _protocol->enableProcessInput(_prevInputProcess);
    }

private:
    std::shared_ptr<ProtocolHandlerInterface> _protocol;
    bool _prevInputProcess;
};

/// Handles non-blocking socket event polling.
/// Only polls on N-Sockets and invokes callback and
/// doesn't manage buffers or client data.
/// Note: uses poll(2) since it has very good performance
/// compared to epoll up to a few hundred sockets and
/// doesn't suffer select(2)'s poor API. Since this will
/// be used per-document we don't expect to have several
/// hundred users on same document to suffer poll(2)'s
/// scalability limit. Meanwhile, epoll(2)'s high
/// overhead to adding/removing sockets is not helpful.
class SocketPoll
{
public:
    /// Create a socket poll, called rather infrequently.
    explicit SocketPoll(std::string threadName);
    virtual ~SocketPoll();

    static std::unique_ptr<Watchdog> PollWatchdog;

    /// Default poll time - useful to increase for debugging.
    static constexpr std::chrono::microseconds DefaultPollTimeoutMicroS = std::chrono::seconds(64);

    /// Stop the polling thread.
    void stop()
    {
        LOG_DBG("Stopping " << logInfo());
        _stop = true;
        if constexpr (!Util::isMobileApp())
        {
            // We don't want to risk some callbacks in _newCallbacks being invoked when we start
            // running a thread for this SocketPoll again.
            std::lock_guard<std::mutex> lock(_mutex);
            if (_newCallbacks.size() > 0)
            {
                LOG_WRN("_newCallbacks is non-empty when stopping, clearing it.");
                _newCallbacks.clear();
            }
        }
        wakeup();
    }

    /// Remove all the sockets we own.
    void removeSockets();

    /// After a fork - close all associated sockets without shutdown.
    void closeAllSockets();

    /// Setup pipes needed for cross-thread wakeups
    void createWakeups();

    bool isAlive() const { return (_threadStarted && !_threadFinished) || _runOnClientThread; }

    /// Check if we should continue polling
    virtual bool continuePolling()
    {
        return !_stop;
    }

    /// Executed inside the poll in case of a wakeup
    virtual void wakeupHook() {}

    ProcUtil::ThreadId getThreadOwner() const { return _owner; }

    /// Are we running in either shutdown, or the polling thread.
    /// Asserts in the debug builds, otherwise just logs.
    void assertCorrectThread(LOG_CAPTURE_CALLER_DECLARATION) const
    {
        if (!ThreadChecks::Inhibit && isAlive())
            ProcUtil::assertCorrectThread(_owner, LOG_PASS_PARENT_CALLER);
    }

    /// Kit poll can be called from COKit's Yield in any thread, adapt to that.
    void checkAndReThread();

    /// Poll the sockets for available data to read or buffer to write.
    /// Returns the return-value of poll(2): 0 on timeout,
    /// -1 for error, and otherwise the number of events signalled.
    int poll(std::chrono::microseconds timeoutMax, bool justPoll = false) { return poll(timeoutMax.count(), justPoll); }

    /// Poll the sockets for available data to read or buffer to write.
    /// Returns the return-value of poll(2): 0 on timeout,
    /// -1 for error, and otherwise the number of events signalled.
    /// Takes the deadline, instead of a timeout.
    /// Note: will *not* return until the deadline expires, or upon
    /// hitting an error. Always polls at least once. Returns last rc.
    int pollUntilDeadline(std::chrono::steady_clock::time_point deadline)
    {
        constexpr auto zero = std::chrono::microseconds::zero();
        auto timeoutMax = std::max(std::chrono::duration_cast<std::chrono::microseconds>(
                                       deadline - std::chrono::steady_clock::now()),
                                   zero);
        int rc = 0;
        do
        {
            // Always poll at least once.
            rc = poll(timeoutMax);
            if (timeoutMax == zero || rc < 0)
            {
                return rc; // Return on error or if we're out of time.
            }

            timeoutMax = std::max(std::chrono::duration_cast<std::chrono::microseconds>(
                                      deadline - std::chrono::steady_clock::now()),
                                  zero);

        } while (timeoutMax > zero && !_stop && !SigUtil::getShutdownRequestFlag());

        return rc;
    }

    /// Write to a wakeup descriptor
    static void wakeup (int fd)
    {
        // wakeup the main-loop.
        int rc;
        do {
#if !MOBILEAPP
            rc = ::write(fd, "w", 1);
#else
            rc = fakeSocketWrite(fd, "w", 1);
#endif
        } while (rc == -1 && errno == EINTR);

        if (rc == -1 && errno != EAGAIN && errno != EWOULDBLOCK)
            LOG_SYS("wakeup socket #" << fd << " is closed at wakeup?");
    }

    /// Wakeup the main polling loop in another thread
    void wakeup() const
    {
        // There is a race when shutting down because
        // SocketPoll threads exit when shutting down.
        if (!isAlive() && !SigUtil::getShutdownRequestFlag())
            LOG_DBG("WARNING: Waking up dead poll thread ["
                    << _name << "], started: " << (_threadStarted ? "true" : "false")
                    << ", finished: " << _threadFinished);

        wakeup(_wakeup[1]);
    }

    /// Global wakeup - signal safe: wakeup all socket polls.
    static void wakeupWorld();

    /// Insert a new socket to be polled.
    /// A socket is removed when it is closed, readIncomingData
    /// returns false, or when removeSockets is called (which is
    /// done automatically when SocketPoll is stopped via joinThread).
    void insertNewSocket(std::shared_ptr<Socket> newSocket)
    {
        if (newSocket)
        {
            LOG_TRC("Inserting socket #" << newSocket->getFD() << ", address ["
                                         << newSocket->clientAddress() << "], into " << _name);
            // sockets in transit are un-owned.
            SocketThreadOwnerChange::resetThreadOwner(*newSocket);

            std::lock_guard<std::mutex> lock(_mutex);
            const bool wasEmpty = taskQueuesEmpty();
            _newSockets.emplace_back(std::move(newSocket));
            if (wasEmpty)
                wakeup();
        }
    }

    /// Schedules an async transfer of a socket from this SocketPoll to
    /// @toPoll.
    ///
    /// @cbAfterArrivalInNewPoll is called when socket is inserted in @toPoll.
    /// See insertNewSocket
    ///
    /// @cbAfterRemovalFromOldPoll is called when socket has been removed
    /// from this SocketPoll. May be nullptr.
    void transferSocketTo(const std::weak_ptr<Socket>& socket,
                          const std::weak_ptr<SocketPoll>& toPoll,
                          SocketDisposition::MoveFunction cbAfterArrivalInNewPoll,
                          std::function<void()> cbAfterRemovalFromOldPoll)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        const bool wasEmpty = taskQueuesEmpty();
        _pendingTransfers.emplace_back(socket, toPoll,
                                       std::move(cbAfterArrivalInNewPoll),
                                       std::move(cbAfterRemovalFromOldPoll));
        if (wasEmpty)
            wakeup();
    }

    /// Takes socket from @fromPoll and moves it to @toPoll.
    /// Blocks until the socket is removed from @fromPoll.
    static void takeSocket(const std::shared_ptr<SocketPoll>& fromPoll,
                           const std::shared_ptr<SocketPoll>& toPoll,
                           const std::shared_ptr<Socket>& socket);

#if !MOBILEAPP
    /// Inserts a new remote websocket to be polled.
    /// NOTE: The DNS lookup is synchronous.
    void insertNewWebSocketSync(const Poco::URI& uri,
                                const std::shared_ptr<WebSocketHandler>& websocketHandler);

    bool insertNewUnixSocket(
        const UnxSocketPath &location,
        const std::string &pathAndQuery,
        const std::shared_ptr<WebSocketHandler>& websocketHandler,
        const std::vector<int>* shareFDs = nullptr);
#else
    bool insertNewFakeSocket(
        int peerSocket,
        const std::shared_ptr<ProtocolHandlerInterface>& websocketHandler);
#endif

    using CallbackFn = std::function<void()>;

    /// Add a callback to be invoked in the polling thread
    void addCallback(CallbackFn fn)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        const bool wasEmpty = taskQueuesEmpty();
        _newCallbacks.emplace_back(std::move(fn));
        if (wasEmpty)
            wakeup();
    }

    virtual void dumpState(std::ostream& os) const;

    size_t getSocketCount() const
    {
        ASSERT_CORRECT_THREAD();
        return _pollSockets.size();
    }

    const std::string& name() const { return _name; }

    /// Start the polling thread (if desired)
    /// Mutually exclusive with runOnClientThread().
    bool startThread();

    /// Stop and join the polling thread before returning (if active)
    void joinThread();

    /// Called to prevent starting own poll thread
    /// when polling is done on the client's thread.
    /// Mutually exclusive with startThread().
    bool runOnClientThread()
    {
        assert(!_threadStarted);

        if (!_threadStarted)
        {
            // TODO: should avoid wakeup resource creation too.
            _runOnClientThread = true;
            return true;
        }

        return false;
    }

    bool isRunOnClientThread() const
    {
        return _runOnClientThread;
    }

    void disableWatchdog();
    void enableWatchdog();

protected:
    bool isStop() const
    {
        return _stop;
    }

    void removeFromWakeupArray();

    bool hasCallbacks()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return _newCallbacks.size() > 0;
    }

    bool hasBuffered() const
    {
        for (const auto& it : _pollSockets)
            if (it->hasBuffered())
                return true;
        return false;
    }

private:
    /// The default implementation of our polling thread
    virtual void pollingThread()
    {
        while (continuePolling())
        {
            poll(DefaultPollTimeoutMicroS);
        }
    }

    /// Actual poll implementation
    int poll(int64_t timeoutMaxMicroS, bool justPoll = false);

    /// Initialize the poll fds array with the right events
    void setupPollFds(std::chrono::steady_clock::time_point now,
                      int64_t &timeoutMaxMicroS)
    {
        const size_t size = _pollSockets.size();

        _pollFds.resize(size + 1); // + wakeup pipe

        for (size_t i = 0; i < size; ++i)
        {
            int events = _pollSockets[i]->getPollEvents(now, timeoutMaxMicroS);
            assert(events >= 0 && "The events bitmask must be non-negative, where 0 means skip all events.");

            if (_pollSockets[i]->ignoringInput())
                events &= ~POLLIN; // mask out input.

            _pollFds[i].fd = _pollSockets[i]->getFD();
            _pollFds[i].events = events;
            _pollFds[i].revents = 0;
            LOGA_TRC(Socket, '#' << _pollFds[i].fd << ": setupPollFds getPollEvents: 0x" << std::hex
                     << events << std::dec);
        }

        // Add the read-end of the wake pipe.
        _pollFds[size].fd = _wakeup[0];
        _pollFds[size].events = POLLIN;
        _pollFds[size].revents = 0;
    }

    [[nodiscard]] std::string logInfo() const
    {
        std::ostringstream os;
        os << "SocketPoll[this " << std::hex << this << std::dec << ", thread[name " << _name
           << ", id[owner " << _owner << ", caller " << ProcUtil::getThreadId() << "]]]";
        return os.str();
    }

    /// The polling thread entry.
    /// Used to set the thread name and mark the thread as stopped when done.
    void pollingThreadEntry();

    bool taskQueuesEmpty() const
    {
        return _newSockets.empty() && _newCallbacks.empty() && _pendingTransfers.empty();
    }

    struct SocketTransfer
    {
        std::weak_ptr<Socket> _socket;
        std::weak_ptr<SocketPoll> _toPoll;
        SocketDisposition::MoveFunction _cbAfterArrivalInNewPoll;
        std::function<void()> _cbAfterRemovalFromOldPoll;

        SocketTransfer(std::weak_ptr<Socket> socket,
                       std::weak_ptr<SocketPoll> toPoll,
                       SocketDisposition::MoveFunction cbAfterArrivalInNewPoll,
                       std::function<void()> cbAfterRemovalFromOldPoll)
            : _socket(std::move(socket))
            , _toPoll(std::move(toPoll))
            , _cbAfterArrivalInNewPoll(std::move(cbAfterArrivalInNewPoll))
            , _cbAfterRemovalFromOldPoll(std::move(cbAfterRemovalFromOldPoll))
       {
       }
    };

    void transfer(const SocketTransfer& pendingTransfer);

    /// Protects _newSockets, _newCallbacks and _pendingTransfers
    std::mutex _mutex;

    /// Debug name used for logging.
    const std::string _name;

    /// The sockets we're controlling
    std::vector<std::shared_ptr<Socket>> _pollSockets;
    std::vector<std::shared_ptr<Socket>> _newSockets;
    std::vector<CallbackFn> _newCallbacks;
    std::vector<SocketTransfer> _pendingTransfers;

    /// The fds to poll.
    std::vector<pollfd> _pollFds;

    /// main-loop wakeup pipe
    int _wakeup[2];
    /// We start handling the poll results of the above sockets at a different index each time, to
    /// not arbitrarily prioritize some
    size_t _pollStartIndex;
    /// The polling thread.
    std::thread _thread;
    ProcUtil::ThreadId _owner;
    /// Flag the thread to stop.
    std::atomic<int64_t> _threadStarted;
#if !MOBILEAPP
    std::atomic<uint64_t> _watchdogTime;
#endif

    std::atomic<bool> _stop;
    std::atomic<bool> _threadFinished;
    std::atomic<bool> _runOnClientThread;
};

/// A SocketPoll that will stop polling and
/// terminate when the TerminationFlag is set.
class TerminatingPoll : public SocketPoll
{
public:
    TerminatingPoll(const std::string &threadName) :
        SocketPoll(threadName) {}

    bool continuePolling() override
    {
        return SocketPoll::continuePolling() && !SigUtil::getTerminationFlag();
    }
};

STATE_ENUM(SharedFDType, SMAPS, URPToKit, URPFromKit);

enum HostType : uint8_t { LocalHost, Other };

// A plain, non-blocking, data streaming socket.
class StreamSocket : public Socket,
                     public std::enable_shared_from_this<StreamSocket>
{
public:
    STATE_ENUM(ReadType, NormalRead, UseRecvmsgExpectFD);

    /// Create a StreamSocket from native FD.
    StreamSocket(std::string host, const int fd, Type type, bool isClient,
                 HostType hostType, ReadType readType = ReadType::NormalRead,
                 std::chrono::steady_clock::time_point creationTime = std::chrono::steady_clock::now() )
        : Socket(fd, type, creationTime)
        , _hostname(std::move(host))
        , _wsState(WSState::HTTP)
        , _readType(readType)
        , _shutdownSignalled(false)
        , _inputProcessingEnabled(true)
        , _doneDisconnect(false)
        , _isClient(isClient)
        , _isLocalHost(hostType == LocalHost)
        , _sentHTTPContinue(false)
    {
        LOG_TRC("StreamSocket ctor");
        if (isExternalCountedConnection())
            ++ExternalConnectionCount;
    }

    ~StreamSocket() override
    {
        LOG_TRC("StreamSocket dtor called with pending write: " << _outBuffer.size()
                                                                << ", read: " << _inBuffer.size());
        if (!_doneDisconnect)
        {
            // This dtor could be called from a different thread when we are owned by
            // a weak_ptr elevated to a shared_ptr while the real owning shared_ptr
            // is destroyed. This can happen when we remove a closed socket from the
            // poll while in another thread a weak_ptr on it has temporarily lock()'d
            // and got another valid reference to it.
            // In that case, the real owner should've called ensureDisconnected()
            // and we won't need it again here, hence the conditional, and won't get
            // tripped-up by the ASSERT_CORRECT_SOCKET_THREAD check inside it.
            // Otherwise, we will invoke it and it's only fair to catch the thread
            // affinity violation.
            ensureDisconnected();
        }

        _socketHandler.reset();

        if (!_shutdownSignalled)
        {
            _shutdownSignalled = true;
            StreamSocket::shutdownConnection();
        }
        if (isExternalCountedConnection())
            --ExternalConnectionCount;
    }

    /// Emit 'onDisconnect' if it has not been done
    void ensureDisconnected()
    {
        ASSERT_CORRECT_SOCKET_THREAD(this);
        if (!_doneDisconnect)
        {
            _doneDisconnect = true;
            if (_socketHandler)
            {
                _socketHandler->onDisconnect();

                // The SocketHandler has a weak pointer to us and we could
                // be getting destroyed at this point, so it won't get a
                // reference to us from the weak pointer, and so can't disconnect.
                if (!isShutdown())
                {
                    asyncShutdown(); // signal
                    shutdownConnection(); // real -> setShutdown()
                }
            }
        }

        if (isOpen())
        {
            // Note: Ensure proper semantics of onDisconnect()
            LOG_WRN("Socket still open post onDisconnect(), forced shutdown.");
        }
    }

    bool isWebSocket() const { return _wsState == WSState::WS; }
    void setWebSocket() { _wsState = WSState::WS; }
    bool isLocalHost() const { return _isLocalHost; }

    /// Returns the peer hostname, if set.
    const std::string& hostname() const { return _hostname; }

    std::ostream& stream(std::ostream& os) const override;

    /// Checks whether StreamSocket is due for forced removal, e.g. by inactivity. Method will shutdown connection and socket on forced removal.
    /// Returns true in case of forced removal, caller shall stop processing
    bool checkRemoval(std::chrono::steady_clock::time_point now);

    void asyncShutdown()
    {
        _shutdownSignalled = true;
        LOG_TRC("Async shutdown requested");
    }

    void ignoreInput() override
    {
        Socket::ignoreInput();
        _inBuffer.clear();
    }

    /// Perform the real shutdown.
    virtual void shutdownConnection() { syncShutdown(); }

    int getPollEvents(std::chrono::steady_clock::time_point now,
                      int64_t &timeoutMaxMicroS) override
    {
        // cf. SslSocket::getPollEvents
        ASSERT_CORRECT_SOCKET_THREAD(this);
        int events = _socketHandler->getPollEvents(now, timeoutMaxMicroS);
        if (!_outBuffer.empty() || _shutdownSignalled)
            events |= POLLOUT;
        return events;
    }

    bool hasBuffered() const override
    {
        return !_outBuffer.empty() || !_inBuffer.empty();
    }

    std::size_t totalBufferCapacity() const override
    {
        return _outBuffer.capacity() + _inBuffer.capacity();
    }

    /// Create a pair of connected stream sockets
    static bool socketpair(std::chrono::steady_clock::time_point creationTime,
                           std::shared_ptr<StreamSocket>& parent,
                           std::shared_ptr<StreamSocket>& child);

    /// Send data to the socket peer.
    void send(const char* data, const int len, const bool doFlush = true)
    {
        ASSERT_CORRECT_SOCKET_THREAD(this);
        if (data != nullptr && len > 0)
        {
            _outBuffer.append(data, len);
            if (doFlush)
                writeOutgoingData();
        }
    }

    /// Send a string to the socket peer.
    void send(const std::string_view str, const bool doFlush = true)
    {
        send(str.data(), str.size(), doFlush);
    }

    /// Send an http::Request and flush.
    /// Does not add any fields to the header.
    /// Will shutdown the socket upon error and return false.
    bool send(http::Request& request);

    /// Send an http::Response and flush.
    /// Does not add any fields to the header.
    /// Will shutdown the socket upon error and return false.
    bool send(const http::Response& response);

    /// Send an http::Response, flush, and shutdown.
    /// Will set 'Connection: close' header.
    /// Returns true if no errors are encountered.
    /// Will always shutdown the socket.
    bool sendAndShutdown(http::Response& response);

    /// Safely attempt to write any outgoing data.
    /// Returns true iff no data is left in the buffer.
    bool attemptWrites()
    {
        if (!_outBuffer.empty())
            writeOutgoingData();

        return _outBuffer.empty();
    }

#if !MOBILEAPP

    /// Sends data with file descriptor as control data.
    /// Can be used only with Unix sockets.
    void sendFDs(const char* data, const uint64_t len, const std::vector<int>& fds)
    {
        ASSERT_CORRECT_SOCKET_THREAD(this);

        // Flush existing non-ancillary data
        // so that our non-ancillary data will
        // match ancillary data.
        attemptWrites();

        msghdr msg;
        iovec iov[1];

        iov[0].iov_base = const_cast<char*>(data);
        iov[0].iov_len = len;

        msg.msg_name = nullptr;
        msg.msg_namelen = 0;
        msg.msg_iov = &iov[0];
        msg.msg_iovlen = 1;

        const size_t fds_size = sizeof(int) * fds.size();
        auto* adata = static_cast<char*>(alloca(CMSG_SPACE(fds_size)));
        cmsghdr *cmsg = (cmsghdr*)adata;
        cmsg->cmsg_type = SCM_RIGHTS;
        cmsg->cmsg_level = SOL_SOCKET;
        cmsg->cmsg_len = CMSG_LEN(fds_size);
        int* fdsField = (int *)CMSG_DATA(cmsg);
        memcpy(fdsField, fds.data(), fds_size);

        msg.msg_control = adata;
        msg.msg_controllen = CMSG_LEN(fds_size);
        msg.msg_flags = 0;

#ifdef LOG_SOCKET_DATA
        if (len > 0)
            LOG_TRC("(Unix) outBuffer (" << len << " bytes):\n"
                                         << HexUtil::dumpHex(std::string(data, len)));
#endif

        //FIXME: retry on EINTR?
        const auto wrote = sendmsg(getFD(), &msg, 0);
        if (wrote < 0)
            LOG_SYS("Failed to send message to unix socket");
        else
            LOG_TRC("Wrote " << wrote << " bytes of " << len);
    }
#endif // !MOBILEAPP

    /// Reads data by invoking readData() and buffering.
    /// Returns the last return from writeData. 0 implies socket is closed.
    virtual int readIncomingData()
    {
        ASSERT_CORRECT_SOCKET_THREAD(this);

        if (ignoringInput())
        {
            LOG_WRN("Ignoring attempted read from " << getFD());
            return false; // error - close it.
        }

        ssize_t len = 0;
        if constexpr (!Util::isMobileApp())
        {
            // SSL decodes blocks of 16Kb, so for efficiency we use the same.
            char buf[16 * 1024];
            int last_errno = 0;
            do
            {
                // Drain the read buffer.
                // Note: we read as much as possible as
                // we are typically capped by hardware buffer
                // size anyway, and better to drain it fast.
                do
                {
                    len = readData(buf, sizeof(buf));
                    if (len < 0)
                        last_errno = errno; // Save only on error.

                    if (len < 0 && last_errno != EAGAIN && last_errno != EWOULDBLOCK)
                    {
                        if (last_errno == ECONNRESET && _inBuffer.empty())
                        {
                            // Unexpected, but often intentional on an idle connection.
                            LOGA_TRC(Socket, "Read failed because the connection is reset by peer, "
                                             "have 0 buffered bytes: ECONNRESET");
                        }
                        else
                        {
                            // Unexpected read error while draining the read buffer.
                            LOG_ERR_ERRNO(last_errno, "Read failed, have " << _inBuffer.size()
                                                                           << " buffered bytes");
                        }
                    }
                    else if (len < 0)
                        LOGA_TRC(Socket, "Read failed (" << len << "), have " << _inBuffer.size()
                                                         << " buffered bytes ("
                                                         << Util::symbolicErrno(last_errno) << ": "
                                                         << std::strerror(last_errno) << ')');
                    else if (len == 0)
                        LOGA_TRC(Socket,
                                 "Read closed (0), have " << _inBuffer.size() << " buffered bytes");
                    else // Success.
                        LOGA_TRC(Socket,
                                 "Read " << len << " bytes in addition to " << _inBuffer.size()
                                         << " buffered bytes"
#ifdef LOG_SOCKET_DATA
                                         << (len ? HexUtil::dumpHex(std::string(buf, len), ":\n")
                                                 : std::string())
#endif
                        );
                } while (len < 0 && last_errno == EINTR);

                if (len > 0)
                {
                    assert(len <= ssize_t(sizeof(buf)) && "Read more data than the buffer size");
                    notifyBytesRcvd(len);
                    const size_t origSize = _inBuffer.size();
                    _inBuffer.append(&buf[0], len);
                    if (origSize < 104857600 && _inBuffer.size() > 104857600)
                        LOG_WRN("inBuffer for " << getFD() << " has grown to " << _inBuffer.size() << " bytes");
                }
                // else poll will handle errors.
            } while (len == static_cast<ssize_t>(sizeof(buf)));

            // Restore errno from the read call.
            errno = last_errno;
        }
        else
        {
            LOG_TRC("readIncomingData #" << getFD());
            ssize_t available = fakeSocketAvailableDataLength(getFD());
            if (available == -1)
                len = -1;
            else if (available == 0)
                len = 0;
            else
            {
                std::vector<char> buf(available);
                len = readData(buf.data(), available);
                assert(len == available);
                notifyBytesRcvd(len);
                // It might happen that several messages need to be buffered if they arrive quicker
                // than we can handle them. In the non-MOBILEAPP case they are WebSocket messages so
                // they already contain a header indicating their length. Not so in the MOBILEAPP
                // case, so prefix them with a length header.
                _inBuffer.append((const char*)&len, sizeof(ssize_t));
                _inBuffer.append(buf.data(), len);
            }
        }

        return len;
    }

    /// Replace the existing SocketHandler with a new one.
    void setHandler(std::shared_ptr<ProtocolHandlerInterface> handler)
    {
        LOG_TRC("setHandler");
        resetHandler(); // don't disconnect the pre-upgrade handler
        _socketHandler = std::move(handler);
        ProtocolThreadOwnerChange::setThreadOwner(*_socketHandler, getThreadOwner());
        _socketHandler->onConnect(shared_from_this());
    }

    /// Explicitly avoids onDisconnect
    void resetHandler()
    {
        LOG_TRC("resetHandler");
        _socketHandler.reset();
    }

    /// Create a socket of type TSocket derived from StreamSocket given an FD and a handler.
    /// We need this helper since the handler needs a shared_ptr to the socket
    /// but we can't have a shared_ptr in the ctor.
    template <typename TSocket>
    static std::shared_ptr<TSocket>
    create(std::string hostname, int fd, Type type, bool isClient, HostType hostType,
           std::shared_ptr<ProtocolHandlerInterface> handler,
           ReadType readType = ReadType::NormalRead,
           std::chrono::steady_clock::time_point creationTime = std::chrono::steady_clock::now())
        requires(std::is_base_of_v<StreamSocket, TSocket>)
    {
        // Without a handler we make no sense object.
        if (!handler)
            throw std::runtime_error("StreamSocket " + std::to_string(fd) +
                                     " expects a valid SocketHandler instance.");

        auto socket = std::make_shared<TSocket>(std::move(hostname), fd, type, isClient, hostType, readType, creationTime);
        socket->setHandler(std::move(handler));

        return socket;
    }

        /// Messages can be in chunks, only parts of message being valid.
    struct MessageMap {
        MessageMap() : _headerSize(0), _messageSize(0) {}
        /// Size of HTTP headers
        size_t _headerSize;
        /// Entire size of data associated with this message
        size_t _messageSize;
        // offset + lengths to collate into the real stream
        std::vector<std::pair<size_t, size_t>> _spans;
    };

    /// remove all queued input bytes
    void clearInput()
    {
        _inBuffer.clear();
    }

    /// Remove the first @count bytes from input buffer
    void eraseFirstInputBytes(const std::size_t count)
    {
        const size_t toErase = std::min(count, _inBuffer.size());
        if (toErase < count)
            LOG_ERR("Attempted to remove: " << count << " which is > size: " << _inBuffer.size()
                                            << " clamped to " << toErase);
        if (toErase > 0)
            _inBuffer.eraseFirst(toErase);
    }

    /// Compacts chunk headers away leaving just the data we want
    /// returns true if we did any re-sizing/movement of _inBuffer.
    bool compactChunks(MessageMap& map);

    ssize_t readHeader(std::string_view clientName, std::istream& message, size_t messagesize,
                       Poco::Net::HTTPRequest& request,
                       std::chrono::duration<float, std::milli> delayMs);

    /// Detects if we have an HTTP header in the provided message and
    /// populates a request for that.
    bool parseHeader(std::string_view clientName, size_t headerSize, size_t bufferSize,
                     const Poco::Net::HTTPRequest& request,
                     std::chrono::duration<float, std::milli> delayMs, MessageMap& map);

    void handleExpect(std::string_view expect);

    bool checkChunks(const Poco::Net::HTTPRequest& request, size_t headerSize, MessageMap& map,
                     std::chrono::duration<float, std::milli> delayMs);

    Buffer& getInBuffer() { return _inBuffer; }

    const Buffer& getOutBuffer() const { return _outBuffer; }
    Buffer& getOutBuffer()
    {
        return _outBuffer;
    }

    int getIncomingFD(SharedFDType eType) const
    {
        const size_t eTypeIdx = static_cast<size_t>(eType);
        if (eTypeIdx < _incomingFDs.size())
            return _incomingFDs[eTypeIdx];
        return -1;
    }

    bool processInputEnabled() const { return _inputProcessingEnabled; }
    void enableProcessInput(bool enable = true){ _inputProcessingEnabled = enable; }

    /// The available number of bytes in the socket
    /// buffer for an optimal transmission.
    int getSendBufferCapacity() const
    {
        if constexpr (Util::isMobileApp())
            return INT_MAX; // We want to always send a single record in one go
        const int capacity = getSendBufferSize();
        return std::max<int>(0, capacity - _outBuffer.size());
    }

    virtual long getSslVerifyResult()
    {
        return 0;
    }

    virtual std::string getSslCert(std::string&)
    {
        return std::string();
    }

    /// Called when a polling event is received.
    /// @events is the mask of events that triggered the wake.
    void handlePoll(SocketDisposition &disposition,
                    std::chrono::steady_clock::time_point now,
                    const int events) override
    {
        ASSERT_CORRECT_SOCKET_THREAD(this);
        assert((getFD() >= 0 || isShutdown()) && "Socket is closed but not marked correctly");

        if (_socketHandler->checkTimeout(now))
        {
            assert(isShutdown() && "checkTimeout should have issued shutdown");
            setShutdown();
            LOGA_DBG(Socket, "socket timeout: " << getStatsString(now) << ", " << *this);
            disposition.setClosed();
            return;
        }

        if (!isOpen() || checkRemoval(now))
        {
            disposition.setClosed();
            return;
        }

        if (!events && _inBuffer.empty())
            return;

        setLastSeenTime(now);

        bool closed = (events & (POLLHUP | POLLERR | POLLNVAL));

        if (events & POLLIN)
        {
            // readIncomingData returns 0 only if the read len is 0 (closed).
            // Oddly enough, we don't necessarily get POLLHUP after read(2) returns 0.
            const int read = readIncomingData();
            const int last_errno = errno;
            LOGA_TRC(Socket, "Incoming data buffer "
                                 << _inBuffer.size() << " bytes, read result: " << read
                                 << ", events: 0x" << std::hex << events << std::dec << " ("
                                 << (closed ? "closed" : "not closed") << ')'
#ifdef LOG_SOCKET_DATA
                                 << (!_inBuffer.empty() ? HexUtil::dumpHex(_inBuffer, ":\n")
                                                        : std::string())
#endif
            );

            if (read > 0 && closed)
            {
                // We might have outstanding data to read, wait until readIncomingData returns closed state.
                LOG_DBG("Closed but will drain incoming data per POLLIN");
                closed = false;
            }
            else if (read < 0 && closed && (last_errno == EAGAIN || last_errno == EINTR))
            {
                LOG_DBG("Ignoring POLLHUP to drain incoming data as we had POLLIN but got "
                        << Util::symbolicErrno(last_errno) << " on read");
                closed = false;
            }
            else if (read == 0 || (read < 0 && (last_errno == EPIPE || last_errno == ECONNRESET)))
            {
                // There is nothing more to read; either we got EOF, or we drained after ECONNRESET.
                LOG_DBG("Closed after reading. Read result: " << read << " errno: " << Util::symbolicErrno(last_errno));
                closed = true;
            }
        }

        // If we have data, allow the app to consume.
        size_t oldSize = 0;
        while (!_inBuffer.empty() && oldSize != _inBuffer.size() && processInputEnabled())
        {
            oldSize = _inBuffer.size();

            try
            {
                // Keep the current handler alive, while the incoming message is handled.
                std::shared_ptr<ProtocolHandlerInterface> socketHandler(_socketHandler);

                _socketHandler->handleIncomingMessage(disposition);
            }
            catch (const std::exception& exception)
            {
                LOG_ERR("Error during handleIncomingMessage: " << exception.what());
                disposition.setClosed();
            }
            catch (...)
            {
                LOG_ERR("Error during handleIncomingMessage.");
                disposition.setClosed();
            }

            if (disposition.isTransfer())
                return;
        }

        do
        {
            // If we have space for writing and that was requested
            if (events & POLLOUT)
            {
                // Try to get multiple blocks at a time.
                const int capacity = getSendBufferCapacity();
                if (capacity > 0)
                    _socketHandler->performWrites(capacity);
            }

            // perform the shutdown if we have sent everything.
            if (_shutdownSignalled && _outBuffer.empty())
            {
                LOG_TRC("Shutdown Signaled. Close Connection.");
                shutdownConnection();
                closed = true;
                break;
            }

            oldSize = _outBuffer.size();

            // Write if we can and have data to write.
            if ((events & POLLOUT) && !_outBuffer.empty())
            {
                if (writeOutgoingData() < 0)
                {
                    const int last_errno = errno;
                    if (last_errno == EPIPE || last_errno == ECONNRESET)
                    {
                        LOG_DBG("Disconnected while writing (" << Util::symbolicErrno(last_errno)
                                                               << "): " << std::strerror(last_errno)
                                                               << ')');
                        closed = true;
                        break;
                    }
                }
            }
        } while (oldSize != _outBuffer.size());

        if (closed)
        {
            LOG_TRC("Closed. Firing onDisconnect.");
            ensureDisconnected();
            setShutdown();
            disposition.setClosed();
        }
        else if (!isOpen())
            disposition.setClosed();
    }

    /// Override to write data out to socket.
    /// Returns the last return from writeData.
    virtual int writeOutgoingData()
    {
        ASSERT_CORRECT_SOCKET_THREAD(this);
        assert(!_outBuffer.empty());
        ssize_t len = 0;
        int last_errno = 0;
        do
        {
            do
            {
                // Writing much more than we can absorb in the kernel causes wastage.
                const int size = std::min((int)_outBuffer.getBlockSize(), getSendBufferSize());
                if (size == 0)
                    break;

                len = writeData(_outBuffer.getBlock(), size);
                if (len < 0)
                    last_errno = errno; // Save only on error.

                // 0 len is unspecified result, according to man write(2).
                if (len < 0 && last_errno != EAGAIN && last_errno != EWOULDBLOCK)
                    LOG_ERR_ERRNO(last_errno, "Socket write returned " << len);
                else if (len <= 0) // Trace errno for debugging, even for "unspecified result."
                    LOGA_TRC(Socket, "Write failed, have " << _outBuffer.size() << " buffered bytes ("
                             << Util::symbolicErrno(last_errno) << ": "
                             << std::strerror(last_errno) << ')');
                else // Success.
                    LOGA_TRC(Socket,
                             "Wrote "
                                 << len << " bytes of " << _outBuffer.size() << " buffered data"
#ifdef LOG_SOCKET_DATA
                                 << (len ? HexUtil::dumpHex(std::string(_outBuffer.getBlock(), len),
                                                            ":\n")
                                         : std::string())
#endif
                    );
            }
            while (len < 0 && last_errno == EINTR);

            if (len > 0)
            {
                LOG_ASSERT_MSG(len <= ssize_t(_outBuffer.size()),
                               "Consumed more data than available");
                notifyBytesSent(len);
                _outBuffer.eraseFirst(len);
            }
            else
            {
                // Poll will handle errors.
                break;
            }
        }
        while (!_outBuffer.empty());

        // Restore errno from the write call.
        errno = last_errno;
        return len;
    }

    /// Does it look like we have some TLS / SSL where we don't expect it ?
    bool sniffSSL() const;

    void dumpState(std::ostream& os) override;

    static size_t getExternalConnectionCount() { return ExternalConnectionCount; }

protected:
    void handshakeFail()
    {
        if (_socketHandler)
            _socketHandler->onHandshakeFail();
    }

#if !MOBILEAPP
    /// Reads data with file descriptors as control data if received.
    /// Can be used only with Unix sockets.
    int readFDs(char* buf, int len, std::vector<int>& fds)
    {
        // 0 is smaps FD
        // 1 is urp FD
        const size_t maxFds = 2;

        msghdr msg;
        iovec iov[1];
        /// We don't expect more than maxFds FDs
        char ctrl[CMSG_SPACE(sizeof(int)) * maxFds];
        int ctrlLen = sizeof(ctrl);

        iov[0].iov_base = buf;
        iov[0].iov_len = len;

        msg.msg_name = nullptr;
        msg.msg_namelen = 0;
        msg.msg_iov = &iov[0];
        msg.msg_iovlen = 1;
        msg.msg_control = ctrl;
        msg.msg_controllen = ctrlLen;
        msg.msg_flags = 0;

        int ret = recvmsg(getFD(), &msg, 0);
        if (ret > 0 && msg.msg_controllen)
        {
            cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
            if (cmsg && cmsg->cmsg_type == SCM_RIGHTS)
            {
                size_t fds_count = static_cast<size_t>(cmsg->cmsg_len - CMSG_LEN(0)) / sizeof(int);
                int* fdsField = (int*)CMSG_DATA(cmsg);
                fds.assign(fdsField, fdsField + fds_count);
                if (_readType == ReadType::UseRecvmsgExpectFD)
                {
                    _readType = ReadType::NormalRead;
                }
            }
        }

        return ret;
    }
#endif // !MOBILEAPP

    /// Override to handle reading of socket data differently.
    virtual int readData(char* buf, int len)
    {
        ASSERT_CORRECT_SOCKET_THREAD(this);
        assert((getFD() >= 0 || isShutdown()) && "Socket is closed but not marked correctly");

        // avoided in readIncomingData
        if (ignoringInput())
            return -1;

#if !MOBILEAPP
        if (_readType == ReadType::UseRecvmsgExpectFD)
            return readFDs(buf, len, _incomingFDs);

#if ENABLE_DEBUG
        if (simulateSocketError(true))
            return -1;
#endif

        return ::read(getFD(), buf, len);
#else
        return fakeSocketRead(getFD(), buf, len);
#endif
    }

    /// Override to handle writing data to socket differently.
    virtual int writeData(const char* buf, const int len)
    {
        ASSERT_CORRECT_SOCKET_THREAD(this);
        assert((getFD() >= 0 || isShutdown()) && "Socket is closed but not marked correctly");

#if !MOBILEAPP
#if ENABLE_DEBUG
        if (simulateSocketError(false))
            return -1;
#endif
        return ::write(getFD(), buf, len);
#else
        return fakeSocketWrite(getFD(), buf, len);
#endif
    }

    void setShutdownSignalled()
    {
        _shutdownSignalled = true;
    }

    bool isShutdownSignalled() const
    {
        return _shutdownSignalled;
    }

    void setThreadOwner(const ProcUtil::ThreadId id, LOG_CAPTURE_CALLER_DECLARATION) override
    {
        Socket::setThreadOwner(id, LOG_PASS_PARENT_CALLER);
        if (_socketHandler)
            ProtocolThreadOwnerChange::setThreadOwner(*_socketHandler, id, LOG_PASS_PARENT_CALLER);
    }

    void resetThreadOwner(LOG_CAPTURE_CALLER_DECLARATION) override
    {
        Socket::resetThreadOwner(LOG_PASS_PARENT_CALLER);
        if (_socketHandler)
            ProtocolThreadOwnerChange::resetThreadOwner(*_socketHandler, LOG_PASS_PARENT_CALLER);
    }

#if ENABLE_DEBUG
    /// Return true and set errno to simulate an error
    bool simulateSocketError(bool read);
#endif

private:
    /// The hostname (or IP) of the peer we are connecting to.
    const std::string _hostname;

    Buffer _inBuffer;
    Buffer _outBuffer;

    std::vector<int> _incomingFDs;

    /// Client handling the actual data.
    std::shared_ptr<ProtocolHandlerInterface> _socketHandler;

    STATE_ENUM(WSState, HTTP, WS);
    WSState _wsState;

    ReadType _readType;

    /// True when shutdown was requested via shutdown().
    /// It's accessed from different threads.
    std::atomic_bool _shutdownSignalled;
    std::atomic_bool _inputProcessingEnabled;

    /// Did we emit the onDisconnect event yet
    bool _doneDisconnect;

    /// True if owner is in client role, otherwise false (server)
    bool _isClient:1;

    /// True if host is localhost
    bool _isLocalHost:1;

    /// True if we've received a Continue in response to an Expect: 100-continue
    bool _sentHTTPContinue:1;

    bool isExternalCountedConnection() const { return !_isClient && isIPType(); }
    static std::atomic<size_t> ExternalConnectionCount; // accepted external TCP IPv4/IPv6 socket count
};

enum class WSOpCode : unsigned char {
    Continuation = 0x0,
    Text         = 0x1,
    Binary       = 0x2,
    Reserved1    = 0x3,
    Reserved2    = 0x4,
    Reserved3    = 0x5,
    Reserved4    = 0x6,
    Reserved5    = 0x7,
    Close        = 0x8,
    Ping         = 0x9,
    Pong         = 0xa
    // ... reserved
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
