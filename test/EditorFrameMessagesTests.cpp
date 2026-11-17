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

#include <string_view>

#include <cppunit/extensions/HelperMacros.h>

#include <common/EditorFrameMessages.hpp>
#include <test/lokassert.hpp>

// The messages that the editor frame of a remote document may send:
class EditorFrameMessagesTests: public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(EditorFrameMessagesTests);
    CPPUNIT_TEST(testLinkUpdateRefused);
    CPPUNIT_TEST_SUITE_END();

    void testLinkUpdateRefused() {
        constexpr std::string_view testname = __func__;
        LOK_ASSERT(!EditorFrameMessages::isAllowed("allowlinkupdate"));
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(EditorFrameMessagesTests);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
