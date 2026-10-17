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
 * Unit tests for the configuration defaults.
 */

#include <config.h>

#include <test/lokassert.hpp>

#include <common/ConfigUtil.hpp>

#include <Poco/AutoPtr.h>
#include <Poco/Util/XMLConfiguration.h>

#include <cppunit/extensions/HelperMacros.h>

#include <string>
#include <string_view>
#include <vector>

/// ConfigUtil unit-tests.
class ConfigUtilTests : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(ConfigUtilTests);

    CPPUNIT_TEST(testDefaultsMatchXml);

    CPPUNIT_TEST_SUITE_END();

    void testDefaultsMatchXml();
};

namespace
{
/// Collect the keys of the leaf elements under the given key.
void collectLeafKeys(const Poco::Util::AbstractConfiguration& config, const std::string& parent,
                     std::vector<std::string>& keys)
{
    Poco::Util::AbstractConfiguration::Keys children;
    config.keys(parent, children);
    if (children.empty())
    {
        keys.push_back(parent);
        return;
    }

    for (const std::string& child : children)
        collectLeafKeys(config, parent.empty() ? child : parent + '.' + child, keys);
}
} // namespace

/// Every setting coolwsd.xml documents with a 'default' attribute must have that default in
/// ConfigUtil's defaults, so a config file that lacks the setting gets the documented value.
void ConfigUtilTests::testDefaultsMatchXml()
{
    constexpr std::string_view testname = __func__;

    // The configured coolwsd.xml, with the configure-dependent defaults filled in.
    Poco::AutoPtr<Poco::Util::XMLConfiguration> xml(
        new Poco::Util::XMLConfiguration(COOLWSD_XML_PATH));
    const auto& defaults = ConfigUtil::getDefaultAppConfig();

    std::vector<std::string> keys;
    collectLeafKeys(*xml, std::string(), keys);
    LOK_ASSERT_MESSAGE("Found no settings in " COOLWSD_XML_PATH, keys.size() > 100);

    std::size_t checked = 0;
    for (const std::string& key : keys)
    {
        const std::string defaultKey = key + "[@default]";
        if (!xml->hasProperty(defaultKey))
            continue;

        const std::string documented = xml->getString(defaultKey);
        const auto it = defaults.find(key);

        if (key == "remote_links.chain_secret")
        {
            // Kept out: COOLWSD passes the defaults' keys to the kits in their environment.
            LOK_ASSERT_MESSAGE_SILENT("The chain secret must not be in the defaults",
                                      it == defaults.end());
            continue;
        }

        if (key == "logging.anonymize.anonymize_user_data")
        {
            // Kept out so the fallback applies.
            LOK_ASSERT_MESSAGE_SILENT("anonymize_user_data must not be in the defaults",
                                      it == defaults.end());
            continue;
        }

        LOK_ASSERT_MESSAGE_SILENT("Setting [" << key << "] has default [" << documented
                                              << "] in coolwsd.xml but none in ConfigUtil",
                                  it != defaults.end());

        if (key == "ssl.cipher_list")
        {
            // Empty, as in coolwsd.xml, falls back to the documented cipher set.
            LOK_ASSERT_EQUAL_STR(std::string(), it->second);
            LOK_ASSERT_EQUAL_STR(std::string(DEFAULT_CIPHER_SET), documented);
            continue;
        }

        LOK_ASSERT_MESSAGE_SILENT("Setting [" << key << "] has default [" << documented
                                              << "] in coolwsd.xml but [" << it->second
                                              << "] in ConfigUtil",
                                  documented == it->second);
        ++checked;
    }

    TST_LOG("Checked " << checked << " defaults against " COOLWSD_XML_PATH);
    LOK_ASSERT_MESSAGE("Found no documented defaults in " COOLWSD_XML_PATH, checked > 100);
}

CPPUNIT_TEST_SUITE_REGISTRATION(ConfigUtilTests);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
