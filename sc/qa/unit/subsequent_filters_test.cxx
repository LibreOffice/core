/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <osl/file.hxx>
#include <config_fonts.h>

#include <svl/numformat.hxx>
#include <svl/zformat.hxx>

#include <svx/svdpage.hxx>
#include <svx/svdocapt.hxx>
#include <svx/svdoole2.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/borderline.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/flditem.hxx>
#include <editeng/justifyitem.hxx>
#include <editeng/lineitem.hxx>
#include <editeng/colritem.hxx>
#include <dbdata.hxx>
#include <dbdocfun.hxx>
#include <inputopt.hxx>
#include <globalnames.hxx>
#include <validat.hxx>
#include <userdat.hxx>
#include <scmod.hxx>
#include <stlsheet.hxx>
#include <docfunc.hxx>
#include <markdata.hxx>
#include <colorscale.hxx>
#include <patattr.hxx>
#include <scitems.hxx>
#include <editutil.hxx>
#include <attrib.hxx>
#include <fillinfo.hxx>
#include <scopetools.hxx>
#include <stlpool.hxx>
#include <detfunc.hxx>
#include <cellmergeoption.hxx>
#include <postit.hxx>
#include <sortparam.hxx>
#include <undomanager.hxx>
#include <tabprotection.hxx>

#include <orcusfilters.hxx>
#include <filter.hxx>

#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>

#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/text/textfield/Type.hpp>

#include "helper/qahelper.hxx"
#include "helper/shared_test_impl.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

/* Implementation of Filters test */

class ScFiltersTest : public ScModelTestBase
{
public:
    ScFiltersTest();

    //ods, xls, xlsx filter tests
    void testContentODS();
    void testContentXLS();
    void testContentXLSX();
    void testContentXLSXStrict(); // strict OOXML
    void testContentLotus123();
    void testContentDIF();
    void testContentXLSB();
    void testContentXLS_XML();
    void testContentGnumeric();
    void testCondFormatOperatorsSameRangeXLSX();
    void testTdf150452();
    void testTdf119292();
    void testTdf48731();
    void testCondFormatFormulaIsXLSX();
    void testCondFormatBeginsAndEndsWithXLSX();
    void testExtCondFormatXLSX();
    void testUpdateCircleInMergedCellODS();
    void testDeleteCircleInMergedCellODS();
    void testBooleanFormatXLSX();
    void testTdf143809();
    void testTdf76310();
    void testBasicCellContentODS();
    void testRangeNameXLS();
    void testRangeNameLocalXLS();
    void testRangeNameXLSX();
    void testRangeNameODS();
    void testHyperlinksXLSX();
    void testHardRecalcODS();
    void testFunctionsODS();
    void testFunctionsExcel2010();
    void testCeilingFloorXLSX();
    void testCachedFormulaResultsODS();
    void testCachedMatrixFormulaResultsODS();
    void testFormulaDepAcrossSheetsODS();
    void testFormulaDepDeleteContentsODS();
    void testDatabaseRangesODS();
    void testDatabaseRangesXLS();
    void testDatabaseRangesXLSX();
    void testFormatsODS();
    // void testFormatsXLS();
    // void testFormatsXLSX();
    void testMatrixODS();
    void testMatrixXLS();
    void testDoubleThinBorder();
    void testBorderODS();
    void testBordersOoo33();
    void testBugFixesODS();
    void testBugFixesXLS();
    void testBugFixesXLSX();
    void testBrokenQuotesCSV();
    void testMergedCellsODS();
    void testRepeatedColumnsODS();
    void testDataValidityODS();
    void testDataValidityXLSX();
    void testDataTableMortgageXLS();
    void testDataTableOneVarXLSX();
    void testDataTableMultiTableXLSX();

    void testDataBarODS();
    void testDataBarXLSX();
    void testColorScaleODS();
    void testColorScaleXLSX();
    void testNewCondFormatODS();
    void testNewCondFormatXLSX();
    void testCondFormatThemeColorXLSX();
    void testCondFormatImportCellIs();
    void testCondFormatThemeColor2XLSX(); // negative bar color and axis color
    void testCondFormatThemeColor3XLSX(); // theme index 2 and 3 are switched
    void testComplexIconSetsXLSX();
    void testTdf101104();
    void testTdf64401();
    void testCondFormatParentXLSX();
    void testColorScaleNumWithRefXLSX();

    void testOrcusODSStyleInterface();

    void testLiteralInFormulaXLS();

    //change this test file only in excel and not in calc
    void testCellValueXLSX();

    /**
     * Test importing of xlsx document that previously had its row index off
     * by one. (fdo#76032)
     */
    void testRowIndex1BasedXLSX();

    //misc tests unrelated to the import filters
    void testPasswordNew();
    void testPasswordOld();
    void testPasswordWrongSHA();

    //test shape import
    void testControlImport();
    void testActiveXOptionButtonGroup();
    void testChartImportODS();
#if HAVE_MORE_FONTS
    void testChartImportXLS();
#endif

    void testNumberFormatHTML();
    void testNumberFormatCSV();

    void testCellAnchoredShapesODS();
    void testCellAnchoredHiddenShapesXLSX();

    void testFormulaDependency();
    void testTdf151046();

    void testRowHeightODS();
    void testRichTextContentODS();

    void testImportCrashes();
    void testTdf129681();
    void testTdf149484();
    void testEscapedUnicodeXLSX();
    void testTdf144758_DBDataDefaultOrientation();
    void testSharedFormulaXLS();
    void testSharedFormulaXLSX();
    void testSharedFormulaRefUpdateXLSX();
    void testSheetNamesXLSX();
    void testTdf150599();
    void testCommentSize();
    void testEnhancedProtectionXLS();
    void testEnhancedProtectionXLSX();
    void testSortWithSharedFormulasODS();
    void testSortWithSheetExternalReferencesODS();
    void testSortWithFormattingXLS();
    void testForcepoint107();

    CPPUNIT_TEST_SUITE(ScFiltersTest);
    CPPUNIT_TEST(testContentODS);
    CPPUNIT_TEST(testContentXLS);
    CPPUNIT_TEST(testContentXLSX);
    CPPUNIT_TEST(testContentXLSXStrict);
    CPPUNIT_TEST(testContentLotus123);
    CPPUNIT_TEST(testContentDIF);
    CPPUNIT_TEST(testContentXLSB);
    CPPUNIT_TEST(testContentXLS_XML);
    CPPUNIT_TEST(testContentGnumeric);
    CPPUNIT_TEST(testCondFormatOperatorsSameRangeXLSX);
    CPPUNIT_TEST(testTdf150452);
    CPPUNIT_TEST(testTdf119292);
    CPPUNIT_TEST(testTdf48731);
    CPPUNIT_TEST(testCondFormatFormulaIsXLSX);
    CPPUNIT_TEST(testCondFormatBeginsAndEndsWithXLSX);
    CPPUNIT_TEST(testExtCondFormatXLSX);
    CPPUNIT_TEST(testUpdateCircleInMergedCellODS);
    CPPUNIT_TEST(testDeleteCircleInMergedCellODS);
    CPPUNIT_TEST(testBooleanFormatXLSX);
    CPPUNIT_TEST(testTdf143809);
    CPPUNIT_TEST(testTdf76310);
    CPPUNIT_TEST(testBasicCellContentODS);
    CPPUNIT_TEST(testRangeNameXLS);
    CPPUNIT_TEST(testRangeNameLocalXLS);
    CPPUNIT_TEST(testRangeNameXLSX);
    CPPUNIT_TEST(testRangeNameODS);
    CPPUNIT_TEST(testHyperlinksXLSX);
    CPPUNIT_TEST(testHardRecalcODS);
    CPPUNIT_TEST(testFunctionsODS);
    CPPUNIT_TEST(testFunctionsExcel2010);
    CPPUNIT_TEST(testCeilingFloorXLSX);
    CPPUNIT_TEST(testCachedFormulaResultsODS);
    CPPUNIT_TEST(testFormulaDepAcrossSheetsODS);
    CPPUNIT_TEST(testFormulaDepDeleteContentsODS);
    CPPUNIT_TEST(testCachedMatrixFormulaResultsODS);
    CPPUNIT_TEST(testDatabaseRangesODS);
    CPPUNIT_TEST(testDatabaseRangesXLS);
    CPPUNIT_TEST(testDatabaseRangesXLSX);
    CPPUNIT_TEST(testFormatsODS);
    //  CPPUNIT_TEST(testFormatsXLS); TODO: Fix this
    //  CPPUNIT_TEST(testFormatsXLSX); TODO: Fix this
    CPPUNIT_TEST(testMatrixODS);
    CPPUNIT_TEST(testMatrixXLS);
    CPPUNIT_TEST(testDoubleThinBorder);
    CPPUNIT_TEST(testBorderODS);
    CPPUNIT_TEST(testBordersOoo33);
    CPPUNIT_TEST(testBugFixesODS);
    CPPUNIT_TEST(testBugFixesXLS);
    CPPUNIT_TEST(testBugFixesXLSX);
    CPPUNIT_TEST(testMergedCellsODS);
    CPPUNIT_TEST(testRepeatedColumnsODS);
    CPPUNIT_TEST(testDataValidityODS);
    CPPUNIT_TEST(testDataValidityXLSX);
    CPPUNIT_TEST(testDataTableMortgageXLS);
    CPPUNIT_TEST(testDataTableOneVarXLSX);
    CPPUNIT_TEST(testDataTableMultiTableXLSX);
    CPPUNIT_TEST(testBrokenQuotesCSV);
    CPPUNIT_TEST(testCellValueXLSX);
    CPPUNIT_TEST(testRowIndex1BasedXLSX);
    CPPUNIT_TEST(testControlImport);
    CPPUNIT_TEST(testActiveXOptionButtonGroup);
    CPPUNIT_TEST(testChartImportODS);
#if HAVE_MORE_FONTS
    CPPUNIT_TEST(testChartImportXLS);
#endif

    CPPUNIT_TEST(testDataBarODS);
    CPPUNIT_TEST(testDataBarXLSX);
    CPPUNIT_TEST(testColorScaleODS);
    CPPUNIT_TEST(testColorScaleXLSX);
    CPPUNIT_TEST(testNewCondFormatODS);
    CPPUNIT_TEST(testNewCondFormatXLSX);
    CPPUNIT_TEST(testCondFormatThemeColorXLSX);
    CPPUNIT_TEST(testCondFormatImportCellIs);
    CPPUNIT_TEST(testCondFormatThemeColor2XLSX);
    CPPUNIT_TEST(testCondFormatThemeColor3XLSX);
    CPPUNIT_TEST(testComplexIconSetsXLSX);
    CPPUNIT_TEST(testTdf101104);
    CPPUNIT_TEST(testTdf64401);
    CPPUNIT_TEST(testCondFormatParentXLSX);
    CPPUNIT_TEST(testColorScaleNumWithRefXLSX);

    CPPUNIT_TEST(testOrcusODSStyleInterface);

    CPPUNIT_TEST(testLiteralInFormulaXLS);

    CPPUNIT_TEST(testNumberFormatHTML);
    CPPUNIT_TEST(testNumberFormatCSV);

    CPPUNIT_TEST(testCellAnchoredShapesODS);
    CPPUNIT_TEST(testCellAnchoredHiddenShapesXLSX);

    CPPUNIT_TEST(testRowHeightODS);
    CPPUNIT_TEST(testFormulaDependency);
    CPPUNIT_TEST(testTdf151046);
    CPPUNIT_TEST(testRichTextContentODS);

    //disable testPassword on MacOSX due to problems with libsqlite3
    //also crashes on DragonFly due to problems with nss/nspr headers
    CPPUNIT_TEST(testPasswordWrongSHA);
    CPPUNIT_TEST(testPasswordOld);
    CPPUNIT_TEST(testPasswordNew);

    CPPUNIT_TEST(testImportCrashes);
    CPPUNIT_TEST(testTdf129681);
    CPPUNIT_TEST(testTdf149484);
    CPPUNIT_TEST(testEscapedUnicodeXLSX);
    CPPUNIT_TEST(testTdf144758_DBDataDefaultOrientation);
    CPPUNIT_TEST(testSharedFormulaXLS);
    CPPUNIT_TEST(testSharedFormulaXLSX);
    CPPUNIT_TEST(testSharedFormulaRefUpdateXLSX);
    CPPUNIT_TEST(testSheetNamesXLSX);
    CPPUNIT_TEST(testTdf150599);
    CPPUNIT_TEST(testCommentSize);
    CPPUNIT_TEST(testEnhancedProtectionXLS);
    CPPUNIT_TEST(testEnhancedProtectionXLSX);
    CPPUNIT_TEST(testSortWithSharedFormulasODS);
    CPPUNIT_TEST(testSortWithSheetExternalReferencesODS);
    CPPUNIT_TEST(testSortWithFormattingXLS);
    CPPUNIT_TEST(testForcepoint107);

    CPPUNIT_TEST_SUITE_END();

private:
    void testImportCrash(const char* rFileName);
};

namespace
{
void testRangeNameImpl(const ScDocument& rDoc)
{
    //check one range data per sheet and one global more detailed
    //add some more checks here
    ScRangeData* pRangeData = rDoc.GetRangeName()->findByUpperName(OUString("GLOBAL1"));
    CPPUNIT_ASSERT_MESSAGE("range name Global1 not found", pRangeData);
    double aValue = rDoc.GetValue(1, 0, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("range name Global1 should reference Sheet1.A1", 1.0, aValue);
    pRangeData = rDoc.GetRangeName(0)->findByUpperName(OUString("LOCAL1"));
    CPPUNIT_ASSERT_MESSAGE("range name Sheet1.Local1 not found", pRangeData);
    aValue = rDoc.GetValue(1, 2, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("range name Sheet1.Local1 should reference Sheet1.A3", 3.0,
                                 aValue);
    pRangeData = rDoc.GetRangeName(1)->findByUpperName(OUString("LOCAL2"));
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
    CPPUNIT_ASSERT_EQUAL_MESSAGE("string imported not correctly", OUString("String1"), aString);
    aString = rDoc.GetString(1, 1, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("string not imported correctly", OUString("String2"), aString);

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
        CPPUNIT_ASSERT_EQUAL_MESSAGE("note text not imported correctly", OUString("Test"),
                                     pNote->GetText());
    }

    //add additional checks here
}
}

void ScFiltersTest::testContentODS()
{
    createScDoc("ods/universal-content.ods");
    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();

    testContentImpl(*getScDoc(), true);
}

void ScFiltersTest::testContentXLS()
{
    createScDoc("xls/universal-content.xls");
    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();

    testContentImpl(*getScDoc(), true);
}

void ScFiltersTest::testContentXLSX()
{
    createScDoc("xlsx/universal-content.xlsx");
    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();

    testContentImpl(*getScDoc(), true);
}

void ScFiltersTest::testContentXLSXStrict()
{
    createScDoc("xlsx/universal-content-strict.xlsx");
    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();

    testContentImpl(*getScDoc(), true);
}

void ScFiltersTest::testContentLotus123()
{
    createScDoc("123/universal-content.123");
    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();

    testContentImpl(*getScDoc(), false);
}

void ScFiltersTest::testContentDIF()
{
    createScDoc("dif/universal-content.dif");
    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();

    testContentImpl(*getScDoc(), false);
}

void ScFiltersTest::testContentXLSB()
{
    createScDoc("xlsb/universal-content.xlsb");
    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();

    testContentImpl(*getScDoc(), true);
}

void ScFiltersTest::testContentXLS_XML()
{
    createScDoc("xml/universal-content.xml");
    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();

    testContentImpl(*getScDoc(), false);
}

void ScFiltersTest::testContentGnumeric()
{
    createScDoc("gnumeric/universal-content.gnumeric");
    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();

    testContentImpl(*getScDoc(), false);
}

void ScFiltersTest::testCondFormatOperatorsSameRangeXLSX()
{
    createScDoc("xlsx/tdf139928.xlsx");

    ScDocument* pDoc = getScDoc();

    ScConditionalFormat* pFormat = pDoc->GetCondFormat(0, 0, 0);
    CPPUNIT_ASSERT(pFormat);

    const ScFormatEntry* pEntry = pFormat->GetEntry(0);
    CPPUNIT_ASSERT(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScFormatEntry::Type::ExtCondition, pEntry->GetType());

    const ScCondFormatEntry* pCondition = static_cast<const ScCondFormatEntry*>(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScConditionMode::ContainsText, pCondition->GetOperation());

    pEntry = pFormat->GetEntry(1);
    CPPUNIT_ASSERT(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScFormatEntry::Type::ExtCondition, pEntry->GetType());

    pCondition = static_cast<const ScCondFormatEntry*>(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScConditionMode::BeginsWith, pCondition->GetOperation());

    pEntry = pFormat->GetEntry(2);
    CPPUNIT_ASSERT(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScFormatEntry::Type::ExtCondition, pEntry->GetType());

    pCondition = static_cast<const ScCondFormatEntry*>(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScConditionMode::EndsWith, pCondition->GetOperation());
}

void ScFiltersTest::testTdf119292()
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

void ScFiltersTest::testTdf150452()
{
    // Without the fix in place, this test would have crashed
    createScDoc("csv/tdf150452.csv");

    ScDocument* pDoc = getScDoc();

    CPPUNIT_ASSERT_EQUAL(OUString("1-GDUSF"), pDoc->GetString(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("1-GE41L"), pDoc->GetString(0, 3998, 0));
}

void ScFiltersTest::testTdf48731()
{
    createScDoc("csv/tdf48731.csv");

    ScDocument* pDoc = getScDoc();

    CPPUNIT_ASSERT_EQUAL(OUString("'"), pDoc->GetString(1, 1, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("''"), pDoc->GetString(1, 2, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("'''"), pDoc->GetString(1, 3, 0));

    // Without the fix in place, this test would have failed with
    // - Expected: '3
    // - Actual  : 3
    CPPUNIT_ASSERT_EQUAL(OUString("'3"), pDoc->GetString(1, 4, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("'word"), pDoc->GetString(1, 5, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("'mword"), pDoc->GetString(1, 6, 0));

    CPPUNIT_ASSERT_EQUAL(OUString("'"), pDoc->GetString(1, 9, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("''"), pDoc->GetString(1, 10, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("'''"), pDoc->GetString(1, 11, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("'3"), pDoc->GetString(1, 12, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("'word"), pDoc->GetString(1, 13, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("'mword"), pDoc->GetString(1, 14, 0));
}

void ScFiltersTest::testCondFormatFormulaIsXLSX()
{
    createScDoc("xlsx/tdf113013.xlsx");

    ScDocument* pDoc = getScDoc();

    // "Formula is" condition
    ScConditionalFormat* pFormatB1 = pDoc->GetCondFormat(1, 0, 0);
    CPPUNIT_ASSERT(pFormatB1);
    ScConditionalFormat* pFormatA2 = pDoc->GetCondFormat(0, 1, 0);
    CPPUNIT_ASSERT(pFormatA2);

    ScRefCellValue aCellB1(*pDoc, ScAddress(1, 0, 0));
    OUString aCellStyleB1 = pFormatB1->GetCellStyle(aCellB1, ScAddress(1, 0, 0));
    CPPUNIT_ASSERT(!aCellStyleB1.isEmpty());

    ScRefCellValue aCellA2(*pDoc, ScAddress(0, 1, 0));
    OUString aCellStyleA2 = pFormatA2->GetCellStyle(aCellA2, ScAddress(0, 1, 0));
    CPPUNIT_ASSERT(!aCellStyleA2.isEmpty());
}

void ScFiltersTest::testCondFormatBeginsAndEndsWithXLSX()
{
    createScDoc("xlsx/tdf120749.xlsx");

    ScDocument* pDoc = getScDoc();

    // begins with and ends with conditions
    ScConditionalFormat* pFormatA1 = pDoc->GetCondFormat(0, 0, 0);
    CPPUNIT_ASSERT(pFormatA1);
    ScConditionalFormat* pFormatA2 = pDoc->GetCondFormat(0, 1, 0);
    CPPUNIT_ASSERT(pFormatA2);
    ScConditionalFormat* pFormatA3 = pDoc->GetCondFormat(0, 2, 0);
    CPPUNIT_ASSERT(pFormatA3);
    ScConditionalFormat* pFormatA4 = pDoc->GetCondFormat(0, 3, 0);
    CPPUNIT_ASSERT(pFormatA4);

    ScRefCellValue aCellA1(*pDoc, ScAddress(0, 0, 0));
    OUString aCellStyleA1 = pFormatA1->GetCellStyle(aCellA1, ScAddress(0, 0, 0));
    CPPUNIT_ASSERT(!aCellStyleA1.isEmpty());

    ScRefCellValue aCellA2(*pDoc, ScAddress(0, 1, 0));
    OUString aCellStyleA2 = pFormatA2->GetCellStyle(aCellA2, ScAddress(0, 1, 0));
    CPPUNIT_ASSERT(!aCellStyleA2.isEmpty());

    ScRefCellValue aCellA3(*pDoc, ScAddress(0, 2, 0));
    OUString aCellStyleA3 = pFormatA3->GetCellStyle(aCellA3, ScAddress(0, 2, 0));
    CPPUNIT_ASSERT(!aCellStyleA3.isEmpty());

    ScRefCellValue aCellA4(*pDoc, ScAddress(0, 3, 0));
    OUString aCellStyleA4 = pFormatA4->GetCellStyle(aCellA4, ScAddress(0, 3, 0));
    CPPUNIT_ASSERT(!aCellStyleA4.isEmpty());
}

void ScFiltersTest::testExtCondFormatXLSX()
{
    createScDoc("xlsx/tdf122102.xlsx");

    ScDocument* pDoc = getScDoc();

    // contains text and not contains text conditions
    ScConditionalFormat* pFormatA1 = pDoc->GetCondFormat(0, 0, 0);
    CPPUNIT_ASSERT(pFormatA1);
    ScConditionalFormat* pFormatA2 = pDoc->GetCondFormat(0, 1, 0);
    CPPUNIT_ASSERT(pFormatA2);
    ScConditionalFormat* pFormatA3 = pDoc->GetCondFormat(0, 2, 0);
    CPPUNIT_ASSERT(pFormatA3);
    ScConditionalFormat* pFormatA4 = pDoc->GetCondFormat(0, 3, 0);
    CPPUNIT_ASSERT(pFormatA4);

    ScRefCellValue aCellA1(*pDoc, ScAddress(0, 0, 0));
    OUString aCellStyleA1 = pFormatA1->GetCellStyle(aCellA1, ScAddress(0, 0, 0));
    CPPUNIT_ASSERT(!aCellStyleA1.isEmpty());

    ScRefCellValue aCellA2(*pDoc, ScAddress(0, 1, 0));
    OUString aCellStyleA2 = pFormatA2->GetCellStyle(aCellA2, ScAddress(0, 1, 0));
    CPPUNIT_ASSERT(!aCellStyleA2.isEmpty());

    ScRefCellValue aCellA3(*pDoc, ScAddress(0, 2, 0));
    OUString aCellStyleA3 = pFormatA3->GetCellStyle(aCellA3, ScAddress(0, 2, 0));
    CPPUNIT_ASSERT(!aCellStyleA3.isEmpty());

    ScRefCellValue aCellA4(*pDoc, ScAddress(0, 3, 0));
    OUString aCellStyleA4 = pFormatA4->GetCellStyle(aCellA4, ScAddress(0, 3, 0));
    CPPUNIT_ASSERT(!aCellStyleA4.isEmpty());
}

void ScFiltersTest::testUpdateCircleInMergedCellODS()
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

void ScFiltersTest::testDeleteCircleInMergedCellODS()
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

void ScFiltersTest::testBasicCellContentODS()
{
    createScDoc("ods/basic-cell-content.ods");

    ScDocument* pDoc = getScDoc();
    OUString aStr = pDoc->GetString(1, 1, 0); // B2
    CPPUNIT_ASSERT_EQUAL(OUString("LibreOffice Calc"), aStr);
    double fVal = pDoc->GetValue(1, 2, 0); // B3
    CPPUNIT_ASSERT_EQUAL(12345.0, fVal);
    aStr = pDoc->GetString(1, 3, 0); // B4
    CPPUNIT_ASSERT_EQUAL(OUString("A < B"), aStr);

    // Numeric value of 0.
    ScRefCellValue aCell;
    aCell.assign(*pDoc, ScAddress(1, 4, 0)); // B5
    CPPUNIT_ASSERT_EQUAL_MESSAGE("This cell must be numeric.", CELLTYPE_VALUE, aCell.getType());
    CPPUNIT_ASSERT_EQUAL(0.0, aCell.getDouble());
}

void ScFiltersTest::testBooleanFormatXLSX()
{
    createScDoc("xlsx/check-boolean.xlsx");
    ScDocument* pDoc = getScDoc();
    SvNumberFormatter* pNumFormatter = pDoc->GetFormatTable();
    // Saved as >"TRUE";"TRUE";"FALSE"< but reading converted back to >BOOLEAN<
    const OUString aBooleanTypeStr = "BOOLEAN";

    for (SCROW i = 0; i <= 1; i++)
    {
        sal_uInt32 nNumberFormat = pDoc->GetNumberFormat(0, i, 0);
        const SvNumberformat* pNumberFormat = pNumFormatter->GetEntry(nNumberFormat);
        const OUString& rFormatStr = pNumberFormat->GetFormatstring();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Number format != boolean", aBooleanTypeStr, rFormatStr);
    }
}

void ScFiltersTest::testTdf143809()
{
    createScDoc("ods/tdf143809.ods");

    ScDocument* pDoc = getScDoc();

    OUString aFormula = pDoc->GetFormula(0, 0, 0);
    CPPUNIT_ASSERT_EQUAL(OUString("=SUMPRODUCT(IFERROR(CEILING.MATH(DURATIONS,300),0))"), aFormula);

    // Without the fix in place, this test would have failed with
    // - Expected: 53700
    // - Actual  : Err:502
    CPPUNIT_ASSERT_EQUAL(OUString("53700"), pDoc->GetString(0, 0, 0));

    aFormula = pDoc->GetFormula(0, 1, 0);
    CPPUNIT_ASSERT_EQUAL(
        OUString(
            "=SUMPRODUCT(IFERROR(CEILING(SUMIFS(DURATIONS,IDS,IDS),300)/COUNTIFS(IDS,IDS),0))"),
        aFormula);
    CPPUNIT_ASSERT_EQUAL(OUString("51900"), pDoc->GetString(0, 1, 0));
}

void ScFiltersTest::testTdf76310()
{
    createScDoc("ods/tdf76310.ods");

    ScDocument* pDoc = getScDoc();

    OUString aFormula = pDoc->GetFormula(0, 0, 0);
    // Without the fix in place, this test would have failed with
    // - Expected: =1
    // +
    // 2
    // - Actual  : =1 + 2
    CPPUNIT_ASSERT_EQUAL(OUString("=1\n+\n2"), aFormula);
    ASSERT_DOUBLES_EQUAL(3.0, pDoc->GetValue(0, 0, 0));
}

void ScFiltersTest::testRangeNameXLS()
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

void ScFiltersTest::testRangeNameLocalXLS()
{
    createScDoc("xls/named-ranges-local.xls");
    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();

    ScDocument* pDoc = getScDoc();
    ScRangeName* pRangeName = pDoc->GetRangeName(0);
    CPPUNIT_ASSERT(pRangeName);
    CPPUNIT_ASSERT_EQUAL(size_t(2), pRangeName->size());

    OUString aFormula = pDoc->GetFormula(3, 11, 0);
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(local_name2)"), aFormula);
    ASSERT_DOUBLES_EQUAL(14.0, pDoc->GetValue(3, 11, 0));

    aFormula = pDoc->GetFormula(6, 4, 0);
    CPPUNIT_ASSERT_EQUAL(OUString("=local_name1"), aFormula);
}

void ScFiltersTest::testRangeNameXLSX()
{
    createScDoc("xlsx/named-ranges-global.xlsx");
    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();

    ScDocument* pDoc = getScDoc();
    testRangeNameImpl(*pDoc);
}

void ScFiltersTest::testRangeNameODS()
{
    createScDoc("ods/named-ranges-global.ods");

    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();

    ScDocument* pDoc = getScDoc();
    testRangeNameImpl(*pDoc);

    OUString aCSVPath = createFilePath(u"contentCSV/rangeExp_Sheet2.csv");
    testFile(aCSVPath, *pDoc, 1);
}

void ScFiltersTest::testHyperlinksXLSX()
{
    createScDoc("xlsx/hyperlinks.xlsx");
    ScDocument* pDoc = getScDoc();

    CPPUNIT_ASSERT_EQUAL(OUString("10:ABC10"), pDoc->GetString(ScAddress(0, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("10:ABC11"), pDoc->GetString(ScAddress(0, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("10:ABC12"), pDoc->GetString(ScAddress(0, 3, 0)));
}

void ScFiltersTest::testHardRecalcODS()
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

void ScFiltersTest::testFunctionsODS()
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

    // crashes at exit while unloading StarBasic code
    // xDocSh = loadDoc("user-defined-function.", FORMAT_ODS);
    // xDocSh->DoHardRecalc();
    // ScDocument& rDocUserDef = xDocSh->GetDocument();
    // createCSVPath("user-defined-function.", aCSVFileName);
    // testFile(aCSVFileName, rDocUserDef, 0);
}

void ScFiltersTest::testFunctionsExcel2010()
{
    createScDoc("xlsx/functions-excel-2010.xlsx");
    ScDocument* pDoc = getScDoc();
    pDoc->CalcAll(); // perform hard re-calculation.

    testFunctionsExcel2010_Impl(*pDoc);
}

void ScFiltersTest::testCeilingFloorXLSX()
{
    createScDoc("xlsx/ceiling-floor.xlsx");
    ScDocument* pDoc = getScDoc();
    pDoc->CalcAll(); // perform hard re-calculation.

    testCeilingFloor_Impl(*pDoc);
}

void ScFiltersTest::testCachedFormulaResultsODS()
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
                    OUStringToOString(aFormula, RTL_TEXTENCODING_UTF8).getStr(), OUString("TRUE"),
                    pDoc->GetString(nCol, nRow + 2, 2));

                OUString aIsTextFormula = "=ISTEXT("
                                          + OUString::number(static_cast<char>('A' + nCol))
                                          + OUString::number(nRow) + ")";
                pDoc->SetString(nCol, nRow + 4, 2, aIsTextFormula);
                CPPUNIT_ASSERT_EQUAL(OUString("FALSE"), pDoc->GetString(nCol, nRow + 4, 2));
            }
        }
    }
}

void ScFiltersTest::testCachedMatrixFormulaResultsODS()
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
    pDoc->SetString(2, 5, 2, "=ISERROR(A6)");
    double nVal = pDoc->GetValue(2, 5, 2);
    CPPUNIT_ASSERT_EQUAL(1.0, nVal);
}

void ScFiltersTest::testFormulaDepAcrossSheetsODS()
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

void ScFiltersTest::testFormulaDepDeleteContentsODS()
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
void testDBRanges_Impl(ScDocument& rDoc, sal_Int32 nFormat)
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
    if (nFormat == FORMAT_ODS) //excel doesn't support named db ranges
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

void ScFiltersTest::testDatabaseRangesODS()
{
    createScDoc("ods/database.ods");
    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();

    ScDocument* pDoc = getScDoc();

    testDBRanges_Impl(*pDoc, FORMAT_ODS);
}

void ScFiltersTest::testDatabaseRangesXLS()
{
    createScDoc("xls/database.xls");
    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();

    ScDocument* pDoc = getScDoc();

    testDBRanges_Impl(*pDoc, FORMAT_XLS);
}

void ScFiltersTest::testDatabaseRangesXLSX()
{
    createScDoc("xlsx/database.xlsx");
    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();

    ScDocument* pDoc = getScDoc();

    testDBRanges_Impl(*pDoc, FORMAT_XLSX);
}

void ScFiltersTest::testFormatsODS()
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

void ScFiltersTest::testMatrixODS()
{
    createScDoc("ods/matrix.ods");
    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();

    ScDocument* pDoc = getScDoc();

    OUString aCSVPath = createFilePath(u"contentCSV/matrix.csv");
    testFile(aCSVPath, *pDoc, 0);
}

void ScFiltersTest::testMatrixXLS()
{
    createScDoc("xls/matrix.xls");
    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();

    ScDocument* pDoc = getScDoc();

    OUString aCSVPath = createFilePath(u"contentCSV/matrix.csv");
    testFile(aCSVPath, *pDoc, 0);
}

void ScFiltersTest::testDoubleThinBorder()
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

void ScFiltersTest::testBorderODS()
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

void ScFiltersTest::testBordersOoo33()
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

void ScFiltersTest::testBugFixesODS()
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
        ScDBData* pDBData = pDoc->GetDBCollection()->getNamedDBs().findByUpperName("DBRANGE1");
        CPPUNIT_ASSERT(pDBData);
        CPPUNIT_ASSERT(pDBData->HasHeader());
        // no header
        pDBData = pDoc->GetDBCollection()->getNamedDBs().findByUpperName("DBRANGE2");
        CPPUNIT_ASSERT(pDBData);
        CPPUNIT_ASSERT(!pDBData->HasHeader());
    }

    {
        // fdo#59240
        OUString aCSVPath = createFilePath(u"contentCSV/bugFix_Sheet4.csv");
        testFile(aCSVPath, *pDoc, 3);
    }
}

void ScFiltersTest::testBugFixesXLS()
{
    createScDoc("xls/bug-fixes.xls");

    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();
}

void ScFiltersTest::testBugFixesXLSX()
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

void ScFiltersTest::testMergedCellsODS()
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

void ScFiltersTest::testRepeatedColumnsODS()
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

void ScFiltersTest::testDataValidityODS()
{
    createScDoc("ods/dataValidity.ods");
    ScDocument* pDoc = getScDoc();

    ScAddress aValBaseAddr1(2, 6, 0); //sheet1
    ScAddress aValBaseAddr2(2, 3, 1); //sheet2
    ScAddress aValBaseAddr3(2, 2, 2); //sheet3

    //sheet1's expected Data Validation Entry values
    ValDataTestParams aVDTParams1(SC_VALID_DECIMAL, ScConditionMode::Greater, "3.14", OUString(),
                                  *pDoc, aValBaseAddr1, "Too small",
                                  "The number you are trying to enter is not greater than 3.14! "
                                  "Are you sure you want to enter it anyway?",
                                  SC_VALERR_WARNING, 1);
    //sheet2's expected Data Validation Entry values
    ValDataTestParams aVDTParams2(SC_VALID_WHOLE, ScConditionMode::Between, "1", "10", *pDoc,
                                  aValBaseAddr2, "Error sheet 2",
                                  "Must be a whole number between 1 and 10.", SC_VALERR_STOP, 2);
    //sheet3's expected Data Validation Entry values
    ValDataTestParams aVDTParams3(SC_VALID_CUSTOM, ScConditionMode::Direct, "ISTEXT(C3)",
                                  OUString(), *pDoc, aValBaseAddr3, "Error sheet 3",
                                  "Must not be a numerical value.", SC_VALERR_STOP, 3);
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

void ScFiltersTest::testDataValidityXLSX()
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

void ScFiltersTest::testDataTableMortgageXLS()
{
    createScDoc("xls/data-table/mortgage.xls");

    ScDocument* pDoc = getScDoc();

    // One-variable table

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", OUString("=PMT(B3/12,B4,-B5)"),
                                 pDoc->GetFormula(3, 1, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", OUString("=MULTIPLE.OPERATIONS(D$2,$B$3,$C3)"),
                                 pDoc->GetFormula(3, 2, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", OUString("=MULTIPLE.OPERATIONS(D$2,$B$3,$C4)"),
                                 pDoc->GetFormula(3, 3, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", OUString("=MULTIPLE.OPERATIONS(D$2,$B$3,$C5)"),
                                 pDoc->GetFormula(3, 4, 0));

    // Two-variable table

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", OUString("=PMT(B9/12,B10,-B11)"),
                                 pDoc->GetFormula(2, 7, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!",
                                 OUString("=MULTIPLE.OPERATIONS($C$8,$B$9,$C9,$B$10,D$8)"),
                                 pDoc->GetFormula(3, 8, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!",
                                 OUString("=MULTIPLE.OPERATIONS($C$8,$B$9,$C10,$B$10,D$8)"),
                                 pDoc->GetFormula(3, 9, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!",
                                 OUString("=MULTIPLE.OPERATIONS($C$8,$B$9,$C11,$B$10,D$8)"),
                                 pDoc->GetFormula(3, 10, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!",
                                 OUString("=MULTIPLE.OPERATIONS($C$8,$B$9,$C9,$B$10,E$8)"),
                                 pDoc->GetFormula(4, 8, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!",
                                 OUString("=MULTIPLE.OPERATIONS($C$8,$B$9,$C10,$B$10,E$8)"),
                                 pDoc->GetFormula(4, 9, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!",
                                 OUString("=MULTIPLE.OPERATIONS($C$8,$B$9,$C11,$B$10,E$8)"),
                                 pDoc->GetFormula(4, 10, 0));
}

void ScFiltersTest::testDataTableOneVarXLSX()
{
    createScDoc("xlsx/data-table/one-variable.xlsx");

    ScDocument* pDoc = getScDoc();

    // Right now, we have a bug that prevents Calc from re-calculating these
    // cells automatically upon file load. We can remove this call if/when we
    // fix the aforementioned bug.
    pDoc->CalcAll();

    // B5:B11 should have multiple operations formula cells.  Just check the
    // top and bottom cells.

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", OUString("=MULTIPLE.OPERATIONS(B$4,$A$2,$A5)"),
                                 pDoc->GetFormula(1, 4, 0));

    CPPUNIT_ASSERT_EQUAL(2.0, pDoc->GetValue(ScAddress(1, 4, 0)));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", OUString("=MULTIPLE.OPERATIONS(B$4,$A$2,$A11)"),
                                 pDoc->GetFormula(1, 10, 0));

    CPPUNIT_ASSERT_EQUAL(14.0, pDoc->GetValue(ScAddress(1, 10, 0)));

    // Likewise, E5:I5 should have multiple operations formula cells.  Just
    // check the left- and right-most cells.

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", OUString("=MULTIPLE.OPERATIONS($D5,$B$2,E$4)"),
                                 pDoc->GetFormula(4, 4, 0));

    CPPUNIT_ASSERT_EQUAL(10.0, pDoc->GetValue(ScAddress(4, 4, 0)));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!", OUString("=MULTIPLE.OPERATIONS($D5,$B$2,I$4)"),
                                 pDoc->GetFormula(8, 4, 0));

    CPPUNIT_ASSERT_EQUAL(50.0, pDoc->GetValue(ScAddress(8, 4, 0)));
}

void ScFiltersTest::testDataTableMultiTableXLSX()
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
                                 OUString("=MULTIPLE.OPERATIONS($A$3,$E$1,$A4,$D$1,B$3)"),
                                 pDoc->GetFormula(1, 3, 0));

    CPPUNIT_ASSERT_EQUAL(1.0, pDoc->GetValue(ScAddress(1, 3, 0)));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula!",
                                 OUString("=MULTIPLE.OPERATIONS($A$3,$E$1,$A15,$D$1,M$3)"),
                                 pDoc->GetFormula(12, 14, 0));

    CPPUNIT_ASSERT_EQUAL(144.0, pDoc->GetValue(ScAddress(12, 14, 0)));
}

void ScFiltersTest::testBrokenQuotesCSV()
{
    createScDoc("csv/fdo48621_broken_quotes.csv");

    ScDocument* pDoc = getScDoc();

    OUString aCSVPath = createFilePath(u"contentCSV/fdo48621_broken_quotes_exported.csv");
    // fdo#48621
    testFile(aCSVPath, *pDoc, 0, StringType::PureString);
}

void ScFiltersTest::testCellValueXLSX()
{
    createScDoc("xlsx/cell-value.xlsx");
    ScDocument* pDoc = getScDoc();

    OUString aCSVPath = createFilePath(u"contentCSV/cell-value.csv");
    testFile(aCSVPath, *pDoc, 0);
}

void ScFiltersTest::testRowIndex1BasedXLSX()
{
    createScDoc("xlsx/row-index-1-based.xlsx");
    ScDocument* pDoc = getScDoc();

    // A1
    OUString aStr = pDoc->GetString(ScAddress(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("Action Plan.Name"), aStr);

    // B1
    aStr = pDoc->GetString(ScAddress(1, 0, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("Action Plan.Description"), aStr);

    // A2
    aStr = pDoc->GetString(ScAddress(0, 1, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("Jerry"), aStr);

    // B2 - multi-line text.
    const EditTextObject* pText = pDoc->GetEditText(ScAddress(1, 1, 0));
    CPPUNIT_ASSERT(pText);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3), pText->GetParagraphCount());
    aStr = pText->GetText(0);
    CPPUNIT_ASSERT_EQUAL(OUString("This is a longer Text."), aStr);
    aStr = pText->GetText(1);
    CPPUNIT_ASSERT_EQUAL(OUString("Second line."), aStr);
    aStr = pText->GetText(2);
    CPPUNIT_ASSERT_EQUAL(OUString("Third line."), aStr);
}

void ScFiltersTest::testImportCrash(const char* rFileName)
{
    createScDoc(rFileName);

    ScDocument* pDoc = getScDoc();
    pDoc->CalcAll(); // perform hard re-calculation.
}

void ScFiltersTest::testPasswordNew()
{
    //tests opening a file with new password algorithm
    createScDoc("ods/password.ods", /*pPassword*/ "test");
}

void ScFiltersTest::testPasswordOld()
{
    //tests opening a file with old password algorithm
    createScDoc("ods/passwordOld.ods", /*pPassword*/ "test");
}

void ScFiltersTest::testPasswordWrongSHA()
{
    //tests opening a file wrongly using the new password algorithm
    //in a sxc with the key algorithm missing
    createScDoc("ods/passwordWrongSHA.ods", /*pPassword*/ "test");
}

void ScFiltersTest::testControlImport()
{
    createScDoc("xlsx/singlecontrol.xlsx");

    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIA(xDoc->getSheets(), UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xIA->getByIndex(0),
                                                                 UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIA_DrawPage(xDrawPageSupplier->getDrawPage(),
                                                         UNO_QUERY_THROW);
    uno::Reference<drawing::XControlShape> xControlShape(xIA_DrawPage->getByIndex(0),
                                                         UNO_QUERY_THROW);
}

void ScFiltersTest::testActiveXOptionButtonGroup()
{
    createScDoc("xlsx/tdf111980_radioButtons.xlsx");
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIA(xDoc->getSheets(), UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xIA->getByIndex(0),
                                                                 UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIA_DrawPage(xDrawPageSupplier->getDrawPage(),
                                                         UNO_QUERY_THROW);

    OUString sGroupName;
    uno::Reference<drawing::XControlShape> xControlShape(xIA_DrawPage->getByIndex(0),
                                                         UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(),
                                                     uno::UNO_QUERY_THROW);
    xPropertySet->getPropertyValue("GroupName") >>= sGroupName;
    CPPUNIT_ASSERT_EQUAL(OUString("Sheet1"), sGroupName);

    // Optionbuttons (without Group names) were not grouped.
    // The two optionbuttons should have the same auto-generated group name.
    OUString sGroupName2; //ActiveX controls
    xControlShape.set(xIA_DrawPage->getByIndex(2), uno::UNO_QUERY_THROW);
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY_THROW);
    xPropertySet->getPropertyValue("GroupName") >>= sGroupName2;
    CPPUNIT_ASSERT_EQUAL(false, sGroupName2.isEmpty());

    OUString sGroupName3;
    xControlShape.set(xIA_DrawPage->getByIndex(3), uno::UNO_QUERY_THROW);
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY_THROW);
    xPropertySet->getPropertyValue("GroupName") >>= sGroupName3;
    CPPUNIT_ASSERT_EQUAL(sGroupName2, sGroupName3);
    CPPUNIT_ASSERT(sGroupName != sGroupName3);

    OUString sGroupName4; //Form controls
    xControlShape.set(xIA_DrawPage->getByIndex(4), uno::UNO_QUERY_THROW);
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY_THROW);
    xPropertySet->getPropertyValue("GroupName") >>= sGroupName4;
    CPPUNIT_ASSERT_EQUAL(false, sGroupName4.isEmpty());

    OUString sGroupName5;
    xControlShape.set(xIA_DrawPage->getByIndex(5), uno::UNO_QUERY_THROW);
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY_THROW);
    xPropertySet->getPropertyValue("GroupName") >>= sGroupName5;
    CPPUNIT_ASSERT_EQUAL(sGroupName4, sGroupName5);
    CPPUNIT_ASSERT(sGroupName2 != sGroupName5);
    CPPUNIT_ASSERT(sGroupName != sGroupName5);

    OUString sGroupName7; //Form radiobutton autogrouped by GroupBox
    xControlShape.set(xIA_DrawPage->getByIndex(7), uno::UNO_QUERY_THROW);
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY_THROW);
    xPropertySet->getPropertyValue("GroupName") >>= sGroupName7;
    CPPUNIT_ASSERT_EQUAL(OUString("autoGroup_Group Box 7"), sGroupName7);

    OUString sGroupName8;
    xControlShape.set(xIA_DrawPage->getByIndex(8), uno::UNO_QUERY_THROW);
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY_THROW);
    xPropertySet->getPropertyValue("GroupName") >>= sGroupName8;
    CPPUNIT_ASSERT_EQUAL(sGroupName7, sGroupName8);
    CPPUNIT_ASSERT(sGroupName4 != sGroupName8);
    CPPUNIT_ASSERT(sGroupName2 != sGroupName8);
    CPPUNIT_ASSERT(sGroupName != sGroupName8);

    OUString sGroupName9; //Form radiobutton not fully inside GroupBox
    xControlShape.set(xIA_DrawPage->getByIndex(9), uno::UNO_QUERY_THROW);
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY_THROW);
    xPropertySet->getPropertyValue("GroupName") >>= sGroupName9;
    CPPUNIT_ASSERT_EQUAL(sGroupName4, sGroupName9);

    OUString sGroupName10; //ActiveX unaffected by GroupBox
    xControlShape.set(xIA_DrawPage->getByIndex(10), uno::UNO_QUERY_THROW);
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY_THROW);
    xPropertySet->getPropertyValue("GroupName") >>= sGroupName10;
    CPPUNIT_ASSERT_EQUAL(sGroupName, sGroupName10);
}

void ScFiltersTest::testChartImportODS()
{
    createScDoc("ods/chart-import-basic.ods");

    ScDocument* pDoc = getScDoc();

    // Ensure that the document contains "Empty", "Chart", "Data" and "Title" sheets in this exact order.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be 4 sheets in this document.", sal_Int16(4),
                                 pDoc->GetTableCount());
    OUString aName;
    pDoc->GetName(0, aName);
    CPPUNIT_ASSERT_EQUAL(OUString("Empty"), aName);
    pDoc->GetName(1, aName);
    CPPUNIT_ASSERT_EQUAL(OUString("Chart"), aName);
    pDoc->GetName(2, aName);
    CPPUNIT_ASSERT_EQUAL(OUString("Data"), aName);
    pDoc->GetName(3, aName);
    CPPUNIT_ASSERT_EQUAL(OUString("Title"), aName);

    // Retrieve the chart object instance from the 2nd page (for the 2nd sheet).
    const SdrOle2Obj* pOleObj = getSingleChartObject(*pDoc, 1);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve a chart object from the 2nd sheet.", pOleObj);

    ScRangeList aRanges = getChartRanges(*pDoc, *pOleObj);

    CPPUNIT_ASSERT_MESSAGE("Data series title cell not found.",
                           aRanges.Contains(ScAddress(1, 0, 3))); // B1 on Title
    CPPUNIT_ASSERT_MESSAGE("Data series label range not found.",
                           aRanges.Contains(ScRange(0, 1, 2, 0, 3, 2))); // A2:A4 on Data
    CPPUNIT_ASSERT_MESSAGE("Data series value range not found.",
                           aRanges.Contains(ScRange(1, 1, 2, 1, 3, 2))); // B2:B4 on Data
}

#if HAVE_MORE_FONTS
void ScFiltersTest::testChartImportXLS()
{
    createScDoc("xls/chartx.xls");

    ScDocument* pDoc = getScDoc();

    // Retrieve the chart object instance from the 2nd page (for the 2nd sheet).
    const SdrOle2Obj* pOleObj = getSingleChartObject(*pDoc, 0);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve a chart object from the 2nd sheet.", pOleObj);

    CPPUNIT_ASSERT_EQUAL(tools::Long(11137), pOleObj->GetLogicRect().getOpenWidth());
    CPPUNIT_ASSERT(8640L > pOleObj->GetLogicRect().getOpenHeight());
}
#endif

void ScFiltersTest::testNumberFormatHTML()
{
    setImportFilterName("calc_HTML_WebQuery");
    createScDoc("html/numberformat.html");

    ScDocument* pDoc = getScDoc();

    // Check the header just in case.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell value is not as expected", OUString("Product"),
                                 pDoc->GetString(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell value is not as expected", OUString("Price"),
                                 pDoc->GetString(1, 0, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell value is not as expected", OUString("Note"),
                                 pDoc->GetString(2, 0, 0));

    // B2 should be imported as a value cell.
    bool bHasValue = pDoc->HasValueData(1, 1, 0);
    CPPUNIT_ASSERT_MESSAGE("Fail to import number as a value cell.", bHasValue);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Incorrect value.", 199.98, pDoc->GetValue(1, 1, 0));
}

void ScFiltersTest::testNumberFormatCSV()
{
    createScDoc("csv/numberformat.csv");

    ScDocument* pDoc = getScDoc();

    // Check the header just in case.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell value is not as expected", OUString("Product"),
                                 pDoc->GetString(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell value is not as expected", OUString("Price"),
                                 pDoc->GetString(1, 0, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell value is not as expected", OUString("Note"),
                                 pDoc->GetString(2, 0, 0));

    // B2 should be imported as a value cell.
    bool bHasValue = pDoc->HasValueData(1, 1, 0);
    CPPUNIT_ASSERT_MESSAGE("Fail to import number as a value cell.", bHasValue);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Incorrect value.", 199.98, pDoc->GetValue(1, 1, 0));
}

void ScFiltersTest::testCellAnchoredShapesODS()
{
    createScDoc("ods/cell-anchored-shapes.ods");

    // There are two cell-anchored objects on the first sheet.
    ScDocument* pDoc = getScDoc();

    CPPUNIT_ASSERT_MESSAGE("There should be at least one sheet.", pDoc->GetTableCount() > 0);

    ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
    SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("draw page for sheet 1 should exist.", pPage);
    const size_t nCount = pPage->GetObjCount();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be 2 objects.", static_cast<size_t>(2), nCount);
    for (size_t i = 0; i < nCount; ++i)
    {
        SdrObject* pObj = pPage->GetObj(i);
        CPPUNIT_ASSERT_MESSAGE("Failed to get drawing object.", pObj);
        ScDrawObjData* pData = ScDrawLayer::GetObjData(pObj);
        CPPUNIT_ASSERT_MESSAGE("Failed to retrieve user data for this object.", pData);
        CPPUNIT_ASSERT_MESSAGE("Bounding rectangle should have been calculated upon import.",
                               !pData->getShapeRect().IsEmpty());
    }
}

void ScFiltersTest::testCellAnchoredHiddenShapesXLSX()
{
    createScDoc("xlsx/cell-anchored-hidden-shapes.xlsx");

    // There are two cell-anchored objects on the first sheet.
    ScDocument* pDoc = getScDoc();

    CPPUNIT_ASSERT_MESSAGE("There should be at least one sheet.", pDoc->GetTableCount() > 0);

    ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
    SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("draw page for sheet 1 should exist.", pPage);
    const size_t nCount = pPage->GetObjCount();
    CPPUNIT_ASSERT_MESSAGE("There should be 2 shapes.", !(nCount == 2));

    SdrObject* pObj = pPage->GetObj(1);
    CPPUNIT_ASSERT_MESSAGE("Failed to get drawing object.", pObj);
    CPPUNIT_ASSERT_MESSAGE(
        "The shape having same twocellanchor from and to attribute values, is visible.",
        !pObj->IsVisible());
}

void ScFiltersTest::testRowHeightODS()
{
    createScDoc("ods/row-height-import.ods");

    SCTAB nTab = 0;
    SCROW nRow = 0;
    ScDocument* pDoc = getScDoc();

    // The first 3 rows have manual heights.
    int nHeight = pDoc->GetRowHeight(nRow, nTab, false);
    bool bManual = pDoc->IsManualRowHeight(nRow, nTab);
    CPPUNIT_ASSERT_EQUAL(600, nHeight);
    CPPUNIT_ASSERT_MESSAGE("this row should have a manual row height.", bManual);
    nHeight = pDoc->GetRowHeight(++nRow, nTab, false);
    bManual = pDoc->IsManualRowHeight(nRow, nTab);
    CPPUNIT_ASSERT_EQUAL(1200, nHeight);
    CPPUNIT_ASSERT_MESSAGE("this row should have a manual row height.", bManual);
    nHeight = pDoc->GetRowHeight(++nRow, nTab, false);
    bManual = pDoc->IsManualRowHeight(nRow, nTab);
    CPPUNIT_ASSERT_EQUAL(1800, nHeight);
    CPPUNIT_ASSERT_MESSAGE("this row should have a manual row height.", bManual);

    // This one should have an automatic row height.
    bManual = pDoc->IsManualRowHeight(++nRow, nTab);
    CPPUNIT_ASSERT_MESSAGE("Row should have an automatic height.", !bManual);

    // Followed by a row with manual height.
    nHeight = pDoc->GetRowHeight(++nRow, nTab, false);
    bManual = pDoc->IsManualRowHeight(nRow, nTab);
    CPPUNIT_ASSERT_EQUAL(2400, nHeight);
    CPPUNIT_ASSERT_MESSAGE("this row should have a manual row height.", bManual);

    // And all the rest should have automatic heights.
    bManual = pDoc->IsManualRowHeight(++nRow, nTab);
    CPPUNIT_ASSERT_MESSAGE("Row should have an automatic height.", !bManual);

    bManual = pDoc->IsManualRowHeight(pDoc->MaxRow(), nTab);
    CPPUNIT_ASSERT_MESSAGE("Row should have an automatic height.", !bManual);
}

void ScFiltersTest::testRichTextContentODS()
{
    createScDoc("ods/rich-text-cells.ods");
    ScDocument* pDoc = getScDoc();

    OUString aTabName;
    CPPUNIT_ASSERT_MESSAGE("Failed to get the name of the first sheet.",
                           pDoc->GetName(0, aTabName));

    // All tested cells are in the first column.
    ScAddress aPos(0, 0, 0);

    // Normal simple string with no formatting.
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_STRING, pDoc->GetCellType(aPos));
    CPPUNIT_ASSERT_EQUAL(OUString("Normal"), pDoc->GetString(aPos.Col(), aPos.Row(), aPos.Tab()));

    // Normal string with bold applied to the whole cell.
    {
        aPos.IncRow();
        CPPUNIT_ASSERT_EQUAL(CELLTYPE_STRING, pDoc->GetCellType(aPos));
        CPPUNIT_ASSERT_EQUAL(OUString("All bold"),
                             pDoc->GetString(aPos.Col(), aPos.Row(), aPos.Tab()));
        const ScPatternAttr* pAttr = pDoc->GetPattern(aPos.Col(), aPos.Row(), aPos.Tab());
        CPPUNIT_ASSERT_MESSAGE("Failed to get cell attribute.", pAttr);
        const SvxWeightItem& rWeightItem = pAttr->GetItem(ATTR_FONT_WEIGHT);
        CPPUNIT_ASSERT_EQUAL(WEIGHT_BOLD, rWeightItem.GetWeight());
    }

    // This cell has an unformatted but multi-line content. Multi-line text is
    // stored in edit cell even if it has no formatting applied.
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_EDIT, pDoc->GetCellType(aPos));
    const EditTextObject* pEditText = pDoc->GetEditText(aPos);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve edit text object.", pEditText);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3), pEditText->GetParagraphCount());
    OUString aParaText = pEditText->GetText(0);
    CPPUNIT_ASSERT_EQUAL(OUString("one"), aParaText);
    aParaText = pEditText->GetText(1);
    CPPUNIT_ASSERT_EQUAL(OUString("two"), aParaText);
    aParaText = pEditText->GetText(2);
    CPPUNIT_ASSERT_EQUAL(OUString("three"), aParaText);

    // Cell with sheet name field item.
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_EDIT, pDoc->GetCellType(aPos));
    pEditText = pDoc->GetEditText(aPos);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve edit text object.", pEditText);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), pEditText->GetParagraphCount());
    aParaText = pEditText->GetText(0);
    CPPUNIT_ASSERT_MESSAGE("Unexpected text.", aParaText.startsWith("Sheet name is "));
    CPPUNIT_ASSERT_MESSAGE("Sheet name field item not found.",
                           pEditText->HasField(text::textfield::Type::TABLE));
    CPPUNIT_ASSERT_EQUAL(OUString("Sheet name is Test."), ScEditUtil::GetString(*pEditText, pDoc));
    CPPUNIT_ASSERT_EQUAL(OUString("Sheet name is ?."), ScEditUtil::GetString(*pEditText, nullptr));

    // Cell with URL field item.
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_EDIT, pDoc->GetCellType(aPos));
    pEditText = pDoc->GetEditText(aPos);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve edit text object.", pEditText);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), pEditText->GetParagraphCount());
    aParaText = pEditText->GetText(0);
    CPPUNIT_ASSERT_MESSAGE("Unexpected text.", aParaText.startsWith("URL: "));
    CPPUNIT_ASSERT_MESSAGE("URL field item not found.",
                           pEditText->HasField(text::textfield::Type::URL));
    CPPUNIT_ASSERT_EQUAL(OUString("URL: http://libreoffice.org"),
                         ScEditUtil::GetString(*pEditText, pDoc));
    CPPUNIT_ASSERT_EQUAL(OUString("URL: http://libreoffice.org"),
                         ScEditUtil::GetString(*pEditText, nullptr));

    // Cell with Date field item.
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_EDIT, pDoc->GetCellType(aPos));
    pEditText = pDoc->GetEditText(aPos);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve edit text object.", pEditText);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), pEditText->GetParagraphCount());
    aParaText = pEditText->GetText(0);
    CPPUNIT_ASSERT_MESSAGE("Unexpected text.", aParaText.startsWith("Date: "));
    CPPUNIT_ASSERT_MESSAGE("Date field item not found.",
                           pEditText->HasField(text::textfield::Type::DATE));
    CPPUNIT_ASSERT_MESSAGE("Date field not resolved with pDoc->",
                           ScEditUtil::GetString(*pEditText, pDoc).indexOf("/20") > 0);
    CPPUNIT_ASSERT_MESSAGE("Date field not resolved with NULL.",
                           ScEditUtil::GetString(*pEditText, nullptr).indexOf("/20") > 0);

    // Cell with DocInfo title field item.
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_EDIT, pDoc->GetCellType(aPos));
    pEditText = pDoc->GetEditText(aPos);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve edit text object.", pEditText);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), pEditText->GetParagraphCount());
    aParaText = pEditText->GetText(0);
    CPPUNIT_ASSERT_MESSAGE("Unexpected text.", aParaText.startsWith("Title: "));
    CPPUNIT_ASSERT_MESSAGE("DocInfo title field item not found.",
                           pEditText->HasField(text::textfield::Type::DOCINFO_TITLE));
    CPPUNIT_ASSERT_EQUAL(OUString("Title: Test Document"), ScEditUtil::GetString(*pEditText, pDoc));
    CPPUNIT_ASSERT_EQUAL(OUString("Title: ?"), ScEditUtil::GetString(*pEditText, nullptr));

    // Cell with sentence with both bold and italic sequences.
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_EDIT, pDoc->GetCellType(aPos));
    pEditText = pDoc->GetEditText(aPos);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve edit text object.", pEditText);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), pEditText->GetParagraphCount());
    aParaText = pEditText->GetText(0);
    CPPUNIT_ASSERT_EQUAL(OUString("Sentence with bold and italic."), aParaText);
    std::vector<EECharAttrib> aAttribs;
    pEditText->GetCharAttribs(0, aAttribs);
    {
        bool bHasBold = false, bHasItalic = false;
        for (const auto& rAttrib : aAttribs)
        {
            OUString aSeg = aParaText.copy(rAttrib.nStart, rAttrib.nEnd - rAttrib.nStart);
            const SfxPoolItem* pAttr = rAttrib.pAttr;
            if (aSeg == "bold" && pAttr->Which() == EE_CHAR_WEIGHT && !bHasBold)
            {
                const SvxWeightItem& rItem = static_cast<const SvxWeightItem&>(*pAttr);
                bHasBold = (rItem.GetWeight() == WEIGHT_BOLD);
            }
            else if (aSeg == "italic" && pAttr->Which() == EE_CHAR_ITALIC && !bHasItalic)
            {
                const SvxPostureItem& rItem = static_cast<const SvxPostureItem&>(*pAttr);
                bHasItalic = (rItem.GetPosture() == ITALIC_NORMAL);
            }
        }
        CPPUNIT_ASSERT_MESSAGE("This sentence is expected to have both bold and italic sequences.",
                               bHasBold);
        CPPUNIT_ASSERT_MESSAGE("This sentence is expected to have both bold and italic sequences.",
                               bHasItalic);
    }

    // Cell with multi-line content with formatting applied.
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_EDIT, pDoc->GetCellType(aPos));
    pEditText = pDoc->GetEditText(aPos);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve edit text object.", pEditText);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3), pEditText->GetParagraphCount());
    aParaText = pEditText->GetText(0);
    CPPUNIT_ASSERT_EQUAL(OUString("bold"), aParaText);
    aParaText = pEditText->GetText(1);
    CPPUNIT_ASSERT_EQUAL(OUString("italic"), aParaText);
    aParaText = pEditText->GetText(2);
    CPPUNIT_ASSERT_EQUAL(OUString("underlined"), aParaText);

    // first line is bold.
    pEditText->GetCharAttribs(0, aAttribs);
    {
        bool bHasBold
            = std::any_of(aAttribs.begin(), aAttribs.end(), [](const EECharAttrib& rAttrib) {
                  return rAttrib.pAttr->Which() == EE_CHAR_WEIGHT
                         && static_cast<const SvxWeightItem&>(*rAttrib.pAttr).GetWeight()
                                == WEIGHT_BOLD;
              });
        CPPUNIT_ASSERT_MESSAGE("First line should be bold.", bHasBold);
    }

    // second line is italic.
    pEditText->GetCharAttribs(1, aAttribs);
    bool bHasItalic
        = std::any_of(aAttribs.begin(), aAttribs.end(), [](const EECharAttrib& rAttrib) {
              return rAttrib.pAttr->Which() == EE_CHAR_ITALIC
                     && static_cast<const SvxPostureItem&>(*rAttrib.pAttr).GetPosture()
                            == ITALIC_NORMAL;
          });
    CPPUNIT_ASSERT_MESSAGE("Second line should be italic.", bHasItalic);

    // third line is underlined.
    pEditText->GetCharAttribs(2, aAttribs);
    bool bHasUnderline
        = std::any_of(aAttribs.begin(), aAttribs.end(), [](const EECharAttrib& rAttrib) {
              return rAttrib.pAttr->Which() == EE_CHAR_UNDERLINE
                     && static_cast<const SvxUnderlineItem&>(*rAttrib.pAttr).GetLineStyle()
                            == LINESTYLE_SINGLE;
          });
    CPPUNIT_ASSERT_MESSAGE("Second line should be underlined.", bHasUnderline);

    // URL with formats applied.  For now, we'll check whether or not the
    // field objects gets imported.  Later we should add checks for the
    // formats.
    aPos.IncRow();
    pEditText = pDoc->GetEditText(aPos);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve edit text object.", pEditText);
    CPPUNIT_ASSERT_MESSAGE("URL field item not found.",
                           pEditText->HasField(text::textfield::Type::URL));

    // Sheet name with formats applied.
    aPos.IncRow();
    pEditText = pDoc->GetEditText(aPos);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve edit text object.", pEditText);
    CPPUNIT_ASSERT_MESSAGE("Sheet name field item not found.",
                           pEditText->HasField(text::textfield::Type::TABLE));

    // Date with formats applied.
    aPos.IncRow();
    pEditText = pDoc->GetEditText(aPos);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve edit text object.", pEditText);
    CPPUNIT_ASSERT_MESSAGE("Date field item not found.",
                           pEditText->HasField(text::textfield::Type::DATE));

    // Document title with formats applied.
    aPos.IncRow();
    pEditText = pDoc->GetEditText(aPos);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve edit text object.", pEditText);
    CPPUNIT_ASSERT_MESSAGE("Date field item not found.",
                           pEditText->HasField(text::textfield::Type::DOCINFO_TITLE));

    // URL for a file in the same directory. It should be converted into an absolute URL on import.
    aPos.IncRow();
    pEditText = pDoc->GetEditText(aPos);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve edit text object.", pEditText);
    const SvxFieldData* pData = pEditText->GetFieldData(0, 0, text::textfield::Type::URL);
    CPPUNIT_ASSERT_MESSAGE("Failed to get the URL data.", pData);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Failed to get the URL data.", text::textfield::Type::URL,
                                 pData->GetClassId());
    const SvxURLField* pURLData = static_cast<const SvxURLField*>(pData);
    CPPUNIT_ASSERT_MESSAGE("URL is not absolute with respect to the file system.",
                           pURLData->GetURL().startsWith("file:///"));

    // Embedded spaces as <text:s text:c='4' />, normal text
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_STRING, pDoc->GetCellType(aPos));
    CPPUNIT_ASSERT_EQUAL(OUString("one     two"),
                         pDoc->GetString(aPos.Col(), aPos.Row(), aPos.Tab()));

    // Leading space as <text:s />.
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_STRING, pDoc->GetCellType(aPos));
    CPPUNIT_ASSERT_EQUAL(OUString(" =3+4"), pDoc->GetString(aPos.Col(), aPos.Row(), aPos.Tab()));

    // Embedded spaces with <text:s text:c='4' /> inside a <text:span>, text
    // partly bold.
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_EDIT, pDoc->GetCellType(aPos));
    pEditText = pDoc->GetEditText(aPos);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve edit text object.", pEditText);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), pEditText->GetParagraphCount());
    aParaText = pEditText->GetText(0);
    CPPUNIT_ASSERT_EQUAL(OUString("one     two"), aParaText);
    pEditText->GetCharAttribs(0, aAttribs);
    {
        auto it = std::find_if(aAttribs.begin(), aAttribs.end(), [](const EECharAttrib& rAttrib) {
            return rAttrib.pAttr->Which() == EE_CHAR_WEIGHT
                   && static_cast<const SvxWeightItem&>(*rAttrib.pAttr).GetWeight() == WEIGHT_BOLD;
        });
        bool bHasBold = (it != aAttribs.end());
        if (bHasBold)
        {
            OUString aSeg = aParaText.copy(it->nStart, it->nEnd - it->nStart);
            CPPUNIT_ASSERT_EQUAL(OUString("e     t"), aSeg);
        }
        CPPUNIT_ASSERT_MESSAGE("Expected a bold sequence.", bHasBold);
    }
}

void ScFiltersTest::testDataBarODS()
{
    createScDoc("ods/databar.ods");

    ScDocument* pDoc = getScDoc();
    testDataBar_Impl(*pDoc);
}

void ScFiltersTest::testDataBarXLSX()
{
    createScDoc("xlsx/databar.xlsx");

    ScDocument* pDoc = getScDoc();
    testDataBar_Impl(*pDoc);
}

void ScFiltersTest::testColorScaleODS()
{
    createScDoc("ods/colorscale.ods");
    ScDocument* pDoc = getScDoc();

    testColorScale2Entry_Impl(*pDoc);
    testColorScale3Entry_Impl(*pDoc);
}

void ScFiltersTest::testColorScaleXLSX()
{
    createScDoc("xlsx/colorscale.xlsx");
    ScDocument* pDoc = getScDoc();

    testColorScale2Entry_Impl(*pDoc);
    testColorScale3Entry_Impl(*pDoc);
}

void ScFiltersTest::testNewCondFormatODS()
{
    createScDoc("ods/new_cond_format_test.ods");

    ScDocument* pDoc = getScDoc();

    OUString aCSVPath = createFilePath(u"contentCSV/new_cond_format_test.csv");
    testCondFile(aCSVPath, pDoc, 0);
}

void ScFiltersTest::testNewCondFormatXLSX()
{
    createScDoc("xlsx/new_cond_format_test.xlsx");

    ScDocument* pDoc = getScDoc();

    OUString aCSVPath = createFilePath(u"contentCSV/new_cond_format_test.csv");
    testCondFile(aCSVPath, pDoc, 0);
}

void ScFiltersTest::testCondFormatImportCellIs()
{
    createScDoc("xlsx/condFormat_cellis.xlsx");

    ScDocument* pDoc = getScDoc();
    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetCondFormList(0)->size());

    ScConditionalFormat* pFormat = pDoc->GetCondFormat(0, 0, 0);
    CPPUNIT_ASSERT(pFormat);

    const ScFormatEntry* pEntry = pFormat->GetEntry(0);
    CPPUNIT_ASSERT(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScFormatEntry::Type::ExtCondition, pEntry->GetType());

    const ScCondFormatEntry* pCondition = static_cast<const ScCondFormatEntry*>(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScConditionMode::Equal, pCondition->GetOperation());

    OUString aStr = pCondition->GetExpression(ScAddress(0, 0, 0), 0);
    CPPUNIT_ASSERT_EQUAL(OUString("$Sheet2.$A$2"), aStr);

    pEntry = pFormat->GetEntry(1);
    CPPUNIT_ASSERT(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScFormatEntry::Type::ExtCondition, pEntry->GetType());

    pCondition = static_cast<const ScCondFormatEntry*>(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScConditionMode::Equal, pCondition->GetOperation());

    aStr = pCondition->GetExpression(ScAddress(0, 0, 0), 0);
    CPPUNIT_ASSERT_EQUAL(OUString("$Sheet2.$A$1"), aStr);
}

void ScFiltersTest::testCondFormatThemeColorXLSX()
{
    createScDoc("xlsx/condformat_theme_color.xlsx");

    ScDocument* pDoc = getScDoc();
    ScConditionalFormat* pFormat = pDoc->GetCondFormat(0, 0, 0);
    const ScFormatEntry* pEntry = pFormat->GetEntry(0);
    CPPUNIT_ASSERT(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScFormatEntry::Type::Databar, pEntry->GetType());
    const ScDataBarFormat* pDataBar = static_cast<const ScDataBarFormat*>(pEntry);
    const ScDataBarFormatData* pDataBarFormatData = pDataBar->GetDataBarData();

    CPPUNIT_ASSERT_EQUAL(Color(157, 195, 230), pDataBarFormatData->maPositiveColor);
    CPPUNIT_ASSERT(pDataBarFormatData->mxNegativeColor);
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, *pDataBarFormatData->mxNegativeColor);

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetCondFormList(1)->size());
    pFormat = pDoc->GetCondFormat(0, 0, 1);
    CPPUNIT_ASSERT(pFormat);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pFormat->size());
    pEntry = pFormat->GetEntry(0);
    CPPUNIT_ASSERT(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScFormatEntry::Type::Colorscale, pEntry->GetType());
    const ScColorScaleFormat* pColorScale = static_cast<const ScColorScaleFormat*>(pEntry);
    CPPUNIT_ASSERT_EQUAL(size_t(2), pColorScale->size());
    const ScColorScaleEntry* pColorScaleEntry = pColorScale->GetEntry(0);
    CPPUNIT_ASSERT(pColorScaleEntry);
    CPPUNIT_ASSERT_EQUAL(Color(255, 230, 153), pColorScaleEntry->GetColor());

    pColorScaleEntry = pColorScale->GetEntry(1);
    CPPUNIT_ASSERT(pColorScaleEntry);
    CPPUNIT_ASSERT_EQUAL(Color(157, 195, 230), pColorScaleEntry->GetColor());
}

void ScFiltersTest::testCondFormatThemeColor2XLSX()
{
    createScDoc("xlsx/cond_format_theme_color2.xlsx");

    ScDocument* pDoc = getScDoc();
    ScConditionalFormat* pFormat = pDoc->GetCondFormat(5, 5, 0);
    CPPUNIT_ASSERT(pFormat);
    const ScFormatEntry* pEntry = pFormat->GetEntry(0);
    CPPUNIT_ASSERT(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScFormatEntry::Type::Databar, pEntry->GetType());
    const ScDataBarFormat* pDataBar = static_cast<const ScDataBarFormat*>(pEntry);
    const ScDataBarFormatData* pDataBarFormatData = pDataBar->GetDataBarData();

    CPPUNIT_ASSERT_EQUAL(Color(99, 142, 198), pDataBarFormatData->maPositiveColor);
    CPPUNIT_ASSERT(pDataBarFormatData->mxNegativeColor);
    CPPUNIT_ASSERT_EQUAL(Color(217, 217, 217), *pDataBarFormatData->mxNegativeColor);
    CPPUNIT_ASSERT_EQUAL(Color(197, 90, 17), pDataBarFormatData->maAxisColor);
}

namespace
{
void checkDatabarPositiveColor(const ScConditionalFormat* pFormat, const Color& rColor)
{
    CPPUNIT_ASSERT(pFormat);
    const ScFormatEntry* pEntry = pFormat->GetEntry(0);
    CPPUNIT_ASSERT(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScFormatEntry::Type::Databar, pEntry->GetType());
    const ScDataBarFormat* pDataBar = static_cast<const ScDataBarFormat*>(pEntry);
    const ScDataBarFormatData* pDataBarFormatData = pDataBar->GetDataBarData();

    CPPUNIT_ASSERT_EQUAL(rColor, pDataBarFormatData->maPositiveColor);
}
}

void ScFiltersTest::testCondFormatThemeColor3XLSX()
{
    createScDoc("xlsx/cond_format_theme_color3.xlsx");

    ScDocument* pDoc = getScDoc();
    ScConditionalFormat* pFormat = pDoc->GetCondFormat(1, 3, 0);
    CPPUNIT_ASSERT(pFormat);
    const ScFormatEntry* pEntry = pFormat->GetEntry(0);
    CPPUNIT_ASSERT(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScFormatEntry::Type::Colorscale, pEntry->GetType());
    const ScColorScaleFormat* pColorScale = static_cast<const ScColorScaleFormat*>(pEntry);

    CPPUNIT_ASSERT_EQUAL(size_t(2), pColorScale->size());
    const ScColorScaleEntry* pColorScaleEntry = pColorScale->GetEntry(0);
    CPPUNIT_ASSERT(pColorScaleEntry);
    CPPUNIT_ASSERT_EQUAL(Color(175, 171, 171), pColorScaleEntry->GetColor());

    pColorScaleEntry = pColorScale->GetEntry(1);
    CPPUNIT_ASSERT(pColorScaleEntry);
    CPPUNIT_ASSERT_EQUAL(Color(51, 63, 80), pColorScaleEntry->GetColor());

    pFormat = pDoc->GetCondFormat(3, 3, 0);
    checkDatabarPositiveColor(pFormat, Color(59, 56, 56));

    pFormat = pDoc->GetCondFormat(5, 3, 0);
    checkDatabarPositiveColor(pFormat, Color(173, 185, 202));

    pFormat = pDoc->GetCondFormat(7, 3, 0);
    checkDatabarPositiveColor(pFormat, Color(89, 89, 89));

    pFormat = pDoc->GetCondFormat(9, 3, 0);
    checkDatabarPositiveColor(pFormat, Color(217, 217, 217));
}

namespace
{
void testComplexIconSetsXLSX_Impl(const ScDocument& rDoc, SCCOL nCol, ScIconSetType eType)
{
    ScConditionalFormat* pFormat = rDoc.GetCondFormat(nCol, 1, 0);
    CPPUNIT_ASSERT(pFormat);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pFormat->size());
    const ScFormatEntry* pEntry = pFormat->GetEntry(0);
    CPPUNIT_ASSERT(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScFormatEntry::Type::Iconset, pEntry->GetType());
    const ScIconSetFormat* pIconSet = static_cast<const ScIconSetFormat*>(pEntry);
    CPPUNIT_ASSERT_EQUAL(eType, pIconSet->GetIconSetData()->eIconSetType);
}

void testCustomIconSetsXLSX_Impl(const ScDocument& rDoc, SCCOL nCol, SCROW nRow,
                                 ScIconSetType eType, sal_Int32 nIndex)
{
    ScConditionalFormat* pFormat = rDoc.GetCondFormat(nCol, 1, 1);
    CPPUNIT_ASSERT(pFormat);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pFormat->size());
    const ScFormatEntry* pEntry = pFormat->GetEntry(0);
    CPPUNIT_ASSERT(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScFormatEntry::Type::Iconset, pEntry->GetType());
    const ScIconSetFormat* pIconSet = static_cast<const ScIconSetFormat*>(pEntry);
    std::unique_ptr<ScIconSetInfo> pInfo(pIconSet->GetIconSetInfo(ScAddress(nCol, nRow, 1)));
    if (nIndex == -1)
        CPPUNIT_ASSERT(!pInfo);
    else
    {
        CPPUNIT_ASSERT(pInfo);
        CPPUNIT_ASSERT_EQUAL(nIndex, pInfo->nIconIndex);
        CPPUNIT_ASSERT_EQUAL(eType, pInfo->eIconSetType);
    }
}
}

void ScFiltersTest::testComplexIconSetsXLSX()
{
    createScDoc("xlsx/complex_icon_set.xlsx");

    ScDocument* pDoc = getScDoc();
    CPPUNIT_ASSERT_EQUAL(size_t(3), pDoc->GetCondFormList(0)->size());
    testComplexIconSetsXLSX_Impl(*pDoc, 1, IconSet_3Triangles);
    testComplexIconSetsXLSX_Impl(*pDoc, 3, IconSet_3Stars);
    testComplexIconSetsXLSX_Impl(*pDoc, 5, IconSet_5Boxes);

    CPPUNIT_ASSERT_EQUAL(size_t(2), pDoc->GetCondFormList(1)->size());
    testCustomIconSetsXLSX_Impl(*pDoc, 1, 1, IconSet_3ArrowsGray, 0);
    testCustomIconSetsXLSX_Impl(*pDoc, 1, 2, IconSet_3ArrowsGray, -1);
    testCustomIconSetsXLSX_Impl(*pDoc, 1, 3, IconSet_3Arrows, 1);
    testCustomIconSetsXLSX_Impl(*pDoc, 1, 4, IconSet_3ArrowsGray, -1);
    testCustomIconSetsXLSX_Impl(*pDoc, 1, 5, IconSet_3Arrows, 2);

    testCustomIconSetsXLSX_Impl(*pDoc, 3, 1, IconSet_4RedToBlack, 3);
    testCustomIconSetsXLSX_Impl(*pDoc, 3, 2, IconSet_3TrafficLights1, 1);
    testCustomIconSetsXLSX_Impl(*pDoc, 3, 3, IconSet_3Arrows, 2);
}

void ScFiltersTest::testTdf101104()
{
    createScDoc("ods/tdf101104.ods");

    ScDocument* pDoc = getScDoc();
    CPPUNIT_ASSERT_EQUAL(size_t(2), pDoc->GetCondFormList(0)->size());

    ScConditionalFormat* pFormat = pDoc->GetCondFormat(1, 1, 0);
    CPPUNIT_ASSERT(pFormat);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pFormat->size());
    const ScFormatEntry* pEntry = pFormat->GetEntry(0);
    CPPUNIT_ASSERT(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScFormatEntry::Type::Iconset, pEntry->GetType());
    const ScIconSetFormat* pIconSet = static_cast<const ScIconSetFormat*>(pEntry);

    for (size_t i = 1; i < 10; ++i)
    {
        std::unique_ptr<ScIconSetInfo> pInfo(pIconSet->GetIconSetInfo(ScAddress(1, i, 0)));

        // Without the fix in place, this test would have failed here
        CPPUNIT_ASSERT(pInfo);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), pInfo->nIconIndex);
        CPPUNIT_ASSERT_EQUAL(IconSet_3Arrows, pInfo->eIconSetType);
    }
}

void ScFiltersTest::testTdf64401()
{
    createScDoc("ods/tdf64401.ods");

    ScDocument* pDoc = getScDoc();
    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetCondFormList(0)->size());

    ScConditionalFormat* pFormat = pDoc->GetCondFormat(0, 0, 0);
    CPPUNIT_ASSERT(pFormat);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pFormat->size());
    const ScFormatEntry* pEntry = pFormat->GetEntry(0);
    CPPUNIT_ASSERT(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScFormatEntry::Type::Iconset, pEntry->GetType());
    const ScIconSetFormat* pIconSet = static_cast<const ScIconSetFormat*>(pEntry);

    for (size_t i = 0; i < 10; ++i)
    {
        sal_Int32 nIndex = 0;
        if (i >= 7) // B5 = 8
            nIndex = 2;
        else if (i >= 3) // B4 = 4
            nIndex = 1;

        std::unique_ptr<ScIconSetInfo> pInfo(pIconSet->GetIconSetInfo(ScAddress(0, i, 0)));
        CPPUNIT_ASSERT(pInfo);
        CPPUNIT_ASSERT_EQUAL(nIndex, pInfo->nIconIndex);
        CPPUNIT_ASSERT_EQUAL(IconSet_3Arrows, pInfo->eIconSetType);
    }

    // Update values in B4 and B5
    pDoc->SetValue(ScAddress(1, 3, 0), 2.0);
    pDoc->SetValue(ScAddress(1, 4, 0), 9.0);

    for (size_t i = 0; i < 10; ++i)
    {
        sal_Int32 nIndex = 0;
        if (i >= 8) // B5 = 9
            nIndex = 2;
        else if (i >= 1) // B4 = 2
            nIndex = 1;

        std::unique_ptr<ScIconSetInfo> pInfo(pIconSet->GetIconSetInfo(ScAddress(0, i, 0)));
        CPPUNIT_ASSERT(pInfo);
        CPPUNIT_ASSERT_EQUAL(nIndex, pInfo->nIconIndex);
        CPPUNIT_ASSERT_EQUAL(IconSet_3Arrows, pInfo->eIconSetType);
    }
}

void ScFiltersTest::testCondFormatParentXLSX()
{
    createScDoc("xlsx/cond_parent.xlsx");

    ScDocument* pDoc = getScDoc();
    const SfxItemSet* pCondSet = pDoc->GetCondResult(2, 5, 0);
    const ScPatternAttr* pPattern = pDoc->GetPattern(2, 5, 0);
    const SfxPoolItem& rPoolItem = pPattern->GetItem(ATTR_VER_JUSTIFY, pCondSet);
    const SvxVerJustifyItem& rVerJustify = static_cast<const SvxVerJustifyItem&>(rPoolItem);
    CPPUNIT_ASSERT_EQUAL(SvxCellVerJustify::Top, rVerJustify.GetValue());
}

void ScFiltersTest::testColorScaleNumWithRefXLSX()
{
    createScDoc("xlsx/colorscale_num_with_ref.xlsx");

    ScDocument* pDoc = getScDoc();
    ScConditionalFormatList* pList = pDoc->GetCondFormList(0);
    CPPUNIT_ASSERT(pList);

    CPPUNIT_ASSERT_EQUAL(size_t(1), pList->size());

    ScConditionalFormat* pFormat = pList->begin()->get();
    CPPUNIT_ASSERT(pFormat);

    CPPUNIT_ASSERT_EQUAL(size_t(1), pFormat->size());
    const ScFormatEntry* pEntry = pFormat->GetEntry(0);
    CPPUNIT_ASSERT(pEntry);

    CPPUNIT_ASSERT_EQUAL(ScFormatEntry::Type::Colorscale, pEntry->GetType());

    const ScColorScaleFormat* pColorScale = dynamic_cast<const ScColorScaleFormat*>(pEntry);
    CPPUNIT_ASSERT(pColorScale);

    const ScColorScaleEntry* pColorScaleEntry = pColorScale->GetEntry(1);
    CPPUNIT_ASSERT_EQUAL(OUString("=$A$1"),
                         pColorScaleEntry->GetFormula(formula::FormulaGrammar::GRAM_NATIVE));
}

void ScFiltersTest::testOrcusODSStyleInterface()
{
    ScDocument aDoc;
    OUString aFullUrl = m_directories.getURLFromSrc(u"sc/qa/unit/data/xml/styles.xml");

    /* This loop below trims file:// from the start because orcus doesn't accept such a URL */
    OUString aValidPath;
    osl::FileBase::getSystemPathFromFileURL(aFullUrl, aValidPath);

    ScOrcusFilters* pOrcus = ScFormatFilter::Get().GetOrcusFilters();
    CPPUNIT_ASSERT(pOrcus);

    pOrcus->importODS_Styles(aDoc, aValidPath);
    ScStyleSheetPool* pStyleSheetPool = aDoc.GetStyleSheetPool();

    /* Test cases for Style "Name1"
     * Has Border and Fill.
     */
    ScStyleSheet* pStyleSheet = pStyleSheetPool->FindCaseIns("Name1", SfxStyleFamily::Para);
    const SfxPoolItem* pItem = nullptr;

    CPPUNIT_ASSERT_MESSAGE("Style Name1 : Doesn't have Attribute background, but it should have.",
                           pStyleSheet->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem));
    const SvxBrushItem* pBackground = static_cast<const SvxBrushItem*>(pItem);
    CPPUNIT_ASSERT_EQUAL(Color(254, 255, 204), pBackground->GetColor());

    CPPUNIT_ASSERT_MESSAGE("Style Name1 : Doesn't have Attribute border, but it should have.",
                           pStyleSheet->GetItemSet().HasItem(ATTR_BORDER, &pItem));
    const SvxBoxItem* pBoxItem = static_cast<const SvxBoxItem*>(pItem);
    CPPUNIT_ASSERT_EQUAL(Color(255, 204, 18), pBoxItem->GetLeft()->GetColor());
    CPPUNIT_ASSERT_EQUAL(Color(255, 204, 18), pBoxItem->GetRight()->GetColor());
    CPPUNIT_ASSERT_EQUAL(Color(255, 204, 18), pBoxItem->GetTop()->GetColor());
    CPPUNIT_ASSERT_EQUAL(Color(255, 204, 18), pBoxItem->GetBottom()->GetColor());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::DOTTED, pBoxItem->GetLeft()->GetBorderLineStyle());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::DOTTED, pBoxItem->GetRight()->GetBorderLineStyle());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::DOTTED, pBoxItem->GetTop()->GetBorderLineStyle());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::DOTTED, pBoxItem->GetBottom()->GetBorderLineStyle());
    ASSERT_DOUBLES_EQUAL_MESSAGE("Error with left width", 1, pBoxItem->GetLeft()->GetWidth());
    ASSERT_DOUBLES_EQUAL_MESSAGE("Error with right width", 1, pBoxItem->GetRight()->GetWidth());
    ASSERT_DOUBLES_EQUAL_MESSAGE("Error with top width", 1, pBoxItem->GetTop()->GetWidth());
    ASSERT_DOUBLES_EQUAL_MESSAGE("Error with bottom width", 1, pBoxItem->GetBottom()->GetWidth());

    CPPUNIT_ASSERT_MESSAGE("Style Name1 : Has Attribute Protection, but it shouldn't.",
                           !pStyleSheet->GetItemSet().HasItem(ATTR_PROTECTION, &pItem));
    CPPUNIT_ASSERT_MESSAGE("Style Name1 : Has Attribute font, but it shouldn't.",
                           !pStyleSheet->GetItemSet().HasItem(ATTR_FONT, &pItem));
    CPPUNIT_ASSERT_MESSAGE("Style Name1 : Has Attribute number format, but it shouldn't.",
                           !pStyleSheet->GetItemSet().HasItem(ATTR_VALUE_FORMAT, &pItem));

    /* Test for Style "Name2"
     * Has 4 sided borders + Diagonal borders.
     */
    pStyleSheet = pStyleSheetPool->FindCaseIns("Name2", SfxStyleFamily::Para);

    CPPUNIT_ASSERT_MESSAGE("Style Name2 : Doesn't have Attribute background, but it should have.",
                           pStyleSheet->GetItemSet().HasItem(ATTR_BORDER, &pItem));

    pBoxItem = static_cast<const SvxBoxItem*>(pItem);
    CPPUNIT_ASSERT_EQUAL(Color(0, 0, 0), pBoxItem->GetLeft()->GetColor());
    CPPUNIT_ASSERT_EQUAL(Color(255, 0, 0), pBoxItem->GetRight()->GetColor());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, pBoxItem->GetLeft()->GetBorderLineStyle());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::DOTTED, pBoxItem->GetRight()->GetBorderLineStyle());
    ASSERT_DOUBLES_EQUAL_MESSAGE("Error with left width", 0, pBoxItem->GetLeft()->GetWidth());
    ASSERT_DOUBLES_EQUAL_MESSAGE("Error with right width", 14, pBoxItem->GetRight()->GetWidth());

    CPPUNIT_ASSERT_MESSAGE(
        "Style Name2 : Doesn't have Attribute diagonal(tl-br) border, but it should have.",
        pStyleSheet->GetItemSet().HasItem(ATTR_BORDER_TLBR, &pItem));

    const SvxLineItem* pTLBR = static_cast<const SvxLineItem*>(pItem);
    CPPUNIT_ASSERT_EQUAL(Color(18, 0, 0), pTLBR->GetLine()->GetColor());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::DASH_DOT, pTLBR->GetLine()->GetBorderLineStyle());
    ASSERT_DOUBLES_EQUAL_MESSAGE("Error with diagonal tl-br width", 14,
                                 pTLBR->GetLine()->GetWidth());

    CPPUNIT_ASSERT_MESSAGE(
        "Style Name2 : Doesn't have Attribute diagonal(bl-tr) border, but it should have.",
        pStyleSheet->GetItemSet().HasItem(ATTR_BORDER_BLTR, &pItem));

    const SvxLineItem* pBLTR = static_cast<const SvxLineItem*>(pItem);
    CPPUNIT_ASSERT_EQUAL(Color(255, 204, 238), pBLTR->GetLine()->GetColor());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::DASHED, pBLTR->GetLine()->GetBorderLineStyle());
    ASSERT_DOUBLES_EQUAL_MESSAGE("Error with diagonal tl-br width", 34,
                                 pBLTR->GetLine()->GetWidth());

    CPPUNIT_ASSERT_MESSAGE("Style Name2 : Has Attribute background, but it shouldn't.",
                           !pStyleSheet->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem));
    CPPUNIT_ASSERT_MESSAGE("Style Name2 : Has Attribute font, but it shouldn't.",
                           !pStyleSheet->GetItemSet().HasItem(ATTR_FONT, &pItem));
    CPPUNIT_ASSERT_MESSAGE("Style Name2 : Has Attribute number format, but it shouldn't.",
                           !pStyleSheet->GetItemSet().HasItem(ATTR_VALUE_FORMAT, &pItem));

    /* Test for Style "Name3"
     * Hidden, protected and content is printed.
     */
    pStyleSheet = pStyleSheetPool->FindCaseIns("Name3", SfxStyleFamily::Para);
    CPPUNIT_ASSERT_MESSAGE("Style Name3 : Doesn't have Attribute Protection, but it should have.",
                           pStyleSheet->GetItemSet().HasItem(ATTR_PROTECTION, &pItem));

    CPPUNIT_ASSERT_MESSAGE("Style Name 3 : Error with Protection attribute.",
                           bool(ScProtectionAttr(true, false, true, true) == *pItem));

    /* Test for Style "Name4"
     * Hidden, protected and content is printed.
     */
    pStyleSheet = pStyleSheetPool->FindCaseIns("Name4", SfxStyleFamily::Para);
    CPPUNIT_ASSERT_MESSAGE("Style Name4 : Doesn't have Attribute Protection, but it should have.",
                           pStyleSheet->GetItemSet().HasItem(ATTR_PROTECTION, &pItem));

    CPPUNIT_ASSERT_MESSAGE("Style Name 4 : Error with Protection attribute.",
                           bool(ScProtectionAttr(true, true, false, false) == *pItem));

    /* Test for Style "Name5"
     * Hidden, protected and content is printed.
     */
    pStyleSheet = pStyleSheetPool->FindCaseIns("Name5", SfxStyleFamily::Para);
    CPPUNIT_ASSERT_MESSAGE("Style Name5 : Doesn't have Attribute Protection, but it should have.",
                           pStyleSheet->GetItemSet().HasItem(ATTR_PROTECTION, &pItem));

    CPPUNIT_ASSERT_MESSAGE("Style Name5 : Error with Protection attribute.",
                           bool(ScProtectionAttr(false, false, false, true) == *pItem));

    CPPUNIT_ASSERT_MESSAGE("Style Name5 : Has Attribute Border, but it shouldn't.",
                           !pStyleSheet->GetItemSet().HasItem(ATTR_BORDER, &pItem));
    CPPUNIT_ASSERT_MESSAGE("Style Name5 : Has Attribute background, but it shouldn't.",
                           !pStyleSheet->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem));
    CPPUNIT_ASSERT_MESSAGE("Style Name5 : Has Attribute font, but it shouldn't.",
                           !pStyleSheet->GetItemSet().HasItem(ATTR_FONT, &pItem));
    CPPUNIT_ASSERT_MESSAGE("Style Name5 : Has Attribute number format, but it shouldn't.",
                           !pStyleSheet->GetItemSet().HasItem(ATTR_VALUE_FORMAT, &pItem));

    /* Test for Style "Name6"
     * Has Font name, posture, weight, color, height
     */
    pStyleSheet = pStyleSheetPool->FindCaseIns("Name6", SfxStyleFamily::Para);
    CPPUNIT_ASSERT_MESSAGE("Style Name6 : Doesn't have Attribute Font, but it should have.",
                           pStyleSheet->GetItemSet().HasItem(ATTR_FONT, &pItem));

    const SvxFontItem* pFontItem = static_cast<const SvxFontItem*>(pItem);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Style Name6 :Error with Font name", OUString("Liberation Sans"),
                                 pFontItem->GetStyleName());

    CPPUNIT_ASSERT_MESSAGE("Style Name6 : Doesn't have Attribute Font Height, but it should have.",
                           pStyleSheet->GetItemSet().HasItem(ATTR_FONT_HEIGHT, &pItem));

    const SvxFontHeightItem* pFontHeightItem = static_cast<const SvxFontHeightItem*>(pItem);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Style Name6 :Error with Font Height",
                                 static_cast<sal_uInt32>(480), pFontHeightItem->GetHeight());

    CPPUNIT_ASSERT_MESSAGE("Style Name6 : Doesn't have Attribute Font Posture, but it should have.",
                           pStyleSheet->GetItemSet().HasItem(ATTR_FONT_POSTURE, &pItem));

    const SvxPostureItem* pFontPostureItem = static_cast<const SvxPostureItem*>(pItem);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Style Name6 :Error with Font Posture", ITALIC_NORMAL,
                                 pFontPostureItem->GetPosture());

    CPPUNIT_ASSERT_MESSAGE("Style Name6 : Doesn't have Attribute Font Weight, but it should have.",
                           pStyleSheet->GetItemSet().HasItem(ATTR_FONT_WEIGHT, &pItem));

    const SvxWeightItem* pFontWeightItem = static_cast<const SvxWeightItem*>(pItem);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Style Name6 :Error with Font Weight", WEIGHT_BOLD,
                                 pFontWeightItem->GetWeight());

    CPPUNIT_ASSERT_MESSAGE("Style Name6 : Doesn't have Attribute Font Color, but it should have.",
                           pStyleSheet->GetItemSet().HasItem(ATTR_FONT_COLOR, &pItem));

    const SvxColorItem* pFontColorItem = static_cast<const SvxColorItem*>(pItem);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Style Name6 :Error with Font Color", Color(128, 128, 128),
                                 pFontColorItem->GetValue());

    CPPUNIT_ASSERT_MESSAGE("Style Name6 : Doesn't have Attribute Underline, but it should have.",
                           pStyleSheet->GetItemSet().HasItem(ATTR_FONT_UNDERLINE, &pItem));

    const SvxUnderlineItem* pUnderlineItem = static_cast<const SvxUnderlineItem*>(pItem);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Style Name6 :Error with Font Underline Style", LINESTYLE_SINGLE,
                                 pUnderlineItem->GetLineStyle());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Style Name6 :Error with Font Underline Color",
                                 Color(128, 128, 128), pUnderlineItem->GetColor());

    /* Test for Style Name "7"
     * Has strikethrough single
     */
    pStyleSheet = pStyleSheetPool->FindCaseIns("Name7", SfxStyleFamily::Para);
    CPPUNIT_ASSERT_MESSAGE("Style Name7 : Doesn't have Attribute Strikeout, but it should have.",
                           pStyleSheet->GetItemSet().HasItem(ATTR_FONT_CROSSEDOUT, &pItem));

    const SvxCrossedOutItem* pCrossedOutItem = static_cast<const SvxCrossedOutItem*>(pItem);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Style Name7 :Error with Strikeout", STRIKEOUT_SINGLE,
                                 pCrossedOutItem->GetStrikeout());

    /* Test for Style Name "8"
     * Has strikethrough bold
     */
    pStyleSheet = pStyleSheetPool->FindCaseIns("Name8", SfxStyleFamily::Para);
    CPPUNIT_ASSERT_MESSAGE("Style Name8 : Doesn't have Attribute Strikeout, but it should have.",
                           pStyleSheet->GetItemSet().HasItem(ATTR_FONT_CROSSEDOUT, &pItem));

    pCrossedOutItem = static_cast<const SvxCrossedOutItem*>(pItem);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Style Name7 :Error with Strikeout", STRIKEOUT_BOLD,
                                 pCrossedOutItem->GetStrikeout());

    /* Test for Style Name "9"
     * Has strikethrough slash
     */
    pStyleSheet = pStyleSheetPool->FindCaseIns("Name9", SfxStyleFamily::Para);
    CPPUNIT_ASSERT_MESSAGE("Style Name9 : Doesn't have Attribute Strikeout, but it should have.",
                           pStyleSheet->GetItemSet().HasItem(ATTR_FONT_CROSSEDOUT, &pItem));

    pCrossedOutItem = static_cast<const SvxCrossedOutItem*>(pItem);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Style Name9 :Error with Strikeout", STRIKEOUT_SLASH,
                                 pCrossedOutItem->GetStrikeout());

    /* Test for Style Name "10"
     * Has ver align, and hor align
     */

    pStyleSheet = pStyleSheetPool->FindCaseIns("Name10", SfxStyleFamily::Para);
    CPPUNIT_ASSERT_MESSAGE("Style Name10 : Doesn't have Attribute hor justify, but it should have.",
                           pStyleSheet->GetItemSet().HasItem(ATTR_HOR_JUSTIFY, &pItem));

    const SvxHorJustifyItem* pHorJustify = static_cast<const SvxHorJustifyItem*>(pItem);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Style Name10 :Error with hor justify", SvxCellHorJustify::Right,
                                 pHorJustify->GetValue());

    pStyleSheet = pStyleSheetPool->FindCaseIns("Name10", SfxStyleFamily::Para);
    CPPUNIT_ASSERT_MESSAGE("Style Name10 : Doesn't have Attribute ver justify, but it should have.",
                           pStyleSheet->GetItemSet().HasItem(ATTR_VER_JUSTIFY, &pItem));

    const SvxVerJustifyItem* pVerJustify = static_cast<const SvxVerJustifyItem*>(pItem);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Style Name10 :Error with ver justify", SvxCellVerJustify::Center,
                                 pVerJustify->GetValue());

    auto checkFontWeight = [pStyleSheetPool](const OUString& rName, FontWeight eExpected) {
        ScStyleSheet* pStyle = pStyleSheetPool->FindCaseIns(rName, SfxStyleFamily::Para);
        CPPUNIT_ASSERT(pStyle);

        const SfxPoolItem* p = nullptr;

        {
            std::ostringstream os;
            os << "Style named '" << rName << "' does not have a font weight attribute.";
            CPPUNIT_ASSERT_MESSAGE(os.str(), pStyle->GetItemSet().HasItem(ATTR_FONT_WEIGHT, &p));
        }

        const SvxWeightItem* pWeight = static_cast<const SvxWeightItem*>(p);
        FontWeight eActual = pWeight->GetWeight();
        {
            std::ostringstream os;
            os << "Wrong font weight value for style named '" << rName
               << "': expected=" << eExpected << "; actual=" << eActual;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(os.str(), eExpected, eActual);
        }
    };

    checkFontWeight("Accent", WEIGHT_BOLD);
    checkFontWeight("Accent 1", WEIGHT_BOLD); // inherits from 'Accent'
    checkFontWeight("Accent 2", WEIGHT_BOLD); // inherits from 'Accent'
    checkFontWeight("Accent 3", WEIGHT_BOLD); // inherits from 'Accent'
}

void ScFiltersTest::testLiteralInFormulaXLS()
{
    createScDoc("xls/shared-string/literal-in-formula.xls");

    ScDocument* pDoc = getScDoc();
    pDoc->CalcAll();

    CPPUNIT_ASSERT_EQUAL(2.0, pDoc->GetValue(ScAddress(0, 0, 0)));
}

void ScFiltersTest::testFormulaDependency()
{
    createScDoc("ods/dependencyTree.ods");

    ScDocument* pDoc = getScDoc();

    // check if formula in A1 changes value
    double nVal = pDoc->GetValue(0, 0, 0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(nVal, 1.0, 1e-10);
    pDoc->SetValue(0, 1, 0, 0.0);
    nVal = pDoc->GetValue(0, 0, 0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(nVal, 2.0, 1e-10);

    // check that the number format is implicitly inherited
    // CPPUNIT_ASSERT_EQUAL(pDoc->GetString(0,4,0), rDoc.GetString(0,5,0));
}

void ScFiltersTest::testTdf151046()
{
    createScDoc("ods/tdf151046.ods");

    ScDocument* pDoc = getScDoc();

    CPPUNIT_ASSERT_EQUAL(1.0, pDoc->GetValue(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL(1.0, pDoc->GetValue(0, 1, 0));

    // Without the fix in place, this test would have failed with
    // - Expected: 1
    // - Actual  : 0
    CPPUNIT_ASSERT_EQUAL(1.0, pDoc->GetValue(0, 2, 0));
}

void ScFiltersTest::testImportCrashes()
{
    testImportCrash("ods/tdf149752.ods");
    testImportCrash("ods/tdf122643.ods");
    testImportCrash("ods/tdf132278.ods");
    testImportCrash("xlsx/tdf130959.xlsx");
    testImportCrash("ods/tdf129410.ods");
    testImportCrash("ods/tdf138507.ods");
    testImportCrash("xlsx/tdf131380.xlsx");
    testImportCrash("ods/tdf139782.ods");
    testImportCrash("ods/tdf136551.ods");
    testImportCrash("ods/tdf90391.ods");
    testImportCrash("xlsx/tdf121887.xlsx"); // 'Maximum number of rows per sheet' warning
    testImportCrash("xlsm/tdf111974.xlsm");
    testImportCrash("ods/tdf149679.ods");
}

void ScFiltersTest::testTdf129681()
{
    createScDoc("ods/tdf129681.ods");

    ScDocument* pDoc = getScDoc();

    CPPUNIT_ASSERT_EQUAL(OUString("Lamb"), pDoc->GetString(ScAddress(4, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("Beef"), pDoc->GetString(ScAddress(4, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("Pork"), pDoc->GetString(ScAddress(4, 4, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("Goat"), pDoc->GetString(ScAddress(4, 5, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("Goat"), pDoc->GetString(ScAddress(4, 6, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("#VALUE!"), pDoc->GetString(ScAddress(4, 7, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("#VALUE!"), pDoc->GetString(ScAddress(4, 8, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("Deer"), pDoc->GetString(ScAddress(4, 9, 0)));

    CPPUNIT_ASSERT_EQUAL(OUString("1"), pDoc->GetString(ScAddress(6, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("2"), pDoc->GetString(ScAddress(6, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("3"), pDoc->GetString(ScAddress(6, 4, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("5"), pDoc->GetString(ScAddress(6, 5, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("8"), pDoc->GetString(ScAddress(6, 6, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("#VALUE!"), pDoc->GetString(ScAddress(6, 7, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("#VALUE!"), pDoc->GetString(ScAddress(6, 8, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("#VALUE!"), pDoc->GetString(ScAddress(6, 9, 0)));
}

void ScFiltersTest::testTdf149484()
{
    createScDoc("ods/tdf149484.ods");
    ScDocument* pDoc = getScDoc();

    // Without the fix in place, this test would have failed with
    // - Expected: -TRUE-
    // - Actual  : TRUE
    CPPUNIT_ASSERT_EQUAL(OUString("-TRUE-"), pDoc->GetString(0, 2, 0));
}

void ScFiltersTest::testEscapedUnicodeXLSX()
{
    createScDoc("xlsx/escape-unicode.xlsx");
    ScDocument* pDoc = getScDoc();

    // Without the fix, there would be "_x000D_" after every new-line char.
    CPPUNIT_ASSERT_EQUAL(OUString("Line 1\nLine 2\nLine 3\nLine 4"), pDoc->GetString(1, 1, 0));
}

void ScFiltersTest::testTdf144758_DBDataDefaultOrientation()
{
    createScDoc("fods/tdf144758-dbdata-no-orientation.fods");
    ScDocument* pDoc = getScDoc();
    ScDBData* pAnonDBData = pDoc->GetAnonymousDBData(0);
    CPPUNIT_ASSERT(pAnonDBData);

    ScSortParam aSortParam;
    pAnonDBData->GetSortParam(aSortParam);

    // Without the fix, the default value for bByRow (in absence of 'table:orientation' attribute
    // in 'table:database-range' element) was false
    CPPUNIT_ASSERT(aSortParam.bByRow);
}

void ScFiltersTest::testSharedFormulaXLS()
{
    createScDoc("xls/shared-formula/basic.xls");
    ScDocument* pDoc = getScDoc();
    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 18; ++i)
    {
        double fVal = pDoc->GetValue(ScAddress(1, i, 0));
        double fCheck = i * 10.0;
        CPPUNIT_ASSERT_EQUAL(fCheck, fVal);
    }

    ScFormulaCell* pCell = pDoc->GetFormulaCell(ScAddress(1, 18, 0));
    CPPUNIT_ASSERT_MESSAGE("This should be a formula cell.", pCell);
    ScFormulaCellGroupRef xGroup = pCell->GetCellGroup();
    CPPUNIT_ASSERT_MESSAGE("This cell should be a part of a cell group.", xGroup);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Incorrect group geometry.", SCROW(1),
                                 xGroup->mpTopCell->aPos.Row());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Incorrect group geometry.", SCROW(18), xGroup->mnLength);

    // The following file contains shared formula whose range is inaccurate.
    // Excel can easily mess up shared formula ranges, so we need to be able
    // to handle these wrong ranges that Excel stores.

    createScDoc("xls/shared-formula/gap.xls");
    pDoc = getScDoc();
    pDoc->CalcAll();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula.", OUString("=A1*20"), pDoc->GetFormula(1, 0, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula.", OUString("=A2*20"), pDoc->GetFormula(1, 1, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula.", OUString("=A3*20"), pDoc->GetFormula(1, 2, 0));

    // There is an intentional gap at row 4.

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula.", OUString("=A5*20"), pDoc->GetFormula(1, 4, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula.", OUString("=A6*20"), pDoc->GetFormula(1, 5, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula.", OUString("=A7*20"), pDoc->GetFormula(1, 6, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong formula.", OUString("=A8*20"), pDoc->GetFormula(1, 7, 0));

    // We re-group formula cells on load. Let's check that as well.

    ScFormulaCell* pFC = pDoc->GetFormulaCell(ScAddress(1, 0, 0));
    CPPUNIT_ASSERT_MESSAGE("Failed to fetch formula cell.", pFC);
    CPPUNIT_ASSERT_MESSAGE("This should be the top cell in formula group.", pFC->IsSharedTop());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(3), pFC->GetSharedLength());

    pFC = pDoc->GetFormulaCell(ScAddress(1, 4, 0));
    CPPUNIT_ASSERT_MESSAGE("Failed to fetch formula cell.", pFC);
    CPPUNIT_ASSERT_MESSAGE("This should be the top cell in formula group.", pFC->IsSharedTop());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(4), pFC->GetSharedLength());
}

void ScFiltersTest::testSharedFormulaXLSX()
{
    createScDoc("xlsx/shared-formula/basic.xlsx");
    ScDocument* pDoc = getScDoc();
    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();
    // Check the results of formula cells in the shared formula range.
    for (SCROW i = 1; i <= 18; ++i)
    {
        double fVal = pDoc->GetValue(ScAddress(1, i, 0));
        double fCheck = i * 10.0;
        CPPUNIT_ASSERT_EQUAL(fCheck, fVal);
    }

    ScFormulaCell* pCell = pDoc->GetFormulaCell(ScAddress(1, 18, 0));
    CPPUNIT_ASSERT_MESSAGE("This should be a formula cell.", pCell);
    ScFormulaCellGroupRef xGroup = pCell->GetCellGroup();
    CPPUNIT_ASSERT_MESSAGE("This cell should be a part of a cell group.", xGroup);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Incorrect group geometry.", SCROW(1),
                                 xGroup->mpTopCell->aPos.Row());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Incorrect group geometry.", SCROW(18), xGroup->mnLength);
}

void ScFiltersTest::testSharedFormulaRefUpdateXLSX()
{
    createScDoc("xlsx/shared-formula/refupdate.xlsx");
    ScDocument* pDoc = getScDoc();
    sc::AutoCalcSwitch aACSwitch(*pDoc, true); // turn auto calc on.
    pDoc->DeleteRow(ScRange(0, 4, 0, pDoc->MaxCol(), 4, 0)); // delete row 5.

    struct TestCase
    {
        SCCOL nCol;
        SCROW nRow;
        SCTAB nTab;
        const char* pExpectedFormula;
        const char* pErrorMsg;
    };

    TestCase aCases[4] = {
        { 1, 0, 0, "=B29+1", "Wrong formula in B1" },
        { 2, 0, 0, "=C29+1", "Wrong formula in C1" },
        { 3, 0, 0, "=D29+1", "Wrong formula in D1" },
        { 4, 0, 0, "=E29+1", "Wrong formula in E1" },
    };

    for (size_t nIdx = 0; nIdx < 4; ++nIdx)
    {
        TestCase& rCase = aCases[nIdx];
        CPPUNIT_ASSERT_EQUAL_MESSAGE(rCase.pErrorMsg,
                                     OUString::createFromAscii(rCase.pExpectedFormula),
                                     pDoc->GetFormula(rCase.nCol, rCase.nRow, rCase.nTab));
    }
}

void ScFiltersTest::testSheetNamesXLSX()
{
    createScDoc("xlsx/sheet-names.xlsx");
    ScDocument* pDoc = getScDoc();

    std::vector<OUString> aTabNames = pDoc->GetAllTableNames();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The document should have 5 sheets in total.", size_t(5),
                                 aTabNames.size());
    CPPUNIT_ASSERT_EQUAL(OUString("S&P"), aTabNames[0]);
    CPPUNIT_ASSERT_EQUAL(OUString("Sam's Club"), aTabNames[1]);
    CPPUNIT_ASSERT_EQUAL(OUString("\"The Sheet\""), aTabNames[2]);
    CPPUNIT_ASSERT_EQUAL(OUString("A<B"), aTabNames[3]);
    CPPUNIT_ASSERT_EQUAL(OUString("C>D"), aTabNames[4]);
}

void ScFiltersTest::testTdf150599()
{
    createScDoc("dif/tdf150599.dif");
    ScDocument* pDoc = getScDoc();

    // Without the fix in place, this test would have failed with
    // - Expected: 1
    // - Actual  : #IND:?
    CPPUNIT_ASSERT_EQUAL(OUString("1"), pDoc->GetString(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("32"), pDoc->GetString(ScAddress(31, 0, 0)));
}

void ScFiltersTest::testCommentSize()
{
    createScDoc("ods/comment.ods");
    ScDocument* pDoc = getScDoc();

    ScAddress aPos(0, 0, 0);
    ScPostIt* pNote = pDoc->GetNote(aPos);
    CPPUNIT_ASSERT(pNote);

    pNote->ShowCaption(aPos, true);
    CPPUNIT_ASSERT(pNote->IsCaptionShown());

    SdrCaptionObj* pCaption = pNote->GetCaption();
    CPPUNIT_ASSERT(pCaption);

    const tools::Rectangle& rOldRect = pCaption->GetLogicRect();
    CPPUNIT_ASSERT_EQUAL(tools::Long(2899), rOldRect.getOpenWidth());
    CPPUNIT_ASSERT_EQUAL(tools::Long(939), rOldRect.getOpenHeight());

    pNote->SetText(aPos, "first\nsecond\nthird");

    const tools::Rectangle& rNewRect = pCaption->GetLogicRect();
    CPPUNIT_ASSERT_EQUAL(rOldRect.getOpenWidth(), rNewRect.getOpenWidth());
    CPPUNIT_ASSERT_EQUAL(tools::Long(1605), rNewRect.getOpenHeight());

    pDoc->GetUndoManager()->Undo();

    CPPUNIT_ASSERT_EQUAL(rOldRect.getOpenWidth(), pCaption->GetLogicRect().getOpenWidth());
    CPPUNIT_ASSERT_EQUAL(rOldRect.getOpenHeight(), pCaption->GetLogicRect().getOpenHeight());
}

static void testEnhancedProtectionImpl(const ScDocument& rDoc)
{
    const ScTableProtection* pProt = rDoc.GetTabProtection(0);

    CPPUNIT_ASSERT(pProt);

    CPPUNIT_ASSERT(!pProt->isBlockEditable(ScRange(0, 0, 0, 0, 0, 0))); // locked
    CPPUNIT_ASSERT(pProt->isBlockEditable(ScRange(0, 1, 0, 0, 1, 0))); // editable without password
    CPPUNIT_ASSERT(pProt->isBlockEditable(ScRange(0, 2, 0, 0, 2, 0))); // editable without password
    CPPUNIT_ASSERT(
        !pProt->isBlockEditable(ScRange(0, 3, 0, 0, 3, 0))); // editable with password "foo"
    CPPUNIT_ASSERT(!pProt->isBlockEditable(ScRange(0, 4, 0, 0, 4, 0))); // editable with descriptor
    CPPUNIT_ASSERT(!pProt->isBlockEditable(
        ScRange(0, 5, 0, 0, 5, 0))); // editable with descriptor and password "foo"
    CPPUNIT_ASSERT(
        pProt->isBlockEditable(ScRange(0, 1, 0, 0, 2, 0))); // union of two different editables
    CPPUNIT_ASSERT(
        !pProt->isBlockEditable(ScRange(0, 0, 0, 0, 1, 0))); // union of locked and editable
    CPPUNIT_ASSERT(!pProt->isBlockEditable(
        ScRange(0, 2, 0, 0, 3, 0))); // union of editable and password editable
}

void ScFiltersTest::testEnhancedProtectionXLS()
{
    createScDoc("xls/enhanced-protection.xls");
    ScDocument* pDoc = getScDoc();

    testEnhancedProtectionImpl(*pDoc);
}

void ScFiltersTest::testEnhancedProtectionXLSX()
{
    createScDoc("xlsx/enhanced-protection.xlsx");
    ScDocument* pDoc = getScDoc();

    testEnhancedProtectionImpl(*pDoc);
}

void ScFiltersTest::testSortWithSharedFormulasODS()
{
    createScDoc("ods/shared-formula/sort-crash.ods");
    ScDocument* pDoc = getScDoc();

    // E2:E10 should be shared.
    const ScFormulaCell* pFC = pDoc->GetFormulaCell(ScAddress(4, 1, 0));
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(1), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(9), pFC->GetSharedLength());

    // E12:E17 should be shared.
    pFC = pDoc->GetFormulaCell(ScAddress(4, 11, 0));
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(11), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(6), pFC->GetSharedLength());

    // Set A1:E17 as an anonymous database range to sheet, or else Calc would
    // refuse to sort the range.
    std::unique_ptr<ScDBData> pDBData(
        new ScDBData(STR_DB_LOCAL_NONAME, 0, 0, 0, 4, 16, true, true));
    pDoc->SetAnonymousDBData(0, std::move(pDBData));

    // Sort ascending by Column E.

    ScSortParam aSortData;
    aSortData.nCol1 = 0;
    aSortData.nCol2 = 4;
    aSortData.nRow1 = 0;
    aSortData.nRow2 = 16;
    aSortData.bHasHeader = true;
    aSortData.maKeyState[0].bDoSort = true;
    aSortData.maKeyState[0].nField = 4;
    aSortData.maKeyState[0].bAscending = true;

    // Do the sorting.  This should not crash.
    ScDocShell* pDocSh = getScDocShell();
    ScDBDocFunc aFunc(*pDocSh);
    bool bSorted = aFunc.Sort(0, aSortData, true, true, true);
    CPPUNIT_ASSERT(bSorted);

    // After the sort, E2:E16 should be shared.
    pFC = pDoc->GetFormulaCell(ScAddress(4, 1, 0));
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(1), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(15), pFC->GetSharedLength());
}

namespace
{
void testSortWithSheetExternalReferencesODS_Impl(ScDocShell& rDocSh, SCROW nRow1, SCROW nRow2,
                                                 bool bCheckRelativeInSheet)
{
    ScDocument& rDoc = rDocSh.GetDocument();

    // Check the original data is there.
    for (SCROW nRow = nRow1 + 1; nRow <= nRow2; ++nRow)
    {
        double const aCheck[] = { 1, 2, 3, 4, 5 };
        CPPUNIT_ASSERT_EQUAL(aCheck[nRow - nRow1 - 1], rDoc.GetValue(ScAddress(0, nRow, 0)));
    }
    for (SCROW nRow = nRow1 + 1; nRow <= nRow2; ++nRow)
    {
        for (SCCOL nCol = 1; nCol <= 3; ++nCol)
        {
            double const aCheck[] = { 1, 12, 123, 1234, 12345 };
            CPPUNIT_ASSERT_EQUAL(aCheck[nRow - nRow1 - 1], rDoc.GetValue(ScAddress(nCol, nRow, 0)));
        }
    }

    // Set as an anonymous database range to sort.
    std::unique_ptr<ScDBData> pDBData(
        new ScDBData(STR_DB_LOCAL_NONAME, 0, 0, nRow1, 3, nRow2, true, true));
    rDoc.SetAnonymousDBData(0, std::move(pDBData));

    // Sort descending by Column A.
    ScSortParam aSortData;
    aSortData.nCol1 = 0;
    aSortData.nCol2 = 3;
    aSortData.nRow1 = nRow1;
    aSortData.nRow2 = nRow2;
    aSortData.bHasHeader = true;
    aSortData.maKeyState[0].bDoSort = true;
    aSortData.maKeyState[0].nField = 0;
    aSortData.maKeyState[0].bAscending = false;

    // Do the sorting.
    ScDBDocFunc aFunc(rDocSh);
    bool bSorted = aFunc.Sort(0, aSortData, true, true, true);
    CPPUNIT_ASSERT(bSorted);
    rDoc.CalcAll();

    // Check the sort and that all sheet references and external references are
    // adjusted to point to the original location.
    for (SCROW nRow = nRow1 + 1; nRow <= nRow2; ++nRow)
    {
        double const aCheck[] = { 5, 4, 3, 2, 1 };
        CPPUNIT_ASSERT_EQUAL(aCheck[nRow - nRow1 - 1], rDoc.GetValue(ScAddress(0, nRow, 0)));
    }
    // The last column (D) are in-sheet relative references.
    SCCOL nEndCol = (bCheckRelativeInSheet ? 3 : 2);
    for (SCROW nRow = nRow1 + 1; nRow <= nRow2; ++nRow)
    {
        for (SCCOL nCol = 1; nCol <= nEndCol; ++nCol)
        {
            double const aCheck[] = { 12345, 1234, 123, 12, 1 };
            CPPUNIT_ASSERT_EQUAL(aCheck[nRow - nRow1 - 1], rDoc.GetValue(ScAddress(nCol, nRow, 0)));
        }
    }
}
}

// https://bugs.freedesktop.org/attachment.cgi?id=100089 from fdo#77018
// mentioned also in fdo#79441
// Document contains cached external references.
void ScFiltersTest::testSortWithSheetExternalReferencesODS()
{
    // We reset the SortRefUpdate value back to the original in tearDown().
    ScInputOptions aInputOption = SC_MOD()->GetInputOptions();
    bool bUpdateReferenceOnSort = aInputOption.GetSortRefUpdate();

    createScDoc("ods/sort-with-sheet-external-references.ods");
    ScDocument* pDoc = getScDoc();
    sc::AutoCalcSwitch aACSwitch(*pDoc, true); // turn auto calc on.
    pDoc->CalcAll();

    // The complete relative test only works with UpdateReferenceOnSort==true,
    // but the internal and external sheet references have to work in both
    // modes.

    aInputOption.SetSortRefUpdate(true);
    SC_MOD()->SetInputOptions(aInputOption);

    // Sort A15:D20 with relative row references. UpdateReferenceOnSort==true
    // With in-sheet relative references.
    ScDocShell* pDocSh = getScDocShell();
    testSortWithSheetExternalReferencesODS_Impl(*pDocSh, 14, 19, true);

    // Undo sort with relative references to perform same sort.
    pDoc->GetUndoManager()->Undo();
    pDoc->CalcAll();

    aInputOption.SetSortRefUpdate(false);
    SC_MOD()->SetInputOptions(aInputOption);

    // Sort A15:D20 with relative row references. UpdateReferenceOnSort==false
    // Without in-sheet relative references.
    testSortWithSheetExternalReferencesODS_Impl(*pDocSh, 14, 19, false);

    // Undo sort with relative references to perform new sort.
    pDoc->GetUndoManager()->Undo();
    pDoc->CalcAll();

    // Sort with absolute references has to work in both UpdateReferenceOnSort
    // modes.

    aInputOption.SetSortRefUpdate(true);
    SC_MOD()->SetInputOptions(aInputOption);

    // Sort A23:D28 with absolute row references. UpdateReferenceOnSort==true
    // With in-sheet relative references.
    testSortWithSheetExternalReferencesODS_Impl(*pDocSh, 22, 27, true);

    // Undo sort with absolute references to perform same sort.
    pDoc->GetUndoManager()->Undo();
    pDoc->CalcAll();

    aInputOption.SetSortRefUpdate(false);
    SC_MOD()->SetInputOptions(aInputOption);

    // Sort A23:D28 with absolute row references. UpdateReferenceOnSort==false
    // With in-sheet relative references.
    testSortWithSheetExternalReferencesODS_Impl(*pDocSh, 22, 27, true);

    if (bUpdateReferenceOnSort != aInputOption.GetSortRefUpdate())
    {
        aInputOption.SetSortRefUpdate(bUpdateReferenceOnSort);
        SC_MOD()->SetInputOptions(aInputOption);
    }
}

void ScFiltersTest::testSortWithFormattingXLS()
{
    createScDoc("xls/tdf129127.xls");
    ScDocument* pDoc = getScDoc();

    // Set as an anonymous database range to sort.
    std::unique_ptr<ScDBData> pDBData(
        new ScDBData(STR_DB_LOCAL_NONAME, 0, 0, 0, 4, 9, false, false));
    pDoc->SetAnonymousDBData(0, std::move(pDBData));

    // Sort ascending by Row 1
    ScSortParam aSortData;
    aSortData.nCol1 = 0;
    aSortData.nCol2 = 4;
    aSortData.nRow1 = 0;
    aSortData.nRow2 = 9;
    aSortData.bHasHeader = false;
    aSortData.bByRow = false;
    aSortData.maKeyState[0].bDoSort = true;
    aSortData.maKeyState[0].nField = 0;
    aSortData.maKeyState[0].bAscending = true;

    // Do the sorting.
    ScDocShell* pDocSh = getScDocShell();
    ScDBDocFunc aFunc(*pDocSh);
    // Without the fix, sort would crash.
    bool bSorted = aFunc.Sort(0, aSortData, true, true, true);
    CPPUNIT_ASSERT(bSorted);
}

// just needs to not crash on recalc
void ScFiltersTest::testForcepoint107()
{
    createScDoc("xlsx/forcepoint107.xlsx");
    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();
}

ScFiltersTest::ScFiltersTest()
    : ScModelTestBase("sc/qa/unit/data")
{
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScFiltersTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
