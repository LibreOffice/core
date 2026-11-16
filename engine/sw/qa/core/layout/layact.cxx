/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <vcl/scheduler.hxx>
#include <vcl/idle.hxx>
#include <comphelper/kit.hxx>

#include <IDocumentLayoutAccess.hxx>
#include <unotxdoc.hxx>
#include <anchoredobject.hxx>
#include <fmtclds.hxx>
#include <fmtfsize.hxx>
#include <fmtpdsc.hxx>
#include <ndtxt.hxx>
#include <hintids.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/formatbreakitem.hxx>
#include <editeng/orphitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/widwitem.hxx>
#include <docsh.hxx>
#include <fmthdft.hxx>
#include <flowfrm.hxx>
#include <frmatr.hxx>
#include <flyfrm.hxx>
#include <pagefrm.hxx>
#include <pagedesc.hxx>
#include <rootfrm.hxx>
#include <rowfrm.hxx>
#include <sortedobjs.hxx>
#include <tabfrm.hxx>
#include <txtfrm.hxx>
#include <wrtsh.hxx>
#include <sectfrm.hxx>

using namespace css;
using namespace ::cpo::uno;

namespace
{
/// Covers sw/source/core/layout/layact.cxx fixes.
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/core/layout/data/"_ustr)
    {
    }

    void setUp() override
    {
        SwFlowFrame::ResetMoveBwdJump();
        SwModelTestBase::setUp();
        SwFlowFrame::ResetMoveBwdJump();
    }

    void tearDown() override
    {
        SwModelTestBase::tearDown();
        SwFlowFrame::ResetMoveBwdJump();
    }
};

SwTextNode* lclFindSectionTextNode(SwDoc& rDoc)
{
    SwNodes& rNodes = rDoc.GetNodes();
    for (SwNodeOffset nIndex(0); nIndex < rNodes.Count(); ++nIndex)
    {
        SwTextNode* pTextNode = rNodes[nIndex]->GetTextNode();
        if (pTextNode && pTextNode->FindSectionNode())
            return pTextNode;
    }
    return nullptr;
}

void lclInsertTextOneCharacterAtATime(SwWrtShell& rWrtShell, SwTextNode& rTextNode,
                                      const OUString& rText)
{
    rWrtShell.GetCursor()->DeleteMark();
    rWrtShell.GetCursor()->GetPoint()->Assign(rTextNode, 0);

    // Insert as keyboard input does, so layout runs after each character.
    for (sal_Int32 nIndex = 0; nIndex < rText.getLength(); ++nIndex)
        rWrtShell.Insert(rText.copy(nIndex, 1));
}

void lclInsertRepeatedJOneCharacterAtATime(SwWrtShell& rWrtShell, SwTextNode& rTextNode,
                                           sal_Int32 nCount = 100)
{
    OUString aText;
    for (sal_Int32 nIndex = 0; nIndex < nCount; ++nIndex)
        aText += u"j"_ustr;
    lclInsertTextOneCharacterAtATime(rWrtShell, rTextNode, aText);
}

SwSectionFrame* lclFindSectionFrame(SwFrame& rFrame)
{
    SwFrame* pFrame = &rFrame;
    while (pFrame && !pFrame->IsSctFrame())
        pFrame = pFrame->GetUpper();
    return pFrame ? static_cast<SwSectionFrame*>(pFrame) : nullptr;
}

SwPageFrame* lclFindPageFrame(SwFrame& rFrame)
{
    SwFrame* pFrame = &rFrame;
    while (pFrame && !pFrame->IsPageFrame())
        pFrame = pFrame->GetUpper();
    return pFrame ? static_cast<SwPageFrame*>(pFrame) : nullptr;
}

void lclAssertFrameInSectionColumn(SwFrame& rFrame, const SwSection& rSection)
{
    CPPUNIT_ASSERT(lclFindPageFrame(rFrame));
    SwLayoutFrame* pColumnBody = rFrame.GetUpper();
    CPPUNIT_ASSERT(pColumnBody);
    CPPUNIT_ASSERT(pColumnBody->IsColBodyFrame());
    SwSectionFrame* pSectionFrame = lclFindSectionFrame(rFrame);
    CPPUNIT_ASSERT(pSectionFrame);
    CPPUNIT_ASSERT_EQUAL(&rSection, static_cast<const SwSection*>(pSectionFrame->GetSection()));
}

SwTextFrame* lclAssertTextFramesInSectionColumns(SwTextNode& rTextNode, const SwRootFrame& rLayout,
                                                 const SwSection& rSection,
                                                 bool bRequireMultipleColumnBodies = false,
                                                 bool bRequireSingleFirstColumn = false,
                                                 bool bRequirePageContinuation = false)
{
    SwContentFrame* pContentFrame = rTextNode.getLayoutFrame(&rLayout);
    CPPUNIT_ASSERT(pContentFrame);
    SwTextFrame* pTextFrame = pContentFrame->DynCastTextFrame();
    CPPUNIT_ASSERT(pTextFrame);
    while (pTextFrame->GetPrecede())
        pTextFrame = static_cast<SwTextFrame*>(pTextFrame->GetPrecede());
    SwTextFrame* pFirstTextFrame = pTextFrame;

    sal_Int32 nExpectedOffset = 0;
    const SwLayoutFrame* pFirstColumnBody = nullptr;
    bool bUsesAnotherColumnBody = false;
    bool bUsesAnotherColumnOnFirstPage = false;
    bool bUsesLaterPage = false;
    sal_Int32 nFrameCount = 0;
    sal_uInt16 nPreviousPage = 0;
    sal_uInt16 nPreviousColumn = 0;
    sal_uInt16 nFirstPage = 0;
    sal_uInt16 nFirstColumn = 0;
    while (pTextFrame)
    {
        CPPUNIT_ASSERT_EQUAL(nExpectedOffset, pTextFrame->GetOffset().get());
        lclAssertFrameInSectionColumn(*pTextFrame, rSection);

        const SwLayoutFrame* pColumnBody = pTextFrame->GetUpper();
        if (!pFirstColumnBody)
            pFirstColumnBody = pColumnBody;
        else if (pColumnBody != pFirstColumnBody)
            bUsesAnotherColumnBody = true;
        const SwPageFrame* pPage = lclFindPageFrame(*pTextFrame);
        CPPUNIT_ASSERT(pPage);
        const SwFrame* pColumn = pColumnBody->GetUpper();
        CPPUNIT_ASSERT(pColumn);
        CPPUNIT_ASSERT(pColumn->IsColumnFrame());
        sal_uInt16 nColumn = 1;
        for (const SwFrame* pPreviousColumn = pColumn->GetPrev(); pPreviousColumn;
             pPreviousColumn = pPreviousColumn->GetPrev())
            ++nColumn;
        const sal_uInt16 nPage = pPage->GetPhyPageNum();
        if (nFrameCount && nPage > nFirstPage)
            bUsesLaterPage = true;
        if (nFrameCount == 0)
        {
            nFirstPage = nPage;
            nFirstColumn = nColumn;
        }
        else
        {
            CPPUNIT_ASSERT(nPage > nPreviousPage
                           || (nPage == nPreviousPage && nColumn > nPreviousColumn));
            if (nPage == nFirstPage && nColumn > nFirstColumn)
                bUsesAnotherColumnOnFirstPage = true;
        }
        nPreviousPage = nPage;
        nPreviousColumn = nColumn;
        ++nFrameCount;

        SwTextFrame* pFollow = pTextFrame->GetFollow();
        const sal_Int32 nFrameEnd = pFollow ? pFollow->GetOffset().get() : rTextNode.Len();
        CPPUNIT_ASSERT(nFrameEnd > nExpectedOffset);
        CPPUNIT_ASSERT(nFrameEnd <= rTextNode.Len());
        nExpectedOffset = nFrameEnd;
        pTextFrame = pFollow;
    }
    CPPUNIT_ASSERT_EQUAL(rTextNode.Len(), nExpectedOffset);
    if (bRequireMultipleColumnBodies)
    {
        CPPUNIT_ASSERT(nFrameCount > 1);
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(1), nFirstPage);
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(1), nFirstColumn);
        CPPUNIT_ASSERT(bUsesAnotherColumnBody);
        CPPUNIT_ASSERT(bUsesAnotherColumnOnFirstPage);
    }
    if (bRequireSingleFirstColumn)
    {
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), nFrameCount);
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(1), nFirstPage);
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(1), nFirstColumn);
    }
    if (bRequirePageContinuation)
        CPPUNIT_ASSERT(bUsesLaterPage);
    return pFirstTextFrame;
}

void lclSetSectionColumnCount(SwWrtShell& rWrtShell, SwSectionNode& rSectionNode,
                              sal_uInt16 nColumnCount)
{
    SwSectionFormat* pSectionFormat = rSectionNode.GetSection().GetFormat();
    SwFormatCol aColumns(pSectionFormat->GetCol());
    aColumns.Init(nColumnCount, aColumns.GetGutterWidth(), aColumns.GetWishWidth());
    SfxItemSet aSet(pSectionFormat->GetAttrSet());
    aSet.Put(aColumns);
    rWrtShell.SetSectionAttr(aSet, pSectionFormat);
}

void lclAssertSectionColumnCount(SwTextNode& rTextNode, const SwRootFrame& rLayout,
                                 sal_uInt16 nExpectedCount)
{
    SwContentFrame* pContentFrame = rTextNode.getLayoutFrame(&rLayout);
    CPPUNIT_ASSERT(pContentFrame);
    SwSectionFrame* pSectionFrame = lclFindSectionFrame(*pContentFrame);
    CPPUNIT_ASSERT(pSectionFrame);

    sal_uInt16 nColumnCount = 0;
    for (const SwFrame* pColumn = pSectionFrame->Lower(); pColumn; pColumn = pColumn->GetNext())
    {
        CPPUNIT_ASSERT(pColumn->IsColumnFrame());
        ++nColumnCount;
    }
    CPPUNIT_ASSERT_EQUAL(nExpectedCount, nColumnCount);
}

class DetachedFrameRestorer
{
    SwFrame& m_rFrame;
    SwLayoutFrame& m_rParent;
    SwFrame* m_pSibling;

public:
    DetachedFrameRestorer(SwFrame& rFrame, SwLayoutFrame& rParent, SwFrame* pSibling)
        : m_rFrame(rFrame)
        , m_rParent(rParent)
        , m_pSibling(pSibling)
    {
    }

    void RestoreAt(SwLayoutFrame& rParent, SwFrame* pSibling)
    {
        if (!m_rFrame.GetUpper())
            m_rFrame.Paste(&rParent, pSibling);
    }

    ~DetachedFrameRestorer()
    {
        if (!m_rFrame.GetUpper())
            m_rFrame.Paste(&m_rParent, m_pSibling);
    }
};

CPPUNIT_TEST_FIXTURE(Test, testTdf169498TypeIntoThreeColumnSection)
{
    createSwDoc("tdf169498-sectioncrash.odt");

    SwTextNode* pSectionTextNode = lclFindSectionTextNode(*getSwDoc());
    CPPUNIT_ASSERT(pSectionTextNode);
    SwSectionNode* pSectionNode = pSectionTextNode->FindSectionNode();
    CPPUNIT_ASSERT(pSectionNode);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(3),
                         pSectionNode->GetSection().GetFormat()->GetCol().GetNumCols());

    calcLayout();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The original document should stay on one page", 1, getPages());
    SwRootFrame* pLayout = getSwDoc()->getIDocumentLayoutAccess().GetCurrentLayout();
    CPPUNIT_ASSERT(pLayout);
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    lclInsertRepeatedJOneCharacterAtATime(*pWrtShell, *pSectionTextNode);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(100), pSectionTextNode->Len());
    calcLayout();
    lclAssertTextFramesInSectionColumns(*pSectionTextNode, *pLayout, pSectionNode->GetSection(),
                                        false, true);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf169498ThreeColumnWidowsOrphansThree)
{
    createSwDoc("tdf169498-sectioncrash.odt");
    SwTextNode* pSectionTextNode = lclFindSectionTextNode(*getSwDoc());
    CPPUNIT_ASSERT(pSectionTextNode);
    SwSectionNode* pSectionNode = pSectionTextNode->FindSectionNode();
    CPPUNIT_ASSERT(pSectionNode);

    pSectionTextNode->SetAttr(SvxWidowsItem(3, RES_PARATR_WIDOWS));
    pSectionTextNode->SetAttr(SvxOrphansItem(3, RES_PARATR_ORPHANS));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(3), pSectionNode->GetSection().GetFormat()->GetCol().GetNumCols());
    CPPUNIT_ASSERT_EQUAL(sal_uInt8(3), pSectionTextNode->GetSwAttrSet().GetWidows().GetValue());
    CPPUNIT_ASSERT_EQUAL(sal_uInt8(3), pSectionTextNode->GetSwAttrSet().GetOrphans().GetValue());

    calcLayout();
    lclInsertRepeatedJOneCharacterAtATime(*getSwDocShell()->GetWrtShell(), *pSectionTextNode,
                                          200);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(200), pSectionTextNode->Len());
    calcLayout();
    const SwRootFrame* pLayout = getSwDoc()->getIDocumentLayoutAccess().GetCurrentLayout();
    CPPUNIT_ASSERT(pLayout);
    // The five lines cannot be split while satisfying both the widow and orphan counts.
    lclAssertTextFramesInSectionColumns(*pSectionTextNode, *pLayout, pSectionNode->GetSection(),
                                        false, true);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf169498NoPrecedingParagraph)
{
    createSwDoc("tdf169498-sectioncrash-no-preceding-paragraph.odt");
    SwDoc& rDoc = *getSwDoc();
    SwTextNode* pSectionTextNode = lclFindSectionTextNode(rDoc);
    CPPUNIT_ASSERT(pSectionTextNode);
    SwSectionNode* pSectionNode = pSectionTextNode->FindSectionNode();
    CPPUNIT_ASSERT(pSectionNode);

    bool bHasPrecedingBodyParagraph = false;
    SwNodes& rNodes = rDoc.GetNodes();
    for (SwNodeOffset nIndex(0); nIndex < pSectionNode->GetIndex(); ++nIndex)
    {
        SwTextNode* pTextNode = rNodes[nIndex]->GetTextNode();
        if (pTextNode && !pTextNode->FindSectionNode())
            bHasPrecedingBodyParagraph = true;
    }
    CPPUNIT_ASSERT_MESSAGE("the fixture must begin with the section", !bHasPrecedingBodyParagraph);

    calcLayout();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The original document should stay on one page", 1, getPages());
    SwRootFrame* pLayout = getSwDoc()->getIDocumentLayoutAccess().GetCurrentLayout();
    CPPUNIT_ASSERT(pLayout);
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    lclInsertRepeatedJOneCharacterAtATime(*pWrtShell, *pSectionTextNode);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(100), pSectionTextNode->Len());
    calcLayout();
    lclAssertTextFramesInSectionColumns(*pSectionTextNode, *pLayout, pSectionNode->GetSection(),
                                        false, true);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf169498BalancedMultiWordText)
{
    createSwDoc("tdf169498-sectioncrash.odt");
    SwTextNode* pSectionTextNode = lclFindSectionTextNode(*getSwDoc());
    CPPUNIT_ASSERT(pSectionTextNode);
    SwSectionNode* pSectionNode = pSectionTextNode->FindSectionNode();
    CPPUNIT_ASSERT(pSectionNode);

    OUString aText = u"lorem ipsum dolor sit amet lorem ipsum dolor sit amet lorem ipsum dolor sit amet"_ustr;
    lclInsertTextOneCharacterAtATime(*getSwDocShell()->GetWrtShell(), *pSectionTextNode, aText);
    CPPUNIT_ASSERT_EQUAL(aText.getLength(), pSectionTextNode->Len());
    calcLayout();

    const SwRootFrame* pLayout = getSwDoc()->getIDocumentLayoutAccess().GetCurrentLayout();
    CPPUNIT_ASSERT(pLayout);
    SwTextFrame* pFirstTextFrame
        = lclAssertTextFramesInSectionColumns(*pSectionTextNode, *pLayout,
                                              pSectionNode->GetSection(), false, true);
    // Spaces move this input across line boundaries; keep the multi-line coverage without
    // assuming the exact font metrics used by each platform.
    CPPUNIT_ASSERT(pFirstTextFrame->GetThisLines() >= 3);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf169498EmptyBodyDoesNotUseSectionFitCheck)
{
    createSwDoc("tdf169498-sectioncrash.odt");
    SwDoc& rDoc = *getSwDoc();
    SwTextNode* pSectionTextNode = lclFindSectionTextNode(rDoc);
    CPPUNIT_ASSERT(pSectionTextNode);
    SwSectionNode* pSectionNode = pSectionTextNode->FindSectionNode();
    CPPUNIT_ASSERT(pSectionNode);

    SwTextNode* pPrecedingTextNode = nullptr;
    for (SwNodeOffset nIndex(0); nIndex < pSectionNode->GetIndex(); ++nIndex)
    {
        SwTextNode* pTextNode = rDoc.GetNodes()[nIndex]->GetTextNode();
        if (pTextNode && !pTextNode->FindSectionNode())
            pPrecedingTextNode = pTextNode;
    }
    CPPUNIT_ASSERT(pPrecedingTextNode);

    SwPageDesc* pSmallFirstPageDesc
        = rDoc.MakePageDesc(UIName(u"TDF169498SmallFirstPage"_ustr), &rDoc.GetPageDesc(0));
    CPPUNIT_ASSERT(pSmallFirstPageDesc);
    pSmallFirstPageDesc->SetFollow(&rDoc.GetPageDesc(0));
    SwFormatFrameSize aSmallPageSize(pSmallFirstPageDesc->GetMaster().GetFrameSize());
    aSmallPageSize.SetHeight(900);
    pSmallFirstPageDesc->GetMaster().SetFormatAttr(aSmallPageSize);
    pSmallFirstPageDesc->GetMaster().SetFormatAttr(SvxULSpaceItem(0, 0, RES_UL_SPACE));
    pSmallFirstPageDesc->GetMaster().SetFormatAttr(SwFormatHeader(true));
    pSmallFirstPageDesc->GetFirstMaster().SetFormatAttr(aSmallPageSize);
    pSmallFirstPageDesc->GetFirstMaster().SetFormatAttr(SvxULSpaceItem(0, 0, RES_UL_SPACE));
    pSmallFirstPageDesc->GetFirstMaster().SetFormatAttr(SwFormatHeader(true));
    pPrecedingTextNode->SetAttr(SwFormatPageDesc(pSmallFirstPageDesc));
    pPrecedingTextNode->SetAttr(SvxFormatBreakItem(SvxBreak::PageAfter, RES_BREAK));

    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->GetCursor()->DeleteMark();
    pWrtShell->GetCursor()->GetPoint()->Assign(*pPrecedingTextNode, 0);
    pWrtShell->Insert(u"temporary filler"_ustr);
    pSectionTextNode->SetAttr(SvxWidowsItem(3, RES_PARATR_WIDOWS));
    pSectionTextNode->SetAttr(SvxOrphansItem(3, RES_PARATR_ORPHANS));
    pWrtShell->GetCursor()->DeleteMark();
    pWrtShell->GetCursor()->GetPoint()->Assign(*pSectionTextNode, 0);
    pWrtShell->Insert(u"lorem ipsum dolor sit amet lorem ipsum dolor sit amet lorem ipsum dolor sit amet"_ustr);

    calcLayout();
    SwRootFrame* pLayout = rDoc.getIDocumentLayoutAccess().GetCurrentLayout();
    CPPUNIT_ASSERT(pLayout);
    auto pPage1 = pLayout->Lower()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage1);
    CPPUNIT_ASSERT_EQUAL(pSmallFirstPageDesc, pPage1->GetPageDesc());
    auto pPage2 = pPage1->GetNext()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage2);
    CPPUNIT_ASSERT(pPage1->GetHeaderFrame());

    SwLayoutFrame* pPage1Body = pPage1->FindBodyCont();
    CPPUNIT_ASSERT(pPage1Body);
    SwContentFrame* pFillerFrame = pPrecedingTextNode->getLayoutFrame(pLayout);
    CPPUNIT_ASSERT(pFillerFrame);
    CPPUNIT_ASSERT_EQUAL(pPage1Body, pFillerFrame->GetUpper());

    SwContentFrame* pSourceContentFrame = pSectionTextNode->getLayoutFrame(pLayout);
    CPPUNIT_ASSERT(pSourceContentFrame);
    SwTextFrame* pSourceFrame = pSourceContentFrame->DynCastTextFrame();
    CPPUNIT_ASSERT(pSourceFrame);
    CPPUNIT_ASSERT_EQUAL(pPage2, lclFindPageFrame(*pSourceFrame));
    SwSectionFrame* pSourceSection = lclFindSectionFrame(*pSourceFrame);
    CPPUNIT_ASSERT(pSourceSection);
    CPPUNIT_ASSERT(!pSourceSection->GetPrecede());
    SwLayoutFrame* pSourceBody = pSourceFrame->GetUpper();
    CPPUNIT_ASSERT(pSourceBody);
    CPPUNIT_ASSERT(pSourceBody->IsColBodyFrame());
    SwFrame* pSourceColumn = pSourceBody->GetUpper();
    CPPUNIT_ASSERT(pSourceColumn);
    CPPUNIT_ASSERT(pSourceColumn->IsColumnFrame());
    CPPUNIT_ASSERT(!pSourceColumn->GetPrev());

    CPPUNIT_ASSERT_GREATER(sal_Int32(2), pSourceFrame->GetThisLines());
    CPPUNIT_ASSERT_GREATER(pPage1Body->getFramePrintArea().Height(),
                           pSourceFrame->getFrameArea().Height());

    DetachedFrameRestorer aFillerRestorer(*pFillerFrame, *pPage1Body,
                                          pFillerFrame->GetNext());
    pFillerFrame->Cut();
    CPPUNIT_ASSERT(pPage1->GetHeaderFrame());
    CPPUNIT_ASSERT_MESSAGE("page 1 body must be empty before the backward search",
                           !pPage1Body->Lower());

    // Page one cannot hold the multi-line text while meeting the three-line
    // widow and orphan constraints. Recalculation must still move it back
    // across the empty body, where the ordinary fit check would reject it.
    pSourceFrame->InvalidateAll();
    calcLayout();
    SwContentFrame* pAfterContentFrame = pSectionTextNode->getLayoutFrame(pLayout);
    CPPUNIT_ASSERT(pAfterContentFrame);
    SwTextFrame* pAfterTextFrame = pAfterContentFrame->DynCastTextFrame();
    CPPUNIT_ASSERT(pAfterTextFrame);
    while (pAfterTextFrame->GetPrecede())
        pAfterTextFrame = static_cast<SwTextFrame*>(pAfterTextFrame->GetPrecede());
    CPPUNIT_ASSERT_EQUAL(pPage1, lclFindPageFrame(*pAfterTextFrame));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf169498LiveThreeToFourColumnsEmpty)
{
    createSwDoc("tdf169498-sectioncrash.odt");
    SwTextNode* pSectionTextNode = lclFindSectionTextNode(*getSwDoc());
    CPPUNIT_ASSERT(pSectionTextNode);
    SwSectionNode* pSectionNode = pSectionTextNode->FindSectionNode();
    CPPUNIT_ASSERT(pSectionNode);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(3), pSectionNode->GetSection().GetFormat()->GetCol().GetNumCols());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pSectionTextNode->Len());

    calcLayout();
    const SwRootFrame* pLayout = getSwDoc()->getIDocumentLayoutAccess().GetCurrentLayout();
    CPPUNIT_ASSERT(pLayout);
    lclAssertSectionColumnCount(*pSectionTextNode, *pLayout, 3);

    // Changing a laid-out empty section exercises live AdjustColumns handling.
    lclSetSectionColumnCount(*getSwDocShell()->GetWrtShell(), *pSectionNode, 4);
    calcLayout();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pSectionTextNode->Len());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(4), pSectionNode->GetSection().GetFormat()->GetCol().GetNumCols());
    lclAssertSectionColumnCount(*pSectionTextNode, *pLayout, 4);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf169498EmptyColumnOneOnPageTwo)
{
    createSwDoc("tdf169498-sectioncrash.odt");
    SwDoc& rDoc = *getSwDoc();
    SwTextNode* pSourceTextNode = lclFindSectionTextNode(rDoc);
    CPPUNIT_ASSERT(pSourceTextNode);
    SwSectionNode* pSectionNode = pSourceTextNode->FindSectionNode();
    CPPUNIT_ASSERT(pSectionNode);
    pSectionNode->GetSection().SetProtect(false);

    SwTextNode* pPrecedingBodyTextNode = nullptr;
    for (SwNodeOffset nIndex(0); nIndex < pSectionNode->GetIndex(); ++nIndex)
    {
        SwTextNode* pTextNode = rDoc.GetNodes()[nIndex]->GetTextNode();
        if (pTextNode && !pTextNode->FindSectionNode())
            pPrecedingBodyTextNode = pTextNode;
    }
    CPPUNIT_ASSERT(pPrecedingBodyTextNode);
    pPrecedingBodyTextNode->SetAttr(SvxFormatBreakItem(SvxBreak::PageAfter, RES_BREAK));

    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->GetCursor()->DeleteMark();
    pWrtShell->GetCursor()->GetPoint()->Assign(*pSourceTextNode, 0);
    pWrtShell->SplitNode();

    SwTextNode* pMasterTextNode = nullptr;
    for (SwNodeOffset nIndex(0); nIndex < pSourceTextNode->GetIndex(); ++nIndex)
    {
        SwTextNode* pTextNode = rDoc.GetNodes()[nIndex]->GetTextNode();
        if (pTextNode && pTextNode->FindSectionNode() == pSectionNode)
            pMasterTextNode = pTextNode;
    }
    CPPUNIT_ASSERT(pMasterTextNode);
    pMasterTextNode->SetAttr(SvxFormatBreakItem(SvxBreak::PageAfter, RES_BREAK));
    pSourceTextNode->SetAttr(SvxWidowsItem(3, RES_PARATR_WIDOWS));
    pSourceTextNode->SetAttr(SvxOrphansItem(3, RES_PARATR_ORPHANS));

    pWrtShell->GetCursor()->DeleteMark();
    pWrtShell->GetCursor()->GetPoint()->Assign(*pMasterTextNode, 0);
    pWrtShell->Insert(u"page two section master"_ustr);
    OUString aText;
    for (sal_Int32 nIndex = 0; nIndex < 200; ++nIndex)
        aText += u"j"_ustr;
    pWrtShell->GetCursor()->DeleteMark();
    pWrtShell->GetCursor()->GetPoint()->Assign(*pSourceTextNode, 0);
    pWrtShell->Insert(aText);
    calcLayout();

    SwRootFrame* pLayout = rDoc.getIDocumentLayoutAccess().GetCurrentLayout();
    CPPUNIT_ASSERT(pLayout);
    SwPageFrame* pPage1 = pLayout->Lower()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage1);
    SwFrame* pNextPage = pPage1->GetNext();
    CPPUNIT_ASSERT(pNextPage);
    SwPageFrame* pPage2 = pNextPage->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage2);
    pNextPage = pPage2->GetNext();
    CPPUNIT_ASSERT(pNextPage);
    SwPageFrame* pPage3 = pNextPage->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage3);

    SwContentFrame* pMasterContentFrame = pMasterTextNode->getLayoutFrame(pLayout);
    CPPUNIT_ASSERT(pMasterContentFrame);
    CPPUNIT_ASSERT_EQUAL(pPage2, lclFindPageFrame(*pMasterContentFrame));

    SwContentFrame* pContentFrame = pSourceTextNode->getLayoutFrame(pLayout);
    CPPUNIT_ASSERT(pContentFrame);
    SwTextFrame* pSourceFrame = pContentFrame->DynCastTextFrame();
    CPPUNIT_ASSERT(pSourceFrame);
    SwSectionFrame* pFollowSection = lclFindSectionFrame(*pSourceFrame);
    CPPUNIT_ASSERT(pFollowSection);
    CPPUNIT_ASSERT(lclFindPageFrame(*pSourceFrame));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(3), lclFindPageFrame(*pSourceFrame)->GetPhyPageNum());
    CPPUNIT_ASSERT(pFollowSection->GetPrecede());
    auto* pMasterSection = static_cast<SwSectionFrame*>(pFollowSection->GetPrecede());
    CPPUNIT_ASSERT_EQUAL(pFollowSection, pMasterSection->GetFollow());
    CPPUNIT_ASSERT_EQUAL(pPage2, lclFindPageFrame(*pMasterSection));
    CPPUNIT_ASSERT_EQUAL(pPage3, lclFindPageFrame(*pSourceFrame));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(3),
                         pSectionNode->GetSection().GetFormat()->GetCol().GetNumCols());
    lclAssertSectionColumnCount(*pMasterTextNode, *pLayout, 3);
    lclAssertFrameInSectionColumn(*pMasterContentFrame, pSectionNode->GetSection());
    SwLayoutFrame* pMasterColumnBody = pMasterContentFrame->GetUpper();
    CPPUNIT_ASSERT(pMasterColumnBody->IsColBodyFrame());
    SwFrame* pMasterColumn = pMasterColumnBody->GetUpper();
    CPPUNIT_ASSERT(pMasterColumn);
    CPPUNIT_ASSERT(pMasterColumn->IsColumnFrame());
    CPPUNIT_ASSERT(!pMasterColumn->GetPrev());

    DetachedFrameRestorer aMasterRestorer(*pMasterContentFrame, *pMasterContentFrame->GetUpper(),
                                          pMasterContentFrame->GetNext());
    pMasterContentFrame->Cut();
    // The section master is now empty on page two, so backward flow targets its first column.
    for (SwFrame* pColumn = pMasterSection->Lower(); pColumn; pColumn = pColumn->GetNext())
    {
        CPPUNIT_ASSERT(pColumn->IsColumnFrame());
        auto* pColumnBody = static_cast<SwLayoutFrame*>(pColumn->GetLower());
        CPPUNIT_ASSERT(pColumnBody);
        CPPUNIT_ASSERT(pColumnBody->IsColBodyFrame());
        CPPUNIT_ASSERT(!pColumnBody->Lower());
    }
    SwLayoutFrame* pSourceColumnBody = pSourceFrame->GetUpper();
    CPPUNIT_ASSERT(pSourceColumnBody);
    CPPUNIT_ASSERT(pSourceColumnBody->IsColBodyFrame());
    SwFrame* pSourceColumn = pSourceColumnBody->GetUpper();
    CPPUNIT_ASSERT(pSourceColumn);
    CPPUNIT_ASSERT(pSourceColumn->IsColumnFrame());
    CPPUNIT_ASSERT(!pSourceColumn->GetPrev());

    pSourceFrame->InvalidateAll();
    calcLayout();

    SwContentFrame* pAfterContentFrame = pSourceTextNode->getLayoutFrame(pLayout);
    CPPUNIT_ASSERT(pAfterContentFrame);
    SwTextFrame* pAfterTextFrame = pAfterContentFrame->DynCastTextFrame();
    CPPUNIT_ASSERT(pAfterTextFrame);
    CPPUNIT_ASSERT(pAfterTextFrame->GetUpper());
    aMasterRestorer.RestoreAt(*pAfterTextFrame->GetUpper(), pAfterTextFrame);
    while (pAfterTextFrame->GetPrecede())
        pAfterTextFrame = static_cast<SwTextFrame*>(pAfterTextFrame->GetPrecede());
    CPPUNIT_ASSERT(!pAfterTextFrame->GetFollow());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(200), pSourceTextNode->Len());
    CPPUNIT_ASSERT_EQUAL(pPage2, lclFindPageFrame(*pAfterTextFrame));
    lclAssertFrameInSectionColumn(*pAfterTextFrame, pSectionNode->GetSection());
    SwFrame* pAfterColumn = pAfterTextFrame->GetUpper()->GetUpper();
    CPPUNIT_ASSERT(pAfterColumn);
    CPPUNIT_ASSERT(pAfterColumn->IsColumnFrame());
    CPPUNIT_ASSERT(!pAfterColumn->GetPrev());
}

CPPUNIT_TEST_FIXTURE(Test, testSplitFlyNextRowInvalidatePos)
{
    // Given a multi-page floating table, row1 is split, i.e. is both on page 1 and page 2:
    createSwDoc("floattable-next-row-invalidate-pos.docx");
    // Make sure the follow anchor's IsCompletePaint() reaches its false state, as it happens in the
    // interactive case.
    Scheduler::ProcessEventsToIdle();
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage1 = pLayout->Lower()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage1);
    auto pPage2 = pPage1->GetNext()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage2);
    CPPUNIT_ASSERT(pPage2->GetSortedObjs());
    SwSortedObjs& rPage2Objs = *pPage2->GetSortedObjs();
    auto pFly2 = rPage2Objs[0]->DynCastFlyFrame();
    auto pTable2 = pFly2->GetLower()->DynCastTabFrame();
    auto pRow2 = pTable2->GetLastLower()->DynCastRowFrame();
    SwTwips nOldRow2Top = pRow2->getFrameArea().Top();

    // When adding a new paragraph at the end of B1:
    // Go to the table: A1 cell.
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->GotoTable(UIName(u"Table1"_ustr));
    // Go to the column: B1 cell.
    pWrtShell->GoNextCell();
    // Go to the end of the B1 cell, on page 2.
    pWrtShell->EndOfSection();
    // Add a new paragraph at the cell end.
    pWrtShell->SplitNode();

    // Then make sure row 2 is shifted down:
    SwTwips nNewRow2Top = pRow2->getFrameArea().Top();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected greater than: 7121
    // - Actual  : 7121
    // i.e. row 2 has to be shifted down to 7390, but this didn't happen.
    CPPUNIT_ASSERT_GREATER(nOldRow2Top, nNewRow2Top);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf157096)
{
    createSwDoc("tdf157096.docx");

    CPPUNIT_ASSERT_EQUAL(2, getPages());
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    // Without the fix in place, it would have crashed here
    dispatchCommand(mxComponent, u".uno:Delete"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

CPPUNIT_TEST_FIXTURE(Test, testSplitFlyInSection)
{
    // Given a document with multiple sections, the 2nd section on page 1 has a one-page floating
    // table:
    createSwDoc("floattable-in-section.docx");

    // When laying out that document:
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();

    // Then make sure the table is on page 1, not on page 2:
    auto pPage1 = pLayout->Lower()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage1);
    // Without the fix in place, it would have failed, the table was on page 2, not on page 1.
    CPPUNIT_ASSERT(pPage1->GetSortedObjs());
    SwSortedObjs& rPage1Objs = *pPage1->GetSortedObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPage1Objs.size());
    auto pPage2 = pPage1->GetNext()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage2);
    CPPUNIT_ASSERT(!pPage2->GetSortedObjs());
}

CPPUNIT_TEST_FIXTURE(Test, testBadSplitSection)
{
    // Given a document with a section, containing 5 paragraphs:
    createSwDoc("bad-split-section.odt");

    // When laying out that document:
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();

    // Then make sure the entire section is on page 1:
    auto pPage = pLayout->Lower()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage);
    auto pBody = pPage->FindBodyCont();
    CPPUNIT_ASSERT(pBody);
    auto pSection = dynamic_cast<SwSectionFrame*>(pBody->GetLastLower());
    CPPUNIT_ASSERT(pSection);
    // Without the fix in place, it would have failed, the section was split between page 1 and page
    // 2.
    CPPUNIT_ASSERT(!pSection->GetFollow());
}

/// anyInput callback that doesn't interrupt for high-priority tasks.
class PriorityAwareAnyInputCallback final
{
public:
    static bool callback(void* /*pData*/, int nPriority)
    {
        // Only interrupt if all ready tasks are low priority.
        return nPriority < 0 || nPriority > static_cast<int>(TaskPriority::REPAINT);
    }

    PriorityAwareAnyInputCallback()
    {
        comphelper::COKit::setAnyInputCallback(&callback, this,
                                               Scheduler::GetMostUrgentTaskPriority);
    }

    ~PriorityAwareAnyInputCallback()
    {
        comphelper::COKit::setAnyInputCallback(nullptr, nullptr, []() -> int { return -1; });
    }
};

CPPUNIT_TEST_FIXTURE(Test, testIdleLayoutingAnyInput)
{
    // Set up COKit:
    comphelper::COKit::setActive(true);

    // Given a document with 3 pages, the first page is visible:
    createSwDoc();
    getSwTextDoc()->initializeForTiledRendering(cpo::uno::Sequence<beans::PropertyValue>());
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->InsertPageBreak();
    pWrtShell->InsertPageBreak();
    SwRootFrame* pLayout = pWrtShell->GetLayout();
    SwPageFrame* pPage1 = pLayout->GetLower()->DynCastPageFrame();
    pWrtShell->setKitVisibleArea(pPage1->getFrameArea().SVRect());
    // Visible page is calculated, the rest is not:
    pWrtShell->StartAllAction();
    pPage1->InvalidateContent();
    SwPageFrame* pPage2 = pPage1->GetNext()->DynCastPageFrame();
    pPage2->InvalidateContent();
    SwPageFrame* pPage3 = pPage2->GetNext()->DynCastPageFrame();
    pPage3->InvalidateContent();
    pWrtShell->EndAllAction();
    CPPUNIT_ASSERT(!pPage1->IsInvalidContent());
    CPPUNIT_ASSERT(pPage2->IsInvalidContent());
    CPPUNIT_ASSERT(pPage3->IsInvalidContent());

    // When idle layout runs and we have a scheduled high priority task together with an any input
    // callback:
    Idle aHighPrioTask("test task");
    aHighPrioTask.SetPriority(TaskPriority::DEFAULT);
    aHighPrioTask.Start();
    PriorityAwareAnyInputCallback aAnyInput;
    pWrtShell->LayoutIdle();

    // Then make sure async layout calculates page 2 but stops before page 3:
    CPPUNIT_ASSERT(!pPage1->IsInvalidContent());
#ifndef _WIN32 // FIXME: Fails on Windows
    CPPUNIT_ASSERT(!pPage2->IsInvalidContent());
#endif
    // Without the fix in place, the idle layout would calculate all pages because the
    // high-priority task caused the any input callback not to interrupt.
    CPPUNIT_ASSERT(pPage3->IsInvalidContent());

    // Tear down COKit:
    aHighPrioTask.Stop();
    Scheduler::ProcessEventsToIdle();
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
