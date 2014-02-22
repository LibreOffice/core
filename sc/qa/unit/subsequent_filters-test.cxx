/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
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

#include "drwlayer.hxx"
#include "svx/svdpage.hxx"
#include "svx/svdoole2.hxx"
#include "editeng/wghtitem.hxx"
#include "editeng/postitem.hxx"
#include "editeng/udlnitem.hxx"
#include "editeng/editobj.hxx"
#include <editeng/borderline.hxx>
#include "editeng/flditem.hxx"
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
#include "dpsave.hxx"
#include "dpshttab.hxx"
#include <scopetools.hxx>

#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include <com/sun/star/sheet/DataPilotFieldGroupBy.hpp>
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

    
    void testBasicCellContentODS();
    void testRangeNameXLS();
    void testRangeNameXLSX();
    void testHyperlinksXLSX();
    void testHardRecalcODS();
    void testFunctionsODS();
    void testFunctionsExcel2010();
    void testCachedFormulaResultsODS();
    void testCachedMatrixFormulaResultsODS();
    void testFormulaDepAcrossSheetsODS();
    void testFormulaDepDeleteContentsODS();
    void testDatabaseRangesODS();
    void testDatabaseRangesXLS();
    void testDatabaseRangesXLSX();
    void testFormatsODS();
    void testFormatsXLS();
    void testFormatsXLSX();
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

    
    void testCellValueXLSX();

    
    void testPasswordNew();
    void testPasswordOld();
    void testPasswordWrongSHA();

    
    void testControlImport();
    void testChartImportODS();

    void testNumberFormatHTML();
    void testNumberFormatCSV();

    void testCellAnchoredShapesODS();

    void testPivotTableBasicODS();
    void testPivotTableSharedCacheGroupODS();
    void testGetPivotDataXLS();

    void testFormulaDependency();

    void testRowHeightODS();
    void testRichTextContentODS();
    void testMiscRowHeights();
    void testOptimalHeightReset();

    void testPrintRangeODS();
    void testOutlineODS();

    void testColumnStyleXLSX();

    void testSharedFormulaHorizontalXLS();

    CPPUNIT_TEST_SUITE(ScFiltersTest);
    CPPUNIT_TEST(testBasicCellContentODS);
    CPPUNIT_TEST(testRangeNameXLS);
    CPPUNIT_TEST(testRangeNameXLSX);
    CPPUNIT_TEST(testHyperlinksXLSX);
    CPPUNIT_TEST(testHardRecalcODS);
    CPPUNIT_TEST(testFunctionsODS);
    CPPUNIT_TEST(testFunctionsExcel2010);
    CPPUNIT_TEST(testCachedFormulaResultsODS);
    CPPUNIT_TEST(testFormulaDepAcrossSheetsODS);
    CPPUNIT_TEST(testFormulaDepDeleteContentsODS);
    CPPUNIT_TEST(testCachedMatrixFormulaResultsODS);
    CPPUNIT_TEST(testDatabaseRangesODS);
    CPPUNIT_TEST(testDatabaseRangesXLS);
    CPPUNIT_TEST(testDatabaseRangesXLSX);
    CPPUNIT_TEST(testFormatsODS);


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
    CPPUNIT_TEST(testPivotTableSharedCacheGroupODS);
    CPPUNIT_TEST(testGetPivotDataXLS);
    CPPUNIT_TEST(testRowHeightODS);
    CPPUNIT_TEST(testFormulaDependency);
    CPPUNIT_TEST(testRichTextContentODS);

    
    
#if !defined(MACOSX) && !defined(DRAGONFLY) && !defined(WNT)
    CPPUNIT_TEST(testPasswordWrongSHA);
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
    CPPUNIT_TEST(testColumnStyleXLSX);
    CPPUNIT_TEST(testSharedFormulaHorizontalXLS);
    CPPUNIT_TEST_SUITE_END();

private:
    void testPassword_Impl(const OUString& rFileNameBase);
    uno::Reference<uno::XInterface> m_xCalcComponent;
};

bool ScFiltersTest::load(const OUString &rFilter, const OUString &rURL,
    const OUString &rUserData, unsigned int nFilterFlags,
        unsigned int nClipboardID, unsigned int nFilterVersion)
{
    ScDocShellRef xDocShRef = ScBootstrapFixture::load( rURL, rFilter, rUserData,
        OUString(), nFilterFlags, nClipboardID, nFilterVersion);
    bool bLoaded = xDocShRef.Is();
    
    if (bLoaded)
        xDocShRef->DoClose();
    return bLoaded;
}


namespace {

void testRangeNameImpl(ScDocument* pDoc)
{
    
    
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
    
    pDoc->GetValue(1,1,0, aValue);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("=global2 should be 2", 2.0, aValue);
    pDoc->GetValue(1,3,0, aValue);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("=local2 should be 4", 4.0, aValue);
    pDoc->GetValue(2,0,0, aValue);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("=SUM(global3) should be 10", 10.0, aValue);
    pDoc->GetValue(1,0,1,aValue);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("range name Sheet2.local1 should reference Sheet1.A5", 5.0, aValue);
    
    pDoc->GetValue(0,5,1, aValue);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("formula Global5 should reference Global6 ( which is evaluated as local1 )", 5.0, aValue);
}

}

void ScFiltersTest::testBasicCellContentODS()
{
    ScDocShellRef xDocSh = loadDoc("basic-cell-content.", ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load basic-cell-content.ods", xDocSh.Is());

    ScDocument* pDoc = xDocSh->GetDocument();
    OUString aStr = pDoc->GetString(1, 1, 0); 
    CPPUNIT_ASSERT_EQUAL(OUString("LibreOffice Calc"), aStr);
    double fVal = pDoc->GetValue(1, 2, 0); 
    CPPUNIT_ASSERT_EQUAL(12345.0, fVal);
    aStr = pDoc->GetString(1, 3, 0); 
    CPPUNIT_ASSERT_EQUAL(OUString("A < B"), aStr);

    
    ScRefCellValue aCell;
    aCell.assign(*pDoc, ScAddress(1,4,0)); 
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "This cell must be numeric.", CELLTYPE_VALUE, aCell.meType);
    CPPUNIT_ASSERT_EQUAL(0.0, aCell.mfValue);

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

void ScFiltersTest::testHyperlinksXLSX()
{
    ScDocShellRef xDocSh = loadDoc("hyperlinks.", XLSX);
    ScDocument* pDoc = xDocSh->GetDocument();

    CPPUNIT_ASSERT_EQUAL(OUString("10:ABC10"), pDoc->GetString(ScAddress(0,1,0)));
    CPPUNIT_ASSERT_EQUAL(OUString("10:ABC11"), pDoc->GetString(ScAddress(0,2,0)));
    CPPUNIT_ASSERT_EQUAL(OUString("10:ABC12"), pDoc->GetString(ScAddress(0,3,0)));

    xDocSh->DoClose();
}

void ScFiltersTest::testHardRecalcODS()
{
    ScDocShellRef xDocSh = loadDoc("hard-recalc.", ODS);
    xDocSh->DoHardRecalc(true);

    CPPUNIT_ASSERT_MESSAGE("Failed to load hard-recalc.*", xDocSh.Is());
    ScDocument* pDoc = xDocSh->GetDocument();
    OUString aCSVFileName;

    
    
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

    
    createCSVPath(OUString("logical-functions."), aCSVFileName);
    testFile(aCSVFileName, pDoc, 0);
    
    createCSVPath(OUString("spreadsheet-functions."), aCSVFileName);
    testFile(aCSVFileName, pDoc, 1);
    
    createCSVPath(OUString("mathematical-functions."), aCSVFileName);
    testFile(aCSVFileName, pDoc, 2, PureString);
    
    createCSVPath(OUString("information-functions."), aCSVFileName);
    testFile(aCSVFileName, pDoc, 3);
    
    createCSVPath(OUString("text-functions."), aCSVFileName);
    testFile(aCSVFileName, pDoc, 4);
    
    createCSVPath(OUString("statistical-functions."), aCSVFileName);
    testFile(aCSVFileName, pDoc, 5);

    xDocSh->DoClose();
}

void ScFiltersTest::testFunctionsExcel2010()
{
    ScDocShellRef xDocSh = loadDoc("functions-excel-2010.", XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to load the document.", xDocSh.Is());
    ScDocument* pDoc = xDocSh->GetDocument();
    pDoc->CalcAll(); 

    
    struct
    {
        SCROW nRow;
        bool  bEvaluate;
    } aTests[] = {
        {  2, false },
        {  3, true  },
        {  4, true  },
        {  5, true  },
        {  6, true  },
        {  7, true  },
        {  8, true  },
        {  9, false },
        { 10, false },
        { 11, true  },
        { 12, true  },
        { 13, true  },
        { 14, true  },
        { 15, true  },
        { 16, true  },
        { 17, true  },
        { 18, true  },
        { 19, true  },
        { 20, false },
        { 21, false },
        { 22, true  },
        { 23, true  },
        { 24, true  },
        { 25, true  },
        { 26, true  },
        { 27, true  },
        { 28, true  },
        { 29, true  },
        { 30, false },
        { 31, true  },
        { 32, true  },
        { 33, true  },
        { 34, true  },
        { 35, true  },
        { 36, true  },
        { 37, true  },
        { 38, true  },
        { 39, false },
        { 40, false },
        { 41, false },
        { 42, false },
        { 43, false },
        { 44, true  },
        { 45, true  },
        { 46, true  },
        { 47, true  },
        { 48, true  },
        { 49, true  },
        { 50, true  },
        { 51, true  },
        { 52, true  },
        { 53, true  },
        { 54, true  },
        { 55, true  },
        { 56, true  },
        { 57, true  },
        { 58, true  },
        { 59, true  },
        { 60, true  },
        { 61, true  },
        { 62, true  },
        { 63, true  },
        { 64, true  },
        { 65, true  },
        { 66, false },
        { 67, true  },
        { 68, true  },
        { 69, true  },
        { 70, true  },
        { 71, true  },
        { 72, false },
        { 73, false }
    };

    for (size_t i=0; i < SAL_N_ELEMENTS(aTests); ++i)
    {
        if (aTests[i].bEvaluate)
        {
            
            
            SCROW nRow = aTests[i].nRow - 1;    
            CPPUNIT_ASSERT_MESSAGE( OString( "Expected a formula cell without error at row " +
                    OString::number( aTests[i].nRow)).getStr(),
                    isFormulaWithoutError( *pDoc, ScAddress( 1, nRow, 0)));
            CPPUNIT_ASSERT_MESSAGE( OString( "Expected a TRUE value at row " +
                    OString::number( aTests[i].nRow)).getStr(),
                    0 != pDoc->GetValue( ScAddress( 3, nRow, 0)));
        }
    }

    xDocSh->DoClose();
}

void ScFiltersTest::testCachedFormulaResultsODS()
{
    {
        ScDocShellRef xDocSh = loadDoc("functions.", ODS);
        CPPUNIT_ASSERT_MESSAGE("Failed to load functions.*", xDocSh.Is());

        ScDocument* pDoc = xDocSh->GetDocument();
        OUString aCSVFileName;

        
        createCSVPath(OUString("logical-functions."), aCSVFileName);
        testFile(aCSVFileName, pDoc, 0);
        
        createCSVPath(OUString("spreadsheet-functions."), aCSVFileName);
        testFile(aCSVFileName, pDoc, 1);
        
        createCSVPath(OUString("mathematical-functions."), aCSVFileName);
        testFile(aCSVFileName, pDoc, 2, PureString);
        
        createCSVPath(OUString("information-functions."), aCSVFileName);
        testFile(aCSVFileName, pDoc, 3);
        
        createCSVPath(OUString("text-functions."), aCSVFileName);
        testFile(aCSVFileName, pDoc, 4);

        xDocSh->DoClose();
    }

    {
        ScDocShellRef xDocSh = loadDoc("cachedValue.", ODS);
        CPPUNIT_ASSERT_MESSAGE("Failed to load cachedValue.*", xDocSh.Is());

        ScDocument* pDoc = xDocSh->GetDocument();
        OUString aCSVFileName;
        createCSVPath("cachedValue.", aCSVFileName);
        testFile(aCSVFileName, pDoc, 0);

        
        
        
        
        OUString sTodayCache("01/25/13 01:06 PM");
        OUString sTodayRecalc(pDoc->GetString(0,0,1));

        CPPUNIT_ASSERT(sTodayCache != sTodayRecalc);

        OUString sTodayRecalcRef(pDoc->GetString(1,0,1));
        CPPUNIT_ASSERT_EQUAL(sTodayRecalc, sTodayRecalcRef);

        
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

    
    OUString aCSVFileName;
    createCSVPath("matrix.", aCSVFileName);
    testFile(aCSVFileName, pDoc, 0);
    
    createCSVPath("matrix2.", aCSVFileName);
    testFile(aCSVFileName, pDoc, 1);
    createCSVPath("matrix3.", aCSVFileName);
    testFile(aCSVFileName, pDoc, 2);
    
    
    
    
    
    
    
    CPPUNIT_ASSERT_EQUAL(OUString(), pDoc->GetString(3,0,2));

    
    
    pDoc->SetString(2, 5, 2, "=ISERROR(A6)");
    double nVal = pDoc->GetValue(2,5,2);
    CPPUNIT_ASSERT_EQUAL(1.0, nVal);

    xDocSh->DoClose();
}

void ScFiltersTest::testFormulaDepAcrossSheetsODS()
{
    ScDocShellRef xDocSh = loadDoc("formula-across-sheets.", ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load the file.", xDocSh.Is());
    ScDocument* pDoc = xDocSh->GetDocument();

    sc::AutoCalcSwitch aACSwitch(*pDoc, true); 

    
    double fA4 = pDoc->GetValue(ScAddress(0,3,2));
    double fB4 = pDoc->GetValue(ScAddress(1,3,2));
    double fC4 = pDoc->GetValue(ScAddress(2,3,2));

    
    double fD4 = pDoc->GetValue(ScAddress(3,3,2));
    pDoc->SetValue(ScAddress(3,3,2), fD4+1.0);

    CPPUNIT_ASSERT_MESSAGE("The value must differ from the original.", fA4 != pDoc->GetValue(ScAddress(0,3,2)));
    CPPUNIT_ASSERT_MESSAGE("The value must differ from the original.", fB4 != pDoc->GetValue(ScAddress(1,3,2)));
    CPPUNIT_ASSERT_MESSAGE("The value must differ from the original.", fC4 != pDoc->GetValue(ScAddress(2,3,2)));

    xDocSh->DoClose();
}

void ScFiltersTest::testFormulaDepDeleteContentsODS()
{
    ScDocShellRef xDocSh = loadDoc("formula-delete-contents.", ODS, true);
    CPPUNIT_ASSERT_MESSAGE("Failed to load the file.", xDocSh.Is());
    ScDocument* pDoc = xDocSh->GetDocument();

    sc::UndoSwitch aUndoSwitch(*pDoc, true); 
    sc::AutoCalcSwitch aACSwitch(*pDoc, true); 

    CPPUNIT_ASSERT_EQUAL(195.0, pDoc->GetValue(ScAddress(3,15,0))); 

    
    ScDocFunc& rFunc = xDocSh->GetDocFunc();
    ScRange aRange(3,1,0,3,4,0);
    ScMarkData aMark;
    aMark.SetMarkArea(aRange);
    aMark.MarkToMulti();
    bool bGood = rFunc.DeleteContents(aMark, IDF_ALL, true, true);
    CPPUNIT_ASSERT(bGood);
    CPPUNIT_ASSERT_EQUAL(0.0, pDoc->GetValue(ScAddress(3,1,0)));
    CPPUNIT_ASSERT_EQUAL(0.0, pDoc->GetValue(ScAddress(3,2,0)));
    CPPUNIT_ASSERT_EQUAL(0.0, pDoc->GetValue(ScAddress(3,3,0)));
    CPPUNIT_ASSERT_EQUAL(0.0, pDoc->GetValue(ScAddress(3,4,0)));

    CPPUNIT_ASSERT_EQUAL(94.0, pDoc->GetValue(ScAddress(3,15,0))); 

    SfxUndoManager* pUndoMgr = pDoc->GetUndoManager();
    CPPUNIT_ASSERT(pUndoMgr);
    pUndoMgr->Undo();
    CPPUNIT_ASSERT_EQUAL(195.0, pDoc->GetValue(ScAddress(3,15,0))); 

    xDocSh->DoClose();
}

namespace {

void testDBRanges_Impl(ScDocument* pDoc, sal_Int32 nFormat)
{
    ScDBCollection* pDBCollection = pDoc->GetDBCollection();
    CPPUNIT_ASSERT_MESSAGE("no database collection", pDBCollection);

    ScDBData* pAnonDBData = pDoc->GetAnonymousDBData(0);
    CPPUNIT_ASSERT_MESSAGE("missing anonymous DB data in sheet 1", pAnonDBData);
    
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
    if(nFormat == ODS) 
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

void ScFiltersTest::testFormatsODS()
{
    ScDocShellRef xDocSh = loadDoc("formats.", ODS);
    xDocSh->DoHardRecalc(true);

    ScDocument* pDoc = xDocSh->GetDocument();

    testFormats(this, pDoc, ODS);
    xDocSh->DoClose();
}

void ScFiltersTest::testFormatsXLS()
{
    ScDocShellRef xDocSh = loadDoc("formats.", XLS);
    xDocSh->DoHardRecalc(true);

    ScDocument* pDoc = xDocSh->GetDocument();

    testFormats(this, pDoc, XLS);
    xDocSh->DoClose();
}

void ScFiltersTest::testFormatsXLSX()
{
    ScDocShellRef xDocSh = loadDoc("formats.", XLSX);
    xDocSh->DoHardRecalc(true);

    ScDocument* pDoc = xDocSh->GetDocument();

    testFormats(this, pDoc, XLSX);
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
    CPPUNIT_ASSERT_EQUAL(
        table::BorderLineStyle::SOLID, pRight->GetBorderLineStyle());

    pDoc->GetBorderLines( 2, 1, 0, &pLeft, &pTop, &pRight, &pBottom );
    CPPUNIT_ASSERT(!pLeft);
    CPPUNIT_ASSERT(!pTop);
    CPPUNIT_ASSERT(!pBottom);

    CPPUNIT_ASSERT(pRight);
    CPPUNIT_ASSERT_EQUAL(
        table::BorderLineStyle::SOLID, pRight->GetBorderLineStyle());
    CPPUNIT_ASSERT_EQUAL(20L, pRight->GetWidth());

    pDoc->GetBorderLines( 2, 8, 0, &pLeft, &pTop, &pRight, &pBottom );

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
        
        OUString aCSVFileName;
        createCSVPath(OUString("bugFix_Sheet2."), aCSVFileName);
        testFile(aCSVFileName, pDoc, 1);
    }

    {
        
        ScDBData* pDBData = pDoc->GetDBCollection()->getNamedDBs().findByUpperName("DBRANGE1");
        CPPUNIT_ASSERT(pDBData);
        CPPUNIT_ASSERT(pDBData->HasHeader());
        
        pDBData = pDoc->GetDBCollection()->getNamedDBs().findByUpperName("DBRANGE2");
        CPPUNIT_ASSERT(pDBData);
        CPPUNIT_ASSERT(!pDBData->HasHeader());
    }

    {
        
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
    CPPUNIT_ASSERT_MESSAGE("No Document", pDoc); 
    xDocSh->DoClose();
}

void ScFiltersTest::testBugFixesXLSX()
{
    ScDocShellRef xDocSh = loadDoc("bug-fixes.", XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to load bugFixes.xls", xDocSh.Is());

    xDocSh->DoHardRecalc(true);
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT_MESSAGE("No Document", pDoc); 
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
    ScDocShellRef xDocSh = loadDoc("merged.", ODS);
    ScDocument* pDoc = xDocSh->GetDocument();

    
    OUString aCSVFileName1;
    createCSVPath(OUString("merged1."), aCSVFileName1);
    testFile(aCSVFileName1, pDoc, 0);

    
    checkMergedCells( pDoc, ScAddress( 0, 0, 0 ),  ScAddress( 5, 11, 0 ) );
    checkMergedCells( pDoc, ScAddress( 7, 2, 0 ),  ScAddress( 9, 12, 0 ) );
    checkMergedCells( pDoc, ScAddress( 3, 15, 0 ),  ScAddress( 7, 23, 0 ) );

    
    OUString aCSVFileName2;
    createCSVPath(OUString("merged2."), aCSVFileName2);
    testFile(aCSVFileName2, pDoc, 1);

    
    checkMergedCells( pDoc, ScAddress( 4, 3, 1 ),  ScAddress( 6, 15, 1 ) );

    xDocSh->DoClose();
}

void ScFiltersTest::testRepeatedColumnsODS()
{
    ScDocShellRef xDocSh = loadDoc("repeatedColumns.", ODS);
    ScDocument* pDoc = xDocSh->GetDocument();

    
    OUString aCSVFileName1;
    createCSVPath(OUString("repeatedColumns1."), aCSVFileName1);
    testFile(aCSVFileName1, pDoc, 0);

    
    OUString aCSVFileName2;
    createCSVPath(OUString("repeatedColumns2."), aCSVFileName2);
    testFile(aCSVFileName2, pDoc, 1);

    xDocSh->DoClose();
}

namespace {


struct ValDataTestParams
{
    ScValidationMode eValMode;
    ScConditionMode eCondOp;
    OUString aStrVal1;
    OUString aStrVal2;
    ScDocument* pDocument;
    ScAddress aPosition;
    OUString aErrorTitle;
    OUString aErrorMessage;
    ScValidErrorStyle eErrorStyle;
    sal_uLong nExpectedIndex;

    ValDataTestParams( ScValidationMode eMode, ScConditionMode eOp,
                       OUString aExpr1, OUString aExpr2, ScDocument* pDoc,
                       ScAddress aPos, OUString aETitle, OUString aEMsg,
                       ScValidErrorStyle eEStyle, sal_uLong nIndex ):
                            eValMode(eMode), eCondOp(eOp), aStrVal1(aExpr1),
                            aStrVal2(aExpr2), pDocument(pDoc), aPosition(aPos),
                            aErrorTitle(aETitle), aErrorMessage(aEMsg),
                            eErrorStyle(eEStyle), nExpectedIndex(nIndex) { };
};

void checkValiditationEntries( const ValDataTestParams& rVDTParams )
{
    ScDocument* pDoc = rVDTParams.pDocument;

    
    ScValidationData aValData(
        rVDTParams.eValMode, rVDTParams.eCondOp, rVDTParams.aStrVal1,
        rVDTParams.aStrVal2, pDoc, rVDTParams.aPosition, EMPTY_OUSTRING,
        EMPTY_OUSTRING, pDoc->GetStorageGrammar(), pDoc->GetStorageGrammar()
    );
    aValData.SetIgnoreBlank( true );
    aValData.SetListType( 1 );
    aValData.ResetInput();
    aValData.SetError( rVDTParams.aErrorTitle, rVDTParams.aErrorMessage, rVDTParams.eErrorStyle );
    aValData.SetSrcString( EMPTY_OUSTRING );

    
    const ScValidationData* pValDataTest = pDoc->GetValidationEntry( rVDTParams.nExpectedIndex );

    sal_Int32 nCol( static_cast<sal_Int32>(rVDTParams.aPosition.Col()) );
    sal_Int32 nRow( static_cast<sal_Int32>(rVDTParams.aPosition.Row()) );
    sal_Int32 nTab( static_cast<sal_Int32>(rVDTParams.aPosition.Tab()) );
    OStringBuffer sMsg("Data Validation Entry with base-cell-address: (");
    sMsg.append(nCol).append(",").append(nRow).append(",").append(nTab).append(") ");
    OString aMsgPrefix = sMsg.makeStringAndClear();

    OString aMsg = aMsgPrefix + "did not get imported at all.";
    CPPUNIT_ASSERT_MESSAGE(aMsg.getStr(), pValDataTest);

    
    if (!aValData.EqualEntries(*pValDataTest))
    {
        aMsg = aMsgPrefix + "got imported incorrectly.";
        CPPUNIT_FAIL(aMsg.getStr());
    }
}

void checkCellValidity( const ScAddress& rValBaseAddr, const ScRange& rRange, const ScDocument* pDoc )
{
    SCCOL nBCol( rValBaseAddr.Col() );
    SCROW nBRow( rValBaseAddr.Row() );
    SCTAB nTab( static_cast<const sal_Int32>(rValBaseAddr.Tab()) );
    
    const SfxUInt32Item* pItem = static_cast<const SfxUInt32Item*>(pDoc->GetAttr(nBCol, nBRow, nTab, ATTR_VALIDDATA) );
    const ScValidationData* pValData = pDoc->GetValidationEntry( pItem->GetValue() );

    
    for(SCCOL i = rRange.aStart.Col(); i <= rRange.aEnd.Col(); ++i)
    {
        for(SCROW j = rRange.aStart.Row(); j <= rRange.aEnd.Row(); ++j)
        {
            const SfxUInt32Item* pItemTest = static_cast<const SfxUInt32Item*>( pDoc->GetAttr(i, j, nTab, ATTR_VALIDDATA) );
            const ScValidationData* pValDataTest = pDoc->GetValidationEntry( pItemTest->GetValue() );
            
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

    ScAddress aValBaseAddr1( 2,6,0 ); 
    ScAddress aValBaseAddr2( 2,3,1 ); 

    
    ValDataTestParams aVDTParams1(
        SC_VALID_DECIMAL, SC_COND_GREATER, "3.14", EMPTY_OUSTRING, pDoc,
        aValBaseAddr1, "Too small",
        "The number you are trying to enter is not greater than 3.14! Are you sure you want to enter it anyway?",
        SC_VALERR_WARNING, 1
    );
    
    ValDataTestParams aVDTParams2(
        SC_VALID_WHOLE, SC_COND_BETWEEN, "1", "10", pDoc,
        aValBaseAddr2, "Error sheet 2",
        "Must be a whole number between 1 and 10.",
        SC_VALERR_STOP, 2
    );
    
    checkValiditationEntries( aVDTParams1 );
    checkValiditationEntries( aVDTParams2 );

    
    ScRange aRange1( 2,2,0, 2,6,0 ); 
    ScRange aRange2( 2,3,1, 6,7,1 ); 

    
    checkCellValidity( aValBaseAddr1, aRange1, pDoc );
    checkCellValidity( aValBaseAddr2, aRange2, pDoc );

    
    OUString aCSVFileName1;
    createCSVPath(OUString("dataValidity1."), aCSVFileName1);
    testFile(aCSVFileName1, pDoc, 0);

    OUString aCSVFileName2;
    createCSVPath(OUString("dataValidity2."), aCSVFileName2);
    testFile(aCSVFileName2, pDoc, 1);

    xDocSh->DoClose();
}

void ScFiltersTest::testDataTableMortgageXLS()
{
    ScDocShellRef xDocSh = loadDoc("data-table/mortgage.", XLS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load the document.", xDocSh.Is());

    ScFormulaOptions aOptions;
    aOptions.SetFormulaSepArg(",");
    aOptions.SetFormulaSepArrayCol(",");
    aOptions.SetFormulaSepArrayRow(";");
    xDocSh->SetFormulaOptions(aOptions);

    ScDocument* pDoc = xDocSh->GetDocument();

    

    if (!checkFormula(*pDoc, ScAddress(3,1,0), "PMT(B3/12,B4,-B5)"))
        CPPUNIT_FAIL("Wrong formula!");

    if (!checkFormula(*pDoc, ScAddress(3,2,0), "MULTIPLE.OPERATIONS(D$2,$B$3,$C3)"))
        CPPUNIT_FAIL("Wrong formula!");

    if (!checkFormula(*pDoc, ScAddress(3,3,0), "MULTIPLE.OPERATIONS(D$2,$B$3,$C4)"))
        CPPUNIT_FAIL("Wrong formula!");

    if (!checkFormula(*pDoc, ScAddress(3,4,0), "MULTIPLE.OPERATIONS(D$2,$B$3,$C5)"))
        CPPUNIT_FAIL("Wrong formula!");

    

    if (!checkFormula(*pDoc, ScAddress(2,7,0), "PMT(B9/12,B10,-B11)"))
        CPPUNIT_FAIL("Wrong formula!");

    if (!checkFormula(*pDoc, ScAddress(3,8,0), "MULTIPLE.OPERATIONS($C$8,$B$9,$C9,$B$10,D$8)"))
        CPPUNIT_FAIL("Wrong formula!");

    if (!checkFormula(*pDoc, ScAddress(3,9,0), "MULTIPLE.OPERATIONS($C$8,$B$9,$C10,$B$10,D$8)"))
        CPPUNIT_FAIL("Wrong formula!");

    if (!checkFormula(*pDoc, ScAddress(3,10,0), "MULTIPLE.OPERATIONS($C$8,$B$9,$C11,$B$10,D$8)"))
        CPPUNIT_FAIL("Wrong formula!");

    if (!checkFormula(*pDoc, ScAddress(4,8,0), "MULTIPLE.OPERATIONS($C$8,$B$9,$C9,$B$10,E$8)"))
        CPPUNIT_FAIL("Wrong formula!");

    if (!checkFormula(*pDoc, ScAddress(4,9,0), "MULTIPLE.OPERATIONS($C$8,$B$9,$C10,$B$10,E$8)"))
        CPPUNIT_FAIL("Wrong formula!");

    if (!checkFormula(*pDoc, ScAddress(4,10,0), "MULTIPLE.OPERATIONS($C$8,$B$9,$C11,$B$10,E$8)"))
        CPPUNIT_FAIL("Wrong formula!");

    xDocSh->DoClose();
}

void ScFiltersTest::testDataTableOneVarXLSX()
{
    ScDocShellRef xDocSh = loadDoc("data-table/one-variable.", XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to load the document.", xDocSh.Is());

    ScFormulaOptions aOptions;
    aOptions.SetFormulaSepArg(",");
    aOptions.SetFormulaSepArrayCol(",");
    aOptions.SetFormulaSepArrayRow(";");
    xDocSh->SetFormulaOptions(aOptions);

    ScDocument* pDoc = xDocSh->GetDocument();

    
    
    
    pDoc->CalcAll();

    
    

    if (!checkFormula(*pDoc, ScAddress(1,4,0), "MULTIPLE.OPERATIONS(B$4,$A$2,$A5)"))
        CPPUNIT_FAIL("Wrong formula!");

    CPPUNIT_ASSERT_EQUAL(2.0, pDoc->GetValue(ScAddress(1,4,0)));

    if (!checkFormula(*pDoc, ScAddress(1,10,0), "MULTIPLE.OPERATIONS(B$4,$A$2,$A11)"))
        CPPUNIT_FAIL("Wrong formula!");

    CPPUNIT_ASSERT_EQUAL(14.0, pDoc->GetValue(ScAddress(1,10,0)));

    
    

    if (!checkFormula(*pDoc, ScAddress(4,4,0), "MULTIPLE.OPERATIONS($D5,$B$2,E$4)"))
        CPPUNIT_FAIL("Wrong formula!");

    CPPUNIT_ASSERT_EQUAL(10.0, pDoc->GetValue(ScAddress(4,4,0)));

    if (!checkFormula(*pDoc, ScAddress(8,4,0), "MULTIPLE.OPERATIONS($D5,$B$2,I$4)"))
        CPPUNIT_FAIL("Wrong formula!");

    CPPUNIT_ASSERT_EQUAL(50.0, pDoc->GetValue(ScAddress(8,4,0)));

    xDocSh->DoClose();
}

void ScFiltersTest::testDataTableMultiTableXLSX()
{
    ScDocShellRef xDocSh = loadDoc("data-table/multi-table.", XLSX);
    CPPUNIT_ASSERT_MESSAGE("Failed to load the document.", xDocSh.Is());

    ScFormulaOptions aOptions;
    aOptions.SetFormulaSepArg(",");
    aOptions.SetFormulaSepArrayCol(",");
    aOptions.SetFormulaSepArrayRow(";");
    xDocSh->SetFormulaOptions(aOptions);

    ScDocument* pDoc = xDocSh->GetDocument();

    
    
    
    pDoc->CalcAll();

    
    

    if (!checkFormula(*pDoc, ScAddress(1,3,0), "MULTIPLE.OPERATIONS($A$3,$E$1,$A4,$D$1,B$3)"))
        CPPUNIT_FAIL("Wrong formula!");

    CPPUNIT_ASSERT_EQUAL(1.0, pDoc->GetValue(ScAddress(1,3,0)));

    if (!checkFormula(*pDoc, ScAddress(12,14,0), "MULTIPLE.OPERATIONS($A$3,$E$1,$A15,$D$1,M$3)"))
        CPPUNIT_FAIL("Wrong formula!");

    CPPUNIT_ASSERT_EQUAL(144.0, pDoc->GetValue(ScAddress(12,14,0)));

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
    CPPUNIT_ASSERT_MESSAGE("No Document", pDoc); 

    OUString aSheet2CSV("fdo48621_broken_quotes_exported.");
    OUString aCSVPath;
    createCSVPath( aSheet2CSV, aCSVPath );
    
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
    CPPUNIT_ASSERT_MESSAGE("No Document", pDoc); 

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
        
        xDocSh.Clear();
    }

    CPPUNIT_ASSERT_MESSAGE("Failed to load password.ods", xDocSh.Is());
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT_MESSAGE("No Document", pDoc); 
    xDocSh->DoClose();

}

void ScFiltersTest::testPasswordNew()
{
    
    const OUString aFileNameBase("password.");
    testPassword_Impl(aFileNameBase);
}

void ScFiltersTest::testPasswordOld()
{
    
    const OUString aFileNameBase("passwordOld.");
    testPassword_Impl(aFileNameBase);
}

void ScFiltersTest::testPasswordWrongSHA()
{
    
    
    const OUString aFileNameBase("passwordWrongSHA.");
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

    
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "There should be 4 sheets in this document.", sal_Int16(4),
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

    
    const SdrOle2Obj* pOleObj = getSingleChartObject(*pDoc, 1);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve a chart object from the 2nd sheet.", pOleObj);

    ScRangeList aRanges = getChartRanges(*pDoc, *pOleObj);

    CPPUNIT_ASSERT_MESSAGE("Data series title cell not found.", aRanges.In(ScAddress(1,0,3))); 
    CPPUNIT_ASSERT_MESSAGE("Data series label range not found.", aRanges.In(ScRange(0,1,2,0,3,2))); 
    CPPUNIT_ASSERT_MESSAGE("Data series value range not found.", aRanges.In(ScRange(1,1,2,1,3,2))); 

    xDocSh->DoClose();
}

void ScFiltersTest::testNumberFormatHTML()
{
    ScDocShellRef xDocSh = loadDoc("numberformat.", HTML);
    CPPUNIT_ASSERT_MESSAGE("Failed to load numberformat.html", xDocSh.Is());

    ScDocument* pDoc = xDocSh->GetDocument();

    
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Cell value is not as expected", OUString("Product"),
        pDoc->GetString(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Cell value is not as expected", OUString("Price"),
        pDoc->GetString(1, 0, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Cell value is not as expected", OUString("Note"),
        pDoc->GetString(2, 0, 0));

    
    bool bHasValue = pDoc->HasValueData(1, 1, 0);
    CPPUNIT_ASSERT_MESSAGE("Fail to import number as a value cell.", bHasValue);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Incorrect value.", 199.98, pDoc->GetValue(1, 1, 0));

    xDocSh->DoClose();
}

void ScFiltersTest::testNumberFormatCSV()
{
    ScDocShellRef xDocSh = loadDoc("numberformat.", CSV);
    CPPUNIT_ASSERT_MESSAGE("Failed to load numberformat.csv", xDocSh.Is());

    ScDocument* pDoc = xDocSh->GetDocument();

    
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Cell value is not as expected", OUString("Product"),
        pDoc->GetString(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Cell value is not as expected", OUString("Price"),
        pDoc->GetString(1, 0, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Cell value is not as expected", OUString("Note"),
        pDoc->GetString(2, 0, 0));

    
    bool bHasValue = pDoc->HasValueData(1, 1, 0);
    CPPUNIT_ASSERT_MESSAGE("Fail to import number as a value cell.", bHasValue);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Incorrect value.", 199.98, pDoc->GetValue(1, 1, 0));

    xDocSh->DoClose();
}

void ScFiltersTest::testCellAnchoredShapesODS()
{
    ScDocShellRef xDocSh = loadDoc("cell-anchored-shapes.", ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load cell-anchored-shapes.ods", xDocSh.Is());

    
    ScDocument* pDoc = xDocSh->GetDocument();

    CPPUNIT_ASSERT_MESSAGE("There should be at least one sheet.", pDoc->GetTableCount() > 0);

    ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
    SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("draw page for sheet 1 should exist.", pPage);
    sal_uIntPtr nCount = pPage->GetObjCount();
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "There should be 2 objects.", sal_uIntPtr(2), nCount);
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
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "There should be exactly two sheets.", sal_Int16(2),
        pDoc->GetTableCount());

    ScDPCollection* pDPs = pDoc->GetDPCollection();
    CPPUNIT_ASSERT_MESSAGE("Failed to get a live ScDPCollection instance.", pDPs);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "There should be exactly one pivot table instance.", size_t(1),
        pDPs->GetCount());

    const ScDPObject* pDPObj = (*pDPs)[0];
    CPPUNIT_ASSERT_MESSAGE("Failed to get an pivot table object.", pDPObj);
    const ScDPSaveData* pSaveData = pDPObj->GetSaveData();
    CPPUNIT_ASSERT_MESSAGE("Failed to get ScDPSaveData instance.", pSaveData);
    std::vector<const ScDPSaveDimension*> aDims;

    
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

    
    pSaveData->GetAllDimensionsByOrientation(sheet::DataPilotFieldOrientation_COLUMN, aDims);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "There should be exactly 2 column fields.",
        std::vector<ScDPSaveDimension const *>::size_type(2), aDims.size());
    CPPUNIT_ASSERT_MESSAGE("Dimension expected, but not found.", hasDimension(aDims, "Col1"));
    CPPUNIT_ASSERT_MESSAGE("Dimension expected, but not found.", hasDimension(aDims, "Col2"));

    
    pSaveData->GetAllDimensionsByOrientation(sheet::DataPilotFieldOrientation_PAGE, aDims);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "There should be exactly 2 page fields.",
        std::vector<ScDPSaveDimension const *>::size_type(2), aDims.size());
    CPPUNIT_ASSERT_MESSAGE("Dimension expected, but not found.", hasDimension(aDims, "Page1"));
    CPPUNIT_ASSERT_MESSAGE("Dimension expected, but not found.", hasDimension(aDims, "Page2"));

    
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
                
                return false;
        }
        else
        {
            if (pMem->GetIsVisible())
                
                return false;
        }
    }

    return bFound;
}

}

void ScFiltersTest::testPivotTableSharedCacheGroupODS()
{
    ScDocShellRef xDocSh = loadDoc("pivot-table-shared-cache-with-group.", ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load file", xDocSh.Is());
    ScDocument* pDoc = xDocSh->GetDocument();

    

    ScDPObject* pDPObj = pDoc->GetDPAtCursor(0, 0, 1); 
    CPPUNIT_ASSERT_MESSAGE("There should be a pivot table here.", pDPObj);
    ScDPSaveData* pSaveData = pDPObj->GetSaveData();
    CPPUNIT_ASSERT_MESSAGE("Save data is expected.", pSaveData);
    ScDPSaveDimension* pDim = pSaveData->GetExistingDimensionByName("Project Name");
    CPPUNIT_ASSERT_MESSAGE("Failed to get page field named 'Project Name'.", pDim);
    const ScDPSaveDimension::MemberList* pMembers = &pDim->GetMembers();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(14), pMembers->size());
    CPPUNIT_ASSERT_MESSAGE("Incorrect member visibility.", checkVisiblePageFieldMember(*pMembers, "APL-01-1"));

    pDPObj = pDoc->GetDPAtCursor(0, 1, 2); 
    CPPUNIT_ASSERT_MESSAGE("There should be a pivot table here.", pDPObj);
    pSaveData = pDPObj->GetSaveData();
    CPPUNIT_ASSERT_MESSAGE("Save data is expected.", pSaveData);
    pDim = pSaveData->GetExistingDimensionByName("Project Name");
    CPPUNIT_ASSERT_MESSAGE("Failed to get page field named 'Project Name'.", pDim);
    pMembers = &pDim->GetMembers();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(14), pMembers->size());
    CPPUNIT_ASSERT_MESSAGE("Incorrect member visibility.", checkVisiblePageFieldMember(*pMembers, "VEN-01-1"));

    
    
    ScDPCollection* pDPs = pDoc->GetDPCollection();
    ScDPCollection::SheetCaches& rSheetCaches = pDPs->GetSheetCaches();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rSheetCaches.size());

    
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
                    aChecks.push_back(i); 
                aChecks.push_back(ScDPItemData::DateLast);
                CPPUNIT_ASSERT_MESSAGE("Unexpected group values for the month group.", aGrpValues == aChecks);
            }
            break;
            case sheet::DataPilotFieldGroupBy::YEARS:
            {
                bHasYears = true;
                aMemberIds.clear();
                pCache->GetGroupDimMemberIds(nGrpDim, aMemberIds);

                
                

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
    ScDocShellRef xDocSh = loadDoc("pivot-getpivotdata.", XLS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load file", xDocSh.Is());
    ScDocument* pDoc = xDocSh->GetDocument();
    pDoc->CalcAll();

    
    

    for (SCROW nRow = 2; nRow <= 19; ++nRow)
        CPPUNIT_ASSERT_EQUAL(pDoc->GetValue(ScAddress(4,nRow,1)), pDoc->GetValue(ScAddress(5,nRow,1)));

    xDocSh->DoClose();
}

void ScFiltersTest::testRowHeightODS()
{
    ScDocShellRef xDocSh = loadDoc("row-height-import.", ODS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load row-height-import.ods", xDocSh.Is());

    SCTAB nTab = 0;
    SCROW nRow = 0;
    ScDocument* pDoc = xDocSh->GetDocument();

    
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

    
    bManual = pDoc->IsManualRowHeight(++nRow, nTab);
    CPPUNIT_ASSERT_MESSAGE("Row should have an automatic height.", !bManual);

    
    nHeight = pDoc->GetRowHeight(++nRow, nTab, false);
    bManual = pDoc->IsManualRowHeight(nRow, nTab);
    CPPUNIT_ASSERT_EQUAL(2400, nHeight);
    CPPUNIT_ASSERT_MESSAGE("this row should have a manual row height.", bManual);

    
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

    
    ScAddress aPos(0, 0, 0);

    
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_STRING, pDoc->GetCellType(aPos));
    CPPUNIT_ASSERT_EQUAL(OUString("Normal"), pDoc->GetString(aPos.Col(), aPos.Row(), aPos.Tab()));

    
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

    
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_EDIT, pDoc->GetCellType(aPos));
    pEditText = pDoc->GetEditText(aPos);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve edit text object.", pEditText);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), pEditText->GetParagraphCount());
    aParaText = pEditText->GetText(0);
    CPPUNIT_ASSERT_MESSAGE("Unexpected text.", aParaText.startsWith("Sheet name is "));
    CPPUNIT_ASSERT_MESSAGE("Sheet name field item not found.", pEditText->HasField(text::textfield::Type::TABLE));
    CPPUNIT_ASSERT_EQUAL(OUString("Sheet name is Test."), ScEditUtil::GetString(*pEditText, pDoc));
    CPPUNIT_ASSERT_EQUAL(OUString("Sheet name is ?."), ScEditUtil::GetString(*pEditText, NULL));

    
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_EDIT, pDoc->GetCellType(aPos));
    pEditText = pDoc->GetEditText(aPos);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve edit text object.", pEditText);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), pEditText->GetParagraphCount());
    aParaText = pEditText->GetText(0);
    CPPUNIT_ASSERT_MESSAGE("Unexpected text.", aParaText.startsWith("URL: "));
    CPPUNIT_ASSERT_MESSAGE("URL field item not found.", pEditText->HasField(text::textfield::Type::URL));
    CPPUNIT_ASSERT_EQUAL(OUString("URL: http:
    CPPUNIT_ASSERT_EQUAL(OUString("URL: http:

    
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_EDIT, pDoc->GetCellType(aPos));
    pEditText = pDoc->GetEditText(aPos);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve edit text object.", pEditText);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), pEditText->GetParagraphCount());
    aParaText = pEditText->GetText(0);
    CPPUNIT_ASSERT_MESSAGE("Unexpected text.", aParaText.startsWith("Date: "));
    CPPUNIT_ASSERT_MESSAGE("Date field item not found.", pEditText->HasField(text::textfield::Type::DATE));
    CPPUNIT_ASSERT_MESSAGE("Date field not resolved with pDoc.", ScEditUtil::GetString(*pEditText, pDoc).indexOf("/20") > 0);
    CPPUNIT_ASSERT_MESSAGE("Date field not resolved with NULL.", ScEditUtil::GetString(*pEditText, NULL).indexOf("/20") > 0);

    
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_EDIT, pDoc->GetCellType(aPos));
    pEditText = pDoc->GetEditText(aPos);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve edit text object.", pEditText);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), pEditText->GetParagraphCount());
    aParaText = pEditText->GetText(0);
    CPPUNIT_ASSERT_MESSAGE("Unexpected text.", aParaText.startsWith("Title: "));
    CPPUNIT_ASSERT_MESSAGE("DocInfo title field item not found.", pEditText->HasField(text::textfield::Type::DOCINFO_TITLE));
    CPPUNIT_ASSERT_EQUAL(OUString("Title: Test Document"), ScEditUtil::GetString(*pEditText, pDoc));
    CPPUNIT_ASSERT_EQUAL(OUString("Title: ?"), ScEditUtil::GetString(*pEditText, NULL));

    
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

    
    
    
    aPos.IncRow();
    pEditText = pDoc->GetEditText(aPos);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve edit text object.", pEditText);
    CPPUNIT_ASSERT_MESSAGE("URL field item not found.", pEditText->HasField(text::textfield::Type::URL));

    
    aPos.IncRow();
    pEditText = pDoc->GetEditText(aPos);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve edit text object.", pEditText);
    CPPUNIT_ASSERT_MESSAGE("Sheet name field item not found.", pEditText->HasField(text::textfield::Type::TABLE));

    
    aPos.IncRow();
    pEditText = pDoc->GetEditText(aPos);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve edit text object.", pEditText);
    CPPUNIT_ASSERT_MESSAGE("Date field item not found.", pEditText->HasField(text::textfield::Type::DATE));

    
    aPos.IncRow();
    pEditText = pDoc->GetEditText(aPos);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve edit text object.", pEditText);
    CPPUNIT_ASSERT_MESSAGE("Date field item not found.", pEditText->HasField(text::textfield::Type::DOCINFO_TITLE));

    
    aPos.IncRow();
    pEditText = pDoc->GetEditText(aPos);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve edit text object.", pEditText);
    const SvxFieldData* pData = pEditText->GetFieldData(0, 0, text::textfield::Type::URL);
    CPPUNIT_ASSERT_MESSAGE("Failed to get the URL data.", pData && pData->GetClassId() == text::textfield::Type::URL);
    const SvxURLField* pURLData = static_cast<const SvxURLField*>(pData);
    CPPUNIT_ASSERT_MESSAGE("URL is not absolute with respect to the file system.", pURLData->GetURL().startsWith("file:

    
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_STRING, pDoc->GetCellType(aPos));
    CPPUNIT_ASSERT_EQUAL(OUString("one     two"), pDoc->GetString(aPos.Col(), aPos.Row(), aPos.Tab()));

    
    aPos.IncRow();
    CPPUNIT_ASSERT_EQUAL(CELLTYPE_STRING, pDoc->GetCellType(aPos));
    CPPUNIT_ASSERT_EQUAL(OUString(" =3+4"), pDoc->GetString(aPos.Col(), aPos.Row(), aPos.Tab()));

    
    
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

    
    double nVal = pDoc->GetValue(0,0,0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(nVal, 1.0, 1e-10);
    pDoc->SetValue(0,1,0, 0.0);
    nVal = pDoc->GetValue(0,0,0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(nVal, 2.0, 1e-10);

    
    

    xDocSh->DoClose();
}

void ScFiltersTest::testMiscRowHeights()
{
    TestParam::RowData DfltRowData[] =
    {
        
        
        
        { 2, 4, 0, -1, 0, false  },
        { 1048573, 1048575, 0, -1, 0, false  },
    };

    TestParam::RowData MultiLineOptData[] =
    {
        
        { 0, 0, 0, 1263, CHECK_OPTIMAL, true  },
        
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





void ScFiltersTest::testOptimalHeightReset()
{
    ScDocShellRef xDocSh = loadDoc("multilineoptimal.", ODS, true);
    SCTAB nTab = 0;
    SCROW nRow = 0;
    ScDocument* pDoc = xDocSh->GetDocument();
    pDoc->EnableAdjustHeight( true );
    
    
    int nHeight = sc::TwipsToHMM ( pDoc->GetRowHeight(nRow, nTab, false) );
    CPPUNIT_ASSERT_EQUAL(1263, nHeight);

    ScDocFunc &rFunc = xDocSh->GetDocFunc();

    
    ScRange aDelRange(0,0,0,0,0,0);
    ScMarkData aMark;
    aMark.SetMarkArea(aDelRange);
    rFunc.DeleteContents( aMark, IDF_ALL, false, true );

    
    nHeight =  sc::TwipsToHMM( pDoc->GetRowHeight(nRow, nTab, false) );

    
    SCCOLROW nRowArr[2];
    nRowArr[0] = nRowArr[1] = 2;
    rFunc.SetWidthOrHeight( false, 1, nRowArr, nTab, SC_SIZE_OPTIMAL, 0, true, true );

    
    int nOptimalHeight = sc::TwipsToHMM( pDoc->GetRowHeight( nRowArr[0], nTab, false) );

    
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

void ScFiltersTest::testColumnStyleXLSX()
{
    ScDocShellRef xDocSh = loadDoc("column-style.", XLSX);
    CPPUNIT_ASSERT(xDocSh.Is());
    ScDocument* pDoc = xDocSh->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    const ScPatternAttr* pPattern = pDoc->GetPattern(0,0,0);
    CPPUNIT_ASSERT(pPattern);

    const ScProtectionAttr& rAttr = static_cast<const ScProtectionAttr&>(pPattern->GetItem(ATTR_PROTECTION));
    CPPUNIT_ASSERT(rAttr.GetProtection());

    pPattern = pDoc->GetPattern(0,1,0);
    CPPUNIT_ASSERT(pPattern);

    const ScProtectionAttr& rAttrNew = static_cast<const ScProtectionAttr&>(pPattern->GetItem(ATTR_PROTECTION));
    CPPUNIT_ASSERT(!rAttrNew.GetProtection());

    xDocSh->DoClose();
}

void ScFiltersTest::testSharedFormulaHorizontalXLS()
{
    ScDocShellRef xDocSh = loadDoc("shared-formula/horizontal.", XLS);
    CPPUNIT_ASSERT(xDocSh.Is());
    ScDocument* pDoc = xDocSh->GetDocument();

    
    ScAddress aPos(0, 1, 1);
    for (SCCOL nCol = 10; nCol <= 18; ++nCol)
    {
        aPos.SetCol(nCol);
        CPPUNIT_ASSERT_MESSAGE("Formula cell is expected here.", pDoc->GetCellType(aPos) == CELLTYPE_FORMULA);
    }

    
    for (SCCOL nCol = 1; nCol <= 9; ++nCol)
    {
        aPos.SetCol(nCol);
        for (SCROW nRow = 2; nRow <= 8; ++nRow)
        {
            aPos.SetRow(nRow);
            CPPUNIT_ASSERT_MESSAGE("Formula cell is expected here.", pDoc->GetCellType(aPos) == CELLTYPE_FORMULA);
        }
    }

    
    aPos.SetRow(1);
    for (SCCOL nCol = 1; nCol <= 8; ++nCol)
    {
        aPos.SetCol(nCol);
        CPPUNIT_ASSERT_MESSAGE("Formula cell is expected here.", pDoc->GetCellType(aPos) == CELLTYPE_FORMULA);
    }

    
    aPos.SetCol(9);
    CPPUNIT_ASSERT_EQUAL(OUString("MW"), pDoc->GetString(aPos));

    xDocSh->DoClose();
}

ScFiltersTest::ScFiltersTest()
      : ScBootstrapFixture( "/sc/qa/unit/data" )
{
}

void ScFiltersTest::setUp()
{
    test::BootstrapFixture::setUp();

    
    
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
