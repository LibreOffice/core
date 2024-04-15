/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sdmodeltestbase.hxx"

#include <com/sun/star/frame/Desktop.hpp>

#include <vcl/scheduler.hxx>
#include <svx/svdview.hxx>
#include <svx/svdpagv.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editobj.hxx>

#include <DrawDocShell.hxx>
#include <unomodel.hxx>
#include <sdpage.hxx>
#include <ViewShell.hxx>

using namespace css;

class TextFittingTest : public SdModelTestBase
{
public:
    TextFittingTest()
        : SdModelTestBase("/sd/qa/unit/data/")
    {
    }
};

CPPUNIT_TEST_FIXTURE(TextFittingTest, testTest)
{
    createSdImpressDoc("TextFitting.odp");

    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    SdPage* pPage = pViewShell->GetActualPage();
    auto pTextObject = DynCastSdrTextObj(pPage->GetObj(0));
    CPPUNIT_ASSERT(pTextObject);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(100.0, pTextObject->GetFontScale(), 1E-2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(100.0, pTextObject->GetSpacingScale(), 1E-2);

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
    auto* pEditEngine = rEditView.GetEditEngine();
    CPPUNIT_ASSERT(pEditEngine);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), pEditEngine->GetParagraphCount());

    // Add paragraph 4
    rEditView.SetSelection(ESelection(3, 0, 3, 0));
    rEditView.InsertText(u"\nD4"_ustr);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), pEditEngine->GetParagraphCount());

    CPPUNIT_ASSERT_DOUBLES_EQUAL(92.5, pEditEngine->getScalingParameters().fFontY, 1E-2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(80.0, pEditEngine->getScalingParameters().fSpacingY, 1E-2);

    // Add paragraph 5
    rEditView.SetSelection(ESelection(4, 0, 4, 0));
    rEditView.InsertText(u"\nD5"_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), pEditEngine->GetParagraphCount());

    CPPUNIT_ASSERT_DOUBLES_EQUAL(70.0, pEditEngine->getScalingParameters().fFontY, 1E-2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(80.0, pEditEngine->getScalingParameters().fSpacingY, 1E-2);

    // Add paragraph 6
    rEditView.SetSelection(ESelection(5, 0, 5, 0));
    rEditView.InsertText(u"\nD6"_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), pEditEngine->GetParagraphCount());

    // Delete paragraph 6
    rEditView.SetSelection(ESelection(4, EE_TEXTPOS_MAX_COUNT, 5, EE_TEXTPOS_MAX_COUNT));
    rEditView.DeleteSelected();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), pEditEngine->GetParagraphCount());

    // Delete paragraph 5
    rEditView.SetSelection(ESelection(3, EE_TEXTPOS_MAX_COUNT, 4, EE_TEXTPOS_MAX_COUNT));
    rEditView.DeleteSelected();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), pEditEngine->GetParagraphCount());

    // Delete paragraph 4
    rEditView.SetSelection(ESelection(2, EE_TEXTPOS_MAX_COUNT, 3, EE_TEXTPOS_MAX_COUNT));
    rEditView.DeleteSelected();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), pEditEngine->GetParagraphCount());

    // We are back to 100%
    CPPUNIT_ASSERT_DOUBLES_EQUAL(100.0, pEditEngine->getScalingParameters().fFontY, 1E-2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(100.0, pEditEngine->getScalingParameters().fSpacingY, 1E-2);

    // are we still in text edit mode?
    CPPUNIT_ASSERT_EQUAL(true, pView1->IsTextEdit());
    pView1->SdrEndTextEdit();
    CPPUNIT_ASSERT_EQUAL(false, pView1->IsTextEdit());

    CPPUNIT_ASSERT_DOUBLES_EQUAL(100.0, pTextObject->GetFontScale(), 1E-2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(100.0, pTextObject->GetSpacingScale(), 1E-2);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
