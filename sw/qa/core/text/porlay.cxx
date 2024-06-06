/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <memory>

#include <IDocumentLayoutAccess.hxx>
#include <doc.hxx>
#include <frame.hxx>
#include <pagefrm.hxx>
#include <rootfrm.hxx>
#include <txtfrm.hxx>

namespace
{
/// Covers sw/source/core/text/porlay.cxx fixes.
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/core/text/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testEmptySuperscript)
{
    // Given a document with 4 paragraphs, the 2nd is empty and is marked as superscript:
    createSwDoc("empty-superscript.docx");

    // When laying out that document:
    calcLayout();

    // Then make sure all 4 text frames have visible heights:
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage = pLayout->GetLower()->DynCastPageFrame();
    auto pText1 = pPage->FindFirstBodyContent()->DynCastTextFrame();
    CPPUNIT_ASSERT_GREATER(static_cast<SwTwips>(0), pText1->getFrameArea().Height());
    auto pText2 = pText1->GetNext()->DynCastTextFrame();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected greater than: 0
    // - Actual  : 0
    // i.e. the 2nd paragraph was not visible.
    CPPUNIT_ASSERT_GREATER(static_cast<SwTwips>(0), pText2->getFrameArea().Height());
    auto pText3 = pText2->GetNext()->DynCastTextFrame();
    CPPUNIT_ASSERT_GREATER(static_cast<SwTwips>(0), pText3->getFrameArea().Height());
    auto pText4 = pText3->GetNext()->DynCastTextFrame();
    CPPUNIT_ASSERT_GREATER(static_cast<SwTwips>(0), pText4->getFrameArea().Height());
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
