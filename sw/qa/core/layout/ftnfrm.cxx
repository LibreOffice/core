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
#include <docsh.hxx>
#include <formatflysplit.hxx>
#include <frmmgr.hxx>
#include <pagefrm.hxx>
#include <rootfrm.hxx>
#include <view.hxx>
#include <wrtsh.hxx>

/// Covers sw/source/core/layout/ftnfrm.cxx fixes.
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase("/sw/qa/core/layout/data/")
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testFlySplitFootnoteLayout)
{
    // Given a document with a split fly (to host a table):
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    SwFlyFrameAttrMgr aMgr(true, pWrtShell, Frmmgr_Type::TEXT, nullptr);
    RndStdIds eAnchor = RndStdIds::FLY_AT_PARA;
    pWrtShell->StartAllAction();
    aMgr.InsertFlyFrame(eAnchor, aMgr.GetPos(), aMgr.GetSize());
    pWrtShell->EndAllAction();
    pWrtShell->StartAllAction();
    sw::FrameFormats<sw::SpzFrameFormat*>& rFlys = *pDoc->GetSpzFrameFormats();
    sw::SpzFrameFormat* pFly = rFlys[0];
    SwAttrSet aSet(pFly->GetAttrSet());
    aSet.Put(SwFormatFlySplit(true));
    pDoc->SetAttr(aSet, *pFly);
    pWrtShell->EndAllAction();
    pWrtShell->UnSelectFrame();
    pWrtShell->LeaveSelFrameMode();
    pWrtShell->GetView().TriggerAttrChangedNotify();
    pWrtShell->MoveSection(GoCurrSection, fnSectionEnd);

    // When inserting a footnote:
    pWrtShell->InsertFootnote(OUString());

    // Then make sure the footnote frame and its container is created:
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage = dynamic_cast<SwPageFrame*>(pLayout->Lower());
    CPPUNIT_ASSERT(pPage);
    // Without the accompanying fix in place, this test would have failed, the footnote frame was
    // not created, the footnote reference was empty.
    CPPUNIT_ASSERT(pPage->FindFootnoteCont());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
