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
#include <flyfrm.hxx>
#include <frame.hxx>
#include <pagefrm.hxx>
#include <porlay.hxx>
#include <rootfrm.hxx>
#include <sortedobjs.hxx>
#include <swmodeltestbase.hxx>
#include <txtfrm.hxx>
#include <wrtsh.hxx>

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

    void applyPageLineSpacing(const uint16_t nPage, const bool bEnable,
                              const OUString& rReferenceStyle);

    Size getTextFrameSize(const uint16_t nTextFrame, const OUString& rTextContent);

private:
    void checkTextAlignedToBaselineGrid(SwTextFrame* pTextFrame, const bool bAligned);
};

void SwPageLineSpacingTest::checkTextAlignedToBaselineGrid(const bool bAligned)
{
    SwDocShell* pDocShell = getSwDocShell();
    CPPUNIT_ASSERT(pDocShell);

    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    SwRootFrame* pRoot = pWrtShell->GetLayout();
    CPPUNIT_ASSERT(pRoot);

    SwFrame* pNextFrame = pRoot;
    bool bLowerFinished = false;
    while (pNextFrame)
    {
        // Check the text alignment within text frames.
        if (pNextFrame->IsTextFrame())
        {
            auto pTextFrame = dynamic_cast<SwTextFrame*>(pNextFrame);
            CPPUNIT_ASSERT(pTextFrame);
            checkTextAlignedToBaselineGrid(pTextFrame, bAligned);
        }

        // Also check the content of text frames.
        if (pNextFrame->IsPageFrame() && !bLowerFinished)
        {
            auto pPageFrame = dynamic_cast<SwPageFrame*>(pNextFrame);
            CPPUNIT_ASSERT(pPageFrame);
            const SwSortedObjs* pObjects = pPageFrame->GetSortedObjs();
            if (pObjects)
            {
                for (SwAnchoredObject* pObject : *pObjects)
                {
                    SwFlyFrame* pFlyFrame = pObject->DynCastFlyFrame();
                    if (pFlyFrame)
                    {
                        auto pTextFrame = dynamic_cast<SwTextFrame*>(pFlyFrame->GetLower());
                        CPPUNIT_ASSERT(pTextFrame);
                        checkTextAlignedToBaselineGrid(pTextFrame, bAligned);
                    }
                }
            }
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

CPPUNIT_TEST_FIXTURE(SwPageLineSpacingTest, testPageLineSpacingDisabledParagraph)
{
    createSwDoc("pageLineSpacingDisabledParagraph.fodt");

    checkTextAlignedToBaselineGrid(false);
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

/* This use case does not work properly (tdf#170000)
CPPUNIT_TEST_FIXTURE(SwPageLineSpacingTest, testColumnSection)
{
    createSwDoc("columnSection.fodt");

    checkTextAlignedToBaselineGrid();
}
*/

CPPUNIT_TEST_FIXTURE(SwPageLineSpacingTest, testPageColumns)
{
    createSwDoc("pageColumns.fodt");

    checkTextAlignedToBaselineGrid();
}

CPPUNIT_TEST_FIXTURE(SwPageLineSpacingTest, testTableWithTopAlignment)
{
    createSwDoc("tableWithTopAlignment.fodt");

    checkTextAlignedToBaselineGrid();
}

/* This use case does not work properly (tdf#93785)
CPPUNIT_TEST_FIXTURE(SwPageLineSpacingTest, testTextFrame)
{
    createSwDoc("textFrame.fodt");

    checkTextAlignedToBaselineGrid();

}
*/

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

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
