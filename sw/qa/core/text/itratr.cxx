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
#include <sortedobjs.hxx>

namespace
{
/// Covers sw/source/core/text/itratr.cxx fixes.
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/core/text/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testFloattableNegativeVertOffsetEmptyAnchor)
{
    // Given a document with 3 floating tables, all of them on page 1, but the anchor of the last
    // floating table is on page 2, but that anchor has no own text:
    createSwDoc("floattable-negative-vert-offset-empty.docx");

    // When laying out that document:
    calcLayout();

    // Then make sure the that 3rd floating table is not shifted to page 2:
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage1 = dynamic_cast<SwPageFrame*>(pLayout->Lower());
    CPPUNIT_ASSERT(pPage1);
    CPPUNIT_ASSERT(pPage1->GetSortedObjs());
    const SwSortedObjs& rPage1Objs = *pPage1->GetSortedObjs();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 3
    // - Actual  : 2
    // i.e. the last floating table was shifted to page 2, which lead to overlapping text in the
    // original document.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), rPage1Objs.size());
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
