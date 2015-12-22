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
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>

#include <comphelper/processfactory.hxx>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XModel2.hpp>

#include <vcl/scheduler.hxx>
#include <osl/thread.hxx>
#include <FactoryIds.hxx>
#include <sdmod.hxx>
#include <tools/shl.hxx>
#include <ImpressViewShellBase.hxx>
#include <SlideSorterViewShell.hxx>
#include <SlideSorter.hxx>
#include <controller/SlideSorterController.hxx>
#include <controller/SlsClipboard.hxx>
#include <controller/SlsPageSelector.hxx>

using namespace ::com::sun::star;

/// Impress miscellaneous tests.
class SdMiscTest : public SdModelTestBase
{
public:
    void testTdf96206();
    void testTdf96708();

    CPPUNIT_TEST_SUITE(SdMiscTest);
    CPPUNIT_TEST(testTdf96206);
    CPPUNIT_TEST(testTdf96708);
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
        while (Scheduler::ProcessTaskScheduling(true));
        if ((pSSVS = sd::slidesorter::SlideSorterViewShell::GetSlideSorter(pViewShell->GetViewShellBase())) != nullptr)
            break;
        TimeValue aSleep(0, 100 * 1000000); // 100 msec
        osl::Thread::wait(aSleep);
    }
    CPPUNIT_ASSERT(pSSVS);

    return xDocSh;
}

void SdMiscTest::testTdf96206()
{
    // Copying/pasting slide referring to a non-default master with a text duplicated the master

    sd::DrawDocShellRef xDocSh = Load(getURLFromSrc("/sd/qa/unit/data/odp/tdf96206.odp"), ODP);
    sd::ViewShell *pViewShell = xDocSh->GetViewShell();
    auto pSSVS = sd::slidesorter::SlideSorterViewShell::GetSlideSorter(pViewShell->GetViewShellBase());
    auto& xSSController = pSSVS->GetSlideSorter().GetController();

    const sal_uInt16 nMasterPageCnt1 = xDocSh->GetDoc()->GetMasterSdPageCount(PageKind::PK_STANDARD);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(2), nMasterPageCnt1);
    xSSController.GetClipboard().DoCopy();
    xSSController.GetClipboard().DoPaste();
    const sal_uInt16 nMasterPageCnt2 = xDocSh->GetDoc()->GetMasterSdPageCount(PageKind::PK_STANDARD);
    CPPUNIT_ASSERT_EQUAL(nMasterPageCnt1, nMasterPageCnt2);

    xDocSh->DoClose();
}

void SdMiscTest::testTdf96708()
{
    sd::DrawDocShellRef xDocSh = Load(getURLFromSrc("/sd/qa/unit/data/odp/tdf96708.odp"), ODP);
    sd::ViewShell *pViewShell = xDocSh->GetViewShell();
    auto pSSVS = sd::slidesorter::SlideSorterViewShell::GetSlideSorter(pViewShell->GetViewShellBase());
    auto& xSSController = pSSVS->GetSlideSorter().GetController();
    auto& xPageSelector = xSSController.GetPageSelector();

    const sal_uInt16 nMasterPageCnt1 = xDocSh->GetDoc()->GetMasterSdPageCount(PageKind::PK_STANDARD);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(4), nMasterPageCnt1);
    xPageSelector.SelectAllPages();
    xSSController.GetClipboard().DoCopy();

    // Now wait for timers to trigger creation of auto-layout
    TimeValue aSleep(0, 100 * 1000000); // 100 msec
    osl::Thread::wait(aSleep);
    Scheduler::ProcessTaskScheduling(true);

    xSSController.GetClipboard().DoPaste();
    const sal_uInt16 nMasterPageCnt2 = xDocSh->GetDoc()->GetMasterSdPageCount(PageKind::PK_STANDARD);
    CPPUNIT_ASSERT_EQUAL(nMasterPageCnt1, nMasterPageCnt2);

    xDocSh->DoClose();
}

CPPUNIT_TEST_SUITE_REGISTRATION(SdMiscTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
