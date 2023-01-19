/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <comphelper/propertyvalue.hxx>

#include <wrtsh.hxx>
#include <docsh.hxx>
#include <ftnidx.hxx>
#include <txtftn.hxx>

namespace
{
/// Covers sw/source/uibase/shells/textfld.cxx fixes.
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase("/sw/qa/uibase/shells/data/")
    {
    }
};
}

CPPUNIT_TEST_FIXTURE(Test, testInsertRefmarkFootnote)
{
    // Given an empty document:
    createSwDoc();

    // When inserting a refmark inside a footnote:
    uno::Sequence<css::beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue("TypeName", uno::Any(OUString("SetRef"))),
        comphelper::makePropertyValue("Name", uno::Any(OUString("myref"))),
        comphelper::makePropertyValue("Content", uno::Any(OUString("content"))),
        comphelper::makePropertyValue("Wrapper", uno::Any(OUString("Footnote"))),
    };
    dispatchCommand(mxComponent, ".uno:InsertField", aArgs);

    // Then make sure that the note body contains the refmark:
    SwDoc* pDoc = getSwDoc();
    SwFootnoteIdxs& rNotes = pDoc->GetFootnoteIdxs();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // i.e. no note was inserted.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rNotes.size());
    SwTextFootnote* pNote = rNotes[0];
    const SwFormatFootnote& rFormatNote = pNote->GetFootnote();
    CPPUNIT_ASSERT(!rFormatNote.IsEndNote());
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    CPPUNIT_ASSERT_EQUAL(OUString("content"), rFormatNote.GetFootnoteText(*pWrtShell->GetLayout()));
}

CPPUNIT_TEST_FIXTURE(Test, testInsertRefmarkEndnote)
{
    // Given an empty document:
    createSwDoc();

    // When inserting a refmark inside an endnote:
    uno::Sequence<css::beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue("TypeName", uno::Any(OUString("SetRef"))),
        comphelper::makePropertyValue("Name", uno::Any(OUString("myref"))),
        comphelper::makePropertyValue("Content", uno::Any(OUString("content"))),
        comphelper::makePropertyValue("Wrapper", uno::Any(OUString("Endnote"))),
    };
    dispatchCommand(mxComponent, ".uno:InsertField", aArgs);

    // Then make sure that the note body contains the refmark:
    SwDoc* pDoc = getSwDoc();
    SwFootnoteIdxs& rNotes = pDoc->GetFootnoteIdxs();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // i.e. no endnote was inserted.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rNotes.size());
    SwTextFootnote* pNote = rNotes[0];
    const SwFormatFootnote& rNote = pNote->GetFootnote();
    CPPUNIT_ASSERT(rNote.IsEndNote());
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    CPPUNIT_ASSERT_EQUAL(OUString("content"), rNote.GetFootnoteText(*pWrtShell->GetLayout()));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
