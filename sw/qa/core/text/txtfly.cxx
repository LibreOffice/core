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

namespace
{
/// Covers sw/source/core/text/txtfly.cxx fixes.
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase("/sw/qa/core/text/data/")
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testClearingBreakWrapThrough)
{
    // Given a document with a clearing break, then a shape in the next paragraph:
    createSwDoc("clearing-break-wrap-through.docx");

    // When laying out that document:
    calcLayout();

    // Then make sure we layout these 2 paragraphs on a single page, since there is enough space for
    // them:
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage = pLayout->Lower()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage);
    // Without the accompanying fix in place, this test would have failed, we had an unexpected 2nd
    // page.
    CPPUNIT_ASSERT(!pPage->GetNext());
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
