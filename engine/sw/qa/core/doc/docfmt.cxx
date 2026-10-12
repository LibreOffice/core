/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <comphelper/propertyvalue.hxx>
#include <comphelper/scopeguard.hxx>

#include <docsh.hxx>
#include <fmtcol.hxx>
#include <ndtxt.hxx>
#include <numrule.hxx>

using namespace css;
using namespace ::cpo;
using namespace ::cpo::uno;

namespace
{
/// Covers sw/source/core/doc/docfmt.cxx fixes.
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/core/doc/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testMergedPasteParaStyle)
{
    // Given an empty Writer document with the merged-paste flag on:
    createSwDoc();
    SwDoc* pDoc = getSwDocShell()->GetDoc();
    pDoc->SetInMergedPaste(true);
    comphelper::ScopeGuard g([pDoc] { pDoc->SetInMergedPaste(false); });

    // When importing an RTF file whose paragraphs are in an Impress outline
    // paragraph style:
    cpo::uno::Sequence<beans::PropertyValue> aArgs
        = { comphelper::makePropertyValue(u"Name"_ustr,
                                          createFileURL(u"merged-paste-style.rtf")) };
    dispatchCommand(mxComponent, u".uno:InsertDoc"_ustr, aArgs);

    // Then the pasted paragraph drops the source style, so the destination's
    // own paragraph style stays in place:
    SwTextNode* pTextNode = nullptr;
    SwNodes& rNodes = pDoc->GetNodes();
    for (SwNodeOffset i(0); i < rNodes.Count(); ++i)
    {
        SwTextNode* pCandidate = rNodes[i]->GetTextNode();
        if (pCandidate && pCandidate->GetText() == u"First")
        {
            pTextNode = pCandidate;
            break;
        }
    }
    CPPUNIT_ASSERT(pTextNode);
    SwTextFormatColl* pColl = pTextNode->GetTextColl();
    CPPUNIT_ASSERT(pColl);
    // Without the accompanying fix in place, this failed with:
    // - Expected: Default Paragraph Style
    // - Actual  : Title and Content~LT~Gliederung 1
    // i.e. the unwanted paragraph style was pasted.
    CPPUNIT_ASSERT_EQUAL(u"Default Paragraph Style"_ustr, pColl->GetName().toString());
}

CPPUNIT_TEST_FIXTURE(Test, testMergedPasteListAnonymous)
{
    // Given an empty Writer document with the merged-paste flag on:
    createSwDoc();
    SwDoc* pDoc = getSwDocShell()->GetDoc();
    pDoc->SetInMergedPaste(true);
    comphelper::ScopeGuard g([pDoc] { pDoc->SetInMergedPaste(false); });

    // When importing an RTF file whose paragraphs are bulleted:
    cpo::uno::Sequence<beans::PropertyValue> aArgs
        = { comphelper::makePropertyValue(u"Name"_ustr,
                                          createFileURL(u"merged-paste-style.rtf")) };
    dispatchCommand(mxComponent, u".uno:InsertDoc"_ustr, aArgs);

    // Then the pasted bullet's numrule is anonymous, so a named list style
    // (WWNum1) is not forced on the destination:
    SwTextNode* pTextNode = nullptr;
    SwNodes& rNodes = pDoc->GetNodes();
    for (SwNodeOffset i(0); i < rNodes.Count(); ++i)
    {
        SwTextNode* pCandidate = rNodes[i]->GetTextNode();
        if (pCandidate && pCandidate->GetText() == u"First")
        {
            pTextNode = pCandidate;
            break;
        }
    }
    CPPUNIT_ASSERT(pTextNode);
    const SwNumRule* pRule = pTextNode->GetNumRule();
    CPPUNIT_ASSERT(pRule);
    // Without the accompanying fix in place, this failed, the pasted bullet
    // was on the WWNum1 named list style instead.
    CPPUNIT_ASSERT(pRule->IsAutoRule());
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
