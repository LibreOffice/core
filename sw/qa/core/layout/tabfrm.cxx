/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <IDocumentLayoutAccess.hxx>
#include <rootfrm.hxx>
#include <pagefrm.hxx>
#include <tabfrm.hxx>

namespace
{
/// Covers sw/source/core/layout/tabfrm.cxx fixes.
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase("/sw/qa/core/layout/data/")
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testTableMissingJoin)
{
    // Given a document with a table on page 2:
    // When laying out that document:
    createSwDoc("table-missing-join.docx");

    // Then make sure that the table fits page 2:
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage1 = pLayout->Lower()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage1);
    auto pPage2 = pPage1->GetNext()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage2);
    SwFrame* pBody = pPage2->FindBodyCont();
    auto pTab = pBody->GetLower()->DynCastTabFrame();
    // Without the accompanying fix in place, this test would have failed, the table continued on
    // page 3.
    CPPUNIT_ASSERT(!pTab->HasFollow());
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
