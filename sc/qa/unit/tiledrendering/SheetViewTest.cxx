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
protected:
    static OUString expectedValues(std::vector<std::u16string_view> const& rValues)
    {
        OUString aString;
        bool bFirst = true;
        for (auto const& rValue : rValues)
        {
            if (bFirst)
            {
                bFirst = false;
                aString = u"\""_ustr + rValue + u"\""_ustr;
            }
            else
                aString += u", \""_ustr + rValue + u"\""_ustr;
        }
        return aString;
    }

    static OUString getValues(ScTabViewShell* pTabView, SCCOL nCol, SCROW nStartRow, SCROW nEndRow)
    {
        OUString aString;

        size_t nSize = nEndRow - nStartRow + 1;

        bool bFirst = true;

        for (size_t nIndex = 0; nIndex < nSize; nIndex++)
        {
            OUString aValue = pTabView->GetCurrentString(nCol, nStartRow + nIndex);
            if (bFirst)
            {
                bFirst = false;
                aString = u"\""_ustr + aValue + u"\""_ustr;
            }
            else
                aString += u", \""_ustr + aValue + u"\""_ustr;
        }

        return aString;
    }

    static OUString getValues(ScDocument* pDocument, SCCOL nCol, SCROW nStartRow, SCROW nEndRow,
                              SCTAB nTab)
    {
        OUString aString;

        size_t nSize = nEndRow - nStartRow + 1;

        bool bFirst = true;

        for (size_t nIndex = 0; nIndex < nSize; nIndex++)
        {
            OUString aValue = pDocument->GetString({ nCol, SCROW(nStartRow + nIndex), nTab });
            if (bFirst)
            {
                bFirst = false;
                aString = u"\""_ustr + aValue + u"\""_ustr;
            }
            else
                aString += u", \""_ustr + aValue + u"\""_ustr;
        }

        return aString;
    }

    static OUString getTextWeight(ScDocument* pDocument, SCCOL nCol, SCROW nStartRow, SCROW nEndRow,
                                  SCTAB nTab)
    {
        OUString aString;

        size_t nSize = nEndRow - nStartRow + 1;

        bool bFirst = true;
        vcl::Font aFont;
        const ScPatternAttr* pPattern;
        for (size_t nIndex = 0; nIndex < nSize; nIndex++)
        {
            pPattern = pDocument->GetPattern(nCol, SCROW(nStartRow + nIndex), nTab);
            pPattern->fillFontOnly(aFont);
            OUString aWeight;

            switch (aFont.GetWeight())
            {
                case WEIGHT_NORMAL:
                    aWeight = "N";
                    break;
                case WEIGHT_BOLD:
                    aWeight = "B";
                    break;
                default:
                    aWeight = "?";
                    break;
            }

            if (bFirst)
            {
                bFirst = false;
                aString = u"\""_ustr + aWeight + u"\""_ustr;
            }
            else
                aString += u", \""_ustr + aWeight + u"\""_ustr;
        }

        return aString;
    }

    void gotoCell(std::u16string_view aCellAddress)
    {
        dispatchCommand(
            mxComponent, u".uno:GoToCell"_ustr,
            comphelper::InitPropertySequence({ { "ToPoint", uno::Any(OUString(aCellAddress)) } }));
    }

    void createNewSheetViewInCurrentView()
    {
        dispatchCommand(mxComponent, u".uno:NewSheetView"_ustr, {});
    }

    void removeSheetViewInCurrentView()
    {
        dispatchCommand(mxComponent, u".uno:RemoveSheetView"_ustr, {});
    }

    void sortAscendingForCell(std::u16string_view aCellAddress)
    {
        gotoCell(aCellAddress);
        dispatchCommand(mxComponent, u".uno:SortAscending"_ustr, {});
    }

    void sortDescendingForCell(std::u16string_view aCellAddress)
    {
        gotoCell(aCellAddress);
        dispatchCommand(mxComponent, u".uno:SortDescending"_ustr, {});
    }

    void setCellBold(std::u16string_view aCellAddress)
    {
        gotoCell(aCellAddress);
        dispatchCommand(mxComponent, u".uno:Bold"_ustr, {});
    }
};

/** Test class that contains methods commonly used for testing sync of sheet views. */
class SyncTest : public SheetViewTest
{
public:
    void tearDown() override
    {
        moSheetView = std::nullopt;
        moDefaultView = std::nullopt;
        SheetViewTest::tearDown();
    }

protected:
    std::optional<ScTestViewCallback> moSheetView;
    std::optional<ScTestViewCallback> moDefaultView;
    ScTabViewShell* mpTabViewSheetView = nullptr;
    ScTabViewShell* mpTabViewDefaultView = nullptr;

    void setupViews()
    {
        moSheetView.emplace();
        mpTabViewSheetView = moSheetView->getTabViewShell();

        SfxLokHelper::createView();
        Scheduler::ProcessEventsToIdle();

        moDefaultView.emplace();
        mpTabViewDefaultView = moDefaultView->getTabViewShell();

        CPPUNIT_ASSERT(mpTabViewSheetView != mpTabViewDefaultView);
        CPPUNIT_ASSERT(moSheetView->getViewID() != moDefaultView->getViewID());
    }

    void switchToSheetView()
    {
        SfxLokHelper::setView(moSheetView->getViewID());
        Scheduler::ProcessEventsToIdle();
    }

    void switchToDefaultView()
    {
        SfxLokHelper::setView(moDefaultView->getViewID());
        Scheduler::ProcessEventsToIdle();
    }
};

/** Check auto-filter sorting.
 *
 * Auto filter sorting should only influence the values in the
 * current sheet view and not in the other viewes.
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
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"3", u"7" }), getValues(pTabView1, 0, 1, 4));

    // Switch to view 2
    SfxLokHelper::setView(aView2.getViewID());
    Scheduler::ProcessEventsToIdle();

    // Check auto-filter values
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"3", u"7" }), getValues(pTabView2, 0, 1, 4));

    // Check what sheet we currently have selected for view 1 & 2
    CPPUNIT_ASSERT_EQUAL(SCTAB(0), pTabView1->GetViewData().GetTabNumber());
    CPPUNIT_ASSERT_EQUAL(SCTAB(0), pTabView2->GetViewData().GetTabNumber());

    // Create a new sheet view for view 2
    createNewSheetViewInCurrentView();

    // Check what sheet we currently have selected for view 1 & 2
    CPPUNIT_ASSERT_EQUAL(SCTAB(0), pTabView1->GetViewData().GetTabNumber());
    CPPUNIT_ASSERT_EQUAL(SCTAB(1), pTabView2->GetViewData().GetTabNumber());

    // Sort AutoFilter descending
    sortDescendingForCell(u"A1");

    CPPUNIT_ASSERT_EQUAL(SCTAB(0), pTabView1->GetViewData().GetTabNumber());
    CPPUNIT_ASSERT_EQUAL(SCTAB(1), pTabView2->GetViewData().GetTabNumber());

    // Check view 2 - sorted
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"7", u"5", u"4", u"3" }), getValues(pTabView2, 0, 1, 4));

    // Check view 1 - unsorted
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"3", u"7" }), getValues(pTabView1, 0, 1, 4));
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
    createNewSheetViewInCurrentView();

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
    createNewSheetViewInCurrentView();

    // Check AutoFilter values for each view
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"3", u"7" }), getValues(pTabView1, 0, 1, 4));
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"3", u"7" }), getValues(pTabView2, 0, 1, 4));

    // Switch to View2
    SfxLokHelper::setView(aView2.getViewID());
    Scheduler::ProcessEventsToIdle();

    // Sort AutoFilter descending
    sortDescendingForCell(u"A1");

    // Check values are sorted for view 2
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"3", u"7" }), getValues(pTabView1, 0, 1, 4));
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"7", u"5", u"4", u"3" }), getValues(pTabView2, 0, 1, 4));

    // Sheet view must be present
    auto pSheetViewManager = pDocument->GetSheetViewManager(0);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pSheetViewManager->size());

    // There should be 2 tables
    CPPUNIT_ASSERT_EQUAL(SCTAB(2), pDocument->GetTableCount());

    // Switch to View1
    SfxLokHelper::setView(aView1.getViewID());
    Scheduler::ProcessEventsToIdle();

    // We remove the current sheet view
    removeSheetViewInCurrentView();
    Scheduler::ProcessEventsToIdle();

    // Sheet view was removed
    CPPUNIT_ASSERT_EQUAL(size_t(0), pSheetViewManager->size());

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
    createNewSheetViewInCurrentView();

    // Create a new sheet view for view 3
    SfxLokHelper::setView(aView3.getViewID());
    Scheduler::ProcessEventsToIdle();
    createNewSheetViewInCurrentView();

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
    CPPUNIT_ASSERT_EQUAL(size_t(2), pSheetViewManager->size());

    auto pSheetView1 = pSheetViewManager->get(0);
    CPPUNIT_ASSERT_EQUAL(true, pSheetView1->isSynced());
    auto pSheetView2 = pSheetViewManager->get(1);
    CPPUNIT_ASSERT_EQUAL(true, pSheetView2->isSynced());

    // Sort, which will unsync sheet views
    sortDescendingForCell(u"A1");

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
    createNewSheetViewInCurrentView();

    // Create a new sheet view for view 3
    SfxLokHelper::setView(aView3.getViewID());
    Scheduler::ProcessEventsToIdle();
    createNewSheetViewInCurrentView();

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
    CPPUNIT_ASSERT_EQUAL(size_t(2), pSheetViewManager->size());

    auto pSheetView1 = pSheetViewManager->get(0);
    CPPUNIT_ASSERT_EQUAL(true, pSheetView1->isSynced());

    auto pSheetView2 = pSheetViewManager->get(1);
    CPPUNIT_ASSERT_EQUAL(true, pSheetView2->isSynced());

    // Sort, which will unsync sheet views
    SfxLokHelper::setView(aView1.getViewID());
    Scheduler::ProcessEventsToIdle();
    sortDescendingForCell(u"A1");

    CPPUNIT_ASSERT_EQUAL(false, pSheetView1->isSynced());
    CPPUNIT_ASSERT_EQUAL(true, pSheetView2->isSynced());
}

CPPUNIT_TEST_FIXTURE(SheetViewTest, testCheckIfSheetViewIsSavedInDocument_ODF)
{
    // Check if sheet view holder table is saved into the ODF document
    ScModelObj* pModelObj = createDoc("SheetView_AutoFilter.ods");
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());

    createNewSheetViewInCurrentView();

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

    createNewSheetViewInCurrentView();

    save(u"Calc Office Open XML"_ustr);

    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/workbook.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/x:workbook/x:sheets/x:sheet", 1);
}

CPPUNIT_TEST_FIXTURE(SheetViewTest, testRemoveTableWithSheetViews)
{
    // Create a new sheet (in addition to the existing one), and create 2 sheet views of the new sheet.
    // After that, delete the sheet and the 2 sheet view holder tables should also be deleted.

    ScModelObj* pModelObj = createDoc("empty.ods");
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ScDocument& rDocument = *pModelObj->GetDocument();

    // Setup views
    ScTestViewCallback aView1;
    ScTabViewShell* pTabView1 = aView1.getTabViewShell();

    // Insert a new sheet - "NewTab"
    uno::Sequence<beans::PropertyValue> aArgsInsert(comphelper::InitPropertySequence(
        { { "Name", uno::Any(u"NewTab"_ustr) }, { "Index", uno::Any(sal_Int16(1)) } }));

    dispatchCommand(mxComponent, u".uno:Insert"_ustr, aArgsInsert);

    {
        auto pSheetViewManager = rDocument.GetSheetViewManager(SCTAB(0));
        CPPUNIT_ASSERT(pSheetViewManager);
        CPPUNIT_ASSERT_EQUAL(size_t(0), pSheetViewManager->size());

        // Check we have the correct table selected
        CPPUNIT_ASSERT_EQUAL(SCTAB(2), rDocument.GetTableCount());
        CPPUNIT_ASSERT_EQUAL(u"NewTab"_ustr, rDocument.GetAllTableNames()[0]);
        CPPUNIT_ASSERT_EQUAL(u"Hoja1"_ustr, rDocument.GetAllTableNames()[1]);

        CPPUNIT_ASSERT_EQUAL(SCTAB(0), pTabView1->GetViewData().GetTabNumber());
    }

    // Create first sheet views
    createNewSheetViewInCurrentView();

    {
        auto pSheetViewManager = rDocument.GetSheetViewManager(SCTAB(0));
        CPPUNIT_ASSERT(pSheetViewManager);
        CPPUNIT_ASSERT_EQUAL(size_t(1), pSheetViewManager->size());

        // View is now on the sheet view tab
        CPPUNIT_ASSERT_EQUAL(SCTAB(1), pTabView1->GetViewData().GetTabNumber());
        auto pSheetView1 = pSheetViewManager->get(0);
        CPPUNIT_ASSERT_EQUAL(SCTAB(1), pSheetView1->getTableNumber());

        CPPUNIT_ASSERT_EQUAL(SCTAB(3), rDocument.GetTableCount());
        CPPUNIT_ASSERT_EQUAL(u"NewTab"_ustr, rDocument.GetAllTableNames()[0]);
        CPPUNIT_ASSERT_EQUAL(u"NewTab_2"_ustr, rDocument.GetAllTableNames()[1]);
        CPPUNIT_ASSERT_EQUAL(u"Hoja1"_ustr, rDocument.GetAllTableNames()[2]);
    }

    // Create second sheet views
    createNewSheetViewInCurrentView();

    {
        // View is now on the latest sheet view tab
        CPPUNIT_ASSERT_EQUAL(SCTAB(1), pTabView1->GetViewData().GetTabNumber());

        CPPUNIT_ASSERT_EQUAL(SCTAB(4), rDocument.GetTableCount());
        CPPUNIT_ASSERT_EQUAL(u"NewTab"_ustr, rDocument.GetAllTableNames()[0]);
        CPPUNIT_ASSERT_EQUAL(u"NewTab_3"_ustr, rDocument.GetAllTableNames()[1]);
        CPPUNIT_ASSERT_EQUAL(u"NewTab_2"_ustr, rDocument.GetAllTableNames()[2]);
        CPPUNIT_ASSERT_EQUAL(u"Hoja1"_ustr, rDocument.GetAllTableNames()[3]);

        // Sheet view must be present
        auto pSheetViewManager = rDocument.GetSheetViewManager(SCTAB(0));
        CPPUNIT_ASSERT(pSheetViewManager);

        CPPUNIT_ASSERT_EQUAL(size_t(2), pSheetViewManager->size());

        auto pSheetView1 = pSheetViewManager->get(0);
        CPPUNIT_ASSERT_EQUAL(SCTAB(2), pSheetView1->getTableNumber());

        auto pSheetView2 = pSheetViewManager->get(1);
        CPPUNIT_ASSERT_EQUAL(SCTAB(1), pSheetView2->getTableNumber());
    }

    // Delete the table - index 0
    uno::Sequence<beans::PropertyValue> aArgs(
        comphelper::InitPropertySequence({ { "Index", uno::Any(sal_uInt16(0)) } }));

    dispatchCommand(mxComponent, u".uno:Remove"_ustr, aArgs);

    {
        CPPUNIT_ASSERT_EQUAL(SCTAB(0), pTabView1->GetViewData().GetTabNumber());

        CPPUNIT_ASSERT_EQUAL(SCTAB(1), rDocument.GetTableCount());
        CPPUNIT_ASSERT_EQUAL(u"Hoja1"_ustr, rDocument.GetAllTableNames()[0]);
    }
}

CPPUNIT_TEST_FIXTURE(SheetViewTest, testRemoveSheetViewHolderTable)
{
    // Delete the sheet view holder table directly

    ScModelObj* pModelObj = createDoc("empty.ods");
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ScDocument& rDocument = *pModelObj->GetDocument();

    // Setup views
    ScTestViewCallback aView1;
    ScTabViewShell* pTabView1 = aView1.getTabViewShell();

    {
        auto pSheetViewManager = rDocument.GetSheetViewManager(SCTAB(0));
        CPPUNIT_ASSERT(pSheetViewManager);
        CPPUNIT_ASSERT_EQUAL(size_t(0), pSheetViewManager->size());
    }

    // Create first sheet views
    createNewSheetViewInCurrentView();

    {
        auto pSheetViewManager = rDocument.GetSheetViewManager(SCTAB(0));
        CPPUNIT_ASSERT(pSheetViewManager);
        CPPUNIT_ASSERT_EQUAL(size_t(1), pSheetViewManager->size());

        CPPUNIT_ASSERT_EQUAL(SCTAB(1), pTabView1->GetViewData().GetTabNumber());
        auto pSheetView1 = pSheetViewManager->get(0);
        CPPUNIT_ASSERT_EQUAL(SCTAB(1), pSheetView1->getTableNumber());

        CPPUNIT_ASSERT_EQUAL(SCTAB(2), rDocument.GetTableCount());
        CPPUNIT_ASSERT_EQUAL(u"Hoja1"_ustr, rDocument.GetAllTableNames()[0]);
        CPPUNIT_ASSERT_EQUAL(u"Hoja1_2"_ustr, rDocument.GetAllTableNames()[1]);
    }

    // Create second sheet views
    createNewSheetViewInCurrentView();

    {
        CPPUNIT_ASSERT_EQUAL(SCTAB(1), pTabView1->GetViewData().GetTabNumber());

        CPPUNIT_ASSERT_EQUAL(SCTAB(3), rDocument.GetTableCount());
        CPPUNIT_ASSERT_EQUAL(u"Hoja1"_ustr, rDocument.GetAllTableNames()[0]);
        CPPUNIT_ASSERT_EQUAL(u"Hoja1_3"_ustr, rDocument.GetAllTableNames()[1]);
        CPPUNIT_ASSERT_EQUAL(u"Hoja1_2"_ustr, rDocument.GetAllTableNames()[2]);

        // Sheet view must be present
        auto pSheetViewManager = rDocument.GetSheetViewManager(SCTAB(0));
        CPPUNIT_ASSERT(pSheetViewManager);

        CPPUNIT_ASSERT_EQUAL(size_t(2), pSheetViewManager->size());

        auto pSheetView1 = pSheetViewManager->get(0);
        CPPUNIT_ASSERT_EQUAL(SCTAB(2), pSheetView1->getTableNumber());

        auto pSheetView2 = pSheetViewManager->get(1);
        CPPUNIT_ASSERT_EQUAL(SCTAB(1), pSheetView2->getTableNumber());
    }

    // Unhide the sheet view holder tables (or they won't be deleted)
    {
        uno::Sequence<beans::PropertyValue> aArgs(
            comphelper::InitPropertySequence({ { "aTableName", uno::Any(u"Hoja1_3"_ustr) } }));
        dispatchCommand(mxComponent, u".uno:Show"_ustr, aArgs);
    }
    {
        uno::Sequence<beans::PropertyValue> aArgs(
            comphelper::InitPropertySequence({ { "aTableName", uno::Any(u"Hoja1_2"_ustr) } }));
        dispatchCommand(mxComponent, u".uno:Show"_ustr, aArgs);
    }

    // Delete the table
    {
        uno::Sequence<beans::PropertyValue> aArgs(
            comphelper::InitPropertySequence({ { "Index", uno::Any(sal_uInt16(2)) } }));

        dispatchCommand(mxComponent, u".uno:Remove"_ustr, aArgs);
    }

    {
        CPPUNIT_ASSERT_EQUAL(SCTAB(2), rDocument.GetTableCount());
        CPPUNIT_ASSERT_EQUAL(u"Hoja1"_ustr, rDocument.GetAllTableNames()[0]);
        CPPUNIT_ASSERT_EQUAL(u"Hoja1_2"_ustr, rDocument.GetAllTableNames()[1]);

        // Sheet view must be present
        auto pSheetViewManager = rDocument.GetSheetViewManager(SCTAB(0));
        CPPUNIT_ASSERT(pSheetViewManager);

        CPPUNIT_ASSERT_EQUAL(size_t(1), pSheetViewManager->size());

        auto pSheetView1 = pSheetViewManager->get(0);
        CPPUNIT_ASSERT_EQUAL(SCTAB(1), pSheetView1->getTableNumber());

        // Not deleted but the sheet view is now null
        auto pSheetView2 = pSheetViewManager->get(1);
        CPPUNIT_ASSERT(!pSheetView2);
    }
}

CPPUNIT_TEST_FIXTURE(SheetViewTest, testSyncAfterSorting_DefaultViewSort)
{
    // Two related scenarios tested:
    // 1. Auto-filter is sorted in the default view, then the data is changed in a sheet view.
    //    In this case the sheet view is unsorted and the default view is sorted, so the data
    //    in the default view needs to be first unsorted so the correct cell is changed.
    // 2. Continuation of scenario 1, where the default view is sorted again (ascending then
    //    descending order). In this case the sort orders must be combined correctly, so the
    //    change in the sheet view would still change the correct cell in default view.

    // Create two views, and leave the second one current.
    ScModelObj* pModelObj = createDoc("SheetView_AutoFilter.ods");
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());

    // Setup views
    ScTestViewCallback aSheetView;
    ScTabViewShell* pTabViewSheetView = aSheetView.getTabViewShell();

    SfxLokHelper::createView();
    Scheduler::ProcessEventsToIdle();

    ScTestViewCallback aDefaultView;
    ScTabViewShell* pTabViewDefaultView = aDefaultView.getTabViewShell();

    CPPUNIT_ASSERT(pTabViewSheetView != pTabViewDefaultView);
    CPPUNIT_ASSERT(aSheetView.getViewID() != aDefaultView.getViewID());

    // Switch to Sheet View and Create
    {
        SfxLokHelper::setView(aSheetView.getViewID());
        Scheduler::ProcessEventsToIdle();

        CPPUNIT_ASSERT_EQUAL(SCTAB(0), pTabViewSheetView->GetViewData().GetTabNumber());
        CPPUNIT_ASSERT_EQUAL(SCTAB(0), pTabViewDefaultView->GetViewData().GetTabNumber());

        createNewSheetViewInCurrentView();

        CPPUNIT_ASSERT_EQUAL(SCTAB(1), pTabViewSheetView->GetViewData().GetTabNumber());
        CPPUNIT_ASSERT_EQUAL(SCTAB(0), pTabViewDefaultView->GetViewData().GetTabNumber());
    }

    // Switch to Default View
    {
        SfxLokHelper::setView(aDefaultView.getViewID());
        Scheduler::ProcessEventsToIdle();

        CPPUNIT_ASSERT_EQUAL(SCTAB(1), pTabViewSheetView->GetViewData().GetTabNumber());
        CPPUNIT_ASSERT_EQUAL(SCTAB(0), pTabViewDefaultView->GetViewData().GetTabNumber());

        // Sort AutoFilter ascending
        sortAscendingForCell(u"A1");

        CPPUNIT_ASSERT_EQUAL(SCTAB(1), pTabViewSheetView->GetViewData().GetTabNumber());
        CPPUNIT_ASSERT_EQUAL(SCTAB(0), pTabViewDefaultView->GetViewData().GetTabNumber());

        // Check values
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"3", u"4", u"5", u"7" }),
                             getValues(pTabViewDefaultView, 0, 1, 4));
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"3", u"7" }),
                             getValues(pTabViewSheetView, 0, 1, 4));

        typeCharsInCell(std::string("9"), 0, 1, pTabViewDefaultView, pModelObj);

        // Check values
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"9", u"4", u"5", u"7" }),
                             getValues(pTabViewDefaultView, 0, 1, 4));
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"9", u"7" }),
                             getValues(pTabViewSheetView, 0, 1, 4));

        // Sort AutoFilter AGAIN descending
        sortDescendingForCell(u"A1");

        // Check values
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"9", u"7", u"5", u"4" }),
                             getValues(pTabViewDefaultView, 0, 1, 4));
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"9", u"7" }),
                             getValues(pTabViewSheetView, 0, 1, 4));

        typeCharsInCell(std::string("6"), 0, 3, pTabViewDefaultView, pModelObj);

        // Check values
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"9", u"7", u"6", u"4" }),
                             getValues(pTabViewDefaultView, 0, 1, 4));
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"6", u"9", u"7" }),
                             getValues(pTabViewSheetView, 0, 1, 4));
    }
}

CPPUNIT_TEST_FIXTURE(SheetViewTest, testSyncAfterSorting_SheetViewSort)
{
    // Two related scenarios tested:
    //
    // 1. Auto-filter is sorted in the sheet view, then the data is changed in a sheet view.
    //    In this case the default view is unsorted and the sheet view is sorted, so the data
    //    in the sheet view needs to be first unsorted so we get the correct position of the
    //    cell that needs to be changed.
    // 2. Continuation of scenario 1, where the sheet view is sorted again (ascending then
    //    descending order). In this case the sort orders must be combined correctly.

    // Create two views, and leave the second one current.
    ScModelObj* pModelObj = createDoc("SheetView_AutoFilter.ods");
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());

    // Setup views
    ScTestViewCallback aSheetView;
    ScTabViewShell* pTabViewSheetView = aSheetView.getTabViewShell();

    SfxLokHelper::createView();
    Scheduler::ProcessEventsToIdle();

    ScTestViewCallback aDefaultView;
    ScTabViewShell* pTabViewDefaultView = aDefaultView.getTabViewShell();

    CPPUNIT_ASSERT(pTabViewSheetView != pTabViewDefaultView);
    CPPUNIT_ASSERT(aSheetView.getViewID() != aDefaultView.getViewID());

    // Switch to Sheet View and Create
    {
        SfxLokHelper::setView(aSheetView.getViewID());
        Scheduler::ProcessEventsToIdle();

        // New Sheet view
        createNewSheetViewInCurrentView();

        // Expect the data is same in both
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"3", u"7" }),
                             getValues(pTabViewSheetView, 0, 1, 4));
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"3", u"7" }),
                             getValues(pTabViewDefaultView, 0, 1, 4));

        // Sort AutoFilter
        sortAscendingForCell(u"A1");

        // Check values - Sheet View
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"3", u"4", u"5", u"7" }),
                             getValues(pTabViewSheetView, 0, 1, 4));
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"3", u"7" }),
                             getValues(pTabViewDefaultView, 0, 1, 4));

        typeCharsInCell(std::string("9"), 0, 1, pTabViewSheetView, pModelObj);

        // Sheet view is automatically sorted if it was sorted before.
        // So it would be { u"9", u"4", u"5", u"7" }
        // and then sorted to { u"4", u"5", u"7", u"9" }
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"7", u"9" }),
                             getValues(pTabViewSheetView, 0, 1, 4));
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"9", u"7" }),
                             getValues(pTabViewDefaultView, 0, 1, 4));

        // Scenario 2

        // Sort AutoFilter
        sortDescendingForCell(u"A1");

        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"9", u"7", u"5", u"4" }),
                             getValues(pTabViewSheetView, 0, 1, 4));
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"9", u"7" }),
                             getValues(pTabViewDefaultView, 0, 1, 4));

        typeCharsInCell(std::string("6"), 0, 3, pTabViewSheetView, pModelObj);

        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"6", u"9", u"7" }),
                             getValues(pTabViewDefaultView, 0, 1, 4));

        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"9", u"7", u"6", u"4" }),
                             getValues(pTabViewSheetView, 0, 1, 4));
    }
}

CPPUNIT_TEST_FIXTURE(SheetViewTest, testSyncAfterSorting_SortInDefaultAndSheetView)
{
    // Create two views, and leave the second one current.
    ScModelObj* pModelObj = createDoc("SheetView_AutoFilter.ods");
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());

    // Setup views
    ScTestViewCallback aSheetView;
    ScTabViewShell* pTabViewSheetView = aSheetView.getTabViewShell();

    SfxLokHelper::createView();
    Scheduler::ProcessEventsToIdle();

    ScTestViewCallback aDefaultView;
    ScTabViewShell* pTabViewDefaultView = aDefaultView.getTabViewShell();

    CPPUNIT_ASSERT(pTabViewSheetView != pTabViewDefaultView);
    CPPUNIT_ASSERT(aSheetView.getViewID() != aDefaultView.getViewID());

    // Switch to Sheet View and Create
    {
        SfxLokHelper::setView(aSheetView.getViewID());
        Scheduler::ProcessEventsToIdle();

        createNewSheetViewInCurrentView();

        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"3", u"7" }),
                             getValues(pTabViewDefaultView, 0, 1, 4));

        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"3", u"7" }),
                             getValues(pTabViewSheetView, 0, 1, 4));

        // Sort AutoFilter
        sortAscendingForCell(u"A1");

        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"3", u"7" }),
                             getValues(pTabViewDefaultView, 0, 1, 4));

        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"3", u"4", u"5", u"7" }),
                             getValues(pTabViewSheetView, 0, 1, 4));
    }

    // Switch to Default view and sort
    {
        SfxLokHelper::setView(aDefaultView.getViewID());
        Scheduler::ProcessEventsToIdle();

        // Sort AutoFilter
        sortDescendingForCell(u"A1");

        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"7", u"5", u"4", u"3" }),
                             getValues(pTabViewDefaultView, 0, 1, 4));

        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"3", u"4", u"5", u"7" }),
                             getValues(pTabViewSheetView, 0, 1, 4));
    }

    // Switch to Sheet view and set a value
    {
        SfxLokHelper::setView(aSheetView.getViewID());
        Scheduler::ProcessEventsToIdle();

        // Change "4" to "44"
        typeCharsInCell(std::string("44"), 0, 2, pTabViewSheetView, pModelObj);

        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"7", u"5", u"44", u"3" }),
                             getValues(pTabViewDefaultView, 0, 1, 4));

        // Result in { u"3", u"44", u"5", u"7" } but we resort the sheet view immediately
        // so we get { u"3", u"5", u"7", u"44" }.
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"3", u"5", u"7", u"44" }),
                             getValues(pTabViewSheetView, 0, 1, 4));
    }
}

CPPUNIT_TEST_FIXTURE(SyncTest, testSyncAfterSorting_SortInDefaultAndSheetView_Formulas)
{
    // Input is an test file that uses formulas instead of values.
    // This test checks if the sorting of the document behaves the same
    // for formulas as values.
    ScModelObj* pModelObj = createDoc("SheetView_AutoFilter_Formulas.ods");
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());

    setupViews();

    // Switch to Sheet View and Create
    {
        switchToSheetView();

        createNewSheetViewInCurrentView();

        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"3", u"7", u"1", u"6" }),
                             getValues(mpTabViewDefaultView, 0, 1, 6));

        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"3", u"7", u"1", u"6" }),
                             getValues(mpTabViewSheetView, 0, 1, 6));

        // Sort AutoFilter
        sortAscendingForCell(u"A1");

        // No change to sorted values
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"3", u"7", u"1", u"6" }),
                             getValues(mpTabViewDefaultView, 0, 1, 6));
        // Expect sorted values for the sheet view
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"1", u"3", u"4", u"5", u"6", u"7" }),
                             getValues(mpTabViewSheetView, 0, 1, 6));
    }

    // Switch to Default view and sort
    {
        switchToDefaultView();

        // Sort AutoFilter
        sortDescendingForCell(u"A1");

        // Expect sorted values for the default view
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"7", u"6", u"5", u"4", u"3", u"1" }),
                             getValues(mpTabViewDefaultView, 0, 1, 6));

        // Previously sorted values - no change
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"1", u"3", u"4", u"5", u"6", u"7" }),
                             getValues(mpTabViewSheetView, 0, 1, 6));
    }

    // Switch to Sheet view and set a value
    {
        switchToSheetView();

        // Change "4" to "44"
        typeCharsInCell(std::string("=40+4"), 0, 3, mpTabViewSheetView, pModelObj);

        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"7", u"6", u"5", u"44", u"3", u"1" }),
                             getValues(mpTabViewDefaultView, 0, 1, 6));

        // Result in { u"1", u"3", u"4", u"5", u"6", u"7" } but we resort the sheet view immediately
        // so we get { u"1", u"3", u"5", u"6", u"7", u"44" }.
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"1", u"3", u"5", u"6", u"7", u"44" }),
                             getValues(mpTabViewSheetView, 0, 1, 6));
    }
}

CPPUNIT_TEST_FIXTURE(SyncTest, testSyncAfterSorting_SortInDefaultView_Attributes)
{
    // Instead of the number, we set the attribute
    ScModelObj* pModelObj = createDoc("SheetView_AutoFilter.ods");
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ScDocument* pDocument = pModelObj->GetDocument();

    setupViews();

    // Create new sheet view
    {
        switchToSheetView();
        createNewSheetViewInCurrentView();
    }

    // Sort autofilter descending in default view
    {
        switchToDefaultView();
        sortDescendingForCell(u"A1");
    }

    // Switch to Sheet view and set "bold" text attribute to all cells in the auto filter one by one
    {
        switchToSheetView();

        // Current state default view
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"7", u"5", u"4", u"3" }),
                             getValues(mpTabViewDefaultView, 0, 1, 4));

        // Current state sheet view
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"3", u"7" }),
                             getValues(mpTabViewSheetView, 0, 1, 4));

        // Current font weight state
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"N", u"N", u"N", u"N" }),
                             getTextWeight(pDocument, 0, 1, 4, 0));

        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"N", u"N", u"N", u"N" }),
                             getTextWeight(pDocument, 0, 1, 4, 1));

        // Set A4 in sheet view -> A5 in default view
        setCellBold(u"A4");

        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"N", u"N", u"N", u"B" }),
                             getTextWeight(pDocument, 0, 1, 4, 0));

        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"N", u"N", u"B", u"N" }),
                             getTextWeight(pDocument, 0, 1, 4, 1));

        // Set A2 in sheet view -> A4 in default view
        setCellBold(u"A2");

        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"N", u"N", u"B", u"B" }),
                             getTextWeight(pDocument, 0, 1, 4, 0));

        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"B", u"N", u"B", u"N" }),
                             getTextWeight(pDocument, 0, 1, 4, 1));

        // Set A3 in sheet view -> A3 in default view
        setCellBold(u"A3");

        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"N", u"B", u"B", u"B" }),
                             getTextWeight(pDocument, 0, 1, 4, 0));

        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"B", u"B", u"B", u"N" }),
                             getTextWeight(pDocument, 0, 1, 4, 1));

        // Set A5 in sheet view -> A2 in default view
        setCellBold(u"A5");

        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"B", u"B", u"B", u"B" }),
                             getTextWeight(pDocument, 0, 1, 4, 0));

        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"B", u"B", u"B", u"B" }),
                             getTextWeight(pDocument, 0, 1, 4, 1));

        // Set range A4:A5, which are A2, A5 in default view
        setCellBold(u"A4:A5");

        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"N", u"B", u"B", u"N" }),
                             getTextWeight(pDocument, 0, 1, 4, 0));

        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"B", u"B", u"N", u"N" }),
                             getTextWeight(pDocument, 0, 1, 4, 1));
    }
}

CPPUNIT_TEST_FIXTURE(SyncTest, testSyncAfterSorting_SortInDefaultAndSheetView_Attributes)
{
    // Instead of the number, we set the attribute
    ScModelObj* pModelObj = createDoc("SheetView_AutoFilter.ods");
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ScDocument* pDocument = pModelObj->GetDocument();

    setupViews();

    // Create new sheet view and sort autofilter ascending
    {
        switchToSheetView();
        createNewSheetViewInCurrentView();
        sortAscendingForCell(u"A1");
    }

    // Sort autofilter descending in default view
    {
        switchToDefaultView();
        sortDescendingForCell(u"A1");
    }

    // Switch to Sheet view and set "bold" text attribute to all cells in the auto filter one by one
    {
        switchToSheetView();

        // Current state default view
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"7", u"5", u"4", u"3" }),
                             getValues(mpTabViewDefaultView, 0, 1, 4));

        // Current state sheet view
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"3", u"4", u"5", u"7" }),
                             getValues(mpTabViewSheetView, 0, 1, 4));

        // Current font weight state
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"N", u"N", u"N", u"N" }),
                             getTextWeight(pDocument, 0, 1, 4, 0));
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"N", u"N", u"N", u"N" }),
                             getTextWeight(pDocument, 0, 1, 4, 1));

        // Set A5 in sheet view -> A2 in default view
        setCellBold(u"A5");

        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"B", u"N", u"N", u"N" }),
                             getTextWeight(pDocument, 0, 1, 4, 0));

        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"N", u"N", u"N", u"B" }),
                             getTextWeight(pDocument, 0, 1, 4, 1));

        // Set A4 in sheet view -> A3 in default view
        setCellBold(u"A4");

        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"B", u"B", u"N", u"N" }),
                             getTextWeight(pDocument, 0, 1, 4, 0));

        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"N", u"N", u"B", u"B" }),
                             getTextWeight(pDocument, 0, 1, 4, 1));

        // Set A3 in sheet view -> A4 in default view
        setCellBold(u"A3");

        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"B", u"B", u"B", u"N" }),
                             getTextWeight(pDocument, 0, 1, 4, 0));

        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"N", u"B", u"B", u"B" }),
                             getTextWeight(pDocument, 0, 1, 4, 1));

        // Set A2 in sheet view -> A4 in default view
        setCellBold(u"A2");

        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"B", u"B", u"B", u"B" }),
                             getTextWeight(pDocument, 0, 1, 4, 0));

        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"B", u"B", u"B", u"B" }),
                             getTextWeight(pDocument, 0, 1, 4, 1));
    }
}

CPPUNIT_TEST_FIXTURE(SyncTest, testSync_DefaultView_DeleteCellOperation)
{
    // Create two views, and leave the second one current.
    ScModelObj* pModelObj = createDoc("SheetView_AutoFilter.ods");
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ScDocument* pDocument = pModelObj->GetDocument();

    setupViews();

    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"3", u"7" }),
                         getValues(pDocument, 0, 1, 4, 0));
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"", u"", u"", u"" }), getValues(pDocument, 0, 1, 4, 1));

    // Switch to Sheet View and Create
    {
        switchToSheetView();
        createNewSheetViewInCurrentView();
        sortDescendingForCell(u"A1");
    }

    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"3", u"7" }),
                         getValues(pDocument, 0, 1, 4, 0));
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"7", u"5", u"4", u"3" }),
                         getValues(pDocument, 0, 1, 4, 1));

    // Switch to Default View
    {
        switchToDefaultView();

        gotoCell(u"A3");

        dispatchCommand(mxComponent, u".uno:ClearContents"_ustr, {});
    }

    OUString aExpected = expectedValues({ u"4", u"", u"3", u"7" });
    CPPUNIT_ASSERT_EQUAL(aExpected, getValues(mpTabViewDefaultView, 0, 1, 4));
    CPPUNIT_ASSERT_EQUAL(aExpected, getValues(pDocument, 0, 1, 4, 0));

    OUString aExpectedSorted = expectedValues({ u"7", u"4", u"3", u"" });
    CPPUNIT_ASSERT_EQUAL(aExpectedSorted, getValues(mpTabViewSheetView, 0, 1, 4));
    CPPUNIT_ASSERT_EQUAL(aExpectedSorted, getValues(pDocument, 0, 1, 4, 1));
}

CPPUNIT_TEST_FIXTURE(SyncTest, testSync_SheetView_DeleteCellOperation)
{
    // Create two views, and leave the second one current.
    ScModelObj* pModelObj = createDoc("SheetView_AutoFilter.ods");
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ScDocument* pDocument = pModelObj->GetDocument();

    setupViews();

    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"3", u"7" }),
                         getValues(pDocument, 0, 1, 4, 0));
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"", u"", u"", u"" }), getValues(pDocument, 0, 1, 4, 1));

    // Switch to Sheet View and Create
    {
        switchToSheetView();

        createNewSheetViewInCurrentView();
        sortDescendingForCell(u"A1");

        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"3", u"7" }),
                             getValues(pDocument, 0, 1, 4, 0));
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"7", u"5", u"4", u"3" }),
                             getValues(pDocument, 0, 1, 4, 1));

        gotoCell(u"A4");

        dispatchCommand(mxComponent, u".uno:ClearContents"_ustr, {});
    }

    OUString aExpected = expectedValues({ u"", u"5", u"3", u"7" });
    CPPUNIT_ASSERT_EQUAL(aExpected, getValues(mpTabViewDefaultView, 0, 1, 4));
    CPPUNIT_ASSERT_EQUAL(aExpected, getValues(pDocument, 0, 1, 4, 0));

    OUString aExpectedSorted = expectedValues({ u"7", u"5", u"3", u"" });
    CPPUNIT_ASSERT_EQUAL(aExpectedSorted, getValues(mpTabViewSheetView, 0, 1, 4));
    CPPUNIT_ASSERT_EQUAL(aExpectedSorted, getValues(pDocument, 0, 1, 4, 1));
}

CPPUNIT_TEST_FIXTURE(SyncTest, testSync_DefaultView_DeleteContentOperation)
{
    // Create two views, and leave the second one current.
    ScModelObj* pModelObj = createDoc("SheetView_AutoFilter.ods");
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ScDocument* pDocument = pModelObj->GetDocument();

    setupViews();

    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"3", u"7" }),
                         getValues(pDocument, 0, 1, 4, 0));
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"", u"", u"", u"" }), getValues(pDocument, 0, 1, 4, 1));

    // Switch to Sheet View and Create
    {
        switchToSheetView();
        createNewSheetViewInCurrentView();
        sortDescendingForCell(u"A1");
    }

    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"3", u"7" }),
                         getValues(pDocument, 0, 1, 4, 0));
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"7", u"5", u"4", u"3" }),
                         getValues(pDocument, 0, 1, 4, 1));

    // Switch to Default View
    {
        switchToDefaultView();

        gotoCell(u"A3:A4");

        dispatchCommand(mxComponent, u".uno:ClearContents"_ustr, {});
    }

    OUString aExpected = expectedValues({ u"4", u"", u"", u"7" });
    CPPUNIT_ASSERT_EQUAL(aExpected, getValues(mpTabViewDefaultView, 0, 1, 4));
    CPPUNIT_ASSERT_EQUAL(aExpected, getValues(pDocument, 0, 1, 4, 0));

    OUString aExpectedSorted = expectedValues({ u"7", u"4", u"", u"" });
    CPPUNIT_ASSERT_EQUAL(aExpectedSorted, getValues(mpTabViewSheetView, 0, 1, 4));
    CPPUNIT_ASSERT_EQUAL(aExpectedSorted, getValues(pDocument, 0, 1, 4, 1));
}

CPPUNIT_TEST_FIXTURE(SyncTest, testSync_SheetView_DeleteContentOperation)
{
    // Create two views, and leave the second one current.
    ScModelObj* pModelObj = createDoc("SheetView_AutoFilter.ods");
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ScDocument* pDocument = pModelObj->GetDocument();

    setupViews();

    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"3", u"7" }),
                         getValues(pDocument, 0, 1, 4, 0));
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"", u"", u"", u"" }), getValues(pDocument, 0, 1, 4, 1));

    // Switch to Sheet View and Create
    {
        switchToSheetView();

        createNewSheetViewInCurrentView();
        sortDescendingForCell(u"A1");

        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"3", u"7" }),
                             getValues(pDocument, 0, 1, 4, 0));
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"7", u"5", u"4", u"3" }),
                             getValues(pDocument, 0, 1, 4, 1));

        gotoCell(u"A3:A4");

        dispatchCommand(mxComponent, u".uno:ClearContents"_ustr, {});
    }

    OUString aExpected = expectedValues({ u"", u"", u"3", u"7" });
    CPPUNIT_ASSERT_EQUAL(aExpected, getValues(mpTabViewDefaultView, 0, 1, 4));
    CPPUNIT_ASSERT_EQUAL(aExpected, getValues(pDocument, 0, 1, 4, 0));

    OUString aExpectedSorted = expectedValues({ u"7", u"3", u"", u"" });
    CPPUNIT_ASSERT_EQUAL(aExpectedSorted, getValues(mpTabViewSheetView, 0, 1, 4));
    CPPUNIT_ASSERT_EQUAL(aExpectedSorted, getValues(pDocument, 0, 1, 4, 1));
}

CPPUNIT_TEST_FIXTURE(SyncTest, testCreateAndDeleteSheetView)
{
    // Test that creating a sheet view, deleting it, creating a new one again,
    // works without an issue.

    ScModelObj* pModelObj = createDoc("SheetView_AutoFilter.ods");
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ScDocument* pDocument = pModelObj->GetDocument();

    setupViews();

    // Initial values
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"3", u"7" }),
                         getValues(pDocument, 0, 1, 4, 0));

    // Create a sheet view
    {
        switchToSheetView();
        createNewSheetViewInCurrentView();
    }

    // Delete the sheet view
    {
        switchToSheetView();
        removeSheetViewInCurrentView();
    }

    // Create a new sheet view again
    {
        switchToSheetView();
        createNewSheetViewInCurrentView();

        typeCharsInCell(std::string("99"), 0, 1, mpTabViewSheetView, pModelObj);

        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"99", u"5", u"3", u"7" }),
                             getValues(mpTabViewSheetView, 0, 1, 4));
    }
}

CPPUNIT_TEST_FIXTURE(SyncTest, testSorting_NonAutoFilterRange)
{
    // When we sort outside of the auto-filtered range, we sync the changes
    // to all views. When sorting auto-filtered range, then we sort only the
    // current view and don't sync.

    ScModelObj* pModelObj = createDoc("SheetView_AutoFilter_Extended.ods");
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ScDocument& rDocument = *pModelObj->GetDocument();

    setupViews();

    // Check sorting when no sheet view was created yet
    {
        switchToDefaultView();

        // Check we have no sheet view yet
        auto pSheetViewManager = rDocument.GetSheetViewManager(SCTAB(0));
        CPPUNIT_ASSERT(pSheetViewManager);
        CPPUNIT_ASSERT(pSheetViewManager->isEmpty());

        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"3", u"7" }),
                             getValues(mpTabViewDefaultView, 0, 1, 4));
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"ccc", u"rrr", u"sss", u"aaa" }),
                             getValues(mpTabViewDefaultView, 1, 1, 4));
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"44", u"22", u"11", u"33" }),
                             getValues(mpTabViewDefaultView, 4, 1, 4));

        // Select and sort the range - no sheet views yet
        sortDescendingForCell(u"E2:E5");

        // Check - we expect to be sorted descending
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"44", u"33", u"22", u"11" }),
                             getValues(mpTabViewDefaultView, 4, 1, 4));
    }

    // Check sheet view sorting
    {
        switchToSheetView();
        createNewSheetViewInCurrentView();

        // Check initial values of the E2:E5 range
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"44", u"33", u"22", u"11" }),
                             getValues(mpTabViewSheetView, 4, 1, 4));
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"44", u"33", u"22", u"11" }),
                             getValues(mpTabViewDefaultView, 4, 1, 4));

        // Sort the range ascending
        sortAscendingForCell(u"E2:E5");

        // Check - we expect both views to be sorted ascending
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"11", u"22", u"33", u"44" }),
                             getValues(mpTabViewSheetView, 4, 1, 4));
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"11", u"22", u"33", u"44" }),
                             getValues(mpTabViewDefaultView, 4, 1, 4));

        // Check the auto-filtered values in column B
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"ccc", u"rrr", u"sss", u"aaa" }),
                             getValues(mpTabViewSheetView, 1, 1, 4));
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"ccc", u"rrr", u"sss", u"aaa" }),
                             getValues(mpTabViewDefaultView, 1, 1, 4));

        // Select the range of an auto-filtered range and sort ascending column B
        sortAscendingForCell(u"B1");

        // Check - only the sheet view should be sorted
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"aaa", u"ccc", u"rrr", u"sss" }),
                             getValues(mpTabViewSheetView, 1, 1, 4));
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"ccc", u"rrr", u"sss", u"aaa" }),
                             getValues(mpTabViewDefaultView, 1, 1, 4));
    }
}

CPPUNIT_TEST_FIXTURE(SheetViewTest, testRemoveSheetViewAndSwitchTab)
{
    // Test that creating a sheet view, removing it, switching to another tab
    // and switching back shows the correct data (not an empty sheet).

    ScModelObj* pModelObj = createDoc("empty.ods");
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ScDocument& rDocument = *pModelObj->GetDocument();

    ScTestViewCallback aView1;
    ScTabViewShell* pTabView1 = aView1.getTabViewShell();

    // Insert a second sheet
    uno::Sequence<beans::PropertyValue> aArgsInsert(comphelper::InitPropertySequence(
        { { "Name", uno::Any(u"Sheet2"_ustr) }, { "Index", uno::Any(sal_Int16(2)) } }));
    dispatchCommand(mxComponent, u".uno:Insert"_ustr, aArgsInsert);

    CPPUNIT_ASSERT_EQUAL(SCTAB(2), rDocument.GetTableCount());

    // Go back to the first sheet
    pTabView1->SetTabNo(0);

    // Put some data in the first sheet
    rDocument.SetString(ScAddress(0, 0, 0), u"ABC"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"ABC"_ustr, rDocument.GetString(ScAddress(0, 0, 0)));

    // Create a sheet view on the first sheet
    createNewSheetViewInCurrentView();

    // Verify we are now on the sheet view tab
    CPPUNIT_ASSERT(rDocument.GetTableSheetViewID(pTabView1->GetViewData().GetTabNumber())
                   != sc::DefaultSheetViewID);

    // Remove the sheet view
    removeSheetViewInCurrentView();

    // Should be back on the default tab (first sheet)
    CPPUNIT_ASSERT_EQUAL(SCTAB(0), pTabView1->GetViewData().GetTabNumber());

    // Switch to the second sheet
    pTabView1->SetTabNo(1);
    CPPUNIT_ASSERT_EQUAL(SCTAB(1), pTabView1->GetViewData().GetTabNumber());

    // Switch back to the first sheet
    pTabView1->SetTabNo(0);
    CPPUNIT_ASSERT_EQUAL(SCTAB(0), pTabView1->GetViewData().GetTabNumber());

    // Verify the data is still visible (not an empty sheet)
    CPPUNIT_ASSERT_EQUAL(u"ABC"_ustr, rDocument.GetString(ScAddress(0, 0, 0)));
}

CPPUNIT_TEST_FIXTURE(SheetViewTest, testNewViewOpensInDefaultView)
{
    // Test that opening a new view while on a sheet view tab opens the new
    // view on the default tab, not on the sheet view tab.

    ScModelObj* pModelObj = createDoc("SheetView_AutoFilter.ods");
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());

    // Setup first view
    ScTestViewCallback aView1;
    ScTabViewShell* pTabView1 = aView1.getTabViewShell();

    // Verify we start on tab 0 (default)
    CPPUNIT_ASSERT_EQUAL(SCTAB(0), pTabView1->GetViewData().GetTabNumber());

    // Create a sheet view - view 1 should move to the sheet view tab
    createNewSheetViewInCurrentView();

    // Create a new view - simulates a new user or new window
    SfxLokHelper::createView();
    Scheduler::ProcessEventsToIdle();

    ScTestViewCallback aView2;
    ScTabViewShell* pTabView2 = aView2.getTabViewShell();

    CPPUNIT_ASSERT(pTabView1 != pTabView2);

    // View 1 should be open on sheet view tab
    CPPUNIT_ASSERT_EQUAL(SCTAB(1), pTabView1->GetViewData().GetTabNumber());
    CPPUNIT_ASSERT(pTabView1->GetViewData().GetSheetViewID() != sc::DefaultSheetViewID);

    // The view 2 should open on the default tab
    CPPUNIT_ASSERT_EQUAL(SCTAB(0), pTabView2->GetViewData().GetTabNumber());
    CPPUNIT_ASSERT_EQUAL(sc::DefaultSheetViewID, pTabView2->GetViewData().GetSheetViewID());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
