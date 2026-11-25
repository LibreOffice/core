/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>

#include <comphelper/propertyvalue.hxx>
#include <comphelper/scopeguard.hxx>

#include <docsh.hxx>
#include <doc.hxx>
#include <ndtxt.hxx>
#include <wrtsh.hxx>

using namespace css;
using namespace cpo;

namespace
{
/// Covers sw/source/core/txtnode/ndtxt.cxx fixes.
class SwCoreNdtxtTest : public SwModelTestBase
{
public:
    SwCoreNdtxtTest()
        : SwModelTestBase(u"/sw/qa/core/txtnode/data/"_ustr)
    {
    }
};
}

CPPUNIT_TEST_FIXTURE(SwCoreNdtxtTest, testMergedPasteListLevelShift)
{
    // Given a Writer document whose "target" paragraph is at list level 1,
    // with the cursor on it:
    createSwDoc();
    SwDoc* pDoc = getSwDocShell()->GetDoc();
    {
        cpo::uno::Sequence<beans::PropertyValue> aTargetArgs
            = { comphelper::makePropertyValue(
                u"Name"_ustr, createFileURL(u"merged-paste-list-target.html")) };
        dispatchCommand(mxComponent, u".uno:InsertDoc"_ustr, aTargetArgs);
    }
    SwNodes& rNodes = pDoc->GetNodes();
    SwTextNode* pOrigDest = nullptr;
    for (SwNodeOffset i(0); i < rNodes.Count(); ++i)
    {
        SwTextNode* pCandidate = rNodes[i]->GetTextNode();
        if (pCandidate && pCandidate->GetText() == u"target")
        {
            pOrigDest = pCandidate;
            break;
        }
    }
    CPPUNIT_ASSERT(pOrigDest);
    CPPUNIT_ASSERT_EQUAL(1, pOrigDest->GetAttrListLevel());
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->GetCursor()->GetPoint()->Assign(*pOrigDest,
                                               pOrigDest->GetText().getLength());

    // When pasting a nested unordered list at that cursor position, with merged paste:
    pDoc->SetInMergedPaste(true);
    pDoc->SetMergedPasteOrigDestNode(pOrigDest);
    comphelper::ScopeGuard g([pDoc] {
        pDoc->SetInMergedPaste(false);
        pDoc->SetMergedPasteOrigDestNode(nullptr);
    });
    cpo::uno::Sequence<beans::PropertyValue> aArgs
        = { comphelper::makePropertyValue(u"Name"_ustr, createFileURL(u"merged-paste-list.html")) };
    dispatchCommand(mxComponent, u".uno:InsertDoc"_ustr, aArgs);

    // Then the pasted "inner" bullet (clipboard depth 1) lands at level 2,
    // the clipboard's own depth shifted by the paste position's level:
    SwTextNode* pInnerNode = nullptr;
    for (SwNodeOffset i(0); i < rNodes.Count(); ++i)
    {
        SwTextNode* pCandidate = rNodes[i]->GetTextNode();
        if (pCandidate && pCandidate->GetText() == u"inner")
        {
            pInnerNode = pCandidate;
            break;
        }
    }
    CPPUNIT_ASSERT(pInnerNode);
    // Without the accompanying fix in place, this test would have failed with
    // level 1, the clipboard's own depth, not shifted by the paste position's
    // level.
    CPPUNIT_ASSERT_EQUAL(2, pInnerNode->GetAttrListLevel());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
