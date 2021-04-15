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
#include <LibreOfficeKit/LibreOfficeKitEnums.h>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/XDrawView.hpp>
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
#include <SlideSorterViewShell.hxx>
#include <SlideSorter.hxx>
#include <controller/SlideSorterController.hxx>
#include <controller/SlsClipboard.hxx>
#include <controller/SlsPageSelector.hxx>
#include <svl/stritem.hxx>
#include <undo/undomanager.hxx>
#include <vcl/scheduler.hxx>

#include <DrawDocShell.hxx>
#include <ViewShell.hxx>
#include <app.hrc>
#include <drawdoc.hxx>
#include <sdpage.hxx>
#include <unomodel.hxx>
#include <osl/thread.hxx>

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
    void insertStringToObject(sal_uInt16 nObj, const std::string& rStr);
    sd::slidesorter::SlideSorterViewShell* getSlideSorterViewShell();
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

void SdUiImpressTest::insertStringToObject(sal_uInt16 nObj, const std::string& rStr)
{
    auto pImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    sd::ViewShell* pViewShell = pImpressDocument->GetDocShell()->GetViewShell();
    SdPage* pPage = pViewShell->GetActualPage();
    SdrObject* pShape = pPage->GetObj(nObj);
    CPPUNIT_ASSERT_MESSAGE("No Shape", pShape);
    SdrView* pView = pViewShell->GetView();
    pView->MarkObj(pShape, pView->GetSdrPageView());

    CPPUNIT_ASSERT(!pView->IsTextEdit());

    for (const char c : rStr)
    {
        pImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, c, 0);
        pImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, c, 0);
        Scheduler::ProcessEventsToIdle();
    }

    CPPUNIT_ASSERT(pView->IsTextEdit());

    pImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::ESCAPE);
    pImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::ESCAPE);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT(!pView->IsTextEdit());
}

sd::slidesorter::SlideSorterViewShell* SdUiImpressTest::getSlideSorterViewShell()
{
    auto pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    sd::slidesorter::SlideSorterViewShell* pSSVS = nullptr;
    // Same as in sd/qa/unit/misc-tests.cxx
    for (int i = 0; i < 1000; i++)
    {
        // Process all Tasks - slide sorter is created here
        Scheduler::ProcessEventsToIdle();
        if ((pSSVS = sd::slidesorter::SlideSorterViewShell::GetSlideSorter(
                 pViewShell->GetViewShellBase()))
            != nullptr)
            break;
        osl::Thread::wait(std::chrono::milliseconds(100));
    }
    CPPUNIT_ASSERT(pSSVS);
    return pSSVS;
}

CPPUNIT_TEST_FIXTURE(SdUiImpressTest, testTdf111522)
{
    // Load the document and create two new windows.
    mxComponent = loadFromDesktop(m_directories.getURLFromSrc(u"sd/qa/unit/data/tdf111522.odp"));
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
    pView2->RotateMarkedObj(pShape2->GetLastBoundRect().Center(), /*nAngle=*/45_deg100);
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
    mxComponent = loadFromDesktop(m_directories.getURLFromSrc(u"sd/qa/unit/data/tdf126197.odp"));
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

CPPUNIT_TEST_FIXTURE(SdUiImpressTest, testTdf124708)
{
    mxComponent = loadFromDesktop(m_directories.getURLFromSrc(u"sd/qa/unit/data/tdf124708.ppt"));

    CPPUNIT_ASSERT(mxComponent.is());

    dispatchCommand(mxComponent, ".uno:NextPage", {});
    Scheduler::ProcessEventsToIdle();

    checkCurrentPageNumber(2);

    auto pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    SdPage* pActualPage = pViewShell->GetActualPage();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(16), pActualPage->GetObjCount());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    Scheduler::ProcessEventsToIdle();

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, ".uno:Delete", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), pActualPage->GetObjCount());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(16), pActualPage->GetObjCount());
}

CPPUNIT_TEST_FIXTURE(SdUiImpressTest, testTdf139996)
{
    mxComponent = loadFromDesktop("private:factory/simpress",
                                  "com.sun.star.presentation.PresentationDocument");

    CPPUNIT_ASSERT(mxComponent.is());

    sd::slidesorter::SlideSorterViewShell* pSSVS = getSlideSorterViewShell();
    auto& rSSController = pSSVS->GetSlideSorter().GetController();
    auto& rPageSelector = rSSController.GetPageSelector();

    CPPUNIT_ASSERT_EQUAL(1, rPageSelector.GetSelectedPageCount());

    rPageSelector.DeselectAllPages();

    CPPUNIT_ASSERT_EQUAL(0, rPageSelector.GetSelectedPageCount());

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, ".uno:MovePageUp", {});
    Scheduler::ProcessEventsToIdle();

    dispatchCommand(mxComponent, ".uno:MovePageDown", {});
    Scheduler::ProcessEventsToIdle();

    dispatchCommand(mxComponent, ".uno:MovePageTop", {});
    Scheduler::ProcessEventsToIdle();

    dispatchCommand(mxComponent, ".uno:MovePageBottom", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(0, rPageSelector.GetSelectedPageCount());
}

CPPUNIT_TEST_FIXTURE(SdUiImpressTest, testTdf128651)
{
    // Error was, that undo and redo changes size of the shape. Affected actions were e.g.
    // extrusion on/off, shadow on/off, changes on line or fill attributes.
    // All these actions do not change the snap rectangle.
    mxComponent = loadFromDesktop(
        m_directories.getURLFromSrc(u"sd/qa/unit/data/tdf128651_CustomShapeUndo.odp"));
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

CPPUNIT_TEST_FIXTURE(SdUiImpressTest, testTdf100950)
{
    mxComponent = loadFromDesktop("private:factory/simpress",
                                  "com.sun.star.presentation.PresentationDocument");

    CPPUNIT_ASSERT(mxComponent.is());

    dispatchCommand(mxComponent, ".uno:InsertPage", {});
    Scheduler::ProcessEventsToIdle();

    dispatchCommand(mxComponent, ".uno:InsertPage", {});
    Scheduler::ProcessEventsToIdle();

    insertStringToObject(0, "Test");

    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    sd::slidesorter::SlideSorterViewShell* pSSVS = getSlideSorterViewShell();
    auto& rSSController = pSSVS->GetSlideSorter().GetController();
    auto& rPageSelector = rSSController.GetPageSelector();

    // Without the fix in place, this test would have failed here
    CPPUNIT_ASSERT(rPageSelector.IsPageSelected(2));
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

CPPUNIT_TEST_FIXTURE(SdUiImpressTest, testmoveSlides)
{
    mxComponent = loadFromDesktop("private:factory/simpress",
                                  "com.sun.star.presentation.PresentationDocument");

    CPPUNIT_ASSERT(mxComponent.is());

    auto pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();

    uno::Sequence<beans::PropertyValue> aArgs(
        comphelper::InitPropertySequence({ { "PageName", uno::makeAny(OUString("Test 1")) },
                                           { "WhatLayout", uno::makeAny(sal_Int32(1)) },
                                           { "IsPageBack", uno::makeAny(false) },
                                           { "IsPageObj", uno::makeAny(false) } }));

    dispatchCommand(mxComponent, ".uno:InsertPage", aArgs);
    Scheduler::ProcessEventsToIdle();
    checkCurrentPageNumber(2);

    CPPUNIT_ASSERT_EQUAL(OUString("Test 1"), pViewShell->GetActualPage()->GetName());

    aArgs = comphelper::InitPropertySequence({ { "PageName", uno::makeAny(OUString("Test 2")) },
                                               { "WhatLayout", uno::makeAny(sal_Int32(1)) },
                                               { "IsPageBack", uno::makeAny(false) },
                                               { "IsPageObj", uno::makeAny(false) } });

    dispatchCommand(mxComponent, ".uno:InsertPage", aArgs);
    Scheduler::ProcessEventsToIdle();
    checkCurrentPageNumber(3);

    CPPUNIT_ASSERT_EQUAL(OUString("Test 2"), pViewShell->GetActualPage()->GetName());

    // Move slide 'Test 2' up
    for (size_t i = 2; i > 0; --i)
    {
        dispatchCommand(mxComponent, ".uno:MovePageUp", {});
        Scheduler::ProcessEventsToIdle();
        checkCurrentPageNumber(i);
        CPPUNIT_ASSERT_EQUAL(OUString("Test 2"), pViewShell->GetActualPage()->GetName());
    }

    // Move slide 'Test 2' down
    for (size_t i = 2; i < 4; ++i)
    {
        dispatchCommand(mxComponent, ".uno:MovePageDown", {});
        Scheduler::ProcessEventsToIdle();
        checkCurrentPageNumber(i);
        CPPUNIT_ASSERT_EQUAL(OUString("Test 2"), pViewShell->GetActualPage()->GetName());
    }

    // Move slide 'Test 2' to the top
    dispatchCommand(mxComponent, ".uno:MovePageFirst", {});
    Scheduler::ProcessEventsToIdle();
    checkCurrentPageNumber(1);
    CPPUNIT_ASSERT_EQUAL(OUString("Test 2"), pViewShell->GetActualPage()->GetName());

    // Move slide 'Test 2' to the bottom
    dispatchCommand(mxComponent, ".uno:MovePageLast", {});
    Scheduler::ProcessEventsToIdle();
    checkCurrentPageNumber(3);
    CPPUNIT_ASSERT_EQUAL(OUString("Test 2"), pViewShell->GetActualPage()->GetName());
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
    mxComponent = loadFromDesktop(m_directories.getURLFromSrc(u"sd/qa/unit/data/tdf126197.odp"));
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
    CPPUNIT_ASSERT_EQUAL(Color(0xff0000), aColor);
}

CPPUNIT_TEST_FIXTURE(SdUiImpressTest, testPageFillGradient)
{
    // Load the document and create two new windows.
    mxComponent = loadFromDesktop(m_directories.getURLFromSrc(u"sd/qa/unit/data/tdf126197.odp"));
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
    CPPUNIT_ASSERT_EQUAL(Color(0xff0000), aGradient.GetStartColor());
    CPPUNIT_ASSERT_EQUAL(Color(0x0000ff), aGradient.GetEndColor());
}

CPPUNIT_TEST_FIXTURE(SdUiImpressTest, testTdf134053)
{
    // Error was, that dashes and dots were longer than in MS Office.
    mxComponent = loadFromDesktop(
        m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf134053_dashdot.pptx"));
    auto pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    SdPage* pActualPage = pViewShell->GetActualPage();
    SdrObject* pShape = pActualPage->GetObj(0);
    CPPUNIT_ASSERT_MESSAGE("No Shape", pShape);

    // Break line into single dash and dot objects
    SdrView* pView = pViewShell->GetView();
    pView->MarkObj(pShape, pView->GetSdrPageView());
    dispatchCommand(mxComponent, ".uno:ConvertIntoMetafile", {});
    dispatchCommand(mxComponent, ".uno:Break", {});

    // Measure the rendered length of dash, dot and distance
    SdrObject* pDash = pActualPage->GetObj(0);
    const tools::Rectangle& rBoundDashRect = pDash->GetCurrentBoundRect();
    const double fDashLength(rBoundDashRect.GetWidth());
    SdrObject* pDot = pActualPage->GetObj(1);
    const tools::Rectangle& rBoundDotRect = pDot->GetCurrentBoundRect();
    const double fDotLength(rBoundDotRect.GetWidth());
    const double fDistance(rBoundDotRect.Left() - rBoundDashRect.Right());

    // Because 0% is not possible as dash length (as of June 2020) 1% is used in the fix.
    // For that a larger delta is here allowed to the ideal value than needed for
    // rounding errors.
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Dot length", 706, fDotLength, 12);

    // tdf#136957 (dotted lines are  disappearing in presentation mode)
    // Test value used as 2089 instead of 2117 for tdf#136957 workaround.
    // If this test fails as Expected: 2089 Actual:2117
    // plaese test tdf#136957 manually and use 2117 as test value again.
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Distance", 2089, fDistance, 12);

    // tdf#136957 (dotted lines are  disappearing in presentation mode)
    // Test value used as 2854 instead of 2822 for tdf#136957 workaround.
    // If this test fails as Expected: 2854 Actual:2822
    // plaese test tdf#136957 manually and use 2822 as test value again.
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Dash length", 2854, fDashLength, 12);
}

CPPUNIT_TEST_FIXTURE(SdUiImpressTest, testSpellOnlineParameter)
{
    mxComponent = loadFromDesktop(m_directories.getURLFromSrc(u"sd/qa/unit/data/empty.fodp"));
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

CPPUNIT_TEST_FIXTURE(SdUiImpressTest, testTdf123841)
{
    // To check if selecting unfilled rectangle produces unfilled rectangle
    mxComponent = loadFromDesktop("private:factory/simpress",
                                  "com.sun.star.presentation.PresentationDocument");
    auto pImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pImpressDocument);

    uno::Sequence<beans::PropertyValue> aArgs(
        comphelper::InitPropertySequence({ { "KeyModifier", uno::makeAny(KEY_MOD1) } }));
    dispatchCommand(mxComponent, ".uno:Rect_Unfilled", aArgs);
    Scheduler::ProcessEventsToIdle();

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws = xDrawPagesSupplier->getDrawPages();

    int getShapes = xDraws->getCount();
    CPPUNIT_ASSERT_EQUAL(1, getShapes);

    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);

    for (int i = 0; i < 3; i++)
    {
        uno::Reference<beans::XPropertySet> XPropSet(xDrawPage->getByIndex(i), uno::UNO_QUERY);
        drawing::FillStyle eFillStyle = drawing::FillStyle_NONE;
        XPropSet->getPropertyValue("FillStyle") >>= eFillStyle;

        // Without the fix in place, this test would have failed with
        // with drawing::FillStyle_NONE != drawing::FillStyle_SOLID
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_NONE, eFillStyle);
    }
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
