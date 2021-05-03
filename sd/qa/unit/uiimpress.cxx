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

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/frame/DispatchHelper.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/intitem.hxx>
#include <svx/svxids.hrc>
#include <svx/svdoashp.hxx>
#include <svx/svdotable.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflclit.hxx>
#include <svx/xflgrit.hxx>
#include <svx/xlndsit.hxx>
#include <svl/stritem.hxx>
#include <undo/undomanager.hxx>
#include <vcl/scheduler.hxx>

#include <DrawDocShell.hxx>
#include <DrawController.hxx>
#include <ViewShell.hxx>
#include <app.hrc>
#include <drawdoc.hxx>
#include <sdpage.hxx>
#include <unomodel.hxx>

using namespace ::com::sun::star;

/// Impress UI tests.
class SdUiImpressTest : public test::BootstrapFixture, public unotest::MacrosTest
{
protected:
    uno::Reference<lang::XComponent> mxComponent;

public:
    virtual void setUp() override;
    virtual void tearDown() override;

    void checkCurrentPageNumber(sal_uInt16 nNum);
};

void SdUiImpressTest::setUp()
{
    test::BootstrapFixture::setUp();

    mxDesktop.set(frame::Desktop::create(mxComponentContext));
}

void SdUiImpressTest::tearDown()
{
    if (mxComponent.is())
        mxComponent->dispose();

    test::BootstrapFixture::tearDown();
}

void SdUiImpressTest::checkCurrentPageNumber(sal_uInt16 nNum)
{
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawView> xDrawView(xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xPage(xDrawView->getCurrentPage(), uno::UNO_SET_THROW);
    uno::Reference<beans::XPropertySet> xPropertySet(xPage, uno::UNO_QUERY);

    sal_uInt16 nPageNumber;
    xPropertySet->getPropertyValue("Number") >>= nPageNumber;
    CPPUNIT_ASSERT_EQUAL(nNum, nPageNumber);
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

CPPUNIT_TEST_FIXTURE(SdUiImpressTest, testTdf126197)
{
    // Load the document and create two new windows.
    mxComponent = loadFromDesktop(m_directories.getURLFromSrc("sd/qa/unit/data/tdf126197.odp"));
    auto pImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    sd::ViewShell* pViewShell = pImpressDocument->GetDocShell()->GetViewShell();
    pViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_NEWWINDOW, SfxCallMode::SYNCHRON);
    sd::ViewShell* pViewShell1 = pImpressDocument->GetDocShell()->GetViewShell();
    pViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_NEWWINDOW, SfxCallMode::SYNCHRON);
    sd::ViewShell* pViewShell2 = pImpressDocument->GetDocShell()->GetViewShell();
    CPPUNIT_ASSERT(pViewShell1 != pViewShell2);

    // Start text edit in window 1.
    SdPage* pPage1 = pViewShell1->GetActualPage();
    SdrObject* pShape1 = pPage1->GetObj(0);
    SdrView* pView1 = pViewShell1->GetView();
    pView1->MarkObj(pShape1, pView1->GetSdrPageView());
    pView1->SdrBeginTextEdit(pShape1);
    CPPUNIT_ASSERT(pView1->IsTextEdit());

    SdPage* pPage2 = pViewShell2->GetActualPage();
    CPPUNIT_ASSERT_EQUAL(pPage1, pPage2);
    SdrObject* pShape2 = pPage2->GetObj(0);
    CPPUNIT_ASSERT_EQUAL(pShape1, pShape2);
    SdrView* pView2 = pViewShell2->GetView();
    pView2->MarkObj(pShape2, pView2->GetSdrPageView());

    // Without the accompanying fix in place, this test would have failed with an assertion failure
    // in SdrObjEditView::SdrEndTextEdit()
    pViewShell2->GetViewFrame()->GetDispatcher()->Execute(SID_DELETE, SfxCallMode::SYNCHRON);
}

CPPUNIT_TEST_FIXTURE(SdUiImpressTest, testTdf128651)
{
    // Error was, that undo and redo changes size of the shape. Affected actions were e.g.
    // extrusion on/off, shadow on/off, changes on line or fill attributes.
    // All these actions do not change the snap rectangle.
    mxComponent = loadFromDesktop(
        m_directories.getURLFromSrc("sd/qa/unit/data/tdf128651_CustomShapeUndo.odp"));
    auto pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    SdPage* pActualPage = pViewShell->GetActualPage();
    SdrObject* pObject = pActualPage->GetObj(0);
    auto pCustomShape = dynamic_cast<SdrObjCustomShape*>(pObject);
    CPPUNIT_ASSERT_MESSAGE("No Shape", pCustomShape);
    const sal_Int32 nOrigWidth(pCustomShape->GetSnapRect().GetWidth());

    SdDrawDocument* pDocument = pXImpressDocument->GetDoc();
    sd::UndoManager* pUndoManager = pDocument->GetUndoManager();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), pUndoManager->GetUndoActionCount());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), pUndoManager->GetRedoActionCount());

    SdrView* pView = pViewShell->GetView();
    pView->MarkObj(pCustomShape, pView->GetSdrPageView());
    pViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_EXTRUSION_TOGGLE,
                                                         SfxCallMode::SYNCHRON);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pUndoManager->GetUndoActionCount());

    pViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_UNDO, SfxCallMode::SYNCHRON);
    const sal_Int32 nUndoWidth(pCustomShape->GetSnapRect().GetWidth());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Undo changes width", nOrigWidth, nUndoWidth);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pUndoManager->GetRedoActionCount());
    pViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_REDO, SfxCallMode::SYNCHRON);
    const sal_Int32 nRedoWidth(pCustomShape->GetSnapRect().GetWidth());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Redo changes width", nUndoWidth, nRedoWidth);
}

CPPUNIT_TEST_FIXTURE(SdUiImpressTest, testTdf129346)
{
    mxComponent = loadFromDesktop("private:factory/simpress",
                                  "com.sun.star.presentation.PresentationDocument");

    CPPUNIT_ASSERT(mxComponent.is());

    dispatchCommand(mxComponent, ".uno:DiaMode", {});
    Scheduler::ProcessEventsToIdle();
    checkCurrentPageNumber(1);

    dispatchCommand(mxComponent, ".uno:InsertPage", {});
    Scheduler::ProcessEventsToIdle();
    checkCurrentPageNumber(2);

    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();
    checkCurrentPageNumber(1);
}

CPPUNIT_TEST_FIXTURE(SdUiImpressTest, testTdf127481)
{
    mxComponent = loadFromDesktop("private:factory/simpress",
                                  "com.sun.star.presentation.PresentationDocument");

    CPPUNIT_ASSERT(mxComponent.is());

    auto pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    SdPage* pActualPage = pViewShell->GetActualPage();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), pActualPage->GetObjCount());

    uno::Sequence<beans::PropertyValue> aArgs(comphelper::InitPropertySequence(
        { { "Rows", uno::makeAny(sal_Int32(1)) }, { "Columns", uno::makeAny(sal_Int32(1)) } }));

    dispatchCommand(mxComponent, ".uno:InsertTable", aArgs);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), pActualPage->GetObjCount());

    dispatchCommand(mxComponent, ".uno:DuplicatePage", aArgs);
    Scheduler::ProcessEventsToIdle();

    checkCurrentPageNumber(2);

    pActualPage = pViewShell->GetActualPage();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), pActualPage->GetObjCount());

    auto pTableObject = dynamic_cast<sdr::table::SdrTableObj*>(pActualPage->GetObj(2));
    CPPUNIT_ASSERT(pTableObject);

    //without the fix, it would crash here
    pViewShell->GetView()->SdrBeginTextEdit(pTableObject);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), pActualPage->GetObjCount());
}

namespace
{
void dispatchCommand(const uno::Reference<lang::XComponent>& xComponent, const OUString& rCommand,
                     const uno::Sequence<beans::PropertyValue>& rPropertyValues)
{
    uno::Reference<frame::XController> xController
        = uno::Reference<frame::XModel>(xComponent, uno::UNO_QUERY_THROW)->getCurrentController();
    CPPUNIT_ASSERT(xController.is());
    uno::Reference<frame::XDispatchProvider> xFrame(xController->getFrame(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xFrame.is());

    uno::Reference<uno::XComponentContext> xContext = ::comphelper::getProcessComponentContext();
    uno::Reference<frame::XDispatchHelper> xDispatchHelper(frame::DispatchHelper::create(xContext));
    CPPUNIT_ASSERT(xDispatchHelper.is());

    xDispatchHelper->executeDispatch(xFrame, rCommand, OUString(), 0, rPropertyValues);
}
}

CPPUNIT_TEST_FIXTURE(SdUiImpressTest, testPageFillColor)
{
    // Load the document and create two new windows.
    mxComponent = loadFromDesktop(m_directories.getURLFromSrc("sd/qa/unit/data/tdf126197.odp"));
    auto pImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    sd::ViewShell* pViewShell = pImpressDocument->GetDocShell()->GetViewShell();

    // Set FillPageColor

    uno::Sequence<beans::PropertyValue> aPropertyValues(comphelper::InitPropertySequence({
        { "Color", uno::makeAny(OUString("ff0000")) },
    }));

    ::dispatchCommand(mxComponent, ".uno:FillPageColor", aPropertyValues);

    SdPage* pPage = pViewShell->getCurrentPage();
    const SfxItemSet& rPageAttr = pPage->getSdrPageProperties().GetItemSet();

    const XFillStyleItem* pFillStyle = rPageAttr.GetItem(XATTR_FILLSTYLE);
    drawing::FillStyle eXFS = pFillStyle->GetValue();
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, eXFS);

    Color aColor = rPageAttr.GetItem(XATTR_FILLCOLOR)->GetColorValue();
    CPPUNIT_ASSERT_EQUAL(OUString("ff0000"), aColor.AsRGBHexString());
}

CPPUNIT_TEST_FIXTURE(SdUiImpressTest, testPageFillGradient)
{
    // Load the document and create two new windows.
    mxComponent = loadFromDesktop(m_directories.getURLFromSrc("sd/qa/unit/data/tdf126197.odp"));
    auto pImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    sd::ViewShell* pViewShell = pImpressDocument->GetDocShell()->GetViewShell();

    // Set FillPageColor

    uno::Sequence<beans::PropertyValue> aPropertyValues(comphelper::InitPropertySequence({
        { "FillPageGradientJSON",
          uno::makeAny(
              OUString("{\"style\":\"LINEAR\",\"startcolor\":\"ff0000\",\"endcolor\":\"0000ff\","
                       "\"angle\":\"300\",\"border\":\"0\",\"x\":\"0\",\"y\":\"0\",\"intensstart\":"
                       "\"100\",\"intensend\":\"100\",\"stepcount\":\"0\"}")) },
    }));

    dispatchCommand(mxComponent, ".uno:FillPageGradient", aPropertyValues);

    SdPage* pPage = pViewShell->getCurrentPage();
    const SfxItemSet& rPageAttr = pPage->getSdrPageProperties().GetItemSet();

    const XFillStyleItem* pFillStyle = rPageAttr.GetItem(XATTR_FILLSTYLE);
    drawing::FillStyle eXFS = pFillStyle->GetValue();
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT, eXFS);

    XGradient aGradient = rPageAttr.GetItem(XATTR_FILLGRADIENT)->GetGradientValue();
    CPPUNIT_ASSERT_EQUAL(OUString("ff0000"), aGradient.GetStartColor().AsRGBHexString());
    CPPUNIT_ASSERT_EQUAL(OUString("0000ff"), aGradient.GetEndColor().AsRGBHexString());
}

CPPUNIT_TEST_FIXTURE(SdUiImpressTest, testTdf134053)
{
    // Error was, that dashes and dots were longer than in MS Office.
    mxComponent = loadFromDesktop(
        m_directories.getURLFromSrc("sd/qa/unit/data/pptx/tdf134053_dashdot.pptx"));
    auto pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    SdPage* pActualPage = pViewShell->GetActualPage();
    SdrObject* pShape = pActualPage->GetObj(0);
    CPPUNIT_ASSERT_MESSAGE("No Shape", pShape);

    XDash dash = pShape->GetMergedItem(XATTR_LINEDASH).GetDashValue();

    // Because 0% is not possible as dash length (as of June 2020) 1% is used in the fix.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Distance", 399.0, dash.GetDistance());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Dot length", 301.0, dash.GetDotLen());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Dash length", 1.0, dash.GetDashLen());
}

CPPUNIT_TEST_FIXTURE(SdUiImpressTest, testSpellOnlineParameter)
{
    mxComponent = loadFromDesktop(m_directories.getURLFromSrc("sd/qa/unit/data/empty.fodp"));
    auto pImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    bool bSet = pImpressDocument->GetDoc()->GetOnlineSpell();

    uno::Sequence<beans::PropertyValue> params(
        comphelper::InitPropertySequence({ { "Enable", uno::makeAny(!bSet) } }));
    dispatchCommand(mxComponent, ".uno:SpellOnline", params);
    CPPUNIT_ASSERT_EQUAL(!bSet, pImpressDocument->GetDoc()->GetOnlineSpell());

    // set the same state as now and we don't expect any change (no-toggle)
    params = comphelper::InitPropertySequence({ { "Enable", uno::makeAny(!bSet) } });
    dispatchCommand(mxComponent, ".uno:SpellOnline", params);
    CPPUNIT_ASSERT_EQUAL(!bSet, pImpressDocument->GetDoc()->GetOnlineSpell());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
