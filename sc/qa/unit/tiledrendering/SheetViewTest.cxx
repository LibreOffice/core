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
#include <docfunc.hxx>
#include <docsh.hxx>
#include <docuno.hxx>
#include <markdata.hxx>
#include <postit.hxx>
#include <scitems.hxx>
#include <SheetView.hxx>
#include <SheetViewManager.hxx>
#include <attrib.hxx>
#include <editeng/brushitem.hxx>
#include <i18nutil/transliteration.hxx>
#include <paramisc.hxx>
#include <cellmergeoption.hxx>
#include <rangenam.hxx>
#include <dbdocfun.hxx>
#include <dbdata.hxx>
#include <subtotalparam.hxx>
#include <drawview.hxx>
#include <drwlayer.hxx>
#include <Sparkline.hxx>
#include <SparklineAttributes.hxx>
#include <SparklineGroup.hxx>
#include <dpobject.hxx>
#include <operation/DeleteCellsOperation.hxx>
#include <operation/InsertCellsOperation.hxx>
#include <operation/ReplacePivotTableOperation.hxx>
#include <dpshttab.hxx>
#include <dpsave.hxx>
#include <generalfunction.hxx>
#include <svx/svdorect.hxx>
#include <svx/svdpage.hxx>

#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>

using namespace css;

class SheetViewTest : public ScTiledRenderingTest
{
protected:
    static ScRange getAutoFilterArea(ScDocument& rDocument, SCTAB nTab)
    {
        ScDBData* pDBData = rDocument.GetAnonymousDBData(nTab);
        CPPUNIT_ASSERT(pDBData);
        CPPUNIT_ASSERT(pDBData->HasAutoFilter());
        ScRange aRange;
        pDBData->GetArea(aRange);
        return aRange;
    }

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

    static OUString getBackgroundColor(ScDocument* pDocument, SCCOL nCol, SCROW nStartRow,
                                       SCROW nEndRow, SCTAB nTab)
    {
        OUString aString;
        bool bFirst = true;
        for (SCROW nRow = nStartRow; nRow <= nEndRow; nRow++)
        {
            const ScPatternAttr* pPattern = pDocument->GetPattern(nCol, nRow, nTab);
            const SvxBrushItem& rBrush = pPattern->GetItem(ATTR_BACKGROUND);
            OUString aColor = rBrush.GetColor().AsRGBHexString();
            if (bFirst)
            {
                bFirst = false;
                aString = u"\""_ustr + aColor + u"\""_ustr;
            }
            else
                aString += u", \""_ustr + aColor + u"\""_ustr;
        }
        return aString;
    }

    // Writes to string the indent state for a row range, Y = indented, N = not indented
    static OUString getIndent(ScDocument* pDocument, SCCOL nCol, SCROW nStartRow, SCROW nEndRow,
                              SCTAB nTab)
    {
        OUString aString;
        bool bFirst = true;
        for (SCROW nRow = nStartRow; nRow <= nEndRow; nRow++)
        {
            const ScPatternAttr* pPattern = pDocument->GetPattern(nCol, nRow, nTab);
            const ScIndentItem& rIndent = pPattern->GetItem(ATTR_INDENT);
            OUString aValue = rIndent.GetValue() > 0 ? u"Y"_ustr : u"N"_ustr;
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

    void undo() { dispatchCommand(mxComponent, u".uno:Undo"_ustr, {}); }

    void redo() { dispatchCommand(mxComponent, u".uno:Redo"_ustr, {}); }
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

    ScDrawView* getDrawView()
    {
        ScTabViewShell* pTabViewShell = ScTabViewShell::GetActiveViewShell();
        CPPUNIT_ASSERT(pTabViewShell);
        ScDrawView* pDrawView = pTabViewShell->GetScDrawView();
        CPPUNIT_ASSERT(pDrawView);
        return pDrawView;
    }

    /** Find first pivot table on the input tab. */
    static ScDPObject* findPivotTableOnTab(ScDocument& rDocument, SCTAB nTab)
    {
        ScDPCollection* pDPCollection = rDocument.GetDPCollection();
        for (size_t i = 0; i < pDPCollection->GetCount(); ++i)
        {
            ScDPObject& rDPObject = (*pDPCollection)[i];
            if (rDPObject.GetOutRange().aStart.Tab() == nTab)
                return &rDPObject;
        }
        return nullptr;
    }

    static size_t countPivotTablesOnTab(ScDocument& rDocument, SCTAB nTab)
    {
        size_t nCount = 0;
        ScDPCollection* pDPCollection = rDocument.GetDPCollection();
        for (size_t i = 0; i < pDPCollection->GetCount(); ++i)
        {
            if ((*pDPCollection)[i].GetOutRange().aStart.Tab() == nTab)
                ++nCount;
        }
        return nCount;
    }

    static void setupPivotTableSourceData(ScDocument& rDocument)
    {
        rDocument.SetString(ScAddress(0, 0, 0), u"Category"_ustr);
        rDocument.SetString(ScAddress(1, 0, 0), u"Value"_ustr);
        rDocument.SetString(ScAddress(0, 1, 0), u"A"_ustr);
        rDocument.SetString(ScAddress(0, 2, 0), u"A"_ustr);
        rDocument.SetString(ScAddress(0, 3, 0), u"B"_ustr);
        rDocument.SetValue(ScAddress(1, 1, 0), 10.0);
        rDocument.SetValue(ScAddress(1, 2, 0), 20.0);
        rDocument.SetValue(ScAddress(1, 3, 0), 30.0);
    }

    static ScDPObject
    createCategoryValueDPObject(ScDocument& rDocument, SCTAB nTab, ScRange aOutRange,
                                css::sheet::DataPilotFieldOrientation eCategoryOrientation)
    {
        ScDPObject aDPObject(&rDocument);
        ScSheetSourceDesc aSheetDesc(&rDocument);
        aSheetDesc.SetSourceRange(ScRange(0, 0, nTab, 1, 3, nTab));
        aDPObject.SetSheetDesc(aSheetDesc);
        aDPObject.SetOutRange(aOutRange);

        ScDPSaveData aSaveData;
        ScDPSaveDimension* pDim = aSaveData.GetNewDimensionByName(u"Category"_ustr);
        pDim->SetOrientation(eCategoryOrientation);
        pDim = aSaveData.GetNewDimensionByName(u"Value"_ustr);
        pDim->SetOrientation(css::sheet::DataPilotFieldOrientation_DATA);
        pDim->SetFunction(ScGeneralFunction::SUM);
        aDPObject.SetSaveData(aSaveData);

        return aDPObject;
    }

    void insertDrawObject(ScDrawLayer* pDrawLayer, const tools::Rectangle& rRectangle)
    {
        ScDrawView* pDrawView = getDrawView();
        rtl::Reference<SdrRectObj> pObject = new SdrRectObj(*pDrawLayer, rRectangle);
        pDrawView->InsertObjectSafe(pObject.get(), *pDrawView->GetSdrPageView());
    }

    void moveDrawObject(SdrObject* pObject, const tools::Rectangle& rNewPosition)
    {
        tools::Rectangle aOldPosition = pObject->GetLogicRect();
        Size aOffset(rNewPosition.Left() - aOldPosition.Left(),
                     rNewPosition.Top() - aOldPosition.Top());
        ScDrawView* pDrawView = getDrawView();
        pDrawView->UnmarkAllObj();
        pDrawView->MarkObj(pObject, pDrawView->GetSdrPageView());
        pDrawView->MoveMarkedObj(aOffset);
    }

    void deleteDrawObject(SdrObject* pObject)
    {
        ScDrawView* pDrawView = getDrawView();
        pDrawView->UnmarkAllObj();
        pDrawView->MarkObj(pObject, pDrawView->GetSdrPageView());
        pDrawView->DeleteMarkedObj();
    }
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

    // Sort on default view
    sortDescendingForCell(u"A1");
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

    // Sort on sheet view 1
    SfxLokHelper::setView(aView1.getViewID());
    Scheduler::ProcessEventsToIdle();
    sortDescendingForCell(u"A1");
}

CPPUNIT_TEST_FIXTURE(SheetViewTest, testCheckIfSheetViewIsSavedInDocument_ODF)
{
    // Check if sheet view holder table is saved into the ODF document
    ScModelObj* pModelObj = createDoc("SheetView_AutoFilter.ods");
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());

    createNewSheetViewInCurrentView();

    save(TestFilter::ODS);

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

    save(TestFilter::XLSX);

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

CPPUNIT_TEST_FIXTURE(SyncTest, testSync_DefaultView_ClearItemsOperation)
{
    ScModelObj* pModelObj = createDoc("SheetView_AutoFilter.ods");
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ScDocument* pDocument = pModelObj->GetDocument();
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());

    setupViews();

    // Switch to Sheet View and Create, sort descending
    {
        switchToSheetView();
        createNewSheetViewInCurrentView();
        sortDescendingForCell(u"A1");
    }

    // Default view: 4, 5, 3, 7
    // Sheet view: 7, 5, 4, 3 (sorted)

    // Set A2 and A3 bold in sheet view
    {
        switchToSheetView();
        setCellBold(u"A2");
        setCellBold(u"A3");
    }

    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"N", u"B", u"N", u"B" }),
                         getTextWeight(pDocument, 0, 1, 4, 0));
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"B", u"B", u"N", u"N" }),
                         getTextWeight(pDocument, 0, 1, 4, 1));

    // Switch to Default View and call ClearItems on A4:A5
    {
        switchToDefaultView();

        ScMarkData aMark(pDocument->GetSheetLimits());
        aMark.SelectTable(0, true);
        aMark.SetMarkArea(ScRange(0, 3, 0, 0, 4, 0));
        sal_uInt16 aWhich[] = { ATTR_FONT_WEIGHT, 0 };
        pDocShell->GetDocFunc().ClearItems(aMark, aWhich, true);
    }

    // Default view: A4 is N so no change, A5 should change from B -> N
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"N", u"B", u"N", u"N" }),
                         getTextWeight(pDocument, 0, 1, 4, 0));

    // Sheet view: A4 -> A5 and A5 -> A2, so A2 should change from B -> N
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"N", u"B", u"N", u"N" }),
                         getTextWeight(pDocument, 0, 1, 4, 1));
}

CPPUNIT_TEST_FIXTURE(SyncTest, testSync_SheetView_ClearItemsOperation)
{
    ScModelObj* pModelObj = createDoc("SheetView_AutoFilter.ods");
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ScDocument* pDocument = pModelObj->GetDocument();
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());

    setupViews();

    // Switch to Sheet View and Create, sort descending
    {
        switchToSheetView();
        createNewSheetViewInCurrentView();
        sortDescendingForCell(u"A1");
    }

    // Default view: 4, 5, 3, 7
    // Sheet view: 7, 5, 4, 3 (sorted)

    // Set all cells bold from sheet view
    {
        switchToSheetView();
        setCellBold(u"A2:A5");
    }

    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"B", u"B", u"B", u"B" }),
                         getTextWeight(pDocument, 0, 1, 4, 0));
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"B", u"B", u"B", u"B" }),
                         getTextWeight(pDocument, 0, 1, 4, 1));

    // Call ClearItems on A3:A4 on sheet view
    {
        switchToSheetView();

        ScMarkData aMark(pDocument->GetSheetLimits());
        aMark.SelectTable(1, true);
        aMark.SetMarkArea(ScRange(0, 2, 1, 0, 3, 1));
        sal_uInt16 aWhich[] = { ATTR_FONT_WEIGHT, 0 };
        pDocShell->GetDocFunc().ClearItems(aMark, aWhich, true);
    }

    // Sheet view: A3, A4 changed from B -> N
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"B", u"N", u"N", u"B" }),
                         getTextWeight(pDocument, 0, 1, 4, 1));

    // Default view: A3 -> A3, A4 -> A2, so A2 and A3 from B -> N
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"N", u"N", u"B", u"B" }),
                         getTextWeight(pDocument, 0, 1, 4, 0));
}

CPPUNIT_TEST_FIXTURE(SyncTest, testSync_AutoFormat_DefaultAndSheetView)
{
    ScModelObj* pModelObj = createDoc("SheetView_AutoFilter.ods");
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ScDocument* pDocument = pModelObj->GetDocument();
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());

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

    // Switch to sheet view and apply AutoFormat
    {
        switchToSheetView();

        // Current state default view
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"7", u"5", u"4", u"3" }),
                             getValues(mpTabViewDefaultView, 0, 1, 4));

        // Current state sheet view
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"3", u"4", u"5", u"7" }),
                             getValues(mpTabViewSheetView, 0, 1, 4));

        // Initial text weights
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"N", u"N", u"N", u"N" }),
                             getTextWeight(pDocument, 0, 1, 4, 0));
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"N", u"N", u"N", u"N" }),
                             getTextWeight(pDocument, 0, 1, 4, 1));

        // Apply AutoFormat (format 0) to A2:A5 on default view (index 0)
        pDocShell->GetDocFunc().AutoFormat(ScRange(0, 1, 0, 0, 4, 0), nullptr, 0, true);

        // Values should remain unchanged after AutoFormat
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"7", u"5", u"4", u"3" }),
                             getValues(mpTabViewDefaultView, 0, 1, 4));
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"3", u"4", u"5", u"7" }),
                             getValues(mpTabViewSheetView, 0, 1, 4));

        // First row is defined as header row, so we can exploit that to test resorting.
        // Default view: AutoFormat applies blue to first row, gray to others
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"000080", u"cccccc", u"cccccc", u"cccccc" }),
                             getBackgroundColor(pDocument, 0, 1, 4, 0));

        // Sheet view: background colors synced and re-sorted
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"cccccc", u"cccccc", u"cccccc", u"000080" }),
                             getBackgroundColor(pDocument, 0, 1, 4, 1));
    }

    // AutoFormat on the sheet view should be blocked - input intersects autofilter
    {
        switchToSheetView();

        SCTAB nSheetViewTab = mpTabViewSheetView->GetViewData().GetTabNumber();
        bool bResult = pDocShell->GetDocFunc().AutoFormat(
            ScRange(0, 1, nSheetViewTab, 0, 4, nSheetViewTab), nullptr, 1, true);

        // Returns false because the running is blocked
        CPPUNIT_ASSERT(!bResult);

        // Background colors should remain unchanged
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"000080", u"cccccc", u"cccccc", u"cccccc" }),
                             getBackgroundColor(pDocument, 0, 1, 4, 0));
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"cccccc", u"cccccc", u"cccccc", u"000080" }),
                             getBackgroundColor(pDocument, 0, 1, 4, nSheetViewTab));
    }
}

CPPUNIT_TEST_FIXTURE(SyncTest, testSync_MultipleOps_DefaultAndSheetView)
{
    ScModelObj* pModelObj = createDoc("empty.ods");
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    ScDocument* pDocument = pModelObj->GetDocument();

    // Set up 2 views: for default view and for sheet view 1
    setupViews();

    // Add another view for sheet view 2
    SfxLokHelper::createView();
    Scheduler::ProcessEventsToIdle();
    ScTestViewCallback aSheetView2;
    int nSheetView2 = aSheetView2.getViewID();

    // Set up data, which will be duplicated to default view and sheet views
    // A1 = 1 - input cell
    // B1 = =A1*10 - formula
    // variable inputs:
    // A3 = 2
    // A4 = 3
    // A5 = 4
    pDocument->SetValue(ScAddress(0, 0, 0), 1);
    pDocument->SetString(ScAddress(1, 0, 0), u"=A1*10"_ustr);
    pDocument->SetValue(ScAddress(0, 2, 0), 2);
    pDocument->SetValue(ScAddress(0, 3, 0), 3);
    pDocument->SetValue(ScAddress(0, 4, 0), 4);

    // Check value
    CPPUNIT_ASSERT_EQUAL(10.0, pDocument->GetValue(ScAddress(1, 0, 0)));

    // Create sheet view 1
    {
        switchToSheetView();
        createNewSheetViewInCurrentView();
    }

    // Create sheet view 2
    {
        SfxLokHelper::setView(nSheetView2);
        Scheduler::ProcessEventsToIdle();
        createNewSheetViewInCurrentView();
    }

    // Perform Multiple Operations from sheet view 1
    {
        switchToSheetView();

        SCTAB nSheetViewTab = mpTabViewSheetView->GetViewData().GetTabNumber();

        ScTabOpParam aParam;
        aParam.aRefFormulaCell = ScRefAddress(1, 0, nSheetViewTab);
        aParam.aRefFormulaEnd = aParam.aRefFormulaCell;
        aParam.aRefColCell = ScRefAddress(0, 0, nSheetViewTab);
        aParam.meMode = ScTabOpParam::Column;

        // Apply Multiple Operations on A3:B5 — fills B3:B5 with 20, 30, 40
        pDocShell->GetDocFunc().TabOp(ScRange(0, 2, nSheetViewTab, 1, 4, nSheetViewTab), nullptr,
                                      aParam, true, true);

        // Verify results in default view (index 0)
        SCTAB nDefaultViewTab = mpTabViewDefaultView->GetViewData().GetTabNumber();
        CPPUNIT_ASSERT_EQUAL(u"20"_ustr, pDocument->GetString(ScAddress(1, 2, nDefaultViewTab)));
        CPPUNIT_ASSERT_EQUAL(u"30"_ustr, pDocument->GetString(ScAddress(1, 3, nDefaultViewTab)));
        CPPUNIT_ASSERT_EQUAL(u"40"_ustr, pDocument->GetString(ScAddress(1, 4, nDefaultViewTab)));

        // Verify same results in sheet view 1
        CPPUNIT_ASSERT_EQUAL(u"20"_ustr, pDocument->GetString(ScAddress(1, 2, nSheetViewTab)));
        CPPUNIT_ASSERT_EQUAL(u"30"_ustr, pDocument->GetString(ScAddress(1, 3, nSheetViewTab)));
        CPPUNIT_ASSERT_EQUAL(u"40"_ustr, pDocument->GetString(ScAddress(1, 4, nSheetViewTab)));

        // Verify same results in sheet view 2
        SCTAB nSheetView2Tab = aSheetView2.getTabViewShell()->GetViewData().GetTabNumber();
        CPPUNIT_ASSERT_EQUAL(u"20"_ustr, pDocument->GetString(ScAddress(1, 2, nSheetView2Tab)));
        CPPUNIT_ASSERT_EQUAL(u"30"_ustr, pDocument->GetString(ScAddress(1, 3, nSheetView2Tab)));
        CPPUNIT_ASSERT_EQUAL(u"40"_ustr, pDocument->GetString(ScAddress(1, 4, nSheetView2Tab)));
    }
}

CPPUNIT_TEST_FIXTURE(SyncTest, testSync_ReplaceNote_DefaultAndSheetView)
{
    ScModelObj* pModelObj = createDoc("SheetView_AutoFilter.ods");
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
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

    // Replace note from default view on C1
    {
        switchToDefaultView();

        OUString aAuthor(u"Test Author"_ustr);
        OUString aDate(u"2026-03-08"_ustr);
        pDocShell->GetDocFunc().ReplaceNote(ScAddress(2, 0, 0), u"Default Note"_ustr, &aAuthor,
                                            &aDate, true);

        // Verify note on default view
        ScPostIt* pNoteDefault = pDocument->GetNote(ScAddress(2, 0, 0));
        CPPUNIT_ASSERT(pNoteDefault);
        CPPUNIT_ASSERT_EQUAL(u"Default Note"_ustr, pNoteDefault->GetText());

        // Verify note synced to sheet view
        SCTAB nSheetViewTab = mpTabViewSheetView->GetViewData().GetTabNumber();
        ScPostIt* pNoteSheet = pDocument->GetNote(ScAddress(2, 0, nSheetViewTab));
        CPPUNIT_ASSERT(pNoteSheet);
        CPPUNIT_ASSERT_EQUAL(u"Default Note"_ustr, pNoteSheet->GetText());
    }

    // Replace note from sheet view on A2
    // Sheet view ascending: A2=3, A3=4, A4=5, A5=7
    // Default view descending: A2=7, A3=5, A4=4, A5=3
    // A2 on sheet view (value 3) maps to A5 on default view (value 3)
    {
        switchToSheetView();

        SCTAB nSheetViewTab = mpTabViewSheetView->GetViewData().GetTabNumber();

        OUString aAuthor(u"Sheet Author"_ustr);
        OUString aDate(u"2026-03-08"_ustr);
        pDocShell->GetDocFunc().ReplaceNote(ScAddress(0, 1, nSheetViewTab), u"Sheet Note"_ustr,
                                            &aAuthor, &aDate, true);

        // Verify note on default view at A5 (value 3)
        ScPostIt* pNoteDefault = pDocument->GetNote(ScAddress(0, 4, 0));
        CPPUNIT_ASSERT(pNoteDefault);
        CPPUNIT_ASSERT_EQUAL(u"Sheet Note"_ustr, pNoteDefault->GetText());

        // Verify note on sheet view at A2 (value 3)
        ScPostIt* pNoteSheet = pDocument->GetNote(ScAddress(0, 1, nSheetViewTab));
        CPPUNIT_ASSERT(pNoteSheet);
        CPPUNIT_ASSERT_EQUAL(u"Sheet Note"_ustr, pNoteSheet->GetText());
    }
}

CPPUNIT_TEST_FIXTURE(SyncTest, testSync_MoveBlock_DefaultAndSheetView)
{
    ScModelObj* pModelObj = createDoc("SheetView_AutoFilter.ods");
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
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

    // Put data in column C (outside autofilter range) on default view
    pDocument->SetString(ScAddress(2, 0, 0), u"Hello"_ustr);
    pDocument->SetValue(ScAddress(2, 1, 0), 10.0);
    pDocument->SetValue(ScAddress(2, 2, 0), 20.0);

    // Move block from default view: C1:C3 -> D1:D3 (outside autofilter)
    {
        switchToDefaultView();

        bool bMoved = pDocShell->GetDocFunc().MoveBlock(
            ScRange(2, 0, 0, 2, 2, 0), ScAddress(3, 0, 0), true, true, false, true);
        CPPUNIT_ASSERT(bMoved);

        // Verify on default view
        CPPUNIT_ASSERT_EQUAL(OUString(), pDocument->GetString(ScAddress(2, 0, 0)));
        CPPUNIT_ASSERT_EQUAL(u"Hello"_ustr, pDocument->GetString(ScAddress(3, 0, 0)));
        CPPUNIT_ASSERT_EQUAL(10.0, pDocument->GetValue(ScAddress(3, 1, 0)));
        CPPUNIT_ASSERT_EQUAL(20.0, pDocument->GetValue(ScAddress(3, 2, 0)));

        // Verify synced on sheet view
        SCTAB nSheetViewTab = mpTabViewSheetView->GetViewData().GetTabNumber();
        CPPUNIT_ASSERT_EQUAL(OUString(), pDocument->GetString(ScAddress(2, 0, nSheetViewTab)));
        CPPUNIT_ASSERT_EQUAL(u"Hello"_ustr, pDocument->GetString(ScAddress(3, 0, nSheetViewTab)));
        CPPUNIT_ASSERT_EQUAL(10.0, pDocument->GetValue(ScAddress(3, 1, nSheetViewTab)));
        CPPUNIT_ASSERT_EQUAL(20.0, pDocument->GetValue(ScAddress(3, 2, nSheetViewTab)));
    }

    // Move block from sheet view: D1:D3 -> E1:E3 (outside autofilter)
    {
        switchToSheetView();

        SCTAB nSheetViewTab = mpTabViewSheetView->GetViewData().GetTabNumber();

        bool bMoved = pDocShell->GetDocFunc().MoveBlock(
            ScRange(3, 0, nSheetViewTab, 3, 2, nSheetViewTab), ScAddress(4, 0, nSheetViewTab), true,
            true, false, true);
        CPPUNIT_ASSERT(bMoved);

        // Verify on default view
        CPPUNIT_ASSERT_EQUAL(OUString(), pDocument->GetString(ScAddress(3, 0, 0)));
        CPPUNIT_ASSERT_EQUAL(u"Hello"_ustr, pDocument->GetString(ScAddress(4, 0, 0)));
        CPPUNIT_ASSERT_EQUAL(10.0, pDocument->GetValue(ScAddress(4, 1, 0)));
        CPPUNIT_ASSERT_EQUAL(20.0, pDocument->GetValue(ScAddress(4, 2, 0)));

        // Verify synced on sheet view
        CPPUNIT_ASSERT_EQUAL(OUString(), pDocument->GetString(ScAddress(3, 0, nSheetViewTab)));
        CPPUNIT_ASSERT_EQUAL(u"Hello"_ustr, pDocument->GetString(ScAddress(4, 0, nSheetViewTab)));
        CPPUNIT_ASSERT_EQUAL(10.0, pDocument->GetValue(ScAddress(4, 1, nSheetViewTab)));
        CPPUNIT_ASSERT_EQUAL(20.0, pDocument->GetValue(ScAddress(4, 2, nSheetViewTab)));
    }

    // Move block from default view inside autofilter: A2:A3 -> C2:C3
    {
        switchToDefaultView();

        bool bMoved = pDocShell->GetDocFunc().MoveBlock(
            ScRange(0, 1, 0, 0, 2, 0), ScAddress(2, 1, 0), true, true, false, true);
        CPPUNIT_ASSERT(bMoved);

        // Verify A2:A3 cleared and C2:C3 has values on default view
        CPPUNIT_ASSERT_EQUAL(OUString(), pDocument->GetString(ScAddress(0, 1, 0)));
        CPPUNIT_ASSERT_EQUAL(OUString(), pDocument->GetString(ScAddress(0, 2, 0)));
        CPPUNIT_ASSERT_EQUAL(7.0, pDocument->GetValue(ScAddress(2, 1, 0)));
        CPPUNIT_ASSERT_EQUAL(5.0, pDocument->GetValue(ScAddress(2, 2, 0)));

        // Verify autofilter column on both views
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"", u"", u"4", u"3" }),
                             getValues(mpTabViewDefaultView, 0, 1, 4));
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"3", u"4", u"", u"" }),
                             getValues(mpTabViewSheetView, 0, 1, 4));
    }

    // Undo the move inside autofilter
    {
        switchToDefaultView();
        undo();

        // Verify autofilter values restored
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"7", u"5", u"4", u"3" }),
                             getValues(mpTabViewDefaultView, 0, 1, 4));
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"3", u"4", u"5", u"7" }),
                             getValues(mpTabViewSheetView, 0, 1, 4));
    }

    // Move block on the sheet view should be blocked as source intersects autofilter
    {
        switchToSheetView();

        SCTAB nSheetViewTab = mpTabViewSheetView->GetViewData().GetTabNumber();

        // Try to move A2:A3 -> C2:C3 on sheet view tab
        bool bResult = pDocShell->GetDocFunc().MoveBlock(
            ScRange(0, 1, nSheetViewTab, 0, 2, nSheetViewTab), ScAddress(2, 1, nSheetViewTab), true,
            true, false, true);
        CPPUNIT_ASSERT(!bResult);

        // Values should remain unchanged
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"7", u"5", u"4", u"3" }),
                             getValues(mpTabViewDefaultView, 0, 1, 4));
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"3", u"4", u"5", u"7" }),
                             getValues(mpTabViewSheetView, 0, 1, 4));
    }
}

CPPUNIT_TEST_FIXTURE(SyncTest, testSync_EnterMatrix_DefaultAndSheetView)
{
    ScModelObj* pModelObj = createDoc("SheetView_AutoFilter.ods");
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());

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

    // Switch to sheet view and enter matrix formulas
    {
        switchToSheetView();

        // Current state default view
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"7", u"5", u"4", u"3" }),
                             getValues(mpTabViewDefaultView, 0, 1, 4));

        // Current state sheet view
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"3", u"4", u"5", u"7" }),
                             getValues(mpTabViewSheetView, 0, 1, 4));

        // Modify Sheet View A3 : 4 -> ={14} (matrix formula)
        pDocShell->GetDocFunc().EnterMatrix(ScRange(0, 2, 1, 0, 2, 1), nullptr, nullptr,
                                            u"={14}"_ustr, true, true, OUString(),
                                            formula::FormulaGrammar::GRAM_ENGLISH_XL_OOX);

        // Default view: translated A3 to A4 : 4 -> 14
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"7", u"5", u"14", u"3" }),
                             getValues(mpTabViewDefaultView, 0, 1, 4));

        // Sheet view: A3 : 4 -> 14, and resort
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"3", u"5", u"7", u"14" }),
                             getValues(mpTabViewSheetView, 0, 1, 4));

        // Modify Default view A2 : 7 -> ={17} (matrix formula)
        pDocShell->GetDocFunc().EnterMatrix(ScRange(0, 1, 0, 0, 1, 0), nullptr, nullptr,
                                            u"={17}"_ustr, true, true, OUString(),
                                            formula::FormulaGrammar::GRAM_ENGLISH_XL_OOX);

        // Default view: A2 : 7 -> 17
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"17", u"5", u"14", u"3" }),
                             getValues(mpTabViewDefaultView, 0, 1, 4));

        // Sheet view: translated A2 to A4 : 7 -> 17, and resort
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"3", u"5", u"14", u"17" }),
                             getValues(mpTabViewSheetView, 0, 1, 4));
    }
}

CPPUNIT_TEST_FIXTURE(SyncTest, testSync_FillSimple_DefaultAndSheetView)
{
    ScModelObj* pModelObj = createDoc("SheetView_AutoFilter.ods");
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());

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

    // Switch to sheet view and fill simple
    {
        switchToSheetView();

        // Current state default view
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"7", u"5", u"4", u"3" }),
                             getValues(mpTabViewDefaultView, 0, 1, 4));

        // Current state sheet view
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"3", u"4", u"5", u"7" }),
                             getValues(mpTabViewSheetView, 0, 1, 4));

        // FillSimple A2:A3 on default view (index 0), FILL_TO_BOTTOM
        // Copies A2 to A3 (value 7)
        pDocShell->GetDocFunc().FillSimple(ScRange(0, 1, 0, 0, 2, 0), nullptr, FILL_TO_BOTTOM,
                                           true);

        // Default view: A3 changed from 5 -> 7
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"7", u"7", u"4", u"3" }),
                             getValues(mpTabViewDefaultView, 0, 1, 4));

        // Sheet view: synced and re-sorted
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"3", u"4", u"7", u"7" }),
                             getValues(mpTabViewSheetView, 0, 1, 4));
    }

    // Try to FillSimple on the sheet view — should be blocked because it intersects autofilter
    {
        switchToSheetView();

        SCTAB nSheetViewTab = mpTabViewSheetView->GetViewData().GetTabNumber();
        bool bResult = pDocShell->GetDocFunc().FillSimple(
            ScRange(0, 1, nSheetViewTab, 0, 2, nSheetViewTab), nullptr, FILL_TO_BOTTOM, true);
        CPPUNIT_ASSERT(!bResult);

        // Values should remain unchanged
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"7", u"7", u"4", u"3" }),
                             getValues(mpTabViewDefaultView, 0, 1, 4));
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"3", u"4", u"7", u"7" }),
                             getValues(mpTabViewSheetView, 0, 1, 4));
    }
}

CPPUNIT_TEST_FIXTURE(SyncTest, testSync_FillSeries_DefaultAndSheetView)
{
    ScModelObj* pModelObj = createDoc("SheetView_AutoFilter.ods");
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());

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

    // Switch to sheet view and fill series
    {
        switchToSheetView();

        // Current state default view
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"7", u"5", u"4", u"3" }),
                             getValues(mpTabViewDefaultView, 0, 1, 4));

        // Current state sheet view
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"3", u"4", u"5", u"7" }),
                             getValues(mpTabViewSheetView, 0, 1, 4));

        // FillSeries A2:A5 on default view (index 0), FILL_TO_BOTTOM, FILL_LINEAR
        // start=40, step=-10: fills 40, 30, 20, 10
        pDocShell->GetDocFunc().FillSeries(ScRange(0, 1, 0, 0, 4, 0), nullptr, FILL_TO_BOTTOM,
                                           FILL_LINEAR, FILL_DAY, 40.0, -10.0, 0.0, true);

        // Default view
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"40", u"30", u"20", u"10" }),
                             getValues(mpTabViewDefaultView, 0, 1, 4));

        // Sheet view: synced and re-sorted
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"10", u"20", u"30", u"40" }),
                             getValues(mpTabViewSheetView, 0, 1, 4));
    }

    // Try to FillSeries on the sheet view — should be blocked because it intersects autofilter
    {
        switchToSheetView();

        SCTAB nSheetViewTab = mpTabViewSheetView->GetViewData().GetTabNumber();
        bool bResult = pDocShell->GetDocFunc().FillSeries(
            ScRange(0, 1, nSheetViewTab, 0, 4, nSheetViewTab), nullptr, FILL_TO_BOTTOM, FILL_LINEAR,
            FILL_DAY, 100.0, 100.0, 0.0, true);
        CPPUNIT_ASSERT(!bResult);

        // Values should remain unchanged
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"40", u"30", u"20", u"10" }),
                             getValues(mpTabViewDefaultView, 0, 1, 4));
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"10", u"20", u"30", u"40" }),
                             getValues(mpTabViewSheetView, 0, 1, 4));
    }
}

CPPUNIT_TEST_FIXTURE(SyncTest, testSync_FillAuto_DefaultAndSheetView)
{
    ScModelObj* pModelObj = createDoc("SheetView_AutoFilter.ods");
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    ScDocument& rDocument = pDocShell->GetDocument();

    // Put data in column C (outside autofilter range) on the default view
    rDocument.SetValue(ScAddress(2, 0, 0), 1.0);
    rDocument.SetValue(ScAddress(2, 1, 0), 2.0);

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

    // FillAuto on the default view inside autofilter
    {
        switchToDefaultView();

        // Current state default view
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"7", u"5", u"4", u"3" }),
                             getValues(mpTabViewDefaultView, 0, 1, 4));

        // Current state sheet view
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"3", u"4", u"5", u"7" }),
                             getValues(mpTabViewSheetView, 0, 1, 4));

        // FillAuto source A2:A3 on default view (index 0), FILL_TO_BOTTOM, nCount=2
        // Source: 7, 5 (step -2), fills A4:A5 with 3, 1
        ScRange aFillRange(0, 1, 0, 0, 2, 0);
        pDocShell->GetDocFunc().FillAuto(aFillRange, nullptr, FILL_TO_BOTTOM, FILL_AUTO, FILL_DAY,
                                         2, 1.0, 100.0, true, true);

        // Default view: 7, 5, 3, 1
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"7", u"5", u"3", u"1" }),
                             getValues(mpTabViewDefaultView, 0, 1, 4));

        // Sheet view: synced and re-sorted
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"1", u"3", u"5", u"7" }),
                             getValues(mpTabViewSheetView, 0, 1, 4));
    }

    // Try to FillAuto on the sheet view — should be blocked because it intersects autofilter
    {
        switchToSheetView();

        SCTAB nSheetViewTab = mpTabViewSheetView->GetViewData().GetTabNumber();
        ScRange aFillRange(0, 1, nSheetViewTab, 0, 2, nSheetViewTab);
        bool bResult = pDocShell->GetDocFunc().FillAuto(
            aFillRange, nullptr, FILL_TO_BOTTOM, FILL_AUTO, FILL_DAY, 2, 1.0, 100.0, true, true);
        CPPUNIT_ASSERT(!bResult);

        // Values should remain unchanged
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"7", u"5", u"3", u"1" }),
                             getValues(mpTabViewDefaultView, 0, 1, 4));
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"1", u"3", u"5", u"7" }),
                             getValues(mpTabViewSheetView, 0, 1, 4));
    }

    // FillAuto from sheet view outside autofilter (column C) should work
    {
        switchToSheetView();

        SCTAB nSheetViewTab = mpTabViewSheetView->GetViewData().GetTabNumber();

        ScRange aFillRange(2, 0, nSheetViewTab, 2, 1, nSheetViewTab);
        bool bResult = pDocShell->GetDocFunc().FillAuto(
            aFillRange, nullptr, FILL_TO_BOTTOM, FILL_AUTO, FILL_DAY, 2, 1.0, 100.0, true, true);
        CPPUNIT_ASSERT(bResult);

        // Verify the returned range is on the sheet view tab,
        // because the caller uses it for MarkRange on the current view
        CPPUNIT_ASSERT_EQUAL(nSheetViewTab, aFillRange.aStart.Tab());
        CPPUNIT_ASSERT_EQUAL(nSheetViewTab, aFillRange.aEnd.Tab());

        // Verify fill result on default view: C1=1, C2=2, C3=3, C4=4
        CPPUNIT_ASSERT_EQUAL(3.0, rDocument.GetValue(ScAddress(2, 2, 0)));
        CPPUNIT_ASSERT_EQUAL(4.0, rDocument.GetValue(ScAddress(2, 3, 0)));

        // Verify synced to sheet view
        CPPUNIT_ASSERT_EQUAL(3.0, rDocument.GetValue(ScAddress(2, 2, nSheetViewTab)));
        CPPUNIT_ASSERT_EQUAL(4.0, rDocument.GetValue(ScAddress(2, 3, nSheetViewTab)));
    }
}

CPPUNIT_TEST_FIXTURE(SyncTest, testSync_TransliterateText_DefaultAndSheetView)
{
    ScModelObj* pModelObj = createDoc("SheetView_AutoFilter_Extended.ods");
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ScDocument* pDocument = pModelObj->GetDocument();
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());

    setupViews();

    // Create new sheet view and sort ascending
    {
        switchToSheetView();
        createNewSheetViewInCurrentView();
        sortAscendingForCell(u"A1");
    }

    // Sort descending in default view
    {
        switchToDefaultView();
        sortDescendingForCell(u"A1");
    }

    // Default view at Column B: aaa, rrr, ccc, sss
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"aaa", u"rrr", u"ccc", u"sss" }),
                         getValues(mpTabViewDefaultView, 1, 1, 4));
    // Sheet view at Column B: sss, ccc, rrr, aaa
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"sss", u"ccc", u"rrr", u"aaa" }),
                         getValues(mpTabViewSheetView, 1, 1, 4));

    // Transliterate B2:B5 to uppercase from sheet view
    {
        switchToSheetView();

        SCTAB nSheetViewTab = mpTabViewSheetView->GetViewData().GetTabNumber();
        ScMarkData aMark(pDocument->GetSheetLimits());
        aMark.SelectTable(nSheetViewTab, true);
        aMark.SetMarkArea(ScRange(1, 1, nSheetViewTab, 1, 4, nSheetViewTab));
        pDocShell->GetDocFunc().TransliterateText(aMark, TransliterationFlags::LOWERCASE_UPPERCASE,
                                                  true);
    }

    // Sheet view: text is uppercased, ascending order
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"SSS", u"CCC", u"RRR", u"AAA" }),
                         getValues(mpTabViewSheetView, 1, 1, 4));

    // Default view: synced, descending order, uppercased
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"AAA", u"RRR", u"CCC", u"SSS" }),
                         getValues(mpTabViewDefaultView, 1, 1, 4));
}

CPPUNIT_TEST_FIXTURE(SyncTest, testSync_ConvertFormulaToValue_DefaultAndSheetView)
{
    ScModelObj* pModelObj = createDoc("empty.ods");
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ScDocument* pDocument = pModelObj->GetDocument();
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());

    // Set up formulas
    pDocument->SetString(ScAddress(0, 0, 0), u"=10+20"_ustr);
    pDocument->SetString(ScAddress(0, 1, 0), u"=30+40"_ustr);

    setupViews();

    // Create sheet view
    {
        switchToSheetView();
        createNewSheetViewInCurrentView();
    }

    SCTAB nSheetViewTab = mpTabViewSheetView->GetViewData().GetTabNumber();

    // Verify formulas exist on both tabs
    CPPUNIT_ASSERT(pDocument->GetFormulaCell(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT(pDocument->GetFormulaCell(ScAddress(0, 1, 0)));
    CPPUNIT_ASSERT(pDocument->GetFormulaCell(ScAddress(0, 0, nSheetViewTab)));
    CPPUNIT_ASSERT(pDocument->GetFormulaCell(ScAddress(0, 1, nSheetViewTab)));

    // Convert formulas to values from sheet view
    {
        switchToSheetView();
        pDocShell->GetDocFunc().ConvertFormulaToValue(
            ScRange(0, 0, nSheetViewTab, 0, 1, nSheetViewTab), false);
    }

    // Sheet view: formula converted, values preserved
    CPPUNIT_ASSERT(!pDocument->GetFormulaCell(ScAddress(0, 0, nSheetViewTab)));
    CPPUNIT_ASSERT(!pDocument->GetFormulaCell(ScAddress(0, 1, nSheetViewTab)));
    CPPUNIT_ASSERT_EQUAL(30.0, pDocument->GetValue(ScAddress(0, 0, nSheetViewTab)));
    CPPUNIT_ASSERT_EQUAL(70.0, pDocument->GetValue(ScAddress(0, 1, nSheetViewTab)));

    // Default view: synced — formulas also converted
    CPPUNIT_ASSERT(!pDocument->GetFormulaCell(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT(!pDocument->GetFormulaCell(ScAddress(0, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(30.0, pDocument->GetValue(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(70.0, pDocument->GetValue(ScAddress(0, 1, 0)));
}

CPPUNIT_TEST_FIXTURE(SyncTest, testSync_SetNoteText_DefaultAndSheetView)
{
    ScModelObj* pModelObj = createDoc("SheetView_AutoFilter.ods");
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ScDocument* pDocument = pModelObj->GetDocument();
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());

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

    // Default view descending: 7, 5, 4, 3
    // Sheet view ascending: 3, 4, 5, 7

    // Set note on A2 in sheet view
    {
        switchToSheetView();

        SCTAB nSheetViewTab = mpTabViewSheetView->GetViewData().GetTabNumber();
        pDocShell->GetDocFunc().SetNoteText(ScAddress(0, 1, nSheetViewTab), u"Hello Note"_ustr,
                                            true);
    }

    // Sheet view: note on A2
    SCTAB nSheetViewTab = mpTabViewSheetView->GetViewData().GetTabNumber();
    ScPostIt* pNoteSheet = pDocument->GetNote(ScAddress(0, 1, nSheetViewTab));
    CPPUNIT_ASSERT(pNoteSheet);
    CPPUNIT_ASSERT_EQUAL(u"Hello Note"_ustr, pNoteSheet->GetText());

    // Default view: note synced to the cell A5
    ScPostIt* pNoteDefault = pDocument->GetNote(ScAddress(0, 4, 0));
    CPPUNIT_ASSERT(pNoteDefault);
    CPPUNIT_ASSERT_EQUAL(u"Hello Note"_ustr, pNoteDefault->GetText());
}

CPPUNIT_TEST_FIXTURE(SyncTest, testSync_ChangeIndent_DefaultAndSheetView)
{
    ScModelObj* pModelObj = createDoc("SheetView_AutoFilter.ods");
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ScDocument* pDocument = pModelObj->GetDocument();
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());

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

    // Switch to Sheet view and change indent on A2:A3
    // Sheet view ascending: 3, 4, 5, 7
    {
        switchToSheetView();

        SCTAB nSheetViewTab = mpTabViewSheetView->GetViewData().GetTabNumber();
        ScMarkData aMark(pDocument->GetSheetLimits());
        aMark.SelectTable(nSheetViewTab, true);
        aMark.SetMarkArea(ScRange(0, 1, nSheetViewTab, 0, 2, nSheetViewTab));
        aMark.MarkToMulti();
        pDocShell->GetDocFunc().ChangeIndent(aMark, true, true);
    }

    // Sheet view: A2,A3 indented, A4,A5 not
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"3", u"4", u"5", u"7" }),
                         getValues(pDocument, 0, 1, 4, 1));
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"Y", u"Y", u"N", u"N" }),
                         getIndent(pDocument, 0, 1, 4, 1));

    // Default view: A4,A5 indented (A4 is A3, A5 is A2)
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"7", u"5", u"4", u"3" }),
                         getValues(pDocument, 0, 1, 4, 0));
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"N", u"N", u"Y", u"Y" }),
                         getIndent(pDocument, 0, 1, 4, 0));
}

CPPUNIT_TEST_FIXTURE(SyncTest, testUndo_DefaultView_DeleteContent)
{
    // Test undo of DeleteContents from the default view with a sheet view present.
    // After undo, both views should be consistent.

    ScModelObj* pModelObj = createDoc("SheetView_AutoFilter.ods");
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ScDocument* pDocument = pModelObj->GetDocument();

    setupViews();

    // Switch to Sheet View and Create, sort descending
    {
        switchToSheetView();
        createNewSheetViewInCurrentView();
        sortDescendingForCell(u"A1");
    }

    // Default view and and sheet view state
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"3", u"7" }),
                         getValues(pDocument, 0, 1, 4, 0));
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"7", u"5", u"4", u"3" }),
                         getValues(pDocument, 0, 1, 4, 1));

    // Clear content in default view
    {
        switchToDefaultView();
        gotoCell(u"A4");
        dispatchCommand(mxComponent, u".uno:ClearContents"_ustr, {});
    }

    // State after delete
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"", u"7" }),
                         getValues(pDocument, 0, 1, 4, 0));
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"7", u"5", u"4", u"" }),
                         getValues(pDocument, 0, 1, 4, 1));

    // Undo from default view
    {
        switchToDefaultView();
        undo();
    }

    // After undo: default should be restored
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"3", u"7" }),
                         getValues(pDocument, 0, 1, 4, 0));

    // Sheet view should also be synced back
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"7", u"5", u"4", u"3" }),
                         getValues(pDocument, 0, 1, 4, 1));

    // Redo from default view
    {
        switchToDefaultView();
        redo();
    }

    // State after delete
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"", u"7" }),
                         getValues(pDocument, 0, 1, 4, 0));
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"7", u"5", u"4", u"" }),
                         getValues(pDocument, 0, 1, 4, 1));

    // Undo from default view
    {
        switchToDefaultView();
        undo();
    }

    // Default view and and sheet view state
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"3", u"7" }),
                         getValues(pDocument, 0, 1, 4, 0));
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"7", u"5", u"4", u"3" }),
                         getValues(pDocument, 0, 1, 4, 1));

    // Clear content in sheet view
    {
        switchToSheetView();
        gotoCell(u"A4");
        dispatchCommand(mxComponent, u".uno:ClearContents"_ustr, {});
    }

    // State after delete
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"", u"5", u"3", u"7" }),
                         getValues(pDocument, 0, 1, 4, 0));
    // Sheet view was resorted
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"7", u"5", u"3", u"" }),
                         getValues(pDocument, 0, 1, 4, 1));

    // Undo from sheet view
    {
        switchToSheetView();
        undo();
    }

    // Default view and and sheet view state
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"3", u"7" }),
                         getValues(pDocument, 0, 1, 4, 0));
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"7", u"5", u"4", u"3" }),
                         getValues(pDocument, 0, 1, 4, 1));

    // Redo from sheet view
    {
        switchToSheetView();
        redo();
    }

    // State after delete
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"", u"5", u"3", u"7" }),
                         getValues(pDocument, 0, 1, 4, 0));
    // Sheet view was resorted
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"7", u"5", u"3", u"" }),
                         getValues(pDocument, 0, 1, 4, 1));
}

CPPUNIT_TEST_FIXTURE(SyncTest, testUndo_DefaultView_ClearItems)
{
    // Test undo of ClearItems (bold removal) from the default view with a sheet view present.
    ScModelObj* pModelObj = createDoc("SheetView_AutoFilter.ods");
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ScDocument* pDocument = pModelObj->GetDocument();
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());

    setupViews();

    // Switch to Sheet View and Create, sort descending
    {
        switchToSheetView();
        createNewSheetViewInCurrentView();
        sortDescendingForCell(u"A1");
    }

    // Set A2 and A3 bold in sheet view (values 7 and 5)
    {
        switchToSheetView();
        setCellBold(u"A2");
        setCellBold(u"A3");
    }

    // Default: N, B, N, B (rows with 5 and 7 are bold)
    // Sheet: B, B, N, N
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"N", u"B", u"N", u"B" }),
                         getTextWeight(pDocument, 0, 1, 4, 0));
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"B", u"B", u"N", u"N" }),
                         getTextWeight(pDocument, 0, 1, 4, 1));

    // Clear bold on A4:A5 in default view (values 3 and 7)
    {
        switchToDefaultView();

        ScMarkData aMark(pDocument->GetSheetLimits());
        aMark.SelectTable(0, true);
        aMark.SetMarkArea(ScRange(0, 3, 0, 0, 4, 0));
        sal_uInt16 aWhich[] = { ATTR_FONT_WEIGHT, 0 };
        pDocShell->GetDocFunc().ClearItems(aMark, aWhich, true);
    }

    // After clear: A5(7) was bold -> now N
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"N", u"B", u"N", u"N" }),
                         getTextWeight(pDocument, 0, 1, 4, 0));
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"N", u"B", u"N", u"N" }),
                         getTextWeight(pDocument, 0, 1, 4, 1));

    // Undo from default view
    {
        switchToDefaultView();
        undo();
    }

    // After undo: default should be restored
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"N", u"B", u"N", u"B" }),
                         getTextWeight(pDocument, 0, 1, 4, 0));

    // Sheet view should also be synced back
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"B", u"B", u"N", u"N" }),
                         getTextWeight(pDocument, 0, 1, 4, 1));

    // Redo from default view
    {
        switchToDefaultView();
        redo();
    }

    // After redo: same as after clear
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"N", u"B", u"N", u"N" }),
                         getTextWeight(pDocument, 0, 1, 4, 0));
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"N", u"B", u"N", u"N" }),
                         getTextWeight(pDocument, 0, 1, 4, 1));

    // Undo from default view
    {
        switchToDefaultView();
        undo();
    }

    // Default view and sheet view state restored
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"N", u"B", u"N", u"B" }),
                         getTextWeight(pDocument, 0, 1, 4, 0));
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"B", u"B", u"N", u"N" }),
                         getTextWeight(pDocument, 0, 1, 4, 1));

    // Clear bold on A2:A3 in sheet view
    {
        switchToSheetView();

        SCTAB nSheetViewTab = mpTabViewSheetView->GetViewData().GetTabNumber();
        ScMarkData aMark(pDocument->GetSheetLimits());
        aMark.SelectTable(nSheetViewTab, true);
        aMark.SetMarkArea(ScRange(0, 1, nSheetViewTab, 0, 2, nSheetViewTab));
        sal_uInt16 aWhich[] = { ATTR_FONT_WEIGHT, 0 };
        pDocShell->GetDocFunc().ClearItems(aMark, aWhich, true);
    }

    // After clear from sheet view: all bold cleared
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"N", u"N", u"N", u"N" }),
                         getTextWeight(pDocument, 0, 1, 4, 0));
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"N", u"N", u"N", u"N" }),
                         getTextWeight(pDocument, 0, 1, 4, 1));

    // Undo from sheet view
    {
        switchToSheetView();
        undo();
    }

    // Default view and sheet view state restored
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"N", u"B", u"N", u"B" }),
                         getTextWeight(pDocument, 0, 1, 4, 0));
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"B", u"B", u"N", u"N" }),
                         getTextWeight(pDocument, 0, 1, 4, 1));

    // Redo from sheet view
    {
        switchToSheetView();
        redo();
    }

    // After redo: all bold cleared
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"N", u"N", u"N", u"N" }),
                         getTextWeight(pDocument, 0, 1, 4, 0));
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"N", u"N", u"N", u"N" }),
                         getTextWeight(pDocument, 0, 1, 4, 1));
}

CPPUNIT_TEST_FIXTURE(SyncTest, testUndo_DefaultView_FillSeries)
{
    // Test undo of FillSeries from default view with a sheet view present.

    ScModelObj* pModelObj = createDoc("SheetView_AutoFilter.ods");
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());

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

    // Current statw default view and sheet view
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"7", u"5", u"4", u"3" }),
                         getValues(mpTabViewDefaultView, 0, 1, 4));
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"3", u"4", u"5", u"7" }),
                         getValues(mpTabViewSheetView, 0, 1, 4));

    // FillSeries: start=40, step=-10 -> 40, 30, 20, 10
    {
        switchToDefaultView();
        pDocShell->GetDocFunc().FillSeries(ScRange(0, 1, 0, 0, 4, 0), nullptr, FILL_TO_BOTTOM,
                                           FILL_LINEAR, FILL_DAY, 40.0, -10.0, 0.0, true);
    }

    // After fill: the default view should have the decreasing values
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"40", u"30", u"20", u"10" }),
                         getValues(mpTabViewDefaultView, 0, 1, 4));
    // Sheet view has ascending sorting
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"10", u"20", u"30", u"40" }),
                         getValues(mpTabViewSheetView, 0, 1, 4));

    // Undo from default view
    {
        switchToDefaultView();
        undo();
    }

    // After undo: default and sheet view should be restored
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"7", u"5", u"4", u"3" }),
                         getValues(mpTabViewDefaultView, 0, 1, 4));
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"3", u"4", u"5", u"7" }),
                         getValues(mpTabViewSheetView, 0, 1, 4));

    // Redo from default view
    {
        switchToDefaultView();
        redo();
    }

    // Values are back
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"40", u"30", u"20", u"10" }),
                         getValues(mpTabViewDefaultView, 0, 1, 4));
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"10", u"20", u"30", u"40" }),
                         getValues(mpTabViewSheetView, 0, 1, 4));
}

CPPUNIT_TEST_FIXTURE(SyncTest, testSync_MergeCells_DefaultAndSheetView)
{
    ScModelObj* pModelObj = createDoc("empty.ods");
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    ScDocument* pDocument = pModelObj->GetDocument();

    // Set up 3 views: default view, sheet view 1, sheet view 2
    setupViews();

    SfxLokHelper::createView();
    Scheduler::ProcessEventsToIdle();
    ScTestViewCallback aView3;
    int nView3ID = aView3.getViewID();

    // Put data in cells that will be merged
    pDocument->SetString(ScAddress(0, 0, 0), u"Hello"_ustr);
    pDocument->SetValue(ScAddress(1, 0, 0), 42.0);

    // Create sheet view 1
    {
        switchToSheetView();
        createNewSheetViewInCurrentView();
    }

    // Create sheet view 2
    {
        SfxLokHelper::setView(nView3ID);
        Scheduler::ProcessEventsToIdle();
        createNewSheetViewInCurrentView();
    }

    // Merge cells A1:B2 from sheet view 1
    {
        switchToSheetView();

        SCTAB nSheetViewTab = mpTabViewSheetView->GetViewData().GetTabNumber();

        ScCellMergeOption aMergeOption(0, 0, 1, 1);
        aMergeOption.maTabs.insert(nSheetViewTab);

        bool bMerged = pDocShell->GetDocFunc().MergeCells(aMergeOption, false, true, true);
        CPPUNIT_ASSERT(bMerged);

        // Verify on default view: A1 should have the content, merge flag set
        CPPUNIT_ASSERT_EQUAL(u"Hello"_ustr, pDocument->GetString(ScAddress(0, 0, 0)));
        CPPUNIT_ASSERT(pDocument->HasAttrib(0, 0, 0, 1, 1, 0,
                                            HasAttrFlags::Merged | HasAttrFlags::Overlapped));

        // Verify on sheet view 1
        CPPUNIT_ASSERT_EQUAL(u"Hello"_ustr, pDocument->GetString(ScAddress(0, 0, nSheetViewTab)));
        CPPUNIT_ASSERT(pDocument->HasAttrib(0, 0, nSheetViewTab, 1, 1, nSheetViewTab,
                                            HasAttrFlags::Merged | HasAttrFlags::Overlapped));

        // Verify on sheet view 2
        SCTAB nSheetView2Tab = aView3.getTabViewShell()->GetViewData().GetTabNumber();
        CPPUNIT_ASSERT_EQUAL(u"Hello"_ustr, pDocument->GetString(ScAddress(0, 0, nSheetView2Tab)));
        CPPUNIT_ASSERT(pDocument->HasAttrib(0, 0, nSheetView2Tab, 1, 1, nSheetView2Tab,
                                            HasAttrFlags::Merged | HasAttrFlags::Overlapped));
    }
}

CPPUNIT_TEST_FIXTURE(SyncTest, testSync_InsertNameList_DefaultAndSheetView)
{
    // Insert name list inserts the list of the named ranges into the sheet.
    // We should sync that to all sheet views
    ScModelObj* pModelObj = createDoc("empty.ods");
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    ScDocument* pDocument = pModelObj->GetDocument();

    // Set up 3 views: default view, sheet view 1, sheet view 2
    setupViews();

    SfxLokHelper::createView();
    Scheduler::ProcessEventsToIdle();
    ScTestViewCallback aView3;
    int nView3ID = aView3.getViewID();

    // Add a named range so InsertNameList has something to insert
    ScRangeName* pGlobalNames = pDocument->GetRangeName();
    ScRangeData* pRangeData = new ScRangeData(*pDocument, u"TestRange"_ustr, u"$A$10:$B$20"_ustr);
    pGlobalNames->insert(pRangeData);

    // Create sheet view 1
    {
        switchToSheetView();
        createNewSheetViewInCurrentView();
    }

    // Create sheet view 2
    {
        SfxLokHelper::setView(nView3ID);
        Scheduler::ProcessEventsToIdle();
        createNewSheetViewInCurrentView();
    }

    // Insert name list from sheet view 1 at A1
    {
        switchToSheetView();

        SCTAB nSheetViewTab = mpTabViewSheetView->GetViewData().GetTabNumber();

        bool bInserted
            = pDocShell->GetDocFunc().InsertNameList(ScAddress(0, 0, nSheetViewTab), true);
        CPPUNIT_ASSERT(bInserted);

        // Verify on default view: A1 should have the range name
        CPPUNIT_ASSERT_EQUAL(u"TestRange"_ustr, pDocument->GetString(ScAddress(0, 0, 0)));

        // Verify synced to sheet views
        CPPUNIT_ASSERT_EQUAL(u"TestRange"_ustr,
                             pDocument->GetString(ScAddress(0, 0, nSheetViewTab)));

        SCTAB nSheetView2Tab = aView3.getTabViewShell()->GetViewData().GetTabNumber();
        CPPUNIT_ASSERT_EQUAL(u"TestRange"_ustr,
                             pDocument->GetString(ScAddress(0, 0, nSheetView2Tab)));
    }
}

CPPUNIT_TEST_FIXTURE(SyncTest, testSync_SubTotals_DefaultAndSheetView)
{
    ScModelObj* pModelObj = createDoc("empty.ods");
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    ScDocument* pDocument = pModelObj->GetDocument();

    // Set up 3 views: default view, sheet view 1, sheet view 2
    setupViews();

    SfxLokHelper::createView();
    Scheduler::ProcessEventsToIdle();
    ScTestViewCallback aView3;
    int nView3ID = aView3.getViewID();

    // Set up data with headers and values for subtotals
    // Column A: Category, Column B: Values
    pDocument->SetString(ScAddress(0, 0, 0), u"Category"_ustr);
    pDocument->SetString(ScAddress(1, 0, 0), u"Value"_ustr);
    pDocument->SetString(ScAddress(0, 1, 0), u"A"_ustr);
    pDocument->SetString(ScAddress(0, 2, 0), u"A"_ustr);
    pDocument->SetString(ScAddress(0, 3, 0), u"B"_ustr);
    pDocument->SetValue(ScAddress(1, 1, 0), 10.0);
    pDocument->SetValue(ScAddress(1, 2, 0), 20.0);
    pDocument->SetValue(ScAddress(1, 3, 0), 30.0);

    // Create a DB area for the data range
    ScDBData* pDBData = new ScDBData(u"TestDB"_ustr, 0, 0, 0, 1, 3);
    pDocument->GetDBCollection()->getNamedDBs().insert(std::unique_ptr<ScDBData>(pDBData));

    // Create sheet view 1
    {
        switchToSheetView();
        createNewSheetViewInCurrentView();
    }

    // Create sheet view 2
    {
        SfxLokHelper::setView(nView3ID);
        Scheduler::ProcessEventsToIdle();
        createNewSheetViewInCurrentView();
    }

    // Run subtotals from sheet view 1
    {
        switchToSheetView();

        SCTAB nSheetViewTab = mpTabViewSheetView->GetViewData().GetTabNumber();

        ScSubTotalParam aParam;
        aParam.bReplace = false;
        aParam.bDoSort = false;
        aParam.nCol1 = 0;
        aParam.nRow1 = 0;
        aParam.nCol2 = 1;
        aParam.nRow2 = 3;
        aParam.aGroups[0].bActive = true;
        aParam.aGroups[0].nField = 0; // group by column A
        SCCOL nSubCols[] = { 1 }; // subtotal on column B
        ScSubTotalFunc nFunctions[] = { SUBTOTAL_FUNC_SUM };
        aParam.SetSubTotals(0, nSubCols, nFunctions, 1);

        ScDBDocFunc aDBDocFunc(*pDocShell);
        aDBDocFunc.DoSubTotals(nSheetViewTab, aParam, true, true);

        // After subtotals, new rows are inserted with subtotal results
        // Row layout: Header(0), A 10(1), A 20(2), A Sum(3), B 30(4), B Sum(5), Grand Total(6)

        // Verify on default view
        CPPUNIT_ASSERT_EQUAL(30.0, pDocument->GetValue(ScAddress(1, 3, 0)));
        CPPUNIT_ASSERT_EQUAL(30.0, pDocument->GetValue(ScAddress(1, 5, 0)));
        CPPUNIT_ASSERT_EQUAL(60.0, pDocument->GetValue(ScAddress(1, 6, 0)));

        // Verify sync to sheet view 1
        CPPUNIT_ASSERT_EQUAL(30.0, pDocument->GetValue(ScAddress(1, 3, nSheetViewTab)));
        CPPUNIT_ASSERT_EQUAL(30.0, pDocument->GetValue(ScAddress(1, 5, nSheetViewTab)));
        CPPUNIT_ASSERT_EQUAL(60.0, pDocument->GetValue(ScAddress(1, 6, nSheetViewTab)));

        // Verify sync to sheet view 2
        SCTAB nSheetView2Tab = aView3.getTabViewShell()->GetViewData().GetTabNumber();
        CPPUNIT_ASSERT_EQUAL(30.0, pDocument->GetValue(ScAddress(1, 3, nSheetView2Tab)));
        CPPUNIT_ASSERT_EQUAL(30.0, pDocument->GetValue(ScAddress(1, 5, nSheetView2Tab)));
        CPPUNIT_ASSERT_EQUAL(60.0, pDocument->GetValue(ScAddress(1, 6, nSheetView2Tab)));
    }
}

CPPUNIT_TEST_FIXTURE(SyncTest, testSync_DrawObject_DefaultAndSheetView)
{
    // Checks that draw objects are synchronized between the default view and sheet views.
    // This checks inserting, moving and deleting draw objects.
    ScModelObj* pModelObj = createDoc("empty.ods");
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ScDocument* pDocument = pModelObj->GetDocument();

    setupViews();

    // Create sheet view
    {
        switchToSheetView();
        createNewSheetViewInCurrentView();
    }

    switchToDefaultView();

    ScDrawLayer* pDrawLayer = pDocument->GetDrawLayer();
    CPPUNIT_ASSERT(pDrawLayer);

    SCTAB nDefaultViewTable = mpTabViewDefaultView->GetViewData().GetTabNumber();
    SCTAB nSheetViewTable = mpTabViewSheetView->GetViewData().GetTabNumber();

    SdrPage* pDefaultViewPage = pDrawLayer->GetPage(sal_uInt16(nDefaultViewTable));
    SdrPage* pSheetViewPage = pDrawLayer->GetPage(sal_uInt16(nSheetViewTable));
    CPPUNIT_ASSERT(pDefaultViewPage);
    CPPUNIT_ASSERT(pSheetViewPage);

    const tools::Rectangle aRectangle1(1000, 1000, 3000, 2000);
    const tools::Rectangle aRectangle2(5000, 5000, 7000, 6000);

    // Initially no objects on either page
    CPPUNIT_ASSERT_EQUAL(size_t(0), pDefaultViewPage->GetObjCount());
    CPPUNIT_ASSERT_EQUAL(size_t(0), pSheetViewPage->GetObjCount());

    // Insert shape on default view
    {
        switchToDefaultView();
        insertDrawObject(pDrawLayer, aRectangle1);

        // Default view page has 1 object at correct position
        CPPUNIT_ASSERT_EQUAL(size_t(1), pDefaultViewPage->GetObjCount());
        CPPUNIT_ASSERT_EQUAL(aRectangle1, pDefaultViewPage->GetObj(0)->GetLogicRect());

        // Sheet view page should also have 1 object at same position after sync
        CPPUNIT_ASSERT_EQUAL(size_t(1), pSheetViewPage->GetObjCount());
        CPPUNIT_ASSERT_EQUAL(aRectangle1, pSheetViewPage->GetObj(0)->GetLogicRect());
    }

    // Insert shape on sheet view
    {
        switchToSheetView();
        insertDrawObject(pDrawLayer, aRectangle2);

        // Default view page has 2 objects at correct positions
        CPPUNIT_ASSERT_EQUAL(size_t(2), pDefaultViewPage->GetObjCount());
        CPPUNIT_ASSERT_EQUAL(aRectangle1, pDefaultViewPage->GetObj(0)->GetLogicRect());
        CPPUNIT_ASSERT_EQUAL(aRectangle2, pDefaultViewPage->GetObj(1)->GetLogicRect());

        // Sheet view page should also have 2 objects at same positions
        CPPUNIT_ASSERT_EQUAL(size_t(2), pSheetViewPage->GetObjCount());
        CPPUNIT_ASSERT_EQUAL(aRectangle1, pSheetViewPage->GetObj(0)->GetLogicRect());
        CPPUNIT_ASSERT_EQUAL(aRectangle2, pSheetViewPage->GetObj(1)->GetLogicRect());
    }

    // Undo the shape insertion from sheet view
    {
        switchToSheetView();
        undo();

        // Back to 1 object on both pages, position preserved
        CPPUNIT_ASSERT_EQUAL(size_t(1), pDefaultViewPage->GetObjCount());
        CPPUNIT_ASSERT_EQUAL(aRectangle1, pDefaultViewPage->GetObj(0)->GetLogicRect());

        CPPUNIT_ASSERT_EQUAL(size_t(1), pSheetViewPage->GetObjCount());
        CPPUNIT_ASSERT_EQUAL(aRectangle1, pSheetViewPage->GetObj(0)->GetLogicRect());
    }

    // Redo the shape insertion
    {
        redo();

        // Back to 2 objects on both pages, positions preserved
        CPPUNIT_ASSERT_EQUAL(size_t(2), pDefaultViewPage->GetObjCount());
        CPPUNIT_ASSERT_EQUAL(aRectangle1, pDefaultViewPage->GetObj(0)->GetLogicRect());
        CPPUNIT_ASSERT_EQUAL(aRectangle2, pDefaultViewPage->GetObj(1)->GetLogicRect());

        CPPUNIT_ASSERT_EQUAL(size_t(2), pSheetViewPage->GetObjCount());
        CPPUNIT_ASSERT_EQUAL(aRectangle1, pSheetViewPage->GetObj(0)->GetLogicRect());
        CPPUNIT_ASSERT_EQUAL(aRectangle2, pSheetViewPage->GetObj(1)->GetLogicRect());
    }

    // Move the first object from the default view
    const tools::Rectangle aRectangle1Moved(1500, 1300, 3500, 2300);
    {
        switchToDefaultView();
        moveDrawObject(pDefaultViewPage->GetObj(0), aRectangle1Moved);

        // Default view page: first object moved, second unchanged
        CPPUNIT_ASSERT_EQUAL(size_t(2), pDefaultViewPage->GetObjCount());
        CPPUNIT_ASSERT_EQUAL(aRectangle1Moved, pDefaultViewPage->GetObj(0)->GetLogicRect());
        CPPUNIT_ASSERT_EQUAL(aRectangle2, pDefaultViewPage->GetObj(1)->GetLogicRect());

        // Sheet view page: same positions after sync
        CPPUNIT_ASSERT_EQUAL(size_t(2), pSheetViewPage->GetObjCount());
        CPPUNIT_ASSERT_EQUAL(aRectangle1Moved, pSheetViewPage->GetObj(0)->GetLogicRect());
        CPPUNIT_ASSERT_EQUAL(aRectangle2, pSheetViewPage->GetObj(1)->GetLogicRect());
    }

    // Move the second object from the sheet view
    const tools::Rectangle aRectangle2Moved(5500, 5300, 7500, 6300);
    {
        switchToSheetView();
        moveDrawObject(pDefaultViewPage->GetObj(1), aRectangle2Moved);

        // Default view page: both objects at moved positions
        CPPUNIT_ASSERT_EQUAL(size_t(2), pDefaultViewPage->GetObjCount());
        CPPUNIT_ASSERT_EQUAL(aRectangle1Moved, pDefaultViewPage->GetObj(0)->GetLogicRect());
        CPPUNIT_ASSERT_EQUAL(aRectangle2Moved, pDefaultViewPage->GetObj(1)->GetLogicRect());

        // Sheet view page: same position as in default view page
        CPPUNIT_ASSERT_EQUAL(size_t(2), pSheetViewPage->GetObjCount());
        CPPUNIT_ASSERT_EQUAL(aRectangle1Moved, pSheetViewPage->GetObj(0)->GetLogicRect());
        CPPUNIT_ASSERT_EQUAL(aRectangle2Moved, pSheetViewPage->GetObj(1)->GetLogicRect());
    }

    // Delete the second object from the default view
    {
        switchToDefaultView();
        deleteDrawObject(pDefaultViewPage->GetObj(1));

        // 1 object remaining on both pages
        CPPUNIT_ASSERT_EQUAL(size_t(1), pDefaultViewPage->GetObjCount());
        CPPUNIT_ASSERT_EQUAL(aRectangle1Moved, pDefaultViewPage->GetObj(0)->GetLogicRect());

        CPPUNIT_ASSERT_EQUAL(size_t(1), pSheetViewPage->GetObjCount());
        CPPUNIT_ASSERT_EQUAL(aRectangle1Moved, pSheetViewPage->GetObj(0)->GetLogicRect());
    }

    // Undo the delete
    {
        undo();

        // Both objects restored on both pages
        CPPUNIT_ASSERT_EQUAL(size_t(2), pDefaultViewPage->GetObjCount());
        CPPUNIT_ASSERT_EQUAL(aRectangle1Moved, pDefaultViewPage->GetObj(0)->GetLogicRect());
        CPPUNIT_ASSERT_EQUAL(aRectangle2Moved, pDefaultViewPage->GetObj(1)->GetLogicRect());

        CPPUNIT_ASSERT_EQUAL(size_t(2), pSheetViewPage->GetObjCount());
        CPPUNIT_ASSERT_EQUAL(aRectangle1Moved, pSheetViewPage->GetObj(0)->GetLogicRect());
        CPPUNIT_ASSERT_EQUAL(aRectangle2Moved, pSheetViewPage->GetObj(1)->GetLogicRect());
    }

    // Delete the first object from the sheet view
    {
        switchToSheetView();
        deleteDrawObject(pDefaultViewPage->GetObj(0));

        // 1 object remaining on both pages
        CPPUNIT_ASSERT_EQUAL(size_t(1), pDefaultViewPage->GetObjCount());
        CPPUNIT_ASSERT_EQUAL(aRectangle2Moved, pDefaultViewPage->GetObj(0)->GetLogicRect());

        CPPUNIT_ASSERT_EQUAL(size_t(1), pSheetViewPage->GetObjCount());
        CPPUNIT_ASSERT_EQUAL(aRectangle2Moved, pSheetViewPage->GetObj(0)->GetLogicRect());
    }

    // Undo the delete from sheet view
    {
        undo();

        // Both objects restored on both pages
        CPPUNIT_ASSERT_EQUAL(size_t(2), pDefaultViewPage->GetObjCount());
        CPPUNIT_ASSERT_EQUAL(aRectangle1Moved, pDefaultViewPage->GetObj(0)->GetLogicRect());
        CPPUNIT_ASSERT_EQUAL(aRectangle2Moved, pDefaultViewPage->GetObj(1)->GetLogicRect());

        CPPUNIT_ASSERT_EQUAL(size_t(2), pSheetViewPage->GetObjCount());
        CPPUNIT_ASSERT_EQUAL(aRectangle1Moved, pSheetViewPage->GetObj(0)->GetLogicRect());
        CPPUNIT_ASSERT_EQUAL(aRectangle2Moved, pSheetViewPage->GetObj(1)->GetLogicRect());
    }
}

CPPUNIT_TEST_FIXTURE(SyncTest, testSync_InsertSparklines_DefaultAndSheetView)
{
    ScModelObj* pModelObj = createDoc("empty.ods");
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    ScDocument& rDocument = pDocShell->GetDocument();

    // Set up data for sparklines in A1:A3
    rDocument.SetValue(ScAddress(0, 0, 0), 1.0);
    rDocument.SetValue(ScAddress(0, 1, 0), 5.0);
    rDocument.SetValue(ScAddress(0, 2, 0), 3.0);

    setupViews();

    // Create sheet view
    {
        switchToSheetView();
        createNewSheetViewInCurrentView();
    }

    SCTAB nSheetViewTab = mpTabViewSheetView->GetViewData().GetTabNumber();

    // Insert sparkline from default view: data A1:A3, sparkline at B1
    {
        switchToDefaultView();

        auto pSparklineGroup = std::make_shared<sc::SparklineGroup>();
        pDocShell->GetDocFunc().InsertSparklines(ScRange(0, 0, 0, 0, 2, 0),
                                                 ScRange(1, 0, 0, 1, 0, 0), pSparklineGroup);

        // Verify on default view
        CPPUNIT_ASSERT(rDocument.HasSparkline(ScAddress(1, 0, 0)));

        // Verify synced to sheet view
        CPPUNIT_ASSERT(rDocument.HasSparkline(ScAddress(1, 0, nSheetViewTab)));
    }

    // Insert sparkline from sheet view: data A1:A3, sparkline at B2
    {
        switchToSheetView();

        auto pSparklineGroup = std::make_shared<sc::SparklineGroup>();
        pDocShell->GetDocFunc().InsertSparklines(ScRange(0, 0, nSheetViewTab, 0, 2, nSheetViewTab),
                                                 ScRange(1, 1, nSheetViewTab, 1, 1, nSheetViewTab),
                                                 pSparklineGroup);

        // Verify on default view
        CPPUNIT_ASSERT(rDocument.HasSparkline(ScAddress(1, 1, 0)));

        // Verify synced to sheet view
        CPPUNIT_ASSERT(rDocument.HasSparkline(ScAddress(1, 1, nSheetViewTab)));
    }
}

CPPUNIT_TEST_FIXTURE(SyncTest, testSync_ChangeSparkline_DefaultAndSheetView)
{
    ScModelObj* pModelObj = createDoc("empty.ods");
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    ScDocument& rDocument = pDocShell->GetDocument();

    // Set up data for sparklines
    rDocument.SetValue(ScAddress(0, 0, 0), 1.0);
    rDocument.SetValue(ScAddress(0, 1, 0), 5.0);
    rDocument.SetValue(ScAddress(0, 2, 0), 3.0);
    rDocument.SetValue(ScAddress(2, 0, 0), 10.0);
    rDocument.SetValue(ScAddress(2, 1, 0), 20.0);
    rDocument.SetValue(ScAddress(2, 2, 0), 30.0);

    // Create a sparkline at B1 with data A1:A3
    auto pSparklineGroup = std::make_shared<sc::SparklineGroup>();
    rDocument.CreateSparkline(ScAddress(1, 0, 0), pSparklineGroup);
    {
        ScRangeList aDataRange;
        aDataRange.push_back(ScRange(0, 0, 0, 0, 2, 0));
        auto pSparkline = rDocument.GetSparkline(ScAddress(1, 0, 0));
        pSparkline->setInputRange(aDataRange);
    }

    setupViews();

    // Create sheet view
    {
        switchToSheetView();
        createNewSheetViewInCurrentView();
    }

    SCTAB nSheetViewTab = mpTabViewSheetView->GetViewData().GetTabNumber();

    // Change sparkline data range from default view: A1:A3 -> C1:C3
    {
        switchToDefaultView();

        auto pSparkline = rDocument.GetSparkline(ScAddress(1, 0, 0));
        CPPUNIT_ASSERT(pSparkline);

        ScRangeList aNewDataRange;
        aNewDataRange.push_back(ScRange(2, 0, 0, 2, 2, 0));
        pDocShell->GetDocFunc().ChangeSparkline(pSparkline, 0, aNewDataRange);

        // Verify input range changed on default view
        ScRangeList aInputRange = pSparkline->getInputRange();
        CPPUNIT_ASSERT_EQUAL(size_t(1), aInputRange.size());
        CPPUNIT_ASSERT_EQUAL(SCCOL(2), aInputRange[0].aStart.Col());

        // Verify synced to sheet view
        auto pSheetViewSparkline = rDocument.GetSparkline(ScAddress(1, 0, nSheetViewTab));
        CPPUNIT_ASSERT(pSheetViewSparkline);
        ScRangeList aSheetViewInputRange = pSheetViewSparkline->getInputRange();
        CPPUNIT_ASSERT_EQUAL(size_t(1), aSheetViewInputRange.size());
        CPPUNIT_ASSERT_EQUAL(SCCOL(2), aSheetViewInputRange[0].aStart.Col());
    }
}

CPPUNIT_TEST_FIXTURE(SyncTest, testSync_DeleteSparkline_DefaultAndSheetView)
{
    ScModelObj* pModelObj = createDoc("empty.ods");
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    ScDocument& rDocument = pDocShell->GetDocument();

    // Create sparklines at B1 and B2
    auto pSparklineGroup = std::make_shared<sc::SparklineGroup>();
    rDocument.CreateSparkline(ScAddress(1, 0, 0), pSparklineGroup);
    rDocument.CreateSparkline(ScAddress(1, 1, 0), pSparklineGroup);

    setupViews();

    // Create sheet view
    {
        switchToSheetView();
        createNewSheetViewInCurrentView();
    }

    SCTAB nSheetViewTab = mpTabViewSheetView->GetViewData().GetTabNumber();

    // Verify sparklines exist on both views
    CPPUNIT_ASSERT(rDocument.HasSparkline(ScAddress(1, 0, 0)));
    CPPUNIT_ASSERT(rDocument.HasSparkline(ScAddress(1, 1, 0)));
    CPPUNIT_ASSERT(rDocument.HasSparkline(ScAddress(1, 0, nSheetViewTab)));
    CPPUNIT_ASSERT(rDocument.HasSparkline(ScAddress(1, 1, nSheetViewTab)));

    // Delete sparkline at B1 from default view
    {
        switchToDefaultView();

        bool bResult = pDocShell->GetDocFunc().DeleteSparkline(ScAddress(1, 0, 0));
        CPPUNIT_ASSERT(bResult);

        // Verify deleted on default view
        CPPUNIT_ASSERT(!rDocument.HasSparkline(ScAddress(1, 0, 0)));

        // Verify synced to sheet view
        CPPUNIT_ASSERT(!rDocument.HasSparkline(ScAddress(1, 0, nSheetViewTab)));
    }

    // Delete sparkline at B2 from sheet view
    {
        switchToSheetView();

        bool bResult = pDocShell->GetDocFunc().DeleteSparkline(ScAddress(1, 1, nSheetViewTab));
        CPPUNIT_ASSERT(bResult);

        // Verify deleted on default view
        CPPUNIT_ASSERT(!rDocument.HasSparkline(ScAddress(1, 1, 0)));

        // Verify synced to sheet view
        CPPUNIT_ASSERT(!rDocument.HasSparkline(ScAddress(1, 1, nSheetViewTab)));
    }
}

CPPUNIT_TEST_FIXTURE(SyncTest, testSync_GroupUngroupSparklines_DefaultAndSheetView)
{
    ScModelObj* pModelObj = createDoc("empty.ods");
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    ScDocument& rDocument = pDocShell->GetDocument();

    // Create two sparklines in different groups at B1 and B2
    auto pGroup1 = std::make_shared<sc::SparklineGroup>();
    auto pGroup2 = std::make_shared<sc::SparklineGroup>();
    rDocument.CreateSparkline(ScAddress(1, 0, 0), pGroup1);
    rDocument.CreateSparkline(ScAddress(1, 1, 0), pGroup2);

    setupViews();

    // Create sheet view
    {
        switchToSheetView();
        createNewSheetViewInCurrentView();
    }

    SCTAB nSheetViewTab = mpTabViewSheetView->GetViewData().GetTabNumber();

    // Verify sparklines are in different groups initially
    {
        auto pSparkline1 = rDocument.GetSparkline(ScAddress(1, 0, 0));
        auto pSparkline2 = rDocument.GetSparkline(ScAddress(1, 1, 0));
        CPPUNIT_ASSERT(pSparkline1->getSparklineGroup() != pSparkline2->getSparklineGroup());
    }

    // Group sparklines from default view: B1:B2 into pGroup1
    {
        switchToDefaultView();

        pDocShell->GetDocFunc().GroupSparklines(ScRange(1, 0, 0, 1, 1, 0), pGroup1);

        // Verify both sparklines are in the same group on default view
        auto pSparkline1 = rDocument.GetSparkline(ScAddress(1, 0, 0));
        auto pSparkline2 = rDocument.GetSparkline(ScAddress(1, 1, 0));
        CPPUNIT_ASSERT_EQUAL(pSparkline1->getSparklineGroup(), pSparkline2->getSparklineGroup());

        // Verify synced to sheet view
        auto pSheetViewSparkline1 = rDocument.GetSparkline(ScAddress(1, 0, nSheetViewTab));
        auto pSheetViewSparkline2 = rDocument.GetSparkline(ScAddress(1, 1, nSheetViewTab));
        CPPUNIT_ASSERT_EQUAL(pSheetViewSparkline1->getSparklineGroup(),
                             pSheetViewSparkline2->getSparklineGroup());
    }

    // Ungroup sparklines from default view: B1:B2
    {
        switchToDefaultView();

        pDocShell->GetDocFunc().UngroupSparklines(ScRange(1, 0, 0, 1, 1, 0));

        // Verify sparklines are in different groups on default view
        auto pSparkline1 = rDocument.GetSparkline(ScAddress(1, 0, 0));
        auto pSparkline2 = rDocument.GetSparkline(ScAddress(1, 1, 0));
        CPPUNIT_ASSERT(pSparkline1->getSparklineGroup() != pSparkline2->getSparklineGroup());

        // Verify synced to sheet view
        auto pSheetViewSparkline1 = rDocument.GetSparkline(ScAddress(1, 0, nSheetViewTab));
        auto pSheetViewSparkline2 = rDocument.GetSparkline(ScAddress(1, 1, nSheetViewTab));
        CPPUNIT_ASSERT(pSheetViewSparkline1->getSparklineGroup()
                       != pSheetViewSparkline2->getSparklineGroup());
    }

    // Group sparklines from sheet view: B1:B2 into pGroup1
    {
        switchToSheetView();

        pDocShell->GetDocFunc().GroupSparklines(ScRange(1, 0, nSheetViewTab, 1, 1, nSheetViewTab),
                                                pGroup1);

        // Verify both sparklines are in the same group on default view
        auto pSparkline1 = rDocument.GetSparkline(ScAddress(1, 0, 0));
        auto pSparkline2 = rDocument.GetSparkline(ScAddress(1, 1, 0));
        CPPUNIT_ASSERT_EQUAL(pSparkline1->getSparklineGroup(), pSparkline2->getSparklineGroup());

        // Verify synced to sheet view
        auto pSheetViewSparkline1 = rDocument.GetSparkline(ScAddress(1, 0, nSheetViewTab));
        auto pSheetViewSparkline2 = rDocument.GetSparkline(ScAddress(1, 1, nSheetViewTab));
        CPPUNIT_ASSERT_EQUAL(pSheetViewSparkline1->getSparklineGroup(),
                             pSheetViewSparkline2->getSparklineGroup());
    }

    // Ungroup sparklines from sheet view: B1:B2
    {
        switchToSheetView();

        pDocShell->GetDocFunc().UngroupSparklines(
            ScRange(1, 0, nSheetViewTab, 1, 1, nSheetViewTab));

        // Verify sparklines are in different groups on default view
        auto pSparkline1 = rDocument.GetSparkline(ScAddress(1, 0, 0));
        auto pSparkline2 = rDocument.GetSparkline(ScAddress(1, 1, 0));
        CPPUNIT_ASSERT(pSparkline1->getSparklineGroup() != pSparkline2->getSparklineGroup());

        // Verify synced to sheet view
        auto pSheetViewSparkline1 = rDocument.GetSparkline(ScAddress(1, 0, nSheetViewTab));
        auto pSheetViewSparkline2 = rDocument.GetSparkline(ScAddress(1, 1, nSheetViewTab));
        CPPUNIT_ASSERT(pSheetViewSparkline1->getSparklineGroup()
                       != pSheetViewSparkline2->getSparklineGroup());
    }
}

CPPUNIT_TEST_FIXTURE(SyncTest, testSync_ChangeSparklineGroupAttributes_DefaultAndSheetView)
{
    ScModelObj* pModelObj = createDoc("empty.ods");
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    ScDocument& rDocument = pDocShell->GetDocument();

    // Create a sparkline group with Column type and a sparkline at B1
    auto pSparklineGroup = std::make_shared<sc::SparklineGroup>();
    pSparklineGroup->getAttributes().setType(sc::SparklineType::Column);
    rDocument.CreateSparkline(ScAddress(1, 0, 0), pSparklineGroup);

    setupViews();

    // Create sheet view
    {
        switchToSheetView();
        createNewSheetViewInCurrentView();
    }

    SCTAB nSheetViewTab = mpTabViewSheetView->GetViewData().GetTabNumber();

    // Change sparkline group attributes from default view: Column -> Stacked
    {
        switchToDefaultView();

        sc::SparklineAttributes aNewAttributes;
        aNewAttributes.setType(sc::SparklineType::Stacked);

        pDocShell->GetDocFunc().ChangeSparklineGroupAttributes(pSparklineGroup, aNewAttributes);

        // Verify on default view
        CPPUNIT_ASSERT_EQUAL(sc::SparklineType::Stacked,
                             pSparklineGroup->getAttributes().getType());

        // Verify synced to sheet view
        auto pSheetViewSparkline = rDocument.GetSparkline(ScAddress(1, 0, nSheetViewTab));
        CPPUNIT_ASSERT(pSheetViewSparkline);
        CPPUNIT_ASSERT_EQUAL(sc::SparklineType::Stacked,
                             pSheetViewSparkline->getSparklineGroup()->getAttributes().getType());
    }
}

CPPUNIT_TEST_FIXTURE(SyncTest, testSync_DeleteSparklineGroup_DefaultAndSheetView)
{
    ScModelObj* pModelObj = createDoc("empty.ods");
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    ScDocument& rDocument = pDocShell->GetDocument();

    // Create two sparkline groups with sparklines
    auto pGroup1 = std::make_shared<sc::SparklineGroup>();
    auto pGroup2 = std::make_shared<sc::SparklineGroup>();
    rDocument.CreateSparkline(ScAddress(1, 0, 0), pGroup1);
    rDocument.CreateSparkline(ScAddress(1, 1, 0), pGroup1);
    rDocument.CreateSparkline(ScAddress(1, 2, 0), pGroup2);

    setupViews();

    // Create sheet view
    {
        switchToSheetView();
        createNewSheetViewInCurrentView();
    }

    SCTAB nSheetViewTab = mpTabViewSheetView->GetViewData().GetTabNumber();

    // Delete sparkline group 1 from default view
    {
        switchToDefaultView();

        bool bResult = pDocShell->GetDocFunc().DeleteSparklineGroup(pGroup1, 0);
        CPPUNIT_ASSERT(bResult);

        // Verify sparklines from group 1 are gone on default view
        CPPUNIT_ASSERT(!rDocument.HasSparkline(ScAddress(1, 0, 0)));
        CPPUNIT_ASSERT(!rDocument.HasSparkline(ScAddress(1, 1, 0)));
        // Group 2 sparkline still exists
        CPPUNIT_ASSERT(rDocument.HasSparkline(ScAddress(1, 2, 0)));

        // Verify synced to sheet view
        CPPUNIT_ASSERT(!rDocument.HasSparkline(ScAddress(1, 0, nSheetViewTab)));
        CPPUNIT_ASSERT(!rDocument.HasSparkline(ScAddress(1, 1, nSheetViewTab)));
        CPPUNIT_ASSERT(rDocument.HasSparkline(ScAddress(1, 2, nSheetViewTab)));
    }

    // Delete sparkline group 2 from sheet view
    {
        switchToSheetView();

        bool bResult = pDocShell->GetDocFunc().DeleteSparklineGroup(pGroup2, nSheetViewTab);
        CPPUNIT_ASSERT(bResult);

        // Verify all sparklines are gone on default view
        CPPUNIT_ASSERT(!rDocument.HasSparkline(ScAddress(1, 2, 0)));

        // Verify synced to sheet view
        CPPUNIT_ASSERT(!rDocument.HasSparkline(ScAddress(1, 2, nSheetViewTab)));
    }
}

CPPUNIT_TEST_FIXTURE(SyncTest, testSync_InsertCells_DefaultAndSheetView)
{
    // Test sync to sheet view when inserting rows

    ScModelObj* pModelObj = createDoc("SheetView_AutoFilter.ods");
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    ScDocument& rDocument = pDocShell->GetDocument();

    // Put data below autofilter range so we can verify the row insert syncs
    rDocument.SetString(ScAddress(0, 6, 0), u"ABC"_ustr);

    setupViews();

    // Create sheet view and sort descending
    {
        switchToSheetView();
        createNewSheetViewInCurrentView();
        sortDescendingForCell(u"A1");
    }

    SCTAB nSheetViewTab = mpTabViewSheetView->GetViewData().GetTabNumber();
    SCTAB nDefaultViewTab = mpTabViewDefaultView->GetViewData().GetTabNumber();

    // Verify initial state: range A2:A5
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"3", u"7" }),
                         getValues(mpTabViewDefaultView, 0, 1, 4));
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"7", u"5", u"4", u"3" }),
                         getValues(mpTabViewSheetView, 0, 1, 4));

    // Insert row before row 7 in default view (outside autofilter range)
    {
        switchToDefaultView();

        sc::InsertCellsOperation aOperation(
            *pDocShell, ScRange(0, 6, nDefaultViewTab, rDocument.MaxCol(), 6, nDefaultViewTab),
            nullptr, INS_INSROWS_BEFORE, true, true, false, 0);
        CPPUNIT_ASSERT(aOperation.run());

        // Autofilter data unchanged in both views
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"3", u"7" }),
                             getValues(mpTabViewDefaultView, 0, 1, 4));
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"7", u"5", u"4", u"3" }),
                             getValues(mpTabViewSheetView, 0, 1, 4));

        // "ABC" shifted from A7 to A8, A7 is now empty - synced to both views
        CPPUNIT_ASSERT_EQUAL(u""_ustr, rDocument.GetString(ScAddress(0, 6, nDefaultViewTab)));
        CPPUNIT_ASSERT_EQUAL(u"ABC"_ustr, rDocument.GetString(ScAddress(0, 7, nDefaultViewTab)));
        CPPUNIT_ASSERT_EQUAL(u""_ustr, rDocument.GetString(ScAddress(0, 6, nSheetViewTab)));
        CPPUNIT_ASSERT_EQUAL(u"ABC"_ustr, rDocument.GetString(ScAddress(0, 7, nSheetViewTab)));
    }

    // Insert row in autofilter range from default view
    {
        switchToDefaultView();

        sc::InsertCellsOperation aOperation(
            *pDocShell, ScRange(0, 2, nDefaultViewTab, rDocument.MaxCol(), 2, nDefaultViewTab),
            nullptr, INS_INSROWS_BEFORE, true, true, false, 0);
        CPPUNIT_ASSERT(aOperation.run());

        // Default view: inserted empty row into A3
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"", u"5", u"3", u"7" }),
                             getValues(mpTabViewDefaultView, 0, 1, 5));
        // Sheet view re-sorted descending
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"7", u"5", u"4", u"3", u"" }),
                             getValues(mpTabViewSheetView, 0, 1, 5));

        // Auto-filter range should have expanded to A1:A6 on both views
        CPPUNIT_ASSERT_EQUAL(ScRange(0, 0, nDefaultViewTab, 0, 5, nDefaultViewTab),
                             getAutoFilterArea(rDocument, nDefaultViewTab));
        CPPUNIT_ASSERT_EQUAL(ScRange(0, 0, nSheetViewTab, 0, 5, nSheetViewTab),
                             getAutoFilterArea(rDocument, nSheetViewTab));
    }

    // Insert row on sheet view in autofilter range — should be blocked
    {
        switchToSheetView();

        sc::InsertCellsOperation aOperation(
            *pDocShell, ScRange(0, 2, nSheetViewTab, rDocument.MaxCol(), 2, nSheetViewTab), nullptr,
            INS_INSROWS_BEFORE, true, true, false, 0);
        CPPUNIT_ASSERT(!aOperation.run());

        // Values should remain unchanged
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"", u"5", u"3", u"7" }),
                             getValues(mpTabViewDefaultView, 0, 1, 5));
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"7", u"5", u"4", u"3", u"" }),
                             getValues(mpTabViewSheetView, 0, 1, 5));

        // Auto-filter ranges should remain unchanged
        CPPUNIT_ASSERT_EQUAL(ScRange(0, 0, nDefaultViewTab, 0, 5, nDefaultViewTab),
                             getAutoFilterArea(rDocument, nDefaultViewTab));
        CPPUNIT_ASSERT_EQUAL(ScRange(0, 0, nSheetViewTab, 0, 5, nSheetViewTab),
                             getAutoFilterArea(rDocument, nSheetViewTab));
    }
}

CPPUNIT_TEST_FIXTURE(SyncTest, testSync_UndoInsertCells_SheetViewSortData)
{
    // Test that undoing insert rows restores sheet view sort data

    ScModelObj* pModelObj = createDoc("SheetView_AutoFilter.ods");
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    ScDocument& rDocument = pDocShell->GetDocument();

    setupViews();

    // Create sheet view and sort descending
    {
        switchToSheetView();
        createNewSheetViewInCurrentView();
        sortDescendingForCell(u"A1");
    }

    SCTAB nDefaultViewTab = mpTabViewDefaultView->GetViewData().GetTabNumber();

    // Verify initial state
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"3", u"7" }),
                         getValues(mpTabViewDefaultView, 0, 1, 4));
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"7", u"5", u"4", u"3" }),
                         getValues(mpTabViewSheetView, 0, 1, 4));

    // Verify sheet view has sort data
    std::shared_ptr<sc::SheetViewManager> pManager = rDocument.GetSheetViewManager(nDefaultViewTab);
    CPPUNIT_ASSERT(pManager);
    auto& rSheetView = *pManager->iterateValidSheetViews().begin();
    CPPUNIT_ASSERT(rSheetView.getSortOrder() != nullptr);

    // Insert row in auto-filter range from default view
    {
        switchToDefaultView();
        sc::InsertCellsOperation aOperation(
            *pDocShell, ScRange(0, 2, nDefaultViewTab, rDocument.MaxCol(), 2, nDefaultViewTab),
            nullptr, INS_INSROWS_BEFORE, true, true, false, 0);
        CPPUNIT_ASSERT(aOperation.run());
    }

    // Verify expanded sort range
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"", u"5", u"3", u"7" }),
                         getValues(mpTabViewDefaultView, 0, 1, 5));
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"7", u"5", u"4", u"3", u"" }),
                         getValues(mpTabViewSheetView, 0, 1, 5));

    // Undo the insert
    {
        switchToDefaultView();
        undo();
    }

    // Verify data is back to original
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"3", u"7" }),
                         getValues(mpTabViewDefaultView, 0, 1, 4));

    // Sheet view sort data should be restored - sort range should be original 4 rows
    CPPUNIT_ASSERT(rSheetView.getSortOrder() != nullptr);
    CPPUNIT_ASSERT(rSheetView.getSortParam() != nullptr);

    // Trigger sync by editing on default view
    {
        switchToDefaultView();
        typeCharsInCell(std::string("44"), 0, 1, mpTabViewDefaultView, pModelObj);
    }

    // Default view: A2=44
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"44", u"5", u"3", u"7" }),
                         getValues(mpTabViewDefaultView, 0, 1, 4));

    // Sheet view should sync correctly with original sort range (descending)
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"44", u"7", u"5", u"3" }),
                         getValues(mpTabViewSheetView, 0, 1, 4));
}

CPPUNIT_TEST_FIXTURE(SyncTest, testSync_UndoDeleteCells_SheetViewSortData)
{
    // Test that undoing delete rows restores sheet view sort data

    ScModelObj* pModelObj = createDoc("SheetView_AutoFilter.ods");
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    ScDocument& rDocument = pDocShell->GetDocument();

    setupViews();

    // Create sheet view and sort descending
    {
        switchToSheetView();
        createNewSheetViewInCurrentView();
        sortDescendingForCell(u"A1");
    }

    SCTAB nDefaultViewTab = mpTabViewDefaultView->GetViewData().GetTabNumber();

    // Verify initial state
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"3", u"7" }),
                         getValues(mpTabViewDefaultView, 0, 1, 4));
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"7", u"5", u"4", u"3" }),
                         getValues(mpTabViewSheetView, 0, 1, 4));

    std::shared_ptr<sc::SheetViewManager> pManager = rDocument.GetSheetViewManager(nDefaultViewTab);
    CPPUNIT_ASSERT(pManager);
    auto& rSheetView = *pManager->iterateValidSheetViews().begin();
    CPPUNIT_ASSERT(rSheetView.getSortOrder() != nullptr);

    // Delete row in auto-filter range from default view (row 3)
    {
        switchToDefaultView();
        sc::DeleteCellsOperation aOperation(
            *pDocShell, ScRange(0, 2, nDefaultViewTab, rDocument.MaxCol(), 2, nDefaultViewTab),
            nullptr, DelCellCmd::Rows, true);
        CPPUNIT_ASSERT(aOperation.run());
    }

    // Verify shrunk sort range
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"3", u"7" }),
                         getValues(mpTabViewDefaultView, 0, 1, 3));
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"7", u"4", u"3" }),
                         getValues(mpTabViewSheetView, 0, 1, 3));

    // Undo the delete
    {
        switchToDefaultView();
        undo();
    }

    // Verify data is back to original
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"3", u"7" }),
                         getValues(mpTabViewDefaultView, 0, 1, 4));

    // Sheet view sort data should be restored
    CPPUNIT_ASSERT(rSheetView.getSortOrder() != nullptr);
    CPPUNIT_ASSERT(rSheetView.getSortParam() != nullptr);

    // Trigger sync by editing on default view
    {
        switchToDefaultView();
        typeCharsInCell(std::string("55"), 0, 2, mpTabViewDefaultView, pModelObj);
    }

    // Default view: A3=55
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"55", u"3", u"7" }),
                         getValues(mpTabViewDefaultView, 0, 1, 4));

    // Sheet view should sync correctly with original sort range (descending)
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"55", u"7", u"4", u"3" }),
                         getValues(mpTabViewSheetView, 0, 1, 4));
}

CPPUNIT_TEST_FIXTURE(SyncTest, testSync_InsertDeleteColumns_SheetViewSortData)
{
    // Test that inserting/deleting columns in the middle of the auto-filter
    // adjusts sheet view sort ranges and that undo/redo restores them.

    ScModelObj* pModelObj = createDoc("SheetView_AutoFilter_Extended.ods");
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    ScDocument& rDocument = pDocShell->GetDocument();

    setupViews();

    // Create sheet view and sort descending on column A
    {
        switchToSheetView();
        createNewSheetViewInCurrentView();
        sortDescendingForCell(u"A1");
    }

    SCTAB nDefaultViewTab = mpTabViewDefaultView->GetViewData().GetTabNumber();

    // Verify initial state
    // Column A: default unsorted, sheet view descending
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"3", u"7" }),
                         getValues(mpTabViewDefaultView, 0, 1, 4));
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"7", u"5", u"4", u"3" }),
                         getValues(mpTabViewSheetView, 0, 1, 4));
    // Column B: follows the sort
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"ccc", u"rrr", u"sss", u"aaa" }),
                         getValues(mpTabViewDefaultView, 1, 1, 4));
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"aaa", u"rrr", u"ccc", u"sss" }),
                         getValues(mpTabViewSheetView, 1, 1, 4));

    SCTAB nSheetViewTab = mpTabViewSheetView->GetViewData().GetTabNumber();
    std::shared_ptr<sc::SheetView> pSheetView = rDocument.GetSheetView(nSheetViewTab);
    CPPUNIT_ASSERT(pSheetView);

    // Verify initial sort ranges
    CPPUNIT_ASSERT_EQUAL(SCCOL(0), pSheetView->getSortOrder()->maSortInfo.mnFirstColumn);
    CPPUNIT_ASSERT_EQUAL(SCCOL(2), pSheetView->getSortOrder()->maSortInfo.mnLastColumn);
    CPPUNIT_ASSERT_EQUAL(SCCOL(0), pSheetView->getSortParam()->nCol1);
    CPPUNIT_ASSERT_EQUAL(SCCOL(2), pSheetView->getSortParam()->nCol2);

    // Insert a column before column B (in the middle of the auto-filter)
    {
        switchToDefaultView();
        sc::InsertCellsOperation aOperation(
            *pDocShell, ScRange(1, 0, nDefaultViewTab, 1, rDocument.MaxRow(), nDefaultViewTab),
            nullptr, INS_INSCOLS_BEFORE, true, true, false, 0);
        CPPUNIT_ASSERT(aOperation.run());
    }

    // Column A unchanged, original B shifted to C
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"3", u"7" }),
                         getValues(mpTabViewDefaultView, 0, 1, 4));
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"ccc", u"rrr", u"sss", u"aaa" }),
                         getValues(mpTabViewDefaultView, 2, 1, 4));
    // Sheet view sorted - column C has the sorted B values
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"7", u"5", u"4", u"3" }),
                         getValues(mpTabViewSheetView, 0, 1, 4));
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"aaa", u"rrr", u"ccc", u"sss" }),
                         getValues(mpTabViewSheetView, 2, 1, 4));

    // Sort ranges should have expanded from A-C to A-D
    CPPUNIT_ASSERT(pSheetView->getSortOrder() != nullptr);
    CPPUNIT_ASSERT(pSheetView->getSortParam() != nullptr);
    CPPUNIT_ASSERT_EQUAL(SCCOL(0), pSheetView->getSortOrder()->maSortInfo.mnFirstColumn);
    CPPUNIT_ASSERT_EQUAL(SCCOL(3), pSheetView->getSortOrder()->maSortInfo.mnLastColumn);
    CPPUNIT_ASSERT_EQUAL(SCCOL(0), pSheetView->getSortParam()->nCol1);
    CPPUNIT_ASSERT_EQUAL(SCCOL(3), pSheetView->getSortParam()->nCol2);

    // Undo the column insert
    {
        switchToDefaultView();
        undo();
    }

    // Data back in original columns
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"3", u"7" }),
                         getValues(mpTabViewDefaultView, 0, 1, 4));
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"ccc", u"rrr", u"sss", u"aaa" }),
                         getValues(mpTabViewDefaultView, 1, 1, 4));
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"7", u"5", u"4", u"3" }),
                         getValues(mpTabViewSheetView, 0, 1, 4));
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"aaa", u"rrr", u"ccc", u"sss" }),
                         getValues(mpTabViewSheetView, 1, 1, 4));

    // Sort ranges should be restored to A-C
    CPPUNIT_ASSERT(pSheetView->getSortOrder() != nullptr);
    CPPUNIT_ASSERT(pSheetView->getSortParam() != nullptr);
    CPPUNIT_ASSERT_EQUAL(SCCOL(0), pSheetView->getSortOrder()->maSortInfo.mnFirstColumn);
    CPPUNIT_ASSERT_EQUAL(SCCOL(2), pSheetView->getSortOrder()->maSortInfo.mnLastColumn);
    CPPUNIT_ASSERT_EQUAL(SCCOL(0), pSheetView->getSortParam()->nCol1);
    CPPUNIT_ASSERT_EQUAL(SCCOL(2), pSheetView->getSortParam()->nCol2);

    // Redo the column insert
    {
        switchToDefaultView();
        redo();
    }

    // Data shifted again - original B now in C
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"3", u"7" }),
                         getValues(mpTabViewDefaultView, 0, 1, 4));
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"ccc", u"rrr", u"sss", u"aaa" }),
                         getValues(mpTabViewDefaultView, 2, 1, 4));
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"7", u"5", u"4", u"3" }),
                         getValues(mpTabViewSheetView, 0, 1, 4));
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"aaa", u"rrr", u"ccc", u"sss" }),
                         getValues(mpTabViewSheetView, 2, 1, 4));

    // Sort ranges should be A-D again
    CPPUNIT_ASSERT(pSheetView->getSortOrder() != nullptr);
    CPPUNIT_ASSERT(pSheetView->getSortParam() != nullptr);
    CPPUNIT_ASSERT_EQUAL(SCCOL(0), pSheetView->getSortOrder()->maSortInfo.mnFirstColumn);
    CPPUNIT_ASSERT_EQUAL(SCCOL(3), pSheetView->getSortOrder()->maSortInfo.mnLastColumn);
    CPPUNIT_ASSERT_EQUAL(SCCOL(0), pSheetView->getSortParam()->nCol1);
    CPPUNIT_ASSERT_EQUAL(SCCOL(3), pSheetView->getSortParam()->nCol2);

    // Undo again to get back to original state for the delete test
    {
        switchToDefaultView();
        undo();
    }

    // Delete column B from default view
    {
        switchToDefaultView();
        sc::DeleteCellsOperation aOperation(
            *pDocShell, ScRange(1, 0, nDefaultViewTab, 1, rDocument.MaxRow(), nDefaultViewTab),
            nullptr, DelCellCmd::Cols, true);
        CPPUNIT_ASSERT(aOperation.run());
    }

    // Column B deleted - original C now in B
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"3", u"7" }),
                         getValues(mpTabViewDefaultView, 0, 1, 4));
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"7", u"5", u"4", u"3" }),
                         getValues(mpTabViewSheetView, 0, 1, 4));

    // Sort ranges should have shrunk from A-C to A-B
    CPPUNIT_ASSERT(pSheetView->getSortOrder() != nullptr);
    CPPUNIT_ASSERT(pSheetView->getSortParam() != nullptr);
    CPPUNIT_ASSERT_EQUAL(SCCOL(0), pSheetView->getSortOrder()->maSortInfo.mnFirstColumn);
    CPPUNIT_ASSERT_EQUAL(SCCOL(1), pSheetView->getSortOrder()->maSortInfo.mnLastColumn);
    CPPUNIT_ASSERT_EQUAL(SCCOL(0), pSheetView->getSortParam()->nCol1);
    CPPUNIT_ASSERT_EQUAL(SCCOL(1), pSheetView->getSortParam()->nCol2);

    // Undo the column delete
    {
        switchToDefaultView();
        undo();
    }

    // Data restored
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"3", u"7" }),
                         getValues(mpTabViewDefaultView, 0, 1, 4));
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"ccc", u"rrr", u"sss", u"aaa" }),
                         getValues(mpTabViewDefaultView, 1, 1, 4));
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"7", u"5", u"4", u"3" }),
                         getValues(mpTabViewSheetView, 0, 1, 4));
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"aaa", u"rrr", u"ccc", u"sss" }),
                         getValues(mpTabViewSheetView, 1, 1, 4));

    // Sort ranges should be restored to A-C
    CPPUNIT_ASSERT(pSheetView->getSortOrder() != nullptr);
    CPPUNIT_ASSERT(pSheetView->getSortParam() != nullptr);
    CPPUNIT_ASSERT_EQUAL(SCCOL(0), pSheetView->getSortOrder()->maSortInfo.mnFirstColumn);
    CPPUNIT_ASSERT_EQUAL(SCCOL(2), pSheetView->getSortOrder()->maSortInfo.mnLastColumn);
    CPPUNIT_ASSERT_EQUAL(SCCOL(0), pSheetView->getSortParam()->nCol1);
    CPPUNIT_ASSERT_EQUAL(SCCOL(2), pSheetView->getSortParam()->nCol2);
}

CPPUNIT_TEST_FIXTURE(SyncTest, testSync_DeleteCells_DefaultAndSheetView)
{
    // Test sync to sheet view when deleting rows

    ScModelObj* pModelObj = createDoc("SheetView_AutoFilter.ods");
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    ScDocument& rDocument = pDocShell->GetDocument();

    // Put data below autofilter range so we can verify the row delete syncs
    rDocument.SetString(ScAddress(0, 6, 0), u"ABC"_ustr);

    setupViews();

    // Create sheet view and sort descending
    {
        switchToSheetView();
        createNewSheetViewInCurrentView();
        sortDescendingForCell(u"A1");
    }

    SCTAB nSheetViewTab = mpTabViewSheetView->GetViewData().GetTabNumber();
    SCTAB nDefaultViewTab = mpTabViewDefaultView->GetViewData().GetTabNumber();

    // Verify initial state - autofilter range A1:A5, data A2:A5
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"3", u"7" }),
                         getValues(mpTabViewDefaultView, 0, 1, 4));
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"7", u"5", u"4", u"3" }),
                         getValues(mpTabViewSheetView, 0, 1, 4));
    CPPUNIT_ASSERT_EQUAL(ScRange(0, 0, nDefaultViewTab, 0, 4, nDefaultViewTab),
                         getAutoFilterArea(rDocument, nDefaultViewTab));
    CPPUNIT_ASSERT_EQUAL(ScRange(0, 0, nSheetViewTab, 0, 4, nSheetViewTab),
                         getAutoFilterArea(rDocument, nSheetViewTab));

    // Delete row 7 from default view (outside autofilter range)
    {
        switchToDefaultView();

        sc::DeleteCellsOperation aOperation(
            *pDocShell, ScRange(0, 6, nDefaultViewTab, rDocument.MaxCol(), 6, nDefaultViewTab),
            nullptr, DelCellCmd::Rows, true);
        CPPUNIT_ASSERT(aOperation.run());

        // Autofilter data unchanged in both views
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"3", u"7" }),
                             getValues(mpTabViewDefaultView, 0, 1, 4));
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"7", u"5", u"4", u"3" }),
                             getValues(mpTabViewSheetView, 0, 1, 4));

        // "ABC" was in A7 - now deleted on both views
        CPPUNIT_ASSERT_EQUAL(u""_ustr, rDocument.GetString(ScAddress(0, 6, nDefaultViewTab)));
        CPPUNIT_ASSERT_EQUAL(u""_ustr, rDocument.GetString(ScAddress(0, 6, nSheetViewTab)));

        // Autofilter range unchanged on both views (delete was outside)
        CPPUNIT_ASSERT_EQUAL(ScRange(0, 0, nDefaultViewTab, 0, 4, nDefaultViewTab),
                             getAutoFilterArea(rDocument, nDefaultViewTab));
        CPPUNIT_ASSERT_EQUAL(ScRange(0, 0, nSheetViewTab, 0, 4, nSheetViewTab),
                             getAutoFilterArea(rDocument, nSheetViewTab));
    }

    // Delete row in autofilter range from default view (delete row 3)
    {
        switchToDefaultView();

        sc::DeleteCellsOperation aOperation(
            *pDocShell, ScRange(0, 2, nDefaultViewTab, rDocument.MaxCol(), 2, nDefaultViewTab),
            nullptr, DelCellCmd::Rows, true);
        CPPUNIT_ASSERT(aOperation.run());

        // Default view: row 3 deleted, range is now A2:A4
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"3", u"7" }),
                             getValues(mpTabViewDefaultView, 0, 1, 3));
        // Sheet view re-sorted
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"7", u"4", u"3" }),
                             getValues(mpTabViewSheetView, 0, 1, 3));

        // Autofilter range should have shrunk from A1:A5 to A1:A4 on both views
        CPPUNIT_ASSERT_EQUAL(ScRange(0, 0, nDefaultViewTab, 0, 3, nDefaultViewTab),
                             getAutoFilterArea(rDocument, nDefaultViewTab));
        CPPUNIT_ASSERT_EQUAL(ScRange(0, 0, nSheetViewTab, 0, 3, nSheetViewTab),
                             getAutoFilterArea(rDocument, nSheetViewTab));
    }

    // Delete row on sheet view in autofilter range - should be blocked
    {
        switchToSheetView();

        sc::DeleteCellsOperation aOperation(
            *pDocShell, ScRange(0, 2, nSheetViewTab, rDocument.MaxCol(), 2, nSheetViewTab), nullptr,
            DelCellCmd::Rows, true);
        CPPUNIT_ASSERT(!aOperation.run());

        // Values should remain unchanged
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"3", u"7" }),
                             getValues(mpTabViewDefaultView, 0, 1, 3));
        CPPUNIT_ASSERT_EQUAL(expectedValues({ u"7", u"4", u"3" }),
                             getValues(mpTabViewSheetView, 0, 1, 3));

        // Autofilter ranges should remain unchanged
        CPPUNIT_ASSERT_EQUAL(ScRange(0, 0, nDefaultViewTab, 0, 3, nDefaultViewTab),
                             getAutoFilterArea(rDocument, nDefaultViewTab));
        CPPUNIT_ASSERT_EQUAL(ScRange(0, 0, nSheetViewTab, 0, 3, nSheetViewTab),
                             getAutoFilterArea(rDocument, nSheetViewTab));
    }
}

CPPUNIT_TEST_FIXTURE(SyncTest, testSync_UndoSortOnSheetView)
{
    // Test case:
    // - Create sheet view
    // - Sort descending
    // - Sort ascending
    // - Undo
    // - Undo
    // Verify SheetView has correct sort data on each undo step

    ScModelObj* pModelObj = createDoc("SheetView_AutoFilter.ods");
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    ScDocument& rDocument = pDocShell->GetDocument();

    setupViews();

    // Create sheet view
    {
        switchToSheetView();
        createNewSheetViewInCurrentView();
    }

    SCTAB nDefaultViewTab = mpTabViewDefaultView->GetViewData().GetTabNumber();

    // Verify initial unsorted state on both views
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"3", u"7" }),
                         getValues(mpTabViewDefaultView, 0, 1, 4));
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"3", u"7" }),
                         getValues(mpTabViewSheetView, 0, 1, 4));

    // Sort descending on sheet view
    {
        switchToSheetView();
        sortDescendingForCell(u"A1");
    }

    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"7", u"5", u"4", u"3" }),
                         getValues(mpTabViewSheetView, 0, 1, 4));
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"3", u"7" }),
                         getValues(mpTabViewDefaultView, 0, 1, 4));

    std::shared_ptr<sc::SheetViewManager> pManager = rDocument.GetSheetViewManager(nDefaultViewTab);
    CPPUNIT_ASSERT(pManager);
    auto& rSheetView = *pManager->iterateValidSheetViews().begin();
    CPPUNIT_ASSERT(rSheetView.getSortOrder() != nullptr);
    CPPUNIT_ASSERT(rSheetView.getSortParam() != nullptr);

    // Sort ascending on sheet view (second sort)
    {
        switchToSheetView();
        sortAscendingForCell(u"A1");
    }

    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"3", u"4", u"5", u"7" }),
                         getValues(mpTabViewSheetView, 0, 1, 4));
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"3", u"7" }),
                         getValues(mpTabViewDefaultView, 0, 1, 4));

    // Undo ascending sort - should revert to descending
    {
        switchToSheetView();
        undo();
    }

    // After first undo, sheet view should show descending sort
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"7", u"5", u"4", u"3" }),
                         getValues(mpTabViewSheetView, 0, 1, 4));
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"3", u"7" }),
                         getValues(mpTabViewDefaultView, 0, 1, 4));

    // SheetView sort data should reflect descending sort
    CPPUNIT_ASSERT(rSheetView.getSortOrder() != nullptr);
    CPPUNIT_ASSERT(rSheetView.getSortParam() != nullptr);

    // Undo descending sort - should revert to original unsorted
    {
        switchToSheetView();
        undo();
    }

    // After second undo, sheet view data should be unsorted
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"3", u"7" }),
                         getValues(mpTabViewSheetView, 0, 1, 4));
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"3", u"7" }),
                         getValues(mpTabViewDefaultView, 0, 1, 4));

    // SheetView sort data should be fully reset
    CPPUNIT_ASSERT(rSheetView.getSortOrder() == nullptr);
    CPPUNIT_ASSERT(rSheetView.getSortParam() == nullptr);
}

CPPUNIT_TEST_FIXTURE(SyncTest, testSync_UndoSortOnDefaultView)
{
    // Test:
    // - create sheet view with sort
    // - sort default view
    // - undo default view sort
    // - edit on default view
    // - verify sheet view sync

    ScModelObj* pModelObj = createDoc("SheetView_AutoFilter.ods");
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    ScDocument& rDocument = pDocShell->GetDocument();

    setupViews();

    // Create sheet view and sort descending
    {
        switchToSheetView();
        createNewSheetViewInCurrentView();
        sortDescendingForCell(u"A1");
    }

    SCTAB nDefaultViewTab = mpTabViewDefaultView->GetViewData().GetTabNumber();

    // Initial state: default unsorted, sheet view descending
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"3", u"7" }),
                         getValues(mpTabViewDefaultView, 0, 1, 4));
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"7", u"5", u"4", u"3" }),
                         getValues(mpTabViewSheetView, 0, 1, 4));

    // Sort ascending on default view
    {
        switchToDefaultView();
        sortAscendingForCell(u"A1");
    }

    // Default view now ascending, sheet view still descending
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"3", u"4", u"5", u"7" }),
                         getValues(mpTabViewDefaultView, 0, 1, 4));
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"7", u"5", u"4", u"3" }),
                         getValues(mpTabViewSheetView, 0, 1, 4));

    // SheetViewManager should have sort data from the default view sort
    std::shared_ptr<sc::SheetViewManager> pManager = rDocument.GetSheetViewManager(nDefaultViewTab);
    CPPUNIT_ASSERT(pManager);
    CPPUNIT_ASSERT(pManager->getSortOrder() != nullptr);

    // Undo the default view sort
    {
        switchToDefaultView();
        undo();
    }

    // Default view should be back to original order
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"3", u"7" }),
                         getValues(mpTabViewDefaultView, 0, 1, 4));

    // Default view sort data should be restored (no sort)
    CPPUNIT_ASSERT(pManager->getSortOrder() == nullptr);

    // Sheet view should still be descending
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"7", u"5", u"4", u"3" }),
                         getValues(mpTabViewSheetView, 0, 1, 4));

    // Edit a cell on the default view to trigger sync - change A2
    {
        switchToDefaultView();
        typeCharsInCell(std::string("44"), 0, 1, mpTabViewDefaultView, pModelObj);
    }

    // Default view: A2=44 replaces 4
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"44", u"5", u"3", u"7" }),
                         getValues(mpTabViewDefaultView, 0, 1, 4));

    // Sheet view should be synced and still sorted descending with the new value
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"44", u"7", u"5", u"3" }),
                         getValues(mpTabViewSheetView, 0, 1, 4));
}

CPPUNIT_TEST_FIXTURE(SyncTest, testSync_RemoveAutoFilter_DefaultAndSheetView)
{
    // Test that removing auto-filter from default view removes it from sheet view
    // and resets the sort data in SheetView and SheetViewManager

    ScModelObj* pModelObj = createDoc("SheetView_AutoFilter.ods");
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    ScDocument& rDocument = pDocShell->GetDocument();

    setupViews();

    // Create sheet view and sort descending
    {
        switchToSheetView();
        createNewSheetViewInCurrentView();
        sortDescendingForCell(u"A1");
    }

    SCTAB nSheetViewTab = mpTabViewSheetView->GetViewData().GetTabNumber();
    SCTAB nDefaultViewTab = mpTabViewDefaultView->GetViewData().GetTabNumber();

    // Verify initial state
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"3", u"7" }),
                         getValues(mpTabViewDefaultView, 0, 1, 4));
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"7", u"5", u"4", u"3" }),
                         getValues(mpTabViewSheetView, 0, 1, 4));

    // Verify auto-filter exists on both views
    ScDBData* pDefaultDBData = rDocument.GetAnonymousDBData(nDefaultViewTab);
    CPPUNIT_ASSERT(pDefaultDBData);
    CPPUNIT_ASSERT(pDefaultDBData->HasAutoFilter());

    ScDBData* pSheetViewDBData = rDocument.GetAnonymousDBData(nSheetViewTab);
    CPPUNIT_ASSERT(pSheetViewDBData);
    CPPUNIT_ASSERT(pSheetViewDBData->HasAutoFilter());

    // Verify SheetView has sort data
    std::shared_ptr<sc::SheetViewManager> pManager = rDocument.GetSheetViewManager(nDefaultViewTab);
    CPPUNIT_ASSERT(pManager);
    CPPUNIT_ASSERT(!pManager->isEmpty());

    auto& rSheetView = *pManager->iterateValidSheetViews().begin();
    CPPUNIT_ASSERT(rSheetView.getSortOrder() != nullptr);
    CPPUNIT_ASSERT(rSheetView.getSortParam() != nullptr);

    // Remove auto-filter from default view
    {
        switchToDefaultView();
        gotoCell(u"A1");
        mpTabViewDefaultView->ToggleAutoFilter();
    }

    // Verify auto-filter is removed from default view
    pDefaultDBData = rDocument.GetAnonymousDBData(nDefaultViewTab);
    CPPUNIT_ASSERT(!pDefaultDBData || !pDefaultDBData->HasAutoFilter());

    // Verify auto-filter is removed from sheet view
    pSheetViewDBData = rDocument.GetAnonymousDBData(nSheetViewTab);
    CPPUNIT_ASSERT(!pSheetViewDBData || !pSheetViewDBData->HasAutoFilter());

    // Verify data is same in both views (unsorted, since auto-filter is gone)
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"3", u"7" }),
                         getValues(mpTabViewDefaultView, 0, 1, 4));
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"3", u"7" }),
                         getValues(mpTabViewSheetView, 0, 1, 4));

    // Verify SheetView sort data is reset
    CPPUNIT_ASSERT(rSheetView.getSortOrder() == nullptr);
    CPPUNIT_ASSERT(rSheetView.getSortParam() == nullptr);

    // Undo removing auto-filter from default view
    {
        switchToDefaultView();
        undo();
    }

    // Verify auto-filter is back on both views
    pDefaultDBData = rDocument.GetAnonymousDBData(nDefaultViewTab);
    CPPUNIT_ASSERT(pDefaultDBData);
    CPPUNIT_ASSERT(pDefaultDBData->HasAutoFilter());

    pSheetViewDBData = rDocument.GetAnonymousDBData(nSheetViewTab);
    CPPUNIT_ASSERT(pSheetViewDBData);
    CPPUNIT_ASSERT(pSheetViewDBData->HasAutoFilter());

    // Verify data is restored - both views show same data
    // (sort data was reset, so sheet view is no longer sorted)
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"3", u"7" }),
                         getValues(mpTabViewDefaultView, 0, 1, 4));
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"3", u"7" }),
                         getValues(mpTabViewSheetView, 0, 1, 4));

    // TODO: Sort data is not restored by undo - we expect it should be.
    CPPUNIT_ASSERT(rSheetView.getSortOrder() == nullptr);
    CPPUNIT_ASSERT(rSheetView.getSortParam() == nullptr);

    // Redo removing auto-filter
    {
        switchToDefaultView();
        redo();
    }

    // Verify auto-filter is removed again from both views
    pDefaultDBData = rDocument.GetAnonymousDBData(nDefaultViewTab);
    CPPUNIT_ASSERT(!pDefaultDBData || !pDefaultDBData->HasAutoFilter());

    pSheetViewDBData = rDocument.GetAnonymousDBData(nSheetViewTab);
    CPPUNIT_ASSERT(!pSheetViewDBData || !pSheetViewDBData->HasAutoFilter());

    // Verify data is unsorted on both views
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"3", u"7" }),
                         getValues(mpTabViewDefaultView, 0, 1, 4));
    CPPUNIT_ASSERT_EQUAL(expectedValues({ u"4", u"5", u"3", u"7" }),
                         getValues(mpTabViewSheetView, 0, 1, 4));

    // Verify SheetView sort data is reset again
    CPPUNIT_ASSERT(rSheetView.getSortOrder() == nullptr);
    CPPUNIT_ASSERT(rSheetView.getSortParam() == nullptr);
}

CPPUNIT_TEST_FIXTURE(SyncTest, testSync_PivotTable_DefaultAndSheetView)
{
    // Test sync to sheet view when creating and updating the pivot table

    ScModelObj* pModelObj = createDoc("empty.ods");
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    ScDocument& rDocument = pDocShell->GetDocument();
    setupPivotTableSourceData(rDocument);

    setupViews();

    // Create sheet view
    {
        switchToSheetView();
        createNewSheetViewInCurrentView();
    }

    SCTAB nSheetViewTab = mpTabViewSheetView->GetViewData().GetTabNumber();
    SCTAB nDefaultViewTab = mpTabViewDefaultView->GetViewData().GetTabNumber();

    // Create pivot table from default view
    {
        switchToDefaultView();

        ScDPObject aDPObject = createCategoryValueDPObject(
            rDocument, nDefaultViewTab, ScRange(ScAddress(4, 0, nDefaultViewTab)),
            css::sheet::DataPilotFieldOrientation_ROW);

        ScDBDocFunc aDBDocFunc(*pDocShell);
        bool bResult = aDBDocFunc.CreatePivotTable(aDPObject, true, true);
        CPPUNIT_ASSERT(bResult);

        // Verify pivot table output on default view - cell E1 should have content
        CPPUNIT_ASSERT(!rDocument.GetString(ScAddress(4, 0, nDefaultViewTab)).isEmpty());

        // Verify the pivot table output is synced to sheet view
        CPPUNIT_ASSERT_EQUAL(rDocument.GetString(ScAddress(4, 0, nDefaultViewTab)),
                             rDocument.GetString(ScAddress(4, 0, nSheetViewTab)));
        CPPUNIT_ASSERT_EQUAL(rDocument.GetString(ScAddress(4, 1, nDefaultViewTab)),
                             rDocument.GetString(ScAddress(4, 1, nSheetViewTab)));
    }

    // Update pivot table layout: change Category from ROW to COLUMN
    {
        switchToDefaultView();

        ScDPObject* pDefaultDPObject = findPivotTableOnTab(rDocument, nDefaultViewTab);
        CPPUNIT_ASSERT(pDefaultDPObject);

        // Change layout: move Category to COLUMN orientation
        ScDPSaveData* pSaveData = pDefaultDPObject->GetSaveData();
        CPPUNIT_ASSERT(pSaveData);
        ScDPSaveDimension* pDim = pSaveData->GetDimensionByName(u"Category"_ustr);
        CPPUNIT_ASSERT(pDim);
        pDim->SetOrientation(css::sheet::DataPilotFieldOrientation_COLUMN);

        ScDBDocFunc aDBDocFunc(*pDocShell);
        bool bResult = aDBDocFunc.UpdatePivotTable(*pDefaultDPObject, true, true);
        CPPUNIT_ASSERT(bResult);

        // Verify the sheet view pivot table has the updated COLUMN layout
        ScDPObject* pSheetViewDPObject = findPivotTableOnTab(rDocument, nSheetViewTab);
        CPPUNIT_ASSERT(pSheetViewDPObject);
        ScDPSaveDimension* pSheetViewDim
            = pSheetViewDPObject->GetSaveData()->GetDimensionByName(u"Category"_ustr);
        CPPUNIT_ASSERT_EQUAL(css::sheet::DataPilotFieldOrientation_COLUMN,
                             pSheetViewDim->GetOrientation());

        // Verify both pivot tables have the same layout
        CPPUNIT_ASSERT_EQUAL(*pDefaultDPObject->GetSaveData(), *pSheetViewDPObject->GetSaveData());
    }

    // Update pivot table layout from sheet view: change Category from COLUMN back to ROW
    {
        switchToSheetView();

        ScDPObject* pSheetViewDPObject = findPivotTableOnTab(rDocument, nSheetViewTab);
        CPPUNIT_ASSERT(pSheetViewDPObject);

        // Change layout: move Category back to ROW orientation
        ScDPSaveData* pSaveData = pSheetViewDPObject->GetSaveData();
        CPPUNIT_ASSERT(pSaveData);
        ScDPSaveDimension* pDim = pSaveData->GetDimensionByName(u"Category"_ustr);
        CPPUNIT_ASSERT(pDim);
        pDim->SetOrientation(css::sheet::DataPilotFieldOrientation_ROW);

        ScDBDocFunc aDBDocFunc(*pDocShell);
        bool bResult = aDBDocFunc.UpdatePivotTable(*pSheetViewDPObject, true, true);
        CPPUNIT_ASSERT(bResult);

        // Verify the default view pivot table has the updated ROW layout
        ScDPObject* pDefaultDPObject = findPivotTableOnTab(rDocument, nDefaultViewTab);
        CPPUNIT_ASSERT(pDefaultDPObject);
        ScDPSaveDimension* pDefaultDim
            = pDefaultDPObject->GetSaveData()->GetDimensionByName(u"Category"_ustr);
        CPPUNIT_ASSERT_EQUAL(css::sheet::DataPilotFieldOrientation_ROW,
                             pDefaultDim->GetOrientation());

        // Verify both pivot tables have the same layout
        CPPUNIT_ASSERT_EQUAL(*pDefaultDPObject->GetSaveData(), *pSheetViewDPObject->GetSaveData());
    }

    // Create pivot table from sheet view
    {
        switchToSheetView();

        ScDPObject aDPObject = createCategoryValueDPObject(
            rDocument, nSheetViewTab, ScRange(ScAddress(7, 0, nSheetViewTab)),
            css::sheet::DataPilotFieldOrientation_ROW);

        ScDBDocFunc aDBDocFunc(*pDocShell);
        bool bResult = aDBDocFunc.CreatePivotTable(aDPObject, true, true);
        CPPUNIT_ASSERT(bResult);

        // Verify pivot table created on default view - cell H1 should have content
        CPPUNIT_ASSERT(!rDocument.GetString(ScAddress(7, 0, nDefaultViewTab)).isEmpty());

        // Verify the pivot table output is synced to sheet view
        CPPUNIT_ASSERT_EQUAL(rDocument.GetString(ScAddress(7, 0, nDefaultViewTab)),
                             rDocument.GetString(ScAddress(7, 0, nSheetViewTab)));
        CPPUNIT_ASSERT_EQUAL(rDocument.GetString(ScAddress(7, 1, nDefaultViewTab)),
                             rDocument.GetString(ScAddress(7, 1, nSheetViewTab)));
    }

    // Remove first pivot table from default view
    {
        switchToDefaultView();

        ScDPObject* pDefaultDPObject = findPivotTableOnTab(rDocument, nDefaultViewTab);
        CPPUNIT_ASSERT(pDefaultDPObject);

        ScDBDocFunc aDBDocFunc(*pDocShell);
        bool bResult = aDBDocFunc.RemovePivotTable(*pDefaultDPObject, true, true);
        CPPUNIT_ASSERT(bResult);

        CPPUNIT_ASSERT_EQUAL(size_t(1), countPivotTablesOnTab(rDocument, nDefaultViewTab));
        CPPUNIT_ASSERT_EQUAL(size_t(1), countPivotTablesOnTab(rDocument, nSheetViewTab));
    }

    // Remove second pivot table from sheet view
    {
        switchToSheetView();

        ScDPObject* pSheetViewDPObject = findPivotTableOnTab(rDocument, nSheetViewTab);
        CPPUNIT_ASSERT(pSheetViewDPObject);

        ScDBDocFunc aDBDocFunc(*pDocShell);
        bool bResult = aDBDocFunc.RemovePivotTable(*pSheetViewDPObject, true, true);
        CPPUNIT_ASSERT(bResult);

        CPPUNIT_ASSERT_EQUAL(size_t(0), countPivotTablesOnTab(rDocument, nDefaultViewTab));
        CPPUNIT_ASSERT_EQUAL(size_t(0), countPivotTablesOnTab(rDocument, nSheetViewTab));
    }
}

CPPUNIT_TEST_FIXTURE(SyncTest, testSync_ReplacePivotTable_DefaultAndSheetView)
{
    // Test sync to sheet view when replacing a pivot table via DataPilotUpdate

    ScModelObj* pModelObj = createDoc("empty.ods");
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ScDocShell* pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    ScDocument& rDocument = pDocShell->GetDocument();
    setupPivotTableSourceData(rDocument);

    setupViews();

    // Create sheet view
    {
        switchToSheetView();
        createNewSheetViewInCurrentView();
    }

    SCTAB nSheetViewTab = mpTabViewSheetView->GetViewData().GetTabNumber();
    SCTAB nDefaultViewTab = mpTabViewDefaultView->GetViewData().GetTabNumber();

    // Create pivot table from default view
    {
        switchToDefaultView();

        ScDPObject aDPObject = createCategoryValueDPObject(
            rDocument, nDefaultViewTab, ScRange(ScAddress(4, 0, nDefaultViewTab)),
            css::sheet::DataPilotFieldOrientation_ROW);

        ScDBDocFunc aDBDocFunc(*pDocShell);
        bool bResult = aDBDocFunc.CreatePivotTable(aDPObject, true, true);
        CPPUNIT_ASSERT(bResult);
    }

    // Replace pivot table from default view: change Category from ROW to COLUMN
    {
        switchToDefaultView();

        ScDPObject* pDPObject = findPivotTableOnTab(rDocument, nDefaultViewTab);
        CPPUNIT_ASSERT(pDPObject);

        // Build a new DPObject with modified layout
        ScDPObject aNewDPObject
            = createCategoryValueDPObject(rDocument, nDefaultViewTab, pDPObject->GetOutRange(),
                                          css::sheet::DataPilotFieldOrientation_COLUMN);

        sc::ReplacePivotTableOperation aOperation(*pDocShell, pDPObject, &aNewDPObject, true, true,
                                                  false);
        CPPUNIT_ASSERT(aOperation.run());

        // Verify the sheet view pivot table has the updated COLUMN layout
        ScDPObject* pSheetViewDPObject = findPivotTableOnTab(rDocument, nSheetViewTab);
        CPPUNIT_ASSERT(pSheetViewDPObject);
        ScDPSaveDimension* pSheetViewDim
            = pSheetViewDPObject->GetSaveData()->GetDimensionByName(u"Category"_ustr);
        CPPUNIT_ASSERT_EQUAL(css::sheet::DataPilotFieldOrientation_COLUMN,
                             pSheetViewDim->GetOrientation());

        // Verify both pivot tables have the same layout
        CPPUNIT_ASSERT_EQUAL(*pDPObject->GetSaveData(), *pSheetViewDPObject->GetSaveData());
    }

    // Replace pivot table from sheet view: change Category from COLUMN back to ROW
    {
        switchToSheetView();

        ScDPObject* pSheetViewDPObject = findPivotTableOnTab(rDocument, nSheetViewTab);
        CPPUNIT_ASSERT(pSheetViewDPObject);

        // Build a new DPObject with modified layout
        ScDPObject aNewDPObject = createCategoryValueDPObject(
            rDocument, nSheetViewTab, pSheetViewDPObject->GetOutRange(),
            css::sheet::DataPilotFieldOrientation_ROW);

        sc::ReplacePivotTableOperation aOperation(*pDocShell, pSheetViewDPObject, &aNewDPObject,
                                                  true, true, false);
        CPPUNIT_ASSERT(aOperation.run());

        // Verify the default view pivot table has the updated ROW layout
        ScDPObject* pDefaultDPObject = findPivotTableOnTab(rDocument, nDefaultViewTab);
        CPPUNIT_ASSERT(pDefaultDPObject);
        ScDPSaveDimension* pDefaultDim
            = pDefaultDPObject->GetSaveData()->GetDimensionByName(u"Category"_ustr);
        CPPUNIT_ASSERT_EQUAL(css::sheet::DataPilotFieldOrientation_ROW,
                             pDefaultDim->GetOrientation());

        // Verify both pivot tables have the same layout
        CPPUNIT_ASSERT_EQUAL(*pDefaultDPObject->GetSaveData(), *pSheetViewDPObject->GetSaveData());
    }
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
