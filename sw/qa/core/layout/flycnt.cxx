/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <comphelper/scopeguard.hxx>

#include <IDocumentLayoutAccess.hxx>
#include <anchoredobject.hxx>
#include <flyfrms.hxx>
#include <formatflysplit.hxx>
#include <pagefrm.hxx>
#include <rootfrm.hxx>
#include <sortedobjs.hxx>
#include <tabfrm.hxx>

namespace
{
/// Covers sw/source/core/layout/flycnt.cxx fixes, i.e. mostly SwFlyAtContentFrame.
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase("/sw/qa/core/layout/data/")
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testSplitFlyWithTable)
{
    // Given a document with a multi-page floating table:
    SwFormatFlySplit::SetForce(true);
    comphelper::ScopeGuard g([] { SwFormatFlySplit::SetForce(false); });
    createSwDoc("floattable.docx");

    // When laying out that document:
    calcLayout();

    // Then make sure that the first row goes to page 1 and the second row goes to page 2, while the
    // table is floating:
    SwDoc* pDoc = getSwDoc();
    // Without the accompanying fix in place, this test would have failed with a stack overflow
    // because the follow frame of the anchor was moved into the follow frame of the fly, so the fly
    // was anchored in itself.
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    // Page 1 has a master fly, which contains a master table:
    auto pPage1 = dynamic_cast<SwPageFrame*>(pLayout->Lower());
    CPPUNIT_ASSERT(pPage1);
    const SwSortedObjs& rPage1Objs = *pPage1->GetSortedObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPage1Objs.size());
    auto pPage1Fly = dynamic_cast<SwFlyAtContentFrame*>(rPage1Objs[0]);
    CPPUNIT_ASSERT(pPage1Fly);
    CPPUNIT_ASSERT(!pPage1Fly->GetPrecede());
    CPPUNIT_ASSERT(pPage1Fly->GetFollow());
    auto pPage1Table = dynamic_cast<SwTabFrame*>(pPage1Fly->GetLower());
    CPPUNIT_ASSERT(pPage1Table);
    CPPUNIT_ASSERT(!pPage1Table->GetPrecede());
    CPPUNIT_ASSERT(pPage1Table->GetFollow());
    // Page 2 has a follow fly, which contains a follow table:
    auto pPage2 = dynamic_cast<SwPageFrame*>(pPage1->GetNext());
    CPPUNIT_ASSERT(pPage2);
    const SwSortedObjs& rPage2Objs = *pPage2->GetSortedObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPage2Objs.size());
    auto pPage2Fly = dynamic_cast<SwFlyAtContentFrame*>(rPage2Objs[0]);
    CPPUNIT_ASSERT(pPage2Fly);
    CPPUNIT_ASSERT(pPage2Fly->GetPrecede());
    CPPUNIT_ASSERT(!pPage2Fly->GetFollow());
    auto pPage2Table = dynamic_cast<SwTabFrame*>(pPage2Fly->GetLower());
    CPPUNIT_ASSERT(pPage2Table);
    CPPUNIT_ASSERT(pPage2Table->GetPrecede());
    CPPUNIT_ASSERT(!pPage2Table->GetFollow());
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
