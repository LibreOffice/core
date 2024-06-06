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
#include <docsh.hxx>
#include <wrtsh.hxx>
#include <viewopt.hxx>

namespace
{
/// Covers sw/source/core/text/widorp.cxx fixes.
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/core/text/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testHideWhitespaceWidorp)
{
    // Given a document with 3 paragraphs: 1.5 on page 1, 1.5 on page 2:
    createSwDoc("hide-whitespace-widorp.odt");

    // When hiding whitespace:
    SwDocShell* pDocShell = getSwDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    SwViewOption aViewOptions(*pWrtShell->GetViewOptions());
    aViewOptions.SetHideWhitespaceMode(true);
    pWrtShell->ApplyViewOptions(aViewOptions);
    calcLayout();

    // Then make sure that paragraph 2 is still split:
    SwDoc* pDoc = pDocShell->GetDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage = pLayout->Lower()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage);
    SwFrame* pBody = pPage->FindBodyCont();
    SwFrame* pPara1 = pBody->GetLower();
    auto pPara2 = pPara1->GetNext()->DynCastTextFrame();
    CPPUNIT_ASSERT(pPara2);
    // Without the accompanying fix in place, this test would have failed, the entire paragraph 2
    // went to page 1, so it was not split anymore.
    CPPUNIT_ASSERT(pPara2->HasFollow());
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
