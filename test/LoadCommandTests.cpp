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

#include <common/LoadCommand.hpp>
#include <common/StringVector.hpp>

#include <test/lokassert.hpp>

#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>

#include <string>
#include <string_view>

/// LoadCommand unit-tests.
class LoadCommandTests : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(LoadCommandTests);
    CPPUNIT_TEST(testEveryOptionRoundTrips);
    CPPUNIT_TEST(testValueStaysOneToken);
    CPPUNIT_TEST(testUtf8Value);
    CPPUNIT_TEST(testEmptyValue);
    CPPUNIT_TEST(testSerializedForm);
    CPPUNIT_TEST(testSetReplaces);
    CPPUNIT_TEST(testParseSkipsBadTokens);
    CPPUNIT_TEST(testParseOnlyReadsLoad);
    CPPUNIT_TEST(testFindOption);
    CPPUNIT_TEST_SUITE_END();

    void testEveryOptionRoundTrips();
    void testValueStaysOneToken();
    void testUtf8Value();
    void testEmptyValue();
    void testSerializedForm();
    void testSetReplaces();
    void testParseSkipsBadTokens();
    void testParseOnlyReadsLoad();
    void testFindOption();
};

void LoadCommandTests::testEveryOptionRoundTrips()
{
    constexpr std::string_view testname = __func__;

    LoadCommand command;
    for (const LoadOptionInfo& info : LoadCommand::Table)
        command.set(info.option, "value of " + std::string(info.name));

    const LoadCommand::Options options =
        LoadCommand::parse(StringVector::tokenize(command.serialize()));

    LOK_ASSERT_EQUAL(LoadCommand::Table.size(), options.size());
    for (std::size_t i = 0; i < options.size(); ++i)
    {
        LOK_ASSERT(LoadCommand::Table[i].option == options[i].first);
        LOK_ASSERT_EQUAL_STR("value of " + std::string(LoadCommand::Table[i].name),
                             options[i].second);
    }
}

void LoadCommandTests::testValueStaysOneToken()
{
    constexpr std::string_view testname = __func__;

    const std::string value = "a b\nc=d%20e\tf,g\"h";

    LoadCommand command;
    command.set(LoadOption::Timezone, value);
    command.set(LoadOption::ReadOnly, true);

    const StringVector tokens = StringVector::tokenize(command.serialize());
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(3), tokens.size());

    const LoadCommand::Options options = LoadCommand::parse(tokens);
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(2), options.size());
    LOK_ASSERT(LoadOption::ReadOnly == options[0].first);
    LOK_ASSERT_EQUAL_STR("true", options[0].second);
    LOK_ASSERT(LoadOption::Timezone == options[1].first);
    LOK_ASSERT_EQUAL_STR(value, options[1].second);
}

void LoadCommandTests::testUtf8Value()
{
    constexpr std::string_view testname = __func__;

    const std::string name = "Zo\xc3\xab M\xc3\xbcller \xe5\x90\x8d\xe5\x89\x8d";

    LoadCommand command;
    command.set(LoadOption::Author, name);

    const LoadCommand::Options options =
        LoadCommand::parse(StringVector::tokenize(command.serialize()));
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(1), options.size());
    LOK_ASSERT(LoadOption::Author == options[0].first);
    LOK_ASSERT_EQUAL_STR(name, options[0].second);
}

void LoadCommandTests::testEmptyValue()
{
    constexpr std::string_view testname = __func__;

    LoadCommand command;
    command.set(LoadOption::Lang, "");
    LOK_ASSERT_EQUAL_STR("load lang=", command.serialize());

    const LoadCommand::Options options =
        LoadCommand::parse(StringVector::tokenize(command.serialize()));
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(1), options.size());
    LOK_ASSERT(LoadOption::Lang == options[0].first);
    LOK_ASSERT_EQUAL_STR("", options[0].second);
}

void LoadCommandTests::testSerializedForm()
{
    constexpr std::string_view testname = __func__;

    LoadCommand command;
    command.set(LoadOption::Url, "file:///tmp/a.odt");
    command.set(LoadOption::ReadOnly, false);
    command.set(LoadOption::MacroSecurityLevel, 2);
    command.set(LoadOption::WatermarkOpacity, 0.2);

    LOK_ASSERT_EQUAL_STR(
        "load macroSecurityLevel=2 readonly=false url=file:///tmp/a.odt watermarkOpacity=0.2",
        command.serialize());
}

void LoadCommandTests::testSetReplaces()
{
    constexpr std::string_view testname = __func__;

    LoadCommand command;
    command.set(LoadOption::AccessibilityState, false);
    command.set(LoadOption::AccessibilityState, true);

    LOK_ASSERT_EQUAL_STR("load accessibilityState=true", command.serialize());
}

void LoadCommandTests::testParseSkipsBadTokens()
{
    constexpr std::string_view testname = __func__;

    const LoadCommand::Options options = LoadCommand::parse(StringVector::tokenize(
        "load lang=de novalue unknown=1 lang=fr timezone=%zz part=3"));

    LOK_ASSERT_EQUAL(static_cast<std::size_t>(2), options.size());
    LOK_ASSERT(LoadOption::Lang == options[0].first);
    LOK_ASSERT_EQUAL_STR("de", options[0].second);
    LOK_ASSERT(LoadOption::Part == options[1].first);
    LOK_ASSERT_EQUAL_STR("3", options[1].second);
}

void LoadCommandTests::testParseOnlyReadsLoad()
{
    constexpr std::string_view testname = __func__;

    LOK_ASSERT(LoadCommand::parse(StringVector::tokenize("save lang=de part=3")).empty());
    LOK_ASSERT(LoadCommand::parse(StringVector::tokenize("lang=de part=3")).empty());
    LOK_ASSERT(LoadCommand::parse(StringVector::tokenize("")).empty());
}

void LoadCommandTests::testFindOption()
{
    constexpr std::string_view testname = __func__;

    const LoadOptionInfo* lang = LoadCommand::findOption("lang");
    LOK_ASSERT(lang != nullptr);
    LOK_ASSERT(LoadOption::Lang == lang->option);

    LOK_ASSERT(LoadCommand::findOption("Lang") == nullptr);
    LOK_ASSERT(LoadCommand::findOption("") == nullptr);
}

CPPUNIT_TEST_SUITE_REGISTRATION(LoadCommandTests);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
