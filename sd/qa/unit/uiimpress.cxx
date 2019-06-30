/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>
#include <unotest/macros_test.hxx>

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/uno/Reference.hxx>

#include <comphelper/processfactory.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/intitem.hxx>
#include <svx/svxids.hrc>
#include <svl/stritem.hxx>

#include <DrawDocShell.hxx>
#include <ViewShell.hxx>
#include <app.hrc>
#include <sdpage.hxx>
#include <unomodel.hxx>

using namespace ::com::sun::star;

/// Impress UI tests.
class SdUiImpressTest : public test::BootstrapFixture, public unotest::MacrosTest
{
protected:
    uno::Reference<uno::XComponentContext> mxComponentContext;
    uno::Reference<lang::XComponent> mxComponent;

public:
    virtual void setUp() override;
    virtual void tearDown() override;
};

void SdUiImpressTest::setUp()
{
    test::BootstrapFixture::setUp();

    mxComponentContext.set(comphelper::getComponentContext(getMultiServiceFactory()));
    mxDesktop.set(frame::Desktop::create(mxComponentContext));
}

void SdUiImpressTest::tearDown()
{
    if (mxComponent.is())
        mxComponent->dispose();

    test::BootstrapFixture::tearDown();
}

CPPUNIT_TEST_FIXTURE(SdUiImpressTest, testTdf111522)
{
    // Load the document and create two new windows.
    mxComponent = loadFromDesktop(m_directories.getURLFromSrc("sd/qa/unit/data/tdf111522.odp"));
    auto pImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    sd::ViewShell* pViewShell = pImpressDocument->GetDocShell()->GetViewShell();
    pViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_NEWWINDOW, SfxCallMode::SYNCHRON);
    sd::ViewShell* pViewShell1 = pImpressDocument->GetDocShell()->GetViewShell();
    pViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_NEWWINDOW, SfxCallMode::SYNCHRON);
    sd::ViewShell* pViewShell2 = pImpressDocument->GetDocShell()->GetViewShell();
    CPPUNIT_ASSERT(pViewShell1 != pViewShell2);

    // Have slide 1 in window 1, slide 2 in window 2.
    SfxRequest aRequest(pViewShell2->GetViewFrame(), SID_SWITCHPAGE);
    aRequest.AppendItem(SfxUInt32Item(ID_VAL_WHATPAGE, 1));
    aRequest.AppendItem(
        SfxUInt32Item(ID_VAL_WHATKIND, static_cast<sal_uInt32>(PageKind::Standard)));
    pViewShell2->ExecuteSlot(aRequest, /*bAsync=*/false);

    // Start text edit in window 1.
    SdPage* pPage1 = pViewShell1->GetActualPage();
    SdrObject* pShape1 = pPage1->GetObj(0);
    SdrView* pView1 = pViewShell1->GetView();
    pView1->MarkObj(pShape1, pView1->GetSdrPageView());
    pView1->SdrBeginTextEdit(pShape1);
    CPPUNIT_ASSERT(pView1->IsTextEdit());

    // Move the shape in window 2 & undo.
    SdPage* pPage2 = pViewShell2->GetActualPage();
    CPPUNIT_ASSERT(pPage1 != pPage2);
    SdrObject* pShape2 = pPage2->GetObj(0);
    SdrView* pView2 = pViewShell2->GetView();
    pView2->MarkObj(pShape2, pView2->GetSdrPageView());
    pView2->MoveMarkedObj(Size(1, 1), /*bCopy=*/false);
    // Without the accompanying fix in place, this test would have failed with an assertion failure
    // in SdrObjEditView::SdrEndTextEdit() as mpOldTextEditUndoManager was not nullptr.
    pViewShell2->GetViewFrame()->GetDispatcher()->Execute(SID_UNDO, SfxCallMode::SYNCHRON);

    // Rotate the shape in window 2 & undo.
    pView2->MarkObj(pShape2, pView2->GetSdrPageView());
    pView2->RotateMarkedObj(pShape2->GetLastBoundRect().Center(), /*nAngle=*/45);
    // Without the accompanying fix in place, this test would have failed with an assertion failure
    // in SdrObjEditView::SdrEndTextEdit() as mpOldTextEditUndoManager was not nullptr.
    pViewShell2->GetViewFrame()->GetDispatcher()->Execute(SID_UNDO, SfxCallMode::SYNCHRON);

    // Start text edit in window 2.
    // tdf#125824
    pView2->MarkObj(pShape2, pView2->GetSdrPageView());
    pView2->SdrBeginTextEdit(pShape2);
    CPPUNIT_ASSERT(pView2->IsTextEdit());
    // Write 'test' inside the shape
    SfxStringItem aInputString(SID_ATTR_CHAR, "test");
    pViewShell2->GetViewFrame()->GetDispatcher()->ExecuteList(SID_ATTR_CHAR, SfxCallMode::SYNCHRON,
                                                              { &aInputString });
    CPPUNIT_ASSERT(pView2->GetTextEditObject());
    EditView& rEditView = pView2->GetTextEditOutlinerView()->GetEditView();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(4), rEditView.GetSelection().nStartPos);
    pView2->SdrEndTextEdit();
    // Without the accompanying fix in place, this test would have failed with an assertion failure
    // in SdrObjEditView::SdrEndTextEdit() as mpOldTextEditUndoManager was not nullptr.
    pViewShell2->GetViewFrame()->GetDispatcher()->Execute(SID_UNDO, SfxCallMode::SYNCHRON);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
