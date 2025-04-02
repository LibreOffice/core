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
#include <com/sun/star/accessibility/XAccessibleContext2.hpp>
#include <com/sun/star/linguistic2/LinguServiceManager.hpp>
#include <com/sun/star/linguistic2/XLinguServiceManager2.hpp>
#include <com/sun/star/linguistic2/XSpellChecker1.hpp>

#include <vcl/scheduler.hxx>

#include <test/a11y/accessibletestbase.hxx>
#include <test/a11y/AccessibilityTools.hxx>

using namespace css;
using namespace css::accessibility;

// FIXME: dialog API doesn't work on macos yet
#if !defined(MACOSX)

CPPUNIT_TEST_FIXTURE(test::AccessibleTestBase, BasicTestSpecialCharactersDialog)
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
CPPUNIT_TEST_FIXTURE(test::AccessibleTestBase, TestSpecialCharactersDialogFocus)
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

CPPUNIT_TEST_FIXTURE(test::AccessibleTestBase, BasicTestHyperlinkDialog)
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

CPPUNIT_TEST_FIXTURE(test::AccessibleTestBase, BasicTestBookmarkDialog)
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

CPPUNIT_TEST_FIXTURE(test::AccessibleTestBase, BasicTestSectionDialog)
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

CPPUNIT_TEST_FIXTURE(test::AccessibleTestBase, BasicTestFontworkDialog)
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

    CPPUNIT_ASSERT_EQUAL(
        u"<PARAGRAPH/><SHAPE name=\"Fontwork 1\"><PARAGRAPH>Simple</PARAGRAPH></SHAPE>"_ustr,
        collectText());
}

CPPUNIT_TEST_FIXTURE(test::AccessibleTestBase, BasicTestFrameDialog)
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

/* Verify that UI elements in the spell check dialog have the accessible IDs
 * set that that Orca screen reader's logic to identify them depends on,
 * see tdf#155447 and following Orca commits:
 *
 * https://gitlab.gnome.org/GNOME/orca/-/commit/6221f4ecf542646a80e47ee7236380360f0e1a85
 * https://gitlab.gnome.org/GNOME/orca/-/commit/40a2d302eb52295433fd84e6c254a7dbe5108a24
 *
 * (Changes should be discussed with the Orca maintainer first.)
 *
 * While the Orca logic depends only on case-insensitive name starting
 * with a certain string, this test uses the full accessible ID
 * (which matches the GtkBuilder ID in ./cui/uiconfig/ui/spellingdialog.ui)
 * in order to identify the elements.
 */
CPPUNIT_TEST_FIXTURE(test::AccessibleTestBase, SpellingDialog)
{
    // spell check depends on dictionary being available, so skip test if unavailable
    uno::Reference<linguistic2::XLinguServiceManager2> xLSM2
        = linguistic2::LinguServiceManager::create(m_xContext);
    uno::Reference<linguistic2::XSpellChecker1> xSpell(xLSM2->getSpellChecker(), uno::UNO_QUERY);
    if (!xSpell.is() || !xSpell->hasLanguage(static_cast<sal_uInt16>(LANGUAGE_ENGLISH_US)))
        return;

    loadFromSrc(u"/sw/qa/extras/accessibility/testdocuments/tdf155705.fodt"_ustr);

    auto dialogWaiter = awaitDialog(u"Spelling: English (USA)", [](Dialog& dialog) {
        uno::Reference<XAccessible> xDialogAcc = dialog.getAccessible();

        uno::Reference<XAccessibleContext2> xDialogContext(xDialogAcc->getAccessibleContext(),
                                                           uno::UNO_QUERY);
        CPPUNIT_ASSERT(xDialogContext.is());
        CPPUNIT_ASSERT_EQUAL(u"SpellingDialog"_ustr, xDialogContext->getAccessibleId());

        uno::Reference<XAccessibleContext> xSentenceAcc
            = AccessibilityTools::getAccessibleObjectForId(xDialogContext, u"errorsentence");
        CPPUNIT_ASSERT(xSentenceAcc.is());

        uno::Reference<XAccessibleContext> xSuggestionsAcc
            = AccessibilityTools::getAccessibleObjectForId(xDialogContext, u"suggestionslb");
        CPPUNIT_ASSERT(xSuggestionsAcc.is());

        CPPUNIT_ASSERT(dialog.tabTo(accessibility::AccessibleRole::PUSH_BUTTON, u"Close"));
        dialog.postKeyEventAsync(0, awt::Key::RETURN);
        Scheduler::ProcessEventsToIdle();
    });

    CPPUNIT_ASSERT(activateMenuItem(u"Tools", u"Spelling..."));
    CPPUNIT_ASSERT(dialogWaiter->waitEndDialog());
}

#endif //defined(MACOSX)

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
