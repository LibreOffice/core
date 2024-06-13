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

namespace
{
/// Covers sw/source/uibase/dialog/ fixes.
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/uibase/dialog/data/"_ustr)
    {
    }
};
}

CPPUNIT_TEST_FIXTURE(Test, testInsertSection)
{
    // Given an empty document:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();

    // When inserting a section with text:
    uno::Sequence<css::beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue(
            u"RegionName"_ustr, uno::Any(u"ZOTERO_BIBL {} CSL_BIBLIOGRAPHY RNDRfiit6mXBc"_ustr)),
        comphelper::makePropertyValue(u"Content"_ustr, uno::Any(u"<p>aaa</p><p>bbb</p>"_ustr)),
    };
    dispatchCommand(mxComponent, u".uno:InsertSection"_ustr, aArgs);

    // Then make sure that we created a section that covers that text:
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->SttEndDoc(/*bStt=*/true);
    pWrtShell->EndOfSection(/*bSelect=*/true);
    SwCursor* pCursor = pWrtShell->GetCursor();
    OUString aActualResult = pCursor->GetText();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: aaa\nbbb
    // - Actual  :
    // i.e. the value of the Content parameter was ignored.
    CPPUNIT_ASSERT_EQUAL(u"aaa\nbbb"_ustr, aActualResult);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
