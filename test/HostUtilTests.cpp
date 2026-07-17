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

#include <HostUtil.hpp>

#include <common/ConfigUtil.hpp>
#include <test/lokassert.hpp>

#include <Poco/URI.h>
#include <Poco/Util/MapConfiguration.h>

#include <cppunit/extensions/HelperMacros.h>

/// HostUtilTests unit-tests.
class HostUtilTests : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(HostUtilTests);

    CPPUNIT_TEST(testParseAlias);
    CPPUNIT_TEST(testParseHostUri);
    CPPUNIT_TEST(testFirstHostTrustedOnlyInFirstMode);

    CPPUNIT_TEST_SUITE_END();

    void testParseAlias();
    void testParseHostUri();
    void testFirstHostTrustedOnlyInFirstMode();

public:
    /// Clear the parsed host state, matching the empty state it starts with, and
    /// set whether WOPI is enabled.
    static void resetHostState(bool wopiEnabled);
};

namespace
{
/// Install a config for the duration of a test and put the previous config, plus
/// an empty and WOPI-disabled host state, back when the scope ends.
class ScopedHostConfig
{
public:
    explicit ScopedHostConfig(const Poco::Util::AbstractConfiguration* config)
        : _previous(ConfigUtil::setConfigForTest(config))
    {
    }

    ~ScopedHostConfig()
    {
        ConfigUtil::setConfigForTest(_previous);
        HostUtilTests::resetHostState(false);
    }

private:
    const Poco::Util::AbstractConfiguration* _previous;
};
}

void HostUtilTests::resetHostState(bool wopiEnabled)
{
    HostUtil::WopiHosts.clear();
    HostUtil::AliasHosts.clear();
    HostUtil::hostList.clear();
    HostUtil::FirstHost.clear();
    HostUtil::WopiEnabled = wopiEnabled;
}

void HostUtilTests::testParseAlias()
{
    constexpr std::string_view testname = __func__;

    LOK_ASSERT_EQUAL_STR("test2\\.local", HostUtil::parseAlias("test2.local"));
    LOK_ASSERT_EQUAL_STR("test3\\.local", HostUtil::parseAlias("http://test3.local"));
    LOK_ASSERT_EQUAL_STR("test3\\.local", HostUtil::parseAlias("https://test3.local"));
    LOK_ASSERT_EQUAL_STR("test4\\.local", HostUtil::parseAlias("https://test4.local:8080"));
    LOK_ASSERT_EQUAL_STR("test5\\.local", HostUtil::parseAlias("https://test5.local:8080/"));
    LOK_ASSERT_EQUAL_STR("test6\\.local", HostUtil::parseAlias("https://test6.local:8080/path"));
    LOK_ASSERT_EQUAL_STR("test7\\.local", HostUtil::parseAlias("test7.local/path"));
    LOK_ASSERT_EQUAL_STR("test8", HostUtil::parseAlias("http://test8"));
    LOK_ASSERT_EQUAL_STR("test9", HostUtil::parseAlias("http://test9:8080"));
    LOK_ASSERT_EQUAL_STR("test10", HostUtil::parseAlias("http://test10:8080/"));
    LOK_ASSERT_EQUAL_STR("test11", HostUtil::parseAlias("http://test11:8080/path"));

    LOK_ASSERT_EQUAL_STR("test", HostUtil::parseAlias("test")); // identical result regardless of interpretation

    LOK_ASSERT_EQUAL_STR("test[1-3]", HostUtil::parseAlias("test[1-3]")); // invalid hostname, interpret as regex
    LOK_ASSERT_EQUAL_STR("test[0-9].local", HostUtil::parseAlias("test[0-9].local"));
    LOK_ASSERT_EQUAL_STR("test[0-9]+.local", HostUtil::parseAlias("test[0-9]+.local"));
    LOK_ASSERT_EQUAL_STR("", HostUtil::parseAlias("test[0-9.local")); // invalid regex

    LOK_ASSERT_EQUAL_STR("https://:8080", HostUtil::parseAlias("https://:8080")); // not a valid url, no hostname
    LOK_ASSERT_EQUAL_STR("/my-path", HostUtil::parseAlias("/my-path")); // not a valid url, no hostname

    LOK_ASSERT_EQUAL_STR("https://aliasname[0-9]{1}:443", HostUtil::parseAlias("https://aliasname[0-9]{1}:443"));
}

void HostUtilTests::testParseHostUri()
{
    constexpr std::string_view testname = __func__;

    // A full URI keeps its host and port.
    {
        const auto uri = HostUtil::parseHostUri("https://example.com:8080");
        LOK_ASSERT(uri.has_value());
        LOK_ASSERT_EQUAL_STR("example.com", uri->getHost());
        LOK_ASSERT_EQUAL_STR("example.com:8080", uri->getAuthority());
    }

    // A host given without a scheme is accepted; only the host and port are used.
    {
        const auto uri = HostUtil::parseHostUri("example.com");
        LOK_ASSERT(uri.has_value());
        LOK_ASSERT_EQUAL_STR("example.com", uri->getHost());
    }

    // A custom port on a scheme-less host is preserved.
    {
        const auto uri = HostUtil::parseHostUri("example.com:8080");
        LOK_ASSERT(uri.has_value());
        LOK_ASSERT_EQUAL_STR("example.com:8080", uri->getAuthority());
    }

    // A regex in the host part is kept, with or without a scheme.
    {
        const auto uri = HostUtil::parseHostUri("example.*\\.com");
        LOK_ASSERT(uri.has_value());
        LOK_ASSERT_EQUAL_STR("example.*\\.com", uri->getHost());
    }
    {
        const auto uri = HostUtil::parseHostUri("https://example.*\\.com:443");
        LOK_ASSERT(uri.has_value());
        LOK_ASSERT_EQUAL_STR("example.*\\.com", uri->getHost());
    }

    // A scheme-less IPv4 regex, as the shipped configuration uses, parses to that regex.
    {
        const auto uri = HostUtil::parseHostUri("192\\.168\\.[0-9]{1,3}\\.[0-9]{1,3}");
        LOK_ASSERT(uri.has_value());
        LOK_ASSERT_EQUAL_STR("192\\.168\\.[0-9]{1,3}\\.[0-9]{1,3}", uri->getHost());
    }

    // Values that are not valid URIs (a regex in the port, an IPv6 literal) are rejected rather
    // than throwing and stopping the whole configuration from loading.
    LOK_ASSERT(!HostUtil::parseHostUri("example.com:1.*").has_value());
    LOK_ASSERT(!HostUtil::parseHostUri("::1").has_value());
}

void HostUtilTests::testFirstHostTrustedOnlyInFirstMode()
{
    constexpr std::string_view testname = __func__;

    Poco::AutoPtr<Poco::Util::MapConfiguration> config(new Poco::Util::MapConfiguration);
    config->setString("storage.wopi[@allow]", "true");

    const ScopedHostConfig scopedConfig(config.get());

    const Poco::URI uri("https://wopi.example.com:8443/");

    // An administrator who selected groups mode but has not added a group yet.
    config->setString("storage.wopi.alias_groups[@mode]", "groups");
    resetHostState(true);
    HostUtil::setFirstHost(uri);
    LOK_ASSERT_MESSAGE("groups mode with no group must not record a first host",
                       HostUtil::FirstHost.empty());
    LOK_ASSERT_MESSAGE("groups mode with no group leaves the host list empty",
                       HostUtil::isWopiHostsEmpty());
    LOK_ASSERT_MESSAGE("groups mode with no group denies the connecting host",
                       !HostUtil::allowedWopiHost(uri.getHost()));

    // The same connection in first mode is adopted as the one trusted host.
    config->setString("storage.wopi.alias_groups[@mode]", "first");
    resetHostState(true);
    HostUtil::setFirstHost(uri);
    LOK_ASSERT_EQUAL_STR(uri.getAuthority(), HostUtil::FirstHost);
    LOK_ASSERT_MESSAGE("first mode trusts the connecting host",
                       HostUtil::allowedWopiHost(uri.getHost()));
}

CPPUNIT_TEST_SUITE_REGISTRATION(HostUtilTests);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
