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
#include <layfrm.hxx>
#include <pagefrm.hxx>
#include <rootfrm.hxx>
#include <txtfrm.hxx>

namespace
{
/// Covers sw/source/core/text/porrst.cxx fixes.
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase("/sw/qa/core/text/data/")
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testFloattableLeftoverParaPortion)
{
    // Given a document with a multi-page floating table, the anchor of the table has some text:
    createSwDoc("floattable-leftover-para-portion.docx");

    // When laying out that document:
    calcLayout();

    // Then make sure all anchor text goes to the second page:
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage = dynamic_cast<SwPageFrame*>(pLayout->Lower());
    CPPUNIT_ASSERT(pPage);
    SwFrame* pBody = pPage->FindBodyCont();
    SwFrame* pPara1 = pBody->GetLower();
    auto pPara2 = pPara1->GetNext()->DynCastTextFrame();
    CPPUNIT_ASSERT(pPara2);
    // Without the accompanying fix in place, this test would have failed, the first page's anchor
    // also had some (duplicated) anchor text.
    CPPUNIT_ASSERT(!pPara2->GetPara());
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
