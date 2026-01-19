/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
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
#include <Outliner.hxx>
#include <DrawDocShell.hxx>
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

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
