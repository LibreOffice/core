/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sdmodeltestbase.hxx"

#include <LibreOfficeKit/LibreOfficeKitEnums.h>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/XDrawView.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XMasterPageTarget.hpp>
#include <com/sun/star/table/XMergeableCell.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>

#include <comphelper/dispatchcommand.hxx>
#include <comphelper/propertysequence.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/intitem.hxx>
#include <svl/srchitem.hxx>
#include <svx/svxids.hrc>
#include <svx/svdoashp.hxx>
#include <svx/svdotable.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflclit.hxx>
#include <svx/xflgrit.hxx>
#include <svx/xlndsit.hxx>
#include <SlideSorterViewShell.hxx>
#include <SlideSorter.hxx>
#include <controller/SlsClipboard.hxx>
#include <controller/SlideSorterController.hxx>
#include <controller/SlsPageSelector.hxx>
#include <svl/stritem.hxx>
#include <undo/undomanager.hxx>
#include <vcl/scheduler.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/sequenceashashmap.hxx>

#include <drawdoc.hxx>
#include <DrawDocShell.hxx>
#include <ViewShell.hxx>
#include <app.hrc>
#include <sdpage.hxx>
#include <unomodel.hxx>
#include <osl/thread.hxx>
#include <slideshow.hxx>

using namespace ::com::sun::star;

/// Impress UI tests.
class SdUiImpressTest : public SdModelTestBase
{
public:
    SdUiImpressTest()
        : SdModelTestBase("/sd/qa/unit/data/")
    {
    }

    void checkCurrentPageNumber(sal_uInt16 nNum);
    void typeString(SdXImpressDocument* rImpressDocument, const std::u16string_view& rStr);
    void typeKey(SdXImpressDocument* rImpressDocument, const sal_uInt16 nKey);
    void insertStringToObject(sal_uInt16 nObj, const std::u16string_view& rStr, bool bUseEscape);
    sd::slidesorter::SlideSorterViewShell* getSlideSorterViewShell();
};

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

void SdUiImpressTest::typeKey(SdXImpressDocument* rImpressDocument, const sal_uInt16 nKey)
{
    rImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, nKey);
    rImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, nKey);
    Scheduler::ProcessEventsToIdle();
}

void SdUiImpressTest::typeString(SdXImpressDocument* rImpressDocument,
                                 const std::u16string_view& rStr)
{
    for (const char16_t c : rStr)
    {
        rImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, c, 0);
        rImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, c, 0);
        Scheduler::ProcessEventsToIdle();
    }
}

void SdUiImpressTest::insertStringToObject(sal_uInt16 nObj, const std::u16string_view& rStr,
                                           bool bUseEscape)
{
    auto pImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    sd::ViewShell* pViewShell = pImpressDocument->GetDocShell()->GetViewShell();
    SdPage* pPage = pViewShell->GetActualPage();
    SdrObject* pShape = pPage->GetObj(nObj);
    CPPUNIT_ASSERT_MESSAGE("No Shape", pShape);
    SdrView* pView = pViewShell->GetView();
    pView->MarkObj(pShape, pView->GetSdrPageView());
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT(!pView->IsTextEdit());

    typeString(pImpressDocument, rStr);

    CPPUNIT_ASSERT(pView->IsTextEdit());

    if (bUseEscape)
    {
        typeKey(pImpressDocument, KEY_ESCAPE);
        CPPUNIT_ASSERT(!pView->IsTextEdit());
    }
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

static void lcl_search(const OUString& rKey, bool bFindAll = false, bool bBackwards = false)
{
    Scheduler::ProcessEventsToIdle();
    SvxSearchCmd eSearch = bFindAll ? SvxSearchCmd::FIND_ALL : SvxSearchCmd::FIND;

    uno::Sequence<beans::PropertyValue> aPropertyValues(comphelper::InitPropertySequence({
        { "SearchItem.SearchString", uno::Any(rKey) },
        { "SearchItem.Backward", uno::Any(bBackwards) },
        { "SearchItem.Command", uno::Any(sal_uInt16(eSearch)) },
    }));

    comphelper::dispatchCommand(".uno:ExecuteSearch", aPropertyValues);
    Scheduler::ProcessEventsToIdle();
}

CPPUNIT_TEST_FIXTURE(SdUiImpressTest, testTdf111522)
{
    // Load the document and create two new windows.
    createSdImpressDoc("tdf111522.odp");
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
    createSdImpressDoc("tdf126197.odp");
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
    createSdImpressDoc("tdf124708.ppt");

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

CPPUNIT_TEST_FIXTURE(SdUiImpressTest, testTdf143412)
{
    createSdImpressDoc();

    auto pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();

    SdPage* pActualPage = pViewShell->GetActualPage();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), pActualPage->GetObjCount());

    OUString aImageURL = createFileURL(u"tdf143412.svg");
    uno::Sequence<beans::PropertyValue> aArgs(comphelper::InitPropertySequence({
        { "FileName", uno::Any(aImageURL) },
    }));
    dispatchCommand(mxComponent, ".uno:InsertGraphic", aArgs);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), pActualPage->GetObjCount());

    // Without the fix in place, this test would have crashed
    // Check that converting an image to the different options doesn't crash

    dispatchCommand(mxComponent, ".uno:ChangeBezier", {});

    dispatchCommand(mxComponent, ".uno:ChangePolygon", {});

    dispatchCommand(mxComponent, ".uno:convert_to_contour", {});

    dispatchCommand(mxComponent, ".uno:ConvertInto3D", {});

    dispatchCommand(mxComponent, ".uno:ConvertInto3DLatheFast", {});

    dispatchCommand(mxComponent, ".uno:ConvertIntoBitmap", {});

    dispatchCommand(mxComponent, ".uno:ConvertIntoMetaFile", {});

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), pActualPage->GetObjCount());
}

CPPUNIT_TEST_FIXTURE(SdUiImpressTest, testTdf96206)
{
    // Copying/pasting slide referring to a non-default master with a text duplicated the master

    createSdImpressDoc("odp/tdf96206.odp");

    sd::slidesorter::SlideSorterViewShell* pSSVS = getSlideSorterViewShell();
    auto& rSSController = pSSVS->GetSlideSorter().GetController();

    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();
    const sal_uInt16 nMasterPageCnt1 = pDoc->GetMasterSdPageCount(PageKind::Standard);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(2), nMasterPageCnt1);
    rSSController.GetClipboard().DoCopy();
    rSSController.GetClipboard().DoPaste();
    const sal_uInt16 nMasterPageCnt2 = pDoc->GetMasterSdPageCount(PageKind::Standard);
    CPPUNIT_ASSERT_EQUAL(nMasterPageCnt1, nMasterPageCnt2);
}

CPPUNIT_TEST_FIXTURE(SdUiImpressTest, testTdf96708)
{
    createSdImpressDoc("odp/tdf96708.odp");

    sd::slidesorter::SlideSorterViewShell* pSSVS = getSlideSorterViewShell();
    auto& rSSController = pSSVS->GetSlideSorter().GetController();
    auto& rPageSelector = rSSController.GetPageSelector();

    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();
    const sal_uInt16 nMasterPageCnt1 = pDoc->GetMasterSdPageCount(PageKind::Standard);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(4), nMasterPageCnt1);
    rSSController.GetClipboard().DoCopy();
    rPageSelector.SelectAllPages();

    // Now wait for timers to trigger creation of auto-layout
    osl::Thread::wait(std::chrono::milliseconds(100));
    Scheduler::ProcessEventsToIdle();

    rSSController.GetClipboard().DoPaste();

    const sal_uInt16 nMasterPageCnt2 = pDoc->GetMasterSdPageCount(PageKind::Standard);
    //FIXME: tdf#151802: Number of master pages should be 4, it's 5 instead
    //CPPUNIT_ASSERT_EQUAL(nMasterPageCnt1, nMasterPageCnt2);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(5), nMasterPageCnt2);
}

CPPUNIT_TEST_FIXTURE(SdUiImpressTest, testTdf139996)
{
    createSdImpressDoc();

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
    createSdImpressDoc("tdf128651_CustomShapeUndo.odp");
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

CPPUNIT_TEST_FIXTURE(SdUiImpressTest, testTdf126605)
{
    createSdImpressDoc();

    dispatchCommand(mxComponent, ".uno:InsertPage", {});
    Scheduler::ProcessEventsToIdle();

    insertStringToObject(0, u"Test", /*bUseEscape*/ false);

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(1),
                                                 uno::UNO_QUERY);

    uno::Reference<beans::XPropertySet> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);

    uno::Reference<text::XText> xText
        = uno::Reference<text::XTextRange>(xShape, uno::UNO_QUERY_THROW)->getText();
    CPPUNIT_ASSERT_MESSAGE("Not a text shape", xText.is());

    uno::Reference<container::XEnumerationAccess> paraEnumAccess(xText, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> paraEnum(paraEnumAccess->createEnumeration());

    // Get first paragraph
    uno::Reference<text::XTextRange> xParagraph(paraEnum->nextElement(), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPropSet(xParagraph, uno::UNO_QUERY_THROW);

    sal_Int16 nWritingMode = 0;
    xPropSet->getPropertyValue("WritingMode") >>= nWritingMode;
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::LR_TB, nWritingMode);

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, ".uno:ParaRightToLeft", {});
    Scheduler::ProcessEventsToIdle();

    xPropSet->getPropertyValue("WritingMode") >>= nWritingMode;
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::RL_TB, nWritingMode);

    dispatchCommand(mxComponent, ".uno:ParaLeftToRight", {});
    Scheduler::ProcessEventsToIdle();

    xPropSet->getPropertyValue("WritingMode") >>= nWritingMode;
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::LR_TB, nWritingMode);
}

CPPUNIT_TEST_FIXTURE(SdUiImpressTest, testTdf100950)
{
    createSdImpressDoc();

    dispatchCommand(mxComponent, ".uno:InsertPage", {});
    Scheduler::ProcessEventsToIdle();

    dispatchCommand(mxComponent, ".uno:InsertPage", {});
    Scheduler::ProcessEventsToIdle();

    insertStringToObject(0, u"Test", /*bUseEscape*/ true);

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
    createSdImpressDoc();

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
    createSdImpressDoc();

    auto pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();

    uno::Sequence<beans::PropertyValue> aArgs(
        comphelper::InitPropertySequence({ { "PageName", uno::Any(OUString("Test 1")) },
                                           { "WhatLayout", uno::Any(sal_Int32(1)) },
                                           { "IsPageBack", uno::Any(false) },
                                           { "IsPageObj", uno::Any(false) } }));

    dispatchCommand(mxComponent, ".uno:InsertPage", aArgs);
    Scheduler::ProcessEventsToIdle();
    checkCurrentPageNumber(2);

    CPPUNIT_ASSERT_EQUAL(OUString("Test 1"), pViewShell->GetActualPage()->GetName());

    aArgs = comphelper::InitPropertySequence({ { "PageName", uno::Any(OUString("Test 2")) },
                                               { "WhatLayout", uno::Any(sal_Int32(1)) },
                                               { "IsPageBack", uno::Any(false) },
                                               { "IsPageObj", uno::Any(false) } });

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

CPPUNIT_TEST_FIXTURE(SdUiImpressTest, testTdf148620)
{
    createSdImpressDoc();

    auto pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xShape(xDrawPage->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString(u""), xShape->getString());

    insertStringToObject(1, u"one", /*bUseEscape*/ false);
    typeKey(pXImpressDocument, KEY_RETURN);
    typeString(pXImpressDocument, u"two");
    typeKey(pXImpressDocument, KEY_RETURN);
    typeString(pXImpressDocument, u"three");
    typeKey(pXImpressDocument, KEY_RETURN);
    typeString(pXImpressDocument, u"four");
    typeKey(pXImpressDocument, KEY_RETURN);
    typeString(pXImpressDocument, u"five");
    typeKey(pXImpressDocument, KEY_RETURN);
    typeString(pXImpressDocument, u"six");

    CPPUNIT_ASSERT_EQUAL(OUString(u"One\nTwo\nThree\nFour\nFive\nsix"), xShape->getString());

    uno::Sequence<beans::PropertyValue> aArgs(
        comphelper::InitPropertySequence({ { "KeyModifier", uno::Any(sal_Int32(0)) } }));
    dispatchCommand(mxComponent, ".uno:OutlineUp", aArgs);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(OUString(u"One\nTwo\nThree\nFour\nsix\nFive"), xShape->getString());

    dispatchCommand(mxComponent, ".uno:OutlineUp", aArgs);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(OUString(u"One\nTwo\nThree\nsix\nFour\nFive"), xShape->getString());

    dispatchCommand(mxComponent, ".uno:OutlineUp", aArgs);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(OUString(u"One\nTwo\nsix\nThree\nFour\nFive"), xShape->getString());

    dispatchCommand(mxComponent, ".uno:OutlineUp", aArgs);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(OUString(u"One\nsix\nTwo\nThree\nFour\nFive"), xShape->getString());

    dispatchCommand(mxComponent, ".uno:OutlineUp", aArgs);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(OUString(u"six\nOne\nTwo\nThree\nFour\nFive"), xShape->getString());

    dispatchCommand(mxComponent, ".uno:OutlineDown", aArgs);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(OUString(u"One\nsix\nTwo\nThree\nFour\nFive"), xShape->getString());

    dispatchCommand(mxComponent, ".uno:OutlineDown", aArgs);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(OUString(u"One\nTwo\nsix\nThree\nFour\nFive"), xShape->getString());

    dispatchCommand(mxComponent, ".uno:OutlineDown", aArgs);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(OUString(u"One\nTwo\nThree\nsix\nFour\nFive"), xShape->getString());

    dispatchCommand(mxComponent, ".uno:OutlineDown", aArgs);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(OUString(u"One\nTwo\nThree\nFour\nsix\nFive"), xShape->getString());

    dispatchCommand(mxComponent, ".uno:OutlineDown", aArgs);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(OUString(u"One\nTwo\nThree\nFour\nFive\nsix"), xShape->getString());
}

CPPUNIT_TEST_FIXTURE(SdUiImpressTest, testTdf141703)
{
    createSdImpressDoc();

    auto pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());

    uno::Sequence<beans::PropertyValue> aArgs(comphelper::InitPropertySequence(
        { { "Rows", uno::Any(sal_Int32(2)) }, { "Columns", uno::Any(sal_Int32(2)) } }));

    dispatchCommand(mxComponent, ".uno:InsertTable", aArgs);
    Scheduler::ProcessEventsToIdle();

    // Move to A1 using Alt + Tab and write 'A'
    for (int i = 0; i < 3; i++)
    {
        pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_SHIFT | KEY_TAB);
        pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_SHIFT | KEY_TAB);
        Scheduler::ProcessEventsToIdle();
    }

    typeString(pXImpressDocument, u"A");

    // Move to A2 with Tab and write 'B'
    typeKey(pXImpressDocument, KEY_TAB);

    typeString(pXImpressDocument, u"B");

    typeKey(pXImpressDocument, KEY_ESCAPE);

    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    SdPage* pActualPage = pViewShell->GetActualPage();
    auto pTableObject = dynamic_cast<sdr::table::SdrTableObj*>(pActualPage->GetObj(2));
    CPPUNIT_ASSERT(pTableObject);

    uno::Reference<css::table::XTable> xTable(pTableObject->getTable(), uno::UNO_SET_THROW);
    uno::Reference<css::table::XMergeableCell> xCellA1(xTable->getCellByPosition(0, 0),
                                                       uno::UNO_QUERY_THROW);
    uno::Reference<css::table::XMergeableCell> xCellA2(xTable->getCellByPosition(1, 0),
                                                       uno::UNO_QUERY_THROW);

    uno::Reference<text::XText> xTextA1
        = uno::Reference<text::XTextRange>(xCellA1, uno::UNO_QUERY_THROW)->getText();

    // Without the fix in place, this test would have failed with
    // - Expected: A
    // - Actual  :
    CPPUNIT_ASSERT_EQUAL(OUString("A"), xTextA1->getString());

    uno::Reference<text::XText> xTextA2
        = uno::Reference<text::XTextRange>(xCellA2, uno::UNO_QUERY_THROW)->getText();
    CPPUNIT_ASSERT_EQUAL(OUString("B"), xTextA2->getString());
}

CPPUNIT_TEST_FIXTURE(SdUiImpressTest, testTdf127481)
{
    createSdImpressDoc();

    auto pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    SdPage* pActualPage = pViewShell->GetActualPage();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), pActualPage->GetObjCount());

    uno::Sequence<beans::PropertyValue> aArgs(comphelper::InitPropertySequence(
        { { "Rows", uno::Any(sal_Int32(1)) }, { "Columns", uno::Any(sal_Int32(1)) } }));

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

CPPUNIT_TEST_FIXTURE(SdUiImpressTest, testPageFillColor)
{
    // Load the document and create two new windows.
    createSdImpressDoc("tdf126197.odp");
    auto pImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    sd::ViewShell* pViewShell = pImpressDocument->GetDocShell()->GetViewShell();

    // Set FillPageColor

    uno::Sequence<beans::PropertyValue> aPropertyValues = {
        comphelper::makePropertyValue("FillColor", static_cast<sal_Int32>(0xff0000)),
    };
    dispatchCommand(mxComponent, ".uno:FillPageColor", aPropertyValues);

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
    createSdImpressDoc("tdf126197.odp");
    auto pImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    sd::ViewShell* pViewShell = pImpressDocument->GetDocShell()->GetViewShell();

    // Set FillPageColor

    uno::Sequence<beans::PropertyValue> aPropertyValues(comphelper::InitPropertySequence({
        { "FillPageGradientJSON",
          uno::Any(
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
    createSdImpressDoc("pptx/tdf134053_dashdot.pptx");
    auto pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    SdPage* pActualPage = pViewShell->GetActualPage();
    SdrObject* pShape = pActualPage->GetObj(0);
    CPPUNIT_ASSERT_MESSAGE("No Shape", pShape);

    SdDrawDocument* pDocument = pXImpressDocument->GetDoc();
    sd::UndoManager* pUndoManager = pDocument->GetUndoManager();

    // tdf#114613: Without the fix in place, this test would have failed with
    // - Expected: 0
    // - Actual  : 8
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), pUndoManager->GetUndoActionCount());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), pUndoManager->GetRedoActionCount());

    XDash dash = pShape->GetMergedItem(XATTR_LINEDASH).GetDashValue();

    // Because 0% is not possible as dash length (as of June 2020) 1% is used in the fix.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Distance", 399.0, dash.GetDistance());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Dot length", 301.0, dash.GetDotLen());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Dash length", 1.0, dash.GetDashLen());
}

CPPUNIT_TEST_FIXTURE(SdUiImpressTest, testSpellOnlineParameter)
{
    createSdImpressDoc();
    auto pImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    bool bSet = pImpressDocument->GetDoc()->GetOnlineSpell();

    uno::Sequence<beans::PropertyValue> params(
        comphelper::InitPropertySequence({ { "Enable", uno::Any(!bSet) } }));
    dispatchCommand(mxComponent, ".uno:SpellOnline", params);
    CPPUNIT_ASSERT_EQUAL(!bSet, pImpressDocument->GetDoc()->GetOnlineSpell());

    // set the same state as now and we don't expect any change (no-toggle)
    params = comphelper::InitPropertySequence({ { "Enable", uno::Any(!bSet) } });
    dispatchCommand(mxComponent, ".uno:SpellOnline", params);
    CPPUNIT_ASSERT_EQUAL(!bSet, pImpressDocument->GetDoc()->GetOnlineSpell());
}

CPPUNIT_TEST_FIXTURE(SdUiImpressTest, testTdf38669)
{
    createSdImpressDoc();
    auto pImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pImpressDocument);

    // Insert shape with ctrl key
    uno::Sequence<beans::PropertyValue> aArgs(
        comphelper::InitPropertySequence({ { "KeyModifier", uno::Any(KEY_MOD1) } }));
    dispatchCommand(mxComponent, ".uno:BasicShapes.rectangle", aArgs);
    Scheduler::ProcessEventsToIdle();

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xDrawPage->getCount());

    typeString(pImpressDocument, u"°");

    uno::Reference<text::XTextRange> xShape(xDrawPage->getByIndex(2), uno::UNO_QUERY);
    // Without the fix in place, this test would have failed with:
    // - Expected: °
    // - Actual  : ㅀ
    CPPUNIT_ASSERT_EQUAL(OUString(u"°"), xShape->getString());
}

CPPUNIT_TEST_FIXTURE(SdUiImpressTest, testTdf151417)
{
    createSdImpressDoc();
    auto pImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pImpressDocument);

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws = xDrawPagesSupplier->getDrawPages();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xDraws->getCount());

    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xDrawPage->getCount());

    uno::Sequence<beans::PropertyValue> aArgs(
        comphelper::InitPropertySequence({ { "KeyModifier", uno::Any(KEY_MOD1) } }));

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, ".uno:Edit", aArgs);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xDrawPage->getCount());
}

CPPUNIT_TEST_FIXTURE(SdUiImpressTest, testTdf123841)
{
    // To check if selecting unfilled rectangle produces unfilled rectangle
    createSdImpressDoc();
    auto pImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pImpressDocument);

    uno::Sequence<beans::PropertyValue> aArgs(
        comphelper::InitPropertySequence({ { "KeyModifier", uno::Any(KEY_MOD1) } }));
    dispatchCommand(mxComponent, ".uno:Rect_Unfilled", aArgs);
    Scheduler::ProcessEventsToIdle();

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xDrawPage->getCount());

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

CPPUNIT_TEST_FIXTURE(SdUiImpressTest, testSearchAllInDocumentAndNotes)
{
    // tdf#142478
    // "find all" produces a crash when the search string exists in notes
    // and the document

    createSdImpressDoc("odp/search-all-notes.odp");

    auto pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    CPPUNIT_ASSERT(pViewShell);

    lcl_search("Crash", /*bFindAll=*/true, /*bBackwards=*/true);
}

#if !defined(_WIN32) && !defined(MACOSX)
// for some reason, the search for "second" (or "Second") didn't return page 2 in WIN and MACOS
CPPUNIT_TEST_FIXTURE(SdUiImpressTest, testTdf123658_SearchAfterSlideChange)
{
    createSdImpressDoc("odp/tdf123658_SearchAfterSlideChange.odp");

    auto pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());

    lcl_search("second");
    checkCurrentPageNumber(2);

    pXImpressDocument->setPart(0); // Switch to 1st page

    lcl_search("of");
    // Instead of finding this on the 1st page (or on the 2nd page would be acceptable too)
    // it was going to the third page.
    checkCurrentPageNumber(1);
}
#endif

CPPUNIT_TEST_FIXTURE(SdUiImpressTest, testTdf142589)
{
    createSdImpressDoc();

    auto pImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    sd::ViewShell* pViewShell = pImpressDocument->GetDocShell()->GetViewShell();

    SfxRequest aRequest(pViewShell->GetViewFrame(), SID_PRESENTATION);
    pImpressDocument->GetDoc()->getPresentationSettings().mbCustomShow = true;
    pImpressDocument->GetDoc()->getPresentationSettings().mbStartCustomShow = true;
    sd::slideshowhelp::ShowSlideShow(aRequest, *pImpressDocument->GetDoc());
    CPPUNIT_ASSERT_EQUAL(false, pImpressDocument->GetDoc()->getPresentationSettings().mbCustomShow);
}

CPPUNIT_TEST_FIXTURE(SdUiImpressTest, testCharColorTheme)
{
    // Given an Impress document with a shape, with its text selected:
    createSdImpressDoc();
    uno::Reference<drawing::XDrawPagesSupplier> xPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xPage(xPagesSupplier->getDrawPages()->getByIndex(0),
                                             uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xShape(xPage->getByIndex(0), uno::UNO_QUERY);
    {
        uno::Reference<text::XSimpleText> xText = xShape->getText();
        xText->insertString(xText->getStart(), u"test", false);
    }
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<view::XSelectionSupplier> xController(xModel->getCurrentController(),
                                                         uno::UNO_QUERY);
    xController->select(uno::Any(xShape));
    Scheduler::ProcessEventsToIdle();
    dispatchCommand(mxComponent, ".uno:Text", {});
    Scheduler::ProcessEventsToIdle();
    auto pImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    sd::ViewShell* pViewShell = pImpressDocument->GetDocShell()->GetViewShell();
    SdrView* pView = pViewShell->GetView();
    CPPUNIT_ASSERT(pView->IsTextEdit());
    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    Scheduler::ProcessEventsToIdle();

    // When picking a theme color on the sidebar:
    uno::Sequence<beans::PropertyValue> aColorArgs = {
        comphelper::makePropertyValue("Color", static_cast<sal_Int32>(0xdae3f3)), // 80% light blue
        comphelper::makePropertyValue("ColorThemeIndex", static_cast<sal_Int16>(4)), // accent 1
        comphelper::makePropertyValue("ColorLumMod", static_cast<sal_Int16>(2000)),
        comphelper::makePropertyValue("ColorLumOff", static_cast<sal_Int16>(8000)),
    };
    dispatchCommand(mxComponent, ".uno:Color", aColorArgs);
    Scheduler::ProcessEventsToIdle();

    // Then make sure the theme "metadata" is set in the document model:
    pView->EndTextEditCurrentView();
    CPPUNIT_ASSERT(!pView->IsTextEdit());
    uno::Reference<container::XEnumerationAccess> xShapeParaAccess(xShape, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xPara(
        xShapeParaAccess->createEnumeration()->nextElement(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPortion(xPara->createEnumeration()->nextElement(),
                                                 uno::UNO_QUERY);
    sal_Int16 nCharColorTheme{};
    xPortion->getPropertyValue("CharColorTheme") >>= nCharColorTheme;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 4
    // - Actual  : -1
    // i.e. the theme index (accent1) was not set.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(4), nCharColorTheme);
    sal_Int16 nCharColorLumMod{};
    xPortion->getPropertyValue("CharColorLumMod") >>= nCharColorLumMod;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(2000), nCharColorLumMod);
    sal_Int16 nCharColorLumOff{};
    xPortion->getPropertyValue("CharColorLumOff") >>= nCharColorLumOff;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(8000), nCharColorLumOff);
}

CPPUNIT_TEST_FIXTURE(SdUiImpressTest, testFillColorTheme)
{
    // Given an Impress document with a selected shape:
    createSdImpressDoc();
    uno::Reference<drawing::XDrawPagesSupplier> xPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xPage(xPagesSupplier->getDrawPages()->getByIndex(0),
                                             uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xShape(xPage->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<view::XSelectionSupplier> xController(xModel->getCurrentController(),
                                                         uno::UNO_QUERY);
    xController->select(uno::Any(xShape));

    // When setting the fill color of that shape, with theme metadata & effects:
    uno::Sequence<beans::PropertyValue> aColorArgs = {
        comphelper::makePropertyValue("FillColor", static_cast<sal_Int32>(0xed7d31)), // orange
        comphelper::makePropertyValue("ColorThemeIndex", static_cast<sal_Int16>(4)), // accent 1
        comphelper::makePropertyValue("ColorLumMod", static_cast<sal_Int16>(4000)),
        comphelper::makePropertyValue("ColorLumOff", static_cast<sal_Int16>(6000)),
    };
    dispatchCommand(mxComponent, ".uno:FillColor", aColorArgs);
    Scheduler::ProcessEventsToIdle();

    // Then make sure the theme index is not lost when the sidebar sets it:
    sal_Int16 nFillColorTheme{};
    xShape->getPropertyValue("FillColorTheme") >>= nFillColorTheme;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 4
    // - Actual  : -1
    // i.e. the theme index was lost during the dispatch of the command.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(4), nFillColorTheme);

    // Then also verify the effects:
    sal_Int16 nFillColorLumMod = 10000;
    xShape->getPropertyValue("FillColorLumMod") >>= nFillColorLumMod;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 4000
    // - Actual  : 10000
    // i.e. the theme index was set, but not the effects.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(4000), nFillColorLumMod);
    sal_Int16 nFillColorLumOff = 0;
    xShape->getPropertyValue("FillColorLumOff") >>= nFillColorLumOff;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(6000), nFillColorLumOff);
}

CPPUNIT_TEST_FIXTURE(SdUiImpressTest, testFillColorNoColor)
{
    // Given an empty Impress document:
    createSdImpressDoc();
    auto pImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    sd::ViewShell* pViewShell = pImpressDocument->GetDocShell()->GetViewShell();
    SfxDispatcher* pDispatcher = pViewShell->GetViewFrame()->GetDispatcher();

    // When dispatching a fill color that only has a fill style (no color), then make sure we don't
    // crash:
    XFillStyleItem aXFillStyleItem(drawing::FillStyle_NONE);
    pDispatcher->ExecuteList(SID_ATTR_FILL_COLOR, SfxCallMode::RECORD, { &aXFillStyleItem });
}

CPPUNIT_TEST_FIXTURE(SdUiImpressTest, testTdf127696)
{
    createSdImpressDoc();

    dispatchCommand(mxComponent, ".uno:InsertPage", {});
    Scheduler::ProcessEventsToIdle();

    insertStringToObject(0, u"Test", /*bUseEscape*/ false);
    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:OutlineFont", {});

    // Save it as PPTX and load it again.
    saveAndReload("Impress Office Open XML");

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(1),
                                                 uno::UNO_QUERY);

    uno::Reference<beans::XPropertySet> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);

    uno::Reference<text::XText> xText
        = uno::Reference<text::XTextRange>(xShape, uno::UNO_QUERY_THROW)->getText();
    CPPUNIT_ASSERT_MESSAGE("Not a text shape", xText.is());

    uno::Reference<container::XEnumerationAccess> paraEnumAccess(xText, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> paraEnum(paraEnumAccess->createEnumeration());

    uno::Reference<text::XTextRange> xParagraph(paraEnum->nextElement(), uno::UNO_QUERY_THROW);

    uno::Reference<container::XEnumerationAccess> runEnumAccess(xParagraph, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> runEnum = runEnumAccess->createEnumeration();

    uno::Reference<text::XTextRange> xRun(runEnum->nextElement(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropSet(xRun, uno::UNO_QUERY_THROW);

    bool bContoured = false;
    xPropSet->getPropertyValue("CharContoured") >>= bContoured;
    CPPUNIT_ASSERT(bContoured);
}

CPPUNIT_TEST_FIXTURE(SdUiImpressTest, testThemeShapeInsert)
{
    // Given a document with a theme, accent1 color is set to 0x000004:
    createSdImpressDoc();
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    uno::Reference<drawing::XMasterPageTarget> xMasterPageTarget(xDrawPage, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xMasterPage(xMasterPageTarget->getMasterPage(),
                                                    uno::UNO_QUERY);
    comphelper::SequenceAsHashMap aMap;
    aMap["Name"] <<= OUString("mytheme");
    aMap["ColorSchemeName"] <<= OUString("mycolorscheme");
    uno::Sequence<util::Color> aColorScheme
        = { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb };
    aMap["ColorScheme"] <<= aColorScheme;
    uno::Any aTheme(aMap.getAsConstPropertyValueList());
    xMasterPage->setPropertyValue("Theme", aTheme);

    // When inserting a shape:
    uno::Sequence<beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue("CreateDirectly", true),
    };
    dispatchCommand(mxComponent, ".uno:BasicShapes.round-rectangle", aArgs);

    // Then make sure the that fill color of the last shape is the accent1 color:
    sal_Int32 nShapeIndex = xDrawPage->getCount() - 1;
    uno::Reference<beans::XPropertySet> xShape(xDrawPage->getByIndex(nShapeIndex), uno::UNO_QUERY);
    sal_Int32 nFillColor{};
    xShape->getPropertyValue("FillColor") >>= nFillColor;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0 / 0x000004 (~black)
    // - Actual  : 7512015 / 0x729fcf (~blue)
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0x4), nFillColor);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
