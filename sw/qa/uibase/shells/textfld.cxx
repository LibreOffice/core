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
        : SwModelTestBase(u"/sw/qa/uibase/shells/data/"_ustr)
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
        comphelper::makePropertyValue(u"TypeName"_ustr, uno::Any(u"SetRef"_ustr)),
        comphelper::makePropertyValue(u"Name"_ustr, uno::Any(u"myref"_ustr)),
        comphelper::makePropertyValue(u"Content"_ustr, uno::Any(u"content"_ustr)),
        comphelper::makePropertyValue(u"Wrapper"_ustr, uno::Any(u"Footnote"_ustr)),
    };
    dispatchCommand(mxComponent, u".uno:InsertField"_ustr, aArgs);

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
    CPPUNIT_ASSERT_EQUAL(u"content"_ustr, rFormatNote.GetFootnoteText(*pWrtShell->GetLayout()));
}

CPPUNIT_TEST_FIXTURE(Test, testInsertHyperlinkKeepSelection)
{
    // Given a document with "foo" selected:
    createSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->Insert2(u"foo"_ustr);
    pWrtShell->SelAll();

    // When inserting a hyperlink whose Hyperlink.Text differs from the selection:
    uno::Sequence<css::beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue(u"Hyperlink.Text"_ustr, uno::Any(u"mytext"_ustr)),
        comphelper::makePropertyValue(u"Hyperlink.TextIsHint"_ustr, uno::Any(true)),
        comphelper::makePropertyValue(u"Hyperlink.URL"_ustr,
                                      uno::Any(u"http://www.example.com"_ustr)),
    };
    dispatchCommand(mxComponent, u".uno:SetHyperlink"_ustr, aArgs);

    // Then make sure the selected text is preserved as the hyperlink text:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: foo
    // - Actual  : mytext
    // i.e. the selected text was lost, the provided text hint was used, which is not wanted.
    CPPUNIT_ASSERT_EQUAL(u"foo"_ustr, pWrtShell->GetSelText());
}

CPPUNIT_TEST_FIXTURE(Test, testInsertRefmarkEndnote)
{
    // Given an empty document:
    createSwDoc();

    // When inserting a refmark inside an endnote:
    uno::Sequence<css::beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue(u"TypeName"_ustr, uno::Any(u"SetRef"_ustr)),
        comphelper::makePropertyValue(u"Name"_ustr, uno::Any(u"myref"_ustr)),
        comphelper::makePropertyValue(u"Content"_ustr, uno::Any(u"content"_ustr)),
        comphelper::makePropertyValue(u"Wrapper"_ustr, uno::Any(u"Endnote"_ustr)),
    };
    dispatchCommand(mxComponent, u".uno:InsertField"_ustr, aArgs);

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
    CPPUNIT_ASSERT_EQUAL(u"content"_ustr, rNote.GetFootnoteText(*pWrtShell->GetLayout()));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
