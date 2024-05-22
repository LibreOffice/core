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
    pWrtShell->GetView().AttrChangedNotify(nullptr);
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

CPPUNIT_TEST_FIXTURE(Test, testInlineEndnoteAndFootnote)
{
    // Given a DOC file with an endnote and then a footnote:
    createSwDoc("inline-endnote-and-footnote.doc");

    // When laying out that document:
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // Then make sure the footnote is below the endnote:
    // Without the accompanying fix in place, this test would have failed with:
    // - xpath should match exactly 1 node
    // i.e. the endnote was also in the footnote container, not at the end of the body text.
    sal_Int32 nEndnoteTop
        = parseDump("/root/page/body/section/column/ftncont/ftn/infos/bounds"_ostr, "top"_ostr)
              .toInt32();
    sal_Int32 nFootnoteTop
        = parseDump("/root/page/ftncont/ftn/infos/bounds"_ostr, "top"_ostr).toInt32();
    // Endnote at the end of body text, footnote at page bottom.
    CPPUNIT_ASSERT_LESS(nFootnoteTop, nEndnoteTop);
}

CPPUNIT_TEST_FIXTURE(Test, testInlineEndnoteAndSection)
{
    // Given a document ending with a section, ContinuousEndnotes is true:
    createSwDoc("inline-endnote-and-section.odt");

    // When laying out that document:
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // Then make sure the endnote section is after the section at the end of the document, not
    // inside it:
    int nToplevelSections = countXPathNodes(pXmlDoc, "/root/page/body/section"_ostr);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2
    // - Actual  : 1
    // and we even crashed on shutdown.
    CPPUNIT_ASSERT_EQUAL(2, nToplevelSections);
}

CPPUNIT_TEST_FIXTURE(Test, testInlineEndnotePosition)
{
    // Given a document, ContinuousEndnotes is true:
    createSwDoc("inline-endnote-position.docx");

    // When laying out that document:
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // Then make sure the endnote separator (line + spacing around it) is large enough, so the
    // endnote text below the separator has the correct position:
    sal_Int32 nEndnoteContTopMargin
        = parseDump("//column/ftncont/infos/prtBounds"_ostr, "top"_ostr).toInt32();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 269
    // - Actual  : 124
    // i.e. the top margin wasn't the default font size with its spacing, but the Writer default,
    // which shifted endnote text up, incorrectly.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(269), nEndnoteContTopMargin);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
