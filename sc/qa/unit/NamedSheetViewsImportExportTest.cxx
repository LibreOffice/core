/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapixml_test.hxx>
#include <comphelper/servicehelper.hxx>
#include <dbdata.hxx>
#include <document.hxx>
#include <docuno.hxx>
#include <queryparam.hxx>
#include <sortparam.hxx>
#include <SheetView.hxx>
#include <SheetViewManager.hxx>

class NamedSheetViewsImportExportTest : public UnoApiXmlTest
{
public:
    NamedSheetViewsImportExportTest()
        : UnoApiXmlTest(u"sc/qa/unit/data"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(NamedSheetViewsImportExportTest, testImportAndCheckState)
{
    loadFromFile(u"xlsx/NamedSheetViews.xlsx");

    ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    // Check sheet view manager
    auto pManager = pDoc->GetSheetViewManager(SCTAB(0));
    CPPUNIT_ASSERT(pManager);
    // 2 sheet views for sheet 1
    CPPUNIT_ASSERT_EQUAL(size_t(2), pManager->size());

    // 3 sheets total: Sheet1 + 2 sheet view tabs
    CPPUNIT_ASSERT_EQUAL(SCTAB(3), pDoc->GetTableCount());
}

CPPUNIT_TEST_FIXTURE(NamedSheetViewsImportExportTest, testRoundtripXLSX)
{
    loadFromFile(u"xlsx/NamedSheetViews.xlsx");

    // Loading is verified in a separate test so here we can assume it's fine

    save(u"Calc Office Open XML"_ustr);

    // Verify the exported XML structure
    xmlDocUniquePtr pNsv = parseExport(u"xl/namedSheetViews/namedSheetView1.xml"_ustr);
    CPPUNIT_ASSERT(pNsv);

    // Two named sheet views: "View1" and "View2"
    assertXPath(pNsv, "/xnsv:namedSheetViews/xnsv:namedSheetView", 2);
    assertXPath(pNsv, "/xnsv:namedSheetViews/xnsv:namedSheetView[1]", "name", u"View1");
    assertXPath(pNsv, "/xnsv:namedSheetViews/xnsv:namedSheetView[2]", "name", u"View2");

    // View1: nsvFilter sort rule on column 2, no column filters
    OString sView1 = "/xnsv:namedSheetViews/xnsv:namedSheetView[1]/xnsv:nsvFilter"_ostr;
    assertXPath(pNsv, sView1, 1);
    assertXPath(pNsv, sView1 + "/xnsv:columnFilter", 0);
    assertXPath(pNsv, sView1 + "/xnsv:sortRules/xnsv:sortRule", 1);
    assertXPath(pNsv, sView1 + "/xnsv:sortRules/xnsv:sortRule", "colId", u"2");
    assertXPath(pNsv, sView1 + "/xnsv:sortRules/xnsv:sortRule/xnsv:sortCondition", "ref", u"C1:C8");

    // View2: nsvFilter with 2 column filters and sort rule on column 0
    OString sView2 = "/xnsv:namedSheetViews/xnsv:namedSheetView[2]/xnsv:nsvFilter"_ostr;
    assertXPath(pNsv, sView2, 1);
    assertXPath(pNsv, sView2 + "/xnsv:columnFilter", 2);

    // Column filter on ID 1
    assertXPath(pNsv, sView2 + "/xnsv:columnFilter[1]", "colId", u"1");
    assertXPath(pNsv, sView2 + "/xnsv:columnFilter[1]/xnsv:filter/x:filters/x:filter", 4);

    // Column filter on ID 2
    assertXPath(pNsv, sView2 + "/xnsv:columnFilter[2]", "colId", u"2");
    assertXPath(pNsv, sView2 + "/xnsv:columnFilter[2]/xnsv:filter/x:filters/x:dateGroupItem", 3);

    // Sort rule on column 0
    assertXPath(pNsv, sView2 + "/xnsv:sortRules/xnsv:sortRule", 1);
    assertXPath(pNsv, sView2 + "/xnsv:sortRules/xnsv:sortRule", "colId", u"0");
    assertXPath(pNsv, sView2 + "/xnsv:sortRules/xnsv:sortRule/xnsv:sortCondition", "ref", u"A1:A8");
}

CPPUNIT_TEST_FIXTURE(NamedSheetViewsImportExportTest, testRoundtripModelState)
{
    loadFromFile(u"xlsx/NamedSheetViews.xlsx");

    // Save and reload to test full round-trip
    saveAndReload(u"Calc Office Open XML"_ustr);

    ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    // Check after roundtrip

    // 3 tabs should exist
    SCTAB nTabCount = pDoc->GetTableCount();
    CPPUNIT_ASSERT_EQUAL(SCTAB(3), nTabCount);

    // Tab index 0 should be visible, the sheet view tabs should be hidden
    CPPUNIT_ASSERT(pDoc->IsVisible(0));
    CPPUNIT_ASSERT(!pDoc->IsVisible(1)); // sheet view 1
    CPPUNIT_ASSERT(!pDoc->IsVisible(2)); // sheet view 2

    // Sheet view tabs should be sheet view holders
    CPPUNIT_ASSERT(!pDoc->IsSheetViewHolder(0));
    CPPUNIT_ASSERT(pDoc->IsSheetViewHolder(1));
    CPPUNIT_ASSERT(pDoc->IsSheetViewHolder(2));

    // Sheet views should exist on tab index 0 - associated with 2 sheet views
    auto pManager = pDoc->GetSheetViewManager(0);
    CPPUNIT_ASSERT(pManager);
    CPPUNIT_ASSERT_EQUAL(size_t(2), pManager->size());

    // Find the tab index for each view by name
    SCTAB nView1Tab = -1;
    SCTAB nView2Tab = -1;
    for (auto& rSheetView : pManager->iterateValidSheetViews())
    {
        if (rSheetView.GetName() == u"View1")
            nView1Tab = rSheetView.getTableNumber();
        else if (rSheetView.GetName() == u"View2")
            nView2Tab = rSheetView.getTableNumber();
    }
    CPPUNIT_ASSERT(nView1Tab >= 0);
    CPPUNIT_ASSERT(nView2Tab >= 0);

    // Sheet View 1: has sort on column 2
    {
        ScDBData* pDBData = pDoc->GetAnonymousDBData(nView1Tab);
        CPPUNIT_ASSERT(pDBData);

        ScSortParam aSortParam;
        pDBData->GetSortParam(aSortParam);
        CPPUNIT_ASSERT(aSortParam.maKeyState[0].bDoSort);
        CPPUNIT_ASSERT_EQUAL(SCCOLROW(2), aSortParam.maKeyState[0].nField);
    }

    // Sheet View 2: has column filters and sort on column 0, with rows filtered
    {
        ScDBData* pDBData = pDoc->GetAnonymousDBData(nView2Tab);
        CPPUNIT_ASSERT(pDBData);

        ScQueryParam aQueryParam;
        pDBData->GetQueryParam(aQueryParam);
        // Should have at least one active query entry
        CPPUNIT_ASSERT(aQueryParam.GetEntry(0).bDoQuery);

        ScSortParam aSortParam;
        pDBData->GetSortParam(aSortParam);
        CPPUNIT_ASSERT(aSortParam.maKeyState[0].bDoSort);
        CPPUNIT_ASSERT_EQUAL(SCCOLROW(0), aSortParam.maKeyState[0].nField);

        // At least one data row should be hidden by the filter
        bool bHasHiddenRow = false;
        for (SCROW nRow = 1; nRow <= 7; ++nRow)
        {
            if (pDoc->RowHidden(nRow, nView2Tab))
            {
                bHasHiddenRow = true;
                break;
            }
        }
        CPPUNIT_ASSERT_MESSAGE("Sheet View 2 should have at least one filtered row", bHasHiddenRow);
    }
}

CPPUNIT_TEST_FIXTURE(NamedSheetViewsImportExportTest, testMultiSheetRoundtripVisibility)
{
    loadFromFile(u"xlsx/NamedSheetViews.xlsx");

    // After loading, the document already has 3 tabs: Sheet1 + 2 view tabs.
    // Insert Sheet2 at the end so that after round-trip it tests that
    // view tab creation doesn't break visibility of subsequent sheets.
    {
        ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
        CPPUNIT_ASSERT(pModelObj);
        ScDocument* pDoc = pModelObj->GetDocument();
        CPPUNIT_ASSERT(pDoc);

        // Add a new tab (Sheet2) at the end
        pDoc->InsertTab(pDoc->GetTableCount(), u"Sheet2"_ustr);
        CPPUNIT_ASSERT_EQUAL(SCTAB(4), pDoc->GetTableCount());
    }

    // Roundtrip it
    saveAndReload(u"Calc Office Open XML"_ustr);

    // Check the state after roundtrip
    {
        ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
        CPPUNIT_ASSERT(pModelObj);
        ScDocument* pDoc = pModelObj->GetDocument();
        CPPUNIT_ASSERT(pDoc);

        // 2 Sheets, Sheet 1 has 2 sheet views
        SCTAB nTabCount = pDoc->GetTableCount();
        CPPUNIT_ASSERT_EQUAL(SCTAB(4), nTabCount);

        // Sheet1 (index 0) should be visible
        CPPUNIT_ASSERT(pDoc->IsVisible(0));

        // Sheet view tabs (index 1, 2) should be hidden and should be sheet view holders
        CPPUNIT_ASSERT(!pDoc->IsVisible(1));
        CPPUNIT_ASSERT(!pDoc->IsVisible(2));
        CPPUNIT_ASSERT(pDoc->IsSheetViewHolder(1));
        CPPUNIT_ASSERT(pDoc->IsSheetViewHolder(2));

        // Sheet2 (index 3) should be visible as it's a normal sheet
        CPPUNIT_ASSERT(pDoc->IsVisible(3));
        CPPUNIT_ASSERT(!pDoc->IsSheetViewHolder(3));

        // Sheet1 (index 0) should be associated with 2 sheet views
        auto pManager = pDoc->GetSheetViewManager(0);
        CPPUNIT_ASSERT(pManager);
        CPPUNIT_ASSERT_EQUAL(size_t(2), pManager->size());
    }
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
