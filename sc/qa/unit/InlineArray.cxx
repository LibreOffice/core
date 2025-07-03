/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "helper/qahelper.hxx" // contains class ScModelTestBase

#include <sfx2/dispatch.hxx>

using namespace css;

class ScInlineArray : public ScModelTestBase
{
public:
    ScInlineArray()
        : ScModelTestBase(u"sc/qa/unit/data"_ustr)
    {
    }
};

/*  These tests cover cases that use inline arrays. The actual visible separators depend on user
    settings. The default of these settings depend on local. The markup in the ODF file uses the
    semicolon as column separator and the pipe symbol as row separator. These separators are used
    in below comments.*/

CPPUNIT_TEST_FIXTURE(ScInlineArray, testLOOKUP_special)
{
    // LOOKUP has some special rules. These are tested here.

    // Load a document with test cases and force recalculation.
    createScDoc("ods/LOOKUP_inlineArray.ods");
    dispatchCommand(mxComponent, u".uno:CalculateHard"_ustr, {});
    ScDocument* pDoc = getScDoc();

    // Test results. Explanations can be found in the test file.
    CPPUNIT_ASSERT_EQUAL(u"Y"_ustr, pDoc->GetString(ScAddress(1, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(u"#N/A"_ustr, pDoc->GetString(ScAddress(1, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(u"Z"_ustr, pDoc->GetString(ScAddress(1, 5, 0)));
    CPPUNIT_ASSERT_EQUAL(u"10"_ustr, pDoc->GetString(ScAddress(1, 6, 0)));
    CPPUNIT_ASSERT_EQUAL(u"#N/A"_ustr, pDoc->GetString(ScAddress(1, 8, 0)));
    CPPUNIT_ASSERT_EQUAL(u"200"_ustr, pDoc->GetString(ScAddress(1, 11, 0)));
    CPPUNIT_ASSERT_EQUAL(u"200"_ustr, pDoc->GetString(ScAddress(1, 12, 0)));
    CPPUNIT_ASSERT_EQUAL(u"6"_ustr, pDoc->GetString(ScAddress(1, 13, 0)));
}

CPPUNIT_TEST_FIXTURE(ScInlineArray, testTdf67134)
{
    // Search and result array in LOOKUP need not have the same direction.
    // This also applies to inline arrays.
    // Before the fix the #N/A error was returned in case of different directions.

    // Load a document with test cases and force recalculation.
    createScDoc("ods/tdf167134_LOOKUP_inlineArray.ods");
    dispatchCommand(mxComponent, u".uno:CalculateHard"_ustr, {});
    ScDocument* pDoc = getScDoc();

    // A2: string "D"
    // B2: formula =LOOKUP(A2;{"B";"C";"D"};{"X"|"Y"|"Z"})
    CPPUNIT_ASSERT_EQUAL(u"Z"_ustr, pDoc->GetString(ScAddress(1, 1, 0)));
    // B3: formula =LOOkUP(A2;{"B"|"C"|"D"};{"X";"Y";"Z"})
    CPPUNIT_ASSERT_EQUAL(u"Z"_ustr, pDoc->GetString(ScAddress(1, 2, 0)));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
