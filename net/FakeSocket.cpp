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

/*
 * Implementation of socket API emulation for mobile platforms.
 * Classes: FakeSocketPair (internal)
 * Functions: fakeSocket* family implementing socket operations
 */

#include <config.h>

#include <common/ProcUtil.hpp>

#include <fcntl.h>
#ifndef _WIN32
#include <poll.h>
#endif

#include <cassert>
#include <cerrno>
#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <mutex>
#include <vector>

// A "fake socket" is represented by a number, a smallish integer, just like a real socket on
// Linux. Unlike real sockets, fake socket numbers are not file descriptors, they have nothing to do
// with the funcitons open(), read() etc, and will overlap with file descriptors.
//
// There is one FakeSocketPair for each two sequential fake socket numbers. When you create a socket
// with fakeSocketSocket(), you will always get the lower (even) number in a pair. When creating a
// socket pair with fakeSocketPipe2() the two sequential numbers will be returned.
//
// An odd number will be returned from fakeSocketAccept4(). It is the number that is one higher than
// the first parameter to the corresponding fakeSocketConnect().
//
// After you create a fake socket, there is basically just two things you can do with it:
//
// 1) Call fakeSocketListen() on it, indicating it is a "server" socket. After that, keep calling
// fakeSocketAccept() and each time that returns successfully, it will return a new fake socket that
// is connected to another fake socket that called fakeSocketConnect() to the server socket. You can
// then call fakeSocketRead() and fakeSocketWrite() on it.
//
// 2) Call fakeSocketConnect() on it giving another fake socket number to connect to. That should be
// a listening socket. Once the connection is successful, you can call fakeSocketRead() and
// fakeSocketWrite() on your original socket.
//
// This all is complicated a bit by the fact that all the API is non-blocking.

struct FakeSocketPair
{
    int fd[2];
    bool listening;
    int connectingFd;
    bool shutdown[2];
    bool readable[2];
    bool eofDetected[2];                      ///< Flag for fakeSocketHasAnyPendingActivityGlobal() to avoid a busy loop after EOF.
    std::vector<std::vector<char>> buffer[2];

    FakeSocketPair()
    {
        fd[0] = -1;
        fd[1] = -1;
        listening = false;
        connectingFd = -1;
        shutdown[0] = false;
        shutdown[1] = false;
        readable[0] = false;
        readable[1] = false;
        eofDetected[0] = false;
        eofDetected[1] = false;
    }
};

static thread_local std::ostringstream loggingBuffer;
static void (*loggingCallback)(const std::string&) = nullptr;

static std::mutex theMutex;
static std::condition_variable theCV;

static int fakeSocketLogLevel = -1;

static void fakeSocketDumpStateImpl();

static std::vector<std::unique_ptr<FakeSocketPair>> fds;

static std::string flush()
{
    static bool alwaysStderr = std::getenv("FAKESOCKET_LOG_ALWAYS_STDERR") != nullptr;
    if (alwaysStderr)
        std::cerr << ProcUtil::getThreadId() << ':' << loggingBuffer.str() << std::endl;
    else if (loggingCallback != nullptr)
        loggingCallback(loggingBuffer.str());
    loggingBuffer.str("");
    return std::string();
}

#ifdef __ANDROID__
// kill the verbose logging on Android
#define FAKESOCKET_LOG(arg)
#else
#define FAKESOCKET_LOG(arg) do { if (fakeSocketLogLevel > 0) { loggingBuffer << arg; } } while (false)
#endif

EXPORT
void fakeSocketSetLoggingCallback(void (*callback)(const std::string&))
{
    loggingCallback = callback;
}

static FakeSocketPair& fakeSocketAllocate()
{
    if (fakeSocketLogLevel == -1)
    {
        char *doLog = std::getenv("FAKESOCKET_LOG");
        if (doLog == nullptr || *doLog == '\0')
            fakeSocketLogLevel = 0;
        else
            fakeSocketLogLevel = 1;
    }

    std::lock_guard<std::mutex> lock(theMutex);

    // We always allocate a new FakeSocketPair struct. Let's not bother with potential issues with
    // reusing them. It isn't like we would be allocating thousands anyway during the typical
    // lifetime of an app. Also, not reusing FakeSocket fd numbers means that it is easier to set a
    // conditional breakpoint on an operation on a specific fd when debugging some problematic
    // scenario.

    const int i = fds.size();
    fds.resize(i + 1);

    fds[i] = std::make_unique<FakeSocketPair>();
    fds[i]->fd[0] = i*2;

    return *(fds[i]);
}

EXPORT
int fakeSocketSocket()
{
    const int result = fakeSocketAllocate().fd[0];

    FAKESOCKET_LOG("FakeSocket Create #" << result << flush());

    return result;
}

EXPORT
int fakeSocketPipe2(int pipefd[2])
{
    FakeSocketPair& pair = fakeSocketAllocate();
    pipefd[0] = pair.fd[0];

    pipefd[1] = pair.fd[1] = pair.fd[0] + 1;

    FAKESOCKET_LOG("FakeSocket Pipe created (#" << pipefd[0] << ",#" << pipefd[1] << ')' << flush());

    return 0;
}

static std::string pollBits(int bits)
{
    if (bits == 0)
        return "-";

    std::string result;

    if (bits & POLLERR)
    {
        if (result != "")
            result += '+';
        result += "ERR";
    }
    if (bits & POLLHUP)
    {
        if (result != "")
            result += '+';
        result += "HUP";
    }
    if (bits & POLLIN)
    {
        if (result != "")
            result += '+';
        result += "IN";
    }
    if (bits & POLLNVAL)
    {
        if (result != "")
            result += '+';
        result += "NVAL";
    }
    if (bits & POLLOUT)
    {
        if (result != "")
            result += '+';
        result += "OUT";
    }
    if (bits & POLLPRI)
    {
        if (result != "")
            result += '+';
        result += "PRI";
    }

    return result;
}

/**
 * Scan a set of pollfds and set their revents according to the fake-socket state.
 *
 * Returns true if any pollfd has any revent ("returned event") set
 * (ie. at least one fd is "ready").
 */
static bool checkForPoll(struct pollfd *pollfds, int nfds)
{
    bool retval = false;

    for (int i = 0; i < nfds; i++)
    {
        struct pollfd &pfd = pollfds[i];

        const int fdRaw = pfd.fd;                                    // raw fd value from the pollfd
        const unsigned pairIndex = static_cast<unsigned>(fdRaw / 2); // which FakeSocketPair
        const int K = (fdRaw & 1);                                   // "this" endpoint index for the current FakeSocketPair
        const int N = 1 - K;                                         // the "peer" endpoint index

        // Validate the fd: negative or pair index out of range -> POLLNVAL
        if (fdRaw < 0 || pairIndex >= fds.size())
        {
            pfd.revents = POLLNVAL;
            retval = true;
            continue;
        }

        // pairIndex is in range, check that the endpoint exists
        const auto& currentPair = fds[pairIndex];
        if (currentPair->fd[K] == -1)
        {
            // this endpoint is closed
            pfd.revents = POLLNVAL;
            retval = true;
            continue;
        }

        // start with no events to return
        pfd.revents = 0;

        // POLLIN readiness:
        //  - readable[K] means there is a buffered record waiting to be read.
        //  - For endpoint K==0 (the "listener" half), POLLIN also signals a pending accept
        //    when listening && connectingFd != -1.
        if (pfd.events & POLLIN)
        {
            if (currentPair->readable[K] ||
                (K == 0 && currentPair->listening && currentPair->connectingFd != -1))
            {
                pfd.revents |= POLLIN;
                retval = true;
            }
        }

        // POLLOUT readiness:
        // With multiple buffers, we consider the socket writable unless the peer
        // endpoint is gone or has been shut down.
        if (pfd.events & POLLOUT)
        {
            if (currentPair->fd[N] != -1 && !currentPair->shutdown[N])
            {
                pfd.revents |= POLLOUT;
                retval = true;
            }
        }

        // POLLHUP:
        // If the peer has been shut down, report HUP unconditionally; even if the caller
        // didn't request it in .events (mirrors the real poll()).
        if (currentPair->shutdown[N])
        {
            pfd.revents |= POLLHUP;
            retval = true;
        }
    }

    return retval;
}

/**
 * Scan all the 'fds' we have for fakeSockets, and check if any of them
 * reports any activity, so that we should rather skip the wait()/wait_until()
 * in fakeSocketWaitAny().
 *
 * NB: Must be called with theMutex held.
 */
static bool fakeSocketHasAnyPendingActivityGlobal()
{
    for (const auto& pairPtr : fds)
    {
        if (!pairPtr)
            continue;

        FakeSocketPair& p = *pairPtr;

        // Endpoint 0 (even fd) and endpoint 1 (odd fd)
        for (int K = 0; K < 2; ++K)
        {
            // Endpoint closed?
            if (p.fd[K] == -1)
                continue;

            const int N = 1 - K;

            // Pending accept on a listening socket?
            // Only meaningful on K==0
            if (K == 0 && p.listening && p.connectingFd != -1)
                return true;

            // Buffered data ready for this endpoint?
            if (!p.buffer[K].empty())
                return true;

            // Peer shutdown/closed?
            // Detect EOF via "readable[K]" so a read() can consume it (0 bytes).
            // But in fakeSocketRead(), we explicitly stay readable after peer
            // closed or shut down, so use a flag to return 'true' just once.
            if ((p.shutdown[N] || p.fd[N] == -1) && p.readable[K] && !p.eofDetected[K])
            {
                p.eofDetected[K] = true; // mark EOF consumed
                return true;
            }
        }
    }
    return false;
}

/**
 * Wait for any event on any of the fake sockets (theCV is notified on write/close/connect/etc.)
 */
EXPORT
void fakeSocketWaitAny(int timeoutUs)
{
    if (timeoutUs == 0)
        return;

    std::unique_lock<std::mutex> lock(theMutex);

    // Use fakeSocketHasAnyPendingActivityGlobal as wait predicate,
    // so that we:
    // a) don't enter wait()/wait_until() if we've got new events
    // that we might have missed since the last poll()
    // b) ignore spurious wakeups
    if (fakeSocketHasAnyPendingActivityGlobal())
        return;

    if (timeoutUs < 0)
    {
        theCV.wait(lock, [](){ return fakeSocketHasAnyPendingActivityGlobal(); });
        return;
    }

    auto const deadline = std::chrono::steady_clock::now() + std::chrono::microseconds(timeoutUs);
    theCV.wait_until(lock, deadline, [](){ return fakeSocketHasAnyPendingActivityGlobal(); });
}

EXPORT
int fakeSocketPoll(struct pollfd *pollfds, int nfds, int timeout)
{
    FAKESOCKET_LOG("FakeSocket Poll ");
    for (int i = 0; i < nfds; i++)
    {
        if (i > 0)
            FAKESOCKET_LOG(',');
        FAKESOCKET_LOG('#' << pollfds[i].fd << ':' << pollBits(pollfds[i].events));
    }
    FAKESOCKET_LOG(", timeout:" << timeout << flush());

    std::unique_lock<std::mutex> lock(theMutex);

    if (timeout > 0)
    {
        auto const now = std::chrono::steady_clock::now();
        auto const end = now + std::chrono::milliseconds(timeout);

        while (!checkForPoll(pollfds, nfds))
            if (theCV.wait_until(lock, end) == std::cv_status::timeout)
            {
                FAKESOCKET_LOG("FakeSocket Poll timeout: 0" << flush());
                return 0;
            }
    }
    else if (timeout == 0)
    {
        checkForPoll(pollfds, nfds);
    }
    else // timeout < 0
    {
        while (!checkForPoll(pollfds, nfds))
            theCV.wait(lock);
    }

    int result = 0;
    for (int i = 0; i < nfds; i++)
    {
        if (pollfds[i].revents != 0)
            result++;
    }

    FAKESOCKET_LOG("FakeSocket Poll result: ");
    for (int i = 0; i < nfds; i++)
    {
        if (i > 0)
            FAKESOCKET_LOG(',');
        FAKESOCKET_LOG('#' << pollfds[i].fd << ':' << pollBits(pollfds[i].revents));
    }
    FAKESOCKET_LOG(": " << result << flush());

    return result;
}

EXPORT
int fakeSocketListen(int fd)
{
    std::unique_lock<std::mutex> lock(theMutex);
    if (fd < 0 || static_cast<unsigned>(fd/2) >= fds.size() || fds[fd/2]->fd[fd&1] == -1)
    {
        FAKESOCKET_LOG("FakeSocket EBADF: Listening on #" << fd << flush());
        errno = EBADF;
        return -1;
    }

    FakeSocketPair& pair = *(fds[fd/2]);

    if (fd&1 || pair.fd[1] != -1)
    {
        FAKESOCKET_LOG("FakeSocket EISCONN: Listening on #" << fd << flush());
        errno = EISCONN;
        return -1;
    }

    if (pair.listening)
    {
        FAKESOCKET_LOG("FakeSocket EIO: Listening on #" << fd << flush());
        errno = EIO;
        return -1;
    }

    pair.listening = true;
    pair.connectingFd = -1;

    FAKESOCKET_LOG("FakeSocket Listen #" << fd << flush());

    return 0;
}

EXPORT
int fakeSocketConnect(int fd1, int fd2)
{
    std::unique_lock<std::mutex> lock(theMutex);
    if (fd1 < 0 || fd2 < 0 || static_cast<unsigned>(fd1/2) >= fds.size() || static_cast<unsigned>(fd2/2) >= fds.size())
    {
        FAKESOCKET_LOG("FakeSocket EBADF: Connect #" << fd1 << " to #" << fd2 << flush());
        errno = EBADF;
        return -1;
    }
    if (fd1/2 == fd2/2)
    {
        FAKESOCKET_LOG("FakeSocket EBADF: Connect #" << fd1 << " to #" << fd2 << flush());
        errno = EBADF;
        return -1;
    }

    FakeSocketPair& pair1 = *(fds[fd1/2]);
    FakeSocketPair& pair2 = *(fds[fd2/2]);

    if ((fd1&1) || (fd2&1))
    {
        FAKESOCKET_LOG("FakeSocket EISCONN: Connect #" << fd1 << " to #" << fd2 << flush());
        errno = EISCONN;
        return -1;
    }

    if (!pair2.listening)
    {
        FAKESOCKET_LOG("FakeSocket ECONNREFUSED: Connect #" << fd1 << " to #" << fd2 << flush());
        errno = ECONNREFUSED;
        return -1;
    }

    // FIXME: This is grim - we should have a queue of fds and
    // accept should pop them off the queue - for now block on
    // the other thread's accept completing.
    while (pair2.connectingFd != -1)
        theCV.wait(lock);

    assert(pair2.connectingFd == -1);
    pair2.connectingFd = fd1;
    theCV.notify_all();

    while (pair1.fd[1] == -1)
        theCV.wait(lock);

    assert(pair1.fd[1] == pair1.fd[0] + 1);

    FAKESOCKET_LOG("FakeSocket Connect #" << fd1 << " to #" << fd2 << ": #" << pair1.fd[1] << flush());

    return 0;
}

EXPORT
int fakeSocketAccept4(int fd)
{
    std::unique_lock<std::mutex> lock(theMutex);
    if (fd < 0 || static_cast<unsigned>(fd/2) >= fds.size())
    {
        FAKESOCKET_LOG("FakeSocket EBADF: Accept #" << fd << flush());
        errno = EBADF;
        return -1;
    }

    if (fd & 1)
    {
        FAKESOCKET_LOG("FakeSocket EISCONN: Accept #" << fd << flush());
        errno = EISCONN;
        return -1;
    }

    FakeSocketPair& pair = *(fds[fd/2]);

    if (!pair.listening)
    {
        FAKESOCKET_LOG("FakeSocket EIO: Accept #" << fd << flush());
        errno = EIO;
        return -1;
    }

    while (pair.connectingFd == -1)
        theCV.wait(lock);

    assert(pair.connectingFd >= 0);

    assert(static_cast<unsigned>(pair.connectingFd/2) < fds.size());
    assert((pair.connectingFd&1) == 0);

    FakeSocketPair& pair2 = *(fds[pair.connectingFd/2]);

    assert(pair2.fd[1] == -1);
    assert(pair2.fd[0] == pair.connectingFd);

    pair.connectingFd = -1;

    pair2.fd[1] = pair2.fd[0] + 1;

    theCV.notify_all();

    FAKESOCKET_LOG("FakeSocket Accept #" << fd << ": #" << pair2.fd[1] << flush());

    fakeSocketDumpStateImpl();

    return pair2.fd[1];
}

EXPORT
int fakeSocketPeer(int fd)
{
    std::unique_lock<std::mutex> lock(theMutex);
    if (fd < 0 || static_cast<unsigned>(fd/2) >= fds.size())
    {
        FAKESOCKET_LOG("FakeSocket EBADF: Peer of #" << fd << flush());
        errno = EBADF;
        return -1;
    }

    FakeSocketPair& pair = *(fds[fd/2]);

    const int K = (fd&1);
    const int N = 1 - K;

    FAKESOCKET_LOG("FakeSocket Peer of #" << fd << ": #" << pair.fd[N] << flush());

    return pair.fd[N];
}

EXPORT
ssize_t fakeSocketAvailableDataLength(int fd)
{
    std::unique_lock<std::mutex> lock(theMutex);
    if (fd < 0 || static_cast<unsigned>(fd/2) >= fds.size())
    {
        errno = EBADF;
        return -1;
    }

    FakeSocketPair& pair = *(fds[fd/2]);

    // K: for this fd
    const int K = (fd&1);

    if (!pair.readable[K])
    {
        FAKESOCKET_LOG("FakeSocket EAGAIN: Available data on #" << fd << flush());
        errno = EAGAIN;
        return -1;
    }

    ssize_t result = 0;
    if (pair.buffer[K].size() > 0)
        result = pair.buffer[K][0].size();

    FAKESOCKET_LOG("FakeSocket Available data on #" << fd << ": " << result << flush());

    return result;
}

EXPORT
ssize_t fakeSocketRead(int fd, void *buf, size_t nbytes)
{
    std::unique_lock<std::mutex> lock(theMutex);
    if (fd < 0 || static_cast<unsigned>(fd/2) >= fds.size())
    {
        FAKESOCKET_LOG("FakeSocket EBADF: Read from #" << fd << ", " << nbytes << (nbytes == 1 ? " byte" : " bytes") << flush());
        errno = EBADF;
        return -1;
    }

    FakeSocketPair& pair = *(fds[fd/2]);

    // K: for this fd
    const int K = (fd&1);
    // N: for its peer
    const int N = 1 - K;

    if (pair.fd[K] == -1)
    {
        FAKESOCKET_LOG("FakeSocket EBADF: Read from #" << fd << ", " << nbytes << (nbytes == 1 ? " byte" : " bytes") << flush());
        errno = EBADF;
        return -1;
    }

    if (pair.shutdown[K])
    {
        FAKESOCKET_LOG("FakeSocket Read from #" << fd << " (shut down) got 0 bytes" << flush());
        return 0;
    }

    if (!pair.readable[K])
    {
        FAKESOCKET_LOG("FakeSocket EAGAIN: Read from #" << fd << ", " << nbytes << (nbytes == 1 ? " byte" : " bytes") << flush());
        errno = EAGAIN;
        return -1;
    }

    ssize_t result = 0;
    if (pair.buffer[K].size() > 0)
    {
        // These sockets are record-oriented. It won't work to read less than the whole record in
        // turn to be read.
        result = pair.buffer[K][0].size();
        if (nbytes < static_cast<unsigned>(result))
        {
            FAKESOCKET_LOG("FakeSocket EAGAIN: Read from #" << fd << ", " << nbytes << (nbytes == 1 ? " byte" : " bytes") << flush());
            errno = EAGAIN; // Not the right errno, but what would be?
            return -1;
        }

        memmove(buf, pair.buffer[K][0].data(), result);
        pair.buffer[K].erase(pair.buffer[K].begin());
    }

    // If peer is closed or shut down, we continue to be readable
    if (pair.fd[N] == -1 || pair.shutdown[N])
        pair.readable[K] = true;
    else if (pair.buffer[K].empty())
        pair.readable[K] = false;

    FAKESOCKET_LOG("FakeSocket Read from #" << fd << " got " << result << (result == 1 ? " byte" : " bytes") << flush());

    return result;
}

EXPORT
ssize_t fakeSocketWrite(int fd, const void *buf, size_t nbytes)
{
    std::unique_lock<std::mutex> lock(theMutex);
    if (fd < 0 || static_cast<unsigned>(fd/2) >= fds.size())
    {
        FAKESOCKET_LOG("FakeSocket EBADF: Write to #" << fd << ", " << nbytes << (nbytes == 1 ? " byte" : " bytes") << flush());
        errno = EBADF;
        return -1;
    }

    FakeSocketPair& pair = *(fds[fd/2]);

    // K: for this fd
    // N: for its peer, whose read buffer we want to write into
    const int K = (fd&1);
    const int N = 1 - K;

    if (pair.fd[K] == -1)
    {
        FAKESOCKET_LOG("FakeSocket EBADF: Write to #" << fd << ", " << nbytes << (nbytes == 1 ? " byte" : " bytes") << flush());
        errno = EBADF;
        return -1;
    }

    if (pair.shutdown[K])
    {
        // Should we raise(SIGPIPE)? Probably not, Online code does not expect SIGPIPE at all...
        FAKESOCKET_LOG("FakeSocket EPIPE: Write to #" << fd << " (shut down), " << nbytes << (nbytes == 1 ? " byte" : " bytes") << flush());
        errno = EPIPE;
        return -1;
    }

    pair.buffer[N].emplace_back(std::vector<char>(nbytes));
    memmove(pair.buffer[N].back().data(), buf, nbytes);
    pair.readable[N] = true;

    theCV.notify_all();

    FAKESOCKET_LOG("FakeSocket Write to #" << fd << ": " << nbytes << (nbytes == 1 ? " byte" : " bytes") << flush());
    return nbytes;
}

EXPORT
int fakeSocketShutdown(int fd)
{
    std::unique_lock<std::mutex> lock(theMutex);
    if (fd < 0 || static_cast<unsigned>(fd/2) >= fds.size())
    {
        FAKESOCKET_LOG("FakeSocket EBADF: Shutdown #" << fd << flush());
        errno = EBADF;
        return -1;
    }

    FakeSocketPair& pair = *(fds[fd/2]);

    const int K = (fd&1);
    const int N = 1 - K;

    if (pair.fd[K] == -1)
    {
        FAKESOCKET_LOG("FakeSocket EBADF: Shutdown #" << fd << flush());
        errno = EBADF;
        return -1;
    }

    if (pair.fd[N] == -1)
    {
        FAKESOCKET_LOG("FakeSocket ENOTCONN: Shutdown #" << fd << flush());
        errno = ENOTCONN;
        return -1;
    }

    pair.shutdown[K] = true;
    pair.readable[N] = true; // wake the peer to observe EOF

    theCV.notify_all();

    FAKESOCKET_LOG("FakeSocket Shutdown #" << fd << flush());

    return 0;
}

EXPORT
int fakeSocketClose(int fd)
{
    std::unique_lock<std::mutex> lock(theMutex);
    if (fd < 0 || static_cast<unsigned>(fd/2) >= fds.size())
    {
        FAKESOCKET_LOG("FakeSocket EBADF: Close #" << fd << flush());
        errno = EBADF;
        return -1;
    }

    FakeSocketPair& pair = *(fds[fd/2]);

    const int K = (fd&1);
    const int N = 1 - K;

    if (pair.fd[K] == -1)
    {
        FAKESOCKET_LOG("FakeSocket EBADF: Close #" << fd << flush());
        errno = EBADF;
        return -1;
    }

    assert(pair.fd[K] == fd);

    pair.fd[K] = -1;
    pair.buffer[K].resize(0);
    pair.readable[N] = true;

    theCV.notify_all();

    FAKESOCKET_LOG("FakeSocket Close #" << fd << flush());

    fakeSocketDumpStateImpl();

    return 0;
}

static void fakeSocketDumpStateImpl()
{
    FAKESOCKET_LOG("FakeSocket open sockets:" << flush());
    for (int i = 0; i < static_cast<int>(fds.size()); i++)
    {
        if (fds[i]->fd[0] != -1)
        {
            assert(fds[i]->fd[0] == i*2);
            FAKESOCKET_LOG("  #" << fds[i]->fd[0]);
            if (fds[i]->fd[1] != -1)
            {
                assert(fds[i]->fd[1] == i*2+1);
                assert(!fds[i]->listening);
                FAKESOCKET_LOG(" <=> #" << fds[i]->fd[1]);
            }
            else if (fds[i]->listening)
            {
                FAKESOCKET_LOG(" listening");
            }
            FAKESOCKET_LOG(flush());
        }
        else if (fds[i]->fd[1] != -1)
        {
            assert(fds[i]->fd[1] == i*2+1);
            assert(!fds[i]->listening);
            FAKESOCKET_LOG("  #" << fds[i]->fd[1] << flush());
        }
    }
}

EXPORT
void fakeSocketDumpState()
{
    std::unique_lock<std::mutex> lock(theMutex);
    fakeSocketDumpStateImpl();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
