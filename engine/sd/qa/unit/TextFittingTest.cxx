/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sdmodeltestbase.hxx"

#include <vcl/scheduler.hxx>
#include <svx/svdview.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editobj.hxx>
#include <svx/compatflags.hxx>
#include <svx/sdtfsitm.hxx>
#include <svx/svddef.hxx>
#include <Outliner.hxx>
#include <DrawDocShell.hxx>
#include <drawdoc.hxx>
#include <unomodel.hxx>
#include <sdpage.hxx>
#include <ViewShell.hxx>

class TextFittingTest : public SdModelTestBase
{
public:
    TextFittingTest()
        : SdModelTestBase(u"/sd/qa/unit/data/"_ustr)
    {
    }
};

// Additionally visually check documents TextFittingComparisonWithMSO_*.pptx
// Those documents contain a bitmap image that includes rendering from MSO
// so we can visually check where our scaling implementation differs with the
// rendering in MSO.

CPPUNIT_TEST_FIXTURE(TextFittingTest, testTest)
{
    createSdImpressDoc("TextFitting.odp");

    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    SdPage* pPage = pViewShell->GetActualPage();
    auto pTextObject = DynCastSdrTextObj(pPage->GetObj(0));
    CPPUNIT_ASSERT(pTextObject);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, pTextObject->GetFontScale(), 1E-4);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, pTextObject->GetSpacingScale(), 1E-4);

    {
        OutlinerParaObject* pOutlinerParagraphObject = pTextObject->GetOutlinerParaObject();
        const EditTextObject& aEdit = pOutlinerParagraphObject->GetTextObject();
        CPPUNIT_ASSERT_EQUAL(u"D1"_ustr, aEdit.GetText(0));
        CPPUNIT_ASSERT_EQUAL(u"D2"_ustr, aEdit.GetText(1));
        CPPUNIT_ASSERT_EQUAL(u"D3"_ustr, aEdit.GetText(2));
    }

    sd::ViewShell* pViewShell1 = pXImpressDocument->GetDocShell()->GetViewShell();
    SdrView* pView1 = pViewShell1->GetView();
    Scheduler::ProcessEventsToIdle();
    pView1->SdrBeginTextEdit(pTextObject);
    CPPUNIT_ASSERT_EQUAL(true, pView1->IsTextEdit());

    auto* pOLV = pView1->GetTextEditOutlinerView();
    CPPUNIT_ASSERT(pOLV);
    auto& rEditView = pOLV->GetEditView();
    auto& rEditEngine = rEditView.getEditEngine();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), rEditEngine.GetParagraphCount());

    // Add paragraph 4
    rEditView.SetSelection(ESelection(3, 0));
    rEditView.InsertText(u"\nD4"_ustr);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), rEditEngine.GetParagraphCount());

    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.85, rEditEngine.getScalingParameters().fFontY, 1E-4);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.9, rEditEngine.getScalingParameters().fSpacingY, 1E-4);

    // Add paragraph 5
    rEditView.SetSelection(ESelection(4, 0));
    rEditView.InsertText(u"\nD5"_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), rEditEngine.GetParagraphCount());

    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.7, rEditEngine.getScalingParameters().fFontY, 1E-4);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.8, rEditEngine.getScalingParameters().fSpacingY, 1E-4);

    // Add paragraph 6
    rEditView.SetSelection(ESelection(5, 0));
    rEditView.InsertText(u"\nD6"_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), rEditEngine.GetParagraphCount());

    // Delete paragraph 6
    rEditView.SetSelection(ESelection(4, EE_TEXTPOS_MAX, 5, EE_TEXTPOS_MAX));
    rEditView.DeleteSelected();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), rEditEngine.GetParagraphCount());

    // Delete paragraph 5
    rEditView.SetSelection(ESelection(3, EE_TEXTPOS_MAX, 4, EE_TEXTPOS_MAX));
    rEditView.DeleteSelected();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), rEditEngine.GetParagraphCount());

    // Delete paragraph 4
    rEditView.SetSelection(ESelection(2, EE_TEXTPOS_MAX, 3, EE_TEXTPOS_MAX));
    rEditView.DeleteSelected();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), rEditEngine.GetParagraphCount());

    // not ideal - scaling should be 100%, but close enough
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, rEditEngine.getScalingParameters().fFontY, 1E-4);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, rEditEngine.getScalingParameters().fSpacingY, 1E-4);

    // are we still in text edit mode?
    CPPUNIT_ASSERT_EQUAL(true, pView1->IsTextEdit());
    pView1->SdrEndTextEdit();
    CPPUNIT_ASSERT_EQUAL(false, pView1->IsTextEdit());

    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, pTextObject->GetFontScale(), 1E-4);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, pTextObject->GetSpacingScale(), 1E-4);
}

CPPUNIT_TEST_FIXTURE(TextFittingTest, testTestBulletDifferenceViewAndEdit)
{
    createSdImpressDoc("TextFittingBulletEditVsView.odp");

    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    SdPage* pPage = pViewShell->GetActualPage();
    auto pTextObject = DynCastSdrTextObj(pPage->GetObj(0));
    CPPUNIT_ASSERT(pTextObject);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.475, pTextObject->GetFontScale(), 1E-4);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.8, pTextObject->GetSpacingScale(), 1E-4);

    // Enter edit mode
    SdrView* pView = pViewShell->GetView();
    Scheduler::ProcessEventsToIdle();
    pView->SdrBeginTextEdit(pTextObject);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(true, pView->IsTextEdit());

    auto* pOLV = pView->GetTextEditOutlinerView();
    CPPUNIT_ASSERT(pOLV);
    auto& rEditView = pOLV->GetEditView();
    auto& rEditEngine = rEditView.getEditEngine();

    // Verify that font and spacing scaling match between view and edit mode
    CPPUNIT_ASSERT_DOUBLES_EQUAL(pTextObject->GetFontScale(),
                                 rEditEngine.getScalingParameters().fFontY, 1E-4);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(pTextObject->GetSpacingScale(),
                                 rEditEngine.getScalingParameters().fSpacingY, 1E-4);

    Outliner* pEditOutliner = pView->GetTextEditOutliner();
    CPPUNIT_ASSERT(pEditOutliner);

    // Get bullet sizes at current scaling
    std::vector<Size> aBulletSizesBefore;
    for (sal_Int32 nPara = 0; nPara < pEditOutliner->GetParagraphCount(); ++nPara)
    {
        EBulletInfo aBulletInfo = pEditOutliner->GetBulletInfo(nPara);
        aBulletSizesBefore.push_back(aBulletInfo.aBounds.GetSize());
    }

    // Add a paragraph to trigger a scaling change
    rEditView.SetSelection(ESelection(rEditEngine.GetParagraphCount() - 1, EE_TEXTPOS_MAX));
    rEditView.InsertText(u"\nNew"_ustr);
    Scheduler::ProcessEventsToIdle();

    // Delete the paragraph to restore original scaling
    sal_Int32 nLastPara = rEditEngine.GetParagraphCount() - 1;
    rEditView.SetSelection(ESelection(nLastPara - 1, EE_TEXTPOS_MAX, nLastPara, EE_TEXTPOS_MAX));
    rEditView.DeleteSelected();
    Scheduler::ProcessEventsToIdle();

    // Scaling should be back to original
    CPPUNIT_ASSERT_DOUBLES_EQUAL(pTextObject->GetFontScale(),
                                 rEditEngine.getScalingParameters().fFontY, 1E-4);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(pTextObject->GetSpacingScale(),
                                 rEditEngine.getScalingParameters().fSpacingY, 1E-4);

    // Get bullet sizes after scaling was changed and restored
    std::vector<Size> aBulletSizesAfter;
    for (sal_Int32 nPara = 0; nPara < pEditOutliner->GetParagraphCount(); ++nPara)
    {
        EBulletInfo aBulletInfo = pEditOutliner->GetBulletInfo(nPara);
        aBulletSizesAfter.push_back(aBulletInfo.aBounds.GetSize());
    }

    CPPUNIT_ASSERT_EQUAL(aBulletSizesBefore.size(), aBulletSizesAfter.size());

    // Bullet sizes should match after scaling round-trip
    for (size_t i = 0; i < aBulletSizesBefore.size(); ++i)
    {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Paragraph " + std::to_string(i)
                                         + " bullet width changed after scaling round-trip",
                                     aBulletSizesBefore[i].Width(), aBulletSizesAfter[i].Width());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Paragraph " + std::to_string(i)
                                         + " bullet height changed after scaling round-trip",
                                     aBulletSizesBefore[i].Height(), aBulletSizesAfter[i].Height());
    }
    pView->SdrEndTextEdit();
    CPPUNIT_ASSERT_EQUAL(false, pView->IsTextEdit());
}

CPPUNIT_TEST_FIXTURE(TextFittingTest, testNewDocumentHasNoLegacyTextFitting)
{
    // A new document fits text the new way, and still does once saved in our own format and
    // loaded again.
    createSdImpressDoc();
    auto pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    CPPUNIT_ASSERT(
        !pXImpressDocument->GetDoc()->GetCompatibilityFlag(SdrCompatibilityFlag::TextFittingLegacy));

    saveAndReload(TestFilter::ODP);
    pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    CPPUNIT_ASSERT(
        !pXImpressDocument->GetDoc()->GetCompatibilityFlag(SdrCompatibilityFlag::TextFittingLegacy));
}

CPPUNIT_TEST_FIXTURE(TextFittingTest, testOwnFormatKeepsLegacyTextFitting)
{
    // A document in our own format saved before the flag existed keeps the older fitting.
    createSdImpressDoc("TextFittingBulletEditVsView.odp");
    auto pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    CPPUNIT_ASSERT(
        pXImpressDocument->GetDoc()->GetCompatibilityFlag(SdrCompatibilityFlag::TextFittingLegacy));
}

CPPUNIT_TEST_FIXTURE(TextFittingTest, testTitleKeepsLineSpacing)
{
    createSdImpressDoc("TextFittingTitle.fodp");
    auto pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    pXImpressDocument->GetDoc()->SetCompatibilityFlag(SdrCompatibilityFlag::TextFittingLegacy,
                                                      false);
    SdPage* pPage = pXImpressDocument->GetDocShell()->GetViewShell()->GetActualPage();

    // Both shapes hold the same text in the same space, so both have to be made smaller.
    auto pTitle = DynCastSdrTextObj(pPage->GetObj(0));
    CPPUNIT_ASSERT(pTitle);
    auto pOutline = DynCastSdrTextObj(pPage->GetObj(1));
    CPPUNIT_ASSERT(pOutline);

    // The title is fitted by making the font smaller and nothing else.
    CPPUNIT_ASSERT_LESS(1.0, pTitle->GetFontScale());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, pTitle->GetSpacingScale(), 1E-4);

    // Any other shape has its line spacing taken down as well.
    CPPUNIT_ASSERT_LESS(1.0, pOutline->GetFontScale());
    CPPUNIT_ASSERT_LESS(1.0, pOutline->GetSpacingScale());
}

CPPUNIT_TEST_FIXTURE(TextFittingTest, testLegacyFittingReducesTitleLineSpacing)
{
    // With the legacy fitting a title is fitted by taking its line spacing down as well as its
    // font size.
    createSdImpressDoc("TextFittingTitle.fodp");
    auto pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    pXImpressDocument->GetDoc()->SetCompatibilityFlag(SdrCompatibilityFlag::TextFittingLegacy,
                                                      true);
    SdPage* pPage = pXImpressDocument->GetDocShell()->GetViewShell()->GetActualPage();
    auto pTitle = DynCastSdrTextObj(pPage->GetObj(0));
    CPPUNIT_ASSERT(pTitle);

    CPPUNIT_ASSERT_LESS(1.0, pTitle->GetFontScale());
    CPPUNIT_ASSERT_LESS(1.0, pTitle->GetSpacingScale());
}

CPPUNIT_TEST_FIXTURE(TextFittingTest, testTitleFitsLikeReference)
{
    // Titles holding the same text in boxes from just large enough down to less than half the
    // height it needs. The reference program takes each overflowing title to 90 percent, leaves
    // its line spacing alone, and goes no further even where the text still overflows.
    createSdImpressDoc("pptx/TextFittingTitleLikeReference.pptx");
    auto pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();

    const sal_uInt16 nPages = pDoc->GetSdPageCount(PageKind::Standard);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(17), nPages);
    for (sal_uInt16 i = 0; i < nPages; ++i)
    {
        auto pTitle = DynCastSdrTextObj(pDoc->GetSdPage(i, PageKind::Standard)->GetObj(0));
        CPPUNIT_ASSERT(pTitle);
        CPPUNIT_ASSERT_EQUAL(SdrObjKind::TitleText, pTitle->GetTextKind());
        OString sSlide = "slide " + OString::number(i + 1);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(sSlide.getStr(), i == 0 ? 1.0 : 0.9,
                                             pTitle->GetFontScale(), 1E-4);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(sSlide.getStr(), 1.0, pTitle->GetSpacingScale(),
                                             1E-4);
    }
}

namespace
{
// Fits two equal lines, the second with 2cm of space above it, into a box too small for them.
// Returns the space above the second line after fitting, in 1/100 mm, and the line spacing scale.
std::pair<tools::Long, double> fitParagraphSpacing(SdXImpressDocument* pXImpressDocument,
                                                   bool bLegacyFitting)
{
    pXImpressDocument->GetDoc()->SetCompatibilityFlag(SdrCompatibilityFlag::TextFittingLegacy,
                                                      bLegacyFitting);
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    auto pTextObject = DynCastSdrTextObj(pViewShell->GetActualPage()->GetObj(0));
    CPPUNIT_ASSERT(pTextObject);

    SdrView* pView = pViewShell->GetView();
    Scheduler::ProcessEventsToIdle();
    pView->SdrBeginTextEdit(pTextObject);
    CPPUNIT_ASSERT(pView->IsTextEdit());
    EditEngine& rEditEngine = pView->GetTextEditOutlinerView()->GetEditView().getEditEngine();
    const double fSpacingY = rEditEngine.getScalingParameters().fSpacingY;
    const tools::Long nSpace = tools::Long(rEditEngine.GetTextHeight(1))
                               - tools::Long(rEditEngine.GetTextHeight(0));
    pView->SdrEndTextEdit();
    return { nSpace, fSpacingY };
}
}

CPPUNIT_TEST_FIXTURE(TextFittingTest, testParagraphSpacingKeepsSize)
{
    // Fitting the text leaves the space above a paragraph at the size it was given.
    createSdImpressDoc("TextFittingParagraphSpacing.fodp");
    auto pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);

    auto [nSpace, fSpacingY] = fitParagraphSpacing(pXImpressDocument, false);
    CPPUNIT_ASSERT_LESS(1.0, fSpacingY);
    CPPUNIT_ASSERT_EQUAL(tools::Long(2000), nSpace);
}

CPPUNIT_TEST_FIXTURE(TextFittingTest, testLegacyFittingReducesParagraphSpacing)
{
    // With the legacy fitting the space above a paragraph is reduced along with the line spacing.
    createSdImpressDoc("TextFittingParagraphSpacing.fodp");
    auto pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);

    auto [nSpace, fSpacingY] = fitParagraphSpacing(pXImpressDocument, true);
    CPPUNIT_ASSERT_LESS(1.0, fSpacingY);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2000.0 * fSpacingY, double(nSpace), 2.0);
}

namespace
{
struct FittedText
{
    tools::Long nHeight; // 1/100 mm
    double fFontScale;
};

// Fits four lines of exact 1.5cm line spacing into a box too small for them and returns the
// height of the fitted text and the font scale it took.
FittedText fitExactLineSpacing(SdXImpressDocument* pXImpressDocument, bool bLegacyFitting)
{
    pXImpressDocument->GetDoc()->SetCompatibilityFlag(SdrCompatibilityFlag::TextFittingLegacy,
                                                      bLegacyFitting);
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    auto pTextObject = DynCastSdrTextObj(pViewShell->GetActualPage()->GetObj(0));
    CPPUNIT_ASSERT(pTextObject);

    SdrView* pView = pViewShell->GetView();
    Scheduler::ProcessEventsToIdle();
    pView->SdrBeginTextEdit(pTextObject);
    CPPUNIT_ASSERT(pView->IsTextEdit());
    EditEngine& rEditEngine = pView->GetTextEditOutlinerView()->GetEditView().getEditEngine();

    // The text does not fit, so the line spacing scale has been taken down.
    CPPUNIT_ASSERT_LESS(1.0, rEditEngine.getScalingParameters().fSpacingY);
    FittedText aFitted{ rEditEngine.GetTextHeight(), rEditEngine.getScalingParameters().fFontY };
    pView->SdrEndTextEdit();
    return aFitted;
}
}

CPPUNIT_TEST_FIXTURE(TextFittingTest, testExactLineSpacingKeepsHeight)
{
    // Fitting the text leaves an exact line spacing at the height it was given.
    createSdImpressDoc("TextFittingExactLineSpacing.fodp");
    auto pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);

    FittedText aFitted = fitExactLineSpacing(pXImpressDocument, false);
    CPPUNIT_ASSERT_EQUAL(tools::Long(6000), aFitted.nHeight);
    // No font scale makes the lines shorter, so the font ends at the smallest scale.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.25, aFitted.fFontScale, 1E-4);
}

CPPUNIT_TEST_FIXTURE(TextFittingTest, testLegacyFittingReducesExactLineSpacing)
{
    // With the legacy fitting an exact line spacing is reduced along with a proportional one.
    createSdImpressDoc("TextFittingExactLineSpacing.fodp");
    auto pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);

    FittedText aFitted = fitExactLineSpacing(pXImpressDocument, true);
    CPPUNIT_ASSERT_EQUAL(tools::Long(4800), aFitted.nHeight);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.85, aFitted.fFontScale, 1E-4);
}

namespace
{
// Shrinks the box around four lines of 150 percent line spacing to 95 percent of their height,
// fits the text, and returns the fitted height as a fraction of the height before fitting.
double fitProportionalLineSpacing(SdXImpressDocument* pXImpressDocument, bool bLegacyFitting)
{
    pXImpressDocument->GetDoc()->SetCompatibilityFlag(SdrCompatibilityFlag::TextFittingLegacy,
                                                      bLegacyFitting);
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    auto pTextObject = DynCastSdrTextObj(pViewShell->GetActualPage()->GetObj(0));
    CPPUNIT_ASSERT(pTextObject);
    SdrView* pView = pViewShell->GetView();
    Scheduler::ProcessEventsToIdle();

    pView->SdrBeginTextEdit(pTextObject);
    CPPUNIT_ASSERT(pView->IsTextEdit());
    tools::Long nNaturalHeight
        = pView->GetTextEditOutlinerView()->GetEditView().getEditEngine().GetTextHeight();
    pView->SdrEndTextEdit();

    tools::Rectangle aRect = pTextObject->GetLogicRect();
    aRect.SetSize(Size(aRect.GetWidth(), nNaturalHeight * 95 / 100
                                             + pTextObject->GetTextUpperDistance()
                                             + pTextObject->GetTextLowerDistance()));
    pTextObject->SetLogicRect(aRect);

    pView->SdrBeginTextEdit(pTextObject);
    CPPUNIT_ASSERT(pView->IsTextEdit());
    EditEngine& rEditEngine = pView->GetTextEditOutlinerView()->GetEditView().getEditEngine();
    // The first reduction of the line spacing is enough, with the font left alone.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, rEditEngine.getScalingParameters().fFontY, 1E-4);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.9, rEditEngine.getScalingParameters().fSpacingY, 1E-4);
    double fRatio = double(rEditEngine.GetTextHeight()) / nNaturalHeight;
    pView->SdrEndTextEdit();
    return fRatio;
}
}

CPPUNIT_TEST_FIXTURE(TextFittingTest, testLineSpacingReductionIsSubtracted)
{
    // A 10 percent reduction takes 150 percent line spacing to 140 percent.
    createSdImpressDoc("pptx/TextFittingProportionalLineSpacing.pptx");
    auto pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(140.0 / 150.0, fitProportionalLineSpacing(pXImpressDocument, false),
                                 0.005);
}

CPPUNIT_TEST_FIXTURE(TextFittingTest, testLegacyFittingMultipliesLineSpacingReduction)
{
    // With the legacy fitting a 10 percent reduction takes 150 percent line spacing to 135.
    createSdImpressDoc("pptx/TextFittingProportionalLineSpacing.pptx");
    auto pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.9, fitProportionalLineSpacing(pXImpressDocument, true), 0.005);
}

CPPUNIT_TEST_FIXTURE(TextFittingTest, testSpaceBelowLastLineLeftOutOfFit)
{
    // Six lines of 150 percent line spacing in boxes of 0.928 and 0.860 of their height. The
    // reference program fits them at 100 percent font with 10 percent less spacing, and 92.5
    // percent with 10 percent less, which fit only when the part of the last line's extra spacing
    // below its baseline is left out of the height.
    createSdImpressDoc("pptx/TextFittingLikeReference.pptx");
    auto pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();

    auto pBody = DynCastSdrTextObj(pDoc->GetSdPage(4, PageKind::Standard)->GetObj(1));
    CPPUNIT_ASSERT(pBody);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, pBody->GetFontScale(), 1E-4);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.9, pBody->GetSpacingScale(), 1E-4);

    pBody = DynCastSdrTextObj(pDoc->GetSdPage(8, PageKind::Standard)->GetObj(1));
    CPPUNIT_ASSERT(pBody);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.925, pBody->GetFontScale(), 1E-4);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.9, pBody->GetSpacingScale(), 1E-4);
}

CPPUNIT_TEST_FIXTURE(TextFittingTest, testFitsLikeReference)
{
    // Each slide holds the same text in a box of a different height, and the reference program
    // chose these font and line spacing scales for them. Six short lines come first, then three
    // paragraphs that each wrap to two lines at full size and fit on one line at 92.5 percent.
    createSdImpressDoc("pptx/TextFittingLikeReference.pptx");
    auto pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();

    static constexpr std::pair<double, double> aExpected[] = {
        { 1.0, 1.0 },   { 1.0, 0.9 },   { 1.0, 0.9 },   { 1.0, 0.9 },   { 1.0, 0.9 },
        { 0.925, 1.0 }, { 0.925, 0.9 }, { 0.925, 0.9 }, { 0.925, 0.9 }, { 0.925, 0.8 },
        { 0.85, 0.9 },  { 0.85, 0.8 },  { 0.85, 0.8 },  { 1.0, 0.9 },   { 0.925, 1.0 },
        { 0.925, 1.0 }, { 0.925, 0.9 },
    };
    CPPUNIT_ASSERT_EQUAL(std::size(aExpected), size_t(pDoc->GetSdPageCount(PageKind::Standard)));
    for (size_t i = 0; i < std::size(aExpected); ++i)
    {
        auto pBody = DynCastSdrTextObj(pDoc->GetSdPage(i, PageKind::Standard)->GetObj(1));
        CPPUNIT_ASSERT(pBody);
        OString sSlide = "slide " + OString::number(i + 1);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(sSlide.getStr(), aExpected[i].first,
                                             pBody->GetFontScale(), 1E-4);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(sSlide.getStr(), aExpected[i].second,
                                             pBody->GetSpacingScale(), 1E-4);
    }
}

CPPUNIT_TEST_FIXTURE(TextFittingTest, testStoredFitKeptUntilEdited)
{
    // The first slide stores a fit of 55 percent font with 20 percent less line spacing, though
    // its text fits at full size. The stored fit is shown until the text is edited, and then the
    // text is fitted afresh.
    createSdImpressDoc("pptx/TextFittingStored.pptx");
    auto pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    auto pBody = DynCastSdrTextObj(pViewShell->GetActualPage()->GetObj(1));
    CPPUNIT_ASSERT(pBody);

    const SdrTextFitToSizeTypeItem& rStored = pBody->GetMergedItem(SDRATTR_TEXT_FITTOSIZE);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.55, rStored.getFontScale(), 1E-4);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.8, rStored.getSpacingScale(), 1E-4);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.55, pBody->GetFontScale(), 1E-4);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.8, pBody->GetSpacingScale(), 1E-4);

    SdrView* pView = pViewShell->GetView();
    Scheduler::ProcessEventsToIdle();
    pView->SdrBeginTextEdit(pBody);
    CPPUNIT_ASSERT(pView->IsTextEdit());
    EditView& rEditView = pView->GetTextEditOutlinerView()->GetEditView();
    rEditView.SetSelection(ESelection(0, 0));
    rEditView.InsertText(u"x"_ustr);
    pView->SdrEndTextEdit();

    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, pBody->GetFontScale(), 1E-4);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, pBody->GetSpacingScale(), 1E-4);
}

CPPUNIT_TEST_FIXTURE(TextFittingTest, testStoredFitBackAfterUndo)
{
    // Undoing an edit brings the text back with the fit stored in the file, and redoing it brings
    // back the fit made afresh after the edit.
    createSdImpressDoc("pptx/TextFittingStored.pptx");
    auto pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    auto pBody = DynCastSdrTextObj(pViewShell->GetActualPage()->GetObj(1));
    CPPUNIT_ASSERT(pBody);

    SdrView* pView = pViewShell->GetView();
    Scheduler::ProcessEventsToIdle();
    pView->SdrBeginTextEdit(pBody);
    CPPUNIT_ASSERT(pView->IsTextEdit());
    EditView& rEditView = pView->GetTextEditOutlinerView()->GetEditView();
    rEditView.SetSelection(ESelection(0, 0));
    rEditView.InsertText(u"x"_ustr);
    pView->SdrEndTextEdit();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, pBody->GetFontScale(), 1E-4);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, pBody->GetSpacingScale(), 1E-4);

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.55, pBody->GetFontScale(), 1E-4);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.8, pBody->GetSpacingScale(), 1E-4);

    dispatchCommand(mxComponent, u".uno:Redo"_ustr, {});
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, pBody->GetFontScale(), 1E-4);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, pBody->GetSpacingScale(), 1E-4);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
