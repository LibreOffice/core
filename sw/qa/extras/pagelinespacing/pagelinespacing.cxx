/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <anchoredobject.hxx>
#include <docsh.hxx>
#include <drawdoc.hxx>
#include <edtwin.hxx>
#include <flyfrm.hxx>
#include <frame.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <pagefrm.hxx>
#include <porlay.hxx>
#include <rootfrm.hxx>
#include <sortedobjs.hxx>
#include <swmodeltestbase.hxx>
#include <txtfrm.hxx>
#include <wrtsh.hxx>

#include <svx/svdpage.hxx>
#include <vcl/event.hxx>
#include <vcl/scheduler.hxx>

namespace
{
class SwPageLineSpacingTest : public SwModelTestBase
{
public:
    SwPageLineSpacingTest()
        : SwModelTestBase(u"/sw/qa/extras/pagelinespacing/data/"_ustr)
    {
    }

protected:
    void checkTextAlignedToBaselineGrid(const bool bAligned = true);
    void checkTextAlignedToBaselineGrid(SwFrame* pFrame, const bool bAligned);
    void checkTextAlignedToBaselineGrid(SwTextFrame* pTextFrame, const bool bAligned);

    void applyPageLineSpacing(const uint16_t nPage, const bool bEnable,
                              const OUString& rReferenceStyle);

    void applyPageLineSpacing(const OUString& rParagraphStyle);

    Size getTextFrameSize(const uint16_t nTextFrame, const OUString& rTextContent);

    void setLineHeightForReferenceStyle(const uint16_t nPage, const sal_uInt16 nLineHeight);

    void moveObject(SdrObject* pObject, const sal_uInt16 nKey);
    void resizeObjectUpward(SdrObject* pObject, const SwTwips nDragDistance);

    SdrObject* findObject(const uint16_t nPage, const uint16_t nFlyFrame);
};

void SwPageLineSpacingTest::checkTextAlignedToBaselineGrid(const bool bAligned)
{
    SwDocShell* pDocShell = getSwDocShell();
    CPPUNIT_ASSERT(pDocShell);

    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    SwRootFrame* pRoot = pWrtShell->GetLayout();
    CPPUNIT_ASSERT(pRoot);

    checkTextAlignedToBaselineGrid(pRoot, bAligned);
}

void SwPageLineSpacingTest::checkTextAlignedToBaselineGrid(SwFrame* pFrame, const bool bAligned)
{
    CPPUNIT_ASSERT(pFrame);

    // Check the text alignment within text frames.
    if (pFrame->IsTextFrame())
    {
        auto pTextFrame = dynamic_cast<SwTextFrame*>(pFrame);
        CPPUNIT_ASSERT(pTextFrame);
        checkTextAlignedToBaselineGrid(pTextFrame, bAligned);
    }

    // Also check the content of fly frames.
    if (pFrame->IsPageFrame())
    {
        auto pPageFrame = dynamic_cast<SwPageFrame*>(pFrame);
        CPPUNIT_ASSERT(pPageFrame);
        const SwSortedObjs* pObjects = pPageFrame->GetSortedObjs();
        if (pObjects)
        {
            for (SwAnchoredObject* pObject : *pObjects)
            {
                SwFlyFrame* pFlyFrame = pObject->DynCastFlyFrame();
                if (pFlyFrame)
                {
                    checkTextAlignedToBaselineGrid(pFlyFrame, bAligned);
                }
            }
        }
    }

    // Header and footer are not expected to be aligned with the baseline grid.
    bool bHeaderFooter = pFrame->IsHeaderFrame() || pFrame->IsFooterFrame();

    // Recursively check the content of lower / next frames.
    if (pFrame->GetLower() && !bHeaderFooter)
    {
        checkTextAlignedToBaselineGrid(pFrame->GetLower(), bAligned);
    }

    if (pFrame->GetNext())
    {
        checkTextAlignedToBaselineGrid(pFrame->GetNext(), bAligned);
    }
}

void SwPageLineSpacingTest::checkTextAlignedToBaselineGrid(SwTextFrame* pTextFrame,
                                                           const bool bAligned)
{
    CPPUNIT_ASSERT(pTextFrame);

    // First identify the parameters of the baseline grid.
    sal_uInt16 nGridOffset = 0;
    sal_uInt16 nGridDistance = 0;
    {
        const SwPageFrame* pPage = pTextFrame->FindPageFrame();
        CPPUNIT_ASSERT(pPage);
        const SwPageDesc* pPageDesc = pPage->GetPageDesc();
        CPPUNIT_ASSERT(pPageDesc);

        const SwTextFormatColl* pRegisterFormat = pPageDesc->GetRegisterFormatColl();
        if (!pRegisterFormat)
        {
            CPPUNIT_ASSERT(!bAligned);
            return;
        }
        pPage->ComputeRegister(pRegisterFormat, nGridDistance, nGridOffset);

        const SwLayoutFrame* pBody = pPage->FindBodyCont();
        CPPUNIT_ASSERT(pBody);

        nGridOffset = pBody->getFrameArea().Pos().Y() + nGridOffset;
    }

    // Make sure the first line of the paragraph is aligned with the baseline grid.
    SwLineLayout* pPara = pTextFrame->GetPara();
    if (!pPara)
    {
        return;
    }
    const SwTwips nLineAscent = pPara->GetAscent();
    const SwTwips nHeightDiff = pPara->GetRealHeight() - pPara->Height();
    const SwTwips nBaseline = pTextFrame->getFrameArea().Top()
                              + pTextFrame->getFramePrintArea().Top() + nLineAscent + nHeightDiff;
    const SwTwips nRemainder = (nBaseline - nGridOffset) % nGridDistance;
    if (bAligned)
    {
        CPPUNIT_ASSERT_EQUAL(SwTwips(0), nRemainder);
    }
    else
    {
        CPPUNIT_ASSERT(nRemainder != 0);
    }

    // All other lines should have a height which is a multiplier of the grid distance
    // to have them also aligned properly to the baseline grid.
    pPara = pPara->GetNext();
    while (pPara)
    {
        if (bAligned)
        {
            CPPUNIT_ASSERT_EQUAL(SwTwips(0), pPara->GetRealHeight() % nGridDistance);
        }
        else
        {
            CPPUNIT_ASSERT(pPara->GetRealHeight() % nGridDistance != 0);
        }
        pPara = pPara->GetNext();
    }
}

void SwPageLineSpacingTest::applyPageLineSpacing(const uint16_t nPage, const bool bEnable,
                                                 const OUString& rReferenceStyle)
{
    SwDocShell* pDocShell = getSwDocShell();
    CPPUNIT_ASSERT(pDocShell);

    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    SwRootFrame* pRoot = pWrtShell->GetLayout();
    CPPUNIT_ASSERT(pRoot);

    SwFrame* pNextFrame = pRoot;
    while (pNextFrame)
    {
        if (pNextFrame->IsPageFrame())
        {
            uint16_t nCurrentPage = 1;
            while (pNextFrame)
            {
                // Find the correct page.
                if (nCurrentPage == nPage)
                {
                    auto pPageFrame = dynamic_cast<SwPageFrame*>(pNextFrame);
                    CPPUNIT_ASSERT(pPageFrame);
                    SwPageDesc* pPageDesc = pPageFrame->GetPageDesc();
                    CPPUNIT_ASSERT(pPageDesc);
                    SwDoc* pDoc = getSwDoc();
                    CPPUNIT_ASSERT(pDoc);
                    if (bEnable)
                    {
                        SwTextFormatColl* pFormat
                            = pDoc->FindTextFormatCollByName(UIName(rReferenceStyle));
                        pPageDesc->SetRegisterFormatColl(pFormat);
                    }
                    else
                    {
                        pPageDesc->SetRegisterFormatColl(nullptr);
                    }
                    break;
                }
                pNextFrame = pNextFrame->GetNext();
                nCurrentPage += 1;
            };
            break;
        }
        pNextFrame = pNextFrame->GetLower();
    };
    calcLayout();
}

void SwPageLineSpacingTest::applyPageLineSpacing(const OUString& rParagraphStyle)
{
    SwDocShell* pDocShell = getSwDocShell();
    CPPUNIT_ASSERT(pDocShell);

    SwDoc* pDoc = pDocShell->GetDoc();
    CPPUNIT_ASSERT(pDoc);

    // Enable page line-spacing for the given paragraph style.
    {
        SwTextFormatColl* pTextFormat = pDoc->FindTextFormatCollByName(UIName(rParagraphStyle));
        CPPUNIT_ASSERT(pTextFormat);

        const SwAttrSet& rAttrSet = pTextFormat->GetAttrSet();
        SwRegisterItem aRegisterItem = rAttrSet.GetRegister();
        aRegisterItem.SetValue(true);
        CPPUNIT_ASSERT_EQUAL(true, aRegisterItem.GetValue());

        std::unique_ptr<SfxItemSet> pNewSet = rAttrSet.Clone();
        pNewSet->Put(aRegisterItem);
        pDoc->ChgFormat(*pTextFormat, *pNewSet);
        calcLayout();
    }

    // Verify the paragraph style was updated properly.
    {
        SwTextFormatColl* pTextFormat = pDoc->FindTextFormatCollByName(UIName(rParagraphStyle));
        const SwAttrSet& rAttrSet = pTextFormat->GetAttrSet();
        CPPUNIT_ASSERT_EQUAL(true, rAttrSet.GetRegister().GetValue());
    }
}

Size SwPageLineSpacingTest::getTextFrameSize(const uint16_t nTextFrame,
                                             const OUString& rTextContent)
{
    SwDocShell* pDocShell = getSwDocShell();
    CPPUNIT_ASSERT(pDocShell);

    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    SwRootFrame* pRoot = pWrtShell->GetLayout();
    CPPUNIT_ASSERT(pRoot);

    SwFrame* pNextFrame = pRoot;
    bool bLowerFinished = false;
    uint16_t nCurrentTextFrame = 1;
    while (pNextFrame)
    {
        if (pNextFrame->IsTextFrame())
        {
            if (nCurrentTextFrame == nTextFrame)
            {
                auto pTextFrame = dynamic_cast<SwTextFrame*>(pNextFrame);
                CPPUNIT_ASSERT(pTextFrame);
                CPPUNIT_ASSERT_EQUAL(rTextContent, pTextFrame->GetText());
                return pTextFrame->getFrameArea().SSize();
            }
            nCurrentTextFrame += 1;
        }

        // Traverse to the next frame in the layout.
        if (pNextFrame->GetLower() && !bLowerFinished)
        {
            pNextFrame = pNextFrame->GetLower();
        }
        else if (pNextFrame->GetNext())
        {
            pNextFrame = pNextFrame->GetNext();
            bLowerFinished = false;
        }
        else
        {
            pNextFrame = pNextFrame->GetUpper();
            bLowerFinished = true;
        }
    };

    return { 0, 0 };
}

void SwPageLineSpacingTest::setLineHeightForReferenceStyle(const uint16_t nPage,
                                                           const sal_uInt16 nLineHeight)
{
    SwDocShell* pDocShell = getSwDocShell();
    CPPUNIT_ASSERT(pDocShell);

    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    SwDoc* pDoc = pDocShell->GetDoc();
    CPPUNIT_ASSERT(pDoc);

    SwFrame* pNextFrame = pWrtShell->GetLayout();
    while (pNextFrame)
    {
        if (pNextFrame->IsPageFrame())
        {
            uint16_t nCurrentPage = 1;
            while (pNextFrame)
            {
                // Find the correct page.
                if (nCurrentPage == nPage)
                {
                    CPPUNIT_ASSERT(pNextFrame->IsPageFrame());
                    auto pPageFrame = dynamic_cast<SwPageFrame*>(pNextFrame);
                    CPPUNIT_ASSERT(pPageFrame);
                    SwPageDesc* pPageDesc = pPageFrame->GetPageDesc();
                    CPPUNIT_ASSERT(pPageDesc);
                    // Modify the reference style of the given page style.
                    {
                        const SwTextFormatColl* pRegisterFormat
                            = pPageDesc->GetRegisterFormatColl();
                        CPPUNIT_ASSERT(pRegisterFormat);
                        SwTextFormatColl* pTextFormat
                            = pDoc->FindTextFormatCollByName(pRegisterFormat->GetName());
                        CPPUNIT_ASSERT(pTextFormat);

                        const SwAttrSet& rAttrSet = pTextFormat->GetAttrSet();
                        SvxLineSpacingItem aLineSpacingItem = rAttrSet.GetLineSpacing();
                        aLineSpacingItem.SetLineHeight(nLineHeight);
                        aLineSpacingItem.SetLineSpaceRule(SvxLineSpaceRule::Fix);
                        CPPUNIT_ASSERT_EQUAL(nLineHeight, aLineSpacingItem.GetLineHeight());
                        CPPUNIT_ASSERT_EQUAL(SvxLineSpaceRule::Fix,
                                             aLineSpacingItem.GetLineSpaceRule());

                        std::unique_ptr<SfxItemSet> pNewSet = rAttrSet.Clone();
                        pNewSet->Put(aLineSpacingItem);
                        pDoc->ChgFormat(*pTextFormat, *pNewSet);
                    }

                    // Verify the reference style was updated properly.
                    {
                        const SwTextFormatColl* pRegisterFormat
                            = pPageDesc->GetRegisterFormatColl();
                        CPPUNIT_ASSERT(pRegisterFormat);
                        const SwAttrSet& rAttrSet = pRegisterFormat->GetAttrSet();
                        const SvxLineSpacingItem& aLineSpacingItem = rAttrSet.GetLineSpacing();
                        CPPUNIT_ASSERT_EQUAL(nLineHeight, aLineSpacingItem.GetLineHeight());
                        CPPUNIT_ASSERT_EQUAL(SvxLineSpaceRule::Fix,
                                             aLineSpacingItem.GetLineSpaceRule());
                    }
                    break;
                }
                pNextFrame = pNextFrame->GetNext();
                nCurrentPage += 1;
            };
            break;
        }
        pNextFrame = pNextFrame->GetLower();
    };
    calcLayout();
}

void SwPageLineSpacingTest::moveObject(SdrObject* pObject, const sal_uInt16 nKey)
{
    CPPUNIT_ASSERT(pObject);

    // Save the original position of the object to verify movement.
    const Point aOriginalPos = pObject->GetLastBoundRect().TopLeft();

    SwDocShell* pDocShell = getSwDocShell();
    CPPUNIT_ASSERT(pDocShell);

    // First select the object.
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    bool bSelected = pWrtShell->SelectObj(Point(), 0, pObject);
    CPPUNIT_ASSERT(bSelected);

    // Then move it using keyboard events.
    SwView* pView = pDocShell->GetView();
    CPPUNIT_ASSERT(pView);

    SwEditWin& rEditWin = pView->GetEditWin();

    KeyEvent aKeyEvent(0, nKey);
    rEditWin.KeyInput(aKeyEvent);
    Scheduler::ProcessEventsToIdle();

    // Verify movement has actually happened.
    const Point aNewPos = pObject->GetLastBoundRect().TopLeft();
    switch (nKey)
    {
        case KEY_UP:
            CPPUNIT_ASSERT_EQUAL(aOriginalPos.X(), aNewPos.X());
            CPPUNIT_ASSERT(aNewPos.Y() < aOriginalPos.Y());
            break;
        case KEY_DOWN:
            CPPUNIT_ASSERT_EQUAL(aOriginalPos.X(), aNewPos.X());
            CPPUNIT_ASSERT(aNewPos.Y() > aOriginalPos.Y());
            break;
        case KEY_LEFT:
            CPPUNIT_ASSERT_EQUAL(aOriginalPos.Y(), aNewPos.Y());
            CPPUNIT_ASSERT(aNewPos.X() < aOriginalPos.X());
            break;
        case KEY_RIGHT:
            CPPUNIT_ASSERT_EQUAL(aOriginalPos.Y(), aNewPos.Y());
            CPPUNIT_ASSERT(aNewPos.X() > aOriginalPos.X());

            break;
        default:
            CPPUNIT_ASSERT(false);
            break;
    }
}

void SwPageLineSpacingTest::resizeObjectUpward(SdrObject* pObject, const SwTwips nDragDistance)
{
    CPPUNIT_ASSERT(pObject);

    // Save the original size of the object to verify resizing.
    const Size aOriginalSize = pObject->GetLastBoundRect().GetSize();

    SwDocShell* pDocShell = getSwDocShell();
    CPPUNIT_ASSERT(pDocShell);
    SwView* pView = pDocShell->GetView();
    CPPUNIT_ASSERT(pView);

    // Do the resizing using drag & drop on the top drag point of the object.
    const Point aDragPoint = pObject->GetCurrentBoundRect().TopCenter();
    const Point aDestinationPoint(aDragPoint.X(), aDragPoint.Y() - nDragDistance);

    vcl::Window& rEditWin = pView->GetEditWin();
    const Point aFromPixels = rEditWin.LogicToPixel(aDragPoint);
    const Point aToPixels = rEditWin.LogicToPixel(aDestinationPoint);

    const MouseEvent aClickEvent(aFromPixels, 1, MouseEventModifiers::SIMPLECLICK, MOUSE_LEFT);
    rEditWin.MouseButtonDown(aClickEvent);
    const MouseEvent aMoveEvent(aToPixels, 0, MouseEventModifiers::SIMPLEMOVE, MOUSE_LEFT);
    rEditWin.MouseMove(aMoveEvent);
    rEditWin.MouseMove(aMoveEvent);
    const MouseEvent aReleaseEvent(aToPixels, 1, MouseEventModifiers::SIMPLECLICK, MOUSE_LEFT);
    rEditWin.MouseButtonUp(aReleaseEvent);
    Scheduler::ProcessEventsToIdle();

    // Verify resizing has actually happened.
    const Size aNewSize = pObject->GetLastBoundRect().GetSize();
    CPPUNIT_ASSERT_EQUAL(aOriginalSize.Width(), aNewSize.Width());
    CPPUNIT_ASSERT(aNewSize.Height() > aOriginalSize.Height());
}

SdrObject* SwPageLineSpacingTest::findObject(const uint16_t nPage, const uint16_t nFlyFrame)
{
    SwDocShell* pDocShell = getSwDocShell();
    CPPUNIT_ASSERT(pDocShell);

    SwDoc* pDoc = pDocShell->GetDoc();
    CPPUNIT_ASSERT(pDoc);

    SwDrawModel* pDrawModel = pDoc->getIDocumentDrawModelAccess().GetDrawModel();
    CPPUNIT_ASSERT(pDrawModel);

    SdrPage* pPage = pDrawModel->GetPage(nPage - 1);
    CPPUNIT_ASSERT(pPage);

    SdrObject* pObject = pPage->GetObj(nFlyFrame - 1);
    CPPUNIT_ASSERT(pObject);

    return pObject;
}

CPPUNIT_TEST_FIXTURE(SwPageLineSpacingTest, testMultipleParagraphs)
{
    createSwDoc("multipleParagraphs.fodt");

    checkTextAlignedToBaselineGrid();
}

CPPUNIT_TEST_FIXTURE(SwPageLineSpacingTest, testMultiplePages)
{
    createSwDoc("multiplePages.fodt");

    checkTextAlignedToBaselineGrid();
}

CPPUNIT_TEST_FIXTURE(SwPageLineSpacingTest, testDoubleLineSpacing)
{
    createSwDoc("doubleLineSpacing.fodt");

    checkTextAlignedToBaselineGrid();
}

CPPUNIT_TEST_FIXTURE(SwPageLineSpacingTest, testProportionalLineSpacing)
{
    createSwDoc("proportionalLineSpacing.fodt");

    checkTextAlignedToBaselineGrid();
}

CPPUNIT_TEST_FIXTURE(SwPageLineSpacingTest, testAtLeastLineSpacing)
{
    createSwDoc("atLeastLineSpacing.fodt");

    checkTextAlignedToBaselineGrid();
}

CPPUNIT_TEST_FIXTURE(SwPageLineSpacingTest, testLeadingLineSpacing)
{
    createSwDoc("leadingLineSpacing.fodt");

    checkTextAlignedToBaselineGrid();
}

CPPUNIT_TEST_FIXTURE(SwPageLineSpacingTest, testFixedLineSpacing)
{
    createSwDoc("fixedLineSpacing.fodt");

    checkTextAlignedToBaselineGrid();
}

CPPUNIT_TEST_FIXTURE(SwPageLineSpacingTest, testTitleAsReferenceStyle)
{
    createSwDoc("titleAsReferenceStyle.fodt");

    checkTextAlignedToBaselineGrid();
}

CPPUNIT_TEST_FIXTURE(SwPageLineSpacingTest, testSimpleSection)
{
    createSwDoc("simpleSection.fodt");

    checkTextAlignedToBaselineGrid();
}

CPPUNIT_TEST_FIXTURE(SwPageLineSpacingTest, testSectionWithColumns)
{
    // tdf#170000: Text was not properly aligned to the baseline grid inside a section with columns.
    createSwDoc("sectionWithColumns.fodt");

    checkTextAlignedToBaselineGrid();
}

CPPUNIT_TEST_FIXTURE(SwPageLineSpacingTest, testPageColumns)
{
    createSwDoc("pageColumns.fodt");

    checkTextAlignedToBaselineGrid();
}

CPPUNIT_TEST_FIXTURE(SwPageLineSpacingTest, testFrame)
{
    // tdf#93785: Text was not properly aligned to the baseline grid inside a fly frame.
    createSwDoc("frame.fodt");

    checkTextAlignedToBaselineGrid();
}

CPPUNIT_TEST_FIXTURE(SwPageLineSpacingTest, testMultiplePageStyles)
{
    createSwDoc("multiplePageStyles.fodt");

    checkTextAlignedToBaselineGrid();
}

CPPUNIT_TEST_FIXTURE(SwPageLineSpacingTest, testBulletsAndNumbering)
{
    createSwDoc("bulletsAndNumbering.fodt");

    checkTextAlignedToBaselineGrid();
}

CPPUNIT_TEST_FIXTURE(SwPageLineSpacingTest, testVariousCharacterProperties)
{
    createSwDoc("variousCharacterProperties.fodt");

    checkTextAlignedToBaselineGrid();
}

CPPUNIT_TEST_FIXTURE(SwPageLineSpacingTest, testApplyPageLineSpacing)
{
    createSwDoc("pageLineSpacingDisabled.fodt");

    checkTextAlignedToBaselineGrid(false);

    applyPageLineSpacing(1, true, u"Body Text"_ustr);

    checkTextAlignedToBaselineGrid();

    applyPageLineSpacing(1, false, "");

    checkTextAlignedToBaselineGrid(false);
}

CPPUNIT_TEST_FIXTURE(SwPageLineSpacingTest, testChangeReferenceStyle)
{
    createSwDoc("multipleParagraphs.fodt");

    checkTextAlignedToBaselineGrid();

    applyPageLineSpacing(1, true, u"Title"_ustr);

    checkTextAlignedToBaselineGrid();

    applyPageLineSpacing(1, true, u"Body Text"_ustr);

    checkTextAlignedToBaselineGrid();
}

CPPUNIT_TEST_FIXTURE(SwPageLineSpacingTest, testRemovePage)
{
    // tdf#169922: Text was not properly aligned to the baseline grid after removing a page.
    createSwDoc("removePage.fodt");

    checkTextAlignedToBaselineGrid();

    // Remove the first (empty) page.
    SwDocShell* pDocShell = getSwDocShell();
    CPPUNIT_ASSERT(pDocShell);
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);
    pWrtShell->SttEndDoc(true);
    pWrtShell->DelRight();

    checkTextAlignedToBaselineGrid();
}

CPPUNIT_TEST_FIXTURE(SwPageLineSpacingTest, testVerticalTextInsideTable)
{
    // tdf#169821: Top-to-bottom vertical text disappeared from the table cell.
    createSwDoc("verticalTextInsideTable.fodt");

    // The original size should not change after applying page line-spacing.
    const Size aBottomToTopSize = getTextFrameSize(1, u"BottomToTop"_ustr);
    const Size aTopToBottomSize = getTextFrameSize(2, u"TopToBottom"_ustr);

    applyPageLineSpacing(1, true, u"Body Text"_ustr);

    const Size aNewTopToBottomSize = getTextFrameSize(2, u"TopToBottom"_ustr);
    // The width was negative, so the text was invisible.
    CPPUNIT_ASSERT(aNewTopToBottomSize.Width() > 0);
    CPPUNIT_ASSERT_EQUAL(aBottomToTopSize, getTextFrameSize(1, u"BottomToTop"_ustr));
    CPPUNIT_ASSERT_EQUAL(aTopToBottomSize, aNewTopToBottomSize);

    applyPageLineSpacing(1, false, "");

    CPPUNIT_ASSERT_EQUAL(aBottomToTopSize, getTextFrameSize(1, u"BottomToTop"_ustr));
    CPPUNIT_ASSERT_EQUAL(aTopToBottomSize, getTextFrameSize(2, u"TopToBottom"_ustr));
}

CPPUNIT_TEST_FIXTURE(SwPageLineSpacingTest, testVerticalPageStyle)
{
    createSwDoc("verticalPageStyle.fodt");

    // The original size should not change after applying page line-spacing.
    const Size aOriginalSize = getTextFrameSize(1, u"RightToLeft"_ustr);

    applyPageLineSpacing(1, true, u"Body Text"_ustr);

    CPPUNIT_ASSERT_EQUAL(aOriginalSize, getTextFrameSize(1, u"RightToLeft"_ustr));

    applyPageLineSpacing(1, false, "");

    CPPUNIT_ASSERT_EQUAL(aOriginalSize, getTextFrameSize(1, u"RightToLeft"_ustr));
}

CPPUNIT_TEST_FIXTURE(SwPageLineSpacingTest, testApplyPageLineSpacingOnTable)
{
    createSwDoc("tableWithTopAlignment.fodt");

    checkTextAlignedToBaselineGrid();

    applyPageLineSpacing(1, false, "");

    checkTextAlignedToBaselineGrid(false);

    applyPageLineSpacing(1, true, u"Body Text"_ustr);

    checkTextAlignedToBaselineGrid();
}

CPPUNIT_TEST_FIXTURE(SwPageLineSpacingTest, testActivatePageLineSpacingForParagraphStyle)
{
    createSwDoc("pageLineSpacingDisabledParagraph.fodt");

    checkTextAlignedToBaselineGrid(false);

    applyPageLineSpacing(u"Title"_ustr);

    checkTextAlignedToBaselineGrid();
}

CPPUNIT_TEST_FIXTURE(SwPageLineSpacingTest, testModifyReferenceStyle)
{
    createSwDoc("multipleParagraphs.fodt");

    checkTextAlignedToBaselineGrid();

    setLineHeightForReferenceStyle(1, 400);

    checkTextAlignedToBaselineGrid();
}

CPPUNIT_TEST_FIXTURE(SwPageLineSpacingTest, testFrameWithColumns)
{
    createSwDoc("frameWithColumns.fodt");

    checkTextAlignedToBaselineGrid();
}

CPPUNIT_TEST_FIXTURE(SwPageLineSpacingTest, testFloatingTable)
{
    createSwDoc("floatingTable.fodt");

    checkTextAlignedToBaselineGrid();
}

CPPUNIT_TEST_FIXTURE(SwPageLineSpacingTest, testMoveFrame)
{
    createSwDoc("frame.fodt");

    checkTextAlignedToBaselineGrid();

    SdrObject* pFlyFrame = findObject(1, 1);

    // Text should still be aligned with the baseline grid after moving the frame.
    moveObject(pFlyFrame, KEY_DOWN);
    checkTextAlignedToBaselineGrid();

    moveObject(pFlyFrame, KEY_RIGHT);
    checkTextAlignedToBaselineGrid();

    moveObject(pFlyFrame, KEY_UP);
    checkTextAlignedToBaselineGrid();

    moveObject(pFlyFrame, KEY_LEFT);
    checkTextAlignedToBaselineGrid();
}

CPPUNIT_TEST_FIXTURE(SwPageLineSpacingTest, testMultiPageFloatingTable)
{
    createSwDoc("multiPageFloatingTable.fodt");

    checkTextAlignedToBaselineGrid();
}

CPPUNIT_TEST_FIXTURE(SwPageLineSpacingTest, testFrameOutsideOfPrintArea)
{
    createSwDoc("frameOutsideOfPrintArea.fodt");

    checkTextAlignedToBaselineGrid();
}

CPPUNIT_TEST_FIXTURE(SwPageLineSpacingTest, testResizeFrame)
{
    createSwDoc("frame.fodt");

    checkTextAlignedToBaselineGrid();

    // Text should still be aligned with the baseline grid after resizing the frame.
    SdrObject* pFlyFrame = findObject(1, 1);
    const SwTwips nDragDistance = 100;
    resizeObjectUpward(pFlyFrame, nDragDistance);

    checkTextAlignedToBaselineGrid();
}

CPPUNIT_TEST_FIXTURE(SwPageLineSpacingTest, testHeaderFooter)
{
    createSwDoc("headerFooter.fodt");

    checkTextAlignedToBaselineGrid();

    // Text is not aligned inside the header.
    SwDocShell* pDocShell = getSwDocShell();
    CPPUNIT_ASSERT(pDocShell);

    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    SwRootFrame* pRoot = pWrtShell->GetLayout();
    CPPUNIT_ASSERT(pRoot);

    SwFrame* pPage = pRoot->GetLower();
    CPPUNIT_ASSERT(pPage);
    CPPUNIT_ASSERT(pPage->IsPageFrame());

    SwFrame* pHeader = pPage->GetLower();
    CPPUNIT_ASSERT(pHeader);
    CPPUNIT_ASSERT(pHeader->IsHeaderFrame());

    SwFrame* pHeaderText = pHeader->GetLower();
    CPPUNIT_ASSERT(pHeaderText);
    CPPUNIT_ASSERT(pHeaderText->IsTextFrame());
    checkTextAlignedToBaselineGrid(pHeaderText, false);

    // Text is not aligned inside the footer.
    CPPUNIT_ASSERT(pHeader->GetNext());
    SwFrame* pFooter = pHeader->GetNext()->GetNext();
    CPPUNIT_ASSERT(pFooter);
    CPPUNIT_ASSERT(pFooter->IsFooterFrame());

    SwFrame* pFooterText = pFooter->GetLower();
    CPPUNIT_ASSERT(pFooterText);
    CPPUNIT_ASSERT(pFooterText->IsTextFrame());
    checkTextAlignedToBaselineGrid(pFooterText, false);
}

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
