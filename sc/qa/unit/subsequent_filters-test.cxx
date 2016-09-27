/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <unotest/filters-test.hxx>
#include <test/bootstrapfixture.hxx>
#include <rtl/strbuf.hxx>
#include <osl/file.hxx>

#include <sfx2/app.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/sfxmodelfactory.hxx>
#include <svl/stritem.hxx>
#include <svl/nfkeytab.hxx>
#include <svl/zformat.hxx>
#include <svx/svdograf.hxx>

#include "drwlayer.hxx"
#include <svx/svdpage.hxx>
#include <svx/svdoole2.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/editobj.hxx>
#include <editeng/borderline.hxx>
#include <editeng/flditem.hxx>
#include <editeng/justifyitem.hxx>
#include <dbdata.hxx>
#include "validat.hxx"
#include "formulacell.hxx"
#include "userdat.hxx"
#include "dpobject.hxx"
#include "dpsave.hxx"
#include "stlsheet.hxx"
#include "docfunc.hxx"
#include "markdata.hxx"
#include "colorscale.hxx"
#include "olinetab.hxx"
#include "patattr.hxx"
#include "scitems.hxx"
#include "docsh.hxx"
#include "editutil.hxx"
#include "cellvalue.hxx"
#include "attrib.hxx"
#include "dpshttab.hxx"
#include "tabvwsh.hxx"
#include "fillinfo.hxx"
#include <scopetools.hxx>
#include <columnspanset.hxx>
#include <tokenstringcontext.hxx>
#include <formula/errorcodes.hxx>
#include "externalrefmgr.hxx"


#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include <com/sun/star/sheet/DataPilotFieldGroupBy.hpp>
#include <com/sun/star/sheet/GeneralFunction.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XModel2.hpp>
#include <com/sun/star/text/textfield/Type.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/data/XDataReceiver.hpp>

#include "helper/qahelper.hxx"
#include "helper/shared_test_impl.hxx"
#include <algorithm>

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
    void testBooleanFormatXLSX();
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
    void testBorderODS();
    void testBordersOoo33();
    void testBugFixesODS();
    void testBugFixesXLS();
    void testBugFixesXLSX();
    void testBrokenQuotesCSV();
    void testMergedCellsODS();
    void testRepeatedColumnsODS();
    void testDataValidityODS();
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
    void testCondFormatThemeColor2XLSX(); // negative bar color and axis color
    void testComplexIconSetsXLSX();
    void testCondFormatParentXLSX();

    void testLiteralInFormulaXLS();

    //change this test file only in excel and not in calc
    void testCellValueXLSX();

    /**
     * Test importing of xlsx document that previously had its row index off
     * by one. (fdo#76032)
     */
    void testRowIndex1BasedXLSX();
    void testErrorOnExternalReferences();

    //misc tests unrelated to the import filters
#if !defined(MACOSX) && !defined(DRAGONFLY) && !defined(WNT)
    void testPasswordNew();
    void testPasswordOld();
    void testPasswordWrongSHA();
#endif

    //test shape import
    void testControlImport();
    void testChartImportODS();
    void testChartImportXLS();

    void testNumberFormatHTML();
    void testNumberFormatCSV();

    void testCellAnchoredShapesODS();
    void testCellAnchoredHiddenShapesXLSX();

    void testPivotTableBasicODS();
    void testPivotTableNamedRangeSourceODS();
    void testPivotTableSharedCacheGroupODS();
    void testGetPivotDataXLS();
    void testPivotTableSharedGroupXLSX();
    void testPivotTableSharedDateGroupXLSX();
    void testPivotTableSharedNestedDateGroupXLSX();
    void testPivotTableSharedNumGroupXLSX();

    void testFormulaDependency();

    void testRowHeightODS();
    void testRichTextContentODS();
    void testMiscRowHeights();
    void testOptimalHeightReset();

    void testPrintRangeODS();
    void testOutlineODS();

    void testColumnStyleXLSX();

    void testSharedFormulaHorizontalXLS();
    void testSharedFormulaWrappedRefsXLS();
    void testSharedFormulaBIFF5();
    void testSharedFormulaXLSB();
    void testSharedFormulaXLS();
    void testExternalRefCacheXLSX();
    void testExternalRefCacheODS();
    void testHybridSharedStringODS();
    void testCopyMergedNumberFormats();
    void testVBAUserFunctionXLSM();
    void testEmbeddedImageXLS();
    void testEditEngStrikeThroughXLSX();
    void testRefStringXLSX();
    void testHiddenSheetsXLSX();

    void testBnc762542();

    CPPUNIT_TEST_SUITE(ScFiltersTest);
    CPPUNIT_TEST(testBooleanFormatXLSX);
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
    CPPUNIT_TEST(testBorderODS);
    CPPUNIT_TEST(testBordersOoo33);
    CPPUNIT_TEST(testBugFixesODS);
    CPPUNIT_TEST(testBugFixesXLS);
    CPPUNIT_TEST(testBugFixesXLSX);
    CPPUNIT_TEST(testMergedCellsODS);
    CPPUNIT_TEST(testRepeatedColumnsODS);
    CPPUNIT_TEST(testDataValidityODS);
    CPPUNIT_TEST(testDataTableMortgageXLS);
    CPPUNIT_TEST(testDataTableOneVarXLSX);
    CPPUNIT_TEST(testDataTableMultiTableXLSX);
    CPPUNIT_TEST(testBrokenQuotesCSV);
    CPPUNIT_TEST(testCellValueXLSX);
    CPPUNIT_TEST(testRowIndex1BasedXLSX);
    CPPUNIT_TEST(testControlImport);
    CPPUNIT_TEST(testChartImportODS);
    CPPUNIT_TEST(testChartImportXLS);

    CPPUNIT_TEST(testDataBarODS);
    CPPUNIT_TEST(testDataBarXLSX);
    CPPUNIT_TEST(testColorScaleODS);
    CPPUNIT_TEST(testColorScaleXLSX);
    CPPUNIT_TEST(testNewCondFormatODS);
    CPPUNIT_TEST(testNewCondFormatXLSX);
    CPPUNIT_TEST(testCondFormatThemeColorXLSX);
    CPPUNIT_TEST(testCondFormatThemeColor2XLSX);
    CPPUNIT_TEST(testComplexIconSetsXLSX);
    CPPUNIT_TEST(testCondFormatParentXLSX);
    CPPUNIT_TEST(testLiteralInFormulaXLS);

    CPPUNIT_TEST(testNumberFormatHTML);
    CPPUNIT_TEST(testNumberFormatCSV);

    CPPUNIT_TEST(testCellAnchoredShapesODS);
    CPPUNIT_TEST(testCellAnchoredHiddenShapesXLSX);

    CPPUNIT_TEST(testPivotTableBasicODS);
    CPPUNIT_TEST(testPivotTableNamedRangeSourceODS);
    CPPUNIT_TEST(testPivotTableSharedCacheGroupODS);
    CPPUNIT_TEST(testGetPivotDataXLS);
    CPPUNIT_TEST(testPivotTableSharedGroupXLSX);
    CPPUNIT_TEST(testPivotTableSharedDateGroupXLSX);
    CPPUNIT_TEST(testPivotTableSharedNestedDateGroupXLSX);
    CPPUNIT_TEST(testPivotTableSharedNumGroupXLSX);
    CPPUNIT_TEST(testRowHeightODS);
    CPPUNIT_TEST(testFormulaDependency);
    CPPUNIT_TEST(testRichTextContentODS);

    //disable testPassword on MacOSX due to problems with libsqlite3
    //also crashes on DragonFly due to problems with nss/nspr headers
#if !defined(MACOSX) && !defined(DRAGONFLY) && !defined(WNT)
    CPPUNIT_TEST(testPasswordWrongSHA);
    CPPUNIT_TEST(testPasswordOld);
    CPPUNIT_TEST(testPasswordNew);
#endif

    CPPUNIT_TEST(testMiscRowHeights);
    CPPUNIT_TEST(testOptimalHeightReset);
    CPPUNIT_TEST(testPrintRangeODS);
    CPPUNIT_TEST(testOutlineODS);
    CPPUNIT_TEST(testColumnStyleXLSX);
    CPPUNIT_TEST(testSharedFormulaHorizontalXLS);
    CPPUNIT_TEST(testSharedFormulaWrappedRefsXLS);
    CPPUNIT_TEST(testSharedFormulaBIFF5);
    CPPUNIT_TEST(testSharedFormulaXLSB);
    CPPUNIT_TEST(testSharedFormulaXLS);
    CPPUNIT_TEST(testExternalRefCacheXLSX);
    CPPUNIT_TEST(testExternalRefCacheODS);
    CPPUNIT_TEST(testHybridSharedStringODS);
    CPPUNIT_TEST(testCopyMergedNumberFormats);
    CPPUNIT_TEST(testVBAUserFunctionXLSM);
    CPPUNIT_TEST(testEmbeddedImageXLS);
    CPPUNIT_TEST(testErrorOnExternalReferences);
    CPPUNIT_TEST(testEditEngStrikeThroughXLSX);
    CPPUNIT_TEST(testRefStringXLSX);

    CPPUNIT_TEST(testBnc762542);

    CPPUNIT_TEST(testHiddenSheetsXLSX);

    CPPUNIT_TEST_SUITE_END();

private:
#if !defined(MACOSX) && !defined(DRAGONFLY) && !defined(WNT)
    void testPassword_Impl(const OUString& rFileNameBase);
#endif

    uno::Reference<uno::XInterface> m_xCalcComponent;
};

bool ScFiltersTest::load(const OUString &rFilter, const OUString &rURL,
    const OUString &rUserData, SfxFilterFlags nFilterFlags,
        SotClipboardFormatId nClipboardID, unsigned int nFilterVersion)
{
    ScDocShellRef xDocShRef = ScBootstrapFixture::load( rURL, rFilter, rUserData,
        OUString(), nFilterFlags, nClipboardID, nFilterVersion);
    bool bLoaded = xDocShRef.Is();
    //reference counting of ScDocShellRef is very confused.
    if (bLoaded)
        xDocShRef->DoClose();
    return bLoaded;
}

namespace {

void testRangeNameImpl(ScDocument& rDoc)
{
    //check one range data per sheet and one global more detailed
    //add some more checks here
    ScRangeData* pRangeData = rDoc.GetRangeName()->findByUpperName(OUString("GLOBAL1"));
    CPPUNIT_ASSERT_MESSAGE("range name Global1 not found", pRangeData);
    double aValue;
    rDoc.GetValue(1,0,0,aValue);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("range name Global1 should reference Sheet1.A1", 1.0, aValue);
    pRangeData = rDoc.GetRangeName(0)->findByUpperName(OUString("LOCAL1"));
    CPPUNIT_ASSERT_MESSAGE("range name Sheet1.Local1 not found", pRangeData);
    rDoc.GetValue(1,2,0,aValue);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("range name Sheet1.Local1 should reference Sheet1.A3", 3.0, aValue);
    pRangeData = rDoc.GetRangeName(1)->findByUpperName(OUString("LOCAL2"));
    CPPUNIT_ASSERT_MESSAGE("range name Sheet2.Local2 not found", pRangeData);
    rDoc.GetValue(1,1,1,aValue);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("range name Sheet2.Local2 should reference Sheet2.A2", 7.0, aValue);
    //check for correct results for the remaining formulas
    rDoc.GetValue(1,1,0, aValue);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("=global2 should be 2", 2.0, aValue);
    rDoc.GetValue(1,3,0, aValue);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("=local2 should be 4", 4.0, aValue);
    rDoc.GetValue(2,0,0, aValue);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("=SUM(global3) should be 10", 10.0, aValue);
    rDoc.GetValue(1,0,1,aValue);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("range name Sheet2.local1 should reference Sheet1.A5", 5.0, aValue);
    // Test if Global5 ( which depends on Global6 ) is evaluated
    rDoc.GetValue(0,5,1, aValue);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("formula Global5 should reference Global6 ( which is evaluated as local1 )", 5.0, aValue);
}

}

void ScFiltersTest::testBasicCellContentODS()
{
    ScDocShellRef xDocSh = loadDoc("basic-cell-content.", FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load basic-cell-content.ods", xDocSh.Is());

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
    ScDocShellRef xDocSh = loadDoc("check-boolean.", FORMAT_XLSX);
    ScDocument& rDoc = xDocSh->GetDocument();
    SvNumberFormatter* pNumFormatter = rDoc.GetFormatTable();
    const OUString aBooleanTypeStr = "\"TRUE\";\"TRUE\";\"FALSE\"";

    CPPUNIT_ASSERT_MESSAGE("Failed to load check-boolean.xlsx", xDocSh.Is());
    sal_uInt32 nNumberFormat;

    for (SCROW i = 0; i <= 1; i++)
    {
        rDoc.GetNumberFormat(0, i, 0, nNumberFormat);
        const SvNumberformat* pNumberFormat = pNumFormatter->GetEntry(nNumberFormat);
        const OUString& rFormatStr = pNumberFormat->GetFormatstring();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Number format != boolean", rFormatStr, aBooleanTypeStr);
    }

    xDocSh->DoClose();
}

void ScFiltersTest::testRangeNameXLS()
{
    ScDocShellRef xDocSh = loadDoc("named-ranges-global.", FORMAT_XLS);
    xDocSh->DoHardRecalc(true);

    ScDocument& rDoc = xDocSh->GetDocument();
    testRangeNameImpl(rDoc);

    OUString aSheet2CSV("rangeExp_Sheet2.");
    OUString aCSVPath;
    createCSVPath( aSheet2CSV, aCSVPath );
    // fdo#44587
    testFile( aCSVPath, rDoc, 1);

    xDocSh->DoClose();
}

void ScFiltersTest::testRangeNameLocalXLS()
{
    ScDocShellRef xDocSh = loadDoc("named-ranges-local.", FORMAT_XLS);
    xDocSh->DoHardRecalc(true);

    ScDocument& rDoc = xDocSh->GetDocument();
    ScRangeName* pRangeName = rDoc.GetRangeName(0);
    CPPUNIT_ASSERT(pRangeName);
    CPPUNIT_ASSERT_EQUAL(size_t(2), pRangeName->size());

    OUString aFormula;
    rDoc.GetFormula(3, 11, 0, aFormula);
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(local_name2)"), aFormula);
    ASSERT_DOUBLES_EQUAL(14.0, rDoc.GetValue(3, 11, 0));

    rDoc.GetFormula(6, 4, 0, aFormula);
    CPPUNIT_ASSERT_EQUAL(OUString("=local_name1"), aFormula);

    xDocSh->DoClose();
}

void ScFiltersTest::testRangeNameXLSX()
{
    ScDocShellRef xDocSh = loadDoc("named-ranges-global.", FORMAT_XLSX);
    xDocSh->DoHardRecalc(true);

    ScDocument& rDoc = xDocSh->GetDocument();
    testRangeNameImpl(rDoc);

    xDocSh->DoClose();
}

void ScFiltersTest::testHyperlinksXLSX()
{
    ScDocShellRef xDocSh = loadDoc("hyperlinks.", FORMAT_XLSX);
    ScDocument& rDoc = xDocSh->GetDocument();

    CPPUNIT_ASSERT_EQUAL(OUString("10:ABC10"), rDoc.GetString(ScAddress(0,1,0)));
    CPPUNIT_ASSERT_EQUAL(OUString("10:ABC11"), rDoc.GetString(ScAddress(0,2,0)));
    CPPUNIT_ASSERT_EQUAL(OUString("10:ABC12"), rDoc.GetString(ScAddress(0,3,0)));

    xDocSh->DoClose();
}

void ScFiltersTest::testHardRecalcODS()
{
    ScDocShellRef xDocSh = loadDoc("hard-recalc.", FORMAT_ODS);
    xDocSh->DoHardRecalc(true);

    CPPUNIT_ASSERT_MESSAGE("Failed to load hard-recalc.*", xDocSh.Is());
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
    ScDocShellRef xDocSh = loadDoc("functions.", FORMAT_ODS);
    xDocSh->DoHardRecalc(true);

    CPPUNIT_ASSERT_MESSAGE("Failed to load functions.*", xDocSh.Is());
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
    testFile(aCSVFileName, rDoc, 2, PureString);
    //test information functions
    createCSVPath("information-functions.", aCSVFileName);
    testFile(aCSVFileName, rDoc, 3);
    // text functions
    createCSVPath("text-functions.", aCSVFileName);
    testFile(aCSVFileName, rDoc, 4, PureString);
    // statistical functions
    createCSVPath("statistical-functions.", aCSVFileName);
    testFile(aCSVFileName, rDoc, 5);
    // financial functions
    createCSVPath("financial-functions.", aCSVFileName);
    testFile(aCSVFileName, rDoc, 6);

    xDocSh->DoClose();

    xDocSh = loadDoc("database-functions.", FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load functions.*", xDocSh.Is());
    xDocSh->DoHardRecalc(true);
    ScDocument& rDoc2 = xDocSh->GetDocument();

    createCSVPath("database-functions.", aCSVFileName);
    testFile(aCSVFileName, rDoc2, 0);

    xDocSh->DoClose();

    xDocSh = loadDoc("date-time-functions.", FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load functions.*", xDocSh.Is());
    xDocSh->DoHardRecalc(true);
    ScDocument& rDoc3 = xDocSh->GetDocument();
    createCSVPath("date-time-functions.", aCSVFileName);
    testFile(aCSVFileName, rDoc3, 0, PureString);

    xDocSh->DoClose();

    // crashes at exit while unloading StarBasic code
    // xDocSh = loadDoc("user-defined-function.", FORMAT_ODS);
    // xDocSh->DoHardRecalc(true);
    // ScDocument& rDocUserDef = xDocSh->GetDocument();
    // createCSVPath("user-defined-function.", aCSVFileName);
    // testFile(aCSVFileName, rDocUserDef, 0);
}

void ScFiltersTest::testFunctionsExcel2010()
{
    ScDocShellRef xDocSh = loadDoc("functions-excel-2010.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to load the document.", xDocSh.Is());
    ScDocument& rDoc = xDocSh->GetDocument();
    rDoc.CalcAll(); // perform hard re-calculation.

    testFunctionsExcel2010_Impl(rDoc);

    xDocSh->DoClose();
}

void ScFiltersTest::testCeilingFloorXLSX()
{
    ScDocShellRef xDocSh = loadDoc("ceiling-floor.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to load the document.", xDocSh.Is());
    ScDocument& rDoc = xDocSh->GetDocument();
    rDoc.CalcAll(); // perform hard re-calculation.

    testCeilingFloor_Impl(rDoc);

    xDocSh->DoClose();
}

void ScFiltersTest::testCachedFormulaResultsODS()
{
    {
        ScDocShellRef xDocSh = loadDoc("functions.", FORMAT_ODS);
        CPPUNIT_ASSERT_MESSAGE("Failed to load functions.*", xDocSh.Is());

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
        testFile(aCSVFileName, rDoc, 2, PureString);
        //test cached formula results of information functions
        createCSVPath("information-functions.", aCSVFileName);
        testFile(aCSVFileName, rDoc, 3);
        // text functions
        createCSVPath("text-functions.", aCSVFileName);
        testFile(aCSVFileName, rDoc, 4, PureString);

        xDocSh->DoClose();
    }

    {
        ScDocShellRef xDocSh = loadDoc("cachedValue.", FORMAT_ODS);
        CPPUNIT_ASSERT_MESSAGE("Failed to load cachedValue.*", xDocSh.Is());

        ScDocument& rDoc = xDocSh->GetDocument();
        OUString aCSVFileName;
        createCSVPath("cachedValue.", aCSVFileName);
        testFile(aCSVFileName, rDoc, 0);

        //we want to me sure that volatile functions are always recalculated
        //regardless of cached results.  if you update the ods file, you must
        //update the values here.
        //if NOW() is recacluated, then it should never equal sTodayCache
        OUString sTodayCache("01/25/13 01:06 PM");
        OUString sTodayRecalc(rDoc.GetString(0,0,1));

        CPPUNIT_ASSERT(sTodayCache != sTodayRecalc);

        OUString sTodayRecalcRef(rDoc.GetString(1,0,1));
        CPPUNIT_ASSERT_EQUAL(sTodayRecalc, sTodayRecalcRef);

        // make sure that error values are not being treated as string values
        for(SCCOL nCol = 0; nCol < 4; ++nCol)
        {
            for(SCROW nRow = 0; nRow < 2; ++nRow)
            {
                OUStringBuffer aIsErrorFormula("=ISERROR(");
                aIsErrorFormula.append((char)('A'+nCol)).append(OUString::number(nRow));
                aIsErrorFormula.append(")");
                OUString aFormula = aIsErrorFormula.makeStringAndClear();
                rDoc.SetString(nCol, nRow + 2, 2, aFormula);
                CPPUNIT_ASSERT_EQUAL_MESSAGE(OUStringToOString(aFormula, RTL_TEXTENCODING_UTF8).getStr(), rDoc.GetString(nCol, nRow +2, 2), OUString("TRUE"));

                OUStringBuffer aIsTextFormula("=ISTEXT(");
                aIsTextFormula.append((char)('A'+nCol)).append(OUString::number(nRow));
                aIsTextFormula.append(")");
                rDoc.SetString(nCol, nRow + 4, 2, aIsTextFormula.makeStringAndClear());
                CPPUNIT_ASSERT_EQUAL_MESSAGE("", rDoc.GetString(nCol, nRow +4, 2), OUString("FALSE"));
            }
        }

        xDocSh->DoClose();
    }
}

void ScFiltersTest::testCachedMatrixFormulaResultsODS()
{
    ScDocShellRef xDocSh = loadDoc("matrix.", FORMAT_ODS);

    CPPUNIT_ASSERT_MESSAGE("Failed to load matrix.*", xDocSh.Is());
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
    ScDocShellRef xDocSh = loadDoc("formula-across-sheets.", FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load the file.", xDocSh.Is());
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
    ScDocShellRef xDocSh = loadDoc("formula-delete-contents.", FORMAT_ODS, true);
    CPPUNIT_ASSERT_MESSAGE("Failed to load the file.", xDocSh.Is());
    ScDocument& rDoc = xDocSh->GetDocument();

    sc::UndoSwitch aUndoSwitch(rDoc, true); // Enable undo.
    sc::AutoCalcSwitch aACSwitch(rDoc, true); // Make sure auto calc is turned on.

    CPPUNIT_ASSERT_EQUAL(195.0, rDoc.GetValue(ScAddress(3,15,0))); // formula in D16

    // Delete D2:D5.
    ScDocFunc& rFunc = xDocSh->GetDocFunc();
    ScRange aRange(3,1,0,3,4,0);
    ScMarkData aMark;
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
    CPPUNIT_ASSERT_MESSAGE("Sheet1: row 0 should be visible", !bHidden && nRow1 == 0 && nRow2 == 0);
    bHidden = rDoc.RowHidden(1, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("Sheet1: rows 1-2 should be hidden", bHidden && nRow1 == 1 && nRow2 == 2);
    bHidden = rDoc.RowHidden(3, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("Sheet1: row 3 should be visible", !bHidden && nRow1 == 3 && nRow2 == 3);
    bHidden = rDoc.RowHidden(4, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("Sheet1: row 4-5 should be hidden", bHidden && nRow1 == 4 && nRow2 == 5);
    bHidden = rDoc.RowHidden(6, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("Sheet1: row 6-end should be visible", !bHidden && nRow1 == 6 && nRow2 == MAXROW);
    if (nFormat == FORMAT_ODS) //excel doesn't support named db ranges
    {
        double aValue;
        rDoc.GetValue(0,10,1, aValue);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Sheet2: A11: formula result is incorrect", 4.0, aValue);
        rDoc.GetValue(1, 10, 1, aValue);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Sheet2: B11: formula result is incorrect", 2.0, aValue);
    }
    double aValue;
    rDoc.GetValue(3,10,1, aValue);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Sheet2: D11: formula result is incorrect", 4.0, aValue);
    rDoc.GetValue(4, 10, 1, aValue);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Sheet2: E11: formula result is incorrect", 2.0, aValue);
}

}

void ScFiltersTest::testDatabaseRangesODS()
{
    ScDocShellRef xDocSh = loadDoc("database.", FORMAT_ODS);
    xDocSh->DoHardRecalc(true);

    ScDocument& rDoc = xDocSh->GetDocument();

    testDBRanges_Impl(rDoc, FORMAT_ODS);
    xDocSh->DoClose();
}

void ScFiltersTest::testDatabaseRangesXLS()
{
    ScDocShellRef xDocSh = loadDoc("database.", FORMAT_XLS);
    xDocSh->DoHardRecalc(true);

    ScDocument& rDoc = xDocSh->GetDocument();

    testDBRanges_Impl(rDoc, FORMAT_XLS);
    xDocSh->DoClose();
}

void ScFiltersTest::testDatabaseRangesXLSX()
{
    ScDocShellRef xDocSh = loadDoc("database.", FORMAT_XLSX);
    xDocSh->DoHardRecalc(true);

    ScDocument& rDoc = xDocSh->GetDocument();

    testDBRanges_Impl(rDoc, FORMAT_XLSX);
    xDocSh->DoClose();
}

void ScFiltersTest::testFormatsODS()
{
    ScDocShellRef xDocSh = loadDoc("formats.", FORMAT_ODS);
    xDocSh->DoHardRecalc(true);

    ScDocument& rDoc = xDocSh->GetDocument();

    testFormats(this, &rDoc, FORMAT_ODS);
    xDocSh->DoClose();
}

// void ScFiltersTest::testFormatsXLS()
// {
//     ScDocShellRef xDocSh = loadDoc("formats.", FORMAT_XLS);
//     xDocSh->DoHardRecalc(true);
//
//     ScDocument& rDoc = xDocSh->GetDocument();
//
//     testFormats(this, rDoc, FORMAT_XLS);
//     xDocSh->DoClose();
// }

// void ScFiltersTest::testFormatsXLSX()
// {
//     ScDocShellRef xDocSh = loadDoc("formats.", FORMAT_XLSX);
//     xDocSh->DoHardRecalc(true);
//
//     ScDocument& rDoc = xDocSh->GetDocument();
//
//     testFormats(this, rDoc, FORMAT_XLSX);
//     xDocSh->DoClose();
// }

void ScFiltersTest::testMatrixODS()
{
    ScDocShellRef xDocSh = loadDoc("matrix.", FORMAT_ODS);
    xDocSh->DoHardRecalc(true);

    ScDocument& rDoc = xDocSh->GetDocument();

    OUString aCSVFileName;
    createCSVPath("matrix.", aCSVFileName);
    testFile(aCSVFileName, rDoc, 0);

    xDocSh->DoClose();
}

void ScFiltersTest::testMatrixXLS()
{
    ScDocShellRef xDocSh = loadDoc("matrix.", FORMAT_XLS);
    xDocSh->DoHardRecalc(true);

    CPPUNIT_ASSERT_MESSAGE("Failed to load matrix.*", xDocSh.Is());
    ScDocument& rDoc = xDocSh->GetDocument();

    OUString aCSVFileName;
    createCSVPath("matrix.", aCSVFileName);
    testFile(aCSVFileName, rDoc, 0);

    xDocSh->DoClose();
}

void ScFiltersTest::testBorderODS()
{
    ScDocShellRef xDocSh = loadDoc("border.", FORMAT_ODS);

    CPPUNIT_ASSERT_MESSAGE("Failed to load border.*", xDocSh.Is());
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
    CPPUNIT_ASSERT_EQUAL(
        table::BorderLineStyle::SOLID, pRight->GetBorderLineStyle());

    rDoc.GetBorderLines( 2, 1, 0, &pLeft, &pTop, &pRight, &pBottom );
    CPPUNIT_ASSERT(!pLeft);
    CPPUNIT_ASSERT(!pTop);
    CPPUNIT_ASSERT(!pBottom);

    CPPUNIT_ASSERT(pRight);
    CPPUNIT_ASSERT_EQUAL(
        table::BorderLineStyle::SOLID, pRight->GetBorderLineStyle());
    CPPUNIT_ASSERT_EQUAL(20L, pRight->GetWidth());

    rDoc.GetBorderLines( 2, 8, 0, &pLeft, &pTop, &pRight, &pBottom );

    CPPUNIT_ASSERT(pLeft);
    CPPUNIT_ASSERT(pTop);
    CPPUNIT_ASSERT(pBottom);
    CPPUNIT_ASSERT(pRight);
    CPPUNIT_ASSERT_EQUAL(
        table::BorderLineStyle::SOLID, pRight->GetBorderLineStyle());
    CPPUNIT_ASSERT_EQUAL(5L, pRight->GetWidth());
    CPPUNIT_ASSERT(pRight->GetColor() == Color(COL_BLUE));

    xDocSh->DoClose();
}

struct Border
{
    sal_Int16 column;
    sal_Int32 row;
    long leftWidth;
    long topWidth;
    long rightWidth;
    long bottomWidth;
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
    sal_Int32 lStyle;
    sal_Int32 tStyle;
    sal_Int32 rStyle;
    sal_Int32 bStyle;
    // that's a monstrum
    Border(sal_Int16 col, sal_Int32 r, sal_Int32 lW, sal_Int32 tW, sal_Int32 rW, sal_Int32 bW, sal_uInt16 lOutW, sal_uInt16 lInW,
        sal_uInt16 lDist, sal_uInt16 tOutW, sal_uInt16 tInW, sal_uInt16 tDist, sal_uInt16 rOutW, sal_uInt16 rInW, sal_uInt16 rDist,
        sal_uInt16 bOutW, sal_uInt16 bInW, sal_uInt16 bDist, sal_Int32 lSt, sal_Int32 tSt, sal_Int32 rSt, sal_Int32 bSt):
    column(col), row(r), leftWidth(lW), topWidth(tW), rightWidth(rW), bottomWidth(bW), lOutWidth(lOutW), lInWidth(lInW), lDistance(lDist),
    tOutWidth(tOutW), tInWidth(tInW), tDistance(tDist), rOutWidth(rOutW), rInWidth(rInW), rDistance(rDist), bOutWidth(bOutW), bInWidth(bInW),
    bDistance(bDist), lStyle(lSt), tStyle(tSt), rStyle(rSt), bStyle(bSt) {};
};

void ScFiltersTest::testBordersOoo33()
{
    std::vector<Border> borders;
    borders.push_back(Border(1, 1, 22, 22, 22, 22, 1, 1, 20, 1, 1, 20, 1, 1, 20, 1, 1, 20, 3, 3, 3, 3));
    borders.push_back(Border(1, 3, 52, 52, 52, 52, 1, 1, 50, 1, 1, 50, 1, 1, 50, 1, 1, 50, 3, 3, 3, 3));
    borders.push_back(Border(1, 5, 60, 60, 60, 60, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 3, 3, 3, 3));
    borders.push_back(Border(1, 7, 150, 150, 150, 150, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 3, 3, 3, 3));
    borders.push_back(Border(1, 9, 71, 71, 71, 71, 20, 1, 50, 20, 1, 50, 20, 1, 50, 20, 1, 50, 3, 3, 3, 3));
    borders.push_back(Border(1, 11, 101, 101, 101, 101, 50, 1, 50, 50, 1, 50, 50, 1, 50, 50, 1, 50, 3, 3, 3, 3));
    borders.push_back(Border(1, 13, 131, 131, 131, 131, 80, 1, 50, 80, 1, 50, 80, 1, 50, 80, 1, 50, 3, 3, 3, 3));
    borders.push_back(Border(1, 15, 120, 120, 120, 120, 50, 20, 50, 50, 20, 50, 50, 20, 50, 50, 20, 50, 3, 3, 3, 3));
    borders.push_back(Border(1, 17, 90, 90, 90, 90, 20, 50, 20, 20, 50, 20, 20, 50, 20, 20, 50, 20, 3, 3, 3, 3));
    borders.push_back(Border(1, 19, 180, 180, 180, 180, 80, 50, 50, 80, 50, 50, 80, 50, 50, 80, 50, 50, 3, 3, 3, 3));
    borders.push_back(Border(1, 21, 180, 180, 180, 180, 50, 80, 50, 50, 80, 50, 50, 80, 50, 50, 80, 50, 3, 3, 3, 3));
    borders.push_back(Border(4, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0));
    borders.push_back(Border(4, 3, 10, 10, 10, 10, 10, 0, 0, 10, 0, 0, 10, 0, 0, 10, 0, 0, 0, 0, 0, 0));
    borders.push_back(Border(4, 5, 20, 20, 20, 20, 20, 0, 0, 20, 0, 0, 20, 0, 0, 20, 0, 0, 0, 0, 0, 0));
    borders.push_back(Border(4, 7, 50, 50, 50, 50, 50, 0, 0, 50, 0, 0, 50, 0, 0, 50, 0, 0, 0, 0, 0, 0));
    borders.push_back(Border(4, 9, 80, 80, 80, 80, 80, 0, 0, 80, 0, 0, 80, 0, 0, 80, 0, 0, 0, 0, 0, 0));
    borders.push_back(Border(4, 11, 100, 100, 100, 100, 100, 0, 0, 100, 0, 0, 100, 0, 0, 100, 0, 0, 0, 0, 0, 0));

    ScDocShellRef xDocSh = loadDoc("borders_ooo33.", FORMAT_ODS);

    CPPUNIT_ASSERT_MESSAGE("Failed to load borders_ooo33.*", xDocSh.Is());
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
                sal_Int32 tempStyle = pLeft->GetBorderLineStyle();
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
    ScDocShellRef xDocSh = loadDoc("bug-fixes.", FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load bugFixes.ods", xDocSh.Is());

    xDocSh->DoHardRecalc(true);
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
    ScDocShellRef xDocSh = loadDoc("bug-fixes.", FORMAT_XLS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load bugFixes.xls", xDocSh.Is());

    xDocSh->DoHardRecalc(true);
    xDocSh->GetDocument();
    xDocSh->DoClose();
}

void ScFiltersTest::testBugFixesXLSX()
{
    ScDocShellRef xDocSh = loadDoc("bug-fixes.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to load bugFixes.xls", xDocSh.Is());

    xDocSh->DoHardRecalc(true);
    xDocSh->GetDocument();
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
    ScDocShellRef xDocSh = loadDoc("merged.", FORMAT_ODS);
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
    ScDocShellRef xDocSh = loadDoc("repeatedColumns.", FORMAT_ODS);
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
                       ScAddress aPos, const OUString& aETitle, const OUString& aEMsg,
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
        rVDTParams.aStrVal2, &rDoc, rVDTParams.aPosition, EMPTY_OUSTRING,
        EMPTY_OUSTRING, rDoc.GetStorageGrammar(), rDoc.GetStorageGrammar()
    );
    aValData.SetIgnoreBlank( true );
    aValData.SetListType( 1 );
    aValData.ResetInput();
    aValData.SetError( rVDTParams.aErrorTitle, rVDTParams.aErrorMessage, rVDTParams.eErrorStyle );
    aValData.SetSrcString( EMPTY_OUSTRING );

    //get actual data validation entry from document
    const ScValidationData* pValDataTest = rDoc.GetValidationEntry( rVDTParams.nExpectedIndex );

    sal_Int32 nCol( static_cast<sal_Int32>(rVDTParams.aPosition.Col()) );
    sal_Int32 nRow( static_cast<sal_Int32>(rVDTParams.aPosition.Row()) );
    sal_Int32 nTab( static_cast<sal_Int32>(rVDTParams.aPosition.Tab()) );
    OStringBuffer sMsg("Data Validation Entry with base-cell-address: (");
    sMsg.append(nCol).append(",").append(nRow).append(",").append(nTab).append(") ");
    OString aMsgPrefix = sMsg.makeStringAndClear();

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
    SCTAB nTab( static_cast<const sal_Int32>(rValBaseAddr.Tab()) );
    //get from the document the data validation entry we are checking against
    const SfxUInt32Item* pItem = static_cast<const SfxUInt32Item*>(rDoc.GetAttr(nBCol, nBRow, nTab, ATTR_VALIDDATA) );
    const ScValidationData* pValData = rDoc.GetValidationEntry( pItem->GetValue() );
    CPPUNIT_ASSERT(pValData);

    //check that each cell in the expected range is associated with the data validation entry
    for(SCCOL i = rRange.aStart.Col(); i <= rRange.aEnd.Col(); ++i)
    {
        for(SCROW j = rRange.aStart.Row(); j <= rRange.aEnd.Row(); ++j)
        {
            const SfxUInt32Item* pItemTest = static_cast<const SfxUInt32Item*>( rDoc.GetAttr(i, j, nTab, ATTR_VALIDDATA) );
            const ScValidationData* pValDataTest = rDoc.GetValidationEntry( pItemTest->GetValue() );
            //prevent string operations for occurring unnecessarily
            if(!(pValDataTest && pValData->GetKey() == pValDataTest->GetKey()))
            {
                sal_Int32 nCol = static_cast<const sal_Int32>(i);
                sal_Int32 nRow = static_cast<const sal_Int32>(j);
                sal_Int32 nTab32 = static_cast<const sal_Int32>(nTab);
                OStringBuffer sMsg("\nData validation entry base-cell-address: (");
                sMsg.append( static_cast<const sal_Int32>(nBCol) ).append(",");
                sMsg.append( static_cast<const sal_Int32>(nBRow) ).append(",");
                sMsg.append( nTab32 ).append(")\n");
                sMsg.append("Cell: (").append(nCol).append(",").append(nRow).append(",").append(nTab32).append(")");
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
    ScDocShellRef xDocSh = loadDoc("dataValidity.", FORMAT_ODS);
    ScDocument& rDoc = xDocSh->GetDocument();

    ScAddress aValBaseAddr1( 2,6,0 ); //sheet1
    ScAddress aValBaseAddr2( 2,3,1 ); //sheet2

    //sheet1's expected Data Validation Entry values
    ValDataTestParams aVDTParams1(
        SC_VALID_DECIMAL, SC_COND_GREATER, "3.14", EMPTY_OUSTRING, rDoc,
        aValBaseAddr1, "Too small",
        "The number you are trying to enter is not greater than 3.14! Are you sure you want to enter it anyway?",
        SC_VALERR_WARNING, 1
    );
    //sheet2's expected Data Validation Entry values
    ValDataTestParams aVDTParams2(
        SC_VALID_WHOLE, SC_COND_BETWEEN, "1", "10", rDoc,
        aValBaseAddr2, "Error sheet 2",
        "Must be a whole number between 1 and 10.",
        SC_VALERR_STOP, 2
    );
    //check each sheet's Data Validation Entries
    checkValiditationEntries( aVDTParams1 );
    checkValiditationEntries( aVDTParams2 );

    //expected ranges to be associated with data validity
    ScRange aRange1( 2,2,0, 2,6,0 ); //sheet1
    ScRange aRange2( 2,3,1, 6,7,1 ); //sheet2

    //check each sheet's cells for data validity
    checkCellValidity( aValBaseAddr1, aRange1, rDoc );
    checkCellValidity( aValBaseAddr2, aRange2, rDoc );

    //check each sheet's content
    OUString aCSVFileName1;
    createCSVPath("dataValidity1.", aCSVFileName1);
    testFile(aCSVFileName1, rDoc, 0);

    OUString aCSVFileName2;
    createCSVPath("dataValidity2.", aCSVFileName2);
    testFile(aCSVFileName2, rDoc, 1);

    xDocSh->DoClose();
}

void ScFiltersTest::testDataTableMortgageXLS()
{
    ScDocShellRef xDocSh = loadDoc("data-table/mortgage.", FORMAT_XLS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load the document.", xDocSh.Is());

    ScFormulaOptions aOptions;
    aOptions.SetFormulaSepArg(",");
    aOptions.SetFormulaSepArrayCol(",");
    aOptions.SetFormulaSepArrayRow(";");
    xDocSh->SetFormulaOptions(aOptions);

    ScDocument& rDoc = xDocSh->GetDocument();

    // One-variable table

    if (!checkFormula(rDoc, ScAddress(3,1,0), "PMT(B3/12,B4,-B5)"))
        CPPUNIT_FAIL("Wrong formula!");

    if (!checkFormula(rDoc, ScAddress(3,2,0), "MULTIPLE.OPERATIONS(D$2,$B$3,$C3)"))
        CPPUNIT_FAIL("Wrong formula!");

    if (!checkFormula(rDoc, ScAddress(3,3,0), "MULTIPLE.OPERATIONS(D$2,$B$3,$C4)"))
        CPPUNIT_FAIL("Wrong formula!");

    if (!checkFormula(rDoc, ScAddress(3,4,0), "MULTIPLE.OPERATIONS(D$2,$B$3,$C5)"))
        CPPUNIT_FAIL("Wrong formula!");

    // Two-variable table

    if (!checkFormula(rDoc, ScAddress(2,7,0), "PMT(B9/12,B10,-B11)"))
        CPPUNIT_FAIL("Wrong formula!");

    if (!checkFormula(rDoc, ScAddress(3,8,0), "MULTIPLE.OPERATIONS($C$8,$B$9,$C9,$B$10,D$8)"))
        CPPUNIT_FAIL("Wrong formula!");

    if (!checkFormula(rDoc, ScAddress(3,9,0), "MULTIPLE.OPERATIONS($C$8,$B$9,$C10,$B$10,D$8)"))
        CPPUNIT_FAIL("Wrong formula!");

    if (!checkFormula(rDoc, ScAddress(3,10,0), "MULTIPLE.OPERATIONS($C$8,$B$9,$C11,$B$10,D$8)"))
        CPPUNIT_FAIL("Wrong formula!");

    if (!checkFormula(rDoc, ScAddress(4,8,0), "MULTIPLE.OPERATIONS($C$8,$B$9,$C9,$B$10,E$8)"))
        CPPUNIT_FAIL("Wrong formula!");

    if (!checkFormula(rDoc, ScAddress(4,9,0), "MULTIPLE.OPERATIONS($C$8,$B$9,$C10,$B$10,E$8)"))
        CPPUNIT_FAIL("Wrong formula!");

    if (!checkFormula(rDoc, ScAddress(4,10,0), "MULTIPLE.OPERATIONS($C$8,$B$9,$C11,$B$10,E$8)"))
        CPPUNIT_FAIL("Wrong formula!");

    xDocSh->DoClose();
}

void ScFiltersTest::testDataTableOneVarXLSX()
{
    ScDocShellRef xDocSh = loadDoc("data-table/one-variable.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to load the document.", xDocSh.Is());

    ScFormulaOptions aOptions;
    aOptions.SetFormulaSepArg(",");
    aOptions.SetFormulaSepArrayCol(",");
    aOptions.SetFormulaSepArrayRow(";");
    xDocSh->SetFormulaOptions(aOptions);

    ScDocument& rDoc = xDocSh->GetDocument();

    // Right now, we have a bug that prevents Calc from re-calculating these
    // cells automatically upon file load. We can remove this call if/when we
    // fix the aforementioned bug.
    rDoc.CalcAll();

    // B5:B11 should have multiple operations formula cells.  Just check the
    // top and bottom cells.

    if (!checkFormula(rDoc, ScAddress(1,4,0), "MULTIPLE.OPERATIONS(B$4,$A$2,$A5)"))
        CPPUNIT_FAIL("Wrong formula!");

    CPPUNIT_ASSERT_EQUAL(2.0, rDoc.GetValue(ScAddress(1,4,0)));

    if (!checkFormula(rDoc, ScAddress(1,10,0), "MULTIPLE.OPERATIONS(B$4,$A$2,$A11)"))
        CPPUNIT_FAIL("Wrong formula!");

    CPPUNIT_ASSERT_EQUAL(14.0, rDoc.GetValue(ScAddress(1,10,0)));

    // Likewise, E5:I5 should have multiple operations formula cells.  Just
    // check the left- and right-most cells.

    if (!checkFormula(rDoc, ScAddress(4,4,0), "MULTIPLE.OPERATIONS($D5,$B$2,E$4)"))
        CPPUNIT_FAIL("Wrong formula!");

    CPPUNIT_ASSERT_EQUAL(10.0, rDoc.GetValue(ScAddress(4,4,0)));

    if (!checkFormula(rDoc, ScAddress(8,4,0), "MULTIPLE.OPERATIONS($D5,$B$2,I$4)"))
        CPPUNIT_FAIL("Wrong formula!");

    CPPUNIT_ASSERT_EQUAL(50.0, rDoc.GetValue(ScAddress(8,4,0)));

    xDocSh->DoClose();
}

void ScFiltersTest::testDataTableMultiTableXLSX()
{
    ScDocShellRef xDocSh = loadDoc("data-table/multi-table.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to load the document.", xDocSh.Is());

    ScFormulaOptions aOptions;
    aOptions.SetFormulaSepArg(",");
    aOptions.SetFormulaSepArrayCol(",");
    aOptions.SetFormulaSepArrayRow(";");
    xDocSh->SetFormulaOptions(aOptions);

    ScDocument& rDoc = xDocSh->GetDocument();

    // Right now, we have a bug that prevents Calc from re-calculating these
    // cells automatically upon file load. We can remove this call if/when we
    // fix the aforementioned bug.
    rDoc.CalcAll();

    // B4:M15 should have multiple operations formula cells.  We'll just check
    // the top-left and bottom-right ones.

    if (!checkFormula(rDoc, ScAddress(1,3,0), "MULTIPLE.OPERATIONS($A$3,$E$1,$A4,$D$1,B$3)"))
        CPPUNIT_FAIL("Wrong formula!");

    CPPUNIT_ASSERT_EQUAL(1.0, rDoc.GetValue(ScAddress(1,3,0)));

    if (!checkFormula(rDoc, ScAddress(12,14,0), "MULTIPLE.OPERATIONS($A$3,$E$1,$A15,$D$1,M$3)"))
        CPPUNIT_FAIL("Wrong formula!");

    CPPUNIT_ASSERT_EQUAL(144.0, rDoc.GetValue(ScAddress(12,14,0)));

    xDocSh->DoClose();
}

void ScFiltersTest::testBrokenQuotesCSV()
{
    const OUString aFileNameBase("fdo48621_broken_quotes.");
    OUString aFileExtension(getFileFormats()[FORMAT_CSV].pName, strlen(getFileFormats()[FORMAT_CSV].pName), RTL_TEXTENCODING_UTF8 );
    OUString aFilterName(getFileFormats()[FORMAT_CSV].pFilterName, strlen(getFileFormats()[FORMAT_CSV].pFilterName), RTL_TEXTENCODING_UTF8) ;
    OUString aFileName;
    createFileURL(aFileNameBase, aFileExtension, aFileName);
    OUString aFilterType(getFileFormats()[FORMAT_CSV].pTypeName, strlen(getFileFormats()[FORMAT_CSV].pTypeName), RTL_TEXTENCODING_UTF8);
    std::cout << getFileFormats()[FORMAT_CSV].pName << " Test" << std::endl;

    SfxFilterFlags nFormatType = getFileFormats()[FORMAT_CSV].nFormatType;
    SotClipboardFormatId nClipboardId = bool(nFormatType) ? SotClipboardFormatId::STARCALC_8 : SotClipboardFormatId::NONE;
    ScDocShellRef xDocSh = ScBootstrapFixture::load(aFileName, aFilterName, OUString(), aFilterType,
        nFormatType, nClipboardId);

    CPPUNIT_ASSERT_MESSAGE("Failed to load fdo48621_broken_quotes.csv", xDocSh.Is());
    ScDocument& rDoc = xDocSh->GetDocument();

    OUString aSheet2CSV("fdo48621_broken_quotes_exported.");
    OUString aCSVPath;
    createCSVPath( aSheet2CSV, aCSVPath );
    // fdo#48621
    testFile( aCSVPath, rDoc, 0, PureString);

    xDocSh->DoClose();
}

void ScFiltersTest::testCellValueXLSX()
{
    const OUString aFileNameBase("cell-value.");
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

    CPPUNIT_ASSERT_MESSAGE("Failed to load cell-value.xlsx", xDocSh.Is());
    ScDocument& rDoc = xDocSh->GetDocument();

    OUString aCSVPath;
    createCSVPath( aFileNameBase, aCSVPath );
    testFile( aCSVPath, rDoc, 0 );

    xDocSh->DoClose();
}

void ScFiltersTest::testRowIndex1BasedXLSX()
{
    ScDocShellRef xDocSh = loadDoc("row-index-1-based.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.Is());
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
    CPPUNIT_ASSERT(pText->GetParagraphCount() == 3);
    aStr = pText->GetText(0);
    CPPUNIT_ASSERT_EQUAL(OUString("This is a longer Text."), aStr);
    aStr = pText->GetText(1);
    CPPUNIT_ASSERT_EQUAL(OUString("Second line."), aStr);
    aStr = pText->GetText(2);
    CPPUNIT_ASSERT_EQUAL(OUString("Third line."), aStr);

    xDocSh->DoClose();
}

#if !defined(MACOSX) && !defined(DRAGONFLY) && !defined(WNT)
void ScFiltersTest::testPassword_Impl(const OUString& aFileNameBase)
{
    OUString aFileExtension(getFileFormats()[0].pName, strlen(getFileFormats()[0].pName), RTL_TEXTENCODING_UTF8 );
    OUString aFilterName(getFileFormats()[0].pFilterName, strlen(getFileFormats()[0].pFilterName), RTL_TEXTENCODING_UTF8) ;
    OUString aFileName;
    createFileURL(aFileNameBase, aFileExtension, aFileName);
    OUString aFilterType(getFileFormats()[0].pTypeName, strlen(getFileFormats()[0].pTypeName), RTL_TEXTENCODING_UTF8);

    SotClipboardFormatId nFormat = SotClipboardFormatId::STARCALC_8;
    SfxFilter* aFilter = new SfxFilter(
        aFilterName,
        OUString(), getFileFormats()[0].nFormatType, nFormat, aFilterType, 0, OUString(),
        OUString(), OUString("private:factory/scalc*") );
    aFilter->SetVersion(SOFFICE_FILEFORMAT_CURRENT);

    ScDocShellRef xDocSh = new ScDocShell;
    SfxMedium* pMedium = new SfxMedium(aFileName, STREAM_STD_READWRITE);
    SfxItemSet* pSet = pMedium->GetItemSet();
    pSet->Put(SfxStringItem(SID_PASSWORD, OUString("test")));
    pMedium->SetFilter(aFilter);
    if (!xDocSh->DoLoad(pMedium))
    {
        xDocSh->DoClose();
        // load failed.
        xDocSh.Clear();
    }

    CPPUNIT_ASSERT_MESSAGE("Failed to load password.ods", xDocSh.Is());
    xDocSh->GetDocument();
    xDocSh->DoClose();
}

void ScFiltersTest::testPasswordNew()
{
    //tests opening a file with new password algorithm
    const OUString aFileNameBase("password.");
    testPassword_Impl(aFileNameBase);
}

void ScFiltersTest::testPasswordOld()
{
    //tests opening a file with old password algorithm
    const OUString aFileNameBase("passwordOld.");
    testPassword_Impl(aFileNameBase);
}

void ScFiltersTest::testPasswordWrongSHA()
{
    //tests opening a file wrongly using the new password algorithm
    //in a sxc with the key algorithm missing
    const OUString aFileNameBase("passwordWrongSHA.");
    testPassword_Impl(aFileNameBase);
}
#endif

void ScFiltersTest::testControlImport()
{
    ScDocShellRef xDocSh = loadDoc("singlecontrol.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to load singlecontrol.xlsx", xDocSh.Is());

    uno::Reference< frame::XModel > xModel = xDocSh->GetModel();
    uno::Reference< sheet::XSpreadsheetDocument > xDoc(xModel, UNO_QUERY_THROW);
    uno::Reference< container::XIndexAccess > xIA(xDoc->getSheets(), UNO_QUERY_THROW);
    uno::Reference< drawing::XDrawPageSupplier > xDrawPageSupplier( xIA->getByIndex(0), UNO_QUERY_THROW);
    uno::Reference< container::XIndexAccess > xIA_DrawPage(xDrawPageSupplier->getDrawPage(), UNO_QUERY_THROW);
    uno::Reference< drawing::XControlShape > xControlShape(xIA_DrawPage->getByIndex(0), UNO_QUERY_THROW);

    CPPUNIT_ASSERT(xControlShape.is());
    xDocSh->DoClose();
}

void ScFiltersTest::testChartImportODS()
{
    ScDocShellRef xDocSh = loadDoc("chart-import-basic.", FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load chart-import-basic.ods.", xDocSh.Is());

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

    CPPUNIT_ASSERT_MESSAGE("Data series title cell not found.", aRanges.In(ScAddress(1,0,3))); // B1 on Title
    CPPUNIT_ASSERT_MESSAGE("Data series label range not found.", aRanges.In(ScRange(0,1,2,0,3,2))); // A2:A4 on Data
    CPPUNIT_ASSERT_MESSAGE("Data series value range not found.", aRanges.In(ScRange(1,1,2,1,3,2))); // B2:B4 on Data

    xDocSh->DoClose();
}

void ScFiltersTest::testChartImportXLS()
{
    ScDocShellRef xDocSh = loadDoc("chartx.", FORMAT_XLS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load chartx.xls.", xDocSh.Is());

    ScDocument& rDoc = xDocSh->GetDocument();

    // Retrieve the chart object instance from the 2nd page (for the 2nd sheet).
    const SdrOle2Obj* pOleObj = getSingleChartObject(rDoc, 0);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve a chart object from the 2nd sheet.", pOleObj);

    CPPUNIT_ASSERT_EQUAL(11137L, pOleObj->GetLogicRect().getWidth());
    CPPUNIT_ASSERT(8640L > pOleObj->GetLogicRect().getHeight());

    xDocSh->DoClose();
}

void ScFiltersTest::testNumberFormatHTML()
{
    ScDocShellRef xDocSh = loadDoc("numberformat.", FORMAT_HTML);
    CPPUNIT_ASSERT_MESSAGE("Failed to load numberformat.html", xDocSh.Is());

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
    ScDocShellRef xDocSh = loadDoc("numberformat.", FORMAT_CSV);
    CPPUNIT_ASSERT_MESSAGE("Failed to load numberformat.csv", xDocSh.Is());

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
    ScDocShellRef xDocSh = loadDoc("cell-anchored-shapes.", FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load cell-anchored-shapes.ods", xDocSh.Is());

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
        CPPUNIT_ASSERT_MESSAGE("Bounding rectangle should have been calculated upon import.", !pData->maLastRect.IsEmpty());
    }

    xDocSh->DoClose();
}

void ScFiltersTest::testCellAnchoredHiddenShapesXLSX()
{
    ScDocShellRef xDocSh = loadDoc("cell-anchored-hidden-shapes.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to load cell-anchored-shapes.xlsx", xDocSh.Is());

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
}

namespace {

class FindDimByName : public std::unary_function<const ScDPSaveDimension*, bool>
{
    OUString maName;
public:
    explicit FindDimByName(const OUString& rName) : maName(rName) {}

    bool operator() (const ScDPSaveDimension* p) const
    {
        return p && p->GetName() == maName;
    }
};

bool hasDimension(const std::vector<const ScDPSaveDimension*>& rDims, const OUString& aName)
{
    return std::any_of(rDims.begin(), rDims.end(), FindDimByName(aName));
}

}

void ScFiltersTest::testPivotTableBasicODS()
{
    ScDocShellRef xDocSh = loadDoc("pivot-table-basic.", FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load pivot-table-basic.ods", xDocSh.Is());

    ScDocument& rDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "There should be exactly two sheets.", sal_Int16(2),
        rDoc.GetTableCount());

    ScDPCollection* pDPs = rDoc.GetDPCollection();
    CPPUNIT_ASSERT_MESSAGE("Failed to get a live ScDPCollection instance.", pDPs);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "There should be exactly one pivot table instance.", size_t(1),
        pDPs->GetCount());

    const ScDPObject* pDPObj = &(*pDPs)[0];
    CPPUNIT_ASSERT_MESSAGE("Failed to get an pivot table object.", pDPObj);
    const ScDPSaveData* pSaveData = pDPObj->GetSaveData();
    CPPUNIT_ASSERT_MESSAGE("Failed to get ScDPSaveData instance.", pSaveData);
    std::vector<const ScDPSaveDimension*> aDims;

    // Row fields
    pSaveData->GetAllDimensionsByOrientation(sheet::DataPilotFieldOrientation_ROW, aDims);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        ("There should be exactly 3 row fields (2 normal dimensions and 1"
         " layout dimension)."),
        std::vector<ScDPSaveDimension const *>::size_type(3), aDims.size());
    CPPUNIT_ASSERT_MESSAGE("Dimension expected, but not found.", hasDimension(aDims, "Row1"));
    CPPUNIT_ASSERT_MESSAGE("Dimension expected, but not found.", hasDimension(aDims, "Row2"));
    const ScDPSaveDimension* pDataLayout = pSaveData->GetExistingDataLayoutDimension();
    CPPUNIT_ASSERT_MESSAGE("There should be a data layout field as a row field.",
                           pDataLayout && pDataLayout->GetOrientation() == sheet::DataPilotFieldOrientation_ROW);

    // Column fields
    pSaveData->GetAllDimensionsByOrientation(sheet::DataPilotFieldOrientation_COLUMN, aDims);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "There should be exactly 2 column fields.",
        std::vector<ScDPSaveDimension const *>::size_type(2), aDims.size());
    CPPUNIT_ASSERT_MESSAGE("Dimension expected, but not found.", hasDimension(aDims, "Col1"));
    CPPUNIT_ASSERT_MESSAGE("Dimension expected, but not found.", hasDimension(aDims, "Col2"));

    // Page fields
    pSaveData->GetAllDimensionsByOrientation(sheet::DataPilotFieldOrientation_PAGE, aDims);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "There should be exactly 2 page fields.",
        std::vector<ScDPSaveDimension const *>::size_type(2), aDims.size());
    CPPUNIT_ASSERT_MESSAGE("Dimension expected, but not found.", hasDimension(aDims, "Page1"));
    CPPUNIT_ASSERT_MESSAGE("Dimension expected, but not found.", hasDimension(aDims, "Page2"));

    // Check the data field.
    pSaveData->GetAllDimensionsByOrientation(sheet::DataPilotFieldOrientation_DATA, aDims);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "There should be exactly 1 data field.",
        std::vector<ScDPSaveDimension const *>::size_type(1), aDims.size());
    const ScDPSaveDimension* pDim = aDims.back();
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Function for the data field should be COUNT.",
        sal_uInt16(sheet::GeneralFunction_COUNT), pDim->GetFunction());

    xDocSh->DoClose();
}

void ScFiltersTest::testPivotTableNamedRangeSourceODS()
{
    ScDocShellRef xDocSh = loadDoc("pivot-table-named-range-source.", FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load pivot-table-named-range-source.ods", xDocSh.Is());

    ScDocument& rDoc = xDocSh->GetDocument();

    ScDPCollection* pDPs = rDoc.GetDPCollection();
    CPPUNIT_ASSERT(pDPs->GetCount() == 1);

    ScDPObject* pDP = &(*pDPs)[0];
    CPPUNIT_ASSERT(pDP);

    // Make sure this pivot table is based on a named range source.
    const ScSheetSourceDesc* pDesc = pDP->GetSheetDesc();
    CPPUNIT_ASSERT(pDesc);
    CPPUNIT_ASSERT_EQUAL(OUString("MyRange"), pDesc->GetRangeName());

    sal_uInt16 nOrient;
    long nDim = pDP->GetHeaderDim(ScAddress(0,1,1), nOrient);
    CPPUNIT_ASSERT_MESSAGE("Failed to detect header dimension.", nDim == 0);
    CPPUNIT_ASSERT_MESSAGE("This dimension should be a page dimension.",
                           nOrient == sheet::DataPilotFieldOrientation_PAGE);

    xDocSh->DoClose();
}

namespace {

bool checkVisiblePageFieldMember( const ScDPSaveDimension::MemberList& rMembers, const OUString& rVisibleMember )
{
    ScDPSaveDimension::MemberList::const_iterator it = rMembers.begin(), itEnd = rMembers.end();
    bool bFound = false;
    for (; it != itEnd; ++it)
    {
        const ScDPSaveMember* pMem = *it;
        if (pMem->GetName() == rVisibleMember)
        {
            bFound = true;
            if (!pMem->GetIsVisible())
                // This member is supposed to be visible.  Fail.
                return false;
        }
        else
        {
            if (pMem->GetIsVisible())
                // This member is supposed to be hidden. Not good.
                return false;
        }
    }

    return bFound;
}

}

void ScFiltersTest::testPivotTableSharedCacheGroupODS()
{
    ScDocShellRef xDocSh = loadDoc("pivot-table-shared-cache-with-group.", FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load file", xDocSh.Is());
    ScDocument& rDoc = xDocSh->GetDocument();

    // Make sure that page field's visibility settings are loaded correctly.

    ScDPObject* pDPObj = rDoc.GetDPAtCursor(0, 0, 1); // A1 on 2nd sheet
    CPPUNIT_ASSERT_MESSAGE("There should be a pivot table here.", pDPObj);
    ScDPSaveData* pSaveData = pDPObj->GetSaveData();
    CPPUNIT_ASSERT_MESSAGE("Save data is expected.", pSaveData);
    ScDPSaveDimension* pDim = pSaveData->GetExistingDimensionByName("Project Name");
    CPPUNIT_ASSERT_MESSAGE("Failed to get page field named 'Project Name'.", pDim);
    const ScDPSaveDimension::MemberList* pMembers = &pDim->GetMembers();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(14), pMembers->size());
    CPPUNIT_ASSERT_MESSAGE("Incorrect member visibility.", checkVisiblePageFieldMember(*pMembers, "APL-01-1"));

    pDPObj = rDoc.GetDPAtCursor(0, 1, 2); // A2 on 3rd sheet
    CPPUNIT_ASSERT_MESSAGE("There should be a pivot table here.", pDPObj);
    pSaveData = pDPObj->GetSaveData();
    CPPUNIT_ASSERT_MESSAGE("Save data is expected.", pSaveData);
    pDim = pSaveData->GetExistingDimensionByName("Project Name");
    CPPUNIT_ASSERT_MESSAGE("Failed to get page field named 'Project Name'.", pDim);
    pMembers = &pDim->GetMembers();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(14), pMembers->size());
    CPPUNIT_ASSERT_MESSAGE("Incorrect member visibility.", checkVisiblePageFieldMember(*pMembers, "VEN-01-1"));

    // These two pivot tables share the same data range. We should only have
    // one pivot cache.
    ScDPCollection* pDPs = rDoc.GetDPCollection();
    ScDPCollection::SheetCaches& rSheetCaches = pDPs->GetSheetCaches();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rSheetCaches.size());

    // Make sure that the cache contains all group field data upon load.
    const ScSheetSourceDesc* pDesc = pDPObj->GetSheetDesc();
    CPPUNIT_ASSERT_MESSAGE("Failed to get the pivot source description instance.", pDesc);
    const ScDPCache* pCache = rSheetCaches.getExistingCache(pDesc->GetSourceRange());
    CPPUNIT_ASSERT_MESSAGE("Pivot cache should exist for this range.", pCache);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(9), pCache->GetFieldCount());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), pCache->GetGroupFieldCount());

    SCCOL nDim = pCache->GetDimensionIndex("StartDate");
    CPPUNIT_ASSERT_MESSAGE("Dimension 'StartDate' doesn't exist in the cache.", nDim >= 0);
    sal_Int32 nGrpType = pCache->GetGroupType(nDim);
    CPPUNIT_ASSERT_EQUAL(sheet::DataPilotFieldGroupBy::DAYS, nGrpType);
    const ScDPNumGroupInfo* pInfo = pCache->GetNumGroupInfo(nDim);
    CPPUNIT_ASSERT_MESSAGE("Number group info doesn't exist in cache for 'StartDate'.", pInfo);

    // We should have two additional group fields and one should be years and
    // the other should be month.  The order is not guaranteed.

    bool bHasYears = false;
    bool bHasMonths = false;
    std::vector<SCROW> aMemberIds;

    for (long nGrpDim = 9; nGrpDim <= 10; ++nGrpDim)
    {
        nGrpType = pCache->GetGroupType(nGrpDim);
        switch (nGrpType)
        {
            case sheet::DataPilotFieldGroupBy::MONTHS:
            {
                bHasMonths = true;
                aMemberIds.clear();
                pCache->GetGroupDimMemberIds(nGrpDim, aMemberIds);

                // There should be a total of 14 items for the month group: 12
                // months plus the start and end value items.

                CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(14), aMemberIds.size());

                std::vector<sal_Int32> aGrpValues;
                for (size_t i = 0, n = aMemberIds.size(); i < n; ++i)
                {
                    const ScDPItemData* pItem = pCache->GetItemDataById(nGrpDim, aMemberIds[i]);
                    CPPUNIT_ASSERT_MESSAGE("Failed to get pivot item.", pItem);
                    CPPUNIT_ASSERT_EQUAL(ScDPItemData::GroupValue, pItem->GetType());
                    ScDPItemData::GroupValueAttr aGrpVal = pItem->GetGroupValue();
                    CPPUNIT_ASSERT_EQUAL(sheet::DataPilotFieldGroupBy::MONTHS, aGrpVal.mnGroupType);
                    aGrpValues.push_back(aGrpVal.mnValue);
                }

                std::sort(aGrpValues.begin(), aGrpValues.end());
                std::vector<sal_Int32> aChecks;
                aChecks.push_back(ScDPItemData::DateFirst);
                for (sal_Int32 i = 1; i <= 12; ++i)
                    aChecks.push_back(i); // January through December.
                aChecks.push_back(ScDPItemData::DateLast);
                CPPUNIT_ASSERT_MESSAGE("Unexpected group values for the month group.", aGrpValues == aChecks);
            }
            break;
            case sheet::DataPilotFieldGroupBy::YEARS:
            {
                bHasYears = true;
                aMemberIds.clear();
                pCache->GetGroupDimMemberIds(nGrpDim, aMemberIds);

                // There should be a total of 4 items and they should be 2012,
                // 2013 and the start and end value items.

                CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), aMemberIds.size());

                std::vector<sal_Int32> aGrpValues;
                for (size_t i = 0, n = aMemberIds.size(); i < n; ++i)
                {
                    const ScDPItemData* pItem = pCache->GetItemDataById(nGrpDim, aMemberIds[i]);
                    CPPUNIT_ASSERT_MESSAGE("Failed to get pivot item.", pItem);
                    CPPUNIT_ASSERT_EQUAL(ScDPItemData::GroupValue, pItem->GetType());
                    ScDPItemData::GroupValueAttr aGrpVal = pItem->GetGroupValue();
                    CPPUNIT_ASSERT_EQUAL(sheet::DataPilotFieldGroupBy::YEARS, aGrpVal.mnGroupType);
                    aGrpValues.push_back(aGrpVal.mnValue);
                }

                std::sort(aGrpValues.begin(), aGrpValues.end());
                std::vector<sal_Int32> aChecks;
                aChecks.push_back(ScDPItemData::DateFirst);
                aChecks.push_back(2012);
                aChecks.push_back(2013);
                aChecks.push_back(ScDPItemData::DateLast);
                CPPUNIT_ASSERT_MESSAGE("Unexpected group values for the year group.", aGrpValues == aChecks);
            }
            break;
            default:
                ;
        }
    }

    CPPUNIT_ASSERT_MESSAGE("Pivot cache doesn't have an additional year group.", bHasYears);
    CPPUNIT_ASSERT_MESSAGE("Pivot cache doesn't have an additional month group.", bHasMonths);

    xDocSh->DoClose();
}

void ScFiltersTest::testGetPivotDataXLS()
{
    ScDocShellRef xDocSh = loadDoc("pivot-getpivotdata.", FORMAT_XLS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load file", xDocSh.Is());
    ScDocument& rDoc = xDocSh->GetDocument();
    rDoc.CalcAll();

    // Check GETPIVOTDATA results in E3:E20. Expected results are given in
    // F3:F20.

    for (SCROW nRow = 2; nRow <= 19; ++nRow)
        CPPUNIT_ASSERT_EQUAL(rDoc.GetValue(ScAddress(4,nRow,1)), rDoc.GetValue(ScAddress(5,nRow,1)));

    xDocSh->DoClose();
}

void ScFiltersTest::testPivotTableSharedGroupXLSX()
{
    ScDocShellRef xDocSh = loadDoc("pivot-table/shared_group.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to load file", xDocSh.Is());
    ScDocument& rDoc = xDocSh->GetDocument();
    rDoc.CalcAll();

    // Check whether right group names are imported for both tables
    // First table
    CPPUNIT_ASSERT_EQUAL(OUString("Csoport1"), rDoc.GetString(ScAddress(0,2,0)));
    CPPUNIT_ASSERT_EQUAL(OUString("Csoport2"), rDoc.GetString(ScAddress(0,3,0)));
    CPPUNIT_ASSERT_EQUAL(OUString("Csoport3"), rDoc.GetString(ScAddress(0,4,0)));
    CPPUNIT_ASSERT_EQUAL(OUString("16"), rDoc.GetString(ScAddress(0,5,0)));
    CPPUNIT_ASSERT_EQUAL(OUString("17"), rDoc.GetString(ScAddress(0,6,0)));
    CPPUNIT_ASSERT_EQUAL(OUString("18"), rDoc.GetString(ScAddress(0,7,0)));

    // Second table
    CPPUNIT_ASSERT_EQUAL(OUString("Csoport1"), rDoc.GetString(ScAddress(0,12,0)));
    CPPUNIT_ASSERT_EQUAL(OUString("Csoport2"), rDoc.GetString(ScAddress(0,13,0)));
    CPPUNIT_ASSERT_EQUAL(OUString("Csoport3"), rDoc.GetString(ScAddress(0,14,0)));
    CPPUNIT_ASSERT_EQUAL(OUString("16"), rDoc.GetString(ScAddress(0,15,0)));
    CPPUNIT_ASSERT_EQUAL(OUString("17"), rDoc.GetString(ScAddress(0,16,0)));
    CPPUNIT_ASSERT_EQUAL(OUString("18"), rDoc.GetString(ScAddress(0,17,0)));

    xDocSh->DoClose();
}

void ScFiltersTest::testPivotTableSharedDateGroupXLSX()
{
    ScDocShellRef xDocSh = loadDoc("pivot-table/shared-dategroup.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to load file", xDocSh.Is());
    ScDocument& rDoc = xDocSh->GetDocument();

    // Check whether right date labels are imported for both tables
    // First table
    CPPUNIT_ASSERT_EQUAL(OUString("1965"), rDoc.GetString(ScAddress(0,4,1)));
    CPPUNIT_ASSERT_EQUAL(OUString("1989"), rDoc.GetString(ScAddress(0,5,1)));
    CPPUNIT_ASSERT_EQUAL(OUString("2000"), rDoc.GetString(ScAddress(0,6,1)));
    CPPUNIT_ASSERT_EQUAL(OUString("2004"), rDoc.GetString(ScAddress(0,7,1)));
    // TODO: check why this fails with 2005
    // CPPUNIT_ASSERT_EQUAL(OUString("2007"), rDoc.GetString(ScAddress(0,8,1)));

    // Second table
    CPPUNIT_ASSERT_EQUAL(OUString("1965"), rDoc.GetString(ScAddress(5,4,1)));
    CPPUNIT_ASSERT_EQUAL(OUString("1989"), rDoc.GetString(ScAddress(5,5,1)));
    CPPUNIT_ASSERT_EQUAL(OUString("2000"), rDoc.GetString(ScAddress(5,6,1)));
    CPPUNIT_ASSERT_EQUAL(OUString("2004"), rDoc.GetString(ScAddress(5,7,1)));
    // TODO: check why this fails with 2005
    // CPPUNIT_ASSERT_EQUAL(OUString("2007"), rDoc.GetString(ScAddress(5,8,1)));

    // There should be exactly 2 pivot tables and 1 cache.
    ScDPCollection* pDPs = rDoc.GetDPCollection();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), pDPs->GetCount());

    ScDPCollection::SheetCaches& rSheetCaches = pDPs->GetSheetCaches();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rSheetCaches.size());

    const ScDPCache* pCache = rSheetCaches.getExistingCache(ScRange(0,0,0,9,24,0));
    CPPUNIT_ASSERT_MESSAGE("Pivot cache is expected for A1:J25 on the first sheet.", pCache);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(10), pCache->GetFieldCount());

    xDocSh->DoClose();
}

void ScFiltersTest::testPivotTableSharedNestedDateGroupXLSX()
{
    ScDocShellRef xDocSh = loadDoc("pivot-table/shared-nested-dategroup.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to load file", xDocSh.Is());
    ScDocument& rDoc = xDocSh->GetDocument();

    // Check whether right date groups are imported for both tables
    // First table
    CPPUNIT_ASSERT_EQUAL(OUString("Years"), rDoc.GetString(ScAddress(0,3,1)));
    CPPUNIT_ASSERT_EQUAL(OUString("1965"), rDoc.GetString(ScAddress(0,4,1)));
    CPPUNIT_ASSERT_EQUAL(OUString("1989"), rDoc.GetString(ScAddress(0,11,1)));
    CPPUNIT_ASSERT_EQUAL(OUString("2000"), rDoc.GetString(ScAddress(0,18,1)));
    CPPUNIT_ASSERT_EQUAL(OUString("2004"), rDoc.GetString(ScAddress(0,21,1)));
    // TODO: check why this fails with the empty string
    //CPPUNIT_ASSERT_EQUAL(OUString("2007"), rDoc.GetString(ScAddress(0,32,1)));
    CPPUNIT_ASSERT_EQUAL(OUString("Quarters"), rDoc.GetString(ScAddress(1,3,1)));

    // Second table
    CPPUNIT_ASSERT_EQUAL(OUString("Years"), rDoc.GetString(ScAddress(6,3,1)));
    CPPUNIT_ASSERT_EQUAL(OUString("1965"), rDoc.GetString(ScAddress(6,4,1)));
    CPPUNIT_ASSERT_EQUAL(OUString("1989"), rDoc.GetString(ScAddress(6,11,1)));
    CPPUNIT_ASSERT_EQUAL(OUString("2000"), rDoc.GetString(ScAddress(6,18,1)));
    CPPUNIT_ASSERT_EQUAL(OUString("2004"), rDoc.GetString(ScAddress(6,21,1)));
    // TODO: check why this fails with the empty string
    //CPPUNIT_ASSERT_EQUAL(OUString("2007"), rDoc.GetString(ScAddress(6,31,1)));
    CPPUNIT_ASSERT_EQUAL(OUString("Quarters"), rDoc.GetString(ScAddress(7,3,1)));

    // There should be exactly 2 pivot tables and 1 cache.
    ScDPCollection* pDPs = rDoc.GetDPCollection();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), pDPs->GetCount());

    ScDPCollection::SheetCaches& rSheetCaches = pDPs->GetSheetCaches();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rSheetCaches.size());

    const ScDPCache* pCache = rSheetCaches.getExistingCache(ScRange(0,0,0,9,24,0));
    CPPUNIT_ASSERT_MESSAGE("Pivot cache is expected for A1:J25 on the first sheet.", pCache);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(10), pCache->GetFieldCount());
    // Two new group field is created
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), pCache->GetGroupFieldCount());

    xDocSh->DoClose();
}

void ScFiltersTest::testPivotTableSharedNumGroupXLSX()
{
    ScDocShellRef xDocSh = loadDoc("pivot-table/shared-numgroup.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to load file", xDocSh.Is());
    ScDocument& rDoc = xDocSh->GetDocument();

    // Check whether right number groups are imported for both tables
    // First table
    CPPUNIT_ASSERT_EQUAL(OUString("32674-47673"), rDoc.GetString(ScAddress(0,4,1)));
    CPPUNIT_ASSERT_EQUAL(OUString("47674-62673"), rDoc.GetString(ScAddress(0,5,1)));
    CPPUNIT_ASSERT_EQUAL(OUString("62674-77673"), rDoc.GetString(ScAddress(0,6,1)));
    CPPUNIT_ASSERT_EQUAL(OUString("77674-92673"), rDoc.GetString(ScAddress(0,7,1)));
    CPPUNIT_ASSERT_EQUAL(OUString("92674-107673"), rDoc.GetString(ScAddress(0,8,1)));

    // Second table
    CPPUNIT_ASSERT_EQUAL(OUString("32674-47673"), rDoc.GetString(ScAddress(5,4,1)));
    CPPUNIT_ASSERT_EQUAL(OUString("47674-62673"), rDoc.GetString(ScAddress(5,5,1)));
    CPPUNIT_ASSERT_EQUAL(OUString("62674-77673"), rDoc.GetString(ScAddress(5,6,1)));
    CPPUNIT_ASSERT_EQUAL(OUString("77674-92673"), rDoc.GetString(ScAddress(5,7,1)));
    CPPUNIT_ASSERT_EQUAL(OUString("92674-107673"), rDoc.GetString(ScAddress(5,8,1)));

    // There should be exactly 2 pivot tables and 1 cache.
    ScDPCollection* pDPs = rDoc.GetDPCollection();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), pDPs->GetCount());

    ScDPCollection::SheetCaches& rSheetCaches = pDPs->GetSheetCaches();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rSheetCaches.size());

    const ScDPCache* pCache = rSheetCaches.getExistingCache(ScRange(0,0,0,9,24,0));
    CPPUNIT_ASSERT_MESSAGE("Pivot cache is expected for A1:J25 on the first sheet.", pCache);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(10), pCache->GetFieldCount());

    xDocSh->DoClose();
}

void ScFiltersTest::testRowHeightODS()
{
    ScDocShellRef xDocSh = loadDoc("row-height-import.", FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load row-height-import.ods", xDocSh.Is());

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
    ScDocShellRef xDocSh = loadDoc("rich-text-cells.", FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load rich-text-cells.ods", xDocSh.Is());
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
        const SvxWeightItem& rWeightItem =
            static_cast<const SvxWeightItem&>(pAttr->GetItem(ATTR_FONT_WEIGHT));
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
    std::vector<EECharAttrib>::const_iterator it = aAttribs.begin(), itEnd = aAttribs.end();
    {
        bool bHasBold = false, bHasItalic = false;
        for (; it != itEnd; ++it)
        {
            OUString aSeg = aParaText.copy(it->nStart, it->nEnd - it->nStart);
            const SfxPoolItem* pAttr = it->pAttr;
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
        CPPUNIT_ASSERT_MESSAGE("This sentence is expected to have both bold and italic sequences.", bHasBold && bHasItalic);
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
        bool bHasBold = false;
        for (it = aAttribs.begin(), itEnd = aAttribs.end(); it != itEnd; ++it)
        {
            if (it->pAttr->Which() == EE_CHAR_WEIGHT)
            {
                const SvxWeightItem& rItem = static_cast<const SvxWeightItem&>(*it->pAttr);
                bHasBold = (rItem.GetWeight() == WEIGHT_BOLD);
                if (bHasBold)
                    break;
            }
        }
        CPPUNIT_ASSERT_MESSAGE("First line should be bold.", bHasBold);
    }

    // second line is italic.
    pEditText->GetCharAttribs(1, aAttribs);
    bool bHasItalic = false;
    for (it = aAttribs.begin(), itEnd = aAttribs.end(); it != itEnd; ++it)
    {
        if (it->pAttr->Which() == EE_CHAR_ITALIC)
        {
            const SvxPostureItem& rItem = static_cast<const SvxPostureItem&>(*it->pAttr);
            bHasItalic = (rItem.GetPosture() == ITALIC_NORMAL);
            if (bHasItalic)
                break;
        }
    }
    CPPUNIT_ASSERT_MESSAGE("Second line should be italic.", bHasItalic);

    // third line is underlined.
    pEditText->GetCharAttribs(2, aAttribs);
    bool bHasUnderline = false;
    for (it = aAttribs.begin(), itEnd = aAttribs.end(); it != itEnd; ++it)
    {
        if (it->pAttr->Which() == EE_CHAR_UNDERLINE)
        {
            const SvxUnderlineItem& rItem = static_cast<const SvxUnderlineItem&>(*it->pAttr);
            bHasUnderline = (rItem.GetLineStyle() == UNDERLINE_SINGLE);
            if (bHasUnderline)
                break;
        }
    }
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
    CPPUNIT_ASSERT_MESSAGE("Failed to get the URL data.", pData && pData->GetClassId() == text::textfield::Type::URL);
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
        bool bHasBold = false;
        for (it = aAttribs.begin(), itEnd = aAttribs.end(); it != itEnd; ++it)
        {
            if (it->pAttr->Which() == EE_CHAR_WEIGHT)
            {
                const SvxWeightItem& rItem = static_cast<const SvxWeightItem&>(*it->pAttr);
                bHasBold = (rItem.GetWeight() == WEIGHT_BOLD);
                if (bHasBold)
                {
                    OUString aSeg = aParaText.copy(it->nStart, it->nEnd - it->nStart);
                    CPPUNIT_ASSERT_EQUAL(OUString("e     t"), aSeg);
                    break;
                }
            }
        }
        CPPUNIT_ASSERT_MESSAGE("Expected a bold sequence.", bHasBold);
    }

    xDocSh->DoClose();
}

void ScFiltersTest::testDataBarODS()
{
    ScDocShellRef xDocSh = loadDoc("databar.", FORMAT_ODS);
    CPPUNIT_ASSERT(xDocSh.Is());

    ScDocument& rDoc = xDocSh->GetDocument();
    testDataBar_Impl(rDoc);

    xDocSh->DoClose();
}

void ScFiltersTest::testDataBarXLSX()
{
    ScDocShellRef xDocSh = loadDoc("databar.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.Is());

    ScDocument& rDoc = xDocSh->GetDocument();
    testDataBar_Impl(rDoc);

    xDocSh->DoClose();
}

void ScFiltersTest::testColorScaleODS()
{
    ScDocShellRef xDocSh = loadDoc("colorscale.", FORMAT_ODS);
    CPPUNIT_ASSERT(xDocSh.Is());
    ScDocument& rDoc = xDocSh->GetDocument();

    testColorScale2Entry_Impl(rDoc);
    testColorScale3Entry_Impl(rDoc);

    xDocSh->DoClose();
}

void ScFiltersTest::testColorScaleXLSX()
{
    ScDocShellRef xDocSh = loadDoc("colorscale.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.Is());
    ScDocument& rDoc = xDocSh->GetDocument();

    testColorScale2Entry_Impl(rDoc);
    testColorScale3Entry_Impl(rDoc);

    xDocSh->DoClose();
}

void ScFiltersTest::testNewCondFormatODS()
{
    ScDocShellRef xDocSh = ScBootstrapFixture::loadDoc("new_cond_format_test.", FORMAT_ODS);

    CPPUNIT_ASSERT_MESSAGE("Failed to load new_cond_format_test.ods", xDocSh.Is());

    ScDocument& rDoc = xDocSh->GetDocument();

    OUString aCSVFile("new_cond_format_test.");
    OUString aCSVPath;
    createCSVPath( aCSVFile, aCSVPath );
    testCondFile(aCSVPath, &rDoc, 0);

    xDocSh->DoClose();
}

void ScFiltersTest::testNewCondFormatXLSX()
{
    ScDocShellRef xDocSh = ScBootstrapFixture::loadDoc("new_cond_format_test.", FORMAT_XLSX);

    CPPUNIT_ASSERT_MESSAGE("Failed to load new_cond_format_test.xlsx", xDocSh.Is());

    ScDocument& rDoc = xDocSh->GetDocument();

    OUString aCSVFile("new_cond_format_test.");
    OUString aCSVPath;
    createCSVPath( aCSVFile, aCSVPath );
    testCondFile(aCSVPath, &rDoc, 0);

    xDocSh->DoClose();
}

void ScFiltersTest::testCondFormatThemeColorXLSX()
{
    ScDocShellRef xDocSh = ScBootstrapFixture::loadDoc("condformat_theme_color.", FORMAT_XLSX);

    CPPUNIT_ASSERT_MESSAGE("Failed to load condformat_theme_color.xlsx", xDocSh.Is());

    ScDocument& rDoc = xDocSh->GetDocument();
    ScConditionalFormat* pFormat = rDoc.GetCondFormat(0, 0, 0);
    const ScFormatEntry* pEntry = pFormat->GetEntry(0);
    CPPUNIT_ASSERT(pEntry);
    CPPUNIT_ASSERT_EQUAL(pEntry->GetType(), condformat::DATABAR);
    const ScDataBarFormat* pDataBar = static_cast<const ScDataBarFormat*>(pEntry);
    const ScDataBarFormatData* pDataBarFormatData = pDataBar->GetDataBarData();

    CPPUNIT_ASSERT_EQUAL(Color(157, 195, 230), pDataBarFormatData->maPositiveColor);
    CPPUNIT_ASSERT(pDataBarFormatData->mpNegativeColor.get());
    CPPUNIT_ASSERT_EQUAL(Color(COL_LIGHTRED), *pDataBarFormatData->mpNegativeColor.get());

    CPPUNIT_ASSERT_EQUAL(size_t(1), rDoc.GetCondFormList(1)->size());
    pFormat = rDoc.GetCondFormat(0, 0, 1);
    CPPUNIT_ASSERT(pFormat);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pFormat->size());
    pEntry = pFormat->GetEntry(0);
    CPPUNIT_ASSERT(pEntry);
    CPPUNIT_ASSERT_EQUAL(pEntry->GetType(), condformat::COLORSCALE);
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
    ScDocShellRef xDocSh = ScBootstrapFixture::loadDoc("cond_format_theme_color2.", FORMAT_XLSX);

    CPPUNIT_ASSERT_MESSAGE("Failed to load cond_format_theme_color2.xlsx", xDocSh.Is());

    ScDocument& rDoc = xDocSh->GetDocument();
    ScConditionalFormat* pFormat = rDoc.GetCondFormat(5, 5, 0);
    CPPUNIT_ASSERT(pFormat);
    const ScFormatEntry* pEntry = pFormat->GetEntry(0);
    CPPUNIT_ASSERT(pEntry);
    CPPUNIT_ASSERT_EQUAL(pEntry->GetType(), condformat::DATABAR);
    const ScDataBarFormat* pDataBar = static_cast<const ScDataBarFormat*>(pEntry);
    const ScDataBarFormatData* pDataBarFormatData = pDataBar->GetDataBarData();

    CPPUNIT_ASSERT_EQUAL(Color(99, 142, 198), pDataBarFormatData->maPositiveColor);
    CPPUNIT_ASSERT(pDataBarFormatData->mpNegativeColor.get());
    CPPUNIT_ASSERT_EQUAL(Color(217, 217, 217), *pDataBarFormatData->mpNegativeColor.get());
    CPPUNIT_ASSERT_EQUAL(Color(197, 90, 17), pDataBarFormatData->maAxisColor);
}

namespace {

void testComplexIconSetsXLSX_Impl(ScDocument& rDoc, SCCOL nCol, ScIconSetType eType)
{
    ScConditionalFormat* pFormat = rDoc.GetCondFormat(nCol, 1, 0);
    CPPUNIT_ASSERT(pFormat);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pFormat->size());
    const ScFormatEntry* pEntry = pFormat->GetEntry(0);
    CPPUNIT_ASSERT(pEntry);
    CPPUNIT_ASSERT_EQUAL(condformat::ICONSET, pEntry->GetType());
    const ScIconSetFormat* pIconSet = static_cast<const ScIconSetFormat*>(pEntry);
    CPPUNIT_ASSERT_EQUAL(eType, pIconSet->GetIconSetData()->eIconSetType);
}

void testCustomIconSetsXLSX_Impl(ScDocument& rDoc, SCCOL nCol, SCROW nRow, ScIconSetType eType, sal_Int32 nIndex)
{
    ScConditionalFormat* pFormat = rDoc.GetCondFormat(nCol, 1, 1);
    CPPUNIT_ASSERT(pFormat);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pFormat->size());
    const ScFormatEntry* pEntry = pFormat->GetEntry(0);
    CPPUNIT_ASSERT(pEntry);
    CPPUNIT_ASSERT_EQUAL(condformat::ICONSET, pEntry->GetType());
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
    ScDocShellRef xDocSh = ScBootstrapFixture::loadDoc("complex_icon_set.", FORMAT_XLSX);

    CPPUNIT_ASSERT_MESSAGE("Failed to load complex_icon_set.xlsx", xDocSh.Is());

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
}

void ScFiltersTest::testCondFormatParentXLSX()
{
    ScDocShellRef xDocSh = ScBootstrapFixture::loadDoc("cond_parent.", FORMAT_XLSX);

    CPPUNIT_ASSERT_MESSAGE("Failed to load cond_parent.xlsx", xDocSh.Is());

    ScDocument& rDoc = xDocSh->GetDocument();
    const SfxItemSet* pCondSet = rDoc.GetCondResult(2, 5, 0);
    const ScPatternAttr* pPattern = rDoc.GetPattern(2, 5, 0);
    const SfxPoolItem& rPoolItem = pPattern->GetItem(ATTR_VER_JUSTIFY, pCondSet);
    const SvxVerJustifyItem& rVerJustify = static_cast<const SvxVerJustifyItem&>(rPoolItem);
    CPPUNIT_ASSERT_EQUAL(SVX_VER_JUSTIFY_TOP, static_cast<SvxCellVerJustify>(rVerJustify.GetValue()));
}

void ScFiltersTest::testLiteralInFormulaXLS()
{
    ScDocShellRef xDocSh = loadDoc("shared-string/literal-in-formula.", FORMAT_XLS);
    CPPUNIT_ASSERT(xDocSh.Is());

    ScDocument& rDoc = xDocSh->GetDocument();
    rDoc.CalcAll();

    CPPUNIT_ASSERT_EQUAL(2.0, rDoc.GetValue(ScAddress(0,0,0)));

    xDocSh->DoClose();
}

void ScFiltersTest::testFormulaDependency()
{
    ScDocShellRef xDocSh = ScBootstrapFixture::loadDoc("dependencyTree.", FORMAT_ODS);

    ScDocument& rDoc = xDocSh->GetDocument();

    // check if formula in A1 changes value
    double nVal = rDoc.GetValue(0,0,0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(nVal, 1.0, 1e-10);
    rDoc.SetValue(0,1,0, 0.0);
    nVal = rDoc.GetValue(0,0,0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(nVal, 2.0, 1e-10);

    // check that the number format is implicity inherited
    // CPPUNIT_ASSERT_EQUAL(rDoc.GetString(0,4,0), rDoc.GetString(0,5,0));

    xDocSh->DoClose();
}

void ScFiltersTest::testMiscRowHeights()
{
    TestParam::RowData DfltRowData[] =
    {
        // check rows at the beginning and end of document
        // and make sure they are reported as the default row
        // height ( indicated by -1 )
        { 2, 4, 0, -1, 0, false  },
        { 1048573, 1048575, 0, -1, 0, false  },
    };

    TestParam::RowData MultiLineOptData[] =
    {
        // Row 0 is 12.63 mm and optimal flag is set
        { 0, 0, 0, 1263, CHECK_OPTIMAL, true  },
        // Row 1 is 11.99 mm and optimal flag is NOT set
        { 1, 1, 0, 1199, CHECK_OPTIMAL, false  },
    };

    TestParam aTestValues[] =
    {
        /* Checks that a document saved to ods with default rows does indeed
           have default row heights ( there was a problem where the optimal
           height was being calcuated after import if no hard height )
        */
        { "alldefaultheights.", FORMAT_ODS, -1, SAL_N_ELEMENTS(DfltRowData), DfltRowData },
        /* Checks the imported height of some multiline input, additionally checks
           that the optimal height flag is set ( or not )
        */
        { "multilineoptimal.", FORMAT_ODS, -1, SAL_N_ELEMENTS(MultiLineOptData), MultiLineOptData },
    };
    miscRowHeightsTest( aTestValues, SAL_N_ELEMENTS(aTestValues) );
}

// regression test at least fdo#59193
// what we want to test here is that when cell contents are deleted
// and the optimal flag is set for that row that the row is actually resized

void ScFiltersTest::testOptimalHeightReset()
{
    ScDocShellRef xDocSh = loadDoc("multilineoptimal.", FORMAT_ODS, true);
    SCTAB nTab = 0;
    SCROW nRow = 0;
    ScDocument& rDoc = xDocSh->GetDocument();
    rDoc.EnableAdjustHeight( true );
    // open document in read/write mode ( otherwise optimal height stuff won't
    // be triggered ) *and* you can't delete cell contents.
    int nHeight = sc::TwipsToHMM ( rDoc.GetRowHeight(nRow, nTab, false) );
    CPPUNIT_ASSERT_EQUAL(1263, nHeight);

    ScDocFunc &rFunc = xDocSh->GetDocFunc();

    // delete content of A1
    ScRange aDelRange(0,0,0,0,0,0);
    ScMarkData aMark;
    aMark.SetMarkArea(aDelRange);
    bool bRet = rFunc.DeleteContents( aMark, InsertDeleteFlags::ALL, false, true );
    CPPUNIT_ASSERT_MESSAGE("DeleteContents failed", bRet);

    // get the new height of A1
    nHeight =  sc::TwipsToHMM( rDoc.GetRowHeight(nRow, nTab, false) );

    // set optimal height for empty row 2
    std::vector<sc::ColRowSpan> aRowArr(1, sc::ColRowSpan(2,2));
    rFunc.SetWidthOrHeight(false, aRowArr, nTab, SC_SIZE_OPTIMAL, 0, true, true);

    // retrieve optimal height
    int nOptimalHeight = sc::TwipsToHMM( rDoc.GetRowHeight(aRowArr[0].mnStart, nTab, false) );

    // check if the new height of A1 ( after delete ) is now the optimal height of an empty cell
    CPPUNIT_ASSERT_EQUAL(nOptimalHeight, nHeight );
    xDocSh->DoClose();
}

void ScFiltersTest::testPrintRangeODS()
{
    ScDocShellRef xDocSh = loadDoc("print-range.", FORMAT_ODS);
    ScDocument& rDoc = xDocSh->GetDocument();
    const ScRange* pRange = rDoc.GetRepeatRowRange(0);
    CPPUNIT_ASSERT(pRange);
    CPPUNIT_ASSERT_EQUAL(ScRange(0,0,0,0,1,0), *pRange);

    pRange = rDoc.GetRepeatRowRange(1);
    CPPUNIT_ASSERT(pRange);
    CPPUNIT_ASSERT_EQUAL(ScRange(0,2,0,0,4,0), *pRange);

    xDocSh->DoClose();
}

void ScFiltersTest::testOutlineODS()
{
    ScDocShellRef xDocSh = loadDoc("outline.", FORMAT_ODS);
    ScDocument& rDoc = xDocSh->GetDocument();

    const ScOutlineTable* pTable = rDoc.GetOutlineTable(0);
    CPPUNIT_ASSERT(pTable);

    const ScOutlineArray& rArr = pTable->GetRowArray();
    size_t nDepth = rArr.GetDepth();
    CPPUNIT_ASSERT_EQUAL(size_t(4), nDepth);

    for(size_t i = 0; i < nDepth; ++i)
    {
        CPPUNIT_ASSERT_EQUAL(size_t(1), rArr.GetCount(i));
    }

    struct OutlineData {
        SCCOLROW nStart;
        SCCOLROW nEnd;
        bool bHidden;
        bool bVisible;

        size_t nDepth;
        size_t nIndex;
    };

    OutlineData aRow[] =
    {
        { 1, 29, false, true, 0, 0 },
        { 2, 26, false, true, 1, 0 },
        { 4, 23, false, true, 2, 0 },
        { 6, 20, true, true, 3, 0 }
    };

    for(size_t i = 0; i < SAL_N_ELEMENTS(aRow); ++i)
    {

        const ScOutlineEntry* pEntry = rArr.GetEntry(aRow[i].nDepth, aRow[i].nIndex);
        SCCOLROW nStart = pEntry->GetStart();
        CPPUNIT_ASSERT_EQUAL(aRow[i].nStart, nStart);

        SCCOLROW nEnd = pEntry->GetEnd();
        CPPUNIT_ASSERT_EQUAL(aRow[i].nEnd, nEnd);

        bool bHidden = pEntry->IsHidden();
        CPPUNIT_ASSERT_EQUAL(aRow[i].bHidden, bHidden);

        bool bVisible = pEntry->IsVisible();
        CPPUNIT_ASSERT_EQUAL(aRow[i].bVisible, bVisible);
    }

    xDocSh->DoClose();
}

void ScFiltersTest::testColumnStyleXLSX()
{
    ScDocShellRef xDocSh = loadDoc("column-style.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.Is());
    ScDocument& rDoc = xDocSh->GetDocument();

    const ScPatternAttr* pPattern = rDoc.GetPattern(0,0,0);
    CPPUNIT_ASSERT(pPattern);

    const ScProtectionAttr& rAttr = static_cast<const ScProtectionAttr&>(pPattern->GetItem(ATTR_PROTECTION));
    CPPUNIT_ASSERT(rAttr.GetProtection());

    pPattern = rDoc.GetPattern(0,1,0);
    CPPUNIT_ASSERT(pPattern);

    const ScProtectionAttr& rAttrNew = static_cast<const ScProtectionAttr&>(pPattern->GetItem(ATTR_PROTECTION));
    CPPUNIT_ASSERT(!rAttrNew.GetProtection());

    xDocSh->DoClose();
}

void ScFiltersTest::testSharedFormulaHorizontalXLS()
{
    ScDocShellRef xDocSh = loadDoc("shared-formula/horizontal.", FORMAT_XLS);
    CPPUNIT_ASSERT(xDocSh.Is());
    ScDocument& rDoc = xDocSh->GetDocument();

    // Make sure K2:S2 on the 2nd sheet are all formula cells.
    ScAddress aPos(0, 1, 1);
    for (SCCOL nCol = 10; nCol <= 18; ++nCol)
    {
        aPos.SetCol(nCol);
        CPPUNIT_ASSERT_MESSAGE("Formula cell is expected here.", rDoc.GetCellType(aPos) == CELLTYPE_FORMULA);
    }

    // Likewise, B3:J9 all should be formula cells.
    for (SCCOL nCol = 1; nCol <= 9; ++nCol)
    {
        aPos.SetCol(nCol);
        for (SCROW nRow = 2; nRow <= 8; ++nRow)
        {
            aPos.SetRow(nRow);
            CPPUNIT_ASSERT_MESSAGE("Formula cell is expected here.", rDoc.GetCellType(aPos) == CELLTYPE_FORMULA);
        }
    }

    // B2:I2 too.
    aPos.SetRow(1);
    for (SCCOL nCol = 1; nCol <= 8; ++nCol)
    {
        aPos.SetCol(nCol);
        CPPUNIT_ASSERT_MESSAGE("Formula cell is expected here.", rDoc.GetCellType(aPos) == CELLTYPE_FORMULA);
    }

    // J2 has a string of "MW".
    aPos.SetCol(9);
    CPPUNIT_ASSERT_EQUAL(OUString("MW"), rDoc.GetString(aPos));

    xDocSh->DoClose();
}

void ScFiltersTest::testSharedFormulaWrappedRefsXLS()
{
    ScDocShellRef xDocSh = loadDoc("shared-formula/wrapped-refs.", FORMAT_XLS);
    CPPUNIT_ASSERT(xDocSh.Is());
    ScDocument& rDoc = xDocSh->GetDocument();
    rDoc.CalcAll();

    // Check the values of H7:H10.
    CPPUNIT_ASSERT_EQUAL(7.0, rDoc.GetValue(ScAddress(7,6,0)));
    CPPUNIT_ASSERT_EQUAL(8.0, rDoc.GetValue(ScAddress(7,7,0)));
    CPPUNIT_ASSERT_EQUAL(9.0, rDoc.GetValue(ScAddress(7,8,0)));
    CPPUNIT_ASSERT_EQUAL(10.0, rDoc.GetValue(ScAddress(7,9,0)));

    // EM7:EM10 should reference H7:H10.
    CPPUNIT_ASSERT_EQUAL(7.0, rDoc.GetValue(ScAddress(142,6,0)));
    CPPUNIT_ASSERT_EQUAL(8.0, rDoc.GetValue(ScAddress(142,7,0)));
    CPPUNIT_ASSERT_EQUAL(9.0, rDoc.GetValue(ScAddress(142,8,0)));
    CPPUNIT_ASSERT_EQUAL(10.0, rDoc.GetValue(ScAddress(142,9,0)));

    // Make sure EM7:EM10 are grouped.
    const ScFormulaCell *pFC = rDoc.GetFormulaCell(ScAddress(142,6,0));
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(6), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(4), pFC->GetSharedLength());

    xDocSh->DoClose();
}

void ScFiltersTest::testSharedFormulaBIFF5()
{
    ScDocShellRef xDocSh = loadDoc("shared-formula/biff5.", FORMAT_XLS);
    CPPUNIT_ASSERT(xDocSh.Is());
    ScDocument& rDoc = xDocSh->GetDocument();
    rDoc.CalcAll();

    // E6:E376 should be all formulas, and they should belong to the same group.
    const ScFormulaCell* pFC = rDoc.GetFormulaCell(ScAddress(4,5,0));
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(5), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(371), pFC->GetSharedLength());

    xDocSh->DoClose();
}

void ScFiltersTest::testSharedFormulaXLSB()
{
    ScDocShellRef xDocSh = loadDoc("shared_formula.", FORMAT_XLSB);
    CPPUNIT_ASSERT(xDocSh.Is());
    ScDocument& rDoc = xDocSh->GetDocument();
    rDoc.CalcAll();

    // A1:A30 should be all formulas, and they should belong to the same group.
    const ScFormulaCell* pFC = rDoc.GetFormulaCell(ScAddress(0,0,0));
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(0), pFC->GetSharedTopRow());
    CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(30), pFC->GetSharedLength());

    for(SCROW nRow = 0; nRow < 30; ++nRow)
    {
        ASSERT_DOUBLES_EQUAL(3.0, rDoc.GetValue(0, nRow, 0));
    }

    xDocSh->DoClose();
}

void ScFiltersTest::testSharedFormulaXLS()
{
    {
        // fdo#80091
        ScDocShellRef xDocSh = loadDoc("shared-formula/relative-refs1.", FORMAT_XLS);
        CPPUNIT_ASSERT(xDocSh.Is());
        ScDocument& rDoc = xDocSh->GetDocument();
        rDoc.CalcAll();

        // A1:A30 should be all formulas, and they should belong to the same group.
        const ScFormulaCell* pFC = rDoc.GetFormulaCell(ScAddress(0,1,0));
        CPPUNIT_ASSERT(pFC);
        CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(1), pFC->GetSharedTopRow());
        CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(29), pFC->GetSharedLength());

        for(SCROW nRow = 0; nRow < 30; ++nRow)
        {
            ASSERT_DOUBLES_EQUAL(double(nRow+1), rDoc.GetValue(0, nRow, 0));
        }

        xDocSh->DoClose();
    }

    {
        // fdo#84556 and some related tests
        ScDocShellRef xDocSh = loadDoc("shared-formula/relative-refs2.", FORMAT_XLS);
        CPPUNIT_ASSERT(xDocSh.Is());
        ScDocument& rDoc = xDocSh->GetDocument();
        rDoc.CalcAll();

        {
            const ScFormulaCell* pFC = rDoc.GetFormulaCell(ScAddress(2,1,0));
            CPPUNIT_ASSERT(pFC);
            CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(1), pFC->GetSharedTopRow());
            CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(10), pFC->GetSharedLength());

            pFC = rDoc.GetFormulaCell(ScAddress(2,10,0));
            CPPUNIT_ASSERT(pFC);
            CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(1), pFC->GetSharedTopRow());
            CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(10), pFC->GetSharedLength());

            OUString aFormula;
            rDoc.GetFormula(2, 1, 0, aFormula);
            CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B9:D9)"), aFormula);

            rDoc.GetFormula(2, 10, 0, aFormula);
            CPPUNIT_ASSERT_EQUAL(OUString("=SUM(B18:D18)"), aFormula);
        }

        {
            const ScFormulaCell* pFC = rDoc.GetFormulaCell(ScAddress(4,8,0));
            CPPUNIT_ASSERT(pFC);
            CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(8), pFC->GetSharedTopRow());
            CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(10), pFC->GetSharedLength());

            pFC = rDoc.GetFormulaCell(ScAddress(4,17,0));
            CPPUNIT_ASSERT(pFC);
            CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(8), pFC->GetSharedTopRow());
            CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(10), pFC->GetSharedLength());

            OUString aFormula;
            rDoc.GetFormula(4, 8, 0, aFormula);
            CPPUNIT_ASSERT_EQUAL(OUString("=SUM(G9:EY9)"), aFormula);

            rDoc.GetFormula(4, 17, 0, aFormula);
            CPPUNIT_ASSERT_EQUAL(OUString("=SUM(G18:EY18)"), aFormula);
        }

        {
            const ScFormulaCell* pFC = rDoc.GetFormulaCell(ScAddress(6,15,0));
            CPPUNIT_ASSERT(pFC);
            CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(15), pFC->GetSharedTopRow());
            CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(10), pFC->GetSharedLength());

            pFC = rDoc.GetFormulaCell(ScAddress(6,24,0));
            CPPUNIT_ASSERT(pFC);
            CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(15), pFC->GetSharedTopRow());
            CPPUNIT_ASSERT_EQUAL(static_cast<SCROW>(10), pFC->GetSharedLength());

            OUString aFormula;
            rDoc.GetFormula(6, 15, 0, aFormula);
            CPPUNIT_ASSERT_EQUAL(OUString("=SUM(A16:A40000)"), aFormula);

            rDoc.GetFormula(6, 24, 0, aFormula);
            CPPUNIT_ASSERT_EQUAL(OUString("=SUM(A25:A40009)"), aFormula);
        }

        xDocSh->DoClose();
    }
}

void ScFiltersTest::testExternalRefCacheXLSX()
{
    ScDocShellRef xDocSh = loadDoc("external-refs.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.Is());
    ScDocument& rDoc = xDocSh->GetDocument();

    // These string values are cached external cell values.
    CPPUNIT_ASSERT_EQUAL(OUString("Name"), rDoc.GetString(ScAddress(0,0,0)));
    CPPUNIT_ASSERT_EQUAL(OUString("Andy"), rDoc.GetString(ScAddress(0,1,0)));
    CPPUNIT_ASSERT_EQUAL(OUString("Bruce"), rDoc.GetString(ScAddress(0,2,0)));
    CPPUNIT_ASSERT_EQUAL(OUString("Charlie"), rDoc.GetString(ScAddress(0,3,0)));

    xDocSh->DoClose();
}

void ScFiltersTest::testExternalRefCacheODS()
{
    ScDocShellRef xDocSh = loadDoc("external-ref-cache.", FORMAT_ODS);

    CPPUNIT_ASSERT(xDocSh.Is());
    ScDocument& rDoc = xDocSh->GetDocument();

    // Cells B2:B4 have VLOOKUP with external references which should all show "text".
    CPPUNIT_ASSERT_EQUAL(OUString("text"), rDoc.GetString(ScAddress(1,1,0)));
    CPPUNIT_ASSERT_EQUAL(OUString("text"), rDoc.GetString(ScAddress(1,2,0)));
    CPPUNIT_ASSERT_EQUAL(OUString("text"), rDoc.GetString(ScAddress(1,3,0)));

    // Both cells A6 and A7 should be registered with scExternalRefManager properly
    CPPUNIT_ASSERT_EQUAL(true, rDoc.GetExternalRefManager()->hasCellExternalReference(ScAddress(0, 5, 0)));
    CPPUNIT_ASSERT_EQUAL(true, rDoc.GetExternalRefManager()->hasCellExternalReference(ScAddress(0, 6, 0)));

    xDocSh->DoClose();
}

void ScFiltersTest::testHybridSharedStringODS()
{
    ScDocShellRef xDocSh = loadDoc("hybrid-shared-string.", FORMAT_ODS);

    CPPUNIT_ASSERT(xDocSh.Is());
    ScDocument& rDoc = xDocSh->GetDocument();

    // A2 contains formula with MATCH function.  The result must be 2, not #N/A!
    CPPUNIT_ASSERT_EQUAL(2.0, rDoc.GetValue(ScAddress(0,1,0)));

    xDocSh->DoClose();
}

void ScFiltersTest::testCopyMergedNumberFormats()
{
    ScDocShellRef xDocSh = loadDoc("copy-merged-number-formats.", FORMAT_ODS);
    CPPUNIT_ASSERT(xDocSh.Is());
    ScDocument& rDoc = xDocSh->GetDocument();

    // Cells B1, C1 and D1 are formatted as dates.
    OUString aStrB1 = rDoc.GetString(ScAddress(1,0,0));
    OUString aStrC1 = rDoc.GetString(ScAddress(2,0,0));
    OUString aStrD1 = rDoc.GetString(ScAddress(3,0,0));

    ScDocument aCopyDoc;
    aCopyDoc.InsertTab(0, "CopyHere");
    rDoc.CopyStaticToDocument(ScRange(1,0,0,3,0,0), 0, &aCopyDoc);

    // Make sure the date formats are copied to the new document.
    CPPUNIT_ASSERT_EQUAL(aStrB1, aCopyDoc.GetString(ScAddress(1,0,0)));
    CPPUNIT_ASSERT_EQUAL(aStrC1, aCopyDoc.GetString(ScAddress(2,0,0)));
    CPPUNIT_ASSERT_EQUAL(aStrD1, aCopyDoc.GetString(ScAddress(3,0,0)));

    xDocSh->DoClose();
}

void ScFiltersTest::testVBAUserFunctionXLSM()
{
    ScDocShellRef xDocSh = loadDoc("vba-user-function.", FORMAT_XLSM);
    CPPUNIT_ASSERT(xDocSh.Is());
    ScDocument& rDoc = xDocSh->GetDocument();

    // A1 contains formula with user-defined function, and the function is defined in VBA.
    ScFormulaCell* pFC = rDoc.GetFormulaCell(ScAddress(0,0,0));
    CPPUNIT_ASSERT(pFC);

    sc::CompileFormulaContext aCxt(&rDoc);
    OUString aFormula = pFC->GetFormula(aCxt);

    CPPUNIT_ASSERT_EQUAL(OUString("=MYFUNC()"), aFormula);

    // Check the formula state after the load.
    sal_uInt16 nErrCode = pFC->GetErrCode();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(0), nErrCode);

    // Check the result.
    CPPUNIT_ASSERT_EQUAL(42.0, rDoc.GetValue(ScAddress(0,0,0)));

    xDocSh->DoClose();
}

void ScFiltersTest::testEmbeddedImageXLS()
{
    // The document has one embedded image on the first sheet.  Make sure it's
    // imported properly.

    ScDocShellRef xDocSh = loadDoc("file-with-png-image.", FORMAT_XLS);
    CPPUNIT_ASSERT(xDocSh.Is());
    ScDocument& rDoc = xDocSh->GetDocument();

    ScDrawLayer* pDL = rDoc.GetDrawLayer();
    CPPUNIT_ASSERT(pDL);
    const SdrPage* pPage = pDL->GetPage(0);
    CPPUNIT_ASSERT(pPage);
    const SdrObject* pObj = pPage->GetObj(0);
    CPPUNIT_ASSERT(pObj);
    const SdrGrafObj* pImageObj = dynamic_cast<const SdrGrafObj*>(pObj);
    CPPUNIT_ASSERT(pImageObj);
    const Graphic& rGrf = pImageObj->GetGraphic();
    BitmapEx aBMP = rGrf.GetBitmapEx();
    CPPUNIT_ASSERT_MESSAGE("Bitmap content should not be empty if the image has been properly imported.", !aBMP.IsEmpty());

    xDocSh->DoClose();
}

void ScFiltersTest::testErrorOnExternalReferences()
{
    ScDocShellRef xDocSh = loadDoc("blank.", FORMAT_ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to open empty doc", xDocSh.Is());

    ScDocument& rDoc = xDocSh->GetDocument();

    // Test tdf#89330
    rDoc.SetString(ScAddress(0,0,0), "='file:///Path/To/FileA.ods'#$Sheet1.A1A");

    ScFormulaCell* pFC = rDoc.GetFormulaCell(ScAddress(0,0,0));
    CPPUNIT_ASSERT(pFC);
    CPPUNIT_ASSERT_EQUAL(ScErrorCodes::errNoName, pFC->GetErrCode());

    if (!checkFormula(rDoc, ScAddress(0,0,0), "'file:///Path/To/FileA.ods'#$Sheet1.A1A"))
        CPPUNIT_FAIL("Formula changed");

    xDocSh->DoClose();
}

void ScFiltersTest::testEditEngStrikeThroughXLSX()
{
    ScDocShellRef xDocSh = loadDoc("strike-through.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to open doc", xDocSh.Is());

    ScDocument& rDoc = xDocSh->GetDocument();

    const EditTextObject* pObj = rDoc.GetEditText(ScAddress(0, 0, 0));
    CPPUNIT_ASSERT(pObj);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pObj->GetParagraphCount());
    CPPUNIT_ASSERT_EQUAL(OUString("this is strike through  this not"), pObj->GetText(0));

    std::vector<EECharAttrib> aAttribs;
    pObj->GetCharAttribs(0, aAttribs);
    for (std::vector<EECharAttrib>::const_iterator itr = aAttribs.begin(); itr != aAttribs.end(); ++itr)
    {
        if (itr->pAttr->Which() == EE_CHAR_STRIKEOUT)
        {
            const SvxCrossedOutItem& rItem = static_cast<const SvxCrossedOutItem&>(*itr->pAttr);
            if (itr->nStart == 0)
            {
                CPPUNIT_ASSERT(rItem.GetStrikeout() != STRIKEOUT_NONE);
            }
            else
            {
                CPPUNIT_ASSERT_EQUAL(STRIKEOUT_NONE, rItem.GetStrikeout());
            }
        }
    }

    xDocSh->DoClose();
}

void ScFiltersTest::testRefStringXLSX()
{
    ScDocShellRef xDocSh = loadDoc("ref_string.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to open doc", xDocSh.Is());

    ScDocument& rDoc = xDocSh->GetDocument();

    double nVal = rDoc.GetValue(2, 2, 0);
    ASSERT_DOUBLES_EQUAL(3.0, nVal);

    const ScCalcConfig& rCalcConfig = rDoc.GetCalcConfig();
    CPPUNIT_ASSERT_EQUAL(formula::FormulaGrammar::CONV_XL_A1, rCalcConfig.meStringRefAddressSyntax);

}

void ScFiltersTest::testBnc762542()
{
    ScDocShellRef xDocSh = loadDoc("bnc762542.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to open doc", xDocSh.Is());

    ScDocument& rDoc = xDocSh->GetDocument();
    ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
    SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("draw page for sheet 1 should exist.", pPage);

    const size_t nCount = pPage->GetObjCount();
    CPPUNIT_ASSERT_MESSAGE("There should be 10 shapes.", nCount == 10);

    // previously, some of the shapes were (incorrectly) rotated by 90 degrees
    for (size_t i : { 1, 2, 4, 5, 7, 9 })
    {
        SdrObject* pObj = pPage->GetObj(i);
        CPPUNIT_ASSERT_MESSAGE("Failed to get drawing object.", pObj);

        Rectangle aRect(pObj->GetCurrentBoundRect());
        CPPUNIT_ASSERT_MESSAGE("Drawing object shouldn't be rotated.", aRect.GetWidth() > aRect.GetHeight());
    }

    xDocSh->DoClose();
}

void ScFiltersTest::testHiddenSheetsXLSX()
{
    ScDocShellRef xDocSh = loadDoc("hidden_sheets.", FORMAT_XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to open doc", xDocSh.Is());

    ScDocument& rDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("1st sheet should be hidden", false, rDoc.IsVisible(0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("2nd sheet should be visible", true, rDoc.IsVisible(1));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("3rd sheet should be hidden", false, rDoc.IsVisible(2));
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

