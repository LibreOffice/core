/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/awt/Key.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>

#include <vcl/scheduler.hxx>

#include <test/a11y/swaccessibletestbase.hxx>
#include <test/a11y/AccessibilityTools.hxx>

using namespace css;

// FIXME: dialog API doesn't work on macos yet
#if !defined(MACOSX)

CPPUNIT_TEST_FIXTURE(test::SwAccessibleTestBase, BasicTestSpecialCharactersDialog)
{
    load(u"private:factory/swriter"_ustr);

    auto dialogWaiter = awaitDialog(u"Special Characters", [this](Dialog& dialog) {
        dumpA11YTree(dialog.getAccessible()->getAccessibleContext());

        CPPUNIT_ASSERT_EQUAL(
            AccessibilityTools::getAccessibleObjectForName(
                dialog.getAccessible(), accessibility::AccessibleRole::TEXT, u"Search:"),
            getFocusedObject(dialog.getAccessible()));

        // search for (c) symbol
        dialog.postExtTextEventAsync(u"copyright"_ustr);
        Scheduler::ProcessEventsToIdle();

        CPPUNIT_ASSERT(dialog.tabTo(accessibility::AccessibleRole::TABLE_CELL, u"©"));

        /* there was a focus issue in this dialog: the table holding the characters always had the
         * selected element as focused, even when tabbing outside.
         * Fixed with https://gerrit.libreoffice.org/c/core/+/147660.
         * Anyway, we still use the target element match API to also exercise it. */
        auto xChild = AccessibilityTools::getAccessibleObjectForName(
            dialog.getAccessible(), accessibility::AccessibleRole::PUSH_BUTTON, u"Insert");
        CPPUNIT_ASSERT(xChild);
        CPPUNIT_ASSERT(dialog.tabTo(xChild));
        dialog.postKeyEventAsync(0, awt::Key::RETURN);

        Scheduler::ProcessEventsToIdle();
    });

    CPPUNIT_ASSERT(activateMenuItem(u"Insert", u"Special Character..."));
    CPPUNIT_ASSERT(dialogWaiter->waitEndDialog());

    CPPUNIT_ASSERT_EQUAL(u"<PARAGRAPH>©</PARAGRAPH>"_ustr, collectText());
}

/* checks for the fix from https://gerrit.libreoffice.org/c/core/+/147660 */
CPPUNIT_TEST_FIXTURE(test::SwAccessibleTestBase, TestSpecialCharactersDialogFocus)
{
    load(u"private:factory/swriter"_ustr);

    auto dialogWaiter = awaitDialog(u"Special Characters", [](Dialog& dialog) {
        CPPUNIT_ASSERT(dialog.tabTo(accessibility::AccessibleRole::TABLE_CELL, u" "));

        /* as there is a bug that focusing the character table doesn't enable the Insert button
         * (https://bugs.documentfoundation.org/show_bug.cgi?id=153806), we move to another cell
         * so it works. */

        // tdf#153918: Check that '!' char has correct accessible name and insert it
        dialog.postKeyEventAsync(0, awt::Key::RIGHT);
        Scheduler::ProcessEventsToIdle();
        CPPUNIT_ASSERT_EQUAL(
            AccessibilityTools::getAccessibleObjectForName(
                dialog.getAccessible(), accessibility::AccessibleRole::TABLE_CELL, u"!"),
            getFocusedObject(dialog.getAccessible()));

        CPPUNIT_ASSERT(dialog.tabTo(accessibility::AccessibleRole::PUSH_BUTTON, u"Insert"));
        dialog.postKeyEventAsync(0, awt::Key::RETURN);

        Scheduler::ProcessEventsToIdle();
    });

    CPPUNIT_ASSERT(activateMenuItem(u"Insert", u"Special Character..."));
    CPPUNIT_ASSERT(dialogWaiter->waitEndDialog());

    CPPUNIT_ASSERT_EQUAL(u"<PARAGRAPH>!</PARAGRAPH>"_ustr, collectText());
}

CPPUNIT_TEST_FIXTURE(test::SwAccessibleTestBase, BasicTestHyperlinkDialog)
{
    load(u"private:factory/swriter"_ustr);

    auto dialogWaiter = awaitDialog(u"Hyperlink", [this](Dialog& dialog) {
        dumpA11YTree(dialog.getAccessible()->getAccessibleContext());

        // Focus the URL box (should be default, but make sure we're on it)
        CPPUNIT_ASSERT(dialog.tabTo(accessibility::AccessibleRole::COMBO_BOX, u"URL:"));
        // Fill in an address
        dialog.postExtTextEventAsync(u"https://libreoffice.org/"_ustr);
        // Validate the whole dialog
        dialog.postKeyEventAsync(0, awt::Key::RETURN);
        Scheduler::ProcessEventsToIdle();
    });

    // Activate the Insert->Hyperlink... menu item to open the Hyperlink dialog
    CPPUNIT_ASSERT(activateMenuItem(u"Insert", u"Hyperlink..."));
    CPPUNIT_ASSERT(dialogWaiter->waitEndDialog());

    CPPUNIT_ASSERT_EQUAL(u"<PARAGRAPH>https://libreoffice.org/</PARAGRAPH>"_ustr, collectText());
}

CPPUNIT_TEST_FIXTURE(test::SwAccessibleTestBase, BasicTestBookmarkDialog)
{
    load(u"private:factory/swriter"_ustr);

    auto dialogWaiter = awaitDialog(u"Bookmark", [this](Dialog& dialog) {
        dumpA11YTree(dialog.getAccessible()->getAccessibleContext());

        CPPUNIT_ASSERT(dialog.tabTo(accessibility::AccessibleRole::TEXT, u"Name:"));
        dialog.postKeyEventAsync(0, awt::Key::SELECT_ALL);
        dialog.postKeyEventAsync(0, awt::Key::DELETE);
        dialog.postExtTextEventAsync(u"Test Bookmark 1"_ustr);
        // Validate the whole dialog
        dialog.postKeyEventAsync(0, awt::Key::RETURN);
        Scheduler::ProcessEventsToIdle();
    });

    // Activate the Insert->Bookmark... menu item to open the Bookmark dialog
    CPPUNIT_ASSERT(activateMenuItem(u"Insert", u"Bookmark..."));
    CPPUNIT_ASSERT(dialogWaiter->waitEndDialog());

    CPPUNIT_ASSERT_EQUAL(u"<PARAGRAPH>#Test Bookmark 1 Bookmark </PARAGRAPH>"_ustr, collectText());
}

CPPUNIT_TEST_FIXTURE(test::SwAccessibleTestBase, BasicTestSectionDialog)
{
    load(u"private:factory/swriter"_ustr);

    auto dialogWaiter = awaitDialog(u"Insert Section", [this](Dialog& dialog) {
        dumpA11YTree(dialog.getAccessible()->getAccessibleContext());

        // Validate the whole dialog
        dialog.postKeyEventAsync(0, awt::Key::RETURN);
        Scheduler::ProcessEventsToIdle();
    });

    CPPUNIT_ASSERT(activateMenuItem(u"Insert", u"Section..."));
    CPPUNIT_ASSERT(dialogWaiter->waitEndDialog());

    CPPUNIT_ASSERT_EQUAL(u"<PARAGRAPH/><PARAGRAPH/>"_ustr, collectText());
}

CPPUNIT_TEST_FIXTURE(test::SwAccessibleTestBase, BasicTestFontworkDialog)
{
    load(u"private:factory/swriter"_ustr);

    auto dialogWaiter = awaitDialog(u"Fontwork Gallery", [this](Dialog& dialog) {
        dumpA11YTree(dialog.getAccessible()->getAccessibleContext());

        // Validate the whole dialog
        dialog.postKeyEventAsync(0, awt::Key::RETURN);
        Scheduler::ProcessEventsToIdle();
    });

    CPPUNIT_ASSERT(activateMenuItem(u"Insert", u"Fontwork..."));
    CPPUNIT_ASSERT(dialogWaiter->waitEndDialog());

    CPPUNIT_ASSERT_EQUAL(u"<PARAGRAPH/><SHAPE name=\"Fontwork 1\" description=\" \"><PARAGRAPH "
                         "description=\"Paragraph: 0 Simple\">Simple</PARAGRAPH></SHAPE>"_ustr,
                         collectText());
}

CPPUNIT_TEST_FIXTURE(test::SwAccessibleTestBase, BasicTestFrameDialog)
{
    load(u"private:factory/swriter"_ustr);

    auto dialogWaiter = awaitDialog(u"Frame", [](Dialog& dialog) {
        // Validate the whole dialog
        dialog.postKeyEventAsync(0, awt::Key::RETURN);
        Scheduler::ProcessEventsToIdle();
    });

    CPPUNIT_ASSERT(activateMenuItem(u"Insert", u"Frame", u"Frame..."));
    CPPUNIT_ASSERT(dialogWaiter->waitEndDialog());

    CPPUNIT_ASSERT_EQUAL(u"<PARAGRAPH/><TEXT_FRAME name=\"Frame1\"><PARAGRAPH/></TEXT_FRAME>"_ustr,
                         collectText());
}

#endif //defined(MACOSX)

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
