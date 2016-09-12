/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sdmodeltestbase.hxx"

#include <vcl/svapp.hxx>
#include <sddll.hxx>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>

#include <comphelper/processfactory.hxx>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XModel2.hpp>

#include <vcl/scheduler.hxx>
#include <osl/thread.hxx>
#include <FactoryIds.hxx>
#include <sdmod.hxx>
#include <svx/sdr/table/tablecontroller.hxx>
#include <sfx2/request.hxx>
#include <svx/svxids.hrc>
#include <editeng/eeitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/editobj.hxx>
#include <ImpressViewShellBase.hxx>
#include <SlideSorterViewShell.hxx>
#include <SlideSorter.hxx>
#include <controller/SlideSorterController.hxx>
#include <controller/SlsClipboard.hxx>
#include <controller/SlsPageSelector.hxx>
#include <undo/undomanager.hxx>
#include <chrono>

using namespace ::com::sun::star;

/// Impress miscellaneous tests.
class SdMiscTest : public SdModelTestBase
{
public:
    void testTdf96206();
    void testTdf96708();
    void testTdf99396();
    void testTdf99396TextEdit();

    CPPUNIT_TEST_SUITE(SdMiscTest);
    CPPUNIT_TEST(testTdf96206);
    CPPUNIT_TEST(testTdf96708);
    CPPUNIT_TEST(testTdf99396);
    CPPUNIT_TEST(testTdf99396TextEdit);
    CPPUNIT_TEST_SUITE_END();

private:
    sd::DrawDocShellRef Load(const OUString& rURL, sal_Int32 nFormat);
};

sd::DrawDocShellRef SdMiscTest::Load(const OUString& rURL, sal_Int32 nFormat)
{
    uno::Reference< frame::XDesktop2 > xDesktop = frame::Desktop::create(::comphelper::getProcessComponentContext());
    CPPUNIT_ASSERT(xDesktop.is());

    // create a frame
    uno::Reference< frame::XFrame > xTargetFrame = xDesktop->findFrame("_blank", 0);
    CPPUNIT_ASSERT(xTargetFrame.is());

    // 1. Open the document
    sd::DrawDocShellRef xDocSh = loadURL(rURL, nFormat);
    CPPUNIT_ASSERT_MESSAGE("Failed to load file.", xDocSh.Is());

    uno::Reference< frame::XModel2 > xModel2(xDocSh->GetModel(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xModel2.is());

    uno::Reference< frame::XController2 > xController(xModel2->createDefaultViewController(xTargetFrame), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xController.is());

    // introduce model/view/controller to each other
    xController->attachModel(xModel2.get());
    xModel2->connectController(xController.get());
    xTargetFrame->setComponent(xController->getComponentWindow(), xController.get());
    xController->attachFrame(xTargetFrame);
    xModel2->setCurrentController(xController.get());

    sd::ViewShell *pViewShell = xDocSh->GetViewShell();
    CPPUNIT_ASSERT(pViewShell);
    sd::slidesorter::SlideSorterViewShell* pSSVS = nullptr;
    for (int i = 0; i < 1000; i++)
    {
        // Process all Tasks - slide sorter is created here
        while (Scheduler::ProcessTaskScheduling());
        if ((pSSVS = sd::slidesorter::SlideSorterViewShell::GetSlideSorter(pViewShell->GetViewShellBase())) != nullptr)
            break;
        osl::Thread::wait(std::chrono::milliseconds(100));
    }
    CPPUNIT_ASSERT(pSSVS);

    return xDocSh;
}

void SdMiscTest::testTdf96206()
{
    // Copying/pasting slide referring to a non-default master with a text duplicated the master

    sd::DrawDocShellRef xDocSh = Load(m_directories.getURLFromSrc("/sd/qa/unit/data/odp/tdf96206.odp"), ODP);
    sd::ViewShell *pViewShell = xDocSh->GetViewShell();
    auto pSSVS = sd::slidesorter::SlideSorterViewShell::GetSlideSorter(pViewShell->GetViewShellBase());
    auto& rSSController = pSSVS->GetSlideSorter().GetController();

    const sal_uInt16 nMasterPageCnt1 = xDocSh->GetDoc()->GetMasterSdPageCount(PageKind::PK_STANDARD);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(2), nMasterPageCnt1);
    rSSController.GetClipboard().DoCopy();
    rSSController.GetClipboard().DoPaste();
    const sal_uInt16 nMasterPageCnt2 = xDocSh->GetDoc()->GetMasterSdPageCount(PageKind::PK_STANDARD);
    CPPUNIT_ASSERT_EQUAL(nMasterPageCnt1, nMasterPageCnt2);

    xDocSh->DoClose();
}

void SdMiscTest::testTdf96708()
{
    sd::DrawDocShellRef xDocSh = Load(m_directories.getURLFromSrc("/sd/qa/unit/data/odp/tdf96708.odp"), ODP);
    sd::ViewShell *pViewShell = xDocSh->GetViewShell();
    auto pSSVS = sd::slidesorter::SlideSorterViewShell::GetSlideSorter(pViewShell->GetViewShellBase());
    auto& rSSController = pSSVS->GetSlideSorter().GetController();
    auto& rPageSelector = rSSController.GetPageSelector();

    const sal_uInt16 nMasterPageCnt1 = xDocSh->GetDoc()->GetMasterSdPageCount(PageKind::PK_STANDARD);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(4), nMasterPageCnt1);
    rPageSelector.SelectAllPages();
    rSSController.GetClipboard().DoCopy();

    // Now wait for timers to trigger creation of auto-layout
    osl::Thread::wait(std::chrono::milliseconds(100));
    Scheduler::ProcessTaskScheduling();

    rSSController.GetClipboard().DoPaste();
    const sal_uInt16 nMasterPageCnt2 = xDocSh->GetDoc()->GetMasterSdPageCount(PageKind::PK_STANDARD);
    CPPUNIT_ASSERT_EQUAL(nMasterPageCnt1, nMasterPageCnt2);

    xDocSh->DoClose();
}

void SdMiscTest::testTdf99396()
{
    // Load the document and select the table.
    sd::DrawDocShellRef xDocSh = Load(m_directories.getURLFromSrc("/sd/qa/unit/data/tdf99396.odp"), ODP);
    sd::ViewShell *pViewShell = xDocSh->GetViewShell();
    SdPage* pPage = pViewShell->GetActualPage();
    SdrObject* pObject = pPage->GetObj(0);
    SdrView* pView = pViewShell->GetView();
    pView->MarkObj(pObject, pView->GetSdrPageView());

    // Make sure that the undo stack is empty.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), xDocSh->GetDoc()->GetUndoManager()->GetUndoActionCount());

    // Set the vertical alignment of the cells to bottom.
    sdr::table::SvxTableController* pTableController = dynamic_cast<sdr::table::SvxTableController*>(pView->getSelectionController().get());
    CPPUNIT_ASSERT(pTableController);
    SfxRequest aRequest(pViewShell->GetViewFrame(), SID_TABLE_VERT_BOTTOM);
    pTableController->Execute(aRequest);
    // This was 0, it wasn't possible to undo a vertical alignment change.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), xDocSh->GetDoc()->GetUndoManager()->GetUndoActionCount());

    xDocSh->DoClose();
}

void SdMiscTest::testTdf99396TextEdit()
{
    // Load the document and select the table.
    sd::DrawDocShellRef xDocSh = Load(m_directories.getURLFromSrc("/sd/qa/unit/data/tdf99396.odp"), ODP);
    sd::ViewShell* pViewShell = xDocSh->GetViewShell();
    SdPage* pPage = pViewShell->GetActualPage();
    SdrObject* pObject = pPage->GetObj(0);
    auto pTableObject = dynamic_cast<sdr::table::SdrTableObj*>(pObject);
    CPPUNIT_ASSERT(pTableObject);
    SdrView* pView = pViewShell->GetView();
    pView->MarkObj(pObject, pView->GetSdrPageView());

    // Make sure that the undo stack is empty.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), xDocSh->GetDoc()->GetUndoManager()->GetUndoActionCount());

    // Set horizontal and vertical adjustment during text edit.
    pView->SdrBeginTextEdit(pObject);
    CPPUNIT_ASSERT(pView->GetTextEditObject());
    {
        SfxRequest aRequest(pViewShell->GetViewFrame(), SID_ATTR_PARA_ADJUST_RIGHT);
        SfxItemSet aEditAttr(xDocSh->GetDoc()->GetPool());
        pView->GetAttributes(aEditAttr);
        SfxItemSet aNewAttr(*(aEditAttr.GetPool()), aEditAttr.GetRanges());
        aNewAttr.Put(SvxAdjustItem(SVX_ADJUST_RIGHT, EE_PARA_JUST));
        aRequest.Done(aNewAttr);
        const SfxItemSet* pArgs = aRequest.GetArgs();
        pView->SetAttributes(*pArgs);
    }
    {
        auto pTableController = dynamic_cast<sdr::table::SvxTableController*>(pView->getSelectionController().get());
        CPPUNIT_ASSERT(pTableController);
        SfxRequest aRequest(pViewShell->GetViewFrame(), SID_TABLE_VERT_BOTTOM);
        pTableController->Execute(aRequest);
    }
    pView->SdrEndTextEdit();

    // Check that the result is what we expect.
    {
        uno::Reference<table::XTable> xTable = pTableObject->getTable();
        uno::Reference<beans::XPropertySet> xCell(xTable->getCellByPosition(0, 0), uno::UNO_QUERY);
        drawing::TextVerticalAdjust eAdjust = xCell->getPropertyValue("TextVerticalAdjust").get<drawing::TextVerticalAdjust>();
        CPPUNIT_ASSERT_EQUAL(drawing::TextVerticalAdjust_BOTTOM, eAdjust);
    }
    {
        const EditTextObject& rEdit = pTableObject->getText(0)->GetOutlinerParaObject()->GetTextObject();
        const SfxItemSet& rParaAttribs = rEdit.GetParaAttribs(0);
        auto pAdjust = static_cast<const SvxAdjustItem*>(rParaAttribs.GetItem(EE_PARA_JUST));
        CPPUNIT_ASSERT_EQUAL(SVX_ADJUST_RIGHT, pAdjust->GetAdjust());
    }

    // Now undo.
    xDocSh->GetUndoManager()->Undo();

    // Check again that the result is what we expect.
    {
        uno::Reference<table::XTable> xTable = pTableObject->getTable();
        uno::Reference<beans::XPropertySet> xCell(xTable->getCellByPosition(0, 0), uno::UNO_QUERY);
        drawing::TextVerticalAdjust eAdjust = xCell->getPropertyValue("TextVerticalAdjust").get<drawing::TextVerticalAdjust>();
        // This failed: Undo() did not change it from drawing::TextVerticalAdjust_BOTTOM.
        CPPUNIT_ASSERT_EQUAL(drawing::TextVerticalAdjust_TOP, eAdjust);
    }
    {
        const EditTextObject& rEdit = pTableObject->getText(0)->GetOutlinerParaObject()->GetTextObject();
        const SfxItemSet& rParaAttribs = rEdit.GetParaAttribs(0);
        auto pAdjust = static_cast<const SvxAdjustItem*>(rParaAttribs.GetItem(EE_PARA_JUST));
        CPPUNIT_ASSERT_EQUAL(SVX_ADJUST_CENTER, pAdjust->GetAdjust());
    }

    xDocSh->DoClose();
}

CPPUNIT_TEST_SUITE_REGISTRATION(SdMiscTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
