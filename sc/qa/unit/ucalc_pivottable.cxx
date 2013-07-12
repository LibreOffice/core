/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ucalc.hxx"
#include "dpshttab.hxx"
#include "dpobject.hxx"
#include "dpsave.hxx"
#include "dpdimsave.hxx"
#include "dpcache.hxx"
#include "dpfilteredcache.hxx"
#include "scopetools.hxx"
#include "queryentry.hxx"
#include "stringutil.hxx"
#include "dbdocfun.hxx"

#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include <com/sun/star/sheet/DataPilotFieldGroupBy.hpp>
#include <com/sun/star/sheet/DataPilotFieldReference.hpp>
#include <com/sun/star/sheet/DataPilotFieldReferenceType.hpp>
#include <com/sun/star/sheet/DataPilotFieldReferenceItemType.hpp>
#include <com/sun/star/sheet/GeneralFunction.hpp>

namespace {

struct DPFieldDef
{
    const char* pName;
    sheet::DataPilotFieldOrientation eOrient;

    /**
     * Function for data field.  It's used only for data field.  When 0, the
     * default function (SUM) is used.
     */
    int eFunc;
};

template<size_t _Size>
ScRange insertDPSourceData(ScDocument* pDoc, DPFieldDef aFields[], size_t nFieldCount, const char* aData[][_Size], size_t nDataCount)
{
    // Insert field names in row 0.
    for (size_t i = 0; i < nFieldCount; ++i)
        pDoc->SetString(static_cast<SCCOL>(i), 0, 0, OUString(aFields[i].pName, strlen(aFields[i].pName), RTL_TEXTENCODING_UTF8));

    // Insert data into row 1 and downward.
    for (size_t i = 0; i < nDataCount; ++i)
    {
        SCROW nRow = static_cast<SCROW>(i) + 1;
        for (size_t j = 0; j < nFieldCount; ++j)
        {
            SCCOL nCol = static_cast<SCCOL>(j);
            pDoc->SetString(
                nCol, nRow, 0, OUString(aData[i][j], strlen(aData[i][j]), RTL_TEXTENCODING_UTF8));
        }
    }

    SCROW nRow1 = 0, nRow2 = 0;
    SCCOL nCol1 = 0, nCol2 = 0;
    pDoc->GetDataArea(0, nCol1, nRow1, nCol2, nRow2, true, false);
    CPPUNIT_ASSERT_MESSAGE("Data is expected to start from (col=0,row=0).", nCol1 == 0 && nRow1 == 0);
    CPPUNIT_ASSERT_MESSAGE("Unexpected data range.",
                           nCol2 == static_cast<SCCOL>(nFieldCount - 1) && nRow2 == static_cast<SCROW>(nDataCount));

    ScRange aSrcRange(nCol1, nRow1, 0, nCol2, nRow2, 0);
    Test::printRange(pDoc, aSrcRange, "Data sheet content");
    return aSrcRange;
}

template<size_t _Size>
bool checkDPTableOutput(ScDocument* pDoc, const ScRange& aOutRange, const char* aOutputCheck[][_Size], const char* pCaption)
{
    bool bResult = true;
    const ScAddress& s = aOutRange.aStart;
    const ScAddress& e = aOutRange.aEnd;
    SheetPrinter printer(e.Row() - s.Row() + 1, e.Col() - s.Col() + 1);
    SCROW nOutRowSize = e.Row() - s.Row() + 1;
    SCCOL nOutColSize = e.Col() - s.Col() + 1;
    for (SCROW nRow = 0; nRow < nOutRowSize; ++nRow)
    {
        for (SCCOL nCol = 0; nCol < nOutColSize; ++nCol)
        {
            OUString aVal = pDoc->GetString(nCol + s.Col(), nRow + s.Row(), s.Tab());
            printer.set(nRow, nCol, aVal);
            const char* p = aOutputCheck[nRow][nCol];
            if (p)
            {
                OUString aCheckVal = OUString::createFromAscii(p);
                bool bEqual = aCheckVal.equals(aVal);
                if (!bEqual)
                {
                    cout << "Expected: " << aCheckVal << "  Actual: " << aVal << endl;
                    bResult = false;
                }
            }
            else if (!aVal.isEmpty())
            {
                cout << "Empty cell expected" << endl;
                bResult = false;
            }
        }
    }
    printer.print(pCaption);
    return bResult;
}

ScDPObject* createDPFromSourceDesc(
    ScDocument* pDoc, const ScSheetSourceDesc& rDesc, DPFieldDef aFields[], size_t nFieldCount,
    bool bFilterButton)
{
    ScDPObject* pDPObj = new ScDPObject(pDoc);
    pDPObj->SetSheetDesc(rDesc);
    pDPObj->SetOutRange(ScAddress(0, 0, 1));

    ScDPSaveData aSaveData;
    // Set data pilot table output options.
    aSaveData.SetIgnoreEmptyRows(false);
    aSaveData.SetRepeatIfEmpty(false);
    aSaveData.SetColumnGrand(true);
    aSaveData.SetRowGrand(true);
    aSaveData.SetFilterButton(bFilterButton);
    aSaveData.SetDrillDown(true);

    // Check the sanity of the source range.
    const ScRange& rSrcRange = rDesc.GetSourceRange();
    SCROW nRow1 = rSrcRange.aStart.Row();
    SCROW nRow2 = rSrcRange.aEnd.Row();
    CPPUNIT_ASSERT_MESSAGE("source range contains no data!", nRow2 - nRow1 > 1);

    // Set the dimension information.
    for (size_t i = 0; i < nFieldCount; ++i)
    {
        OUString aDimName = OUString::createFromAscii(aFields[i].pName);
        ScDPSaveDimension* pDim = aSaveData.GetNewDimensionByName(aDimName);
        pDim->SetOrientation(static_cast<sal_uInt16>(aFields[i].eOrient));
        pDim->SetUsedHierarchy(0);

        if (aFields[i].eOrient == sheet::DataPilotFieldOrientation_DATA)
        {
            sheet::GeneralFunction eFunc = sheet::GeneralFunction_SUM;
            if (aFields[i].eFunc)
                eFunc = static_cast<sheet::GeneralFunction>(aFields[i].eFunc);

            pDim->SetFunction(eFunc);
            pDim->SetReferenceValue(NULL);
        }
        else
        {
            sheet::DataPilotFieldSortInfo aSortInfo;
            aSortInfo.IsAscending = true;
            aSortInfo.Mode = 2;
            pDim->SetSortInfo(&aSortInfo);

            sheet::DataPilotFieldLayoutInfo aLayInfo;
            aLayInfo.LayoutMode = 0;
            aLayInfo.AddEmptyLines = false;
            pDim->SetLayoutInfo(&aLayInfo);
            sheet::DataPilotFieldAutoShowInfo aShowInfo;
            aShowInfo.IsEnabled = false;
            aShowInfo.ShowItemsMode = 0;
            aShowInfo.ItemCount = 0;
            pDim->SetAutoShowInfo(&aShowInfo);
        }
    }

    // Don't forget the data layout dimension.
    ScDPSaveDimension* pDim = aSaveData.GetDataLayoutDimension();
    pDim->SetOrientation(sheet::DataPilotFieldOrientation_ROW);
    pDim->SetShowEmpty(true);

    pDPObj->SetSaveData(aSaveData);
    pDPObj->InvalidateData();

    return pDPObj;
}

ScDPObject* createDPFromRange(
    ScDocument* pDoc, const ScRange& rRange, DPFieldDef aFields[], size_t nFieldCount,
    bool bFilterButton)
{
    ScSheetSourceDesc aSheetDesc(pDoc);
    aSheetDesc.SetSourceRange(rRange);
    return createDPFromSourceDesc(pDoc, aSheetDesc, aFields, nFieldCount, bFilterButton);
}

ScRange refresh(ScDPObject* pDPObj)
{
    bool bOverFlow = false;
    ScRange aOutRange = pDPObj->GetNewOutputRange(bOverFlow);
    CPPUNIT_ASSERT_MESSAGE("Table overflow!?", !bOverFlow);

    pDPObj->Output(aOutRange.aStart);
    aOutRange = pDPObj->GetOutRange();
    return aOutRange;
}

ScRange refreshGroups(ScDPCollection* pDPs, ScDPObject* pDPObj)
{
    // We need to first create group data in the cache, then the group data in
    // the object.
    std::set<ScDPObject*> aRefs;
    bool bSuccess = pDPs->ReloadGroupsInCache(pDPObj, aRefs);
    CPPUNIT_ASSERT_MESSAGE("Failed to reload group data in cache.", bSuccess);
    CPPUNIT_ASSERT_MESSAGE("There should be only one table linked to this cache.", aRefs.size() == 1);
    pDPObj->ReloadGroupTableData();

    return refresh(pDPObj);
}

}

void Test::testPivotTable()
{
    m_pDoc->InsertTab(0, OUString("Data"));
    m_pDoc->InsertTab(1, OUString("Table"));

    // Dimension definition
    DPFieldDef aFields[] = {
        { "Name",  sheet::DataPilotFieldOrientation_ROW, 0 },
        { "Group", sheet::DataPilotFieldOrientation_COLUMN, 0 },
        { "Score", sheet::DataPilotFieldOrientation_DATA, 0 }
    };

    // Raw data
    const char* aData[][3] = {
        { "Andy",    "A", "30" },
        { "Bruce",   "A", "20" },
        { "Charlie", "B", "45" },
        { "David",   "B", "12" },
        { "Edward",  "C",  "8" },
        { "Frank",   "C", "15" },
    };

    size_t nFieldCount = SAL_N_ELEMENTS(aFields);
    size_t nDataCount = SAL_N_ELEMENTS(aData);

    ScRange aSrcRange = insertDPSourceData(m_pDoc, aFields, nFieldCount, aData, nDataCount);
    SCROW nRow1 = aSrcRange.aStart.Row(), nRow2 = aSrcRange.aEnd.Row();
    SCCOL nCol1 = aSrcRange.aStart.Col(), nCol2 = aSrcRange.aEnd.Col();

    ScDPObject* pDPObj = createDPFromRange(
        m_pDoc, ScRange(nCol1, nRow1, 0, nCol2, nRow2, 0), aFields, nFieldCount, false);

    ScDPCollection* pDPs = m_pDoc->GetDPCollection();
    bool bSuccess = pDPs->InsertNewTable(pDPObj);
    CPPUNIT_ASSERT_MESSAGE("failed to insert a new datapilot object into document", bSuccess);
    CPPUNIT_ASSERT_MESSAGE("there should be only one data pilot table.",
                           pDPs->GetCount() == 1);
    pDPObj->SetName(pDPs->CreateNewName());

    bool bOverFlow = false;
    ScRange aOutRange = pDPObj->GetNewOutputRange(bOverFlow);
    CPPUNIT_ASSERT_MESSAGE("Table overflow!?", !bOverFlow);

    pDPObj->Output(aOutRange.aStart);
    aOutRange = pDPObj->GetOutRange();
    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][5] = {
            { "Sum - Score", "Group", 0, 0, 0 },
            { "Name", "A", "B", "C", "Total Result" },
            { "Andy", "30", 0, 0, "30" },
            { "Bruce", "20", 0, 0, "20" },
            { "Charlie", 0, "45", 0, "45" },
            { "David", 0, "12", 0, "12" },
            { "Edward", 0, 0, "8", "8" },
            { "Frank", 0, 0, "15", "15" },
            { "Total Result", "50", "57", "23", "130" }
        };

        bSuccess = checkDPTableOutput<5>(m_pDoc, aOutRange, aOutputCheck, "DataPilot table output");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }
    CPPUNIT_ASSERT_MESSAGE("There should be only one data cache.", pDPs->GetSheetCaches().size() == 1);

    // Update the cell values.
    double aData2[] = { 100, 200, 300, 400, 500, 600 };
    for (size_t i = 0; i < SAL_N_ELEMENTS(aData2); ++i)
    {
        SCROW nRow = i + 1;
        m_pDoc->SetValue(2, nRow, 0, aData2[i]);
    }

    printRange(m_pDoc, ScRange(nCol1, nRow1, 0, nCol2, nRow2, 0), "Data sheet content (modified)");

    // Now, create a copy of the datapilot object for the updated table, but
    // don't reload the cache which should force the copy to use the old data
    // from the cache.
    ScDPObject* pDPObj2 = new ScDPObject(*pDPObj);
    pDPs->InsertNewTable(pDPObj2);

    aOutRange = pDPObj2->GetOutRange();
    pDPObj2->ClearTableData();
    pDPObj2->Output(aOutRange.aStart);
    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][5] = {
            { "Sum - Score", "Group", 0, 0, 0 },
            { "Name", "A", "B", "C", "Total Result" },
            { "Andy", "30", 0, 0, "30" },
            { "Bruce", "20", 0, 0, "20" },
            { "Charlie", 0, "45", 0, "45" },
            { "David", 0, "12", 0, "12" },
            { "Edward", 0, 0, "8", "8" },
            { "Frank", 0, 0, "15", "15" },
            { "Total Result", "50", "57", "23", "130" }
        };

        bSuccess = checkDPTableOutput<5>(m_pDoc, aOutRange, aOutputCheck, "DataPilot table output (from old cache)");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    CPPUNIT_ASSERT_MESSAGE("There should be only one data cache.", pDPs->GetSheetCaches().size() == 1);

    // Free the first datapilot object after the 2nd one gets reloaded, to
    // prevent the data cache from being deleted before the reload.
    pDPs->FreeTable(pDPObj);

    CPPUNIT_ASSERT_MESSAGE("There should be only one data cache.", pDPs->GetSheetCaches().size() == 1);

    // This time clear the cache to refresh the data from the source range.
    CPPUNIT_ASSERT_MESSAGE("This datapilot should be based on sheet data.", pDPObj2->IsSheetData());
    std::set<ScDPObject*> aRefs;
    sal_uLong nErrId = pDPs->ReloadCache(pDPObj2, aRefs);
    CPPUNIT_ASSERT_MESSAGE("Cache reload failed.", nErrId == 0);
    CPPUNIT_ASSERT_MESSAGE("Reloading a cache shouldn't remove any cache.",
                           pDPs->GetSheetCaches().size() == 1);

    pDPObj2->ClearTableData();
    pDPObj2->Output(aOutRange.aStart);

    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][5] = {
            { "Sum - Score", "Group", 0, 0, 0 },
            { "Name", "A", "B", "C", "Total Result" },
            { "Andy", "100", 0, 0, "100" },
            { "Bruce", "200", 0, 0, "200" },
            { "Charlie", 0, "300", 0, "300" },
            { "David", 0, "400", 0, "400" },
            { "Edward", 0, 0, "500", "500" },
            { "Frank", 0, 0, "600", "600" },
            { "Total Result", "300", "700", "1100", "2100" }
        };

        bSuccess = checkDPTableOutput<5>(m_pDoc, aOutRange, aOutputCheck, "DataPilot table output (refreshed)");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    CPPUNIT_ASSERT_MESSAGE("Cache should be here.", pDPs->GetSheetCaches().hasCache(aSrcRange));

    // Swap the two sheets.
    m_pDoc->MoveTab(1, 0);
    CPPUNIT_ASSERT_MESSAGE("Swapping the sheets shouldn't remove the cache.",
                           pDPs->GetSheetCaches().size() == 1);
    CPPUNIT_ASSERT_MESSAGE("Cache should have moved.", !pDPs->GetSheetCaches().hasCache(aSrcRange));
    aSrcRange.aStart.SetTab(1);
    aSrcRange.aEnd.SetTab(1);
    CPPUNIT_ASSERT_MESSAGE("Cache should be here.", pDPs->GetSheetCaches().hasCache(aSrcRange));

    pDPs->FreeTable(pDPObj2);
    CPPUNIT_ASSERT_MESSAGE("There shouldn't be any data pilot table stored with the document.",
                           pDPs->GetCount() == 0);

    CPPUNIT_ASSERT_MESSAGE("There shouldn't be any more data cache.",
                           pDPs->GetSheetCaches().size() == 0);

    // Insert a brand new pivot table object once again, but this time, don't
    // create the output to avoid creating a data cache.
    m_pDoc->DeleteTab(1);
    m_pDoc->InsertTab(1, OUString("Table"));

    pDPObj = createDPFromRange(
        m_pDoc, ScRange(nCol1, nRow1, 0, nCol2, nRow2, 0), aFields, nFieldCount, false);
    bSuccess = pDPs->InsertNewTable(pDPObj);
    CPPUNIT_ASSERT_MESSAGE("failed to insert a new datapilot object into document", bSuccess);
    CPPUNIT_ASSERT_MESSAGE("there should be only one data pilot table.",
                           pDPs->GetCount() == 1);
    pDPObj->SetName(pDPs->CreateNewName());
    CPPUNIT_ASSERT_MESSAGE("Data cache shouldn't exist yet before creating the table output.",
                           pDPs->GetSheetCaches().size() == 0);

    // Now, "refresh" the table.  This should still return a reference to self
    // even with the absence of data cache.
    aRefs.clear();
    pDPs->ReloadCache(pDPObj, aRefs);
    CPPUNIT_ASSERT_MESSAGE("It should return the same object as a reference.",
                           aRefs.size() == 1 && *aRefs.begin() == pDPObj);

    pDPs->FreeTable(pDPObj);

    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

void Test::testPivotTableLabels()
{
    m_pDoc->InsertTab(0, OUString("Data"));
    m_pDoc->InsertTab(1, OUString("Table"));

    // Dimension definition
    DPFieldDef aFields[] = {
        { "Software", sheet::DataPilotFieldOrientation_ROW, 0 },
        { "Version",  sheet::DataPilotFieldOrientation_COLUMN, 0 },
        { "1.2.3",    sheet::DataPilotFieldOrientation_DATA, 0 }
    };

    // Raw data
    const char* aData[][3] = {
        { "LibreOffice", "3.3.0", "30" },
        { "LibreOffice", "3.3.1", "20" },
        { "LibreOffice", "3.4.0", "45" },
    };

    size_t nFieldCount = SAL_N_ELEMENTS(aFields);
    size_t nDataCount = SAL_N_ELEMENTS(aData);

    ScRange aSrcRange = insertDPSourceData(m_pDoc, aFields, nFieldCount, aData, nDataCount);
    SCROW nRow1 = aSrcRange.aStart.Row(), nRow2 = aSrcRange.aEnd.Row();
    SCCOL nCol1 = aSrcRange.aStart.Col(), nCol2 = aSrcRange.aEnd.Col();

    ScDPObject* pDPObj = createDPFromRange(
        m_pDoc, ScRange(nCol1, nRow1, 0, nCol2, nRow2, 0), aFields, nFieldCount, false);

    ScDPCollection* pDPs = m_pDoc->GetDPCollection();
    bool bSuccess = pDPs->InsertNewTable(pDPObj);
    CPPUNIT_ASSERT_MESSAGE("failed to insert a new datapilot object into document", bSuccess);
    CPPUNIT_ASSERT_MESSAGE("there should be only one data pilot table.",
                           pDPs->GetCount() == 1);
    pDPObj->SetName(pDPs->CreateNewName());

    ScRange aOutRange = refresh(pDPObj);
    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][5] = {
            { "Sum - 1.2.3", "Version", 0, 0, 0 },
            { "Software", "3.3.0", "3.3.1", "3.4.0", "Total Result" },
            { "LibreOffice", "30", "20", "45", "95" },
            { "Total Result", "30", "20", "45", "95" }
        };

        bSuccess = checkDPTableOutput<5>(m_pDoc, aOutRange, aOutputCheck, "DataPilot table output");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    pDPs->FreeTable(pDPObj);

    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

void Test::testPivotTableDateLabels()
{
    m_pDoc->InsertTab(0, OUString("Data"));
    m_pDoc->InsertTab(1, OUString("Table"));

    // Dimension definition
    DPFieldDef aFields[] = {
        { "Name",  sheet::DataPilotFieldOrientation_ROW, 0 },
        { "Date",  sheet::DataPilotFieldOrientation_COLUMN, 0 },
        { "Value", sheet::DataPilotFieldOrientation_DATA, 0 }
    };

    // Raw data
    const char* aData[][3] = {
        { "Zena",   "2011-1-1", "30" },
        { "Yodel",  "2011-1-2", "20" },
        { "Xavior", "2011-1-3", "45" }
    };

    size_t nFieldCount = SAL_N_ELEMENTS(aFields);
    size_t nDataCount = SAL_N_ELEMENTS(aData);

    ScRange aSrcRange = insertDPSourceData(m_pDoc, aFields, nFieldCount, aData, nDataCount);
    SCROW nRow1 = aSrcRange.aStart.Row(), nRow2 = aSrcRange.aEnd.Row();
    SCCOL nCol1 = aSrcRange.aStart.Col(), nCol2 = aSrcRange.aEnd.Col();

    ScDPObject* pDPObj = createDPFromRange(
        m_pDoc, ScRange(nCol1, nRow1, 0, nCol2, nRow2, 0), aFields, nFieldCount, false);

    ScDPCollection* pDPs = m_pDoc->GetDPCollection();
    bool bSuccess = pDPs->InsertNewTable(pDPObj);
    CPPUNIT_ASSERT_MESSAGE("failed to insert a new datapilot object into document", bSuccess);
    CPPUNIT_ASSERT_MESSAGE("there should be only one data pilot table.",
                           pDPs->GetCount() == 1);
    pDPObj->SetName(pDPs->CreateNewName());

    ScRange aOutRange = refresh(pDPObj);
    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][5] = {
            { "Sum - Value", "Date", 0, 0, 0 },
            { "Name", "2011-01-01", "2011-01-02", "2011-01-03", "Total Result" },
            { "Xavior",  0, 0, "45", "45" },
            { "Yodel",  0, "20", 0, "20" },
            { "Zena",  "30", 0, 0, "30" },
            { "Total Result", "30", "20", "45", "95" }
        };

        bSuccess = checkDPTableOutput<5>(m_pDoc, aOutRange, aOutputCheck, "DataPilot table output");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    {
        const char* aChecks[] = {
            "2011-01-01", "2011-01-02", "2011-01-03"
        };

        // Make sure those cells that contain dates are numeric.
        SCROW nRow = aOutRange.aStart.Row() + 1;
        nCol1 = aOutRange.aStart.Col() + 1;
        nCol2 = nCol1 + 2;
        for (SCCOL nCol = nCol1; nCol <= nCol2; ++nCol)
        {
            OUString aVal = m_pDoc->GetString(nCol, nRow, 1);
            CPPUNIT_ASSERT_MESSAGE("Cell value is not as expected.", aVal.equalsAscii(aChecks[nCol-nCol1]));
            CPPUNIT_ASSERT_MESSAGE("This cell contains a date value and is supposed to be numeric.",
                                   m_pDoc->HasValueData(nCol, nRow, 1));
        }
    }

    pDPs->FreeTable(pDPObj);

    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

void Test::testPivotTableFilters()
{
    m_pDoc->InsertTab(0, OUString("Data"));
    m_pDoc->InsertTab(1, OUString("Table"));

    // Dimension definition
    DPFieldDef aFields[] = {
        { "Name",   sheet::DataPilotFieldOrientation_HIDDEN, 0 },
        { "Group1", sheet::DataPilotFieldOrientation_HIDDEN, 0 },
        { "Group2", sheet::DataPilotFieldOrientation_PAGE, 0 },
        { "Val1",   sheet::DataPilotFieldOrientation_DATA, 0 },
        { "Val2",   sheet::DataPilotFieldOrientation_DATA, 0 }
    };

    // Raw data
    const char* aData[][5] = {
        { "A", "1", "A", "1", "10" },
        { "B", "1", "A", "1", "10" },
        { "C", "1", "B", "1", "10" },
        { "D", "1", "B", "1", "10" },
        { "E", "2", "A", "1", "10" },
        { "F", "2", "A", "1", "10" },
        { "G", "2", "B", "1", "10" },
        { "H", "2", "B", "1", "10" }
    };

    size_t nFieldCount = SAL_N_ELEMENTS(aFields);
    size_t nDataCount = SAL_N_ELEMENTS(aData);

    ScRange aSrcRange = insertDPSourceData(m_pDoc, aFields, nFieldCount, aData, nDataCount);
    SCROW nRow1 = aSrcRange.aStart.Row(), nRow2 = aSrcRange.aEnd.Row();
    SCCOL nCol1 = aSrcRange.aStart.Col(), nCol2 = aSrcRange.aEnd.Col();

    ScDPObject* pDPObj = createDPFromRange(
        m_pDoc, ScRange(nCol1, nRow1, 0, nCol2, nRow2, 0), aFields, nFieldCount, true);

    ScDPCollection* pDPs = m_pDoc->GetDPCollection();
    bool bSuccess = pDPs->InsertNewTable(pDPObj);
    CPPUNIT_ASSERT_MESSAGE("failed to insert a new datapilot object into document", bSuccess);
    CPPUNIT_ASSERT_MESSAGE("there should be only one data pilot table.",
                           pDPs->GetCount() == 1);
    pDPObj->SetName(pDPs->CreateNewName());

    ScRange aOutRange = refresh(pDPObj);
    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][2] = {
            { "Filter", 0 },
            { "Group2", "- all -" },
            { 0, 0 },
            { "Data", 0 },
            { "Sum - Val1", "8" },
            { "Sum - Val2", "80" }
        };

        bSuccess = checkDPTableOutput<2>(m_pDoc, aOutRange, aOutputCheck, "DataPilot table output (unfiltered)");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    sc::AutoCalcSwitch aACSwitch(*m_pDoc, true); // turn on auto calculation.

    ScAddress aFormulaAddr = aOutRange.aEnd;
    aFormulaAddr.IncRow(2);
    m_pDoc->SetString(aFormulaAddr.Col(), aFormulaAddr.Row(), aFormulaAddr.Tab(),
                      OUString("=B6"));
    double fTest = m_pDoc->GetValue(aFormulaAddr);
    CPPUNIT_ASSERT_MESSAGE("Incorrect formula value that references a cell in the pivot table output.", fTest == 80.0);

    // Set current page of 'Group2' to 'A'.
    pDPObj->BuildAllDimensionMembers();
    ScDPSaveData aSaveData(*pDPObj->GetSaveData());
    ScDPSaveDimension* pPageDim = aSaveData.GetDimensionByName(
        OUString("Group2"));
    CPPUNIT_ASSERT_MESSAGE("Dimension not found", pPageDim);
    OUString aPage("A");
    pPageDim->SetCurrentPage(&aPage);
    pDPObj->SetSaveData(aSaveData);
    aOutRange = refresh(pDPObj);
    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][2] = {
            { "Filter", 0 },
            { "Group2", "A" },
            { 0, 0 },
            { "Data", 0 },
            { "Sum - Val1", "4" },
            { "Sum - Val2", "40" }
        };

        bSuccess = checkDPTableOutput<2>(m_pDoc, aOutRange, aOutputCheck, "DataPilot table output (filtered by page)");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    fTest = m_pDoc->GetValue(aFormulaAddr);
    CPPUNIT_ASSERT_MESSAGE("Incorrect formula value that references a cell in the pivot table output.", fTest == 40.0);

    // Set query filter.
    ScSheetSourceDesc aDesc(*pDPObj->GetSheetDesc());
    ScQueryParam aQueryParam(aDesc.GetQueryParam());
    CPPUNIT_ASSERT_MESSAGE("There should be at least one query entry.", aQueryParam.GetEntryCount() > 0);
    ScQueryEntry& rEntry = aQueryParam.GetEntry(0);
    rEntry.bDoQuery = true;
    rEntry.nField = 1;  // Group1
    rEntry.GetQueryItem().mfVal = 1;
    aDesc.SetQueryParam(aQueryParam);
    pDPObj->SetSheetDesc(aDesc);
    aOutRange = refresh(pDPObj);
    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][2] = {
            { "Filter", 0 },
            { "Group2", "A" },
            { 0, 0 },
            { "Data", 0 },
            { "Sum - Val1", "2" },
            { "Sum - Val2", "20" }
        };

        bSuccess = checkDPTableOutput<2>(m_pDoc, aOutRange, aOutputCheck, "DataPilot table output (filtered by query)");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    fTest = m_pDoc->GetValue(aFormulaAddr);
    CPPUNIT_ASSERT_MESSAGE("Incorrect formula value that references a cell in the pivot table output.", fTest == 20.0);

    // Set the current page of 'Group2' back to '- all -'. The query filter
    // should still be in effect.
    pPageDim->SetCurrentPage(NULL); // Remove the page.
    pDPObj->SetSaveData(aSaveData);
    aOutRange = refresh(pDPObj);
    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][2] = {
            { "Filter", 0 },
            { "Group2", "- all -" },
            { 0, 0 },
            { "Data", 0 },
            { "Sum - Val1", "4" },
            { "Sum - Val2", "40" }
        };

        bSuccess = checkDPTableOutput<2>(m_pDoc, aOutRange, aOutputCheck, "DataPilot table output (filtered by page)");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }


    pDPs->FreeTable(pDPObj);
    CPPUNIT_ASSERT_MESSAGE("There shouldn't be any data pilot table stored with the document.",
                           pDPs->GetCount() == 0);

    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

void Test::testPivotTableNamedSource()
{
    m_pDoc->InsertTab(0, OUString("Data"));
    m_pDoc->InsertTab(1, OUString("Table"));

    // Dimension definition
    DPFieldDef aFields[] = {
        { "Name",  sheet::DataPilotFieldOrientation_ROW, 0 },
        { "Group", sheet::DataPilotFieldOrientation_COLUMN, 0 },
        { "Score", sheet::DataPilotFieldOrientation_DATA, 0 }
    };

    // Raw data
    const char* aData[][3] = {
        { "Andy",    "A", "30" },
        { "Bruce",   "A", "20" },
        { "Charlie", "B", "45" },
        { "David",   "B", "12" },
        { "Edward",  "C",  "8" },
        { "Frank",   "C", "15" },
    };

    size_t nFieldCount = SAL_N_ELEMENTS(aFields);
    size_t nDataCount = SAL_N_ELEMENTS(aData);

    // Insert the raw data.
    ScRange aSrcRange = insertDPSourceData(m_pDoc, aFields, nFieldCount, aData, nDataCount);
    OUString aRangeStr;
    aSrcRange.Format(aRangeStr, SCR_ABS_3D, m_pDoc);

    // Name this range.
    OUString aRangeName("MyData");
    ScRangeName* pNames = m_pDoc->GetRangeName();
    CPPUNIT_ASSERT_MESSAGE("Failed to get global range name container.", pNames);
    ScRangeData* pName = new ScRangeData(
        m_pDoc, aRangeName, aRangeStr);
    bool bSuccess = pNames->insert(pName);
    CPPUNIT_ASSERT_MESSAGE("Failed to insert a new name.", bSuccess);

    ScSheetSourceDesc aSheetDesc(m_pDoc);
    aSheetDesc.SetRangeName(aRangeName);
    ScDPObject* pDPObj = createDPFromSourceDesc(m_pDoc, aSheetDesc, aFields, nFieldCount, false);
    CPPUNIT_ASSERT_MESSAGE("Failed to create a new pivot table object.", pDPObj);

    ScDPCollection* pDPs = m_pDoc->GetDPCollection();
    bSuccess = pDPs->InsertNewTable(pDPObj);
    CPPUNIT_ASSERT_MESSAGE("failed to insert a new pivot table object into document.", bSuccess);
    CPPUNIT_ASSERT_MESSAGE("there should be only one data pilot table.",
                           pDPs->GetCount() == 1);
    pDPObj->SetName(pDPs->CreateNewName());

    ScRange aOutRange = refresh(pDPObj);
    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][5] = {
            { "Sum - Score", "Group", 0, 0, 0 },
            { "Name", "A", "B", "C", "Total Result" },
            { "Andy", "30", 0, 0, "30" },
            { "Bruce", "20", 0, 0, "20" },
            { "Charlie", 0, "45", 0, "45" },
            { "David", 0, "12", 0, "12" },
            { "Edward", 0, 0, "8", "8" },
            { "Frank", 0, 0, "15", "15" },
            { "Total Result", "50", "57", "23", "130" }
        };

        bSuccess = checkDPTableOutput<5>(m_pDoc, aOutRange, aOutputCheck, "DataPilot table output");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    CPPUNIT_ASSERT_MESSAGE("There should be one named range data cache.",
                           pDPs->GetNameCaches().size() == 1 && pDPs->GetSheetCaches().size() == 0);

    // Move the table with pivot table to the left of the source data sheet.
    m_pDoc->MoveTab(1, 0);
    OUString aTabName;
    m_pDoc->GetName(0, aTabName);
    CPPUNIT_ASSERT_MESSAGE( "Wrong sheet name.", aTabName == "Table" );
    CPPUNIT_ASSERT_MESSAGE("Pivot table output is on the wrong sheet!",
                           pDPObj->GetOutRange().aStart.Tab() == 0);

    CPPUNIT_ASSERT_MESSAGE("Moving the pivot table to another sheet shouldn't have changed the cache state.",
                           pDPs->GetNameCaches().size() == 1 && pDPs->GetSheetCaches().size() == 0);

    const ScSheetSourceDesc* pDesc = pDPObj->GetSheetDesc();
    CPPUNIT_ASSERT_MESSAGE("Sheet source description doesn't exist.", pDesc);
    CPPUNIT_ASSERT_MESSAGE("Named source range has been altered unexpectedly!",
                           pDesc->GetRangeName().equals(aRangeName));

    CPPUNIT_ASSERT_MESSAGE("Cache should exist.", pDPs->GetNameCaches().hasCache(aRangeName));

    pDPs->FreeTable(pDPObj);
    CPPUNIT_ASSERT_MESSAGE("There should be no more tables.", pDPs->GetCount() == 0);
    CPPUNIT_ASSERT_MESSAGE("There shouldn't be any more cache stored.",
                           pDPs->GetNameCaches().size() == 0);

    pNames->clear();
    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

void Test::testPivotTableCache()
{
    m_pDoc->InsertTab(0, OUString("Data"));

    // Raw data
    const char* aData[][3] = {
        { "F1", "F2", "F3" },
        { "Z",  "A", "30" },
        { "R",  "A", "20" },
        { "A",  "B", "45" },
        { "F",  "B", "12" },
        { "Y",  "C",  "8" },
        { "12", "C", "15" },
    };

    ScAddress aPos(1,1,0);
    ScRange aDataRange = insertRangeData(m_pDoc, aPos, aData, SAL_N_ELEMENTS(aData));
    CPPUNIT_ASSERT_MESSAGE("failed to insert range data at correct position", aDataRange.aStart == aPos);

    ScDPCache aCache(m_pDoc);
    aCache.InitFromDoc(m_pDoc, aDataRange);
    long nDimCount = aCache.GetColumnCount();
    CPPUNIT_ASSERT_MESSAGE("wrong dimension count.", nDimCount == 3);
    OUString aDimName = aCache.GetDimensionName(0);
    CPPUNIT_ASSERT_MESSAGE("wrong dimension name", aDimName.equalsAscii("F1"));
    aDimName = aCache.GetDimensionName(1);
    CPPUNIT_ASSERT_MESSAGE("wrong dimension name", aDimName.equalsAscii("F2"));
    aDimName = aCache.GetDimensionName(2);
    CPPUNIT_ASSERT_MESSAGE("wrong dimension name", aDimName.equalsAscii("F3"));

    // In each dimension, member ID values also represent their sort order (in
    // source dimensions only, not in group dimensions). Value items are
    // sorted before string ones. Also, no duplicate dimension members should
    // exist.

    // Dimension 0 - a mix of strings and values.
    long nMemCount = aCache.GetDimMemberCount(0);
    CPPUNIT_ASSERT_MESSAGE("wrong dimension member count", nMemCount == 6);
    const ScDPItemData* pItem = aCache.GetItemDataById(0, 0);
    CPPUNIT_ASSERT_MESSAGE("wrong item value", pItem &&
                           pItem->GetType() == ScDPItemData::Value &&
                           pItem->GetValue() == 12);
    pItem = aCache.GetItemDataById(0, 1);
    CPPUNIT_ASSERT_MESSAGE("wrong item value", pItem &&
                           pItem->GetType() == ScDPItemData::String &&
                           pItem->GetString().equalsAscii("A"));
    pItem = aCache.GetItemDataById(0, 2);
    CPPUNIT_ASSERT_MESSAGE("wrong item value", pItem &&
                           pItem->GetType() == ScDPItemData::String &&
                           pItem->GetString().equalsAscii("F"));
    pItem = aCache.GetItemDataById(0, 3);
    CPPUNIT_ASSERT_MESSAGE("wrong item value", pItem &&
                           pItem->GetType() == ScDPItemData::String &&
                           pItem->GetString().equalsAscii("R"));
    pItem = aCache.GetItemDataById(0, 4);
    CPPUNIT_ASSERT_MESSAGE("wrong item value", pItem &&
                           pItem->GetType() == ScDPItemData::String &&
                           pItem->GetString().equalsAscii("Y"));
    pItem = aCache.GetItemDataById(0, 5);
    CPPUNIT_ASSERT_MESSAGE("wrong item value", pItem &&
                           pItem->GetType() == ScDPItemData::String &&
                           pItem->GetString().equalsAscii("Z"));
    pItem = aCache.GetItemDataById(0, 6);
    CPPUNIT_ASSERT_MESSAGE("wrong item value", !pItem);

    // Dimension 1 - duplicate values in source.
    nMemCount = aCache.GetDimMemberCount(1);
    CPPUNIT_ASSERT_MESSAGE("wrong dimension member count", nMemCount == 3);
    pItem = aCache.GetItemDataById(1, 0);
    CPPUNIT_ASSERT_MESSAGE("wrong item value", pItem &&
                           pItem->GetType() == ScDPItemData::String &&
                           pItem->GetString().equalsAscii("A"));
    pItem = aCache.GetItemDataById(1, 1);
    CPPUNIT_ASSERT_MESSAGE("wrong item value", pItem &&
                           pItem->GetType() == ScDPItemData::String &&
                           pItem->GetString().equalsAscii("B"));
    pItem = aCache.GetItemDataById(1, 2);
    CPPUNIT_ASSERT_MESSAGE("wrong item value", pItem &&
                           pItem->GetType() == ScDPItemData::String &&
                           pItem->GetString().equalsAscii("C"));
    pItem = aCache.GetItemDataById(1, 3);
    CPPUNIT_ASSERT_MESSAGE("wrong item value", !pItem);

    // Dimension 2 - values only.
    nMemCount = aCache.GetDimMemberCount(2);
    CPPUNIT_ASSERT_MESSAGE("wrong dimension member count", nMemCount == 6);
    pItem = aCache.GetItemDataById(2, 0);
    CPPUNIT_ASSERT_MESSAGE("wrong item value", pItem &&
                           pItem->GetType() == ScDPItemData::Value &&
                           pItem->GetValue() == 8);
    pItem = aCache.GetItemDataById(2, 1);
    CPPUNIT_ASSERT_MESSAGE("wrong item value", pItem &&
                           pItem->GetType() == ScDPItemData::Value &&
                           pItem->GetValue() == 12);
    pItem = aCache.GetItemDataById(2, 2);
    CPPUNIT_ASSERT_MESSAGE("wrong item value", pItem &&
                           pItem->GetType() == ScDPItemData::Value &&
                           pItem->GetValue() == 15);
    pItem = aCache.GetItemDataById(2, 3);
    CPPUNIT_ASSERT_MESSAGE("wrong item value", pItem &&
                           pItem->GetType() == ScDPItemData::Value &&
                           pItem->GetValue() == 20);
    pItem = aCache.GetItemDataById(2, 4);
    CPPUNIT_ASSERT_MESSAGE("wrong item value", pItem &&
                           pItem->GetType() == ScDPItemData::Value &&
                           pItem->GetValue() == 30);
    pItem = aCache.GetItemDataById(2, 5);
    CPPUNIT_ASSERT_MESSAGE("wrong item value", pItem &&
                           pItem->GetType() == ScDPItemData::Value &&
                           pItem->GetValue() == 45);
    pItem = aCache.GetItemDataById(2, 6);
    CPPUNIT_ASSERT_MESSAGE("wrong item value", !pItem);

    {
        // Check the integrity of the source data.
        ScDPItemData aTest;
        long nDim;

        {
            // Dimension 0: Z, R, A, F, Y, 12
            nDim = 0;
            const char* aChecks[] = { "Z", "R", "A", "F", "Y" };
            for (size_t i = 0; i < SAL_N_ELEMENTS(aChecks); ++i)
            {
                pItem = aCache.GetItemDataById(nDim, aCache.GetItemDataId(nDim, i, false));
                aTest.SetString(OUString::createFromAscii(aChecks[i]));
                CPPUNIT_ASSERT_MESSAGE("wrong data value", pItem && *pItem == aTest);
            }

            pItem = aCache.GetItemDataById(nDim, aCache.GetItemDataId(nDim, 5, false));
            aTest.SetValue(12);
            CPPUNIT_ASSERT_MESSAGE("wrong data value", pItem && *pItem == aTest);
        }

        {
            // Dimension 1: A, A, B, B, C, C
            nDim = 1;
            const char* aChecks[] = { "A", "A", "B", "B", "C", "C" };
            for (size_t i = 0; i < SAL_N_ELEMENTS(aChecks); ++i)
            {
                pItem = aCache.GetItemDataById(nDim, aCache.GetItemDataId(nDim, i, false));
                aTest.SetString(OUString::createFromAscii(aChecks[i]));
                CPPUNIT_ASSERT_MESSAGE("wrong data value", pItem && *pItem == aTest);
            }
        }

        {
            // Dimension 2: 30, 20, 45, 12, 8, 15
            nDim = 2;
            double aChecks[] = { 30, 20, 45, 12, 8, 15 };
            for (size_t i = 0; i < SAL_N_ELEMENTS(aChecks); ++i)
            {
                pItem = aCache.GetItemDataById(nDim, aCache.GetItemDataId(nDim, i, false));
                aTest.SetValue(aChecks[i]);
                CPPUNIT_ASSERT_MESSAGE("wrong data value", pItem && *pItem == aTest);
            }
        }
    }

    // Now, on to testing the filtered cache.

    {
        // Non-filtered cache - everything should be visible.
        ScDPFilteredCache aFilteredCache(aCache);
        aFilteredCache.fillTable();

        sal_Int32 nRows = aFilteredCache.getRowSize();
        CPPUNIT_ASSERT_MESSAGE("Wrong dimension.", nRows == 6 && aFilteredCache.getColSize() == 3);

        for (sal_Int32 i = 0; i < nRows; ++i)
        {
            if (!aFilteredCache.isRowActive(i))
            {
                std::ostringstream os;
                os << "Row " << i << " should be visible but it isn't.";
                CPPUNIT_ASSERT_MESSAGE(os.str().c_str(), false);
            }
        }
    }

    // TODO : Add test for filtered caches.

    m_pDoc->DeleteTab(0);
}

void Test::testPivotTableDuplicateDataFields()
{
    m_pDoc->InsertTab(0, OUString("Data"));
    m_pDoc->InsertTab(1, OUString("Table"));

    // Raw data
    const char* aData[][2] = {
        { "Name", "Value" },
        { "A",       "45" },
        { "A",        "5" },
        { "A",       "41" },
        { "A",       "49" },
        { "A",        "4" },
        { "B",       "33" },
        { "B",       "84" },
        { "B",       "74" },
        { "B",        "8" },
        { "B",       "68" }
    };

    // Dimension definition
    DPFieldDef aFields[] = {
        { "Name",  sheet::DataPilotFieldOrientation_ROW, 0 },
        { "Value", sheet::DataPilotFieldOrientation_DATA, sheet::GeneralFunction_SUM },
        { "Value", sheet::DataPilotFieldOrientation_DATA, sheet::GeneralFunction_COUNT }
    };

    ScAddress aPos(2,2,0);
    ScRange aDataRange = insertRangeData(m_pDoc, aPos, aData, SAL_N_ELEMENTS(aData));
    CPPUNIT_ASSERT_MESSAGE("failed to insert range data at correct position", aDataRange.aStart == aPos);

    ScDPObject* pDPObj = createDPFromRange(
        m_pDoc, aDataRange, aFields, SAL_N_ELEMENTS(aFields), false);

    ScDPCollection* pDPs = m_pDoc->GetDPCollection();
    bool bSuccess = pDPs->InsertNewTable(pDPObj);

    CPPUNIT_ASSERT_MESSAGE("failed to insert a new pivot table object into document.", bSuccess);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("there should be only one data pilot table.",
                           pDPs->GetCount(), static_cast<size_t>(1));
    pDPObj->SetName(pDPs->CreateNewName());

    ScRange aOutRange = refresh(pDPObj);
    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][3] = {
            { "Name", "Data", 0 },
            { "A", "Sum - Value", "144" },
            { 0, "Count - Value", "5" },
            { "B", "Sum - Value", "267" },
            { 0, "Count - Value", "5" },
            { "Total Sum - Value", 0, "411" },
            { "Total Count - Value", 0, "10" },
        };

        bSuccess = checkDPTableOutput<3>(m_pDoc, aOutRange, aOutputCheck, "DataPilot table output");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    // Move the data layout dimension from row to column.
    ScDPSaveData* pSaveData = pDPObj->GetSaveData();
    CPPUNIT_ASSERT_MESSAGE("No save data!?", pSaveData);
    ScDPSaveDimension* pDataLayout = pSaveData->GetDataLayoutDimension();
    CPPUNIT_ASSERT_MESSAGE("No data layout dimension.", pDataLayout);
    pDataLayout->SetOrientation(sheet::DataPilotFieldOrientation_COLUMN);
    pDPObj->SetSaveData(*pSaveData);

    // Refresh the table output.
    aOutRange = refresh(pDPObj);
    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][3] = {
            { 0, "Data", 0 },
            { "Name", "Sum - Value", "Count - Value" },
            { "A", "144", "5" },
            { "B", "267", "5" },
            { "Total Result", "411", "10" }
        };

        bSuccess = checkDPTableOutput<3>(m_pDoc, aOutRange, aOutputCheck, "DataPilot table output");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    ScPivotParam aParam;
    pDPObj->FillLabelData(aParam);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be exactly 4 labels (2 original, 1 data layout, and 1 duplicate dimensions).",
                           aParam.maLabelArray.size(), static_cast<size_t>(4));

    pDPs->FreeTable(pDPObj);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be no more tables.", pDPs->GetCount(), static_cast<size_t>(0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There shouldn't be any more cache stored.",
                           pDPs->GetSheetCaches().size(), static_cast<size_t>(0));

    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

void Test::testPivotTableNormalGrouping()
{
    m_pDoc->InsertTab(0, OUString("Data"));
    m_pDoc->InsertTab(1, OUString("Table"));

    // Raw data
    const char* aData[][2] = {
        { "Name", "Value" },
        { "A", "1" },
        { "B", "2" },
        { "C", "3" },
        { "D", "4" },
        { "E", "5" },
        { "F", "6" },
        { "G", "7" }
    };

    // Dimension definition
    DPFieldDef aFields[] = {
        { "Name",  sheet::DataPilotFieldOrientation_ROW, 0 },
        { "Value", sheet::DataPilotFieldOrientation_DATA, sheet::GeneralFunction_SUM },
    };

    ScAddress aPos(1,1,0);
    ScRange aDataRange = insertRangeData(m_pDoc, aPos, aData, SAL_N_ELEMENTS(aData));
    CPPUNIT_ASSERT_MESSAGE("failed to insert range data at correct position", aDataRange.aStart == aPos);

    ScDPObject* pDPObj = createDPFromRange(
        m_pDoc, aDataRange, aFields, SAL_N_ELEMENTS(aFields), false);

    ScDPCollection* pDPs = m_pDoc->GetDPCollection();
    bool bSuccess = pDPs->InsertNewTable(pDPObj);

    CPPUNIT_ASSERT_MESSAGE("failed to insert a new pivot table object into document.", bSuccess);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("there should be only one data pilot table.",
                           pDPs->GetCount(), static_cast<size_t>(1));
    pDPObj->SetName(pDPs->CreateNewName());

    ScRange aOutRange = refresh(pDPObj);
    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][2] = {
            { "Name", 0 },
            { "A", "1" },
            { "B", "2" },
            { "C", "3" },
            { "D", "4" },
            { "E", "5" },
            { "F", "6" },
            { "G", "7" },
            { "Total Result", "28" }
        };

        bSuccess = checkDPTableOutput<2>(m_pDoc, aOutRange, aOutputCheck, "Initial output without grouping");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    ScDPSaveData* pSaveData = pDPObj->GetSaveData();
    CPPUNIT_ASSERT_MESSAGE("No save data !?", pSaveData);
    ScDPDimensionSaveData* pDimData = pSaveData->GetDimensionData();
    CPPUNIT_ASSERT_MESSAGE("Failed to create dimension data.", pDimData);

    OUString aGroupPrefix("Group");
    OUString aBaseDimName("Name");
    OUString aGroupDimName =
        pDimData->CreateGroupDimName(aBaseDimName, *pDPObj, false, NULL);

    {
        // Group A, B and C together.
        ScDPSaveGroupDimension aGroupDim(aBaseDimName, aGroupDimName);
        OUString aGroupName = aGroupDim.CreateGroupName(aGroupPrefix);
        CPPUNIT_ASSERT_MESSAGE("Unexpected group name", aGroupName.equalsAscii("Group1"));

        ScDPSaveGroupItem aGroup(aGroupName);
        aGroup.AddElement(OUString("A"));
        aGroup.AddElement(OUString("B"));
        aGroup.AddElement(OUString("C"));
        aGroupDim.AddGroupItem(aGroup);
        pDimData->AddGroupDimension(aGroupDim);

        ScDPSaveDimension* pDim = pSaveData->GetDimensionByName(aGroupDimName);
        pDim->SetOrientation(sheet::DataPilotFieldOrientation_ROW);
        pSaveData->SetPosition(pDim, 0); // Set it before the base dimension.
    }

    pDPObj->SetSaveData(*pSaveData);
    aOutRange = refreshGroups(pDPs, pDPObj);
    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][3] = {
            { "Name2", "Name", 0 },
            { "D", "D", "4" },
            { "E", "E", "5" },
            { "F", "F", "6" },
            { "G", "G", "7" },
            { "Group1", "A", "1" },
            { 0,        "B", "2" },
            { 0,        "C", "3" },
            { "Total Result", 0, "28" }
        };

        bSuccess = checkDPTableOutput<3>(m_pDoc, aOutRange, aOutputCheck, "A, B, C grouped by Group1.");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    pSaveData = pDPObj->GetSaveData();
    pDimData = pSaveData->GetDimensionData();

    {
        // Group D, E, F together.
        ScDPSaveGroupDimension* pGroupDim = pDimData->GetGroupDimAccForBase(aBaseDimName);
        CPPUNIT_ASSERT_MESSAGE("There should be an existing group dimension.", pGroupDim);
        OUString aGroupName = pGroupDim->CreateGroupName(aGroupPrefix);
        CPPUNIT_ASSERT_MESSAGE("Unexpected group name", aGroupName.equalsAscii("Group2"));

        ScDPSaveGroupItem aGroup(aGroupName);
        aGroup.AddElement(OUString("D"));
        aGroup.AddElement(OUString("E"));
        aGroup.AddElement(OUString("F"));
        pGroupDim->AddGroupItem(aGroup);
    }

    pDPObj->SetSaveData(*pSaveData);
    aOutRange = refreshGroups(pDPs, pDPObj);
    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][3] = {
            { "Name2", "Name", 0 },
            { "G", "G", "7" },
            { "Group1", "A", "1" },
            { 0,        "B", "2" },
            { 0,        "C", "3" },
            { "Group2", "D", "4" },
            { 0,        "E", "5" },
            { 0,        "F", "6" },
            { "Total Result", 0, "28" }
        };

        bSuccess = checkDPTableOutput<3>(m_pDoc, aOutRange, aOutputCheck, "D, E, F grouped by Group2.");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    pDPs->FreeTable(pDPObj);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be no more tables.", pDPs->GetCount(), static_cast<size_t>(0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There shouldn't be any more cache stored.",
                           pDPs->GetSheetCaches().size(), static_cast<size_t>(0));

    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

void Test::testPivotTableNumberGrouping()
{
    m_pDoc->InsertTab(0, OUString("Data"));
    m_pDoc->InsertTab(1, OUString("Table"));

    // Raw data
    const char* aData[][2] = {
        { "Order", "Score" },
        { "43", "171" },
        { "18", "20"  },
        { "69", "159" },
        { "95", "19"  },
        { "96", "163" },
        { "46", "70"  },
        { "22", "36"  },
        { "81", "49"  },
        { "54", "61"  },
        { "39", "62"  },
        { "86", "17"  },
        { "34", "0"   },
        { "30", "25"  },
        { "24", "103" },
        { "16", "59"  },
        { "24", "119" },
        { "15", "86"  },
        { "69", "170" }
    };

    // Dimension definition
    DPFieldDef aFields[] = {
        { "Order", sheet::DataPilotFieldOrientation_ROW, 0 },
        { "Score", sheet::DataPilotFieldOrientation_DATA, sheet::GeneralFunction_SUM },
    };

    ScAddress aPos(1,1,0);
    ScRange aDataRange = insertRangeData(m_pDoc, aPos, aData, SAL_N_ELEMENTS(aData));
    CPPUNIT_ASSERT_MESSAGE("failed to insert range data at correct position", aDataRange.aStart == aPos);

    ScDPObject* pDPObj = createDPFromRange(
        m_pDoc, aDataRange, aFields, SAL_N_ELEMENTS(aFields), false);

    ScDPCollection* pDPs = m_pDoc->GetDPCollection();
    bool bSuccess = pDPs->InsertNewTable(pDPObj);

    CPPUNIT_ASSERT_MESSAGE("failed to insert a new pivot table object into document.", bSuccess);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("there should be only one data pilot table.",
                           pDPs->GetCount(), static_cast<size_t>(1));
    pDPObj->SetName(pDPs->CreateNewName());

    ScDPSaveData* pSaveData = pDPObj->GetSaveData();
    CPPUNIT_ASSERT_MESSAGE("No save data !?", pSaveData);
    ScDPDimensionSaveData* pDimData = pSaveData->GetDimensionData();
    CPPUNIT_ASSERT_MESSAGE("No dimension data !?", pDimData);

    {
        ScDPNumGroupInfo aInfo;
        aInfo.mbEnable = true;
        aInfo.mbAutoStart = false;
        aInfo.mbAutoEnd = false;
        aInfo.mbDateValues = false;
        aInfo.mbIntegerOnly = true;
        aInfo.mfStart = 30;
        aInfo.mfEnd = 60;
        aInfo.mfStep = 10;
        ScDPSaveNumGroupDimension aGroup(OUString("Order"), aInfo);
        pDimData->AddNumGroupDimension(aGroup);
    }

    pDPObj->SetSaveData(*pSaveData);
    ScRange aOutRange = refreshGroups(pDPs, pDPObj);
    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][2] = {
            { "Order", 0 },
            { "<30",   "423" },
            { "30-39", "87"  },
            { "40-49", "241" },
            { "50-60", "61"  },
            { ">60",   "577" },
            { "Total Result", "1389" }
        };

        bSuccess = checkDPTableOutput<2>(m_pDoc, aOutRange, aOutputCheck, "Order grouped by numbers");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    pDPs->FreeTable(pDPObj);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be no more tables.", pDPs->GetCount(), static_cast<size_t>(0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There shouldn't be any more cache stored.",
                           pDPs->GetSheetCaches().size(), static_cast<size_t>(0));

    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

void Test::testPivotTableDateGrouping()
{
    m_pDoc->InsertTab(0, OUString("Data"));
    m_pDoc->InsertTab(1, OUString("Table"));

    // Raw data
    const char* aData[][2] = {
        { "Date", "Value" },
        { "2011-01-01", "1" },
        { "2011-03-02", "2" },
        { "2012-01-04", "3" },
        { "2012-02-23", "4" },
        { "2012-02-24", "5" },
        { "2012-03-15", "6" },
        { "2011-09-03", "7" },
        { "2012-12-25", "8" }
    };

    // Dimension definition
    DPFieldDef aFields[] = {
        { "Date", sheet::DataPilotFieldOrientation_ROW, 0 },
        { "Value", sheet::DataPilotFieldOrientation_DATA, sheet::GeneralFunction_SUM },
    };

    ScAddress aPos(1,1,0);
    ScRange aDataRange = insertRangeData(m_pDoc, aPos, aData, SAL_N_ELEMENTS(aData));
    CPPUNIT_ASSERT_MESSAGE("failed to insert range data at correct position", aDataRange.aStart == aPos);

    ScDPObject* pDPObj = createDPFromRange(
        m_pDoc, aDataRange, aFields, SAL_N_ELEMENTS(aFields), false);

    ScDPCollection* pDPs = m_pDoc->GetDPCollection();
    bool bSuccess = pDPs->InsertNewTable(pDPObj);

    CPPUNIT_ASSERT_MESSAGE("failed to insert a new pivot table object into document.", bSuccess);
    CPPUNIT_ASSERT_MESSAGE("there should be only one data pilot table.",
                           pDPs->GetCount() == 1);
    pDPObj->SetName(pDPs->CreateNewName());

    ScDPSaveData* pSaveData = pDPObj->GetSaveData();
    CPPUNIT_ASSERT_MESSAGE("No save data !?", pSaveData);
    ScDPDimensionSaveData* pDimData = pSaveData->GetDimensionData();
    CPPUNIT_ASSERT_MESSAGE("No dimension data !?", pDimData);

    OUString aBaseDimName("Date");

    ScDPNumGroupInfo aInfo;
    aInfo.mbEnable = true;
    aInfo.mbAutoStart = true;
    aInfo.mbAutoEnd = true;
    {
        // Turn the Date dimension into months.  The first of the date
        // dimensions is always a number-group dimension which replaces the
        // original dimension.
        ScDPSaveNumGroupDimension aGroup(aBaseDimName, aInfo, sheet::DataPilotFieldGroupBy::MONTHS);
        pDimData->AddNumGroupDimension(aGroup);
    }

    {
        // Add quarter dimension.  This will be an additional dimension.
        OUString aGroupDimName =
            pDimData->CreateDateGroupDimName(
                sheet::DataPilotFieldGroupBy::QUARTERS, *pDPObj, true, NULL);
        ScDPSaveGroupDimension aGroupDim(aBaseDimName, aGroupDimName);
        aGroupDim.SetDateInfo(aInfo, sheet::DataPilotFieldGroupBy::QUARTERS);
        pDimData->AddGroupDimension(aGroupDim);

        // Set orientation.
        ScDPSaveDimension* pDim = pSaveData->GetDimensionByName(aGroupDimName);
        pDim->SetOrientation(sheet::DataPilotFieldOrientation_ROW);
        pSaveData->SetPosition(pDim, 0); // set it to the left end.
    }

    {
        // Add year dimension.  This is a new dimension also.
        OUString aGroupDimName =
            pDimData->CreateDateGroupDimName(
                sheet::DataPilotFieldGroupBy::YEARS, *pDPObj, true, NULL);
        ScDPSaveGroupDimension aGroupDim(aBaseDimName, aGroupDimName);
        aGroupDim.SetDateInfo(aInfo, sheet::DataPilotFieldGroupBy::YEARS);
        pDimData->AddGroupDimension(aGroupDim);

        // Set orientation.
        ScDPSaveDimension* pDim = pSaveData->GetDimensionByName(aGroupDimName);
        pDim->SetOrientation(sheet::DataPilotFieldOrientation_ROW);
        pSaveData->SetPosition(pDim, 0); // set it to the left end.
    }

    pDPObj->SetSaveData(*pSaveData);
    ScRange aOutRange = refreshGroups(pDPs, pDPObj);
    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][4] = {
            { "Years", "Quarters", "Date", 0 },
            { "2011", "Q1", "Jan", "1" },
            { 0, 0,         "Mar", "2" },
            { 0,      "Q3", "Sep", "7" },
            { "2012", "Q1", "Jan", "3" },
            { 0, 0,         "Feb", "9" },
            { 0, 0,         "Mar", "6" },
            { 0,      "Q4", "Dec", "8" },
            { "Total Result", 0, 0, "36" },
        };

        bSuccess = checkDPTableOutput<4>(m_pDoc, aOutRange, aOutputCheck, "Years, quarters and months date groups.");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    {
        // Let's hide year 2012.
        pSaveData = pDPObj->GetSaveData();
        ScDPSaveDimension* pDim = pSaveData->GetDimensionByName(OUString("Years"));
        CPPUNIT_ASSERT_MESSAGE("Years dimension should exist.", pDim);
        ScDPSaveMember* pMem = pDim->GetMemberByName(OUString("2012"));
        CPPUNIT_ASSERT_MESSAGE("Member should exist.", pMem);
        pMem->SetIsVisible(false);
    }
    pDPObj->SetSaveData(*pSaveData);
    pDPObj->ReloadGroupTableData();
    pDPObj->InvalidateData();

    aOutRange = refresh(pDPObj);
    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][4] = {
            { "Years", "Quarters", "Date", 0 },
            { "2011", "Q1", "Jan", "1" },
            { 0, 0,         "Mar", "2" },
            { 0,      "Q3", "Sep", "7" },
            { "Total Result", 0, 0, "10" },
        };

        bSuccess = checkDPTableOutput<4>(m_pDoc, aOutRange, aOutputCheck, "Year 2012 data now hidden");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    // Remove all date grouping. The source dimension "Date" has two
    // external dimensions ("Years" and "Quarters") and one internal ("Date"
    // the same name but different hierarchy).  Remove all of them.
    pSaveData = pDPObj->GetSaveData();
    pSaveData->RemoveAllGroupDimensions(aBaseDimName);
    pDPObj->SetSaveData(*pSaveData);
    pDPObj->ReloadGroupTableData();
    pDPObj->InvalidateData();

    aOutRange = refresh(pDPObj);
    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][2] = {
            { "Date", 0 },
            { "2011-01-01", "1" },
            { "2011-03-02", "2" },
            { "2011-09-03", "7" },
            { "2012-01-04", "3" },
            { "2012-02-23", "4" },
            { "2012-02-24", "5" },
            { "2012-03-15", "6" },
            { "2012-12-25", "8" },
            { "Total Result", "36" }
        };

        bSuccess = checkDPTableOutput<2>(m_pDoc, aOutRange, aOutputCheck, "Remove all date grouping.");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    pDPs->FreeTable(pDPObj);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be no more tables.", pDPs->GetCount(), static_cast<size_t>(0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There shouldn't be any more cache stored.",
                           pDPs->GetSheetCaches().size(), static_cast<size_t>(0));

    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

void Test::testPivotTableEmptyRows()
{
    m_pDoc->InsertTab(0, OUString("Data"));
    m_pDoc->InsertTab(1, OUString("Table"));

    // Raw data
    const char* aData[][2] = {
        { "Name", "Value" },
        { "A", "1" },
        { "B", "2" },
        { "C", "3" },
        { "D", "4" },
    };

    // Dimension definition
    DPFieldDef aFields[] = {
        { "Name", sheet::DataPilotFieldOrientation_ROW, 0 },
        { "Value", sheet::DataPilotFieldOrientation_DATA, sheet::GeneralFunction_SUM },
    };

    ScAddress aPos(1,1,0);
    ScRange aDataRange = insertRangeData(m_pDoc, aPos, aData, SAL_N_ELEMENTS(aData));
    CPPUNIT_ASSERT_MESSAGE("failed to insert range data at correct position", aDataRange.aStart == aPos);

    // Extend the range downward to include some trailing empty rows.
    aDataRange.aEnd.IncRow(2);

    ScDPObject* pDPObj = createDPFromRange(
        m_pDoc, aDataRange, aFields, SAL_N_ELEMENTS(aFields), false);

    ScDPCollection* pDPs = m_pDoc->GetDPCollection();
    bool bSuccess = pDPs->InsertNewTable(pDPObj);

    CPPUNIT_ASSERT_MESSAGE("failed to insert a new pivot table object into document.", bSuccess);
    CPPUNIT_ASSERT_MESSAGE("there should be only one data pilot table.",
                           pDPs->GetCount() == 1);
    pDPObj->SetName(pDPs->CreateNewName());

    ScRange aOutRange = refresh(pDPObj);

    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][2] = {
            { "Name", 0 },
            { "A", "1" },
            { "B", "2" },
            { "C", "3" },
            { "D", "4" },
            { "(empty)", 0 },
            { "Total Result", "10" },
        };

        bSuccess = checkDPTableOutput<2>(m_pDoc, aOutRange, aOutputCheck, "Include empty rows");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    // This time, ignore empty rows.
    ScDPSaveData* pSaveData = pDPObj->GetSaveData();
    CPPUNIT_ASSERT_MESSAGE("Save data doesn't exist.", pSaveData);
    pSaveData->SetIgnoreEmptyRows(true);
    pDPObj->ClearTableData();
    aOutRange = refresh(pDPObj);

    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][2] = {
            { "Name", 0 },
            { "A", "1" },
            { "B", "2" },
            { "C", "3" },
            { "D", "4" },
            { "Total Result", "10" },
        };

        bSuccess = checkDPTableOutput<2>(m_pDoc, aOutRange, aOutputCheck, "Ignore empty rows");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    // Modify the source to remove member 'A', then refresh the table.
    m_pDoc->SetString(1, 2, 0, "B");

    std::set<ScDPObject*> aRefs;
    sal_uLong nErr = pDPs->ReloadCache(pDPObj, aRefs);
    CPPUNIT_ASSERT_MESSAGE("Failed to reload cache.", !nErr);
    CPPUNIT_ASSERT_MESSAGE("There should only be one pivot table linked to this cache.",
                           aRefs.size() == 1 && *aRefs.begin() == pDPObj);

    pDPObj->ClearTableData();
    aOutRange = refresh(pDPObj);

    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][2] = {
            { "Name", 0 },
            { "B", "3" },
            { "C", "3" },
            { "D", "4" },
            { "Total Result", "10" },
        };

        bSuccess = checkDPTableOutput<2>(m_pDoc, aOutRange, aOutputCheck, "Ignore empty rows");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    pDPs->FreeTable(pDPObj);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be no more tables.", pDPs->GetCount(), static_cast<size_t>(0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There shouldn't be any more cache stored.",
                           pDPs->GetSheetCaches().size(), static_cast<size_t>(0));

    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

void Test::testPivotTableTextNumber()
{
    m_pDoc->InsertTab(0, OUString("Data"));
    m_pDoc->InsertTab(1, OUString("Table"));

    // Raw data
    const char* aData[][2] = {
        { "Name", "Value" },
        { "0001", "1" },
        { "0002", "2" },
        { "0003", "3" },
        { "0004", "4" },
    };

    // Dimension definition
    DPFieldDef aFields[] = {
        { "Name", sheet::DataPilotFieldOrientation_ROW, 0 },
        { "Value", sheet::DataPilotFieldOrientation_DATA, sheet::GeneralFunction_SUM },
    };

    // Insert raw data such that the first column values are entered as text.
    size_t nRowCount = SAL_N_ELEMENTS(aData);
    for (size_t nRow = 0; nRow < nRowCount; ++nRow)
    {
        ScSetStringParam aParam;
        aParam.mbDetectNumberFormat = false;
        aParam.meSetTextNumFormat = ScSetStringParam::Always;
        m_pDoc->SetString(0, nRow, 0, OUString::createFromAscii(aData[nRow][0]), &aParam);
        aParam.meSetTextNumFormat = ScSetStringParam::Never;
        m_pDoc->SetString(1, nRow, 0, OUString::createFromAscii(aData[nRow][1]), &aParam);

        if (nRow == 0)
            // Don't check the header row.
            continue;

        // Check the data rows.
        CPPUNIT_ASSERT_MESSAGE("This cell is supposed to be text.", m_pDoc->HasStringData(0, nRow, 0));
        CPPUNIT_ASSERT_MESSAGE("This cell is supposed to be numeric.", m_pDoc->HasValueData(1, nRow, 0));
    }

    ScRange aDataRange(0, 0, 0, 1, 4, 0);

    ScDPObject* pDPObj = createDPFromRange(
        m_pDoc, aDataRange, aFields, SAL_N_ELEMENTS(aFields), false);

    ScDPCollection* pDPs = m_pDoc->GetDPCollection();
    bool bSuccess = pDPs->InsertNewTable(pDPObj);

    CPPUNIT_ASSERT_MESSAGE("failed to insert a new pivot table object into document.", bSuccess);
    CPPUNIT_ASSERT_MESSAGE("there should be only one data pilot table.",
                           pDPs->GetCount() == 1);
    pDPObj->SetName(pDPs->CreateNewName());

    ScRange aOutRange = refresh(pDPObj);

    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][2] = {
            { "Name", 0 },
            { "0001", "1" },
            { "0002", "2" },
            { "0003", "3" },
            { "0004", "4" },
            { "Total Result", "10" },
        };

        bSuccess = checkDPTableOutput<2>(m_pDoc, aOutRange, aOutputCheck, "Text number field members");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    pDPs->FreeTable(pDPObj);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be no more tables.", pDPs->GetCount(), static_cast<size_t>(0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There shouldn't be any more cache stored.",
                                 pDPs->GetSheetCaches().size(), static_cast<size_t>(0));

    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

void Test::testPivotTableCaseInsensitiveStrings()
{
    m_pDoc->InsertTab(0, OUString("Data"));
    m_pDoc->InsertTab(1, OUString("Table"));

    // Raw data
    const char* aData[][2] = {
        { "Name", "Value" },
        { "A", "1" },
        { "a", "2" },
    };

    // Dimension definition
    DPFieldDef aFields[] = {
        { "Name", sheet::DataPilotFieldOrientation_ROW, 0 },
        { "Value", sheet::DataPilotFieldOrientation_DATA, sheet::GeneralFunction_SUM },
    };

    ScAddress aPos(1,1,0);
    ScRange aDataRange = insertRangeData(m_pDoc, aPos, aData, SAL_N_ELEMENTS(aData));
    CPPUNIT_ASSERT_MESSAGE("failed to insert range data at correct position", aDataRange.aStart == aPos);

    ScDPObject* pDPObj = createDPFromRange(
        m_pDoc, aDataRange, aFields, SAL_N_ELEMENTS(aFields), false);

    ScDPCollection* pDPs = m_pDoc->GetDPCollection();
    bool bSuccess = pDPs->InsertNewTable(pDPObj);

    CPPUNIT_ASSERT_MESSAGE("failed to insert a new pivot table object into document.", bSuccess);
    CPPUNIT_ASSERT_MESSAGE("there should be only one data pilot table.",
                           pDPs->GetCount() == 1);
    pDPObj->SetName(pDPs->CreateNewName());

    ScRange aOutRange = refresh(pDPObj);

    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][2] = {
            { "Name", 0 },
            { "A", "3" },
            { "Total Result", "3" },
        };

        bSuccess = checkDPTableOutput<2>(m_pDoc, aOutRange, aOutputCheck, "Case insensitive strings");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    pDPs->FreeTable(pDPObj);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be no more tables.", pDPs->GetCount(), static_cast<size_t>(0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There shouldn't be any more cache stored.",
                           pDPs->GetSheetCaches().size(), static_cast<size_t>(0));

    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

void Test::testPivotTableNumStability()
{
    FormulaGrammarSwitch aFGSwitch(m_pDoc, formula::FormulaGrammar::GRAM_ENGLISH_XL_R1C1);

    // Raw Data
    const char* aData[][4] = {
        { "Name",   "Time Start", "Time End", "Total"          },
        { "Sam",    "07:48 AM",   "09:00 AM", "=RC[-1]-RC[-2]" },
        { "Sam",    "09:00 AM",   "10:30 AM", "=RC[-1]-RC[-2]" },
        { "Sam",    "10:30 AM",   "12:30 PM", "=RC[-1]-RC[-2]" },
        { "Sam",    "12:30 PM",   "01:00 PM", "=RC[-1]-RC[-2]" },
        { "Sam",    "01:00 PM",   "01:30 PM", "=RC[-1]-RC[-2]" },
        { "Sam",    "01:30 PM",   "02:00 PM", "=RC[-1]-RC[-2]" },
        { "Sam",    "02:00 PM",   "07:15 PM", "=RC[-1]-RC[-2]" },
        { "Sam",    "07:47 AM",   "09:00 AM", "=RC[-1]-RC[-2]" },
        { "Sam",    "09:00 AM",   "10:00 AM", "=RC[-1]-RC[-2]" },
        { "Sam",    "10:00 AM",   "11:00 AM", "=RC[-1]-RC[-2]" },
        { "Sam",    "11:00 AM",   "11:30 AM", "=RC[-1]-RC[-2]" },
        { "Sam",    "11:30 AM",   "12:45 PM", "=RC[-1]-RC[-2]" },
        { "Sam",    "12:45 PM",   "01:15 PM", "=RC[-1]-RC[-2]" },
        { "Sam",    "01:15 PM",   "02:30 PM", "=RC[-1]-RC[-2]" },
        { "Sam",    "02:30 PM",   "02:45 PM", "=RC[-1]-RC[-2]" },
        { "Sam",    "02:45 PM",   "04:30 PM", "=RC[-1]-RC[-2]" },
        { "Sam",    "04:30 PM",   "06:00 PM", "=RC[-1]-RC[-2]" },
        { "Sam",    "06:00 PM",   "07:15 PM", "=RC[-1]-RC[-2]" },
        { "Mike",   "06:15 AM",   "08:30 AM", "=RC[-1]-RC[-2]" },
        { "Mike",   "08:30 AM",   "10:03 AM", "=RC[-1]-RC[-2]" },
        { "Mike",   "10:03 AM",   "12:00 PM", "=RC[-1]-RC[-2]" },
        { "Dennis", "11:00 AM",   "01:00 PM", "=RC[-1]-RC[-2]" },
        { "Dennis", "01:00 PM",   "02:00 PM", "=RC[-1]-RC[-2]" }
    };

    // Dimension definition
    DPFieldDef aFields[] = {
        { "Name",  sheet::DataPilotFieldOrientation_ROW, 0 },
        { "Total", sheet::DataPilotFieldOrientation_DATA, sheet::GeneralFunction_SUM },
    };

    m_pDoc->InsertTab(0, OUString("Data"));
    m_pDoc->InsertTab(1, OUString("Table"));

    size_t nRowCount = SAL_N_ELEMENTS(aData);
    ScAddress aPos(1,1,0);
    ScRange aDataRange = insertRangeData(m_pDoc, aPos, aData, nRowCount);

    // Insert formulas to manually calculate sums for each name.
    m_pDoc->SetString(aDataRange.aStart.Col(), aDataRange.aEnd.Row()+1, aDataRange.aStart.Tab(), "=SUMIF(R[-23]C:R[-1]C;\"Dennis\";R[-23]C[3]:R[-1]C[3])");
    m_pDoc->SetString(aDataRange.aStart.Col(), aDataRange.aEnd.Row()+2, aDataRange.aStart.Tab(), "=SUMIF(R[-24]C:R[-2]C;\"Mike\";R[-24]C[3]:R[-2]C[3])");
    m_pDoc->SetString(aDataRange.aStart.Col(), aDataRange.aEnd.Row()+3, aDataRange.aStart.Tab(), "=SUMIF(R[-25]C:R[-3]C;\"Sam\";R[-25]C[3]:R[-3]C[3])");

    m_pDoc->CalcAll();

    // Get correct sum values.
    double fDennisTotal = m_pDoc->GetValue(aDataRange.aStart.Col(), aDataRange.aEnd.Row()+1, aDataRange.aStart.Tab());
    double fMikeTotal = m_pDoc->GetValue(aDataRange.aStart.Col(), aDataRange.aEnd.Row()+2, aDataRange.aStart.Tab());
    double fSamTotal = m_pDoc->GetValue(aDataRange.aStart.Col(), aDataRange.aEnd.Row()+3, aDataRange.aStart.Tab());

    ScDPObject* pDPObj = createDPFromRange(
        m_pDoc, aDataRange, aFields, SAL_N_ELEMENTS(aFields), false);

    ScDPCollection* pDPs = m_pDoc->GetDPCollection();
    bool bSuccess = pDPs->InsertNewTable(pDPObj);

    CPPUNIT_ASSERT_MESSAGE("failed to insert a new pivot table object into document.", bSuccess);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("there should be only one data pilot table.",
                           pDPs->GetCount(), static_cast<size_t>(1));
    pDPObj->SetName(pDPs->CreateNewName());

    ScRange aOutRange = refresh(pDPObj);

    // Manually check the total value for each name.
    //
    // +--------------+----------------+
    // | Name         |                |
    // +--------------+----------------+
    // | Dennis       | <Dennis total> |
    // +--------------+----------------+
    // | Mike         | <Miks total>   |
    // +--------------+----------------+
    // | Sam          | <Sam total>    |
    // +--------------+----------------+
    // | Total Result | ...            |
    // +--------------+----------------+

    aPos = aOutRange.aStart;
    aPos.IncCol();
    aPos.IncRow();
    double fTest = m_pDoc->GetValue(aPos);
    CPPUNIT_ASSERT_MESSAGE("Incorrect value for Dennis.", rtl::math::approxEqual(fTest, fDennisTotal));
    aPos.IncRow();
    fTest = m_pDoc->GetValue(aPos);
    CPPUNIT_ASSERT_MESSAGE("Incorrect value for Mike.", rtl::math::approxEqual(fTest, fMikeTotal));
    aPos.IncRow();
    fTest = m_pDoc->GetValue(aPos);
    CPPUNIT_ASSERT_MESSAGE("Incorrect value for Sam.", rtl::math::approxEqual(fTest, fSamTotal));

    pDPs->FreeTable(pDPObj);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be no more tables.", pDPs->GetCount(), static_cast<size_t>(0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There shouldn't be any more cache stored.",
                           pDPs->GetSheetCaches().size(), static_cast<size_t>(0));

    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

void Test::testPivotTableFieldReference()
{
    m_pDoc->InsertTab(0, OUString("Data"));
    m_pDoc->InsertTab(1, OUString("Table"));

    // Raw data
    const char* aData[][2] = {
        { "Name", "Value" },
        { "A", "1" },
        { "B", "2" },
        { "C", "4" },
        { "D", "8" },
    };

    // Dimension definition
    DPFieldDef aFields[] = {
        { "Name", sheet::DataPilotFieldOrientation_ROW, 0 },
        { "Value", sheet::DataPilotFieldOrientation_DATA, sheet::GeneralFunction_SUM },
    };

    ScAddress aPos(1,1,0);
    ScRange aDataRange = insertRangeData(m_pDoc, aPos, aData, SAL_N_ELEMENTS(aData));
    CPPUNIT_ASSERT_MESSAGE("failed to insert range data at correct position", aDataRange.aStart == aPos);

    ScDPObject* pDPObj = createDPFromRange(
        m_pDoc, aDataRange, aFields, SAL_N_ELEMENTS(aFields), false);

    ScDPCollection* pDPs = m_pDoc->GetDPCollection();
    bool bSuccess = pDPs->InsertNewTable(pDPObj);

    CPPUNIT_ASSERT_MESSAGE("failed to insert a new pivot table object into document.", bSuccess);
    CPPUNIT_ASSERT_MESSAGE("there should be only one data pilot table.",
                           pDPs->GetCount() == 1);
    pDPObj->SetName(pDPs->CreateNewName());

    ScRange aOutRange = refresh(pDPObj);

    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][2] = {
            { "Name", 0 },
            { "A", "1" },
            { "B", "2" },
            { "C", "4" },
            { "D", "8" },
            { "Total Result", "15" },
        };

        bSuccess = checkDPTableOutput<2>(m_pDoc, aOutRange, aOutputCheck, "Field reference (none)");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    ScDPSaveData aSaveData = *pDPObj->GetSaveData();
    sheet::DataPilotFieldReference aFieldRef;
    aFieldRef.ReferenceType = sheet::DataPilotFieldReferenceType::ITEM_DIFFERENCE;
    aFieldRef.ReferenceField = "Name";
    aFieldRef.ReferenceItemType = sheet::DataPilotFieldReferenceItemType::NAMED;
    aFieldRef.ReferenceItemName = "A";
    ScDPSaveDimension* pDim = aSaveData.GetDimensionByName("Value");
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve dimension 'Value'.", pDim);
    pDim->SetReferenceValue(&aFieldRef);
    pDPObj->SetSaveData(aSaveData);

    aOutRange = refresh(pDPObj);
    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][2] = {
            { "Name", 0 },
            { "A", 0 },
            { "B", "1" },
            { "C", "3" },
            { "D", "7" },
            { "Total Result", 0 },
        };

        bSuccess = checkDPTableOutput<2>(m_pDoc, aOutRange, aOutputCheck, "Field reference (difference from)");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    aFieldRef.ReferenceType = sheet::DataPilotFieldReferenceType::ITEM_PERCENTAGE;
    pDim->SetReferenceValue(&aFieldRef);
    pDPObj->SetSaveData(aSaveData);

    aOutRange = refresh(pDPObj);
    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][2] = {
            { "Name", 0 },
            { "A", "100.00%" },
            { "B", "200.00%" },
            { "C", "400.00%" },
            { "D", "800.00%" },
            { "Total Result", 0 },
        };

        bSuccess = checkDPTableOutput<2>(m_pDoc, aOutRange, aOutputCheck, "Field reference (% of)");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    aFieldRef.ReferenceType = sheet::DataPilotFieldReferenceType::ITEM_PERCENTAGE_DIFFERENCE;
    pDim->SetReferenceValue(&aFieldRef);
    pDPObj->SetSaveData(aSaveData);

    aOutRange = refresh(pDPObj);
    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][2] = {
            { "Name", 0 },
            { "A", 0 },
            { "B", "100.00%" },
            { "C", "300.00%" },
            { "D", "700.00%" },
            { "Total Result", 0 },
        };

        bSuccess = checkDPTableOutput<2>(m_pDoc, aOutRange, aOutputCheck, "Field reference (% difference from)");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    aFieldRef.ReferenceType = sheet::DataPilotFieldReferenceType::RUNNING_TOTAL;
    pDim->SetReferenceValue(&aFieldRef);
    pDPObj->SetSaveData(aSaveData);

    aOutRange = refresh(pDPObj);
    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][2] = {
            { "Name", 0 },
            { "A", "1" },
            { "B", "3" },
            { "C", "7" },
            { "D", "15" },
            { "Total Result", 0 },
        };

        bSuccess = checkDPTableOutput<2>(m_pDoc, aOutRange, aOutputCheck, "Field reference (Running total)");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    aFieldRef.ReferenceType = sheet::DataPilotFieldReferenceType::COLUMN_PERCENTAGE;
    pDim->SetReferenceValue(&aFieldRef);
    pDPObj->SetSaveData(aSaveData);

    aOutRange = refresh(pDPObj);
    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][2] = {
            { "Name", 0 },
            { "A", "6.67%" },
            { "B", "13.33%" },
            { "C", "26.67%" },
            { "D", "53.33%" },
            { "Total Result", "100.00%" },
        };

        bSuccess = checkDPTableOutput<2>(m_pDoc, aOutRange, aOutputCheck, "Field reference (% of column)");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    pDPs->FreeTable(pDPObj);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be no more tables.", pDPs->GetCount(), static_cast<size_t>(0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There shouldn't be any more cache stored.",
                           pDPs->GetSheetCaches().size(), static_cast<size_t>(0));

    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

void Test::testPivotTableDocFunc()
{
    m_pDoc->InsertTab(0, "Data");
    m_pDoc->InsertTab(1, "Table");

    // Raw data
    const char* aData[][2] = {
        { "Name",      "Value" },
        { "Sun",       "1" },
        { "Oracle",    "2" },
        { "Red Hat",   "4" },
        { "SUSE",      "8" },
        { "Apple",     "16" },
        { "Microsoft", "32" },
    };

    // Dimension definition
    DPFieldDef aFields[] = {
        { "Name", sheet::DataPilotFieldOrientation_ROW, 0 },
        { "Value", sheet::DataPilotFieldOrientation_DATA, sheet::GeneralFunction_SUM },
    };

    ScAddress aPos(1,1,0);
    ScRange aDataRange = insertRangeData(m_pDoc, aPos, aData, SAL_N_ELEMENTS(aData));
    CPPUNIT_ASSERT_MESSAGE("failed to insert range data at correct position", aDataRange.aStart == aPos);

    ScDPObject* pDPObj = createDPFromRange(
        m_pDoc, aDataRange, aFields, SAL_N_ELEMENTS(aFields), false);

    CPPUNIT_ASSERT_MESSAGE("Failed to create pivot table object.", pDPObj);

    // Craete a new pivot table output.
    ScDBDocFunc aFunc(getDocShell());
    bool bSuccess = aFunc.CreatePivotTable(*pDPObj, false, true);
    CPPUNIT_ASSERT_MESSAGE("Failed to create pivot table output via ScDBDocFunc.", bSuccess);
    ScDPCollection* pDPs = m_pDoc->GetDPCollection();
    CPPUNIT_ASSERT_MESSAGE("Failed to get pivot table collection.", pDPs);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pDPs->GetCount());
    pDPObj = (*pDPs)[0];
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve pivot table object from the collection", pDPObj);
    ScRange aOutRange = pDPObj->GetOutRange();
    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][2] = {
            { "Name", 0 },
            { "Apple", "16" },
            { "Microsoft", "32" },
            { "Oracle", "2" },
            { "Red Hat", "4" },
            { "Sun", "1" },
            { "SUSE", "8" },
            { "Total Result", "63" },
        };

        bSuccess = checkDPTableOutput<2>(m_pDoc, aOutRange, aOutputCheck, "Pivot table created via ScDBDocFunc");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    // Remove this pivot table output. This should also clear the pivot cache
    // it was referencing.
    bSuccess = aFunc.RemovePivotTable(*pDPObj, false, true);
    CPPUNIT_ASSERT_MESSAGE("Failed to remove pivot table output via ScDBDocFunc.", bSuccess);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), pDPs->GetCount());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), pDPs->GetSheetCaches().size());

    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

void Test::testFuncGETPIVOTDATA()
{
    m_pDoc->InsertTab(0, "Data");
    m_pDoc->InsertTab(1, "Table");

    // Raw data
    const char* aData[][2] = {
        { "Name", "Value" },
        { "A", "1" },
        { "A", "2" },
        { "A", "3" },
        { "B", "4" },
        { "B", "5" },
        { "B", "6" },
    };

    ScAddress aPos(1,1,0);
    ScRange aDataRange = insertRangeData(m_pDoc, aPos, aData, SAL_N_ELEMENTS(aData));
    CPPUNIT_ASSERT_MESSAGE("failed to insert range data at correct position", aDataRange.aStart == aPos);

    ScDPObject* pDPObj = NULL;

    {
        // Dimension definition
        DPFieldDef aFields[] = {
            { "Name", sheet::DataPilotFieldOrientation_ROW, 0 },
            { "Value", sheet::DataPilotFieldOrientation_DATA, sheet::GeneralFunction_SUM },
        };

        pDPObj = createDPFromRange(m_pDoc, aDataRange, aFields, SAL_N_ELEMENTS(aFields), false);
    }

    ScDPCollection* pDPs = m_pDoc->GetDPCollection();
    bool bSuccess = pDPs->InsertNewTable(pDPObj);

    CPPUNIT_ASSERT_MESSAGE("failed to insert a new pivot table object into document.", bSuccess);
    CPPUNIT_ASSERT_MESSAGE("there should be only one data pilot table.",
                           pDPs->GetCount() == 1);
    pDPObj->SetName(pDPs->CreateNewName());

    ScRange aOutRange = refresh(pDPObj);
    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][2] = {
            { "Name", 0 },
            { "A", "6" },
            { "B", "15" },
            { "Total Result", "21" },
        };

        bSuccess = checkDPTableOutput<2>(m_pDoc, aOutRange, aOutputCheck, "Pivot table created for GETPIVOTDATA");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    aPos = aOutRange.aEnd;
    aPos.IncRow(2); // Move 2 rows down from the table outout.

    OUString aPivotPosStr;
    aOutRange.aStart.Format(aPivotPosStr, SCA_ABS);

    sc::AutoCalcSwitch aSwitch(*m_pDoc, true); // turn autocalc on.

    // First, get the grand total.
    OUString aFormula("=GETPIVOTDATA(\"Value\";");
    aFormula += aPivotPosStr;
    aFormula += ")";
    m_pDoc->SetString(aPos, aFormula);
    double fVal = m_pDoc->GetValue(aPos);
    CPPUNIT_ASSERT_EQUAL(21.0, fVal);

    // Get the subtotal for 'A'.
    aFormula = "=GETPIVOTDATA(\"Value\";" + aPivotPosStr + ";\"Name\";\"A\")";
    m_pDoc->SetString(aPos, aFormula);
    fVal = m_pDoc->GetValue(aPos);
    CPPUNIT_ASSERT_EQUAL(6.0, fVal);

    // Get the subtotal for 'B'.
    aFormula = "=GETPIVOTDATA(\"Value\";" + aPivotPosStr + ";\"Name\";\"B\")";
    m_pDoc->SetString(aPos, aFormula);
    fVal = m_pDoc->GetValue(aPos);
    CPPUNIT_ASSERT_EQUAL(15.0, fVal);

    clearRange(m_pDoc, aPos); // Delete the formula.

    pDPs->FreeTable(pDPObj);

    {
        // Dimension definition
        DPFieldDef aFields[] = {
            { "Name", sheet::DataPilotFieldOrientation_ROW, 0 },
            { "Value", sheet::DataPilotFieldOrientation_DATA, sheet::GeneralFunction_SUM },
            { "Value", sheet::DataPilotFieldOrientation_DATA, sheet::GeneralFunction_COUNT },
        };

        pDPObj = createDPFromRange(m_pDoc, aDataRange, aFields, SAL_N_ELEMENTS(aFields), false);
    }

    pDPs->InsertNewTable(pDPObj);
    aOutRange = refresh(pDPObj);

    {
        // Expected output table content.  0 = empty cell
        const char* aOutputCheck[][3] = {
            { "Name",                "Data",           0   },
            { "A",                   "Sum - Value",   "6"  },
            {  0,                    "Count - Value", "3"  },
            { "B",                   "Sum - Value",  "15"  },
            {  0,                    "Count - Value", "3"  },
            { "Total Sum - Value",   0,               "21" },
            { "Total Count - Value", 0,               "6"  },
        };

        bSuccess = checkDPTableOutput<3>(m_pDoc, aOutRange, aOutputCheck, "Pivot table refreshed");
        CPPUNIT_ASSERT_MESSAGE("Table output check failed", bSuccess);
    }

    aPos = aOutRange.aEnd;
    aPos.IncRow(2); // move 2 rows down from the output.

    aOutRange.aStart.Format(aPivotPosStr, SCA_ABS);

    // First, get the grand totals.
    aFormula = ("=GETPIVOTDATA(\"Sum - Value\";") + aPivotPosStr + ")";
    m_pDoc->SetString(aPos, aFormula);
    fVal = m_pDoc->GetValue(aPos);
    CPPUNIT_ASSERT_EQUAL(21.0, fVal);
    aFormula = ("=GETPIVOTDATA(\"Count - Value\";") + aPivotPosStr + ")";
    m_pDoc->SetString(aPos, aFormula);
    fVal = m_pDoc->GetValue(aPos);
    CPPUNIT_ASSERT_EQUAL(6.0, fVal);

    // Get the subtotals for 'A'.
    aFormula = "=GETPIVOTDATA(\"Sum - Value\";" + aPivotPosStr + ";\"Name\";\"A\")";
    m_pDoc->SetString(aPos, aFormula);
    fVal = m_pDoc->GetValue(aPos);
    CPPUNIT_ASSERT_EQUAL(6.0, fVal);
    aFormula = "=GETPIVOTDATA(\"Count - Value\";" + aPivotPosStr + ";\"Name\";\"A\")";
    m_pDoc->SetString(aPos, aFormula);
    fVal = m_pDoc->GetValue(aPos);
    CPPUNIT_ASSERT_EQUAL(3.0, fVal);

    // Get the subtotals for 'B'.
    aFormula = "=GETPIVOTDATA(\"Sum - Value\";" + aPivotPosStr + ";\"Name\";\"B\")";
    m_pDoc->SetString(aPos, aFormula);
    fVal = m_pDoc->GetValue(aPos);
    CPPUNIT_ASSERT_EQUAL(15.0, fVal);
    aFormula = "=GETPIVOTDATA(\"Count - Value\";" + aPivotPosStr + ";\"Name\";\"B\")";
    m_pDoc->SetString(aPos, aFormula);
    fVal = m_pDoc->GetValue(aPos);
    CPPUNIT_ASSERT_EQUAL(3.0, fVal);

    pDPs->FreeTable(pDPObj);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be no more tables.", pDPs->GetCount(), static_cast<size_t>(0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There shouldn't be any more cache stored.",
                           pDPs->GetSheetCaches().size(), static_cast<size_t>(0));

    m_pDoc->DeleteTab(1);
    m_pDoc->DeleteTab(0);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
