/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/a11y/accessibletestbase.hxx>

// FIXME: dialog API doesn't work on macos yet
// and this test crashes on some jenkins Windows boxes
#if !defined(MACOSX) && !defined(_WIN32)

/* Checks an unexpected dialog opening (instead of the expected one) is properly caught, as it would
 * otherwise block the test potentially indefinitely */
CPPUNIT_TEST_FIXTURE(test::AccessibleTestBase, SelfTestIncorrectDialog)
{
    load(u"private:factory/swriter"_ustr);

    auto dialogWaiter = awaitDialog(u"This Dialog Does Not Exist", [](Dialog&) {
        CPPUNIT_ASSERT_MESSAGE("This code should not be reached", false);
    });

    CPPUNIT_ASSERT(activateMenuItem(u"Insert", u"Section..."));
    /* Make sure an incorrect dialog popping up is caught and raises.  The exception is thrown in
     * waitEndDialog() for consistency even though the error itself is likely to have been triggered
     * by the activateMenuItem() call above */
    CPPUNIT_ASSERT_THROW(dialogWaiter->waitEndDialog(), css::uno::RuntimeException);
}

/* Checks that an exception in the dialog callback code is properly handled and won't disturb
 * subsequent tests if caught -- especially that DialogWaiter::waitEndDialog() won't timeout. */
CPPUNIT_TEST_FIXTURE(test::AccessibleTestBase, SelfTestThrowInDialogCallback)
{
    load(u"private:factory/swriter"_ustr);

    class DummyException : public std::exception
    {
    };

    auto dialogWaiter = awaitDialog(u"Hyperlink", [](Dialog&) { throw DummyException(); });

    CPPUNIT_ASSERT(activateMenuItem(u"Insert", u"Hyperlink..."));
    CPPUNIT_ASSERT_THROW(dialogWaiter->waitEndDialog(), DummyException);
}

// Checks timeout if dialog does not show up as expected
CPPUNIT_TEST_FIXTURE(test::AccessibleTestBase, SelfTestNoDialog)
{
    load(u"private:factory/swriter"_ustr);

    auto dialogWaiter = awaitDialog(u"This Dialog Did Not Show Up", [](Dialog&) {
        CPPUNIT_ASSERT_MESSAGE("This code should not be reached", false);
    });

    // as we don't actually call any dialog up, this should fail after a timeout
    CPPUNIT_ASSERT(!dialogWaiter->waitEndDialog());
}

#endif //defined(MACOSX)

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
