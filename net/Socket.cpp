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
 * Implementation of non-blocking socket I/O, polling, and protocol dispatch.
 * Classes: Socket, StreamSocket, SocketPoll, SocketDisposition, WebSocketHandler integration
 */

#include <config.h>
#include <config_version.h>

#include "Socket.hpp"

#include <common/ConfigUtil.hpp>
#include <common/HexUtil.hpp>
#include <common/Log.hpp>
#include <common/SigUtil.hpp>
#if !MOBILEAPP
#include <common/Syscall.hpp>
#endif
#include <common/TraceEvent.hpp>
#include <common/Unit.hpp>
#include <common/Util.hpp>
#include <common/Watchdog.hpp>
#include <common/base64.hpp>
#include <net/HttpRequest.hpp>
#include <net/NetUtil.hpp>
#include <net/ServerSocket.hpp>
#include <net/WebSocketHandler.hpp>

#if !MOBILEAPP && ENABLE_SSL
#include <net/SslSocket.hpp>
#include <openssl/x509v3.h>
#endif

#include <cctype>
#include <cerrno>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <cstring>
#include <iomanip>
#include <memory>
#include <ostream>
#include <ratio>
#include <sstream>
#include <stdexcept>
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
// The windows app build cannot compile Poco's net headers.
#ifndef _WIN32
#include <Poco/Net/WebSocket.h> // computeAccept
#endif

#include <Poco/URI.h>

#if ENABLE_SSL
#include <Poco/Net/X509Certificate.h>
#endif

// Bug in pre C++17 where static constexpr must be defined. Fixed in C++17.
constexpr std::chrono::microseconds SocketPoll::DefaultPollTimeoutMicroS;
constexpr std::chrono::microseconds WebSocketHandler::InitialPingDelayMicroS;

std::unique_ptr<Watchdog> SocketPoll::PollWatchdog;

std::atomic<size_t> StreamSocket::ExternalConnectionCount = 0;

net::DefaultValues net::Defaults = { .inactivityTimeout = net::DefaultInactivityTimeout,
                                     .maxExtConnections = net::DefaultMaxExtConnections,
                                     .pongTimeout = net::DefaultPongTimeout };

size_t StreamSocket::BufferBloatCloseSize = 100 * 1024 * 1024;
std::chrono::milliseconds StreamSocket::BufferBloatCloseDuration = std::chrono::minutes(2);
std::atomic<size_t> StreamSocket::BufferBloatClosedCount = 0;

constexpr std::string_view Socket::toString(Type t)
{
    switch (t)
    {
        case Type::IPv4:
            return "IPv4";
        case Type::IPv6:
            return "IPv6";
        case Type::All:
            return "All";
        case Type::Unix:
            return "Unix";
    }

    return "Unknown";
}

int Socket::createSocket(Socket::Type type)
{
    if (!Util::isMobileApp())
        return net::openStreamSocket(type);

    return fakeSocketSocket();
}

std::ostream& Socket::streamStats(std::ostream& os,
                                  const std::chrono::steady_clock::time_point now) const
{
    const auto durTotal = std::chrono::duration_cast<std::chrono::milliseconds>(now - _creationTime);
    const auto durLast = std::chrono::duration_cast<std::chrono::milliseconds>(now - _lastSeenTime);

    float kBpsIn, kBpsOut;
    if (durTotal.count() > 0)
    {
        kBpsIn = float(_bytesRcvd) / float(durTotal.count());
        kBpsOut = float(_bytesSent) / float(durTotal.count());
    }
    else
    {
        kBpsIn = float(_bytesRcvd) / 1000.0f;
        kBpsOut = float(_bytesSent) / 1000.0f;
    }

    const std::streamsize p = os.precision();
    os.precision(1);
    os << "Stats[dur[total "
        << durTotal.count() << "ms, last "
        << durLast.count() << " ms], kBps[in "
        << kBpsIn << ", out " << kBpsOut
        << "]]";
    os.precision(p);
    return os;
}

std::string Socket::getStatsString(const std::chrono::steady_clock::time_point now) const
{
    std::ostringstream oss;
    streamStats(oss, now);
    return oss.str();
}

std::ostream& Socket::streamImpl(std::ostream& os) const
{
    os << "Socket[#" << getFD() << ", " << toString(type()) << " @ " << clientAddress() << ":"
       << clientPort() << ']';
    return os;
}

std::string Socket::toStringImpl() const
{
    std::ostringstream oss;
    streamImpl(oss);
    return oss.str();
}


#if ENABLE_DEBUG
static std::atomic<long> socketErrorCount;

bool StreamSocket::simulateSocketError(bool read)
{
    if ((socketErrorCount++ % 7) == 0)
    {
        LOGA_TRC(Socket, "Simulating socket error during " << (read ? "read." : "write."));
        errno = EAGAIN;
        return true;
    }

    return false;
}
#endif //ENABLE_DEBUG

#if ENABLE_SSL
static std::string X509_NAME_to_utf8(X509_NAME* name)
{
    BIO* bio = BIO_new(BIO_s_mem());
    X509_NAME_print_ex(bio, name, 0,
                       (ASN1_STRFLGS_RFC2253 | XN_FLAG_SEP_COMMA_PLUS | XN_FLAG_FN_SN |
                        XN_FLAG_DUMP_UNKNOWN_FIELDS) &
                           ~ASN1_STRFLGS_ESC_MSB);
    BUF_MEM* buf;
    BIO_get_mem_ptr(bio, &buf);
    std::string text(buf->data, buf->length);
    BIO_free(bio);
    return text;
}

bool SslStreamSocket::verifyCertificate()
{
    if (_verification == ssl::CertificateVerification::Disabled || isLocalHost())
    {
        return true;
    }

    LOG_TRC("Verifying certificate of [" << hostname() << ']');
    X509* x509 = SSL_get1_peer_certificate(_ssl);
    if (x509)
    {
        // Dump cert info, for debugging only.
        const std::string issuerName = X509_NAME_to_utf8(X509_get_issuer_name(x509));
        const std::string subjectName = X509_NAME_to_utf8(X509_get_subject_name(x509));
        std::string serialNumber;
        BIGNUM* bigNumber = ASN1_INTEGER_to_BN(X509_get_serialNumber(x509), nullptr);
        if (bigNumber)
        {
            char* Sn = BN_bn2hex(bigNumber);
            if (Sn)
            {
                serialNumber = Sn;
                OPENSSL_free(Sn);
            }

            BN_free(bigNumber);
        }

        LOG_TRC("SSL cert issuer: " << issuerName << ", subject: " << subjectName
                                    << ", serial: " << serialNumber);

        Poco::Net::X509Certificate cert(x509);
        if (cert.verify(hostname()))
        {
            LOG_TRC("SSL cert verified for host [" << hostname() << ']');
            return true;
        }

        LOG_INF("SSL cert failed verification for host [" << hostname() << ']');
        return false;
    }

    return false;
}

std::string SslStreamSocket::getSslCert(std::string& subjectHash)
{
    std::ostringstream strstream;
    if (X509* x509 = SSL_get1_peer_certificate(_ssl))
    {
        Poco::Net::X509Certificate cert(x509);
        cert.save(strstream);

        std::stringstream hexstream;
        hexstream << std::setfill('0') << std::setw(8) << std::hex << X509_subject_name_hash(x509);
        subjectHash = hexstream.str();
    }
    return strstream.str();
}
#endif //ENABLE_SSL

// help with initialization AND destruction order.
//
// These are intentionally leaked (heap-allocated, never destroyed): a global or
// static SocketPoll (e.g. a TerminatingPoll) can be torn down during exit-handler
// processing, and SocketPoll::~SocketPoll -> removeFromWakeupArray() touches both
// of these. A plain function-local static would already have been destroyed by
// then, so the access would be a use-after-free (the vector) / UB (locking a
// destroyed mutex). Leaking keeps them alive until the very end; because the
// pointer is held in static storage, LeakSanitizer still sees them as reachable.
namespace {
    std::vector<int> &getWakeupsArray()
    {
        static std::vector<int>* pollWakeups = new std::vector<int>;
        return *pollWakeups;
    }
    std::mutex &getPollWakeupsMutex()
    {
        static std::mutex* pollWakeupsMutex = new std::mutex;
        return *pollWakeupsMutex;
    }
}

SocketPoll::SocketPoll(std::string threadName)
    : _name(std::move(threadName))
    , _pollStartIndex(0)
    , _owner(ProcUtil::getThreadId())
    , _threadStarted(0)
    , _watchdogTime(Watchdog::getDisableStamp())
    , _stop(false)
    , _threadFinished(false)
    , _runOnClientThread(false)
{
    ProfileZone profileZone("SocketPoll::SocketPoll");

    static bool watchDogProfile = !!getenv("COOL_WATCHDOG");
    if (watchDogProfile && !PollWatchdog)
        PollWatchdog = std::make_unique<Watchdog>();

    _wakeup[0] = -1;
    _wakeup[1] = -1;

    createWakeups();

    LOG_DBG("New " << logInfo());

    if (PollWatchdog)
        PollWatchdog->addTime(&_watchdogTime, &_owner);
}

SocketPoll::~SocketPoll()
{
    LOG_DBG("~" << logInfo());

    if (PollWatchdog)
        PollWatchdog->removeTime(&_watchdogTime);

    joinThread();

    removeFromWakeupArray();
}

void SocketPoll::checkAndReThread()
{
    if (ThreadChecks::Inhibit)
        return; // in late shutdown
    const ProcUtil::ThreadId us = ProcUtil::getThreadId();
    if (_owner == us)
        return; // all well
    LOG_DBG("Unusual - SocketPoll used from a new thread");

    _owner = us;
    for (const auto& it : _pollSockets)
        SocketThreadOwnerChange::setThreadOwner(*it, us);
    // _newSockets are adapted as they are inserted.
}

void SocketPoll::removeFromWakeupArray()
{
    if (_wakeup[1] != -1)
    {
        std::lock_guard<std::mutex> lock(getPollWakeupsMutex());
        auto it = std::find(getWakeupsArray().begin(),
                            getWakeupsArray().end(),
                            _wakeup[1]);

        if (it != getWakeupsArray().end())
            getWakeupsArray().erase(it);
    }

    if (!Util::isMobileApp())
    {
        net::closeDescriptor(_wakeup[0]);
        net::closeDescriptor(_wakeup[1]);
    }
    else
    {
        fakeSocketClose(_wakeup[0]);
        fakeSocketClose(_wakeup[1]);
    }

    _wakeup[0] = -1;
    _wakeup[1] = -1;
}

bool SocketPoll::startThread()
{
    assert(!_runOnClientThread);

    // In a race, only the first gets in.
    if (_threadStarted++ == 0)
    {
        _threadFinished = false;
        _stop = false;
        try
        {
            LOG_TRC("Creating thread for SocketPoll " << _name);
            _thread = std::thread(&SocketPoll::pollingThreadEntry, this);
            return true;
        }
        catch (const std::exception& exc)
        {
            LOG_ERR("Failed to start SocketPoll thread [" << _name << "]: " << exc.what());
            _threadStarted = 0;
        }
    }
    else if (isAlive())
    {
        // Most likely a programming error--use isAlive().
        LOG_DBG("SocketPoll [" << _name << "] thread is already running.");
    }
    else
    {
        // This is most likely a programming error.
        // There is no point in starting a new thread either,
        // because the owner is unlikely to recover.
        // If there is a valid use-case for restarting
        // an expired thread, we should add a way to reset it.
        LOG_ASSERT_MSG(!"Expired thread",
                       "SocketPoll [" << _name
                                      << "] thread has ran and finished. Will not start it again");
    }

    return false;
}

void SocketPoll::joinThread()
{
    if (isAlive())
    {
        stop();
    }

    if (_threadStarted && _thread.joinable())
    {
        if (_thread.get_id() == std::this_thread::get_id())
            LOG_ERR("DEADLOCK PREVENTED: joining own thread!");
        else
        {
            _thread.join();
            _threadStarted = 0;
        }
    }

    if (_runOnClientThread)
    {
        removeSockets();
    }

    assert(_pollSockets.empty());
}

void SocketPoll::pollingThreadEntry()
{
    try
    {
        ProcUtil::setThreadName(_name);
        _owner = ProcUtil::getThreadId();
        LOG_INF("Starting polling thread [" << _name << "] with thread affinity set to " << _owner);

        // Invoke the virtual implementation.
        pollingThread();
    }
    catch (const std::exception& exc)
    {
        LOG_ERR("Exception in polling thread [" << _name << "]: " << exc.what());
    }

    // Release sockets.
    removeSockets();

    _threadFinished = true;
    LOG_INF("Finished polling thread [" << _name << "].");
}

void SocketPoll::disableWatchdog()
{
    _watchdogTime = Watchdog::getDisableStamp();
}

void SocketPoll::enableWatchdog()
{
    _watchdogTime = Watchdog::getTimestamp();
}

int SocketPoll::poll(int64_t timeoutMaxMicroS, bool justPoll)
{
    if (_runOnClientThread)
        checkAndReThread();
    else
        ASSERT_CORRECT_SOCKET_THREAD(this);

#if ENABLE_DEBUG
    // perturb - to rotate errors among several busy sockets.
    socketErrorCount++;
#endif

    const std::chrono::steady_clock::time_point now =
        std::chrono::steady_clock::now();

    // The events to poll on change each spin of the loop.
    setupPollFds(now, timeoutMaxMicroS);
    const size_t size = _pollSockets.size();

    // disable watchdog - it's good to sleep
    disableWatchdog();

    int rc;
    do
    {
        LOGA_TRC(Socket, "poll start, timeoutMicroS: " << timeoutMaxMicroS << " size " << size);
        if (!Util::isMobileApp())
            rc = net::pollDescriptors(_pollFds.data(), size + 1, timeoutMaxMicroS);
        else
        {
            const int timeoutMaxMs = (timeoutMaxMicroS + 999) / 1000;
            rc = fakeSocketPoll(_pollFds.data(), size + 1, std::max(timeoutMaxMs, 0));
        }
    }
    while (rc < 0 && errno == EINTR);
    LOGA_TRC(Socket, "Poll completed with " << rc << " live polls max (" <<
             timeoutMaxMicroS << "us)" << ((rc==0) ? "(timedout)" : ""));

    if (rc == 0)
    {
        // We timed out. Flush the thread-local log
        // buffer to avoid falling too much behind.
        Log::flush();
    }

    // from now we want to race back to sleep.
    enableWatchdog();

    if (justPoll)
    {
        // Done with the poll(), don't process anything.
        int ret = 0;
        // Run through the poll entries (except the wakeup poll), and combine them into an answer.
        for (size_t i = 0; i < size; ++i)
        {
            if (_pollFds[i].revents)
            {
                ret = 1;
                break;
            }
        }
        return ret;
    }

    // First process the wakeup pipe (always the last entry).
    if (_pollFds[size].revents)
    {
        LOGA_TRC(Socket, "Handling events of wakeup pipe (" << _pollFds[size].fd << "): 0x"
                                                            << std::hex << _pollFds[size].revents
                                                            << std::dec);

        // Clear the data.
        int dump[32];
        if (!Util::isMobileApp())
            dump[0] = net::readDescriptor(_wakeup[0], &dump, sizeof(dump));
        else
            dump[0] = fakeSocketRead(_wakeup[0], &dump, sizeof(dump));
        LOGA_TRC(Socket, "Wakeup pipe (" << _wakeup[0] << ") read " << dump[0] << " bytes");

        std::vector<CallbackFn> invoke;
        std::vector<SocketTransfer> pendingTransfers;
        {
            std::lock_guard<std::mutex> lock(_mutex);

            if (!_newSockets.empty())
            {
                LOGA_TRC(Socket, "Inserting " << _newSockets.size() << " new sockets after the existing "
                         << _pollSockets.size());

                // Update thread ownership.
                for (auto& i : _newSockets)
                    SocketThreadOwnerChange::setThreadOwner(*i, ProcUtil::getThreadId());

                // Copy the new sockets over and clear.
                _pollSockets.insert(_pollSockets.end(), _newSockets.begin(), _newSockets.end());

                _newSockets.clear();
            }

            // Extract list of callbacks to process
            std::swap(_newCallbacks, invoke);
            std::swap(_pendingTransfers, pendingTransfers);
        }

        if (invoke.size() > 0)
            LOGA_TRC(Socket, "Invoking " << invoke.size() << " callbacks");
        for (const auto& callback : invoke)
        {
            try
            {
                callback();
            }
            catch (const std::exception& exc)
            {
                LOG_ERR("Exception while invoking poll [" << _name <<
                        "] callback: " << exc.what());
            }
        }

        if (pendingTransfers.size() > 0)
            LOGA_TRC(Socket, "Invoking " << pendingTransfers.size() << " transfers");
        for (const auto& pendingTransfer : pendingTransfers)
        {
            try
            {
                transfer(pendingTransfer);
            }
            catch (const std::exception& exc)
            {
                LOG_ERR("Exception while invoking poll [" << _name <<
                        "] transfer: " << exc.what());
            }
        }

        pendingTransfers.clear();
        invoke.clear();

        try
        {
            wakeupHook();
        }
        catch (const std::exception& exc)
        {
            LOG_ERR("Exception while invoking poll [" << _name <<
                    "] wakeup hook: " << exc.what());
        }
    }

    if (_pollSockets.size() != size)
    {
        LOG_TRC("PollSocket container size has changed from " << size << " to "
                                                              << _pollSockets.size());
    }

    // If we had sockets to process.
    if (size > 0)
    {
        assert(!_pollSockets.empty() && "All existing sockets disappeared from the SocketPoll");

        // Fire the poll callbacks and remove dead fds.
        const std::chrono::steady_clock::time_point newNow = std::chrono::steady_clock::now();

        // We use the _pollStartIndex to start the polling at a different index each time. Do some
        // sanity check first to handle the case where we removed one or several sockets last time.
        ++_pollStartIndex;
        if (_pollStartIndex > size - 1)
            _pollStartIndex = 0;

        size_t itemsErased = 0;
        size_t i = _pollStartIndex;
        for (std::size_t j = 0; j < size; ++j)
        {
            if (i >= _pollSockets.size())
            {
                // re-entrancy hazard
                LOG_DBG("Unexpected socket poll resize");
            }
            else if (!_pollSockets[i])
            {
                // removed in a callback
                ++itemsErased;
            }
            else if (_pollFds[i].fd == _pollSockets[i]->getFD())
            {
                SocketDisposition disposition(_pollSockets[i]);
                try
                {
                    LOGA_TRC(Socket, '#' << _pollFds[i].fd << ": Handling poll events of " << _name
                             << " at index " << i << " (of " << size << "): 0x" << std::hex
                             << _pollFds[i].revents << std::dec);

                    _pollSockets[i]->handlePoll(disposition, newNow, _pollFds[i].revents);
                }
                catch (const std::exception& exc)
                {
                    LOG_ERR('#' << _pollFds[i].fd << ": Error while handling poll at " << i
                                << " in " << _name << ": " << exc.what());
                    disposition.setClosed();
                    rc = -1;
                }

                if (!_pollSockets[i]->isOpen() || !disposition.isContinue())
                {
                    ++itemsErased;
                    LOGA_TRC(Socket, '#' << _pollFds[i].fd << ": Removing socket (at " << i
                             << " of " << _pollSockets.size() << ") from " << _name);
                    _pollSockets[i] = nullptr;
                }

                disposition.execute();
            }
            else
            {
                LOG_DBG("Unexpected socket in the wrong position. Expected #"
                        << _pollFds[i].fd << " at index " << i << " but found "
                        << _pollSockets[i]->getFD() << " instead. Skipping");
                assert(!"Unexpected socket at the wrong position");
            }

            // wrap for _pollStartIndex rotation
            if (i == 0)
                i = size - 1;
            else
                i--;
        }

        if (itemsErased)
        {
            LOG_TRC("Scanning to removing " << itemsErased << " defunct sockets from "
                    << _pollSockets.size() << " sockets");

            _pollSockets.erase(
                std::remove_if(_pollSockets.begin(), _pollSockets.end(),
                    [](const std::shared_ptr<Socket>& s)->bool
                    { return !s; }),
                _pollSockets.end());
        }
    }

    return rc;
}

void SocketPoll::transfer(const SocketTransfer& pendingTransfer)
{
    std::shared_ptr<Socket> socket = pendingTransfer._socket.lock();
    std::shared_ptr<SocketPoll> toPoll = pendingTransfer._toPoll.lock();
    if (!socket)
    {
        LOG_WRN("Socket for transfer no longer exists");
        return;
    }
    if (!toPoll)
    {
        LOG_WRN("Destination Poll for socket transfer no longer exists");
        return;
    }
    auto it = std::find(_pollSockets.begin(), _pollSockets.end(), socket);
    if (it == _pollSockets.end())
        LOG_WRN("Trying to move socket out of the wrong poll");
    else
    {
        SocketDisposition disposition(socket);
        disposition.setTransfer(*toPoll, pendingTransfer._cbAfterArrivalInNewPoll);
        // leave empty entry in _pollSockets to be added to toErase and
        // cleaned later.
        *it = nullptr;
        if (!disposition.execute())
        {
            LOG_WRN("Failed to transfer socket #"
                    << socket->getFD() << " to poll [" << toPoll->name()
                    << "]; it has been removed from [" << name() << "] and dropped");
        }

        if (pendingTransfer._cbAfterRemovalFromOldPoll)
            pendingTransfer._cbAfterRemovalFromOldPoll();
    }
}

void SocketPoll::wakeupWorld()
{
    std::lock_guard<std::mutex> lock(getPollWakeupsMutex());
    for (const auto& fd : getWakeupsArray())
        wakeup(fd);
}

// NB. if we just ~Socket we do a shutdown which closes
// the parent copy of the same socket, which is exactly
// what we don't want.
void SocketPoll::closeAllSockets()
{
    // We just forked so we need to shift thread ids to this thread.
    checkAndReThread();

    removeFromWakeupArray();
    for (std::shared_ptr<Socket> &it : _pollSockets)
    {
        // first close the underlying socket/fakeSocket
        it->closeFD(*this);
        assert(!it->isOpen() && "Socket is still open after closing");

        // avoid the socketHandler' getting an onDisconnect
        auto stream = dynamic_cast<StreamSocket *>(it.get());
        if (stream)
            stream->resetHandler();
    }
    // only then remove
    removeSockets();
    assert(_newSockets.empty());
}

void SocketPoll::takeSocket(const std::shared_ptr<SocketPoll>& fromPoll,
                            const std::shared_ptr<SocketPoll>& toPoll,
                            const std::shared_ptr<Socket>& inSocket)
{
    std::mutex mut;
    std::condition_variable cond;
    bool transferred = false;

    // Important we're not blocking the fromPoll thread.
    ASSERT_CORRECT_SOCKET_THREAD(toPoll);

    int socketFD = inSocket->getFD();

    const bool scheduled = fromPoll->transferSocketTo(inSocket, toPoll,
        [](const std::shared_ptr<Socket>& /*moveSocket*/){},
        [&mut,&cond,&transferred,socketFD](){

        LOG_TRC("Socket #" << socketFD << " moved across polls");

        // Let the caller know we've done our job.
        std::unique_lock<std::mutex> lock(mut);
        transferred = true;
        cond.notify_all();
    });

    if (!scheduled)
    {
        // The source poll isn't running, so the transfer was never queued and
        // the removal callback above will never fire; waiting would hang forever.
        LOG_WRN("Failed to transfer Socket #"
                << socketFD << " from poll [" << fromPoll->name() << "] to poll [" << toPoll->name()
                << "] as the source poll isn't alive; socket not moved");
        return;
    }

    LOG_TRC("Waiting to transfer Socket #" << socketFD <<
            " from: " << fromPoll->name() << " to new poll: " << toPoll->name());
    std::unique_lock<std::mutex> lock(mut);
    while (!transferred && toPoll->continuePolling()) // in case of exit during transfer.
        cond.wait_for(lock, std::chrono::milliseconds(50));

    LOG_TRC("Transfer of Socket #" << socketFD <<
            " from: " << fromPoll->name() << " to new poll: " << toPoll->name() << " complete");
}

void SocketPoll::createWakeups()
{
    assert(_wakeup[0] == -1 && _wakeup[1] == -1);

    // Create the wakeup fd.
    if ((!Util::isMobileApp() ? net::createPipe(_wakeup) : fakeSocketPipe2(_wakeup)) == -1)
    {
        throw std::runtime_error("Failed to allocate pipe for SocketPoll [" + _name + "] waking.");
    }

    LOG_DBG("Created wakeup FDs for SocketPoll [" << _name << "], rfd: " << _wakeup[0]
                                                  << ", wfd: " << _wakeup[1]);

    std::lock_guard<std::mutex> lock(getPollWakeupsMutex());
    getWakeupsArray().push_back(_wakeup[1]);
}

void SocketPoll::removeSockets()
{
    LOG_DBG("Removing all " << _pollSockets.size() + _newSockets.size()
                            << " sockets from SocketPoll thread " << _name);
    ASSERT_CORRECT_SOCKET_THREAD(this);

    while (!_pollSockets.empty())
    {
        const std::shared_ptr<Socket>& socket = _pollSockets.back();
        assert(socket);

        LOG_DBG("Removing socket #" << socket->getFD() << " from " << _name);
        ASSERT_CORRECT_SOCKET_THREAD(socket);
        SocketThreadOwnerChange::resetThreadOwner(*socket);

        _pollSockets.pop_back();
    }

    while (!_newSockets.empty())
    {
        const std::shared_ptr<Socket>& socket = _newSockets.back();
        assert(socket);

        LOG_DBG("Removing socket #" << socket->getFD() << " from newSockets of " << _name);

        _newSockets.pop_back();
    }
}

bool SocketPoll::insertNewFakeSocket(
    int peerSocket,
    const std::shared_ptr<ProtocolHandlerInterface>& websocketHandler)
{
    LOG_INF("Connecting to " << peerSocket);
    int fd = fakeSocketSocket();
    int res = fakeSocketConnect(fd, peerSocket);
    if (fd < 0 || (res < 0 && errno != EINPROGRESS))
    {
        LOG_ERR("Failed to connect to the 'wsd' socket");
        fakeSocketClose(fd);
    }
    else
    {
        std::shared_ptr<StreamSocket> socket;
        socket = StreamSocket::create<StreamSocket>(std::string(), fd, Socket::Type::Unix, true,
                                                    HostType::Other, websocketHandler);
        if (socket)
        {
            LOG_TRC("Sending 'hello' instead of HTTP GET for now");
            socket->send("hello");
            insertNewSocket(socket);
            return true;
        }
        else
        {
            LOG_ERR("Failed to allocate socket for client websocket");
            fakeSocketClose(fd);
        }
    }
    return false;
}

void ServerSocket::dumpState(std::ostream& os)
{
    os << '\t' << getFD() << "\t<accept>\n";
}

bool SocketDisposition::execute()
{
    const int socketFd = _socket->getFD();

    if (_disposition != Type::CONTINUE)
        LOG_TRC("Executing SocketDisposition of #" << socketFd << ": " << name(_disposition));

    // We should have hard ownership of this socket.
    ASSERT_CORRECT_SOCKET_THREAD(_socket);
    if (_socketMove)
    {
        assert(_disposition == Type::TRANSFER);

        // Drop pretentions of ownership before _socketMove.
        SocketThreadOwnerChange::resetThreadOwner(*_socket);

        assert (isTransfer() && _toPoll);
        if (!_toPoll->isAlive() && !_toPoll->isThreadStarted())
        {
            // Ensure the thread is running before adding callback.
            LOG_DBG("Starting target poll thread [" << _toPoll->name() << "] while moving socket #"
                                                    << socketFd);
            _toPoll->startThread();
        }

        auto callback = [pollCopy = _toPoll, socket = std::move(_socket),
                         socketMoveFn = std::move(_socketMove)]() mutable
        {
            pollCopy->insertNewSocket(socket);
            socketMoveFn(socket);
            // Clear lambda's socket capture while in the polling thread
            socket.reset();
        };
        _socketMove = nullptr;
        assert(!_socket && "should be unset after move");

        const bool success = _toPoll->addCallback(std::move(callback));
        if (!success)
        {
            // This can happen due to programming error or a race with the thread.
            LOG_WRN("Failed to transfer socket #" << socketFd << " to poll [" << _toPoll->name()
                                                  << "] as the poll isn't alive");
        }

        _toPoll = nullptr;
        return success;
    }
    else
    {
        assert(_disposition != Type::TRANSFER);
    }

    return true;
}

void WebSocketHandler::dumpState(std::ostream& os, const std::string& indent) const
{
    os << (_shuttingDown ? "shutd " : "alive ");
    os << "closeOrigin " << static_cast<int>(_closeOrigin) << ' ';
    os << std::setw(5) << _pingTimeUs/1000. << "ms ";
    if (_closeOnPongTimeout)
        os << "last pong "
           << std::chrono::duration_cast<std::chrono::milliseconds>(
                  std::chrono::steady_clock::now() - _lastPongTime)
           << " ago of "
           << std::chrono::duration_cast<std::chrono::seconds>(net::Defaults.pongTimeout) << ' ';
    if (_wsPayload.size() > 0)
        HexUtil::dumpHex(os, _wsPayload, "\t\tws queued payload:\n", "\t\t");
    os << '\n';
    if (_msgHandler)
    {
        os << indent << "msgHandler:\n";
        _msgHandler->dumpState(os);
    }
}

void StreamSocket::dumpState(std::ostream& os)
{
    int64_t timeoutMaxMicroS = SocketPoll::DefaultPollTimeoutMicroS.count();
    const int events = getPollEvents(std::chrono::steady_clock::now(), timeoutMaxMicroS);

    // The format of the table is as follows (spaces are really tabs):
    // "fd events status rbuffered rcapacity wbuffered wcapacity rtotal wtotal clientaddress";
    os << '\t' << std::setw(6) << getFD() << "\t0x" << std::hex << events << std::dec
       << (ignoringInput() ? "\t\tignore\t" : "\t\tprocess\t") << std::setw(7) << _inBuffer.size()
       << '\t' << std::setw(7) << _inBuffer.capacity() << '\t' << std::setw(6) << _outBuffer.size()
       << '\t' << std::setw(7) << _outBuffer.capacity() << '\t' << " r: " << std::setw(6)
       << bytesRcvd() << "\t w: " << std::setw(6) << bytesSent() << '\t' << clientAddress() << '\t';
    _socketHandler->dumpState(os);
    if (_inBuffer.size() > 0)
        HexUtil::dumpHex(os, _inBuffer, "\t\tinBuffer:\n", "\t\t");
    _outBuffer.dumpHex(os, "\t\toutBuffer:\n", "\t\t");
}

bool StreamSocket::send(const http::Response& response)
{
    if (response.writeData(_outBuffer))
    {
        attemptWrites();
        return true;
    }

    asyncShutdown();
    return false;
}

#if !(defined _WIN32 || defined(MACOS))
// CODA-W/-M build fine without HttpRequest.cpp, which is where the below writeData() is, and also
// without this function.

bool StreamSocket::send(http::Request& request)
{
    if (request.writeData(_outBuffer, getSendBufferCapacity()))
    {
        attemptWrites();
        return true;
    }

    asyncShutdown();
    return false;
}
#endif

bool StreamSocket::sendAndShutdown(http::Response& response)
{
    response.setConnectionToken(http::Header::ConnectionToken::Close);
    if (send(response))
    {
        asyncShutdown();
        return true;
    }

    return false;
}

void SocketPoll::dumpState(std::ostream& os) const
{
    THREAD_UNSAFE_DUMP_BEGIN
    // FIXME: NOT thread-safe! _pollSockets is modified from the polling thread!
    const std::vector<std::shared_ptr<Socket>> pollSockets = _pollSockets;

    os << "\n  SocketPoll [" << name() << "] with " << pollSockets.size() << " socket(s) and "
       << _newCallbacks.size() << " callback(s) - wakeup rfd: " << _wakeup[0]
       << " wfd: " << _wakeup[1] << '\n';

    if (!pollSockets.empty())
    {
        os << "\t\tfd\tevents\tstatus\trbuffered\trcapacity\twbuffered\twcapacity\trtotal\twtotal\t"
              "clientaddress\n";
        std::size_t totalCapacity = 0;
        for (const std::shared_ptr<Socket>& socket : pollSockets)
        {
            socket->dumpState(os);
            totalCapacity += socket->totalBufferCapacity();
        }

        os << "\n  Total socket buffer capacity: " << totalCapacity / 1024 << " KB\n";
    }

    os << "\n  Done SocketPoll [" << name() << "]\n";
    THREAD_UNSAFE_DUMP_END
}

/// Returns true on success only.
bool ServerSocket::bind(Type type, int port)
{
    if (!Util::isMobileApp())
        return net::bindToPort(getFD(), Socket::type(), type == Type::Public, port);

    return true;
}


std::shared_ptr<Socket> ServerSocket::accept()
{
    // Accept a connection (if any) and set it to non-blocking.
    // There still need the client's address to filter request from POST(call from REST) here.
    UnitWSD* const unitWsd = UnitWSD::isUnitTesting() ? &UnitWSD::get() : nullptr;

    net::PeerAddress peer;
    int rc;
    if (!Util::isMobileApp())
    {
        assert(Socket::type() != Socket::Type::Unix);

        if (UNITWSD_CALL_INSTANCE(unitWsd, simulateExternalAcceptError()))
            return nullptr; // Recoverable error, ignore to retry

        rc = net::acceptConnection(getFD(), peer);
    }
    else
        rc = fakeSocketAccept4(getFD());

    if (rc < 0)
        return nullptr;

    LOG_TRC("Accepted socket #" << rc << ", creating socket object.");

    if (Util::isMobileApp())
        return createSocketFromAccept(rc, Socket::Type::Unix);

    const size_t extConnCount = StreamSocket::getExternalConnectionCount();
    if (net::Defaults.maxExtConnections > 0 && extConnCount >= net::Defaults.maxExtConnections)
    {
        LOG_WRN("Limiter rejected extConn[" << extConnCount << "/" << net::Defaults.maxExtConnections << "]: #"
                << rc << " has family "
                << peer.family << ", address " << peer.address << ":" << peer.port);
        net::closeSocketDescriptor(rc);
        return nullptr;
    }

    try
    {
        // Create a socket object using the factory.
        std::shared_ptr<Socket> socket = createSocketFromAccept(rc, peer.type);
        UNITWSD_CALL_INSTANCE(unitWsd, simulateExternalSocketCtorException(socket));

        socket->setClientAddress(peer.address, peer.port);

        LOG_TRC("Accepted socket #" << socket->getFD() << " has family " << peer.family << ", "
                                    << *socket);
        return socket;
    }
    catch (const std::exception& ex)
    {
        LOG_ERR("Failed to create client socket #" << rc << ". Error: " << ex.what());
    }
    return nullptr;
}


std::ostream& StreamSocket::stream(std::ostream& os) const
{
    os << "StreamSocket[#" << getFD()
       << ", " << nameShort(_wsState)
       << ", " << Socket::toString(type())
       << " @ ";
    if (Type::IPv6 == type())
    {
        os << "[" << clientAddress() << "]:" << clientPort();
    }
    else
    {
        os << clientAddress() << ":" << clientPort();
    }
    return os << "]";
}

bool StreamSocket::checkRemoval(std::chrono::steady_clock::time_point now)
{
    if (!isIPType())
        return false;

    // Forced removal on outside-facing IPv{4,6} network connections only.
    const auto durLast =
        std::chrono::duration_cast<std::chrono::milliseconds>(now - getLastSeenTime());

    // Timeout criteria: Violate maximum inactivity (default 3600s).
    const bool isInactive = net::Defaults.inactivityTimeout > std::chrono::microseconds::zero() &&
        durLast > net::Defaults.inactivityTimeout;

    // Timeout criteria: Shall terminate?
    const bool isTermination = SigUtil::getTerminationFlag();
    if (isInactive || isTermination)
    {
        LOG_WRN("CheckRemoval: Timeout: {Inactive " << isInactive << ", Termination "
                                                    << isTermination << "}, " << getStatsString(now)
                                                    << ", " << *this);
        ensureDisconnected();
        if (!isShutdown())
        {
            asyncShutdown(); // signal
            shutdownConnection(); // real -> setShutdown()
        }

        assert(isShutdown() && "Should have issued shutdown");
        assert(!isOpen() && "Socket is still open after closing");
        return true;
    }

    return false;
}


#ifndef _WIN32
namespace {
    /// To make the protected 'computeAccept' accessible.
    class PublicComputeAccept final : public Poco::Net::WebSocket
    {
    public:
        static std::string doComputeAccept(const std::string &key)
        {
            return computeAccept(key);
        }
    };
}

std::string WebSocketHandler::computeAccept(const std::string &key)
{
    return PublicComputeAccept::doComputeAccept(key);
}
#endif // !_WIN32

std::string WebSocketHandler::generateKey()
{
    auto random = Util::rng::getBytes(16);
    return macaron::Base64::Encode(std::string_view(random.data(), random.size()));
}

void SocketPoll::stop()
{
    LOG_DBG("Stopping " << logInfo());
    _stop = true;
    if (!Util::isMobileApp())
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

int SocketPoll::pollUntilDeadline(std::chrono::steady_clock::time_point deadline)
{
    static constexpr auto zero = std::chrono::microseconds::zero();
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

void SocketPoll::wakeup(int fd)
{
    // wakeup the main-loop.
    int rc;
    do {
        if (Util::isMobileApp())
            rc = fakeSocketWrite(fd, "w", 1);
        else
            rc = net::writeDescriptor(fd, "w", 1);
    } while (rc == -1 && errno == EINTR);

    if (rc == -1 && errno != EAGAIN && errno != EWOULDBLOCK)
        LOG_SYS("wakeup socket #" << fd << " is closed at wakeup?");
}

void SocketPoll::wakeup() const
{
    // There is a race when shutting down because
    // SocketPoll threads exit when shutting down.
    if (!isAlive() && !SigUtil::getShutdownRequestFlag())
        LOG_DBG("WARNING: Waking up dead poll thread ["
                << _name << "], started: " << (_threadStarted ? "true" : "false")
                << ", finished: " << _threadFinished);

    wakeup(_wakeup[1]);
}

bool SocketPoll::insertNewSocket(std::shared_ptr<Socket> newSocket)
{
    if (newSocket)
    {
        LOG_TRC("Inserting socket #" << newSocket->getFD() << ", address ["
                                     << newSocket->clientAddress() << "], into " << _name);
        // sockets in transit are un-owned.
        SocketThreadOwnerChange::resetThreadOwner(*newSocket);

        bool wasEmpty = false;
        const bool alive = isAlive();
        if (alive)
        {
            std::lock_guard<std::mutex> lock(_mutex);
            wasEmpty = taskQueuesEmpty();
            _newSockets.emplace_back(std::move(newSocket));
        }

        if (wasEmpty)
            wakeup();

        return alive;
    }

    return false;
}

bool SocketPoll::transferSocketTo(const std::weak_ptr<Socket>& socket,
                                  const std::weak_ptr<SocketPoll>& toPoll,
                                  SocketDisposition::MoveFunction cbAfterArrivalInNewPoll,
                                  std::function<void()> cbAfterRemovalFromOldPoll)
{
    bool wasEmpty = false;
    const bool alive = isAlive();
    if (alive)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        wasEmpty = taskQueuesEmpty();
        _pendingTransfers.emplace_back(socket, toPoll, std::move(cbAfterArrivalInNewPoll),
                                       std::move(cbAfterRemovalFromOldPoll));
    }

    if (wasEmpty)
        wakeup();

    return alive;
}

bool SocketPoll::addCallback(CallbackFn fn)
{
    bool wasEmpty = false;
    const bool alive = isAlive();
    if (alive)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        wasEmpty = taskQueuesEmpty();
        _newCallbacks.emplace_back(std::move(fn));
    }

    if (wasEmpty)
        wakeup();

    return alive;
}

bool SocketPoll::runOnClientThread()
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

void SocketPoll::setupPollFds(std::chrono::steady_clock::time_point now, int64_t& timeoutMaxMicroS)
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

StreamSocket::~StreamSocket()
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

void StreamSocket::ensureDisconnected()
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

void StreamSocket::send(const char* data, const int len, const bool doFlush)
{
    ASSERT_CORRECT_SOCKET_THREAD(this);
    if (data != nullptr && len > 0)
    {
        _outBuffer.append(data, len);
        if (doFlush)
            writeOutgoingData();
    }
}

void StreamSocket::sendFDs([[maybe_unused]] const char* data, [[maybe_unused]] const uint64_t len,
                           [[maybe_unused]] const std::vector<int>& fds)
{
#ifndef _WIN32
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
    cmsghdr *cmsg = reinterpret_cast<cmsghdr*>(adata);
    cmsg->cmsg_type = SCM_RIGHTS;
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_len = CMSG_LEN(fds_size);
    int* fdsField = reinterpret_cast<int *>(CMSG_DATA(cmsg));
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
#endif
}

int StreamSocket::readIncomingData()
{
    ASSERT_CORRECT_SOCKET_THREAD(this);

    if (ignoringInput())
    {
        LOG_WRN("Ignoring attempted read from " << getFD());
        return 0; // error - close it.
    }

    ssize_t len = 0;
    if (!Util::isMobileApp())
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
#ifdef LOG_SOCKET_DATA
                    LOGA_TRC(Socket,
                             "Read " << len << " bytes in addition to " << _inBuffer.size()
                                     << " buffered bytes"
                                     << (len ? HexUtil::dumpHex(std::string(buf, len), ":\n")
                                             : std::string())
                    );
#else
                    LOGA_TRC(Socket,
                             "Read " << len << " bytes in addition to " << _inBuffer.size()
                                     << " buffered bytes"
                    );
#endif
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
            _inBuffer.append(reinterpret_cast<const char*>(&len), sizeof(ssize_t));
            _inBuffer.append(buf.data(), len);
        }
    }

    return len;
}

void StreamSocket::handlePoll(SocketDisposition& disposition,
                              std::chrono::steady_clock::time_point now, const int events)
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

    // Warn as the outgoing buffer crosses the large threshold, once per
    // crossing, so a peer that has stopped reading and is making data pile
    // up on our side is visible in the logs.
    if (_outBuffer.size() > LargeBufferWarnSize)
    {
        if (!_warnedLargeOutBuffer)
        {
            LOG_WRN("Outgoing buffer for socket #" << getFD() << " has grown to "
                    << _outBuffer.size() << " bytes; the peer appears to be reading slowly.");
            _warnedLargeOutBuffer = true;
        }
    }
    else
        _warnedLargeOutBuffer = false;

    if (checkBufferBloat(now))
    {
        LOG_WRN("Socket #" << getFD() << " has kept over " << BufferBloatCloseSize
                << " bytes buffered (in: " << _inBuffer.size() << ", out: "
                << _outBuffer.size() << ") for more than "
                << std::chrono::duration_cast<std::chrono::seconds>(BufferBloatCloseDuration).count()
                << " seconds; closing the connection to reclaim the memory.");
        ++BufferBloatClosedCount;
        ensureDisconnected();
        setShutdown();
        disposition.setClosed();
        return;
    }

    if (!events && _inBuffer.empty())
        return;

    setLastSeenTime(now);

    bool closed = (events & (POLLHUP | POLLERR | POLLNVAL));
    bool unreadAfterHangup = false;

    if (events & POLLIN)
    {
        // readIncomingData returns 0 only if the read len is 0 (closed).
        // Oddly enough, we don't necessarily get POLLHUP after read(2) returns 0.
        const int read = readIncomingData();
        const int last_errno = errno;
#ifdef LOG_SOCKET_DATA
        LOGA_TRC(Socket, "Incoming data buffer "
                             << _inBuffer.size() << " bytes, read result: " << read
                             << ", events: 0x" << std::hex << events << std::dec << " ("
                             << (closed ? "closed" : "not closed") << ')'
                             << (!_inBuffer.empty() ? HexUtil::dumpHex(_inBuffer, ":\n")
                                                    : std::string())
        );
#else
        LOGA_TRC(Socket, "Incoming data buffer "
                             << _inBuffer.size() << " bytes, read result: " << read
                             << ", events: 0x" << std::hex << events << std::dec << " ("
                             << (closed ? "closed" : "not closed") << ')'
        );
#endif

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
            unreadAfterHangup = true;
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
                    // Input still unread is handled on the next poll, which then closes.
                    closed = !unreadAfterHangup;
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

int StreamSocket::writeOutgoingData()
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
            const int size = std::min(int(_outBuffer.getBlockSize()), getSendBufferSize());
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
#ifdef LOG_SOCKET_DATA
                LOGA_TRC(Socket,
                         "Wrote "
                             << len << " bytes of " << _outBuffer.size() << " buffered data"
                             << (len ? HexUtil::dumpHex(std::string(_outBuffer.getBlock(), len),
                                                        ":\n")
                                     : std::string())
                );
#else
                LOGA_TRC(Socket,
                         "Wrote "
                             << len << " bytes of " << _outBuffer.size() << " buffered data"
                );
#endif
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

int StreamSocket::readFDs([[maybe_unused]] char* buf, [[maybe_unused]] int len,
                          [[maybe_unused]] std::vector<int>& fds)
{
#ifdef _WIN32
    return -1;
#else
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
            int* fdsField = reinterpret_cast<int*>(CMSG_DATA(cmsg));
            fds.assign(fdsField, fdsField + fds_count);
            if (_readType == ReadType::UseRecvmsgExpectFD)
            {
                _readType = ReadType::NormalRead;
            }
        }
    }

    return ret;
#endif
}

int StreamSocket::readData(char* buf, int len)
{
    ASSERT_CORRECT_SOCKET_THREAD(this);
    assert((getFD() >= 0 || isShutdown()) && "Socket is closed but not marked correctly");

    // avoided in readIncomingData
    if (ignoringInput())
        return -1;

    if (Util::isMobileApp())
        return fakeSocketRead(getFD(), buf, len);

#ifdef _WIN32
    return -1;
#else
    if (_readType == ReadType::UseRecvmsgExpectFD)
        return readFDs(buf, len, _incomingFDs);

#if ENABLE_DEBUG
    if (simulateSocketError(true))
        return -1;
#endif

    return ::read(getFD(), buf, len);
#endif
}

int StreamSocket::writeData(const char* buf, const int len)
{
    ASSERT_CORRECT_SOCKET_THREAD(this);
    assert((getFD() >= 0 || isShutdown()) && "Socket is closed but not marked correctly");

    if (Util::isMobileApp())
        return fakeSocketWrite(getFD(), buf, len);

#ifdef _WIN32
    return -1;
#else
#if ENABLE_DEBUG
    if (simulateSocketError(false))
        return -1;
#endif
    return ::write(getFD(), buf, len);
#endif
}

bool StreamSocket::checkBufferBloat(std::chrono::steady_clock::time_point now)
{
    const size_t largestBuffer = std::max(_inBuffer.size(), _outBuffer.size());
    if (largestBuffer <= BufferBloatCloseSize)
    {
        // Reset clock, we're good again
        _largeBufferSince = std::chrono::steady_clock::time_point();
        return false;
    }

    if (_largeBufferSince == std::chrono::steady_clock::time_point())
    {
        // First sample over the threshold: start the clock.
        _largeBufferSince = now;
        return false;
    }

    return now - _largeBufferSince >= BufferBloatCloseDuration;
}

// Required by Android and iOS apps.
namespace http
{
std::string getAgentString() { return "COOLWSD HTTP Agent " COOLWSD_VERSION; }

std::string getServerString()
{
    CONFIG_STATIC const bool sig = ConfigUtil::getBool("security.server_signature", false);
    if (sig)
        return "COOLWSD HTTP Server " COOLWSD_VERSION;

    return " ";
}
}

extern "C" {
    void handleUserProfileSignal(const int /* signal */)
    {
#if defined(__linux__) && !defined(__ANDROID__) && defined(SYS_futimesat)
        const struct timeval times[2] = {};
        // call something fairly obscure that perf can trigger on.  futimesat
        // look a good candidate (calling "futimesat" typically results in
        // using syscall SYS_utimensat so use SYS_futimesat directly).
        syscall(SYS_futimesat, -1, "/tmp/kit-watchdog", times);
#endif
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
