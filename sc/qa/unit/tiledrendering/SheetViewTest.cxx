/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sctiledrenderingtest.hxx>

#include <comphelper/propertyvalue.hxx>
#include <comphelper/propertysequence.hxx>
#include <sfx2/lokhelper.hxx>
#include <vcl/scheduler.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <sctestviewcallback.hxx>
#include <docuno.hxx>
#include <SheetView.hxx>
#include <SheetViewManager.hxx>

using namespace css;

class SheetViewTest : public ScTiledRenderingTest
{
};

/** Check auto-filter sorting.
 *
 * Auto filter sorting should only influence the values in the
 * current sheet view and not in the other views.
 */
CPPUNIT_TEST_FIXTURE(SheetViewTest, testSheetViewAutoFilter)
{
    // Create two views, and leave the second one current.
    ScModelObj* pModelObj = createDoc("SheetView_AutoFilter.ods");
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());

    // Setup 2 views
    ScTestViewCallback aView1;
    ScTabViewShell* pTabView1 = aView1.getTabViewShell();

    SfxLokHelper::createView();
    Scheduler::ProcessEventsToIdle();

    ScTestViewCallback aView2;
    ScTabViewShell* pTabView2 = aView2.getTabViewShell();

    CPPUNIT_ASSERT(pTabView1 != pTabView2);
    CPPUNIT_ASSERT(aView1.getViewID() != aView2.getViewID());

    // Switch to view 1
    SfxLokHelper::setView(aView1.getViewID());
    Scheduler::ProcessEventsToIdle();

    // Check AutoFilter values
    CPPUNIT_ASSERT_EQUAL(u"4"_ustr, pTabView1->GetCurrentString(0, 1));
    CPPUNIT_ASSERT_EQUAL(u"5"_ustr, pTabView1->GetCurrentString(0, 2));
    CPPUNIT_ASSERT_EQUAL(u"3"_ustr, pTabView1->GetCurrentString(0, 3));
    CPPUNIT_ASSERT_EQUAL(u"7"_ustr, pTabView1->GetCurrentString(0, 4));

    // Switch to view 2
    SfxLokHelper::setView(aView2.getViewID());
    Scheduler::ProcessEventsToIdle();

    // Check auto-filter values
    CPPUNIT_ASSERT_EQUAL(u"4"_ustr, pTabView2->GetCurrentString(0, 1));
    CPPUNIT_ASSERT_EQUAL(u"5"_ustr, pTabView2->GetCurrentString(0, 2));
    CPPUNIT_ASSERT_EQUAL(u"3"_ustr, pTabView2->GetCurrentString(0, 3));
    CPPUNIT_ASSERT_EQUAL(u"7"_ustr, pTabView2->GetCurrentString(0, 4));

    // Check what sheet we currently have selected for view 1 & 2
    CPPUNIT_ASSERT_EQUAL(SCTAB(0), pTabView1->GetViewData().GetTabNumber());
    CPPUNIT_ASSERT_EQUAL(SCTAB(0), pTabView2->GetViewData().GetTabNumber());

    // Create a new sheet view for view 2
    dispatchCommand(mxComponent, u".uno:NewSheetView"_ustr, {});
    Scheduler::ProcessEventsToIdle();

    // Check what sheet we currently have selected for view 1 & 2
    CPPUNIT_ASSERT_EQUAL(SCTAB(0), pTabView1->GetViewData().GetTabNumber());
    CPPUNIT_ASSERT_EQUAL(SCTAB(0), pTabView2->GetViewData().GetTabNumber());

    // Sort AutoFilter descending
    dispatchCommand(mxComponent, u".uno:SortDescending"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(SCTAB(0), pTabView1->GetViewData().GetTabNumber());
    CPPUNIT_ASSERT_EQUAL(SCTAB(0), pTabView2->GetViewData().GetTabNumber());

    // Check view 2 - sorted
    CPPUNIT_ASSERT_EQUAL(u"7"_ustr, pTabView2->GetCurrentString(0, 1));
    CPPUNIT_ASSERT_EQUAL(u"5"_ustr, pTabView2->GetCurrentString(0, 2));
    CPPUNIT_ASSERT_EQUAL(u"4"_ustr, pTabView2->GetCurrentString(0, 3));
    CPPUNIT_ASSERT_EQUAL(u"3"_ustr, pTabView2->GetCurrentString(0, 4));

    // Check view 1 - unsorted
    CPPUNIT_ASSERT_EQUAL(u"4"_ustr, pTabView1->GetCurrentString(0, 1));
    CPPUNIT_ASSERT_EQUAL(u"5"_ustr, pTabView1->GetCurrentString(0, 2));
    CPPUNIT_ASSERT_EQUAL(u"3"_ustr, pTabView1->GetCurrentString(0, 3));
    CPPUNIT_ASSERT_EQUAL(u"7"_ustr, pTabView1->GetCurrentString(0, 4));
}

CPPUNIT_TEST_FIXTURE(SheetViewTest, testSyncValuesBetweenMainSheetAndSheetView)
{
    // Create two views, and leave the second one current.
    ScModelObj* pModelObj = createDoc("empty.ods");
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ScDocument* pDocument = pModelObj->GetDocument();

    // Setup 2 views
    ScTestViewCallback aView1;
    ScTabViewShell* pTabView1 = aView1.getTabViewShell();
    SfxLokHelper::createView();
    Scheduler::ProcessEventsToIdle();
    ScTestViewCallback aView2;
    ScTabViewShell* pTabView2 = aView2.getTabViewShell();
    CPPUNIT_ASSERT(pTabView1 != pTabView2);
    CPPUNIT_ASSERT(aView1.getViewID() != aView2.getViewID());

    // Setup data
    // String in A1, and a formula in A2
    ScAddress aA1(0, 0, 0);
    ScAddress aA1SheetView(0, 0, 1);

    ScAddress aA2(0, 1, 0);
    ScAddress aA2SheetView(0, 1, 1);

    typeCharsInCell(std::string("ABCD"), aA1.Col(), aA1.Row(), pTabView1, pModelObj);
    typeCharsInCell(std::string("=UPPER(\"A\"&\"b\"&\"C\"&\"d\")"), aA2.Col(), aA2.Row(), pTabView1,
                    pModelObj);

    CPPUNIT_ASSERT_EQUAL(u"ABCD"_ustr, pDocument->GetString(aA1));
    CPPUNIT_ASSERT_EQUAL(u"ABCD"_ustr, pDocument->GetString(aA2));

    // Check what the View1 and View2 sees
    CPPUNIT_ASSERT_EQUAL(u"ABCD"_ustr, pTabView1->GetCurrentString(aA1.Col(), aA1.Row()));
    CPPUNIT_ASSERT_EQUAL(u"ABCD"_ustr, pTabView2->GetCurrentString(aA1.Col(), aA1.Row()));

    CPPUNIT_ASSERT_EQUAL(u"ABCD"_ustr, pTabView1->GetCurrentString(aA2.Col(), aA2.Row()));
    CPPUNIT_ASSERT_EQUAL(u"ABCD"_ustr, pTabView2->GetCurrentString(aA2.Col(), aA2.Row()));

    // Create a sheet view in View2
    SfxLokHelper::setView(aView2.getViewID());
    Scheduler::ProcessEventsToIdle();
    dispatchCommand(mxComponent, u".uno:NewSheetView"_ustr, {});

    // Change content in View1 with default view -> default view ro sheet view sync
    SfxLokHelper::setView(aView1.getViewID());
    typeCharsInCell(std::string("XYZ"), aA1.Col(), aA1.Row(), pTabView1, pModelObj);
    typeCharsInCell(std::string("=UPPER(\"x\"&\"Y\"&\"z\""), aA2.Col(), aA2.Row(), pTabView1,
                    pModelObj);

    // Check the content is synced
    CPPUNIT_ASSERT_EQUAL(u"XYZ"_ustr, pTabView1->GetCurrentString(aA1.Col(), aA1.Row()));
    CPPUNIT_ASSERT_EQUAL(u"XYZ"_ustr, pTabView2->GetCurrentString(aA1.Col(), aA1.Row()));

    CPPUNIT_ASSERT_EQUAL(u"XYZ"_ustr, pTabView1->GetCurrentString(aA2.Col(), aA2.Row()));
    CPPUNIT_ASSERT_EQUAL(u"XYZ"_ustr, pTabView2->GetCurrentString(aA2.Col(), aA2.Row()));

    // Check the content directly in sheets
    CPPUNIT_ASSERT_EQUAL(u"XYZ"_ustr, pDocument->GetString(aA1));
    CPPUNIT_ASSERT_EQUAL(u"XYZ"_ustr, pDocument->GetString(aA1SheetView));

    CPPUNIT_ASSERT_EQUAL(u"XYZ"_ustr, pDocument->GetString(aA2));
    CPPUNIT_ASSERT_EQUAL(u"XYZ"_ustr, pDocument->GetString(aA2SheetView));

    // Change content in the View2 with the sheet view -> sheet view to default view sync
    SfxLokHelper::setView(aView2.getViewID());
    Scheduler::ProcessEventsToIdle();

    typeCharsInCell(std::string("ABC123"), aA1.Col(), aA1.Row(), pTabView2, pModelObj);
    typeCharsInCell(std::string("=UPPER(\"aBc\"&\"123\""), aA2.Col(), aA2.Row(), pTabView2,
                    pModelObj);

    // Check the content is synced
    CPPUNIT_ASSERT_EQUAL(u"ABC123"_ustr, pTabView1->GetCurrentString(aA1.Col(), aA1.Row()));
    CPPUNIT_ASSERT_EQUAL(u"ABC123"_ustr, pTabView2->GetCurrentString(aA1.Col(), aA1.Row()));

    CPPUNIT_ASSERT_EQUAL(u"ABC123"_ustr, pTabView1->GetCurrentString(aA2.Col(), aA2.Row()));
    CPPUNIT_ASSERT_EQUAL(u"ABC123"_ustr, pTabView2->GetCurrentString(aA2.Col(), aA2.Row()));

    // Check the content directly in sheets
    CPPUNIT_ASSERT_EQUAL(u"ABC123"_ustr, pDocument->GetString(aA1));
    CPPUNIT_ASSERT_EQUAL(u"ABC123"_ustr, pDocument->GetString(aA1SheetView));

    CPPUNIT_ASSERT_EQUAL(u"ABC123"_ustr, pDocument->GetString(aA2));
    CPPUNIT_ASSERT_EQUAL(u"ABC123"_ustr, pDocument->GetString(aA2SheetView));
}

CPPUNIT_TEST_FIXTURE(SheetViewTest, testRemoveSheetView)
{
    // Create two views, and leave the second one current.
    ScModelObj* pModelObj = createDoc("SheetView_AutoFilter.ods");
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ScDocument* pDocument = pModelObj->GetDocument();

    // Setup views
    ScTestViewCallback aView1;
    ScTabViewShell* pTabView1 = aView1.getTabViewShell();
    SfxLokHelper::createView();
    Scheduler::ProcessEventsToIdle();
    ScTestViewCallback aView2;
    ScTabViewShell* pTabView2 = aView2.getTabViewShell();
    CPPUNIT_ASSERT(pTabView1 != pTabView2);
    CPPUNIT_ASSERT(aView1.getViewID() != aView2.getViewID());

    // Switch to View1
    SfxLokHelper::setView(aView1.getViewID());
    Scheduler::ProcessEventsToIdle();

    // Create a new sheet view for view 1
    dispatchCommand(mxComponent, u".uno:NewSheetView"_ustr, {});
    Scheduler::ProcessEventsToIdle();

    // Check AutoFilter values for each view
    CPPUNIT_ASSERT_EQUAL(u"4"_ustr, pTabView1->GetCurrentString(0, 1));
    CPPUNIT_ASSERT_EQUAL(u"5"_ustr, pTabView1->GetCurrentString(0, 2));
    CPPUNIT_ASSERT_EQUAL(u"3"_ustr, pTabView1->GetCurrentString(0, 3));
    CPPUNIT_ASSERT_EQUAL(u"7"_ustr, pTabView1->GetCurrentString(0, 4));

    CPPUNIT_ASSERT_EQUAL(u"4"_ustr, pTabView2->GetCurrentString(0, 1));
    CPPUNIT_ASSERT_EQUAL(u"5"_ustr, pTabView2->GetCurrentString(0, 2));
    CPPUNIT_ASSERT_EQUAL(u"3"_ustr, pTabView2->GetCurrentString(0, 3));
    CPPUNIT_ASSERT_EQUAL(u"7"_ustr, pTabView2->GetCurrentString(0, 4));

    // Switch to View2
    SfxLokHelper::setView(aView2.getViewID());
    Scheduler::ProcessEventsToIdle();

    // Sort AutoFilter descending
    dispatchCommand(mxComponent, u".uno:SortDescending"_ustr, {});
    Scheduler::ProcessEventsToIdle();

    // Check values are sorted for view 2
    CPPUNIT_ASSERT_EQUAL(u"7"_ustr, pTabView2->GetCurrentString(0, 1));
    CPPUNIT_ASSERT_EQUAL(u"5"_ustr, pTabView2->GetCurrentString(0, 2));
    CPPUNIT_ASSERT_EQUAL(u"4"_ustr, pTabView2->GetCurrentString(0, 3));
    CPPUNIT_ASSERT_EQUAL(u"3"_ustr, pTabView2->GetCurrentString(0, 4));

    // Sheet view must be present
    auto pSheetViewManager = pDocument->GetSheetViewManager(0);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pSheetViewManager->getSheetViews().size());

    // There should be 2 tables
    CPPUNIT_ASSERT_EQUAL(SCTAB(2), pDocument->GetTableCount());

    // Switch to View1
    SfxLokHelper::setView(aView1.getViewID());
    Scheduler::ProcessEventsToIdle();

    // We remove the current sheet view
    dispatchCommand(mxComponent, u".uno:RemoveSheetView"_ustr, {});
    Scheduler::ProcessEventsToIdle();

    // Sheet view is retained, but null
    CPPUNIT_ASSERT_EQUAL(size_t(1), pSheetViewManager->getSheetViews().size());
    CPPUNIT_ASSERT(!pSheetViewManager->getSheetViews().at(0));

    // Only 1 table left
    CPPUNIT_ASSERT_EQUAL(SCTAB(1), pDocument->GetTableCount());
}

CPPUNIT_TEST_FIXTURE(SheetViewTest, testSheetViewOperationRestrictions_DefaultViewChanged)
{
    // Create two views, and leave the second one current.
    ScModelObj* pModelObj = createDoc("SheetView_AutoFilter.ods");
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ScDocument* pDocument = pModelObj->GetDocument();

    // Setup views
    // View 1
    ScTestViewCallback aView1;
    ScTabViewShell* pTabView1 = aView1.getTabViewShell();

    // View 2
    SfxLokHelper::createView();
    Scheduler::ProcessEventsToIdle();
    ScTestViewCallback aView2;
    ScTabViewShell* pTabView2 = aView2.getTabViewShell();

    // View 3
    SfxLokHelper::createView();
    Scheduler::ProcessEventsToIdle();
    ScTestViewCallback aView3;
    ScTabViewShell* pTabView3 = aView3.getTabViewShell();

    // Create a new sheet view for view 1
    SfxLokHelper::setView(aView1.getViewID());
    Scheduler::ProcessEventsToIdle();
    dispatchCommand(mxComponent, u".uno:NewSheetView"_ustr, {});
    Scheduler::ProcessEventsToIdle();

    // Create a new sheet view for view 3
    SfxLokHelper::setView(aView3.getViewID());
    Scheduler::ProcessEventsToIdle();
    dispatchCommand(mxComponent, u".uno:NewSheetView"_ustr, {});
    Scheduler::ProcessEventsToIdle();

    // Check AutoFilter values for each view
    CPPUNIT_ASSERT_EQUAL(u"4"_ustr, pTabView1->GetCurrentString(0, 1));
    CPPUNIT_ASSERT_EQUAL(u"5"_ustr, pTabView1->GetCurrentString(0, 2));
    CPPUNIT_ASSERT_EQUAL(u"3"_ustr, pTabView1->GetCurrentString(0, 3));
    CPPUNIT_ASSERT_EQUAL(u"7"_ustr, pTabView1->GetCurrentString(0, 4));

    CPPUNIT_ASSERT_EQUAL(u"4"_ustr, pTabView2->GetCurrentString(0, 1));
    CPPUNIT_ASSERT_EQUAL(u"5"_ustr, pTabView2->GetCurrentString(0, 2));
    CPPUNIT_ASSERT_EQUAL(u"3"_ustr, pTabView2->GetCurrentString(0, 3));
    CPPUNIT_ASSERT_EQUAL(u"7"_ustr, pTabView2->GetCurrentString(0, 4));

    CPPUNIT_ASSERT_EQUAL(u"4"_ustr, pTabView3->GetCurrentString(0, 1));
    CPPUNIT_ASSERT_EQUAL(u"5"_ustr, pTabView3->GetCurrentString(0, 2));
    CPPUNIT_ASSERT_EQUAL(u"3"_ustr, pTabView3->GetCurrentString(0, 3));
    CPPUNIT_ASSERT_EQUAL(u"7"_ustr, pTabView3->GetCurrentString(0, 4));

    // Switch to View2
    SfxLokHelper::setView(aView2.getViewID());
    Scheduler::ProcessEventsToIdle();

    // Sheet view must be present
    auto pSheetViewManager = pDocument->GetSheetViewManager(SCTAB(0));
    CPPUNIT_ASSERT_EQUAL(size_t(2), pSheetViewManager->getSheetViews().size());

    auto pSheetView1 = pSheetViewManager->get(0);
    CPPUNIT_ASSERT_EQUAL(true, pSheetView1->isSynced());
    auto pSheetView2 = pSheetViewManager->get(1);
    CPPUNIT_ASSERT_EQUAL(true, pSheetView2->isSynced());

    // Sort, which will unsync sheet views
    pTabView2->SetCursor(0, 0);
    dispatchCommand(mxComponent, u".uno:SortDescending"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(false, pSheetView1->isSynced());
    CPPUNIT_ASSERT_EQUAL(false, pSheetView2->isSynced());
}

CPPUNIT_TEST_FIXTURE(SheetViewTest, testSheetViewOperationRestrictions_SheetViewChanged)
{
    // Create two views, and leave the second one current.
    ScModelObj* pModelObj = createDoc("SheetView_AutoFilter.ods");
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ScDocument* pDocument = pModelObj->GetDocument();

    // Setup views
    // View 1
    ScTestViewCallback aView1;
    ScTabViewShell* pTabView1 = aView1.getTabViewShell();

    // View 2
    SfxLokHelper::createView();
    Scheduler::ProcessEventsToIdle();
    ScTestViewCallback aView2;
    ScTabViewShell* pTabView2 = aView2.getTabViewShell();

    // View 3
    SfxLokHelper::createView();
    Scheduler::ProcessEventsToIdle();
    ScTestViewCallback aView3;
    ScTabViewShell* pTabView3 = aView3.getTabViewShell();

    // Create a new sheet view for view 1
    SfxLokHelper::setView(aView1.getViewID());
    Scheduler::ProcessEventsToIdle();
    dispatchCommand(mxComponent, u".uno:NewSheetView"_ustr, {});
    Scheduler::ProcessEventsToIdle();

    // Create a new sheet view for view 3
    SfxLokHelper::setView(aView3.getViewID());
    Scheduler::ProcessEventsToIdle();
    dispatchCommand(mxComponent, u".uno:NewSheetView"_ustr, {});
    Scheduler::ProcessEventsToIdle();

    // Check AutoFilter values for each view
    CPPUNIT_ASSERT_EQUAL(u"4"_ustr, pTabView1->GetCurrentString(0, 1));
    CPPUNIT_ASSERT_EQUAL(u"5"_ustr, pTabView1->GetCurrentString(0, 2));
    CPPUNIT_ASSERT_EQUAL(u"3"_ustr, pTabView1->GetCurrentString(0, 3));
    CPPUNIT_ASSERT_EQUAL(u"7"_ustr, pTabView1->GetCurrentString(0, 4));

    CPPUNIT_ASSERT_EQUAL(u"4"_ustr, pTabView2->GetCurrentString(0, 1));
    CPPUNIT_ASSERT_EQUAL(u"5"_ustr, pTabView2->GetCurrentString(0, 2));
    CPPUNIT_ASSERT_EQUAL(u"3"_ustr, pTabView2->GetCurrentString(0, 3));
    CPPUNIT_ASSERT_EQUAL(u"7"_ustr, pTabView2->GetCurrentString(0, 4));

    CPPUNIT_ASSERT_EQUAL(u"4"_ustr, pTabView3->GetCurrentString(0, 1));
    CPPUNIT_ASSERT_EQUAL(u"5"_ustr, pTabView3->GetCurrentString(0, 2));
    CPPUNIT_ASSERT_EQUAL(u"3"_ustr, pTabView3->GetCurrentString(0, 3));
    CPPUNIT_ASSERT_EQUAL(u"7"_ustr, pTabView3->GetCurrentString(0, 4));

    // Sheet view must be present
    auto pSheetViewManager = pDocument->GetSheetViewManager(SCTAB(0));
    CPPUNIT_ASSERT_EQUAL(size_t(2), pSheetViewManager->getSheetViews().size());

    auto pSheetView1 = pSheetViewManager->get(0);
    CPPUNIT_ASSERT_EQUAL(true, pSheetView1->isSynced());

    auto pSheetView2 = pSheetViewManager->get(1);
    CPPUNIT_ASSERT_EQUAL(true, pSheetView2->isSynced());

    // Sort, which will unsync sheet views
    SfxLokHelper::setView(aView1.getViewID());
    Scheduler::ProcessEventsToIdle();
    pTabView1->SetCursor(0, 0);
    dispatchCommand(mxComponent, u".uno:SortDescending"_ustr, {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(false, pSheetView1->isSynced());
    CPPUNIT_ASSERT_EQUAL(true, pSheetView2->isSynced());
}

CPPUNIT_TEST_FIXTURE(SheetViewTest, testSheetViewManager)
{
    sc::SheetViewID nNonExistingID = 99;

    // Empty
    sc::SheetViewManager maSheetViewManager;
    CPPUNIT_ASSERT_EQUAL(sc::DefaultSheetViewID,
                         maSheetViewManager.getNextSheetView(sc::DefaultSheetViewID));
    CPPUNIT_ASSERT_EQUAL(sc::DefaultSheetViewID,
                         maSheetViewManager.getNextSheetView(0)); // First non existing
    CPPUNIT_ASSERT_EQUAL(sc::DefaultSheetViewID,
                         maSheetViewManager.getNextSheetView(1)); // Next non existing
    CPPUNIT_ASSERT_EQUAL(sc::DefaultSheetViewID,
                         maSheetViewManager.getNextSheetView(nNonExistingID));
    CPPUNIT_ASSERT_EQUAL(sc::InvalidSheetViewID,
                         maSheetViewManager.getNextSheetView(sc::InvalidSheetViewID));
    CPPUNIT_ASSERT_EQUAL(sc::InvalidSheetViewID, maSheetViewManager.getNextSheetView(-99));

    CPPUNIT_ASSERT_EQUAL(sc::DefaultSheetViewID,
                         maSheetViewManager.getPreviousSheetView(sc::DefaultSheetViewID));
    CPPUNIT_ASSERT_EQUAL(sc::DefaultSheetViewID,
                         maSheetViewManager.getPreviousSheetView(0)); // First non existing
    CPPUNIT_ASSERT_EQUAL(sc::DefaultSheetViewID,
                         maSheetViewManager.getPreviousSheetView(1)); // Next non existing
    CPPUNIT_ASSERT_EQUAL(sc::DefaultSheetViewID,
                         maSheetViewManager.getPreviousSheetView(nNonExistingID));
    CPPUNIT_ASSERT_EQUAL(sc::InvalidSheetViewID,
                         maSheetViewManager.getPreviousSheetView(sc::InvalidSheetViewID));
    CPPUNIT_ASSERT_EQUAL(sc::InvalidSheetViewID, maSheetViewManager.getPreviousSheetView(-99));

    // Add ID 1
    auto nID1 = maSheetViewManager.create(nullptr);
    CPPUNIT_ASSERT(maSheetViewManager.get(nID1));

    CPPUNIT_ASSERT_EQUAL(nID1, maSheetViewManager.getNextSheetView(sc::DefaultSheetViewID));
    CPPUNIT_ASSERT_EQUAL(sc::DefaultSheetViewID, maSheetViewManager.getNextSheetView(nID1));
    CPPUNIT_ASSERT_EQUAL(sc::DefaultSheetViewID,
                         maSheetViewManager.getNextSheetView(nNonExistingID));
    CPPUNIT_ASSERT_EQUAL(sc::InvalidSheetViewID,
                         maSheetViewManager.getNextSheetView(sc::InvalidSheetViewID));
    CPPUNIT_ASSERT_EQUAL(sc::InvalidSheetViewID, maSheetViewManager.getNextSheetView(-99));

    CPPUNIT_ASSERT_EQUAL(sc::DefaultSheetViewID, maSheetViewManager.getPreviousSheetView(nID1));
    CPPUNIT_ASSERT_EQUAL(nID1, maSheetViewManager.getPreviousSheetView(sc::DefaultSheetViewID));
    CPPUNIT_ASSERT_EQUAL(sc::DefaultSheetViewID,
                         maSheetViewManager.getPreviousSheetView(nNonExistingID));
    CPPUNIT_ASSERT_EQUAL(sc::InvalidSheetViewID,
                         maSheetViewManager.getPreviousSheetView(sc::InvalidSheetViewID));
    CPPUNIT_ASSERT_EQUAL(sc::InvalidSheetViewID, maSheetViewManager.getPreviousSheetView(-99));

    // Add ID 2
    auto nID2 = maSheetViewManager.create(nullptr);
    CPPUNIT_ASSERT(maSheetViewManager.get(nID2));
    CPPUNIT_ASSERT_EQUAL(nID1, maSheetViewManager.getNextSheetView(sc::DefaultSheetViewID));
    CPPUNIT_ASSERT_EQUAL(nID2, maSheetViewManager.getNextSheetView(nID1));
    CPPUNIT_ASSERT_EQUAL(sc::DefaultSheetViewID, maSheetViewManager.getNextSheetView(nID2));
    CPPUNIT_ASSERT_EQUAL(sc::DefaultSheetViewID,
                         maSheetViewManager.getNextSheetView(nNonExistingID));
    CPPUNIT_ASSERT_EQUAL(sc::InvalidSheetViewID,
                         maSheetViewManager.getNextSheetView(sc::InvalidSheetViewID));
    CPPUNIT_ASSERT_EQUAL(sc::InvalidSheetViewID, maSheetViewManager.getNextSheetView(-99));

    CPPUNIT_ASSERT_EQUAL(nID2, maSheetViewManager.getPreviousSheetView(sc::DefaultSheetViewID));
    CPPUNIT_ASSERT_EQUAL(nID1, maSheetViewManager.getPreviousSheetView(nID2));
    CPPUNIT_ASSERT_EQUAL(sc::DefaultSheetViewID, maSheetViewManager.getPreviousSheetView(nID1));
    CPPUNIT_ASSERT_EQUAL(sc::DefaultSheetViewID,
                         maSheetViewManager.getPreviousSheetView(nNonExistingID));
    CPPUNIT_ASSERT_EQUAL(sc::InvalidSheetViewID,
                         maSheetViewManager.getPreviousSheetView(sc::InvalidSheetViewID));
    CPPUNIT_ASSERT_EQUAL(sc::InvalidSheetViewID, maSheetViewManager.getPreviousSheetView(-99));
}

CPPUNIT_TEST_FIXTURE(SheetViewTest, testCheckIfSheetViewIsSavedInDocument_ODF)
{
    // Check if sheet view holder table is saved into the ODF document
    ScModelObj* pModelObj = createDoc("SheetView_AutoFilter.ods");
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());

    dispatchCommand(mxComponent, u".uno:NewSheetView"_ustr, {});
    Scheduler::ProcessEventsToIdle();

    save(u"calc8"_ustr);

    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "//table:table", 1);
}

CPPUNIT_TEST_FIXTURE(SheetViewTest, testCheckIfSheetViewIsSavedInDocument_OOXML)
{
    // Check if sheet view holder table is saved into the OOXML document
    ScModelObj* pModelObj = createDoc("SheetView_AutoFilter.ods");
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());

    dispatchCommand(mxComponent, u".uno:NewSheetView"_ustr, {});
    Scheduler::ProcessEventsToIdle();

    save(u"Calc Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/workbook.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/x:workbook/x:sheets/x:sheet", 1);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
