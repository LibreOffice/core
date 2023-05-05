/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <officecfg/Office/Common.hxx>

#include "sdmodeltestbase.hxx"

#include <com/sun/star/uno/Reference.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>

#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPages.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/table/XTable.hpp>
#include <com/sun/star/table/XMergeableCellRange.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>

#include <DrawDocShell.hxx>
#include <editeng/editeng.hxx>
#include <drawdoc.hxx>
#include <vcl/scheduler.hxx>
#include <svx/sdr/table/tablecontroller.hxx>
#include <sfx2/request.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svxids.hrc>
#include <editeng/eeitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/editobj.hxx>
#include <undo/undomanager.hxx>
#include <GraphicViewShell.hxx>
#include <sdpage.hxx>
#include <comphelper/base64.hxx>
#include <LayerTabBar.hxx>
#include <vcl/event.hxx>
#include <vcl/keycodes.hxx>
#include <svx/svdoashp.hxx>
#include <tools/gen.hxx>
#include <svx/view3d.hxx>
#include <svx/scene3d.hxx>
#include <svx/sdmetitm.hxx>
#include <unomodel.hxx>

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
        CPPUNIT_ASSERT_EQUAL(OUString(u"D1"), aEdit.GetText(0));
        CPPUNIT_ASSERT_EQUAL(OUString(u"D2"), aEdit.GetText(1));
        CPPUNIT_ASSERT_EQUAL(OUString(u"D3"), aEdit.GetText(2));
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
    rEditView.InsertText(u"\nD4");
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), pEditEngine->GetParagraphCount());

    CPPUNIT_ASSERT_DOUBLES_EQUAL(87.49, pEditEngine->getGlobalFontScale().getY(), 1E-2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(90.0, pEditEngine->getGlobalSpacingScale().getY(), 1E-2);

    // Add paragraph 5
    rEditView.SetSelection(ESelection(4, 0, 4, 0));
    rEditView.InsertText(u"\nD5");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), pEditEngine->GetParagraphCount());

    CPPUNIT_ASSERT_DOUBLES_EQUAL(54.68, pEditEngine->getGlobalFontScale().getY(), 1E-2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(100.0, pEditEngine->getGlobalSpacingScale().getY(), 1E-2);

    // Add paragraph 6
    rEditView.SetSelection(ESelection(5, 0, 5, 0));
    rEditView.InsertText(u"\nD6");
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

    // not ideal - scaling should be 100%, but close enough
    CPPUNIT_ASSERT_DOUBLES_EQUAL(99.05, pEditEngine->getGlobalFontScale().getY(), 1E-2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(100.0, pEditEngine->getGlobalSpacingScale().getY(), 1E-2);

    // are we still in text edit mode?
    CPPUNIT_ASSERT_EQUAL(true, pView1->IsTextEdit());
    pView1->SdrEndTextEdit();
    CPPUNIT_ASSERT_EQUAL(false, pView1->IsTextEdit());

    CPPUNIT_ASSERT_DOUBLES_EQUAL(100.0, pTextObject->GetFontScale(), 1E-2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(100.0, pTextObject->GetSpacingScale(), 1E-2);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
