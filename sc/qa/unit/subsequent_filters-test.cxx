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
#include "svx/svdpage.hxx"
#include "svx/svdoole2.hxx"

#include "editeng/wghtitem.hxx"
#include "editeng/postitem.hxx"
#include "editeng/udlnitem.hxx"
#include "editeng/editobj.hxx"
#include "editeng/brushitem.hxx"
#include <editeng/brushitem.hxx>
#include <editeng/justifyitem.hxx>
#include <editeng/borderline.hxx>
#include "editeng/flditem.hxx"
#include <dbdata.hxx>
#include "validat.hxx"
#include "formulacell.hxx"
#include "drwlayer.hxx"
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

#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include <com/sun/star/sheet/GeneralFunction.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/text/textfield/Type.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/data/XDataReceiver.hpp>

#define TEST_BUG_FILES 0

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
        const OUString &rUserData, unsigned int nFilterFlags,
        unsigned int nClipboardID, unsigned int nFilterVersion);

    virtual void setUp();
    virtual void tearDown();

    //ods, xls, xlsx filter tests
    void testBasicCellContentODS();
    void testRangeNameXLS();
    void testRangeNameXLSX();
    void testHardRecalcODS();
    void testFunctionsODS();
    void testCachedFormulaResultsODS();
    void testCachedMatrixFormulaResultsODS();
    void testDatabaseRangesODS();
    void testDatabaseRangesXLS();
    void testDatabaseRangesXLSX();
    void testFormatsODS();
    void testFormatsXLS();
    void testFormatsXLSX();
    void testMatrixODS();
    void testMatrixXLS();
    void testBorderODS();
    void testBorderXLS();
    void testBorderXLSX();
    void testBordersOoo33();
    void testBugFixesODS();
    void testBugFixesXLS();
    void testBugFixesXLSX();
    void testBrokenQuotesCSV();
    void testMergedCellsODS();
    void testRepeatedColumnsODS();
    void testDataValidityODS();

    void testDataBarODS();
    void testDataBarXLSX();
    void testColorScaleODS();
    void testColorScaleXLSX();
    void testNewCondFormatODS();
    void testNewCondFormatXLSX();

    //change this test file only in excel and not in calc
    void testCellValueXLSX();

    //misc tests unrelated to the import filters
    void testPasswordNew();
    void testPasswordOld();

    //test shape import
    void testControlImport();
    void testChartImportODS();

    void testNumberFormatHTML();
    void testNumberFormatCSV();

    void testCellAnchoredShapesODS();

    void testPivotTableBasicODS();
    void testFormulaDependency();

    void testRowHeightODS();
    void testRichTextContentODS();
    void testMiscRowHeights();
    void testOptimalHeightReset();

    void testPrintRangeODS();
    void testOutlineODS();

    CPPUNIT_TEST_SUITE(ScFiltersTest);
    CPPUNIT_TEST(testBasicCellContentODS);
    CPPUNIT_TEST(testRangeNameXLS);
    CPPUNIT_TEST(testRangeNameXLSX);
    CPPUNIT_TEST(testHardRecalcODS);
    CPPUNIT_TEST(testFunctionsODS);
    CPPUNIT_TEST(testCachedFormulaResultsODS);
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
    CPPUNIT_TEST(testBorderXLS);
    CPPUNIT_TEST(testBorderXLSX);
    CPPUNIT_TEST(testBordersOoo33);
    CPPUNIT_TEST(testBugFixesODS);
    CPPUNIT_TEST(testBugFixesXLS);
    CPPUNIT_TEST(testBugFixesXLSX);
    CPPUNIT_TEST(testMergedCellsODS);
    CPPUNIT_TEST(testRepeatedColumnsODS);
    CPPUNIT_TEST(testDataValidityODS);
    CPPUNIT_TEST(testBrokenQuotesCSV);
    CPPUNIT_TEST(testCellValueXLSX);
    CPPUNIT_TEST(testControlImport);
    CPPUNIT_TEST(testChartImportODS);

    CPPUNIT_TEST(testDataBarODS);
    CPPUNIT_TEST(testDataBarXLSX);
    CPPUNIT_TEST(testColorScaleODS);
    CPPUNIT_TEST(testColorScaleXLSX);
    CPPUNIT_TEST(testNewCondFormatODS);
    CPPUNIT_TEST(testNewCondFormatXLSX);

    CPPUNIT_TEST(testNumberFormatHTML);
    CPPUNIT_TEST(testNumberFormatCSV);

    CPPUNIT_TEST(testCellAnchoredShapesODS);

    CPPUNIT_TEST(testPivotTableBasicODS);
    CPPUNIT_TEST(testRowHeightODS);
    CPPUNIT_TEST(testFormulaDependency);
    CPPUNIT_TEST(testRichTextContentODS);

    //disable testPassword on MacOSX due to problems with libsqlite3
    //also crashes on DragonFly due to problems with nss/nspr headers
#if !defined(MACOSX) && !defined(DRAGONFLY) && !defined(WNT)
    CPPUNIT_TEST(testPasswordOld);
    CPPUNIT_TEST(testPasswordNew);
#endif

#if TEST_BUG_FILES
    CPPUNIT_TEST(testBugFiles);
    CPPUNIT_TEST(testBugFilesXLS);
    CPPUNIT_TEST(testBugFilesXLSX);
#endif
    CPPUNIT_TEST(testMiscRowHeights);
    CPPUNIT_TEST(testOptimalHeightReset);
    CPPUNIT_TEST(testPrintRangeODS);
    CPPUNIT_TEST(testOutlineODS);
    CPPUNIT_TEST_SUITE_END();

private:
    void testPassword_Impl(const OUString& rFileNameBase);
    void testBorderImpl( sal_uLong nFormatType );
    uno::Reference<uno::XInterface> m_xCalcComponent;
};

bool ScFiltersTest::load(const OUString &rFilter, const OUString &rURL,
    const OUString &rUserData, unsigned int nFilterFlags,
        unsigned int nClipboardID, unsigned int nFilterVersion)
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

void testRangeNameImpl(ScDocument* pDoc)
{
    //check one range data per sheet and one global more detailed
    //add some more checks here
    ScRangeData* pRangeData = pDoc->GetRangeName()->findByUpperName(OUString("GLOBAL1"));
    CPPUNIT_ASSERT_MESSAGE("range name Global1 not found", pRangeData);
    double aValue;
    pDoc->GetValue(1,0,0,aValue);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("range name Global1 should reference Sheet1.A1", 1.0, aValue);
    pRangeData = pDoc->GetRangeName(0)->findByUpperName(OUString("LOCAL1"));
    CPPUNIT_ASSERT_MESSAGE("range name Sheet1.Local1 not found", pRangeData);
    pDoc->GetValue(1,2,0,aValue);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("range name Sheet1.Local1 should reference Sheet1.A3", 3.0, aValue);
    pRangeData = pDoc->GetRangeName(1)->findByUpperName(OUString("LOCAL2"));
    CPPUNIT_ASSERT_MESSAGE("range name Sheet2.Local2 not found", pRangeData);
    pDoc->GetValue(1,1,1,aValue);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("range name Sheet2.Local2 should reference Sheet2.A2", 7.0, aValue);
    //check for correct results for the remaining formulas
    pDoc->GetValue(1,1,0, aValue);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("=global2 should be 2", 2.0, aValue);
    pDoc->GetValue(1,3,0, aValue);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("=local2 should be 4", 4.0, aValue);
    pDoc->GetValue(2,0,0, aValue);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("=SUM(global3) should be 10", 10.0, aValue);
    pDoc->GetValue(1,0,1,aValue);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("range name Sheet2.local1 should reference Sheet1.A5", 5.0, aValue);
    // Test if Global5 ( which depends on Global6 ) is evaluated
    pDoc->GetValue(0,5,1, aValue);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("formula Global5 should reference Global6 ( which is evaluated as local1 )", 5.0, aValue);
}

}

void ScFiltersTest::testBasicCellContentODS()
{
    ScDocShellRef xDocSh = loadDoc("basic-cell-content.", ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load basic-cell-content.ods", xDocSh.Is());

    ScDocument* pDoc = xDocSh->GetDocument();
    OUString aStr = pDoc->GetString(1, 1, 0); // B2
    CPPUNIT_ASSERT_EQUAL(OUString("LibreOffice Calc"), aStr);
    double fVal = pDoc->GetValue(1, 2, 0); // B3
    CPPUNIT_ASSERT_EQUAL(12345.0, fVal);
    aStr = pDoc->GetString(1, 3, 0); // B4
    CPPUNIT_ASSERT_EQUAL(OUString("A < B"), aStr);

    xDocSh->DoClose();
}

void ScFiltersTest::testRangeNameXLS()
{
    ScDocShellRef xDocSh = loadDoc("named-ranges-global.", XLS);
    xDocSh->DoHardRecalc(true);

    ScDocument* pDoc = xDocSh->GetDocument();
    testRangeNameImpl(pDoc);

    OUString aSheet2CSV("rangeExp_Sheet2.");
    OUString aCSVPath;
    createCSVPath( aSheet2CSV, aCSVPath );
    // fdo#44587
    testFile( aCSVPath, pDoc, 1);

    xDocSh->DoClose();
}

void ScFiltersTest::testRangeNameXLSX()
{
    ScDocShellRef xDocSh = loadDoc("named-ranges-global.", XLSX);
    xDocSh->DoHardRecalc(true);

    ScDocument* pDoc = xDocSh->GetDocument();
    testRangeNameImpl(pDoc);

    xDocSh->DoClose();
}

void ScFiltersTest::testHardRecalcODS()
{
    ScDocShellRef xDocSh = loadDoc("hard-recalc.", ODS);
    xDocSh->DoHardRecalc(true);

    CPPUNIT_ASSERT_MESSAGE("Failed to load hard-recalc.*", xDocSh.Is());
    ScDocument* pDoc = xDocSh->GetDocument();
    OUString aCSVFileName;

    //test hard recalc: document has an incorrect cached formula result
    //hard recalc should have updated to the correct result
    createCSVPath(OUString("hard-recalc."), aCSVFileName);
    testFile(aCSVFileName, pDoc, 0);

    xDocSh->DoClose();
}

void ScFiltersTest::testFunctionsODS()
{
    ScDocShellRef xDocSh = loadDoc("functions.", ODS);
    xDocSh->DoHardRecalc(true);

    CPPUNIT_ASSERT_MESSAGE("Failed to load functions.*", xDocSh.Is());
    ScDocument* pDoc = xDocSh->GetDocument();
    OUString aCSVFileName;

    //test logical functions
    createCSVPath(OUString("logical-functions."), aCSVFileName);
    testFile(aCSVFileName, pDoc, 0);
    //test spreadsheet functions
    createCSVPath(OUString("spreadsheet-functions."), aCSVFileName);
    testFile(aCSVFileName, pDoc, 1);
    //test mathematical functions
    createCSVPath(OUString("mathematical-functions."), aCSVFileName);
    testFile(aCSVFileName, pDoc, 2, PureString);
    //test information functions
    createCSVPath(OUString("information-functions."), aCSVFileName);
    testFile(aCSVFileName, pDoc, 3);

    xDocSh->DoClose();
}

void ScFiltersTest::testCachedFormulaResultsODS()
{
    {
        ScDocShellRef xDocSh = loadDoc("functions.", ODS);
        CPPUNIT_ASSERT_MESSAGE("Failed to load functions.*", xDocSh.Is());

        ScDocument* pDoc = xDocSh->GetDocument();
        OUString aCSVFileName;

        //test cached formula results of logical functions
        createCSVPath(OUString("logical-functions."), aCSVFileName);
        testFile(aCSVFileName, pDoc, 0);
        //test cached formula results of spreadsheet functions
        createCSVPath(OUString("spreadsheet-functions."), aCSVFileName);
        testFile(aCSVFileName, pDoc, 1);
        //test cached formula results of mathematical functions
        createCSVPath(OUString("mathematical-functions."), aCSVFileName);
        testFile(aCSVFileName, pDoc, 2, PureString);
        //test cached formula results of information functions
        createCSVPath(OUString("information-functions."), aCSVFileName);
        testFile(aCSVFileName, pDoc, 3);

        xDocSh->DoClose();
    }

    {
        ScDocShellRef xDocSh = loadDoc("cachedValue.", ODS);
        CPPUNIT_ASSERT_MESSAGE("Failed to load cachedValue.*", xDocSh.Is());

        ScDocument* pDoc = xDocSh->GetDocument();
        OUString aCSVFileName;
        createCSVPath("cachedValue.", aCSVFileName);
        testFile(aCSVFileName, pDoc, 0);

        //we want to me sure that volatile functions are always recalculated
        //regardless of cached results.  if you update the ods file, you must
        //update the values here.
        //if NOW() is recacluated, then it should never equal sTodayCache
        OUString sTodayCache("01/25/13 01:06 PM");
        OUString sTodayRecalc(pDoc->GetString(0,0,1));

        CPPUNIT_ASSERT(sTodayCache != sTodayRecalc);

        OUString sTodayRecalcRef(pDoc->GetString(1,0,1));
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
                pDoc->SetString(nCol, nRow + 2, 2, aFormula);
                CPPUNIT_ASSERT_EQUAL_MESSAGE(OUStringToOString(aFormula, RTL_TEXTENCODING_UTF8).getStr(), pDoc->GetString(nCol, nRow +2, 2), OUString("TRUE"));

                OUStringBuffer aIsTextFormula("=ISTEXT(");
                aIsTextFormula.append((char)('A'+nCol)).append(OUString::number(nRow));
                aIsTextFormula.append(")");
                pDoc->SetString(nCol, nRow + 4, 2, aIsTextFormula.makeStringAndClear());
                CPPUNIT_ASSERT_EQUAL_MESSAGE("", pDoc->GetString(nCol, nRow +4, 2), OUString("FALSE"));
            }
        }

        xDocSh->DoClose();
    }
}

void ScFiltersTest::testCachedMatrixFormulaResultsODS()
{
    ScDocShellRef xDocSh = loadDoc("matrix.", ODS);

    CPPUNIT_ASSERT_MESSAGE("Failed to load matrix.*", xDocSh.Is());
    ScDocument* pDoc = xDocSh->GetDocument();

    //test matrix
    OUString aCSVFileName;
    createCSVPath("matrix.", aCSVFileName);
    testFile(aCSVFileName, pDoc, 0);
    //test matrices with special cases
    createCSVPath("matrix2.", aCSVFileName);
    testFile(aCSVFileName, pDoc, 1);
    createCSVPath("matrix3.", aCSVFileName);
    testFile(aCSVFileName, pDoc, 2);
    //The above testFile() does not catch the below case.
    //If a matrix formula has a matrix reference cell that is intended to have
    //a blank text result, the matrix reference cell is actually saved(export)
    //as a float cell with 0 as the value and an empty <text:p/>.
    //Import works around this by setting these cells as text cells so that
    //the blank text is used for display instead of the number 0.
    //If this is working properly, the following cell should NOT have value data.
    CPPUNIT_ASSERT_EQUAL(pDoc->GetString(3,0,2), OUString());

    // fdo#59293 with cached value import error formulas require special
    // treatment
    pDoc->SetString(2, 5, 2, "=ISERROR(A6)");
    double nVal = pDoc->GetValue(2,5,2);
    CPPUNIT_ASSERT_EQUAL(1.0, nVal);

    xDocSh->DoClose();
}

namespace {

void testDBRanges_Impl(ScDocument* pDoc, sal_Int32 nFormat)
{
    ScDBCollection* pDBCollection = pDoc->GetDBCollection();
    CPPUNIT_ASSERT_MESSAGE("no database collection", pDBCollection);

    ScDBData* pAnonDBData = pDoc->GetAnonymousDBData(0);
    CPPUNIT_ASSERT_MESSAGE("missing anonymous DB data in sheet 1", pAnonDBData);
    //control hidden rows
    bool bHidden;
    SCROW nRow1, nRow2;
    bHidden = pDoc->RowHidden(0, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("Sheet1: row 0 should be visible", !bHidden && nRow1 == 0 && nRow2 == 0);
    bHidden = pDoc->RowHidden(1, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("Sheet1: rows 1-2 should be hidden", bHidden && nRow1 == 1 && nRow2 == 2);
    bHidden = pDoc->RowHidden(3, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("Sheet1: row 3 should be visible", !bHidden && nRow1 == 3 && nRow2 == 3);
    bHidden = pDoc->RowHidden(4, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("Sheet1: row 4-5 should be hidden", bHidden && nRow1 == 4 && nRow2 == 5);
    bHidden = pDoc->RowHidden(6, 0, &nRow1, &nRow2);
    CPPUNIT_ASSERT_MESSAGE("Sheet1: row 6-end should be visible", !bHidden && nRow1 == 6 && nRow2 == MAXROW);
    if(nFormat == ODS) //excel doesn't support named db ranges
    {
        double aValue;
        pDoc->GetValue(0,10,1, aValue);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Sheet2: A11: formula result is incorrect", 4.0, aValue);
        pDoc->GetValue(1, 10, 1, aValue);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Sheet2: B11: formula result is incorrect", 2.0, aValue);
    }
    double aValue;
    pDoc->GetValue(3,10,1, aValue);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Sheet2: D11: formula result is incorrect", 4.0, aValue);
    pDoc->GetValue(4, 10, 1, aValue);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Sheet2: E11: formula result is incorrect", 2.0, aValue);

}

}

void ScFiltersTest::testDatabaseRangesODS()
{
    ScDocShellRef xDocSh = loadDoc("database.", ODS);
    xDocSh->DoHardRecalc(true);

    ScDocument* pDoc = xDocSh->GetDocument();

    testDBRanges_Impl(pDoc, ODS);
    xDocSh->DoClose();
}

void ScFiltersTest::testDatabaseRangesXLS()
{
    ScDocShellRef xDocSh = loadDoc("database.", XLS);
    xDocSh->DoHardRecalc(true);

    ScDocument* pDoc = xDocSh->GetDocument();

    testDBRanges_Impl(pDoc, XLS);
    xDocSh->DoClose();
}

void ScFiltersTest::testDatabaseRangesXLSX()
{
    ScDocShellRef xDocSh = loadDoc("database.", XLSX);
    xDocSh->DoHardRecalc(true);

    ScDocument* pDoc = xDocSh->GetDocument();

    testDBRanges_Impl(pDoc, XLSX);
    xDocSh->DoClose();
}

namespace {

void testFormats_Impl(ScFiltersTest* pFiltersTest, ScDocument* pDoc, sal_Int32 nFormat)
{
    //test Sheet1 with csv file
    OUString aCSVFileName;
    pFiltersTest->createCSVPath(OUString("numberFormat."), aCSVFileName);
    testFile(aCSVFileName, pDoc, 0, PureString);
    //need to test the color of B3
    //it's not a font color!
    //formatting for B5: # ??/100 gets lost during import

    //test Sheet2
    const ScPatternAttr* pPattern = NULL;
    pPattern = pDoc->GetPattern(0,0,1);
    Font aFont;
    pPattern->GetFont(aFont,SC_AUTOCOL_RAW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("font size should be 10", 200l, aFont.GetSize().getHeight());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("font color should be black", COL_AUTO, aFont.GetColor().GetColor());
    pPattern = pDoc->GetPattern(0,1,1);
    pPattern->GetFont(aFont, SC_AUTOCOL_RAW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("font size should be 12", 240l, aFont.GetSize().getHeight());
    pPattern = pDoc->GetPattern(0,2,1);
    pPattern->GetFont(aFont, SC_AUTOCOL_RAW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("font should be italic", ITALIC_NORMAL, aFont.GetItalic());
    pPattern = pDoc->GetPattern(0,4,1);
    pPattern->GetFont(aFont, SC_AUTOCOL_RAW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("font should be bold", WEIGHT_BOLD, aFont.GetWeight());
    pPattern = pDoc->GetPattern(1,0,1);
    pPattern->GetFont(aFont, SC_AUTOCOL_RAW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("font should be blue", COL_BLUE, aFont.GetColor().GetColor());
    pPattern = pDoc->GetPattern(1,1,1);
    pPattern->GetFont(aFont, SC_AUTOCOL_RAW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("font should be striked out with a single line", STRIKEOUT_SINGLE, aFont.GetStrikeout());
    //some tests on sheet2 only for ods
    if (nFormat == ODS)
    {
        pPattern = pDoc->GetPattern(1,2,1);
        pPattern->GetFont(aFont, SC_AUTOCOL_RAW);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("font should be striked out with a double line", STRIKEOUT_DOUBLE, aFont.GetStrikeout());
        pPattern = pDoc->GetPattern(1,3,1);
        pPattern->GetFont(aFont, SC_AUTOCOL_RAW);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("font should be underlined with a dotted line", UNDERLINE_DOTTED, aFont.GetUnderline());
        //check row height import
        //disable for now until we figure out cause of win tinderboxes test failures
        //CPPUNIT_ASSERT_EQUAL( static_cast<sal_uInt16>(256), pDoc->GetRowHeight(0,1) ); //0.178in
        //CPPUNIT_ASSERT_EQUAL( static_cast<sal_uInt16>(304), pDoc->GetRowHeight(1,1) ); //0.211in
        //CPPUNIT_ASSERT_EQUAL( static_cast<sal_uInt16>(477), pDoc->GetRowHeight(5,1) ); //0.3311in
        //check column width import
        CPPUNIT_ASSERT_EQUAL( static_cast<sal_uInt16>(555), pDoc->GetColWidth(4,1) );  //0.3854in
        CPPUNIT_ASSERT_EQUAL( static_cast<sal_uInt16>(1280), pDoc->GetColWidth(5,1) ); //0.889in
        CPPUNIT_ASSERT_EQUAL( static_cast<sal_uInt16>(4153), pDoc->GetColWidth(6,1) ); //2.8839in
        //test case for i53253 where a cell has text with different styles and space between the text.
        OUString aTestStr = pDoc->GetString(3,0,1);
        OUString aKnownGoodStr("text14 space");
        CPPUNIT_ASSERT_EQUAL( aKnownGoodStr, aTestStr );
        //test case for cell text with line breaks.
        aTestStr = pDoc->GetString(3,5,1);
        aKnownGoodStr = "Hello,\nCalc!";
        CPPUNIT_ASSERT_EQUAL( aKnownGoodStr, aTestStr );
    }
    pPattern = pDoc->GetPattern(1,4,1);
    Color aColor = static_cast<const SvxBrushItem&>(pPattern->GetItem(ATTR_BACKGROUND)).GetColor();
    CPPUNIT_ASSERT_MESSAGE("background color should be green", aColor == COL_LIGHTGREEN);
    pPattern = pDoc->GetPattern(2,0,1);
    SvxCellHorJustify eHorJustify = static_cast<SvxCellHorJustify>(static_cast<const SvxHorJustifyItem&>(pPattern->GetItem(ATTR_HOR_JUSTIFY)).GetValue());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("cell content should be aligned centre horizontally", SVX_HOR_JUSTIFY_CENTER, eHorJustify);
    //test alignment
    pPattern = pDoc->GetPattern(2,1,1);
    eHorJustify = static_cast<SvxCellHorJustify>(static_cast<const SvxHorJustifyItem&>(pPattern->GetItem(ATTR_HOR_JUSTIFY)).GetValue());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("cell content should be aligned right horizontally", SVX_HOR_JUSTIFY_RIGHT, eHorJustify);
    pPattern = pDoc->GetPattern(2,2,1);
    eHorJustify = static_cast<SvxCellHorJustify>(static_cast<const SvxHorJustifyItem&>(pPattern->GetItem(ATTR_HOR_JUSTIFY)).GetValue());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("cell content should be aligned block horizontally", SVX_HOR_JUSTIFY_BLOCK, eHorJustify);

    //test Sheet3 only for ods
    if ( nFormat == ODS || nFormat == XLSX )
    {
        pFiltersTest->createCSVPath(OUString("conditionalFormatting."), aCSVFileName);
        testCondFile(aCSVFileName, pDoc, 2);
        // test parent cell style import ( fdo#55198 )
        if ( nFormat == XLSX )
        {
            pPattern = pDoc->GetPattern(1,1,3);
            ScStyleSheet* pStyleSheet = (ScStyleSheet*)pPattern->GetStyleSheet();
            // check parent style name
            OUString sExpected("Excel Built-in Date");
            OUString sResult = pStyleSheet->GetName();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("parent style for Sheet4.B2 is 'Excel Built-in Date'", sExpected, sResult);
            // check  align of style
            SfxItemSet& rItemSet = pStyleSheet->GetItemSet();
            eHorJustify = static_cast<SvxCellHorJustify>(static_cast< const SvxHorJustifyItem& >(rItemSet.Get( ATTR_HOR_JUSTIFY ) ).GetValue() );
            CPPUNIT_ASSERT_EQUAL_MESSAGE("'Excel Built-in Date' style should be aligned centre horizontally", SVX_HOR_JUSTIFY_CENTER, eHorJustify);
            // check date format ( should be just month e.g. 29 )
            sResult =pDoc->GetString( 1,1,3 );
            sExpected = OUString("29");
            CPPUNIT_ASSERT_EQUAL_MESSAGE("'Excel Built-in Date' style should just display month", sExpected, sResult );

            // check actual align applied to cell, should be the same as
            // the style
            eHorJustify = static_cast<SvxCellHorJustify>(static_cast< const SvxHorJustifyItem& >(pPattern->GetItem( ATTR_HOR_JUSTIFY ) ).GetValue() );
            CPPUNIT_ASSERT_EQUAL_MESSAGE("cell with 'Excel Built-in Date' style should be aligned centre horizontally", SVX_HOR_JUSTIFY_CENTER, eHorJustify);
        }
    }

    ScConditionalFormat* pCondFormat = pDoc->GetCondFormat(0,0,2);
    const ScRangeList& rRange = pCondFormat->GetRange();
    CPPUNIT_ASSERT(rRange == ScRange(0,0,2,3,0,2));

    pCondFormat = pDoc->GetCondFormat(0,1,2);
    const ScRangeList& rRange2 = pCondFormat->GetRange();
    CPPUNIT_ASSERT(rRange2 == ScRange(0,1,2,0,1,2));

    pCondFormat = pDoc->GetCondFormat(1,1,2);
    const ScRangeList& rRange3 = pCondFormat->GetRange();
    CPPUNIT_ASSERT(rRange3 == ScRange(1,1,2,3,1,2));
}

}

void ScFiltersTest::testFormatsODS()
{
    ScDocShellRef xDocSh = loadDoc("formats.", ODS);
    xDocSh->DoHardRecalc(true);

    ScDocument* pDoc = xDocSh->GetDocument();

    testFormats_Impl(this, pDoc, ODS);
    xDocSh->DoClose();
}

void ScFiltersTest::testFormatsXLS()
{
    ScDocShellRef xDocSh = loadDoc("formats.", XLS);
    xDocSh->DoHardRecalc(true);

    ScDocument* pDoc = xDocSh->GetDocument();

    testFormats_Impl(this, pDoc, XLS);
    xDocSh->DoClose();
}

void ScFiltersTest::testFormatsXLSX()
{
    ScDocShellRef xDocSh = loadDoc("formats.", XLSX);
    xDocSh->DoHardRecalc(true);

    ScDocument* pDoc = xDocSh->GetDocument();

    testFormats_Impl(this, pDoc, XLSX);
    xDocSh->DoClose();
}

void ScFiltersTest::testMatrixODS()
{
    ScDocShellRef xDocSh = loadDoc("matrix.", ODS);
    xDocSh->DoHardRecalc(true);

    ScDocument* pDoc = xDocSh->GetDocument();

    OUString aCSVFileName;
    createCSVPath(OUString("matrix."), aCSVFileName);
    testFile(aCSVFileName, pDoc, 0);

    xDocSh->DoClose();
}

void ScFiltersTest::testMatrixXLS()
{
    ScDocShellRef xDocSh = loadDoc("matrix.", XLS);
    xDocSh->DoHardRecalc(true);

    CPPUNIT_ASSERT_MESSAGE("Failed to load matrix.*", xDocSh.Is());
    ScDocument* pDoc = xDocSh->GetDocument();

    OUString aCSVFileName;
    createCSVPath(OUString("matrix."), aCSVFileName);
    testFile(aCSVFileName, pDoc, 0);

    xDocSh->DoClose();
}

void ScFiltersTest::testBorderODS()
{
    ScDocShellRef xDocSh = loadDoc("border.", ODS);

    CPPUNIT_ASSERT_MESSAGE("Failed to load border.*", xDocSh.Is());
    ScDocument* pDoc = xDocSh->GetDocument();

    const editeng::SvxBorderLine* pLeft = NULL;
    const editeng::SvxBorderLine* pTop = NULL;
    const editeng::SvxBorderLine* pRight = NULL;
    const editeng::SvxBorderLine* pBottom = NULL;

    pDoc->GetBorderLines( 0, 1, 0, &pLeft, &pTop, &pRight, &pBottom );
    CPPUNIT_ASSERT(!pLeft);
    CPPUNIT_ASSERT(!pTop);
    CPPUNIT_ASSERT(!pBottom);
    CPPUNIT_ASSERT(pRight);
    CPPUNIT_ASSERT_EQUAL(pRight->GetBorderLineStyle(),
            table::BorderLineStyle::SOLID);

    pDoc->GetBorderLines( 2, 1, 0, &pLeft, &pTop, &pRight, &pBottom );
    CPPUNIT_ASSERT(!pLeft);
    CPPUNIT_ASSERT(!pTop);
    CPPUNIT_ASSERT(!pBottom);

    CPPUNIT_ASSERT(pRight);
    CPPUNIT_ASSERT_EQUAL(pRight->GetBorderLineStyle(),
            table::BorderLineStyle::SOLID);
    CPPUNIT_ASSERT_EQUAL(pRight->GetWidth(),20L);

    pDoc->GetBorderLines( 2, 8, 0, &pLeft, &pTop, &pRight, &pBottom );

    CPPUNIT_ASSERT(pLeft);
    CPPUNIT_ASSERT(pTop);
    CPPUNIT_ASSERT(pBottom);
    CPPUNIT_ASSERT(pRight);
    CPPUNIT_ASSERT_EQUAL(pRight->GetBorderLineStyle(),
            table::BorderLineStyle::SOLID);
    CPPUNIT_ASSERT_EQUAL(pRight->GetWidth(),5L);
    CPPUNIT_ASSERT(pRight->GetColor() == Color(COL_BLUE));

    xDocSh->DoClose();
}

void ScFiltersTest::testBorderImpl( sal_uLong nFormatType )
{
    ScDocShellRef xDocSh = loadDoc("border.", nFormatType );

    CPPUNIT_ASSERT_MESSAGE("Failed to load border.xls", xDocSh.Is());
    ScDocument* pDoc = xDocSh->GetDocument();

    const editeng::SvxBorderLine* pLeft = NULL;
    const editeng::SvxBorderLine* pTop = NULL;
    const editeng::SvxBorderLine* pRight = NULL;
    const editeng::SvxBorderLine* pBottom = NULL;

    pDoc->GetBorderLines( 2, 3, 0, &pLeft, &pTop, &pRight, &pBottom );
    CPPUNIT_ASSERT(pRight);
    CPPUNIT_ASSERT_EQUAL(pRight->GetBorderLineStyle(),
            table::BorderLineStyle::SOLID);
    CPPUNIT_ASSERT_EQUAL(pRight->GetWidth(),1L);

    pDoc->GetBorderLines( 3, 5, 0, &pLeft, &pTop, &pRight, &pBottom );
    CPPUNIT_ASSERT(pRight);
    CPPUNIT_ASSERT_EQUAL(pRight->GetBorderLineStyle(),
            table::BorderLineStyle::SOLID);
    CPPUNIT_ASSERT_EQUAL(pRight->GetWidth(),20L);

    pDoc->GetBorderLines( 5, 7, 0, &pLeft, &pTop, &pRight, &pBottom );
    CPPUNIT_ASSERT(pRight);
    CPPUNIT_ASSERT_EQUAL(pRight->GetBorderLineStyle(),
            table::BorderLineStyle::SOLID);
    CPPUNIT_ASSERT_EQUAL(pRight->GetWidth(),30L);

    pDoc->GetBorderLines( 7, 9, 0, &pLeft, &pTop, &pRight, &pBottom );
    CPPUNIT_ASSERT(pRight);
    CPPUNIT_ASSERT_EQUAL(pRight->GetBorderLineStyle(),
            table::BorderLineStyle::FINE_DASHED);
    CPPUNIT_ASSERT_EQUAL(pRight->GetWidth(),1L);
}

void ScFiltersTest::testBorderXLS()
{
    testBorderImpl( XLS );
}

void ScFiltersTest::testBorderXLSX()
{
    testBorderImpl( XLSX );
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

    ScDocShellRef xDocSh = loadDoc("borders_ooo33.", ODS);

    CPPUNIT_ASSERT_MESSAGE("Failed to load borders_ooo33.*", xDocSh.Is());
    ScDocument* pDoc = xDocSh->GetDocument();

    const editeng::SvxBorderLine* pLeft = NULL;
    const editeng::SvxBorderLine* pTop = NULL;
    const editeng::SvxBorderLine* pRight = NULL;
    const editeng::SvxBorderLine* pBottom = NULL;
    sal_Int16 temp = 0;
    for(sal_Int16 i = 0; i<6; ++i)
    {
        for(sal_Int32 j = 0; j<22; ++j)
        {
            pDoc->GetBorderLines( i, j, 0, &pLeft, &pTop, &pRight, &pBottom );
            if(pLeft!=NULL && pTop!=NULL && pRight!=NULL && pBottom!=NULL)
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
    ScDocShellRef xDocSh = loadDoc("bug-fixes.", ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load bugFixes.ods", xDocSh.Is());

    xDocSh->DoHardRecalc(true);
    ScDocument* pDoc = xDocSh->GetDocument();

    {
        // fdo#40967
        OUString aCSVFileName;
        createCSVPath(OUString("bugFix_Sheet2."), aCSVFileName);
        testFile(aCSVFileName, pDoc, 1);
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
        OUString aCSVFileName;
        createCSVPath("bugFix_Sheet4.", aCSVFileName);
        testFile(aCSVFileName, pDoc, 3);
    }

    xDocSh->DoClose();
}

void ScFiltersTest::testBugFixesXLS()
{
    ScDocShellRef xDocSh = loadDoc("bug-fixes.", XLS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load bugFixes.xls", xDocSh.Is());

    xDocSh->DoHardRecalc(true);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT_MESSAGE("No Document", pDoc); //remove with first test
    xDocSh->DoClose();
}

void ScFiltersTest::testBugFixesXLSX()
{
    ScDocShellRef xDocSh = loadDoc("bug-fixes.", XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to load bugFixes.xls", xDocSh.Is());

    xDocSh->DoHardRecalc(true);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT_MESSAGE("No Document", pDoc); //remove with first test
    xDocSh->DoClose();
}

namespace {

void checkMergedCells( ScDocument* pDoc, const ScAddress& rStartAddress,
                       const ScAddress& rExpectedEndAddress )
{
    SCCOL nActualEndCol = rStartAddress.Col();
    SCROW nActualEndRow = rStartAddress.Row();
    pDoc->ExtendMerge( rStartAddress.Col(), rStartAddress.Row(),
                       nActualEndCol, nActualEndRow, rStartAddress.Tab(), false );
    OString sTab = OString::valueOf( static_cast<sal_Int32>(rStartAddress.Tab() + 1) );
    OString msg = "Merged cells are not correctly imported on sheet" + sTab;
    OString msgCol = msg + "; end col";
    OString msgRow = msg + "; end row";
    CPPUNIT_ASSERT_EQUAL_MESSAGE( msgCol.pData->buffer, rExpectedEndAddress.Col(), nActualEndCol );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( msgRow.pData->buffer, rExpectedEndAddress.Row(), nActualEndRow );
}

}

void ScFiltersTest::testMergedCellsODS()
{
    ScDocShellRef xDocSh = loadDoc("merged.", ODS);
    ScDocument* pDoc = xDocSh->GetDocument();

    //check sheet1 content
    OUString aCSVFileName1;
    createCSVPath(OUString("merged1."), aCSVFileName1);
    testFile(aCSVFileName1, pDoc, 0);

    //check sheet1 merged cells
    checkMergedCells( pDoc, ScAddress( 0, 0, 0 ),  ScAddress( 5, 11, 0 ) );
    checkMergedCells( pDoc, ScAddress( 7, 2, 0 ),  ScAddress( 9, 12, 0 ) );
    checkMergedCells( pDoc, ScAddress( 3, 15, 0 ),  ScAddress( 7, 23, 0 ) );

    //check sheet2 content
    OUString aCSVFileName2;
    createCSVPath(OUString("merged2."), aCSVFileName2);
    testFile(aCSVFileName2, pDoc, 1);

    //check sheet2 merged cells
    checkMergedCells( pDoc, ScAddress( 4, 3, 1 ),  ScAddress( 6, 15, 1 ) );

    xDocSh->DoClose();
}

void ScFiltersTest::testRepeatedColumnsODS()
{
    ScDocShellRef xDocSh = loadDoc("repeatedColumns.", ODS);
    ScDocument* pDoc = xDocSh->GetDocument();

    //text
    OUString aCSVFileName1;
    createCSVPath(OUString("repeatedColumns1."), aCSVFileName1);
    testFile(aCSVFileName1, pDoc, 0);

    //numbers
    OUString aCSVFileName2;
    createCSVPath(OUString("repeatedColumns2."), aCSVFileName2);
    testFile(aCSVFileName2, pDoc, 1);

    xDocSh->DoClose();
}

namespace {

//for cleaner passing of parameters
struct ValDataTestParams
{
    ScValidationMode eValMode;
    ScConditionMode eCondOp;
    String aStrVal1, aStrVal2;
    ScDocument* pDocument;
    ScAddress aPosition;
    String aErrorTitle, aErrorMessage;
    ScValidErrorStyle eErrorStyle;
    sal_uLong nExpectedIndex;

    ValDataTestParams( ScValidationMode eMode, ScConditionMode eOp,
                       String aExpr1, String aExpr2, ScDocument* pDoc,
                       ScAddress aPos, String aETitle, String aEMsg,
                       ScValidErrorStyle eEStyle, sal_uLong nIndex ):
                            eValMode(eMode), eCondOp(eOp), aStrVal1(aExpr1),
                            aStrVal2(aExpr2), pDocument(pDoc), aPosition(aPos),
                            aErrorTitle(aETitle), aErrorMessage(aEMsg),
                            eErrorStyle(eEStyle), nExpectedIndex(nIndex) { };
};

void checkValiditationEntries( const ValDataTestParams& rVDTParams )
{
    ScDocument* pDoc = rVDTParams.pDocument;

    //create expected data validation entry
    ScValidationData aValData(
        rVDTParams.eValMode, rVDTParams.eCondOp, rVDTParams.aStrVal1,
        rVDTParams.aStrVal2, pDoc, rVDTParams.aPosition, EMPTY_STRING,
        EMPTY_STRING, pDoc->GetStorageGrammar(), pDoc->GetStorageGrammar()
    );
    aValData.SetIgnoreBlank( true );
    aValData.SetListType( 1 );
    aValData.ResetInput();
    aValData.SetError( rVDTParams.aErrorTitle, rVDTParams.aErrorMessage, rVDTParams.eErrorStyle );
    aValData.SetSrcString( EMPTY_STRING );

    //get actual data validation entry from document
    const ScValidationData* pValDataTest = pDoc->GetValidationEntry( rVDTParams.nExpectedIndex );

    sal_Int32 nCol( static_cast<sal_Int32>(rVDTParams.aPosition.Col()) );
    sal_Int32 nRow( static_cast<sal_Int32>(rVDTParams.aPosition.Row()) );
    sal_Int32 nTab( static_cast<sal_Int32>(rVDTParams.aPosition.Tab()) );
    OStringBuffer sMsg("Data Validation Entry with base-cell-address: (");
    sMsg.append(nCol).append(",").append(nRow).append(",").append(nTab).append(") was not imported correctly.");
    //check if expected and actual data validation entries are equal
    CPPUNIT_ASSERT_MESSAGE( sMsg.getStr(), pValDataTest && aValData.EqualEntries(*pValDataTest) );
}

void checkCellValidity( const ScAddress& rValBaseAddr, const ScRange& rRange, const ScDocument* pDoc )
{
    SCCOL nBCol( rValBaseAddr.Col() );
    SCROW nBRow( rValBaseAddr.Row() );
    SCTAB nTab( static_cast<const sal_Int32>(rValBaseAddr.Tab()) );
    //get from the document the data validation entry we are checking against
    const SfxUInt32Item* pItem = static_cast<const SfxUInt32Item*>(pDoc->GetAttr(nBCol, nBRow, nTab, ATTR_VALIDDATA) );
    const ScValidationData* pValData = pDoc->GetValidationEntry( pItem->GetValue() );

    //check that each cell in the expected range is associated with the data validation entry
    for(SCCOL i = rRange.aStart.Col(); i <= rRange.aEnd.Col(); ++i)
    {
        for(SCROW j = rRange.aStart.Row(); j <= rRange.aEnd.Row(); ++j)
        {
            const SfxUInt32Item* pItemTest = static_cast<const SfxUInt32Item*>( pDoc->GetAttr(i, j, nTab, ATTR_VALIDDATA) );
            const ScValidationData* pValDataTest = pDoc->GetValidationEntry( pItemTest->GetValue() );
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
                sal_uInt32 actualKey(-1);
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
    ScDocShellRef xDocSh = loadDoc("dataValidity.", ODS);
    ScDocument* pDoc = xDocSh->GetDocument();

    ScAddress aValBaseAddr1( 2,6,0 ); //sheet1
    ScAddress aValBaseAddr2( 2,3,1 ); //sheet2

    //sheet1's expected Data Validation Entry values
    ValDataTestParams aVDTParams1(
        SC_VALID_DECIMAL, SC_COND_GREATER, String("3.14"), EMPTY_STRING, pDoc,
        aValBaseAddr1, String("Too small"),
        String("The number you are trying to enter is not greater than 3.14! Are you sure you want to enter it anyway?"),
        SC_VALERR_WARNING, 1
    );
    //sheet2's expected Data Validation Entry values
    ValDataTestParams aVDTParams2(
        SC_VALID_WHOLE, SC_COND_BETWEEN, String("1"), String("10"), pDoc,
        aValBaseAddr2, String("Error sheet 2"),
        String("Must be a whole number between 1 and 10."),
        SC_VALERR_STOP, 2
    );
    //check each sheet's Data Validation Entries
    checkValiditationEntries( aVDTParams1 );
    checkValiditationEntries( aVDTParams2 );

    //expected ranges to be associated with data validity
    ScRange aRange1( 2,2,0, 2,6,0 ); //sheet1
    ScRange aRange2( 2,3,1, 6,7,1 ); //sheet2

    //check each sheet's cells for data validity
    checkCellValidity( aValBaseAddr1, aRange1, pDoc );
    checkCellValidity( aValBaseAddr2, aRange2, pDoc );

    //check each sheet's content
    OUString aCSVFileName1;
    createCSVPath(OUString("dataValidity1."), aCSVFileName1);
    testFile(aCSVFileName1, pDoc, 0);

    OUString aCSVFileName2;
    createCSVPath(OUString("dataValidity2."), aCSVFileName2);
    testFile(aCSVFileName2, pDoc, 1);

    xDocSh->DoClose();
}

void ScFiltersTest::testBrokenQuotesCSV()
{
    const OUString aFileNameBase("fdo48621_broken_quotes.");
    OUString aFileExtension(getFileFormats()[CSV].pName, strlen(getFileFormats()[CSV].pName), RTL_TEXTENCODING_UTF8 );
    OUString aFilterName(getFileFormats()[CSV].pFilterName, strlen(getFileFormats()[CSV].pFilterName), RTL_TEXTENCODING_UTF8) ;
    OUString aFileName;
    createFileURL(aFileNameBase, aFileExtension, aFileName);
    OUString aFilterType(getFileFormats()[CSV].pTypeName, strlen(getFileFormats()[CSV].pTypeName), RTL_TEXTENCODING_UTF8);
    std::cout << getFileFormats()[CSV].pName << " Test" << std::endl;

    unsigned int nFormatType = getFileFormats()[CSV].nFormatType;
    unsigned int nClipboardId = nFormatType ? SFX_FILTER_IMPORT | SFX_FILTER_USESOPTIONS : 0;
    ScDocShellRef xDocSh = ScBootstrapFixture::load(aFileName, aFilterName, OUString(), aFilterType,
        nFormatType, nClipboardId, SOFFICE_FILEFORMAT_CURRENT);

    CPPUNIT_ASSERT_MESSAGE("Failed to load fdo48621_broken_quotes.csv", xDocSh.Is());
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT_MESSAGE("No Document", pDoc); //remove with first test

    OUString aSheet2CSV("fdo48621_broken_quotes_exported.");
    OUString aCSVPath;
    createCSVPath( aSheet2CSV, aCSVPath );
    // fdo#48621
    testFile( aCSVPath, pDoc, 0, PureString);

    xDocSh->DoClose();
}

void ScFiltersTest::testCellValueXLSX()
{
    const OUString aFileNameBase("cell-value.");
    OUString aFileExtension(getFileFormats()[XLSX].pName, strlen(getFileFormats()[XLSX].pName), RTL_TEXTENCODING_UTF8 );
    OUString aFilterName(getFileFormats()[XLSX].pFilterName, strlen(getFileFormats()[XLSX].pFilterName), RTL_TEXTENCODING_UTF8) ;
    OUString aFileName;
    createFileURL(aFileNameBase, aFileExtension, aFileName);
    OUString aFilterType(getFileFormats()[XLSX].pTypeName, strlen(getFileFormats()[XLSX].pTypeName), RTL_TEXTENCODING_UTF8);
    std::cout << getFileFormats()[XLSX].pName << " Test" << std::endl;

    unsigned int nFormatType = getFileFormats()[XLSX].nFormatType;
    unsigned int nClipboardId = nFormatType ? SFX_FILTER_IMPORT | SFX_FILTER_USESOPTIONS : 0;
    ScDocShellRef xDocSh = ScBootstrapFixture::load( aFileName, aFilterName, OUString(), aFilterType,
        nFormatType, nClipboardId, SOFFICE_FILEFORMAT_CURRENT);

    CPPUNIT_ASSERT_MESSAGE("Failed to load cell-value.xlsx", xDocSh.Is());
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT_MESSAGE("No Document", pDoc); //remove with first test

    OUString aCSVPath;
    createCSVPath( aFileNameBase, aCSVPath );
    testFile( aCSVPath, pDoc, 0 );

    xDocSh->DoClose();
}

void ScFiltersTest::testPassword_Impl(const OUString& aFileNameBase)
{
    OUString aFileExtension(getFileFormats()[0].pName, strlen(getFileFormats()[0].pName), RTL_TEXTENCODING_UTF8 );
    OUString aFilterName(getFileFormats()[0].pFilterName, strlen(getFileFormats()[0].pFilterName), RTL_TEXTENCODING_UTF8) ;
    OUString aFileName;
    createFileURL(aFileNameBase, aFileExtension, aFileName);
    OUString aFilterType(getFileFormats()[0].pTypeName, strlen(getFileFormats()[0].pTypeName), RTL_TEXTENCODING_UTF8);

    sal_uInt32 nFormat = SFX_FILTER_IMPORT | SFX_FILTER_USESOPTIONS;
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
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT_MESSAGE("No Document", pDoc); //remove with first test
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

void ScFiltersTest::testControlImport()
{
    ScDocShellRef xDocSh = loadDoc("singlecontrol.", XLSX);
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
    ScDocShellRef xDocSh = loadDoc("chart-import-basic.", ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load chart-import-basic.ods.", xDocSh.Is());

    ScDocument* pDoc = xDocSh->GetDocument();

    // Ensure that the document contains "Empty", "Chart", "Data" and "Title" sheets in this exact order.
    CPPUNIT_ASSERT_MESSAGE("There should be 4 sheets in this document.", pDoc->GetTableCount() == 4);
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
    ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve the drawing layer object.", pDrawLayer);
    const SdrPage* pPage = pDrawLayer->GetPage(1); // for the 2nd sheet.
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve the page object.", pPage);
    CPPUNIT_ASSERT_MESSAGE("This page should contain one drawing object.", pPage->GetObjCount() == 1);
    const SdrObject* pObj = pPage->GetObj(0);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve the drawing object.", pObj);
    CPPUNIT_ASSERT_MESSAGE("This is not an OLE2 object.", pObj->GetObjIdentifier() == OBJ_OLE2);
    const SdrOle2Obj& rOleObj = static_cast<const SdrOle2Obj&>(*pObj);
    CPPUNIT_ASSERT_MESSAGE("This should be a chart object.", rOleObj.IsChart());

    // Make sure the chart object has correct range references.
    Reference<frame::XModel> xModel = rOleObj.getXModel();
    CPPUNIT_ASSERT_MESSAGE("Failed to get the embedded object interface.", xModel.is());
    Reference<chart2::XChartDocument> xChartDoc(xModel, UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("Failed to get the chart document interface.", xChartDoc.is());
    Reference<chart2::data::XDataSource> xDataSource(xChartDoc, UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("Failed to get the data source interface.", xDataSource.is());
    Sequence<Reference<chart2::data::XLabeledDataSequence> > xDataSeqs = xDataSource->getDataSequences();
    CPPUNIT_ASSERT_MESSAGE("There should be at least one data sequences.", xDataSeqs.getLength() > 0);
    Reference<chart2::data::XDataReceiver> xDataRec(xChartDoc, UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("Failed to get the data receiver interface.", xDataRec.is());
    Sequence<OUString> aRangeReps = xDataRec->getUsedRangeRepresentations();
    CPPUNIT_ASSERT_MESSAGE("There should be at least one range representations.", aRangeReps.getLength() > 0);

    ScRangeList aRanges;
    for (sal_Int32 i = 0, n = aRangeReps.getLength(); i < n; ++i)
    {
        ScRange aRange;
        sal_uInt16 nRes = aRange.Parse(aRangeReps[i], pDoc, pDoc->GetAddressConvention());
        if (nRes & SCA_VALID)
            // This is a range address.
            aRanges.Append(aRange);
        else
        {
            // Parse it as a single cell address.
            ScAddress aAddr;
            nRes = aAddr.Parse(aRangeReps[i], pDoc, pDoc->GetAddressConvention());
            CPPUNIT_ASSERT_MESSAGE("Failed to parse a range representation.", (nRes & SCA_VALID));
            aRanges.Append(aAddr);
        }
    }

    CPPUNIT_ASSERT_MESSAGE("Data series title cell not found.", aRanges.In(ScAddress(1,0,3))); // B1 on Title
    CPPUNIT_ASSERT_MESSAGE("Data series label range not found.", aRanges.In(ScRange(0,1,2,0,3,2))); // A2:A4 on Data
    CPPUNIT_ASSERT_MESSAGE("Data series value range not found.", aRanges.In(ScRange(1,1,2,1,3,2))); // B2:B4 on Data

    xDocSh->DoClose();
}

void ScFiltersTest::testNumberFormatHTML()
{
    ScDocShellRef xDocSh = loadDoc("numberformat.", HTML);
    CPPUNIT_ASSERT_MESSAGE("Failed to load numberformat.html", xDocSh.Is());

    ScDocument* pDoc = xDocSh->GetDocument();

    // Check the header just in case.
    CPPUNIT_ASSERT_MESSAGE("Cell value is not as expected", pDoc->GetString(0, 0, 0) == "Product");
    CPPUNIT_ASSERT_MESSAGE("Cell value is not as expected", pDoc->GetString(1, 0, 0) == "Price");
    CPPUNIT_ASSERT_MESSAGE("Cell value is not as expected", pDoc->GetString(2, 0, 0) == "Note");

    // B2 should be imported as a value cell.
    bool bHasValue = pDoc->HasValueData(1, 1, 0);
    CPPUNIT_ASSERT_MESSAGE("Fail to import number as a value cell.", bHasValue);
    CPPUNIT_ASSERT_MESSAGE("Incorrect value.", pDoc->GetValue(1, 1, 0) == 199.98);

    xDocSh->DoClose();
}

void ScFiltersTest::testNumberFormatCSV()
{
    ScDocShellRef xDocSh = loadDoc("numberformat.", CSV);
    CPPUNIT_ASSERT_MESSAGE("Failed to load numberformat.csv", xDocSh.Is());

    ScDocument* pDoc = xDocSh->GetDocument();

    // Check the header just in case.
    CPPUNIT_ASSERT_MESSAGE("Cell value is not as expected", pDoc->GetString(0, 0, 0) == "Product");
    CPPUNIT_ASSERT_MESSAGE("Cell value is not as expected", pDoc->GetString(1, 0, 0) == "Price");
    CPPUNIT_ASSERT_MESSAGE("Cell value is not as expected", pDoc->GetString(2, 0, 0) == "Note");

    // B2 should be imported as a value cell.
    bool bHasValue = pDoc->HasValueData(1, 1, 0);
    CPPUNIT_ASSERT_MESSAGE("Fail to import number as a value cell.", bHasValue);
    CPPUNIT_ASSERT_MESSAGE("Incorrect value.", pDoc->GetValue(1, 1, 0) == 199.98);

    xDocSh->DoClose();
}

void ScFiltersTest::testCellAnchoredShapesODS()
{
    ScDocShellRef xDocSh = loadDoc("cell-anchored-shapes.", ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load cell-anchored-shapes.ods", xDocSh.Is());

    // There are two cell-anchored objects on the first sheet.
    ScDocument* pDoc = xDocSh->GetDocument();

    CPPUNIT_ASSERT_MESSAGE("There should be at least one sheet.", pDoc->GetTableCount() > 0);

    ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
    SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("draw page for sheet 1 should exist.", pPage);
    sal_uIntPtr nCount = pPage->GetObjCount();
    CPPUNIT_ASSERT_MESSAGE("There should be 2 objects.", nCount == 2);
    for (sal_uIntPtr i = 0; i < nCount; ++i)
    {
        SdrObject* pObj = pPage->GetObj(i);
        CPPUNIT_ASSERT_MESSAGE("Failed to get drawing object.", pObj);
        ScDrawObjData* pData = ScDrawLayer::GetObjData(pObj, false);
        CPPUNIT_ASSERT_MESSAGE("Failed to retrieve user data for this object.", pData);
        CPPUNIT_ASSERT_MESSAGE("Bounding rectangle should have been calculated upon import.", !pData->maLastRect.IsEmpty());
    }

    xDocSh->DoClose();
}

namespace {

class FindDimByName : std::unary_function<const ScDPSaveDimension*, bool>
{
    OUString maName;
public:
    FindDimByName(const OUString& rName) : maName(rName) {}

    bool operator() (const ScDPSaveDimension* p) const
    {
        return p && p->GetName() == maName;
    }
};

bool hasDimension(const std::vector<const ScDPSaveDimension*>& rDims, const OUString& aName)
{
    return std::find_if(rDims.begin(), rDims.end(), FindDimByName(aName)) != rDims.end();
}

}

void ScFiltersTest::testPivotTableBasicODS()
{
    ScDocShellRef xDocSh = loadDoc("pivot-table-basic.", ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load pivot-table-basic.ods", xDocSh.Is());

    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT_MESSAGE("There should be exactly two sheets.", pDoc->GetTableCount() == 2);

    ScDPCollection* pDPs = pDoc->GetDPCollection();
    CPPUNIT_ASSERT_MESSAGE("Failed to get a live ScDPCollection instance.", pDPs);
    CPPUNIT_ASSERT_MESSAGE("There should be exactly one pivot table instance.", pDPs->GetCount() == 1);

    const ScDPObject* pDPObj = (*pDPs)[0];
    CPPUNIT_ASSERT_MESSAGE("Failed to get an pivot table object.", pDPObj);
    const ScDPSaveData* pSaveData = pDPObj->GetSaveData();
    CPPUNIT_ASSERT_MESSAGE("Failed to get ScDPSaveData instance.", pSaveData);
    std::vector<const ScDPSaveDimension*> aDims;

    // Row fields
    pSaveData->GetAllDimensionsByOrientation(sheet::DataPilotFieldOrientation_ROW, aDims);
    CPPUNIT_ASSERT_MESSAGE("There should be exactly 3 row fields (2 normal dimensions and 1 layout dimension).", aDims.size() == 3);
    CPPUNIT_ASSERT_MESSAGE("Dimension expected, but not found.", hasDimension(aDims, "Row1"));
    CPPUNIT_ASSERT_MESSAGE("Dimension expected, but not found.", hasDimension(aDims, "Row2"));
    const ScDPSaveDimension* pDataLayout = pSaveData->GetExistingDataLayoutDimension();
    CPPUNIT_ASSERT_MESSAGE("There should be a data layout field as a row field.",
                           pDataLayout && pDataLayout->GetOrientation() == sheet::DataPilotFieldOrientation_ROW);

    // Column fields
    pSaveData->GetAllDimensionsByOrientation(sheet::DataPilotFieldOrientation_COLUMN, aDims);
    CPPUNIT_ASSERT_MESSAGE("There should be exactly 2 column fields.", aDims.size() == 2);
    CPPUNIT_ASSERT_MESSAGE("Dimension expected, but not found.", hasDimension(aDims, "Col1"));
    CPPUNIT_ASSERT_MESSAGE("Dimension expected, but not found.", hasDimension(aDims, "Col2"));

    // Page fields
    pSaveData->GetAllDimensionsByOrientation(sheet::DataPilotFieldOrientation_PAGE, aDims);
    CPPUNIT_ASSERT_MESSAGE("There should be exactly 2 page fields.", aDims.size() == 2);
    CPPUNIT_ASSERT_MESSAGE("Dimension expected, but not found.", hasDimension(aDims, "Page1"));
    CPPUNIT_ASSERT_MESSAGE("Dimension expected, but not found.", hasDimension(aDims, "Page2"));

    // Check the data field.
    pSaveData->GetAllDimensionsByOrientation(sheet::DataPilotFieldOrientation_DATA, aDims);
    CPPUNIT_ASSERT_MESSAGE("There should be exactly 1 data field.", aDims.size() == 1);
    const ScDPSaveDimension* pDim = aDims.back();
    CPPUNIT_ASSERT_MESSAGE("Function for the data field should be COUNT.", pDim->GetFunction() == sheet::GeneralFunction_COUNT);

    xDocSh->DoClose();
}

void ScFiltersTest::testRowHeightODS()
{
    ScDocShellRef xDocSh = loadDoc("row-height-import.", ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load row-height-import.ods", xDocSh.Is());

    SCTAB nTab = 0;
    SCROW nRow = 0;
    ScDocument* pDoc = xDocSh->GetDocument();

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

    bManual = pDoc->IsManualRowHeight(MAXROW, nTab);
    CPPUNIT_ASSERT_MESSAGE("Row should have an automatic height.", !bManual);

    xDocSh->DoClose();
}

void ScFiltersTest::testRichTextContentODS()
{
    ScDocShellRef xDocSh = loadDoc("rich-text-cells.", ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load rich-text-cells.ods", xDocSh.Is());
    ScDocument* pDoc = xDocSh->GetDocument();

    OUString aTabName;
    CPPUNIT_ASSERT_MESSAGE("Failed to get the name of the first sheet.", pDoc->GetName(0, aTabName));

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
        CPPUNIT_ASSERT_EQUAL(OUString("All bold"), pDoc->GetString(aPos.Col(), aPos.Row(), aPos.Tab()));
        const ScPatternAttr* pAttr = pDoc->GetPattern(aPos.Col(), aPos.Row(), aPos.Tab());
        CPPUNIT_ASSERT_MESSAGE("Failed to get cell attribute.", pAttr);
        const SvxWeightItem& rWeightItem =
            static_cast<const SvxWeightItem&>(pAttr->GetItem(ATTR_FONT_WEIGHT));
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
    CPPUNIT_ASSERT_MESSAGE("Unexpected text.", aParaText.indexOf("Sheet name is ") == 0);
    CPPUNIT_ASSERT_MESSAGE("Sheet name field item not found.", pEditText->HasField(text::textfield::Type::TABLE));
    CPPUNIT_ASSERT_EQUAL(OUString("Sheet name is Test."), ScEditUtil::GetString(*pEditText, pDoc));
    CPPUNIT_ASSERT_EQUAL(OUString("Sheet name is ?."), ScEditUtil::GetString(*pEditText, NULL));

    // Cell with URL field item.
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_EDIT, pDoc->GetCellType(aPos));
    pEditText = pDoc->GetEditText(aPos);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve edit text object.", pEditText);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), pEditText->GetParagraphCount());
    aParaText = pEditText->GetText(0);
    CPPUNIT_ASSERT_MESSAGE("Unexpected text.", aParaText.indexOf("URL: ") == 0);
    CPPUNIT_ASSERT_MESSAGE("URL field item not found.", pEditText->HasField(text::textfield::Type::URL));
    CPPUNIT_ASSERT_EQUAL(OUString("URL: http://libreoffice.org"), ScEditUtil::GetString(*pEditText, pDoc));
    CPPUNIT_ASSERT_EQUAL(OUString("URL: http://libreoffice.org"), ScEditUtil::GetString(*pEditText, NULL));

    // Cell with Date field item.
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_EDIT, pDoc->GetCellType(aPos));
    pEditText = pDoc->GetEditText(aPos);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve edit text object.", pEditText);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), pEditText->GetParagraphCount());
    aParaText = pEditText->GetText(0);
    CPPUNIT_ASSERT_MESSAGE("Unexpected text.", aParaText.indexOf("Date: ") == 0);
    CPPUNIT_ASSERT_MESSAGE("Date field item not found.", pEditText->HasField(text::textfield::Type::DATE));
    CPPUNIT_ASSERT_MESSAGE("Date field not resolved with pDoc.", ScEditUtil::GetString(*pEditText, pDoc).indexOf("/20") > 0);
    CPPUNIT_ASSERT_MESSAGE("Date field not resolved with NULL.", ScEditUtil::GetString(*pEditText, NULL).indexOf("/20") > 0);

    // Cell with DocInfo title field item.
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_EDIT, pDoc->GetCellType(aPos));
    pEditText = pDoc->GetEditText(aPos);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve edit text object.", pEditText);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), pEditText->GetParagraphCount());
    aParaText = pEditText->GetText(0);
    CPPUNIT_ASSERT_MESSAGE("Unexpected text.", aParaText.indexOf("Title: ") == 0);
    CPPUNIT_ASSERT_MESSAGE("DocInfo title field item not found.", pEditText->HasField(text::textfield::Type::DOCINFO_TITLE));
    CPPUNIT_ASSERT_EQUAL(OUString("Title: Test Document"), ScEditUtil::GetString(*pEditText, pDoc));
    CPPUNIT_ASSERT_EQUAL(OUString("Title: ?"), ScEditUtil::GetString(*pEditText, NULL));

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
    pEditText = pDoc->GetEditText(aPos);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve edit text object.", pEditText);
    CPPUNIT_ASSERT_MESSAGE("URL field item not found.", pEditText->HasField(text::textfield::Type::URL));

    // Sheet name with formats applied.
    aPos.IncRow();
    pEditText = pDoc->GetEditText(aPos);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve edit text object.", pEditText);
    CPPUNIT_ASSERT_MESSAGE("Sheet name field item not found.", pEditText->HasField(text::textfield::Type::TABLE));

    // Date with formats applied.
    aPos.IncRow();
    pEditText = pDoc->GetEditText(aPos);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve edit text object.", pEditText);
    CPPUNIT_ASSERT_MESSAGE("Date field item not found.", pEditText->HasField(text::textfield::Type::DATE));

    // Document title with formats applied.
    aPos.IncRow();
    pEditText = pDoc->GetEditText(aPos);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve edit text object.", pEditText);
    CPPUNIT_ASSERT_MESSAGE("Date field item not found.", pEditText->HasField(text::textfield::Type::DOCINFO_TITLE));

    // URL for a file in the same directory. It should be converted into an absolute URL on import.
    aPos.IncRow();
    pEditText = pDoc->GetEditText(aPos);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve edit text object.", pEditText);
    const SvxFieldData* pData = pEditText->GetFieldData(0, 0, text::textfield::Type::URL);
    CPPUNIT_ASSERT_MESSAGE("Failed to get the URL data.", pData && pData->GetClassId() == text::textfield::Type::URL);
    const SvxURLField* pURLData = static_cast<const SvxURLField*>(pData);
    CPPUNIT_ASSERT_MESSAGE("URL is not absolute with respect to the file system.", pURLData->GetURL().startsWith("file:///"));

    // Embedded spaces as <text:s text:c='4' />, normal text
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_STRING, pDoc->GetCellType(aPos));
    CPPUNIT_ASSERT_EQUAL(OUString("one     two"), pDoc->GetString(aPos.Col(), aPos.Row(), aPos.Tab()));

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
    ScDocShellRef xDocSh = loadDoc("databar.", ODS);
    CPPUNIT_ASSERT(xDocSh.Is());

    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    testDataBar_Impl(pDoc);

    xDocSh->DoClose();
}

void ScFiltersTest::testDataBarXLSX()
{
    ScDocShellRef xDocSh = loadDoc("databar.", XLSX);
    CPPUNIT_ASSERT(xDocSh.Is());

    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    testDataBar_Impl(pDoc);

    xDocSh->DoClose();
}

void ScFiltersTest::testColorScaleODS()
{
    ScDocShellRef xDocSh = loadDoc("colorscale.", ODS);
    CPPUNIT_ASSERT(xDocSh.Is());
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    testColorScale2Entry_Impl(pDoc);
    testColorScale3Entry_Impl(pDoc);

    xDocSh->DoClose();
}

void ScFiltersTest::testColorScaleXLSX()
{
    ScDocShellRef xDocSh = loadDoc("colorscale.", XLSX);
    CPPUNIT_ASSERT(xDocSh.Is());
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    testColorScale2Entry_Impl(pDoc);
    testColorScale3Entry_Impl(pDoc);

    xDocSh->DoClose();
}

void ScFiltersTest::testNewCondFormatODS()
{
    ScDocShellRef xDocSh = ScBootstrapFixture::loadDoc( "new_cond_format_test.", ODS );

    CPPUNIT_ASSERT_MESSAGE("Failed to load new_cond_format_test.xlsx", xDocSh.Is());

    ScDocument* pDoc = xDocSh->GetDocument();

    OUString aCSVFile("new_cond_format_test.");
    OUString aCSVPath;
    createCSVPath( aCSVFile, aCSVPath );
    testCondFile(aCSVPath, pDoc, 0);

    xDocSh->DoClose();
}

void ScFiltersTest::testNewCondFormatXLSX()
{
    ScDocShellRef xDocSh = ScBootstrapFixture::loadDoc( "new_cond_format_test.", XLSX );

    CPPUNIT_ASSERT_MESSAGE("Failed to load new_cond_format_test.xlsx", xDocSh.Is());

    ScDocument* pDoc = xDocSh->GetDocument();

    OUString aCSVFile("new_cond_format_test.");
    OUString aCSVPath;
    createCSVPath( aCSVFile, aCSVPath );
    testCondFile(aCSVPath, pDoc, 0);

    xDocSh->DoClose();
}

void ScFiltersTest::testFormulaDependency()
{
    ScDocShellRef xDocSh = ScBootstrapFixture::loadDoc( "dependencyTree.", ODS );

    ScDocument* pDoc = xDocSh->GetDocument();

    // check if formula in A1 changes value
    double nVal = pDoc->GetValue(0,0,0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(nVal, 1.0, 1e-10);
    pDoc->SetValue(0,1,0, 0.0);
    nVal = pDoc->GetValue(0,0,0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(nVal, 2.0, 1e-10);

    // check that the number format is implicity inherited
    // CPPUNIT_ASSERT_EQUAL(pDoc->GetString(0,4,0), pDoc->GetString(0,5,0));

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
        { "alldefaultheights.", ODS, -1, SAL_N_ELEMENTS(DfltRowData), DfltRowData },
        /* Checks the imported height of some multiline input, additionally checks
           that the optimal height flag is set ( or not )
        */
        { "multilineoptimal.", ODS, -1, SAL_N_ELEMENTS(MultiLineOptData), MultiLineOptData },
    };
    miscRowHeightsTest( aTestValues, SAL_N_ELEMENTS(aTestValues) );
}

// regression test at least fdo#59193
// what we want to test here is that when cell contents are deleted
// and the optimal flag is set for that row that the row is actually resized

void ScFiltersTest::testOptimalHeightReset()
{
    ScDocShellRef xDocSh = loadDoc("multilineoptimal.", ODS, true);
    SCTAB nTab = 0;
    SCROW nRow = 0;
    ScDocument* pDoc = xDocSh->GetDocument();
    pDoc->EnableAdjustHeight( true );
    // open document in read/write mode ( otherwise optimal height stuff won't
    // be triggered ) *and* you can't delete cell contents.
    int nHeight = sc::TwipsToHMM ( pDoc->GetRowHeight(nRow, nTab, false) );
    CPPUNIT_ASSERT_EQUAL(1263, nHeight);

    ScDocFunc &rFunc = xDocSh->GetDocFunc();

    // delete content of A1
    ScRange aDelRange(0,0,0,0,0,0);
    ScMarkData aMark;
    aMark.SetMarkArea(aDelRange);
    rFunc.DeleteContents( aMark, IDF_ALL, false, true );

    // get the new height of A1
    nHeight =  sc::TwipsToHMM( pDoc->GetRowHeight(nRow, nTab, false) );

    // set optimal height for empty row 2
    SCCOLROW nRowArr[2];
    nRowArr[0] = nRowArr[1] = 2;
    rFunc.SetWidthOrHeight( false, 1, nRowArr, nTab, SC_SIZE_OPTIMAL, 0, sal_True, sal_True );

    // retrieve optimal height
    int nOptimalHeight = sc::TwipsToHMM( pDoc->GetRowHeight( nRowArr[0], nTab, false) );

    // check if the new height of A1 ( after delete ) is now the optimal height of an empty cell
    CPPUNIT_ASSERT_EQUAL(nOptimalHeight, nHeight );
    xDocSh->DoClose();
}

void ScFiltersTest::testPrintRangeODS()
{
    ScDocShellRef xDocSh = loadDoc("print-range.", ODS);
    ScDocument* pDoc = xDocSh->GetDocument();
    const ScRange* pRange = pDoc->GetRepeatRowRange(0);
    CPPUNIT_ASSERT(pRange);
    CPPUNIT_ASSERT_EQUAL(ScRange(0,0,0,0,1,0), *pRange);

    pRange = pDoc->GetRepeatRowRange(1);
    CPPUNIT_ASSERT(pRange);
    CPPUNIT_ASSERT_EQUAL(ScRange(0,2,0,0,4,0), *pRange);
}

void ScFiltersTest::testOutlineODS()
{
    ScDocShellRef xDocSh = loadDoc("outline.", ODS);
    ScDocument* pDoc = xDocSh->GetDocument();

    const ScOutlineTable* pTable = pDoc->GetOutlineTable(0);
    CPPUNIT_ASSERT(pTable);

    const ScOutlineArray* pArr = pTable->GetRowArray();
    size_t nDepth = pArr->GetDepth();
    CPPUNIT_ASSERT_EQUAL(size_t(4), nDepth);

    for(size_t i = 0; i < nDepth; ++i)
    {
        CPPUNIT_ASSERT_EQUAL(size_t(1), pArr->GetCount(i));
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

        const ScOutlineEntry* pEntry = pArr->GetEntry(aRow[i].nDepth, aRow[i].nIndex);
        SCCOLROW nStart = pEntry->GetStart();
        CPPUNIT_ASSERT_EQUAL(aRow[i].nStart, nStart);

        SCCOLROW nEnd = pEntry->GetEnd();
        CPPUNIT_ASSERT_EQUAL(aRow[i].nEnd, nEnd);

        bool bHidden = pEntry->IsHidden();
        CPPUNIT_ASSERT_EQUAL(aRow[i].bHidden, bHidden);

        bool bVisible = pEntry->IsVisible();
        CPPUNIT_ASSERT_EQUAL(aRow[i].bVisible, bVisible);
    }
}

ScFiltersTest::ScFiltersTest()
      : ScBootstrapFixture( "/sc/qa/unit/data" )
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
