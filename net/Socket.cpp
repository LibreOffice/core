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
#if !MOBILEAPP
#include <common/Watchdog.hpp>
#endif
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
#if !MOBILEAPP
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/NetException.h>
#include <Poco/Net/WebSocket.h> // computeAccept
#endif

#include <Poco/URI.h>

#if ENABLE_SSL
#include <Poco/Net/X509Certificate.h>
#endif

// Bug in pre C++17 where static constexpr must be defined. Fixed in C++17.
constexpr std::chrono::microseconds SocketPoll::DefaultPollTimeoutMicroS;
constexpr std::chrono::microseconds WebSocketHandler::InitialPingDelayMicroS;

#if !MOBILEAPP

std::unique_ptr<Watchdog> SocketPoll::PollWatchdog;

#ifndef __APPLE__
#define SOCKET_ABSTRACT_UNIX_NAME "coolwsd-"
#else
#define SOCKET_ABSTRACT_UNIX_NAME "/tmp/coolwsd-"
#endif

#endif

std::atomic<size_t> StreamSocket::ExternalConnectionCount = 0;

net::DefaultValues net::Defaults = { .inactivityTimeout = std::chrono::seconds(3600),
                                     .maxExtConnections = 200000 /* arbitrary value to be resolved */ };

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

int Socket::createSocket([[maybe_unused]] Socket::Type type)
{
#if !MOBILEAPP
    int domain = AF_UNSPEC;
    switch (type)
    {
    case Type::IPv4: domain = AF_INET;  break;
    case Type::IPv6: domain = AF_INET6; break;
    case Type::All:  domain = AF_INET6; break;
    case Type::Unix: domain = AF_UNIX;  break;
    default: assert(!"Unknown Socket::Type"); break;
    }

    return Syscall::socket_cloexec_nonblock(domain, SOCK_STREAM /*| SOCK_NONBLOCK | SOCK_CLOEXEC*/, 0);
#else
    return fakeSocketSocket();
#endif
}

std::ostream& Socket::streamStats(std::ostream& os,
                                  const std::chrono::steady_clock::time_point now) const
{
    const auto durTotal = std::chrono::duration_cast<std::chrono::milliseconds>(now - _creationTime);
    const auto durLast = std::chrono::duration_cast<std::chrono::milliseconds>(now - _lastSeenTime);

    float kBpsIn, kBpsOut;
    if (durTotal.count() > 0)
    {
        kBpsIn = (float)_bytesRcvd / (float)durTotal.count();
        kBpsOut = (float)_bytesSent / (float)durTotal.count();
    }
    else
    {
        kBpsIn = (float)_bytesRcvd / 1000.0f;
        kBpsOut = (float)_bytesSent / 1000.0f;
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

#if !MOBILEAPP

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

#endif

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
    std::string text = std::string(buf->data, buf->length);
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
        BIGNUM* bigNumber = ASN1_INTEGER_to_BN(X509_get_serialNumber(const_cast<X509*>(x509)), 0);
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

// help with initialization order
namespace {
    std::vector<int> &getWakeupsArray()
    {
        static std::vector<int> pollWakeups;
        return pollWakeups;
    }
    std::mutex &getPollWakeupsMutex()
    {
        static std::mutex pollWakeupsMutex;
        return pollWakeupsMutex;
    }
}

SocketPoll::SocketPoll(std::string threadName)
    : _name(std::move(threadName))
    , _pollStartIndex(0)
    , _owner(ProcUtil::getThreadId())
    , _threadStarted(0)
#if !MOBILEAPP
    , _watchdogTime(Watchdog::getDisableStamp())
#endif
    , _stop(false)
    , _threadFinished(false)
    , _runOnClientThread(false)
{
    ProfileZone profileZone("SocketPoll::SocketPoll");

#if !MOBILEAPP
    static bool watchDogProfile = !!getenv("COOL_WATCHDOG");
    if (watchDogProfile && !PollWatchdog)
        PollWatchdog = std::make_unique<Watchdog>();
#endif

    _wakeup[0] = -1;
    _wakeup[1] = -1;

    createWakeups();

    LOG_DBG("New " << logInfo());

#if !MOBILEAPP
    if (PollWatchdog)
        PollWatchdog->addTime(&_watchdogTime, &_owner);
#endif
}

SocketPoll::~SocketPoll()
{
    LOG_DBG("~" << logInfo());

#if !MOBILEAPP
    if (PollWatchdog)
        PollWatchdog->removeTime(&_watchdogTime);
#endif

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

#if !MOBILEAPP
    ::close(_wakeup[0]);
    ::close(_wakeup[1]);
#else
    fakeSocketClose(_wakeup[0]);
    fakeSocketClose(_wakeup[1]);
#endif

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
#if !MOBILEAPP
    _watchdogTime = Watchdog::getDisableStamp();
#endif
}

void SocketPoll::enableWatchdog()
{
#if !MOBILEAPP
    _watchdogTime = Watchdog::getTimestamp();
#endif
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
#if !MOBILEAPP
#  if HAVE_PPOLL
        LOGA_TRC(Socket, "ppoll start, timeoutMicroS: " << timeoutMaxMicroS << " size " << size);
        timeoutMaxMicroS = std::max(timeoutMaxMicroS, (int64_t)0);
        struct timespec timeout;
        timeout.tv_sec = timeoutMaxMicroS / (1000 * 1000);
        timeout.tv_nsec = (timeoutMaxMicroS % (1000 * 1000)) * 1000;
        rc = ::ppoll(_pollFds.data(), size + 1, &timeout, nullptr);
#  else
        int timeoutMaxMs = (timeoutMaxMicroS + 999) / 1000;
        LOG_TRC("Legacy Poll start, timeoutMs: " << timeoutMaxMs);
        rc = ::poll(_pollFds.data(), size + 1, std::max(timeoutMaxMs,0));
#  endif
#else
        LOG_TRC("SocketPoll Poll");
        int timeoutMaxMs = (timeoutMaxMicroS + 999) / 1000;
        rc = fakeSocketPoll(_pollFds.data(), size + 1, std::max(timeoutMaxMs,0));
#endif
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
        bool ret = false;
        // Run through the poll entries (except the wakeup poll), and combine them into an answer.
        for (size_t i = 0; i < size; ++i)
        {
            if (_pollFds[i].revents)
            {
                ret = true;
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
#if !MOBILEAPP
        dump[0] = ::read(_wakeup[0], &dump, sizeof(dump));
#else
        dump[0] = fakeSocketRead(_wakeup[0], &dump, sizeof(dump));
#endif
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
        disposition.execute();
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

    fromPoll->transferSocketTo(inSocket, toPoll,
        [](const std::shared_ptr<Socket>& /*moveSocket*/){},
        [&mut,&cond,&transferred,socketFD](){

        LOG_TRC("Socket #" << socketFD << " moved across polls");

        // Let the caller know we've done our job.
        std::unique_lock<std::mutex> lock(mut);
        transferred = true;
        cond.notify_all();
    });

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
    if (
#if !MOBILEAPP
        Syscall::pipe2(_wakeup, O_CLOEXEC | O_NONBLOCK) == -1
#else
        fakeSocketPipe2(_wakeup) == -1
#endif
        )
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

#if !MOBILEAPP

void SocketPoll::insertNewWebSocketSync(const Poco::URI& uri,
                                        const std::shared_ptr<WebSocketHandler>& websocketHandler)
{
    LOG_TRC("Connecting WS to " << uri.getHost());

    const bool isSSL = uri.getScheme() != "ws";
#if !ENABLE_SSL
    if (isSSL)
    {
        LOG_ERR("Error: wss for client websocket requested but SSL not compiled in.");
        return;
    }
#endif

    http::Request req(uri.getPathAndQuery());
    req.set("User-Foo", "Adminbits");
    //FIXME: Why do we need the following here?
    req.set("Accept-Language", "en");
    req.set("Cache-Control", "no-cache");
    req.set("Pragma", "no-cache");

    const std::string port = std::to_string(uri.getPort());
    if (websocketHandler->wsRequest(req, uri.getHost(), port, isSSL, *this))
    {
        LOG_DBG("Connected WS to " << uri.getHost());
    }
    else
    {
        LOG_ERR("Failed to connected WS to " << uri.getHost());
    }
}

bool SocketPoll::insertNewUnixSocket(
    const UnxSocketPath &location,
    const std::string &pathAndQuery,
    const std::shared_ptr<WebSocketHandler>& websocketHandler,
    const std::vector<int>* shareFDs)
{
    LOG_DBG("Connecting to local UDS " << location);
    const int fd = Syscall::socket_cloexec_nonblock(AF_UNIX, SOCK_STREAM /*| SOCK_NONBLOCK | SOCK_CLOEXEC*/, 0);
    if (fd < 0)
    {
        LOG_SYS("Failed to connect to unix socket at " << location);
        return false;
    }

    struct sockaddr_un addrunix;
    std::memset(&addrunix, 0, sizeof(addrunix));
    addrunix.sun_family = AF_UNIX;
    location.fillInto(addrunix);

    const int res = connect(fd, (const struct sockaddr*)&addrunix, sizeof(addrunix));
    if (res < 0 && errno != EINPROGRESS)
    {
        LOG_SYS("Failed to connect to unix socket at " << location);
        ::close(fd);
        return false;
    }

    std::shared_ptr<StreamSocket> socket
        = StreamSocket::create<StreamSocket>(std::string(), fd, Socket::Type::Unix,
                                             true, HostType::Other, websocketHandler);
    if (!socket)
    {
        LOG_ERR("Failed to create socket unix socket at " << location);
        return false;
    }

    LOG_DBG("Connected to local UDS " << location << " #" << socket->getFD());

    http::Request req(pathAndQuery);
    req.set("User-Foo", "Adminbits");
    req.set("Sec-WebSocket-Key", websocketHandler->getWebSocketKey());
    req.set("Sec-WebSocket-Version", "13");
    //FIXME: Why do we need the following here?
    req.set("Accept-Language", "en");
    req.set("Cache-Control", "no-cache");
    req.set("Pragma", "no-cache");

    LOG_TRC("Requesting upgrade of websocket at path " << pathAndQuery << " #" << socket->getFD());
    if (!shareFDs || shareFDs->empty())
    {
        socket->send(req);
    }
    else
    {
        Buffer buf;
        req.writeData(buf, INT_MAX); // Write the whole request.
        socket->sendFDs(buf.getBlock(), buf.getBlockSize(), *shareFDs);
    }

    std::static_pointer_cast<ProtocolHandlerInterface>(websocketHandler)->onConnect(socket);
    insertNewSocket(socket);

    // We send lots of data back via this local UDS'
    socket->setSocketBufferSize(Socket::MaximumSendBufferSize);

    return true;
}

#else

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
#endif

void ServerSocket::dumpState(std::ostream& os)
{
    os << '\t' << getFD() << "\t<accept>\n";
}

void SocketDisposition::execute()
{
    if (_disposition != Type::CONTINUE)
        LOG_TRC("Executing SocketDisposition of #" << _socket->getFD() <<
                ": " << name(_disposition));

    // We should have hard ownership of this socket.
    ASSERT_CORRECT_SOCKET_THREAD(_socket);
    if (_socketMove)
    {
        assert(_disposition == Type::TRANSFER);

        // Drop pretentions of ownership before _socketMove.
        SocketThreadOwnerChange::resetThreadOwner(*_socket);

        assert (isTransfer() && _toPoll);
        if (!_toPoll->isAlive())
        {
            // Ensure the thread is running before adding callback.
            LOG_DBG("Starting target poll thread [" << _toPoll->name() << "] while moving socket #"
                                                    << _socket->getFD());
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

        _toPoll->addCallback(std::move(callback));

        // This can happen due to programming error or a race with the thread.
        if (!_toPoll->isAlive())
            LOG_WRN("Thread poll [" << _toPoll->name()
                                    << "] is not alive after adding transfer callback");

        _toPoll = nullptr;
    }
    else
    {
        assert(_disposition != Type::TRANSFER);
    }
}

void WebSocketHandler::dumpState(std::ostream& os, const std::string& indent) const
{
    os << (_shuttingDown ? "shutd " : "alive ");
#if !MOBILEAPP
    os << std::setw(5) << _pingTimeUs/1000. << "ms ";
#endif
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

#if !(defined QTAPP || defined _WIN32 || defined(MACOS))
// CODA-Q/-W/-M build fine without HttpRequest.cpp, which is where the below writeData() is, and also
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

    os << "\n  SocketPoll [" << name() << "] with " << pollSockets.size() << " socket(s)" << " and "
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
bool ServerSocket::bind([[maybe_unused]] Type type, [[maybe_unused]] int port)
{
#if !MOBILEAPP
    // Enable address reuse to avoid stalling after
    // recycling, when previous socket is TIME_WAIT.
    //TODO: Might be worth refactoring out.
    const int reuseAddress = 1;
    constexpr unsigned int len = sizeof(reuseAddress);
    if (::setsockopt(getFD(), SOL_SOCKET, SO_REUSEADDR, &reuseAddress, len) == -1)
        LOG_SYS("Failed setsockopt SO_REUSEADDR on socket fd " << getFD() << ": " << strerror(errno));

    int rc;

    assert (_type != Socket::Type::Unix);
    if (_type == Socket::Type::IPv4)
    {
        struct sockaddr_in addrv4;
        std::memset(&addrv4, 0, sizeof(addrv4));
        addrv4.sin_family = AF_INET;
        addrv4.sin_port = htons(port);
        if (type == Type::Public)
            addrv4.sin_addr.s_addr = htonl(INADDR_ANY);
        else
            addrv4.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

        rc = ::bind(getFD(), (const sockaddr *)&addrv4, sizeof(addrv4));
    }
    else
    {
        struct sockaddr_in6 addrv6;
        std::memset(&addrv6, 0, sizeof(addrv6));
        addrv6.sin6_family = AF_INET6;
        addrv6.sin6_port = htons(port);
        if (type == Type::Public)
            addrv6.sin6_addr = in6addr_any;
        else
            addrv6.sin6_addr = in6addr_loopback;

        const int ipv6only = (_type == Socket::Type::All ? 0 : 1);
        if (::setsockopt(getFD(), IPPROTO_IPV6, IPV6_V6ONLY, (char*)&ipv6only, sizeof(ipv6only)) == -1)
            LOG_SYS("Failed set ipv6 socket to " << ipv6only);

        rc = ::bind(getFD(), (const sockaddr *)&addrv6, sizeof(addrv6));
    }

    if (rc)
        LOG_SYS("Failed to bind to: " << (_type == Socket::Type::IPv4 ? "IPv4" : "IPv6")
                                      << " port: " << port);
    else
        LOG_TRC("Bind to: " << (_type == Socket::Type::IPv4 ? "IPv4" : "IPv6")
                            << " port: " << port);

    return rc == 0;
#else
    return true;
#endif
}

#if !MOBILEAPP

bool ServerSocket::isUnrecoverableAcceptError(const int cause) const
{
    constexpr const char * messagePrefix = "Failed to accept. (errno: ";
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

#endif

std::shared_ptr<Socket> ServerSocket::accept()
{
    // Accept a connection (if any) and set it to non-blocking.
    // There still need the client's address to filter request from POST(call from REST) here.
#if !MOBILEAPP
    assert(_type != Socket::Type::Unix);

    UnitWSD* const unitWsd = UnitWSD::isUnitTesting() ? &UnitWSD::get() : nullptr;
    if (UNITWSD_CALL_INSTANCE(unitWsd, simulateExternalAcceptError()))
        return nullptr; // Recoverable error, ignore to retry

    struct sockaddr_in6 clientInfo;
    socklen_t addrlen = sizeof(clientInfo);
    const int rc = Syscall::accept_cloexec_nonblock(getFD(), (struct sockaddr *)&clientInfo, &addrlen);
    if (rc < 0)
    {
        if (isUnrecoverableAcceptError(errno))
            Util::forcedExit(EX_SOFTWARE);
        return nullptr;
    }
#else
    const int rc = fakeSocketAccept4(getFD());
#endif
    LOG_TRC("Accepted socket #" << rc << ", creating socket object.");

#if !MOBILEAPP
    char addrstr[INET6_ADDRSTRLEN];

    Socket::Type type;
    const void *inAddr;
    if (clientInfo.sin6_family == AF_INET)
    {
        struct sockaddr_in *ipv4 = (struct sockaddr_in *)&clientInfo;
        inAddr = &(ipv4->sin_addr);
        type = Socket::Type::IPv4;
    }
    else
    {
        struct sockaddr_in6 *ipv6 = &clientInfo;
        inAddr = &(ipv6->sin6_addr);
        type = Socket::Type::IPv6;
    }
    ::inet_ntop(clientInfo.sin6_family, inAddr, addrstr, sizeof(addrstr));

    const size_t extConnCount = StreamSocket::getExternalConnectionCount();
    if (net::Defaults.maxExtConnections > 0 && extConnCount >= net::Defaults.maxExtConnections)
    {
        LOG_WRN("Limiter rejected extConn[" << extConnCount << "/" << net::Defaults.maxExtConnections << "]: #"
                << rc << " has family "
                << clientInfo.sin6_family << ", address " << addrstr << ":" << clientInfo.sin6_port);
        ::close(rc);
        return nullptr;
    }

    try
    {
        // Create a socket object using the factory.
        std::shared_ptr<Socket> socket = createSocketFromAccept(rc, type);
        UNITWSD_CALL_INSTANCE(unitWsd, simulateExternalSocketCtorException(socket));

        socket->setClientAddress(addrstr, clientInfo.sin6_port);

        LOG_TRC("Accepted socket #" << socket->getFD() << " has family " << clientInfo.sin6_family
                                    << ", " << *socket);
        return socket;
    }
    catch (const std::exception& ex)
    {
        LOG_ERR("Failed to create client socket #" << rc << ". Error: " << ex.what());
    }
    return nullptr;
#else
    return createSocketFromAccept(rc, Socket::Type::Unix);
#endif
}

#if !MOBILEAPP

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

        rc = ::bind(getFD(), (const sockaddr *)&addrunix, sizeof(struct sockaddr_un));
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

bool LocalServerSocket::linkTo([[maybe_unused]] std::string toPath)
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

#endif // !MOBILEAPP

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

#if !MOBILEAPP

ssize_t StreamSocket::readHeader(const std::string_view clientName, std::istream& message,
                                 size_t messagesize,
                                 Poco::Net::HTTPRequest& request,
                                 std::chrono::duration<float, std::milli> delayMs)
{
    constexpr std::chrono::duration<float, std::milli> delayMax =
        std::chrono::duration_cast<std::chrono::milliseconds>(SocketPoll::DefaultPollTimeoutMicroS);

    // Find the end of the header, if any.
    constexpr std::string_view marker("\r\n\r\n");
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
        return false;
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

namespace {
    /// To make the protected 'computeAccept' accessible.
    class PublicComputeAccept final : public Poco::Net::WebSocket
    {
    public:
        static std::string doComputeAccept(const std::string &key)
        {
            return computeAccept(key);
        }

        static std::string generateKey()
        {
            auto random = Util::rng::getBytes(16);
            return macaron::Base64::Encode(std::string_view(random.data(), random.size()));
        }
    };
}

std::string WebSocketHandler::computeAccept(const std::string &key)
{
    return PublicComputeAccept::doComputeAccept(key);
}

std::string WebSocketHandler::generateKey()
{
    return PublicComputeAccept::generateKey();
}

#endif // !MOBILEAPP

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
