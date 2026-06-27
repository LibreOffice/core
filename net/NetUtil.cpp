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
 * Implementation of network utilities: host resolution, connection, SSL setup.
 * Classes: net::HostEntry
 * Functions: connect(), resolve(), localhostIPv4/IPv6()
 */

#include <config.h>

#include <common/NumUtil.hpp>
#include <common/Syscall.hpp>
#include <common/Unit.hpp>
#include <common/Util.hpp>
#include <net/AsyncDNS.hpp>
#include <net/NetUtil.hpp>
#include <net/Socket.hpp>
#include <net/Uri.hpp>
#if ENABLE_SSL && !MOBILEAPP
#include <net/SslSocket.hpp>
#endif

#include <Poco/Exception.h>
#include <Poco/Net/DNS.h>
#include <Poco/Net/NetException.h>
#include <Poco/Net/NetworkInterface.h>

#include <netdb.h>

#include <Poco/Net/SocketAddress.h>

namespace net
{

std::string HostEntry::makeIPAddress(const sockaddr* ai_addr)
{
    char addrstr[INET6_ADDRSTRLEN];

    static_assert(INET6_ADDRSTRLEN >= INET_ADDRSTRLEN, "ipv6 addresses are longer than ipv4");

    const void *inAddr = nullptr;
    switch (ai_addr->sa_family)
    {
        case AF_INET:
        {
            auto ipv4 = reinterpret_cast<const sockaddr_in*>(ai_addr);
            inAddr = &(ipv4->sin_addr);
            break;
        }
        case AF_INET6:
        {
            auto ipv6 = reinterpret_cast<const sockaddr_in6*>(ai_addr);
            inAddr = &(ipv6->sin6_addr);
            break;
        }
    }

    if (!inAddr)
    {
        LOG_ERR("Unknown sa_family: " << ai_addr->sa_family);
        return std::string();
    }

    const char* result = inet_ntop(ai_addr->sa_family, inAddr, addrstr, sizeof(addrstr));
    if (!result)
    {
        _saved_errno = errno;
        LOG_WRN("inet_ntop failure: " << errorMessage());
        return std::string();
    }
    return std::string(result);
}

void HostEntry::setEAI(int eaino)
{
    _eaino = eaino;
    // EAI_SYSTEM: Other system error; errno is set to indicate the error.
    if (_eaino == EAI_SYSTEM)
        _saved_errno = errno;
}

std::string HostEntry::errorMessage() const
{
    const char* errmsg;
    if (_eaino && _eaino != EAI_SYSTEM)
        errmsg = gai_strerror(_eaino);
    else
        errmsg = strerror(_saved_errno);
    return "[" + _requestName + "]: " + errmsg;
}

HostEntry::HostEntry(std::string desc)
    : _requestName(std::move(desc))
    , _saved_errno(0)
    , _eaino(0)
{
    addrinfo hints;
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_CANONNAME | AI_ADDRCONFIG;

    addrinfo* ainfo = nullptr;
    int rc = getaddrinfo(_requestName.c_str(), nullptr, &hints, &ainfo);
    if (rc != 0)
    {
        setEAI(rc);
        LOG_SYS("Failed to lookup host " << errorMessage());
        return;
    }
    _ainfo.reset(ainfo, freeaddrinfo);
    for (const addrinfo* ai = _ainfo.get(); ai; ai = ai->ai_next)
    {
        if (ai->ai_canonname)
            _canonicalName.assign(ai->ai_canonname);

        if (!ai->ai_addrlen || !ai->ai_addr)
            continue;

        std::string address = makeIPAddress(ai->ai_addr);
        if (!good())
            break;
        _ipAddresses.push_back(std::move(address));
    }
}

HostEntry::~HostEntry() = default;

namespace {

struct DNSCacheEntry
{
    std::string queryAddress;
    HostEntry hostEntry;
    std::chrono::steady_clock::time_point lookupTime;

    DNSCacheEntry(std::string address, HostEntry entry,
                  const std::chrono::steady_clock::time_point time)
        : queryAddress(std::move(address))
        , hostEntry(std::move(entry))
        , lookupTime(time)
    {
    }
};

}

static HostEntry resolveDNS(const std::string& addressToCheck,
                            std::vector<DNSCacheEntry>& querycache)
{
    const auto now = std::chrono::steady_clock::now();

    // search for hit
    auto findIt = std::find_if(querycache.begin(), querycache.end(),
                               [&addressToCheck](const auto& entry)->bool {
                                 return entry.queryAddress == addressToCheck;
                               });
    if (findIt != querycache.end())
    {
        // remove entries >= 20 seconds old
        static constexpr std::chrono::seconds MaxAge(20);
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - findIt->lookupTime) <
            MaxAge)
        {
            return findIt->hostEntry; // Valid and recent-enough.
        }

        // Too old; erase it and any other old entries.
        std::erase_if(querycache,
                      [now](const DNSCacheEntry& entry) -> bool
                      {
                          return std::chrono::duration_cast<std::chrono::milliseconds>(
                                     now - entry.lookupTime) >= MaxAge;
                      });
    }

    // lookup and cache
    HostEntry hostEntry(addressToCheck);
    querycache.emplace_back(addressToCheck, hostEntry, now);
    return hostEntry;
}

class DNSResolver
{
private:
    std::vector<DNSCacheEntry> _querycache;
public:
    HostEntry resolveDNS(const std::string& addressToCheck)
    {
        return net::resolveDNS(addressToCheck, _querycache);
    }
};

namespace
{
HostEntry syncResolveDNS(const std::string& addressToCheck)
{
#if !MOBILEAPP
    // Where we have async DNS then use it for the sync DNS use cases too
    // so we have a single cache of DNS results.
    std::mutex mutex;
    std::condition_variable cv;

    std::shared_ptr<HostEntry> result;

    net::AsyncDNS::DNSThreadDumpStateFn dumpState = [addressToCheck]() -> std::string
    { return "syncResolveDNS: [" + addressToCheck + ']'; };

    net::AsyncDNS::DNSThreadFn callback = [&mutex, &result, &cv](const HostEntry& hostEntry)
    {
        std::unique_lock<std::mutex> lock(mutex);
        result = std::make_shared<HostEntry>(hostEntry);
        lock.unlock();
        cv.notify_one();
    };

    std::unique_lock<std::mutex> lock(mutex);

    AsyncDNS::lookup(addressToCheck, std::move(callback), std::move(dumpState));

    cv.wait(lock, [&result]{ return static_cast<bool>(result); });

    return *result;
#else
    thread_local DNSResolver resolver;
    return resolver.resolveDNS(addressToCheck);
#endif
}

/// Returns true if the address is a cloud instance metadata IP,
/// matching core's opensocket_callback in CurlSession.cxx.
bool isInstanceMetadataAddress(const sockaddr* ai_addr)
{
    char addrstr[INET6_ADDRSTRLEN];
    const void* inAddr = nullptr;

    if (ai_addr->sa_family == AF_INET)
        inAddr = &(reinterpret_cast<const sockaddr_in*>(ai_addr)->sin_addr);
    else if (ai_addr->sa_family == AF_INET6)
        inAddr = &(reinterpret_cast<const sockaddr_in6*>(ai_addr)->sin6_addr);

    if (!inAddr)
        return false;

    if (!inet_ntop(ai_addr->sa_family, inAddr, addrstr, sizeof(addrstr)))
        return false;

    const std::string_view addr(addrstr);
    return addr == "169.254.169.254" || addr == "fd00:ec2::254";
}

using sockaddr_ptr = std::unique_ptr<sockaddr, void (*)(void*)>;

sockaddr_ptr dupAddrWithPort(const sockaddr* addr, socklen_t addrLen, uint16_t port)
{
    sockaddr_ptr newAddr(static_cast<sockaddr*>(malloc(addrLen)), free);
    memcpy(newAddr.get(), addr, addrLen);

    // Change port based on address family
    if (newAddr->sa_family == AF_INET)
    {
        sockaddr_in* addr_in = reinterpret_cast<sockaddr_in*>(newAddr.get());
        addr_in->sin_port = htons(port);
    }
    else if (newAddr->sa_family == AF_INET6)
    {
        sockaddr_in6* addr_in6 = reinterpret_cast<sockaddr_in6*>(newAddr.get());
        addr_in6->sin6_port = htons(port);
    }
    else
    {
        LOG_ERR("Unknown sa_family: " << newAddr->sa_family);
        newAddr.reset();
    }

    return newAddr;
}
} // namespace

#if !MOBILEAPP

std::string canonicalHostName(const std::string& addressToCheck)
{
    return syncResolveDNS(addressToCheck).getCanonicalName();
}

std::vector<std::string> resolveAddresses(const std::string& addressToCheck)
{
    HostEntry hostEntry = syncResolveDNS(addressToCheck);
    return hostEntry.getAddresses();
}

std::string HostEntry::resolveHostAddress() const
{
    if (!_ipAddresses.empty())
        return _ipAddresses[0];

    LOG_WRN("resolveHostAddress(\"" << _requestName << "\") failed");

    try
    {
        return Poco::Net::IPAddress(_requestName).toString();
    }
    catch (const Poco::Exception& exc1)
    {
        LOG_WRN("Poco::Net::IPAddress(\"" << _requestName
                                          << "\") failed: " << exc1.displayText());
    }

    return _requestName;
}

std::string resolveHostAddress(const std::string& targetHost)
{
    return syncResolveDNS(targetHost).resolveHostAddress();
}

bool HostEntry::isLocalhost() const
{
    const std::string targetAddress = resolveHostAddress();

    try
    {
        const auto list = Poco::Net::NetworkInterface::list(true, true);
        for (const auto& netif : list)
        {
            std::string address = netif.address().toString();
            address = address.substr(0, address.find('%', 0));
            if (address == targetAddress)
            {
                LOG_TRC("Host [" << _requestName << "] is on the same host as the client: \""
                                 << targetAddress << "\".");
                return true;
            }
        }
    }
    catch (const Poco::Exception& exc)
    {
        // possibly getifaddrs failed
        LOG_WRN("Poco::Net::NetworkInterface::list failed: " << exc.displayText() <<
                " (" << Util::symbolicErrno(errno) << ' ' << strerror(errno) << ")");
    }

    LOG_TRC("Host [" << _requestName << "] is not on the same host as the client: \"" << targetAddress
                     << "\".");
    return false;
}

bool isLocalhost(const std::string& targetHost)
{
    return syncResolveDNS(targetHost).isLocalhost();
}

void AsyncDNS::startThread()
{
    assert(!_thread);
    _exit = false;
    _thread.reset(new std::thread(&AsyncDNS::resolveDNS, this));
}

void AsyncDNS::joinThread()
{
    {
        std::unique_lock<std::mutex> guard(_lock);
        _exit = true;
    }
    _condition.notify_all();
    _thread->join();
    _thread.reset();
}

void AsyncDNS::dumpQueueState(std::ostream& os) const
{
    THREAD_UNSAFE_DUMP_BEGIN
    // NOT thread-safe
    Lookup activeLookup = _activeLookup;
    std::queue<Lookup> lookups = _lookups;
    os << "  active lookup: " << (activeLookup.cb ? "true" : "false") << '\n';
    if (activeLookup.cb)
    {
        os << "    lookup: " << activeLookup.query << '\n';
        os << "    callback: " << activeLookup.dumpState() << '\n';
    }
    os << "  queued lookups: " << lookups.size() << '\n';
    while (!lookups.empty())
    {
        os << "    lookup: " << lookups.front().query << '\n';
        os << "    callback: " << lookups.front().dumpState() << '\n';
        lookups.pop();
    }
    THREAD_UNSAFE_DUMP_END
}

AsyncDNS::AsyncDNS()
    : _unitWsd(UnitWSD::isUnitTesting() ? &UnitWSD::get() : nullptr)
    , _resolver(std::make_unique<DNSResolver>())
{
    startThread();
}

AsyncDNS::~AsyncDNS()
{
    joinThread();
}

void AsyncDNS::resolveDNS()
{
    ProcUtil::setThreadName("asyncdns");
    std::unique_lock<std::mutex> guard(_lock);
    while (true)
    {
        while (_lookups.empty() && !_exit)
            _condition.wait(guard);

        if (_exit)
            break;

        _activeLookup = _lookups.front();
        _lookups.pop();

        // Unlock to allow entries to queue up in _lookups while resolving
        _lock.unlock();

        UNITWSD_CALL_INSTANCE(_unitWsd, filterResolveDNS(_activeLookup.query));

        _activeLookup.cb(_resolver->resolveDNS(_activeLookup.query));

        _activeLookup = {};

        _lock.lock();
    }
}

void AsyncDNS::addLookup(std::string lookup, DNSThreadFn cb, DNSThreadDumpStateFn dumpState)
{
    std::unique_lock<std::mutex> guard(_lock);
    _lookups.emplace(std::move(lookup), std::move(cb), std::move(dumpState));
    guard.unlock();
    _condition.notify_one();
}

static std::unique_ptr<AsyncDNS> AsyncDNSThread;

//static
void AsyncDNS::startAsyncDNS()
{
    static std::once_flag once;
    std::call_once(once, []() { AsyncDNSThread = std::make_unique<AsyncDNS>(); });
}

//static
void AsyncDNS::dumpState(std::ostream& os)
{
    if (AsyncDNSThread)
    {
        os << "AsyncDNS:\n";
        AsyncDNSThread->dumpQueueState(os);
    }
    else
    {
        os << "AsyncDNS : doesn't exist.\n";
    }
}

//static
void AsyncDNS::stopAsyncDNS()
{
    AsyncDNSThread.reset();
}

//static
void AsyncDNS::lookup(std::string searchEntry, DNSThreadFn cb, DNSThreadDumpStateFn dumpState)
{
    AsyncDNSThread->addLookup(std::move(searchEntry), std::move(cb), std::move(dumpState));
}

void asyncConnect(std::string host, const std::string& port, const bool isSSL,
                  const std::shared_ptr<ProtocolHandlerInterface>& protocolHandler,
                  const asyncConnectCB& asyncCb)
{
    if (host.empty() || port.empty())
    {
        LOG_ERR("Invalid host/port " << host << ':' << port);
        asyncCb(nullptr, AsyncConnectResult::HostNameError);
        return;
    }

    LOG_DBG("Connecting to " << host << ':' << port << " (" << (isSSL ? "SSL)" : "Unencrypted)"));

#if !ENABLE_SSL
    if (isSSL)
    {
        LOG_ERR("Error: isSSL socket requested but SSL is not compiled in.");
        asyncCb(nullptr, AsyncConnectResult::MissingSSLError);
        return;
    }
#endif

    net::AsyncDNS::DNSThreadFn callback = [isSSL, host, port, protocolHandler,
                                           asyncCb](const HostEntry& hostEntry)
    {
        std::shared_ptr<StreamSocket> socket;

        AsyncConnectResult result = AsyncConnectResult::UnknownHostError;

        if (const addrinfo* ainfo = hostEntry.getAddrInfo())
        {
            for (const addrinfo* ai = ainfo; ai; ai = ai->ai_next)
            {
                if (ai->ai_addrlen && ai->ai_addr)
                {
                    if (isInstanceMetadataAddress(ai->ai_addr))
                    {
                        LOG_WRN("Blocking connection to instance metadata address for " << host);
                        result = AsyncConnectResult::ConnectionError;
                        break;
                    }

                    int fd = Syscall::socket_cloexec_nonblock(ai->ai_addr->sa_family, SOCK_STREAM /*| SOCK_NONBLOCK | SOCK_CLOEXEC*/, 0);
                    if (fd < 0)
                    {
                        result = AsyncConnectResult::SocketError;
                        LOG_SYS("Failed to create socket");
                        continue;
                    }

                    auto addrWithPort =
                        dupAddrWithPort(ai->ai_addr, ai->ai_addrlen, NumUtil::stoi(port));
                    int res = ::connect(fd, addrWithPort.get(), ai->ai_addrlen);
                    if (res < 0 && errno != EINPROGRESS)
                    {
                        result = AsyncConnectResult::ConnectionError;
                        LOG_SYS("Failed to connect to " << host);
                        ::close(fd);
                    }
                    else
                    {
                        Socket::Type type = ai->ai_family == AF_INET ? Socket::Type::IPv4 : Socket::Type::IPv6;
                        HostType hostType = hostEntry.isLocalhost() ? HostType::LocalHost : HostType::Other;
#if ENABLE_SSL
                        if (isSSL)
                        {
                            socket = StreamSocket::create<SslStreamSocket>(host, fd, type, true,
                                                                           hostType, protocolHandler);
                        }
#endif
                        if (!socket && !isSSL)
                        {
                            socket = StreamSocket::create<StreamSocket>(host, fd, type, true,
                                                                        hostType, protocolHandler);
                        }

                        if (socket)
                        {
                            LOG_DBG('#' << fd << " New socket connected to " << host << ':' << port
                                        << " (" << (isSSL ? "SSL)" : "Unencrypted)"));
                            result = AsyncConnectResult::Ok;
                            break;
                        }

                        result = AsyncConnectResult::SocketError;

                        LOG_ERR("Failed to allocate socket for client websocket " << host);
                        ::close(fd);
                        break;
                    }
                }
            }
        }
        else
            LOG_SYS("Failed to lookup host [" << host << "]. Skipping");

        asyncCb(std::move(socket), result);
    };

    std::string state = "asyncConnect: [" + host + ':' + port + ']';
    net::AsyncDNS::DNSThreadDumpStateFn dumpState = [state = std::move(state)]() -> std::string { return state; };

    AsyncDNS::lookup(std::move(host), std::move(callback), std::move(dumpState));
}

std::shared_ptr<StreamSocket>
connect(const std::string& host, const std::string& port, const bool isSSL,
        const std::shared_ptr<ProtocolHandlerInterface>& protocolHandler)
{
    std::shared_ptr<StreamSocket> socket;

    if (host.empty() || port.empty())
    {
        LOG_ERR("Invalid host/port " << host << ':' << port);
        return socket;
    }

    LOG_DBG("Connecting to " << host << ':' << port << " (" << (isSSL ? "SSL)" : "Unencrypted)"));

#if !ENABLE_SSL
    if (isSSL)
    {
        LOG_ERR("Error: isSSL socket requested but SSL is not compiled in.");
        return socket;
    }
#endif

    HostEntry hostEntry(syncResolveDNS(host));
    if (const addrinfo* ainfo = hostEntry.getAddrInfo())
    {
        for (const addrinfo* ai = ainfo; ai; ai = ai->ai_next)
        {
            if (ai->ai_addrlen && ai->ai_addr)
            {
                if (isInstanceMetadataAddress(ai->ai_addr))
                {
                    LOG_WRN("Blocking connection to instance metadata address for " << host);
                    break;
                }

                int fd = Syscall::socket_cloexec_nonblock(ai->ai_addr->sa_family, SOCK_STREAM /*| SOCK_NONBLOCK | SOCK_CLOEXEC*/, 0);
                if (fd < 0)
                {
                    LOG_SYS("Failed to create socket");
                    continue;
                }

                auto addrWithPort =
                    dupAddrWithPort(ai->ai_addr, ai->ai_addrlen, NumUtil::stoi(port));
                int res = ::connect(fd, addrWithPort.get(), ai->ai_addrlen);
                if (res < 0 && errno != EINPROGRESS)
                {
                    LOG_SYS("Failed to connect to " << host);
                    ::close(fd);
                }
                else
                {
                    Socket::Type type = ai->ai_family == AF_INET ? Socket::Type::IPv4 : Socket::Type::IPv6;
                    HostType hostType = hostEntry.isLocalhost() ? HostType::LocalHost : HostType::Other;
#if ENABLE_SSL
                    if (isSSL)
                    {
                        socket = StreamSocket::create<SslStreamSocket>(host, fd, type, true,
                                                                       hostType, protocolHandler);
                    }
#endif
                    if (!socket && !isSSL)
                    {
                        socket = StreamSocket::create<StreamSocket>(host, fd, type, true,
                                                                    hostType, protocolHandler);
                    }

                    if (socket)
                    {
                        LOG_DBG('#' << fd << " New socket connected to " << host << ':' << port
                                    << " (" << (isSSL ? "SSL)" : "Unencrypted)"));
                        break;
                    }

                    LOG_ERR("Failed to allocate socket for client websocket " << host);
                    ::close(fd);
                    break;
                }
            }
        }
    }
    else
        LOG_SYS("Failed to lookup host [" << host << "]. Skipping");

    return socket;
}

std::shared_ptr<StreamSocket>
connect(std::string uri, const std::shared_ptr<ProtocolHandlerInterface>& protocolHandler)
{
    std::string scheme;
    std::string host;
    std::string port;
    if (!parseUri(std::move(uri), scheme, host, port))
    {
        return nullptr;
    }

    const bool isSsl = Util::iequal(scheme, "https://") || Util::iequal(scheme, "wss://");

    return connect(host, port, isSsl, protocolHandler);
}

#else //!MOBILEAPP

bool HostEntry::isLocalhost() const
{
    return true;
}

#endif //!MOBILEAPP

bool sameOrigin(const std::string& expectedOrigin, const std::string& actualOrigin)
{
    // common case, and allow empty string to be equivalent
    if (expectedOrigin == actualOrigin)
        return true;

    std::string expectedScheme, expectedHostname, expectedPortString;
    if (!net::parseUri(expectedOrigin, expectedScheme, expectedHostname, expectedPortString))
    {
        LOG_ERR("Invalid expected origin URI [" << expectedOrigin << "] to sameOrigin");
        return false;
    }

    std::string actualScheme, actualHostname, actualPortString;
    if (!net::parseUri(actualOrigin, actualScheme, actualHostname, actualPortString))
    {
        LOG_ERR("Invalid actual origin URI [" << actualOrigin << "] to sameOrigin");
        return false;
    }

    if (expectedScheme != actualScheme || expectedHostname != actualHostname)
        return false;

    if (expectedPortString.empty())
        expectedPortString = getDefaultPortForScheme(expectedScheme);

    if (actualPortString.empty())
        actualPortString = getDefaultPortForScheme(actualScheme);

    return expectedPortString == actualPortString;
}

} // namespace net
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
