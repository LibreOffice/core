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
 * Unit tests for the restricted and locked command lists.
 */

#include <config.h>

#include <test/lokassert.hpp>

#include <common/CommandControl.hpp>

#include <cppunit/extensions/HelperMacros.h>

#include <string>
#include <string_view>

/// CommandControl unit-tests.
class CommandControlTests : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(CommandControlTests);

    CPPUNIT_TEST(testTrackChangesBlocksAllRecordingCommands);
    CPPUNIT_TEST(testUnrelatedListStaysTheSame);

    CPPUNIT_TEST_SUITE_END();

    void testTrackChangesBlocksAllRecordingCommands();
    void testUnrelatedListStaysTheSame();
};

void CommandControlTests::testTrackChangesBlocksAllRecordingCommands()
{
    constexpr std::string_view testname = __func__;

    // Listing .uno:TrackChanges also lists the commands that switch recording for one view or for
    // all views.
    LOK_ASSERT_EQUAL(
        std::string(
            ".uno:Bold .uno:TrackChanges .uno:TrackChangesInThisView .uno:TrackChangesInAllViews"),
        CommandControl::addRelatedCommands(".uno:Bold .uno:TrackChanges"));
}

void CommandControlTests::testUnrelatedListStaysTheSame()
{
    constexpr std::string_view testname = __func__;

    // A list without a command that has related commands is unchanged, and so is an empty list.
    LOK_ASSERT_EQUAL(std::string(".uno:Bold .uno:Italic"),
                     CommandControl::addRelatedCommands(".uno:Bold .uno:Italic"));
    LOK_ASSERT_EQUAL(std::string(), CommandControl::addRelatedCommands(std::string()));
    // A command whose name starts with .uno:TrackChanges is not .uno:TrackChanges itself.
    LOK_ASSERT_EQUAL(std::string(".uno:TrackChangesBar"),
                     CommandControl::addRelatedCommands(".uno:TrackChangesBar"));
}

CPPUNIT_TEST_SUITE_REGISTRATION(CommandControlTests);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
