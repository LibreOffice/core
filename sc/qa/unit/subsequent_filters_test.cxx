/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <editeng/editobj.hxx>
#include <svl/numformat.hxx>
#include <svl/zformat.hxx>
#include <svl/intitem.hxx>
#include <svx/svdpage.hxx>
#include <editeng/borderline.hxx>
#include <cellvalue.hxx>
#include <dbdata.hxx>
#include <validat.hxx>
#include <docfunc.hxx>
#include <markdata.hxx>
#include <scitems.hxx>
#include <scopetools.hxx>
#include <detfunc.hxx>
#include <cellmergeoption.hxx>
#include <postit.hxx>
#include <undomanager.hxx>

#include <com/sun/star/sheet/NamedRangeFlag.hdl>
#include "helper/qahelper.hxx"

using namespace ::com::sun::star;

/* Implementation of Filters test */

class ScFiltersTest : public ScModelTestBase
{
public:
    ScFiltersTest();
};

namespace
{
void testRangeNameImpl(const ScDocument& rDoc)
{
    //check one range data per sheet and one global more detailed
    //add some more checks here
    ScRangeData* pRangeData = rDoc.GetRangeName()->findByUpperName(u"GLOBAL1"_ustr);
    CPPUNIT_ASSERT_MESSAGE("range name Global1 not found", pRangeData);
    double aValue = rDoc.GetValue(1, 0, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("range name Global1 should reference Sheet1.A1", 1.0, aValue);
    pRangeData = rDoc.GetRangeName(0)->findByUpperName(u"LOCAL1"_ustr);
    CPPUNIT_ASSERT_MESSAGE("range name Sheet1.Local1 not found", pRangeData);
    aValue = rDoc.GetValue(1, 2, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("range name Sheet1.Local1 should reference Sheet1.A3", 3.0,
                                 aValue);
    pRangeData = rDoc.GetRangeName(1)->findByUpperName(u"LOCAL2"_ustr);
    CPPUNIT_ASSERT_MESSAGE("range name Sheet2.Local2 not found", pRangeData);
    aValue = rDoc.GetValue(1, 1, 1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("range name Sheet2.Local2 should reference Sheet2.A2", 7.0,
                                 aValue);
    //check for correct results for the remaining formulas
    aValue = rDoc.GetValue(1, 1, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("=global2 should be 2", 2.0, aValue);
    aValue = rDoc.GetValue(1, 3, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("=local2 should be 4", 4.0, aValue);
    aValue = rDoc.GetValue(2, 0, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("=SUM(global3) should be 10", 10.0, aValue);
    aValue = rDoc.GetValue(1, 0, 1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("range name Sheet2.local1 should reference Sheet1.A5", 5.0,
                                 aValue);
    // Test if Global5 ( which depends on Global6 ) is evaluated
    aValue = rDoc.GetValue(0, 5, 1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "formula Global5 should reference Global6 ( which is evaluated as local1 )", 5.0, aValue);
}

void testContentImpl(ScDocument& rDoc, bool bCheckMergedCells)
{
    double fValue;
    //check value import
    fValue = rDoc.GetValue(0, 0, 0);
    ASSERT_DOUBLES_EQUAL_MESSAGE("value not imported correctly", 1.0, fValue);
    fValue = rDoc.GetValue(0, 1, 0);
    ASSERT_DOUBLES_EQUAL_MESSAGE("value not imported correctly", 2.0, fValue);
    OUString aString = rDoc.GetString(1, 0, 0);

    //check string import
    CPPUNIT_ASSERT_EQUAL_MESSAGE("string imported not correctly", u"String1"_ustr, aString);
    aString = rDoc.GetString(1, 1, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("string not imported correctly", u"String2"_ustr, aString);

    //check basic formula import
    // in case of DIF it just contains values
    fValue = rDoc.GetValue(2, 0, 0);
    ASSERT_DOUBLES_EQUAL_MESSAGE("=2*3", 6.0, fValue);
    fValue = rDoc.GetValue(2, 1, 0);
    ASSERT_DOUBLES_EQUAL_MESSAGE("=2+3", 5.0, fValue);
    fValue = rDoc.GetValue(2, 2, 0);
    ASSERT_DOUBLES_EQUAL_MESSAGE("=2-3", -1.0, fValue);
    fValue = rDoc.GetValue(2, 3, 0);
    ASSERT_DOUBLES_EQUAL_MESSAGE("=C1+C2", 11.0, fValue);

    //check merged cells import
    if (bCheckMergedCells)
    {
        SCCOL nCol = 4;
        SCROW nRow = 1;
        rDoc.ExtendMerge(4, 1, nCol, nRow, 0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("merged cells are not imported", SCCOL(5), nCol);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("merged cells are not imported", SCROW(2), nRow);

        //check notes import
        ScAddress aAddress(7, 2, 0);
        ScPostIt* pNote = rDoc.GetNote(aAddress);
        CPPUNIT_ASSERT_MESSAGE("note not imported", pNote);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("note text not imported correctly", u"Test"_ustr,
                                     pNote->GetText());
    }

    //add additional checks here
}
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest, testContentODS)
{
    createScDoc("ods/universal-content.ods");
    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();

    testContentImpl(*getScDoc(), true);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest, testContentXLS)
{
    createScDoc("xls/universal-content.xls");
    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();

    testContentImpl(*getScDoc(), true);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest, testContentXLSX)
{
    createScDoc("xlsx/universal-content.xlsx");
    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();

    testContentImpl(*getScDoc(), true);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest, testContentXLSXStrict)
{
    // strict OOXML
    createScDoc("xlsx/universal-content-strict.xlsx");
    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();

    testContentImpl(*getScDoc(), true);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest, testContentLotus123)
{
    createScDoc("123/universal-content.123");
    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();

    testContentImpl(*getScDoc(), false);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest, testContentDIF)
{
    createScDoc("dif/universal-content.dif");
    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();

    testContentImpl(*getScDoc(), false);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest, testContentXLSB)
{
    createScDoc("xlsb/universal-content.xlsb");
    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();

    testContentImpl(*getScDoc(), true);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest, testContentXLS_XML)
{
    createScDoc("xml/universal-content.xml");
    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();

    testContentImpl(*getScDoc(), false);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest, testContentGnumeric)
{
    createScDoc("gnumeric/universal-content.gnumeric");
    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();

    testContentImpl(*getScDoc(), false);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest, testTdf119292)
{
    createScDoc("xlsx/tdf119292.xlsx");

    ScDocument* pDoc = getScDoc();
    auto* pDev = pDoc->GetRefDevice();
    Size aMarkSize(4, 6);
    Color aArrowFillCol(COL_LIGHTRED);

    // check the points of the polygon if the text is rotated 90 degrees
    tools::Rectangle aMarkRect1(0, 0, 45, 3);
    tools::Polygon aPoly90degrees
        = SvxFont::DrawArrow(*pDev, aMarkRect1, aMarkSize, aArrowFillCol, true, true);
    Point aPoly90Pos1 = aPoly90degrees.GetPoint(0);
    Point aPoly90Pos2 = aPoly90degrees.GetPoint(1);
    Point aPoly90Pos3 = aPoly90degrees.GetPoint(2);
    CPPUNIT_ASSERT_EQUAL(Point(19, 3), aPoly90Pos1);
    CPPUNIT_ASSERT_EQUAL(Point(22, 0), aPoly90Pos2);
    CPPUNIT_ASSERT_EQUAL(Point(25, 3), aPoly90Pos3);

    // check the points of the polygon if the text is rotated 270 degrees
    tools::Rectangle aMarkRect2(89, 62, 134, 57);
    tools::Polygon aPoly270degrees
        = SvxFont::DrawArrow(*pDev, aMarkRect2, aMarkSize, aArrowFillCol, false, true);
    Point aPoly270Pos1 = aPoly270degrees.GetPoint(0);
    Point aPoly270Pos2 = aPoly270degrees.GetPoint(1);
    Point aPoly270Pos3 = aPoly270degrees.GetPoint(2);
    CPPUNIT_ASSERT_EQUAL(Point(108, 54), aPoly270Pos1);
    CPPUNIT_ASSERT_EQUAL(Point(111, 57), aPoly270Pos2);
    CPPUNIT_ASSERT_EQUAL(Point(114, 54), aPoly270Pos3);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest, testTdf139934)
{
    createScDoc("xlsx/tdf139934.xlsx");

    ScDocument* pDoc = getScDoc();

    // Without the fix in place, this test would have failed with
    // - Expected: 1/20/2021
    // - Actual  : 44216
    CPPUNIT_ASSERT_EQUAL(u"1/20/2021"_ustr, pDoc->GetString(0, 2, 0));
    CPPUNIT_ASSERT_EQUAL(u"11/25/2020"_ustr, pDoc->GetString(0, 61, 0));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest, testTdf150452)
{
    // Without the fix in place, this test would have crashed
    createScDoc("csv/tdf150452.csv");

    ScDocument* pDoc = getScDoc();

    CPPUNIT_ASSERT_EQUAL(u"1-GDUSF"_ustr, pDoc->GetString(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"1-GE41L"_ustr, pDoc->GetString(0, 3998, 0));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest, testTdf48731)
{
    createScDoc("csv/tdf48731.csv");

    ScDocument* pDoc = getScDoc();

    CPPUNIT_ASSERT_EQUAL(u"'"_ustr, pDoc->GetString(1, 1, 0));
    CPPUNIT_ASSERT_EQUAL(u"''"_ustr, pDoc->GetString(1, 2, 0));
    CPPUNIT_ASSERT_EQUAL(u"'''"_ustr, pDoc->GetString(1, 3, 0));

    // Without the fix in place, this test would have failed with
    // - Expected: '3
    // - Actual  : 3
    CPPUNIT_ASSERT_EQUAL(u"'3"_ustr, pDoc->GetString(1, 4, 0));
    CPPUNIT_ASSERT_EQUAL(u"'word"_ustr, pDoc->GetString(1, 5, 0));
    CPPUNIT_ASSERT_EQUAL(u"'mword"_ustr, pDoc->GetString(1, 6, 0));

    CPPUNIT_ASSERT_EQUAL(u"'"_ustr, pDoc->GetString(1, 9, 0));
    CPPUNIT_ASSERT_EQUAL(u"''"_ustr, pDoc->GetString(1, 10, 0));
    CPPUNIT_ASSERT_EQUAL(u"'''"_ustr, pDoc->GetString(1, 11, 0));
    CPPUNIT_ASSERT_EQUAL(u"'3"_ustr, pDoc->GetString(1, 12, 0));
    CPPUNIT_ASSERT_EQUAL(u"'word"_ustr, pDoc->GetString(1, 13, 0));
    CPPUNIT_ASSERT_EQUAL(u"'mword"_ustr, pDoc->GetString(1, 14, 0));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest, testUpdateCircleInMergedCellODS)
{
    createScDoc("ods/updateCircleInMergedCell.ods");

    ScDocument* pDoc = getScDoc();
    pDoc->EnableChangeReadOnly(true);

    ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
    SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("draw page for sheet 1 should exist.", pPage);

    // There should be four circle objects!
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), pPage->GetObjCount());

    ScCellMergeOption aCellMergeOption(0, 0, 1, 1); // A1:B2
    aCellMergeOption.maTabs.insert(0);
    ScDocShell* pDocSh = getScDocShell();
    pDocSh->GetDocFunc().MergeCells(aCellMergeOption, false, true, true, false);

    // There should be a circle object!
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pPage->GetObjCount());

    pDocSh->GetDocFunc().UnmergeCells(aCellMergeOption, true, nullptr);

    // There should be four circle objects!
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), pPage->GetObjCount());
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest, testDeleteCircleInMergedCellODS)
{
    createScDoc("ods/deleteCircleInMergedCell.ods");

    ScDocument* pDoc = getScDoc();

    ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
    SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("draw page for sheet 1 should exist.", pPage);

    // There should be a circle object!
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pPage->GetObjCount());

    ScRefCellValue aMergedCell;
    ScAddress aPosMergedCell(0, 0, 0);
    aMergedCell.assign(*pDoc, aPosMergedCell);

    // The value of merged cell change to 6.
    aMergedCell = ScRefCellValue(6);

    // Check that the data is valid.(True if the value = 6)
    const ScValidationData* pData = pDoc->GetValidationEntry(1);
    bool bValidA1 = pData->IsDataValid(aMergedCell, aPosMergedCell);
    // if valid, delete circle.
    if (bValidA1)
        ScDetectiveFunc(*pDoc, 0).DeleteCirclesAt(aPosMergedCell.Col(), aPosMergedCell.Row());

    // There should not be a circle object!
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), pPage->GetObjCount());
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest, testBasicCellContentODS)
{
    createScDoc("ods/basic-cell-content.ods");

    ScDocument* pDoc = getScDoc();
    OUString aStr = pDoc->GetString(1, 1, 0); // B2
    CPPUNIT_ASSERT_EQUAL(u"LibreOffice Calc"_ustr, aStr);
    double fVal = pDoc->GetValue(1, 2, 0); // B3
    CPPUNIT_ASSERT_EQUAL(12345.0, fVal);
    aStr = pDoc->GetString(1, 3, 0); // B4
    CPPUNIT_ASSERT_EQUAL(u"A < B"_ustr, aStr);

    // Numeric value of 0.
    ScRefCellValue aCell;
    aCell.assign(*pDoc, ScAddress(1, 4, 0)); // B5
    CPPUNIT_ASSERT_EQUAL_MESSAGE("This cell must be numeric.", CELLTYPE_VALUE, aCell.getType());
    CPPUNIT_ASSERT_EQUAL(0.0, aCell.getDouble());
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest, testBooleanFormatXLSX)
{
    createScDoc("xlsx/check-boolean.xlsx");
    ScDocument* pDoc = getScDoc();
    SvNumberFormatter* pNumFormatter = pDoc->GetFormatTable();
    // Saved as >"TRUE";"TRUE";"FALSE"< but reading converted back to >BOOLEAN<

    for (SCROW i = 0; i <= 1; i++)
    {
        sal_uInt32 nNumberFormat = pDoc->GetNumberFormat(0, i, 0);
        const SvNumberformat* pNumberFormat = pNumFormatter->GetEntry(nNumberFormat);
        const OUString& rFormatStr = pNumberFormat->GetFormatstring();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Number format != boolean", u"BOOLEAN"_ustr, rFormatStr);
    }
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest, testTdf143809)
{
    createScDoc("ods/tdf143809.ods");

    ScDocument* pDoc = getScDoc();

    OUString aFormula = pDoc->GetFormula(0, 0, 0);
    CPPUNIT_ASSERT_EQUAL(u"=SUMPRODUCT(IFERROR(CEILING.MATH(DURATIONS,300),0))"_ustr, aFormula);

    // Without the fix in place, this test would have failed with
    // - Expected: 53700
    // - Actual  : Err:502
    CPPUNIT_ASSERT_EQUAL(u"53700"_ustr, pDoc->GetString(0, 0, 0));

    aFormula = pDoc->GetFormula(0, 1, 0);
    CPPUNIT_ASSERT_EQUAL(
        u"=SUMPRODUCT(IFERROR(CEILING(SUMIFS(DURATIONS,IDS,IDS),300)/COUNTIFS(IDS,IDS),0))"_ustr,
        aFormula);
    CPPUNIT_ASSERT_EQUAL(u"51900"_ustr, pDoc->GetString(0, 1, 0));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest, testTdf76310)
{
    createScDoc("ods/tdf76310.ods");

    ScDocument* pDoc = getScDoc();

    OUString aFormula = pDoc->GetFormula(0, 0, 0);
    // Without the fix in place, this test would have failed with
    // - Expected: =1
    // +
    // 2
    // - Actual  : =1 + 2
    CPPUNIT_ASSERT_EQUAL(u"=1\n+\n2"_ustr, aFormula);
    ASSERT_DOUBLES_EQUAL(3.0, pDoc->GetValue(0, 0, 0));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest, testRangeNameXLS)
{
    createScDoc("xls/named-ranges-global.xls");
    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();

    ScDocument* pDoc = getScDoc();
    testRangeNameImpl(*pDoc);

    OUString aCSVPath = createFilePath(u"contentCSV/rangeExp_Sheet2.csv");
    // fdo#44587
    testFile(aCSVPath, *pDoc, 1);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest, testRangeNameLocalXLS)
{
    createScDoc("xls/named-ranges-local.xls");
    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();

    ScDocument* pDoc = getScDoc();
    ScRangeName* pRangeName = pDoc->GetRangeName(0);
    CPPUNIT_ASSERT(pRangeName);
    CPPUNIT_ASSERT_EQUAL(size_t(2), pRangeName->size());

    OUString aFormula = pDoc->GetFormula(3, 11, 0);
    CPPUNIT_ASSERT_EQUAL(u"=SUM(local_name2)"_ustr, aFormula);
    ASSERT_DOUBLES_EQUAL(14.0, pDoc->GetValue(3, 11, 0));

    aFormula = pDoc->GetFormula(6, 4, 0);
    CPPUNIT_ASSERT_EQUAL(u"=local_name1"_ustr, aFormula);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest, testRangeNameXLSX)
{
    createScDoc("xlsx/named-ranges-global.xlsx");
    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();

    ScDocument* pDoc = getScDoc();
    testRangeNameImpl(*pDoc);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest, testRangeNameODS)
{
    createScDoc("ods/named-ranges-global.ods");

    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();

    ScDocument* pDoc = getScDoc();
    testRangeNameImpl(*pDoc);

    OUString aCSVPath = createFilePath(u"contentCSV/rangeExp_Sheet2.csv");
    testFile(aCSVPath, *pDoc, 1);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest, testHiddenRangeNameODS)
{
    createScDoc("ods/named-ranges-hidden.ods");
    ScDocument* pDoc = getScDoc();

    // This named range is set to "hidden"
    ScRangeData* pRangeData1 = pDoc->GetRangeName()->findByUpperName(u"NAMEDRANGE1"_ustr);
    CPPUNIT_ASSERT(pRangeData1);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(sheet::NamedRangeFlag::HIDDEN),
                         pRangeData1->GetUnoType() & sheet::NamedRangeFlag::HIDDEN);
    // This named range is visible
    ScRangeData* pRangeData2 = pDoc->GetRangeName()->findByUpperName(u"NAMEDRANGE2"_ustr);
    CPPUNIT_ASSERT(pRangeData2);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(0),
                         pRangeData2->GetUnoType() & sheet::NamedRangeFlag::HIDDEN);

    // Set NamedRange2 to hidden
    pRangeData2->AddType(ScRangeData::Type::Hidden);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(sheet::NamedRangeFlag::HIDDEN),
                         pRangeData2->GetUnoType() & sheet::NamedRangeFlag::HIDDEN);

    // Check if both named ranges are hidden after saving and reloading
    saveAndReload(u"calc8"_ustr);
    pDoc = getScDoc();
    pRangeData1 = pDoc->GetRangeName()->findByUpperName(u"NAMEDRANGE1"_ustr);
    CPPUNIT_ASSERT(pRangeData1);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(sheet::NamedRangeFlag::HIDDEN),
                         pRangeData1->GetUnoType() & sheet::NamedRangeFlag::HIDDEN);
    pRangeData2 = pDoc->GetRangeName()->findByUpperName(u"NAMEDRANGE2"_ustr);
    CPPUNIT_ASSERT(pRangeData2);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(sheet::NamedRangeFlag::HIDDEN),
                         pRangeData2->GetUnoType() & sheet::NamedRangeFlag::HIDDEN);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest, testHiddenRangeNameXLSX)
{
    createScDoc("xlsx/named-ranges-hidden.xlsx");
    ScDocument* pDoc = getScDoc();

    // This named range is set to "hidden"
    ScRangeData* pRangeData1 = pDoc->GetRangeName()->findByUpperName(u"NAMEDRANGE1"_ustr);
    CPPUNIT_ASSERT(pRangeData1);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(sheet::NamedRangeFlag::HIDDEN),
                         pRangeData1->GetUnoType() & sheet::NamedRangeFlag::HIDDEN);
    // This named range is visible
    ScRangeData* pRangeData2 = pDoc->GetRangeName()->findByUpperName(u"NAMEDRANGE2"_ustr);
    CPPUNIT_ASSERT(pRangeData2);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(0),
                         pRangeData2->GetUnoType() & sheet::NamedRangeFlag::HIDDEN);

    // Save as ODS and test if the named ranges are still with the correct hidden flag
    saveAndReload(u"calc8"_ustr);
    pDoc = getScDoc();
    pRangeData1 = pDoc->GetRangeName()->findByUpperName(u"NAMEDRANGE1"_ustr);
    CPPUNIT_ASSERT(pRangeData1);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(sheet::NamedRangeFlag::HIDDEN),
                         pRangeData1->GetUnoType() & sheet::NamedRangeFlag::HIDDEN);
    pRangeData2 = pDoc->GetRangeName()->findByUpperName(u"NAMEDRANGE2"_ustr);
    CPPUNIT_ASSERT(pRangeData2);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(0),
                         pRangeData2->GetUnoType() & sheet::NamedRangeFlag::HIDDEN);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest, testHiddenNamedExpression)
{
    createScDoc();
    ScDocument* pDoc = getScDoc();

    // Adds two hidden named expressions and two non-hidden named expressions
    ScRangeName* pNamedRanges = pDoc->GetRangeName();
    ScRangeData* pRangeData1 = new ScRangeData(*pDoc, u"NAME1"_ustr, u"100"_ustr);
    pRangeData1->AddType(ScRangeData::Type::Hidden);
    pNamedRanges->insert(pRangeData1);
    ScRangeData* pRangeData2 = new ScRangeData(*pDoc, u"NAME2"_ustr, u"text1"_ustr);
    pRangeData2->AddType(ScRangeData::Type::Hidden);
    pNamedRanges->insert(pRangeData2);
    ScRangeData* pRangeData3 = new ScRangeData(*pDoc, u"NAME3"_ustr, u"200"_ustr);
    pNamedRanges->insert(pRangeData3);
    ScRangeData* pRangeData4 = new ScRangeData(*pDoc, u"NAME4"_ustr, u"text2"_ustr);
    pNamedRanges->insert(pRangeData4);
    CPPUNIT_ASSERT_EQUAL(size_t(4), pNamedRanges->size());

    // Save and reload to test whether the named expressions retain the hidden  where applicable
    saveAndReload(u"calc8"_ustr);
    pDoc = getScDoc();
    pNamedRanges = pDoc->GetRangeName();
    CPPUNIT_ASSERT_EQUAL(size_t(4), pNamedRanges->size());
    pRangeData1 = pNamedRanges->findByUpperName(u"NAME1"_ustr);
    CPPUNIT_ASSERT(pRangeData1);
    CPPUNIT_ASSERT_EQUAL(ScRangeData::Type::Hidden, pRangeData1->GetType());
    CPPUNIT_ASSERT_EQUAL(u"100"_ustr, pRangeData1->GetSymbol());
    pRangeData2 = pNamedRanges->findByUpperName(u"NAME2"_ustr);
    CPPUNIT_ASSERT(pRangeData2);
    CPPUNIT_ASSERT_EQUAL(ScRangeData::Type::Hidden, pRangeData2->GetType());
    CPPUNIT_ASSERT_EQUAL(u"text1"_ustr, pRangeData2->GetSymbol());
    pRangeData3 = pNamedRanges->findByUpperName(u"NAME3"_ustr);
    CPPUNIT_ASSERT(pRangeData3);
    CPPUNIT_ASSERT_EQUAL(ScRangeData::Type::Name, pRangeData3->GetType());
    CPPUNIT_ASSERT_EQUAL(u"200"_ustr, pRangeData3->GetSymbol());
    pRangeData4 = pNamedRanges->findByUpperName(u"NAME4"_ustr);
    CPPUNIT_ASSERT(pRangeData4);
    CPPUNIT_ASSERT_EQUAL(ScRangeData::Type::Name, pRangeData4->GetType());
    CPPUNIT_ASSERT_EQUAL(u"text2"_ustr, pRangeData4->GetSymbol());
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest, testHiddenNamedExpressionODS)
{
    createScDoc("ods/NamedExpressionsHidden.ods");
    ScDocument* pDoc = getScDoc();

    // The document has 2 named expressions; the first is hidden; the second is visible
    ScRangeName* pNamedRanges = pDoc->GetRangeName();
    ScRangeData* pRangeData1 = pNamedRanges->findByUpperName(u"NAME1"_ustr);
    CPPUNIT_ASSERT(pRangeData1);
    CPPUNIT_ASSERT_EQUAL(ScRangeData::Type::Hidden, pRangeData1->GetType());
    CPPUNIT_ASSERT_EQUAL(u"100"_ustr, pRangeData1->GetSymbol());
    ScRangeData* pRangeData2 = pNamedRanges->findByUpperName(u"NAME2"_ustr);
    CPPUNIT_ASSERT(pRangeData2);
    CPPUNIT_ASSERT_EQUAL(ScRangeData::Type::Name, pRangeData2->GetType());
    CPPUNIT_ASSERT_EQUAL(u"200"_ustr, pRangeData2->GetSymbol());
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest, testHyperlinksXLSX)
{
    createScDoc("xlsx/hyperlinks.xlsx");
    ScDocument* pDoc = getScDoc();

    CPPUNIT_ASSERT_EQUAL(u"10:ABC10"_ustr, pDoc->GetString(ScAddress(0, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(u"10:ABC11"_ustr, pDoc->GetString(ScAddress(0, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(u"10:ABC12"_ustr, pDoc->GetString(ScAddress(0, 3, 0)));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest, testHardRecalcODS)
{
    createScDoc("ods/hard-recalc.ods");
    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();

    ScDocument* pDoc = getScDoc();

    //test hard recalc: document has an incorrect cached formula result
    //hard recalc should have updated to the correct result
    OUString aCSVPath = createFilePath(u"contentCSV/hard-recalc.csv");
    testFile(aCSVPath, *pDoc, 0);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest, testFunctionsODS)
{
    createScDoc("ods/functions.ods");
    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();

    ScDocument* pDoc = getScDoc();

    //test logical functions
    OUString aCSVPath = createFilePath(u"contentCSV/logical-functions.csv");
    testFile(aCSVPath, *pDoc, 0);
    //test spreadsheet functions
    aCSVPath = createFilePath(u"contentCSV/spreadsheet-functions.csv");
    testFile(aCSVPath, *pDoc, 1);
    //test mathematical functions
    aCSVPath = createFilePath(u"contentCSV/mathematical-functions.csv");
    testFile(aCSVPath, *pDoc, 2, StringType::PureString);
    //test information functions
    aCSVPath = createFilePath(u"contentCSV/information-functions.csv");
    testFile(aCSVPath, *pDoc, 3);
    // text functions
    aCSVPath = createFilePath(u"contentCSV/text-functions.csv");
    testFile(aCSVPath, *pDoc, 4, StringType::PureString);
    // statistical functions
    aCSVPath = createFilePath(u"contentCSV/statistical-functions.csv");
    testFile(aCSVPath, *pDoc, 5);
    // financial functions
    aCSVPath = createFilePath(u"contentCSV/financial-functions.csv");
    testFile(aCSVPath, *pDoc, 6);

    createScDoc("ods/database-functions.ods");
    pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();
    pDoc = getScDoc();

    aCSVPath = createFilePath(u"contentCSV/database-functions.csv");
    testFile(aCSVPath, *pDoc, 0);

    createScDoc("ods/date-time-functions.ods");
    pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();
    pDoc = getScDoc();
    aCSVPath = createFilePath(u"contentCSV/date-time-functions.csv");
    testFile(aCSVPath, *pDoc, 0, StringType::PureString);

    createScDoc("ods/user-defined-function.ods");
    pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();
    pDoc = getScDoc();
    aCSVPath = createFilePath(u"contentCSV/user-defined-function.csv");
    testFile(aCSVPath, *pDoc, 0);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest, testCachedFormulaResultsODS)
{
    {
        createScDoc("ods/functions.ods");

        ScDocument* pDoc = getScDoc();

        //test cached formula results of logical functions
        OUString aCSVPath = createFilePath(u"contentCSV/logical-functions.csv");
        testFile(aCSVPath, *pDoc, 0);
        //test cached formula results of spreadsheet functions
        aCSVPath = createFilePath(u"contentCSV/spreadsheet-functions.csv");
        testFile(aCSVPath, *pDoc, 1);
        //test cached formula results of mathematical functions
        aCSVPath = createFilePath(u"contentCSV/mathematical-functions.csv");
        testFile(aCSVPath, *pDoc, 2, StringType::PureString);
        //test cached formula results of information functions
        aCSVPath = createFilePath(u"contentCSV/information-functions.csv");
        testFile(aCSVPath, *pDoc, 3);
        // text functions
        aCSVPath = createFilePath(u"contentCSV/text-functions.csv");
        testFile(aCSVPath, *pDoc, 4, StringType::PureString);
    }

    {
        createScDoc("ods/cachedValue.ods");

        ScDocument* pDoc = getScDoc();
        OUString aCSVPath = createFilePath(u"contentCSV/cachedValue.csv");
        testFile(aCSVPath, *pDoc, 0);

        //we want to me sure that volatile functions are always recalculated
        //regardless of cached results.  if you update the ods file, you must
        //update the values here.
        //if NOW() is recalculated, then it should never equal "01/25/13 01:06 PM"
        OUString sTodayRecalc(pDoc->GetString(0, 0, 1));

        CPPUNIT_ASSERT("01/25/13 01:06 PM" != sTodayRecalc);

        OUString sTodayRecalcRef(pDoc->GetString(1, 0, 1));
        CPPUNIT_ASSERT_EQUAL(sTodayRecalc, sTodayRecalcRef);

        // make sure that error values are not being treated as string values
        for (SCCOL nCol = 0; nCol < 4; ++nCol)
        {
            for (SCROW nRow = 0; nRow < 2; ++nRow)
            {
                OUString aFormula = "=ISERROR(" + OUStringChar(static_cast<char>('A' + nCol))
                                    + OUString::number(nRow) + ")";
                pDoc->SetString(nCol, nRow + 2, 2, aFormula);
                CPPUNIT_ASSERT_EQUAL_MESSAGE(
                    OUStringToOString(aFormula, RTL_TEXTENCODING_UTF8).getStr(), u"TRUE"_ustr,
                    pDoc->GetString(nCol, nRow + 2, 2));

                OUString aIsTextFormula = "=ISTEXT("
                                          + OUString::number(static_cast<char>('A' + nCol))
                                          + OUString::number(nRow) + ")";
                pDoc->SetString(nCol, nRow + 4, 2, aIsTextFormula);
                CPPUNIT_ASSERT_EQUAL(u"FALSE"_ustr, pDoc->GetString(nCol, nRow + 4, 2));
            }
        }
    }
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest, testCachedMatrixFormulaResultsODS)
{
    createScDoc("ods/matrix.ods");

    ScDocument* pDoc = getScDoc();

    //test matrix
    OUString aCSVPath = createFilePath(u"contentCSV/matrix.csv");
    testFile(aCSVPath, *pDoc, 0);
    //test matrices with special cases
    aCSVPath = createFilePath(u"contentCSV/matrix2.csv");
    testFile(aCSVPath, *pDoc, 1);
    aCSVPath = createFilePath(u"contentCSV/matrix3.csv");
    testFile(aCSVPath, *pDoc, 2);
    //The above testFile() does not catch the below case.
    //If a matrix formula has a matrix reference cell that is intended to have
    //a blank text result, the matrix reference cell is actually saved(export)
    //as a float cell with 0 as the value and an empty <text:p/>.
    //Import works around this by setting these cells as text cells so that
    //the blank text is used for display instead of the number 0.
    //If this is working properly, the following cell should NOT have value data.
    CPPUNIT_ASSERT_EQUAL(OUString(), pDoc->GetString(3, 0, 2));

    // fdo#59293 with cached value import error formulas require special
    // treatment
    pDoc->SetString(2, 5, 2, u"=ISERROR(A6)"_ustr);
    double nVal = pDoc->GetValue(2, 5, 2);
    CPPUNIT_ASSERT_EQUAL(1.0, nVal);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest, testFormulaDepAcrossSheetsODS)
{
    createScDoc("ods/formula-across-sheets.ods");
    ScDocument* pDoc = getScDoc();

    sc::AutoCalcSwitch aACSwitch(*pDoc, true); // Make sure auto calc is turned on.

    // Save the original values of A4:C4.
    double fA4 = pDoc->GetValue(ScAddress(0, 3, 2));
    double fB4 = pDoc->GetValue(ScAddress(1, 3, 2));
    double fC4 = pDoc->GetValue(ScAddress(2, 3, 2));

    // Change the value of D4. This should trigger A4:C4 to be recalculated.
    double fD4 = pDoc->GetValue(ScAddress(3, 3, 2));
    pDoc->SetValue(ScAddress(3, 3, 2), fD4 + 1.0);

    CPPUNIT_ASSERT_MESSAGE("The value must differ from the original.",
                           fA4 != pDoc->GetValue(ScAddress(0, 3, 2)));
    CPPUNIT_ASSERT_MESSAGE("The value must differ from the original.",
                           fB4 != pDoc->GetValue(ScAddress(1, 3, 2)));
    CPPUNIT_ASSERT_MESSAGE("The value must differ from the original.",
                           fC4 != pDoc->GetValue(ScAddress(2, 3, 2)));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest, testFormulaDepDeleteContentsODS)
{
    createScDoc("ods/formula-delete-contents.ods");
    ScDocument* pDoc = getScDoc();

    sc::UndoSwitch aUndoSwitch(*pDoc, true); // Enable undo.
    sc::AutoCalcSwitch aACSwitch(*pDoc, true); // Make sure auto calc is turned on.

    CPPUNIT_ASSERT_EQUAL(195.0, pDoc->GetValue(ScAddress(3, 15, 0))); // formula in D16

    // Delete D2:D5.
    ScDocShell* pDocSh = getScDocShell();
    ScDocFunc& rFunc = pDocSh->GetDocFunc();
    ScRange aRange(3, 1, 0, 3, 4, 0);
    ScMarkData aMark(pDoc->GetSheetLimits());
    aMark.SetMarkArea(aRange);
    aMark.MarkToMulti();
    bool bGood = rFunc.DeleteContents(aMark, InsertDeleteFlags::ALL, true, true);
    CPPUNIT_ASSERT(bGood);
    CPPUNIT_ASSERT_EQUAL(0.0, pDoc->GetValue(ScAddress(3, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(0.0, pDoc->GetValue(ScAddress(3, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(0.0, pDoc->GetValue(ScAddress(3, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(0.0, pDoc->GetValue(ScAddress(3, 4, 0)));

    CPPUNIT_ASSERT_EQUAL(94.0, pDoc->GetValue(ScAddress(3, 15, 0))); // formula in D16

    SfxUndoManager* pUndoMgr = pDoc->GetUndoManager();
    CPPUNIT_ASSERT(pUndoMgr);
    pUndoMgr->Undo();
    CPPUNIT_ASSERT_EQUAL(195.0, pDoc->GetValue(ScAddress(3, 15, 0))); // formula in D16
}

namespace
{
void testDBRanges_Impl(ScDocument& rDoc, bool bIsODS)
{
    ScDBCollection* pDBCollection = rDoc.GetDBCollection();
    CPPUNIT_ASSERT_MESSAGE("no database collection", pDBCollection);

    ScDBData* pAnonDBData = rDoc.GetAnonymousDBData(0);
    CPPUNIT_ASSERT_MESSAGE("missing anonymous DB data in sheet 1", pAnonDBData);
    //control hidden rows
    bool bHidden;
    SCROW nRow1, nRow2;
    bHidden = rDoc.RowHidden(0, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("Sheet1: row 0 should be visible", !bHidden);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Sheet1: row 0 should be visible", SCROW(0), nRow1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Sheet1: row 0 should be visible", SCROW(0), nRow2);
    bHidden = rDoc.RowHidden(1, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("Sheet1: rows 1-2 should be hidden", bHidden);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Sheet1: rows 1-2 should be hidden", SCROW(1), nRow1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Sheet1: rows 1-2 should be hidden", SCROW(2), nRow2);
    bHidden = rDoc.RowHidden(3, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("Sheet1: row 3 should be visible", !bHidden);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Sheet1: row 3 should be visible", SCROW(3), nRow1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Sheet1: row 3 should be visible", SCROW(3), nRow2);
    bHidden = rDoc.RowHidden(4, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("Sheet1: row 4-5 should be hidden", bHidden);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Sheet1: row 4-5 should be hidden", SCROW(4), nRow1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Sheet1: row 4-5 should be hidden", SCROW(5), nRow2);
    bHidden = rDoc.RowHidden(6, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("Sheet1: row 6-end should be visible", !bHidden);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Sheet1: row 6-end should be visible", SCROW(6), nRow1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Sheet1: row 6-end should be visible", rDoc.MaxRow(), nRow2);
    if (bIsODS) //excel doesn't support named db ranges
    {
        double aValue = rDoc.GetValue(0, 10, 1);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Sheet2: A11: formula result is incorrect", 4.0, aValue);
        aValue = rDoc.GetValue(1, 10, 1);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Sheet2: B11: formula result is incorrect", 2.0, aValue);
    }
    double aValue = rDoc.GetValue(3, 10, 1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Sheet2: D11: formula result is incorrect", 4.0, aValue);
    aValue = rDoc.GetValue(4, 10, 1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Sheet2: E11: formula result is incorrect", 2.0, aValue);
}
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest, testDatabaseRangesODS)
{
    createScDoc("ods/database.ods");
    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();

    ScDocument* pDoc = getScDoc();

    testDBRanges_Impl(*pDoc, true);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest, testDatabaseRangesXLS)
{
    createScDoc("xls/database.xls");
    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();

    ScDocument* pDoc = getScDoc();

    testDBRanges_Impl(*pDoc, false);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest, testDatabaseRangesXLSX)
{
    createScDoc("xlsx/database.xlsx");
    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();

    ScDocument* pDoc = getScDoc();

    testDBRanges_Impl(*pDoc, false);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest, testFormatsODS)
{
    createScDoc("ods/formats.ods");
    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();

    ScDocument* pDoc = getScDoc();

    testFormats(pDoc, u"calc8");
}

// void ScFiltersTest::testFormatsXLS()
// {
//     ScDocShellRef xDocSh = loadDoc("formats.", FORMAT_XLS);
//     xDocSh->DoHardRecalc();
//
//     ScDocument& rDoc = xDocSh->GetDocument();
//
//     testFormats(rDoc, FORMAT_XLS);
// }

// void ScFiltersTest::testFormatsXLSX()
// {
//     ScDocShellRef xDocSh = loadDoc("formats.", FORMAT_XLSX);
//     xDocSh->DoHardRecalc();
//
//     ScDocument& rDoc = xDocSh->GetDocument();
//
//     testFormats(rDoc, FORMAT_XLSX);
// }

CPPUNIT_TEST_FIXTURE(ScFiltersTest, testMatrixODS)
{
    createScDoc("ods/matrix.ods");
    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();

    ScDocument* pDoc = getScDoc();

    OUString aCSVPath = createFilePath(u"contentCSV/matrix.csv");
    testFile(aCSVPath, *pDoc, 0);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest, testMatrixXLS)
{
    createScDoc("xls/matrix.xls");
    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();

    ScDocument* pDoc = getScDoc();

    OUString aCSVPath = createFilePath(u"contentCSV/matrix.csv");
    testFile(aCSVPath, *pDoc, 0);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest, testDoubleThinBorder)
{
    // double-thin borders created with size less than 1.15 where invisible (and subsequently lost) on round-trips.
    createScDoc("ods/tdf88827_borderDoubleThin.ods");

    ScDocument* pDoc = getScDoc();

    const editeng::SvxBorderLine* pLeft = nullptr;
    const editeng::SvxBorderLine* pTop = nullptr;
    const editeng::SvxBorderLine* pRight = nullptr;
    const editeng::SvxBorderLine* pBottom = nullptr;

    pDoc->GetBorderLines(2, 2, 0, &pLeft, &pTop, &pRight, &pBottom);
    CPPUNIT_ASSERT(pTop);
    CPPUNIT_ASSERT(pRight);
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::DOUBLE_THIN, pRight->GetBorderLineStyle());
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest, testBorderODS)
{
    createScDoc("ods/border.ods");

    ScDocument* pDoc = getScDoc();

    const editeng::SvxBorderLine* pLeft = nullptr;
    const editeng::SvxBorderLine* pTop = nullptr;
    const editeng::SvxBorderLine* pRight = nullptr;
    const editeng::SvxBorderLine* pBottom = nullptr;

    pDoc->GetBorderLines(0, 1, 0, &pLeft, &pTop, &pRight, &pBottom);
    CPPUNIT_ASSERT(!pLeft);
    CPPUNIT_ASSERT(!pTop);
    CPPUNIT_ASSERT(!pBottom);
    CPPUNIT_ASSERT(pRight);
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, pRight->GetBorderLineStyle());

    pDoc->GetBorderLines(2, 1, 0, &pLeft, &pTop, &pRight, &pBottom);
    CPPUNIT_ASSERT(!pLeft);
    CPPUNIT_ASSERT(!pTop);
    CPPUNIT_ASSERT(!pBottom);

    CPPUNIT_ASSERT(pRight);
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, pRight->GetBorderLineStyle());
    CPPUNIT_ASSERT_EQUAL(tools::Long(20), pRight->GetWidth());

    pDoc->GetBorderLines(2, 8, 0, &pLeft, &pTop, &pRight, &pBottom);

    CPPUNIT_ASSERT(pLeft);
    CPPUNIT_ASSERT(pTop);
    CPPUNIT_ASSERT(pBottom);
    CPPUNIT_ASSERT(pRight);
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, pRight->GetBorderLineStyle());
    CPPUNIT_ASSERT_EQUAL(tools::Long(5), pRight->GetWidth());
    CPPUNIT_ASSERT_EQUAL(COL_BLUE, pRight->GetColor());
}

namespace
{
struct Border
{
    sal_Int16 column;
    sal_Int32 row;
    tools::Long leftWidth;
    tools::Long topWidth;
    tools::Long rightWidth;
    tools::Long bottomWidth;
    sal_uInt16 lOutWidth;
    sal_uInt16 lInWidth;
    sal_uInt16 lDistance;
    sal_uInt16 tOutWidth;
    sal_uInt16 tInWidth;
    sal_uInt16 tDistance;
    sal_uInt16 rOutWidth;
    sal_uInt16 rInWidth;
    sal_uInt16 rDistance;
    sal_uInt16 bOutWidth;
    sal_uInt16 bInWidth;
    sal_uInt16 bDistance;
    SvxBorderLineStyle lStyle;
    SvxBorderLineStyle tStyle;
    SvxBorderLineStyle rStyle;
    SvxBorderLineStyle bStyle;
    // that's a monster
    Border(sal_Int16 col, sal_Int32 r, sal_Int32 lW, sal_Int32 tW, sal_Int32 rW, sal_Int32 bW,
           sal_uInt16 lOutW, sal_uInt16 lInW, sal_uInt16 lDist, sal_uInt16 tOutW, sal_uInt16 tInW,
           sal_uInt16 tDist, sal_uInt16 rOutW, sal_uInt16 rInW, sal_uInt16 rDist, sal_uInt16 bOutW,
           sal_uInt16 bInW, sal_uInt16 bDist, sal_Int16 lSt, sal_Int16 tSt, sal_Int16 rSt,
           sal_Int16 bSt)
        : column(col)
        , row(r)
        , leftWidth(lW)
        , topWidth(tW)
        , rightWidth(rW)
        , bottomWidth(bW)
        , lOutWidth(lOutW)
        , lInWidth(lInW)
        , lDistance(lDist)
        , tOutWidth(tOutW)
        , tInWidth(tInW)
        , tDistance(tDist)
        , rOutWidth(rOutW)
        , rInWidth(rInW)
        , rDistance(rDist)
        , bOutWidth(bOutW)
        , bInWidth(bInW)
        , bDistance(bDist)
        , lStyle(static_cast<SvxBorderLineStyle>(lSt))
        , tStyle(static_cast<SvxBorderLineStyle>(tSt))
        , rStyle(static_cast<SvxBorderLineStyle>(rSt))
        , bStyle(static_cast<SvxBorderLineStyle>(bSt)){};
};
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest, testBordersOoo33)
{
    std::vector<Border> borders;
    borders.emplace_back(1, 1, 22, 22, 22, 22, 1, 1, 20, 1, 1, 20, 1, 1, 20, 1, 1, 20, 3, 3, 3, 3);
    borders.emplace_back(1, 3, 52, 52, 52, 52, 1, 1, 50, 1, 1, 50, 1, 1, 50, 1, 1, 50, 3, 3, 3, 3);
    borders.emplace_back(1, 5, 60, 60, 60, 60, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 3, 3,
                         3, 3);
    borders.emplace_back(1, 7, 150, 150, 150, 150, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50,
                         3, 3, 3, 3);
    borders.emplace_back(1, 9, 71, 71, 71, 71, 20, 1, 50, 20, 1, 50, 20, 1, 50, 20, 1, 50, 3, 3, 3,
                         3);
    borders.emplace_back(1, 11, 101, 101, 101, 101, 50, 1, 50, 50, 1, 50, 50, 1, 50, 50, 1, 50, 3,
                         3, 3, 3);
    borders.emplace_back(1, 13, 131, 131, 131, 131, 80, 1, 50, 80, 1, 50, 80, 1, 50, 80, 1, 50, 3,
                         3, 3, 3);
    borders.emplace_back(1, 15, 120, 120, 120, 120, 50, 20, 50, 50, 20, 50, 50, 20, 50, 50, 20, 50,
                         3, 3, 3, 3);
    borders.emplace_back(1, 17, 90, 90, 90, 90, 20, 50, 20, 20, 50, 20, 20, 50, 20, 20, 50, 20, 3,
                         3, 3, 3);
    borders.emplace_back(1, 19, 180, 180, 180, 180, 80, 50, 50, 80, 50, 50, 80, 50, 50, 80, 50, 50,
                         3, 3, 3, 3);
    borders.emplace_back(1, 21, 180, 180, 180, 180, 50, 80, 50, 50, 80, 50, 50, 80, 50, 50, 80, 50,
                         3, 3, 3, 3);
    borders.emplace_back(4, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0);
    borders.emplace_back(4, 3, 10, 10, 10, 10, 10, 0, 0, 10, 0, 0, 10, 0, 0, 10, 0, 0, 0, 0, 0, 0);
    borders.emplace_back(4, 5, 20, 20, 20, 20, 20, 0, 0, 20, 0, 0, 20, 0, 0, 20, 0, 0, 0, 0, 0, 0);
    borders.emplace_back(4, 7, 50, 50, 50, 50, 50, 0, 0, 50, 0, 0, 50, 0, 0, 50, 0, 0, 0, 0, 0, 0);
    borders.emplace_back(4, 9, 80, 80, 80, 80, 80, 0, 0, 80, 0, 0, 80, 0, 0, 80, 0, 0, 0, 0, 0, 0);
    borders.emplace_back(4, 11, 100, 100, 100, 100, 100, 0, 0, 100, 0, 0, 100, 0, 0, 100, 0, 0, 0,
                         0, 0, 0);

    createScDoc("ods/borders_ooo33.ods");

    ScDocument* pDoc = getScDoc();

    const editeng::SvxBorderLine* pLeft = nullptr;
    const editeng::SvxBorderLine* pTop = nullptr;
    const editeng::SvxBorderLine* pRight = nullptr;
    const editeng::SvxBorderLine* pBottom = nullptr;
    sal_Int16 temp = 0;
    for (sal_Int16 i = 0; i < 6; ++i)
    {
        for (sal_Int32 j = 0; j < 22; ++j)
        {
            pDoc->GetBorderLines(i, j, 0, &pLeft, &pTop, &pRight, &pBottom);
            if (pLeft != nullptr && pTop != nullptr && pRight != nullptr && pBottom != nullptr)
            {
                CPPUNIT_ASSERT_EQUAL(borders[temp].column, i);
                CPPUNIT_ASSERT_EQUAL(borders[temp].row, j);
                CPPUNIT_ASSERT_EQUAL(borders[temp].leftWidth, pLeft->GetWidth());
                CPPUNIT_ASSERT_EQUAL(borders[temp].topWidth, pTop->GetWidth());
                CPPUNIT_ASSERT_EQUAL(borders[temp].rightWidth, pRight->GetWidth());
                CPPUNIT_ASSERT_EQUAL(borders[temp].bottomWidth, pBottom->GetWidth());
                CPPUNIT_ASSERT_EQUAL(borders[temp].lOutWidth, pLeft->GetOutWidth());
                CPPUNIT_ASSERT_EQUAL(borders[temp].lInWidth, pLeft->GetInWidth());
                CPPUNIT_ASSERT_EQUAL(borders[temp].lDistance, pLeft->GetDistance());
                CPPUNIT_ASSERT_EQUAL(borders[temp].tOutWidth, pTop->GetOutWidth());
                CPPUNIT_ASSERT_EQUAL(borders[temp].tInWidth, pTop->GetInWidth());
                CPPUNIT_ASSERT_EQUAL(borders[temp].tDistance, pTop->GetDistance());
                CPPUNIT_ASSERT_EQUAL(borders[temp].rOutWidth, pRight->GetOutWidth());
                CPPUNIT_ASSERT_EQUAL(borders[temp].rInWidth, pRight->GetInWidth());
                CPPUNIT_ASSERT_EQUAL(borders[temp].rDistance, pRight->GetDistance());
                CPPUNIT_ASSERT_EQUAL(borders[temp].bOutWidth, pBottom->GetOutWidth());
                CPPUNIT_ASSERT_EQUAL(borders[temp].bInWidth, pBottom->GetInWidth());
                CPPUNIT_ASSERT_EQUAL(borders[temp].bDistance, pBottom->GetDistance());
                SvxBorderLineStyle tempStyle = pLeft->GetBorderLineStyle();
                CPPUNIT_ASSERT_EQUAL(borders[temp].lStyle, tempStyle);
                tempStyle = pTop->GetBorderLineStyle();
                CPPUNIT_ASSERT_EQUAL(borders[temp].tStyle, tempStyle);
                tempStyle = pRight->GetBorderLineStyle();
                CPPUNIT_ASSERT_EQUAL(borders[temp].rStyle, tempStyle);
                tempStyle = pBottom->GetBorderLineStyle();
                CPPUNIT_ASSERT_EQUAL(borders[temp].bStyle, tempStyle);
                ++temp;
            }
        }
    }
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest, testBugFixesODS)
{
    createScDoc("ods/bug-fixes.ods");

    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();
    ScDocument* pDoc = getScDoc();

    {
        // fdo#40967
        OUString aCSVPath = createFilePath(u"contentCSV/bugFix_Sheet2.csv");
        testFile(aCSVPath, *pDoc, 1);
    }

    {
        // fdo#40426
        ScDBData* pDBData
            = pDoc->GetDBCollection()->getNamedDBs().findByUpperName(u"DBRANGE1"_ustr);
        CPPUNIT_ASSERT(pDBData);
        CPPUNIT_ASSERT(pDBData->HasHeader());
        // no header
        pDBData = pDoc->GetDBCollection()->getNamedDBs().findByUpperName(u"DBRANGE2"_ustr);
        CPPUNIT_ASSERT(pDBData);
        CPPUNIT_ASSERT(!pDBData->HasHeader());
    }

    {
        // fdo#59240
        OUString aCSVPath = createFilePath(u"contentCSV/bugFix_Sheet4.csv");
        testFile(aCSVPath, *pDoc, 3);
    }
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest, testBugFixesXLS)
{
    createScDoc("xls/bug-fixes.xls");

    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest, testBugFixesXLSX)
{
    createScDoc("xlsx/bug-fixes.xlsx");

    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();
}

namespace
{
void checkMergedCells(ScDocument& rDoc, const ScAddress& rStartAddress,
                      const ScAddress& rExpectedEndAddress)
{
    SCCOL nActualEndCol = rStartAddress.Col();
    SCROW nActualEndRow = rStartAddress.Row();
    rDoc.ExtendMerge(rStartAddress.Col(), rStartAddress.Row(), nActualEndCol, nActualEndRow,
                     rStartAddress.Tab());
    OString sTab = OString::number(rStartAddress.Tab() + 1);
    OString msg = "Merged cells are not correctly imported on sheet" + sTab;
    OString msgCol = msg + "; end col";
    OString msgRow = msg + "; end row";
    CPPUNIT_ASSERT_EQUAL_MESSAGE(msgCol.pData->buffer, rExpectedEndAddress.Col(), nActualEndCol);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(msgRow.pData->buffer, rExpectedEndAddress.Row(), nActualEndRow);
}
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest, testMergedCellsODS)
{
    createScDoc("ods/merged.ods");
    ScDocument* pDoc = getScDoc();

    //check sheet1 content
    OUString aCSVPath = createFilePath(u"contentCSV/merged1.csv");
    testFile(aCSVPath, *pDoc, 0);

    //check sheet1 merged cells
    checkMergedCells(*pDoc, ScAddress(0, 0, 0), ScAddress(5, 11, 0));
    checkMergedCells(*pDoc, ScAddress(7, 2, 0), ScAddress(9, 12, 0));
    checkMergedCells(*pDoc, ScAddress(3, 15, 0), ScAddress(7, 23, 0));

    //check sheet2 content
    aCSVPath = createFilePath(u"contentCSV/merged2.csv");
    testFile(aCSVPath, *pDoc, 1);

    //check sheet2 merged cells
    checkMergedCells(*pDoc, ScAddress(4, 3, 1), ScAddress(6, 15, 1));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest, testRepeatedColumnsODS)
{
    createScDoc("ods/repeatedColumns.ods");
    ScDocument* pDoc = getScDoc();

    //text
    OUString aCSVPath = createFilePath(u"contentCSV/repeatedColumns1.csv");
    testFile(aCSVPath, *pDoc, 0);

    //numbers
    aCSVPath = createFilePath(u"contentCSV/repeatedColumns2.csv");
    testFile(aCSVPath, *pDoc, 1);
}

namespace
{
//for cleaner passing of parameters
struct ValDataTestParams
{
    ScValidationMode eValMode;
    ScConditionMode eCondOp;
    OUString aStrVal1;
    OUString aStrVal2;
    ScDocument& rDocument;
    ScAddress aPosition;
    OUString aErrorTitle;
    OUString aErrorMessage;
    ScValidErrorStyle eErrorStyle;
    sal_uInt32 nExpectedIndex;

    ValDataTestParams(ScValidationMode eMode, ScConditionMode eOp, const OUString& aExpr1,
                      const OUString& aExpr2, ScDocument& rDoc, const ScAddress& aPos,
                      const OUString& aETitle, const OUString& aEMsg, ScValidErrorStyle eEStyle,
                      sal_uInt32 nIndex)
        : eValMode(eMode)
        , eCondOp(eOp)
        , aStrVal1(aExpr1)
        , aStrVal2(aExpr2)
        , rDocument(rDoc)
        , aPosition(aPos)
        , aErrorTitle(aETitle)
        , aErrorMessage(aEMsg)
        , eErrorStyle(eEStyle)
        , nExpectedIndex(nIndex){};
};

void checkValiditationEntries(const ValDataTestParams& rVDTParams)
{
    ScDocument& rDoc = rVDTParams.rDocument;

    //create expected data validation entry
    ScValidationData aValData(rVDTParams.eValMode, rVDTParams.eCondOp, rVDTParams.aStrVal1,
                              rVDTParams.aStrVal2, rDoc, rVDTParams.aPosition, OUString(),
                              OUString(), rDoc.GetStorageGrammar(), rDoc.GetStorageGrammar());
    aValData.SetIgnoreBlank(true);
    aValData.SetListType(1);
    aValData.ResetInput();
    aValData.SetError(rVDTParams.aErrorTitle, rVDTParams.aErrorMessage, rVDTParams.eErrorStyle);
    aValData.SetSrcString(OUString());

    //get actual data validation entry from document
    const ScValidationData* pValDataTest = rDoc.GetValidationEntry(rVDTParams.nExpectedIndex);

    sal_Int32 nCol(static_cast<sal_Int32>(rVDTParams.aPosition.Col()));
    sal_Int32 nRow(static_cast<sal_Int32>(rVDTParams.aPosition.Row()));
    sal_Int32 nTab(static_cast<sal_Int32>(rVDTParams.aPosition.Tab()));
    OString aMsgPrefix = "Data Validation Entry with base-cell-address: (" + OString::number(nCol)
                         + "," + OString::number(nRow) + "," + OString::number(nTab) + ") ";

    OString aMsg = aMsgPrefix + "did not get imported at all.";
    CPPUNIT_ASSERT_MESSAGE(aMsg.getStr(), pValDataTest);

    //check if expected and actual data validation entries are equal
    if (!aValData.EqualEntries(*pValDataTest))
    {
        aMsg = aMsgPrefix + "got imported incorrectly.";
        CPPUNIT_FAIL(aMsg.getStr());
    }
}

void checkCellValidity(const ScAddress& rValBaseAddr, const ScRange& rRange, const ScDocument& rDoc)
{
    SCCOL nBCol(rValBaseAddr.Col());
    SCROW nBRow(rValBaseAddr.Row());
    SCTAB nTab(static_cast<sal_Int32>(rValBaseAddr.Tab()));
    //get from the document the data validation entry we are checking against
    const SfxUInt32Item* pItem = rDoc.GetAttr(nBCol, nBRow, nTab, ATTR_VALIDDATA);
    const ScValidationData* pValData = rDoc.GetValidationEntry(pItem->GetValue());
    CPPUNIT_ASSERT(pValData);

    //check that each cell in the expected range is associated with the data validation entry
    for (SCCOL i = rRange.aStart.Col(); i <= rRange.aEnd.Col(); ++i)
    {
        for (SCROW j = rRange.aStart.Row(); j <= rRange.aEnd.Row(); ++j)
        {
            const SfxUInt32Item* pItemTest = rDoc.GetAttr(i, j, nTab, ATTR_VALIDDATA);
            const ScValidationData* pValDataTest = rDoc.GetValidationEntry(pItemTest->GetValue());
            //prevent string operations for occurring unnecessarily
            if (!(pValDataTest && pValData->GetKey() == pValDataTest->GetKey()))
            {
                sal_Int32 nCol = static_cast<sal_Int32>(i);
                sal_Int32 nRow = static_cast<sal_Int32>(j);
                sal_Int32 nTab32 = static_cast<sal_Int32>(nTab);
                OString sMsg = "\nData validation entry base-cell-address: ("
                               + OString::number(static_cast<sal_Int32>(nBCol)) + ","
                               + OString::number(static_cast<sal_Int32>(nBRow)) + ","
                               + OString::number(nTab32)
                               + ")\n"
                                 "Cell: ("
                               + OString::number(nCol) + "," + OString::number(nRow) + ","
                               + OString::number(nTab32) + ")";
                sal_uInt32 expectedKey(pValData->GetKey());
                sal_uInt32 actualKey(0);
                if (pValDataTest)
                    actualKey = pValDataTest->GetKey();
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sMsg.getStr(), expectedKey, actualKey);
            }
        }
    }
}
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest, testDataValidityODS)
{
    createScDoc("ods/dataValidity.ods");
    ScDocument* pDoc = getScDoc();

    ScAddress aValBaseAddr1(2, 6, 0); //sheet1
    ScAddress aValBaseAddr2(2, 3, 1); //sheet2
    ScAddress aValBaseAddr3(2, 2, 2); //sheet3

    //sheet1's expected Data Validation Entry values
    ValDataTestParams aVDTParams1(SC_VALID_DECIMAL, ScConditionMode::Greater, u"3.14"_ustr,
                                  OUString(), *pDoc, aValBaseAddr1, u"Too small"_ustr,
                                  u"The number you are trying to enter is not greater than 3.14! "
                                  "Are you sure you want to enter it anyway?"_ustr,
                                  SC_VALERR_WARNING, 1);
    //sheet2's expected Data Validation Entry values
    ValDataTestParams aVDTParams2(
        SC_VALID_WHOLE, ScConditionMode::Between, u"1"_ustr, u"10"_ustr, *pDoc, aValBaseAddr2,
        u"Error sheet 2"_ustr, u"Must be a whole number between 1 and 10."_ustr, SC_VALERR_STOP, 2);
    //sheet3's expected Data Validation Entry values
    ValDataTestParams aVDTParams3(SC_VALID_CUSTOM, ScConditionMode::Direct, u"ISTEXT(C3)"_ustr,
                                  OUString(), *pDoc, aValBaseAddr3, u"Error sheet 3"_ustr,
                                  u"Must not be a numerical value."_ustr, SC_VALERR_STOP, 3);
    //check each sheet's Data Validation Entries
    checkValiditationEntries(aVDTParams1);
    checkValiditationEntries(aVDTParams2);
    checkValiditationEntries(aVDTParams3);

    //expected ranges to be associated with data validity
    ScRange aRange1(2, 2, 0, 2, 6, 0); //sheet1
    ScRange aRange2(2, 3, 1, 6, 7, 1); //sheet2
    ScRange aRange3(2, 2, 2, 2, 6, 2); //sheet3

    //check each sheet's cells for data validity
    checkCellValidity(aValBaseAddr1, aRange1, *pDoc);
    checkCellValidity(aValBaseAddr2, aRange2, *pDoc);
    checkCellValidity(aValBaseAddr3, aRange3, *pDoc);

    //check each sheet's content
    OUString aCSVPath = createFilePath(u"contentCSV/dataValidity1.csv");
    testFile(aCSVPath, *pDoc, 0);

    aCSVPath = createFilePath(u"contentCSV/dataValidity2.csv");
    testFile(aCSVPath, *pDoc, 1);

    aCSVPath = createFilePath(u"contentCSV/dataValidity3.csv");
    testFile(aCSVPath, *pDoc, 2);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest, testDataValidityXLSX)
{
    createScDoc("xlsx/dataValidity.xlsx");
    ScDocument* pDoc = getScDoc();

    ScAddress aValBaseAddr1(2, 6, 0); //sheet1
    ScAddress aValBaseAddr2(2, 3, 1); //sheet2
    ScAddress aValBaseAddr3(2, 2, 2); //sheet3

    //expected ranges to be associated with data validity
    ScRange aRange1(2, 2, 0, 2, 6, 0); //sheet1
    ScRange aRange2(2, 3, 1, 6, 7, 1); //sheet2
    ScRange aRange3(2, 2, 2, 2, 6, 2); //sheet3

    //check each sheet's cells for data validity
    checkCellValidity(aValBaseAddr1, aRange1, *pDoc);
    checkCellValidity(aValBaseAddr2, aRange2, *pDoc);
    checkCellValidity(aValBaseAddr3, aRange3, *pDoc);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest, testDataTableMortgageXLS)
{
    createScDoc("xls/data-table/mortgage.xls");

    ScDocument* pDoc = getScDoc();

    // One-variable table

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", u"=PMT(B3/12,B4,-B5)"_ustr,
                                 pDoc->GetFormula(3, 1, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", u"=MULTIPLE.OPERATIONS(D$2,$B$3,$C3)"_ustr,
                                 pDoc->GetFormula(3, 2, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", u"=MULTIPLE.OPERATIONS(D$2,$B$3,$C4)"_ustr,
                                 pDoc->GetFormula(3, 3, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", u"=MULTIPLE.OPERATIONS(D$2,$B$3,$C5)"_ustr,
                                 pDoc->GetFormula(3, 4, 0));

    // Two-variable table

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", u"=PMT(B9/12,B10,-B11)"_ustr,
                                 pDoc->GetFormula(2, 7, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!",
                                 u"=MULTIPLE.OPERATIONS($C$8,$B$9,$C9,$B$10,D$8)"_ustr,
                                 pDoc->GetFormula(3, 8, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!",
                                 u"=MULTIPLE.OPERATIONS($C$8,$B$9,$C10,$B$10,D$8)"_ustr,
                                 pDoc->GetFormula(3, 9, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!",
                                 u"=MULTIPLE.OPERATIONS($C$8,$B$9,$C11,$B$10,D$8)"_ustr,
                                 pDoc->GetFormula(3, 10, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!",
                                 u"=MULTIPLE.OPERATIONS($C$8,$B$9,$C9,$B$10,E$8)"_ustr,
                                 pDoc->GetFormula(4, 8, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!",
                                 u"=MULTIPLE.OPERATIONS($C$8,$B$9,$C10,$B$10,E$8)"_ustr,
                                 pDoc->GetFormula(4, 9, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!",
                                 u"=MULTIPLE.OPERATIONS($C$8,$B$9,$C11,$B$10,E$8)"_ustr,
                                 pDoc->GetFormula(4, 10, 0));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest, testDataTableOneVarXLSX)
{
    createScDoc("xlsx/data-table/one-variable.xlsx");

    ScDocument* pDoc = getScDoc();

    // Right now, we have a bug that prevents Calc from re-calculating these
    // cells automatically upon file load. We can remove this call if/when we
    // fix the aforementioned bug.
    pDoc->CalcAll();

    // B5:B11 should have multiple operations formula cells.  Just check the
    // top and bottom cells.

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", u"=MULTIPLE.OPERATIONS(B$4,$A$2,$A5)"_ustr,
                                 pDoc->GetFormula(1, 4, 0));

    CPPUNIT_ASSERT_EQUAL(2.0, pDoc->GetValue(ScAddress(1, 4, 0)));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", u"=MULTIPLE.OPERATIONS(B$4,$A$2,$A11)"_ustr,
                                 pDoc->GetFormula(1, 10, 0));

    CPPUNIT_ASSERT_EQUAL(14.0, pDoc->GetValue(ScAddress(1, 10, 0)));

    // Likewise, E5:I5 should have multiple operations formula cells.  Just
    // check the left- and right-most cells.

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", u"=MULTIPLE.OPERATIONS($D5,$B$2,E$4)"_ustr,
                                 pDoc->GetFormula(4, 4, 0));

    CPPUNIT_ASSERT_EQUAL(10.0, pDoc->GetValue(ScAddress(4, 4, 0)));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", u"=MULTIPLE.OPERATIONS($D5,$B$2,I$4)"_ustr,
                                 pDoc->GetFormula(8, 4, 0));

    CPPUNIT_ASSERT_EQUAL(50.0, pDoc->GetValue(ScAddress(8, 4, 0)));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest, testDataTableMultiTableXLSX)
{
    createScDoc("xlsx/data-table/multi-table.xlsx");

    ScDocument* pDoc = getScDoc();

    // Right now, we have a bug that prevents Calc from re-calculating these
    // cells automatically upon file load. We can remove this call if/when we
    // fix the aforementioned bug.
    pDoc->CalcAll();

    // B4:M15 should have multiple operations formula cells.  We'll just check
    // the top-left and bottom-right ones.

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!",
                                 u"=MULTIPLE.OPERATIONS($A$3,$E$1,$A4,$D$1,B$3)"_ustr,
                                 pDoc->GetFormula(1, 3, 0));

    CPPUNIT_ASSERT_EQUAL(1.0, pDoc->GetValue(ScAddress(1, 3, 0)));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!",
                                 u"=MULTIPLE.OPERATIONS($A$3,$E$1,$A15,$D$1,M$3)"_ustr,
                                 pDoc->GetFormula(12, 14, 0));

    CPPUNIT_ASSERT_EQUAL(144.0, pDoc->GetValue(ScAddress(12, 14, 0)));
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest, testBrokenQuotesCSV)
{
    createScDoc("csv/fdo48621_broken_quotes.csv");

    ScDocument* pDoc = getScDoc();

    OUString aCSVPath = createFilePath(u"contentCSV/fdo48621_broken_quotes_exported.csv");
    // fdo#48621
    testFile(aCSVPath, *pDoc, 0, StringType::PureString);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest, testCellValueXLSX)
{
    //change this test file only in excel and not in calc
    createScDoc("xlsx/cell-value.xlsx");
    ScDocument* pDoc = getScDoc();

    OUString aCSVPath = createFilePath(u"contentCSV/cell-value.csv");
    testFile(aCSVPath, *pDoc, 0);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest, testRowIndex1BasedXLSX)
{
    /**
     * Test importing of xlsx document that previously had its row index off
     * by one. (fdo#76032)
     */
    createScDoc("xlsx/row-index-1-based.xlsx");
    ScDocument* pDoc = getScDoc();

    // A1
    OUString aStr = pDoc->GetString(ScAddress(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"Action Plan.Name"_ustr, aStr);

    // B1
    aStr = pDoc->GetString(ScAddress(1, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"Action Plan.Description"_ustr, aStr);

    // A2
    aStr = pDoc->GetString(ScAddress(0, 1, 0));
    CPPUNIT_ASSERT_EQUAL(u"Jerry"_ustr, aStr);

    // B2 - multi-line text.
    const EditTextObject* pText = pDoc->GetEditText(ScAddress(1, 1, 0));
    CPPUNIT_ASSERT(pText);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3), pText->GetParagraphCount());
    aStr = pText->GetText(0);
    CPPUNIT_ASSERT_EQUAL(u"This is a longer Text."_ustr, aStr);
    aStr = pText->GetText(1);
    CPPUNIT_ASSERT_EQUAL(u"Second line."_ustr, aStr);
    aStr = pText->GetText(2);
    CPPUNIT_ASSERT_EQUAL(u"Third line."_ustr, aStr);
}

ScFiltersTest::ScFiltersTest()
    : ScModelTestBase(u"sc/qa/unit/data"_ustr)
{
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
