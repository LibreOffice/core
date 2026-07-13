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

#include <kit/KitWebSocket.hpp>

#include <test/lokassert.hpp>

#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>

#include <string>
#include <string_view>

/// Unit-tests for how the parent classifies jsdialog messages that arrive
/// from the background save child process.
class BgSaveWhiteBoxTests : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(BgSaveWhiteBoxTests);
    CPPUNIT_TEST(testQuickFindDoesNotFailSave);
    CPPUNIT_TEST(testToolbarNotificationsDoNotFailSave);
    CPPUNIT_TEST(testInteractiveDialogFailsSave);
    CPPUNIT_TEST_SUITE_END();

    void testQuickFindDoesNotFailSave();
    void testToolbarNotificationsDoNotFailSave();
    void testInteractiveDialogFailsSave();
};

// A quickfind notification can arrive from the child while a background save
// is running. It is a search-panel update, not a prompt the user.
void BgSaveWhiteBoxTests::testQuickFindDoesNotFailSave()
{
    constexpr std::string_view testname = __func__;
    const std::string quickFind = "{ \"id\": 0, \"jsontype\": \"quickfind\"}";
    LOK_ASSERT_EQUAL(true,
                     BgSaveParentWebSocketHandler::isBenignBgSaveJSDialog(quickFind));
}

// The toolbar-shaped notifications and a closing dialog are leftover pre-fork
// messages, so they are dropped rather than failing the save.
void BgSaveWhiteBoxTests::testToolbarNotificationsDoNotFailSave()
{
    constexpr std::string_view testname = __func__;
    LOK_ASSERT_EQUAL(true, BgSaveParentWebSocketHandler::isBenignBgSaveJSDialog(
                               "{ \"jsontype\": \"notebookbar\"}"));
    LOK_ASSERT_EQUAL(true, BgSaveParentWebSocketHandler::isBenignBgSaveJSDialog(
                               "{ \"jsontype\": \"sidebar\"}"));
    LOK_ASSERT_EQUAL(true, BgSaveParentWebSocketHandler::isBenignBgSaveJSDialog(
                               "{ \"jsontype\": \"formulabar\"}"));
    LOK_ASSERT_EQUAL(true, BgSaveParentWebSocketHandler::isBenignBgSaveJSDialog(
                               "{ \"jsontype\": \"dialog\", \"action\": \"close\"}"));
}

// A dialog that opens is a real interactive prompt during the save, so it must
// still fail the background save. Unparseable payloads fail it too.
void BgSaveWhiteBoxTests::testInteractiveDialogFailsSave()
{
    constexpr std::string_view testname = __func__;
    LOK_ASSERT_EQUAL(false, BgSaveParentWebSocketHandler::isBenignBgSaveJSDialog(
                                "{ \"jsontype\": \"dialog\", \"action\": \"show\"}"));
    LOK_ASSERT_EQUAL(false,
                     BgSaveParentWebSocketHandler::isBenignBgSaveJSDialog("not json"));
}

CPPUNIT_TEST_SUITE_REGISTRATION(BgSaveWhiteBoxTests);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
