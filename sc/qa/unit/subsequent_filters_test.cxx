/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <unotest/filters-test.hxx>
#include <osl/file.hxx>
#include <config_fonts.h>

#include <svl/numformat.hxx>
#include <svl/zformat.hxx>

#include <svx/svdpage.hxx>
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
#include <validat.hxx>
#include <userdat.hxx>
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
#include <sortparam.hxx>

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

class ScFiltersTest
    : public test::FiltersTest
    , public ScBootstrapFixture
{
public:
    ScFiltersTest();

    virtual bool load( const OUString &rFilter, const OUString &rURL,
        const OUString &rUserData, SfxFilterFlags nFilterFlags,
        SotClipboardFormatId nClipboardID, unsigned int nFilterVersion) override;

    virtual void setUp() override;
    virtual void tearDown() override;

    //ods, xls, xlsx filter tests
    void testCondFormatOperatorsSameRangeXLSX();
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

    void testRowHeightODS();
    void testRichTextContentODS();

    void testTdf122643();
    void testTdf132278();
    void testTdf130959();
    void testTdf129410();
    void testTdf138507();
    void testTdf131380();
    void testTdf139782();
    void testTdf136551();
    void testTdf129681();
    void testTdf111974XLSM();
    void testEscapedUnicodeXLSX();
    void testTdf144758_DBDataDefaultOrientation();

    CPPUNIT_TEST_SUITE(ScFiltersTest);
    CPPUNIT_TEST(testCondFormatOperatorsSameRangeXLSX);
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
    CPPUNIT_TEST(testRichTextContentODS);

    //disable testPassword on MacOSX due to problems with libsqlite3
    //also crashes on DragonFly due to problems with nss/nspr headers
    CPPUNIT_TEST(testPasswordWrongSHA);
    CPPUNIT_TEST(testPasswordOld);
    CPPUNIT_TEST(testPasswordNew);

    CPPUNIT_TEST(testTdf122643);
    CPPUNIT_TEST(testTdf132278);
    CPPUNIT_TEST(testTdf130959);
    CPPUNIT_TEST(testTdf129410);
    CPPUNIT_TEST(testTdf138507);
    CPPUNIT_TEST(testTdf131380);
    CPPUNIT_TEST(testTdf139782);
    CPPUNIT_TEST(testTdf136551);
    CPPUNIT_TEST(testTdf129681);
    CPPUNIT_TEST(testTdf111974XLSM);
    CPPUNIT_TEST(testEscapedUnicodeXLSX);
    CPPUNIT_TEST(testTdf144758_DBDataDefaultOrientation);

    CPPUNIT_TEST_SUITE_END();

private:
    void testImportCrash(std::u16string_view rFileName, sal_Int32 nFormat);
    void testPassword_Impl(std::u16string_view rFileNameBase);

    uno::Reference<uno::XInterface> m_xCalcComponent;
};

bool ScFiltersTest::load(const OUString &rFilter, const OUString &rURL,
    const OUString &rUserData, SfxFilterFlags nFilterFlags,
        SotClipboardFormatId nClipboardID, unsigned int nFilterVersion)
{
    ScDocShellRef xDocShRef = ScBootstrapFixture::load( rURL, rFilter, rUserData,
        OUString(), nFilterFlags, nClipboardID, nFilterVersion);
    bool bLoaded = xDocShRef.is();
    //reference counting of ScDocShellRef is very confused.
    if (bLoaded)
        xDocShRef->DoClose();
    return bLoaded;
}

namespace {

void testRangeNameImpl(const ScDocument& rDoc)
{
    //check one range data per sheet and one global more detailed
    //add some more checks here
    ScRangeData* pRangeData = rDoc.GetRangeName()->findByUpperName(OUString("GLOBAL1"));
    CPPUNIT_ASSERT_MESSAGE("range name Global1 not found", pRangeData);
    double aValue = rDoc.GetValue(1,0,0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("range name Global1 should reference Sheet1.A1", 1.0, aValue);
    pRangeData = rDoc.GetRangeName(0)->findByUpperName(OUString("LOCAL1"));
    CPPUNIT_ASSERT_MESSAGE("range name Sheet1.Local1 not found", pRangeData);
    aValue = rDoc.GetValue(1,2,0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("range name Sheet1.Local1 should reference Sheet1.A3", 3.0, aValue);
    pRangeData = rDoc.GetRangeName(1)->findByUpperName(OUString("LOCAL2"));
    CPPUNIT_ASSERT_MESSAGE("range name Sheet2.Local2 not found", pRangeData);
    aValue = rDoc.GetValue(1,1,1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("range name Sheet2.Local2 should reference Sheet2.A2", 7.0, aValue);
    //check for correct results for the remaining formulas
    aValue = rDoc.GetValue(1,1,0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("=global2 should be 2", 2.0, aValue);
    aValue = rDoc.GetValue(1,3,0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("=local2 should be 4", 4.0, aValue);
    aValue = rDoc.GetValue(2,0,0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("=SUM(global3) should be 10", 10.0, aValue);
    aValue = rDoc.GetValue(1,0,1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("range name Sheet2.local1 should reference Sheet1.A5", 5.0, aValue);
    // Test if Global5 ( which depends on Global6 ) is evaluated
    aValue = rDoc.GetValue(0,5,1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("formula Global5 should reference Global6 ( which is evaluated as local1 )", 5.0, aValue);
}

}

void ScFiltersTest::testCondFormatOperatorsSameRangeXLSX()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf139928.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to load tdf139928.xlsx", xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();

    ScConditionalFormat* pFormat = rDoc.GetCondFormat(0, 0, 0);
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

    xDocSh->DoClose();
}

void ScFiltersTest::testTdf119292()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf119292.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to load tdf119292.xlsx", xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();
    auto* pDev = rDoc.GetRefDevice();
    Size aMarkSize(4, 6);
    Color aArrowFillCol(COL_LIGHTRED);

    // check the points of the polygon if the text is rotated 90 degrees
    tools::Rectangle aMarkRect1(0, 0, 45, 3);
    tools::Polygon aPoly90degrees = SvxFont::DrawArrow(*pDev, aMarkRect1, aMarkSize, aArrowFillCol, true, true);
    Point aPoly90Pos1 = aPoly90degrees.GetPoint(0);
    Point aPoly90Pos2 = aPoly90degrees.GetPoint(1);
    Point aPoly90Pos3 = aPoly90degrees.GetPoint(2);
    CPPUNIT_ASSERT_EQUAL(Point(19,3),aPoly90Pos1);
    CPPUNIT_ASSERT_EQUAL(Point(22,0),aPoly90Pos2);
    CPPUNIT_ASSERT_EQUAL(Point(25,3),aPoly90Pos3);

    // check the points of the polygon if the text is rotated 270 degrees
    tools::Rectangle aMarkRect2(89, 62, 134, 57);
    tools::Polygon aPoly270degrees = SvxFont::DrawArrow(*pDev, aMarkRect2, aMarkSize, aArrowFillCol, false, true);
    Point aPoly270Pos1 = aPoly270degrees.GetPoint(0);
    Point aPoly270Pos2 = aPoly270degrees.GetPoint(1);
    Point aPoly270Pos3 = aPoly270degrees.GetPoint(2);
    CPPUNIT_ASSERT_EQUAL(Point(108,54),aPoly270Pos1);
    CPPUNIT_ASSERT_EQUAL(Point(111,57),aPoly270Pos2);
    CPPUNIT_ASSERT_EQUAL(Point(114,54),aPoly270Pos3);

    xDocSh->DoClose();
}

void ScFiltersTest::testTdf48731()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf48731.", FORMAT_CSV);
    CPPUNIT_ASSERT_MESSAGE("Failed to load document", xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();

    CPPUNIT_ASSERT_EQUAL(OUString("'"), rDoc.GetString(1, 1, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("''"), rDoc.GetString(1, 2, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("'''"), rDoc.GetString(1, 3, 0));

    // Without the fix in place, this test would have failed with
    // - Expected: '3
    // - Actual  : 3
    CPPUNIT_ASSERT_EQUAL(OUString("'3"), rDoc.GetString(1, 4, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("'word"), rDoc.GetString(1, 5, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("'mword"), rDoc.GetString(1, 6, 0));

    CPPUNIT_ASSERT_EQUAL(OUString("'"), rDoc.GetString(1, 9, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("''"), rDoc.GetString(1, 10, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("'''"), rDoc.GetString(1, 11, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("'3"), rDoc.GetString(1, 12, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("'word"), rDoc.GetString(1, 13, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("'mword"), rDoc.GetString(1, 14, 0));

    xDocSh->DoClose();
}

void ScFiltersTest::testCondFormatFormulaIsXLSX()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf113013.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to load tdf113013.xlsx", xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();

    // "Formula is" condition
    ScConditionalFormat* pFormatB1 = rDoc.GetCondFormat(1, 0, 0);
    CPPUNIT_ASSERT(pFormatB1);
    ScConditionalFormat* pFormatA2 = rDoc.GetCondFormat(0, 1, 0);
    CPPUNIT_ASSERT(pFormatA2);

    ScRefCellValue aCellB1(rDoc, ScAddress(1, 0, 0));
    OUString aCellStyleB1 = pFormatB1->GetCellStyle(aCellB1, ScAddress(1, 0, 0));
    CPPUNIT_ASSERT(!aCellStyleB1.isEmpty());

    ScRefCellValue aCellA2(rDoc, ScAddress(0, 1, 0));
    OUString aCellStyleA2 = pFormatA2->GetCellStyle(aCellA2, ScAddress(0, 1, 0));
    CPPUNIT_ASSERT(!aCellStyleA2.isEmpty());

    xDocSh->DoClose();
}

void ScFiltersTest::testCondFormatBeginsAndEndsWithXLSX()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf120749.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to load tdf120749.xlsx", xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();

    // begins with and ends with conditions
    ScConditionalFormat* pFormatA1 = rDoc.GetCondFormat(0, 0, 0);
    CPPUNIT_ASSERT(pFormatA1);
    ScConditionalFormat* pFormatA2 = rDoc.GetCondFormat(0, 1, 0);
    CPPUNIT_ASSERT(pFormatA2);
    ScConditionalFormat* pFormatA3 = rDoc.GetCondFormat(0, 2, 0);
    CPPUNIT_ASSERT(pFormatA3);
    ScConditionalFormat* pFormatA4 = rDoc.GetCondFormat(0, 3, 0);
    CPPUNIT_ASSERT(pFormatA4);

    ScRefCellValue aCellA1(rDoc, ScAddress(0, 0, 0));
    OUString aCellStyleA1 = pFormatA1->GetCellStyle(aCellA1, ScAddress(0, 0, 0));
    CPPUNIT_ASSERT(!aCellStyleA1.isEmpty());

    ScRefCellValue aCellA2(rDoc, ScAddress(0, 1, 0));
    OUString aCellStyleA2 = pFormatA2->GetCellStyle(aCellA2, ScAddress(0, 1, 0));
    CPPUNIT_ASSERT(!aCellStyleA2.isEmpty());

    ScRefCellValue aCellA3(rDoc, ScAddress(0, 2, 0));
    OUString aCellStyleA3 = pFormatA3->GetCellStyle(aCellA3, ScAddress(0, 2, 0));
    CPPUNIT_ASSERT(!aCellStyleA3.isEmpty());

    ScRefCellValue aCellA4(rDoc, ScAddress(0, 3, 0));
    OUString aCellStyleA4 = pFormatA4->GetCellStyle(aCellA4, ScAddress(0, 3, 0));
    CPPUNIT_ASSERT(!aCellStyleA4.isEmpty());

    xDocSh->DoClose();
}

void ScFiltersTest::testExtCondFormatXLSX()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf122102.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to load tdf122102.xlsx", xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();

    // contains text and not contains text conditions
    ScConditionalFormat* pFormatA1 = rDoc.GetCondFormat(0, 0, 0);
    CPPUNIT_ASSERT(pFormatA1);
    ScConditionalFormat* pFormatA2 = rDoc.GetCondFormat(0, 1, 0);
    CPPUNIT_ASSERT(pFormatA2);
    ScConditionalFormat* pFormatA3 = rDoc.GetCondFormat(0, 2, 0);
    CPPUNIT_ASSERT(pFormatA3);
    ScConditionalFormat* pFormatA4 = rDoc.GetCondFormat(0, 3, 0);
    CPPUNIT_ASSERT(pFormatA4);

    ScRefCellValue aCellA1(rDoc, ScAddress(0, 0, 0));
    OUString aCellStyleA1 = pFormatA1->GetCellStyle(aCellA1, ScAddress(0, 0, 0));
    CPPUNIT_ASSERT(!aCellStyleA1.isEmpty());

    ScRefCellValue aCellA2(rDoc, ScAddress(0, 1, 0));
    OUString aCellStyleA2 = pFormatA2->GetCellStyle(aCellA2, ScAddress(0, 1, 0));
    CPPUNIT_ASSERT(!aCellStyleA2.isEmpty());

    ScRefCellValue aCellA3(rDoc, ScAddress(0, 2, 0));
    OUString aCellStyleA3 = pFormatA3->GetCellStyle(aCellA3, ScAddress(0, 2, 0));
    CPPUNIT_ASSERT(!aCellStyleA3.isEmpty());

    ScRefCellValue aCellA4(rDoc, ScAddress(0, 3, 0));
    OUString aCellStyleA4 = pFormatA4->GetCellStyle(aCellA4, ScAddress(0, 3, 0));
    CPPUNIT_ASSERT(!aCellStyleA4.isEmpty());

    xDocSh->DoClose();
}

void ScFiltersTest::testUpdateCircleInMergedCellODS()
{
    ScDocShellRef xDocSh = loadDoc(u"updateCircleInMergedCell.", FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load updateCircleInMergedCell.ods", xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();
    rDoc.EnableChangeReadOnly(true);

    ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
    SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("draw page for sheet 1 should exist.", pPage);

    // There should be four circle objects!
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), pPage->GetObjCount());

    ScCellMergeOption aCellMergeOption(0,0,1,1); // A1:B2
    aCellMergeOption.maTabs.insert(0);
    xDocSh->GetDocFunc().MergeCells(aCellMergeOption, false, true, true, false);

    // There should be a circle object!
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pPage->GetObjCount());

    xDocSh->GetDocFunc().UnmergeCells(aCellMergeOption, true, nullptr);

    // There should be four circle objects!
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), pPage->GetObjCount());

    xDocSh->DoClose();
}

void ScFiltersTest::testDeleteCircleInMergedCellODS()
{
    ScDocShellRef xDocSh = loadDoc(u"deleteCircleInMergedCell.", FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load deleteCircleInMergedCell.ods", xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();

    ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
    SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("draw page for sheet 1 should exist.", pPage);

    // There should be a circle object!
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pPage->GetObjCount());

    ScRefCellValue aMergedCell;
    ScAddress aPosMergedCell(0, 0, 0);
    aMergedCell.assign(rDoc, aPosMergedCell);

    // The value of merged cell change to 6.
    aMergedCell.mfValue = 6;

    // Check that the data is valid.(True if the value = 6)
    const ScValidationData* pData = rDoc.GetValidationEntry(1);
    bool bValidA1 = pData->IsDataValid(aMergedCell, aPosMergedCell);
    // if valid, delete circle.
    if (bValidA1)
        ScDetectiveFunc(rDoc, 0).DeleteCirclesAt(aPosMergedCell.Col(), aPosMergedCell.Row());

    // There should not be a circle object!
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), pPage->GetObjCount());

    xDocSh->DoClose();
}

void ScFiltersTest::testBasicCellContentODS()
{
    ScDocShellRef xDocSh = loadDoc(u"basic-cell-content.", FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load basic-cell-content.ods", xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();
    OUString aStr = rDoc.GetString(1, 1, 0); // B2
    CPPUNIT_ASSERT_EQUAL(OUString("LibreOffice Calc"), aStr);
    double fVal = rDoc.GetValue(1, 2, 0); // B3
    CPPUNIT_ASSERT_EQUAL(12345.0, fVal);
    aStr = rDoc.GetString(1, 3, 0); // B4
    CPPUNIT_ASSERT_EQUAL(OUString("A < B"), aStr);

    // Numeric value of 0.
    ScRefCellValue aCell;
    aCell.assign(rDoc, ScAddress(1,4,0)); // B5
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "This cell must be numeric.", CELLTYPE_VALUE, aCell.meType);
    CPPUNIT_ASSERT_EQUAL(0.0, aCell.mfValue);

    xDocSh->DoClose();
}

void ScFiltersTest::testBooleanFormatXLSX()
{
    ScDocShellRef xDocSh = loadDoc(u"check-boolean.", FORMAT_XLSX);
    ScDocument& rDoc = xDocSh->GetDocument();
    SvNumberFormatter* pNumFormatter = rDoc.GetFormatTable();
    // Saved as >"TRUE";"TRUE";"FALSE"< but reading converted back to >BOOLEAN<
    const OUString aBooleanTypeStr = "BOOLEAN";

    CPPUNIT_ASSERT_MESSAGE("Failed to load check-boolean.xlsx", xDocSh.is());

    for (SCROW i = 0; i <= 1; i++)
    {
        sal_uInt32 nNumberFormat = rDoc.GetNumberFormat(0, i, 0);
        const SvNumberformat* pNumberFormat = pNumFormatter->GetEntry(nNumberFormat);
        const OUString& rFormatStr = pNumberFormat->GetFormatstring();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Number format != boolean", aBooleanTypeStr, rFormatStr);
    }

    xDocSh->DoClose();
}

void ScFiltersTest::testTdf143809()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf143809.", FORMAT_ODS);

    ScDocument& rDoc = xDocSh->GetDocument();

    OUString aFormula = rDoc.GetFormula(0, 0, 0);
    CPPUNIT_ASSERT_EQUAL(OUString("=SUMPRODUCT(IFERROR(CEILING.MATH(DURATIONS,300),0))"), aFormula);

    // Without the fix in place, this test would have failed with
    // - Expected: 53700
    // - Actual  : Err:502
    CPPUNIT_ASSERT_EQUAL(OUString("53700"), rDoc.GetString(0, 0, 0));

    aFormula = rDoc.GetFormula(0, 1, 0);
    CPPUNIT_ASSERT_EQUAL(
            OUString("=SUMPRODUCT(IFERROR(CEILING(SUMIFS(DURATIONS,IDS,IDS),300)/COUNTIFS(IDS,IDS),0))"), aFormula);
    CPPUNIT_ASSERT_EQUAL(OUString("51900"), rDoc.GetString(0, 1, 0));

    xDocSh->DoClose();
}

void ScFiltersTest::testTdf76310()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf76310.", FORMAT_ODS);

    ScDocument& rDoc = xDocSh->GetDocument();

    OUString aFormula = rDoc.GetFormula(0, 0, 0);
    // Without the fix in place, this test would have failed with
    // - Expected: =1
    // +
    // 2
    // - Actual  : =1 + 2
    CPPUNIT_ASSERT_EQUAL(OUString("=1\n+\n2"), aFormula);
    ASSERT_DOUBLES_EQUAL(3.0, rDoc.GetValue(0, 0, 0));

    xDocSh->DoClose();
}

void ScFiltersTest::testRangeNameXLS()
{
    ScDocShellRef xDocSh = loadDoc(u"named-ranges-global.", FORMAT_XLS);
    xDocSh->DoHardRecalc();

    ScDocument& rDoc = xDocSh->GetDocument();
    testRangeNameImpl(rDoc);

    OUString aCSVPath;
    createCSVPath( "rangeExp_Sheet2.", aCSVPath );
    // fdo#44587
    testFile( aCSVPath, rDoc, 1);

    xDocSh->DoClose();
}

void ScFiltersTest::testRangeNameLocalXLS()
{
    ScDocShellRef xDocSh = loadDoc(u"named-ranges-local.", FORMAT_XLS);
    xDocSh->DoHardRecalc();

    ScDocument& rDoc = xDocSh->GetDocument();
    ScRangeName* pRangeName = rDoc.GetRangeName(0);
    CPPUNIT_ASSERT(pRangeName);
    CPPUNIT_ASSERT_EQUAL(size_t(2), pRangeName->size());

    OUString aFormula = rDoc.GetFormula(3, 11, 0);
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(local_name2)"), aFormula);
    ASSERT_DOUBLES_EQUAL(14.0, rDoc.GetValue(3, 11, 0));

    aFormula = rDoc.GetFormula(6, 4, 0);
    CPPUNIT_ASSERT_EQUAL(OUString("=local_name1"), aFormula);

    xDocSh->DoClose();
}

void ScFiltersTest::testRangeNameXLSX()
{
    ScDocShellRef xDocSh = loadDoc(u"named-ranges-global.", FORMAT_XLSX);
    xDocSh->DoHardRecalc();

    ScDocument& rDoc = xDocSh->GetDocument();
    testRangeNameImpl(rDoc);

    xDocSh->DoClose();
}

void ScFiltersTest::testHyperlinksXLSX()
{
    ScDocShellRef xDocSh = loadDoc(u"hyperlinks.", FORMAT_XLSX);
    ScDocument& rDoc = xDocSh->GetDocument();

    CPPUNIT_ASSERT_EQUAL(OUString("10:ABC10"), rDoc.GetString(ScAddress(0,1,0)));
    CPPUNIT_ASSERT_EQUAL(OUString("10:ABC11"), rDoc.GetString(ScAddress(0,2,0)));
    CPPUNIT_ASSERT_EQUAL(OUString("10:ABC12"), rDoc.GetString(ScAddress(0,3,0)));

    xDocSh->DoClose();
}

void ScFiltersTest::testHardRecalcODS()
{
    ScDocShellRef xDocSh = loadDoc(u"hard-recalc.", FORMAT_ODS);
    xDocSh->DoHardRecalc();

    CPPUNIT_ASSERT_MESSAGE("Failed to load hard-recalc.*", xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();
    OUString aCSVFileName;

    //test hard recalc: document has an incorrect cached formula result
    //hard recalc should have updated to the correct result
    createCSVPath("hard-recalc.", aCSVFileName);
    testFile(aCSVFileName, rDoc, 0);

    xDocSh->DoClose();
}

void ScFiltersTest::testFunctionsODS()
{
    ScDocShellRef xDocSh = loadDoc(u"functions.", FORMAT_ODS);
    xDocSh->DoHardRecalc();

    CPPUNIT_ASSERT_MESSAGE("Failed to load functions.*", xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();
    OUString aCSVFileName;

    //test logical functions
    createCSVPath("logical-functions.", aCSVFileName);
    testFile(aCSVFileName, rDoc, 0);
    //test spreadsheet functions
    createCSVPath("spreadsheet-functions.", aCSVFileName);
    testFile(aCSVFileName, rDoc, 1);
    //test mathematical functions
    createCSVPath("mathematical-functions.", aCSVFileName);
    testFile(aCSVFileName, rDoc, 2, StringType::PureString);
    //test information functions
    createCSVPath("information-functions.", aCSVFileName);
    testFile(aCSVFileName, rDoc, 3);
    // text functions
    createCSVPath("text-functions.", aCSVFileName);
    testFile(aCSVFileName, rDoc, 4, StringType::PureString);
    // statistical functions
    createCSVPath("statistical-functions.", aCSVFileName);
    testFile(aCSVFileName, rDoc, 5);
    // financial functions
    createCSVPath("financial-functions.", aCSVFileName);
    testFile(aCSVFileName, rDoc, 6);

    xDocSh->DoClose();

    xDocSh = loadDoc(u"database-functions.", FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load functions.*", xDocSh.is());
    xDocSh->DoHardRecalc();
    ScDocument& rDoc2 = xDocSh->GetDocument();

    createCSVPath("database-functions.", aCSVFileName);
    testFile(aCSVFileName, rDoc2, 0);

    xDocSh->DoClose();

    xDocSh = loadDoc(u"date-time-functions.", FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load functions.*", xDocSh.is());
    xDocSh->DoHardRecalc();
    ScDocument& rDoc3 = xDocSh->GetDocument();
    createCSVPath("date-time-functions.", aCSVFileName);
    testFile(aCSVFileName, rDoc3, 0, StringType::PureString);

    xDocSh->DoClose();

    // crashes at exit while unloading StarBasic code
    // xDocSh = loadDoc("user-defined-function.", FORMAT_ODS);
    // xDocSh->DoHardRecalc();
    // ScDocument& rDocUserDef = xDocSh->GetDocument();
    // createCSVPath("user-defined-function.", aCSVFileName);
    // testFile(aCSVFileName, rDocUserDef, 0);
}

void ScFiltersTest::testFunctionsExcel2010()
{
    ScDocShellRef xDocSh = loadDoc(u"functions-excel-2010.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to load the document.", xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();
    rDoc.CalcAll(); // perform hard re-calculation.

    testFunctionsExcel2010_Impl(rDoc);

    xDocSh->DoClose();
}

void ScFiltersTest::testCeilingFloorXLSX()
{
    ScDocShellRef xDocSh = loadDoc(u"ceiling-floor.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to load the document.", xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();
    rDoc.CalcAll(); // perform hard re-calculation.

    testCeilingFloor_Impl(rDoc);

    xDocSh->DoClose();
}

void ScFiltersTest::testCachedFormulaResultsODS()
{
    {
        ScDocShellRef xDocSh = loadDoc(u"functions.", FORMAT_ODS);
        CPPUNIT_ASSERT_MESSAGE("Failed to load functions.*", xDocSh.is());

        ScDocument& rDoc = xDocSh->GetDocument();
        OUString aCSVFileName;

        //test cached formula results of logical functions
        createCSVPath("logical-functions.", aCSVFileName);
        testFile(aCSVFileName, rDoc, 0);
        //test cached formula results of spreadsheet functions
        createCSVPath("spreadsheet-functions.", aCSVFileName);
        testFile(aCSVFileName, rDoc, 1);
        //test cached formula results of mathematical functions
        createCSVPath("mathematical-functions.", aCSVFileName);
        testFile(aCSVFileName, rDoc, 2, StringType::PureString);
        //test cached formula results of information functions
        createCSVPath("information-functions.", aCSVFileName);
        testFile(aCSVFileName, rDoc, 3);
        // text functions
        createCSVPath("text-functions.", aCSVFileName);
        testFile(aCSVFileName, rDoc, 4, StringType::PureString);

        xDocSh->DoClose();
    }

    {
        ScDocShellRef xDocSh = loadDoc(u"cachedValue.", FORMAT_ODS);
        CPPUNIT_ASSERT_MESSAGE("Failed to load cachedValue.*", xDocSh.is());

        ScDocument& rDoc = xDocSh->GetDocument();
        OUString aCSVFileName;
        createCSVPath("cachedValue.", aCSVFileName);
        testFile(aCSVFileName, rDoc, 0);

        //we want to me sure that volatile functions are always recalculated
        //regardless of cached results.  if you update the ods file, you must
        //update the values here.
        //if NOW() is recalculated, then it should never equal "01/25/13 01:06 PM"
        OUString sTodayRecalc(rDoc.GetString(0,0,1));

        CPPUNIT_ASSERT("01/25/13 01:06 PM" != sTodayRecalc);

        OUString sTodayRecalcRef(rDoc.GetString(1,0,1));
        CPPUNIT_ASSERT_EQUAL(sTodayRecalc, sTodayRecalcRef);

        // make sure that error values are not being treated as string values
        for(SCCOL nCol = 0; nCol < 4; ++nCol)
        {
            for(SCROW nRow = 0; nRow < 2; ++nRow)
            {
                OUString aFormula = "=ISERROR(" +
                    OUStringChar(static_cast<char>('A'+nCol)) + OUString::number(nRow) +
                    ")";
                rDoc.SetString(nCol, nRow + 2, 2, aFormula);
                CPPUNIT_ASSERT_EQUAL_MESSAGE(OUStringToOString(aFormula, RTL_TEXTENCODING_UTF8).getStr(), OUString("TRUE"), rDoc.GetString(nCol, nRow +2, 2));

                OUString aIsTextFormula = "=ISTEXT(" +
                    OUString::number(static_cast<char>('A'+nCol))+ OUString::number(nRow) +
                    ")";
                rDoc.SetString(nCol, nRow + 4, 2, aIsTextFormula);
                CPPUNIT_ASSERT_EQUAL(OUString("FALSE"), rDoc.GetString(nCol, nRow +4, 2));
            }
        }

        xDocSh->DoClose();
    }
}

void ScFiltersTest::testCachedMatrixFormulaResultsODS()
{
    ScDocShellRef xDocSh = loadDoc(u"matrix.", FORMAT_ODS);

    CPPUNIT_ASSERT_MESSAGE("Failed to load matrix.*", xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();

    //test matrix
    OUString aCSVFileName;
    createCSVPath("matrix.", aCSVFileName);
    testFile(aCSVFileName, rDoc, 0);
    //test matrices with special cases
    createCSVPath("matrix2.", aCSVFileName);
    testFile(aCSVFileName, rDoc, 1);
    createCSVPath("matrix3.", aCSVFileName);
    testFile(aCSVFileName, rDoc, 2);
    //The above testFile() does not catch the below case.
    //If a matrix formula has a matrix reference cell that is intended to have
    //a blank text result, the matrix reference cell is actually saved(export)
    //as a float cell with 0 as the value and an empty <text:p/>.
    //Import works around this by setting these cells as text cells so that
    //the blank text is used for display instead of the number 0.
    //If this is working properly, the following cell should NOT have value data.
    CPPUNIT_ASSERT_EQUAL(OUString(), rDoc.GetString(3,0,2));

    // fdo#59293 with cached value import error formulas require special
    // treatment
    rDoc.SetString(2, 5, 2, "=ISERROR(A6)");
    double nVal = rDoc.GetValue(2,5,2);
    CPPUNIT_ASSERT_EQUAL(1.0, nVal);

    xDocSh->DoClose();
}

void ScFiltersTest::testFormulaDepAcrossSheetsODS()
{
    ScDocShellRef xDocSh = loadDoc(u"formula-across-sheets.", FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load the file.", xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();

    sc::AutoCalcSwitch aACSwitch(rDoc, true); // Make sure auto calc is turned on.

    // Save the original values of A4:C4.
    double fA4 = rDoc.GetValue(ScAddress(0,3,2));
    double fB4 = rDoc.GetValue(ScAddress(1,3,2));
    double fC4 = rDoc.GetValue(ScAddress(2,3,2));

    // Change the value of D4. This should trigger A4:C4 to be recalculated.
    double fD4 = rDoc.GetValue(ScAddress(3,3,2));
    rDoc.SetValue(ScAddress(3,3,2), fD4+1.0);

    CPPUNIT_ASSERT_MESSAGE("The value must differ from the original.", fA4 != rDoc.GetValue(ScAddress(0,3,2)));
    CPPUNIT_ASSERT_MESSAGE("The value must differ from the original.", fB4 != rDoc.GetValue(ScAddress(1,3,2)));
    CPPUNIT_ASSERT_MESSAGE("The value must differ from the original.", fC4 != rDoc.GetValue(ScAddress(2,3,2)));

    xDocSh->DoClose();
}

void ScFiltersTest::testFormulaDepDeleteContentsODS()
{
    ScDocShellRef xDocSh = loadDoc(u"formula-delete-contents.", FORMAT_ODS, true);
    CPPUNIT_ASSERT_MESSAGE("Failed to load the file.", xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();

    sc::UndoSwitch aUndoSwitch(rDoc, true); // Enable undo.
    sc::AutoCalcSwitch aACSwitch(rDoc, true); // Make sure auto calc is turned on.

    CPPUNIT_ASSERT_EQUAL(195.0, rDoc.GetValue(ScAddress(3,15,0))); // formula in D16

    // Delete D2:D5.
    ScDocFunc& rFunc = xDocSh->GetDocFunc();
    ScRange aRange(3,1,0,3,4,0);
    ScMarkData aMark(rDoc.GetSheetLimits());
    aMark.SetMarkArea(aRange);
    aMark.MarkToMulti();
    bool bGood = rFunc.DeleteContents(aMark, InsertDeleteFlags::ALL, true, true);
    CPPUNIT_ASSERT(bGood);
    CPPUNIT_ASSERT_EQUAL(0.0, rDoc.GetValue(ScAddress(3,1,0)));
    CPPUNIT_ASSERT_EQUAL(0.0, rDoc.GetValue(ScAddress(3,2,0)));
    CPPUNIT_ASSERT_EQUAL(0.0, rDoc.GetValue(ScAddress(3,3,0)));
    CPPUNIT_ASSERT_EQUAL(0.0, rDoc.GetValue(ScAddress(3,4,0)));

    CPPUNIT_ASSERT_EQUAL(94.0, rDoc.GetValue(ScAddress(3,15,0))); // formula in D16

    SfxUndoManager* pUndoMgr = rDoc.GetUndoManager();
    CPPUNIT_ASSERT(pUndoMgr);
    pUndoMgr->Undo();
    CPPUNIT_ASSERT_EQUAL(195.0, rDoc.GetValue(ScAddress(3,15,0))); // formula in D16

    xDocSh->DoClose();
}

namespace {

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
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Sheet1: row 6-end should be visible", MAXROW, nRow2);
    if (nFormat == FORMAT_ODS) //excel doesn't support named db ranges
    {
        double aValue = rDoc.GetValue(0,10,1);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Sheet2: A11: formula result is incorrect", 4.0, aValue);
        aValue = rDoc.GetValue(1, 10, 1);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Sheet2: B11: formula result is incorrect", 2.0, aValue);
    }
    double aValue = rDoc.GetValue(3,10,1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Sheet2: D11: formula result is incorrect", 4.0, aValue);
    aValue = rDoc.GetValue(4, 10, 1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Sheet2: E11: formula result is incorrect", 2.0, aValue);
}

}

void ScFiltersTest::testDatabaseRangesODS()
{
    ScDocShellRef xDocSh = loadDoc(u"database.", FORMAT_ODS);
    xDocSh->DoHardRecalc();

    ScDocument& rDoc = xDocSh->GetDocument();

    testDBRanges_Impl(rDoc, FORMAT_ODS);
    xDocSh->DoClose();
}

void ScFiltersTest::testDatabaseRangesXLS()
{
    ScDocShellRef xDocSh = loadDoc(u"database.", FORMAT_XLS);
    xDocSh->DoHardRecalc();

    ScDocument& rDoc = xDocSh->GetDocument();

    testDBRanges_Impl(rDoc, FORMAT_XLS);
    xDocSh->DoClose();
}

void ScFiltersTest::testDatabaseRangesXLSX()
{
    ScDocShellRef xDocSh = loadDoc(u"database.", FORMAT_XLSX);
    xDocSh->DoHardRecalc();

    ScDocument& rDoc = xDocSh->GetDocument();

    testDBRanges_Impl(rDoc, FORMAT_XLSX);
    xDocSh->DoClose();
}

void ScFiltersTest::testFormatsODS()
{
    ScDocShellRef xDocSh = loadDoc(u"formats.", FORMAT_ODS);
    xDocSh->DoHardRecalc();

    ScDocument& rDoc = xDocSh->GetDocument();

    testFormats(this, &rDoc, FORMAT_ODS);
    xDocSh->DoClose();
}

// void ScFiltersTest::testFormatsXLS()
// {
//     ScDocShellRef xDocSh = loadDoc("formats.", FORMAT_XLS);
//     xDocSh->DoHardRecalc();
//
//     ScDocument& rDoc = xDocSh->GetDocument();
//
//     testFormats(this, rDoc, FORMAT_XLS);
//     xDocSh->DoClose();
// }

// void ScFiltersTest::testFormatsXLSX()
// {
//     ScDocShellRef xDocSh = loadDoc("formats.", FORMAT_XLSX);
//     xDocSh->DoHardRecalc();
//
//     ScDocument& rDoc = xDocSh->GetDocument();
//
//     testFormats(this, rDoc, FORMAT_XLSX);
//     xDocSh->DoClose();
// }

void ScFiltersTest::testMatrixODS()
{
    ScDocShellRef xDocSh = loadDoc(u"matrix.", FORMAT_ODS);
    xDocSh->DoHardRecalc();

    ScDocument& rDoc = xDocSh->GetDocument();

    OUString aCSVFileName;
    createCSVPath("matrix.", aCSVFileName);
    testFile(aCSVFileName, rDoc, 0);

    xDocSh->DoClose();
}

void ScFiltersTest::testMatrixXLS()
{
    ScDocShellRef xDocSh = loadDoc(u"matrix.", FORMAT_XLS);
    xDocSh->DoHardRecalc();

    CPPUNIT_ASSERT_MESSAGE("Failed to load matrix.*", xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();

    OUString aCSVFileName;
    createCSVPath("matrix.", aCSVFileName);
    testFile(aCSVFileName, rDoc, 0);

    xDocSh->DoClose();
}

void ScFiltersTest::testDoubleThinBorder()
{
// double-thin borders created with size less than 1.15 where invisible (and subsequently lost) on round-trips.
    ScDocShellRef xDocSh = loadDoc(u"tdf88827_borderDoubleThin.", FORMAT_ODS);

    CPPUNIT_ASSERT_MESSAGE("Failed to load tdf88827_borderDoubleThin.*", xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();

    const editeng::SvxBorderLine* pLeft = nullptr;
    const editeng::SvxBorderLine* pTop = nullptr;
    const editeng::SvxBorderLine* pRight = nullptr;
    const editeng::SvxBorderLine* pBottom = nullptr;

    rDoc.GetBorderLines( 2, 2, 0, &pLeft, &pTop, &pRight, &pBottom );
    CPPUNIT_ASSERT(pTop);
    CPPUNIT_ASSERT(pRight);
    CPPUNIT_ASSERT_EQUAL( SvxBorderLineStyle::DOUBLE_THIN, pRight->GetBorderLineStyle() );
    xDocSh->DoClose();
}

void ScFiltersTest::testBorderODS()
{
    ScDocShellRef xDocSh = loadDoc(u"border.", FORMAT_ODS);

    CPPUNIT_ASSERT_MESSAGE("Failed to load border.*", xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();

    const editeng::SvxBorderLine* pLeft = nullptr;
    const editeng::SvxBorderLine* pTop = nullptr;
    const editeng::SvxBorderLine* pRight = nullptr;
    const editeng::SvxBorderLine* pBottom = nullptr;

    rDoc.GetBorderLines( 0, 1, 0, &pLeft, &pTop, &pRight, &pBottom );
    CPPUNIT_ASSERT(!pLeft);
    CPPUNIT_ASSERT(!pTop);
    CPPUNIT_ASSERT(!pBottom);
    CPPUNIT_ASSERT(pRight);
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, pRight->GetBorderLineStyle());

    rDoc.GetBorderLines( 2, 1, 0, &pLeft, &pTop, &pRight, &pBottom );
    CPPUNIT_ASSERT(!pLeft);
    CPPUNIT_ASSERT(!pTop);
    CPPUNIT_ASSERT(!pBottom);

    CPPUNIT_ASSERT(pRight);
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, pRight->GetBorderLineStyle());
    CPPUNIT_ASSERT_EQUAL(tools::Long(20), pRight->GetWidth());

    rDoc.GetBorderLines( 2, 8, 0, &pLeft, &pTop, &pRight, &pBottom );

    CPPUNIT_ASSERT(pLeft);
    CPPUNIT_ASSERT(pTop);
    CPPUNIT_ASSERT(pBottom);
    CPPUNIT_ASSERT(pRight);
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::SOLID, pRight->GetBorderLineStyle());
    CPPUNIT_ASSERT_EQUAL(tools::Long(5), pRight->GetWidth());
    CPPUNIT_ASSERT_EQUAL(COL_BLUE, pRight->GetColor());

    xDocSh->DoClose();
}

namespace {

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
    Border(sal_Int16 col, sal_Int32 r, sal_Int32 lW, sal_Int32 tW, sal_Int32 rW, sal_Int32 bW, sal_uInt16 lOutW, sal_uInt16 lInW,
        sal_uInt16 lDist, sal_uInt16 tOutW, sal_uInt16 tInW, sal_uInt16 tDist, sal_uInt16 rOutW, sal_uInt16 rInW, sal_uInt16 rDist,
        sal_uInt16 bOutW, sal_uInt16 bInW, sal_uInt16 bDist, sal_Int16 lSt, sal_Int16 tSt, sal_Int16 rSt, sal_Int16 bSt):
    column(col), row(r), leftWidth(lW), topWidth(tW), rightWidth(rW), bottomWidth(bW), lOutWidth(lOutW), lInWidth(lInW), lDistance(lDist),
    tOutWidth(tOutW), tInWidth(tInW), tDistance(tDist), rOutWidth(rOutW), rInWidth(rInW), rDistance(rDist), bOutWidth(bOutW), bInWidth(bInW),
    bDistance(bDist),
    lStyle(static_cast<SvxBorderLineStyle>(lSt)), tStyle(static_cast<SvxBorderLineStyle>(tSt)), rStyle(static_cast<SvxBorderLineStyle>(rSt)), bStyle(static_cast<SvxBorderLineStyle>(bSt)) {};
};

}

void ScFiltersTest::testBordersOoo33()
{
    std::vector<Border> borders;
    borders.emplace_back(1, 1, 22, 22, 22, 22, 1, 1, 20, 1, 1, 20, 1, 1, 20, 1, 1, 20, 3, 3, 3, 3);
    borders.emplace_back(1, 3, 52, 52, 52, 52, 1, 1, 50, 1, 1, 50, 1, 1, 50, 1, 1, 50, 3, 3, 3, 3);
    borders.emplace_back(1, 5, 60, 60, 60, 60, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 3, 3, 3, 3);
    borders.emplace_back(1, 7, 150, 150, 150, 150, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 3, 3, 3, 3);
    borders.emplace_back(1, 9, 71, 71, 71, 71, 20, 1, 50, 20, 1, 50, 20, 1, 50, 20, 1, 50, 3, 3, 3, 3);
    borders.emplace_back(1, 11, 101, 101, 101, 101, 50, 1, 50, 50, 1, 50, 50, 1, 50, 50, 1, 50, 3, 3, 3, 3);
    borders.emplace_back(1, 13, 131, 131, 131, 131, 80, 1, 50, 80, 1, 50, 80, 1, 50, 80, 1, 50, 3, 3, 3, 3);
    borders.emplace_back(1, 15, 120, 120, 120, 120, 50, 20, 50, 50, 20, 50, 50, 20, 50, 50, 20, 50, 3, 3, 3, 3);
    borders.emplace_back(1, 17, 90, 90, 90, 90, 20, 50, 20, 20, 50, 20, 20, 50, 20, 20, 50, 20, 3, 3, 3, 3);
    borders.emplace_back(1, 19, 180, 180, 180, 180, 80, 50, 50, 80, 50, 50, 80, 50, 50, 80, 50, 50, 3, 3, 3, 3);
    borders.emplace_back(1, 21, 180, 180, 180, 180, 50, 80, 50, 50, 80, 50, 50, 80, 50, 50, 80, 50, 3, 3, 3, 3);
    borders.emplace_back(4, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0);
    borders.emplace_back(4, 3, 10, 10, 10, 10, 10, 0, 0, 10, 0, 0, 10, 0, 0, 10, 0, 0, 0, 0, 0, 0);
    borders.emplace_back(4, 5, 20, 20, 20, 20, 20, 0, 0, 20, 0, 0, 20, 0, 0, 20, 0, 0, 0, 0, 0, 0);
    borders.emplace_back(4, 7, 50, 50, 50, 50, 50, 0, 0, 50, 0, 0, 50, 0, 0, 50, 0, 0, 0, 0, 0, 0);
    borders.emplace_back(4, 9, 80, 80, 80, 80, 80, 0, 0, 80, 0, 0, 80, 0, 0, 80, 0, 0, 0, 0, 0, 0);
    borders.emplace_back(4, 11, 100, 100, 100, 100, 100, 0, 0, 100, 0, 0, 100, 0, 0, 100, 0, 0, 0, 0, 0, 0);

    ScDocShellRef xDocSh = loadDoc(u"borders_ooo33.", FORMAT_ODS);

    CPPUNIT_ASSERT_MESSAGE("Failed to load borders_ooo33.*", xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();

    const editeng::SvxBorderLine* pLeft = nullptr;
    const editeng::SvxBorderLine* pTop = nullptr;
    const editeng::SvxBorderLine* pRight = nullptr;
    const editeng::SvxBorderLine* pBottom = nullptr;
    sal_Int16 temp = 0;
    for(sal_Int16 i = 0; i<6; ++i)
    {
        for(sal_Int32 j = 0; j<22; ++j)
        {
            rDoc.GetBorderLines( i, j, 0, &pLeft, &pTop, &pRight, &pBottom );
            if(pLeft!=nullptr && pTop!=nullptr && pRight!=nullptr && pBottom!=nullptr)
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

    xDocSh->DoClose();
}

void ScFiltersTest::testBugFixesODS()
{
    ScDocShellRef xDocSh = loadDoc(u"bug-fixes.", FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load bugFixes.ods", xDocSh.is());

    xDocSh->DoHardRecalc();
    ScDocument& rDoc = xDocSh->GetDocument();

    {
        // fdo#40967
        OUString aCSVFileName;
        createCSVPath("bugFix_Sheet2.", aCSVFileName);
        testFile(aCSVFileName, rDoc, 1);
    }

    {
        // fdo#40426
        ScDBData* pDBData = rDoc.GetDBCollection()->getNamedDBs().findByUpperName("DBRANGE1");
        CPPUNIT_ASSERT(pDBData);
        CPPUNIT_ASSERT(pDBData->HasHeader());
        // no header
        pDBData = rDoc.GetDBCollection()->getNamedDBs().findByUpperName("DBRANGE2");
        CPPUNIT_ASSERT(pDBData);
        CPPUNIT_ASSERT(!pDBData->HasHeader());
    }

    {
        // fdo#59240
        OUString aCSVFileName;
        createCSVPath("bugFix_Sheet4.", aCSVFileName);
        testFile(aCSVFileName, rDoc, 3);
    }

    xDocSh->DoClose();
}

void ScFiltersTest::testBugFixesXLS()
{
    ScDocShellRef xDocSh = loadDoc(u"bug-fixes.", FORMAT_XLS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load bugFixes.xls", xDocSh.is());

    xDocSh->DoHardRecalc();
    xDocSh->DoClose();
}

void ScFiltersTest::testBugFixesXLSX()
{
    ScDocShellRef xDocSh = loadDoc(u"bug-fixes.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to load bugFixes.xls", xDocSh.is());

    xDocSh->DoHardRecalc();
    xDocSh->DoClose();
}

namespace {

void checkMergedCells( ScDocument& rDoc, const ScAddress& rStartAddress,
                       const ScAddress& rExpectedEndAddress )
{
    SCCOL nActualEndCol = rStartAddress.Col();
    SCROW nActualEndRow = rStartAddress.Row();
    rDoc.ExtendMerge( rStartAddress.Col(), rStartAddress.Row(),
                       nActualEndCol, nActualEndRow, rStartAddress.Tab() );
    OString sTab = OString::number( rStartAddress.Tab() + 1 );
    OString msg = "Merged cells are not correctly imported on sheet" + sTab;
    OString msgCol = msg + "; end col";
    OString msgRow = msg + "; end row";
    CPPUNIT_ASSERT_EQUAL_MESSAGE( msgCol.pData->buffer, rExpectedEndAddress.Col(), nActualEndCol );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( msgRow.pData->buffer, rExpectedEndAddress.Row(), nActualEndRow );
}

}

void ScFiltersTest::testMergedCellsODS()
{
    ScDocShellRef xDocSh = loadDoc(u"merged.", FORMAT_ODS);
    ScDocument& rDoc = xDocSh->GetDocument();

    //check sheet1 content
    OUString aCSVFileName1;
    createCSVPath("merged1.", aCSVFileName1);
    testFile(aCSVFileName1, rDoc, 0);

    //check sheet1 merged cells
    checkMergedCells( rDoc, ScAddress( 0, 0, 0 ),  ScAddress( 5, 11, 0 ) );
    checkMergedCells( rDoc, ScAddress( 7, 2, 0 ),  ScAddress( 9, 12, 0 ) );
    checkMergedCells( rDoc, ScAddress( 3, 15, 0 ),  ScAddress( 7, 23, 0 ) );

    //check sheet2 content
    OUString aCSVFileName2;
    createCSVPath("merged2.", aCSVFileName2);
    testFile(aCSVFileName2, rDoc, 1);

    //check sheet2 merged cells
    checkMergedCells( rDoc, ScAddress( 4, 3, 1 ),  ScAddress( 6, 15, 1 ) );

    xDocSh->DoClose();
}

void ScFiltersTest::testRepeatedColumnsODS()
{
    ScDocShellRef xDocSh = loadDoc(u"repeatedColumns.", FORMAT_ODS);
    ScDocument& rDoc = xDocSh->GetDocument();

    //text
    OUString aCSVFileName1;
    createCSVPath("repeatedColumns1.", aCSVFileName1);
    testFile(aCSVFileName1, rDoc, 0);

    //numbers
    OUString aCSVFileName2;
    createCSVPath("repeatedColumns2.", aCSVFileName2);
    testFile(aCSVFileName2, rDoc, 1);

    xDocSh->DoClose();
}

namespace {

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
    sal_uLong nExpectedIndex;

    ValDataTestParams( ScValidationMode eMode, ScConditionMode eOp,
                       const OUString& aExpr1, const OUString& aExpr2, ScDocument& rDoc,
                       const ScAddress& aPos, const OUString& aETitle, const OUString& aEMsg,
                       ScValidErrorStyle eEStyle, sal_uLong nIndex ):
                            eValMode(eMode), eCondOp(eOp), aStrVal1(aExpr1),
                            aStrVal2(aExpr2), rDocument(rDoc), aPosition(aPos),
                            aErrorTitle(aETitle), aErrorMessage(aEMsg),
                            eErrorStyle(eEStyle), nExpectedIndex(nIndex) { };
};

void checkValiditationEntries( const ValDataTestParams& rVDTParams )
{
    ScDocument& rDoc = rVDTParams.rDocument;

    //create expected data validation entry
    ScValidationData aValData(
        rVDTParams.eValMode, rVDTParams.eCondOp, rVDTParams.aStrVal1,
        rVDTParams.aStrVal2, rDoc, rVDTParams.aPosition, OUString(),
        OUString(), rDoc.GetStorageGrammar(), rDoc.GetStorageGrammar()
    );
    aValData.SetIgnoreBlank( true );
    aValData.SetListType( 1 );
    aValData.ResetInput();
    aValData.SetError( rVDTParams.aErrorTitle, rVDTParams.aErrorMessage, rVDTParams.eErrorStyle );
    aValData.SetSrcString( OUString() );

    //get actual data validation entry from document
    const ScValidationData* pValDataTest = rDoc.GetValidationEntry( rVDTParams.nExpectedIndex );

    sal_Int32 nCol( static_cast<sal_Int32>(rVDTParams.aPosition.Col()) );
    sal_Int32 nRow( static_cast<sal_Int32>(rVDTParams.aPosition.Row()) );
    sal_Int32 nTab( static_cast<sal_Int32>(rVDTParams.aPosition.Tab()) );
    OString aMsgPrefix = "Data Validation Entry with base-cell-address: (" +
        OString::number(nCol) + "," + OString::number(nRow) + "," + OString::number(nTab) + ") ";

    OString aMsg = aMsgPrefix + "did not get imported at all.";
    CPPUNIT_ASSERT_MESSAGE(aMsg.getStr(), pValDataTest);

    //check if expected and actual data validation entries are equal
    if (!aValData.EqualEntries(*pValDataTest))
    {
        aMsg = aMsgPrefix + "got imported incorrectly.";
        CPPUNIT_FAIL(aMsg.getStr());
    }
}

void checkCellValidity( const ScAddress& rValBaseAddr, const ScRange& rRange, const ScDocument& rDoc )
{
    SCCOL nBCol( rValBaseAddr.Col() );
    SCROW nBRow( rValBaseAddr.Row() );
    SCTAB nTab( static_cast<sal_Int32>(rValBaseAddr.Tab()) );
    //get from the document the data validation entry we are checking against
    const SfxUInt32Item* pItem = rDoc.GetAttr(nBCol, nBRow, nTab, ATTR_VALIDDATA);
    const ScValidationData* pValData = rDoc.GetValidationEntry( pItem->GetValue() );
    CPPUNIT_ASSERT(pValData);

    //check that each cell in the expected range is associated with the data validation entry
    for(SCCOL i = rRange.aStart.Col(); i <= rRange.aEnd.Col(); ++i)
    {
        for(SCROW j = rRange.aStart.Row(); j <= rRange.aEnd.Row(); ++j)
        {
            const SfxUInt32Item* pItemTest = rDoc.GetAttr(i, j, nTab, ATTR_VALIDDATA);
            const ScValidationData* pValDataTest = rDoc.GetValidationEntry( pItemTest->GetValue() );
            //prevent string operations for occurring unnecessarily
            if(!(pValDataTest && pValData->GetKey() == pValDataTest->GetKey()))
            {
                sal_Int32 nCol = static_cast<sal_Int32>(i);
                sal_Int32 nRow = static_cast<sal_Int32>(j);
                sal_Int32 nTab32 = static_cast<sal_Int32>(nTab);
                OString sMsg = "\nData validation entry base-cell-address: (" +
                    OString::number( static_cast<sal_Int32>(nBCol) ) + "," +
                    OString::number( static_cast<sal_Int32>(nBRow) ) + "," +
                    OString::number( nTab32 ) + ")\n"
                    "Cell: (" + OString::number(nCol) + "," +
                    OString::number(nRow) + "," +
                    OString::number(nTab32) + ")";
                sal_uInt32 expectedKey(pValData->GetKey());
                sal_uInt32 actualKey(0);
                if(pValDataTest)
                    actualKey = pValDataTest->GetKey();
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sMsg.getStr(), expectedKey, actualKey);
            }
        }
    }
}

}

void ScFiltersTest::testDataValidityODS()
{
    ScDocShellRef xDocSh = loadDoc(u"dataValidity.", FORMAT_ODS);
    ScDocument& rDoc = xDocSh->GetDocument();

    ScAddress aValBaseAddr1( 2,6,0 ); //sheet1
    ScAddress aValBaseAddr2( 2,3,1 ); //sheet2
    ScAddress aValBaseAddr3( 2,2,2 ); //sheet3

    //sheet1's expected Data Validation Entry values
    ValDataTestParams aVDTParams1(
        SC_VALID_DECIMAL, ScConditionMode::Greater, "3.14", OUString(), rDoc,
        aValBaseAddr1, "Too small",
        "The number you are trying to enter is not greater than 3.14! Are you sure you want to enter it anyway?",
        SC_VALERR_WARNING, 1
    );
    //sheet2's expected Data Validation Entry values
    ValDataTestParams aVDTParams2(
        SC_VALID_WHOLE, ScConditionMode::Between, "1", "10", rDoc,
        aValBaseAddr2, "Error sheet 2",
        "Must be a whole number between 1 and 10.",
        SC_VALERR_STOP, 2
    );
    //sheet3's expected Data Validation Entry values
    ValDataTestParams aVDTParams3(
        SC_VALID_CUSTOM, ScConditionMode::Direct, "ISTEXT(C3)", OUString(), rDoc,
        aValBaseAddr3, "Error sheet 3",
        "Must not be a numerical value.",
        SC_VALERR_STOP, 3
    );
    //check each sheet's Data Validation Entries
    checkValiditationEntries( aVDTParams1 );
    checkValiditationEntries( aVDTParams2 );
    checkValiditationEntries( aVDTParams3 );

    //expected ranges to be associated with data validity
    ScRange aRange1( 2,2,0, 2,6,0 ); //sheet1
    ScRange aRange2( 2,3,1, 6,7,1 ); //sheet2
    ScRange aRange3( 2,2,2, 2,6,2 ); //sheet3

    //check each sheet's cells for data validity
    checkCellValidity( aValBaseAddr1, aRange1, rDoc );
    checkCellValidity( aValBaseAddr2, aRange2, rDoc );
    checkCellValidity( aValBaseAddr3, aRange3, rDoc );

    //check each sheet's content
    OUString aCSVFileName1;
    createCSVPath("dataValidity1.", aCSVFileName1);
    testFile(aCSVFileName1, rDoc, 0);

    OUString aCSVFileName2;
    createCSVPath("dataValidity2.", aCSVFileName2);
    testFile(aCSVFileName2, rDoc, 1);

    OUString aCSVFileName3;
    createCSVPath("dataValidity3.", aCSVFileName3);
    testFile(aCSVFileName3, rDoc, 2);

    xDocSh->DoClose();
}

void ScFiltersTest::testDataValidityXLSX()
{
    ScDocShellRef xDocSh = loadDoc(u"dataValidity.", FORMAT_XLSX);
    ScDocument& rDoc = xDocSh->GetDocument();

    ScAddress aValBaseAddr1( 2,6,0 ); //sheet1
    ScAddress aValBaseAddr2( 2,3,1 ); //sheet2
    ScAddress aValBaseAddr3( 2,2,2 ); //sheet3

    //expected ranges to be associated with data validity
    ScRange aRange1( 2,2,0, 2,6,0 ); //sheet1
    ScRange aRange2( 2,3,1, 6,7,1 ); //sheet2
    ScRange aRange3( 2,2,2, 2,6,2 ); //sheet3

    //check each sheet's cells for data validity
    checkCellValidity( aValBaseAddr1, aRange1, rDoc );
    checkCellValidity( aValBaseAddr2, aRange2, rDoc );
    checkCellValidity( aValBaseAddr3, aRange3, rDoc );

    xDocSh->DoClose();
}

void ScFiltersTest::testDataTableMortgageXLS()
{
    ScDocShellRef xDocSh = loadDoc(u"data-table/mortgage.", FORMAT_XLS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load the document.", xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();

    // One-variable table

    ASSERT_FORMULA_EQUAL(rDoc, ScAddress(3,1,0), "PMT(B3/12,B4,-B5)", "Wrong formula!");
    ASSERT_FORMULA_EQUAL(rDoc, ScAddress(3,2,0), "MULTIPLE.OPERATIONS(D$2,$B$3,$C3)", "Wrong formula!");
    ASSERT_FORMULA_EQUAL(rDoc, ScAddress(3,3,0), "MULTIPLE.OPERATIONS(D$2,$B$3,$C4)", "Wrong formula!");
    ASSERT_FORMULA_EQUAL(rDoc, ScAddress(3,4,0), "MULTIPLE.OPERATIONS(D$2,$B$3,$C5)", "Wrong formula!");

    // Two-variable table

    ASSERT_FORMULA_EQUAL(rDoc, ScAddress(2,7,0), "PMT(B9/12,B10,-B11)", "Wrong formula!");
    ASSERT_FORMULA_EQUAL(rDoc, ScAddress(3,8,0), "MULTIPLE.OPERATIONS($C$8,$B$9,$C9,$B$10,D$8)", "Wrong formula!");
    ASSERT_FORMULA_EQUAL(rDoc, ScAddress(3,9,0), "MULTIPLE.OPERATIONS($C$8,$B$9,$C10,$B$10,D$8)", "Wrong formula!");
    ASSERT_FORMULA_EQUAL(rDoc, ScAddress(3,10,0), "MULTIPLE.OPERATIONS($C$8,$B$9,$C11,$B$10,D$8)", "Wrong formula!");
    ASSERT_FORMULA_EQUAL(rDoc, ScAddress(4,8,0), "MULTIPLE.OPERATIONS($C$8,$B$9,$C9,$B$10,E$8)", "Wrong formula!");
    ASSERT_FORMULA_EQUAL(rDoc, ScAddress(4,9,0), "MULTIPLE.OPERATIONS($C$8,$B$9,$C10,$B$10,E$8)", "Wrong formula!");
    ASSERT_FORMULA_EQUAL(rDoc, ScAddress(4,10,0), "MULTIPLE.OPERATIONS($C$8,$B$9,$C11,$B$10,E$8)", "Wrong formula!");

    xDocSh->DoClose();
}

void ScFiltersTest::testDataTableOneVarXLSX()
{
    ScDocShellRef xDocSh = loadDoc(u"data-table/one-variable.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to load the document.", xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();

    // Right now, we have a bug that prevents Calc from re-calculating these
    // cells automatically upon file load. We can remove this call if/when we
    // fix the aforementioned bug.
    rDoc.CalcAll();

    // B5:B11 should have multiple operations formula cells.  Just check the
    // top and bottom cells.

    ASSERT_FORMULA_EQUAL(rDoc, ScAddress(1,4,0), "MULTIPLE.OPERATIONS(B$4,$A$2,$A5)", "Wrong formula!");

    CPPUNIT_ASSERT_EQUAL(2.0, rDoc.GetValue(ScAddress(1,4,0)));

    ASSERT_FORMULA_EQUAL(rDoc, ScAddress(1,10,0), "MULTIPLE.OPERATIONS(B$4,$A$2,$A11)", "Wrong formula!");

    CPPUNIT_ASSERT_EQUAL(14.0, rDoc.GetValue(ScAddress(1,10,0)));

    // Likewise, E5:I5 should have multiple operations formula cells.  Just
    // check the left- and right-most cells.

    ASSERT_FORMULA_EQUAL(rDoc, ScAddress(4,4,0), "MULTIPLE.OPERATIONS($D5,$B$2,E$4)", "Wrong formula!");

    CPPUNIT_ASSERT_EQUAL(10.0, rDoc.GetValue(ScAddress(4,4,0)));

    ASSERT_FORMULA_EQUAL(rDoc, ScAddress(8,4,0), "MULTIPLE.OPERATIONS($D5,$B$2,I$4)", "Wrong formula!");

    CPPUNIT_ASSERT_EQUAL(50.0, rDoc.GetValue(ScAddress(8,4,0)));

    xDocSh->DoClose();
}

void ScFiltersTest::testDataTableMultiTableXLSX()
{
    ScDocShellRef xDocSh = loadDoc(u"data-table/multi-table.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to load the document.", xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();

    // Right now, we have a bug that prevents Calc from re-calculating these
    // cells automatically upon file load. We can remove this call if/when we
    // fix the aforementioned bug.
    rDoc.CalcAll();

    // B4:M15 should have multiple operations formula cells.  We'll just check
    // the top-left and bottom-right ones.

    ASSERT_FORMULA_EQUAL(rDoc, ScAddress(1,3,0), "MULTIPLE.OPERATIONS($A$3,$E$1,$A4,$D$1,B$3)", "Wrong formula!");

    CPPUNIT_ASSERT_EQUAL(1.0, rDoc.GetValue(ScAddress(1,3,0)));

    ASSERT_FORMULA_EQUAL(rDoc, ScAddress(12,14,0), "MULTIPLE.OPERATIONS($A$3,$E$1,$A15,$D$1,M$3)", "Wrong formula!");

    CPPUNIT_ASSERT_EQUAL(144.0, rDoc.GetValue(ScAddress(12,14,0)));

    xDocSh->DoClose();
}

void ScFiltersTest::testBrokenQuotesCSV()
{
    OUString aFileExtension(getFileFormats()[FORMAT_CSV].pName, strlen(getFileFormats()[FORMAT_CSV].pName), RTL_TEXTENCODING_UTF8 );
    OUString aFilterName(getFileFormats()[FORMAT_CSV].pFilterName, strlen(getFileFormats()[FORMAT_CSV].pFilterName), RTL_TEXTENCODING_UTF8) ;
    OUString aFileName;
    createFileURL(u"fdo48621_broken_quotes.", aFileExtension, aFileName);
    OUString aFilterType(getFileFormats()[FORMAT_CSV].pTypeName, strlen(getFileFormats()[FORMAT_CSV].pTypeName), RTL_TEXTENCODING_UTF8);
    std::cout << getFileFormats()[FORMAT_CSV].pName << " Test" << std::endl;

    SfxFilterFlags nFormatType = getFileFormats()[FORMAT_CSV].nFormatType;
    SotClipboardFormatId nClipboardId = bool(nFormatType) ? SotClipboardFormatId::STARCALC_8 : SotClipboardFormatId::NONE;
    ScDocShellRef xDocSh = ScBootstrapFixture::load(aFileName, aFilterName, OUString(), aFilterType,
        nFormatType, nClipboardId);

    CPPUNIT_ASSERT_MESSAGE("Failed to load fdo48621_broken_quotes.csv", xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();

    OUString aCSVPath;
    createCSVPath( "fdo48621_broken_quotes_exported.", aCSVPath );
    // fdo#48621
    testFile( aCSVPath, rDoc, 0, StringType::PureString);

    xDocSh->DoClose();
}

void ScFiltersTest::testCellValueXLSX()
{
    static const OUStringLiteral aFileNameBase(u"cell-value.");
    OUString aFileExtension(getFileFormats()[FORMAT_XLSX].pName, strlen(getFileFormats()[FORMAT_XLSX].pName), RTL_TEXTENCODING_UTF8 );
    OUString aFilterName(getFileFormats()[FORMAT_XLSX].pFilterName, strlen(getFileFormats()[FORMAT_XLSX].pFilterName), RTL_TEXTENCODING_UTF8) ;
    OUString aFileName;
    createFileURL(aFileNameBase, aFileExtension, aFileName);
    OUString aFilterType(getFileFormats()[FORMAT_XLSX].pTypeName, strlen(getFileFormats()[FORMAT_XLSX].pTypeName), RTL_TEXTENCODING_UTF8);
    std::cout << getFileFormats()[FORMAT_XLSX].pName << " Test" << std::endl;

    SfxFilterFlags nFormatType = getFileFormats()[FORMAT_XLSX].nFormatType;
    SotClipboardFormatId nClipboardId = bool(nFormatType) ? SotClipboardFormatId::STARCALC_8 : SotClipboardFormatId::NONE;
    ScDocShellRef xDocSh = ScBootstrapFixture::load( aFileName, aFilterName, OUString(), aFilterType,
        nFormatType, nClipboardId);

    CPPUNIT_ASSERT_MESSAGE("Failed to load cell-value.xlsx", xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();

    OUString aCSVPath;
    createCSVPath( aFileNameBase, aCSVPath );
    testFile( aCSVPath, rDoc, 0 );

    xDocSh->DoClose();
}

void ScFiltersTest::testRowIndex1BasedXLSX()
{
    ScDocShellRef xDocSh = loadDoc(u"row-index-1-based.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();

    // A1
    OUString aStr = rDoc.GetString(ScAddress(0,0,0));
    CPPUNIT_ASSERT_EQUAL(OUString("Action Plan.Name"), aStr);

    // B1
    aStr = rDoc.GetString(ScAddress(1,0,0));
    CPPUNIT_ASSERT_EQUAL(OUString("Action Plan.Description"), aStr);

    // A2
    aStr = rDoc.GetString(ScAddress(0,1,0));
    CPPUNIT_ASSERT_EQUAL(OUString("Jerry"), aStr);

    // B2 - multi-line text.
    const EditTextObject* pText = rDoc.GetEditText(ScAddress(1,1,0));
    CPPUNIT_ASSERT(pText);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3), pText->GetParagraphCount());
    aStr = pText->GetText(0);
    CPPUNIT_ASSERT_EQUAL(OUString("This is a longer Text."), aStr);
    aStr = pText->GetText(1);
    CPPUNIT_ASSERT_EQUAL(OUString("Second line."), aStr);
    aStr = pText->GetText(2);
    CPPUNIT_ASSERT_EQUAL(OUString("Third line."), aStr);

    xDocSh->DoClose();
}

void ScFiltersTest::testImportCrash(std::u16string_view rFileName, sal_Int32 nFormat)
{
    ScDocShellRef xDocSh =loadDoc(rFileName, nFormat);
    CPPUNIT_ASSERT_MESSAGE(OString("Failed to load " + OUStringToOString(rFileName, RTL_TEXTENCODING_UTF8)).getStr(), xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();
    rDoc.CalcAll(); // perform hard re-calculation.

    xDocSh->DoClose();
}

void ScFiltersTest::testPassword_Impl(std::u16string_view aFileNameBase)
{
    OUString aFileExtension(getFileFormats()[0].pName, strlen(getFileFormats()[0].pName), RTL_TEXTENCODING_UTF8 );
    OUString aFilterName(getFileFormats()[0].pFilterName, strlen(getFileFormats()[0].pFilterName), RTL_TEXTENCODING_UTF8) ;
    OUString aFileName;
    createFileURL(aFileNameBase, aFileExtension, aFileName);
    OUString aFilterType(getFileFormats()[0].pTypeName, strlen(getFileFormats()[0].pTypeName), RTL_TEXTENCODING_UTF8);

    SfxFilterFlags nFormatType = getFileFormats()[0].nFormatType;
    OUString aPass("test");
    ScDocShellRef xDocSh = ScBootstrapFixture::load(aFileName, aFilterName, OUString(), aFilterType,
        nFormatType, SotClipboardFormatId::STARCALC_8, SOFFICE_FILEFORMAT_CURRENT, &aPass);

    CPPUNIT_ASSERT_MESSAGE("Failed to load password.ods", xDocSh.is());
    xDocSh->DoClose();
}

void ScFiltersTest::testPasswordNew()
{
    //tests opening a file with new password algorithm
    testPassword_Impl(u"password.");
}

void ScFiltersTest::testPasswordOld()
{
    //tests opening a file with old password algorithm
    testPassword_Impl(u"passwordOld.");
}

void ScFiltersTest::testPasswordWrongSHA()
{
    //tests opening a file wrongly using the new password algorithm
    //in a sxc with the key algorithm missing
    testPassword_Impl(u"passwordWrongSHA.");
}

void ScFiltersTest::testControlImport()
{
    ScDocShellRef xDocSh = loadDoc(u"singlecontrol.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to load singlecontrol.xlsx", xDocSh.is());

    uno::Reference< frame::XModel > xModel = xDocSh->GetModel();
    uno::Reference< sheet::XSpreadsheetDocument > xDoc(xModel, UNO_QUERY_THROW);
    uno::Reference< container::XIndexAccess > xIA(xDoc->getSheets(), UNO_QUERY_THROW);
    uno::Reference< drawing::XDrawPageSupplier > xDrawPageSupplier( xIA->getByIndex(0), UNO_QUERY_THROW);
    uno::Reference< container::XIndexAccess > xIA_DrawPage(xDrawPageSupplier->getDrawPage(), UNO_QUERY_THROW);
    uno::Reference< drawing::XControlShape > xControlShape(xIA_DrawPage->getByIndex(0), UNO_QUERY_THROW);

    xDocSh->DoClose();
}

void ScFiltersTest::testActiveXOptionButtonGroup()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf111980_radioButtons.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to load tdf111980_radioButtons.xlsx", xDocSh.is());
    uno::Reference< frame::XModel > xModel = xDocSh->GetModel();
    uno::Reference< sheet::XSpreadsheetDocument > xDoc(xModel, UNO_QUERY_THROW);
    uno::Reference< container::XIndexAccess > xIA(xDoc->getSheets(), UNO_QUERY_THROW);
    uno::Reference< drawing::XDrawPageSupplier > xDrawPageSupplier( xIA->getByIndex(0), UNO_QUERY_THROW);
    uno::Reference< container::XIndexAccess > xIA_DrawPage(xDrawPageSupplier->getDrawPage(), UNO_QUERY_THROW);

    OUString sGroupName;
    uno::Reference< drawing::XControlShape > xControlShape(xIA_DrawPage->getByIndex(0), UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(), uno::UNO_QUERY_THROW);
    xPropertySet->getPropertyValue("GroupName") >>= sGroupName;
    CPPUNIT_ASSERT_EQUAL(OUString("Sheet1"), sGroupName);

    // Optionbuttons (without Group names) were not grouped.
    // The two optionbuttons should have the same auto-generated group name.
    OUString sGroupName2; //ActiveX controls
    xControlShape.set(xIA_DrawPage->getByIndex(2), uno::UNO_QUERY_THROW);
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY_THROW);
    xPropertySet->getPropertyValue("GroupName") >>= sGroupName2;
    CPPUNIT_ASSERT_EQUAL( false, sGroupName2.isEmpty() );

    OUString sGroupName3;
    xControlShape.set(xIA_DrawPage->getByIndex(3), uno::UNO_QUERY_THROW);
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY_THROW);
    xPropertySet->getPropertyValue("GroupName") >>= sGroupName3;
    CPPUNIT_ASSERT_EQUAL( sGroupName2, sGroupName3 );
    CPPUNIT_ASSERT( sGroupName != sGroupName3 );

    OUString sGroupName4; //Form controls
    xControlShape.set(xIA_DrawPage->getByIndex(4), uno::UNO_QUERY_THROW);
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY_THROW);
    xPropertySet->getPropertyValue("GroupName") >>= sGroupName4;
    CPPUNIT_ASSERT_EQUAL( false, sGroupName4.isEmpty() );

    OUString sGroupName5;
    xControlShape.set(xIA_DrawPage->getByIndex(5), uno::UNO_QUERY_THROW);
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY_THROW);
    xPropertySet->getPropertyValue("GroupName") >>= sGroupName5;
    CPPUNIT_ASSERT_EQUAL( sGroupName4, sGroupName5 );
    CPPUNIT_ASSERT( sGroupName2 != sGroupName5 );
    CPPUNIT_ASSERT( sGroupName != sGroupName5 );

    OUString sGroupName7; //Form radiobutton autogrouped by GroupBox
    xControlShape.set(xIA_DrawPage->getByIndex(7), uno::UNO_QUERY_THROW);
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY_THROW);
    xPropertySet->getPropertyValue("GroupName") >>= sGroupName7;
    CPPUNIT_ASSERT_EQUAL( OUString("autoGroup_Group Box 7"), sGroupName7 );

    OUString sGroupName8;
    xControlShape.set(xIA_DrawPage->getByIndex(8), uno::UNO_QUERY_THROW);
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY_THROW);
    xPropertySet->getPropertyValue("GroupName") >>= sGroupName8;
    CPPUNIT_ASSERT_EQUAL( sGroupName7, sGroupName8 );
    CPPUNIT_ASSERT( sGroupName4 != sGroupName8 );
    CPPUNIT_ASSERT( sGroupName2 != sGroupName8 );
    CPPUNIT_ASSERT( sGroupName != sGroupName8 );

    OUString sGroupName9; //Form radiobutton not fully inside GroupBox
    xControlShape.set(xIA_DrawPage->getByIndex(9), uno::UNO_QUERY_THROW);
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY_THROW);
    xPropertySet->getPropertyValue("GroupName") >>= sGroupName9;
    CPPUNIT_ASSERT_EQUAL( sGroupName4, sGroupName9 );

    OUString sGroupName10; //ActiveX unaffected by GroupBox
    xControlShape.set(xIA_DrawPage->getByIndex(10), uno::UNO_QUERY_THROW);
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY_THROW);
    xPropertySet->getPropertyValue("GroupName") >>= sGroupName10;
    CPPUNIT_ASSERT_EQUAL( sGroupName, sGroupName10 );

    xDocSh->DoClose();
}

void ScFiltersTest::testChartImportODS()
{
    ScDocShellRef xDocSh = loadDoc(u"chart-import-basic.", FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load chart-import-basic.ods.", xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();

    // Ensure that the document contains "Empty", "Chart", "Data" and "Title" sheets in this exact order.
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "There should be 4 sheets in this document.", sal_Int16(4),
        rDoc.GetTableCount());
    OUString aName;
    rDoc.GetName(0, aName);
    CPPUNIT_ASSERT_EQUAL(OUString("Empty"), aName);
    rDoc.GetName(1, aName);
    CPPUNIT_ASSERT_EQUAL(OUString("Chart"), aName);
    rDoc.GetName(2, aName);
    CPPUNIT_ASSERT_EQUAL(OUString("Data"), aName);
    rDoc.GetName(3, aName);
    CPPUNIT_ASSERT_EQUAL(OUString("Title"), aName);

    // Retrieve the chart object instance from the 2nd page (for the 2nd sheet).
    const SdrOle2Obj* pOleObj = getSingleChartObject(rDoc, 1);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve a chart object from the 2nd sheet.", pOleObj);

    ScRangeList aRanges = getChartRanges(rDoc, *pOleObj);

    CPPUNIT_ASSERT_MESSAGE("Data series title cell not found.", aRanges.Contains(ScAddress(1,0,3))); // B1 on Title
    CPPUNIT_ASSERT_MESSAGE("Data series label range not found.", aRanges.Contains(ScRange(0,1,2,0,3,2))); // A2:A4 on Data
    CPPUNIT_ASSERT_MESSAGE("Data series value range not found.", aRanges.Contains(ScRange(1,1,2,1,3,2))); // B2:B4 on Data

    xDocSh->DoClose();
}

#if HAVE_MORE_FONTS
void ScFiltersTest::testChartImportXLS()
{
    ScDocShellRef xDocSh = loadDoc(u"chartx.", FORMAT_XLS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load chartx.xls.", xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();

    // Retrieve the chart object instance from the 2nd page (for the 2nd sheet).
    const SdrOle2Obj* pOleObj = getSingleChartObject(rDoc, 0);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve a chart object from the 2nd sheet.", pOleObj);

    CPPUNIT_ASSERT_EQUAL(tools::Long(11137), pOleObj->GetLogicRect().getWidth());
    CPPUNIT_ASSERT(8640L > pOleObj->GetLogicRect().getHeight());

    xDocSh->DoClose();
}
#endif

void ScFiltersTest::testNumberFormatHTML()
{
    ScDocShellRef xDocSh = loadDoc(u"numberformat.", FORMAT_HTML);
    CPPUNIT_ASSERT_MESSAGE("Failed to load numberformat.html", xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();

    // Check the header just in case.
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Cell value is not as expected", OUString("Product"),
        rDoc.GetString(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Cell value is not as expected", OUString("Price"),
        rDoc.GetString(1, 0, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Cell value is not as expected", OUString("Note"),
        rDoc.GetString(2, 0, 0));

    // B2 should be imported as a value cell.
    bool bHasValue = rDoc.HasValueData(1, 1, 0);
    CPPUNIT_ASSERT_MESSAGE("Fail to import number as a value cell.", bHasValue);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Incorrect value.", 199.98, rDoc.GetValue(1, 1, 0));

    xDocSh->DoClose();
}

void ScFiltersTest::testNumberFormatCSV()
{
    ScDocShellRef xDocSh = loadDoc(u"numberformat.", FORMAT_CSV);
    CPPUNIT_ASSERT_MESSAGE("Failed to load numberformat.csv", xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();

    // Check the header just in case.
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Cell value is not as expected", OUString("Product"),
        rDoc.GetString(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Cell value is not as expected", OUString("Price"),
        rDoc.GetString(1, 0, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Cell value is not as expected", OUString("Note"),
        rDoc.GetString(2, 0, 0));

    // B2 should be imported as a value cell.
    bool bHasValue = rDoc.HasValueData(1, 1, 0);
    CPPUNIT_ASSERT_MESSAGE("Fail to import number as a value cell.", bHasValue);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Incorrect value.", 199.98, rDoc.GetValue(1, 1, 0));

    xDocSh->DoClose();
}

void ScFiltersTest::testCellAnchoredShapesODS()
{
    ScDocShellRef xDocSh = loadDoc(u"cell-anchored-shapes.", FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load cell-anchored-shapes.ods", xDocSh.is());

    // There are two cell-anchored objects on the first sheet.
    ScDocument& rDoc = xDocSh->GetDocument();

    CPPUNIT_ASSERT_MESSAGE("There should be at least one sheet.", rDoc.GetTableCount() > 0);

    ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
    SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("draw page for sheet 1 should exist.", pPage);
    const size_t nCount = pPage->GetObjCount();
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "There should be 2 objects.", static_cast<size_t>(2), nCount);
    for (size_t i = 0; i < nCount; ++i)
    {
        SdrObject* pObj = pPage->GetObj(i);
        CPPUNIT_ASSERT_MESSAGE("Failed to get drawing object.", pObj);
        ScDrawObjData* pData = ScDrawLayer::GetObjData(pObj);
        CPPUNIT_ASSERT_MESSAGE("Failed to retrieve user data for this object.", pData);
        CPPUNIT_ASSERT_MESSAGE("Bounding rectangle should have been calculated upon import.", !pData->getShapeRect().IsEmpty());
    }

    xDocSh->DoClose();
}

void ScFiltersTest::testCellAnchoredHiddenShapesXLSX()
{
    ScDocShellRef xDocSh = loadDoc(u"cell-anchored-hidden-shapes.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to load cell-anchored-shapes.xlsx", xDocSh.is());

    // There are two cell-anchored objects on the first sheet.
    ScDocument& rDoc = xDocSh->GetDocument();

    CPPUNIT_ASSERT_MESSAGE("There should be at least one sheet.", rDoc.GetTableCount() > 0);

    ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
    SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("draw page for sheet 1 should exist.", pPage);
    const size_t nCount = pPage->GetObjCount();
    CPPUNIT_ASSERT_MESSAGE("There should be 2 shapes.", !(nCount == 2));

    SdrObject* pObj = pPage->GetObj(1);
    CPPUNIT_ASSERT_MESSAGE("Failed to get drawing object.", pObj);
    CPPUNIT_ASSERT_MESSAGE("The shape having same twocellanchor from and to attribute values, is visible.", !pObj->IsVisible());

    xDocSh->DoClose();
}

void ScFiltersTest::testRowHeightODS()
{
    ScDocShellRef xDocSh = loadDoc(u"row-height-import.", FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load row-height-import.ods", xDocSh.is());

    SCTAB nTab = 0;
    SCROW nRow = 0;
    ScDocument& rDoc = xDocSh->GetDocument();

    // The first 3 rows have manual heights.
    int nHeight = rDoc.GetRowHeight(nRow, nTab, false);
    bool bManual = rDoc.IsManualRowHeight(nRow, nTab);
    CPPUNIT_ASSERT_EQUAL(600, nHeight);
    CPPUNIT_ASSERT_MESSAGE("this row should have a manual row height.", bManual);
    nHeight = rDoc.GetRowHeight(++nRow, nTab, false);
    bManual = rDoc.IsManualRowHeight(nRow, nTab);
    CPPUNIT_ASSERT_EQUAL(1200, nHeight);
    CPPUNIT_ASSERT_MESSAGE("this row should have a manual row height.", bManual);
    nHeight = rDoc.GetRowHeight(++nRow, nTab, false);
    bManual = rDoc.IsManualRowHeight(nRow, nTab);
    CPPUNIT_ASSERT_EQUAL(1800, nHeight);
    CPPUNIT_ASSERT_MESSAGE("this row should have a manual row height.", bManual);

    // This one should have an automatic row height.
    bManual = rDoc.IsManualRowHeight(++nRow, nTab);
    CPPUNIT_ASSERT_MESSAGE("Row should have an automatic height.", !bManual);

    // Followed by a row with manual height.
    nHeight = rDoc.GetRowHeight(++nRow, nTab, false);
    bManual = rDoc.IsManualRowHeight(nRow, nTab);
    CPPUNIT_ASSERT_EQUAL(2400, nHeight);
    CPPUNIT_ASSERT_MESSAGE("this row should have a manual row height.", bManual);

    // And all the rest should have automatic heights.
    bManual = rDoc.IsManualRowHeight(++nRow, nTab);
    CPPUNIT_ASSERT_MESSAGE("Row should have an automatic height.", !bManual);

    bManual = rDoc.IsManualRowHeight(MAXROW, nTab);
    CPPUNIT_ASSERT_MESSAGE("Row should have an automatic height.", !bManual);

    xDocSh->DoClose();
}

void ScFiltersTest::testRichTextContentODS()
{
    ScDocShellRef xDocSh = loadDoc(u"rich-text-cells.", FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load rich-text-cells.ods", xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();

    OUString aTabName;
    CPPUNIT_ASSERT_MESSAGE("Failed to get the name of the first sheet.", rDoc.GetName(0, aTabName));

    // All tested cells are in the first column.
    ScAddress aPos(0, 0, 0);

    // Normal simple string with no formatting.
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_STRING, rDoc.GetCellType(aPos));
    CPPUNIT_ASSERT_EQUAL(OUString("Normal"), rDoc.GetString(aPos.Col(), aPos.Row(), aPos.Tab()));

    // Normal string with bold applied to the whole cell.
    {
        aPos.IncRow();
        CPPUNIT_ASSERT_EQUAL(CELLTYPE_STRING, rDoc.GetCellType(aPos));
        CPPUNIT_ASSERT_EQUAL(OUString("All bold"), rDoc.GetString(aPos.Col(), aPos.Row(), aPos.Tab()));
        const ScPatternAttr* pAttr = rDoc.GetPattern(aPos.Col(), aPos.Row(), aPos.Tab());
        CPPUNIT_ASSERT_MESSAGE("Failed to get cell attribute.", pAttr);
        const SvxWeightItem& rWeightItem = pAttr->GetItem(ATTR_FONT_WEIGHT);
        CPPUNIT_ASSERT_EQUAL(WEIGHT_BOLD, rWeightItem.GetWeight());
    }

    // This cell has an unformatted but multi-line content. Multi-line text is
    // stored in edit cell even if it has no formatting applied.
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_EDIT, rDoc.GetCellType(aPos));
    const EditTextObject* pEditText = rDoc.GetEditText(aPos);
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
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_EDIT, rDoc.GetCellType(aPos));
    pEditText = rDoc.GetEditText(aPos);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve edit text object.", pEditText);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), pEditText->GetParagraphCount());
    aParaText = pEditText->GetText(0);
    CPPUNIT_ASSERT_MESSAGE("Unexpected text.", aParaText.startsWith("Sheet name is "));
    CPPUNIT_ASSERT_MESSAGE("Sheet name field item not found.", pEditText->HasField(text::textfield::Type::TABLE));
    CPPUNIT_ASSERT_EQUAL(OUString("Sheet name is Test."), ScEditUtil::GetString(*pEditText, &rDoc));
    CPPUNIT_ASSERT_EQUAL(OUString("Sheet name is ?."), ScEditUtil::GetString(*pEditText, nullptr));

    // Cell with URL field item.
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_EDIT, rDoc.GetCellType(aPos));
    pEditText = rDoc.GetEditText(aPos);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve edit text object.", pEditText);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), pEditText->GetParagraphCount());
    aParaText = pEditText->GetText(0);
    CPPUNIT_ASSERT_MESSAGE("Unexpected text.", aParaText.startsWith("URL: "));
    CPPUNIT_ASSERT_MESSAGE("URL field item not found.", pEditText->HasField(text::textfield::Type::URL));
    CPPUNIT_ASSERT_EQUAL(OUString("URL: http://libreoffice.org"), ScEditUtil::GetString(*pEditText, &rDoc));
    CPPUNIT_ASSERT_EQUAL(OUString("URL: http://libreoffice.org"), ScEditUtil::GetString(*pEditText, nullptr));

    // Cell with Date field item.
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_EDIT, rDoc.GetCellType(aPos));
    pEditText = rDoc.GetEditText(aPos);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve edit text object.", pEditText);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), pEditText->GetParagraphCount());
    aParaText = pEditText->GetText(0);
    CPPUNIT_ASSERT_MESSAGE("Unexpected text.", aParaText.startsWith("Date: "));
    CPPUNIT_ASSERT_MESSAGE("Date field item not found.", pEditText->HasField(text::textfield::Type::DATE));
    CPPUNIT_ASSERT_MESSAGE("Date field not resolved with rDoc.", ScEditUtil::GetString(*pEditText, &rDoc).indexOf("/20") > 0);
    CPPUNIT_ASSERT_MESSAGE("Date field not resolved with NULL.", ScEditUtil::GetString(*pEditText, nullptr).indexOf("/20") > 0);

    // Cell with DocInfo title field item.
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_EDIT, rDoc.GetCellType(aPos));
    pEditText = rDoc.GetEditText(aPos);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve edit text object.", pEditText);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), pEditText->GetParagraphCount());
    aParaText = pEditText->GetText(0);
    CPPUNIT_ASSERT_MESSAGE("Unexpected text.", aParaText.startsWith("Title: "));
    CPPUNIT_ASSERT_MESSAGE("DocInfo title field item not found.", pEditText->HasField(text::textfield::Type::DOCINFO_TITLE));
    CPPUNIT_ASSERT_EQUAL(OUString("Title: Test Document"), ScEditUtil::GetString(*pEditText, &rDoc));
    CPPUNIT_ASSERT_EQUAL(OUString("Title: ?"), ScEditUtil::GetString(*pEditText, nullptr));

    // Cell with sentence with both bold and italic sequences.
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_EDIT, rDoc.GetCellType(aPos));
    pEditText = rDoc.GetEditText(aPos);
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
        CPPUNIT_ASSERT_MESSAGE("This sentence is expected to have both bold and italic sequences.", bHasBold);
        CPPUNIT_ASSERT_MESSAGE("This sentence is expected to have both bold and italic sequences.", bHasItalic);
    }

    // Cell with multi-line content with formatting applied.
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_EDIT, rDoc.GetCellType(aPos));
    pEditText = rDoc.GetEditText(aPos);
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
        bool bHasBold = std::any_of(aAttribs.begin(), aAttribs.end(), [](const EECharAttrib& rAttrib) {
            return rAttrib.pAttr->Which() == EE_CHAR_WEIGHT &&
                static_cast<const SvxWeightItem&>(*rAttrib.pAttr).GetWeight() == WEIGHT_BOLD; });
        CPPUNIT_ASSERT_MESSAGE("First line should be bold.", bHasBold);
    }

    // second line is italic.
    pEditText->GetCharAttribs(1, aAttribs);
    bool bHasItalic = std::any_of(aAttribs.begin(), aAttribs.end(), [](const EECharAttrib& rAttrib) {
            return rAttrib.pAttr->Which() == EE_CHAR_ITALIC &&
                static_cast<const SvxPostureItem&>(*rAttrib.pAttr).GetPosture() == ITALIC_NORMAL; });
    CPPUNIT_ASSERT_MESSAGE("Second line should be italic.", bHasItalic);

    // third line is underlined.
    pEditText->GetCharAttribs(2, aAttribs);
    bool bHasUnderline = std::any_of(aAttribs.begin(), aAttribs.end(), [](const EECharAttrib& rAttrib) {
        return rAttrib.pAttr->Which() == EE_CHAR_UNDERLINE &&
            static_cast<const SvxUnderlineItem&>(*rAttrib.pAttr).GetLineStyle() == LINESTYLE_SINGLE; });
    CPPUNIT_ASSERT_MESSAGE("Second line should be underlined.", bHasUnderline);

    // URL with formats applied.  For now, we'll check whether or not the
    // field objects gets imported.  Later we should add checks for the
    // formats.
    aPos.IncRow();
    pEditText = rDoc.GetEditText(aPos);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve edit text object.", pEditText);
    CPPUNIT_ASSERT_MESSAGE("URL field item not found.", pEditText->HasField(text::textfield::Type::URL));

    // Sheet name with formats applied.
    aPos.IncRow();
    pEditText = rDoc.GetEditText(aPos);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve edit text object.", pEditText);
    CPPUNIT_ASSERT_MESSAGE("Sheet name field item not found.", pEditText->HasField(text::textfield::Type::TABLE));

    // Date with formats applied.
    aPos.IncRow();
    pEditText = rDoc.GetEditText(aPos);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve edit text object.", pEditText);
    CPPUNIT_ASSERT_MESSAGE("Date field item not found.", pEditText->HasField(text::textfield::Type::DATE));

    // Document title with formats applied.
    aPos.IncRow();
    pEditText = rDoc.GetEditText(aPos);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve edit text object.", pEditText);
    CPPUNIT_ASSERT_MESSAGE("Date field item not found.", pEditText->HasField(text::textfield::Type::DOCINFO_TITLE));

    // URL for a file in the same directory. It should be converted into an absolute URL on import.
    aPos.IncRow();
    pEditText = rDoc.GetEditText(aPos);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve edit text object.", pEditText);
    const SvxFieldData* pData = pEditText->GetFieldData(0, 0, text::textfield::Type::URL);
    CPPUNIT_ASSERT_MESSAGE("Failed to get the URL data.", pData);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Failed to get the URL data.", text::textfield::Type::URL, pData->GetClassId());
    const SvxURLField* pURLData = static_cast<const SvxURLField*>(pData);
    CPPUNIT_ASSERT_MESSAGE("URL is not absolute with respect to the file system.", pURLData->GetURL().startsWith("file:///"));

    // Embedded spaces as <text:s text:c='4' />, normal text
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_STRING, rDoc.GetCellType(aPos));
    CPPUNIT_ASSERT_EQUAL(OUString("one     two"), rDoc.GetString(aPos.Col(), aPos.Row(), aPos.Tab()));

    // Leading space as <text:s />.
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_STRING, rDoc.GetCellType(aPos));
    CPPUNIT_ASSERT_EQUAL(OUString(" =3+4"), rDoc.GetString(aPos.Col(), aPos.Row(), aPos.Tab()));

    // Embedded spaces with <text:s text:c='4' /> inside a <text:span>, text
    // partly bold.
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_EDIT, rDoc.GetCellType(aPos));
    pEditText = rDoc.GetEditText(aPos);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve edit text object.", pEditText);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), pEditText->GetParagraphCount());
    aParaText = pEditText->GetText(0);
    CPPUNIT_ASSERT_EQUAL(OUString("one     two"), aParaText);
    pEditText->GetCharAttribs(0, aAttribs);
    {
        auto it = std::find_if(aAttribs.begin(), aAttribs.end(), [](const EECharAttrib& rAttrib) {
            return rAttrib.pAttr->Which() == EE_CHAR_WEIGHT &&
                static_cast<const SvxWeightItem&>(*rAttrib.pAttr).GetWeight() == WEIGHT_BOLD; });
        bool bHasBold = (it != aAttribs.end());
        if (bHasBold)
        {
            OUString aSeg = aParaText.copy(it->nStart, it->nEnd - it->nStart);
            CPPUNIT_ASSERT_EQUAL(OUString("e     t"), aSeg);
        }
        CPPUNIT_ASSERT_MESSAGE("Expected a bold sequence.", bHasBold);
    }

    xDocSh->DoClose();
}

void ScFiltersTest::testDataBarODS()
{
    ScDocShellRef xDocSh = loadDoc(u"databar.", FORMAT_ODS);
    CPPUNIT_ASSERT(xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();
    testDataBar_Impl(rDoc);

    xDocSh->DoClose();
}

void ScFiltersTest::testDataBarXLSX()
{
    ScDocShellRef xDocSh = loadDoc(u"databar.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();
    testDataBar_Impl(rDoc);

    xDocSh->DoClose();
}

void ScFiltersTest::testColorScaleODS()
{
    ScDocShellRef xDocSh = loadDoc(u"colorscale.", FORMAT_ODS);
    CPPUNIT_ASSERT(xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();

    testColorScale2Entry_Impl(rDoc);
    testColorScale3Entry_Impl(rDoc);

    xDocSh->DoClose();
}

void ScFiltersTest::testColorScaleXLSX()
{
    ScDocShellRef xDocSh = loadDoc(u"colorscale.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());
    ScDocument& rDoc = xDocSh->GetDocument();

    testColorScale2Entry_Impl(rDoc);
    testColorScale3Entry_Impl(rDoc);

    xDocSh->DoClose();
}

void ScFiltersTest::testNewCondFormatODS()
{
    ScDocShellRef xDocSh = ScBootstrapFixture::loadDoc(u"new_cond_format_test.", FORMAT_ODS);

    CPPUNIT_ASSERT_MESSAGE("Failed to load new_cond_format_test.ods", xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();

    OUString aCSVPath;
    createCSVPath( "new_cond_format_test.", aCSVPath );
    testCondFile(aCSVPath, &rDoc, 0);

    xDocSh->DoClose();
}

void ScFiltersTest::testNewCondFormatXLSX()
{
    ScDocShellRef xDocSh = ScBootstrapFixture::loadDoc(u"new_cond_format_test.", FORMAT_XLSX);

    CPPUNIT_ASSERT_MESSAGE("Failed to load new_cond_format_test.xlsx", xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();

    OUString aCSVPath;
    createCSVPath( "new_cond_format_test.", aCSVPath );
    testCondFile(aCSVPath, &rDoc, 0);

    xDocSh->DoClose();
}

void ScFiltersTest::testCondFormatImportCellIs()
{
    ScDocShellRef xDocSh = ScBootstrapFixture::loadDoc(u"condFormat_cellis.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to load condFormat_cellis.xlsx", xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT_EQUAL(size_t(1), rDoc.GetCondFormList(0)->size());

    ScConditionalFormat* pFormat = rDoc.GetCondFormat(0, 0, 0);
    CPPUNIT_ASSERT(pFormat);

    const ScFormatEntry* pEntry = pFormat->GetEntry(0);
    CPPUNIT_ASSERT(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScFormatEntry::Type::ExtCondition, pEntry->GetType());

    const ScCondFormatEntry* pCondition = static_cast<const ScCondFormatEntry*>(pEntry);
    CPPUNIT_ASSERT_EQUAL( ScConditionMode::Equal,  pCondition->GetOperation());

    OUString aStr = pCondition->GetExpression(ScAddress(0, 0, 0), 0);
    CPPUNIT_ASSERT_EQUAL( OUString("$Sheet2.$A$2"), aStr );

    pEntry = pFormat->GetEntry(1);
    CPPUNIT_ASSERT(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScFormatEntry::Type::ExtCondition, pEntry->GetType());

    pCondition = static_cast<const ScCondFormatEntry*>(pEntry);
    CPPUNIT_ASSERT_EQUAL( ScConditionMode::Equal,  pCondition->GetOperation());

    aStr = pCondition->GetExpression(ScAddress(0, 0, 0), 0);
    CPPUNIT_ASSERT_EQUAL( OUString("$Sheet2.$A$1"), aStr );

    xDocSh->DoClose();
}

void ScFiltersTest::testCondFormatThemeColorXLSX()
{
    ScDocShellRef xDocSh = ScBootstrapFixture::loadDoc(u"condformat_theme_color.", FORMAT_XLSX);

    CPPUNIT_ASSERT_MESSAGE("Failed to load condformat_theme_color.xlsx", xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();
    ScConditionalFormat* pFormat = rDoc.GetCondFormat(0, 0, 0);
    const ScFormatEntry* pEntry = pFormat->GetEntry(0);
    CPPUNIT_ASSERT(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScFormatEntry::Type::Databar, pEntry->GetType());
    const ScDataBarFormat* pDataBar = static_cast<const ScDataBarFormat*>(pEntry);
    const ScDataBarFormatData* pDataBarFormatData = pDataBar->GetDataBarData();

    CPPUNIT_ASSERT_EQUAL(Color(157, 195, 230), pDataBarFormatData->maPositiveColor);
    CPPUNIT_ASSERT(pDataBarFormatData->mxNegativeColor);
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, *pDataBarFormatData->mxNegativeColor);

    CPPUNIT_ASSERT_EQUAL(size_t(1), rDoc.GetCondFormList(1)->size());
    pFormat = rDoc.GetCondFormat(0, 0, 1);
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

    xDocSh->DoClose();
}

void ScFiltersTest::testCondFormatThemeColor2XLSX()
{
    ScDocShellRef xDocSh = ScBootstrapFixture::loadDoc(u"cond_format_theme_color2.", FORMAT_XLSX);

    CPPUNIT_ASSERT_MESSAGE("Failed to load cond_format_theme_color2.xlsx", xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();
    ScConditionalFormat* pFormat = rDoc.GetCondFormat(5, 5, 0);
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

    xDocSh->DoClose();
}

namespace {

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
    ScDocShellRef xDocSh = ScBootstrapFixture::loadDoc(u"cond_format_theme_color3.", FORMAT_XLSX);

    CPPUNIT_ASSERT_MESSAGE("Failed to load document", xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();
    ScConditionalFormat* pFormat = rDoc.GetCondFormat(1, 3, 0);
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

    pFormat = rDoc.GetCondFormat(3, 3, 0);
    checkDatabarPositiveColor(pFormat, Color(59, 56, 56));

    pFormat = rDoc.GetCondFormat(5, 3, 0);
    checkDatabarPositiveColor(pFormat, Color(173, 185, 202));

    pFormat = rDoc.GetCondFormat(7, 3, 0);
    checkDatabarPositiveColor(pFormat, Color(89, 89, 89));

    pFormat = rDoc.GetCondFormat(9, 3, 0);
    checkDatabarPositiveColor(pFormat, Color(217, 217, 217));

    xDocSh->DoClose();
}

namespace {

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

void testCustomIconSetsXLSX_Impl(const ScDocument& rDoc, SCCOL nCol, SCROW nRow, ScIconSetType eType, sal_Int32 nIndex)
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
    ScDocShellRef xDocSh = ScBootstrapFixture::loadDoc(u"complex_icon_set.", FORMAT_XLSX);

    CPPUNIT_ASSERT_MESSAGE("Failed to load complex_icon_set.xlsx", xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT_EQUAL(size_t(3), rDoc.GetCondFormList(0)->size());
    testComplexIconSetsXLSX_Impl(rDoc, 1, IconSet_3Triangles);
    testComplexIconSetsXLSX_Impl(rDoc, 3, IconSet_3Stars);
    testComplexIconSetsXLSX_Impl(rDoc, 5, IconSet_5Boxes);

    CPPUNIT_ASSERT_EQUAL(size_t(2), rDoc.GetCondFormList(1)->size());
    testCustomIconSetsXLSX_Impl(rDoc, 1, 1, IconSet_3ArrowsGray, 0);
    testCustomIconSetsXLSX_Impl(rDoc, 1, 2, IconSet_3ArrowsGray, -1);
    testCustomIconSetsXLSX_Impl(rDoc, 1, 3, IconSet_3Arrows, 1);
    testCustomIconSetsXLSX_Impl(rDoc, 1, 4, IconSet_3ArrowsGray, -1);
    testCustomIconSetsXLSX_Impl(rDoc, 1, 5, IconSet_3Arrows, 2);

    testCustomIconSetsXLSX_Impl(rDoc, 3, 1, IconSet_4RedToBlack, 3);
    testCustomIconSetsXLSX_Impl(rDoc, 3, 2, IconSet_3TrafficLights1, 1);
    testCustomIconSetsXLSX_Impl(rDoc, 3, 3, IconSet_3Arrows, 2);

    xDocSh->DoClose();
}

void ScFiltersTest::testTdf101104()
{
    ScDocShellRef xDocSh = ScBootstrapFixture::loadDoc(u"tdf101104.", FORMAT_ODS);

    CPPUNIT_ASSERT_MESSAGE("Failed to load tdf101104.ods", xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT_EQUAL(size_t(2), rDoc.GetCondFormList(0)->size());

    ScConditionalFormat* pFormat = rDoc.GetCondFormat(1, 1, 0);
    CPPUNIT_ASSERT(pFormat);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pFormat->size());
    const ScFormatEntry* pEntry = pFormat->GetEntry(0);
    CPPUNIT_ASSERT(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScFormatEntry::Type::Iconset, pEntry->GetType());
    const ScIconSetFormat* pIconSet = static_cast<const ScIconSetFormat*>(pEntry);

    for(size_t i = 1; i < 10; ++i)
    {
        std::unique_ptr<ScIconSetInfo> pInfo(pIconSet->GetIconSetInfo(ScAddress(1, i, 0)));

        // Without the fix in place, this test would have failed here
        CPPUNIT_ASSERT(pInfo);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), pInfo->nIconIndex);
        CPPUNIT_ASSERT_EQUAL(IconSet_3Arrows, pInfo->eIconSetType);

    }

    xDocSh->DoClose();
}

void ScFiltersTest::testTdf64401()
{
    ScDocShellRef xDocSh = ScBootstrapFixture::loadDoc(u"tdf64401.", FORMAT_ODS);

    CPPUNIT_ASSERT_MESSAGE("Failed to load tdf64401.ods", xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT_EQUAL(size_t(1), rDoc.GetCondFormList(0)->size());

    ScConditionalFormat* pFormat = rDoc.GetCondFormat(0, 0, 0);
    CPPUNIT_ASSERT(pFormat);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pFormat->size());
    const ScFormatEntry* pEntry = pFormat->GetEntry(0);
    CPPUNIT_ASSERT(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScFormatEntry::Type::Iconset, pEntry->GetType());
    const ScIconSetFormat* pIconSet = static_cast<const ScIconSetFormat*>(pEntry);

    for(size_t i = 0; i < 10; ++i)
    {
        sal_Int32 nIndex = 0;
        if ( i >= 7 ) // B5 = 8
            nIndex = 2;
        else if ( i >= 3 ) // B4 = 4
            nIndex = 1;

        std::unique_ptr<ScIconSetInfo> pInfo(pIconSet->GetIconSetInfo(ScAddress(0, i, 0)));
        CPPUNIT_ASSERT(pInfo);
        CPPUNIT_ASSERT_EQUAL(nIndex, pInfo->nIconIndex);
        CPPUNIT_ASSERT_EQUAL(IconSet_3Arrows, pInfo->eIconSetType);

    }

    // Update values in B4 and B5
    rDoc.SetValue(ScAddress(1,3,0), 2.0);
    rDoc.SetValue(ScAddress(1,4,0), 9.0);

    for(size_t i = 0; i < 10; ++i)
    {
        sal_Int32 nIndex = 0;
        if ( i >= 8 ) // B5 = 9
            nIndex = 2;
        else if ( i >= 1 ) // B4 = 2
            nIndex = 1;

        std::unique_ptr<ScIconSetInfo> pInfo(pIconSet->GetIconSetInfo(ScAddress(0, i, 0)));
        CPPUNIT_ASSERT(pInfo);
        CPPUNIT_ASSERT_EQUAL(nIndex, pInfo->nIconIndex);
        CPPUNIT_ASSERT_EQUAL(IconSet_3Arrows, pInfo->eIconSetType);

    }

    xDocSh->DoClose();
}

void ScFiltersTest::testCondFormatParentXLSX()
{
    ScDocShellRef xDocSh = ScBootstrapFixture::loadDoc(u"cond_parent.", FORMAT_XLSX);

    CPPUNIT_ASSERT_MESSAGE("Failed to load cond_parent.xlsx", xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();
    const SfxItemSet* pCondSet = rDoc.GetCondResult(2, 5, 0);
    const ScPatternAttr* pPattern = rDoc.GetPattern(2, 5, 0);
    const SfxPoolItem& rPoolItem = pPattern->GetItem(ATTR_VER_JUSTIFY, pCondSet);
    const SvxVerJustifyItem& rVerJustify = static_cast<const SvxVerJustifyItem&>(rPoolItem);
    CPPUNIT_ASSERT_EQUAL(SvxCellVerJustify::Top, rVerJustify.GetValue());

    xDocSh->DoClose();
}

void ScFiltersTest::testColorScaleNumWithRefXLSX()
{
    ScDocShellRef xDocSh = ScBootstrapFixture::loadDoc(u"colorscale_num_with_ref.", FORMAT_XLSX);

    CPPUNIT_ASSERT_MESSAGE("Failed to load colorscale_num_with_ref.xlsx", xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();
    ScConditionalFormatList* pList = rDoc.GetCondFormList(0);
    CPPUNIT_ASSERT(pList);

    CPPUNIT_ASSERT_EQUAL(size_t(1), pList->size());

    ScConditionalFormat* pFormat = pList->begin()->get();
    CPPUNIT_ASSERT(pFormat);

    CPPUNIT_ASSERT_EQUAL(size_t(1), pFormat->size());
    const ScFormatEntry* pEntry = pFormat->GetEntry(0);
    CPPUNIT_ASSERT(pEntry);

    CPPUNIT_ASSERT_EQUAL(ScFormatEntry::Type::Colorscale, pEntry->GetType());

    const ScColorScaleFormat* pColorScale= dynamic_cast<const ScColorScaleFormat*>(pEntry);
    CPPUNIT_ASSERT(pColorScale);

    const ScColorScaleEntry* pColorScaleEntry = pColorScale->GetEntry(1);
    CPPUNIT_ASSERT_EQUAL(OUString("=$A$1"),
            pColorScaleEntry->GetFormula(formula::FormulaGrammar::GRAM_NATIVE));

    xDocSh->DoClose();
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

    CPPUNIT_ASSERT_MESSAGE("Style Name2 : Doesn't have Attribute diagonal(tl-br) border, but it should have.",
        pStyleSheet->GetItemSet().HasItem(ATTR_BORDER_TLBR, &pItem));

    const SvxLineItem* pTLBR= static_cast<const SvxLineItem*>(pItem);
    CPPUNIT_ASSERT_EQUAL(Color(18, 0, 0), pTLBR->GetLine()->GetColor());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::DASH_DOT, pTLBR->GetLine()->GetBorderLineStyle());
    ASSERT_DOUBLES_EQUAL_MESSAGE("Error with diagonal tl-br width", 14, pTLBR->GetLine()->GetWidth());

    CPPUNIT_ASSERT_MESSAGE("Style Name2 : Doesn't have Attribute diagonal(bl-tr) border, but it should have.",
        pStyleSheet->GetItemSet().HasItem(ATTR_BORDER_BLTR, &pItem));

    const SvxLineItem* pBLTR= static_cast<const SvxLineItem*>(pItem);
    CPPUNIT_ASSERT_EQUAL(Color(255, 204, 238), pBLTR->GetLine()->GetColor());
    CPPUNIT_ASSERT_EQUAL(SvxBorderLineStyle::DASHED, pBLTR->GetLine()->GetBorderLineStyle());
    ASSERT_DOUBLES_EQUAL_MESSAGE("Error with diagonal tl-br width", 34, pBLTR->GetLine()->GetWidth());

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

    const SvxFontItem* pFontItem= static_cast<const SvxFontItem*>(pItem);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Style Name6 :Error with Font name", OUString("Liberation Sans"), pFontItem->GetStyleName());

    CPPUNIT_ASSERT_MESSAGE("Style Name6 : Doesn't have Attribute Font Height, but it should have.",
        pStyleSheet->GetItemSet().HasItem(ATTR_FONT_HEIGHT, &pItem));

    const SvxFontHeightItem* pFontHeightItem= static_cast<const SvxFontHeightItem*>(pItem);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Style Name6 :Error with Font Height", static_cast<sal_uInt32>(480), pFontHeightItem->GetHeight());

    CPPUNIT_ASSERT_MESSAGE("Style Name6 : Doesn't have Attribute Font Posture, but it should have.",
        pStyleSheet->GetItemSet().HasItem(ATTR_FONT_POSTURE, &pItem));

    const SvxPostureItem* pFontPostureItem= static_cast<const SvxPostureItem*>(pItem);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Style Name6 :Error with Font Posture", ITALIC_NORMAL, pFontPostureItem->GetPosture());

    CPPUNIT_ASSERT_MESSAGE("Style Name6 : Doesn't have Attribute Font Weight, but it should have.",
        pStyleSheet->GetItemSet().HasItem(ATTR_FONT_WEIGHT, &pItem));

    const SvxWeightItem* pFontWeightItem= static_cast<const SvxWeightItem*>(pItem);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Style Name6 :Error with Font Weight", WEIGHT_BOLD, pFontWeightItem->GetWeight());

    CPPUNIT_ASSERT_MESSAGE("Style Name6 : Doesn't have Attribute Font Color, but it should have.",
        pStyleSheet->GetItemSet().HasItem(ATTR_FONT_COLOR, &pItem));

    const SvxColorItem* pFontColorItem= static_cast<const SvxColorItem*>(pItem);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Style Name6 :Error with Font Color", Color(128, 128, 128), pFontColorItem->GetValue());

    CPPUNIT_ASSERT_MESSAGE("Style Name6 : Doesn't have Attribute Underline, but it should have.",
        pStyleSheet->GetItemSet().HasItem(ATTR_FONT_UNDERLINE, &pItem));

    const SvxUnderlineItem* pUnderlineItem= static_cast<const SvxUnderlineItem*>(pItem);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Style Name6 :Error with Font Underline Style", LINESTYLE_SINGLE, pUnderlineItem->GetLineStyle());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Style Name6 :Error with Font Underline Color", Color(128, 128, 128), pUnderlineItem->GetColor());

    /* Test for Style Name "7"
     * Has strikethrough single
     */
    pStyleSheet = pStyleSheetPool->FindCaseIns("Name7", SfxStyleFamily::Para);
    CPPUNIT_ASSERT_MESSAGE("Style Name7 : Doesn't have Attribute Strikeout, but it should have.",
        pStyleSheet->GetItemSet().HasItem(ATTR_FONT_CROSSEDOUT, &pItem));

    const SvxCrossedOutItem* pCrossedOutItem = static_cast<const SvxCrossedOutItem*>(pItem);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Style Name7 :Error with Strikeout", STRIKEOUT_SINGLE, pCrossedOutItem->GetStrikeout());

    /* Test for Style Name "8"
     * Has strikethrough bold
     */
    pStyleSheet = pStyleSheetPool->FindCaseIns("Name8", SfxStyleFamily::Para);
    CPPUNIT_ASSERT_MESSAGE("Style Name8 : Doesn't have Attribute Strikeout, but it should have.",
        pStyleSheet->GetItemSet().HasItem(ATTR_FONT_CROSSEDOUT, &pItem));

    pCrossedOutItem = static_cast<const SvxCrossedOutItem*>(pItem);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Style Name7 :Error with Strikeout", STRIKEOUT_BOLD, pCrossedOutItem->GetStrikeout());

    /* Test for Style Name "9"
     * Has strikethrough slash
     */
    pStyleSheet = pStyleSheetPool->FindCaseIns("Name9", SfxStyleFamily::Para);
    CPPUNIT_ASSERT_MESSAGE("Style Name9 : Doesn't have Attribute Strikeout, but it should have.",
        pStyleSheet->GetItemSet().HasItem(ATTR_FONT_CROSSEDOUT, &pItem));

    pCrossedOutItem = static_cast<const SvxCrossedOutItem*>(pItem);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Style Name9 :Error with Strikeout", STRIKEOUT_SLASH, pCrossedOutItem->GetStrikeout());

    /* Test for Style Name "10"
     * Has ver align, and hor align
     */

    pStyleSheet = pStyleSheetPool->FindCaseIns("Name10", SfxStyleFamily::Para);
    CPPUNIT_ASSERT_MESSAGE("Style Name10 : Doesn't have Attribute hor justify, but it should have.",
        pStyleSheet->GetItemSet().HasItem(ATTR_HOR_JUSTIFY, &pItem));

    const SvxHorJustifyItem* pHorJustify = static_cast<const SvxHorJustifyItem*>(pItem);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Style Name10 :Error with hor justify", SvxCellHorJustify::Right, pHorJustify->GetValue());

    pStyleSheet = pStyleSheetPool->FindCaseIns("Name10", SfxStyleFamily::Para);
    CPPUNIT_ASSERT_MESSAGE("Style Name10 : Doesn't have Attribute ver justify, but it should have.",
        pStyleSheet->GetItemSet().HasItem(ATTR_VER_JUSTIFY, &pItem));

    const SvxVerJustifyItem* pVerJustify = static_cast<const SvxVerJustifyItem*>(pItem);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Style Name10 :Error with ver justify", SvxCellVerJustify::Center, pVerJustify->GetValue());

    auto checkFontWeight = [pStyleSheetPool](const OUString& rName, FontWeight eExpected)
    {
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
            os << "Wrong font weight value for style named '" << rName << "': expected="
                << eExpected << "; actual=" << eActual;
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
    ScDocShellRef xDocSh = loadDoc(u"shared-string/literal-in-formula.", FORMAT_XLS);
    CPPUNIT_ASSERT(xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();
    rDoc.CalcAll();

    CPPUNIT_ASSERT_EQUAL(2.0, rDoc.GetValue(ScAddress(0,0,0)));

    xDocSh->DoClose();
}

void ScFiltersTest::testFormulaDependency()
{
    ScDocShellRef xDocSh = ScBootstrapFixture::loadDoc(u"dependencyTree.", FORMAT_ODS);

    ScDocument& rDoc = xDocSh->GetDocument();

    // check if formula in A1 changes value
    double nVal = rDoc.GetValue(0,0,0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(nVal, 1.0, 1e-10);
    rDoc.SetValue(0,1,0, 0.0);
    nVal = rDoc.GetValue(0,0,0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(nVal, 2.0, 1e-10);

    // check that the number format is implicitly inherited
    // CPPUNIT_ASSERT_EQUAL(rDoc.GetString(0,4,0), rDoc.GetString(0,5,0));

    xDocSh->DoClose();
}

void ScFiltersTest::testTdf122643() { testImportCrash(u"tdf122643.", FORMAT_ODS); }

void ScFiltersTest::testTdf132278() { testImportCrash(u"tdf132278.", FORMAT_ODS); }

void ScFiltersTest::testTdf130959() { testImportCrash(u"tdf130959.", FORMAT_XLSX); }

void ScFiltersTest::testTdf129410() { testImportCrash(u"tdf129410.", FORMAT_ODS); }

void ScFiltersTest::testTdf138507() { testImportCrash(u"tdf138507.", FORMAT_ODS); }

void ScFiltersTest::testTdf131380() { testImportCrash(u"tdf131380.", FORMAT_XLSX); }

void ScFiltersTest::testTdf139782() { testImportCrash(u"tdf139782.", FORMAT_ODS); }

void ScFiltersTest::testTdf136551() { testImportCrash(u"tdf136551.", FORMAT_ODS); }

void ScFiltersTest::testTdf129681()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf129681.", FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to open doc", xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();

    CPPUNIT_ASSERT_EQUAL(OUString("Lamb"), rDoc.GetString(ScAddress(4, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("Beef"), rDoc.GetString(ScAddress(4, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("Pork"), rDoc.GetString(ScAddress(4, 4, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("Goat"), rDoc.GetString(ScAddress(4, 5, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("Goat"), rDoc.GetString(ScAddress(4, 6, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("#VALUE!"), rDoc.GetString(ScAddress(4, 7, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("#VALUE!"), rDoc.GetString(ScAddress(4, 8, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("Deer"), rDoc.GetString(ScAddress(4, 9, 0)));

    CPPUNIT_ASSERT_EQUAL(OUString("1"), rDoc.GetString(ScAddress(6, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("2"), rDoc.GetString(ScAddress(6, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("3"), rDoc.GetString(ScAddress(6, 4, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("5"), rDoc.GetString(ScAddress(6, 5, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("8"), rDoc.GetString(ScAddress(6, 6, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("#VALUE!"), rDoc.GetString(ScAddress(6, 7, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("#VALUE!"), rDoc.GetString(ScAddress(6, 8, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("#VALUE!"), rDoc.GetString(ScAddress(6, 9, 0)));

    xDocSh->DoClose();
}

void ScFiltersTest::testTdf111974XLSM() { testImportCrash(u"tdf111974.", FORMAT_XLSM); }

void ScFiltersTest::testEscapedUnicodeXLSX()
{
    ScDocShellRef xDocSh = loadDoc(u"escape-unicode.", FORMAT_XLSX);
    ScDocument& rDoc = xDocSh->GetDocument();

    // Without the fix, there would be "_x000D_" after every new-line char.
    CPPUNIT_ASSERT_EQUAL(OUString("Line 1\nLine 2\nLine 3\nLine 4"), rDoc.GetString(1, 1, 0));

    xDocSh->DoClose();
}

void ScFiltersTest::testTdf144758_DBDataDefaultOrientation()
{
    ScDocShellRef xDocSh = loadDoc(u"tdf144758-dbdata-no-orientation.", FORMAT_FODS);
    CPPUNIT_ASSERT(xDocSh);
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDBData* pAnonDBData = rDoc.GetAnonymousDBData(0);
    CPPUNIT_ASSERT(pAnonDBData);

    ScSortParam aSortParam;
    pAnonDBData->GetSortParam(aSortParam);

    // Without the fix, the default value for bByRow (in absence of 'table:orientation' attribute
    // in 'table:database-range' element) was false
    CPPUNIT_ASSERT(aSortParam.bByRow);

    xDocSh->DoClose();
}

ScFiltersTest::ScFiltersTest()
      : ScBootstrapFixture( "sc/qa/unit/data" )
{
}

void ScFiltersTest::setUp()
{
    test::BootstrapFixture::setUp();

    // This is a bit of a fudge, we do this to ensure that ScGlobals::ensure,
    // which is a private symbol to us, gets called
    m_xCalcComponent =
        getMultiServiceFactory()->createInstance("com.sun.star.comp.Calc.SpreadsheetDocument");
    CPPUNIT_ASSERT_MESSAGE("no calc component!", m_xCalcComponent.is());
}

void ScFiltersTest::tearDown()
{
    uno::Reference< lang::XComponent >( m_xCalcComponent, UNO_QUERY_THROW )->dispose();
    test::BootstrapFixture::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScFiltersTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

