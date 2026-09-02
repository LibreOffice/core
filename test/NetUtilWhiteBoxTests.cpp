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
 * White box unit test for network utilities including buffers and URI handling.
 */

#include <config.h>

#include <common/IpNetwork.hpp>
#include <net/Buffer.hpp>
#include <net/NetUtil.hpp>
#include <net/Socket.hpp>
#include <net/Uri.hpp>

#include <test/lokassert.hpp>

#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>

#include <sys/socket.h>
#include <unistd.h>

#include <chrono>
#include <memory>
#include <vector>

namespace
{
class NoOpSocketHandler final : public SimpleSocketHandler
{
public:
    void onConnect(const std::shared_ptr<StreamSocket>&) override {}
    void handleIncomingMessage(SocketDisposition&) override {}
    int getPollEvents(std::chrono::steady_clock::time_point, int64_t&) override { return 0; }
    void performWrites(std::size_t) override {}
    void onDisconnect() override {}
};
}

/// Net-utility WhiteBox unit-tests.
class NetUtilWhiteBoxTests : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(NetUtilWhiteBoxTests);
    CPPUNIT_TEST(testBufferClass);
    CPPUNIT_TEST(testParseUri);
    CPPUNIT_TEST(testParseUriUrl);
    CPPUNIT_TEST(testParseUrl);
    CPPUNIT_TEST(testSameOrigin);
    CPPUNIT_TEST(testIpNetwork);
    CPPUNIT_TEST(testStreamSocketBufferBloatClose);
    CPPUNIT_TEST_SUITE_END();

    void testBufferClass();
    void testParseUri();
    void testParseUriUrl();
    void testParseUrl();
    void testSameOrigin();
    void testIpNetwork();
    void testStreamSocketBufferBloatClose();
};

void NetUtilWhiteBoxTests::testBufferClass()
{
    constexpr std::string_view testname = __func__;

    Buffer buf;
    LOK_ASSERT_EQUAL(0UL, buf.size());
    LOK_ASSERT_EQUAL(true, buf.empty());
    LOK_ASSERT_EQUAL(static_cast<const char*>(nullptr), buf.getBlock());
    buf.eraseFirst(buf.size());
    LOK_ASSERT_EQUAL(0UL, buf.size());
    LOK_ASSERT_EQUAL(true, buf.empty());

    // Small data.
    const char data[] = "abcdefghijklmnop";
    buf.append(data, sizeof(data));

    LOK_ASSERT_EQUAL(static_cast<std::size_t>(sizeof(data)), buf.size());
    LOK_ASSERT_EQUAL(false, buf.empty());
    LOK_ASSERT(buf.getBlock() != nullptr);
    LOK_ASSERT_EQUAL(0, memcmp(buf.getBlock(), data, buf.size()));

    // Erase one char at a time.
    for (std::size_t i = buf.size(); i > 0; --i)
    {
        buf.eraseFirst(1);
        LOK_ASSERT_EQUAL(i - 1, buf.size());
        LOK_ASSERT_EQUAL(i == 1, buf.empty()); // Not empty until the last element.
        LOK_ASSERT_EQUAL(buf.getBlock() != nullptr, !buf.empty());
        if (!buf.empty())
            LOK_ASSERT_EQUAL(0, memcmp(buf.getBlock(), data + (sizeof(data) - i) + 1, buf.size()));
    }

    // Large data.
    constexpr std::size_t BlockSize = 512 * 1024; // We add twice this.
    constexpr std::size_t BlockCount = 10;
    for (std::size_t i = 0; i < BlockCount; ++i)
    {
        const auto prevSize = buf.size();

        const std::vector<char> dataLarge(2 * BlockSize, 'a' + i); // Block of a single char.
        buf.append(dataLarge.data(), dataLarge.size());
        LOK_ASSERT_EQUAL(prevSize + (2 * BlockSize), buf.size());

        // Remove half.
        buf.eraseFirst(BlockSize);
        LOK_ASSERT_EQUAL(prevSize + BlockSize, buf.size());
        LOK_ASSERT_EQUAL(0, memcmp(buf.getBlock() + prevSize, dataLarge.data(), BlockSize));
    }

    LOK_ASSERT_EQUAL(BlockSize * BlockCount, buf.size());
    LOK_ASSERT_EQUAL(false, buf.empty());

    // Remove each block of data and test.
    for (std::size_t i = BlockCount / 2; i < BlockCount; ++i) // We removed half above.
    {
        LOK_ASSERT_EQUAL(false, buf.empty());
        LOK_ASSERT_EQUAL(BlockSize * 2 * (BlockCount - i), buf.size());

        const std::vector<char> dataLarge(BlockSize * 2, 'a' + i); // Block of a single char.
        LOK_ASSERT_EQUAL(0, memcmp(buf.getBlock(), dataLarge.data(), BlockSize));

        buf.eraseFirst(BlockSize * 2);
    }

    LOK_ASSERT_EQUAL(0UL, buf.size());
    LOK_ASSERT_EQUAL(true, buf.empty());

    // Very large data.
    const std::vector<char> dataLarge(20 * BlockSize, 'x'); // Block of a single char.
    buf.append(dataLarge.data(), dataLarge.size());
    LOK_ASSERT_EQUAL(dataLarge.size(), buf.size());

    buf.append(data, sizeof(data)); // Add small data.
    LOK_ASSERT_EQUAL(dataLarge.size() + sizeof(data), buf.size());

    buf.eraseFirst(dataLarge.size()); // Remove large data.
    LOK_ASSERT_EQUAL(sizeof(data), buf.size());
    LOK_ASSERT_EQUAL(false, buf.empty());
    LOK_ASSERT_EQUAL(0, memcmp(buf.getBlock(), data, buf.size()));

    buf.eraseFirst(buf.size()); // Remove all.
    LOK_ASSERT_EQUAL(0UL, buf.size());
    LOK_ASSERT_EQUAL(true, buf.empty());
}

void NetUtilWhiteBoxTests::testParseUri()
{
    constexpr std::string_view testname = __func__;

    std::string scheme = "***";
    std::string host = "***";
    std::string port = "***";

    LOK_ASSERT(!net::parseUri(std::string(), scheme, host, port));
    LOK_ASSERT(scheme.empty());
    LOK_ASSERT(host.empty());
    LOK_ASSERT(port.empty());

    LOK_ASSERT(net::parseUri("localhost", scheme, host, port));
    LOK_ASSERT(scheme.empty());
    LOK_ASSERT_EQUAL_STR("localhost", host);
    LOK_ASSERT(port.empty());

    LOK_ASSERT(net::parseUri("127.0.0.1", scheme, host, port));
    LOK_ASSERT(scheme.empty());
    LOK_ASSERT_EQUAL_STR("127.0.0.1", host);
    LOK_ASSERT(port.empty());

    LOK_ASSERT(net::parseUri("domain.com", scheme, host, port));
    LOK_ASSERT(scheme.empty());
    LOK_ASSERT_EQUAL_STR("domain.com", host);
    LOK_ASSERT(port.empty());

    LOK_ASSERT(net::parseUri("127.0.0.1:9999", scheme, host, port));
    LOK_ASSERT(scheme.empty());
    LOK_ASSERT_EQUAL_STR("127.0.0.1", host);
    LOK_ASSERT_EQUAL_STR("9999", port);

    LOK_ASSERT(net::parseUri("domain.com:88", scheme, host, port));
    LOK_ASSERT(scheme.empty());
    LOK_ASSERT_EQUAL_STR("domain.com", host);
    LOK_ASSERT_EQUAL_STR("88", port);

    LOK_ASSERT(net::parseUri("http://domain.com", scheme, host, port));
    LOK_ASSERT_EQUAL_STR("http://", scheme);
    LOK_ASSERT_EQUAL_STR("domain.com", host);
    LOK_ASSERT(port.empty());

    LOK_ASSERT(net::parseUri("https://domain.com:88", scheme, host, port));
    LOK_ASSERT_EQUAL_STR("https://", scheme);
    LOK_ASSERT_EQUAL_STR("domain.com", host);
    LOK_ASSERT_EQUAL_STR("88", port);

    LOK_ASSERT(net::parseUri("http://domain.com/path/to/file", scheme, host, port));
    LOK_ASSERT_EQUAL_STR("http://", scheme);
    LOK_ASSERT_EQUAL_STR("domain.com", host);
    LOK_ASSERT(port.empty());

    LOK_ASSERT(net::parseUri("https://domain.com:88/path/to/file", scheme, host, port));
    LOK_ASSERT_EQUAL_STR("https://", scheme);
    LOK_ASSERT_EQUAL_STR("domain.com", host);
    LOK_ASSERT_EQUAL_STR("88", port);

    LOK_ASSERT(net::parseUri("wss://127.0.0.1:9999/", scheme, host, port));
    LOK_ASSERT_EQUAL_STR("wss://", scheme);
    LOK_ASSERT_EQUAL_STR("127.0.0.1", host);
    LOK_ASSERT_EQUAL_STR("9999", port);

    // IPv6 literals are bracketed so their colons are not read as a port; the
    // brackets are stripped from the returned host.
    LOK_ASSERT(net::parseUri("[::1]", scheme, host, port));
    LOK_ASSERT(scheme.empty());
    LOK_ASSERT_EQUAL_STR("::1", host);
    LOK_ASSERT(port.empty());

    LOK_ASSERT(net::parseUri("[::1]:9980", scheme, host, port));
    LOK_ASSERT(scheme.empty());
    LOK_ASSERT_EQUAL_STR("::1", host);
    LOK_ASSERT_EQUAL_STR("9980", port);

    LOK_ASSERT(net::parseUri("https://[2001:db8::1]:88", scheme, host, port));
    LOK_ASSERT_EQUAL_STR("https://", scheme);
    LOK_ASSERT_EQUAL_STR("2001:db8::1", host);
    LOK_ASSERT_EQUAL_STR("88", port);

    // A bare (unbracketed) IPv6 literal is accepted as a host with no port;
    // brackets are only needed when a port follows.
    LOK_ASSERT(net::parseUri("::1", scheme, host, port));
    LOK_ASSERT(scheme.empty());
    LOK_ASSERT_EQUAL_STR("::1", host);
    LOK_ASSERT(port.empty());

    LOK_ASSERT(net::parseUri("2001:db8::1", scheme, host, port));
    LOK_ASSERT(scheme.empty());
    LOK_ASSERT_EQUAL_STR("2001:db8::1", host);
    LOK_ASSERT(port.empty());

    // Malformed: an unterminated bracket is rejected.
    LOK_ASSERT(!net::parseUri("[::1", scheme, host, port));
}

void NetUtilWhiteBoxTests::testParseUriUrl()
{
    constexpr std::string_view testname = __func__;

    std::string scheme = "***";
    std::string host = "***";
    std::string port = "***";
    std::string pathAndQuery = "***";

    LOK_ASSERT(!net::parseUri(std::string(), scheme, host, port, pathAndQuery));
    LOK_ASSERT(scheme.empty());
    LOK_ASSERT(host.empty());
    LOK_ASSERT(port.empty());
    LOK_ASSERT(pathAndQuery.empty());

    LOK_ASSERT(net::parseUri("localhost", scheme, host, port, pathAndQuery));
    LOK_ASSERT(scheme.empty());
    LOK_ASSERT_EQUAL_STR("localhost", host);
    LOK_ASSERT(port.empty());
    LOK_ASSERT(pathAndQuery.empty());

    LOK_ASSERT(net::parseUri("127.0.0.1", scheme, host, port, pathAndQuery));
    LOK_ASSERT(scheme.empty());
    LOK_ASSERT_EQUAL_STR("127.0.0.1", host);
    LOK_ASSERT(port.empty());
    LOK_ASSERT(pathAndQuery.empty());

    LOK_ASSERT(net::parseUri("domain.com", scheme, host, port, pathAndQuery));
    LOK_ASSERT(scheme.empty());
    LOK_ASSERT_EQUAL_STR("domain.com", host);
    LOK_ASSERT(port.empty());
    LOK_ASSERT(pathAndQuery.empty());

    LOK_ASSERT(net::parseUri("127.0.0.1:9999", scheme, host, port, pathAndQuery));
    LOK_ASSERT(scheme.empty());
    LOK_ASSERT_EQUAL_STR("127.0.0.1", host);
    LOK_ASSERT_EQUAL_STR("9999", port);
    LOK_ASSERT(pathAndQuery.empty());

    LOK_ASSERT(net::parseUri("domain.com:88", scheme, host, port, pathAndQuery));
    LOK_ASSERT(scheme.empty());
    LOK_ASSERT_EQUAL_STR("domain.com", host);
    LOK_ASSERT_EQUAL_STR("88", port);
    LOK_ASSERT(pathAndQuery.empty());

    LOK_ASSERT(net::parseUri("http://domain.com", scheme, host, port, pathAndQuery));
    LOK_ASSERT_EQUAL_STR("http://", scheme);
    LOK_ASSERT_EQUAL_STR("domain.com", host);
    LOK_ASSERT(port.empty());
    LOK_ASSERT(pathAndQuery.empty());

    LOK_ASSERT(net::parseUri("https://domain.com:88", scheme, host, port, pathAndQuery));
    LOK_ASSERT_EQUAL_STR("https://", scheme);
    LOK_ASSERT_EQUAL_STR("domain.com", host);
    LOK_ASSERT_EQUAL_STR("88", port);

    LOK_ASSERT(net::parseUri("http://domain.com/path/to/file", scheme, host, port, pathAndQuery));
    LOK_ASSERT_EQUAL_STR("http://", scheme);
    LOK_ASSERT_EQUAL_STR("domain.com", host);
    LOK_ASSERT(port.empty());
    LOK_ASSERT_EQUAL_STR("/path/to/file", pathAndQuery);

    LOK_ASSERT(
        net::parseUri("https://domain.com:88/path/to/file", scheme, host, port, pathAndQuery));
    LOK_ASSERT_EQUAL_STR("https://", scheme);
    LOK_ASSERT_EQUAL_STR("domain.com", host);
    LOK_ASSERT_EQUAL_STR("88", port);
    LOK_ASSERT_EQUAL_STR("/path/to/file", pathAndQuery);

    LOK_ASSERT(net::parseUri("wss://127.0.0.1:9999/", scheme, host, port, pathAndQuery));
    LOK_ASSERT_EQUAL_STR("wss://", scheme);
    LOK_ASSERT_EQUAL_STR("127.0.0.1", host);
    LOK_ASSERT_EQUAL_STR("9999", port);
    LOK_ASSERT_EQUAL_STR("/", pathAndQuery);

    // IPv6 literal with a port and a path.
    LOK_ASSERT(
        net::parseUri("https://[2001:db8::1]:88/path/to/file", scheme, host, port, pathAndQuery));
    LOK_ASSERT_EQUAL_STR("https://", scheme);
    LOK_ASSERT_EQUAL_STR("2001:db8::1", host);
    LOK_ASSERT_EQUAL_STR("88", port);
    LOK_ASSERT_EQUAL_STR("/path/to/file", pathAndQuery);

    LOK_ASSERT(net::parseUri("[::1]:9980/", scheme, host, port, pathAndQuery));
    LOK_ASSERT(scheme.empty());
    LOK_ASSERT_EQUAL_STR("::1", host);
    LOK_ASSERT_EQUAL_STR("9980", port);
    LOK_ASSERT_EQUAL_STR("/", pathAndQuery);
}

void NetUtilWhiteBoxTests::testParseUrl()
{
    constexpr std::string_view testname = __func__;

    LOK_ASSERT_EQUAL_STR(std::string(), net::parseUrl(""));

    LOK_ASSERT_EQUAL_STR(std::string(), net::parseUrl("https://sub.domain.com:80"));
    LOK_ASSERT_EQUAL_STR("/", net::parseUrl("https://sub.domain.com:80/"));

    LOK_ASSERT_EQUAL_STR("/some/path", net::parseUrl("https://sub.domain.com:80/some/path"));
}

void NetUtilWhiteBoxTests::testSameOrigin()
{
    constexpr std::string_view testname = __func__;

    LOK_ASSERT(net::sameOrigin("", ""));
    LOK_ASSERT(!net::sameOrigin("http://sub.domain.com", ""));
    LOK_ASSERT(net::sameOrigin("http://sub.domain.com", "http://sub.domain.com"));
    LOK_ASSERT(net::sameOrigin("https://sub.domain.com", "https://sub.domain.com"));
    LOK_ASSERT(!net::sameOrigin("http://sub.domain.com", "https://sub.domain.com"));
    LOK_ASSERT(net::sameOrigin("https://sub.domain.com", "https://sub.domain.com:443"));
    LOK_ASSERT(net::sameOrigin("http://sub.domain.com", "http://sub.domain.com:80"));
    LOK_ASSERT(!net::sameOrigin("https://sub.domain.com", "https://sub.domain.com:80"));
    LOK_ASSERT(!net::sameOrigin("http://sub.domain.com", "http://sub.domain.com:443"));
    LOK_ASSERT(!net::sameOrigin("http://sub.domain.com:88", "http://sub.domain.com:80"));
}

// A peer that stops reading makes data pile up in our outgoing buffer. Once it
// has stayed very large for long enough, the poll handler must drop the
// connection so the memory is reclaimed. The buffer-bloat limits are wound
// down here so the test triggers in milliseconds rather than minutes.
void NetUtilWhiteBoxTests::testIpNetwork()
{
    constexpr std::string_view testname = __func__;

    // Not CIDR notation at all: plain addresses, host names and regular expressions.
    LOK_ASSERT(!Util::IpNetwork::parse("").has_value());
    LOK_ASSERT(!Util::IpNetwork::parse("127.0.0.1").has_value());
    LOK_ASSERT(!Util::IpNetwork::parse("localhost").has_value());
    LOK_ASSERT(!Util::IpNetwork::parse(R"(192\.168\.[0-9]{1,3}\.[0-9]{1,3})").has_value());
    LOK_ASSERT(!Util::IpNetwork::parse("/8").has_value());
    LOK_ASSERT(!Util::IpNetwork::parse("10.0.0.0/").has_value());
    LOK_ASSERT(!Util::IpNetwork::parse("10.0.0.0/8/").has_value());
    LOK_ASSERT(!Util::IpNetwork::parse("10.0.0.0/x").has_value());
    LOK_ASSERT(!Util::IpNetwork::parse("10.0.0.0/-1").has_value());
    LOK_ASSERT(!Util::IpNetwork::parse("10.0.0.0/8 ").has_value());
    LOK_ASSERT(!Util::IpNetwork::parse("example.com/8").has_value());

    // Prefix length must fit the address family.
    LOK_ASSERT(!Util::IpNetwork::parse("10.0.0.0/33").has_value());
    LOK_ASSERT(Util::IpNetwork::parse("10.0.0.0/32").has_value());
    LOK_ASSERT(Util::IpNetwork::parse("10.0.0.0/0").has_value());
    LOK_ASSERT(!Util::IpNetwork::parse("fd00::/129").has_value());
    LOK_ASSERT(Util::IpNetwork::parse("fd00::/128").has_value());

    // Carrier-grade NAT range, as used for Kubernetes pod networks.
    std::optional<Util::IpNetwork> cgnat = Util::IpNetwork::parse("100.64.0.0/10");
    LOK_ASSERT(cgnat.has_value());
    LOK_ASSERT_EQUAL(std::string("100.64.0.0/10"), cgnat->toString());
    LOK_ASSERT(cgnat->contains("100.64.0.0"));
    LOK_ASSERT(cgnat->contains("100.64.0.1"));
    LOK_ASSERT(cgnat->contains("100.100.200.3"));
    LOK_ASSERT(cgnat->contains("100.127.255.255"));
    LOK_ASSERT(!cgnat->contains("100.63.255.255"));
    LOK_ASSERT(!cgnat->contains("100.128.0.0"));
    LOK_ASSERT(!cgnat->contains("10.64.0.1"));
    // IPv4-mapped IPv6 spelling of the same peers, as seen on a dual-stack socket.
    LOK_ASSERT(cgnat->contains("::ffff:100.64.0.1"));
    LOK_ASSERT(cgnat->contains("::ffff:100.100.200.3"));
    LOK_ASSERT(!cgnat->contains("::ffff:100.128.0.0"));
    // Not addresses.
    LOK_ASSERT(!cgnat->contains(""));
    LOK_ASSERT(!cgnat->contains("localhost"));
    LOK_ASSERT(!cgnat->contains("100.64.0.1/10"));
    // A native IPv6 address is never in an IPv4 network.
    LOK_ASSERT(!cgnat->contains("::1"));
    LOK_ASSERT(!cgnat->contains("fd00::1"));

    // Prefix that does not end on a byte boundary.
    std::optional<Util::IpNetwork> slash12 = Util::IpNetwork::parse("172.16.0.0/12");
    LOK_ASSERT(slash12.has_value());
    LOK_ASSERT(slash12->contains("172.16.0.1"));
    LOK_ASSERT(slash12->contains("172.31.255.254"));
    LOK_ASSERT(!slash12->contains("172.15.255.255"));
    LOK_ASSERT(!slash12->contains("172.32.0.0"));

    // Single host and match-all.
    std::optional<Util::IpNetwork> single = Util::IpNetwork::parse("203.0.113.7/32");
    LOK_ASSERT(single.has_value());
    LOK_ASSERT(single->contains("203.0.113.7"));
    LOK_ASSERT(single->contains("::ffff:203.0.113.7"));
    LOK_ASSERT(!single->contains("203.0.113.8"));

    std::optional<Util::IpNetwork> all = Util::IpNetwork::parse("0.0.0.0/0");
    LOK_ASSERT(all.has_value());
    LOK_ASSERT(all->contains("203.0.113.8"));
    LOK_ASSERT(all->contains("::ffff:1.2.3.4"));
    LOK_ASSERT(!all->contains("::1"));

    // IPv6.
    std::optional<Util::IpNetwork> ula = Util::IpNetwork::parse("fd00::/8");
    LOK_ASSERT(ula.has_value());
    LOK_ASSERT(ula->contains("fd00::1"));
    LOK_ASSERT(ula->contains("fdab:cdef::1234"));
    LOK_ASSERT(!ula->contains("fe80::1"));
    LOK_ASSERT(!ula->contains("::1"));
    LOK_ASSERT(!ula->contains("10.0.0.1"));

    std::optional<Util::IpNetwork> loopback6 = Util::IpNetwork::parse("::1/128");
    LOK_ASSERT(loopback6.has_value());
    LOK_ASSERT(loopback6->contains("::1"));
    LOK_ASSERT(loopback6->contains("[::1]"));
    LOK_ASSERT(loopback6->contains("0:0:0:0:0:0:0:1"));
    LOK_ASSERT(loopback6->contains("0000:0000:0000:0000:0000:0000:0000:0001"));
    LOK_ASSERT(!loopback6->contains("::2"));
    LOK_ASSERT(!loopback6->contains("127.0.0.1"));

    // IPv6 text forms that must be rejected.
    LOK_ASSERT(!ula->contains("fd00:::1"));
    LOK_ASSERT(!ula->contains("fd00::1::2"));
    LOK_ASSERT(!ula->contains("fd00::12345"));
    LOK_ASSERT(!ula->contains("fd00::g"));
    LOK_ASSERT(!ula->contains("fd00:1:2:3:4:5:6:7:8"));
    LOK_ASSERT(!ula->contains("fd00:1:2:3:4:5:6"));
    LOK_ASSERT(!ula->contains(":fd00::1"));
    LOK_ASSERT(!ula->contains("fd00::1:"));
    LOK_ASSERT(!ula->contains("1.2.3.4::"));
    // Uppercase hex and an embedded IPv4 tail are fine.
    LOK_ASSERT(ula->contains("FD00::ABCD"));
    LOK_ASSERT(ula->contains("fd00::1.2.3.4"));
    // The mapped range itself folds to IPv4, where a /96 makes no sense.
    LOK_ASSERT(!Util::IpNetwork::parse("::ffff:0:0/96").has_value());
    LOK_ASSERT(cgnat->contains("[::ffff:100.64.0.1]"));
}

void NetUtilWhiteBoxTests::testStreamSocketBufferBloatClose()
{
    constexpr std::string_view testname = __func__;

    int fds[2];
    LOK_ASSERT_EQUAL(0, ::socketpair(AF_UNIX, SOCK_STREAM, 0, fds));

    // Build the socket directly (not via StreamSocket::socketpair, which marks
    // the socket no-shutdown) so a forced close really shuts it down.
    std::shared_ptr<StreamSocket> socket = std::make_shared<StreamSocket>(
        "buffer-bloat-test", fds[0], Socket::Type::Unix, /*isClient=*/true, HostType::Other);
    socket->setHandler(std::make_shared<NoOpSocketHandler>());

    // A freshly connected socket is open.
    LOK_ASSERT(socket->isOpen());

    // Wind the limits right down: 4 KB held for 100 ms is enough to close.
    StreamSocket::BufferBloatCloseSize = 4096;
    StreamSocket::BufferBloatCloseDuration = std::chrono::milliseconds(100);

    const auto start = std::chrono::steady_clock::now();

    // A socket well under the threshold is never flagged, however long we wait.
    LOK_ASSERT_EQUAL(false, socket->checkBufferBloat(start + std::chrono::hours(1)));

    // Simulate a peer that has stopped reading: keep writing until the outgoing
    // buffer sits above the threshold.
    const std::vector<char> chunk(1024, 'x');
    while (socket->getOutBuffer().size() <= StreamSocket::BufferBloatCloseSize)
        socket->getOutBuffer().append(chunk.data(), chunk.size());
    LOK_ASSERT(socket->getOutBuffer().size() > StreamSocket::BufferBloatCloseSize);

    // Crossing the threshold only starts the timer; the socket stays open.
    {
        SocketDisposition disposition(socket);
        socket->handlePoll(disposition, start, 0);
        LOK_ASSERT_EQUAL(false, disposition.isClosed());
        LOK_ASSERT(socket->isOpen());
    }

    // Still bloated, but within the period: still open.
    {
        SocketDisposition disposition(socket);
        socket->handlePoll(disposition, start + std::chrono::milliseconds(50), 0);
        // Tests sometimes run extremely slow in CI
        bool shouldBeClosed = std::chrono::steady_clock::now() - start >= StreamSocket::BufferBloatCloseDuration;
        LOK_ASSERT_EQUAL(shouldBeClosed, disposition.isClosed());
        LOK_ASSERT_EQUAL(socket->isOpen(), !shouldBeClosed);
    }

    // Once it has stayed bloated past the period, the poll handler closes it
    // and the buffer-bloat close is counted.
    {
        const size_t closedBefore = StreamSocket::getBufferBloatClosedCount();
        SocketDisposition disposition(socket);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        // by now has to be closed
        socket->handlePoll(disposition, start + std::chrono::milliseconds(150), 0);
        LOK_ASSERT_EQUAL(true, disposition.isClosed());
        LOK_ASSERT(!socket->isOpen());
        LOK_ASSERT(socket->isShutdown());
        LOK_ASSERT_EQUAL(closedBefore + 1, StreamSocket::getBufferBloatClosedCount());
    }

    ::close(fds[1]);
}

CPPUNIT_TEST_SUITE_REGISTRATION(NetUtilWhiteBoxTests);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
