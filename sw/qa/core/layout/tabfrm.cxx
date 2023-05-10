/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

/// Covers sw/source/core/layout/tabfrm.cxx fixes.
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase("/sw/qa/core/layout/data/")
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testTablePrintAreaLeft)
{
    // Given a document with a header containing an image, and also with an overlapping table:
    createSwDoc("table-print-area-left.docx");

    // When laying out that document & parsing the left margin of the table:
    SwTwips nTablePrintLeft = parseDump("//tab/infos/prtBounds", "left").toInt32();

    // Then make sure it has ~no left margin:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 5
    // - Actual  : 10646
    // i.e. the table was shifted outside the page, was invisible.
    CPPUNIT_ASSERT_EQUAL(static_cast<SwTwips>(5), nTablePrintLeft);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
