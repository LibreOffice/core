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

// FIXME: dialog doesn't pop up on macos and doesn't close on win32...
#if !defined(_WIN32) && !defined(MACOSX)
CPPUNIT_TEST_FIXTURE(test::SwAccessibleTestBase, BasicTestHyperlinkDialog)
{
    load(u"private:factory/swriter");

    auto dialogWaiter = awaitDialog(u"Hyperlink", [this](Dialog& dialog) {
        dumpA11YTree(dialog.getAccessible()->getAccessibleContext());

        // Focus the URL box (should be default, but make sure we're on it)
        CPPUNIT_ASSERT(dialog.tabTo(accessibility::AccessibleRole::COMBO_BOX, u"URL:"));
        // Fill in an address
        dialog.postExtTextEventAsync(u"https://libreoffice.org/");
        // Validate the whole dialog
        dialog.postKeyEventAsync(0, awt::Key::RETURN);
        Scheduler::ProcessEventsToIdle();
    });

    // Activate the Insert->Hyperlink... menu item to open the Hyperlink dialog
    CPPUNIT_ASSERT(activateMenuItem(u"Insert", u"Hyperlink..."));
    CPPUNIT_ASSERT(dialogWaiter->waitEndDialog());

    CPPUNIT_ASSERT_EQUAL(rtl::OUString("<PARAGRAPH>https://libreoffice.org/</PARAGRAPH>"),
                         collectText());
}
#endif

// FIXME: dialog doesn't pop up on macos and doesn't close on win32...
#if !defined(_WIN32) && !defined(MACOSX)
CPPUNIT_TEST_FIXTURE(test::SwAccessibleTestBase, BasicTestBookmarkDialog)
{
    load(u"private:factory/swriter");

    auto dialogWaiter = awaitDialog(u"Bookmark", [this](Dialog& dialog) {
        dumpA11YTree(dialog.getAccessible()->getAccessibleContext());

        CPPUNIT_ASSERT(dialog.tabTo(accessibility::AccessibleRole::TEXT, u"Name:"));
        dialog.postKeyEventAsync(0, awt::Key::SELECT_ALL);
        dialog.postKeyEventAsync(0, awt::Key::DELETE);
        dialog.postExtTextEventAsync(u"Test Bookmark 1");
        // Validate the whole dialog
        dialog.postKeyEventAsync(0, awt::Key::RETURN);
        Scheduler::ProcessEventsToIdle();
    });

    // Activate the Insert->Bookmark... menu item to open the Bookmark dialog
    CPPUNIT_ASSERT(activateMenuItem(u"Insert", u"Bookmark..."));
    CPPUNIT_ASSERT(dialogWaiter->waitEndDialog());

    CPPUNIT_ASSERT_EQUAL(rtl::OUString("<PARAGRAPH>#Test Bookmark 1 Bookmark </PARAGRAPH>"),
                         collectText());
}
#endif

// FIXME: dialog doesn't pop up on macos and doesn't close on win32...
#if !defined(_WIN32) && !defined(MACOSX)
CPPUNIT_TEST_FIXTURE(test::SwAccessibleTestBase, BasicTestSectionDialog)
{
    load(u"private:factory/swriter");

    auto dialogWaiter = awaitDialog(u"Insert Section", [this](Dialog& dialog) {
        dumpA11YTree(dialog.getAccessible()->getAccessibleContext());

        // Validate the whole dialog
        dialog.postKeyEventAsync(0, awt::Key::RETURN);
        Scheduler::ProcessEventsToIdle();
    });

    CPPUNIT_ASSERT(activateMenuItem(u"Insert", u"Section..."));
    CPPUNIT_ASSERT(dialogWaiter->waitEndDialog());

    CPPUNIT_ASSERT_EQUAL(rtl::OUString("<PARAGRAPH/><PARAGRAPH/>"), collectText());
}
#endif

// FIXME: dialog doesn't pop up on macos and doesn't close on win32...
#if !defined(_WIN32) && !defined(MACOSX)
CPPUNIT_TEST_FIXTURE(test::SwAccessibleTestBase, BasicTestFontworkDialog)
{
    load(u"private:factory/swriter");

    auto dialogWaiter = awaitDialog(u"Fontwork Gallery", [this](Dialog& dialog) {
        dumpA11YTree(dialog.getAccessible()->getAccessibleContext());

        // Validate the whole dialog
        dialog.postKeyEventAsync(0, awt::Key::RETURN);
        Scheduler::ProcessEventsToIdle();
    });

    CPPUNIT_ASSERT(activateMenuItem(u"Insert", u"Fontwork..."));
    CPPUNIT_ASSERT(dialogWaiter->waitEndDialog());

    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString("<PARAGRAPH/><SHAPE name=\"Simple\" description=\" \"><PARAGRAPH "
                      "description=\"Paragraph: 0 Simple\">Simple</PARAGRAPH></SHAPE>"),
        collectText());
}
#endif

// FIXME: dialog doesn't pop up on macos and doesn't close on win32...
#if !defined(_WIN32) && !defined(MACOSX)
CPPUNIT_TEST_FIXTURE(test::SwAccessibleTestBase, BasicTestFrameDialog)
{
    load(u"private:factory/swriter");

    auto dialogWaiter = awaitDialog(u"Frame", [](Dialog& dialog) {
        // Validate the whole dialog
        dialog.postKeyEventAsync(0, awt::Key::RETURN);
        Scheduler::ProcessEventsToIdle();
    });

    CPPUNIT_ASSERT(activateMenuItem(u"Insert", u"Frame", u"Frame..."));
    CPPUNIT_ASSERT(dialogWaiter->waitEndDialog());

    CPPUNIT_ASSERT_EQUAL(
        rtl::OUString("<PARAGRAPH/><TEXT_FRAME name=\"Frame1\"><PARAGRAPH/></TEXT_FRAME>"),
        collectText());
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
