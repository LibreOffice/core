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
#include <ndtxt.hxx>
#include <numrule.hxx>

using namespace css;
using namespace ::cpo::uno;

namespace
{
/// Covers sw/source/core/doc/number.cxx fixes.
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/core/doc/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testBadHeadingIndent)
{
    // Given a DOCX file with a single paragraph, no heading styles present:
    createSwDoc("bad-heading-indent.docx");

    // When marking that paragraph heading 1:
    cpo::uno::Sequence<beans::PropertyValue> aPropertyValues = {
        comphelper::makePropertyValue(u"Style"_ustr, cpo::uno::Any(u"Heading 1"_ustr)),
        comphelper::makePropertyValue(u"FamilyName"_ustr, cpo::uno::Any(u"ParagraphStyles"_ustr)),
    };
    dispatchCommand(mxComponent, u".uno:StyleApply"_ustr, aPropertyValues);

    // Then make sure that doesn't result in unexpected indent:
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    int nTabCount
        = getXPathContent(pXmlDoc, "count(//SwLineLayout/child::*[@type='PortionType::TabLeft'])")
              .toInt32();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0
    // - Actual  : 1
    // i.e. an unexpected tab portion was inserted before the paragraph text.
    CPPUNIT_ASSERT_EQUAL(0, nTabCount);
}

CPPUNIT_TEST_FIXTURE(Test, testMergedPasteBullets)
{
    // Given an empty Writer document with the merged-paste flag on:
    createSwDoc();
    SwDoc* pDoc = getSwDocShell()->GetDoc();
    pDoc->SetInMergedPaste(true);
    comphelper::ScopeGuard g([pDoc] { pDoc->SetInMergedPaste(false); });

    // When importing an HTML file with a nested unordered list into it:
    cpo::uno::Sequence<beans::PropertyValue> aArgs
        = { comphelper::makePropertyValue(u"Name"_ustr, createFileURL(u"merged-paste-bullet.html")) };
    dispatchCommand(mxComponent, u".uno:InsertDoc"_ustr, aArgs);

    // Then the pasted list uses the Writer UI bullet char and font at each
    // level:
    SwTextNode* pInnerNode = nullptr;
    SwNodes& rNodes = pDoc->GetNodes();
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
    const SwNumRule* pRule = pInnerNode->GetNumRule();
    CPPUNIT_ASSERT(pRule);
    const SwNumFormat& rLevel0 = pRule->Get(0);
    const SwNumFormat& rLevel1 = pRule->Get(1);
    CPPUNIT_ASSERT_EQUAL(sal_UCS4(0x2022), rLevel0.GetBulletChar());
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 9702
    // - Actual  : 8226
    // i.e. the inner bullet's char was the same as the outer one.
    CPPUNIT_ASSERT_EQUAL(sal_UCS4(0x25e6), rLevel1.GetBulletChar());
    const std::optional<vcl::Font>& oFont = rLevel0.GetBulletFont();
    CPPUNIT_ASSERT(oFont);
    CPPUNIT_ASSERT_EQUAL(u"OpenSymbol"_ustr, oFont->GetFamilyName());
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
