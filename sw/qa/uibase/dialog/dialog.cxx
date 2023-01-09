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
        : SwModelTestBase("/sw/qa/uibase/dialog/data/")
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
            "RegionName", uno::Any(OUString("ZOTERO_BIBL {} CSL_BIBLIOGRAPHY RNDRfiit6mXBc"))),
        comphelper::makePropertyValue("Content", uno::Any(OUString("<p>aaa</p><p>bbb</p>"))),
    };
    dispatchCommand(mxComponent, ".uno:InsertSection", aArgs);

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
    CPPUNIT_ASSERT_EQUAL(OUString("aaa\nbbb"), aActualResult);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
