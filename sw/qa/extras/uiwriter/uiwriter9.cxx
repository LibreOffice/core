/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <vcl/scheduler.hxx>
#include <com/sun/star/table/TableBorder2.hpp>
#include <com/sun/star/text/XDocumentIndex.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <comphelper/propertysequence.hxx>
#include <swdtflvr.hxx>
#include <o3tl/string_view.hxx>

#include <view.hxx>
#include <wrtsh.hxx>
#include <unotxdoc.hxx>
#include <ndtxt.hxx>
#include <toxmgr.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <fmtinfmt.hxx>
#include <rootfrm.hxx>

namespace
{
class SwUiWriterTest9 : public SwModelTestBase
{
public:
    SwUiWriterTest9()
        : SwModelTestBase("/sw/qa/extras/uiwriter/data/")
    {
    }
};

CPPUNIT_TEST_FIXTURE(SwUiWriterTest9, testTdf158785)
{
    // given a document with a hyperlink surrounded by N-dashes (–www.dordt.edu–)
    createSwDoc("tdf158785_hyperlink.fodt");
    SwDoc& rDoc = *getSwDoc();
    SwWrtShell* pWrtShell = rDoc.GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    // go to the end of the hyperlink
    pWrtShell->SttEndDoc(/*bStart=*/false);
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    // get last point that will be part of the hyperlink (current position 1pt wide).
    Point aLogicL(pWrtShell->GetCharRect().Center());
    Point aLogicR(aLogicL);

    // sanity check - we really are right by the hyperlink
    aLogicL.AdjustX(-1);
    SwContentAtPos aContentAtPos(IsAttrAtPos::InetAttr);
    pWrtShell->GetContentAtPos(aLogicL, aContentAtPos);
    CPPUNIT_ASSERT_EQUAL(IsAttrAtPos::InetAttr, aContentAtPos.eContentAtPos);

    // The test: the position of the N-dash should not indicate hyperlink properties
    // cursor pos would NOT be considered part of the hyperlink, but increase for good measure...
    aLogicR.AdjustX(1);
    pWrtShell->GetContentAtPos(aLogicR, aContentAtPos);
    CPPUNIT_ASSERT_EQUAL(IsAttrAtPos::NONE, aContentAtPos.eContentAtPos);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest9, testTdf159816)
{
    createSwDoc();

    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    // Add 5 empty paragraphs
    pWrtShell->SplitNode();
    pWrtShell->SplitNode();
    pWrtShell->SplitNode();
    pWrtShell->SplitNode();
    pWrtShell->SplitNode();

    // Add a bookmark at the very end
    IDocumentMarkAccess& rIDMA(*pDoc->getIDocumentMarkAccess());
    rIDMA.makeMark(*pWrtShell->GetCursor(), "Mark", IDocumentMarkAccess::MarkType::BOOKMARK,
                   sw::mark::InsertMode::New);

    // Get coordinates of the end point in the document
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    SwFrame* pPage = pLayout->Lower();
    SwFrame* pBody = pPage->GetLower();
    SwFrame* pLastPara = pBody->GetLower()->GetNext()->GetNext()->GetNext()->GetNext()->GetNext();
    Point ptTo = pLastPara->getFrameArea().BottomRight();

    pWrtShell->SelAll();

    // Drag-n-drop to its own end
    rtl::Reference<SwTransferable> xTransfer = new SwTransferable(*pWrtShell);
    // Without the fix, this would crash: either in CopyFlyInFlyImpl (tdf#159813):
    // Assertion failed: !pCopiedPaM || pCopiedPaM->End()->GetNode() == rRg.aEnd.GetNode()
    // or in BigPtrArray::operator[] (tdf#159816):
    // Assertion failed: idx < m_nSize
    xTransfer->PrivateDrop(*pWrtShell, ptTo, /*bMove=*/true, /*bXSelection=*/true);
}

} // end of anonymouse namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
