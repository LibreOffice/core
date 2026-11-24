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

#include <chrono>
#include <string>
#include <string_view>

#include <cppunit/extensions/HelperMacros.h>

#include <common/RotatingToken.hpp>
#include <test/lokassert.hpp>

// A value of a token that rotates by time stays valid for one or two intervals after it was
// current:
class RotatingTokenTests: public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(RotatingTokenTests);
    CPPUNIT_TEST(testValidForTwoIntervals);
    CPPUNIT_TEST(testInvalidAfterAPause);
    CPPUNIT_TEST_SUITE_END();

    void testValidForTwoIntervals() {
        constexpr std::string_view testname = __func__;
        RotatingToken token(16, std::chrono::minutes(5));
        auto const start = std::chrono::steady_clock::now();
        std::string const first = token.getCurrent();
        LOK_ASSERT(!token.rotateIfNeeded(start + std::chrono::minutes(4)));
        LOK_ASSERT_EQUAL(first, token.getCurrent());
        LOK_ASSERT(token.rotateIfNeeded(start + std::chrono::minutes(9)));
        LOK_ASSERT(token.matches(first));
        LOK_ASSERT(first != token.getCurrent());
        // The rotation at the check after 9 minutes counts from the end of the first interval, so
        // the next one comes after 10 minutes:
        LOK_ASSERT(token.rotateIfNeeded(start + std::chrono::minutes(11)));
        LOK_ASSERT(!token.matches(first));
    }

    void testInvalidAfterAPause() {
        constexpr std::string_view testname = __func__;
        RotatingToken token(16, std::chrono::minutes(5));
        auto const start = std::chrono::steady_clock::now();
        std::string const first = token.getCurrent();
        std::string const previous = token.getPrevious();
        LOK_ASSERT(token.rotateIfNeeded(start + std::chrono::minutes(12)));
        LOK_ASSERT(!token.matches(first));
        LOK_ASSERT(!token.matches(previous));
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(RotatingTokenTests);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
