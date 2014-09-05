/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/types.h>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <valgrind/callgrind.h>

#include <TestAutoCorrDoc.hxx>
#include <editeng/svxacorr.hxx>

namespace {

class Test : public CppUnit::TestFixture
{
    void testAutocorrect();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testAutocorrect);
    CPPUNIT_TEST_SUITE_END();
};

void Test::testAutocorrect()
{
    OUString sShareAutocorrFile;
    OUString sUserAutocorrFile;
    SvxAutoCorrect aAutoCorrect(sShareAutocorrFile, sUserAutocorrFile);
    {
        OUString sInput("TEst-TEst");
        sal_Unicode cNextChar(' ');
        OUString sExpected("Test-Test ");

        CALLGRIND_START_INSTRUMENTATION;
        CALLGRIND_ZERO_STATS;
        TestAutoCorrDoc aFoo(sInput, LANGUAGE_ENGLISH_US);
        aAutoCorrect.DoAutoCorrect(aFoo, sInput, sInput.getLength(), cNextChar, true);
        CALLGRIND_STOP_INSTRUMENTATION;

        CPPUNIT_ASSERT_MESSAGE("autocorrect", aFoo.getResult() == sExpected);
    }
    CALLGRIND_DUMP_STATS;
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
