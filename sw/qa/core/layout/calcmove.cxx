/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <test/xmldocptr.hxx>

namespace
{
/// Covers sw/source/core/layout/calcmove.cxx fixes.
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase("/sw/qa/core/layout/data/")
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testIgnoreTopMargin)
{
    // Given a DOCX (>= Word 2013) file, with 2 pages:
    // When loading that document:
    createSwDoc("ignore-top-margin.docx");

    // Then make sure that the paragraph on the 2nd page has no top margin:
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    sal_Int32 nParaTopMargin
        = getXPath(pXmlDoc, "/root/page[2]/body/txt/infos/prtBounds"_ostr, "top"_ostr).toInt32();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0
    // - Actual  : 2400
    // i.e. the top margin in the first para of a non-first page wasn't ignored, like in Word.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), nParaTopMargin);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
