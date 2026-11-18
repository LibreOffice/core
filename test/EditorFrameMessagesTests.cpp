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

#include <string_view>

#include <cppunit/extensions/HelperMacros.h>

#include <common/EditorFrameMessages.hpp>
#include <test/lokassert.hpp>

// The messages that the editor frame of a remote document may send:
class EditorFrameMessagesTests: public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(EditorFrameMessagesTests);
    CPPUNIT_TEST(testLinkUpdateRefused);
    CPPUNIT_TEST(testUnoFileArgumentRefused);
    CPPUNIT_TEST(testUnoFileArgumentWithEscapesRefused);
    CPPUNIT_TEST(testUnoTextWithEscapesAllowed);
    CPPUNIT_TEST_SUITE_END();

    void testLinkUpdateRefused() {
        constexpr std::string_view testname = __func__;
        LOK_ASSERT(!EditorFrameMessages::isAllowed("allowlinkupdate"));
    }

    void testUnoFileArgumentRefused() {
        constexpr std::string_view testname = __func__;
        LOK_ASSERT(!EditorFrameMessages::isAllowed(
            R"(uno .uno:InsertDoc {"Name":{"type":"string","value":"file:///tmp/a.txt"}})"));
    }

    void testUnoFileArgumentWithEscapesRefused() {
        constexpr std::string_view testname = __func__;
        LOK_ASSERT(!EditorFrameMessages::isAllowed(
            R"(uno .uno:InsertDoc {"Name":{"type":"string","value":"fil\u0065:///tmp/a.txt"}})"));
    }

    void testUnoTextWithEscapesAllowed() {
        constexpr std::string_view testname = __func__;
        LOK_ASSERT(EditorFrameMessages::isAllowed(
            R"(uno .uno:InsertText {"Text":{"type":"string","value":"caf\u00e9 \u0041\u0042"}})"));
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(EditorFrameMessagesTests);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
