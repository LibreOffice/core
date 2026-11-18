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

#include <config.h>

#include <string>
#include <string_view>

#include <Poco/JSON/Object.h>
#include <cppunit/extensions/HelperMacros.h>

#include <common/JsonUtil.hpp>
#include <common/SettingsSecrets.hpp>
#include <common/ViewSettings.hpp>
#include <test/lokassert.hpp>

// A settings body leaves out its stored secrets on the way to a page:
class SettingsSecretsTests: public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(SettingsSecretsTests);
    CPPUNIT_TEST(testSecretLeftBlankAndFlagged);
    CPPUNIT_TEST(testBodyWithoutSecretUnchanged);
    CPPUNIT_TEST(testBodyThatDoesNotParseEmpty);
    CPPUNIT_TEST_SUITE_END();

    void testSecretLeftBlankAndFlagged() {
        constexpr std::string_view testname = __func__;
        std::string const redacted = SettingsSecrets::redactSettingSecrets(
            R"({"zoteroAPIKey":"secret","accessibilityState":true})", ViewSettings::SecretFields);
        Poco::JSON::Object::Ptr json;
        LOK_ASSERT(JsonUtil::parseJSON(redacted, json));
        std::string key = "unset";
        JsonUtil::findJSONValue(json, "zoteroAPIKey", key);
        LOK_ASSERT_EQUAL(std::string(), key);
        bool stored = false;
        JsonUtil::findJSONValue(json, "zoteroAPIKeyStored", stored);
        LOK_ASSERT(stored);
        bool accessibility = false;
        JsonUtil::findJSONValue(json, "accessibilityState", accessibility);
        LOK_ASSERT(accessibility);
    }

    void testBodyWithoutSecretUnchanged() {
        constexpr std::string_view testname = __func__;
        std::string const body = R"({"zoteroAPIKey":"","accessibilityState":true})";
        LOK_ASSERT_EQUAL(
            body, SettingsSecrets::redactSettingSecrets(body, ViewSettings::SecretFields));
    }

    void testBodyThatDoesNotParseEmpty() {
        constexpr std::string_view testname = __func__;
        LOK_ASSERT_EQUAL(
            std::string(),
            SettingsSecrets::redactSettingSecrets(
                R"({"zoteroAPIKey":"secret",)", ViewSettings::SecretFields));
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(SettingsSecretsTests);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
